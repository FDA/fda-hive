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
        /* System generated locals */
        idx b_dim1, b_offset, vect_dim1, vect_offset, i__1, i__2, i__3;
        real d__1, d__2;

        /* Local variables */
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

/*     --------------------------------------------------- */
/*      CALCULATES EIGENVALUES AND EIGENVECTORS OF real SYMMETRIC MATRIX
*/
/*      STORED IN PACKED UPPER TRIANGULAR FORM. */
/*     62.3  GIVENS  -EIGENVALUES AND EIGENVECTORS BY THE GIVENS METHOD.
*/
/*      BY FRANKLIN PROSSER, INDIANA UNIVERSITY. */
/*      SEPTEMBER, 1967 */

/*      THANKS ARE DUE TO F. E. HARRIS (STANFORD UNIVERSITY) AND H. H. */
/*      MICHELS (UNITED AIRCRAFT RESEARCH LABORATORIES) FOR EXCELLENT */
/*      WORK ON NUMERICAL DIFFICULTIES WITH EARLIER VERSIONS OF THIS */
/*      PROGRAM. */

/*      THE PARAMETERS FOR THE ROUTINE ARE... */
/*          NX     ORDER OF MATRIX */
/*          NROOTX NUMBER OF ROOTS WANTED.  THE NROOTX SMALLEST (MOST */
/*                  NEGATIVE) ROOTS WILL BE CALCULATED.  IF NO VECTORS */
/*                  ARE WANTED, MAKE THIS NUMBER NEGATIVE. */
/*          NJX    ROW DIMENSION OF VECT ARRAY.  SEE :VECT: BELOW. */
/*                  NJX MUST BE NOT LESS THAN NX. */
/*          A      MATRIX STORED BY COLUMNS IN PACKED UPPER TRIANGULAR */
/*                 FORM, I.E. OCCUPYING NX*(NX+1)/2 CONSECUTIVE */
/*                 LOCATIONS. */
/*          B      SCRATCH ARRAY USED BY GIVENS.  MUST BE AT LEAST */
/*                  NX*5 CELLS. */
/*          ROOT   ARRAY TO HOLD THE EIGENVALUES.  MUST BE AT LEAST */
/*                 NROOTX CELLS LONG.  THE NROOTX SMALLEST ROOTS ARE */
/*                  ORDERED LARGEST FIRST IN THIS ARRAY. */
/*          VECT   EIGENVECTOR ARRAY.  EACH COLUMN WILL HOLD AN */
/*                  EIGENVECTOR FOR THE CORRESPONDING ROOT.  MUST BE */
/*                  DIMENSIONED WITH :NJX: ROWS AND AT LEAST :NROOTX: */
/*                  COLUMNS, UNLESS NO VECTORS */
/*                  ARE REQUESTED (NEGATIVE NROOTX).  IN THIS LATTER */
/*                  CASE, THE ARGUMENT VECT IS JUST A DUMMY, AND THE */
/*                  STORAGE IS NOT USED. */

/*      THE ARRAYS A AND B ARE DESTROYED BY THE COMPUTATION.  THE RESULTS 
*/
/*      APPEAR IN ROOT AND VECT. */
/*      FOR PROPER FUNCTIONING OF THIS ROUTINE, THE RESULT OF A FLOATING 
*/
/*      POINT UNDERFLOW SHOULD BE A ZERO. */

/*      THE ORIGINAL REFERENCE TO THE GIVENS TECHNIQUE IS IN OAK RIDGE */
/*      REPORT NUMBER ORNL 1574 (PHYSICS), BY WALLACE GIVENS. */
/*      THE METHOD AS PRESENTED IN THIS PROGRAM CONSISTS OF FOUR STEPS, */

/*      ALL MODIFICATIONS OF THE ORIGINAL METHOD... */
/*      FIRST, THE INPUT MATRIX IS REDUCED TO TRIDIAGONAL FORM BY THE */
/*      HOUSEHOLDER TECHNIQUE (J. H. WILKINSON, COMP. J. 3, 23 (1960)). */

/*      THE ROOTS ARE THEN LOCATED BY THE STURM SEQUENCE METHOD (J. M. */
/*      ORTEGA (SEE REFERENCE BELOW).  THE VECTORS OF THE TRIDIAGONAL */
/*      FORM ARE THEN EVALUATED (J. H. WILKINSON, COMP. J. 1, 90 (1958)), 
*/
/*      AND LAST THE TRIDIAGONAL VECTORS ARE ROTATED TO VECTORS OF THE */
/*      ORIGINAL ARRAY (FIRST REFERENCE). */
/*      VECTORS FOR DEGENERATE (OR NEAR-DEGENERATE) ROOTS ARE FORCED */
/*      TO BE ORTHOGONAL, USING A METHOD SUGGESTED BY B. GARBOW, ARGONNE 
*/
/*      NATIONAL LABS (PRIVATE COMMUNICATION, 1964).  THE GRAM-SCHMIDT */
/*      PROCESS IS USED FOR THE ORTHOGONALIZATION. */

/*      AN EXCELLENT PRESENTATION OF THE GIVENS TECHNIQUE IS FOUND IN */
/*      J. M. ORTEGA:S ARTICLE IN :MATHEMATICS FOR DIGITAL COMPUTERS,: */
/*      VOLUME 2, ED. BY RALSTON AND WILF, WILEY (1967), PAGE 94. */


/*      ALL real LIBRARY FUNCTIONS AND INTERNAL FUNCTIONS ARE DEFINED */
/*      FOLLOWING STATEMENT FUNCTIONS.  THIS IS TO FACILITATE CONVERSION 
*/
/*      OF THIS ROUTINE TO DOUBLE PRECISION ON IBM 360 MACHINES. */
/*      TO ACCOMPLISH THIS, CHANGE THE FUNCTION DEFINITIONS TO THEIR */
/*      DOUBLE PRECISION COUNTERPARTS, AND ADD AN :IMPLICIT: STATEMENT */
/*      OF THE FORM...      IMPLICIT real*8(A-H),real*8(O-Z) */
/*     ZSQRT(X)=SQRT(X) */
/*     ZABS(X)=ABS(X) */
/*     ZMAX1(X,Y)=AMAX1(X,Y) */
/*     ZMIN1(X,Y)=AMIN1(X,Y) */
/*     ZSIGN(X,Y)=SIGN(X,Y) */

/*      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
*/
/*      USERS PLEASE NOTE... */
/*      THE FOLLOWING TWO PARAMETERS, ETA AND THETA, SHOULD BE ADJUSTED */

/*      BY THE USER FOR HIS PARTICULAR MACHINE. */
/*      ETA IS AN INDICATION OF THE PRECISION OF THE FLOATING POINT */
/*      REPRESENTATION ON THE COMPUTER BEING USED (ROUGHLY 10**(-M), */
/*      WHERE M IS THE NUMBER OF DECIMALS OF PRECISION ). */
/*      THETA IS AN INDICATION OF THE RANGE OF NUMBERS THAT CAN BE */
/*      EXPRESSED IN THE FLOATING POINT REPRESENTATION (ROUGHLY THE */
/*      LARGEST NUMBER). */
/*      SOME RECOMMENDED VALUES FOLLOW. */
/*      FOR CONTROL DATA 3600 (36-BIT BINARY FRACTION, 11-BIT BINARY */
/*      EXPONENT), ETA=1.E-11, THETA=1.E307. */
/*      FOR CONTROL DATA 6600 (48-BIT BINARY FRACTION, 11-BIT BINARY */
/*      EXPONENT), ETA=1.E-14, THETA=1.E307. */
/*      FOR IBM 7094, UNIVAC 1108, ETC. (27-BIT BINARY FRACTION, 8-BIT */
/*      BINARY EXPONENT), ETA=1.E-8, THETA=1.E37. */
/*      FOR IBM 360/50 AND 360/65 DOUBLE PRECISION (56-BIT HEXADECIMAL */
/*      FRACTION, 7-BIT HEXADECIMAL EXPONENT), ETA=1.E-16, THETA=1.E75. */


        /* Parameter adjustments */
        b_dim1 = nmax;
        b_offset = b_dim1 + 1;
        b -= b_offset;
        vect_dim1 = nmax;
        vect_offset = vect_dim1 + 1;
        vect -= vect_offset;
        --root;
        --a;

        /* Function Body */
        eta = 1e-15;
        theta = 1e65;
/*      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
        i__1 = njx;
        for (i = 1; i <= i__1; ++i) {
        i__2 = nrootx;
        for (j = 1; j <= i__2; ++j) {
                vect[i + j * vect_dim1] = 0.;
/* L10: */
        }
/* L20: */
        }
        i__1 = nx;
        for (i = 1; i <= i__1; ++i) {
        for (j = 1; j <= 5; ++j) {
                b[i + j * b_dim1] = 0.;
/* L30: */
        }
/* L40: */
    }

    del1 = eta / 100.;
/* Computing 2nd power */
    d__1 = eta;
    delta = d__1 * d__1 * 100.;
/* Computing 2nd power */
    d__1 = eta;
    small = d__1 * d__1 / 100.;
    delbig = theta * delta / 1e3;
    theta1 = .001 / theta;
/*      TOLER  IS A FACTOR USED TO DETERMINE IF TWO ROOTS ARE CLOSE */
/*      ENOUGH TO BE CONSIDERED DEGENERATE FOR PURPOSES OF ORTHOGONALI- */

/*      ZING THEIR VECTORS.  FOR THE MATRIX NORMED TO UNITY, IF THE */
/*      DIFFERENCE BETWEEN TWO ROOTS IS LESS THAN TOLER, THEN */
/*      ORTHOGONALIZATION WILL OCCUR. */
    toler = eta * 1e4;

/*      INITIAL VALUE FOR PSEUDORANDOM NUMBER GENERATOR... (2**23)-3 */
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
/*     NSIZE    NUMBER OF ELEMENTS IN THE PACKED ARRAY */
    nsize = n * (n + 1) / 2;
    nm1 = n - 1;
    nm2 = n - 2;

/*     SCALE MATRIX TO EUCLIDEAN NORM OF 1.  SCALE FACTOR IS ANORM. */
    factor = 0.;
    i__1 = nsize;
    for (i = 1; i <= i__1; ++i) {
        zz = a[i];
        zz = fabs(zz);
/* L70: */
        factor = sMax(factor,zz);
    }
    if (factor != 0.) {
        goto L100;
    }
/*     NULL MATRIX.  FIX UP ROOTS AND VECTORS, THEN EXIT. */
        i__1 = nroot;
    for (i = 1; i <= i__1; ++i) {
        if (nrootx < 0) {
            goto L90;
        }
        i__2 = n;
        for (j = 1; j <= i__2; ++j) {
/* L80: */
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
/* Computing 2nd power */
        d__1 = a[i] / factor;
        anorm += d__1 * d__1 / 2.;
        ++k;
        j += k;
        goto L120;
L110:
/* Computing 2nd power */
        d__1 = a[i] / factor;
        anorm += d__1 * d__1;
L120:
        ;
    }
    anorm = sqrt(anorm * 2.) * factor;
    i__1 = nsize;
        for (i = 1; i <= i__1; ++i) {
/* L130: */
        a[i] /= anorm;
    }
    alimit = 1.;

/*      TRIDIA SECTION. */
/*      TRIDIAGONALIZATION OF SYMMETRIC MATRIX */
    id = 0;
    ia = 1;
    if (nm2 == 0) {
        goto L240;
    }
    i__1 = nm2;
        for (j = 1; j <= i__1; ++j) {
/*      J       COUNTS ROW  OF A-MATRIX TO BE DIAGONALIZED */
/*      IA      START OF NON-CODIAGONAL ELEMENTS IN THE ROW */
/*      ID      INDEX OF CODIAGONAL ELEMENT ON ROW BEING CODIAGONALIZE
D. */
        ia = ia + j + 2;
        id = id + j + 1;
        jp2 = j + 2;
/*      SUM SQUARES OF NON-CODIAGONAL ELEMENTS IN ROW J */
        ii = ia;
        sum = 0.;
        i__2 = n;
        for (i = jp2; i <= i__2; ++i) {
/* Computing 2nd power */
            d__1 = a[ii];
            sum += d__1 * d__1;
/* L140: */
            ii += i;
        }
        temp = a[id];
        if (sum > small) {
            goto L150;
        }
/*      NO TRANSFORMATION NECESSARY IF ALL THE NON-CODIAGONAL */
/*      ELEMENTS ARE TINY. */
        b[j + b_dim1] = temp;
        a[id] = 0.;
        goto L230;
/*      NOW COMPLETE THE SUM OF OFF-DIAGONAL SQUARES */
L150:
/* Computing 2nd power */
        d__1 = temp;
        sum = sqrt(sum + d__1 * d__1);
/*      NEW CODIAGONAL ELEMENT */
        b[j + b_dim1] = -fabs(sum)*sSign(temp);
/*      FIRST NON-ZERO ELEMENT OF THIS W-VECTOR */
        b[j + 1 + (b_dim1 << 1)] = sqrt((fabs(temp) / sum + 1.) / 2.);
/*      FORM REST OF THE W-VECTOR ELEMENTS */
        d__1 = (float).5 / (b[j + 1 + (b_dim1 << 1)] * sum);
        temp = fabs(d__1)*sSign(temp);
        ii = ia;
        i__2 = n;
        for (i = jp2; i <= i__2; ++i) {
            b[i + (b_dim1 << 1)] = a[ii] * temp;
/* L160: */
            ii += i;
        }
/*      FORM P-VECTOR AND SCALAR.  P-VECTOR = A-MATRIX*W-VECTOR. */
/*      SCALAR = W-VECTOR*P-VECTOR. */
        ak = 0.;
/*      IC      LOCATION OF NEXT DIAGONAL ELEMENT */
        ic = id + 1;
        j1 = j + 1;
        i__2 = n;
        for (i = j1; i <= i__2; ++i) {
            jj = ic;
            temp = 0.;
            i__3 = n;
            for (ii = j1; ii <= i__3; ++ii) {
/*      I       RUNS OVER THE NON-ZERO P-ELEMENTS */
/*      II      RUNS OVER ELEMENTS OF W-VECTOR */
                temp += b[ii + (b_dim1 << 1)] * a[jj];
/*      CHANGE INCREMENTING MODE AT THE DIAGONAL ELEMENTS. */
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
/*      BUILD UP THE K-SCALAR (AK) */
            ak += temp * b[i + (b_dim1 << 1)];
            b[i + b_dim1] = temp;
/*      MOVE IC TO TOP OF NEXT A-MATRIX :ROW: */
/* L190: */
            ic += i;
        }
/*      FORM THE Q-VECTOR */
        i__2 = n;
        for (i = j1; i <= i__2; ++i) {
/* L200: */
            b[i + b_dim1] -= ak * b[i + (b_dim1 << 1)];
        }
/*      TRANSFORM THE REST OF THE A-MATRIX */
/*      JJ      START-1 OF THE REST OF THE A-MATRIX */
        jj = id;
/*      MOVE W-VECTOR INTO THE OLD A-MATRIX LOCATIONS TO SAVE SPACE */

/*      I       RUNS OVER THE SIGNIFICANT ELEMENTS OF THE W-VECTOR */
        i__2 = n;
        for (i = j1; i <= i__2; ++i) {
            a[jj] = b[i + (b_dim1 << 1)];
            i__3 = i;
            for (ii = j1; ii <= i__3; ++ii) {
                ++jj;
/* L210: */
                a[jj] -= (b[i + b_dim1] * b[ii + (b_dim1 << 1)] + b[i + (
                        b_dim1 << 1)] * b[ii + b_dim1]) * 2.;
            }
/* L220: */
            jj += j;
        }
L230:
        ;
    }
/*      MOVE LAST CODIAGONAL ELEMENT OUT INTO ITS PROPER PLACE */
L240:
    b[nm1 + b_dim1] = a[nsize - 1];
    a[nsize - 1] = 0.;

/*     STURM SECTION. */
/*     STURM SEQUENCE ITERATION TO OBTAIN ROOTS OF TRIDIAGONAL FORM. */
/*     MOVE DIAGONAL ELEMENTS INTO SECOND N ELEMENTS OF B-VECTOR. */
/*     THIS IS A MORE CONVENIENT INDEXING POSITION. */
/*     ALSO, PUT SQUARE OF CODIAGONAL ELEMENTS IN THIRD N ELEMENTS. */
    jump = 1;
    i__1 = n;
        for (j = 1; j <= i__1; ++j) {
        b[j + (b_dim1 << 1)] = a[jump];
/* Computing 2nd power */
        d__1 = b[j + b_dim1];
        b[j + b_dim1 * 3] = d__1 * d__1;
/* L250: */
        jump = jump + j + 1;
    }
    i__1 = nroot;
    for (i = 1; i <= i__1; ++i) {
/* L260: */
        root[i] = alimit;
    }
    rootl = -alimit;
/*     ISOLATE THE ROOTS.  THE NROOT LOWEST ROOTS ARE FOUND, LOWEST FIRST 
*/
    i__1 = nroot;
    for (i = 1; i <= i__1; ++i) {
/*     FIND CURRENT :BEST: UPPER BOUND */
        rootx = alimit;
        i__2 = nroot;
        for (j = i; j <= i__2; ++j) {
/* L270: */
/* Computing MIN */
            d__1 = rootx, d__2 = root[j];
            rootx = sMin(d__1,d__2);
        }
        root[i] = rootx;
/*     GET IMPROVED TRIAL ROOT */
L280:
        trial = (rootl + root[i]) * .5;
        if (trial == rootl || trial == root[i]) {
            goto L340;
        }
/*     FORM STURM SEQUENCE RATIOS, USING ORTEGA:S ALGORITHM (MODIFIED)
. */
/*     NOMTCH IS THE NUMBER OF ROOTS LESS THAN THE TRIAL VALUE. */
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
/*     SINCE MATRIX IS NORMED TO UNITY, MAGNITUDE OF B(J,3) IS LESS TH
AN */
/*     ONE, SO OVERFLOW IS NOT POSSIBLE AT THE DIVISION STEP, SINCE */

/*     F0 IS GREATER THAN THETA1. */
        f0 = b[j + (b_dim1 << 1)] - trial - b[j - 1 + b_dim1 * 3] / f0;
        goto L300;
L310:
        j += 2;
        --nomtch;
        if (j <= n) {
            goto L290;
        }
L320:
/*     FIX NEW BOUNDS ON ROOTS */
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
/*     REVERSE THE ORDER OF THE EIGENVALUES, SINCE CUSTOM DICTATES */
/*     :LARGEST FIRST:.  THIS SECTION MAY BE REMOVED IF DESIRED WITHOUT */

/*     AFFECTING THE REMAINDER OF THE ROUTINE. */
    nrt = nroot / 2;
    i__1 = nrt;
        for (i = 1; i <= i__1; ++i) {
        save = root[i];
        nmip1 = nroot - i + 1;
        root[i] = root[nmip1];
/* L350: */
        root[nmip1] = save;
    }
/*     TRIVEC SECTION. */
/*     EIGENVECTORS OF CODIAGONAL FORM */
/*     QUIT NOW IF NO VECTORS WERE REQUESTED. */
        if (nrootx < 0) {
        goto L680;
    }
/*     INITIALIZE VECTOR ARRAY. */
        i__1 = n;
    for (i = 1; i <= i__1; ++i) {
        i__2 = nroot;
        for (j = 1; j <= i__2; ++j) {
/* L360: */
            vect[i + j * vect_dim1] = 1.;
        }
    }
    i__2 = nroot;
    for (i = 1; i <= i__2; ++i) {
        aroot = root[i];
/*     ORTHOGONALIZE IF ROOTS ARE CLOSE. */
        if (i == 1) {
            goto L370;
        }
/*     THE ABSOLUTE VALUE IN THE NEXT TEST IS TO ASSURE THAT THE TRIVE
C */
/*     SECTION IS INDEPENDENT OF THE ORDER OF THE EIGENVALUES. */
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
/*     GET THE CORRECT PIVOT EQUATION FOR THIS STEP. */
            if (fabs(elim1) <= (d__1 = b[j + b_dim1], fabs(d__1))) {
                goto L390;
            }
/*     FIRST (ELIM1) EQUATION IS THE PIVOT THIS TIME.  CASE 1. */
            b[j + (b_dim1 << 1)] = elim1;
            b[j + b_dim1 * 3] = elim2;
            b[j + (b_dim1 << 2)] = 0.;
            temp = b[j + b_dim1] / elim1;
                elim1 = a[jump] - aroot - temp * elim2;
            elim2 = b[j + 1 + b_dim1];
            goto L400;
/*     SECOND EQUATION IS THE PIVOT THIS TIME.  CASE 2. */
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
/*     SAVE FACTOR FOR SECOND ITERATION. */
L400:
            b[j + b_dim1 * 5] = temp;
/* L410: */
        }
        b[n + (b_dim1 << 1)] = elim1;
        b[n + b_dim1 * 3] = 0.;
        b[n + (b_dim1 << 2)] = 0.;
        b[nm1 + (b_dim1 << 2)] = 0.;
        iter = 1;
        if (ia != 0) {
            goto L510;
        }
/*     BACK SUBSTITUTE TO GET THIS VECTOR. */
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
/*     IF OVERFLOW IS CONCEIVABLE, SCALE THE VECTOR DOWN. */
/*     THIS APPROACH IS USED TO AVOID MACHINE-DEPENDENT AND SYSTEM
- */
/*     DEPENDENT CALLS TO OVERFLOW ROUTINES. */
            if (fabs(elim1) > delbig) {
                goto L440;
            }
            temp = b[l + (b_dim1 << 1)];
            if ((d__1 = b[l + (b_dim1 << 1)], fabs(d__1)) < delta) {
                temp = delta;
            }
            vect[l + i * vect_dim1] = elim1 / temp;
            goto L460;
/*     VECTOR IS TOO BIG.  SCALE IT DOWN. */
L440:
            i__3 = n;
            for (k = 1; k <= i__3; ++k) {
/* L450: */
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
/*     SECOND ITERATION.  (BOTH ITERATIONS FOR REPEATED-ROOT VECTORS).
 */
L470:
        ++iter;
L480:
        elim1 = vect[i * vect_dim1 + 1];
        i__1 = nm1;
        for (j = 1; j <= i__1; ++j) {
            if (b[j + (b_dim1 << 1)] == b[j + b_dim1]) {
                goto L490;
            }
/*     CASE ONE. */
                vect[j + i * vect_dim1] = elim1;
            elim1 = vect[j + 1 + i * vect_dim1] - elim1 * b[j + b_dim1 * 5];
            goto L500;
/*     CASE TWO. */
L490:
                vect[j + i * vect_dim1] = vect[j + 1 + i * vect_dim1];
            elim1 -= vect[j + 1 + i * vect_dim1] * temp;
L500:
            ;
        }
        vect[n + i * vect_dim1] = elim1;
        goto L420;
/*     PRODUCE A RANDOM VECTOR */
L510:
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
/*     GENERATE PSEUDORANDOM NUMBERS WITH UNIFORM DISTRIBUTION IN 
(-1,1). */
/*     THIS RANDOM NUMBER SCHEME IS OF THE FORM... */
/*     RAND1 = AMOD((2**12+3)*RAND1,2**23) */
/*     IT HAS A PERIOD OF 2**21 NUMBERS. */
            aaaa = rand1 * 4099.;
            bbbb = 8388608.;
                iiii = (idx) (aaaa / bbbb);
                cccc = (real) iiii;
                rand1 = aaaa - cccc * bbbb;
/* L520: */
            vect[j + i * vect_dim1] = rand1 / 4194304. - 1.;
        }
        goto L420;

/*     ORTHOGONALIZE THIS REPEATED-ROOT VECTOR TO OTHERS WITH THIS ROO
T. */
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
/* L540: */
                temp += vect[j + i * vect_dim1] * vect[j + k * vect_dim1];
                }
            i__3 = n;
            for (j = 1; j <= i__3; ++j) {
/* L550: */
                vect[j + i * vect_dim1] -= temp * vect[j + k * vect_dim1];
            }
/* L560: */
        }
L570:
        switch ((idx)iter) {
            case 1:  goto L480;
            case 2:  goto L580;
        }
/*     NORMALIZE THE VECTOR */
L580:
        elim1 = 0.;
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
/* L590: */
/* Computing MAX */
            d__2 = (d__1 = vect[j + i * vect_dim1], fabs(d__1));
            elim1 = sMax(d__2,elim1);
        }
        temp = 0.;
        i__1 = n;
        for (j = 1; j <= i__1; ++j) {
            elim2 = vect[j + i * vect_dim1] / elim1;
/* L600: */
/* Computing 2nd power */
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
/* L610: */
        }
/* L620: */
    }

/*      SIMVEC SECTION. */
/*      ROTATE CODIAGONAL VECTORS INTO VECTORS OF ORIGINAL ARRAY */
/*      LOOP OVER ALL THE TRANSFORMATION VECTORS */
    if (nm2 == 0) {
        goto L680;
    }
    jump = nsize - (n + 1);
    im = nm1;
    i__2 = nm2;
    for (i = 1; i <= i__2; ++i) {
        j1 = jump;
/*      MOVE A TRANSFORMATION VECTOR OUT INTO BETTER INDEXING POSITION
. */
        i__1 = n;
        for (j = im; j <= i__1; ++j) {
            b[j + (b_dim1 << 1)] = a[j1];
/* L630: */
            j1 += j;
        }
/*      MODIFY ALL REQUESTED VECTORS. */
        i__1 = nroot;
        for (k = 1; k <= i__1; ++k) {
            temp = 0.;
/*      FORM SCALAR PRODUCT OF TRANSFORMATION VECTOR WITH EIGENVEC
TOR */
            i__3 = n;
            for (j = im; j <= i__3; ++j) {
/* L640: */
                temp += b[j + (b_dim1 << 1)] * vect[j + k * vect_dim1];
            }
                temp += temp;
            i__3 = n;
            for (j = im; j <= i__3; ++j) {
/* L650: */
                vect[j + k * vect_dim1] -= temp * b[j + (b_dim1 << 1)];
            }
/* L660: */
        }
        jump -= im;
/* L670: */
        --im;
        }
L680:
/*      RESTORE ROOTS TO THEIR PROPER SIZE. */
    i__2 = nroot;
    for (i = 1; i <= i__2; ++i) {
/* L690: */
        root[i] *= anorm;
    }
L700:
        return 0;
} /* givens_ */

