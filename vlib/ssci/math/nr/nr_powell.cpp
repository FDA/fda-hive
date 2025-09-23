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

#define TINY 1.0e-25
#define XI(_v_i,_v_j, _v_n)   xi2d[(_v_i)*(_v_n+1)+(_v_j)]

idx sMathNR::powell(idx maxiter, real p[], idx n, real ftol, real *fret,callBackOptim func, void * usr)
{
    idx iter=0;

    real * xi2d=sMathNRUtil::vector(1,(n+1)*(n+1));
    for(idx ii=0;ii<=n;++ii) {
        for(idx jj=0;jj<=n;++jj) {
            XI(ii,jj,n)= (ii==jj) ? 1 : 0;
        }
    }


    idx i,ibig,j;
    real del,fp,fptt,t,*pt,*ptt,*xit;

    pt=sMathNRUtil::vector(1,n);
    ptt=sMathNRUtil::vector(1,n);
    xit=sMathNRUtil::vector(1,n);
    *fret=(*func)(p,usr);
    for (j=1;j<=n;j++) pt[j]=p[j];
    for (iter=1;;++(iter)) {
        fp=(*fret);
        ibig=0;
        del=0.0;
        for (i=1;i<=n;i++) {
            for (j=1;j<=n;j++) xit[j]=XI(j,i,n);
            fptt=(*fret);
            linmin(p,xit,n,fret,func, usr);
            if (fptt-(*fret) > del) {
                del=fptt-(*fret);
                ibig=i;
            }
        }
        if (2.0*(fp-(*fret)) <= ftol*(fabs(fp)+fabs(*fret))+TINY) {
            sMathNRUtil::free_vector(xit,1,n);
            sMathNRUtil::free_vector(ptt,1,n);
            sMathNRUtil::free_vector(pt,1,n);
            sMathNRUtil::free_vector(xi2d,1,(n+1)*(n+1));
            return iter;
        }
        if (iter == maxiter) sMathNRUtil::nrerror("powell exceeding maximum iterations.");
        for (j=1;j<=n;j++) {
            ptt[j]=2.0*p[j]-pt[j];
            xit[j]=p[j]-pt[j];
            pt[j]=p[j];
        }
        fptt=(*func)(ptt,usr);
        if (fptt < fp) {
            t=2.0*(fp-2.0*(*fret)+fptt)*sSqr(fp-(*fret)-del)-del*sSqr(fp-fptt);
            if (t < 0.0) {
                linmin(p,xit,n,fret,func, usr);
                for (j=1;j<=n;j++) {
                    XI(j,ibig,n)=XI(j,n,n);
                    XI(j,n,n)=xit[j];
                }
            }
        }
    }
}
