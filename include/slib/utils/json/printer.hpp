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
    //! Low-overhead pretty printer in JSON format
    /*! Typical usage:
     \code
         sStr buf;
         sJSONPrinter printer(&buf);
         printer.startObject();
         printer.addKey("Hello");
         printer.addValue("World");
         printer.addKey("Forgot value?");
         printer.addKey("Bye");
         printer.startArray();
         printer.addValue(false);
         printer.addValue(1);
         printer.addValue(2.345, "%.2f");
         printer.endArray();
         printer.finish(); // takes care of endObject() automatically
         printf("%s\n", buf.ptr());
     \endcode
     will output
     \code
        {
            "Hello": "World",
            "Forgot value?": null,
            "Bye": [ false, 1, 2.35 ]
        }
     \endcode
     */
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

            EState curState() const;
            void insertSeparator(bool multiline);

        public:
            /*! \param out string buffer into which to print; if 0, must be set later using init()
             * \param indent indentation at each level; if 0, 4 spaces will be used
             * \param newline newline characters; if 0, "\r\n" will be used */
            sJSONPrinter(sStr * out = 0, const char * indent = 0, const char * newline = 0)
            {
                _out = 0;
                _indent = _newline = 0;
                init(out, indent, newline);
            }

            ~sJSONPrinter()
            {
                finish();
            }

            //! initialize or reset the printer
            /*! \param out string buffer into which to print
             * \param indent indentation at each level; if 0, 4 spaces will be used
             * \param newline newline characters; if 0, "\r\n" will be used */
            void init(sStr * out, const char * indent = 0, const char * newline = 0);
            //! add all necessary ']' and '}' and null values for value-less last keys (unless no_close_parens is true) and empty the state stack
            void finish(bool no_close_parens = false);
            //! print '{' to start a new object
            /*! if preceded by a value-less object key, will first add a null value
             * \returns true on success */
            bool startObject();
            //! print an object keyname and ':', e.g. <code>"name":</code>
            /*! if preceded by a value-less object key, will first add a null value
             * \returns true on success, false if an object key is not allowed here (e.g. inside an array) */
            bool addKey(const char * name, idx len = 0);
            //! print '}' to end an object
            /*! \returns true on success, false if '}' is not allowed here (e.g. no corresponding '{') */
            bool endObject();
            //! print '[' to start a new array
            /*! if preceded by a value-less object key, will first add a null value.
             * \param force_multiline Always print array elements on different lines (by default, a heuristic
             *                        is used to print arrays of scalar values on one line).
             * \returns true on success, false if '[' is not allowed here (e.g. inside an object, and a key was not provided) */
            bool startArray(bool force_multiline = false);
            //! print ']' to end an array
            /*! \returns true on success, false if ']' is not allowed here (e.g. no corresponding '[') */
            bool endArray();
            //! print null literal
            /*! \returns true on success, false if a null is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addNull();
            //! print string value
            /*! \param empty_as_null if true, and val is empty or NULL, print null JSON literal instead of empty string
             *  \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(const char * val, idx len = 0, bool empty_as_null = false);
            bool addKeyValue(const char * name, const char * val, idx len = 0, bool empty_as_null = false) { return addKey(name) && addValue(val, len, empty_as_null); }
            //! print real value
            /*! \param fmt printf format string ("%g" will be used if 0)
             *  \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(real r, const char * fmt = 0);
            bool addKeyValue(const char * name, real r, const char * fmt = 0) { return addKey(name) && addValue(r, fmt); }
            //! print integer value
            /*! \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(idx i);
            bool addValue(int i) { return addValue((idx)i); }
            bool addKeyValue(const char * name, idx i) { return addKey(name) && addValue(i); }
            bool addKeyValue(const char * name, int i) { return addKey(name) && addValue(i); }
            //! print integer value
            /*! \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(udx u);
            bool addKeyValue(const char * name, udx u) { return addKey(name) && addValue(u); }
            //! print a single character as a string value
            /*! \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(char c);
            bool addKeyValue(const char * name, char c) { return addKey(name) && addValue(c); }
            //! print boolean value
            /*! \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(bool b);
            bool addKeyValue(const char * name, bool b) { return addKey(name) && addValue(b); }
            //! print a hive ID as an integer value if possible (no domain or ION id), or as a string value otherwise
            /*! \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(const sHiveId & id);
            bool addKeyValue(const char * name, const sHiveId & id) { return addKey(name) && addValue(id); }
            //! print a value
            /*! \returns true on success, false if a value is not allowed here (e.g. inside an object, and a key was not provided) */
            bool addValue(const sVariant & val);
            bool addKeyValue(const char * name, const sVariant & val) { return addKey(name) && addValue(val); }
            //! insert raw text which is assumed to be a valid encoded JSON value; use with caution!!
            bool addRaw(const char * txt, idx len = 0);
    };
};

#endif
