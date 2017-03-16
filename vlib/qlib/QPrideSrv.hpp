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
#ifndef _QPrideSrv_qLib_hpp
#define _QPrideSrv_qLib_hpp

#include <qlib/QPrideProc.hpp>

#include <slib/std.hpp>

namespace slib {
    class sQPrideSrv: public sQPrideProc
    {

        public:
            sQPrideSrv(const char * defline00, const char * service = "qm");
            virtual ~sQPrideSrv()
            {
            }

        public:
            virtual idx OnGrab(idx forceReq = 0);
            virtual bool OnInit(void);
            virtual void OnQuit(void);
            virtual bool OnCommand(const char * command, const char * value);
            virtual idx OnMaintain(void);

            virtual bool OnCommandKnockout(const char * command, const char *value);
            virtual bool OnCommandKillImpolites(const char * command, const char *value);
            virtual bool OnCommandCleanTerminated(const char * command, const char *value);
            virtual bool OnCommandStopJobs(const char * command, const char *value);
            virtual bool OnCommandLaunch(const char * command, const char *value);
            virtual bool OnCommandSoundWake(const char * command, const char *value);
            virtual bool OnCommandRecover(const char * command, const char *value);
            virtual bool OnCommandPurge(const char * command, const char *value);
            virtual bool OnCommandRegister(const char * command, const char *value);
            virtual bool OnCommandEmail(const char * command, const char *value);
            virtual bool OnCommandCapacity(const char * command, const char *value);

        protected:
            idx __findServiceForJob(Job * jobSvcID);

            struct TPurgeData
            {
                    const char * path00;
                    idx cleanUpDays;
                    const char * masks00;
                    sDic<idx> reqs;
                    sDic<udx> objs;
                    udx size;
                    TPurgeData()
                        : path00(0), cleanUpDays(0), masks00(0), size(0)
                    {
                    }
            };

            virtual bool init(void);
            virtual void purge(TPurgeData & data);
            void purgeDir(TPurgeData & data);

            virtual idx doKillProcesses(idx * ppids, idx cnt = 1);

        protected:
            sVec<Service> svcList;
            real usedCapacity;
            sVec<sVec<sPS::Stat> > svcPSList;
            sVec < idx > svcJobsAlive;
            sVec<idx> svcIDs;
            idx m_secsToPurge;
            bool m_isMaintainer;
            udx emailBatch;
            sTime m_maintainTimeLast;
            bool m_isBroadcaster;
            sStr m_nodeMan;
    };

}

#endif // _QPrideSrv_qLib_hpp
