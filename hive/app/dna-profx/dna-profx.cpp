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

#include "dna-profx.hpp"

class DnaProfXProc : public sQPrideProc {
    public:

    DnaProfXProc(const char * defline00,const char * srv) : sQPrideProc(defline00,srv){}
    virtual idx OnExecute(idx);

};

idx DnaProfXProc::OnExecute(idx req)
{
    if (!isLastInGroup()){ //if multiple requests are submitted for profiler, only one has to be processed
        reqProgress(0, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }

    const char * algorithm = formValue("profSelector");
    std::auto_ptr<DnaProfX> profx;

    if( algorithm && strcmp(algorithm, "svc-profx-samtools") == 0 ) {
        profx.reset(new DnaProfXsamtools());
        profx->algorithm.printf("samtools");
    } else if (algorithm && strcmp(algorithm, "svc-profx-varscan") == 0) {
        profx.reset(new DnaProfXvarscan());
        profx->algorithm.printf("varscan");
    } else if( algorithm && strcmp(algorithm, "svc-profx-cuffdiff") == 0 ) {
        profx.reset(new DnaProfXcuffdiff());
        profx->algorithm.printf("cuffdiff");
    } else {
        logOut(eQPLogType_Error, "Unknown algorithm %s\n", algorithm ? algorithm : "unspecified");
        reqSetInfo(req,eQPInfoLevel_Error,"Unknown algorithm %s", algorithm ? algorithm : "unspecified");
        profx->qp->reqSetInfo(profx->qp->reqId, profx->qp->eQPInfoLevel_Error, "request %" DEC " was submitted with unknown algorithm %s", profx->qp->reqId, algorithm ? algorithm : "unspecified");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0; // error
    }

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ initialize the parameters
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    profx->qp = this;
    //profx->scoreFilter = formIValue("scoreFilter", 0);
    sIO log((idx) 0, (sIO::callbackFun) ::printf);

    // Determine the temporary directory assigned by the system
    cfgStr(&(profx->workDir), 0, "qm.tempDirectory");

    // Determine the system type (i.e. Linux)
    cfgStr(&profx->resourceRoot, 0, "qm.resourceRoot");

    profx->workDir.printf("profx-%" DEC, reqId);;
    sDir::makeDir(profx->workDir.ptr());
    if (!sDir::exists(profx->workDir.ptr())) {
        profx->qp->reqSetInfo(profx->qp->reqId, profx->qp->eQPInfoLevel_Error, "request %" DEC " could not create working directory %s", profx->qp->reqId, profx->workDir.ptr() ? profx->workDir.ptr() : "unspecified");
        reqSetStatus(req, eQPReqStatus_SysError);
        return 0;
    }

    if (!sDir::exists(profx->resourceRoot.ptr())) {
        profx->qp->reqSetInfo(profx->qp->reqId, profx->qp->eQPInfoLevel_Error, "request %" DEC " could not find resource Root %s", profx->qp->reqId, profx->resourceRoot.ptr() ? profx->resourceRoot.ptr() : "unspecified");
        reqSetStatus(req, eQPReqStatus_SysError);
        return 0;
    }

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ load the alignment object
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    const char * alignerID=formValue("parent_proc_ids");
    //sUsrObj profile(*sQPride::user, alignerID);

    sStr errMsg;
    idx retn = profx->PrepareData(*user, alignerID , (const char*)(profx->workDir.ptr()), errMsg);

    if (!retn) {
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errMsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    }
    reqProgress(1, 10, 100);


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ profiling
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    if(log)logOut(eQPLogType_Info,"RUNNING alignment: %s\n",log.ptr());
    sStr outputFile;
    idx pret = profx->Profile(&log, &outputFile, profx->workDir.ptr(), *user, alignerID);
    if (!pret) {
        reqSetInfo(reqId, eQPInfoLevel_Error, "Could not run profile.");
        reqSetStatus(reqId, eQPReqStatus_SysError);
        reqProgress(1, 50, 100);
        return 0;
    }

    idx finalizeData = profx->Finalize(&log, &outputFile, profx->workDir.ptr(), *user, alignerID);
    if (!finalizeData) {
        reqSetInfo(reqId, eQPInfoLevel_Error, "Could not Finalize profX.");
        reqSetStatus(reqId, eQPReqStatus_SysError);
        reqProgress(1, 50, 100);
        return 0;
    }
#if not _DEBUG
        sDir::removeDir(profx->workDir.ptr());
#endif

    reqProgress(outputFile.length(), 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);// change the status
    return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaProfXProc backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-profx",argv[0]));
    return (int)backend.run(argc,argv);
}



