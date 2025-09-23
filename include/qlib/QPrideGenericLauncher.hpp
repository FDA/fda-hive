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
#ifndef _QPrideGenericLauncher_qLib_hpp
#define _QPrideGenericLauncher_qLib_hpp

#include <qlib/QPrideProc.hpp>
#include <ulib/uquery.hpp>
#include <ulib/ufile.hpp>

namespace slib
{
    using namespace qlang;
    class sQPrideGenericLauncher: public sQPrideProc
    {
        public:
            sStr cmdLineTemplate;
            sStr cmdLineTemplateFile;
            sStr launcherDir;
            sStr regExpResultList00;
            sStr serviceToRegister;
            idx defaultMemUlimit;
            bool deleteWhenFinish;
            const char * addDispatchedFunctions00;

            sUsrInternalQueryEngine *sQLan;

            sQPrideGenericLauncher(const char * defline00, const char * srv)
                : sQPrideProc(defline00, srv)
            {
                    sQLan=0;
                    deleteWhenFinish = true;
                    addDispatchedFunctions00=0;
                    defaultMemUlimit=0;
                    init();
            }

            virtual idx OnReleaseRequest(idx req)
            {
                cmdLineTemplate.cut(0);
                cmdLineTemplateFile.cut(0);
                launcherDir.cut(0);
                regExpResultList00.cut(0);
                serviceToRegister.cut(0);
                return 0;
            }
            virtual idx OnExecute(idx);

            void registerCallbackFunctions(const char * addFunc00) {
                for(const char * p = addFunc00; p; p = sString::next00(p)) {
                    sQLan->registerBuiltinFunction(p, dispatcher_callbackStatic, (void*) this);
                }
            }

            virtual bool dispatcher_callback(sVariant &result, const qlang::BuiltinFunction &funcObj, qlang::Context &ctx, sVariant *topic, sVariant *args, idx nargs, void *param)
                {return sUsrInternalContext::dispatcher_callback(result, funcObj, ctx, topic, args, nargs, param);}

            static bool dispatcher_callbackStatic(sVariant &result, const qlang::BuiltinFunction &funcObj, qlang::Context &ctx, sVariant *topic, sVariant *args, idx nargs, void *param)
                {return ((sQPrideGenericLauncher* )param)->dispatcher_callback(result, funcObj, ctx, topic, args, nargs, param);}

            virtual idx prepareForLaunch(sStr * cmdline,sStr * errorMsg, idx * actually_inside_prepareForLaunch=0)
            {
                return 0;
            }
    };

}

#endif
