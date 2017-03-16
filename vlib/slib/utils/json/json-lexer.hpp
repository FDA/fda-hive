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
#pragma once
#ifndef sLib_json_lexer_h
#define sLib_json_lexer_h

#include <slib/utils/json/parser.hpp>

namespace slib {
    struct sJSONParser::TokenValue {
        idx start_line;
        idx start_col;
        idx start_pos;
        idx raw_len;

        union {
            real r;
            idx i;
        } num;

        idx str_start;
        idx str_len;
        bool str_in_decode_buf;
        sStr & decode_buf;

        TokenValue(sStr & decode_buf_) : decode_buf(decode_buf_)
        {
            start_line = start_col = start_pos = raw_len = 0;
            num.i = 0;
            str_start = str_len = 0;
            str_in_decode_buf = false;
        }
    };

    class sJSONParser::Lexer {
        public:
            Lexer(sJSONParser * parser, const char * inp, idx len)
            {
                _parser = parser;
                _inp = inp;
                _len = len;
                _line = _col = _pos = 0;
            }
            EToken lex(TokenValue & val);
            idx getPos() const { return _pos; }

        private:
            sJSONParser * _parser;
            const char * _inp;
            idx _len;

            idx _line;
            idx _col;
            idx _pos;
            void incrPosCol(idx i=1)
            {
                _col += i;
                _pos += i;
            }
            void incrPosNewLine(idx i=1)
            {
                _pos += i;
                _col = 0;
                _line++;
            }
            sStr _conversion_buf;
    };
};

#endif
