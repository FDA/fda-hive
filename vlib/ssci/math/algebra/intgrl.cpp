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
#include <ssci/math/algebra/algebra.hpp>

using namespace slib;

real sAlgebra::integral::calcUnispace(idx method, const real *y, idx n, real h)
{
    static real methodList[4][10]={
        {1./2,  0}, // trapezoid
        {5./12, 13./12, 0}, // cubic
        {3./8,  7./6,   23./24,  0}, // N4
        {1./3,  0}, // simpson // 4./3,   2./3 alternation
    };
    real * met=methodList[method];

    idx i, ord;
    real itgrl=0;

    // we do the left tail first
    for( i=0, ord=0; i<n && met[ord]!=0 ; ++i, ++ord ) 
        itgrl+=y[i]*met[ord];
    
    if(method==eSympson) {
        real coef=4./3;
        for( ; i<n-ord; ++i) {
            itgrl+=y[i]*coef; 
            coef=2-coef;
        } 
    }
 
    else for( ; i<n-ord; ++i) 
        itgrl+=y[i];
    
    for( --ord ; i<n; ++i, --ord) 
        itgrl+=y[i]*met[ord];

    return itgrl*h;

}


