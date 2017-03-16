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
#include <qlang/parser.hpp>
#include "lexerExtras.hpp"
#define YY_HEADER_EXPORT_START_CONDITIONS 1
#include "qlang-flex.hpp"
#include "parserPrivate.hpp"

using namespace slib;

// needed for Bison
void yy::sQLangBison::error(const yy::sQLangBison::location_type& loc, const std::string& msg)
{
    parser_driver.setError(msg.c_str(), loc.begin.line, loc.begin.column);
}

bool qlang::Parser::parse(const char *buf, const char *bufend, idx flags)
{
    delete _astRoot;
    _astRoot = NULL;
    _error.cut(0);

    _flags = flags;

    _buf = _bufcur = buf;
    _bufend = bufend ? bufend : _buf + sLen(_buf);

    qlang::ParserPriv priv;
    _ppriv = &priv;

    yylex_init_extra(this, &(priv.scanner));
    yyPushState(INITIAL);

    yy::sQLangBison parser_bison(*this, priv.scanner);
    parser_bison.set_debug_level(debugParser);

    if (flags & fTemplate)
        yyPushState(TEMPLATE);

    int res = parser_bison.parse();

    yylex_destroy(priv.scanner);
    _ppriv = NULL;

    // Need to check _error.length() because it might be e.g. set by the lexer for invalid trailing characters
    return !res && !_error.length();
}

bool qlang::Parser::parse(const sStr &s, idx flags)
{
    if(!s.length())
        return false;

    return parse(s.ptr(), s.last());
}

void qlang::Parser::setError(const char *s, idx line, idx col)
{
    _error.printf(0, "%"DEC":%"DEC": %s", line, col, s);
    if (!_buf)
        return;

    // find beginning of line with error
    const char * c = _buf;
    for (idx i=1; i<line; i++) {
        while (c < _bufend && *c && *c != '\r' && *c != '\n')
            c++;

        if (*c == '\r' && c + 1 < _bufend && c[1] == '\n')
            c+=2;
        else if (c < _bufend && (*c == '\r' || *c == '\n'))
            c++;
    }

    if (c >= _bufend)
        return;

    const char *eol;
    for (eol = c; eol < _bufend && *eol && *eol != '\r' && *eol != '\n'; eol++);
    _error.shrink00();
    _error.add("\n\n", 2);
    if (eol > c)
        _error.add(c, eol - c);
    _error.add("\n", 1);
    for (idx i=1; i<col; i++)
        _error.add(" ", 1);
    _error.printf("^\n");
}

void qlang::Parser::printError(sStr &s)
{
    if (_error.length())
        s.printf("%s", _error.ptr());
}

const char * qlang::Parser::getErrorStr()
{
    return _error.length() ? _error.ptr() : sStr::zero;
}
