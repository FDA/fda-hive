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

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

real sMathNR::select(udx k, udx n, real arr[])
{
        udx i,ir,j,l,mid;
        real a,temp;

        l=1;
        ir=n;
        for (;;) {
                if (ir <= l+1) {
                        if (ir == l+1 && arr[ir] < arr[l]) {
                                SWAP(arr[l],arr[ir])
                        }
                        return arr[k];
                } else {
                        mid=(l+ir) >> 1;
                        SWAP(arr[mid],arr[l+1])
                        if (arr[l] > arr[ir]) {
                                SWAP(arr[l],arr[ir])
                        }
                        if (arr[l+1] > arr[ir]) {
                                SWAP(arr[l+1],arr[ir])
                        }
                        if (arr[l] > arr[l+1]) {
                                SWAP(arr[l],arr[l+1])
                        }
                        i=l+1;
                        j=ir;
                        a=arr[l+1];
                        for (;;) {
                                do i++; while (arr[i] < a);
                                do j--; while (arr[j] > a);
                                if (j < i) break;
                                SWAP(arr[i],arr[j])
                        }
                        arr[l+1]=arr[j];
                        arr[j]=a;
                        if (j >= k) ir=j-1;
                        if (j <= k) l=i;
                }
        }
}
#undef SWAP
