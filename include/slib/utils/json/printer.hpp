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
#ifndef sLib_json_printer_h
#define sLib_json_printer_h

#include <slib/core/str.hpp>
#include <slib/std/variant.hpp>

namespace slib {
    class sJSONPrinter {
        private:
            enum EState {
                eNone,
                eFinished,
                eEmptyInlineArray,
                eEmptyMultilineArray,
                eInlineArray,
                eMultilineArray,
                eEmptyObject,
                eObjectKey,
                eObjectValue,
            };
            sVec<EState> _states;
            sStr _space_buf, _conv_buf;
            sStr * _out;
            const char * _indent;
            const char * _newline;
            const char * _spacer;

            EState curState() const;
            void insertSeparator(bool multiline);

        public:
            sJSONPrinter(sStr * out = 0, const char * indent = 0, const char * newline = 0)
            {
                _out = 0;
                _indent = _newline = _spacer = 0;
                init(out, indent, newline);
            }

            ~sJSONPrinter()
            {
                finish();
            }

            void init(sStr * out, const char * indent = 0, const char * newline = 0);
            void finish(bool no_close_parens = false);
            bool startObject();
            bool addKey(const char * name, idx len = 0);
            bool endObject();
            bool startArray(bool force_multiline = false);
            bool endArray();
            bool addNull();
            bool addValue(const char * val, idx len = 0, bool empty_as_null = false);
            bool addKeyValue(const char * name, const char * val, idx len = 0, bool empty_as_null = false) { return addKey(name) && addValue(val, len, empty_as_null); }
            bool addValue(real r, const char * fmt = 0);
            bool addKeyValue(const char * name, real r, const char * fmt = 0) { return addKey(name) && addValue(r, fmt); }
            bool addValue(idx i);
            bool addValue(int i) { return addValue((idx)i); }
            bool addKeyValue(const char * name, idx i) { return addKey(name) && addValue(i); }
            bool addKeyValue(const char * name, int i) { return addKey(name) && addValue(i); }
            bool addValue(udx u);
            bool addKeyValue(const char * name, udx u) { return addKey(name) && addValue(u); }
            bool addValue(char c);
            bool addKeyValue(const char * name, char c) { return addKey(name) && addValue(c); }
            bool addValue(bool b);
            bool addKeyValue(const char * name, bool b) { return addKey(name) && addValue(b); }
            bool addValue(const sHiveId & id);
            bool addKeyValue(const char * name, const sHiveId & id) { return addKey(name) && addValue(id); }
            bool addValue(const sVariant & val);
            bool addKeyValue(const char * name, const sVariant & val) { return addKey(name) && addValue(val); }
            bool addRaw(const char * txt, idx len = 0);
    };
};

#endif
