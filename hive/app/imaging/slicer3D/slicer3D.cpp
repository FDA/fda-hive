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
#include <gdocker.hpp>
#include <ion/sJson.hpp>
using namespace slib;

#define PATIENT_MRN_UID
class Slicer3DProc: public sGDockerProc
{
    public:
        Slicer3DProc(const char * defline00, const char * srv)
            : sGDockerProc(defline00, srv)
        {
        }
        idx OnExecute(idx req);
};



idx Slicer3DProc::OnExecute(idx req)
{

    sStr uid,buf;
    #ifdef PATIENT_MRN_UID
        uid.printf("%s",formValue("mrn"));
    #else
        uid.printf("%s.",formValue("objType"));
        uid.printf("%s.",formValue("objID"));
        uid.printf("%s",formValue("widgetID"));
    #endif

    sStrT dockerContainerName;dockerContainerName.printf("OBJ-%" DEC,objs[0].Id().objId());
    propSet("dockerContainerName", dockerContainerName.ptr());

    user->ensureUniqueObjectProvided(objs, "sysdocker","name","slicer3D","sysdocker");
    user->ensureUniqueObjectProvided(objs, "sysapp","name","slicer3D","sysappdep");
    propSet("uniqueIDPath",objs[0].getFilePathname(buf));buf.cut(0);
    propSet("hostName", cfgStr(&buf,0,"internalWWW"));
    propSet("PID", uid.ptr());


    sUsrObj objWorkspace;
    user->createSharedUniqueObject(&objWorkspace, "slicer3D-workspace", "uID",uid.ptr(0), "Inbox", formValue("share") );
    propSet("workspaceId", objWorkspace.IdStr());

    inputFolderTemplate="DICOMImports";
    processExecute(req);


    if( !SR.error) {
        reqSetStatus(reqId, SR.status );
        reqProgress(SR.progress, SR.progress100, SR.progress100);
    } else {
        logOut(eQPLogType_Error, "%s\n", SR.error.ptr());
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



