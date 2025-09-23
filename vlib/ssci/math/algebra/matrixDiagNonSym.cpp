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
#include <ssci/math/algebra/algebra.hpp>
#include <slib/core/vec.hpp>
#include <math.h>

using namespace slib;



#define dabs sAbs


static int scale_(idx *n, idx *nm, real *a, real *h__, 
    real *prfact, real *enorm)
{
    idx a_dim1, a_offset, h_dim1, h_offset, i__1, i__2;
    real r__1;

    real sqrt(real);

    static idx i__, j;
    static real q, row;
    static idx iter;
    static real fnorm;
    static real bound1, bound2;
    static real factor, column;
    static idx ncount;







    --prfact;
    h_dim1 = *nm;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    a_dim1 = *nm;
    a_offset = 1 + a_dim1;
    a -= a_offset;

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        h__[i__ + j * h_dim1] = a[i__ + j * a_dim1];
    }
    prfact[i__] = 1.f;
    }
    bound1 = .75f;
    bound2 = 1.33f;
    iter = 0;
L3:
    ncount = 0;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    column = 0.f;
    row = 0.f;
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        if (i__ == j) {
        goto L4;
        }
        column += (r__1 = a[j + i__ * a_dim1], dabs(r__1));
        row += (r__1 = a[i__ + j * a_dim1], dabs(r__1));
L4:
        ;
    }
    if (column == 0.f) {
        goto L5;
    }
    if (row == 0.f) {
        goto L5;
    }
    q = column / row;
    if (q < bound1) {
        goto L6;
    }
    if (q > bound2) {
        goto L6;
    }
L5:
    ++ncount;
    goto L8;
L6:
    factor = sqrt(q);
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        if (i__ == j) {
        goto L7;
        }
        a[i__ + j * a_dim1] *= factor;
        a[j + i__ * a_dim1] /= factor;
L7:
        ;
    }
    prfact[i__] *= factor;
L8:
    ;
    }
    ++iter;
    if (iter > 30) {
    goto L11;
    }
    if (ncount < *n) {
    goto L3;
    }
    fnorm = 0.f;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        q = a[i__ + j * a_dim1];
        fnorm += q * q;
    }
    }
    fnorm = sqrt(fnorm);
    i__2 = *n;
    for (i__ = 1; i__ <= i__2; ++i__) {
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
        a[i__ + j * a_dim1] /= fnorm;
    }
    }
    *enorm = fnorm;
    goto L13;
L11:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        a[i__ + j * a_dim1] = h__[i__ + j * h_dim1];
    }
    }
    *enorm = 1.f;
L13:
    return 0;
}

static int hesqr_(idx *n, idx *nm, real *a, real *h__, real 
    *evr, real *evi, real *subdia, idx *indic, real *eps, real *ex)
{
    idx a_dim1, a_offset, h_dim1, h_offset, i__1, i__2, i__3;
    real r__1;

    real sqrt(real);

    static idx i__, j, k, l, m;
    static real r__;
    static real s;
    static real t;
    static real x, y, z__;
    static idx m1, ns;
    static real sr, sr2;
    static real shift;
    static idx maxst;









    --indic;
    --subdia;
    --evi;
    --evr;
    h_dim1 = *nm;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    a_dim1 = *nm;
    a_offset = 1 + a_dim1;
    a -= a_offset;

    if ((i__1 = *n - 2) < 0) {
    goto L14;
    } else if (i__1 == 0) {
    goto L1;
    } else {
    goto L2;
    }
L1:
    subdia[1] = a[a_dim1 + 2];
    goto L14;
L2:
    m = *n - 2;
    i__1 = m;
    for (k = 1; k <= i__1; ++k) {
    l = k + 1;
    s = 0.f;
    i__2 = *n;
    for (i__ = l; i__ <= i__2; ++i__) {
        h__[i__ + k * h_dim1] = a[i__ + k * a_dim1];
        s += (r__1 = a[i__ + k * a_dim1], dabs(r__1));
    }
    if (s != (r__1 = a[k + 1 + k * a_dim1], dabs(r__1))) {
        goto L4;
    }
    subdia[k] = a[k + 1 + k * a_dim1];
    h__[k + 1 + k * h_dim1] = 0.f;
    goto L12;
L4:
    sr2 = 0.f;
    i__2 = *n;
    for (i__ = l; i__ <= i__2; ++i__) {
        sr = a[i__ + k * a_dim1];
        sr /= s;
        a[i__ + k * a_dim1] = sr;
        sr2 += sr * sr;
    }
    sr = sqrt(sr2);
    if (a[l + k * a_dim1] < 0.f) {
        goto L6;
    }
    sr = -sr;
L6:
    sr2 -= sr * a[l + k * a_dim1];
    a[l + k * a_dim1] -= sr;
    h__[l + k * h_dim1] -= sr * s;
    subdia[k] = sr * s;
    x = s * sqrt(sr2);
    i__2 = *n;
    for (i__ = l; i__ <= i__2; ++i__) {
        h__[i__ + k * h_dim1] /= x;
        subdia[i__] = a[i__ + k * a_dim1] / sr2;
    }


    i__2 = *n;
    for (j = l; j <= i__2; ++j) {
        sr = 0.f;
        i__3 = *n;
        for (i__ = l; i__ <= i__3; ++i__) {
        sr += a[i__ + k * a_dim1] * a[i__ + j * a_dim1];
        }
        i__3 = *n;
        for (i__ = l; i__ <= i__3; ++i__) {
        a[i__ + j * a_dim1] -= subdia[i__] * sr;
        }
    }


    i__3 = *n;
    for (j = 1; j <= i__3; ++j) {
        sr = 0.f;
        i__2 = *n;
        for (i__ = l; i__ <= i__2; ++i__) {
        sr += a[j + i__ * a_dim1] * a[i__ + k * a_dim1];
        }
        i__2 = *n;
        for (i__ = l; i__ <= i__2; ++i__) {
        a[j + i__ * a_dim1] -= subdia[i__] * sr;
        }
    }
L12:
    ;
    }
    i__1 = m;
    for (k = 1; k <= i__1; ++k) {
    a[k + 1 + k * a_dim1] = subdia[k];
    }


    subdia[*n - 1] = a[*n + (*n - 1) * a_dim1];
L14:
    *eps = 0.f;
    i__1 = *n;
    for (k = 1; k <= i__1; ++k) {
    indic[k] = 0;
    if (k != *n) {
        r__1 = subdia[k];
        *eps += r__1 * r__1;
    }
    i__2 = *n;
    for (i__ = k; i__ <= i__2; ++i__) {
        h__[k + i__ * h_dim1] = a[k + i__ * a_dim1];
        r__1 = a[k + i__ * a_dim1];
        *eps += r__1 * r__1;
    }
    }
    *eps = *ex * sqrt(*eps);



    shift = a[*n + (*n - 1) * a_dim1];
    if (*n <= 2) {
    shift = 0.f;
    }
    if (a[*n + *n * a_dim1] != 0.f) {
    shift = 0.f;
    }
    if (a[*n - 1 + *n * a_dim1] != 0.f) {
    shift = 0.f;
    }
    if (a[*n - 1 + (*n - 1) * a_dim1] != 0.f) {
    shift = 0.f;
    }
    m = *n;
    ns = 0;
    maxst = *n * 10;


    i__2 = *n;
    for (i__ = 2; i__ <= i__2; ++i__) {
    i__1 = *n;
    for (k = i__; k <= i__1; ++k) {
        if (a[i__ - 1 + k * a_dim1] != 0.f) {
        goto L18;
        }
    }
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    indic[i__] = 1;
    evr[i__] = a[i__ + i__ * a_dim1];
    evi[i__] = 0.f;
    }
    goto L37;


L18:
    k = m - 1;
    m1 = k;
    i__ = k;


    if (k < 0) {
    goto L37;
    } else if (k == 0) {
    goto L34;
    } else {
    goto L19;
    }
L19:
    if ((r__1 = a[m + k * a_dim1], dabs(r__1)) <= *eps) {
    goto L34;
    }
    if (m - 2 == 0) {
    goto L35;
    }
L20:
    --i__;
    if ((r__1 = a[k + i__ * a_dim1], dabs(r__1)) <= *eps) {
    goto L21;
    }
    k = i__;
    if (k > 1) {
    goto L20;
    }
L21:
    if (k == m1) {
    goto L35;
    }


    s = a[m + m * a_dim1] + a[m1 + m1 * a_dim1] + shift;
    r__1 = shift;
    sr = a[m + m * a_dim1] * a[m1 + m1 * a_dim1] - a[m + m1 * a_dim1] * a[m1 
        + m * a_dim1] + r__1 * r__1 * .25f;
    a[k + 2 + k * a_dim1] = 0.f;


    x = a[k + k * a_dim1] * (a[k + k * a_dim1] - s) + a[k + (k + 1) * a_dim1] 
        * a[k + 1 + k * a_dim1] + sr;
    y = a[k + 1 + k * a_dim1] * (a[k + k * a_dim1] + a[k + 1 + (k + 1) * 
        a_dim1] - s);
    r__ = dabs(x) + dabs(y);
    if (r__ == 0.f) {
    shift = a[m + (m - 1) * a_dim1];
    }
    if (r__ == 0.f) {
    goto L21;
    }
    z__ = a[k + 2 + (k + 1) * a_dim1] * a[k + 1 + k * a_dim1];
    shift = 0.f;
    ++ns;


    i__1 = m1;
    for (i__ = k; i__ <= i__1; ++i__) {
    if (i__ == k) {
        goto L22;
    }


    x = a[i__ + (i__ - 1) * a_dim1];
    y = a[i__ + 1 + (i__ - 1) * a_dim1];
    z__ = 0.f;
    if (i__ + 2 > m) {
        goto L22;
    }
    z__ = a[i__ + 2 + (i__ - 1) * a_dim1];
L22:
    sr2 = dabs(x) + dabs(y) + dabs(z__);
    if (sr2 == 0.f) {
        goto L23;
    }
    x /= sr2;
    y /= sr2;
    z__ /= sr2;
L23:
    s = sqrt(x * x + y * y + z__ * z__);
    if (x < 0.f) {
        goto L24;
    }
    s = -s;
L24:
    if (i__ == k) {
        goto L25;
    }
    a[i__ + (i__ - 1) * a_dim1] = s * sr2;
L25:
    if (sr2 != 0.f) {
        goto L26;
    }
    if (i__ + 3 > m) {
        goto L33;
    }
    goto L32;
L26:
    sr = 1.f - x / s;
    s = x - s;
    x = y / s;
    y = z__ / s;


    i__2 = m;
    for (j = i__; j <= i__2; ++j) {
        s = a[i__ + j * a_dim1] + a[i__ + 1 + j * a_dim1] * x;
        if (i__ + 2 > m) {
        goto L27;
        }
        s += a[i__ + 2 + j * a_dim1] * y;
L27:
        s *= sr;
        a[i__ + j * a_dim1] -= s;
        a[i__ + 1 + j * a_dim1] -= s * x;
        if (i__ + 2 > m) {
        goto L28;
        }
        a[i__ + 2 + j * a_dim1] -= s * y;
L28:
        ;
    }


    l = i__ + 2;
    if (i__ < m1) {
        goto L29;
    }
    l = m;
L29:
    i__2 = l;
    for (j = k; j <= i__2; ++j) {
        s = a[j + i__ * a_dim1] + a[j + (i__ + 1) * a_dim1] * x;
        if (i__ + 2 > m) {
        goto L30;
        }
        s += a[j + (i__ + 2) * a_dim1] * y;
L30:
        s *= sr;
        a[j + i__ * a_dim1] -= s;
        a[j + (i__ + 1) * a_dim1] -= s * x;
        if (i__ + 2 > m) {
        goto L31;
        }
        a[j + (i__ + 2) * a_dim1] -= s * y;
L31:
        ;
    }
    if (i__ + 3 > m) {
        goto L33;
    }
    s = -a[i__ + 3 + (i__ + 2) * a_dim1] * y * sr;
L32:
    a[i__ + 3 + i__ * a_dim1] = s;
    a[i__ + 3 + (i__ + 1) * a_dim1] = s * x;
    a[i__ + 3 + (i__ + 2) * a_dim1] = s * y + a[i__ + 3 + (i__ + 2) * 
        a_dim1];
L33:
    ;
    }
    if (ns > maxst) {
    goto L37;
    }
    goto L18;


L34:
    evr[m] = a[m + m * a_dim1];
    evi[m] = 0.f;
    indic[m] = 1;
    m = k;
    goto L18;


L35:
    r__ = (a[k + k * a_dim1] + a[m + m * a_dim1]) * .5f;
    s = (a[m + m * a_dim1] - a[k + k * a_dim1]) * .5f;
    s = s * s + a[k + m * a_dim1] * a[m + k * a_dim1];
    indic[k] = 1;
    indic[m] = 1;
    if (s < 0.f) {
    goto L36;
    }
    t = sqrt(s);
    evr[k] = r__ - t;
    evr[m] = r__ + t;
    evi[k] = 0.f;
    evi[m] = 0.f;
    m += -2;
    goto L18;
L36:
    t = sqrt(-s);
    evr[k] = r__;
    evi[k] = t;
    evr[m] = r__;
    evi[m] = -t;
    m += -2;
    goto L18;
L37:
    return 0;
}

static int realve_(idx *n, idx *nm, idx *m, idx *
    ivec, real *a, real *vecr, real *evr, real *evi, idx *iwork, real 
    *work, idx *indic, real *eps, real *ex)
{
    idx a_dim1, a_offset, vecr_dim1, vecr_offset, i__1, i__2;
    real r__1, r__2;

    static idx i__, j, k, l;
    static real r__;
    static real s;
    static real t, r1;
    static idx ns;
    static real sr;
    static idx iter;
    static real bound, evalue, previs;













    --indic;
    --work;
    --iwork;
    --evi;
    --evr;
    vecr_dim1 = *nm;
    vecr_offset = 1 + vecr_dim1;
    vecr -= vecr_offset;
    a_dim1 = *nm;
    a_offset = 1 + a_dim1;
    a -= a_offset;

    vecr[*ivec * vecr_dim1 + 1] = 1.f;
    if (*m == 1) {
    goto L24;
    }


    evalue = evr[*ivec];
    if (*ivec == *m) {
    goto L2;
    }
    k = *ivec + 1;
    r__ = 0.f;
    i__1 = *m;
    for (i__ = k; i__ <= i__1; ++i__) {
    if (evalue != evr[i__]) {
        goto L1;
    }
    if (evi[i__] != 0.f) {
        goto L1;
    }
    r__ += 3.f;
L1:
    ;
    }
    evalue += r__ * *ex;
L2:
    i__1 = *m;
    for (k = 1; k <= i__1; ++k) {
    a[k + k * a_dim1] -= evalue;
    }


    k = *m - 1;
    i__1 = k;
    for (i__ = 1; i__ <= i__1; ++i__) {
    l = i__ + 1;
    iwork[i__] = 0;
    if (a[i__ + 1 + i__ * a_dim1] != 0.f) {
        goto L4;
    }
    if (a[i__ + i__ * a_dim1] != 0.f) {
        goto L8;
    }
    a[i__ + i__ * a_dim1] = *eps;
    goto L8;
L4:
    if ((r__1 = a[i__ + i__ * a_dim1], dabs(r__1)) >= (r__2 = a[i__ + 1 + 
        i__ * a_dim1], dabs(r__2))) {
        goto L6;
    }
    iwork[i__] = j;
    i__2 = *m;
    for (j = i__; j <= i__2; ++j) {
        r__ = a[i__ + j * a_dim1];
        a[i__ + j * a_dim1] = a[i__ + 1 + j * a_dim1];
        a[i__ + 1 + j * a_dim1] = r__;
    }
L6:
    r__ = -a[i__ + 1 + i__ * a_dim1] / a[i__ + i__ * a_dim1];
    a[i__ + 1 + i__ * a_dim1] = r__;
    i__2 = *m;
    for (j = l; j <= i__2; ++j) {
        a[i__ + 1 + j * a_dim1] += r__ * a[i__ + j * a_dim1];
    }
L8:
    ;
    }
    if (a[*m + *m * a_dim1] != 0.f) {
    goto L9;
    }
    a[*m + *m * a_dim1] = *eps;


L9:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    if (i__ >= *m) {
        goto L10;
    }
    work[i__] = 1.f;
    goto L11;
L10:
    work[i__] = 0.f;
L11:
    ;
    }


    bound = .01f / (*ex * (real) (*n));
    ns = 0;
    iter = 1;


L12:
    r__ = 0.f;
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    j = *m - i__ + 1;
    s = work[j];
    if (j == *m) {
        goto L14;
    }
    l = j + 1;
    i__2 = *m;
    for (k = l; k <= i__2; ++k) {
        sr = work[k];
        s -= sr * a[j + k * a_dim1];
    }
L14:
    work[j] = s / a[j + j * a_dim1];
    t = (r__1 = work[j], dabs(r__1));
    if (r__ >= t) {
        goto L15;
    }
    r__ = t;
L15:
    ;
    }


    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    work[i__] /= r__;
    }


    r1 = 0.f;
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    t = 0.f;
    i__2 = *m;
    for (j = i__; j <= i__2; ++j) {
        t += a[i__ + j * a_dim1] * work[j];
    }
    t = dabs(t);
    if (r1 >= t) {
        goto L18;
    }
    r1 = t;
L18:
    ;
    }
    if (iter == 1) {
    goto L19;
    }
    if (previs <= r1) {
    goto L24;
    }
L19:
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    vecr[i__ + *ivec * vecr_dim1] = work[i__];
    }
    previs = r1;
    if (ns == 1) {
    goto L24;
    }
    if (iter > 6) {
    goto L25;
    }
    ++iter;
    if (r__ < bound) {
    goto L21;
    }
    ns = 1;


L21:
    k = *m - 1;
    i__1 = k;
    for (i__ = 1; i__ <= i__1; ++i__) {
    r__ = work[i__ + 1];
    if (iwork[i__] == 0) {
        goto L22;
    }
    work[i__ + 1] = work[i__] + work[i__ + 1] * a[i__ + 1 + i__ * a_dim1];
    work[i__] = r__;
    goto L23;
L22:
    work[i__ + 1] += work[i__] * a[i__ + 1 + i__ * a_dim1];
L23:
    ;
    }
    goto L12;
L24:
    indic[*ivec] = 2;
L25:
    if (*m == *n) {
    goto L27;
    }
    j = *m + 1;
    i__1 = *n;
    for (i__ = j; i__ <= i__1; ++i__) {
    vecr[i__ + *ivec * vecr_dim1] = 0.f;
    }
L27:
    return 0;
}

static int compve_(idx *n, idx *nm, idx *m, idx *
    ivec, real *a, real *vecr, real *h__, real *evr, real *evi, idx *
    indic, idx *iwork, real *subdia, real *work1, real *work2, real *
    work, real *eps, real *ex)
{
    idx a_dim1, a_offset, h_dim1, h_offset, vecr_dim1, vecr_offset, i__1, 
        i__2, i__3;
    real r__1, r__2;

    real sqrt(real);

    static real b;
    static real d__;
    static idx i__, j, k, l;
    static real r__, s, u, v;
    static real d1;
    static idx i1, i2, ns;
    static real eta, fksi;
    static idx iter;
    static real bound, previs;










    --work;
    --work2;
    --work1;
    --subdia;
    --iwork;
    --indic;
    --evi;
    --evr;
    h_dim1 = *nm;
    h_offset = 1 + h_dim1;
    h__ -= h_offset;
    vecr_dim1 = *nm;
    vecr_offset = 1 + vecr_dim1;
    vecr -= vecr_offset;
    a_dim1 = *nm;
    a_offset = 1 + a_dim1;
    a -= a_offset;

    fksi = evr[*ivec];
    eta = evi[*ivec];


    if (*ivec == *m) {
    goto L2;
    }
    k = *ivec + 1;
    r__ = 0.f;
    i__1 = *m;
    for (i__ = k; i__ <= i__1; ++i__) {
    if (fksi != evr[i__]) {
        goto L1;
    }
    if (dabs(eta) != (r__1 = evi[i__], dabs(r__1))) {
        goto L1;
    }
    r__ += 3.f;
L1:
    ;
    }
    r__ *= *ex;
    fksi += r__;
    eta += r__;


L2:
    r__ = fksi * fksi + eta * eta;
    s = fksi * 2.f;
    l = *m - 1;
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *m;
    for (j = i__; j <= i__2; ++j) {
        d__ = 0.f;
        a[j + i__ * a_dim1] = 0.f;
        i__3 = j;
        for (k = i__; k <= i__3; ++k) {
        d__ += h__[i__ + k * h_dim1] * h__[k + j * h_dim1];
        }
        a[i__ + j * a_dim1] = d__ - s * h__[i__ + j * h_dim1];
    }
    a[i__ + i__ * a_dim1] += r__;
    }
    i__1 = l;
    for (i__ = 1; i__ <= i__1; ++i__) {
    r__ = subdia[i__];
    a[i__ + 1 + i__ * a_dim1] = -s * r__;
    i1 = i__ + 1;
    i__2 = i1;
    for (j = 1; j <= i__2; ++j) {
        a[j + i__ * a_dim1] += r__ * h__[j + (i__ + 1) * h_dim1];
    }
    if (i__ == 1) {
        goto L7;
    }
    a[i__ + 1 + (i__ - 1) * a_dim1] = r__ * subdia[i__ - 1];
L7:
    i__2 = *m;
    for (j = i__; j <= i__2; ++j) {
        a[i__ + 1 + j * a_dim1] += r__ * h__[i__ + j * h_dim1];
    }
    }


    k = *m - 1;
    i__1 = k;
    for (i__ = 1; i__ <= i__1; ++i__) {
    i1 = i__ + 1;
    i2 = i__ + 2;
    iwork[i__] = 0;
    if (i__ == k) {
        goto L10;
    }
    if (a[i__ + 2 + i__ * a_dim1] != 0.f) {
        goto L11;
    }
L10:
    if (a[i__ + 1 + i__ * a_dim1] != 0.f) {
        goto L11;
    }
    if (a[i__ + i__ * a_dim1] != 0.f) {
        goto L18;
    }
    a[i__ + i__ * a_dim1] = *eps;
    goto L18;
L11:
    if (i__ == k) {
        goto L12;
    }
    if ((r__1 = a[i__ + 1 + i__ * a_dim1], dabs(r__1)) >= (r__2 = a[i__ + 
        2 + i__ * a_dim1], dabs(r__2))) {
        goto L12;
    }
    if ((r__1 = a[i__ + i__ * a_dim1], dabs(r__1)) >= (r__2 = a[i__ + 2 + 
        i__ * a_dim1], dabs(r__2))) {
        goto L16;
    }
    l = i__ + 2;
    iwork[i__] = 2;
    goto L13;
L12:
    if ((r__1 = a[i__ + i__ * a_dim1], dabs(r__1)) >= (r__2 = a[i__ + 1 + 
        i__ * a_dim1], dabs(r__2))) {
        goto L15;
    }
    l = i__ + 1;
    iwork[i__] = 1;
L13:
    i__2 = *m;
    for (j = i__; j <= i__2; ++j) {
        r__ = a[i__ + j * a_dim1];
        a[i__ + j * a_dim1] = a[l + j * a_dim1];
        a[l + j * a_dim1] = r__;
    }
L15:
    if (i__ != k) {
        goto L16;
    }
    i2 = i1;
L16:
    i__2 = i2;
    for (l = i1; l <= i__2; ++l) {
        r__ = -a[l + i__ * a_dim1] / a[i__ + i__ * a_dim1];
        a[l + i__ * a_dim1] = r__;
        i__3 = *m;
        for (j = i1; j <= i__3; ++j) {
        a[l + j * a_dim1] += r__ * a[i__ + j * a_dim1];
        }
    }
L18:
    ;
    }
    if (a[*m + *m * a_dim1] != 0.f) {
    goto L19;
    }
    a[*m + *m * a_dim1] = *eps;


L19:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    if (i__ > *m) {
        goto L20;
    }
    vecr[i__ + *ivec * vecr_dim1] = 1.f;
    vecr[i__ + (*ivec - 1) * vecr_dim1] = 1.f;
    goto L21;
L20:
    vecr[i__ + *ivec * vecr_dim1] = 0.f;
    vecr[i__ + (*ivec - 1) * vecr_dim1] = 0.f;
L21:
    ;
    }


    bound = .01f / (*ex * (real) (*n));
    ns = 0;
    iter = 1;
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    work[i__] = h__[i__ + i__ * h_dim1] - fksi;
    }



L23:
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    d__ = work[i__] * vecr[i__ + *ivec * vecr_dim1];
    if (i__ == 1) {
        goto L24;
    }
    d__ += subdia[i__ - 1] * vecr[i__ - 1 + *ivec * vecr_dim1];
L24:
    l = i__ + 1;
    if (l > *m) {
        goto L26;
    }
    i__3 = *m;
    for (k = l; k <= i__3; ++k) {
        d__ += h__[i__ + k * h_dim1] * vecr[k + *ivec * vecr_dim1];
    }
L26:
    vecr[i__ + (*ivec - 1) * vecr_dim1] = d__ - eta * vecr[i__ + (*ivec - 
        1) * vecr_dim1];
    }


    k = *m - 1;
    i__1 = k;
    for (i__ = 1; i__ <= i__1; ++i__) {
    l = i__ + iwork[i__];
    r__ = vecr[l + (*ivec - 1) * vecr_dim1];
    vecr[l + (*ivec - 1) * vecr_dim1] = vecr[i__ + (*ivec - 1) * 
        vecr_dim1];
    vecr[i__ + (*ivec - 1) * vecr_dim1] = r__;
    vecr[i__ + 1 + (*ivec - 1) * vecr_dim1] += a[i__ + 1 + i__ * a_dim1] *
         r__;
    if (i__ == k) {
        goto L28;
    }
    vecr[i__ + 2 + (*ivec - 1) * vecr_dim1] += a[i__ + 2 + i__ * a_dim1] *
         r__;
L28:
    ;
    }


    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    j = *m - i__ + 1;
    d__ = vecr[j + (*ivec - 1) * vecr_dim1];
    if (j == *m) {
        goto L30;
    }
    l = j + 1;
    i__3 = *m;
    for (k = l; k <= i__3; ++k) {
        d1 = a[j + k * a_dim1];
        d__ -= d1 * vecr[k + (*ivec - 1) * vecr_dim1];
    }
L30:
    vecr[j + (*ivec - 1) * vecr_dim1] = d__ / a[j + j * a_dim1];
    }


    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    d__ = work[i__] * vecr[i__ + (*ivec - 1) * vecr_dim1];
    if (i__ == 1) {
        goto L32;
    }
    d__ += subdia[i__ - 1] * vecr[i__ - 1 + (*ivec - 1) * vecr_dim1];
L32:
    l = i__ + 1;
    if (l > *m) {
        goto L34;
    }
    i__3 = *m;
    for (k = l; k <= i__3; ++k) {
        d__ += h__[i__ + k * h_dim1] * vecr[k + (*ivec - 1) * vecr_dim1];
    }
L34:
    vecr[i__ + *ivec * vecr_dim1] = (vecr[i__ + *ivec * vecr_dim1] - d__) 
        / eta;
    }


    l = 1;
    s = 0.f;
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    r__1 = vecr[i__ + *ivec * vecr_dim1];
    r__2 = vecr[i__ + (*ivec - 1) * vecr_dim1];
    r__ = r__1 * r__1 + r__2 * r__2;
    if (r__ <= s) {
        goto L36;
    }
    s = r__;
    l = i__;
L36:
    ;
    }


    u = vecr[l + (*ivec - 1) * vecr_dim1];
    v = vecr[l + *ivec * vecr_dim1];
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    b = vecr[i__ + *ivec * vecr_dim1];
    r__ = vecr[i__ + (*ivec - 1) * vecr_dim1];
    vecr[i__ + *ivec * vecr_dim1] = (r__ * u + b * v) / s;
    vecr[i__ + (*ivec - 1) * vecr_dim1] = (b * u - r__ * v) / s;
    }


    b = 0.f;
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
    r__ = work[i__] * vecr[i__ + (*ivec - 1) * vecr_dim1] - eta * vecr[
        i__ + *ivec * vecr_dim1];
    u = work[i__] * vecr[i__ + *ivec * vecr_dim1] + eta * vecr[i__ + (*
        ivec - 1) * vecr_dim1];
    if (i__ == 1) {
        goto L38;
    }
    r__ += subdia[i__ - 1] * vecr[i__ - 1 + (*ivec - 1) * vecr_dim1];
    u += subdia[i__ - 1] * vecr[i__ - 1 + *ivec * vecr_dim1];
L38:
    l = i__ + 1;
    if (l > *m) {
        goto L40;
    }
    i__3 = *m;
    for (j = l; j <= i__3; ++j) {
        r__ += h__[i__ + j * h_dim1] * vecr[j + (*ivec - 1) * vecr_dim1];
        u += h__[i__ + j * h_dim1] * vecr[j + *ivec * vecr_dim1];
    }
L40:
    u = r__ * r__ + u * u;
    if (b >= u) {
        goto L41;
    }
    b = u;
L41:
    ;
    }
    if (iter == 1) {
    goto L42;
    }
    if (previs <= b) {
    goto L44;
    }
L42:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    work1[i__] = vecr[i__ + *ivec * vecr_dim1];
    work2[i__] = vecr[i__ + (*ivec - 1) * vecr_dim1];
    }
    previs = b;
    if (ns == 1) {
    goto L46;
    }
    if (iter > 6) {
    goto L47;
    }
    ++iter;
    if (bound > sqrt(s)) {
    goto L23;
    }
    ns = 1;
    goto L23;
L44:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    vecr[i__ + *ivec * vecr_dim1] = work1[i__];
    vecr[i__ + (*ivec - 1) * vecr_dim1] = work2[i__];
    }
L46:
    indic[*ivec - 1] = 2;
    indic[*ivec] = 2;
L47:
    return 0;
}









idx sAlgebra::matrix::diagNonSym(idx *n, idx *nm, real *a, real *t, real * evr, real *evi, real *vecr, real *veci, idx *indic)
{
    idx a_dim1, a_offset, veci_dim1, veci_offset, vecr_dim1, vecr_offset, 
        i__1, i__2, i__3;
    real r__1, r__2;


    idx i__, j, k, l, m;
    real r__;
    real d1, d2, d3;
    idx k1, l1;
    real r1, ex, eps;
    idx kon, ivec;
    idx ddim=(*n+1);
    sVec<real> Work; Work.resize(ddim); real * work=Work.ptr();
    sVec<real> Work1; Work1.resize(ddim); real * work1=Work1.ptr();
    sVec<real> Work2; Work2.resize(ddim); real * work2=Work2.ptr();
    sVec<idx> Local; Local.resize(ddim); idx * local=Local.ptr();
    real enorm;
    sVec<idx> Iwork; Iwork.resize(ddim); idx * iwork=Iwork.ptr();
    sVec<real> Subdia; Subdia.resize(ddim); real * subdia=Subdia.ptr();
    sVec<real> Prfact; Prfact.resize(ddim); real * prfact=Prfact.ptr();
















    --indic;
    veci_dim1 = *nm;
    veci_offset = 1 + veci_dim1;
    veci -= veci_offset;
    vecr_dim1 = *nm;
    vecr_offset = 1 + vecr_dim1;
    vecr -= vecr_offset;
    --evi;
    --evr;
    a_dim1 = *nm;
    a_offset = 1 + a_dim1;
    a -= a_offset;

    if (*n != 1) {
    goto L1;
    }
    evr[1] = a[a_dim1 + 1];
    evi[1] = 0.f;
    vecr[vecr_dim1 + 1] = 1.f;
    veci[veci_dim1 + 1] = 0.f;
    indic[1] = 2;
    goto L25;
L1:
    scale_(n, nm, &a[a_offset], &veci[veci_offset], prfact, &enorm);


    ex = exp(-(*t) * log(2.f));
    hesqr_(n, nm, &a[a_offset], &veci[veci_offset], &evr[1], &evi[1], subdia, 
        &indic[1], &eps, &ex);


    j = *n;
    i__ = 1;
    local[0] = 1;
    if (j == 1) {
    goto L4;
    }
L2:
    if ((r__1 = subdia[j - 2], dabs(r__1)) > eps) {
    goto L3;
    }
    ++i__;
    local[i__ - 1] = 0;
L3:
    --j;
    ++local[i__ - 1];
    if (j != 1) {
    goto L2;
    }


L4:
    k = 1;
    kon = 0;
    l = local[0];
    m = *n;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    ivec = *n - i__ + 1;
    if (i__ <= l) {
        goto L5;
    }
    ++k;
    m = *n - l;
    l += local[k - 1];
L5:
    if (indic[ivec] == 0) {
        goto L10;
    }
    if (evi[ivec] != 0.f) {
        goto L8;
    }


    i__2 = m;
    for (k1 = 1; k1 <= i__2; ++k1) {
        i__3 = m;
        for (l1 = k1; l1 <= i__3; ++l1) {
        a[k1 + l1 * a_dim1] = veci[k1 + l1 * veci_dim1];
        }
        if (k1 == 1) {
        goto L7;
        }
        a[k1 + (k1 - 1) * a_dim1] = subdia[k1 - 2];
L7:
        ;
    }


    realve_(n, nm, &m, &ivec, &a[a_offset], &vecr[vecr_offset], &evr[1], &
        evi[1], iwork, work, &indic[1], &eps, &ex);
    goto L10;


L8:
    if (kon != 0) {
        goto L9;
    }
    kon = 1;
    compve_(n, nm, &m, &ivec, &a[a_offset], &vecr[vecr_offset], &veci[
        veci_offset], &evr[1], &evi[1], &indic[1], iwork, subdia, 
        work1, work2, work, &eps, &ex);
    goto L10;
L9:
    kon = 0;
L10:
    ;
    }


    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *n;
    for (j = i__; j <= i__2; ++j) {
        a[i__ + j * a_dim1] = 0.f;
        a[j + i__ * a_dim1] = 0.f;
    }
    a[i__ + i__ * a_dim1] = 1.f;
    }
    if (*n <= 2) {
    goto L15;
    }
    m = *n - 2;
    i__1 = m;
    for (k = 1; k <= i__1; ++k) {
    l = k + 1;
    i__2 = *n;
    for (j = 2; j <= i__2; ++j) {
        d1 = 0.f;
        i__3 = *n;
        for (i__ = l; i__ <= i__3; ++i__) {
        d2 = veci[i__ + k * veci_dim1];
        d1 += d2 * a[j + i__ * a_dim1];
        }
        i__3 = *n;
        for (i__ = l; i__ <= i__3; ++i__) {
        a[j + i__ * a_dim1] -= veci[i__ + k * veci_dim1] * d1;
        }
    }
    }


L15:
    kon = 1;
    i__3 = *n;
    for (i__ = 1; i__ <= i__3; ++i__) {
    l = 0;
    if (evi[i__] == 0.f) {
        goto L16;
    }
    l = 1;
    if (kon == 0) {
        goto L16;
    }
    kon = 0;
    goto L24;
L16:
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        d1 = 0.f;
        d2 = 0.f;
        i__1 = *n;
        for (k = 1; k <= i__1; ++k) {
        d3 = a[j + k * a_dim1];
        d1 += d3 * vecr[k + i__ * vecr_dim1];
        if (l == 0) {
            goto L17;
        }
        d2 += d3 * vecr[k + (i__ - 1) * vecr_dim1];
L17:
        ;
        }
        work[j - 1] = d1 / prfact[j - 1];
        if (l == 0) {
        goto L18;
        }
        subdia[j - 1] = d2 / prfact[j - 1];
L18:
        ;
    }


    if (l == 1) {
        goto L21;
    }
    d1 = 0.f;
    i__2 = *n;
    for (m = 1; m <= i__2; ++m) {
        r__1 = work[m - 1];
        d1 += r__1 * r__1;
    }
    d1 = sqrt(d1);
    i__2 = *n;
    for (m = 1; m <= i__2; ++m) {
        veci[m + i__ * veci_dim1] = 0.f;
        vecr[m + i__ * vecr_dim1] = work[m - 1] / d1;
    }
    evr[i__] *= enorm;
    goto L24;
L21:
    kon = 1;
    evr[i__] *= enorm;
    evr[i__ - 1] = evr[i__];
    evi[i__] *= enorm;
    evi[i__ - 1] = -evi[i__];
    r__ = 0.f;
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        r__1 = work[j - 1];
        r__2 = subdia[j - 1];
        r1 = r__1 * r__1 + r__2 * r__2;
        if (r__ >= r1) {
        goto L22;
        }
        r__ = r1;
        l = j;
L22:
        ;
    }
    d3 = work[l - 1];
    r1 = subdia[l - 1];
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
        d1 = work[j - 1];
        d2 = subdia[j - 1];
        vecr[j + i__ * vecr_dim1] = (d1 * d3 + d2 * r1) / r__;
        veci[j + i__ * veci_dim1] = (d2 * d3 - d1 * r1) / r__;
        vecr[j + (i__ - 1) * vecr_dim1] = vecr[j + i__ * vecr_dim1];
        veci[j + (i__ - 1) * veci_dim1] = -veci[j + i__ * veci_dim1];
    }
L24:
    ;
    }
L25:

    return 0;
}
