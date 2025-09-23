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
    isProblemReported = 0;
    sleepTimeOverride = 0;

#ifdef SLIB_WIN
    doStdin = 0;
#endif

    doStdin = 1;
    tempDBConnection = false;
    maxmemMailSent = false;

    _argvBuf = 0;
    _argvBufLen = 0;
    _argvBufChanged = false;

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
    if( sql() ) {
        sql()->disconnect();
    }
    if( sQPrideBase::user ) {
        delete sQPrideBase::user;
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
    pc->exitCode = pc->OnExecute(pc->reqId);
    const idx status = pc->reqGetStatus(pc->reqId);
    pc->logOut(eQPLogType_Info, "%s finished, returned %" DEC ", status %" DEC "\n", log.ptr(), pc->exitCode, status);
    ++pc->loopCnt;
    if (pc->svc.runInMT > 1) {
        pc->messageSubmit(pc->vars.value("thisHostName"), pc->svc.svcID, false, "wake");
    }
    if( pc->objs.dim() && pc->isEmailSender() ) {
        pc->user->sendEmailOnFinish(status, pc->objs[0]);
    }

    if( status > eQPReqStatus_Running && pc->isLastInGroup() ) {
        for(idx io = 0; io < pc->objs.dim(); ++io) {
            pc->objs[io].propSync();
        }
    }
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
        std::unique_ptr<sUsrObj> obj(user->objFactory(id));
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
    sStr spl_vals00;
    const char * fld_val = pForm->value(fld);
    if( !fld_val && so ) {
        so->propGet00(fld, &spl_vals00);
    } else {
        sString::searchAndReplaceSymbols(&spl_vals00, fld_val, 0, ";,\n", 0, 0, true, true, true, true);
    }

    return sString::cnt00(spl_vals00.ptr(0));
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

const char * sQPrideProc::getSplitType(sUsrObj * so, sVar * pForm, sStr * buf)
{
    const char * splitType = so ? so->propGet("splitType", buf) : 0;
    if( !pForm ) {
        pForm = this->pForm;
    }
    if( so ) {
        splitType = so->propGet("splitType", buf);
        if ( !splitType ) {
            splitType = so->propGet("scissors", buf);
        }
    }
    if( !splitType && pForm ) {
        splitType = pForm->value("splitType");
        if( !splitType ) {
            splitType = pForm->value("scissors");
        }
    }
    return splitType;
}
const char * sQPrideProc::getSplitField(sUsrObj * so, sVar * pForm, sStr * buf)
{
    const char * splitField = 0;
    if( !pForm ) {
        pForm = this->pForm;
    }
    if( so ) {
        splitField = so->propGet("splitField", buf);
        if ( !splitField ) {
            splitField = so->propGet("split", buf);
        }
    }
    if( !splitField && pForm ) {
        splitField = pForm->value("splitField");
        if( !splitField ) {
            splitField = pForm->value("split");
        }
    }
    return splitField;
}
const char * sQPrideProc::getSplitSize(sUsrObj * so, sVar * pForm, sStr * buf)
{
    const char * splitSize = 0;
    if( !pForm ) {
        pForm = this->pForm;
    }
    if( so ) {
        splitSize = so->propGet("splitSize", buf);
        if ( !splitSize ) {
            splitSize = so->propGet("slice", buf);
        }
    }
    if( !splitSize && pForm ) {
        splitSize = pForm->value("splitSize");
        if( !splitSize ) {
            splitSize = pForm->value("slice");
        }
    }
    return splitSize;
}

sRC sQPrideProc::genericSplitting(idx &cntParallel, sVar * pForm , sUsr * user, sUsrProc * obj, sQPride::Service * pSvc, sStr * log)
{
    sUsrObj * so = (sUsrObj *) obj;

    const char * batch = so ? so->propGet00("batch_param") : 0;
    if( batch && *batch ) {
        cntParallel = 1;
        return sRC::zero;
    }

    sStr sSplitType,sSplitField,sSplitSlice;
    const char * splitType = getSplitType(so, pForm, &sSplitType);
    const char * splitField = getSplitField(so, pForm, &sSplitField);
    const char * splitSize = getSplitSize(so, pForm, &sSplitSlice);

    cntParallel = 1;
    sStr lstType00,lstField00,lstSlice00;
    sString::searchAndReplaceSymbols(&lstType00, splitType, 0, " ,\r\n", 0, 0, true, true, true, true);
    sString::searchAndReplaceSymbols(&lstField00, splitField, 0, " ,\r\n", 0, 0, true, true, true, true);
    sString::searchAndReplaceSymbols(&lstSlice00, splitSize, 0, " ,\r\n", 0, 0, true, true, true, true);
    if( lstType00 && sString::cnt00(lstType00) != sString::cnt00(lstField00) && sString::cnt00(lstType00) != sString::cnt00(lstSlice00) ) {
        return RC(sRC::eSplitting,sRC::eRequest,sRC::eField, sRC::eIncorrect);
    }
    sRC rc= sRC::zero;
    const char * s_type = lstType00, * s_field = lstField00, * s_size = lstSlice00;
    for(; s_type && s_field && s_size; s_type = sString::next00(s_type), s_field = sString::next00(s_field),s_size = sString::next00(s_size)) {
        splittertFunction splf = getSplitFunction(s_type);
        if( !splf ) {
            RCSET(rc, sRC::eSplitting,sRC::eRequest,sRC::eFunction, sRC::eNotFound);
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
                    rc = RC(sRC::eSplitting, sRC::eRequest, sRC::eResult, sRC::eIncompatible);
                }
            }
        }
    } else {
        rc = RC(sRC::eSplitting, sRC::eRequest, sRC::eRequest, sRC::eUninitialized);
    }
    if( !rc ) {
        sVec<idx> reqList;
        grp2Req(reqId, &reqList, 0, reqId);
        for(idx ig = 0; ig < reqList.dim(); ++ig) {
            reqSetAction(reqList[ig], eQPReqAction_Run);
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
    if( reqId != 0 ) {
        grpId = req2Grp(reqId);
        masterId = req2Grp(reqId, 0, true);

        reqForm.empty();
        pForm = reqGetData(masterId, "formT.qpride", &reqForm);

        udx projectId = reqForm.uvalue("projectID", 0);
        if( !projectId ) {
            sVar grpForm;
            reqGetData(grpId, "formT.qpride", &grpForm);
            projectId = grpForm.uvalue("projectID", 0);
        }
        if( !user->init(reqGetUser(reqId), projectId) ) {
            psMessage("sleeping...");
            return 0;
        }
        psMessage("user %" DEC " req %" DEC, (user && user->Id()) ? user->Id() : 0, reqId);

        progress100Start = 0;
        progress100End = 100;
        progress100Last = 0;
        objs.cut(0);
        if( user ) {
            sStr rootPath;
            sRC rc = sUsrObj::initStorage(cfgStr(&rootPath, 0, "user.rootStoreManager"), cfgInt(0, "user.storageMinKeepFree", (udx) 20 * 1024 * 1024 * 1024), this);
            if( !rc ) {
                sStr strObjList;
                sVec<sHiveId> objIds;
                requestGetPar(reqId, eQPReqPar_Objects, &strObjList, true);
                if( strObjList.length() < 2 ) {
                    strObjList.cut0cut();
                    requestGetPar(masterId ? masterId : grpId, eQPReqPar_Objects, &strObjList, false);
                }
                if( strObjList ) {
                    sHiveId::parseRangeSet(objIds, strObjList, strObjList.length());
                }
                for(idx io = 0; io < objIds.dim(); ++io) {
                    idx cnt = objs.dim();
                    sUsrProc * p = objs.add(1);
                    new (p) sUsrProc(*user, objIds[io]);
                    if( !objs[cnt].Id() ) {
                        objs.cut(cnt);
                    }
                }
            } else {
                logOut(eQPLogType_Error, "%s", rc.print());
                reqSetStatus(reqId, sQPrideBase::eQPReqStatus_Waiting);
                return 0;
            }
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
        ok = executeCommand(argv[i], argv[i + 1]);
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
    if (values && formValues00(prop, &buf00, 0, iObj))
        for (const char *p = buf00.ptr(); p; p = sString::next00(p), ret++)
            sscanf(p, "%" UDEC, values->add(1));
    return ret;
}

idx sQPrideProc::formHiveIdValues(const char *prop, sVec<sHiveId> & values, idx iObj)
{
    sStr buf00;
    idx ret = values.dim();
    if(formValues00(prop, &buf00, 0, iObj)) {
        sHiveId id;
        for (const char *p = buf00.ptr(); p; p = sString::next00(p)) {
            if( id.parse(p) && id ) {
                sHiveId * added = values.add(1);
                if( added ) {
                    *added = id;
                } else {
                    values.resize(ret);
                    break;
                }
            }
        }
    }
    return values.dim();
}

idx sQPrideProc::formRValues(const char *prop, sVec<real> *values, idx iObj)
{
    sStr buf00;
    idx ret = -1;
    if (values && formValues00(prop, &buf00, 0, iObj))
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
    if( rc.isSet() )
#endif
    {
        logOut(rc.isSet() ? eQPLogType_Error : eQPLogType_Debug, "exec[%s: %" DEC "] %s", rc.print(), e.retcode(), cmdline.printBash());
        if( log ) {
            sStr buf;
            sString::searchAndReplaceSymbols(&buf, log->ptr(), log->length(), "\r\n", 0, 0, true, true, false, true);
            for(const char * p = buf.ptr(); p; p = sString::next00(p)) {
                logOut(rc.isSet() ? eQPLogType_Error : eQPLogType_Debug, "%s", p);
            }
        }
    }
    return rc;
}

bool sQPrideProc::lockKey(const char * key)
{
    const idx lockLifetime = 10 * 60;
    idx timeoutSec = 2 * lockLifetime;
    while( !reqLock(-reqId, key, 0, lockLifetime) && timeoutSec > 0 ) {
        --timeoutSec;
        sleepSeconds(1);
    }
    return timeoutSec > 0;
}

bool sQPrideProc::unlockKey(const char * key)
{
    return reqUnlock(-reqId, key);
}

bool sQPrideProc::isLastInGroup(const char * svcName )
{
    if( !svcName ) {
        svcName = vars.value("serviceName");
    }
    sVec<idx> stat;
    grpGetStatus(grpId, &stat, svcName);
    return getGroupCntStatusIs(grpId, eQPReqStatus_Done, &stat, sQPrideBase::eStatusEqual, svcName) >= stat.dim() - 1;
}

bool sQPrideProc::isLastInMasterGroup(const char *svcName)
{
    if( !svcName ) {
        svcName = vars.value("serviceName");
    }
    sVec<idx> stat;
    grpGetStatus(masterId, &stat, svcName, masterId);
    idx cntis = 0;
    for(idx i = 0; i < stat.dim(); ++i) {
        cntis += (stat[i] == eQPReqStatus_Done ? 1 : 0);
    }
    return cntis == stat.dim() - 1;
}

bool sQPrideProc::isLastInMasterGroupWithLock(void)
{
    const char * svcName = vars.value("serviceName");
    sStr key("%" DEC "-%s", masterId, __func__);
    if( !lockKey(key.ptr()) ) {
        logOut(eQPLogType_Warning, "Checking %s w/o lock", key.ptr());
    }
    sVec<idx> stat;
    grpGetStatus(masterId, &stat, svcName, masterId);
    idx cntis = 0;
    for(idx i = 0; i < stat.dim(); ++i) {
        cntis += (stat[i] == eQPReqStatus_Done ? 1 : 0);
    }
    reqSetStatus(reqId, eQPReqStatus_Done);
    if( !unlockKey(key.ptr()) ) {
        logOut(eQPLogType_Warning, "Failed to release %s lock", __func__);
    }
    return cntis == stat.dim() - 1;
}

bool sQPrideProc::isEmailSender(void)
{
    bool isEmailSender = false;
    const char * svcName = vars.value("serviceName");
    sStr key("%" DEC "-%s", grpId, __func__);
    sVec<idx> stat;
    grpGetStatus(grpId, &stat, svcName);
    idx cntis = 0;
    for(idx i = 0; i < stat.dim(); ++i) {
        cntis += (stat[i] >= eQPReqStatus_Done ? 1 : 0);
    }
    if( cntis == stat.dim() && reqLock(-reqId, key.ptr()) ) {
        isEmailSender = true;
    }
    return isEmailSender;
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

sUsrQueryEngine * sQPrideProc::queryEngineFactory(idx flags)
{
    return queryEngineInit(new sUsrInternalQueryEngine(this, *user, flags));
}

sUsrQueryEngine * sQPrideProc::queryEngineInit(sUsrQueryEngine * qe)
{
    if( qe) {
        if( objs.dim() != 0 ) {
            qe->registerBuiltinThis(objs[0].Id());
            qe->registerBuiltinIdxProperty(objs[0].Id(), "grpId", grpId);
        } else {
            const char *type = formValue("type");
            if( type ) {
                qe->registerBuiltinThisPropertiesForm(type, *pForm);
            }
        }
    }
    return qe;
}
