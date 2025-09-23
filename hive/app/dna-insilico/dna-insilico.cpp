/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <qlib/QPrideProc.hpp>
#include <ulib/ulib.hpp>
#include <math.h>
#include <violin/violin.hpp>
#include <qlib/QPrideClient.hpp>
#include <qpsvc/archiver.hpp>
#include <ssci/math/rand/rand.hpp>
#include <ssci/bio/randseq.hpp>
#include <ion/sIon-core.hpp>
#include <ctime>

#define posMatrix( _v_i, _v_j, _v_len, _v_mat) _v_mat[(((_v_i)+1) * ((_v_len)+1)) + ((_v_j)+1)]
#define RAND1 (real)rand()/RAND_MAX

class DnaInsilicoProc: public sQPrideProc
{
    public:
        DnaInsilicoProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
            init();
        }

        sRandomSeq randseq;

        void init()
        {
            qryFiles.cut(0);
            idAppend.cut(0);
            randseq.init();
            useDefLineColumnAsPrefix = false;
        }

        sStr qryFiles, idAppend;
        sHiveId qryFileId;
        sHiveId mutFileId;
        sHiveId targetSeqFileId;
        sHiveId qualityFileId;
        sHiveId annotFileId;
        sHiveId cnvFileId;
        bool useDefLineColumnAsPrefix;

        virtual idx OnExecute(idx);
        bool loadGeneralAttributes(sStr *err);
        bool loadRefsForm(sStr *err);

        bool mutationCSVoutput(sFil *out, sBioseq &sub, sStr &err);
        bool mutationVCFoutput(sFil *out, const char *refId, sBioseq &sub, sStr &err);
        idx parseAnnotation(sBioseq &sub, sStr &err);
        idx parseRangeExtractionFile(sBioseq &sub, sStr &err);
        bool validateHiveID (sHiveId &hiveId, sStr *err, sStr *cont = 0, const char *key = 0);
        bool generateFastaFile(const char *newOutputfile, const char *outPath, idx option, sStr &err);
        bool generateRecombination(sFil &out, sFil &outTable, sStr &err);
        bool generateCNV(sFil &out, sStr &err);
        static idx rangeComparator(DnaInsilicoProc * myThis, void * arr, udx i1, udx i2);
        bool printSequenceRanges (sFil &out, sBioseq *sub, idx irow, idx start = 0, idx end = 0, idx isrev = false, idx iscomp = false);

        bool reqInitFile(sMex & fil, const char * name, const char *ext, sStr * path_buf = 0)
        {
            static sStr local_path_buf;
            if( !path_buf ) {
                path_buf = &local_path_buf;
            }
            const char * path = reqAddFile(*path_buf, "%s%s", name, ext);
            if( path && fil.init(path) && fil.ok() ) {
#ifdef _DEBUG
                logOut(eQPLogType_Trace, "Created %s", path);
#endif
                return true;
            } else {
                logOut(eQPLogType_Error, "Failed to create %s%s", name, ext);
                return false;
            }
        }

};

idx DnaInsilicoProc::rangeComparator(DnaInsilicoProc * myThis, void * arr, udx i1, udx i2)
{
    sRandomSeq::RangeSeq * range1 = myThis->randseq.rangeContainer.ptr(i1);
    sRandomSeq::RangeSeq * range2 = myThis->randseq.rangeContainer.ptr(i2);

    if (range1->destSeqnum < range2->destSeqnum){
        return -1;
    }
    else if (range2->destSeqnum < range1->destSeqnum){
        return 1;
    }

    if (range1->destPosition < range2->destPosition){
        return -1;
    }
    return 1;
}

bool DnaInsilicoProc::validateHiveID (sHiveId &hiveId, sStr *err, sStr *cont, const char *key)
{
    if (hiveId.objId() == 0){
        return false;
    }
    sUsrFile obj2(hiveId, user);
    sStr aux;

    if( obj2.Id() ) {
        aux.cut(0);
        if (key){
            obj2.getFilePathname(aux, key);
        }
        else {
            obj2.getFile(aux);
        }
        if( !sFile::exists(aux) ) {
            err->printf("No data file for file with ID: %s; terminating\n", hiveId.print());
            return false;
        }
    }
    if (cont){
        cont->add(aux, aux.length());
    }
    return true;
}

bool DnaInsilicoProc::loadGeneralAttributes(sStr *err)
{
    const sUsrObjPropsTree * tree = objs[0].propsTree();
    if (!tree){
        err->printf("Variable data (Props Tree) is not accessible");
        return false;
    }

    randseq.quaType = tree->findIValue("inSilicoQuaType");
    sStr flnmQuality;
    flnmQuality.cut(0);
    if( randseq.quaType == 2 ) {
        sHiveseq qFile(user, qualityFileId.print());
        tree->findHiveIdValue("inSilicoQuaObjId", qualityFileId);

        bool usequaFile = validateHiveID (qualityFileId, err, &flnmQuality, ".qc2.sumPositionTable.csv");

        if (!usequaFile && (qFile.dim() > 0)){
            err->printf("File to mimic qualities is not accessible or invalid");
            return false;
        }
    }

    const char *hiveseqstring = tree->findValue("inSilicoSourceObjId", 0);
    sHiveseq qryPreloadedGenome(user, hiveseqstring);
    if (qryPreloadedGenome.dim()){
        qryFiles.printf(0,"%s", hiveseqstring);
    }

    tree->findHiveIdValue("inSilicoMutationTable", mutFileId);
    bool useMutFile = validateHiveID (mutFileId, err);
    if (useMutFile && !qryPreloadedGenome.dim()){
        err->printf("A Preloaded Genome is not accessible or invalid\n");
        return false;
    }

    sFil tblFile(0);
    sStr flnmTargetSeq;
    flnmTargetSeq.cut(0);
    tree->findHiveIdValue("inSilicoTargetSeqTable", targetSeqFileId);
    bool useTargetFile = validateHiveID (targetSeqFileId, err, &flnmTargetSeq);
    if (useTargetFile){
        tblFile.init(flnmTargetSeq, sMex::fReadonly);
        if( !tblFile.ok() ) {
            err->printf("Range Extraction file is not accessible\n");
            return false;
        }
        if (!qryPreloadedGenome.dim()){
            err->printf("A Preloaded Genome is not accessible to extract ranges\n");
            return false;
        }
    }

    tree->findHiveIdValue("inSilicoAnnotObjID", annotFileId);
    sHiveIon hionAnnot(user, annotFileId.print(), 0, "ion");

    tree->findHiveIdValue("inSilicoCNVObjID", cnvFileId);
    sHiveIon hivecnvionAnnot(user, cnvFileId.print(), 0, "ion");
    if( hivecnvionAnnot.ionCnt ) {
        if (!qryPreloadedGenome.dim()){
            err->printf("A Preloaded Genome is not accessible or invalid\n");
            return false;
        }
    }


    idx option = tree->findIValue("inSilicoOption", 0);

    idx inSilicoFlags = 0;
    if( option == 0 ) {
        idx format = tree->findIValue("inSilicoFormat", 1);
        inSilicoFlags |= (format == 0) ? sRandomSeq::eSeqFastA : sRandomSeq::eSeqFastQ;

        idx showId = tree->findIValue("inSilicoIDField", 0);

        inSilicoFlags |= (showId == 0) ? sRandomSeq::eSeqNoId : 0;

        randseq.numReads = tree->findIValue("inSilicoReadsNumber", 1000);
        if( !randseq.validate(randseq.numReads, 1, -1) ) {
            err->printf("Invalid number of Reads");
            return false;
        }
        randseq.minLength = tree->findIValue("inSilicoMinLength", 100);
        randseq.maxLength = tree->findIValue("inSilicoMaxLength", 100);
        randseq.lengthDistributionType = tree->findIValue("inSilicoLengthDist", 0);

        if( randseq.minLength > randseq.maxLength ) {
            err->printf("Minimum length must be greater than Maximum Length");
            return false;
        }
        if( !randseq.validate(randseq.minLength, 0, -1) && !randseq.validate(randseq.maxLength, 0, -1) ) {
            err->printf("Invalid length");
            return false;
        }

        randseq.lowComplexityString.printf("%s", tree->findValue("inSilicoLowCompString", 0));
        if( randseq.validate(randseq.lowComplexityString) ) {
            randseq.lowComplexityMin = tree->findIValue("inSilicoLowCompMin", 0);
            randseq.lowComplexityMax = tree->findIValue("inSilicoLowCompMax", 0);
            randseq.lowComplexityFreq = tree->findIValue("inSilicoLowCompFreq", 0);
            if( (randseq.lowComplexityMax == -1 || randseq.lowComplexityMin < randseq.lowComplexityMax) && randseq.validate(randseq.lowComplexityFreq, 1, 100) ) {
                randseq.prepareLowComplexity();
                inSilicoFlags |= sRandomSeq::eSeqLowComplexity;
            }
        } else {
            randseq.lowComplexityMin = 0;
            randseq.lowComplexityMax = 0;
            randseq.lowComplexityFreq = 0;
        }

        randseq.primersString.printf("%s", tree->findValue("inSilicoPrimerString", 0));
        if( randseq.validate(randseq.primersString) ) {
            randseq.primersMin = tree->findIValue("inSilicoPrimerMin", 0);
            randseq.primersFreq = tree->findIValue("inSilicoPrimerFreq", 0);
            if( randseq.validate(randseq.primersFreq, 1, -1) && randseq.validate(randseq.primersFreq, 0, 100) ) {
                randseq.preparePrimers();
                inSilicoFlags |= sRandomSeq::eSeqPrimers;
            }
        } else {
            randseq.primersMin = 0;
            randseq.primersFreq = 0;
        }

        randseq.strandedness = tree->findIValue("inSilicoStranded", 2);
        bool pairedEnd = tree->findBoolValue("inSilicoPEactive", false);
        if( pairedEnd ) {
            randseq.minPEread = tree->findIValue("inSilicoPEmindist", 100);
            randseq.maxPEread = tree->findIValue("inSilicoPEmaxdist", 500);
            if( randseq.minPEread < randseq.minLength ) {
                err->printf("Please check your Paired end inputs, minimum Paired End (%" DEC " < minimum Length %" DEC ")", randseq.minPEread, randseq.minLength);
                return false;
            }
            if( (randseq.minPEread < randseq.maxPEread) && randseq.validate(randseq.minPEread, 0, -1) && randseq.validate(randseq.maxPEread, 0, -1) ) {
                inSilicoFlags |= sRandomSeq::eSeqPrintPairedEnd;
            } else {
                err->printf("Please check the Paired end inputs, they are invalid");
                return false;
            }
        } else {
            randseq.minPEread = 0;
            randseq.maxPEread = 0;
        }

        randseq.quaMinValue = tree->findIValue("inSilicoQuaMin", 25);
        randseq.quaMaxValue = tree->findIValue("inSilicoQuaMax", 40);

        if( randseq.quaType == 2 ) {
            randseq.qualityTable = 0;
            inSilicoFlags |= sRandomSeq::eSeqMimicQuality;
            sFil tblFile(flnmQuality.ptr(), sMex::fReadonly);
            randseq.qualityTable = randseq.tableParser(flnmQuality.ptr(), ",", true, 0);
        }

        inSilicoFlags |= sRandomSeq::eSeqPrintRandomReads;
    } else if( option == 1 ) {
        inSilicoFlags |= sRandomSeq::eSeqPrintRecombinantsOnly;
    }

    randseq.complexityEntropy = tree->findRValue("inSilicoLCentropy", 0);
    randseq.complexityWindow = tree->findIValue("inSilicoLCwindow", 0);
    if( randseq.complexityWindow && randseq.complexityEntropy ) {
        inSilicoFlags |= sRandomSeq::eSeqComplexityFilter;
    }

    randseq.filterNperc = tree->findIValue("inSilicofilterNperc", -1);
    if( randseq.validate(randseq.filterNperc, 0, 100) ) {
        inSilicoFlags |= sRandomSeq::eSeqFilterNs;
    }

    bool qryPreGenomeDim = qryPreloadedGenome.dim() > 0 ? true : false;
    if (qryPreGenomeDim){
        inSilicoFlags |= sRandomSeq::eSeqPreloadedGenomeFile;
    }
    if( useMutFile && qryPreGenomeDim ) {
        randseq.qryFileCnt = qryPreloadedGenome.dim();
        inSilicoFlags |= sRandomSeq::eSeqParseMutationFile;
    }

    if( useTargetFile && qryPreGenomeDim ) {
        randseq.qryFileCnt = qryPreloadedGenome.dim();
        sFil tblFile(flnmTargetSeq.ptr(), sMex::fReadonly);
        randseq.rangeExtraction = randseq.tableParser(flnmTargetSeq, ",", true, 0);
        inSilicoFlags |= sRandomSeq::eSeqParseTargetSeq;
    }

    const char *rangeExtractionString = tree->findValue("inSilicoRangeExtractInline", 0);
    if (rangeExtractionString && qryPreGenomeDim){
        sTxtTbl *tbl = new sTxtTbl();
        tbl->setBuf(rangeExtractionString);
        idx flags = sTblIndex::fColsepCanRepeat | sTblIndex::fLeftHeader | sTblIndex::fTopHeader;
        tbl->parseOptions().flags = flags;
        tbl->parseOptions().colsep = ",";
        tbl->parseOptions().comment = "#";
        tbl->parseOptions().initialOffset = 0;
        tbl->parse();
        tbl->remapReadonly();
        if (randseq.rangeExtraction){
        }
        else {
            randseq.rangeExtraction = tbl;
        }
        inSilicoFlags |= sRandomSeq::eSeqParseTargetSeq;
    }



    if( hionAnnot.ionCnt ) {
        randseq.useAllAnnotRanges = tree->findBoolValue("inSilicoAnnotAll", false);

        if  (!randseq.useAllAnnotRanges){
            for(const sUsrObjPropsNode * nodeAnnotSeqRow = tree->find("inSilicoAnnotExtraction"); nodeAnnotSeqRow; nodeAnnotSeqRow = nodeAnnotSeqRow->nextSibling("inSilicoAnnotExtraction")) {
                sRandomSeq::AnnotQuery * annot = randseq.annotRanges.add(1);
                const char *seqid = nodeAnnotSeqRow->findValue("inSilicoAnnotSeqID", 0);
                const char *id = nodeAnnotSeqRow->findValue("inSilicoAnnotID", 0);
                const char *type = nodeAnnotSeqRow->findValue("inSilicoAnnotType", 0);
                randseq.addAnnotRange (annot, randseq.annotStringBuf, seqid, id, type);
            }
        }
        if( randseq.useAllAnnotRanges || (randseq.annotRanges.dim() != 0) ) {
            inSilicoFlags |= sRandomSeq::eSeqParseAnnotFile;
        }
    }

        idx countRanges = randseq.cnvRanges.dim();
        for(const sUsrObjPropsNode * nodeAnnotSeqRow = tree->find("inSilicoCNVList"); nodeAnnotSeqRow; nodeAnnotSeqRow = nodeAnnotSeqRow->nextSibling("inSilicoCNVList")) {
            sRandomSeq::AnnotQuery * annot = randseq.cnvRanges.add(1);
            const char *seqid = nodeAnnotSeqRow->findValue("inSilicoCNVSeqID", 0);
            const char *id = nodeAnnotSeqRow->findValue("inSilicoCNVId", 0);
            const char *type = nodeAnnotSeqRow->findValue("inSilicoCNVFeature", 0);
            idx tandem = nodeAnnotSeqRow->findIValue("inSilicoCNVTandem", 0);
            idx after = nodeAnnotSeqRow->findIValue("inSilicoCNVAfter", 0);
            idx before = nodeAnnotSeqRow->findIValue("inSilicoCNVBefore", 0);
            bool valid = randseq.addAnnotRange(annot, randseq.annotStringBuf, seqid, id, type, tandem, before, after, (void *)nodeAnnotSeqRow);
            if (!valid){
                randseq.cnvRanges.cut(randseq.cnvRanges.dim()-1);
            }
        }

        if( randseq.cnvRanges.dim() != countRanges ) {
            inSilicoFlags |= sRandomSeq::eSeqParseCNVFile;
            if ((inSilicoFlags & sRandomSeq::eSeqPreloadedGenomeFile) == 0){
                err->printf("error: CNV can't read the preloaded Genome");
                return false;
            }

        }

    idx randseed = tree->findIValue("inSilicoRandSeed", rand_seed);
    if (!randseed){
        inSilicoFlags |= sRandomSeq::eSeqSetRandomSeed0;
    }
    randseq.setRandSeed(randseed);

    randseq.noiseOriginalTable[0][0] = tree->findRValue("inSilicoNoiseAA", 1);
    randseq.noiseOriginalTable[0][1] = tree->findRValue("inSilicoNoiseAC", 1);
    randseq.noiseOriginalTable[0][2] = tree->findRValue("inSilicoNoiseAG", 1);
    randseq.noiseOriginalTable[0][3] = tree->findRValue("inSilicoNoiseAT", 1);
    randseq.noiseOriginalTable[0][4] = tree->findRValue("inSilicoNoiseAI", 0);
    randseq.noiseOriginalTable[0][5] = tree->findRValue("inSilicoNoiseAD", 0);
    randseq.noiseOriginalTable[1][0] = tree->findRValue("inSilicoNoiseCA", 1);
    randseq.noiseOriginalTable[1][1] = tree->findRValue("inSilicoNoiseCC", 1);
    randseq.noiseOriginalTable[1][2] = tree->findRValue("inSilicoNoiseCG", 1);
    randseq.noiseOriginalTable[1][3] = tree->findRValue("inSilicoNoiseCT", 1);
    randseq.noiseOriginalTable[1][4] = tree->findRValue("inSilicoNoiseCI", 0);
    randseq.noiseOriginalTable[1][5] = tree->findRValue("inSilicoNoiseCD", 0);
    randseq.noiseOriginalTable[2][0] = tree->findRValue("inSilicoNoiseGA", 1);
    randseq.noiseOriginalTable[2][1] = tree->findRValue("inSilicoNoiseGC", 1);
    randseq.noiseOriginalTable[2][2] = tree->findRValue("inSilicoNoiseGG", 1);
    randseq.noiseOriginalTable[2][3] = tree->findRValue("inSilicoNoiseGT", 1);
    randseq.noiseOriginalTable[2][4] = tree->findRValue("inSilicoNoiseGI", 0);
    randseq.noiseOriginalTable[2][5] = tree->findRValue("inSilicoNoiseGD", 0);
    randseq.noiseOriginalTable[3][0] = tree->findRValue("inSilicoNoiseTA", 1);
    randseq.noiseOriginalTable[3][1] = tree->findRValue("inSilicoNoiseTC", 1);
    randseq.noiseOriginalTable[3][2] = tree->findRValue("inSilicoNoiseTG", 1);
    randseq.noiseOriginalTable[3][3] = tree->findRValue("inSilicoNoiseTT", 1);
    randseq.noiseOriginalTable[3][4] = tree->findRValue("inSilicoNoiseTI", 0);
    randseq.noiseOriginalTable[3][5] = tree->findRValue("inSilicoNoiseTD", 0);
    randseq.noiseOriginalTable[4][0] = 1;
    randseq.noiseOriginalTable[4][1] = 1;
    randseq.noiseOriginalTable[4][2] = 1;
    randseq.noiseOriginalTable[4][3] = 1;
    randseq.noiseOriginalTable[4][4] = 0;
    randseq.noiseOriginalTable[4][5] = 0;

    if( !randseq.normalizeTable() ) {
        err->printf("error at parsing the table");
        return false;
    }
    randseq.noisePercentage = tree->findRValue("inSilicoNoiseType", 0);
    if( randseq.noisePercentage != 0 ) {
        inSilicoFlags |= sRandomSeq::eSeqNoise;
    }

    randseq.randMutNumber = tree->findIValue("inSilicoPreMutNum", 0);
    if( randseq.validate(randseq.randMutNumber, 1, -1) ) {
        inSilicoFlags |= sRandomSeq::eSeqGenerateRandomMut;
    }
    randseq.randMutStringLen = tree->findIValue("inSilicoPreMutLength", 0);
    if( randseq.randMutStringLen == 0 ) {
        randseq.randMutStringLen = 1;
    }
    randseq.randMutFreq = tree->findIValue("inSilicoPreMutFrequency", 0);
    if( !randseq.validate(randseq.randMutFreq, 0, 100) ) {
        randseq.randMutFreq = -1;
    }

    randseq.setFlags(inSilicoFlags);

    return true;
}


bool DnaInsilicoProc::loadRefsForm(sStr *err)
{
    sUsrObjPropsTree propsTree(*user, "svc-dna-insilico");
    propsTree.useForm(*pForm);
    idx numNodeEntries = 0;

    sStr hiveseqList;
    sStr auxbuf;
    idx iref = 0;
    idx refLengthMax = 0;

    randseq.mutationStringContainer.cut(0);


    for(const sUsrObjPropsNode * nodeEntry = propsTree.find("inSilicoEntry"); nodeEntry; nodeEntry = nodeEntry->nextSibling("inSilicoEntry"), numNodeEntries++) {
        randseq.refSeqs.resize(iref + 1);

        refLengthMax = 0;
        idx refcount = 0;
        idx accumulateLength = 0;
        randseq.refSeqs[iref].rangeseqsOffset = randseq.rangeContainer.dim();
        for(const sUsrObjPropsNode * nodeRefSeqRow = nodeEntry->find("inSilicoRefSeq"); nodeRefSeqRow; nodeRefSeqRow = nodeRefSeqRow->nextSibling("inSilicoRefSeq")) {
            hiveseqList.cut(0);
            randseq.rangeContainer.add();
            sRandomSeq::RangeSeq * range = randseq.rangeContainer.ptr(randseq.refSeqs[iref].rangeseqsOffset + refcount);
            sHiveId hiveId;
            nodeRefSeqRow->findHiveIdValue("inSilicoObjID", hiveId);
            if( !hiveId ) {
                continue;
            }
#if _DEBUG
            fprintf(stderr, "ref %" DEC ": add inSilicoObjID = %s\n", iref, hiveId.print());
#endif
            idx irow = nodeRefSeqRow->findIValue("inSilicoSeqID");
            range->sourceStartRange = nodeRefSeqRow->findIValue("inSilicoStartRange")-1;
            range->sourceEndRange = nodeRefSeqRow->findIValue("inSilicoEndRange")-1;
            range->orientation = nodeRefSeqRow->findIValue("inSilicoOrientation");
            range->tandem = 1;

            sHiveseq qry(user, hiveId.print(), sBioseq::eBioModeLong);

            if( irow <= 0 ) {
                irow = 0;
            }
            range->sourceSeqnum = -1;
            range->destSeqnum = -1;
            range->hiveseqListOffset = randseq.hiveseqListContainer.length();
            randseq.hiveseqListContainer.printf("%s, %" DEC ", %" DEC, hiveId.print(), irow, irow);
            randseq.hiveseqListContainer.add0();
            if( !qry.dim() ) {
                err->printf("Sequence specified is not valid in ObjId: %s \n", hiveId.print());
                return false;
            }
            idx seqlen = qry.len(irow-1);

            if( (range->sourceEndRange > 0) && range->sourceStartRange > range->sourceEndRange ) {
                err->printf("Sequence range specified is not valid in ObjId: %s \n", hiveId.print());
                return false;

            }
            if( range->sourceStartRange > seqlen ) {
                err->printf("Sequence Start range is invalid in ObjId: %s (%" DEC " > %" DEC ")", hiveId.print(), range->sourceStartRange, seqlen);
                return false;
            }
            if( range->sourceStartRange < 0 ) {
                range->sourceStartRange = 0;
            }
            if( (range->sourceEndRange < 0) || (range->sourceEndRange >= seqlen) ) {
                range->sourceEndRange = seqlen - 1;
            }

            accumulateLength += range->sourceEndRange - range->sourceStartRange;
            if( accumulateLength > refLengthMax ) {
                refLengthMax = accumulateLength;
            }

            ++refcount;
        }


        randseq.refSeqs[iref].rangecnt = refcount;
        if( !refcount ) {
            err->printf("No RefSeq's in the form; terminating\n");
            return false;
        }
        randseq.refSeqs[iref].coverage = nodeEntry->findIValue("inSilicoCoverage");
        if( randseq.refSeqs[iref].coverage <= 0 ) {
            randseq.refSeqs[iref].coverage = 0;
        }

        refcount = 0;
        randseq.refSeqs[iref].mutInfoOffset = randseq.mutContainer.dim();
        for(const sUsrObjPropsNode * nodeMutRow = nodeEntry->find("inSilicoMutation"); nodeMutRow; nodeMutRow = nodeMutRow->nextSibling("inSilicoMutation")) {
            randseq.mutContainer.add();
            sRandomSeq::RefMutation *mutInfo = randseq.mutContainer.ptr(randseq.refSeqs[iref].mutInfoOffset + refcount);
            mutInfo->mutationOffset = randseq.mutationStringContainer.length();
            randseq.mutationStringContainer.addString(nodeMutRow->findValue("inSilicoMutString"));
            if( randseq.mutationStringContainer.length() == mutInfo->mutationOffset ) {
                randseq.mutContainer.cut(randseq.mutContainer.dim() - 1);
                continue;
            }
            randseq.mutationStringContainer.add0();
            mutInfo->position = nodeMutRow->findIValue("inSilicoMutPos") - 1;
            mutInfo->refNumSeq = iref;
            mutInfo->refLength = 1;
            mutInfo->refBase = 0;
            mutInfo->frequency = nodeMutRow->findIValue("inSilicoMutFreq");
            mutInfo->quality = nodeMutRow->findIValue("inSilicoMutQua");
            mutInfo->allele = nodeMutRow->findIValue("inSilicoMutAllele");
            mutInfo->mutBiasStart = nodeMutRow->findIValue("inSilicoMutBiasStart");
            mutInfo->mutBiasEnd = nodeMutRow->findIValue("inSilicoMutBiasEnd");
            mutInfo->groupid = refcount;
            mutInfo->count = 0;
            mutInfo->miss = 0;
            ++refcount;
        }

        for(const sUsrObjPropsNode * nodeMutRandRow = nodeEntry->find("inSilicoRandomMutation"); nodeMutRandRow; nodeMutRandRow = nodeMutRandRow->nextSibling("inSilicoMutation")) {
            idx randNumber = nodeMutRandRow->findIValue("inSilicoRandMutNumber");
            idx randlength = nodeMutRandRow->findIValue("inSilicoRandMutStringLength");
            idx randfreq = nodeMutRandRow->findIValue("inSilicoRandMutFrequency");
            idx randqua = nodeMutRandRow->findIValue("inSilicoRandMutQuality");
            idx randallele = nodeMutRandRow->findIValue("inSilicoRandMutAllele");

            if( !randseq.validate(randNumber, 0, -1) ) {
                err->printf("Invalid number of mutations (%" DEC ") to generate randomly", randNumber);
                return false;
            }
            for(idx imut = 0; imut < randNumber; ++imut) {
                randseq.mutContainer.add();
                sRandomSeq::RefMutation *mutInfo = randseq.mutContainer.ptr(randseq.refSeqs[iref].mutInfoOffset + refcount);
                randseq.generateRandString(auxbuf, randlength);

                mutInfo->mutationOffset = randseq.mutationStringContainer.length();
                randseq.mutationStringContainer.addString(auxbuf.ptr(), randlength);
                randseq.mutationStringContainer.add0();
                mutInfo->position = (idx) randseq.randDist(0, refLengthMax);
                mutInfo->refNumSeq = iref;
                mutInfo->refLength = 1;
                mutInfo->refBase = -1;
                mutInfo->frequency = (randfreq == -1) ? (idx) randseq.randDist(0, 100) : randfreq;
                mutInfo->quality = (randqua == -1) ? (idx) randseq.randDist(randseq.quaMinValue, randseq.quaMaxValue) : randqua;
                mutInfo->allele = (randallele == -1) ? (idx) randseq.randDist(0, 1 + nodeEntry->findIValue("inSilicoDiploidicity")) : randallele;
                mutInfo->mutBiasStart = 0;
                mutInfo->mutBiasEnd = 0;
                mutInfo->groupid = refcount;
                mutInfo->count = 0;
                mutInfo->miss = 0;
                ++refcount;
            }
        }
        randseq.refSeqs[iref].mutcnt = refcount;
        if( refcount ) {
            randseq.addFlags (sRandomSeq::eSeqMutations);
        }

        randseq.refSeqs[iref].diploidicity = nodeEntry->findIValue("inSilicoDiploidicity");

        iref++;
    }

    if( !numNodeEntries ) {
        err->printf("No ObjID's in the form; terminating\n");
        return false;
    }

    if( iref > 0 ) {
        return true;
    }
    err->printf("No data in inSilicoRefSeq in form; terminating\n");
    return false;
}

idx DnaInsilicoProc::parseAnnotation(sBioseq &sub, sStr &err)
{

    idx cntRanges = randseq.rangeContainer.dim();
    sHiveIon hionAnnot(user, annotFileId.print(), 0, "ion");

    sIonWander * wander = 0;

    if( randseq.useAllAnnotRanges ) {
        wander = hionAnnot.addIonWander("getRange", "b=foreach.seqID(\"\");a=find.annot(seqID=b.1,type='strand');");
        if( !wander ) {
            err.printf("Error at extracting ranges from Annot File");
            return 0;
        }
        randseq.Sub = &sub;
        wander->callbackFunc = sRandomSeq::ionWanderCallback;
        wander->callbackFuncParam = (void *) &randseq;
        wander->traverse();

    } else {
        sStr findAnnot, name;
        for(idx i = 0; i < randseq.annotRanges.dim(); ++i) {
            findAnnot.printf(0, "a=find.annot(");
            name.printf(0, "getRange%" DEC, i + 1);
            sRandomSeq::AnnotQuery *annot = randseq.annotRanges.ptr(i);

            if( annot->seqoffset != -1 ) {
                findAnnot.printf("seqID=\"%s\",", randseq.annotStringBuf.ptr(annot->seqoffset));
            }
            if( annot->typeoffset != -1 ) {
                findAnnot.printf("type=\"%s\",", randseq.annotStringBuf.ptr(annot->typeoffset));
            }
            if( annot->idoffset != -1 ) {
                findAnnot.printf("id=\"%s\",", randseq.annotStringBuf.ptr(annot->idoffset));
            }
            findAnnot.cut(-1);
            findAnnot.printf(");");

            wander = hionAnnot.addIonWander(name.ptr(0), "%s", findAnnot.ptr());
            if( !wander ) {
                err.printf("Error at extracting ranges from Annot File");
                return 0;
            }
            wander->callbackFunc = sRandomSeq::ionWanderCallback;
            wander->callbackFuncParam = (void *) &randseq;
            wander->traverse();
        }
    }
    return (randseq.rangeContainer.dim() - cntRanges);
}

idx DnaInsilicoProc::parseRangeExtractionFile(sBioseq &sub, sStr &err)
{

    sTxtTbl * table = randseq.getrangeTable();
    idx cntRefs = 0;
    randseq.Sub = &sub;
    idx failedToExtract = 0;
    if (table){
        enum rangeExtractionTable
        {
            recombID = -1,
            seqID = 0,
            startrange = 1,
            endrange = 2,
            direction = 3,
            defLine = 4
        };
        idx dimTable = table->rows();
        if (!dimTable){
            err.printf("Empty table to extract ranges");
            return 0;
        }
        sStr cbuf;
        idx irow, seqstart, seqend, seqdirection;
        idx prevRecombID, currRecombID;
        idx seqlen;
        if (table->colId("id", 2) == defLine){
            useDefLineColumnAsPrefix = true;
        }
        prevRecombID = -1;
        for(idx j = 0; j < dimTable; ++j) {
            cbuf.cut(0);
            currRecombID = table->ival(j, recombID);
            table->printCell(cbuf, j, seqID);
            irow = sFilterseq::getrownum(randseq.idlist, cbuf.ptr());
            if( irow >= 0 && irow < sub.dim() ) {
                seqstart = table->ival(j, startrange) - 1;
                seqend = table->ival(j, endrange) - 1;
                seqdirection = table->ival(j, direction);

                seqlen = sub.len(irow);
                if( (seqstart > 0) && (seqstart > seqend) ) {
                    failedToExtract += 1;
                    err.printf("please check start and end ranges in row number %" DEC, j+1);
                    return 0;
                }
                if( seqstart > seqlen ) {
                    failedToExtract += 1;
                    err.printf("please check start and end ranges in row number %" DEC, j+1);
                    return 0;
                }
                if( seqstart < 0 ) {
                    seqstart = 0;
                }
                if( (seqend < 0) || (seqend >= seqlen) ) {
                    seqend = seqlen - 1;
                }
                if (currRecombID != prevRecombID){
                    randseq.refSeqs.resize(cntRefs + 1);
                    randseq.refSeqs[cntRefs].rangeseqsOffset = randseq.rangeContainer.dim();
                    randseq.refSeqs[cntRefs].rangecnt = 1;
                    randseq.refSeqs[cntRefs].coverage = 0;
                    ++cntRefs;
                    prevRecombID = currRecombID;
                }
                else {
                    randseq.refSeqs[cntRefs-1].rangecnt += 1;
                }
                sRandomSeq::RangeSeq *range = randseq.rangeContainer.add();
                range->sourceSeqnum = irow;
                range->sourceStartRange = seqstart;
                range->sourceEndRange = seqend;
                range->destSeqnum = -1;
                range->hiveseqListOffset = 0;
                range->orientation = seqdirection;
                range->coverage = 1;

                cbuf.cut(0);
                table->printCell(cbuf, j, defLine);
                if (cbuf.length()){
                    range->posidAppend = idAppend.length();
                    idAppend.add(cbuf.ptr(), cbuf.length());
                    idAppend.add0();
                }
                else {
                    range->posidAppend = -1;
                }
            }
            else {
                failedToExtract += 1;
            }
        }
    }
    else {
        err.printf("table to extract ranges is not accessible");
    }
    if (!cntRefs){
        err.printf("table to extract ranges can't match id's with source File");
    }
    return cntRefs;
}

bool DnaInsilicoProc::generateFastaFile(const char *newOutputfile, const char *outPathPrefix, idx option, sStr &err)
{
    sFil srcFile, tblFile;
    sStr pathfilename;
    if( !reqInitFile(srcFile, outPathPrefix, ".fasta", &pathfilename) ) {
        err.printf("failed to create destination recombination fasta file \n");
        return false;
    }
    srcFile.empty();



    bool success = false;
    if (option == 0){
        if( !reqInitFile(tblFile, outPathPrefix, ".csv") ) {
            err.printf("failed to create destination recombination csv file \n");
            return false;
        }
        success = generateRecombination(srcFile, tblFile, err);
    }
    else {
        success = generateCNV(srcFile, err);
    }

    if( !success || srcFile.length() == 0 ) {
        return false;
    }


    sFil vioseqFile(newOutputfile);
    if( !vioseqFile.ok() ) {
        err.printf("failed to create destination vioseq2 file \n");
        return false;
    }
    vioseqFile.empty();
    sVioseq2 v;
    idx reqsubmitAlgorithm = randseq.launchParser(newOutputfile, pathfilename, v, err);

    if( !reqsubmitAlgorithm ) {
        err.printf("Cannot launch PARSER process");
        return false;
    }
    return true;
}



bool DnaInsilicoProc::generateRecombination(sFil &out, sFil &outTable, sStr &err)
{

    idx sStart, seqLen;
    bool isrev = false, iscomp = false;
    bool quaBit;
    sStr t;
    sStr id;
    sBioseq *sub;
    outTable.addString("recombinantID,seqID,start,end,direction,label\n");
    for(idx iref = 0; iref < randseq.refSeqs.dim(); ++iref) {
        id.cut(0);
        idx irecomb = 0;
        idx irow = 0;
        for(irecomb = 0; irecomb < randseq.refSeqs[iref].rangecnt; ++irecomb) {
            sRandomSeq::RangeSeq * range = randseq.rangeContainer.ptr(randseq.refSeqs[iref].rangeseqsOffset + irecomb);
            sHiveseq intSub;
            if (range->sourceSeqnum < 0){
                if (!intSub.parse(randseq.hiveseqListContainer.ptr(range->hiveseqListOffset), sBioseq::eBioModeLong, true, user)){
                    continue;
                }
                sub = &intSub;
                irow = 0;
            }
            else {
                sub = randseq.Sub;
                irow = range->sourceSeqnum;
            }
            id.printf(";%" DEC ".",irecomb + 1);
            if (useDefLineColumnAsPrefix){
                id.printf("%s ", idAppend.ptr(range->posidAppend));
            }
            id.printf("%s: RANGE (%" DEC ",%" DEC ")", sub->id(irow), range->sourceStartRange+1, range->sourceEndRange+1);
            switch(range->orientation) {
                case 1:
                    id.printf("REV");
                    break;
                case 2:
                    id.printf("COMP");
                    break;
                case 3:
                    id.printf("REV COMP");
                    break;
            }
            if ((range->posidAppend != -1) && (!useDefLineColumnAsPrefix) ){
                id.printf(" %s", idAppend.ptr(range->posidAppend));
            }
        }
        out.printf(">%s\n", id.ptr(irecomb == 1 ? 3 : 1));
        t.cut(0);
        for(irecomb = 0; irecomb < randseq.refSeqs[iref].rangecnt; ++irecomb) {
            sRandomSeq::RangeSeq * range = randseq.rangeContainer.ptr(randseq.refSeqs[iref].rangeseqsOffset + irecomb);
            sHiveseq intSub;
            if (range->sourceSeqnum < 0){
                if (!intSub.parse(randseq.hiveseqListContainer.ptr(range->hiveseqListOffset), sBioseq::eBioModeLong, true, user)){
                    continue;
                }
                irow = 0;
                sub = &intSub;
            }
            else {
                sub = randseq.Sub;
                irow = range->sourceSeqnum;
            }
            idx seqlen = sub->len(irow);
            sStart = range->sourceStartRange;
            seqLen = range->sourceEndRange - range->sourceStartRange+1;
            randseq.translateRevComp(range->orientation, &isrev, &iscomp);
            if( sStart < 0 ) {
                sStart = 0;
            }
            if( range->sourceEndRange < 0 ) {
                seqLen = seqlen - sStart;
            }
            outTable.printf("%" DEC ",%s,%" DEC ",%" DEC ",%" DEC ",%s\n", iref+1, sub->id(irow), sStart + 1, sStart + seqLen, range->orientation, idAppend.length() ? idAppend.ptr(range->posidAppend) : "");
            const char * seq = sub->seq(irow);
            idx startT = t.length();
            sBioseq::uncompressATGC(&t, seq, sStart, seqLen, true, 0, isrev, iscomp);
            quaBit = sub->getQuaBit(irow);
            if( quaBit ) {
                const char * seqqua = sub->qua(irow);
                if( seqqua ) {
                    if( !isrev ) {
                        for(idx i = sStart, pos = startT; i < sStart + seqLen; ++i, ++pos) {
                            if( sub->Qua(seqqua, i, quaBit) == 0 ) {
                                t[pos] = 'N';
                            }
                        }
                    } else {
                        for(idx i = sStart + seqLen - 1, pos = startT; i >= sStart; --i, ++pos) {
                            if( sub->Qua(seqqua, i, quaBit) == 0 ) {
                                t[pos] = 'N';
                            }
                        }
                    }
                }
            }
        }
        out.printf("%s\n", t.ptr());
    }
    return true;
}

bool DnaInsilicoProc::generateCNV(sFil &out, sStr &err)
{
    sHiveIon hionAnnot(user, cnvFileId.print(), 0, "ion");
    sStr findAnnot, name, t;
    sIonWander * wander = 0;
    sHiveseq sub(user, qryFiles, sBioseq::eBioModeLong);
    randseq.Sub = &sub;

    randseq.refSeqs.cut(0);
    randseq.refSeqs.resize(sub.dim());

    for(idx i = 0; i < randseq.cnvRanges.dim(); ++i) {
        findAnnot.printf(0, "a=find.annot(");
        name.printf(0, "getRange%" DEC, i + 1);
        sRandomSeq::AnnotQuery *rangeQuery = randseq.cnvRanges.ptr(i);

        const sUsrObjPropsNode *node = (const sUsrObjPropsNode *)rangeQuery->extraInfo;

        bool extractRangeION = true;
        sRandomSeq::RangeSeq rangeInfo;

        if (!node){
            continue;
        }
        idx action = node->findIValue("inSilicoCNVAction", 0);
        idx rangeAfter = node->findIValue("inSilicoCNVAfter", 0);
        idx rangeBefore = node->findIValue("inSilicoCNVBefore", 0);

        idx isValid = true;
        switch (action) {
            case 0:
                rangeInfo.tandem = node->findIValue("inSilicoCNVTandem", 0);
                if (rangeInfo.tandem <= 0){
                    isValid = false;
                }
                break;

            case 1:
                rangeInfo.orientation = 3;
                break;

            case 2:
                rangeInfo.deleteRange = true;
                break;

            case 3:
                rangeInfo.destSeqnum = node->findIValue("inSilicoCNVTargetSeqID", -1);
                rangeInfo.destPosition = node->findIValue("inSilicoCNVTargetPosition", -1);
                if (rangeInfo.destSeqnum == -1 || rangeInfo.destPosition == -1){
                    isValid = false;
                }
                break;
        };

        if (!isValid){
            continue;
        }

        if (node){
            node->findIValue("inSilicoCNVSeqID", 0);
            node->findIValue("inSilicoCNVStartRange", 0);
            node->findIValue("inSilicoCNVEndRange", 0);
            node->findIValue("inSilicoCNVAction", 0);
            node->findIValue("inSilicoCNVTandem", 0);
            node->findIValue("inSilicoCNVTargetPosition", 0);
            node->findIValue("inSilicoCNVTargetSeqID", 0);
            extractRangeION = false;
        }


        if (extractRangeION){
            if( rangeQuery->seqoffset != -1 ) {
                findAnnot.printf("seqID=\"%s\",", randseq.annotStringBuf.ptr(rangeQuery->seqoffset));
            }
            if( rangeQuery->typeoffset != -1 ) {
                findAnnot.printf("type=\"%s\",", randseq.annotStringBuf.ptr(rangeQuery->typeoffset));
            }
            if( rangeQuery->idoffset != -1 ) {
            }
            findAnnot.cut(-1);
            findAnnot.printf(");");

            wander = hionAnnot.addIonWander(name.ptr(0), "%s", findAnnot.ptr());
            if( !wander ) {
                err.printf("Error at extracting ranges from Copy Number Variant File");
                return 0;
            }
            idx rangeExistingStart = randseq.rangeContainer.dim();

            wander->callbackFunc = sRandomSeq::ionWanderCallback;
            wander->callbackFuncParam = (void *) &randseq;
            wander->traverse();

            sRandomSeq::RangeSeq *rCont = randseq.rangeContainer.ptr(rangeExistingStart);
            idx rLen = randseq.rangeContainer.dim() - rangeExistingStart;

            if (rLen && 1){
                idx rmin = sIdxMax;
                idx rmax = 0;
                idx irow = rCont[0].sourceSeqnum;
                for (idx j = 0; j < rLen; ++j){
                    if (rCont[j].sourceSeqnum == irow){
                        if (rmin > rCont[j].sourceStartRange){
                            rmin = rCont[j].sourceStartRange;
                        }
                        if (rmax < rCont[j].sourceEndRange){
                            rmax = rCont[j].sourceEndRange;
                        }
                    }
                }

                if (rangeBefore > 0){
                    rmin -= rangeBefore;
                }
                if (rangeAfter > 0){
                    rmax += rangeAfter;
                }
                rangeInfo.sourceSeqnum = irow;
                rangeInfo.sourceStartRange = rmin;
                rangeInfo.sourceEndRange = rmax;
            }
            randseq.rangeContainer.cut(rangeExistingStart);
        }
        else {
            rangeInfo.sourceSeqnum = node->findIValue("inSilicoCNVSeqID", 0);
            rangeInfo.sourceStartRange = node->findIValue("inSilicoCNVStartRange", 0);
            rangeInfo.sourceEndRange = node->findIValue("inSilicoCNVEndRange", 0);

        }
        sRandomSeq::RangeSeq *r0 = randseq.rangeContainer.add(1);
        r0->loadRange(rangeInfo);
    }

    sVec <idx> sortRangeContainer;
    sortRangeContainer.resize(randseq.rangeContainer.dim());
    sSort::sortSimpleCallback((sSort::sCallbackSorterSimple) rangeComparator, (void *) this, randseq.rangeContainer.dim(), randseq.rangeContainer.ptr(0), sortRangeContainer.ptr(0));

    sRandomSeq::RangeSeq *range = randseq.rangeContainer.ptr(sortRangeContainer[0]);
    sRandomSeq::RangeSeq *lastRange = 0;
    idx iRange = 0;
    idx totRanges = randseq.rangeContainer.dim();
    for(idx i = 0; i < sub.dim(); ++i) {

        if (iRange >= totRanges || range->destSeqnum != i){
            sub.printFastXRow(&out, false, i, 0, 0, 0, true);
            continue;
        }
        idx auxRange = iRange;
        out.printf(">%s", sub.id(i));
        while ( auxRange < totRanges && range->destSeqnum == i){
            out.printf(" RANGE %" DEC " (%" DEC ",%" DEC ")", range->tandem, range->sourceStartRange + 1, range->sourceEndRange+1);
            range = randseq.rangeContainer.ptr(sortRangeContainer[++auxRange]);
        }
        out.printf("\n");

        range = randseq.rangeContainer.ptr(sortRangeContainer[iRange]);
        printSequenceRanges (out, &sub, range->sourceSeqnum, 0, range->sourceStartRange);
        while( iRange < totRanges && range->destSeqnum == i ) {
            for (idx n = 0; n <= range->tandem; ++n){
                printSequenceRanges (out, &sub, range->sourceSeqnum, range->sourceStartRange, range->sourceEndRange);
            }
            lastRange = range;
            range = randseq.rangeContainer.ptr(sortRangeContainer[++iRange]);
        }

        printSequenceRanges (out, &sub, lastRange->sourceSeqnum, lastRange->sourceEndRange, -1);

        out.printf("\n");
    }


    return true;
}

bool DnaInsilicoProc::printSequenceRanges (sFil &out, sBioseq *sub, idx irow, idx start, idx end, idx isrev, idx iscomp)
{
    const char * seq = sub->seq(irow);
    idx sLen = sub->len(irow);
    idx startOut = out.length();
    if (start < 0){
        start = 0;
    }
    if (end == -1){
        end = sLen;
    }
    idx seqLen = end-start;
    if (seqLen <= 0){
        return false;
    }
    if (sLen < seqLen){
        seqLen = sLen;
    }
    sBioseq::uncompressATGC(&out, seq, start, seqLen, true, 0, isrev, iscomp);
    char *t = out.ptr(startOut);
    bool quaBit = sub->getQuaBit(irow);
    if( quaBit ) {
        const char * seqqua = sub->qua(irow);
        if( seqqua ) {
            if( !isrev ) {
                for(idx i = start, pos = 0; i < start + seqLen; ++i, ++pos) {
                    if( sub->Qua(seqqua, i, quaBit) == 0 ) {
                        t[pos] = 'N';
                    }
                }
            } else {
                for(idx i = start + seqLen - 1, pos = 0; i >= start; --i, ++pos) {
                    if( sub->Qua(seqqua, i, quaBit) == 0 ) {
                        t[pos] = 'N';
                    }
                }
            }
        }
    }
    return true;
}

idx DnaInsilicoProc::OnExecute(idx req)
{
    init();
    const sHiveId objID = objs[0].Id();
    bool createFastaFile = false;

    sUsrFile sf(objID, user);
    if( !sf.Id()) {
        logOut(eQPLogType_Info, "Object %s not found or access denied", objID.print());
        reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", objID.print());
        return 1;
    }

    logOut(eQPLogType_Info, "processing object %s\n", objID.print());


    sStr prefixId;
    prefixId.printf("HIVE-%s ", objs[0].IdStr());
    randseq.setPrefixID(prefixId.ptr());
    randseq.m_callback = reqProgressStatic;
    randseq.m_callbackParam = this;


    sStr err;
    reqProgress(1, 1, 100);

    if( !loadGeneralAttributes(&err) ) {
        logOut(eQPLogType_Error, "%s\n", err.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
        return 1;
    }

    reqProgress(1, 3, 100);

    sStr lazyLenPath;
    sStr hivesStr;
    idx randseqFlags = randseq.getFlags ();

    sHiveseq qry2;

    if (randseqFlags & sRandomSeq::eSeqPreloadedGenomeFile){
        qry2.parse(qryFiles, sBioseq::eBioModeLong, true, user);
        if( !qry2.dim() ) {
            logOut(eQPLogType_Error, "Preloaded Genome File is not valid in ObjId: %s \n", hivesStr.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "Preloaded Genome File is not valid in ObjId: %s \n", hivesStr.ptr());
            return false;
        }
    }


    if( randseqFlags & sRandomSeq::eSeqParseTargetSeq ) {

        bool parseDicValidation = sFilterseq::parseDicBioseq(randseq.idlist, qry2, &err);
        if( !parseDicValidation ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }

        if( !parseRangeExtractionFile(qry2, err) ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }
        randseq.emptyIdDictionary();
        createFastaFile = true;
    }

    if( ((randseqFlags & sRandomSeq::eSeqParseMutationFile) == 0) && ((randseqFlags & sRandomSeq::eSeqPreloadedGenomeFile) == 0) ) {
        if( !loadRefsForm(&err) ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }
        createFastaFile = true;
    }

    if( createFastaFile){
        sStr newOutputfile;
        sf.addFilePathname(newOutputfile, true, "_.vioseq2");
        bool success = generateFastaFile (newOutputfile.ptr(), "genomeRecombination", 0, err);
        if( !success) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }
        hivesStr.printf(0, "%s", newOutputfile.ptr());
    } else {
        hivesStr.printf(0, "%s", qryFiles.ptr());
    }
    sHiveseq qry(user, hivesStr.ptr(0), sBioseq::eBioModeLong);

    if( !qry.dim() ) {
        logOut(eQPLogType_Error, "Genome Resultant is not valid in ObjId: %s \n", hivesStr.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "Genome Resultant is not valid in ObjId: %s \n", hivesStr.ptr());
        return false;
    }

    randseqFlags = randseq.getFlags ();

    bool parseDicValidation = sFilterseq::parseDicBioseq(randseq.idlist, qry, &err);
    if( !parseDicValidation ) {
        logOut(eQPLogType_Error, "%s\n", err.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
        return 1;
    }

    if( randseqFlags & sRandomSeq::eSeqParseCNVFile) {
        sStr outPath, newOutputfile;
        sf.addFilePathname(outPath, true, "genome.fasta");
        sf.addFilePathname(newOutputfile, true, "genome.vioseq2");

        bool option = 1;
        bool success = generateFastaFile (newOutputfile.ptr(), outPath.ptr(), option, err);
        if( !success) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }

        qry.empty();
        qry.parse(newOutputfile.ptr(), sBioseq::eBioModeLong, 1);
        if (!qry.dim()){
            logOut(eQPLogType_Error, "CNV Genome Resultant is empty \n");
            reqSetInfo(req, eQPInfoLevel_Error, "CNV Genome Resultant is empty \n");
            return false;

        }

    }

    if( randseqFlags & sRandomSeq::eSeqPrintRecombinantsOnly ) {
        reqProgress(0, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 1;
    }


    if( randseqFlags & sRandomSeq::eSeqParseMutationFile ) {
        sUsrFile obj2(mutFileId, user);
        if( obj2.Id() ) {
            sStr flnm2;
            obj2.getFile(flnm2);
            if( !sFile::exists(flnm2) ) {
                logOut(eQPLogType_Error, "No data file for service ID %s; terminating\n", mutFileId.print());
                reqSetInfo(req, eQPInfoLevel_Error, "No data file for service ID %s", mutFileId.print());
                return 1;
            }
            sFil tblFile(flnm2, sMex::fReadonly);
            if( !randseq.readTableMutations(flnm2, &qry, &err) ) {
                logOut(eQPLogType_Error, "%s\n", err.ptr());
                reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
                return 1;
            }
            randseqFlags = randseq.getFlags ();
        } else if( (randseqFlags & sRandomSeq::eSeqGenerateRandomMut) ) {
            logOut(eQPLogType_Error, "A table is necessary for preloaded table\n");
            reqSetInfo(req, eQPInfoLevel_Error, "A table is necessary for preloaded table");
            return 1;
        }
    }


    if( randseqFlags & sRandomSeq::eSeqParseAnnotFile ) {
        if( !parseAnnotation(qry, err) ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }
    }

    randseq.mutationFillandFix(qry);

    if( (randseqFlags & sRandomSeq::eSeqParseMutationFile) && (randseqFlags & sRandomSeq::eSeqGenerateRandomMut) ) {
        bool success = randseq.generateRandomMutations(qry, err);
        if( !success ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }
    }

    if( (randseqFlags & sRandomSeq::eSeqPrintRandomReads) && (randseqFlags & sRandomSeq::eSeqPrintPairedEnd) ) {
        sStr outReadsPath1;
        sf.addFilePathname(outReadsPath1, true, "shortReads_1.%s", (randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
        sFil readsFile1(outReadsPath1);
        if( !readsFile1.ok() ) {
            logOut(eQPLogType_Error, "failed to create destination shortReads_1 file \n");
            reqSetInfo(req, eQPInfoLevel_Error, "failed to create destination shortReads_1 file");
            return 1;
        }
        sStr outReadsPath2;
        sf.addFilePathname(outReadsPath2, true, "shortReads_2.%s", (randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
        sFil readsFile2(outReadsPath2);
        if( !readsFile2.ok() ) {
            logOut(eQPLogType_Error, "failed to create destination shortReads_2 file \n");
            reqSetInfo(req, eQPInfoLevel_Error, "failed to create destination shortReads_2 file");
            return 1;
        }
        readsFile1.empty();
        readsFile2.empty();
        sStr path;
        reqAddFile(path, "cumullen.bin");

        idx numReads = randseq.randomize(&readsFile1, qry, err, &readsFile2, lazyLenPath.ptr(0));

        if( !numReads || (readsFile1.length() == 0) || (readsFile2.length() == 0) ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }
    } else {
        sStr outReadsPath;
        sf.addFilePathname(outReadsPath, true, "shortReads.%s", (randseqFlags & sRandomSeq::eSeqFastA) ? "fasta" : "fastq");
        sFil readsFile(outReadsPath);
        if( !readsFile.ok() ) {
            logOut(eQPLogType_Error, "failed to create destination shortReads file \n");
            reqSetInfo(req, eQPInfoLevel_Error, "failed to create destination shortReads file");
            return 1;
        }
        readsFile.empty();
        idx numReads = randseq.randomize(&readsFile, qry, err, 0, lazyLenPath.ptr(0));

        if( !numReads || readsFile.length() == 0 ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }

    }

    if( randseqFlags & sRandomSeq::eSeqMutations ) {
        sStr csvTableOutput;
        sf.addFilePathname(csvTableOutput, true, "mutationTable.csv");
        sFil csvFile(csvTableOutput);

        if( !csvFile.ok() ) {
            logOut(eQPLogType_Error, "failed to create destination mutation Table \n");
            reqSetInfo(req, eQPInfoLevel_Error, "failed to create destination mutation Table");
            return 1;
        }
        csvFile.empty();
        bool success = randseq.mutationCSVoutput(&csvFile, qry, err);
        if( !success ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }

        sStr vcfTableOutput;
        sf.addFilePathname(vcfTableOutput, true, "mutationTable.vcf");
        sFil vcfFile(vcfTableOutput);

        if( !vcfFile.ok() ) {
            logOut(eQPLogType_Error, "failed to create destination mutation Table \n");
            reqSetInfo(req, eQPInfoLevel_Error, "failed to create destination mutation Table");
            return 1;
        }
        vcfFile.empty();
        success = randseq.mutationVCFoutput(&vcfFile, objs[0].IdStr(), qry, err);
        if( !success ) {
            logOut(eQPLogType_Error, "%s\n", err.ptr());
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            return 1;
        }
    }



    reqProgress(0, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);
    return 1;
}

int main(int argc, const char *argv[], const char *envp[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    DnaInsilicoProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-insilico", argv[0]));
    return (int) backend.run(argc, argv);

}
