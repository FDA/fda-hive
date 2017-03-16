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

#define a(val1,val2)    (*( (real *)arr+n*(val1)-n+(val2)-1 ))
#define v(val1,val2)    (*( (real *)evecs+n*(val1)-n+(val2)-1 ))
#define ROTATE(_a,i,j,k,l) g=_a(i,j);h=_a(k,l);_a(i,j)=g-s*(h+g*tau);_a(k,l)=h+s*(g-h*tau);

idx sMathNR::jacobi(real *arr, idx n, real * d, real * evecs, real jactol, idx maxiter)
{
        idx rot;
        idx j,iq,ip,i;
        real tresh,theta,tau,t,sm,s,h,g,c,*b,*z;

        b=sMathNRUtil::vector(1,n);
        z=sMathNRUtil::vector(1,n);
        for (ip=1;ip<=n;ip++) {
                for (iq=1;iq<=n;iq++) v(ip,iq)=0.0;
                v(ip,ip)=1.0;
        }
        for (ip=1;ip<=n;ip++) {
                b[ip]=d[ip]=a(ip,ip);
                z[ip]=0.0;
        }
        rot=0;
        for (i=1;i<=maxiter;i++) {
                sm=0.0;
                for (ip=1;ip<=n-1;ip++) {
                        for (iq=ip+1;iq<=n;iq++)
                                sm += fabs(a(ip,iq));
                }
                
                if(sm<=jactol){
                //if (sm == 0.0) {
                        sMathNRUtil::free_vector(z,1,n);
                        sMathNRUtil::free_vector(b,1,n);
                        return rot;
                }
                if (i < 4)
                        tresh=0.2*sm/(n*n);
                else
                        tresh=0.0;
                for (ip=1;ip<=n-1;ip++) {
                        for (iq=ip+1;iq<=n;iq++) {
                                g=100.0*fabs(a(ip,iq));
                                if (i > 4 && (float)(fabs(d[ip])+g) == (float)fabs(d[ip])
                                        && (float)(fabs(d[iq])+g) == (float)fabs(d[iq]))
                                        a(ip,iq)=0.0;
                                else if (fabs(a(ip,iq)) > tresh) {
                                        h=d[iq]-d[ip];
                                        if ((float)(fabs(h)+g) == (float)fabs(h))
                                                t=(a(ip,iq))/h;
                                        else {
                                                theta=0.5*h/(a(ip,iq));
                                                t=1.0/(fabs(theta)+sqrt(1.0+theta*theta));
                                                if (theta < 0.0) t = -t;
                                        }
                                        c=1.0/sqrt(1+t*t);
                                        s=t*c;
                                        tau=s/(1.0+c);
                                        h=t*a(ip,iq);
                                        z[ip] -= h;
                                        z[iq] += h;
                                        d[ip] -= h;
                                        d[iq] += h;
                                        a(ip,iq)=0.0;
                                        for (j=1;j<=ip-1;j++) {
                                                ROTATE(a,j,ip,j,iq)
                                        }
                                        for (j=ip+1;j<=iq-1;j++) {
                                                ROTATE(a,ip,j,j,iq)
                                        }
                                        for (j=iq+1;j<=n;j++) {
                                                ROTATE(a,ip,j,iq,j)
                                        }
                                        for (j=1;j<=n;j++) {
                                                ROTATE(v,j,ip,j,iq)
                                        }
                                        ++rot;
                                }
                        }
                }
                for (ip=1;ip<=n;ip++) {
                        b[ip] += z[ip];
                        d[ip]=b[ip];
                        z[ip]=0.0;
                }
        }
        //nrerror("Too many iterations in routine jacobi");
        return rot;
}
#undef ROTATE
#undef v
#undef a
