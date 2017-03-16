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
    //sStr _buf;
    sVec<uint16_t> _posbuf;
    //sStr _sumbuf;
    idx _delta;
    bool _needSummary;
    real _minFreq;

    struct positionSummary {
        char consensus;
        char consensus0;
        char consensus1;
        char totalFreq;
        positionSummary() { sSet(this, 0); }
    };
    sVec<positionSummary> _summaries;

    enum EPositionBits {
        fFreq0 = 1 << 0, //!< at least one profile has A frequency at this position which is above noise floor
        fFreq1 = 1 << 1, //!< at least one profile has C (T) frequency at this position which is above noise floor
        fFreq2 = 1 << 2, //!< at least one profile has G frequency at this position which is above noise floor
        fFreq3 = 1 << 3, //!< at least one profile has T (C) frequency at this position which is above noise floor
        fFreq4 = 1 << 4, //!< at least one profile has In frequency at this position which is above noise floor
        fFreq5 = 1 << 5, //!< at least one profile has Del frequency at this position which is above noise floor
        fDeltaTotalFreqMeetsThreshold = 1 << 6, //!< at least one profile within delta of the position has a frequency which satisfies the threshold
        fNotAllTotalFreqMeetsThreshold = 1 << 7, //!< not all profiles have SNP-relevantnt ACGTID frequencies which satisfy the threshold
        fUsed = 1 << 8, //!< at least one profile has a record at this position
        fSomeCoverage = 1 << 9, //!< at least one profile has coverage at this position which satisfies threshold
        fNotAllCoverage = 1 << 10, //!< not all profiles have coverage at this position which satisfies threshold
        fNotAllSameConsensus1 = 1 << 11, //!< not all consensus letters are equal (for single-base consensus)
        fNotAllSameConsensus2 = 1 << 12, //!< all consensus letters are equal (for two-base consensus)
        fNotAllSameConsensusReference = 1 << 13, //!< not all consensus letters match reference
        fConsensusSet = 1 << 14, //!< consensus letter is set
        fNotAllSameTotalFreq = 1 << 15 //!< not all profiles have the same total frequency
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

//     FIXME: need to reverse order of loops when setting
//    void emptySummary() { _summaries.empty(); }

    void set(idx recIdx, bool matchTotalFreq, bool hasFreq[6], bool matchCov, sBioseqSNP::SNPRecord &rec, real totalFreq)
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
//printf("set @ %"DEC" : %d %d %c %c : %02x -> ", recIdx, matchFreq, matchCov, letter, consensus, (unsigned int)_buf[recIdx] << 24 >> 24);

        _posbuf[recIdx] |= fUsed;

        for(idx i=0; i<6; i++) {
            if( hasFreq[i] ) {
                _posbuf[i] |= (fFreq0 << i);
            }
        }

        if( matchTotalFreq ) {
            for (idx pos = sMax<idx>(0, recIdx-_delta); pos <= recIdx+_delta; pos++) {
                _posbuf[pos] |= fDeltaTotalFreqMeetsThreshold;
            }
        } else if( _needSummary ) {
            _posbuf[recIdx] |= fNotAllTotalFreqMeetsThreshold;
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

            //for consensus consisting of two most frequent bases
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
                //myconsensus0 = _summaries[recIdx].consensus0;
                //myconsensus1 = _summaries[recIdx].consensus1;

                if( _minFreq > 0 ) { //set freq to zero if it is below minFreq
                    if( totalFreq * 100 < _minFreq )
                        totalFreq = 0;
                    if( _summaries[recIdx].totalFreq < _minFreq )
                        _summaries[recIdx].totalFreq = 0;
                }

                if (fabs(totalFreq * 100 - _summaries[recIdx].totalFreq) > 100) {
                    _posbuf[recIdx] |= fNotAllSameTotalFreq;
                }
            } else {
                _summaries[recIdx].consensus = rec.consensus;
                _summaries[recIdx].consensus0 = (letNum0>=0)?(sBioseq::mapRevATGC[letNum0]):rec.consensus;
                _summaries[recIdx].consensus1 = (letNum1>=0)?(sBioseq::mapRevATGC[letNum1]):rec.consensus;
                _summaries[recIdx].totalFreq = 100 * totalFreq;
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

//printf("%02x (%02x %02x %s)\n", (unsigned int)_buf[recIdx] << 24 >> 24, HEXCHAR(_buf[recIdx] & 1), HEXCHAR(_buf[recIdx] & (1<<1)), meetsThreshold(recIdx) ? "ok" : "no");
    }

    inline idx dim() const { return _posbuf.dim(); }

    inline idx isUsed(idx recIdx) const
    {
        return recIdx >= 0 && recIdx < dim() && (_posbuf[recIdx] & fUsed);
    }

    bool meetsThreshold(idx recIdx, bool allowPartialCoverage) const
    {
        return isUsed(recIdx) &&
               (_posbuf[recIdx] & fDeltaTotalFreqMeetsThreshold) &&
               (allowPartialCoverage ? (_posbuf[recIdx] & fSomeCoverage) : !(_posbuf[recIdx] & fNotAllCoverage));
    }

    bool meetsThresholdACGT(idx recIdx, idx acgtIdx, bool allowPartialCoverage) const
    {
        return isUsed(recIdx) &&
               (_posbuf[recIdx] & fDeltaTotalFreqMeetsThreshold) &&
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

    bool hasSameTotalFreq(idx recIdx) const
    {
        if (recIdx < 0 || recIdx >= dim())
            return false;

        return !(_posbuf[recIdx] & fNotAllSameTotalFreq);
    }

    bool hasAllMeetFreqThreshold(idx recIdx) const
    {
        if (recIdx < 0 || recIdx >= dim())
            return false;

        return !(_posbuf[recIdx] & fNotAllTotalFreqMeetsThreshold);
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
                if( const char * path = profiler_obj.getFilePathname(path_buf, "SNPprofile-%"DEC".csv", references[iref]) ) {
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
                            if( const char * noise_path = profiler_obj.getFilePathname(path_buf, "NoiseIntegral-%"DEC".csv", references[iref]) ) {
                                _hexagon_noise_handles[iref] = _pool->declare(noise_path, sMex::fReadonly);
                                proc.logOut(sQPrideBase::eQPLogType_Debug, "Added %s to pool", noise_path);
                            } else {
                                proc.logOut(sQPrideBase::eQPLogType_Warning, "Not added to pool: profiler %s does not have NoiseIntegral-%"DEC".csv", profiler_obj.Id().print(), references[iref]);
                            }
                        }
                    }
                } else {
                    _hexagon_handles[iref] = _pool->invalidHandle();
                    _hexagon_noise_handles[iref] = _pool->invalidHandle();
                    proc.logOut(sQPrideBase::eQPLogType_Debug, "Not added to pool: profiler %s does not have SNPprofile-%"DEC".csv", profiler_obj.Id().print(), references[iref]);
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

            noiseCutoffs[0][0] = -1; // invalidate the cutoffs

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
                            _proc->logOut(sQPrideBase::eQPLogType_Error, "Failed to find subject %"DEC" in noise file for heptagon %s", isub, _profiler_id.print());
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
                            _proc->logOut(sQPrideBase::eQPLogType_Error, "Failed to parse noise file for subject %"DEC" for profiler %s", isub, _profiler_id.print());
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

        /* rhs may have a different lifetime from us, so we need to request _file on our own */
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
        // if we already are initialized and using a file, make sure to release it now
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
        valid_position_lists[_iref].set(this->_rec.position, meetsThreshold(this->_totalFreq), has_freq, meetsCoverage(), this->_rec, this->_totalFreq);
    }

    inline bool hasSameConsensus() { return _valid_position_lists[_iref].hasSameConsensus(this->_rec.position); }
    inline bool hasSame2Consensus() { return _valid_position_lists[_iref].hasSame2Consensus(this->_rec.position); }
    inline bool hasSameConsensusReference() { return _valid_position_lists[_iref].hasSameConsensusReference(this->_rec.position); }
    inline bool hasSameTotalFreq() { return _valid_position_lists[_iref].hasSameTotalFreq(this->_rec.position); }
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
    ShrunkIter* clone_impl() const { return new ShrunkIter(*this); }
    inline ShrunkIter& increment_impl()
    {
        _pos++;
        _total_pos++;
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
    inline const sBioseqSNP::SNPRecord& getSNPRecord() const
    {
        return _it.getSNPRecord();
    }
};

static const char * algorithmNames =
    "neighbor-joining"_
    "fast-neighbor-joining"_
    "single-link"_
    "complete-link"_
    "SNPcompare"__;

enum eAlgorithms {
    eNeighborJoining = 0,
    eFastNeighborJoining,
    eSingleLink,
    eCompleteLink,
    eSNPcompare
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

        // algorithmic parameters
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

        // data and calculations
        sHiveseq * _referenceSeq;
        sVec<sPooledSNPCSVFreqIter> _iters;
        sFilPool _profilePool;
        sVec<SNPprofileHandleSet> _profileHandles;
        sVec<validPositionList> _positionLists;
        sHierarchicalClustering * _clust;

        // loadParams helpers
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
        }

        ~DnaClustContext()
        {
            delete _referenceSeq;
            delete _dist;
            delete _clust;
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

// Print profiling job IDs as names in Newick/CSV output
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
    if (_proc.formHiveIdValues("profileID", &_profileIDs) < 0) {
        _proc.logOut(eQPLogType_Error, "Empty profileID list for %"DEC" request; terminating\n", _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Need a non-empty list of profiling results");
        return false;
    }
    return true;
}

bool DnaClust::DnaClustContext::loadGenomeIDs()
{
    if (_proc.formHiveIdValues("referenceID", &_genomeIDs) < 0) {
        _proc.logOut(eQPLogType_Error, "Empty referenceID genome list for %"DEC" request; terminating\n", _req);
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
    //_referenceSeq->reindex();
    if (_referenceSeq->dim() < 1) {
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "No data found for reference genome IDs %s%s%s", genomeIDsStr.ptr(), errStr.length() ? ": " : "", errStr.length() ? errStr.ptr() : "");
        return false;
    }
    errStr.cut0cut();

    bool incompleteOK = false;
    const char * tmpstr = _proc.formValue("incompleteReferences");
    if (tmpstr && !strcmp(tmpstr, "zero"))
        incompleteOK = true;

    // map from reference id to number of profiles using it (if >= 0); or -1 to ignore
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

    // Count in how many profiles is each reference present
    sVec<bool> incrementRefCounts(sMex::fExactSize);
    sStr profileRefsBuf;
    for (idx i=0; i<_profileIDs.dim(); i++) {
        sUsrFile profileObject(_profileIDs[i], _proc.user);
        // handle both modern multivalue subSet and legacy ";"-delimeted single-value subSet
        profileRefsBuf.cut(0);
        if( const char * profileRefs = profileObject.propGet00("subSet", &profileRefsBuf, ";") ) {
            for(idx ic=0; ic<profileRefsBuf.length(); ic++) {
                if( profileRefsBuf[ic] == ';' )
                    profileRefsBuf[ic] = 0;
            }
            // Profile object has explicit list of references; take care to increment each count only once
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
            // profile object used all references
            for(idx ref=1; ref<allRefCounts.dim(); ref++) {
                if( allRefCounts[ref] >= 0 ) {
                    allRefCounts[ref]++;
                }
            }
        }
    }

    /* If incompleteReferences is "zero", we use all requested subjects present in
     * at least one profile (missing ones will be automatically treated as zero).
     * If incompleteReferences is "skip", we use only those requested subects
     * present in all profiles */
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
        _clust = 0;
        break;
    default:
        _clust = new sNeighborJoining;
    }

    sStr distanceName;
    _proc.formValue("distance", &distanceName);

    if (!strcmp(distanceName.ptr(), "manhattan"))
        _dist = new sManhattanDist<real,sPooledSNPCSVFreqIter>();
    else if (!strcmp(distanceName.ptr(), "maximum"))
        _dist = new sMaximumDist<real,sPooledSNPCSVFreqIter>();
    else if (!strcmp(distanceName.ptr(), "pnorm")) {
        real p = _proc.formRValue("pValue");
        if (p < 1.0) {
            _proc.logOut(eQPLogType_Error, "Invalid pValue == %g, expected pValue >= 1.0 for %"DEC" request; terminating\n", p, _req);
            _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Expected parameter p >= 1.0, but %g was given", p);
            return false;
        }
        _dist = new sPNormDist<real,sPooledSNPCSVFreqIter>(p);
    } else if (!strcmp(distanceName.ptr(), "canberra"))
        _dist = new sCanberraDist<real,sPooledSNPCSVFreqIter>();
    else if (!strcmp(distanceName.ptr(), "pearson")) {
        bool uncentered = _proc.formIValue("pearsonUncentered");
        bool squared = _proc.formIValue("pearsonSquared");
        _dist = new sPearsonDist<real,sPooledSNPCSVFreqIter>(uncentered, squared);
    } else if (!strcmp(distanceName.ptr(), "cosine"))
        _dist = new sCosineDist<real,sPooledSNPCSVFreqIter>();
    else // "euclidean" is the default
        _dist = new sEuclideanDist<real,sPooledSNPCSVFreqIter>();

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
        _proc.logOut(eQPLogType_Error, "Invalid thresholdWindow = %"DEC", expected >= 0 for %"DEC" request; terminating\n", _thresholdWindow, _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Expected non-negative position range parameter, but %"DEC" was given", _thresholdWindow);
        return false;
    } else if (_thresholdWindow > 0 && !_thresholdAny) {
        _proc.logOut(eQPLogType_Error, "If thresholdWindow > 0, expect thresholdAny to be true for %"DEC" request; terminating\n", _req);
        _proc.reqSetInfo(_req, eQPInfoLevel_Error, "Expected 'all profiles as long as one meets constraint' parameter to be enabled if a non-empty position range parameter was given");
        return false;
    }

#ifdef _DEBUG
    printf("SNP freq threshold: [%.0f%%, %.0f%%]; coverage: >= %"DEC"; thresholdAny: %s; thresholdWindow: %"DEC"\n", _iterParams.getMinThreshold()*100, _iterParams.getMaxThreshold()*100, _iterParams.getMinCoverage(), _thresholdAny?"true":"false", _thresholdWindow);
#endif

    return true;
}

bool DnaClust::DnaClustContext::loadShrunk()
{
    _shrunkGenome = _proc.formBoolValue("saveShrunk");
    _shrunkSNPClass = _proc.formBoolValue("saveShrunkSNPClass");
    _shrunkSNPPos = _proc.formBoolValue("saveShrunkSNPPos");
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
            // Do not attempt to filter this profiler object's noise if it was already filtered during profiling
            if (!profileNoiseFilterParams || !strcmp(profileNoiseFilterParams, "none")) {
                want_noise = true;
            }
        }

        if( profileObject.getFilePathname(profile_path, "SNPprofile.csv") ) {
            // this is a heptagon profile with one snp file for all references
            num_files_opened = _profileHandles[i].initHeptagon(_proc, profileObject, &_profilePool, want_noise);
        } else {
            num_files_opened = _profileHandles[i].initHexagon(_proc, profileObject, &_profilePool, _references.ptr(), _references.dim(), want_noise);
        }

        if( !num_files_opened ) {
            // nothing to do...
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
                else { //if ((!it.hasSameConsensus())||(!it.hasSameTotalFreq()) || !it.hasAllMeetFreqThreshold())
                    if ((!it.hasSameConsensus())||(!it.hasSameTotalFreq()))
                        mismatch.printf("%"DEC",%"UDEC"\r\n", _references[it.segment()], it.getSNPRecord().position);
                    else
                        match.printf("%"DEC",%"UDEC"\r\n", _references[it.segment()], it.getSNPRecord().position); }
                break;
            case eSNPCompareAlgorithm_FreqCutOff:
                if (it.hasSameConsensusReference())
                    continue;
                //nothing.printf("%"DEC",%u\r\n", subjectList[ir], it.getSNPRecord().position);
                else {
                    if ((!it.hasSameTotalFreq() || !it.hasAllMeetFreqThreshold()))
                        mismatch.printf("%"DEC",%"UDEC"\r\n", _references[it.segment()], it.getSNPRecord().position);
                    else
                        match.printf("%"DEC",%"UDEC"\r\n", _references[it.segment()], it.getSNPRecord().position);
                }
                break;
            case eSNPCompareAlgorithm_Compare2Consensus:
                //if (it.hasSameConsensusReference() || !it.hasAllMeetFreqThreshold())
                if (it.hasSameConsensusReference())
                    continue;
                //else { if ((!it.hasSame2Consensus())||(!it.hasSameTotalFreq()) || !it.hasAllMeetFreqThreshold())
                else {
                    if ((!it.hasSame2Consensus())||(!it.hasSameTotalFreq()))
                        mismatch.printf("%"DEC",%"UDEC"\r\n", _references[it.segment()], it.getSNPRecord().position);
                    else
                        match.printf("%"DEC",%"UDEC"\r\n", _references[it.segment()], it.getSNPRecord().position);
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
        if( !reqInitFile(*shrunkSNPClassFile, "shrunk-snp-per-position.csv") ) {
            return false;
        }
    }

    idx shrunkPos = -1, shrunkSize = -1;

    _proc.logOut(sQPrideBase::eQPLogType_Trace, "Finding profile labels for shrunk genomes\n");
    sVec<sVariant> profileLabels;
    profileLabels.resize(_profileIDs.dim());
    for (idx i=0; i<_profileIDs.dim(); i++) {
        queryText.cut(0);
        queryText.printf("(\"%s\" as obj).name", _profileIDs[i].print());

        if (engine.parse(queryText) && engine.eval(queryResult)) {
            profileLabels[i].setSprintf("%s: %s", _profileIDs[i].print(), queryResult.asString());
        } else {
            profileLabels[i].setHiveId(_profileIDs[i]);
        }
    }

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

    if (_shrunkSNPClass) {
        // Print the header
        _proc.logOut(sQPrideBase::eQPLogType_Trace, "Starting to print shrunk-snp-classification.csv\n");
        shrunkSNPClassFile->printf("profile");
        for (ShrunkIter it(_iters[0], _positionLists.ptr(), _references.dim(), !_thresholdAny); it.valid(); ++it) {
            for (idx k=0; k<6; k++) {
                sVariant columnLabel;
                columnLabel.setSprintf("isub=%"DEC" pos=%"DEC" ", _references[it.segment()], it.getSNPRecord().position);
                if (k<4)
                    columnLabel.append("%c", sBioseq::mapRevATGC[k]);
                else if (k == 4)
                    columnLabel.append("ins");
                else
                    columnLabel.append("del");

                shrunkSNPClassFile->printf(",");
                columnLabel.print(*shrunkSNPClassFile, sVariant::eCSV);
            }
        }
        shrunkSNPClassFile->printf("\r\n");
    }

    sVec<real> freqsCache;
    for (idx i=0; i<_profileIDs.dim(); i++) {
        _proc.logOut(sQPrideBase::eQPLogType_Trace, "Finding shrunk for profile %s\n", _profileIDs[i].print());
        // shrunk genome subsequence name
        if (_shrunkGenome)
            shrunkGenomeFile->printf("%s>%s\r\n", i?"\r\n":"", profileLabels[i].asString());

        // row label for classification table
        if (_shrunkSNPClass)
            profileLabels[i].print(*shrunkSNPClassFile, sVariant::eCSV);

        shrunkPos = 0;

        for (ShrunkIter it(_iters[i], _positionLists.ptr(), _references.dim(), !_thresholdAny); it.valid(); ++it, shrunkPos++) {
            if (_shrunkGenome)
                shrunkGenomeFile->printf("%s%c", !shrunkPos || shrunkPos % 80 ? "" : "\r\n", *it);
            if (_shrunkSNPClass || _shrunkSNPPos) {
                for (idx k=0; k<6; k++) {
                    real freq = 100 * it.getDenoisedFreq(k);
                    if (_shrunkSNPClass)
                        shrunkSNPClassFile->printf((freq < 0.01) ? ",0" : ",%.2lf", freq);
                    if (_shrunkSNPPos)
                        freqsCache.vadd(1, freq);
                }
            }
        }

        if (_shrunkSNPClass)
            shrunkSNPClassFile->printf("\r\n");

#ifdef _DEBUG
        printf("\tShrunk %s to %"DEC" positions\n", _profileIDs[i].print(), shrunkPos);
#endif
        // Sanity check
        if (shrunkSize >= 0 && shrunkSize != shrunkPos) {
            _proc.logOut(eQPLogType_Error, "Inconsistent shrunk genome lengths for %"DEC" request; terminating\n", _req);
            _proc.reqSetInfo(_req, eQPInfoLevel_Error, DNA_CLUST_DEFAULT_ERROR);
            return false;
        }

        shrunkSize = shrunkPos;
    }

    if (_shrunkSNPPos) {
        _proc.logOut(sQPrideBase::eQPLogType_Trace, "Starting to print shrunk-snp-per-position.csv\n");
        // Print the header
        shrunkSNPPosFile->printf("isub,position,letter");
        for (idx i=0; i<_profileIDs.dim(); i++) {
            shrunkSNPPosFile->printf(",");
            profileLabels[i].print(*shrunkSNPPosFile, sVariant::eCSV);
        }
        shrunkSNPPosFile->printf("\r\n");

        shrunkPos=0;

        for (ShrunkIter it(_iters[0], _positionLists.ptr(), _references.dim(), !_thresholdAny); it.valid(); ++it, shrunkPos++) {
            for (idx k=0; k<6; k++) {
                // Print the row label
                shrunkSNPPosFile->printf("%"DEC",%"UDEC",", _references[it.segment()], it.getSNPRecord().position);
                if (k<4)
                    shrunkSNPPosFile->printf("%c", sBioseq::mapRevATGC[k]);
                else if (k == 4)
                    shrunkSNPPosFile->printf("ins");
                else
                    shrunkSNPPosFile->printf("del");

                // Print transposed shrunk frequency table row
                for (idx ip=0; ip<_profileIDs.dim(); ip++) {
                    real freq = freqsCache[6*(shrunkSize * ip + shrunkPos) + k];
                    shrunkSNPPosFile->printf((freq < 0.01) ? ",0" : ",%.2lf", freq);
                }
                shrunkSNPPosFile->printf("\r\n");
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
        _proc.logOut(eQPLogType_Error, "Failed to calculate distance matrix for %"DEC" request; terminating\n", _req);
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
        _proc.logOut(eQPLogType_Error, "Failed to calculate clusterization for %"DEC" request; terminating\n", _req);
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

    // FIXME : for debugging in release mode
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
        sQPrideBase::reqProgress(reqId, 0, -1, 100, 100); // force progress to 100% when done
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

    progress100End = 99;
    if (!ctx.calcClust()) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    progress100Start = 0;
    progress100End = 100;
    sQPrideBase::reqProgress(reqId, 0, -1, 100, 100); // force progress to 100% when done
    reqSetStatus(req, eQPReqStatus_Done); // change the status
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaClust backend("config=qapp.cfg"__,sQPrideProc::QPrideSrvName(&tmp,"dna-clust",argv[0]));
    return (int)backend.run(argc,argv);
}
