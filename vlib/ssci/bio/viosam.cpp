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
#include <ctype.h>
#include <ctime>

#include <ssci/bio/viosam.hpp>
#include <ssci/bio/bioseqtree.hpp>
#include <ssci/bio/filterseq.hpp>
#include <ssci/bio/vioalt.hpp>
using namespace slib;


char * sViosam::scanNumUntilEOL(const char * ptr, idx * pVal, const char * lastpos)
{
    idx p = 0;
    while ( (ptr[p]<'0' || ptr[p]>'9') && ptr[p]!='\n' && ptr[p]!='*' && ptr+p<lastpos )
        ++p;
    *pVal=sNotIdx;
    if( ptr+p==lastpos || ptr[p]=='\n' || ptr[p]=='*' )return (char *)(ptr+p+1);

    *pVal=0;
    while ( ptr[p]>='0' && ptr[p]<='9'&& ptr+p<lastpos )
    {
        *pVal=*pVal*10+(*(ptr+p)-'0');
        ++p;
    }
    return (char *) (ptr+p);
}

char * sViosam::skipBlanks(const char * ptr, const char * lastpos)
{
    idx p = 0;
    while ( (ptr[p]==' ' || ptr[p]=='\t') && ptr[p]!='\n' && ptr+p<lastpos )
        ++p;

    return (char *)(ptr+p);
}

char * sViosam::skipUntilEOL(const char * ptr, const char * lastpos)
{
    idx p = 0;
    while ( ptr[p]!='\n' && ptr+p<lastpos )
        ++p;

    if (ptr+p<lastpos) ++p;
    return (char *)(ptr +p);
}

char * sViosam::scanUntilLetterOrSpace(const char * ptr, idx * pVal, const char * lastpos)
{
    idx p = 0;
    while ( (ptr[p]<'0' || ptr[p]>'9') && ptr[p]!='\n' && ptr[p]!='*' && ptr+p<lastpos )
        ++p;
    *pVal=sNotIdx;
    if( ptr+p==lastpos || ptr[p]=='\n' || ptr[p]=='*' || ptr[p]==' ' || ptr[p]=='\t')return (char *)(ptr+p+1);

    *pVal=0;
    while ( ptr[p]>='0' && ptr[p]<='9'&& ptr+p<lastpos )
    {
        *pVal=*pVal*10+(*(ptr+p)-'0');
        ++p;
    }
    return (char *) (ptr+p);
}

char * sViosam::scanAllUntilSpace(const char * ptr, sStr * strVal, const char * lastpos)
{
    idx p = 0;
    while ( ptr[p]!='\n' && ptr+p<lastpos && ((ptr[p]=='\t')||(ptr[p]==' ')) )
        ++p;

    const char * start = ptr + p;

    while (ptr+p<lastpos && ptr[p]!='\n' && ptr[p]!=' ' && ptr[p]!='\t')
        ++p;

    ptr = ptr + p;
    if( ptr > start && strVal) {
        strVal->cut(0);
        strVal->add(start, ptr-start);
        strVal->add0();
    }
    return (char *) ptr;

}

char * sViosam::cigar_parser(const char * ptr, const char * lastpos, sVec<idx> * alignOut, idx * lenalign, idx * qryStart)
{
    sStr full_str;
    *lenalign=0;
    idx shift=0,firstTime=true;
    idx current_qry = 0;
    idx current_sub = 0;

    for( ; strchr("0123456789MIDNSHPX=*+",*ptr) && ptr<lastpos ; ++ptr)
    {
        idx number;
        ptr=scanUntilLetterOrSpace(ptr, &number, lastpos);
        if (*ptr==' ' || *ptr=='\t') {
            break;
        }

        if(*ptr=='N' || *ptr=='H' ) {
            current_sub+=number;
            continue;
        }
        if(*ptr=='S' || *ptr=='H') {
            current_qry+=number;
            continue;
        }

        if( !strchr("MXDI=",*ptr) )
            continue;


        if(firstTime) {
            shift=current_qry;
            firstTime=false;
        }

        idx * addPtr=alignOut->add(2*number);
        for (idx k=0; k<number; ++k)
        {
             addPtr[2*k]=(*ptr=='I') ? -1: current_sub+k-shift ;
             addPtr[2*k+1]=(*ptr=='D') ? -1 : current_qry+k-shift ;
             ++(*lenalign);
        }
        if(*ptr!='I')current_qry += number;
        if(*ptr!='D')current_sub += number;

        }

    if(qryStart)*qryStart=shift;
    return (char *)ptr;
}

idx sViosam::ParseSamFile(char * fileContent, idx filesize, sVec < idx > * alignOut, const char * vioseqFilename, sDic <idx> *rgm )
{
    sBioseq::initModule(sBioseq::eACGT);
    sFilePath baseFileName(vioseqFilename,"%%pathx.baseFile.tmp");

    sVioDB db(vioseqFilename, "vioseq2", (idx)4, (idx)3);
    idx relationlistIDS[1]={2};
    idx relationlistREC[3]={1, 3, 4};
    idx relationlistSEQ[1]={3};
    idx relationlistQUA[1]={4};

    db.AddType(sVioDB::eOther,1, relationlistIDS,"ids", 1);
    db.AddType(sVioDB::eOther,3, relationlistREC,"rec", 2);
    db.AddType(sVioDB::eOther,1, relationlistSEQ,"seq", 3);
    db.AddType(sVioDB::eOther,1,relationlistQUA,"qua", 4);

    sFil baseFile(baseFileName);
    baseFile.cut(0);

    idx res= ParseAlignment(fileContent, filesize, db, baseFile, alignOut, rgm );

    if (res == -1)
    {
        db.deleteAllJobs();
        sFile::remove(baseFileName);
        return 0;
    }

    vioDB.init(vioseqFilename,0,0,0);
    sFile::remove(baseFileName);
    return res;
}

idx sViosam::convertVarScan2OutputintoCSV(const char * varscanFile, const char * csvFileTmplt, sBioseq * Sub, bool skipHeader) {
    sFil varscanFileContent(varscanFile, sMex::fReadonly);
    if( !varscanFileContent )
        return 0;

    const char * varscanFileLastPos = varscanFileContent.ptr() + varscanFileContent.length();


    sStr csvFileRef;
    csvFileRef.printf("%s.csv", csvFileTmplt);

    sFil csvFileContent(csvFileRef);
    csvFileContent.cut(0);
    if (!skipHeader) {
        csvFileContent.printf("Chromosome,Position,Letter,Consensus,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,"
            "Count Forward,Count Reverse,Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T\n");
    }


    const char * buf = varscanFileContent.ptr();

    buf = skipUntilEOL(buf, varscanFileLastPos);

    for(idx iAl = 0; buf < varscanFileLastPos; ++iAl) {



        sStr _refIdStr;
        buf = scanAllUntilSpace(buf, &_refIdStr, varscanFileLastPos);
        if (_refIdStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        sDic<idx> subjectIdDictionary;
        sFilterseq::parseDicBioseq(subjectIdDictionary, *Sub);
        idx refId = sFilterseq::getrownum (subjectIdDictionary, _refIdStr.ptr(), _refIdStr.length());


        sStr refPosStr;
        buf = scanAllUntilSpace(buf, &refPosStr, varscanFileLastPos);
        if (refPosStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        sStr refNucleotideStr;
        buf = scanAllUntilSpace(buf, &refNucleotideStr, varscanFileLastPos);
        if (refNucleotideStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        sStr consensusNucleotideStr;
        buf = scanAllUntilSpace(buf, &consensusNucleotideStr, varscanFileLastPos);
        if (consensusNucleotideStr.length()==0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }



        idx reads1Count = -1;
        buf = scanNumUntilEOL(buf, &reads1Count, varscanFileLastPos);
        if (reads1Count < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }


        idx reads2Count = -1;
        buf = scanNumUntilEOL(buf, &reads2Count, varscanFileLastPos);
        if (reads2Count < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx countACGT[4] = {0, 0, 0, 0};

        idx referenceMap = sBioseq::mapATGC[(idx)refNucleotideStr[0]];
        idx consensusMap = sBioseq::mapATGC[(idx)consensusNucleotideStr[0]];

        idx totalCount = reads1Count + reads2Count;

        if (referenceMap == consensusMap) {
            countACGT[referenceMap] = totalCount;
        } else {
            countACGT[referenceMap] = reads1Count;
            countACGT[consensusMap] = reads2Count;
        }


        sStr _dummy;
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);


        idx quality1 = -1;
        buf = scanNumUntilEOL(buf, &quality1, varscanFileLastPos);
        if (quality1 < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx quality2 = -1;
        buf = scanNumUntilEOL(buf, &quality2, varscanFileLastPos);
        if (quality2 < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx quality = (reads1Count/totalCount * quality1) + (reads2Count/totalCount * quality2);

        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);
        buf = scanAllUntilSpace(buf, &_dummy, varscanFileLastPos);



        idx reads1Plus = -1;
        buf = scanNumUntilEOL(buf, &reads1Plus, varscanFileLastPos);
        if (reads1Plus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx reads1Minus = -1;
        buf = scanNumUntilEOL(buf, &reads1Minus, varscanFileLastPos);
        if (reads1Minus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx reads2Plus = -1;
        buf = scanNumUntilEOL(buf, &reads2Plus, varscanFileLastPos);
        if (reads2Plus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx reads2Minus = -1;
        buf = scanNumUntilEOL(buf, &reads2Minus, varscanFileLastPos);
        if (reads2Minus < 0) {
            buf = skipUntilEOL(buf, varscanFileLastPos);
            continue;
        }

        idx countForward = reads1Plus + reads2Plus;
        idx countReverse = reads1Minus + reads2Minus;

        float freqACGT[4];
        freqACGT[0] = countACGT[0] / totalCount;
        freqACGT[1] = countACGT[1] / totalCount;
        freqACGT[2] = countACGT[2] / totalCount;
        freqACGT[3] = countACGT[3] / totalCount;


        csvFileContent.printf("%" DEC ",%s,%s,%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",0,0,%" DEC ",%" DEC ",%" DEC ",%" DEC ",0,0,%.2lf,%.2lf,%.2lf,%.2lf\n",
            refId,refPosStr.ptr(), refNucleotideStr.ptr(), consensusNucleotideStr.ptr(), countACGT[0], countACGT[1], countACGT[2], countACGT[3], totalCount,
            countForward, countReverse, quality, freqACGT[0], freqACGT[1],freqACGT[2],freqACGT[3]);
    }
    return 1;
}




idx sViosam::convertVCFintoCSV(const char * vcfFile, const char * csvFileTmplt, sBioseq * Sub, bool skipHeader)
{
    sFil vcfFileContent(vcfFile, sMex::fReadonly);
    if( !vcfFileContent )
        return 0;

    const char * lastPos = vcfFileContent.ptr() + vcfFileContent.length();


    sStr csvFileRef;
    csvFileRef.printf("%s.csv", csvFileTmplt);

    sFil csvFileContent(csvFileRef);
    csvFileContent.cut(0);
    if (!skipHeader) {
        csvFileContent.printf("Chromosome,Position,Letter,Consensus,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,"
            "Count Forward,Count Reverse,Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T\n");
    }

    for(idx refNum = 0; refNum < Sub->dim(); ++refNum) {
        const char * buf = vcfFileContent.ptr();


        {

            for(idx iAl = 0; buf < lastPos; ++iAl) {

                if( *buf == '#' ) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr refIdStr;
                buf = scanAllUntilSpace(buf, &refIdStr, lastPos);
                if (refIdStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                idx idSubptr;
                sscanf(refIdStr.ptr(), "%" DEC, &idSubptr);
                if (idSubptr==sNotIdx || idSubptr<0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                if (idSubptr != refNum) continue;

                sStr refPosStr;
                buf = scanAllUntilSpace(buf, &refPosStr, lastPos);
                if (refPosStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr skipStr;
                buf = scanAllUntilSpace(buf, &skipStr, lastPos);
                if (skipStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                sStr refLetterStr;
                buf = scanAllUntilSpace(buf, &refLetterStr, lastPos);
                if (refLetterStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                sStr consensusLettersStr;
                buf = scanAllUntilSpace(buf, &consensusLettersStr, lastPos);
                if (consensusLettersStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                idx totAltLetterCount = 1;
                for (idx is = 0; is < consensusLettersStr.length(); is++) {
                    if (consensusLettersStr[is] == ',') totAltLetterCount++;
                }

                sStr quaStr;
                buf = scanAllUntilSpace(buf, &quaStr, lastPos);
                if (quaStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                skipStr.cut(0);
                buf = scanAllUntilSpace(buf, &skipStr, lastPos);
                if (skipStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr dpStr;
                buf = scanAllUntilSpace(buf, &dpStr, lastPos);
                if (dpStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                char *dps = strstr(dpStr.ptr(), "DP=");
                if (!dps) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                char *dpe = strstr(dps, ";");
                if (!dpe) dpe = strstr(dps, "\n");
                if (!dpe || (dpe-dps-3<1)) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }
                sStr dpvalStr; dpvalStr.resize(dpe-dps-3);
                strncpy(dpvalStr.ptr(), dps+3, dpe-dps-3);
                dpvalStr.add0(dpe-dps-3+1);

                idx dpval; sscanf(dpvalStr.ptr(), "%" DEC, &dpval);
                if (dpval==sNotIdx || dpval<1) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                sStr otherStr;
                buf = scanAllUntilSpace(buf, &otherStr, lastPos);
                if (otherStr.length()==0) {
                    buf = skipUntilEOL(buf, lastPos);
                    continue;
                }

                buf = skipUntilEOL(buf, lastPos);

                char *dp4s = strstr(dpStr.ptr(), "DP4=");
                if( !dp4s ) {
                    continue;
                }
                char *dp4e = strstr(dp4s, ";");
                if( !dp4e )
                    dp4e = strstr(dp4s, "\n");
                if( !dp4e || (dp4e - dp4s - 4 < 1) ) {
                    continue;
                }
                sStr dp4valStr;
                dp4valStr.resize(dp4e - dp4s - 4);
                strncpy(dp4valStr.ptr(), dp4s + 4, dp4e - dp4s - 4);
                dp4valStr.add0(dp4e - dp4s - 4 + 1);
                if (dp4valStr.length()<1) {
                    continue;
                }
                sStr dp4valStrZ;
                sString::searchAndReplaceSymbols(&dp4valStrZ, dp4valStr.ptr(), 0, ",", 0, 0, true, true, false, true);
                sVec <idx> dpIdx(sMex::fSetZero); dpIdx.add(4);
                idx dpCounter = 0;
                for( const char * onedp=dp4valStrZ.ptr(); onedp; onedp=sString::next00(onedp)) {
                    sscanf(onedp, "%" DEC, &(dpIdx[dpCounter]));
                    ++dpCounter;
                }
                idx refCount = dpIdx[0] + dpIdx[1];
                idx altCount = dpIdx[2] + dpIdx[3];
                idx forwardCount = dpIdx[0] + dpIdx[2];
                idx reverseCount = dpIdx[1] + dpIdx[3];
                idx totDP = (forwardCount+reverseCount>0)?(forwardCount+reverseCount):dpval;

                sVec <idx> dpLetCnt(sMex::fSetZero); dpLetCnt.add(4);
                sVec <real> freq(sMex::fSetZero); freq.add(4);

                for (idx letN=0; letN<4; ++letN) {
                    for (idx i=0; i<consensusLettersStr.length(); i=i+2) {
                        if( consensusLettersStr[i] == sBioseq::mapRevATGC[letN]) {
                            dpLetCnt[letN] = altCount / (real)totAltLetterCount;
                            freq[letN] = (real)altCount / (real)totDP / (real)totAltLetterCount;
                        }
                    }
                    if( refLetterStr[0] == sBioseq::mapRevATGC[letN] ) {
                        dpLetCnt[letN] = refCount;
                        freq[letN] = (real)refCount / (real)totDP;
                    }
                }

                if (consensusLettersStr.length()>1) {
                    consensusLettersStr.cut(1);
                    consensusLettersStr.add0(2);
                }

                if (idSubptr == refNum) {

                    csvFileContent.printf("%" DEC ",%s,%s,%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",0,0,%" DEC ",%" DEC ",%" DEC ",%s,0,0,%.2lf,%.2lf,%.2lf,%.2lf\n",
                        refNum,refPosStr.ptr(), refLetterStr.ptr(), consensusLettersStr.ptr(), dpLetCnt[0], dpLetCnt[1], dpLetCnt[2], dpLetCnt[3],
                        totDP, forwardCount, reverseCount, quaStr.ptr(), freq[0],freq[1],freq[2],freq[3]);
                }
            }

        }
    }


    return 1;
}


idx sViosam::ParseAlignment(const char * fileContent, idx filesize, sVioDB &db , sFil & baseFile, sVec < idx > * alignOut, sDic <idx> * rgm)
{
    RecSam rec;
    sVec < RecSam > vofs (sMex::fBlockDoubling);
    idx unique = 0;
    idx uniqueCount = 0;
    idx removeCount = 0;
    sVec < idx > ids, ods;
    ids.cut(0);
    ods.cut(0);
    BioseqTree tree (&baseFile, 0);
    sStr SEQ;
    sStr QUA;
    sStr qty;
    sStr tmp;
    sStr idQry;

    const char * buf=fileContent;
    const char * lastPos=fileContent+filesize;

    idx flag, idSub,subStart, score, cntFound=0, lenalign, qryStart;
    sStr idSubStr;

    bool alignflag;

    for (idx iAl=0; buf<lastPos ; ++iAl ){

        PERF_START ("PARSE LINES");

        if( *buf=='@') {
            buf=skipUntilEOL(buf, lastPos);
            continue;
        }

        buf=scanAllUntilSpace(buf, &idQry , lastPos);
        const char * id = idQry.ptr(0);
        idx idlen = idQry.length();

        buf=scanNumUntilEOL(buf, &flag, lastPos);
        if(flag==sNotIdx)
            continue;
        idx dir_flag = 0;
        if (flag&0x4)  { alignflag = false; }
        else {           alignflag = true;  }

        if (flag&0x10) dir_flag = sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignBackward;
        else           dir_flag = sBioseqAlignment::fAlignForward;

        if (flag&0x100){
            buf=skipUntilEOL(buf, lastPos);
            continue;
        }

        sStr idSubStr2;
        buf=scanAllUntilSpace(buf, &idSubStr2, lastPos);
        idSubStr.cut(0);
        idSubStr.printf(">%s",idSubStr2.ptr());

        if (idSubStr.length()<1)
            continue;

        idx * idSubptr = 0;
        if (rgm != 0){
            idSubptr = rgm->get(idSubStr.ptr());
        }
        if (!idSubptr){
            char *p = strstr(idSubStr.ptr(), "HIVESEQID=");
            if (p){
                sscanf(p+10,"%" DEC "", idSubptr);
            }
            else
                idSubptr = &iAl;
        }
        else
            idSubptr = &iAl;

        idSub = *idSubptr;
        if(idSub==sNotIdx)
            continue;

        buf=scanNumUntilEOL(buf, &subStart, lastPos);
        if(subStart) subStart--;
        if(idSub==sNotIdx)
            continue;

        buf=scanNumUntilEOL(buf, &score, lastPos);
        if(score==sNotIdx)
            continue;

        buf=skipBlanks(buf, lastPos);
        if( *buf=='\n' || buf>=lastPos)
            continue;

        idx ofsThisAl = 0, headerSize;
        if ((alignOut != 0) && (alignflag == true)){
            ofsThisAl=alignOut->dim();
            headerSize = sizeof(sBioseqAlignment::Al)/sizeof(idx);
            alignOut->add(headerSize);
            buf = cigar_parser(buf, lastPos, alignOut, &lenalign,&qryStart);
        }
        else {
            buf=scanAllUntilSpace(buf, 0, lastPos);
        }
        idx len;

        PERF_START("MOVE ONLY");
        buf=scanAllUntilSpace(buf, 0, lastPos);
        buf=scanAllUntilSpace(buf, 0, lastPos);
        buf=scanAllUntilSpace(buf, 0, lastPos);
        PERF_END();
        buf=scanAllUntilSpace(buf, &SEQ, lastPos);
        len = SEQ.length()-1;
        if (strstr (SEQ, "*") || (len == 0)){
            buf=skipUntilEOL(buf, lastPos);
            continue;
        }
        buf=scanAllUntilSpace(buf, &QUA, lastPos);
        PERF_END();

        PERF_START("COMPRESSION");

        const char * seq= SEQ;
        const char * qua= QUA;

        rec.ofsSeq=baseFile.length();
        char * cpy=baseFile.add(0, ( len )/4+1 );
        qty.cut(0);
        qty.resize(len/8 + 1);
        rec.lenSeq=sBioseq::compressATGC(cpy,seq,len, qty.ptr());

        baseFile.cut(rec.ofsSeq+(rec.lenSeq-1)/4+1);

        char *pqu;
        if( rec.lenSeq ) {
            pqu=baseFile.add(0,rec.lenSeq);
            for ( idx iq=0; iq<rec.lenSeq; ++iq){
                pqu[iq]=qua[iq]-33;
            }
        }

        PERF_END();
        PERF_START("TREE INSERT");
        db.AddRecord(eRecID_TYPE,(void *)id, idlen);
        db.AddRecordRelationshipCounter(eRecID_TYPE, 0, 1);

        char *currQua = baseFile.ptr(rec.ofsSeq) + ((rec.lenSeq-1)/4 +1);
        char * Nb=(char *)qty.ptr() ;
        for (idx is = 0; is < rec.lenSeq; is++){
            if (Nb[is/8] != 0){
                if (Nb[is/8] & (0x01<<(is%8))){
                    currQua[is] = 0;
                }
            }
            else is += 7;
        }

        unique = tree.addSequence(rec.ofsSeq, rec.lenSeq);
        if (unique == -1){
            rec.countSeq = 1;
            *vofs.add()=rec;
            uniqueCount ++;
            ids.vadd(1, uniqueCount-1);
            ods.vadd(1, cntFound);
        }
        else {
            RecSam *origRec = vofs.ptr(unique);
            char *origQua = baseFile.ptr( origRec->ofsSeq) + ((origRec->lenSeq-1)/4 +1);
            for (idx iqua = 0; iqua < origRec->lenSeq; iqua++){
                origQua[iqua] = ((origQua[iqua] * origRec->countSeq) + currQua[iqua]) / (origRec->countSeq + 1);
            }

            origRec->countSeq++;
            baseFile.cut(rec.ofsSeq);
            removeCount ++;

            ids.vadd(1, unique);
        }


        PERF_END();

        buf=skipUntilEOL(buf, lastPos);
        if ((alignOut != 0) && (alignflag == true)){
            idx dimAlign=lenalign*2;
            if( dimAlign==0) {
                alignOut->cut(ofsThisAl);
                continue;
            }

            sBioseqAlignment::Al * hdr=(sBioseqAlignment::Al *)alignOut->ptr(ofsThisAl);
            hdr->setIdSub(idSub);
            hdr->setIdQry(atoidx(idQry.ptr()));
            hdr->setScore(score);
            hdr->setFlags(dir_flag);
            hdr->setLenAlign(lenalign);
            hdr->setDimAlign(dimAlign);

            idx * m=hdr->match();

            subStart+=m[0];
            qryStart+=m[1];
            hdr->setSubStart(subStart);
            hdr->setQryStart(qryStart);


            hdr->setDimAlign(sBioseqAlignment::compressAlignment(hdr, m, m));
            alignOut->cut(ofsThisAl+headerSize+hdr->dimAlign());
        }
        ++cntFound;
        }

    PERF_START("WRITING");
    sVec < RecSam > vofsSort (sMex::fBlockDoubling);
    sVec < idx > inSort;
    inSort.cut(0);
    sVec < idx > simSort;
    simSort.cut(0);
    sVec < idx > outSort;
    outSort.resize(uniqueCount);

    tree.inOrderTree3(0, &inSort, &simSort);

    for (idx inx = 0; inx < inSort.dim(); inx++){
        rec = vofs[inSort[inx]];
        {
            idx sim = simSort[inx];
            idx rpt = rec.countSeq;
            if (sim == -1) {sim = 0;}
            rec.countSeq = (sim << 32) | (rpt & 0xFFFF);
        }
        *vofsSort.add()=rec;
        outSort[inSort[inx]] = inx;
        db.AddRecord(eRecREC_TYPE,(void *)&rec, sizeof(RecSam));
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 1);
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 2);
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 3);
        char *cpy=baseFile.ptr(rec.ofsSeq);
        char *pqu=cpy+(rec.lenSeq-1)/4+1;
        db.AddRecord(eRecSEQ_TYPE,(void *)cpy, (rec.lenSeq-1)/4+1);
        db.AddRecord(eRecQUA_TYPE,(void *)pqu, rec.lenSeq);
    }

    db.AllocRelation();
    for (idx i = 0; i < ids.dim(); i++){
        db.AddRelation(eRecID_TYPE, 1, i + 1, outSort[ids[i]]+1 );
    }

    for (idx i = 0; i < uniqueCount; i++){
        db.AddRelation(eRecREC_TYPE, 1, i+1, ods[inSort[i]]+1 );
        db.AddRelation(eRecREC_TYPE, 2 , i+1, i+1 );
        db.AddRelation(eRecREC_TYPE, 3, i+1, i+1 );
    }
    db.Finalize();
    PERF_END();
    PERF_PRINT();
    return vofsSort.dim();
}

idx sViosam::convertVioaltIntoSam(sBioal *bioal, sFil & samHeader, sFil & samFooter, idx subId, sBioseq *Qry, sBioseq *Sub, bool originalrefIds, const char * outputFilename, FILE * fstream, void * myCallbackParam, callbackType myCallbackFunction)
{
    const bool samHeaderGood = samHeader.ok() && (samHeader.length() > 0);

    sBioseq::initModule(sBioseq::eACGT);

    sBioal::ParamsAlignmentIterator PA;
    sStr SAMFile;

    if( outputFilename ) {
        sFile::remove(outputFilename);
        SAMFile.init(outputFilename);
        PA.str = &SAMFile;
    } else {
        PA.str = &SAMFile;
        PA.outF = fstream;
    }

    bioal->progress_CallbackFunction = myCallbackFunction;
    bioal->progress_CallbackParam = myCallbackParam;

    if( Sub ) {
        bioal->Sub = Sub;
    } else {
        Sub = bioal->Sub;
    }
    if( Qry ) {
        bioal->Qry = Qry;
    } else {
        Qry = bioal->Qry;
    }

    printHeaderHD(SAMFile);

    if (samHeaderGood) {
        const char * fp = samHeader.ptr(0);
        const char * end = samHeader.last();
        while ( *fp && fp < end) {
            const char * lp = fp;
            char lc;
            while ( (lc = *lp) && lc != '\n' && lp < end) {
                ++lp;
            }
            if (strncmp(fp, "@HD\t", 4)) {
                SAMFile.add(fp, lp - fp);
                SAMFile.add("\n", 1);
            }
            fp = lp;
            if (*fp && fp < end) {
                ++fp;
            }
        }
        SAMFile.add0cut();
    }

    sVec<idx> subjectAligns;

    if( subId == -1 ) {
        for(idx i = 0; i < Sub->dim(); ++i) {
            if ( !samHeaderGood )
                printHeaderSQ(SAMFile, *Sub, i, originalrefIds);
            
            subjectAligns.vadd(1, i);

            if( myCallbackFunction && myCallbackFunction(myCallbackParam, i, -1, 100) == 0 ) {
                return 0;
            }

        }

        PA.navigatorFlags = sBioal::alPrintCollapseRpt;


        sStr subIDStr;

        for(idx iter = 0; iter < subjectAligns.dim(); ++iter) {
            idx igenome = subjectAligns[iter];
            subIDStr.printf(0, "%s", Sub->id(igenome));
            idx idstart = 0;
            if( *subIDStr.ptr(0) == '>' ) {
                ++idstart;
            }

            idx start = 0;
            
            if ( !samHeaderGood )
                printHeaderSQ(SAMFile, *Sub, igenome, originalrefIds);

            if (originalrefIds){
                PA.userPointer = 0;
            }
            else{
                PA.userPointer = &subIDStr;
            }

            bioal->iterateAlignments(0, start, 0, igenome, (sBioal::typeCallbackIteratorFunction) &vioaltIteratorFunction, (sBioal::ParamsAlignmentIterator *) &PA);

            if( myCallbackFunction && myCallbackFunction(myCallbackParam, iter, -1, 100) == 0 ) {
                return 0;
            }
        }

    } else {

        PA.navigatorFlags = sBioal::alPrintCollapseRpt;
        sStr subIDStr("%s", Sub->id(subId));
        idx idstart = 0;
        if( *subIDStr.ptr(0) == '>' ) {
            ++idstart;
        }
        idx len = Sub->len(subId);

        if( originalrefIds ) {
            sStr subIDStrNoSpace;
            sString::copyUntil(&subIDStrNoSpace, subIDStr.ptr(idstart), 0, " ");
            if (!samHeaderGood) {
                SAMFile.printf("@SQ\tSN:%s\tLN:%" DEC "\n", subIDStrNoSpace.ptr(), len);
            }
            subIDStr.printf(0, "%s", subIDStrNoSpace.ptr());
        } else {
            if (!samHeaderGood) {
                SAMFile.printf("@SQ\tSN:%" DEC "\tLN:%" DEC "\n", subId + 1, len);
            }
            subIDStr.printf(0, "%" DEC, subId + 1);
        }

        idx start = 0;
        if (originalrefIds){
            PA.userPointer = 0;
        }
        else {
            PA.userPointer = &subIDStr;
        }

        bioal->iterateAlignments(0, start, 0, subId, (sBioal::typeCallbackIteratorFunction) &vioaltIteratorFunction, (sBioal::ParamsAlignmentIterator *) &PA);

        if( myCallbackFunction && myCallbackFunction(myCallbackParam, bioal->dimAl(), -1, 100) == 0 ) {
            return 0;
        }
    }
    if( samFooter.ok() && samFooter.length() > 0 ) {
        if( outputFilename ) {
            SAMFile.add(samFooter.ptr(), samFooter.length());
        } else if( PA.outF ) {
            fwrite(samFooter.ptr(), samFooter.length(), 1, PA.outF);
        }
    }
    return 1;
}

void sViosam::convertDIProfIntoSam(sBioal *bioal, bool originalrefIds, const char * outputFilename, FILE * fstream, sVec<idx> * readsArray) 
{
    sBioseq::initModule(sBioseq::eACGT);

    sBioal::ParamsAlignmentIterator PA;
    sStr SAMFile;

    if( outputFilename ) {
        sFile::remove(outputFilename);
        SAMFile.init(outputFilename);
        PA.str = &SAMFile;
    } else {
        PA.str = &SAMFile;
        PA.outF = fstream;
    }

    SAMFile.printf("@HD\tVN:1.0\tSO:unsorted\n");

    sVec<idx> subjectAligns;
    for(idx i = 0; i < bioal->Sub->dim(); ++i) 
    {
        idx len = bioal->Sub->len(i);

        if( originalrefIds ) 
        {
            sStr subIDStrNoSpace;
            sString::copyUntil(&subIDStrNoSpace, bioal->Sub->id(i), 0, " ");
            SAMFile.printf("@SQ\tSN:%s\tLN:%" DEC "\n", subIDStrNoSpace.ptr(), len);
        } else {
            SAMFile.printf("@SQ\tSN:%" DEC "\tLN:%" DEC "\n", i + 1, len);
        }

        subjectAligns.vadd(1, i);
    }

    PA.navigatorFlags = sBioal::alPrintCollapseRpt;


    bioal->iterateAlignments(0, 0, 0, -2, (sBioal::typeCallbackIteratorFunction) &sViosam::vioaltIteratorFunction, (sBioal::ParamsAlignmentIterator *) &PA, 0, 0, readsArray);
}

idx sViosam::vioaltIteratorFunction(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAl)
{

    static sStr queryATGCsequence;
    static sStr qualityATGCsequence;
    static sStr subIDStrNoSpace;
    static sStr qryIDStrNoSpace;
    queryATGCsequence.cut(0);
    qualityATGCsequence.cut(0);
    sStr CIGAR;
    CIGAR.cut(0);

    const bool quaBit = bioal->Qry->getQuaBit(hdr->idQry());

    idx samContentSize = 0;
    const char * samContent = bioal->getSAMContent(iAl, &samContentSize);
    if( samContent && samContentSize ) {
        const char * p_flags = sString::skipWords(samContent, samContentSize, 1, "\t");
        const char * p_opt_fld = sString::skipWords(samContent, samContentSize, 9, "\t");
        if( p_flags ) {
            const idx flag = atoi(p_flags);
            const sBioseq::ESeqDirection direction = flag & 16 ? sBioseq::eSeqReverseComplement : sBioseq::eSeqForward;
            param->str->add(samContent, p_opt_fld ? p_opt_fld - samContent : samContentSize);
            bioal->Qry->printSequence(param->str, hdr->idQry(), 0, 0, direction, true);
            if( !p_opt_fld || (p_opt_fld && p_opt_fld[0] != '*') ) {
                param->str->add("\t", 1);
                sRC rc = !quaBit ? bioal->Qry->printQualities(*(param->str), hdr->idQry(), 0, direction & sBioseq::eSeqReverseComplement) : RC(sRC::eReading, sRC::eString, sRC::ePointer, sRC::eNull);
                if( rc != sRC::zero ) {
                    param->str->add("*", 1);
                }
            }
            if( p_opt_fld && (p_opt_fld - samContent) > 0 ) {
                param->str->add("\t", 1);
                param->str->add(p_opt_fld, samContentSize - (p_opt_fld - samContent));
            }
            param->str->add("\n", 1);
            param->str->add0cut();
            return 1;
        }
    }

    m = sBioseqAlignment::uncompressAlignment(hdr, m);

    idx samflags = 0;

    if( hdr->isReverseComplement()){
        samflags |= eSamRevComp;
    }


    idx mcounter = 0;
    idx dcounter = 0;
    idx icounter = 0;

    idx subAlign;
    idx queryAlign;

        for(idx i = 0; i < hdr->lenAlign() ; i ++) {

            subAlign = sBioseqAlignment::Al::getSubjectIndex(m,i);
            queryAlign = sBioseqAlignment::Al::getQueryIndex(m,i);

            if( subAlign == -1 ) {
                if( dcounter ) {
                    CIGAR.printf("%" DEC "D", dcounter);
                    dcounter = 0;
                }
                if( mcounter ) {
                    CIGAR.printf("%" DEC "M", mcounter);
                    mcounter = 0;
                }

                icounter++;
            }
            else if( queryAlign == -1 ) {
                if( icounter ) {
                    CIGAR.printf("%" DEC "I", icounter);
                    icounter = 0;
                }
                if( mcounter ) {
                    CIGAR.printf("%" DEC "M", mcounter);
                    mcounter = 0;
                }

                dcounter++;
            }
            else {
                if( icounter ) {
                    CIGAR.printf("%" DEC "I", icounter);
                    icounter = 0;
                }
                if( dcounter ) {
                    CIGAR.printf("%" DEC "D", dcounter);
                    dcounter = 0;
                }

                mcounter++;
            }
        }


    if( icounter )
        CIGAR.printf("%" DEC "I", icounter);
    if( dcounter )
        CIGAR.printf("%" DEC "D", dcounter);
    if( mcounter )
        CIGAR.printf("%" DEC "M", mcounter);

    const char * qry = bioal->Qry->seq(hdr->idQry());
    idx qrybuflen=bioal->Qry->len(hdr->idQry()), iqx = 0;

    for(idx i = 0; i < hdr->lenAlign() ; i++) {
        iqx = hdr->getQueryPosition(m,i,qrybuflen);
        if( iqx >= 0 ) {
            char chQ = hdr->getQueryCharByPosition(iqx,qry);
            queryATGCsequence.add(&chQ, 1);
        }
    }
    const char * seqqua = bioal->Qry->qua(hdr->idQry());
    if (seqqua && !quaBit) {
        for(idx i = 0; i < hdr->lenAlign(); i++) {
            iqx = hdr->getQueryPosition(m,i, qrybuflen);
            if( iqx >= 0 ) {
                char chQual = seqqua[iqx] + 33;
                qualityATGCsequence.add(&chQual, 1);
            }
        }
    }
    else {
        qualityATGCsequence.addString("*",1);
    }


    idx rptCnt = bioal->Qry->rpt(hdr->idQry());

    if( param->userPointer ){
        for(idx ii=0; ii<rptCnt; ii++){
            param->str->addNum(hdr->idQry());
            if (rptCnt > 1){
                param->str->add("_",1);
                param->str->addNum(ii+1);
            }
            param->str->add("\t",1);
            param->str->addNum(samflags);
            param->str->add("\t",1);
            param->str->add(((sStr*) param->userPointer)->ptr(0), ((sStr*) param->userPointer)->length());
            param->str->add("\t",1);
            param->str->addNum(hdr->getSubjectStart(m) + 1);
            param->str->add("\t",1);
            param->str->addNum(hdr->score());
            param->str->add("\t",1);
            param->str->add(CIGAR.ptr(), CIGAR.length());
            param->str->add("\t*\t0\t0\t", 7);
            param->str->add(queryATGCsequence.ptr(), queryATGCsequence.length());
            param->str->add("\t",1);
            param->str->add(qualityATGCsequence.ptr(), qualityATGCsequence.length());
            param->str->add("\n",1);

        }
    } else {
        const char *subid = bioal->Sub->id(hdr->idSub());
        if (*subid == '>'){
            ++subid;
        }
        subIDStrNoSpace.cut(0);
        sString::copyUntil(&subIDStrNoSpace, subid, 0, " ");
        subIDStrNoSpace.shrink00();
        const char *qryid = bioal->Qry->id(hdr->idQry());
        if (*qryid == '>'){
            ++qryid;
        }
        qryIDStrNoSpace.cut(0);
        idx qryidlen = sString::copyUntil(&qryIDStrNoSpace, qryid, 0, " ");
        for(idx ii=0; ii<rptCnt; ii++){
            param->str->add(qryIDStrNoSpace.ptr(), qryidlen);
            if( rptCnt > 1 ) {
                param->str->add("_", 1);
                param->str->addNum(ii + 1);
            }
            param->str->add("\t",1);
            param->str->addNum(samflags);
            param->str->add("\t",1);
            param->str->add(subIDStrNoSpace.ptr(),subIDStrNoSpace.length());
            param->str->add("\t",1);
            param->str->addNum(hdr->getSubjectStart(m) + 1);
            param->str->add("\t",1);
            param->str->addNum(hdr->score());
            param->str->add("\t",1);
            param->str->add(CIGAR.ptr(), CIGAR.length());
            param->str->add("\t*\t0\t0\t", 7);
            param->str->add(queryATGCsequence.ptr(), queryATGCsequence.length());
            param->str->add("\t",1);
            param->str->add(qualityATGCsequence.ptr(), qualityATGCsequence.length());
            param->str->add("\n",1);

        }
    }

    param->str->add0cut();

    return 1;
}

idx sViosam::createVCFheader(FILE * stream, const char * refname,  real threshold) {

    time_t t = time(0);
    tm * now = localtime( &t);


    fprintf(stream,
        "##fileformat=VCFv4.0\n"
        "##fileDate=%d%02d%02d\n"
        "##source=HIVE\n", (now->tm_year + 1900), (now->tm_mon +1), now->tm_mday);
    if (refname) {
        fprintf(stream, "##reference=%s\n", refname);
    }
    fprintf(stream,
        "##INFO=<ID=AC,Number=.,Type=Integer,Description=\"Allele count in genotypes, for each ALT allele, in the same order as listed\">\n"
        "##INFO=<ID=AF,Number=.,Type=Float,Description=\"Allele Frequency\">\n"
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##INFO=<ID=CG,Number=1,Type=Integer,Description=\"Consensus Genotype\">\n"
        "##FILTER=<ID=PASS,Description=\"Coverage Threshold level of at least %lf\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\n", threshold);

    return 1;
}

idx sViosam::convertSNPintoVCF(sBioseqSNP::SNPRecord * snpRecord, sBioseqSNP::ParamsProfileIterator * params,idx iNum)
{
    sStr alleleFrequency;
    sStr alleleFreqPercentage;

    idx coverageCount = snpRecord->coverage();

    idx position;
    if ((idx)(snpRecord->position) == sNotIdx)
        return -1;
    else position = snpRecord->position - 1;


    sStr ref;

    sBioseqSNP::InDels * indels = static_cast<sBioseqSNP::InDels*>(params->userPointer);


    static idx lastReportedDeletion = -1;
    static idx prevSub = -1;

    if ( !(snpRecord->letter=='A'|| snpRecord->letter=='C'|| snpRecord->letter=='G'|| snpRecord->letter=='T'|| snpRecord->letter=='N') )
        return -1;

    idx reportDeletion = 0,max_deletion_length = 0;

    sStr alt;
    int isMutationStartInPrevPosition = 0;
    if ( params->prev_rec.position != snpRecord->position ) {
        isMutationStartInPrevPosition = 1;
    }

    ref.printf("%c", snpRecord->letter);
    if( !indels ) {
        if( prevSub != params->iSub ) {
            lastReportedDeletion = -1;
        }
        prevSub = params->iSub;

        sBioseqSNP::SNPRecord deletion_reportingRec = *snpRecord;

        real av_del_freq = 0;
        idx deletion_count = 0;
        if( lastReportedDeletion <= position ) {
            const char * tmp_snpLine = params->current_row;
            lastReportedDeletion = position;
            idx last_valid = lastReportedDeletion;
            while( deletion_reportingRec.freqRaw(5) >= params->threshold ) {
                ++lastReportedDeletion;
                av_del_freq += deletion_reportingRec.freqRaw(5);
                deletion_count += deletion_reportingRec.indel[1];
                ++reportDeletion;
                last_valid = sBioseqSNP::getNextRecord(params, deletion_reportingRec, lastReportedDeletion, last_valid);
                if( !(params->flags & sBioseqSNP::eCollapseConsecutiveDeletions) )
                    break;
            }
            max_deletion_length = lastReportedDeletion - position;
            params->current_row = tmp_snpLine;
        }
        if(reportDeletion) {
            av_del_freq/=reportDeletion ;
            if(av_del_freq >= params->threshold) {
                alleleFrequency.printf(",%" DEC, deletion_count/reportDeletion);
                alleleFreqPercentage.printf(",%.03f", av_del_freq/reportDeletion);
                ref.cut0cut();
                position -= isMutationStartInPrevPosition;
                sBioseq::uncompressATGC(&ref, params->seq, position, max_deletion_length + isMutationStartInPrevPosition);
                alt.printf(",%.*s", isMutationStartInPrevPosition, ref.ptr(0));
            } else {
                reportDeletion = 0;
            }
        }
        if(!reportDeletion){
            isMutationStartInPrevPosition = 0;
        }
    } else if(indels->getInDel(snpRecord->iSub-1,snpRecord->position-1)){
        sDic<sBioseqSNP::PosInDel::info> *  dels = indels->getDeletions(snpRecord->iSub-1,snpRecord->position-1);
        sBioseqSNP::PosInDel::info * cur_indel = 0;
        idx cur_indel_length = 0;
        real cur_indel_freq = 0;
        for( idx i_d = 0;  dels && i_d<dels->dim() ;++i_d) {
            cur_indel = dels->ptr(i_d);
            cur_indel_freq = cur_indel->freq(snpRecord->coverage() ? snpRecord->coverage() : 1);
            if( cur_indel_freq >= params->threshold ) {
                ++reportDeletion;
                cur_indel_length = *(idx*)dels->id(i_d);
                max_deletion_length = sMax(max_deletion_length,cur_indel_length);
                alleleFrequency.printf(",%" DEC, cur_indel->cnt());
                alleleFreqPercentage.printf(",%.03f", cur_indel_freq);
            }
        }
        if(reportDeletion) {
            ref.cut0cut();
            position -= isMutationStartInPrevPosition;
            sBioseq::uncompressATGC(&ref, params->seq, position, max_deletion_length + isMutationStartInPrevPosition);
        } else {
            isMutationStartInPrevPosition = 0;
        }
        for( idx i_d = 0;  dels && i_d<dels->dim() ;++i_d) {
            cur_indel = dels->ptr(i_d);
            cur_indel_freq = cur_indel->freq(snpRecord->coverage() ? snpRecord->coverage() : 1);
            if( cur_indel_freq >= params->threshold ) {
                cur_indel_length = *(idx*)dels->id(i_d);
                alt.printf(",%.*s", (int)(max_deletion_length-cur_indel_length+isMutationStartInPrevPosition), ref.ptr(0));
            }
        }

        sDic<sBioseqSNP::PosInDel::info> *  ins = indels->getInsertions(snpRecord->iSub-1,snpRecord->position-1);
        const char * cur_ins = 0;
        for( idx i_d = 0;  ins && i_d<ins->dim() ;++i_d) {
            cur_indel = ins->ptr(i_d);
            cur_indel_freq = cur_indel->freq(snpRecord->coverage() ? snpRecord->coverage() : 1);
            if( cur_indel_freq >= params->threshold ) {
                cur_ins = (const char *)ins->id(i_d, &cur_indel_length);
                alt.printf(",%.*s", isMutationStartInPrevPosition+1, ref.ptr(0));
                alt.printf("%.*s", (int)cur_indel_length, cur_ins);
                alt.printf("%s", ref.ptr(isMutationStartInPrevPosition+1));
                alleleFrequency.printf(",%" DEC, cur_indel->cnt());
                alleleFreqPercentage.printf(",%.03f", cur_indel_freq);
            }
        }
    }

    idx quality = 0;
    if (snpRecord->qua != sNotIdx)
        quality = snpRecord->qua;

    if(!reportDeletion) {
        isMutationStartInPrevPosition = 0;
    }


    idx alphabetsize = indels?sBioseqSNP::SNPRecord::symbol_LastBase:sBioseqSNP::SNPRecord::symbol_Last;

    for(idx i = 0; i <= alphabetsize  ; ++i) {

        if( i==sBioseqSNP::SNPRecord::symbol_Del ) {
            continue;
        }
        real freq = snpRecord->freqRaw(i);
        if( freq >= params->threshold && (sBioseqSNP::SNPRecord::isIndexAnIndel(i) || snpRecord->letter != sBioseq::mapRevATGC[i]) ) {
            alleleFrequency.printf(",%" DEC "", snpRecord->atgc[i]);

            alleleFreqPercentage.printf(",%.03f", freq);

            alt.printf(",");
            alt.printf("%.*s", isMutationStartInPrevPosition, ref.ptr(0));

            if( sBioseqSNP::SNPRecord::isIndexABase(i) ) {
                alt.printf("%c", sBioseq::mapRevATGC[i] );
            } else if( i == sBioseqSNP::SNPRecord::symbol_Ins ) {
                alt.printf("%c",ref.ptr(ref.length()-1)[0]);
                alt.printf("N");
            } else if ( !reportDeletion ) continue;

            if( ref.length() > isMutationStartInPrevPosition + 1 ) {
                alt.printf("%s", ref.ptr(isMutationStartInPrevPosition+1));
            }
        }
    }

    if( alt.length() > 1 ){
        params->str->printf("%s\t%" UDEC "\t.\t%s\t%s\t%" DEC "\tPASS\tDP=%" DEC ";AC=%s;AF=%s\n", params->chrName.ptr(), position + 1, ref.ptr(), alt.ptr(1), quality, coverageCount, alleleFrequency.ptr(1), alleleFreqPercentage.ptr(1));
    }
    return 1;
}

void sViosam::printSam ( sBioseq *Qry, sStr * samData, sStr *outFile)
{
    const char * buf = samData->ptr(0);
    const char * lastPos = buf + samData->length();
    const char * ptr;

    idx iAl;
    for(iAl = 0; buf < lastPos; ++iAl) {
        ptr = buf;
        if( *buf == '@' ) {
            ptr = skipUntilEOL(buf, lastPos);
            outFile->add(buf, ptr - buf);
            buf = skipUntilEOL(buf, lastPos);
        } else
            break;
    }

    for(idx ipos = 0; ipos < Qry->dim(); ipos++) {


        idx lnLen = lastPos - buf;
        const char *ptr1 = sString::skipWords(buf, lnLen, 1, "\t");
        int flag = atoi(ptr1);
        sBioseq::ESeqDirection isRevCmp = flag & eSamRevComp ? sBioseq::eSeqReverseComplement : sBioseq::eSeqForward;
        outFile->add(buf, ptr1 - buf);

        buf = ptr1;
        ptr1 = sString::skipWords(ptr1, lnLen, 8, "\t");
        outFile->add(buf, ptr1 - buf);


        Qry->printSequence(outFile, ipos, 0, 0, isRevCmp, true);
        outFile->addString("\t");

        const char * qua = Qry->qua(ipos);
        if( isRevCmp == sBioseq::eSeqReverseComplement ) {
            for(idx i = Qry->len(ipos) - 1; i >= 0; --i) {
                char new_qua = qua[i] + 33;
                outFile->add(&new_qua, 1);
            }
        } else {
            for(idx i = 0; i < Qry->len(ipos); ++i) {
                char new_qua = qua[i] + 33;
                outFile->add(&new_qua, 1);
            }
        }
        outFile->addString("\t");
        buf = ptr1;
        ptr1 = skipUntilEOL(buf, lastPos);

        outFile->add(buf, ptr1 - buf);
        buf = ptr1;
    }
}
