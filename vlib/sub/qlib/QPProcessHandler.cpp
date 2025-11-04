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

#include <qlib/QPrideProc.hpp>
#include <slib/std/file.hpp>
#include <slib/utils/tbl.hpp>
#include <ulib/upropset.hpp>

#include <violin/violin.hpp>
#include <qlib/QPProcessHandler.hpp>

using namespace slib;

sQPProcessHandler * sQPProcessHandler::initHandler(const char * lworkflowName, sQPrideBase * lqp, sUsrObj * lobjs, const char * reportObj, idx lreq, const char * lmappath, idx ldelayResubmit)
{
    qp=lqp? lqp : reinterpret_cast < sQPrideProc * > (this) ;
    error=0;
    req=lreq ? lreq : (qp ? qp->reqId : 0);
    qpObjs=lobjs;
    delayResubmit=ldelayResubmit;
    reportO=0;
    reportObjId=0;
    if(reportObj) {
        if( reportObj && memcmp(reportObj, "prop:" ,5)==0 )
            reportObj=qpObjs->propGet(reportObj+5);
        reportObjId=atoidx (reportObj);
    }else {
        reportObjId=qpObjs ? qpObjs->Id().objId() :  0;
    }
    if(reportObjId)reportO=new sUsrObj(*(qp->user),sHiveId(reportObjId,0));

    sumStat=sQPrideBase::eQPReqStatus_Any;
    sumProgress=0;
    sumProgress100=0;
    sumCnt=0;
    defaultFolder=0;

    procFolder.cut(0);
    if(qpObjs)qpObjs->getFilePathname(procFolder, "%s", "");

    mappath.cut(0);
    if(lmappath)mappath.printf("%s",lmappath);
    else if(qpObjs)qpObjs->makeFilePathname(mappath,"procmap.csv");
    if(!mappath.length())return this;

    workflow=0;
    if(lworkflowName){
        workflowName.printf(0,"%s",lworkflowName);
        loadWorkflow(workflowName.ptr());
    }

    if( ! deserialize(mappath.ptr()) ) return 0;

    return this;
}

bool sQPProcessHandler::err(bool condition, const char * fmt, ... )
{
    if(condition)return false;
    sStr err;
    sCallVarg(err.vprintf,fmt);
    if(reportO) reportO->propSet("all_status","progError");
    qp->reqSetInfo(qp->reqId,sQPrideBase::eQPInfoLevel_Error, err.ptr());
    qp->reqSetStatus(qp->reqId, sQPrideBase::eQPReqStatus_ProgError );

    return 1;
}

const char * sQPProcessHandler::objQry(sStr & buf, const char * fmt, ... )
{
    sStr t;
    sCallVarg(t.vprintf,fmt);
    qlang::sUsrEngine engine(*(qp->user), 0);
    sStr error; 
    sVariant results;
    engine.parse(t.ptr(), 0, &error) && engine.eval(results, &error);
    buf.addString(results.asString());

    return buf.ptr();
}


idx sQPProcessHandler::cannotContinue(const char * role, const char * fmt, ... )
{
    if(error) {
        sStr out("%s: ",role);
        if(fmt) sCallVarg(out.vprintf,fmt);
        qp->reqSetInfo(req,sQPrideBase::eQPInfoLevel_Error,fmt ? out.ptr() : log.ptr() );
        qp->reqSetStatus(req, sQPrideBase::eQPReqStatus_ProgError );
        return 1;
    }

    idx prg, prg100;
    sStr errs;
    sQPride::eQPReqStatus stat=statusProcess(role,&prg,&prg100,sQPrideBase::eQPInfoLevel_Error,&errs);

    if(reportObjId)setStageStatus(role,stat, prg, prg100, errs.length() ? errs.ptr() : 0 );
    qp->logOut(sQPrideBase::eQPLogType_Info,"Status '%s'=%s ==> %" DEC "%%\n", role,sString::next00("any" _ "waiting" _ "processing" _ "running" _ "suspended" _ "done" _ "killed" _ "progError" _ "sysError" __, stat), prg100);


    if(stat==sQPrideBase::eQPReqStatus_Done) {
        return 0;
    }

    if(stat<sQPrideBase::eQPReqStatus_Done) {
        qp->reqReSubmit(req,delayResubmit);
        return 1;
    }

    else if(stat==sQPrideBase::eQPReqStatus_Killed) {
        qp->reqSetInfo(req,sQPrideBase::eQPInfoLevel_Error,"Process '%s' was stopped", role );
        qp->reqSetStatus(req, sQPrideBase::eQPReqStatus_Killed);
        return 1;
    }
    else if(stat==sQPrideBase::eQPReqStatus_ProgError) {
        qp->reqSetInfo(req,sQPrideBase::eQPInfoLevel_Error,"Data error detected by '%s'", role );
        qp->reqSetStatus(req, sQPrideBase::eQPReqStatus_ProgError );
        return 1;
    }
    else if(stat==sQPrideBase::eQPReqStatus_ProgError) {
        qp->reqSetInfo(req,sQPrideBase::eQPInfoLevel_Error,"System error in '%s'", role );
        qp->reqSetStatus(req, sQPrideBase::eQPReqStatus_SysError);
        return 1;
    }

    return 1;
}

idx sQPProcessHandler::loadWorkflow(const char * workflowName, const char * namevar, const char * type, const char * filename)
{

    sUsrObj jsonConf;
    sStr jsonPath;
    qp->user->findUniqueObject(&jsonConf,"workflow","name",workflowName);
    jsonConf.getFilePathname(jsonPath,filename);

    if(workflow) delete workflow;
    workflow = new sJson();
    workflow->initFile(jsonPath.ptr());

    return 1;

}

idx sQPProcessHandler::serialize(const char * path, bool clean)
{
    if(!objsets.dim())return 0;

    if(!path)path=mappath.ptr();
    sFil fl(path);
    fl.printf(0,"name,processID\n");
    idx cnt=0;
    for ( idx i=0; i<objsets.dim() ; ++i) {
        idx idlen;const char * id=(const char *) objsets.id(i,&idlen);
        for ( idx ip=0; ip<objsets[i].dim() ; ++ip) {
            fl.printf("%.*s,%" DEC "\n",(int)idlen,id,*(objsets[i].ptr(ip)));
            ++cnt;
        }
    }
    if(clean)objsets.empty();

    if(!varset.dim())
        return cnt;
    sFilePath vardic; vardic.makeName(path,"%%pathx.vars");
    sFil vf(vardic);
    vf.printf(0,"name,value\n");
    for(idx id=0; id<varset.dim(); ++id){
        vf.printf("%s,\"%s\"\n",(const char * )varset.id(id),varset.validx(id));
    }
    vf.printf("-,-\n");
    if(clean)varset.empty();

    return cnt;
}

idx sQPProcessHandler::deserialize(const char * path)
{
    if(!path)path=mappath.ptr();
    sTbl tbl;tbl.parseFile(path);
    sStrT obj;
    for( idx it=1; it<tbl.rows(); ++it) {
        idx idlen;const char * id=tbl.cell(it,(idx)0,&idlen);
        idx procId=tbl.ival(it,1);
        if(id[idlen-1]=='/')defaultFolder=procId;

        if(procId<0) {
            obj.cut(0);
            qp->requestGetPar(-procId,1,&obj);
            if(obj.length()) {
                idx oi=atoidx(obj.ptr());
                if(oi)
                    procId=oi;
            }
            else return 0;
        }
        *objsets.set(id,idlen)->add(1)=procId;
    }

    sFilePath vardic; vardic.makeName(path,"%%pathx.vars");
    sFil vf(vardic);
    sTbl vbl;vbl.parseFile(vardic);
    for( idx it=1; it<vbl.rows()-1; ++it) {
        sStr var;vbl.get(&var,it,(idx)0);
        sStr val;vbl.get(&val,it,1);
        varset.inp(var,val);
    }

    return tbl.rows()-1;
}

idx sQPProcessHandler::setObjset(const char * role, const char * ids)
{
    sVec < idx> idlist; sString::scanRangeSet(ids,0,&idlist,0,0,0,false);
    for( idx i=0 ; i<idlist.dim(); ++i) {
        *objsets.set(role)->add(1)=idlist[i];
    }
    return idlist.dim();
}

idx sQPProcessHandler::cntObjset(const char * setname)
{
    sVec < idx > * objs=objsets.get(setname);if(!objs)return 0;
    return objs->dim();

}
const char * sQPProcessHandler::listObjRegex(const char * setname, sStr * buf, sVec < sHiveId> * hid, bool clnBuf)
{
    static sStr Buf;if(!buf)buf=&Buf;
    if(clnBuf)buf->cut(0);
    regex_t regp;
    regcomp(&regp,setname, REG_EXTENDED | REG_ICASE) ;
                
    sStr t;
    idx l;
    for( idx io=0;io<objsets.dim(); ++io) {
        const char * id=(const char * )objsets.id(io,&l);
        t.printf(0,"%.*s",(int)l,id);
        if(regexec(&regp, t.ptr(0), 0, NULL, 0))
            continue;
        listObjset(t.ptr(0),buf,hid,false);
    }
    return buf ? buf->ptr() : 0;
}

const char * sQPProcessHandler::listObjset(const char * setname, sStr * buf, sVec < sHiveId> * hid, bool clnBuf)
{
    static sStr Buf;if(!buf)buf=&Buf;
    if(clnBuf)buf->cut(0);
    sVec < idx > * objs=objsets.get(setname);if(!objs)return 0;
    for( idx ii=0; ii<objs->dim(); ++ii){
        if(buf->length())buf->add(",",1);
        buf->printf("%" DEC , (*objs)[ii]);
        if(hid)
            new (hid->add(1)) sHiveId((*objs)[ii],0);
    }
    return buf->length() ? buf->ptr() : 0;
}


idx sQPProcessHandler::launchProcess(const char * role, const char * svc,const char * jsonFmt, ... )
{

    sStr json;sCallVarg(json.vprintf,jsonFmt);
    sStr strObjList;

    sVar form;form.inp("svc",svc);form.inp("_json",json);
    if(defaultFolder)form.inpv("folder","%" DEC, defaultFolder);

    sQPride::Service S;qp->serviceGet(&S, svc, 0);

    sVec < sUsrProc > procSet;
    error = sUsrProc::createProcesForsubmission(qp, &form, qp->user, procSet, &S, &strObjList, &log);
    idx cntParallel = 1;
    if(error)return 0;

    idx locreq;
    error = sUsrProc::standardizedSubmission(qp, &form, qp->user, procSet, cntParallel, &locreq, &S, 0, &strObjList, &log);
    if(error) return 0;

    sVec < idx > & procObjs=*(objsets.set(role,0)); 
    for(idx ip = 0; ip < procSet.dim(); ++ip) {
        procSet[ip].propSync();
        procObjs.vadd(1,procSet[ip].Id().objId());

    }

    return 1;
}


idx sQPProcessHandler::ensureProcess(const char * role, const char * svc, sUsrObj * obj, sVar * form)
{
    if(!hasObjects(role)) {
        qp->logOut(sQPrideBase::eQPLogType_Info,"Launching '%s'\n", role);
        launchProcess(role, svc, obj, form);
    } else{
        qp->logOut(sQPrideBase::eQPLogType_Info,"Process '%s' exists\n", role);
    }
    return !cannotContinue(role,0);
}

idx sQPProcessHandler::ensureProcess(const char * role, const char * svc, const char * jsonFmt, ... )
{
    if(!hasObjects(role)) {
        qp->logOut(sQPrideBase::eQPLogType_Info,"Launching '%s'\n", role);
        sStr t;sCallVarg(t.vprintf,jsonFmt);
        launchProcess(role, svc,"%s",t.ptr());
    } else{
        qp->logOut(sQPrideBase::eQPLogType_Info,"Process '%s' exists\n", role);
    }
    return !cannotContinue(role,0);
}


idx sQPProcessHandler::launchProcess(const char * role, const char * svc, sUsrObj * obj, sVar * form)
{
    sStr tpl("[");
    sStr tmpRole;
    sString::copyUntil(&tmpRole,role,0,".");
    loadWorkflow(workflowName.ptr());
    JSNode node=workflow->find("name",tmpRole);
    if(!node.ok())return 0;

    sStrT Svc;
    if(!svc){
        idx len;
        svc=node["..svc"].val(&len);
        if(len && svc)
            Svc.printf("%.*s",(int)len,svc);
            svc=Svc;
    }
    if(!obj)obj=qpObjs;

    JSNode exec=node["..execute.0._id"];

    exec.print(&tpl);
    tpl.printf("]");

    sVar vars;
    if(!form)form=&vars;
    sStr objList;
    for ( idx io=0; io<objsets.dim(); ++io) {
        idx idlen;const char * id=(const char * )objsets.id(io,&idlen);
        sVec < idx > * objs=objsets.ptr(io);
        objList.cut(0);
        for( idx ii=0; ii<objs->dim(); ++ii){
            const char * cur=objList.printf(",%" DEC , (*objs)[ii])+1;
            form->inpf(cur,0,"%.*s[%" DEC "]",(int)idlen,id,ii);
        }
        form->inp(id,objList.ptr(1),objList.length()-1,idlen);
    }

    for ( idx io=0; io<varset.dim(); ++io) {
        idx idlen;const char * id=(const char * )varset.id(io,&idlen);
        idx sizeval;const char * vval=varset.value(id,0,&sizeval);
        form->inp(id,vval,sizeval,idlen);
    }

    sStr wfl;
    if(!qp->user->replaceVarsFromObjForm(&wfl, tpl.ptr(), obj, form, &log, true )) {
        error=1;
        return 0;
    }
    return launchProcess(role, svc,"%s", wfl.ptr(0));
}

idx sQPProcessHandler::hasObjects(const char * roles, idx * ptot)
{
    idx cnt=0;
    if(ptot)*ptot=0;
    sStr rolelist; sString::searchAndReplaceSymbols(&rolelist,roles,0,",; ",0,0,true,true,true,true,0);
    for ( const char * rl=rolelist.ptr(); rl; rl=sString::next00(rl)) {
        sVec < idx > * procObjs=objsets.get(rl,0);
        if(!procObjs)continue;
        ++cnt;
        if(ptot)
            *ptot+=objsets.dim();
    }

    return cnt;
}

idx sQPProcessHandler::procId(const char * role, idx num)
{
    sVec < idx > * procObjs=objsets.get(role,0);
    if(!procObjs || !procObjs->dim())return 0;

    return (idx)*(procObjs->ptr(0));

}


sQPride::eQPReqStatus sQPProcessHandler::statusProcess(const char * roles, idx * prg, idx * prg100, sQPrideBase::eQPInfoLevel level, sStr * logs)
{
    sVec < idx > stat, reqIds;
    sQPride::eQPReqStatus leastStatus=sQPride::eQPReqStatus_Max;
    sQPride::eQPReqStatus mostStatus=sQPride::eQPReqStatus_Any;

    sStr rolelist; sString::searchAndReplaceSymbols(&rolelist,roles,0,",; ",0,0,true,true,true,true,0);
    idx cnt=0, lprg=0, lprg100=0;
    if(prg)*prg=0;if(prg100)*prg100=0;
    for ( const char * rl=rolelist.ptr(); rl; rl=sString::next00(rl)) {
        sVec < idx > * procObjs=objsets.get(rl,0);

        if(!procObjs)continue;
        for ( idx i=0; i<procObjs->dim(); ++i ) {
            idx req=*procObjs->ptr(i);
            if(req<0)req=-req;
            else {
                sUsrProc pO(*(qp->user),sHiveId(*procObjs->ptr(i),0));
                req=pO.propGetI("reqID");
            }
            stat.cut(0);
            sVec<sQPrideBase::QPLogMessage> infos;

            sQPrideBase::Request R; sSet(&R);
            idx ret_reqGet = qp->requestGet(req, &R);
            if (!ret_reqGet) {
                sUsrProc pO(*(qp->user),sHiveId(*procObjs->ptr(i),0));
                R.stat=pO.propGetI("status");
                R.progress=pO.propGetI("progress");
                R.progress100=pO.propGetI("progress100");
            }
            else {
                cnt+=qp->grp2Req(req, &reqIds);reqIds.cut(0);
                if(cnt>1) {
                    qp->grpGetStatus( req, &stat);
                    qp->
                    grpGetProgress(req,&lprg,&lprg100);
                    if(logs){
                        qp->grpGetInfo(req, level, infos);
                    }
                }
            }
            if (R.stat){
                stat.vadd(1,R.stat);
                lprg=R.progress;
                lprg100=R.progress100;
                if(logs){
                    qp->reqGetInfo(req, level, infos);
                }
            }
            
            if(logs) {
                for ( idx ii=0; ii<infos.dim() ; ++ii) {
                    if( infos[ii].level >=level ) {
                        logs->printf("%s\n",infos[ii].message());
                    }
                }
            }

            if(prg)*prg+=lprg;
            if(prg100)*prg100+=lprg100;

            for ( idx is=0; is<stat.dim() ; ++is) {
                if(stat[is]>mostStatus)mostStatus=(sQPride::eQPReqStatus)stat[is];
                if(stat[is]<leastStatus)leastStatus=(sQPride::eQPReqStatus)stat[is];
            }
        }
    }

    if(prg100) { *prg100 = cnt ? *prg100/cnt : 0 ; }

    if(mostStatus>sQPrideBase::eQPReqStatus_Done)return  mostStatus;
    return leastStatus;
}

idx sQPProcessHandler::initFormObject(const char * setname, const char * id )
{
    return initObject(setname, qpObjs->propGet(id) );
}

idx sQPProcessHandler::initObject(const char * setname, const char * id )
{
    sUsrObj sampleObj(*qp->user,sHiveId(id));
    return sampleObj.Id().objId();
}

idx sQPProcessHandler::searchObjects(const char * setname, const char * type, const char * pars, const char * vals, ... )
{
    sUsrObjRes obj_res;
    idx cnt=0;
    sStr Vals;
    if(vals) {
        sCallVarg(Vals.vprintf,vals);
        vals=Vals.ptr();
    }

    if( !qp->user->objs2(type, obj_res,(udx*)0,pars,vals) )
        return 0;

    if(!obj_res.dim())return 0;

    sUsrObjRes::IdIter it = obj_res.first();
    if(setname) {
        sVec < idx > * objs=objsets.set(setname,0);
        objs->empty();
        for(; obj_res.has(it); obj_res.next(it)) {
            objs->vadd(1,obj_res.id(it)->objId());
            ++cnt;
        }
    } else {
        return obj_res.id(it)->objId();
    }


    return cnt;
}


idx sQPProcessHandler::searchArchived(const char * setname, const char * roles, const char * type)
{
    sStr buf;
    sStr rolelist; sString::searchAndReplaceSymbols(&rolelist,roles,0,",; ",0,0,true,true,true,true,0);
    idx cnt=0;

    for ( const char * rl=rolelist.ptr(); rl; rl=sString::next00(rl)) {

        sVec < idx > * procObjs=objsets.get(rl,0);
        if(!procObjs)continue;
        for ( idx i=0; i<procObjs->dim(); ++i ) {

            idx objId=*(procObjs->ptr(i));
            buf.printf(0,"archiver/%" DEC, objId);

            sUsrObjRes obj_res;
            if( !qp->user->objs2(type, obj_res,(udx*)0,"base_tag",buf.ptr(0)) )
                continue;
            if(!obj_res.dim())continue;

            sUsrObjRes::IdIter it = obj_res.first();
            if(setname) {
                sVec < idx > * objs=objsets.set(setname,0);
                objs->empty();
                for(; obj_res.has(it); obj_res.next(it)) {
                    objs->vadd(1,obj_res.id(it)->objId());
                    ++cnt;

                }
            } else {
                return obj_res.id(it)->objId();
            }

            procObjs=objsets.get(rl,0) ;
        }
    }
    return cnt;

}


idx sQPProcessHandler::archiveCGI(const char * role, const char * source_setname, const char * fileName , const char * addFmt, ... )
{

    if(!hasObjects(role)) {
        qp->logOut(sQPrideBase::eQPLogType_Info,"Launching '%s'\n", role);

        sVar form;

        form.inp("objs",listObjset(source_setname));
        form.inp("down","1");
        form.inp("screen","0");
        form.inp("compress","0");
        form.inp("backend","1");
        form.inp("numID","0");
        form.inp("check","1");
        form.inp("arch","1");
        form.inp("raw","1");
        const char * ext=strchr(fileName,'.');if(!ext)ext="";else ++ext;
        form.inp("ext",ext);
        if(defaultFolder)form.inpv("folder","%" DEC, defaultFolder);

        form.inpv("arc_dstname","%s",fileName );
        form.inpv("cgi_dstname","%s",fileName );

        if(addFmt) {
            sStr buf;
            sCallVarg(buf.vprintf,addFmt);buf.add0(2);
            sString::searchAndReplaceSymbols(buf.ptr(0),0,"&=",0,0,true,false,false,false,0);
            for ( const char *v=buf.ptr(), * p=buf.ptr(); p && v; p=sString::next00(v) ){
                v = sString::next00(p);if(!v)break;
                form.inp(p,v);
            }
        }

        sQPride::Service S;qp->serviceGet(&S, "dnaCGI", 0);

        sVec < sUsrProc> procObjs;
        idx reqId;error = sUsrProc::standardizedSubmission(qp, &form, qp->user, procObjs, 1, &reqId, &S, 0,0, &log);

        objsets.set(role,0)->vadd(1,-reqId);
    }
    else {
        qp->logOut(sQPrideBase::eQPLogType_Info,"Archival process '%s' exists\n", role);
    }
    return !cannotContinue(role,0);

}


idx sQPProcessHandler::ensureDownload(const char * role, const char * name, const char * source, const char * ids, ...  )
{
    if(!hasObjects(role)) {
        qp->logOut(sQPrideBase::eQPLogType_Info,"Launching '%s'\n", role);

        sStr cln,idlist;
        sCallVarg(idlist.vprintf,ids);
        sString::searchAndReplaceSymbols(&cln,idlist.ptr(),0,"\n\t\t",",",0,true,true,true,true,0);

        sVar form;

        form.inp("name",name);
        form.inp("uri",cln.ptr(0));
        form.inp("screen","0");
        form.inp("concurrency","1000000");
        form.inp("chkauto_Downloader","-1");
        form.inp("idx_Downloader","1");
        form.inp("screen_Downloader","1");
        form.inp("qc_Downloader","1");
        if(defaultFolder)form.inpv("folder","%" DEC, defaultFolder);


        sStr json("[{ \
            \"_id\": \"$newid\", \
            \"_type\": \"svc-download\", \
            \"name\": \"%s\", \
            \"datasource\": \"%s\", \
            \"uri\": [ \"%s\" ], \
            \"folder\": %" DEC ", \
            \"download_concurrency\": 1000000, \
            \"processing_Downloader\": { \
                \"chkauto_Downloader\": -1, \
                \"idx_Downloader\": true, \
                \"screen_Downloader\": true, \
                \"qc_Downloader\": true \
            } \
        }]"
        ,name,source,cln.ptr(), defaultFolder);
        form.inp("_json",json.ptr());

        sQPride::Service S;qp->serviceGet(&S, "dmDownloader", 0);

        idx reqId;
        sVec < sUsrProc> procObjs;
        sStr objList, log;
        bool withObjs=true;
        if( withObjs ){
            error = sUsrProc::createProcesForsubmission(qp, &form, qp->user, procObjs, &S, &objList, &log);
        } if(!error) {
            error = sUsrProc::standardizedSubmission(qp, &form, qp->user, procObjs, 1, &reqId, &S, 0,&objList, &log);
        }

        objsets.set(role,0)->vadd(1,-reqId);

    }
    else {
        qp->logOut(sQPrideBase::eQPLogType_Info,"Downloader process '%s' exists\n", role);
    }
    return !cannotContinue(role,0);

}


idx sQPProcessHandler::setStageStatus(const char * role,sQPrideBase::eQPReqStatus status, idx progress, idx progress100, const char * errs)
{
    if( status > sQPrideBase::eQPReqStatus_Done )sumStat=sMax(status,sumStat);
    else sumStat=sMin(sumStat,status);
    sumProgress+=progress;
    sumProgress100+=progress100;
    sumCnt++;


    sJson oj;qp->user->objJson(reportObjId,&oj);
    JSNode root(&oj,"$root");root.del("batch");

    JSNode * pNode=oj.pathAuto("$root.[]detail_progress.{}#.detail_stage.=%s",role);if(!pNode)return 0;
    JSNode node=*pNode;
    node.link("detail_status",sQPrideBase::reqStatText(status));

    node.link("detail_progressNum",progress);
    node.link("detail_progress100",progress100);
    node.link("detail_ids",listObjset(role));
    if(epoch.length())node.link("detail_epoch",epoch.ptr());
    if(errs)node.link("detail_err",errs);



    bool ret=qp->user->propSetJson(&oj,0,&log);
    return ret ? 1 : 0 ;

}


idx sQPProcessHandler::setFinalStatus(void)
{
    reportO->propSet("all_status",sQPrideBase::reqStatText(sumStat));
    reportO->propSetI("all_progress",sumProgress);
    reportO->propSetI("all_progress100",sumCnt ? sumProgress100/sumCnt : 0 );
    return 1;

}


idx sQPProcessHandler::setFolder(const char * role, const char * name, const char * parent)
{
    if(!name)return 0;
    if(!hasObjects(role)) {
        sHiveId id(parent);
        if(!id) {
            sUsrFolder * _inbox = sSysFolder::Inbox(*(qp->user));
            id.parse(_inbox->IdStr());
        }
        sUsrFolder src_obj(*(qp->user),id);
        if( !src_obj.Id() ) {
            return 0;
        }
        sUsrFolder * fld=src_obj.createSubFolder(name);
        setObjset(role,fld->IdStr());
    }
    defaultFolder=procId(role);
    return 1;
}

idx sQPProcessHandler::moveObject(const char * setname, idx proc, idx folder, bool copy)
{
    if(!folder) folder=defaultFolder;
    sUsrFolder dst(*qp->user,sHiveId(folder,0));if(!dst.Id())return 0;
    sVec < sHiveId > lst;
    if(proc)new (lst.add(1)) sHiveId(proc>0 ? proc : qpObjs->Id().objId() ,0);
    listObjset(setname,0,&lst);
    for( idx i=0;i<lst.dim() ; ++i) {
        sUsrObj o(*qp->user,lst[i]); if(!o.Id())continue;
        dst.attach(o);
    }
    return lst.dim();
}

char * sQPProcessHandler::getFilePath(const char * role, const char * fileName, idx iNum, sStr * filename)
{
    sVec < idx > * objs=objsets.get(role);if(!objs)return 0;
    if(iNum>=objs->dim())return 0;

    static sStrT Flnm;
    if(!filename)filename=&Flnm;
    filename->cut(0);

    sUsrObj o(*(qp->user),sHiveId((*objs)[iNum],0));
    o.getFilePathname(*filename,"%s",fileName);
    return filename->length() ? filename->ptr(): 0;
}


sFil * sQPProcessHandler::getFile(const char * role, const char * fileName, idx iNum, idx mode, sFil * fl)
{
    static sFil Fl;
    if(!fl)fl=&Fl;
    fl->destroy();

    const char * path=getFilePath(role, fileName, iNum, 0);
    if(!path)return 0;

    fl->init(path,mode);
    return (fl->ok()) ? fl : 0;
}


idx sQPProcessHandler::copyFiles(const char * srcset, const char * filecard, const char * dstset, const char* dstflnm, bool doAppend, const char * prefix)
{
    sVec < idx > * objs=objsets.get(srcset);if(!objs)return 0;

    sUsrObj odst(*qp->user,sHiveId(dstset ? procId(dstset) : qpObjs->Id().objId() , 0)    );
    sStr path; odst.getFilePathname(path);idx len=path.length();
    sStr psrc;
    idx cnt=0;
    for( idx ii=0; ii<objs->dim(); ++ii){
        sUsrObj o(*qp->user,sHiveId(*objs[ii],0)); if(!o.Id()) continue;
        sDir flist00;
        o.files(flist00, sFlag(sDir::bitFiles),filecard ? filecard : "*" );
        psrc.cut(0);o.getFilePathname(psrc);idx lsrc=psrc.length();
        for( const char * ff=flist00.ptr(); ff; ff=sString::next00(ff)) {
            path.printf(len,"%s%s%s",prefix ? prefix : "" , prefix ? "-" : "" , dstflnm ? dstflnm : sFilePath::nextToSlash(ff));
            psrc.printf(lsrc,"%s",sFilePath::nextToSlash(ff));
            sFile::copy(psrc,path,doAppend);
        }
        ++cnt;
    }
    return cnt;
}


