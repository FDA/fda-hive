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
#ifndef sClones_hpp
#define sClones_hpp

#include <slib/utils.hpp>
#include <ssci/math/clust/clust2.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>

#define DEBUGGING_KILLING
#define DEBUGGING_MERGING

#define RAND1 (real)rand()/RAND_MAX

class sPopul: public sFrame {
        typedef sFrame cParent;
    public:

        struct bifurcation_structure{
                idx _POS,_fatherclID, _childclID,_oldCnt;
                sVec<idx> bases;
                bifurcation_structure(){
                    _POS=_fatherclID=_childclID = _oldCnt = 0;
                }
        };


        sVec<idx> _alClonIds;
        idx bifurcationDecisionFlag;
        bool getBifurcationPvalue,getBifurcationBayes;
        bool cloneContinuity;
        bool pairEnd_mode;
        bool toEnd;
        real noise_tolerance;
        real getBifurcationPvalueThrshld, getBifurcationBayesThrshld;
    private:

        sFrameConsensus * _consensus;

        sVec<real> _buf;
        sStr _strBuf;
        idx refill_mode;

        idx _fingStep,_last_fngprnt_pos;
        idx _lastSupportedPosition;
        idx * _order_indices;
        sBioal * _bioal;idx _bioal_long_cnt;
        idx _alCnt_Max;

        idx t_paired_iAl;


        struct hdrStruct{
                sBioseqAlignment::Al * hdr;
                const char * vqry, * quastring;
                idx * m, qrybuflen, nonCompFlags,iAl,subStart,subEnd;
                hdrStruct(){sSet(this,0);}
        };

        idx setAlCloneIdx(idx Al, idx cl) {
            idx cnt = 1;
            _alClonIds[Al]=cl;
            return cnt;
        }

        idx getAlCloneIdx (idx iAl, idx * p_iAl = 0) {
            idx res = _alClonIds[iAl];
            if(pairEnd_mode && res<0) {
                idx p_cnt = getPairedAlignmentCnt(iAl);
                for( idx ip = 0 ; res<0 && ip<p_cnt; ++ip ){
                    getPairedAlignment(iAl,t_paired_iAl,ip);
                    res=_alClonIds[t_paired_iAl];
                    if(res>=0) {
                        if(p_iAl)*p_iAl = t_paired_iAl;
                        return res;
                    }
                }
            }
            return res;
        }

        bool isPairedInClone(idx iAl, idx dst) {
            if(!pairEnd_mode) return false;
            idx p_cnt = getPairedAlignmentCnt(iAl);
            for(idx ipa = 0 ; ipa < p_cnt ; ++ipa ) {
                getPairedAlignment(iAl,t_paired_iAl,ipa);
                if(_alClonIds[t_paired_iAl]==dst){
                    return true;
                }
            }
            return false;
        }

        idx closestClone(hdrStruct &hdrS, idx prv_cl);
        idx distOfBifByFing(hdrStruct &hdrS, sBioseqpopul::clonePat * chilP);
        idx alToClone(hdrStruct &hdrS,bifurcation_structure &frmS);
        idx selectCl(sVec<idx> &clIDs, bool random) {
            if( !clIDs.dim() ) return -1;
            idx tot_avCov = 0, selected_clID=0;
            sClone * w = 0;
            if(random) {
                selected_clID=clIDs[0];
                for( idx i = 0 ; i < clIDs.dim() ; ++i ) {
                    w = ptr(clIDs[i]);
                    if( tot_avCov <= sRand::random1()*(tot_avCov + w->avCoverage() ) )
                        selected_clID = clIDs[i];
                    tot_avCov += w->avCoverage();
                }
            } else {
                for( idx i = 0 ; i < clIDs.dim() ; ++i ) {
                    w = ptr(clIDs[i]);
                    if( tot_avCov < w->avCoverage() ) {
                        tot_avCov = w->avCoverage();
                        selected_clID = clIDs[i];
                    }
                }
            }
            return selected_clID;
        }
        void _extract(sClone * w, sCloneConsensus * t, idx start, idx extred);

    public:
        idx _iAlEnd,_iAlStart,_iAlPrevStart,_POS, minOverlap,_quality_threshold;
        bifurcation_structure _bfrcS;
        sVec<idx> _pairVec;sVec<idx> _pairInd;

        typedef idx (*callbackType)(void * param, idx countDone, idx progressCur, idx percentMax);
        callbackType progress_CallbackFunction;
        void * progress_CallbackParam;

        real fuzzy_threshold;

        typedef enum eAlCloneDistance {
            eHamming = 0,
            eEuclidean = 1
        } EAlCloneDistance;

        EAlCloneDistance alcl_dist_m;

        sPopul(sBioal * bioal,sFrameConsensus * t_consensus,idx t_ref_cnt, idx fing_step, idx * order_indices,real t_cutoff,real t_gap_thrshld, idx t_pos=0, idx al_cnt = 0, idx t_alStart = 0, idx t_alEnd = 0 ) : sFrame(t_ref_cnt,t_gap_thrshld,t_cutoff,0) {
            _bioal = bioal;
            _fingStep = fing_step;
            _order_indices = order_indices;
            _consensus = t_consensus;
            _alCnt_Max = al_cnt?al_cnt:_bioal->dimAl();
            _POS = t_pos;
            _iAlEnd = t_alEnd;
            _iAlStart = t_alStart;
            _iAlPrevStart = t_alStart;
            t_paired_iAl=0;
            _cutoff = t_cutoff;
            refill_mode = 0;getBifurcationPvalue=false;getBifurcationBayes=false;cloneContinuity=false;
            toEnd = false;
            getBifurcationPvalueThrshld = 1;
            getBifurcationBayesThrshld = 0;
            _last_fngprnt_pos = 0;
            _quality_threshold = 0;
            _alClonIds.add(_alCnt_Max);
            _alClonIds.set(-1);
            _lastSupportedPosition = 0;
            fuzzy_threshold = 0;_bioal_long_cnt = 0;
            progress_CallbackParam = 0;
            progress_CallbackFunction = 0;
            bifurcationDecisionFlag = 0;
            minOverlap = 0;
            pairEnd_mode = 0;
            noise_tolerance = 0.03;
            alcl_dist_m = eEuclidean;
            _buf.init(sMex::fSetZero);
        }
        struct ParamsAllSubjSorter{
            sVec < sBioseqAlignment::Al * > * alSub;
            sVec< sVec <idx> > * alSubLUT;
            sHiveal::ParamsAlignmentSorter sorterParams;
            ParamsAllSubjSorter(){alSub=0;alSubLUT=0;}
        };

idx getPairedAlignment(idx ia, idx & pair_ia, idx ip = 0) {
            idx cnt = getPairedAlignmentCnt(ia);
            if(!cnt) return 0;
            if(ip >= cnt) ip = cnt-1;
            pair_ia = _pairVec[_pairInd[2*ia+1]+ip];
            return cnt;
        }

idx getPairedAlignmentCnt(idx ia) {
            if(!pairEnd_mode) return 0;
            return _pairInd[2*ia];
        }

        idx getAlStart() {
            return (refill_mode==1 || refill_mode ==-1)?_iAlPrevStart:_iAlStart;
        }

        idx getAlindex(idx iAl) {
            if(_order_indices) {
                return _order_indices[iAl];
            }
            return iAl;
        }

        idx extractCons(idx range);

        idx getBifurcationStart(sClone * w, idx i, idx end);

        const char * difAl(idx icl, idx iAl, sStr * out = 0, bool onlydiff = true);
        const char * difAl2(idx icl1, idx icl2, idx iAl, sStr * out = 0, bool onlydiff = true);

        void InitCoordinates(idx start_pos, idx length) {

            SetNextCoordinates(start_pos);
            init(length);
        }

        idx current_pos(){
            sClone * w = getMainClone();
            if(!w)return -1;
            return w->curSubPOS;
        }

        idx getLastFingerPrintedPosition() {
            return _last_fngprnt_pos;
        }

        void setLastFingerPrintedPosition() {
            _last_fngprnt_pos = (refill_mode==1 || refill_mode ==-1)?ptr(_bfrcS._fatherclID)->curSubPOS+ptr(_bfrcS._fatherclID)->offset:_POS;
        }
        void updateLastFingerPrintedPosition(idx * t_pos = 0) {
            if(!t_pos){
                t_pos = &_last_fngprnt_pos;
            }
            *t_pos += _fingStep;
        }

        bool updateFingerPrints(idx subPos, idx * relativeTo = 0) {
            if( subPos > _last_fngprnt_pos ) {
                _last_fngprnt_pos = subPos + _fingStep;
                return true;
            }
            return false;
        }

        sClone * findCloneDestination(hdrStruct * al, sClone * last);

        const char * print(sStr * out = 0);

        inline static idx basecallInsertsOnMatchIndex(idx &i, hdrStruct * alHdr, idx quality_threshold, sVec<idx> & basecalls, idx * k_isx = 0);
        inline static idx basecallOnMatchIndex(idx &i, hdrStruct * alHdr, idx quality_threshold, idx * k_isx = 0);
        inline static idx basecallOnQryIndex(idx i, hdrStruct * alHdr, idx quality_threshold);
        inline static idx basecallOnSubjectIndex(idx i, hdrStruct * alHdr, idx quality_threshold);
        inline static idx basecallInsertsOnSubjectIndex(idx i, hdrStruct * alHdr, idx quality_threshold, sVec<idx> & basecalls);

        bool inRange(void) {
            return _iAlEnd < _alCnt_Max;
        }

        bool areMerging(sClone * w1, sClone * w2);
        idx merge(void);
        idx kill( void );
        idx computeStep(void);
        idx fill( idx refill,idx * relativeTo);
        bool bifurcate();
        idx isBifurcatedPosition(sBioseqpopul::position * p, idx clID, idx pos, real cutoff,idx minCov, idx * firstInd, sVec<idx> & secInd, idx * compare = 0);
        idx shift( idx Subtart,idx step);

        idx getClonalPositionDifferences(sClone *cl, idx pos, idx * bascalls, idx basecnt, real * compbuf);

        bool getCloneBoundaries(idx start_pos, idx length) {
            toEnd = getAlBoundaries(_bioal,_order_indices,start_pos,start_pos+length,_iAlStart,_iAlEnd,_alCnt_Max);
            return toEnd;
        }

        void SetNextCoordinates(idx pos) {
            _bfrcS._POS = pos;
            _POS = pos;
            _bfrcS._fatherclID = -1;
            _bfrcS._childclID = -1;
            _last_fngprnt_pos = -1;

        }

        void ensureAliveClone(idx pos) {
            if(!cntAlive()){
                sClone * cl = add();
                cl->init(_coverage.dim() / 2, 0, cnt()-1, ref_cnt);
                cl->parentClID = cl->clID;
                cl->startSubPOS = pos;
                cl->curSubPOS = pos;
                cl->curMatPOS = _coverage.curMatPOS;
                cl->extrPos = pos;
            }
        }

        const char * showAliveClones(sStr * out = 0);

        static idx allSortComparator(void * parameters, void * A, void * B,void * objSrc,idx i1,idx i2 );

        void getBifurcationStats(bifurcation_structure * bif = 0);

};

#endif 