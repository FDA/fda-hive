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
#include <ssci/math/func/func.hpp>
#include <math.h>

using namespace slib;



real sFunc::special::lnGamma(real xx)
{
    double x,y,tmp,ser;
    static double cof[6]={
        76.18009172947146,-86.50532032941677,
        24.01409824083091,-1.231739572450155,
        0.1208650973866179e-2,-0.5395239384953e-5
    };

    y=x=xx;
    tmp=x+5.5;
    tmp -= (x+0.5)*log(tmp);
    ser=1.000000000190015;
    for (idx j=0;j<=5;j++) 
        ser += cof[j]/++y;
    return -tmp+log(2.5066282746310005*ser/x);
}

#define MAXIT 1000
#define EPS 3.0e-13
#define FPMIN REAL_MIN
real sFunc::special::betaContinuedFraction(real a, real b, real x)
{
    idx m,m2;
    real aa,c,d,del,h,qab,qam,qap;

    qab=a+b;
    qap=a+1.0;
    qam=a-1.0;
    c=1.0;
    d=1.0-qab*x/qap;
    if (fabs(d) < FPMIN) d=FPMIN;
    d=1.0/d;
    h=d;
    for (m=1;m<=MAXIT;m++) {
        m2=2*m;
        aa=m*(b-m)*x/((qam+m2)*(a+m2));
        d=1.0+aa*d;
        if (fabs(d) < FPMIN) d=FPMIN;
        c=1.0+aa/c;
        if (fabs(c) < FPMIN) c=FPMIN;
        d=1.0/d;
        h *= d*c;
        aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
        d=1.0+aa*d;
        if (fabs(d) < FPMIN) d=FPMIN;
        c=1.0+aa/c;
        if (fabs(c) < FPMIN) c=FPMIN;
        d=1.0/d;
        del=d*c;
        h *= del;
        if (fabs(del-1.0) < EPS) break;
    }
    
    return h;
}

real sFunc::special::betaIncomplete(real a, real b, real x)
{
    real bt;
    if (x == 0.0 || x == 1.0) 
        bt=0.0;
    else
        bt=exp(lnGamma(a+b)-lnGamma(a)-lnGamma(b)+a*log(x)+b*log(1.0-x));
    if (x < (a+1.0)/(a+b+2.0))
        return bt*betaContinuedFraction(a,b,x)/a;
    else
        return 1.0-bt*betaContinuedFraction(b,a,1.0-x)/b;
}

























#define ITMAX 100

real sFunc::special::gser(real a, real x)
{
    int n;
    if (x <= 0.0)
        return 0.0 ;
    else {
        real ap=a;
        real sum=1.0/a,del=sum;
        for (n=1;n<=ITMAX;n++) {
            ++ap;
            del *= x/ap;
            sum += del;
            if (fabs(del) < fabs(sum)*EPS) {
                return sum*exp(-x+a*log(x)-lnGamma(a));
            }
        }
        return 0;
    }
}

real sFunc::special::gcf(real a, real x)
{
    int i;
    real an,del;
    real b=x+1.0-a;
    real c=1.0/FPMIN;
    real d=1.0/b;
    real h=d;
    for (i=1;i<=ITMAX;i++) {
        an = -i*(i-a);
        b += 2.0;
        d=an*d+b;
        if (fabs(d) < FPMIN) d=FPMIN;
        c=b+an/c;
        if (fabs(c) < FPMIN) c=FPMIN;
        d=1.0/d;
        del=d*c;
        h *= del;
        if (fabs(del-1.0) < EPS) break;
    }

    return exp(-x+a*log(x)-lnGamma(a))*h;
}

real sFunc::special::gammap(real a, real x)
{
    if (x < (a+1.0))
        return gser(a,x);
    else
        return 1.0-gcf(a,x); 
    
}
 
real sFunc::special::errf(real x)
{
    return x < 0.0 ? -gammap(0.5,x*x) : gammap(0.5,x*x);
}




static const real a[] =
{
    -3.969683028665376e+01,
     2.209460984245205e+02,
    -2.759285104469687e+02,
     1.383577518672690e+02,
    -3.066479806614716e+01,
     2.506628277459239e+00
};

static const real b[] =
{
    -5.447609879822406e+01,
     1.615858368580409e+02,
    -1.556989798598866e+02,
     6.680131188771972e+01,
    -1.328068155288572e+01
};

static const real c[] =
{
    -7.784894002430293e-03,
    -3.223964580411365e-01,
    -2.400758277161838e+00,
    -2.549732539343734e+00,
     4.374664141464968e+00,
     2.938163982698783e+00
};

static const real d[] =
{
    7.784695709041462e-03,
    3.224671290700398e-01,
    2.445134137142996e+00,
    3.754408661907416e+00
};

#define LOW 0.02425
#define HIGH 0.97575

real sFunc::special::errfinv(real p)
{
    real q, r;


    if (p < 0 || p > 1)
    {
        return 0.0;
    }
    else if (p == 0)
    {
        return -HUGE_VAL;
    }
    else if (p == 1)
    {
        return HUGE_VAL;
    }
    else if (p < LOW)
    {
        q = sqrt(-2*log(p));
        return (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
            ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    }
    else if (p > HIGH)
    {
        q  = sqrt(-2*log(1-p));
        return -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
            ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    }
    else
    {
            q = p - 0.5;
            r = q*q;
        return (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
            (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1);
    }
}



