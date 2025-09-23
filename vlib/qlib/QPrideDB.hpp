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
#ifndef _QPrideDB_qLib_hpp
#define _QPrideDB_qLib_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>

#include "QPrideConnection.hpp"

namespace slib
{
    class sSql;

    class sQPrideDB: public sQPrideConnection
    {
        private:
            sSql * db, * dbdel;

        public:

            sQPrideDB(sSql * lsql);
            sQPrideDB(const char *defline);
            ~sQPrideDB();

            sSql * sql(void){return db;}

        public:
            static char * _QP_whereChoice(sSql * db, sStr * str, const char * choice00, bool single=true, bool quoted=true, const char * wherepar="par");
            static idx _QP_GetIdxVar(sSql * db, idx req, const char * tblname,const char * idxname, const char * vname);
            static idx _QP_SetIdxVar(sSql * db, idx req, idx val, const char * tblname, const char * idxname, const char * vname, const char * extraset=0);
            static idx _QP_SetIdxVar(sSql * db, sVec < idx > * reqs, idx val, const char * tblname, const char * idxname, const char * vname, const char * extraset=0 );
            static idx _QP_GetIdxVar(sSql * db, sVec < idx > * reqs, sVec < idx > * vals, const char * tblname, const char * idxname, const char * vname );


        public:
            void QP_flushCache();
            char * QP_configGet( sStr * vals00, const char * pars00, bool single=true);
            bool QP_configSet(const char * par, const char * val);

        public:
            idx QP_reqSubmit( const char * serviceName, idx subip , idx priority,  idx usrid );
            idx QP_reqReSubmit(idx * req, idx cnt, idx delaySeconds = 0);
            idx QP_requestGet(idx req , void * r, bool isGrp=0, const char * serviceName=0) ;
            idx QP_requestGetForGrp(idx grp , void * RList , const char * serviceName=0);
            idx QP_requestGetForGrp2(idx grp , sVec<sQPrideBase::Request> * RList , const char * serviceName=0);
            char * QP_requestGetPar(idx req, idx type, sStr * va, const bool req_onlyl);
            idx QP_reqGrab(const char * service, idx job, idx inBundle,idx status, idx action);

        public:
            idx QP_grpSubmit(const char * serviceName, idx subip, idx priority, idx numSubReqs,idx usrid,idx previousGrpSubmitCounter);
            idx QP_grpSubmit2(const char * serviceName, idx subip, const sQPrideBase::PriorityCnt * priority_cnts, idx num_priority_cnts, idx num_subreqs, idx user_id, idx prev_num_subreqs, idx grp_id);
            idx QP_grp2Req(idx grp, sVec< idx > * reqs, const char * svc=0, idx masterGroup=0) ;
            idx QP_grpAssignReqID(idx req, idx grp, idx jobIDSerial);
            idx QP_req2Grp(idx req, sVec < idx  > * grp, bool isMaster=false);
            idx QP_req2GrpSerial(idx req, idx grp, idx * pcnt, idx svc);
            idx QP_grpGetProgress(idx grp, idx * progress, idx * progress100);

        public:
            bool QP_reqSetPar(idx req, idx type, const char * value,bool isOverwrite);
            char * QP_reqDataGet(idx req, const char * dataName, sMex * data, idx * timestamp=0);
            idx QP_reqDataGetTimestamp(idx req, const char * dataName);
            bool QP_reqDataSet(idx req, const char * dataName, idx dsize , const void * data);
            idx QP_reqDataGetAll(idx req, sVec < sStr > * dataVec, sStr * infos00, sVec < idx > * timestampVec=0 );
            void QP_reqCleanTbl(sVec <sStr> *tblfiles, idx req, const char *dataname);

        public:
            void QP_jobRegisterAlive(idx job);
            idx QP_jobSetStatus(idx job, idx jobstat);
            idx QP_jobSetStatus(sVec < idx > * jobs, idx jobstat);
            idx QP_jobSetAction(idx job, idx jobact);
            idx QP_jobSetAction(sVec < idx > * jobs, idx jobact);
            idx QP_jobRegister(const char * serviceName, const char * hostName, idx pid, idx inParallel);
            idx QP_jobSetMem(idx job, idx curMemSize, idx maxMemSize);
            idx QP_jobGetAction(idx job);
            idx QP_jobGet(idx jobID, sQPrideBase::Job * jobs, idx jobCnt, const char * _reserved=0);
            idx QP_jobSetReq(idx job, idx req);

        public:
            idx QP_serviceID(const char * serviceName);
            idx QP_serviceGet( void * Svc, const char * serviceName=0, idx svcId=0);
            idx QP_serviceUp(const char * svc, idx isUpMask);
            idx QP_serviceList(sStr * lst00, void * svcVecList);
            void QP_servicePurgeOld(sVec < idx > * reqList, const char * service=0, idx limit = -1, bool no_delete = false);
            void QP_servicePath2Clean(sVarSet & res);
            void QP_registerHostIP(const char * sys);
            real QP_getHostCapacity(const char * hostname);
            void QP_getRegisteredIP(sVec <sStr> * ips, const char * equCmd);
            idx QP_workRegisterTime(const char * svc="", const char * params="", idx amount=0, idx time=0);
            idx QP_workEstimateTime(const char * svc="", const char * params="", idx amount=0);

        public:
            idx QP_dbHasLiveConnection(void);
            void QP_dbDisconnect(void);
            idx QP_dbReconnect(void);

        public:
            idx QP_reqSetCgiIP(idx req, idx val);
            idx QP_reqSetSubIP(idx req, idx val);


        public:
            idx QP_reqSetAction(idx req, idx val);
            idx QP_reqSetAction(sVec < idx > * reqs, idx val);
            idx QP_reqGetAction(idx req) ;
            idx QP_reqGetAction(sVec < idx > * reqs, sVec <idx > * vals);

            idx QP_reqGetUser(idx req);
            idx QP_reqSetUser(idx req, idx val);
            void QP_reqRegisterAlive(idx reqID);

            idx QP_reqSetUserKey(idx req, idx val);
            idx QP_reqSetUserKey(sVec < idx > * reqs, idx val);
            idx QP_reqGetUserKey(idx req) ;
            idx QP_getReqByUserKey(idx userKey, const char * serviceName);

            idx QP_reqSetStatus(idx req, idx val);
            idx QP_reqSetStatus(sVec < idx > * reqs, idx val);
            idx QP_reqGetStatus(idx req) ;
            idx QP_reqGetStatus(sVec < idx > * reqs, sVec <idx > * vals);

            idx QP_reqSetProgress(idx req, idx progress, idx progress100);

            bool QP_setLog(idx req, idx job, idx level, const char * txt);
            idx QP_getLog(idx req, bool isGrp, idx job, idx level, sVarSet & log);
            idx QP_getLogGrp(idx grpReq, idx level, sVarSet & log);

            idx QP_purgeReq(sVec<idx> * recVec, idx stat);

        public:
            bool QP_reqLock(idx req, const char * key, idx * req_locked_by, idx max_lifetime, bool force);
            bool QP_reqUnlock(idx req, const char * key, bool force);
            idx QP_reqCheckLock(const char * key);

        public:
            char * QP_resourceGet(const char * service, const char * resName,    sMex * data, idx * timestamp);
            bool QP_resourceSet(const char * service, const char * resName, idx ressize , const void * data);
            idx QP_resourceGetAll(const char * service,  sStr * infos00 , sVec < sStr > * dataVec, sVec < idx > * timestamps );
            bool QP_resourceDel(const char * service, const char * dataName = 0);

        public:

            idx QP_sysPeekOnHost(void * srvl,  const char * hostname );
            idx QP_sysPeekReqOrder(idx req, const char * srv=0 , idx * pRunning=0);

            idx QP_sysGetKnockoutJobs(sVec <sQPrideBase::Job> * jobs, sVec < idx > * svcIDs);
            idx QP_sysGetImpoliteJobs(sVec <sQPrideBase::Job> * jobs, sVec < idx > * svcIDs);
            idx QP_sysJobsGetList(sVec <sQPrideBase::Job> * jobs , idx stat, idx act, const char * hostname );
            idx QP_sysRecoverRequests(sVec <idx > * killed, sVec < idx > * recovered);
            idx QP_sysCapacityNeed(idx * capacity_total = 0);

    };

}

#endif 

