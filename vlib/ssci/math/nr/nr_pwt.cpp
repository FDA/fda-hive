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

void sMathNR::pwt(real a[], udx n, idx isign)
{
        real ai,ai1,*wksp;
        udx i,ii,j,jf,jr,k,n1,ni,nj,nh,nmod;

        if (n < 4) return;
        wksp=sMathNRUtil::vector(1,n);
        nmod=wfilt.ncof*n;
        n1=n-1;
        nh=n >> 1;
        for (j=1;j<=n;j++) wksp[j]=0.0;
        if (isign >= 0) {
                for (ii=1,i=1;i<=n;i+=2,ii++) {
                        ni=i+nmod+wfilt.ioff;
                        nj=i+nmod+wfilt.joff;
                        for (k=1;k<=wfilt.ncof;k++) {
                                jf=n1 & (ni+k);
                                jr=n1 & (nj+k);
                                wksp[ii] += wfilt.cc[k]*a[jf+1];
                                wksp[ii+nh] += wfilt.cr[k]*a[jr+1];
                        }
                }
        } else {
                for (ii=1,i=1;i<=n;i+=2,ii++) {
                        ai=a[ii];
                        ai1=a[ii+nh];
                        ni=i+nmod+wfilt.ioff;
                        nj=i+nmod+wfilt.joff;
                        for (k=1;k<=wfilt.ncof;k++) {
                                jf=(n1 & (ni+k))+1;
                                jr=(n1 & (nj+k))+1;
                                wksp[jf] += wfilt.cc[k]*ai;
                                wksp[jr] += wfilt.cr[k]*ai1;
                        }
                }
        }
        for (j=1;j<=n;j++) a[j]=wksp[j];
        sMathNRUtil::free_vector(wksp,1,n);
}


sMathNR::wavefilt sMathNR::wfilt;

void sMathNR::pwtset(idx n)
{
    static idx prvN=-1;
    if(prvN==n)return;

        idx k;
        real sig = -1.0;
        static real c4[5]={0.0,0.4829629131445341,0.8365163037378079,
                        0.2241438680420134,-0.1294095225512604};
        static real c12[13]={0.0,0.111540743350, 0.494623890398, 0.751133908021,
                0.315250351709,-0.226264693965,-0.129766867567,
                0.097501605587, 0.027522865530,-0.031582039318,
                0.000553842201, 0.004777257511,-0.001077301085};
        static real c20[21]={0.0,0.026670057901, 0.188176800078, 0.527201188932,
                0.688459039454, 0.281172343661,-0.249846424327,
                -0.195946274377, 0.127369340336, 0.093057364604,
                -0.071394147166,-0.029457536822, 0.033212674059,
                0.003606553567,-0.010733175483, 0.001395351747,
                0.001992405295,-0.000685856695,-0.000116466855,
                0.000093588670,-0.000013264203};
        static real c4r[5],c12r[13],c20r[21];

        wfilt.ncof=n;
        if (n == 4) {
                wfilt.cc=c4;
                wfilt.cr=c4r;
        }
        else if (n == 12) {
                wfilt.cc=c12;
                wfilt.cr=c12r;
        }
        else if (n == 20) {
                wfilt.cc=c20;
                wfilt.cr=c20r;
        }
        else sMathNRUtil::nrerror("unimplemented value n in pwtset");
        for (k=1;k<=n;k++) {
                wfilt.cr[wfilt.ncof+1-k]=sig*wfilt.cc[k];
                sig = -sig;
        }
        wfilt.ioff = wfilt.joff = -(n >> 1);
        prvN=n;
}
