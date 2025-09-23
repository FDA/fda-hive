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


namespace slib {

    class sBioseqpopul
    {

        public:
            typedef enum eBaseCalls {
                baseNone = -1,
                baseFirst = 0,
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
                idx base[8];
                idx basecall;
                idx coverage;
                sDic<idx> referenceSimil;
                void addSim(idx ref_ind, idx cnt) {
                    static sStr bbuf;
                    bbuf.cut0cut();
                    bbuf.printf("%" DEC ,ref_ind);bbuf.add0();
                    if( !referenceSimil(bbuf.ptr()) )
                        referenceSimil[bbuf.ptr()] = cnt;
                    else
                        referenceSimil[bbuf.ptr()] += cnt;
                }
                void mergeSim(position & o) {
                    const char * cr_id = 0;
                    idx slen = 0;
                    for(idx i = 0 ; i < o.referenceSimil.dim() ; ++i) {
                        cr_id =  (const char *)o.referenceSimil.id(i,&slen);
                        if( !referenceSimil.get(cr_id,slen) )
                            *referenceSimil.set(cr_id,slen) = o.referenceSimil[i];
                        else
                            *referenceSimil.set(cr_id,slen) += o.referenceSimil[i];
                        o.referenceSimil[i] = 0;
                    }
                }
                void addInsSim(idx ref_ind,const char *  ins, idx cnt) {
                    static sStr bbuf;
                    bbuf.cut0cut();
                    bbuf.printf("%" DEC "-ins-%s",ref_ind,ins);bbuf.add0();
                    if( !referenceSimil(bbuf.ptr()) )
                        referenceSimil[bbuf.ptr()] = cnt;
                    else
                    referenceSimil[bbuf.ptr()] += cnt;
                }
                position()
                {
                    init();
                }
                inline idx getACGTCoverage(bool withAmbiguities)
                {
                    idx res = getBaseCoverage(baseA) + getBaseCoverage(baseC) + getBaseCoverage(baseG) + getBaseCoverage(baseT);
                    if( withAmbiguities ) res += getBaseCoverage(baseN);
                    if( res < 0 ) res = 0;
                    return res;
                }
                inline idx getCoverage(bool withAmbiguities)
                {
                    idx res = coverage;
                    if( !withAmbiguities ) res -= base[baseN];
                    if( res < 0 ) res = 0;
                    return res;
                }
                inline bool isSupportedInsertion(idx * compare, real cutoff, idx minCov, idx * cov = 0 ) {
                    return isCoverageSupported(cov?*cov:base[baseIns],compare,cutoff,minCov);
                }

                inline bool isInsertionPhased(idx call, idx * compare, real cutoff, idx minCov, idx * cov = 0, bool isConfirmed = false) {
                    idx cur_cov = (cov?*cov:base[baseIns]), closest_call, min_dist = sIdxMax, cur_dist = 0;
                    for(idx k = 0; k < baseIns; ++k) {
                        cur_dist = sAbs(base[baseIns] - base[k]);
                        if( cur_dist < min_dist ) {
                            min_dist = cur_dist;
                            closest_call = k;
                        }
                    }
                    if(closest_call!=call)
                        return false;
                    if(!isConfirmed)
                        return !isCoverageBifurcating(sAbs(base[call] - cur_cov),compare,cutoff,minCov);
                    return (((real)cur_cov)/coverage) >= cutoff;
                }

                inline bool isCoverageBifurcating(idx cov, idx * compare, real cutoff, idx minCov) {
                    idx minResCov = sMin(cov, (coverage-cov));
                    real freq = (real) minResCov / (compare ? (*compare) : coverage);

                    return ( (freq > cutoff) && minResCov >= minCov );
                }
                inline bool isCoverageSupported(idx cov, idx * compare, real cutoff, idx minCov) {
                    real freq = (real) cov / (compare ? (*compare) : coverage);

                    return ( (freq > cutoff) && cov >= minCov );
                }
                inline idx getBaseCoverage(baseCalls b){
                    if( base[b] < 0 )
                        return 0;
                    return base[b];
                }
                inline idx getBasecallCoverage(){
                    return base[basecall];
                }
                inline real getBasecallConfidence(){
                    return getBaseConfidence((baseCalls)basecall);
                }
                inline real getBaseConfidence(baseCalls cbase){
                    return (real)getBaseCoverage(cbase)/getCoverage(false);
                }
                inline real getLikeability(void){
                    if(getCoverage(false)==1)return 0;
                    return (real)((getBaseCoverage(baseA)*getBaseCoverage(baseC)) + (getBaseCoverage(baseA)*getBaseCoverage(baseG)) + (getBaseCoverage(baseA)*getBaseCoverage(baseT))
                        + (getBaseCoverage(baseA)*getBaseCoverage(baseDel)) + (getBaseCoverage(baseA)*getBaseCoverage(baseGap))
                        + (getBaseCoverage(baseC)*getBaseCoverage(baseG)) + (getBaseCoverage(baseC)*getBaseCoverage(baseT))
                        + (getBaseCoverage(baseC)*getBaseCoverage(baseDel)) + (getBaseCoverage(baseC)*getBaseCoverage(baseGap))
                        + (getBaseCoverage(baseG)*getBaseCoverage(baseT)) + (getBaseCoverage(baseG)*getBaseCoverage(baseDel)) + (getBaseCoverage(baseG)*getBaseCoverage(baseGap))
                        + (getBaseCoverage(baseT)*getBaseCoverage(baseDel)) + (getBaseCoverage(baseG)*getBaseCoverage(baseGap))
                        + (getBaseCoverage(baseDel)*getBaseCoverage(baseGap)))/(getCoverage(false)*(getCoverage(false)-1));
                }
                inline idx setbasecall()
                {
                    idx max = 0; baseCalls maxI = baseNone;
                    for(idx i = baseFirst; i <= baseLast; ++i) {
                        if( i == baseIns || i == baseN ) continue;
                        if( base[i] > max ) {
                            max = base[i];
                            maxI = (baseCalls)i;
                        }
                    }
                    basecall = maxI;
                    if(maxI<0 && base[baseN]>0)
                        basecall = baseN;
                    return maxI;
                }
                inline char basechar() {
                    if(basecall>baseNone && basecall<=baseT)
                        return (char)sBioseq::mapRevATGC[basecall];
                    if(isN())
                        return 'N';
                    if(basecall==baseDel || basecall==baseGap)
                        return '-';
                    return 'I';
                }
                inline bool isGap(real thrshld)
                {
                    return ( ((real)getBaseCoverage(baseGap)/getCoverage(true)) >= thrshld );
                }
                inline bool isN()
                {
                    return (base[basecall] < base[baseN]);

                }
                void init()
                {
                    for(idx j = 0; j < sDim(base); ++j) {
                        base[j] = 0;
                    }
                    referenceSimil.mex()->flags |= sMex::fSetZero;
                    referenceSimil.empty();
                    coverage = 0;
                    basecall = baseNone;
                }
                void copy(position & p)
                {
                    coverage = p.coverage;
                    basecall = p.basecall;
                    if(p.referenceSimil.dim()) {
                        referenceSimil.borrow(&p.referenceSimil);
                    }
                    for(idx l = 0; l < sDim(base); ++l) {
                        base[l] = p.base[l];
                    }
                }
                inline bool add(idx rpts, baseCalls letter, idx ref_sim_ind)
                {
                    if( letter > baseLast || letter < baseFirst ) {
                        return false;
                    }
                    base[letter] += rpts;
                    if( base[letter] < 0 ) {
                        base[letter] -= rpts;
                        if(letter!=baseIns)
                            coverage -= rpts;
                    }
                    if(letter!=baseIns) {
                        coverage += rpts;
                        addSim(ref_sim_ind,rpts);
                        if(letter!=baseN) {
                            if( basecall<=baseNone )
                                basecall=letter;
                            else if( (basecall != letter && rpts>0) || (basecall == letter && rpts<0) )
                                setbasecall();
                        }
                    }
                    return true;
                }
                void transfer(position &p)
                {
                    coverage += p.coverage;
                    p.coverage = 0;
                    for(idx l = 0; l < sDim(base) ; ++l) {
                        base[l] += p.base[l];
                        p.base[l] = 0;
                    }
                    setbasecall();
                    p.basecall = baseNone;
                    mergeSim(p);
                }
                inline void remove(idx rpts, baseCalls letter, idx ref_sim_ind)
                {
                    add(-rpts, letter, ref_sim_ind);
                }

                inline bool isDiff(idx alBase, real l_cutoff) {
                    if( alBase == baseN || isN() || (basecall == alBase) )
                        return false;
                    return getBaseConfidence((sBioseqpopul::baseCalls) alBase) < l_cutoff;
                }

                inline bool isDiff(position * cmp)
                {
                    if ( isN() || cmp->isN() ) {
                        return false;
                    }
                    return basecall!=cmp->basecall;
                }

                bool isBifurcatingPosition(real cutoff,idx minCov, idx * firstInd = 0, idx * secInd = 0, idx * compare = 0)
                {
                    idx max = 0, secondmax = 0;
                    for(idx k = 0; k < baseIns; ++k) {
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
                    idx minResCov = sMin(secondmax, (coverage-secondmax));
                    real freq = (real) minResCov / (compare ? (*compare) : coverage);

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

                    clonePosition(){
                        baseCov = 0;
                    }
                    inline idx base(void)
                    {
                        return (baseCov >> 60) &0xF;
                    }
                    inline bool isInsertion(void)
                    {
                        return (baseCov >> 60) < 0;
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

                    inline bool isDeletion()
                    {
                        return ( base() == baseDel);
                    }


                    inline bool isAnyGap()
                    {
                        return isMultiGap() || isDeletion();
                    }

                    inline void setPosition(position * p)
                    {
                        setBase(p->basecall);
                        setCoverage(p->getCoverage(true));
                    }
                    inline void setGap (position * p)
                    {
                        setBase(-1);
                        setCoverage( p->getBaseCoverage(baseGap) );
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
                    inline bool isDiverging(void) {
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
                    sVec<idx> bases;
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

            static const char * convertBaseCallVector2String(sVec<idx> & basecalls, sStr & buf) {
                return convertBaseCallArr2String(basecalls.ptr(),basecalls.dim(),buf);
            }
            static const char * convertBaseCallArr2String(idx * basecalls, idx basecnt, sStr & buf) {
                buf.cut0cut();
                for(idx i = 0 ; i < basecnt ; ++i ) {
                    buf.printf("%c", sBioseq::mapRevATGC[basecalls[i]]);
                }
                return buf.ptr();
            }
            static idx convertString2BaseCallVector(const char *  basecallstr, sVec<idx> & basecalls, idx slen = 0) {
                idx len = slen?slen:sLen(basecallstr);
                for(idx i = 0 ; i < len ; ++i ) {
                    basecalls.vadd(1, sBioseq::mapATGC[(idx)basecallstr[i]]);
                }
                return basecalls.dim();
            }

            static bool isDiffBases (idx * bases1, idx bcnt1, idx * bases2, idx bcnt2) {
                if( bcnt1 != bcnt2)
                    return true;
                idx i = 0;
                for( ; i < bcnt1 && (bases1[i]==bases2[i]) ; ++i ) {
                    if(bases1[i]==baseN || bases2[i]==baseN)return false;
                }
                return i != bcnt1;
            }

        private:
    };

    template<class Tobj> class sWdw
    {
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

            virtual void init(idx size)
            {
                _vec.add(size);
                curMatPOS = 0;
                mergeMatPOS = 0;
                extrPos = 0;
            }

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
                mergingScore = 0;
                mergingCl = -1;
                mergedToCl = -1;
                lastSubPOS = -1;
                avCov = 0;
            }
            void init(sClone * newCl, idx t_clID);
            idx offset;
            idx clID, parentClID;
            idx startSubPOS;
            idx curSubPOS;

            idx ref_cnt;


            idx mergedToCl, mergingCl, mergingScore;
            idx lastSubPOS;
            real avCov;

            bool merged, merging, killed, dead;


            idx supportCnt;
            idx bifurcation_pos;
            sVec<sBioseqpopul::points> prevBif;
            void registerBifurcation(idx pos, sVec<idx> new_bases) {
                sBioseqpopul::points * cur_bif = prevBif.add();
                cur_bif->position = pos;
                cur_bif->bases.copy( &new_bases );
            }
            bool wasBifurcationConsidered(idx new_pos, sVec<idx> &new_bases) {
                for(idx i = 0 ; i < prevBif.dim() ; ++i) {
                    sBioseqpopul::points * cur_bif = prevBif.ptr(i);
                    if( cur_bif->position == new_pos && !sBioseqpopul::isDiffBases( cur_bif->bases.ptr(), cur_bif->bases.dim(), new_bases.ptr(), new_bases.dim() ) ) {
                        return true;
                    }
                }
                if(prevBif.dim() && prevBif.last()->position < new_pos) {
                    prevBif.cut(0);
                }
                return false;
            }
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

            bool valid()
            {
                return !(killed || dead || merged || (length() + curSubPOS < startSubPOS));
            }

            bool active()
            {
                return !dead && !(merged && lastSubPOS < 0);
            }


            bool hasParent()
            {
                return (parentClID >=0 && parentClID!=clID);
            }

            bool createdWithinFrame( idx frameCnt = 0)
            {
                return startSubPOS >= curSubPOS - (frameCnt*length());
            }

            real avCoverage() {
                if( avCov ) {
                    return avCov;
                }
                idx ret = 0, cov_pos = 0;
                for(idx i = 0; i < dim(); ++i) {
                    if( ptr(i)->getCoverage(true) ){
                        ret += ptr(i)->getCoverage(true);
                        ++cov_pos;
                    }
                }
                if(!ret) return 0;
                avCov = (real) ret / cov_pos;
                return avCov;
            }

            bool isGap(idx i, real thrshld) {
                if( !avCoverage() ) return false;
                return (real)pos(i)->getACGTCoverage(true)/avCov < thrshld;
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

            void strech(idx addSize);

            virtual void mergeTo(sClone * merged, idx pos);

            void markAsDead() {
                lastSubPOS = -10;
                mergeMatPOS = getrawfirst();
            };

            inline idx getfirstNonZero(idx * start = 0, idx * end = 0)
            {
                idx st = start ? *start : getrawfirst(), ed = end ? *end : getTailLast();
                while( !(pos(st) && pos(st)->getCoverage(true)) && st < ed )
                    ++st;
                return st;
            }

            inline idx getlastNonZero(idx * start = 0, idx * end = 0)
            {
                idx st = start ? *start : getrawfirst(), ed = end ? *end : getTailLast();
                --ed;
                if( merged && lastSubPOS < ed )
                    ed = lastSubPOS;
                while( !(pos(ed) && pos(ed)->getCoverage(true)) && ed > st )
                    --ed;
                return ed + 1;
            }

            inline void addL(idx isx, idx rpts, sBioseqpopul::baseCalls letter, idx ref_sim_ind){
                pos(isx)->add(rpts,letter,_report_ref?ref_sim_ind:0);
            }
            inline void removeL(idx isx, idx rpts, sBioseqpopul::baseCalls letter, idx ref_sim_ind){
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

            inline idx getSummaryEnd() {
                if(summary.end<0){
                    return summary.start+seqCov.dim();
                }
                return summary.end;
            }
            sBioseqpopul::clonePosition *  getSubPos(idx pos);
            inline idx getLastSupportedPosition(idx pos = 0) {
                idx my_start = summary.start;
                if(pos <= 0)
                    pos = seqCov.dim() - 1 + my_start;
                idx res = pos;
                while ( res >= my_start && getSubPos(res)->isMultiGap() ) {
                    --res;
                }
                if( res <= my_start ) {
                    return -1;
                }
                return res;
            }
            inline idx getFirstSupportedPosition(idx pos = 0 ) {
                idx my_start = summary.start;
                if(pos <= 0)
                    pos = my_start;
                idx res = pos;
                while (res < seqCov.dim() + my_start && getSubPos(res)->isMultiGap() ) {
                    ++res;
                }
                if( res >= seqCov.dim() + my_start ) {
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


    };

    class sFrameConsensus
    {
        private:
            sVec<sCloneConsensus> _vec;
            sVec<sVec<sMex::Pos> > _frame_shifts;
            sVec<sMex::Pos> _common_coordinates;
            sStr _simil_id_buf;
        public:
            typedef idx (*callbackType)(void * param, idx countDone, idx progressCur, idx percentMax);
            callbackType progress_CallbackFunction;
            void * progress_CallbackParam;

            idx sub_start, sub_end, simil_size;
            sFrameConsensus(idx t_simil_size) {
                sub_start = sub_end = 0;
                progress_CallbackFunction = 0; progress_CallbackParam = 0;
                simil_size = t_simil_size;
            }

            sDic<idx> similarities;

            idx dim(){return _vec.dim();};

            sCloneConsensus * ptr(idx i = 0 ) {return _vec.ptr(i);};
            sCloneConsensus * add( idx i = 0 ){return _vec.add();};
            sCloneConsensus * ptrx( idx i = 0 ){_frame_shifts.resize(i+1); return _vec.ptrx(i);};

            bool reorderClones(void);
            idx mergeAll(void);
            idx trimClones(void);
            void getCloneStats(sBioseqpopul::cloneStats * clSum);
            sCloneConsensus * getParentOnLocation(idx iCl,idx pos);
            sCloneConsensus * getMergedOnLocation(idx iCl,idx pos);
            idx getClonesPositionsOnLocation(idx pos, sVec<idx> &cloneIDs, sVec<sBioseqpopul::clonePosition> &ret );
            idx getFirstMutation(idx iCl, idx pos = 0, bool ignoreGaps = false);
            idx getLastMutation(idx iCl, idx pos = 0 , bool ignoreGaps = false);
            idx remapSimilarities(sDic<idx> *sim, idx icl, idx pos, idx remapped_icl);
            void remapSimilarities( sVec<idx> &icl,sVec<idx> &sorted, idx pos);
            const char * contructSimilaritiesID ( idx icl, idx pos, const char * reference );

            static idx sortPositionsComparator(void * parameters, void * arr, idx i1, idx i2);

            idx common_coord_pos(idx pos) {
                return frame_pos(-1,pos);
            }
            idx get_common_coord_ins_size(idx pos) {
                for(idx i = 0 ; i < _common_coordinates.dim() ; ++i ) {
                    if( _common_coordinates[i].pos == pos )
                        return _common_coordinates[i].size;
                    if( _common_coordinates[i].pos > pos )
                        return 0;
                }
                return 0;
            }
            idx frame_pos( idx icl, idx pos){
                sVec< sMex::Pos > * cr_frm_shifts = &_common_coordinates;
                if(icl>=0)
                    cr_frm_shifts = _frame_shifts.ptrx(icl);
                idx sub_start = (icl>=0)?ptr(icl)->summary.start:0;
                if(cr_frm_shifts->dim()==0)return sub_start+pos;
                idx i = 0, tot_ins = 0;
                while( i < cr_frm_shifts->dim() && pos > cr_frm_shifts->ptr(i)->pos){
                    tot_ins += cr_frm_shifts->ptr(i)->size;
                    ++i;
                }
                if(i==0) return sub_start+pos;
                return sub_start + pos + tot_ins;
            };
            bool isLink(sCloneConsensus * cl) {
                if ( !cl->summary.hasParent() || !cl->summary.hasMerged() || !cl->summary.isDiverging() ){
                    return false;
                }
                sCloneConsensus * pCl = getParentOnLocation( cl->summary.clID, cl->summary.start );
                sCloneConsensus * mCl = getMergedOnLocation( cl->summary.clID, cl->getSummaryEnd() );
                if (mCl || pCl) {
                    return false;
                }
                if( pCl->summary.end > cl->summary.end && mCl->summary.start < cl->summary.start )
                    return true;
                return false;
            }
            void register_frameshift(idx icl , idx pos, idx size) {
                sVec< sMex::Pos > * cr_frm_shifts = _frame_shifts.ptrx(icl);

                sMex::Pos * frmshift = cr_frm_shifts->add();
                frmshift->pos = pos;
                frmshift->size = size;
                idx i = 0;
                for( ; i < _common_coordinates.dim() ; ++i) {
                    if(_common_coordinates[i].pos > pos)
                        break;
                    else if ( _common_coordinates.ptr(i)->pos == pos) {
                        if ( _common_coordinates.ptr(i)->size < size ) {
                            _common_coordinates.ptr(i)->size = size;
                            i = -1;
                            break;
                        }
                    }
                }
                if(i>=0) {
                    frmshift = _common_coordinates.insert(i,1);
                    frmshift->pos = pos;
                    frmshift->size = size;
                }
            }
            bool mergePositions(sCloneConsensus * dstCl, sCloneConsensus * srcCl, idx frmStart = 0, idx frmEnd = 0, bool adjustForGaps = false);

            idx getSimilarities(idx cloneID, idx framePos, sBioseqpopul::position * p );
            idx getInsSimilarities(idx cloneID, idx framePos, sBioseqpopul::position * p, const char * ins );

            idx getCloneSupport(idx i);

            void setCloneSupport(void) {
                for(idx i = 0; i < dim(); ++i) {
                    ptr(i)->stats.support = getCloneSupport(i);
                }
            }

            void updateDependencies(sCloneConsensus * oldCl, sCloneConsensus * newCl) {
                if( oldCl && newCl && oldCl!=newCl ) {
                    for( idx j = 0 ; j < dim(); ++j ) {
                        sCloneConsensus * chCl = ptr(j);
                        if( j > oldCl->summary.clID ) {
                            if( chCl->summary.hasParent() && chCl->summary.parentClID == oldCl->summary.clID )
                                chCl->summary.parentClID = newCl->summary.clID;
                        }
                        if(chCl->summary.hasMerged() && chCl->summary.mergeclID == oldCl->summary.clID)
                            chCl->summary.mergeclID = newCl->summary.clID;
                    }
                }
            }

            void fixParentDependencies(sCloneConsensus * cl) {
                if( cl ) {
                    for( idx j = 0 ; j < dim(); ++j ) {
                        sCloneConsensus * chCl = ptr(j);
                        if( j > cl->summary.clID && chCl->summary.hasParent() && chCl->summary.parentClID == cl->summary.clID && chCl->summary.start < cl->summary.start ) {
                            sCloneConsensus * pCl = getParentOnLocation( cl->summary.clID, chCl->summary.start );
                            chCl->summary.parentClID = pCl ? pCl->summary.clID : chCl->summary.clID;
                        }
                        if( chCl->summary.hasMerged() && chCl->summary.mergeclID == cl->summary.clID && chCl->getSummaryEnd() < cl->summary.start ) {
                            sCloneConsensus * pCl = getParentOnLocation( cl->summary.clID, chCl->getSummaryEnd() );
                            chCl->summary.mergeclID = pCl ? pCl->summary.clID : chCl->summary.clID;
                        }
                    }
                }
            }
            void fixMergedDependencies(sCloneConsensus * cl) {
                if( cl ) {
                    for( idx j = 0 ; j < dim(); ++j ) {
                        sCloneConsensus * chCl = ptr(j);
                        if( j > cl->summary.clID && chCl->summary.hasParent() && chCl->summary.parentClID == cl->summary.clID && chCl->summary.start > cl->getSummaryEnd() ) {
                            sCloneConsensus * mCl = getMergedOnLocation(cl->summary.clID, chCl->summary.start);
                            chCl->summary.parentClID = mCl ? mCl->summary.clID : chCl->summary.clID;
                        }
                        if( chCl->summary.hasMerged() && chCl->summary.mergeclID == cl->summary.clID && chCl->getSummaryEnd() > cl->getSummaryEnd() ) {
                            sCloneConsensus * mCl = getMergedOnLocation(cl->summary.clID, chCl->getSummaryEnd());
                            chCl->summary.mergeclID = mCl ? mCl->summary.clID : chCl->summary.clID;
                        }
                    }
                }
            }
            bool setConsensusStat(sBioseqpopul::cloneStats &clSum);

            bool areDifferent(sCloneConsensus * cl1, sCloneConsensus * cl2, idx start = 0 , idx end = 0);

            idx cleanOutClones(sVec<idx> &cloneIndex);
            idx sOutSimilarities(const char * flnm);
            idx createFrameMaps( const char * skp2gps, const char * gps2skp, const char * skp2gpsSpprt, const char * gps2skpSpprt);
    };

    class sFrame
    {
        private:
            sVec<sClone> _clones;
            sVec<idx> _cloneInds;
            real avCov;
            sDic< sDic <idx> > _all_frame_insertions;
        protected:
            real gap_thrshld,_cutoff;
            sDic<idx> * getInsertionsDic(idx icl, idx pos) {
                static sStr cr_buf;
                cr_buf.cut0cut();
                cr_buf.printf(0,"%" DEC "-%" DEC ,icl,pos);
                cr_buf.add0();
                return _all_frame_insertions.get(cr_buf.ptr());
            }
            sDic<idx> & setInsertionsDic(idx icl, idx pos) {
                static sStr cr_buf;
                cr_buf.cut0cut();
                cr_buf.printf("%" DEC "-%" DEC ,icl,pos);
                cr_buf.add0();
                return  _all_frame_insertions[cr_buf.ptr()];
            }
        public:
            idx minBifCov;
            sWdw<idx> _coverage;
            idx ref_cnt;

            sFrame(idx t_ref_cnt, real t_gap_thrshld, real t_cutoff, idx t_minBifCov)
            {
                _coverage.flagOn(sMex::fSetZero);
                avCov = 0;
                ref_cnt = t_ref_cnt;
                gap_thrshld = t_gap_thrshld;
                _cutoff = t_cutoff;
                minBifCov = t_minBifCov;
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

            void addInsertion(idx cloneID, idx subpos, sVec<idx> & basecalls, idx rpts, idx subID = -1) {
                sDic<idx> & _insertions =  setInsertionsDic(cloneID,subpos);
                static sStr cr_buf;
                cr_buf.cut0cut(0);
                const char * insertion_call = sBioseqpopul::convertBaseCallVector2String(basecalls, cr_buf);
                idx * in_rpts = _insertions.get(insertion_call);
                if ( !in_rpts )
                    *_insertions.set(insertion_call) = rpts;
                else
                    *in_rpts += rpts;
                if(subID>=0)
                    ptr(cloneID)->pos(subpos)->addInsSim(subID,insertion_call,rpts);
            }

            void removeInsertion(idx cloneID, idx subpos, sVec<idx> & basecalls, idx rpts, idx subID = -1) {
                addInsertion(cloneID,subpos,basecalls,-rpts, subID);
            }

            idx getPhasedInsertionStr(sClone * w, idx ip, real cutoff, idx minBifCov, sStr &s_insertions, bool isConfirmed = false) {
                sBioseqpopul::position * p = w->pos(ip);
                if( p->isSupportedInsertion(0,cutoff,minBifCov) && p->isInsertionPhased(p->basecall,_coverage.pos(ip-w->curSubPOS),cutoff,minBifCov,0,true) ) {
                    idx ins_cov = 0, slen;
                    const char * cr_ins = getDominantInsertionChar(w->clID,ip,slen,ins_cov);
                    if(!cr_ins || !p->isSupportedInsertion(0,cutoff,minBifCov,&ins_cov) || !p->isInsertionPhased(p->basecall,_coverage.pos(ip-w->curSubPOS),cutoff,minBifCov,&ins_cov, isConfirmed) )
                        return 0;
                    s_insertions.cutAddString(0,cr_ins,slen);
                    return ins_cov;
                }
                return 0;
            }

            sClone * getValidClone(idx ind, idx * start = 0 , idx * mergedPos = 0) {
                sClone * w = ptr(ind);
                if(!w || w->killed) return 0;
                while(w->merged){
                    if(start && mergedPos){
                        if(w->lastSubPOS>=*start || w->lastSubPOS<0)
                            *mergedPos=w->lastSubPOS;
                    }
                    w=ptr(w->mergedToCl);
                }
                if(!w || w->killed) return 0;
                return w;
            }

            bool getPhasedInsertionVec(sClone * w, idx ip, real cutoff, idx minBifCov, sVec<idx> &insertions, bool isConfirmed = false) {
                static sStr c_buf;
                c_buf.cut0cut(0);
                idx ins_cov = getPhasedInsertionStr(w,ip,cutoff,minBifCov,c_buf,isConfirmed);
                if(!ins_cov)
                    return 0;
                sBioseqpopul::convertString2BaseCallVector(c_buf,insertions,c_buf.length());
                return ins_cov;
            }

            idx getPositionInsertionBasecalls(sClone * cl, idx ip, real cutoff, idx minBifCov, sVec<idx> & basecalls, idx * ins_cov = 0) {
                sBioseqpopul::position * p = cl->pos(ip);
                basecalls.cut(0);

                basecalls.vadd(1,p->basecall);
                idx l_ins_cov = getPhasedInsertionVec(cl, ip, cutoff, minBifCov,basecalls);
                if(ins_cov)*ins_cov = l_ins_cov;
                return basecalls.dim();
            }

            bool isDiffPositionInsertionBasecalls(sClone * cl1, sClone * cl2, idx ip, real cutoff, idx minBifCov) {
                static sVec<idx> basecalls1;
                static sVec<idx> basecalls2;
                basecalls1.cut(0);basecalls2.cut(0);
                sBioseqpopul::position * p1 = cl1->pos(ip), * p2 = cl2->pos(ip);
                getPositionInsertionBasecalls(cl1,ip, cutoff,minBifCov, basecalls1);
                getPositionInsertionBasecalls(cl2,ip, cutoff,minBifCov, basecalls2);
                if( p1->isN() || p2->isN() )
                    return false;
                return sBioseqpopul::isDiffBases(basecalls1.ptr(),basecalls1.dim(),basecalls2.ptr(),basecalls2.dim());
            }

            bool isDiffPositionInsertion(sClone * cl1, sClone * cl2, idx ip ,real cutoff, idx minBifCov) {
                static sVec<idx> basecalls;
                basecalls.cut(0);
                sBioseqpopul::position * p = cl1->pos(ip);
                getPositionInsertionBasecalls(cl1,ip, cutoff,minBifCov, basecalls);
                if( !p->isN() && areBasecallsThere(cl2,ip,basecalls,cutoff,minBifCov) )
                    return false;
                basecalls.cut(0);
                p = cl2->pos(ip);
                getPositionInsertionBasecalls(cl2,ip, cutoff,minBifCov, basecalls);
                if(  !p->isN() && areBasecallsThere(cl1,ip,basecalls,cutoff,minBifCov) )
                    return false;

                if( cl1->pos(ip)->isN() && cl1->pos(ip)->isN() )
                    return false;
                return true;
            }

            bool areBasecallsThere(sClone *cl, idx ip, sVec<idx> &basecalls, real cutoff, idx minBifCov) {
                sBioseqpopul::position * p = cl->pos(ip);
                if(!basecalls.dim()) {
                    if(!p->getCoverage(false))
                        return true;
                    return false;
                }
                idx c_base = basecalls[0];
                if( ((real)p->getBaseCoverage(static_cast<sBioseqpopul::baseCalls>(c_base))/ p->getCoverage(false)) < cutoff)
                    return false;
                if( basecalls.dim() > 1 ) {
                    idx ins_cov = isInsertionInPos(cl->clID, ip, basecalls.ptr(1), basecalls.dim()-1);
                    if(((real)ins_cov/p->getCoverage(false))<cutoff)
                        return false;
                }
                return true;
            }

            idx getDominantInsertion(idx cloneID, idx subpos, idx &max_cnt) {
                sDic<idx> * _insertions =  getInsertionsDic(cloneID,subpos);
                if(!_insertions)
                    return 0;
                sStr id_buf;
                idx max_i = -1;
                max_cnt = 0;
                for(idx i = 0 ; i < _insertions->dim() ; ++i ) {
                    if( max_cnt < *_insertions->ptr(i) ) {
                        max_cnt = *_insertions->ptr(i);
                        max_i = i;
                    }
                }
                return max_i;
            }

            idx getDominantInsertion(idx cloneID, idx subpos, sVec<idx> & basecalls, idx &cnt) {
                idx slen = 0;
                const char * cr_ins = getDominantInsertionChar(cloneID, subpos, slen, cnt);
                if(!cr_ins)return 0;
                sBioseqpopul::convertString2BaseCallVector(cr_ins,basecalls,slen);
                return basecalls.dim();
            }
            const char * getDominantInsertionChar(idx cloneID, idx subpos, idx &slen, idx &cnt) {
                sDic<idx> * _insertions =  getInsertionsDic(cloneID,subpos);
                if(!_insertions)
                    return 0;
                sStr id_buf;
                idx max_cnt = 0, max_i = 0;
                for(idx i = 0 ; i < _insertions->dim() ; ++i ) {
                    if( max_cnt < *_insertions->ptr(i) ) {
                        max_cnt = *_insertions->ptr(i);
                        max_i = i;
                    }
                }
                cnt = max_cnt;
                return (const char *)_insertions->id(max_i, &slen);
            }

            idx isInsertionInPos(idx cloneID, idx subpos, idx * basecalls, idx basecnt) {
                static sStr cr_buf;
                sDic<idx> * _insertions =  getInsertionsDic(cloneID,subpos);
                if(!_insertions)
                    return 0;
                cr_buf.cut0cut();
                const char * insertion_call = sBioseqpopul::convertBaseCallArr2String(basecalls, basecnt, cr_buf);
                idx * in_rpts = _insertions->get(insertion_call);
                if ( !in_rpts )
                    return 0;
                return *in_rpts;
            }

            idx getDominantInsertionCov(idx cloneID, idx subpos) {
                sDic<idx> * _insertions =  getInsertionsDic(cloneID,subpos);
                if(!_insertions)
                    return 0;
                sStr id_buf;
                idx max_cnt = 0;
                for(idx i = 0 ; i < _insertions->dim() ; ++i ) {
                    if( max_cnt < *_insertions->ptr(i) ) {
                        max_cnt = *_insertions->ptr(i);
                    }
                }
                return max_cnt;
            }

            idx getSecondDominantInsertionCov(idx cloneID, idx subpos) {
                sDic<idx> * _insertions =  getInsertionsDic(cloneID,subpos);
                if(!_insertions)
                    return 0;
                sStr id_buf;
                idx max_cnt = 0,second_cnt = 0, cr_cnt = 0;
                for(idx i = 0 ; i < _insertions->dim() ; ++i ) {
                    cr_cnt = *_insertions->ptr(i);
                    if( max_cnt < cr_cnt ) {
                        second_cnt = max_cnt;
                        max_cnt = cr_cnt;
                    }
                    if( second_cnt < cr_cnt && cr_cnt < max_cnt )
                        second_cnt = cr_cnt;
                }
                return second_cnt;
            }
            idx getSecondDominantInsertion(idx cloneID, idx subpos, sVec<idx> &basecalls, idx &cnt) {
                sDic<idx> * _insertions =  getInsertionsDic(cloneID,subpos);
                if(!_insertions)
                    return 0;
                sStr id_buf;
                idx max_cnt = 0,second_cnt = 0, sec_ind = 0,  cr_cnt = 0;
                for(idx i = 0 ; i < _insertions->dim() ; ++i ) {
                    cr_cnt = *_insertions->ptr(i);
                    if( max_cnt < cr_cnt ) {
                        second_cnt = max_cnt;
                        max_cnt = cr_cnt;
                    }
                    if( second_cnt < cr_cnt && cr_cnt < max_cnt ) {
                        second_cnt = cr_cnt;
                        sec_ind = i;
                    }
                }
                cnt = second_cnt;
                idx slen = 0;

                sBioseqpopul::convertString2BaseCallVector((const char *)_insertions->id(sec_ind,&slen),basecalls,slen);
                return basecalls.dim();
            }

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
                    if( tcl->active() ) {
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
                return isFramePositionFuzzy(p, cutoff) || isClonalPositionFuzzy(w, p, cutoff) ;
            }
            inline bool isFramePositionFuzzy( idx p, real cutoff )
            {
                if(!avCoverage())return true;
                return (real)*_coverage.pos(p - getMainClone()->curSubPOS)/avCoverage() < cutoff;
            }
            inline idx getLastNonFuzzyPosition(sClone * w , real cutoff) {
                idx start = w->getfirstNonZero();
                idx end = w->getlastNonZero();
                for(idx i = end ; i>=start ; --i) {
                    if(!isClonalPositionFuzzy(w,i,cutoff))
                        return i;
                }
                return start-1;
            }
            inline bool isClonalPositionFuzzy(sClone * w, idx p, real cutoff)
            {
                if( *_coverage.pos(p - w->curSubPOS) <= 0)return true;
                return ( (real)w->pos(p)->getCoverage(false)/ sMax((real)*_coverage.pos(p-w->curSubPOS), w->avCoverage()) < cutoff);
            }
            inline bool isClonalInsertionFuzzy(sClone * w, idx p, real cutoff, idx &ins_ind)
            {
                if( *_coverage.pos(p - w->curSubPOS) <= 0)return true;
                idx comb_cov = sMax((real)*_coverage.pos(p-w->curSubPOS), w->avCoverage());
                if ( ( (real)w->pos(p)->getBaseCoverage(sBioseqpopul::baseIns)/ comb_cov < cutoff) )
                    return true;
                idx ins_cnt;
                ins_ind = getDominantInsertion(w->clID,p,ins_cnt);
                return ( (real)ins_cnt/ comb_cov < cutoff);
            }
            void move(idx pos);
            void setCalls(void);
            void correctInsertions(sClone * w);

            sClone * branch(idx prevID, sVec<idx> & setbase, idx offset);
            void resize(idx addSize);

            idx getSingleFingPrint(sBioseqpopul::clonePat * p, idx parentClID, idx childID, real fuzzy_coverages_threshold, idx bif_start, idx * start = 0, idx * end = 0);

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

            void mergeClones(sClone * dst, sClone * src, idx matPos)
            {
                idx start = src->subPos(matPos);
                idx ed = src->getTailLast(), cnt = 0;
                idx slen = 0;
                const char * basecalls = 0;
                sVec<idx> ins_vec;
                for(idx i = start; i < ed; ++i) {
                    if(src->pos(i)->base[sBioseqpopul::baseIns]) {
                        sDic<idx> * _insertions =  getInsertionsDic(src->clID,i);
                        if(!_insertions)
                            continue;
                        for(idx in = 0 ; in < _insertions->dim() ; ++in ) {
                            ins_vec.cut();
                            cnt = *_insertions->ptr(in);
                            basecalls = (const char*)_insertions->id(in,&slen);
                            sBioseqpopul::convertString2BaseCallVector(basecalls,ins_vec,slen);
                            removeInsertion(src->clID,i,ins_vec,cnt);
                            addInsertion(dst->clID,i,ins_vec,cnt);
                            _insertions =  getInsertionsDic(src->clID,i);
                        }
                    }
                }
                src->mergeTo(dst, matPos);
            }

            void mergeCloneRange(sClone * dst, sClone * src, idx subStart, idx subEnd) {
                for(idx i = subStart; i < subEnd; ++i) {
                    dst->pos(i)->transfer(*src->pos(i));
                }
            }

            const char * diff(idx cl1, idx cl2,bool onlydiffs = true);
            idx cntAlive()
            {
                idx ali = 0;
                for(idx i = 0; i < cnt(); ++i) {
                    if( ptr(i)->valid() && !ptr(i)->killed )
                        ++ali;
                }
                return ali;
            }
            idx getMainCloneIdx()
            {
                idx clon = -1;
                for(idx i = 0; i < cnt(); ++i) {
                    if( ptr(i)->valid() ) {
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
                    if( ptr(i)->valid() )
                        ptr(i)->avCov = 0;
                }
            }

            void killall(void)
            {
                for(idx i = 0; i < cnt(); ++i) {
                    if( ptr(i)->valid() )
                        ptr(i)->killed = true;
                }
            }

            static bool getAlBoundaries(sBioal * bioal, idx * alList, idx posStart, idx posEnd, idx &alStart, idx &alEnd, idx alCnt);

    };
}

#endif 
