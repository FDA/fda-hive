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
#ifndef sMath_nr_hpp
#define sMath_nr_hpp

#include <slib/core/def.hpp>

namespace slib
{
    class sMathNR {

        public:
                        
            typedef real (*callBackOptim)(real [] , void * );

                        // optimization taskks
            static void linmin(real p[], real xi[], idx n, real *fret, callBackOptim func, void * usr);
            static real f1dim(real x, void * usr);
            static real brent(real ax, real bx, real cx, real (*f)(real, void * ), real tol, real *xmin, void * usr);
            static void mnbrak(real *ax, real *bx, real *cx, real *fa, real *fb, real *fc,  real (*func)(real, void * ), void * usr);
            static idx powell(idx maxiter, real p[], idx n, real ftol, real *fret,callBackOptim func, void * usr);

                        // matrix inversion
            static idx gaussj(real * A, idx n, real *B, idx m);
            static idx inverse_matrix(real *A, idx n);
            static void inverse_matrix_lud(real *A, idx n);


                        /// smoothing
            static void savgol(real c[], idx np, idx nl, idx nr, idx ld, idx m);

            class SavGol{
                public:
                real * cSavGol; 
                idx left, right, degree, np, datadim;
                SavGol(idx datadim, idx lleft, idx lright, idx ldegree);
                void computeFilterCoef(real c[], idx np, idx nl, idx nr, idx ld, idx m);
                
                ~SavGol();
                void smooth( real * dst, real * src ) ;
            };
            
            // matrix division
            static void ludcmp(real **a, idx n, idx *indx, real *d);
            static void lubksb(real **a, idx n, idx *indx, real b[]);
            
/*            static void ludcmpR(real *A, idx n, idx *indx, real *d);
            static void lubksbR(real *A, idx n, idx *indx, real *b);
*/
            // matrix diagonalizaion
            static idx jacobi(real *arr, idx n, real * evals, real * evecs, real jactol, idx maxiter);


            // fourier transforms
            static void four1(real data[], udx nn, idx isign);
            static void realft(real data[], udx n, idx isign);
            static void twofft(real data1[], real data2[], real fft1[], real fft2[], udx n);
            static void convlv(real data[], udx n, real respns[], udx m, idx isign, real ans[]);
            
            static void fftCutoff(real * src, real * dst , idx cnt, idx daubNum, idx cutMin, idx cutMax);

            // wavelets 
            typedef void (*waveletfunc)(real [], udx, idx);
            static void wt1(real a[], idx n, idx isign, waveletfunc func );
            struct wavefilt{
                    udx ncof,ioff,joff;
                    real *cc,*cr;
            } ;
            static wavefilt wfilt;
            static void pwtset(idx n);
            static void daub4(real a[], udx n, idx isign);
            static void pwt(real a[], udx n, idx isign);

            static real select(udx k, udx n, real arr[]) ; // select k-th largest member of the array of n

    };

}

#endif // sMath_nr_hpp




