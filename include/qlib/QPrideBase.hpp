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
#ifndef _QPrideBase_qLib_hpp
#define _QPrideBase_qLib_hpp

#include <slib/core/def.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/tim.hpp>
#include <slib/std/varset.hpp>

namespace slib
{

    class sQPrideConnection;
    class sUsr;

    class sQPrideBase
    {

         public:
            sQPrideBase(sQPrideConnection * connection=0, const char * service="qm");
            sQPrideBase * init(sQPrideConnection * connection=0, const char * service="qm");
            virtual ~sQPrideBase();
        private :
            sQPrideConnection * QPDB;// underlying low level dB pointer hidden from class users
        public:
            idx svcID;
            sUsr * user;
            idx inDomain;
            idx jobId, pid;
            idx reqId, grpId, masterId;
            bool promptOK;
            sVar vars;
            idx qmBaseUDPPort;
            idx thisHostNumInDomain;
            bool isSubNetworkHead;
            bool isDomainFound;
            idx ok;
            sTime lazyTime,tmCount;
            sStr lastErr;


            idx largeDataReposSize;


            struct QPCfg{
               idx cleanUpDays;
               idx permID;
               sStr nameMasks;
               sStr val;
            };

            struct Service {

                Service() {
                    memset(this, 0, sizeof(*this));
                }

                idx svcID;
                idx permID;
                idx svcType;
                idx knockoutSec;
                idx maxJobs;
                idx nice;
                idx sleepTime;
                idx maxLoops;
                idx parallelJobs;
                idx delayLaunchSec;
                idx politeExitTimeoutSec;
                idx maxTrials;
                idx restartSec;
                idx priority;
                idx cleanUpDays;
                idx runInMT;
                idx noGrabDisconnect;
                idx noGrabExit;
                idx lazyReportSec;
                idx isUp;
                idx activeJobReserve;
                idx maxmemSoft;
                idx maxmemHard;
                idx cdate;
                char name[128];
                char title[128];
                char cmdLine[1024];
                char hosts[4096];
                char emails[256];
                char categories[256];
                idx hasReqToGrab;
                real capacity;
            };


            struct Request
            {
                idx reqID;
                idx svcID;
                idx jobID;
                idx userID;
                idx subIp;
                idx cgiIp;
                idx stat;
                idx act;
                idx takenCnt;
                idx priority;
                idx inParallel;
                idx progress;
                idx progress100;
                idx userKey;
                idx takenTm;
                idx cdate;
                idx actTm;
                idx aliveTm;
                idx doneTm;
                idx purgeTm;
                idx grpID;
                char svcName [128];
                idx scheduleGrab;
            };

            struct Job
            {
                idx jobID;
                idx svcID;
                idx userID;
                idx pid;
                idx reqID;
                idx stat;
                idx act;
                idx cntGrabbed;
                idx inParallel;
                idx cdate;
                idx aliveTm;
                idx actTm;
                idx psTm;
                idx mem;
                idx maxmem;
                idx killCnt;
                char hostName [1024];
            };

            enum eQPApp {
                eQPApp_DB      = 0x00000001,
                eQPApp_Console = 0x00000002,
            };
            idx appMode;

            class QPLogMessage {
                public:
                    QPLogMessage();
                    ~QPLogMessage();
                    void init(idx req, idx job, idx level, idx cdate, const char * txt);
                    const char * message(void);
                    idx req;
                    idx job;
                    idx level;
                    idx cdate;
                private:
                    idx txt_idx;
                    const char * txt;
            };

            // system logging
            enum eQPLogType
            {
                eQPLogType_Min = 1,
                eQPLogType_Trace = eQPLogType_Min,
                eQPLogType_Debug,
                eQPLogType_Info,
                eQPLogType_Warning,
                eQPLogType_Error,
                eQPLogType_Fatal,
                eQPLogType_Max = eQPLogType_Fatal
            };
            // user visible logging
            enum eQPInfoLevel
            {
                eQPInfoLevel_Min = 100,
                eQPInfoLevel_Trace = eQPInfoLevel_Min,
                eQPInfoLevel_Debug = 200,
                eQPInfoLevel_Info = 300,
                eQPInfoLevel_Warning = 400,
                eQPInfoLevel_Error = 500,
                eQPInfoLevel_Fatal = 600,
                eQPInfoLevel_Max = eQPInfoLevel_Fatal,
            };
            // works for both eQPLogType and eQPInfoLevel
            static const char * const getLevelName(idx level);

            enum eQPReqStatus
            { // CURRENT STATUS FOR THE REQUEST
              eQPReqStatus_Any=0,  // if got this return value, check message in log
              eQPReqStatus_Waiting,    // waiting to be taken for processing
              eQPReqStatus_Processing, // job was taken from req-queue but not yet running
              eQPReqStatus_Running,    // job was submitted for running
              eQPReqStatus_Suspended,  // job was suspended by user or by sysyem
              eQPReqStatus_Done,       // job has finished
              eQPReqStatus_Killed,     // job was killed upon user request
              eQPReqStatus_ProgError,  // job executeable reported an error
              eQPReqStatus_SysError,   // system error, infrastructure has had a problem
              // update isStaleLock() in QPrideProc.cpp when adding new stopped/error states
              eQPReqStatus_Max
            };
            enum eQPReqAction{ // CURRENT ACTION FOR THE REQUEST
                eQPReqAction_Any=0,
                eQPReqAction_None,      // job on hold (not to execute) until flag changed
                eQPReqAction_Run,       // job handler should execute this job
                eQPReqAction_Kill,      // job handler should kill this job
                eQPReqAction_Suspend,   // job handler should suspend this job
                eQPReqAction_Resume,    // job handler should resume previously suspended job
                eQPReqAction_Max
            } ;

            enum eQPReqPar{ // Types of User specified values for request
                eQPReqPar_None=0,
                eQPReqPar_Objects,             // User name for this request
                eQPReqPar_Name,              // User name for this request
                eQPReqPar_Max=1000
            } ;

            enum eQPJobAction {
                eQPJobAction_Any=0,
                eQPJobAction_Run,
                eQPJobAction_Kill,
                eQPJobAction_Max
            };
            enum eQPJobStatus {
                eQPJobStatus_Any=0,
                eQPJobStatus_Running,
                eQPJobStatus_ExitNormal,
                eQPJobStatus_ExitError,
                eQPJobStatus_Knockouted,
                eQPJobStatus_Terminated,
                eQPJobStatus_Killed,
                eQPJobStatus_Max

            };

            enum eQPIPStatus{
                eQPIPStatus_Any=0,
                eQPIPStatus_Blocked,
                eQPIPStatus_Deprioritized,
                eQPIPStatus_Max
            } ;



        public: // Logging, messaging, emailing

            // Diagnositcs message functionality
            void logOut(eQPLogType level, const char * formatDescription , ...) __attribute__((format(printf, 3, 4)));
            void vlogOut(eQPLogType level, const char * formatDescription, va_list ap);

            //! if logging < 0 prints to stdout also abs(logging) determines min level of msg going to QPLog table
            idx setupLog(bool force = false, idx force_level = 0);

            idx messageSubmit(const char * server, idx svcId, bool isSingular, const char * fmt, ...) __attribute__((format(printf, 5, 6)));
            idx messageSubmit(const char * server, const char * service,bool isSingular, const char * fmt, ...    ) __attribute__((format(printf, 5, 6)));
            idx messageSubmitToDomainHeader(const char * fmt, ...) __attribute__((format(printf, 2, 3)));
            idx messageSubmitToDomain(const char * service,const char * fmt,...) __attribute__((format(printf, 3, 4)));
            idx messageWakePulljob(const char * service);

        public: // Config
            char * configGet(sStr * str , sVar * pForm, const char * par, const char * defval, const char * fmt, ... ) __attribute__((format(scanf, 6, 7)));
            char * configGetAll( sStr * vals00, const char * pars00);
            bool configSet(const char * par, const char * fmt, ... ) __attribute__((format(printf, 3, 4)));
            void flushCache();

            char * cfgStr(sStr * buf, sVar * pForm, const char * par, const char * defval=0 ){return configGet(buf,pForm, par,defval,0);}
            idx cfgInt(sVar * pForm, const char * par, idx defval=0){idx res=defval; sStr tmp, oo("%"DEC,defval);  configGet(&tmp,pForm, par, oo.ptr(), "%"DEC,&res);return res;}
            real cfgReal(sVar * pForm, const char * par, real defval=0){real res=defval; sStr tmp, oo("%lf",defval); configGet(&tmp,pForm, par, oo.ptr(), "%lf",&res);return res;}
            char * cfgPath(sStr & path, sVar * pForm, const char * flnm, const char * par)
            {
                if( flnm && *flnm != '/' && *flnm != '\\' ) {
                    path.cut(0);
                    cfgStr(&path, pForm, par, 0);
                    path.printf("%s", flnm);
                    flnm = path;
                }
                return sNonConst(flnm);
            }

            sStr parFind00, parRepl00;
            void makeVar00(void);
            char * replVar00(sStr * str, const char * src, idx len=0);

        public: // Service
            idx serviceID(const char * serviceName=0);
            idx serviceGet(Service * svc=0,const char * serviceName=0, idx svcId=0);
            idx serviceUp(const char * svc, idx mask);
            idx serviceList(sStr * lst00=0, sVec < Service > * svclist=0) ;

            void servicePurgeOld(sVec<idx> * reqList, const char * service = 0, idx limit = -1, bool no_delete=false);
            idx servicePath2Clean(sVarSet & res);
            void registerHostIP();
            void getRegisteredIP(sVec <sStr> * ips, const char * equCmd);
            real getHostCapacity();
            idx workRegisterTime(const char * svc="", const char * params="", idx amount=0, idx time=0);
            idx workEstimateTime(const char * svc="", const char * params="", idx amount=0);
        public: // Submission
            idx reqSubmit(const char * serviceName=0, idx subip=0, idx priority=0, idx usrid=0);
            idx grpSubmit(const char * serviceName=0, idx subip=0, idx priority=0, idx numSubReqs=0, idx usrid=0, idx previousGrpSubmitCounter=0);
            idx reqReSubmit(idx req, idx delaySeconds = 0);
            idx grpReSubmit(idx grp, const char * serviceName, idx delaySeconds = 0, idx excludeReq = 0);
            idx reqCache( const char * serviceName= "qmcache");

            idx reqGrab(const char * service, idx job, idx inBundle=0,idx status=eQPReqStatus_Waiting, idx action=eQPReqAction_Run);

        public: // Data handling
            bool reqAuthorized(idx req);
            bool reqAuthorized(const Request & R);
            void reqCleanTbl(idx req, const char * dataname);

            bool reqSetPar(idx req, idx type, const char * value,bool isOverwrite=false);
            bool reqSetData(idx req, const char * dataName, idx datasize, const void * data);
            bool reqSetData(idx req, const char * dataName, const char * fmt, ... ) __attribute__((format(printf, 4, 5)));
            bool reqSetData(idx req, const char * blobName, const sMex * mex);
            //! If dataName was stored as a file which is too small, move it to DB
            bool reqRepackData(idx req, const char * dataName);

            char * reqGetData(idx req, const char * dataName, sMex * data, bool emptyold=false, idx * timestamp=0);
            char * reqUseData(idx req, const char * dataName, sMex * data, idx openmode=sMex::fReadonly); // map the existing data
            char * reqDataPath(idx req, const char * dataName, sStr * path);
            idx reqDataTimestamp(idx req, const char * dataName);
            char * grpDataPaths(idx grp, const char * dataName, sStr * path, const char * svc=0, const char * separator=" ");
            idx * grpDataTimestamps(idx grp, const char * dataName, sVec < idx > * timestampVec);

            idx grpGetData( idx grp , const char * blobName, sVec < sStr > * dat, sVec < idx > * timestamps=0 );
            template <class Tobj> idx grpGetData( idx grp , const char * blobName, sVec < sVec<Tobj> > * dat, sVec < idx > * timestamps=0)
            {
                sVec < idx > reqs;grp2Req(grp, &reqs) ;

                if(!reqs.dim()) {
                    dat->resize(1);
                    reqGetData(grp, blobName, (*dat)[0].mex(), true, timestamps ? timestamps->ptrx(0) : 0);
                    return 1;
                }
                dat->resize(reqs.dim());
                if( timestamps )
                    timestamps->resize(reqs.dim());

                for( idx i=0; i<reqs.dim(); ++i)
                        reqGetData(reqs[i], blobName, (*dat)[i].mex(), true, timestamps ? timestamps->ptr(i) : 0);
                return reqs.dim();
            }

            idx dataGetAll(idx req, sVec <sStr> * dataVec, sStr * infos00, sVec <idx> * timestamps=0);
            idx grpGetData( idx grp , const char * blobName , sMex * data, bool emptyold=false, const char * separ = 0, idx * timestamp = 0);

            bool reqSetData(idx req, const char * blobName, sVar * pForm){sStr txt;pForm->serialOut(&txt);return reqSetData(req,blobName,txt.mex());}
            sVar * reqGetData(idx req, const char * dataName, sVar * pForm) {sStr txt;reqGetData(req, dataName, txt.mex());pForm->serialIn(txt.ptr(), txt.length());return pForm;}



        public: // Progress and Status

            idx requestGet(idx req, Request * r) ;
            idx requestGetForGrp(idx grp, sVec< sQPrideBase::Request > * r, const char * serviceName = 0);
            char * requestGetPar(idx req, idx type, sStr * val) ;

            //!
             /* get request infos for the whole group with severity level >= level
             *
             *  \return number of messages add dto log
             */
            idx reqGetInfo(idx req, idx level, sVec<QPLogMessage> & log);
            idx grpGetInfo(idx grp, idx level, sVec<QPLogMessage> & log);
            idx getLog(idx req, bool isGrp, idx job, idx level, sVec<QPLogMessage> & log);
            bool vreqSetInfo(idx req, idx level, const char * formatDescription, va_list ap);
            bool reqSetInfo(idx req, idx level, const char * fmt, ...) __attribute__((format(printf, 4, 5)));

            idx grpGetStatus( idx grp , sVec < idx > * stat, const char * svcname=0, idx masterGroup=0);
            enum EGroupStatusCmp {
                eStatusNotEqual = 0,
                eStatusEqual = 1,
                eStatusLessThan = -1,
                eStatusGreaterThan = 2
            };
            idx getGroupCntStatusIs(idx grp, idx whatstat, sVec<idx> * stat, EGroupStatusCmp statusis=eStatusEqual, const char * svcname=0, idx masterGroup=0);
            idx reqSetAction(idx req, idx action);
            idx reqSetAction(sVec <idx> * reqs, idx action);
            idx reqGetAction(idx req);

            idx reqGetUser(idx req);
            /*idx reqSetObject(idx req, idx status);
            idx reqGetObject(idx req);
            */
            idx reqSetStatus(idx req, idx status);
            idx reqSetStatus(sVec <idx> * reqs, idx status);
            idx reqGetStatus(idx req);
            idx reqGetStatus(sVec <idx> * reqs, sVec < idx  > * status);

            idx reqSetUserKey(idx req, idx key);
            idx reqSetUserKey(sVec <idx> * reqs, idx key);
            idx reqGetUserKey(idx key);
            idx getReqByUserKey(idx userKey, const char * serviceName=0);

            //! Attempt to register a lock on key for the given request
            /*! \param req request ID if positive, or other private-use handle if negative, or this->reqId if zero
             *  \param key name of entity to lock, typically a filesystem path
             *  \param[out] req_locked_by request currently holding the lock
             *  \param max_lifetime max duration of lock in seconds; 48 hours by default; negative or zero values
             *                      are invalid, and might be reset to some arbitrary positive duration.
             *  \param force always overwrite existing lock (even if belonging to another living request)
             *  \returns true if lock was taken */
            bool reqLock(idx req, const char * key, idx * req_locked_by=0, idx max_lifetime=48*60*60, bool force=false);
            bool reqLock(const char * key, idx * req_locked_by=0, idx max_lifetime=48*60*60)
            {
                return reqLock(reqId, key, req_locked_by, max_lifetime, false);
            }
            //! Attempt to remove lock on key for the given request
            /*! \param req request ID if positive, or other private-use handle if negative, or this->reqId if zero
             *  \param key name of entity to unlock, typically a filesystem path
             *  \param force always remove existing lock (even if belonging to another living request)
             *  \returns true if lock was removed */
            bool reqUnlock(idx req, const char * key, bool force=false);
            bool reqUnlock(const char * key)
            {
                return reqUnlock(reqId, key, false);
            }

            //! Find request (if any) currently holding the specified lock
            /*! \param key name of entity to lock, typically a filesystem path
                \returns request ID currently holding the lock, or 0 if none */
            idx reqCheckLock(const char * key);

            void killReq(idx req);
            void killGrp(idx req);
            void purgeReq(idx req);

            // Progress reporting functions
            idx progress100Start; //!< 0 by default; reqProgress() scales progress percentage to be in  progress100Start..progress100End range
            idx progress100End; //!< 100 by default; reqProgress() scales progress percentage to be in  progress100Start..progress100End range

            //! Report a request as alive and update its progress level
            /*!
                \param req request ID
                \param minReportFrequency the db will be updated only if more than this number of seconds passed since the last reqProgress() call
                \param items number of "items" (bytes, rows, records, ...) processed, or -1 to ignore; values less than -1 means ignore minReportFrequency and force record abs(items) in db
                \param progress current raw progress level; sNotIdx means \a items parameter will be used as raw progress level
                \param progressMax maximum raw progress level; negative value means progress level will not be updated in db
                \returns 1 normally, or 0 if the request needs to be stopped
                \note
                The raw progress level is not directly recorded in db.
                \note
                The value of \a items is recorded in the 'progress' column in db.
                \note
                The progress percentage (0-100) is recorded in the 'progress100' column in db; it is calculated as
                progress100Start + (progress / progressMax) * (progress100End - progress100Start)
            */
            idx reqProgress(idx req, idx minReportFrequency, idx items, idx progress, idx progressMax);
            //! static version of sQPrideBase::reqProgress
            /*! \param param sQPrideBase pointer on which to call reqProgress(), or 0 for no-op */
            static idx reqProgressStatic(void * param, idx req, idx minReportFrequency, idx items, idx progress, idx progressMax);

            //! Old fashioned direct update, careful it will reset BOTH counters, unless progress is < 0 or progress100 is < 0 or > 100
            /*
                Provide negative value for the other counter if you want to change only one of them
             */
            idx reqSetProgress(idx req, idx progress, idx progress100);
            idx grpGetProgress(idx grp, idx * progress, idx * progress100);
        protected:
            //! could be reset to 0 to allow drop in percentage reported
            idx progress100Last;
            idx progress2Percent(idx items, idx progress, idx progressMax)
            {
                if( progress == sNotIdx ) {
                    progress = items;
                }
                return progressMax > 0 ? progress100Start + progress * ((real)(progress100End - progress100Start) / progressMax) : -1;
            }

        public: // Grouping
            idx grpAssignReqID(idx req, idx grp, idx jobIDSerial=0) ;
            idx req2Grp(idx req, sVec<idx> * grpIds=0, bool isMaster=false);
            idx req2GrpSerial(idx req, idx grp, idx * pcnt=0, idx svc=sNotIdx);
            idx grp2Req(idx grp, sVec<idx> * reqIds=0, const char * svc=0, idx masterGroup=0, bool returnSelf=true);



        public: // jobs
            //! Register job and/or request as alive in the database in a rate-limited way
            /*! \param req if non-0, request id whose alive time needs to be updated (assuming isReadonly is false and notMoreFrequentlySec had passed)
             *  \param job if non-0, job id whose alive time needs to be updated (assuming isReadonly is false and notMoreFrequentlySec had passed)
             *  \param notMoreFrequentlySec minimum time interval in seconds for database updates
             *  \param isReadonly if true, don't update the database or modify sQPrideBase's timers
             *  \returns true if >= notMoreFrequentlySec had passed since previous database update */
            bool jobRegisterAlive(idx job, idx req, idx notMoreFrequentlySec, bool isReadonly=false);
            idx jobSetStatus(idx job, idx jobstat);
            idx jobSetStatus(sVec < idx > *  jobs, idx jobstat);
            idx jobSetAction(sVec < idx > *  jobs, idx jobact);
            idx jobRegister(const char * serviceName, const char * hostName, idx pid, idx inParallel);
            idx jobSetMem(idx job, idx curMemSize, idx maxMemSize);
            idx jobGetAction(idx jobId);
            // interconnect job with request
            idx jobSetReq(idx job, idx req);

        public: // db
            idx dbHasLiveConnection(void);
            void dbDisconnect(void);
            idx dbReconnect(void);

        public: // overloadables
            virtual bool OnLogOut(idx , const char * message);
            virtual bool OnInit(void)
                {
                return true;
                }
            virtual void OnQuit(void)
                {}
            public:
                static char * QPrideSrvName(sStr * buf, const char * srvbase, const char * progname);

        public: // for parralellization
            idx iJobArrStart, iJobArrEnd;
            idx threadID,threadsWorking,threadsCnt;
            idx parGetThreadID(void){return threadID++;}

            struct ThreadSpecific {
                sQPrideBase * thisObj;
                idx iThreadNum, isThreadDone;
                idx progressDone, iThreadStart, iThreadEnd;

                ThreadSpecific () {
                    thisObj=0;
                    iThreadNum=0;
                    progressDone=0;
                    iThreadStart=0;
                    iThreadEnd=0;
                    isThreadDone=0;
                }

            };
            sVec < ThreadSpecific > thrSpc;

            static idx parSplit(idx globalStart, idx globalEnd, idx parallelJobs, idx parallelThreads, idx myJobNum, idx myThreadNum, idx * myStart, idx * myEnd);


            ThreadSpecific * parSplitForThread(idx globalStart, idx globalEnd, idx myJobNum, idx myThreadID, Service * svc);
            idx parSplitForJob(idx globalStart, idx globalEnd, idx myJobNum, Service * svc);
            idx parProgressReport(idx req, const char * blbname="progress.qpride") ;
            idx parReqsStillIncomplete(idx grp, const char * svcOnly=0) ;
            idx parReportingThread(void);


        public: // resource comands
            char * resourceGet(const char * service, const char * resName,    sMex * data, idx * timestamp);
            bool resourceSet(const char * service, const char * resName, idx ressize , const void * data);
            idx resourceGetAll(const char * service, sStr * infos00, sVec < sStr > * dataVec,   sVec < idx > * tmStmps  );
            bool resourceDel(const char * service, const char * resName = 0);
            idx resourceSync(const char * resourceRoot, const char * service, const char * platformSpec = 0);

        public: // system operations
            idx sysPeekReqOrder(idx req, const char * srv=0, idx * pRunning=0 );

            idx sysPeekOnHost(sVec < Service > * srvlist,  const char * hostname=0 );
            idx sysGetKnockoutJobs(sVec < Job > * jobs, sVec < idx > * svcIDs=0);
            idx sysGetImpoliteJobs(sVec < Job > * jobs, sVec < idx > * svcIDs=0);
            idx sysJobsGetList(sVec < Job > * jobs , idx stat, idx act, const char * hostname=0 );
            idx sysRecoverRequests(sVec <idx > * killed=0, sVec < idx > * recovered=0);
            idx sysCapacityNeed(idx * capacity_total = 0);

            bool hostNameMatch (const char * rp, const char * hostName, idx * pmaxjob=0);
            bool hostNameListMatch (const char * rp, const char * hostName, idx * pmaxjob=0, idx * pwhich=0);

        public: //
            idx reqProcSubmit(idx cntParallel,sVar * pForm, const char * svc , idx grp, idx autoAction = sQPrideBase::eQPReqAction_Run, bool requiresGroupSubmission = false, idx priority=0, idx previousGrpSubmitCounter=0); //=eQPReqAction_None
            idx reqProgressReport(sStr * group, sStr * list, idx grp, idx start, idx cnt, idx minLevelInfo = eQPInfoLevel_Min, const char * svcName = 0, sDic<idx> * svcName00 = 0);
    };
}

#endif // _QPrideBase_qLib_hpp



