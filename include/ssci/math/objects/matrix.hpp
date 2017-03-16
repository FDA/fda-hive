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
#ifndef sLib_math_matrix_hpp
#define sLib_math_matrix_hpp

#include <ssci/math/algebra/algebra.hpp>
#include <ssci/math/nr/nr.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/str.hpp>
#include <slib/utils/tbl.hpp>

// from Lapack
//void dgeev( char* jobvl, char* jobvr, int* n, double* a, int* lda, double* wr, double* wi, double* vl, int* ldvl, double* vr, int* ldvr, double* work, int* lwork, int* info );
//#include <cblas.h>
//#include <lapacke.h>
//#include <clapack.h>
//#include <gsl/gsl_math.h>

namespace slib
{
    class sMatrix : private sVec <real>
    {
        real * ptr(idx index=0){return (real *) (((char*)sVec<real>::ptr(index))+sizeof(Head)) ;}
        const real * ptr(idx index=0) const {return (real *) (((char*)sVec<real>::ptr(index))+sizeof(Head)) ;}
    public:
        struct Head{idx rows, cols;};
        Head * head(void){return (Head *)(sVec<real>::ptr());}
        const Head * head(void) const {return reinterpret_cast<const Head*>(sVec<real>::ptr());}

        sMatrix(idx lflags=0, const char * flnm=0 ):sVec<real>( lflags, flnm ){}
        sMatrix( const char * flnm):sVec<real>( flnm)  {}

        sMatrix * init(const char * flnm){sVec<real>::init(flnm);return this;}
        sMatrix * init(idx flags){sVec<real>::init(flags);return this;}

        real * ptr(idx ir, idx ic){return ptr(ir*cols()+ic);}
        const real * ptr(idx ir, idx ic) const {return ptr(ir*cols()+ic);}
        real & operator() (idx ir, idx ic){return *ptr(ir*cols()+ic);}
        const real & operator() (idx ir, idx ic) const {return *ptr(ir*cols()+ic);}
        real * operator [](idx ir){return ptr(ir*cols());}
        const real * operator [](idx ir) const {return ptr(ir*cols());}
        real & val(idx ir, idx ic){return *ptr(ir*cols()+ic);}
        const real & val(idx ir, idx ic) const {return *ptr(ir*cols()+ic);}

        idx rows() const {const Head * h=head(); return h ? h->rows : 0;}
        idx cols() const {const Head * h=head(); return h ? h->cols : 0 ;}
        real * resize(idx r,idx c){sVec<real>::resize(r*c+2-1);Head *h=head();h->rows=r; h->cols=c;return ptr();}
        void set(real o){memset(ptr(),(int)o,rows()*cols()*sizeof(real));}
        void empty(void){sVec<real>::empty();}
        void destroy(){sVec<real>::destroy();}

        typedef void (* MatrixOutput)(sStr * out, void * param, void * pVal, idx row, idx col, bool forCSV);
        struct MatrixDicHeaders {
            sDic < idx > * cols, * rows; sVec < idx > * colset, * rowset;
            MatrixDicHeaders(void){cols=0;rows=0;colset=0;rowset=0;}
        };
        static void matOutput(sStr * out, void * , real * pVal, idx row, idx col, bool forCSV=true);
        //static void matOutputDicHeader(sStr * out, MatrixDicHeaders  * op, real * pVal, idx row, idx col);

        void out(sStr * ot, void * param=0, bool transpose=false, bool header=false, const char * fmt="%lf", MatrixOutput ho=0,idx colstart=0, idx colend=0, idx rowstart=0, idx rowend=0) const;
        void out(const char * flnm, void * param=0, bool transpose=false, bool header=false, const char * fmt="%lf", MatrixOutput ho=0, idx colstart=0, idx colend=0, idx rowstart=0, idx rowend=0) const;
        void out(sStr * ot, MatrixDicHeaders * param, bool transpose=false, bool header=false, const char * fmt="%lf", idx colstart=0, idx colend=0, idx rowstart=0, idx rowend=0) const {
            out(ot, (void * )param, transpose, header, fmt, (MatrixOutput)matOutput, colstart, colend, rowstart, rowend);
        }
        void outSingleEvecSrt(sStr * out, real * evals, idx col, sDic < idx > * ids, sStr * outshort=0) const;

        void copy(sMatrix & newmat, bool transpose=false) const;
        void extractRowset(sMatrix & newmat, sVec < idx> & rowset) const;
        void extractRowset(sMatrix & newmat, sVec < sVec < idx > > & rowset) const;
        void extractColset(sMatrix & newmat, sVec < idx> & colset) const;
        void extractColset(sMatrix & newmat, sVec < sVec < idx > > & colset) const;

        void shiftRows(const real * shift, real scale=1){sAlgebra::matrix::shiftRows(ptr(0,0),rows(),cols(),shift,scale);}
        void cutoffMinVal(real mintol){sAlgebra::matrix::cutoffMinVal(ptr(0,0),rows(),cols(),mintol);}
        void multiplyMatrixes(const sMatrix & mat1, const sMatrix & mat2, bool transposedsecond=false){resize(mat1.rows(),transposedsecond ? mat2.rows() : mat2.cols() );sAlgebra::matrix::multiplyToMatrix(mat1.ptr(0,0),mat1.rows(),mat1.cols(),mat2.ptr(0,0),mat2.rows(),mat2.cols(),ptr(0,0),transposedsecond);return ;}
        void transpose(void){sAlgebra::matrix::transpose(ptr(0,0),rows(),cols());}
        void normalilzeCols(bool sqr=true, real scale=1.){sAlgebra::matrix::normalizeCols(ptr(0,0),rows(),cols(),false,sqr,scale);}
        void normalilzeRows(bool sqr=true, real scale=1.){sAlgebra::matrix::normalizeCols(ptr(0,0),rows(),cols(),true,sqr,scale);}
        void sortOrderByCol(idx * indexes, idx col) const;
        void inverse(void){sMathNR::inverse_matrix(ptr(0,0), cols());}
        void inverseR(void){sMathNR::inverse_matrix_lud(ptr(0,0), cols());}

        void squareRoot(sMatrix & result) const;

        struct DistrRowSetStruc {
            sVec< real> * distributionPerRow;
            sVec< real> * distributionPerCol;
            real * dPerColStdDev, * dPerColMean,* dPerColStdDevCls, * dPerColMeanCls;
            real * dPerRowStdDev, * dPerRowMean;
            real * dPerColArm, * dPerRowArm;
        };

        void statisticsRowset( DistrRowSetStruc * ds, sDic < sVec < idx > > & rowsetset) const;


        void computeRowStat(real * aves,real * stddev=0, real * minp=0, real * maxp=0){sAlgebra::matrix::computeRowStat(ptr(0,0),rows(),cols(),aves,stddev,minp,maxp);}
        void covariance(sMatrix & covararr, real scl=0){covararr.resize(cols(),cols());sAlgebra::matrix::covariance(ptr(0,0),rows(),cols(),covararr.ptr(0,0),scl ? scl : rows()-1);}
        void scatter(sMatrix & scatter, real scl=0){scatter.resize(cols(),cols());sAlgebra::matrix::covariance(ptr(0,0),rows(),cols(),scatter.ptr(0,0),scl ? scl : rows());}
        void pca(sVec < real > & evals , sMatrix & evecs)
        {
            idx n=cols();
            evecs.resize(n,n);
            evals.resize(n);
            sAlgebra::matrix::pca(ptr(0,0), rows(), cols(), evals.ptr(0), evecs.ptr(0,0) );
        }


        idx diagJacoby(real * evals, sMatrix * evecs, idx issort=-1)
        {
            idx n=cols();
            evecs->resize(n,n);
            sVec < real > B; B.resize(n*2);
            sAlgebra::matrix::diagJacoby(n, ptr(0,0),evals,evecs->ptr(0,0),B,1e-24,1000);
            if(issort)sAlgebra::matrix::diagSort(n,evals,evecs->ptr(0,0),issort);
            return 1;
        }
        idx diagNonSymOld(real * evals, sMatrix * evecs, idx issort=-1){
            transpose();
            idx n=cols(),nm=rows(); real t=24;
            sVec <idx> indic; indic.resize(n);
            sAlgebra::matrix::diagNonSym( &n,&nm, ptr(0,0), &t, evals,evals+n, evecs->ptr(0,0),evecs->ptr(rows(),0), indic );
            evecs->transpose();
            if(issort)sAlgebra::matrix::diagSort(n,evals,evecs->ptr(0,0),issort);

            return 1;
        }
        idx diagNonSym_original(real * evals, sMatrix * evecs, idx issort=-1){
            transpose();
            idx n=cols(),nm=rows(); real t=24;
            sVec <idx> indic; indic.resize(n);
            sAlgebra::matrix::diagNonSym( &n,&nm, ptr(0,0), &t, evals,evals+n, evecs->ptr(0,0),evecs->ptr(rows(),0), indic );
            evecs->transpose();
            if(issort)sAlgebra::matrix::diagSort(n,evals,evecs->ptr(0,0),issort);

            return 1;
        }

        idx diagNonSym(real * evals, sMatrix * evecs, idx issort=-1)
        {
            idx n=cols();
            sVec <idx> iters;iters.resize(n);
            sAlgebra::matrix::eigen(
                   1,           /* switch for computing evectors ...*/
                   0,         /* orthogonal Hessenberg reduction? */
                   0,       /* normalize Eigenvectors? .........*/
                   n,             /* size of matrix ..................*/
                   ptr(0,0),           /* input matrix ....................*/
                   evecs->ptr(0,0),         /* Eigenvectors ....................*/
                   evals,         /* real parts of eigenvalues .......*/
                   evals+n,         /* imaginary parts of eigenvalues ..*/
                   iters.ptr(0)            /* Iteration counter ...............*/
                   );
            if(issort)sAlgebra::matrix::diagSort(n,evals,evecs->ptr(0,0),issort);

            return 1;
        }
        idx diagNonSym2(real * evals, sMatrix * evecs, idx issort=-1){
            idx n=cols(),nm=rows(); real t=24;
            sVec <idx> indic; indic.resize(n);
            sAlgebra::matrix::diagNonSym( &n,&nm, ptr(0,0), &t, evals,evals+n, evecs->ptr(0,0),evecs->ptr(rows(),0), indic );
            if(issort)sAlgebra::matrix::diagSort(n,evals,evecs->ptr(0,0),issort);

            return 1;
        }



        idx parseCsv(sStr * flbuf, const char* src, idx len, sVec < sMex::Pos> * colIds, sVec < sMex::Pos> * rowIds,const char * ignoresym=0, idx dicMode=0, real binThreshold=0, bool readNumsAsNums=false, bool transpose=false, bool supportquote=false, const char * filterRows=0, const char * filterCols=0, idx nonzeroMin=0, idx zeroMax=0);
        idx parseTabular(sTabular * tbl, sVec< idx > * rowSet, sVec< idx > * colSet, sDic < idx> * colIds, sDic < idx > * rowIds, const char * ignoresym=0, idx dicMode=0, real binThreshold=0, bool readNumsAsNums=false, sVec< idx > * revertColSet=0 , idx forceRowID=0);


    };

}

#endif // sLib_math_func_hpp




