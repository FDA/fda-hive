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
#line 1 "qlang-flex.cpp"

#line 3 "qlang-flex.cpp"

#define  YY_INT_ALIGNED short int


#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 6
#define YY_FLEX_SUBMINOR_VERSION 4
#if YY_FLEX_SUBMINOR_VERSION > 0
#define FLEX_BETA
#endif

#ifdef yyget_lval
#define yyget_lval_ALREADY_DEFINED
#else
#define yyget_lval yyget_lval
#endif

#ifdef yyset_lval
#define yyset_lval_ALREADY_DEFINED
#else
#define yyset_lval yyset_lval
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

#ifndef SIZE_MAX
#define SIZE_MAX               (~(size_t)0)
#endif

#endif 
#endif 

#define yyconst const

#if defined(__GNUC__) && __GNUC__ >= 3
#define yynoreturn __attribute__((__noreturn__))
#else
#define yynoreturn
#endif

#define YY_NULL 0

#define YY_SC_TO_UI(c) ((YY_CHAR) (c))

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
#define YY_NEW_FILE yyrestart( yyin , yyscanner )
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

#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif

#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2
    
    #define YY_LESS_LINENO(n)
    #define YY_LINENO_REWIND_TO(ptr)
    
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

#ifndef YY_STRUCT_YY_BUFFER_STATE
#define YY_STRUCT_YY_BUFFER_STATE
struct yy_buffer_state
    {
    FILE *yy_input_file;

    char *yy_ch_buf;
    char *yy_buf_pos;

    int yy_buf_size;

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

void yyrestart ( FILE *input_file , yyscan_t yyscanner );
void yy_switch_to_buffer ( YY_BUFFER_STATE new_buffer , yyscan_t yyscanner );
YY_BUFFER_STATE yy_create_buffer ( FILE *file, int size , yyscan_t yyscanner );
void yy_delete_buffer ( YY_BUFFER_STATE b , yyscan_t yyscanner );
void yy_flush_buffer ( YY_BUFFER_STATE b , yyscan_t yyscanner );
void yypush_buffer_state ( YY_BUFFER_STATE new_buffer , yyscan_t yyscanner );
void yypop_buffer_state ( yyscan_t yyscanner );

static void yyensure_buffer_stack ( yyscan_t yyscanner );
static void yy_load_buffer_state ( yyscan_t yyscanner );
static void yy_init_buffer ( YY_BUFFER_STATE b, FILE *file , yyscan_t yyscanner );
#define YY_FLUSH_BUFFER yy_flush_buffer( YY_CURRENT_BUFFER , yyscanner)

YY_BUFFER_STATE yy_scan_buffer ( char *base, yy_size_t size , yyscan_t yyscanner );
YY_BUFFER_STATE yy_scan_string ( const char *yy_str , yyscan_t yyscanner );
YY_BUFFER_STATE yy_scan_bytes ( const char *bytes, int len , yyscan_t yyscanner );

void *yyalloc ( yy_size_t , yyscan_t yyscanner );
void *yyrealloc ( void *, yy_size_t , yyscan_t yyscanner );
void yyfree ( void * , yyscan_t yyscanner );

#define yy_new_buffer yy_create_buffer
#define yy_set_interactive(is_interactive) \
    { \
    if ( ! YY_CURRENT_BUFFER ){ \
        yyensure_buffer_stack (yyscanner); \
        YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer( yyin, YY_BUF_SIZE , yyscanner); \
    } \
    YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
    }
#define yy_set_bol(at_bol) \
    { \
    if ( ! YY_CURRENT_BUFFER ){\
        yyensure_buffer_stack (yyscanner); \
        YY_CURRENT_BUFFER_LVALUE =    \
            yy_create_buffer( yyin, YY_BUF_SIZE , yyscanner); \
    } \
    YY_CURRENT_BUFFER_LVALUE->yy_at_bol = at_bol; \
    }
#define YY_AT_BOL() (YY_CURRENT_BUFFER_LVALUE->yy_at_bol)


#define yywrap(yyscanner) (1)
#define YY_SKIP_YYWRAP
typedef flex_uint8_t YY_CHAR;

typedef int yy_state_type;

#define yytext_ptr yytext_r

static yy_state_type yy_get_previous_state ( yyscan_t yyscanner );
static yy_state_type yy_try_NUL_trans ( yy_state_type current_state  , yyscan_t yyscanner);
static int yy_get_next_buffer ( yyscan_t yyscanner );
static void yynoreturn yy_fatal_error ( const char* msg , yyscan_t yyscanner );

#define YY_DO_BEFORE_ACTION \
    yyg->yytext_ptr = yy_bp; \
    yyleng = (int) (yy_cp - yy_bp); \
    yyg->yy_hold_char = *yy_cp; \
    *yy_cp = '\0'; \
    yyg->yy_c_buf_p = yy_cp;
#define YY_NUM_RULES 81
#define YY_END_OF_BUFFER 82
struct yy_trans_info
    {
    flex_int32_t yy_verify;
    flex_int32_t yy_nxt;
    };
static const flex_int16_t yy_accept[211] =
    {   0,
        7,    7,    0,    0,    0,    0,    9,    9,    7,    7,
       82,   80,    7,    7,   29,   80,   80,   41,   80,   80,
       68,   69,   37,   32,   75,   35,   74,   80,   14,   76,
       77,   25,   42,   26,   79,   67,   67,   67,   70,   71,
       67,   67,   67,   67,   67,   67,   67,   67,   67,   67,
       67,   67,   67,   67,   67,   72,   78,   73,    6,    6,
        6,    5,    3,    3,    9,    9,   80,   39,    7,   19,
       24,    0,   15,    0,   10,   11,    0,   40,   27,    0,
       16,    0,   36,   30,   31,   33,   34,    0,    2,    1,
        0,   13,   14,    0,   21,   18,   23,   22,   67,   67,

       67,   52,   67,   67,   67,   67,   67,   67,   67,   67,
       67,   44,   67,   67,   67,   67,   67,   67,   67,   67,
       67,   67,   28,    4,    3,    9,    0,    8,   38,   11,
        0,    0,   12,    0,   16,    0,    0,   17,    0,   13,
        0,    0,   13,   20,   13,   67,   67,   67,   67,   67,
       67,   46,   67,   43,   54,   67,   59,   67,   67,   67,
       67,   67,   67,   67,   17,    0,   13,   53,   67,   67,
       62,   45,   67,   67,   67,   66,   67,   57,   67,   67,
       63,   64,   55,   67,   48,   67,   67,   65,   67,   67,
       67,   67,   67,   47,   67,   67,   67,   67,   67,   51,

       58,   67,   67,   67,   56,   60,   49,   61,   50,    0
    } ;

static const YY_CHAR yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    1,    4,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    2,    5,    6,    1,    7,    8,    9,   10,   11,
       12,   13,   14,   15,   16,   17,   18,   19,   19,   19,
       19,   19,   19,   19,   19,   19,   19,   20,   21,   22,
       23,   24,   25,    1,   26,   27,   27,   27,   28,   29,
       27,   27,   30,   27,   27,   27,   27,   31,   27,   27,
       27,   27,   27,   27,   27,   27,   27,   27,   27,   27,
       32,   33,   34,    1,   27,    1,   35,   36,   37,   38,

       39,   40,   41,   42,   43,   44,   45,   46,   47,   48,
       49,   27,   27,   50,   51,   52,   53,   27,   54,   27,
       27,   27,   55,   56,   57,   58,    1,    1,    1,    1,
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

static const YY_CHAR yy_meta[59] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    1,
        1,    1,    1,    1,    1,    2,    2,    2,    2,    2,
        2,    1,    1,    1,    2,    2,    2,    2,    2,    2,
        2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
        2,    2,    2,    2,    3,    1,    1,    1
    } ;

static const flex_int16_t yy_base[224] =
    {   0,
        0,    0,   56,   57,   58,   60,   64,   65,  277,  276,
      293,  296,   71,   74,   42,   60,   60,  269,  282,   70,
      296,  296,  267,   67,  296,   66,  296,   73,   66,  296,
      296,  266,   61,  265,  296,    0,  256,  260,  296,  296,
      234,   38,  235,  248,  236,   60,  246,   56,   57,  244,
      240,  226,   58,  234,  234,  296,  219,  296,  296,  296,
      256,  296,   94,  108,  266,  265,  260,  103,  120,  296,
      296,   96,  296,  267,  250,    0,   74,  296,  296,  104,
      296,  115,  296,  296,  296,  296,  296,   99,  296,  296,
      265,  108,  111,  119,  243,  296,  296,  296,    0,  237,

      234,    0,  215,  224,  214,  209,  209,  213,  208,  209,
      205,    0,   94,  207,  208,  209,  105,  202,  204,  197,
      201,  205,  296,  296,  138,  240,  235,  296,  296,    0,
       87,  242,  296,  133,  135,  146,  134,  201,  240,  132,
      139,  223,  222,  296,    0,  194,  204,  186,  198,  197,
      184,    0,  197,    0,  187,  186,  185,  184,  176,  185,
      188,  187,  173,  178,  296,  204,  203,    0,  176,  177,
      167,    0,  179,  165,  173,    0,  172,    0,  164,  165,
        0,    0,    0,  173,    0,  130,  134,    0,  133,  124,
      123,  125,  131,    0,  117,  122,  116,  112,  111,    0,

        0,  123,  122,  111,    0,    0,    0,    0,    0,  296,
      179,  182,  185,  188,  190,  193,  196,  152,  199,   97,
      202,  205,  208
    } ;

static const flex_int16_t yy_def[224] =
    {   0,
      210,    1,  211,  211,  212,  212,  213,  213,    1,    1,
      210,  210,  210,  210,  210,  214,  215,  210,  210,  216,
      210,  210,  210,  210,  210,  210,  210,  217,  210,  210,
      210,  210,  210,  210,  210,  218,  218,  218,  210,  210,
      218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
      218,  218,  218,  218,  218,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  219,  219,  219,  210,  210,  210,
      210,  214,  210,  214,  210,  220,  221,  210,  210,  216,
      210,  222,  210,  210,  210,  210,  210,  223,  210,  210,
      217,  210,  210,  210,  210,  210,  210,  210,  218,  218,

      218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
      218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
      218,  218,  210,  210,  210,  219,  219,  210,  210,  220,
      221,  221,  210,  216,  216,  222,  223,  210,  223,  210,
      210,  210,  210,  210,  218,  218,  218,  218,  218,  218,
      218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
      218,  218,  218,  218,  210,  210,  210,  218,  218,  218,
      218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
      218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
      218,  218,  218,  218,  218,  218,  218,  218,  218,  218,

      218,  218,  218,  218,  218,  218,  218,  218,  218,    0,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210
    } ;

static const flex_int16_t yy_nxt[355] =
    {   0,
       12,   13,   14,   13,   15,   16,   17,   18,   19,   20,
       21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
       31,   32,   33,   34,   35,   36,   36,   36,   36,   37,
       38,   39,   12,   40,   41,   42,   43,   44,   45,   46,
       36,   47,   48,   36,   36,   36,   36,   49,   50,   51,
       52,   53,   54,   55,   56,   57,   58,   12,   60,   60,
       63,   64,   63,   64,   70,   73,   66,   66,   61,   61,
       67,   67,   69,   69,   69,   69,   69,   69,   75,   81,
       84,   86,   92,   96,   93,   89,  103,  104,   87,   85,
       90,  114,   74,   94,  108,  112,  125,  125,  130,   71,

      119,   73,   82,  113,   94,   91,  132,  120,  109,  115,
      125,  125,  110,   81,   77,   89,  138,   80,   97,  132,
       90,   69,   69,   69,  135,  129,  140,   92,   74,   93,
      133,  139,  142,  145,  142,  141,   82,  143,   94,  158,
      125,  125,   81,  133,   81,  155,  141,  136,   80,   94,
      140,  138,  166,   99,  166,  135,  159,  167,  209,  141,
      208,  207,  206,  205,  204,   82,  139,   82,  203,  202,
      141,  201,  200,  199,  198,  197,  196,  195,  136,   59,
       59,   59,   62,   62,   62,   65,   65,   65,   72,   72,
       72,   76,   76,   80,   80,   80,   88,   88,   88,  126,

      126,  126,  131,  131,  131,  134,  134,  134,  137,  137,
      137,  194,  193,  192,  191,  190,  189,  188,  187,  186,
      185,  167,  167,  184,  183,  182,  181,  180,  179,  178,
      177,  176,  175,  174,  173,  172,  171,  170,  169,  168,
      143,  143,  210,  165,  210,  210,  127,  164,  163,  162,
      161,  160,  157,  156,  145,  154,  153,  152,  151,  150,
      149,  148,  147,  146,  145,  145,  144,  210,   75,  210,
      128,  127,  127,  124,  123,  122,  121,  118,  117,  116,
      111,  107,  106,  105,  102,  101,  100,   98,   95,   83,
       79,   78,  210,   68,   68,   11,  210,  210,  210,  210,

      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210
    } ;

static const flex_int16_t yy_chk[355] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    3,    4,
        5,    5,    6,    6,   15,   16,    7,    8,    3,    4,
        7,    8,   13,   13,   13,   14,   14,   14,   17,   20,
       24,   26,   29,   33,   29,   28,   42,   42,   26,   24,
       28,   49,   16,   29,   46,   48,   63,   63,  220,   15,

       53,   72,   20,   48,   29,   28,   77,   53,   46,   49,
       64,   64,   46,   80,   17,   68,   88,   82,   33,  131,
       68,   69,   69,   69,   82,   68,   92,   93,   72,   93,
       77,   88,   94,  113,   94,   92,   80,   94,   93,  117,
      125,  125,  134,  131,  135,  113,   92,   82,  136,   93,
      140,  137,  141,  218,  141,  136,  117,  141,  204,  140,
      203,  202,  199,  198,  197,  134,  137,  135,  196,  195,
      140,  193,  192,  191,  190,  189,  187,  186,  136,  211,
      211,  211,  212,  212,  212,  213,  213,  213,  214,  214,
      214,  215,  215,  216,  216,  216,  217,  217,  217,  219,

      219,  219,  221,  221,  221,  222,  222,  222,  223,  223,
      223,  184,  180,  179,  177,  175,  174,  173,  171,  170,
      169,  167,  166,  164,  163,  162,  161,  160,  159,  158,
      157,  156,  155,  153,  151,  150,  149,  148,  147,  146,
      143,  142,  139,  138,  132,  127,  126,  122,  121,  120,
      119,  118,  116,  115,  114,  111,  110,  109,  108,  107,
      106,  105,  104,  103,  101,  100,   95,   91,   75,   74,
       67,   66,   65,   61,   57,   55,   54,   52,   51,   50,
       47,   45,   44,   43,   41,   38,   37,   34,   32,   23,
       19,   18,   11,   10,    9,  210,  210,  210,  210,  210,

      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210,  210,  210,  210,  210,  210,  210,
      210,  210,  210,  210
    } ;

#define REJECT reject_used_but_not_detected
#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0
#define YY_RESTORE_YY_MORE_OFFSET
#line 1 "qlang-flex.l"
#line 16 "qlang-flex.l"
#include <slib/std/string.hpp>
#include <qlang/parser.hpp>
#include "lexerExtras.hpp"
#include "parserPrivate.hpp"

#define yyterminate() return token::END
#line 604 "qlang-flex.cpp"
#define YY_NO_UNISTD_H 1

#line 48 "qlang-flex.l"
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
#line 626 "qlang-flex.cpp"
#line 627 "qlang-flex.cpp"

#define INITIAL 0
#define COMMENT_C 1
#define COMMENT_CPP 2
#define TEMPLATE 3
#define NO_REGEX 4

#ifndef YY_NO_UNISTD_H
#include <unistd.h>
#endif

#define YY_EXTRA_TYPE slib::qlang::Parser *

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

static int yy_init_globals ( yyscan_t yyscanner );

    #    define yylval yyg->yylval_r
    
int yylex_init (yyscan_t* scanner);

int yylex_init_extra ( YY_EXTRA_TYPE user_defined, yyscan_t* scanner);


int yylex_destroy ( yyscan_t yyscanner );

int yyget_debug ( yyscan_t yyscanner );

void yyset_debug ( int debug_flag , yyscan_t yyscanner );

YY_EXTRA_TYPE yyget_extra ( yyscan_t yyscanner );

void yyset_extra ( YY_EXTRA_TYPE user_defined , yyscan_t yyscanner );

FILE *yyget_in ( yyscan_t yyscanner );

void yyset_in  ( FILE * _in_str , yyscan_t yyscanner );

FILE *yyget_out ( yyscan_t yyscanner );

void yyset_out  ( FILE * _out_str , yyscan_t yyscanner );

            int yyget_leng ( yyscan_t yyscanner );

char *yyget_text ( yyscan_t yyscanner );

int yyget_lineno ( yyscan_t yyscanner );

void yyset_lineno ( int _line_number , yyscan_t yyscanner );

int yyget_column  ( yyscan_t yyscanner );

void yyset_column ( int _column_no , yyscan_t yyscanner );

YYSTYPE * yyget_lval ( yyscan_t yyscanner );

void yyset_lval ( YYSTYPE * yylval_param , yyscan_t yyscanner );


#ifndef YY_SKIP_YYWRAP
#ifdef __cplusplus
extern "C" int yywrap ( yyscan_t yyscanner );
#else
extern int yywrap ( yyscan_t yyscanner );
#endif
#endif

#ifndef YY_NO_UNPUT
    
    static void yyunput ( int c, char *buf_ptr  , yyscan_t yyscanner);
    
#endif

#ifndef yytext_ptr
static void yy_flex_strncpy ( char *, const char *, int , yyscan_t yyscanner);
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen ( const char * , yyscan_t yyscanner);
#endif

#ifndef YY_NO_INPUT
#ifdef __cplusplus
static int yyinput ( yyscan_t yyscanner );
#else
static int input ( yyscan_t yyscanner );
#endif

#endif

    static void yy_push_state ( int _new_state , yyscan_t yyscanner);
    
    static void yy_pop_state ( yyscan_t yyscanner );
    
    static int yy_top_state ( yyscan_t yyscanner );
    
#ifndef YY_READ_BUF_SIZE
#ifdef __ia64__
#define YY_READ_BUF_SIZE 16384
#else
#define YY_READ_BUF_SIZE 8192
#endif #endif

#ifndef ECHO
#define ECHO do { if (fwrite( yytext, (size_t) yyleng, 1, yyout )) {} } while (0)
#endif

#ifndef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
    if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \
        { \
        int c = '*'; \
        int n; \
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
        while ( (result = (int) fread(buf, 1, (yy_size_t) max_size, yyin)) == 0 && ferror(yyin)) \
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
               (YYSTYPE * yylval_param , yyscan_t yyscanner);

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
    yy_state_type yy_current_state;
    char *yy_cp, *yy_bp;
    int yy_act;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

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
                yy_create_buffer( yyin, YY_BUF_SIZE , yyscanner);
        }

        yy_load_buffer_state( yyscanner );
        }

    {
#line 68 "qlang-flex.l"



#line 72 "qlang-flex.l"
yylloc->step();


#line 917 "qlang-flex.cpp"

    while (1 )
        {
        yy_cp = yyg->yy_c_buf_p;

        *yy_cp = yyg->yy_hold_char;

        yy_bp = yy_cp;

        yy_current_state = yyg->yy_start;
yy_match:
        do
            {
            YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)] ;
            if ( yy_accept[yy_current_state] )
                {
                yyg->yy_last_accepting_state = yy_current_state;
                yyg->yy_last_accepting_cpos = yy_cp;
                }
            while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
                {
                yy_current_state = (int) yy_def[yy_current_state];
                if ( yy_current_state >= 211 )
                    yy_c = yy_meta[yy_c];
                }
            yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
            ++yy_cp;
            }
        while ( yy_current_state != 210 );
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
#line 75 "qlang-flex.l"
{ yy_push_state(COMMENT_CPP, yyscanner); }
    YY_BREAK
case 2:
YY_RULE_SETUP
#line 76 "qlang-flex.l"
{ yy_push_state(COMMENT_C, yyscanner); }
    YY_BREAK
case YY_STATE_EOF(COMMENT_CPP):
#line 78 "qlang-flex.l"
{ yy_pop_state(yyscanner); }
    YY_BREAK
case 3:
YY_RULE_SETUP
#line 79 "qlang-flex.l"
{ yy_pop_state(yyscanner); }
    YY_BREAK
case YY_STATE_EOF(COMMENT_C):
#line 80 "qlang-flex.l"
{
        parser_driver.setError("unterminated comment", yylloc->begin.line, yylloc->begin.column);
        yyterminate();
    }
    YY_BREAK
case 4:
YY_RULE_SETUP
#line 84 "qlang-flex.l"
{ yy_pop_state(yyscanner); }
    YY_BREAK
case 5:
YY_RULE_SETUP
#line 86 "qlang-flex.l"
{ yylloc->step(); }
    YY_BREAK
case 6:
YY_RULE_SETUP
#line 87 "qlang-flex.l"
{ yylloc->step(); }
    YY_BREAK
case 7:
YY_RULE_SETUP
#line 88 "qlang-flex.l"
{ yylloc->step(); }
    YY_BREAK
case 8:
YY_RULE_SETUP
#line 90 "qlang-flex.l"
{
        yy_push_state(INITIAL, yyscanner);
        return token::TMPL_CODE_START;
    }
    YY_BREAK
case 9:
YY_RULE_SETUP
#line 94 "qlang-flex.l"
{
        yylval_param->strVal = new slib::sStr("%s", yytext);
        return token::TMPL_STRING;
    }
    YY_BREAK
case YY_STATE_EOF(TEMPLATE):
#line 98 "qlang-flex.l"
{ yy_pop_state(yyscanner); SQLANG_YY_NO_REGEX; }
    YY_BREAK
case 10:
YY_RULE_SETUP
#line 100 "qlang-flex.l"
{
        SQLANG_YY_NO_REGEX;
        yylval_param->intVal = atoidx(yytext + 1);
        return token::DOLLAR_NUM;
    }
    YY_BREAK
case 11:
YY_RULE_SETUP
#line 105 "qlang-flex.l"
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
#line 112 "qlang-flex.l"
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
#line 121 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; yylval->realVal = atof(yytext); return token::REAL_LITERAL; }
    YY_BREAK
case 14:
YY_RULE_SETUP
#line 122 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; yylval_param->intVal = atoidx(yytext); return token::INT_LITERAL; }
    YY_BREAK
case 15:
YY_RULE_SETUP
#line 123 "qlang-flex.l"
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
#line 131 "qlang-flex.l"
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
#line 139 "qlang-flex.l"
{
        SQLANG_YY_NO_REGEX;
        yytext++;
        char * last_slash = strrchr(yytext, '/');
        *last_slash = 0;
        yylval_param->strVal = new slib::sStr("%s", yytext);
        yylval_param->strVal->add0();
        yylval_param->strVal->printf("%s", last_slash + 1);
        yylval_param->strVal->add0(2);
        return token::REGEX_LITERAL;
    }
    YY_BREAK
case 18:
YY_RULE_SETUP
#line 151 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::EQ; }
    YY_BREAK
case 19:
YY_RULE_SETUP
#line 152 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::NE; }
    YY_BREAK
case 20:
YY_RULE_SETUP
#line 153 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::CMP; }
    YY_BREAK
case 21:
YY_RULE_SETUP
#line 154 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::LE; }
    YY_BREAK
case 22:
YY_RULE_SETUP
#line 155 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::GE; }
    YY_BREAK
case 23:
YY_RULE_SETUP
#line 156 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::MATCH; }
    YY_BREAK
case 24:
YY_RULE_SETUP
#line 157 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::NMATCH; }
    YY_BREAK
case 25:
YY_RULE_SETUP
#line 158 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '<'; }
    YY_BREAK
case 26:
YY_RULE_SETUP
#line 159 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '>'; }
    YY_BREAK
case 27:
YY_RULE_SETUP
#line 160 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::AND; }
    YY_BREAK
case 28:
YY_RULE_SETUP
#line 161 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::OR; }
    YY_BREAK
case 29:
YY_RULE_SETUP
#line 162 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '!'; }
    YY_BREAK
case 30:
YY_RULE_SETUP
#line 163 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::INCREMENT; }
    YY_BREAK
case 31:
YY_RULE_SETUP
#line 164 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::PLUS_INPLACE; }
    YY_BREAK
case 32:
YY_RULE_SETUP
#line 165 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '+'; }
    YY_BREAK
case 33:
YY_RULE_SETUP
#line 166 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::DECREMENT; }
    YY_BREAK
case 34:
YY_RULE_SETUP
#line 167 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::MINUS_INPLACE; }
    YY_BREAK
case 35:
YY_RULE_SETUP
#line 168 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '-'; }
    YY_BREAK
case 36:
YY_RULE_SETUP
#line 169 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::MULTIPLY_INPLACE; }
    YY_BREAK
case 37:
YY_RULE_SETUP
#line 170 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '*'; }
    YY_BREAK
case 38:
YY_RULE_SETUP
#line 171 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::DIVIDE_INPLACE; }
    YY_BREAK
case 39:
YY_RULE_SETUP
#line 172 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '/'; }
    YY_BREAK
case 40:
YY_RULE_SETUP
#line 173 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::REMAINDER_INPLACE; }
    YY_BREAK
case 41:
YY_RULE_SETUP
#line 174 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '%'; }
    YY_BREAK
case 42:
YY_RULE_SETUP
#line 175 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '='; }
    YY_BREAK
case 43:
YY_RULE_SETUP
#line 177 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::HAS; }
    YY_BREAK
case 44:
YY_RULE_SETUP
#line 178 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::IF; }
    YY_BREAK
case 45:
YY_RULE_SETUP
#line 179 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::ELSE; }
    YY_BREAK
case 46:
YY_RULE_SETUP
#line 180 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::FOR; }
    YY_BREAK
case 47:
YY_RULE_SETUP
#line 181 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::WHILE; }
    YY_BREAK
case 48:
YY_RULE_SETUP
#line 182 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::BREAK; }
    YY_BREAK
case 49:
YY_RULE_SETUP
#line 183 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::CONTINUE; }
    YY_BREAK
case 50:
YY_RULE_SETUP
#line 184 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::FUNCTION; }
    YY_BREAK
case 51:
YY_RULE_SETUP
#line 185 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::RETURN; }
    YY_BREAK
case 52:
YY_RULE_SETUP
#line 186 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::AS; }
    YY_BREAK
case 53:
YY_RULE_SETUP
#line 187 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::BOOL; }
    YY_BREAK
case 54:
YY_RULE_SETUP
#line 188 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::INT; }
    YY_BREAK
case 55:
YY_RULE_SETUP
#line 189 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::UINT; }
    YY_BREAK
case 56:
YY_RULE_SETUP
#line 190 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::INTLIST; }
    YY_BREAK
case 57:
YY_RULE_SETUP
#line 191 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::REAL; }
    YY_BREAK
case 58:
YY_RULE_SETUP
#line 192 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::STRING; }
    YY_BREAK
case 59:
YY_RULE_SETUP
#line 193 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::OBJ; }
    YY_BREAK
case 60:
YY_RULE_SETUP
#line 194 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::OBJLIST; }
    YY_BREAK
case 61:
YY_RULE_SETUP
#line 195 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::DATETIME; }
    YY_BREAK
case 62:
YY_RULE_SETUP
#line 196 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::DATE; }
    YY_BREAK
case 63:
YY_RULE_SETUP
#line 197 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return token::TIME; }
    YY_BREAK
case 64:
YY_RULE_SETUP
#line 199 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; yylval_param->intVal = 1; return token::BOOL_LITERAL; }
    YY_BREAK
case 65:
YY_RULE_SETUP
#line 200 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; yylval_param->intVal = 0; return token::BOOL_LITERAL; }
    YY_BREAK
case 66:
YY_RULE_SETUP
#line 201 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; return token::NULL_LITERAL; }
    YY_BREAK
case 67:
YY_RULE_SETUP
#line 203 "qlang-flex.l"
{
        SQLANG_YY_NO_REGEX;
        yylval_param->strVal = new slib::sStr("%s", yytext);
        return token::NAME;
    }
    YY_BREAK
case 68:
YY_RULE_SETUP
#line 209 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '('; }
    YY_BREAK
case 69:
YY_RULE_SETUP
#line 210 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; return ')'; }
    YY_BREAK
case 70:
YY_RULE_SETUP
#line 211 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '['; }
    YY_BREAK
case 71:
YY_RULE_SETUP
#line 212 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; return ']'; }
    YY_BREAK
case 72:
YY_RULE_SETUP
#line 213 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '{'; }
    YY_BREAK
case 73:
YY_RULE_SETUP
#line 214 "qlang-flex.l"
{ SQLANG_YY_NO_REGEX; return '}'; }
    YY_BREAK
case 74:
YY_RULE_SETUP
#line 215 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '.'; }
    YY_BREAK
case 75:
YY_RULE_SETUP
#line 216 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return ','; }
    YY_BREAK
case 76:
YY_RULE_SETUP
#line 217 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return ':'; }
    YY_BREAK
case 77:
YY_RULE_SETUP
#line 218 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return ';'; }
    YY_BREAK
case 78:
YY_RULE_SETUP
#line 219 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '|'; }
    YY_BREAK
case 79:
YY_RULE_SETUP
#line 220 "qlang-flex.l"
{ SQLANG_YY_OK_REGEX; return '?'; }
    YY_BREAK
case 80:
YY_RULE_SETUP
#line 222 "qlang-flex.l"
{
        parser_driver.setError("invalid character", yylloc->begin.line, yylloc->begin.column);
        yyterminate();
    }
    YY_BREAK
case 81:
YY_RULE_SETUP
#line 227 "qlang-flex.l"
YY_FATAL_ERROR( "flex scanner jammed" );
    YY_BREAK
#line 1452 "qlang-flex.cpp"
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

                if ( yywrap( yyscanner ) )
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
}

static int yy_get_next_buffer (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    char *dest = YY_CURRENT_BUFFER_LVALUE->yy_ch_buf;
    char *source = yyg->yytext_ptr;
    int number_to_move, i;
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


    number_to_move = (int) (yyg->yy_c_buf_p - yyg->yytext_ptr - 1);

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

            YY_BUFFER_STATE b = YY_CURRENT_BUFFER_LVALUE;

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
                    yyrealloc( (void *) b->yy_ch_buf,
                             (yy_size_t) (b->yy_buf_size + 2) , yyscanner );
                }
            else
                b->yy_ch_buf = NULL;

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
            yyg->yy_n_chars, num_to_read );

        YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
        }

    if ( yyg->yy_n_chars == 0 )
        {
        if ( number_to_move == YY_MORE_ADJ )
            {
            ret_val = EOB_ACT_END_OF_FILE;
            yyrestart( yyin  , yyscanner);
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

    if ((yyg->yy_n_chars + number_to_move) > YY_CURRENT_BUFFER_LVALUE->yy_buf_size) {
        int new_size = yyg->yy_n_chars + number_to_move + (yyg->yy_n_chars >> 1);
        YY_CURRENT_BUFFER_LVALUE->yy_ch_buf = (char *) yyrealloc(
            (void *) YY_CURRENT_BUFFER_LVALUE->yy_ch_buf, (yy_size_t) new_size , yyscanner );
        if ( ! YY_CURRENT_BUFFER_LVALUE->yy_ch_buf )
            YY_FATAL_ERROR( "out of dynamic memory in yy_get_next_buffer()" );
        YY_CURRENT_BUFFER_LVALUE->yy_buf_size = (int) (new_size - 2);
    }

    yyg->yy_n_chars += number_to_move;
    YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] = YY_END_OF_BUFFER_CHAR;
    YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] = YY_END_OF_BUFFER_CHAR;

    yyg->yytext_ptr = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[0];

    return ret_val;
}


    static yy_state_type yy_get_previous_state (yyscan_t yyscanner)
{
    yy_state_type yy_current_state;
    char *yy_cp;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yy_current_state = yyg->yy_start;

    for ( yy_cp = yyg->yytext_ptr + YY_MORE_ADJ; yy_cp < yyg->yy_c_buf_p; ++yy_cp )
        {
        YY_CHAR yy_c = (*yy_cp ? yy_ec[YY_SC_TO_UI(*yy_cp)] : 1);
        if ( yy_accept[yy_current_state] )
            {
            yyg->yy_last_accepting_state = yy_current_state;
            yyg->yy_last_accepting_cpos = yy_cp;
            }
        while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
            {
            yy_current_state = (int) yy_def[yy_current_state];
            if ( yy_current_state >= 211 )
                yy_c = yy_meta[yy_c];
            }
        yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
        }

    return yy_current_state;
}

    static yy_state_type yy_try_NUL_trans  (yy_state_type yy_current_state , yyscan_t yyscanner)
{
    int yy_is_jam;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    char *yy_cp = yyg->yy_c_buf_p;

    YY_CHAR yy_c = 1;
    if ( yy_accept[yy_current_state] )
        {
        yyg->yy_last_accepting_state = yy_current_state;
        yyg->yy_last_accepting_cpos = yy_cp;
        }
    while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
        {
        yy_current_state = (int) yy_def[yy_current_state];
        if ( yy_current_state >= 211 )
            yy_c = yy_meta[yy_c];
        }
    yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
    yy_is_jam = (yy_current_state == 210);

    (void)yyg;
    return yy_is_jam ? 0 : yy_current_state;
}

#ifndef YY_NO_UNPUT

    static void yyunput (int c, char * yy_bp , yyscan_t yyscanner)
{
    char *yy_cp;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yy_cp = yyg->yy_c_buf_p;

    *yy_cp = yyg->yy_hold_char;

    if ( yy_cp < YY_CURRENT_BUFFER_LVALUE->yy_ch_buf + 2 )
        {
        int number_to_move = yyg->yy_n_chars + 2;
        char *dest = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[
                    YY_CURRENT_BUFFER_LVALUE->yy_buf_size + 2];
        char *source =
                &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move];

        while ( source > YY_CURRENT_BUFFER_LVALUE->yy_ch_buf )
            *--dest = *--source;

        yy_cp += (int) (dest - source);
        yy_bp += (int) (dest - source);
        YY_CURRENT_BUFFER_LVALUE->yy_n_chars =
            yyg->yy_n_chars = (int) YY_CURRENT_BUFFER_LVALUE->yy_buf_size;

        if ( yy_cp < YY_CURRENT_BUFFER_LVALUE->yy_ch_buf + 2 )
            YY_FATAL_ERROR( "flex scanner push-back overflow" );
        }

    *--yy_cp = (char) c;

    yyg->yytext_ptr = yy_bp;
    yyg->yy_hold_char = *yy_cp;
    yyg->yy_c_buf_p = yy_cp;
}

#endif

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
            int offset = (int) (yyg->yy_c_buf_p - yyg->yytext_ptr);
            ++yyg->yy_c_buf_p;

            switch ( yy_get_next_buffer( yyscanner ) )
                {
                case EOB_ACT_LAST_MATCH:

                    yyrestart( yyin , yyscanner);


                case EOB_ACT_END_OF_FILE:
                    {
                    if ( yywrap( yyscanner ) )
                        return 0;

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
            yy_create_buffer( yyin, YY_BUF_SIZE , yyscanner);
    }

    yy_init_buffer( YY_CURRENT_BUFFER, input_file , yyscanner);
    yy_load_buffer_state( yyscanner );
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
    yy_load_buffer_state( yyscanner );

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
    
    b = (YY_BUFFER_STATE) yyalloc( sizeof( struct yy_buffer_state ) , yyscanner );
    if ( ! b )
        YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );

    b->yy_buf_size = size;

    b->yy_ch_buf = (char *) yyalloc( (yy_size_t) (b->yy_buf_size + 2) , yyscanner );
    if ( ! b->yy_ch_buf )
        YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );

    b->yy_is_our_buffer = 1;

    yy_init_buffer( b, file , yyscanner);

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
        yyfree( (void *) b->yy_ch_buf , yyscanner );

    yyfree( (void *) b , yyscanner );
}

    static void yy_init_buffer  (YY_BUFFER_STATE  b, FILE * file , yyscan_t yyscanner)

{
    int oerrno = errno;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    yy_flush_buffer( b , yyscanner);

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
        yy_load_buffer_state( yyscanner );
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

    yy_load_buffer_state( yyscanner );
    yyg->yy_did_buffer_switch_on_eof = 1;
}

void yypop_buffer_state (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if (!YY_CURRENT_BUFFER)
        return;

    yy_delete_buffer(YY_CURRENT_BUFFER , yyscanner);
    YY_CURRENT_BUFFER_LVALUE = NULL;
    if (yyg->yy_buffer_stack_top > 0)
        --yyg->yy_buffer_stack_top;

    if (YY_CURRENT_BUFFER) {
        yy_load_buffer_state( yyscanner );
        yyg->yy_did_buffer_switch_on_eof = 1;
    }
}

static void yyensure_buffer_stack (yyscan_t yyscanner)
{
    yy_size_t num_to_alloc;
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

        yy_size_t grow_size = 8;

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
        return NULL;

    b = (YY_BUFFER_STATE) yyalloc( sizeof( struct yy_buffer_state ) , yyscanner );
    if ( ! b )
        YY_FATAL_ERROR( "out of dynamic memory in yy_scan_buffer()" );

    b->yy_buf_size = (int) (size - 2);
    b->yy_buf_pos = b->yy_ch_buf = base;
    b->yy_is_our_buffer = 0;
    b->yy_input_file = NULL;
    b->yy_n_chars = b->yy_buf_size;
    b->yy_is_interactive = 0;
    b->yy_at_bol = 1;
    b->yy_fill_buffer = 0;
    b->yy_buffer_status = YY_BUFFER_NEW;

    yy_switch_to_buffer( b , yyscanner );

    return b;
}

YY_BUFFER_STATE yy_scan_string (const char * yystr , yyscan_t yyscanner)
{
    
    return yy_scan_bytes( yystr, (int) strlen(yystr) , yyscanner);
}

YY_BUFFER_STATE yy_scan_bytes  (const char * yybytes, int  _yybytes_len , yyscan_t yyscanner)
{
    YY_BUFFER_STATE b;
    char *buf;
    yy_size_t n;
    int i;
    
    n = (yy_size_t) (_yybytes_len + 2);
    buf = (char *) yyalloc( n , yyscanner );
    if ( ! buf )
        YY_FATAL_ERROR( "out of dynamic memory in yy_scan_bytes()" );

    for ( i = 0; i < _yybytes_len; ++i )
        buf[i] = yybytes[i];

    buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;

    b = yy_scan_buffer( buf, n , yyscanner);
    if ( ! b )
        YY_FATAL_ERROR( "bad buffer in yy_scan_bytes()" );

    b->yy_is_our_buffer = 1;

    return b;
}

    static void yy_push_state (int  _new_state , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    if ( yyg->yy_start_stack_ptr >= yyg->yy_start_stack_depth )
        {
        yy_size_t new_size;

        yyg->yy_start_stack_depth += YY_START_STACK_INCR;
        new_size = (yy_size_t) yyg->yy_start_stack_depth * sizeof( int );

        if ( ! yyg->yy_start_stack )
            yyg->yy_start_stack = (int *) yyalloc( new_size , yyscanner );

        else
            yyg->yy_start_stack = (int *) yyrealloc(
                    (void *) yyg->yy_start_stack, new_size , yyscanner );

        if ( ! yyg->yy_start_stack )
            YY_FATAL_ERROR( "out of memory expanding start-condition stack" );
        }

    yyg->yy_start_stack[yyg->yy_start_stack_ptr++] = YY_START;

    BEGIN(_new_state);
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

static void yynoreturn yy_fatal_error (const char* msg , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    (void)yyg;
    fprintf( stderr, "%s\n", msg );
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

void yyset_lineno (int  _line_number , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

        if (! YY_CURRENT_BUFFER )
           YY_FATAL_ERROR( "yyset_lineno called with no buffer" );
    
    yylineno = _line_number;
}

void yyset_column (int  _column_no , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

        if (! YY_CURRENT_BUFFER )
           YY_FATAL_ERROR( "yyset_column called with no buffer" );
    
    yycolumn = _column_no;
}

void yyset_in (FILE *  _in_str , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyin = _in_str ;
}

void yyset_out (FILE *  _out_str , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyout = _out_str ;
}

int yyget_debug  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    return yy_flex_debug;
}

void yyset_debug (int  _bdebug , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yy_flex_debug = _bdebug ;
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

int yylex_init_extra( YY_EXTRA_TYPE yy_user_defined, yyscan_t* ptr_yy_globals )
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

    yyg->yy_buffer_stack = NULL;
    yyg->yy_buffer_stack_top = 0;
    yyg->yy_buffer_stack_max = 0;
    yyg->yy_c_buf_p = NULL;
    yyg->yy_init = 0;
    yyg->yy_start = 0;

    yyg->yy_start_stack_ptr = 0;
    yyg->yy_start_stack_depth = 0;
    yyg->yy_start_stack =  NULL;

#ifdef YY_STDINIT
    yyin = stdin;
    yyout = stdout;
#else
    yyin = NULL;
    yyout = NULL;
#endif

    return 0;
}

int yylex_destroy  (yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;

    while(YY_CURRENT_BUFFER){
        yy_delete_buffer( YY_CURRENT_BUFFER , yyscanner );
        YY_CURRENT_BUFFER_LVALUE = NULL;
        yypop_buffer_state(yyscanner);
    }

    yyfree(yyg->yy_buffer_stack , yyscanner);
    yyg->yy_buffer_stack = NULL;

        yyfree( yyg->yy_start_stack , yyscanner );
        yyg->yy_start_stack = NULL;

    yy_init_globals( yyscanner);

    yyfree ( yyscanner , yyscanner );
    yyscanner = NULL;
    return 0;
}


#ifndef yytext_ptr
static void yy_flex_strncpy (char* s1, const char * s2, int n , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    (void)yyg;

    int i;
    for ( i = 0; i < n; ++i )
        s1[i] = s2[i];
}
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen (const char * s , yyscan_t yyscanner)
{
    int n;
    for ( n = 0; s[n]; ++n )
        ;

    return n;
}
#endif

void *yyalloc (yy_size_t  size , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    (void)yyg;
    return malloc(size);
}

void *yyrealloc  (void * ptr, yy_size_t  size , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    (void)yyg;

    return realloc(ptr, size);
}

void yyfree (void * ptr , yyscan_t yyscanner)
{
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    (void)yyg;
    free( (char *) ptr );
}

#define YYTABLES_NAME "yytables"

#line 227 "qlang-flex.l"


idx qlang::Parser::yyInput(char *lexerBuf, size_t size)
{
    if (_bufcur >= _bufend)
        return YY_NULL;
    if (size > (size_t)(_bufend - _bufcur))
        size = (size_t)(_bufend - _bufcur);
    memcpy(lexerBuf, _bufcur, size);
    _bufcur += size;
    return size;
}

void qlang::Parser::yyPushState(int new_state)
{
    yy_push_state(new_state, static_cast<qlang::ParserPriv*>(_ppriv)->scanner);
}

void qlang::Parser::yyPopState()
{
    yy_pop_state(static_cast<qlang::ParserPriv*>(_ppriv)->scanner);
}

void qlang::Parser::yyPopStateUntilTemplate()
{
    yyscan_t scanner = static_cast<qlang::ParserPriv*>(_ppriv)->scanner;
    yy_pop_state(scanner);
    if (yy_top_state(scanner) == TEMPLATE)
        yy_pop_state(scanner);
}

