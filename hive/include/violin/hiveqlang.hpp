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
#ifndef violin_hiveqlang_hpp
#define violin_hiveqlang_hpp

#include <ulib/uquery.hpp>

namespace slib {
    class sTaxIon;
    namespace qlang {
        class sHiveContext: public sUsrContext
        {

                typedef sUsrContext TParent;

            private:
                sTaxIon * _tax_ion;

                virtual void registerDefaultBuiltins();

            public:
                sHiveContext();
                sHiveContext(sUsr & usr, idx flags = 0);
                virtual ~sHiveContext();
                virtual void init(sUsr & usr, idx flags = 0);
                virtual void reset();

                bool ensureTaxIon();
                sTaxIon* getTaxIon()
                {
                    return _tax_ion;
                }
        };

        class sHiveEngine: public sUsrEngine
        {
                typedef sUsrEngine TParent;

            public:
                sHiveEngine();
                sHiveEngine(sUsr & usr, idx ctx_flags = 0);
                virtual ~sHiveEngine();
                virtual void init(sUsr & usr, idx ctx_flags = 0);
                virtual sHiveContext& getContext()
                {
                    return *static_cast<sHiveContext*>(_ctx);
                }
        };

        class sHiveInternalContext: public sUsrInternalContext
        {

                typedef sUsrInternalContext TParent;

            private:
                sTaxIon * _tax_ion;

                virtual void registerDefaultBuiltins();

            public:
                sHiveInternalContext();
                sHiveInternalContext(sQPrideBase* qp, sUsr & usr, idx flags = 0);
                virtual ~sHiveInternalContext();
                virtual void init(sQPrideBase * qp, sUsr & usr, idx flags = 0);
                virtual void reset();

                bool ensureTaxIon();
                sTaxIon* getTaxIon()
                {
                    return _tax_ion;
                }
        };

        class sHiveInternalEngine: public sUsrInternalEngine
        {
                typedef sUsrInternalEngine TParent;

            public:
                sHiveInternalEngine();
                sHiveInternalEngine(sQPrideBase * qp, sUsr & usr, idx ctx_flags = 0);
                virtual ~sHiveInternalEngine();
                virtual void init(sQPrideBase * qp, sUsr & usr, idx ctx_flags = 0);
                virtual sHiveInternalContext& getContext()
                {
                    return *static_cast<sHiveInternalContext*>(_ctx);
                }
        };
};

    typedef qlang::sHiveContext sHiveQueryContext;
    typedef qlang::sHiveEngine sHiveQueryEngine;
    typedef qlang::sHiveInternalContext sHiveInternalQueryContext;
    typedef qlang::sHiveInternalEngine sHiveInternalQueryEngine;
};
#endif
