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
#include <slib/core/tim.hpp>

using namespace slib;


sQPrideProc::sQPrideProc(const char * defline00, const char * service)
    : sQPride(defline00, service ? (service[0]=='+' ? sQPrideBase::QPrideSrvName(0, service+1, sApp::argv[0]) : service) : "qm" )
{
    if( !ok ) {
        return;
    }
    if(!service) service="qm";
    alwaysRun = false;

    loopCnt = 0;
    noGrabs = 0;
    maxMemSize = 0;
    exitCode = eQPErr_None;
    inBundle = 0;
    socketSelect = 0;
    isProblemReported = 0;
    sleepTimeOverride = 0;
    lastInGroup = false;

#ifdef SLIB_WIN
    doStdin = 0;
#endif

    doStdin = 1;
    tempDBConnection = false;
    maxmemMailSent = false;

    _argvBuf = 0;
    _argvBufLen = 0;
    _argvBufChanged = false;

    if( strcmp(service, "data_cache") == 0 )
        return;

    tmfix.time();
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
        sQPrideBase::user=0;
    }
}




void sQPrideProc::executeRequest(void * procthis)
{
    sQPrideProc *pc = (sQPrideProc*) procthis;
    pc->tmExec.time();
    sStr log("==> request %" DEC " (%" DEC ", %" DEC ") user %" UDEC, pc->reqId, pc->masterId, pc->grpId, pc->user ? pc->user->Id() : 0);
    if( pc->objs.dim() ) {
        log.printf(" proc obj[0] %s", pc->objs[0].Id().print());
            for(idx io = 0; io < pc->objs.dim(); ++io) {
        if( pc->reqId == pc->grpId ) {
                pc->objs[io].propSync();
            }
        }
    }
    pc->logOut(eQPLogType_Trace, "%s started\n", log.ptr());
    pc->reqGetData(pc->reqId,"_reqStage", &pc->requestStage, true);
    if( !pc->requestStage.length() ) {
        pc->requestStage.add("init", 5);
    }
    pc->user->pForm=pc->pForm;
    pc->exitCode = pc->OnExecute(pc->reqId);
    idx status = pc->reqGetStatus(pc->reqId);
    pc->logOut(eQPLogType_Info, "%s finished, returned %" DEC ", status %" DEC "\n", log.ptr(), pc->exitCode, status);
    ++pc->loopCnt;
    if (pc->svc.runInMT > 1) {
        pc->messageSubmit(pc->vars.value("thisHostName"), pc->svc.svcID, false, "wake");
    }
    if( pc->isLastInMasterGroup() ) {
        pc->OnCollect(pc->reqId);
    }
    if( pc->isLastInGroup() ) {
        pc->OnFinalize(pc->reqId);
    }
    if( status > eQPReqStatus_Running || pc->lastInGroup || pc->isLastInGroup() ) {
        for(idx io = 0; io < pc->objs.dim(); ++io) {
            pc->objs[io].propSync();
        }
    }
    pc->OnReleaseRequest(pc->reqId);
    pc->reqId = 0;

}


idx sQPrideProc::splitFile(sVar * pForm, sUsr * user, sUsrProc * obj, const char * fld, idx slice)
{
    sUsrObj * so = (sUsrObj *)obj;
    sStr splitFieldValue;
    const char * fld_val = pForm->value(fld);
    if( !fld_val && so ) {
        fld_val = so->propGet00(fld, &splitFieldValue, "\n");
    }

    idx cntChunks = 0;
    sHiveId id(fld_val);
    if( id.valid() ) {
        std::auto_ptr<sUsrObj> obj(user->objFactory(id));
        sUsrFile * file = dynamic_cast<sUsrFile *>(obj.get());
        if( file ) {
            sStr buf;
            sFil f(file->getFile(buf), sMex::fReadonly);
            if( f.ok() ) {
                cntChunks = (f.recCnt(true) - 1) / slice + 1;
            }
        }
    }
    return cntChunks;
}
idx sQPrideProc::splitList(sVar * pForm , sUsr * user, sUsrProc * obj, const char * fld, idx slice)
{
    sUsrObj * so = (sUsrObj *)obj;
    sStr splitFieldValue;
    const char * fld_val = pForm->value(fld);
    if( !fld_val && so ) {
        fld_val = so->propGet00(fld, &splitFieldValue, "\n");
    }

    sVec<idx> lstN;
    sString::scanRangeSet(fld_val, 0, &lstN, 0, 0, 0);
    return lstN.dim();
}
idx sQPrideProc::splitMultiplier(sVar * pForm , sUsr * user, sUsrProc * obj, const char * fld, idx slice)
{
    sUsrObj * so = (sUsrObj *)obj;
    sStr splitFieldValue;
    const char * fld_val = pForm->value(fld);
    if( !fld_val && so ) {
        fld_val = so->propGet00(fld, &splitFieldValue, "\n");
    }
    return atoidx(fld_val);
}



sRC sQPrideProc::OnSplit(idx req, idx &cnt)
{
    return genericSplitting(cnt, pForm,user,objs.ptr(),&svc,0);
}

enum eSplitTypes {
    eSplitFile,
    eSplitList,
    eSplitMultiplier
};
const char * sQPrideProc::listTypes="file" _ "list" _ "multiplier" __;

sQPrideProc::splittertFunction sQPrideProc::getSplitFunction(const char * type)
{
    if(!type)
        return 0;

    idx splittypenum=-1;
    sString::compareChoice(type, sQPrideProc::listTypes, &splittypenum, false, 0, true);

    switch(splittypenum) {
        case eSplitFile: {
            return (sQPrideProc::splitFile);
        }
        case eSplitList: {
            return (sQPrideProc::splitList);
        }
        case eSplitMultiplier: {
            return (sQPrideProc::splitMultiplier);
        }
        default: return 0;
    }
    return 0;
}

const char * sQPrideProc::_getSplitValue(const char * field_name, const char * alt_field_name, sVar * pForm, const char * def_val, sStr * buf)
{
    const char * splitValue = 0;
    if( !pForm ) {
        pForm = this->pForm;
    }

    if( pForm ) {
        splitValue = pForm->value(field_name, 0);
        if (!splitValue){
            static sUsrObjPropsTree prop_tree(*user,(const char*)0);
            prop_tree.empty(true);
            bool use_prop_tree = false;
            sStr err;
            if( user->propSet(*pForm, err, prop_tree.getTable(), false) ){
                if( prop_tree.useTable(prop_tree.getTable()) || !prop_tree.initialized() )
                    use_prop_tree = true;
            }

            splitValue = use_prop_tree?prop_tree.findValue(field_name, def_val):pForm->value(field_name, def_val);
        }
    }
    if(splitValue && buf)
        splitValue = buf->addString(splitValue);

    return splitValue;
}


sRC sQPrideProc::genericSplitting(idx &cntParallel, sVar * pForm , sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log)
{
    sUsrObj * so = (sUsrObj *) obj;

    if (so) {
        const char * doBatch = so->propGet00("batch_svc") ;
        if( doBatch ) {
            const char * batch = so->propGet00("batch_param");
            if( batch && *batch ) {
                cntParallel = 1;
                return sRC::zero;
            }
        }
    }


    sStr sSplitType,sSplitField,sSplitSlice;

    const char * splitType = getSplitType(pForm, 0, &sSplitType);
    const char * splitField = getSplitField(pForm, 0, &sSplitField);
    const char * splitSize = getSplitSize(pForm, 0, &sSplitSlice);

    cntParallel = 1;
    sStr lstType00,lstField00,lstSlice00;
    sString::searchAndReplaceSymbols(&lstType00, splitType, 0, " ,\r\n", 0, 0, true, true, true, true);
    sString::searchAndReplaceSymbols(&lstField00, splitField, 0, " ,\r\n", 0, 0, true, true, true, true);
    sString::searchAndReplaceSymbols(&lstSlice00, splitSize, 0, " ,\r\n", 0, 0, true, true, true, true);
    if( sString::cnt00(lstType00) != sString::cnt00(lstField00) && sString::cnt00(lstType00) != sString::cnt00(lstSlice00) ) {
        return sRC(sRC::eSplitting,sRC::eRequest,sRC::eField, sRC::eIncorrect);
    }
    sRC rc= sRC::zero;
    const char * s_type = lstType00, * s_field = lstField00, * s_size = lstSlice00;
    for(; s_type && s_field && s_size; s_type = sString::next00(s_type), s_field = sString::next00(s_field),s_size = sString::next00(s_size)) {
        splittertFunction splf = getSplitFunction(s_type);
        if( !splf ) {
            rc.set(sRC::eSplitting,sRC::eRequest,sRC::eFunction, sRC::eNotFound);
            continue;
        }
        idx currentCnt = splf(pForm, user, obj, s_field, atoidx(s_size));
        cntParallel *= currentCnt ? currentCnt : 1;
    }
    return rc;
}

sRC sQPrideProc::splitRequest(void)
{
    sRC rc;
    if( reqId ) {
        if( reqGetAction(reqId) == eQPReqAction_Split ) {
            pForm = &reqForm;
            idx cntParallel = 1;
            rc = OnSplit(reqId, cntParallel);
            if( !rc ) {
                if( cntParallel > 1 ) {
                    grpSubmit(svc.name, 0, 0, cntParallel - 1, 0, reqId);
                    if( !isGroupId() ) {
                        sVec<idx> reqList;
                        grp2Req(reqId, &reqList, 0, reqId);
                        for(idx ig = 1; ig < reqList.dim(); ++ig) {
                            grpAssignReqID(reqList[ig], grpId, ig);
                        }
                    }
                } else if( cntParallel < 1 ) {
                    rc = sRC(sRC::eSplitting, sRC::eRequest, sRC::eResult, sRC::eIncompatible);
                }
            }
        }
    } else {
        rc = sRC(sRC::eSplitting, sRC::eRequest, sRC::eRequest, sRC::eUninitialized);
    }
    if( !rc ) {
        bool is_postponed = sUsrProc::isPostponed(0,0, pForm);
        sVec<idx> reqList;
        grp2Req(reqId, &reqList, 0, reqId);
        for(idx ig = 0; ig < reqList.dim(); ++ig) {
            if(is_postponed) {
                reqSetStatus(reqList[ig], eQPReqStatus_Suspended);
                reqSetAction(reqList[ig], eQPReqAction_Postpone);
            } else {
                reqSetAction(reqList[ig], eQPReqAction_Run);
            }
        }
    }
    return rc;
}

idx sQPrideProc::OnGrab(idx forceReq)
{
    isRegrab = 0;

    if( forceReq ) {
        reqId = forceReq;
        isRegrab = 1;
        reqSetStatus(reqId, eQPReqStatus_Processing);
    } else {
        reqId = reqGrab(0, jobId, inBundle, eQPReqStatus_Waiting, eQPReqAction_Split);
        if(!reqId) {
            reqId = reqGrab(0, jobId, inBundle, eQPReqStatus_Waiting, eQPReqAction_Run);
        }
    }
    const idx prvReqId = reqId;

    lazyTime.time();
    if( !user ) {
        user = new sUsr();
    }
    if( reqId != 0 && user->init(reqGetUser(reqId)) ) {
        psMessage("user %" DEC " req %" DEC, (user && user->Id()) ? user->Id() : 0, reqId);
        grpId = req2Grp(reqId);
        masterId = req2Grp(reqId, 0, true);

        reqForm.empty();
        pForm = reqGetData(masterId, "formT.qpride", &reqForm);

        reqLocForm.empty();
        pLocForm = reqGetData(reqId, "formT.qpride", &reqLocForm);
        progress100Start = 0;
        progress100End = 100;
        progress100Last = 0;
        objs.cut(0);
        if( user ) {
            sStr strObjList;
            sVec<sHiveId> objIds;
            requestGetPar(grpId, eQPReqPar_Objects, &strObjList);

            if( strObjList )
                sHiveId::parseRangeSet(objIds, strObjList, strObjList.length());

            for(idx io = 0; io < objIds.dim(); ++io) {
                idx cnt = objs.dim();
                sUsrProc * p = objs.add(1);
                new (p) sUsrProc(*user, objIds[io]);
                if( !objs[cnt].Id() ) {
                    objs.cut(cnt);
                }
            }
            sStr rootPath;
            sUsrObj::initStorage(cfgStr(&rootPath, 0, "user.rootStoreManager"), cfgInt(0, "user.storageMinKeepFree", (udx) 20 * 1024 * 1024 * 1024));
        }
        if( sRC rc = splitRequest() ) {
            reqSetInfo(reqId, sQPrideBase::eQPInfoLevel_Error, "%s", rc.print());
            reqSetStatus(reqId, sQPrideBase::eQPReqStatus_SysError);
            return 0;
        }
        reqSliceCnt = 1;
        reqSliceId = req2GrpSerial(reqId, masterId, &reqSliceCnt, svc.svcID) - 1;
        if( reqSliceId < 0 ) {
            reqSliceId = 0;
        }
        if( !reqSliceCnt ) {
            reqSliceCnt = 1;
        }

        reqSetStatus(reqId, eQPReqStatus_Running);
        rand_seed = pForm->ivalue("rand_seed", time(0)) + reqSliceId;
        srand(rand_seed);
        if( svc.runInMT > 1 ) {
            idx code = 0;
            threadBegin(executeRequest, (void* )this, code);
            if(code)code=0;
        } else {
            executeRequest((void *) this);
            reqId = 0;
        }
        user->init();
    } else {
        if( !promptOK ) {
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
                memset(buf.ptr(pos), ' ', tail);
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
        return 0;
    }
    for(idx i = 1; i + 1 < argc; ++i) {
        if(strcmp(argv[i],"-svc")!=0){
            ok = executeCommand(argv[i], argv[i + 1]);
        } else ok=true;
        ++i;
        if( !ok ) {
            break;
        }
    }
    if( doStdin == 0 && argc > 0 ) {
        _argvBuf = const_cast<char *>(argv[1]);
        for(idx i = 1; i < argc; ++i) {
            _argvBufLen += strlen(argv[i]) + 1;
        }
        _argvBufLen--;
    }
    sStr tmpB;
    cfgStr(&tmpB,0,"qm.domains","");
    char * domains00=vars.inp("domains",tmpB.ptr());
    sString::searchAndReplaceSymbols(domains00,0, "/", 0, 0, true,true,true,true);
    const char* thisHostName=vars.value("thisHostName");

    for(char * qmThisDomain=domains00; qmThisDomain && *qmThisDomain; qmThisDomain=sString::next00(qmThisDomain), ++inDomain){
        sStr td; td.printf("qm.domains_%s",qmThisDomain);
        tmpB.cut(0);cfgStr(&tmpB,0,td,""); sString::searchAndReplaceSymbols(tmpB.ptr(),0, "/", 0, 0, true,true,true,true);

        idx insideDomain=0;
        for( char * rp=tmpB.ptr(); rp; rp=sString::next00(rp), ++insideDomain) {

            isDomainFound=hostNameMatch(rp, thisHostName);

            if(isDomainFound) {
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

    for (loopCnt = 0; loopCnt < svc.maxLoops && exitCode == eQPErr_None;) {

        if( reqId != 0 ) {
            selectSleep(sleepTimeOverride);
            if( loopCnt >= svc.maxLoops ) {
                logOut(eQPLogType_Info, "Exit requested. Waiting for %" DEC " to finish\n", reqId);
                loopCnt = svc.maxLoops - 1;
            }
            if( memReport(reqId, svcName) ) {
                break;
            }
            continue;
        }

        if (!dbHasLiveConnection()) {
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

        idx act;
        act=jobGetAction(jobId);
        if (act == eQPJobAction_Kill) {
            logOut(eQPLogType_Trace,"Exiting politely ... according to action required for this job\n");
            break;
        }

        idx tmdiff = tmfix.time();
        if (tmdiff > ((idx) (svc.cleanUpDays) * 24 * 3600 - 2 * 3600)) {
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

        const idx grabbedReqId = OnGrab();
        if( !grabbedReqId ) {
            ++noGrabs;
            if (svc.noGrabExit && noGrabs >= svc.noGrabExit) {
                logOut(eQPLogType_Trace, "Nothing to grab after %" DEC " attempts ... exiting temporarily\n", noGrabs);
                break;
            }
            if (svc.noGrabDisconnect && noGrabs >= svc.noGrabDisconnect) {
                if (tempDBConnection == false)
                    logOut(eQPLogType_Trace, "Nothing to grab after %" DEC " attempts ... closing the DB connection temporarily\n", noGrabs);
                tempDBConnection = true;
                dbDisconnect();
            }
            selectSleep(sleepTimeOverride);

        } else {
            if( memReport(grabbedReqId, svcName) ) {
                break;
            }
            noGrabs = 0;
            tempDBConnection = false;
        }

        if(!jobShouldRun())
            loopCnt =svc.maxLoops+1;

        if (svc.maxLoops && loopCnt >= svc.maxLoops ) {
            logOut(eQPLogType_Trace, "Exiting after executing %" DEC " requests \n", loopCnt);
            break;
        }
        flushCache();
    }

    logOut(eQPLogType_Trace, "Exiting legally\n");
    jobSetStatus(jobId, eQPJobStatus_ExitNormal);

    OnQuit();

    return exitCode;
}

bool sQPrideProc::memReport(const idx req, const char * svcName)
{
    sPS psLocal;
    idx mem = psLocal.getMem(pid);
    if( maxMemSize < mem ) {
        maxMemSize = mem;
    }
    jobSetMem(jobId, mem, maxMemSize);
    if( svc.maxmemSoft != 0 && mem > svc.maxmemSoft && maxmemMailSent == false ) {
        logOut(eQPLogType_Warning, "While executing %" DEC " service '%s' used %" DEC "MB of memory (Soft=%" DEC ", Hard=%" DEC ")\n",
            req, svcName, mem / (1024 * 1024), svc.maxmemSoft / (1024 * 1024), svc.maxmemHard / (1024 * 1024));
        maxmemMailSent = true;
    }
    if( svc.maxmemHard != 0 && mem > svc.maxmemHard ) {
        logOut(eQPLogType_Fatal, "While executing %" DEC " service '%s' used %" DEC "MB of memory (Soft=%" DEC ", Hard=%" DEC "). Hard limit reached: exiting...\n",
            req, svcName, mem / (1024 * 1024), svc.maxmemSoft / (1024 * 1024), svc.maxmemHard / (1024 * 1024));
        jobSetStatus(jobId, eQPJobStatus_ExitError);
        return true;
    }
    return false;
}


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
    if (!promptOK) {
        promptOK = 1;
        time_t tt;
        time(&tt);
        struct tm & t = *localtime(&tt);
        printf("\n%" DEC "/%" DEC "/%" DEC " %" DEC ":%" DEC ":%" DEC " %" DEC "/%" DEC " %s > ", (idx)t.tm_mday, (idx)t.tm_mon + 1,
                (idx)t.tm_year + 1900, (idx)t.tm_hour, (idx)t.tm_min, (idx)t.tm_sec, (idx)jobId, (idx)pid,
                vars.value("serviceName"));

        fflush(0);
    }

    fd_set rfds;
    idx retval = 0;
    idx len = 0;

    if (!slpTm)
        slpTm = svc.sleepTime;
    initializeTriggerPorts();

    #ifdef ZZZ_WIN32
        struct _SECURITY_ATTRIBUTES secat;
        secat.nLength = sizeof (struct _SECURITY_ATTRIBUTES);
        secat.lpSecurityDescriptor = NULL;
        secat.bInheritHandle = FALSE;
        HANDLE conin = CreateFile("CONIN$", GENERIC_READ, FILE_SHARE_READ, &secat, OPEN_EXISTING, 0, 0);

        HANDLE handle[2];
        handle[0]=conin;


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

    #else
        struct timeval tv;
        tv.tv_sec = (long) (slpTm / 1000);
        tv.tv_usec = (slpTm % 1000) * 1000;

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
            fgets(Buf, sizeof(Buf), stdin);
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
#define jobIsCmd(_cmd) (!strcmp(nam,_cmd) || !strcmp(nam,"-" _cmd))

    if( !reqId && !dbHasLiveConnection() ) {
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
        svc.maxLoops = 0;
        logOut(eQPLogType_Trace, "Exit requested.\n");
    } else if( jobIsCmd("shell")) {
        system(val);
    } else if( jobIsCmd("time")) {
        tmCount.time();
    } else if( jobIsCmd("sleep")) {
        sscanf(val, "%" DEC, &sleepTimeOverride);
    } else if( jobIsCmd("stdin")) {
        sscanf(val, "%" DEC, &doStdin);
    } else if( jobIsCmd("loops")) {
        sscanf(val, "%" DEC, &svc.maxLoops);
    } else if( jobIsCmd("force")) {
        alwaysRun = true;
    } else if ( jobIsCmd("logLevel") ) {
        idx log_level = 0;
        if( sscanf(val, "%" DEC, &log_level) == 1 ) {
            setupLog(true, log_level);
        }
    } else if( jobIsCmd("reqSliceId")) {
        sscanf(val, "%" DEC, &reqSliceId);
    } else if( jobIsCmd("reqSliceCnt")) {
        sscanf(val, "%" DEC, &reqSliceCnt);
    } else if( jobIsCmd("grab")) {
        sVec<idx> reqList;
        sString::scanRangeSet(val, 0, &reqList, 0, 0, 0);
        for(idx i = 0; i < reqList.dim(); ++i) {
            OnGrab(reqList[i]);
        }
    } else if( jobIsCmd("trigger")) {

    } else if( jobIsCmd("daemon")) {

    } else if( jobIsCmd("wake")) {
        logOut(eQPLogType_Trace, "awaken\n");
    } else if( jobIsCmd("jobarr")) {
        sscanf(val, "%" DEC, &inBundle);
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
        if (!propVal){
            propVal = pLocForm->value(prop, defaultValue, &propValLen);
        }
        if( propVal && lbuf ) {
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
            sscanf(p, "%" DEC, values->add(1));
    return ret;
}

idx sQPrideProc::formUValues(const char *prop, sVec<udx> *values, idx iObj)
{
    sStr buf00;
    idx ret = -1;
    if (formValues00(prop, &buf00, 0, iObj))
        for (const char *p = buf00.ptr(); p; p = sString::next00(p), ret++)
            sscanf(p, "%" UDEC, values->add(1));
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

sFil * sQPrideProc::reqAddFile(sFil * f, idx flags, const char * flnmFmt, ...)
{
    sStr pathVariableName("file://");
    sCallVarg(pathVariableName.vprintf, flnmFmt);
    const char * flnm = pathVariableName.ptr(7);

    sStr buf;
    if( objs.dim() && !sIs("req-", flnm) && !sIs("reqself-", flnm) && !sIs("reqgrp-", flnm) ) {
        if( !objs[0].addFilePathname(buf, true, "%s", flnm) ) {
            return 0;
        }
    }
    if( !buf.length() ) {
        idx dataReq = (grpId && !sIs("req-", flnm)) ? grpId : reqId;
        if( sIs("reqself-", flnm) ) {
            dataReq = reqId;
        }
        if( reqSetData(dataReq, pathVariableName, 0, 0) ) {
            reqDataPath(dataReq, flnm, &buf);
        }
    }

    f->destroy();
    f->init(buf,flags);

    return f->ok() ? f : 0 ;
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
        reqSetData(dataReq, pathVariableName, 0, 0);
        reqDataPath(dataReq, flnm, buf);
    }
    return buf->ptr(pos);
}

idx sQPrideProc::reqProgress(idx items, idx progress, idx progressMax)
{
    bool abort_proc = false;
    if( reqId && _argvBuf && (!_argvBufChanged || jobRegisterAlive(0, 0, svc.lazyReportSec, true)) ) {
        abort_proc = !OnProgress(reqId);
        if( !abort_proc ) {
            const idx prcnt = progress2Percent(items, progress, progressMax);
            psMessage("user %" UDEC " req %" DEC " %" DEC "%%", user ? user->Id() : 0, reqId, prcnt);
        }
    }
    if( !abort_proc ) {
        return sQPrideBase::reqProgress(reqId, svc.lazyReportSec, items, progress, progressMax);
    }
    return 0;
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

typedef struct reqProgressFSStatic2_data_struct {
    sQPrideProc * qp;
    sStr FSPath;
} reqProgressFSStatic2_data;

bool sQPrideProc::reqProgressFSStatic2(idx pid, const char * path, struct stat * st, void * param)
{
    reqProgressFSStatic2_data * d = static_cast<reqProgressFSStatic2_data *>(param);
    if( d ) {
        const idx items = path ? sDir::size(d->FSPath) : 0;
        return d->qp->reqProgress(items, 0, -1) == 0 ? false : true;
    }
    return false;
}

idx sQPrideProc::exec(const char * cmdline, const char * input, const char * path, sIO * log, idx sleepSecForExec)
{
    return sPipe::exeFS(log, cmdline, 0, sleepSecForExec ? reqProgressFSStatic : 0 , this, path, sleepSecForExec > 0 ? sleepSecForExec : svc.lazyReportSec);
}

sRC sQPrideProc::exec2(sPipe2::CmdLine & cmdline, const char * path, sIO * log, idx sleepSecForExec)
{
    sPipe2 e(&cmdline);
    if( sleepSecForExec ) {
        reqProgressFSStatic2_data param;
        param.qp = this;
        if( path ) {
            param.FSPath.printf("%s", path);
        }
        param.FSPath.add0(2);
        e.setMonitor(reqProgressFSStatic2, 0, &param, sleepSecForExec > 0 ? sleepSecForExec : svc.lazyReportSec, true);
    }
    if( log ) {
        e.setStdErr(log).setStdOut(log);
    }
    sRC rc = e.execute();
#if !_DEBUG
    if( rc )
#endif
    {
        logOut(rc ? eQPLogType_Error : eQPLogType_Debug, "exec[%s: %" DEC "] %s", rc.print(), e.retcode(), cmdline.printBash());
        if( log ) {
            sStr buf;
            sString::searchAndReplaceSymbols(&buf, log->ptr(), log->length(), "\r\n", 0, 0, true, true, false, true);
            for(const char * p = buf.ptr(); p; p = sString::next00(p)) {
                logOut(rc ? eQPLogType_Error : eQPLogType_Debug, "%s", p);
            }
        }
    }
    return rc;
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


#include <ulib/uquery.hpp>

const char * sQPrideProc::replaceCommandLineVars(sStr * dst, const char * src, idx len, const char * svcName, const char * resourceRoot)
{
    sStr repl00;
    repl00.addString(svcName);
    repl00.add0();
    repl00.addString(resourceRoot);
    repl00.add0(2);
    return sString::searchAndReplaceStrings(dst, src, len, "$(svc)" _ "$(resourceRoot)" __, repl00.ptr(), 0, false);
}
const char * sQPrideProc::replaceCommandLineArgs(sStr& proc, sStr* args, const char * svcName, const char * svcCmdLine, const char * resourceRoot, sUsr* user, const char * apptype)
{
    if(!apptype)apptype="sysappsrv";
    sStr buf("%s", svcCmdLine);
    char * objDefinition, *objDefEnd;
    bool oldMode=user->m_SuperUserMode;user->m_SuperUserMode=true;
    while( (objDefinition = strstr(buf, "$(obj")) != 0 && (objDefEnd = strstr(objDefinition, ")")) != 0 ) {

        sStr error;
        sVariant results;
        qlang::sUsrEngine engine(*user, 0);
        const char * objID=0;
        if( !isdigit(objDefinition[6])  ) {
            sStrT t;
            if(objDefEnd==objDefinition+5)t.printf("alloftype('%s',{'name':'%s'})[0]",apptype,svcName );
            else t.printf("alloftype('%s',{'name':'%.*s'})[0]",apptype,(int)(objDefEnd-objDefinition-6), objDefinition+6);
            if( engine.parse(t.ptr(), 0, &error) && engine.eval(results, &error))
                objID=results.asString();
        } else
            objID=objDefinition + 6;
        if(!objID)
            continue;

        sHiveId objId(atoidx(objID),0);
        sStr objPath;
        if( user ) {
            sUsrObj obj(*user, objId);
            if(obj.Id()) {
                obj.getFilePathname(objPath);
                sDir::chDir(objPath);
            }
        }

        sStr b1;
        b1.add(objDefinition, objDefEnd - objDefinition + 1);
        b1.add0(2);
        sStr b2;
        sString::searchAndReplaceStrings(&b2, buf.ptr(), buf.length(), b1.ptr(), objPath.ptr(), 0, false);
        buf.printf(0, "%s", b2.ptr());
    }
    user->m_SuperUserMode=oldMode;
    proc.cut(0);
    const char * cl = buf.ptr();
    if( *cl != '\\' && *cl != '/' ) {
        proc.printf("%s", resourceRoot);
        sDir::chDir(resourceRoot);
    }
    sStr exe;
    if(args)cl += sString::copyUntil(&exe, cl, 0, sString_symbolsBlank);
    else exe.printf("%s",cl);
    if( args ) {
        args->cut0cut();
        replaceCommandLineVars(args, cl, 0, svcName, resourceRoot);
        args->shrink00();
    }
    replaceCommandLineVars(&proc, exe.ptr(), exe.length(), svcName, resourceRoot);
    proc.shrink00();
    return proc.ptr();
}

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


    }
    *pCntList=countThisDomain;
    return numHost;
}


bool sQPrideProc::jobShouldRun(void)
{
    if (alwaysRun)
        return true;

    idx maxnumJobs = svc.maxJobs;
    hostNumInPool(&svc, (idx*)0, &maxnumJobs);


    sFilePath Proc,buf,tmp;
    sString::searchAndReplaceStrings(&buf,svc.cmdLine,0, "$(svc)" __,svc.name,0,false);
    replVar00(&tmp,buf.ptr(),buf.length());
    char * proc=Proc.procNameFromCmdLine(tmp.ptr());

    sVec<sPS::Stat> pi;
    sStr t("jobarr %" DEC, inBundle);
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
                logOut(eQPLogType_Info,"Cannot initiate server %s on UDP port %" DEC " in %s domain.\n", vars.value("thisHostName"), port,vars.value("thisDomain","undefined") );
            }
            ++isProblemReported;
        } else {
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



const char *  sQPrideProc::prepareFileAndDirs(sUsrObj * objDst, sStr * lPathToFileAndDirs, const char * pathTemplate, sUsrObj * objSrc, const char * fileObjList)
{
    if(!pathTemplate)return 0;
    sStrT buf;
    const char * srcObjList00=(objSrc && objSrc->Id()) ? objSrc->propGet00(fileObjList,&buf) :0 ;
    if(!srcObjList00)return 0;

    sStrT Pfd; if(!lPathToFileAndDirs)lPathToFileAndDirs=&Pfd;
    lPathToFileAndDirs->cut(0);
    char * path;
    path=(char*)objDst->addFilePathname(*lPathToFileAndDirs, false, pathTemplate);
    objs[0].propSet("importFolder", lPathToFileAndDirs->ptr(0));

    if(path){

        idx pathLen=lPathToFileAndDirs->length();

        sDir::makeDir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        for ( const char * srcObj=srcObjList00; srcObj; srcObj=sString::next00(srcObj) ) {

            sUsrObj o(*user,sHiveId(srcObj));
            sDir d;sStr pathL;o.getFilePathname(pathL);
            idx lPathLen=pathL.length();
            o.files(d, sFlag(sDir::bitSubdirs)|sFlag(sDir::bitFiles),"*" );

            for ( const char * p=d.ptr(); p; p=sString::next00(p) ) {
                lPathToFileAndDirs->printf(pathLen,"/o%s-%s",srcObj,sFilePath::nextToSlash(p));
                pathL.printf(lPathLen,"%s",p);
                sStr cmd;
                if( strncmp(p-4,".zip",4)==0 ){
                    cmd.printf("mkdir %s; unzip %s -x -d %s",lPathToFileAndDirs->ptr(0),pathL.ptr(0),lPathToFileAndDirs->ptr(0));
                } else  if( strncmp(p-7,".tar.gz",7)==0 ) {
                    cmd.printf("mkdir %s; tar xvfz %s -C %s " ,lPathToFileAndDirs->ptr(0),pathL.ptr(0),lPathToFileAndDirs->ptr(0));
                } else if ( strncmp(p-5,".gzip",5)==0  ) {
                    cmd.printf("gunzip -c %s > %.*s " ,pathL.ptr(0),(int)(lPathToFileAndDirs->length()-5),lPathToFileAndDirs->ptr(0));
                } else if ( strncmp(p-3,".gz",3)==0) {
                    cmd.printf("gunzip -c %s > %.*s " ,pathL.ptr(0),(int)(lPathToFileAndDirs->length()-3),lPathToFileAndDirs->ptr(0));
                }

                if(cmd.length()){
                    sPS ps;
                    ps.exec(cmd);
                }
                else {
                    sFile::symlink(pathL.ptr(),lPathToFileAndDirs->ptr());
                }
            }
        }

        lPathToFileAndDirs->cut(pathLen);lPathToFileAndDirs->add0();
        path=lPathToFileAndDirs->ptr(0);

    }else {
        path=(char*)objDst->getFilePathname(*lPathToFileAndDirs, pathTemplate);
    }
    path[sLen(path)-sLen(pathTemplate)-1]=0;



    return lPathToFileAndDirs->ptr();
}


idx sQPrideProc::prepareCmdLineArgsFromVars(sUsrObj * obj, sStr * pars, sStr * vals, const char * keyValueHead, const char * parametersPrefix, const char * keyFmt, const char * keyValEnd)
{
    sStrT keyArr;
    idx totCnt=0;
    const char * prop, * key, * value;
    if(keyValueHead) {
        const sUsrObjPropsNode * paramArr= obj->propsTree()->find(keyValueHead) ;
        if(paramArr){
            for(const sUsrObjPropsNode * paramRow = paramArr->firstChild(); paramRow; paramRow = paramRow->nextSibling()) {
                const char * key = paramRow->find("key")->value();

                prop=strchr(key,'.');
                if(prop) {
                    prop=prop+1;
                }
                pars->printf(keyFmt,key);

                const char * value = paramRow->find("value")->value();
                if(prop) {
                    sUsrObj Ov, *ov ;
                    if( value[0]=='.')ov=&(objs[0]) ;
                    else { ov=&Ov; new(ov) sUsrObj(*user,sHiveId(value)); }
                    if(ov->Id()) {
                        value=ov->propGet(prop,(sStr *)0,true);
                    }
                }

                if(vals) {
                    pars->add0(1);
                    sString::searchAndReplaceStrings(vals, value, 0, "\"" _ "\'" _ "\\" __ , "\\\"" _ "\\\'" _ "\\\\" __ , 0, false);
                    vals->shrink00();vals->add0(1);
                } else {
                    sString::searchAndReplaceStrings(pars, value, 0, "\"" _ "\'" _ "\\" __ , "\\\"" _ "\\\'" _ "\\\\" __ , 0, false);
                    pars->shrink00();
                    pars->printf(keyValEnd);
                }
                ++totCnt;
            }
        }
    }

    if(parametersPrefix){
        sDic < sStrT > arrVals;
        const sVarSet & tbl=obj->propsTree()->getTable();
        idx size, lenPrefix=sLen(parametersPrefix);

        sDic < idx >  Grps;
        sDic < idx >  Cnts(0,sMex::fSetZero);
        for(idx ir=0; ir<tbl.rows; ++ir ) {
            key=tbl.val(ir, 1, &size);
            if( !(size==4 and strncmp(key,"name",4)==0) && (size<lenPrefix-1 || strncmp(key,parametersPrefix,lenPrefix)!=0))
                continue;
            if(key[0]==parametersPrefix[0]){key+=lenPrefix;size-=lenPrefix;}
            idx * pCnt=Cnts.set(key,size);
            (*pCnt)++;

            if(ir<tbl.rows){
                const char * grp=tbl.val(ir, 2, &size);
                idx * pIndex=Grps.get(grp,size);
                if(!pIndex)*Grps.set(grp,size)=*pCnt-1;
            }
        }

        for(idx ir=0; ir<tbl.rows; ++ir ) {
            idx index=-1;
            key=tbl.val(ir, 1, &size);
            if( !(size==4 and strncmp(key,"name",4)==0) && (size<lenPrefix-1 || strncmp(key,parametersPrefix,lenPrefix)!=0))
                continue;
            if(key[0]==parametersPrefix[0]){key+=lenPrefix;size-=lenPrefix;}

            idx cnt=*Cnts.get(key,size);
            if(cnt>1) {
                const char * grp=tbl.val(ir, 2, &size);
                index=*Grps.get(grp,size);
            }



            if(index>-1) {
                keyArr.printf(0,"%s[%" DEC "]",key,index); pars->printf(keyFmt,keyArr.ptr(0));
            }
            else {
                prop=strchr(key,'.');
                if(prop) {
                    prop=prop+1;
                }
                pars->printf(keyFmt,key);
            }
            value=tbl.val(ir, 3, &size);

            if(prop) {
                sUsrObj Ov, *ov ;
                if( value[0]=='.')ov=&(objs[0]) ;
                else { ov=&Ov; new(ov) sUsrObj(*user,sHiveId(value));}
                if(ov->Id()) {
                    value=ov->propGet(prop,(sStr *)0,true);
                }
            }

            if(vals) {
                pars->add0(1);
                sString::searchAndReplaceStrings(vals, value, 0, "\"" _ "\'" _ "\\" __ , "\\\"" _ "\\\'" _ "\\\\" __ , 0, false);
                vals->shrink00();vals->add0(1);
            } else {
                sString::searchAndReplaceStrings(pars, value, 0, "\"" _ "\'" _ "\\" __ , "\\\"" _ "\\\'" _ "\\\\" __ , 0, false);
                pars->shrink00();
                pars->printf(keyValEnd);
            }

            if(index>-1 && value) {
                sStr * t=arrVals.set(key);
                t->printf("%s%s",t->length() ? "," :  "" , value);
                if(index==cnt-1) {
                    pars->printf(keyFmt,key);
                    if(vals) {
                        pars->add0(1);
                        vals->printf("%s", t->ptr(0));
                        vals->shrink00();vals->add0(1);
                    } else {
                        pars->printf("%s", t->ptr(0));
                        pars->shrink00();
                        pars->printf(keyValEnd);
                    }
                }
            }

            ++totCnt;
        }
    }

    if(vals) {
        pars->add0(2);
        vals->add0(2);
    }
    return totCnt;

}







idx sQPrideProc::cmdLineObjExec(sStr * cmdLine, const char * objType, const char * idorname, const char * script, sStr * logFileName)
{
    sStr pathAlgo;
    sUsrObj obj;
    sStrT buf1,buf2;
    sFil scriptFile;

    user->uniqueObjectAndPath(objType,idorname,&obj,&pathAlgo);
    if(strncmp(script,"var:",4)==0){
        obj.propGet(script+4,&buf1);
        script=buf1.ptr(0);
        if(!script)return 0;
    }
    if(strncmp(script,"file:",5)==0){
        if(script[5]=='/') {
            script=script+5;
        } else {
            script=buf2.printf("%s/%s",pathAlgo.ptr(0),script+5);
        }
        scriptFile.init(script,sMex::fReadonly);
        if(!scriptFile.length())
            return 0;
        script=scriptFile.ptr();
    }

    sStrT dst;
    user->replaceVarsFromObjForm(&dst, script, &objs[0], pLocForm );

    time_t ta = time(0);struct tm & t = *localtime(&ta);char outstr[256];
    sprintf(outstr, "%d/%d/%d %d:%d:%d",t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec);

    sStrT buf;
    sStrT logFlNm;if(!logFileName)logFileName=&logFlNm;ProcFile(buf.printf(0,"glauncher.log"), true, logFileName);
    sFil logFile;logFile.destroy();logFile.init(logFileName->ptr());
    replaceObjMacros(*cmdLine, buf.printf(0,"%s >> %s",dst.ptr(0),logFileName->ptr()) );
    logFile.printf("%s command line : %s\n\n\n",outstr,cmdLine->ptr(0));
    return 1;
}

