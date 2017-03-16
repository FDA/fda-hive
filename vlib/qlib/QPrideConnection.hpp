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
#ifndef _QPrideConnection_qLib_hpp
#define _QPrideConnection_qLib_hpp

#include <slib/core/var.hpp>
#include <slib/core/vec.hpp>
#include <qlib/QPrideBase.hpp>

#ifdef SLIB_WIN
    #pragma warning( disable : 4100)
#endif

namespace slib
{

    class sQPrideConnection
    {
        public:

            sQPrideConnection( ){}
            virtual ~sQPrideConnection(){}

        public: // configuration
            virtual char * QP_configGet( sStr * vals00, const char * pars00, bool single=true){return 0;}
            virtual bool QP_configSet(const char * par, const char * val){return false;}
            virtual  void QP_flushCache(){return;}

        public: //     requests
            virtual idx QP_reqSubmit( const char * serviceName, idx subip , idx priority , idx usrid){return 0;}
            virtual idx QP_reqReSubmit(idx * req, idx cnt, idx delaySeconds = 0)
            {
                return 0;
            }
            virtual idx QP_requestGet(idx req , void * r, bool isGrp=0, const char * serviceName=0) {return 0;} // r is of sQPrideBase::Request type
            virtual idx QP_requestGetForGrp(idx grp, void * r, const char * serviceName=0)  {return 0;}
            virtual char * QP_requestGetPar(idx req, idx type, sStr * val){return 0;}
            virtual idx QP_reqGrab(const char * service, idx job, idx inBundle,idx status, idx action){return getpid();}

        public: //     groups
            virtual idx QP_grpSubmit(const char * serviceName, idx subip, idx priority, idx numSubReqs, idx usrid, idx previousGrpSubmitCounter){return 0;}
            virtual idx QP_grp2Req(idx grp, sVec< idx > * reqs, const char * svc=0, idx masterGroup=0){return grp;}
            virtual idx QP_grpAssignReqID(idx req, idx grp, idx jobIDSerial){return req;}
            virtual idx QP_req2Grp(idx req, sVec < idx  > * grp, bool isMaster=false){return req;}
            virtual idx QP_req2GrpSerial(idx req, idx grp, idx * pcnt, idx svc){return 0;}
            virtual idx QP_grpGetProgress(idx grp, idx * progress, idx * progress100){if(progress)*progress=0;if(progress100)*progress100=0;return 0;}
            //virtual idx QP_grpSetCollector(idx grp, idx jobID){return 0;}
            //virtual idx QP_grpGetCollector(idx grp){return grp;}

        public: //     data
            virtual bool QP_reqSetPar(idx req, idx type, const char * value,bool isOverwrite=true){return 0;}
            virtual char * QP_reqDataGet(idx req, const char * dataName, sMex * data, idx * timestamp=0){return 0;}
            virtual idx QP_reqDataGetTimestamp(idx req, const char * dataName){return 0;}
            virtual bool QP_reqDataSet(idx req, const char * dataName, idx dsize , const void * data){return 0;}
            virtual idx QP_reqDataGetAll(idx req, sVec < sStr > * dataVec, sStr * infos00, sVec < idx > * timestampVec=0 ){return 0;}
            virtual void QP_reqCleanTbl(sVec <sStr> *tblfiles, idx req, const char *dataname) {};

        public: // jobs
            virtual void QP_jobRegisterAlive(idx job){return;}
            virtual idx QP_jobSetStatus(idx job, idx jobstat){return 0;}
            virtual idx QP_jobSetStatus(sVec < idx > * jobs, idx jobstat){return 0;}
            virtual idx QP_jobSetAction(idx job, idx jobstat){return 0;}
            virtual idx QP_jobSetAction(sVec < idx > * jobs, idx jobact){return 0;}
            virtual idx QP_jobRegister(const char * serviceName, const char * hostName, idx pid, idx inParallel){return 0;}
            virtual idx QP_jobSetMem(idx job, idx curMemSize, idx maxMemSize){return 0;}
            virtual idx QP_jobGetAction(idx job){return 0;}
            virtual idx QP_jobSetReq(idx job, idx req){return 0;}

        public: //     services
            virtual idx QP_serviceID(const char * serviceName){return 0;} // make it lazy
            virtual idx QP_serviceGet(void * Svc,const char * serviceName, idx svcId){return 0;}
            virtual idx QP_serviceUp(const char * svc, idx isUpMask){return 0;}
            // void * is sVec < Service >*
            virtual idx QP_serviceList(sStr * lst00, void * svcVecList) = 0;
            virtual void QP_servicePurgeOld(sVec < idx > * reqList, const char * service=0, idx limit = -1, bool no_delete=false){return  ;}
            virtual void QP_servicePath2Clean(sVarSet & res){}
            virtual void QP_registerHostIP(const char * sys) {return ; }
            virtual real QP_getHostCapacity(const char * hostname){ return 0;}
            virtual void QP_getRegisteredIP(sVec <sStr> * ips, const char * equCmd) {return ; }
            virtual idx QP_workRegisterTime(const char * svc="", const char * params="", idx amount=0, idx time=0){return 0;};
            virtual idx QP_workEstimateTime(const char * svc="", const char * params="", idx amount=0){return 0;};

        public: // db
            virtual idx QP_dbHasLiveConnection(void){return 1;}
            virtual void QP_dbDisconnect(void){return ;}
            virtual idx QP_dbReconnect(void){return 1;}

        public: //     ips
            virtual idx QP_reqSetCgiIP(idx req, idx val){return 0;}
            virtual idx QP_reqSetSubIP(idx req, idx val){return 0;}

        public: //     status, action and progress
            virtual idx QP_reqSetAction(idx req, idx val){return 0;}
            virtual idx QP_reqSetAction(sVec < idx > * reqs, idx val){return 0;}
            virtual idx QP_reqGetAction(idx req) {return 0;}
            virtual idx QP_reqGetAction(sVec < idx > * reqs, sVec <idx > * vals){return 0;}
            virtual void QP_reqRegisterAlive(idx reqID){}
            virtual idx QP_reqGetUser(idx req){return 0;}
            /*virtual idx QP_reqGetObject(idx req){return 0;}
            virtual idx QP_reqSetObject(idx req, idx obj){return 0;}
            */

            virtual idx QP_reqSetStatus(idx req, idx val){return 0;}
            virtual idx QP_reqSetStatus(sVec < idx > * reqs, idx val){return 0;}
            virtual idx QP_reqGetStatus(idx req) {return 0;}
            virtual idx QP_reqGetStatus(sVec < idx > * reqs, sVec <idx > * vals){return 0;}

            virtual idx QP_reqSetUserKey(idx req, idx val){return 0;}
            virtual idx QP_reqSetUserKey(sVec < idx > * reqs, idx val){return 0;}
            virtual idx QP_reqGetUserKey(idx req) {return 0;}
            virtual idx QP_getReqByUserKey(idx userKey, const char * serviceName){return userKey;}

            virtual idx QP_reqSetProgress(idx req, idx progress, idx progress100){printf("progress = %" DEC " %" DEC "%%\n",progress,progress100);return progress;}

            virtual bool QP_setLog(idx req, idx job, idx level, const char * txt)
            {
                if( req ) {
                    printf("REQ[%" DEC "]", req);
                }
                if( job ) {
                    printf("%sJOB[%" DEC "]", req ? " " : "", job);
                }
                if( txt ) {
                    printf(":\t%s\n", txt);
                }
                return true;
            }
            virtual idx QP_getLog(idx req, bool isGrp, idx job, idx level, sVarSet & log)
            {
                return 0;
            }
            virtual idx QP_purgeReq(sVec<idx> * recVec, idx stat) { return 0;}

        public: // locking
            virtual bool QP_reqLock(idx req, const char * key, idx * req_locked_by, idx max_lifetime, bool force) { return false; }
            virtual bool QP_reqUnlock(idx req, const char * key, bool force) { return false; }
            virtual idx QP_reqCheckLock(const char * key) { return 0; }

        public:// resources
            virtual char * QP_resourceGet(const char * service, const char * resName,    sMex * data, idx * timestamp){return 0;}
            virtual bool QP_resourceSet(const char * service, const char * resName, idx ressize , const void * data){return false;}
            virtual idx QP_resourceGetAll(const char * service, sStr * infos00, sVec < sStr > * dataVec , sVec < idx > * timestamps ){return 0;}
            virtual bool QP_resourceDel(const char * service, const char * resName = 0){return false;}



        public: // system operations

            virtual idx QP_sysPeekOnHost(void * srvl,  const char * hostname ){return 0;}
            virtual idx QP_sysPeekReqOrder(idx req, const char * srv=0 , idx * pRunning=0 ){return 0;}

            virtual idx QP_sysGetKnockoutJobs(sVec <sQPrideBase::Job> * jobs, sVec < idx > * svcIDs){return 0;}
            virtual idx QP_sysGetImpoliteJobs(sVec <sQPrideBase::Job> * jobs, sVec < idx > * svcIDs){return 0;}
            virtual idx QP_sysJobsGetList(sVec <sQPrideBase::Job> * jobs , idx stat, idx act, const char * hostname ){return 0;}
            virtual idx QP_sysRecoverRequests(sVec <idx > * killed, sVec < idx > * recovered){return 0;}
            virtual idx QP_sysCapacityNeed(idx * capacity_total = 0){return 0;}

        public:
            static bool hostNameMatch (const char * hosts, const char * hostName, idx * pmaxjob=0);
            static bool hostNameListMatch(const char * hosts, const char * hostName, idx * pmaxjob=0, idx * pwhich=0);




    };



}/// namespace slib

#ifdef SLIB_WIN
    #pragma warning( default: 4100)
#endif

#endif // _QPrideBase_qLib_hpp



