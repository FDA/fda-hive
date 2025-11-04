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
#include <qlib/QPrideProc.hpp>
#include <slib/std/app.hpp>

#include <qlib/QPProcessHandler.hpp>

using namespace slib;

class CFlowProc: public sQPrideProc, public sQPProcessHandler
{
    public:
        CFlowProc(const char * defline00, const char * srv): sQPrideProc(defline00, srv){}
        CFlowProc(const char * srv) : sQPrideProc(0, QPrideSrvName(0, srv, sApp::argv[0]) ) {}
        ~CFlowProc(){}
        virtual idx OnReleaseRequest(idx ){
            sQPProcessHandler::setFinalStatus();
            sQPProcessHandler::serialize();
            sQPProcessHandler::destroy();
            return 0;
        }
        virtual idx OnRunFlow(idx req){return 0;}
        virtual idx OnExecute(idx req){return OnRunFlow(req);};
};

#define CFLOW_START(_V_WORKFLOW_NAME, _V_SRV_NAME) \
    class _V_WORKFLOW_NAME##_CFlowProc: public CFlowProc \
    { \
        public: \
        _V_WORKFLOW_NAME##_CFlowProc(const char * srv): CFlowProc(srv){} \
        idx OnRunFlow(idx req); \
    }; \
    int main(int argc, const char * argv[]) \
    { \
        sApp::args(argc, argv); \
        _V_WORKFLOW_NAME##_CFlowProc backend("+" _V_SRV_NAME); \
        return (int) backend.run(argc, argv); \
    } \
    idx _V_WORKFLOW_NAME##_CFlowProc::OnRunFlow(idx req) \




#define CFLOW_STOP() ;


