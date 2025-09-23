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


void sAlgebra::matrix::pcaReMap(real * dst, const real * orig, idx cols, idx rows, const real * evecs)
{
    idx ic2,ir, ic1;
    real coef;

    for( ic1=0; ic1<cols; ++ic1) {
        for(ir=0 ; ir<rows ; ++ir ) {
            coef=0;
            for( ic2=0; ic2<cols; ++ic2) {
                coef+=evecs[ic2*cols+ic1]*orig[ir*cols+ic2];
            }
            dst[ir*cols+ic1]=coef;
        }
    }

}

void sAlgebra::matrix::pca(real * actmat, idx rows, idx cols, real * evals, real * evecs )
{

    
    
    idx totsize = cols*cols
        + rows * cols
        + cols*2
        ;


    real * covar = (real * )sNew(totsize*sizeof(real));
    real * tmparr = covar+(cols*cols);
    real * B = tmparr+(rows*cols);
    
    computeRowStat(actmat, rows, cols, B,0);
    shiftRows(actmat,rows, cols, B ); 
    memcpy(tmparr,actmat,sizeof(real)*cols*rows);
    covariance(tmparr,rows, cols, covar);
    
    diagJacoby(cols,covar,evals,evecs,B,1e-16,1000);
    diagSort(cols,evals,evecs,-1, false);
        
    pcaReMap(actmat,tmparr, cols, rows, evecs);

    sDel(covar);
}



