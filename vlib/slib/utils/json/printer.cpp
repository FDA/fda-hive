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

#include <slib/utils/json/printer.hpp>
#include <slib/std/string.hpp>

#include <ctype.h>
#include <math.h>

using namespace slib;

const char * default_indent = "    ";
const char * default_newline = "\n";

inline sJSONPrinter::EState sJSONPrinter::curState() const
{
    return _states.dim() ? _states[_states.dim() - 1] : eNone;
}

static inline void newlineAndIndent(sStr * out, const char * newline, const char * indent, idx level)
{
    out->addString(newline);
    for(idx i=0; i<level; i++) {
        out->addString(indent);
    }
}

void sJSONPrinter::insertSeparator(bool multiline)
{
    bool comma = true;
    switch(curState()) {
        case eEmptyInlineArray:
            comma = false;
            _states[_states.dim() - 1] = eInlineArray;
        case eInlineArray:
            if( multiline ) {
                _states[_states.dim() - 1] = eMultilineArray;
            }
            break;
        case eEmptyMultilineArray:
            comma = false;
            _states[_states.dim() - 1] = eMultilineArray;
        case eMultilineArray:
            multiline = true;
            break;
        case eEmptyObject:
            comma = false;
        case eObjectValue:
            multiline = true;
            break;
        default:
            return;
    }

    if( comma ) {
        _out->addString(",", 1);
    }

    if( multiline ) {
        newlineAndIndent(_out, _newline, _indent, _states.dim());
    } else {
        _out->addString(_spacer);
    }
}

void sJSONPrinter::init(sStr * out, const char * indent, const char * newline)
{
    finish();
    _out = out;
    _space_buf.cut(0);
    idx indent_offset = -1, newline_offset = -1;
    if( indent ) {
        indent_offset = _space_buf.length();
        _space_buf.addString(indent);
        _space_buf.add0();
    }
    if( newline ) {
        newline_offset = _space_buf.length();
        _space_buf.addString(newline);
        _space_buf.add0();
    }
    _indent = indent_offset >= 0 ? _space_buf.ptr(indent_offset) : default_indent;
    _newline = newline_offset >= 0 ? _space_buf.ptr(newline_offset) : default_newline;
    _spacer = sLen(_indent) ? " " : sStr::zero;
}

void sJSONPrinter::finish(bool no_close_parens)
{
    if( !no_close_parens ) {
        for(idx i=_states.dim() - 1; i >= 0; i--) {
            switch(_states[i]) {
                case eEmptyObject:
                case eObjectKey:
                case eObjectValue:
                    endObject();
                    break;
                case eEmptyInlineArray:
                case eEmptyMultilineArray:
                case eInlineArray:
                case eMultilineArray:
                    endArray();
                    break;
                default:
                    break;
            }
        }
    }
    _out = 0;
    _states.cut(0);
}

#define START_COMPLEX_VALUE(block, type) \
    switch(curState()) { \
        case eNone: \
            do block while(0); \
            break; \
        case eEmptyInlineArray: \
        case eInlineArray: \
        case eEmptyMultilineArray: \
        case eMultilineArray: \
            insertSeparator(true); \
            do block while(0); \
            break; \
        case eObjectKey: \
            do block while(0); \
            _states[_states.dim() - 1] = eObjectValue; \
            break; \
        default: \
            return false; \
    } \
    *_states.add(1) = (type); \
    return true;

#define ADD_SCALAR_VALUE(block) \
    switch(curState()) { \
        case eNone: \
            do block while(0); \
            *_states.add(1) = eFinished; \
            break; \
        case eEmptyInlineArray: \
        case eInlineArray: \
        case eEmptyMultilineArray: \
        case eMultilineArray: \
            insertSeparator(false); \
            do block while(0); \
            break; \
        case eObjectKey: \
            do block while(0); \
            _states[_states.dim() - 1] = eObjectValue; \
            break; \
        default: \
            return false; \
    } \
    return true;

bool sJSONPrinter::startObject()
{
    START_COMPLEX_VALUE({
        _out->addString("{");
    }, eEmptyObject);
}

bool sJSONPrinter::addKey(const char * name, idx len)
{
    switch(curState()) {
        case eObjectKey:
            addNull();
        case eEmptyObject:
        case eObjectValue:
            insertSeparator(true);
            sString::escapeForJSON(*_out, name, len);
            _out->addString(":");
            _out->addString(_spacer);
            _states[_states.dim() - 1] = eObjectKey;
            break;
        default:
            return false;
    }
    return true;
}

bool sJSONPrinter::endObject()
{
    switch(curState()) {
        case eObjectKey:
            addNull();
        case eObjectValue:
            newlineAndIndent(_out, _newline, _indent, _states.dim() - 1);
        case eEmptyObject:
            _out->addString("}");
            _states.cut(_states.dim() - 1);
            if( !_states.dim() ) {
                *_states.add(1) = eFinished;
            }
            break;
        default:
            return false;
    }
    return true;
}

bool sJSONPrinter::startArray(bool force_multiline)
{
    START_COMPLEX_VALUE({
        _out->addString("[");
    }, force_multiline ? eEmptyMultilineArray : eEmptyInlineArray);
}

bool sJSONPrinter::endArray()
{
    switch(curState()) {
        case eEmptyInlineArray:
        case eEmptyMultilineArray:
            break;
        case eMultilineArray:
            newlineAndIndent(_out, _newline, _indent, _states.dim() - 1);
            break;
        case eInlineArray:
            _out->addString(_spacer);
            break;
        default:
            return false;
    }
    _out->addString("]");
    _states.cut(_states.dim() - 1);
    if( !_states.dim() ) {
        *_states.add(1) = eFinished;
    }
    return true;
}

bool sJSONPrinter::addNull()
{
    ADD_SCALAR_VALUE({
        _out->addString("null");
    });
}

bool sJSONPrinter::addValue(const char * val, idx len, bool empty_as_null)
{
    ADD_SCALAR_VALUE({
        if( empty_as_null && (!val || !*val) ) {
            _out->addString("null");
        } else {
            sString::escapeForJSON(*_out, val ? val : sStr::zero, len);
        }
    });
}

static const char * valid_real_fmt(const char * fmt)
{
    if( !fmt ) {
        return 0;
    }
    if( *fmt == '%' ) {
        const char * f = fmt;
        for(f++; *f == '#' || *f == '0' || *f == '-' || *f == '+' || *f == ' '; f++);
        for(; isdigit(*f); f++);
        if( *f == '.' ) {
            f++;
        }
        for(; isdigit(*f); f++);
        if( (*f == 'e' || *f == 'E' || *f == 'f' || *f == 'F' || *f == 'g' || *f == 'G') && f[1] == 0 ) {
            return fmt;
        } else {
            return 0;
        }
    } else if( isdigit(*fmt) || ((*fmt == '+' || *fmt == '-') && isdigit(fmt[1])) ) {
        idx pos = 0;

        if( fmt[pos] == '+' ) {
            fmt++;
        } else if( fmt[pos] == '-' ) {
            pos = 1;
        }

        if( fmt[pos] == '0' ) {
            pos++;
            if( fmt[pos] && fmt[pos] != '.' ) {
                return 0;
            }
        } else if( isdigit(fmt[pos]) ) {
            do {
                pos++;
            } while(isdigit(fmt[pos]));
        } else {
            return 0;
        }

        if( fmt[pos] == '.' ) {
            pos++;
            if( !isdigit(fmt[pos]) ) {
                return 0;
            }
            do {
                pos++;
            } while(isdigit(fmt[pos]));
        }

        if( fmt[pos] == 'e' || fmt[pos] == 'E' ) {
            pos++;
            if( fmt[pos] == '+' || fmt[pos] == '-' ) {
                pos++;
            }
            if( !isdigit(fmt[pos]) ) {
                return 0;
            }
            do {
                pos++;
            } while(isdigit(fmt[pos]));
        }

        if( fmt[pos] ) {
            return 0;
        }

        return fmt;
    }
    return 0;
}

bool sJSONPrinter::addValue(real r, const char * fmt)
{
    ADD_SCALAR_VALUE({
        if( unlikely(isnan(r)) ) {
            _out->addString("\"NaN\"");
        } else if( unlikely(isinf(r)) ) {
           if( r > 0 ) {
               _out->addString("\"Infinity\"");
           } else {
               _out->addString("\"-Infinity\"");
           }
        } else {
            fmt = valid_real_fmt(fmt);
            if( !fmt ) {
                fmt = "%g";
            }
            _out->printf(fmt, r);
        }
    });
}

bool sJSONPrinter::addValue(idx i)
{
    ADD_SCALAR_VALUE({
        _out->printf("%" DEC, i);
    });
}

bool sJSONPrinter::addValue(udx u)
{
    ADD_SCALAR_VALUE({
        _out->printf("%" UDEC, u);
    });
}

bool sJSONPrinter::addValue(char c)
{
    ADD_SCALAR_VALUE({
        sString::escapeForJSON(*_out, &c, 1);
    });
}

bool sJSONPrinter::addValue(bool b)
{
    ADD_SCALAR_VALUE({
        _out->addString(b ? "true" : "false");
    });
}

bool sJSONPrinter::addValue(const sHiveId & id)
{
    ADD_SCALAR_VALUE({
        sVariant val;
        val.setHiveId(id);
        val.print(*_out, sVariant::eJSON);
    });
}

bool sJSONPrinter::addValue(const sVariant & val)
{
    ADD_SCALAR_VALUE({
        sVariant::Whitespace ws;
        ws.space = _spacer;
        ws.indent = _indent;
        ws.newline = _newline;
        val.print(*_out, sVariant::eJSON, &ws, _states.dim());
    });
}

bool sJSONPrinter::addRaw(const char * txt, idx len)
{
    ADD_SCALAR_VALUE({
        _out->addString(txt, len);
    });
}
