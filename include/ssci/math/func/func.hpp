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
#ifndef sMath_func_hpp
#define sMath_func_hpp

#include <slib/core/def.hpp>
#include <slib/core/vec.hpp>


namespace slib {
    class sFunc
    { // lower level functions
        public:
            class special
            {
                public:
                    static real lnGamma(real xx);
                    static real betaContinuedFraction(real a, real b, real x);
                    static real betaIncomplete(real a, real b, real x);

                    static real gser(real a, real x); // Returns the incomplete gamma function P(a; x) evaluated by its series representation as gamser.
                    static real gcf(real a, real x); // Returns the incomplete gamma function Q(a; x) evaluated by its continued fraction representation as gammcf.
                    static real gammap(real a, real x);
                    static real errf(real x); // Returns the error function erf(x).
                    static real errfinv(real p);

            };

            class spline
            {
                public:
                    static real calc(real *xa, real *ya, real *y2a, idx n, real x, real *y);
                    static void secDeriv(real *x, real *y, idx n, real yp1, real ypn, real * y2, real * tmp);
            };

            class sSearch
            {
                public:
                    template <class Tobj> static void binarysearch(Tobj * array, idx num, Tobj target, bool returnIndexbeforeTarget = false){
                        idx lo = 0, mid = 0, hi = num - 1;
                        Tobj aux;
                        while( lo <= hi ) {
                            mid = lo + (hi - lo) / 2;
                            aux = array[mid];
                            if( aux == target ) {
                                break;
                            } else if( aux < target ) {
                                lo = mid + 1;
                            } else {
                                hi = mid - 1;
                            }

                        }
                        while( returnIndexbeforeTarget && (mid > 0) && (array[mid] >= target) ) {
                            mid -= 1;
                        }
                    }
            };
    };

}

#endif // sMath_func_hpp




