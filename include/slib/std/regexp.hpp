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
#ifndef sLib_regexp_h
#define sLib_regexp_h

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>

namespace slib {
    class sRegExp {
        public:
            enum EFlags {
                fIgnoreCase = 1,
                fGlobal = 1 << 1,
                fMultiline = 1 << 2
            };
            static bool use_jit;
            sRegExp(const char * pat = 0, idx flags = fIgnoreCase)
            {
                _flags = flags;
                init(pat, flags);
            }
            bool init(const char * pat, idx flags = fIgnoreCase);
            void destroy();
            ~sRegExp() { destroy(); }
            bool ok() const;
            idx flags() const { return _flags; }

            static bool isPCRE();

            const char * search(const char * str, idx len = 0, idx * out_len_matched = 0);
            idx exec(sVec<sMex::Pos> & res, const char * str, idx len = 0);
            idx replace(sStr & out, const char * str, idx str_len, const char * repl);
            const char * err() const { return _err.length() ? _err.ptr() : 0; }

        private:
            struct Priv;
            Priv * getPriv();

            idx _flags;
            sMex _mex;
            sStr _err;
    };
};

#endif
