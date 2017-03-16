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

//idx sBioseqpopul::referenceSize=0;

/*-------------------
 * sFrame
 --------------------*/
void sFrame::move(idx steps)
{
    _coverage.move(steps);
    sClone * wk = getMainClone();
    if( !wk )
        return;
    resetAvCoverage();


    for(idx k = 0; k < cnt(); ++k) {
        wk = ptr(k);
        if( wk->alive() )
            wk->move(steps);
    }
}

void sFrame::setCalls(void)
{
    sClone * w = 0;
    for(idx i = 0; i < cnt(); ++i) {
        w = ptr(i);
        if( w && !w->dead ) {
            w->setCalls();
        }
    }
}

void sFrame::resize(idx addSize)
{
    for(idx i = 0; i < cnt(); ++i) {
        sClone * w = ptr(i);
        if( !w->alive() )
            continue;
        w->strech(addSize);
    }
    _coverage.stretch(addSize);
}

idx sFrame::mergeClones(real cutoff)
{
    for(idx k = 0; k < cnt(); ++k) {
//        real av_cov = avCoverage();
        sClone * w1 = ptr(k);
//        idx matSize=_frame.frameSize();
        if( w1->alive() ) {
            for(idx l = k + 1; l < cnt(); ++l) {
                idx Merge = 0, sup_Merge = 0;
                sClone * w2 = ptr(l);
                if( w2->alive()) {
                    idx st = w2->getrawfirst(), ed = w2->getTailLast() - 1;//, lastdif = st - 1;
                    bool mergeB = false;
                    sBioseqpopul::position * p1, *p2; //=w1->Matrix.ptr(st%matSize), *p2=w2->Matrix.ptr(st%matSize);
                        /*-----------------------------------------------------------------------------------------------------------------------
                         *              if the window is newly created it might have non covered position from the start pos  of the subject
                         *              up to the first read that touches the bifurcation position.
                         *-----------------------------------------------------------------------------------------------------------------------*/
                    if( w1->curSubPOS == w1->startSubPOS )
                        st = w1->getfirstNonZero(&st);
                    if( w2->curSubPOS == w2->startSubPOS )
                        st = w2->getfirstNonZero(&st);
                    idx i = st;
                    ed = w1->getlastNonZero(&st, &ed);
                    ed = w2->getlastNonZero(&st, &ed);
                    idx overlapSupported = 0;
                    for(i = ed; i >= st; --i) {
                        p1 = w1->pos(i);
                        p2 = w2->pos(i);

                        if( isClonalPositionFuzzy(w1,i,cutoff) || isClonalPositionFuzzy(w2,i,cutoff)) {
                            ++Merge;

                        } else {
                            ++overlapSupported;
                            if( !p1->isDiff(p2,gap_thrshld) ) {
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

                        if( Merge >= w1->length() || Merge >= w2->length() ) {
                            --i;
                            mergeB = true;
                            break;
                        }
                    }
                    if( sup_Merge >= overlapSupported && overlapSupported >= w1->length() ) {
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
                        break;
                    }
                }
            }
        }
    }
    for(idx i = cnt() - 1; i >= 0; --i) {
        sClone * w = ptr(i);
        if( w->merging && w ) {
            sClone * wOrig = ptr(w->mergingCl);
            while( wOrig->merging && wOrig->mergeMatPOS < w->mergeMatPOS ) {
                w->mergeMatPOS = wOrig->mergeMatPOS;
                wOrig = ptr(wOrig->mergingCl);
            }

            //If merging to other than parent
            if( w->hasParent() && w->parentClID!=wOrig->clID && w->subPos(w->mergeMatPOS) < w->bifurcation_pos ) {
                w->bifurcation_pos = w->subPos(w->mergeMatPOS) - 1;
            }

            for(idx j = i ; j < cnt() ; ++j ) {
                sClone * chW = ptr(j);
                if( chW->parentClID == w->clID && chW->alive() && chW->bifurcation_pos > w->subPos(w->mergeMatPOS) ) {
                    chW->parentClID = wOrig->clID;
                }
            }

            w->mergeTo(wOrig, w->mergeMatPOS);
        }
    }
    idx cntMerged = 0;
    for(idx ic=0;ic<cnt();++ic) {
        if(ptr(ic)->merged && !ptr(ic)->dead){
            ++cntMerged;
#ifdef _DEBUG
        ::printf("Merged clone:%"DEC" to : %"DEC" in position: %"DEC"\n", ptr(ic)->clID, ptr(ic)->mergedToCl,ptr(ic)->mergeSubPOS);
#endif
        }
    }
#ifdef _DEBUG
    if(cntMerged)::printf("Newly Merged clones:%3"DEC"\n",cntMerged);
#endif
    return cntMerged;
}

idx sFrame::killClones(real cutoff, bool keepOneAlive)
{
    sClone*w;
    idx cntKilled = 0;
    for(idx i = cnt()-1; i >= 0; --i) {
        w = ptr(i);
        if( !w->dead && !w->merged ) {
            idx st = w->getrawfirst(), ed = w->getTailLast();
            st = w->getfirstNonZero(&st, &ed);
            idx j = 0;
            idx last_covered = st;
            for(j = ed-1; j >= st; --j) {
                if( !isClonalPositionFuzzy(w,j,cutoff) ){
                    last_covered = j;
                    break;
                }
            }
            if( last_covered - st < (win_size()) && ( cntAlive() > 1 || !keepOneAlive ) ) {
                w->killed = true;
                w->deathSubPOS = j;
#ifdef _DEBUG
                ::printf("Killed clone with id:%"DEC"\nBecause of covered size (%"DEC") being less than the window size(%"DEC")\n", w->clID, j - st + 1, win_size());
#endif
                ++cntKilled;
            }
        }
    }
#ifdef _DEBUG
    for(idx ic=0;ic<cnt();++ic) {
        if(ptr(ic)->killed && !ptr(ic)->dead)
        ::printf("Killed clone:%"DEC" in position: %"DEC"\n", ptr(ic)->clID,ptr(ic)->deathSubPOS);
    }
    if(cntKilled)::printf("Newly killed clones:%3"DEC"\n",cntKilled);
#endif
    return cntKilled;
}

idx sFrame::computeStep(real cutoff, idx minOverlap)
{
    sClone * w = 0;
    idx minI = win_size() - minOverlap, step = 0;
//    real av_cov = avCoverage();
    for(idx j = 0; j < cnt(); ++j) {
        w = ptr(j);
        if( !w )
            continue;
        if( !w->dead && !w->killed && !w->merged ) {
            idx st = w->getTailfirst(), ed = w->getTailLast();
            st = w->getfirstNonZero(&st, &ed);

            for(idx i = ed - 1; i >= st; --i) {
//                sBioseqpopul::position * p = w->pos(i);
                if( !isClonalPositionFuzzy(w,i,cutoff) ) {
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
//    step = ;
//    if(step < minI/2 )step = minI/2;
    return step > minI ? step : minI;
}

idx sFrame::getFingPrints(sVec<sBioseqpopul::clonePat> * patterns, idx * relativeTo, real fuzzy_coverages_threshold)
{
    //scan tails to get fingerprints of each clone
    resetAvCoverage();
    real AVcov = avCoverage();
    idx oo = getMainCloneIdx();
    sClone * w0 = ptr(oo);
    if( !w0 )
        return 0;

    if( relativeTo )
        *relativeTo = oo;

    if( !AVcov ) {
        return 0;
    }

    for(idx k = 0; k < cnt(); ++k) {
        sClone * w1 = ptr(k);
        if( w0->clID == w1->clID )
            continue;
        if( w1 && w1->alive() ) {
            idx st = w1->getrawfirst(), ed = w1->getTailLast();
            sBioseqpopul::clonePat * p = patterns->add();
            p->curpos = -1;
            p->clID = w1->clID;

            idx i = st;
            //less or equal becuase the first read might start before the bifurcation position
            i = w1->getfirstNonZero(&i, &ed);
            i = w0->getfirstNonZero(&i, &ed);

            ed = w1->getlastNonZero(&i, &ed);
            ed = w0->getlastNonZero(&i, &ed);

            for(; i < ed; ++i) {
                sBioseqpopul::position * p1 = w1->pos(i);
                sBioseqpopul::position * p0 = w0->pos(i);
                if( p0->isDiff(p1,gap_thrshld) ) {
                    if(!isClonalPositionFuzzy(w0,i,fuzzy_coverages_threshold) && !isClonalPositionFuzzy(w1,i,fuzzy_coverages_threshold)) { //both coverage have to be above the fuzzy threshold
                        sBioseqpopul::points t;
                        t.base = p1->basecall;
                        t.position = i;
                        *(p->pattern.add()) = t;
                   }
                }
            }
        }
    }

    return 1;
}

idx sFrame::getSingleFingPrint(sBioseqpopul::clonePat * p, idx parentClID, idx childID, real fuzzy_coverages_threshold, idx * start, idx * end)
{
    if( parentClID < 0 || childID < 0 )
        return 0;

    real av_cov = avCoverage();
    if( !av_cov )
        return 0;
    sClone * w0 = ptr(parentClID);
    sClone* w1 = ptr(childID);

    if( w1->merged || w1->dead || w1->killed )
        return 0;

    idx st = start ? *start : w1->getrawfirst(), ed = end ? *end : w1->getTailLast();

    p->curpos = -1;
    p->clID = childID;
    p->pattern.cut(0);

    idx i = st;

    if( w1->curSubPOS == w1->startSubPOS )
        i = w1->getfirstNonZero(&i, &ed);
    if( w0->curSubPOS == w1->startSubPOS )
        i = w0->getfirstNonZero(&i, &ed);

    ed = w1->getlastNonZero(&i, &ed);
    ed = w0->getlastNonZero(&i, &ed);
//    idx w1cov = 0, w0cov = 0;
//    real covCoef = 0;
    for(; i < ed; ++i) {
        sBioseqpopul::position * p1 = w1->pos(i);
        sBioseqpopul::position * p0 = w0->pos(i);

        p1->setbasecall();
        p0->setbasecall();
        if( p0->isDiff( p1, gap_thrshld) ) {
//            w1cov = p1->getCoverage();
//            w0cov = p0->getCoverage();
            if( !isClonalPositionFuzzy(w0,i,fuzzy_coverages_threshold) && !isClonalPositionFuzzy(w1,i,fuzzy_coverages_threshold) ) { //both coverage have to be above the fuzzy threshold
                sBioseqpopul::points * t = p->pattern.add();
                t->base = p1->basecall;
                t->position = i;
            }
        }
    }

    if( (p->pattern.dim()) <= 1 ) {
        w1->mergeTo(w0, w1->matPos(w1->getrawfirst()) );

        w1->mergeSubPOS = -10;
        w1->mergeMatPOS = w1->getrawfirst();
        return 0;
    }
    return 1;
}

static sStr st_buf;

const char * sFrame::diff(idx cl1, idx cl2)
{
    sClone * w1 = ptr(cl1);
    sClone * w2 = ptr(cl2);
    if( !w1 || !w2 ) {
        return 0;
    }
    sStr * out = &st_buf;
    idx cnt = 0, cov_base1 = 0, cov_base2 = 0;
    out->printf(0, "| # |  sub  |%7"DEC"|  cov  |%7"DEC"|  cov  |  dif  |\n", w1->clID, w2->clID);
    out->printf("  |---+-------+-------+-------+-------+-------+-------|\n");
    for(idx i = 0; i < w1->dim(); ++i) {
        sBioseqpopul::position * p1 = w1->ptr(i);
        p1->setbasecall();
        sBioseqpopul::position * p2 = w2->ptr(i);
        p2->setbasecall();
        sStr flag;
        flag.printf("   ");
        if( p1->basecall != p2->basecall && p1->basecall >= 0 && p2->basecall >= 0 ) {
            flag.printf(0, "***");
            ++cnt;
        }
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
        out->printf("|%3"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7s|\n", i, w1->subPos(i), (idx)p1->basecall, cov_base1, (idx)p2->basecall, cov_base2, flag.ptr());
    }
    out->printf("|-----------------------------------+-Tot-diff--%3"DEC"-|\n", cnt);
    return out->ptr(0);
}

idx sClone::cmpClones(void * param, void * arr, idx i1, idx i2)
{
    sClone * clones = static_cast<sClone*>(arr);
    if(clones[i1].dead || clones[i2].dead) {
        return -1;
    }
    idx iv1 = clones[i1].getfirstNonZero();
    idx iv2 = clones[i2].getfirstNonZero();

    return  iv1 - iv2;
}


idx sFrame::extractCons(sFrameConsensus * cloneCons, idx POS, bool toEnd)
{
    idx cntNewClones = cnt() - cloneCons->dim();
    if( cntNewClones > 0 ) {
        sVec<idx> inds(sMex::fExactSize), indsTmp;
        inds.add(cntNewClones);
        sClone * newClones = ptr(cloneCons->dim());
        sSort::sortSimpleCallback(sClone::cmpClones, this, cntNewClones, newClones, inds.ptr());

        reorder(inds.ptr(),cloneCons->dim(),cntNewClones);
    }
    for(idx k = 0; k < cnt(); ++k) {
        sClone * w = ptr(k);
        idx extrst = w->extrPos;
        if( !w->dead ) {
            idx extred = (toEnd ? w->getTailLast() : w->getrawfirst() + POS), i = extrst;

            if( w->merged || w->killed ) {
                extred = w->getTailLast();
                if( w->merged && w->mergeSubPOS < 0 )
                    extred = extrst;

                w->dead = true;
            }

            sCloneConsensus * t = cloneCons->ptrx(w->clID);

            if( !(t->seqCov.dim()) && extrst != extred ) {
                i = w->getfirstNonZero(&i, &extred);
                t->summary.clID = w->clID;
                t->summary.start = i;// w->startSubPOS;// + i - extrst;
                t->summary.first_bif_pos = w->bifurcation_pos;
                t->summary.end = -1;
                t->summary.parentClID = w->parentClID;
                t->summary.mergeclID = (w->merged ? w->mergedToCl : w->clID);
                if( t->summary.start < cloneCons->sub_start ){
                    cloneCons->sub_start = t->summary.start;
                }
                if( w->bifurcation_bayes != sNotIdx ) {
                    t->stats.bifurcation_bayes = w->bifurcation_bayes;
                }
                if( w->bifurcation_pvalue != sNotIdx ) {
                    t->stats.bifurcation_pvalue = w->bifurcation_pvalue;

                }
            }
            if( w->merged && w->mergeSubPOS < extred )
                extred = w->mergeSubPOS;
            sBioseqpopul::position * p = 0;
            sBioseqpopul::clonePosition * cp = 0;
            for(; i < extred; ++i) {
                p = w->pos(i);
                if(w->dead)*_coverage.pos(i - w->curSubPOS)-=p->getSupportedCoverage();
                cp = t->seqCov.ptrx( i - t->summary.start );
                if( p->isGap(gap_thrshld) /*|| w->isGap(i,gap_thrshld)*/ ) {
                    cp->setGap(p);
                }
                else {
                    cp->setPosition(p);
                }
                cloneCons->getSimilarities(t->summary.clID,i,p);
            }
            t->stats.support = w->supportCnt;
            if( w->merged || w->killed )
                t->summary.end = i;

            if( w->dead ) {
                w->destroy();
            }

        }
        w->extrPos += POS;
    }
    return 1;
}

idx sFrame::getAlBoundaries(sBioal * bioal, idx * alList, idx posStart, idx posEnd, idx &iAlStart, idx &iAlEnd, idx alCnt/*,sVec<sBioseqAlignment::Al *> * alSub,sVec< sVec<idx> > * alSubLUT*/)
{
    sBioseqAlignment::Al * hdrA = bioal->getAl(alList ? alList[iAlStart] : iAlStart);
    idx compValue = hdrA->subStart();

    compValue += bioal->getMatch(alList ? alList[iAlStart] : iAlStart)[0];
    while( compValue < posStart ) {
        if( iAlStart == alCnt )
            break;
        hdrA = bioal->getAl(alList ? alList[++iAlStart] : iAlStart);
        compValue = hdrA->subStart();
        compValue += bioal->getMatch(alList ? alList[iAlStart] : iAlStart)[0];
    }
    if( iAlEnd == alCnt ) {
        return 1;
    }
    hdrA = bioal->getAl(alList ? alList[iAlEnd] : iAlEnd);
    compValue = hdrA->subStart();
    compValue += bioal->getMatch(alList ? alList[iAlEnd] : iAlEnd)[0];
    while( compValue < posEnd ) {
//        ++iAlEnd;
        if( ++iAlEnd == alCnt ) {
            return 1;
        }
        hdrA = bioal->getAl(alList ? alList[iAlEnd] : iAlEnd);
        compValue = hdrA->subStart();
        compValue += bioal->getMatch(alList ? alList[iAlEnd] : iAlEnd)[0];
        if( iAlEnd == alCnt )
            break;
    }
    return (iAlEnd == alCnt);
}

sClone * sFrame::branch(idx prevID, idx setbase, idx t_offset)
{
    sClone * cl = add();
    sClone * w = ptr(prevID);
    w->lastBif.position = w->curSubPOS + t_offset;
    w->lastBif.base = setbase;
    w->reset(t_offset - w->offset);

    cl->init(w, cnt() - 1);
    return cl;
}

/*-------------------
 * sClone
 --------------------*/
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

    for(idx l = 0; l < 2 * addSize; ++l) {
        sBioseqpopul::position * p = add();
        p->referenceSimil.resize(ref_cnt);
        p->referenceSimil.set(0);
    }

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
    mergeSubPOS = subPos(matPos);

    for(idx i = mergeSubPOS; i < ed; ++i) {
        dest->pos(i)->transfer(*pos(i));
    }
}

const char * sClone::print(real cutoff, idx minCov, sStr * out)
{
    if( !out ) {
        out = &st_buf;
        out->cut(0);
    }
    out->printf("|Clone__%7"DEC"_____________________________________________________________|\n", clID);
    out->printf("| # |  sub  |   A   |   C   |   G   |   T   |  Del  |  In   |  SNV  | flags |\n");
    out->printf("|---+-------+-------+-------+-------+-------+-------+-------+-------+-------|\n");
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
        out->printf("|%3"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7"DEC"|%7s|%7s|\n", i, subPos(i), p->base[0], p->base[1], p->base[2], p->base[3], p->base[4], p->base[5], p->isBifurcatingPosition(cutoff,minCov) ? "  ***  " : "", flag.ptr());
    }
    out->printf("|---------------------------------------------------------------------------|\n");
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
        out->printf("|%3"DEC"|%7"DEC"|%7"DEC"|%7s|\n", i, getMainClone()->subPos(i), _coverage[i], flag.ptr());
    }
    out->printf("|---------------------------|\n");
    return out->ptr();
}


idx sFrame::getFirstDiff(sClone * w1, sClone * w2,idx start, idx end, real cutoff)
{
    for( idx i = start; i < end; ++i ) {
        sBioseqpopul::position * p1 = w1->pos(i);
        sBioseqpopul::position * p2 = w2->pos(i);
        if( p1->isDiff(p2, gap_thrshld) && !this->isClonalPositionFuzzy(w1, i, cutoff) && !this->isClonalPositionFuzzy(w1, i, cutoff) )
            return i;
    }
    return -1;
}


idx sFrame::getLastDiff(sClone * w1, sClone * w2,idx start, idx end, real cutoff)
{
    for(idx i = end; i >= start; --i) {
        sBioseqpopul::position * p1 = w1->pos(i);
        sBioseqpopul::position * p2 = w2->pos(i);
        if( p1->isDiff(p2, gap_thrshld) && !this->isClonalPositionFuzzy(w1, i, cutoff) && !this->isClonalPositionFuzzy(w1, i, cutoff) )
            return i;
    }
    return -1;
}

/*-------------------
 * sBioseqpopul
 --------------------*/
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



/*-------------------
 * sCloneConsensus
 --------------------*/
//void sCloneConsensus::readClone(sClone * cl, real avCoverage, bool toEnd)
//{
//    if( cl->dead ) {
//        return;
//    }
//    idx extrst = cl->extrPos;
//    idx extred = cl->getlast();
//
//    cl->extrPos = extred;
//    if( toEnd )
//        extred = cl->getTailLast();
//    idx notfirst = 0;
//    idx i = extrst;
//    while( !(cl->pos(i)->getCoverage()) ) {
//        ++i;
//        if( i > cl->getTailLast() + cl->dim() ) {
//            cl->dead = true;
//            continue;
//        }
//    }
//    if( seqCov.dim() ) {
//        summary.clID = cl->clID;
//        summary.start = cl->curSubPOS + i - extrst;
//        summary.end = -1;
//        summary.parentClID = cl->parentClID;
//        summary.mergeclID = cl->clID;
//    }
//
//    for(; i < extred; ++i) {
//        sBioseqpopul::position * p = cl->pos(i);
//        if( toEnd && !p->getCoverage() )
//            break;
//        if( p->getCoverage() < 0.001 * avCoverage ) {
//            cl->dead = true;
//            summary.end = i - extrst;
//            summary.mergeclID = -1;
//            cl->clID = -1;
//            break;
//        }
//
//        if( cl->merged && i > cl->mergeMatPOS ) {
//            if( !notfirst ) {
//                idx endCl = cl->mergeSubPOS;
//                summary.end = endCl;
//                summary.mergeclID = cl->mergedToCl;
//                ++notfirst;
//            }
//        } else {
//            p->setbasecall();
//            sBioseqpopul::clonePosition * cp = seqCov.add();
//            cp->setPosition(p);
//        }
//
//    }
//
//    //check if clone dies exactly after the length of the queries
//    if( !toEnd && !cl->dead && !cl->merged && cl->pos(++i)->getCoverage() < 0.001 * avCoverage ) {
//        cl->dead = true;
//        summary.end = i - extrst;
//        summary.mergeclID = -1;
//        cl->clID = -1;
//    }
//    --i;
//    //keep track of the last extracted position relative to the subject
//    extrSubPOS += extred - extrst;
//}

idx sCloneConsensus::getSubPosBase(idx pos)
{
    sBioseqpopul::clonePosition * p = getSubPos(pos);
    if(!p) {
        return -1;
    }
    return p->base();
}
idx sCloneConsensus::getSubPosCoverage(idx pos)
{
    sBioseqpopul::clonePosition * p = getSubPos(pos);
    if(!p) {
        return -1;
    }
    return p->coverage();
}
sBioseqpopul::clonePosition * sCloneConsensus::getSubPos(idx pos)
{
    idx ipos=pos - summary.start;
    if(ipos < 0 || ipos >= seqCov.dim() ) {
        ::printf("Fatal ERROR: requesting position %"DEC" in a vector size %"DEC"\n",ipos,seqCov.dim());
        return 0;
    }
    return seqCov.ptr(ipos);
}

/*-------------------
 * sFrameConsensus
 --------------------*/
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
    idx simils = 0;
//#ifdef _DEBUG
//    idx tot_sim = 0;
//#endif
    for( idx i = 0 ; i < p->referenceSimil.dim() ; ++i ) {
        if( p->referenceSimil[i] > 0 ) {
            ++simils;
            contructSimilaritiesID(cloneID,framePos,i);
            *similarities.set( _simil_id_buf.ptr() ,_simil_id_buf.length() ) = p->referenceSimil[i];
//#ifdef _DEBUG
//            tot_sim += p->referenceSimil[i];
//#endif
        }
    }
//#ifdef _DEBUG
//    if( tot_sim != p->getSupportedCoverage() ) {
//        ::printf("BAD SIMILARITIES ON extractCONSENSUS: clone :%"DEC" sim_tot = %"DEC" vs coverage=%"DEC"\n",cloneID, tot_sim, p->getSupportedCoverage() );
//    }
//#endif
    return simils;
}

sCloneConsensus * sFrameConsensus::getParentOnLocation(idx iCl, idx pos)
{
    sCloneConsensus * cl = ptr(iCl);
    if( !cl->summary.hasParent() || cl->summary.parentClID>=dim()) {
        return 0;
    }
    cl = ptr(cl->summary.parentClID);
    if( sOverlap(pos,pos,cl->summary.start ,cl->summary.end - 1) ) { // -1 because end is not inclusive
        return cl;
    } else if ( pos < cl->summary.start && cl->summary.hasParent() ){
        return getParentOnLocation(cl->summary.clID,pos);
    } else if( pos >= cl->summary.end && cl->summary.hasMerged() ) {
        return getMergedOnLocation(cl->summary.clID,pos);
    }
    return 0;
}

idx sFrameConsensus::getFirstMutation(idx iCl, idx pos /*= 0*/ )
{
    sCloneConsensus * cl = ptr(iCl);
    if( !cl->summary.hasParent() || cl->summary.parentClID>=dim()) {
        return -1;
    }
    sCloneConsensus * parent = getParentOnLocation(iCl,cl->summary.start);
    if(!parent) {
        return -1;
    }
    sBioseqpopul::clonePosition * p1 = 0, * p2 = 0;
    idx base1 = 0, base2 = 0;
    if( pos ) {
        pos -= cl->summary.start;
    } else {
        pos = cl->getFirstSupportedPosition() - cl->summary.start;
    }
    for( idx i = pos ; i < cl->seqCov.dim() ; ++i ) {
        if(i + cl->summary.start < parent->summary.start)
            continue;
        if( i + cl->summary.start > parent->summary.end ) {
            break;
        }
        p1 = cl->getSubPos( i + cl->summary.start);
        if( !p1 ) {
            continue;
        }
        p2 = parent->getSubPos( i + cl->summary.start);
        if( !p2 ) {
            continue;
        }
        base1 = p1->base();
        base2 = p2->base();
        if( base1 != base2 && base1 >= 0 && base2 >= 0) {
            return i+cl->summary.start;
        }
    }
    return -1;
}

sCloneConsensus * sFrameConsensus::getMergedOnLocation(idx iCl, idx pos)
{
    sCloneConsensus * cl = ptr(iCl);
    if( !cl->summary.hasMerged() || cl->summary.mergeclID>=dim()) {
        return 0;
    }
    cl = ptr(cl->summary.mergeclID);
    if( sOverlap(pos,pos,cl->summary.start ,cl->summary.end) ) {
        return cl;
    } else if ( pos < cl->summary.start && cl->summary.hasParent() ){
        return getParentOnLocation(cl->summary.clID,pos);
    } else if( pos >= cl->summary.end && cl->summary.hasMerged() ) {
        return getMergedOnLocation(cl->summary.mergeclID,pos);
    }
    return 0;
}

bool sFrameConsensus::mergePositions(sCloneConsensus * dstCl, sCloneConsensus * srcCl, idx start /*= 0*/, idx cnt /*= 0*/)
{
    if( !start ) {
        start = srcCl->summary.start;
    }
    if( !cnt ) {
        cnt = srcCl->seqCov.dim();
    }
    idx frmEnd = start + cnt;
    if( sOverlap(dstCl->summary.start, dstCl->summary.start+dstCl->seqCov.dim()-1,start,start+cnt-1) ) {
        if( start < dstCl->summary.start ) {
            dstCl->seqCov.insert(0,dstCl->summary.start -start);
            dstCl->summary.start = start;
        }
        idx dimDif = start+cnt - (dstCl->summary.start + dstCl->seqCov.dim());
        if( dimDif > 0 ) {
            dstCl->seqCov.resize( dimDif );
        }
        for (idx pos = start ; pos < frmEnd ; ++pos ){
            dstCl->seqCov.ptrx(pos - dstCl->summary.start)->add( srcCl->seqCov.ptr(pos - srcCl->summary.start) );
            for (idx i = 0 ; i < simil_size ; ++i ) {
                idx * sv = similarities.get( contructSimilaritiesID(srcCl->summary.clID, pos, i) );
                if( sv ) {
                    idx svv = *sv;
                    idx * dv = similarities.get( contructSimilaritiesID(dstCl->summary.clID, pos, i) );
                    if( dv ) {
                        *dv += svv;
                    } else {
                        dv = similarities.set( contructSimilaritiesID(dstCl->summary.clID, pos, i) );
                        if(dv)
                            *dv = svv;
                        else {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 1;
}

idx sFrameConsensus::trimClones() {
    sCloneConsensus * cl = 0;
    idx start_bif_dif = 0, p = 0;
    for(idx i = 0; i < dim(); ++i) {
        if(progress_CallbackFunction){
            if( !progress_CallbackFunction(progress_CallbackParam,-1,-1,-1) )
                return 0;
        }
        cl = ptr(i);
        start_bif_dif = cl->summary.first_bif_pos - cl->summary.start;
        if( start_bif_dif > 1 && start_bif_dif < cl->seqCov.dim() ){ //do not cut exactly on the mutation
            --start_bif_dif;//do not cut exactly on the mutation
            if( cl->summary.hasParent() ) {
                sCloneConsensus * pCl = ptr(cl->summary.parentClID );
                for ( p = start_bif_dif + cl->summary.start - 1 ; p >= cl->summary.start ; --p )
                {
                    if( pCl->summary.start > p ) {
                        pCl = getParentOnLocation(pCl->summary.clID,p);
                        if(!pCl) {
                            start_bif_dif = p - cl->summary.start -1;
                            break;
                        }
                    }
                    if(!mergePositions(pCl,cl,p,1)) {
                        return 0;
                    }
                }
            }

            cl->stats.size -= start_bif_dif;
            cl->summary.start += start_bif_dif;
            fixParentDependencies(cl);
            cl->seqCov.del(0,start_bif_dif);
        }
    }
#ifdef _DEBUG
    for(idx i = 0 ; i < dim() ; ++i ) {
        if( cl->summary.clID < 0 || !cl->seqCov.dim() ){
            continue;
        }
        sCloneConsensus * cl = ptr(i);
        if( cl->summary.hasParent() ) {
            sCloneConsensus * pcl = ptr(cl->summary.parentClID);
            if( (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.first_bif_pos ) || pcl->summary.start > cl->summary.first_bif_pos) {
                ::printf("Problem with clone %"DEC" originating at position %"DEC" from clone %"DEC" with range [%"DEC",%"DEC"]\n", cl->summary.clID, cl->summary.first_bif_pos, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->summary.end || (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.end) ) {
                ::printf("Problem with clone %"DEC" ending at position %"DEC" to clone %"DEC" with range [%"DEC",%"DEC"]\n", cl->summary.clID, cl->summary.end, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
            }
        }
    }
#endif
    return 1;
}

//static
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

bool sFrameConsensus::validateSimilarities(idx simil_size, sDic<idx> * ext_simil){
    sCloneConsensus * cl = 0;
    ///unused sBioseqpopul::clonePosition * p = 0;
    sDic<idx> * sim = ext_simil ? ext_simil : &similarities;
    bool res = true;
    idx sum = 0, *val = 0;sStr tt_buf;
    for(idx i = 0 ; i < dim(); ++i) {
        cl = ptr(i);
        idx sub_start = cl->summary.start, sub_end = sub_start + cl->seqCov.dim();
        for ( idx j = sub_start ; j < sub_end ; ++j ) {
            sum = 0;
            for(idx is = 0 ; is < simil_size ; ++is ) {
                tt_buf.printf(0,"%s",contructSimilaritiesID(cl->summary.clID,j,is));
                val = sim->get((const void *)tt_buf.ptr(),tt_buf.length());
                if(val) {
                    sum += *val;
                }
            }
            if( sum!=cl->seqCov.ptr(j - cl->summary.start)->coverage() ) {

#ifdef _DEBUG
                ::printf("not matching dictionary and seqCov. clNum:%"DEC"  pos:%"DEC" \n", cl->summary.clID, j);
#endif
                res = false;
            }
        }
    }
    return res;
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
//        sim_id_v = similarities.get((const void *)id_s, sim_id_l);

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
                if( cl->seqCov.ptr(j)->isSupportedGap() ) {
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
            parent = getParentOnLocation(i,cl->summary.end-1);
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
            if( (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.first_bif_pos ) || pcl->summary.start > cl->summary.first_bif_pos) {
                ::printf("Problem with clone %"DEC" originating at position %"DEC" from clone %"DEC" with range [%"DEC",%"DEC"]", cl->summary.clID, cl->summary.first_bif_pos, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->summary.end || (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.end) ) {
                ::printf("Problem with clone %"DEC" ending at position %"DEC" to clone %"DEC" with range [%"DEC",%"DEC"]", cl->summary.clID, cl->summary.end, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
            }
        }
    }
#endif
    return 1;
}

const char * sFrameConsensus::contructSimilaritiesID ( idx icl, idx pos, idx reference ) {
    _simil_id_buf.printf(0,"%"DEC"-%"DEC"-%"DEC, icl,pos,reference);
    return _simil_id_buf;
}

idx sFrameConsensus::remapSimilarities(sDic<idx> *sim, idx icl, idx pos, idx remapped_icl) {
    idx * cov = 0, remapped = 0;
    sStr t_s;
    for(idx i = 0 ; i < simil_size ; ++i ) {
        t_s.printf(0,"%s",contructSimilaritiesID(icl, pos, i));
        cov = similarities.get( (const void *)t_s.ptr(),t_s.length() );
        if( cov ) {
            t_s.printf(0,"%s",contructSimilaritiesID(remapped_icl, pos, i));
            *sim->set( (const void *)t_s.ptr(),t_s.length() ) = *cov;
            ++remapped;
        }
    }
    return remapped;
}

void sFrameConsensus::remapSimilarities( sVec<idx> &vec_icl,sVec<idx> &vec_sort, idx pos) {
    ///unused idx *tmp_cov_1 = 0, *tmp_cov_2 = 0, tc2 = 0 ;
    assert(vec_icl.dim()==vec_sort.dim());
    sDic<idx> values;idx * v=0;
    for( idx i = 0 ; i < vec_sort.dim() ; ++i ) {
        for(idx ir = 0 ; ir < simil_size ; ++ir ) {
            _simil_id_buf.printf(0,"%s",contructSimilaritiesID(vec_icl[vec_sort[i]], pos, ir));
            v=similarities.get( (const void *)_simil_id_buf.ptr(), _simil_id_buf.length());
            if( v ) {
                _simil_id_buf.printf(0,"%s",contructSimilaritiesID(vec_icl[i],pos,ir));
                *values.set((const void *)_simil_id_buf.ptr(),_simil_id_buf.length()) = *v;
            }
        }
    }
    idx * t = 0;
    for( idx i = 0 ; i < vec_sort.dim() ; ++i ) {
        for(idx ir = 0 ; ir < simil_size ; ++ir ) {

            _simil_id_buf.printf(0,"%s",contructSimilaritiesID(vec_icl[i],pos,ir));
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
        if(!clP) {
            ::printf("FATAL ERROR unexpected invalid position\n");
        }
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

idx sFrameConsensus::cleanOutClones(sVec<idx> &cloneIndex)
{
    idx icl = 0;
    sFrameConsensus cleanClones(simil_size);
    cloneIndex.cut(0);
    cloneIndex.add(dim());
    cloneIndex.set(0);
    sCloneConsensus * addedclone = 0, *cl = 0, * t_cl = 0;

//    createChildList();
//    createMergeList();

#ifdef _DEBUG
    ::printf("CLEAN OUT START\n");
    for(idx i = 0 ; i < dim() ; ++i ) {
        sCloneConsensus * cl = ptr(i);
        if( cl->summary.clID < 0 || !cl->seqCov.dim() ){
            continue;
        }
        if( cl->summary.hasParent() ) {
            sCloneConsensus * pcl = ptr(cl->summary.parentClID);
            if( (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.start ) || pcl->summary.start > cl->summary.start) {
                ::printf("Problem with clone %"DEC" originating at position %"DEC" from clone %"DEC" with range [%"DEC",%"DEC"]\n", cl->summary.clID, cl->summary.start, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->summary.end || (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.end) ) {
                ::printf("Problem with clone %"DEC" ending at position %"DEC" to clone %"DEC" with range [%"DEC",%"DEC"]\n", cl->summary.clID, cl->summary.end, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
            }
        }
    }
#endif
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
            t_cl = getParentOnLocation(i,cl->summary.start);
            if(!t_cl)
                t_cl = cl;
            cl->summary.parentClID = t_cl->summary.clID;
        }
        if( cl->summary.hasMerged() ) {
            t_cl = getMergedOnLocation(i,cl->summary.end-1);
            if(!t_cl)
                t_cl = cl;
            cl->summary.mergeclID = t_cl->summary.clID;
        }
        if( cl->summary.hasParent() && getFirstMutation(i) < 0 && !cl->summary.isLink() ) {
            t_cl = ptr(cl->summary.parentClID);
            if( t_cl->isValid() ) {
                mergePositions(t_cl, cl);
                t_cl->summary.end = t_cl->seqCov.dim() + t_cl->summary.start;
                updateDependencies(cl,t_cl);
            }
            cl->summary.clID = -1;
            continue;
        } else {
            addedclone = cleanClones._vec.add();
        }

        idx t_end = cl->getLastSupportedPosition() + 1;
        if( t_end > cl->summary.start && t_end < cl->summary.start + cl->seqCov.dim() ) {
            if(t_end - cl->summary.start < 2 ) t_end = cl->summary.start + 2;
            cl->seqCov.cut(t_end - cl->summary.start);
            cl->summary.end = t_end;
            fixMergedDependencies(cl);
        }
        idx offstart = 0;
        idx t_start = cl->getFirstSupportedPosition();
        if( t_start > cl->summary.start) {
            offstart = t_start - cl->summary.start;
            cl->summary.start = t_start;
            fixParentDependencies(cl);
        }

        if( cl->summary.hasParent() ) {
            cl->summary.first_bif_pos = getFirstMutation(i);
            if( cl->summary.first_bif_pos < cl->summary.start || cl->summary.first_bif_pos > cl->summary.end ) {
                cl->summary.first_bif_pos = cl->summary.start;
            }
        } else {
            cl->summary.first_bif_pos = cl->summary.start;
        }

        addedclone->stats = cl->stats;
        addedclone->summary = cl->summary;

        addedclone->seqCov.resize(cl->seqCov.dim() - offstart);

        addedclone->summary.end = addedclone->summary.start + cl->seqCov.dim();

        for(idx j = offstart; j < addedclone->seqCov.dim(); ++j) {
            remapSimilarities(&cleanClones.similarities,i, addedclone->summary.start+j,icl);
            addedclone->seqCov.ptr(j - offstart)->copy(cl->seqCov.ptr(j));
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
//    for(idx i = 0 ; i < cleanClones.dim(); ++i ) {
//        addedclone = ptr(i);
//        if( addedclone->summary.hasParent() ) {
//            addedclone->summary.first_bif_pos = getFirstMutation(i);
//            if( addedclone->summary.first_bif_pos < 0 ) {
//                addedclone->summary.parentClID = addedclone->summary.clID;
//            }
//        }
//        if( addedclone->summary.hasMerged() ) {
//            idx mergePos = getLastMutation(i);
//            if(mergePos < 0 ) {
//                addedclone->summary.mergeclID = addedclone->summary.clID;
//            }
//        }
//    }
#ifdef _DEBUG
    ::printf("CLEAN OUT END\n");
    for(idx i = 0 ; i < dim() ; ++i ) {
        if( cl->summary.clID < 0 || !cl->seqCov.dim() ){
            continue;
        }
        sCloneConsensus * cl = ptr(i);
        if( cl->summary.hasParent() ) {
            sCloneConsensus * pcl = ptr(cl->summary.parentClID);
            if( (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.start ) || pcl->summary.start > cl->summary.first_bif_pos) {
                ::printf("Problem with clone %"DEC" originating at position %"DEC" from clone %"DEC" with range [%"DEC",%"DEC"]\n", cl->summary.clID, cl->summary.start, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
            }
        }
        if( cl->summary.hasMerged() ) {
            sCloneConsensus * pcl = ptr(cl->summary.mergeclID);
            if( pcl->summary.start > cl->summary.end || (pcl->summary.end >= 0 && pcl->summary.end < cl->summary.end) ) {
                ::printf("Problem with clone %"DEC" ending at position %"DEC" to clone %"DEC" with range [%"DEC",%"DEC"]\n", cl->summary.clID, cl->summary.end, pcl->summary.clID, pcl->summary.start, pcl->summary.end);
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
;

idx sFrameConsensus::pairClones(sVec<short> * alClonIds, sBioal * bioal, sVec<idx> * cloneIndex, sVec<sBioseqAlignment::Al *> * alSub, sVec<sVec<idx> > * alSubLUT)
{
    idx clCnt = dim();

    if( !clCnt || !alClonIds )
        return 0;
    sVec<sVec<idx> > cloneMap(sMex::fExactSize), bifLMap(sMex::fExactSize), bifRMap(sMex::fExactSize);
    cloneMap.add(clCnt);    //cloneMap.set(0);
    bifLMap.add(clCnt);
    bifRMap.add(clCnt);
    for(idx ic = 0; ic < cloneMap.dim(); ++ic) {
        cloneMap.ptr(ic)->add(clCnt);
        cloneMap.ptr(ic)->set(0);
        bifLMap.ptr(ic)->add(clCnt);
        bifLMap.ptr(ic)->set(0);
        bifRMap.ptr(ic)->add(clCnt);
        bifRMap.ptr(ic)->set(0);
    }

    bioal->Qry->setmode(sBioseq::eBioModeLong);
    idx qrCnt = bioal->Qry->dim();
    if( qrCnt % 2 )
        return 0;
    idx iAs = 0, Cln1 = 0, Cln2 = 0;
    qrCnt = qrCnt / 2;
    for(idx iAl = 0; iAl < qrCnt; ++iAl) {
        /*===================================================================================
         Grab the pairs and check whether both of the reads have been aligned
         ----------------------------------------------------------------------------
         If not then ignore this pair
         ===================================================================================*/
        iAs = bioal->Qry->long2short(iAl);
        sBioseqAlignment::Al* hdrA = bioal->getAl(iAs);

        Cln1 = (idx) (*alClonIds->ptr(iAs));

        iAs = bioal->Qry->long2short(iAl + qrCnt);
        sBioseqAlignment::Al * hdrB = bioal->getAl(iAs);

        Cln2 = (idx) (*alClonIds->ptr(iAs));
        if( Cln1 < 0 || Cln2 < 0 )
            continue;
        Cln1 = *cloneIndex->ptr(Cln1);
        Cln2 = *cloneIndex->ptr(Cln2);
        /*===================================================================================
         First alignmnet from the pair ends
         ----------------------------------------------------------------------------
         It is assumed that it is the left one.
         As a result we are interested only in the first position of alignment (m[0])
         ===================================================================================*/

        sCloneConsensus * cl1 = ptr(Cln1);
        if( cl1->summary.clID < 0 ) {
            Cln1 = cl1->summary.mergeclID;
            cl1 = ptr(Cln1);
        }

        idx * m1 = bioal->getMatch(iAs);
        idx vecMA[2] = {
            *m1,
            0 };
        m1 = (idx *) &vecMA;
        idx hdr1SubStart = hdrA->subStart() + m1[0], offsMutAl1 = 0;
        if( alSub ) {
            sBioseqAlignment::Al * hdrToA = *alSub->ptr(hdrA->idSub());
            idx * mToA = hdrToA->match();

            offsMutAl1 = hdrToA->subStart();
            sBioseqAlignment::remapAlignment(hdrA, hdrToA, m1, mToA, 1, true, alSubLUT->ptr(hdrA->idSub()));
        }
        hdr1SubStart += offsMutAl1;

        /*===================================================================================
         Second alignmnet from the pair ends
         ----------------------------------------------------------------------------
         It is assumed that it is the right one.
         As a result we are NOT interested just in the first position of alignment (m[0])
         but in the last. So we need to uncompress and remap the whole match train
         ===================================================================================*/

        sCloneConsensus * cl2 = ptr(Cln2);
        if( cl2->summary.clID < 0 ) {
            Cln2 = cl2->summary.mergeclID;
            cl2 = ptr(Cln2);
        }

        idx * m2 = bioal->getMatch(iAs);
        //-----------------------------------------------------------------------
        // deal with compressed alignments
        //-----------------------------------------------------------------------
        sVec<idx> uncompressMM;

        if( hdrA->flags() & sBioseqAlignment::fAlignCompressed ) {
            uncompressMM.resize(hdrB->lenAlign() * 2);
            sBioseqAlignment::uncompressAlignment(hdrB, m2, uncompressMM.ptr());
            m2 = uncompressMM.ptr();
        }
        //______________________________________________________________________
        idx hdr2SubEnd = hdrB->subStart(), offsMutAl2 = 0;
        if( alSubLUT ) {
            sBioseqAlignment::Al * hdrToB = *alSub->ptr(hdrB->idSub());
            idx * mToB = hdrToB->match();

            offsMutAl2 = hdrToB->subStart();
            sBioseqAlignment::remapAlignment(hdrB, hdrToB, m2, mToB, 0, true, alSubLUT->ptr(hdrB->idSub()));
        }
        hdr2SubEnd += offsMutAl2 + m2[2 * (hdrB->lenAlign() - 1)];

        if( sOverlap(cl2->summary.start, cl2->summary.start, hdr1SubStart, hdr2SubEnd) ) {
            ++bifLMap[Cln1][Cln2];
        } else if( sOverlap(cl2->summary.end, cl2->summary.end, hdr1SubStart, hdr2SubEnd) ) {
            ++bifRMap[Cln1][Cln2];
        } else
            ++cloneMap[Cln1][Cln2];
    }
    sVec<idx> pairMap;
    pairMap.add(clCnt);
    pairMap.set(-1);
    idx remapped = 0;
    for(idx ic = 0; ic < clCnt; ++ic) {
        sVec<idx> clVec;
        clVec.add(clCnt);
        clVec.set(0);
        for(idx ir = 0; ir < clCnt; ++ir) {
            clVec[ic] = cloneMap[ic][ir] + cloneMap[ir][ic];
        }

        idx maxP = 0, iM = -1;
        for(idx ir = 0; ir < clVec.dim(); ++ir) {
            if( clVec[ir] > maxP ) {
                maxP = clVec[ir];
                iM = ir;
            }
        }
        if( iM >= 0 && iM != ic ) {
            idx iC = ic;
            ++remapped;
            if( cloneMap[iM][iM] < cloneMap[iC][iC] ) {
                iC = iM;
                iM = ic;
            }
            pairMap[iC] = iM;
            for(idx ir = 0; ir < clCnt; ++ir) {
                if( ir != iC )
                    cloneMap[iM][ir] += cloneMap[iC][ir];
                else
                    cloneMap[iM][iM] += cloneMap[iC][ir];

                if( ir != iC )
                    cloneMap[ir][iM] += cloneMap[ir][iC];
                else
                    cloneMap[iM][iM] += cloneMap[ir][iC];

                cloneMap[iC][ir] = 0;
                cloneMap[ir][iC] = 0;
            }
        }
    }
    return remapped;
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
            if( cp->isSupportedGap() )
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
                idx t_b = cp->base();
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
