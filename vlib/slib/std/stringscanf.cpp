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

#define    BUF        513

#define    LONG        0x01
#define    LONGDBL        0x02
#define    SHORT        0x04
#define    SUPPRESS    0x08
#define    POINTER        0x10
#define    NOSKIP        0x20
#define    LONGLONG    0x400
#define    INTMAXT        0x800
#define    PTRDIFFT    0x1000
#define    SIZET        0x2000
#define    SHORTSHORT    0x4000
#define    UNSIGNED    0x8000

#define    SIGNOK        0x40
#define    NDIGITS        0x80
#define    PFXOK        0x100
#define    NZDIGITS    0x200
#define    HAVESIGN    0x10000

#define    CT_CHAR        0
#define    CT_CCL        1
#define    CT_STRING    2
#define    CT_INT        3
#define    CT_FLOAT    4

static const char *__sccl(char *, const char *);
static idx parsefloat(const char **buf, const char *bufend, char *tempbuf, char *tempend);

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
    int c;
    size_t width;
    char *p;
    unsigned int n;
    int flags;
    char *p0;
    int nassigned;
    int nconversions;
    int nread;
    int base = 0;
    char ccltab[256];
    char tempbuf[BUF];
    wchar_t *wcp;
    size_t nconv;
    mbstate_t mbs;

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
            flags |= LONGLONG;
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
            flags |= PFXOK;
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
        case 'c':
            flags |= NOSKIP;
            c = CT_CHAR;
            break;

        case 'p':
            flags |= POINTER | PFXOK;
            c = CT_INT;
            flags |= UNSIGNED;
            base = 16;
            break;

        case 'n':
            nconversions++;
            if (flags & SUPPRESS)
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

        case '\0':
            return (EOF);
        }

        if (textScan >= textScanEnd)
            goto input_failure;

        if ((flags & NOSKIP) == 0) {
            while (isspace(*textScan)) {
                nread++;
                if (textScan < textScanEnd)
                    textScan++;
                else
                    goto input_failure;
            }
        }

        switch (c) {

        case CT_CHAR:
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
            if (width == 0)
                width = (size_t)~0;
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
#ifdef hardway
            if (width == 0 || width > sizeof(tempbuf) - 1)
                width = sizeof(tempbuf) - 1;
#else
            if (--width > sizeof(tempbuf) - 2)
                width = sizeof(tempbuf) - 2;
            width++;
#endif
            flags |= SIGNOK | NDIGITS | NZDIGITS;
            for (p = tempbuf; width; width--) {
                c = *textScan;
                switch (c) {

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

                case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    base = basefix[base];
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                case '8': case '9':
                    base = basefix[base];
                    if (base <= 8)
                        break;
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                case 'A': case 'B': case 'C':
                case 'D': case 'E': case 'F':
                case 'a': case 'b': case 'c':
                case 'd': case 'e': case 'f':
                    if (base <= 10)
                        break;
                    flags &= ~(SIGNOK | PFXOK | NDIGITS);
                    goto ok;

                case '+': case '-':
                    if (flags & SIGNOK) {
                        flags &= ~SIGNOK;
                        flags |= HAVESIGN;
                        goto ok;
                    }
                    break;
                    
                case 'x': case 'X':
                    if (flags & PFXOK && p ==
                        tempbuf + 1 + !!(flags & HAVESIGN)) {
                        base = 16;
                        flags &= ~PFXOK;
                        goto ok;
                    }
                    break;
                }

                break;
        ok:
                *p++ = c;
                if (textScan < textScanEnd)
                    textScan++;
                else
                    break;
            }
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

static const char *
__sccl(char *tab, const char *formatDescription)
{
    int c, n, v;

    c = *formatDescription++;
    if (c == '^') {
        v = 1;
        c = *formatDescription++;
    } else
        v = 0;

    (void) memset(tab, v, 256);

    if (c == 0)
        return (formatDescription - 1);

    v = 1 - v;
    for (;;) {
        tab[c] = v;
doswitch:
        n = *formatDescription++;
        switch (n) {

        case 0:
            return (formatDescription - 1);

        case '-':
            n = *formatDescription;
            if (n == ']' || n < c) {
                c = '-';
                break;
            }
            formatDescription++;
            do {
                tab[++c] = v;
            } while (c < n);
#if 1                c = n;
            goto doswitch;
#else
            c = *formatDescription++;
            if (c == 0)
                return (formatDescription - 1);
            if (c == ']')
                return (formatDescription);
#endif
            break;

        case ']':
            return (formatDescription);

        default:
            c = n;
            break;
        }
    }
}

static idx
parsefloat(const char **ptextScan, const char *textScanEnd, char *tempbuf, char *tempend)
{
    char *commit, *p;
    int infnanpos = 0, decptpos = 0;
    enum {
        S_START, S_GOTSIGN, S_INF, S_NAN, S_DONE, S_MAYBEHEX,
        S_DIGITS, S_DECPT, S_FRAC, S_EXP, S_EXPDIGITS
    } state = S_START;
    unsigned char c;
    const char *decpt = ".";
    bool gotmantdig = 0, ishex = 0;

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
                commit = p;
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
            } else {
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
                    state = S_FRAC;
                    if (gotmantdig)
                        commit = p;
                }
                break;
            } else if (!decptpos) {
                state = S_FRAC;
                goto reswitch;
            } else {
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
            break;
    }

parsedone:
    while (commit < --p)
        (*ptextScan)--;
    *++commit = '\0';
    return (commit - tempbuf);
}

