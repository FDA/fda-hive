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
#include <ulib/ufile.hpp>
#include <violin/violin.hpp>


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
};



class DnaHeptagonCollect : public sQPrideProc
{
    public:
        DnaHeptagonCollect(const char * defline00,const char * srv) : sQPrideProc(defline00,srv)
        {
            fileNames00 = "Noise" _ "FreqProfile" _ "HistProfile" _
                        "SNPprofile" _ "AAprofile" _ "ProfileInfo" _ "SNPthumb" _ "ThumbInfo" __;
            fileSuffix00 = "" _ "MergeL" _ "MergeR" _ "MergeF" __;
            filePaths = 0;
            fileList = 0;
        }
        virtual idx OnExecute(idx);

    private:
        sDic<sFil> * fileList;
        sDic<sStr> * filePaths;

        bool getChunkRanges(idx &heptagonStart, idx &heptagonEnd, idx &ionStart, idx &ionEnd, idx * t_reqSliceId = 0);

        enum eMergeFileNames {eNoise,eFreqProfile,eHistProfile,eMergeFileNameMax,
            eSNPprofile = eMergeFileNameMax,eAAprofile,eProfileInfo,eSNPthumb,eThumbInfo,eConcatFileNameMax, eFileNameMax = eConcatFileNameMax};
        const char * fileNames00;

        enum eMergeStates {eNoMerge=0x01,eLeftMerge=0x02,eMaxConcat=eLeftMerge,eRightMerge=0x04,eFullMerge=0x08, eLastMerge};
        const char * fileSuffix00;

        const char * getFileName(sStr &name, idx v_enum, idx mergestate = eNoMerge);
        const char * getFilePath(idx v_enum, idx mergeState, idx * req , bool cleanold) {
            sStr t;
            getFileName(t,v_enum,mergeState);
            return getFilePath(t.ptr(), req, cleanold);
        };
        const char * getFilePath(const char * name, idx * req = 0, bool cleanold=true);
        sFil * ensureFile( idx v_enum, idx mergeState = eNoMerge, idx * req = 0, idx flgs = 0, bool cleanold = true );
        idx getCurSubMergeState(idx iSub, idx prevSub=-1, idx nextSub=-1) {
            if(iSub==prevSub){
                if(iSub==nextSub)
                    return eFullMerge;
                return eLeftMerge;
            } else if (iSub==nextSub){
                return eRightMerge;
            } else {
                return eNoMerge;
            }
        }
        idx getMergeStates(idx start, idx end, sVec<SNPRange> &profileRegionsAll );
        idx getMergeStates(sVec<SNPRange> &profileRegionsAll, idx * pReqsliceId = 0);
        bool generateNoiseFilters(sBioseqSNP::SNPParams & SP, sHiveId &noiseFilterBase, idx noiseFilterBaseThreshold, idx curSub);
};

struct RANGEStartIdx {
        idx iStart, iEnd;
};

const char * DnaHeptagonCollect::getFilePath(const char * name, idx * req, bool cleanold) {
    if(!filePaths) return 0;
    idx t_req = reqId;
    if(req)
       t_req = *req;

    sStr tt("%" DEC "%s",t_req,name);
    sStr * t = filePaths->get(tt.ptr());
    if(!t) {
        t = filePaths->set(tt.ptr());
        sStr tt1("file://%s",name);
        if(cleanold)
            reqSetData(t_req,tt1,0,0);
        reqDataPath( t_req, name, t);
    }
    return t->ptr();
}

const char * DnaHeptagonCollect::getFileName(sStr &name, idx v_enum, idx mergestate) {
    name.cut(0);
    name.printf("%s",sString::next00(fileNames00,v_enum));
    if( mergestate > eNoMerge) {
        idx mergeShift = 0;
        while(mergestate>>=1) mergeShift++;
        name.printf("%s",sString::next00(fileSuffix00,mergeShift));
    }
    return name.ptr();
}

sFil * DnaHeptagonCollect::ensureFile(idx v_enum, idx mergeState, idx * p_req, idx flgs, bool cleanold ) {
    if(!fileList) return 0;
    sStr name, id_Name;
    if(!getFileName(name,v_enum,mergeState))
        return 0;
    cleanold &= !(flgs&sMex::fReadonly);
    idx req = p_req?*p_req:reqId;
    id_Name.printf("%" DEC "%s",req,name.ptr());
    sFil * fl = fileList->get(id_Name);
    if( !fl ) {
        fl = fileList->set(id_Name);
        name.printf(0,"%s",getFilePath(name,&req, cleanold));
        if( cleanold && sFile::exists(name) )sFile::remove(name);
        fl->init( name, flgs);
    }
    return fl;
}

idx DnaHeptagonCollect::getMergeStates(sVec<SNPRange> &profileRegionsAll, idx * pReqSliceId) {
    idx start=0, end=0, istart = 0, iend = 0;
    getChunkRanges(start,end, istart,iend, pReqSliceId);
    if(!end)end=profileRegionsAll.dim();
    return getMergeStates(start, end, profileRegionsAll);
}
idx DnaHeptagonCollect::getMergeStates(idx start, idx end, sVec<SNPRange> &profileRegionsAll) {
    idx prevSub = start? profileRegionsAll[start-1].iSub :-1;
    idx nextSub = end<profileRegionsAll.dim()-1? profileRegionsAll[end].iSub :-1;
    idx states = 0;
    if( prevSub == profileRegionsAll[start].iSub ) {
        states |= eLeftMerge;
    }
    if( nextSub == profileRegionsAll[end-1].iSub ) {
        states |= eRightMerge;
    }
    if( nextSub==prevSub && profileRegionsAll[end-1].iSub==prevSub ) {
        states = eFullMerge;
    }
    if(!(states&eFullMerge)) {
        idx is = start;
        SNPRange * v =0;
        while(is<end) {
            v = profileRegionsAll.ptr(is++);
            if(v->iSub!=prevSub && v->iSub!=nextSub) {
                states|=eNoMerge;
                break;
            }
        }
    }
    return states;
}

bool DnaHeptagonCollect::getChunkRanges(idx &heptagonStart, idx &heptagonEnd, idx &ionStart, idx &ionEnd, idx * t_reqSliceId)
{
    sVec<idx> hept_Chunks, ion_Chunks;
    reqGetData(grpId,"heptChunkPos",hept_Chunks.mex());
    reqGetData(grpId,"ionChunkPos",ion_Chunks.mex());

    if( ion_Chunks.dim() < 1 || hept_Chunks.dim() < 1 )
        return 0;

    if(!t_reqSliceId)t_reqSliceId = &reqSliceId;
    idx l_t_reqSliceId = *t_reqSliceId;
    if( l_t_reqSliceId < 0 || ion_Chunks.dim() <= l_t_reqSliceId || hept_Chunks.dim() <= l_t_reqSliceId )
        return 0;

    heptagonStart = heptagonEnd = ionStart = ionEnd = 0;

    heptagonStart = hept_Chunks[l_t_reqSliceId];
    ionStart = ion_Chunks[l_t_reqSliceId];

    if( l_t_reqSliceId < hept_Chunks.dim()-1 ) {
        heptagonEnd = hept_Chunks[l_t_reqSliceId+1];
    } else {
        heptagonEnd = 0;
    }
    if( l_t_reqSliceId < ion_Chunks.dim() - 1 ) {
        ionEnd= ion_Chunks[l_t_reqSliceId+1];
    } else {
        ionEnd = 0;
    }
    return ionStart + ionEnd + heptagonStart + heptagonEnd + hept_Chunks.dim() + ion_Chunks.dim();
}

bool DnaHeptagonCollect::generateNoiseFilters(sBioseqSNP::SNPParams & SP, sHiveId &noiseFilterBase, idx noiseFilterBaseThreshold, idx curSub)
{
    const char * scanNoiseFrom=0;
    for(idx f=0;f<6;++f){for(idx to=0;to<6;++to)SP.noiseCutoffs[f][to]=(f==to)?0:0.00;}
    sFil fl;
    if( noiseFilterBase ) {
        sStr pathNoiser;

        sUsrFile noiser(noiseFilterBase, user);
        real noiseFilterResolution = noiser.propGetR("noiseProfileResolution");
        if(!noiseFilterResolution) noiseFilterResolution = SP.noiseProfileResolution;
        noiser.getFilePathname(pathNoiser, "%s.csv",sString::next00(fileNames00,eNoise));
        fl.init(pathNoiser, sMex::fReadonly);
        if( fl.length() ) {
            sDic<sVec<real> > noiseThreshold;
            const char * buf = fl.ptr(), * bufend = fl.ptr()+ fl.length();
            buf = sBioseqSNP::binarySearchReferenceNoise( buf, bufend, curSub+1, false );
            bufend = sBioseqSNP::binarySearchReferenceNoise( buf, bufend, curSub+1,true );
            sBioseqSNP::integralFromProfileCSV( buf, bufend, noiseThreshold, noiseFilterResolution, &sBioseqSNP::noiseCutoffThresholds[noiseFilterBaseThreshold], 1);
            if( noiseThreshold.dim() ){
                for(idx ie = 0 ; ie < noiseThreshold.ptr(0)->dim() ; ++ie ) {
                    SP.noiseCutoffs[ie/4][ie%4] = *noiseThreshold.ptr(0)->ptr(ie);
                }
                return true;
            }
        } else {
            sTbl tbl;
            noiser.getFilePathname(pathNoiser, "%s-%" DEC ".csv",sString::next00(fileNames00,eNoise),curSub+1);
            fl.init(pathNoiser, sMex::fReadonly);
            if( fl.length() ) {
                tbl.parse(fl.ptr(), fl.length());
                scanNoiseFrom = tbl.cell(noiseFilterBaseThreshold + 1, 1);
            }
            if(scanNoiseFrom){
                #define a (SP.noiseCutoffs)
                sscanf(scanNoiseFrom,"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                        &a[0][1],&a[0][2],&a[0][3],
                        &a[1][0],&a[1][2],&a[1][3],
                        &a[2][0],&a[2][1],&a[2][3],
                        &a[3][0],&a[3][1],&a[3][2]);
                #undef a
                return true;
            }
        }
    }
    return false;
}
idx DnaHeptagonCollect::OnExecute(idx req)
{

    sStr errS;

    sBioseqSNP::SNPParams SP;
    SP.lenPercCutoff=formIValue("lenPercCutoff",0);
    SP.maxRptIns=formIValue("maxRptIns",3);
    SP.cutEnds=formIValue("cutEnds",0);
    SP.disbalanceFR=formIValue("disbalanceFR",3);
    SP.minCover=formIValue("minCover",50);
    SP.minFreqPercent=formRValue("minFreqPercent",0);
    SP.entrCutoff=formRValue("entrCutoff",1);
    SP.snpCompare=formIValue("snpCompare",0);
    SP.noiseProfileResolution=formRValue("noiseProfileResolution",sBioseqSNP::noiseProfileResolution);
    SP.freqProfileResolution=formRValue("freqProfileResolution",sBioseqSNP::freqProfileResolution);
    SP.histProfileResolution=formRValue("histProfileResolution",sBioseqSNP::histCoverResolution);
    SP.noiseProfileMax=formRValue("noiseProfileMax",0.01);
    SP.maxLowQua=formIValue("maxLowQua",0);
    SP.useQuaFilter=formIValue("useQuaFilter",20);
    SP.minImportantEntropy=formRValue("minImportantEntropy",0.7);
    SP.filterZeros=formIValue("filterZeros",1);
    SP.minFreqIgnoreSNP=formIValue("minFreqIgnoreSNP",0);
    SP.directionalityInfo=formBoolValue("directionalityInfo",false);
    SP.supportedDeletions=formBoolValue("supportedDeletions",true);
    SP.countAAs=formBoolIValue("countAAs",1);
    SP.collapseRpts=formBoolValue("collapseRpts",false);
    const char * refAnnot=formValue("referenceAnnot");
    bool computeAAs = (SP.countAAs && !refAnnot);

    bool generateIon = formBoolValue("generateIon",false);

    sStr noiseCutoff( formValue("noiseFilterParams") );
    sHiveId noiseFilterBase(formValue("noiseFilterBase"));
    idx noiseFilterBaseThreshold=formIValue("noiseFilterBaseThreshold",sBioseqSNP::noiseCutoffThresholdsIdx_DEFAULT);

    bool extraInfo=false;
    if(formIValue("histograms",0) || SP.entrCutoff)
        extraInfo=true;

    for(idx f=0;f<6;++f){for(idx to=0;to<6;++to)SP.noiseCutoffs[f][to]=(f==to)?0:0.00;}


    sDic<sFil> l_fileList; fileList = &l_fileList;
    sDic<sStr> l_filePaths; filePaths = &l_filePaths;


    sHiveId alignerID(formValue("parent_proc_ids"));
    sUsrFile aligner(alignerID,sQPride::user);



    sStr subject;
    aligner.propGet00("subject", &subject, ";");

    sStr parentAlignmentPath;
    aligner.getFilePathname00(parentAlignmentPath, "alignment.hiveal" _ "alignment.vioal" __);

    sHiveal hiveal(user);

    sHiveseq Sub(sQPride::user, subject.ptr(), hiveal.parseSubMode(parentAlignmentPath));Sub.reindex();
    if(Sub.dim()==0) {
        errS.printf("Reference '%s' sequences are missing or corrupted\n", subject.length() ? subject.ptr() : "unspecified");
        logOut(eQPLogType_Error,"%s",errS.ptr());
        reqSetInfo(req, eQPInfoLevel_Error, "%s",errS.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }


    sVec < SNPRange > profileRegionsAll;
    reqGetData(grpId,"profileRegions",profileRegionsAll.mex());

    sVec<idx> profRegionsAll_alphabeticalSortInd;
    reqGetData(grpId,"profileRegionsAlphabeticallySortedIndex",profRegionsAll_alphabeticalSortInd.mex());
    if( profileRegionsAll.dim() != profRegionsAll_alphabeticalSortInd.dim() || !profileRegionsAll || !profRegionsAll_alphabeticalSortInd ) {
        logOut(eQPLogType_Error,"Internal Error. Failed to find profileRegions file\n" );
        reqSetInfo(req, eQPInfoLevel_Error, "Internal Error. Failed to find intermediate  summary file\n");
        return 0;
    }
    sVec < idx > subsetN;sString::scanRangeSet(formValue("subSet",0,""), 0, &subsetN, -1, 0, 0);
    sSort::sort(subsetN.dim(),subsetN.ptr());


    sBioseqSNP::SNPminmax minmax, * p_minmax = 0;
    minmax.num_of_points = formIValue("thumbSize",1000);

    idx heptagonStart, heptagonEnd, ionStart, ionEnd ;

    sDic < sVec < idx > > noiseProfile;
    sDic < sVec < idx > > freqProfile;
    sVec < sBioseqSNP::HistogHistog > histogramCoverage;

    if( !getChunkRanges(heptagonStart, heptagonEnd, ionStart, ionEnd) ) {
        logOut(eQPLogType_Warning,"Empty chunk");
        reqSetInfo(req, eQPInfoLevel_Warning, "%s","Empty chunk");
        reqSetProgress(req, 0, 100);
        reqSetStatus(req, eQPReqStatus_Done);
        return 0;
    }
    if( !heptagonEnd ) {
        heptagonEnd = profileRegionsAll.dim();
    }
    if( !ionEnd) {
        ionEnd = profRegionsAll_alphabeticalSortInd.dim();
    }

    idx iHeptRanges = heptagonEnd - heptagonStart,
        iIonRanges = ionEnd - ionStart;
    idx iSubPrev = -1, curSub = -1,
        prevSub = heptagonStart? profileRegionsAll[heptagonStart-1].iSub :-10,
        nextSub = heptagonEnd<profileRegionsAll.dim()? profileRegionsAll[heptagonEnd].iSub :-10;

    idx iHaveDoneThat=0;

    bool doBreak = false;
    {


        logOut(eQPLogType_Debug,"\n\nComputing\n");

        sVec <sBioseqSNP::SNPFreq> dst;
        if( generateIon ) {
            sStr dstIonPath;
            const char * ionflpath = sQPrideProc::reqAddFile(dstIonPath,"ion#%" DEC "#",reqSliceId);
            if( ionflpath ) {
                sIonAnnot iannot(ionflpath,sMex::fMapRemoveFile);
                idx expected_hashSize = 0;
                for( idx is=ionStart; is<ionEnd ; ++is ) expected_hashSize += profileRegionsAll.ptr(profRegionsAll_alphabeticalSortInd[is])->posCovered;
                expected_hashSize*=8;
                if( expected_hashSize ) {
                    idx lg_expected_hashSize = 0;
                    while( expected_hashSize ) {
                        expected_hashSize = expected_hashSize>>1;
                        ++lg_expected_hashSize;
                    }

                    iannot.expect("annot",lg_expected_hashSize);

                    for( idx is=ionStart; is<ionEnd ; ++is ){

                        if( !reqProgress(iHaveDoneThat, iHaveDoneThat, iHeptRanges+iIonRanges) ) {
                            doBreak = true;
                            break;
                        }
                        ++iHaveDoneThat;

                        SNPRange * ion_v=profileRegionsAll.ptr(profRegionsAll_alphabeticalSortInd[is]);
                        if(!ion_v) continue;
                        curSub = ion_v->iSub;
                        const char * subID=Sub.id(curSub);
                        iannot.indexArr[0]=iannot.addRecord(sIonAnnot::eSeqID,sLen(subID),subID );

                        sStr r1;reqDataPath(ion_v->reqID,"profile.vioprof",&r1);
                        sVec <sBioseqSNP::SNPFreq> dstIn(sMex::fAlignPage, r1.ptr(0),ion_v->ofsInFile,ion_v->rEnd-ion_v->rStart), dst;
                        if(dstIn.dim()) {
                            dst.cut(0);sBioseqSNP::SNPFreq * f=dst.addM(dstIn.dim());
                            memcpy(f,dstIn.ptr(0),dstIn.dim()*sizeof(sBioseqSNP::SNPFreq));

                            logOut(eQPLogType_Debug,"ionizing # %" DEC " out of %" DEC "\n",is+1,profileRegionsAll.dim());
                            logOut(eQPLogType_Debug,"\t range %" DEC "-%" DEC " mapping %" DEC " bases from req %" DEC " file offset %" DEC "\n",ion_v->rStart,ion_v->rEnd,ion_v->posMapped,ion_v->reqID,ion_v->ofsInFile);
                            const char * ion_seq=Sub.seq(curSub);
                            if(iSubPrev!=curSub){
                                generateNoiseFilters(SP,noiseFilterBase,noiseFilterBaseThreshold,curSub);
                            }
                            sBioseqSNP::snpCleanTable( dst.ptr(), ion_seq, ion_v->rStart, dst.dim(), &SP);
                            sBioseqSNP::snpOutTable_version2( ion_seq, dst.ptr(0), ion_v->rStart, ion_v->rEnd-ion_v->rStart, &SP , &iannot);
                            iSubPrev=curSub;
                        }
                    }
                }

            } else {
                logOut(eQPLogType_Warning,"Failed to add ion file for %s for request %" DEC, objs[0].IdStr(), reqSliceId );
                reqSetInfo(req, eQPInfoLevel_Warning, "Failed to add ion file for request %" DEC, reqSliceId);
            }
        } else {
            iIonRanges = 0;
        }

        iSubPrev = -1;
        sFil * fl = 0;
        idx subMergeState = 0;

        for( idx is=heptagonStart; is<heptagonEnd; ++is ){

            if( !reqProgress(iHaveDoneThat, iHaveDoneThat, iHeptRanges+iIonRanges) ) {
                doBreak = true;
                break;
            }
            ++iHaveDoneThat;

            SNPRange * v=profileRegionsAll.ptr(is);
            if(!v) continue;
            curSub = v->iSub;

            sStr r1;reqDataPath(v->reqID,"profile.vioprof",&r1);
            sVec <sBioseqSNP::SNPFreq> dstIn(sMex::fReadonly|sMex::fAlignPage, r1.ptr(0),v->ofsInFile,v->rEnd-v->rStart);
            dst.cut(0);sBioseqSNP::SNPFreq * f=dst.addM(dstIn.dim());
            memcpy(f,dstIn.ptr(0),dstIn.dim()*sizeof(sBioseqSNP::SNPFreq));

            sStr r1X;reqDataPath(v->reqID,"profile.vioprofX",&r1X);
            sVec <sBioseqSNP::ProfilePosInfo> dstX(sMex::fReadonly|sMex::fAlignPage, r1X.ptr(0),v->ofsInFileX,v->rEnd-v->rStart);
            sVec <sBioseqSNP::ProfileAAInfo> vdstA, * dstA = 0;
            if(computeAAs) {
                sStr r1A;reqDataPath(v->reqID,"aa.vioprofX",&r1A);
                vdstA.init(sMex::fReadonly|sMex::fAlignPage, r1A.ptr(0),v->ofsInFileA,v->rEnd-v->rStart);
                dstA = &vdstA;
            }


            if(!dst.dim())continue;


            if(iSubPrev!=curSub){
                generateNoiseFilters(SP,noiseFilterBase,noiseFilterBaseThreshold,curSub);
            }

            logOut(eQPLogType_Debug,"Analyzing subject %" DEC " range #%" DEC "\n",curSub,v->iRange);
            logOut(eQPLogType_Debug,"\t range %" DEC "-%" DEC " mapping %" DEC " bases from req %" DEC " file offset %" DEC "\n",v->rStart,v->rEnd,v->posMapped,v->reqID,v->ofsInFile);

            const char * seq=Sub.seq(curSub);
            sBioseqSNP::snpCleanTable( dst.ptr(), seq, v->rStart, dst.dim(), &SP);


            if(iSubPrev!=curSub){
                subMergeState = getCurSubMergeState(iSubPrev,prevSub,nextSub);
                if(noiseProfile.dim()) {
                    fl = ensureFile(eNoise,subMergeState);
                    sBioseqSNP::snpOutNoise(fl,0, SP.noiseProfileResolution, &noiseProfile,0,0,iSubPrev+1,false);
                    noiseProfile.empty();
                }

                if(freqProfile.dim()){
                    fl = ensureFile(eFreqProfile,subMergeState);
                    sBioseqSNP::snpOutNoise(fl, 0, SP.freqProfileResolution, &freqProfile,0,0,iSubPrev+1,false);
                    freqProfile.empty();
                }

                if (extraInfo) {
                    if(histogramCoverage.dim()){
                        fl = ensureFile(eHistProfile,subMergeState);
                        sBioseqSNP::snpOutHistog(histogramCoverage, SP.histProfileResolution, *fl,iSubPrev+1,false);
                        histogramCoverage.cut(0);
                    }
                }

                ensureFile(eSNPprofile);

                p_minmax = minmax.initBucket( Sub.len(curSub), v->rStart );

                if(dstA && dstA->dim())
                    ensureFile(eAAprofile);

                if( p_minmax ) {
                    p_minmax->SP=&SP;
                    p_minmax->subseq = seq;
                    p_minmax->isub = curSub+1;
                    ensureFile(eSNPthumb);

                    if( dstX.dim() ) {
                        ensureFile(eProfileInfo);
                        ensureFile(eThumbInfo);
                        p_minmax->outInfo =  ensureFile(eThumbInfo);
                    }
                    p_minmax->out = ensureFile(eSNPthumb);
                }

                iSubPrev=curSub;
            }

            sBioseqSNP::snpCountNoise(dst.ptr(),seq, v->rStart, v->rEnd-v->rStart,  &noiseProfile, SP.noiseProfileResolution, SP.noiseProfileMax);
            sBioseqSNP::snpCountNoise(dst.ptr(),seq, v->rStart, v->rEnd-v->rStart,  &freqProfile, SP.freqProfileResolution, 1. , extraInfo ? &histogramCoverage : 0, SP.minCover);

            sBioseqSNP::snpOutTable( ensureFile(eSNPprofile), dstX.ptr(0)?ensureFile(eProfileInfo):0,(dstA && dstA->ptr(0))?ensureFile(eAAprofile):0,(sStr *) 0, curSub+1 , seq, dst.ptr(0),dstX.ptr(0),(dstA && dstA->ptr(0))?dstA->ptr(0):0, v->rStart, v->rEnd-v->rStart, &SP , 0, p_minmax,0);

        }
    }
    idx subMergeState = getCurSubMergeState(curSub,prevSub,nextSub);
    if(noiseProfile.dim())
        sBioseqSNP::snpOutNoise(ensureFile(eNoise,subMergeState),0, SP.noiseProfileResolution, &noiseProfile,0,0,curSub+1,false);
    if(freqProfile.dim())
        sBioseqSNP::snpOutNoise(ensureFile(eFreqProfile,subMergeState),0, SP.freqProfileResolution, &freqProfile,0,0,curSub+1,false);
    if( extraInfo && histogramCoverage.dim() )
        sBioseqSNP::snpOutHistog(histogramCoverage, SP.histProfileResolution, *ensureFile(eHistProfile,subMergeState),curSub+1,false);


    for ( idx i=0; i<fileList->dim(); ++i) {
        sFil * ff = fileList->ptr(i);
        if(ff) {
            if(!ff->ptr() || !ff->ok()) {
                sFile::remove(filePaths->ptr(i)->ptr());
            }
            fileList->ptr(i)->destroy();
        }
    }

    fileList->empty();
    filePaths->empty();

    if( !isLastInMasterGroupWithLock() ) {
        if( !doBreak ) {
            reqSetProgress(req, iHaveDoneThat, 100);
            reqSetStatus(req, eQPReqStatus_Done);
        }
        return 0;
    }
    reqSetStatus(req, eQPReqStatus_Running);


    progress100Start=50;
    progress100End=100;

    sStr fnames[eFileNameMax];
    for(idx fi = 0; fi < eFileNameMax; ++fi) {
        fnames[fi].cut(0);
        if(!computeAAs && fi==eAAprofile) continue;
        sQPrideProc::reqAddFile( (fnames[fi]), "%s.csv",sString::next00(fileNames00,fi));
        sFil ffl(fnames[fi].ptr());
        sFil * c_file = &ffl;
        if( !c_file->ok() ) continue;
        switch (fi){
            case eNoise:
            case eFreqProfile:{
                c_file->addString("Reference,Frequency");
                for ( idx ins=0; ins<4 ; ++ins) {
                    for ( idx ine=0; ine<4 ; ++ine) {
                        if(ins==ine) continue;
                        char noiseType[32];sprintf(noiseType,"%c-%c",(char)sBioseq::mapRevATGC[ins],(char)sBioseq::mapRevATGC[ine]);
                        c_file->printf(",%s",noiseType);
                    }
                }
                c_file->addString("\n");
                break;
            }
            case eSNPthumb:
            case eSNPprofile: {
                c_file->addString("Reference,Position,Letter,Consensus,Count-A,Count-C,Count-G,Count-T,Count Insertions,Count Deletions,Count Total,Count Forward,Count Reverse,Quality,Entropy,SNP-Entropy,Frequency A,Frequency C,Frequency G,Frequency T" );
                if(SP.entrCutoff)c_file->addString(",Entropy Total,Entropy SNP,Entropy Forward,Entropy Reverse,Entropy A,Entropy C,Entropy G,Entropy T");
                if(SP.directionalityInfo)c_file->addString(",Quality Forward,Quality Reverse,Count Forward A,Count Forward C,Count Forward G,Count Forward T,Count Forward Insertions,Count Forward Deletions");
                c_file->addString("\n");
                break;
            }
            case eThumbInfo:
            case eProfileInfo: {
                c_file->addString("Reference,Position,Letter,Score,Length of Sequences,Length of Alignments,Left Tail,Right Tail,Max Left Tail,Max Right Tail,Length Anisotropy,Quality Forward A,Quality Forward C,Quality Forward G,Quality Forward T,Quality Reverse A,Quality Reverse C,Quality Reverse G,Quality Reverse T");
                c_file->addString("\n");
                break;
            }
            case eAAprofile: {
                c_file->addString("Reference,NUCPOS,AAPOS,TCOV,AAREF,AASUB,VCOV,AAFREQ");
                c_file->addString("\n");
                break;
            }
            case eHistProfile: {
                c_file->addString("Reference,SNPfrequency,letter,histogram");
                c_file->addString("\n");
                break;
            }
        }
    }

    sVec<idx> reqs;
    grp2Req(masterId, &reqs, vars.value("serviceName"), masterId);

    noiseProfile.empty();freqProfile.empty();histogramCoverage.cut(0);

    idx fileTypeHasData = 0, mergeStates = 0, iSub = 0, tSub = 0, l_reqId = req;
    sStr t_fileName;
    sFil * fl;
    for ( idx ic = 0 ; ic < reqs.dim() ; ++ic ) {
        l_reqId = reqs[ic];
        mergeStates = getMergeStates(profileRegionsAll,&ic);
        for(idx fi = eMergeFileNameMax ; fi < eFileNameMax ; ++fi ) {
            if(!computeAAs && fi==eAAprofile) continue;
            const char * cpflsrc = getFilePath(fi,eNoMerge,&l_reqId, false);
            if( cpflsrc && sFile::copy(cpflsrc ,fnames[fi],true,false)) {
                fileTypeHasData |= (udx)1<<fi;
            }
        }
        if(mergeStates&eLeftMerge) {
            fl = ensureFile(eNoise,eLeftMerge,&l_reqId,0,false);
            if( fl && fl->ptr() && fl->ok() ) {
                tSub = sBioseqSNP::noiseProfileFromCSV(fl->ptr(), 0, &noiseProfile,SP.noiseProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->cut(0);
            }
            if(noiseProfile.dim() && iSub > 0) {
                sBioseqSNP::snpOutNoise(fl,0, SP.noiseProfileResolution, &noiseProfile,0,0,iSub,false);
                fl->destroy();
                const char * cpflsrc = getFilePath(eNoise,eLeftMerge,&l_reqId, false);
                if( cpflsrc && sFile::copy(cpflsrc,fnames[eNoise],true,false)) {
                    fileTypeHasData |= (udx)1<<eNoise;
                }
            }

            fl = ensureFile(eFreqProfile,eLeftMerge,&l_reqId,0,false);
            if( fl && fl->ptr() && fl->ok() ) {
                tSub = sBioseqSNP::noiseProfileFromCSV(fl->ptr(), 0, &freqProfile, SP.freqProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->cut(0);
            }
            if(freqProfile.dim() && iSub > 0) {
                sBioseqSNP::snpOutNoise(fl,0, SP.freqProfileResolution, &freqProfile,0,0,iSub,false);
                fl->destroy();
                const char * cpflsrc = getFilePath(eFreqProfile,eLeftMerge,&l_reqId, false);
                if( cpflsrc && sFile::copy(cpflsrc,fnames[eFreqProfile],true,false)) {
                    fileTypeHasData |= (udx)1<<eFreqProfile;
                }
            }
            fl = ensureFile(eHistProfile,eLeftMerge,&l_reqId,0,false);
            if( fl && fl->ptr() && fl->ok() ) {
                tSub = sBioseqSNP::histogramProfileFromCSV(fl->ptr(), 0, &histogramCoverage, SP.histProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->cut(0);
            }
            if(histogramCoverage.dim() && iSub > 0) {
                sBioseqSNP::snpOutHistog(histogramCoverage, SP.histProfileResolution, *fl,iSub,false);
                fl->destroy();
                const char * cpflsrc = getFilePath(eHistProfile,eLeftMerge,&l_reqId, false);
                if( cpflsrc && sFile::copy(cpflsrc,fnames[eHistProfile],true,false)) {
                    fileTypeHasData |= (udx)1<<eHistProfile;
                }
            }
            noiseProfile.empty();
            freqProfile.empty();
            histogramCoverage.cut(0);
            iSub = 0;
        }
        if(mergeStates&eNoMerge) {
            noiseProfile.empty();
            freqProfile.empty();
            histogramCoverage.cut(0);
            for(idx fi = eNoise ; fi < eMergeFileNameMax ; ++fi ){
                const char * cpflsrc = getFilePath(fi,eNoMerge,&l_reqId,false);
                if( cpflsrc && sFile::copy(cpflsrc,fnames[fi],true,false)){
                    fileTypeHasData |= (udx)1<<fi;
                }
            }
            iSub = 0;
        }
        if(mergeStates&eRightMerge) {
            noiseProfile.empty();
            freqProfile.empty();
            histogramCoverage.cut(0);
            iSub = 0;
            fl = ensureFile(eNoise,eRightMerge,&l_reqId,sMex::fReadonly);
            if( fl && fl->ok() ){
                tSub = sBioseqSNP::noiseProfileFromCSV(fl->ptr(), 0, &noiseProfile,SP.noiseProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->destroy();
            }
            fl = ensureFile(eFreqProfile,eRightMerge,&l_reqId,sMex::fReadonly);
            if( fl && fl->ok() ){
                tSub = sBioseqSNP::noiseProfileFromCSV(fl->ptr(), 0, &freqProfile,SP.freqProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->destroy();
            }
            fl = ensureFile(eHistProfile,eRightMerge,&l_reqId,sMex::fReadonly);
            if( fl && fl->ok() ){
                tSub = sBioseqSNP::histogramProfileFromCSV(fl->ptr(), 0, &histogramCoverage, SP.histProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->destroy();
            }
        }
        if(mergeStates&eFullMerge) {
            fl = ensureFile(eNoise,eFullMerge,&l_reqId,sMex::fReadonly);
            if( fl && fl->ptr() && fl->ok() ){
                tSub = sBioseqSNP::noiseProfileFromCSV(fl->ptr(), 0, &noiseProfile,SP.noiseProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->destroy();
            }
            fl = ensureFile(eFreqProfile,eFullMerge,&l_reqId,sMex::fReadonly);
            if( fl && fl->ptr() && fl->ok() ){
                tSub = sBioseqSNP::noiseProfileFromCSV(fl->ptr(), 0, &freqProfile,SP.freqProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->destroy();
            }
            fl = ensureFile(eHistProfile,eFullMerge,&l_reqId,sMex::fReadonly);
            if( fl && fl->ptr() && fl->ok() ){
                tSub = sBioseqSNP::histogramProfileFromCSV(fl->ptr(), 0, &histogramCoverage, SP.histProfileResolution);
                if( tSub > 0 ) iSub = tSub;
                fl->destroy();
            }
        }

        if( !reqProgress(ic, ic, reqs.dim()) ) {
            doBreak = true;
            break;
        }
    }
    for(idx fi = 0 ; fi < eFileNameMax ; ++fi ) {
        if(!computeAAs && fi==eAAprofile) continue;
        if( !(fileTypeHasData&((udx)1<<fi)) ) {
            sFile::remove(fnames[fi]);
        }
#ifndef _DEBUG
        for ( idx ic = 0 ; ic < reqs.dim() ; ++ic ) {
            l_reqId = reqs[ic];
            const char * dltNoMergeflnm =  getFilePath(fi,eNoMerge,&l_reqId, false);
            if(dltNoMergeflnm)sFile::remove(dltNoMergeflnm);
            if(fi<eMergeFileNameMax)
            {
                const char * dltLeftflm = getFilePath(fi,eLeftMerge,&l_reqId, false);
                if(dltLeftflm)sFile::remove(dltLeftflm);
                const char * dltRightflm = getFilePath(fi,eRightMerge,&l_reqId, false);
                if(dltRightflm)sFile::remove(dltRightflm);
                const char * dltFullflm = getFilePath(fi,eFullMerge,&l_reqId, false);
                if(dltFullflm)sFile::remove(dltFullflm);
            }
        }
#endif
    }

    if( generateIon ) {
        sDir fileList00;
        objs[0].files(fileList00, sFlag(sDir::bitFiles)|sFlag(sDir::bitNoExtension), "ion#*#.ion");
        if(fileList00.length()) {
            sStr path;reqAddFile(path,"ion");
            sFil fp(path);
            if( !fp.ok() ) {
                logOut(eQPLogType_Warning,"Failed to add ion file for %s", objs[0].IdStr() );
                reqSetInfo(req, eQPInfoLevel_Warning, "Failed to add ion file");
            }
            fp.add("list",4);
            for(const char * fn=fileList00.ptr(); fn; fn=sString::next00(fn)){
                fp.printf("\n%s",fn);
            }
        }
    }



#ifndef _DEBUG
    idx profGrpId=req2Grp(profileRegionsAll.ptr(0)->reqID);
    sVec<idx> profReqIds;
    grp2Req(profGrpId,&profReqIds);
    for(idx pi=0; pi<profReqIds.dim();++pi){
        idx profRedID=profReqIds[pi];
        sStr r1;reqDataPath(profRedID,"profile.vioprof",&r1);
        if(sFile::exists(r1.ptr()) && !sFile::remove(r1.ptr())) {
            logOut(eQPLogType_Warning,"Failed to delete \"profile.vioprof\" file of request:%" DEC "\n",profRedID);
        }
        sStr r1X;reqDataPath(profRedID,"profile.vioprofX",&r1X);
        if(sFile::exists(r1X.ptr()) && !sFile::remove(r1X.ptr())) {
            logOut(eQPLogType_Warning,"Failed to delete \"profile.vioprofX\" file of request:%" DEC "\n",profRedID);
        }
    }
#endif
    reqSetProgress(req, profileRegionsAll.dim(), 100);
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv);

    DnaHeptagonCollect backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-heptagon-collect",argv[0]));
    return (int)backend.run(argc,argv);
}




