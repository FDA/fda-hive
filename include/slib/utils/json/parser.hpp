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
#ifndef sLib_json_parser_h
#define sLib_json_parser_h

#include <slib/core/str.hpp>
#include <slib/std/variant.hpp>

namespace slib {
    class sJSONParser {


    public:
        sJSONParser();
        virtual ~sJSONParser() {}

        enum ENodeType {
            eNodeTopLevel,
            eNodeArrayElement,
            eNodeObjectElement
        };
        enum EValueType {
            eValueNull,
            eValueBool,
            eValueInt,
            eValueReal,
            eValueString,
            eValueArray,
            eValueObject,
            eValueEndOfContainer
        };
        struct ParseNode {
            ENodeType node_type;
            EValueType value_type;

            idx depth;
            idx index;

            const char * key_str;
            idx key_str_len;

            const char * key_raw;
            idx key_raw_len;

            idx key_line;
            idx key_col;
            idx key_pos;

            union {
                idx i;
                real r;
                const char * str;
            } val;
            idx val_str_len;

            const char * val_raw;
            idx val_raw_len;

            idx val_line;
            idx val_col;
            idx val_pos;
        };
        typedef void (*parseCallback)(ParseNode & node, sJSONParser & parser, void * param);

        void setParseCallback(parseCallback cb, void * cb_param, bool no_result, sStr * decode_buf = 0);
        enum EParseFlags {
            fParseEmpty = 1 << 0,
            fParsePrefix = 1 << 1
        };
        bool parse(const char * buf, idx len = 0, idx * plen_parsed = 0, idx flags = fParseEmpty);
        bool parse(const sStr & s, idx * plen_parsed = 0, idx flags = fParseEmpty)
        {
            return parse(s.length() ? s.ptr() : 0, 0, plen_parsed, flags);
        }
        sVariant & result() { return _result; }

        void stopParser();

        void setError(idx line, idx col, const char * raw, const char * msg_fmt, ...);
        void setKeyError(ParseNode & node, const char * msg_fmt, ...);
        void setValueError(ParseNode & node, const char * msg_fmt, ...);
        void vsetError(idx line, idx col, const char * raw, const char * msg_fmt, va_list ap);
        void clearError();
        const char * printError(sStr & out);

    public:
        bool debug_lexer;
        bool debug_parser;

    private:
        enum EToken {
            eTokenError,
            eTokenEOF,
            eTokenNull,
            eTokenBool,
            eTokenInt,
            eTokenReal,
            eTokenString,
            eTokenComma,
            eTokenColon,
            eTokenOpenArray,
            eTokenCloseArray,
            eTokenOpenObject,
            eTokenCloseObject
        };
        struct NodeLevel {
            ENodeType node_type;
            idx index;
            sVariant * accum;
        };

        sStr _error;
        sVec<NodeLevel> _levels;
        sStr * _decode_buf;
        parseCallback _cb;
        void * _cb_param;
        bool _no_result;
        bool _stopped;
        const char *_buf;
        idx _buflen;
        sVariant _result;

        static const char * tokenName(EToken token);

        class Lexer;
        struct TokenValue;

        bool handleValue(ParseNode & node, EToken token, TokenValue * tokval);
        void wrongToken(EToken token, const TokenValue * tokval, const char * expected);
    };
};

#endif
