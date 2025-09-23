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


#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_AESNI_C)

#include "mbedtls/aesni.h"

#include <string.h>

#ifndef asm
#define asm __asm
#endif

#if defined(MBEDTLS_HAVE_X86_64)

int mbedtls_aesni_has_support( unsigned int what )
{
    static int done = 0;
    static unsigned int c = 0;

    if( ! done )
    {
        asm( "movl  $1, %%eax   \n\t"
             "cpuid             \n\t"
             : "=c" (c)
             :
             : "eax", "ebx", "edx" );
        done = 1;
    }

    return( ( c & what ) != 0 );
}

#define AESDEC      ".byte 0x66,0x0F,0x38,0xDE,"
#define AESDECLAST  ".byte 0x66,0x0F,0x38,0xDF,"
#define AESENC      ".byte 0x66,0x0F,0x38,0xDC,"
#define AESENCLAST  ".byte 0x66,0x0F,0x38,0xDD,"
#define AESIMC      ".byte 0x66,0x0F,0x38,0xDB,"
#define AESKEYGENA  ".byte 0x66,0x0F,0x3A,0xDF,"
#define PCLMULQDQ   ".byte 0x66,0x0F,0x3A,0x44,"

#define xmm0_xmm0   "0xC0"
#define xmm0_xmm1   "0xC8"
#define xmm0_xmm2   "0xD0"
#define xmm0_xmm3   "0xD8"
#define xmm0_xmm4   "0xE0"
#define xmm1_xmm0   "0xC1"
#define xmm1_xmm2   "0xD1"

int mbedtls_aesni_crypt_ecb( mbedtls_aes_context *ctx,
                     int mode,
                     const unsigned char input[16],
                     unsigned char output[16] )
{
    asm( "movdqu    (%3), %%xmm0    \n\t"         "movdqu    (%1), %%xmm1    \n\t"         "pxor      %%xmm1, %%xmm0  \n\t"         "add       $16, %1         \n\t"         "subl      $1, %0          \n\t"         "test      %2, %2          \n\t"         "jz        2f              \n\t"
         "1:                        \n\t"         "movdqu    (%1), %%xmm1    \n\t"         AESENC     xmm1_xmm0      "\n\t"         "add       $16, %1         \n\t"         "subl      $1, %0          \n\t"         "jnz       1b              \n\t"
         "movdqu    (%1), %%xmm1    \n\t"         AESENCLAST xmm1_xmm0      "\n\t"         "jmp       3f              \n\t"

         "2:                        \n\t"         "movdqu    (%1), %%xmm1    \n\t"
         AESDEC     xmm1_xmm0      "\n\t"         "add       $16, %1         \n\t"
         "subl      $1, %0          \n\t"
         "jnz       2b              \n\t"
         "movdqu    (%1), %%xmm1    \n\t"         AESDECLAST xmm1_xmm0      "\n\t"
         "3:                        \n\t"
         "movdqu    %%xmm0, (%4)    \n\t"         :
         : "r" (ctx->nr), "r" (ctx->rk), "r" (mode), "r" (input), "r" (output)
         : "memory", "cc", "xmm0", "xmm1" );


    return( 0 );
}

void mbedtls_aesni_gcm_mult( unsigned char c[16],
                     const unsigned char a[16],
                     const unsigned char b[16] )
{
    unsigned char aa[16], bb[16], cc[16];
    size_t i;

    for( i = 0; i < 16; i++ )
    {
        aa[i] = a[15 - i];
        bb[i] = b[15 - i];
    }

    asm( "movdqu (%0), %%xmm0               \n\t"         "movdqu (%1), %%xmm1               \n\t"
         "movdqa %%xmm1, %%xmm2             \n\t"         "movdqa %%xmm1, %%xmm3             \n\t"         "movdqa %%xmm1, %%xmm4             \n\t"         PCLMULQDQ xmm0_xmm1 ",0x00         \n\t"         PCLMULQDQ xmm0_xmm2 ",0x11         \n\t"         PCLMULQDQ xmm0_xmm3 ",0x10         \n\t"         PCLMULQDQ xmm0_xmm4 ",0x01         \n\t"         "pxor %%xmm3, %%xmm4               \n\t"         "movdqa %%xmm4, %%xmm3             \n\t"         "psrldq $8, %%xmm4                 \n\t"         "pslldq $8, %%xmm3                 \n\t"         "pxor %%xmm4, %%xmm2               \n\t"         "pxor %%xmm3, %%xmm1               \n\t"
         "movdqa %%xmm1, %%xmm3             \n\t"         "movdqa %%xmm2, %%xmm4             \n\t"         "psllq $1, %%xmm1                  \n\t"         "psllq $1, %%xmm2                  \n\t"         "psrlq $63, %%xmm3                 \n\t"         "psrlq $63, %%xmm4                 \n\t"         "movdqa %%xmm3, %%xmm5             \n\t"         "pslldq $8, %%xmm3                 \n\t"         "pslldq $8, %%xmm4                 \n\t"         "psrldq $8, %%xmm5                 \n\t"         "por %%xmm3, %%xmm1                \n\t"         "por %%xmm4, %%xmm2                \n\t"         "por %%xmm5, %%xmm2                \n\t"
         "movdqa %%xmm1, %%xmm3             \n\t"         "movdqa %%xmm1, %%xmm4             \n\t"         "movdqa %%xmm1, %%xmm5             \n\t"         "psllq $63, %%xmm3                 \n\t"         "psllq $62, %%xmm4                 \n\t"         "psllq $57, %%xmm5                 \n\t"
         "pxor %%xmm4, %%xmm3               \n\t"         "pxor %%xmm5, %%xmm3               \n\t"         "pslldq $8, %%xmm3                 \n\t"         "pxor %%xmm3, %%xmm1               \n\t"
         "movdqa %%xmm1,%%xmm0              \n\t"         "movdqa %%xmm1,%%xmm4              \n\t"         "movdqa %%xmm1,%%xmm5              \n\t"         "psrlq $1, %%xmm0                  \n\t"         "psrlq $2, %%xmm4                  \n\t"         "psrlq $7, %%xmm5                  \n\t"         "pxor %%xmm4, %%xmm0               \n\t"         "pxor %%xmm5, %%xmm0               \n\t"         "movdqa %%xmm1,%%xmm3              \n\t"         "movdqa %%xmm1,%%xmm4              \n\t"         "movdqa %%xmm1,%%xmm5              \n\t"         "psllq $63, %%xmm3                 \n\t"         "psllq $62, %%xmm4                 \n\t"         "psllq $57, %%xmm5                 \n\t"         "pxor %%xmm4, %%xmm3               \n\t"         "pxor %%xmm5, %%xmm3               \n\t"         "psrldq $8, %%xmm3                 \n\t"         "pxor %%xmm3, %%xmm0               \n\t"         "pxor %%xmm1, %%xmm0               \n\t"         "pxor %%xmm2, %%xmm0               \n\t"
         "movdqu %%xmm0, (%2)               \n\t"         :
         : "r" (aa), "r" (bb), "r" (cc)
         : "memory", "cc", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5" );

    for( i = 0; i < 16; i++ )
        c[i] = cc[15 - i];

    return;
}

void mbedtls_aesni_inverse_key( unsigned char *invkey,
                        const unsigned char *fwdkey, int nr )
{
    unsigned char *ik = invkey;
    const unsigned char *fk = fwdkey + 16 * nr;

    memcpy( ik, fk, 16 );

    for( fk -= 16, ik += 16; fk > fwdkey; fk -= 16, ik += 16 )
        asm( "movdqu (%0), %%xmm0       \n\t"
             AESIMC  xmm0_xmm0         "\n\t"
             "movdqu %%xmm0, (%1)       \n\t"
             :
             : "r" (fk), "r" (ik)
             : "memory", "xmm0" );

    memcpy( ik, fk, 16 );
}

static void aesni_setkey_enc_128( unsigned char *rk,
                                  const unsigned char *key )
{
    asm( "movdqu (%1), %%xmm0               \n\t"         "movdqu %%xmm0, (%0)               \n\t"         "jmp 2f                            \n\t"
         "1:                                \n\t"
         "pshufd $0xff, %%xmm1, %%xmm1      \n\t"         "pxor %%xmm0, %%xmm1               \n\t"         "pslldq $4, %%xmm0                 \n\t"         "pxor %%xmm0, %%xmm1               \n\t"         "pslldq $4, %%xmm0                 \n\t"         "pxor %%xmm0, %%xmm1               \n\t"
         "pslldq $4, %%xmm0                 \n\t"
         "pxor %%xmm1, %%xmm0               \n\t"         "add $16, %0                       \n\t"         "movdqu %%xmm0, (%0)               \n\t"         "ret                               \n\t"

         "2:                                \n\t"
         AESKEYGENA xmm0_xmm1 ",0x01        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x02        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x04        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x08        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x10        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x20        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x40        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x80        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x1B        \n\tcall 1b \n\t"
         AESKEYGENA xmm0_xmm1 ",0x36        \n\tcall 1b \n\t"
         :
         : "r" (rk), "r" (key)
         : "memory", "cc", "0" );
}

static void aesni_setkey_enc_192( unsigned char *rk,
                                  const unsigned char *key )
{
    asm( "movdqu (%1), %%xmm0   \n\t"         "movdqu %%xmm0, (%0)   \n\t"
         "add $16, %0           \n\t"
         "movq 16(%1), %%xmm1   \n\t"
         "movq %%xmm1, (%0)     \n\t"
         "add $8, %0            \n\t"
         "jmp 2f                \n\t"
         "1:                            \n\t"
         "pshufd $0x55, %%xmm2, %%xmm2  \n\t"         "pxor %%xmm0, %%xmm2           \n\t"         "pslldq $4, %%xmm0             \n\t"         "pxor %%xmm0, %%xmm2           \n\t"
         "pslldq $4, %%xmm0             \n\t"
         "pxor %%xmm0, %%xmm2           \n\t"
         "pslldq $4, %%xmm0             \n\t"
         "pxor %%xmm2, %%xmm0           \n\t"         "movdqu %%xmm0, (%0)           \n\t"
         "add $16, %0                   \n\t"
         "pshufd $0xff, %%xmm0, %%xmm2  \n\t"         "pxor %%xmm1, %%xmm2           \n\t"         "pslldq $4, %%xmm1             \n\t"         "pxor %%xmm2, %%xmm1           \n\t"         "movq %%xmm1, (%0)             \n\t"
         "add $8, %0                    \n\t"
         "ret                           \n\t"

         "2:                            \n\t"
         AESKEYGENA xmm1_xmm2 ",0x01    \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x02    \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x04    \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x08    \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x10    \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x20    \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x40    \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x80    \n\tcall 1b \n\t"

         :
         : "r" (rk), "r" (key)
         : "memory", "cc", "0" );
}

static void aesni_setkey_enc_256( unsigned char *rk,
                                  const unsigned char *key )
{
    asm( "movdqu (%1), %%xmm0           \n\t"
         "movdqu %%xmm0, (%0)           \n\t"
         "add $16, %0                   \n\t"
         "movdqu 16(%1), %%xmm1         \n\t"
         "movdqu %%xmm1, (%0)           \n\t"
         "jmp 2f                        \n\t"
         "1:                                \n\t"
         "pshufd $0xff, %%xmm2, %%xmm2      \n\t"
         "pxor %%xmm0, %%xmm2               \n\t"
         "pslldq $4, %%xmm0                 \n\t"
         "pxor %%xmm0, %%xmm2               \n\t"
         "pslldq $4, %%xmm0                 \n\t"
         "pxor %%xmm0, %%xmm2               \n\t"
         "pslldq $4, %%xmm0                 \n\t"
         "pxor %%xmm2, %%xmm0               \n\t"
         "add $16, %0                       \n\t"
         "movdqu %%xmm0, (%0)               \n\t"

         AESKEYGENA xmm0_xmm2 ",0x00        \n\t"
         "pshufd $0xaa, %%xmm2, %%xmm2      \n\t"
         "pxor %%xmm1, %%xmm2               \n\t"
         "pslldq $4, %%xmm1                 \n\t"
         "pxor %%xmm1, %%xmm2               \n\t"
         "pslldq $4, %%xmm1                 \n\t"
         "pxor %%xmm1, %%xmm2               \n\t"
         "pslldq $4, %%xmm1                 \n\t"
         "pxor %%xmm2, %%xmm1               \n\t"
         "add $16, %0                       \n\t"
         "movdqu %%xmm1, (%0)               \n\t"
         "ret                               \n\t"

         "2:                                \n\t"
         AESKEYGENA xmm1_xmm2 ",0x01        \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x02        \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x04        \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x08        \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x10        \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x20        \n\tcall 1b \n\t"
         AESKEYGENA xmm1_xmm2 ",0x40        \n\tcall 1b \n\t"
         :
         : "r" (rk), "r" (key)
         : "memory", "cc", "0" );
}

int mbedtls_aesni_setkey_enc( unsigned char *rk,
                      const unsigned char *key,
                      size_t bits )
{
    switch( bits )
    {
        case 128: aesni_setkey_enc_128( rk, key ); break;
        case 192: aesni_setkey_enc_192( rk, key ); break;
        case 256: aesni_setkey_enc_256( rk, key ); break;
        default : return( MBEDTLS_ERR_AES_INVALID_KEY_LENGTH );
    }

    return( 0 );
}

#endif 
#endif 