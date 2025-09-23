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
#include <slib/core/tim.hpp>
#include <slib/core/index.hpp>
#include <slib/std/online.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>
#include <qlib/QPrideBase.hpp>
#include "QPrideConnection.hpp"
#include <ulib/usr.hpp>
#include <ulib/uquery.hpp>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

using namespace slib;


sQPrideBase::sQPrideBase(sQPrideConnection * connection, const char * service)
    : QPDB(0), svcID(0), user(0), inDomain(0), jobId(0), pid(0), reqId(0), grpId(0), masterId(0),
      promptOK(true), qmBaseUDPPort(0), thisHostNumInDomain(0), isSubNetworkHead(false),
      isDomainFound(false), ok(false), largeDataReposSize(0), appMode(0),
      progress100Start(0), progress100End(100), progress100Last(0),
      iJobArrStart(0), iJobArrEnd(0), threadID(0), threadsWorking(0), threadsCnt(0)
{
    init(connection, service);
}

sQPrideBase * sQPrideBase::init(sQPrideConnection * lconnection, const char * service)
{
    if(!lconnection)return 0;

    sStr tmpB;

    QPDB=lconnection;

    svcID=0;
    jobId=0;
    inDomain=0;
    user=0;
    sUsr::setQPride(this);
    pid = getpid();
    reqId = 0;
    grpId = 0;
    masterId = 0;
    promptOK=true;
    ok=false;

    iJobArrStart=0;
    iJobArrEnd=0;
    threadID=0;
    threadsCnt=1;
    threadsWorking=threadsCnt;
    lazyTime.time();
    thisHostNumInDomain=-1;
    isDomainFound=false;

    largeDataReposSize=8*1024*1024;

    vars.inp("os", SLIB_PLATFORM);
    vars.inp("serviceName",service);

    tmpB.add(0, 16 * 1024);
    gethostname(tmpB.ptr(), (int) tmpB.length());
    vars.inp("thisHostName", tmpB.ptr());

    tmpB.printf(0, "%" DEC, pid);
    vars.inp("pid", tmpB.ptr());

    tmpB.cut(0);
    cfgStr(&tmpB, 0, "qm.tempDirectory", "/tmp/");
    vars.inp("qm.tempDirectory", tmpB.ptr(0));
    vars.inp("/tmp/", tmpB.ptr(0));

        tmpB.cut(0);
    cfgStr(&tmpB,0,"qm.subNetworkHeadList","");
    char * subdomainHeads00=vars.inp("subNetworkHeadList",tmpB.ptr());
    sString::searchAndReplaceSymbols(subdomainHeads00,0, "/", 0, 0, true,true,true,true);
    const char * thisHostName = vars.value("thisHostName");
    isSubNetworkHead=sString::compareChoice(thisHostName,subdomainHeads00,0,true,0,true)==-1 ? false : true;

    tmpB.cut(0);
    cfgStr(&tmpB,0,"qm.%platform%.largeDataRepository;qm.largeDataRepository",0);
    vars.inp("largeDataRepository",tmpB.ptr());
    largeDataReposSize=cfgInt(0,"qm.largeDataReposSize",8*1024*1024);

    tmpB.cut(0);
    cfgStr(&tmpB,0,"qm.baseSvcUDPPort","13666");
    qmBaseUDPPort=atoi(tmpB.ptr());if(!qmBaseUDPPort)qmBaseUDPPort=13666;

    tmpB.cut(0);
    cfgStr(&tmpB,0,"qm.%platform%.resourceRoot;qm.resourceRoot",0);
    vars.inp("resourceRoot",tmpB.ptr());

    umask(0);

    ok=true;
    return this;
}

char* sQPrideBase::QPrideSrvName(sStr * buf, const char * srvbase, const char * progname)
{
    if( !progname ) {
        progname = "unspecified";
    }
    const char * lastSlash = strrchr(progname, '/');
    if( !lastSlash ) {
        lastSlash = strrchr(progname, '\\');
    }
    if( !lastSlash ) {
        lastSlash = progname;
    } else {
        ++lastSlash;
    }
    if( !srvbase ) {
        srvbase = lastSlash;
    }
    const char * dash = strrchr(lastSlash, '~');
    if( dash ) {
        buf->printf("%s%s", srvbase, dash);
    } else {
        buf->printf("%s", srvbase);
    }
    const char * dotos = strstr(buf->ptr(0), ".os");
    if( dotos ) {
        buf->cut(dotos - buf->ptr(0));
        buf->add0();
    }
    return buf->ptr(0);
}

sQPrideBase::~sQPrideBase()
{
    if( sUsr::QPride() == this )
        sUsr::setQPride(0);
}



void sQPrideBase::logOut(eQPLogType level, const char * formatDescription, ...)
{
    va_list ap;
    va_start(ap, formatDescription);
    vlogOut(level, formatDescription, ap);
    va_end(ap);
}

void sQPrideBase::vlogOut(eQPLogType level, const char * formatDescription, va_list ap)
{
    if( formatDescription && eQPLogType_Trace <= level && level <= eQPLogType_Max ) {
        sStr dummy;
        sStr * str = level <= eQPLogType_Error ? &lastErr : &dummy;
        str->cut(0);
        str->vprintf(formatDescription, ap);
        if( str->length() ) {
            if( level >= abs(setupLog()) ) {
                QPDB->QP_setLog(reqId, jobId, level, str->ptr());
            }
            OnLogOut(level, str->ptr());
        }
    }
}

idx sQPrideBase::setupLog(bool force, idx force_level)
{
    static idx logging = -sIdxMax;

    if( logging == -sIdxMax || force ) {
        logging = force_level ? force_level : cfgInt(0, "qm.dbLogMinLevel",
#if _DEBUG
            eQPLogType_Min
#else
            eQPLogType_Warning
#endif
            );
        sStr tmp;
        cfgStr(&tmp, 0, "qm.logStdout",
#if _DEBUG
            "on"
#else
            "off"
#endif
            );
        if( sString::parseBool(tmp) && logging > 0 ) {
            logging *= -1;
        }
    }
    return logging;
}

bool sQPrideBase::OnLogOut(idx level, const char * message)
{
    if( setupLog() < 0 ) {
        time_t tt = time(0);
        struct tm & t = *localtime(&tt);
        fprintf(stderr, "%s%d/%d/%d %d:%d:%d %" DEC "/%" DEC " %s %s // ",
            promptOK ? "\n" : "", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, jobId, pid, vars.value("serviceName"), getLevelName(level));
        if( tmCount ) {
            sTime t;
            fprintf(stderr, "%" DEC "// ", t.time(&tmCount));
        }
        fprintf(stderr, "%s%s", message, message[sLen(message) - 1] == '\n' ? "" : "\n");
        fflush(stderr);
    }
    return false;
}

idx sQPrideBase::messageSubmit(const char * server, idx svcId, bool isSingular, const char * fmt, ...)
{
    sStr str; sCallVarg(str.vprintf,fmt);
    return sConIP::sendUDP(server, qmBaseUDPPort+svcId ,str.ptr(), str.length(), isSingular );
}

idx sQPrideBase::messageSubmit(const char * server, const char * service,bool isSingular, const char * fmt, ... )
{
    sStr str; sCallVarg(str.vprintf,fmt);
    return messageSubmit(server, QPDB->QP_serviceID(service) ,isSingular, str.ptr());
}

idx sQPrideBase::messageSubmitToDomainHeader(const char * fmt, ...)
{
    sStr str; sCallVarg(str.vprintf,fmt);

    idx cnt=0;

    for(const char * qmThisDomain=vars.value("subNetworkHeadList"); qmThisDomain; qmThisDomain=sString::next00(qmThisDomain)){
        cnt+=messageSubmit(qmThisDomain, "qm", true, str.ptr());
    }
    return cnt;
}


idx sQPrideBase::messageSubmitToDomain(const char * service,const char * fmt,...)
{
    sStr str;
    str.printf("broadcast:%s ",service);
    sCallVarg(str.vprintf,fmt);
    return messageSubmitToDomainHeader("%s",str.ptr());
}

idx sQPrideBase::messageWakePulljob(const char * service)
{
    return messageSubmitToDomainHeader(service,"wake");
}

void sQPrideBase::flushCache(){
    QPDB->QP_flushCache();
}

bool sQPrideBase::configSet(const char * par, const char * fmt, ... )
{
    sStr strB;
    if(par[0]=='.')strB.printf(vars.value("serviceName"));
    sCallVarg(strB.vprintf,fmt);
    if(!strB.length())return false;

    return QPDB->QP_configSet(par, strB.ptr()) ;
}

char * sQPrideBase::configGet(sStr * str , sVar * pForm, const char * parList, const char * defval, const char * fmt, ... )
{
    sStr tmpB,par00,resRepl;
    const char *varval=0;
    const char * macroSymbolsFind00="%platform%" __;

    bool tryPlatforms= strstr( parList,  macroSymbolsFind00 ) ? false : true;

    if(parList)sString::searchAndReplaceSymbols(&par00 , parList,0, ";", 0,0,true,true,true,true);
    par00.add0(2);
    for( const char * par=par00.ptr(); par && !varval; par=sString::next00(par) ) {
        if(pForm) {
            varval=pForm->value(par);
        }
        if(!varval) {
            sStr strB, strD, strT;
            if(par[0]=='.')strB.printf(vars.value("serviceName"));
            strB.printf("%s",par);
            tmpB.cut(0);

            if( tryPlatforms ) {
                strD.printf("%s-%s", SLIB_PLATFORM, strB.ptr());
                strD.add0(2);
                QPDB->QP_configGet(&tmpB, strD);
                if( !tmpB.length() ) {
                    strB.add0(2);
                    QPDB->QP_configGet(&tmpB, strB);
                }
            } else {
                strT.printf("%s", SLIB_PLATFORM);
                strT.add0(2);
                sString::searchAndReplaceStrings(&strD, strB, strB.length(), macroSymbolsFind00, strT.ptr(), 0, false);
                strD.add0(2);
                QPDB->QP_configGet(&tmpB, strD);
            }

            if(tmpB.length())
                varval=tmpB.ptr();
            if(varval && !(*varval))
                varval=0;
        }


        if(!varval)
            continue;

        sStr out,in;
        idx ichunk=0;
        sString::cleanMarkup(&out,varval,0,"$(" __,")" __,0,0,false,false, false);
        sString::cleanMarkup(&in,varval,0,"$(" __,")" __,0,0,true,false, false);

        for( const char * chunk=out.ptr(),* tag=in.ptr(); chunk || tag; chunk=sString::next00(chunk) , tag=sString::next00(tag) , ++ichunk) {

            if(tag && *tag){
                sStr v;
                configGet(&v, pForm, tag, 0, 0 );
                if(v)resRepl.printf("%s",v.ptr());
                else resRepl.printf("$(%s)",tag);
            }
            if(chunk && *chunk)resRepl.printf("%s",chunk);
        }
        varval=resRepl.ptr(0);

        if(varval)break;
    }

    if(!varval)
        varval=defval;

    if(varval && str )
        varval = str->printf("%s",varval);

    if( !fmt )
        return sString::nonconst(varval);

    if(!varval)
        return 0;

    sCallVargPara(vsscanf,varval,fmt);

    return sString::nonconst(varval);
}

char * sQPrideBase::configGetAll( sStr * vals00, const char * pars00)
{
    return QPDB->QP_configGet(vals00, pars00, false);
}

void sQPrideBase::makeVar00(void)
{
    parFind00.cut(0);parRepl00.cut(0);
    for ( idx i=0; i<vars.dim() ; ++i){
        const char * id=(const char *) vars.id(i);
        parFind00.printf("$(%s)",id); parFind00.add0();
        parRepl00.printf("%s",(const char *) vars.value(id)); parRepl00.add0();
    }
    parFind00.add0();
    parRepl00.add0();

}

char * sQPrideBase::replVar00(sStr * str, const char * src, idx len)
{
    sString::searchAndReplaceStrings(str,src,len, parFind00.ptr(0), parRepl00.ptr(),0,false);
    return str->ptr();
}

idx sQPrideBase::dbHasLiveConnection(void)
{
    return QPDB->QP_dbHasLiveConnection();
}
void sQPrideBase::dbDisconnect(void)
{
    QPDB->QP_dbDisconnect();
}
idx sQPrideBase::dbReconnect(void)
{
    return QPDB->QP_dbReconnect();
}

idx sQPrideBase::serviceGet(Service * svc, const char * service, idx svcId)
{
    if(svcId)return QPDB->QP_serviceGet( (void *)svc, 0 , svcId );
    else return QPDB->QP_serviceGet( (void *)svc, service ? service : vars.value("serviceName") , svcId );
}
idx sQPrideBase::serviceUp(const char * svc, idx isUpMask)
{
    return QPDB->QP_serviceUp(svc,isUpMask);
}
idx sQPrideBase::serviceList(sStr * lst00 , sVec < Service > * svclist)
{
    return QPDB->QP_serviceList(lst00 , svclist);
}

idx sQPrideBase::serviceID(const char * service)
{
    bool self=false;
    const char * mySvc=vars.value("serviceName");
    if(!service || strcmp(mySvc,service)==0 )
        self=true;
    if(self && svcID!=0)
        return svcID;
    idx svc=QPDB->QP_serviceID(service ? service : mySvc );
    if(self)
        svcID=svc;
    return svc;
}

void sQPrideBase::getRegisteredIP(sVec <sStr> * ips, const char * equCmd)
{
    QPDB->QP_getRegisteredIP(ips, equCmd);
}

void sQPrideBase::registerHostIP()
{
    const char * sys = vars.value("os");
    QPDB->QP_registerHostIP(sys);
}

real sQPrideBase::getHostCapacity()
{
    real capacity = 0;
    const char * hostname = vars.value("thisHostName");
    if( hostname && hostname[0] ) {
        capacity = QPDB->QP_getHostCapacity(hostname);
    }
    return capacity;
}

bool sQPrideBase::reqAuthorized(idx req)
{
    if( req > 0 ) {
        Request R;
        sSet(&R);
        requestGet(reqId, &R);
        return reqAuthorized(R);
    } else {
        return false;
    }
}

bool sQPrideBase::reqAuthorized(const sQPrideBase::Request & R)
{
    if( !R.reqID ) {
        return false;
    }
    if( R.userID && user && !user->isAdmin() && (udx)R.userID != user->Id() ) {
        return false;
    }
    return true;
}

void sQPrideBase::reqCleanTbl(idx req, const char * dataname)
{
    if( req <= 0 )return;
    if( !dataname ) return;
    sVec <sStr> tblfiles;
    QPDB->QP_reqCleanTbl(&tblfiles,req,dataname);

    sStr tblDir;

    if ( getReqFilePath(tblDir,req, dataname) ) {
        sDir::removeDir(tblDir,true);
    } else {
        cfgStr(&tblDir, 0, "qm.largeDataRepository");

        for (idx i=0; i<tblfiles.dim();++i) {
            sStr onefile;
            onefile.printf("%s",tblfiles.ptr(i)->ptr(0));

            sStr fullPath;
            fullPath.printf("%s%" DEC "-%s",tblDir.ptr(),req,onefile.ptr());

            sFile::remove(fullPath.ptr());
        }
    }
}

idx sQPrideBase::workRegisterTime(const char * svc, const char * params, idx amount, idx time)
{
    if (!svc)return 0;
    return QPDB->QP_workRegisterTime(svc, params, amount, time);
}

idx sQPrideBase::workEstimateTime(const char * svc, const char * params, idx amount)
{
    if( !svc )return 0;
    if( amount <= 0 )return 0;
    return QPDB->QP_workEstimateTime(svc, params, amount);
}

idx sQPrideBase::servicePath2Clean(sVarSet & tbl)
{
    sVarSet res1;
    QPDB->QP_servicePath2Clean(res1);
    for(idx r = 0; r < res1.rows; ++r) {
        sStr p00, m00;
        sString::searchAndReplaceSymbols(&p00, res1.val(r, 0), 0, ",", 0, 0, true, true, true, true);
        sString::searchAndReplaceSymbols(&m00, res1.val(r, 2), 0, "/", 0, 0, true, true, true, true);
        p00.add0();
        m00.add0();
        tbl.addRow().addCol(p00, p00.length()).addCol(res1.ival(r, 1)).addCol(m00, m00.length());
    }
    return res1.rows;
}

void sQPrideBase::servicePurgeOld(sVec<idx> * reqList, const char * service, idx limit, bool no_delete)
{
    QPDB->QP_servicePurgeOld(reqList, service, limit, no_delete);
}




idx sQPrideBase::reqProcSubmit( idx cntParallel, sVar * pForm, const char * svc , idx grp, idx autoAction, bool requiresGroupSubmission, idx priority , idx previousGrpSubmitCounter)
{
#if 1
    idx req=0;
    if( cntParallel > 1 || requiresGroupSubmission ) {
        req=grpSubmit(svc,0,priority,cntParallel,0,previousGrpSubmitCounter);
        if(grp) {
            sVec < idx > reqList;
            grp2Req(req,&reqList,0,req);
            for (idx ig=0; ig< reqList[ig]; ++ig ) {
                grpAssignReqID(reqList[ig], grp, ig);
            }
        }
    } else {
        req = reqSubmit(svc,0,priority);
        idx g = grp ? grp : (pForm ? pForm->ivalue("grp", 0) : 0);
        if( g ) {
            grpAssignReqID(req, g, 0);
        }
    }

    if(req){
        if(pForm) reqSetData(req,"formT.qpride",pForm);

        if( cntParallel > 1 || requiresGroupSubmission ) {
            sVec < idx > reqIds;
            grp2Req(req, &reqIds);
            reqSetAction(&reqIds, autoAction);
        } else {
            reqSetAction(req, autoAction);
        }
    }
    return req;
#else
    sVec<sQPrideBase::PriorityCnt> priority_cnts(sMex::fExactSize);
    priority_cnts.resize(1);
    priority_cnts[0].priority = priority;
    priority_cnts[0].cnt = 0;
    return reqProcSubmit2(cntParallel, pForm, svc, grp, autoAction, requiresGroupSubmission, &priority_cnts, previousGrpSubmitCounter);
#endif
}

sVec<sQPrideBase::PriorityCnt> * sQPrideBase::reqMakePriorityCnts(sVec<sQPrideBase::PriorityCnt> & priority_cnts, idx cntParallel, idx start_priority, idx prevTotalCntParallel)
{
    static const idx BASE = 10;

    priority_cnts.cut(0);

    idx base = BASE;
    idx priority = start_priority;

    while( base <= prevTotalCntParallel ) {
        base *= BASE;
        priority++;
    }

    idx prev = prevTotalCntParallel;

    while( base < prevTotalCntParallel + cntParallel ) {
        idx ip = priority_cnts.dim();
        priority_cnts.add(1);
        priority_cnts[ip].priority = priority;
        priority_cnts[ip].cnt = base - prev;

        prev = base;
        base *= BASE;
        priority++;
    }

    if( prev < prevTotalCntParallel + cntParallel  || !priority_cnts.dim() ) {
        idx ip = priority_cnts.dim();
        priority_cnts.add(1);
        priority_cnts[ip].priority = priority;
        priority_cnts[ip].cnt = prevTotalCntParallel + cntParallel - prev;
    }

    return &priority_cnts;
}

idx sQPrideBase::reqProcSubmit2( idx cntParallel, sVar * pForm, const char * svc , idx grp, idx autoAction, bool requiresGroupSubmission, const sVec<sQPrideBase::PriorityCnt> * priority_cnts, idx previousGrpSubmitCounter)
{
    idx req=0;
    if( cntParallel > 1 || requiresGroupSubmission ) {
        req=grpSubmit2(svc,0, priority_cnts ? priority_cnts->ptr() : 0, priority_cnts ? priority_cnts->dim() : 0, cntParallel, 0, previousGrpSubmitCounter);
        if(grp) {
            sVec < idx > reqList;
            grp2Req(req,&reqList,0,req);
            for (idx ig=0; ig< reqList[ig]; ++ig ) {
                grpAssignReqID(reqList[ig], grp, ig);
            }
        }
    } else {
        idx priority = 0;
        if( priority_cnts && priority_cnts->dim() ) {
            for(idx ip = 0; ip < priority_cnts->dim(); ip++) {
                priority = priority_cnts->ptr(ip)->priority;
                if( priority_cnts->ptr(ip)->cnt ) {
                    break;
                }
            }
        }
        req = reqSubmit(svc, 0, priority);
        idx g = grp ? grp : (pForm ? pForm->ivalue("grp", 0) : 0);
        if( g ) {
            grpAssignReqID(req, g, 0);
        }
    }

    if(req){
        if(pForm) reqSetData(req,"formT.qpride",pForm);

        if( cntParallel > 1 || requiresGroupSubmission ) {
            sVec < idx > reqIds;
            grp2Req(req, &reqIds);
            reqSetAction(&reqIds, autoAction);
        } else {
            reqSetAction(req, autoAction);
        }
    }
    return req;
}

idx sQPrideBase::reqSubmit( const char * serviceName , idx subip, idx priority , idx usrid)
{
    return QPDB->QP_reqSubmit( !serviceName ? vars.value("serviceName") : serviceName , subip, priority, usrid ? usrid : (user ? user->Id() : 0 ));
}

idx sQPrideBase::reqCache( const char * serviceName )
{
    return reqSubmit(serviceName);
}

idx sQPrideBase::reqReSubmit(idx req, idx delaySeconds)
{
    return QPDB->QP_reqReSubmit(&req, 1, delaySeconds);
}

idx sQPrideBase::grpReSubmit(idx grp, const char * serviceName, idx delaySeconds, idx excludeReq)
{
    sVec<idx> reqs;
    grp2Req(grp, &reqs, serviceName);
    if( excludeReq ) {
        for(idx i = 0; i < reqs.dim(); ++i) {
            if( reqs[i] == excludeReq ) {
                reqs.del(i, 1);
            }
        }
    }
    return QPDB->QP_reqReSubmit(reqs.ptr(), reqs.dim(), delaySeconds);
}

idx sQPrideBase::grpSubmit( const char * serviceName, idx subip , idx priority, idx numSubReqs, idx usrid , idx previousGrpSubmit)
{
    return QPDB->QP_grpSubmit(!serviceName ? vars.value("serviceName") : serviceName, subip, priority , numSubReqs , usrid ? usrid : user->Id() , previousGrpSubmit);
}

idx sQPrideBase::grpSubmit2(const char * serviceName, idx subip, const sQPrideBase::PriorityCnt * priority_cnts, idx num_priority_cnts, idx num_subreqs, idx user_id, idx start_subreq, idx grp_id)
{
    return QPDB->QP_grpSubmit2(!serviceName ? vars.value("serviceName") : serviceName, subip, priority_cnts, num_priority_cnts, num_subreqs, user_id ? user_id : user->Id(), start_subreq, grp_id);
}

idx sQPrideBase::reqGrab(const char * serviceName , idx job, idx inBundle,idx status, idx action)
{
    return QPDB->QP_reqGrab(!serviceName ? vars.value("serviceName") : serviceName, job, inBundle,status, action);
}



bool sQPrideBase::reqSetPar(idx req, idx type, const char * value,bool isOverwrite)
{
    return QPDB->QP_reqSetPar(req, type, value, isOverwrite );
}


const char* sQPrideBase::constructReqFilePath(sStr & res, idx req, const char * dataName, bool create)
{
    static sVec<sStr> s_repos;

    if( !s_repos.dim() ) {
        const char * pathList = vars.value("largeDataRepository");
        sStr path00;
        sString::searchAndReplaceSymbols(&path00, pathList, 0, ";,", 0, 0, true, true, false, true);
        path00.add0();
        for(char * p = path00.ptr(); p; p = sString::next00(p)) {
            sStr * r = 0;
            if( sDir::exists(p) ) {
                r = s_repos.add(1);
                if( r ) {
                    r->printf("%s", p);
                    r->add0();
                }
            }
            if( !r ) {
                logOut(sQPrideBase::eQPLogType_Error, "sUsrObj::%s() : failed to read storage path '%s'", __func__, p);
#if _DEBUG
                ::fprintf(stderr, "sUsrObj::%s() : failed to read storage path '%s'\n", __func__, p);
#endif
            }
        }
    }
    if( !s_repos.dim() ) {
        logOut(eQPLogType_Error, "FAILED blob area list empty");
        reqSetInfo(reqId, eQPInfoLevel_Error, "An error occurred during intermediate file creation.");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    if( reqId != req && create ) {
        logOut(eQPLogType_Warning, "Request %" DEC " accessing path for request %" DEC " with create permission", reqId, req);
    }
    const sStr & path = s_repos[req % s_repos.dim()];
    sFilePath t_res("", "%s%s%" DEC "/", path.ptr(), path.ptr(sLen(path.ptr()) - 1)[0] == '/' ? "" : "/", req);
    t_res.simplifyPath();
    if( create ) {
        if( !sDir::exists(t_res.ptr()) ) {
            if( !sDir::makeDir(t_res, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IWOTH) ) {
                logOut(eQPLogType_Error, "FAILED to create blob area '%s': %s", t_res.ptr(), strerror(errno));
                reqSetInfo(reqId, eQPInfoLevel_Error, "An error occurred during intermediate file creation.");
                reqSetStatus(reqId, eQPReqStatus_ProgError);
                return 0;
            }
        }
    } else {
        if( !sDir::exists(t_res.ptr()) ) {
            for(idx i = 0; i < s_repos.dim(); ++i) {
                const sStr & path = s_repos[i];
                t_res.printf(0, "%s%s/%" DEC "/", path.ptr(), path.ptr(sLen(path.ptr()) - 1)[0] == '/' ? "" : "/", req);
                if( sDir::exists(t_res.simplifyPath()) ) {
                    break;
                }
                t_res.cut(0);
            }
        }
    }
    return t_res ? res.printf("%s%s", t_res.ptr(), dataName ? dataName : "") : 0;
}

const char* sQPrideBase::getWorkDir(sStr& path)
{
    idx pos = path.length();
    cfgStr(&path, 0, "qm.tempDirectory");
    if( !path ) {
        path.cut0cut(pos);
        if( !constructReqFilePath(path, reqId, 0, true) ) {
            return 0;
        }
    } else {
        path.printf("%s%" DEC "/", path.ptr(path.length() - 1)[0] == '/' ? "" : "/", reqId);
        if( !sDir::makeDir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) ) {
            logOut(sQPrideBase::eQPLogType_Error, "failed to create directory '%s'", path.ptr());
            return 0;
        }
    }
    return path.ptr(pos);
}

bool sQPrideBase::reqSetData(idx req, const char * dataName, idx datasize, const void * data)
{
    if( !datasize && data ) {
        datasize = sLen(data);
    }
    const char * const fpfx = "file://";
    const idx fpfx_sz = sizeof(fpfx) - 1;
    const bool forceFile = strncmp(dataName, fpfx, fpfx_sz) == 0;
    if( forceFile ) {
        dataName += fpfx_sz;
    }
    sStr path(fpfx);

    bool retval = true;
    if( forceFile || (largeDataReposSize && datasize > sAbs(largeDataReposSize)) ) {
        if ( !constructReqFilePath(path,req,dataName) ) {
            return false;
        }
        if( sFile::exists(path.ptr(fpfx_sz)) ) {
            retval = sFile::remove(path.ptr(fpfx_sz));
        }
        if( retval && datasize ) {

            sFil fl(path.ptr(fpfx_sz));
            retval = fl.ok();
            if( retval ) {
                fl.add((const char*) data, datasize);
                fl.destroy();
                retval = sFile::size(path.ptr(fpfx_sz)) == datasize;
#ifndef WIN32
                if( retval ) {
                    chmod(path.ptr(fpfx_sz), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
                }
#endif
            }
        }
        if( retval ) {
            path.add0();
            data = path.ptr();
            datasize = path.length();
        }
    } else {
        if( sFile::exists(path.ptr(fpfx_sz)) ) {
            retval = sFile::remove(path.ptr(fpfx_sz));
        }
    }
    retval &= QPDB->QP_reqDataSet(req, dataName, datasize, retval ? data : 0);
    return retval;
}

bool sQPrideBase::reqSetData(idx req, const char * dataName, const char * fmt, ... )
{
    sStr str; sCallVarg(str.vprintf,fmt);
    if(!str.length())return false;
    return QPDB->QP_reqDataSet(req, dataName, str.length()+1, str.ptr() );
}

bool sQPrideBase::reqRepackData(idx req, const char * dataName)
{
    if( !largeDataReposSize ) {
        return true;
    }

    sStr path;
    getReqFilePath(path,req,dataName);
    sFil fl(path.ptr(), sMex::fReadonly);
    if( fl.length() < largeDataReposSize ) {
        if( QPDB->QP_reqDataSet(req, dataName, fl.length(), fl.ptr(0)) ) {
            sFile::remove(path.ptr());
            return true;
        } else {
            return false;
        }
    } else {
        sStr dataUrl("file://%s", path.ptr());
        return QPDB->QP_reqDataSet(req, dataName, dataUrl.length() + 1, dataUrl.ptr(0));
    }
}

char * sQPrideBase::reqGetData(idx req, const char * dataName, sMex * data, bool emptyold, idx * timestamp)
{
    if(emptyold)data->empty();
    idx pos=data->pos();

    char * ret=QPDB->QP_reqDataGet(req, dataName, data, timestamp);
    if(largeDataReposSize!=0 && ret && strncmp(ret,"file://",7)==0 ) {

        sFil fl(ret+7,sMex::fReadonly);
        if(fl.length()) {
            if( timestamp )
                *timestamp=sFile::time(ret+7);

            data->cut(pos);
            data->add(fl.ptr(),fl.length());
            return (char*)data->ptr(pos);
        }else
            sFile::remove(ret+7);
    }
    return ret;
}

char * sQPrideBase::reqUseData(idx req, const char * dataName, sMex * data, idx openmode )
{
    char * ret=QPDB->QP_reqDataGet(req, dataName, data);
    if(largeDataReposSize!=0 && ret && strncmp(ret,"file://",7)==0 ) {
        data->init(ret+7,openmode);
        if( (openmode&sMex::fReadonly) && !data->pos()) {
            data->destroy();
            sFile::remove(ret+7);
        }

    }
    return ret;
}

char * sQPrideBase::reqDataPath(idx req, const char * dataName, sStr * path)
{
    sStr t;
    QPDB->QP_reqDataGet(req, dataName, &t);
    if(!t)return 0;
    idx pos=( strncmp((const char*)t.ptr(),"file://",7)==0) ? 7 : 0 ;
    path->printf("%s",t.ptr(pos));
    return path->ptr();
}

idx sQPrideBase::reqDataTimestamp(idx req, const char * dataName)
{
    sStr t;
    idx stamp;
    QPDB->QP_reqDataGet(req, dataName, &t, &stamp);
    if( !t )
        return sIdxMax;
    if( strncmp(t.ptr(), "file://", 7) == 0 )
        return sFile::time(t.ptr(7));
    return stamp;
}

char * sQPrideBase::grpDataPaths(idx grp, const char * dataName, sStr * path, const char * svc, const char * separator)
{
    sVec < idx > reqList; grp2Req(grp, &reqList, svc);
    for( idx ir=0; ir<reqList.dim() ; ++ir ){
        if(ir>0){
            if(separator)path->printf("%s",separator);
            else path->add0();
        }
        reqDataPath(reqList[ir],dataName,path);
    }
    path->add0(2);
    return path->ptr();

}

idx * sQPrideBase::grpDataTimestamps(idx grp, const char * dataName, sVec < idx > * timestampVec)
{
    sVec < idx > reqs;
    grp2Req(grp, &reqs);
    for( idx ir=0; ir<reqs.dim(); ++ir )
        timestampVec->vadd(1, reqDataTimestamp(reqs[ir], dataName));
    return timestampVec->ptr();
}

idx sQPrideBase::grpGetData( idx grp , const char * blobName, sVec < sStr > * dat, sVec < idx > * timestamps )
{
    sVec < idx > reqs;grp2Req(grp, &reqs) ;

    dat->resize(reqs.dim());
    if( timestamps )
        timestamps->resize(reqs.dim());

    for( idx i=0; i<reqs.dim(); ++i)
        reqGetData(reqs[i], blobName, (*dat)[i].mex(), true, timestamps ? timestamps->ptr(i) : 0);
    return reqs.dim();
}

idx sQPrideBase::grpGetData( idx grp , const char * blobName , sMex * data, bool emptyold, const char * separ, idx * timestamp)
{
    sVec < idx > reqs;grp2Req(grp, &reqs) ;
        if(emptyold)data->empty();
        for( idx i=0; i<reqs.dim(); ++i) {
            idx curTimestamp=0;
            if( i && separ) data->add(separ,sLen(separ));
            reqGetData(reqs[i], blobName, data, false, &curTimestamp);
            if( timestamp )
                *timestamp = sMax<idx>(*timestamp, curTimestamp);
        }
    return data->pos();
}

idx sQPrideBase::dataGetAll(idx req, sVec <sStr> * dataVec, sStr * infos00, sVec <idx> * timestamps)
{
    return QPDB->QP_reqDataGetAll(req, dataVec, infos00, timestamps);
}

bool sQPrideBase::reqSetData(idx req, const char * blobName, const sMex * mex)
{
    return reqSetData(req,blobName,mex->pos(),mex->ptr());
}



idx sQPrideBase::requestGet(idx req, sQPrideBase::Request * r)
{
    return QPDB->QP_requestGet(req, (void *)r) ;
}

idx sQPrideBase::requestGetForGrp(idx grp, sVec< sQPrideBase::Request > * r, const char * serviceName)
{
    return QPDB->QP_requestGetForGrp(grp, (void *)r,serviceName) ;
}

idx sQPrideBase::requestGetForGrp2(idx grp, sVec< sQPrideBase::Request > * r, const char * serviceName)
{
    return QPDB->QP_requestGetForGrp2(grp, r,serviceName) ;
}

char * sQPrideBase::requestGetPar(idx req, idx type, sStr * val, const bool req_only)
{
    return QPDB->QP_requestGetPar(req, type, val, req_only);
}


sQPrideBase::QPLogMessage::QPLogMessage()
    : req(0), job(0), level(0), cdate(0)
{
}


void sQPrideBase::QPLogMessage::init(idx preq, idx pjob, idx plevel, real pcdate, const char * ptxt)
{
    req = preq;
    job = pjob;
    level = plevel;
    cdate = pcdate;
    msg.printf("%s", ptxt ? ptxt : "");
}
void sQPrideBase::QPLogMessage::init(sVariant& in)
{
    sVariant* val = in.getDicElt("jobID");
    if(val && val->isInt()) {
        job = val->asInt();
    }
    val = in.getDicElt("level");
    if(val) {
        if(val->isInt()) {
            level = val->asInt();
        }
        else if(val->isString() ) {
            level = sQPrideBase::getLevelCode((char*)val->asString());
        }
    }
    val = in.getDicElt("date");
    if(val && val->isInt()) {
        cdate = val->asInt();
    }
    val = in.getDicElt("msg");
    if(val && val->isString() ) {
        msg.printf("%s", val->asString());
    }
}

const char * sQPrideBase::QPLogMessage::message(void)
{
    return msg.ptr();
}


const char * const sQPrideBase::getLevelName(idx level)
{
    switch(level) {
        case eQPLogType_Trace:
        case eQPInfoLevel_Trace:
            return "Trace";
        case eQPLogType_Debug:
        case eQPInfoLevel_Debug:
            return "Debug";
        case eQPLogType_Info:
        case eQPInfoLevel_Info:
            return "Info";
        case eQPLogType_Warning:
        case eQPInfoLevel_Warning:
            return "Warning";
        case eQPLogType_Error:
        case eQPInfoLevel_Error:
            return "Error";
        case eQPLogType_Fatal:
            return "Fatal";
        default:
            return "";
    }
}

idx sQPrideBase::getLevelCode(const char * level)
{
    idx retVal = eQPInfoLevel_Trace;
    if( level ) {
        if( isalpha(*level) ) {
            sStr buf;
            sString::changeCase(&buf, level, 0, sString::eCaseLo);
            sString::xscanf(buf.ptr(), "%n=0^all^trace^debug^info^warning^error^fatal;", &retVal);
        } else {
            retVal = atoidx(level);
        }
    }
    switch(retVal) {
        case eQPLogType_Trace:
        case eQPInfoLevel_Trace:
        case eQPLogType_Debug:
        case eQPInfoLevel_Debug:
        case eQPLogType_Info:
        case eQPInfoLevel_Info:
        case eQPLogType_Warning:
        case eQPInfoLevel_Warning:
        case eQPLogType_Error:
        case eQPInfoLevel_Error:
        case eQPLogType_Fatal:
            break;
        default:
            retVal = eQPLogType_Trace;
    }
#ifndef _DEBUG
            retVal *= 100;
#endif
    return retVal;
}

idx sQPrideBase::getLog(idx req, bool isGrp, idx job, idx level, sVec<QPLogMessage> & log)
{
    sVarSet res;
    if( QPDB->QP_getLog(req, isGrp, job, level, res) ) {
        const idx sz = log.dim();
        log.add(res.rows);
        for(idx i = 0; i < res.rows; ++i) {
            QPLogMessage & m = log[i + sz];
            m.init(res.ival(i, 0), res.ival(i, 1), res.ival(i, 2), res.rval(i, 3), res(i, 4));
        }
    }
    return res.rows;
}

idx sQPrideBase::getLog2(idx req, idx level, sVec<QPLogMessage> & log)
{
    sVarSet res;
    if( QPDB->QP_getLogGrp(req, level, res) ) {
        const idx sz = log.dim();
        log.add(res.rows);
        for(idx i = 0; i < res.rows; ++i) {
            QPLogMessage & m = log[i + sz];
            m.init(res.ival(i, 0), res.ival(i, 1), res.ival(i, 2), res.rval(i, 3), res(i, 4));
        }
    }
    return res.rows;
}


idx sQPrideBase::reqGetInfo(idx req, idx level, sVec<QPLogMessage> & log)
{
    if( level < eQPInfoLevel_Min) {
        level = eQPInfoLevel_Min;
    } else if( level > eQPInfoLevel_Max ) {
        level = eQPInfoLevel_Max;
    }
    return getLog(req, false, 0, level, log);
}

idx sQPrideBase::grpGetInfo(idx grp, idx level, sVec<QPLogMessage> & log)
{
    if( level < eQPInfoLevel_Min) {
        level = eQPInfoLevel_Min;
    } else if( level > eQPInfoLevel_Max ) {
        level = eQPInfoLevel_Max;
    }
    return getLog(grp, true, 0, level, log);
}

idx sQPrideBase::grpGetInfo2(idx grp, idx level, sVec<QPLogMessage> & log)
{
    return getLog2(grp, level, log);
}

bool sQPrideBase::reqSetInfo(idx req, idx level, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bool ret = vreqSetInfo(req, level, fmt, ap);
    va_end(ap);
    return ret;
}

bool sQPrideBase::vreqSetInfo(idx req, idx level, const char * fmt, va_list ap)
{
    if( (eQPInfoLevel_Min <= level && level <= eQPInfoLevel_Max) || (eQPLogType_Trace <= level && level <= eQPLogType_Max) ) {
        sStr str;
        str.vprintf(fmt, ap);
        if( str ) {
            while( strchr("\n\r \t", *str.last()) != 0 ) {
                str.cut(-1);
            }
            OnLogOut(level, str);
            return QPDB->QP_setLog(req, jobId, level, str.ptr());
        }
    }
    return false;
}

idx sQPrideBase::grpGetStatus( idx grp , sVec < idx > * stat, const char * svcname, idx masterGroup)
{
    sVec < idx > reqs;
        grp2Req(grp, &reqs, svcname, masterGroup);
    QPDB->QP_reqGetStatus(&reqs,stat);
    return stat->dim();
}


idx sQPrideBase::getGroupCntStatusIs(idx grp, idx whatstat, sVec<idx> * stat, sQPrideBase::EGroupStatusCmp statusis, const char * svcname, idx masterGroup)
{
    sVec<idx> l_stat;
    if(!stat) {
        grpGetStatus(grp, &l_stat, svcname, masterGroup);
        stat=&l_stat;
    }
    idx cntis = 0;
    for(idx i = 0; i < stat->dim(); ++i) {
        switch(statusis) {
            case eStatusNotEqual:
                cntis += (*(stat->ptr(i)) != whatstat) ? 1 : 0;
                break;
            case eStatusEqual:
                cntis += (*(stat->ptr(i)) == whatstat) ? 1 : 0;
                break;
            case eStatusLessThan:
                cntis += (*(stat->ptr(i)) < whatstat) ? 1 : 0;
                break;
            case eStatusGreaterThan:
                cntis += (*(stat->ptr(i)) > whatstat) ? 1 : 0;
                break;
        }
    }
    return cntis;
}

idx sQPrideBase::reqSetAction(idx req, idx action)
{
    return QPDB->QP_reqSetAction(req,action);
}

idx sQPrideBase::reqSetAction(sVec <idx> * reqs, idx action)
{
    return QPDB->QP_reqSetAction(reqs,action);
}

idx sQPrideBase::reqGetAction(idx req)
{
    return QPDB->QP_reqGetAction(req);
}

idx sQPrideBase::reqGetUser(idx req)
{
    return QPDB->QP_reqGetUser(req);
}
idx sQPrideBase::reqSetUser(idx req, idx val)
{
    return QPDB->QP_reqSetUser(req, val);
}


idx sQPrideBase::reqSetStatus(idx req, idx status)
{
    return QPDB->QP_reqSetStatus(req,status);
}
idx sQPrideBase::reqSetStatus(sVec <idx> * reqs, idx status)
{
    return QPDB->QP_reqSetStatus( reqs,status);
}
idx sQPrideBase::reqGetStatus(idx req)
{
    return QPDB->QP_reqGetStatus(req);
}
idx sQPrideBase::reqGetStatus(sVec <idx> * reqs, sVec < idx  > * status)
{
    return QPDB->QP_reqGetStatus(reqs,status);
}

idx sQPrideBase::reqSetUserKey(idx req, idx key)
{
    return QPDB->QP_reqSetUserKey(req,key);
}
idx sQPrideBase::reqSetUserKey(sVec <idx> * reqs, idx key)
{
    return QPDB->QP_reqSetUserKey( reqs,key);
}
idx sQPrideBase::reqGetUserKey(idx req)
{
    return QPDB->QP_reqGetUserKey(req);
}
idx sQPrideBase::getReqByUserKey(idx userKey, const char * serviceName)
{
    return QPDB->QP_getReqByUserKey(userKey, serviceName);
}

bool sQPrideBase::reqLock(idx req, const char * key, idx * req_locked_by, idx max_lifetime, bool force)
{
    return QPDB->QP_reqLock(req ? req : this->reqId, key, req_locked_by, max_lifetime, force);
}

bool sQPrideBase::reqUnlock(idx req, const char * key, bool force)
{
    return QPDB->QP_reqUnlock(req ? req : this->reqId, key, force);
}

idx sQPrideBase::reqCheckLock(const char * key)
{
    return QPDB->QP_reqCheckLock(key);
}

idx sQPrideBase::reqProgressStatic(void * param, idx req, idx minReportFrequency, idx items, idx progress, idx progressMax)
{
    sQPrideBase * qp = static_cast<sQPrideBase*>(param);
    return qp ? qp->reqProgress(req, minReportFrequency, items, progress, progressMax) : 1;
}

idx sQPrideBase::reqProgress(idx req, idx minReportFrequency, idx items, idx progress, idx progressMax)
{
    progress = progress2Percent(items, progress, progressMax);
    if( jobRegisterAlive(jobId, req, minReportFrequency) || progress > progress100Last || items < -1 ) {
        progress100Last = progress;
        QPDB->QP_reqSetProgress(req, items < -1 ? -items : items, progress);
        if( reqGetAction(req) == eQPReqAction_Kill ) {
            reqSetStatus(req, eQPReqStatus_Killed);
            return 0;
        }
    }
    return 1;
}

idx sQPrideBase::reqSetProgress(idx req, idx progress, idx progress100)
{
    return QPDB->QP_reqSetProgress(req, progress, progress100);
}


idx sQPrideBase::grpGetProgress(idx grp, idx * progress, idx * progress100)
{
    return QPDB->QP_grpGetProgress( grp, progress, progress100) ;
}

void sQPrideBase::killReq(idx req)
{
    sVec<idx> recVec;
    req2Grp(req, &recVec);
    reqSetAction(&recVec, eQPReqAction_Kill);
}

void sQPrideBase::killGrp(idx req)
{
    sVec<idx> v1, v2;
    req2Grp(req, &v1);
    for(idx i = 0; i < v1.dim(); ++i) {
        grp2Req(v1[i], &v2);
    }
    reqSetAction(&v2, eQPReqAction_Kill);
}

void sQPrideBase::purgeReq(idx req)
{
    sVec<idx> v1, v2;
    req2Grp(req, &v1);
    for(idx i = 0; i < v1.dim(); ++i) {
        grp2Req(v1[i], &v2);
    }
    QPDB->QP_purgeReq(&v2, eQPReqStatus_Killed);
}



bool sQPrideBase::jobRegisterAlive(idx job, idx req, idx notMoreFrequentlySec, bool isReadonly)
{
    sTime t;
    idx timeHasPassedSinceLastTime = t.time(&lazyTime);
    if( notMoreFrequentlySec && timeHasPassedSinceLastTime < notMoreFrequentlySec ) {
        return false;
    }
    if( !isReadonly ) {
        if( job ) {
            QPDB->QP_jobRegisterAlive(job);
        }
        if( req ) {
            QPDB->QP_reqRegisterAlive(req);
        }
        lazyTime = t;
    }
    return true;
}

idx sQPrideBase::jobSetStatus(idx job, idx jobstat)
{
    return QPDB->QP_jobSetStatus(job, jobstat);
}

idx sQPrideBase::jobSetStatus(sVec < idx > *  jobs, idx jobstat)
{
    return QPDB->QP_jobSetStatus(jobs, jobstat);
}

idx sQPrideBase::jobSetAction(sVec < idx > *  jobs, idx jobact)
{
    return QPDB->QP_jobSetAction(jobs, jobact);
}

idx sQPrideBase::jobRegister(const char * serviceName, const char * hostName, idx pid, idx inParallel)
{
    return QPDB->QP_jobRegister(serviceName, hostName, pid, inParallel);
}

idx sQPrideBase::jobSetMem(idx job, idx curMemSize, idx maxMemSize)
{
    return QPDB->QP_jobSetMem( job, curMemSize, maxMemSize);
}

idx sQPrideBase::jobGetAction(idx jobId)
{
    return QPDB->QP_jobGetAction(jobId);
}

idx sQPrideBase::jobSetReq(idx job, idx req)
{
    return QPDB->QP_jobSetReq(job, req);
}


idx sQPrideBase::grpAssignReqID(idx req, idx grp, idx jobIDSerial)
{
    return QPDB->QP_grpAssignReqID(grp, req, jobIDSerial) ;
}
idx sQPrideBase::req2Grp(idx req, sVec<idx> * grpIds, bool isMaster)
{
    return QPDB->QP_req2Grp(req, grpIds,isMaster);
}
idx sQPrideBase::req2GrpSerial(idx req, idx grp, idx * pcnt, idx svc)
{
    return QPDB->QP_req2GrpSerial(req, grp, pcnt,svc);
}

idx sQPrideBase::grp2Req(idx grp, sVec<idx> * reqIds, const char * svc, idx masterGroup, bool returnSelf)
{
    sVec<idx> reqs;
    if(!reqIds)reqIds=&reqs;
    QPDB->QP_grp2Req(grp, reqIds, svc,masterGroup);
    if(returnSelf && !reqIds->dim())reqIds->vadd(1,grp);
    return reqIds->dim();
}






idx sQPrideBase::parSplit(idx globalStart, idx globalEnd, idx parallelJobs, idx parallelThreads, idx myJobNum, idx myThreadNum, idx * myStart, idx * myEnd)
{
    if(parallelJobs==0)parallelJobs=1;
    if(parallelThreads==0)parallelThreads=1;

    idx globalCnt=globalEnd-globalStart;
    idx chunkForJob=globalCnt/parallelJobs;
    idx myJobStart=myJobNum ? chunkForJob*(myJobNum-1) : globalStart;
    idx myJobEnd=myJobNum ? myJobStart+chunkForJob : globalEnd;
        if(myJobNum && (myJobNum-1)==parallelJobs-1)
            myJobEnd=globalEnd;

    idx cntForJob=myJobEnd-myJobStart;
    idx chunkForThread=cntForJob/parallelThreads;
    idx myThreadStart=myJobStart+myThreadNum*chunkForThread;
    idx myThreadEnd=myThreadStart+chunkForThread;
        if(myThreadNum==parallelThreads-1)
            myThreadEnd=myJobEnd;

    if(myStart)*myStart=myThreadStart;
    if(myEnd)*myEnd=myThreadEnd;
    return myThreadEnd-myThreadStart;
}

sQPrideBase::ThreadSpecific * sQPrideBase::parSplitForThread(idx globalStart, idx globalEnd, idx myJobNum, idx myThreadID, sQPrideBase::Service * svc)
{
    thrSpc.resize(myThreadID+1);
    ThreadSpecific * t=&thrSpc[myThreadID];
    t->iThreadNum=myThreadID;
    t->progressDone=0;
    t->thisObj=this;

    parSplit(globalStart, globalEnd, svc->parallelJobs, threadsCnt, myJobNum, t->iThreadNum, &t->iThreadStart, &t->iThreadEnd);
    return t;
}

idx sQPrideBase::parSplitForJob(idx globalStart, idx globalEnd, idx myJobNum, sQPrideBase::Service * svc)
{

    return parSplit(globalStart,globalEnd, svc->parallelJobs, 1, myJobNum, 0, &iJobArrStart,&iJobArrEnd);
}


idx sQPrideBase::parProgressReport(idx req, const char * blbname)
{
    sStr t;
    idx percent100, progressDone=0;

    t.printf("%s,%" DEC ",%" DEC,vars.value("thisHostName"),req,thrSpc.dim());

    ThreadSpecific * tr;
    for( idx it=0; it<thrSpc.dim();  ++it) {
        tr=thrSpc.ptr(it);
        percent100=tr->progressDone*100/(tr->iThreadEnd-tr->iThreadStart);
        t.printf(",%" DEC ",%" DEC,tr->progressDone,percent100);
        progressDone+=tr->progressDone;
    }
    percent100=progressDone*100/(iJobArrEnd-iJobArrStart);
    t.printf(",%" DEC ",%" DEC,progressDone,percent100);

    t.printf("//");

    reqSetData(req,blbname,&t);
    reqProgress(req, 0, progressDone, percent100,100);
    return progressDone;
}


idx sQPrideBase::parReqsStillIncomplete(idx grp, const char * svcOnly)
{
    idx incompl=0;
    sVec < idx > stas;
    grpGetStatus( grp , &stas, svcOnly);

    if(!stas.dim()) {
        return (reqGetStatus(grp)<=eQPReqStatus_Running) ? 1 : 0;
    }
    for  ( idx i =0;  i< stas.dim(); ++i) {
        if(stas[i]<=eQPReqStatus_Running)
            ++incompl;
    }

    return incompl;
}


idx sQPrideBase::parReportingThread(void)
{
    for( idx it=0; it<thrSpc.dim();  ++it)
        if(!thrSpc[it].isThreadDone)return it;
    return 0;
}


idx sQPrideBase::sysPeekOnHost(sVec < Service > * srvlst, const char * hostname )
{
    if(!hostname) hostname=vars.value("thisHostName");
    return QPDB->QP_sysPeekOnHost(srvlst, hostname);
}

bool sQPrideBase::hostNameMatch (const char * rp, const char * hostName, idx * pmaxjob)
{
    return QPDB->hostNameMatch(rp, hostName,pmaxjob);
}

bool sQPrideBase::hostNameListMatch (const char * rp, const char * hostName, idx * pmaxjob, idx * pwhich)
{
    return QPDB->hostNameListMatch(rp, hostName,pmaxjob);
}

idx sQPrideBase::sysPeekReqOrder(idx req, const char * srv , idx * pRunning )
{
    return QPDB->QP_sysPeekReqOrder( req, srv , pRunning );
}


idx sQPrideBase::sysGetKnockoutJobs(sVec < Job > * jobs, sVec < idx > * svcIDs)
{
    return QPDB->QP_sysGetKnockoutJobs(jobs,svcIDs);
}

idx sQPrideBase::sysGetImpoliteJobs(sVec < Job > * jobs, sVec < idx > * svcIDs)
{
    return QPDB->QP_sysGetImpoliteJobs(jobs,svcIDs);
}

idx sQPrideBase::sysJobsGetList(sVec < Job > * jobs , idx stat, idx act, const char * hostname )
{
    if(!hostname) hostname=vars.value("thisHostName");
    return QPDB->QP_sysJobsGetList(jobs,stat,act,hostname);
}

idx sQPrideBase::sysRecoverRequests(sVec <idx > * killed, sVec < idx > * recovered)
{
    return QPDB->QP_sysRecoverRequests(killed,recovered);
}

idx sQPrideBase::sysCapacityNeed(idx * capacity_total)
{
    return QPDB->QP_sysCapacityNeed(capacity_total);
}

char * sQPrideBase::resourceGet(const char * service, const char * resName,    sMex * data, idx * timestamp)
{
    return QPDB->QP_resourceGet(service,resName,data, timestamp);
}

bool sQPrideBase::resourceSet(const char * service, const char * resName, idx ressize , const void * data)
{
    return QPDB->QP_resourceSet(service,resName,ressize,data);
}
idx sQPrideBase::resourceGetAll(const char * service, sStr * infos00, sVec < sStr > * dataVec,  sVec < idx > * tmStmps )
{
    return QPDB->QP_resourceGetAll(service, infos00, dataVec,tmStmps);
}

bool sQPrideBase::resourceDel(const char * service, const char * resName)
{
    return QPDB->QP_resourceDel(service,resName);
}


idx sQPrideBase::resourceSync(const char * resourceRoot, const char * service, const char * platformSpec)
{
    sStr toExec, toSymlink, rslst00, blob, dst, src00, log;
    bool logHasErr = false;
    sVec<idx> tmStmps;
    resourceGetAll(service, &rslst00, 0, &tmStmps);
    if( resourceRoot ) {
        sDir::chDir(resourceRoot);
    }
    sUsrQueryEngine * qengine = 0;
    sUsr * curr_user = user;
    const char * rs = rslst00;
    for(idx ir = 0; rs; rs = sString::next00(rs), ++ir) {
        if( platformSpec ) {
            const char * plat = strstr(rs, ".os");
            if( plat && strncmp(platformSpec, plat + 3, sLen(platformSpec)) != 0 ) {
                continue;
            }
        }
        blob.cut0cut();
        src00.cut0cut();
        resourceGet(service, rs, blob.mex(), 0);
        if( !blob.length() ) {
            log.printf("\tEMPTY resource blob is ignored for %s@%s\n", rs, service);
            logHasErr = true;
            continue;
        }
        bool iscmd = false, islink = false, isQry = false;
        const char * rs0 = rs;
        idx srcTime = tmStmps[ir];
        sStr rsScript;
        if( strncasecmp(rs, "query://", 8) == 0 ) {
            rs += 8;
            isQry = true;
        }
        if( rs[0] == '>' ) {
            ++rs;
            iscmd = true;
        } else if( rs[0] == '-' && rs[1] == '>' ) {
            rs += 2;
            islink = isQry ? false : true;
        }
        if( isQry ) {
            sStr lerr;
            if( !curr_user ) {
                curr_user = new sUsr("qpride", true);
            }
            if( curr_user && curr_user->Id()) {
                if( !qengine ) {
                    qengine = new sUsrInternalQueryEngine(this, *curr_user);
                }
                if( qengine ) {
                    qengine->parse(blob.ptr(), blob.length(), &lerr);
                    if( !lerr ) {
                        sVariant * v = qengine->run(&lerr);
                        if( !lerr ) {
                            if( v && v->isList() && v->dim() > 0 ) {
                                sHiveId id;
                                v->getListElt(0)->asHiveId(&id);
                                sUsrObj * obj = curr_user->objFactory(id);
                                if( obj ) {
                                    log.printf("\tresource %s@%s using object %s\n", rs0, service, obj->Id().print());
                                    sDir files;
                                    const idx len = sLen(rs);
                                    if( len && (rs[len - 1] == '/' || rs[len - 1] == '\\') ) {
                                        const idx flags = sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs);
                                        obj->files(files, flags);
                                    } else {
                                        files._list00.printf(0, "%s", rs);
                                        files._list00.add0(2);
                                    }
                                    srcTime = 0;
                                    for(const char * f = files._list00; f; f = sString::next00(f)) {
                                        const char * fpath = obj->getFilePathname(src00, "%s", f);
                                        if( !fpath ) {
                                            lerr.printf("file '%s' not found in object %s", f, id.print());
                                        } else {
                                            const idx tm = sFile::time(fpath, true);
                                            if( tm > srcTime ) {
                                                srcTime = tm;
                                            }
                                            src00.add0();
                                        }
                                    }
                                    src00.add0(2);
                                    obj->propGet("init_script", &rsScript);
                                    delete obj;
                                } else {
                                    lerr.printf("object %s not found or permission denied", id.print());
                                }
                            } else {
                                lerr.printf("query result is empty or not a list of objects");
                            }
                        }
                    }
                } else {
                    lerr.printf("cannot setup query engine - insufficient resources");
                }
            } else {
                lerr.printf("cannot setup query engine - no user");
            }
            if( lerr ) {
                log.printf("\tquery failed %s@%s: %s\n", rs, service, lerr.ptr());
                logHasErr = true;
                continue;
            }
        }

        if( *rs == '\\' || *rs == '/' ) {
            dst.printf(0, "%s", rs);
        } else if( *rs == '~' ) {
            const char * hm = getenv(
#ifdef WIN32
                "USERPROFILE"
#else
                "HOME"
#endif
                );
            if( hm && hm[0] ) {
                dst.printf(0, "%s%s", hm, &rs[1]);
            } else {
                log.printf("\tcannot resolve home directory for resource %s@%s\n", rs, service);
                logHasErr = true;
            }
        } else {
            dst.printf(0, "%s%s", resourceRoot ? resourceRoot : "./", rs);
        }
        const bool dstIsDir = dst.last()[-1] == '/' || dst.last()[-1] == '\\';
        const idx mtm = sFile::time(dst, false);
        if( mtm != sIdxMax && srcTime != sIdxMax && srcTime <= mtm ) {
            sStr buf;
            const char * s = sString::printDateTime(buf, srcTime);
            buf.add0(2);
            const char * m = sString::printDateTime(buf, mtm);
            log.printf("\tresource %s@%s is up-to-date: %s <= %s\n", rs0, service, s, m);
            continue;
        }
        log.printf("\tresource %s@%s %" DEC " %s to %s\n", rs0, service, src00 ? sString::cnt00(src00) : blob.length(), src00 ? "files" : "bytes", dst.ptr());
        if( sDir::exists(dst) ) {
            if( !sDir::removeDir(dst, true) ) {
                log.printf("\tfailed to delete directory '%s'\n", dst.ptr());
                logHasErr = true;
                continue;
            }
            log.printf("\tdeleted directory '%s'\n", dst.ptr());
        } else if( sFile::exists(dst, false) ) {
            if( !sFile::remove(dst) ) {
                log.printf("\tfailed to delete file '%s..'\n", dst.ptr());
                logHasErr = true;
                continue;
            }
            log.printf("\tdeleted file '%s..'\n", dst.ptr());
        }
        if( src00 ) {
            if( dstIsDir ) {
                if( !sDir::makeDir(dst, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH) ) {
                    log.printf("\tFAILED to create path '%s'\n", dst.ptr());
                    logHasErr = true;
                    continue;
                }
                struct stat st;
                if( stat(dst, &st) == 0 ) {
                    st.st_mtime = srcTime;
                    sFile::setAttributes(dst, &st);
                }

            }
            for(const char * entry = src00; entry; entry = sString::next00(entry)) {
                sFilePath p(entry, "%s%%flnm", dstIsDir ? dst.ptr() : "");
                idx numCopied = 1;
                if( (sDir::exists(entry) && sDir::copyDir(entry, p, false, &numCopied) ) ||
                    (sFile::exists(entry, false) && sFile::copy(entry, p, false, false)) ) {
                    logOut(eQPLogType_Debug, "for resource %s %" DEC " files copied %s to %s\n", rs0, numCopied, entry, p.ptr());
                } else {
                    log.printf("\tFAILED to copy resource %s@%s %s to %s: %s\n", rs0, service, entry, p.ptr(), strerror(errno));
                    logHasErr = true;
                    continue;
                }
            }
            if( iscmd ) {
                toExec.printf("%s", dst.ptr());
                toExec.add0();
            }
        } else {
            if( islink ) {
                toSymlink.printf("%.*s", (int) blob.length(), blob.ptr());
                toSymlink.add0();
                toSymlink.printf("%s", rs);
                toSymlink.add0();
            } else {
                if( iscmd ) {
                    toExec.printf("%s", dst.ptr());
                    toExec.add0();
                }
                sFilePath ppp(dst, "%%dir");
                if( ppp ) {
                    if( !sDir::makeDir(ppp, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH) ) {
                        log.printf("\tFAILED to create path '%s'\n", ppp.ptr());
                        logHasErr = true;
                        continue;
                    }
                }
                if( !dstIsDir ) {
                    int fl = open(dst, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                    if( fl < 0 ) {
                        log.printf("\tFAILED to open resource %s@%s to %s: %s\n", rs0, service, dst.ptr(), strerror(errno));
                        logHasErr = true;
                        continue;
                    }
                    idx written = 0, togo = blob.length();
                    errno = 0;
                    while( togo - written > 0 && errno == 0 ) {
                        written += write(fl, blob.ptr(written), togo - written);
                    }
                    if( errno != 0 ) {
                        log.printf("\tFAILED to write resource %s@%s to %s: %s\n", rs0, service, dst.ptr(), strerror(errno));
                        logHasErr = true;
                    }
                    close(fl);
                }
#ifndef SLIB_WIN
                chmod(dst, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
            }
        }
        if(rsScript) {
            toExec.printf("cd '%s' && './%s'", dst.ptr(), rsScript.ptr());
            toExec.add0();
        }
    }
    if( curr_user != user ) {
        delete curr_user;
    }
    delete qengine;
    toSymlink.add0(2);
    for(rs = toSymlink.ptr(); rs && *rs; rs = sString::next00(rs)) {
        const char * lnk = sString::next00(rs);
        if( !lnk || !lnk[0] ) {
            log.printf("\tpath '%s' missing symlink name\n", rs);
            logHasErr = true;
        } else if( !sFile::symlink(rs, lnk) ) {
            log.printf("\tfailed to symlink '%s' to '%s'\n", rs, lnk);
            logHasErr = true;
        } else {
            log.printf("\tsymlinked '%s' to '%s'\n", rs, lnk);
        }
        rs = lnk;
    }
    toExec.add0(2);
    for(rs = toExec.ptr(); rs && *rs; rs = sString::next00(rs)) {
        log.printf("\texecuting for %s: '%s'\n", service, rs);
#ifndef SLIB_WIN
        chmod(rs, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
        const idx ret = sPS::execute(rs);
        if( ret != 0 ) {
            log.printf("\texecution returned %" DEC "\n", ret);
            logHasErr = true;
        }
    }
    if( log.length() ) {
        logOut(logHasErr ? eQPLogType_Error : eQPLogType_Info, " synchronizing:\n%s\n", log.ptr());
    }
    return 1;
}
