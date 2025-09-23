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


idx sBioal::remap( sBioal * mutual, sVec<idx> &remappedHits, idx * alSortList ) {

    bool mutualMode = ( mutual->Qry->getmode() != Sub->getmode() )?true:false;
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

            if(hdr->flags()&sBioseqAlignment::fAlignCompressed){
                m = sBioseqAlignment::uncompressAlignment(hdr,m);
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

            if( progress_CallbackFunction && progress_CallbackFunction(progress_CallbackParam, ii, 100 * (ii) / totAl, 100) == 0 ) {
                return 0;
            }
        }
    }
    return remappedHits.dim();
}

idx sBioal::stableRemap( sBioal * mutual, sVec<idx> &remappedHits, idx al_start, idx al_cnt, idx * alSortList, sBioseqAlignment * l_seqAl ) {
    bool subjectInDifferentlMode = ( mutual->Qry->getmode() != Sub->getmode() )?true:false;
    idx alCnt = 0 , iAl = 0, ii = 0, totAl = dimAl();
    sBioseqAlignment::Al *hdrTo, *hdr, *hdrN;
    idx * mTo, curSub, curAl;
    sVec<idx *> sub_ms;
    sVec<const char *> subs;
    sBioseqAlignment seqAl, * p_seqAl;
    if(l_seqAl)
        p_seqAl = l_seqAl;
    else {
        idx seed = 11;
        seqAl.costMatch=5;seqAl.costMismatch=-4;seqAl.costMismatchNext=-6;seqAl.costGapOpen=-12;seqAl.costGapNext=-4;
        seqAl.computeDiagonalWidth=6*seed;
        seqAl.considerGoodSubalignments=1;

        seqAl.scoreFilter=0;
        seqAl.trimLowScoreEnds=0;
        seqAl.allowShorterEnds=seqAl.minMatchLen;
        seqAl.maxExtensionGaps=0;
        seqAl.hashStp=seed;
        seqAl.bioHash.hashStp=1;

        p_seqAl = &seqAl;
    }
    for(idx iSub = 0 ; iSub < mutual->Qry->dim() ; ++iSub ) {
        hdrTo = mutual->getAl( iSub );
        sub_ms.vadd(1,mutual->getMatch( iSub ));
        subs.vadd(1,mutual->Qry->seq(hdrTo->idQry()));

    }
    idx substart = 0, subend = 0, sublen = 0, qrystart = 0, qryend = 0, qrylen = 0;
    sVec < idx > uncompressMM;
    sVec<idx> subjectsCovered(sMex::fSetZero|sMex::fExactSize);
    idx subdim = Sub->dim();

    if(!al_cnt)al_cnt=totAl;
    if(subjectInDifferentlMode) {
        subjectsCovered.resize(Sub->getlongCount());
        subdim = subjectsCovered.dim();
    }
    for(idx iSub = 0 ; iSub < subdim ; ++iSub )
    {
        hdrTo = mutual->getAl( iSub );
        mTo = mutual->getMatch( iSub );

        if( subjectInDifferentlMode ) {
            if( sBioseq::isBioModeLong(mutual->Qry->getmode()) ) {
                curSub = mutual->Qry->long2short(hdrTo->idQry());
                if( subjectsCovered[curSub] )
                    continue;
                ++subjectsCovered[curSub];
            } else {
                curSub = iSub;
                idx imS = 0;
                while( imS<mutual->dimAl() && mutual->getAl(imS)->idQry()!= Sub->long2short(iSub) )
                    ++imS;
                if(imS>=mutual->dimAl())return 0;
                hdrTo = mutual->getAl(imS);
                mTo = mutual->getMatch(imS);
            }
        } else {
            hdrTo = mutual->getAl( iSub );
            mTo = mutual->getMatch( iSub );
            curSub = hdrTo->idQry();
        }
        iAl = listSubAlIndex( curSub, &alCnt);
        if( !iAl ) {
            continue;
        }
        if( ii + alCnt - 1 < al_start ) {
            ii += alCnt;
            continue;
        }
        else if( ii >= (al_start + al_cnt) )
            break;

        iAl -=1 ;
        for(idx i = 0; i < alCnt; ++i,++iAl, ++ii) {
            if(ii<al_start)
                continue;
            if( ii >= (al_start + al_cnt) )
                break;
            curAl = iAl;
            if(alSortList) {
                curAl = alSortList[iAl];
            }
            hdr=getAl(curAl);
            idx * qry_m = getMatch(curAl);
            substart = sBioseqAlignment::remapQueryPosition(hdrTo, mTo,hdr->getSubjectStart(qry_m),1);
            subend = sBioseqAlignment::remapQueryPosition(hdrTo, mTo,hdr->getSubjectEnd(qry_m),1);
            sublen = subend - substart + 1;
            qrystart = hdr->getQueryStart(qry_m);
            qryend = hdr->getQueryEnd(qry_m);
            qrylen = qryend - qrystart + 1;
            const char * qryseq = Qry->seq(hdr->idQry());
            hdrN = p_seqAl->alignSWProfile(remappedHits, subs.ptr(),sub_ms.ptr(),substart,sublen,subs.dim(),&qryseq,0,Qry->len(hdr->idQry()),qrystart,qrylen,1,hdr->flags()|sBioseqAlignment::fAlignGlobal);
            if(hdrN) {
                hdrN->ids = hdr->ids;
                hdrN->setFlags(hdr->flags());
            }
            if( progress_CallbackFunction && progress_CallbackFunction(progress_CallbackParam, ii - al_start, 100 * (ii - al_start) / al_cnt, 100) == 0 ) {
                return 0;
            }
        }
    }
    return remappedHits.dim();
}

idx sBioal::getConsensus(sStr &out, idx wrap, idx mode) {
    idx iVis = 0;
    sVec <idx> resVec(sMex::fSetZero);
    ParamsAlignmentIterator params;
    params.userPointer = &resVec;
    params.navigatorFlags = mode;
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

idx sBioal::bSearchAlignments(idx iAlPoint,idx iAlmin, idx iAlmax, sSort::sSearchHitType hitType){
    return sSort::binarySearch((sSort::sCallbackSearchSimple)rngComparator,0,&iAlPoint,iAlmax,this,0,hitType,false,iAlmin);
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
    idx tot=0;
    bool isQ = (params->navigatorFlags&alPrintQualities);
    idx tot_done = (cnt>qryDim)?qryDim:cnt;
    bool printRepeats = !(params->navigatorFlags & alPrintCollapseRpt);
    for(idx iAl=0;iAl<qryDim && *piVis<start+cnt ;++iAl){

        if( progress_CallbackFunction && progress_CallbackFunction(progress_CallbackParam, iAl, 100 * ((cnt > qryDim) ? iAl : *piVis) / tot_done, 100) == 0 ) {
            return 0;
        }

        if(!(QFaiBM[iAl/8]&(((idx)1)<<(iAl%8)))){
            ++*piVis;
            if(*piVis<start)continue;

            Qry->printFastXRow(params->str, isQ, iAl, 0, 0, 0, true, false, 0, 0, sBioseq::eSeqForward, true, 0, true,false,printRepeats);

            tot+= printRepeats?Qry->rpt(iAl):1;
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
                    out.printf(",%" DEC ",%" DEC, rgs + 1, rge);
                    loop = false;
                } else {
                    rg_ofs += rgs + 1;
                }
            } else {
                loop = false;
                out.printf(",%" DEC ",%" DEC, rgs + 1, rge);
            }
        }
        else{
            loop = false;
        }
    }
    return matchS;
}

idx isPrimeFilter( sBioal::ParamsAlignmentIterator * callbackParam )
{
    if(callbackParam->rangestart!=callbackParam->rangeend && !(callbackParam->navigatorFlags&sBioal::alPrintMultiple) )
        return true;
    if( callbackParam->navigatorFlags&sBioal::alPrintFilterTail )
        return true;
    if( !(callbackParam->navigatorFlags&sBioal::alPrintForward) != !(callbackParam->navigatorFlags&sBioal::alPrintReverse))
        return true;
    if(callbackParam->navigatorFlags&sBioal::alPrintRepeatsOnly)
        return true;
    if(callbackParam->navigatorFlags&sBioal::alPrintTouchingOnly)
        return true;
    return false;
}

idx sBioal::iterateAlignments(idx * piVis, idx start, idx cnt, idx iSub, typeCallbackIteratorFunction callbackFunc, ParamsAlignmentIterator * callbackParam, typeCallbackIteratorFunction secondaryCallbackFunc, ParamsAlignmentIterator * secondaryCallbackParam, sVec<idx> * sortArr)
{
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
        dif = callbackParam->rangeend - callbackParam->rangestart + 1;
        start *=dimAl();
    }

    if ( !secondaryCallbackFunc && (callbackParam->navigatorFlags&alPrintReadsInFasta) ) {
        secondaryCallbackFunc = printFastXSingle;
    }


    if( iSub==sNotIdx ) {iterUnAligned(piVis,start,cnt,callbackParam);return 1;}
    else if (iSub < 0) alCnt = dimAl();
    else if(iSub<Sub->dim() ) {
        alListIndex=listSubAlIndex(iSub, &alCnt)-1;
        if( alListIndex < 0 )
            alListIndex = 0;
    }
    else return 0;
    idx res=0, buflenBefore;

    bool primaryFilters=isPrimeFilter(callbackParam);
    bool secondaryFilters=(callbackParam->regp || secondaryCallbackFunc)? true : false;

    idx iAlStart=0,iAlEnd=callbackParam->navigatorFlags&alPrintMultiple?dimAl():alCnt;
    if(primaryFilters && callbackParam->rangestart!=callbackParam->rangeend && alListIndex){
        idx iAlRangeStart=callbackParam->rangestart-(callbackParam->maxAlLen?callbackParam->maxAlLen:callbackParam->winSize);
        iAlRangeStart=iAlRangeStart<0?0:iAlRangeStart;
        iAlStart = bSearchAlignments(iAlRangeStart, alListIndex, alListIndex + iAlEnd, sSort::eSearch_First);

        bool hasFoundAlStart = true;
        if( iAlStart < 0 ) {
            idx window_step = (callbackParam->maxAlLen ? callbackParam->maxAlLen : callbackParam->winSize / 2);
            idx moved_iAlRangeStart = iAlRangeStart + window_step;
            while( iAlStart && moved_iAlRangeStart < callbackParam->rangeend ) {
                iAlStart = bSearchAlignments(moved_iAlRangeStart, alListIndex, alListIndex + alCnt, sSort::eSearch_First);
                moved_iAlRangeStart += window_step;
            }
            if( iAlStart < 0 ) {
                iAlStart = 0;
                hasFoundAlStart = false;
            }
        }
        if(callbackParam->rangeend<(getAl(alListIndex+iAlEnd-1)->getSubjectStart(getMatch(alListIndex+iAlEnd-1)))) {
            iAlEnd=bSearchAlignments(callbackParam->rangeend,iAlStart,alListIndex+iAlEnd,sSort::eSearch_Last)+1;

            if (iAlEnd == 0 && hasFoundAlStart) {
                iAlEnd = iAlStart + 1;
            }
        }
        alListIndex = 0;
    }

    if(!secondaryFilters && cnt!=sIdxMax)++cnt;

    idx * randInds=0;
    sVec<idx> randIndArr;
    if(sortArr) {
        randInds = sortArr->ptr();
        iAlEnd = iAlStart + sortArr->dim();
    } else if( !randInds && !(callbackParam->navigatorFlags & alPrintMultiple) && ((callbackParam->navigatorFlags & (alPrintInRandom)) || !(callbackParam->navigatorFlags & alPrintCollapseRpt)) ) {
        idx t_ia = 0, t_rpts = 0, i_add = 0;
        randIndArr.resize(iAlEnd - iAlStart);
        for(; iAlStart < iAlEnd; ++iAlStart) {
            t_ia = alListIndex ? alListIndex + iAlStart : iAlStart;
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
    idx ia=iAlStart, iAlStart2End = iAlEnd - iAlStart;
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
            iAl=alListIndex ? alListIndex+ia : ia ;
        }
        sBioseqAlignment::Al *  hdr=getAl(iAl);
        idx * m=getMatch(iAl);


        bool isok=true;
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
                if( qs<callbackParam->leftTailPrint && (Qry->len(hdr->idQry()) -1 - qe < callbackParam->rightTailPrint) )
                    isok=false;
            }

            if((callbackParam->navigatorFlags&alPrintTouchingOnly) && callbackParam->High >= 0 && (alStart>callbackParam->High || alEnd<callbackParam->High))
                isok=false;
            if( !(callbackParam->navigatorFlags&sBioal::alPrintForward) != !(callbackParam->navigatorFlags&sBioal::alPrintReverse) ){
                if( ( (callbackParam->navigatorFlags&sBioal::alPrintForward) && hdr->isReverseComplement() ) || ((callbackParam->navigatorFlags&sBioal::alPrintReverse) && hdr->isForward()) ){
                    isok=false;
                }
            }
        }

        if( isok && secondaryFilters ) {
            res=callbackFunc(this,callbackParam, hdr, m ,iAlCnt-iAlStart, iAl);
            if(!res)isok=false;

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
                    sString::extractSubstring(&compStr,hdl1,0,callbackParam->alCol,"," __,false,false);
                    matchS = regexAlignmentSingle(compStr, hdl1, callbackParam->High - added_padd, callbackParam->High - added_padd, callbackParam);

                }
                if((callbackParam->navigatorFlags&alPrintRegExpInt) && hdl2){
                    sStr compStr;
                    sString::extractSubstring(&compStr,hdl2,0,callbackParam->alCol,"," __,false,false);
                    matchU = regexAlignmentSingle(compStr, hdl2, callbackParam->High - added_padd, callbackParam->High - added_padd, callbackParam);
                }
                if((callbackParam->navigatorFlags&alPrintRegExpQry) && hdl3){
                    sStr compStr;
                    sString::extractSubstring(&compStr,hdl3,0,callbackParam->alCol,"," __,false,false);
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
        if( isok ) {
            if(piVis){
                ++(*piVis);
                if(*piVis<= start){
                    isok=false;
                }
                if(*piVis>=start+cnt){
                    break;
                }
            }
        }

        if( isok && !secondaryFilters ) {

            res=callbackFunc(this,callbackParam, hdr, m, callbackParam->navigatorFlags&alPrintMultiple?(iFound/dimAl()):(ia-iAlStart), iAl);
        }
        else {
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

        if ( progress_CallbackFunction && iAlCnt % 1000 == 0 && progress_CallbackFunction(progress_CallbackParam, iAlCnt, (100* iAlCnt ) / iAlEnd, 100) == 0 ) {
            return 0;
        }
    }


    return iFound;
}


idx sBioal::printAlignmentSummaryBySubject(slib::sVec < sBioal::Stat > & statistics, slib::sStr * str, sBioal::ParamsAlignmentSummary * sumParams)
{
    sStr hdr, tmpBody, taxid;
    const char * taxInfo = 0;

    hdr.printf(0,"id,Reference,Hits,Hits Unique,Length");
    if(sumParams->coverage){
        hdr.printf(",RPKM,CPM,Percentage,Density");
    }

    bool isPE = isPairedEnd();
    if( isPE ) {
        hdr.printf(",FPKM");
    }
    bool taxonomyInfo = sumParams->taxion ? true : false;
    if (taxonomyInfo){
        hdr.printf(",taxID,taxNAME");
    }

    sDic <idx> ionHdrDic;
    if(!sumParams->callBackExtension){
        hdr.printf("\n");
    }

    if (sumParams->cnt > Sub->dim()) {
        sumParams->reportTotals=true;
    }
    sIO buf;

    idx tot=0,totRpt=0, total_unique_aligned_reads = 0, len=0,iVis=0,buflenBefore=0,showTot=0,showTotRpt=0,showLen=0,tot_coverage = 0,show_coverage = 0;
    real curRPKM = 0, curFPKM = 0;
    sStr tmp, escapeCSV_buf;bool printOther=false;

    sVec<idx> statHits,sortedInd;
    statHits.resize(Sub->dim());sortedInd.add(Sub->dim());
    for(idx is=0; is<Sub->dim(); ++is ) {
        statHits[is]=statistics[is+1].foundRpt;
        totRpt += statHits[is];
        total_unique_aligned_reads += statistics[is+1].found;
    }
    idx total_unique_reads = total_unique_aligned_reads + statistics[0].found;
    sSort::sort(statHits.dim(), statHits.ptr(), sortedInd.ptr());

    if(sumParams->reportFailed ){
        ++iVis;
        tmpBody.printf("%" DEC ",\"%s\",%" DEC ",%" DEC ",",(idx)0,"Unaligned", statistics[0].foundRpt , statistics[0].found );
        if(sumParams->coverage){
            tmpBody.printf(",-,-,%.2f,-", 100*(float)statistics[0].found/total_unique_reads);
        }
        if (taxonomyInfo){
            tmpBody.printf(",-,-");
        }
        tmpBody.printf("\n");
        --sumParams->cnt;
    }

    if(sumParams->reportTotals)--sumParams->cnt;

    for(idx is=Sub->dim()-1; is>=0; --is ) {
        idx srtID=sortedInd[is];

        if(!sumParams->reportZeroHits && !statistics[srtID+1].found){
            continue;
        }
       if(sumParams->callBackExtension ) {
             buf.cut(0);
             sDic <sStr > dic;
             sStr tmpHdrForIon;
             sumParams->callBackExtension(sumParams->param, &buf,0, &dic,Sub->id(srtID),1,0,1,0, &tmpHdrForIon);
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
        if( sumParams->coverage && sumParams->coverage->dim() > srtID) {
            cur_cov = sumParams->coverage->ptr(srtID);
            if(cur_cov){
                tot_coverage += *cur_cov;
            }
        }

        tot+=statistics[srtID+1].found;
        curRPKM = (statistics[srtID+1].foundRpt*1000000000.)/Sub->len(srtID)/totRpt;
        if(isPE) {
            curFPKM = (statistics[Sub->dim()+srtID+1].foundRpt*1000000000.)/Sub->len(srtID)/totRpt;
        }

        if (taxonomyInfo){
            taxid.cut(0);
            taxInfo = sumParams->taxion->extractTaxIDfromSeqID(&taxid, 0, 0, Sub->id(srtID), 0, "NO_INFO");
            if (strncmp(taxInfo,"NO_INFO", 7) != 0){
                const char *buf = sumParams->taxion->getTaxIdInfo(taxid.ptr(0), taxid.length(), _);
                taxid.add(",",1);
                sString::escapeForCSV(taxid, sString::next00(buf,3));
            }
            else {
                taxid.addString(",NO_INFO");
            }

        }
        if(sumParams->processedSubs->dim() != 0){
            idx imax=sumParams->processedSubs->dim(),imin=0,isThere=0;
            while( imax >= imin ) {
                idx imid =  imin + (imax -imin) /2;
                if( *sumParams->processedSubs->ptr(imid) < srtID+1 )
                    imin = imid + 1;
                else if( *sumParams->processedSubs->ptr(imid) > srtID+1 )
                    imax = imid - 1;
                else{
                    isThere=1;break;
                }
            }
            if(!isThere)continue;
        }

        ++iVis;
        if(iVis<=sumParams->start && sumParams->cnt>0){printOther=true;continue;}
        if(iVis>sumParams->start+sumParams->cnt && sumParams->cnt>0){
            printOther=true;
            if (sumParams->reportTotals) {
                continue;
            }
            else {
                break;
            }
        }
        buflenBefore = tmpBody.length();
        regmatch_t pmatch1[1];
        if(iVis<=sumParams->start+sumParams->cnt || sumParams->cnt<=0){
            escapeCSV_buf.cut0cut();
            sString::escapeForCSV(escapeCSV_buf, Sub->id(srtID));
            tmp.printf(0,"%" DEC ",%s,%" DEC ",%" DEC ",%" DEC,srtID+1, escapeCSV_buf.ptr() ,statistics[srtID+1].foundRpt,statistics[srtID+1].found,Sub->len(srtID));
            if(cur_cov){
                tmp.printf(",%.1f,%.1f,%.2f,%.1f",curRPKM,1000000*(float)statistics[srtID+1].found/total_unique_aligned_reads,100*(float)statistics[srtID+1].found/total_unique_reads,(real)(*cur_cov)/Sub->len(srtID) );
            }
            if(isPE) {
                tmp.printf(",%.1f",curFPKM );
            }
            if (taxonomyInfo){
                tmp.printf(",%s", taxInfo );
            }
            if(sumParams->callBackExtension && buf.length()){
                tmp.add(",",1);
                tmp.add(buf.ptr(),buf.length());
            }
            else tmp.printf("\n");
        }
        if(sumParams->regp && iVis<=sumParams->start+sumParams->cnt && sumParams->cnt>0)
            if(!(regexec(sumParams->regp, tmp, 1, pmatch1, 0)==0)){--iVis;printOther=true;continue;}

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
        str->cut(0);sumParams->reportTotals=false;
    }
    if(sumParams->reportTotals){
        if( (printOther || sumParams->processedSubs) && (len-showLen) && showLen ){
            tmpBody.printf("+,\"%s\",%" DEC ",%" DEC ",%" DEC,"other",totRpt-showTotRpt,tot-showTot,len-showLen);
            if(sumParams->coverage){
                tmpBody.printf(",-,%.1f,%.2f,%" DEC,1000000*(float)(total_unique_aligned_reads-showTot)/total_unique_aligned_reads,100*(float)(total_unique_reads-showTot)/total_unique_reads, (tot_coverage-show_coverage)/(len-showLen));
            }
            if (taxonomyInfo){
                tmpBody.printf(",-,-");
            }
            tmpBody.printf("\n");
        }
        tmpBody.printf("+,\"%s\",%" DEC ",%" DEC ",%" DEC "", "total" ,totRpt,tot, len);
        if(sumParams->coverage){
            tmpBody.printf(",-,%" DEC ",%" DEC ",%.1f",(idx)1000000,(idx)100,(real)tot_coverage/len);
        }
        if (taxonomyInfo){
            tmpBody.printf(",-,-");
        }
        tmpBody.printf("\n");
    }

    if (sumParams->callBackExtension) {
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

    out->printf("position,All Reads,Aligned Reads,Unaligned Reads,Alignments,Left Incomplete Alignments,Right Incomplete Alignments\n");
    for( idx ihH=0; ihH<lenHistogram->dim(); ++ihH ){
        idx ih=idxArr[ihH];
        sBioal::LenHistogram  * ll=lenHistogram->ptr(ih);
        idx * ilen=(idx*)lenHistogram->id(ih);

        out->printf("%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC "\n",*ilen,ll->cntRead,ll->cntSeq, ll->cntFailed, ll->cntAl, ll->cntLeft, ll->cntRight);
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
        for(idx is=0; is<size; ++is ) {
            statistics[is].found+=pS[is].found;
            statistics[is].foundRpt+=pS[is].foundRpt;
        }
    }
    return cnt;
}

sBioal::Stat sBioal::getTotalAlignmentStats()
{
    Stat statistics;
    idx size;

    for( idx it=0, ct=dimStat(); it<ct; ++it ){
        Stat * pS=getStat(it,0,&size);
        for(idx is=1; is<size; ++is ) {
            statistics.found+=pS[is].found;
            statistics.foundRpt+=pS[is].foundRpt;
        }
    }
    return statistics;
}

sBioal::Stat sBioal::getSubjectAlignmentStats(idx iSub)
{
    Stat statistics, * pS;

    for( idx it=0, ct=dimStat(); it<ct; ++it ){
        pS=getStat(it,iSub);
        statistics.found+=pS->found;
        statistics.foundRpt+=pS->foundRpt;
    }
    return statistics;
}

idx sBioal::countAlignmentLetters(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAl)
{
    sVec<idx> * vec = (sVec<idx> *)params->userPointer;
    idx ind = 0, is = 0, iqx =0, alphabetSize = 5;

    idx qrybuflen=bioal->Qry->len(al->idQry()), sStart=al->getSubjectStart(m);

    while(is < sStart) {
        ++ind;
        vec->resize(++is*alphabetSize);
        (*(vec->ptr((is-1)*alphabetSize+4)))++;
    }

    m = sBioseqAlignment::uncompressAlignment(al,m);
    const char * qry=bioal->Qry->seq(al->idQry());
    for( idx i=0 ; i<al->lenAlign(); i++) {
        is = al->getSubjectPosition(m, i);
        if (is >= 0) {
            ++ind;
            vec->resize((is+1)*alphabetSize);
        }
        else {
            continue;
        }
        iqx = al->getQueryPosition(m, i, qrybuflen);
        if( iqx==-1 ) {
            (*(vec->ptr(is*alphabetSize+4)))++;
        }
        else {
            (*(vec->ptr(is*alphabetSize+al->getQueryLetterByPosition(iqx,qry))))++;
        }
    }
    return ind;
}

idx sBioal::getSaturation(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd) {

    sDic<idx> * dicSubHits = (sDic<idx> *)params->userPointer;

    if( params->currentChunk > 0 ) {
        Stat * c_stat = bioal->getStat(0,hdr->idSub() + 1 );
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
        buf.printf(0,"bin-%" DEC,(params->userIndex/params->wrap)+1);
        ( *(dicSubHits->setString( buf.ptr() )) ) = cntSubs;
        if( params->userIndex > iNum ) {
            params->userIndex = iNum;
            break;
        }
    }

    ++params->userIndex;
    buf.printf(0,"%" DEC,hdr->idSub());
    (*(dicSubHits->setString( buf.ptr() )))++;

    return 1;
}

idx sBioal::printSubSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd) {
    sStr tSub;
    sString::searchAndReplaceSymbols(&tSub, bioal->Sub->id(al->idSub()), 0, ","," ",0,true,true,false,false);
    params->str->printf(
        "%" DEC ","
        "%" DEC ",\"%s\""
        ,iNum+1
        ,al->idSub()+1,tSub.ptr());
    return 1;
}

idx sBioal::printMatchSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    sStr tSub,tQry;
    sString::searchAndReplaceSymbols(&tSub, bioal->Sub->id(al->idSub()), 0, ","," ",0,true,true,false,false);
    sString::searchAndReplaceSymbols(&tQry, bioal->Qry->id(al->idQry()), 0, ","," ",0,true,true,false,false);
    m = sBioseqAlignment::uncompressAlignment(al, m);
    params->str->printf(
        "%" DEC ","
        "%" DEC ",\"%s\",%" DEC ",\"%s\","
        "%" DEC ",%" DEC ","
        "%" DEC ",%" DEC ",%" DEC ","
        "%" DEC ",%" DEC ",%" DEC ",%" DEC ","
        "%.2f,"
        "%.2lf\n"

        ,iNum+1
        ,al->idSub()+1,tSub.ptr(),al->idQry()+1, tQry.ptr()
        ,bioal->Qry->len(al->idQry()),bioal->Sub->len(al->idSub())
        ,al->score(),(al->flags()&(sBioseqAlignment::fAlignBackward) ) ? (idx)(-1) : (idx)(1), al->lenAlign()

        ,al->getSubjectStart(m)+1,al->getSubjectEnd_uncompressed(m)+1,al->getQueryStart(m)+1,al->getQueryEnd_uncompressed(m)+1
        ,al->percentIdentity(m,bioal->Qry->len(al->idQry()),bioal->Qry->seq(al->idQry()),bioal->Sub->seq(al->idSub()),bioal->Qry->qua(al->idQry()),bioal->Qry->getQuaBit(al->idQry()))
        ,(real)(100*al->countMatches(m,bioal->Qry->len(al->idQry()),bioal->Qry->seq(al->idQry()),bioal->Sub->seq(al->idSub()),bioal->Qry->qua(al->idQry()),bioal->Qry->getQuaBit(al->idQry())))/bioal->Sub->len(al->idSub())
        );

    return 1;
}

idx sBioal::printBEDSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    sStr tSub,tQry;
    sString::searchAndReplaceSymbols(&tSub, bioal->Sub->id(al->idSub()), 0, ","," ",0,true,true,false,false);
    sString::searchAndReplaceSymbols(&tQry, bioal->Qry->id(al->idQry()), 0, ","," ",0,true,true,false,false);
    params->str->printf(
        "\"%s\",%" DEC ",%" DEC ",U0,0,%s\n", tSub.ptr(), al->getQueryStart(m) + 1, al->getQueryEnd(m) + 1, (al->flags() & (sBioseqAlignment::fAlignBackward)) ? ("-") : ("+"));

    return 1;
}

idx sBioal::printFastXSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    idx iq = al->idQry();
    char appendID[64], * papID  = 0;
    if(params->navigatorFlags & alPrintCollapseRpt) {
        sprintf(appendID,"%" DEC,iNum);
        papID = &appendID[0];
    }

    return bioal->Qry->printFastXRow(params->str, (params->navigatorFlags&alPrintQualities), iq, 0, 0, 0, true, false, papID, 0, al->getDirectionality(), (params->navigatorFlags&alPrintNs), 0, (params->navigatorFlags & alPrintCollapseRpt), false, (params->navigatorFlags & alPrintCollapseRpt));
}

#define isSameBase(baseS,baseQ) (baseS!='N'&&baseQ!='N'&&baseS==baseQ)

#define printAlignmnetSingleLeftColumns \
{\
    if(outwhat&alPrintAsFasta){ \
        hdl3.printf(">%s\n",tQry.ptr()); \
    } else { \
        if( (outwhat&alPrintSubject) && !(outwhat&alPrintSequenceOnly) ) \
            hdl1.printf("%" DEC ",%5" DEC ", (%s) ,%5" DEC "",iNum|curChunk, idSub+1,((flags&sBioseqAlignment::fAlignBackward) ? "-" :"+" ),isS+1); \
        if(outwhat&(alPrintUpperInterm|alPrintLowerInterm) ){ \
           hdl2.printf("%" DEC ",,",iNum|curChunk); \
           if( params->alignmentStickDirectional==2 ){ \
                if( flags&sBioseqAlignment::fAlignBackward ) \
                    hdl2.printf("(-)"); \
                else \
                    hdl2.printf("(+)"); \
            } \
           hdl2.printf(","); \
        } \
        if( (outwhat&alPrintQuery) && !(outwhat&alPrintSequenceOnly) ) { \
            iqnn=al->getQueryPosition(m,i,qrybuflen); \
            if(!sBioseqAlignment::Al::isDeletion(m,i) && (outwhat&alPrintNonFlippedPosforRev))iqnn=al->getQueryPositionRaw(m,i); \
            hdl3.printf("%" DEC ",%5" DEC ", (%c) ,",iNum|curChunk , idQry+1,( flags&sBioseqAlignment::fAlignBackward ) ? '-' : '+'); \
            hdl3.printf("%5" DEC "",iqnn+1 ); \
        } \
    } \
}
#define printAlignmnetSinglePaddingAndLeftTail \
{\
    idx offs=0, leftTail = al->getQueryStart(m), iqLeft = 0; \
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
    if(params->leftTailPrint && leftTail >= params->leftTailPrint && (outwhat&alPrintTailDisplayTail) ){ \
        for(; iqLeft < leftTail ; ++iqLeft ) { \
            iqx = al->getQueryPosition(iqLeft - qStart,qrybuflen); \
            char chQ=al->getQueryCharByPosition(iqx,qry); \
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
        if(!(outwhat&alPrintTailDisplayAlignment) ) \
            l3.printf("..."); \
        iqRight = al->getQueryEnd_uncompressed(m)+1; \
        for(; iqRight<qrybuflen ; ++iqRight, ++isRight) { \
            if( params->winTailLimit ) { \
                if( rangeend && isRight>=rangeend ) \
                    break; \
            } \
            char chQ=al->getQueryCharByPosition(iqRight,qry); \
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
idx sBioal::printAlignmentSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd)
{
    idx qrybuflen = bioal->Qry->len(al->idQry()), idSub = al->idSub(), idQry = al->idQry(), qStart = al->qryStart(), flags = al->flags(),
        rangeend = params->rangeend, rangestart = params->rangestart,
        highlightposition = params->High, outwhat = params->navigatorFlags, curChunk = (params->currentChunk << 32) & 0xFFFFFFFF;
    sStr tSub,tQry,l1, hdl1, l1comp, hdl1comp, l2, hdl2, l3, hdl3, l3comp, hdl3comp,* out=params->str, l4,hdl4,l5,hdl5,l6,l7;
    sVec < idx > uncompressMM, * entr=(sVec < idx > *)params->userPointer;
    idx prtChars=0, pstChars = 0;

    const char * ids=idSub>=0?bioal->Sub->id(idSub):0;if(ids && *ids=='>')++ids;
    sString::searchAndReplaceSymbols(&tSub, ids, 0, ","," ",0,true,true,false,false);
    const char * idq=bioal->Qry->id(idQry);if(idq && *idq=='>')++idq;
    sString::searchAndReplaceSymbols(&tQry, idq, 0, ","," ",0,true,true,false,false);

    const char * sub=idSub>=0?bioal->Sub->seq(idSub):0, * qry=bioal->Qry->seq(idQry), * seqSubQua = idSub>=0?bioal->Sub->qua(idSub):0, * seqQryQua = bioal->Qry->qua(idQry);
    bool isSubQuaBit = idSub>=0?bioal->Sub->getQuaBit(idSub):0;
    bool isQryQuaBit = bioal->Qry->getQuaBit(idQry);

    if((flags&sBioseqAlignment::fAlignCompressed) || (outwhat&alPrintMultiple)){
        m = sBioseqAlignment::uncompressAlignment(al, m);
    }
    idx sStart=al->getSubjectStart(m), sEnd=al->getSubjectEnd_uncompressed(m);

    printAlignmnetSingleAssertRange
    params->indels=0;

    if(!(outwhat&alPrintSubject) && (outwhat&alPrintMutBiasOnly)){
        entr->resize(4*(rangeend-rangestart+1));
    }
    if(params->id){
        out->printf("%s[%" DEC "]<->Query[%" DEC "] : score=%" DEC "\n",
            params->id , idSub,idQry,al->score());
    }
    idx isx=0, iqx=0,i=0, isS=0,isL=0,isQ=-1,iqnn=0, isnn=0,iqHigh=-1, isNotSide=0, isRightSide=0, isNonPerfect=0,stringHigh=0,intermHigh=0,callLetterHigh=0,iqRight=0,isRight=0;

    char chS='N',chQ='N';
    for( i=0 ; i<al->lenAlign(); i++) {
        isx = al->getSubjectPosition(m,i);
        iqx = sBioseqAlignment::Al::getQueryIndex(m,i);
        if(!sBioseqAlignment::Al::isDeletion(m,i))isQ = al->getQueryPositionRaw(m,i);
        if(!sBioseqAlignment::Al::isInsertion(m,i))isL=al->getSubjectPosition(m,i);
        else if ( outwhat&alPrintSequenceOnly ) ++isL;

        if(!((highlightposition>=0 || (outwhat&alPrintBasedOnRange) || (outwhat&alPrintMultiple)) && (isL<rangestart || (!(outwhat&alPrintSubject) && sBioseqAlignment::Al::isInsertion(m,i) && (!(outwhat&alPrintSequenceOnly)) ))))
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
    bool isInsertion = false, isDeletion = false;
    for( ;i<al->lenAlign(); i++ ) {
        iqx=al->getQueryPosition(m,i,qrybuflen);
        isx=al->getSubjectPosition(m,i);
        isInsertion=sBioseqAlignment::Al::isInsertion(m,i);
        isDeletion=sBioseqAlignment::Al::isDeletion(m,i);

        if(!isDeletion)iqRight=sBioseqAlignment::Al::getQueryIndex(m,i);
        if(!isInsertion){isL=isx; isRight=isx;}
        else if ( outwhat&alPrintSequenceOnly ) {++isL;}

        printAlignmnetSingleAssertPosition
        if(!isInsertion)isnn=isx;

        if( !(outwhat&alPrintMultiple) ){
            chS=al->getSubjectCharByPosition(isx,sub);
            if( (outwhat&alPrintNs) && seqSubQua && !sBioseq::Qua(seqSubQua ,isx ,isSubQuaBit) )
                chS = 'N';
        }

        if( isDeletion ) {
            chQ='-';
        } else {
            chQ = al->getQueryCharByPosition(iqx, qry);
            if(isQ<0) isQ = iqx;
            if( (outwhat&alPrintNs) && seqQryQua && !sBioseq::Qua(seqQryQua ,iqx ,isQryQuaBit) )
                chQ = 'N';
        }

        if(!isDeletion && (outwhat&alPrintNonFlippedPosforRev))iqnn=al->getQueryPositionRaw(m,i);
        else iqnn=iqx;

        if(highlightposition!=sNotIdx && highlightposition==isx){
            if(outwhat&alPrintVariationOnly){if( chS==chQ ) return 0;}
            callLetterHigh=chQ;
        }

        if( outwhat&alPrintSubject ){
            if( !(isInsertion && (outwhat&alPrintExcludeInsertions) ) ){
                if(isInsertion){
                    l1.printf("-");
                    if(highlightposition!=sNotIdx && (isL + 1) <=highlightposition)
                        ++params->indels;
                }
                else if(!(outwhat&alPrintIgnoreCaseMissmatches))
                    l1.printf("%c",(isDeletion || (!isSameBase(chS,chQ) && chS!='N') ) ? tolower(chS) : toupper(chS) ) ;
                else
                    l1.printf("%c", toupper(chS) );
                ++pstChars;
            }
        }
        if( ((outwhat&(alPrintUpperInterm|alPrintLowerInterm)) ) && !(outwhat&alPrintDotsFormat) )
        {
            if( !( (isDeletion && (outwhat&alPrintExcludeDeletions) ) || (isInsertion && (outwhat&alPrintExcludeInsertions) ) ) ) {
                if(isInsertion)l2.printf("-");
                else if(isDeletion) l2.printf(".");
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
            if( !(isDeletion && (outwhat&alPrintExcludeDeletions) ) ) {
                if(outwhat&alPrintDotsFormat)
                    l3.printf("%c",!isDeletion ? ( (isInsertion || !isSameBase(chS,chQ)) ? toupper(chQ) : '.') : '-' );
                else if(!(outwhat&alPrintIgnoreCaseMissmatches))
                    l3.printf("%c",!isDeletion ? ( ( !(outwhat&alPrintMultiple) && (isInsertion || ( !isSameBase(chS,chQ) && chQ!='N') ) ) ? tolower(chQ) : toupper(chQ) ) : '-' );
                else
                    l3.printf("%c",!isDeletion ? ( toupper(chQ) )  : '-' );
                ++prtChars;
                if(params->wrap && !(prtChars%params->wrap)) {
                    l3.printf("\n");
                }
            }
        }

        if(highlightposition!=sNotIdx && highlightposition==isx){
            if(outwhat&alPrintVariationOnly){if( isSameBase(chS,chQ) ) return 0;}
            if(!isDeletion && (outwhat&alPrintNonFlippedPosforRev))iqHigh=al->getQueryPositionRaw(m,i);
            else iqHigh=iqx;
            ++isNotSide;
        }

        if( (isInsertion || isDeletion || !isSameBase(chS,chQ)) && !isNonPerfect)
            ++isNonPerfect;
        if( (isInsertion || isDeletion || !isSameBase(chS,chQ)) && entr){
            (*entr->ptr(4*(isx-rangestart)+(idx)sBioseq::mapATGC[(idx)chQ]))+=(outwhat & alPrintCollapseRpt)?bioal->getRpt(iAlInd):1;
        }

    }

    if((outwhat&alPrintFilterTail) && !(outwhat&alPrintTailDisplayAlignment) ) {
        idx qryalstart = al->getQueryStart(m), qryalend = al->getQueryEnd_uncompressed(m);
        if( (flags&sBioseqAlignment::fAlignBackward) && ((flags)&sBioseqAlignment::fAlignReverseEngine)==0 ) {
            qryalstart = qrybuflen - 1 - qryalstart;
            qryalend = qrybuflen - 1 - qryalend;
        }
        l3.printf("|%" DEC "|%" DEC "| <- %" DEC " ->  |%" DEC "|%" DEC "|", al->getSubjectStart(m)+1,qryalstart+1, al->lenAlign(),qryalend+1,al->getSubjectEnd_uncompressed(m)+1);
    }

    printAlignmnetSingleRightTail
    printAlignmnetSingleAssertFilters

    intermHigh=highlightposition-isS;
    stringHigh=intermHigh+params->padding+1;

    if((outwhat&alPrintSubject) && l1 ) {
        if( !(outwhat&alPrintSequenceOnly) ){
            hdl1.printf(",%s,%" DEC "",l1.ptr(params->indels), isnn+1);
        }
        else {
            hdl1.printf("%s",l1.ptr(params->indels));
        }
    }
    if((outwhat&alPrintUpperInterm) && !(outwhat&alPrintDotsFormat) && l2 ) {
        if(!(outwhat&alPrintSequenceOnly) ) {
            hdl2.printf(",%s,%" DEC,l2.ptr(params->indels),i);
        }
        else {
            hdl2.printf("%s",l2.ptr(params->indels));
        }
    }
    if((outwhat&alPrintQuery) && l3 ) {
        if( !(outwhat&alPrintSequenceOnly) ) {
            hdl3.printf(",%s,%" DEC "",l3.ptr(params->indels), iqnn+1);
        }
        else {
            hdl3.printf("%s",l3.ptr(params->indels) );
        }
    }

    if( (outwhat & alPrintSubject) && l1 && !(outwhat&alPrintSequenceOnly) ) {
        hdl1.printf(",%s",tSub.ptr());
        if(highlightposition != sNotIdx&& highlightposition >= 0 ){
            hdl1.printf(",,%" DEC ",%" DEC,highlightposition + 1,stringHigh);
        }
        else {
            hdl1.printf(",,");
        }
    }
    if( (outwhat&alPrintQuery) && l3 && !(outwhat&alPrintSequenceOnly) ) {
        hdl3.printf(",%s",tQry.ptr());
        hdl3.printf(",%" DEC,(outwhat & alPrintCollapseRpt)?bioal->getRpt(iAlInd):1);

        if( highlightposition != sNotIdx && highlightposition >= 0 )
            hdl3.printf(",%" DEC ",%" DEC, iqHigh + 1,stringHigh);
        else
            hdl3.printf(",");

    }
    if((outwhat&alPrintUpperInterm) && !(outwhat&alPrintDotsFormat) && l2 && !(outwhat&alPrintSequenceOnly) ) {
        if( highlightposition != sNotIdx && highlightposition >= 0 )
            hdl2.printf(",,,%" DEC ",%" DEC, intermHigh+1,stringHigh);
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
        }
    }

    UNUSED_VAR(callLetterHigh);

    return 1;
}

idx sBioal::BioseqAlignmentComparator(void * parameters, void * A, void * B,void * objSrc,idx i1,idx i2 )
{
    ParamsAlignmentSorter * param = (ParamsAlignmentSorter *)parameters;
    if(param->extComparator){
        return param->extComparator(param->extCompParams, A,  B, objSrc, i1, i2);
    }
    sBioseqAlignment::Al * hdrA= (sBioseqAlignment::Al * )A; sBioseqAlignment::Al * hdrB=(sBioseqAlignment::Al * )B;
    idx compVal1=0,compVal2=0,compId1=0,compId2=0;
    idx * m1=param->bioal->getMatch(i1);
    idx * m2=param->bioal->getMatch(i2);


    if(param->flags&sBioal::alSortByPosStart)
    {
        compVal1=hdrA->getSubjectStart(m1);
        compVal2=hdrB->getSubjectStart(m2);
        if(!(param->flags^sBioal::alSortByPosStart))
            return (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1)  : 0 ;
    }
    else if(param->flags&sBioal::alSortByPosEnd){
        compVal1=hdrA->getSubjectEnd(m1)+1;
        compVal1=hdrB->getSubjectEnd(m2)+1;
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
