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

#include "json-lexer.hpp"

#include <assert.h>
#include <ctype.h>
// In glibc <= 2.18, __STDC_LIMIT_MACROS must be defined before including stdint.h
// if you want to use INTn_MAX/INTn_MIN macros
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif
#include <stdint.h>

using namespace slib;

uint64_t readUCS2Hex(const char * src)
{
    uint64_t code = 0;

    for(idx i=0; i<4; i++) {
        uint16_t nibble = 0;
        if( src[i] >= '0' && src[i] <= '9' ) {
            nibble = src[i] - '0';
        } else if ( src[i] >= 'a' && src[i] <= 'f' ) {
            nibble = 10 + src[i] - 'a';
        } else if ( src[i] >= 'A' && src[i] <= 'F' ) {
            nibble = 10 + src[i] - 'A';
        } else {
            return UINT64_MAX;
        }

        code += nibble << (4 * (3 - i));
    }

    return code;
}

static idx decodeUTF16Hex(sStr & dst, const char * src, idx pos, idx len)
{
    // expect \uXXXX
    if( len - pos < 6 ) {
        return 0;
    }

    if( src[pos] != '\\' || src[pos + 1] != 'u' ) {
        return 0;
    }

    uint64_t code = readUCS2Hex(src + pos + 2);
    if( code == UINT64_MAX ) {
        // error condition
        return 0;
    }

    idx ret = 6;

    if( code >= 0xd800 && code <= 0xdbff && len - pos >= 12 && src[pos + 6] == '\\' && src[pos + 7] == 'u' ) {
        // surrogate pair
        uint64_t code2 = readUCS2Hex(src + pos + 8);
        if( code2 == UINT64_MAX ) {
            // error condition
            return 0;
        }

        if( code2 >= 0xdc00 && code2 <= 0xdfff ) {
            // second surrogate pair found
            code -= 0xd800;
            code <<= 10;
            code += code2 - 0xdc00 + 0x10000;
            ret = 12;
        }
    }

    if( code < 0x80 ) {
        unsigned char * dst_buf = (unsigned char *)dst.add(0, 1);
        dst_buf[0] = (unsigned char)code;
    } else if( code < 0x800 ) {
        unsigned char * dst_buf = (unsigned char *)dst.add(0, 2);
        dst_buf[0] = 0xC0 | (code >> 6);
        dst_buf[1] = 0x80 | (code & 0x3F);
    } else if( code < 0x10000 ) {
        unsigned char * dst_buf = (unsigned char *)dst.add(0, 3);
        dst_buf[0] = 0xE0 | (code >> 12);
        dst_buf[1] = 0x80 | ((code >> 6) & 0x3F);
        dst_buf[2] = 0x80 | (code & 0x3F);
    } else {
        unsigned char * dst_buf = (unsigned char *)dst.add(0, 4);
        dst_buf[0] = 0xF0 | (code >> 18);
        dst_buf[1] = 0x80 | ((code >> 12) & 0x3F);
        dst_buf[2] = 0x80 | ((code >> 6) & 0x3F);
        dst_buf[3] = 0x80 | (code & 0x3F);
    }

    return ret;
}

sJSONParser::EToken sJSONParser::Lexer::lex(sJSONParser::TokenValue & val)
{
    // skip any whitespace
    while( _pos < _len ) {
        char c = _inp[_pos];
        if( c == ' ' || c == '\t' ) {
            incrPosCol();
        } else if( c == '\n' ) {
            incrPosNewLine();
        } else if( c == '\r' ) {
            incrPosNewLine();
            if( _pos < _len && _inp[_pos] == '\n' ) {
                _pos++;
            }
        } else {
            break;
        }
    }

    val.start_line = _line;
    val.start_col = _col;
    val.start_pos = sMin<idx>(_pos, _len - 1);
    val.raw_len = 0;
    val.num.i = 0;
    val.str_start = -1;
    val.str_len = 0;
    val.str_in_decode_buf = false;

    if( _pos >= _len ) {
        return eTokenEOF;
    }

    char c = _inp[_pos];

    switch( c ) {
        case 0:
            return eTokenEOF;
        case ',':
            incrPosCol();
            return eTokenComma;
        case ':':
            incrPosCol();
            return eTokenColon;
        case '[':
            incrPosCol();
            return eTokenOpenArray;
        case ']':
            incrPosCol();
            return eTokenCloseArray;
        case '{':
            incrPosCol();
            return eTokenOpenObject;
        case '}':
            incrPosCol();
            return eTokenCloseObject;
        // no default
    }

    if( _len - _pos >= 4 /* strlen("null") */ && strncmp(_inp + _pos, "null", 4) == 0 ) {
        val.raw_len = 4;
        incrPosCol(val.raw_len);
        return eTokenNull;
    }

    if( _len - _pos >= 4 /* strlen("true") */ && strncmp(_inp + _pos, "true", 4) == 0 ) {
        val.raw_len = 4;
        incrPosCol(val.raw_len);
        val.num.i = 1;
        return eTokenBool;
    }

    if( _len - _pos >= 5 /* strlen("false") */ && strncmp(_inp + _pos, "false", 5) == 0 ) {
        val.raw_len = 5;
        incrPosCol(val.raw_len);
        val.num.i = 0;
        return eTokenBool;
    }

    if( isdigit(c) || c == '-' ) {
        // integer or real
        bool is_real = false;
        bool is_negate = false;
        if( c == '-' ) {
            is_negate = true;
            incrPosCol();
            c = _inp[_pos];
        }
        if( c == '0' && _pos + 1 < _len && isdigit(_inp[_pos + 1]) ) {
            _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: ECMA-404 standard does not allow leading zeroes in numeric values");
            return eTokenError;
        }

        val.num.i = c - '0';
        for( incrPosCol(); _pos < _len && isdigit(_inp[_pos]); incrPosCol()) {
            idx next = val.num.i * 10 + (_inp[_pos] - '0');
            if( next < val.num.i ) {
                is_real = true; // integer overflow
            }
            val.num.i = next;
        }
        if( is_negate ) {
            val.num.i = -val.num.i;
        }

        if( _pos < _len && _inp[_pos] == '.' ) {
            is_real = true;
            incrPosCol();
            if( _pos >= _len || !isdigit(_inp[_pos]) ) {
                _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: ECMA-404 standard does not allow empty fractional part in floating point values");
                return eTokenError;
            }
            for( ; _pos < _len && isdigit(_inp[_pos]); incrPosCol());
        }

        if( _pos < _len && (_inp[_pos] == 'e' || _inp[_pos] == 'E') ) {
            is_real = true;
            incrPosCol();
            if( _pos < _len && (_inp[_pos] == '+' || _inp[_pos] == '-') ) {
                incrPosCol();
            }
            if( _pos >= _len || !isdigit(_inp[_pos]) ) {
                _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: ECMA-404 standard does not allow empty exponent in floating point values");
                return eTokenError;
            }
            for( ; _pos < _len && isdigit(_inp[_pos]); incrPosCol());
        }

        val.raw_len = _pos - val.start_pos;
        const char * conversion = _inp + val.start_pos;
        if( _pos >= _len ) {
            _conversion_buf.cut(0);
            _conversion_buf.addString(_inp + val.start_pos, val.raw_len);
            conversion = _conversion_buf.ptr();
        }

        if( is_real ) {
            char * end = 0;
            val.num.r = strtod(conversion, &end);
            if( end - conversion != val.raw_len ) {
                val.raw_len = 0;
                _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: invalid numeric value");
                return eTokenError;
            } else {
                return eTokenReal;
            }
        } else {
            return eTokenInt;
        }
    }

    if( c == '"' ) {
        // string
        incrPosCol(); // skip opening quote
        val.str_start = _pos;
        while( _pos < _len && _inp[_pos] != '"' ) {
            if( _inp[_pos] == '\\' ) {
                incrPosCol();
                if( _pos >= _len ) {
                    _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: unterminated string");
                    return eTokenError;
                }

                if( !val.str_in_decode_buf ) {
                    val.str_in_decode_buf = true;
                    val.str_start = val.decode_buf.length();
                    idx copy_pos = val.start_pos + 1; // exclude initial "
                    idx copy_len = _pos - copy_pos - 1; // exclude \ that we just scanned
                    if( copy_len ) {
                        val.decode_buf.add(_inp + copy_pos, copy_len);
                    }
                }

                switch( _inp[_pos] ) {
                case '"':
                case '\\':
                case '/':
                    val.decode_buf.add(_inp + _pos, 1);
                    incrPosCol();
                    break;
                case 'b':
                    val.decode_buf.add("\b", 1);
                    incrPosCol();
                    break;
                case 'f':
                    val.decode_buf.add("\f", 1);
                    incrPosCol();
                    break;
                case 'n':
                    val.decode_buf.add("\n", 1);
                    incrPosCol();
                    break;
                case 'r':
                    val.decode_buf.add("\r", 1);
                    incrPosCol();
                    break;
                case 't':
                    val.decode_buf.add("\t", 1);
                    incrPosCol();
                    break;
                case 'u':
                    if( idx len_decoded = decodeUTF16Hex(val.decode_buf, _inp, _pos - 1, _len) ) {
                        incrPosCol(len_decoded - 1);
                    } else {
                        _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: invalid unicode escape in string");
                        return eTokenError;
                    }
                    break;
                default:
                    _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: invalid escape code in string");
                    return eTokenError;
                }
            } else if( (unsigned char)_inp[_pos] >= (unsigned char)' ' ){
                if( val.str_in_decode_buf ) {
                    val.decode_buf.add(_inp + _pos, 1);
                }

                if( _inp[_pos] == '\r' ) {
                    incrPosNewLine();
                } else if( _inp[_pos] == '\n' ) {
                    if( _inp[_pos - 1] == '\r' ) {
                        _pos++;
                    } else {
                        incrPosNewLine();
                    }
                } else {
                    incrPosCol();
                }
            } else {
                _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: unescaped control character in string");
                return eTokenError;
            }
        }

        if( _pos < _len ) {
            assert(_inp[_pos] == '"');
            incrPosCol();
            val.raw_len = _pos - val.start_pos;
            if( val.str_in_decode_buf ) {
                val.str_len = val.decode_buf.length() - val.str_start;
                val.decode_buf.add0cut();
            } else {
                val.str_len = _pos - val.str_start - 1; // exclude terminal "
            }
            return eTokenString;
        } else {
            // buffer ended, terminal " not found
            _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: unterminated string value");
            return eTokenError;
        }
    }

    if( isprint(c) ) {
        _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: unexpected character '%c'", c);
    } else {
        _parser->setError(_line, _col, _inp + _pos, "JSON syntax error: unexpected character 0x%02x", (unsigned int)c);
    }
    return eTokenError;
}
