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
#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>


extern sPerf bioPerf;

class DnaRecomb:public sQPrideProc
{
    public:
        DnaRecomb(const char * defline00,const char * srv):sQPrideProc(defline00,srv) {}

        static idx similCumulator(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum);

        idx parseMultipleAlignments(sHiveId &mutualAlignmentID, sHiveal & mutAl);

        struct RecombData {
            idx * simil,* simcov;
            idx * acgt_indel;
            sBioal * mutAl;
            idx startAl,endAl,req;
            void * obj;
            idx alignLen;
        };
        virtual idx OnExecute(idx);
};

idx DnaRecomb::parseMultipleAlignments(sHiveId &mutualAlignmentID, sHiveal & mutAl)
{
    sUsrFile mutualAlignmentObj(mutualAlignmentID, sQPride::user);
    progress100Last=1;
    idx mutualAlMode =1;
    sStr mutualAlignmentFilePath;
    if( mutualAlignmentObj.Id() ) {
        mutualAlignmentObj.getFile(mutualAlignmentFilePath);
    } else {
        reqSetInfo(reqId,eQPInfoLevel_Error,"Object not found or access denied %s.\n", mutualAlignmentID.print());
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    sStr log;

    if(sFile::size(mutualAlignmentFilePath)) {

        sVec < idx > alignmentMap;

        sFil fl(mutualAlignmentFilePath,sMex::fReadonly);

        sBioseqAlignment::readMultipleAlignment(&alignmentMap, fl.ptr(),fl.length(), sBioseqAlignment::eAlRelativeToMultiple, 0, false);

        sStr pathT;
        reqSetData(reqId, "file://alignment-slice.vioal", 0, 0);
        reqDataPath(reqId, "alignment-slice.vioal", &pathT);
        sFile::remove(pathT);
        {
            sFil ff(pathT);
            sBioseqAlignment::filterChosenAlignments(&alignmentMap,0,0, &ff);
        }

        sStr dstAlignmentsT;mutualAlignmentObj.addFilePathname(dstAlignmentsT, true, "alignment.hiveal" );
        idx flagSet=sBioseqAlignment::fAlignForward;
        sVioal vioAltAAA(0,mutAl.Qry,mutAl.Qry);

        vioAltAAA.progress_CallbackFunction = sQPrideProc::reqProgressStatic;
        vioAltAAA.progress_CallbackParam = (void *)this;

        sVioal::digestParams params;
        params.flags= flagSet;
        params.countHiveAlPieces = 1000000;
        params.combineFiles = false;
        vioAltAAA.DigestCombineAlignmentsRaw(dstAlignmentsT,pathT, params);
        mutAl.parse(dstAlignmentsT);
    }
    else{
        reqSetInfo(reqId,eQPInfoLevel_Error,"Alignment file '%s' is missing or corrupted\n", mutualAlignmentFilePath.ptr() ? mutualAlignmentFilePath.ptr() : "unspecified");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    return 1;
};

idx DnaRecomb::similCumulator(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum)
{
    RecombData  * SPData=(RecombData *)param->userPointer;

    m = sBioseqAlignment::uncompressAlignment(hdr,m);

    idx iSub = SPData->mutAl->Qry->getmode()? SPData->mutAl->Qry->short2long(hdr->idSub()) : hdr->idSub();
    sBioseqAlignment::Al * hdrTo= SPData->mutAl->getAl(iSub);
    idx * mTo=SPData->mutAl->getMatch(iSub);

    sBioseqAlignment::Al hdrToWrite=*hdr;
    hdr=&hdrToWrite;
    sBioseqAlignment::remapAlignment(&hdrToWrite, hdrTo, m, mTo ) ;
    const char * qrybits=bioal->Qry->seq(hdr->idQry());
    char ch=0;
    idx qrybuflen=bioal->Qry->len(hdr->idQry());

    idx ishift=iSub*SPData->alignLen;
    idx lastSubOK=0;
    for (idx ipp=0; ipp< hdr->lenAlign() ; ipp++){
        idx is=hdr->getSubjectPosition(m,ipp);
        idx iq=hdr->getQueryPosition(m,ipp,qrybuflen);

        if(is<0){ ch=4; is=lastSubOK;}
        else if(iq<0)ch=5;
        else {
            ch=hdr->getQueryLetterByPosition(iq,qrybits);
        }

        if(is>=0 ){
            SPData->simil[ishift+is]+=hdr->score();
            ++SPData->simcov[ishift+is];
        }

        ++SPData->acgt_indel[is*6+ch];

    }

    ((DnaRecomb *)SPData->obj)->reqProgressStatic(SPData->obj, iNum - SPData->startAl + 1, iNum - SPData->startAl + 1, SPData->endAl - SPData->startAl);

    return 1;
}


idx DnaRecomb::OnExecute(idx req)
{
    sStr errS;


    sBioal::ParamsAlignmentIterator PA;
    RecombData RD;
    sVec < idx > SimilStack(sMex::fSetZero);
    PA.userPointer=(void*)&RD;


    sHiveId alignerID(formValue("parent_proc_ids"));
    sUsrObj aligner(*sQPride::user, alignerID);

    sStr subject;
    QPSvcDnaHexagon::getSubject00(aligner,subject);
    sStr query;
    QPSvcDnaHexagon::getQuery00(aligner,query);

    sStr path;
    aligner.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __);
    sHiveal hiveal(user, path);
    sBioal * bioal = &hiveal;
    bioal=&hiveal;

    if( !bioal->isok() || bioal->dimAl()==0){
        errS.printf("Alignment file %s is missing or corrupted\n", alignerID.print());
        logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);

        return 0;
    }

    sHiveId mutualAlignmentID(formValue("mutualAligmentID"));
    sStr mutualAlignmentPath;
    sUsrFile mutualAlignmentObj(mutualAlignmentID, sQPride::user);
    if( mutualAlignmentObj.Id() ) {
        mutualAlignmentObj.getFilePathname00(mutualAlignmentPath,"alignment.vioalt" _ "alignment.vioal" _ "alignment.hiveal" __);
    } else {
        errS.printf("Mutual Alignment object not found or corrupted %s\n", mutualAlignmentID.print());
        logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    sHiveal mutAl(user, mutualAlignmentPath);
    sBioseq::EBioMode mutualAlMode = sBioseq::eBioModeLong;
    sHiveseq Sub_mut(sQPride::user, subject ,mutualAlMode);
    mutAl.Qry = &Sub_mut;
    if(!mutAl.isok() && !parseMultipleAlignments(mutualAlignmentID,mutAl) ) {
        return 0;
    }
    RD.mutAl = &mutAl;
    RD.alignLen = 0;
    sBioseqAlignment::Al * hdr = 0;
    sVec<idx> mutual_uncompressMM;
    for( idx iMutAl = 0, is =0 , *m =0  ; iMutAl < RD.mutAl->dimAl() ; ++iMutAl ) {
        m = RD.mutAl->getMatch(iMutAl);
        hdr = RD.mutAl->getAl(iMutAl);
        is = hdr->getQueryEnd(m);
        if ( RD.alignLen < is) {
            RD.alignLen = is;
        }
    }



    sHiveseq Sub(sQPride::user, subject.ptr());Sub.reindex();
    if(Sub.dim()==0) {
        errS.printf("Reference '%s' sequences are missing or corrupted\n", subject.length() ? subject.ptr() : "unspecified");
        logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    RD.mutAl->Qry = &Sub;


    sHiveseq Qry(sQPride::user, query.ptr());Qry.reindex();
    if(Qry.dim()==0) {
        errS.printf("Query '%s' sequences are missing or corrupted\n",query.ptr() ? query.ptr() : "unspecified");
        logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }



    bioal->Sub=&Sub;
    bioal->Qry=&Qry;

    sVec < idx > subsetN;sString::scanRangeSet(formValue("subSet",0,""), 0, &subsetN, -1, 0, 0);
    idx numAlChunks=subsetN.dim() ? subsetN.dim() : Sub.dim() ;
    idx totalAls=0,cntFound=0,dimAl;
    if( !subsetN.dim() ) {
        totalAls = bioal->dimAl();
    } else {
        for( idx is=0; is<numAlChunks; ++is ) {
            bioal->listSubAlIndex(subsetN.dim() ? subsetN[is] : is, &dimAl);
            totalAls+=dimAl;
        }
    }
    idx slice = (totalAls - 1) /reqSliceCnt + 1;

    sStr recombName,recombPath;
    for( idx is=0; is<numAlChunks; ++is) {
        idx iSub= subsetN.dim() ? subsetN[is] : is;

        bioal->listSubAlIndex(iSub, &dimAl);

        idx start=reqSliceId*slice, end=start+slice;
        start-=totalAls;end-=totalAls;
        if(start<0)start=0;
        if(end>dimAl)end=dimAl;

        if(start>=dimAl){totalAls+=dimAl;continue;}
        if(end<0)break;

        RD.startAl = start;RD.endAl = end;RD.req = req; RD.obj = (void *)this;
        if(!SimilStack.dim()) {
            recombName.printf(0,"file://recomb-slice-%" DEC ".vioprof",reqSliceId);
            reqSetData(req,recombName,0,0);
            reqDataPath(req,recombName.ptr(7),&recombPath);
            sFile::remove(recombPath);
            SimilStack.init(recombPath,sMex::fSetZero|sMex::fExactSize);
            RD.simil=SimilStack.add(RD.alignLen*Sub.dim()*2+RD.alignLen*6);
            RD.simcov=RD.simil+RD.alignLen*Sub.dim();
            RD.acgt_indel=RD.simcov+RD.alignLen*Sub.dim();

        }

        cntFound+=bioal->iterateAlignments(0, start, end-start, iSub, (sBioal::typeCallbackIteratorFunction)&similCumulator, (sBioal::ParamsAlignmentIterator *) &PA);

        totalAls+=dimAl;
    }

    SimilStack.destroy();

    logOut(eQPLogType_Info,"Analyzing results\n");
    reqProgress(cntFound, 100, 100);

    if ( !isLastInMasterGroup() ) {
        reqSetStatus(req, eQPReqStatus_Done);
        logOut(eQPLogType_Info,"Done processing %" DEC " request for execution\n",req);
        return 0;
    }


    PERF_START("Output");
    sVec < idx > reqList; grp2Req(grpId, &reqList, vars.value("serviceName"));

    sVec < idx > AllSimilVec(sMex::fExactSize|sMex::fSetZero);
    AllSimilVec.addM(RD.alignLen*Sub.dim()*2+RD.alignLen*6);
    idx * AllSimil=AllSimilVec.ptr();
    idx * AllSimcov=AllSimil+RD.alignLen*Sub.dim();
    idx * AllAcgt_indel=AllSimcov+RD.alignLen*Sub.dim();

    for( idx ir=0; ir<reqList.dim(); ++ir ) {
        recombName.printf(0,"recomb-slice-%" DEC ".vioprof",ir);
        recombPath.cut(0);
        reqDataPath(reqList[ir],recombName.ptr(0),&recombPath);
        sVec <idx > stack(sMex::fReadonly|sMex::fExactSize,recombPath);
        idx * simil=stack.ptr();
        idx * simcov=simil+RD.alignLen*Sub.dim();
        idx * acgt_indel=simcov+RD.alignLen*Sub.dim();

        for( idx is=0; is<Sub.dim(); ++is ){
            idx ishift=is*RD.alignLen;

            for( idx il=0; il<RD.alignLen; ++il ){
                AllSimil[ishift+il]+=simil[ishift+il];
                AllSimcov[ishift+il]+=simcov[ishift+il];
            }
        }

        for( idx il=0; il<RD.alignLen; ++il ){
            for( idx ch=0; ch<6; ++ch ){
                AllAcgt_indel[il*6+ch]+=acgt_indel[il*6+ch];
            }
        }
    }


    PERF_END();
    PERF_PRINT();


    sStr bufPath, failedPath00;
    const char * dstPath = sQPrideProc::reqAddFile(bufPath, "RecombPolyplotSimilarity.csv");
    bufPath.add0();
    if ( dstPath ) {
        sFil str(dstPath);
        str.printf("position");
        for( idx is=0; is<Sub.dim(); ++is ){
            const char * id=Sub.id(is);
            if(*id=='>')++id;
            str.printf(",\"%s\"",id);
        }
        str.printf(",Most similar\n");
        idx max_simil = 0;
        idx most_simil_Sub = 0;
        for( idx il=0; il<RD.alignLen; ++il ){
            max_simil = 0;most_simil_Sub=0;
            str.printf("%" DEC,il+1);
            for( idx is=0; is<Sub.dim(); ++is ){
                idx ishift=is*RD.alignLen;

                if( AllSimil[ishift+il] > max_simil ) {
                    max_simil = AllSimil[ishift+il];
                    most_simil_Sub = is;
                }
                str.printf(",%" DEC,AllSimil[ishift+il]);
            }
            str.printf(",%s\n",Sub.id(most_simil_Sub) );
        }
    } else {
        failedPath00.printf("RecombPolyplotSimilarity.csv");failedPath00.add0();
    }


    dstPath = sQPrideProc::reqAddFile( bufPath, "RecombPolyplotCoverage.csv" );
    bufPath.add0();
    if( dstPath ) {
        sFil str(dstPath);
        str.printf("position");
        for( idx is=0; is<Sub.dim(); ++is ){
            const char * id=Sub.id(is);
            if(*id=='>')++id;
            str.printf(",\"%s\"",id);
        }
        str.printf(",Most covered\n");
        idx max_cover = 0;
        idx most_cover_Sub = 0;
        for( idx il=0; il<RD.alignLen; ++il ){
            max_cover = 0;most_cover_Sub=0;
            str.printf("%" DEC,il+1);
            for( idx is=0; is<Sub.dim(); ++is ){
                idx ishift=is*RD.alignLen;

                if( AllSimil[ishift+il] > max_cover ) {
                    max_cover = AllSimil[ishift+il];
                    most_cover_Sub = is;
                }
                str.printf(",%" DEC,AllSimcov[ishift+il]);
            }
            str.printf(",%s\n",Sub.id(most_cover_Sub) );
        }
    } else {
        failedPath00.printf("RecombPolyplotCoverage.csv");failedPath00.add0();
    }



    dstPath = sQPrideProc::reqAddFile(bufPath, "RecombSNPProfile.csv");
    bufPath.add0();
    if( dstPath ){
        sFil str(dstPath);
        str.printf("Position,Letter,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,Frequency A,Frequency C,Frequency G,Frequency T\n" );

        for( idx il=0; il<RD.alignLen; ++il ){
            idx consLet=0;
            for( idx ch=1; ch<4; ++ch ){
                if( AllAcgt_indel[il*6+consLet]<=AllAcgt_indel[il*6+ch] )
                    consLet=ch;
            }

            str.printf("%" DEC ",%c",il+1,(char)sBioseq::mapRevATGC[consLet]);

            idx tot=0;
            for( idx ch=0; ch<4; ++ch ){
                tot+=AllAcgt_indel[il*6+ch];
                str.printf(",%" DEC,AllAcgt_indel[il*6+ch]);
            }
            str.printf(",%" DEC ",%" DEC,AllAcgt_indel[il*6+4],AllAcgt_indel[il*6+5]);
            str.printf(",%" DEC,tot);

            for( idx ch=0; ch<4; ++ch ){
                real f=AllAcgt_indel[il*6+ch] ? ((real)(AllAcgt_indel[il*6+ch]))/tot : 0;
                if(ch==consLet)f=0;
                if(f)str.printf(",%.2lg",f);
                else str.printf(",0");
            }
            str.printf("\n");
        }

    } else {
        failedPath00.printf("RecombSNPProfile.csv");failedPath00.add0();
    }

    if( failedPath00.ptr() ){
        bufPath.cut(0);failedPath00.add0();
        for( const char * fn=failedPath00.ptr(); fn; fn=sString::next00(fn) ) {
            bufPath.printf(",%s",fn);
        }
        reqSetInfo(req, eQPInfoLevel_Error, "Failed to add %s", bufPath.ptr(1) );
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    reqSetStatus(req, eQPReqStatus_Done);
    reqProgress(cntFound, 100, 100);
    logOut(eQPLogType_Info,"Found %" DEC " hits\n",cntFound);

return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eATGC);

    sStr tmp;
    sApp::args(argc,argv);

    DnaRecomb backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-recomb",argv[0]));
    return (int)backend.run(argc,argv);
}


