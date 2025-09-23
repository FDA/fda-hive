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
#ifndef sLib_core_heap_h
#define sLib_core_heap_h

#include <slib/core/iter.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/vec.hpp>
#include <assert.h>

#ifdef _DEBUG_HEAP
#include <iostream>
#endif

namespace slib
{
    enum EHeapFlags {
        eHeapFlags_Pushy = 0,
        eHeapFlags_DEFAULT = 0
    };

    template <class Tobj, class Tcmp, class Titer=sBufferIter<Tobj> >
    class sHeap
    {
    protected:
        static inline idx parent(idx i) {return (i-1)/2;}
        static inline idx child(idx i) {return 2*i + 1;}

        static inline idx smallChild(const Tcmp &cmp, sVec<Tobj> &array, idx i, idx dim)
        {
            idx ichild = sHeap<Tobj,Tcmp,Titer>::child(i);
            if (ichild >= dim)
                return -1;
            return (ichild == dim-1 || cmp(array[ichild], array[ichild+1])) ? ichild : ichild+1;
        }
        static inline void swap(sVec<Tobj> &array, idx i, idx j)
        {
            Tobj temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }

        static idx siftdown(const Tcmp &cmp, sVec<Tobj> &array, idx i, idx dim)
        {
            idx ichild = sHeap<Tobj,Tcmp,Titer>::smallChild(cmp, array, i, dim);
            while (ichild > 0 && ichild <= dim-1 && !cmp(array[i], array[ichild])) {
                swap(array, i, ichild);
                i = ichild;
                ichild = sHeap<Tobj,Tcmp,Titer>::smallChild(cmp, array, i, dim);
            }
            return i;
        }

        static idx siftup(const Tcmp &cmp, sVec<Tobj> &array, idx i, idx dim)
        {
            idx iparent = sHeap<Tobj,Tcmp,Titer>::parent(i);
            while (i > 0 && !cmp(array[iparent], array[i])) {
                swap(array, i, iparent);
                i = iparent;
                iparent = sHeap<Tobj,Tcmp,Titer>::parent(i);
            }
            return i;
        }

    public:

        static void heapify(const Tcmp &cmp, sVec<Tobj> &array, idx dim)
        {
            for (idx i=dim/2 - 1; i >= 0; i--)
                siftdown(cmp, array, i, dim);
        }

        static inline Tobj peek(sVec<Tobj> &array, idx dim) {assert(dim>0); return array[0];}

        static Tobj pop(const Tcmp &cmp, sVec<Tobj> &array, idx dim)
        {
            assert (dim > 0);
            Tobj ret = array[0];
            array[0] = array[dim-1];
            sHeap<Tobj,Tcmp,Titer>::siftdown(cmp, array, 0, dim-1);
            return ret;
        }

        static void push(const Tcmp &cmp, sVec<Tobj> &array, idx dim, Tobj x)
        {
            array.resize(dim+1);
            array[dim] = x;
            sHeap<Tobj,Tcmp,Titer>::siftup(cmp, array, dim, dim);
        }

    protected:
        template <class Tobj_>
        struct sHeapNode
        {
            idx index;
            Tobj_ value;
            sHeapNode<Tobj_>() {}
            sHeapNode<Tobj_>(idx index_, Tobj_ value_): index(index_), value(value_) {}
        };
        typedef sHeapNode<Tobj> Tnode;

        const Tcmp *_cmp;
        sVec<Tnode> _array;

        sVec<idx> _arrayPositionV;
        sDic<idx> _arrayPositionD;

        idx _dim;
        idx _maxdim;
        idx _flags;

        inline idx getArrayPosition(idx index) const
        {
            if (_flags & eHeapFlags_Pushy) {
                const idx* pposition = _arrayPositionD.get(&index, sizeof(index));
                if (pposition)
                    return *pposition;
                return -1;
            }
            return _arrayPositionV[index];
        }
        inline void setArrayPosition(idx index, idx position)
        {
            if (_flags & eHeapFlags_Pushy) {
                *(_arrayPositionD.set(&index, sizeof(index))) = position;
            } else {
                _arrayPositionV[index] = position;
            }
        }
        inline void delArrayPosition(idx index)
        {
            if (_flags & eHeapFlags_Pushy) {
                idx idict = _arrayPositionD.find(&index, sizeof(index));
                if (idict--)
                    _arrayPositionD.undict(idict);
            } else {
                _arrayPositionV[index] = -1;
            }
        }
        void emptyArrayPositions()
        {
            _arrayPositionV.empty();
            _arrayPositionD.empty();
        }
        void resizeArrayPositions(idx dim)
        {
            if (_flags & eHeapFlags_Pushy) {
                _arrayPositionV.resize(0);
            } else {
                _arrayPositionV.resize(dim);
                _arrayPositionD.resize(0);
            } 
        }

        inline virtual bool cmpi(idx i, idx j) const {return (*_cmp)(_array[i].value, _array[j].value);}
        inline virtual bool cmp(idx i, Tobj x) const {return (*_cmp)(_array[i].value, x);}
        inline idx smallChild(idx i) const
        {
            idx ichild = sHeap<Tobj,Tcmp,Titer>::child(i);
            if (ichild >= _dim)
                return -1;
            return (ichild == _dim-1 || cmpi(ichild, ichild+1)) ? ichild : ichild+1;
        }
        inline void swap(idx i, idx j)
        {
            Tnode temp = _array[i];
            _array[i] = _array[j];
            _array[j] = temp;
            setArrayPosition(_array[i].index, i);
            setArrayPosition(_array[j].index, j);
        }
        void assertSanity() const
        {
            assert(_dim >= 0);
            assert(_maxdim >= _dim);
            for (idx i=0; i<_maxdim; i++) {
                idx position = getArrayPosition(i);
                if (position >= 0) {
                    assert(position < _dim);
                    assert(_array[position].index == i);
                }
            }

            for (idx i=0; i<_dim; i++) {
                idx ichild = sHeap<Tobj,Tcmp,Titer>::child(i);
                if (ichild < _dim)
                    assert(cmpi(i, ichild));
                if (ichild+1 < _dim)
                    assert(cmpi(i, ichild+1));
            }
        }

        idx siftdown(idx i, bool force=false)
        {
            idx ichild = smallChild(i);
            while (ichild > 0 && ichild <= _dim-1 && (force || !cmpi(i, ichild))) {
                swap(i, ichild);
                i = ichild;
                ichild = smallChild(i);
            }
            return i;
        }

        idx siftup(idx i, bool force=false)
        {
            idx iparent = sHeap<Tobj,Tcmp,Titer>::parent(i);
            while (i > 0 && (force || !cmpi(iparent, i))) {
                swap(i, iparent);
                i = iparent;
                iparent = sHeap<Tobj,Tcmp,Titer>::parent(i);
            }
            return i;
        }

    public:
        virtual void reset(sVec<Tobj> &vec, idx dim)
        {
            _dim = dim;
            _maxdim = dim;

            _array.resize(_dim);
            emptyArrayPositions();
            resizeArrayPositions(_dim);
            for (idx i=0; i<_dim; i++) {
                _array[i].index = i;
                _array[i].value = vec[i];
                setArrayPosition(i, i);
            }

            for (idx i=dim/2 - 1; i >= 0; i--)
                siftdown(i);
        }

        virtual void reset(const sIter<Tobj, Titer> &iter, idx dim)
        {
            _dim = dim;
            _maxdim = dim;
            sIter<Tobj, Titer> *piter = iter.clone();

            _array.resize(_dim);
            emptyArrayPositions();
            resizeArrayPositions(_dim);
            for (idx i=0; i<_dim; i++, ++(*piter)) {
                _array[i].index = i;
                _array[i].value = *(*piter);
                setArrayPosition(i, i);
            }

            for (idx i=dim/2 - 1; i >= 0; i--)
                siftdown(i);
            delete piter;
        }

        virtual void reset(const sHeap<Tobj, Tcmp, Titer> &rhs)
        {
            _dim = rhs._dim;
            _maxdim = rhs._maxdim;

            _array.resize(_dim);
            _array.copy(&rhs._array);
            emptyArrayPositions();
            resizeArrayPositions(_dim);
            if( _flags == rhs._flags && !(_flags & eHeapFlags_Pushy) ) {
                _arrayPositionV.copy(&rhs._arrayPositionV);
            } else {
                for(idx i = 0; i < _dim; i++) {
                    setArrayPosition(_array[i].index, i);
                }
            }
        }

        virtual inline void reset(const sFixedLengthIter<Tobj, Titer> &iter) { reset(iter, iter.dim()); }

        virtual inline void setCmp(const Tcmp *cmp)
        {
            assert (!_dim);
            _cmp = cmp;
        }

        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp=NULL, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(0), _maxdim(0), _flags(flags) {}

        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp, sVec<Tobj> &vec, idx dim, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(dim), _maxdim(dim), _flags(flags) {reset(vec, dim);}
        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp, const sIter<Tobj, Titer> &iter, idx dim, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(dim), _maxdim(dim), _flags(flags) {reset(iter, dim);}
        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp, const sFixedLengthIter<Tobj, Titer> &iter, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(iter.dim()), _maxdim(iter.dim()), _flags(flags) {reset(iter);}

        virtual ~sHeap<Tobj,Tcmp,Titer>() {}

        virtual inline Tobj peekValue() const {return _array[0].value;}
        virtual inline idx peekIndex() const {return _array[0].index;}

        virtual bool validIndex(idx index) const
        {
            if (index < 0)
                return false;
            idx i = getArrayPosition(index);
            if (i < 0 || i >= _dim)
                return false;
            return true;
        }

        virtual inline Tobj peekValue(idx index) const
        {
            assert(validIndex(index));
            return _array[getArrayPosition(index)].value;
        }

        virtual inline Tobj pop()
        {
            assert(_dim > 0);
            _dim--;
            Tobj ret = _array[0].value;
            delArrayPosition(_array[0].index);
            if (_dim > 0) {
                _array[0] = _array[_dim];
                setArrayPosition(_array[0].index, 0);
                siftdown(0);
            }
            return ret;
        }

        virtual inline void push(Tobj x)
        {
            _dim++;
            _maxdim++;
            _array.resize(_dim);
            resizeArrayPositions(_maxdim);

            _array[_dim-1].value = x;
            _array[_dim-1].index = _maxdim-1;
            setArrayPosition(_maxdim-1, siftup(_dim-1));
        }

        virtual inline void adjust(idx index, Tobj value)
        {
            assert(validIndex(index));
            idx i = getArrayPosition(index);
            if (cmp(i, value)) {
                _array[i].value = value;
                siftdown(i);
            } else {
                _array[i].value = value;
                siftup(i);
            }
        }

        virtual void remove(idx index)
        {
            assert(validIndex(index));
            idx i = getArrayPosition(index);
            siftup(i, true);
            pop();
        }

        virtual void clear()
        {
            _dim = _maxdim = 0;
        }

        virtual inline idx dim() const {return _dim;}

#ifdef _DEBUG_HEAP
        void dump()
        {
            std::cout << "dim: " << _dim << "; maxdim: " << _maxdim << "; pushy mode: " << (_flags & eHeapFlags_Pushy) << std::endl;
            std::cout << "fromIndex: ";
            for (idx i=0; i<_maxdim * 5; i++)
                std::cout << getArrayPosition(i) << " ";
            std::cout << std::endl << "array: ";
            for (idx i=0; i<_dim; i++)
                std::cout << "(value:" << _array[i].value << ", index:" << _array[i].index << ") ";
            std::cout << std::endl;
        }
#endif

    };

    template <class Tobj>
    struct sLessThanOrEqual {
        inline bool operator() (Tobj x, Tobj y) const {return x <= y;}
    };

    template <class Tobj>
    struct sGreaterThanOrEqual {
        inline bool operator() (Tobj x, Tobj y) const {return x >= y;}
    };

    template <class Tobj, class Titer=sBufferIter<Tobj> >
    class sMinHeap: public sHeap<Tobj, sLessThanOrEqual<Tobj>, Titer> {
    protected:
        typedef sLessThanOrEqual<Tobj> Tcmp;
    public:
        sMinHeap<Tobj, Titer>(idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), flags) {}
        sMinHeap<Tobj, Titer>(sVec<Tobj> &vec, idx dim, idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), vec, dim, flags) {}
        sMinHeap<Tobj, Titer>(const sIter<Tobj, Titer> &iter, idx dim, idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), iter, dim, flags) {}
        sMinHeap<Tobj, Titer>(const sFixedLengthIter<Tobj, Titer> &iter, idx dim, idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), iter, flags) {}
        virtual ~sMinHeap<Tobj, Titer>()
        {
            if (this->_cmp) {
                delete this->_cmp;
                this->_cmp = NULL;
            }
        }
    };

    template <class Tobj, class Titer=sBufferIter<Tobj> >
    class sMaxHeap: public sHeap<Tobj, sGreaterThanOrEqual<Tobj>, Titer> {
    protected:
        typedef sGreaterThanOrEqual<Tobj> Tcmp;
    public:
        sMaxHeap<Tobj, Titer>(idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), flags) {}
        sMaxHeap<Tobj, Titer>(sVec<Tobj> &vec, idx dim, idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), vec, dim, flags) {}
        sMaxHeap<Tobj, Titer>(const sIter<Tobj, Titer> &iter, idx dim, idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), iter, dim, flags) {}
        sMaxHeap<Tobj, Titer>(const sFixedLengthIter<Tobj, Titer> &iter, idx dim, idx flags=eHeapFlags_DEFAULT): sHeap<Tobj,Tcmp,Titer>(new Tcmp(), iter, flags) {}
        virtual ~sMaxHeap<Tobj, Titer>()
        {
            if (this->_cmp) {
                delete this->_cmp;
                this->_cmp = NULL;
            }
        }
    };
};

#endif
