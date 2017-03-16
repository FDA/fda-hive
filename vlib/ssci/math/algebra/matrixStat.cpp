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
#include <math.h>

using namespace slib;

#define val(val1,val2)    (*( (real  *)src+cols*(val1)+(val2) ))
#define cov(val1,val2)    (*( (real  *)cvr+cols*(val1)+(val2) ))

void sAlgebra::matrix::computeRowStat(const real * src, idx rows, idx cols, real * aves,real * stddev, real * minp, real * maxp)
{
    idx ic,ir;
    for(ic=0; ic<cols; ++ic) {
        aves[ic]=0;
        if(stddev)stddev[ic]=0;
        if(minp)minp[ic]=REAL_MAX;
        if(maxp)maxp[ic]=-REAL_MAX;
    }

            
    for(ir=0; ir<rows; ++ir) {
        for(ic=0; ic<cols; ++ic) {
            real v=val(ir,ic);
            aves[ic]+=v;
            if(minp && minp[ic]>v)minp[ic]=v;
            if(maxp && maxp[ic]<v)maxp[ic]=v;
        }
    }
    for(ic=0; ic<cols; ++ic) aves[ic]/=rows;
    if(!stddev) return;

    for(ir=0; ir<rows; ++ir) {
        for(ic=0; ic<cols; ++ic) {
            real dev=val(ir,ic)-aves[ic];
            stddev[ic]+=dev*dev;
        }
    }
    for(ic=0; ic<cols; ++ic) stddev[ic]=sqrt(stddev[ic]/(rows-1));
}



void sAlgebra::matrix::covariance(const real * src, idx rows, idx cols, real * cvr , real scl)
{
    idx ic2,ir, ic1;
    real coef;

    if(!scl)scl=(real)(rows-1);

    for( ic1=0; ic1<cols; ++ic1) {
        for( ic2=ic1; ic2<cols; ++ic2) {
            for(coef=0,ir=0; ir<rows; ++ir)coef+=val(ir,ic1)*val(ir,ic2);  // compute the covariance matrix 
            coef/=scl;
            cov(ic1,ic2)=coef;
            if(ic1!=ic2)cov(ic2,ic1)=coef;
        }
    }
}


#undef val
