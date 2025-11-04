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
#include <ssci/math/nr/nrutil.h>
#include <ssci/math/constants.hpp>
#include <math.h>

real sMathNR::poissonln(real lambda, real x )
{
    return -lambda+x*log(lambda)-sMathNR::gammln(x+1);
}
real sMathNR::poisson(real lambda, real x)
{
    return exp(poissonln(lambda,x));
}
real sMathNR::poissoncumul(real lambda, idx xstart, idx xend )
{
    real p=0;
    for(idx x=xstart; x<=xend; ++x)
        p+=x*poisson(lambda,x);
    return p;
}


real sMathNR::binome_Stirling(idx N, real freq, idx k)
{
    if(k==0)return 0;
    real b = pow((N * freq / k), k) * pow((1. - freq) * N / (N - k), (N - k)) * sqrt( N/ (2. * sConstants::PI * k * (N - k)));
    return b;
}

real sMathNR::poissonCells_binomeHits_momentum(idx cntCells, real propAlteredCells, idx cntMaxCells, real lambdaHits, idx startCntHits, idx endCntHits )
{

    real res=0.;
    real pb=sMathNR::poissoncumul(lambdaHits, startCntHits, endCntHits);
    for( idx k=1; k<=cntMaxCells; ++k) {
        real Bs=sMathNR::binome_Stirling(cntCells, propAlteredCells, k);
        res+=pb*Bs*k;
    }
    return res;
}
