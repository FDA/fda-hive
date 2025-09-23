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
#ifndef sLib_utils_part_hpp
#define sLib_utils_part_hpp

#include <slib/core/vec.hpp>

namespace slib {

        class sPart
        {

            public:
            typedef enum eOperator_enum {
                eMOV = 0,
                eEQU,
                eLTN,
                eLEQ,
                eGTN,
                eGEQ,
                eADD,
                eSUB,
                eMUL,
                eDIV
            } sOperation;

            typedef idx (* sCallbackALO)(void * param, void * i1, void * i2, sOperation oper);

        private:
            static void reconstruct_partition(sVec<idx> &D, idx w, idx n, idx k_ways, idx *res ) {
                if(k_ways > 1) {
                    n = D[n*w+k_ways];
                    reconstruct_partition(D, w, n, k_ways-1, res);
                    res[k_ways-2] = n;
                }
            }

        public:


            template <class Tobj> static void linearpartition(Tobj * arr, idx n, idx & k_ways, idx * res, idx * sortInds = 0)
            {
                idx ir,ic,ix;
                Tobj a,max,zero;
                sSet(&max,-1);
                sSet(&zero,0);
                sVec < Tobj > presums; presums.add(n);
                presums[0] = zero;
                for (idx i = 1 ; i < presums.dim() ; ++i) {
                    presums[i] = presums[i-1] + arr[sortInds?sortInds[i-1]:i-1];
                }
                sVec< Tobj > M(sMex::fSetZero); sVec<idx> D(sMex::fSetZero);
                idx l = n+1, w= k_ways+1;
                M.add( l * w );D.add( l * w );
                for ( ir = 1 ; ir <= n ; ++ir ) {
                    M[ir*w + 1] = presums[ir + 1];
                }
                for ( ic = 1 ; ic <= k_ways ; ++ic ) {
                    M[w + ic] = arr[sortInds?sortInds[0]:0];
                }

                for ( ir = 2 ; ir <= n ; ++ir) {
                    for ( ic = 2 ; ic <= k_ways ; ++ic ) {
                        M[ir*w + ic] = max;
                        for (ix = 1 ; ix < ir ; ++ix) {
                            a = (M[ix*w+ic-1] <= max) ? (presums[ir]-presums[ix]) : sMax(M[ix*w+ic-1],presums[ir]-presums[ix]);
                            if ( M[ir*w + ic] == max || M[ir*w + ic] >= a ) {
                                M[ ir*w + ic ] = a;
                                D[ ir*w + ic ] = ix;
                            }
                        }
                    }
                }
                reconstruct_partition(D, w, n, k_ways, res);
            }

            template <class Tobj> static void linearpartition(sCallbackALO alu, void * param, Tobj * arr, idx n, idx & k_ways, idx * res, idx * sortInds = 0)
            {
                idx i,j,x;
                if( k_ways > n )k_ways = n;
                idx l = n+1, w= k_ways+1;
                Tobj tmp,max, zero;
                sSet(&max,-1);
                sSet(&zero,0);
                sVec < Tobj > presums;
                presums.add(l);
                alu(param, presums.ptr(0), &zero, eMOV);
                for (i = 1 ; i <= n ; ++i) {
                    alu(param, presums.ptr(i), presums.ptr(i-1), eMOV);
                    alu(param, presums.ptr(i), &arr[sortInds?sortInds[i-1]:i-1], eADD);
                }
                sVec< Tobj > M(sMex::fSetZero); sVec<idx> D(sMex::fSetZero);
                M.add( l * w );D.add( l * w );
                for ( i = 1 ; i <= n ; ++i ) {
                    alu(param, M.ptr(i*w + 1), presums.ptr(i),  eMOV);
                }
                for ( j = 1 ; j <= k_ways ; ++j ) {
                    alu(param, M.ptr(w + j), &arr[sortInds?sortInds[0]:0],  eMOV);
                }

                for ( i = 2 ; i <= n ; ++i) {
                    for ( j = 2 ; j <= k_ways ; ++j ) {
                        alu(param, M.ptr(i*w + j), &max,  eMOV);
                        for (x = 1 ; x < i ; ++x) {
                            alu(param, &tmp, presums.ptr(i), eMOV);
                            alu(param, &tmp, presums.ptr(x), eSUB);
                            if( alu(param, M.ptr(x*w+j-1), &tmp,  eGTN) ) {
                                alu(param, &tmp, M.ptr(x*w+j-1), eMOV);
                            }
                            if ( alu(param, M.ptr(i*w + j), &max,  eEQU) || alu(param, M.ptr(i*w + j), &tmp,  eGEQ) ) {
                                alu(param, M.ptr(i*w + j), &tmp, eMOV);
                                D[ i*w + j ] = x;
                            }
                        }
                    }
                }
                reconstruct_partition(D, w, n, k_ways, res);
            }
        };
}

#endif
