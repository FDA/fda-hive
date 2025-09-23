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

#define GOLD 1.618034
#define GLIMIT 100.0
#define TINY (real)(1.0e-20)
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);


void sMathNR::mnbrak(real *ax, real *bx, real *cx, real *fa, real *fb, real *fc,    real (*func)(real, void * ), void * usr)
{
    real ulim,u,r,q,fu,dum;

    *fa=(*func)(*ax,usr);
    *fb=(*func)(*bx,usr);
    if (*fb > *fa) {
        SHFT(dum,*ax,*bx,dum)
        SHFT(dum,*fb,*fa,dum)
    }
    *cx=(*bx)+GOLD*(*bx-*ax);
    *fc=(*func)(*cx,usr);
    while (*fb > *fc) {
        r=(*bx-*ax)*(*fb-*fc);
        q=(*bx-*cx)*(*fb-*fa);
        u=(*bx)-((*bx-*cx)*q-(*bx-*ax)*r)/ (2.0*SIGN(sMax(fabs(q-r),TINY),q-r));
        ulim=(*bx)+GLIMIT*(*cx-*bx);
        if ((*bx-u)*(u-*cx) > 0.0) {
            fu=(*func)(u,usr);
            if (fu < *fc) {
                *ax=(*bx);
                *bx=u;
                *fa=(*fb);
                *fb=fu;
                return;
            } else if (fu > *fb) {
                *cx=u;
                *fc=fu;
                return;
            }
            u=(*cx)+GOLD*(*cx-*bx);
            fu=(*func)(u,usr);
        } else if ((*cx-u)*(u-ulim) > 0.0) {
            fu=(*func)(u,usr);
            if (fu < *fc) {
                SHFT(*bx,*cx,u,*cx+GOLD*(*cx-*bx))
                SHFT(*fb,*fc,fu,(*func)(u,usr))
            }
        } else if ((u-ulim)*(ulim-*cx) >= 0.0) {
            u=ulim;
            fu=(*func)(u,usr);
        } else {
            u=(*cx)+GOLD*(*cx-*bx);
            fu=(*func)(u,usr);
        }
        SHFT(*ax,*bx,*cx,u)
        SHFT(*fa,*fb,*fc,fu)
    }
}
#undef GOLD
#undef GLIMIT
#undef TINY
#undef SHFT
#undef NRANSI
