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
#include <ssci/bio/bioseqpopul.hpp>

using namespace slib;


void sFrame::move(idx steps)
{
    _coverage.move(steps);
    sClone * wk = getMainClone();
    if( !wk )
        return;
    resetAvCoverage();


    for(idx k = 0; k < cnt(); ++k) {
        wk = ptr(k);
        if( wk->valid() )
            wk->move(steps);
    }
}

void sFrame::setCalls(void)
{
    sClone * w = 0;
    for(idx i = 0; i < cnt(); ++i) {
        w = ptr(i);
        if( w && w->active() ) {
            w->setCalls();
            correctInsertions(w);
        }
    }
}

void sFrame::correctInsertions( sClone * w )
{
    idx st=w->getrawfirst(), end=w->getTailLast();
    for(idx ip = st; ip < end; ++ip) {
        sDic<idx> * _insertions =  getInsertionsDic(w->clID,ip);
        if( _insertions && _insertions->dim() > 1) {
            idx rpts = 0, len = 0, c_len = 0, ic = 0;
            const char * ins = getDominantInsertionChar(w->clID,ip,len,rpts), * c_ins = 0;
            for(idx in = 0 ; in < _insertions->dim() ; ++in ) {
                c_ins =(const char *)_insertions->id(in,&c_len);
                if(c_ins == ins) continue;

                if( c_len == len ) {
                    for( ; ic < len && (ins[ic]==c_ins[ic] || c_ins[ic]=='N' || ins[ic]=='N'); ++ic);
                    if( ic == len ) {
                        *_insertions->get(ins,len) += *_insertions->ptr(in);
                        *_insertions->ptr(in) = 0;
                    }
                }
            }
        }
    }
}

void sFrame::resize(idx addSize)
{
    for(idx i = 0; i < cnt(); ++i) {
        sClone * w = ptr(i);
        if( !w->valid() )
            continue;
        w->strech(addSize);
    }
    _coverage.stretch(addSize);
}


idx sFrame::getSingleFingPrint(sBioseqpopul::clonePat * p, idx parentClID, idx childID, real fuzzy_coverages_threshold, idx bif_start, idx * start, idx * end)
{
    if( parentClID < 0 || childID < 0 )
        return 0;

    real av_cov = avCoverage();
    if( !av_cov )
        return 0;
    sClone * w0 = ptr(parentClID);
    sClone* w1 = ptr(childID);

    if( !w1->valid() )
        return 0;

    idx st = start ? *start : w1->getrawfirst(), ed = end ? *end : w1->getTailLast();

    p->curpos = -1;
    p->clID = childID;
    p->pattern.cut(0);

    idx i = st;
    idx post_bifurcation_diffs = 0;

    if( w1->curSubPOS == w1->startSubPOS )
        i = w1->getfirstNonZero(&i, &ed);
    if( w0->curSubPOS == w1->startSubPOS )
        i = w0->getfirstNonZero(&i, &ed);

    ed = w1->getlastNonZero(&i, &ed);
    ed = w0->getlastNonZero(&i, &ed);
    for(; i < ed; ++i) {
        sBioseqpopul::position * p1 = w1->pos(i);
        sBioseqpopul::position * p0 = w0->pos(i);

        p1->setbasecall();
        p0->setbasecall();
        if( isDiffPositionInsertionBasecalls(w0,w1,i,_cutoff,0) ) {
            if( !isClonalPositionFuzzy(w0,i,fuzzy_coverages_threshold) && !isClonalPositionFuzzy(w1,i,fuzzy_coverages_threshold) ) {
                sBioseqpopul::points * t = p->pattern.add();
                getPositionInsertionBasecalls( w1, i, _cutoff, 0, t->bases);
                t->position = i;
                if( i > bif_start )
                    ++post_bifurcation_diffs;
            }
        }
    }

    if( (p->pattern.dim()) <= 1 || post_bifurcation_diffs == 0 ) {
        return 0;
    }
    return 1;
}

static sStr st_buf;

const char * sFrame::diff(idx cl1, idx cl2, bool onlydiffs)
{
    sClone * w1 = ptr(cl1);
    sClone * w2 = ptr(cl2);
    if( !w1 || !w2 ) {
        return 0;
    }
    sStr * out = &st_buf;
    idx cnt = 0, cov_base1 = 0, cov_base2 = 0;
    out->printf(0, "| # |  sub  |%7" DEC "|  cov  |%7" DEC "|  cov  |  dif  |\n", w1->clID, w2->clID);
    out->printf("|---+-------+-------+-------+-------+-------+-------|\n");
    for(idx i = 0; i < w1->dim(); ++i) {
        bool isdiff = false;
        sBioseqpopul::position * p1 = w1->ptr(i);
        p1->setbasecall();
        sBioseqpopul::position * p2 = w2->ptr(i);
        p2->setbasecall();
        sStr flag;
        flag.printf("   ");
        if( p1->basecall != p2->basecall && p1->basecall >= 0 && p2->basecall >= 0 ) {
            isdiff = true;
            flag.printf(0, "***");
            ++cnt;
        }
        if(onlydiffs && !isdiff)
            continue;
        if( p1->basecall >= 0 && p1->basecall <= 4 ) {
            cov_base1 = p1->base[p1->basecall];
        } else {
            cov_base1 = 0;
        }
        if( p2->basecall >= 0 && p2->basecall <= 4 ) {
            cov_base2 = p2->base[p2->basecall];
        } else {
            cov_base2 = 0;
        }
        out->printf("|%3" DEC "|%7" DEC "|   %c   |%7" DEC "|   %c   |%7" DEC "|%7s|\n", i, w1->subPos(i), p1->basechar(), cov_base1, p2->basechar(), cov_base2, flag.ptr());
    }
    out->printf("|-----------------------------------+-Tot-diff--%3" DEC "-|\n", cnt);
    return out->ptr(0);
}

idx sClone::cmpClones(void * param, void * arr, idx i1, idx i2)
{
    sClone * clones = static_cast<sClone*>(arr);
    if(!clones[i1].active() || !clones[i2].active()) {
        return -1;
    }
    idx iv1 = clones[i1].getfirstNonZero();
    idx iv2 = clones[i2].getfirstNonZero();

    return  iv1 - iv2;
}


bool sFrame::getAlBoundaries(sBioal * bioal, idx * alList, idx posStart, idx posEnd, idx &iAlStart, idx &iAlEnd, idx alCnt)
{
    sBioseqAlignment::Al * hdrA = bioal->getAl(alList ? alList[iAlStart] : iAlStart);
    idx compValue = hdrA->getSubjectStart(bioal->getMatch(alList ? alList[iAlStart] : iAlStart));

    while( compValue < posStart ) {
        if( iAlStart == alCnt )
            break;
        ++iAlStart;
        hdrA = bioal->getAl(alList ? alList[iAlStart] : iAlStart);
        compValue = hdrA->getSubjectStart(bioal->getMatch(alList ? alList[iAlStart] : iAlStart));
        compValue += bioal->getMatch(alList ? alList[iAlStart] : iAlStart)[0];
    }
    if( iAlEnd == alCnt ) {
        return 1;
    }
    hdrA = bioal->getAl(alList ? alList[iAlEnd] : iAlEnd);
    compValue = hdrA->getSubjectStart(bioal->getMatch(alList ? alList[iAlEnd] : iAlEnd));
    while( compValue < posEnd ) {
        ++iAlEnd;
        if( iAlEnd == alCnt ) {
            return 1;
        }
        hdrA = bioal->getAl(alList ? alList[iAlEnd] : iAlEnd);
        compValue = hdrA->getSubjectStart(bioal->getMatch(alList ? alList[iAlEnd] : iAlEnd));
        if( iAlEnd == alCnt )
            break;
    }
    return (iAlEnd == alCnt);
}

sClone * sFrame::branch(idx prevID, sVec<idx> & setbase, idx t_offset)
{
    sClone * cl = add();
    sClone * w = ptr(prevID);
    w->registerBifurcation(w->curSubPOS+t_offset, setbase);
    w->reset(t_offset - w->offset);

    cl->init(w, cnt() - 1);
    return cl;
}

void sClone::init(sClone * w, idx t_clID)
{
    init(w->dim() / 2, w->offset, t_clID, w->ref_cnt);
    _report_ref = w->_report_ref;
    parentClID = w->clID;
    extrPos = w->extrPos;
    bifurcation_pos = w->curSubPOS + w->offset;
    startSubPOS = w->curSubPOS;
    curMatPOS = w->curMatPOS;
    curSubPOS = w->curSubPOS;
    bifurcation_bayes = sNotIdx;
    bifurcation_pvalue = sNotIdx;
}
;

void sClone::move(idx step)
{
    idx st = getrawfirst(), ed = st + step;
    for(idx i = st; i < ed; ++i) {
        pos(i)->init();
    }
    curMatPOS = mod(step);
    curSubPOS += step;
    mergingScore = 0;
    offset = 0;

}
;

void sClone::strech(idx addSize)
{
    idx oldSize = dim(), real_add = 2 * addSize;
    idx last = getTailLast() - 1, first = getfirst();
    add(2 * addSize);

    sBioseqpopul::position * p = 0;
    idx _mat_pos = 0;
    for(idx j = last; j >= first; --j) {
        _mat_pos = matPos(j);
        if( _mat_pos < curMatPOS || _mat_pos >= oldSize )
            continue;

        p = pos(j);
        if( p ) {
            pos(j + real_add)->copy(*p);
            p->init();
        }
    }

    curMatPOS += 2 * addSize;
    if( mergeMatPOS ) {
        mergeMatPOS += 2 * addSize;
    }
}

void sClone::mergeTo(sClone * dest, idx matPos)
{
    idx ed = getTailLast();
    merging = false;
    merged = true;
    mergedToCl = dest->clID;
    lastSubPOS = subPos(matPos);

    for(idx i = lastSubPOS; i < ed; ++i) {
        dest->pos(i)->transfer(*pos(i));
    }
}

const char * sClone::print(real cutoff, idx minCov, sStr * out)
{
    if( !out ) {
        out = &st_buf;
        out->cut(0);
    }
    out->printf("|Clone__%7" DEC "_____________________________________________________________________________|\n", clID);
    out->printf("| # |  sub  |   A   |   C   |   G   |   T   |  Del  |  Ins  |  GAP  |  Any  |  SNV  | flags |\n");
    out->printf("|---+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|\n");
    sBioseqpopul::position * p = 0;
    for(idx i = 0; i < dim(); ++i) {
        p = ptr(i);
        sStr flag;
        flag.printf(" ");
        if( curMatPOS == i )
            flag.printf(0, " M");
        if( mod(offset) == i ) {
            flag.printf(" O");
        }
        out->printf("|%3" DEC "|%7" DEC "|%7" DEC "|%7" DEC "|%7" DEC "|%7" DEC "|%7" DEC "|%7" DEC "|%7" DEC "|%7" DEC "|%7s|%7s|\n",
            i, subPos(i), p->base[sBioseqpopul::baseA], p->base[sBioseqpopul::baseC], p->base[sBioseqpopul::baseG],
            p->base[sBioseqpopul::baseT], p->base[sBioseqpopul::baseDel], p->base[sBioseqpopul::baseIns], p->base[sBioseqpopul::baseGap], p->base[sBioseqpopul::baseN],
            p->isBifurcatingPosition(cutoff,minCov) ? "  ***  " : "", flag.ptr());
    }
    out->printf("|-------------------------------------------------------------------------------------------|\n");
    return out->ptr();
}

const char * sFrame::printCoverage(sStr * out)
{
    if( !out ) {
        out = &st_buf;
        out->cut(0);
    }
    out->printf("|Coverage___________________|\n");
    out->printf("| # |  sub  |  Cov  | flags |\n");
    out->printf("|---+-------+-------+-------|\n");
    for(idx i = 0; i < _coverage.dim(); ++i) {
        sStr flag;
        flag.printf(" ");
        if( _coverage.curMatPOS == i )
            flag.printf(0, " M");
        out->printf("|%3" DEC "|%7" DEC "|%7" DEC "|%7s|\n", i, getMainClone()->subPos(i), _coverage[i], flag.ptr());
    }
    out->printf("|---------------------------|\n");
    return out->ptr();
}


idx sFrame::getFirstDiff(sClone * w1, sClone * w2,idx start, idx end, real cutoff)
{
    if( !w1 || !w2) return -1;
    idx w1_end = getLastNonFuzzyPosition(w1,cutoff)+1;
    idx w2_end = getLastNonFuzzyPosition(w2,cutoff)+1;
    end = sMin(end,sMin(w1_end,w2_end));
    for( idx i = start; i < end; ++i ) {
        if( isDiffPositionInsertionBasecalls(w1, w2, i ,cutoff, minBifCov) )
            return i;
    }
    return -1;
}


idx sFrame::getLastDiff(sClone * w1, sClone * w2,idx start, idx end, real cutoff)
{
    if( !w1 || !w2) return -1;
    idx w1_end = getLastNonFuzzyPosition(w1,cutoff)+1;
    idx w2_end = getLastNonFuzzyPosition(w2,cutoff)+1;
    end = sMin(end,sMin(w1_end,w2_end));
    for(idx i = end; i >= start; --i) {
        if( isDiffPositionInsertionBasecalls(w1, w2, i ,cutoff, minBifCov) )
            return i;
    }
    return start;
}

idx sBioseqpopul::sortClonesComparator(void * parameters, void * varr, idx i1, idx i2)
{
    ParamsCloneSorter * param = (ParamsCloneSorter *) parameters;
    sCloneConsensus * clA = ((sCloneConsensus *) varr) + i1;
    sCloneConsensus * clB = ((sCloneConsensus *) varr) + i2;

    if( clA->summary.clID < 0 && clB->summary.clID < 0 )
        return 0;
    else if( clA->summary.clID < 0 )
        return 1;
    else if( clB->summary.clID < 0 )
        return -1;
    idx compVal1 = 0, compVal2 = 0;

    if( param->flags & clSortByMaxCov ) {
        compVal1 = clA->stats.maxCov;
        compVal2 = clB->stats.maxCov;
        return (compVal1 != compVal2) ? ((compVal1 > compVal2) ? -1 : 1) : 0;
    } else if( param->flags & clSortByAvCov ) {
        compVal1 = clA->stats.avCov;
        compVal2 = clB->stats.avCov;
        return (compVal1 != compVal2) ? ((compVal1 > compVal2) ? -1 : 1) : 0;
    } else if( param->flags & clSortByPos ) {
        compVal1 = clA->summary.start;
        compVal2 = clB->summary.start;
        return (compVal1 != compVal2) ? ((compVal1 > compVal2) ? -1 : 1) : 0;
    }
    return 0;
}



sBioseqpopul::clonePosition * sCloneConsensus::getSubPos(idx pos)
{
    idx ipos=pos - summary.start;
    if(ipos < 0 || ipos >= seqCov.dim() ) {
#ifdef _DEBUG
        ::printf("Fatal ERROR: clone %" DEC " requesting position %" DEC " in a vector size %" DEC "\n",summary.clID,ipos,seqCov.dim());
#endif
        return 0;
    }
    return seqCov.ptr(ipos);
}

idx sFrameConsensus::getClonesPositionsOnLocation(idx pos, sVec<idx> &clIDs, sVec<sBioseqpopul::clonePosition> &ret )
{
    sCloneConsensus * cl = 0;
    sBioseqpopul::clonePosition * p = 0;
    for(idx i = 0 ; i < dim() ; ++i ) {
        cl = ptr(i);
        p = cl->getSubPos(pos);
        if( p ) {
            clIDs.vadd(1,i);
            ret.add()->copy(p);
        }
    }
    return clIDs.dim();
}

idx sFrameConsensus::getSimilarities(idx cloneID, idx framePos, sBioseqpopul::position * p ) {
    idx simils = 0, slen = 0;
    idx tot_sim = 0;
    const char * cr_id = 0 ;
    sStr str_id;idx * c_p_sim = 0;
    const char * inssep = "-ins-";
    for( idx i = 0 ; i < p->referenceSimil.dim() ; ++i ) {
        cr_id = (const char *)p->referenceSimil.id(i,&slen);
        str_id.cut0cut();
        str_id.add(cr_id,slen);
        str_id.add0();
        if( !strstr(str_id.ptr(),inssep) && p->referenceSimil[i] > 0 ) {
            ++simils;
            contructSimilaritiesID(cloneID,framePos,str_id);
            c_p_sim = similarities.get( _simil_id_buf.ptr() ,_simil_id_buf.length() );
            if( c_p_sim ) {
                *c_p_sim +=  p->referenceSimil[i];
            } else {
                *similarities.set( _simil_id_buf.ptr() ,_simil_id_buf.length() ) = p->referenceSimil[i];
            }
            tot_sim += p->referenceSimil[i];
        }
    }
#ifdef _DEBUG
    if( tot_sim != p->getCoverage(true) ) {
        ::printf("BAD SIMILARITIES ON extractCONSENSUS: clone :%" DEC " sim_tot = %" DEC " vs coverage=%" DEC "\n",cloneID, tot_sim, p->getCoverage(true) );
    }
#endif
    return tot_sim;
}
idx sFrameConsensus::getInsSimilarities(idx cloneID, idx framePos, sBioseqpopul::position * p, const char * ins )
{
    idx simils = 0, slen = 0, tot_sim = 0;
    const char * cr_id = 0 ;
    sStr str_id;
    const char * inssep = "-ins-";
    for( idx i = 0 ; i < p->referenceSimil.dim() ; ++i ) {
        cr_id = (const char *)p->referenceSimil.id(i,&slen);
        str_id.cut0cut();
        str_id.addString(cr_id,slen);
        char * mp = strstr(str_id.ptr(),inssep);
        if( mp && !strcmp(mp+sLen(inssep), ins) && p->referenceSimil[i] > 0 ) {
            ++simils;
            str_id.cut0cut((idx)(mp-str_id.ptr()));
            contructSimilaritiesID(cloneID,framePos,str_id);
            *similarities.set( _simil_id_buf.ptr() ,_simil_id_buf.length() ) = p->referenceSimil[i];
            tot_sim += p->referenceSimil[i];
        }
    }
    return tot_sim;
}
sCloneConsensus * sFrameConsensus::getParentOnLocation(idx iCl, idx pos)
{
    sCloneConsensus * cl = ptr(iCl);
    if( !cl->summary.hasParent() || cl->summary.parentClID>=dim()) {
        return 0;
    }
    cl = ptr(cl->summary.parentClID);
    if( sOverlap(pos,pos,cl->summary.start ,cl->getSummaryEnd() - 1) ) {
        return cl;
    } else if ( pos < cl->summary.start && cl->summary.hasParent() ){
        return getParentOnLocation(cl->summary.clID,pos);
    } else if( pos >= cl->getSummaryEnd() && cl->summary.hasMerged() ) {
        return getMergedOnLocation(cl->summary.clID,pos);
    }
    return 0;
}

idx sFrameConsensus::getFirstMutation(idx iCl, idx pos, bool ignoreGaps)
{
    sCloneConsensus * cl = ptr(iCl);
    if( !cl->summary.hasParent() || cl->summary.parentClID>=dim()) {
        return -1;
    }
    idx cl_mut_start = cl->summary.start;
    sCloneConsensus * parent = getParentOnLocation(iCl,cl_mut_start);
    if(!parent) {
        parent = getParentOnLocation(iCl,cl->summary.first_bif_pos);
        if(!parent)
            return -1;
    }
    sBioseqpopul::clonePosition * p1 = 0, * p2 = 0;
    idx base1 = 0, base2 = 0;
    if( pos ) {
        pos -= cl_mut_start;
    } else {
        pos = cl->getFirstSupportedPosition() - cl_mut_start;
    }
    for( idx i = pos, i_p = pos; i < cl->seqCov.dim() && i_p < parent->seqCov.dim(); ++i , ++i_p) {
        if(i + cl_mut_start < parent->summary.start)
            continue;
        if( i + cl_mut_start > parent->getSummaryEnd() ) {
            break;
        }
        p1 = cl->getSubPos( i + cl_mut_start);
        if( !p1 ) {
            continue;
        }
        p2 = parent->getSubPos( i_p + cl_mut_start);
        if( !p2 ) {
            continue;
        }
        if( ignoreGaps){
            while( p1 && p1->isAnyGap() && i < cl->seqCov.dim() ) {
                ++i;
                p1 = cl->getSubPos(i+cl_mut_start);
            }
            if( i >= cl->seqCov.dim() )
                break;
            while( p2 && p2->isAnyGap() && i_p < parent->seqCov.dim() ) {
                ++i_p;
                p2 = parent->getSubPos(i_p + cl_mut_start);
            }
            if( i_p >= parent->seqCov.dim() )
                break;
        }
        if(!p1 || !p2) continue;
        base1 = p1->base();
        base2 = p2->base();
        if( base1 != base2 && base1 >= 0 && base2 >= 0) {
            return i+cl_mut_start;
        }
    }
    return -1;
}

idx sFrameConsensus::getLastMutation(idx iCl, idx pos, bool ignoreGaps)
{
    sCloneConsensus * cl = ptr(iCl);
    if( !cl->summary.hasMerged() || cl->summary.mergeclID>=dim()) {
        return -1;
    }
    idx cl_mut_end = cl->getSummaryEnd();
    idx cl_mut_start = cl->summary.start;
    sCloneConsensus * merged = getMergedOnLocation(iCl,cl_mut_end);
    if(!merged) {
        return -1;
    }
    sBioseqpopul::clonePosition * p1 = 0, * p2 = 0;
    idx base1 = 0, base2 = 0;
    if( pos ) {
        pos -= cl_mut_start;
    } else {
        pos = cl->getLastSupportedPosition() + 1 - cl_mut_start;
    }
    idx res = -1;
    for( idx i = 0, i_p = 0; i < pos && i_p < cl->seqCov.dim(); ++i , ++i_p) {
        if(i + cl_mut_start < merged->summary.start)
            continue;
        if( i + cl_mut_start > merged->getSummaryEnd() ) {
            break;
        }
        p1 = cl->getSubPos( i + cl_mut_start);
        if( !p1 ) {
            continue;
        }
        p2 = merged->getSubPos( i_p + cl_mut_start);
        if( !p2 ) {
            continue;
        }
        if( ignoreGaps){
            while( p1 && p1->isAnyGap() && i < pos ) {
                ++i;
                p1 = cl->getSubPos(i+cl_mut_start);
            }
            if( i >= pos )
                break;
            while( p2 && p2->isAnyGap() && i_p < cl->seqCov.dim() ) {
                ++i_p;
                p2 = merged->getSubPos(i_p + cl_mut_start);
            }
            if( i_p >= cl->seqCov.dim() )
                break;
        }
        base1 = p1->base();
        base2 = p2->base();
        if( base1 != base2 && base1 >= 0 && base2 >= 0) {
            res = i+cl_mut_start;
        }
    }
    return res;
}

sCloneConsensus * sFrameConsensus::getMergedOnLocation(idx iCl, idx pos)
{
    sCloneConsensus * cl = ptr(iCl);
    if( !cl->summary.hasMerged() || cl->summary.mergeclID>=dim()) {
        return 0;
    }
    cl = ptr(cl->summary.mergeclID);
    if( sOverlap(pos,pos,cl->summary.start ,cl->getSummaryEnd() ) ) {
        return cl;
    } else if ( pos < cl->summary.start && cl->summary.hasParent() ){
        return getParentOnLocation(cl->summary.clID,pos);
    } else if( pos >= cl->getSummaryEnd() && cl->summary.hasMerged() ) {
        return getMergedOnLocation(cl->summary.mergeclID,pos);
    }
    return 0;
}

bool sFrameConsensus::mergePositions(sCloneConsensus * dstCl, sCloneConsensus * srcCl, idx start, idx cnt, bool adjustForGaps)
{
    if(srcCl == dstCl)return false;
    if( !start ) {
        start = srcCl->summary.start;
    }
    if( !cnt ) {
        cnt = srcCl->seqCov.dim() - (start - srcCl->summary.start);
    }
    idx frmEnd = start + cnt;
    sStr ref_ind;
        if( start < dstCl->summary.start ) {
            dstCl->seqCov.insert(0,dstCl->summary.start - start);
            dstCl->summary.start = start;
        }
        idx dimDif = start+cnt - (dstCl->summary.start + dstCl->seqCov.dim());
        if( dimDif > 0 ) {
            dstCl->seqCov.resize( dimDif );
            dstCl->summary.end += dimDif;
        }
        sBioseqpopul::clonePosition * p_dst = 0, * p_src;
        for (idx pos_src = start, pos_dst = start ; pos_src < frmEnd && pos_dst < frmEnd ; ++pos_src , ++pos_dst){
            if( adjustForGaps ) {
                p_dst = dstCl->getSubPos(pos_dst);
                while( p_dst && pos_dst < frmEnd && p_dst->isAnyGap() ) {
                    p_dst = dstCl->getSubPos(++pos_dst);
                }
                if( pos_dst >= frmEnd )
                    break;
                p_src = dstCl->getSubPos(pos_dst);
                while( p_src && pos_src < frmEnd && p_src->isAnyGap() ) {
                    p_src = srcCl->getSubPos(++pos_src);
                }
                if( pos_src >= frmEnd )
                    break;
            }
            dstCl->seqCov.ptrx(pos_dst - dstCl->summary.start)->add( srcCl->seqCov.ptr(pos_src - srcCl->summary.start) );
            for (idx i = 0 ; i < simil_size ; ++i ) {
                ref_ind.cut(0);
                ref_ind.addNum(i);
                idx * sv = similarities.get( contructSimilaritiesID(srcCl->summary.clID, pos_src, ref_ind) );
                if( sv ) {
                    idx svv = *sv;
                    idx * dv = similarities.get( contructSimilaritiesID(dstCl->summary.clID, pos_dst, ref_ind) );
                    if( dv ) {
                        *dv += svv;
                    } else {
                        dv = similarities.set( contructSimilaritiesID(dstCl->summary.clID, pos_dst, ref_ind) );
                        if(dv)
                            *dv = svv;
                        else {
                            return false;
                        }
                    }
                }
            }
        }
    return true;
}

idx sFrameConsensus::trimClones() {
    sCloneConsensus * cl = 0;
    idx start_bif_dif = 0, p = 0;
    bool killedClone = false;
    for(idx i = 0; i < dim(); ++i) {
        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }
        cl = ptr(i);
        start_bif_dif = cl->summary.first_bif_pos - cl->summary.start;
        if( start_bif_dif > 1 && start_bif_dif < cl->seqCov.dim() ){
            --start_bif_dif;
            if( cl->summary.hasParent() ) {
                sCloneConsensus * pCl = ptr(cl->summary.parentClID ), * onBpCl = pCl;

                for ( p = cl->summary.start ; p < start_bif_dif + cl->summary.start ; ++p )
                {
                    if( pCl->summary.start > p ) {
                        pCl = getParentOnLocation(cl->summary.clID,p);
                        if(!pCl) {
                            if(!onBpCl) {
                                start_bif_dif = p - cl->summary.start -1;
                                break;
                            } else {
                                pCl = onBpCl;
                                mergePositions(onBpCl,cl,p,1);
                                continue;
                            }
                        }
                    }
                    if(!mergePositions(pCl,cl,p,1)) {
                        return 0;
                    }
                }
            }
            if(start_bif_dif) {
                cl->summary.start += start_bif_dif;
                cl->seqCov.del(0,start_bif_dif);
                cl->stats.size = cl->seqCov.dim();
                fixParentDependencies(cl);
            }
        }
        if(cl->summary.hasMerged() ) {
            idx last_bif_dif = getLastMutation(cl->summary.clID, 0, true);
            if( isLink(cl) && last_bif_dif < cl->summary.start+2 ) {
                last_bif_dif = cl->summary.start+2;
            }
            if( last_bif_dif < cl->summary.start )
                last_bif_dif = cl->summary.start - 1;
            sCloneConsensus * mCl = ptr(cl->summary.mergeclID ), * onMmCl = mCl;
            for( p = cl->getSummaryEnd() - 1 ; p > last_bif_dif  ; --p ) {
                if( mCl->summary.start > p ) {
                    mCl = getMergedOnLocation(cl->summary.clID,p);
                    if(!mCl) {
                        if(!onMmCl) {
                            last_bif_dif = p;
                            break;
                        } else {
                            mCl = onMmCl;
                            mergePositions(onMmCl,cl,p,1);
                            continue;
                        }
                    }
                }
                if(!mergePositions(mCl,cl,p,1)) {
                    last_bif_dif = p;
                    break;
                }
            }
            if( last_bif_dif < cl->summary.end ) {
                cl->seqCov.del(1+last_bif_dif-cl->summary.start,cl->getSummaryEnd() - last_bif_dif - 1);
                cl->summary.end = last_bif_dif+1;
                cl->stats.size = cl->seqCov.dim();
                fixMergedDependencies(cl);
            }
        }
        if(!cl->seqCov.dim()) {
            cl->summary.clID = -1;
            killedClone = true;
        }
    }

    if(killedClone) {
        sVec<idx> cloneIndex(sMex::fSetZero);
        cloneIndex.add(dim());

        sFrameConsensus cleanClones(simil_size);
        sCloneConsensus * addedclone = 0;
        for(idx i = 0 , icl = 0; i < dim() ; ++i) {
            if( ptr(i)->isValid() ) {
                cl = ptr(i);
                addedclone = cleanClones.add();
                cleanClones._frame_shifts.add()->copy(_frame_shifts.ptr(i));
                addedclone->stats = cl->stats;
                addedclone->summary = cl->summary;

                addedclone->seqCov.copy(&cl->seqCov,false);

                for(idx j = 0; j < addedclone->seqCov.dim(); ++j) {
                    remapSimilarities(&cleanClones.similarities,i, addedclone->summary.start+j,icl);
                    addedclone->seqCov.ptr(j)->copy(cl->seqCov.ptr(j));
                }
                cloneIndex[addedclone->summary.clID] = icl;
                addedclone->summary.clID = icl++;
            }
        }

        _vec.cut(0);
        similarities.empty();
        idx pLen = 0;
        for( idx i = 0 ; i < cleanClones.similarities.dim() ; ++i ) {
            const void * p_id= cleanClones.similarities.id(i, &pLen);
            *similarities.set(p_id,pLen) = *(idx *)cleanClones.similarities.get(p_id,pLen);
        }
        _vec.resize( cleanClones.dim() );
        for(idx i = 0; i < cleanClones.dim(); ++i) {
            if(progress_CallbackFunction){
                if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                    return 0;
            }
            addedclone = ptr(i);
            cl = cleanClones.ptr(i);

            if( cl->summary.mergeclID >= 0 && cl->summary.mergeclID < cloneIndex.dim() ) {
                cl->summary.mergeclID = cloneIndex[cl->summary.mergeclID];
            }
            if( cl->summary.parentClID >= 0 && cl->summary.parentClID < cloneIndex.dim() ) {
                cl->summary.parentClID = cloneIndex[cl->summary.parentClID];
            }
            addedclone->stats = cl->stats;
            addedclone->summary = cl->summary;

            addedclone->seqCov.copy(&cl->seqCov);
        }
    }

#ifdef _DEBUG
    ::printf("TRIMMED\n");
    for(idx i = 0 ; i < dim() ; ++i ) {
        if( cl->summary.clID < 0 || !cl->seqCov.dim() ){
            continue;
        }
        sCloneConsensus * cl = ptr(i);
        if( cl->summary.hasParent() ) {
            sCloneConsensus * pcl = ptr(cl->summary.parentClID);
            if( (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->summary.first_bif_pos ) || pcl->summary.start > cl->summary.first_bif_pos) {
                ::printf("Problem with clone %" DEC " originating at position %" DEC " from clone %" DEC " with range [%" DEC ",%" DEC "]\n", cl->summary.clID, cl->summary.first_bif_pos, pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->getSummaryEnd() || (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->getSummaryEnd()) ) {
                ::printf("Problem with clone %" DEC " ending at position %" DEC " to clone %" DEC " with range [%" DEC ",%" DEC "]\n", cl->summary.clID, cl->getSummaryEnd(), pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
    }
#endif
    return 1;
}

idx sFrameConsensus::sortPositionsComparator(void * parameters, void * varr, idx i1, idx i2)
{
    sBioseqpopul::clonePosition * pA = ((sBioseqpopul::clonePosition *) varr) + i1;
    sBioseqpopul::clonePosition * pB = ((sBioseqpopul::clonePosition *) varr) + i2;
    return (pA->coverage() != pB->coverage()) ? ((pA->coverage() > pB->coverage()) ? -1 : 1) : 0;
}

bool sFrameConsensus::reorderClones() {
    sVec<idx> clIDs, sort_ind;
    sVec<sBioseqpopul::clonePosition> ret;

    for(idx i = sub_start; i <= sub_end; ++i) {
        if(progress_CallbackFunction && !(i%1000)){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }
        sort_ind.cut(0);clIDs.cut(0);ret.cut(0);
        getClonesPositionsOnLocation(i, clIDs, ret );
        sort_ind.resize(clIDs.dim());
        if( clIDs.dim() > 1 ) {
            sSort::sortSimpleCallback(sFrameConsensus::sortPositionsComparator, 0, ret.dim(), ret.ptr(),sort_ind.ptr());
            if( sort_ind.dim() ){
                remapSimilarities(clIDs,sort_ind,i);
                for( idx j = 0 ; j < sort_ind.dim() ; ++j ) {
                    ptr(clIDs[j])->getSubPos(i)->copy(ret.ptr(sort_ind[j]));
                }
            }
        }
    }
    return 1;
}

idx sFrameConsensus::sOutSimilarities(const char * flnm) {
    if(!flnm) return 0;
    sDic <idx> filt_similarities;
    idx * sim_v, sim_id_l;
    for( idx s = 0 ; s < similarities.dim() ; ++s ) {
        if(progress_CallbackFunction && !(s%1000) ){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }
        sim_v = similarities.ptr(s);
        const char * id_s = (const char *)similarities.id(s,&sim_id_l);

        if( *sim_v ) {
            *filt_similarities.set( (const void *)id_s,sim_id_l ) = *sim_v;
        }
    }

    sFile::remove(flnm);
    sFil smfl(flnm);
    if( smfl.ok() ) {
        filt_similarities.serialOut(smfl);
    }
    return smfl.length();
}

idx sFrameConsensus::createFrameMaps(const char * skp2gps, const char * gps2skp, const char * skp2gpsSpprt, const char * gps2skpSpprt) {

    sVec<idx> skp2gpsMap(skp2gps);
    sVec<idx> gps2skpMap(gps2skp);
    sVec<idx> skp2gpsSuppMap(skp2gpsSpprt);
    sVec<idx> gps2skpSuppMap(gps2skpSpprt);

    skp2gpsMap.add(dim());
    gps2skpMap.add(dim());
    skp2gpsSuppMap.add(dim());
    gps2skpSuppMap.add(dim());

    sCloneConsensus * cl;
    idx skp_pos = 0, skp_pos_supp = 0;

    for(idx i = 0 ; i < dim(); ++i) {
        if( progress_CallbackFunction ) {
            if( !progress_CallbackFunction(progress_CallbackParam, -1, -1, -1) )
                return 0;
        }
        cl = ptr(i);
        skp_pos = 0;
        skp_pos_supp = 0;

        skp2gpsMap[i] = skp2gpsMap.dim();
        gps2skpMap[i] = gps2skpMap.dim();
        skp2gpsSuppMap[i] = skp2gpsSuppMap.dim();
        gps2skpSuppMap[i] = gps2skpSuppMap.dim();

        for(idx j = 0; j < cl->seqCov.dim(); ++j) {
            if( !cl->seqCov.ptr(j)->isMultiGap() ) {
                *gps2skpMap.add() = skp_pos++;
                *skp2gpsMap.add() = j;
                if( cl->seqCov.ptr(j)->isDeletion() ) {
                    *gps2skpSuppMap.add() = -2;
                } else {
                    *gps2skpSuppMap.add() = skp_pos_supp++;
                    *skp2gpsSuppMap.add() = j;
                }
            }
            else {
                *gps2skpMap.add() = -1;
                *gps2skpSuppMap.add() = -1;
            }
        }
        if( skp2gpsMap[i] >= skp2gpsMap.dim() ) {
            skp2gpsMap[i] = -1;
            skp2gpsSuppMap[i] = -1;
        }

        if( gps2skpMap[i] >= gps2skpMap.dim() ) {
            gps2skpMap[i] = -1;
            gps2skpSuppMap[i] = -1;
        }
    }
    return gps2skpMap.dim()+skp2gpsMap.dim() - (2*dim());
}

idx sFrameConsensus::mergeAll() {
    sCloneConsensus * cl = 0;
    sCloneConsensus * parent =0 ;
    for(idx i = 0; i < dim(); ++i) {
        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }
        cl = ptr(i);
        if( !cl->summary.hasMerged() && cl->summary.hasParent() ) {
            parent = getParentOnLocation(i,cl->getSummaryEnd()-1);
            if( parent ) {
                cl->summary.mergeclID = parent->summary.clID;
            }
        }
    }
#ifdef _DEBUG
    ::printf("MERGE\n");
    for(idx i = 0 ; i < dim() ; ++i ) {
        if( cl->summary.clID < 0 || !cl->seqCov.dim() ){
            continue;
        }
        sCloneConsensus * cl = ptr(i);
        if( cl->summary.hasParent() ) {
            sCloneConsensus * pcl = ptr(cl->summary.parentClID);
            if( (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->summary.first_bif_pos ) || pcl->summary.start > cl->summary.first_bif_pos) {
                ::printf("Problem with clone %" DEC " originating at position %" DEC " from clone %" DEC " with range [%" DEC ",%" DEC "]", cl->summary.clID, cl->summary.first_bif_pos, pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->getSummaryEnd() || (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->getSummaryEnd()) ) {
                ::printf("Problem with clone %" DEC " ending at position %" DEC " to clone %" DEC " with range [%" DEC ",%" DEC "]", cl->summary.clID, cl->getSummaryEnd(), pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
    }
#endif
    return 1;
}

const char * sFrameConsensus::contructSimilaritiesID ( idx icl, idx pos, const char * reference ) {
    _simil_id_buf.printf(0,"%" DEC "-%" DEC "-%s", icl,pos,reference);
    return _simil_id_buf;
}

idx sFrameConsensus::remapSimilarities(sDic<idx> *sim, idx icl, idx pos, idx remapped_icl) {
    idx * cov = 0, remapped = 0;
    sStr t_s, ref_ind;
    for(idx i = 0 ; i < simil_size ; ++i ) {
        ref_ind.cut(0);ref_ind.addNum(i);
        t_s.printf(0,"%s",contructSimilaritiesID(icl, pos, ref_ind));
        cov = similarities.get( (const void *)t_s.ptr(),t_s.length() );
        if( cov ) {
            t_s.printf(0,"%s",contructSimilaritiesID(remapped_icl, pos, ref_ind));
            *sim->set( (const void *)t_s.ptr(),t_s.length() ) = *cov;
            ++remapped;
        }
    }
    return remapped;
}

void sFrameConsensus::remapSimilarities( sVec<idx> &vec_icl,sVec<idx> &vec_sort, idx pos) {
    assert(vec_icl.dim()==vec_sort.dim());
    sDic<idx> values;idx * v=0;
    sStr ref_ind;
    for( idx i = 0 ; i < vec_sort.dim() ; ++i ) {
        for(idx ir = 0 ; ir < simil_size ; ++ir ) {
            ref_ind.cut(0);ref_ind.addNum(ir);
            _simil_id_buf.printf(0,"%s",contructSimilaritiesID(vec_icl[vec_sort[i]], pos, ref_ind));
            v=similarities.get( (const void *)_simil_id_buf.ptr(), _simil_id_buf.length());
            if( v ) {
                _simil_id_buf.printf(0,"%s",contructSimilaritiesID(vec_icl[i],pos,ref_ind));
                *values.set((const void *)_simil_id_buf.ptr(),_simil_id_buf.length()) = *v;
            }
        }
    }
    idx * t = 0;
    for( idx i = 0 ; i < vec_sort.dim() ; ++i ) {
        for(idx ir = 0 ; ir < simil_size ; ++ir ) {
            ref_ind.cut(0);ref_ind.addNum(ir);
            _simil_id_buf.printf(0,"%s",contructSimilaritiesID(vec_icl[i],pos,ref_ind));
            v=values.get( (const void *)_simil_id_buf.ptr(),_simil_id_buf.length() );
            t=similarities.get( (const void *)_simil_id_buf.ptr(),_simil_id_buf.length() );

            if( t ) {
                if(v) {
                    *similarities.set((const void *)_simil_id_buf.ptr(),_simil_id_buf.length()) = *v;
                }
                else{
                    *similarities.set((const void *)_simil_id_buf.ptr(),_simil_id_buf.length()) = 0;
                }
            }
            else if(v){
                *similarities.set((const void *)_simil_id_buf.ptr(),_simil_id_buf.length()) = *v;
            }
        }
    }
}

idx sFrameConsensus::getCloneSupport(idx ic ){
    idx supportCnt=0;
    sCloneConsensus * cl = ptr(ic);
    sCloneConsensus * parentCl=0;
    if( !cl->summary.hasParent() ) {
        return 0;
    }
    parentCl =  ptr(cl->summary.parentClID);
    sBioseqpopul::clonePosition * clP, * parentP;
    for (idx i = cl->summary.start ; i < cl->summary.start + cl->seqCov.dim() ; ++i ) {
        clP = cl->getSubPos(i);
#ifdef _DEBUG
        if(!clP) {
            ::printf("FATAL ERROR unexpected invalid position\n");
        }
#endif
        if( i - parentCl->summary.start >= parentCl->seqCov.dim() ) {
            supportCnt += cl->seqCov.dim() + cl->summary.start - i;
            break;
        }
        parentP = parentCl->getSubPos(i);
        if( clP && parentP ) {
            if( clP->coverage() > 0 && parentP->coverage() > 0) {
                if ( clP->base() != parentP->base() ) {
                    ++supportCnt;
                }
            } else if( clP->coverage() || parentP->coverage() ) {
                ++supportCnt;
            }
        }
    }
    return supportCnt;
}

bool sFrameConsensus::areDifferent(sCloneConsensus * cl1, sCloneConsensus * cl2, idx start, idx end)
{
    if( start <= 0 ) {
        start = (cl1->summary.start < cl2->summary.start) ? cl2->summary.start : cl1->summary.start;
    }
    if( end <= 0 ) {
        idx end1 = cl1->getSummaryEnd();
        idx end2 = cl2->getSummaryEnd();
        end = (end1 > end2) ? end2 : end1;
    }
    if( start > end )
        return true;
    sBioseqpopul::clonePosition * p1 = 0, *p2 = 0;
    for(idx i1 = start, i2 = start; i1 < end && i2 < end; ++i1, ++i2) {
        p1 = cl1->getSubPos(i1);
        p2 = cl2->getSubPos(i2);
        while( p1 && p1->isAnyGap() && i1 < end ) {
            p1 = cl1->getSubPos(++i1);
        }
        if( i1 >= end )
            break;
        while( p2 && p2->isAnyGap() && i2 < end ) {
            p2 = cl2->getSubPos(++i2);
        }
        if( i2 >= end )
            break;
        if( p1 && p2 ) {
            if( p1->base() != p2->base() ) {
                return true;
            }
        } else if( !p1 && !p2 ) {
            continue;
        } else {
            return true;
        }
    }
    return false;
}

idx sFrameConsensus::cleanOutClones(sVec<idx> &cloneIndex)
{
    idx icl = 0;
    sFrameConsensus cleanClones(simil_size);
    cloneIndex.cut(0);
    cloneIndex.add(dim());
    cloneIndex.set(0);
    sCloneConsensus * addedclone = 0, *cl = 0, * t_cl = 0, * m_cl = 0, *cl2 = 0, * t_cl2 = 0, * m_cl2 = 0;


#ifdef _DEBUG
    ::printf("CLEAN OUT START\n");
    for(idx i = 0 ; i < dim() ; ++i ) {
        sCloneConsensus * cl = ptr(i);
        if( cl->summary.clID < 0 || !cl->seqCov.dim() ){
            continue;
        }
        if( cl->summary.hasParent() ) {
            sCloneConsensus * pcl = ptr(cl->summary.parentClID);
            if( (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->summary.start ) || pcl->summary.start > cl->summary.start) {
                ::printf("Problem with clone %" DEC " originating at position %" DEC " from clone %" DEC " with range [%" DEC ",%" DEC "]\n", cl->summary.clID, cl->summary.start, pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->getSummaryEnd() || (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->getSummaryEnd()) ) {
                ::printf("Problem with clone %" DEC " ending at position %" DEC " to clone %" DEC " with range [%" DEC ",%" DEC "]\n", cl->summary.clID, cl->getSummaryEnd(), pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
    }
#endif

    for(idx i = 0 ; i < dim() ; ++i ) {
        cl = ptr(i);

        if( !cl->isValid() )
            continue;
        idx updated_first_bif = getFirstMutation(cl->summary.clID,0,true);
        if(updated_first_bif >= cl->summary.start)
            cl->summary.first_bif_pos = updated_first_bif;
        t_cl = 0;
        if( cl->summary.hasParent() )
            t_cl = getParentOnLocation(i,cl->summary.first_bif_pos);
        m_cl = 0;
        if( cl->summary.hasMerged() )
            m_cl = getMergedOnLocation(i,cl->getSummaryEnd()-1);

        if(!m_cl && !t_cl) {
            m_cl=t_cl=cl;
        } else if(!m_cl) {
            m_cl=t_cl;
        }else if(!t_cl) {
            t_cl=m_cl;
        }

        idx start1 = cl->summary.first_bif_pos, end1=cl->getSummaryEnd();;
        for(idx j = 0 ; j < dim() ; ++j ) {
            if(i==j) continue;
            cl2 = ptr(j);
            if( !cl2->isValid() )
                continue;
            if( isLink(cl) && isLink(cl2) ) {
                if( !(cl->summary.parentClID == cl2->summary.parentClID && cl->summary.mergeclID == cl2->summary.mergeclID) )
                    continue;
            } else if ( isLink(cl) || isLink(cl2) )
                continue;

            t_cl2 = 0;
            if( cl2->summary.hasParent() )
                t_cl2 = getParentOnLocation(j,cl2->summary.first_bif_pos);

            m_cl2 = 0;
            if( cl2->summary.hasMerged() )
                m_cl2 = getMergedOnLocation(j,cl2->getSummaryEnd()-1);
            if(!m_cl2 && !t_cl2)
                continue;
            if(!m_cl2)m_cl2=t_cl2;
            else if(!t_cl2)t_cl2=m_cl2;
            idx start2 = cl2->summary.first_bif_pos, end2=cl2->getSummaryEnd();;
            if( !sOverlap(start2,end2,start1,end1) ) {
                continue;
            }
            if( !areDifferent(cl, cl2) ) {
                cl2->summary.mergeclID = cl->summary.clID;
                if( !isLink(cl2) ) {
                    if( mergePositions(cl, cl2, 0, 0, true) ) {
                        updateDependencies(cl2, cl);
                        cl2->summary.clID = -1;
                    }
                } else {
                    if( mergePositions(cl, cl2, cl2->summary.start + 3, 0, true) ) {
                        updateDependencies(cl2, cl);
                    }
                }
            }
        }
    }
    sVec<idx> offstarts(sMex::fSetZero);
    offstarts.resize(dim());
    for(idx i = 0; i < dim(); ++i) {
        cl = ptr(i);
        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }
        if( !cl->isValid() ){
            continue;
        }
        if( cl->summary.hasParent() ) {
            t_cl = getParentOnLocation(i,cl->summary.first_bif_pos);
            if(!t_cl)
                t_cl = cl;
            cl->summary.parentClID = t_cl->summary.clID;
        }
        if( cl->summary.hasMerged() ) {
            t_cl = getMergedOnLocation(i,cl->getSummaryEnd()-1);
            if(!t_cl)
                t_cl = cl;
            cl->summary.mergeclID = t_cl->summary.clID;
        }
        if( cl->summary.hasParent() && getFirstMutation(i,0,true) < 0) {
            t_cl = ptr(cl->summary.parentClID);
            if( t_cl->isValid() ) {
                mergePositions(t_cl, cl,0,0,true);
                t_cl->summary.end = t_cl->seqCov.dim() + t_cl->summary.start;
                updateDependencies(cl,t_cl);
            }
            cl->summary.clID = -1;
            continue;
        }
 
        idx t_end = cl->getLastSupportedPosition() + 1;
        if( t_end > cl->summary.start && t_end <= cl->summary.start + cl->seqCov.dim() ) {
            if(t_end - cl->summary.start < 2 ) t_end = cl->summary.start + 2;
            cl->seqCov.cut(t_end - cl->summary.start);
            cl->summary.end = t_end;
            fixMergedDependencies(cl);
        }
        idx t_start = cl->getFirstSupportedPosition();
        if( t_start > cl->summary.start) {
            *offstarts.ptrx(i) = t_start - cl->summary.start;
            cl->summary.start = t_start;
            fixParentDependencies(cl);
        }

        if( cl->summary.hasParent() ) {
            cl->summary.first_bif_pos = getFirstMutation(i,0,true);
            if( cl->summary.first_bif_pos < cl->summary.start || cl->summary.first_bif_pos > cl->getSummaryEnd() ) {
                cl->summary.first_bif_pos = cl->summary.start;
            }
        } else {
            cl->summary.first_bif_pos = cl->summary.start;
        }
    }
    for(idx i = 0 ; i < dim() ; ++i) {
        cl = ptr(i);

        if( !cl->isValid() ) {
            continue;
        }
        addedclone = cleanClones.add();
        cleanClones._frame_shifts.add()->copy(_frame_shifts.ptr(i));
        addedclone->stats = cl->stats;
        
        addedclone->summary = cl->summary;

        addedclone->seqCov.resize(cl->seqCov.dim() - offstarts[i]);

        addedclone->summary.end =  addedclone->summary.start + cl->seqCov.dim();

        for(idx j = offstarts[i]; j < addedclone->seqCov.dim(); ++j) {
            remapSimilarities(&cleanClones.similarities,i, addedclone->summary.start+j,icl);
            addedclone->seqCov.ptr(j - offstarts[i])->copy(cl->seqCov.ptr(j));
        }
        cloneIndex[addedclone->summary.clID] = icl;
        addedclone->summary.clID = icl++;
    }

    _vec.cut(0);
    similarities.empty();
    idx pLen = 0;
    for( idx i = 0 ; i < cleanClones.similarities.dim() ; ++i ) {
        const void * p_id= cleanClones.similarities.id(i, &pLen);
        *similarities.set(p_id,pLen) = *(idx *)cleanClones.similarities.get(p_id,pLen);
    }
    _vec.resize( cleanClones.dim() );
    for(idx i = 0; i < cleanClones.dim(); ++i) {
        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }
        addedclone = ptr(i);
        cl = cleanClones.ptr(i);

        if( cl->summary.mergeclID >= 0 && cl->summary.mergeclID < cloneIndex.dim() ) {
            cl->summary.mergeclID = cloneIndex[cl->summary.mergeclID];
        }
        if( cl->summary.parentClID >= 0 && cl->summary.parentClID < cloneIndex.dim() ) {
            cl->summary.parentClID = cloneIndex[cl->summary.parentClID];
        }
        addedclone->stats = cl->stats;
        addedclone->summary = cl->summary;

        addedclone->seqCov.resize(cl->seqCov.dim());

        for(idx j = 0; j < addedclone->seqCov.dim(); ++j) {
            addedclone->seqCov.ptr(j)->copy(cl->seqCov.ptr(j));
        }
    }

#ifdef _DEBUG
    ::printf("CLEAN OUT END\n");
    for(idx i = 0 ; i < dim() ; ++i ) {
        if( cl->summary.clID < 0 || !cl->seqCov.dim() ){
            continue;
        }
        sCloneConsensus * cl = ptr(i);
        if( cl->summary.hasParent() ) {
            sCloneConsensus * pcl = ptr(cl->summary.parentClID);
            if( (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->summary.start ) || pcl->summary.start > cl->summary.first_bif_pos) {
                ::printf("Problem with clone %" DEC " originating at position %" DEC " from clone %" DEC " with range [%" DEC ",%" DEC "]\n", cl->summary.clID, cl->summary.start, pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->getSummaryEnd() || (pcl->getSummaryEnd() >= 0 && pcl->getSummaryEnd() < cl->getSummaryEnd()) ) {
                ::printf("Problem with clone %" DEC " ending at position %" DEC " to clone %" DEC " with range [%" DEC ",%" DEC "]\n", cl->summary.clID, cl->getSummaryEnd(), pcl->summary.clID, pcl->summary.start, pcl->getSummaryEnd());
            }
        }
    }
#endif
    return 1;
}

void sFrameConsensus::getCloneStats(sBioseqpopul::cloneStats * clSum)
{
    clSum->avCov = 0;
    clSum->maxCov = 0;
    clSum->size = 0;
    clSum->support = simil_size;
    for(idx i = 0; i < dim(); ++i) {
        sCloneConsensus *cl = ptr(i);
        if( cl->summary.clID < 0 ) {

#ifdef _DEBUG
            ::printf("NEGATIVE CLONE EVEN AFTER CLEANING THEM\n");
#endif
            continue;
        }
        cl->summary.end = cl->summary.start + cl->seqCov.dim();
        if(sub_end < cl->summary.end) {
            sub_end = cl->summary.end;
        }
        cl->stats.maxCov = 0;
        cl->stats.avCov = 0;
        cl->stats.size = cl->summary.end - cl->summary.start;
        for(idx j = 0; j < cl->seqCov.dim(); ++j) {
            cl->stats.avCov += (real) cl->seqCov[j].coverage() / cl->seqCov.dim();
            if( cl->stats.maxCov < (cl->seqCov[j].coverage()) )
                cl->stats.maxCov = cl->seqCov[j].coverage();
        }
        clSum->avCov += cl->stats.avCov / dim();
        if( clSum->maxCov < cl->stats.maxCov )
            clSum->maxCov = cl->stats.maxCov;
        if( clSum->size < cl->summary.end )
            clSum->size = cl->summary.end;
    }
}


bool sFrameConsensus::setConsensusStat(sBioseqpopul::cloneStats &clSum)
{
    bool skipGaps=true;
    idx sq_size = 0;
    sCloneConsensus * cl, * parentCl=0, * mergeCl = 0, * orCl = 0;
    sVec<sBioseqpopul::clonePosition> * tSC;

    for( idx i = 0 ; i < dim() ; ++i ){
        cl = ptr(i);

        sBioseqpopul::clonePosition * cp = 0;
        parentCl = 0; mergeCl = 0;
        cl->stats.stacked_size = 0;
        cl->stats.size = 0;
        if( cl->summary.hasParent() ) parentCl =  ptr(cl->summary.parentClID);
        if( cl->summary.hasMerged() ) mergeCl =  ptr(cl->summary.mergeclID);
        tSC = &cl->seqCov;
        for( idx pos = 0; pos < tSC->dim() ; ++pos ) {
            cp = tSC->ptr(pos);
            idx t_b = cp->base();
            if( !( (t_b > 3 || !cp->coverage()) && skipGaps) ){
                ++cl->stats.stacked_size;
                ++cl->stats.size;
            }
            if( cp->isDeletion() )
                ++cl->stats.supportedGapsCnt;
            if( cp->isMultiGap() )
                ++cl->stats.multiGapsCnt;
        }

        orCl = cl;
        while( parentCl ){
            tSC = &parentCl->seqCov;
            sq_size = tSC->dim();
            if(sq_size > cl->summary.start-parentCl->summary.start) sq_size = cl->summary.start-parentCl->summary.start;
            else break;

            cl=parentCl;
            parentCl = 0;
            if( cl->summary.hasParent() ) parentCl = ptr(cl->summary.parentClID);
            sBioseqpopul::clonePosition * tcp = 0;
            for( idx pos = 0; pos < sq_size ; ++pos ) {
                tcp = tSC->ptr(pos);
                idx t_b = tcp->base();
                if( !( (t_b > 3 || !tcp->coverage()) && skipGaps) ){
                    ++orCl->stats.stacked_size;
                }
            }
        }
        cl = orCl;
        while( mergeCl ){
            tSC = &mergeCl->seqCov;
            sq_size = tSC->dim();
            idx pos =  cl->summary.end - mergeCl->summary.start;

            cl=mergeCl;
            mergeCl = 0;
            if( cl->summary.hasMerged() ) mergeCl = ptr(cl->summary.mergeclID);

            sBioseqpopul::clonePosition * mcp = 0;
            for( ; pos < sq_size ; ++pos ) {
                mcp = tSC->ptr(pos);
                char t_b = mcp->base();
                if( !( (t_b > 3 || !mcp->coverage()) && skipGaps) ){
                    ++orCl->stats.stacked_size;
                }
            }
        }
    }
    return true;
}
