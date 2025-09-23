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
#line 2 "lexer.cpp"

#line 4 "lexer.cpp"

#define  YY_INT_ALIGNED short int


#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 5
#define YY_FLEX_SUBMINOR_VERSION 35
#if YY_FLEX_SUBMINOR_VERSION > 0
#define FLEX_BETA
#endif


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>



#ifndef FLEXINT_H
#define FLEXINT_H


#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif

#include <inttypes.h>
typedef int8_t flex_int8_t;
typedef uint8_t flex_uint8_t;
typedef int16_t flex_int16_t;
typedef uint16_t flex_uint16_t;
typedef int32_t flex_int32_t;
typedef uint32_t flex_uint32_t;
#else
typedef signed char flex_int8_t;
typedef short int flex_int16_t;
typedef int flex_int32_t;
typedef unsigned char flex_uint8_t; 
typedef unsigned short int flex_uint16_t;
typedef unsigned int flex_uint32_t;

#ifndef INT8_MIN
#define INT8_MIN               (-128)
#endif
#ifndef INT16_MIN
#define INT16_MIN              (-32767-1)
#endif
#ifndef INT32_MIN
#define INT32_MIN              (-2147483647-1)
#endif
#ifndef INT8_MAX
#define INT8_MAX               (127)
#endif
#ifndef INT16_MAX
#define INT16_MAX              (32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX              (255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX             (65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX             (4294967295U)
#endif

#endif 
#endif 
#ifdef __cplusplus

#define YY_USE_CONST

#else    
#if defined (__STDC__)

#define YY_USE_CONST

#endif    #endif    
#ifdef YY_USE_CONST
#define yyconst const
#else
#define yyconst
#endif

#define YY_NULL 0

#define YY_SC_TO_UI(c) ((unsigned int) (unsigned char) c)

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#define yyin yyg->yyin_r
#define yyout yyg->yyout_r
#define yyextra yyg->yyextra_r
#define yyleng yyg->yyleng_r
#define yytext yyg->yytext_r
#define yylineno (YY_CURRENT_BUFFER_LVALUE->yy_bs_lineno)
#define yycolumn (YY_CURRENT_BUFFER_LVALUE->yy_bs_column)
#define yy_flex_debug yyg->yy_flex_debug_r

#define BEGIN yyg->yy_start = 1 + 2 *

#define YY_START ((yyg->yy_start - 1) / 2)
#define YYSTATE YY_START

#define YY_STATE_EOF(state) (YY_END_OF_BUFFER + state + 1)

#define YY_NEW_FILE yyrestart(yyin ,yyscanner )

#define YY_END_OF_BUFFER_CHAR 0

#ifndef YY_BUF_SIZE
#ifdef __ia64__
#define YY_BUF_SIZE 32768
#else
#define YY_BUF_SIZE 16384
#endif #endif

#define YY_STATE_BUF_SIZE   ((YY_BUF_SIZE + 2) * sizeof(yy_state_type))

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif

#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2

    #define YY_LESS_LINENO(n)
    
#define yyless(n) \
    do \
        { \\
        int yyless_macro_arg = (n); \
        YY_LESS_LINENO(yyless_macro_arg);\
        *yy_cp = yyg->yy_hold_char; \
        YY_RESTORE_YY_MORE_OFFSET \
        yyg->yy_c_buf_p = yy_cp = yy_bp + yyless_macro_arg - YY_MORE_ADJ; \
        YY_DO_BEFORE_ACTION;\
        } \
    while ( 0 )

#define unput(c) yyunput( c, yyg->yytext_ptr , yyscanner )

#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif

#ifndef YY_STRUCT_YY_BUFFER_STATE
#define YY_STRUCT_YY_BUFFER_STATE
struct yy_buffer_state
    {
    FILE *yy_input_file;

    char *yy_ch_buf;
    char *yy_buf_pos;

    yy_size_t yy_buf_size;

    int yy_n_chars;

    int yy_is_our_buffer;

    int yy_is_interactive;

    int yy_at_bol;

    int yy_bs_lineno;
    int yy_bs_column;
    
    int yy_fill_buffer;

    int yy_buffer_status;

#define YY_BUFFER_NEW 0
#define YY_BUFFER_NORMAL 1
#define YY_BUFFER_EOF_PENDING 2

    };
#endif 
#define YY_CURRENT_BUFFER ( yyg->yy_buffer_stack \
                          ? yyg->yy_buffer_stack[yyg->yy_buffer_stack_top] \
                          : NULL)

#define YY_CURRENT_BUFFER_LVALUE yyg->yy_buffer_stack[yyg->yy_buffer_stack_top]

void yyrestart (FILE *input_file ,yyscan_t yyscanner );
void yy_switch_to_buffer (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
YY_BUFFER_STATE yy_create_buffer (FILE *file,int size ,yyscan_t yyscanner );
void yy_delete_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
void yy_flush_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
void yypush_buffer_state (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
void yypop_buffer_state (yyscan_t yyscanner );

static void yyensure_buffer_stack (yyscan_t yyscanner );
static void yy_load_buffer_state (yyscan_t yyscanner );
static void yy_init_buffer (YY_BUFFER_STATE b,FILE *file ,yyscan_t yyscanner );

#define YY_FLUSH_BUFFER yy_flush_buffer(YY_CURRENT_BUFFER ,yyscanner)

YY_BUFFER_STATE yy_scan_buffer (char *base,yy_size_t size ,yyscan_t yyscanner );
YY_BUFFER_STATE yy_scan_string (yyconst char *yy_str ,yyscan_t yyscanner );
YY_BUFFER_STATE yy_scan_bytes (yyconst char *bytes,int len ,yyscan_t yyscanner );

void *yyalloc (yy_size_t ,yyscan_t yyscanner );
void *yyrealloc (void *,yy_size_t ,yyscan_t yyscanner );
void yyfree (void * ,yyscan_t yyscanner );

#define yy_new_buffer yy_create_buffer

#define yy_set_interactive(is_interactive) \
    { \
    if ( ! YY_CURRENT_BUFFER ){ \
        yyensure_buffer_stack (yyscanner); \
        YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner); \
    } \
    YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
    }

#define yy_set_bol(at_bol) \
    { \
    if ( ! YY_CURRENT_BUFFER ){\
        yyensure_buffer_stack (yyscanner); \
        YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner); \
    } \
    YY_CURRENT_BUFFER_LVALUE->yy_at_bol = at_bol; \
    }

#define YY_AT_BOL() (YY_CURRENT_BUFFER_LVALUE->yy_at_bol)


#define yywrap(n) 1
#define YY_SKIP_YYWRAP

typedef unsigned char YY_CHAR;

typedef int yy_state_type;

#define yytext_ptr yytext_r

static yy_state_type yy_get_previous_state (yyscan_t yyscanner );
static yy_state_type yy_try_NUL_trans (yy_state_type current_state  ,yyscan_t yyscanner);
static int yy_get_next_buffer (yyscan_t yyscanner );
static void yy_fatal_error (yyconst char msg[] ,yyscan_t yyscanner );

#define YY_DO_BEFORE_ACTION \
    yyg->yytext_ptr = yy_bp; \
    yyleng = (size_t) (yy_cp - yy_bp); \
    yyg->yy_hold_char = *yy_cp; \
    *yy_cp = '\0'; \
    yyg->yy_c_buf_p = yy_cp;

#define YY_NUM_RULES 76
#define YY_END_OF_BUFFER 77
struct yy_trans_info
    {
    flex_int32_t yy_verify;
    flex_int32_t yy_nxt;
    };
static yyconst flex_int16_t yy_accept[193] =
    {   0,
        7,    7,    0,    0,    0,    0,    9,    9,    7,    7,
       77,   75,    7,    7,   29,   75,   75,   41,   75,   75,
       64,   65,   37,   32,   71,   35,   70,   75,   14,   72,
       73,   25,   42,   26,   63,   63,   63,   66,   67,   63,
       63,   63,   63,   63,   63,   63,   63,   63,   63,   63,
       63,   63,   63,   68,   74,   69,    6,    6,    6,    5,
        3,    3,    9,    9,   75,   39,    7,   19,   24,    0,
       15,    0,   10,   11,    0,   40,   27,    0,   16,    0,
       36,   30,   31,   33,   34,    0,    2,    1,   13,   14,
        0,   21,   18,   23,   22,   63,   63,   63,   52,   63,

       63,   63,   63,   63,   63,   63,   63,   44,   63,   63,
       63,   63,   63,   63,   63,   63,   28,    4,    3,    9,
        0,    8,   38,   11,    0,    0,   12,    0,   16,    0,
        0,   17,   13,    0,    0,   13,   20,   13,   63,   63,
       63,   63,   63,   46,   63,   43,   54,   59,   63,   63,
       63,   63,   63,   63,    0,   13,   53,   63,   63,   45,
       63,   63,   63,   63,   57,   63,   63,   61,   55,   63,
       48,   63,   62,   63,   63,   63,   63,   63,   47,   63,
       63,   63,   63,   51,   58,   63,   63,   56,   60,   49,
       50,    0

    } ;

static yyconst flex_int32_t yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    1,    4,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    2,    5,    6,    1,    7,    8,    9,   10,   11,
       12,   13,   14,   15,   16,   17,   18,   19,   19,   19,
       19,   19,   19,   19,   19,   19,   19,   20,   21,   22,
       23,   24,    1,    1,   25,   26,   26,   26,   27,   28,
       26,   26,   29,   26,   26,   26,   26,   30,   26,   26,
       26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
       31,   32,   33,    1,   26,    1,   34,   35,   36,   26,

       37,   38,   39,   40,   41,   42,   43,   44,   26,   45,
       46,   26,   26,   47,   48,   49,   50,   26,   51,   26,
       26,   26,   52,   53,   54,   55,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,

        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1
    } ;

static yyconst flex_int32_t yy_meta[56] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    1,
        1,    1,    1,    1,    2,    2,    2,    2,    2,    2,
        1,    3,    1,    2,    2,    2,    2,    2,    2,    2,
        2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
        2,    4,    1,    1,    1
    } ;

static yyconst flex_int16_t yy_base[206] =
    {   0,
        0,    0,   53,   54,   55,   57,   61,   62,  263,  262,
      279,  282,   68,   71,   39,   57,   57,  255,  268,   67,
      282,  282,  253,   64,  282,   63,  282,   67,   64,  282,
      282,  252,   59,  251,    0,  243,  247,  282,  282,  223,
       46,  224,  225,   50,  234,   50,  233,  231,  228,  215,
      216,  221,  221,  282,  207,  282,  282,  282,  241,  282,
       94,   99,  251,  250,  245,   92,  104,  282,  282,   84,
      282,  252,  235,    0,   72,  282,  282,  101,  282,  109,
      282,  282,  282,  282,  282,  235,  282,  282,   94,  101,
      108,  228,  282,  282,  282,    0,  223,  220,    0,  203,

      211,  202,  198,  201,  197,  198,  194,    0,   85,  196,
      198,   83,  192,  188,  192,  195,  282,  282,  126,  228,
      223,  282,  282,    0,   93,  230,  282,  125,  126,  134,
      214,  282,  123,  129,  212,  211,  282,    0,  185,  194,
      178,  189,  177,    0,  188,    0,  179,  178,  177,  170,
      178,  181,  168,  172,  196,  195,    0,  170,  171,    0,
      174,  161,  167,  151,    0,  137,  120,    0,    0,  127,
        0,  118,    0,  121,  113,  111,  111,  116,    0,  104,
      107,  103,  102,    0,    0,  112,  101,    0,    0,    0,
        0,  282,  166,  170,  174,  178,  181,  185,  189,  138,

      193,  137,  197,  201,  205
    } ;

static yyconst flex_int16_t yy_def[206] =
    {   0,
      192,    1,  193,  193,  194,  194,  195,  195,    1,    1,
      192,  192,  192,  192,  192,  196,  197,  192,  192,  198,
      192,  192,  192,  192,  192,  192,  192,  199,  192,  192,
      192,  192,  192,  192,  200,  200,  200,  192,  192,  200,
      200,  200,  200,  200,  200,  200,  200,  200,  200,  200,
      200,  200,  200,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  201,  201,  201,  192,  192,  192,  192,  196,
      192,  196,  192,  202,  203,  192,  192,  198,  192,  204,
      192,  192,  192,  192,  192,  205,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  200,  200,  200,  200,  200,

      200,  200,  200,  200,  200,  200,  200,  200,  200,  200,
      200,  200,  200,  200,  200,  200,  192,  192,  192,  201,
      201,  192,  192,  202,  203,  203,  192,  198,  198,  204,
      205,  192,  192,  192,  192,  192,  192,  200,  200,  200,
      200,  200,  200,  200,  200,  200,  200,  200,  200,  200,
      200,  200,  200,  200,  192,  192,  200,  200,  200,  200,
      200,  200,  200,  200,  200,  200,  200,  200,  200,  200,
      200,  200,  200,  200,  200,  200,  200,  200,  200,  200,
      200,  200,  200,  200,  200,  200,  200,  200,  200,  200,
      200,    0,  192,  192,  192,  192,  192,  192,  192,  192,

      192,  192,  192,  192,  192
    } ;

static yyconst flex_int16_t yy_nxt[338] =
    {   0,
       12,   13,   14,   13,   15,   16,   17,   18,   19,   20,
       21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
       31,   32,   33,   34,   35,   35,   35,   35,   36,   37,
       38,   12,   39,   40,   41,   42,   43,   44,   35,   45,
       46,   35,   35,   35,   47,   48,   49,   50,   51,   52,
       53,   54,   55,   56,   12,   58,   58,   61,   62,   61,
       62,   68,   71,   64,   64,   59,   59,   65,   65,   67,
       67,   67,   67,   67,   67,   73,   79,   82,   84,   87,
       89,   93,   90,  104,   88,   85,   83,  108,   72,   71,
       91,  100,  101,   69,  109,  105,  119,  119,   80,  106,

       91,  119,  119,  126,   87,   67,   67,   67,   75,   88,
       79,   78,  133,   94,  123,   72,  149,   89,  129,   90,
      134,  135,  138,  135,  126,  127,  136,   91,  119,  119,
      134,  150,   80,  147,   79,   79,   78,   91,  124,   96,
      130,  133,  155,  129,  155,  191,  127,  156,  190,  134,
      189,  188,  187,  186,  185,  184,   80,   80,  183,  134,
      182,  181,  180,  179,  178,  130,   57,   57,   57,   57,
       60,   60,   60,   60,   63,   63,   63,   63,   70,   70,
       70,   70,   74,  177,   74,   78,   78,   78,   78,   86,
       86,  176,   86,  120,  120,  120,  120,  125,  125,  125,

      125,  128,  128,  128,  128,  131,  131,  175,  131,  174,
      173,  172,  171,  156,  156,  170,  169,  168,  167,  166,
      165,  164,  163,  162,  161,  160,  159,  158,  157,  136,
      136,  132,  192,  192,  121,  154,  153,  152,  151,  148,
      138,  146,  145,  144,  143,  142,  141,  140,  139,  138,
      138,  137,  132,   73,  192,  122,  121,  121,  118,  117,
      116,  115,  114,  113,  112,  111,  110,  107,  103,  102,
       99,   98,   97,   95,   92,   81,   77,   76,  192,   66,
       66,   11,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,

      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192
    } ;

static yyconst flex_int16_t yy_chk[338] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    3,    4,    5,    5,    6,
        6,   15,   16,    7,    8,    3,    4,    7,    8,   13,
       13,   13,   14,   14,   14,   17,   20,   24,   26,   28,
       29,   33,   29,   44,   28,   26,   24,   46,   16,   70,
       29,   41,   41,   15,   46,   44,   61,   61,   20,   44,

       29,   62,   62,   75,   66,   67,   67,   67,   17,   66,
       78,   80,   89,   33,   66,   70,  112,   90,   80,   90,
       89,   91,  109,   91,  125,   75,   91,   90,  119,  119,
       89,  112,   78,  109,  128,  129,  130,   90,  202,  200,
       80,  133,  134,  130,  134,  187,  125,  134,  186,  133,
      183,  182,  181,  180,  178,  177,  128,  129,  176,  133,
      175,  174,  172,  170,  167,  130,  193,  193,  193,  193,
      194,  194,  194,  194,  195,  195,  195,  195,  196,  196,
      196,  196,  197,  166,  197,  198,  198,  198,  198,  199,
      199,  164,  199,  201,  201,  201,  201,  203,  203,  203,

      203,  204,  204,  204,  204,  205,  205,  163,  205,  162,
      161,  159,  158,  156,  155,  154,  153,  152,  151,  150,
      149,  148,  147,  145,  143,  142,  141,  140,  139,  136,
      135,  131,  126,  121,  120,  116,  115,  114,  113,  111,
      110,  107,  106,  105,  104,  103,  102,  101,  100,   98,
       97,   92,   86,   73,   72,   65,   64,   63,   59,   55,
       53,   52,   51,   50,   49,   48,   47,   45,   43,   42,
       40,   37,   36,   34,   32,   23,   19,   18,   11,   10,
        9,  192,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,

      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192,  192,  192,  192,
      192,  192,  192,  192,  192,  192,  192
    } ;

#define REJECT reject_used_but_not_detected
#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0
#define YY_RESTORE_YY_MORE_OFFSET
#line 1 "lexer.l"
#line 16 "lexer.l"
#include <slib/utils/qlang/parserDriver.hpp>
#include <slib/std/string.hpp>
#include "lexerExtraDefs.hpp"
#include "parserDriverPrivate.hpp"

#define yyterminate() return token::END
#define YY_NO_UNISTD_H 1




#line 48 "lexer.l"
#define YY_USER_ACTION { \
    const char *c = yytext; \
    while (*c) { \
        if (*c == '\r' && c[1] == '\n') { \
            yylloc->lines(); \
            c++; \
        } else if (*c == '\r' || *c == '\n') { \
            yylloc->lines(); \
        } else \
            yylloc->columns(); \
        c++; \
    } \
}

#define YY_INPUT(buf,result,max_size) { (result) = yyget_extra(yyscanner)->yyInput((buf), (max_size)); }

#define SQLANG_YY_OK_REGEX if (YY_START == NO_REGEX) yy_pop_state(yyscanner)
#define SQLANG_YY_NO_REGEX if (YY_START == INITIAL) yy_push_state(NO_REGEX, yyscanner)
#line 627 "lexer.cpp"

#define INITIAL 0
#define COMMENT_C 1
#define COMMENT_CPP 2
#define TEMPLATE 3
#define NO_REGEX 4

#ifndef YY_NO_UNISTD_H
#include <unistd.h>
#endif

#define YY_EXTRA_TYPE slib::sQLangParserDriver *

struct yyguts_t
    {

    YY_EXTRA_TYPE yyextra_r;

    FILE *yyin_r, *yyout_r;
    size_t yy_buffer_stack_top;
    size_t yy_buffer_stack_max;
    YY_BUFFER_STATE * yy_buffer_stack;
    char yy_hold_char;
    int yy_n_chars;
    int yyleng_r;
    char *yy_c_buf_p;
    int yy_init;
    int yy_start;
    int yy_did_buffer_switch_on_eof;
    int yy_start_stack_ptr;
    int yy_start_stack_depth;
    int *yy_start_stack;
    yy_state_type yy_last_accepting_state;
    char* yy_last_accepting_cpos;

    int yylineno_r;
    int yy_flex_debug_r;

    char *yytext_r;
    int yy_more_flag;
    int yy_more_len;

    YYSTYPE * yylval_r;

    };

static int yy_init_globals (yyscan_t yyscanner );

    #    define yylval yyg->yylval_r
    
int yylex_init (yyscan_t* scanner);

int yylex_init_extra (YY_EXTRA_TYPE user_defined,yyscan_t* scanner);


int yylex_destroy (yyscan_t yyscanner );

int yyget_debug (yyscan_t yyscanner );

void yyset_debug (int debug_flag ,yyscan_t yyscanner );

YY_EXTRA_TYPE yyget_extra (yyscan_t yyscanner );

void yyset_extra (YY_EXTRA_TYPE user_defined ,yyscan_t yyscanner );

FILE *yyget_in (yyscan_t yyscanner );

void yyset_in  (FILE * in_str ,yyscan_t yyscanner );

FILE *yyget_out (yyscan_t yyscanner );

void yyset_out  (FILE * out_str ,yyscan_t yyscanner );

int yyget_leng (yyscan_t yyscanner );

char *yyget_text (yyscan_t yyscanner );

int yyget_lineno (yyscan_t yyscanner );

void yyset_lineno (int line_number ,yyscan_t yyscanner );

YYSTYPE * yyget_lval (yyscan_t yyscanner );

void yyset_lval (YYSTYPE * yylval_param ,yyscan_t yyscanner );


#ifndef YY_SKIP_YYWRAP
#ifdef __cplusplus
extern "C" int yywrap (yyscan_t yyscanner );
#else
extern int yywrap (yyscan_t yyscanner );
#endif
#endif

    static void yyunput (int c,char *buf_ptr  ,yyscan_t yyscanner);
    
#ifndef yytext_ptr
static void yy_flex_strncpy (char *,yyconst char *,int ,yyscan_t yyscanner);
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen (yyconst char * ,yyscan_t yyscanner);
#endif

#ifndef YY_NO_INPUT

#ifdef __cplusplus
static int yyinput (yyscan_t yyscanner );
#else
static int input (yyscan_t yyscanner );
#endif

#endif

    static void yy_push_state (int new_state ,yyscan_t yyscanner);
    
    static void yy_pop_state (yyscan_t yyscanner );
    
    static int yy_top_state (yyscan_t yyscanner );
    
#ifndef YY_READ_BUF_SIZE
#ifdef __ia64__
#define YY_READ_BUF_SIZE 16384
#else
#define YY_READ_BUF_SIZE 8192
#endif #endif

#ifndef ECHO
#define ECHO do { if (fwrite( yytext, yyleng, 1, yyout )) {} } while (0)
#endif

#ifndef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
    if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \
        { \
        int c = '*'; \
        size_t n; \
        for ( n = 0; n < max_size && \
                 (c = getc( yyin )) != EOF && c != '\n'; ++n ) \
            buf[n] = (char) c; \
        if ( c == '\n' ) \
            buf[n++] = (char) c; \
        if ( c == EOF && ferror( yyin ) ) \
            YY_FATAL_ERROR( "input in flex scanner failed" ); \
        result = n; \
        } \
    else \
        { \
        errno=0; \
        while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
            { \
            if( errno != EINTR) \
                { \
                YY_FATAL_ERROR( "input in flex scanner failed" ); \
                break; \
                } \
            errno=0; \
            clearerr(yyin); \
            } \
        }\
\

#endif

#ifndef yyterminate
#define yyterminate() return YY_NULL
#endif

#ifndef YY_START_STACK_INCR
#define YY_START_STACK_INCR 25
#endif

#ifndef YY_FATAL_ERROR
#define YY_FATAL_ERROR(msg) yy_fatal_error( msg , yyscanner)
#endif


#ifndef YY_DECL
#define YY_DECL_IS_OURS 1

extern int yylex \
               (YYSTYPE * yylval_param ,yyscan_t yyscanner);

#define YY_DECL int yylex \
               (YYSTYPE * yylval_param , yyscan_t yyscanner)
#endif 
#ifndef YY_USER_ACTION
#define YY_USER_ACTION
#endif

#ifndef YY_BREAK
#define YY_BREAK break;
#endif

#define YY_RULE_SETUP \
    YY_USER_ACTION

YY_DECL
{
    register yy_state_type yy_current_state;
    register char *yy_cp, *yy_bp;
    register int yy_act;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

#line 68 "lexer.l"



yylloc->step();


#line 880 "lexer.cpp"

    yylval = yylval_param;

    if ( !yyg->yy_init )
        {
        yyg->yy_init = 1;

#ifdef YY_USER_INIT
        YY_USER_INIT;
#endif

        if ( ! yyg->yy_start )
            yyg->yy_start = 1;

        if ( ! yyin )
            yyin = stdin;

        if ( ! yyout )
            yyout = stdout;

        if ( ! YY_CURRENT_BUFFER ) {
            yyensure_buffer_stack (yyscanner);
            YY_CURRENT_BUFFER_LVALUE =
                yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner);
        }

        yy_load_buffer_state(yyscanner );
        }

    while ( 1 )
        {
        yy_cp = yyg->yy_c_buf_p;

        *yy_cp = yyg->yy_hold_char;

        yy_bp = yy_cp;

        yy_current_state = yyg->yy_start;
yy_match:
        do
            {
            register YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
            if ( yy_accept[yy_current_state] )
                {
                yyg->yy_last_accepting_state = yy_current_state;
                yyg->yy_last_accepting_cpos = yy_cp;
                }
            while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
                {
                yy_current_state = (int) yy_def[yy_current_state];
                if ( yy_current_state >= 193 )
                    yy_c = yy_meta[(unsigned int) yy_c];
                }
            yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
            ++yy_cp;
            }
        while ( yy_current_state != 192 );
        yy_cp = yyg->yy_last_accepting_cpos;
        yy_current_state = yyg->yy_last_accepting_state;

yy_find_action:
        yy_act = yy_accept[yy_current_state];

        YY_DO_BEFORE_ACTION;

do_action:

        switch ( yy_act )
    {
            case 0:
            *yy_cp = yyg->yy_hold_char;
            yy_cp = yyg->yy_last_accepting_cpos;
            yy_current_state = yyg->yy_last_accepting_state;
            goto yy_find_action;

case 1:
YY_RULE_SETUP
#line 74 "lexer.l"
{ yy_push_state(COMMENT_CPP, yyscanner); }
    YY_BREAK
case 2:
YY_RULE_SETUP
#line 75 "lexer.l"
{ yy_push_state(COMMENT_C, yyscanner); }
    YY_BREAK
case YY_STATE_EOF(COMMENT_CPP):
#line 77 "lexer.l"
{ yy_pop_state(yyscanner); }
    YY_BREAK
case 3:
YY_RULE_SETUP
#line 78 "lexer.l"
{ yy_pop_state(yyscanner); }
    YY_BREAK
case YY_STATE_EOF(COMMENT_C):
#line 79 "lexer.l"
{
        driver.setError("unterminated comment", yylloc->begin.line, yylloc->begin.column);
        yyterminate();
    }
    YY_BREAK
case 4:
YY_RULE_SETUP
#line 83 "lexer.l"
{ yy_pop_state(yyscanner); }
    YY_BREAK
case 5:
YY_RULE_SETUP
#line 85 "lexer.l"
{ yylloc->step(); }
    YY_BREAK
case 6:
YY_RULE_SETUP
#line 86 "lexer.l"
{ yylloc->step(); }
    YY_BREAK
case 7:
YY_RULE_SETUP
#line 87 "lexer.l"
{ yylloc->step(); }
    YY_BREAK
case 8:
YY_RULE_SETUP
#line 89 "lexer.l"
{
        yy_push_state(INITIAL, yyscanner);
        return token::TMPL_CODE_START;
    }
    YY_BREAK
case 9:
YY_RULE_SETUP
#line 93 "lexer.l"
{
        yylval_param->strVal = new slib::sStr("%s", yytext);
        return token::TMPL_STRING;
    }
    YY_BREAK
case YY_STATE_EOF(TEMPLATE):
#line 97 "lexer.l"
{ yy_pop_state(yyscanner); SQLANG_YY_NO_REGEX; }
    YY_BREAK
case 10:
YY_RULE_SETUP
#line 99 "lexer.l"
{
        SQLANG_YY_NO_REGEX;
        yylval_param->intVal = atoidx(yytext + 1);
        return token::DOLLAR_NUM;
    }
    YY_BREAK
case 11:
YY_RULE_SETUP
#line 104 "lexer.l"
{
        SQLANG_YY_NO_REGEX;
        yylval_param->strVal = new slib::sStr();
        yytext++;
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::DOLLAR_NAME;
    }
    YY_BREAK
case 12:
YY_RULE_SETUP
#line 111 "lexer.l"
{
        SQLANG_YY_NO_REGEX;
        yylval_param->strVal = new slib::sStr();
        yytext += 2;
        yytext[strlen(yytext)-1] = 0;
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::DOLLAR_NAME;
    }
    YY_BREAK
case 13:
YY_RULE_SETUP
#line 120 "lexer.l"
{ SQLANG_YY_NO_REGEX; yylval->realVal = atof(yytext); return token::REAL_LITERAL; }
    YY_BREAK
case 14:
YY_RULE_SETUP
#line 121 "lexer.l"
{ SQLANG_YY_NO_REGEX; yylval_param->intVal = atoidx(yytext); return token::INT_LITERAL; }
    YY_BREAK
case 15:
YY_RULE_SETUP
#line 122 "lexer.l"
{
        SQLANG_YY_NO_REGEX;
        yytext++;
        yytext[strlen(yytext)-1] = 0;
        yylval_param->strVal = new slib::sStr();
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::STRING_LITERAL;
    }
    YY_BREAK
case 16:
YY_RULE_SETUP
#line 130 "lexer.l"
{
        SQLANG_YY_NO_REGEX;
        yytext++;
        yytext[strlen(yytext)-1] = 0;
        yylval_param->strVal = new slib::sStr();
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::STRING_LITERAL;
    }
    YY_BREAK
case 17:
YY_RULE_SETUP
#line 138 "lexer.l"
{
        SQLANG_YY_NO_REGEX;
        yytext++;
        yytext[strlen(yytext)-1] = 0;
        yylval_param->strVal = new slib::sStr("%s", yytext);
        return token::REGEX_LITERAL;
    }
    YY_BREAK
case 18:
YY_RULE_SETUP
#line 146 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::EQ; }
    YY_BREAK
case 19:
YY_RULE_SETUP
#line 147 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::NE; }
    YY_BREAK
case 20:
YY_RULE_SETUP
#line 148 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::CMP; }
    YY_BREAK
case 21:
YY_RULE_SETUP
#line 149 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::LE; }
    YY_BREAK
case 22:
YY_RULE_SETUP
#line 150 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::GE; }
    YY_BREAK
case 23:
YY_RULE_SETUP
#line 151 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::MATCH; }
    YY_BREAK
case 24:
YY_RULE_SETUP
#line 152 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::NMATCH; }
    YY_BREAK
case 25:
YY_RULE_SETUP
#line 153 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '<'; }
    YY_BREAK
case 26:
YY_RULE_SETUP
#line 154 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '>'; }
    YY_BREAK
case 27:
YY_RULE_SETUP
#line 155 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::AND; }
    YY_BREAK
case 28:
YY_RULE_SETUP
#line 156 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::OR; }
    YY_BREAK
case 29:
YY_RULE_SETUP
#line 157 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '!'; }
    YY_BREAK
case 30:
YY_RULE_SETUP
#line 158 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::INCREMENT; }
    YY_BREAK
case 31:
YY_RULE_SETUP
#line 159 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::PLUS_INPLACE; }
    YY_BREAK
case 32:
YY_RULE_SETUP
#line 160 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '+'; }
    YY_BREAK
case 33:
YY_RULE_SETUP
#line 161 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::DECREMENT; }
    YY_BREAK
case 34:
YY_RULE_SETUP
#line 162 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::MINUS_INPLACE; }
    YY_BREAK
case 35:
YY_RULE_SETUP
#line 163 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '-'; }
    YY_BREAK
case 36:
YY_RULE_SETUP
#line 164 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::MULTIPLY_INPLACE; }
    YY_BREAK
case 37:
YY_RULE_SETUP
#line 165 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '*'; }
    YY_BREAK
case 38:
YY_RULE_SETUP
#line 166 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::DIVIDE_INPLACE; }
    YY_BREAK
case 39:
YY_RULE_SETUP
#line 167 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '/'; }
    YY_BREAK
case 40:
YY_RULE_SETUP
#line 168 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::REMAINDER_INPLACE; }
    YY_BREAK
case 41:
YY_RULE_SETUP
#line 169 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '%'; }
    YY_BREAK
case 42:
YY_RULE_SETUP
#line 170 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '='; }
    YY_BREAK
case 43:
YY_RULE_SETUP
#line 172 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::HAS; }
    YY_BREAK
case 44:
YY_RULE_SETUP
#line 173 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::IF; }
    YY_BREAK
case 45:
YY_RULE_SETUP
#line 174 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::ELSE; }
    YY_BREAK
case 46:
YY_RULE_SETUP
#line 175 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::FOR; }
    YY_BREAK
case 47:
YY_RULE_SETUP
#line 176 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::WHILE; }
    YY_BREAK
case 48:
YY_RULE_SETUP
#line 177 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::BREAK; }
    YY_BREAK
case 49:
YY_RULE_SETUP
#line 178 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::CONTINUE; }
    YY_BREAK
case 50:
YY_RULE_SETUP
#line 179 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::FUNCTION; }
    YY_BREAK
case 51:
YY_RULE_SETUP
#line 180 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::RETURN; }
    YY_BREAK
case 52:
YY_RULE_SETUP
#line 181 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::AS; }
    YY_BREAK
case 53:
YY_RULE_SETUP
#line 182 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::BOOL; }
    YY_BREAK
case 54:
YY_RULE_SETUP
#line 183 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::INT; }
    YY_BREAK
case 55:
YY_RULE_SETUP
#line 184 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::UINT; }
    YY_BREAK
case 56:
YY_RULE_SETUP
#line 185 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::INTLIST; }
    YY_BREAK
case 57:
YY_RULE_SETUP
#line 186 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::REAL; }
    YY_BREAK
case 58:
YY_RULE_SETUP
#line 187 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::STRING; }
    YY_BREAK
case 59:
YY_RULE_SETUP
#line 188 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::OBJ; }
    YY_BREAK
case 60:
YY_RULE_SETUP
#line 189 "lexer.l"
{ SQLANG_YY_OK_REGEX; return token::OBJLIST; }
    YY_BREAK
case 61:
YY_RULE_SETUP
#line 191 "lexer.l"
{ SQLANG_YY_NO_REGEX; yylval_param->intVal = 1; return token::INT_LITERAL; }
    YY_BREAK
case 62:
YY_RULE_SETUP
#line 192 "lexer.l"
{ SQLANG_YY_NO_REGEX; yylval_param->intVal = 0; return token::INT_LITERAL; }
    YY_BREAK
case 63:
YY_RULE_SETUP
#line 194 "lexer.l"
{
        SQLANG_YY_NO_REGEX;
        yylval_param->strVal = new slib::sStr("%s", yytext);
        return token::NAME;
    }
    YY_BREAK
case 64:
YY_RULE_SETUP
#line 200 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '('; }
    YY_BREAK
case 65:
YY_RULE_SETUP
#line 201 "lexer.l"
{ SQLANG_YY_NO_REGEX; return ')'; }
    YY_BREAK
case 66:
YY_RULE_SETUP
#line 202 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '['; }
    YY_BREAK
case 67:
YY_RULE_SETUP
#line 203 "lexer.l"
{ SQLANG_YY_NO_REGEX; return ']'; }
    YY_BREAK
case 68:
YY_RULE_SETUP
#line 204 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '{'; }
    YY_BREAK
case 69:
YY_RULE_SETUP
#line 205 "lexer.l"
{ SQLANG_YY_NO_REGEX; return '}'; }
    YY_BREAK
case 70:
YY_RULE_SETUP
#line 206 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '.'; }
    YY_BREAK
case 71:
YY_RULE_SETUP
#line 207 "lexer.l"
{ SQLANG_YY_OK_REGEX; return ','; }
    YY_BREAK
case 72:
YY_RULE_SETUP
#line 208 "lexer.l"
{ SQLANG_YY_OK_REGEX; return ':'; }
    YY_BREAK
case 73:
YY_RULE_SETUP
#line 209 "lexer.l"
{ SQLANG_YY_OK_REGEX; return ';'; }
    YY_BREAK
case 74:
YY_RULE_SETUP
#line 210 "lexer.l"
{ SQLANG_YY_OK_REGEX; return '|'; }
    YY_BREAK
case 75:
YY_RULE_SETUP
#line 212 "lexer.l"
{
        driver.setError("invalid character", yylloc->begin.line, yylloc->begin.column);
        yyterminate();
    }
    YY_BREAK
case 76:
YY_RULE_SETUP
#line 217 "lexer.l"
YY_FATAL_ERROR( "flex scanner jammed" );
    YY_BREAK
#line 1414 "lexer.cpp"
case YY_STATE_EOF(INITIAL):
case YY_STATE_EOF(NO_REGEX):
    yyterminate();

    case YY_END_OF_BUFFER:
        {
        int yy_amount_of_matched_text = (int) (yy_cp - yyg->yytext_ptr) - 1;

        *yy_cp = yyg->yy_hold_char;
        YY_RESTORE_YY_MORE_OFFSET

        if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_NEW )
            {
            yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
            YY_CURRENT_BUFFER_LVALUE->yy_input_file = yyin;
            YY_CURRENT_BUFFER_LVALUE->yy_buffer_status = YY_BUFFER_NORMAL;
            }

        if ( yyg->yy_c_buf_p <= &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
            {
            yy_state_type yy_next_state;

            yyg->yy_c_buf_p = yyg->yytext_ptr + yy_amount_of_matched_text;

            yy_current_state = yy_get_previous_state( yyscanner );


            yy_next_state = yy_try_NUL_trans( yy_current_state , yyscanner);

            yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;

            if ( yy_next_state )
                {
                yy_cp = ++yyg->yy_c_buf_p;
                yy_current_state = yy_next_state;
                goto yy_match;
                }

            else
                {
                yy_cp = yyg->yy_last_accepting_cpos;
                yy_current_state = yyg->yy_last_accepting_state;
                goto yy_find_action;
                }
            }

        else switch ( yy_get_next_buffer( yyscanner ) )
            {
            case EOB_ACT_END_OF_FILE:
                {
                yyg->yy_did_buffer_switch_on_eof = 0;

                if ( yywrap(yyscanner ) )
                    {
                    yyg->yy_c_buf_p = yyg->yytext_ptr + YY_MORE_ADJ;

                    yy_act = YY_STATE_EOF(YY_START);
                    goto do_action;
                    }

                else
                    {
                    if ( ! yyg->yy_did_buffer_switch_on_eof )
                        YY_NEW_FILE;
                    }
                break;
                }

            case EOB_ACT_CONTINUE_SCAN:
                yyg->yy_c_buf_p =
                    yyg->yytext_ptr + yy_amount_of_matched_text;

                yy_current_state = yy_get_previous_state( yyscanner );

                yy_cp = yyg->yy_c_buf_p;
                yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
                goto yy_match;

            case EOB_ACT_LAST_MATCH:
                yyg->yy_c_buf_p =
                &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars];

                yy_current_state = yy_get_previous_state( yyscanner );

                yy_cp = yyg->yy_c_buf_p;
                yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
                goto yy_find_action;
            }
        break;
        }

    default:
        YY_FATAL_ERROR(
            "fatal flex scanner internal error--no action found" );
    }
        }
}

static int yy_get_next_buffer (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    register char *dest = YY_CURRENT_BUFFER_LVALUE->yy_ch_buf;
    register char *source = yyg->yytext_ptr;
    register int number_to_move, i;
    int ret_val;

    if ( yyg->yy_c_buf_p > &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] )
        YY_FATAL_ERROR(
        "fatal flex scanner internal error--end of buffer missed" );

    if ( YY_CURRENT_BUFFER_LVALUE->yy_fill_buffer == 0 )
        {
        if ( yyg->yy_c_buf_p - yyg->yytext_ptr - YY_MORE_ADJ == 1 )
            {
            return EOB_ACT_END_OF_FILE;
            }

        else
            {
            return EOB_ACT_LAST_MATCH;
            }
        }


    number_to_move = (int) (yyg->yy_c_buf_p - yyg->yytext_ptr) - 1;

    for ( i = 0; i < number_to_move; ++i )
        *(dest++) = *(source++);

    if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_EOF_PENDING )
        YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars = 0;

    else
        {
            int num_to_read =
            YY_CURRENT_BUFFER_LVALUE->yy_buf_size - number_to_move - 1;

        while ( num_to_read <= 0 )
            {

            YY_BUFFER_STATE b = YY_CURRENT_BUFFER;

            int yy_c_buf_p_offset =
                (int) (yyg->yy_c_buf_p - b->yy_ch_buf);

            if ( b->yy_is_our_buffer )
                {
                int new_size = b->yy_buf_size * 2;

                if ( new_size <= 0 )
                    b->yy_buf_size += b->yy_buf_size / 8;
                else
                    b->yy_buf_size *= 2;

                b->yy_ch_buf = (char *)
                    yyrealloc((void *) b->yy_ch_buf,b->yy_buf_size + 2 ,yyscanner );
                }
            else
                b->yy_ch_buf = 0;

            if ( ! b->yy_ch_buf )
                YY_FATAL_ERROR(
                "fatal error - scanner input buffer overflow" );

            yyg->yy_c_buf_p = &b->yy_ch_buf[yy_c_buf_p_offset];

            num_to_read = YY_CURRENT_BUFFER_LVALUE->yy_buf_size -
                        number_to_move - 1;

            }

        if ( num_to_read > YY_READ_BUF_SIZE )
            num_to_read = YY_READ_BUF_SIZE;

        YY_INPUT( (&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),
            yyg->yy_n_chars, (size_t) num_to_read );

        YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
        }

    if ( yyg->yy_n_chars == 0 )
        {
        if ( number_to_move == YY_MORE_ADJ )
            {
            ret_val = EOB_ACT_END_OF_FILE;
            yyrestart(yyin  ,yyscanner);
            }

        else
            {
            ret_val = EOB_ACT_LAST_MATCH;
            YY_CURRENT_BUFFER_LVALUE->yy_buffer_status =
                YY_BUFFER_EOF_PENDING;
            }
        }

    else
        ret_val = EOB_ACT_CONTINUE_SCAN;

    if ((yy_size_t) (yyg->yy_n_chars + number_to_move) > YY_CURRENT_BUFFER_LVALUE->yy_buf_size) {
        yy_size_t new_size = yyg->yy_n_chars + number_to_move + (yyg->yy_n_chars >> 1);
        YY_CURRENT_BUFFER_LVALUE->yy_ch_buf = (char *) yyrealloc((void *) YY_CURRENT_BUFFER_LVALUE->yy_ch_buf,new_size ,yyscanner );
        if ( ! YY_CURRENT_BUFFER_LVALUE->yy_ch_buf )
            YY_FATAL_ERROR( "out of dynamic memory in yy_get_next_buffer()" );
    }

    yyg->yy_n_chars += number_to_move;
    YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] = YY_END_OF_BUFFER_CHAR;
    YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] = YY_END_OF_BUFFER_CHAR;

    yyg->yytext_ptr = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[0];

    return ret_val;
}


    static yy_state_type yy_get_previous_state (yyscan_t yyscanner)
{
    register yy_state_type yy_current_state;
    register char *yy_cp;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yy_current_state = yyg->yy_start;

    for ( yy_cp = yyg->yytext_ptr + YY_MORE_ADJ; yy_cp < yyg->yy_c_buf_p; ++yy_cp )
        {
        register YY_CHAR yy_c = (*yy_cp ? yy_ec[YY_SC_TO_UI(*yy_cp)] : 1);
        if ( yy_accept[yy_current_state] )
            {
            yyg->yy_last_accepting_state = yy_current_state;
            yyg->yy_last_accepting_cpos = yy_cp;
            }
        while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
            {
            yy_current_state = (int) yy_def[yy_current_state];
            if ( yy_current_state >= 193 )
                yy_c = yy_meta[(unsigned int) yy_c];
            }
        yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
        }

    return yy_current_state;
}

    static yy_state_type yy_try_NUL_trans  (yy_state_type yy_current_state , yyscan_t yyscanner)
{
    register int yy_is_jam;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    register char *yy_cp = yyg->yy_c_buf_p;

    register YY_CHAR yy_c = 1;
    if ( yy_accept[yy_current_state] )
        {
        yyg->yy_last_accepting_state = yy_current_state;
        yyg->yy_last_accepting_cpos = yy_cp;
        }
    while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
        {
        yy_current_state = (int) yy_def[yy_current_state];
        if ( yy_current_state >= 193 )
            yy_c = yy_meta[(unsigned int) yy_c];
        }
    yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
    yy_is_jam = (yy_current_state == 192);

    return yy_is_jam ? 0 : yy_current_state;
}

    static void yyunput (int c, register char * yy_bp , yyscan_t yyscanner)
{
    register char *yy_cp;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yy_cp = yyg->yy_c_buf_p;

    *yy_cp = yyg->yy_hold_char;

    if ( yy_cp < YY_CURRENT_BUFFER_LVALUE->yy_ch_buf + 2 )
        {
        register int number_to_move = yyg->yy_n_chars + 2;
        register char *dest = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[
                    YY_CURRENT_BUFFER_LVALUE->yy_buf_size + 2];
        register char *source =
                &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move];

        while ( source > YY_CURRENT_BUFFER_LVALUE->yy_ch_buf )
            *--dest = *--source;

        yy_cp += (int) (dest - source);
        yy_bp += (int) (dest - source);
        YY_CURRENT_BUFFER_LVALUE->yy_n_chars =
            yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_buf_size;

        if ( yy_cp < YY_CURRENT_BUFFER_LVALUE->yy_ch_buf + 2 )
            YY_FATAL_ERROR( "flex scanner push-back overflow" );
        }

    *--yy_cp = (char) c;

    yyg->yytext_ptr = yy_bp;
    yyg->yy_hold_char = *yy_cp;
    yyg->yy_c_buf_p = yy_cp;
}

#ifndef YY_NO_INPUT
#ifdef __cplusplus
    static int yyinput (yyscan_t yyscanner)
#else
    static int input  (yyscan_t yyscanner)
#endif

{
    int c;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    *yyg->yy_c_buf_p = yyg->yy_hold_char;

    if ( *yyg->yy_c_buf_p == YY_END_OF_BUFFER_CHAR )
        {
        if ( yyg->yy_c_buf_p < &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
            *yyg->yy_c_buf_p = '\0';

        else
            {
            int offset = yyg->yy_c_buf_p - yyg->yytext_ptr;
            ++yyg->yy_c_buf_p;

            switch ( yy_get_next_buffer( yyscanner ) )
                {
                case EOB_ACT_LAST_MATCH:

                    yyrestart(yyin ,yyscanner);


                case EOB_ACT_END_OF_FILE:
                    {
                    if ( yywrap(yyscanner ) )
                        return EOF;

                    if ( ! yyg->yy_did_buffer_switch_on_eof )
                        YY_NEW_FILE;
#ifdef __cplusplus
                    return yyinput(yyscanner);
#else
                    return input(yyscanner);
#endif
                    }

                case EOB_ACT_CONTINUE_SCAN:
                    yyg->yy_c_buf_p = yyg->yytext_ptr + offset;
                    break;
                }
            }
        }

    c = *(unsigned char *) yyg->yy_c_buf_p;
    *yyg->yy_c_buf_p = '\0';
    yyg->yy_hold_char = *++yyg->yy_c_buf_p;

    return c;
}
#endif    
    void yyrestart  (FILE * input_file , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    if ( ! YY_CURRENT_BUFFER ){
        yyensure_buffer_stack (yyscanner);
        YY_CURRENT_BUFFER_LVALUE =
            yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner);
    }

    yy_init_buffer(YY_CURRENT_BUFFER,input_file ,yyscanner);
    yy_load_buffer_state(yyscanner );
}

    void yy_switch_to_buffer  (YY_BUFFER_STATE  new_buffer , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yyensure_buffer_stack (yyscanner);
    if ( YY_CURRENT_BUFFER == new_buffer )
        return;

    if ( YY_CURRENT_BUFFER )
        {
        *yyg->yy_c_buf_p = yyg->yy_hold_char;
        YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
        YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
        }

    YY_CURRENT_BUFFER_LVALUE = new_buffer;
    yy_load_buffer_state(yyscanner );

    yyg->yy_did_buffer_switch_on_eof = 1;
}

static void yy_load_buffer_state  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
    yyg->yytext_ptr = yyg->yy_c_buf_p = YY_CURRENT_BUFFER_LVALUE->yy_buf_pos;
    yyin = YY_CURRENT_BUFFER_LVALUE->yy_input_file;
    yyg->yy_hold_char = *yyg->yy_c_buf_p;
}

    YY_BUFFER_STATE yy_create_buffer  (FILE * file, int  size , yyscan_t yyscanner)
{
    YY_BUFFER_STATE b;
    
    b = (YY_BUFFER_STATE) yyalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
    if ( ! b )
        YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );

    b->yy_buf_size = size;

    b->yy_ch_buf = (char *) yyalloc(b->yy_buf_size + 2 ,yyscanner );
    if ( ! b->yy_ch_buf )
        YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );

    b->yy_is_our_buffer = 1;

    yy_init_buffer(b,file ,yyscanner);

    return b;
}

    void yy_delete_buffer (YY_BUFFER_STATE  b , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    if ( ! b )
        return;

    if ( b == YY_CURRENT_BUFFER )
        YY_CURRENT_BUFFER_LVALUE = (YY_BUFFER_STATE) 0;

    if ( b->yy_is_our_buffer )
        yyfree((void *) b->yy_ch_buf ,yyscanner );

    yyfree((void *) b ,yyscanner );
}

    static void yy_init_buffer  (YY_BUFFER_STATE  b, FILE * file , yyscan_t yyscanner)

{
    int oerrno = errno;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yy_flush_buffer(b ,yyscanner);

    b->yy_input_file = file;
    b->yy_fill_buffer = 1;

    if (b != YY_CURRENT_BUFFER){
        b->yy_bs_lineno = 1;
        b->yy_bs_column = 0;
    }

        b->yy_is_interactive = 0;
    
    errno = oerrno;
}

    void yy_flush_buffer (YY_BUFFER_STATE  b , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if ( ! b )
        return;

    b->yy_n_chars = 0;

    b->yy_ch_buf[0] = YY_END_OF_BUFFER_CHAR;
    b->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;

    b->yy_buf_pos = &b->yy_ch_buf[0];

    b->yy_at_bol = 1;
    b->yy_buffer_status = YY_BUFFER_NEW;

    if ( b == YY_CURRENT_BUFFER )
        yy_load_buffer_state(yyscanner );
}

void yypush_buffer_state (YY_BUFFER_STATE new_buffer , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if (new_buffer == NULL)
        return;

    yyensure_buffer_stack(yyscanner);

    if ( YY_CURRENT_BUFFER )
        {
        *yyg->yy_c_buf_p = yyg->yy_hold_char;
        YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
        YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
        }

    if (YY_CURRENT_BUFFER)
        yyg->yy_buffer_stack_top++;
    YY_CURRENT_BUFFER_LVALUE = new_buffer;

    yy_load_buffer_state(yyscanner );
    yyg->yy_did_buffer_switch_on_eof = 1;
}

void yypop_buffer_state (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if (!YY_CURRENT_BUFFER)
        return;

    yy_delete_buffer(YY_CURRENT_BUFFER ,yyscanner);
    YY_CURRENT_BUFFER_LVALUE = NULL;
    if (yyg->yy_buffer_stack_top > 0)
        --yyg->yy_buffer_stack_top;

    if (YY_CURRENT_BUFFER) {
        yy_load_buffer_state(yyscanner );
        yyg->yy_did_buffer_switch_on_eof = 1;
    }
}

static void yyensure_buffer_stack (yyscan_t yyscanner)
{
    int num_to_alloc;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    if (!yyg->yy_buffer_stack) {

        num_to_alloc = 1;
        yyg->yy_buffer_stack = (struct yy_buffer_state**)yyalloc
                                (num_to_alloc * sizeof(struct yy_buffer_state*)
                                , yyscanner);
        if ( ! yyg->yy_buffer_stack )
            YY_FATAL_ERROR( "out of dynamic memory in yyensure_buffer_stack()" );
                                  
        memset(yyg->yy_buffer_stack, 0, num_to_alloc * sizeof(struct yy_buffer_state*));
                
        yyg->yy_buffer_stack_max = num_to_alloc;
        yyg->yy_buffer_stack_top = 0;
        return;
    }

    if (yyg->yy_buffer_stack_top >= (yyg->yy_buffer_stack_max) - 1){

        int grow_size = 8;

        num_to_alloc = yyg->yy_buffer_stack_max + grow_size;
        yyg->yy_buffer_stack = (struct yy_buffer_state**)yyrealloc
                                (yyg->yy_buffer_stack,
                                num_to_alloc * sizeof(struct yy_buffer_state*)
                                , yyscanner);
        if ( ! yyg->yy_buffer_stack )
            YY_FATAL_ERROR( "out of dynamic memory in yyensure_buffer_stack()" );

        memset(yyg->yy_buffer_stack + yyg->yy_buffer_stack_max, 0, grow_size * sizeof(struct yy_buffer_state*));
        yyg->yy_buffer_stack_max = num_to_alloc;
    }
}

YY_BUFFER_STATE yy_scan_buffer  (char * base, yy_size_t  size , yyscan_t yyscanner)
{
    YY_BUFFER_STATE b;
    
    if ( size < 2 ||
         base[size-2] != YY_END_OF_BUFFER_CHAR ||
         base[size-1] != YY_END_OF_BUFFER_CHAR )
        return 0;

    b = (YY_BUFFER_STATE) yyalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
    if ( ! b )
        YY_FATAL_ERROR( "out of dynamic memory in yy_scan_buffer()" );

    b->yy_buf_size = size - 2;
    b->yy_buf_pos = b->yy_ch_buf = base;
    b->yy_is_our_buffer = 0;
    b->yy_input_file = 0;
    b->yy_n_chars = b->yy_buf_size;
    b->yy_is_interactive = 0;
    b->yy_at_bol = 1;
    b->yy_fill_buffer = 0;
    b->yy_buffer_status = YY_BUFFER_NEW;

    yy_switch_to_buffer(b ,yyscanner );

    return b;
}

YY_BUFFER_STATE yy_scan_string (yyconst char * yystr , yyscan_t yyscanner)
{
    
    return yy_scan_bytes(yystr,strlen(yystr) ,yyscanner);
}

YY_BUFFER_STATE yy_scan_bytes  (yyconst char * yybytes, int  _yybytes_len , yyscan_t yyscanner)
{
    YY_BUFFER_STATE b;
    char *buf;
    yy_size_t n;
    int i;
    
    n = _yybytes_len + 2;
    buf = (char *) yyalloc(n ,yyscanner );
    if ( ! buf )
        YY_FATAL_ERROR( "out of dynamic memory in yy_scan_bytes()" );

    for ( i = 0; i < _yybytes_len; ++i )
        buf[i] = yybytes[i];

    buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;

    b = yy_scan_buffer(buf,n ,yyscanner);
    if ( ! b )
        YY_FATAL_ERROR( "bad buffer in yy_scan_bytes()" );

    b->yy_is_our_buffer = 1;

    return b;
}

    static void yy_push_state (int  new_state , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if ( yyg->yy_start_stack_ptr >= yyg->yy_start_stack_depth )
        {
        yy_size_t new_size;

        yyg->yy_start_stack_depth += YY_START_STACK_INCR;
        new_size = yyg->yy_start_stack_depth * sizeof( int );

        if ( ! yyg->yy_start_stack )
            yyg->yy_start_stack = (int *) yyalloc(new_size ,yyscanner );

        else
            yyg->yy_start_stack = (int *) yyrealloc((void *) yyg->yy_start_stack,new_size ,yyscanner );

        if ( ! yyg->yy_start_stack )
            YY_FATAL_ERROR( "out of memory expanding start-condition stack" );
        }

    yyg->yy_start_stack[yyg->yy_start_stack_ptr++] = YY_START;

    BEGIN(new_state);
}

    static void yy_pop_state  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if ( --yyg->yy_start_stack_ptr < 0 )
        YY_FATAL_ERROR( "start-condition stack underflow" );

    BEGIN(yyg->yy_start_stack[yyg->yy_start_stack_ptr]);
}

    static int yy_top_state  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyg->yy_start_stack[yyg->yy_start_stack_ptr - 1];
}

#ifndef YY_EXIT_FAILURE
#define YY_EXIT_FAILURE 2
#endif

static void yy_fatal_error (yyconst char* msg , yyscan_t yyscanner)
{
        (void) fprintf( stderr, "%s\n", msg );
    exit( YY_EXIT_FAILURE );
}


#undef yyless
#define yyless(n) \
    do \
        { \\
        int yyless_macro_arg = (n); \
        YY_LESS_LINENO(yyless_macro_arg);\
        yytext[yyleng] = yyg->yy_hold_char; \
        yyg->yy_c_buf_p = yytext + yyless_macro_arg; \
        yyg->yy_hold_char = *yyg->yy_c_buf_p; \
        *yyg->yy_c_buf_p = '\0'; \
        yyleng = yyless_macro_arg; \
        } \
    while ( 0 )


YY_EXTRA_TYPE yyget_extra  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyextra;
}

int yyget_lineno  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    
        if (! YY_CURRENT_BUFFER)
            return 0;
    
    return yylineno;
}

int yyget_column  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    
        if (! YY_CURRENT_BUFFER)
            return 0;
    
    return yycolumn;
}

FILE *yyget_in  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyin;
}

FILE *yyget_out  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyout;
}

int yyget_leng  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yyleng;
}


char *yyget_text  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yytext;
}

void yyset_extra (YY_EXTRA_TYPE  user_defined , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyextra = user_defined ;
}

void yyset_lineno (int  line_number , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

        if (! YY_CURRENT_BUFFER )
           yy_fatal_error( "yyset_lineno called with no buffer" , yyscanner); 
    
    yylineno = line_number;
}

void yyset_column (int  column_no , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

        if (! YY_CURRENT_BUFFER )
           yy_fatal_error( "yyset_column called with no buffer" , yyscanner); 
    
    yycolumn = column_no;
}

void yyset_in (FILE *  in_str , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyin = in_str ;
}

void yyset_out (FILE *  out_str , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyout = out_str ;
}

int yyget_debug  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yy_flex_debug;
}

void yyset_debug (int  bdebug , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yy_flex_debug = bdebug ;
}


YYSTYPE * yyget_lval  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yylval;
}

void yyset_lval (YYSTYPE *  yylval_param , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yylval = yylval_param;
}



int yylex_init(yyscan_t* ptr_yy_globals)

{
    if (ptr_yy_globals == NULL){
        errno = EINVAL;
        return 1;
    }

    *ptr_yy_globals = (yyscan_t) yyalloc ( sizeof( struct yyguts_t ), NULL );

    if (*ptr_yy_globals == NULL){
        errno = ENOMEM;
        return 1;
    }

    memset(*ptr_yy_globals,0x00,sizeof(struct yyguts_t));

    return yy_init_globals ( *ptr_yy_globals );
}


int yylex_init_extra(YY_EXTRA_TYPE yy_user_defined,yyscan_t* ptr_yy_globals )

{
    struct yyguts_t dummy_yyguts;

    yyset_extra (yy_user_defined, &dummy_yyguts);

    if (ptr_yy_globals == NULL){
        errno = EINVAL;
        return 1;
    }
    
    *ptr_yy_globals = (yyscan_t) yyalloc ( sizeof( struct yyguts_t ), &dummy_yyguts );
    
    if (*ptr_yy_globals == NULL){
        errno = ENOMEM;
        return 1;
    }
    
    memset(*ptr_yy_globals,0x00,sizeof(struct yyguts_t));
    
    yyset_extra (yy_user_defined, *ptr_yy_globals);
    
    return yy_init_globals ( *ptr_yy_globals );
}

static int yy_init_globals (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yyg->yy_buffer_stack = 0;
    yyg->yy_buffer_stack_top = 0;
    yyg->yy_buffer_stack_max = 0;
    yyg->yy_c_buf_p = (char *) 0;
    yyg->yy_init = 0;
    yyg->yy_start = 0;

    yyg->yy_start_stack_ptr = 0;
    yyg->yy_start_stack_depth = 0;
    yyg->yy_start_stack =  NULL;

#ifdef YY_STDINIT
    yyin = stdin;
    yyout = stdout;
#else
    yyin = (FILE *) 0;
    yyout = (FILE *) 0;
#endif

    return 0;
}

int yylex_destroy  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    while(YY_CURRENT_BUFFER){
        yy_delete_buffer(YY_CURRENT_BUFFER ,yyscanner );
        YY_CURRENT_BUFFER_LVALUE = NULL;
        yypop_buffer_state(yyscanner);
    }

    yyfree(yyg->yy_buffer_stack ,yyscanner);
    yyg->yy_buffer_stack = NULL;

        yyfree(yyg->yy_start_stack ,yyscanner );
        yyg->yy_start_stack = NULL;

    yy_init_globals( yyscanner);

    yyfree ( yyscanner , yyscanner );
    yyscanner = NULL;
    return 0;
}


#ifndef yytext_ptr
static void yy_flex_strncpy (char* s1, yyconst char * s2, int n , yyscan_t yyscanner)
{
    register int i;
    for ( i = 0; i < n; ++i )
        s1[i] = s2[i];
}
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen (yyconst char * s , yyscan_t yyscanner)
{
    register int n;
    for ( n = 0; s[n]; ++n )
        ;

    return n;
}
#endif

void *yyalloc (yy_size_t  size , yyscan_t yyscanner)
{
    return (void *) malloc( size );
}

void *yyrealloc  (void * ptr, yy_size_t  size , yyscan_t yyscanner)
{
    return (void *) realloc( (char *) ptr, size );
}

void yyfree (void * ptr , yyscan_t yyscanner)
{
    free( (char *) ptr );
}

#define YYTABLES_NAME "yytables"

#line 217 "lexer.l"



idx sQLangParserDriver::yyInput(char *lexerBuf, size_t size)
{
    if (_bufcur >= _bufend)
        return YY_NULL;
    if (size > (size_t)(_bufend - _bufcur))
        size = (size_t)(_bufend - _bufcur);
    memcpy(lexerBuf, _bufcur, size);
    _bufcur += size;
    return size;
}

void sQLangParserDriver::yyPushState(int new_state)
{
    yy_push_state(new_state, static_cast<sQLangParserDriverPriv*>(_ppriv)->scanner);
}

void sQLangParserDriver::yyPopState()
{
    yy_pop_state(static_cast<sQLangParserDriverPriv*>(_ppriv)->scanner);
}

