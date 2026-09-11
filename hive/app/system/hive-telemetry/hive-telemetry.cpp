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

#include <slib/std/app.hpp>
#include <qlib/QPrideProc.hpp>

using namespace slib;

class HiveTelemetry: public sQPrideProc
{
    private:

    public:
        HiveTelemetry(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }

        virtual idx OnExecute(idx req);
};


idx HiveTelemetry::OnExecute(idx req)
{
#ifdef _DEBUG
    fprintf(stderr, "qpride form for req %" DEC ":\n", req);
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
    fprintf(stderr, "vars for req %" DEC ":\n", req);
    for (idx i=0; i<vars.dim(); i++) {
        const char * key = static_cast<const char*>(vars.id(i));
        const char * value = vars.value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
#endif

    if( !user || !user->isAdmin() ) {
        reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Unauthorized or invalid user");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sUsr superuser("queen", true);
    if( !superuser.Id() ) {
        reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Failed to switch to superuser mode");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sUsr * cur_user = user;
    user = &superuser;

    sHiveId existingId;
    bool found=false;
    const char * objType = "hive-telemetry-data";

    sUsrObjRes obj_res; user->objs2(objType, obj_res,(udx*)0);
    for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
        sUsrObj * fobj = user->objFactory(*obj_res.id(it));
        if( fobj ) {
            existingId = *obj_res.id(it);
            found = true;
            break;
        }
    }
    std::auto_ptr<sUsrObj> newObj;
    sUsrObj * obj =0;
    if (!found) {
        sHiveId nid; sRC rc;
        rc = user->objCreate(nid, objType);
        if( !rc.isSet() ) {
            newObj.reset(user->objFactory(nid));
            obj = newObj.get();
            if( obj ) {
                reqSetInfo(req, eQPInfoLevel_Info, "created object %s", obj->Id().print());
            }
        }
        else {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "Cannot find/create object : %s", rc.print());
        }
    }
    else {
        obj = user->objFactory(existingId);;
        if (!obj) {
            reqSetInfo(req, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
            logOut(eQPLogType_Error, "Cannot access existing object %s", existingId.print());
        }
        ::printf("Updating existing object %s for hive-telemetry\n", obj->Id().print());
    }
    if (obj) {
        sStr path; obj->getFilePathname(path);

        sStrT cmdLine;replaceObjMacros(cmdLine,"$(obj)/runner.sh","sysappdep","hive-telemetry");        
        
        sPipe2::CmdLine cmdLineBuilder;            
        cmdLineBuilder.exe(cmdLine.ptr(0));
        cmdLineBuilder.arg("--out");
        sStr outputPath("%s/hello.txt",path.ptr());
        cmdLineBuilder.arg(outputPath.ptr());
         
        logOut(eQPLogType_Debug, "Running: %s", cmdLineBuilder.printBash());
        ::printf("Running: %s", cmdLineBuilder.printBash());
        sIO log;
        sPipe mps; mps.exeSys(&log, cmdLineBuilder.printBash(), 0,0) ;

        reqReSubmit(300);
    }

    user = cur_user;
    return 0;
}


int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    HiveTelemetry backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "hive-telemetry", argv[0]));
    return (int) backend.run(argc, argv);
}
