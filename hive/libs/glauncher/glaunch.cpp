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
#include <ion/sJson.hpp>

const char * sGLauncherProc::inputFolderTemplate;

idx sGLauncherProc::OnExecute(idx req)
{
    processExecute(req);
    reqSetStatus(reqId, SR.status );
    reqProgress(SR.progress, SR.progress100, SR.progress100);

    return 0;
}

idx sGLauncherProc::processExecute(idx req)
{
    onReset();

    prepareFileAndDirs(objs, 0, inputFolderTemplate,objs, "fileSourceObjs");

    sStr cmdLine, logFileName, dst;
    cmdLineObjExec(&cmdLine, "sysapp+", formValue("sysapp"), "var:executable", &logFileName);
    const char * xtra=formValue("extra-params"); if(xtra)cmdLine.printf(" %s ; echo $!",xtra);

    sStrT sessionFile; ProcFile("sessionProgress.log",false,&sessionFile);
    idx pid=processLaunch(cmdLine, sessionFile);

    processMonitor(pid, sessionFile);

    return 0;
}

idx sGLauncherProc::processLaunch(const char * cmdLine, const char * sessionFile)
{
    sPipe ps;
    sStr dst;
    ps.exeSys(&dst,cmdLine);
    return dst.length() ? atoidx(dst) : 0;
}

sGLauncherProc::eQPReqStatus sGLauncherProc::processMonitor(idx pid, const char * sessionFile)
{
    idx iLoop, start=sTime::gmtNow();

    for ( iLoop=0; iLoop*sleepSec<maxLoopsSec; ++iLoop) {

        sessionCheck(sessionFile, &SR);
        pid=processCheck(pid);
        {
            if(!SR.progress)SR.progress=iLoop;
            if(!SR.progress100)SR.progress100=(maxLoopsSec/sleepSec);
            reqSetProgress(reqId,SR.progress,SR.progress100);

            if(!SR.status)SR.status=pid ? eQPReqStatus_Running : eQPReqStatus_Done;

            if(SR.info.length()){
                reqSetInfo(reqId,eQPInfoLevel_Info,"%s",SR.info.ptr());
                logOut(eQPLogType_Info, "%s",SR.info.ptr());
            }
            if(SR.error.length()){
                reqSetInfo(reqId,eQPInfoLevel_Error,"%s",SR.error.ptr());
                logOut(eQPLogType_Error, "%s",SR.error.ptr());
            }
            if(SR.warning.length()){
                reqSetInfo(reqId,eQPInfoLevel_Warning,"%s",SR.warning.ptr());
                logOut(eQPLogType_Warning, "%s",SR.warning.ptr());
            }
        }
        idx timeDiffInSec=SR.lastInteractionTime-start;

        if(!pid) {
            if(SR.status<eQPReqStatus_Done ) SR.status=eQPReqStatus_Done;
            break;
        }

        if(SR.status!=eQPReqStatus_Running ) {
            processStop(pid);
            pid=0;
            break;
        }

        if(timeDiffInSec>maxInactivitySec) {
            logOut(eQPLogType_Info, "Stopping process %" DEC" due to inactivity of %" DEC "(>%" DEC ") seconds\n", pid, timeDiffInSec,maxInactivitySec);
            processStop(pid);
            pid=0;
            break;
        }

        if(pid)
            jobRegisterAlive((idx)getpid(), reqId, 60, false);

        sleep(sleepSec);
    }

    return SR.status;
}

idx sGLauncherProc::sessionCheck(const char * sessionFile, SessionReport * sr)
{
    sr->info.cut(0);
    sr->error.cut(0);
    sr->warning.cut(0);

    if(sFile::exists(sessionFile)) {
        sJsonFile jproc( sessionFile );
        sr->lastInteractionTime=sTime::gmtScan("%Y-%m-%dT%H:%M:%SZ",jproc.value("$root.lastInteractionTime"),0);
        if(!sr->lastInteractionTime)sr->lastInteractionTime=sTime::gmtNow();

        sr->progress=jproc.ivalue("$root.progress");
        sr->progress100=jproc.ivalue("$root.progress100");
        const char * txt=jproc.value("$root.info");if(txt)sr->info.printf("%s",txt);
        txt=jproc.value("$root.error");if(txt)sr->error.printf("%s",txt);
        txt=jproc.value("$root.warning");if(txt)sr->warning.printf("%s",txt);
        txt=jproc.value("$root.status");if(txt)sString::xscanf(txt, "%n=0^any^waiting^processing^running^suspended^done^killed^progError^SysError^error;", &sr->status);
    }
    return sr->status;
}
idx sGLauncherProc::processCheck(idx pid)
{
    sStr oo,cmdps;
    sPipe mps;mps.exeSys(&oo, cmdps.printf(0,"ps -p %" DEC , pid), 0,0) ;
    return oo.length() ? atoidx (oo) : 0 ;
}

idx sGLauncherProc::processStop(idx pid)
{
    sStr cmdps;
    sPS ps;ps.execute(cmdps.printf(0,"kill -9 %" DEC , pid)) ;

    return pid;
}
