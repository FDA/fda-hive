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
#include <ssci/math/geom/dist.hpp>
#include <ssci/math/clust/clust2.hpp>
#include <ulib/uquery.hpp>
#include <violin/violin.hpp>

#include <qpsvc/archiver.hpp>

#include <stdio.h>
#include <assert.h>

#define CLUST_MAX_FILES 512

class validPositionList {
private:
    sVec<uint16_t> _posbuf;
    idx _delta;
    bool _needSummary;
    real _minFreq;

    struct positionSummary {
        char consensus;
        char consensus0;
        char consensus1;
        char maxFreq;
        positionSummary() { sSet(this, 0); }
    };
    sVec<positionSummary> _summaries;

    enum EPositionBits {
        fFreq0 = 1 << 0,
        fFreq1 = 1 << 1,
        fFreq2 = 1 << 2,
        fFreq3 = 1 << 3,
        fFreq4 = 1 << 4,
        fFreq5 = 1 << 5,
        fDeltaMaxFreqMeetsThreshold = 1 << 6,
        fNotAllMaxFreqMeetsThreshold = 1 << 7,
        fUsed = 1 << 8,
        fSomeCoverage = 1 << 9,
        fNotAllCoverage = 1 << 10,
        fNotAllSameConsensus1 = 1 << 11,
        fNotAllSameConsensus2 = 1 << 12,
        fNotAllSameConsensusReference = 1 << 13,
        fConsensusSet = 1 << 14,
        fNotAllSameMaxFreq = 1 << 15
    };

public:
    validPositionList(idx len=0, bool needSummary=false, idx delta = 0, real minFreq = 0.0)
    {
        init(len, needSummary, delta, minFreq);
    }

    void init(idx len, bool needSummary=false, idx delta = 0, real minFreq = 0.0)
    {
        _needSummary = needSummary;
        _delta = delta;
        _minFreq = minFreq;

        if( len ) {
            _posbuf.init(sMex::fExactSize|sMex::fSetZero);
            _summaries.init(sMex::fExactSize|sMex::fSetZero);
            idx expect_size = len + _delta + 2;
            _posbuf.resize(expect_size);
            if( _needSummary ) {
                _summaries.resize(expect_size);
            }
        }
        _posbuf.mex()->flags = sMex::fBlockDoubling | sMex::fSetZero;
        _summaries.mex()->flags = sMex::fBlockDoubling | sMex::fSetZero;
    }


    void set(idx recIdx, bool matchMaxFreq, bool hasFreq[6], bool matchCov, sBioseqSNP::SNPRecord &rec, real maxFreq)
    {
        assert(recIdx >= 0);

        idx oldsize = _posbuf.dim();
        idx newsize = recIdx + _delta + 1;
        if( newsize > oldsize ) {
            _posbuf.resize(newsize);
            if( _needSummary ) {
                _summaries.resize(newsize);
            }
        }

        _posbuf[recIdx] |= fUsed;

        for(idx i=0; i<6; i++) {
            if( hasFreq[i] ) {
                _posbuf[i] |= (fFreq0 << i);
            }
        }

        if( matchMaxFreq ) {
            for (idx pos = sMax<idx>(0, recIdx-_delta); pos <= recIdx+_delta; pos++) {
                _posbuf[pos] |= fDeltaMaxFreqMeetsThreshold;
            }
        } else if( _needSummary ) {
            _posbuf[recIdx] |= fNotAllMaxFreqMeetsThreshold;
        }

        if( matchCov ) {
            _posbuf[recIdx] |= fSomeCoverage;
        } else {
            _posbuf[recIdx] |= fNotAllCoverage;
        }

        if (_needSummary) {
            char myconsensus = rec.consensus;
            char myconsensus0;
            char myconsensus1;

            idx letNum0=-1, letNum1=-1;
            idx cnt0=0, cnt1=0;
            for (idx let=0; let<=3; let++) {
                if (rec.atgc[let] > cnt0) {
                    cnt1 = cnt0;
                    cnt0 = rec.atgc[let];
                    letNum1 = letNum0;
                    letNum0 = let;
                }
                else if (rec.atgc[let] > cnt1) {
                    cnt1 = rec.atgc[let];
                    letNum1 = let;
                }
            }
            if (letNum0 > letNum1) { idx t = letNum1; letNum1 = letNum0; letNum0 = t; t=cnt1; cnt1=cnt0; cnt0=t;}
            myconsensus0 = (letNum0>=0)?(sBioseq::mapRevATGC[letNum0]):(rec.consensus);
            myconsensus1 = (letNum1>=0)?(sBioseq::mapRevATGC[letNum1]):(sBioseq::mapRevATGC[letNum0]);

            if (_posbuf[recIdx] & fConsensusSet) {
                myconsensus = _summaries[recIdx].consensus;

                if( _minFreq > 0 ) {
                    if( maxFreq * 100 < _minFreq )
                        maxFreq = 0;
                    if( _summaries[recIdx].maxFreq < _minFreq )
                        _summaries[recIdx].maxFreq = 0;
                }

                if (fabs(maxFreq * 100 - _summaries[recIdx].maxFreq) > 100) {
                    _posbuf[recIdx] |= fNotAllSameMaxFreq;
                }
            } else {
                _summaries[recIdx].consensus = rec.consensus;
                _summaries[recIdx].consensus0 = (letNum0>=0)?(sBioseq::mapRevATGC[letNum0]):rec.consensus;
                _summaries[recIdx].consensus1 = (letNum1>=0)?(sBioseq::mapRevATGC[letNum1]):rec.consensus;
                _summaries[recIdx].maxFreq = 100 * maxFreq;
                _posbuf[recIdx] |= fConsensusSet;
            }

            if (myconsensus != rec.consensus) {
                _posbuf[recIdx] |= fNotAllSameConsensus1;
            }

            if ((_posbuf[recIdx] & fConsensusSet) && (( myconsensus0 != _summaries[recIdx].consensus0 ) || ( myconsensus1 != _summaries[recIdx].consensus1 ))) {
                _posbuf[recIdx] |= fNotAllSameConsensus2;
            }

            if (rec.letter != rec.consensus || rec.letter != myconsensus) {
                _posbuf[recIdx] |= fNotAllSameConsensusReference;
            }
        }

    }

    inline idx dim() const { return _posbuf.dim(); }

    inline idx isUsed(idx recIdx) const
    {
        return recIdx >= 0 && recIdx < dim() && (_posbuf[recIdx] & fUsed);
    }

    bool meetsThreshold(idx recIdx, bool allowPartialCoverage) const
    {
        return isUsed(recIdx) &&
               (_posbuf[recIdx] & fDeltaMaxFreqMeetsThreshold) &&
               (allowPartialCoverage ? (_posbuf[recIdx] & fSomeCoverage) : !(_posbuf[recIdx] & fNotAllCoverage));
    }

    bool meetsThresholdACGT(idx recIdx, idx acgtIdx, bool allowPartialCoverage) const
    {
        return isUsed(recIdx) &&
               (_posbuf[recIdx] & fDeltaMaxFreqMeetsThreshold) &&
               (_posbuf[recIdx] & (fFreq0 << acgtIdx)) &&
               (allowPartialCoverage ? (_posbuf[recIdx] & fSomeCoverage) : !(_posbuf[recIdx] & fNotAllCoverage));
    }

    bool hasSameConsensus(idx recIdx) const
    {
        if (recIdx < 0 || recIdx >= dim())
            return false;

        return !(_posbuf[recIdx] & fNotAllSameConsensus1);
    }

    bool hasSame2Consensus(idx recIdx) const
    {
        if (recIdx < 0 || recIdx >= dim())
            return false;

        return !(_posbuf[recIdx] & fNotAllSameConsensus2);
    }

    bool hasSameConsensusReference(idx recIdx) const
    {
        if (recIdx < 0 || recIdx >= dim())
            return false;

        return !(_posbuf[recIdx] & fNotAllSameConsensusReference);
    }

    bool hasSameMaxFreq(idx recIdx) const
    {
        if (recIdx < 0 || recIdx >= dim())
            return false;

        return !(_posbuf[recIdx] & fNotAllSameMaxFreq);
    }

    bool hasAllMeetFreqThreshold(idx recIdx) const
    {
        if (recIdx < 0 || recIdx >= dim())
            return false;

        return !(_posbuf[recIdx] & fNotAllMaxFreqMeetsThreshold);
    }
};

struct SNPprofileHandleSet
{
    private:
        sQPrideProc * _proc;
        sFilPool * _pool;
        sHiveId _profiler_id;
        bool _is_heptagon;
        idx _heptagon_handle;
        idx _heptagon_noise_handle;
        idx _heptagon_noise_prev_iref;
        idx _heptagon_noise_offset;
        sVec<idx> _hexagon_handles;
        sVec<idx> _hexagon_noise_handles;

    public:
        SNPprofileHandleSet() : _pool(0), _hexagon_handles(sMex::fExactSize), _hexagon_noise_handles(sMex::fExactSize)
        {
            _proc = 0;
            _is_heptagon = false;
            _heptagon_handle = _heptagon_noise_handle = _heptagon_noise_prev_iref = -1;
            _heptagon_noise_offset = 0;
        }
        idx initHexagon(sQPrideProc & proc, const sUsrObj & profiler_obj, sFilPool * pool, idx * references, idx num_references, bool with_noise)
        {
            _proc = &proc;
            _pool = pool;
            _profiler_id = profiler_obj.Id();
            _is_heptagon = false;
            _heptagon_handle = pool->invalidHandle();
            _hexagon_handles.resize(num_references);
            _hexagon_noise_handles.resize(num_references);
            sStr path_buf;
            idx ret = 0;

            bool per_profile_noise = false;
            idx first_valid_iref = -1;

            for( idx iref=0; iref<num_references; iref++) {
                path_buf.cut(0);
                if( const char * path = profiler_obj.getFilePathname(path_buf, "SNPprofile-%" DEC ".csv", references[iref]) ) {
                    _hexagon_handles[iref] = _pool->declare(path, sMex::fReadonly);
                    _hexagon_noise_handles[iref] = _pool->invalidHandle();
                    proc.logOut(sQPrideBase::eQPLogType_Debug, "Added %s to pool", path);
                    ret++;

                    if( first_valid_iref < 0 ) {
                        first_valid_iref = iref;
                        if( with_noise ) {
                            path_buf.cut(0);
                            if( const char * noise_path = profiler_obj.getFilePathname(path_buf, "NoiseIntegral.csv") ) {
                                per_profile_noise = true;
                                _hexagon_noise_handles[iref] = _pool->declare(noise_path, sMex::fReadonly);
                                proc.logOut(sQPrideBase::eQPLogType_Debug, "Added %s to pool", noise_path);
                            }
                        }
                    }

                    if( with_noise ) {
                        if( per_profile_noise ) {
                            if( iref > first_valid_iref ) {
                                _hexagon_noise_handles[iref] = _hexagon_noise_handles[first_valid_iref];
                            }
                        } else {
                            path_buf.cut(0);
                            if( const char * noise_path = profiler_obj.getFilePathname(path_buf, "NoiseIntegral-%" DEC ".csv", references[iref]) ) {
                                _hexagon_noise_handles[iref] = _pool->declare(noise_path, sMex::fReadonly);
                                proc.logOut(sQPrideBase::eQPLogType_Debug, "Added %s to pool", noise_path);
                            } else {
                                proc.logOut(sQPrideBase::eQPLogType_Warning, "Not added to pool: profiler %s does not have NoiseIntegral-%" DEC ".csv", profiler_obj.Id().print(), references[iref]);
                            }
                        }
                    }
                } else {
                    _hexagon_handles[iref] = _pool->invalidHandle();
                    _hexagon_noise_handles[iref] = _pool->invalidHandle();
                    proc.logOut(sQPrideBase::eQPLogType_Debug, "Not added to pool: profiler %s does not have SNPprofile-%" DEC ".csv", profiler_obj.Id().print(), references[iref]);
                }
            }
            return ret;
        }
        idx initHeptagon(sQPrideProc & proc, const sUsrObj & profiler_obj, sFilPool * pool, bool with_noise)
        {
            _pool = pool;
            _is_heptagon = true;
            sStr path_buf;
            if( const char * path = profiler_obj.getFilePathname(path_buf, "SNPprofile.csv") ) {
                _heptagon_handle = _pool->declare(path, sMex::fReadonly);
                _heptagon_noise_handle = _pool->invalidHandle();
                proc.logOut(sQPrideBase::eQPLogType_Debug, "Added %s to pool", path);

                if( with_noise ) {
                    path_buf.cut(0);
                    if( profiler_obj.getFilePathname(path_buf, "NoiseIntegral.csv") && sFile::size(path_buf) ) {
                        _heptagon_noise_handle = _pool->declare(path, sMex::fReadonly);
                        proc.logOut(sQPrideBase::eQPLogType_Debug, "Added %s to pool", path);
                    } else {
                        proc.logOut(sQPrideBase::eQPLogType_Warning, "Not added to pool: heptagon %s does not have NoiseIntegral.csv", profiler_obj.Id().print());
                    }
                }

                return 1;
            } else {
                _heptagon_handle = _pool->invalidHandle();
                _heptagon_noise_handle = _pool->invalidHandle();
                proc.logOut(sQPrideBase::eQPLogType_Debug, "Not added to pool: heptagon %s does not have SNPprofile.csv", profiler_obj.Id().print());
                return 0;
            }
        }
        bool isHeptagon() const
        {
            return _is_heptagon;
        }
        sFilPool * getPool()
        {
            return _pool;
        }
        bool validHandle(idx iref)
        {
            idx handle = _is_heptagon ? _heptagon_handle : _hexagon_handles[iref];
            return _pool->validHandle(handle);
        }
        sFil * requestFile(idx iref)
        {
            idx handle = _is_heptagon ? _heptagon_handle : _hexagon_handles[iref];
            return _pool->request(handle);
        }
        void releaseFile(idx iref)
        {
            idx handle = _is_heptagon ? _heptagon_handle : _hexagon_handles[iref];
            return _pool->release(handle);
        }
        bool parseNoise(idx iref, idx isub, real noiseCutoffs[6][6])
        {
            bool ret = true;

            noiseCutoffs[0][0] = -1;

            if( _is_heptagon ) {
                if( _pool->validHandle(_heptagon_noise_handle) ) {
                    sFil * fil = _pool->request(_heptagon_noise_handle);
                    idx offset = iref > _heptagon_noise_prev_iref ? _heptagon_noise_offset : 0;
                    _heptagon_noise_prev_iref = iref;
                    const char * noise_next = sBioseqSNP::snpConcatenatedNoiseCutoffsFromIntegralCSV(fil->ptr(offset), fil->last(), noiseCutoffs, isub);
                    if( noise_next ) {
                        _heptagon_noise_offset = noise_next - fil->ptr(0);
                        _heptagon_noise_prev_iref = iref;
                    } else {
                        if( _proc ) {
                            _proc->logOut(sQPrideBase::eQPLogType_Error, "Failed to find subject %" DEC " in noise file for heptagon %s", isub, _profiler_id.print());
                        }
                        ret = false;
                    }
                    _pool->release(_heptagon_noise_handle);
                }
            } else {
                const idx handle = _hexagon_noise_handles[iref];
                if( _pool->validHandle(handle) ) {
                    sFil * fil = _pool->request(handle);
                    if( !sBioseqSNP::snpNoiseCutoffsFromIntegralCSV(fil->ptr(), fil->last(), noiseCutoffs) ) {
                        if( _proc ) {
                            _proc->logOut(sQPrideBase::eQPLogType_Error, "Failed to parse noise file for subject %" DEC " for profiler %s", isub, _profiler_id.print());
                        }
                        ret = false;
                    }
                    _pool->release(handle);
                }
            }
            return ret;
        }
};

class sPooledSNPCSVFreqIter: public sSNPCSVFreqIter<sPooledSNPCSVFreqIter>
{
private:
    SNPprofileHandleSet * _handle_set;

    sFil * _file;
    const char * _buf_prev;
    bool _buf_initialized;

    const idx * _references;
    idx _num_references;

    idx _pos;
    idx _iref;

    const validPositionList * _valid_position_lists;

    friend class sSNPCSVFreqIter<sPooledSNPCSVFreqIter>;

    void init(const sPooledSNPCSVFreqIter &rhs)
    {
        sSNPCSVFreqIter<sPooledSNPCSVFreqIter>::init(rhs);

        _handle_set = rhs._handle_set;
        _references = rhs._references;
        _num_references = rhs._num_references;
        _pos = rhs._pos;
        _iref = rhs._iref;
        _valid_position_lists = rhs._valid_position_lists;

        _file = 0;
        _buf_initialized = false;
        _buf = _bufend  = _buf_prev = 0;

        if (rhs._file) {
            requestData_impl();
        }
    }

    void readRec_impl()
    {
        if( _iref >= _num_references ) {
            return;
        }

        do {
            if( !readyData() ) {
                if( !_file && _handle_set->validHandle(_iref) ) {
                    _file = _handle_set->requestFile(_iref);
                }

                if( _file ) {
                    if( _buf_concatenated ) {
                        assert(_iref == 0 || _references[_iref] > _references[_iref - 1]);
                        this->_buf = _buf_prev = sBioseqSNP::binarySearchReference(_buf_prev ? _buf_prev : _file->ptr(), _file->last(), _references[_iref], false);
                    } else {
                        this->_buf = _buf_prev = _file->ptr();
                    }
                    this->_bufend = _file->last();
                    _buf_initialized = true;
                    _handle_set->parseNoise(_iref, _references[_iref], _params._snpparams.noiseCutoffs);
                } else {
                    _buf = _bufend = _buf_prev = 0;
                }
                _i = _j = 0;
                _rec.reset();
            }

            _buf_prev = this->_buf;
            sSNPCSVFreqIter<sPooledSNPCSVFreqIter>::readRec_default();

            if( _buf_concatenated && sSNPCSVFreqIter<sPooledSNPCSVFreqIter>::validData_default() ) {
                assert(_rec.iSub == _references[_iref]);
            }

            if (!sSNPCSVFreqIter<sPooledSNPCSVFreqIter>::validData_default()) {
                if( _file && !_buf_concatenated ) {
                    _handle_set->releaseFile(_iref);
                    _file = 0;
                    _buf = _bufend = _buf_prev = 0;
                }
                _buf_initialized = false;
                _iref++;
            }
        } while( _iref < _num_references && (!readyData() || (_valid_position_lists && !_valid_position_lists[_iref].meetsThreshold(this->_rec.position, false))) );

        if( _iref >= _num_references ) {
            releaseData_impl();
        }
    }

public:
    void requestData_impl()
    {
        if( readyData() || _iref >= _num_references ) {
            return;
        }

        readRec();
    }

    void releaseData_impl()
    {
        if( _file ) {
            _handle_set->releaseFile(_iref);
            _file = 0;
            _buf = _bufend = _buf_prev = 0;
        }
        _buf_initialized = false;
    }

    inline bool readyData_impl() const { return _file && _buf_initialized; }

    inline bool validData_impl() const { return _iref < _num_references && sSNPCSVFreqIter<sPooledSNPCSVFreqIter>::validData_default(); }

    inline idx pos_impl() const { return _pos; }
    inline idx segment_impl() const { return _iref; }
    inline idx segmentPos_impl() const { return sSNPCSVFreqIter<sPooledSNPCSVFreqIter>::segmentPos_default(); }

    sPooledSNPCSVFreqIter(SNPprofileHandleSet * handle_set = 0, const idx * references = 0, idx num_references = 0, const sSNPCSVFreqIterParams *params = 0, const validPositionList *valid_position_lists = 0):
        sSNPCSVFreqIter<sPooledSNPCSVFreqIter>(0, 0, handle_set && handle_set->isHeptagon(), params)
    {
        _handle_set = handle_set;
        _file = 0;
        _buf_initialized = false;
        _references = references;
        _num_references = num_references;
        _pos = 0;
        _iref = 0;
        _buf_prev = 0;
        _valid_position_lists = valid_position_lists;
    }

    sPooledSNPCSVFreqIter(const sPooledSNPCSVFreqIter &rhs) { init (rhs); }
    sPooledSNPCSVFreqIter& operator=(const sPooledSNPCSVFreqIter &rhs)
    {
        releaseData_impl();
        init(rhs);
        return *this;
    }
    inline sPooledSNPCSVFreqIter* clone_impl() const { return new sPooledSNPCSVFreqIter(*this); }
    ~sPooledSNPCSVFreqIter() { releaseData_impl(); }
    inline sPooledSNPCSVFreqIter& increment_impl()
    {
        do {
            requestData_impl();
            sSNPCSVFreqIter<sPooledSNPCSVFreqIter>::increment_default();
            _pos++;
        } while( validData_impl() && !(_valid_position_lists && _valid_position_lists[_iref].meetsThreshold(this->_rec.position, false)) );
        return *this;
    }

    inline bool equals_impl(const sPooledSNPCSVFreqIter &rhs) const { return _iref == rhs._iref && _i == rhs._i && _j == rhs._j; }
    inline bool lessThan_impl(const sPooledSNPCSVFreqIter &rhs) const
    {
        if( _iref != rhs._iref ) {
            return _iref < rhs._iref;
        } else if( _i != rhs._i ) {
            return _i < rhs._i;
        } else {
            return _j < rhs._j;
        }
    }
    inline bool greaterThan_impl(const sPooledSNPCSVFreqIter &rhs) const
    {
        if( _iref != rhs._iref ) {
            return _iref > rhs._iref;
        } else if( _i != rhs._i ) {
            return _i > rhs._i;
        } else {
            return _j > rhs._j;
        }
    }

    void updatePosition(validPositionList * valid_position_lists)
    {
        requestData();
        bool has_freq[6];
        for(idx i=0; i<6; i++) {
            has_freq[i] = getDenoisedFreq(i);
        }
        valid_position_lists[_iref].set(this->_rec.position, meetsThreshold(this->_maxFreq), has_freq, meetsCoverage(), this->_rec, this->_maxFreq);
    }

    inline real getMaxFreq() const { return this->_maxFreq; }

    inline bool hasSameConsensus() { return _valid_position_lists[_iref].hasSameConsensus(this->_rec.position); }
    inline bool hasSame2Consensus() { return _valid_position_lists[_iref].hasSame2Consensus(this->_rec.position); }
    inline bool hasSameConsensusReference() { return _valid_position_lists[_iref].hasSameConsensusReference(this->_rec.position); }
    inline bool hasSameMaxFreq() { return _valid_position_lists[_iref].hasSameMaxFreq(this->_rec.position); }
    inline bool hasAllMeetFreqThreshold() { return _valid_position_lists[_iref].hasAllMeetFreqThreshold(this->_rec.position); }
};

class ShrunkIter : public sIter<char, ShrunkIter> {
protected:
    sPooledSNPCSVFreqIter _it;
    const validPositionList * _valid_position_lists;
    idx _num_references;
    bool _allowPartialCoverage;
    idx _iref;
    udx _pos;
    idx _total_pos;
    idx _num_increments;
    char _c;
    bool _haveRec;

    void readRec()
    {
        if( _haveRec )
            return;

        while( validData() && !_valid_position_lists[_iref].meetsThreshold(_pos, _allowPartialCoverage) ) {
            _pos++;
            _total_pos++;
            if( _pos >= (udx)_valid_position_lists[_iref].dim() ) {
                _pos = 0;
                _iref++;
            }
        }

        while( _it.valid() && (_it.segment() < _iref || (_it.segment() == _iref && _it.getSNPRecord().position < _pos)) ) {
            _it.nextRecord();
        }

        if (validData() && _it.valid() && _it.segment() == _iref && _it.getSNPRecord().position == _pos)
            _c =  _it.getSNPRecord().consensus;
        else
            _c = '-';

        _haveRec = true;
    }

public:
    ShrunkIter(sPooledSNPCSVFreqIter &it, const validPositionList * valid_position_lists, idx num_references, bool allowPartialCoverage): _it(it)
    {
        _valid_position_lists = valid_position_lists;
        _num_references = num_references;
        _allowPartialCoverage = allowPartialCoverage;
        _iref = 0;
        _pos = 0;
        _total_pos = 0;
        _num_increments = 0;
        _c = '-';
        _haveRec = false;
    }

    ShrunkIter(const ShrunkIter &rhs): _it(rhs._it)
    {
        _valid_position_lists = rhs._valid_position_lists;
        _num_references = rhs._num_references;
        _allowPartialCoverage = rhs._allowPartialCoverage;
        _iref = rhs._iref;
        _pos = rhs._pos;
        _total_pos = rhs._total_pos;
        _num_increments = rhs._num_increments;
        _c = rhs._c;
        _haveRec = rhs._haveRec;
    }

    virtual ~ShrunkIter() { }

    inline bool readyData_impl() const { return _haveRec; }
    inline void requestData_impl()
    {
        _it.requestData();
        readRec();
    }
    inline void releaseData_impl() { _it.releaseData(); }
    inline bool validData_impl() const
    {
        return _iref < _num_references && _pos < (udx)_valid_position_lists[_iref].dim();
    }
    inline idx segment_impl() const { return _iref; }
    inline idx segmentPos_impl() const { return _pos; }
    inline idx pos_impl() const { return _total_pos; }
    inline idx numIncrements() const { return _num_increments; }
    ShrunkIter* clone_impl() const { return new ShrunkIter(*this); }
    inline ShrunkIter& increment_impl()
    {
        _pos++;
        _total_pos++;
        _num_increments++;
        if( _pos >= (udx)_valid_position_lists[_iref].dim() ) {
            _pos = 0;
            _iref++;
        }
        _haveRec = false;
        readRec();
        return *this;
    }
    inline char dereference_impl() const { return _c; }
    inline real getDenoisedFreq(idx atgcIndex) const
    {
        return _c == '-' ? 0 : _it.getDenoisedFreq(atgcIndex);
    }
    inline real getMaxFreq() const
    {
        return _c == '-' ? 0 : _it.getMaxFreq();
    }
    inline idx coverage() const
    {
        return _c == '-' ? 0 : _it.getSNPRecord().coverage();
    }
    inline idx atgcCount(idx atgcIndex) const
    {
        if( _c == '-' || atgcIndex < 0 || atgcIndex >= 6 ) {
            return 0;
        } else if( atgcIndex < 4 ) {
            return _it.getSNPRecord().atgc[atgcIndex];
        } else {
            return _it.getSNPRecord().indel[atgcIndex - 4];
        }
    }
};

static const char * algorithmNames =
    "neighbor-joining" _
    "fast-neighbor-joining" _
    "single-link" _
    "complete-link" _
    "SNPcompare" _
    "shrunk-only" __;

enum eAlgorithms {
    eNeighborJoining = 0,
    eFastNeighborJoining,
    eSingleLink,
    eCompleteLink,
    eSNPcompare,
    eShrunkOnly
};

enum eSNPCompareAlgorithms {
    eSNPCompareAlgorithm_CompareConsensus = 0,
    eSNPCompareAlgorithm_FreqCutOff,
    eSNPCompareAlgorithm_Compare2Consensus
};

class DnaClust : public sQPrideProc
{
protected:
    class DnaClustContext
    {
    protected:
        DnaClust &_proc;
        idx _req;

        sVec<sHiveId> _profileIDs;
        sVec<sHiveId> _genomeIDs;
        sVec<idx> _references;
        eAlgorithms _algorithm;
        eSNPCompareAlgorithms _snpCompareAlgorithm;
        sDist<real, sPooledSNPCSVFreqIter> *_dist;
        bool _filterNoise;
        sSNPCSVFreqIterParams _iterParams;
        bool _thresholdAny;
        idx _thresholdWindow;
        bool _shrunkGenome;
        bool _shrunkSNPClass;
        bool _shrunkSNPPos;
        bool _shrunkOnly;
        enum {
            eSaveShrunkSNP_frequency,
            eSaveShrunkSNP_coverage
        } _saveShrunkSNPClassCellType;
        enum {
            eSaveShrunkSNP_position,
            eSaveShrunkSNP_atgc,
            eSaveShrunkSNP_atgcindel
        } _saveShrunkSNPClassColType;
        sStr _shrunkSNPClassProfilesTitle;
        qlang::ast::Node *_saveShrunkSNPClassColLabelTemplate;
        qlang::ast::Node *_profileLabelTemplate;

        sHiveseq * _referenceSeq;
        sVec<sPooledSNPCSVFreqIter> _iters;
        sFilPool _profilePool;
        sVec<SNPprofileHandleSet> _profileHandles;
        sVec<validPositionList> _positionLists;
        sHierarchicalClustering * _clust;

        bool loadProfileIDs();
        bool loadGenomeIDs();
        bool loadReferences();
        bool loadAlgorithm();
        bool loadThreshold();
        bool loadShrunk();

        bool loadNoiseIntegral(idx iprof, sUsrFile &profileObject, bool is_heptagon);

        bool reqInitFile(sMex & fil, const char * name, sStr * path_buf = 0)
        {
            static sStr local_path_buf;
            if( !path_buf ) {
                path_buf = &local_path_buf;
            }
            const char * path = _proc.reqAddFile(*path_buf, name);
            if( path && fil.init(path) && fil.ok() ) {
#ifdef _DEBUG
                _proc.logOut(eQPLogType_Trace, "Created %s", path);
#endif
                return true;
            } else {
                _proc.logOut(eQPLogType_Error, "Failed to create %s", name);
                return false;
            }
        }

    public:
        DnaClustContext(DnaClust & proc, idx req):
            _proc(proc), _req(req), _iters(sMex::fExactSize), _profilePool(CLUST_MAX_FILES), _profileHandles(sMex::fExactSize), _positionLists(sMex::fExactSize)
        {
            _algorithm = eNeighborJoining;
            _snpCompareAlgorithm = eSNPCompareAlgorithm_Compare2Consensus;
            _referenceSeq = NULL;
            _dist = NULL;
            _clust = NULL;
            _filterNoise = true;
            _thresholdAny = true;
            _thresholdWindow = 0;
            _shrunkGenome = false;
            _shrunkSNPClass = false;
            _shrunkSNPPos = false;
            _shrunkOnly = false;
            _saveShrunkSNPClassCellType = eSaveShrunkSNP_frequency;
            _saveShrunkSNPClassColType = eSaveShrunkSNP_atgcindel;
            _saveShrunkSNPClassColLabelTemplate = 0;
            _profileLabelTemplate = 0;
        }

        ~DnaClustContext()
        {
            delete _referenceSeq;
            delete _dist;
            delete _clust;
            delete _saveShrunkSNPClassColLabelTemplate;
            delete _profileLabelTemplate;
        }

        bool loadParams();
        bool saveExaminedReferences();
        void loadProfileIters();

        bool calcSNPCompare();
        bool calcShrunk();
        bool calcClust();

        eAlgorithms getAlgorithm() const { return _algorithm; }
        bool wantShrunk() const { return _shrunkGenome || _shrunkSNPClass || _shrunkSNPPos; }
        bool wantPositionLists() const { return _thresholdAny || wantShrunk(); }
    };

public:
    DnaClust(const char * defline00,const char * srv) : sQPrideProc(defline00,srv) {}
    virtual idx OnExecute(idx);
};

static idx nodePrintfCallback(sStr &out, sHierarchicalClustering &clust, idx x, void *param)
{
    sVec <sHiveId> *idList = static_cast<sVec<sHiveId>*>(param);
    if (x >= 0 && x < idList->dim())
        idList->ptr(x)->print(out);
    return 0;
}

#define DNA_CLUST_DEFAULT_ERROR "Clusterization process failed"

bool DnaClust::DnaClustContext::saveExaminedReferences()
{
    sStr path;
    sFil out;
    if( !reqInitFile(out, "examined-references.txt", &path) ) {
        return false;
    }

    sString::printfIVec(&out, &_references);
    out.printf("\n");
#ifdef _DEBUG
    printf("%s: %s", path.ptr(), out.ptr());
#endif
    return true;
}

bool DnaClust::DnaClustContext::loadProfileIDs()
{
    if (_proc.formHiveIdValues("profileID", _profileIDs) <= 0) {
        _proc.logOut(eQPLogType_Error, "Empty profileID list for %" DEC " request; terminating\n", _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Need a non-empty list of profiling results");
        return false;
    }

    sUsrQueryEngine engine(*_proc.user);
    sStr err;

    const char * profileLabelTemplateString = _proc.formValue("profileLabelTemplate");
    if( !profileLabelTemplateString || !*profileLabelTemplateString ) {
        profileLabelTemplateString = "$(profile): $(profile.name)";
    }

    if( engine.parseTemplate(profileLabelTemplateString, 0, &err) ) {
        _profileLabelTemplate = engine.getParser().releaseAstRoot();
    } else {
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Failed to parse profile label template: %s", err ? err.ptr() : "");
        return false;
    }
    return true;
}

bool DnaClust::DnaClustContext::loadGenomeIDs()
{
    if (_proc.formHiveIdValues("referenceID", _genomeIDs) <= 0) {
        _proc.logOut(eQPLogType_Error, "Empty referenceID genome list for %" DEC " request; terminating\n", _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Need a non-empty list of reference genomes");
        return false;
    }

    return true;
}

bool DnaClust::DnaClustContext::loadReferences()
{
    sStr genomeIDsStr, errStr;
    sHiveId::printVec(genomeIDsStr, _genomeIDs, ";");
    _referenceSeq = new sHiveseq(_proc.user, genomeIDsStr.ptr(), sBioseq::eBioModeShort, false, false, &errStr);
    if (_referenceSeq->dim() < 1) {
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "No data found for reference genome IDs %s%s%s", genomeIDsStr.ptr(), errStr.length() ? ": " : "", errStr.length() ? errStr.ptr() : "");
        return false;
    }
    errStr.cut0cut();

    bool incompleteOK = false;
    const char * tmpstr = _proc.formValue("incompleteReferences");
    if (tmpstr && !strcmp(tmpstr, "zero"))
        incompleteOK = true;

    sVec<idx> allRefCounts(sMex::fExactSize);
    allRefCounts.resize(_referenceSeq->dim() + 1);
    allRefCounts[0] = -1;
    bool haveExplicitRefList = false;

    if( const char * subjects = _proc.formValue("subjects") ) {
        sVec<idx> explicitRefs;
        sString::sscanfIVec(&explicitRefs, subjects);
        if( explicitRefs.dim() ) {
            haveExplicitRefList = true;
            for(idx ref=1; ref<allRefCounts.dim(); ref++) {
                allRefCounts[ref] = -1;
            }
            for(idx ir=0; ir<explicitRefs.dim(); ir++) {
                if( explicitRefs[ir] > 0 && explicitRefs[ir] < allRefCounts.dim() ) {
                    allRefCounts[explicitRefs[ir]] = 0;
                }
            }
        }
    }
    if( !haveExplicitRefList ) {
        for(idx ref=1; ref<allRefCounts.dim(); ref++) {
            allRefCounts[ref] = 0;
        }
    }

    sVec<bool> incrementRefCounts(sMex::fExactSize);
    sStr profileRefsBuf;
    for (idx i=0; i<_profileIDs.dim(); i++) {
        sUsrFile profileObject(_profileIDs[i], _proc.user);
        profileRefsBuf.cut(0);
        if( const char * profileRefs = profileObject.propGet00("subSet", &profileRefsBuf, ";") ) {
            for(idx ic=0; ic<profileRefsBuf.length(); ic++) {
                if( profileRefsBuf[ic] == ';' )
                    profileRefsBuf[ic] = 0;
            }
            incrementRefCounts.resize(allRefCounts.dim());
            for(idx ref=0; ref<allRefCounts.dim(); ref++) {
                incrementRefCounts[ref] = false;
            }
            for(const char * profileRef = profileRefs; profileRef && *profileRef; profileRef = sString::next00(profileRef)) {
                idx ref = atoidx(profileRef);
                if( ref > 0 && ref < allRefCounts.dim() && allRefCounts[ref] >= 0 ) {
                    incrementRefCounts[ref] = true;
                }
            }
            for(idx ref=1; ref<allRefCounts.dim(); ref++) {
                if( incrementRefCounts[ref] ) {
                    allRefCounts[ref]++;
                }
            }
        } else {
            for(idx ref=1; ref<allRefCounts.dim(); ref++) {
                if( allRefCounts[ref] >= 0 ) {
                    allRefCounts[ref]++;
                }
            }
        }
    }

    for (idx ref=1; ref<allRefCounts.dim(); ref++) {
        if( allRefCounts[ref] > 0 && (incompleteOK || allRefCounts[ref] == _profileIDs.dim()) ) {
            _references.vadd(1, ref);
        }
    }

    return true;
}

bool DnaClust::DnaClustContext::loadAlgorithm()
{
    idx tmp;
    sString::compareChoice(_proc.formValue("algorithm"), algorithmNames, &tmp, true, 0, true);
    _algorithm = static_cast<eAlgorithms>(tmp);
    _snpCompareAlgorithm = static_cast<eSNPCompareAlgorithms>(_proc.formIValue("SNPCompareAlgorithm", eSNPCompareAlgorithm_FreqCutOff));

    switch (_algorithm) {
    case eFastNeighborJoining:
        _clust = new sFastNeighborJoining;
        break;
    case eSingleLink:
        _clust = new sSingleLinkClustering;
        break;
    case eCompleteLink:
        _clust = new sCompleteLinkClustering;
        break;
    case eSNPcompare:
    case eShrunkOnly:
        _clust = 0;
        break;
    default:
        _clust = new sNeighborJoining;
    }

    if( _clust ) {
        sStr distanceName;
        _proc.formValue("distance", &distanceName);

        if( !strcmp(distanceName.ptr(), "manhattan") ) {
            _dist = new sManhattanDist<real, sPooledSNPCSVFreqIter>();
        } else if( !strcmp(distanceName.ptr(), "maximum") ) {
            _dist = new sMaximumDist<real, sPooledSNPCSVFreqIter>();
        } else if( !strcmp(distanceName.ptr(), "pnorm") ) {
            real p = _proc.formRValue("pValue");
            if( p < 1.0 ) {
                _proc.logOut(eQPLogType_Error, "Invalid pValue == %g, expected pValue >= 1.0 for %" DEC " request; terminating\n", p, _req);
                _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Expected parameter p >= 1.0, but %g was given", p);
                return false;
            }
            _dist = new sPNormDist<real, sPooledSNPCSVFreqIter>(p);
        } else if( !strcmp(distanceName.ptr(), "canberra") ) {
            _dist = new sCanberraDist<real, sPooledSNPCSVFreqIter>();
        } else if( !strcmp(distanceName.ptr(), "pearson") ) {
            bool uncentered = _proc.formIValue("pearsonUncentered");
            bool squared = _proc.formIValue("pearsonSquared");
            _dist = new sPearsonDist<real, sPooledSNPCSVFreqIter>(uncentered, squared);
        } else if( !strcmp(distanceName.ptr(), "cosine") ) {
            _dist = new sCosineDist<real, sPooledSNPCSVFreqIter>();
        } else {
            _dist = new sEuclideanDist<real, sPooledSNPCSVFreqIter>();
        }
    }

    return true;
}

bool DnaClust::DnaClustContext::loadThreshold()
{
    real minFreqThreshold = _proc.formRValue("minThresholdPercent", 0.0);
    _iterParams.setMinThreshold(minFreqThreshold / 100);
    _iterParams.setMaxThreshold(_proc.formRValue("maxThresholdPercent") / 100);
    _iterParams.setMinCoverage(_proc.formIValue("minCoverage"));

    if (!strcmp(_proc.formValue("noise"), "none"))
        _filterNoise = false;

    _thresholdAny = _proc.formBoolValue("thresholdAny");
    _thresholdWindow = _proc.formIValue("thresholdWindow");
    if (_thresholdWindow < 0) {
        _proc.logOut(eQPLogType_Error, "Invalid thresholdWindow = %" DEC ", expected >= 0 for %" DEC " request; terminating\n", _thresholdWindow, _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Expected non-negative position range parameter, but %" DEC " was given", _thresholdWindow);
        return false;
    } else if (_thresholdWindow > 0 && !_thresholdAny) {
        _proc.logOut(eQPLogType_Error, "If thresholdWindow > 0, expect thresholdAny to be true for %" DEC " request; terminating\n", _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Expected 'all profiles as long as one meets constraint' parameter to be enabled if a non-empty position range parameter was given");
        return false;
    }

#ifdef _DEBUG
    printf("SNP freq threshold: [%.0f%%, %.0f%%]; coverage: >= %" DEC "; thresholdAny: %s; thresholdWindow: %" DEC "\n", _iterParams.getMinThreshold()*100, _iterParams.getMaxThreshold()*100, _iterParams.getMinCoverage(), _thresholdAny?"true":"false", _thresholdWindow);
#endif

    return true;
}

bool DnaClust::DnaClustContext::loadShrunk()
{
    _shrunkGenome = _proc.formBoolValue("saveShrunk");
    _shrunkSNPClass = _proc.formBoolValue("saveShrunkSNPClass");
    _shrunkSNPPos = _proc.formBoolValue("saveShrunkSNPPos");
    _shrunkOnly = _proc.formBoolValue("saveShrunkOnly");

    if( sIsExactly(_proc.formValue("saveShrunkSNPClassCellType"), "frequency") ) {
        _saveShrunkSNPClassCellType = eSaveShrunkSNP_frequency;
    } else if( sIsExactly(_proc.formValue("saveShrunkSNPClassCellType"), "coverage") ) {
        _saveShrunkSNPClassCellType = eSaveShrunkSNP_coverage;
    }

    if( sIsExactly(_proc.formValue("saveShrunkSNPClassColType"), "position") ) {
        _saveShrunkSNPClassColType = eSaveShrunkSNP_position;
    } else if( sIsExactly(_proc.formValue("saveShrunkSNPClassColType"), "atgc") ) {
        _saveShrunkSNPClassColType = eSaveShrunkSNP_atgc;
    } else if( sIsExactly(_proc.formValue("saveShrunkSNPClassColType"), "atgcindel") ) {
        _saveShrunkSNPClassColType = eSaveShrunkSNP_atgcindel;
    }

    _proc.formValue("saveShrunkSNPClassProfilesTitle", &_shrunkSNPClassProfilesTitle);
    _shrunkSNPClassProfilesTitle.shrink00();
    if( !_shrunkSNPClassProfilesTitle.length() ) {
        _shrunkSNPClassProfilesTitle.addString("profile");
    }

    sUsrQueryEngine engine(*_proc.user);
    sStr err;

    const char * saveShrunkSNPClassColLabelTemplateString = _proc.formValue("saveShrunkSNPClassColLabelTemplate");
    if( !saveShrunkSNPClassColLabelTemplateString || !*saveShrunkSNPClassColLabelTemplateString ) {
        if( _saveShrunkSNPClassColType == eSaveShrunkSNP_position ) {
            saveShrunkSNPClassColLabelTemplateString = "isub=$(isub) pos=$(position)";
        } else {
            saveShrunkSNPClassColLabelTemplateString = "isub=$(isub) pos=$(position) $(letter)";
        }
    }

    if( engine.parseTemplate(saveShrunkSNPClassColLabelTemplateString, 0, &err) ) {
        _saveShrunkSNPClassColLabelTemplate = engine.getParser().releaseAstRoot();
    } else {
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Failed to parse shrunk SNP classification table column label template: %s", err ? err.ptr() : "");
        return false;
    }

    return true;
}

bool DnaClust::DnaClustContext::loadParams()
{
    if (!loadProfileIDs() || !loadGenomeIDs() || !loadReferences())
        return false;

    if (!loadAlgorithm() || !loadThreshold() || !loadShrunk())
        return false;

    return true;
}

void DnaClust::DnaClustContext::loadProfileIters()
{
    _profileHandles.resize(_profileIDs.dim());

    if (wantPositionLists()) {
        _positionLists.resize(_references.dim());
        for (idx ir=0; ir<_references.dim(); ir++) {
            _positionLists[ir].init(_referenceSeq->len(_references[ir]-1), getAlgorithm() == eSNPcompare, _thresholdWindow, _iterParams.getMinThreshold() * 100);
        }
    }

    sStr profile_path;

    _iters.resize(_profileIDs.dim());

    for (idx i=0; i<_profileIDs.dim(); i++) {
        _proc.reqProgress(-1, i, _profileIDs.dim());

        sUsrFile profileObject(_profileIDs[i], _proc.user);

        profile_path.cut(0);
        idx num_files_opened = 0;

        bool want_noise = false;
        if( _filterNoise ) {
            const char *profileNoiseFilterParams = profileObject.propGet("noiseFilterParams");
            if (!profileNoiseFilterParams || !strcmp(profileNoiseFilterParams, "none")) {
                want_noise = true;
            }
        }

        if( profileObject.getFilePathname(profile_path, "SNPprofile.csv") ) {
            num_files_opened = _profileHandles[i].initHeptagon(_proc, profileObject, &_profilePool, want_noise);
        } else {
            num_files_opened = _profileHandles[i].initHexagon(_proc, profileObject, &_profilePool, _references.ptr(), _references.dim(), want_noise);
        }

        if( !num_files_opened ) {
            _proc.logOut(sQPrideBase::eQPLogType_Info, "Skip profile %s, no relevant subjects\n", profileObject.Id().print());
            continue;
        }

        if (wantPositionLists()) {
            _proc.logOut(sQPrideBase::eQPLogType_Trace, "Updating position lists for profile %s\n", profileObject.Id().print());
            for (sPooledSNPCSVFreqIter preprocess(_profileHandles.ptr(i), _references.ptr(), _references.dim(), &_iterParams); preprocess.valid(); preprocess.nextRecord()) {
                preprocess.updatePosition(_positionLists.ptr());
            }
        }

        _proc.logOut(sQPrideBase::eQPLogType_Trace, "Initializing pooled iterator for profile %s\n", _profileIDs[i].print());
        if (wantPositionLists()) {
            _iters[i] = sPooledSNPCSVFreqIter(_profileHandles.ptr(i), _references.ptr(), _references.dim(), 0, _positionLists.ptr());
        } else {
            _iters[i] = sPooledSNPCSVFreqIter(_profileHandles.ptr(i), _references.ptr(), _references.dim(), &_iterParams);
        }
    }
}

bool DnaClust::DnaClustContext::calcSNPCompare()
{
    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Will calculate SNPcompare\n");
    sFil match;
    if( !reqInitFile(match, "SNPcompare-match.csv") ) {
        return false;
    }
    match.printf("Reference ID,Position\r\n");

    sFil mismatch;
    if( !reqInitFile(match, "SNPcompare-mismatch.csv") ) {
        return false;
    }
    mismatch.printf("Reference ID,Position\r\n");

    for (sPooledSNPCSVFreqIter it = _iters[0]; it.valid(); it.nextRecord()) {
        switch(_snpCompareAlgorithm) {
            case eSNPCompareAlgorithm_CompareConsensus:
                if (it.hasSameConsensusReference())
                    continue;
                else {
                    if ((!it.hasSameConsensus())||(!it.hasSameMaxFreq()))
                        mismatch.printf("%" DEC ",%" UDEC "\r\n", _references[it.segment()], it.getSNPRecord().position);
                    else
                        match.printf("%" DEC ",%" UDEC "\r\n", _references[it.segment()], it.getSNPRecord().position); }
                break;
            case eSNPCompareAlgorithm_FreqCutOff:
                if (it.hasSameConsensusReference())
                    continue;
                else {
                    if ((!it.hasSameMaxFreq() || !it.hasAllMeetFreqThreshold()))
                        mismatch.printf("%" DEC ",%" UDEC "\r\n", _references[it.segment()], it.getSNPRecord().position);
                    else
                        match.printf("%" DEC ",%" UDEC "\r\n", _references[it.segment()], it.getSNPRecord().position);
                }
                break;
            case eSNPCompareAlgorithm_Compare2Consensus:
                if (it.hasSameConsensusReference())
                    continue;
                else {
                    if ((!it.hasSame2Consensus())||(!it.hasSameMaxFreq()))
                        mismatch.printf("%" DEC ",%" UDEC "\r\n", _references[it.segment()], it.getSNPRecord().position);
                    else
                        match.printf("%" DEC ",%" UDEC "\r\n", _references[it.segment()], it.getSNPRecord().position);
                }
                break;
        }
    }
    return true;
}

bool DnaClust::DnaClustContext::calcShrunk()
{
    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Will calculate shrunk genomes\n");
    sVariant queryResult;
    sUsrQueryEngine engine(*_proc.user);

    sStr queryText, subjectTag;
    sFil *shrunkGenomeFile = NULL, *shrunkSNPClassFile = NULL, *shrunkSNPPosFile = NULL;
    if (_shrunkGenome) {
        shrunkGenomeFile = new sFil;
        if( !reqInitFile(*shrunkGenomeFile, "shrunk-genomes.fa") ) {
            return false;
        }
    }
    if (_shrunkSNPClass) {
        shrunkSNPClassFile = new sFil;
        if( !reqInitFile(*shrunkSNPClassFile, "shrunk-snp-classification.csv") ) {
            return false;
        }
    }
    if (_shrunkSNPPos) {
        shrunkSNPPosFile = new sFil;
        if( !reqInitFile(*shrunkSNPPosFile, "shrunk-snp-per-position.csv") ) {
            return false;
        }
    }

    idx shrunkPos = -1, shrunkSize = -1;

    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Finding profile labels for shrunk genomes\n");
    sVec<sVariant> profileLabels;
    profileLabels.resize(_profileIDs.dim());
    for (idx i=0; i<_profileIDs.dim(); i++) {
        if( _profileLabelTemplate ) {
            sVariant profileID;
            profileID.setHiveId(_profileIDs[i]);
            engine.registerBuiltinValue("profile", profileID);
            if( _profileLabelTemplate->run(engine.getContext()) ) {
                profileLabels[i] = engine.getContext().getReturnValue();
            }
        }

        if( !profileLabels[i].asBool() ) {
            profileLabels[i].setHiveId(_profileIDs[i]);
        }
    }
    engine.getContext().removeBuiltin("profile");

    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Finding reference names for shrunk genomes\n");
    sVec<const char *> referenceNames;
    referenceNames.resize(_references.dim());
    for (idx ir=0; ir<_references.dim(); ir++) {
        referenceNames[ir] = _referenceSeq->id(_references[ir]-1);
        if (!referenceNames[ir])
            referenceNames[ir] = sStr::zero;
        else if (referenceNames[ir][0] == '>')
            referenceNames[ir]++;
    }

    const idx nShrunkSnpClassLetters = (_saveShrunkSNPClassColType == eSaveShrunkSNP_position) ? 1 : (_saveShrunkSNPClassColType == eSaveShrunkSNP_atgc) ? 4 : 6;
    idx total_ref_size = 0;
    for( idx i = 0; i < _positionLists.dim(); i++ ) {
        total_ref_size += _positionLists[i].dim();
    }

    if (_shrunkSNPClass) {
        _proc.logOut(sQPrideBase::eQPLogType_Trace, "Starting to print shrunk-snp-classification.csv\n");
        sString::escapeForCSV(*shrunkSNPClassFile, _shrunkSNPClassProfilesTitle);

        sVariant columnLabel;
        sStr referenceName, letterStr;
        for (ShrunkIter it(_iters[0], _positionLists.ptr(), _references.dim(), !_thresholdAny); it.valid(); ++it) {
            for(idx k = 0; k < nShrunkSnpClassLetters; k++) {
                engine.getContext().registerBuiltinIdxConst("isub", _references[it.segment()]);
                referenceName.cutAddString(0, referenceNames[it.segment()]);
                engine.getContext().registerBuiltinStringPtr("id", &referenceName, true);
                engine.getContext().registerBuiltinIdxConst("position", it.segmentPos());
                letterStr.cut0cut();
                if( k < 4 ) {
                    letterStr.addString((const char *)&sBioseq::mapRevATGC[k], 1);
                } else if( k == 4 ) {
                    letterStr.addString("ins");
                } else {
                    letterStr.addString("del");
                }

                columnLabel.setNull();
                if( _saveShrunkSNPClassColLabelTemplate->run(engine.getContext()) ) {
                    columnLabel = engine.getContext().getReturnValue();
                }
                shrunkSNPClassFile->printf(",");
                columnLabel.print(*shrunkSNPClassFile, sVariant::eCSV);
            }
            if( it.numIncrements() % 16384 == 0 ) {
                _proc.reqProgress(-1, it.pos(), total_ref_size * 3);
            }
        }
        shrunkSNPClassFile->printf("\r\n");
    }

    sVec<real> freqsCache;
    for (idx i=0; i<_profileIDs.dim(); i++) {
        _proc.logOut(sQPrideBase::eQPLogType_Trace, "Finding shrunk for profile %s\n", _profileIDs[i].print());
        if (_shrunkGenome)
            shrunkGenomeFile->printf("%s>%s\r\n", i?"\r\n":"", profileLabels[i].asString());

        if (_shrunkSNPClass)
            profileLabels[i].print(*shrunkSNPClassFile, sVariant::eCSV);

        shrunkPos = 0;

        for (ShrunkIter it(_iters[i], _positionLists.ptr(), _references.dim(), !_thresholdAny); it.valid(); ++it, shrunkPos++) {
            if (_shrunkGenome)
                shrunkGenomeFile->printf("%s%c", !shrunkPos || shrunkPos % 80 ? "" : "\r\n", *it);
            if (_shrunkSNPClass || _shrunkSNPPos) {
                for (idx k=0; k<6; k++) {
                    real freq = 100 * it.getDenoisedFreq(k);
                    if( _shrunkSNPClass && k < nShrunkSnpClassLetters ) {
                        if( _saveShrunkSNPClassColType == eSaveShrunkSNP_position ) {
                            if( _saveShrunkSNPClassCellType == eSaveShrunkSNP_frequency ) {
                                real max_freq = 100 * it.getMaxFreq();
                                shrunkSNPClassFile->printf((max_freq < 0.01) ? ",0" : ",%.2lf", max_freq);
                            } else {
                                idx coverage = it.coverage();
                                shrunkSNPClassFile->printf(",%" DEC, coverage);
                            }
                        } else {
                            if( _saveShrunkSNPClassCellType == eSaveShrunkSNP_frequency ) {
                                shrunkSNPClassFile->printf((freq < 0.01) ? ",0" : ",%.2lf", freq);
                            } else {
                                idx atgcCount = it.atgcCount(k);
                                shrunkSNPClassFile->printf(",%" DEC, atgcCount);
                            }
                        }
                    }
                    if( _shrunkSNPPos ) {
                        freqsCache.vadd(1, freq);
                    }
                }
            }
            if( it.numIncrements() % 16384 == 0 ) {
                _proc.reqProgress(-1, total_ref_size + it.pos(), total_ref_size * 3);
            }
        }

        if (_shrunkSNPClass)
            shrunkSNPClassFile->printf("\r\n");

#ifdef _DEBUG
        printf("\tShrunk %s to %" DEC " positions\n", _profileIDs[i].print(), shrunkPos);
#endif
        if (shrunkSize >= 0 && shrunkSize != shrunkPos) {
            _proc.logOut(eQPLogType_Error, "Inconsistent shrunk genome lengths for %" DEC " request; terminating\n", _req);
            _proc.reqSetInfo(_req, eQPInfoLevel_Error, DNA_CLUST_DEFAULT_ERROR);
            return false;
        }

        shrunkSize = shrunkPos;
    }

    if (_shrunkSNPPos) {
        _proc.logOut(sQPrideBase::eQPLogType_Trace, "Starting to print shrunk-snp-per-position.csv\n");
        shrunkSNPPosFile->printf("isub,position,letter");
        for (idx i=0; i<_profileIDs.dim(); i++) {
            shrunkSNPPosFile->printf(",");
            profileLabels[i].print(*shrunkSNPPosFile, sVariant::eCSV);
        }
        shrunkSNPPosFile->printf("\r\n");

        shrunkPos=0;

        for (ShrunkIter it(_iters[0], _positionLists.ptr(), _references.dim(), !_thresholdAny); it.valid(); ++it, shrunkPos++) {
            for (idx k=0; k<6; k++) {
                shrunkSNPPosFile->printf("%" DEC ",%" UDEC ",", _references[it.segment()], it.segmentPos());
                if (k<4)
                    shrunkSNPPosFile->printf("%c", sBioseq::mapRevATGC[k]);
                else if (k == 4)
                    shrunkSNPPosFile->printf("ins");
                else
                    shrunkSNPPosFile->printf("del");

                for (idx ip=0; ip<_profileIDs.dim(); ip++) {
                    real freq = freqsCache[6*(shrunkSize * ip + shrunkPos) + k];
                    shrunkSNPPosFile->printf((freq < 0.01) ? ",0" : ",%.2lf", freq);
                }
                shrunkSNPPosFile->printf("\r\n");
            }
            if( it.numIncrements() % 16384 == 0 ) {
                _proc.reqProgress(-1, total_ref_size * 2 + it.pos(), total_ref_size * 3);
            }
        }
    }

    delete shrunkGenomeFile;
    delete shrunkSNPClassFile;
    delete shrunkSNPPosFile;

    return true;
}

bool DnaClust::DnaClustContext::calcClust()
{
    sStr path;
    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Calculating distance matrix\n");

    idx initial_progress100Start = _proc.progress100Start;
    idx initial_progress100End = _proc.progress100End;

    _proc.progress100End = initial_progress100Start + 0.50 * (initial_progress100End - initial_progress100Start);

    if (!_clust->resetDistance<real, sPooledSNPCSVFreqIter>(_iters.ptr(), _profileIDs.dim(), *_dist, reqProgressStatic, &_proc)) {
        _proc.logOut(eQPLogType_Error, "Failed to calculate distance matrix for %" DEC " request; terminating\n", _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, DNA_CLUST_DEFAULT_ERROR);
        return false;
    }

    sFil matrixFile;
    path.cut(0);
    if( !reqInitFile(matrixFile, "distance-matrix.csv", &path) ) {
        return false;
    }

    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Printing distance matrix\n");
    _clust->printMatrixCSV(matrixFile, nodePrintfCallback, &_profileIDs);

#ifdef _DEBUG
    printf("Matrix: %s\n", path.ptr());
    for (idx i=0; i<_profileIDs.dim(); i++) {
        for (idx j=0; j<_profileIDs.dim(); j++) {
            printf("%g\t", _clust->dist(i,j));
        }
        printf("\n");
    }
#endif

    _proc.progress100Start = _proc.progress100End;
    _proc.progress100End = initial_progress100Start + 0.95 * (initial_progress100End - initial_progress100Start);

    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Calculating clustering\n");
    if (!_clust->recluster(reqProgressStatic, &_proc)) {
        _proc.logOut(eQPLogType_Error, "Failed to calculate clusterization for %" DEC " request; terminating\n", _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, DNA_CLUST_DEFAULT_ERROR);
        return false;
    }

    _proc.progress100Start = _proc.progress100End;
    _proc.progress100End = initial_progress100End;

    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Printing clustering tree\n");
    sFil newickFile;
    path.cut(0);
    if( !reqInitFile(newickFile, "result.tre", &path) ) {
        return false;
    }
    _clust->printNewick(newickFile, sHierarchicalClusteringTree::Newick_PRINT_DISTANCE|sHierarchicalClusteringTree::Newick_PRINT_LEAF_NAMES, nodePrintfCallback, &_profileIDs);

#ifdef _DEBUG
    printf("\n%s: %s\n\n", path.ptr(), newickFile.ptr());
#endif

    return true;
}

idx DnaClust::OnExecute(idx req)
{
#ifdef _DEBUG
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        printf("%s = %s\n", key, value);
    }
#endif

    setupLog(true, sQPrideBase::eQPLogType_Trace);

    DnaClustContext ctx(*this, req);
    if (!ctx.loadParams()) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    if( !ctx.saveExaminedReferences() ) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    reqProgress(-1, 1, 100);
    progress100End = 15;

    ctx.loadProfileIters();

    reqProgress(-1, 100, 100);
    progress100Start = 15;
    progress100End = 100;

    if (ctx.getAlgorithm() == eSNPcompare) {
        ctx.calcSNPCompare();
        sQPrideBase::reqProgress(reqId, 0, -1, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }

    if (ctx.wantShrunk()) {
        progress100End = 33;

        if( !ctx.calcShrunk()) {
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }

        progress100Start = 33;
    }

    if( ctx.getAlgorithm() != eSNPcompare && ctx.getAlgorithm() != eShrunkOnly ) {
        progress100End = 99;

        if( !ctx.calcClust() ) {
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }

        progress100Start = 99;
    }

    progress100Start = 0;
    progress100End = 100;
    sQPrideBase::reqProgress(reqId, 0, -1, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv);

    DnaClust backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-clust",argv[0]));
    return (int)backend.run(argc,argv);
}
