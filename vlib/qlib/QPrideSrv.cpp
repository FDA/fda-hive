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
#include <slib/std.hpp>
#include <ulib/ulib.hpp>
#include "QPrideSrv.hpp"

using namespace slib;

/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  Initialization
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */

static const char * replaceCommandLineVars(sStr * dst, const char * src, idx len, const sQPrideSrv::Service& svc, const char * resourceRoot)
{
    sStr repl00;
    repl00.addString(svc.name);
    repl00.add0();
    repl00.addString(resourceRoot);
    repl00.add0(2);
    return sString::searchAndReplaceStrings(dst, src, len, "$(svc)"_"$(resourceRoot)"__, repl00.ptr(), 0, false);
}

const char * replaceCommandLineArgs(sStr& proc, sStr* args, const sQPrideSrv::Service& svc, const char * resourceRoot, sUsr* user)
{
    sStr buf("%s", svc.cmdLine);
    const char * objDefinition, *objDefEnd;
    while( (objDefinition = strstr(buf, "$(obj=")) != 0 && (objDefEnd = strstr(objDefinition, ")")) != 0 ) {
        sHiveId objId(objDefinition + 6);
        sStr objPath;
        if( user ) {
            sUsrObj obj(*user, objId);
            if( obj.Id() && obj.getFilePathname(objPath, "any") ) {
                objPath.cut(-3); // hack to make a path while implementation here is undecided
            }
        }
        sStr b1;
        b1.add(objDefinition, objDefEnd - objDefinition + 1);
        b1.add0(2);
        sStr b2;
        sString::searchAndReplaceStrings(&b2, buf.ptr(), buf.length(), b1.ptr(), objPath.ptr(), 0, false);
        buf.printf(0, "%s", b2.ptr());
    }
    proc.cut(0);
    const char * cl = buf.ptr();
    if( *cl != '\\' && *cl != '/' ) {
        proc.printf("%s", resourceRoot); // if not an absolute path - start from resource root
    }
    sStr exe;
    cl += sString::copyUntil(&exe, cl, 0, sString_symbolsBlank);
    if( args ) {
        args->cut0cut();
        replaceCommandLineVars(args, cl, 0, svc, resourceRoot);
        args->shrink00();
    }
    replaceCommandLineVars(&proc, exe.ptr(), exe.length(), svc, resourceRoot);
    proc.shrink00();
    return proc.ptr();
}

sQPrideSrv::sQPrideSrv(const char * defline00, const char * service)
    : sQPrideProc(defline00, service), usedCapacity(0), m_secsToPurge(300), m_isMaintainer(false), emailBatch(20), m_maintainTimeLast(0), m_isBroadcaster(false)
{
    if( !ok ) {
        return;
    }
    ps.setMode(sPS::eExec_BG);
    m_secsToPurge = cfgInt(0, "qm.PurgeFrequency", 300);
    //doStdin = 0;
}

bool sQPrideSrv::init(void)
{
    sStr rootPath;
    sUsrObj::initStorage(cfgStr(&rootPath, 0, "user.rootStoreManager"), cfgInt(0, "user.storageMinKeepFree", (udx)20 * 1024 * 1024 * 1024));
    return true;
}

bool sQPrideSrv::OnInit(void)
{
    sQPrideProc::OnInit();
    return init();
}

void sQPrideSrv::OnQuit(void)
{
    if( !m_isMaintainer && m_nodeMan ) {
        bool haveJobsRunning = false;
        for(idx i = 0; i < svcJobsAlive.dim(); ++i) {
            if( svcJobsAlive[i] > 0 && strcmp(svcList[i].name, "qm") != 0 ) {
                haveJobsRunning = true;
                break;
            }
        }
        logOut(eQPLogType_Info, "onQuit node manager there %s alive jobs\n", haveJobsRunning ? "are" : "is no");
        if( !haveJobsRunning ) {
            sStr cmd("%s -stop", m_nodeMan.ptr());
            const idx ret = sPS::execute(cmd);
            if( ret != 0 ) {
                logOut(eQPLogType_Error, "Node man call '%s' returned %"DEC"\n", cmd.ptr(), ret);
            }
        }
    }
}

bool sQPrideSrv::OnCommand(const char * command, const char * value)
{
    idx ll;
#define jobIsCmd(_cmd) (!strncmp(command,_cmd,(ll=sLen(_cmd))))

    if( !init() ) {
        return false;
    } else if( jobIsCmd("maintain")) {
        idx x = 0;
        sscanf(value, "%"DEC, &x);
        m_isMaintainer = x != 0;
    } else if( jobIsCmd("broadcast:")) {
        sleepMS(rand()*200/RAND_MAX);
        // wait some random time 1/5 of a second
        messageSubmit("broadcast", command + ll, false, "%s", value);
    } else if( jobIsCmd("broadcast")) {
        m_isBroadcaster = true;
    } else if( jobIsCmd("knockout")) {
        return OnCommandKnockout(command, value);
    } else if( jobIsCmd("impolites")) {
        return OnCommandKillImpolites(command, value);
    } else if( jobIsCmd("terminated")) {
        return OnCommandCleanTerminated(command, value);
    } else if( jobIsCmd("stopjobs")) {
        return OnCommandStopJobs(command, value);
    } else if( jobIsCmd("launch")) {
        return OnCommandLaunch(command, value);
    } else if( jobIsCmd("soundwake")) {
        return OnCommandSoundWake(command, value);
    } else if( jobIsCmd("wake")) {
        messageSubmit(vars.value("thisHostName"), value, false, "wake");
        return false;
    } else if( jobIsCmd("recover")) {
        return OnCommandRecover(command, value);
    } else if( jobIsCmd("purge")) {
        return OnCommandPurge(command, value);
    } else if( jobIsCmd("register")) {
        return OnCommandRegister(command, value);
    } else if( jobIsCmd("email")) {
        if( value && value[0] ) {
            sscanf(value, "%"UDEC, &emailBatch);
        }
        if( emailBatch ) {
            return OnCommandEmail(command, value);
        }
    } else if( jobIsCmd("nodeman") ) {
        m_nodeMan.empty();
        if( value && value[0] ) {
            sFilePath fullp;
            if( value[0] != '/' && value[0] != '\\' ) {
                fullp.curDir();
            }
            fullp.printf("%s.os"SLIB_PLATFORM, value);
            if( sFile::exists(fullp) ) {
                m_nodeMan.printf(0, "%s", fullp.ptr());
            } else {
                logOut(eQPLogType_Error, "Script '%s' not found\n", m_nodeMan.ptr());
            }
        }
    } else if( jobIsCmd("capacity") ) {
        return OnCommandCapacity(command, value);
    } else {
        return false;
    }
    return true;
}

idx sQPrideSrv::OnGrab(idx)
{
    makeVar00();

    // prepare the list of services running on this host
    svcList.cut(0);
    sysPeekOnHost(&svcList);
    svcIDs.cut(0);
    for(idx is = 0; is < svcList.dim(); ++is) {
        if( svcList[is].parallelJobs == 1 ) {
            svcList[is].parallelJobs = 0;
        }
        svcIDs.vadd(1, svcList[is].svcID);
    }

    // prepare the lists of all processes running per service on this host
    sVec<sPS::Stat> all_ps;
    // grab all processes for current user
#if _DEBUG
    logOut(eQPLogType_Info, "ps mode: %u '%s'\n", ps.getMode(), ps.getScript() ? ps.getScript() : "");
#endif
    idx psRes = ps.getProcList(&all_ps, 0, true);
#if _DEBUG
    logOut(eQPLogType_Info, "ps qty: %"DEC"\n", all_ps.dim());
#endif
    if( psRes >= 0 && ps.getMode() == sPS::eExec_Extern ) {
        // this is to pickup qsrv or any other local jobs in case when we run in external mode
        sPS lcl;
        psRes = lcl.getProcList(&all_ps, 0, true);
#if _DEBUG
        logOut(eQPLogType_Info, "ps += local qty: %"DEC"\n", all_ps.dim());
#endif
    }
    if( psRes >= 0 ) {
        svcPSList.cut(0);
        svcJobsAlive.cut(0);
        sFilePath Proc, tmp;
        const char * resourceRoot = vars.value("resourceRoot");
        const real usedCapacityPrev = usedCapacity;
        usedCapacity = 0;
#if _DEBUG
        logOut(eQPLogType_Info, "Host capacity used %f\n", usedCapacity);
        if( all_ps.dim() ) {
            sStr pslog("ps[%"DEC"]:", all_ps.dim());
            for(idx i = 0; i < all_ps.dim(); ++i) {
                pslog.printf(" %"DEC" %s,", all_ps.ptr(i)->pid, all_ps.ptr(i)->cmd);
            }
            logOut(eQPLogType_Info, "%s\n", pslog.ptr());
        }
#endif
        for(idx is = 0; is < svcList.dim(); ++is) {
            svcJobsAlive.vadd(1, 0); // reserve space for count
            // Get the process name
            Proc.cut(0);
            const char * proc = replaceCommandLineArgs(tmp, 0, svcList[is], resourceRoot, user);
            tmp.cut(0);
            proc = replVar00(&tmp, proc);
            sVec<sPS::Stat> * psProc = svcPSList.add();
            for(idx i = 0; proc && psProc && i < all_ps.dim(); ++i) {
                if( strcmp(proc, all_ps.ptr(i)->cmd) == 0 ) {
                    usedCapacity += svcList[is].capacity;
                    sPS::Stat* st = psProc->add();
                    if( st ) {
                        *st = *(all_ps.ptr(i));
                    }
                }
            }
#if _DEBUG
            if( psProc && psProc->dim() ) {
                sStr pslog("%s[%"DEC"]:", proc, psProc->dim());
                for(idx i = 0; i < psProc->dim(); ++i) {
                    pslog.printf(" %"DEC" %s,",psProc->ptr(i)->pid, psProc->ptr(i)->cmd);
                }
                logOut(eQPLogType_Info, "%s\n", pslog.ptr());
            }
#endif
        }
        if( usedCapacity != usedCapacityPrev ) {
            logOut(eQPLogType_Info, "Host capacity used %f of %f\n", usedCapacity, getHostCapacity());
        }
    } else {
        logOut(eQPLogType_Info, "ps FAILED: %"DEC"\n", psRes);
    }

    OnCommand("soundwake", 0);
    // cleanup
    OnCommand("terminated", 0);
    if( m_isMaintainer ) {
        OnMaintain();
    }
    OnCommand("impolites", 0);
    OnCommand("knockout", 0);
    if( psRes >= 0 ) {
        OnCommand("stopjobs", 0);
        OnCommand("launch", 0);
    }
    OnCommand("register", 0);
    ++loopCnt;
    return 0;
}

idx sQPrideSrv::OnMaintain(void)
{
    if( m_secsToPurge ) {
        sTime t;
        idx timeHasPassedSinceLastTime = t.time(&m_maintainTimeLast);
        if( timeHasPassedSinceLastTime > m_secsToPurge ) {
            m_maintainTimeLast = t;
            OnCommand("recover", 0);
            OnCommand("purge", 0);
            OnCommand("email", 0);
            OnCommand("capacity", 0);
        }
    }
    return 0;
}


/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  Operations
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */

idx sQPrideSrv::__findServiceForJob(Job * job)
{
    idx ia;
    for(ia = 0; ia < svcList.dim() && svcList[ia].svcID != job->svcID; ++ia)
        ;
    if( ia >= svcList.dim() )
        return -1;

    idx cntHosts, numHost = hostNumInPool(svcList.ptr(ia), &cntHosts);  // get the host numeral in the pool of hosts
    if( svcList[ia].parallelJobs > 1 && (int) (numHost % (svcList[ia].parallelJobs)) != (int) (((job->inParallel - 1)) % cntHosts) ) // check if this host is carrying this parralel job
        return -1;
    return ia;
}

bool sQPrideSrv::OnCommandKnockout(const char *, const char *)
{
    sVec<Job> jobs;
    sysGetKnockoutJobs(&jobs, &svcIDs);
    if( jobs.dim() == 0 )
        return true;

    sVec<idx> pidsToKill, jobsKilledBefore, reqsToKill;
    sStr sJob, sReq, sKnock;

    //
    // Loop through all jobs that should be knocked out
    //
    for(idx ij = 0, ia, ip; ij < jobs.dim(); ++ij) {

        // find the service (ia) under consideration and find the process in the list of running processes (ip)
        if( (ia = __findServiceForJob(&jobs[ij])) == -1 )
            continue;
        for(ip = 0; ip < svcPSList[ia].dim() && (jobs[ij].pid != svcPSList[ia][ip].pid); ip++)
            ; // find if this process is still running

        if( ip == svcPSList[ia].dim() ) {  // this knockout candidate has been killed before
            jobsKilledBefore.vadd(1, jobs[ij].jobID);
            sKnock.printf("\t%s[%"DEC"]\n", svcList[ia].name, jobs[ij].jobID);
        } else {
            pidsToKill.vadd(1, jobs[ij].pid);
            sJob.printf("\t%s[%"DEC"/%"DEC"/%"DEC"]\n", svcList[ia].name, jobs[ij].jobID, jobs[ij].pid, jobs[ij].reqID);

            if( jobs[ij].reqID ) {
                reqsToKill.vadd(1, jobs[ij].reqID);
                sReq.printf("\t%s[%"DEC"]\n", svcList[ia].name, jobs[ij].reqID);
            }
            //jobs[ij].killCnt++;
        }

    }

    if( pidsToKill.dim() ) {
        logOut(eQPLogType_Info, " knocking out (killing) process(es):\n%s\n", sJob.ptr());
        doKillProcesses(pidsToKill.ptr(), pidsToKill.dim());
        jobSetStatus(&jobsKilledBefore, eQPJobStatus_Knockouted);
    }

    if( reqsToKill.dim() ) {
        logOut(eQPLogType_Info, " requests from knock-outed jobs(s) will be available for recovery:\n%s\n", sReq.ptr());
        //reqSetStatus(&reqsToKill, eQPReqStatus_Killed);
    }

    if( jobsKilledBefore.dim() ) {
        logOut(eQPLogType_Info, " marking knocked out job(s) as such:\n%s\n", sKnock.ptr());
        jobSetStatus(&jobsKilledBefore, eQPJobStatus_Knockouted);

    }
    return true;
}

bool sQPrideSrv::OnCommandKillImpolites(const char *, const char *)
{
    sVec<Job> jobs;
    sysGetImpoliteJobs(&jobs, &svcIDs);
    if( jobs.dim() == 0 )
        return true;

    sStr sJob, sReq;
    sVec<idx> pidsToKill, reqsToKill, jobsToKill;

    for(idx ij = 0, ia, ip; ij < jobs.dim(); ++ij) {
        if( jobs[ij].pid == getpid() )
            continue; // never kill self

        // find the service (ia) under consideration and find the process in the list of running processes (ip)
        if( (ia = __findServiceForJob(&jobs[ij])) == -1 )
            continue;
        for(ip = 0; ip < svcPSList[ia].dim() && (jobs[ij].pid != svcPSList[ia][ip].pid); ip++)
            ; // find if this process is still running
        if( ip == svcPSList[ia].dim() )
            continue; // no running process corresponds to this impolite

        sJob.printf("\t%s[%"DEC"/%"DEC"/%"DEC"]\n", svcList[ia].name, jobs[ij].jobID, jobs[ij].pid, jobs[ij].reqID);
        pidsToKill.vadd(1, jobs[ij].pid);
        jobsToKill.vadd(1, jobs[ij].jobID);
        if( jobs[ij].reqID ) {
            reqsToKill.vadd(1, jobs[ij].reqID);
            sReq.printf("\t%s[%"DEC"]\n", svcList[ia].name, jobs[ij].reqID);
        }

    }

    if( pidsToKill.dim() ) {
        logOut(eQPLogType_Info, " killing process(es):\n%s\n", sJob.ptr());
        doKillProcesses(pidsToKill.ptr(), pidsToKill.dim());
        jobSetStatus(&jobsToKill, eQPJobStatus_Killed);
    }

    if( reqsToKill.dim() ) {
        logOut(eQPLogType_Info, " requests from killed impolite jobs are ready for recovery:\n%s\n", sReq.ptr());
        //reqSetStatus(&reqsToKill, eQPReqStatus_Killed);
    }
    return true;
}

bool sQPrideSrv::OnCommandCleanTerminated(const char *, const char *)
{
    sVec<Job> jobs;
    sysJobsGetList(&jobs, eQPJobStatus_Running, eQPJobAction_Any);

    sVec<idx> jobsToMarkAsTerminated, reqsToMarkAsKilled;
    sStr sJob, sReq;

    for(idx ij = 0, ia, ip; ij < jobs.dim(); ++ij) {
        // find the service (ia) under consideration and find the process in the list of running processes (ip)
        if( (ia = __findServiceForJob(&jobs[ij])) == -1 )
            continue;
        for(ip = 0; ip < svcPSList[ia].dim() && (jobs[ij].pid != svcPSList[ia][ip].pid); ip++)
            ; // find if this process is still running
        if( ip < svcPSList[ia].dim() ) {
            ++svcJobsAlive[ia];
            //lastTimeSomethingWasAliveOnThisMchine=time();
            continue;  // this job is still running , we do not touch it
        }

        sJob.printf("\t%s[%"DEC"/%"DEC"/%"DEC"]\n", svcList[ia].name, jobs[ij].jobID, jobs[ij].pid, jobs[ij].reqID);
        jobsToMarkAsTerminated.vadd(1, jobs[ij].jobID);
        if( jobs[ij].reqID ) {
            reqsToMarkAsKilled.vadd(1, jobs[ij].reqID);
            sReq.printf("\t%s[%"DEC"]\n", svcList[ia].name, jobs[ij].reqID);
        }

    }

    if( jobsToMarkAsTerminated.dim() ) {
        logOut(eQPLogType_Info, " declaring job(s) as terminated:\n%s\n", sJob.ptr());
        jobSetStatus(&jobsToMarkAsTerminated, eQPJobStatus_Terminated);
    }

    if( reqsToMarkAsKilled.dim() ) {
        logOut(eQPLogType_Info, " the following requests(s) from terminated jobs will be dangling until recovery:\n%s\n", sJob.ptr());
        //reqSetStatus(&reqsToMarkAsKilled, eQPReqStatus_Killed);
    }

    return true;
}

bool sQPrideSrv::OnCommandStopJobs(const char *, const char *)
{
    sVec<Job> jobs;
    sysJobsGetList(&jobs, eQPJobStatus_Running, eQPJobAction_Any);

    sVec<idx> jobsToStop;
    sStr sJob, sSvc;
    sDic<idx> svcToStop;

    for(idx ij = 0, ia; ij < jobs.dim(); ++ij) {
        if( jobs[ij].svcID < 2  ||(svcList[ij].name && (strcmp(svcList[ij].name,"qm")==0)))
            continue; // never stop ourselves
        // find the service (ia) under consideration and find the process in the list of running processes (ip)
        if( (ia = __findServiceForJob(&jobs[ij])) == -1 )
            continue;

        // check if this service is still up
        if( svcList[ia].parallelJobs > 1 ) {
            if( ((svcList[ia].isUp) & (((idx) 1) << ((jobs[ij].inParallel - 1)))) )  // this particular parallel job is up ?
                continue;  // if this thread is Up
        } else if( svcList[ia].isUp != 0 )
            continue;

        sJob.printf("\t%s[%"DEC"/%"DEC"/%"DEC"]\n", svcList[ia].name, jobs[ij].jobID, jobs[ij].pid, jobs[ij].reqID);
        jobsToStop.vadd(1, jobs[ij].jobID);

        svcToStop[&jobs[ij].svcID] = jobs[ij].svcID;
        sSvc.printf("\t%s\n", svcList[ia].name);

    }

    if( jobsToStop.dim() ) {
        logOut(eQPLogType_Info, " signaling job(s) to stop:\n%s\n", sJob.ptr());
        jobSetAction(&jobsToStop, eQPJobAction_Kill);
    }

    if( svcToStop.dim() ) {
        const char * thisHost = vars.value("thisHostName");
        logOut(eQPLogType_Info, " messaging services(s) to stop:\n%s\n", sSvc.ptr());
        for(idx is = 0; is < svcToStop.dim(); ++is)
            messageSubmit(m_isBroadcaster ? "broadcast" : thisHost, svcToStop[is], false, "-exit");
    }
    return true;
}

bool sQPrideSrv::OnCommandSoundWake(const char *, const char *)
{
    idx somout, is;

    const char * thisHost = vars.value("thisHostName");

    for(is = 0, somout = 0; is < svcList.dim(); ++is) {

        //if( strcmp(cmd,"wake")!=0 ){
        if( svcList[is].hasReqToGrab == 0 ) // nothing to be executed for this service
            continue;
        if( svcList[is].isUp == 0 ) // we do not send trigger if the service is down: there should be nobody to respond anyway
            continue;
        if( svcList[is].svcID == 1 || (svcList[is].name && (strcmp(svcList[is].name,"qm")==0)) ) // we never wake up daemon service itself
            continue;
        //}

        if( !somout )
            logOut(eQPLogType_Info, " messaging services(s) to wake up:\n");
        logOut(eQPLogType_Info, "\t%s\n", svcList[is].name);
        ++somout;
        messageSubmit(m_isBroadcaster ? "broadcast" : thisHost, svcList[is].svcID, false, "wake");
    }
    if( somout )
        logOut(eQPLogType_Info, "\n");

    return true;
}

bool sQPrideSrv::OnCommandRecover(const char *, const char *)
{
    sVec<idx> killed, recovered;
    sStr sKil, sRec;

    sysRecoverRequests(&killed, &recovered);

    if( killed.dim() ) {
        for(idx ii = 0; ii < killed.dim(); ++ii)
            sKil.printf("\t%"DEC"\n", killed[ii]);
        logOut(eQPLogType_Info, " following abandoned requests have been marked as killed:\n%s\n", sKil.ptr());
    }

    if( recovered.dim() ) {
        for(idx ii = 0; ii < recovered.dim(); ++ii)
            sRec.printf("\t%"DEC"\n", recovered[ii]);
        logOut(eQPLogType_Info, " following abandoned requests have been resubmitted:\n%s\n", sRec.ptr());
    }
    return true;
}

bool sQPrideSrv::OnCommandRegister(const char *, const char *)
{
    registerHostIP();
    return true;
}

void sQPrideSrv::purge(TPurgeData & data)
{
    sVec<idx> reqListToSync;
    idx limit = cfgInt(0, "qm.purgeReqLimit", 1000);
    servicePurgeOld(&reqListToSync, 0, limit, true ); // service name is null here  means we delete from all services
    sStr pr, reqs;
    if( reqListToSync.dim() ) {
        for(idx i = 0; i < reqListToSync.dim(); ++i) {
            pr.printf(",reqID");
            reqs.printf(",%"DEC, reqListToSync[i]);
        }
    }
    sDic<char> synced;
    if( pr && reqs ) {
        sUsr qpride("qpride");
        qpride.m_SuperUserMode = true;
        sUsrObjRes out;
        qpride.objs2("process+", out, 0, pr.ptr(1), reqs.ptr(1));
        for(sUsrObjRes::IdIter it = out.first(); out.has(it); out.next(it)) {
            const sHiveId * id = out.id(it);
            sUsrProc p(qpride, *id);
            if( p.Id() && !p.propSync() ) {
                logOut(eQPLogType_Info, "Properties of request %s failed to sync\n", id->print());
            } else {
                idx req = p.reqID();
                synced.set(&req, sizeof(req));
            }
        }
    }
    sVec<idx> reqListToDelete;
    servicePurgeOld(&reqListToDelete, 0, limit, false); // service name is null here  means we delete from all services
    if( reqListToDelete.dim() ) {
        sStr purged("\t");
        for(idx i = 0; i < reqListToDelete.dim(); ++i) {
            if( !synced.get(&reqListToDelete[i], sizeof(reqListToDelete[i])) ) {
                logOut(eQPLogType_Info, "Request %"DEC" deleted w/o properties sync\n", reqListToDelete[i]);
            }
            data.reqs.set(&reqListToDelete[i], sizeof(reqListToDelete[i])); //save for file cleanup in usrv
            purged.printf("%"DEC"%s", reqListToDelete[i], ((i + 1) % 5) ? ", " : "\n\t");
        }
        logOut(eQPLogType_Info, "following %"DEC" requests have been purged:\n%s\n", reqListToDelete.dim(), purged.ptr());
    }
}

bool sQPrideSrv::OnCommandPurge(const char *, const char *)
{
    TPurgeData data;
    purge(data); // accumulate req and obj ids
    sVarSet res;
    servicePath2Clean(res);
    for(idx r = 0; r < res.rows; ++r) {
        data.path00 = res.val(r, 0);
        data.cleanUpDays = res.ival(r, 1);
        data.masks00 = res.val(r, 2);
        purgeDir(data);
    }
    if( data.size ) {
        logOut(eQPLogType_Info, "%"UDEC" bytes returned to storage\n", data.size);
    }
    return true;
}

bool sQPrideSrv::OnCommandLaunch(const char *, const char *)
{

    const char * resourceRoot = vars.value("resourceRoot");
    const char * thisHost = vars.value("thisHostName");

    sStr cmdline, tmp, buf;
    idx numHost, cntHosts;
    const real hostCapacity = getHostCapacity();
    real currCapacity = usedCapacity;

    const bool loggingOn = setupLog(true) < 0;

#if _DEBUG
    if( hostCapacity ) {
        logOut(eQPLogType_Info, "Host capacity used %f of %f\n", currCapacity, hostCapacity);
    }
#endif
    for(idx is = 0, ip; is < svcList.dim(); ++is) {

        if( hostCapacity && (svcList[is].capacity + currCapacity) > hostCapacity ) {
            // host has no available capacity for this service
            continue;
        }

        idx cntThisKindOnAllMachines=svcJobsAlive[is];

        if( svcList[is].svcID < 2 || (svcList[is].name && (strcmp(svcList[is].name,"qm")==0)) )
            continue; // we do not launch ourselves
        if( svcList[is].isUp == 0 )
            continue; // if the service is down we do not launch it
        if( svcList[is].maxJobs == 0 )
            continue; // never launch place-holder services

        if( svcList[is].hasReqToGrab == 0 && (cntThisKindOnAllMachines>=svcList[is].activeJobReserve) ) // nothing to be executed for this service, no reason to launch a working daemon
            continue;

        idx maxJobOnThisMachine = svcList[is].maxJobs;

        //
        // count the number of jobs of each kind running on this host
        //
        sVec<idx> cntKinds;
        cntKinds.vadd(1, svcPSList[is].dim()); // the zero-th element has the total number for all parallel jobs
        for(idx ib = 1, cntThi=0; ib <= svcList[is].parallelJobs; ++ib) { // now for all parallel jobs
            cmdline.printf(0, "jobarr %"DEC, ib - 1 + 1); // which are specified like this on a command line
            for(ip = 0, cntThi = 0; ip < svcPSList[is].dim(); ++ip) { // count how many are there of this kind
                if( strstr(svcPSList[is][ip].cmd + svcPSList[is][ip].args, cmdline) )
                    ++cntThi;
            }
            cntKinds.vadd(1, cntThi);
        }
        numHost = hostNumInPool(&svcList[is], &cntHosts, &maxJobOnThisMachine);  // get the host numeral in the pool of hosts

        // TODO: implement limited launch attempts for broken jobs and emailing
        //idx maxAllowedAttemtps=svcAttribs[is].maxJobs*(60000/sleepTime) *(cntKinds.dim()>1 ? cntKinds.dim()-1 : 1 );

        //
        // at this point we need to check if there are enough jobs running for this particular service
        //
        for(idx ib = 0, ifrst = 1; ib < (int) cntKinds.dim(); ++ib) {
            // make sure this job needs to be launched
            if( cntKinds[ib] >= maxJobOnThisMachine )
                continue;
            if( ib == 0 && svcList[is].parallelJobs > 1 )
                continue; // 0-th element is read only for non parallel jobs
            if( ib > 0 ) {
                if( !((svcList[is].isUp) & (((idx) 1) << (ib - 1))) )
                    continue; // this particular parallel job is down
                if( (int) (numHost % (svcList[is].parallelJobs)) != (int) ((ib - 1) % cntHosts) )
                    continue; // check if this host is not carrying this parallel job
            }

            if( ifrst ) {
                resourceSync(resourceRoot, svcList[is].name, vars.value("os"));
                ifrst = 0;
            }

            //
            // prepare the command line
            //
            replaceCommandLineArgs(cmdline, &tmp, svcList[is], resourceRoot, user);
            /*{
                // check if the os specific resource exists
                idx pos = cmdline.length();
                cmdline.printf(".os%s", vars.value("os"));
                sFil fl(cmdline.ptr(), sMex::fReadonly);
                if( fl.length() == 0 )
                    cmdline.cut(pos); // the file doesn't exist , cut it back to non-os specific
            }*/
            if( ib > 0 ) {
                cmdline.printf(" jobarr %"DEC" ", ib - 1 + 1);
            }
            if( ps.getMode() == sPS::eExec_Extern ) {
                cmdline.printf(" psman \"%s\" ", ps.getScript());
            }
            // add extra spaces to enlarge argv buffer for psMessage function
            static char psMessagePlaceHolder[64];
            static const char * dummy = sSet(psMessagePlaceHolder, ' ', sizeof(psMessagePlaceHolder));
            cmdline.printf("%s stdin \"0%.*s\" ", tmp.ptr(), (int)(sizeof(psMessagePlaceHolder)), dummy); // dummy used to avert warning
            if( loggingOn ) {
                cmdline.printf(" > $(/tmp/)qp-%s-", svcList[is].name);
            }
            tmp.cut(0);
            replVar00(&tmp, cmdline.ptr(), cmdline.length());
            tmp.shrink00();
            const idx log_pos = tmp.length();

            logOut(eQPLogType_Info, "queue on %s for service %s: reserve %"DEC" max %"DEC" reqs %"DEC" cnt %"DEC" all %"DEC"\n", thisHost, svcList[is].name,
                svcList[is].activeJobReserve, maxJobOnThisMachine, svcList[is].hasReqToGrab,
                cntKinds[ib], cntThisKindOnAllMachines);
            for(idx il = 0; cntKinds[ib] < maxJobOnThisMachine &&
                               ( (svcList[is].hasReqToGrab && cntKinds[ib] < svcList[is].hasReqToGrab )
                                   ||
                                  (cntThisKindOnAllMachines < svcList[is].activeJobReserve && cntKinds[ib] < svcList[is].activeJobReserve)
                                  ) ; ++cntKinds[ib], ++cntThisKindOnAllMachines, ++il) { //
                if( il ) {
                    if( svcList[is].delayLaunchSec < 0 ) { // if negative the next job of this kind will be launched during the next sync
                        break;
                    }
                    if( svcList[is].delayLaunchSec != 0 ) { // sleep between launches
                        sleepMS(svcList[is].delayLaunchSec * 1000);
                    }
                }
                if( loggingOn ) {
                    tmp.printf(log_pos, "%u.log 2>&1", rand());
                }
                logOut(eQPLogType_Info, " launching job for service %s (capacity %f) on %s as %s\n", svcList[is].name, svcList[is].capacity, thisHost, tmp.ptr());
                logOut(eQPLogType_Info, " there are %"DEC" pending requests and %"DEC" active reserve inquiries\n", svcList[is].hasReqToGrab,svcList[is].activeJobReserve);
                ps.exec(tmp);
                currCapacity += svcList[is].capacity;

                if( hostCapacity && (svcList[is].capacity + currCapacity) >= hostCapacity ) {
                    // host has no more capacity left for this service
                    break;
                }
            }
        }
    }
#if _DEBUG
    if( hostCapacity ) {
        logOut(eQPLogType_Info, "Host capacity used %f of %f\n", currCapacity, hostCapacity);
    }
#endif
    return true;

}

bool sQPrideSrv::OnCommandEmail(const char *, const char *)
{

    sUsr usr("qpride", true);
    if( usr.Id() <= 0 ) {
        logOut(eQPLogType_Info, "qpride service account inaccessible\n");
        return (-1);
    }
    idx smtpPort = 25;
    sStr smtpSrv, emailFrom;
    cfgStr(&smtpSrv, 0, "smtpSrv");
    cfgStr(&emailFrom, 0, "emailAddr");
    if( !smtpSrv || !emailFrom ) {
        logOut(eQPLogType_Info, "smtpSrv and/or emailAddr not found in db table QPCfg\n");
        return (-1);
    }

    sUsrObjRes emails;
    usr.objs2("email", emails, 0, "!sent", 0, 0, false, 0, emailBatch);
    udx sent = 0, failed = 0;

    for(sUsrObjRes::IdIter it = emails.first(); emails.has(it); emails.next(it)) {
        sUsrEmail email(usr, *emails.id(it));
        /* TODO check here if last failed attempt was more than a day ago in case something was down somewhere */
        if( !email.isSent() && email.retries() < 2 ) {
            sStr subj("%s", email.subject());
            sStr body("%s", email.body());
            sStr rcpt, to, cc;
            sVarSet lst;
            email.to(lst);
            idx qto = lst.rows;
            email.cc(lst);
            idx qcc = lst.rows;
            email.bcc(lst);
            for(idx r = 0; r < lst.rows; ++r) {
                rcpt.printf(",%s", lst.val(r, 0));
                if( r < qto ) {
                    to.printf(",%s", lst.val(r, 0));
                }
                if( r >= qto && r < qcc ) {
                    cc.printf(",%s", lst.val(r, 0));
                }
            }
            sStr response;
            idx errcode = sConClient::sendMail(&response, smtpSrv.ptr(), smtpPort, emailFrom.ptr(), rcpt ? rcpt.ptr(1) : 0, to ? to.ptr(1) : 0, cc ? cc.ptr(1) : 0, subj.ptr(), body.ptr());
            email.errmsg(response);
            email.sent(time(0));
            if( errcode >= 400 ) {
                email.isSent(false);
                email.retries(email.retries() + 1);
                ++failed;
            } else {
                email.isSent(true);
                ++sent;
            }
        }
    }
    if( sent || failed ) {
        logOut(eQPLogType_Info, " emails: %"UDEC" sent, %"UDEC" failed\n", sent, failed);
    }
    return true;
}

// most probably this function should be in storage manager
void sQPrideSrv::purgeDir(TPurgeData & data)
{
    if( data.cleanUpDays || (data.masks00 && data.masks00[0]) ) {
        sStr plog, mlog;
        idx cleanUpSecs = data.cleanUpDays * 24 * 60 * 60;
        const idx now = cleanUpSecs ? sTime::systime() : 0;
        for(const char * mask = data.masks00; mask; mask = sString::next00(mask)) {
            if( *mask ) {
                mlog.printf("/%s", mask);
            }
        }
        for(const char * path = data.path00; path; path = sString::next00(path)) {
            if( !*path ) {
                continue;
            }
            plog.printf(",%s", path);
            sDir files;
            files.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs), path, "*");
            for(const char * pnm = files.ptr(); pnm; pnm = sString::next00(pnm)) {
                if( !*pnm ) {
                    continue;
                }
                sStr remove;
                if( data.masks00 && data.masks00[0] ) {
                    const char * nm = sFilePath::nextToSlash(pnm);
                    for(const char * mask = data.masks00; mask; mask = sString::next00(mask)) {
                        if( *mask ) {
                            if( data.reqs.dim() ) {
                                sStr freq;
                                sString::searchAndReplaceStrings(&freq, mask, 0, "@reqID@"__, "%"DEC""__, 0, true);
                                idx req = 0;
                                if( sscanf(nm, freq, &req) == 1 && data.reqs.get(&req, sizeof(req)) ) {
                                    remove.printf("req %"DEC, req);
                                }
                            }
                            if( !remove && data.objs.dim() ) {
                                // TODO : allow purging non-domain-0 object storage
                                sStr fobj;
                                sString::searchAndReplaceStrings(&fobj, mask, 0, "@objID@"__, "%"UDEC""__, 0, true);
                                udx obj = 0;
                                if( sscanf(nm, fobj, &obj) == 1 && data.objs.get(&obj, sizeof(obj)) ) {
                                    remove.printf("obj %"UDEC, obj);
                                }
                            }
                        }
                    }
                }
                if( !remove && cleanUpSecs ) {
                    idx tm = sFile::atime(pnm);
                    if( (now - tm) > cleanUpSecs ) {
                        remove.printf("time %"DEC" - %"DEC, now, tm);
                    }
                }
                if( remove ) {
#if _DEBUG
                    bool done = true;
#else
                    bool done = sFile::remove(pnm) || sDir::removeDir(pnm, true);
#endif
                    if( done ) {
                        logOut(eQPLogType_Info, "removed by %s '%s'\n", remove.ptr(), pnm);
                        data.size += sDir::exists(pnm) ? sDir::size(pnm) : sFile::size(pnm);
                    }
                }
            }
        }
        logOut(eQPLogType_Info, "purging '%s': %"DEC" days, masks: '%s%s'\n", plog ? plog.ptr(1) : "", data.cleanUpDays, mlog ? mlog.ptr() : "", mlog ? "/" : "");
    }
}

idx sQPrideSrv::doKillProcesses(idx * ppids, idx cnt)
{
    for(idx i = 0; i < cnt; ++i) {
        ps.killProcess(ppids[i], -9);
    }
    return cnt;
}

bool sQPrideSrv::OnCommandCapacity(const char *, const char *)
{
    if( m_nodeMan ) {
        idx total = 0;
        idx need = sysCapacityNeed(&total);
        if( need > total ) {
            sStr cmd("%s -add %"DEC, m_nodeMan.ptr(), need - total);
            const idx ret = sPS::execute(cmd);
            if( ret != 0 ) {
                logOut(eQPLogType_Error, "Node man call '%s' returned %"DEC"\n", cmd.ptr(), ret);
            }
        }
    }
    return m_nodeMan;
}
