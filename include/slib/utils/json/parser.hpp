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
    //! Driver for JSON parser. Generates an sVariant.
    class sJSONParser {


    public:
        sJSONParser();
        virtual ~sJSONParser() {}

        enum ENodeType {
            eNodeTopLevel, //! top-level value not inside any container
            eNodeArrayElement, //! array element with integer index
            eNodeObjectElement //! object element with string key
        };
        enum EValueType {
            eValueNull,
            eValueBool,
            eValueInt,
            eValueReal,
            eValueString,
            eValueArray,
            eValueObject,
            eValueEndOfContainer //! pseudo-value indicating end of array or object
        };
        struct ParseNode {
            ENodeType node_type;
            EValueType value_type;

            idx depth; //! incremented by 1 inside each array or object
            idx index; //! index of element in array, or number of key/value pair in object

            // Only used for object elements
            const char * key_str; //! object key decoded as string, or 0 if inapplicable
            idx key_str_len; //! length of key_str in bytes

            const char * key_raw; //! pointer to raw object key (non-decoded, with quotes etc.) in source buffer, or 0 if inapplicable
            idx key_raw_len; //! length of key_raw in bytes

            idx key_line; //! location of key_raw in source: line number (-1 if inapplicable)
            idx key_col; //! location of key_raw in source: column number (-1 if inapplicable)
            idx key_pos; //! location of key_raw in source: offset from start in bytes (-1 if inapplicable)

            union {
                idx i; //! used for eBool and eInt values
                real r; //! used for eReal values
                const char * str; //! used for eString values
            } val; //! value
            idx val_str_len; //! length of val.str in bytes

            const char * val_raw; //! pointer to raw value (non-decoded, with quotes etc.) in source buffer
            idx val_raw_len; //! length of val_raw in bytes

            idx val_line; //! location of val_raw in source: line number
            idx val_col; //! location of val_raw in source: column number
            idx val_pos; //! location of val_raw in source: offset from start in bytes
        };
        typedef void (*parseCallback)(ParseNode & node, sJSONParser & parser, void * param);

        //! define parser callback
        /*! \param cb callback to call
            \param cb_param param for callback
            \param no_result if true, JSON result will not be accumulated into sVariant, so result() will return nothing
            \param decode_buf optional buffer to use for decoding escaped characters in JSON strings */
        void setParseCallback(parseCallback cb, void * cb_param, bool no_result, sStr * decode_buf = 0);
        enum EParseFlags {
            fParseEmpty = 1 << 0, //!< empty string or pure whitespace is considered to be valid JSON text (however, parseCallback will not be called on it)
            fParsePrefix = 1 << 1 //!< parser stops when valid JSON text ends, so e.g. JSON text followed by other data is considered valid
        };
        //! parse prefix of buffer as JSON text
        /*! \param buf buffer to parse
            \param len length of buffer; if 0, buf is assumed to be 0-terminated and length calculated automatically
            \param[out] plen_parsed number of bytes parsed (set to 0 on failure)
            \param flags bitwise-OR of EParseFlags
            \returns number of bytes parsed, if parsing succeeded */
        bool parse(const char * buf, idx len = 0, idx * plen_parsed = 0, idx flags = fParseEmpty);
        //! parse prefix of buffer as JSON text
        /*! \param buf buffer to parse
            \param[out] plen_parsed number of bytes parsed (set to 0 on failure)
            \param flags bitwise-OR of EParseFlags
            \returns number of bytes parsed, if parsing succeeded */
        bool parse(const sStr & s, idx * plen_parsed = 0, idx flags = fParseEmpty)
        {
            return parse(s.length() ? s.ptr() : 0, 0, plen_parsed, flags);
        }
        //! return parsed result (assuming a parse callback with no_result=true had not been set)
        sVariant & result() { return _result; }

        //! can be used inside parseCallback
        void stopParser();

        void setError(idx line, idx col, const char * raw, const char * msg_fmt, ...);
        void setKeyError(ParseNode & node, const char * msg_fmt, ...);
        void setValueError(ParseNode & node, const char * msg_fmt, ...);
        void vsetError(idx line, idx col, const char * raw, const char * msg_fmt, va_list ap);
        void clearError();
        const char * printError(sStr & out);

    public:
        bool debug_lexer; //<! print lexer diagnostic messages
        bool debug_parser; //<! print parser diagnostic messages

    private:
        enum EToken {
            eTokenError, // unknown token
            eTokenEOF, // end of file
            eTokenNull,
            eTokenBool,
            eTokenInt,
            eTokenReal,
            eTokenString,
            eTokenComma, // ,
            eTokenColon, // :
            eTokenOpenArray, // [
            eTokenCloseArray, // ]
            eTokenOpenObject, // {
            eTokenCloseObject // }
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
