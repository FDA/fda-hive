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
#include <slib/std/string.hpp>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <wctype.h>
#include <sys/types.h>
#include <wchar.h>

using namespace slib;

#define    BUF        513    /* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define    LONG        0x01    /* l: long or double */
#define    LONGDBL        0x02    /* L: long double */
#define    SHORT        0x04    /* h: short */
#define    SUPPRESS    0x08    /* *: suppress assignment */
#define    POINTER        0x10    /* p: void * (as hex) */
#define    NOSKIP        0x20    /* [ or c: do not skip blanks */
#define    LONGLONG    0x400    /* ll: long long (+ deprecated q: quad) */
#define    INTMAXT        0x800    /* j: intmax_t */
#define    PTRDIFFT    0x1000    /* t: ptrdiff_t */
#define    SIZET        0x2000    /* z: size_t */
#define    SHORTSHORT    0x4000    /* hh: char */
#define    UNSIGNED    0x8000    /* %[oupxX] conversions */

/*
 * The following are used in integral conversions only:
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS
 */
#define    SIGNOK        0x40    /* +/- is (still) legal */
#define    NDIGITS        0x80    /* no digits detected */
#define    PFXOK        0x100    /* 0x prefix is (still) legal */
#define    NZDIGITS    0x200    /* no zero digits detected */
#define    HAVESIGN    0x10000    /* sign detected */

/*
 * Conversion types.
 */
#define    CT_CHAR        0    /* %c conversion */
#define    CT_CCL        1    /* %[...] conversion */
#define    CT_STRING    2    /* %s conversion */
#define    CT_INT        3    /* %[dioupxX] conversion */
#define    CT_FLOAT    4    /* %[efgEFG] conversion */

static const char *__sccl(char *, const char *);
static idx parsefloat(const char **buf, const char *bufend, char *tempbuf, char *tempend); //, locale_t);

idx sString::bufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, ...)
{
    va_list marker;
    idx ret;
    va_start(marker, formatDescription);
    ret = vbufscanf(textScan, textScanEnd, formatDescription, marker);
    va_end(marker);
    return ret;
}

idx sString::vbufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, va_list marker)
{
    int c;            /* character from format, or conversion */
    size_t width;        /* field width, or 0 */
    char *p;        /* points into all kinds of strings */
    unsigned int n;            /* handy integer */
    int flags;        /* flags as defined above */
    char *p0;        /* saves original value of p when necessary */
    int nassigned;        /* number of fields assigned */
    int nconversions;    /* number of conversions */
    int nread;        /* number of characters consumed from buf */
    int base = 0;        /* base argument to conversion function */
    char ccltab[256];    /* character class table for %[...] */
    char tempbuf[BUF];        /* buffer for numeric and mb conversions */
    wchar_t *wcp;        /* handy wide character pointer */
    size_t nconv;        /* length of multibyte sequence converted */
    mbstate_t mbs;

    /* `basefix' is used to avoid `if' tests in the integer scanner */
    static short basefix[17] =
        { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    if (!textScan)
        return 0;

    if (!textScanEnd)
        textScanEnd = textScan + strlen(textScan);

    nassigned = 0;
    nconversions = 0;
    nread = 0;
    for (;;) {
        c = *formatDescription++;
        if (c == 0)
            return (nassigned);
        if (isspace(c)) {
            while (textScan < textScanEnd && isspace(*textScan))
                nread++, textScan++;
            continue;
        }
        if (c != '%')
            goto literal;
        width = 0;
        flags = 0;
        /*
         * switch on the format.  continue if done;
         * break once format type is derived.
         */
again:        c = *formatDescription++;
        switch (c) {
        case '%':
literal:
            if (textScan >= textScanEnd)
                goto input_failure;
            if (*textScan != c)
                goto match_failure;
            textScan++;
            nread++;
            continue;

        case '*':
            flags |= SUPPRESS;
            goto again;
        case 'j':
            flags |= INTMAXT;
            goto again;
        case 'l':
            if (flags & LONG) {
                flags &= ~LONG;
                flags |= LONGLONG;
            } else
                flags |= LONG;
            goto again;
        case 'q':
            flags |= LONGLONG;    /* not quite */
            goto again;
        case 't':
            flags |= PTRDIFFT;
            goto again;
        case 'z':
            flags |= SIZET;
            goto again;
        case 'L':
            flags |= LONGDBL;
            goto again;
        case 'h':
            if (flags & SHORT) {
                flags &= ~SHORT;
                flags |= SHORTSHORT;
            } else
                flags |= SHORT;
            goto again;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            width = width * 10 + c - '0';
            goto again;

        /*
         * Conversions.
         */
        case 'd':
            c = CT_INT;
            base = 10;
            break;

        case 'i':
            c = CT_INT;
            base = 0;
            break;

        case 'o':
            c = CT_INT;
            flags |= UNSIGNED;
            base = 8;
            break;

        case 'u':
            c = CT_INT;
            flags |= UNSIGNED;
            base = 10;
            break;

        case 'X':
        case 'x':
            flags |= PFXOK;    /* enable 0x prefixing */
            c = CT_INT;
            flags |= UNSIGNED;
            base = 16;
            break;

        case 'A': case 'E': case 'F': case 'G':
        case 'a': case 'e': case 'f': case 'g':
            c = CT_FLOAT;
            break;

        case 'S':
            flags |= LONG;
            /* FALLTHROUGH */
        case 's':
            c = CT_STRING;
            break;

        case '[':
            formatDescription = __sccl(ccltab, formatDescription);
            flags |= NOSKIP;
            c = CT_CCL;
            break;

        case 'C':
            flags |= LONG;
            /* FALLTHROUGH */
        case 'c':
            flags |= NOSKIP;
            c = CT_CHAR;
            break;

        case 'p':    /* pointer format is like hex */
            flags |= POINTER | PFXOK;
            c = CT_INT;        /* assumes sizeof(uintmax_t) */
            flags |= UNSIGNED;    /*      >= sizeof(uintptr_t) */
            base = 16;
            break;

        case 'n':
            nconversions++;
            if (flags & SUPPRESS)    /* ??? */
                continue;
            if (flags & SHORTSHORT)
                *va_arg(marker, char *) = nread;
            else if (flags & SHORT)
                *va_arg(marker, short *) = nread;
            else if (flags & LONG)
                *va_arg(marker, long *) = nread;
            else if (flags & LONGLONG)
                *va_arg(marker, long long *) = nread;
            else if (flags & INTMAXT)
                *va_arg(marker, intmax_t *) = nread;
            else if (flags & SIZET)
                *va_arg(marker, size_t *) = nread;
            else if (flags & PTRDIFFT)
                *va_arg(marker, ptrdiff_t *) = nread;
            else
                *va_arg(marker, int *) = nread;
            continue;

        default:
            goto match_failure;

        /*
         * Disgusting backwards compatibility hack.    XXX
         */
        case '\0':    /* compat */
            return (EOF);
        }

        /*
         * We have a conversion that requires input.
         */
        if (textScan >= textScanEnd)
            goto input_failure;

        /*
         * Consume leading white space, except for formats
         * that suppress this.
         */
        if ((flags & NOSKIP) == 0) {
            while (isspace(*textScan)) {
                nread++;
                if (textScan < textScanEnd)
                    textScan++;
                else
                    goto input_failure;
            }
            /*
             * Note that there is at least one character in
             * the buffer, so conversions that do not set NOSKIP
             * ca no longer result in an input failure.
             */
        }

        /*
         * Do the conversion.
         */
        switch (c) {

        case CT_CHAR:
            /* scan arbitrary characters (sets NOSKIP) */
            if (width == 0)
                width = 1;
            if (flags & LONG) {
                if ((flags & SUPPRESS) == 0)
                    wcp = va_arg(marker, wchar_t *);
                else
                    wcp = NULL;
                n = 0;
                while (width != 0) {
                    if (n == MB_CUR_MAX)
                        goto input_failure;

                    tempbuf[n++] = *textScan++;
                    memset (&mbs,0,sizeof(mbs));
                    nconv = mbrtowc(wcp, tempbuf, n, &mbs);
                    if (nconv == (size_t)-1)
                        goto input_failure;

                    if (nconv == 0 && !(flags & SUPPRESS))
                        *wcp = L'\0';
                    if (nconv != (size_t)-2) {
                        nread += n;
                        width--;
                        if (!(flags & SUPPRESS))
                            wcp++;
                        n = 0;
                    }
                    if (textScan >= textScanEnd) {
                        if (n != 0)
                            goto input_failure;
                        break;
                    }
                }
                if (!(flags & SUPPRESS))
                    nassigned++;
            } else if (flags & SUPPRESS) {
                size_t sum = 0;
                for (;;) {
                    if ((n = textScanEnd - textScan - 1) < width) {
                        sum += n;
                        width -= n;
                        textScan += n;
                        if (textScan >= textScanEnd) {
                            if (sum == 0)
                                goto input_failure;
                            break;
                        }
                    } else {
                        sum += width;
                        textScan += width;
                        break;
                    }
                }
                nread += sum;
            } else {
                if (textScan + width >= textScanEnd)
                    goto input_failure;
                memcpy((void *)va_arg(marker, char *), textScan, width);
                textScan += width;
                nread += width;
                nassigned++;
            }
            nconversions++;
            break;

        case CT_CCL:
            /* scan a (nonempty) character class (sets NOSKIP) */
            if (width == 0)
                width = (size_t)~0;    /* `infinity' */
            /* take only those things in the class */
            if (flags & LONG) {
                wchar_t twc;
                unsigned int nchars;

                if ((flags & SUPPRESS) == 0)
                    wcp = va_arg(marker, wchar_t *);
                else
                    wcp = &twc;
                n = 0;
                nchars = 0;
                while (width != 0) {
                    if (n == MB_CUR_MAX)
                        goto input_failure;
                    tempbuf[n++] = *textScan++;
                    memset (&mbs,0,sizeof(mbs));
                    nconv = mbrtowc(wcp, textScan, n, &mbs);
                    if (nconv == (size_t)-1)
                        goto input_failure;
                    if (nconv == 0)
                        *wcp = L'\0';
                    if (nconv != (size_t)-2) {
                        if (wctob(*wcp) != EOF &&
                            !ccltab[wctob(*wcp)]) {
                            while (n != 0) {
                                n--;
                                textScan--;
                            }
                            break;
                        }
                        nread += n;
                        width--;
                        if (!(flags & SUPPRESS))
                            wcp++;
                        nchars++;
                        n = 0;
                    }
                    if (textScan >= textScanEnd) {
                        if (n != 0)
                            goto input_failure;
                        break;
                    }
                }
                if (n != 0)
                    goto input_failure;
                n = nchars;
                if (n == 0)
                    goto match_failure;
                if (!(flags & SUPPRESS)) {
                    *wcp = L'\0';
                    nassigned++;
                }
            } else if (flags & SUPPRESS) {
                n = 0;
                while (ccltab[(int)*textScan]) {
                    n++;
                    textScan++;
                    if (--width == 0)
                        break;
                    if (textScan >= textScanEnd) {
                        if (n == 0)
                            goto input_failure;
                        break;
                    }
                }
                if (n == 0)
                    goto match_failure;
            } else {
                p0 = p = va_arg(marker, char *);
                while (ccltab[(int)*textScan]) {
                    *p++ = *textScan++;
                    if (--width == 0)
                        break;
                    if (textScan >= textScanEnd) {
                        if (p == p0)
                            goto input_failure;
                        break;
                    }
                }
                n = p - p0;
                if (n == 0)
                    goto match_failure;
                *p = 0;
                nassigned++;
            }
            nread += n;
            nconversions++;
            break;

        case CT_STRING:
            /* like CCL, but zero-length string OK, & no NOSKIP */
            if (width == 0)
                width = (size_t)~0;
            if (flags & LONG) {
                wchar_t twc;

                if ((flags & SUPPRESS) == 0)
                    wcp = va_arg(marker, wchar_t *);
                else
                    wcp = &twc;
                n = 0;
                while (!isspace(*textScan) && width != 0) {
                    if (n == MB_CUR_MAX)
                        goto input_failure;
                    tempbuf[n++] = *textScan++;
                    memset (&mbs,0,sizeof(mbs));
                    nconv = mbrtowc(wcp, tempbuf, n, &mbs);
                    if (nconv == (size_t)-1)
                        goto input_failure;
                    if (nconv == 0)
                        *wcp = L'\0';
                    if (nconv != (size_t)-2) {
                        if (iswspace(*wcp)) {
                            while (n != 0) {
                                n--;
                                textScan--;
                            }
                            break;
                        }
                        nread += n;
                        width--;
                        if (!(flags & SUPPRESS))
                            wcp++;
                        n = 0;
                    }
                    if (textScan >= textScanEnd) {
                        if (n != 0)
                            goto input_failure;
                        break;
                    }
                }
                if (!(flags & SUPPRESS)) {
                    *wcp = L'\0';
                    nassigned++;
                }
            } else if (flags & SUPPRESS) {
                n = 0;
                while (!isspace(*textScan)) {
                    n++;
                    textScan++;
                    if (--width == 0)
                        break;
                    if (textScan >= textScanEnd)
                        break;
                }
                nread += n;
            } else {
                p0 = p = va_arg(marker, char *);
                while (!isspace(*textScan)) {
                    *p++ = *textScan++;
                    if (--width == 0)
                        break;
                    if (textScan >= textScanEnd)
                        break;
                }
                *p = 0;
                nread += p - p0;
                nassigned++;
            }
            nconversions++;
            continue;

        case CT_INT:
            /* scan an integer as if by the conversion function */
#ifdef hardway
            if (width == 0 || width > sizeof(tempbuf) - 1)
                width = sizeof(tempbuf) - 1;
#else
            /* size_t is unsigned, hence this optimisation */
            if (--width > sizeof(tempbuf) - 2)
                width = sizeof(tempbuf) - 2;
            width++;
#endif
            flags |= SIGNOK | NDIGITS | NZDIGITS;
            for (p = tempbuf; width; width--) {
                c = *textScan;
                /*
                 * Switch on the character; `goto ok'
                 * if we accept it as a part of number.
                 */
                switch (c) {

                /*
                 * The digit 0 is always legal, but is
                 * special.  For %i conversions, if no
                 * digits (zero or nonzero) have been
                 * scanned (only signs), we will have
                 * base==0.  In that case, we should set
                 * it to 8 and enable 0x prefixing.
                 * Also, if we have not scanned zero digits
                 * before this, do not turn off prefixing
                 * (someone else will turn it off if we
                 * have scanned any nonzero digits).
                 */
                case '0':
                    if (base == 0) {
                        base = 8;
                        flags |= PFXOK;
                    }
                    if (flags & NZDIGITS)
                        flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
                    else
                        flags &= ~(SIGNOK|PFXOK|NDIGITS);
                    goto ok;

                /* 1 through 7 always legal */
                case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    base = basefix[base];
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* digits 8 and 9 ok iff decimal or hex */
                case '8': case '9':
                    base = basefix[base];
                    if (base <= 8)
                        break;    /* not legal here */
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* letters ok iff hex */
                case 'A': case 'B': case 'C':
                case 'D': case 'E': case 'F':
                case 'a': case 'b': case 'c':
                case 'd': case 'e': case 'f':
                    /* no need to fix base here */
                    if (base <= 10)
                        break;    /* not legal here */
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                /* sign ok only as first character */
                case '+': case '-':
                    if (flags & SIGNOK) {
                        flags &= ~SIGNOK;
                        flags |= HAVESIGN;
                        goto ok;
                    }
                    break;
                    
                /*
                 * x ok iff flag still set & 2nd char (or
                 * 3rd char if we have a sign).
                 */
                case 'x': case 'X':
                    if (flags & PFXOK && p ==
                        tempbuf + 1 + !!(flags & HAVESIGN)) {
                        base = 16;    /* if %i */
                        flags &= ~PFXOK;
                        goto ok;
                    }
                    break;
                }

                /*
                 * If we got here, c is not a legal character
                 * for a number.  Stop accumulating digits.
                 */
                break;
        ok:
                /*
                 * c is legal: store it and look at the next.
                 */
                *p++ = c;
                if (textScan < textScanEnd)
                    textScan++;
                else
                    break;        /* EOF */
            }
            /*
             * If we had only a sign, it is no good; push
             * back the sign.  If the number ends in `x',
             * it was [sign] '0' 'x', so push back the x
             * and treat it as [sign] '0'.
             */
            if (flags & NDIGITS) {
                if (p > tempbuf) {
                    textScan--;
                    p--;
                }
                goto match_failure;
            }
            c = ((u_char *)p)[-1];
            if (c == 'x' || c == 'X') {
                p--;
                textScan--;
            }
            if ((flags & SUPPRESS) == 0) {
                uintmax_t res;

                *p = 0;
                if ((flags & UNSIGNED) == 0)
                    res = strtoimax(tempbuf, (char **)NULL, base);
                else
                    res = strtoumax(tempbuf, (char **)NULL, base);
                if (flags & POINTER)
                    *va_arg(marker, void **) =
                            (void *)(uintptr_t)res;
                else if (flags & SHORTSHORT)
                    *va_arg(marker, char *) = res;
                else if (flags & SHORT)
                    *va_arg(marker, short *) = res;
                else if (flags & LONG)
                    *va_arg(marker, long *) = res;
                else if (flags & LONGLONG)
                    *va_arg(marker, long long *) = res;
                else if (flags & INTMAXT)
                    *va_arg(marker, intmax_t *) = res;
                else if (flags & PTRDIFFT)
                    *va_arg(marker, ptrdiff_t *) = res;
                else if (flags & SIZET)
                    *va_arg(marker, size_t *) = res;
                else
                    *va_arg(marker, int *) = res;
                nassigned++;
            }
            nread += p - tempbuf;
            nconversions++;
            break;

        case CT_FLOAT:
            /* scan a floating point number as if by strtod */
            if (width == 0 || width > sizeof(tempbuf) - 1)
                width = sizeof(tempbuf) - 1;
            if ((width = parsefloat(&textScan, textScanEnd, tempbuf, tempbuf + width)) == 0)
                goto match_failure;
            if ((flags & SUPPRESS) == 0) {
                if (flags & LONGDBL) {
                    long double res = strtold(tempbuf, &p);
                    *va_arg(marker, long double *) = res;
                } else if (flags & LONG) {
                    double res = strtod(tempbuf, &p);
                    *va_arg(marker, double *) = res;
                } else {
                    float res = strtof(tempbuf, &p);
                    *va_arg(marker, float *) = res;
                }
                nassigned++;
            }
            nread += width;
            nconversions++;
            break;
        }
    }
input_failure:
    return (nconversions != 0 ? nassigned : EOF);
match_failure:
    return (nassigned);
}

/*
 * Fill in the given table from the scanset at the given format
 * (just after `[').  Return a pointer to the character past the
 * closing `]'.  The table has a 1 wherever characters should be
 * considered part of the scanset.
 */
static const char *
__sccl(char *tab, const char *formatDescription)
{
    int c, n, v;

    /* first `clear' the whole table */
    c = *formatDescription++;        /* first char hat => negated scanset */
    if (c == '^') {
        v = 1;        /* default => accept */
        c = *formatDescription++;    /* get new first char */
    } else
        v = 0;        /* default => reject */

    /* XXX: Will not work if sizeof(tab*) > sizeof(char) */
    (void) memset(tab, v, 256);

    if (c == 0)
        return (formatDescription - 1);/* format ended before closing ] */

    /*
     * Now set the entries corresponding to the actual scanset
     * to the opposite of the above.
     *
     * The first character may be ']' (or '-') without being special;
     * the last character may be '-'.
     */
    v = 1 - v;
    for (;;) {
        tab[c] = v;        /* take character c */
doswitch:
        n = *formatDescription++;        /* and examine the next */
        switch (n) {

        case 0:            /* format ended too soon */
            return (formatDescription - 1);

        case '-':
            /*
             * A scanset of the form
             *    [01+-]
             * is defined as `the digit 0, the digit 1,
             * the character +, the character -', but
             * the effect of a scanset such as
             *    [a-zA-Z0-9]
             * is implementation defined.  The V7 Unix
             * scanf treats `a-z' as `the letters a through
             * z', but treats `a-a' as `the letter a, the
             * character -, and the letter a'.
             *
             * For compatibility, the `-' is not considerd
             * to define a range if the character following
             * it is either a close bracket (required by ANSI)
             * or is not numerically greater than the character
             * we just stored in the table (c).
             */
            n = *formatDescription;
            if (n == ']' || n < c) {
                c = '-';
                break;    /* resume the for(;;) */
            }
            formatDescription++;
            /* fill in the range */
            do {
                tab[++c] = v;
            } while (c < n);
#if 1    /* XXX another disgusting compatibility hack */
            c = n;
            /*
             * Alas, the V7 Unix scanf also treats formats
             * such as [a-c-e] as `the letters a through e'.
             * This too is permitted by the standard....
             */
            goto doswitch;
#else
            c = *formatDescription++;
            if (c == 0)
                return (formatDescription - 1);
            if (c == ']')
                return (formatDescription);
#endif
            break;

        case ']':        /* end of scanset */
            return (formatDescription);

        default:        /* just another character */
            c = n;
            break;
        }
    }
    /* NOTREACHED */
}

static idx
parsefloat(const char **ptextScan, const char *textScanEnd, char *tempbuf, char *tempend) //, locale_t locale)
{
    char *commit, *p;
    int infnanpos = 0, decptpos = 0;
    enum {
        S_START, S_GOTSIGN, S_INF, S_NAN, S_DONE, S_MAYBEHEX,
        S_DIGITS, S_DECPT, S_FRAC, S_EXP, S_EXPDIGITS
    } state = S_START;
    unsigned char c;
    const char *decpt = "."; // localeconv_l(locale)->decimal_point;
    bool gotmantdig = 0, ishex = 0;

    /*
     * We set commit = p whenever the string we have read so far
     * constitutes a valid representation of a floating point
     * number by itself.  At some point, the parse will complete
     * or fail, and we will ungetc() back to the last commit point.
     * To ensure that the file offset gets updated properly, it is
     * always necessary to read at least one character that doesn't
     * match; thus, we can't short-circuit "infinity" or "nan(...)".
     */
    commit = tempbuf - 1;
    for (p = tempbuf; p < tempend; ) {
        c = **ptextScan;
reswitch:
        switch (state) {
        case S_START:
            state = S_GOTSIGN;
            if (c == '-' || c == '+')
                break;
            else
                goto reswitch;
        case S_GOTSIGN:
            switch (c) {
            case '0':
                state = S_MAYBEHEX;
                commit = p;
                break;
            case 'I':
            case 'i':
                state = S_INF;
                break;
            case 'N':
            case 'n':
                state = S_NAN;
                break;
            default:
                state = S_DIGITS;
                goto reswitch;
            }
            break;
        case S_INF:
            if (infnanpos > 6 ||
                (c != "nfinity"[infnanpos] &&
                 c != "NFINITY"[infnanpos]))
                goto parsedone;
            if (infnanpos == 1 || infnanpos == 6)
                commit = p;    /* inf or infinity */
            infnanpos++;
            break;
        case S_NAN:
            switch (infnanpos) {
            case 0:
                if (c != 'A' && c != 'a')
                    goto parsedone;
                break;
            case 1:
                if (c != 'N' && c != 'n')
                    goto parsedone;
                else
                    commit = p;
                break;
            case 2:
                if (c != '(')
                    goto parsedone;
                break;
            default:
                if (c == ')') {
                    commit = p;
                    state = S_DONE;
                } else if (!isalnum(c) && c != '_')
                    goto parsedone;
                break;
            }
            infnanpos++;
            break;
        case S_DONE:
            goto parsedone;
        case S_MAYBEHEX:
            state = S_DIGITS;
            if (c == 'X' || c == 'x') {
                ishex = 1;
                break;
            } else {    /* we saw a '0', but no 'x' */
                gotmantdig = 1;
                goto reswitch;
            }
        case S_DIGITS:
            if ((ishex && isxdigit(c)) || isdigit(c)) {
                gotmantdig = 1;
                commit = p;
                break;
            } else {
                state = S_DECPT;
                goto reswitch;
            }
        case S_DECPT:
            if (c == decpt[decptpos]) {
                if (decpt[++decptpos] == '\0') {
                    /* We read the complete decpt seq. */
                    state = S_FRAC;
                    if (gotmantdig)
                        commit = p;
                }
                break;
            } else if (!decptpos) {
                /* We didn't read any decpt characters. */
                state = S_FRAC;
                goto reswitch;
            } else {
                /*
                 * We read part of a multibyte decimal point,
                 * but the rest is invalid, so bail.
                 */
                goto parsedone;
            }
        case S_FRAC:
            if (((c == 'E' || c == 'e') && !ishex) ||
                ((c == 'P' || c == 'p') && ishex)) {
                if (!gotmantdig)
                    goto parsedone;
                else
                    state = S_EXP;
            } else if ((ishex && isxdigit(c)) || isdigit(c)) {
                commit = p;
                gotmantdig = 1;
            } else
                goto parsedone;
            break;
        case S_EXP:
            state = S_EXPDIGITS;
            if (c == '-' || c == '+')
                break;
            else
                goto reswitch;
        case S_EXPDIGITS:
            if (isdigit(c))
                commit = p;
            else
                goto parsedone;
            break;
        default:
            abort();
        }
        *p++ = c;
        if (textScanEnd - *ptextScan > 0)
            (*ptextScan)++;
        else
            break;    /* EOF */
    }

parsedone:
    while (commit < --p)
        (*ptextScan)--;
    *++commit = '\0';
    return (commit - tempbuf);
}

