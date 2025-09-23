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

#define TOL 2.0e-4

idx ncom;
real *pcom,*xicom,(*nrfunc)(real [], void * );

void sMathNR::linmin(real p[], real xi[], idx n, real *fret, callBackOptim func, void * usr)
{
    idx j;
    real xx,xmin,fx,fb,fa,bx,ax;

    ncom=n;
    pcom=sMathNRUtil::vector(1,n);
    xicom=sMathNRUtil::vector(1,n);
    nrfunc=func;
    for (j=1;j<=n;j++) {
        pcom[j]=p[j];
        xicom[j]=xi[j];
    }
    ax=0.0;
    xx=1.0;
    mnbrak(&ax,&xx,&bx,&fa,&fx,&fb,&sMathNR::f1dim,usr);
    *fret=brent(ax,xx,bx,&sMathNR::f1dim,TOL,&xmin,usr);
    for (j=1;j<=n;j++) {
        xi[j] *= xmin;
        p[j] += xi[j];
    }
    sMathNRUtil::free_vector(xicom,1,n);
    sMathNRUtil::free_vector(pcom,1,n);
}
#undef TOL
#undef NRANSI
