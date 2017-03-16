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
    /*! \brief Bitflags for initializing sHeap and its subclasses */
    enum EHeapFlags {
// FIXME: for pushy mode, need a new data structure, one whose size is O(number of pushes). A balanced tree maybe?
        eHeapFlags_Pushy = 0, //1, //!< Total number of push() operations may far exceed max size of heap
        eHeapFlags_DEFAULT = 0 //!< Default bitwise conjunction of #EHeapFlags for initializing sHeap
    };

    /*! \brief Basic binary heap; can be used as a container, or an aribtrary sArr can be heapified using static methods.
     * \tparam Tobj Type of data stored
     * \tparam Tcmp binary comparison function object for Tobj which defines the heap property, e.g. \code
struct compare { bool operator() {idx x, idx y} const { return x <= y;} };
    \endcode
     *
     * Runtime peformance is O(dim) to initialize or reset, O(1) to peek, O(log dim) to pop, push, or modify.
     * Memory usage for the container is O(max-heap-size + total-number-of-push()-calls) by default.
     * If the heap was initialized with #eHeapFlags_Pushy in flags, memory usage is O(max-heap-size), but
     * each operation will be slower.
     */
    template <class Tobj, class Tcmp, class Titer=sBufferIter<Tobj> >
    class sHeap
    {
    protected:
        static inline idx parent(idx i) {return (i-1)/2;} //!< index of parent of i
        static inline idx child(idx i) {return 2*i + 1;} //!< index of first child of i

        //! \brief index of the smallest (for min-heap) of i's two children
        static inline idx smallChild(const Tcmp &cmp, sVec<Tobj> &array, idx i, idx dim)
        {
            idx ichild = sHeap<Tobj,Tcmp,Titer>::child(i);
            if (ichild >= dim)
                return -1;
            return (ichild == dim-1 || cmp(array[ichild], array[ichild+1])) ? ichild : ichild+1;
        }
        //! \brief swap array[i] and array[j]
        static inline void swap(sVec<Tobj> &array, idx i, idx j)
        {
            Tobj temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }

        //! \brief O(log dim - log i)
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

        //! \brief O(log i)
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
        // Static methods for using any sArr as a heap

        /*! \brief Turn array into a heap; O(dim)
         * \param cmp Binary comparison function object
         * \param array Array to heapify
         * \param dim Size of array
         */
        static void heapify(const Tcmp &cmp, sVec<Tobj> &array, idx dim)
        {
            for (idx i=dim/2 - 1; i >= 0; i--)
                siftdown(cmp, array, i, dim);
        }

        /*! \brief Peek at top (i.e. smallest in case of minheap) element of a heap; O(1)
         * \param array Heapified array
         * \param dim Size of array
         */
        static inline Tobj peek(sVec<Tobj> &array, idx dim) {assert(dim>0); return array[0];}

        /*! \brief Pop the top (i.e. smallest in case of minheap) element from a heap; O(log dim)
         * \param cmp Binary comparison function object
         * \param array Heapified array
         * \param dim Size of array
         * \warning The array will not be resized; the last element should simply be ignored. The caller must manually decrement dim after using this function.
         */
        static Tobj pop(const Tcmp &cmp, sVec<Tobj> &array, idx dim)
        {
            assert (dim > 0);
            Tobj ret = array[0];
            array[0] = array[dim-1];
            sHeap<Tobj,Tcmp,Titer>::siftdown(cmp, array, 0, dim-1);
            return ret;
        }

        /*! \brief Add an element to a heap; O(log dim)
         * \param cmp Binary comparison function object
         * \param array Heapified array
         * \param dim Size of array
         * \param x Value to add to heap
         * \warning The caller must manually increment dim after using this function.
         */
        static void push(const Tcmp &cmp, sVec<Tobj> &array, idx dim, Tobj x)
        {
            array.resize(dim+1);
            array[dim] = x;
            sHeap<Tobj,Tcmp,Titer>::siftup(cmp, array, dim, dim);
        }

        // Container methods
    protected:
        //! We wrap values in this struct to allow retrieving individual values by index, i.e. by the order in which they were first added to the heap
        //! \tparam Tobj_ sHeap's Tobj
        template <class Tobj_>
        struct sHeapNode
        {
            idx index;
            Tobj_ value;
            sHeapNode<Tobj_>() {}
            sHeapNode<Tobj_>(idx index_, Tobj_ value_): index(index_), value(value_) {}
        };
        typedef sHeapNode<Tobj> Tnode;

        const Tcmp *_cmp; //!< comparison function object
        sVec<Tnode> _array; //!< the actual heap

        sVec<idx> _arrayPositionV; //!< maps from node.index to node's position in _array when !(_flags & eHeapFlags_Pushy)
        sDic<idx> _arrayPositionD; //!< maps from node.index to node's position in _array when _flags & eHeapFlags_Pushy

        idx _dim; //!< current number of elements in _array
        idx _maxdim; //!< maximum number of elements ever present in _array
        idx _flags;

        //! wrap _arrayPositionV / _arrayPositionI
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

        //! \brief compare _array[i].value and _array[j].value
        inline virtual bool cmpi(idx i, idx j) const {return (*_cmp)(_array[i].value, _array[j].value);}
        //! \brief compare _array[i].value and x
        inline virtual bool cmp(idx i, Tobj x) const {return (*_cmp)(_array[i].value, x);}
        //! \brief position in array of the smaller of _array[i]'s two children
        inline idx smallChild(idx i) const
        {
            idx ichild = sHeap<Tobj,Tcmp,Titer>::child(i);
            if (ichild >= _dim)
                return -1;
            return (ichild == _dim-1 || cmpi(ichild, ichild+1)) ? ichild : ichild+1;
        }
        //! \brief swap _array[i] and _array[j], and update _arrayPosition
        inline void swap(idx i, idx j)
        {
            Tnode temp = _array[i];
            _array[i] = _array[j];
            _array[j] = temp;
            setArrayPosition(_array[i].index, i);
            setArrayPosition(_array[j].index, j);
        }
        //! \brief assert that the data is consistent and the heap property holds
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

        //! \brief O(log _dim - log i); if force is true, will always sift i to bottom of heap, ignoring its value
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

        //! \brief O(log i); if force is true, will always sift i to top of heap, ignoring its value
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
        /*! \brief Clear the heap and reload with new data; O(dim)
         * \param vec New data
         * \param dim Size of vec */
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

        /*!  \brief Clear the heap and reload with new data; O(dim)
         * \param iter New data
         * \param dim Steps in iter */
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

        /*! \brief Clear the heap and reload with new data; O(iter.dim)
         * \param iter New data */
        virtual inline void reset(const sFixedLengthIter<Tobj, Titer> &iter) { reset(iter, iter.dim()); }

        /*! \brief Set the heap's binary comparison object
         * \warning Asserts if the heap has elements
         * \param cmp Pointer to a binary comparison object */
        virtual inline void setCmp(const Tcmp *cmp)
        {
            assert (!_dim);
            _cmp = cmp;
        }

        /*! \brief Initialize an empty heap; O(1)
         * \param cmp Binary comparison function object
         * \param flags Bitwise disjunction of #EHeapFlags; make sure to set #eHeapFlags_Pushy if the total number of push() calls
         *              might significantly exceed the maximum number of heaped objects. */
        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp=NULL, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(0), _maxdim(0), _flags(flags) {}

        /*! \brief Initialize the heap with given data; O(dim)
         * \param cmp Binary comparison function object
         * \param vec Data
         * \param dim Size of vec
         * \param flags Bitwise disjunction of #EHeapFlags; make sure to set #eHeapFlags_Pushy if the total number of push() calls
         *              might significantly exceed the maximum number of heaped objects. */
        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp, sVec<Tobj> &vec, idx dim, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(dim), _maxdim(dim), _flags(flags) {reset(vec, dim);}
        /*! \brief Initialize the heap with given data; O(dim)
         * \param cmp Binary comparison function object
         * \param iter Data
         * \param dim Steps in iter
         * \param flags Bitwise disjunction of #EHeapFlags; make sure to set #eHeapFlags_Pushy if the total number of push() calls
         *              might significantly exceed the maximum number of heaped objects. */
        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp, const sIter<Tobj, Titer> &iter, idx dim, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(dim), _maxdim(dim), _flags(flags) {reset(iter, dim);}
        /*! \brief Initialize the heap with given data; O(iter.dim)
         * \param cmp Binary comparison function object
         * \param iter Data
         * \param flags Bitwise disjunction of #EHeapFlags; make sure to set #eHeapFlags_Pushy if the total number of push() calls
         *              might significantly exceed the maximum number of heaped objects. */
        sHeap<Tobj,Tcmp,Titer>(const Tcmp *cmp, const sFixedLengthIter<Tobj, Titer> &iter, idx flags=eHeapFlags_DEFAULT): _cmp(cmp), _dim(iter.dim()), _maxdim(iter.dim()), _flags(flags) {reset(iter);}

        virtual ~sHeap<Tobj,Tcmp,Titer>() {}

        /*! \brief Peek at the top of the heap; O(1)
         * \returns Value on top of the heap (i.e. the smallest in case of minheap) */
        virtual inline Tobj peekValue() const {return _array[0].value;}
        /*! \brief Peek at the top of the heap; O(1)
         * \returns Order index in which the value on top of the heap (i.e. the smallest in case of minheap) was originally added to the heap */
        virtual inline idx peekIndex() const {return _array[0].index;}

        /*! \brief Check if an element is in the heap; O(1)
         * \returns true if the index'th element to be added to the heap is still in the heap */
        virtual bool validIndex(idx index) const
        {
            if (index < 0)
                return false;
            idx i = getArrayPosition(index);
            if (i < 0 || i >= _dim)
                return false;
            return true;
        }

        /*! \brief Peek by order index; O(1)
         * \param index Order index
         * \returns Value which was the index'th to be added to the heap
         * \warning Asserts if the index'th value to be added to the heap is no longer in the heap */
        virtual inline Tobj peekValue(idx index) const
        {
            assert(validIndex(index));
            return _array[getArrayPosition(index)].value;
        }

        /*! \brief Pop the top (smallest in case of min heap) value from the heap; O(log dim())
         * \returns Value at the top of the heap
         * \warning Asserts if the heap is empty */
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
//            assertSanity();
            return ret;
        }

        /*! \brief Add a value to the heap; O(log dim())
         * \warning If the heap was initialized without #eHeapFlags_Pushy in \a flags,
         *          the heap's memory usage will be O(total number of push() calls)
         * \param x Value to be added */
        virtual inline void push(Tobj x)
        {
            _dim++;
            _maxdim++;
            _array.resize(_dim);
            resizeArrayPositions(_maxdim);

            _array[_dim-1].value = x;
            _array[_dim-1].index = _maxdim-1;
            setArrayPosition(_maxdim-1, siftup(_dim-1));
//            assertSanity();
        }

        /*! \brief Modify a value in the heap; O(log dim())
         * \param index Order index in which the value had been added to the heap
         * \param value The new value for the index'th element to be added to the heap
         * \warning Asserts if the index'th value to be added to the heap is no longer in the heap */
        virtual inline void adjust(idx index, Tobj value)
        {
            assert(validIndex(index));
            idx i = getArrayPosition(index);
            if (cmp(i, value)) {
                _array[i].value = value;
                siftdown(i);
//                assertSanity();
            } else {
                _array[i].value = value;
                siftup(i);
//                assertSanity();
            }
        }

        /*! \brief Remove a value from the heap; O(log dim())
         * \param index Order index in which the value had been added to the heap
         * \warning Asserts if the index'th value to be added to the heap is no longer in the heap */
        virtual void remove(idx index)
        {
            assert(validIndex(index));
            idx i = getArrayPosition(index);
            siftup(i, true);
            pop();
        }

        /*! \brief Clear the heap of all data; O(1) */
        virtual void clear()
        {
            _dim = _maxdim = 0;
        }

        /*! \returns Number of elements in the heap; O(1) */
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

    /*! \brief <= binary comparison object
     * \tparam Tobj Type to compare; must support the <= operator */
    template <class Tobj>
    struct sLessThanOrEqual {
        inline bool operator() (Tobj x, Tobj y) const {return x <= y;}
    };

    /*! \brief >= binary comparison object
     * \tparam Tobj Type to compare; must support the >= operator */
    template <class Tobj>
    struct sGreaterThanOrEqual {
        inline bool operator() (Tobj x, Tobj y) const {return x >= y;}
    };

    /*! \brief Basic binary min heap
     * \tparam Tobj Type to be heapified; must support the <= operator */
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

    /*! \brief Basic binary max heap
     * \tparam Tobj Type to be heapified; must support the >= operator */
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
