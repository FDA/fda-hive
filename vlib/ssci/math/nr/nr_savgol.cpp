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



void sMathNR::SavGol::computeFilterCoef(real c[], idx np, idx nl, idx nr, idx ld, idx m)
{
    idx imj,ipj,j,k,kk,mm,*indx;
    real d,fac,sum,**a,*b;

    if (np < nl+nr+1 || nl < 0 || nr < 0 || ld > m || nl+nr < m)
    sMathNRUtil::nrerror("bad args in savgol");
    indx=sMathNRUtil::ivector(1,m+1);
    a=sMathNRUtil::matrix(1,m+1,1,m+1);
    b=sMathNRUtil::vector(1,m+1);
    for (ipj=0;ipj<=(m << 1);ipj++) {
        sum=(ipj ? 0.0 : 1.0);
        for (k=1;k<=nr;k++) sum += pow((double)k,(double)ipj);
        for (k=1;k<=nl;k++) sum += pow((double)-k,(double)ipj);
        mm=IMIN(ipj,2*m-ipj);
        for (imj = -mm;imj<=mm;imj+=2) a[1+(ipj+imj)/2][1+(ipj-imj)/2]=sum;
    }
    ludcmp(a,m+1,indx,&d);
    for (j=1;j<=m+1;j++) b[j]=0.0;
    b[ld+1]=1.0;
    lubksb(a,m+1,indx,b);
    for (kk=1;kk<=np;kk++) c[kk]=0.0;
    for (k = -nl;k<=nr;k++) {
        sum=b[1];
        fac=1.0;
        for (mm=1;mm<=m;mm++) sum += b[mm+1]*(fac *= k);
        kk=((np-k) % np)+1;
        c[kk]=sum;
    }
    sMathNRUtil::free_vector(b,1,m+1);
    sMathNRUtil::free_matrix(a,1,m+1,1,m+1);
    sMathNRUtil::free_ivector(indx,1,m+1);
}




sMathNR::SavGol::SavGol(idx ldatadim, idx lleft, idx lright, idx ldegree)
{
    left=lleft, right=lright, degree=ldegree,np=right+left+1, datadim=ldatadim; 
    cSavGol=sMathNRUtil::vector(1,datadim);
    sSet(cSavGol,0,sizeof(real)*(datadim+1));
    computeFilterCoef(cSavGol,np,left,right,0,degree);
}

sMathNR::SavGol::~SavGol()
{ 
    sMathNRUtil::free_vector(cSavGol,1,datadim);
}

                
void sMathNR::SavGol::smooth( real * dst, real * src ) 
{
    sSet(dst,0,datadim);
    idx ii;

    for(ii=0; ii<left; ++ii)dst[ii]=src[ii];

    for(; ii<datadim-right; ++ii) {
        dst[ii]=cSavGol[0+1]*src[ii];
        for(idx il=0; il<left;++il)
            dst[ii]+=cSavGol[il+1+1]*src[ii-il-1];
        for(idx ir=0; ir<right;++ir)
            dst[ii]+=cSavGol[np-ir-1+1]*src[ii+ir+1];
    }
    for(; ii<datadim; ++ii)dst[ii]=src[ii];
}


