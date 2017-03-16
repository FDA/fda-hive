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
#include <math.h>

void sMathNR::fftCutoff(real * src, real * dst , idx cnt, idx daubNum, idx cutMin, idx cutMax)
{
    if(daubNum)sMathNR::pwtset(daubNum);
    
    idx k,is;
    
    // determine a size which is bigger than cnt and is degree of 2
    for(k=1; k<cnt; k<<=1) ;
    real * coef=(real *)sNew(2*(k+1)*sizeof(real));
    memset(coef,0,2*(k+1)*sizeof(real));
    memcpy((void *)(coef+1),src,sizeof(real)*cnt); // make a copy of the array  shifted by one beacuse of NR 

    // perform wavelett analysis 
    if(daubNum)sMathNR::wt1(coef,k,1,sMathNR::pwt);
    else sMathNR::realft(coef,k,1);
    
    // now find the portion of the coefficients we do nullify
    for(is=0; is<cutMin; ++is) coef[is] = 0;
    for(is=cutMax; is<k; ++is) coef[is] = 0;

    if(daubNum)sMathNR::wt1(coef,k,-1,sMathNR::pwt);
    else { 
        sMathNR::realft(coef,k,-1);
        for(is=1; is<cnt+1; is++) coef[is]*=2./k;
    }

    memcpy(dst,(void*)(coef+1),sizeof(real)*cnt);

    sDel(coef);
}
