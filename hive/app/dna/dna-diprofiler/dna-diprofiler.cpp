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
#include <qlib/QPrideProc.hpp>
#include <ssci/bio.hpp>
#include <ssci/math/stat/stat.hpp>
#include <violin/violin.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>

#define QIDX(_v_iq, _v_len)    ((flags&sBioseqAlignment::fAlignBackward) ? ((_v_len)-1-(_v_iq)) : (_v_iq))

class DnaDIprofiler: public sQPrideProc
{
    public:
        DnaDIprofiler(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }

        static idx similCumulator(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAl);

        struct cov
        {
                idx fwd, rev;
                sVec<idx> alIDs;
                idx sum(void)
                {
                    return fwd + rev;
                }
                void set(cov * c2)
                {
                    set(c2->fwd, c2->rev, c2->alIDs);
                }
                void set(idx l_fwd, idx l_rev, sVec<idx> & oAlIDs)
                {
                    fwd = l_fwd;
                    rev = l_rev;
                    alIDs.copy(&oAlIDs, false);
                }
                void set(idx l_fwd, idx l_rev, idx oAlID1, idx oAlID2)
                {
                    fwd = l_fwd;
                    rev = l_rev;
                    for(idx i = 0; i < l_fwd + l_rev; ++i)
                        alIDs.vadd(2, oAlID1, oAlID2);
                }
                void reset()
                {
                    fwd = 0;
                    rev = 0;
                    alIDs.cut(0);
                }
                void add(cov * c2)
                {
                    add(c2->fwd, c2->rev, c2->alIDs);
                }
                void add(idx l_fwd, idx l_rev, sVec<idx> & oAlIDs)
                {
                    fwd += l_fwd;
                    rev += l_rev;
                    alIDs.copy(&oAlIDs, true);
                }
                void add(idx l_fwd, idx l_rev, idx oAlID1, idx oAlID2)
                {
                    fwd += l_fwd;
                    rev += l_rev;
                    for(idx i = 0; i < l_fwd + l_rev; ++i)
                        alIDs.vadd(2, oAlID1, oAlID2);
                }
                cov()
                {
                    reset();
                }
        };
        struct crshit
        {
                idx sub, pos, dir, g_start, g_end;
                cov hits;
                bool isOK(idx l_sub, idx l_dir)
                {
                    return (sub == l_sub) && (dir == l_dir);
                }
                bool isSame(idx l_sub, idx l_dir, idx l_pos)
                {
                    return isOK(l_sub, l_dir) && l_pos == pos;
                }
                void set(crshit & o) {
                    sub = o.sub;
                    pos = o.pos;
                    dir = o.dir;
                    g_start = o.g_start;
                    g_end = o.g_end;
                    hits.set(&o.hits);
                }
                crshit()
                {
                    pos = dir = sub = g_start = g_end = 0;
                    hits.reset();
                }
        };
        struct crshitVec
        {
                crshit poshit;
                sVec<crshit> crshits;
        };
        static idx compareCrsHit(void *params, void * arr, idx i1, idx i2)
        {
            crshit * c_arr = (crshit *) arr;
            return compareCrsHit(params, c_arr + i1, c_arr + i2);
        }
        static idx compareCrsHitVec(void *params, void * arr, idx i1, idx i2)
        {
            crshitVec * c_arr = (crshitVec *) arr;
            if( c_arr[i1].poshit.sub != c_arr[i2].poshit.sub )
                return c_arr[i1].poshit.sub - c_arr[i2].poshit.sub;
            if( c_arr[i1].poshit.pos != c_arr[i2].poshit.pos )
                return c_arr[i1].poshit.pos - c_arr[i2].poshit.pos;
            return c_arr[i1].poshit.dir - c_arr[i2].poshit.dir;
        }
        static idx compareCrsHit(void *param, void * ind1, void * ind2)
        {
            crshit * c1 = (crshit *) ind1;
            crshit * c2 = (crshit *) ind2;
            if( c1->sub != c2->sub )
                return c1->sub - c2->sub;
            if( c1->pos != c2->pos )
                return c1->pos - c2->pos;
            return c1->dir - c2->dir;
        }

        class diprofData
        {
            public:
                class readSubAl: public sBioseqAlignment::Al
                {
                    private:
                        idx * _m, _qrylen, _rpts, _qStart,_qEnd, _sStart, _sEnd, _iAl;
                        bool _dir;
                    public:
                        readSubAl(sBioseqAlignment::Al * hdr, idx * m, idx rpts, idx qrylen, idx iAl) : sBioseqAlignment::Al(*hdr)
                        {
                            _iAl = iAl;
                            _m = m;
                            _qrylen = qrylen;
                            _rpts = rpts;
                            _qStart = isForward() ? getQueryStart(_m) : _qrylen - getQueryEnd(_m) - 1;
                            _qEnd = isForward() ? getQueryEnd(_m) : _qrylen - getQueryStart(_m) - 1;
                            _sStart = getSubjectStart(_m);
                            _sEnd = getSubjectEnd(_m);
                            _dir = isForward();
                        };

                        typedef enum EAlTrainType_enum
                        {
                            eAlTrainMatch       = 0,
                            eAlTrainMisMatch,
                            eAlTrainInsertion,
                            eAlTrainDeletion = eAlTrainInsertion,
                            eAlTrainGap = eAlTrainInsertion
                        } EAlTrainType;

                        inline bool isPerfect(idx score_match, idx align_len = -1) {
                            if( align_len < 0 )
                                align_len = _qrylen;
                            return ( align_len * score_match) == score() ;
                        }
                        inline idx sStart(void) {
                            return _sStart;
                        }
                        inline idx sEnd(void) {
                            return _sEnd;
                        }
                        inline idx qStart(void) {
                            return _qStart;
                        }
                        inline idx qEnd(void) {
                            return _qEnd;
                        }
                        inline idx rpts(void) {
                            return _rpts;
                        }
                        inline bool dir(void) {
                            return _dir;
                        }
                        inline void flip_dir() {
                            _dir = !_dir;
                        }
                        inline idx * get_m(void) {
                            return _m;
                        }
                        inline idx get_iAl(void) {
                            return _iAl;
                        }
                        inline idx qryCov(void)
                        {
                            return qEnd() - qStart() + 1;
                        }
                        inline idx subCov(void)
                        {
                            return sEnd() - sStart() + 1;
                        }
                        inline idx readOverlapLen(readSubAl & subAl2)
                        {
                            idx res = sMin(qEnd(),subAl2.qEnd()) - sMax(qStart(),subAl2.qStart()) + 1;
                            return res > 0 ? res : 0;
                        }
                        inline idx readOverlapStart(readSubAl & subAl2)
                        {
                            if( readOverlapLen(subAl2) <= 0 )
                                return -sIdxMax;
                            return subAl2.qStart();
                        }
                        inline idx readOverlapEnd(readSubAl & subAl2)
                        {
                            if( readOverlapLen(subAl2) <= 0 )
                                return -sIdxMax;
                            return sMin(subAl2.qEnd(),qEnd());
                        }
                        inline idx subOverlap(readSubAl & subAl2)
                        {
                            idx res = sMin(sEnd(),subAl2.sEnd()) - sMax(sStart(),subAl2.sStart());
                            return res > 0 ? res : 0;
                        }
                        inline idx readCov(readSubAl & subAl2)
                        {
                            return qryCov() + subAl2.qryCov() - readOverlapLen(subAl2);
                        }
                        inline idx subCov(readSubAl & subAl2)
                        {
                            return subCov() + subAl2.subCov() - subOverlap(subAl2);
                        }
                        inline bool isSuperSet(readSubAl & subAl2)
                        {
                            return ( qStart() <= subAl2.qStart() && qEnd() >= subAl2.qEnd() );
                        }
                        idx query2SubjectPos(idx qryPos, bool allow_gaps = false, idx * match_train = 0) {
                            if ( qryPos == qStart() )
                                return isReverseComplement()?sEnd():sStart();
                            if ( qryPos == qEnd() )
                                return isReverseComplement()?sStart():sEnd();

                            if( !match_train ) {
                                match_train = get_m();
                                static sVec<idx> uncompressedMM;
                                if( flags()&sBioseqAlignment::fAlignCompressed ){
                                    uncompressedMM.resize(2*lenAlign());
                                    sBioseqAlignment::uncompressAlignment(static_cast<sBioseqAlignment::Al *>(this),get_m(),uncompressedMM.ptr());
                                    match_train=uncompressedMM.ptr();
                                }
                            }
                            idx is = -sIdxMax, iq = 0, last_valid_is=match_train[0];
                            for(idx i = 0 ; i < 2*lenAlign() ; i+=2 ) {
                                iq = match_train[i+1];
                                if( iq < 0 )
                                    continue;
                                is = match_train[i];
                                iq += qryStart();
                                if ( isReverseComplement() )
                                    iq = _qrylen-1-iq;
                                if( iq == qryPos  ) {
                                    if(is<0) {
                                        if(allow_gaps)
                                            return is;
                                        else {
                                            return last_valid_is+subStart();
                                        }
                                    } else {
                                        return is+subStart();
                                    }
                                }
                                if(is>=0)
                                    last_valid_is = is;
                            }
                            return -sIdxMax;
                        }

                        inline idx basecallOnQryIndex(const char * vqry, idx iq){
                            if(iq<0){
                                return -1;
                            }
                            iq += qryStart();
                            idx iqx = isReverseComplement() ? (_qrylen-1-iq): iq;
                            return (idx)sBioseqAlignment::_seqBits(vqry, iqx, flags());
                        }

                        inline idx basecallOnSubjectIndex(const char * vsub, idx isx){
                            if(isx<0){
                                return -1;
                            }
                            return (idx)sBioseqAlignment::_seqBits(vsub, isx + subStart(), 0);
                        }

                        idx subAlScore(idx match_s, idx mismatch_s, idx mismatchN_s, idx gap_s, idx gapN_s, const char * vqry, const char * vsub, idx qry_subrange_Start = -sIdxMax, idx qry_subrange_End = -sIdxMax, idx * score_array = 0) {
                            if(qry_subrange_Start < -1) qry_subrange_Start = qStart();
                            if(qry_subrange_End < -1) qry_subrange_End = qEnd();
                            if( qry_subrange_Start < 0 || qry_subrange_End < 0 || qry_subrange_Start > qry_subrange_End )
                                return 0;
                            if( !isForward() ) {
                                idx qry_subrange_Tmp = _qrylen - qry_subrange_Start - 1;
                                qry_subrange_Start = _qrylen - qry_subrange_End - 1;
                                qry_subrange_End = qry_subrange_Tmp;
                            }

                            idx iq = 0, iqx = qryStart(), p_iq = qry_subrange_Start-1, is = 0, score = 0, * match_train = get_m();

                            static sVec<idx> uncompressedMM;
                            if(flags()&sBioseqAlignment::fAlignCompressed){
                                uncompressedMM.resize(2*lenAlign());
                                sBioseqAlignment::uncompressAlignment(static_cast<sBioseqAlignment::Al *>(this),_m,uncompressedMM.ptr());
                                match_train=uncompressedMM.ptr();
                            }
                            EAlTrainType prev_match = eAlTrainMatch;
                            idx curr_cost = 0;

                            for(idx i = 0 ; i < 2*lenAlign() ; i+=2 ) {
                                is = match_train[i];
                                iq = match_train[i+1];
                                iqx = iq + qryStart();
                                if( p_iq < qry_subrange_Start && (iq < 0 || iqx < qry_subrange_Start) ) {
                                    continue;
                                } else if ( iqx > qry_subrange_End ) {
                                    break;
                                }
                                if(iq >= 0) {
                                    if( iqx - p_iq - 1 > 0) {
                                        curr_cost += ((i==0||(match_train[i-2]>=0 && match_train[i-1]>=0))?gap_s:gapN_s) + (iqx - p_iq - 2)*gapN_s;
                                    }
                                }
                                if( iq < 0 || is < 0 ) {
                                    curr_cost += (prev_match==eAlTrainGap)?gapN_s:gap_s;
                                    prev_match = eAlTrainGap;

                                }
                                else if( basecallOnQryIndex(vqry, iq ) == basecallOnSubjectIndex(vsub,is ) ) {
                                    curr_cost += match_s;
                                    prev_match = eAlTrainMatch;
                                }
                                else {
                                    curr_cost += (prev_match==eAlTrainMisMatch)?mismatchN_s:mismatch_s;
                                    prev_match = eAlTrainMisMatch;
                                }
                                if(iq >= 0) {
                                    score += curr_cost;
                                    if(score_array) {
                                        if (isForward() || isForwardComplement()) {
                                            score_array[iqx-qry_subrange_Start] = curr_cost;
                                        } else {
                                            score_array[ qry_subrange_End - iqx ] = curr_cost;
                                        }
                                    }
                                    curr_cost = 0;
                                    p_iq = iqx;
                                }
                            }
                            score += curr_cost;
                            return score;
                        }
                };

                class junctionSubAl {
                    private:

                        idx _left_qry_start_point, _left_qry_break_point, _left_qry_end_point, _right_qry_start_point, _right_qry_break_point, _right_qry_end_point;
                        idx _left_sub_start_point, _left_sub_break_point, _left_sub_end_point, _right_sub_start_point, _right_sub_break_point, _right_sub_end_point;
                        idx _left_overlap_score, _right_overlap_score;
                        idx _overlap_max_score, _overlap_break_point, _pair_score;

                        bool _isFwd, _is_left_superset, _is_right_superset;

                        diprofData * _dpd;

                        inline idx _getLeftSplitPos() {
                            return _left_sub_break_point;
                        }

                        inline idx _getRightSplitPos() {
                            return _right_sub_break_point;
                        }

                    public:

                        readSubAl *_left, *_right;

                        junctionSubAl( diprofData * dpd, readSubAl * l_left = 0, readSubAl * l_right = 0 ) {
                            _dpd = dpd;
                            reset();
                            if( l_left && l_right ) {
                                init(l_left, l_right);
                            }
                        }

                        void swap() {
                            sSwap(_left, _right);
                            sSwap(_left_qry_start_point, _right_qry_start_point);
                            sSwap(_left_qry_break_point, _right_qry_break_point);
                            sSwap(_left_qry_end_point, _right_qry_end_point);
                            sSwap(_left_sub_start_point, _right_sub_start_point);
                            sSwap(_left_sub_break_point, _right_sub_break_point);
                            sSwap(_left_sub_end_point, _right_sub_end_point);
                            sSwap(_left_overlap_score, _right_overlap_score);
                            sSwap(_is_left_superset, _is_right_superset);
                        }

                        void reset() {
                            _left = _right = 0;
                            _left_qry_start_point = _left_qry_break_point = _left_qry_end_point = _right_qry_start_point = _right_qry_break_point = _right_qry_end_point = 0;
                            _left_sub_start_point = _left_sub_break_point = _left_sub_end_point = _right_sub_start_point = _right_sub_break_point = _right_sub_end_point = 0;
                            _left_overlap_score = _right_overlap_score = 0;
                            _isFwd = true;
                            _overlap_max_score = _pair_score = 0;
                            _overlap_break_point = -1;
                            _is_left_superset = _is_right_superset = false;
                        }

                        bool isValid() {
                            return _left && _right;
                        }

                        bool init(readSubAl * l_left, readSubAl * l_right) {
                            _left = l_left;
                            _right = l_right;

                            _left_qry_start_point = _left->qStart();
                            _left_qry_end_point = _left->qEnd();
                            _right_qry_start_point = _right->qStart();
                            _right_qry_end_point = _right->qEnd();

                            _left_sub_start_point = _left->sStart();
                            _left_sub_end_point = _left->sEnd();
                            _right_sub_start_point = _right->sStart();
                            _right_sub_end_point = _right->sEnd();

                            if( _left->qStart() > _right->qStart() ) {
                                swap();
                            }

                            idx overlap_max_break_point = 0, overlap_min_break_point = 0;

                            if ( isOverlap() ) {

                                _is_left_superset = _left->isSuperSet(*_right);
                                _is_right_superset = _right->isSuperSet(*_left);

                                idx original_overlap_score, original_min_overlap_iq, original_max_overlap_iq;
                                idx reversed_overlap_score, reversed_min_overlap_iq, reversed_max_overlap_iq;

                                original_overlap_score = computeOverlapMaxScore( original_min_overlap_iq, original_max_overlap_iq );
                                reversed_overlap_score = computeOverlapMaxScore( reversed_min_overlap_iq, reversed_max_overlap_iq, true );

                                _overlap_max_score = original_overlap_score;
                                overlap_max_break_point = original_max_overlap_iq;
                                overlap_min_break_point = original_min_overlap_iq;

                                if( areEqual() ) {
                                    if( original_overlap_score < reversed_overlap_score) {
                                        swap();
                                        overlap_max_break_point = reversed_max_overlap_iq;
                                        overlap_min_break_point = reversed_min_overlap_iq;
                                    }
                                } else if ( _is_left_superset ) {
                                    idx left_left_non_overlap = _dpd->subAlignmentScore(_left,_left->qStart(),_left->readOverlapStart(*_right) - 1);
                                    idx left_right_non_overlap = _dpd->subAlignmentScore(_left,_left->readOverlapEnd(*_right)+1, _left->qEnd());
                                    if( left_left_non_overlap + original_overlap_score < reversed_overlap_score + left_right_non_overlap ) {
                                        _left_qry_start_point = _right_qry_start_point;
                                        swap();
                                        _overlap_max_score = reversed_overlap_score;
                                        overlap_max_break_point = reversed_max_overlap_iq;
                                        overlap_min_break_point = reversed_min_overlap_iq;
                                    } else {
                                        _left_qry_end_point = _right_qry_end_point;
                                    }
                                } else if ( _is_right_superset ) {
                                    idx right_left_non_overlap = _dpd->subAlignmentScore(_right,_right->qStart(),_right->readOverlapStart(*_left) - 1);
                                    idx right_right_non_overlap = _dpd->subAlignmentScore(_right,_right->readOverlapEnd(*_left)+1, _right->qEnd());
                                    if( original_overlap_score + right_right_non_overlap < right_left_non_overlap + reversed_overlap_score) {
                                        _right_qry_end_point = _left_qry_end_point;
                                        swap();
                                        _overlap_max_score = reversed_overlap_score;
                                        overlap_max_break_point = reversed_max_overlap_iq;
                                        overlap_min_break_point = reversed_min_overlap_iq;
                                    } else {
                                        _right_qry_start_point = _left_qry_start_point;
                                    }
                                } else {
                                    idx left_left_non_overlap = _dpd->subAlignmentScore(_left,_left->qStart(),_left->readOverlapStart(*_right) - 1);
                                    idx right_right_non_overlap = _dpd->subAlignmentScore(_right,_right->readOverlapEnd(*_left)+1, _right->qEnd());
                                    if( left_left_non_overlap + original_overlap_score + right_right_non_overlap < reversed_overlap_score) {
                                        _right_qry_end_point = _left_qry_end_point;
                                        _left_qry_start_point = _right_qry_start_point;
                                        swap();
                                        _overlap_max_score = reversed_overlap_score;
                                        overlap_max_break_point = reversed_max_overlap_iq;
                                        overlap_min_break_point = reversed_min_overlap_iq;
                                    }
                                }
                                if( overlap_max_break_point - overlap_min_break_point != getOverlapLen() ) {
                                    if( overlap_max_break_point == _left_qry_end_point ) {
                                        overlap_min_break_point = overlap_max_break_point;
                                    } else if ( overlap_min_break_point == _right_qry_start_point ) {
                                        overlap_max_break_point = overlap_min_break_point;
                                    }
                                }
                            }

                            _pair_score = _overlap_max_score +
                                _dpd->subAlignmentScore(_left,_left->qStart(),_left->readOverlapStart(*_right) - 1) +
                                _dpd->subAlignmentScore(_right,_right->readOverlapEnd(*_left)+1, _right->qEnd());

                            _overlap_break_point = overlap_max_break_point;
                            if ( _left->sStart() > _right->sStart() ) {
                                _overlap_break_point = overlap_min_break_point;
                                _isFwd = false;
                            }

                            if ( _left->readOverlapLen(*_right) <= 0 ) {
                                _left_qry_break_point = _left_qry_end_point;
                                _right_qry_break_point = _right_qry_start_point;
                                _left_sub_break_point = _left->dir()?_left->sEnd():_left->sStart();
                                _right_sub_break_point = _right->dir()?_right->sStart():_right->sEnd();
                            } else {
                                _left_qry_break_point = _right_qry_break_point = _overlap_break_point;
                                _left_sub_break_point =  _left->query2SubjectPos(_left_qry_break_point);
                                _right_sub_break_point = _right->query2SubjectPos(_right_qry_break_point);
                            }

                            if ( (_left_sub_break_point > _right_sub_break_point ) ==_isFwd ) {
                                _isFwd = (_left_sub_break_point <= _right_sub_break_point );
                                _overlap_break_point = _isFwd ? overlap_max_break_point : overlap_min_break_point;
                                if ( _left->readOverlapLen(*_right) > 0 ) {
                                    _left_qry_break_point = _right_qry_break_point = _overlap_break_point;
                                    _left_sub_break_point =  _left->query2SubjectPos(_left_qry_break_point);
                                    _right_sub_break_point = _right->query2SubjectPos(_right_qry_break_point);
                                }
                            }


                            return _isFwd;
                        }

                        bool areEqual() {
                            return _is_left_superset && _is_right_superset;
                        }

                        bool isOverlap() {
                            return getOverlapLen() > 0;
                        }

                        inline idx getOverlapLen() {
                            return _left->readOverlapLen( *_right );
                        }

                        inline idx getPairLen() {
                            return _left->qEnd() - _left->qStart() + _right->qEnd() - _right->qStart() + 2 - _left->readOverlapLen( *_right );
                        }

                        idx computeOverlapMaxScore( idx & min_iq, idx & max_iq, bool is_reversed = false )
                        {
                            max_iq = -1;
                            idx overlap = _left->readOverlapLen( *_right );
                            if( !overlap )
                                return 0;


                            sVec<idx> overlap_score_arr1(sMex::fExactSize|sMex::fSetZero), overlap_score_arr2(sMex::fExactSize|sMex::fSetZero), overlap_score_arr_diff(sMex::fExactSize|sMex::fSetZero);
                            overlap_score_arr1.resize(overlap);overlap_score_arr2.resize(overlap), overlap_score_arr_diff.resize(overlap);
                            idx overlap_start = _left->readOverlapStart(*_right), overlap_end = _left->readOverlapEnd(*_right);

                            if( _left_overlap_score <= 0 ) {
                                _left_overlap_score = _dpd->subAlignmentScore(_left, overlap_start, overlap_end, overlap_score_arr1);
                            }
                            if( _right_overlap_score <= 0 ) {
                                _right_overlap_score = _dpd->subAlignmentScore(_right, overlap_start, overlap_end, overlap_score_arr2);
                            }

                            idx max_diff = 0, max_i = -sIdxMax, min_i = -sIdxMax;
                            for(idx i = 0 ; i < overlap_score_arr_diff.dim() ; ++i) {
                                overlap_score_arr_diff[i] = overlap_score_arr1[i] - overlap_score_arr2[i];
                                if( is_reversed )
                                    overlap_score_arr_diff[i] *= -1;
                                if( i ) {
                                    overlap_score_arr_diff[i] += overlap_score_arr_diff[i - 1];
                                }
                                if ( max_diff <= overlap_score_arr_diff[i] ) {
                                    max_i = i;
                                    if ( max_diff < overlap_score_arr_diff[i] ) {
                                        min_i = i;
                                        max_diff = overlap_score_arr_diff[i];
                                    }
                                }
                            }

                            max_i += 1;

                            if ( max_i < 0 )
                                max_iq = overlap_start;
                            else
                                max_iq = max_i - 1 + overlap_start;

                            min_i += 1;

                            if ( min_i < 0 ) {
                                if(max_i < 0)
                                    min_iq = max_iq;
                                else
                                    min_iq = overlap_start;
                            }
                            else
                                min_iq = min_i - 1 + overlap_start;

                            if( max_i >= overlap_score_arr1.dim() ) {
                                return !is_reversed ? _left_overlap_score : _right_overlap_score;
                            } else if( max_i < 0 ) {
                                return !is_reversed ? _right_overlap_score : _left_overlap_score;
                            }
                            idx score = 0;
                            for(idx i = 0 ; i < overlap_score_arr1.dim() ; ++i) {
                                score += (!is_reversed&&(i<max_i))?overlap_score_arr1[i]:overlap_score_arr2[i];
                            }

                            return score;
                        }

                        inline bool isCrossHitFwd() {
                            return _isFwd;
                        }

                        inline idx getPairedAlignmentScore()
                        {
                            return _pair_score;
                        }

                        inline idx getOverlapMaxScore() {
                            return _overlap_max_score;
                        }

                        inline idx _getLeftNonOverlap() {
                            return (isOverlap() ? _right->qStart() : _left->qEnd()) - _left->qStart();
                        }

                        inline idx _getRightNonOverlap() {
                            return _right->qEnd() - (isOverlap() ? _left->qEnd() : _right->qStart());
                        }
                        inline idx _getLeftOverlapScore() {
                            return _left_overlap_score;
                        }

                        inline idx _getRightOverlapScore() {
                            return _right_overlap_score;
                        }

                        inline idx getLeftSplitPos() {
                            return isCrossHitFwd()?_getLeftSplitPos():_getRightSplitPos();
                        }

                        inline idx getRightSplitPos() {
                            return (!isCrossHitFwd())?_getLeftSplitPos():_getRightSplitPos();
                        }

                        inline idx getLeftNonOverlap() {
                            return isCrossHitFwd()?_getLeftNonOverlap():_getRightNonOverlap();
                        }

                        inline idx getRightNonOverlap() {
                            return (!isCrossHitFwd())?_getLeftNonOverlap():_getRightNonOverlap();
                        }

                        inline idx getLeftOverlapScore() {
                            return isCrossHitFwd()?_getLeftOverlapScore():_getRightOverlapScore();
                        }

                        inline idx getRightOverlapScore() {
                            return (!isCrossHitFwd())?_getLeftOverlapScore():_getRightOverlapScore();
                        }

                        inline bool isLeftSuperset() {
                            return isCrossHitFwd()?_is_left_superset:_is_right_superset;
                        }

                        inline bool isRightSuperset() {
                            return isCrossHitFwd()?_is_right_superset:_is_left_superset;
                        }

                        inline idx getLeftScore() {
                            return _dpd->subAlignmentScore(left());
                        }

                        inline idx getRightScore() {
                            return _dpd->subAlignmentScore(right());
                        }

                        inline readSubAl *left() {
                            return isCrossHitFwd()?_left:_right;
                        }
                        inline readSubAl *right() {
                            return isCrossHitFwd()?_right:_left;
                        }


                };

                typedef enum eMultiAlScore {
                    eNoScore = 0,
                    eScoreSum = 1,
                    eMaxReadCov
                } EMultiAlScore;
                sStr ibuf;
                sVec<readSubAl> sub_al_map;
                sDic<sDic<cov> > * crossDic;
                sBioseqAlignment::Al * prevAl;
                idx startAl, endAl;
                idx minDistance, maxDistance, maxOverlap, minReadCov, minSubCov;
                idx progressStart, progressEnd;
                idx windowSize, minPeakCov, minSameRefDist,peakDetection_width;
                idx jumpRangeStart,jumpRangeEnd,maximumChewbackLeft,maximumChewbackRight;
                sStrT jumpRangeRelativeTo;
                idx minSubAlLength, minNonOverlappingSubAlignmentLength;
                EMultiAlScore scoreType;
                real threshold, influence, overlap_score_change_filter;
                idx match_score, mismatch_score, gap_score, mismatchNext_score, gapNext_score;
                sHiveal * hiveal;
            public:
                void * obj;

                diprofData()
                    : sub_al_map(sMex::fSetZero)
                {
                    startAl = endAl = 0;
                    prevAl = 0;
                    crossDic = 0;
                    obj = 0;
                    hiveal = 0;
                    minPeakCov = 0;
                    minSameRefDist=0;
                    maxDistance = maxOverlap = minReadCov = minSubCov = windowSize = 0;
                    minDistance = -sIdxMax;
                    jumpRangeStart=-1,jumpRangeEnd=-1;maximumChewbackLeft=0;maximumChewbackRight=0;
                    threshold = influence = 0;
                    minNonOverlappingSubAlignmentLength = 0;
                    gap_score = mismatch_score = match_score = mismatchNext_score = gapNext_score = 0;
                    overlap_score_change_filter = 0;
                    progressStart = 0;
                    progressEnd = 0;
                    peakDetection_width = 0;
                    scoreType = eNoScore;
                    minSubAlLength = 0;
                }

                inline idx isLessThanMaxDistance(readSubAl & subAl1, readSubAl & subAl2)
                {
                    return sAbs(subAl2.qStart() - subAl1.qEnd()) <= maxDistance;
                }

                inline idx isMoreThanMinDistance(readSubAl & subAl1, readSubAl & subAl2)
                {
                    return sAbs(subAl2.qStart() - subAl1.qEnd()) >= minDistance;
                }

                inline idx isLessThanMaxOverlap(readSubAl & subAl1, readSubAl & subAl2)
                {
                    return sAbs(subAl1.qEnd() - subAl2.qStart()) <= maxOverlap;
                }
                inline idx isReadMoreThanMinAligned(readSubAl & subAl1, readSubAl & subAl2)
                {
                    return subAl1.readCov(subAl2) >= minReadCov;
                }
                inline idx isSubjectMoreThanMinAligned(readSubAl & subAl1, readSubAl & subAl2)
                {
                    return subAl1.subCov(subAl2) >= minSubCov;
                }

                idx progress(idx iNum)
                {
                    return ((DnaDIprofiler *) obj)->reqProgressStatic(obj, iNum - startAl + 1, progressStart + (iNum - startAl + 1) * ((real) (progressEnd - progressStart) / (endAl - startAl)), 100);
                }

                char * printCrossRefPos(readSubAl * subAl, idx splitpos, bool dir)
                {
                    return ibuf.printf(0, "%" DEC ":%" DEC ":%c:", subAl->idSub(), splitpos, dir ? '+' : '-');
                }
                static bool parseCrossRefPos(const char * in, idx &ref, idx &pos, idx &strdn)
                {
                    char d;
                    if( !(sString::bufscanf(in, 0, "%" DEC ":%" DEC ":%c:", &ref, &pos, &d) == 3) ) {
                        return false;
                    }
                    strdn = (d == '+');
                    return true;
                }

                idx pairedAlignmentMeasurement(junctionSubAl & jctAls)
                {
                    switch (scoreType) {
                        case eScoreSum:
                            return jctAls.getPairedAlignmentScore();
                        case eMaxReadCov:
                            return jctAls._left->readCov(*jctAls._right);
                        default:
                            return 1;
                    }
                }

                idx subAlignmentScore(readSubAl *  subAl, idx qry_subrange_Start = -sIdxMax, idx qry_subrange_End = -sIdxMax, idx * score_arr = 0 ) {
                    return subAl->subAlScore(match_score, mismatch_score, mismatchNext_score, gap_score, gapNext_score, hiveal->Qry->seq(subAl->idQry()), hiveal->Sub->seq(subAl->idSub()), qry_subrange_Start, qry_subrange_End, score_arr);
                }

                inline bool isPairOfSubAlignmentsValid(junctionSubAl & jctAls) {
                    readSubAl * left = jctAls._left, *right = jctAls._right;
                    if( !left || !right ) {
                        return false;
                    }
                    readSubAl * cur_subAl = 0;
                    for(idx i = 0 ; i < sub_al_map.dim() ; ++i ) {
                        cur_subAl = sub_al_map.ptr(i);
                        if ( !isPairBetterThanSignleAlignment(cur_subAl, jctAls) )
                            return false;
                    }
                    return true;
                }

                inline bool isTherePerfectSubAl() {
                    for( idx i = 0 ; i < sub_al_map.dim() ; ++i ) {
                        if(sub_al_map[i].isPerfect(match_score))return true;
                    }
                    return false;
                }

                inline bool areSubAlignmentsValidPair(junctionSubAl & jctAls, sBioal * bioal)
                {
                    readSubAl * left = jctAls._left;
                    readSubAl * right = jctAls._right;
                    if( !left || !right ) {
                        return false;
                    }
                    bool passed_overlap_filter = isLessThanMaxOverlap(*jctAls._left, *jctAls._right);
                    if ( !passed_overlap_filter ) {
                        if( jctAls._getLeftNonOverlap() > minNonOverlappingSubAlignmentLength || jctAls._getRightNonOverlap() > minNonOverlappingSubAlignmentLength) {
                            real thrs_score = overlap_score_change_filter * jctAls.getPairLen() * match_score;
                            idx cmpScore = 0;
                            if( jctAls.isLeftSuperset() ) {
                                cmpScore = jctAls.getLeftScore();
                            } else if( jctAls.isRightSuperset() ) {
                                cmpScore = jctAls.getRightScore();
                            } else {
                                cmpScore = sMax(jctAls.getLeftScore(), jctAls.getRightScore());
                            }

                            if ( 0 && (jctAls.getPairedAlignmentScore() - cmpScore ) >  thrs_score ) {
                                passed_overlap_filter = true;
                            }
                        }
                    }

                    if( passed_overlap_filter && (jctAls._getLeftNonOverlap() < minNonOverlappingSubAlignmentLength || jctAls._getRightNonOverlap() < minNonOverlappingSubAlignmentLength)) {
                        passed_overlap_filter=false;
                    }

                    if(passed_overlap_filter && (jumpRangeStart!=sIdxMax || jumpRangeEnd !=sIdxMax) ){

                        idx relPosLeft=0,relPosRight=0,rLen=0, pos=0, leftCode=0,rightCode=0;
                        const char * id=bioal->Sub->id(jctAls.right()->idSub());
                        if(jumpRangeRelativeTo.length()){
                            rLen=sLen(jumpRangeRelativeTo.ptr());
                            const char * rel=strstr(id, jumpRangeRelativeTo.ptr());
                            if(rel)relPosRight=atoidx(rel+rLen);
                            else relPosRight=-1;
                        }

                        if(relPosRight!=-1){
                            pos=jctAls.getRightSplitPos();
                            if( pos<jumpRangeStart+relPosRight-maximumChewbackRight) rightCode=-2;
                            else if( pos>jumpRangeEnd+relPosRight+maximumChewbackRight) rightCode=2;
                            else if( pos<jumpRangeStart+relPosRight) rightCode=-1;
                            else if( pos>jumpRangeEnd+relPosRight) rightCode=1;
                        }

                        relPosLeft=0;
                        id=bioal->Sub->id(jctAls.left()->idSub());
                        if(jumpRangeRelativeTo.length()){
                            const char * rel=strstr(id, jumpRangeRelativeTo.ptr());
                            if(rel)relPosLeft=atoidx(rel+rLen);
                            else relPosLeft=-1;
                        }

                        if(relPosLeft!=-1){
                            pos=jctAls.getLeftSplitPos();
                            if( pos<jumpRangeStart+relPosLeft-maximumChewbackLeft) leftCode=-2;
                            else if( pos>jumpRangeEnd+relPosLeft+maximumChewbackLeft) leftCode=2;
                            else if( pos<jumpRangeStart+relPosLeft) leftCode=-1;
                            else if( pos>jumpRangeEnd+relPosLeft) leftCode=1;
                        }

                        if( sAbs(leftCode)==2 || sAbs(rightCode)==2)
                            passed_overlap_filter=false;
                        else if ( sAbs(leftCode* rightCode) !=0 )
                            passed_overlap_filter=false;



                    }

                    return (isMoreThanMinDistance(*left, *right) && isLessThanMaxDistance(*left, *right) && passed_overlap_filter && isReadMoreThanMinAligned(*left, *right) && isSubjectMoreThanMinAligned(*left, *right));
                }

                inline bool isPairBetterThanSignleAlignment(readSubAl * subAl_s, junctionSubAl & jctAls) {
                    return jctAls.getPairedAlignmentScore() > subAl_s->score();
                }
        };

        idx peakFiltration(sVec<crshitVec> & crossDic, diprofData * SPData);
        sRC printCrossCovResults( sVec<crshitVec> & crcv, sVec<idx> & readIDs, sDic<sMex::Pos> & readLU, const char * flnm, sBioseq * sub, bool printGroupBoundaries, idx minPeakCov, idx minSameRefDist, idx taxAndLineage_inf);

        sTaxIon * taxion;
        virtual idx OnExecute(idx);
};

idx DnaDIprofiler::similCumulator(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAl)
{
    diprofData * SPData = (diprofData *) param->userPointer;

    idx cnt = SPData->sub_al_map.dim();

    static diprofData::junctionSubAl jctAls(SPData), bestJctAls(SPData);
    bestJctAls.reset();

    if( SPData->prevAl && (SPData->prevAl->idQry() != hdr->idQry() || iNum == SPData->endAl - 1) && cnt ) {
        if( !SPData->isTherePerfectSubAl() ) {
            idx best_score = 0, cur_score = 0;
            for(idx i = 0; i < cnt; ++i) {
                for(idx j = i + 1; j < cnt; ++j) {
                    jctAls.reset();
                    jctAls.init(SPData->sub_al_map.ptr(i), SPData->sub_al_map.ptr(j));

                    if( SPData->areSubAlignmentsValidPair(jctAls, bioal) && SPData->isPairOfSubAlignmentsValid(jctAls) ) {
                        if( SPData->scoreType == diprofData::eNoScore ) {
                            SPData->printCrossRefPos(jctAls.left(), jctAls.getLeftSplitPos(), jctAls.isCrossHitFwd()?jctAls.left()->dir():!jctAls.left()->dir());
                            sDic<cov> * pairedSubAl = SPData->crossDic->setString(SPData->ibuf.ptr());

                            bool keepRightSubAlForDirectionality = (jctAls.right()->dir() != jctAls.left()->dir() && jctAls.isCrossHitFwd() != jctAls.left()->dir());

                            SPData->printCrossRefPos(jctAls.right(), jctAls.getRightSplitPos(), jctAls.isCrossHitFwd()?jctAls.right()->dir():!jctAls.right()->dir());

                            if( jctAls.isCrossHitFwd() ) {
                                pairedSubAl->setString(SPData->ibuf.ptr())->add(jctAls.left()->rpts(), 0, keepRightSubAlForDirectionality ? jctAls.right()->get_iAl() : jctAls.left()->get_iAl(),
                                    keepRightSubAlForDirectionality ? jctAls.left()->get_iAl() : jctAls.right()->get_iAl());
                            } else {
                                pairedSubAl->setString(SPData->ibuf.ptr())->add(0, jctAls.left()->rpts(), keepRightSubAlForDirectionality ? jctAls.right()->get_iAl() : jctAls.left()->get_iAl(),
                                    keepRightSubAlForDirectionality ? jctAls.left()->get_iAl() : jctAls.right()->get_iAl());
                            }
                        } else {
                            cur_score = SPData->pairedAlignmentMeasurement(jctAls);
                            if( cur_score > best_score ) {
                                best_score = cur_score;
                                bestJctAls = jctAls;
                            }
                        }
                    }
                }
            }
            if( cnt && SPData->scoreType != diprofData::eNoScore && bestJctAls.isValid() ) {
                SPData->printCrossRefPos(bestJctAls.left(), bestJctAls.getLeftSplitPos(), bestJctAls.isCrossHitFwd()?bestJctAls.left()->dir():!bestJctAls.left()->dir());
                sDic<cov> * pairedSubAl = SPData->crossDic->setString(SPData->ibuf.ptr());
                bool keepRightSubAlForDirectionality = (bestJctAls.right()->dir() != bestJctAls.left()->dir() && bestJctAls.isCrossHitFwd() != bestJctAls.left()->dir());

                SPData->printCrossRefPos(bestJctAls.right(), bestJctAls.getRightSplitPos(), bestJctAls.isCrossHitFwd()?bestJctAls.right()->dir():!bestJctAls.right()->dir());

                if( bestJctAls.isCrossHitFwd() ) {
                    pairedSubAl->setString(SPData->ibuf.ptr())->add(bestJctAls.left()->rpts(), 0, keepRightSubAlForDirectionality ? bestJctAls.right()->get_iAl() : bestJctAls.left()->get_iAl(),
                        keepRightSubAlForDirectionality ? bestJctAls.left()->get_iAl() : bestJctAls.right()->get_iAl());
                } else {
                    pairedSubAl->setString(SPData->ibuf.ptr())->add(0, bestJctAls.left()->rpts(), keepRightSubAlForDirectionality ? bestJctAls.right()->get_iAl() : bestJctAls.left()->get_iAl(),
                        keepRightSubAlForDirectionality ? bestJctAls.left()->get_iAl() : bestJctAls.right()->get_iAl());
                }
            }
        }
        SPData->sub_al_map.cut(0);
    }

    SPData->progress(iNum);

    diprofData::readSubAl subAl(hdr, m, bioal->getRpt(iAl), bioal->Qry->len(hdr->idQry()), iAl);
    SPData->sub_al_map.vadd(1, subAl);
    SPData->prevAl = hdr;

    return 1;
}

DnaDIprofiler::crshit * getCrshit(DnaDIprofiler::crshit * rec)
{
    return rec;
}
DnaDIprofiler::crshit * getCrshit(DnaDIprofiler::crshitVec * rec)
{
    return &rec->poshit;
}

template<class Tobj> idx sparseArray2timeSeries(sVec<Tobj> * vec, sVec<idx> &tS, idx subId, idx slen, bool dir, DnaDIprofiler::diprofData * SPData)
{
    idx lastValid = 0, start = 0;
    tS.flagOn(sMex::fSetZero);
    for(idx i = 0; i < vec->dim(); ++i) {
        Tobj * crcv = vec->ptr(i);
        DnaDIprofiler::crshit * cr = getCrshit(crcv);
        if( cr->isOK(subId, dir) ) {
            if( !tS.dim() ) {
                tS.add(SPData->windowSize);
                start = cr->pos - SPData->windowSize;
                lastValid = cr->pos - 1;
            }
            tS.add(cr->pos - lastValid);
            *tS.last() = cr->hits.sum();
            lastValid = cr->pos;
        }
    }
    return start;
}
void mergeCrossPos(DnaDIprofiler::crshit * from, DnaDIprofiler::crshit * to, idx isub, idx idir, DnaDIprofiler::diprofData * SPData)
{
    if( to && from ) {
        to->hits.add( &from->hits );
        from->hits.reset();
    }
}

void mergeCrossPos(DnaDIprofiler::crshitVec * from, DnaDIprofiler::crshitVec * to, idx isub, idx idir, DnaDIprofiler::diprofData * SPData)
{
    if( from && to ) {
        DnaDIprofiler::crshit * c1 = 0, *c2 = 0;
        to->poshit.hits.add( &from->poshit.hits );
        from->poshit.hits.reset();
        idx existing_i = -1, cmpres = 0;
        for(idx i = 0; i < from->crshits.dim(); ++i) {
            c1 = from->crshits.ptr(i);
            existing_i = sSort::binarySearch(DnaDIprofiler::compareCrsHit, 0, c1, to->crshits.dim(), to->crshits.ptr(), 0, sSort::eSearch_Any, true);
            c2 = to->crshits.ptr(existing_i);
            cmpres = DnaDIprofiler::compareCrsHit(0, c1, c2);
            if( cmpres == 0 ) {
                c2->hits.add( &c1->hits );
            } else {
                if( cmpres > 0 )
                    existing_i += 1;
                c2 = to->crshits.insert(existing_i, 1);
                c2->set(*c1);
            }
            c1->hits.reset();
        }
    }

}

template<class Tobj> idx groupPeaks(sVec<Tobj> * crossVec, sVec<idx> &peakMap, idx peakStart, idx start, idx isub, idx idir, DnaDIprofiler::diprofData * SPData)
{
    idx cur_pos, mapped_pos, mapped_cnt = 0;
    DnaDIprofiler::crshit * crhit = 0;
    idx group_first_ic = 0, last_peak_ic = 0, last_peak = 0;
    sVec<idx> cr_peakMap(sMex::fSetZero);
    for(idx i = 0, ic = 0; i < peakMap.dim() - peakStart; ++i) {
        cur_pos = i + start;
        mapped_pos = peakMap[i + peakStart] - peakStart + start;
        crhit = getCrshit(crossVec->ptr(ic));
        while( !crhit->isSame(isub, idir, cur_pos) && (crhit->sub < isub || crhit->pos < cur_pos + (idir ? 1 : 0)) && ic < crossVec->dim() ) {
            crhit = getCrshit(crossVec->ptr(++ic));
        }
        if( crhit->isSame(isub, idir, cur_pos) ) {
            if( mapped_pos != last_peak ) {
                last_peak = mapped_pos;
                group_first_ic = ic;
            }
            if( cur_pos > mapped_pos ) {
                *cr_peakMap.ptrx(ic) = last_peak_ic;
            } else if( cur_pos == mapped_pos ) {
                last_peak_ic = ic;
                for(; group_first_ic <= ic; ++group_first_ic)
                    *cr_peakMap.ptrx(group_first_ic) = last_peak_ic;
            }
            ++ic;
        }
    }
    Tobj * o_from, *o_to;
    DnaDIprofiler::crshit * crhit_from = 0, *crhit_to = 0;
    for(idx i = 0; i < crossVec->dim(); ++i) {
        o_from = crossVec->ptr(i);
        crhit_from = getCrshit(o_from);
        if( crhit_from->isOK(isub, idir) ) {
            o_to = crossVec->ptr(cr_peakMap[i]);
            crhit_to = getCrshit(o_to);
            if( !crhit_to->g_start )
                crhit_to->g_start = crhit_from->pos;
            if( crhit_to->g_end < crhit_from->pos )
                crhit_to->g_end = crhit_from->pos;
            if( o_from != o_to ) {
                mergeCrossPos(o_from, o_to, isub, idir, SPData);
            } else {
                ++mapped_cnt;
            }
        }
    }
    return mapped_cnt;
}

idx DnaDIprofiler::peakFiltration(sVec<crshitVec> & crossVec, diprofData * SPData)
{
    idx progress100SubTotal = progress100End - progress100Start;
    progress100End = progress100Start + progress100SubTotal / 2;
    for(idx is = 0; is < SPData->hiveal->Sub->dim(); ++is) {
        if( !reqProgress(is, is, SPData->hiveal->Sub->dim()) )
            return 0;
        for(idx d = 0; d <= 1; ++d) {
            sVec<idx> xpos;
            idx x_start;
            x_start = sparseArray2timeSeries(&crossVec, xpos, is, SPData->hiveal->Sub->len(is), d, SPData);
            if( !xpos.dim() ) {
                continue;
            }
            sVec<idx> peakMap(sMex::fSetZero);
            sStat::peakDetection(xpos, 0, &peakMap, true,SPData->peakDetection_width,1);
            xpos.destroy();
            groupPeaks(&crossVec, peakMap, SPData->windowSize, x_start, is, d, SPData);
        }
    }
    idx crpos_cnt = 0;
    progress100Start = progress100End;
    progress100End = progress100End + progress100SubTotal / 2;
    for(idx ic = 0; ic < crossVec.dim(); ++ic) {
        if( !reqProgress(ic, ic, crossVec.dim()) ) {
            return 0;
        }
        crshitVec * crcv = crossVec.ptr(ic);
        if( !crcv )
            continue;
        ++crpos_cnt;

        for(idx is2 = 0; is2 < SPData->hiveal->Sub->dim(); ++is2) {
            for(idx d2 = 0; d2 <= 1; ++d2) {
                sVec<idx> ypos;
                idx y_start;
                y_start = sparseArray2timeSeries(&crcv->crshits, ypos, is2, SPData->hiveal->Sub->len(is2), d2, SPData);
                if( !ypos.dim() )
                    continue;
                sVec<idx> peakMap(sMex::fSetZero);
                sStat::peakDetection(ypos, 0, &peakMap, true,SPData->peakDetection_width,1);
                ypos.destroy();
                groupPeaks(&crcv->crshits, peakMap, SPData->windowSize, y_start, is2, d2, SPData);
            }
        }
        SPData->progress(ic);
    }
    return crpos_cnt;
}

void printHeader(sStr & buf, sBioseq * sub, bool printGroupBoundaries, bool print_construct_info, idx taxAndLineage_info=0)
{
    if( sub && sub->dim() > 1 ) {
        buf.addString("Subject (left),");
    }
    buf.addString("Position (left),");

    if( printGroupBoundaries ) {
        buf.addString("Group start (left),Group end (left),");
    }
    buf.addString("Strandness (left),");

    if( sub && sub->dim() > 1 ) {
        buf.addString("Subject (right),");
    }
    buf.addString("Position (right),");

    if( printGroupBoundaries ) {
        buf.addString("Group start (right),Group end (right),");
    }
    buf.addString("Strandness (right),");
    if( print_construct_info ) {
        buf.addString("DVG,Length,Delta,");
    }
    buf.addString("Forward hits,Reverse hits");
    if (taxAndLineage_info==1) {
        buf.addString(",TaxID (left),TaxID (right)");
    } else if (taxAndLineage_info==2){
        buf.addString(",TaxID (left),Lineage (left),TaxID (right),Lineage (right)");
    }
    buf.addString("\n");
}

void printSingleHit(sStr & buf, DnaDIprofiler::crshit * crcv, sBioseq * sub, bool printGroupBoundaries)
{
    if( sub && sub->dim() > 1 ) {
        const char * id=sub->id(crcv->sub);idx id_len=sLen(id);
        while( id_len>0  && (((char*)id)[id_len-1]==' ' || ((char*)id)[id_len-1]=='\t' ))--id_len;
        buf.printf("\"%.*s\",", (int)id_len,id);
    }
    buf.printf("%" DEC ",", crcv->pos+1);
    if( printGroupBoundaries ) {
        if( crcv->g_start == crcv->g_end )
            buf.printf("-,-,");
        else
            buf.printf("%" DEC ",%" DEC ",", crcv->g_start+1, crcv->g_end+1);
    }
    buf.printf("%c,", (crcv->dir ? '+' : '-'));
}
void printDVGInfo(sStr & buf, DnaDIprofiler::crshit * crcv1, DnaDIprofiler::crshit * crcv2, idx RefLength1, idx RefLength2, bool is_genome_antisense = false)
{
    if( crcv1->dir && crcv2->dir ) {
        buf.addString("deletion,");
        buf.printf("%" DEC",", crcv1->pos + 1 + RefLength2 - crcv2->pos);
    } else if( !crcv1->dir && !crcv2->dir ) {
        buf.addString("insertion,");
        buf.printf("%" DEC",", crcv2->pos + RefLength1 + 1 - crcv1->pos);
    } else {
        if( crcv1->dir && !crcv2->dir ) {
            buf.printf("%c' cb,",is_genome_antisense?'3':'5');
            buf.printf("%" DEC",", crcv1->pos + crcv2->pos + 2);
        } else {
            buf.printf("%c' cb,",is_genome_antisense?'5':'3');
            buf.printf("%" DEC",", (RefLength1 + RefLength2 + 2) - crcv1->pos - crcv2->pos);
        }
    }
    buf.printf("%" DEC",", crcv2->pos - crcv1->pos);
}

sRC DnaDIprofiler::printCrossCovResults(sVec<crshitVec> & crcv, sVec<idx> & readIDs, sDic<sMex::Pos> & readLU, const char * flnm, sBioseq * sub, bool printGroupBoundaries = false, idx minPeakCov = 0, idx minSameRefDist=0, idx taxAndLineage_info =0 )
{
    sStr bufPath;
    const char * dstPath = sQPrideProc::reqAddFile(bufPath, flnm);

    if( !dstPath ) {
        logOut(eQPLogType_Warning, "Failed to add DI profiler file %s for %s", flnm, objs[0].IdStr());
        reqSetInfo(reqId, eQPInfoLevel_Warning, "Failed to add DI profiler file %s for %s", flnm, objs[0].IdStr());
        return sRC(sRC::eCreating, sRC::eFile, sRC::eFile, sRC::eFailed);
    }
    sFil buf(dstPath);

    bool print_construct_info = false;
    if ( formBoolValue("print_construct_info") ) {
        print_construct_info = true;
    }
    bool is_genome_antisense = false;
    if ( formBoolValue("is_genome_antisense") ) {
        is_genome_antisense = true;
    }

    printHeader(buf, sub, printGroupBoundaries, print_construct_info, taxAndLineage_info);
    idx bufprevlen = 0, curbuflen2dic = 0;
    sStrT taxid1, acc1, lin1;
    sStrT taxid2, acc2, lin2;
    bool bioeng1=false, bioeng2=false;
    for(idx ic = 0; ic < crcv.dim(); ++ic) {
        crshitVec * crcv2 = crcv.ptr(ic);
        if( !crcv2 || !crcv2->poshit.hits.sum() || crcv2->poshit.hits.sum() < minPeakCov )
            continue;
        for(idx ic2 = 0; ic2 < crcv2->crshits.dim(); ++ic2) {
            crshit * cov = crcv2->crshits.ptr(ic2);
            if( !cov || !cov->hits.sum() || cov->hits.sum() < minPeakCov )
                continue;
            if(cov->sub==crcv2->poshit.sub) {
                if(sAbs(cov->pos - crcv2->poshit.pos)<minSameRefDist)
                    continue;

            }

            bufprevlen = buf.length();
            printSingleHit(buf, &crcv2->poshit, sub, printGroupBoundaries);
            printSingleHit(buf, cov, sub, printGroupBoundaries);
            curbuflen2dic = buf.length();
            if( print_construct_info ) {
                printDVGInfo(buf, &crcv2->poshit, cov, sub->len(crcv2->poshit.sub), sub->len(cov->sub), is_genome_antisense);
            }
            sMex::Pos * rvlu =  readLU.set(buf.ptr(bufprevlen),curbuflen2dic-bufprevlen-1);
            rvlu->pos = readIDs.dim();
            rvlu->size = cov->hits.alIDs.dim();
            readIDs.copy(&cov->hits.alIDs,true);

            buf.printf("%" DEC ",%" DEC "", cov->hits.fwd, cov->hits.rev);
            if (taxAndLineage_info) {
                taxid1.cut(0); taxid2.cut(0);
                bioeng1=false; bioeng2=false;
                const char * sub_id1 = sub->id(crcv2->poshit.sub);
                const char * sub_id2 = sub->id(cov->sub);
                if (strstr(sub_id1,"BIOENGINEERED")) {
                    taxid1.printf("BIOENGINEERED");
                    bioeng1=true;
                } else {
                    taxion->extractTaxIDfromSeqID(&taxid1, 0, 0, sub_id1, 0, "NO_INFO");
                }
                if (strstr(sub_id2,"BIOENGINEERED")) {
                    taxid2.printf("BIOENGINEERED");
                    bioeng2=true;
                } else {
                    taxion->extractTaxIDfromSeqID(&taxid2, 0, 0, sub_id2, 0, "NO_INFO");
                }
                if (taxAndLineage_info==1) {
                    buf.printf(",%s,%s", taxid1.ptr(), taxid2.ptr());
                } else {
                    acc1.cut(0); acc2.cut(0); lin1.printf(0,"%s",""); lin2.printf(0,"%s","");
                    if (!bioeng1) {
                        lin1.cut(0);
                        sString::copyUntil(&acc1,sub->id(crcv2->poshit.sub),0," \r\t\n");
                        taxion->getLineageByAccession(acc1,&lin1);
                    }
                    if (!bioeng2) {
                        lin2.cut(0);
                        sString::copyUntil(&acc2,sub->id(cov->sub),0," \r\t\n");
                        taxion->getLineageByAccession(acc2,&lin2);
                    }

                    buf.printf(",%s,%s,%s,%s", taxid1.ptr(), lin1.ptr(), taxid2.ptr(), lin2.ptr());
                }
            } 
            buf.printf("\n");
        }
    }
    return sRC::zero;
}

idx DnaDIprofiler::OnExecute(idx req)
{
    sStr errS;


    sBioal::ParamsAlignmentIterator PA;
    diprofData RD;
    sDic<sDic<cov> > CrossCov;
    PA.userPointer = (void*) &RD;
    PA.navigatorFlags = sBioal::alPrintCollapseRpt;


    sHiveId alignerID(formValue("alignmentID"));

    if( !alignerID.valid() ) {
        errS.addString("Alignment computation is not accessible or missing\n");
        logOut(eQPLogType_Error, errS);
        reqSetInfo(req, eQPInfoLevel_Error, errS);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sUsrObj aligner(*sQPride::user, alignerID);

    idx aligner_matches_to_keep = aligner.propGetI("keepAllMatches");
    if( aligner_matches_to_keep != 3 && aligner_matches_to_keep != 2 ) {
        errS.addString("Alignment computation must not be set to keep only one hit\n Please change the \"Matches to Keep\" parameter and try again\n");
        logOut(eQPLogType_Error, errS);
        reqSetInfo(req, eQPInfoLevel_Error, errS);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    RD.minSubAlLength = aligner.propGetI("minMatchLen");
    RD.match_score = aligner.propGetI("costMatch");
    RD.mismatch_score = aligner.propGetI("costMismatch");
    RD.gap_score = aligner.propGetI("costGapOpen");
    RD.mismatchNext_score = aligner.propGetI("costMismatchNext");
    RD.gapNext_score = aligner.propGetI("costGapNext");

    sStr subject;
    QPSvcDnaHexagon::getSubject00(aligner,subject);
    sStr query;
    QPSvcDnaHexagon::getQuery00(aligner,query);

    sStr path;
    aligner.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __);
    sHiveal r_hiveal(user, path), *hiveal = &r_hiveal;

    if( !hiveal->isok() || hiveal->dimAl() == 0 ) {
        errS.printf("Alignment file %s is missing or corrupted\n", alignerID.print());
        logOut(eQPLogType_Error, errS);
        reqSetInfo(req, eQPInfoLevel_Error, errS);
        reqSetStatus(req, eQPReqStatus_ProgError);

        return 0;
    }
    sVec<idx> qrySortAl(sMex::fExactSize), qrySortAlInd(sMex::fExactSize);
    qrySortAl.resize(hiveal->dimAl());
    qrySortAlInd.resize(hiveal->dimAl());
    for(idx iAl = 0; iAl < hiveal->dimAl(); ++iAl) {
        sBioseqAlignment::Al * hdr = hiveal->getAl(iAl);
        qrySortAl[iAl] = hdr->idQry();
    }
    sSort::sort(qrySortAl.dim(), qrySortAl.ptr(), qrySortAlInd.ptr());
    qrySortAl.destroy();

    sHiveseq Sub(sQPride::user, subject.ptr());
    Sub.reindex();
    if( Sub.dim() == 0 ) {
        errS.printf("Reference '%s' sequences are missing or corrupted\n", subject.length() ? subject.ptr() : "unspecified");
        logOut(eQPLogType_Error, errS);
        reqSetInfo(req, eQPInfoLevel_Error, errS);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }


    sHiveseq Qry(sQPride::user, query.ptr());
    Qry.reindex();
    if( Qry.dim() == 0 ) {
        errS.printf("Query '%s' sequences are missing or corrupted\n", query.ptr() ? query.ptr() : "unspecified");
        logOut(eQPLogType_Error, errS);
        reqSetInfo(req, eQPInfoLevel_Error, errS);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    hiveal->Sub = &Sub;
    hiveal->Qry = &Qry;

    RD.hiveal = hiveal;

    idx numAlChunks = 1;
    idx cntFound = 0;

    sStr diprofName, diprofPath;
    RD.prevAl = 0;
    RD.maxDistance = formIValue("maxNonOverlap", 50);
    RD.maxOverlap = formIValue("maxOverlap", 20);
    RD.minReadCov = formIValue("minReadCov", 30);
    RD.minSubCov = formIValue("minSubCov", 30);
    RD.peakDetection_width = formIValue("peakDetectionWidth",5);
    RD.overlap_score_change_filter = formRValue("overlapScoreFilter", 3)/100;
    RD.scoreType = (diprofData::EMultiAlScore)formIValue("multiScore", diprofData::eScoreSum);

    RD.crossDic = &CrossCov;

    RD.minNonOverlappingSubAlignmentLength = formIValue("minNonOverlappingSubAlignmentLength", 20);


    RD.jumpRangeStart= formIValue("jumpRangeStart", sIdxMax);
    RD.jumpRangeEnd= formIValue("jumpRangeEnd", sIdxMax);
    RD.maximumChewbackLeft = formIValue("maximumChewback",0);
    RD.maximumChewbackRight = formIValue("maximumChewbackRight",0);
    formValue("jumpRangeRelativeTo", &RD.jumpRangeRelativeTo,0);


    RD.progressStart = progress100Start = 0;
    RD.progressEnd = progress100End = 60;
    for(idx is = 0; is < numAlChunks; ++is) {
        idx start = 0, end = hiveal->dimAl();
        RD.startAl = start;
        RD.endAl = end;
        RD.obj = (void *) this;
        if( !is ) {
            RD.crossDic = &CrossCov;
        }
        cntFound += hiveal->iterateAlignments(0, start, end - start, -2, (sBioal::typeCallbackIteratorFunction) &similCumulator, (sBioal::ParamsAlignmentIterator *) &PA, 0, 0, &qrySortAlInd);
    }

    logOut(eQPLogType_Info, "Analyzing results\n");
    reqProgress(cntFound, 100, 100);

    if( !isLastInMasterGroup() ) {
        reqSetStatus(req, eQPReqStatus_Done);
        reqSetProgress(cntFound, 100, 100);
        logOut(eQPLogType_Info, "Done processing %" DEC " request for execution\n", req);
        return 0;
    }


    sVec<idx> reqList;
    grp2Req(grpId, &reqList, vars.value("serviceName"));

    idx taxAndLineage_info = formIValue("taxAndLineageInfo",2);
    sStr ionP, error_log;
    if( sviolin::SpecialObj::findTaxDbIonPath(ionP, *user, 0, 0, &error_log) ) {
        taxion = new sTaxIon(ionP.ptr());
    }
    if (!taxion) {
        taxAndLineage_info = 0;
    }

    RD.minPeakCov = formIValue("minPeakCov", 10);

    const char * st= formValue("minSameRefDist" );
    if(st && strcmp(st,"none")==0)RD.minSameRefDist = sIdxMax;
    else RD.minSameRefDist = st ? atoidx (st)  :0;

    RD.windowSize = formIValue("windowSize", 0);
    RD.influence = formRValue("influence", 0.3);
    RD.threshold = formIValue("threshold", 2);
    sVec<crshitVec> crossCovVec;

    crossCovVec.resize(CrossCov.dim());

    sVec<crshit> temp_cv;
    crshit * temp_c = 0;
    for(idx ix = 0; ix < CrossCov.dim(); ++ix) {
        temp_c = temp_cv.add();
        RD.parseCrossRefPos((const char *) CrossCov.id(ix), temp_c->sub, temp_c->pos, temp_c->dir);
    }

    sVec<idx> ccInd;
    ccInd.resize(CrossCov.dim());
    sSort::sortSimpleCallback(compareCrsHit, 0, temp_cv.dim(), temp_cv.ptr(), ccInd.ptr());
    idx progressLeft = 100 - progress100End;
    progress100Start = progress100End;
    progress100End = progress100End+progressLeft/5;

    idx subId, pos, dir;
    for(idx ix = 0; ix < CrossCov.dim(); ++ix) {
        sVec<idx> cr2Ind;
        if( !reqProgress(ix, ix, CrossCov.dim()) ) {
            return 0;
        }
        sDic<cov> * crcv = CrossCov.ptr(ccInd[ix]);
        crshitVec * cr1 = crossCovVec.ptrx(ix);

        cr1->poshit.set( temp_cv[ccInd[ix]] );
        cr1->poshit.hits.reset();
        for(idx ic = 0; ic < crcv->dim(); ++ic) {
            cr1->poshit.hits.add(crcv->ptr(ic));
        }

        sVec<crshit> t_crshits;
        for(idx iy = 0; iy < crcv->dim(); ++iy) {
            cov * crhits = crcv->ptr(iy);
            crshit * cr2 = t_crshits.add();
            cr2->hits.set(crhits);
            if( RD.parseCrossRefPos((const char *) crcv->id(iy), subId, pos, dir) ){
                cr2->pos = pos;
                cr2->sub = subId;
                cr2->dir = dir;
            }
        }
        cr2Ind.resize(t_crshits.dim());
        sSort::sortSimpleCallback(compareCrsHit, 0, t_crshits.dim(), t_crshits.ptr(),cr2Ind.ptr());

        for(idx iy = 0; iy < t_crshits.dim(); ++iy) {
            crossCovVec.ptr(ix)->crshits.add()->set(t_crshits[cr2Ind[iy]]);
        }
    }

    sStr bufPath;
    sDic<sMex::Pos> readLU;

    const char * cr_flnm = "_.vec";
    bufPath.cut0cut();
    const char * dstPath = sQPrideProc::reqAddFile(bufPath, cr_flnm);

    if( !dstPath ) {
        reqSetProgress(req, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        errS.printf("%s : Failed to add DI profiler file %s for %s", sRC(sRC::eCreating, sRC::eFile, sRC::eFile, sRC::eFailed).print(), cr_flnm, objs[0].IdStr());
        logOut(eQPLogType_Warning, errS);
        reqSetInfo(reqId, eQPInfoLevel_Warning, errS);
        return 0;
    }
    sVec<idx> readIDs(sMex::fSetZero,dstPath);

    sRC rc1 = printCrossCovResults(crossCovVec, readIDs, readLU, "di-profile.csv", &Sub,0,0,RD.minSameRefDist, taxAndLineage_info);

    progress100Start = progress100End;
    progress100End = progress100End+4*progressLeft/5;
    if( !peakFiltration(crossCovVec, &RD) ) {
        reqSetProgress(req, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        errS.printf("%s : No DIs detected for %s", sRC(sRC::eRunning, sRC::eProcess, sRC::eResult, sRC::eEmpty).print(), objs[0].IdStr());
        logOut(eQPLogType_Warning, errS);
        reqSetInfo(reqId, eQPInfoLevel_Warning, errS);
        return 0;
    }
    sRC rc2 = printCrossCovResults(crossCovVec, readIDs, readLU, "filtered-di-profile.csv", &Sub, true, RD.minPeakCov,RD.minSameRefDist, taxAndLineage_info);


    cr_flnm = "_.dic";
    dstPath = sQPrideProc::reqAddFile(bufPath, cr_flnm);

    if( !dstPath ) {
        reqSetProgress(req, 100, 100);
        reqSetStatus(req, eQPReqStatus_ProgError);
        errS.printf("%s : Failed to add DI profiler file %s for %s", sRC(sRC::eRunning, sRC::eProcess, sRC::eResult, sRC::eEmpty).print(), cr_flnm, objs[0].IdStr());
        logOut(eQPLogType_Warning, errS);
        reqSetInfo(reqId, eQPInfoLevel_Warning, errS);
        return 0;
    }
    sFil readLUso(dstPath);
    readLU.serialOut(readLUso);

    if( rc1 != sRC::zero && rc2 != sRC::zero ) {
        if( rc1 != sRC::zero )
            errS.printf("%s", rc1.print());
        if( rc1 != sRC::zero )
            errS.printf(" %s", rc2.print());
        logOut(eQPLogType_Warning, errS);
        reqSetInfo(reqId, eQPInfoLevel_Warning, errS);
        reqSetStatus(req, eQPReqStatus_ProgError);
    } else {
        reqSetProgress(req, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    }

    if (taxion) {
        delete taxion;
    }
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    DnaDIprofiler backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-diprofiler", argv[0]));
    return (int) backend.run(argc, argv);
}

