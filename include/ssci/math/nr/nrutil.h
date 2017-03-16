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
#ifndef sMathNr_nrutil_h
#define sMathNr_nrutil_h

#include "nr.hpp"
using namespace slib;

#define IMIN    sMin
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
//extern real maxarg1,maxarg2;
//#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))
//extern real sqrarg;
//#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

namespace slib
{
    class sMathNRUtil {
                public:
            static char lastErr[4096];
            static void nrerror(const char * error_text);

            static real* vector(idx nl, idx nh);
            static void free_vector(real *v, idx nl, idx nh);
            static idx * ivector(idx nl, idx nh);
            static void free_ivector(idx *v, idx nl, idx nh);
            static real ** matrix(idx nrl, idx nrh, idx ncl, idx nch); 
            static void free_matrix(real **m, idx nrl, idx nrh, idx ncl, idx nch);

        };
}
#endif // sMath_nr_nrutil_h




