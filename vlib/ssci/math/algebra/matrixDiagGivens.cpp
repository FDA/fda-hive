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
#include <math.h>
#include <ssci/math/algebra/algebra.hpp>

using namespace slib;

idx sAlgebra::matrix::diagGivens(idx nmax,idx nx,idx nrootx,idx njx,real * a,real * root,real * vect,real * b)
{
        idx b_dim1, b_offset, vect_dim1, vect_offset, i__1, i__2, i__3;
        real d__1, d__2;

        static idx iiii;
        static real save;
        static idx iter;
        static real temp;
        static idx jump;
        static real rand1, elim1, elim2;
        static idx nmip1, i, j, k, l, n;
        static real delta, theta, small, trial, anorm, aroot, toler, f0;
        static idx nsize, j1;
        static real rootl;
        static idx nroot;
        static real rootx, theta1;
        static idx ia;
        static real ak;
        static idx id, ic, ii, jj;
        static real delbig;
        static idx im;
        static real factor, alimit, zz;
        static idx nomtch, nm1, nm2, jp2;
        static real eta;
        static idx nom, nrt;
        static real sum, del1, aaaa, bbbb, cccc;














        b_dim1 = nmax;
        b_offset = b_dim1 + 1;
        b -= b_offset;
        vect_dim1 = nmax;
        vect_offset = vect_dim1 + 1;
        vect -= vect_offset;
        --root;
        --a;

        eta = 1e-15;
        theta = 1e65;
        i__1 = njx;
        for (i = 1; i <= i__1; ++i) {
        i__2 = nrootx;
        for (j = 1; j <= i__2; ++j) {
                vect[i + j * vect_dim1] = 0.;
        }
        }
        i__1 = nx;
        for (i = 1; i <= i__1; ++i) {
        for (j = 1; j <= 5; ++j) {
                b[i + j * b_dim1] = 0.;
        }
    }

    del1 = eta / 100.;
    d__1 = eta;
    delta = d__1 * d__1 * 100.;
    d__1 = eta;
    small = d__1 * d__1 / 100.;
    delbig = theta * delta / 1e3;
    theta1 = .001 / theta;

    toler = eta * 1e4;

    rand1 = 8388605.;

    n = nx;
        nroot = abs(nrootx);
    if (nroot == 0) {
        goto L700;
    }
    if ((i__1 = n - 1) < 0) {
        goto L700;
    } else if (i__1 == 0) {
        goto L50;
    } else {
        goto L60;
        }
L50:
    root[1] = a[1];
        if (nrootx > 0) {
        vect[vect_dim1 + 1] = 1.;
    }
        goto L700;
L60:
    nsize = n * (n + 1) / 2;
    nm1 = n - 1;
    nm2 = n - 2;

    factor = 0.;
    i__1 = nsize;
    for (i = 1; i <= i__1; ++i) {
        zz = a[i];
        zz = fabs(zz);
        factor = sMax(factor,zz);
    }
    if (factor != 0.) {
        goto L100;
    }
        i__1 = nroot;
    for (i = 1; i <= i__1; ++i) {
        if (nrootx < 0) {
            goto L90;
        }
        i__2 = n;
        for (j = 1; j <= i__2; ++j) {
            vect[j + i * vect_dim1] = 0.;
        }
        vect[i + i * vect_dim1] = 1.;
L90:
        root[i] = 0.;
    }
        goto L700;

L100:
    anorm = 0.;
    j = 1;
    k = 1;
        i__1 = nsize;
    for (i = 1; i <= i__1; ++i) {
        if (i != j) {
            goto L110;
        }
        d__1 = a[i] / factor;
        anorm += d__1 * d__1 / 2.;
        ++k;
        j += k;
        goto L120;
L110:
        d__1 = a[i] / factor;
        anorm += d__1 * d__1;
L120:
        ;
    }
    anorm = sqrt(anorm * 2.) * factor;
    i__1 = nsize;
        for (i = 1; i <= i__1; ++i) {
        a[i] /= anorm;
    }
    alimit = 1.;

    id = 0;
    ia = 1;
    if (nm2 == 0) {
        goto L240;
    }
    i__1 = nm2;
        for (j = 1; j <= i__1; ++j) {
        ia = ia + j + 2;
        id = id + j + 1;
        jp2 = j + 2;
        ii = ia;
        sum = 0.;
        i__2 = n;
        for (i = jp2; i <= i__2; ++i) {
            d__1 = a[ii];
            sum += d__1 * d__1;
            ii += i;
        }
        temp = a[id];
        if (sum > small) {
            goto L150;
        }
        b[j + b_dim1] = temp;
        a[id] = 0.;
        goto L230;
L150:
        d__1 = temp;
        sum = sqrt(sum + d__1 * d__1);
        b[j + b_dim1] = -fabs(sum)*sSign(temp);
        b[j + 1 + (b_dim1 << 1)] = sqrt((fabs(temp) / sum + 1.) / 2.);
        d__1 = (float).5 / (b[j + 1 + (b_dim1 << 1)] * sum);
        temp = fabs(d__1)*sSign(temp);
        ii = ia;
        i__2 = n;
        for (i = jp2; i <= i__2; ++i) {
            b[i + (b_dim1 << 1)] = a[ii] * temp;
            ii += i;
        }
        ak = 0.;
        ic = id + 1;
        j1 = j + 1;
        i__2 = n;
        for (i = j1; i <= i__2; ++i) {
            jj = ic;
            temp = 0.;
            i__3 = n;
            for (ii = j1; ii <= i__3; ++ii) {
                temp += b[ii + (b_dim1 << 1)] * a[jj];
                if (ii < i) {
                    goto L170;
                }
                jj += ii;
                goto L180;
L170:
                ++jj;
L180:
                ;
            }
            ak += temp * b[i + (b_dim1 << 1)];
            b[i + b_dim1] = temp;
            ic += i;
        }
        i__2 = n;
        for (i = j1; i <= i__2; ++i) {
            b[i + b_dim1] -= ak * b[i + (b_dim1 << 1)];
        }
        jj = id;

        i__2 = n;
        for (i = j1; i <= i__2; ++i) {
            a[jj] = b[i + (b_dim1 << 1)];
            i__3 = i;
            for (ii = j1; ii <= i__3; ++ii) {
                ++jj;
                a[jj] -= (b[i + b_dim1] * b[ii + (b_dim1 << 1)] + b[i + (
                        b_dim1 << 1)] * b[ii + b_dim1]) * 2.;
            }
            jj += j;
        }
L230:
        ;
    }
L240:
    b[nm1 + b_dim1] = a[nsize - 1];
    a[nsize - 1] = 0.;

    jump = 1;
    i__1 = n;
        for (j = 1; j <= i__1; ++j) {
        b[j + (b_dim1 << 1)] = a[jump];
        d__1 = b[j + b_dim1];
        b[j + b_dim1 * 3] = d__1 * d__1;
        jump = jump + j + 1;
    }
    i__1 = nroot;
    for (i = 1; i <= i__1; ++i) {
        root[i] = alimit;
    }
    rootl = -alimit;
    i__1 = nroot;
    for (i = 1; i <= i__1; ++i) {
        rootx = alimit;
        i__2 = nroot;
        for (j = i; j <= i__2; ++j) {
            d__1 = rootx, d__2 = root[j];
            rootx = sMin(d__1,d__2);
        }
        root[i] = rootx;
L280:
        trial = (rootl + root[i]) * .5;
        if (trial == rootl || trial == root[i]) {
            goto L340;
        }
        nomtch = n;
        j = 1;
L290:
        f0 = b[j + (b_dim1 << 1)] - trial;
L300:
        if (fabs(f0) < theta1) {
            goto L310;
        }
        if (f0 >= 0.) {
                --nomtch;
        }
        ++j;
        if (j > n) {
            goto L320;
        }

        f0 = b[j + (b_dim1 << 1)] - trial - b[j - 1 + b_dim1 * 3] / f0;
        goto L300;
L310:
        j += 2;
        --nomtch;
        if (j <= n) {
            goto L290;
        }
L320:
        if (nomtch >= i) {
            goto L330;
        }
        rootl = trial;
        goto L280;
L330:
        root[i] = trial;
        nom = sMin(nroot,nomtch);
        root[nom] = trial;
        goto L280;
L340:
        ;
    }

    nrt = nroot / 2;
    i__1 = nrt;
        for (i = 1; i <= i__1; ++i) {
        save = root[i];
        nmip1 = nroot - i + 1;
        root[i] = root[nmip1];
        root[nmip1] = save;
    }
        if (nrootx < 0) {
        goto L680;
    }
        i__1 = n;
    for (i = 1; i <= i__1; ++i) {
        i__2 = nroot;
        for (j = 1; j <= i__2; ++j) {
            vect[i + j * vect_dim1] = 1.;
        }
    }
    i__2 = nroot;
    for (i = 1; i <= i__2; ++i) {
        aroot = root[i];
        if (i == 1) {
            goto L370;
        }
        if ((d__1 = root[i - 1] - aroot, fabs(d__1)) < toler) {
            goto L380;
        }
L370:
        ia = -1;
L380:
        ++ia;
        elim1 = a[1] - aroot;
        elim2 = b[b_dim1 + 1];
        jump = 1;
        i__1 = nm1;
        for (j = 1; j <= i__1; ++j) {
            jump = jump + j + 1;
            if (fabs(elim1) <= (d__1 = b[j + b_dim1], fabs(d__1))) {
                goto L390;
            }
            b[j + (b_dim1 << 1)] = elim1;
            b[j + b_dim1 * 3] = elim2;
            b[j + (b_dim1 << 2)] = 0.;
            temp = b[j + b_dim1] / elim1;
                elim1 = a[jump] - aroot - temp * elim2;
            elim2 = b[j + 1 + b_dim1];
            goto L400;
L390:
            b[j + (b_dim1 << 1)] = b[j + b_dim1];
                b[j + b_dim1 * 3] = a[jump] - aroot;
            b[j + (b_dim1 << 2)] = b[j + 1 + b_dim1];
            temp = 1.;
            if ((d__1 = b[j + b_dim1], fabs(d__1)) > theta1) {
                temp = elim1 / b[j + b_dim1];
            }
            elim1 = elim2 - temp * b[j + b_dim1 * 3];
            elim2 = -temp * b[j + 1 + b_dim1];
L400:
            b[j + b_dim1 * 5] = temp;
        }
        b[n + (b_dim1 << 1)] = elim1;
        b[n + b_dim1 * 3] = 0.;
        b[n + (b_dim1 << 2)] = 0.;
        b[nm1 + (b_dim1 << 2)] = 0.;
        iter = 1;
        if (ia != 0) {
            goto L510;
        }
L420:
        l = n + 1;
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
            --l;
L430:
            elim1 = vect[l + i * vect_dim1];
            if (l < n) {
                elim1 -= vect[l + 1 + i * vect_dim1] * b[l + b_dim1 * 3];
            }
            if (l < n - 1) {
                elim1 -= vect[l + 2 + i * vect_dim1] * b[l + (b_dim1 << 2)];
                }
            if (fabs(elim1) > delbig) {
                goto L440;
            }
            temp = b[l + (b_dim1 << 1)];
            if ((d__1 = b[l + (b_dim1 << 1)], fabs(d__1)) < delta) {
                temp = delta;
            }
            vect[l + i * vect_dim1] = elim1 / temp;
            goto L460;
L440:
            i__3 = n;
            for (k = 1; k <= i__3; ++k) {
                vect[k + i * vect_dim1] /= delbig;
                }
            goto L430;
L460:
            ;
        }
        switch ((idx)iter) {
                case 1:  goto L470;
            case 2:  goto L530;
        }
L470:
        ++iter;
L480:
        elim1 = vect[i * vect_dim1 + 1];
        i__1 = nm1;
        for (j = 1; j <= i__1; ++j) {
            if (b[j + (b_dim1 << 1)] == b[j + b_dim1]) {
                goto L490;
            }
                vect[j + i * vect_dim1] = elim1;
            elim1 = vect[j + 1 + i * vect_dim1] - elim1 * b[j + b_dim1 * 5];
            goto L500;
L490:
                vect[j + i * vect_dim1] = vect[j + 1 + i * vect_dim1];
            elim1 -= vect[j + 1 + i * vect_dim1] * temp;
L500:
            ;
        }
        vect[n + i * vect_dim1] = elim1;
        goto L420;
L510:
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
            aaaa = rand1 * 4099.;
            bbbb = 8388608.;
                iiii = (idx) (aaaa / bbbb);
                cccc = (real) iiii;
                rand1 = aaaa - cccc * bbbb;
            vect[j + i * vect_dim1] = rand1 / 4194304. - 1.;
        }
        goto L420;

L530:
        if (ia == 0) {
            goto L570;
        }
        i__1 = ia;
        for (j1 = 1; j1 <= i__1; ++j1) {
                k = i - j1;
            temp = 0.;
            i__3 = n;
            for (j = 1; j <= i__3; ++j) {
                temp += vect[j + i * vect_dim1] * vect[j + k * vect_dim1];
                }
            i__3 = n;
            for (j = 1; j <= i__3; ++j) {
                vect[j + i * vect_dim1] -= temp * vect[j + k * vect_dim1];
            }
        }
L570:
        switch ((idx)iter) {
            case 1:  goto L480;
            case 2:  goto L580;
        }
L580:
        elim1 = 0.;
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
            d__2 = (d__1 = vect[j + i * vect_dim1], fabs(d__1));
            elim1 = sMax(d__2,elim1);
        }
        temp = 0.;
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
            elim2 = vect[j + i * vect_dim1] / elim1;
            d__1 = elim2;
            temp += d__1 * d__1;
        }
        temp = 1. / (sqrt(temp) * elim1);
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
            vect[j + i * vect_dim1] *= temp;
            if ((d__1 = vect[j + i * vect_dim1], fabs(d__1)) < del1) {
                vect[j + i * vect_dim1] = 0.;
            }
        }
    }

    if (nm2 == 0) {
        goto L680;
    }
    jump = nsize - (n + 1);
    im = nm1;
    i__2 = nm2;
    for (i = 1; i <= i__2; ++i) {
        j1 = jump;
        i__1 = n;
        for (j = im; j <= i__1; ++j) {
            b[j + (b_dim1 << 1)] = a[j1];
            j1 += j;
        }
        i__1 = nroot;
        for (k = 1; k <= i__1; ++k) {
            temp = 0.;
            i__3 = n;
            for (j = im; j <= i__3; ++j) {
                temp += b[j + (b_dim1 << 1)] * vect[j + k * vect_dim1];
            }
                temp += temp;
            i__3 = n;
            for (j = im; j <= i__3; ++j) {
                vect[j + k * vect_dim1] -= temp * b[j + (b_dim1 << 1)];
            }
        }
        jump -= im;
        --im;
        }
L680:
    i__2 = nroot;
    for (i = 1; i <= i__2; ++i) {
        root[i] *= anorm;
    }
L700:
        return 0;
}

