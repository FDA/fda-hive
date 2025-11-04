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

#include <glauncher.hpp>
#include <slib/std/file.hpp>
#include <ion/sJson.hpp>

#include <time.h>


class WfloProc: public sQPrideProc
{
    public:
        WfloProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {

        }
        ~WfloProc()
        {
        }
        virtual idx OnExecute(idx);
        sRC OnSplit(idx req, idx & cnt){cnt=1;return sRC::zero;}
};


idx WfloProc::OnExecute(idx req)
{
    sMex dataBlob;
    idx delayLaunch=formIValue("delayLaunch",10);
    idx progressAll=100,itemProgress=0;


    const char * spid=reqGetData(req,"pid", &dataBlob, true, 0 );idx pid = spid ? atoidx(spid) : 0, ip;
    if(pid) {
        sVec < sPS::Stat> plist;
        sPS ps;ps.getProcList(&plist, 0 , true, 0);
        for(ip=0; ip<plist.dim(); ++ip) {if(pid==plist[ip].pid)break;}
        if(ip<plist.dim()) {
            reqReSubmit(req, delayLaunch);
            return 0;
        }
    }
    reqSetData(req,"pid","%" DEC , (idx)getpid());

    sStr tmplt;
    sUsrObj pipelineObj(*user,sHiveId((formValue("pipeline"))) );
    if(!pipelineObj.Id()) {
        reqSetInfo(req, eQPInfoLevel_Error, "No pipeline specified\n" );
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    pipelineObj.getFilePathname(tmplt, "_.json");
    const char * flnm=ProcFile("pipeline.json",false);
    if(!sFile::exists( flnm )) {
        sFile::copy( tmplt,flnm );
    }

    sStr pars("\'app.verbose=1;debug on;");
    pars.printf("wflow._id=\"%s\";",objs[0].IdStr());
    prepareCmdLineArgsFromVars(objs, &pars, 0, "wflow-parameters", "wflow-", "%s=\"", "\";");
    pars.printf("\'");

    sFilePath flnmrun(flnm,"%%pathx-run.%%ext");

    sStr tus;
    sStrT pswd;formValue("pswd",&pswd);
    sStrT email;formValue("login",&email);
    if(email.length() ){
        tus.printf(" -user '%s' '%s' ",email.ptr(),pswd.length() ? pswd.ptr() : "");
    }
    sStrT cmdlineFmt;cfgStr(&cmdlineFmt, 0, "wflo.engine");
    sStrT cmdLine; cmdLine.printf(cmdlineFmt.ptr(),tus.length() ? tus.ptr(0) : "", pars.ptr(0),flnm);


    sStr tt;replaceObjMacros(tt, cmdLine);

    time_t ta = time(0);struct tm & t = *localtime(&ta);char outstr[256];
    sprintf(outstr, "%d/%d/%d %d:%d:%d",t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec);

    const char * logFile=ProcFile("pipeline-log.txt",false);sFile::chmod(logFile,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH );
    {sFil lf(logFile,sMex::fMapRemoveFile);lf.printf("Command line:%s:\n%s\n\n",outstr,tt.ptr());}

    tt.printf(" >> %s",logFile);
    sPS ps;

    ps.execute(tt);

    const char * errFile=ProcFile("pipeline-run-err.log",false);
    if(sFile::size(errFile)) {
        sFil errf(errFile,sMex::fReadonly);
        reqSetStatus(reqId, eQPReqStatus_ProgError );
        reqSetInfo(req, eQPInfoLevel_Error,"%.*s",(int)errf.length(),errf.ptr());
        return 0;
    }


    sJson pip;sFil fl(flnmrun);pip.initMem(fl,fl.length());

    sJson::Node steps(&pip,"$root.workflows.steps"),n;
    progressAll=steps.dim();
    for( idx i=0; i<steps.dim(); ++i ) {
        sJson::Node stp=steps[i];
        const char * status=stp["status"];
        if(status && sIsExactly(status,"done"))
            ++itemProgress;
    }
    pip.cln();

    if(itemProgress<progressAll)
        reqReSubmit(req, delayLaunch);
    else
        reqSetStatus(req, eQPReqStatus_Done );
    sFile::chmod(flnmrun.ptr(0),S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
    reqProgress(itemProgress, itemProgress*100./progressAll, 100);
    reqSetData(req,"pid",0,0);



    return 0;
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    WfloProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "wflo", argv[0]));
    return (int) backend.run(argc, argv);
}






