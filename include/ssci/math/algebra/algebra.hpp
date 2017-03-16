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
#ifndef sMath_algebra_hpp
#define sMath_algebra_hpp

#include <slib/core/def.hpp>

namespace slib
{
    class sAlgebra { // lower level algebraic functions

    public:
        class matrix {
        public:
            ///static void multiplyToMatrix(idx  n,real   * A,real   * B,real   * C);
            static void multiplyToMatrix(const real   * A,idx arows, idx acols, const real   * B, idx brows, idx bcols, real   * C, bool transposesecond=false);
            static void multiplyToVector(const real   * A,idx arows, idx acols, const real   * P,real   * Q);
            static void transpose(real * src, idx rows, idx cols);
            static void normalizeCols(real * src, idx rows, idx cols, bool transpose=false, bool sqr=true, real scale=1.);
            static void shiftRows(real * src, idx rows, idx cols, const real * shift, real scale=1.);
            static void cutoffMinVal(real * src, idx rows, idx cols, real mintol);


            static void diagSort(idx n,real   * evals,real * evecs,idx  issort,bool keepParity =true);
            static idx diagJacoby(idx n ,real * arr,real   * evals,real * evecs,real * B,real jactol=1e-13,idx  maxiter=1000);

            static idx diagGivens(idx nmax,idx nx,idx nrootx,idx njx,real * a,real * root,real * vect,real * b);

            // eigenp_
            static idx  diagNonSym(idx *n, idx *nm, real *a, real *t, real * evr, real *evi, real *vecr, real *veci, idx *indic);

            static void computeRowStat(const real * src, idx rows, idx cols, real * aves,real * stddev=0, real * minp=0, real * maxp=0);
            static void covariance(const real * src, idx rows, idx cols, real * cvr , real scl=0);
            static void pca(real * src, idx rows,idx cols , real * evals, real * evecs );
            static void pcaReMap(real * dst, const real * orig, idx cols, idx rows, const real * evecs);


            static idx eigen (                /* Compute all evalues/evectors of a matrix ..*/
                   idx vec,           /* switch for computing evectors ...*/
                   idx ortho,         /* orthogonal Hessenberg reduction? */
                   idx ev_norm,       /* normalize Eigenvectors? .........*/
                   idx n,             /* size of matrix ..................*/
                   //REAL ** mat,           /* input matrix ....................*/
                   real * mat_,           /* input matrix ....................*/
                   //REAL ** eivec,         /* Eigenvectors ....................*/
                   real * eivec_,         /* Eigenvectors ....................*/
                   real * valre,         /* real parts of eigenvalues .......*/
                   real * valim,         /* imaginary parts of eigenvalues ..*/
                   idx  * cnt            /* Iteration counter ...............*/
                  );


        };

        class integral {
        public:
            enum eIntegralType {
                eTrapezoid=0,
                eN3,
                eN4,
                eSympson
            };
            static real calcUnispace(idx method, const real *y, idx n, real h=1.);

        };
    };
}

#endif // sLib_math_func_hpp




