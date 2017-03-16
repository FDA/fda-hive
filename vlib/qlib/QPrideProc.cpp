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
#include <slib/std/online.hpp>
#include <qlib/QPrideProc.hpp>
#include <ulib/uobj.hpp>
#include <ulib/ufile.hpp>

using namespace slib;

/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  Initialization
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */

sQPrideProc::sQPrideProc(const char * defline00, const char * service)
    : sQPride(defline00, service)
{
    if( !ok ) {
        return;
    }
    alwaysRun = false;

    loopCnt = 0;
    noGrabs = 0;
    maxMemSize = 0;
    exitCode = eQPErr_None;
    inBundle = 0;
    socketSelect = 0;
    prvGrabReqID = -1;
    isProblemReported = 0;
    sleepTimeOverride = 0;
    lastInGroup = false;

#ifdef SLIB_WIN
    doStdin = 0; // windows doesn't select() on stdin
#endif

    doStdin = 1;
    tempDBConnection = false;
    maxmemMailSent = false;

    _argvBuf = 0;
    _argvBufLen = 0;
    _argvBufChanged = false;

    if( strcmp(service, "data_cache") == 0 )
        return;

    tmfix.time(); // set the initial clock
    if( !serviceGet(&svc, 0, 0) ) {
        logOut(eQPLogType_Fatal, "Service '%s' is not operational\n", service);
        return;
    }

    threadsCnt = sAbs(svc.runInMT);
    initializeTriggerPorts();
}

sQPrideProc::~sQPrideProc()
{
    if( sQPrideBase::user ) {
        delete sQPrideBase::user;
    }
}


/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  Running
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */


void sQPrideProc::executeRequest(void * procthis)
{
    sQPrideProc *pc = (sQPrideProc*) procthis;
    pc->tmExec.time();
    sStr log("==> request %"DEC" (%"DEC", %"DEC") user %"UDEC, pc->reqId, pc->masterId, pc->grpId, pc->user ? pc->user->Id() : 0);
/*    if( pc->objs.dim() ) {
        log.printf(" proc obj[0] %s", pc->objs[0].Id().print());
        for(idx io = 0; io < pc->objs.dim(); ++io) {
            pc->objs[io].propSync();
        }
    }*/
    pc->logOut(eQPLogType_Trace, "%s started\n", log.ptr());
    pc->reqGetData(pc->reqId,"_reqStage", &pc->requestStage, true);
    if( !pc->requestStage.length() ) {
        pc->requestStage.add("init", 5);
    }
    pc->exitCode = pc->OnExecute(pc->reqId);
    idx status = pc->reqGetStatus(pc->reqId);
    pc->logOut(eQPLogType_Info, "%s finished, returned %"DEC", status %"DEC"\n", log.ptr(), pc->exitCode, status);
    ++pc->loopCnt;
    pc->reqId = 0;
    if (pc->svc.runInMT > 1) { // TODO  wake my checking thread
        pc->messageSubmit(pc->vars.value("thisHostName"), pc->svc.svcID, false, "wake");
    }
    if( pc->isLastInMasterGroup() ) {
        pc->OnCollect(pc->reqId);
    }
    if( pc->isLastInGroup() ) {
        pc->OnFinalize(pc->reqId);
    }
    // synchronize object when a request is finished to provide recent meta data
    if( status > eQPReqStatus_Running || pc->lastInGroup || pc->isLastInGroup() ) {
        for(idx io = 0; io < pc->objs.dim(); ++io) {
            pc->objs[io].propSync();
        }
    }
}

/*
void sQPrideProc::executeAThread(void * param)
{
    ThreadSpecific * tr=(ThreadSpecific *)param;
    sQPrideProc * app=(sQPrideProc *)tr->thisObj;
    app->OnComputeThread(app->reqId,tr);
    return ;
}


idx sQPrideProc::executeThreads(idx , idx rangeCnt, idx rangeStart) //req
{
    idx myError=0;
    threadsWorking=0;threadID=0; // to start from zero
    parSplitForJob(rangeStart,rangeCnt, inBundle, &svc); // determine the boundaries for the whole job, not just this thread
    for( idx i=1; i<threadsCnt; ++i) {  // launch parallel threads
        ThreadSpecific * t=parSplitForThread(rangeStart,rangeCnt,inBundle, parGetThreadID(), &svc); // determine the thread specific parameters for this thread, this job
        threadBegin(executeAThread,(void*)t,myError);
        sleepSeconds(1); /// wait so threads do not conflict during initialization
    }
    executeAThread(parSplitForThread(rangeStart,rangeCnt,inBundle,parGetThreadID(),&svc)); // computation by self

    //
    // wait until all threads have finished
    //
    while(threadsWorking>0){
        sleepSeconds(1);
    }
    return 0;
}


idx sQPrideProc::OnExecute(idx req)
{
    // group id is needed since some data may be associated not with the request but with the MASTER group
    grpId = req2Grp(req);
    masterId = req2Grp(req, 0, true);
    idx myError=OnPrepare(req) ;

    if(myError==0){
        //bool isCollector=strstr(vars.value("serviceName"), "-collector")!=0 ? true : false;

        //if( isCollector ){
        //    myError=OnCollect(req);
        //} else {
            myError=OnCompute(req);
        //}

        //if( myError==0 && !isCollector && svc.parallelJobs>1 && parReqsStillIncomplete(grpId) ==1){
        //    idx reqCollect=grp2Req(grpId,0,"-collector");
        //    reqSetAction(reqCollect,eQPReqAction_Run);
        //}
        if(svc.parallelJobs>1 && myError==0 && req==masterId  ){
            logOut(eQPLogType_Trace,"Waiting for other parallel jobs to finish\n");
            while ( parReqsStillIncomplete(masterId) >0 )
                sleepMS(1000);
            OnCollect(req);
        }

    }

    OnCleanup(req);

    return 0;
}
*/

idx sQPrideProc::OnGrab(idx forceReq)
{
    isRegrab = 0;

    // we grab the requests to execute
    if( forceReq ) {
        reqId = forceReq;
        isRegrab = 1;
        reqSetStatus(reqId, eQPReqStatus_Processing);
    } else {
        reqId = reqGrab(0, jobId, inBundle, eQPReqStatus_Waiting, eQPReqAction_Run);
    }

    lazyTime.time(); // since Grabber always resets the alive time counter, we make sure our lazy time is in consistency
    if( !user ) {
        user = new sUsr();
    }
    // has request and switched to the submitting user
    if( reqId != 0 && user->init(reqGetUser(reqId)) ) {
        psMessage("req %"DEC, reqId);
        grpId = req2Grp(reqId); // group id is needed since some data may be associated not with the request but with the group
        masterId = req2Grp(reqId, 0, true);
        reqSliceCnt = 1;
        reqSliceId = req2GrpSerial(reqId, masterId, &reqSliceCnt, svc.svcID) - 1;
        if( reqSliceId < 0 ) {
            reqSliceId = 0;
        }
        if( !reqSliceCnt ) {
            reqSliceCnt = 1;
        }
        reqForm.empty(); // ensure reqForm doesn't contain values from previous grabs
        pForm = reqGetData(masterId, "formT.qpride", &reqForm);
        // used by outProgress Function
        progress100Start = 0;
        progress100End = 100;
        progress100Last = 0;
        objs.cut(0);
        if( user ) { // map the defined objects for this request to sUsrObjects
            sStr strObjList;
            sVec<sHiveId> objIds;
            requestGetPar(grpId, eQPReqPar_Objects, &strObjList);

            if( strObjList )
                sHiveId::parseRangeSet(objIds, strObjList, strObjList.length());

            for(idx io = 0; io < objIds.dim(); ++io) {
                idx cnt = objs.dim();
                //user->objFactory(objIds[io])
                sUsrProc * p = objs.add(1);
                new (p) sUsrProc(*user, objIds[io]);
                if( !objs[cnt].Id() ) {
                    objs.cut(cnt);
                }
            }
            sStr rootPath;
            sUsrObj::initStorage(cfgStr(&rootPath, 0, "user.rootStoreManager"), cfgInt(0, "user.storageMinKeepFree", (udx) 20 * 1024 * 1024 * 1024));
        }
        reqSetStatus(reqId, eQPReqStatus_Running);
        prvReqId = reqId;
        rand_seed = pForm->ivalue("rand_seed", time(0)) + reqSliceId;
        srand(rand_seed);
        if( svc.runInMT > 1 ) {
            idx code;
            threadBegin(executeRequest, (void* )this, code);
        } else {
            executeRequest((void *) this);
            reqId = 0;
        }
        user->init(); //forget user
        prvGrabReqID = reqId;
    } else {
        if( !promptOK ) {
            prvGrabReqID = reqId;
            logOut(eQPLogType_Trace, "No available requests to perform\n");
        }
        psMessage("sleeping...");
        return 0;
    }
    psMessage("sleeping...");
    return prvReqId;
}

void sQPrideProc::psMessage(const char * fmt, ...)
{
    if( _argvBuf && _argvBufLen ) {
#ifdef WIN32
        // not tested or verified
        /*
        #include <Windows.h>
        #include <Winternl.h> // for PROCESS_BASIC_INFORMATION and ProcessBasicInformation
        #include <stdio.h>
        #include <tchar.h>

        typedef NTSTATUS (NTAPI *PFN_NT_QUERY_INFORMATION_PROCESS) (
            IN HANDLE ProcessHandle,
            IN PROCESSINFOCLASS ProcessInformationClass,
            OUT PVOID ProcessInformation,
            IN ULONG ProcessInformationLength,
            OUT PULONG ReturnLength OPTIONAL);

        int main()
        {
            HANDLE hProcess = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                           FALSE, GetCurrentProcessId());
            PROCESS_BASIC_INFORMATION pbi;
            ULONG ReturnLength;
            PFN_NT_QUERY_INFORMATION_PROCESS pfnNtQueryInformationProcess =
                (PFN_NT_QUERY_INFORMATION_PROCESS) GetProcAddress (
                    GetModuleHandle(TEXT("ntdll.dll")), "NtQueryInformationProcess");
            NTSTATUS status = pfnNtQueryInformationProcess (
                hProcess, ProcessBasicInformation,
                (PVOID)&pbi, sizeof(pbi), &ReturnLength);
            // remove full information about my command line
            pbi.PebBaseAddress->ProcessParameters->CommandLine.Length = 0;

            getchar(); // wait till we can verify the results
            return 0;
        }*/
#else
        static sStr buf(sMex::fExactSize);
        buf.cut(0);
        sCallVarg(buf.vprintf, fmt);
        if( buf.length() > _argvBufLen ) {
            buf.cut(_argvBufLen - 3);
            buf.printf("...");
        } else {
            const idx tail = _argvBufLen - buf.length();
            if( tail ) {
                const idx pos = buf.length();
                buf.resize(_argvBufLen);
                memset(buf.ptr(pos), ' ', tail); // clean tail of previous output
            }
        }
        memcpy(_argvBuf, buf.ptr(), _argvBufLen);
        _argvBufChanged = true;
#endif
    }
}

idx sQPrideProc::run(idx argc, const char *argv[])
{
    if (!ok) {
        ///printf("FATAL ERROR: cannot connect to QueueManager\n");
        return 0;
    }
    // process all arguments
    for(idx i = 1; i + 1 < argc; ++i) {
        ok = executeCommand(argv[i], argv[i + 1]);
        ++i;
        if( !ok ) {
            break;
        }
    }
    if( doStdin == 0 && argc > 0 ) {
        // only works in "stdin 0" mode
        // can't overwrite argv[0] - ps will break
        _argvBuf = const_cast<char *>(argv[1]);
        for(idx i = 1; i < argc; ++i) {
            _argvBufLen += strlen(argv[i]) + 1;
        }
        _argvBufLen--; // leave space for last \0 from printf
    }
    sStr tmpB;
    cfgStr(&tmpB,0,"qm.domains","");
    char * domains00=vars.inp("domains",tmpB.ptr());
    sString::searchAndReplaceSymbols(domains00,0, "/", 0, 0, true,true,true,true);
    const char* thisHostName=vars.value("thisHostName");
    /* for(char * qmThisDomain=domains00; qmThisDomain && *qmThisDomain; qmThisDomain=sString::next00(qmThisDomain), ++inDomain){
        sStr td; td.printf("qm.domains_%s",qmThisDomain);
        // get the list of workstations in this domain
        tmpB.cut(0);cfgStr(&tmpB,0,td,""); sString::searchAndReplaceSymbols(tmpB.ptr(),0, "/", 0, 0, true,true,true,true);

        if( sString::compareChoice( thisHostName, tmpB.ptr() ,&thisHostNumInDomain,true, 0, true) !=-1) {
            vars.inp("thisDomain", tmpB.ptr(0), tmpB.length());
            isDomainFound=true;
            break;
        }
    }
    if(!isDomainFound){
        logOut(eQPLogType_Fatal, "Domain configuration error: this host doesn't belong to domain. Cannot continue!\n" );
        ok=false;
        return 0;
    }
    */

    for(char * qmThisDomain=domains00; qmThisDomain && *qmThisDomain; qmThisDomain=sString::next00(qmThisDomain), ++inDomain){
        sStr td; td.printf("qm.domains_%s",qmThisDomain);
        // get the list of workstations in this domain
        tmpB.cut(0);cfgStr(&tmpB,0,td,""); sString::searchAndReplaceSymbols(tmpB.ptr(),0, "/", 0, 0, true,true,true,true);

        idx insideDomain=0;
        for( char * rp=tmpB.ptr(); rp; rp=sString::next00(rp), ++insideDomain) {

            isDomainFound=hostNameMatch(rp, thisHostName);

            if(isDomainFound) {
                //vars.inp("thisDomain", rp);
                vars.inp("thisDomain", tmpB.ptr(0), tmpB.length());
                vars.inp("thisDomainHostMatch", rp);
                thisHostNumInDomain=insideDomain;
                break;
            }
        }
        if(isDomainFound)
            break;
    }
    if(!isDomainFound) {

        logOut(eQPLogType_Fatal, "Domain configuration error: host %s doesn't belong to domain. Cannot continue!\n" , thisHostName );
        ok=false;
        return 0;
    }



    makeVar00();

    ok = OnInit();
    if (!ok) {
        logOut(eQPLogType_Fatal, "%s", "Cannot initialize the process.");
        return 1;
    }

    if (!alwaysRun && svc.isUp == 0) {
        logOut(eQPLogType_Warning,
                "Service has been stopped: this job quits.\nEither start the service or use 'force 1' in command line\n");
        if(jobId)jobSetStatus(jobId, eQPJobStatus_ExitNormal);
        return eQPErr_ServiceStopped;
    }
    if (!jobShouldRun()) {
        if(svc.svcID>1)logOut(eQPLogType_Warning, "Enough jobs running: this job quits.\n");
        if(jobId)jobSetStatus(jobId, eQPJobStatus_ExitNormal);
        return eQPErr_TooManyJobs;
    }

    const char * svcName=vars.value("serviceName");
    jobId = jobRegister(svcName, vars.value("thisHostName"), vars.ivalue("pid"), inBundle);

    logOut(eQPLogType_Trace, "Starting service %s\n", svcName);
    jobSetStatus(jobId, eQPJobStatus_Running);
    maxMemSize = ps.getMem(pid);
    jobSetMem(jobId, maxMemSize, maxMemSize);

    // run the main cycle
    for (loopCnt = 0; loopCnt < svc.maxLoops && exitCode == eQPErr_None;) {

        if( reqId != 0 ) { // if we have a working thread running - we do not try to grab more
            selectSleep(sleepTimeOverride);
            if( loopCnt >= svc.maxLoops && reqId != 0 ) {
                logOut(eQPLogType_Info, "Exit requested. Waiting for %"DEC" to finish\n", reqId);
                loopCnt = svc.maxLoops - 1;
            }
            if( memReport(svcName) ) {
                break;
            }
            //sTim_Global= tmfix.time();
            continue;
        }

        if (!dbHasLiveConnection()) { // hot switch to live connection from the pool of DBs
            dbDisconnect();
            ok = dbReconnect();
            if (ok) {
                if (tempDBConnection == false)
                    logOut(eQPLogType_Warning,"Restoring the connection... successfully\n");
            } else {
                logOut(eQPLogType_Fatal,"Exiting ... no live connection available\n");
                jobSetStatus(jobId, eQPJobStatus_ExitError);
                return eQPErr_DB_LostConnection;
            }
        }

        // exit politely if asked to do so.
        // we do polite exits if we have no running requests
        idx act;
        act=jobGetAction(jobId);
        if (act == eQPJobAction_Kill) {
            logOut(eQPLogType_Trace,"Exiting politely ... according to action required for this job\n");
            break;
        }

        idx tmdiff = tmfix.time();
        if (tmdiff > ((idx) (svc.cleanUpDays) * 24 * 3600 - 2 * 3600)) { // if less than two hours are left for this job to expire
            logOut(eQPLogType_Trace, "Exiting politely ... according to timespan allowed for this job\n");
            break;
        }

        if (!alwaysRun) {
            serviceGet( &svc, 0, 0);
            if ((svc.parallelJobs <= 1 && svc.isUp == 0) || (svc.parallelJobs > 1 && ((svc.isUp & ( ((idx)1) << (inBundle - 1))) == 0))) {
                logOut( eQPLogType_Trace, "Exiting politely ... the service '%s' has been stopped\n", vars.value("serviceName"));
                break;
            }
        }

        if (!OnGrab()) {
            ++noGrabs;
            if (svc.noGrabExit && noGrabs >= svc.noGrabExit) {
                logOut(eQPLogType_Trace, "Nothing to grab after %"DEC" attempts ... exiting temporarily\n", noGrabs);
                break;
            }
            if (svc.noGrabDisconnect && noGrabs >= svc.noGrabDisconnect) {
                if (tempDBConnection == false)
                    logOut(eQPLogType_Trace, "Nothing to grab after %"DEC" attempts ... closing the DB connection temporarily\n", noGrabs);
                tempDBConnection = true;
                dbDisconnect();
            }
            selectSleep(sleepTimeOverride);
            //logOut(eQPLogType_Trace, "Nothing to grab \n");

        } else {
            if( memReport(svcName) ) {
                break;
            }
            noGrabs = 0;
            tempDBConnection = false;
        }

        if(!jobShouldRun())
            loopCnt =svc.maxLoops+1;
        //if( OnWatch() ) {
        //    jobRegisterAlive(jobId,svc.lazyReportSec);
        //}

        if (svc.maxLoops && loopCnt >= svc.maxLoops ) {
            logOut(eQPLogType_Trace, "Exiting after executing %"DEC" requests \n", loopCnt);
            break;
        }
        flushCache();
    }

    logOut(eQPLogType_Trace, "Exiting legally\n");
    jobSetStatus(jobId, eQPJobStatus_ExitNormal);

    OnQuit();

    return exitCode;
}

bool sQPrideProc::memReport(const char * svcName)
{
    sPS psLocal;
    idx mem = psLocal.getMem(pid);
    if( maxMemSize < mem ) {
        maxMemSize = mem;
    }
    jobSetMem(jobId, mem, maxMemSize);
    if( svc.maxmemSoft != 0 && mem > svc.maxmemSoft && maxmemMailSent == false ) {
        logOut(eQPLogType_Warning, "While executing %"DEC" service '%s' used %"DEC"MB of memory (Soft=%"DEC", Hard=%"DEC")\n",
            prvReqId, svcName, mem / (1024 * 1024), svc.maxmemSoft / (1024 * 1024), svc.maxmemHard / (1024 * 1024));
        maxmemMailSent = true;
    }
    if( svc.maxmemHard != 0 && mem > svc.maxmemHard ) {
        logOut(eQPLogType_Fatal, "While executing %"DEC" service '%s' used %"DEC"MB of memory (Soft=%"DEC", Hard=%"DEC"). Hard limit reached: exiting...\n",
            prvReqId, svcName, mem / (1024 * 1024), svc.maxmemSoft / (1024 * 1024), svc.maxmemHard / (1024 * 1024));
        jobSetStatus(jobId, eQPJobStatus_ExitError);
        return true;
    }
    return false;
}

/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/QmSetJ
 _/  Commands
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */

idx sQPrideProc::selectSleep(idx slpTm)
{
    if (!slpTm)
        slpTm = svc.sleepTime;
    idx chunk=3000, len=0;
    idx cntSleep=slpTm/chunk;
    for( idx ic=0 ; ic<cntSleep; ++ic ) {
        len=selectSleepSingle(chunk);
        if(len)break;
    }
    return len;
}
idx sQPrideProc::selectSleepSingle(idx slpTm)
{
    // output the prompt
    if (!promptOK) {
        promptOK = 1;
        time_t tt;
        time(&tt);
        struct tm & t = *localtime(&tt);
        printf("\n%"DEC"/%"DEC"/%"DEC" %"DEC":%"DEC":%"DEC" %"DEC"/%"DEC" %s > ", (idx)t.tm_mday, (idx)t.tm_mon + 1,
                (idx)t.tm_year + 1900, (idx)t.tm_hour, (idx)t.tm_min, (idx)t.tm_sec, (idx)jobId, (idx)pid,
                vars.value("serviceName"));

        fflush(0);
    }

    fd_set rfds;
    idx retval = 0;
    //static idx non_sel = 0;
    idx len = 0;

    if (!slpTm)
        slpTm = svc.sleepTime;
    initializeTriggerPorts(); // trying to reinitialize the triggers

    #ifdef ZZZ_WIN32
        struct _SECURITY_ATTRIBUTES secat;
        secat.nLength = sizeof (struct _SECURITY_ATTRIBUTES);
        secat.lpSecurityDescriptor = NULL;
        secat.bInheritHandle = FALSE;
        HANDLE conin = CreateFile("CONIN$", GENERIC_READ, FILE_SHARE_READ, &secat, OPEN_EXISTING, 0, 0);

        HANDLE handle[2];
        handle[0]=conin;//STD_INPUT_HANDLE
        //handle[1]=socketSelect;

        //while (1){
        //cout <<"I start my loop...";
        //DWORD test = WaitForMultipleObjects(  1, handle,  TRUE,  slpTm);

        DWORD test = WaitForSingleObject(handle[0], slpTm);
        if( test == WAIT_OBJECT_0 )
        {
            printf("WAIT_OBJECT_0");
            INPUT_RECORD r[512];
            DWORD read;
            ReadConsoleInput(handle[0], r, 512, &read);
            printf("Read: %d", read);
        }

        if (test ==WAIT_TIMEOUT) printf( " ...Timeout!\n");
        else if (test ==WAIT_FAILED) printf("error!\n");

        retval= (idx)GetLastError();

        //if(infile!=-1)retval=WaitForSingleObject(stdin,slpTm);
    #else
        struct timeval tv;
        tv.tv_sec = (long) (slpTm / 1000);
        tv.tv_usec = (slpTm % 1000) * 1000;

        // if stdin ihas something, get it!
        FD_ZERO(&rfds);
        if (doStdin){
            FD_SET(0 , &rfds);
        }
        if (socketSelect) {
            FD_SET( ((unsigned int )socketSelect), &rfds);
        }

        retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);

    #endif

    char Buf[1024];Buf[0] = 0;

    if(retval) {

        if (socketSelect && FD_ISSET(socketSelect, &rfds)) {
            struct sockaddr_in from;
            socklen_t fromlen = sizeof(from);
            #if defined (SLIB64) && defined (WIN32)
                len = recvfrom(socketSelect, Buf, sizeof(Buf), 0, (struct sockaddr *) &from, (int*) &fromlen);
            #else
                len = recvfrom(socketSelect, Buf, sizeof(Buf), 0, (struct sockaddr *) &from, (socklen_t*) &fromlen);
            #endif
            Buf[len] = 0;
            fflush(0);
        } else if (doStdin)
            fgets(Buf, sizeof(Buf), stdin); //read from stdin
    }
    releaseTriggerPorts();
    if(!retval)return 0;
    len=sLen(Buf);

    sString::searchAndReplaceSymbols(Buf, 0, sString_symbolsBlank, 0, 0, true, true, false );
    if(sLen(Buf))
        executeCommand(Buf, Buf + sLen(Buf) + 1);

    return len;
}

bool sQPrideProc::executeCommand(const char * nam, const char * val)
{
#define jobIsCmd(_cmd) (!strcmp(nam,_cmd) || !strcmp(nam,"-"_cmd))

    if( !reqId && !dbHasLiveConnection() ) { // hot switch to live connection from the pool of DBs
        ok = dbReconnect();
        if( !ok ) {
            logOut(eQPLogType_Fatal, "Exiting ... no live connection available\n");
            return false;
        }
    }
    if( sIs("env-", val) ) {
        val = getenv(&val[4]);
        if( !val ) {
            val = "";
        }
    }
    if( OnCommand(nam, val) ) {
        return true;
    }
    if( strncmp(nam, "set", 3) == 0 ) {
        if( *val ) {
            vars.inp(nam + 3, val);
        }
    } else if( jobIsCmd("exit") || jobIsCmd("quit")) {
        //if(!alwaysRun)
        svc.maxLoops = 0;
        logOut(eQPLogType_Trace, "Exit requested.\n");
    } else if( jobIsCmd("shell")) {
        system(val);
    } else if( jobIsCmd("time")) {
        tmCount.time(); // set the initial clock
    } else if( jobIsCmd("sleep")) {
        sscanf(val, "%"DEC, &sleepTimeOverride);
    } else if( jobIsCmd("stdin")) {
        sscanf(val, "%"DEC, &doStdin);
    } else if( jobIsCmd("loops")) {
        sscanf(val, "%"DEC, &svc.maxLoops);
    } else if( jobIsCmd("force")) {
        alwaysRun = true;
    } else if ( jobIsCmd("logLevel") ) {
        idx log_level = 0;
        if( sscanf(val, "%"DEC, &log_level) == 1 ) {
            setupLog(true, log_level);
        }
    } else if( jobIsCmd("reqSliceId")) {
        sscanf(val, "%"DEC, &reqSliceId);
    } else if( jobIsCmd("reqSliceCnt")) {
        sscanf(val, "%"DEC, &reqSliceCnt);
    } else if( jobIsCmd("grab")) {
        sVec<idx> reqList;
        sString::scanRangeSet(val, 0, &reqList, 0, 0, 0);
        for(idx i = 0; i < reqList.dim(); ++i) {
            OnGrab(reqList[i]);
        }
        //sscanf(val, "%"DEC, &ival);
        //OnGrab(ival);
    } else if( jobIsCmd("trigger")) {

    } else if( jobIsCmd("daemon")) {

    } else if( jobIsCmd("wake")) {
        logOut(eQPLogType_Trace, "awaken\n");
    } else if( jobIsCmd("jobarr")) {
        sscanf(val, "%"DEC, &inBundle);
    } else if( jobIsCmd("start")) {
        serviceUp(0, sNotIdx );
        OnCommand("launch", 0);
    } else if( jobIsCmd("stop")) {
        serviceUp(0, 0);
        OnCommand("kill", 0);
    } else if( jobIsCmd("psman")) {
        if( !ps.setMode(sPS::eExec_Extern, val) ) {
            logOut(eQPLogType_Error, "External process manager location is not valid \"%s\"\n", val);
            return false;
        }
    } else if( jobIsCmd("oninit")) {
        OnInit();
    } else {
        logOut(eQPLogType_Warning, "Unrecognized Command  \"%s\"\n", nam);
        return false;
    }
    fflush(0);
    promptOK = 0;
    return true;
}

/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  Utilities
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */

const char * sQPrideProc::formValue(const char * prop, sStr * lbuf, const char * defaultValue, idx iObj)
{
    sStr * buf = lbuf;
    if( !buf ) {
        static sStr staticFormValueBuf;
        buf = &staticFormValueBuf;
        staticFormValueBuf.cut(0);
    }
    if( reqGetData(grpId, prop, buf->mex()) ) {
        return buf->ptr();
    }
    const char * propVal = 0;
    if( objs.dim() > iObj ) {
        propVal = objs[iObj].propGet00(prop, buf, "\n");
    }
    if( !propVal ) {
        idx propValLen = 0;
        propVal = pForm->value(prop, defaultValue, &propValLen);
        if( propVal && lbuf ) {
            // printf imitation
            lbuf->add(propVal, propValLen);
            lbuf->add0();
            lbuf->cut(-1);
        }
    }
    if( !propVal ) {
        if( lbuf && defaultValue ) {
            propVal = lbuf->printf("%s", defaultValue);
        } else {
            propVal = defaultValue;
        }
    }
    return propVal;
}

const char * sQPrideProc::formValues00(const char *prop, sStr *buf00, const char * altSeparator, idx iObj)
{
    if (iObj >= objs.dim())
        return NULL;

    return ((sUsrObj&)(objs[iObj])).propGet00(prop, buf00, altSeparator );
}

idx sQPrideProc::formIValues(const char *prop, sVec<idx> *values, idx iObj)
{
    sStr buf00;
    idx ret = -1;
    if (formValues00(prop, &buf00, 0, iObj))
        for (const char *p = buf00.ptr(); p; p = sString::next00(p), ret++)
            sscanf(p, "%"DEC, values->add(1));
    return ret;
}

idx sQPrideProc::formUValues(const char *prop, sVec<udx> *values, idx iObj)
{
    sStr buf00;
    idx ret = -1;
    if (formValues00(prop, &buf00, 0, iObj))
        for (const char *p = buf00.ptr(); p; p = sString::next00(p), ret++)
            sscanf(p, "%"UDEC, values->add(1));
    return ret;
}

idx sQPrideProc::formHiveIdValues(const char *prop, sVec<sHiveId> *values, idx iObj)
{
    sStr buf00;
    idx ret = -1;
    if (formValues00(prop, &buf00, 0, iObj))
        for (const char *p = buf00.ptr(); p; p = sString::next00(p), ret++)
            values->add(1)->parse(p);
    return ret;
}

idx sQPrideProc::formRValues(const char *prop, sVec<real> *values, idx iObj)
{
    sStr buf00;
    idx ret = -1;
    if (formValues00(prop, &buf00, 0, iObj))
        for (const char *p = buf00.ptr(); p; p = sString::next00(p), ret++)
            sscanf(p, "%lf", values->add(1));
    return ret;
}

const char * sQPrideProc::reqAddFile(sStr & buf, const char * flnmFmt, ...)
{
    sStr pathVariableName("file://");
    sCallVarg(pathVariableName.vprintf, flnmFmt);
    const char * flnm = pathVariableName.ptr(7);

    const idx pos = buf.pos();
    if( objs.dim() && !sIs("req-", flnm) && !sIs("reqself-", flnm) && !sIs("reqgrp-", flnm) ) {
        if( !objs[0].addFilePathname(buf, true, "%s", flnm) ) {
            return 0;
        }
    }
    if( pos == buf.pos() ) {
        idx dataReq = (grpId && !sIs("req-", flnm)) ? grpId : reqId;
        if( sIs("reqself-", flnm) ) {
            dataReq = reqId;
        }
        // create and empty file for this request
        if( reqSetData(dataReq, pathVariableName, 0, 0) ) {
            reqDataPath(dataReq, flnm, &buf);
        }
    }
    return buf.pos() == pos ? 0 : buf.ptr(pos);
}

const char * sQPrideProc::reqGetFile(sStr & buf, const char * flnmFmt, ...)
{
    sStr flnm;
    sCallVarg(flnm.vprintf, flnmFmt);

    const idx pos = buf.pos();
    if( objs.dim() && (!sIs("req-", flnm) && !sIs("reqself-", flnm) && !sIs("reqgrp-", flnm)) ) {
        objs[0].getFilePathname(buf, "%s", flnm.ptr());
    }
    if( pos == buf.length() ) {
        idx dataReq = (grpId && !sIs("req-", flnm)) ? grpId : reqId;
        if( sIs("reqself-", flnm) ) {
            dataReq = reqId;
        }
        reqDataPath(dataReq, flnm, &buf);
    }
    return buf.pos() == pos ? 0 : buf.ptr(pos);
}

//! OBSOLETE DO NOT USE, use add/getFile instead
const char * sQPrideProc::destPath(sStr * buf, const char * flnmFmt, ... )
{
    sStr pathVariableName("file://");
    sCallVarg(pathVariableName.vprintf, flnmFmt);
    const char * flnm = pathVariableName.ptr(7);

    idx pos = buf->length();
    if( (!sIs("req-",flnm) && !sIs("reqself-",flnm) && !sIs("reqgrp-", flnm)) && objs.dim() ) {
        if( !objs[0].getFilePathname(*buf, "%s", flnm) ) {
            objs[0].addFilePathname(*buf, true, "%s", flnm);
        }
    }
    if( pos == buf->length() ) {
        idx dataReq = ((!sIs("req-",flnm)) && grpId ) ? grpId : reqId;
        if(sIs("reqself-",flnm))
            dataReq = reqId;
        reqSetData(dataReq, pathVariableName, 0, 0); // create and empty file for this request
        reqDataPath(dataReq, flnm, buf);
    }
    return buf->ptr(pos);
}

idx sQPrideProc::reqProgress(idx items, idx progress, idx progressMax)
{
    // update argv message only the first reqProgress() call, and every svc.lazyReportSec seconds afterwards
    bool abort_proc = false;
    if( reqId && _argvBuf && (!_argvBufChanged || jobRegisterAlive(0, 0, svc.lazyReportSec, true)) ) {
        abort_proc = !OnProgress(reqId);
        if( !abort_proc ) {
            const idx prcnt = progress2Percent(items, progress, progressMax);
            psMessage("req %"DEC" %"DEC"%%", reqId, prcnt);
        }
    }
    if( !abort_proc ) {
        return sQPrideBase::reqProgress(reqId, svc.lazyReportSec, items, progress, progressMax);
    }
    return 0; // mimics sQPrideBase::reqProgress on kill
}

idx sQPrideProc::reqProgressStatic(void * param, idx items, idx progress, idx progressMax)
{
    sQPrideProc * qp = static_cast<sQPrideProc*>(param);
    return qp ? qp->reqProgress(items, progress, progressMax) : 1;
}

idx sQPrideProc::reqProgressFSStatic(void * param, idx items)
{
    sQPrideProc * qp = static_cast<sQPrideProc*>(param);
    return qp ? qp->reqProgress(items, 0, -1) : 1;
}

idx sQPrideProc::exec(const char * cmdline, const char * input, const char * path, sIO * log, idx sleepSecForExec /*  = sNotIdx */)
{
    return sPipe::exeFS(log, cmdline, 0, sleepSecForExec ? reqProgressFSStatic : 0 , this, path, sleepSecForExec > 0 ? sleepSecForExec : svc.lazyReportSec);
}

bool sQPrideProc::isLastInGroup(const char * svcName )
{
    if(!svcName )
        svcName=vars.value("serviceName");
    sVec<idx> stat;
    grpGetStatus(grpId, &stat, svcName);
    lastInGroup=(getGroupCntStatusIs(grpId, eQPReqStatus_Done, &stat, sQPrideBase::eStatusEqual, svcName) >= stat.dim()-1) ? true : false ;
    return lastInGroup;
}

bool sQPrideProc::isLastInMasterGroup(const char * svcName)
{
    sVec<idx> stat;
    if(!svcName )
        svcName=vars.value("serviceName");

    grpGetStatus(masterId, &stat, svcName, masterId);
    idx cntis = 0;
    for(idx i = 0; i < stat.dim(); ++i) {
        cntis += (stat[i] == eQPReqStatus_Done ? 1 : 0);
    }
    return cntis == stat.dim() - 1;
}

/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  Utilities
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/




idx sQPrideProc::hostNumInPool(Service * svc, idx * pCntList, idx * pmaxjob)
{
    sStr hstlist;
    sString::searchAndReplaceSymbols(&hstlist, svc->hosts,0, "/", 0, 0, true ,true,true,true);
    const char * thisHost=vars.value("thisHostName");
    const char * thisDomain00=vars.value("thisDomain");


    if(pmaxjob)*pmaxjob=svc->maxJobs;

    idx countThisDomain=1, numHost=0;
    for(char *p=hstlist; p; p=sString::next00(p)) {
        if( hostNameMatch((char*)p, thisHost, pmaxjob))
            break;
    }

    if(!pCntList)
        return 0;

    for(char *p=hstlist; p; p=sString::next00(p)){

        for(const char * qmThisDomain=thisDomain00; qmThisDomain && *qmThisDomain; qmThisDomain=sString::next00(qmThisDomain)){
            if( hostNameMatch((char*)qmThisDomain, p, pmaxjob)){
                numHost=countThisDomain;
                break;
            }
        }
        ++countThisDomain;

/*
        char * col=strchr(p,':');
        if(col){*col=0;++col;}
        if( sString::compareChoice( p, thisDomain00 ,0, 0, true) ==-1)continue; // not even this domain
        if(strcmp(p,thisHost)==0)
            numHost=countThisDomain;

        if(col){
            if(strcmp(p,thisHost)==0 && pmaxjob)sscanf(col,"%"DEC,pmaxjob);
            p=col;
        }
*/

    }
    *pCntList=countThisDomain;
    return numHost;
}


bool sQPrideProc::jobShouldRun(void)
{
    if (alwaysRun)
        return true;

    // now determine the maximum number of jobs allowed on this computer
    idx maxnumJobs = svc.maxJobs;
    hostNumInPool(&svc, (idx*)0, &maxnumJobs);

    //idx bundle = svc.parallelJobs;
    //if (bundle < 2)
    //    bundle = 1;

    sFilePath Proc,buf,tmp;
    sString::searchAndReplaceStrings(&buf,svc.cmdLine,0, "$(svc)"__,svc.name,0,false);
    replVar00(&tmp,buf.ptr(),buf.length());
    char * proc=Proc.procNameFromCmdLine(tmp.ptr());

    sVec<sPS::Stat> pi;
    sStr t("jobarr %"DEC, inBundle);
    idx psRes = 0;
    if( strcmp(svc.name, "qm") == 0 ) {
        sPS lcl;
        psRes = lcl.getProcList(&pi, proc, true, inBundle ? t.ptr() : 0);
    } else {
        psRes = ps.getProcList(&pi, proc, true, inBundle ? t.ptr() : 0);
    }
    return (psRes < 0 || pi.dim() > maxnumJobs) ? false : true;
}

bool sQPrideProc::initializeTriggerPorts(void)
{
    if (!socketSelect) {
        idx port = qmBaseUDPPort + svc.svcID;
        socketSelect = udp.initUDP(port);
        if (socketSelect == 0) {
            if(isProblemReported==0) {
                logOut(eQPLogType_Info,"Cannot initiate server %s on UDP port %"DEC" in %s domain.\n", vars.value("thisHostName"), port,vars.value("thisDomain","undefined") );
            }
            ++isProblemReported;
        } else {
            //logOut(eQPLogType_Trace,"Initiated server %s UDP port %"DEC" in %s domain.\n", vars.value("thisHostName"), port,vars.value("thisDomain","undefined") );
            isProblemReported=0;
        }
    }

    return true;
}

void sQPrideProc::releaseTriggerPorts(void)
{
    close((int)socketSelect);
    socketSelect=0;
}

