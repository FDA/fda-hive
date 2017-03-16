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


extern sPerf bioPerf;
#define QIDX(_v_iq, _v_len)    ((flags&sBioseqAlignment::fAlignBackward) ? ((_v_len)-1-(_v_iq)) : (_v_iq))

class DnaRecomb:public sQPrideProc
{
    public:
        DnaRecomb(const char * defline00,const char * srv):sQPrideProc(defline00,srv) {}

        static idx similCumulator(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum);

        idx parseMultipleAlignments(sHiveId &mutualAlignmentID, sHiveal & mutAl);

        struct RecombData {
            idx * simil,* simcov;
            idx * acgt_indel; // 6x * length of the total reference frame
            sVec <idx> uncompressMM;
            sBioal * mutAl;
            idx startAl,endAl,req; //needed to report progress
            void * obj; //needed to report progress
            idx alignLen;
        };
        virtual idx OnExecute(idx);
};

idx DnaRecomb::parseMultipleAlignments(sHiveId &mutualAlignmentID, sHiveal & mutAl)
{
    sUsrFile mutualAlignmentObj(mutualAlignmentID, sQPride::user);
    progress100Count=1;
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
        reqSetData(reqId, "file://alignment-slice.vioal", 0, 0); // create and empty file for this request
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

        vioAltAAA.DigestCombineAlignmentsRaw(dstAlignmentsT,pathT, 1000000, false, flagSet, mutualAlMode );
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

    //idx idSub=hdr->idSub();
    //idx idQry=hdr->idQry();


    SPData->uncompressMM.resize(hdr->lenAlign()*2);
    sBioseqAlignment::uncompressAlignment(hdr,m,SPData->uncompressMM.ptr());
    m=SPData->uncompressMM.ptr();

    // remap to the alignment mutual alignment of subjects
    idx iSub = SPData->mutAl->Qry->getmode()? SPData->mutAl->Qry->short2long(hdr->idSub()) : hdr->idSub();
    sBioseqAlignment::Al * hdrTo= SPData->mutAl->getAl(iSub);
    idx * mTo=SPData->mutAl->getMatch(iSub);

    sBioseqAlignment::Al hdrToWrite=*hdr; // because *hdr might be readonly
    hdr=&hdrToWrite;
    sBioseqAlignment::remapAlignment(&hdrToWrite, hdrTo, m, mTo ) ;
    const char * qrybits=bioal->Qry->seq(hdr->idQry());
    char ch=0;
    idx qrybuflen=bioal->Qry->len(hdr->idQry());

    // accumulate the similarity map
    idx ishift=iSub*SPData->alignLen; // hdrTo->idQry is the sequential number of the subject on the map it is aligned to
    idx lastSubOK=0;
    idx flags=hdr->flags();
    //for (idx ipp=hdr->subStart+m[0]; ipp<hdr->subEnd; ++ipp){
    for (idx ipp=0; ipp< hdr->lenAlign() ; ipp+=2){
        idx is=hdr->subStart()+m[ipp];
        idx iq=hdr->qryStart()+m[ipp+1];
        idx iqx=-1;
        if(is>=0 && iq>=0)
            iqx=QIDX( hdr->qryStart()+iq, qrybuflen );

        if(is<0){ ch=4; is=lastSubOK;}  // we remember inserts on the last mapped position
        else if(iq<0)ch=5; // deletions
        else {
            ch=sBioseqAlignment::_seqBits(qrybits, iqx , flags ) ;  // mapped ok
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

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ initialize the parameters
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sBioal::ParamsAlignmentIterator PA;
    RecombData RD;
    sVec < idx > SimilStack(sMex::fSetZero);
    PA.userPointer=(void*)&RD;

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ prepare the Alignment files
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sHiveId alignerID(formValue("parent_proc_ids"));
    sUsrObj aligner(*sQPride::user, alignerID);

    sStr subject;
    aligner.propGet00("subject", &subject, ";");
    sStr query;
    aligner.propGet00("query", &query, ";");

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

        return 0; // error
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
        return 0; // error
    }

    sHiveal mutAl(user, mutualAlignmentPath);
    sBioseq::EBioMode mutualAlMode = sBioseq::eBioModeLong;
    sHiveseq Sub_mut(sQPride::user, subject ,mutualAlMode);//Sub.reindex();
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
        mutual_uncompressMM.resize(hdr->lenAlign()*2);
        sBioseqAlignment::uncompressAlignment(hdr,m,mutual_uncompressMM.ptr());
        m=mutual_uncompressMM.ptr();
        hdr = RD.mutAl->getAl(iMutAl);
        is = hdr->subStart() + m[2*(hdr->lenAlign()-1)];
        if ( RD.alignLen < is) {
            RD.alignLen = is;
        }
    }

//    sFil fal(mutualAlignmentPath, sMex::fReadonly);
//    sVec < idx > alSub;
//    if(fal.length())sBioseqAlignment::readMultipleAlignment(&alSub,fal.ptr(),fal.length(),sBioseqAlignment::eAlRelativeToMultiple, &RD.alignLen,false);
//    if(alSub.dim()==0) {
//        errS.printf("Multiple alignments are missing or corrupted\n");
//        logOut(eQPLogType_Error,"%s",errS.ptr());
//        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
//        reqSetStatus(req, eQPReqStatus_ProgError);
//        return 0; // error
//    }
//    sBioseqAlignment::Al * hdr, * hdrs=(sBioseqAlignment::Al *)(alSub.ptr(0)), * hdre=sShift(hdrs,alSub.dim()*sizeof(idx));
//    idx iN;
//    for ( hdr=hdrs, iN=0; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()) , ++iN)
//        RD.subAlList.vadd(1,hdr);


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ load the subject and query sequences
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sHiveseq Sub(sQPride::user, subject.ptr());Sub.reindex();
    if(Sub.dim()==0) {
        errS.printf("Reference '%s' sequences are missing or corrupted\n", subject.length() ? subject.ptr() : "unspecified");
        logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0; // error
    }
    RD.mutAl->Qry = &Sub;

    // load the subject and query sequences

    sHiveseq Qry(sQPride::user, query.ptr());Qry.reindex();
    if(Qry.dim()==0) {
        errS.printf("Query '%s' sequences are missing or corrupted\n",query.ptr() ? query.ptr() : "unspecified");
        logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0; // error
    }



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ iterate through alignments
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    bioal->Sub=&Sub;
    bioal->Qry=&Qry;

    sVec < idx > subsetN;sString::scanRangeSet(formValue("subSet",0,""), 0, &subsetN, -1, 0, 0);
    idx numAlChunks=subsetN.dim() ? subsetN.dim() : Sub.dim() ;
    idx totalAls=0,cntFound=0,dimAl;
    idx slice=formIValue("slice",sHiveseq::defaultSliceSizeI);

    sStr recombName,recombPath;;
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
            reqSetData(req,recombName,0,0); // create and empty file for this request
            reqDataPath(req,recombName.ptr(7),&recombPath);
            sFile::remove(recombPath);
            SimilStack.init(recombPath,sMex::fSetZero|sMex::fExactSize);
            RD.simil=SimilStack.add(RD.alignLen*Sub.dim()*2+RD.alignLen*6); // for scores and for coverages and 6 for acgts and indels
            RD.simcov=RD.simil+RD.alignLen*Sub.dim();
            RD.acgt_indel=RD.simcov+RD.alignLen*Sub.dim();

        }

//        logOut(eQPLogType_Error,"Computing recombinants %s from \n\tsubject %" DEC " %" DEC " alignments from %" DEC " to %" DEC " \n",recombPath.ptr(0),iSub, end-start, start, end );
        cntFound+=bioal->iterateAlignments(0, start, end-start, iSub, (sBioal::typeCallbackIteratorFunction)&similCumulator, (sBioal::ParamsAlignmentIterator *) &PA);

        totalAls+=dimAl;
    }

    SimilStack.destroy();

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Analyze results
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    logOut(eQPLogType_Info,"Analyzing results\n");
    reqProgress(cntFound, 100);

    if ( !isLastInMasterGroup() ) {
        reqSetStatus(req, eQPReqStatus_Done);// change the status
        logOut(eQPLogType_Info,"Done processing %" DEC " request for execution\n",req);
        return 0;
    }

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Accumulate the blobs from all
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    PERF_START("Output");
    sVec < idx > reqList; grp2Req(grpId, &reqList, vars.value("serviceName"));

    sVec < idx > AllSimilVec(sMex::fExactSize|sMex::fSetZero);
    AllSimilVec.addM(RD.alignLen*Sub.dim()*2+RD.alignLen*6);
    //SimilStack.init(dstRecombPath,sMex::fSetZero|sMex::fExactSize);
    idx * AllSimil=AllSimilVec.ptr(); // for scores and for coverages
    idx * AllSimcov=AllSimil+RD.alignLen*Sub.dim();
    idx * AllAcgt_indel=AllSimcov+RD.alignLen*Sub.dim();

    for( idx ir=0; ir<reqList.dim(); ++ir ) {
        recombName.printf(0,"recomb-slice-%" DEC ".vioprof",ir);
        recombPath.cut(0);
        reqDataPath(reqList[ir],recombName.ptr(0),&recombPath);
        sVec <idx > stack(sMex::fReadonly|sMex::fExactSize,recombPath);
        idx * simil=stack.ptr(); // for scores and for coverages
        idx * simcov=simil+RD.alignLen*Sub.dim();
        idx * acgt_indel=simcov+RD.alignLen*Sub.dim();

        for( idx is=0; is<Sub.dim(); ++is ){
            idx ishift=is*RD.alignLen; // hdrTo->idQry is the sequential number of the subject on the map it is aligned to

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


    // output the coverage
    sStr bufPath, failedPath00;
    const char * dstPath = sQPrideProc::reqAddFile(bufPath, "RecombPolyplotSimilarity.csv");
    bufPath.add0();
    if ( dstPath ) {
        sFil str(reqAddFile);
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
                idx ishift=is*RD.alignLen; // hdrTo->idQry is the sequential number of the subject on the map it is aligned to

                if( AllSimil[ishift+il] > max_simil ) {
                    max_simil = AllSimil[ishift+il];
                    most_simil_Sub = is;
                }
                str.printf(",%" DEC,AllSimil[ishift+il]);
                //AllSimcov[ishift+il]+=simcov[ishift+il];
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
                idx ishift=is*RD.alignLen; // hdrTo->idQry is the sequential number of the subject on the map it is aligned to

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



    // output the coverage
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
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Finita La comedia
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    reqSetStatus(req, eQPReqStatus_Done);// change the status
    reqProgress(cntFound, 100);
    logOut(eQPLogType_Info,"Found %" DEC " hits\n",cntFound);

return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eATGC);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaRecomb backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-recomb",argv[0]));
    return (int)backend.run(argc,argv);
}


