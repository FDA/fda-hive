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
#include <ssci/math/nr/nr.hpp>
#include <ssci/math/algebra/algebra.hpp>
#include <math.h>

using namespace slib;

#define a(val1,val2)    (*( (real *)arr+n*(val1)-n+(val2)-1 ))
#define v(val1,val2)    (*( (real *)evecs+n*(val1)-n+(val2)-1 ))
#define z(val)  (*( (real *)Z+(val)-1 ))
#define d(val)  (*( (real *)evals+(val)-1 ))
#define b(val)  (*( (real *)B+(val)-1 ))


idx sAlgebra::matrix::diagJacoby(idx n ,real * arr,real   * evals,real * evecs,real * B,real jactol,idx  maxiter)
{
    //return sMathNR::jacobi(arr, n, evecs, evals,jactol, maxiter);
    idx p,q,i,j,rot;
    real    sm,c,s,t,h,g,tau,theta,tresh;
    real * Z=B+n;


    if(evecs){
        for(p=1;p<=n;p++){
            for(q=1;q<=n;q++){
                v(p,q) =((p==q) ? 1 : 0);
            }
        }
    }
    for(p=1;p<=n;p++){
        b(p)=a(p,p);z(p)=0;
        d(p)=a(p,p);
    }
    rot=0;

    for(i=1;i<maxiter;i++){
//        swp:;
        sm=0;
        for(p=1;p<=n-1;p++){
            for(q=p+1;q<=n;q++){
                sm+=fabs(a(p,q));
            }
        }
        if(sm<=jactol)goto out;
        tresh=(i<4) ? 0.2*sm/(n*n) : 0.0;
        for(p=1;p<=n-1;p++){
            for(q=p+1;q<=n;q++){
                g=fabs(a(p,q));
                if((i>4) && fabs(d(p))+g==fabs(d(p)) && fabs(d(q))+g==fabs(d(q)))
                    a(p,q)=0;
                else if(fabs(a(p,q))>tresh){ /* need rotation */
//                    rotate:
                    h=d(q)-d(p);
                    if(fabs(h)+g==fabs(h)){
                        t=a(p,q)/h;
                    }
                    else {
                        theta=0.5*h/(a(p,q));
                        t=1/(fabs(theta)+sqrt(1+theta*theta));
                        if(theta<0)t=-t;
                    }
                    c=1/sqrt(1+t*t);
                    s=t*c;
                    tau=s/(1+c);
                    h=t*a(p,q);
                    z(p)-=h;z(q)+=h;
                    d(p)-=h;d(q)+=h;
                    a(p,q)=0;
                    for(j=1;j<=p-1;j++){
                        g=a(j,p);h=a(j,q);
                        a(j,p)=g-s*(h+g*tau);
                        a(j,q)=h+s*(g-h*tau);
                    }
                    for(j=p+1;j<=q-1;j++){
                        g=a(p,j);h=a(j,q);
                        a(p,j)=g-s*(h+g*tau);
                        a(j,q)=h+s*(g-h*tau);
                    }
                    for(j=q+1;j<=n;j++){
                        g=a(p,j);h=a(q,j);
                        a(p,j)=g-s*(h+g*tau);
                        a(q,j)=h+s*(g-h*tau);
                    }
                    if(evecs){
                        for(j=1;j<=n;j++){
                            g=v(j,p);h=v(j,q);
                            v(j,p)=g-s*(h+g*tau);
                            v(j,q)=h+s*(g-h*tau);
                        }
                    } /* end if eivec */
                    rot++;
                } /* end if need rotation */
            }       /* end q */
        }       /* end p */
        for(p=1;p<=n;p++){
            d(p)=b(p)=b(p)+z(p);
            z(p)=0;
        }       /* end p */
    }       /* end Iter */
    out:
    return rot;
}

#undef  a
#undef  v
#undef  z
#undef  d
#undef  b






