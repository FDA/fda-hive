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
#include <qlib/QPrideCGI.hpp>

bool sQPrideCGI::OnCGIInit(void)
{
    reqId = pForm->ivalue("req");
    cookieSecureOverHTTP(cfgInt(0, "user.cookieSecureOverHTTP", 1));

    bool res = sUsrCGI::OnCGIInit();
    sQPrideBase::user = &m_User;

    if( m_backEndUser ) {
        sQPrideBase::user = m_backEndUser;
    }

    cntParallel = 0;
    requiresGroupSubmission = false;
    objs.cut(0);
    sVec<sHiveId> objIds;
    const char * objsS = pForm->value("objs");
    const char * objLink = pForm->value("objLink");

    sHiveId::parseRangeSet(objIds, objsS);
    for(idx io = 0; io < objIds.dim(); ++io) {

        if(objLink) {
            sUsrObj ol(*user, objIds[io]);
            if(ol.Id()) {
                objIds[io].parse(ol.propGet(objLink));
            }

        }

        idx cnt = objs.dim();
        sUsrObj * p = objs.add(1);
        new (p) sUsrObj(*user, objIds[io]);

        if( !objs[cnt].Id() )
            objs.cut(cnt);
    }

    sStr rootPath;
    sRC rc = sUsrObj::initStorage(cfgStr(&rootPath, 0, "user.rootStoreManager"), cfgInt(0, "user.storageMinKeepFree", (udx)20 * 1024 * 1024 * 1024), this);
    if( rc ) {
        logOut(eQPLogType_Error, "%s", rc.print());
        res = false;
    }

    return res;
}

bool sQPrideCGI::runAsBackEnd(void)
{
    return formBoolValue("-daemon");
}

idx sQPrideCGI::run(const char * rcmd)
{
    if( formBoolValue("backend",false) && !runAsBackEnd() ) {
        return sCGI::run("-qpRawSubmit");
    }
    return sCGI::run();
}

const char * sQPrideCGI::formValue(const char * prop, sStr * buf, const char * defaultValue, idx iObj )
{

    const char * propVal=0;
    if( objs.dim()>iObj ) {
        propVal=objs[iObj].propGet00(prop, buf, "\n");
    }
    if(!propVal) {
        propVal=pForm->value(prop, defaultValue);
        if(propVal && buf)
            propVal=buf->printf("%s",propVal);

    }

    if(!propVal) {
        if( buf && defaultValue)
            propVal=buf->printf("%s",defaultValue);
        else propVal=defaultValue;
    }

    return propVal;

}

char * sQPrideCGI::listObjOrReqFiles( sStr * pathlist, const char * flnm00, const char * svc, const char* separator, bool forceReq )
{
    pathlist->cut(0);
    const char * flnm = flnm00;
    idx startList = 0, endList = objs.dim();
    sVec<idx> reqIds;
    bool isObj = true;

    if( !objs.dim() || forceReq ) {
        if( reqId > 0 ) {
            grp2Req(reqId, &reqIds, svc);
        } else {
            reqId = -reqId;
        }
        if( !reqIds.dim() ) {
            reqIds.vadd(1, reqId);
        }
        endList = reqIds.dim();
        flnm = sString::next00(flnm);
        isObj = false;
    }
    for(idx is = startList; is < endList; ++is) {
        sStr path;
        if( isObj && !objs[is].getFilePathname(path, "%s", flnm) ) {
            continue;
        } else {
            reqDataPath(reqIds[is], flnm, &path);
        }
        if(!path.length() || !sFile::size(path)) {
            continue;
        }
        if(is>startList) {
            if(separator)pathlist->printf("%s",separator);
            else pathlist->add0();
        }
        pathlist->printf("%s",path.ptr());
    }
    if(!forceReq && isObj && !pathlist->length() ) {
        pathlist->cut(0);
        return listObjOrReqFiles( pathlist, flnm00, svc,separator,true);
    }
    if(!separator) {
        pathlist->add0(2);
    }
    return pathlist->ptr();
}

idx sQPrideCGI::QPSubmit(sVar & forma, bool withObjs, sQPride::Service & S, sStr * strObjList)
{
    sVec<sUsrProc> procObjs;
    sStr objList, log;
    idx err = 0;
    if( withObjs ) {
        if( !strObjList ) {
            strObjList = &objList;
        }
        err = sUsrProc::createProcesForsubmission(this, &forma, user, procObjs, &S, strObjList, &log);
        if( err ) {
            for(idx ip = 0; ip < procObjs.dim(); ++ip) {
                procObjs[ip].actDelete();
            }
            error("Failure: to set process:\n%s\n", log.ptr());
            return 1;
        }
    }
    sMex *fileSlices = 0;
    cntParallel = customizeSubmission(&forma, user, procObjs.dim() ? procObjs.ptr(0) : 0, &S, &log, &fileSlices);
    if( !cntParallel ) {
        for(idx ip = 0; ip < procObjs.dim(); ++ip) {
            procObjs[ip].actDelete();
        }
        error("Failure: to customize submission:\n%s\n", log.ptr());
        return 1;
    }
    if( cntParallel < 1 || (cntParallel > 20000 && pForm->ivalue("serviceSubmissionLarge", 0) != 13) ) {
        for(idx ip = 0; ip < procObjs.dim(); ++ip) {
            procObjs[ip].actDelete();
        }
        error("Failure: submission too large %" DEC " \n", cntParallel);
        return 1;
    }
    err = sUsrProc::standardizedSubmission(this, &forma, user, procObjs, cntParallel, &reqId, &S, 0, strObjList, &log, 0, forma.boolvalue("forceGroup"));
    if( err ) {
        error("%s\n", log.ptr());
        return 1;
    }

    for(idx ip = 0; ip < procObjs.dim(); ++ip) {
        procObjs[ip].propSync();
    }
    return 0;
}

static void addJsonError(sJSONPrinter & printer, const sHiveId & id, const char * signame, const char * msg)
{
    printer.addKey(id.print());
    printer.startObject();
    {
        printer.addKeyValue("signal", signame);
        printer.addKey("data");
        printer.startObject();
        {
            printer.addKeyValue("error", msg);
        }
        printer.endObject();
    }
    printer.endObject();
}

idx sQPrideCGI::Cmd(const char * cmd)
{
    enum enumCommands{
        eQPSubmit,eQPCheck,eQPData,eQPDataNames,eQPFile,eQPResubmit,eQPReqInfo,eQPKillReq,eQPRawSubmit,eQPRawCheck,eQPProcSubmit,eQPProcReSubmit,eQPProcClone,
        eQPReqSetAction,
        eQPReqRegisterAlive,
        eQPPerformRegisterTime,eQPPerformEstimateTime,
        eQPGRList,
        eQPRQList,
        eLast
    };
    const char * listCommands=
        "-qpSubmit" _ "-qpCheck" _ "-qpData" _ "-qpDataNames" _ "-qpFile" _ "-qpResubmit" _ "-qpReqInfo" _ "-qpKillReq" _ "-qpRawSubmit" _ "-qpRawCheck" _ "-qpProcSubmit" _ "-qpProcReSubmit" _ "-qpProcClone" _
        "-qpReqSetAction" _
        "-qpReqRegisterAlive" _
        "-qpPerformReg" _ "-qpPerformETA" _
        "-qpGRList" _
        "-qpRQList" _
        __;
    idx cmdnum = -1;
    if( cmd ) {
        sString::compareChoice(cmd, listCommands, &cmdnum, false, 0, true);
    }
    if( reqId < 0 ) {
        reqId = getReqByUserKey(reqId);
    }
    sHiveId reqObjID(pForm->value("reqObjID"));
    bool need_check_req_auth = true;
    if( reqObjID ) {
        sUsrProc pr(*user, reqObjID);
        
        if( objs.dim() <= 0 ){
            sUsrObj * p = objs.add(1);
            new (p) sUsrObj(*user, reqObjID);
            if( !objs[0].Id() ) {
                objs.cut(0);
            }
        }

        if( pr.Id() ) {
            reqId = pr.reqID();
            need_check_req_auth = false;
        }
    }

    if( objs.dim() <= 0 && reqId > 0 ) 
    {
        sStr strObjList;
        sVec<sHiveId> objIds;
        requestGetPar(reqId, eQPReqPar_Objects, &strObjList, true);
        if(strObjList.length() < 2) {
            strObjList.cut0cut();
            requestGetPar(grpId, eQPReqPar_Objects, &strObjList, false);
        }

        if( strObjList )
            sHiveId::parseRangeSet(objIds, strObjList, strObjList.length());
        
        if(objIds.dim() > 0) {
            idx cnt = objs.dim();
            sUsrObj * p = objs.add(1);
            new (p) sUsrObj(*user, objIds[0]);
            if( !objs[cnt].Id() ) {
                objs.cut(cnt);
            }
        }
    }

    sQPride::Request R;
    sSet(&R);
    if( reqId ) {
        requestGet(reqId, &R);
        if( need_check_req_auth && !reqAuthorized(R) ) {
            reqId = 0;
            sSet(&R);
            pForm->inp("req", "0");
        }
    }

    sQPride::Service S;
    const char * svc = pForm->value("svc");
    if( svc && *svc == 0 ) {
        svc = 0;
    }
    if( svc ) {
        vars.inp("serviceName", svc);
        serviceGet(&S, svc, 0);
    } else if( reqId ) {
        serviceGet(&S, 0, R.svcID);
        vars.inp("serviceName", (svc = S.name));
    } else {
        svc = vars.value("serviceName");
        serviceGet(&S, svc, 0);
    }
    idx ret = 0;
    if( ret ) {
        outHtml();
        return ret;
    }

    sStr tmp, buf;

    switch(cmdnum) {

        case eQPResubmit: {
            bool doResubmit = true, isObj = false;
            if( pForm ) {
                if( objs.dim() ) {
                    user->updateStart();
                    isObj = true;
                    reqId=objs[0].propGetI("reqID");
                    if(!reqId) {
                        error("Failure: cannot associate object %s with an reqId", objs[0].IdStr());
                        doResubmit = false;
                    } else {
                        reqSetData(reqId, "formT.qpride", pForm);
                        if( !objs[0].propInit(true) ) {
                            error("Failure: cannot reset the properties of the object %s", objs[0].IdStr());
                            doResubmit = false;
                        }
                        sStr log;
                        if( doResubmit && !user->propSet(*pForm, log) ) {
                            doResubmit = false;
                            error(log);
                        }
                    }
                    if(doResubmit)
                        user->updateComplete();
                    else
                        user->updateAbandon();
                } else
                    reqSetData(reqId, "formT.qpride", pForm);
            }
            if(doResubmit) {
                reqReSubmit(reqId);
                reqSetAction(reqId, eQPReqAction_Run);
                if( !isObj ) {
                    linkSelf("-qpCheck", "req=%" DEC "&svc=%s", reqId, svc);
                } else {
                    dataForm.printf("%" DEC ",%s", reqId, objs[0].IdStr());
                }
            } else {
            }
            outHtml();
        }
            return 1;

        case eQPProcClone: {
            sVec<sHiveId> src_ids;
            sVec<sUsrProc *> srcs(sMex::fExactSize|sMex::fSetZero);
            sStr newObjList;
            sHiveId::parseRangeSet(src_ids, pForm->value("src_ids"));
            srcs.resize(src_ids.dim());
            idx cnt_valid_srcs = 0;

            sJSONPrinter printer(&dataForm);
            bool is_json = sIsExactly(pForm->value("mode", "json"), "json") && src_ids.dim();
            if( is_json ) {
                printer.startObject();
            }

            for(idx i = 0; i < src_ids.dim(); ++i) {
                sUsrObj * obj = user->objFactory(src_ids[i]);
                if( obj ) {
                    sUsrProc * pobj = dynamic_cast<sUsrProc *>(obj);
                    if( pobj && pobj->isTypeOf("^process$+") ) {
                        srcs[i] = pobj;
                        obj = 0;
                        cnt_valid_srcs++;
                    } else {
                        if( is_json ) {
                            addJsonError(printer, src_ids[i], "clone", "object is not a process");
                        } else {
                            error("Failure: cannot clone '%s' - not a process", src_ids[i].print());
                        }
                    }
                } else {
                    if( is_json ) {
                        addJsonError(printer, src_ids[i], "clone", "object not found");
                    } else {
                        error("Failure: cannot clone '%s' - not found", src_ids[i].print());
                    }
                }
                delete obj;
            }
            if( !src_ids.dim() ) {
                error("Failure: cannot clone - empty src_ids list");
            } else if( srcs.dim() == cnt_valid_srcs ) {
                sStr nm, buf, objList;
                sVar forma;
                for(idx i = 0; i < srcs.dim(); ++i) {
                    sUsrProc * pobj = srcs[i];
                    sVarSet v;
                    if( pobj->propBulk(v) ) {
                        v.addRow().addCol(pobj->Id()).addCol("_type").addCol((const char*) 0).addCol(pobj->getTypeName());
                        buf.cut(0);
                        pForm->serialOut(&buf);
                        forma.empty();
                        forma.serialIn(buf, buf.length());
                        sQPride::Service sss;
                        for(idx r = 0; r < v.rows; ++r) {
                            const char * id = v.val(r, 0);
                            const char * name = v.val(r, 1);
                            nm.printf(0, "prop.clone_%s.%s", id, name);
                            const char * path = v.val(r, 2);
                            if( path && path[0] ) {
                                nm.printf(".%s", path);
                            }
                            nm.add0(3);
                            const char * val = v.val(r, 3);
                            if( strcasecmp("name", name) == 0 ) {
                                val = nm.printf("Resubmitted %s: %s", id, val);
                            } else if( strcasecmp("svc", name) == 0 ) {
                                serviceGet(&sss, val, 0);
                                forma.inp(name, val);
                            }
                            forma.inp(nm, val);
#if _DEBUG
                            fprintf(stderr, "\n%s=%s", nm.ptr(), val);
#endif
                        }
                        objList.cut0cut();
                        if( !sss.svcID || QPSubmit(forma, true, sss, &objList) ) {
                            if( is_json ) {
                                addJsonError(printer, src_ids[i], "clone", "submit failed");
                            } else {
                                error("Failure: cannot clone '%s' - submit failed", pobj->Id().print());
                                newObjList.printf("-1 ");
                            }
                        } else {
                            if( is_json ) {
                                printer.addKey(pobj->Id().print());
                                printer.startObject();
                                {
                                    printer.addKeyValue("signal", "clone");
                                    printer.addKey("data");
                                    printer.startObject();
                                    {
                                        sHiveId to_id(objList.ptr());
                                        printer.addKeyValue("to", to_id);
                                    }
                                    printer.endObject();
                                }
                                printer.endObject();
                            } else {
                                newObjList.printf("%s ", objList.ptr());
                            }
                        }
                    } else {
                        if( is_json ) {
                            addJsonError(printer, src_ids[i], "clone", "object is empty");
                        } else {
                            error("Failure: cannot clone '%s' - object empty", pobj->Id().print());
                        }
                    }
                    delete pobj;
                }
            } else {
                for(idx i = 0; i < srcs.dim(); ++i) {
                    if( is_json ) {
                        if( srcs[i] ) {
                            addJsonError(printer, src_ids[i], "clone", "not resubmitted because of validation errors in other objects");
                        }
                    } else {
                        newObjList.printf("-1 ");
                    }
                    delete srcs[i];
                    srcs[i] = 0;
                }
            }

            if( is_json ) {
                printer.endObject();
            } else {
                dataForm.cut0cut();
                dataForm.printf("%s\n", newObjList.ptr());
                raw = 1;
            }
            outHtml();
            return 1;
        }
        case eQPProcReSubmit:
        case eQPRawSubmit:
        case eQPProcSubmit:
        case eQPSubmit: {
            if(cmdnum == eQPProcReSubmit) {
                user->updateStart();
                sVarSet tbl;
                sStr tmp_log;
                user->propSet(*pForm,tmp_log,tbl,true);
                const char * block_id = sStr::zero;
                const char * row_id = sStr::zero;
                bool failed_to_reset_id = false;
                for(idx irow = 0; irow < tbl.rows; ++irow) {
                    block_id = tbl.val(irow, 0);
                    if( !block_id ) {
                        block_id = sStr::zero;
                    }
                    if( strcmp(block_id, row_id) != 0 ) {
                        sHiveId hid(block_id);
                        if( !sUsrObj::propInit(*user,hid) ) {
                            error("Failure: cannot reset the properties of the object %s", hid.print());
                            failed_to_reset_id = true;
                        }
                    }
                    row_id = block_id;
                }
                if(failed_to_reset_id) {
                    user->updateAbandon();
                    outHtml();
                    return 1;
                } else {
                    user->updateComplete();
                }
            }
            sStr strObjList;
            bool withObjs = (cmdnum == eQPProcSubmit) || (cmdnum == eQPProcReSubmit);
            if( QPSubmit(*pForm, withObjs, S, &strObjList) ) {
                outHtml();
                return 1;
            }
            idx isdelay = pForm->ivalue("delay", 0);
            if( isdelay ) {
                sleepMS(isdelay);
            }
            if( !raw ) {
                if( cmdnum == eQPRawSubmit ) {
                    linkSelf("-qpRawCheck", "req=%" DEC "&svc=%s", reqId, svc);
                } else {
                    linkSelf("-qpCheck", "req=%" DEC "&svc=%s", reqId, svc);
                }
                outHtml();
                return 1;
            } else {
                if( !pForm->ivalue("check", 0) ) {
                    dataForm.printf("%" DEC ",%s", reqId, strObjList ? strObjList.ptr(0) : "0");
                    outHtml();
                    return 1;
                }
            }

        }

        case eQPRawCheck:
        case eQPCheck:{
badHackforDumbCode:
            if( cmdnum == eQPSubmit || cmdnum == eQPCheck ) {
                idx prg = 0, prg100 = 0, cSt = 0;
                idx grpCnt = 1;
                req2GrpSerial(reqId, reqId, &grpCnt, S.svcID);
                if( grpCnt < 1 ) {
                    grpCnt = 1;
                }
                idx igrpid = (req2Grp(reqId) == reqId && grpCnt > 1) ? true : false;
                if( igrpid ) {
                    cSt = parReqsStillIncomplete(reqId);
                    grpGetProgress(reqId, &prg, &prg100);
                } else {
                    cSt = (R.stat <= eQPReqStatus_Running) ? 1 : 0;
                    prg = R.progress;
                    prg100 = R.progress100;
                }

                idx timDiff=(time(0))-R.actTm;

                tmp.printf(0,"%s,%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC ",%" DEC,S.title, " " , reqId,cSt>0 ? eQPReqStatus_Running : R.stat,timDiff,prg,prg100/grpCnt, grpCnt, R.act);

                tmp.add0();

                if(raw) {
                    dataForm.printf("\n%s",tmp.ptr());
                }else {
                    idx prg=pForm->ivalue("progressonly",0);
                    if(prg){
                        sectionsToHide=eSectionHideTop|eSectionHideBottom;
                    }

                    idx qppos=pForm->ivalue("QPpos",1);
                    if(qppos==1){
                        outSection("qpRefresh");
                        executeJS("gInitList+=\"QPride_check(0,'%s');\";",tmp.ptr());
                    }
                    if(!prg)
                        outSection(buf.printf(0,"%s_check",svc ));
                    if(qppos==2){
                        outSection("qpRefresh");
                        executeJS("gInitList+=\"QPride_check(0,'%s');\";",tmp.ptr());
                    }
                }
            } else {
                sStr prgGrp, prgList;
                sStr * p_prgList = 0;

                if( pForm->boolvalue("showreqs",true) ) {
                    p_prgList = &prgList;
                }
                const char * svcIDList = pForm->value("svcID");
                sDic<idx> svcIDs;
                if(svcIDList) {
                    sStr svcIDs00;
                    sString::searchAndReplaceStrings(&svcIDs00, svcIDList, 0, ",",0,0,true );
                    const char * svcID = svcIDs00.ptr();
                    while (svcID){
                        svcIDs[svcID] = 1;
                        svcID = sString::next00(svcID);
                    }
                }

                idx result = reqProgressReport(&prgGrp, p_prgList, reqId,pForm->ivalue("start",0),pForm->ivalue("cnt",50), eQPIPStatus_Any , pForm->value("svcName"), svcIDs.dim()?&svcIDs:0 );
                if(raw) {
                    if(prgGrp)dataForm.printf("%s",prgGrp.ptr());
                    if(prgList)dataForm.printf("%s",prgList.ptr());
                }else {
                    outSection("qpRefresh");
                    executeJS("gInitList+=\"QPride_check(0,'%s%s');\";",prgGrp.ptr()? prgGrp.ptr() :"", prgList.ptr() ? prgList.ptr() : ""  );

                    outSection(buf.printf(0,"%s_check",svc ));

                }
                if(!result) {
                    const char * miss=pForm->value("default");
                    if(miss){
                        dataForm.printf("%s",miss);
                    }else {
                        dataForm.printf("unknown");
                    }
                }
            }
            if( pForm->boolvalue("down") ) {
                outBin( dataForm.ptr(),dataForm.length(),0,true,"progress-%" DEC ".csv",reqId);
            }
            else{
                outHtml();
            }
            return 1;
        }return 1;
        case eQPRQList:
            {
                idx status = 0;
                const char * filter=pForm->value("status"); 
                if( filter ) {
                    sStr buf;
                    sString::changeCase(&buf, filter, 0, sString::eCaseLo); 
                    sString::xscanf(buf.ptr(), "%n=0^any^waiting^processing^running^suspended^done^killed^progError^SysError^error;", &status);
                }
                sJSONPrinter printer(&dataForm);
                idx result = reqProgressReportReqOnly(printer, reqId, getLevelCode(pForm->value("showmsg", "0")), pForm->ivalue("start",0), pForm->ivalue("cnt",50), status, objs.dim() ? &objs[0] : nullptr);
                
                if(!result) {
                    const char * miss=pForm->value("default");
                    if(miss){
                        dataForm.printf("%s",miss);
                    }else {
                        dataForm.printf("unknown");
                    }
                }
            }
            if( pForm->boolvalue("down") ) {
                outBin( dataForm.ptr(),dataForm.length(),0,true,"progress-%" DEC ".JSON",reqId);
            }
            else{
                outHtml();
            }
            return 1;
        case eQPGRList:
            {
                sJSONPrinter printer(&dataForm);

                idx result = reqProgressReport2(printer, reqId, pForm->boolvalue("showreqs", false), getLevelCode(pForm->value("showmsg", "0")), &(objs[0]));
                 
                if(!result) {
                    const char * miss=pForm->value("default");
                    if(miss){
                        dataForm.printf("%s",miss);
                    }else {
                        dataForm.printf("unknown");
                    }
                }
            }
            if( pForm->boolvalue("down") ) {
                outBin( dataForm.ptr(),dataForm.length(),0,true,"progress-%" DEC ".JSON",reqId);
            }
            else{
                outHtml();
            }
            return 1;

        case eQPDataNames:{
            if(!raw)
                raw=1;
            sStr dnames00;dataGetAll(reqId, 0, &dnames00);
            for( const char * p=dnames00.ptr(); p; p=sString::next00(p)) tmp.printf("%s%s",tmp.length() ? "," :"",p);
            tmp.add0();
            dataForm.printf("%s",tmp.ptr());
            outHtml();
        } return 1;

        case eQPKillReq:{
            sVec < idx > reqIds;
            if(pForm->ivalue("isGrp"))grp2Req(reqId, &reqIds);
            if(reqIds.dim()<1)reqIds.vadd(1,reqId);

            reqSetAction(&reqIds,eQPReqAction_Kill);
            reqSetStatus(&reqIds,eQPReqStatus_Suspended);

            if(!raw) {
                linkSelf("-qpCheck","req=%" DEC "&svc=%s",reqId,svc);
                outHtml();
                return 1;
            }
        } return 1;
        case eQPReqSetAction: {
            sVec<idx> reqIds;
            const bool isGrp = pForm->boolvalue("isGrp");
            sStr svc_name;
            if( pForm->value("svcName") ) {
                svc_name.printf("%s", pForm->value("svcName"));
            }
            if( !svc_name.length() && pForm->ivalue("svcID") ) {
                Service l_svc;
                serviceGet(&l_svc, 0, pForm->ivalue("svcID"));
                svc_name.printf("%s", l_svc.name);
            }
            if( isGrp ) {
                grp2Req(reqId, &reqIds, svc_name);
            }
            if( reqIds.dim() < 1 ) {
                reqIds.vadd(1, reqId);
            }
            sStr str_act("%s", pForm->value("act"));
            idx act = eQPReqAction_None;

            if( atoi(str_act.ptr()) ) {
                act = pForm->ivalue("act");
            } else {
                if( strcasecmp(str_act.ptr(), "Kill") == 0 ) {
                    act = eQPReqAction_Kill;
                } else if( strcasecmp(str_act.ptr(), "Suspend") == 0 ) {
                    act = eQPReqAction_Suspend;
                } else if( strcasecmp(str_act.ptr(), "Resume") == 0 ) {
                    act = eQPReqAction_Resume;
                } else if( strcasecmp(str_act.ptr(), "Run") == 0 ) {
                    act = eQPReqAction_Run;
                } 
            }

            if( act == eQPReqAction_Suspend ) {
                reqSetStatus(&reqIds, eQPReqStatus_Suspended);
            } else if( act == eQPReqAction_Kill ) {
                reqSetStatus(&reqIds, eQPReqStatus_Killed);
            } else if( act == eQPReqAction_Resume ) {
                reqSetStatus(&reqIds, eQPReqStatus_Waiting);
                act = eQPReqAction_Run;
            } else if( act == -eQPReqAction_Run || act == eQPReqAction_Run ) {
                for(idx ir = 0; ir < reqIds.dim(); ++ir) {
                    reqReSubmit(reqIds[ir]);
                }
            }
            if( act < 0 ) {
                act = -act;
            }
            reqSetAction(&reqIds, act);
            if( raw ) {
                cmdnum = eQPCheck;
                goto badHackforDumbCode;
            } else {
                linkSelf("-qpCheck", "req=%" DEC "&svc=%s", reqId, svc);
                outHtml();
            }
        }
        return 1;

        case eQPData:{if(!raw)raw=1;
            const char * dname=pForm->value("dname");
            const char * dsaveas=pForm->value("dsaveas", dname);
            idx dsize=pForm->ivalue("dsize");
            const char * miss=pForm->value("default");
            if(dname){
                sStr dat;
                if(pForm->is("grp"))grpGetData(reqId,dname,dat.mex());
                else reqGetData(reqId,dname,dat.mex());
                if( dat.length() ) {
                    if( dsize && dsize < dat.length() - 15 ) {
                        dat.cut(dsize);
                        dat.printf("\n... more ...\n");
                    }
                    if( dsaveas[0] == '-' )
                        outBin(dat.ptr(), dat.length(), 0, true, "%s", dsaveas + 1);
                    else
                        outBin(dat.ptr(), dat.length(), 0, true, "%" DEC "-%s", reqId, dsaveas);
                } else if( miss ) {
                    outBin(miss, sLen(miss), 0, true, "%" DEC "-%s", reqId, dsaveas);
                } else {
                    outHtml();
                }
            }else outHtml();
        }return 1;

        case eQPReqInfo: {
            if( !raw ) {
                raw = 1;
            }
            eQPInfoLevel level = (eQPInfoLevel)pForm->ivalue("level");
            sVec<idx> reqIds;
            idx svcID = pForm->ivalue("svcID",0);
            sQPrideBase::Service svc;
            char * svcName = 0;
            if(svcID){
                serviceGet( &svc,0,svcID);
                svcName = svc.name;
            }
            sVec <Request> rList(sMex::fSetZero);
            requestGetForGrp(reqId, &rList, svcName);

            if(!rList.dim()) {
                requestGet(reqId,rList.add());
            }
            if(rList.dim()==1) {
                rList[0].grpID=rList[0].reqID;
            }
            sVec<QPLogMessage> infos;
            if( rList.dim() > 1 ) {
                grpGetInfo(rList[0].grpID, level, infos);
            } else if ( rList.dim() == 1 ) {
                reqGetInfo(rList[0].reqID, level, infos);
            }

            sDic < idx > svcs;
            idx prevReq = -1;
            dataForm.printf("name,parent,reqID,svcName,date,infoTxt,infoLvl,infoLvlNum\n");
            sStr res, tmp1, tmp2;
            for(idx ig = 0; ig < rList.dim(); ++ig) {
                Request * c_req = rList.ptr(ig);
                if(c_req->reqID==prevReq) {
                    continue;
                }
                tmp1.cut0cut(0);
                sString::escapeForCSV(tmp1, c_req->svcName);
                prevReq=c_req->reqID;
                idx top_level = eQPInfoLevel_Min;
                sStr top_level_txt_escaped;
                idx ii = 0;
                for(idx i = 0; i < infos.dim(); ++i) {
                    if(c_req->reqID != infos[i].req ) {
                        continue;
                    }
                    time_t ttm = (time_t) infos[i].cdate;
                    sStr ttt;
                    sString::copyUntil(&ttt, asctime(localtime(&ttm)), 0, sString_symbolsEndline);
                    if( infos[i].level > (idx)eQPInfoLevel_Min){
                        tmp2.cut0cut(0);
                        sString::escapeForCSV(tmp2, infos[i].message());
                        res.printf("%" DEC ",%" DEC ",%" DEC ",%s,%s,%s,%s,%" DEC "\n", ++ii, c_req->reqID, c_req->reqID, tmp1.ptr(), ttt.ptr(), tmp2.ptr(), getLevelName((eQPInfoLevel)infos[i].level), infos[i].level);
                        if( (idx)top_level < infos[i].level ) {
                            top_level = (eQPInfoLevel)infos[i].level;
                            top_level_txt_escaped.printf(0, "%s", tmp2.ptr());
                        }
                    }
                }
                idx * svc_level = &svcs[&c_req->svcName];
                if( *svc_level < top_level ){
                    *svc_level = top_level;
                }
                if( top_level > eQPInfoLevel_Min) {
                    res.printf("%" DEC ",%s,%" DEC ",%s,,%s,%s,%" DEC "\n", c_req->reqID, tmp1.ptr(), c_req->reqID, tmp1.ptr(), top_level_txt_escaped.ptr(),getLevelName((eQPInfoLevel)top_level),top_level);
                }
            }
            idx max_level = eQPInfoLevel_Min;
            for(idx id=0; id<svcs.dim(); ++id) {
                const char * svcName=(const char *)svcs.id(id);
                idx c_level = svcs[id];
                if( max_level < c_level ) {
                    max_level = c_level;
                }
                if (c_level > (idx)eQPInfoLevel_Min)
                    res.printf("\"%s\",\"Grouped Info\",0,,,,%s,%" DEC "\n", svcName, getLevelName((eQPInfoLevel)c_level), c_level);
            }
            if(res.length()){
                res.printf("\"Grouped Info\",\"root\",0,,,,%s,%" DEC "\n",  getLevelName((eQPInfoLevel)max_level), max_level);
                dataForm.printf("%s",res.ptr());
            }
            outHtml();
        }
        return 1;

        case eQPReqRegisterAlive: {
            if( !raw ) {
                raw = 1;
            }
            idx job=pForm->ivalue("job",(idx)getpid());
            jobRegisterAlive(job, reqId, 0);

            dataForm.printf("alive");

            outHtml();
        }
        return 1;

        case eQPFile:
            {
                if( !raw ) {
                    raw = 1;
                }
                sStr path, canon;
                cfgStr(&path, pForm, "qm.tempDirectory");
                path.printf("%s", pForm->value("file"));
                sString::searchAndReplaceStrings(&canon, path.ptr(), 0, "../" __, "" __, sIdxMax, true);
                sFil fl(canon, sMex::fReadonly);
                if( fl.length() ) {
                    outBin(fl.ptr(), fl.length(), sFile::size(path.ptr()), "%s", pForm->value("file"));
                } else {
                    outHtml();
                }
            }
            return 1;

        case eQPPerformRegisterTime:{if(!raw)raw=1;

            idx amount=pForm->ivalue("amount",0);
            idx regtm=pForm->ivalue("time",0);

            sStr svcNameStr;
            svcNameStr.printf("%s",pForm->value("svc",""));

            sStr workparams;
            workparams.printf("%s",pForm->value("workparams",""));

            idx ret = workRegisterTime(svcNameStr.ptr(), workparams.ptr(), amount, regtm);

            if( ret <= 0 ) {}
            dataForm.printf("%" DEC,ret);
            outHtml();

        } return 1;

        case eQPPerformEstimateTime:{if(!raw)raw=1;

            idx amount=pForm->ivalue("amount",0);

            sStr svcNameStr;
            svcNameStr.printf("%s",pForm->value("svc",""));
            sStr workparams;
            workparams.printf("%s",pForm->value("workparams",""));

            idx est = workEstimateTime(svcNameStr.ptr(), workparams.ptr(), amount);

            if( est < 0 ) {}
            dataForm.printf("%" DEC,est);
            outHtml();
        } return 1;



        default: return sUsrCGI::Cmd(cmd);
    }


    return 1;
}



idx sQPrideCGIProc::OnExecute(idx req)
{
    if( !procCGI_qapp )
        procCGI_qapp = new sQPrideCGI("config=qapp.cfg" __,svcName, sApp::argc, sApp::argv, sApp::envp, stdin, true, true);

    procCGI_qapp->dataForm.cut0cut();
    procCGI_qapp->htmlBody.cut0cut();
    procCGI_qapp->htmlDirs00.cut0cut();
    procCGI_qapp->execJS.cut0cut();
    procCGI_qapp->redirectURL.cut0cut();

    reqGetData(req, "formT.qpride", procCGI_qapp->pForm);

    sHtml::inputCGI( (FILE *)stdin, sApp::argc, sApp::argv, procCGI_qapp->pForm,procCGI_qapp->mangleNameChar,true,0);

    procCGI_qapp->cmd = procCGI_qapp->pForm->value("cmd");
    procCGI_qapp->reqId = req;
    sStr out;
    procCGI_qapp->outP = &out;
    procCGI_qapp->proc_obj = this;

    const char *  risky_cmds00 = "-qpSubmit" _ "-qpRawSubmit" _ "-qpProcSubmit" __;
    if (! procCGI_qapp->cmd || sString::compareChoice( procCGI_qapp->cmd, risky_cmds00, 0, true, 0, true) >= 0) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    const char * cgi_dstname = procCGI_qapp->pForm->value("cgi_dstname","cgi_output");
    sStr datasource("file://%s",cgi_dstname);
    sStr cgi_output_path;

    reqSetData(req, datasource, 0, 0);
    reqDataPath(req, datasource.ptr(7), &cgi_output_path);

    procCGI_qapp->setFlOut(fopen(cgi_output_path.ptr(), "w"));
    procCGI_qapp->raw = 2;
    procCGI_qapp->run();

    fclose(procCGI_qapp->flOut);
    procCGI_qapp->setFlOut(0);


{
        procCGI_qapp->reqRepackData(req, cgi_dstname);
    }



    reqSetProgress(1, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}


idx sQPrideCGIProc::run()
{
    sCmdLine cmd;
    if( sApp::argc > 1 ) {
        cmd.init(sApp::argc, sApp::argv);
    }

    if( sString::parseBool( cmd.next("-daemon") ) ){
        sStr tmp;

        sQPrideProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, svcName, sApp::argv[0]));
        return (int) backend.run(sApp::argc, sApp::argv);
    }
    return sQPrideCGI::run();
}


