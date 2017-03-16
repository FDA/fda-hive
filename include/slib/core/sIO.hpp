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
#ifndef sLib_core_sIO_hpp
#define sLib_core_sIO_hpp

#include <slib/core/str.hpp>

namespace slib {

    class sIO: public sStr
    {
        public:

            typedef void * (*callbackFun)(sIO * str, void * param, char * cont);

            callbackFun _funcCallback;
            callbackFun _funcAccumulator;
            void * _funcCallbackParam;

            // construction
            sIO(idx flags = sMex::fBlockDoubling, callbackFun funcCallback = 0, void * funcCallbackParam = 0, callbackFun funcAccumulator = 0)
                : sStr(flags)
            {
                _funcCallback = funcCallback;
                _funcCallbackParam = funcCallbackParam;
                _funcAccumulator = funcAccumulator;
            }
            enum bitFlags
            {

                fDualOutput = sMex::fLast<<1,
                fNoAutoCallback= sMex::fLast<<2,
                fLast = sMex::fLast<<3
            // upper sixteen bits are for filehandles
            };

        public:
            void callback(char * cont);

            char * add(const char * addb, idx sizeAdd = 0)
            {
                char * cont = sStr::add(addb, sizeAdd);
                if((flags&fNoAutoCallback)==0)callback(cont);
                return cont;
            }
            char * addString(const char * addb, idx sizeAdd=0)
            {
                char * cont = sStr::addString(addb, sizeAdd);
                if((flags&fNoAutoCallback)==0)callback(cont);
                return cont;
            }
            char * vprintf(const char * formatDescription, va_list marker)
            {
                char * cont = sStr::vprintf(formatDescription, marker);
                if((flags&fNoAutoCallback)==0)callback(cont);
                return cont;
            }
            char * printf(idx len, const char * formatDescription, ...)
            {
                cut(len);
                char * ret;
                sCallVargRes(ret, vprintf, formatDescription);
                return ret;
            }
            char * printf(const char * formatDescription, ...)
            {
                char * ret;
                sCallVargRes(ret, vprintf, formatDescription);
                return ret;
            }

            //char * readIO(FILE * fp, idx chunks);

        public:
            /*
             inline void _sOutf( void ) {
             _sOutDw;
             sStr::_sOutf();
             sOut((sCallbackUniversal)_funcCallback);
             sOut((sCallbackUniversal)_funcAccumulator);
             sOut(_funcCallbackParam);
             _sOutUp;
             }
             */
    };
}
#endif // sLib_core_sIO_hpp
