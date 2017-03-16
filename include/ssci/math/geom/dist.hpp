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
#ifndef sMath_dist_h
#define sMath_dist_h

#include <math.h>
#include <assert.h>

#include <slib/core/def.hpp>
#include <slib/core/iter.hpp>
#include <slib/core/iteriter.hpp>
#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>

#define SDIST_LOOP(code) \
{ \
    Titer itpair[2]; \
    itpair[0] = x; \
    itpair[1] = y; \
    for(sIterPair<Tdata,Titer> pair(x, y); pair.valid(); ++pair) { \
        Tdata _x(pair.deref_x()); \
        Tdata _y(pair.deref_y()); \
        code \
    } \
}

namespace slib
{
    //! Base class for a distance accumulator between a pair of vectors
    /*! \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sIter<Tdata> */
    template <class Tdata, class Titer>
    class sDistAccum
    {
    public:
        //! Accumulate the distance between a corresponding pair of values from the two vectors
        virtual void accum(Tdata x, Tdata y) = 0;
        //! Accumulate the distance between a corresponding pair of segments from the two vectors
        virtual void accumSegment(const Titer &xseg, const Titer &yseg)
        {
            for(sIterPair<Tdata,Titer> segpair(xseg, yseg); segpair.valid(); ++segpair) {
                accum(segpair.deref_x(), segpair.deref_y());
            }
        }
        virtual real result() const = 0;
        virtual ~sDistAccum<Tdata,Titer>() {}
    };

    //! Accumulator for the Euclidean metric, a.k.a. <em>L</em><sup>2</sup> metric
    template <class Tdata, class Titer>
    class sEuclideanDistAccum : public sDistAccum<Tdata,Titer>
    {
    protected:
        real _d;
    public:
        sEuclideanDistAccum<Tdata,Titer>(): _d(0) {}
        inline void accum(Tdata x, Tdata y)
        {
            real diff = x - y;
            _d += diff * diff;
        }
        //! \returns \f$\sqrt{\sum (x_i - y_i)^2}\f$
        inline real result() const { return sqrt(_d); }
    };

    //! Accumulator for the Manhattan metric, a.k.a. taxicab metric, a.k.a. <em>L</em><sup>1</sup> metric
    template <class Tdata, class Titer>
    class sManhattanDistAccum : public sDistAccum<Tdata,Titer>
    {
    protected:
        real _d;
    public:
        sManhattanDistAccum<Tdata,Titer>(): _d(0) {}
        inline void accum(Tdata x, Tdata y)
        {
            //_d += sAbs<Tdata>(x - y);
            _d += x>y ? (x - y) : y-x;
        }
        //! \returns \f$\sum \|x_i - y_i\|\f$
        inline real result() const { return _d; }
    };

    //! Accumulator for the maximum metric, a.k.a. uniform metric, \htmlonly a.k.a. <em>L</em><sup>&infin;</sup> metric \endhtmlonly
    template <class Tdata, class Titer>
    class sMaximumDistAccum : public sDistAccum<Tdata,Titer>
    {
    protected:
        real _d;
    public:
        sMaximumDistAccum<Tdata,Titer>(): _d(0) {}
        inline void accum(Tdata x, Tdata y)
        {
            //real diff = sAbs<Tdata>(x - y);
            real diff = x>y ? (x - y) : y-x;
            if (diff > _d)
                _d = diff;
        }
        //! \returns \f$\max \|x_i - y_i\|\f$
        inline real result() const { return _d; }
    };

    //! Accumulator for the <em>p</em>-norm, a.k.a. <em>L<sup>p</sup></em> metric
    template <class Tdata, class Titer>
    class sPNormDistAccum : public sDistAccum<Tdata,Titer>
    {
    protected:
        real _d;
        real _p;
    public:
        //! \param p >= 1
        sPNormDistAccum<Tdata,Titer>(real p = 2): _d(0), _p(p) { assert(_p >= 1); }
        inline void accum(Tdata x, Tdata y)
        {
            _d += pow(sAbs<Tdata>(x - y), _p);
        }
        //! \returns \f$\left(\sum\|x_i - y_i\|^p\right)^\frac{1}{p}\f$
        inline real result() const { return pow(_d, (real)1.0/_p); }
    };

    //! Accumulator for the Canberra distance
    template <class Tdata, class Titer>
    class sCanberraDistAccum : public sDistAccum<Tdata,Titer>
    {
    protected:
        real _d;
    public:
        sCanberraDistAccum<Tdata,Titer>(): _d(0) {}
        inline void accum(Tdata x, Tdata y)
        {
            if (x != 0 || y != 0)
                _d += sAbs<Tdata>(x - y) / (sAbs<Tdata>(x) + sAbs<Tdata>(y));
        }
        //! \returns \f$\sum \frac{|x_i-y_i|}{|x_i|+|y_i|}\f$
        inline real result() const { return _d; }
    };

    //! Accumulator for the cosine similarity
    /*! \note see http://reference.wolfram.com/mathematica/ref/CosineDistance.html */
    template <class Tdata, class Titer>
    class sCosineDistAccum : public sDistAccum<Tdata,Titer>
    {
    protected:
        real _d, _sprod, _varx, _vary;
    public:
        sCosineDistAccum<Tdata,Titer>(): _d(0), _sprod(0), _varx(0), _vary(0) {}
        inline void accum(Tdata x, Tdata y)
        {
            _sprod += x * y;
            _varx += x * x;
            _vary += y * y;
        }
        //! \returns \f$1 - \cos(\theta) = 1 - \frac{\sum x_i y_i}{\sqrt{\sum x_i^2} \sqrt{\sum y_i^2}}\f$
        inline real result() const
        {
            real rootvarxy = sqrt(_varx * _vary);
            return rootvarxy > 0 ? 1 - _sprod / rootvarxy : 0;
        }
    };

    /*! \brief Distance or similarity function for a pair of vectors, which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sDist
    {
    public:
        /*! Pure virtual overridden in derived classes, useful when passing a generic distance function as a parameter */
        virtual real dist(const Titer &x, const Titer &y) const = 0;

        virtual sDist<Tdata,Titer>* clone() const = 0;

        virtual ~sDist<Tdata,Titer>() {}

        /* Individual distance functions are available as static methods for ease of use
           when a specific distance function is needed in code */
        //! \returns \f$\sqrt{\sum (x_i - y_i)^2}\f$
        static real euclidean(const Titer &x, const Titer &y)
        {
            sEuclideanDistAccum<Tdata,Titer> d;
            d.accumSegment(x, y);
            return d.result();
        }

        //! \returns \f$\sum \|x_i - y_i\|\f$
        static real manhattan(const Titer &x, const Titer &y)
        {
            sManhattanDistAccum<Tdata,Titer> d;
            d.accumSegment(x, y);
            return d.result();
        }

        //! \returns \f$\max \|x_i - y_i\|\f$
        static real maximum(const Titer &x, const Titer &y)
        {
            sMaximumDistAccum<Tdata,Titer> d;
            d.accumSegment(x, y);
            return d.result();
        }

        //! \returns \f$\left(\sum\|x_i - y_i\|^p\right)^\frac{1}{p}\f$
        static real pNorm(const Titer &x, const Titer &y, real p = 2)
        {
            sPNormDistAccum<Tdata,Titer> d(p);
            d.accumSegment(x, y);
            return d.result();
        }

        //! \returns \f$\sum \frac{|x_i-y_i|}{|x_i|+|y_i|}\f$
        static real canberra(const Titer &x, const Titer &y)
        {
            sCanberraDistAccum<Tdata,Titer> d;
            d.accumSegment(x, y);
            return d.result();
        }

        //! \returns \f$1 - \cos(\theta) = 1 - \frac{\sum x_i y_i}{\sqrt{\sum x_i^2} \sqrt{\sum y_i^2}}\f$
        static inline real cosine(const Titer &x, const Titer &y)
        {
            sCosineDistAccum<Tdata,Titer> d;
            d.accumSegment(x, y);
            return d.result();
        }

        // Pearson correlation cannot be efficiently represented as an accumulator

        /*! Similarity derived from Pearson correlation coefficient
         * \returns \f$1 - \frac{\sum(x_i - \bar{x})(y_i - \bar{y})}{\sqrt{\sum (x_i - \bar{x})^2} \sqrt{\sum (y_i - \bar{y})^2}}\f$ by default; \f$1 - \frac{\sum x_i y_i}{\sqrt{\sum x_i^2} \sqrt{\sum y_i^2}}\f$ if uncentered; \f$\left(\frac{\sum(x_i - \bar{x})(y_i - \bar{y})}{\sqrt{\sum (x_i - \bar{x})^2} \sqrt{\sum (y_i - \bar{y})^2}}\right)^2\f$ if squared
         */
        static real pearson(const Titer &x, const Titer &y, bool uncentered = false, bool squared = false)
        {
            real similarity = 0, meanx = 0, varx = 0, meany = 0, vary = 0;

            if (!uncentered) {
                real sumx = 0, sumy = 0;
                idx dim = 0;
                SDIST_LOOP(
                    sumx += _x;
                    sumy += _y;
                    dim++;
                )
                if (dim > 0) {
                    meanx = sumx/dim;
                    meany = sumy/dim;
                }
            }

            SDIST_LOOP(
                _x -= meanx;
                _y -= meany;
                similarity += _x * _y;
                varx += _x * _x;
                vary += _y * _y;)

            if (varx * vary == 0.0)
                return 0.0;

            similarity /= sqrt(varx * vary);
            if (squared)
                return similarity * similarity;

            return 1 - similarity;
        }
    };

    /*! \brief Euclidean metric, a.k.a. <em>L</em><sup>2</sup> metric, for a pair of vectors which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sEuclideanDist : public sDist<Tdata, Titer>
    {
    public:
        //! \returns \f$\sqrt{\sum (x_i - y_i)^2}\f$
        inline real dist(const Titer &x, const Titer &y) const {return sDist<Tdata, Titer>::euclidean(x,y);}
        inline sEuclideanDist<Tdata,Titer>* clone() const {return new sEuclideanDist<Tdata,Titer>();}
    };

    /*! \brief Manhattan metric, a.k.a. taxicab metric, a.k.a. <em>L</em><sup>1</sup> metric, for a pair of vectors which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sManhattanDist : public sDist<Tdata, Titer>
    {
    public:
        //! \returns \f$\sum \|x_i - y_i\|\f$
        inline real dist(const Titer &x, const Titer &y) const {return sDist<Tdata, Titer>::manhattan(x,y);}
        inline sManhattanDist<Tdata,Titer>* clone() const {return new sManhattanDist<Tdata,Titer>();}
    };

    /*! \brief Maximum metric, a.k.a. uniform metric, \htmlonly a.k.a. <em>L</em><sup>&infin;</sup> metric, \endhtmlonly for a pair of vectors which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sMaximumDist : public sDist<Tdata, Titer>
    {
    public:
        //! \returns \f$\max \|x_i - y_i\|\f$
        inline real dist(const Titer &x, const Titer &y) const {return sDist<Tdata, Titer>::maximum(x,y);}
        inline sMaximumDist<Tdata,Titer>* clone() const {return new sMaximumDist<Tdata,Titer>();}
    };

    /*! \brief <em>p</em>-norm, a.k.a. <em>L<sup>p</sup></em> metric, for a pair of vectors which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sPNormDist : public sDist<Tdata, Titer>
    {
    protected:
        real _p;
    public:
        sPNormDist<Tdata,Titer>(real p = 2.0): _p(p) {}
        //! \returns \f$\left(\sum\|x_i - y_i\|^p\right)^\frac{1}{p}\f$
        inline real dist(const Titer &x, const Titer &y) const {return sDist<Tdata, Titer>::pNorm(x,y, _p);}
        inline sPNormDist<Tdata,Titer>* clone() const {return new sPNormDist<Tdata,Titer>(_p);}
    };

    /*! \brief Canberra distance for a pair of vectors which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sCanberraDist : public sDist<Tdata, Titer>
    {
    public:
        //! \returns \f$\sum \frac{|x_i-y_i|}{|x_i|+|y_i|}\f$
        inline real dist(const Titer &x, const Titer &y) const {return sDist<Tdata, Titer>::canberra(x,y);}
        inline sCanberraDist<Tdata,Titer>* clone() const {return new sCanberraDist<Tdata,Titer>();}
    };

    /*! \brief Similarity derived from Pearson correlation coefficient for a pair of vectors which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sPearsonDist : public sDist<Tdata, Titer>
    {
    protected:
        bool _uncentered;
        bool _squared;
    public:
        sPearsonDist<Tdata, Titer>(bool uncentered = false, bool squared = false): _uncentered(uncentered), _squared(squared) {}
        //! \returns \f$1 - \frac{\sum(x_i - \bar{x})(y_i - \bar{y})}{\sqrt{\sum (x_i - \bar{x})^2} \sqrt{\sum (y_i - \bar{y})^2}}\f$ by default; \f$1 - \frac{\sum x_i y_i}{\sqrt{\sum x_i^2} \sqrt{\sum y_i^2}}\f$ if uncentered; \f$\left(\frac{\sum(x_i - \bar{x})(y_i - \bar{y})}{\sqrt{\sum (x_i - \bar{x})^2} \sqrt{\sum (y_i - \bar{y})^2}}\right)^2\f$ if squared
        inline real dist(const Titer &x, const Titer &y) const {return sDist<Tdata,Titer>::pearson(x,y, _uncentered, _squared);}
        inline sPearsonDist<Tdata,Titer>* clone() const {return new sPearsonDist<Tdata,Titer>(_uncentered, _squared);}
    };

    /*! \brief Cosine similarity for a pair of vectors which are represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    template <class Tdata, class Titer>
    class sCosineDist : public sDist<Tdata, Titer>
    {
    public:
        //! \returns \f$1 - \cos(\theta) = 1 - \frac{\sum x_i y_i}{\sqrt{\sum x_i^2} \sqrt{\sum y_i^2}}\f$
        inline real dist(const Titer &x, const Titer &y) const {return sDist<Tdata,Titer>::cosine(x,y);}
        inline sCosineDist<Tdata,Titer>* clone() const {return new sCosineDist<Tdata,Titer>();}
    };

    class sDistMatrix;

    /*! \brief Iterator over a row of an sDistMatrix
     */
    class sDistMatrixRowIter: public sIter<real, sDistMatrixRowIter>, public sFixedLengthIter<real, sDistMatrixRowIter>
    {
    protected:
        const sDistMatrix &_matrix;
        const idx _x;
        idx _y;
        inline void assertComparable(const sDistMatrixRowIter &rhs) const {assert(_x == rhs._x);}
    public:
        sDistMatrixRowIter(const sDistMatrix &matrix, idx x): _matrix(matrix), _x(x), _y(0) {}
        sDistMatrixRowIter(const sDistMatrixRowIter &rhs): _matrix(rhs._matrix), _x(rhs._x), _y(rhs._y) {}
        sDistMatrixRowIter* clone_impl() const {return new sDistMatrixRowIter(*this);}
        bool validData_impl() const;
        inline sDistMatrixRowIter& increment_impl() {++_y; return *this;}
        inline sDistMatrixRowIter& incrementBy_impl(idx i) {_y += i; return *this;}
        inline idx difference_impl(const sDistMatrixRowIter &rhs) const {assertComparable(rhs); return _y - rhs._y;}
        inline bool equals_impl(const sDistMatrixRowIter &rhs) const {return _x == rhs._x && _y == rhs._y;}
        inline bool lessThan_impl(const sDistMatrixRowIter &rhs) const {assertComparable(rhs); return _y < rhs._y;}
        inline bool greaterThan_impl(const sDistMatrixRowIter &rhs) const {assertComparable(rhs); return _y > rhs._y;}
        real dereference_impl() const;
        real dereferenceAt_impl(idx i) const;
        idx dim_impl() const;
        inline idx pos_impl() const {return _y;} //!< current position, relative to initial one
    };

    /*! \brief Symmetric distance matrix between vectors (points) represented as iterators
     * \tparam Tdata Type that Titer iterates over
     * \tparam Titer Assumed to have the same interface as sConstBaseIter<Tdata>; we use the iterator as a template parameter to allow using arrays of iterators (as opposed to arrays of sConstBaseIter<Tdata> pointers)
     */
    class sDistMatrix
    {
    public:
        typedef idx (*PointPrintfCallback)(sStr &out, sDistMatrix &matrix, idx x, void *param);
        typedef real (*DistCallback)(idx x, idx y, void * param);

    protected:
        idx _npoints; //!< number of points
        sVec<real> _array; //!< upper triangular distance array
        sVec<bool> _valid; //!< whether point still represents a valid point, or if it was removed

        inline idx arraydim() const { return (_npoints*(_npoints+1))>>1; } //!< size of _array
        inline idx index(idx x, idx y) const { return x <= y ? x*(_npoints-1) - (x*(x-1)>>1) + y : index(y, x); } //!< position in _array of distance between _points[x] and _points[y]

    public:
        //*! Same interface as sQPrideProc::reqProgress()
        typedef idx (*progressCb)(void * param, idx items, idx progress, idx progressMax);

        template<class Tdata, class Titer>
        bool reset(const Titer *points, const idx npoints, const sDist<Tdata,Titer> &distObj, progressCb prog_cb = 0, void * prog_param = 0)
        {
            if( prog_cb ) {
                prog_cb(prog_param, 0, sNotIdx, npoints * npoints);
            }
            _npoints = npoints;
            _array.resize(arraydim());
            _valid.resize(_npoints);
            for(idx x=0; x<npoints; x++) {
                _valid[x] = true;
                for (idx y=x; y<npoints; y++) {
                    setDist(x, y, distObj.dist(points[x], points[y]));
                    if( prog_cb ) {
                        prog_cb(prog_param, x * npoints + y, sNotIdx, npoints * npoints);
                    }
                }
            }
            return true;
        }

        virtual bool reset(const idx npoints, DistCallback d, void * param, progressCb prog_cb = 0, void * prog_param = 0)
        {
            if( prog_cb ) {
                prog_cb(prog_param, 0, sNotIdx, npoints * npoints);
            }
            _npoints = npoints;
            _array.resize(arraydim());
            _valid.resize(_npoints);
            for(idx x=0; x<npoints; x++) {
                _valid[x] = true;
                for (idx y=x; y<npoints; y++) {
                    setDist(x, y, d(x, y, param));
                    if( prog_cb ) {
                        prog_cb(prog_param, x * npoints + y, sNotIdx, npoints * npoints);
                    }
                }
            }
            return true;
        }

        virtual bool reset(idx npoints)
        {
            _npoints = npoints;
            _array.resize(arraydim());
            _valid.resize(_npoints);
            for(idx x=0; x<npoints; x++) {
                _valid[x] = true;
                for (idx y=0; y<npoints; y++)
                    setDist(x, y, 0);
            }
            return true;
        }

        // for use by derived classes
        virtual bool reset(const sDistMatrix &rhs)
        {
            if (&rhs == this)
                return true;

            _npoints = rhs._npoints;
            _array.resize(arraydim());
            memcpy(_array.ptr(), rhs._array.ptr(), sizeof(real) * arraydim());
            _valid.resize(_npoints);
            memcpy(_valid.ptr(), rhs._valid.ptr(), sizeof(bool) * _npoints);
            return true;
        }

        sDistMatrix(idx npoints=0): _array(sMex::fExactSize), _valid(sMex::fExactSize) { reset(npoints); }
        sDistMatrix(const sDistMatrix &rhs): _array(sMex::fExactSize), _valid(sMex::fExactSize) { reset(rhs); }
        virtual ~sDistMatrix() { }

        inline virtual bool inRange(idx x) const {return x >= 0 && x < _npoints;} //!< \returns true if 0 <= x < dim()
        inline virtual real dist(idx x, idx y) const {assert(inRange(x)); return *(_array.ptr(index(x,y)));} //!< returns distance between points x and y
        inline virtual sDistMatrixRowIter rowIter(idx x) const {assert(inRange(x)); return sDistMatrixRowIter(*this, x);} //!< \returns sDistMatrixRowIter over matrix row x
        inline virtual void setDist(idx x, idx y, real dist) {assert(inRange(x)); _array[index(x,y)] = dist;} //!< Set distance between x and y to dist
        inline virtual idx dim() const {return _npoints;} //!< \returns Number of points (including both valid and invalid)
        inline virtual bool valid(idx x) const {assert(inRange(x)); return *(_valid.ptr(x));} //!< \returns Whether x is a valid point, or if it was invalidated

        /*! Set x as invalid */
        inline virtual void setInvalid(idx x) {
            assert(inRange(x));
            _valid[x] = false;
            for (idx y=0; y<_npoints; y++)
                setDist(x, y, NAN);
        }

        virtual void printMatrixCSV(sStr &out, PointPrintfCallback func = NULL, void *param = NULL)
        {
            if (_npoints < 1)
                return;

            out.printf(" ");
            for (idx x=0; x<_npoints; x++) {
                if (func) {
                    out.printf(",");
                    func(out, *this, x, param);
                } else
                    out.printf(",%"DEC, x);
            }
            out.printf("\r\n");
            for (idx x=0; x<_npoints; x++) {
                if (func)
                    func(out, *this, x, param);
                else
                    out.printf("%"DEC, x);

                for (idx y=0; y<_npoints; y++)
                    out.printf(",%g", dist(x, y));

                out.printf("\r\n");
            }
        }
    };

    inline bool sDistMatrixRowIter::validData_impl() const {return _matrix.inRange(_x) && _matrix.inRange(_y);}
    inline real sDistMatrixRowIter::dereference_impl() const {return _matrix.dist(_x, _y);}
    inline real sDistMatrixRowIter::dereferenceAt_impl(idx i) const {return _matrix.dist(_x, _y + i);}
    inline idx sDistMatrixRowIter::dim_impl() const {return _matrix.dim();}
};

#endif
