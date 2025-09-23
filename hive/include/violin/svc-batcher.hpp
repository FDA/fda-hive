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
#ifndef SvcBatcher_hpp
#define SvcBatcher_hpp

#include <slib/core/mex.hpp>
#include <qlib/QPrideProc.hpp>

using namespace slib;

namespace slib {
    class SvcBatcher: public sQPrideProc
    {
        public:
            SvcBatcher(const char * defline00, const char * srv)
                : sQPrideProc(defline00, srv), submittedGrpIDs(sMex::fSetZero)
            {
                doCreateProcesses = true;
                selfService = true;
                svcToSubmit = 0;
                svcToWaitFor = 0;
                stillRunning=0;
                alreadyDone=0;
                killed=0;
            }
            virtual idx OnExecute(idx);
            void recordSubmittedProcessIDs(idx num);
            const char * prop2readableLog(sStr & out_buf, const char * prop_fmt_log);

            idx stillRunning, alreadyDone, killed;
            bool doCreateProcesses, selfService;
            const char * svcToSubmit;
            const char * svcToWaitFor;
            sVec<idx> submittedGrpIDs;
            sVec<idx> waitedReqs;
            sVec<sHiveId> submittedProcessIDs;

            void clearClass()
            {
                submittedGrpIDs.empty();
                waitedReqs.empty();
                submittedProcessIDs.empty();
                doCreateProcesses = true;
                selfService = true;
                svcToSubmit = 0;
                svcToWaitFor = 0;
            }

        private:
            class BatchingIterator;
            enum EProgressStage {
                eCreatedForm = 0,
                eCreatedProcess,
                eCreatedSubmission,
                eStartedRequest,
                eDoneBatching
            };
            virtual idx batchReqProgress(BatchingIterator * batch_iter, EProgressStage stage);
    };
};


#endif

