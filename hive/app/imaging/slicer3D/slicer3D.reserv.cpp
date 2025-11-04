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



#include <slib/utils/tbl.hpp>
#include <qlib/QPrideProc.hpp>
#include <ion/sJson.hpp>
using namespace slib;

#define PATIENT_MRN_UID

idx doesEnd(const char * path,  const char * end , idx value=1){
    idx len_path = sLen(path);
    idx len_end = sLen(end);
    return (len_path >= len_end && strncasecmp(end,path+len_path-len_end,len_end)==0) ? value : 0;
}




class Slicer3DProc: public sQPrideProc
{
    public:
        Slicer3DProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {

        }
        idx OnExecute(idx req);
};



idx Slicer3DProc::OnExecute(idx req)
{
    sStr errmsg;
    idx pid=objs[0].Id().objId();

    sStr uid;
    #ifdef PATIENT_MRN_UID
        uid.printf("%s",formValue("mrn"));
    #else
        uid.printf("%s.",formValue("objType"));
        uid.printf("%s.",formValue("objID"));
        uid.printf("%s",formValue("widgetID"));
    #endif

    sStrT dockerUname;
    dockerUname.printf("OBJ-%" DEC,pid);
    sString::searchAndReplaceSymbols(dockerUname,0, "@", "_", 0, true, false, false, false, 0);


    sUsrObjRes obj_res;
    sHiveId hid;

    idx maxInactivitySec=formIValue("maxInactivitySec",15*60),sleepSec=formIValue("sleepSec",3),maxLoopsSec=formIValue("maxLoopsSec",24*60*60);

    bool isnew=true;
    if( user->objs2("^slicer3D-workspace", obj_res,(udx*)0,"uID", uid) )  {
        hid=*obj_res.id(obj_res.first());
        isnew=false;
    }else {
        user->objCreate(hid, "slicer3D-workspace");
        const char * shareTo=formValue("share");
        if(shareTo && *shareTo) {
            sVec<idx> gs;
            user->groupIdFromPath(shareTo, &gs);
            for(idx g = 0; ok && g < gs.dim(); ++g) {
                user->setPermission(gs[g],hid,7,2);
            }
        }
    }
    sUsrObj obj(*user,hid);
    if(isnew) {
        obj.propSet("uID", uid);
    }

    const char * grp ="1", * val=hid.print();
    objs[0].propSet("workspaceId", &grp, &val,1);


    sStr pathB; char * path;
    path=(char*)obj.addFilePathname(pathB, false, "DICOMImports");
    obj.propSet("folder", "Inbox");

    if(path){
        idx pathLen=pathB.length();
            sDir::makeDir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            sUsrObj oform(*user,sHiveId(formValue("objID")));
            sStrT buf;
            const char * dicomObjList00=oform.Id() ? oform.propGet00("files",&buf) :0 ;

                for ( const char * dicomObj=dicomObjList00; dicomObj; dicomObj=sString::next00(dicomObj) ) {

                    sHiveId hido(dicomObj);
                    sUsrObj o(*user,hido);
                    sDir d;sStr pathL;o.getFilePathname(pathL);
                    idx lPathLen=pathL.length();
                    o.files(d, sFlag(sDir::bitSubdirs)|sFlag(sDir::bitFiles),"*" );

                    for ( const char * p=d.ptr(); p; p=sString::next00(p) ) {
                        pathB.printf(pathLen,"/o%s-%s",dicomObj,sFilePath::nextToSlash(p));
                        pathL.printf(lPathLen,"%s",p);

                        sStr cmd;
                        if( doesEnd(p,".zip",1) ){
                            cmd.printf("mkdir %s; unzip %s -x -d %s",pathB.ptr(0),pathL.ptr(0),pathB.ptr(0));
                        }else  if( doesEnd(p,".tar.gz",2) ) {
                            cmd.printf("mkdir %s; tar xvfz %s -C %s " ,pathB.ptr(0),pathL.ptr(0),pathB.ptr(0));
                        }
                        else if (doesEnd(p,".gzip",3) ) {
                            cmd.printf("gunzip %s > %.*s " ,pathL.ptr(0),(int)(pathB.length()-5),pathB.ptr(0));
                        }
                        else if (doesEnd(p,".gz",3)) {
                            cmd.printf("gunzip %s > %.*s " ,pathL.ptr(0),(int)(pathB.length()-3),pathB.ptr(0));
                        }

                        if(cmd.length()){
                            sPS ps;
                            ps.exec(cmd);
                        }
                        else
                            sFile::copy(pathL.ptr(),pathB.ptr());
                    }
                }
        pathB.cut(pathLen);pathB.add0();
        path=pathB.ptr(0);

    }else {
        path=(char*)obj.getFilePathname(pathB, "DICOMImports");
    }
    path[sLen(path)-13]=0;


    sUsrObjRes obr;
    const char * serviceID=pForm->value("version",0);
    sStr pathAlgo;
    if( user->objs2("^sysdocker", obr,(udx*)0,serviceID ? "_id" : "name", serviceID ? serviceID : "^slicer3D$") ) {
        sUsrObj serviceAlgo( *user, *obr.id(obj_res.first()) );
        serviceAlgo.getFilePathname(pathAlgo);

    }


    sPipe mps;
    sStr oo,cmdps;
    cmdps.printf(0,"docker ps | grep '%s'",dockerUname.ptr());
    mps.exeSys(&oo, cmdps.ptr(0), 0,0) ;
    idx dockerId=0;
    if(oo.length()) sscanf(oo.ptr(),"%llx",(udx*)(&dockerId));
    bool demoMode=formBoolValue("demo", false) ;

    if(!dockerId) {
        sStrT cmdline;
        const char * find00 = "%ALGOPATH%" _ "%UID%" _ "%MRN%" _ "%FOLDER%" _ "%IMPORT%" _ "%WIDGETID%" _ "%IMAGETYPE%" _ "%SCREENSIZE%" _ "%USER%" _ "%SLICERPORT%" _ "%VNCPORT%" _ "%NOVNCPORT%" _ "%IGTLINKPORT%" _ "%CURRENTHOSTNAME%" _ "%PID%" _ "%TIMEOUT%" _ "%HIVENETWORK%" _ "%NAME%" __ ;
        sStr replace00;
        #define RPL(_v_n) {formValue((_v_n),&replace00," ");replace00.cut(-1);}

        replace00.printf(pathAlgo.length() ? pathAlgo.ptr() : "/home/qpride/bin/HiveSlicerVNCWidget");replace00.add0();
        replace00.printf(uid.ptr());replace00.add0();
        RPL("mrn");
        replace00.printf("%s/",demoMode ? "/home/qpride/bin/HiveWidgetFolder-Demo" : path);replace00.add0();
        if(demoMode)replace00.printf("%s","NO_DICOM");else replace00.printf("%s/Incoming/",path);replace00.add0();
        RPL("widgetID");
        RPL("imageType");
        RPL("screenSize");
        RPL("user");
        RPL("slicerPort");
        RPL("VNCPort");
        RPL("noVNCPort");
        RPL("igtLinkPort");
        replace00.printf("localhost");replace00.add0();
        replace00.printf("%" DEC, pid);replace00.add0();
        replace00.printf("%" DEC, maxInactivitySec);replace00.add0();
        replace00.printf("HIVE-NETWORK-");RPL("uID");
        replace00.printf("%s", dockerUname.ptr());replace00.add0();
        replace00.add0(2);
        sStrT cmdlineFmt;
        cfgStr(&cmdlineFmt, 0, "slicer3D.start");
        sString::searchAndReplaceStrings(&cmdline, cmdlineFmt, 0 , find00, replace00.ptr(0), 0, false);

        if(isnew) {
            obj.propSet("cmd", cmdline.ptr(0));
        }else {
            obj.propSet("cmd",cmdline.ptr(0));
        }

        {
            sPipe psex;
            sFil log(ProcFile("slicer-docker.log", true));
            log.printf("command line : %s\n\n\n",cmdline.ptr(0));
            psex.exeSys(&log,cmdline);
            log.printf("command line : %s\n",cmdline.ptr(0));
        }

        oo.cut(0);
        mps.exeSys(&oo, cmdps.ptr(0), 0,0) ;
        if(oo.length()) sscanf(oo.ptr(),"%llx",(udx*)(&dockerId));
        if(!dockerId)  {
            logOut(eQPLogType_Error, "Could not initiate a slicer docker image\n");
            reqSetStatus(req, eQPReqStatus_ProgError);
        }

    }

    maxInactivitySec*=2;
    sStrT sessionFile;sessionFile.printf("%s/Sessions/pid_%" DEC".json",path,pid);
    idx dockerMonId=0;
    idx start=sTime::gmtNow();
    maxLoopsSec=0;
    for ( idx iLoop=0; iLoop*sleepSec<maxLoopsSec; ++iLoop) {

        dockerMonId=0;
        oo.cut(0);mps.exeSys(&oo, cmdps.ptr(0), 0,0) ;
        if( oo.length() ) sscanf(oo.ptr(),"%llx",(udx*)(&dockerMonId));
        if(!dockerMonId) {
            logOut(eQPLogType_Info, "slicer docker OBJ-%" DEC " has quit after %" DEC " seconds of monitoring\n", pid, iLoop*sleepSec);
            break;
        }

        idx now=sTime::gmtNow();
        idx timeDiffInSec=now-start;
        if(sFile::exists(sessionFile.ptr())) {
            sJsonFile jproc( sessionFile.ptr() );
            const char * ptim=jproc.value("$root.lastInteractionTime");

            idx update=sTime::gmtScan("%Y-%m-%dT%H:%M:%SZ",ptim,start);

            timeDiffInSec=now-update;

            if(timeDiffInSec>maxInactivitySec) {
                sStrT cmdlineFmt,cmdline,t;
                cfgStr(&cmdlineFmt, 0, "slicer3D.stop");
                if(cmdlineFmt.ptr(0)){
                    t.printf("%" HEX, dockerMonId);t.add0(2);
                    sString::searchAndReplaceStrings(&cmdline, cmdlineFmt, 0 , "%DOCKERID%", t.ptr(0), 0, false);
                    sPS kill;
                    kill.exec(cmdline);
                }
                logOut(eQPLogType_Info, "stopping slicer docker %" HEX " due to inactivity of %" DEC "(>%" DEC ") seconds\n", dockerMonId, timeDiffInSec,maxInactivitySec);
                break;
            }

        }

        jobRegisterAlive((idx)getpid(), req, 60, false);
        logOut(eQPLogType_Info,"last activity of docker %" HEX " was %" DEC " seconds ago \n",dockerMonId, timeDiffInSec);
        sleep(sleepSec);
        reqSetProgress(0, 99, 100);
    }


    if( !errmsg ) {
        reqProgress(0, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    } else {
        logOut(eQPLogType_Error, "%s\n", errmsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    }

    return 0;
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    Slicer3DProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "slicer3D", argv[0]));
    return (int) backend.run(argc, argv);
}



