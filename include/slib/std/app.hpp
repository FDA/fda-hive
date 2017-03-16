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
#ifndef Vstd_app_hpp
#define Vstd_app_hpp

#include <slib/core/var.hpp>
#include <slib/std/string.hpp>

namespace slib {

    class sApp
    {
        public:
            static sStr gCfg;
            static sStr * cfg;

            static sVar gVar;
            static sVar * var;

            static idx argc;
            static const char ** argv;
            static const char ** envp;

            // could be file offset, line number, record number, etc.
            static udx err_location;

            sApp(int largc = 0, const char ** largv = 0, const char ** lenvp = 0);
            ~sApp(void);

            static void args(int largc = 0, const char ** largv = 0, const char ** lenvp = 0)
            {
                argc = largc;
                argv = largv;
                envp = lenvp;
            }
            static void cfgset(const char * section, const char * name, const char * value = 0)
            {
                sStr nm("%s.%s", section, name);
                var->inp(nm.ptr(), value);
            }

            static char * cfgget(const char * section, const char * name, const char * defVal = 0);
            template<class Tobj> static Tobj * cfgscan(const char * section, const char * name, const char * frmt, Tobj * valPtr)
            {
                char * val = cfgget(section, name);
                if( !val )
                    return 0;
                if( !sscanf(val, frmt, valPtr) )
                    return 0;
                return valPtr;
            }
            static idx cfgsave(const char * section00, bool truncate);
    };

}  // end namespace slib

#endif // Vstd_app_hpp
