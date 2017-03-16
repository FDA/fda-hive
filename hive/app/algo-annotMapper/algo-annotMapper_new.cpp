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



class algoIonAnnotMapperProc: public sQPrideProc
{
    public:
        algoIonAnnotMapperProc(const char * defline00, const char * srv) : sQPrideProc(defline00, srv) {};
        virtual idx OnExecute(idx);

        struct AnnotationOpenInformationCacheItem {
            sUsrObj * pann;
            sFil * fil;
            bool isProfiler;
            char curSeq[128]; // applicable for ionAnnot
            idx curSeqLen;  // for ionAnnot
            sDic <idx> * prof_idList;
            sIonWander * wander1, * wander2, *wander3;

            AnnotationOpenInformationCacheItem(){
                fil=0;
                pann=0;
                prof_idList=0;
                wander1=wander2=wander3=0;
                isProfiler = false;
                curSeqLen=-1;
            }
            ~AnnotationOpenInformationCacheItem()
            {
                if(pann)delete pann;
                if(fil)delete fil;
                if(wander1)delete wander1;
                if(wander2)delete wander2;
                if(wander3)delete wander3;
            }
        };

        struct aParams {
                sVec <idx> * aBM;
                bool isGb;
        };

        static idx myPosTraverserCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults);

        void headerPreparation(sVec <sHiveId> * oList,sDic <idx> * hdrDic, sStr * hdrOut=0);

        void checkAnotSource(AnnotationOpenInformationCacheItem * annotEl,aParams & aP);

};


idx extractIdentifier(const char * fullDefLineFasta, sStr * id=0,bool * isNCBIFormat=0) {
    idx curIdLen=0; sStr curId;
    if (!id) {
        id=&curId;
    }
    id->cut(0);
    const char * myDesc = strchr(fullDefLineFasta,' ');
    if (!myDesc) {
        return 0;
    }
    curIdLen=myDesc-fullDefLineFasta;
    id->addString(fullDefLineFasta,curIdLen);
    id->add0(2);
    if (isNCBIFormat) {
        *isNCBIFormat = strchr(id->ptr(),'|') ? true : false;
    }
    return curIdLen;
}

// Populate the seqID lists when the Ion is first called
idx myIdTraverserCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults)
{
    if(!traverserStatement->label || traverserStatement->label[0]!='p')
        return 1;
    sDic < idx > * idList=(sDic < idx > * )ts->callbackFuncParam;

    *idList->set(curResults[0].body,curResults[0].size)=1;

    return 1;
}

// Populate found position in the bit mask array
idx algoIonAnnotMapperProc::myPosTraverserCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults)
{
    if(!traverserStatement->label || traverserStatement->label[0]!='p')
        return 1;

    aParams * myParams = (aParams *) ts->callbackFuncParam;
    if (myParams->isGb) {
        if (strncmp((const char *)curResults[-2].body,"accession",9)==0){
            return 1;
        }
    }
    sVec< idx > * BM=myParams->aBM;

    idx pos=*((idx * )curResults[-4].body);
    idx pstart =((pos)>>32);
    idx pend = ((pos)&0xFFFFFFFF);


    if ((pend/64 + 1) > BM->dim()){ // check if the end not less than the array length
        BM->resize( pend/64 +1 );
    }
    idx * bm=BM->ptr(0);
    for( idx ipos=pstart; ipos <= pend; ++ipos)  {
        bm[ipos/64] |= ((udx)1)<<(ipos%64) ;
    }
    return 1;
}

const char * searchUsingNCBIseqFormat(sIonWander * myWander,const char * orignal_id,idx seqLen, idx * newLen=0)
{
    sStr seqid; bool isNcbi=false;
    idx curLen = extractIdentifier(orignal_id,&seqid,&isNcbi);

    if (curLen && seqid.length()) {

        myWander->setSearchTemplateVariable("$seqID",6,seqid.ptr(), curLen);
        myWander->resetResultBuf();
        myWander->traverse();

        if (myWander->traverseBuf.length()) {
            return seqid.ptr();
        }
    }

    if (isNcbi) {
        const char * nxt;
        nxt = orignal_id + seqLen;
        idx sizeSeqId=nxt-orignal_id;
        idx lenToCompare=0;
        for( const char * p=orignal_id; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){ // scan until pipe | separated types and ids are there
               nxt=strpbrk(p,"| "); // find the separator
               if(!nxt || *nxt==' ')
                   break;

               const char * curId=nxt+1;
               nxt=strpbrk(nxt+1," |"); // find the separator
               if(!nxt) // if not more ... put it to thee end of the id line
                   nxt=orignal_id+sizeSeqId;/// nxt=seqid+id1Id[1];
               if(*nxt==' ')
                   break;

               lenToCompare = nxt-curId;
               myWander->setSearchTemplateVariable("$seqID",6,curId, lenToCompare);
               myWander->resetResultBuf();
               myWander->traverse();

               if (myWander->traverseBuf.length()){
                   if (newLen) {
                       *newLen=lenToCompare;
                   }
                   return curId;
               }
               const char * dot = strpbrk(curId,".");
               if (dot){
                   lenToCompare = dot-curId;
                   myWander->setSearchTemplateVariable("$seqID",6,curId, lenToCompare);
                   myWander->resetResultBuf();
                   myWander->traverse();
                   if (myWander->traverseBuf.length()){
                       if (newLen){
                           *newLen=lenToCompare;
                       }
                       return curId;
                  }
              }
        }
        if (newLen) {
            *newLen=-1; // not Found
        }
        return 0;
    }
    return 0;
}

idx getAnnotation(sIonWander * myWander,const char * orignal_id,idx seqLen, idx start, idx end,sDic <idx > * hdrDic, sDic < sDic <idx> > * contentDic) {

    if (!myWander) {
        return 0;
    }

    char szStart[128],szEnd[128];szEnd[0]='0';szEnd[1]=':';
    idx lenStart,lenEnd;

    sIPrintf(szStart,lenStart,start,10);memcpy(szStart+lenStart,":0",3);lenStart+=2;
    sIPrintf(szEnd+2,lenEnd,end,10);lenEnd+=2;

    myWander->setSearchTemplateVariable("$seqID1",7,orignal_id,seqLen);
    myWander->setSearchTemplateVariable("$seqID2",7,orignal_id,seqLen);
    myWander->setSearchTemplateVariable("$start",6,szStart,lenStart);
    myWander->setSearchTemplateVariable("$end",4,szEnd,lenEnd);
    myWander->resetResultBuf();
    myWander->traverse();
    if (!myWander->traverseBuf.length()) {
        return 0;
    }
    myWander->traverseBuf.shrink00(0,2);

    idx lenHdr=0; idx lenToCmp=0;

    bool isFound=false;
    for (idx ih=0; ih < hdrDic->dim(); ++ih) {
        const char * id = (const char *) hdrDic->id(ih,&lenHdr);
        isFound=false;
      //  isConcat=false; isInside=false;
        for (char *p=myWander->traverseBuf.ptr(0); p ; p=sString::next00(p)) {
            char * content = strchr(p,':');
            if (content) {
                lenToCmp = (lenHdr>(content -p)) ? lenHdr : (content -p);
                content = content+1;

                if (strncmp(p,id,lenToCmp)!=0) { // not the same as the header
                    continue;
                }
                isFound=true;
                sDic <idx> * conD = contentDic->set(id,lenToCmp);

                idx contentLen=sLen(p)- (content-p);
                if (conD->find(content,contentLen)) {
                    continue;
                }

                for (idx ic=0; ic<contentLen; ++ic) {
                    if (content[ic] && content[ic]==',') {
                        *(content+ic)=' ';
                    }
                }
                *(conD->set(content,contentLen))=1;
            }
        }
        if (!isFound) {
            contentDic->set(id,lenHdr);
        }


    }

    return 0;
}

void algoIonAnnotMapperProc::headerPreparation(sVec <sHiveId> * oList,sDic <idx> * hdrDic, sStr * hdrOut) {

    sStr tmpPath;
    for (idx io=0; io < oList->dim(); ++io) {
        sUsrObj obj(*user, *oList->ptr(io));
        if( !obj.Id() )
            continue;
        const char * type=obj.getTypeName();// at this point we must monitor is the source of annotation a profiler object or real annotation object
        if (sIs("svc-profiler",type)){ // If it is a profiler, no need to get type
            continue;
        }
        tmpPath.cut(0);
        obj.getFilePathname(tmpPath, "ion.ion");
        if (!tmpPath.length()) {
            continue;
        }
        char* s=strrchr(tmpPath.ptr(0),'.');if(s)*s=0;

        sIonWander curWander;
        const char *a = 0;
        curWander.attachIons(tmpPath,sMex::fReadonly,1);
        curWander.traverseRecordSeparator = (const char *) &a;
        curWander.traverseCompile("p=foreach.type(\"\");printCSV(p.1);");
        curWander.traverse();

        curWander.traverseBuf.shrink00(0,2);

        for (const char * p=curWander.traverseBuf.ptr(0); p ; p=sString::next00(p)) {
            if (sIs("listOfConnected",p)) {
                continue;
            }
            if (hdrDic && !hdrDic->find(p)){
                *hdrDic->set(p)=1;
                if (hdrOut) {
                    hdrOut->printf(",%s",p);
                }
            }
        }


    }
}

void algoIonAnnotMapperProc::checkAnotSource(AnnotationOpenInformationCacheItem * annotEl,aParams & aP){
    const char * aSrc = annotEl->pann->propGet("annot_source");
    if (aSrc && strncmp(aSrc,"genbank",7)==0) {
        aP.isGb = true;
    } else {
        aP.isGb = false;
    }
}


idx algoIonAnnotMapperProc::OnExecute(idx req)
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

    // ================Header==========================
    sVec<sHiveId> objList;
    formHiveIdValues("annot",&objList);
    sDic <idx> hdrD; sStr tmpHdr;
    headerPreparation(&objList,&hdrD,&tmpHdr);

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ prepare a pool of reference sequences to use for range mappings
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    const char * refList=formValue("reference"); //



    // =========================================
    // Find the specific reference IDs user is interested in

    sStr refSeqs;
    formValue("referenceSubID",&refSeqs);
    sString::searchAndReplaceSymbols(refSeqs,0,"\n;",0,0,true,true,true,true);

    // Create the reference Object
    sHiveseq ref(user, refList);

    // Create the dictionary of seqID
    sDic < idx > idList;
    idx num;

    // If the reference is specified
    const char * mySeqIDs;
    for ( idx iSub=0 ; iSub<ref.dim() ; ++iSub ) {  /* Start loop for the iSub*/
        mySeqIDs=ref.id(iSub);
        if(mySeqIDs[0]=='>')++mySeqIDs;
        if( refSeqs.length() && strcmp(refSeqs.ptr(),"all") != 0 ) {
            if( sString::compareChoice(mySeqIDs,refSeqs.ptr(0),&num,false,0,true)==sNotIdx)
                continue;

        }
        else num=iSub;
        *idList.set(mySeqIDs)=num;
    }

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ prepare the output file
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    sStr resultPath;
    reqAddFile(resultPath, "crossingRanges.csv");
    sFil rangeFile(resultPath);
    //rangeFile.printf("Reference,start,end\n");
    if (tmpHdr.length()) {
        rangeFile.printf("Reference,start,end,length%s\n",tmpHdr.ptr());
    }
    else rangeFile.printf("Reference,start,end,length\n");


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

    /*sDic <bool> refDic;*/
    sDic < AnnotationOpenInformationCacheItem >  annotationCache;
    sDic < sDic <idx> > dictForProfilers;
    aParams aP;
    // ========================



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ loop over all reference sequences
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    for ( idx iSub=0 ; iSub<idList.dim() ; ++iSub ) {
        idx seqIDLen = 0; // in Ion, there is no zero ended for seqID, need to keep track of length
        const char * seqID=(const char * )idList.id(iSub, &seqIDLen);

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

        // loop over blocks of annotations (annots and/or profilers)
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
            aP.aBM = BM;

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
                    annotEl->pann=new sUsrObj(*user, annot);
                    if(!annotEl->pann->Id()) {
                        continue;  // wrong object should never happen, generally -> Should this be an error or just continue?  Maybe a warning?
                    }

                    //
                    // Returns either "svc-profiler" or "u-annot" to check to see if it is a profile or a vioAnnot object
                    //
                    const char * type=annotEl->pann->getTypeName();// at this point we must monitor is the source of annotation a profiler object or real annotation object
                    annotEl->isProfiler=sIs("svc-profiler",type); // If it is a profiler, save the flag


                    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                    // _/
                    // _/ actually initialize sequence feature annotation object - either as a profiler
                    // _/  or as a sequence feature annotation object
                    // _/
                    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
                    sStr path;

                    if(annotEl->isProfiler){
                        // Profiler needs to know the index of its references in order to retrieve the correct information in SNP-profile.csv
                        annotEl->pann->getFilePathname(path, "SNPprofile.csv");
                        annotEl->fil=new sFil(path,sMex::fReadonly);

                        // go get the alignments used for this profiler
                        sVec<sHiveId> alignmentIds;
                        annotEl->pann->propGetHiveIds("parent_proc_ids", alignmentIds);

                        //idx prvCountRef=refDic.dim();
                        sVec<sHiveId> references;
                        for ( idx ia=0; ia<alignmentIds.dim(); ia++ ){
                            sUsrObj alObj(*user, alignmentIds[ia]);
                            // for every alignment use reference sequences

                            alObj.propGetHiveIds("subject", references);
                            /*for ( idx ir=0; ir<references.dim(); ir++ ) {
                                refDic.set(references.ptr(ir), sizeof(sHiveId));
                            }*/
                        }

                        if(!annotListBlockNum && noRefsYet) { // only the first block can initialize a reference and only if no reference pool existed from reference sequence set
                            if(idList[(idx)0]==-2) {
                                idList.empty();
                                iSub=0; // so we can restart the whole sequecne ID list again
                            }
                        }

                        //if(prvCountRef!=refDic.dim()) { // if this next profiler didn't bring new reference sequence files into the game: do not add those sequence IDs into our pool
                          if (!annotEl->prof_idList) {
                            sStr ids; // get a uniqified list of all reference sequences

                            /*for ( idx iid=0; iid< refDic.dim(); ++iid ) {
                                ids.printf(",%s", static_cast<const sHiveId*>(refDic.id(iid))->print());
                            }*/
                            for (idx iid=0; iid< references.dim(); ++iid) {
                                ids.printf(",%s", references[iid].print());
                            }
                            annotEl->prof_idList = dictForProfilers.set(annotId);

                            sHiveseq hs(user,ids.ptr(1)); // open a hiveseq to retrieve all id lines of those sequences

                            const char * cur_seqid;
                            for ( idx ihs=0 ; ihs<hs.dim() ; ++ihs ) {
                                cur_seqid=hs.id(ihs);
                                if(cur_seqid[0]=='>')++cur_seqid;
                                if(!annotEl->prof_idList->get(cur_seqid)){
                                    *annotEl->prof_idList->set(cur_seqid)=ihs;
                                }
                                if(noRefsYet && !idList.get(cur_seqid)){ // if first time
                                    *idList.set(cur_seqid)=ihs; // in SNP-profiler, the index is 1-based
                                }
                            }
                        }
                        if (!iSub) // if first time
                            seqID=(const char * )idList.id(iSub,&seqIDLen);

                    }else { // open an annotation file if it has not been open before
                        checkAnotSource(annotEl,aP);

                        annotEl->wander1=new sIonWander();
                        annotEl->pann->getFilePathname(path, "ion.ion");
                        char* s=strrchr(path.ptr(0),'.');if(s)*s=0;
                        annotEl->wander1->attachIons(path);
                        annotEl->wander1->traverseCompile("p=foreach.seqID(\"\");");
                        annotEl->wander1->callbackFuncParam=&idList;
                        annotEl->wander1->callbackFunc=myIdTraverserCallback;

                        annotEl->wander2=new sIonWander();
                        annotEl->wander2->attachIons(path);
                        annotEl->wander2->traverseCompile("a=find.annot(seqID=$seqID);unique.1(a.pos);p=blob(a.pos);");
                        //annotEl->wander2->callbackFuncParam=BM;
                        annotEl->wander2->callbackFuncParam=&aP;
                        annotEl->wander2->callbackFunc=algoIonAnnotMapperProc::myPosTraverserCallback;

                        const char *a = 0;
                        annotEl->wander3=new sIonWander();
                        annotEl->wander3->attachIons(path);
                        //annotEl->wander3->traverseRecordSeparator = ";";
                        annotEl->wander3->traverseRecordSeparator = (const char *)&a;
                        annotEl->wander3->traverseFieldSeparator = ":";
                        annotEl->wander3->traverseCompile("a=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);unique.1(a.record);b=find.annot(seqID=a.seqID,record=a.record);print(b.type,b.id)");
                        // r=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);unique.1(r.pos);blob(r.pos,r.type,r.id);

                        if(!annotListBlockNum && noRefsYet) { // only the first block can initialize a reference and only if no reference pool existed from reference sequence set
                            if(idList[(idx)0]==-2) {
                                idList.empty();
                                iSub=0; // so we can restart the whole sequecne ID list again
                            }
                            // reset idList if it is currently at 'dummy reference'
                            annotEl->wander1->traverse();
                            seqID=(const char * )idList.id(iSub,&seqIDLen);
                        }
                    }
                }


                if(annotEl->isProfiler){
                    idx * pm=idList.get(seqID,seqIDLen);

                    if (pm && annotEl->prof_idList) { // if found in the main idList
                        idx *tmp_pm = annotEl->prof_idList->get(seqID);
                        if (tmp_pm)
                            *pm=*tmp_pm +1; // in SNP-profiler, the index is 1-based
                        else *pm=0;
                    }
                    else *pm=0;
                    idx mySeqID=(pm ? (*pm) : 0 );

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
                            bm[ipos/64] |= ((udx)1)<<(ipos%64) ;
                            ++cntNumberOfBitsOn;
                        }
                    }
                    if(prof!=annotEl->fil)
                        delete prof;
                }else {

                    annotEl->curSeqLen =-1;
                    idx sl=seqIDLen;
                    annotEl->wander2->setSearchTemplateVariable("$seqID",6,seqID,sl);
                    annotEl->wander2->traverse();

                    // use for Human seqID, because sometimes it has the prefix 'chr', sometimes not, try both case to match them
                    if( strncasecmp(seqID,"chr",3)==0 ) {
                        annotEl->wander2->setSearchTemplateVariable("$seqID",6,seqID+3,sl-3);
                        annotEl->wander2->resetResultBuf();
                        annotEl->wander2->traverse();
                        if (annotEl->wander2->traverseBuf.length()) {
                            annotEl->curSeqLen = sl-3;
                            strncpy(annotEl->curSeq, seqID+3,annotEl->curSeqLen);
                        }
                    }
                    else if(isdigit(seqID[0])) {
                        char b[128] = "chr";
                        strncat(b,seqID, seqIDLen);
                        annotEl->wander2->setSearchTemplateVariable("$seqID",6,b,sl+3);
                        annotEl->wander2->resetResultBuf();
                        annotEl->wander2->traverse();
                        if (annotEl->wander2->traverseBuf.length()) {
                            annotEl->curSeqLen = sl+3;
                            strncpy(annotEl->curSeq, b,annotEl->curSeqLen);
                        }
                    }

                    if (!annotEl->wander2->traverseBuf.length()){
                        annotEl->wander2->resetResultBuf();
                        annotEl->wander2->setSearchTemplateVariable("$seqID",6,seqID,sl);
                        annotEl->wander2->traverse();
                        if (annotEl->wander2->traverseBuf.length()) {
                            annotEl->curSeqLen = sl;
                            strncpy(annotEl->curSeq, seqID,annotEl->curSeqLen);
                        }
                    }
                    if (!annotEl->wander2->traverseBuf.length()){
                        const char * matchSeq = searchUsingNCBIseqFormat(annotEl->wander2,seqID,sl, &annotEl->curSeqLen);
                        if (annotEl->curSeqLen!=-1){
                            strncpy(annotEl->curSeq, matchSeq,annotEl->curSeqLen);
                        }
                    }

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

        sDic < sDic < idx > > outD;

        for ( idx ipos=0; ipos<cntBM; ++ ipos) {
            if(  bm[ipos/64] & ((idx)1)<<(ipos%64) )
                mode=1;
            else mode=0;
            if(mode!=prvmode){
                if( mode==1 ) {
                    start=ipos;
                } else{
                    end=ipos-1;
                    //rangeFile.printf("\"%.*s\",%" DEC ",%" DEC ",%" DEC "\n",(int)seqIDLen,seqID,start,end,end-start+1);
                    rangeFile.printf("\"%.*s\",%" DEC ",%" DEC ",%" DEC "",(int)seqIDLen,seqID,start,end,end-start+1);
                    outD.empty();
                    for (idx iiA=0; iiA < annotationCache.dim(); ++iiA) {
                        if (annotationCache.ptr(iiA)->isProfiler)
                            continue;
                        if (annotationCache.ptr(iiA)->curSeqLen != -1){
                            getAnnotation(annotationCache.ptr(iiA)->wander3,annotationCache.ptr(iiA)->curSeq,annotationCache.ptr(iiA)->curSeqLen,start,end,&hdrD,&outD);
                        } else {
                            getAnnotation(annotationCache.ptr(iiA)->wander3,seqID,seqIDLen,start,end,&hdrD,&outD);
                        }
                    }
                    idx ctLen=0;
                    for (idx ih=0; ih < outD.dim(); ++ih) {
                        rangeFile.printf(",");
                        for (idx ict=0; ict < outD.ptr(ih)->dim(); ++ict) {
                            if (ict) {
                                rangeFile.printf("|");
                            }
                            const char * curContent = (const char *)outD.ptr(ih)->id(ict,&ctLen);
                            rangeFile.addString(curContent,ctLen);
                        }
                    }
                    rangeFile.printf("\n");
                    ++outputed;
                }
            }
            prvmode=mode;
            if(cntMaxOut!=sNotIdx && outputed>=cntMaxOut)
                break;
        }


        if(cntMaxOut!=sNotIdx && outputed>=cntMaxOut)
            break;

        outD.empty();
        if(mode==1 ) {
            end=cntBM-1;
            rangeFile.printf("\"%.*s\",%" DEC ",%" DEC ",%" DEC ",",(int)seqIDLen,seqID,start,end,end-start+1);
            for (idx iiA=0; iiA < annotationCache.dim(); ++iiA) {
                if (annotationCache.ptr(iiA)->isProfiler)
                    continue;
                 if (annotationCache.ptr(iiA)->curSeqLen !=-1){
                     getAnnotation(annotationCache.ptr(iiA)->wander3,annotationCache.ptr(iiA)->curSeq,annotationCache.ptr(iiA)->curSeqLen,start,end,&hdrD,&outD);
                 }
                 else {
                     getAnnotation(annotationCache.ptr(iiA)->wander3,seqID,seqIDLen,start,end,&hdrD,&outD);
                 }
           }
            idx ctLen=0;
            for (idx ih=0; ih < outD.dim(); ++ih) {
                rangeFile.printf(",");
                for (idx ict=0; ict < outD.ptr(ih)->dim(); ++ict) {
                    if (ict) {
                        rangeFile.printf("|");
                    }
                    const char * curContent = (const char *)outD.ptr(ih)->id(ict,&ctLen);
                    rangeFile.addString(curContent,ctLen);
                }
            }
            rangeFile.printf("\n");

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

    algoIonAnnotMapperProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "algo-ionAnnotMapper", argv[0]));
    return (int) backend.run(argc, argv);
}


