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
#include <slib/utils.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <ssci/bio/biogencode.hpp>
#define isSameBase(baseS,baseQ) (baseS!='N'&&baseQ!='N'&&baseS==baseQ)
class DnaAlignQC: public sQPrideProc
{
    public:

        DnaAlignQC(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
        struct posdata {
            idx matchdata[2]; //mismatch, match
            idx insertion;
            idx deletion[3]; //1 2 3+ continuous deletion
            idx count;
        };
        
};

idx DnaAlignQC::OnExecute(idx req)
{
    sHiveId objID;
    objID = objs[0].Id();
    sUsrObj obj(*user, objID);

    if( !obj.Id() ) {
        logOut(eQPLogType_Info, "Object %s not found or access denied", objID.print());
        reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", objID.print());
        return 1;
    } else {
        logOut(eQPLogType_Info, "processing object %s\n", objID.print());
    }

    sVec<sHiveId> alignerIDs;
    sHiveId::parseRangeSet(alignerIDs, formValue("alignQC_objid"));
//    sHiveseq reads(user, objids[0].print(), sBioseq::eBioModeShort);

    // Get the alignment object
    sStr pathAl, buffer;
    sUsrFile aligner(alignerIDs[0], user);
    aligner.getFilePathname00(pathAl, "alignment.hiveal"_"alignment.vioal"__);

    sHiveal hiveal(user, pathAl);

    sStr subjectStr;
    aligner.propGet00("subject", &subjectStr, ";");
    sHiveseq Sub(user, subjectStr.ptr(), hiveal.getSubMode());//Sub.reindex();

    sStr queryStr;
    aligner.propGet00("query", &queryStr, ";");
    sHiveseq Qry(user, queryStr.ptr(), hiveal.getQryMode());//Sub.reindex();

    sBioal * bioal = &hiveal;

    sStr error;
    sStr out, *outP;
    outP = &out;
//    sHiveseq Sub(user, formValue("subject"), hiveal.getSubMode());        //Sub.reindex();
//    sHiveseq Qry(user, formValue("query"), hiveal.getQryMode());        //Qry.reindex();

    if( !Sub.dim() ) {
        error.addString("alignment subject invalid or not accessible");
    }
    if( !Qry.dim() ) {
        error.addString("alignment query invalid or not accessible");
    }
    bioal->Sub = &Sub;
    bioal->Qry = &Qry;
    sBioal::ParamsAlignmentIterator par(outP);
    par.navigatorFlags = sBioal::alPrintSubject | sBioal::alPrintInRandom | sBioal::alPrintSequenceOnly | sBioal::alPrintIgnoreCaseMissmatches | sBioal::alPrintExcludeDeletions | sBioal::alPrintUpperCaseOnly;

    idx numAligns = hiveal.dimAl();

    //here we need the maxlen of alignments and maxlen of offsets and tails

    sVec<idx> lefttail (sMex::fSetZero);
    sVec<idx> righttail (sMex::fSetZero);

    sVec <posdata> dataContainer (sMex::fSetZero);  // all position data

    sStr outInfo;
    for(idx iter = 0; iter < numAligns; iter++) {

        if (iter % 5000 == 0){
            if( !reqProgress(iter, iter, numAligns) ) {
                logOut(eQPLogType_Error, "Computation will stop\n");
                reqSetInfo(req, eQPInfoLevel_Error, "Computation will stop\n");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
        }

        sBioseqAlignment::Al * hdr = hiveal.getAl(iter);
        idx idSub = hdr->idSub();
        idx idQry = hdr->idQry();
        const char * sub= idSub>=0?bioal->Sub->seq(idSub):0;
        const char * seqSubQua = idSub>=0?bioal->Sub->qua(idSub):0;
        const char * qry=bioal->Qry->seq(idQry);
        const char * seqQryQua = bioal->Qry->qua(idQry);
        bool isSubQuaBit = idSub>=0?bioal->Sub->getQuaBit(idSub):0;
        bool isQryQuaBit = bioal->Qry->getQuaBit(idQry);
        idx lenAlign = hdr->lenAlign();         //alignment length
        idx qrybuflen=bioal->Qry->len(idQry);   //query length
        idx qryrpt = bioal->Qry->rpt(idQry);  //repeat

        dataContainer.resize(qrybuflen+1);  //starts from 0 to qrybuflen
        dataContainer[qrybuflen].count++;
        idx flags= hdr->flags();

        sStr l1;
        idx * m = hiveal.getMatch(iter);
        idx nonCompFlags=(flags)&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement));
        sVec < idx > uncompressMM;
        if(flags&sBioseqAlignment::fAlignCompressed){ // deal with compressed alignments
            uncompressMM.resize(2*lenAlign);
            sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
            m = uncompressMM.ptr();
        }

        sStr aux;
        idx qryOffset = 0;  //offset tail

        // Calculate the offset
        bool printReverseComp = (flags & sBioseqAlignment::fAlignBackward) && ((flags) & sBioseqAlignment::fAlignReverseEngine) == 0 ? true : false;
        if (!printReverseComp){
            qryOffset = hdr->qryStart()+m[1];
        }
        else {
            qryOffset = bioal->Qry->len(idQry) - m[2 * hdr->lenAlign() - 1] - 1;
        }


        idx tailflag = 0;  // mark the last query position

        ::printf("% "DEC, iter);

        idx qrypos = 0;

        for(idx j=0;j<2*lenAlign; j+=2 ) {
            idx is = m[j]; // Get the Sub
            idx iq = m[j + 1]; // Get the query
            if (iq >= 0) qrypos = iq+1-qryOffset;  //Get query position

            if(iq+1 > tailflag) tailflag = iq+1;
            char chQ='N', chS='N';

            idx isx=(hdr->subStart()+is);
            if (is >= 0){
                chS=( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine) ) ? sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(sub, isx, nonCompFlags)]] : sBioseq::mapRevATGC[(idx)sBioseqAlignment::_seqBits(sub, isx,nonCompFlags)] ;
                if( seqSubQua && !sBioseq::Qua(seqSubQua ,isx ,isSubQuaBit) ){
                    chS = 'N';
                }
            }
            else {
                chS = '-';
            }
            // Get the Query letter
            if( iq >= 0 ) {
                idx posQry = hdr->qryStart() + iq;

                idx iqx = ((flags & sBioseqAlignment::fAlignBackward) && ((flags) & sBioseqAlignment::fAlignReverseEngine) == 0) ? (qrybuflen - 1 - (posQry)) : (posQry);
                char chQf = sBioseq::mapRevATGC[(idx) sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)];
                char chQr = sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx) sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)]];
                chQ = ((flags & sBioseqAlignment::fAlignBackward) && ((flags) & sBioseqAlignment::fAlignReverseEngine) == 0) ? chQr : chQf;
                if( seqQryQua && !sBioseq::Qua(seqQryQua ,iqx ,isQryQuaBit) ){
                    chQ = 'N';
                }
            }


            if (is < 0){
                dataContainer[qrypos].insertion += qryrpt;
            }
            else if (iq < 0){
                if (m[j+3] > 0){
                    idx k;
                    for (k = 1; k <= 4; k++){
                        if (j+1-2*k < 0 || m[j+1-2*k] > 0) break;
                    }
                    dataContainer[qrypos].deletion[k-1] += qryrpt;
                }

            }
            else{
                if (!isSameBase(chS, chQ)){
                    dataContainer[qrypos].matchdata[0] += qryrpt; //mismatch
                }
                else{
                    dataContainer[qrypos].matchdata[1] += qryrpt;  //match
                }
            }


//            // Check if there is a deletion
//            if( iq < 0 ) {
//                // Do not print anything, as we are not interested in query
//                if( icounter ) {
//                    // Report the insertion information
//                    outInfo.printf("%"DEC",%"DEC",ins:%s\n", is, is, aux.ptr(1));
//                    aux.cut(0);
//                }
//                if( !dcounter ) {
//                    aux.printf(",");
//                }
//                aux.printf("%c", chS);
//                icounter = 0;
//                ++dcounter;
//            }
//            // Check if there is an insertion
//            else if( is < 0 ) {
//                // print the insertion
//                if( dcounter ) {
//                    // Report the deletion information
//                    outInfo.printf("%"DEC",%"DEC",del:%s\n", is - dcounter, is - 1, aux.ptr(1));
//                    aux.cut(0);
//                }
//                dcounter = 0;
//                l1.addString("-", 1);
//                if( !icounter ) {
//                    aux.printf(",");
//                }
//                aux.printf("%c", chQ);
//                ++icounter;
//            }
//            else {
//                if( icounter ) {
//                    // Report the insertion information
//                    outInfo.printf("%"DEC",%"DEC",ins:%s\n", is, is, aux.ptr(1));
//                    aux.cut(0);
//                    icounter = 0;
//                }
//                if( dcounter ) {
//                    // Report the deletion information
//                    outInfo.printf("%"DEC",%"DEC",del:%s\n", is - dcounter, is - 1, aux.ptr(1));
//                    aux.cut(0);
//                    dcounter = 0;
//                }
//                // print the correct base
//                bool missmatch = (iq < 0 || (!isSameBase(chS, chQ) && chS != 'N'));
//                if( missmatch ) {
//                    outInfo.printf("%"DEC",%"DEC",mut:%c=>%c\n", is, is, chQ, chS);
//                    aux.cut(0);
//                }
//                l1.printf("%c", missmatch ? tolower(chS) : toupper(chS));
//            }


        }

        lefttail.resize(qryOffset+1);  //from 0 to qryOffset
        lefttail[qryOffset] += qryrpt;
        righttail.resize(qrybuflen - tailflag+1);
        righttail[qrybuflen - tailflag] += qryrpt;

//        ::printf("% "DEC, iter);
//        ::printf("%s", outInfo.ptr(0));
    }

    //collect count
    idx datasize = dataContainer.dim();
    for (idx i = 2; i < datasize; i++){
        if (dataContainer[i].count > 0){
            for( idx j = 1; j < i; j++){
                dataContainer[j].count += dataContainer[i].count;
            }
        }
    }



    //here to create file and print results, using dim()

    idx leftsize = lefttail.dim();
    idx rightsize = righttail.dim();

    sStr filename1, filename2, filename3, filename4, filename5; // get the full path one of the output files demo.sumLetterTable.csv
    // match-mismatch, insertion, deletion, left tail, right tail
    sQPrideProc::reqAddFile(filename1, "dna-alignQC-match.csv");
    sQPrideProc::reqAddFile(filename2, "dna-alignQC-insert-delet.csv");
    sQPrideProc::reqAddFile(filename3, "dna-alignQC-deletion.csv");
    sQPrideProc::reqAddFile(filename4, "dna-alignQC-lefttail.csv");
    sQPrideProc::reqAddFile(filename5, "dna-alignQC-righttail.csv");

    if (!filename1 || !filename2 || !filename3 || !filename4 || !filename5){
        // report an error
        logOut(eQPLogType_Error, "Can't open results file\n");
        reqSetInfo(req, eQPInfoLevel_Error, "Can't open results file\n");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sFil outfile1(filename1); // make a file object ... where we can print
    sFil outfile2(filename2);
    sFil outfile3(filename3);
    sFil outfile4(filename4);
    sFil outfile5(filename5);
    outfile1.empty();
    outfile2.empty();
    outfile3.empty();
    outfile4.empty();
    outfile5.empty();

    outfile1.printf("Query Position,Mismatch,NormalizedMismatch");
    outfile2.printf("Query Position,Insertion,Deletion1,Deletion2,Deletion3+");
//    outfile3.printf("Query Position,Deletion1,Deletion2,Deletion3+");
    for (idx i = 0; i < datasize; i++){
        if (i!=0){
            outfile1.printf("\n%"DEC",%"DEC, i, dataContainer[i].matchdata[0]);
            outfile1.printf(",%.4f", (float)(dataContainer[i].matchdata[0]*100)/dataContainer[i].count);
        }
        outfile2.printf("\n%"DEC",%"DEC, i, dataContainer[i].insertion);
        for (idx j = 0; j < 3; j++){
            outfile2.printf(",%"DEC, dataContainer[i].deletion[j]);
        }
//        outfile3.printf("\n%"DEC,i);
//        for (idx j = 0; j < 3; j++){
//            outfile3.printf(",%"DEC, dataContainer[i].deletion[j]);
//        }
    }
    outfile4.printf("Left Tail Length,Frequency");
    for (idx i = 0; i < leftsize; i++){
        outfile4.printf("\n%"DEC",%"DEC, i, lefttail[i]);
    }
    outfile5.printf("Right Tail Length,Frequency");
    for (idx i = 0; i < rightsize; i++){
        outfile5.printf("\n%"DEC",%"DEC, i, righttail[i]);
    }

    reqProgress(numAligns, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);

    return 0;

}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future

    DnaAlignQC backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "dna-alignQC", argv[0]));
    return (int) backend.run(argc, argv);
}
