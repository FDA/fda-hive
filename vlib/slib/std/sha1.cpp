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


#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/time.h>

#include <slib/std/cryptohash.hpp>

typedef slib::sSHA1::Context sha1_ctxt;

static u_int32_t _K[] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };
#define K(t)    _K[(t) / 20]

#define F0(b, c, d) (((b) & (c)) | ((~(b)) & (d)))
#define F1(b, c, d) (((b) ^ (c)) ^ (d))
#define F2(b, c, d) (((b) & (c)) | ((b) & (d)) | ((c) & (d)))
#define F3(b, c, d) (((b) ^ (c)) ^ (d))

#define S(n, x)     (((x) << (n)) | ((x) >> (32 - n)))

#define H(n)    (ctxt->h.b32[(n)])
#define COUNT   (ctxt->count)
#define BCOUNT  (ctxt->c.b64[0] / 8)
#define W(n)    (ctxt->m.b32[(n)])

#define PUTBYTE(x)  { \
    ctxt->m.b8[(COUNT % 64)] = (x);     \
    COUNT++;                \
    COUNT %= 64;                \
    ctxt->c.b64[0] += 8;            \
    if (COUNT % 64 == 0)            \
        sha1_step(ctxt);        \
     }

#define PUTPAD(x)   { \
    ctxt->m.b8[(COUNT % 64)] = (x);     \
    COUNT++;                \
    COUNT %= 64;                \
    if (COUNT % 64 == 0)            \
        sha1_step(ctxt);        \
     }

static void sha1_step(sha1_ctxt *ctxt)
{
    u_int32_t   a, b, c, d, e;
    size_t t, s;
    u_int32_t   tmp;

#if BYTE_ORDER == LITTLE_ENDIAN
    sha1_ctxt tctxt;
    bcopy(&ctxt->m.b8[0], &tctxt.m.b8[0], 64);
    ctxt->m.b8[0] = tctxt.m.b8[3]; ctxt->m.b8[1] = tctxt.m.b8[2];
    ctxt->m.b8[2] = tctxt.m.b8[1]; ctxt->m.b8[3] = tctxt.m.b8[0];
    ctxt->m.b8[4] = tctxt.m.b8[7]; ctxt->m.b8[5] = tctxt.m.b8[6];
    ctxt->m.b8[6] = tctxt.m.b8[5]; ctxt->m.b8[7] = tctxt.m.b8[4];
    ctxt->m.b8[8] = tctxt.m.b8[11]; ctxt->m.b8[9] = tctxt.m.b8[10];
    ctxt->m.b8[10] = tctxt.m.b8[9]; ctxt->m.b8[11] = tctxt.m.b8[8];
    ctxt->m.b8[12] = tctxt.m.b8[15]; ctxt->m.b8[13] = tctxt.m.b8[14];
    ctxt->m.b8[14] = tctxt.m.b8[13]; ctxt->m.b8[15] = tctxt.m.b8[12];
    ctxt->m.b8[16] = tctxt.m.b8[19]; ctxt->m.b8[17] = tctxt.m.b8[18];
    ctxt->m.b8[18] = tctxt.m.b8[17]; ctxt->m.b8[19] = tctxt.m.b8[16];
    ctxt->m.b8[20] = tctxt.m.b8[23]; ctxt->m.b8[21] = tctxt.m.b8[22];
    ctxt->m.b8[22] = tctxt.m.b8[21]; ctxt->m.b8[23] = tctxt.m.b8[20];
    ctxt->m.b8[24] = tctxt.m.b8[27]; ctxt->m.b8[25] = tctxt.m.b8[26];
    ctxt->m.b8[26] = tctxt.m.b8[25]; ctxt->m.b8[27] = tctxt.m.b8[24];
    ctxt->m.b8[28] = tctxt.m.b8[31]; ctxt->m.b8[29] = tctxt.m.b8[30];
    ctxt->m.b8[30] = tctxt.m.b8[29]; ctxt->m.b8[31] = tctxt.m.b8[28];
    ctxt->m.b8[32] = tctxt.m.b8[35]; ctxt->m.b8[33] = tctxt.m.b8[34];
    ctxt->m.b8[34] = tctxt.m.b8[33]; ctxt->m.b8[35] = tctxt.m.b8[32];
    ctxt->m.b8[36] = tctxt.m.b8[39]; ctxt->m.b8[37] = tctxt.m.b8[38];
    ctxt->m.b8[38] = tctxt.m.b8[37]; ctxt->m.b8[39] = tctxt.m.b8[36];
    ctxt->m.b8[40] = tctxt.m.b8[43]; ctxt->m.b8[41] = tctxt.m.b8[42];
    ctxt->m.b8[42] = tctxt.m.b8[41]; ctxt->m.b8[43] = tctxt.m.b8[40];
    ctxt->m.b8[44] = tctxt.m.b8[47]; ctxt->m.b8[45] = tctxt.m.b8[46];
    ctxt->m.b8[46] = tctxt.m.b8[45]; ctxt->m.b8[47] = tctxt.m.b8[44];
    ctxt->m.b8[48] = tctxt.m.b8[51]; ctxt->m.b8[49] = tctxt.m.b8[50];
    ctxt->m.b8[50] = tctxt.m.b8[49]; ctxt->m.b8[51] = tctxt.m.b8[48];
    ctxt->m.b8[52] = tctxt.m.b8[55]; ctxt->m.b8[53] = tctxt.m.b8[54];
    ctxt->m.b8[54] = tctxt.m.b8[53]; ctxt->m.b8[55] = tctxt.m.b8[52];
    ctxt->m.b8[56] = tctxt.m.b8[59]; ctxt->m.b8[57] = tctxt.m.b8[58];
    ctxt->m.b8[58] = tctxt.m.b8[57]; ctxt->m.b8[59] = tctxt.m.b8[56];
    ctxt->m.b8[60] = tctxt.m.b8[63]; ctxt->m.b8[61] = tctxt.m.b8[62];
    ctxt->m.b8[62] = tctxt.m.b8[61]; ctxt->m.b8[63] = tctxt.m.b8[60];
#endif

    a = H(0); b = H(1); c = H(2); d = H(3); e = H(4);

    for (t = 0; t < 20; t++) {
        s = t & 0x0f;
        if (t >= 16) {
            W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
        }
        tmp = S(5, a) + F0(b, c, d) + e + W(s) + K(t);
        e = d; d = c; c = S(30, b); b = a; a = tmp;
    }
    for (t = 20; t < 40; t++) {
        s = t & 0x0f;
        W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
        tmp = S(5, a) + F1(b, c, d) + e + W(s) + K(t);
        e = d; d = c; c = S(30, b); b = a; a = tmp;
    }
    for (t = 40; t < 60; t++) {
        s = t & 0x0f;
        W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
        tmp = S(5, a) + F2(b, c, d) + e + W(s) + K(t);
        e = d; d = c; c = S(30, b); b = a; a = tmp;
    }
    for (t = 60; t < 80; t++) {
        s = t & 0x0f;
        W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
        tmp = S(5, a) + F3(b, c, d) + e + W(s) + K(t);
        e = d; d = c; c = S(30, b); b = a; a = tmp;
    }

    H(0) = H(0) + a;
    H(1) = H(1) + b;
    H(2) = H(2) + c;
    H(3) = H(3) + d;
    H(4) = H(4) + e;

    bzero(&ctxt->m.b8[0], 64);
}


inline static void sha1_init(sha1_ctxt *ctxt)
{
    bzero(ctxt, sizeof(sha1_ctxt));
    H(0) = 0x67452301;
    H(1) = 0xefcdab89;
    H(2) = 0x98badcfe;
    H(3) = 0x10325476;
    H(4) = 0xc3d2e1f0;
}

static void sha1_pad(sha1_ctxt *ctxt)
{
    size_t padlen;
    size_t padstart;

    PUTPAD(0x80);

    padstart = COUNT % 64;
    padlen = 64 - padstart;
    if (padlen < 8) {
        bzero(&ctxt->m.b8[padstart], padlen);
        COUNT += padlen;
        COUNT %= 64;
        sha1_step(ctxt);
        padstart = COUNT % 64;
        padlen = 64 - padstart;
    }
    bzero(&ctxt->m.b8[padstart], padlen - 8);
    COUNT += (padlen - 8);
    COUNT %= 64;
#if BYTE_ORDER == BIG_ENDIAN
    PUTPAD(ctxt->c.b8[0]); PUTPAD(ctxt->c.b8[1]);
    PUTPAD(ctxt->c.b8[2]); PUTPAD(ctxt->c.b8[3]);
    PUTPAD(ctxt->c.b8[4]); PUTPAD(ctxt->c.b8[5]);
    PUTPAD(ctxt->c.b8[6]); PUTPAD(ctxt->c.b8[7]);
#else
    PUTPAD(ctxt->c.b8[7]); PUTPAD(ctxt->c.b8[6]);
    PUTPAD(ctxt->c.b8[5]); PUTPAD(ctxt->c.b8[4]);
    PUTPAD(ctxt->c.b8[3]); PUTPAD(ctxt->c.b8[2]);
    PUTPAD(ctxt->c.b8[1]); PUTPAD(ctxt->c.b8[0]);
#endif
}

inline static void sha1_loop(sha1_ctxt *ctxt, const u_int8_t *input, size_t len)
{
    size_t gaplen;
    size_t gapstart;
    size_t off;
    size_t copysiz;

    off = 0;

    while (off < len) {
        gapstart = COUNT % 64;
        gaplen = 64 - gapstart;

        copysiz = (gaplen < len - off) ? gaplen : len - off;
        bcopy(&input[off], &ctxt->m.b8[gapstart], copysiz);
        COUNT += copysiz;
        COUNT %= 64;
        ctxt->c.b64[0] += copysiz * 8;
        if (COUNT % 64 == 0)
            sha1_step(ctxt);
        off += copysiz;
    }
}

inline static void sha1_result(sha1_ctxt *ctxt, caddr_t digest0)
{
    u_int8_t *digest;

    digest = (u_int8_t *)digest0;
    sha1_pad(ctxt);
#if BYTE_ORDER == BIG_ENDIAN
    bcopy(&ctxt->h.b8[0], digest, 20);
#else
    digest[0] = ctxt->h.b8[3]; digest[1] = ctxt->h.b8[2];
    digest[2] = ctxt->h.b8[1]; digest[3] = ctxt->h.b8[0];
    digest[4] = ctxt->h.b8[7]; digest[5] = ctxt->h.b8[6];
    digest[6] = ctxt->h.b8[5]; digest[7] = ctxt->h.b8[4];
    digest[8] = ctxt->h.b8[11]; digest[9] = ctxt->h.b8[10];
    digest[10] = ctxt->h.b8[9]; digest[11] = ctxt->h.b8[8];
    digest[12] = ctxt->h.b8[15]; digest[13] = ctxt->h.b8[14];
    digest[14] = ctxt->h.b8[13]; digest[15] = ctxt->h.b8[12];
    digest[16] = ctxt->h.b8[19]; digest[17] = ctxt->h.b8[18];
    digest[18] = ctxt->h.b8[17]; digest[19] = ctxt->h.b8[16];
#endif
}


using namespace slib;

void sSHA1::init_impl()
{
    sha1_init(&_context);
}

void sSHA1::update_impl(const void * buffer, idx len)
{
    sha1_loop(&_context, (unsigned char *)buffer, len);
}

void sSHA1::finish_impl()
{
    sha1_result(&_context, (caddr_t)_output_bin);
}
