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
    //! Regular expression matching. Wraps around PCRE or POSIX regular expression engine.
    class sRegExp {
        public:
            enum EFlags {
                fIgnoreCase = 1, //!< case-insensitive match; "i" flag in perl and javascript
                fGlobal = 1 << 1, //!< exec/replace don't stop after first match; "g" flag in perl and javascript
                fMultiline = 1 << 2 //!< $ and ^ match newlines within data; "m" flag in perl and javascript
            };
            //! If 1, regular expressions in PCRE mode will be compiled with JIT (faster matching, but e.g. debugging with valgrind will require \--smc-check=all flag)
            /*! Initialized to true by default, or to value of REGEXP_JIT environment variable if it had been set */
            static bool use_jit;
            //! construct and initialize sRegExp object
            /*! \param pat regular expression pattern valid for current engine (PCRE or POSIX), or 0 to not compile
                \param flags bitwise-or of sRegExp::EFlags
                \note in PCRE mode, regexp will be compiled in JIT mode if (as by default) sRegExp::use_jit is true;
                      so you will need \--smc-check-all flag if debugging in valgrind */
            sRegExp(const char * pat = 0, idx flags = fIgnoreCase)
            {
                _flags = flags;
                init(pat, flags);
            }
            //! (re)initialize sRegExp object
            /*! \param pat regular expression pattern valid for current engine (PCRE or POSIX)
                \param flags bitwise-or of sRegExp::EFlags
                \returns true if the regexp was compiled successfully
                \note in PCRE mode, regexp will be compiled in JIT mode if (as by default) sRegExp::use_jit is true;
                      so you will need \--smc-check-all flag if debugging in valgrind */
            bool init(const char * pat, idx flags = fIgnoreCase);
            //! deallocate compiled regular expression
            void destroy();
            ~sRegExp() { destroy(); }
            //! check if the regexp had been compiled successfully
            bool ok() const;
            //! bitwise-or of sRegExp::EFlags set in initialization
            idx flags() const { return _flags; }

            //! Check if PCRE syntax is supported
            /*! \returns true if using PCRE engine, false if using POSIX engine */
            static bool isPCRE();

            //! Search for first match
            /*! \param str string to search
                \param len length of str, or 0 to assume str is 0-terminated
                \param[out] out_len_matched retrieves length of match or -1 on match failure;
                            pass NULL pointer to ignore.
                \warning in POSIX mode, if len is non-0, str will be copied to a temporary buffer
                         because POSIX regular expressions require 0-terminated strings
                \returns pointer to first match in str on success, 0 on failure */
            const char * search(const char * str, idx len = 0, idx * out_len_matched = 0);
            //! Execute to obtain match and submatches; or in fGlobal mode, execute repeatedly and retrieve all top-level matches
            /*! \param[out] res obtained matches/submatches. Normally, the first element (res[0] assuming
                                res was initially empty) is the top-level match, and further elements
                                added are submatches. In fGlobal mode, all elements added to res are
                                top-level matches obtained by repeated execution.
                \param str string to search
                \param len length of str, or 0 to assume str is 0-terminated
                \warning in POSIX mode, if len is non-0, str will be copied to a temporary buffer
                         because POSIX regular expressions require 0-terminated strings
                \returns number of matches/submatches found and added to res, or 0 on failure */
            idx exec(sVec<sMex::Pos> & res, const char * str, idx len = 0);
            //! Replace substrings from a buffer (repeatedly in fGlobal mode)
            /*! \param[out] out buffer in which result will be printed
                \param str string to search
                \param str_len length of str, or 0 to assume str is 0-terminated
                \param repl replacement pattern with the standard $ codes:
                            - $1 ... $99 - insert submatch #1 ... 99
                            - $& - insert entire matched substring
                            - $` - insert substring before the match
                            - $' - insert substring following the match
                            - $$ - insert '$' character
                \warning in POSIX mode, if len is non-0, str will be copied to a temporary buffer
                         because POSIX regular expressions require 0-terminated strings
                \returns number of replacements made, or 0 on failure
                \note if no matches are found and no replacements are made, str will be copied into out */
            idx replace(sStr & out, const char * str, idx str_len, const char * repl);
            //! Error string from underlying engine if pattern failed to compile
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
