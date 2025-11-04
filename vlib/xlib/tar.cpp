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
#include <sys/stat.h>
#include <slib/std/string.hpp>
#include <xlib/tar.hpp>

using namespace slib;


#define VERSION "1.4"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef SEEK_SET
# define SEEK_SET 0
#endif

#ifdef _WEAK_POSIX
# ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE
# endif
#endif

#ifdef _POSIX_SOURCE
# include <sys/types.h>
# include <sys/stat.h>
# include <utime.h>
# ifdef _WEAK_POSIX
#  define mode_t int
# else
#  include <unistd.h>
# endif
#endif

#define WSIZE    32768
#define TSIZE    512
#define CR    13
#define LF    10

typedef unsigned char    Uchar_t;
typedef unsigned short    Ushort_t;
typedef unsigned long    Ulong_t;

typedef struct
{
    Uchar_t    magic[2];
    Uchar_t    compression;
    Uchar_t    flags;
    Uchar_t    mtime[4];
    Uchar_t    extraflags;
    Uchar_t    os;
} gzhdr_t;
#define MAGIC0    0x1f
#define MAGIC1    0x8b
#define DEFLATE    0x08
#define NAME    0x08

typedef struct
{
    char    filename[100];
    char    mode[8];
    char    owner[8];
    char    group[8];
    char    size[12];
    char    mtime[12];
    char    checksum[8];
    char    type;
    char    linkto[100];
    char    brand[8];
    char    ownername[32];
    char    groupname[32];
    char    devmajor[8];
    char    defminor[8];
    char    prefix[155];
    char    RESERVED[12];
} tar_t;
#define ISREGULAR(hdr)    ((hdr).type < '1' || (hdr).type > '6')

typedef struct huft {
    Uchar_t e;
    Uchar_t b;
    union {
        Ushort_t    n;
        struct huft    *t;
    } v;
} huft_t;

static int wp;
static Uchar_t slide[WSIZE];
#define error(desc)    {fprintf(stderr, "%s:%s", inname, (desc)); exit(1);}

static unsigned t_border[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static Ushort_t cplens[] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
static Ushort_t cplext[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99};
static Ushort_t cpdist[] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577};
static Ushort_t cpdext[] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
    12, 12, 13, 13};


static char    *inname;
static FILE    *infp;
static FILE    *outfp;
static Ulong_t    outsize;
static FILE    *tarfp;
static int    maketar;
static int    listing;
static int    quiet;
static int    verbose;
static int    force;
static int    abspath;
static int    convert;
static int    noname;
static char    **only;
static int    nonlys;
static int    didabs;

static Ulong_t bb;
static unsigned bk;

Ushort_t mask_bits[] = {
        0x0000,
        0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
        0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#define NEXTBYTE()  (Uchar_t)getc(infp)
#define NEEDBITS(n) {while(k<(n)){b|=((Ulong_t)NEXTBYTE())<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

static int lbits = 9;
static int dbits = 6;


#define BMAX    16
#define N_MAX    288


static unsigned hufts;


static FILE *createpath(char * name)
{
    FILE    *fp;
    int    i;

    if (!force && access(name, 0) == 0)
    {
        fprintf(stderr, "%s: exists, will not overwrite without \"-f\"\n", name);
        return NULL;
    }

    fp = fopen(name, convert ? "w" : "wb");
    if (fp)
        return fp;

    for (i = 0; name[i]; i++)
    {
        if (name[i] == '/')
        {
            name[i] = '\0';
            (void)mkdir(name, 0777);
            name[i] = '/';
        }
    }
    fp = fopen(name, convert ? "w" : "wb");
    if (!fp)
        perror(name);
    return fp;
}

static void linkorcopy(char * src,  char * dst, int sym)
{
    FILE    *fpsrc;
    FILE    *fpdst;
    int    c;

    fpsrc = fopen(src, "rb");
    if (!fpsrc)
    {
        perror(src);
        return;
    }

    fpdst = createpath(dst);
    if (!fpdst)
        return;

#ifdef _POSIX_SOURCE
# ifndef _WEAK_POSIX
    fclose(fpdst);
    unlink(dst);
    if (sym)
    {
        if (symlink(src, dst))
        {
            perror(dst);
        }
        fclose(fpsrc);
        return;
    }
    if (!link(src, dst))
    {
        fclose(fpsrc);
        return;
    }

    fpdst = fopen(dst, "wb");

# endif #endif 
    while ((c = getc(fpsrc)) != EOF)
        putc(c, fpdst);

    fclose(fpsrc);
    fclose(fpdst);

    printf("%s: copy instead of link\n", dst);
}

static void cvtwrite(Uchar_t * blk, Ulong_t size, FILE * fp)
{
    int    i, j;
    static Uchar_t mod[TSIZE];

    if (convert)
    {
        for (i = j = 0; i < (int)size; i++)
        {
            if (blk[i] == LF)
                mod[j++] = '\n';
            else if (blk[i] == CR && (i+1 >= (int)size || blk[i+1] == LF))
                ;
            else
                mod[j++] = blk[i];
        }
        size = j;
        blk = mod;
    }

    fwrite(blk, (size_t)size, sizeof(Uchar_t), fp);
}


static long checksum(tar_t * tblk, int sunny)
{
    long    sum;
    char    *scan;

    sum = 0L;
    for (scan = (char *)tblk; scan < tblk->checksum; scan++)
    {
        sum += (*scan) & 0xff;
        if (sunny && (*scan & 0x80) != 0)
            sum -= 256;
    }

    sum += ' ' * sizeof tblk->checksum;
    scan += sizeof tblk->checksum;

    for (; scan < (char *)tblk + sizeof *tblk; scan++)
    {
        sum += (*scan) & 0xff;
        if (sunny && (*scan & 0x80) != 0)
            sum -= 256;
    }

    return sum;
}



static int untar(Uchar_t * blk)
{
    static char    nbuf[256];
    static char    *name,*n2;
    static int    first = 1;
    long        sum;
    int        i;
    tar_t        tblk[1];

#ifdef _POSIX_SOURCE
    static mode_t        mode;
    static struct utimbuf    timestamp;
#endif

    tblk[0] = *(tar_t *)blk;

    if (outsize > TSIZE)
    {
        if (outfp)
            cvtwrite(blk, (Ulong_t)TSIZE, outfp);
        outsize -= TSIZE;
    }
    else if (outsize > 0)
    {
        if (outfp)
        {
            cvtwrite(blk, outsize, outfp);
            fclose(outfp);
            outfp = NULL;
#ifdef _POSIX_SOURCE
            utime(nbuf, &timestamp);
            chmod(nbuf, mode);
#endif
        }
        outsize = 0;
    }
    else if ((tblk)->filename[0] == '\0')
    {
        if (didabs)
            fprintf(stderr, "WARNING: Removed leading slashes because \"-p\" wasn't given.\n");
        return 0;
    }
    else
    {
    
        if ((tblk)->filename[99] != '\0'
         || ((tblk)->size[0] < '0'
            && (tblk)->size[0] != ' ')
         || (tblk)->size[0] > '9')
        {
            if (first)
            {
                fprintf(stderr, "%s: not a valid tar file\n", inname);
                return 2;
            }
            else
            {
                printf("WARNING: Garbage detected; preceding file may be damaged\n");
                return  2;
            }
        }

        memset(nbuf, 0, sizeof nbuf);
        name = nbuf;
        if ((tblk)->prefix[0])
        {
            strncpy(name, (tblk)->prefix, sizeof (tblk)->prefix);
            strcat(name, "/");
            strncat(name + strlen(name), (tblk)->filename,
                sizeof (tblk)->filename);
        }
        else
        {
            strncpy(name, (tblk)->filename,
                sizeof (tblk)->filename);
        }

        if (!abspath && (*name == '/' || *name == '\\'))
            didabs = 1;
        for (n2 = nbuf; *name; name++)
        {
            if (*name == '\\')
                *name = '/';
            if (*name != '/'
             || (abspath && n2 == nbuf)
             || (n2 != nbuf && n2[-1] != '/'))
                *n2++ = *name;
        }
        if (n2 == nbuf)
            *n2++ = '/';
        *n2 = '\0';

        for (sum = 0L, i = 0; i < (int)sizeof((tblk)->checksum); i++)
        {
            if ((tblk)->checksum[i] >= '0'
                        && (tblk)->checksum[i] <= '7')
                sum = sum * 8 + (tblk)->checksum[i] - '0';
        }
        if (sum != checksum(tblk, 0) && sum != checksum(tblk, 1))
        {
            if (!first)
                printf("WARNING: Garbage detected; preceding file may be damaged\n");
            fflush(stdout);
            fprintf(stderr, "%s: header has bad checksum for %s\n", inname, nbuf);
            exit(2);
        }

        first = 0;

        if (*nbuf && nbuf[strlen(nbuf) - 1] == '/')
            (tblk)->type = '5';

        for (outsize = 0L, i = 0; i < (int)sizeof((tblk)->size); i++)
        {
            if ((tblk)->size[i] >= '0' && (tblk)->size[i] <= '7')
                outsize = outsize * 8 + (tblk)->size[i] - '0';
        }

#ifdef _POSIX_SOURCE
        for (timestamp.modtime=0L, i=0; i < (int)sizeof((tblk)->mtime); i++)
        {
            if ((tblk)->mtime[i] >= '0' && (tblk)->mtime[i] <= '7')
                timestamp.modtime = timestamp.modtime * 8
                        + (tblk)->mtime[i] - '0';
        }
        timestamp.actime = timestamp.modtime;

        for (mode = i = 0; i < (int)sizeof((tblk)->mode); i++)
        {
            if ((tblk)->mode[i] >= '0' && (tblk)->mode[i] <= '7')
                mode = mode * 8 + (tblk)->mode[i] - '0';
        }
#endif

        if (nonlys > 0)
        {
            for (i = 0;
                 i < nonlys
                && strcmp(only[i], nbuf)
                && (strncmp(only[i], nbuf, strlen(only[i]))
                    || nbuf[strlen(only[i])] != '/');
                i++)
            {
            }
            if (i >= nonlys)
            {
                outfp = NULL;
                return 0;
            }
        }

        if (verbose)
            printf("%c %s",
                ISREGULAR(*tblk) ? '-' : ("hlcbdp"[(tblk)->type - '1']),
                nbuf);
        else if (!quiet)
            printf("%s\n", nbuf);

        if (tblk->type == '1' || tblk->type == '2')
        {
            if (verbose)
                printf(" -> %s\n", tblk->linkto);
            if (!listing)
                linkorcopy(tblk->linkto, nbuf, tblk->type == '2');
            outsize = 0L;
            return 0;
        }

        if (tblk->type == '5')
        {
            if (listing)
                n2 = sString::nonconst(" directory");
#ifdef _POSIX_SOURCE
            else if (mkdir(nbuf, mode) == 0)
#else
            else if (mkdir(nbuf, 0755) == 0)
#endif
                n2 = sString::nonconst(" created");
            else
                n2 = sString::nonconst(" ignored");
            if (verbose)
                printf("%s\n", n2);
            return 0;
        }

        if (!ISREGULAR(*tblk))
        {
            if (verbose)
                printf(" ignored\n");
            outsize = 0L;
            return 0;
        }

        if (verbose)
        {
            printf(" (%ld byte%s, %ld tape block%s)\n",
                outsize,
                outsize == 1 ? "" : "s",
                (outsize + TSIZE - 1) / TSIZE,
                (outsize > 0  && outsize <= TSIZE) ? "" : "s");
        }

        if (!listing)
            outfp = createpath(nbuf);
        else
            outfp = NULL;

        if (outsize == 0 && outfp)
        {
            fclose(outfp);
#ifdef _POSIX_SOURCE
            utime(nbuf, &timestamp);
            chmod(nbuf, mode);
#endif
        }
    }
    return 0;
}

static void flush_output(unsigned w)
{
    unsigned    i;

    if (tarfp)
    {
        cvtwrite(slide, (Ulong_t)w, tarfp);
    }
    else
    {
        for (i = 0; i + TSIZE <= w; i += TSIZE)
        {
            untar(&slide[i]);
        }
    }
}


static int huft_build(unsigned *b, unsigned n, unsigned s, Ushort_t * d, Ushort_t * e, huft_t * * t, int * m)
{
    unsigned      a;
    unsigned      c[BMAX+1];
    unsigned      f;
    int          g;
    int          h;
    register unsigned i;
    register unsigned j;
    register int      k;
    int          l;
    register unsigned *p;
    register huft_t   *q;
    huft_t          r;
    huft_t          *u[BMAX];
    unsigned      v[N_MAX];
    register int      w;
    unsigned      x[BMAX+1];
    unsigned      *xp;
    int          y;
    unsigned      z;


    memset(c, 0, sizeof(c));
    p = b;    i = (int)n;
    do {
        c[*p++]++;
    } while (--i);
    if (c[0] == n)
    {
        *t = (huft_t *)NULL;
        *m = 0;
        return 0;
    }


    l = *m;
    for (j = 1; j <= BMAX; j++)
        if (c[j])
            break;
    k = j;
    if ((unsigned)l < j)
        l = j;
    for (i = BMAX; i; i--)
        if (c[i])
            break;
    g = i;
    if ((unsigned)l > i)
        l = i;
    *m = l;


    for (y = 1 << j; j < i; j++, y <<= 1)
        if ((y -= c[j]) < 0)
            return 2;
    if ((y -= c[i]) < 0)
        return 2;
    c[i] += y;


    x[1] = j = 0;
    p = c + 1;    xp = x + 2;
    while (--i) {
        *xp++ = (j += *p++);
    }

    p = b;    i = 0;
    do {
        if ((j = *p++) != 0)
            v[x[j]++] = i;
    } while (++i < n);


    x[0] = i = 0;
    p = v;
    h = -1;
    w = -l;
    u[0] = (huft_t *)NULL;
    q = (huft_t *)NULL;
    z = 0;

    for (; k <= g; k++)
    {
        a = c[k];
        while (a--)
        {
            while (k > w + l)
            {
                h++;
                w += l;

                z = (z = g - w) > (unsigned)l ? l : z;
                if ((f = 1 << (j = k - w)) > a + 1)
                {
                    f -= a + 1;
                    xp = c + k;
                    while (++j < z)
                    {
                        if ((f <<= 1) <= *++xp)
                            break;
                        f -= *xp;
                    }
                }
                z = 1 << j;

                q = (huft_t *)malloc((z + 1)*sizeof(huft_t));
                hufts += z + 1;
                *t = q + 1;
                *(t = &(q->v.t)) = (huft_t *)NULL;
                u[h] = ++q;

                if (h)
                {
                    x[h] = i;
                    r.b = (Uchar_t)l;
                    r.e = (Uchar_t)(16 + j);
                    r.v.t = q;
                    j = i >> (w - l);
                    u[h-1][j] = r;
                }
            }

            r.b = (Uchar_t)(k - w);
            if (p >= v + n)
                r.e = 99;
            else if (*p < s)
            {
                r.e = (Uchar_t)(*p < 256 ? 16 : 15);
                r.v.n = *p++;
            }
            else
            {
                r.e = (Uchar_t)e[*p - s];
                r.v.n = d[*p++ - s];
            }

            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f)
                q[j] = r;

            for (j = 1 << (k - 1); i & j; j >>= 1)
                i ^= j;
            i ^= j;

            while ((i & ((1 << w) - 1)) != x[h])
            {
                h--;
                w -= l;
            }
        }
    }


    return y != 0 && g != 1;
}



static int huft_free(huft_t * t)
{
    register huft_t *p, *q;


    p = t;
    while (p != (huft_t *)NULL)
    {
        q = (--p)->v.t;
        free(p);
        p = q;
    } 
    return 0;
}


static int inflate_codes(huft_t * tl, huft_t * td, int bl, int bd)
{
    register unsigned e;
    unsigned      n, d;
    unsigned      w;
    huft_t          *t;
    unsigned      ml, md;
    register Ulong_t  b;
    register unsigned k;


    b = bb;
    k = bk;
    w = wp;

    ml = mask_bits[bl];
    md = mask_bits[bd];
    for (;;)
    {
        NEEDBITS((unsigned)bl)
        if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)
            do
            {
                if (e == 99)
                    return 1;
                DUMPBITS(t->b)
                e -= 16;
                NEEDBITS(e)
            } while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
        DUMPBITS(t->b)
        if (e == 16)
        {
            slide[w++] = (Uchar_t)t->v.n;
            if (w == WSIZE)
            {
                flush_output(w);
                w = 0;
            }
        }
        else
        {
            if (e == 15)
                break;

            NEEDBITS(e)
            n = t->v.n + ((unsigned)b & mask_bits[e]);
            DUMPBITS(e);

            NEEDBITS((unsigned)bd)
            if ((e = (t = td + ((unsigned)b & md))->e) > 16)
                do
                {
                    if (e == 99)
                        return 1;
                    DUMPBITS(t->b)
                    e -= 16;
                    NEEDBITS(e)
                } while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
            DUMPBITS(t->b)
            NEEDBITS(e)
            d = w - t->v.n - ((unsigned)b & mask_bits[e]);
            DUMPBITS(e)

            do
            {
                n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
#if !defined(NOMEMCPY) && !defined(DEBUG)
                if (w - d >= e)
                {
                    memcpy(slide + w, slide + d, e);
                    w += e;
                    d += e;
                }
                else
#endif                     do
                    {
                        slide[w++] = slide[d++];
                    } while (--e);
                if (w == WSIZE)
                {
                    flush_output(w);
                    w = 0;
                }
            } while (n);
        }
    }

    wp = w;
    bb = b;
    bk = k;

    return 0;
}



static int inflate_stored()
{
    unsigned      n;
    unsigned      w;
    register Ulong_t  b;
    register unsigned k;

    b = bb;
    k = bk;
    w = wp;

    n = k & 7;
    DUMPBITS(n);

    NEEDBITS(16)
    n = ((unsigned)b & 0xffff);
    DUMPBITS(16)
    NEEDBITS(16)
    if (n != (unsigned)((~b) & 0xffff))
        return 1;
    DUMPBITS(16)

    while (n--)
    {
        NEEDBITS(8)
        slide[w++] = (Uchar_t)b;
        if (w == WSIZE)
        {
            flush_output(w);
            w = 0;
        }
        DUMPBITS(8)
    }


    wp = w;
    bb = b;
    bk = k;

    return 0;
}

static int inflate_fixed()
{
    int        i;
    huft_t        *tl;
    huft_t        *td;
    int        bl;
    int        bd;
    unsigned    l[288];


    for (i = 0; i < 144; i++)
        l[i] = 8;
    for (; i < 256; i++)
        l[i] = 9;
    for (; i < 280; i++)
        l[i] = 7;
    for (; i < 288; i++)
        l[i] = 8;
    bl = 7;
    if ((i = huft_build(l, 288, 257, cplens, cplext, &tl, &bl)) != 0)
        return i;

    for (i = 0; i < 30; i++)
        l[i] = 5;
    bd = 5;
    if ((i = huft_build(l, 30, 0, cpdist, cpdext, &td, &bd)) > 1)
    {
        huft_free(tl);

        return i;
    }

    if (inflate_codes(tl, td, bl, bd))
        return 1;

    huft_free(tl);
    huft_free(td);
    return 0;
}



static int inflate_dynamic()
{
    int        i;
    unsigned    j;
    unsigned    l;
    unsigned    m;
    unsigned    n;
    huft_t        *tl;
    huft_t        *td;
    int        bl;
    int        bd;
    unsigned    nb;
    unsigned    nl;
    unsigned    nd;
    unsigned    ll[286+30];
    register Ulong_t b;
    register unsigned k;


    b = bb;
    k = bk;

    NEEDBITS(5)
    nl = 257 + ((unsigned)b & 0x1f);
    DUMPBITS(5)
    NEEDBITS(5)
    nd = 1 + ((unsigned)b & 0x1f);
    DUMPBITS(5)
    NEEDBITS(4)
    nb = 4 + ((unsigned)b & 0xf);
    DUMPBITS(4)
    if (nl > 286 || nd > 30)
        return 1;

    for (j = 0; j < nb; j++)
    {
        NEEDBITS(3)
        ll[t_border[j]] = (unsigned)b & 7;
        DUMPBITS(3)
    }
    for (; j < 19; j++)
        ll[t_border[j]] = 0;

    bl = 7;
    if ((i = huft_build(ll, 19, 19, NULL, NULL, &tl, &bl)) != 0)
    {
        if (i == 1)
            huft_free(tl);
        return i;
    }

    n = nl + nd;
    m = mask_bits[bl];
    i = l = 0;
    while ((unsigned)i < n)
    {
        NEEDBITS((unsigned)bl)
        j = (td = tl + ((unsigned)b & m))->b;
        DUMPBITS(j)
        j = td->v.n;
        if (j < 16)
            ll[i++] = l = j;
        else if (j == 16)
        {
            NEEDBITS(2)
            j = 3 + ((unsigned)b & 3);
            DUMPBITS(2)
            if ((unsigned)i + j > n)
                return 1;
            while (j--)
                ll[i++] = l;
        }
        else if (j == 17)
        {
            NEEDBITS(3)
            j = 3 + ((unsigned)b & 7);
            DUMPBITS(3)
            if ((unsigned)i + j > n)
                return 1;
            while (j--)
                ll[i++] = 0;
            l = 0;
        }
        else
        {
            NEEDBITS(7)
            j = 11 + ((unsigned)b & 0x7f);
            DUMPBITS(7)
            if ((unsigned)i + j > n)
                return 1;
            while (j--)
                ll[i++] = 0;
            l = 0;
        }
    }


    huft_free(tl);

    bb = b;
    bk = k;

    bl = lbits;
    if ((i = huft_build(ll, nl, 257, cplens, cplext, &tl, &bl)) != 0)
    {
        if (i == 1) {
            error(" incomplete literal tree\n");
            huft_free(tl);
        }
        return i;
    }
    bd = dbits;
    if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) != 0)
    {
        if (i == 1) {
            error(" incomplete distance tree\n");
            huft_free(td);
        }
        huft_free(tl);
        return i;
    }

    if (inflate_codes(tl, td, bl, bd))
        return 1;

    huft_free(tl);
    huft_free(td);

    return 0;
}



static int inflate_block(int * e)
{
    unsigned        t;
    register Ulong_t    b;
    register unsigned    k;

    b = bb;
    k = bk;

    NEEDBITS(1)
    *e = (int)b & 1;
    DUMPBITS(1)

    NEEDBITS(2)
    t = (unsigned)b & 3;
    DUMPBITS(2)

    bb = b;
    bk = k;

    if (t == 2)
        return inflate_dynamic();
    if (t == 0)
        return inflate_stored();
    if (t == 1)
        return inflate_fixed();

    return 2;
}



static int inflate()
{
    int    e;
    int    r;
    unsigned h;


    wp = 0;
    bk = 0;
    bb = 0;

    h = 0;
    do
    {
        hufts = 0;
        if ((r = inflate_block(&e)) != 0)
            return r;
        if (hufts > h)
            h = hufts;
    } while (!e);

    while (bk >= 8)
    {
        bk -= 8;
    }

    flush_output(wp);

    return 0;
}


static void doarchive(char * filename)
{
    char    gunzipname[300];
    int    ch, len;

    inname = filename;
    infp = fopen(filename, "rb");
    if (!infp)
    {
        perror(filename);
        return;
    }

    fread(slide, 1, sizeof(gzhdr_t), infp);
    if (((gzhdr_t *)slide)->magic[0] == MAGIC0
     && ((gzhdr_t *)slide)->magic[1] == MAGIC1)
    {

        if (((gzhdr_t *)slide)->compression != DEFLATE)
        {
            fprintf(stderr, "Unsupported compression type\n");
            exit(1);
        }

        if (!noname && (((gzhdr_t *)slide)->flags & NAME) != 0)
        {
            for (len = 0; (ch = getc(infp)) != '\0'; len++)
            {
                gunzipname[len] = ch;
            }
            gunzipname[len] = '\0';
        }
        else if (maketar)
        {
            if ((((gzhdr_t *)slide)->flags & NAME) != 0)
                while ((ch = getc(infp)) != '\0')
                {
                }

            strcpy(gunzipname, filename);
            len = strlen(filename);
            if (len > 3 && (!strcmp(filename + len - 3, ".gz")
                    || !strcmp(filename + len - 3, ".GZ")))
            {
                gunzipname[len - 3] = '\0';
            }
            else if (len > 2 && (!strcmp(filename + len - 2, ".z")
                    || !strcmp(filename + len - 2, ".Z")))
            {
                gunzipname[len - 2] = '\0';
            }
            else if (len > 4 && (!strcmp(filename + len - 4, ".tgz")
                    || !strcmp(filename + len - 4, ".TGZ")))
            {
                strcpy(&gunzipname[len - 4], ".tar");
            }
            else
            {
                strcpy(gunzipname, "untar.out");
            }
        }

        if (maketar)
        {
            if (!quiet && listing)
            {
                printf("%s: would be gunzipped to %s\n", filename, gunzipname);
                fclose(infp);
                return;
            }

            if (!force && access(gunzipname, 0) == 0)
            {
                fprintf(stderr, "%s: exists, will not overwrite without \"-f\"\n", gunzipname);
                exit(2);
            }
            tarfp = fopen(gunzipname, convert ? "w" : "wb");
            if (!tarfp)
            {
                perror(gunzipname);
                exit(2);
            }
        }

        if (inflate() != 0)
        {
            fprintf(stderr, "%s: bad compression data\n", filename);
            exit(2);
        }
    }
    else
    {

        if (maketar)
        {
            fprintf(stderr, "%s: isn't gzipped\n", filename);
            fclose(infp);
            return;
        }

        fread(&slide[sizeof(gzhdr_t)], 1, TSIZE - sizeof(gzhdr_t), infp);

        do
        {
            untar(slide);
        } while (fread(slide, 1, TSIZE, infp) == TSIZE);
    }

    fclose(infp);
    if (tarfp)
    {
        fclose(tarfp);
        tarfp = NULL;
        if (!quiet)
        {
            printf("%s: gunzipped to %s\n", filename, gunzipname);
        }
    }
    if (outsize > 0)
    {
        printf("WARNING: Last file might be truncated!\n");
        fclose(outfp);
        outfp = NULL;
    }
}

static void usage(char * argv0, int exitcode)
{
    printf("Usage: %s [options] archive.tgz [filename] ...\n", argv0);
    printf("   or: %s [options] -d filename.gz ...\n", argv0);
    printf("\n");
    printf("Options: -t   Test -- list contents but don't extract\n");
    printf("         -f   Force -- allow existing files to be overwritten\n");
    printf("         -q   Quiet -- suppress the normal chatter\n");
    printf("         -v   Verbose -- output extra information about each file\n");
    printf("         -p   Path -- allow absolute pathnames (don't strip leading '/')\n");
    printf("         -c   Convert -- convert files to local text format\n");
    printf("         -d   Decompress -- perform \"gunzip\" but not \"tar x\"\n");
    printf("         -n   No-name -- with \"-d\", ignore original name in gzip header\n");
    printf("\n");
    printf("This program lists/extracts files from a \"*.tar\" or \"*.tgz\" archive.  You can\n");
    printf("optionally specify certain files or directories to list/extract; otherwise it\n");
#ifdef _POSIX_SOURCE
# ifdef _WEAK_POSIX
    printf("will list/extract them all.  File attributes are preserved fairly well, but\n");
    printf("linked files are restored via COPYING.  This program can also be used (with -d)\n");
    printf("to gunzip non-tar files.\n");
# else     printf("will list/extract them all.  File attributes are preserved, and linked files\n");
    printf("will be restored as links.  This program can also be used (with -d) to gunzip\n");
    printf("non-tar files.\n");
# endif #else     printf("will list/extract them all.  File attributes are NOT preserved.  Linked files\n");
    printf("will be restored via COPYING.  This program can also be used (with -d) to\n");
    printf("gunzip non-tar files.\n");
#endif     printf("\n");
    printf("THIS PROGRAM IS IN THE PUBLIC DOMAIN, AND IS FREELY REDISTRIBUTABLE.\n");
    printf("Report bugs to kirkenda@cs.pdx.edu\n");
    exit(exitcode);
}

static int ZZmain(int argc, char * * argv)
{
    int    i, j;

    if (argc < 2 || (argc == 2 && !strcmp(argv[1], "--help")))
        usage(argv[0], 0);
    if (argc == 2 && !strcmp(argv[1], "--version"))
    {
        printf("untar %s\n", VERSION);
        printf("Placed in public domain by the author, Steve Kirkendall\n");
        exit(0);
    }

    for (i = 1; i < argc && argv[i][0] == '-'; i++)
    {
        if (!argv[i][1])
            usage(argv[0], 2);
        for (j = 1; argv[i][j]; j++)
        {
            switch (argv[i][j])
            {
              case 'd':    maketar = 1;    break;
              case 'n':    noname = 1;    break;
              case 't':    listing = 1;    break;
              case 'f':    force = 1;    break;
              case 'q':    quiet = 1;    break;
              case 'c':    convert = 1;    break;
              case 'p':    abspath = 1;    break;
              case 'v':    verbose = 1;    break;
              default:
                usage(argv[0], 2);
            }
        }
    }
    if (i >= argc)
    {
        usage(argv[0], 2);
    }

    if (maketar)
    {
        for (; i < argc; i++)
        {
            doarchive(argv[i]);
        }
    }
    else
    {
        if (i + 1 < argc)
        {
            only = &argv[i + 1];
            nonlys = argc - (i + 1);
        }
        else
        {
            nonlys = 0;
        }
    
        doarchive(argv[i]);
    }

    if (didabs)
        fprintf(stderr, "WARNING: Removed leading slashes because \"-p\" wasn't given.\n");
    exit(0);
}
