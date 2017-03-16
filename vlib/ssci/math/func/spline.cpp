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

using namespace slib;

//Given arrays x[1..n] and y[1..n] containing a tabulated function, i.e., yi = f(xi), with
//x1 <x2 < :: : < xN, and given values yp1 and ypn for the first derivative of the interpolating
//function at points 1 and n, respectively, this routine returns an array y2[1..n] that contains
//the second derivatives of the interpolating function at the tabulated points xi. If yp1 and/or
//ypn are equal to 1-1030 or larger, the routine is signaled to set the corresponding boundary-
//condition for a natural spline, with zero second derivative on that boundary.

void sFunc::spline::secDeriv(real *x, real *y, idx n, real yp1, real ypn, real * y2, real * tmp)
{
    idx i,k;
    real p,qn,sig,un;

    //tmp=sMathNRUtil::vector(1,n-1);
    if (yp1 >= REAL_MAX) //The lower boundary condition is set either to be "natural"
        y2[1-1]=tmp[1-1]=0.0; 
    else { // or else to have a specified first derivative.
        y2[1-1] = -0.5;
        tmp[1-1]=(3.0/(x[2-1]-x[1-1]))*((y[2-1]-y[1-1])/(x[2-1]-x[1-1])-yp1);
    }

    for (i=2-1;i<n-1;i++) { // This is the decomposition loop of the tridiagonal algorithm.
        // y2 and tmp are used for temporary storage of the decomposed factors.
        sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
        p=sig*y2[i-1]+2.0;
        y2[i]=(sig-1.0)/p;
        tmp[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
        tmp[i]=(6.0*tmp[i]/(x[i+1]-x[i-1])-sig*tmp[i-1])/p;
    }
    
    if (ypn >= REAL_MAX) // The upper boundary condition is set either to be
        qn=un=0.0; //"natural" or else to have a specified first derivative.
    else { 
        qn=0.5;
        un=(3.0/(x[n-1]-x[n-1-1]))*(ypn-(y[n-1]-y[n-1-1])/(x[n-1]-x[n-1-1]));
    }
    y2[n-1]=(un-qn*tmp[n-1-1])/(qn*y2[n-1-1]+1.0);
    for (k=n-1-1;k>=1-1;k--) //This is the backsubstitution loop of the tridiagonal
        y2[k]=y2[k]*y2[k+1]+tmp[k]; // algorithm.
//    free_vector(tmp,1,n-1);
}

//Given the arrays xa[1..n] and ya[1..n], which tabulate a function (with the xai's in order),
//and given the array y2a[1..n], which is the output from spline above, and given a value of
//x, this routine returns a cubic-spline interpolated value y.
real sFunc::spline::calc(real *xa, real *ya, real *y2a, idx n, real x, real *y)
{
    
    idx klo,khi,k;
    real h,b,a;
    klo=1-1; // We will find the right place in the table by means of bisection. 
    // This is optimal if sequential calls to this
    // routine are at random values of x. If sequential calls
    // are in order, and closely spaced, one would do better
    // to store previous values of klo and khi and test if
    // they remain appropriate on the next call.
    khi=n-1;
    while (khi-klo > 1) {
        k=(khi+klo) >> 1;
        if (xa[k] > x) khi=k;
        else klo=k;
    } // klo and khi now bracket the input value of x.
    h=xa[khi]-xa[klo];
    // if (h == 0.0) sMathNRUtil::nrerror("Bad xa input to routine splint"); //The xa's must be distinct.
    a=(xa[khi]-x)/h; 
    b=(x-xa[klo])/h; // Cubic spline polynomial is now evaluated.
    double yval=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
    if(y)*y=yval;
    return yval;
}



