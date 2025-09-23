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
#include <qpsvc/qpsvc-dna-hexagon.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <violin/hiveproc.hpp>


struct SNPRange{
    idx reqID;
    idx iSub;
    idx iRange;
    idx posMapped;
    idx ofsInFile;
    idx ofsInFileX;
    idx ofsInFileA;
    idx rStart;
    idx rEnd;
    idx posCovered;

    SNPRange(){
        sSet(this,0);
    }
};


class DnaHeptagon : public sHiveProc
{
    public:
        DnaHeptagon(const char * defline00,const char * srv) : sHiveProc(defline00,srv)
        {
        }
        virtual idx OnExecute(idx);

        static idx alphabeticalSortReferences(void * param, void * arr, idx i1, idx i2 );
        static idx partitionSNPRangeALO(void * param, void * i1, void * i2, sPart::sOperation op);
        static idx partitionAARangeALO(void * param, void * i1, void * i2, sPart::sOperation op);
        static idx AARangesorter(void * param, void * arr, idx i1, idx i2);
        idx launchCollector(sVec<SNPRange> &profileRegionsAll, sVec<idx> &profRegionsAll_alphabeticalSortInd);
        idx launchAnnotator(sDic<sBioseqSNP::AnnotAlRange> &aaRangesAll);
        idx addToRanges(sDic<sBioseqSNP::AnnotAlRange> &aaRanges, sIonWander * wander, sBioseqAlignment::Al * hdr, idx * m, idx iAl, idx iSub, const char * original_seqID);
};


struct RANGEStartIdx {
        idx iStart, iEnd;
};

idx DnaHeptagon::partitionSNPRangeALO(void * param, void * i1, void * i2, sPart::sOperation op)
{
    SNPRange * r1 = (SNPRange *)i1;
    SNPRange * r2 = (SNPRange *)i2;
    idx res = 0;
    switch (op) {
        case sPart::eMOV:
            *r1 = *r2;
            break;
        case sPart::eEQU:
            res = (r1->posMapped == r2->posMapped );
            break;
        case sPart::eLTN:
            res = (r1->posMapped < r2->posMapped);
            break;
        case sPart::eLEQ:
            res = !partitionSNPRangeALO(param,i1,i2,sPart::eGTN);
            break;
        case sPart::eGTN:
            res = partitionSNPRangeALO(param, i2, i1, sPart::eLTN);
            break;
        case sPart::eGEQ:
            res = !partitionSNPRangeALO(param, i1, i2, sPart::eLTN);
            break;
        case sPart::eADD:
            r1->posMapped += r2->posMapped;
            break;
        case sPart::eSUB:
            r1->posMapped -= r2->posMapped;
            break;
        default:
            res = 0;
            break;
    }
    return res;
}


idx DnaHeptagon::partitionAARangeALO(void * param, void * i1, void * i2, sPart::sOperation op)
{
    sBioseqSNP::AnnotAlRange * r1 = (sBioseqSNP::AnnotAlRange *)i1;
    sBioseqSNP::AnnotAlRange * r2 = (sBioseqSNP::AnnotAlRange *)i2;
    idx res = 0;
    switch (op) {
        case sPart::eMOV:
            *r1 = *r2;
            break;
        case sPart::eEQU:
            res = (r1->iAlDim() == r2->iAlDim() );
            break;
        case sPart::eLTN:
            res = (r1->iAlDim() < r2->iAlDim());
            break;
        case sPart::eLEQ:
            res = !partitionAARangeALO(param,i1,i2,sPart::eGTN);
            break;
        case sPart::eGTN:
            res = partitionAARangeALO(param, i2, i1, sPart::eLTN);
            break;
        case sPart::eGEQ:
            res = !partitionAARangeALO(param, i1, i2, sPart::eLTN);
            break;
        case sPart::eADD:
            r1->iAlEnd += r2->iAlDim();
            break;
        case sPart::eSUB:
            r1->iAlEnd -= r2->iAlDim();
            break;
        default:
            res = 0;
            break;
    }
    return res;
}

idx DnaHeptagon::AARangesorter(void * param, void * arr, idx i1, idx i2)
{
    sBioseqSNP::AnnotAlRange * aaArray = (sBioseqSNP::AnnotAlRange *)arr;
    sBioseqSNP::AnnotAlRange * r1 = aaArray + i1;
    sBioseqSNP::AnnotAlRange * r2 = aaArray + i2;
    if( r1->iSub < r2->iSub )
        return 1;
    else if( r1->iSub > r2->iSub )
        return -1;
    else
        return r2->Start - r1->Start;
}

idx DnaHeptagon::alphabeticalSortReferences (void * param, void * arr, idx i1, idx i2 ) {
    sHiveseq * subs = (sHiveseq*) param;
    SNPRange * r1 = &((SNPRange*)arr)[i1];
    SNPRange * r2 = &((SNPRange*)arr)[i2];
    idx res = strcmp(subs->id(r1->iSub),subs->id(r2->iSub)) ;
    if (!res) res = i1 - i2;
    return res;
}

idx DnaHeptagon::addToRanges(sDic<sBioseqSNP::AnnotAlRange> &aaRanges, sIonWander * wander, sBioseqAlignment::Al * hdr, idx * m, idx iAl, idx iSub, const char * original_seqID) {
    idx alStart = hdr->getSubjectStart(m), alEnd = hdr->getSubjectEnd_uncompressed(m);;
    idx lenStart = 0, lenEnd = 0 ;
    idx recSize = 4;
    char szStart[128], szEnd[128];
    sIPrintf(szStart, lenStart, alStart, 10);
    memcpy(szStart + lenStart, ":0", 3);
    lenStart += 2;
    memcpy(szEnd, "0:", 3);
    sIPrintf(szEnd + 2, lenEnd, alEnd, 10);
    lenEnd += 2;
    const char * seqID = strchr(original_seqID,' ');
    if (seqID) {
        wander->setSearchTemplateVariable("$seqID1", 7, original_seqID, seqID-original_seqID);
    } else {
        wander->setSearchTemplateVariable("$seqID1", 7, original_seqID, sLen(original_seqID));
    }

    wander->setSearchTemplateVariable("$start", 6, szStart, lenStart);
    wander->setSearchTemplateVariable("$end", 4, szEnd, lenEnd);

    wander->resetResultBuf();
    wander->traverse();
    idx recDim=wander->traverseBuf.length()/sizeof(idx)/recSize;
    idx *p=recDim ? (idx * )wander->traverseBuf.ptr(0) : 0 ;

    if(!p) {
        p = sBioseqSNP::tryAlternativeWay(wander, original_seqID, &recDim);
        recDim /= recSize;
    }
    for ( idx iRec = 0 ; iRec < recDim ; ++iRec ) {
        idx irec=*sConvInt2Ptr(p[1 + iRec*recSize],idx);
        sBioseqSNP::AnnotAlRange * rng = aaRanges.get((const void *)&irec, sizeof(idx));
        if( !rng ) {
            rng = aaRanges.set(&irec, sizeof(idx));
            idx posrange=*sConvInt2Ptr(p[3 + iRec*recSize],idx);
            rng->iSub = iSub;
            rng->End = ((posrange)&0xFFFFFFFF);
            rng->Start  = ((posrange)>>32);
            rng->iAlStart = iAl;
            rng->iRec = irec;
        }
        rng->iAlEnd = iAl;
    }
    return recDim;
}

idx DnaHeptagon::launchCollector(sVec<SNPRange> &profileRegionsAll, sVec<idx> &profRegionsAll_alphabeticalSortInd){

    reqSetData (masterId ? masterId : grpId, "profileRegions", profileRegionsAll.mex());
    reqSetData (masterId ? masterId : grpId, "profileRegionsAlphabeticallySortedIndex", profRegionsAll_alphabeticalSortInd.mex());

    idx cntPosMapped = 0, chunkSize = 1024*1024, prevSub = -1, cntSub = 0;
    for( idx is=0; is<profileRegionsAll.dim() ; ++is ){
        SNPRange * v=profileRegionsAll.ptr(is);
        if(!v) continue;
        cntPosMapped += v->rEnd-v->rStart;
        if( v->iSub != prevSub ) {
            prevSub = v->iSub;
            ++cntSub;
        }
    }
    idx chunkCnt = (cntPosMapped-1)/chunkSize + 1;
    if( chunkCnt > profileRegionsAll.dim() )
        chunkCnt = profileRegionsAll.dim();
    if( chunkCnt > 100 ) {
        chunkCnt = 100;
        chunkSize = (cntPosMapped - 1)/chunkCnt+1;
    }
    sVec<idx> ion_splitPos(sMex::fExactSize|sMex::fSetZero), hept_splitPos(sMex::fExactSize|sMex::fSetZero);
    ion_splitPos.add(chunkCnt);hept_splitPos.add(chunkCnt);

    if(chunkCnt>1){
        if(profileRegionsAll.dim()<=1000) {
            sPart::linearpartition(DnaHeptagon::partitionSNPRangeALO, 0, profileRegionsAll.ptr(),profileRegionsAll.dim(),chunkCnt,hept_splitPos.ptr(1));
            sPart::linearpartition(DnaHeptagon::partitionSNPRangeALO, 0, profileRegionsAll.ptr(),profileRegionsAll.dim(),chunkCnt,ion_splitPos.ptr(1),profRegionsAll_alphabeticalSortInd.ptr());
        } else {
            for(idx i = 0 ; i < chunkCnt ; ++i ) {
                hept_splitPos[i] = ion_splitPos[i] = i*(profileRegionsAll.dim()/chunkCnt);
            }
        }
    }

    reqSetData (masterId ? masterId : grpId, "ionChunkPos", ion_splitPos.mex());
    reqSetData (masterId ? masterId : grpId, "heptChunkPos", hept_splitPos.mex());

    sStr str;
    reqSetData(masterId,"formT.qpride",pForm);

    str.printf(0,"%s-collect",vars.value("serviceName") );
    Request r;requestGet(reqId, &r) ;
    idx newgrp=grpSubmit(str.ptr(),0,-r.priority,chunkCnt);
    sVec < idx > reqsnew; grp2Req(newgrp,&reqsnew, str.ptr());
    for(idx igg=0; igg<reqsnew.dim(); ++igg) {
        grpAssignReqID(reqsnew[igg], masterId ? masterId : grpId, igg+1);
        reqSetAction(reqsnew[igg],eQPReqAction_Run);
    }

    return chunkCnt;
}

idx DnaHeptagon::launchAnnotator(sDic<sBioseqSNP::AnnotAlRange> &aaRanges){
    sVec<sBioseqSNP::AnnotAlRange> aaRangesAll;aaRangesAll.add(aaRanges.dim());
    for(idx i = 0; i < aaRanges.dim(); ++i) {
        aaRangesAll[i] = *aaRanges.ptr(i);
    }
    aaRanges.empty();
    reqSetData (masterId ? masterId : grpId, "annotRegions", aaRangesAll.mex());

    sVec<idx> sind(sMex::fExactSize);
    sind.resizeM(aaRangesAll.dim());
    idx totAnnotatedAls=0;
    for(idx i = 0 ; i < aaRangesAll.dim() ; i++){
        sind[i]=i;
        totAnnotatedAls+=aaRangesAll.ptr(i)->iAlDim();
    }
    sSort::sortSimpleCallback(DnaHeptagon::AARangesorter, 0, aaRangesAll.dim(),aaRangesAll.ptr(), sind.ptr());

    idx chunkSize = 1024*1024;
    idx maxChunks = 100;

    idx chunkCnt = (totAnnotatedAls-1)/chunkSize + 1;

    if( chunkCnt > maxChunks ) {
        chunkCnt = maxChunks;
        chunkSize = (totAnnotatedAls - 1)/chunkCnt+1;
    }
    sVec<idx> annot_splitPos(sMex::fExactSize|sMex::fSetZero);
    annot_splitPos.add(chunkCnt);

    if(chunkCnt>1) {
        if(aaRangesAll.dim()<=1000) {
            sPart::linearpartition(DnaHeptagon::partitionAARangeALO, 0, aaRangesAll.ptr(),aaRangesAll.dim(),chunkCnt,annot_splitPos.ptr(1));
        } else {
            for(idx i = 0 ; i < chunkCnt ; ++i ) {
                annot_splitPos[i] = i*(aaRangesAll.dim()/chunkCnt);
            }
        }
    }


    reqSetData (masterId ? masterId : grpId, "annotChunkPos", annot_splitPos.mex());


    sStr str;
    reqSetData(masterId,"formT.qpride",pForm);

    str.printf(0,"%s-annotate",vars.value("serviceName") );
    Request r;requestGet(reqId, &r) ;
    idx newgrp=grpSubmit(str.ptr(),0,-r.priority,chunkCnt);
    sVec < idx > reqsnew; grp2Req(newgrp,&reqsnew, str.ptr());
    for(idx igg=0; igg<reqsnew.dim(); ++igg) {
        grpAssignReqID(reqsnew[igg], masterId ? masterId : grpId, igg+1);
        reqSetAction(reqsnew[igg],eQPReqAction_Run);
    }

    return chunkCnt;
}

idx DnaHeptagon::OnExecute(idx req)
{
    sStr errS;

    sBioseqSNP::SNPParams SP;
    SP.lenPercCutoff=formIValue("lenPercCutoff",0);
    SP.maxMissmatchPercCutoff=formIValue("maxMissmatchPercCutoff",0);
    SP.maxRptIns=formIValue("maxRptIns",3);
    SP.cutEnds=formIValue("cutEnds",0);
    SP.disbalanceFR=formIValue("disbalanceFR",3);
    SP.minCover=formIValue("minCover",50);
    SP.minFreqPercent=formRValue("minFreqPercent",0);
    SP.entrCutoff=formRValue("entrCutoff",1);

    SP.snpCompare=formIValue("snpCompare",0);
    SP.noiseProfileResolution=formRValue("noiseProfileResolution",0.0001);
    SP.noiseProfileMax=formRValue("noiseProfileMax",0.01);
    SP.maxLowQua=formIValue("maxLowQua",0);
    SP.useQuaFilter=formIValue("useQuaFilter",20);
    SP.minImportantEntropy=formRValue("minImportantEntropy",0.7);
    SP.rangeSize=formIValue("rangeSize",4*1024*1024);
    SP.filterZeros=formIValue("filterZeros",1);
    SP.minFreqIgnoreSNP=formIValue("minFreqIgnoreSNP",0);

    SP.countAAs=formBoolIValue("countAAs",1);
    SP.collapseRpts=formBoolValue("collapseRpts",false);

    bool extraInfo=false;
    if(formIValue("histograms",0) || SP.entrCutoff)
        extraInfo=true;


    FILE * fp=0, * fpX=0, * fpA=0;
    idx fpSize=0,fpXSize=0, fpASize=0;

    sBioal::ParamsAlignmentIterator PA;


    idx slice=sHiveseq::defaultSliceSizeI;
    const char * splitSize = getSplitSize(objs.ptr());
    if( splitSize )
        slice = atoidx(splitSize);


    sStr profName;
    profName.printf(0,"file://profile.vioprof");
    reqSetData(req,profName,0,0);
    sStr reqProfPath;reqDataPath(req,profName.ptr(7),&reqProfPath);
    sFile::remove(reqProfPath);

    sStr reqProfPathX;
    if(extraInfo) {
        profName.printf(0,"file://profile.vioprofX");
        reqSetData(req,profName,0,0);
        reqDataPath(req,profName.ptr(7),&reqProfPathX);
        sFile::remove(reqProfPathX);
    }

    sStr indelsPath;
    profName.printf(0,"file://indels.dict");
    reqSetData(req,profName,0,0);
    reqDataPath(req,profName.ptr(7),&indelsPath);
    sFile::remove(indelsPath);

    sStr reqAAPathX;
    if(SP.countAAs) {
        profName.printf(0,"file://aa.vioprofX");
        reqSetData(req,profName,0,0);
        reqDataPath(req,profName.ptr(7),&reqAAPathX);
        sFile::remove(reqAAPathX);
    }

    sVec < idx > subsetN;sString::scanRangeSet(formValue("subSet",0,""), 0, &subsetN, -1, 0, 0);
    sSort::sort(subsetN.dim(),subsetN.ptr());

    sVec<sHiveId> alignerIDs;
    sHiveId::parseRangeSet(alignerIDs, formValue("parent_proc_ids"));
    idx startSlice=0, sliceTot=0;

    sHiveId alignerID;
    for(idx ia=0; ia<alignerIDs.dim(); ia++) {
        alignerID = alignerIDs[ia];
        sUsrFile aligner(alignerID, user);
        sStr path;
        aligner.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __);
        sHiveal hiveal(user, path);
        sBioal * bioal = &hiveal;
        idx subSetDimAl = 0;
        if( subsetN.dim() ) {
            idx dimAl = 0;
            for(idx is = 0; is < subsetN.dim(); ++is) {
                bioal->listSubAlIndex(subsetN[is], &dimAl);
                subSetDimAl += dimAl;
            }
        } else {
            subSetDimAl = bioal->dimAl();
        }
        sliceTot += ceil((real) subSetDimAl / slice);
        if( sliceTot >= (reqSliceId + 1) ) {
            break;
        }
    }
    sUsrFile aligner(alignerID, user);

    sStr path;
    aligner.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __);
    sHiveal hiveal(user, path);
    sBioal * bioal = &hiveal;
    bioal=&hiveal;
    if( !bioal->isok() || bioal->dimAl()==0){
        errS.printf("No alignments are detected or the alignment file %s is missing or corrupted\n", alignerID.print());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }



    sStr subject;
    QPSvcDnaHexagon::getSubject00(aligner,subject);
    sHiveseq Sub(user, subject.ptr(), hiveal.getSubMode(), false, false, &errS);
    if(Sub.dim()==0) {
        reqSetInfo(req, eQPInfoLevel_Error, "Reference '%s' sequences are missing or corrupted%s%s", subject.length() ? subject.ptr() : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);

        return 0;
    }
    errS.cut0cut();

    idx numAlChunks=subsetN.dim() ? subsetN.dim() : Sub.dim() ;

    sStr str;
    idx cntSub=0;


    sStr query;
    QPSvcDnaHexagon::getQuery00(aligner,query);
    sHiveseq Qry(user, query, hiveal.getQryMode(), false, false, &errS);
    if(Qry.dim()==0) {
        reqSetInfo(req, eQPInfoLevel_Error, "Query '%s' sequences are missing or corrupted%s%s", query.length() ? query.ptr() : "unspecified", errS.length() ? ": " : "", errS.length() ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);

        return 0;
    }
    errS.cut0cut();

    bioal->Sub=&Sub;
    bioal->Qry=&Qry;


    idx totalAls=0,cntFound=0,dimAl;
    idx totMapped=0;


    bool doBreak=false;


    sIonWander * wander=0;
    sHiveIon hionAnnot(user);

    sDic < sBioseqSNP::AnnotAlRange > aaRanges;
    if(SP.countAAs) {
        const char * refAnnot=formValue("referenceAnnot");
        if(refAnnot) {
            hionAnnot.init(user,refAnnot,0,"ion");
            wander=hionAnnot.addIonWander("myion","seq=foreach($seqID1);a=find.annot(#range=possort-max,seq.1,$start,seq.1,$end);b=find.annot(seqID=a.seqID,record=a.record,id=\"CDS\");unique.1(b.pos);blob(b.record, b.pos);");
            aaRanges.init(reqAAPathX);
        } else {
            logOut(eQPLogType_Warning,"No annotation file detected. Amino acid calls will be calculated assuming each subject is also an ORF\n");
            reqSetInfo(req, eQPInfoLevel_Warning, "No annotation file detected. Amino acid calls will be calculated assuming each subject is also an ORF\n");
            SP.computeAAs = true;
        }
    }



    sVec < SNPRange > subRanges;
    sVec < RANGEStartIdx > rangeStartIndexes;


    progress100End=50;

    idx reportInterval = 1 ;
    idx reqAl = ((reqSliceId - startSlice + 1) * slice > bioal->dimAl()) ? (bioal->dimAl() - (reqSliceId - startSlice) * slice) : slice;
    idx curAl = 0 , reportIntervalSize = reportInterval * reqAl / 100 ;
    if(reportIntervalSize<=0)reportIntervalSize=1;

    sBioseqSNP::InDels my_indels;

    for( idx is=0; is<numAlChunks; ++is) {

        idx iSub= subsetN.dim() ? subsetN[is] : is;
        idx alListIndex = bioal->listSubAlIndex(iSub, &dimAl);
        if( !alListIndex ){
            continue;
        }
        idx sublen=Sub.len(iSub);

        idx start=(reqSliceId-startSlice)*slice, end=start+slice;
        start-=totalAls;end-=totalAls;
        if(start<0)start=0;
        if(end>dimAl)end=dimAl;
        if(start>=dimAl){totalAls+=dimAl;continue;}
        if(end<0)break;

        logOut(eQPLogType_Info,"Profiling %s from subject %" DEC "\n",reqProfPath.ptr(0),iSub );

        idx rangeCount = SP.rangeSize ? (sublen-1)/SP.rangeSize+1 : 1 ;
        rangeStartIndexes.resizeM( rangeCount );
        for(idx ir=0; ir<rangeCount; ++ir ){
            rangeStartIndexes[ir].iStart=sIdxMax;
            rangeStartIndexes[ir].iEnd=0;
        }
        if( rangeCount > 1 && SP.countAAs && SP.computeAAs ) {
            SP.countAAs = 0;
            logOut(eQPLogType_Warning,"Subject %" DEC " longer than %" DEC " limit. Amino acid calls will not be calculated\n",iSub, SP.rangeSize );
            reqSetInfo(req, eQPInfoLevel_Warning, "Subject %" DEC " longer than %" DEC " limit. Amino acid calls will not be calculated\n",iSub, SP.rangeSize);
        }
        for (idx ia=start; ia<end; ++ia) {
            idx iAl=alListIndex+ia-1;

            sBioseqAlignment::Al *  hdr=bioal->getAl(iAl);
            if(hdr->idSub()!=iSub || !hdr->lenAlign())continue;
            idx * m=bioal->getMatch(iAl);
            idx alStart=hdr->getSubjectStart(m);
            idx alEnd=hdr->getSubjectEnd(m);
            idx rangeStart=alStart/SP.rangeSize;
            idx rangeEnd=alEnd/SP.rangeSize;
            for(idx ir=rangeStart; ir<=rangeEnd && ir<rangeCount; ++ir ) {
                if(ia < rangeStartIndexes[ir].iStart )rangeStartIndexes[ir].iStart=ia;
                if(ia > rangeStartIndexes[ir].iEnd )rangeStartIndexes[ir].iEnd=ia;
            }
        }

        for( idx irange=0; irange<rangeCount ; ++irange){
            if(rangeStartIndexes[irange].iStart >  rangeStartIndexes[irange].iEnd)
                continue;

            SNPRange SR;
            SR.reqID=req;
            SR.iSub=iSub;
            SR.iRange=irange;
            SR.rStart=0+irange*SP.rangeSize;
            SR.rEnd=SR.rStart+SP.rangeSize;
            if(SR.rEnd>sublen)SR.rEnd=sublen;
            SR.posMapped=SR.posCovered=0;
            SR.ofsInFile=(totMapped)  ? fpSize :0;
            SR.ofsInFileX=(totMapped && extraInfo)  ? fpXSize :0;
            SR.ofsInFileA=(totMapped && SP.countAAs && SP.computeAAs)  ? fpASize :0;

            sVec < sBioseqSNP::SNPFreq > Freq(sMex::fSetZero|sMex::fAlignPage|sMex::fMaintainAlignment);
            Freq.add( SR.rEnd-SR.rStart);

            sVec < sBioseqSNP::ProfilePosInfo > Pinf(sMex::fSetZero|sMex::fAlignPage|sMex::fMaintainAlignment);
            if(extraInfo)Pinf.add( SR.rEnd-SR.rStart);

            sVec < sBioseqSNP::ProfileAAInfo > Ainf(sMex::fSetZero|sMex::fAlignPage|sMex::fMaintainAlignment);
            if(SP.countAAs && SP.computeAAs)Ainf.add((SR.rEnd-SR.rStart)/3+1);

            sBioseqSNP::ProfileExtraInfo PExIn;
            if(SP.entrCutoff)PExIn.EntroMap.add( SP.rangeSize);

            for ( idx ia= rangeStartIndexes[irange].iStart; ia<= rangeStartIndexes[irange].iEnd; ++ia) {

                idx iAl=alListIndex+ia-1;
                sBioseqAlignment::Al *  hdr=bioal->getAl(iAl);
                if(!hdr->lenAlign())continue;
                idx * m=bioal->getMatch(iAl);
                idx idQry=hdr->idQry();
                m = sBioseqAlignment::uncompressAlignment(hdr,m);

                const char * seqIDOriginal=Sub.id(iSub);

                idx nowFound=0;
                nowFound=sBioseqSNP::snpCountSingleSeq(Freq.ptr(0), Pinf.ptr(0), Ainf.ptr(0), SR.rStart, SR.rEnd-SR.rStart,Sub.seq(iSub), my_indels,Sub.len(iSub), bioal->Qry->seq(idQry),
                    bioal->Qry->len(idQry),bioal->Qry->qua(idQry), false, hdr, m, &SP, SP.collapseRpts?1:bioal->getRpt(iAl), &PExIn, 0, 0);

                if(nowFound){
                    if( !SP.computeAAs && SP.countAAs && nowFound && wander ) {
                        addToRanges(aaRanges, wander, hdr, m, iAl, iSub, seqIDOriginal);
                    }
                    SR.posMapped+=nowFound;
                    cntFound+=nowFound;
                }
                ++curAl;
                if( (curAl%1000)==0 && !reqProgress(cntFound, curAl, reqAl) ) {
                    doBreak = true;
                    break;
                }
            }
            if(doBreak)break;

            if(SP.entrCutoff && SR.posMapped)
                sBioseqSNP::snpCountPosInfo(Freq.ptr(0), Pinf.ptr(0), Sub.seq(iSub), SR.rStart, SR.rEnd-SR.rStart, &SP, &PExIn );

            if(!SR.posMapped){
                Freq.cut(0);
                Pinf.cut(0);
                Ainf.cut(0);
            }
            else {
                SR.posCovered = 0;
                for(idx i = 0 ; i < Freq.dim() ; ++i ) {
                    if(Freq.ptr(i)->coverage())
                        ++SR.posCovered;
                }
                *subRanges.add(1)=SR;
                if(!fp)fp=fopen(reqProfPath,"a");
                if(fp){
                    idx written= fwrite((char*)Freq.mex()->ptr(),1,Freq.mex()->pos(),fp);
                    fpSize+=written;
                    if(written!=Freq.mex()->pos() ) {
                        reqSetInfo(req, eQPInfoLevel_Error, "Insufficient resources to operate. Cannot write to results to file.");
                        reqSetStatus(req, eQPReqStatus_SysError);
                        fclose(fp);
                        return 1;
                    }

                }
                if(extraInfo){
                    if(!fpX)fpX=fopen(reqProfPathX,"a");
                    if(fpX){
                        idx written=fwrite((char*)Pinf.mex()->ptr(),1,Pinf.mex()->pos(),fpX);
                        fpXSize+=written;
                    }
                }

                if(SP.countAAs && SP.computeAAs){
                    if(!fpA)fpA=fopen(reqAAPathX,"a");
                    if(fpA){
                        idx written=fwrite((char*)Ainf.mex()->ptr(),1,Ainf.mex()->pos(),fpA);
                        fpASize+=written;
                    }
                }

                totMapped+=SR.posMapped;

            }
            Freq.destroy();
            Pinf.destroy();
            Ainf.destroy();

            logOut(eQPLogType_Info,"\t %" DEC "/%" DEC ": mapping alignments %" DEC "-%" DEC " to range %" DEC "-%" DEC " = %" DEC " bases at offset %" DEC " \n",irange+1,rangeCount,rangeStartIndexes[irange].iStart, rangeStartIndexes[irange].iEnd, SR.rStart, SR.rEnd, SR.posMapped , SR.ofsInFile);
        }

        totalAls+=dimAl;
    }

    if(fp)fclose(fp);
    if(fpX)fclose(fpX);
    if(fpA)fclose(fpA);

    reqSetData(req,"subRanges",subRanges.mex());
    subRanges.destroy();
    rangeStartIndexes.destroy();
    {
        sFil my_indels_buf(indelsPath);
        if( !my_indels_buf.ok() ) {
            logOut(eQPLogType_Error, "Cannot create indels mapping file '%s'", indelsPath.ptr());
            return 0;
        }
        my_indels.serialOut(my_indels_buf);
    }

    if( doBreak ) {
        return 0;
    }
    if( !isLastInMasterGroupWithLock() ) {
        reqSetProgress(req, cntFound, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }
    reqSetStatus(req, eQPReqStatus_Running);

    logOut(eQPLogType_Info,"\n\nCoalescing results\n");

    progress100Start=50;
    progress100End=100;
    reqSetProgress(req, cntFound, progress100Start );

    sDic < SNPRange > profileRegions;
    sVec < SNPRange > profileRegionsAll;
    idx tmpRange_subject=-1,tmpRange_irange=-1;

    sStr tt;
    sVec < idx > reqList; grp2Req(masterId, &reqList, vars.value("serviceName"));
    sVec <sBioseqSNP::SNPFreq> * dst=0 ;
    sVec <sBioseqSNP::ProfilePosInfo > * dstX=0 ;
    sVec <sBioseqSNP::ProfileAAInfo > * dstA=0 ;

    idx totSubRanges=0,curOverSubRange=0;
    for( idx ir=0; ir<reqList.dim() ; ++ir ){
        subRanges.cut(0);
        reqGetData(reqList[ir],"subRanges",subRanges.mex());
        totSubRanges+=subRanges.dim();
        for(idx is = 0; is < subRanges.dim() && SP.countAAs; ++is) {
            if( subRanges[is].iRange > 1 ) {
                SP.countAAs = false;
            }
        }
    }
    sBioseqSNP::InDels all_indels;
    sDic<sBioseqSNP::AnnotAlRange> aaRangesAll;
    for( idx ir=0; ir<reqList.dim() ; ++ir ){
        sStr req_indels_path;
        reqDataPath(reqList[ir],"indels.dict",&req_indels_path);
        sFil req_indels_buf(req_indels_path,sMex::fReadonly);
        if( !req_indels_buf.ok() ) {
            logOut(eQPLogType_Error, "Cannot read indels mapping file '%s'", req_indels_buf.ptr());
            return 0;
        }
        sBioseqSNP::InDels req_indels;
        req_indels.serialIn(req_indels_buf.ptr(),req_indels_buf.length());
        all_indels.merge(req_indels);

        subRanges.cut(0);
        reqGetData(reqList[ir],"subRanges",subRanges.mex());
        if( !SP.computeAAs && SP.countAAs ) {
            sStr r2;reqDataPath(reqList[ir],"aa.vioprofX",&r2);
            sDic <sBioseqSNP::AnnotAlRange> crAARanges;
            crAARanges.init(r2.ptr());
            for (idx ik = 0; ik < crAARanges.dim() ; ++ik ) {
                void * key = crAARanges.id(ik);
                sBioseqSNP::AnnotAlRange * crng = crAARanges.ptr(ik);
                sBioseqSNP::AnnotAlRange * rng = aaRangesAll.get(key, sizeof(idx));
                if( !rng ) {
                    rng = aaRangesAll.set(key,sizeof(idx));
                    *rng = *crng ;
                } else {
                    if( rng->iAlStart > crng->iAlStart ) {
                        rng->iAlStart = crng->iAlStart;
                    }
                    if( rng->iAlEnd < crng->iAlEnd ) {
                        rng->iAlEnd = crng->iAlEnd;
                    }
                }
            }

        }
        if( formIValue("countAAs", 1) != SP.countAAs ) {
            sStr r2X;
            reqDataPath(reqList[ir], "aa.vioprofX", &r2X);
            sFile::remove(r2X.ptr(0));
        }
        for( idx is=0; is<subRanges.dim() ; ++is ){
            if( !reqProgress(ir, curOverSubRange++, totSubRanges) ) {
                doBreak = true;
                break;
            }
            tt.printf(0,"%" DEC "-%" DEC,subRanges[is].iSub,subRanges[is].iRange);

            SNPRange * v=profileRegions.get(tt.ptr());
            if(!v){
                v=profileRegions.set(tt.ptr());if(!v)continue;
                *v=subRanges[is];
                logOut(eQPLogType_Info,"Registering subject %" DEC " range #%" DEC "\n",v->iSub,v->iRange);
                logOut(eQPLogType_Info,"\t range %" DEC "-%" DEC " mapping %" DEC " bases from req %" DEC " file offset %" DEC "\n",v->rStart,v->rEnd,v->posMapped,subRanges[is].reqID,v->ofsInFile);

                continue;
            }
            if(v->iSub!=tmpRange_subject || v->iRange!=tmpRange_irange){
                if(dst){
                    delete (dst);
                    dst=0;
                }
                if(dstX){
                    delete (dstX);
                    dstX=0;
                }
                if(dstA){
                    delete (dstA);
                    dstA=0;
                }
            }

            logOut(eQPLogType_Info,"\t+range %" DEC "-%" DEC " mapping %" DEC " bases from req %" DEC " file offset %" DEC "\n",subRanges[is].rStart,subRanges[is].rEnd,subRanges[is].posMapped,subRanges[is].reqID,subRanges[is].ofsInFile);


            sStr r1;reqDataPath(v->reqID,"profile.vioprof",&r1);
            if(!dst) {
                dst= new sVec <sBioseqSNP::SNPFreq>(sMex::fAlignPage, r1.ptr(0),v->ofsInFile,v->rEnd-v->rStart);
                if(dst) {
                    tmpRange_subject=v->iSub;
                    tmpRange_irange=v->iRange;
                }else {
                    reqSetInfo(req, eQPInfoLevel_Error, "Insufficient resources. Cannot allocate space");
                    reqSetStatus(req, eQPReqStatus_SysError);
                    return 1;
                }


            }
            sStr r2;reqDataPath(subRanges[is].reqID,"profile.vioprof",&r2);
            sVec <sBioseqSNP::SNPFreq> src(sMex::fAlignPage, r2.ptr(0),subRanges[is].ofsInFile,v->rEnd-v->rStart);
            if(!src.ok()){
                reqSetInfo(req, eQPInfoLevel_Error, "Insufficient resources. Cannot open intermediate file");
                reqSetStatus(req, eQPReqStatus_SysError);
                return 1;
            }

            logOut(eQPLogType_Info,"\tappending %s->%s\n",r2.ptr(0),r1.ptr(0));
            idx len=v->rEnd-v->rStart;
            idx srclen=sFile::size(r2.ptr());
            if(len>srclen)len=srclen;
            for ( idx ipos=0 ; ipos< len ; ++ipos){
                sBioseqSNP::SNPFreq  * lineDst=dst->ptr(ipos);
                sBioseqSNP::SNPFreq  * lineSrc=src.ptr(ipos);
                if(!lineSrc->coverage())
                    continue;
                if(!lineDst->coverage())
                    ++v->posCovered;
                lineDst->add(lineSrc);
                sSet(lineSrc,0,sizeof(sBioseqSNP::SNPFreq));
            }


            if(extraInfo){
                sStr r1X;reqDataPath(v->reqID,"profile.vioprofX",&r1X);
                if(!dstX) {
                    dstX=new sVec <sBioseqSNP::ProfilePosInfo>(sMex::fAlignPage, r1X.ptr(0),v->ofsInFileX,v->rEnd-v->rStart);
                }
                sStr r2X;reqDataPath(subRanges[is].reqID,"profile.vioprofX",&r2X);
                sVec <sBioseqSNP::ProfilePosInfo> src(sMex::fAlignPage, r2X.ptr(0),subRanges[is].ofsInFileX,v->rEnd-v->rStart);
                logOut(eQPLogType_Info,"\tappending %s->%s\n",r2X.ptr(0),r1X.ptr(0));
                idx len=v->rEnd-v->rStart;
                idx srclen=sFile::size(r2X.ptr());
                if(len>srclen)len=srclen;
                for ( idx ipos=0 ; ipos< len ; ++ipos){
                    sBioseqSNP::ProfilePosInfo  * lineDst=dstX->ptr(ipos);
                    sBioseqSNP::ProfilePosInfo  * lineSrc=src.ptr(ipos);
                    if(!lineSrc->coverage())
                        continue;
                    lineDst->sum(lineSrc);
                    sSet(lineSrc,0,sizeof(sBioseqSNP::ProfilePosInfo));
                }

            }

            if(SP.countAAs && SP.computeAAs){
                sStr r1X;reqDataPath(v->reqID,"aa.vioprofX",&r1X);
                if(!dstA) {
                    dstA=new sVec <sBioseqSNP::ProfileAAInfo >(sMex::fAlignPage, r1X.ptr(0),v->ofsInFileA,v->rEnd-v->rStart);
                }
                sStr r2X;reqDataPath(subRanges[is].reqID,"aa.vioprofX",&r2X);
                sVec <sBioseqSNP::ProfileAAInfo> src(sMex::fAlignPage, r2X.ptr(0),subRanges[is].ofsInFileA,v->rEnd-v->rStart);
                logOut(eQPLogType_Info,"\tappending %s->%s\n",r2X.ptr(0),r1X.ptr(0));
                idx len=v->rEnd-v->rStart;
                idx srclen=sFile::size(r2X.ptr());
                if(len>srclen)len=srclen;
                for ( idx ipos=0 ; ipos< 1+len/3 ; ++ipos){
                    sBioseqSNP::ProfileAAInfo  * lineDst=dstA->ptr(ipos);
                    sBioseqSNP::ProfileAAInfo  * lineSrc=src.ptr(ipos);
                    if(!lineSrc->coverage())
                        continue;
                    lineDst->sum(lineSrc);
                    sSet(lineSrc,0,sizeof(sBioseqSNP::ProfileAAInfo));
                }
            }


            v->posMapped+=subRanges[is].posMapped;

        }
        if(dst){
            delete (dst);
            dst=0;
        }
        if(dstX){
            delete (dstX);
            dstX=0;
        }
        if(dstA){
            delete (dstA);
            dstA=0;
        }
        if( !reqProgress(ir, ir, reqList.dim()) ) {
            doBreak = true;
            break;
        }
    }
    if(dst)delete dst;
    if(dstX)delete dstX;
    if(dstA)delete dstA;

    sStr all_indels_path;
    sHiveProc::destPath(&all_indels_path,"all_indels.dict");
    sFile::remove(all_indels_path);
    sFil all_indels_buf(all_indels_path);
    if( !all_indels_buf.ok() ) {
        logOut(eQPLogType_Error, "Cannot create indels mapping file '%s'", all_indels_path.ptr());
        return 0;
    }
    all_indels.serialOut(all_indels_buf);

    for( idx is=0; is<numAlChunks; ++is) {

        idx iSub= subsetN.dim() ? subsetN[is] : is;
        idx sublen=Sub.len(iSub);
        idx rangeCount = SP.rangeSize ? (sublen-1)/SP.rangeSize+1 : 1 ;
        bool cntedSub = false;
        for( idx irange=0; irange<rangeCount ; ++irange){
            tt.printf(0,"%" DEC "-%" DEC,iSub,irange);
            SNPRange * v=profileRegions.get(tt.ptr());
            if(v){
                *profileRegionsAll.add()=*v;
                if(!cntedSub){
                    cntedSub = true;
                    ++cntSub;
                }
            }
        }
    }

    if( aaRangesAll.dim() ) {
        if( !launchAnnotator(aaRangesAll) ) {
            reqSetInfo(req, eQPInfoLevel_Warning, "No jobs for the annotator stage");
            logOut(eQPLogType_Warning,"Failed to launch annotator stage");
            reqSetStatus(req, eQPReqStatus_Done);
        }
    }
    if( profileRegionsAll.dim() ) {

        sVec<idx>profRegionsAll_alphabeticalSortInd(sMex::fSetZero|sMex::fExactSize);
        profRegionsAll_alphabeticalSortInd.add(profileRegionsAll.dim());
        sSort::sortSimpleCallback( (sSort::sCallbackSorterSimple)alphabeticalSortReferences , (void*)&Sub , profileRegionsAll.dim(), profileRegionsAll.ptr(), profRegionsAll_alphabeticalSortInd.ptr() );

        if( !launchCollector(profileRegionsAll, profRegionsAll_alphabeticalSortInd) ) {
            reqSetInfo(req, eQPInfoLevel_Warning, "No valid jobs for the collector stage");
            logOut(eQPLogType_Warning,"Failed to launch collector stage");
            reqSetStatus(req, eQPReqStatus_Done);
        }

    } else {
        reqSetInfo(req, eQPInfoLevel_Warning, "No valid positions collected");
    }

    if( !doBreak ) {
        reqSetProgress(req, cntFound, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    }
    return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv);

    DnaHeptagon backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-heptagon",argv[0]));
    return (int)backend.run(argc,argv);
}
