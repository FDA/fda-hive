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
#include "bioclones.hpp"

#include <slib/utils.hpp>
#include <ssci/math/func/func.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>


const char * sPopul::difAl2(idx icl1, idx icl2, idx iAl, sStr * out, bool onlydiffs) {
    if( !out ) {
        out = &_strBuf;
        out->cut(0);
    }

    sBioseqAlignment::Al * hdr = _bioal->getAl(iAl);
    idx * m=_bioal->getMatch(iAl),idQ=hdr->idQry();

    sVec < idx > uncompressMM;

    if(hdr->flags()&sBioseqAlignment::fAlignCompressed){
        uncompressMM.resize(hdr->lenAlign()*2);
        sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
        m=uncompressMM.ptr();
    }

    hdrStruct qrAlStr;
    qrAlStr.hdr = hdr;
    qrAlStr.m = m;
    qrAlStr.qrybuflen = _bioal->Qry->len(idQ);
    qrAlStr.nonCompFlags = ((hdr->flags())&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement)));
    qrAlStr.subEnd = hdr->subStart()+m[2*(hdr->lenAlign()-1)];;
    qrAlStr.subStart = hdr->subStart()+m[0];
    qrAlStr.vqry = _bioal->Qry->seq(idQ);
    qrAlStr.iAl = iAl;

    idx lenAl=2*hdr->lenAlign(),is=0,alBase=-2;
    sClone * cl1 = ptr(icl1);
    sClone * cl2 = ptr(icl2);
    sBioseqpopul::position * p1 = 0,* p2 = 0;

    out->printf("|  #  |   Al  | cl %2" DEC " |   Cov |  dif  | cl %2" DEC " |   Cov |  dif  |\n",icl1,icl2);
    out->printf("|-----+-------+-------+-------+-------+-------+-------+-------|\n");

    for(idx i=0;i<lenAl;i+=2){
        bool match1 = false, match2 = false;
        alBase = sPopul::basecallOnMatchIndex(i,&qrAlStr,_quality_threshold,&is);
        bool fuzzy1 = isClonalPositionFuzzy(cl1, is, fuzzy_threshold), fuzzy2 = isClonalPositionFuzzy(cl2, is, fuzzy_threshold);
        bool fuzzyfr = isFramePositionFuzzy(is,fuzzy_threshold);
        p1 = cl1->pos(is);
        p2 = cl2->pos(is);
        if(!p1->isDiff(alBase, 1))
            match1 = true;
        if(!p2->isDiff(alBase, 1))
            match2 = true;
        if( fuzzyfr || fuzzy1 || fuzzy2 ){
            if (fuzzyfr) {
                fuzzy1 = fuzzy2 = true;
            }
            if(onlydiffs && fuzzy1 && fuzzy2)continue;
            if ( fuzzy1 && fuzzy2 )
                out->printf("|%5" DEC "|---%c---|---%c---|-%5" DEC "-|---%c---|---%c---|-%5" DEC "-|---%c---|\n", is, (alBase<0?'l':(char)sBioseq::mapRevATGC[alBase]), p1->basechar(), p1->getCoverage(false), match1 ?'-':'*', p2->basechar(), p2->getCoverage(false), match2 ?'-':'*' );
            else if ( fuzzy1 )
                out->printf("|%5" DEC "|   %c   |---%c---|-%5" DEC "-|---%c---|   %c   | %5" DEC " |   %c   |\n", is, (alBase<0?'l':(char)sBioseq::mapRevATGC[alBase]), p1->basechar(), p1->getCoverage(false), match1 ?'-':'*', p2->basechar(), p2->getCoverage(false), match2 ?' ':'*' );
            else if ( fuzzy2 )
                out->printf("|%5" DEC "|   %c   |   %c   | %5" DEC " |   %c   |---%c---|-%5" DEC "-|---%c---|\n", is, (alBase<0?'l':(char)sBioseq::mapRevATGC[alBase]), p1->basechar(), p1->getCoverage(false), match1 ?' ':'*', p2->basechar(), p2->getCoverage(false), match2 ?'-':'*' );
            continue;
        }

        if(onlydiffs && (match1 && match2))continue;
        out->printf("|%5" DEC "|   %c   |   %c   | %5" DEC " |   %c   |   %c   | %5" DEC " |   %c   |\n", is, (alBase<0?'l':(char)sBioseq::mapRevATGC[alBase]), p1->basechar(), p1->getCoverage(false), match1 ?' ':'*', p2->basechar(), p2->getCoverage(false), match2 ?' ':'*' );
    }
    out->printf("|-------------------------------------------------------------|\n");
    return out->ptr(0);
}

const char * sPopul::difAl(idx icl, idx iAl, sStr * out, bool onlydiffs) {

    if( !out ) {
        out = &_strBuf;
        out->cut(0);
    }

    sBioseqAlignment::Al * hdr = _bioal->getAl(iAl);
    idx * m=_bioal->getMatch(iAl),idQ=hdr->idQry();

    sVec < idx > uncompressMM;

    if(hdr->flags()&sBioseqAlignment::fAlignCompressed){
        uncompressMM.resize(hdr->lenAlign()*2);
        sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
        m=uncompressMM.ptr();
    }

    hdrStruct qrAlStr;
    qrAlStr.hdr = hdr;
    qrAlStr.m = m;
    qrAlStr.qrybuflen = _bioal->Qry->len(idQ);
    qrAlStr.nonCompFlags = ((hdr->flags())&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement)));
    qrAlStr.subEnd = hdr->subStart()+m[2*(hdr->lenAlign()-1)];;
    qrAlStr.subStart = hdr->subStart()+m[0];
    qrAlStr.vqry = _bioal->Qry->seq(idQ);
    qrAlStr.iAl = iAl;

    idx lenAl=2*hdr->lenAlign(),is=0,alBase=-2;
    sClone * cl = ptr(icl);
    sBioseqpopul::position * p = 0;

    out->printf("|Clone__%7" DEC "____________________|\n", icl);
    out->printf("|  #  | clone |   Cov |   Al  |  dif  |\n");
    out->printf("|-----+-------+-------+-------+-------|\n");

    for(idx i=0;i<lenAl;i+=2){
        bool match = false;
        alBase = sPopul::basecallOnMatchIndex(i,&qrAlStr,_quality_threshold,&is);
        p = cl->pos(is);
        if(isFramePositionFuzzy(is,fuzzy_threshold) || isClonalPositionFuzzy(cl, is, fuzzy_threshold) ){
            if(onlydiffs)continue;
            out->printf("|%5" DEC "|---%c---|-%5" DEC "-|---%c---|-------|\n", is, p->basechar(), p->getCoverage(false), ((alBase<0)?'l':(char)sBioseq::mapRevATGC[alBase]));
            continue;
        }
        if(!p->isDiff(alBase,1)){
            match = true;
        }
        if(onlydiffs && match)continue;
        out->printf("|%5" DEC "|   %c   | %5" DEC " |   %c   |%7s|\n", is, p->basechar(), p->getCoverage(false), (alBase<0?'l':(char)sBioseq::mapRevATGC[alBase]), match ?"":"*" );
    }
    out->printf("|-------------------------------------|\n");
    return out->ptr(0);
}
idx sPopul::getClonalPositionDifferences(sClone *cl, idx is, idx * basecalls, idx basecnt, real * compbuf)
{
    if( (basecnt <= 0)) {
        return -1;
    }
    ++compbuf[0];
    if(basecnt==1 && basecalls[0]==sBioseqpopul::baseN)return 0;
    real ambig_weight = 0;

    if( alcl_dist_m == eEuclidean ) {
        ambig_weight = (real)cl->pos(is)->getBaseConfidence((sBioseqpopul::baseCalls)basecalls[0]);
    }
    if(basecnt==1) {
        ambig_weight = 1 - ambig_weight;
        if( alcl_dist_m == eEuclidean ) {
            compbuf[1] += pow(ambig_weight,2);
        } else {
            if(cl->pos(is)->isDiff(basecalls[0],1)) {
                compbuf[1] += 1;
            }
        }
        return 1;
    }

    idx ins_cov = isInsertionInPos(cl->clID,is,basecalls+1,basecnt-1);
    real ins_ambig_weight = 0;

    if( alcl_dist_m == eEuclidean ) {
        ins_ambig_weight = (real)ins_cov/(cl->pos(is)->getCoverage(false));
        ambig_weight = sMin(ambig_weight,ins_ambig_weight);
        ambig_weight = 1 - ambig_weight;
        compbuf[1] += pow(ambig_weight,2);
    } else {
        if( (real)ins_cov/ sMax((real)*_coverage.pos(is-cl->curSubPOS), cl->avCoverage()) < fuzzy_threshold ) {
            compbuf[1] += 1;
        }
    }
    return 1;
}

idx sPopul::closestClone(hdrStruct &hdrS, idx prv_cl)
{
    sBioseqAlignment::Al * hdr=hdrS.hdr;

    static sVec<real> candCl(sMex::fSetZero|sMex::fExactSize);
    static sVec<real> curr_candCl(sMex::fSetZero|sMex::fExactSize);
    static sVec<idx> last_nonFuzzy(sMex::fSetZero|sMex::fExactSize);
    candCl.resize( 2*cnt() );
    curr_candCl.resize( 2*cnt() );
    last_nonFuzzy.resize(cnt());
    candCl.set(0);
    curr_candCl.set(0);
    last_nonFuzzy.set(0);

    for(idx i = 0 ; i < cnt() ; ++i) {
        if(ptr(i)->valid()) {
            last_nonFuzzy[i]=getLastNonFuzzyPosition(ptr(i),fuzzy_threshold);
        }
    }
    idx lenAl=2*hdr->lenAlign(),is=0;
    sClone * cl = 0, * m_cl = getMainClone();
    static sVec<idx> alBases;
    bool is_cov_by_all = true;
    idx is_cov = 0;
    for(idx i=0;i<lenAl;i+=2){
        curr_candCl.set(0);
        alBases.cut(0);
        basecallInsertsOnMatchIndex(i,&hdrS,_quality_threshold,alBases,&is);
        is_cov_by_all = true;
        if(is<m_cl->curSubPOS) continue;
        if(isFramePositionFuzzy(is,fuzzy_threshold))continue;
        for(idx k=0;k<cnt();++k){
            cl = ptr(k);
            if( !cl->valid() ) {
                continue;
            }
            if(last_nonFuzzy[k] < is) {
                is_cov = -1;
            } else {
                is_cov = getClonalPositionDifferences(cl, is, alBases.ptr(), alBases.dim(), &curr_candCl[2*k]);
            }
            if( is_cov < 0 ) {
                is_cov_by_all = false;
            }
        }
        if(is_cov_by_all || true) {
            for(idx l = 0 ; l < curr_candCl.dim() ; ++l)
                candCl[l]+=curr_candCl[l];
        }
    }
    real amb_w_min=REAL_MAX, amb_w_score = 0;
    idx amb_w_fCl = -1;
    bool amb_call = false;
    for(idx k=0;k<cnt();++k)
    {
        if(!candCl[2*k])continue;
        amb_w_score = (real)(candCl[2*k+1])/candCl[2*k];
        if(amb_w_score <= amb_w_min ) {
            if( amb_w_score == amb_w_min ) {
                amb_call = true;
                if( k!=prv_cl ) continue;
            }
            else
                amb_call = false;
            amb_w_fCl=k;
            amb_w_min=amb_w_score;
        }
    }

    if(refill_mode == 0 && amb_call) return -10;
    if(amb_w_fCl < 0) {
        amb_w_fCl = getMainCloneIdx();
    }
    return amb_w_fCl;
}

idx sPopul::distOfBifByFing(hdrStruct &hdrS, sBioseqpopul::clonePat * chilP)
{
    sBioseqAlignment::Al * hdr=hdrS.hdr;
    if(!chilP->pattern.dim())return _bfrcS._fatherclID;
    sVec<idx> candCl;
    candCl.add(2);
    candCl.set(0);

    idx lenAl=2*hdr->lenAlign(),is=0,j=0;
    sVec<idx> alBases;
    for(idx i=0;i<lenAl ;i+=2)
    {
        alBases.cut(0);
        sPopul::basecallInsertsOnMatchIndex(i, &hdrS,_quality_threshold, alBases, &is);

        if( is>chilP->last() )
            break;

        idx compPos=chilP->pattern.ptr(j)->position;
        while(is>compPos && j<chilP->pattern.dim()){
          compPos=chilP->pattern.ptr(++j)->position;
        }
        if(j>=chilP->pattern.dim())break;
        if( is == compPos ) {
            if( !sBioseqpopul::isDiffBases( chilP->pattern[j].bases.ptr(), chilP->pattern[j].bases.dim(), alBases.ptr(), alBases.dim()) )
                ++candCl[1];
            else
                ++candCl[0];
        }
    }
    return (candCl[0]>=candCl[1])?_bfrcS._fatherclID:_bfrcS._childclID;
}

idx sPopul::alToClone(hdrStruct &hdrS,bifurcation_structure &frmS )
{
    sBioseqAlignment::Al * hdr=hdrS.hdr;
    idx positofBifurc;
    idx clID=_alClonIds[hdrS.iAl];

    sClone * w=ptr(clID);
    if( w->valid() && clID==frmS._fatherclID ){
        sClone *w1=ptr(frmS._childclID);
        positofBifurc=w1->bifurcation_pos;

        if(hdr->subStart()+hdrS.m[0]>=0)
        {
            sVec<idx> alBases;
            if(positofBifurc< hdrS.subStart){
                return clID;
            }
            if(positofBifurc>hdrS.subEnd){
                return frmS._fatherclID;
            }

            sPopul::basecallInsertsOnSubjectIndex(positofBifurc,&hdrS, _quality_threshold,alBases);

            if( !sBioseqpopul::isDiffBases(frmS.bases.ptr(),frmS.bases.dim(),alBases.ptr(),alBases.dim()) )
                clID=frmS._childclID;
        }
    }

    return clID;
}
idx sPopul::basecallInsertsOnMatchIndex(idx &i, hdrStruct * alHdr, idx quality_threshold, sVec<idx> & basecalls, idx * k_isx){
    idx * m = alHdr->m;
    while( i<2*alHdr->hdr->lenAlign() && m[i]<0 ) {
        basecalls.vadd(1,basecallOnQryIndex(m[i+1], alHdr, quality_threshold));
        i+=2;
    }
    basecalls.vadd(1,basecallOnQryIndex(m[i+1], alHdr, quality_threshold));
    if(k_isx)*k_isx = m[i] + alHdr->hdr->subStart();
    i+=2;
    while( i<2*alHdr->hdr->lenAlign() && m[i]<0 )
    {
        basecalls.vadd(1,basecallOnQryIndex(m[i+1], alHdr, quality_threshold));
        i+=2;
    }
    i-=2;
    return basecalls.dim();
}
idx sPopul::basecallOnMatchIndex(idx &i, hdrStruct * alHdr, idx quality_threshold, idx * k_isx){
    idx * m = alHdr->m;
    while( i<2*alHdr->hdr->lenAlign() && m[i]<0 )
        i+=2;
    if( i >= 2 * alHdr->hdr->lenAlign() )
        return sBioseqpopul::baseNone;
    if(k_isx)*k_isx = m[i] + alHdr->hdr->subStart();

    return basecallOnQryIndex(m[i+1], alHdr, quality_threshold);
}
idx sPopul::basecallOnQryIndex(idx iq, hdrStruct * alHdr, idx quality_threshold){
    if(iq<0){
        return sBioseqpopul::baseDel;
    }
    if( quality_threshold && alHdr->quastring && sBioseq::Qua(alHdr->quastring,iq,false) < quality_threshold ) {
        return sBioseqpopul::baseN;
    }

    idx iqx = ( alHdr->hdr->flags()&sBioseqAlignment::fAlignBackward ) ? (alHdr->qrybuflen-1-(alHdr->hdr->qryStart()+iq)): (alHdr->hdr->qryStart()+iq);
    return (alHdr->hdr->flags()&sBioseqAlignment::fAlignBackward ) ?sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(alHdr->vqry, iqx, alHdr->nonCompFlags)] :
                    (idx)sBioseqAlignment::_seqBits(alHdr->vqry, iqx, alHdr->nonCompFlags);
}
idx sPopul::basecallOnSubjectIndex(idx subInd, hdrStruct * alHdr, idx quality_threshold){
    idx * m = alHdr->m;
    for(idx i=0; i<2*alHdr->hdr->lenAlign();i+=2){
        while( (i+2)<2*alHdr->hdr->lenAlign() && m[i]<0 )
            i+=2;
        if( i >= 2 * alHdr->hdr->lenAlign() )
            return sBioseqpopul::baseNone;
        if( alHdr->hdr->subStart() + m[i] ==subInd)
            return basecallOnQryIndex(m[i+1],alHdr,quality_threshold);
    }
    return sBioseqpopul::baseNone;
}
idx sPopul::basecallInsertsOnSubjectIndex(idx subInd, hdrStruct * alHdr, idx quality_threshold, sVec<idx> & basecalls) {
    idx * m = alHdr->m;
    for(idx i=0; i<2*alHdr->hdr->lenAlign();i+=2){
        while( (i+2)<2*alHdr->hdr->lenAlign() && m[i]<0 )
            i+=2;
        if( i >= 2 * alHdr->hdr->lenAlign() )
            return sBioseqpopul::baseNone;
        if( (m[i] + alHdr->hdr->subStart()) == subInd )
            return basecallInsertsOnMatchIndex(i, alHdr, quality_threshold, basecalls);
        else if( (m[i] + alHdr->hdr->subStart()) > subInd )
            return sBioseqpopul::baseNone;
    }
    return sBioseqpopul::baseNone;
}

sClone * sPopul::findCloneDestination( hdrStruct * alHdr, sClone * last )
{

    if(refill_mode!=2 && refill_mode!=0) {
        if(cntAlive()==1 && (cloneContinuity || !_lastSupportedPosition) ) {
            return getMainClone();
        } else if (!cloneContinuity && _lastSupportedPosition && alHdr->subStart > _lastSupportedPosition ) {
            killall();
            ensureAliveClone(_POS);
            return getMainClone();
        }
    }

    idx iAl=alHdr->iAl, p_iAl = -1;
    idx l_sendToCln=getAlCloneIdx(iAl, &p_iAl);
    idx sendToCln = _alClonIds[iAl], mergedPos = -1;
    if(last && sendToCln>=0)last=ptr(sendToCln);
    sBioseqpopul::clonePat singleFingP;

    if( refill_mode == 1 ) {
        if( sendToCln != _bfrcS._fatherclID ) {
            return 0;
        }
        if( alHdr->subEnd <= _bfrcS._POS )
            return getValidClone(sendToCln,&_bfrcS._POS,&mergedPos);


        if( alHdr->subStart <= _bfrcS._POS ) {
            sendToCln = alToClone(*alHdr, _bfrcS);
        } else {
            if( getSingleFingPrint(&singleFingP, _bfrcS._fatherclID, _bfrcS._childclID, fuzzy_threshold, _bfrcS._POS) ) {
                sendToCln = distOfBifByFing(*alHdr, &singleFingP);
            } else {
                mergeClones(ptr(_bfrcS._fatherclID),ptr(_bfrcS._childclID),ptr(_bfrcS._childclID)->matPos(ptr(_bfrcS._childclID)->getrawfirst()+1));
                ptr(_bfrcS._childclID)->markAsDead();
                sendToCln = _bfrcS._fatherclID;
            }
        }
    } else if( refill_mode == -1 ) {
        if( (sendToCln == _bfrcS._childclID && alHdr->subStart <= _bfrcS._POS && alHdr->subEnd >=_bfrcS._POS) ) {
            sendToCln = closestClone(*alHdr, l_sendToCln);
        } else
            return 0;
        if( sendToCln == _bfrcS._childclID )
            return 0;
    } else if (refill_mode == 2) {
        if( sendToCln == -10 ) {
            sendToCln = closestClone(*alHdr, l_sendToCln);
        } else return 0;
    } else {
        if( sendToCln < 0 ) {
            sendToCln = closestClone(*alHdr, l_sendToCln);
            if(sendToCln < 0 ) {
                _alClonIds[iAl] = -10;
                return 0;
            }
        } else return 0;
    }

     if( sendToCln < 0 ) {
         sendToCln = getMainCloneIdx();
     }
     return getValidClone(sendToCln,&_bfrcS._POS,&mergedPos);
}

idx sPopul::fill( idx refill,idx * relativeTo )
{
    refill_mode = refill;
    idx numofAlsCl=0;
    if( (refill_mode ==1 || refill_mode == -1)&& !ptr(_bfrcS._childclID)->valid()) {
        return 0;
    }
    hdrStruct qrAlStr ;
    sVec<idx> ins_vec;

    idx basecall=0,t_basecall=0;
    idx isx=0,lastvalid_isx=0,lenAlign=0,rpts=0, basecall_start;
    bool al2Cl=false;

#ifdef _DEBUG
    sVec<idx> added_als(sMex::fSetZero|sMex::fExactSize);
    added_als.add(cnt());
#endif

    sVec < idx > uncompressMM;
    for(idx iAl = getAlStart() ;iAl<_iAlEnd;++iAl)
    {
        idx iAlList=getAlindex(iAl);
        sBioseqAlignment::Al * hdr= _bioal->getAl(iAlList);
        idx * m=_bioal->getMatch(iAlList),idQ=hdr->idQry();


        if(hdr->flags()&sBioseqAlignment::fAlignCompressed){
            uncompressMM.resize((1+hdr->lenAlign())*2);
            sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr(),false,hdr->lenAlign()*2);
            m=uncompressMM.ptr();
        }

        idx extFrame=(m[2*(hdr->lenAlign()-1)] - m[0]) -win_size();
        if(extFrame>0){
            resize(extFrame);
            getCloneBoundaries(_POS,win_size());

        }

        qrAlStr.hdr = hdr;
        qrAlStr.m = m;
        qrAlStr.qrybuflen = _bioal->Qry->len(idQ);
        qrAlStr.nonCompFlags = ((hdr->flags())&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement)));
        qrAlStr.subEnd = hdr->subStart()+m[2*(hdr->lenAlign()-1)];;
        qrAlStr.subStart = hdr->subStart()+m[0];
        qrAlStr.vqry = _bioal->Qry->seq(idQ);
        qrAlStr.iAl = iAlList;

        bool isQuaBit = _bioal->Qry->getQuaBit(idQ);
        qrAlStr.quastring = !isQuaBit? _bioal->Qry->qua(idQ) : 0;

        if(qrAlStr.subEnd < _POS)
            continue;

        sClone * wp = 0 ,* w = findCloneDestination( &qrAlStr, 0 );

        if(!w || w->killed) {
            continue;
        }

        basecall_start = w->curSubPOS;
        if (refill_mode == 1 || refill_mode == -1) {

            wp = (_alClonIds[qrAlStr.iAl]>=0)?ptr(_alClonIds[qrAlStr.iAl]):0;
            if( wp ) {
                wp = getValidClone(wp->clID, &_bfrcS._POS, &basecall_start);
                if(basecall_start<0)basecall_start = w->curSubPOS;
            }
            if( !wp || w->clID == wp->clID ) {
                continue;
            }
        }


        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }

        isx=qrAlStr.subStart-1; lenAlign=hdr->lenAlign();rpts=_bioal->getRpt(qrAlStr.iAl);

        al2Cl=false;


#ifdef _DEBUG
        added_als[w->clID]+=rpts;
        if(refill_mode == 1 || refill_mode == -1)
            added_als[wp->clID]-=rpts;
#endif

        for( idx i=0, lastvalid_i=0 ; i<2*lenAlign; i+=2 ) {
            ins_vec.cut(0);
            lastvalid_i = i;
            lastvalid_isx = isx;
            basecall = basecallOnMatchIndex(i,&qrAlStr,_quality_threshold, &isx);

            if( isx < basecall_start || basecall < 0) {
                continue;
            } else if( isx > w->curSubPOS+ size()-1){
                break;
            }

            al2Cl=true;
            while( i > lastvalid_i ) {
                ins_vec.vadd(1,basecallOnQryIndex(m[lastvalid_i+1],&qrAlStr, _quality_threshold));
                lastvalid_i += 2;
                if(i == lastvalid_i) {
                    addInsertion(w->clID,lastvalid_isx,ins_vec,rpts,hdr->idSub());
                    w->addL(lastvalid_isx,rpts,sBioseqpopul::baseIns,hdr->idSub());
                    if(refill_mode ==1 || refill_mode == -1) {
                        removeInsertion(wp->clID,lastvalid_isx,ins_vec,rpts,hdr->idSub());
                        wp->removeL(lastvalid_isx,rpts,sBioseqpopul::baseIns,hdr->idSub());
                    }
                }
            }
            while( isx > lastvalid_isx ) {
                ++lastvalid_isx;
                if( isx > lastvalid_isx ) {
                    if( lastvalid_isx < w->curSubPOS )
                        continue;
                    t_basecall = sBioseqpopul::baseGap;
                } else
                    t_basecall = basecall;

                w->addL(lastvalid_isx, rpts, (sBioseqpopul::baseCalls) t_basecall, hdr->idSub());
                if(refill_mode ==1 || refill_mode == -1)
                    wp->removeL(lastvalid_isx, rpts, (sBioseqpopul::baseCalls) t_basecall, hdr->idSub());
                else
                    *_coverage.pos(lastvalid_isx - w->curSubPOS) += rpts;
            }
        }
        if(lastvalid_isx > _lastSupportedPosition)
            _lastSupportedPosition = lastvalid_isx;
        if(al2Cl)
            ++numofAlsCl;

        setAlCloneIdx(qrAlStr.iAl,w->clID);
    }

#ifdef _DEBUG
    for(idx ll = 0 ; ll < added_als.dim() ; ++ll) {
        if(ptr(ll)->valid())
            ::printf("clone %" DEC " added %" DEC "  ",ll, added_als[ll]);
    }
    ::printf("\n");
#endif
    if(refill_mode==1) {
        numofAlsCl += fill(-1, relativeTo);
    } else if( refill_mode == 0 && cntAlive() > 1 ) {
        numofAlsCl += fill(2,relativeTo);
    }
    return numofAlsCl;
}

idx sPopul::isBifurcatedPosition(sBioseqpopul::position * p, idx clID, idx pos, real cutoff,idx minCov, idx * firstInd, sVec<idx> & secInd, idx * compare)
{
    secInd.cut(0);secInd.resize(1);
    if( p->isBifurcatingPosition(_cutoff,minBifCov,firstInd,secInd.ptr(),compare) )
        return true;
    secInd.cut(0);
    if( !p->isSupportedInsertion(compare,cutoff,minCov) ) {
       return false;
    }

    idx major_ins_cov;
    getDominantInsertion(clID,pos,major_ins_cov);
    if( !p->isSupportedInsertion(compare,cutoff,minCov,&major_ins_cov) ) {
       return false;
    }

    secInd.vadd(1,*firstInd);
    idx insDiff, minResCov; real freq;
    if( p->isInsertionPhased(*firstInd,compare,cutoff,minCov,&major_ins_cov) ) {
        idx second_ins_cnt;
        getSecondDominantInsertion(clID,pos,secInd,second_ins_cnt);

        insDiff = sAbs(p->getBaseCoverage((sBioseqpopul::baseCalls)*firstInd) - second_ins_cnt);
        minResCov = sMin(insDiff, (p->getCoverage(false)-insDiff));
        freq = (real) minResCov / (compare ? (*compare) : p->getCoverage(false));
        if( (freq > cutoff) && minResCov >= minCov ) {
            return true;
        }
        return false;
    }

    getDominantInsertion(clID,pos,secInd,major_ins_cov);
    insDiff = sAbs(p->getBaseCoverage((sBioseqpopul::baseCalls)*firstInd) - major_ins_cov);
    minResCov = sMin(insDiff, (p->getCoverage(false)-insDiff));
    freq = (real) minResCov / (compare ? (*compare) : p->getCoverage(false));
    return ( (freq > cutoff) && minResCov >= minCov );
}

idx sPopul::getBifurcationStart(sClone * w, idx is, idx end) {
    idx i = is;
    while(isClonalPositionFuzzy(w,i,fuzzy_threshold) && i<end){
        ++i;
    }
    if(i>=end)return i;
    if(!w->hasParent()) return i;
    sClone * wp = getValidClone(w->parentClID);
    if(!wp) return w->bifurcation_pos;

    for(;i<end ; ++i) {
        if(isDiffPositionInsertion(w,wp,i,fuzzy_threshold,0)) {
            break;
        }
    }
    if( i > w->bifurcation_pos )
        i = w->bifurcation_pos;
    return i;
}

void sPopul::_extract(sClone * w, sCloneConsensus * t, idx start, idx extred) {
    sBioseqpopul::position * p = 0;
    sBioseqpopul::clonePosition * cp = 0;
    idx ins_cov = 0, common_coord_i = 0;
    idx i = start;
    sStr s_insertions;
    for(; i < extred; ++i) {
        p = w->pos(i);
        common_coord_i = _consensus->common_coord_pos(i);
        cp = t->seqCov.ptrx( common_coord_i - t->summary.start );
        if( p->isGap(gap_thrshld)) {
            cp->setGap(p);
        }
        else {
            cp->setPosition(p);
        }
        _consensus->getSimilarities(t->summary.clID,common_coord_i,p);
        ins_cov = getPhasedInsertionStr(w,i,_cutoff,minBifCov,s_insertions,true);
        idx common_ins_size = _consensus->get_common_coord_ins_size(i);
        if( !ins_cov ) {
            s_insertions.cut0cut(0);
        }
        for(idx ins = 0 ; ins < s_insertions.length() ; ++ins ) {
            cp = t->seqCov.add();
            cp->setBase((idx)sBioseq::mapATGC[(idx)*(s_insertions.ptr(ins))]);
            cp->setCoverage(ins_cov);
            _consensus->getInsSimilarities(t->summary.clID,t->summary.start + t->seqCov.dim() - 1,p, s_insertions.ptr());
        }
        idx extended_coord_ins = common_ins_size - s_insertions.length();
        if ( extended_coord_ins > 0 ) {
            for(idx ie = 0 ; ie < extended_coord_ins ; ++ie) {
                cp = t->seqCov.add();
                cp->setBase(-1);
                cp->setCoverage(p->getCoverage(true));
                _consensus->getSimilarities(t->summary.clID,t->summary.start + t->seqCov.dim() - 1,p);

            }
        }
    }
    common_coord_i = _consensus->common_coord_pos(i);
    t->stats.support = w->supportCnt;
    if( w->merged || w->killed ) {
        t->summary.end = common_coord_i;
        if( i >= w->lastSubPOS ) {
            w->dead = true;
        }
    }
}

idx sPopul::extractCons(idx cPOS) {
    sStr s_insertions;
    for(idx k = 0; k < cnt(); ++k) {
        sClone * w = ptr(k);
        idx extrst = w->extrPos;
        if( !w->active() ) continue;


        idx extred = (toEnd ? w->getTailLast() : (extrst + cPOS)), i = extrst;

        if( w->merged || w->killed ){
            if(w->lastSubPOS < extred )
                extred = w->lastSubPOS;
        }

        for(; i < extred; ++i) {
            idx ins_cov = getPhasedInsertionStr(w,i,_cutoff,minBifCov,s_insertions,true);
            if( ins_cov ) {
                _consensus->register_frameshift(w->clID,i,s_insertions.length());
            }
        }

    }
    for(idx k = 0; k < cnt(); ++k) {
        sClone * w = ptr(k);
        idx extrst = w->extrPos;
        if( w->active() ) {
            idx extred = (toEnd ? w->getTailLast() : (extrst + cPOS)), i = extrst;

            if( w->merged || w->killed ){
                if(w->lastSubPOS < extred )
                    extred = w->lastSubPOS;
            }

            sCloneConsensus * t = _consensus->ptrx(w->clID);

            if( !(t->seqCov.dim()) && extrst != extred ) {
                i = w->getfirstNonZero(&i, &extred);
                t->summary.clID = w->clID;
                t->summary.start = _consensus->common_coord_pos(i);
                t->summary.first_bif_pos = _consensus->common_coord_pos(w->bifurcation_pos);
                t->summary.end = -1;
                if( t->summary.start < _consensus->sub_start ){
                    _consensus->sub_start = t->summary.start;
                }
                if( w->bifurcation_bayes != sNotIdx ) {
                    t->stats.bifurcation_bayes = w->bifurcation_bayes;
                }
                if( w->bifurcation_pvalue != sNotIdx ) {
                    t->stats.bifurcation_pvalue = w->bifurcation_pvalue;

                }
            }
            t->summary.parentClID = w->parentClID;
            t->summary.mergeclID = (w->merged ? w->mergedToCl : w->clID);

            _extract(w,t,i,extred);
        }
        w->extrPos += cPOS;
    }
    for(idx ic = 0 ; ic < cnt() ; ++ic) {
        if( ptr(ic)->dead && ptr(ic)->dim() )
            ptr(ic)->destroy();
    }
    return 1;
}
bool sPopul::areMerging(sClone * w1, sClone * w2) {
    idx Merge = 0, sup_Merge = 0;
    if( w2->valid() && w1->valid()) {
        idx st = w2->getrawfirst(), ed = w2->getTailLast() - 1;
        bool mergeB = false;
        if( w1->curSubPOS == w1->startSubPOS )
            st = w1->getfirstNonZero(&st);
        if( w2->curSubPOS == w2->startSubPOS )
            st = w2->getfirstNonZero(&st);
        idx i = st;
        ed = w1->getlastNonZero(&st, &ed);
        ed = w2->getlastNonZero(&st, &ed);
        idx overlapSupported = 0;
        bool outOfFuzzy = false;
        idx mergeLen = ed-st+1;
        if( mergeLen > w2->length() )
            mergeLen = w2->length();
        for(i = ed; i >= st; --i) {
            if( !outOfFuzzy && (isClonalPositionFuzzy(w1,i,fuzzy_threshold) || isClonalPositionFuzzy(w2,i,fuzzy_threshold))) {
                ++Merge;
            } else {
                outOfFuzzy = true;
                ++overlapSupported;
                if( !isDiffPositionInsertionBasecalls(w1,w2,i,fuzzy_threshold,0) ) {
                    ++Merge;
                    ++sup_Merge;
                }
                else {
                    if( w2->parentClID == w1->clID ) {
                        ++w2->supportCnt;
                    }
                    sup_Merge = 0;
                    Merge = 0;
                }
            }

            if( Merge >= mergeLen ) {
                --i;
                mergeB = true;
                break;
            }
        }
        if( sup_Merge >= overlapSupported && overlapSupported >= mergeLen ) {
            mergeB = true;
        }

        if( mergeB && w2->mergingScore < Merge ) {
            w2->merging = true;
            w2->mergeMatPOS = w2->matPos(++i);
            w2->mergingScore = Merge;
            if( !w1->merged )
                w2->mergingCl = w1->clID;
            else {
                w2->mergingCl = w1->mergedToCl;
            }
            return true;
        }
    }
    return false;
}
idx sPopul::merge(void) {
    for(idx k = cnt()-1; k >= 0; --k) {
        sClone * w1 = ptr(k);
        if( w1->valid() ) {
            for(idx l = k - 1; l >= 0 ; --l) {
                if(areMerging(ptr(l),w1))
                    break;
            }
        }
    }

    idx cntMerged = 0;
    for(idx i = cnt() - 1; i >= 0; --i) {
        sClone * wSrc = ptr(i);
        bool diff_parents = false;
        if( wSrc->merging && wSrc ) {
            sClone * wDest = getValidClone(wSrc->mergingCl);
            if(wSrc->hasParent() && wSrc->parentClID!=wDest->clID )
                diff_parents = true;

            if( diff_parents && wSrc->subPos(wSrc->mergeMatPOS) < wSrc->bifurcation_pos ) {
                wSrc->bifurcation_pos = wSrc->subPos(wSrc->mergeMatPOS) - 1;
            }

            for(idx j = i ; j < cnt() ; ++j ) {
                sClone * chW = ptr(j);
                if( chW->parentClID == wSrc->clID && chW->valid() && chW->bifurcation_pos > wSrc->subPos(wSrc->mergeMatPOS) ) {
                    chW->parentClID = wDest->clID;
                }
            }
            sBioseqpopul::clonePat clfp;
            if( wSrc->createdWithinFrame() && !getSingleFingPrint(&clfp, wDest->clID, wSrc->clID, fuzzy_threshold, wSrc->curSubPOS) ) {
                mergeClones(wDest, wSrc, wSrc->matPos(wSrc->getrawfirst()));
                if( wDest->createdWithinFrame(1) && diff_parents ) {
                    idx lastDest = getLastDiff(wDest,getValidClone(wDest->parentClID),wDest->getrawfirst(),wDest->subPos(wDest->mergeMatPOS),fuzzy_threshold);
                    idx last_examining_pos = getLastNonFuzzyPosition(wDest,fuzzy_threshold);
                    idx firstSrc = getFirstDiff(wDest,getValidClone(wSrc->parentClID),wDest->getrawfirst(),last_examining_pos,fuzzy_threshold);
                    if(lastDest >=0 && firstSrc>=0 && firstSrc>=lastDest && wDest->hasParent() && wDest->parentClID > wSrc->parentClID) {
                        wDest->parentClID = wSrc->parentClID;
                        if(wDest->bifurcation_pos > wSrc->bifurcation_pos)
                            wDest->bifurcation_pos = wSrc->bifurcation_pos;
                    }
                }

                wSrc->markAsDead();

            } else {
                mergeClones(wDest, wSrc, wSrc->mergeMatPOS);
            }
            ++cntMerged;
#ifdef _DEBUG
        ::printf("Merged clone:%" DEC " to : %" DEC " in position: %" DEC "\n", wSrc->clID, wSrc->mergedToCl,wSrc->lastSubPOS);
#endif
        }
    }

#ifdef _DEBUG
    if(cntMerged)::printf("Newly Merged clones:%3" DEC "\n",cntMerged);
#endif
    return cntMerged;
}

idx sPopul::kill(void) {
    sClone*w1, *w2;
    idx cntKilled = 0;
    for(idx i = cnt()-1; i >= 0; --i) {
        w1 = ptr(i);
        if( w1->valid() ) {
            idx st = w1->getrawfirst(), ed = w1->getTailLast();
            st = w1->getfirstNonZero(&st, &ed);
            idx j = 0;
            idx last_covered = st;
            for(j = ed-1; j >= st; --j) {
                if( !isClonalPositionFuzzy(w1,j,fuzzy_threshold) ){
                    last_covered = j;
                    break;
                }
            }
            if( last_covered - st < (win_size()) && ( cntAlive() > 1 || !cloneContinuity ) ) {
                idx cur_last_diff = ed;
                idx first_last_diff = ed;
                idx first_cl = i;
                for( idx ij = 0 ; ij < cnt() ; ++ij ) {
                    if(i==ij)continue;
                    w2 = ptr(ij);
                    if(!w2->valid())continue;
                    cur_last_diff = getLastDiff(w1, w2, w1->getrawfirst(), w1->getTailLast(),fuzzy_threshold);

                    if(cur_last_diff < first_last_diff) {
                        first_cl = ij;
                        first_last_diff = cur_last_diff + 1;
                    }
                }
                if(first_cl!=i && (ed-first_last_diff)>(w1->length()/2)) {
                    if(first_last_diff<w1->bifurcation_pos)
                        first_last_diff = w1->bifurcation_pos+3;
                    w1->mergeMatPOS = w1->matPos(first_last_diff);
                    mergeClones(ptr(first_cl), w1, w1->mergeMatPOS);
                } else {
                    w1->killed = true;
                    w1->lastSubPOS = j;
#ifdef _DEBUG
                    ::printf("Killed clone with id:%" DEC "\nBecause of covered size (%" DEC ") being less than the window size(%" DEC ")\n", w1->clID, j - st + 1, win_size());
#endif
                    ++cntKilled;
                }
            }
        }
    }
    return cntKilled;
}

idx sPopul::computeStep(void)
{
    sClone * w = 0;
    idx minI = win_size() - minOverlap, step = 0;
    for(idx j = 0; j < cnt(); ++j) {
        w = ptr(j);
        if( !w )
            continue;
        if( w->valid() ) {
            idx st = w->getTailfirst(), ed = w->getTailLast();
            st = w->getfirstNonZero(&st, &ed);

            for(idx i = ed - 1; i >= st; --i) {
                if( !isClonalPositionFuzzy(w,i,fuzzy_threshold) ) {
                    if( i - st>=0 && (i - st <= step || !step) )
                        step = i - st;
                    break;
                }
            }
        }
    }
    if( step<=0 ) {
        step = win_size();
    }
    return step > minI ? step : minI;
}

bool sPopul::bifurcate()
{
    idx numOfbifurcEvents=0,i;
    idx frameSize=cnt();
    for(i=0; i<frameSize;++i)
    {
        if(numOfbifurcEvents)
            break;
        sClone * w=ptr(i);
        if( w->valid() )
        {
            bool found=false;
            correctInsertions(w);

            idx stPosW= w->getfirst();
            idx endPosW=w->getlast()+1;
            resetAvCoverage();
            _bfrcS.bases.cut(0);
            for(idx posW=stPosW ; posW<endPosW && !found; ++posW)
            {
                sBioseqpopul::position * p=w->pos(posW);
                if( isFuzzy(w,posW,fuzzy_threshold) )continue;
                idx firstind=-1;
                idx t_cov = *_coverage.pos(posW-w->curSubPOS);

                if( isBifurcatedPosition( p, w->clID, posW,  _cutoff,minBifCov,&firstind,_bfrcS.bases,&t_cov) ) {
                    idx newoff = w->offset + posW - stPosW;
                    if( !w->wasBifurcationConsidered( w->curSubPOS + newoff , _bfrcS.bases) ) {
#ifdef _DEBUG
                        ::printf("---bifurcate at clone %" DEC "-----------------------------------------------------------\n"
                                 "in Sub %" DEC "/Mat %" DEC " with Coverage=%" DEC " and bifurc cover=%" DEC " and letters: %c - %c\n"
                                 "Sum Coverage in this position=%" DEC " and average Coverage=%lf\n"
                                 "----------------------------------------------------------------------------------\n",i,posW,w->matPos(posW),p->getCoverage(true),p->base[_bfrcS.bases[0]],sBioseq::mapRevATGC[firstind],sBioseq::mapRevATGC[_bfrcS.bases[0]],*_coverage.pos(i),avCoverage());
#endif
                        _bfrcS._fatherclID = w->clID;
                        _bfrcS._POS = w->curSubPOS + newoff;
                        _bfrcS._childclID = branch( w->clID, _bfrcS.bases, newoff )->clID;

                        found = true;
                        ++numOfbifurcEvents;
                        break;
                    }
                } else {
                    _bfrcS.bases.cut(0);
                }
            }
        }
    }
    return numOfbifurcEvents;
}

idx sPopul::shift(idx sStart,idx step){
    idx iAlStart=_iAlStart,iAlEnd=_iAlEnd;

    getAlBoundaries(_bioal,_order_indices,sStart+step,(sStart+win_size()),iAlStart,iAlEnd,_alCnt_Max);

    _iAlPrevStart=_iAlStart;

    move(step);
    _POS +=step;
    resetAvCoverage();

    return 1;
}
void sPopul::getBifurcationStats(bifurcation_structure * bif)
{
    if(!bif)bif = &_bfrcS;
    sClone * b = ptr(bif->_childclID);
    sClone * p = ptr(bif->_fatherclID);
    if( b->valid() && p->valid() ) {
        sBioseqpopul::position * p_pos = p->pos(bif->_POS);
        sBioseqpopul::position * b_pos = b->pos(bif->_POS);
        idx tot = *_coverage.pos(bif->_POS - b->curSubPOS);
        if(getBifurcationPvalue){
                real ref_exp = (1 - noise_tolerance)*tot;
                real mut_exp = noise_tolerance*tot;
                idx b_base = sBioseqpopul::baseGap;
                if(bif->bases.dim() > 1) {
                    b_base = sBioseqpopul::baseIns;
                } else if ( bif->bases.dim() == 1){
                    b_base = bif->bases[0];
                }
                real d = mut_exp?(real)(mut_exp - b_pos->getBaseCoverage((sBioseqpopul::baseCalls)b_base))*(mut_exp - b_pos->getBaseCoverage((sBioseqpopul::baseCalls)b_base))  /  mut_exp:0;

                d += ref_exp?(real)(ref_exp - (tot-b_pos->getBaseCoverage((sBioseqpopul::baseCalls)b_base)))*(ref_exp - (tot-b_pos->getBaseCoverage((sBioseqpopul::baseCalls)b_base)))  /  ref_exp : 0;

                b->bifurcation_pvalue = 1 - sFunc::special::gammap( 0.5, d / 2);
        }
        if(getBifurcationBayes) {
            real p_cln = (real)b_pos->getCoverage(false)/(p_pos->getCoverage(false)+b_pos->getCoverage(false));
            real p_ref = 1- p_cln;

            b->bifurcation_bayes = p_cln/(p_cln + noise_tolerance*p_ref);
        }
    }
}
idx sPopul::allSortComparator(void * parameters, void * A, void * B,void * objSrc,idx i1,idx i2 ){
    ParamsAllSubjSorter * cparams=(ParamsAllSubjSorter *)parameters;
    sBioal::ParamsAlignmentSorter * param=&cparams->sorterParams;

    sBioseqAlignment::Al * hdrA= (sBioseqAlignment::Al * )A; sBioseqAlignment::Al * hdrB=(sBioseqAlignment::Al * )B;
    idx compVal1=0,compVal2=0,compId1=0,compId2=0;
    idx * mA=param->bioal->getMatch(i1),offsfromA=hdrA->subStart();
    idx * mB=param->bioal->getMatch(i2),offsfromB=hdrB->subStart();

    if(cparams->alSub){
        idx vecMA[2]={*mA,0},vecMB[2]={*mB,0};
        mA=(idx *)&vecMA;
        mB=(idx *)&vecMB;

        sBioseqAlignment::Al * hdrToA= *cparams->alSub->ptr(hdrA->idSub());
        idx * mToA=hdrToA->match(),offsToA=hdrToA->subStart();
        if( cparams->alSubLUT && sBioseqAlignment::remapAlignment(hdrA,hdrToA,mA,mToA,1,true,cparams->alSubLUT->ptr(hdrA->idSub())))
            offsfromA+=offsToA;


        sBioseqAlignment::Al * hdrToB= *cparams->alSub->ptr(hdrB->idSub());
        idx * mToB=hdrToB->match(),offsToB=hdrToB->subStart();
        if( cparams->alSubLUT && sBioseqAlignment::remapAlignment(hdrB,hdrToB,mB,mToB,1,true,cparams->alSubLUT->ptr(hdrB->idSub())))
            offsfromB+=offsToB;

    }

    if(param->flags&sBioal::alSortByPosStart)
    {
        compVal1=offsfromA+ mA[0];
        compVal2=offsfromB+ mB[0];

        if(!(param->flags^sBioal::alSortByPosStart))
            return (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1)  : 0 ;
    }
    else if(param->flags&sBioal::alSortByPosEnd){
        compVal1=offsfromA+mA[0]+1+hdrA->lenAlign();
        compVal1=offsfromB+mB[0]+1+hdrB->lenAlign();
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

    if(param->flags& sBioal::alSortByPosONSubID)
        return (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1)  : ( (compId1 != compId2)? ( (compId1 > compId2) ? 1 : -1):0) ;
    else if(param->flags&sBioal::alSortBySubIdONPos)
        return (compId1 != compId2)? ( (compId1 > compId2) ? 1 : -1)  : ( (compVal1 != compVal2)? ( (compVal1 > compVal2) ? 1 : -1):0) ;

    return 0;
}


const char * sPopul::showAliveClones(sStr * out )
{
    if( !out ) {
        out = &_strBuf;
        out->cut(0);
    }
    bool found=false;
    out->printf("clones:");
    for(idx i = 0; i < cnt(); ++i) {
        if(ptr(i)->valid()) {
            if(found)
                out->printf(",");
            out->printf("%" DEC,i);
            found=true;
        }
    }
    return out->ptr();
}

const char * sPopul::print(sStr * out){
    if( !out ) {
        out = &_strBuf;
        out->cut(0);
    }
    for(idx i = 0 ; i < cnt() ; ++i ) {
        sClone * c = ptr(i);
        if( c->valid() ) {
            c->print(_cutoff,minBifCov,out);
            out->printf("\n");
        }
    }
    return printCoverage(out);
}
