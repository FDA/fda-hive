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

#ifndef _QPProcessHandler_qLib_hpp
#define _QPProcessHandler_qLib_hpp

#include <qlib/QPrideProc.hpp>
#include <ion/sJson.hpp>

namespace slib
{
    class sQPProcessHandler
    {
    public:
        sQPrideBase * qp;
        idx req;
        sStrT log;
        idx error,delayResubmit;
        sStrT mappath,workflowName,epoch,procFolder;
        sJson * workflow;
        sUsrObj * qpObjs;
        idx reportObjId;
        sUsrObj * reportO;


        sQPProcessHandler (const char * lworkflowName="config=qapp.cfg" __, sQPrideBase * lqp=0, sUsrObj * lobjs=0, const char * reportObj=0, idx lreq=0, const char * lmappath=0, idx ldelayResubmit=5){
            initHandler(workflowName,lqp,lobjs, reportObj, lreq, lmappath,ldelayResubmit);
        }
        sQPProcessHandler * initHandler(const char * lworkflowName, sQPrideBase * lqp=0, sUsrObj * lobjs=0, const char * reportObj=0, idx lreq=0, const char * lmappath=0, idx ldelayResubmit=5);

        ~sQPProcessHandler() { destroy();}
        void destroy(void) {
            if(mappath)serialize(mappath.ptr());
            objsets.empty();
            if(reportO){delete reportO;reportO=0;}
            if(workflow){delete workflow;workflow=0;}
        }

        bool err(bool condition, const char * fmt, ... );

        idx serialize(const char * path=0, bool clean=true);
        idx deserialize(const char * path=0);
        idx loadWorkflow(const char * workflowName, const char * namevar="name", const char * type="workflow", const char * filename="_.json");


        sDic < sVec < idx >  > objsets;
        sVar varset;
        idx setObjset(const char * role, const char * ids);
        idx launchProcess(const char * role, const char * svc, const char * jsonFmt...);
        idx launchProcess(const char * role, const char * svc, sUsrObj * obj=0, sVar * form=0 );
        const char * setEpoch(const char * lepoch){return (lepoch) ? epoch.printf(0,"%s",lepoch) : 0;};
        idx ensureProcess(const char * role, const char * svc=0, sUsrObj * obj=0, sVar * form=0);
        idx ensureProcess(const char * role, const char * svc, const char * jsonFmt, ... );
        sQPride::eQPReqStatus statusProcess(const char * roles, idx * prg=0, idx * prg100=0, sQPrideBase::eQPInfoLevel level=sQPrideBase::eQPInfoLevel_Error , sStr * logs=0);
        idx hasObjects(const char * roles, idx * ptot=0);
        idx procId(const char * role, idx num=0);
        idx canContinue(const char * role);
        idx cannotContinue(const char * role, const char * fmt, ... );

        idx initFormObject(const char * setname, const char * id );
        idx initObject(const char * setname, const char * id );
        idx cntObjset(const char * setname);
        const char * listObjset(const char * setname, sStr * buf=0, sVec < sHiveId > * hid=0,bool clnBuf=true);
        const char * listObjRegex(const char * setname, sStr * buf=0, sVec < sHiveId > * hid=0,bool clnBuf=true);
        idx searchObjects(const char * type=0, const char * setname=0, const char * pars=0, const char * vals=0, ...);
        idx searchArchived(const char * type, const char * roles, const char * setname=0);
        idx archiveReads(const char * setname, const char * source_setname, const char * typeOut, idx subId, const char * name );
        idx archiveCGI(const char * role, const char * source_setname, const char * fileName , const char * addFmt, ... );
        idx defaultFolder;
        idx setFolder(const char * role, const char * folderName, const char * parent=0);
        idx moveObject(const char * setname, idx proc=-1, idx folder=0, bool copy=true);
        idx moveSelf(idx folder=0, bool copy=false){return moveObject(0, qpObjs->Id().objId(), folder, copy);};

        idx setStageStatus(const char * role,sQPrideBase::eQPReqStatus status, idx progress, idx progress100, const char * errs=0);
        idx setFinalStatus(void);

        sQPrideBase::eQPReqStatus sumStat;
        idx sumProgress,sumProgress100,sumCnt;


        char * getFilePath(const char * role, const char * fileName=0, idx iNum=0, sStr * filename=0);
        sFil * getFile(const char * role, const char * fileName=0,idx iNum=0, idx mode=0, sFil * fl=0);

        idx ensureDownload(const char * role, const char * name, const char * source, const char * ids, ...  );

        idx copyFiles(const char * srcset, const char * filecard, const char * dstset=0, const char* dstflnm=0, bool doAppend=false,const char * prefix=0);
        
        const char * objQry(sStr & buf, const char * fmt, ... );


    };

    typedef bool (* sQPPRocessHandler_ModuleEntry)(void *, idx req);
    #define foreachFilePath(_v_fil, _v_role, _v_flnm)      char * _v_fil; for( idx QPProcHandler_iN=0; (_v_fil=getFilePath((_v_role), (_v_flnm), QPProcHandler_iN)) ;++QPProcHandler_iN)
    #define foreachFile(_v_fil, _v_role, _v_flnm, _v_mode)      sFil * _v_fil; for( idx QPProcHandler_iN=0; (_v_fil=getFile((_v_role), (_v_flnm), QPProcHandler_iN,(_v_mode))) ;++QPProcHandler_iN)

}
#endif 