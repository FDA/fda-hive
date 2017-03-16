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
#ifndef sLib_iter_h
#define sLib_iter_h

#include <iterator>

#include <slib/core/def.hpp>
#include <slib/core/vec.hpp>

namespace slib
{

    /*! \brief Base class for a read-only iterator
     * \tparam Titer iterator derived class providing *_impl() function for each part of the sIter interface
     * \tparam Tdata type iterated over; assumed to be a built-in type or a small struct that is cheap to allocate
     */
    template <class Tdata, class Titer>
    class sIter : public std::iterator<std::forward_iterator_tag, Tdata, idx>
    {
    public:
        //! For iterators over lazy data sets, requests that the data be available
        void requestData()
        {
            static_cast<Titer*>(this)->requestData_impl();
        }
        //! For iterators over lazy data sets, allows the data to be released
        void releaseData()
        {
            static_cast<Titer*>(this)->releaseData_impl();
        }
        //! For iterators over lazy data sets, checks that the data is ready to be iterated over
        bool readyData() const
        {
            return static_cast<const Titer*>(this)->readyData_impl();
        }
        //! For iterators over lazy data sets, assuming that the data is ready, verify that the iterator is pointing to somewhere valid in it
        bool validData() const
        {
            return static_cast<const Titer*>(this)->validData_impl();
        }
        /* !\returns true if the iterator is pointing to something valid
         * \note Not \a const because for iterators over lazy data sets, checking validity
         *       could involve initializing the data set itself via requestData() */
        bool valid()
        {
            if (!readyData())
                requestData();

            return validData();
        }
        //! number of times the iterator was incremented
        idx pos() const
        {
            return static_cast<const Titer*>(this)->pos_impl();
        }
        //! iterator's underlying segment in a segmented data set; on ++ should either increase or stay constant
        idx segment() const
        {
            return static_cast<const Titer*>(this)->segment_impl();
        }
        //! iterator's underlying position in a sparse data segment; within a single segment, segmentPos() must increase on ++
        idx segmentPos() const
        {
            return static_cast<const Titer*>(this)->segmentPos_impl();
        }
        sIter<Tdata, Titer> * clone() const
        {
            return static_cast<const Titer*>(this)->clone_impl();
        }
        sIter<Tdata, Titer> & operator++()
        {
            return static_cast<Titer*>(this)->increment_impl();
        }
        bool operator==(const sIter<Tdata, Titer> & rhs) const
        {
            return static_cast<const Titer*>(this)->equals_impl(rhs);
        }
        bool operator<(const sIter<Tdata, Titer> & rhs) const
        {
            return static_cast<const Titer*>(this)->lessThan_impl(rhs);
        }
        bool operator>(const sIter<Tdata, Titer> & rhs) const
        {
            return static_cast<const Titer*>(this)->greaterThan_impl(rhs);
        }
        Tdata operator*() const
        {
            return static_cast<const Titer*>(this)->dereference_impl();
        }
    };

    /*! \brief Mixin for a read-only iterator over a fixed number of values
     * \tparam Tdata type iterated over; assumed to be a built-in type or a small struct that is cheap to allocate */
    template <class Tdata, class Titer>
    class sFixedLengthIter
    {
    public:
        //!\returns Number of elements from the initial position and up
        idx dim() const
        {
            return static_cast<const Titer*>(this)->dim_impl();
        }
        sFixedLengthIter<Tdata, Titer>* clone() const
        {
            return static_cast<const Titer*>(this)->clone_impl();
        }
        idx operator-(const sFixedLengthIter<Tdata, Titer> &rhs) const
        {
            return static_cast<const Titer*>(this)->difference_impl();
        }
        sFixedLengthIter<Tdata, Titer> & operator+=(idx i)
        {
            return static_cast<Titer*>(this)->incrementBy_impl(i);
        }
        Tdata operator[](idx i) const
        {
            return static_cast<const Titer*>(this)->dereferenceAt_impl(i);
        }
        operator sIter<Tdata, Titer> & ()
        {
            return static_cast<sIter<Tdata, Titer> &>(*this);
        }
        operator const sIter<Tdata, Titer> & () const
        {
            return static_cast<const sIter<Tdata, Titer> &>(*this);
        }
    };

    /*! \brief Mixin for a read-write iterator
     * \tparam Tdata type iterated over; assumed to be a built-in type or a small struct that is cheap to allocate */
    template <class Tdata, class Titer>
    class sMutableIter
    {
    public:
        sMutableIter<Tdata, Titer> * clone() const
        {
            return static_cast<const Titer*>(this)->clone_impl();
        }
        Tdata & operator*()
        {
            return static_cast<Titer*>(this)->dereferenceMutable_impl();
        }
        operator sIter<Tdata, Titer> & ()
        {
            return static_cast<sIter<Tdata, Titer> &>(*this);
        }
        operator const sIter<Tdata, Titer> & () const
        {
            return static_cast<const sIter<Tdata, Titer> &>(*this);
        }
    };

    /*! \brief Mixin for a read-write iterator over a fixed number of values
     * \tparam T Type iterated over; assumed to be a built-in type or a small struct that is cheap to allocate */
    template <class Tdata, class Titer>
    class sMutableFixedLengthIter
    {
    public:
        sMutableIter<Tdata, Titer> * clone() const
        {
            return static_cast<const Titer*>(this)->clone_impl();
        }
        Tdata & operator[](idx i)
        {
            return static_cast<Titer*>(this)->dereferenceMutableAt_impl(i);
        }
        operator sIter<Tdata, Titer> & ()
        {
            return static_cast<sIter<Tdata, Titer> &>(*this);
        }
        operator const sIter<Tdata, Titer> & () const
        {
            return static_cast<const sIter<Tdata, Titer> &>(*this);
        }
    };

    /*! \brief Read-only iterator for a flat buffer of T
     * \tparam T Type iterated over; assumed to be a built-in type or a small struct that is cheap to allocate */
    template <class T>
    class sBufferIter : public sIter<T, sBufferIter<T> >, public sFixedLengthIter<T, sBufferIter<T> >
    {
    protected:
        const T* _buf;
        idx _dim;
        idx _i;
    public:
        inline void requestData_impl() {}
        inline void releaseData_impl() {}
        inline bool readyData_impl() const { return true; }
        inline bool validData_impl() const { return _buf != NULL && _i >= 0 && _i < _dim; }
        inline idx pos_impl() const { return _i; }
        inline idx segment_impl() const { return 0; }
        inline idx segmentPos_impl() const {return _i;}

        sBufferIter<T>(const T* buf = NULL, idx bufdim = 0): _buf(buf), _dim(bufdim), _i(0) {}
        sBufferIter<T>(const sBufferIter<T> &rhs): _buf(rhs._buf), _dim(rhs._dim), _i(rhs._i) {}
        inline sBufferIter<T>* clone_impl() const { return new sBufferIter<T>(*this); }
        inline sBufferIter<T>& increment_impl() { ++_i; return *this; }
        inline sBufferIter<T>& incrementBy_impl(idx i) { _i += i; return *this; }
        inline idx difference_impl(const sBufferIter<T> &rhs) const { return _buf + _i - (rhs._buf + rhs._i); }
        inline bool equals_impl(const sBufferIter<T> &rhs) const
        {
            return _buf + _i == rhs._buf + rhs._i && _buf + _dim == rhs._buf + rhs._dim;
        }
        inline bool lessThan_impl(const sBufferIter<T> &rhs) const { return _buf + _i < rhs._buf + rhs._i; }
        inline bool greaterThan_impl(const sBufferIter<T> &rhs) const { return _buf + _i > rhs._buf + rhs._i; }
        inline T dereference_impl() const { return _buf[_i]; }
        inline const T* operator->() const { return _buf + _i; }
        inline T dereferenceAt_impl(idx i) const { return _buf[_i + i]; }

        inline idx dim_impl() const { return _dim; }
    };

    /*! \brief Read-write iterator for a flat buffer of T
     * \tparam T Type iterated over; assumed to be a built-in type or a small struct that is cheap to allocate
     */
    template <class T>
    class sMutableBufferIter : public sBufferIter<T>, sMutableFixedLengthIter<T, sMutableBufferIter<T> >
    {
    protected:
        T* _rwbuf;

    public:
        sMutableBufferIter<T>(T* buf = NULL, idx bufdim = 0): sBufferIter<T>(buf, bufdim), _rwbuf(buf) {}
        sMutableBufferIter<T>(const sBufferIter<T> &rhs): sBufferIter<T>(rhs), _rwbuf(rhs._rwbuf) {}
        inline sMutableBufferIter<T>* clone_impl() const { return new sMutableBufferIter<T>(*this); }
        inline T& dereferenceMutable_impl() { return _rwbuf[sBufferIter<T>::_i]; }
        inline T* operator->() { return _rwbuf + sBufferIter<T>::_i; }
        inline T& dereferenceMutableAt_impl(idx i) { return _rwbuf[sBufferIter<T>::_i + i]; }
    };

};

#endif
