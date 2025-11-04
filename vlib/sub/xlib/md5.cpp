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


#include <xlib/md5.hpp>
using namespace slib;

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>

#if STDC_HEADERS || defined _LIBC
# include <stdlib.h>
# include <string.h>
#else
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) memmove ((d), (s), (n))
# endif
#endif



#ifdef _LIBC
# include <sys/types.h>
typedef u_int32_t md5_uint32;
typedef uintptr_t md5_uintptr;
#else
#  define INT_MAX_32_BITS 2147483647


# ifndef INT_MAX
#  define INT_MAX INT_MAX_32_BITS
# endif

# if INT_MAX == INT_MAX_32_BITS
   typedef unsigned int md5_uint32;
# else
#  if SHRT_MAX == INT_MAX_32_BITS
    typedef unsigned short md5_uint32;
#  else
#   if LONG_MAX == INT_MAX_32_BITS
     typedef unsigned long md5_uint32;
#   else
     "Cannot determine unsigned 32-bit data type."
#   endif
#  endif
# endif
typedef unsigned long int md5_uintptr;
#endif


struct md5_ctx
{

  md5_uint32 A;
  md5_uint32 B;
  md5_uint32 C;
  md5_uint32 D;

  md5_uint32 total[2];
  md5_uint32 buflen;
  char buffer[128] ;
};


#ifdef _LIBC
# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define WORDS_BIGENDIAN 1
# endif
#endif

#ifdef WORDS_BIGENDIAN
# define SWAP(n)                            \
    (((n) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) | ((n) >> 24))
#else
# define SWAP(n) (n)
#endif


static const unsigned char fillbuf[64] = { 0x80, 0};


void sMD5::md5_init_ctx (void *vctx)
{
  md5_ctx * ctx=(md5_ctx *)vctx;
  ctx->A = (md5_uint32) 0x67452301;
  ctx->B = (md5_uint32) 0xefcdab89;
  ctx->C = (md5_uint32) 0x98badcfe;
  ctx->D = (md5_uint32) 0x10325476;

  ctx->total[0] = ctx->total[1] = 0;
  ctx->buflen = 0;
}

void * sMD5::md5_read_ctx (const void *vctx, void *resbuf)
{
  if(!resbuf)resbuf=(void*)bin;
  const md5_ctx * ctx=(const md5_ctx *)vctx;
  ((md5_uint32 *) resbuf)[0] = SWAP (ctx->A);
  ((md5_uint32 *) resbuf)[1] = SWAP (ctx->B);
  ((md5_uint32 *) resbuf)[2] = SWAP (ctx->C);
  ((md5_uint32 *) resbuf)[3] = SWAP (ctx->D);

  unsigned char * o=(unsigned char *)resbuf;
  for ( idx i=0; i<MD5size; ++i){
    sprintf(sum+2*i,"%s%x",o[i] <0x10 ? "0":"" , o[i]);
  }
  sum[2*MD5size]=0;

  return resbuf;
}

void * sMD5::md5_finish_ctx (void *vctx, void *resbuf)
{
    md5_ctx * ctx=(md5_ctx *)vctx;
  md5_uint32 bytes = ctx->buflen;
  size_t pad;

  ctx->total[0] += bytes;
  if (ctx->total[0] < bytes)
    ++ctx->total[1];

  pad = bytes >= 56 ? 64 + 56 - bytes : 56 - bytes;
  memcpy (&ctx->buffer[bytes], fillbuf, pad);

  *(md5_uint32 *) &ctx->buffer[bytes + pad] = SWAP (ctx->total[0] << 3);
  *(md5_uint32 *) &ctx->buffer[bytes + pad + 4] = SWAP ((ctx->total[1] << 3) |
                            (ctx->total[0] >> 29));

  md5_process_block (ctx->buffer, bytes + pad + 8, ctx);

  return md5_read_ctx (ctx, resbuf);
}

int sMD5::parse_stream (FILE *stream, void *resblock)
{
#define BLOCKSIZE (4096*64)
  struct md5_ctx ctx;
  char buffer[BLOCKSIZE + 72];
  size_t sum;

  md5_init_ctx (&ctx);

  while (1)
    {
      size_t n;
      sum = 0;

      do
    {
      n = fread (buffer + sum, 1, BLOCKSIZE - sum, stream);

      sum += n;
    }
      while (sum < BLOCKSIZE && n != 0);
      if (n == 0 && ferror (stream))
        return 1;

      if (n == 0)
    break;

      md5_process_block (buffer, BLOCKSIZE, &ctx);
    }

  if (sum > 0)
    md5_process_bytes (buffer, sum, &ctx);

  md5_finish_ctx (&ctx, resblock);
  return 0;
}

int sMD5::parse_file(const char * filename, void * resblock)
{
    FILE * fp=fopen(filename,"r");if(!fp)return 0;
    int res=parse_stream(fp,resblock);
    fclose(fp);
    return res;
}

void * sMD5::parse_buffer (const char *buffer, idx len, void *resblock)
{
  struct md5_ctx ctx;

  md5_init_ctx (&ctx);

  md5_process_bytes (buffer, len, &ctx);

  return md5_finish_ctx (&ctx, resblock);
}


void sMD5::md5_process_bytes (const void *buffer, idx len, void * vctx)
{
    md5_ctx * ctx=(md5_ctx *)vctx;
  if (ctx->buflen != 0)
    {
      size_t left_over = ctx->buflen;
      size_t add = 128 - left_over > (size_t)len ? (size_t)len : 128 - left_over;

      memcpy (&ctx->buffer[left_over], buffer, add);
      ctx->buflen += add;

      if (left_over + add > 64)
    {
      md5_process_block (ctx->buffer, (left_over + add) & ~63, ctx);
      memcpy (ctx->buffer, &ctx->buffer[(left_over + add) & ~63],
          (left_over + add) & 63);
      ctx->buflen = (left_over + add) & 63;
    }

      buffer = (const void *) ((const char *) buffer + add);
      len -= add;
    }

  if (len > 64)
    {
#if !_STRING_ARCH_unaligned
# if __GNUC__ >= 2
#  define UNALIGNED_P(p) (((md5_uintptr) p) % __alignof__ (md5_uint32) != 0)
# else
#  define UNALIGNED_P(p) (((md5_uintptr) p) % sizeof (md5_uint32) != 0)
# endif
      if (UNALIGNED_P (buffer))
        while (len > 64)
          {
            md5_process_block (memcpy (ctx->buffer, buffer, 64), 64, ctx);
            buffer = (const char *) buffer + 64;
            len -= 64;
          }
      else
#endif
      md5_process_block (buffer, len & ~63, ctx);
      buffer = (const void *) ((const char *) buffer + (len & ~63));
      len &= 63;
    }

  if (len > 0)
    {
      memcpy (ctx->buffer, buffer, len);
      ctx->buflen = len;
    }
}


#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))


void sMD5::md5_process_block (const void *buffer, idx len, void *vctx)
{
    md5_ctx * ctx=(md5_ctx *)vctx;
  md5_uint32 correct_words[16];
  const md5_uint32 *words = (const md5_uint32 *) buffer;
  size_t nwords = len / sizeof (md5_uint32);
  const md5_uint32 *endp = words + nwords;
  md5_uint32 A = ctx->A;
  md5_uint32 B = ctx->B;
  md5_uint32 C = ctx->C;
  md5_uint32 D = ctx->D;

  ctx->total[0] += len;
  if (ctx->total[0] < len)
    ++ctx->total[1];

  while (words < endp)
    {
      md5_uint32 *cwp = correct_words;
      md5_uint32 A_save = A;
      md5_uint32 B_save = B;
      md5_uint32 C_save = C;
      md5_uint32 D_save = D;


#define OP(a, b, c, d, s, T)                        \
      do                                \
        {                                \
      a += FF (b, c, d) + (*cwp++ = SWAP (*words)) + T;        \
      ++words;                            \
      CYCLIC (a, s);                        \
      a += b;                            \
        }                                \
      while (0)

#define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))


      OP (A, B, C, D,  7, (md5_uint32) 0xd76aa478);
      OP (D, A, B, C, 12, (md5_uint32) 0xe8c7b756);
      OP (C, D, A, B, 17, (md5_uint32) 0x242070db);
      OP (B, C, D, A, 22, (md5_uint32) 0xc1bdceee);
      OP (A, B, C, D,  7, (md5_uint32) 0xf57c0faf);
      OP (D, A, B, C, 12, (md5_uint32) 0x4787c62a);
      OP (C, D, A, B, 17, (md5_uint32) 0xa8304613);
      OP (B, C, D, A, 22, (md5_uint32) 0xfd469501);
      OP (A, B, C, D,  7, (md5_uint32) 0x698098d8);
      OP (D, A, B, C, 12, (md5_uint32) 0x8b44f7af);
      OP (C, D, A, B, 17, (md5_uint32) 0xffff5bb1);
      OP (B, C, D, A, 22, (md5_uint32) 0x895cd7be);
      OP (A, B, C, D,  7, (md5_uint32) 0x6b901122);
      OP (D, A, B, C, 12, (md5_uint32) 0xfd987193);
      OP (C, D, A, B, 17, (md5_uint32) 0xa679438e);
      OP (B, C, D, A, 22, (md5_uint32) 0x49b40821);

#undef OP
#define OP(a, b, c, d, k, s, T)                        \
      do                                 \
    {                                \
      a += FX (b, c, d) + correct_words[k] + T;            \
      CYCLIC (a, s);                        \
      a += b;                            \
    }                                \
      while (0)

#define FX(b, c, d) FG (b, c, d)

      OP (A, B, C, D,  1,  5, (md5_uint32) 0xf61e2562);
      OP (D, A, B, C,  6,  9, (md5_uint32) 0xc040b340);
      OP (C, D, A, B, 11, 14, (md5_uint32) 0x265e5a51);
      OP (B, C, D, A,  0, 20, (md5_uint32) 0xe9b6c7aa);
      OP (A, B, C, D,  5,  5, (md5_uint32) 0xd62f105d);
      OP (D, A, B, C, 10,  9, (md5_uint32) 0x02441453);
      OP (C, D, A, B, 15, 14, (md5_uint32) 0xd8a1e681);
      OP (B, C, D, A,  4, 20, (md5_uint32) 0xe7d3fbc8);
      OP (A, B, C, D,  9,  5, (md5_uint32) 0x21e1cde6);
      OP (D, A, B, C, 14,  9, (md5_uint32) 0xc33707d6);
      OP (C, D, A, B,  3, 14, (md5_uint32) 0xf4d50d87);
      OP (B, C, D, A,  8, 20, (md5_uint32) 0x455a14ed);
      OP (A, B, C, D, 13,  5, (md5_uint32) 0xa9e3e905);
      OP (D, A, B, C,  2,  9, (md5_uint32) 0xfcefa3f8);
      OP (C, D, A, B,  7, 14, (md5_uint32) 0x676f02d9);
      OP (B, C, D, A, 12, 20, (md5_uint32) 0x8d2a4c8a);

#undef FX
#define FX(b, c, d) FH (b, c, d)

      OP (A, B, C, D,  5,  4, (md5_uint32) 0xfffa3942);
      OP (D, A, B, C,  8, 11, (md5_uint32) 0x8771f681);
      OP (C, D, A, B, 11, 16, (md5_uint32) 0x6d9d6122);
      OP (B, C, D, A, 14, 23, (md5_uint32) 0xfde5380c);
      OP (A, B, C, D,  1,  4, (md5_uint32) 0xa4beea44);
      OP (D, A, B, C,  4, 11, (md5_uint32) 0x4bdecfa9);
      OP (C, D, A, B,  7, 16, (md5_uint32) 0xf6bb4b60);
      OP (B, C, D, A, 10, 23, (md5_uint32) 0xbebfbc70);
      OP (A, B, C, D, 13,  4, (md5_uint32) 0x289b7ec6);
      OP (D, A, B, C,  0, 11, (md5_uint32) 0xeaa127fa);
      OP (C, D, A, B,  3, 16, (md5_uint32) 0xd4ef3085);
      OP (B, C, D, A,  6, 23, (md5_uint32) 0x04881d05);
      OP (A, B, C, D,  9,  4, (md5_uint32) 0xd9d4d039);
      OP (D, A, B, C, 12, 11, (md5_uint32) 0xe6db99e5);
      OP (C, D, A, B, 15, 16, (md5_uint32) 0x1fa27cf8);
      OP (B, C, D, A,  2, 23, (md5_uint32) 0xc4ac5665);

#undef FX
#define FX(b, c, d) FI (b, c, d)

      OP (A, B, C, D,  0,  6, (md5_uint32) 0xf4292244);
      OP (D, A, B, C,  7, 10, (md5_uint32) 0x432aff97);
      OP (C, D, A, B, 14, 15, (md5_uint32) 0xab9423a7);
      OP (B, C, D, A,  5, 21, (md5_uint32) 0xfc93a039);
      OP (A, B, C, D, 12,  6, (md5_uint32) 0x655b59c3);
      OP (D, A, B, C,  3, 10, (md5_uint32) 0x8f0ccc92);
      OP (C, D, A, B, 10, 15, (md5_uint32) 0xffeff47d);
      OP (B, C, D, A,  1, 21, (md5_uint32) 0x85845dd1);
      OP (A, B, C, D,  8,  6, (md5_uint32) 0x6fa87e4f);
      OP (D, A, B, C, 15, 10, (md5_uint32) 0xfe2ce6e0);
      OP (C, D, A, B,  6, 15, (md5_uint32) 0xa3014314);
      OP (B, C, D, A, 13, 21, (md5_uint32) 0x4e0811a1);
      OP (A, B, C, D,  4,  6, (md5_uint32) 0xf7537e82);
      OP (D, A, B, C, 11, 10, (md5_uint32) 0xbd3af235);
      OP (C, D, A, B,  2, 15, (md5_uint32) 0x2ad7d2bb);
      OP (B, C, D, A,  9, 21, (md5_uint32) 0xeb86d391);

      A += A_save;
      B += B_save;
      C += C_save;
      D += D_save;
    }

  ctx->A = A;
  ctx->B = B;
  ctx->C = C;
  ctx->D = D;
}


