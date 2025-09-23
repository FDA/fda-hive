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
#ifndef sLib_iteriter_h
#define sLib_iteriter_h

#include <slib/core/iter.hpp>
#include <slib/core/heap.hpp>
#include <slib/core/vec.hpp>

namespace slib
{

    template <class Tdata, class Titer>
    class sIterIter : public sIter<Tdata, sIterIter<Tdata, Titer> >
    {
    protected:
        const Titer *_iters;
        idx _niters;
        Titer _iter;
        idx _iiter;
        idx _pos;
        bool _segmented;
        idx _segment;

        idx nextValidIter(idx iiter)
        {
            for (idx i=sMax<idx>(0, iiter); i<_niters; i++) {
                Titer it(_iters[i]);
                if (it.valid())
                    return i;
            }
            return _niters;
        }

        inline bool validIndices() const { return _iiter >= 0 && _iiter < _niters && _iters != NULL; }

    public:
        inline void requestData_impl() { _iter.requestData(); }
        inline void releaseData_impl() { _iter.releaseData(); }
        inline bool readyData_impl() const { return validIndices() && _iter.readyData(); }
        inline bool validData_impl() const {return validIndices() && _iter.validData();}
        inline idx pos_impl() const { return _pos; }
        inline idx segment_impl() const { return _segment; }
        inline idx segmentPos_impl() const { return _iter.segmentPos(); }
        sIterIter<Tdata,Titer>(const Titer *iters = NULL, idx niters = 0, bool segmented=true): _iters(iters), _niters(niters), _segmented(segmented)
        {
            _iiter = nextValidIter(0);
            _pos = _segment = 0;
            if (_niters > 0 && _iiter >= 0 && _iiter < _niters)
                _iter = _iters[_iiter];
        }
        ~sIterIter<Tdata,Titer>() {}

        sIterIter<Tdata,Titer>* clone() const {return new sIterIter<Tdata,Titer>(*this);}
        inline sIterIter<Tdata,Titer>& increment_impl()
        {
            _pos++;
            idx iterSegmentPrev = _iter.segment();
            ++_iter;
            if (_iter.valid()) {
                if (_iter.segment() != iterSegmentPrev)
                    _segment++;
            } else {
                if ((_iiter = nextValidIter(_iiter+1)) < _niters) {
                    _iter.releaseData();
                    _iter = _iters[_iiter];
                    _iter.requestData();
                    if (_segmented || _iter.segment() != iterSegmentPrev)
                        _segment++;
                }
            }
            return *this;
        }

        inline bool equals_impl(const sIterIter<Tdata,Titer> &rhs) const
        {
            return _iters == rhs._iters && _niters == rhs._niters && this->segment() == rhs.segment() && this->segmentPos() == rhs.segmentPos();
        }
        inline bool lessThan_impl(const sIterIter<Tdata,Titer> &rhs) const
        {
            if (_iters != rhs._iters || _niters != rhs._niters)
                return false;
            return this->segment() == rhs.segment() ? this->segmentPos() < rhs.segmentPos() : this->segment() < rhs.segment();
        }
        inline bool greaterThan_impl(const sIterIter<Tdata,Titer> &rhs) const
        {
            if (_iters != rhs._iters || _niters != rhs._niters)
                return false;
            return this->segment() == rhs.segment() ? this->segmentPos() > rhs.segmentPos() : this->segment() > rhs.segment();
        }
        inline Tdata dereference_impl() const
        {
            return *_iter;
        }
    };

    template <class Tdata, class Titer>
    class sIterPair;

    template <class Tdata, class Titer>
    class sIterBundle : public sIter<const Tdata*, sIterBundle<Tdata, Titer> >
    {
    protected:
        template <class Titer_>
        struct SegCoord {
            idx segment;
            idx segmentPos;
            SegCoord(idx s=0, idx p=0) {set(s,p);}
            inline void set(idx s, idx p) {segment = s; segmentPos = p;}
            inline void set(Titer_ &i)
            {
                if (i.valid())
                    set(i.segment(), i.segmentPos());
                else
                    set(sIdxMax, sIdxMax);
            }
            inline bool matches(const Titer_ &i) const {return segment == i.segment() && segmentPos == i.segmentPos();}
            inline bool operator<(const SegCoord &rhs) const { return segment == rhs.segment ? segmentPos < rhs.segmentPos : segment < rhs.segment; }
            inline bool operator==(const SegCoord &rhs) const { return segment == rhs.segment && segmentPos == rhs.segmentPos; }
            inline bool operator<=(const SegCoord &rhs) const { return operator<(rhs) || operator==(rhs); }
        };
        typedef SegCoord<Titer> TSegCoord;

        sVec<Titer> _iters;
        sVec<bool> _is_incremented;
        idx _niters;
        Tdata _default;
        sVec<Tdata> _val;
        idx _nvalid;
        idx _pos;
        sMinHeap<TSegCoord> _segCoordHeap;
        bool _eof;

        inline void fillVal()
        {
            TSegCoord minSegCoord = _segCoordHeap.peekValue();
            _nvalid = 0;
            for (idx i=0; i<_niters; i++) {
                _val[i] = _default;
                if (_iters[i].valid()) {
                    _nvalid++;
                    if (minSegCoord.matches(_iters[i]))
                        _val[i] = *(_iters[i]);
                }
            }
        }

        void incrementMin()
        {
            TSegCoord segCoord, minSegCoord = _segCoordHeap.peekValue();
            memset(_is_incremented.ptr(), 0, _niters * sizeof(bool));
            for (idx ii=0; minSegCoord == _segCoordHeap.peekValue() && ii<_niters; ii++) {
                idx i = _segCoordHeap.peekIndex();
                if (_iters[i].valid()) {
                    if (_is_incremented[i]) {
                        break;
                    }
                    ++_iters[i];
                    _is_incremented[i] = true;
                }

                segCoord.set(_iters[i]);
                _segCoordHeap.adjust(i, segCoord);
            }
        }

        void init(const Titer *iters)
        {
            _iters.resize(_niters);
            _is_incremented.resize(_niters);
            _val.resize(_niters);
            _pos = 0;
            _nvalid = 0;
            sVec<TSegCoord> segCoordVec(sMex::fExactSize);
            segCoordVec.resize(_niters);
            for (idx i=0; i<_niters; i++) {
                _iters[i] = iters[i];
                segCoordVec[i].set(_iters[i]);
            }
            _segCoordHeap.reset(segCoordVec, _niters);
            fillVal();
        }

        friend class sIterPair<Tdata, Titer>;

    public:
        inline bool validData_impl() const { return _niters > 0 && _nvalid > 0; }
        inline idx pos_impl() const { return _pos; }
        inline idx segmentPos_impl() const { return _niters ? _segCoordHeap.peekValue().segmentPos : 0; }
        inline idx segment_impl() const { return _niters ? _segCoordHeap.peekValue().segment : 0; }

        sIterBundle<Tdata,Titer>(const Titer *iters = NULL, idx niters = 0, const Tdata &default_ = 0):
            _iters(sMex::fExactSize), _is_incremented(sMex::fExactSize|sMex::fSetZero), _niters(niters), _default(default_), _val(sMex::fExactSize), _pos(0), _eof(false) { init(iters); }

        sIterBundle<Tdata,Titer>(const sIterBundle<Tdata,Titer> &rhs):
            _iters(rhs._iters), _is_incremented(sMex::fExactSize|sMex::fSetZero), _niters(rhs._niters), _default(rhs._default), _val(sMex::fExactSize)
        {
            init(rhs._iters.ptr());
            _pos = rhs._pos;
        }

        sIterBundle<Tdata,Titer>* clone_impl() const { return new sIterBundle<Tdata,Titer>(*this); }
        inline sIterBundle<Tdata,Titer>& increment_impl()
        {
            incrementMin();
            fillVal();
            _pos++;
            return *this;
        }

        inline bool equals_impl(const sIterBundle<Tdata,Titer> &rhs) const
        {
            return this->segment() == rhs.segment() && this->segmentPos() == rhs.segmentPos();
        }
        inline bool lessThan_impl(const sIterBundle<Tdata,Titer> &rhs) const
        {
            return this->segment() == rhs.segment() ? this->segmentPos() < rhs.segmentPos() : this->segment() < rhs.segment();
        }
        inline bool greaterThan_impl(const sIterBundle<Tdata,Titer> &rhs) const
        {
            return this->segment() == rhs.segment() ? this->segmentPos() > rhs.segmentPos() : this->segment() > rhs.segment();
        }
        inline const Tdata* dereference_impl() const { return _val.ptr(); }
    };

    template <class Tdata, class Titer>
    class sIterPair : public sIter<const Tdata*, sIterPair<Tdata, Titer> >
    {
    protected:
        typedef typename sIterBundle<Tdata, Titer>::TSegCoord TSegCoord;

        Titer _iters[2];
        Tdata _default;
        Tdata _val[2];
        idx _nvalid;
        idx _pos;
        TSegCoord _segCoords[2];

        inline const TSegCoord & getMinSegCoord() const { return _segCoords[0] <= _segCoords[1] ? _segCoords[0] : _segCoords[1]; }

        inline void fillVal()
        {
            const TSegCoord & minSegCoord = getMinSegCoord();
            _nvalid = 0;
            for (idx i=0; i<2; i++) {
                _val[i] = _default;
                if (_iters[i].valid()) {
                    _nvalid++;
                    if (minSegCoord.matches(_iters[i]))
                        _val[i] = *(_iters[i]);
                }
            }
        }

        void incrementMin()
        {
            idx first = 0, second = -1;
            if( _segCoords[1] == _segCoords[0] ) {
                second = 1;
            } else if( _segCoords[1] < _segCoords[0] ) {
                first = 1;
                second = 0;
            }

            if( _iters[first].valid() ) {
                ++_iters[first];
                _segCoords[first].set(_iters[first]);
            }
            if( second >= 0 && _iters[second].valid() ) {
                ++_iters[second];
                _segCoords[second].set(_iters[second]);
            }
        }

        void init(const Titer & x, const Titer & y)
        {
            _iters[0] = x;
            _iters[1] = y;
            _segCoords[0].set(_iters[0]);
            _segCoords[1].set(_iters[1]);
            _pos = 0;
            _nvalid = 0;
            fillVal();
        }

    public:
        inline void requestData_impl()
        {
            _iters[0].requestData();
            _iters[1].requestData();
        }
        inline void releaseData_impl()
        {
            _iters[0].releaseData();
            _iters[1].releaseData();
        }
        inline bool readyData_impl() const
        {
            return _iters[0].readyData() && _iters[1].readyData();
        }
        inline bool validData_impl() const { return _nvalid > 0; }
        inline idx pos_impl() const { return _pos; }
        inline idx segmentPos_impl() const { return getMinSegCoord().segmentPos; }
        inline idx segment_impl() const { return getMinSegCoord().segment; }

        sIterPair<Tdata,Titer>(const Titer & x, const Titer & y, const Tdata & default_ = 0):
            _default(default_)
        {
            init(x, y);
        }

        sIterPair<Tdata,Titer>(const sIterPair<Tdata,Titer> &rhs):
            _default(rhs._default)
        {
            init(rhs._iters[0], rhs._iters[1]);
            _pos = rhs._pos;
        }

        sIterPair<Tdata,Titer>* clone() const { return new sIterPair<Tdata,Titer>(*this); }
        inline sIterPair<Tdata,Titer>& increment_impl()
        {
            incrementMin();
            fillVal();
            _pos++;
            return *this;
        }

        inline bool equals_impl(const sIterPair<Tdata,Titer> &rhs) const { return this->segment() == rhs.segment() && this->segmentPos() == rhs.segmentPos(); }
        inline bool lessThan_impl(const sIterPair<Tdata,Titer> &rhs) const { return this->segment() == rhs.segment() ? this->segmentPos() < rhs.segmentPos() : this->segment() < rhs.segment(); }
        inline bool greaterThan_impl(const sIterPair<Tdata,Titer> &rhs) const { return this->segment() == rhs.segment() ? this->segmentPos() > rhs.segmentPos() : this->segment() > rhs.segment(); }
        inline const Tdata* dereference_impl() const { return _val; }
        inline const Tdata & deref_x() const { return _val[0]; }
        inline const Tdata & deref_y() const { return _val[1]; }
    };
};

#endif
