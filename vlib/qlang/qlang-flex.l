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
/*
* To generate the lexical analyzer, run: "flex qlang-flex.l"
*/

%{
#include <slib/std/string.hpp>
#include <qlang/parser.hpp>
#include "lexerExtras.hpp"
#include "parserPrivate.hpp"

#define yyterminate() return token::END
%}

%option outfile="qlang-flex.cpp" header-file="qlang-flex.hpp"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge
%option extra-type="slib::qlang::Parser *"
%option stack

DIGIT   [0-9]+
ALPHA   [[:alpha:]_]
ALNUM   [[:alnum:]_]
NAME    {ALPHA}{ALNUM}*
WS      [ \t\r\n]*
INT     [[:digit:]]+
EXPONENT [eE][-+]?[[:digit:]]+
REAL    ([[:digit:]]+(\.[[:digit:]]*{EXPONENT}?|{EXPONENT})|inf|INF|nan|NAN)
ANY     (.|\n)

%x COMMENT_C
%x COMMENT_CPP
%x TEMPLATE
%s NO_REGEX

%{
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
%}

%%

%{
yylloc->step();
%}

"//"    { yy_push_state(COMMENT_CPP, yyscanner); }
"/*"    { yy_push_state(COMMENT_C, yyscanner); }

<COMMENT_CPP><<EOF>>    { yy_pop_state(yyscanner); }
<COMMENT_CPP>[\r\n]+    { yy_pop_state(yyscanner); }
<COMMENT_C><<EOF>>      {
        parser_driver.setError("unterminated comment", yylloc->begin.line, yylloc->begin.column);
        yyterminate();
    }
<COMMENT_C>"*/"         { yy_pop_state(yyscanner); }

<COMMENT_CPP>.      { yylloc->step(); }
<COMMENT_C>{ANY}    { yylloc->step(); }
{WS}                { yylloc->step(); }

<TEMPLATE>"$("      {
        yy_push_state(INITIAL, yyscanner);
        return token::TMPL_CODE_START;
    }
<TEMPLATE>([^$]|$[^(])* {
        yylval_param->strVal = new slib::sStr("%s", yytext);
        return token::TMPL_STRING;
    }
<TEMPLATE><<EOF>>   { yy_pop_state(yyscanner); SQLANG_YY_NO_REGEX; }

"$"{INT}            {
        SQLANG_YY_NO_REGEX;
        yylval_param->intVal = atoidx(yytext + 1);
        return token::DOLLAR_NUM;
    }
"$"{NAME}           {
        SQLANG_YY_NO_REGEX;
        yylval_param->strVal = new slib::sStr();
        yytext++;
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::DOLLAR_NAME;
    }
"${"(\\.|[^\\}])*"}" {
        SQLANG_YY_NO_REGEX;
        yylval_param->strVal = new slib::sStr();
        yytext += 2;
        yytext[strlen(yytext)-1] = 0;
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::DOLLAR_NAME;
    }

{REAL}  { SQLANG_YY_NO_REGEX; yylval->realVal = atof(yytext); return token::REAL_LITERAL; }
{INT}   { SQLANG_YY_NO_REGEX; yylval_param->intVal = atoidx(yytext); return token::INT_LITERAL; }
\"(\\.|[^\\"])*\" {
        SQLANG_YY_NO_REGEX;
        yytext++;
        yytext[strlen(yytext)-1] = 0;
        yylval_param->strVal = new slib::sStr();
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::STRING_LITERAL;
    }
'(\\.|[^'])*' {
        SQLANG_YY_NO_REGEX;
        yytext++;
        yytext[strlen(yytext)-1] = 0;
        yylval_param->strVal = new slib::sStr();
        sString::replaceEscapeSequences(yylval_param->strVal, yytext);
        return token::STRING_LITERAL;
    }
<INITIAL>"/"(\\.|[^\\*/])(\\.|[^\\/])*"/"i? {
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

"=="    { SQLANG_YY_OK_REGEX; return token::EQ; }
"!="    { SQLANG_YY_OK_REGEX; return token::NE; }
"<=>"   { SQLANG_YY_OK_REGEX; return token::CMP; }
"<="    { SQLANG_YY_OK_REGEX; return token::LE; }
">="    { SQLANG_YY_OK_REGEX; return token::GE; }
"=~"    { SQLANG_YY_OK_REGEX; return token::MATCH; }
"!~"    { SQLANG_YY_OK_REGEX; return token::NMATCH; }
"<"     { SQLANG_YY_OK_REGEX; return '<'; }
">"     { SQLANG_YY_OK_REGEX; return '>'; }
"&&"    { SQLANG_YY_OK_REGEX; return token::AND; }
"||"    { SQLANG_YY_OK_REGEX; return token::OR; }
"!"     { SQLANG_YY_OK_REGEX; return '!'; }
"++"    { SQLANG_YY_OK_REGEX; return token::INCREMENT; }
"+="    { SQLANG_YY_OK_REGEX; return token::PLUS_INPLACE; }
"+"     { SQLANG_YY_OK_REGEX; return '+'; }
"--"    { SQLANG_YY_OK_REGEX; return token::DECREMENT; }
"-="    { SQLANG_YY_OK_REGEX; return token::MINUS_INPLACE; }
"-"     { SQLANG_YY_OK_REGEX; return '-'; }
"*="    { SQLANG_YY_OK_REGEX; return token::MULTIPLY_INPLACE; }
"*"     { SQLANG_YY_OK_REGEX; return '*'; }
<NO_REGEX>"/="    { SQLANG_YY_OK_REGEX; return token::DIVIDE_INPLACE; }
<NO_REGEX>"/"     { SQLANG_YY_OK_REGEX; return '/'; }
"%="    { SQLANG_YY_OK_REGEX; return token::REMAINDER_INPLACE; }
"%"     { SQLANG_YY_OK_REGEX; return '%'; }
"="     { SQLANG_YY_OK_REGEX; return '='; }

"has"       { SQLANG_YY_OK_REGEX; return token::HAS; }
"if"        { SQLANG_YY_OK_REGEX; return token::IF; }
"else"      { SQLANG_YY_OK_REGEX; return token::ELSE; }
"for"       { SQLANG_YY_OK_REGEX; return token::FOR; }
"while"     { SQLANG_YY_OK_REGEX; return token::WHILE; }
"break"     { SQLANG_YY_OK_REGEX; return token::BREAK; }
"continue"  { SQLANG_YY_OK_REGEX; return token::CONTINUE; }
"function"  { SQLANG_YY_OK_REGEX; return token::FUNCTION; }
"return"    { SQLANG_YY_OK_REGEX; return token::RETURN; }
"as"        { SQLANG_YY_OK_REGEX; return token::AS; }
"bool"      { SQLANG_YY_OK_REGEX; return token::BOOL; }
"int"       { SQLANG_YY_OK_REGEX; return token::INT; }
"uint"      { SQLANG_YY_OK_REGEX; return token::UINT; }
"intlist"   { SQLANG_YY_OK_REGEX; return token::INTLIST; }
"real"      { SQLANG_YY_OK_REGEX; return token::REAL; }
"string"    { SQLANG_YY_OK_REGEX; return token::STRING; }
"obj"       { SQLANG_YY_OK_REGEX; return token::OBJ; }
"objlist"   { SQLANG_YY_OK_REGEX; return token::OBJLIST; }
"datetime"  { SQLANG_YY_OK_REGEX; return token::DATETIME; }
"date"      { SQLANG_YY_OK_REGEX; return token::DATE; }
"time"      { SQLANG_YY_OK_REGEX; return token::TIME; }

"true"      { SQLANG_YY_NO_REGEX; yylval_param->intVal = 1; return token::BOOL_LITERAL; }
"false"     { SQLANG_YY_NO_REGEX; yylval_param->intVal = 0; return token::BOOL_LITERAL; }
"null"      { SQLANG_YY_NO_REGEX; return token::NULL_LITERAL; }

{NAME} {
        SQLANG_YY_NO_REGEX;
        yylval_param->strVal = new slib::sStr("%s", yytext);
        return token::NAME;
    }

"("     { SQLANG_YY_OK_REGEX; return '('; }
")"     { SQLANG_YY_NO_REGEX; return ')'; }
"["     { SQLANG_YY_OK_REGEX; return '['; }
"]"     { SQLANG_YY_NO_REGEX; return ']'; }
"{"     { SQLANG_YY_OK_REGEX; return '{'; }
"}"     { SQLANG_YY_NO_REGEX; return '}'; }
"."     { SQLANG_YY_OK_REGEX; return '.'; }
","     { SQLANG_YY_OK_REGEX; return ','; }
":"     { SQLANG_YY_OK_REGEX; return ':'; }
";"     { SQLANG_YY_OK_REGEX; return ';'; }
"|"     { SQLANG_YY_OK_REGEX; return '|'; }
"?"     { SQLANG_YY_OK_REGEX; return '?'; }

<*>. {
        parser_driver.setError("invalid character", yylloc->begin.line, yylloc->begin.column);
        yyterminate();
    }

%%

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
    // yy_top_state() returns the *previous* state, not the current state. If the
    // previous state was TEMPLATE, we pop again and set current state to template.
    // Note: this is reasonable only because we know we need to pop either 1 or 2
    // states to get to TEMPLATE, and there is no TEMPLATE below TEMPLATE.
    // Unfortunately, Flex doesn't have a real API for reading the current state
    // or the entire state stack.
    yy_pop_state(scanner);
    if (yy_top_state(scanner) == TEMPLATE)
        yy_pop_state(scanner);
}
