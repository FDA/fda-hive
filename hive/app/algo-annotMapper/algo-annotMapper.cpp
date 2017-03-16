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
            //sVioAnnot * myAnnotPtr;
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


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ prepare a pool of reference sequences to use for range mappings
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    const char * refList=formValue("reference"); //

    // Find the specific reference IDs user is interested in
    sStr refSeqs;
    formValue("referenceSubID",&refSeqs);
    sString::searchAndReplaceSymbols(refSeqs,0,"\n;",0,0,true,true,true,true);

    // Create the reference Object
    sHiveseq ref(user, refList);

    // Create the dictionary of seqID
    sDic < idx > idList;
    idx num;

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ prepare the output file
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sStr resultPath;
    reqAddFile(resultPath, "crossingRanges.csv");
    sFil rangeFile(resultPath);
    //rangeFile.printf("Reference,start,end\n");
    rangeFile.printf("Reference,start,end,length\n");


    const char * mySeqID;
    for ( idx iSub=0 ; iSub<ref.dim() ; ++iSub ) {  /* Start loop for the iSub*/
        mySeqID=ref.id(iSub);
        if(mySeqID[0]=='>')++mySeqID;
        if( refSeqs.length() && strcmp(refSeqs.ptr(),"all") != 0 ) {
            if( sString::compareChoice(mySeqID,refSeqs.ptr(0),&num,false,0,true)==sNotIdx)
                continue;

        }
        else num=iSub;
        *idList.set(mySeqID)=num;
    }

    // if pool is empty ... fake it: we will generate real ojne from the pool of real annotation pools
    idx noRefsYet=0;
    if(!idList.dim()) {
        *idList.set("no-reference-specified-just-a-dummy")=-2;
        noRefsYet=1;
    }




    // ____/____/____/____/____/____/____/____/____/____/____/____/____/____/____/
    // ____/
    // ____/ retrieve the list of annotation sources
    // ____/
    // ____/____/____/____/____/____/____/____/____/____/____/____/____/____/____/

    const sUsrObjPropsTree * objPropsTree=objs[0].propsTree();
    const sUsrObjPropsNode * annotListList= objPropsTree->find("annotListList") ; // starting from root
    sDic < sVioAnnot > VioAnnotList;
    sStr buf;

    sDic <bool> refDic;
    sDic < AnnotationOpenInformationCacheItem >  annotationCache;

    idx runNumber = 0;
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ loop over all reference sequences
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    for ( idx iSub=0 ; iSub<idList.dim() ; ++iSub ) {
        const char * seqID=(const char * )idList.id(iSub);

        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ looping over annotation blocks: each annotation block is compared to another annotation block
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


        sVec <idx> BMref; // instead of using BMs(vector of vector), we use one bit mask reference vector and an other bit mask vector to Map to the reference vector
        sVec <idx> BMOrthogonal;
        BMref.flagOn(sMex::fSetZero); // ensure that allocated bit arrays are filled with zero
        BMOrthogonal.flagOn(sMex::fSetZero); // ensure that allocated bit arrays are filled with zero

        idx annotListBlockNum=0;

        for(const sUsrObjPropsNode * annotListListRow = annotListList->firstChild(); annotListListRow; annotListListRow = annotListListRow->nextSibling(), ++annotListBlockNum) {


            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
            // _/
            // _/ each annotation block has multiple annotation files
            // _/ so we loop over annotation files
            // _/
            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

            const sUsrObjPropsNode * annotList = annotListListRow->find("annotList");
            if (!annotList) { // this generally should not happen - annotation block should have at least one annotation file
                // There is an annotation block without any annotation in it.
                // Set error for now
                // Could just skip the block; i.e. continue
                reqSetInfo(req, eQPInfoLevel_Error, "Annotation block is set with no annotation files entered.\n");
                reqSetStatus(req, eQPReqStatus_ProgError);
                break;
            }

            //
            // If it is the first time through, we'll store everything in BMref.  This is our 'reference' bitmap of annotations that we'll modify
            // with each annotation list.  For storing annotation bitmaps temporarily (the second list on) we'll use BMOrthogonal.
            //
            BMOrthogonal.set(0);
            sVec< idx > * BM=(annotListBlockNum==0) ? &BMref : &BMOrthogonal; // &BMToMap;

            idx cntNumberOfBitsOn=0; // to maintain if an annotation block has generated even a single bit for this reference

            for(const sUsrObjPropsNode * annotListRow = annotList->firstChild(); annotListRow; annotListRow = annotListRow->nextSibling()) {

                AnnotationOpenInformationCacheItem * annotEl;
                // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                // _/
                // _/ each annotation block has multiple annotation files
                // _/ so we loop over annotation files and open them sequentially
                // _/
                // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                const sUsrObjPropsNode * annotNode= annotListRow->find("annot");
                if(!annotNode) // again this should not happen ... if from interface
                    break;
                const char * annotId=annotNode->value();

                //
                // Query from the cache of annotations that have already been generated to see if the annotation file has already been opened
                // This prevents a single annotation file from being opened repeatedly
                //
                annotEl=annotationCache.get(annotId);

                // If there is no annotation file in the cache, we need to find it and open it for the first time
                if( ! annotEl) {
                    annotEl=annotationCache.set(annotId); // Set the annotation cache so we are able to tell that the annotation has already been opened

                    sHiveId annot(annotId); // retrieve a particular annotation file now

                    //
                    // Save the user object into the annotEl for future use
                    //
                    annotEl->pann=new sUsrObj(*user, annot);
                    sUsrObj & ann=*(annotEl->pann); // why do we use ann instead of annotEl->pann
                    if(!ann.Id()) {
                        printf("ERROR at 213\n");
                        continue;  // wrong object should never happen, generally -> Should this be an error or just continue?  Maybe a warning?
                    }

                    //
                    // Returns either "svc-profiler" or "u-annot" to check to see if it is a profile or a vioAnnot object
                    //
                    const char * type=ann.getTypeName();// at this point we must monitor is the source of annotation a profiler object or real annotation object
                    annotEl->isProfiler=sIs("svc-profiler",type); // If it is a profiler, save the flag


                    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                    // _/
                    // _/ actually initialize sequence feature annotation object - either as a profiler
                    // _/  or as a sequence feature annotation object
                    // _/
                    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                    if(annotEl->isProfiler){
                        sStr path;
                        annotEl->pann->getFilePathname(path, "SNPprofile.csv");
                        annotEl->fil=new sFil(path,sMex::fReadonly);
                    }else { // open an annotation file if it has not been open before
                        //annotEl->myAnnotPtr=VioAnnotList.get(annotId);
                        if(!annotEl->myAnnotPtr()) {
                            if( annotId ) {

                               //::printf("%" DEC " %s %s \n ",runNumber, seqID, annotId);
                               //annotEl->myAnnotPtr= VioAnnotList.set(annotId);
                                annotEl->setMyAnnotPtr(VioAnnotList, annotId);
                                sStr path;
                                ann.getFilePathname00(path, ".vioannot" __);
                                annotEl->myAnnotPtr()->init(path, sMex::fReadonly);
                            }
                        }
                        if(!annotEl->myAnnotPtr()) // check if annotation object is corrupted
                            continue;
                    }


                    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                    // _/
                    // _/ in case if there was not actual reference pool
                    // _/ specified here we initialize from our list of annotations or profilers
                    // _/
                    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                    if(annotListBlockNum==0 && noRefsYet) { // only the first block can initialize a reference and only if no reference pool existed from reference sequence set
                        //idList.empty();

                        if(annotEl->isProfiler){

                            // go get the alignments used for this profiler
                            sVec<sHiveId> alignmentIds;
                            ann.propGetHiveIds("parent_proc_ids", alignmentIds);

                            idx prvCountRef=refDic.dim();
                            for ( idx ia=0; ia<alignmentIds.dim(); ia++ ){
                                sUsrObj alObj(*user, alignmentIds[ia]);
                                // for every alignment use reference sequences
                                sVec<sHiveId> references;
                                alObj.propGetHiveIds("subject", references);
                                for ( idx ir=0; ir<references.dim(); ir++ ) {
                                    refDic.set(references.ptr(ir), sizeof(sHiveId));
                                }
                            }

                            if(prvCountRef!=refDic.dim()) { // if this next profiler didn't bring new reference sequence files into the game: do not add those sequence IDs into our pool
                                sStr ids; // get a uniqified list of all reference sequences

                                for ( idx iid=0; iid< refDic.dim(); ++iid ) {
                                    ids.printf(",%s", static_cast<const sHiveId*>(refDic.id(iid))->print());
                                }
                                if(idList[(idx)0]==-2) {
                                    idList.empty();
                                    iSub=0; // so we can restart the whole sequecne ID list again
                                }

                                sHiveseq hs(user,ids.ptr(1)); // open a hiveseq to retrieve all id lines of those sequences
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
                            // Count the number of IDs in the annotation
                            // Set the pointer to the annotation
                            idx cntId =0;
                            idx * idPtr = annotEl->myAnnotPtr()->getIdByIdType("seqID",&cntId);

                            idx prv=-1;
                            // reset idList if it is currently at 'dummy reference'
                            if(idList[(idx)0]==-2) { // -2 set initially if no reference selected
                                idList.empty();
                                iSub=0; // so we can restart the whole sequecne ID list again
                            }

                            for( idx is=0; is< cntId; ++is) {
                                if(prv==idPtr[is]){
                                    //::printf("%" DEC " %s %s \n ",runNumber, buf.ptr(0), annotId);
                                    continue;
                                }
                                // Read the seqID into buf
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
                //runNumber +=1;
                //::printf("%" DEC " %s %s \n ",runNumber, seqID, annotId);
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

                            // for all nonsimple mapping mode the first block has the complete lengths - because it may store more bits at the end
                            // annotListBlockNum==0 &&
                            if(crossMapMode>inAllSets)
                                BM->resize(ipos/64 + 1);


                            idx tot=0,var=0;
                            for ( idx ic=0; ic<4; ++ic) {
                                if(ic!=(char)sBioseq::mapATGC[(idx)Line.letter]){
                                    var+=Line.atgc[ic];
                                }
                            }
                            tot=Line.coverage();

                            // Frequency Threshold filter
                            if (freqThreshold){
                                if(!tot || var*100<tot*freqThreshold){
                                    continue;
                                }
                            }

                            // Coverage Threshold Filter
                            if (coverageThreshold){
                                if (Line.coverage() < coverageThreshold){
                                    continue;
                                }
                            }

                            if(crossMapMode==inAllSets ) // otherwise this is is already extended
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

                    //
                    // Loop through ranges.  Ignore if the reference is not set the first time
                    //
                    for ( idx irange=0; irange< cntRanges ; ++irange ) {

                        //__/__/__/__/__/__/__/__/__/__/__/__/__/__/
                        // now for every annotation source go through the filters
                        //__/__/__/__/__/__/__/__/__/

                        idx isMatch=-1;

                        const sUsrObjPropsNode * filterList= annotListRow->find("filterList");
                        for(const sUsrObjPropsNode * filterListRow = filterList->firstChild(); filterListRow ; filterListRow = filterListRow ->nextSibling()) {

                            const sUsrObjPropsNode * filterColumnNode= filterListRow->find("filterColumn");
                            const sUsrObjPropsNode * filterValueNode= filterListRow->find("filterValue");
                            const sUsrObjPropsNode * filterLogicNode= filterListRow->find("filterLogic");

                            const char * filterColumn = filterColumnNode ? filterColumnNode->value() : 0;
                            const char * filterValue = filterValueNode ? filterValueNode->value() : 0;
                            idx filterLogic = filterLogicNode ? filterLogicNode->ivalue() : 0;  // 0: OR  | 1: AND

                            if (!sLen(filterColumn))
                                break;

                            if (!sLen(filterValue))
                                filterValue = sStr::zero;


                            if(isMatch==-1)
                                isMatch=0;

                            //__/__/__/__/__/__/__/__/__/__/__/
                            //
                            idx cntIDsForRange=annotEl->myAnnotPtr()->getNberOfIdsByRangeIndex(indexRangePtr[irange]);

                            idx isMatchLocal= 0;
                            for( idx iid=0; iid<cntIDsForRange; ++iid)  {  /* loop for id list */
                                const char * idPtr,*idTypePtr;
                                annotEl->myAnnotPtr()->getIdTypeByRangeIndexAndIdIndex(indexRangePtr[irange], iid, &idPtr, 0, &idTypePtr, 0);

                                // Need to do pattern matching for idPtr & filterValue based on filterValue as the smaller case (i.e. not symetric)
                                // Pattern match

                                bool matchedPattern = false;
                                if (sString::searchSubstring(idPtr, sizeof(idPtr), filterValue,1,"", 0)) matchedPattern = true;

                                if ( strcasecmp(filterColumn,idTypePtr)==0 && matchedPattern ) { //&& strcasecmp(filterValue,idPtr)==0
                                    isMatchLocal=1;
                                    break;
                                }

                            }

                            if(filterLogic==0) // that is an OR
                                isMatch |= isMatchLocal;
                            if(filterLogic==1) // that is an AND
                                isMatch &= isMatchLocal;
                            //}
                        }
                        if (isMatch ) {
                            idx cntRangeJoints=0;
                            sVioAnnot::startEnd * rangePtr = annotEl->myAnnotPtr()->getRangeJointsByRangeIndex(indexRangePtr[irange],&cntRangeJoints);  // LAM
                            cntRangeJoints/=sizeof(sVioAnnot::startEnd);

                            for( idx irj=0; irj<cntRangeJoints; ++irj) {
                                idx end = rangePtr[irj].end;
                                if ((end/64 + 1) > BM->dim()){ // check if the end not less than the array length
                                    BM->resize( end/64 +1 );
                                }
                                idx * bm=BM->ptr(0);

                                for( idx ipos=rangePtr[irj].start; ipos <= rangePtr[irj].end; ++ipos)  {
#if 0
                                    if (BM->dim() > 1) {
                                        bool hit0 = false;
                                        if (ipos/64 <= BM->dim()) {
                                            hit0 = BM[ipos/64] & (((idx)1) << (ipos%64));
                                        }
                                        if (hit0)
                                            fprintf(stderr, "    hit BM%" DEC " pos %" DEC "\n", BM->dim(), ipos);
                                    }
#endif
                                    bm[ipos/64] |= ((idx)1)<<(ipos%64) ;
                                    ++cntNumberOfBitsOn;

                                }
                            }
                        }

                    } // if not profiler



                } // End loop for ranges of a particular annotation file
            } // End loop for annotation files in one comparison block

            noRefsYet=0;

            if(annotListBlockNum==0)
                continue;

            idx cntBM = sMax(BMref.dim(), BM->dim()); // They should be identical at this point

            BMref.resize(cntBM);
            BMOrthogonal.resize(cntBM);

            // make an AND operatin of BMToMap and BMRef
            idx * bm0=BMref.ptr();
            idx * bm=BM->ptr();


            //BMs[0].resize(cntBM);
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
            } else { // Make this the default
                for ( idx ipos=0; ipos<cntBM; ++ ipos) {
                    bm0[ipos]&=bm[ipos];
                   if(bm0[ipos]) ++cntSatisfactory;
                }
            }
            //BMref.cut(cntBM);
            //::printf(" cntSatisfactory %" DEC "\n",cntSatisfactory);

        } // End loop for annotation blocks


        // Start reading the bit mask reference vector in order to print out the range for the specific seqID
        // Find the larger of the two arrays
        idx cntBM=sMax(BMref.dim(), BMOrthogonal.dim());
        // Resize both so they are the same.  The large will not change, the smaller will resize with zeros (fSetZero flags are on)
        BMref.resize(cntBM);
        BMOrthogonal.resize(cntBM);

        // determine the number of actual positions to loop through
        cntBM *= 64;
        idx * bm=BMref.ptr(0);
        idx start=0,end=0;
        idx mode=0,prvmode=0;

      /*  for ( idx ipos=0; ipos<cntBM; ++ ipos) {
            if(  bm[ipos/64] & ((idx)1)<<(ipos%64) ){
               rangeFile.printf("\"%s\",%" DEC ",%" DEC "\n",seqID,ipos,ipos);
            }
            prvmode=mode;
            if(cntMaxOut!=sNotIdx && outputed>=cntMaxOut)
                break;
        }*/


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

    }  // End loop for the iSub

    if ( !reqProgress(0, 100, 100) ) {
        // Ready to set progress to 100, but system is returning that the process was terminated.
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
    sApp::args(argc, argv); // remember arguments in global for future

    algoAnnotMapperProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "algo-annotMapper", argv[0]));
    return (int) backend.run(argc, argv);
}


