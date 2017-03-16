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

//#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

void sMathNR::convlv(real data[], udx n, real respns[], udx m, idx isign, real ans[])
{
  //  real sqrarg;
    //void realft(real data[], unsigned idx n, idx isign);
        //void twofft(real data1[], real data2[], real fft1[], real fft2[],unsigned idx n);
        udx i,no2;
        real dum,mag2,*fft;

        fft=sMathNRUtil::vector(1,n<<1);
        for (i=1;i<=(m-1)/2;i++)
                respns[n+1-i]=respns[m+1-i];
        for (i=(m+3)/2;i<=n-(m-1)/2;i++)
                respns[i]=0.0;
        twofft(data,respns,fft,ans,n);
        no2=n>>1;
        for (i=2;i<=n+2;i+=2) {
                if (isign == 1) {
                        ans[i-1]=(fft[i-1]*(dum=ans[i-1])-fft[i]*ans[i])/no2;
                        ans[i]=(fft[i]*dum+fft[i-1]*ans[i])/no2;
                } else if (isign == -1) {
                        if ((mag2=sSqr(ans[i-1])+sSqr(ans[i])) == 0.0)
                                sMathNRUtil::nrerror("Deconvolving at response zero in convlv");
                        ans[i-1]=(fft[i-1]*(dum=ans[i-1])+fft[i]*ans[i])/mag2/no2;
                        ans[i]=(fft[i]*dum-fft[i-1]*ans[i])/mag2/no2;
                } else sMathNRUtil::nrerror("No meaning for isign in convlv");
        }
        ans[2]=ans[n+1];
        realft(ans,n,-1);
        sMathNRUtil::free_vector(fft,1,n<<1);
}
