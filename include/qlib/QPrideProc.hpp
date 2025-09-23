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
#ifndef _QPrideProc_qLib_hpp
#define _QPrideProc_qLib_hpp

#include <slib/core/tim.hpp>
#include <slib/std/online.hpp>
#include <slib/std/file.hpp>
#include <slib/std/pipe2.hpp>
#include <qlib/QPride.hpp>
#include <ulib/usr.hpp>
#include <ulib/uproc.hpp>
#include <ulib/uquery.hpp>

namespace slib
{
    class sSql;
    class sIO;

    class sQPrideProc: public sQPride
    {

        public:
            Service svc;
            sTime tmfix,tmExec;

            bool alwaysRun;
            idx loopCnt;
            idx noGrabs;
            idx maxMemSize;
            idx reqSliceId, reqSliceCnt;
            idx prvGrabReqID;

            idx exitCode;
            idx inBundle;
            idx socketSelect,isProblemReported;
            idx doStdin;
            bool tempDBConnection;
            bool maxmemMailSent;
            idx isRegrab;
            idx rand_seed;
            idx sleepTimeOverride;
            sStr requestStage;

            sVec < sUsrProc > objs;

            sPS ps;

            sConIP udp;
            sVar reqForm, * pForm;

         public:
            sQPrideProc(const char * defline00, const char * service = "qm");
            virtual ~sQPrideProc();

            idx run(idx argc, const char *argv[]);
         public:

            enum eQPErr{
                eQPErr_None=0,
                eQPErr_ServiceStopped_None,
                eQPErr_DB_LostConnection,
                eQPErr_ServiceStopped,
                eQPErr_TooManyJobs,
                eQPErr_Max
            } ;

        public:
            virtual idx OnGrab(idx forceReq=0);
            virtual bool OnLogOut(idx typ, const char * message){
                promptOK=sQPrideBase::OnLogOut(typ, message);
                return promptOK;
            }
            virtual bool OnCommand(const char * , const char * ){return false;}


            virtual idx OnExecute(idx ){return 0;}
            virtual bool OnProgress(idx reqId)
            {
                return true;
            }

            virtual idx OnPrepare(idx ) {return 0;}
            virtual idx OnCompute(idx ){ return 0;}
            virtual idx OnComputeThread(idx , ThreadSpecific * ){ return 0;}
            virtual idx OnCleanup(idx){return 0;}

            virtual bool OnInit(void){return true;}
            virtual void OnQuit(void){};

            void psMessage(const char * fmt, ...);

            typedef idx (*splittertFunction)(sVar *, sUsr *, sUsrProc *, const char *, idx);
            sRC genericSplitting(idx &cntParallel, sVar * pForm , sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log = 0);

            virtual sRC OnSplit(idx req, idx &cnt);


            const char * getSplitType(sUsrObj * so, sVar * pForm = 0, sStr * buf = 0);
            const char * getSplitField(sUsrObj * so, sVar * pForm = 0, sStr * buf = 0);
            const char * getSplitSize(sUsrObj * so, sVar * pForm = 0, sStr * buf = 0);
        protected:
            virtual splittertFunction getSplitFunction(const char * type);

        private:
            static const char * listTypes;
            static idx splitFile(sVar * pForm, sUsr * user, sUsrProc * obj, const char * fld_val, idx slice);
            static idx splitList(sVar * pForm, sUsr * user, sUsrProc * obj, const char * fld_val, idx slice);
            static idx splitMultiplier(sVar * pForm, sUsr * user, sUsrProc * obj, const char * fld_val, idx slice);

            idx executeThreads(idx req,idx rangeCnt, idx rangeStart=0);
            bool jobShouldRun(void) ;
            idx selectSleep(idx slpTm=0);
            bool memReport(const idx req, const char * svcName);
        private:
            static void executeRequest(void * procthis);
            static void executeAThread(void * param);

            sRC splitRequest(void);

            bool initializeTriggerPorts(void);
            void releaseTriggerPorts(void);

            bool executeCommand(const char * nam, const char * val);
            idx selectSleepSingle(idx slpTm=0);

            char * _argvBuf;
            int _argvBufLen;
            bool _argvBufChanged;
            static sDic<splittertFunction> _splitFunctionDic;

        public:

            static void registerSplitFunction(const char * name, splittertFunction splitter)
            {
                sQPrideProc::_splitFunctionDic[name] = splitter;
            }
            idx hostNumInPool(Service * svc, idx * pCntList, idx * pmaxjob=0);

            const char * formValue(const char * prop, sStr * buf=0, const char * defaultValue=0, idx iObj=0);
            idx formIValue(const char * prop, idx defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                idx ival=defaultValue;
                if(p){
                    sscanf(p,"%" DEC,&ival);
                }
                return ival;
            }
            udx formUValue(const char * prop, udx defaultValue = 0, idx iObj = 0)
            {
                sStr t;
                const char * p = formValue(prop, &t, 0, iObj);
                udx uval = defaultValue;
                if( p )
                    sscanf(p, "%" UDEC, &uval);
                return uval;
            }
            real formRValue(const char * prop, real defaultValue=0, idx iObj=0){
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                real rval=defaultValue;
                if(p)sscanf(p,"%lf",&rval);
                return rval;
            }
            bool formBoolValue(const char * prop, bool defaultValue=false, idx iObj=0)
            {
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                bool bval=defaultValue;
                if(p) bval=sString::parseBool(p);
                return bval;
            }
            idx formBoolIValue(const char * prop, idx defaultValue=0, idx iObj=0)
            {
                sStr t;
                const char * p=formValue(prop, &t, 0, iObj);
                idx ival=defaultValue;
                if(p) ival=sString::parseIBool(p);
                return ival;
            }

            const char * formValues00(const char *prop, sStr *buf00, const char * altSeparator=0, idx iObj=0);


            idx formIValues(const char *prop, sVec<idx> *values, idx iObj=0);
            idx formUValues(const char *prop, sVec<udx> *values, idx iObj=0);
            idx formHiveIdValues(const char *prop, sVec<sHiveId> & values, idx iObj=0);

            idx formRValues(const char *prop, sVec<real> *values, idx iObj=0);

            const char * destPath(sStr * buf, const char * flnmFmt, ... ) __attribute__((format(printf, 3, 4)));
            const char * reqAddFile(sStr & buf, const char * flnmFmt, ... ) __attribute__((format(printf, 3, 4)));
            const char * reqGetFile(sStr & buf, const char * flnmFmt, ... ) __attribute__((format(printf, 3, 4)));

            idx reqProgress(idx items, idx progress, idx progressMax);
            static idx reqProgressStatic(void * param, idx items, idx progress, idx progressMax);
            static idx reqProgressFSStatic(void * param, idx items);
            static bool reqProgressFSStatic2(idx pid, const char * path, struct stat * st, void * param);

            idx exec(const char * cmdline, const char * input, const char * path, sIO * log, idx sleepSecForExec = sNotIdx);
            sRC exec2(sPipe2::CmdLine & cmdline, const char * path, sIO * log, idx sleepSecForExec = sNotIdx);

            inline bool isGroupId( void )
            {
                return grpId == reqId;
            }
            inline bool isMasterGroupId( void )
            {
                return masterId == reqId;
            }
            bool isLastInGroup(const char * svcName=0 );
            bool isLastInMasterGroup(const char * svcName=0);
            bool isLastInMasterGroupWithLock(void);
            bool isEmailSender(void);

            virtual sUsrQueryEngine * queryEngineFactory(idx flags = 0);

        protected:
            virtual sUsrQueryEngine * queryEngineInit(sUsrQueryEngine * qe);
        private:
            bool lockKey(const char * key);
            bool unlockKey(const char * key);
    };

}

#endif 