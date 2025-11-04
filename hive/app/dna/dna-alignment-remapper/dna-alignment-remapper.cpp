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
#include <violin/hiveproc.hpp>

#include <slib/utils.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>


#define REALFNLNM "alignment.hiveal"

class DnaAlignmentRemapper:public sHiveProc
{
    public:
        DnaAlignmentRemapper(const char * defline00,const char * srv) : sHiveProc(defline00,srv){}

        sRC remap(sHiveal * hiveal, sStr &path, sStr &subject, sVec<idx> & alignmentMap, idx chnk_start, idx chnk_cnt);

        virtual idx OnExecute(idx);
};

sRC DnaAlignmentRemapper::remap(sHiveal * hiveal, sStr &path, sStr &subject, sVec<idx> & alignmentMap, idx chnk_start, idx chnk_cnt)
{
    sRC rc;
    sHiveId mutualAligmentID(formValue("multipleAlignment"));
    sUsrObj mutAligner(*user, mutualAligmentID);

    if( mutAligner.Id() ) {
        sStr mut_path;
        mutAligner.getFilePathname00(mut_path, "alignment.hiveal" _ "alignment.vioal" __);
        sHiveal mutAl(user, mut_path);
        sHiveseq Sub_mut(sQPride::user, subject.ptr(),mutAl.getSubMode());
        mutAl.Qry = &Sub_mut;
        sHiveal * mutAlT = &mutAl;
        sUsrFile mutualAligmentObj(mutualAligmentID, sQPride::user);
        if( !mutAlT->isok() || mutAlT->dimAl()==0){
            progress100End=5;

            sStr mutualAligmentFilePath;
            if( mutualAligmentObj.Id() ) {
                mutualAligmentObj.getFile(mutualAligmentFilePath);
            } else {
                return sRC(sRC::eProcessing,sRC::eFile,sRC::eObject,sRC::eNotFound);
            }
            sStr log;

            if(sFile::size(mutualAligmentFilePath)) {

                sVec < idx > alignmentMap;

                sFil fl(mutualAligmentFilePath,sMex::fReadonly);

                sBioseqAlignment::readMultipleAlignment(&alignmentMap, fl.ptr(),fl.length(), sBioseqAlignment::eAlRelativeToMultiple, 0, false);

                sStr pathT;
                reqSetData(reqId, "file://alignment-slice.vioal", 0, 0);
                reqDataPath(reqId, "alignment-slice.vioal", &pathT);
                sFile::remove(pathT);
                {
                    sFil ff(pathT);
                    sBioseqAlignment::filterChosenAlignments(&alignmentMap,0,0, &ff);
                }

                sStr dstAlignmentsT;mutualAligmentObj.addFilePathname(dstAlignmentsT, true, "alignment.hiveal");
                idx flagSet=sBioseqAlignment::fAlignForward;
                sVioal vioAltAAA(0,&Sub_mut,&Sub_mut);

                vioAltAAA.progress_CallbackFunction = sQPrideProc::reqProgressStatic;
                vioAltAAA.progress_CallbackParam = (void *)this;

                sVioal::digestParams params;
                params.flags= flagSet;
                params.countHiveAlPieces = 1000000;
                params.combineFiles = false;
                vioAltAAA.DigestCombineAlignmentsRaw(dstAlignmentsT,pathT, params);
                mutAl.parse(dstAlignmentsT);
                mutAlT = &mutAl;
            } else{
                return sRC(sRC::eProcessing,sRC::eFile,sRC::eInput,sRC::eNotFound);
            }
        }
        progress100End=100;
        hiveal->progress_CallbackFunction = sQPrideProc::reqProgressStatic;
        hiveal->progress_CallbackParam = (void *)this;

        if(hiveal->stableRemap(mutAlT,alignmentMap,chnk_start,chnk_cnt)) {
            return sRC::zero;
        }
        return sRC(sRC::eProcessing, sRC::eFile, sRC::eAlgorithm, sRC::eFailed);
    }
    return sRC(sRC::eProcessing,sRC::eFile,sRC::eInput,sRC::eNotFound);
}

idx DnaAlignmentRemapper::OnExecute(idx lreq)
{
    sHiveId alignerID(formValue("alignment"));
    sUsrObj aligner(*user, alignerID);
    sStr path;
    if( aligner.Id() ) {
        aligner.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __);
    } else {
        reqSetInfo(reqId,eQPInfoLevel_Error,"Object not found or access denied %s\n", alignerID.print());
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    std::auto_ptr<sHiveal> hiveal( new sHiveal(user, path) );
    if(!hiveal.get()) {
        reqSetInfo(reqId,eQPInfoLevel_Error,"Alignment object %s is missing or corrupted\n", alignerID.print());
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }

    if( !hiveal->isok() || hiveal->dimAl()==0){
        sFilePath flpthnm(path,"%%flnm");
        reqSetInfo(reqId,eQPInfoLevel_Error,"Alignment file '%s' is missing or corrupted\n", flpthnm ? flpthnm.ptr() : "unspecified");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }

    sStr subject;
    QPSvcDnaHexagon::getSubject00(aligner,subject);
    sHiveseq Sub(sQPride::user, subject.ptr(),hiveal->getSubMode());
    if(Sub.dim()==0) {
        sFilePath flpthnm(subject,"%%flnm");
        reqSetInfo(reqId,eQPInfoLevel_Error,"Reference '%s' sequences are missing or corrupted\n",flpthnm ? flpthnm.ptr() : "unspecified");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }

    hiveal->Sub=&Sub;

    sStr query;
    QPSvcDnaHexagon::getQuery00(aligner,query);
    sHiveseq Qry(sQPride::user, query,hiveal->getQryMode());
    if(Qry.dimRefs()>5)Qry.reindex();
    if(Qry.dim()==0) {
        sFilePath flpthnm(query,"%%flnm");
        reqSetInfo(reqId, eQPInfoLevel_Error, "Query '%s' sequences are missing or corrupted\n",flpthnm ? flpthnm.ptr() : "unspecified");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    hiveal->Qry=&Qry;

    std::auto_ptr<sUsrObj> obj;

    const char * passed_obj = formValue("obj");
    if(passed_obj) {
        const sHiveId objID(passed_obj);
        obj.reset(user->objFactory(objID));
    } else {
        obj.reset(new sUsrObj(*user,"svc-align"));
        obj->propSet("submitter","dna-hexagon");
        sStr remapped_name("remapped %s", aligner.propGet("name"));
        obj->propSet("name",remapped_name);
        sVec<sHiveId> qids, sids;
        sHiveId::parseRangeSet(qids, query);
        obj->propSetHiveIds("query", qids);
        sHiveId::parseRangeSet(sids, subject);
        obj->propSetHiveIds("subject", sids);
    }
    if( !obj.get() || !obj->Id() ) {
        reqSetInfo(reqId,eQPInfoLevel_Error,"Object could not be constructed\n");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }

PERF_START("SORTING ALIGNMENTS");

#define REALCHNKNM "re-alignment-slice.vioalt"

    const char * alflnm = formValue("resultsFileName");
    sFilePath flpth;
    if( alflnm ) {
        alflnm = flpth.makeName(alflnm,"%%flnm");
    } else {
        alflnm = REALFNLNM;
    }

    sStr pathT;
    if(!reqAddFile(pathT,"reqself-" REALCHNKNM) ) {
        reqSetInfo(reqId, eQPInfoLevel_Fatal, "Cannot create temporary file " REALCHNKNM);
        reqSetStatus(reqId,eQPReqStatus_SysError);
    }

    sVec<idx> almap;
    idx chnk_start = (hiveal->dimAl()*reqSliceId)/reqSliceCnt;
    idx chnk_cnt = ((hiveal->dimAl()*(reqSliceId+1))/reqSliceCnt) - chnk_start;
    sRC rc = remap(hiveal.get(), path, subject, almap, chnk_start, chnk_cnt);
    if( rc != sRC::zero ) {
        reqSetInfo(reqId, eQPInfoLevel_Error, "%s", rc.print());
        reqSetStatus(reqId,eQPReqStatus_ProgError);
        return 0;
    }

    reqSetProgress(reqId, chnk_cnt, progress100End);
    sFil ff(pathT);
    sBioseqAlignment::filterChosenAlignments(&almap,0,0, &ff);



    if( isLastInMasterGroup() ) {
        ff.destroy();
        progress100Start=50;
        progress100End=100;
        reqSetProgress(reqId, 0, progress100Start );

        sVec < idx > reqList; grp2Req(masterId, &reqList, vars.value("serviceName"));

        sVec<idx> remappedHits;
        sStr srcAlignmentsT00;

        grpDataPaths(masterId, "reqself-" REALCHNKNM, &srcAlignmentsT00);

        sStr pathT;
        obj->addFilePathname(pathT, true, alflnm);
        if(!pathT.ptr()) {
            reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to add file for remapped alignments");
            reqSetStatus(reqId, eQPReqStatus_SysError);
            return 0;
        }

        sVioal vioAltAAA(0,hiveal->Sub, hiveal->Qry);
        vioAltAAA.progress_CallbackFunction = sQPrideProc::reqProgressStatic;
        vioAltAAA.progress_CallbackParam = (void *)this;

        sVioal::digestParams params;
        params.flags= 0;
        params.countHiveAlPieces = 4000000;
        params.combineFiles = false;
        vioAltAAA.DigestCombineAlignmentsRaw(pathT, srcAlignmentsT00, params);

        const char * serviceToLaunce = formValue("serviceToBeLaunched");

        if(serviceToLaunce) {
            idx reqCollector = reqSubmit(serviceToLaunce);
            grpAssignReqID(reqCollector, grpId, reqSliceCnt);
            reqSetAction(reqCollector, eQPReqAction_Run);
        }

        reqSetProgress(reqId, chnk_cnt + reqList.dim(), 100);
    } else {
        reqSetProgress(reqId, chnk_cnt, 100);
    }
    reqSetStatus(reqId, eQPReqStatus_Done);

    return 0;
}


int main(int argc,const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT );

    sStr tmp;
    sApp::args(argc,argv);

    DnaAlignmentRemapper backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-alignment-remapper",argv[0]));
    return (int)backend.run(argc,argv);
}
