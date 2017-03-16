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
#pragma once
#ifndef sBio_seqpopul_hpp
#define sBio_seqpopul_hpp
#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/utils.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioal.hpp>

//#define DEBUGGING_WINDOWRESIZING

namespace slib {

    class sBioseqpopul
    {

        public:
            typedef enum eBaseCalls {
                baseNone = -1,
                baseA = 0,
                baseC = 1,
                baseG = 2,
                baseT = 3,
                baseDel = 4,
                baseIns = 5,
                baseGap = 6,
                baseN = 7,
                baseLast = baseN
            } baseCalls;

            struct position
            {
                idx base[6];
                idx baseAny;
                baseCalls basecall;
                idx coverage;
                sVec<idx> referenceSimil;

                position()
                {
                    init();
                }
                inline idx getGappyCoverage(bool withAmbiguities = false)  // ACGT, read supported Deletion and GAPS
                {
                    idx res = coverage;
                    if( res < 0 ) res = 0;
                    return res;
                }
                inline idx getSupportedCoverage(bool withAmbiguities = false)  //ACGT and read supported Deletion
                {
                    idx res = base[baseA] + base[baseC] + base[baseG] + base[baseT] + base[baseDel];
                    if( res < 0 ) res = 0;
                    if( withAmbiguities ) res += baseAny;
                    return res;
                }
                inline idx getAllCoverage(bool withAmbiguities = false) //everythin ACGT, read supported Deletions, GAPS and Insertions
                {
                    idx res = coverage + base[baseIns];
                    if( res < 0 ) res = 0;
                    if( withAmbiguities ) res += baseAny;
                    return res;
                }
                inline idx getCoverageGap()    //Get GAPS
                {
                    return getGappyCoverage() - getSupportedCoverage();
                }
                inline idx getCoverage(baseCalls b){
                    if( base[b] < 0 )
                        return 0;
                    return base[b];
                }
                inline idx setbasecall()
                {
                    idx max = 0; baseCalls maxI = baseNone;
                    for(idx i = 0; i <= baseDel; ++i) {
                        if( base[i] > max ) {
                            max = base[i];
                            maxI = (baseCalls)i;
                        }
                    }
                    basecall = maxI;
                    if(maxI<0 && baseAny>0)
                        basecall = baseN;
                    return maxI;
                }
                inline bool isGap(real thrshld)
                {
                    return ( ((real)getCoverageGap()/getGappyCoverage(true)) >= thrshld );
                }
                inline bool isN()
                {
                    return (basecall == baseN);

                }
                void init()
                {
                    for(idx j = 0; j < sDim(base); ++j) {
                        base[j] = 0;
                    }
                    referenceSimil.set(0);
                    coverage = 0;
                    baseAny = 0;
                    basecall = baseNone;
                }
                void copy(position & p)
                {
                    coverage = p.coverage;
                    basecall = p.basecall;
                    if(p.referenceSimil.dim())referenceSimil.copy(&p.referenceSimil);
                    for(idx l = 0; l < sDim(base); ++l) {
                        base[l] = p.base[l];
                    }
                    baseAny = p.baseAny;
                }
                inline bool add(idx rpts, idx letter, idx ref_sim_ind)
                {
                    if( letter > baseLast ) { // wrong input
                        return false;
                    }
                    if ( letter < 0 ) { //multiple alignment gap
                        coverage += rpts;
                        return true;
                    }
                    if(letter == baseN) {
                        baseAny += rpts;
                        return true;
                    }
                    base[letter] += rpts;
                    if(letter<5) {   // if NOT insertion adjust coverage, similarities and basecall.
                        coverage += rpts;
                        if(referenceSimil.dim()>ref_sim_ind)referenceSimil[ref_sim_ind] += rpts;

                        if( base[letter] < 0 ) {
                            base[letter] = 0;
                        }
                        if((idx)basecall<0)basecall=(baseCalls)letter;
                        else if( basecall != letter && rpts>0 ){
                            setbasecall();
                        }
                        else if( basecall == letter && rpts<0 ){
                            setbasecall();
                        }
                    }
                    return true;
                }
                void transfer(position &p)
                {
                    coverage += p.coverage;
                    baseAny += p.baseAny;
                    p.coverage = 0;
                    p.baseAny = 0;
                    for(idx l = 0; l < sDim(base) ; ++l) {
                        base[l] += p.base[l];
                        p.base[l] = -1;
                    }
                    setbasecall();
                    p.basecall = baseNone;
                    for(idx iR = 0; iR < referenceSimil.dim(); ++iR) {
                        referenceSimil[iR] += p.referenceSimil[iR];
                        p.referenceSimil[iR] = -1;
                    }
                }
                inline void remove(idx rpts, idx letter, idx ref_sim_ind)
                {
                    add(-rpts, letter, ref_sim_ind);
                }
//                inline void set(idx rpts, idx letter, idx ref_sim_ind)
//                {
//                    coverage = rpts;
//                    base[letter] = rpts;
//                    if(referenceSimil.dim()>ref_sim_ind)referenceSimil[ref_sim_ind] = rpts;
//                }

                inline bool isDiff(idx alBase) {
                    return !(isN() || (alBase==6) || (basecall == alBase));
                }

                inline bool isDiff(position * cmp, real thrshld)
                {
                    bool isp1G=isGap(thrshld);
                    bool isp2G=cmp->isGap(thrshld);
                    if( isp1G && isp2G ) {
                        return false;
                    } else if( isp1G || isp2G ) {
                        return true;
                    } else if ( isN() || cmp->isN() ) {
                        return false;
                    }
                    return basecall!=cmp->basecall;
                }

                bool isBifurcatingPosition(real cutoff,idx minCov, idx * firstInd = 0, idx * secInd = 0, idx * compare = 0)
                {
                    idx max = 0, secondmax = 0;
                    for(idx k = 0; k < 5; ++k) {
                        if( max < base[k] ) {
                            secondmax = max;
                            max = base[k];
                            if( secInd && firstInd ) {
                                *secInd = *firstInd;
                            }
                            if( firstInd ) {
                                *firstInd = k;
                            }
                        } else if( secondmax < base[k] ) {
                            secondmax = base[k];
                            if( secInd && firstInd ) {
                                *secInd = k;
                            }
                        }
                    }
                    if(!secondmax) return false;
                    //check that both the newly created clone and the remaining clone are above threshold
                    real freq = sMin((real) secondmax / (compare ? (*compare) : coverage), (real) (coverage-secondmax) / (compare ? (*compare) : coverage));
                    idx minResCov = sMin(secondmax, (coverage-secondmax));

                    return (freq > cutoff) && minResCov >= minCov;
                }

                void reset()
                {
                    basecall = baseNone;
                }
            };
            struct clonePosition
            {
                    idx baseCov;
//                    sVec<idx> coverPerRef;

                    clonePosition(){
                        baseCov = 0;
//                        coverPerRef.set(0);
                    }
                    inline idx base()
                    {
                        return (baseCov >> 60) & 0xF;
                    }
                    inline idx coverage()
                    {
                        return (baseCov & 0xFFFFFFFFFFFFFFF);
                    }
                    inline void setCoverage(idx val)
                    {
                        if( val < 0 )
                            val = 0;
                        baseCov = (baseCov & 0xF000000000000000) | (val & 0xFFFFFFFFFFFFFFF);
                    }
//                    inline void setBase(idx val)
//                    {
//                        if( val == 6 )
//                            val = 7;
//                        if( val < 0 )
//                            val = 6;
//                        baseCov = (baseCov & 0xFFFFFFFFFFFFFFF) | ((val & 0xF) << 60);
//                    }
                    inline void setBase(idx val)
                    {
                        if( val < 0 )
                            val = baseGap;
                        baseCov = (baseCov & 0xFFFFFFFFFFFFFFF) | ((val & 0xF) << 60);
                    }
                    inline bool isMultiGap()
                    {
                        return ( base() == baseGap);
                    }

                    inline bool isN()
                    {
                        return ( base() == baseN);
                    }

                    inline bool isSupportedGap()
                    {
                        return ( base() == baseDel);
                    }

//
//                    inline bool isMultiGap()
//                    {
//                        return ( base() == 6);
//                    }
//
//                    inline bool isN()
//                    {
//                        return ( base() == 7);
//                    }
//
//                    inline bool isSupportedGap()
//                    {
//                        return ( base() == 4);
//                    }

                    inline bool isAnyGap()
                    {
                        return isMultiGap() || isSupportedGap();
                    }

                    inline void setPosition(sBioseqpopul::position * p)
                    {
                        setBase(p->basecall);
                        setCoverage(p->getSupportedCoverage(true));
                    }
                    inline void setGap (sBioseqpopul::position * p)
                    {
                        setBase(-1);
                        setCoverage( p->getCoverageGap() );
                    }
                    void swap (clonePosition * n_pos) {
                        clonePosition p;
                        p.copy(this);
                        copy(n_pos);
                        n_pos->copy(&p);
                    }

                    void copy (clonePosition * n_pos) {
                        baseCov = n_pos->baseCov;
                    }
                    inline void add(clonePosition * n) {
                        if ( n->coverage() > coverage() ) {
                            setBase( n->base() );
                        }
                        setCoverage( coverage() + n->coverage() );
                    }
            };

            struct cloneSummary
            {
                    idx start, end, first_bif_pos, parentClID, clID, mergeclID;
                    inline bool hasParent(void) {
                        return parentClID>=0 && parentClID!=clID;
                    }
                    inline bool hasMerged(void) {
                        return mergeclID>=0 && mergeclID!=clID;
                    }
                    inline bool isLink(void) {
                        return hasParent() && hasMerged() && parentClID!=mergeclID;
                    }
                    inline bool wasKilled(void) {
                        return clID == -1;
                    }
                    cloneSummary()
                    {
                        sSet(this, 0);
                    }
            };

            struct cloneStats
            {
                    idx maxCov, size, support, stacked_size, supportedGapsCnt, multiGapsCnt;
                    real avCov, bifurcation_bayes, bifurcation_pvalue;
                    cloneStats()
                    {
                        sSet(this, 0);
                        bifurcation_bayes = sNotIdx; bifurcation_pvalue = sNotIdx;
                    }
            };

            struct points
            {
                    idx position;
                    idx base;
                    points()
                    {
                        sSet(this, 0);
                    }
            };
            struct clonePat
            {
                    sVec<points> pattern;
                    idx posScanned;
                    idx curpos;
                    idx clID;
                    idx first()
                    {
                        return (pattern.dim() > 0) ? pattern[(idx) 0].position : -1;
                    }
                    idx current()
                    {
                        return (pattern.dim() > 0) ? (curpos < pattern.dim() ? pattern[curpos].position : 0) : -1;
                    }
                    idx last()
                    {
                        return (pattern.dim() > 0) ? pattern[pattern.dim() - 1].position : -1;
                    }

                    clonePat()
                    {
                        posScanned = curpos = clID = 0;
                    }
            };
            struct cloneFingPrs
            {
                    sVec<clonePat> fingerPrints;
//            idx relativeTo;
            };

            static idx sortClonesComparator(void * parameters, void * arr, idx i1, idx i2);

            struct ParamsCloneSorter
            {
                    idx flags;
            };

            enum fSortCloneFlags
            {
                clSortByMaxCov = 0x001,
                clSortByAvCov = 0x002,
                clSortByPos = 0x004,
                clSortByFathID = 0x008,
                clSortBySize = 0x010
            };

        private:

//        static idx referenceSize;
    };

    template<class Tobj> class sWdw
    {
//            sVec<Tobj> wParent;
        private:
            sVec<Tobj> _vec;
            inline idx _mod(idx i)
            {
                return (i >= 0) ? i % _vec.dim() : -1;
            }
            inline Tobj * _pos(idx i)
            {
                idx ix = _mod(i);
                return ix >= 0 ? _vec.ptr(ix) : 0;
            }

        public:
//            sWdw(idx size){
//                init(size);
//            };

            virtual void init(idx size)
            {
                _vec.add(size);
                curMatPOS = 0;
                mergeMatPOS = 0;
                extrPos = 0;
            }

            //  Only one bifurcation position is considered in each window.
            //  It is still a vector of bifurcated IDs and bases to catch the case of
            //  multiple bifurcations in one position (e.g.: A:60% T:20% G:15% C:5%).
//            idx base;                                 //  base holds the base in this clone that created the bifurc from the father clone
            inline idx length()
            {
                return (_vec.dim() / 2);
            }
            idx curMatPOS;

            idx mergeMatPOS;

            idx extrPos;

            virtual ~sWdw()
            { destroy(); };

            virtual void destroy() {
                _vec.destroy();
            }

            inline idx mod(idx i)
            {
                return _mod(i + curMatPOS);
            }
            inline Tobj * pos(idx i)
            {
                return _pos(i + curMatPOS);
            }

            virtual inline idx getfirst()
            {
                idx t = -1;
                if( _vec.dim() ) {
                    t = curMatPOS;
                }
                return t;
            }
            virtual inline idx getlast()
            {
                idx t = -1;
                if( _vec.dim() ) {
                    t = curMatPOS + length();
                }
                return t;
            }
            virtual inline idx getTailfirst()
            {
                idx t = -1;
                if( _vec.dim() ) {
                    t = curMatPOS + length();
                }
                return t;
            }
            virtual inline idx getTailLast()
            {
                idx t = -1;
                if( _vec.dim() ) {
                    t = curMatPOS + 2 * length();
                }
                return t;
            }

            idx expMatPOS(idx matPos)
            {
                return matPos >= curMatPOS ? matPos - curMatPOS : (dim() - curMatPOS + matPos);
            }

            void clean()
            {
                idx matSize = _vec.dim();
                for(idx i = 0; i < matSize; ++i) {
                    sSet(_vec.ptr(i), 0);
                }
            }
            ;

            virtual void move(idx t_pos)
            {
                idx st = getfirst(), ed = st + t_pos;
                Tobj * t_o = 0;
                for(idx i = st; i < ed; ++i) {
                    t_o = _pos(i);
                    if( t_o ) {
                        sSet(t_o, 0, sizeof(Tobj));
                    }
                }
                curMatPOS = mod(t_pos);
            }
            void stretch(idx addSize)
            {
                idx t_size = 2 * addSize;

                _vec.insert(getfirst(), t_size);

                curMatPOS += t_size;
//                if(extrPos)
//                    extrPos+=t_size;
                if( mergeMatPOS )
                    mergeMatPOS += t_size;
            }

            virtual void mergeTo(sWdw * merged, idx pos)
            {
            }
            ;

            Tobj * add(idx cntAdd = 1)
            {
                return _vec.add(cntAdd);
            }
            virtual idx dim()
            {
                return _vec.dim();
            }
            void flagOn(idx fl)
            {
                _vec.flagOn(fl);
            }

            Tobj & operator [](idx index)
            {
                return *_vec.ptr(index);
            }

            Tobj * ptr(idx index)
            {
                return _vec.ptr(index);
            }
    };

    class sClone: public sWdw<sBioseqpopul::position>
    {
            typedef sWdw<sBioseqpopul::position> cParent;
        private:
            bool _report_ref;

        public:
            void init(idx t_size, idx t_ofs, idx t_clID, idx t_ref_cnt)
            {
                cParent::init(2 * t_size);
                offset = t_ofs;
                clID = t_clID;
                if(t_ref_cnt) {
                    _report_ref=true;
                }
                else {
                    _report_ref=false;
                }
                ref_cnt = _report_ref?t_ref_cnt:1;
                supportCnt = 0;
                bifurcation_pos = 0;
                bifurcation_bayes = sNotIdx;
                bifurcation_pvalue = sNotIdx;
                merged = false;
                merging = false;
                killed = false;
                dead = false;
                curSubPOS = 0;
                parentClID = 0;
                startSubPOS = 0;
                deathSubPOS = 0;
                mergingScore = 0;
                mergingCl = -1;
                mergedToCl = -1;
                mergeSubPOS = -1;
                avCov = 0;
                for(idx i = 0; i < dim(); ++i) {
                    sBioseqpopul::position * p = ptr(i);
                    p->referenceSimil.resize(ref_cnt);
                    p->referenceSimil.set(0);
                }
            }
            void init(sClone * newCl, idx t_clID);
            idx offset;
            //Clone indices
            idx clID, parentClID;
            idx startSubPOS;
            idx curSubPOS;

            idx ref_cnt;

            //Death event
            idx deathSubPOS;

            //Merge event
            idx mergedToCl, mergingCl, mergingScore;
            idx mergeSubPOS;

            real avCov;

            //event
            bool merged, merging, killed, dead;

            //extracted

            //Bifurcation event
            idx supportCnt;
            idx bifurcation_pos;
            sBioseqpopul::points lastBif;
            real bifurcation_pvalue,bifurcation_bayes;

            virtual sBioseqpopul::position * pos(idx i)
            {
                return cParent::pos(i - curSubPOS);
            }

            void reset(idx step)
            {
                offset += step;
                idx st = getrawfirst(), end = getTailLast();
                for(idx i = st; i < end; ++i)
                    pos(i)->reset();
            }

            bool alive()
            {
                return !(killed || dead || merged || length() + curSubPOS < startSubPOS);
            }


            bool hasParent()
            {
                return (parentClID >=0 && parentClID!=clID);
            }

            real avCoverage() {
                if( avCov ) {
                    return avCov;
                }
                idx ret = 0, cov_pos = 0;
                for(idx i = 0; i < dim(); ++i) {
                    if( ptr(i)->getGappyCoverage() ){
                        ret += ptr(i)->getGappyCoverage();
                        ++cov_pos;
                    }
                }
                if(!ret) return 0;
                avCov = (real) ret / cov_pos;
                return avCov;
            }

            bool isGap(idx i, real thrshld) {
                if( !avCoverage() ) return false;
                return (real)pos(i)->getSupportedCoverage(true)/avCov < thrshld;
            }

            void setCalls(idx st = -1, idx end = -1)
            {
                if( end < 0 )
                    st = getrawfirst();
                if( end < 0 )
                    end = getTailLast();
                for(idx i = st; i < end; ++i)
                    pos(i)->setbasecall();
            }

            virtual inline idx getrawfirst()
            {
                idx t = -1;
                if( dim() ) {
                    t = curSubPOS;
                }
                return t;
            }
            virtual inline idx getfirst()
            {
                idx t = -1;
                if( dim() ) {
                    t = curSubPOS + offset;
                }
                return t;
            }
            virtual inline idx getlast()
            {
                idx t = -1;
                if( dim() ) {
                    t = curSubPOS + length();
                }
                return t;
            }
            virtual inline idx getTailfirst()
            {
                idx t = -1;
                if( dim() ) {
                    t = curSubPOS + length();
                }
                return t;
            }
            virtual inline idx getTailLast()
            {
                idx t = -1;
                if( dim() ) {
                    t = curSubPOS + 2 * length();
                }
                return t;
            }

            inline idx subPos(idx matPos)
            {
                return curSubPOS + expMatPOS(matPos);
            }
            inline idx matPos(idx subPos)
            {
                return mod(subPos - curSubPOS);
            }

            idx getLastDiff(sClone * dest,idx start,idx end);

//            inline bool isFuzzy(idx p, real cutoff)
//            {
//                if(!avCoverage())return true;
//                return (real)pos(p)->getCoverage()/avCoverage() < cutoff;
//            }
            void strech(idx addSize);

            virtual void mergeTo(sClone * merged, idx pos);

            inline idx getfirstNonZero(idx * start = 0, idx * end = 0)
            {
                idx st = start ? *start : getrawfirst(), ed = end ? *end : getTailLast();
                while( !(pos(st) && pos(st)->getSupportedCoverage(true)) && st < ed )
                    ++st;
                return st;
            }

            inline idx getlastNonZero(idx * start = 0, idx * end = 0)
            {
                idx st = start ? *start : getrawfirst(), ed = end ? *end : getTailLast();
                --ed;
                if( merged && mergeSubPOS < ed )
                    ed = mergeSubPOS;
                while( !(pos(ed) && pos(ed)->getSupportedCoverage(true)) && ed > st )
                    --ed;
                return ed + 1;
            }

            inline void addL(idx isx, idx rpts, idx letter, idx ref_sim_ind){
                pos(isx)->add(rpts,letter,_report_ref?ref_sim_ind:0);
            }
            inline void removeL(idx isx, idx rpts, idx letter, idx ref_sim_ind){
                pos(isx)->remove(rpts,letter,_report_ref?ref_sim_ind:0);
            }

            void move(idx step);

            const char * print(real cutoff, idx minCov = 0, sStr * out = 0);
            static idx cmpClones(void * param, void * arr, idx i1, idx i2);
    };

    class sCloneConsensus
    {
        public:
            idx extrSubPOS;
            sVec<sBioseqpopul::clonePosition> seqCov;
            sBioseqpopul::cloneSummary summary;
            sBioseqpopul::cloneStats stats;

            inline idx getSubPosBase(idx pos);
            inline idx getSubPosCoverage(idx pos);
            sBioseqpopul::clonePosition *  getSubPos(idx pos);
            inline idx getLastSupportedPosition(idx pos = 0) {
                if(pos <= 0)
                    pos = seqCov.dim() - 1 + summary.start;
                idx res = pos;
                while ( res >= summary.start && getSubPos(res)->isMultiGap() ) {
                    --res;
                }
                if( res <= summary.start ) {
                    return -1;
                }
                return res;
            }
            inline idx getFirstSupportedPosition(idx pos = 0 ) {
                if(pos <= 0)
                    pos = summary.start;
                idx res = pos;
                while (res < seqCov.dim() + summary.start && getSubPos(res)->isMultiGap() ) {
                    ++res;
                }
                if( res >= seqCov.dim() + summary.start ) {
                    return -1;
                }
                return res;
            }
            inline bool hasValidCoverage() {
                for(idx i = 0; i < seqCov.dim() ; ++i ) {
                    if( seqCov[i].coverage() && !seqCov[i].isAnyGap() ) {
                        return true;
                    }
                }
                return false;
            }

            inline bool isValid() {
                return ( summary.clID >= 0 && seqCov.dim() && hasValidCoverage() );
            }

            sCloneConsensus(){
                extrSubPOS=-1;
                seqCov.set(0);
            }


//            void readClone(sClone * clone, real avCoverage, bool toEnd = false);
    };

//    struct sSimilIndex { idx defint; idx simil;}
    class sFrameConsensus
    {
        private:
            sVec<sCloneConsensus> _vec;
            sVec < sVec < idx > > _childList;
            sVec < sVec < idx > > _mergeList;
            sStr _simil_id_buf;
        public:
            typedef idx (*callbackType)(void * param, idx countDone, idx progressCur, idx percentMax);
            callbackType progress_CallbackFunction;
            void * progress_CallbackParam;

            idx sub_start, sub_end, simil_size;
            sFrameConsensus(idx t_simil_size) {
                sub_start = 0;
                sub_end = 0;
                simil_size = t_simil_size;
            }

            sDic<idx> similarities;

            void createChildList()
            {
                for(idx i = 0; i < dim(); ++i) {
                    if( ptr(i)->summary.clID >= 0 && ptr(i)->seqCov.dim() ) {
                        sVec<idx> * children = _childList.ptrx(ptr(i)->summary.clID);
                        for(idx j = i; j < dim(); ++j) {
                            if( ptr(j)->summary.hasParent() && ptr(j)->summary.parentClID ) {
                                *children->add() = ptr(j)->summary.parentClID;
                            }
                        }
                    }
                }
            }

            sVec<idx> * getChildren(sCloneConsensus * cl)
            {
                if( cl->summary.clID < 0 || cl->summary.clID >= _childList.dim() ) {
                    return 0;
                }
                if( !_childList.ptr(cl->summary.clID)->dim() ) {
                    return 0;
                }
                return _childList.ptr(cl->summary.clID);
            }

            bool updateChildList(sCloneConsensus * cl, idx oldParentClID, idx newParentClID) {
                if(oldParentClID != newParentClID) {
                    if( oldParentClID < _childList.dim() ) {
                        return false;
                    }
                    sVec<idx> * op = _childList.ptr(oldParentClID);
                    sVec<idx> * np = _childList.ptr(newParentClID);
                    idx i = 0;
                    for(i = 0 ; i < np->dim() && (*(np->ptr(i)) == cl->summary.clID ) ; ++i );
                    *np->ptrx(i)=cl->summary.clID;
                    for(i = 0 ; i < op->dim() ; ++i ) {
                        if(*(op->ptr(i)) == cl->summary.clID ) {
                            op->del(i,1);
                            return true;
                        }
                    }
                    return true;
                }
                return false;
            }

            void createMergeList()
            {
                for(idx i = 0; i < dim(); ++i) {
                    if( ptr(i)->summary.clID >= 0 && ptr(i)->seqCov.dim() ) {
                        sVec<idx> * merged = _mergeList.ptrx(ptr(i)->summary.clID);
                        for(idx j = i; j < dim(); ++j) {
                            if( ptr(j)->summary.hasMerged() && ptr(j)->summary.mergeclID ) {
                                *merged->add() = ptr(j)->summary.mergeclID;
                            }
                        }
                    }
                }
            }

            sVec<idx> * getMerged(sCloneConsensus * cl) {
                if( cl->summary.clID < 0 || cl->summary.clID >= _mergeList.dim() ) {
                    return 0;
                }
                if( !_mergeList.ptr(cl->summary.clID)->dim() ) {
                    return 0;
                }
                return _mergeList.ptr(cl->summary.clID);
            }

            bool updateMergeList(sCloneConsensus * cl, idx oldMergeClID, idx newMergeClID) {
                if(oldMergeClID != newMergeClID) {
                    if( oldMergeClID < _mergeList.dim() ) {
                        return false;
                    }
                    sVec<idx> * om = _mergeList.ptr(oldMergeClID);
                    sVec<idx> * nm = _mergeList.ptr(newMergeClID);
                    idx i = 0;
                    for(i = 0 ; i < nm->dim() && (*(nm->ptr(i)) == cl->summary.clID ) ; ++i );
                    *nm->ptrx(i)=cl->summary.clID;
                    for(i = 0 ; i < om->dim() ; ++i ) {
                        if(*(om->ptr(i)) == cl->summary.clID ) {
                            om->del(i,1);
                            return true;
                        }
                    }
                    return true;
                }
                return false;
            }

            bool reorderClones(void);
            idx mergeAll(void);
            idx trimClones(void);  //trim everything before first variation and after last variation
            bool patchClones();
            idx cleanOutClones(sVec<idx> &cloneIndex);
            void getCloneStats(sBioseqpopul::cloneStats * clSum);
            idx pairClones(sVec<short> * alClonIds, sBioal * bioal, sVec<idx> * cloneIndex, sVec<sBioseqAlignment::Al *> * alSub = 0, sVec<sVec<idx> > * alSubLUT = 0);
            sCloneConsensus * getParentOnLocation(idx iCl,idx pos);
            sCloneConsensus * getMergedOnLocation(idx iCl,idx pos);
            idx getClonesPositionsOnLocation(idx pos, sVec<idx> &cloneIDs, sVec<sBioseqpopul::clonePosition> &ret );
            static idx sortPositionsComparator(void * parameters, void * arr, idx i1, idx i2);
            idx getFirstMutation(idx iCl, idx pos = 0);

            idx remapSimilarities(sDic<idx> *sim, idx icl, idx pos, idx remapped_icl);
            void remapSimilarities( sVec<idx> &icl,sVec<idx> &sorted, idx pos);
            const char * contructSimilaritiesID ( idx icl, idx pos, idx reference );

            sCloneConsensus * ptr(idx i = 0 ) {return _vec.ptr(i);};
            sCloneConsensus * ptrx( idx i = 0 ){return _vec.ptrx(i);};
            idx dim(){return _vec.dim();};
            bool mergePositions(sCloneConsensus * dstCl, sCloneConsensus * srcCl, idx frmStart = 0, idx frmEnd = 0);

            idx getSimilarities(idx cloneID, idx framePos, sBioseqpopul::position * p );

            idx getCloneSupport(idx i);

            void setCloneSupport(void) {
                for(idx i = 0; i < dim(); ++i) {
                    ptr(i)->stats.support = getCloneSupport(i);
                }
            }

            void updateDependencies(sCloneConsensus * oldCl, sCloneConsensus * newCl) {
                if( oldCl && newCl ) {
                    for( idx j = 0 ; j < dim(); ++j ) {
                        sCloneConsensus * chCl = ptr(j);
                        if( j > oldCl->summary.clID ) {
                            if( chCl->summary.hasParent() && chCl->summary.parentClID == oldCl->summary.clID )
                                chCl->summary.parentClID = newCl->summary.clID;
                            if(chCl->summary.hasMerged() && chCl->summary.mergeclID == oldCl->summary.clID)
                                chCl->summary.mergeclID = newCl->summary.clID;
                        }
                    }
                }
            }

            void fixParentDependencies(sCloneConsensus * cl) {
                if( cl ) {
                    for( idx j = 0 ; j < dim(); ++j ) {
                        sCloneConsensus * chCl = ptr(j);
                        if( j > cl->summary.clID && chCl->summary.hasParent() && chCl->summary.parentClID == cl->summary.clID && chCl->summary.first_bif_pos < cl->summary.start ) {
                            sCloneConsensus * pCl = getParentOnLocation( cl->summary.clID, chCl->summary.first_bif_pos );
                            chCl->summary.parentClID = pCl ? pCl->summary.clID : chCl->summary.clID;
                        }
                        if( chCl->summary.hasMerged() && chCl->summary.mergeclID == cl->summary.clID && chCl->summary.end < cl->summary.start ) {
                            sCloneConsensus * pCl = getParentOnLocation( cl->summary.clID, chCl->summary.end );
                            chCl->summary.mergeclID = pCl ? pCl->summary.clID : chCl->summary.clID;
                        }
                    }
                }
            }
            void fixMergedDependencies(sCloneConsensus * cl) {
                if( cl ) {
                    for( idx j = 0 ; j < dim(); ++j ) {
                        sCloneConsensus * chCl = ptr(j);
                        if( j > cl->summary.clID && chCl->summary.hasParent() && chCl->summary.parentClID == cl->summary.clID && chCl->summary.first_bif_pos > cl->summary.end ) {
                            sCloneConsensus * mCl = getMergedOnLocation(cl->summary.clID, chCl->summary.first_bif_pos);
                            chCl->summary.parentClID = mCl ? mCl->summary.clID : chCl->summary.clID;
                        }
                        if( chCl->summary.hasMerged() && chCl->summary.mergeclID == cl->summary.clID && chCl->summary.end > cl->summary.end ) {
                            sCloneConsensus * mCl = getMergedOnLocation(cl->summary.clID, chCl->summary.end);
                            chCl->summary.mergeclID = mCl ? mCl->summary.clID : chCl->summary.clID;
                        }
                    }
                }
            }
            bool setConsensusStat(sBioseqpopul::cloneStats &clSum);

            bool validateSimilarities(idx simil_size, sDic<idx> * ext_sim = 0);
            idx sOutSimilarities(const char * flnm);
            idx createFrameMaps( const char * skp2gps, const char * gps2skp, const char * skp2gpsSpprt, const char * gps2skpSpprt);
    };

    class sFrame
    {
        private:
            sVec<sClone> _clones;
            sVec<idx> _cloneInds;
            real avCov;
            real gap_thrshld;
//            sStr _buf;
        public:
            sWdw<idx> _coverage;
            idx ref_cnt;

            sFrame(idx t_ref_cnt, real t_gap_thrshld)
            {
                _coverage.flagOn(sMex::fSetZero);
                avCov = 0;
                ref_cnt = t_ref_cnt;
                gap_thrshld = t_gap_thrshld;
            }

            void clean() {
                _clones.destroy();
                _cloneInds.destroy();
                _coverage.destroy();
            }

            void init(idx length)
            {
                if( !cnt() ) {
                    sClone * t_cl = add();
                    t_cl->init(length, 0, 0, ref_cnt);
                }
                if( !_coverage.dim() ) {
                    _coverage.init(2 * length);
                }
            }

            const char * printCoverage(sStr * out = 0);

            inline sClone * ptr(idx k = 0)
            {
                if (k < cnt() && k >= 0) {
                    return _clones.ptr(_cloneInds[k]);
                } else {
                    return 0;
                }
            }
            inline idx cnt()
            {
                return _clones.dim();
            }
            inline idx size()
            {
                return _coverage.dim();
            }
            inline sClone * add()
            {
                *_cloneInds.add() = _clones.dim();
                return _clones.add();
            }
            inline void swap(idx i1, idx i2){
                idx tmpi = _cloneInds[i1];
                _cloneInds[i1] = _cloneInds[i2];
                _cloneInds[i2] = tmpi;
            }
            void reorder(idx * rInds, idx start, idx cnt) {
                sClone * tcl = 0, *jcl = 0;
                idx newclID = 0, oldclID = 0;
                for(idx k = 0; k < cnt; ++k) {
                    oldclID = rInds[k] + start;
                    newclID = k +start;
                    if(oldclID==newclID) continue;
                    tcl = ptr(oldclID);
                    if( !tcl->dead && !(tcl->merged && tcl->mergeSubPOS<0) ) {
                        for(idx j = 0; j < start+ cnt; ++j) {
                            jcl = ptr( j );
                            if( j >= start) {
                                if( jcl->parentClID == oldclID )
                                    jcl->parentClID = -newclID-1;
                            }
                            if( jcl->mergedToCl == oldclID ) {
                                jcl->mergedToCl = -newclID-1;
                            }
                        }
                    }
                    tcl->clID = newclID;
                }
                for( idx i = 0 ; i < start + cnt ; ++i ) {
                    jcl = ptr( i );
                    if( jcl->mergedToCl < 0 ) jcl->mergedToCl = -jcl->mergedToCl-1;
                    if( jcl->parentClID < 0 ) jcl->parentClID = -jcl->parentClID-1;
                }
                for( idx i = 0 ; i < cnt ; ++i ) {
                    _cloneInds[i+start] = rInds[i] +start;
                }
            }
            inline idx win_size()
            {
                return _coverage.dim() / 2;
            }
            inline bool isFuzzy( sClone * w,idx p, real cutoff )
            {
                return isFramePositionFuzzy(p-w->curSubPOS, cutoff) || isClonalPositionFuzzy(w, p, cutoff) ;
            }
            inline bool isFramePositionFuzzy( idx p, real cutoff )
            {
                if(!avCoverage())return true;
                return (real)*_coverage.pos(p)/avCoverage() < cutoff;
            }
            inline bool isClonalPositionFuzzy(sClone * w, idx p, real cutoff)
            {
                if(! *_coverage.pos(p -w->curSubPOS))return true;
                return ( (real)w->pos(p)->getGappyCoverage()/ *_coverage.pos(p-w->curSubPOS) < cutoff || (real)w->pos(p)->getGappyCoverage() / w->avCoverage() < cutoff);
            }
            void move(idx pos);
            void setCalls(void);

            sClone * branch(idx prevID, idx setbase, idx offset);
            void resize(idx addSize);

            idx mergeClones(real cutoff);
            idx killClones(real cutoff, bool keepOneAlive);
            idx computeStep(real cutoff, idx minOveral);

            void shiftFrames(idx moveTo);
            idx extractCons(sFrameConsensus * cloneCons, idx POS, bool toEnd = false);

            idx getSingleFingPrint(sBioseqpopul::clonePat * p, idx parentClID, idx childID, real fuzzy_coverages_threshold, idx * start = 0, idx * end = 0);
            idx getFingPrints(sVec<sBioseqpopul::clonePat> * patterns, idx * relativeTo, real fuzzy_coverages_threshold);

            idx getLastDiff(sClone * w1, sClone * w2, idx start, idx end ,real cutoff);
            idx getFirstDiff(sClone * w1, sClone * w2, idx start, idx end ,real cutoff);

            idx getfirst()
            {
                sClone * wk = getMainClone();
                idx t = -1;
                if( wk && wk->dim() ) {
                    t = wk->curSubPOS + wk->offset;
                }
                return t;
            }
            idx getrawfirst()
            {
                sClone * wk = getMainClone();
                idx t = -1;
                if( wk && wk->dim() ) {
                    t = wk->curSubPOS;
                }
                return t;
            }
            idx getlast()
            {
                sClone * wk = getMainClone();
                idx t = -1;
                if( wk && wk->dim() ) {
                    t = wk->curSubPOS + wk->length();
                }
                return t;
            }
            idx getTailfirst()
            {
                sClone * wk = getMainClone();
                idx t = -1;
                if( wk && wk->dim() ) {
                    t = wk->curSubPOS + wk->length();
                }
                return t;
            }
            idx getTailLast()
            {
                sClone * wk = getMainClone();
                idx t = -1;
                if( wk && wk->dim() ) {
                    t = wk->curSubPOS + 2 * wk->length();
                }
                return t;
            }

            //AUX
            const char * diff(idx cl1, idx cl2);
//            void clean(bool full=false){if(full){for(idx i=0;i<_coverage.dim();++i)*_coverage.ptr(i)=0;}   for(idx i=0;i<cnt();++i) ptr(i)->clean();}//initFrames
            idx cntAlive()
            {
                idx ali = 0;
                for(idx i = 0; i < cnt(); ++i) {
                    if( ptr(i)->alive() && !ptr(i)->killed )
                        ++ali;
                }
                return ali;
            }
            idx getMainCloneIdx()
            {
                idx clon = -1;
                for(idx i = 0; i < cnt(); ++i) {
                    if( ptr(i)->alive() ) {
                        clon = i;
                        break;
                    }
                }
                return clon;
            }
            sClone * getMainClone()
            {
                return getMainCloneIdx()>=0?ptr(getMainCloneIdx()):0;
            }

            real avCoverage()
            {
                if( avCov ) {
                    return avCov;
                }
                idx ret = 0, cov_pos = 0;
                for(idx i = 0; i < _coverage.dim(); ++i) {
                    if(_coverage[i]) {
                        ret += _coverage[i];
                        ++cov_pos;
                    }
                }
                avCov = (real) ret / cov_pos;
                return avCov;
            }
            void resetAvCoverage(void)
            {
                avCov = 0;
                for(idx i = 0; i < cnt(); ++i) {
                    if( ptr(i)->alive() && !ptr(i)->killed )
                        ptr(i)->avCov = 0;
                }
            }

            void killall(void)
            {
                for(idx i = 0; i < cnt(); ++i) {
                    if( ptr(i)->alive() )
                        ptr(i)->killed = true;
                }
            }

            static idx getAlBoundaries(sBioal * bioal, idx * alList, idx posStart, idx posEnd, idx &alStart, idx &alEnd, idx alCnt);

    };
}

#endif // sBio_seqpopul_hpp

