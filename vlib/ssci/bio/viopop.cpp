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

#include <ssci/bio/viopop.hpp>
#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>


idx sViopop::Digest(const char* outputfilename, sFrameConsensus * Clones,idx * arrSortClon,sBioseqpopul::cloneStats * sumStats,idx * sortedSubjInd)
{



    sVioDB db(outputfilename,"viopop2",5,7);


    idx relationlistClones[8]={eTypeClone,eTypeClone,eTypeClone,eTypeSeqCov,eTypeStats,eTypeSummary,eTypeSortArr};
    idx relationlistSeqCov[1]={eTypeClone};
    idx relationlistStats[1]={eTypeClone};
    idx relationlistSummary[1]={eTypeClone};
    idx relationlistsortArr[1]={eTypeClone};

    db.AddType(sVioDB::eOther,7,relationlistClones,"clone", eTypeClone);
    db.AddType(sVioDB::eOther,1,relationlistSeqCov,"seqcov", eTypeSeqCov);
    db.AddType(sVioDB::eOther,1,relationlistStats,"stats", eTypeStats);
    db.AddType(sVioDB::eOther,1,relationlistSummary,"summary", eTypeSummary);
    db.AddType(sVioDB::eOther,1,relationlistsortArr,"sortArr", eTypeSortArr);

    sVec< sVec<idx> > clonesSeqCov;clonesSeqCov.add(Clones->dim());
    for(idx iCl=0; iCl<Clones->dim();++iCl){
        sCloneConsensus * clone=Clones->ptr(iCl);

        db.AddRecord(eTypeClone,(const void *)&(clone->summary), sizeof(sBioseqpopul::cloneSummary) );

        if(clone->summary.clID>=0){

            if(clone->summary.parentClID>=0 && clone->summary.clID!=clone->summary.parentClID)db.AddRecordRelationshipCounter(eTypeClone, 0, eClone2FatherClone);

            if(clone->summary.mergeclID>=0 && clone->summary.clID!=clone->summary.mergeclID)db.AddRecordRelationshipCounter(eTypeClone, 0, eClone2MergeClone);

            db.AddRecordRelationshipCounter(eTypeClone, 0, eClone2SeqCov);
            db.AddRecordRelationshipCounter(eTypeClone, 0, eClone2Stats);

            idx myRecordID=0;
            if(clone->seqCov.dim()){
                sVec<idx> * seqCov=clonesSeqCov.ptr(iCl);
                seqCov->add(clone->seqCov.dim());  seqCov->set(0);

                for(idx iC=0;iC<seqCov->dim();++iC){
                    *seqCov->ptr(iC)=clone->seqCov[iC].baseCov;
                }

                if(db.SetRecordIndexByBody((const void *)seqCov->ptr(), eTypeSeqCov, &myRecordID, sizeof(idx)*seqCov->dim() )){
                    db.AddRecord(eTypeSeqCov,(const void *)(seqCov->ptr()), sizeof(idx)*seqCov->dim() );
                }
                db.AddRecordRelationshipCounter(eTypeSeqCov, myRecordID, eSeqCov2Clone);

                myRecordID=0;
                if(db.SetRecordIndexByBody((const void *)&(clone->stats), eTypeStats, &myRecordID, sizeof(sBioseqpopul::cloneStats) ))
                    db.AddRecord(eTypeStats,(const void *)&(clone->stats), sizeof(sBioseqpopul::cloneStats) );
            }

        }

    }

    for(idx fCl=0; fCl<Clones->dim();++fCl){
        sCloneConsensus * fclone=Clones->ptr(fCl);
        if(fclone->summary.clID<0)continue;
        for(idx bCl=0; bCl<Clones->dim();++bCl){
            if(bCl==fCl)continue;
            sCloneConsensus * bclone=Clones->ptr(bCl);
            if(bclone->summary.parentClID>=0 && bclone->summary.parentClID==fclone->summary.clID){
                db.AddRecordRelationshipCounter(eTypeClone, fCl+1, eClone2Clone);
            }
        }
    }

    db.AddRecord(eTypeSummary,(const void *)sumStats, sizeof(sBioseqpopul::cloneStats));
    db.AddRecordRelationshipCounter(eTypeClone, 1, eClone2Summary);
    db.AddRecord(eTypeSortArr,(const void *)arrSortClon, sizeof(idx)*Clones->dim());
    db.AddRecordRelationshipCounter(eTypeClone, 1, eClone2SortArr);

    db.AllocRelation();

    for(idx iCl=0; iCl<Clones->dim();++iCl){

        sCloneConsensus * clone=Clones->ptr(iCl);

        if(clone->summary.clID<0)continue;
        idx mySeqCov=db.GetRecordIndexByBody((const void *)clonesSeqCov[iCl].ptr(),eTypeSeqCov,sizeof(idx)*clonesSeqCov[iCl].dim());
        idx myStats=db.GetRecordIndexByBody((const void *)&(clone->stats),eTypeStats, sizeof(sBioseqpopul::cloneStats) );
        db.AddRelation(eTypeClone, eClone2SeqCov, iCl+1, mySeqCov);
        db.AddRelation(eTypeClone, eClone2Stats, iCl+1, myStats);

        db.AddRelation(eTypeSeqCov, eSeqCov2Clone,mySeqCov, iCl+1);

        for(idx bCl=0; bCl<Clones->dim();++bCl){
            sCloneConsensus * bclone=Clones->ptr(bCl);
            if(iCl==bCl)continue;
            if(bclone->summary.parentClID==clone->summary.clID)
                db.AddRelation(eTypeClone,eClone2Clone,iCl+1,bCl+1);
            if(bclone->summary.clID==clone->summary.mergeclID)
                db.AddRelation(eTypeClone,eClone2MergeClone,iCl+1,bCl+1);
            if(bclone->summary.clID==clone->summary.parentClID)
                db.AddRelation(eTypeClone,eClone2FatherClone,iCl+1,bCl+1);
        }
    }
    db.AddRelation(eTypeClone, eClone2Summary, 1, 1);
    db.AddRelation(eTypeClone, eClone2SortArr, 1, 1);

    db.Finalize(true);

    return 1;
}

bool sViopop::buildMergeDictionary(ParamCloneIterator * params, sDic<sVec<idx> > &mergeTree, idx clCnt)
{
    if( !(params->minCov || params->minLen || params->minSup || params->hiddenClones) ) {
        return false;
    }
    sStr dst;
    sDic<idx> cl2hide;
    sString::searchAndReplaceSymbols(&dst, params->hiddenClones, 0, ",", 0, 0, true, true, false, false);
    sBioseqpopul::cloneSummary * parentCl = 0;
    const char * lst = 0;
    for(lst = dst; lst; lst = sString::next00(lst)) {
        cl2hide[lst] = true;
    }

    bool mergeCl = false;
    idx iSC = 0, length = 0;
    sStr t_buf;
    bool isGapsInfo = isVersion2();
    for(idx iCl = 0; iCl < clCnt; ++iCl) {
        sBioseqpopul::cloneStats * clStat = getStats(iCl);
        sBioseqpopul::cloneSummary * clSum = getCl(iCl);
        t_buf.printf(0, "%" DEC, iCl);
        mergeCl = false;
        if( params->hiddenClones && cl2hide.get(t_buf.ptr()) ) {
            mergeCl = true;
        }

        length = clSum->end - clSum->start;
        if( isGapsInfo && (params->flags & clSkipSupportedDeletions) )
            length -= clStat->supportedGapsCnt;
        if( isGapsInfo && (params->flags & clPrintNoGapsFrame) )
            length -= clStat->multiGapsCnt;

        if( !mergeCl && ((params->minCov && clStat->maxCov < params->minCov) || (params->minLen && length < params->minLen) || (params->minSup && getCl(iCl)->hasParent() && clStat->support < params->minSup)) ) {
            mergeCl = true;
        }
        if( !mergeCl )
            mergeTree.set(t_buf.ptr());
        else {
            ++iSC;
            parentCl = getCl(iCl);
            if( parentCl && parentCl->hasParent() ) {
                parentCl = getCl(parentCl->parentClID);
            } else {
                parentCl = 0;
            }
            while( parentCl ) {
                t_buf.printf(0, "%" DEC, parentCl->clID);
                sVec<idx> *tm = mergeTree.get(t_buf.ptr());
                if( tm ) {
                        tm->vadd(1, iCl);
                    break;
                }
                if( parentCl->hasParent() ) {
                    parentCl = getCl(parentCl->parentClID);
                } else {
                    parentCl = 0;
                }
            }
        }
    }
    mergeDict = &mergeTree;
    return true;
}

idx sViopop::buildMergeCompositionDictionary( ParamCloneIterator * params, sDic< sVec<contigComp> > &mergeComp ) {
    idx prev_res = mergeComp.dim();

    if( mergeDict ) {
        sStr t_buf;
        sVec< contigComp > * comp = 0;
        for( idx i = 0 ; i < dimCl() ; ++i ) {
            t_buf.printf(0,"%" DEC,i);
            if( mergeDict->get(t_buf.ptr()) ) {
                comp = mergeComp.set(t_buf.ptr());
                constructCloneComposition(i, comp);
            }
        }
        if( mergeComp.dim() - prev_res ) {
            mergeCompDict = &mergeComp;
        }
    }
    return mergeComp.dim() - prev_res;
}

idx sViopop::iterateClones(idx * iVis, idx start, idx cnt, typeCallbackIteratorFunction callbackFunc,ParamCloneIterator * params,idx * arrSortClon)
{


    idx * clList=0;
    idx clCnt=0, iFound=0,res=0;
    sVec<idx> sumPosCov;
    sDic< sVec<idx> > mergeTree;

    clCnt=dimCl();
    if(arrSortClon){
        clList=arrSortClon;
        setMode(1);
    }
    else{
        clList=getArrSort();
    }

    if(params->resolution){
        idx minStart=sIdxMax,maxEnd=0, frmStart = 0, frmEnd = 0 ;

        sVec<seqCovPosition> seqCov;sVec<idx> g2sMap, * pG2SMap = 0;
        sVec<contigComp> cComp;

        for(idx iCl=0;iCl<clCnt;++iCl){
            sBioseqpopul::cloneSummary * clSum=getCl(iCl), mergedCl;
            if(!clSum) continue;
            if( mergeDict){
                if( isValidClone(clSum->clID) ){
                    seqCov.cut(0);g2sMap.cut(0);cComp.cut(0);
                    pG2SMap = &g2sMap;
                    if (mergeClone(clSum, mergedCl,cComp, params,&seqCov, pG2SMap ) ) {
                        clSum = &mergedCl;
                    }
                }
                else
                    continue;
            }

            frmStart = getFramePos ( clSum, clSum->start, pG2SMap,true );
            frmEnd = getFramePos ( clSum, clSum->end -1, pG2SMap,true ) + 1;
            if( frmStart < minStart)
                minStart = frmStart;
            if( frmEnd > maxEnd)
                maxEnd = frmEnd;
        }
        minStart--;
        maxEnd--;
        if(params->sStart>=0 && minStart<params->sStart)
            minStart=params->sStart;
        if(params->sEnd>=0 && maxEnd>params->sEnd)
            maxEnd=params->sEnd;

        params->resolution=(params->resolution && params->resolution>0 )? (((maxEnd-minStart-2)/params->resolution > 0) ? (maxEnd-minStart-2)/params->resolution : 1) : 0;
    }


    if(params->isNormCov){
        sBioseqpopul::cloneStats * gStats=getGenStats();
        sumPosCov.add(gStats->size);sumPosCov.set(0);
        for (idx icl=0; icl<clCnt; ++icl){
            idx iCl=clList ? clList[icl] : icl;
            sBioseqpopul::cloneSummary * clSum=getCl(iCl);
            if(!clSum || clSum->clID<0 || clSum->start==clSum->end)continue;
            sVec<idx> scCov(sMex::fExactSize);
                getCov(iCl,scCov);
            for(idx p=clSum->start;p<clSum->end;++p){
                idx ip=p-(clSum->start);
                sumPosCov[p]+=scCov[ip];
            }
        }
        params->normCov=sumPosCov.ptr();
    }



    for (idx icl=0; icl<clCnt; ++icl) {
        idx iCl=clList ? clList[icl] : icl;
        sBioseqpopul::cloneSummary * clSum=getCl(iCl);

        if(clSum && clSum->clID<0)continue;
        sBioseqpopul::cloneStats * clStat=getStats(iCl);
        if(!clStat)continue;
        if( clSum->start==clSum->end)continue;

        res=callbackFunc(this, params, iCl);
        if(res)++iFound;

        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,iCl,100*(iCl)/clCnt,100) )
                return 0;
        }
    }

    return iFound;
}

idx sViopop::clonesCoverage( ParamCloneIterator * params, sBioseqpopul::cloneSummary * cl, sVec< seqCovPosition > * seqCov, idx refCnt, sVec < contigComp > * comp , sVec<idx> * skp2gapMap , sVec<idx> * gap2skpMap )
{
    real maxCov = -1;

    idx spcFrmStart = getFramePos ( cl, cl->start, gap2skpMap,true );
    idx spcFrmRangeStart = spcFrmStart;
    idx spcFrmEnd = getFramePos ( cl, cl->end -1, gap2skpMap,true ) + 1;
    idx spcFrmRangeEnd = spcFrmEnd;
    idx cov = 0; char c;
    if( params->sStart >= 0 || params->sEnd >= 0 ) {
        if( params->sStart >= 0 ) {
            if( params->sStart >= spcFrmRangeEnd )
                return maxCov;
            else if( params->sStart > spcFrmRangeStart )
                spcFrmRangeStart = params->sStart;
        }
        if( params->sEnd >= 0 ) {
            if( params->sEnd <= spcFrmRangeStart ) {
                return maxCov;
            } else if( params->sEnd < spcFrmRangeEnd )
                spcFrmRangeEnd = params->sEnd + 1;
        }
    }

    params->out->printf("%" DEC ",%" DEC ",", spcFrmRangeStart + 1, spcFrmRangeEnd);
    sBioseqpopul::cloneSummary * p_cl = 0, * m_cl = 0;
    idx parentID = cl->clID, mergeID = cl->clID, bifurcPos = -1, mergePos = -1;
    if( cl->hasParent() ) {
        p_cl = getValidParentSummary(cl);

        if( p_cl ) {
            parentID = p_cl->clID;
            bifurcPos = spcFrmStart + 1;
        }
    }
    if( cl->hasMerged() ) {
        m_cl = getValidMergingSummary(cl);
        if( m_cl ) {
            mergeID = m_cl->clID;
                mergePos = getMergedFramePos(m_cl, cl->end -1,params,true)+1;
        }
    }
    params->out->printf("Clone_%" DEC ",Clone_%" DEC ",%" DEC ",%" DEC ",", parentID,mergeID,bifurcPos,mergePos );

    seqCovPosition * sCP = 0;
    idx sq_size = 0;
    if( !seqCov ) {
        sCP = getSeqCov(cl->clID,&sq_size);
    }
    else {
        sCP = seqCov->ptr(0);
        sq_size = seqCov->dim();
    }

    idx interval;
    interval = params->resolution;
    idx fi = 0, ci = 0, ti = 0, cPos = -1;
    sStr oCov, oSimil, single_simil;
    oCov.printf("[");

    idx contigSpcFrmStart = spcFrmRangeStart - spcFrmStart;
    idx contigMultStart = getSkip2GapContigPos(cl, contigSpcFrmStart, skp2gapMap);
    idx contigSpcFrmEnd = spcFrmRangeEnd - spcFrmStart;
    idx contigMultEnd = getSkip2GapContigPos(cl, contigSpcFrmEnd - 1, skp2gapMap) + 1;
    idx frmMultEnd = cl->start + contigMultEnd;

    while( cPos<0 ) {
        cPos = getContigPos(cl, contigMultStart++, gap2skpMap);
    }

    idx frmMultStart = cl->start + contigMultStart;
    fi = frmMultStart;

    ci = contigMultStart;
    ti = spcFrmStart+cPos;
    c = sCP[ci].baseChar();
    cov = sCP[ci].isMultiGap()?0:sCP[ci].coverage();
    if( simil ) {
        oSimil.cut(0);
        printSimilaritiesinJSON(oSimil, cl->clID, fi, cov, refCnt, params, comp);
    }
    printPositionCoverageinJSON(oCov,ti + 1,c,cov,oSimil.ptr(),params->normCov,params->normCov?params->normCov[fi]:0);
    maxCov = params->normCov ? (params->normCov[fi] ? 100 * (real) cov / params->normCov[fi] : 0) : cov;

    idx elms = 0, minPosi = 0, maxPosi = 0;
    real maxIntCov = -REAL_MAX, minIntCov = REAL_MAX;
    sStr tmpOut, minOut, maxOut;
    for(fi = frmMultStart + 1, ci = contigMultStart + 1; fi < frmMultEnd - 1 && ci < contigMultEnd - 1 ; ++fi, ++ci) {

        cPos = getContigPos(cl, ci, gap2skpMap);
        if( cPos < 0 || (gap2skpMap && !sCP[ci].base() && !sCP[ci].coverage()) ) {
            continue;
        }
        ti = spcFrmStart + cPos;

        c = sCP[ci].baseChar();
        cov = sCP[ci].isMultiGap()?0:sCP[ci].coverage();

        if( interval > 0 ) {
            if( elms == interval || ci == (contigMultEnd - 1) ) {
                if( minOut.length() && maxOut.length() ) {
                    if( minPosi < maxPosi )
                        oCov.printf("%s%s", minOut.ptr(), maxOut.ptr());
                    else if( minPosi > maxPosi )
                        oCov.printf("%s%s", maxOut.ptr(), minOut.ptr());
                } else if( minOut.length() || maxOut.length() ) {
                    if( maxOut.length() )
                        oCov.printf("%s", maxOut.ptr());
                    else if( minOut.length() )
                        oCov.printf("%s", minOut.ptr());
                }

                maxIntCov = -REAL_MAX;
                minIntCov = REAL_MAX;
                elms = 0;
            }
            ++elms;
        }
        if( simil ) {
            oSimil.cut(0);
            printSimilaritiesinJSON(oSimil, cl->clID, fi, cov, refCnt, params, comp);
        }
        tmpOut.printf(0,",");
        printPositionCoverageinJSON(tmpOut,ti+1,c,cov,oSimil.ptr(),params->normCov,params->normCov?params->normCov[fi]:0);

        real curCov;
        if( params->normCov ) {
            curCov = (params->normCov[fi] ? 100 * (real) cov / params->normCov[fi] : 0);
            if( maxCov < curCov )
                maxCov = curCov;
        } else {
            curCov = cov;
            if( maxCov < cov )
                maxCov = (real) cov;
        }
        if( interval > 0 ) {
            if( curCov > maxIntCov ) {
                maxIntCov = curCov;
                maxPosi = ti;
                maxOut.printf(0, "%s", tmpOut.ptr());
            } else if( curCov < minIntCov ) {
                minIntCov = curCov;
                minPosi = ti;
                minOut.printf(0, "%s", tmpOut.ptr());
            }
        } else
            oCov.printf("%s", tmpOut.ptr());
    }

    if( interval > 0 ) {
        if( minOut.length() && maxOut.length() ) {
            if( minPosi < maxPosi )
                oCov.printf("%s%s", minOut.ptr(), maxOut.ptr());
            else if( minPosi > maxPosi )
                oCov.printf("%s%s", maxOut.ptr(), minOut.ptr());
        } else if( minOut.length() || maxOut.length() ) {
            if( maxOut.length() )
                oCov.printf("%s", maxOut.ptr());
            else if( minOut.length() )
                oCov.printf("%s", minOut.ptr());
        }

        maxIntCov = -REAL_MAX;
        minIntCov = REAL_MAX;
        elms = 0;
        ++elms;
    }

    idx seqcovEnd = contigMultEnd - 1 , cordEnd = frmMultEnd - 1;
    cPos = getContigPos(cl, seqcovEnd, gap2skpMap);
    if( cPos>=0 ) {
        ti = spcFrmStart + cPos;

        c = sCP[seqcovEnd].baseChar();
        cov = sCP[seqcovEnd].isMultiGap()?0:sCP[seqcovEnd].coverage();

        if( simil ) {
            oSimil.cut(0);
            printSimilaritiesinJSON(oSimil, cl->clID, fi, cov, refCnt, params, comp);
        }
        oCov.printf(",");
        printPositionCoverageinJSON(oCov,ti+1,c,cov,oSimil.ptr(),params->normCov,params->normCov?params->normCov[cordEnd]:0);
    }

    oCov.printf("]");
    if( !params->normCov )
        params->out->printf("%" DEC ",\"%s\"", (idx) maxCov, oCov.ptr());
    else {
        params->out->printf("%lf,\"%s\"", maxCov, oCov.ptr());
        maxCov = 100;
    }
    return maxCov;
}

bool sViopop::printSimilaritiesinJSON(sStr & out, idx cl, idx pos, idx cov, idx rfCnt, ParamCloneIterator * params, sVec < contigComp > * comp) {
    idx sim_cnt = params->similarity_cnt;
    real thresh_sim = params->similarity_threshold;

    sStr t_out;
    real t_sim = 0;idx iS = 0;
    sVec< real > sort_sim(sMex::fExactSize);
    if( sim_cnt && sim_cnt<rfCnt ) {
        sort_sim.add(rfCnt);
    }

    if(cov) {

        for( iS = 0; iS < rfCnt; ++iS) {

            t_sim = getMergedPosSimils(cl, pos, iS, cov, comp);
            if( sort_sim.dim() ) {
                sort_sim[iS] = t_sim;
                continue;
            }

            if( cov && t_sim > thresh_sim )
                t_out.printf(",'%" DEC "':%.4lf",iS, t_sim );
        }
        if( sort_sim.dim() ) {
            sVec<idx> ofs(sMex::fExactSize);ofs.add(sort_sim.dim() );
            sSort::sort(sort_sim.dim(), sort_sim.ptr(), ofs.ptr());
            cl = 0;
            real tot_sim = 0;
            for( iS = rfCnt - 1; iS > 0 && cl < sim_cnt; --iS) {
                t_sim = sort_sim[ofs[iS]];
                tot_sim += t_sim;
                if(!t_sim) {
                    break;
                }
                t_out.printf(",'%" DEC "':%.4lf",ofs[iS], t_sim );
                ++cl;
            }
        }
    }
    if( t_out.length() ){
        out.printf("{%s}",t_out.ptr(1) );
        return true;
    }
    return false;
}

void sViopop::printPositionCoverageinJSON(sStr & out, idx pos, char let ,idx cov, const char * sim, bool norm, idx t_cov ) {

    out.printf("{'p':%" DEC ",'l':'%c','c':", pos, let);
    if(norm){
        out.printf("%.4lf",t_cov?(real)cov*100/t_cov:0 );
    }
    else{
        out.printf("%" DEC,cov );
    }
    if(sim){
        out.printf(",'s':%s",sim);
    }
    out.printf("}");
}


bool sViopop::mergeClone(sBioseqpopul::cloneSummary * cl, sBioseqpopul::cloneSummary &resCl, sVec < contigComp > & comp, ParamCloneIterator * params, sVec< seqCovPosition > * seqCov, sVec<idx> * l_gap2skpMap, sVec<idx> *l_skp2gapMap)
{
    if(l_gap2skpMap && l_gap2skpMap!=gap2skipMap) {
        l_gap2skpMap->cut(0);
    }
    if(l_skp2gapMap && l_skp2gapMap!=skip2gapMap) {
        l_skp2gapMap->cut(0);
    }
    comp.cut(0);
    if(!cl) {
        return false;
    }
    bool res = getValidCloneComposition(cl, comp );
    if( !res ) {
        return false;
    }
    else if ( comp.dim() <= 1 ) {
        return false;
    }
    else {
        idx gapFlag = params?params->flags:gapFrameLevel;
        resCl.start = sIdxMax;
        resCl.end = 0;
        resCl.clID = cl->clID;
        resCl.parentClID = cl->parentClID;
        resCl.mergeclID = cl->mergeclID;
        contigComp * ctg = 0;
        sBioseqpopul::cloneSummary * c_cl, * t_cl;
        idx start = 0, end = 0, sq_size = 0, pos = 0, base = 0, skpPos = 0 , gapPos = 0;
        seqCovPosition * c_seqCov, * r_seqCov;
        for(idx i = 0 ; i < comp.dim() ; ++i ) {
            ctg = comp.ptr(i);
            if( !ctg->contigInds.dim() ) {
                continue;
            }
            if ( ctg->start < resCl.start ) {
                resCl.start = ctg->start;
                t_cl = getCl( ctg->contigInds[0] );
                if ( t_cl && t_cl->hasParent() )
                    resCl.parentClID = getValidCloneID(t_cl->parentClID);
                else
                    resCl.parentClID = resCl.clID;
                if( resCl.parentClID < 0 )
                    resCl.parentClID = resCl.clID;
            }
            if ( ctg->end > resCl.end ) {
                resCl.end = ctg->end;
                t_cl = getCl( ctg->contigInds[0] );
                if( t_cl && t_cl->hasMerged() )
                    resCl.mergeclID = getValidCloneID(t_cl->mergeclID);
                else
                    resCl.mergeclID = resCl.clID;
                if( resCl.mergeclID < 0 )
                    resCl.mergeclID = resCl.clID;
            }

            if( seqCov ) {
                sVec< cloneProfPos > prof(sMex::fExactSize);
                prof.add(ctg->end - ctg->start);

                for( idx j = 0 ; j < ctg->contigInds.dim() ; ++j ) {
                    c_cl = getCl( ctg->contigInds[j] );
                    if(!c_cl) continue;
                    sq_size = 0;
                    c_seqCov = getSeqCov( c_cl->clID, &sq_size );
                    start = ctg->start - c_cl->start;
                    pos = ctg->start;
                    end = ctg->end - c_cl->start;

                    assert(end <= sq_size );

                    for( idx l = start ; l < end ; ++l, ++pos ) {
                        cloneProfPos * profpos = prof.ptrx( l - start );
                        if ( c_seqCov[l].isMultiGap() ){
                            (profpos->pos[sBioseqpopul::baseGap]) += c_seqCov[l].coverage() ? c_seqCov[l].coverage() : 1;
                        }
                        else {
                            profpos->pos[c_seqCov[l].base()] += c_seqCov[l].coverage();
                        }
                    }
                }
                for ( idx j = 0; j < prof.dim() ; ++j ) {
                    r_seqCov = seqCov->ptrx(ctg->start + j - resCl.start);
                    base = prof[j].maxI();

                    if( prof[j].isMultiGap() ){
                        r_seqCov->setCoverage( prof[j].pos[sBioseqpopul::baseGap] );
                        r_seqCov->setBase( sBioseqpopul::baseGap );
                        if(l_gap2skpMap && gap2skipMap )*(l_gap2skpMap->add()) = -1;
                        ++gapPos;
                    }
                    else if( prof[j].isDeletion() && ( gapFlag&clSkipSupportedDeletions) ){
                        r_seqCov->setCoverage( prof[j].pos[sBioseqpopul::baseDel] );
                        r_seqCov->setBase( sBioseqpopul::baseDel );
                        if(l_gap2skpMap && gap2skipMap )*(l_gap2skpMap->add()) = -2;
                        ++gapPos;
                    }
                    else if( base != sBioseqpopul::baseIns ) {
                        r_seqCov->setCoverage( prof[j].sum() );
                        r_seqCov->setBase( base );
                        if(l_gap2skpMap && gap2skipMap )*(l_gap2skpMap->add()) = skpPos++;
                        if(l_skp2gapMap && skip2gapMap )*(l_skp2gapMap->add()) = gapPos;
                        ++gapPos;
                    }
                }
            }
        }
    }
    return true;
}

idx sViopop::getValidCloneID( idx iCl, bool followMergingPath)
{
    idx res = -1, cl;
    if( mergeDict ) {
        _t_buf.printf(0,"%" DEC,iCl);
        if( !mergeDict->get(_t_buf.ptr(),_t_buf.length()) ) {
            sBioseqpopul::cloneSummary * clSum = getCl(iCl);
            idx followID = followMergingPath?clSum->mergeclID:clSum->parentClID;
            if(clSum && followID != clSum->clID ) {
                cl =  getValidCloneID( followID, followMergingPath );
                if(cl>=0) {
                    _t_buf.printf(0,"%" DEC,cl);
                    sVec<idx> * mergedIDs = mergeDict->get(_t_buf.ptr(),_t_buf.length());
                    if (followMergingPath && mergedIDs) {
                        return cl;
                    }
                    for (idx i = 0 ; i < mergedIDs->dim(); ++i ) {
                        if( iCl == *mergedIDs->ptr(i) ) {
                            return cl;
                        }
                    }
                    return -1;
                }
            }
        }
        else {
            res = iCl;
        }
    } else {
        res = iCl;
    }
    return res;
}


sBioseqpopul::cloneSummary * sViopop::getValidCloneSummary( idx iCl, bool followMergingPath)
{
    idx res = getValidCloneID(iCl,followMergingPath);
    if (res < 0 ) {
        return 0;
    }
    return getCl(res);
}

sBioseqpopul::cloneSummary * sViopop::getValidParentSummary(sBioseqpopul::cloneSummary * cl) {
    idx iCl = cl->parentClID;
    sBioseqpopul::cloneSummary * rcl = getValidCloneSummary(iCl);
    if( rcl && rcl->clID == cl->clID ) {
        return getValidCloneSummary(iCl,true);
    }
    return rcl;
}

sBioseqpopul::cloneSummary * sViopop::getValidMergingSummary(sBioseqpopul::cloneSummary * cl) {
    idx iCl = cl->mergeclID;
    sBioseqpopul::cloneSummary * rcl = getValidCloneSummary(iCl);
    if( rcl->clID == cl->clID ) {
        return getValidCloneSummary(iCl,true);
    }
    return rcl;
}

void sViopop::constructCloneComposition(idx iCl, sVec< contigComp > *comp)
{
    comp->cut(0);
    cloneRegion t_contig;
    contigComp * c_Comp;
    sBioseqpopul::cloneSummary * cl = getCl(iCl), * m_cl;

    if( cl &&  mergeDict ) {
        _t_buf.printf(0,"%" DEC,iCl);
        sVec<idx> * merged = mergeDict->get(_t_buf.ptr());

        sVec< idx > rangePos;
        rangePos.vadd(2, cl->start, cl->end);

        if( merged ) {
            for( idx i = 0 ; i < merged->dim() ; ++i ) {
                m_cl = getCl( *(merged->ptr(i)) );
                if(!m_cl) continue;
                rangePos.vadd(2,m_cl->start, m_cl->end);
            }
            sSort::sort( rangePos.dim(),rangePos.ptr());
            for( idx i = 0 ; i+1 < rangePos.dim(); ++i ) {
                if(rangePos[i]==rangePos[i+1]) {
                    continue;
                }
                c_Comp = comp->add();
                c_Comp->start = rangePos[i];
                c_Comp->end = rangePos[i+1];
            }

            for( idx j = 0 ; j < comp->dim() ; ++j ) {
                if ( cl->start <= comp->ptr(j)->start  && cl->end >= comp->ptr(j)->end ) {
                    comp->ptr(j)->contigInds.vadd(1,cl->clID);
                }
                else if ( cl->start > comp->ptr(j)->start ) {
                    break;
                }
            }

            for( idx i = 0 ; i < merged->dim() ; ++i ) {
                m_cl = getCl( *(merged->ptr(i)) );
                for( idx j = 0 ; j < comp->dim() && m_cl ; ++j ) {
                    if ( m_cl->start <= comp->ptr(j)->start  && m_cl->end >= comp->ptr(j)->end ) {
                        comp->ptr(j)->contigInds.vadd(1,m_cl->clID);
                    }
                    else if ( m_cl->end < comp->ptr(j)->start ) {
                        break;
                    }
                }
            }
        }
        else {
            c_Comp = comp->add();
            c_Comp->start = cl->start;
            c_Comp->end = cl->end;
            c_Comp->contigInds.vadd(1, cl->clID);
        }
    }
    else if( cl ){
        comp->cut(0);
        c_Comp = comp->add();
        c_Comp->start = cl->start;
        c_Comp->end = cl->end;
        c_Comp->contigInds.vadd(1, cl->clID);
    }
}

bool sViopop::getValidCloneComposition(sBioseqpopul::cloneSummary * cl, sVec < contigComp > & c_comp )
{
    c_comp.cut(0);
    if ( mergeCompDict ) {
        _t_buf.printf(0,"%" DEC, cl->clID);
        sVec < contigComp > * l_comp = mergeCompDict->get(_t_buf.ptr());
        if(l_comp) {
            for(idx i = 0 ; i < l_comp->dim() ; ++i ) {
                contigComp * tt = c_comp.add();
                tt->start = l_comp->ptr(i)->start;
                tt->end = l_comp->ptr(i)->end;
                tt->contigInds.copy( &l_comp->ptr(i)->contigInds );
            }
            return true;
        }
        return false;
    }
    else {
        contigComp * lcomp = c_comp.add();
        lcomp->start = cl->start;
        lcomp->end = cl->end;
        lcomp->contigInds.vadd(1,cl->clID);
        return true;
    }
}

idx sViopop::printAllClones(sViopop * viopop, ParamCloneIterator * params, idx clIndex)
{

    sBioseqpopul::cloneSummary * cl = viopop->getCl(clIndex), mergedCl;

    if(!cl) return 0;

    sBioseqpopul::cloneStats * gStats = viopop->getGenStats ();
    idx refCnt=gStats->support;
    sVec< seqCovPosition > seqCov, * pSC = 0;
    sVec < contigComp > cComp, * pC_Comp;
    sVec<idx> skp2gapMap, gap2skpMap, * pS2Gmap = 0,* pG2Smap = 0;
    if( params->flags&sViopop::clPrintNoGapsFrame ) {
        pS2Gmap = &skp2gapMap;
        pG2Smap = &gap2skpMap;
    }

    if( viopop->mergeDict){
        if( viopop->isValidClone(cl->clID) ){
            if (viopop->mergeClone(cl, mergedCl,cComp, params,&seqCov, pG2Smap, pS2Gmap ) ) {
                pSC = &seqCov;
                cl = &mergedCl;
                if (cComp.dim()){
                    pC_Comp = &cComp;
                }
            }
        }
        else
            return 0;
    }

    params->out->printf("\n");
    params->out->printf("Clone_%" DEC ",%" DEC ",%" DEC ",", cl->clID, viopop->getFramePos(cl, cl->start, pG2Smap,true)+1, viopop->getFramePos(cl, cl->end-1, pG2Smap,true)+1);

    if( viopop->clonesCoverage(params, cl, pSC, refCnt, pC_Comp, pS2Gmap, pG2Smap ) < 0 )
        return 0;

    params->out->printf(",");
    sBioseqpopul::cloneSummary * pCl = viopop->getValidParentSummary(cl);
    if( cl->hasParent() && pCl ) {
        sVec< seqCovPosition > pSeqCov, * ppSC = 0;
        sVec < contigComp > pComp;
        sBioseqpopul::cloneSummary pMergedCl;
        if( viopop->mergeDict){
            if (viopop->mergeClone(pCl, pMergedCl, pComp, params, &pSeqCov) ) {
                ppSC = &pSeqCov;
                pCl = & pMergedCl;
            }
        }
        viopop->clonesDifferences(params, pCl, cl, ppSC, pSC, pG2Smap, viopop->getStats(cl->clID) );
    } else {
        params->out->printf("-1,,,,,");
    }

    return 1;
}

idx sViopop::clonesDifferences(ParamCloneIterator * params, sBioseqpopul::cloneSummary * parentCl, sBioseqpopul::cloneSummary * childCl, sVec<seqCovPosition> * pSC,  sVec<seqCovPosition> * cSC, sVec<idx> * child_gap2skpMap, sBioseqpopul::cloneStats * c_stats )
{
    idx pSqSize = 0, cSqSize = 0;
    seqCovPosition * pSeqCov = 0, * cSeqCov = 0;
    if ( pSC ) {
        pSqSize = pSC->dim();
        pSeqCov = pSC->ptr();
    }
    else {
        pSeqCov = getSeqCov(parentCl->clID,&pSqSize);
    }
    if ( cSC ) {
        cSqSize = cSC->dim();
        cSeqCov = cSC->ptr();
    }
    else {
        cSeqCov = getSeqCov(childCl->clID,&cSqSize);
    }
    if(!pSeqCov || !cSeqCov) return 0;


    if( childCl->start > parentCl->end || childCl->end < parentCl->start )
        return 0;

    idx start = (childCl->start < parentCl->start) ? parentCl->start : childCl->start;
    idx end = (childCl->end < parentCl->end) ? childCl->end : parentCl->end;
    idx pi = 0, ci = 0, diffs = 0;
    sStr oDifs;
    idx firstDif = start+1,lastDif = end;
    oDifs.printf("\"");
    for(idx i = start; i < end; ++i) {
        bool isDiff=false;
        pi = i - parentCl->start;
        ci = i - childCl->start;
        if( pSeqCov[pi].coverage() && cSeqCov[ci].coverage() ) {
            if( pSeqCov[pi].base() != cSeqCov[ci].base() ) {
                isDiff=true;
            }
        }
        else if ( !cSeqCov[ci].coverage() && !pSeqCov[pi].coverage() ) {
            isDiff = false;
        }
        else {
            isDiff = true;
        }
        if( isDiff ) {
            if( !diffs )
                firstDif = i+1;
            else
                oDifs.printf(",");
            char fB = pSeqCov[pi].baseChar() ;
            char cB = pSeqCov[pi].baseChar();
            oDifs.printf("%" DEC ">%" DEC "/%c:%" DEC "/%c", i+1, pSeqCov[pi].coverage(), fB, cSeqCov[ci].coverage(), cB);
            lastDif = i+1;
            ++diffs;
        }
    }
    oDifs.printf("\"");
    params->out->printf("%" DEC ",%" DEC ",%" DEC ",%s", getFramePos(childCl,firstDif, child_gap2skpMap,true), getFramePos(childCl, lastDif, child_gap2skpMap,true), diffs, oDifs.ptr());

    if ( c_stats )
    params->out->printf(",\"{\'pvalue\':%lf,\'bayes\':%lf}\"",c_stats->bifurcation_pvalue,c_stats->bifurcation_bayes);

    return diffs;
}

idx sViopop::printHierarchySingle(sViopop * viopop, ParamCloneIterator * params, idx clIndex){
    sBioseqpopul::cloneSummary * clSum=0;
    sVec<idx> scCov(sMex::fExactSize), scSeq(sMex::fExactSize);

    if(params->flags&sViopop::clPrintSummary)
        clSum=viopop->getCl(clIndex);
    if(params->flags&sViopop::clPrintCoverage)
        viopop->getCov(clIndex,scCov);
    if(params->flags&sViopop::clPrintConsensus)
        viopop->getSeq(clIndex,scCov);

    sStr t_buf("%" DEC,clIndex);

    if( viopop->mergeDict ){
        sVec<idx> * mergingCl=viopop->mergeDict->get( t_buf.ptr() );
        if(!mergingCl){
            return 0;
        }
    }
    if(clSum && clSum->clID>=0 && clSum->start!=clSum->end){
        params->out->printf("Clone_%" DEC ",%" DEC ",%" DEC ",%" DEC,clSum->clID,clSum->start,clSum->end,clSum->mergeclID);


        if(params->flags&sViopop::clPrintTreeMode){
            params->out->printf(",/");
            sVec<idx> fArr;
            sBioseqpopul::cloneSummary * fCl=viopop->getFatherCl(clIndex);
            idx fClID = -1;
            while(fCl){
                fClID= viopop->getValidCloneID( fCl->clID );
                if(fClID >=0 ) {
                    fArr.vadd(1, fClID);
                }
                fCl=viopop->getFatherCl(fClID);
            }
            for(idx iF=fArr.dim()-1;iF>=0;--iF){
                params->out->printf("Clone_%" DEC "/",fArr[iF]);
            }
        }
        params->out->printf("\n");
    }
    return 1;
}

idx sViopop::printAllCoverageClones(idx * iVis, idx start, idx cnt,sStr & out,idx * arrSortClon){
    idx * clList=0;
    idx clCnt=0, iFound=0;

    clCnt=dimCl();
    if(arrSortClon){
        clList=arrSortClon;
        setMode(1);
    }
    else{
        clList=getArrSort();
    }
    out.printf("Position");
    for (idx icl=0; icl<clCnt; ++icl) {
        idx iCl=clList ? clList[icl] : icl;
        out.printf(",Clone_%" DEC,getCl(iCl)->clID);
    }
    sBioseqpopul::cloneStats * stats=getGenStats();
    for(idx is=0;is<stats->size;++is){
        out.printf("\n");
        out.printf("%" DEC,is+1);
        for (idx icl=clCnt-1; icl>=0; --icl) {
            idx iCl=clList ? clList[icl] : icl;
            sVec<idx> scCov(sMex::fExactSize);
            getCov(iCl,scCov);
            sBioseqpopul::cloneSummary* cl=getCl(iCl);
            if(!cl || cl->clID<0)continue;
            if(cl->start>is || cl->end<=is)
                out.printf(",");
            else
                out.printf(",%" DEC,scCov[is-cl->start]);
        }
        ++iFound;
    }

    return iFound;
}
idx sViopop::getClonesOnPosition( sViopop * viopop, idx pos, sVec<idx> * clInds ) {
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    sVec<contigComp> comp;

    clInds->cut(0);
    for(idx cLi = 0 ; cLi < viopop->dimCl() ; ++cLi ) {
        cl = viopop->getCl(cLi);
        if(!cl) continue;
        if( viopop->mergeClone(cl,rsCl,comp) ) {
            cl = &rsCl;
        }
        else if (!viopop->isValidClone(cl->clID)) {
            continue;
        }
        if( pos >= cl->start && pos < cl->end ) {
            *clInds->add() = cLi;
        }
    }
    return clInds->dim();
}
idx sViopop::getBifurcatedClonesOnPosition( idx pos, sVec<idx> * clInds, idx clId ) {
    sDic< sVec<idx> > * merged = mergeDict;
    sBioseqpopul::cloneSummary * clSum=0;
    clInds->cut(0);
    for(idx cLi = 0 ; cLi < dimCl() ; ++cLi ) {
        _t_buf.printf(0,"%" DEC,cLi);
        if( merged && !merged->get( (const void *)_t_buf.ptr(),_t_buf.length() ) ) {
            continue;
        }
        clSum = getCl(cLi);
        if( clSum && pos == clSum->start && getValidCloneID( clSum->parentClID ) == clId) {
            *clInds->add() = cLi;
        }
    }
    return clInds->dim();
}

bool sViopop::isCloneAncestor(sBioseqpopul::cloneSummary * clA, sBioseqpopul::cloneSummary * clB)
{
    sBioseqpopul::cloneSummary * parentCl = 0;
    if( clB->hasParent() ) parentCl =  getCl(clB->parentClID);
    while( parentCl ) {
        if( parentCl && parentCl->clID==clA->clID )
            return true;
        clB=parentCl;
        parentCl=0;
        if( clB->hasParent() ) parentCl =  getCl(clB->parentClID);
    }
    return false;
}

bool sViopop::isCloneAncestor(sBioseqpopul::cloneSummary * t_sum, sDic<bool> & clones)
{
    sBioseqpopul::cloneSummary * t_cl=0;
    for( idx i = 0 ; i < clones.dim() ; ++i ) {
        t_cl = getCl( *(idx *)clones.id(i) );
        if( t_cl && isCloneAncestor(t_sum,t_cl)) {
            return true;
        }
    }
    return false;
}

bool sViopop::cloneOverlap(sBioseqpopul::cloneSummary * clA, sBioseqpopul::cloneSummary * clB)
{
    return sOverlap(clA->start,clA->end,clB->start,clB->end);
}

bool sViopop::cloneOverlap(sBioseqpopul::cloneSummary * clSum, sDic<bool> & clones)
{
    sBioseqpopul::cloneSummary * t_cl=0;
    for( idx i = 0 ; i < clones.dim() ; ++i ) {
        t_cl = getCl( *(idx *)clones.id(i) );
        if( cloneOverlap(clSum, t_cl) ) {
            return true;
        }
    }
    return false;
}


idx sViopop::getPermutationsCompositions( const char * clones_input, sVec< sVec <sViopop::cloneRegion> > & out, sVec< idx > & vclIds , ParamCloneIterator * params) {
    sDic<bool> clIds;
    sVec<idx> vec_clIds;

    sVec< sViopop::cloneRegion > tmp_suf, * c_Out, * t_Out, s_Out;
    cloneRegion * t_Contig;

    if(clones_input) {
        sString::scanRangeSet(clones_input, 0, &vec_clIds, 0, 0, 0);
    }

    for( idx i = 0 ; i < vec_clIds.dim() ; ++i ){
        *clIds.set( (void * )vec_clIds.ptr(i), sizeof(idx) ) = true;
    }
    vec_clIds.empty();
    sStack<sBioseqpopul::cloneSummary> clonesParsed;
    sStack< sVec <sViopop::cloneRegion> > permStack;
    sBioseqpopul::cloneSummary lastcl, * cl, rsCl, * t_cl=0, rsTCl, rsMergedCl;
    sVec<contigComp> comp;

    sStr  t_buf;

    idx pos = 0, cnt=0;

    bool again = true;
    while( again ){
        getClonesOnPosition(this,pos++,&vec_clIds);
        again = !vec_clIds.dim();
        for( idx i = 0 ; i < vec_clIds.dim() && mergeDict ; ++i ) {
            if( !mergeDict->get( t_buf.printf(0,"%" DEC,vec_clIds[i]) ) ) {
                again = true;
                break;
            }
        }
    }
    --pos;
    for (idx c = 0 ; c < vec_clIds.dim() ; ++c){
        if( mergeDict && !mergeDict->get( t_buf.printf(0,"%" DEC,vec_clIds[c]) ) )
            continue;
        else
            vclIds.vadd(1,vec_clIds[c]);

        lastcl.clID = vec_clIds[c];
        lastcl.start = pos;
        *clonesParsed.push() = lastcl;
        c_Out = permStack.push();
        t_Contig = c_Out->add();
        t_Contig->contigInd = lastcl.clID;
        t_Contig->start = pos - lastcl.start;

    }

    bool found = false;

    sBioseqpopul::cloneStats * gen = getGenStats();

    while( clonesParsed.dim() && cnt<params->cnt ) {
        lastcl = *clonesParsed.top();
        s_Out = *permStack.top();
        c_Out = &s_Out;

        clonesParsed.pop();
        permStack.pop();

        cl = getCl(lastcl.clID);
        if( mergeClone(cl,rsCl,comp,params) ) {
            cl = &rsCl;
        }
        else if (!isValidClone(cl->clID)) {
            break;
        }

        found = clIds.find( (void *)&cl->clID, sizeof (idx) );

        for( pos = lastcl.start; pos < gen->size ; ++pos ) {
            if( !found && pos && getBifurcatedClonesOnPosition(pos,&vec_clIds,cl->clID) ) {
                for( idx j = 0 ; j < vec_clIds.dim() ; ++j ) {
                    t_cl = getCl(vec_clIds[j]);
                    if( mergeClone(t_cl,rsTCl,comp,params) ) {
                        t_cl = &rsTCl;
                    }
                    else if (!isValidClone(t_cl->clID)) {
                        continue;
                    }

                    if( t_cl->clID == cl->clID || ( mergeDict && !mergeDict->get( t_buf.printf(0,"%" DEC,vec_clIds[j]) ) ) ) {
                        continue;
                    }
                    if( clIds.find( (void *)&t_cl->clID, sizeof (idx) ) || isCloneAncestor(t_cl, clIds) ) {
                        cl = t_cl;

                        c_Out->ptr(c_Out->dim()-1)->end = pos - getCl(c_Out->ptr(c_Out->dim()-1)->contigInd)->start;
                        t_Contig = c_Out->add();
                        t_Contig->contigInd = cl->clID;
                        t_Contig->start = pos - cl->start;
                        if( clIds.find( (void *)&cl->clID, sizeof (idx) ) ) {
                            found = true;
                        }
                        break;
                    }
                    if( !cloneOverlap( t_cl , clIds) ) {
                        clonesParsed.top()->start = pos+1;
                        lastcl.start = pos;
                        lastcl.clID = t_cl->clID;
                        *clonesParsed.push() = lastcl;

                        t_Out = permStack.push();
                        t_Out->copy( c_Out );
                        if( t_Out->dim() ) {
                            t_Out->ptr(t_Out->dim()-1)->end = pos - getCl(t_Out->ptr(t_Out->dim()-1)->contigInd)->start;
                        }
                        t_Contig = t_Out->add();
                        t_Contig->contigInd = lastcl.clID;
                        t_Contig->start = 0;
                    }
                }
            }

            if( pos >= cl->end ) {
                if( !cl->hasMerged() ) {
                    break;
                }
                t_cl = getValidMergingSummary(cl);
                if( t_cl ) {
                    cl = t_cl;
                    c_Out->ptr(c_Out->dim()-1)->end = pos - getCl(c_Out->ptr(c_Out->dim()-1)->contigInd)->start;
                    t_Contig = c_Out->add();
                    t_Contig->contigInd = cl->clID;
                    t_Contig->start = pos - cl->start;
                    found = clIds.find( (void *)&cl->clID );
                }
            }
        }

        cl = getCl( c_Out->ptr(c_Out->dim()-1)->contigInd );
        if( mergeClone(cl,rsCl,comp,params) ) {
            cl = &rsCl;
        }
        else if (!isValidClone(cl->clID)) {
            break;
        }
        c_Out->ptr(c_Out->dim()-1)->end = cl->end - cl->start;

        idx idsFnd = 0;
        for(idx ic = 0 ; ic < c_Out->dim() ; ++ic ) {
            if( clIds.find( (void *)&c_Out->ptr(ic)->contigInd, sizeof (idx) ) )
                ++idsFnd;
            if(clIds.dim() == idsFnd) break;
        }
        if( clIds.dim() == idsFnd ){
            t_Out = out.add();
            t_Out->copy(c_Out);
            ++cnt;
        }

    }
    return cnt;
}

idx sViopop::getExtendedComposition( sBioseqpopul::cloneSummary * cl, sVec<sViopop::cloneRegion> * c_Out, ParamCloneIterator * params)
{
    if(!c_Out || !cl) {
        return 0;
    }

    sVec<contigComp> comp;
    sBioseqpopul::cloneSummary rsCl, * parentCl=0, rsParnentCl, * mergeCl = 0, rsMergedCl, * orCl = 0;
    sStr title;
    sVec< sViopop::cloneRegion > tmp_suf;
    cloneRegion * t_Contig;

    if( mergeClone(cl,rsCl,comp,params) ) {
        cl = &rsCl;
    }
    else if (!isValidClone(cl->clID)) {
        return 0;
    }

    parentCl = 0; mergeCl = 0;
    if( cl->hasParent() ) {
        parentCl =  getValidParentSummary(cl);
        if( parentCl && mergeClone(parentCl,rsParnentCl,comp,params) ) {
            parentCl = &rsParnentCl;
        }
        else if (!parentCl || !isValidClone(parentCl->clID)) {
             parentCl = 0;
        }
    }

    tmp_suf.cut(0);

    orCl = cl;
    while( parentCl ){
        if( parentCl->start < cl->start ) {
            t_Contig = tmp_suf.add();
            t_Contig->contigInd = parentCl->clID;
            t_Contig->start = parentCl->start - parentCl->start;
            t_Contig->end = cl->start - parentCl->start;
        }

        cl=parentCl;
        parentCl = 0;
        if( cl->hasParent() ) {
            parentCl = getCl(cl->parentClID);
            if(!parentCl) break;
            if( mergeClone(parentCl,rsParnentCl,comp,params) ) {
                parentCl = &rsParnentCl;
            }
            else if (!isValidClone(parentCl->clID)) {
                parentCl = 0;
            }
        }

    }
    for (idx i = tmp_suf.dim()-1 ; i >=0 ; --i ) {
        c_Out->vadd(1,tmp_suf[i]);
    }

    cl = orCl;
    t_Contig = c_Out->add();
    t_Contig->contigInd = cl->clID;
    t_Contig->start = cl->start - cl->start;
    t_Contig->end = cl->end - cl->start;

    if( cl->hasMerged() ) {
        mergeCl =  getCl(cl->mergeclID);
        if(!mergeCl) return 0;
        if( mergeClone(mergeCl,rsMergedCl,comp,params) ) {
            mergeCl = &rsMergedCl;
        }
        else if (!isValidClone(mergeCl->clID)) {
            mergeCl = 0;
        }
    }

    while( mergeCl ){
        if( mergeCl->end > cl->end ) {

            t_Contig = c_Out->add();
            t_Contig->contigInd = mergeCl->clID;
            t_Contig->start = cl->end - mergeCl->start;
            t_Contig->end = mergeCl->end - mergeCl->start;
        }

        cl=mergeCl;
        mergeCl = 0;
        if( cl->hasMerged() ) {
            mergeCl = getCl(cl->mergeclID);
            if(!mergeCl) break;
            if( mergeClone(mergeCl,rsMergedCl,comp,params) ) {
                mergeCl = &rsMergedCl;
            }
            else if (!isValidClone(mergeCl->clID)) {
                mergeCl = 0;
            }
        }
    }

    return c_Out->dim();
}

idx sViopop::getExtendedCompositions( const char * clones_input, sVec< sVec <sViopop::cloneRegion> > & out, sVec< idx > & clIds, ParamCloneIterator * params ) {
    sVec<idx> vec_clIds;
    sBioseqpopul::cloneSummary * cl;

    if(clones_input) {
        sString::scanRangeSet(clones_input, 0, &vec_clIds, 0, 0, 0);
    }
    idx clCnt = vec_clIds.dim() ? vec_clIds.dim() : dimCl();
    idx iC = 0, i = 0;

    for( i = 0 ; i < clCnt && params->cnt ; ++i ){
        iC = vec_clIds.dim()?vec_clIds[i]:i;
        cl = getCl(iC);
        if ( !getExtendedComposition( cl, out.add(), params) )
            out.del(out.dim()-1);
        else
            clIds.vadd(1,cl->clID);
    }

    return clCnt;
}

idx sViopop::getContigCompositions( const char * clones_input, sVec< sVec <sViopop::cloneRegion> > & out, sVec< idx > & clIds, ParamCloneIterator * params ) {
    sVec<idx> vec_clIds;
    sBioseqpopul::cloneSummary * cl;

    if(clones_input) {
        sString::scanRangeSet(clones_input, 0, &vec_clIds, 0, 0, 0);
    }
    idx clCnt = vec_clIds.dim() ? vec_clIds.dim() : dimCl();
    idx iC = 0;

    for( idx i = 0 ; i < clCnt && i < params->cnt ; ++i ){
        iC = vec_clIds.dim()?vec_clIds[i]:i;
        cl = getCl(iC);
        if ( !getContigComposition( cl, out.add(), params) )
            out.del(out.dim()-1);
        else
            clIds.vadd(1,cl->clID);
    }
    return clCnt;
}

idx sViopop::getContigComposition( sBioseqpopul::cloneSummary * cl, sVec<sViopop::cloneRegion> * c_Out, ParamCloneIterator * params ) {
    sVec<idx> vec_clIds;
    sBioseqpopul::cloneSummary rsCl;
    sVec<contigComp> comp;
    cloneRegion * t_Contig;

    if( mergeClone(cl,rsCl,comp,params) ) {
        cl = &rsCl;
    }
    else if (!isValidClone(cl->clID)) {
        return 0;
    }

    t_Contig = c_Out->add();
    t_Contig->contigInd = cl->clID;
    t_Contig->start = cl->start - cl->start;
    t_Contig->end = cl->end - cl->start;
    return c_Out->dim();
}
struct clfr {
    sBioseqpopul::cloneSummary iCl;real fr;idx normalized_len; idx prv_i;
    clfr(){sSet(this, 0);prv_i=-1;};
};

struct rangeGraphNode {
    idx start, end;
    idx length(void){return end-start;}
    sVec<clfr> cls;
    idx getMostDominantContig(){
        real maxfr = 0;idx maxC = 0;
        for(idx i = 0 ; i < cls.dim() ; ++i) {
            if( cls[i].fr > maxfr ){
                maxfr = cls[i].fr;
                maxC = i;
            }
        }
        return maxC;
    }
    idx getICl(idx clID) {
        for (idx i = 0 ; i < cls.dim() ; ++i ) {
            if(cls[i].iCl.clID==clID)
                return i;
        }
        return -1;
    }
    void scale(real minF) {
        real sum=0;
        for(idx i = 0 ; i < cls.dim() ; ++i) sum += cls[i].fr;
        for(idx i = 0 ; i < cls.dim() ; ++i) cls[i].fr *= (sum-minF)/sum;
    }
    void subtract(real minF) {
        for(idx i = 0 ; i < cls.dim() ; ++i) cls[i].fr -= minF;
    }
    idx getContigCnt(real minF) {
        idx iR = 0;
        for(idx i = 0 ; i < cls.dim() ; ++i ) {
            if(cls[i].fr > minF) {
                iR++;
            }
        }
        return iR;
    }
    bool isForwardConnection(sBioseqpopul::cloneSummary * current, sBioseqpopul::cloneSummary * next)
    {
        return (next->parentClID == current->clID && next->start==start) || (next->clID == current->mergeclID && current->end==start);
    }
    bool isBackwardConnection(sBioseqpopul::cloneSummary * current, sBioseqpopul::cloneSummary * previous)
    {
        return (previous->clID == current->parentClID && current->start==end) || (previous->mergeclID == current->clID && previous->end==end);
    }
    bool isForwardRelated(sBioseqpopul::cloneSummary * current, sBioseqpopul::cloneSummary * next)
    {
        return (next->parentClID == current->clID ) || (next->clID == current->mergeclID );
    }
    bool isBackwardRelated(sBioseqpopul::cloneSummary * current, sBioseqpopul::cloneSummary * previous)
    {
        return (previous->clID == current->parentClID ) || (previous->mergeclID == current->clID );
    }
    bool isConnected(idx iNode, rangeGraphNode * gn) {
        sBioseqpopul::cloneSummary * cc = &cls[iNode].iCl;
        bool forward = (gn->start > start);
        for(idx i = 0 ; i < gn->cls.dim() ; i++) {
            sBioseqpopul::cloneSummary * ic = &cls[i].iCl;
            if ( forward && isForwardConnection(cc,ic) )
                return true;
            else if ( !forward && isBackwardConnection(cc,ic) )
                return true;
        }
        return false;
    }
    idx getClosestContig(sViopop * vp, sBioseqpopul::cloneSummary * cc,real fr, bool forward, real minF){
        sBioseqpopul::cloneSummary * ic = 0;
        real minFrDistance = REAL_MAX; idx iR = -1;
        bool unconnected = true, unrelated = true;
        for(idx i = 0 ; i < cls.dim() ; ++i) {
            ic = &cls[i].iCl;
            if( ( cls[i].iCl.clID == cc->clID ) || (forward && isForwardConnection(cc,ic) ) || ( !forward && isBackwardConnection(cc,ic) ) ) {
                unconnected =false;
                if( cls[i].fr > minF ) {
                    if( sAbs(fr - cls[i].fr) < minFrDistance ) {
                        minFrDistance = sAbs(fr - cls[i].fr);
                        iR = i;
                    }
                }
            }
        }
        if( iR < 0 && !unconnected ) {
            for(idx i = 0 ; i < cls.dim() ; ++i) {
                ic = &cls[i].iCl;
                if( ( cls[i].iCl.clID == cc->clID ) || (forward && isForwardRelated(cc,ic) ) || ( !forward && isBackwardRelated(cc,ic) ) ) {
                    unrelated =false;
                    if( cls[i].fr > minF ) {
                        if( sAbs(fr - cls[i].fr) < minFrDistance ) {
                            minFrDistance = sAbs(fr - cls[i].fr);
                            iR = i;
                        }
                    }
                }
            }
        }
        if( iR < 0 && !unrelated ) {
            for(idx i = 0 ; i < cls.dim() ; ++i) {
                ic = &cls[i].iCl;
                if( cls[i].fr > minF ) {
                    if( sAbs(fr - cls[i].fr) < minFrDistance ) {
                        minFrDistance = sAbs(fr - cls[i].fr);
                        iR = i;
                    }
                }
            }
        }
        return iR;
    }
    rangeGraphNode(){start = end = 0;}
};

real getPath(sViopop * vp, sVec<rangeGraphNode> & grph, idx iNode, sVec<sViopop::cloneRegion> & out, real t_minF) {
    sVec<idx> iClGr(sMex::fSetZero|sMex::fExactSize);
    iClGr.add(grph.dim());
    iClGr[iNode] = grph[iNode].getMostDominantContig();

    real iF = grph[iNode].cls[iClGr[iNode]].fr;
    real minF = iF;
    idx start_node = -1, end_node = grph.dim();
    for(idx i = iNode - 1 ; i >= 0 ; i-- ) {
        iClGr[i] = grph[i].getClosestContig(vp, &grph[i+1].cls[iClGr[i+1]].iCl,iF,(iNode < i),t_minF );
        if(iClGr[i] < 0 ) {
            start_node = i;
            break;
        }
        if(grph[i].cls[iClGr[i]].fr < minF) {
            minF = grph[i].cls[iClGr[i]].fr;
        }
    }
    for(idx i = iNode + 1 ; i < grph.dim() ; i++ ) {
        iClGr[i] = grph[i].getClosestContig(vp, &grph[i-1].cls[iClGr[i-1]].iCl,iF, (iNode < i),t_minF );
        if(iClGr[i] < 0 ) {
            end_node = i;
            break;
        }
        if( grph[i].cls[iClGr[i]].fr < minF) {
            minF = grph[i].cls[iClGr[i]].fr;
        }
    }
    for(idx i = 0 ; i <= start_node; ++i) {
        grph[i].subtract(minF);
    }
    for(idx i = end_node ; i < grph.dim(); ++i) {
        grph[i].subtract(minF);
    }
    sViopop::cloneRegion * cr = 0;
    for(idx i = start_node+1 ; i < end_node ; ++i ) {
        cr = out.add();
        cr->contigInd = grph[i].cls[iClGr[i]].iCl.clID;
        cr->start = grph[i].start - grph[i].cls[iClGr[i]].iCl.start;
        cr->end = grph[i].end - grph[i].cls[iClGr[i]].iCl.start;
        cr->freq = minF/grph[i].cls[iClGr[i]].fr;
        grph[i].cls[iClGr[i]].fr -= minF;
    }
    return minF;
}
idx getSeedNode(sVec<rangeGraphNode> & grph, real minF ){
    idx maxCnt = 0, crCnt = 0, iS = -1;
    for (idx i = 0 ; i < grph.dim() ; ++i ) {
        crCnt = grph[i].getContigCnt(minF);
        if(  crCnt && crCnt > maxCnt ) {
            maxCnt = crCnt;
            iS= i;
        }
    }
    return iS;
}

void normalizeRangeGraph(sVec<rangeGraphNode> &rangeGraph)
{
    for(idx i = 1 ; i < rangeGraph.dim() ; ++i) {
        rangeGraphNode * gn = rangeGraph.ptr(i), * t_gn = 0;
        rangeGraphNode * prv_gn =rangeGraph.ptr(i-1);
        for (idx p_j = 0 ; p_j < prv_gn->cls.dim() ; ++p_j ) {
            real norm_len = 0;
            if( prv_gn->cls[p_j].normalized_len == 0 ) {
                if ( !prv_gn->isConnected(p_j,gn) ) {
                    idx j = gn->getICl(prv_gn->cls[p_j].iCl.clID);
                    if(j < 0 )continue;
                    if( prv_gn->cls[p_j].normalized_len == 0 )
                        norm_len = (prv_gn->length());
                    else
                        norm_len = prv_gn->cls[p_j].normalized_len;
                    gn->cls[j].fr = ((gn->cls[j].fr * gn->length()) + (prv_gn->cls[p_j].fr*norm_len))/(gn->length()+norm_len);
                    gn->cls[j].normalized_len = gn->length()+norm_len;
                    gn->cls[j].prv_i = p_j;
                    t_gn = gn;
                    while( gn->cls[j].prv_i >= 0) {
                        prv_gn = gn-1;
                        prv_gn->cls[gn->cls[j].prv_i].fr = gn->cls[j].fr;
                        j = gn->cls[j].prv_i;
                        gn = gn - 1;
                    }
                    gn = t_gn;
                } else {
                    prv_gn->cls[p_j].normalized_len=-1;
                }
            }
        }
    }
    for(idx i = 0 ; i < rangeGraph.dim() ; ++i) {
        rangeGraphNode * gn = rangeGraph.ptr(i);
        real tot_sum = 0, const_sum = 0;
        for (idx j = 0 ; j < gn->cls.dim() ; ++j ) {
            tot_sum += gn->cls[j].fr;
            if( gn->cls[j].normalized_len >=0 ) {
                const_sum += gn->cls[j].fr;
            }
        }
        real coef = (tot_sum!=const_sum) ? (1-const_sum)/(tot_sum-const_sum): 1;
        for (idx j = 0 ; j < gn->cls.dim() && tot_sum!=const_sum ; ++j ) {
            if( gn->cls[j].normalized_len < 0 ) {
                gn->cls[j].fr *= coef;
            }
        }
        tot_sum = 0;
        for (idx j = 0 ; j < gn->cls.dim() ; ++j ) tot_sum += gn->cls[j].fr;
    }
}

idx sViopop::getPredictedGlobal(sVec< sVec <sViopop::cloneRegion> > & out, sVec< real > & frequencies, ParamCloneIterator * params)
{
    popGraph vpGrph(this);
    if( !vpGrph.buildGraph(params) )
        return 0;

    bool min_diversity = params->minDiv;
    sDic< sVec< sViopop::cloneRegion > > flows;
    idx cnt=0;
    for(idx i = 0 ; i < params->mc_iters ; i++ ) {
        idx seed_ind = vpGrph.getSeed(min_diversity);
        sVec< cloneRegion > c_flow, * dicted_c_flow;
        vpGrph.traverse(seed_ind,c_flow, min_diversity);

        sStr ids;
        for(idx i = 0 ; i < c_flow.dim(); ++i)
            ids.printf("_%" DEC,c_flow[i].contigInd);

        if( !flows.find(ids.ptr(),ids.length()) ) {
            dicted_c_flow = flows.set(ids.ptr(),ids.length());
            c_flow[0].freq = 0;
            dicted_c_flow->copy(&c_flow);
        }
        flows.get(ids.ptr())->ptr(0)->freq++;
        ++cnt;
    }
    real freq_sum = 0, cr_freq = 0;
    sVec<real> l_freq;
    for(idx i = 0 ; i < flows.dim() ; ++i) {
        sVec<cloneRegion> * c_fl = flows.ptr(i);
        cr_freq = c_fl->ptr(0)->freq/cnt;
        if( cr_freq > params->minF )
            freq_sum += cr_freq;
        l_freq.vadd(1,c_fl->ptr(0)->freq/cnt);
        for(idx j = 0 ; j < c_fl->dim() ; ++j ) {
            c_fl->ptr(j)->freq = 1;
        }
    }
    sVec<idx> freq_sort_ind(sMex::fExactSize);
    freq_sort_ind.resize(l_freq.dim());
    sSort::sort(l_freq.dim(),l_freq.ptr(),freq_sort_ind.ptr());
    for(idx i = flows.dim() -1 ; i >= 0  ; --i) {
        if( l_freq[freq_sort_ind[i]] < params->minF )
            break;
        sVec<sViopop::cloneRegion> * clr = out.add();
        clr->copy(flows.ptr(freq_sort_ind[i]));
        frequencies.vadd(1,100*l_freq[freq_sort_ind[i]]/freq_sum);
    }

    return out.dim();
}

idx popGraph::buildGraph(sViopop::ParamCloneIterator * params)
{
    sVec< idx > rng_cl(sMex::fSetZero|sMex::fExactSize), sorted_cl_rngs(sMex::fSetZero);
    sBioseqpopul::cloneSummary * cl = 0;
    sVec<sViopop::contigComp> comp;
    sBioseqpopul::cloneSummary rsCl;
    for( idx i = 0 ; i < 2*_vp->dimCl() ; i+=2 ){
        cl = _vp->getCl(i/2);
        if( _vp->mergeClone(cl,rsCl,comp,params) ) {
            cl = &rsCl;
        }
        else if (!_vp->isValidClone(cl->clID)) {
            continue;
        }
        rng_cl.vadd(2,cl->start,cl->end);
    }
    sSort::sort(rng_cl.dim(),rng_cl.ptr());
    sorted_cl_rngs.vadd(1,rng_cl[0]);
    for(idx i = 1 ; i < rng_cl.dim() ; ++i ) {
        if( rng_cl[i-1] != rng_cl[i] )
            sorted_cl_rngs.vadd(1,rng_cl[i]);
    }
    idx c_start = 0, c_end = 0;
    sVec<idx> res, * p_res = 0;
    node * cn = 0;
    sVec<idx> rng_tot_cov(sMex::fSetZero|sMex::fExactSize);
    rng_tot_cov.resize(sorted_cl_rngs.dim()-1);
    for(idx i = 1 ; i < sorted_cl_rngs.dim() ; ++i ){
        c_end=sorted_cl_rngs[i];
        c_start=sorted_cl_rngs[i-1];
        res.cut(0);
        p_res = &res;
        sViopop::getClonesOnPosition(_vp,(c_start+c_end)/2,p_res);
        bool is_max_contig = ( res.dim() > _max_contigs.dim() );
        if( is_max_contig ) {
            _max_contigs.cut(0);
            _max_contigs_freqs.cut(0);
            _max_sum = 0;
        }
        sVec<sBioseqpopul::cloneSummary> cl_sums;
        sVec< sVec< sViopop::seqCovPosition > > seqCovs;
        cl_sums.resize(res.dim());
        seqCovs.resize(res.dim());
        for (idx j = 0 ; j < res.dim() ; ++j ) {
            if( !_vp->mergeClone(_vp->getCl(res[j]),cl_sums[j],comp,0,seqCovs.ptr(j)) ) {
                seqCovs.ptr(j)->cut(0);
                cl_sums[j] = *_vp->getCl(res[j]);
            }
        }
        sViopop::seqCovPosition * sc = 0;
        idx seqlen = 0;
        for (idx j = 0 ; j < res.dim() ; ++j ) {
            sBioseqpopul::cloneSummary cl_sum = cl_sums[j];
            if(seqCovs[j].dim()) {
                sc = seqCovs[j].ptr();
                seqlen = seqCovs[j].dim();
            } else {
                sc = _vp->getSeqCov(cl_sum.clID,&seqlen,0);
            }

            if( cl_sum.start == c_start ) {
                cn = addNode(c_start,c_end,i-1,cl_sum);
                if( cn->clone.hasParent() ) {
                    idx i_pnode = getNodeIndFromRange( cn->clone.parentClID, c_start);
                    if( i_pnode >= 0 )
                        connectNodes(_nodes.ptr(i_pnode), cn);
                }
            } else {
                sVec<idx> merged_res;
                idx merged_cnt = findMerged(c_start,&cl_sum,merged_res);
                idx bifurc_cnt = 0;
                for(idx k = 0 ; k < cl_sums.dim() ; ++k) {
                    if( cl_sums[k].start == c_start && cl_sums[k].hasParent() && cl_sums[k].parentClID == cl_sum.clID)
                        ++bifurc_cnt;
                }
                idx i_pnode = getNodeIndFromRange(cl_sum.clID,c_start);
                if( i_pnode >= 0 ) {
                    if( !merged_cnt && !bifurc_cnt ) {
                        cn = _nodes.ptr(i_pnode);
                        extendFrw(cn, c_end, i - 1);
                    } else {
                        cn = addNode(c_start, c_end, i - 1, cl_sum);
                        for(idx l = 0; l < merged_res.dim(); ++l) {
                            connectNodes(_nodes.ptr(merged_res[l]), cn);
                        }
                        connectNodes(_nodes.ptr(i_pnode), cn);
                    }
                }
            }
            if( is_max_contig ) {
                _max_contigs.vadd(1,cn - _nodes.ptr());
            }
            real cr_cov = 0;
            for(idx ic = c_start - cn->clone.start ; ic < c_end - cn->clone.start ; ++ic ) {
                cr_cov += (real)sc[ic].coverage();
            }
            cn->cov += cr_cov;
            rng_tot_cov[i-1] += cr_cov;
            if( is_max_contig ) {
                _max_contigs_freqs.vadd(1,(cr_cov)/cn->length());
                _max_sum += *_max_contigs_freqs.last();
            }
        }
    }
    _tot_sum = 0;
    for( idx i = 0 ; i < _nodes.dim() ; ++i ) {
        real cr_rng_tot = 0;
        for(idx j = _nodes[i].irng_s ; j < _nodes[i].irng_e ; ++j) {
            cr_rng_tot += rng_tot_cov[j];
        }
        _nodes[i].cov /= cr_rng_tot;
        _tot_sum += _nodes[i].length()*_nodes[i].cov;
    }

    _max_contigs_freqs.cut(0);
    _max_sum = 0;
    for( idx i = 0 ; i < _max_contigs.dim() ; ++i ) {
        _max_contigs_freqs.vadd(1,_nodes[_max_contigs[i]].cov);
        _max_sum += _nodes[_max_contigs[i]].cov;
    }

    return _nodes.dim();
}
idx popGraph::traverse(idx seed_ind, sVec<sViopop::cloneRegion> & out, bool min_diversity)
{
    if( seed_ind < 0 || seed_ind >= _nodes.dim() )
        return 0;
    node * cn = _nodes.ptr(seed_ind);
    node * seed_n = cn;
    sVec<sViopop::cloneRegion> t_out;
    while(cn) {
        sViopop::cloneRegion * cr;
        if( t_out.dim() && t_out.last()->contigInd == cn->clone.clID) {
            t_out.last()->start = cn->start - cn->clone.start;
        } else {
            cr = t_out.add();
            cr->start = cn->start - cn->clone.start;
            cr->end = cn->end - cn->clone.start;
            cr->contigInd = cn->clone.clID;
        }
        cn = getPrevNode(cn,min_diversity?seed_n->cov:0);
    }
    for(idx i = t_out.dim()-1 ; i >= 0 ; --i ) {
        out.vadd(1,t_out[i]);
    }
    cn = getNextNode(seed_n,min_diversity?seed_n->cov:0);
    while(cn) {
        sViopop::cloneRegion * cr;
        if( out.dim() && out.last()->contigInd == cn->clone.clID) {
            out.last()->end = cn->end - cn->clone.start;
        } else {
            cr = out.add();
            cr->start = cn->start - cn->clone.start;
            cr->end = cn->end - cn->clone.start;
            cr->contigInd = cn->clone.clID;
        }
        cn = getNextNode(cn,min_diversity?seed_n->cov:0);
    }
    return out.dim();
}

void sViopop::getDiversityMeasureUnits(sVec< sVec< sViopop::cloneRegion > > & compositions, ParamCloneIterator * param, sVec<real> & avCovs, sVec<sVec<seqCovPosition> > & all_seqs)
{
    sBioseqpopul::cloneStats * stats=getGenStats();

    avCovs.init(sMex::fSetZero|sMex::fExactSize);
    avCovs.addM(compositions.dim());

    all_seqs.init(sMex::fExactSize);
    all_seqs.add(compositions.dim());

    cloneRegion * t_region;
    seqCovPosition * tSC = 0;
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    real totAvCovs = 0;

    for(idx ic = 0; ic < compositions.dim(); ++ic) {
        real & avCov = avCovs[ic];avCov = 0;
        all_seqs.ptr(ic)->init(sMex::fExactSize|sMex::fSetZero);
        all_seqs.ptr(ic)->add(stats->size);
        sVec<seqCovPosition> * seq = all_seqs.ptr(ic);
        sVec<cloneRegion> & composition = compositions[ic];
        if( composition.dim() <= 0 ) continue;
        sVec< seqCovPosition > seqCov ;
        idx pos = 0;

        sVec<contigComp> comp;
        idx sq_size = 0, start = 0 , offset = 0,len = 0, s_pos = 0;
        sStr title;
        for( idx i = 0 ; i < composition.dim() ; ++i )
        {
            t_region = composition.ptr(i);
            cl = getCl( t_region->contigInd );
            if(!cl) continue;

            if( mergeClone(cl,rsCl,comp,param,&seqCov) ) {
                cl = &rsCl;
                tSC = seqCov.ptr(0);
            }
            else if (!isValidClone(cl->clID)) {
                continue;
            }
            else {
                tSC = getSeqCov ( cl->clID, &sq_size );
            }
            start = getContigPos(cl, t_region->start, 0, true);
            if(start==-1) {
                continue;
            }


            for (idx i = pos ; i < cl->start + t_region->start ; ++i ) {
                seq->ptr(s_pos++)->setBase(sBioseqpopul::baseDel);
            }

            for( pos = t_region->start ; pos < t_region->end ; ++pos, ++offset )
            {
                if( getContigPos(cl,pos) < 0 ) {
                    seq->ptr(s_pos++)->setBase(sBioseqpopul::baseDel);
                    --offset;
                } else {
                    if( isAnyGap(cl,pos) || tSC[pos].base()==4 || (param->covThrs && param->covThrs >= tSC[pos].coverage() ) ){
                        seq->ptr(s_pos++)->setBase(sBioseqpopul::baseDel);
                    }
                    else {
                        seq->ptr(s_pos++)->baseCov =  tSC[pos].baseCov;
                        avCov += tSC[pos].coverage()*t_region->freq;
                        ++len;
                    }
                }
            }
            pos += cl->start;
        }

        for( ; pos < stats->size ; ++pos ) {
            seq->ptr(s_pos++)->setBase(sBioseqpopul::baseDel);
        }
        if(avCov) {
            avCov/=len;
            totAvCovs += avCov;
        }
    }
}

void sViopop::printContigsDiversityMeasurements(sVec< sVec< sViopop::cloneRegion > > & compositions, sVec<idx> & clIds, sStr & out, ParamCloneIterator * param)
{
    sBioseqpopul::cloneStats * stats=getGenStats();
    idx pair_cnt = (compositions.dim() * (compositions.dim()-1))/2;


    sVec< sVec<seqCovPosition> > all_seqs;
    sVec<real> avCovs, norm_avCovs;

    getDiversityMeasureUnits(compositions,param,avCovs,all_seqs);

    norm_avCovs.copy(&avCovs);
    sAlgebra::matrix::normalizeCols(norm_avCovs.ptr(),1,norm_avCovs.dim(), true);

    sMatrix pair_dist(sMex::fSetZero|sMex::fExactSize);
    pair_dist.resize(pair_cnt, pair_cnt);

    real mfmin = 0, mfe = 0, mfmax = 0;
    seqCovPosition * p1 = 0, * p2 = 0;
    idx calls[5], calls_cov[5], s1, s2, max_call_cov = 0 , cons_base = sBioseqpopul::baseDel;
    idx dim_calls = sDim(calls), size_calls = sizeof(calls);
    idx base_call = 0, tot_cov = 0;
    for(idx i = 0 ; i < stats->size ; ++i) {
        sSet(calls, 0, size_calls);
        sSet(calls_cov, 0, size_calls);

        max_call_cov = 0;
        cons_base = 0;

        for(s1 = 0 ; s1 < all_seqs.dim() ; ++s1) {
              p1 = all_seqs[s1].ptr(i);
              base_call = p1->isAnyGap()?sBioseqpopul::baseDel:p1->base();
              calls[base_call]++;
              calls_cov[base_call] += p1->coverage();

              tot_cov += calls_cov[base_call];

              for(idx c = 0 ; c < dim_calls; ++c) {
                  if( calls_cov[c] > max_call_cov) {
                      cons_base = c;
                  }
                  if( calls_cov[c] ) {
                      ++mfmin;
                      mfe += calls[c];
                      mfmax += calls_cov[c];
                  }
              }
              if( mfmin ) {
                  mfe -= calls[cons_base];
                  mfmax -= calls_cov[cons_base];
              }


              for(s2 = s1 ; s2 < all_seqs.dim() ; ++s2) {
                  p2 = all_seqs[s2].ptr(i);
                  if( !((p1->isAnyGap() && p2->isAnyGap() ) || p1->base() == p2->base()) ) {
                      ++pair_dist.val(s1,s2);
                  }
              }
        }
    }
    real nucl_diversity = 0, fad = 0;

    for(s1 = 0 ; s1 < all_seqs.dim() ; ++s1) {
        for(s2 = s1 ; s2 < all_seqs.dim() ; ++s2) {
            nucl_diversity += norm_avCovs[s1] * norm_avCovs[s2] * pair_dist.val(s1,s2);
            fad += pair_dist.val(s1,s2);
        }
    }

    idx haplotype_cnt = all_seqs.dim();
    idx length = all_seqs.dim();
    idx read_cnt = *(idx *)param->userPointer;
    out.printf("Reads,"
        "Haplotypes,"
        "Mfmin,"
        "Mfe,"
        "Mfmax,"
        "Shannon,"
        "Shannon log(Reads) normalized,"
        "Shannon log(Haplotypoes) normalized,"
        "Shannon bias corrected,"
        "population nucleotide diversity,"
        "sample diversity,"
        "Simpsons index,"
        "GS index (reads normalized),"
        "FAD\n");


    real shannonEntr = sStat::shannonEntropy(norm_avCovs);
    real simpsIndex = sStat::simpsonIndex(norm_avCovs);
    real read_norm_coef = (real)read_cnt/(read_cnt-1);
    out.printf("%" DEC ",",read_cnt);
    out.printf("%" DEC ",",haplotype_cnt);
    out.printf("%lf,",tot_cov?(real)mfmin/tot_cov:0);
    out.printf("%lf,",(length && haplotype_cnt)?(real)mfe/(length * haplotype_cnt):0);
    out.printf("%lf,",tot_cov?(real)mfmax/tot_cov:0);
    out.printf("%lf,",shannonEntr);
    out.printf("%lf,",read_cnt?shannonEntr/log10(read_cnt):0);
    out.printf("%lf,",haplotype_cnt?shannonEntr/log10(haplotype_cnt):0);
    out.printf("%lf,",shannonEntr - (read_cnt?(real)(haplotype_cnt - 1)/(2*read_cnt):0));
    out.printf("%lf,",length?nucl_diversity/length:length);
    out.printf("%lf,",length?(read_norm_coef*((real)nucl_diversity)/length):length);
    out.printf("%lf,",simpsIndex);
    out.printf("%lf,",(1-simpsIndex)*read_norm_coef);
    out.printf("%lf",length?fad/length:length);
    out.addString("\n");
}

idx sViopop::printContigsPrevalence(sVec< sVec< sViopop::cloneRegion > > & compositions, sVec<idx> & clIds, sStr & out, ParamCloneIterator * param)
{
    if( compositions.dim() <= 0 ) return 0;
    sVec<real> avCovs(sMex::fSetZero|sMex::fExactSize);
    avCovs.add(compositions.dim());
    cloneRegion * t_region;
    seqCovPosition * tSC = 0;
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    sStr titles;
    real totAvCovs = 0;
    for(idx ic = 0; ic < compositions.dim(); ++ic) {
        real & avCov = avCovs[ic];avCov = 0;
        sVec<cloneRegion> & composition = compositions[ic];
        if( composition.dim() <= 0 ) continue;
        idx & cur_clId = clIds[ic];
        sVec< seqCovPosition > seqCov ;
        sVec<idx> gap2skpMap, * pG2S = 0;
        bool isSkpGaps = (param->flags&clPrintNoGapsFrame);
        if( isSkpGaps  ) {
            pG2S = &gap2skpMap;
        }
        sVec<contigComp> comp;
        idx sq_size = 0, start = 0 , end = 0, offset = 0,len = 0;
        sStr title;
        printTitle(title,param,cur_clId);
        for( idx i = 0 ; i < composition.dim() ; ++i )
        {
            t_region = composition.ptr(i);
            cl = getCl( t_region->contigInd );
            if(!cl) continue;

            if( mergeClone(cl,rsCl,comp,param,&seqCov, pG2S) ) {
                cl = &rsCl;
                tSC = seqCov.ptr(0);
            }
            else if (!isValidClone(cl->clID)) {
                avCov=0;
                break;
            }
            else {
                tSC = getSeqCov ( cl->clID, &sq_size );
            }
            start = getContigPos(cl, t_region->start, pG2S, true);
            end = getContigPos(cl, t_region->end-1, pG2S, true)+1;
            if(start==-1) {
                avCov=0;
                break;
            }

            printTitle(title,param, t_region->contigInd, offset + 1, end - start + offset,"|");

            for( idx pos = t_region->start ; pos < t_region->end ; ++pos, ++offset )
            {
                if( getContigPos(cl,pos, pG2S) < 0 ) {
                    --offset;
                    continue;
                }
                if( isAnyGap(cl,pos, pG2S) || tSC[pos].base()==4 || (param->covThrs && param->covThrs >= tSC[pos].coverage() ) ){
                    if( !isSkpGaps ) {
                        len++;
                    }
                    else {
                        --offset;
                        continue;
                    }
                }
                else {
                    len++;
                    avCov += tSC[pos].coverage()*t_region->freq;
                }
            }
        }
        if(avCov) {
            avCov/=len;
            totAvCovs += avCov;
            if(param->fastaTmplt){
                sString::searchAndReplaceStrings(&titles,param->fastaTmplt,0,"$_(v)" __,title.ptr(1),1,false);
                titles.shrink00();
            }
            else{
                titles.addString(title.ptr(1));
            }
            titles.add0();
        } else {
#ifdef _DEBUG
            ::printf("no cov?");
#endif
        }
    }
    titles.add0(2);
    idx validCovs = 0;
    idx cnt = param->userPointer?(*(idx*)(param->userPointer)):avCovs.dim();
    if( cnt > avCovs.dim() )
        cnt = avCovs.dim();
    for(idx ic = 0; ic < cnt ; ++ic) {
        if( avCovs[ic] ) {
            ++validCovs;
            out.printf("%s,%.4f,%.4f\n",sString::next00(titles,ic),avCovs[ic],param->frequencies?param->frequencies[ic]:(100*avCovs[ic]/totAvCovs));
        }
    }
    return validCovs;
}

idx sViopop::printContig( sVec< sVec< sViopop::cloneRegion > > & composition, sVec<idx> & clIds, sStr & out, ParamCloneIterator * params, idx print_type) {
    idx res = 0;
    switch (print_type) {
        case ePrintContigSeq:
            for( idx i = 0 ; i < composition.dim() ; ++i ) {
                res = printContigSequences(composition[i], out, params, clIds[i], 0);
            }
            break;
        case ePrintContigAl:
            for( idx i = 0 ; i < composition.dim() ; ++i ) {
                res = printContigAlignments(composition[i], out, params, clIds[i], 0);
            }
            break;
        case ePrintContigCov:
            for( idx i = 0 ; i < composition.dim() ; ++i ) {
                res = printContigCoverages(composition[i], out, params, clIds[i], 0);
            }
            break;
        case ePrintContigComp:
            for( idx i = 0 ; i < composition.dim() ; ++i ) {
                res = printContigComposition(composition[i], out, params, clIds[i], 0);
            }
            break;
        case ePrintContigBreakpoints:
            for( idx i = 0 ; i < composition.dim() ; ++i ) {
                res = printContigBreakpoints(composition[i], out, params, clIds[i], 0);
            }
            break;
        case ePrintContigSummary:
            params->out->printf("Clone ID,Start,End,Merged ID,Bifurcated ID,Bifurcation start,Bifurcation End,Supporting positions,Differences\n");
            for( idx i = 0 ; i < composition.dim() ; ++i ) {
                res = printContigSummary( composition[i], out, params);
            }
            break;
        case ePrintContigPrevalence:
            params->out->printf("Clone ID,Prevalence,Average Coverage\n");
            res = printContigsPrevalence(composition, clIds, out, params);
            break;
        case ePrintContigDiversityMeasures:

            printContigsDiversityMeasurements(composition,clIds,out,params);
            break;
        default:
            out.printf("Please specify printing option");
            break;
    }
    return res;
}
bool appendToLastComp(sStr & title,idx &contgId,idx &start, idx &end, const char * sep) {
    char * src = title.ptr();
    bool isAppended = false;
    char * last = sString::searchSubstring(src,title.length(),"contig_" __,sNotIdx,"",false);
    if( last ) {
        idx prevCntId,prevStart,prevEnd;
        idx ret = sString::bufscanf(last,title.last(), "contig_%" DEC "[%" DEC "-%" DEC "]",&prevCntId,&prevStart,&prevEnd);
        if(ret == 3 && prevCntId == contgId) {
            title.printf((idx)(last-src),"contig_%" DEC "[%" DEC "-%" DEC "]",contgId,prevStart,end);
            isAppended = true;
        }
    }
    if( !isAppended ) {
        title.printf("contig_%" DEC "[%" DEC "-%" DEC "]",contgId,start,end);
    }
    return isAppended;
}
const char * sViopop::printTitle(sStr & title, ParamCloneIterator * param, idx contgId, idx start, idx end, const char * sep)
{
    if( !sep )
        sep = ",";
    const char * seq_tag = "clone";
    if(param->flags&clPrintGlobal)
        seq_tag = "global";
    if( param && (param->flags & clPrintFastaTitleSimple) && !title.length() ) {
        title.printf("%s%s_%" DEC,sep,seq_tag,contgId);
    } else if( param && (param->flags & clPrintFastaTitleNumbersOnly)&& !title.length() ) {
        title.printf("%s%" DEC,sep,contgId);
    } else if ( !param || (param->flags & clPrintFastaTitleComposition) ) {
        if(start==end && title.length()==0 ) {
            title.printf("%s%s_%" DEC " | composition:", sep, seq_tag, contgId);
        } else {
            if( title.ptr(title.length()-1)[0] != ':') {
                title.printf("%s",sep);
            }
            appendToLastComp(title,contgId,start,end,sep);
        }
    }
    return title.ptr();
}

idx sViopop::printContigSequences( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx cur_clId, idx offset )
{
    if( composition.dim() <= 0 ) return 0;
    sViopop::cloneRegion * t_region;
    seqCovPosition * tSC = 0;
    sVec< seqCovPosition > seqCov ;
    sVec<idx> gap2skpMap, * pG2S = 0;
    bool isSkpGaps = (param->flags&sViopop::clPrintNoGapsFrame);
    if( isSkpGaps  ) {
        pG2S = &gap2skpMap;
    }
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    sVec<contigComp> comp;
    idx sq_size = 0, cnt = 0, start = 0 , end = 0, t_b = 0;
    sStr seq;

    sStr title;
    printTitle(title,param,cur_clId);
    for( idx i = 0 ; i < composition.dim() ; ++i )
    {
        t_region = composition.ptr(i);
        cl = getCl( t_region->contigInd );
        if(!cl) continue;

        if( mergeClone(cl,rsCl,comp,param,&seqCov, pG2S) ) {
            cl = &rsCl;
            tSC = seqCov.ptr(0);
        }
        else if (!isValidClone(cl->clID)) {
            return -1;
        }
        else {
            tSC = getSeqCov ( cl->clID, &sq_size );
        }
        start = getContigPos(cl, t_region->start, pG2S, true);
        end = getContigPos(cl, t_region->end-1, pG2S, true)+1;
        if(start==-1) {
            return -1;
        }

        printTitle(title,param, t_region->contigInd, offset + 1, end - start + offset);

        for( idx pos = t_region->start ; pos < t_region->end ; ++pos, ++offset )
        {
            if( getContigPos(cl,pos, pG2S) < 0 ) {
                --offset;
                continue;
            }
            t_b = tSC[pos].base();
            if( isAnyGap(cl,pos, pG2S) || t_b==4 || (param->covThrs && param->covThrs >= tSC[pos].coverage() ) ){
                if( !isSkpGaps ) {
                    seq.printf("-");
                }
                else {
                    --offset;
                    continue;
                }
            }
            else {
                seq.printf("%c",tSC[pos].baseChar());
            }
            if ( !((++cnt)%param->wrap) ) {
                seq.printf("\n");
            }
        }
    }
    out.addString(">");
    printTitleFreq(title, param, cur_clId);
    if(param->fastaTmplt){
        sString::searchAndReplaceStrings(&out,param->fastaTmplt,0,"$_(v)" __,title.ptr(1),1,false);
        out.shrink00();
    }
    else{
        out.addString(title.ptr(1));
    }
    out.printf("\n%s\n", seq.ptr());
    return offset;
}

idx sViopop::printContigAlignments( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx cur_clId, idx offset)
{
    if( composition.dim() <= 0 ) return 0;
    sViopop::cloneRegion * t_region;
    seqCovPosition * tSC = 0;
    sVec< seqCovPosition > seqCov ;
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    sBioseqpopul::cloneStats * stats=getGenStats();
    sVec<contigComp> comp;
    idx sq_size = 0, cnt = 0, start = 0 , end = 0, t_b = 0, pos = 0;
    sStr seq;

    sStr title;
    printTitle(title,param,cur_clId);
    for( idx i = 0 ; i < composition.dim() ; ++i )
    {
        t_region = composition.ptr(i);
        cl = getCl( t_region->contigInd );
        if(!cl) continue;

        if( mergeClone(cl,rsCl,comp,param,&seqCov) ) {
            cl = &rsCl;
            tSC = seqCov.ptr(0);
        }
        else if (!isValidClone(cl->clID)) {
            return -1;
        }
        else {
            tSC = getSeqCov ( cl->clID, &sq_size );
        }
        start = getContigPos(cl, t_region->start, 0, true);
        end = getContigPos(cl, t_region->end-1, 0, true)+1;
        if(start==-1) {
            return -1;
        }

        printTitle(title,param, t_region->contigInd, offset + 1, end - start + offset,"|");
        for (idx i = pos ; i < cl->start + t_region->start ; ++i ) {
            seq.addString("-");
            if ( !((++cnt)%param->wrap) ) {
                seq.printf("\n");
            }
        }
        for( pos = t_region->start ; pos < t_region->end ; ++pos, ++offset )
        {
            if( getContigPos(cl,pos) < 0 ) {
                seq.printf("-");
                --offset;
            } else {
                t_b = tSC[pos].base();
                if( isAnyGap(cl,pos) || t_b==4 || (param->covThrs && param->covThrs >= tSC[pos].coverage() ) ){
                    seq.printf("-");
                }
                else {
                    seq.printf("%c",tSC[pos].baseChar());
                }
            }
            if ( !((++cnt)%param->wrap) ) {
                seq.printf("\n");
            }
        }
        pos += cl->start;
    }
    for( ; pos < stats->size ; ++pos ) {
        seq.addString("-");
        if ( !((++cnt)%param->wrap) ) {
            seq.printf("\n");
        }
    }
    out.addString(">");
    printTitleFreq(title, param, cur_clId);
    if(param->fastaTmplt){
        sString::searchAndReplaceStrings(&out,param->fastaTmplt,0,"$_(v)" __,title.ptr(1),1,false);
        out.shrink00();
    }
    else{
        out.addString(title.ptr(1));
    }
    out.printf("\n%s\n", seq.ptr());
    return offset;
}

idx sViopop::printContigCoverages( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx cur_clId, idx offset)
{
    if( composition.dim() <= 0 ) return 0;
    sViopop::cloneRegion * t_region;
    seqCovPosition * tSC = 0;
    sVec< seqCovPosition > seqCov ;
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    sBioseqpopul::cloneStats * stats=getGenStats();
    sVec<contigComp> comp;
    idx sq_size = 0, start = 0 , end = 0, t_b = 0;
    sStr seq;

    sStr title;
    printTitle(title,param,cur_clId);
    idx pos = 0;
    for( idx i = 0 ; i < composition.dim() ; ++i )
    {
        t_region = composition.ptr(i);
        cl = getCl( t_region->contigInd );
        if(!cl) continue;

        if( mergeClone(cl,rsCl,comp,param,&seqCov) ) {
            cl = &rsCl;
            tSC = seqCov.ptr(0);
        }
        else if (!isValidClone(cl->clID)) {
            return -1;
        }
        else {
            tSC = getSeqCov ( cl->clID, &sq_size );
        }
        start = getContigPos(cl, t_region->start, 0, true);
        end = getContigPos(cl, t_region->end-1, 0, true)+1;
        if(start==-1) {
            return -1;
        }

        printTitle(title,param, t_region->contigInd, offset + 1, end - start + offset,"|");
        for( pos = t_region->start ; pos < t_region->end ; ++pos, ++offset )
        {
            if( getContigPos(cl,pos) < 0 ) {
                seq.printf(",0");
                --offset;
                continue;
            }
            t_b = tSC[pos].base();
            if( isAnyGap(cl,pos) || t_b==4 || (param->covThrs && param->covThrs >= tSC[pos].coverage() ) ){
                seq.printf(",0");
            }
            else {
                seq.printf(",%" DEC,(idx)(tSC[pos].coverage()*(t_region->freq)));
            }
        }
        pos += cl->start;
    }
    for( ; pos < stats->size ; ++pos ) {
        seq.addString(",0");
    }
    printTitleFreq(title, param, cur_clId);
    if(param->fastaTmplt){
        sString::searchAndReplaceStrings(&out,param->fastaTmplt,0,"$_(v)" __,title.ptr(1),1,false);
        out.shrink00();
    }
    else{
        out.addString(title.ptr(1));
    }
    out.printf(",%s\n", seq.ptr(1));
    return offset;
}


idx sViopop::printContigComposition( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx cur_clId, idx offset)
{
    if( composition.dim() <= 0 ) return 0;
    sViopop::cloneRegion * t_region;
    sVec<idx> gap2skpMap, * pG2S = 0;
    sVec< seqCovPosition > seqCov ;
    if( param->flags&sViopop::clPrintNoGapsFrame ) {
        pG2S = &gap2skpMap;
    }
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    sVec<contigComp> comp;
    idx start = 0 , end = 0;
    sStr seq;

    sStr title;
    printTitle(title,param,cur_clId);
    for( idx i = 0 ; i < composition.dim() ; ++i )
    {
        t_region = composition.ptr(i);
        cl = getCl( t_region->contigInd );
        if(!cl) continue;

        if( mergeClone(cl,rsCl,comp,param, &seqCov, pG2S) ) {
            cl = &rsCl;
        }
        else if (!isValidClone(cl->clID)) {
            return -1;
        }

        start =  getContigPos(cl,t_region->start, pG2S,true);
        end = getContigPos(cl, t_region->end-1, pG2S,true)+1;
        if(start==-1) {
            return -1;
        }

        printTitle(title,param, t_region->contigInd, offset + 1, end - start + offset);
        if( !(param->flags&clPrintNoGapsFrame) && (param->flags&clPrintContigsInMutualFrame) ) {
            seq.printf("%" DEC " %" DEC " clone_%" DEC ":%" DEC "-%" DEC "\n",offset+1,end-start+offset, t_region->contigInd,cl->start + start+1,cl->start + end);
        }
        else {
            seq.printf("%" DEC " %" DEC " clone_%" DEC ":%" DEC "-%" DEC "\n",offset+1,end-start+offset, t_region->contigInd,start+1,end);
        }
        offset+=end-start;
    }
    printTitleFreq(title, param, cur_clId);
    if(param->fastaTmplt){
        sString::searchAndReplaceStrings(&out,param->fastaTmplt,0,"$_(v)" __,title.ptr(1),1,false);
        out.shrink00();
    }
    else{
        out.addString(">");
        out.addString(title.ptr(1));
    }
    out.printf("\n%s\n", seq.ptr());
    return offset;
}



idx sViopop::printContigBreakpoints( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx cur_clId, idx offset)
{
    if( composition.dim() <= 0 ) return 0;
    sBioseqpopul::cloneStats * gStats = getGenStats ();
    idx refCnt=gStats->support;
    sVec<idx> skp2gapMap,gap2skpMap, * pG2S = 0;
    sBioseqpopul::cloneSummary * cl=0, rsCl;
    sVec<contigComp> comp, * pComp = 0;
    sVec< seqCovPosition > seqCov ;
    if( param->flags&sViopop::clPrintNoGapsFrame ) {
        pG2S = &gap2skpMap;
    }
    real simil_thrshld = param->similarity_threshold;
    if( simil_thrshld < 0.1 ) {
        simil_thrshld = 0.1;
    }
    sVec< real > similsV(sMex::fExactSize);
    similsV.add(refCnt);
    sStr simils, t_simils;

    sViopop::cloneRegion * t_region;
    idx start, end, cnt_valid_simil = 0;
    sStr seq;
    bool first = true;

    sStr title,t_title;
    printTitle(title,param,cur_clId);
    seq.addString("1");
    t_simils.cut(0);
    for( idx i = 0 ; i < composition.dim() ; ++i )
    {
        t_region = composition.ptr(i);
        cl = getCl( t_region->contigInd );
        if(!cl) continue;

        if( mergeClone(cl,rsCl,comp,param,&seqCov, pG2S) ) {
            cl = &rsCl;
            pComp = &comp;
        }
        else if (!isValidClone(cl->clID)) {
            return -1;
        }

        start = getContigPos(cl, t_region->start, pG2S, true);
        end = getContigPos(cl, t_region->end-1, pG2S, true)+1;
        if(start==-1) {
            return -1;
        }

        printTitle(title,param, t_region->contigInd, offset + 1, end - start + offset);


        for( idx pos = t_region->start ; pos < t_region->end ; ++pos, ++offset )
        {
            if( getContigPos(cl,pos, pG2S) < 0 ) {
                --offset;
                continue;
            }
            if ( isMultiGap(cl,pos, pG2S) ) {
                continue;
            }
            getPositionSimil(cl->clID, cl->start + pos, refCnt, similsV.ptr(), true, pComp );

            cnt_valid_simil = getSimilAboveThrshld(similsV, simils, simil_thrshld, (sBioseq *)param->userPointer);
            if(first) {
                t_simils.printf(0,"%s",simils.ptr());
                first = false;
            }
            if( !cnt_valid_simil && !(param->flags&sViopop::clPrintLowDiversityBreaks) ) {
                continue;
            }
            if( isBreakpoint(t_simils,simils) ) {
                seq.printf(" %" DEC " %s\n%" DEC,offset, t_simils.ptr() ,offset+1);
                t_simils.printf(0,"%s",simils.ptr());
            }
        }
    }
    seq.printf(" %" DEC " %s\n",offset,simils.ptr());
    t_title.cut(0);
    printTitleFreq(title, param, cur_clId);
    if(param->fastaTmplt){
        sString::searchAndReplaceStrings(&t_title,param->fastaTmplt,0,"$_(v)" __,title.ptr(1),1,false);
        out.shrink00();
    }
    else{
        t_title.addString(title.ptr(1));
    }
    out.printf("%s\n%s",t_title.ptr(), seq.ptr());
    return offset;
}

idx sViopop::printContigSummary(sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * params, idx offset ){
    if( composition.dim() !=1 ) {
        params->out->printf("Summary is reported only for Contigs\n");
        return 0;
    }

    sBioseqpopul::cloneSummary * cl, rsCl, * pcl = 0, rsPCl, * mcl = 0;
    sVec< seqCovPosition > seqCov, * pSC = 0, pSeqCov, *pPSC = 0;
    sVec < contigComp > cComp, pComp;
    sViopop::cloneRegion * t_region = composition.ptr(0);

    cl = getCl( t_region->contigInd );
    if(!cl) return 0;
    if( mergeDict){
        if( isValidClone(cl->clID) ){
            if (mergeClone(cl, rsCl,cComp, params,&seqCov ) ) {
                pSC = &seqCov;
                cl = &rsCl;
            }
        } else {
            params->out->printf("Invalid contigs %" DEC "\n", cl->clID);
            return 0;
        }
    }
    params->out->printf("Clone_%" DEC ",%" DEC ",%" DEC ",",cl->clID,cl->start + 1,cl->end);
    if( cl->hasMerged() ) {
        mcl = getValidMergingSummary(cl);
        if( mcl ) {
            params->out->printf("Clone_%" DEC,mcl->clID);
        }
    }
    params->out->addString(",");
    if( cl->hasParent() ) {
        pcl = getValidParentSummary(cl);
        if ( mergeClone(pcl, rsPCl,pComp, params,&pSeqCov ) ) {
            pPSC = &pSeqCov;
            pcl = &rsPCl;
        }
        if(pcl) {
            params->out->printf("Clone_%" DEC ",",pcl->clID);
            clonesDifferences(params, pcl, cl, pPSC, pSC);
        } else {
            params->out->addString(",,,,");
        }
    } else {
        params->out->addString(",,,,");
    }
    params->out->addString("\n");
    return 1;
}

idx sViopop::printAllSequenceClones(idx * iVis, idx start, idx cnt,sStr & out,idx * arrSortClon){
    idx * clList=0;
    idx clCnt=0, iFound=0,base=0;

    clCnt=dimCl();
    if(arrSortClon){
        clList=arrSortClon;
        setMode(1);
    }
    else{
        clList=getArrSort();
    }

    sBioseqpopul::cloneStats * stats=getGenStats();

    out.printf("Position");
    for (idx icl=0; icl<clCnt; ++icl) {
        idx iCl=clList ? clList[icl] : icl;
        out.printf(",Clone_%" DEC,getCl(iCl)->clID);
    }
    out.printf("\n");
    for(idx is=0;is<stats->size;++is){
        out.printf("%" DEC,is+1);
        for (idx icl=0; icl<clCnt; ++icl) {
            idx iCl=clList ? clList[icl] : icl;
            sVec<idx> scSeq(sMex::fExactSize);
            getSeq(iCl,scSeq);
            sBioseqpopul::cloneSummary* cl=getCl(iCl);
            if(!cl || cl->clID<0)continue;
            if(cl->start>is || cl->end<=is)
                out.printf(",");
            else{
                base=scSeq[is-cl->start];
                if(base>=0 && base<=3)
                    out.printf(",%c",sBioseq::mapRevATGC[base]);
                else if(base==sBioseqpopul::baseN)
                    out.printf(",N");
                else
                    out.printf(",-");

            }
        }
        out.printf("\n");
        ++iFound;
    }
    return iFound;
}


idx sViopop::printCoverageSingle(sViopop * viopop, ParamCloneIterator * params, idx clIndex){
    sBioseqpopul::cloneSummary * clSum=0;
    sVec<idx> scCov(sMex::fExactSize), scSeq(sMex::fExactSize);
    idx step=params->step;

    if(params->flags&sViopop::clPrintSummary)
        clSum=viopop->getCl(clIndex);
    if(params->flags&sViopop::clPrintCoverage)
        viopop->getCov(clIndex,scCov);
    if(params->flags&sViopop::clPrintConsensus)
        viopop->getSeq(clIndex,scSeq);

    if(clSum){
        params->out->printf("Clone_%" DEC ",%" DEC ",%" DEC ",%" DEC ",",clSum->clID,clSum->start,clSum->end,clSum->mergeclID);
        sBioseqpopul::cloneSummary * fCl=viopop->getFatherCl(clIndex);

        if(fCl){
            params->out->printf("Clone_%" DEC,fCl->clID);
        }
        else
            params->out->printf("-1");
    }

    if(scCov.dim()){
        idx avFilCov=0,maxFilCov=0,iC=0;
        params->out->printf(",\"");
        for(idx i=0;i<scSeq.dim();++i){
            if(!(i%step) || !i || i==scSeq.dim()-1){
                if(i)params->out->printf(",");
                params->out->printf("%" DEC,scCov[i]);
                avFilCov+=scCov[i];++iC;
                if(maxFilCov<scCov[i])
                    maxFilCov=scCov[i];
            }
        }
        params->out->printf("\"");
        params->out->printf(",%" DEC ",%.2lf",maxFilCov,(real)avFilCov/iC);
    }
    params->out->printf("\n");
    return 1;
}

idx sViopop::printStackedStats(sStr * out, ParamCloneIterator * params){
     sBioseqpopul::cloneStats * stat_cl;
     sBioseqpopul::cloneSummary * summ_cl;
     sVec<idx> stck_lengths;
     sVec<idx> stck_lengths_ind;
     stck_lengths.add(dimCl());
     stck_lengths_ind.add(dimCl());

     for( idx i = 0 ; i < dimCl() ; ++i ){
         stat_cl = getStats(i);
         stck_lengths[i] = stat_cl->stacked_size;
         stck_lengths_ind[i]=i;
     }
     sSort::sort(stck_lengths.dim(),stck_lengths.ptr(),stck_lengths_ind.ptr());
     idx srt_i=0;
     out->printf("clone,stacked_size,region_size\n");
     for( idx i = stck_lengths.dim() - 1 ; i >=0  ; --i ){
         srt_i = stck_lengths_ind[i];
         stat_cl = getStats(srt_i);
         summ_cl = getCl(srt_i);
         out->printf("Clone_%" DEC ",%" DEC ",%" DEC "\n",summ_cl->clID,stat_cl->stacked_size,stat_cl->size);
     }

     return stck_lengths.dim();
}

idx sViopop::getContigDistance(sBioseqpopul::cloneSummary * cl, idx end, idx start, sVec<idx> * contigMap, bool forceValid) {
    if( ( !gap2skipMap || !gap2skipMap->dim() ) && ( !contigMap || !contigMap->dim() ) ) {
        if ( end < cl->end-cl->start && start >= 0)
            return end - start;
        else {
            return -1;
        }
    }
    else {
        idx rend = getGap2SkipContigPos(cl,end,contigMap,forceValid);
        if( rend < 0 ) {
            return rend;
        }
        return rend - getGap2SkipContigPos(cl,start,contigMap,forceValid);
    }
}


idx sViopop::getGap2SkipFramePos(sBioseqpopul::cloneSummary * cl, idx pos, sVec<idx> * contigMap, bool forceValid ) {
    idx clPos = 0;

    clPos = getContigPos(cl, pos - cl->start, contigMap, forceValid);
    sBioseqpopul::cloneSummary * parentCl = cl->hasParent()?getValidParentSummary(cl):0;
    if( parentCl ) {
        sVec< contigComp > comp;
        sBioseqpopul::cloneSummary resCl;
        sVec<seqCovPosition> sc;
        sVec<idx> t_contigMap;
        if ( mergeClone(parentCl,resCl,comp,0,&sc,&t_contigMap) ){
            clPos += sViopop::getGap2SkipFramePos(&resCl, cl->start, &t_contigMap, true);
        } else {
            clPos += sViopop::getGap2SkipFramePos(parentCl, cl->start, 0, true);
        }
    }
    else if( cl->hasMerged() ) {
        sBioseqpopul::cloneSummary * m_cl = getValidMergingSummary(cl);
        if( m_cl && (!m_cl->hasParent() || !isCloneAncestor(cl,m_cl) ) ) {
            idx mergePos, contigLen = getContigPos(cl, cl->end - cl->start - 1, contigMap, forceValid);

            sVec< contigComp > comp;
            sBioseqpopul::cloneSummary resCl;
            sVec<seqCovPosition> sc;
            sVec<idx> t_contigMap;
            if ( mergeClone(m_cl,resCl,comp,0,&sc,&t_contigMap) ){
                mergePos = getFramePos(m_cl, cl->end -1,&t_contigMap,true)+1;
            } else {
                mergePos = getFramePos(m_cl, cl->end -1,0,true)+1;
            }
            if( mergePos - contigLen > 0) {
                clPos += mergePos - contigLen;
            }
        }
    } else {
        clPos = clPos+1;
        clPos -= 1;
    }
    return clPos;
}

idx sViopop::getMergedFramePos(sBioseqpopul::cloneSummary * cl , idx pos, ParamCloneIterator * params, bool forceValid ) {
    sBioseqpopul::cloneSummary mergedCl;
    sVec< seqCovPosition > seqCov;
    sVec < contigComp > cComp;
    sVec<idx> skp2gapMap, gap2skpMap, * pG2Smap = 0;
    if( params->flags&sViopop::clPrintNoGapsFrame ) {
        pG2Smap = &gap2skpMap;
    }

    if( mergeDict){
        if( isValidClone(cl->clID) ){
            if (mergeClone(cl, mergedCl,cComp, params,&seqCov, pG2Smap ) ) {
                cl = &mergedCl;
            }
        }
        else
            return -1;
    }
    return getFramePos(cl, pos, pG2Smap, forceValid);
}

idx sViopop::getMergedContigPos(sBioseqpopul::cloneSummary * cl , idx pos, ParamCloneIterator * params, bool forceValid ) {
    sBioseqpopul::cloneSummary mergedCl;
    sVec< seqCovPosition > seqCov;
    sVec < contigComp > cComp;
    sVec<idx> skp2gapMap, gap2skpMap, * pG2Smap = 0;
    if( params->flags&sViopop::clPrintNoGapsFrame ) {
        pG2Smap = &gap2skpMap;
    }

    if( mergeDict){
        if( isValidClone(cl->clID) ){
            if (mergeClone(cl, mergedCl,cComp, params,&seqCov, pG2Smap ) ) {
                cl = &mergedCl;
            }
        }
        else
            return -1;
    }
    return getContigPos(cl, pos, pG2Smap, forceValid);
}
