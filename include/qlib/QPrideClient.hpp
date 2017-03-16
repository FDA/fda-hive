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
#ifndef _QPrideClient_qLib_hpp
#define _QPrideClient_qLib_hpp

#include <qlib/QPride.hpp>
#include <slib/std/cmdline.hpp>

namespace slib {

    class sQPrideClient: public sQPride
    {

        public:
            // constructor
            static sCmdLine::exeCommand cmdExes[];
            sQPrideClient(const char * defline00 = 0, const char * service = "qm")
                : sQPride(defline00, service)
            {
                outP = 0;
                comma = ",";
                endl = "\n";
                for(idx i = 0; cmdExes[i].param != sNotPtr; ++i)
                    if( cmdExes[i].cmdFun )
                        cmdExes[i].param = (void *) this;
            }
            virtual ~sQPrideClient()
            {
            }

        public:
            virtual void printf(const char * formatDescription, ...) __attribute__((format(printf, 2, 3)));
            virtual idx CmdForm(const char * cmd, sVar * pForm); // to execute a single command based on a form : this implementation is used by QPrideCGI

            static idx reqProgressFSStatic(void * param, idx items)
            {
                sQPrideClient * QP = static_cast<sQPrideClient*>(param);
                if( QP->reqId )
                    QP->reqProgress(QP->reqId,0,items, 0, -1);
                else
                    QP->printf("Processed items: %"DEC,items);
                return 1;
            }
        public:
            sStr * outP;
            const char * comma, *endl;

    };
}

#endif // _QPrideClient_qLib_hpp

