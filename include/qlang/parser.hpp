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
#ifndef sLib_qlang_parser_h
#define sLib_qlang_parser_h

#include <qlang/interpreter.hpp>
#include <qlang/ast.hpp>

namespace slib {
    namespace qlang {
        class Parser {
        public:
            enum eParserFlags {
                fTemplate = 1,
                fDollarValues = 1<<2
            };

        protected:
            sStr _error;
            ast::Node *_astRoot;
            const char *_buf, *_bufcur, *_bufend;
            void *_ppriv;
            idx _flags;

        public:
            bool debugLexer;
            bool debugParser;

            Parser(): _astRoot(NULL), _buf(NULL), _bufcur(NULL), _bufend(NULL), _ppriv(NULL), _flags(0), debugLexer(false), debugParser(false) {}
            virtual ~Parser() { delete _astRoot; }

            idx yyInput(char *lexerBuf, size_t size);
            void yyPushState(int new_state=0);
            void yyPopState();
            void yyPopStateUntilTemplate();

            bool parse(const char *buf, const char *bufend, idx flags=0);
            bool parse(const sStr &s, idx flags=0);

            void setError(const char *s, idx line=-1, idx col=-1);
            void printError(sStr &s);
            const char * getErrorStr();

            void setAstRoot(ast::Node *node) { _astRoot = node; }
            ast::Node* getAstRoot() { return _astRoot; }
            ast::Node* releaseAstRoot()
            {
                ast::Node* tmp = _astRoot;
                _astRoot = NULL;
                return tmp;
            }

            idx getFlags() const { return _flags; }
        };
    };
};

#endif
