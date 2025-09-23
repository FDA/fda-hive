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
#include <violin/violin.hpp>
#include <ssci/bio/bioseqsnp.hpp>



class algoAnnotMapperProc: public sQPrideProc
{
    public:
        algoAnnotMapperProc(const char * defline00, const char * srv) : sQPrideProc(defline00, srv) {};
        virtual idx OnExecute(idx);

        struct AnnotationOpenInformationCacheItem {
            sFil * fil;
            sUsrObj * pann;
            bool isProfiler;
            idx myAnnotIndex;
            sDic < sVioAnnot > * myVioAnnotList;
            AnnotationOpenInformationCacheItem(){
                fil=0;
                pann=0;
                myAnnotIndex=-1;
                myVioAnnotList=0;
            }
            ~AnnotationOpenInformationCacheItem()
            {
                if(pann)delete pann;
                if(fil)delete fil;
            }
            inline sVioAnnot * myAnnotPtr()
            {
                return likely(myAnnotIndex >= 0) ? myVioAnnotList->ptr(myAnnotIndex) : 0;
            }
            bool setMyAnnotPtr(sDic < sVioAnnot > & vioAnnotList, const char * annotId)
            {
                myVioAnnotList = &vioAnnotList;
                return myVioAnnotList->setString(annotId, 0, &myAnnotIndex);
            }
        };

};

idx algoAnnotMapperProc::OnExecute(idx req)
{

#ifdef _DEBUG
    printf("Form:\n");
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        printf("%s = %s\n", key, value);
    }
#endif

    real freqThreshold = formRValue("profilerFreqThreshold",0);
    real coverageThreshold = formRValue("profilerCoverageThreshold",0);
    idx cntMaxOut=formIValue("cntMaxOut",sNotIdx), outputed=0;
    enum eCrossMapMode{inAllSets,inFirstNotInSetsAfter,inLastSetOnly,inOnlyOneSet};
    idx crossMapMode=formIValue("crossMapMode",inAllSets);


    const char * refList=formValue("reference");

    sStr refSeqs;
    formValue("referenceSubID",&refSeqs);
    sString::searchAndReplaceSymbols(refSeqs,0,"\n;",0,0,true,true,true,true);

    sHiveseq ref(user, refList);

    sDic < idx > idList;
    idx num;


    sStr resultPath;
    reqAddFile(resultPath, "crossingRanges.csv");
    sFil rangeFile(resultPath);
    rangeFile.printf("Reference,start,end,length\n");


    const char * mySeqID;
    for ( idx iSub=0 ; iSub<ref.dim() ; ++iSub ) {
        mySeqID=ref.id(iSub);
        if(mySeqID[0]=='>')++mySeqID;
        if( refSeqs.length() && strcmp(refSeqs.ptr(),"all") != 0 ) {
            if( sString::compareChoice(mySeqID,refSeqs.ptr(0),&num,false,0,true)==sNotIdx)
                continue;

        }
        else num=iSub;
        *idList.set(mySeqID)=num;
    }

    idx noRefsYet=0;
    if(!idList.dim()) {
        *idList.set("no-reference-specified-just-a-dummy")=-2;
        noRefsYet=1;
    }





    const sUsrObjPropsTree * objPropsTree=objs[0].propsTree();
    const sUsrObjPropsNode * annotListList= objPropsTree->find("annotListList") ;
    sDic < sVioAnnot > VioAnnotList;
    sStr buf;

    sDic <bool> refDic;
    sDic < AnnotationOpenInformationCacheItem >  annotationCache;

    for ( idx iSub=0 ; iSub<idList.dim() ; ++iSub ) {
        const char * seqID=(const char * )idList.id(iSub);



        sVec <idx> BMref;
        sVec <idx> BMOrthogonal;
        BMref.flagOn(sMex::fSetZero);
        BMOrthogonal.flagOn(sMex::fSetZero);

        idx annotListBlockNum=0;

        for(const sUsrObjPropsNode * annotListListRow = annotListList->firstChild(); annotListListRow; annotListListRow = annotListListRow->nextSibling(), ++annotListBlockNum) {



            const sUsrObjPropsNode * annotList = annotListListRow->find("annotList");
            if (!annotList) {
                reqSetInfo(req, eQPInfoLevel_Error, "Annotation block is set with no annotation files entered.\n");
                reqSetStatus(req, eQPReqStatus_ProgError);
                break;
            }

            BMOrthogonal.set(0);
            sVec< idx > * BM=(annotListBlockNum==0) ? &BMref : &BMOrthogonal;

            idx cntNumberOfBitsOn=0;

            for(const sUsrObjPropsNode * annotListRow = annotList->firstChild(); annotListRow; annotListRow = annotListRow->nextSibling()) {

                AnnotationOpenInformationCacheItem * annotEl;
                const sUsrObjPropsNode * annotNode= annotListRow->find("annot");
                if(!annotNode)
                    break;
                const char * annotId=annotNode->value();

                annotEl=annotationCache.get(annotId);

                if( ! annotEl) {
                    annotEl=annotationCache.set(annotId);

                    sHiveId annot(annotId);

                    annotEl->pann=new sUsrObj(*user, annot);
                    sUsrObj & ann=*(annotEl->pann);
                    if(!ann.Id()) {
                        printf("ERROR at 213\n");
                        continue;
                    }

                    const char * type=ann.getTypeName();
                    annotEl->isProfiler=sIs("svc-profiler",type);


                    if(annotEl->isProfiler){
                        sStr path;
                        annotEl->pann->getFilePathname(path, "SNPprofile.csv");
                        annotEl->fil=new sFil(path,sMex::fReadonly);
                    }else {
                        if(!annotEl->myAnnotPtr()) {
                            if( annotId ) {

                                annotEl->setMyAnnotPtr(VioAnnotList, annotId);
                                sStr path;
                                ann.getFilePathname00(path, ".vioannot" __);
                                annotEl->myAnnotPtr()->init(path, sMex::fReadonly);
                            }
                        }
                        if(!annotEl->myAnnotPtr())
                            continue;
                    }


                    if(annotListBlockNum==0 && noRefsYet) {

                        if(annotEl->isProfiler){

                            sVec<sHiveId> alignmentIds;
                            ann.propGetHiveIds("parent_proc_ids", alignmentIds);

                            idx prvCountRef=refDic.dim();
                            for ( idx ia=0; ia<alignmentIds.dim(); ia++ ){
                                sUsrObj alObj(*user, alignmentIds[ia]);
                                sVec<sHiveId> references;
                                alObj.propGetHiveIds("subject", references);
                                for ( idx ir=0; ir<references.dim(); ir++ ) {
                                    refDic.set(references.ptr(ir), sizeof(sHiveId));
                                }
                            }

                            if(prvCountRef!=refDic.dim()) {
                                sStr ids;

                                for ( idx iid=0; iid< refDic.dim(); ++iid ) {
                                    ids.printf(",%s", static_cast<const sHiveId*>(refDic.id(iid))->print());
                                }
                                if(idList[(idx)0]==-2) {
                                    idList.empty();
                                    iSub=0;
                                }

                                sHiveseq hs(user,ids.ptr(1));
                                for ( idx ihs=0 ; ihs<hs.dim() ; ++ihs ) {
                                    seqID=hs.id(ihs);
                                    if(seqID[0]=='>')++seqID;
                                    if(idList.get(seqID))
                                        continue;
                                    *idList.set(seqID)=ihs;
                                }
                            }
                        }
                        else {
                            idx cntId =0;
                            idx * idPtr = annotEl->myAnnotPtr()->getIdByIdType("seqID",&cntId);

                            idx prv=-1;
                            if(idList[(idx)0]==-2) {
                                idList.empty();
                                iSub=0;
                            }

                            for( idx is=0; is< cntId; ++is) {
                                if(prv==idPtr[is]){
                                    continue;
                                }
                                buf.cut(0);
                                annotEl->myAnnotPtr()->getIdByIdIndex(buf,idPtr[is]);

                                if (buf) {
                                    *idList.set(buf.ptr(0))=1;
                                }
                                prv=idPtr[is];
                            }
                        }
                        seqID=(const char * )idList.id(iSub);
                    }
                }
                idx cntRanges= 0;
                idx * indexRangePtr=0;
                if(annotEl->isProfiler){
                    idx mySeqID=idList[seqID]+1;

                    sFil * prof;

                    bool oldStyle=false;
                    if(!annotEl->fil->ok() ) {
                        sStr path;
                        annotEl->pann->getFilePathname(path, "SNPprofile-%" DEC ".csv",mySeqID);
                        oldStyle=true;
                        prof= new sFil(path,sMex::fReadonly);
                        if(!prof->ok()){
                            reqSetInfo(req, eQPInfoLevel_Error, "Profile missing or corrupted.");
                            reqSetStatus(req, eQPReqStatus_ProgError);
                        }
                    }else prof=annotEl->fil;



                    if(prof->ok()) {

                        const char * startPos;
                        if(oldStyle==false) {
                            startPos=sBioseqSNP::binarySearchReference(sString::skipWords(prof->ptr(),0,1,sString_symbolsEndline),prof->ptr()+prof->length(),mySeqID);
                            if(!startPos) continue;
                        }else
                            startPos=prof->ptr();

                        sBioseqSNP::SNPRecord Line;
                        Line.position = (unsigned int)-1;

                        const char  * endSNP=prof->ptr()+prof->length();
                        const char * SNPline=startPos ? (oldStyle ? sBioseqSNP::SNPRecordNext(startPos, &Line,endSNP) : sBioseqSNP::SNPConcatenatedRecordNext(startPos, &Line,endSNP, mySeqID )) : 0 ;
                        for (;  SNPline && SNPline < endSNP;
                            SNPline=(oldStyle ? sBioseqSNP::SNPRecordNext(SNPline, &Line,endSNP) : sBioseqSNP::SNPConcatenatedRecordNext(SNPline, &Line,endSNP, mySeqID ) )
                            ) {
                            idx ipos = Line.position;

                            if(crossMapMode>inAllSets)
                                BM->resize(ipos/64 + 1);


                            idx tot=0,var=0;
                            for ( idx ic=0; ic<4; ++ic) {
                                if(ic!=(char)sBioseq::mapATGC[(idx)Line.letter]){
                                    var+=Line.atgc[ic];
                                }
                            }
                            tot=Line.coverage();

                            if (freqThreshold){
                                if(!tot || var*100<tot*freqThreshold){
                                    continue;
                                }
                            }

                            if (coverageThreshold){
                                if (Line.coverage() < coverageThreshold){
                                    continue;
                                }
                            }

                            if(crossMapMode==inAllSets )
                                BM->resize(ipos/64 + 1);
                            idx * bm = BM->ptr(0);
                            bm[ipos/64] |= ((idx)1)<<(ipos%64) ;
                            ++cntNumberOfBitsOn;
                        }
                    }
                    if(prof!=annotEl->fil)
                        delete prof;
                }else {
                    indexRangePtr = annotEl->myAnnotPtr()->getNumberOfRangesByIdTypeAndId("seqID",seqID, cntRanges);

                    for ( idx irange=0; irange< cntRanges ; ++irange ) {


                        idx isMatch=-1;

                        const sUsrObjPropsNode * filterList= annotListRow->find("filterList");
                        for(const sUsrObjPropsNode * filterListRow = filterList->firstChild(); filterListRow ; filterListRow = filterListRow ->nextSibling()) {

                            const sUsrObjPropsNode * filterColumnNode= filterListRow->find("filterColumn");
                            const sUsrObjPropsNode * filterValueNode= filterListRow->find("filterValue");
                            const sUsrObjPropsNode * filterLogicNode= filterListRow->find("filterLogic");

                            const char * filterColumn = filterColumnNode ? filterColumnNode->value() : 0;
                            const char * filterValue = filterValueNode ? filterValueNode->value() : 0;
                            idx filterLogic = filterLogicNode ? filterLogicNode->ivalue() : 0;

                            if (!sLen(filterColumn))
                                break;

                            if (!sLen(filterValue))
                                filterValue = sStr::zero;


                            if(isMatch==-1)
                                isMatch=0;

                            idx cntIDsForRange=annotEl->myAnnotPtr()->getNberOfIdsByRangeIndex(indexRangePtr[irange]);

                            idx isMatchLocal= 0;
                            for( idx iid=0; iid<cntIDsForRange; ++iid)  {
                                const char * idPtr,*idTypePtr;
                                annotEl->myAnnotPtr()->getIdTypeByRangeIndexAndIdIndex(indexRangePtr[irange], iid, &idPtr, 0, &idTypePtr, 0);


                                bool matchedPattern = false;
                                if (sString::searchSubstring(idPtr, sizeof(idPtr), filterValue,1,"", 0)) matchedPattern = true;

                                if ( strcasecmp(filterColumn,idTypePtr)==0 && matchedPattern ) {
                                    isMatchLocal=1;
                                    break;
                                }

                            }

                            if(filterLogic==0)
                                isMatch |= isMatchLocal;
                            if(filterLogic==1)
                                isMatch &= isMatchLocal;
                        }
                        if (isMatch ) {
                            idx cntRangeJoints=0;
                            sVioAnnot::startEnd * rangePtr = annotEl->myAnnotPtr()->getRangeJointsByRangeIndex(indexRangePtr[irange],&cntRangeJoints);
                            cntRangeJoints/=sizeof(sVioAnnot::startEnd);

                            for( idx irj=0; irj<cntRangeJoints; ++irj) {
                                idx end = rangePtr[irj].end;
                                if ((end/64 + 1) > BM->dim()){
                                    BM->resize( end/64 +1 );
                                }
                                idx * bm=BM->ptr(0);

                                for( idx ipos=rangePtr[irj].start; ipos <= rangePtr[irj].end; ++ipos)  {
                                    bm[ipos/64] |= ((idx)1)<<(ipos%64) ;
                                    ++cntNumberOfBitsOn;

                                }
                            }
                        }

                    }



                }
            }

            noRefsYet=0;

            if(annotListBlockNum==0)
                continue;

            idx cntBM = sMax(BMref.dim(), BM->dim());

            BMref.resize(cntBM);
            BMOrthogonal.resize(cntBM);

            idx * bm0=BMref.ptr();
            idx * bm=BM->ptr();


            idx cntSatisfactory=0;
            if(crossMapMode==inFirstNotInSetsAfter) {
                for ( idx ipos=0; ipos<cntBM; ++ ipos) {
                    bm0[ipos]=(bm0[ipos]) & (~bm[ipos]) ;
                }
            } else if(crossMapMode==inLastSetOnly) {
                for ( idx ipos=0; ipos<cntBM; ++ ipos) {
                    bm0[ipos]=(~bm0[ipos]) & bm[ipos];

                }
            } else if(crossMapMode==inOnlyOneSet) {
                for ( idx ipos=0; ipos<cntBM; ++ ipos) {
                    bm0[ipos]= bm0[ipos] ^ bm[ipos];
                }
            } else {
                for ( idx ipos=0; ipos<cntBM; ++ ipos) {
                    bm0[ipos]&=bm[ipos];
                   if(bm0[ipos]) ++cntSatisfactory;
                }
            }

        }


        idx cntBM=sMax(BMref.dim(), BMOrthogonal.dim());
        BMref.resize(cntBM);
        BMOrthogonal.resize(cntBM);

        cntBM *= 64;
        idx * bm=BMref.ptr(0);
        idx start=0,end=0;
        idx mode=0,prvmode=0;



        for ( idx ipos=0; ipos<cntBM; ++ ipos) {
            if(  bm[ipos/64] & ((idx)1)<<(ipos%64) )
                mode=1;
            else mode=0;
            if(mode!=prvmode){
                if( mode==1 ) {
                    start=ipos;
                } else{
                    end=ipos-1;
                    rangeFile.printf("\"%s\",%" DEC ",%" DEC ",%" DEC "\n",seqID,start,end,end-start+1);
                    ++outputed;
                }
            }
            prvmode=mode;
            if(cntMaxOut!=sNotIdx && outputed>=cntMaxOut)
                break;
        }


        if(cntMaxOut!=sNotIdx && outputed>=cntMaxOut)
            break;

        if(mode==1 ) {
            end=cntBM-1;
            rangeFile.printf("\"%s\",%" DEC ",%" DEC ",%" DEC "\n",seqID,start,end,end-start+1);
        }

        if (iSub) {
            if( idList.dim()>99 && (iSub % (idList.dim()/100)==0) )
                if ( !reqProgress(iSub,iSub,idList.dim()) ) {
                    logOut(eQPLogType_Debug, "Interrupted by user");
                    break;
                }
        }

    }

    if ( !reqProgress(0, 100, 100) ) {
        logOut(eQPLogType_Debug, "Interrupted by user");
    }
    reqSetStatus(req, eQPReqStatus_Done);

#ifdef _DEBUG
    ::printf("Range file:\n%s", rangeFile.ptr());
#endif

    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    algoAnnotMapperProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "algo-annotMapper", argv[0]));
    return (int) backend.run(argc, argv);
}


