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
#include <math.h>
#include <violin/violin.hpp>
#include <qpsvc/dna-qc.hpp>

#include "dna-qc.hpp"

#include <slib/utils.hpp>
#include <ssci/bio.hpp>
#include <ssci/bio/biogencode.hpp>

#define numAllCdTbl 19      //number of all codon tables
#define numUniqCdTbl 7      //number of unique tables

 /*! DNA-QC
  *  It reads a Query Input in a vioseq2 format with short reads or genomes
  *  and performs quality control statistics of A,C,G,T's and their Phred Scores
  */
idx DnaQCProc::TbGroup[]= {
    0,1,2,2,2,3,2,2,0,0,2,4,5,2,6,0,2,2,0
};


void DnaQCProc::initStopCodon(){
    idx listOfUniqCdTbl[numUniqCdTbl];
    idx numUniqTable = 0;

    sBioGenCode GenCodeData;

    for (idx i = 0; i < numAllCdTbl; i++){
        bool repeat = false;
        for (idx j = 0; j < i; j++){
            if (TbGroup[i] == TbGroup[j]){
                repeat = true;
                break;
            }
        }
        if (!repeat){
            listOfUniqCdTbl[numUniqTable++] = i;
        }
    }

    stopBase.resize(numUniqTable);
    for(idx TbId = 0; TbId < numUniqCdTbl; TbId++) {

        stopCODONS = GenCodeData.AACodons[20][listOfUniqCdTbl[TbId]];

//        numStop[TbId] = sString::cnt00(stopCODONS);

        stopBase[TbId].resize(sString::cnt00(stopCODONS)*3);
        char *codon = sString::nonconst(stopCODONS);
        for(idx icodon = 0; icodon < stopBase[TbId].dim(); icodon += 3, codon = sString::next00(codon)) {
            for(idx i = 0; i < 3; i++) {
                stopBase[TbId][icodon + i] = sBioseq::mapATGC[(idx)*(codon + i)];
            }

        }
    }

}

void DnaQCProc::getCodonQC(sVec<idx> &countHas, const char* str, idx seqLen, idx rpt){
    sVec<idx> countUniqHas (sMex::fSetZero);
    countUniqHas.resize(numUniqCdTbl);

    idx cod[3];
    idx charlength = seqLen / 4, rem = seqLen % 4;
    if( rem != 0 ){
        charlength++;
    }

    idx thisReadData[numUniqCdTbl][6];
    for(idx i = 0; i < numUniqCdTbl; i++){   //reset to -1
        for(idx j = 0; j < 6; j++){
            thisReadData[i][j] = -1;
        }
    }
    bool thisReadResult[numUniqCdTbl];

    idx charpos1 = 0, basepos1 = 0;
    for (idx j = 0; j < 3 && j < seqLen - 2; j++) {
        idx findcount = 0;      //count the find in the two frames
        idx charpos2 = charpos1, basepos2 = basepos1++;

        while( charpos2 * 4 + basepos2 < seqLen - 2 ) {

            for(idx i = 0; i < 3; i++) {        //get three bases
                cod[i] = getBase(charpos2, basepos2, str);
                basepos2++;
                if( basepos2 == 4 ) {
                    charpos2++;
                    basepos2 = 0;
                }
            }

            for (idx i = 0; i < numUniqCdTbl; i++){         //record
                if(thisReadData[i][j]==-1 && stopCodon(cod[0], cod[1] ,cod[2], i)) {
                    thisReadData[i][j] = charpos2*4 + basepos2 - 2;
                    findcount++;
                }
                if(thisReadData[i][j+3]==-1 && stopCodon(sBioseq::mapComplementATGC[cod[2]], sBioseq::mapComplementATGC[cod[1]], sBioseq::mapComplementATGC[cod[0]], i)) {
                    thisReadData[i][j+3] = charpos2*4 + basepos2;
                    findcount++;
                }
            }
            if (findcount == numUniqCdTbl * 2) {        //two frames finished
                break;
            }

        }

    }

    for (idx i = 0; i < numUniqCdTbl; i++){
        thisReadResult[i] = true;
        for (idx j = 0; j < 6; j++){
            if (thisReadData[i][j] == -1) {
                thisReadResult[i] = false;
                break;
            }
        }
        if (thisReadResult[i]) {
            countUniqHas[i] += rpt;
        }
    }

    for (idx i = 0; i < numAllCdTbl; i++){
        countHas[i] += countUniqHas[TbGroup[i]];
    }

}


/*!
 * executing function of dna-qc
 * Currently, It does not take into consideration any other nucleotide besides: A,C,G and T's
 *
 */
bool DnaQCProc::dnaqcProc(idx req, sHiveId &objId, sStr &errmsg)
{
    sStr path;
    std::auto_ptr < sUsrObj > obj(user->objFactory(objId));
    if( !obj.get() || !obj->Id() ) {
        errmsg.printf("Object %s not found or access denied", obj->IdStr());
    } else {
        obj->getFilePathname00(path, ".hiveseq" _ ".vioseqlist" _ ".vioseq2" __);
    }
    sHiveseq hs(user, path);
    if( !errmsg ) {
        if( hs.dim() == 0 ) {
            logOut(eQPLogType_Error, "query %s\n", obj->IdStr());
            errmsg.printf("Empty data set");
        }
    }
    sBioseq& Qry = hs;

    bool isQual = false;
    idx MAXSEQLEN = 0;
    const idx MAX = 32 * 1024; // max length of sequence QC'ed
    sVec<QCstats> quaStat(sMex::fSetZero);
    sDic<Lenstats> lenStat;
    lenStat.mex()->flags |= sMex::fSetZero;

    quaStat.cut(MAXSEQLEN);
    initLengthStats();

    initStopCodon();
    sVec<idx> countHas (sMex::fSetZero);
    countHas.resize(numAllCdTbl);
    idx complexCnt = 0;

    if( !errmsg ) {
        logOut(eQPLogType_Info, "query %s requested for execution\n", obj->IdStr());

        for(idx ipos = 0; ipos < Qry.dim(); ++ipos) {
            idx realseqlen = Qry.len(ipos);
            idx seqlen = realseqlen;

            // Add seqlen to dictionary
            if( !progressReport(req, ipos, ipos * 70 / Qry.dim()) ) {
                errmsg.printf("Killed");
                break;
            }
            if( seqlen > MAX ) {
                seqlen = MAX;
            }
            if( seqlen > MAXSEQLEN ) {
                MAXSEQLEN = seqlen;
                quaStat.resize(seqlen);
            } else if( seqlen <= 0 ) {
                continue;
            }

            idx seqrpt = Qry.rpt(ipos);
            const char *seqqua = Qry.qua(ipos);

            bool isQbit = Qry.getQuaBit(ipos);  // false means 8 bit qualities, true means 1 bit qualities
            if( seqqua != 0 && *seqqua) {
                isQual = true;
            }
            const char * seq = Qry.seq(ipos);
            if (!seq){
                errmsg.printf("Empty sequence at read: %" DEC, ipos);
                break;
            }
            // Calculate statistics for objId.
            addlengthStat(realseqlen, seqrpt);

            sStr s;
            sBioseq::uncompressATGC(&s, seq, 0, seqlen);
            idx sumqua = 0;

            Lenstats * auxlenStat = lenStat.set(&seqlen, sizeof(seqlen));
            auxlenStat->num += seqrpt;
            // Go into each read and accumulate the values
            for(idx i = 0; i < seqlen; i++) {
                char let = sBioseq::mapATGC[(int) s[i]];
                idx qua = -1;
                if (isQual){
                    qua = (seqqua && !isQbit) ? seqqua[i] : -1;
                    if (qua < 0){
                        qua = -1;
                    }
                }

                Stats_sample(quaStat.ptr(i), let, qua, seqrpt);
                sumqua += qua * seqrpt;
            }
            if( sumqua > 0 ) {
                auxlenStat->sum += (sumqua / seqlen);
            }

            getCodonQC(countHas, seq, seqlen, seqrpt);

            idx passFilter = 0;
            for (idx i = 1; i < 4; i++){
                if (sFilterseq::complexityFilter_wholeSeq_ChunkSize(seq, seqlen, i, 1.8, false)){
                    passFilter++;
                }
                else break;
            }
            if (passFilter == 3){
                complexCnt += seqrpt;
            }
        }
    }
    sVec<idx> ind;
    ind.cut(0);
    ind.add(lenStat.dim());
    sSort::sortCallback(sSort::sort_idxDicComparator, 0, lenStat.dim(), &lenStat, ind.ptr(0));

    if( !errmsg ) {
        path.cut(0);
        obj->addFilePathname(path, true, ".qc2.countsAtPositionTable.csv");
        sFil fp(path);
        if( fp.ok() ) {
            fp.cut(0);
            fp.printf("position,count,quality\n");
            for (idx l = 0; l < lenStat.dim(); ++l){
                idx aux = *(idx *)lenStat.id(ind[l]);
                Lenstats *auxlen = lenStat.ptr(ind[l]);
                real avg =  auxlen->sum / auxlen->num;
                if( avg < .01 ) {
                    fp.printf("%" DEC ",%" DEC ",0\n", aux, auxlen->num);
                } else {
                    fp.printf("%" DEC ",%" DEC ",%.2lf\n", aux, auxlen->num, avg );
                }
                if( !progressReport(req, l, l * 10 / lenStat.dim()) ) {
                    errmsg.printf("Killed by the user");
                    break;
                }
            }
        } else {
            errmsg.printf("Can't write '%s'", path.ptr());
        }
    }
    if( !errmsg ) {
        path.cut(0);
        obj->addFilePathname(path, true, ".qc2.sumLetterTable.csv");
        sFil fp(path);
        if( fp.ok() ) {
            fp.cut(0);
            fp.printf("letter,count,quality\n");
            idx sumCount[4] = { 0, 0, 0, 0 };
            real sumQuality[4] = { 0, 0, 0, 0 };
            for(idx l = 0; l < MAXSEQLEN; ++l) {
                for(idx i = 0; i < 4; ++i) {
                    sumCount[i] += quaStat.ptr(l)->cACGT[i];
                    sumQuality[i] += quaStat.ptr(l)->cqACGT[i];
                }
                if( !progressReport(req, l, l * 10 / MAXSEQLEN) ) {
                    errmsg.printf("Killed");
                    break;
                }
            }
            for(idx let = 0; let < 4; ++let) {
                fp.printf("%c,%" DEC ",%.3lf\n", sBioseq::mapRevATGC[let], sumCount[let], sumCount[let] ? sumQuality[let] / (real) sumCount[let] : 0);
            }
        } else {
            errmsg.printf("Can't write '%s'", path.ptr());
        }
    }
    if( !errmsg ) {
        path.cut(0);
        obj->addFilePathname(path, true, ".qc2.sumPositionTable.csv");
        sFil fp(path);
        if( fp.ok() ) {
            fp.cut(0);
        //  fp3.printf("pos,minA,maxA,meanA,stddevA,countA,minC,maxC,meanC,stddevC,countC,minG,maxG,meanG,stddevG,countG,minT,maxT,meanT,stddevT,countT\n");
            fp.printf("pos,mindA,qualityA,maxdA,countA,mindC,qualityC,maxdC,countC,mindG,qualityG,maxdG,countG,mindT,qualityT,maxdT,countT\n");
            for(idx ipos = 0; ipos < MAXSEQLEN; ++ipos) {
                QCstats *qcstat = quaStat.ptr(ipos);
                fp.printf("%" DEC, ipos);
                for(idx l = 0; l < 4; ++l) {
                    real mean = Stats_mean(qcstat, l);
                    real stdev = Stats_stddev(qcstat, l);
                    if (mean == 0){
                        fp.printf(",0,0,0,%" DEC, qcstat->cACGT[l]);
                    }
                    else {
                        fp.printf(",%.3lf,%.3lf,%.3lf,%" DEC, mean - (stdev / 2.0), mean, mean + (stdev / 2.0), qcstat->cACGT[l]);
                    }

                }
                fp.printf("\n");
                if( !progressReport(req, ipos, ipos * 10 / MAXSEQLEN) ) {
                    errmsg.printf("Killed");
                    break;
                }
            }
        } else {
            errmsg.printf("Can't write '%s'", path.ptr());
        }
    }
    if (!errmsg && Qry.dim() > 0){
        idx prop_ok = 0;
        prop_ok += obj->propSetI("len-min", minLength);
        prop_ok += obj->propSetI("len-max", maxLength);
        prop_ok += obj->propSetI("len-avg", (idx)(sumLength/numLength)+0.5);
        prop_ok += obj->propSetI("bases-count", sumLength);
        if (prop_ok != 4){
            reqSetInfo(req, eQPInfoLevel_Warning, "Failed to assign statistics properties to object Id = %" DEC, objId.objId());
        }
    }

    if( !errmsg ) {
        path.cut(0);
        obj->addFilePathname(path, true, ".qc2.codonQCTable.csv");
        sFil fp(path);
        if( fp.ok() ) {
            fp.cut(0);
            fp.printf("Table Name,Not Coding,Protein Coding");
            sBioGenCode GenCodeData;
            for(idx i = 0; i < numAllCdTbl; i++) {
                fp.printf("\n\"%s\"", GenCodeData.genCodes[i].nm);
                fp.printf(",%" DEC, countHas[i]);
                fp.printf(",%" DEC, Qry.longcount() - countHas[i]);

                if( !progressReport(req, i, i *10 / numAllCdTbl) ) {
                    errmsg.printf("Killed by the user");
                    break;
                }
            }
        } else {
            errmsg.printf("Can't write '%s'", path.ptr());
        }
    }
    if( !errmsg ) {
            path.cut(0);
            obj->addFilePathname(path, true, ".qc2.ComplexityTable.csv");
            sFil fp(path);
            if( fp.ok() ) {
                fp.cut(0);
                fp.printf("Reads,Count");
                fp.printf("\nComplex,%" DEC, complexCnt);
                fp.printf("\nNot Complex,%" DEC, Qry.longcount()-complexCnt);

            } else {
                errmsg.printf("Can't write '%s'", path.ptr());
            }
        }

    return errmsg.length();
}

idx DnaQCProc::OnExecute(idx req)
{
    sStr errmsg, qryBlb, lockBuf;

    // Get the objId from the Form
    const char * qry = formValue("query");
    if( !qry ) {
        // Get the objId from the Blob
        reqGetData(grpId, "query", &qryBlb);
        qry = qryBlb.ptr();
    }
    sHiveId objId(qry && strncmp(qry, "obj://", 6) == 0 ? qry + 6 : qry);

    idx valid = sIdxMax;
    if( objId ) {
        DnaQC dnaqc(*this, objId);
        const char *lockStringkey = dnaqc.getLockString(lockBuf, objId);

        idx reqLockedby;
        reqLock(lockStringkey, &reqLockedby);
        if (req==reqLockedby){
            valid = dnaqcProc(req, objId, errmsg); // Execute the actual dna-qc service
            reqUnlock(lockStringkey);
        }
        else {
            // There is another process taking care of the job
            // Do nothing for now
            valid = 0;
        }
    }
    else {
        errmsg.printf("Invalid objID in %" DEC " request", req);
    }


    if( !errmsg ) {
        reqProgress(0, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    } else {
        logOut(eQPLogType_Error, "%s\n", errmsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    }

    return valid;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future
    DnaQCProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-qc", argv[0]));
    return (int) backend.run(argc, argv);
}
