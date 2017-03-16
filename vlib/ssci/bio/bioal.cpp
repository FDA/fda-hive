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

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioal.hpp>
#include <ssci/math/rand/rand.hpp>

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Bioal information
// _/
// _/_/_/_/_/_/_/_/_/_/_/

idx sBioal::remap( sBioal * mutual, sVec<idx> &remappedHits, idx * alSortList ) {

    bool mutualMode = ( mutual->Qry->getmode() != Sub->getmode() )?true:false;
    //static
    idx alCnt = 0 , iAl = 0, ii = 0, totAl = dimAl();
    sBioseqAlignment::Al *hdrTo, *hdr, *hdrN;
    idx * mTo, * m , dimAl,curofs, curAl, curSub;

    for(idx iSub = 0 ; iSub < mutual->Qry->dim() ; ++iSub )
    {
        hdrTo = mutual->getAl( iSub );
        mTo = mutual->getMatch( iSub );
        curSub = mutualMode?(mutual->Qry->getmode()?mutual->Qry->long2short(hdrTo->idQry()): mutual->Qry->short2long(hdrTo->idQry())):hdrTo->idQry();
        iAl = listSubAlIndex( curSub, &alCnt);
        if( !iAl ) {
            continue;
        }
        iAl -=1 ;
        for(idx i = 0; i < alCnt; ++i,++iAl, ++ii) {
            curAl = iAl;
            if(alSortList) {
                curAl = alSortList[iAl];
            }
            hdr=getAl(curAl);
            curofs = remappedHits.dim();
            remappedHits.addM(sizeof(sBioseqAlignment::Al)/sizeof(idx));
            hdrN=(sBioseqAlignment::Al*)remappedHits.ptr(curofs);
            hdrN->lengths = hdr->lengths;
            hdrN->ids = hdr->ids;
            hdrN->starts = hdr->starts;
            hdrN->flscore = hdr->flscore;

            m=getMatch(curAl);

            sVec < idx > uncompressMM;

            if(hdr->flags()&sBioseqAlignment::fAlignCompressed){
                uncompressMM.resize(hdr->lenAlign()*2);
                sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
                m=uncompressMM.ptr();
            }

            sBioseqAlignment::remapAlignment(hdrN, hdrTo, m, mTo, 0, 0);
            hdrN->ids = hdr->ids;

            hdrN->setDimAlign(2*hdrN->lenAlign());
            idx * mdst = remappedHits.addM(hdrN->dimAlign());
            hdrN = (sBioseqAlignment::Al*)remappedHits.ptr(curofs);

            mdst[0] = m[0];mdst[1] = m[1];
            dimAl = sBioseqAlignment::compressAlignment(hdrN, m, mdst);
            hdrN->setDimAlign(dimAl);

            remappedHits.cut(curofs + hdrN->sizeofFlat()/sizeof(idx) );

            if(progress_CallbackFunction){
                if( !progress_CallbackFunction(progress_CallbackParam,ii,100*(ii)/totAl,100) )
                    return 0;
            }
        }
    }
    return remappedHits.dim();
}

idx sBioal::getConsensus(sStr &out, idx wrap /*= 0*/, idx mode /*= 0*/) {
    idx iVis = 0;
    sVec <idx> resVec(sMex::fSetZero);
    ParamsAlignmentIterator params;
    params.userPointer = &resVec;
    params.navigatorFlags = mode;
//    params.str = &out;
    idx res = iterateAlignments(&iVis,0,sIdxMax,-2,countAlignmentLetters,&params);
    idx max = 0, maxI = 4;
    iVis = 0;

    for (idx i = 0 ; i < resVec.dim() && res; i+=5 ) {
        ++iVis;
        max = 0;
        maxI =4;
        for(idx j = 0 ; j<5 ; ++j) {
            if(max < resVec[i+j] || (resVec[i+j] && max == resVec[i+j] && rand()%2)) {
                if( (mode&alConsensusOverlap) && j==4) {
                    continue;
                }
                max = resVec[i+j];
                maxI = j;
            }
        }
        if(maxI<4)
            out.printf("%c",sBioseq::mapRevATGC[maxI]);
        else if (!(mode&alConsensusIgnoreGaps))
            out.printf("-");
        else{
            max = -1;
            --iVis;
        }
        if(max>=0 && wrap && !(iVis%wrap)) {
            out.printf("\n");
        }
    }
    return iVis;
}

//binary search for alignment ranges. Returns the last Al having the startRange or if not such the next greater position
//and the reverse for max. Needs the indexes of the sorted relations for alignments
idx sBioal::bSearchAlignments(idx iAlPoint,idx iAlmin, idx iAlmax, idx sortIndFirst, idx isEnd){
    while(iAlmax>=iAlmin){
        idx iAlmid=iAlmin+((iAlmax-iAlmin)/2);
        sBioseqAlignment::Al * hdrMid=getAl(sortIndFirst+iAlmid-1);
        idx mAlmid=getMatch(sortIndFirst+iAlmid-1)[0];
        if(!isEnd)mAlmid+=2*hdrMid->lenAlign();
        if(hdrMid->subStart()+mAlmid < iAlPoint)
            iAlmin=iAlmid+1;
        else if(hdrMid->subStart()+mAlmid > iAlPoint)
            iAlmax=iAlmid-1;
        else if(!isEnd)
            iAlmax=iAlmid-1;
        else
            iAlmin=iAlmid+1;
    }
    return (!isEnd?iAlmin-1:iAlmax+1);
}


idx sBioal::iterUnAligned(idx * piVis,idx start,idx cnt,ParamsAlignmentIterator * params)
{
    idx alCnt=dimAl();

    idx qryDim=Qry->dim();
    sVec<char> qFaiBitmap;
    char * QFaiBM=qFaiBitmap.addM(qryDim/8+1);
    qFaiBitmap.set(0);
    idx qy=0;
    for(idx iAl=0;iAl<alCnt;++iAl){
        qy=getAl(iAl)->idQry();
        QFaiBM[qy/8]|=((idx(1))<<((qy%8)));
    }
    idx tot=0, rpts=0;
    bool isQ = (params->navigatorFlags&alPrintQualities);
    sStr _rptbuf;
    idx tot_done = (cnt>qryDim)?qryDim:cnt;
    for(idx iAl=0;iAl<qryDim && *piVis<start+cnt ;++iAl){

        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,iAl,100*((cnt>qryDim)?iAl:*piVis)/tot_done,100) )
                return 0;
        }

        if(!(QFaiBM[iAl/8]&(((idx)1)<<(iAl%8)))){
            ++*piVis;
            if(*piVis<start)continue;

            Qry->printFastXRow(params->str, isQ, iAl, 0, 0, 0, true, false, 0, 0, 0, true, 0, false);
            rpts = (!(params->navigatorFlags & alPrintCollapseRpt))?Qry->rpt(iAl):1;
            for( idx r=1 ; r < rpts ; ++r ){
                _rptbuf.printf(0, "%"DEC,r+1);
                Qry->printFastXRow(params->str, isQ, iAl, 0, 0, 0, true, false, _rptbuf.ptr(), 0, 0, true, 0, false);
            }
            tot+=rpts;
        }
        if(params->outF){
            fwrite(params->str->ptr(),params->str->length(),1,params->outF);
            params->str->cut(0);
        }
    }
    return tot;
}

bool sBioal::regexAlignmentSingle(sStr &compStr, sStr &out, idx start, idx end, ParamsAlignmentIterator * callbackParam)
{
    idx rg_ofs = 0, rgs = 0, rge = 0;
    regmatch_t pmatch[1];
    bool matchS = false, loop = true;
    while( loop ) {
        matchS = (regexec(callbackParam->regp, compStr.ptr(rg_ofs), 1, pmatch, 0) == 0) ? true : false;
        if( matchS ) {
            rgs = pmatch[0].rm_so + rg_ofs;
            rge = pmatch[0].rm_eo + rg_ofs;
            if( (callbackParam->navigatorFlags & alPrintPositionalRegExp) && callbackParam->High>=0 ) {
                if( sOverlap(rgs, rge, start, end ) ) {
                    out.printf(",%"DEC",%"DEC, rgs + 1, rge);
                    loop = false;
                } else {
                    rg_ofs += rgs;
                }
            } else {
                loop = false;
                out.printf(",%"DEC",%"DEC, rgs + 1, rge);
            }
        }
        else{
            loop = false;
        }
    }
    return matchS;
}

// filters logic
idx isPrimeFilter( sBioal::ParamsAlignmentIterator * callbackParam )
{
    if(callbackParam->rangestart!=callbackParam->rangeend && !(callbackParam->navigatorFlags&sBioal::alPrintMultiple) ) //Range filter
        return true;
    if( callbackParam->navigatorFlags&sBioal::alPrintFilterTail ) //Tail filter
        return true;
    if( !(callbackParam->navigatorFlags&sBioal::alPrintForward) != !(callbackParam->navigatorFlags&sBioal::alPrintReverse)) //Direction filter
        return true;
    if(callbackParam->navigatorFlags&sBioal::alPrintRepeatsOnly)
        return true;
    if(callbackParam->navigatorFlags&sBioal::alPrintTouchingOnly)
        return true;
    return false;
}

idx sBioal::iterateAlignments(idx * piVis, idx start, idx cnt, idx iSub, typeCallbackIteratorFunction callbackFunc, ParamsAlignmentIterator * callbackParam /*= 0*/, typeCallbackIteratorFunction secondaryCallbackFunc /*= 0*/, ParamsAlignmentIterator * secondaryCallbackParam /*= 0*/, idx * sortArr /*= 0*/ )
{
    //idx * alList=0;
    idx alListIndex=0;
    idx alCnt=0, iFound=0;

    idx myVis=0;
    if(!piVis)piVis=&myVis;
    if(!cnt)cnt=sIdxMax;
    else if (callbackParam->navigatorFlags&alPrintMultiple) {
        cnt *=dimAl();
    }

    idx dif = 0;
    if (callbackParam->navigatorFlags&alPrintMultiple) {
        dif = callbackParam->rangeend - callbackParam->rangestart;
        start *=dimAl();
    }

    if ( !secondaryCallbackFunc && (callbackParam->navigatorFlags&alPrintReadsInFasta) ) {
        secondaryCallbackFunc = printFastaSingle;
    }


    if( iSub==sNotIdx ) {iterUnAligned(piVis,start,cnt,callbackParam);return 1;}//alCnt=dimAl();
    else if (iSub < 0) alCnt = dimAl();
    else if(iSub<Sub->dim() ) alListIndex=listSubAlIndex(iSub, &alCnt);
    else return 0;

    idx res=0, buflenBefore;

    bool primaryFilters=isPrimeFilter(callbackParam);
    bool secondaryFilters=(callbackParam->regp || secondaryCallbackFunc)? true : false;

    idx iAlStart=0,iAlEnd=callbackParam->navigatorFlags&alPrintMultiple?dimAl():alCnt;
    if(primaryFilters && callbackParam->rangestart!=callbackParam->rangeend && alListIndex){
        idx iAlRangeStart=callbackParam->rangestart-(callbackParam->maxAlLen?callbackParam->maxAlLen:callbackParam->winSize);
        iAlRangeStart=iAlRangeStart<0?0:iAlRangeStart;
        if(iAlRangeStart){
            iAlStart=bSearchAlignments(iAlRangeStart,0,alCnt-1,alListIndex,1);
        }
        if(callbackParam->rangeend<(getAl(alCnt-1)->subStart() + getMatch(alCnt-1)[0]))
            iAlEnd=bSearchAlignments(callbackParam->rangeend,iAlStart,alCnt-1,alListIndex,1)+1;
    }
    if(!secondaryFilters && cnt!=sIdxMax)++cnt;

    idx * randInds=sortArr;
    sVec<idx> randIndArr;

    idx iAlStart2End = iAlEnd - iAlStart;
    if( !randInds && !(callbackParam->navigatorFlags & alPrintMultiple) && ((callbackParam->navigatorFlags & (alPrintInRandom)) || !(callbackParam->navigatorFlags & alPrintCollapseRpt)) ) {
        idx t_ia = 0, t_rpts = 0, i_add = 0;
        randIndArr.resize(iAlStart2End);
        for(; iAlStart < iAlEnd; ++iAlStart) {
            t_ia = alListIndex ? alListIndex + iAlStart - 1 : iAlStart;
            sBioseqAlignment::Al * hdr = getAl(t_ia);
            idx * m = getMatch(t_ia);
            idx alStart=hdr->getSubjectStart(m), alEnd=hdr->getSubjectEnd(m);
            if( primaryFilters && callbackParam->rangestart != callbackParam->rangeend && alStart > callbackParam->rangeend )
                continue;
            if( primaryFilters && (callbackParam->navigatorFlags&alPrintTouchingOnly) && callbackParam->High >= 0 && ((alStart>callbackParam->High || alEnd<callbackParam->High)) )
                continue;

            randIndArr[i_add++] = t_ia;
            if( !(callbackParam->navigatorFlags & alPrintCollapseRpt) ) {
                t_rpts = getRpt(t_ia);
                if( t_rpts - 1 > 0 ) {
                    randIndArr.add(t_rpts - 1);
                    for(idx r = 1; r < t_rpts; ++r) {
                        randIndArr[i_add++] = t_ia;
                    }
                }
            }
        }
        randIndArr.cut(i_add);
        iAlEnd = iAlStart + randIndArr.dim();
        if(randIndArr.dim()){
            randInds=randIndArr.ptr();
            iAlStart=0;iAlEnd=randIndArr.dim();
        }
    }
    idx ia=iAlStart;
    iAlEnd = iAlStart + randIndArr.dim();
    for (idx iAlCnt = 0 ; ia<iAlEnd; ++ia, ++iAlCnt) {
        idx iAl=0;
        if(randInds){
            iAl=randInds[ia];
            if(callbackParam->navigatorFlags&alPrintInRandom){
                idx iInd = (sRand::random1()*(iAlStart2End-1));
                iAl=randInds[iInd];
                randInds[iInd]=randInds[iAlStart2End-1];
                --iAlStart2End;
                iAlEnd=iAlStart2End;
                --ia;
            }
        }
        else{
            iAl=alListIndex ? alListIndex+ia-1 : ia ;///alList ? alList[ia]-1 : ia;
        }
        sBioseqAlignment::Al *  hdr=getAl(iAl);
        idx * m=getMatch(iAl);


        // filters which do not need an output to be generated
        bool isok=true;
        // we might need this to roll back additions to the buffer
        buflenBefore=callbackParam->str ? callbackParam->str->length() : 0 ;
        if(primaryFilters){
            idx alStart=hdr->getSubjectStart(m), alEnd=hdr->getSubjectEnd(m);
            if( (callbackParam->rangeend!=callbackParam->rangestart) && (alStart>callbackParam->rangeend || alEnd < callbackParam->rangestart) )
                isok=false;

            if(callbackParam->navigatorFlags&alPrintRepeatsOnly ) {
                isok=false;
                if( iAl>0){
                    sBioseqAlignment::Al *  prv=getAl( iAl - 1 );
                    if(prv->idQry()==hdr->idQry())
                        isok=true;
                }
                if( iAl<alCnt-1 && !isok){
                    sBioseqAlignment::Al *  nxt=getAl( iAl +1);
                    if(nxt->idQry()==hdr->idQry())
                        isok=true;
                }
            }

            if(callbackParam->navigatorFlags&sBioal::alPrintFilterTail){
                idx qs=hdr->getQueryStart(m);
                idx qe=hdr->getQueryEnd(m);
                if( qs<callbackParam->leftTailPrint && ( (Qry->len(hdr->idQry()) - qe < callbackParam->rightTailPrint)  || (alEnd>=callbackParam->rangeend) ) ) // ||
                    isok=false;
            }

            if((callbackParam->navigatorFlags&alPrintTouchingOnly) && callbackParam->High >= 0 && (alStart>callbackParam->High || alEnd<callbackParam->High))
                isok=false;
            if( !(callbackParam->navigatorFlags&sBioal::alPrintForward) != !(callbackParam->navigatorFlags&sBioal::alPrintReverse) ){
                if( ( (callbackParam->navigatorFlags&sBioal::alPrintForward) && hdr->isBackwardComplement() ) || ((callbackParam->navigatorFlags&sBioal::alPrintReverse) && hdr->isForward()) ){
                    isok=false;
                }
            }
        }

        if( isok && secondaryFilters ) { // call the function only if it has passed preliminary filters and needs a secondary filter
            res=callbackFunc(this,callbackParam, hdr, m ,iAlCnt-iAlStart, iAl);
            if(!res)isok=false;

            //secondary filters which needed an output content to filter with
            if(callbackParam->regp && isok && callbackParam->str)
            {
                sStr thdl,hdl1,hdl2,hdl3;
                const char * curOut=callbackParam->str->ptr(buflenBefore);
                sString::searchAndReplaceSymbols(&thdl, curOut, 0, "\n",0,0,true,true,false,false);
                curOut=thdl.ptr();
                if((callbackParam->navigatorFlags&sBioal::alPrintSubject) && curOut){
                    hdl1.printf("%s",curOut);curOut=sString::next00(curOut);}
                if((callbackParam->navigatorFlags&sBioal::alPrintUpperInterm) && curOut){
                    hdl2.printf("%s",curOut);curOut=sString::next00(curOut);}
                if((callbackParam->navigatorFlags&sBioal::alPrintQuery) && curOut){
                    hdl3.printf("%s",curOut);curOut=sString::next00(curOut);}
                bool matchS=false,matchU=false,matchQ=false;
                idx added_padd = callbackParam->subRealS - callbackParam->padding - (callbackParam->navigatorFlags&alPrintPosInDelRegExp?1:0);
                if((callbackParam->navigatorFlags&alPrintRegExpSub) && hdl1){
                    sStr compStr;
                    sString::extractSubstring(&compStr,hdl1,0,callbackParam->alCol,","__,false,false);
                    matchS = regexAlignmentSingle(compStr, hdl1, callbackParam->High - added_padd, callbackParam->High - added_padd, callbackParam);

                }
                if((callbackParam->navigatorFlags&alPrintRegExpInt) && hdl2){
                    sStr compStr;
                    sString::extractSubstring(&compStr,hdl2,0,callbackParam->alCol,","__,false,false);
                    matchU = regexAlignmentSingle(compStr, hdl2, callbackParam->High - added_padd, callbackParam->High - added_padd, callbackParam);
                }
                if((callbackParam->navigatorFlags&alPrintRegExpQry) && hdl3){
                    sStr compStr;
                    sString::extractSubstring(&compStr,hdl3,0,callbackParam->alCol,","__,false,false);
                    matchQ = regexAlignmentSingle(compStr, hdl3, callbackParam->High - added_padd, callbackParam->High - added_padd, callbackParam);
                }
                callbackParam->padding=0;callbackParam->indels=0;
                if(matchQ || matchU || matchS){
                    callbackParam->str->cut(buflenBefore);
                    if(hdl1) callbackParam->str->printf("%s\n",hdl1.ptr());
                    if(hdl2) callbackParam->str->printf("%s\n",hdl2.ptr());
                    if(hdl3) callbackParam->str->printf("%s\n",hdl3.ptr());
                }
                else if(callbackParam->navigatorFlags&(alPrintRegExpSub|alPrintRegExpInt|alPrintRegExpQry))
                    isok=false;
            }

            if(isok && secondaryCallbackFunc) {
                callbackParam->str->cut(buflenBefore);
                res = secondaryCallbackFunc(this,secondaryCallbackParam?secondaryCallbackParam:callbackParam, hdr, m ,iAlCnt-iAlStart, iAl);
                if(!res)isok=false;
            }

        }
        // now consider if we need to really output this or not
        if( isok ) {
            if(piVis){
                ++(*piVis);
                if(*piVis<= start){
                    isok=false; // after this point isok is only used as a marker for cutting the str back
                }
                if(*piVis>=start+cnt){
                    break;
                }
            }
        }

        if( isok && !secondaryFilters ) {

            res=callbackFunc(this,callbackParam, hdr, m , callbackParam->navigatorFlags&alPrintMultiple?(iFound/dimAl()):(ia-iAlStart), iAl);// iAl);
            // this function has not yet been called if secondary filters were not defined
        }
        else { // cut out the last additions, those have not passed the filter
            if(callbackParam->str && !isok)
                callbackParam->str->cut(buflenBefore);
        }
        if(isok && !res && piVis)--(*piVis);
        if(res){
            ++iFound;
            if(callbackParam->outF){
                fwrite(callbackParam->str->ptr(),callbackParam->str->length(),1,callbackParam->outF);
                callbackParam->str->cut(0);
            }
        }

        if(res==sNotIdx)
            break;

        if( (callbackParam->navigatorFlags&alPrintMultiple) && ia==(iAlEnd-1) && callbackParam->rangeend < hdr->lenAlign() ) {
            ia = -1;
            callbackParam->rangestart += dif;
            callbackParam->rangeend += dif;
            callbackParam->str->printf("\n");
        }

        //
        // Report progress in order to prevent timeout with large data files
        //
        if ( iAlCnt % 100000 == 0 && callbackParam->reqProgressFunction ) {
            int percDone = (iAlCnt / iAlEnd * 80) + 12;
            callbackParam->reqProgressFunction(callbackParam->reqProgressParam, iAlCnt, percDone, 100);
        }
    }


    return iFound;
}


idx sBioal::printAlignmentSummaryBySubject(sVec < sBioal::Stat > & statistics, sStr * str, ParamsAlignmentIterator * params,bool reportZeroHits, bool reportTotals, bool reportFailed,idx start ,idx cnt,sVec<idx> * processedSubs, sVec<idx> * coverage, sBioal::HitListExtensionCallback callBackExtension,  void * param)
{
    sStr hdr, tmpBody;
    hdr.printf(0,"id,Reference,Hits,Hits Unique,Length");
    if(coverage){
        hdr.printf(",RPKM,Density");
    }

    bool isPE = isPairedEnd();
    if( isPE ) {
        hdr.printf(",FPKM");
    }

    sDic <idx> ionHdrDic;
    if(!callBackExtension){
        hdr.printf("\n");
    }
    sIO buf;

    idx tot=0,totRpt=0,len=0,iVis=0,buflenBefore=0,showTot=0,showTotRpt=0,showLen=0,tot_coverage = 0,show_coverage = 0;
    real curRPKM = 0, curFPKM = 0;
    sStr tmp;bool printOther=false;
    if(reportFailed ){
        ++iVis;
        tmpBody.printf("%"DEC",\"%s\",%"DEC",%"DEC",",(idx)0,"Unaligned", statistics[0].foundRpt , statistics[0].found );
        if(coverage){
            tmpBody.printf(",-,-");
        }
        tmpBody.printf("\n");
        --cnt;
    }

    if(reportTotals)--cnt;
    sVec<idx> statHits,sortedInd;
    statHits.resize(Sub->dim());sortedInd.add(Sub->dim());
    for(idx is=0; is<Sub->dim(); ++is ) {
        statHits[is]=statistics[is+1].foundRpt;
        totRpt += statHits[is];
    }
    sSort::sort(statHits.dim(), statHits.ptr(), sortedInd.ptr());


    for(idx is=Sub->dim()-1; is>=0; --is ) {
        idx srtID=sortedInd[is];

        if(!reportZeroHits && !statistics[srtID+1].found){
            continue;
        }
       if(callBackExtension ) {
             buf.cut(0);
             sDic <sStr > dic;
             sStr tmpHdrForIon;
             callBackExtension(param, &buf,0, &dic,Sub->id(srtID),1,0,1,0, &tmpHdrForIon);
             if (dic.dim()) {
                 idx lenkey = 0;
                 for (idx iik=0; iik < dic.dim(); ++ iik) {
                     lenkey=0;
                     const char * kkey = (const char *)dic.id(iik, &lenkey);
                     if (!ionHdrDic.find(kkey,lenkey)) {
                         *ionHdrDic.set(kkey,lenkey)=1;
                     }
                 }
             }
         }

        len+=Sub->len(srtID);
        idx * cur_cov = 0;
        if( coverage && coverage->dim() > srtID) {
            cur_cov = coverage->ptr(srtID);
            if(cur_cov){
                tot_coverage += *cur_cov;
            }
        }

        tot+=statistics[srtID+1].found;
        curRPKM = (statistics[srtID+1].foundRpt*1000000000.)/Sub->len(srtID)/totRpt;
        if(isPE) {
            curFPKM = (statistics[Sub->dim()+srtID+1].foundRpt*1000000000.)/Sub->len(srtID)/totRpt;
        }

        if(processedSubs){
            idx imax=processedSubs->dim(),imin=0,isThere=0;
            while( imax >= imin ) {
                idx imid =  imin + (imax -imin) /2;
                if( *processedSubs->ptr(imid) < srtID+1 )
                    imin = imid + 1;
                else if( *processedSubs->ptr(imid) > srtID+1 )
                    imax = imid - 1;
                else{
                    isThere=1;break;
                }
            }
            if(!isThere)continue;
        }

        ++iVis;
        if(iVis<=start && cnt>0){printOther=true;continue;}
        if(iVis>start+cnt && cnt>0){printOther=true;continue;}
        buflenBefore = tmpBody.length();
        regmatch_t pmatch1[1];
        if(iVis<=start+cnt || cnt<=0){
            tmp.printf(0,"%"DEC",\"%s\",%"DEC",%"DEC",%"DEC,srtID+1, Sub->id(srtID) ,statistics[srtID+1].foundRpt,statistics[srtID+1].found,Sub->len(srtID));
            if(cur_cov){
                tmp.printf(",%.1f,%.1f",curRPKM,(real)(*cur_cov)/Sub->len(srtID) );
            }
            if(isPE) {
                tmp.printf(",%.1f",curFPKM );
            }
            if(callBackExtension && buf.length()){
                tmp.add(",",1);
                tmp.add(buf.ptr(),buf.length());
            }
            else tmp.printf("\n");
        }
        if(params->regp && iVis<=start+cnt && cnt>0)
            if(!(regexec(params->regp, tmp, 1, pmatch1, 0)==0)){--iVis;printOther=true;continue;}

        tmpBody.add(tmp.ptr(),tmp.length());
        showLen+=Sub->len(srtID);
        if(cur_cov){
            show_coverage += *cur_cov;
        }
        showTot+=statistics[srtID+1].found;
        showTotRpt+=statistics[srtID+1].foundRpt;
    }

    if(!len) {
        return 0;
    }
    if(!iVis){
        str->cut(0);reportTotals=false;
    }
    if(reportTotals){
        if( (printOther || processedSubs) && (len-showLen) && showLen ){
            tmpBody.printf("+,\"%s\",%"DEC",%"DEC",%"DEC,"other",totRpt-showTotRpt,tot-showTot,len-showLen);
            if(coverage){
                tmpBody.printf(",-,%"DEC,tot_coverage-show_coverage/(len-showLen));
            }
            tmpBody.printf("\n");
        }
        tmpBody.printf("+,\"%s\",%"DEC",%"DEC",%"DEC"", "total" ,totRpt,tot, len);
        if(coverage){
            tmpBody.printf(",-,%"DEC,tot_coverage/len);
        }
        tmpBody.printf("\n");
    }

    if (callBackExtension) {
        if (ionHdrDic.dim()) {
            idx lenkey=0;
            for (idx iik=0; iik<ionHdrDic.dim(); ++iik) {
                lenkey=0;
                const char * hdrkey = (const char *) ionHdrDic.id(iik,&lenkey);
                hdr.add(",",1);
                hdr.addString(hdrkey,lenkey);
            }
            hdr.add("\n",1);
        }


    }
    str->add(hdr.ptr(),hdr.length());
    str->add(tmpBody.ptr(),tmpBody.length());
    if(printOther)
        tmpBody.cut(buflenBefore);

    return iVis;
}

idx __sort_LenHistogramSorter(void * param, void * arr , idx i1, idx i2)
{
    sDic < sBioal::LenHistogram > * lenHistogram=(sDic < sBioal::LenHistogram > *)param;
    idx ipos1=*(idx*)lenHistogram->id(i1);
    idx ipos2=*(idx*)lenHistogram->id(i2);

    if( ipos1> ipos2)
        return 1;
    else if( ipos1< ipos2)
        return -1;
    return 0;
}

idx sBioal::printAlignmentHistogram(sStr * out , sDic < sBioal::LenHistogram > * lenHistogram )
{

    sVec <idx >idxArr; idxArr.add(lenHistogram->dim());
    sSort::sortSimpleCallback<sBioal::LenHistogram>((sSort::sCallbackSorterSimple)__sort_LenHistogramSorter, lenHistogram, lenHistogram->dim(), lenHistogram->ptr(), idxArr.ptr() );

    //struct LenHistogram {idx cntSeq, cntAl, cntLeft, cntRight; LenHistogram(){sSet(this,0);} };
    out->printf("position,All Reads,Aligned Reads,Unaligned Reads,Alignments,Left Incomplete Alignments,Right Incomplete Alignments\n");
    for( idx ihH=0; ihH<lenHistogram->dim(); ++ihH ){
        idx ih=idxArr[ihH];
        sBioal::LenHistogram  * ll=lenHistogram->ptr(ih);
        idx * ilen=(idx*)lenHistogram->id(ih);

        out->printf("%"DEC",%"DEC",%"DEC",%"DEC",%"DEC",%"DEC",%"DEC"\n",*ilen,ll->cntRead,ll->cntSeq, ll->cntFailed, ll->cntAl, ll->cntLeft, ll->cntRight);
    }
    return lenHistogram->dim();
}


idx sBioal::countAlignmentSummaryBySubject(sVec < Stat >  & statistics)
{
    statistics.flagOn(sMex::fSetZero );
    statistics.resize(Sub->dim()+1);
    idx cnt=0, size;

    for( idx it=0, ct=dimStat(); it<ct; ++it ){
        Stat * pS=getStat(it,0,&size);
        statistics.resize(size);
        cnt+=size;
        for(idx is=0; is<size; ++is ) { //Sub->dim()+1
            //listSubAl(is,&cnt);
            statistics[is].found+=pS[is].found;
            statistics[is].foundRpt+=pS[is].foundRpt;
        }
    }
    return cnt;
}

idx sBioal::countAlignmentLetters(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAl)
{
    sVec<idx> * vec = (sVec<idx> *)params->userPointer;
    idx ind = 0, is = 0, iq = 0, iqx, alphabetSize = 5;

    idx qrybuflen=bioal->Qry->len(al->idQry()), sStart=al->subStart()+m[0], qStart=al->qryStart(), flags=al->flags();
    idx nonCompFlags=(flags)&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement));

    while(is < sStart) {
        ++ind;
        vec->resize(++is*alphabetSize);
        (*(vec->ptr((is-1)*alphabetSize+4)))++;
    }

    sVec < idx > uncompressMM;uncompressMM.resize(2*al->lenAlign());
    sBioseqAlignment::uncompressAlignment(al,m,uncompressMM.ptr());
    const char * qry=bioal->Qry->seq(al->idQry());
    for( idx i=0 ; i<2*al->lenAlign(); i+=2) {
        is = m[i];
        if (is >= 0) {
            ++ind;
            is += sStart;
            vec->resize((is+1)*alphabetSize);
        }
        else {
            continue;
        }
        iq = m[i+1];

        iqx = ( flags&sBioseqAlignment::fAlignBackward ) ? (qrybuflen-1-(qStart+iq)): (qStart+iq);
        if( iq==-1 ) {
            (*(vec->ptr(is*alphabetSize+4)))++;
        }
        else {
            (*(vec->ptr(is*alphabetSize+((idx)sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)))))++;
        }
    }
    return ind;
}

//static
idx sBioal::getSaturation(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd) {

    sDic<idx> * dicSubHits = (sDic<idx> *)params->userPointer;

    if( params->currentChunk > 0 ) {
        Stat * c_stat = bioal->getStat(0,hdr->idSub() + 1 ); //ZEROth is all
        if( c_stat && (((params->navigatorFlags&alPrintCollapseRpt) && c_stat->found < params->currentChunk) || (!(params->navigatorFlags&alPrintCollapseRpt) && c_stat->foundRpt < params->currentChunk)) ) {
            return 0;
        }
    }

    sStr buf;

    if( (params->navigatorFlags&sBioal::alPrintSaturationReduced) ) {
        iNum = params->userIndex;
    }

    idx lb = 0, * plb = dicSubHits->get("last_bin",sLen("last_bin"));
    if(plb) lb = *plb;

    while( lb < ((iNum+1)/params->wrap) ) {
        *(dicSubHits->setString("last_bin")) = lb+1;
        lb++;
        idx cntSubs = dicSubHits->dim() - (params->userIndex/params->wrap) - 1;
        buf.printf(0,"bin-%"DEC,(params->userIndex/params->wrap)+1);
        ( *(dicSubHits->setString( buf.ptr() )) ) = cntSubs;
        if( params->userIndex > iNum ) {
            params->userIndex = iNum;
            break;
        }
    }

    ++params->userIndex;
    buf.printf(0,"%"DEC,hdr->idSub());
    (*(dicSubHits->setString( buf.ptr() )))++;

    return 1;
}

//static
idx sBioal::printSubSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd) {
    sStr tSub;
    sString::searchAndReplaceSymbols(&tSub, bioal->Sub->id(al->idSub()), 0, ","," ",0,true,true,false,false);
    params->str->printf(
        "%"DEC","
        "%"DEC",\"%s\""
        ,iNum+1
        ,al->idSub()+1,tSub.ptr());
    return 1;
}

//static
idx sBioal::printMatchSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    sStr tSub,tQry;
    sString::searchAndReplaceSymbols(&tSub, bioal->Sub->id(al->idSub()), 0, ","," ",0,true,true,false,false);
    sString::searchAndReplaceSymbols(&tQry, bioal->Qry->id(al->idQry()), 0, ","," ",0,true,true,false,false);
    sVec < idx > uncompressMM;uncompressMM.resize(2*al->lenAlign());
    sBioseqAlignment::uncompressAlignment(al,m,uncompressMM.ptr());
    m=uncompressMM.ptr();
    params->str->printf(
        "%"DEC","
        "%"DEC",\"%s\",%"DEC",\"%s\","
        "%"DEC",%"DEC",%"DEC","
        "%"DEC",%"DEC",%"DEC",%"DEC"\n"

        ,iNum+1
        ,al->idSub()+1,tSub.ptr(),al->idQry()+1, tQry.ptr()
        ,al->score(),(al->flags()&(sBioseqAlignment::fAlignBackward) ) ? (idx)(-1) : (idx)(1), al->lenAlign()

        ,al->subStart()+m[0]+1,al->subStart()+m[(al->lenAlign()-1)*2]+1
        ,al->qryStart()+m[1]+1,al->qryStart()+m[(al->lenAlign()-1)*2+1]+1
        );

    return 1;
}

//static
idx sBioal::printBEDSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    sStr tSub,tQry;
    sString::searchAndReplaceSymbols(&tSub, bioal->Sub->id(al->idSub()), 0, ","," ",0,true,true,false,false);
    sString::searchAndReplaceSymbols(&tQry, bioal->Qry->id(al->idQry()), 0, ","," ",0,true,true,false,false);
    sVec < idx > uncompressMM;uncompressMM.resize(2*al->lenAlign());
    sBioseqAlignment::uncompressAlignment(al,m,uncompressMM.ptr());
    m=uncompressMM.ptr();

   params->str->printf(
        "\"%s\",%"DEC",%"DEC",U0,0,%s\n"
        ,tSub.ptr()
        ,al->qryStart()+m[1]+1, al->qryStart()+m[(al->lenAlign()-1)*2+1]+1
        ,(al->flags()&(sBioseqAlignment::fAlignBackward) ) ? ("-") : ("+")
        );

    return 1;
}

//static
idx sBioal::printFastaSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    idx iseq=al->idQry();

    const char * id = bioal->Qry->id(iseq);
    if(*id=='>')++id;

    params->str->printf(
        ">%s %"DEC"\n"
        ,id,(params->navigatorFlags & alPrintCollapseRpt)?bioal->getRpt(iAlInd):1);

    if(params->navigatorFlags&alPrintMultiple) {
        params->rangeend = al->lenAlign();
        printAlignmentSingle(bioal, params, al, m, iNum, iAlInd);
    }
    else {
        sStr t;
        sBioseq::uncompressATGC(&t,bioal->Qry->seq(iseq), 0, bioal->Qry->len(iseq),true,params->wrap);
        params->str->printf("%s\n",t.ptr());
    }

    return 1;
}

//static
idx sBioal::printFastXSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    idx iq = al->idQry();
    char appendID[64], * papID  = 0;
    if(params->navigatorFlags & alPrintCollapseRpt) {
        sprintf(appendID,"%"DEC,iNum);
        papID = &appendID[0];
    }

    return bioal->Qry->printFastXRow(params->str, (params->navigatorFlags&alPrintQualities), iq, 0, 0, 0, true, false,
        papID, 0, al->isBackwardComplement()?3:0, (params->navigatorFlags&alPrintNs), 0, (params->navigatorFlags & alPrintCollapseRpt), false);
}

#define isSameBase(baseS,baseQ) (baseS!='N'&&baseQ!='N'&&baseS==baseQ)

#define printAlignmnetSingleLeftColumns \
{\
    if(outwhat&alPrintAsFasta){ \
        hdl3.printf(">%s\n",tQry.ptr()); \
    } else { \
        if( (outwhat&alPrintSubject) && !(outwhat&alPrintSequenceOnly) ) \
            hdl1.printf("%"DEC",%5"DEC", (%s) ,%5"DEC"",iNum|curChunk, idSub+1,((flags&sBioseqAlignment::fAlignBackward) ? "-" :"+" ),isS+1); \
        if(outwhat&(alPrintUpperInterm|alPrintLowerInterm) ){ \
           hdl2.printf("%"DEC",,",iNum|curChunk); \
           if( params->alignmentStickDirectional==2 ){ \
                if( flags&sBioseqAlignment::fAlignBackward ) \
                    hdl2.printf("(-)"); \
                else \
                    hdl2.printf("(+)"); \
            } \
           hdl2.printf(","); \
        } \
        if( (outwhat&alPrintQuery) && !(outwhat&alPrintSequenceOnly) ) { \
            iqnn=isQ; \
            if ( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) \
                iqnn = qrybuflen-1-iqnn; \
            if(iqx>=0 && (outwhat&alPrintNonFlippedPosforRev))iqnn=isQ; \
            hdl3.printf("%"DEC",%5"DEC", (%c) ,",iNum|curChunk , idQry+1,( flags&sBioseqAlignment::fAlignBackward ) ? '-' : '+'); \
            hdl3.printf("%5"DEC"",iqnn+1 ); \
        } \
    } \
}
#define printAlignmnetSinglePaddingAndLeftTail \
{\
    idx offs=0, leftTail = qStart+m[1], iqLeft = 0; \
    if( (outwhat&alPrintBasedOnRange) || highlightposition>=0 ) offs = sStart-rangestart; \
    else if(params->leftTailPrint && leftTail && (outwhat&alPrintTailDisplayTail)) offs = leftTail; \
    if ( offs>0 ){ \
        if(sStart>highlightposition && highlightposition>=0) \
            ++isRightSide; \
        params->padding=offs; \
        if(outwhat&alPrintSubject)l1.printf("%*c",(int)offs,' '); \
        if(outwhat&(alPrintUpperInterm|alPrintLowerInterm))l2.printf("%*c",(int)offs,' '); \
        if(outwhat&alPrintQuery){ \
            if(params->leftTailPrint && (outwhat&alPrintTailDisplayTail) && ((outwhat&alPrintBasedOnRange) || highlightposition>=0) ){ \
                offs-=leftTail; \
                if(offs<0){ \
                    iqLeft-=offs; \
                    offs=0; \
                } \
            } \
            if(offs>0 && ((outwhat&alPrintBasedOnRange) || highlightposition>=0 ))l3.printf("%*c",(int)offs,' '); \
        } \
    } else { \
        params->padding=0; \
        if( !((outwhat&alPrintTailDisplayTail) && !(outwhat&alPrintTailDisplayAlignment) ) ) \
            leftTail = 0; \
    } \
    if(params->leftTailPrint && leftTail > params->leftTailPrint && (outwhat&alPrintTailDisplayTail) ){ \
        for(; iqLeft< leftTail ; ++iqLeft ) { \
            iqx=( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) ? (qrybuflen-1-(qStart+iqLeft)) : (qStart+iqLeft); \
            char chQf=sBioseq::mapRevATGC[(idx)sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)]; \
            char chQr=sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)]]; \
            char chQ=( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) ? chQr : chQf ; \
            if( (outwhat&alPrintNs) && seqQryQua && !sBioseq::Qua(seqQryQua ,iqx ,isQryQuaBit) ) { \
                chQ = 'N'; \
            } \
            l3.printf("%c",(outwhat&alPrintUpperCaseOnly) ? toupper(chQ) : tolower(chQ)); \
            ++prtChars; \
        } \
        if( qStart+m[1] && !(outwhat&alPrintTailDisplayAlignment) ) \
            l3.printf("..."); \
    } \
}

#define printAlignmnetSingleRightTail \
{ \
    idx rightTail = qrybuflen - al->getQueryEnd_uncompressed(m); \
    if(params->rightTailPrint && rightTail >= params->rightTailPrint && (outwhat&alPrintTailDisplayTail) ){ \
        iqx = ( flags&sBioseqAlignment::fAlignBackward ) ? (qrybuflen-1-(qStart+iq)): (qStart+iq); \
        if(!(outwhat&alPrintTailDisplayAlignment) ) \
            l3.printf("..."); \
        iqRight = al->getQueryEnd_uncompressed(m)+1; \
        for(; iqRight<qrybuflen ; ++iqRight, ++isRight) { \
            if( params->winTailLimit ) { \
                if( rangeend && isRight>=rangeend ) \
                    break; \
            } \
            iqx=( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) ? (qrybuflen-1-(qStart+iqRight)) : (qStart+iqRight); \
            char chQf=sBioseq::mapRevATGC[(idx)sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)];\
            char chQr=sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)]]; \
            char chQ=( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) ? chQr : chQf ; \
            l3.printf("%c",(outwhat&alPrintUpperCaseOnly) ? toupper(chQ) : tolower(chQ));\
            ++prtChars; \
        } \
    } \
}
#define printAlignmnetSingleAssertPosition \
{\
    if((highlightposition>=0 || (outwhat&alPrintBasedOnRange) || (outwhat&alPrintMultiple) ) ) { \
        if ( (outwhat&alPrintTouchingOnly) && ( highlightposition == sNotIdx ) ){ \
            if( (outwhat&alPrintExcludeDeletions) ){ \
                if( prtChars>rangeend - rangestart) { \
                    break; \
                } \
            } \
            else if ( (outwhat&alPrintExcludeInsertions) ) { \
                if(pstChars>rangeend - rangestart ) { \
                    break; \
                } \
            } \
        } \
        else if (isx>rangeend){ \
            break; \
        } \
    } \
}
#define printAlignmnetSingleAssertRange \
{\
    idx  sEnd=al->subStart()+m[2*(al->lenAlign()-1)] ; \
    if((outwhat&alPrintMode) &&( (highlightposition!=sNotIdx || (outwhat&alPrintBasedOnRange) ) && (rangeend<sStart || rangestart>=sEnd))) return 0; \
    if (outwhat & alPrintTouchingOnly) { \
        if( highlightposition == sNotIdx ) { \
            if( !(rangeend <= sEnd && rangestart >= sStart) ) { \
                return 0; \
            } \
        } else if( highlightposition < sStart || highlightposition > sEnd ) { \
            return 0; \
        } \
    } \
}
#define printAlignmnetSingleAssertFilters \
{\
    if ( (outwhat&alPrintTouchingOnly) && ( highlightposition == sNotIdx )) { \
        if( (outwhat&alPrintExcludeDeletions) && prtChars-1<rangeend-rangestart) { \
            return 0; \
        } \
        if( (outwhat&alPrintExcludeInsertions) && pstChars-1<rangeend-rangestart) { \
            return 0; \
        } \
    } \
    if(!prtChars && !pstChars && !(outwhat&alPrintMutBiasOnly)) \
        return 0; \
    if((outwhat&alPrintVariationOnly) && !isNotSide)return 0; \
    if((outwhat&alPrintNonPerfectOnly) && !isNonPerfect)return 0; \
}
//static
idx sBioal::printAlignmentSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    idx qrybuflen=bioal->Qry->len(al->idQry()), idSub=al->idSub(), idQry=al->idQry(), sStart=al->subStart()+m[0], qStart=al->qryStart(), flags=al->flags() , rangeend=params->rangeend,rangestart=params->rangestart,
        highlightposition=params->High, outwhat=params->navigatorFlags, curChunk=(params->currentChunk<<32)&0xFFFFFFFF, nonCompFlags=(flags)&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement));
    sStr tSub,tQry,l1, hdl1, l1comp, hdl1comp, l2, hdl2, l3, hdl3, l3comp, hdl3comp,* out=params->str, l4,hdl4,l5,hdl5,l6,l7; // for qualities
    sVec < idx > uncompressMM, * entr=(sVec < idx > *)params->userPointer;
    idx prtChars=0, pstChars = 0;

    const char * ids=idSub>=0?bioal->Sub->id(idSub):0;if(ids && *ids=='>')++ids;
    sString::searchAndReplaceSymbols(&tSub, ids, 0, ","," ",0,true,true,false,false);
    const char * idq=bioal->Qry->id(idQry);if(idq && *idq=='>')++idq;
    sString::searchAndReplaceSymbols(&tQry, idq, 0, ","," ",0,true,true,false,false);

    const char * sub=idSub>=0?bioal->Sub->seq(idSub):0, * qry=bioal->Qry->seq(idQry), * seqSubQua = idSub>=0?bioal->Sub->qua(idSub):0, * seqQryQua = bioal->Qry->qua(idQry);
    bool isSubQuaBit = idSub>=0?bioal->Sub->getQuaBit(idSub):0;
    bool isQryQuaBit = bioal->Qry->getQuaBit(idQry);

    if(flags&sBioseqAlignment::fAlignCompressed){ // deal with compressed alignments
        uncompressMM.resize(2*al->lenAlign());
        sBioseqAlignment::uncompressAlignment(al,m,uncompressMM.ptr());
        m=uncompressMM.ptr();
    }

    printAlignmnetSingleAssertRange
    params->indels=0;

    //check size of sVec holding the entropy of highlighted position.
    if(!(outwhat&alPrintSubject) && (outwhat&alPrintMutBiasOnly)){
        entr->resize(4*(qrybuflen+1));
    }
    if(params->id){
        out->printf("%s[%"DEC"]<->Query[%"DEC"] : score=%"DEC"\n",
            params->id , idSub,idQry,al->score());
    }
    idx isx=0, iqx=0,i=0, is=0,isS=0,isL=0,isQ=-1,iq=0, iqnn=0, isnn=0,iqHigh=-1, isNotSide=0, isRightSide=0, isNonPerfect=0,stringHigh=0,intermHigh=0,callLetterHigh=0,iqRight=0,isRight=0;

    char chS='N',chQ='N';
    for( i=0 ; i<2*al->lenAlign(); i+=2) {
        isx=(al->subStart()+m[i]);
        iqx = al->qryStart() + m[i+1];
        if(iqx>isQ)isQ = iqx;
        if(m[i]>=0){isL=isx;}
        else if ( outwhat&alPrintSequenceOnly ) {++isL;}

        if(!((highlightposition>=0 || (outwhat&alPrintBasedOnRange) || (outwhat&alPrintMultiple)) && (isL<rangestart || (!(outwhat&alPrintSubject) && m[i]<0 && (!(outwhat&alPrintSequenceOnly)) ))))
            break;
    }
    params->subRealS=isS=isx;

    printAlignmnetSingleLeftColumns
    printAlignmnetSinglePaddingAndLeftTail

    if( (outwhat&alPrintTailDisplayTail) && !(outwhat&alPrintTailDisplayAlignment) )
        highlightposition = 0;

    bool isPrintQry = false;
    if( (outwhat&alPrintQuery) && !((outwhat&alPrintFilterTail) && !(outwhat&alPrintTailDisplayAlignment) ) ) {
        isPrintQry = true;
    }
    for( ;i<2*al->lenAlign(); i+=2 ) {
        is=m[i];
        iq=m[i+1];
        if(iq>0)iqRight=iq;
        isx=(al->subStart()+is);

        if(is>=0){isL=isx; isRight=isx;}
        else if ( outwhat&alPrintSequenceOnly ) {++isL;}

        printAlignmnetSingleAssertPosition
        if(isx>=0)isnn=isx;

        if( !(outwhat&alPrintMultiple) ){
            chS=( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine) ) ? sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(sub, isx, nonCompFlags)]] : sBioseq::mapRevATGC[(idx)sBioseqAlignment::_seqBits(sub, isx,nonCompFlags)] ;
            if( (outwhat&alPrintNs) && seqSubQua && !sBioseq::Qua(seqSubQua ,isx ,isSubQuaBit) )
                chS = 'N';
        }

        if( iq <0 ) {
            chQ='-';
        } else {
            if ( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) {
                iqx = qrybuflen-1-qStart-iq;
                chQ = sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)]];
            } else {
                iqx = qStart+iq;
                chQ = sBioseq::mapRevATGC[(idx)sBioseqAlignment::_seqBits(qry, iqx, nonCompFlags)];
            }
            if(isQ<0) isQ = iqx;
            if( (outwhat&alPrintNs) && seqQryQua && !sBioseq::Qua(seqQryQua ,iqx ,isQryQuaBit) )
                chQ = 'N';
        }

        if(iqx>=0 && (outwhat&alPrintNonFlippedPosforRev))iqnn=qStart+iq;
        else iqnn=iqx;

        if(highlightposition!=sNotIdx && highlightposition==isx){
            if(outwhat&alPrintVariationOnly){if( chS==chQ ) return 0;}
            callLetterHigh=chQ;
        }

        if( outwhat&alPrintSubject ){
            if( !(is<0 && (outwhat&alPrintExcludeInsertions) ) ){
                if(is<0){
                    l1.printf("-");
                    if(highlightposition!=sNotIdx && (isL + 1) <=highlightposition)
                        ++params->indels;//Accumulate subject insertions
                }
                else if(!(outwhat&alPrintIgnoreCaseMissmatches))
                    l1.printf("%c",(iq<0 || (!isSameBase(chS,chQ) && chS!='N') ) ? tolower(chS) : toupper(chS) ) ;
                else
                    l1.printf("%c", toupper(chS) );
                ++pstChars;
            }
        }
        if( ((outwhat&(alPrintUpperInterm|alPrintLowerInterm)) ) && !(outwhat&alPrintDotsFormat) )
        {
            if( !( (iq<0 && (outwhat&alPrintExcludeDeletions) ) || (is<0 && (outwhat&alPrintExcludeInsertions) ) ) ) {
                if(is<0)l2.printf("-");
                else if(iq<0) l2.printf(".");
                else if( isSameBase(chS,chQ) ) {
                    if( params->alignmentStickDirectional==1 ) {
                        if( flags&sBioseqAlignment::fAlignBackward )
                            l2.printf("<");
                        else
                            l2.printf(">");
                    }
                    else
                        l2.printf("|");
                }
                else l2.printf(" ");
            }
        }

        if( isPrintQry ){
            if( !(iq<0 && (outwhat&alPrintExcludeDeletions) ) ) {
                if(outwhat&alPrintDotsFormat)
                    l3.printf("%c",iq>=0 ? ( (is<0 || !isSameBase(chS,chQ)) ? toupper(chQ) : '.') : '-' );
                else if(!(outwhat&alPrintIgnoreCaseMissmatches))
                    l3.printf("%c",iq>=0 ? ( ( !(outwhat&alPrintMultiple) && (is<0 || ( !isSameBase(chS,chQ) && chQ!='N') ) ) ? tolower(chQ) : toupper(chQ) ) : '-' );
                else
                    l3.printf("%c",iq>=0 ? ( toupper(chQ) )  : '-' );
                ++prtChars;
                if(params->wrap && !(prtChars%params->wrap)) {
                    l3.printf("\n");
                }
            }
        }

        if(highlightposition!=sNotIdx && highlightposition==isx){
            if(outwhat&alPrintVariationOnly){if( isSameBase(chS,chQ) ) return 0;}
            if(iqx>=0 && (outwhat&alPrintNonFlippedPosforRev))iqHigh=qStart+iq;
            else iqHigh=iqx;
            ++isNotSide;
        }

        if( (is<0 || iq<0 || !isSameBase(chS,chQ)) && !isNonPerfect)
            ++isNonPerfect;
    }

    if((outwhat&alPrintFilterTail) && !(outwhat&alPrintTailDisplayAlignment) ) {
        idx qryalstart = al->getQueryStart(m), qryalend = al->getQueryEnd_uncompressed(m);
        if( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) {
            qryalstart = qrybuflen - 1 - qryalstart;
            qryalend = qrybuflen - 1 - qryalend;
        }
        l3.printf("|%"DEC"|%"DEC"| <- %"DEC" ->  |%"DEC"|%"DEC"|", al->getSubjectStart(m)+1,qryalstart+1, al->lenAlign(),qryalend+1,al->getSubjectEnd_uncompressed(m)+1);
    }

    printAlignmnetSingleRightTail
    printAlignmnetSingleAssertFilters

    intermHigh=highlightposition-isS;
    stringHigh=intermHigh+params->padding+1;

    if((outwhat&alPrintSubject) && l1 ) {
        if( !(outwhat&alPrintSequenceOnly) ){
            hdl1.printf(",%s,%"DEC"",l1.ptr(params->indels), isnn+1);
        }
        else {
            hdl1.printf("%s",l1.ptr(params->indels));
        }
    }
    if((outwhat&alPrintUpperInterm) && !(outwhat&alPrintDotsFormat) && l2 ) {
        if(!(outwhat&alPrintSequenceOnly) ) {
            hdl2.printf(",%s,%"DEC,l2.ptr(params->indels),i/2);
        }
        else {
            hdl2.printf("%s",l2.ptr(params->indels));
        }
    }
    if((outwhat&alPrintQuery) && l3 ) {
        if( !(outwhat&alPrintSequenceOnly) ) {
            hdl3.printf(",%s,%"DEC"",l3.ptr(params->indels), iqnn+1);
        }
        else {
            hdl3.printf("%s",l3.ptr(params->indels) );
        }
    }

    if( (outwhat & alPrintSubject) && l1 && !(outwhat&alPrintSequenceOnly) ) {
        hdl1.printf(",%s",tSub.ptr());
        if(highlightposition != sNotIdx&& highlightposition >= 0 ){
            hdl1.printf(",,%"DEC",%"DEC,highlightposition + 1,stringHigh);
        }
        else {
            hdl1.printf(",,");
        }
    }
    if( (outwhat&alPrintQuery) && l3 && !(outwhat&alPrintSequenceOnly) ) {
        hdl3.printf(",%s",tQry.ptr());
        hdl3.printf(",%"DEC,(outwhat & alPrintCollapseRpt)?bioal->getRpt(iAlInd):1);

        if( highlightposition != sNotIdx && highlightposition >= 0 )
            hdl3.printf(",%"DEC",%"DEC, iqHigh + 1,stringHigh);
        else
            hdl3.printf(",");

    }
    if((outwhat&alPrintUpperInterm) && !(outwhat&alPrintDotsFormat) && l2 && !(outwhat&alPrintSequenceOnly) ) {
        if( highlightposition != sNotIdx && highlightposition >= 0 )
            hdl2.printf(",,,%"DEC",%"DEC, intermHigh+1,stringHigh);
        else
            hdl2.printf(",,,");
    }

    if(hdl1.ptr())out->printf("%s\n",hdl1.ptr());
    if(hdl2.ptr())out->printf("%s\n",hdl2.ptr());
    if(hdl3.ptr())out->printf("%s\n",hdl3.ptr());

    if(highlightposition>=0 && !(outwhat&alPrintSubject))
    {
        if(iqHigh+1){
            if((outwhat&alPrintMirroredPos) && iqHigh>qrybuflen/2)
                iqHigh=qrybuflen-iqHigh;
            if(entr)
                (*entr->ptr(4*iqHigh+sBioseq::mapATGC[callLetterHigh]))+=(outwhat & alPrintCollapseRpt)?bioal->getRpt(iAlInd):1;
        }
    }
    return 1;
}

idx sBioal::BioseqAlignmentComparator(void * parameters, void * A, void * B,void * objSrc,idx i1,idx i2 )
{
    ParamsAlignmentSorter * param = (ParamsAlignmentSorter *)parameters;
    if(param->extComparator){
//        sSort::sCallbackSorterSimple compare=(sSort::sCallbackSorterSimple*)param->extComparator;
        return param->extComparator(param->extCompParams, A,  B, objSrc, i1, i2);
    }
    sBioseqAlignment::Al * hdrA= (sBioseqAlignment::Al * )A; sBioseqAlignment::Al * hdrB=(sBioseqAlignment::Al * )B;
    idx compVal1=0,compVal2=0,compId1=0,compId2=0;
    idx * m1=param->bioal->getMatch(i1);
    idx * m2=param->bioal->getMatch(i2);


    if(param->flags&sBioal::alSortByPosStart)
    {
        compVal1=hdrA->subStart()+ m1[0];
        compVal2=hdrB->subStart()+ m2[0];
        //if(!(param->flags^sBioal::alSortByPosStart))
        if(!(param->flags^sBioal::alSortByPosStart))
            return (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1)  : 0 ;
    }
    else if(param->flags&sBioal::alSortByPosEnd){
        compVal1=hdrA->subStart()+m1[0]+1+hdrA->lenAlign();
        compVal1=hdrB->subStart()+m2[0]+1+hdrB->lenAlign();
        if(!(param->flags^sBioal::alSortByPosEnd))
            return (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1)  : 0 ;
    }

    if(param->flags&sBioal::alSortBySubID)
    {
        compId1=hdrA->idSub();
        compId2=hdrB->idSub();
        if(!(param->flags^sBioal::alSortBySubID))
            return (compId1 != compId2)? ( (compId1 > compId2) ? 1 : -1)  : 0 ;
    }
    else if(param->flags&sBioal::alSortByQryID)
    {
        compId1=hdrA->idQry();
        compId2=hdrB->idQry();
        if(!(param->flags^sBioal::alSortByQryID))
            return (compId1 != compId2)? ( (compId1 > compId2) ? 1 : -1)  : 0 ;
    }

    if(param->flags& alSortByPosONSubID)
        return (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1)  : ( (compId1 != compId2)? ( (compId1 > compId2) ? 1 : -1):0) ;
    else if(param->flags&alSortBySubIdONPos)
        return (compId1 != compId2)? ( (compId1 > compId2) ? 1 : -1)  : ( (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1):0) ;

    return 0;
}
