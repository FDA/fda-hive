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
        algoIonAnnotMapperProc(const char * defline00, const char * srv) : sQPrideProc(defline00, srv) {
            profilerHdr="Ref->Alt:Freq:Cov";
        };

        virtual idx OnExecute(idx);

        struct profilerRetrieveInfo {
                idx curOffSet;
                idx curSeqIndex;
                profilerRetrieveInfo(){
                    curOffSet=curSeqIndex=-1;
                }
        };

        struct AnnotationOpenInformationCacheItem {
            sUsrObj * pann;
            sFil * fil;
            bool isProfiler;
            profilerRetrieveInfo prof_info;
            char curSeq[128];
            idx curSeqLen;
            idx objid;
            sIonWander * wander1, * wander2, *wander3;

            AnnotationOpenInformationCacheItem(){
                fil=0;
                pann=0;
                objid=0;
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

        const char * profilerHdr;

        void collectRefToDict(sDic <idx> & idList, sHiveseq & ref);
        void prepareMyWanders(AnnotationOpenInformationCacheItem * annotEl,aParams & aP,sDic <idx> & idList);
        static idx myPosTraverserCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults);

        void headerPreparation(sVec <sHiveId> * oList,sDic <idx> * hdrDic, sStr * hdrOut=0);

        void checkAnotSource(AnnotationOpenInformationCacheItem * annotEl,aParams & aP);

        void getSeqIdListFromProfiler(idx * iSub,AnnotationOpenInformationCacheItem * annotEl, idx annotListBlockNum, idx noRefsYet ,sDic < sDic <idx> > & dictForProfilers, sDic <idx> & idList);
        idx getProfilerInfo(AnnotationOpenInformationCacheItem * annotEl, sDic < sDic <idx> > & dictForProfilers,const char * seqid, idx seqLen, idx start, idx end, sDic <idx> * hdrDic,sDic < sDic <idx> > * contentDic);

        void retrieveSNPInfo(sBioseqSNP::SNPRecord & Line, real & totFreq, idx & iMutation);

        idx getSeqIdLen(const char ** seqid) {
            idx cur_len=0;
            if (!seqid || !(*seqid)) {
                return cur_len;
            }
            if ((*seqid)[0]=='>') {
                ++(*seqid);
            }
            cur_len=sLen(*seqid);
            const char * space = strpbrk(*seqid," ");
            if(space) {
                cur_len = space-*seqid;
            }
            return cur_len;
        }

        bool filtermyVariant(sBioseqSNP::SNPRecord & Line, idx freqThreshold=5){
            for ( idx ic=0; ic<4; ++ic) {
                if(ic!=(char)sBioseq::mapATGC[(idx)Line.letter] && (( (real)Line.atgc[ic]/Line.coverage() )*100) >= freqThreshold){
                    return true;
                }
            }
            if ( ((( (real)Line.indel[0]/Line.coverage() )*100) >= freqThreshold) || ((( (real)Line.indel[1]/Line.coverage() )*100) >= freqThreshold )  ) {
                return true;
            }

            return false;
        }


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

idx myIdTraverserCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults)
{
    if(!traverserStatement->label || traverserStatement->label[0]!='p')
        return 1;
    sDic < idx > * idList=(sDic < idx > * )ts->callbackFuncParam;

    *idList->set(curResults[0].body,curResults[0].size)=1;

    return 1;
}

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


    if ((pend/64 + 1) > BM->dim()){
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
        for( const char * p=orignal_id; p && *p && !strchr(sString_symbolsSpace,*p); p=nxt+1 ){
               nxt=strpbrk(p,"| ");
               if(!nxt || *nxt==' ')
                   break;

               const char * curId=nxt+1;
               nxt=strpbrk(nxt+1," |");
               if(!nxt)
                   nxt=orignal_id+sizeSeqId;
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
            *newLen=-1;
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
    myWander->traverseBuf.add0(2);

    idx lenHdr=0; idx lenToCmp=0;

    bool isFound=false;
    sStr _tmpBuf;
    for (idx ih=0; ih < hdrDic->dim(); ++ih) {
        const char * id = (const char *) hdrDic->id(ih,&lenHdr);
        isFound=false;

        sString::searchAndReplaceSymbols(&_tmpBuf,myWander->traverseBuf.ptr(0),myWander->traverseBuf.length(),"@",0,0,true,true,true,true);

        for (char *p=_tmpBuf.ptr(); p ; p=sString::next00(p)) {
            char * content = (char *)memchr(p,':',sLen(p));
            if (content) {
                lenToCmp = (lenHdr>(content -p)) ? lenHdr : (content -p);
                content = content+1;

                if (strncmp(p,id,lenToCmp)!=0) {
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


void algoIonAnnotMapperProc::getSeqIdListFromProfiler(idx * iSub,AnnotationOpenInformationCacheItem * annotEl, idx annotListBlockNum, idx noRefsYet ,sDic < sDic <idx> > & dictForProfilers, sDic <idx> & idList) {

    sStr path;
    annotEl->pann->getFilePathname(path, "SNPprofile.csv");
    annotEl->fil=new sFil(path,sMex::fReadonly);

    sVec<sHiveId> alignmentIds;
    annotEl->pann->propGetHiveIds("parent_proc_ids", alignmentIds);

    sVec<sHiveId> references;
    for ( idx ia=0; ia<alignmentIds.dim(); ia++ ){
        sUsrObj alObj(*user, alignmentIds[ia]);
        alObj.propGetHiveIds("subject", references);
    }

    if(!annotListBlockNum && noRefsYet) {
        if(idList[(idx)0]==-2) {
            idList.empty();
            (*iSub)=0;
        }
    }

    sDic <idx> * prof_idList = dictForProfilers.get(&annotEl->objid,sizeof(annotEl->objid));

    if (!prof_idList) {
        prof_idList = dictForProfilers.set(&annotEl->objid,sizeof(annotEl->objid));
        sStr ids;

        for (idx iid=0; iid< references.dim(); ++iid) {
            ids.printf(";%s", references[iid].print());
        }


        sHiveseq hs(user,ids.ptr(1));

        const char * cur_seqid; idx cur_len=0;
        for ( idx ihs=0 ; ihs<hs.dim() ; ++ihs ) {
            cur_seqid=hs.id(ihs);
            cur_len=getSeqIdLen(&cur_seqid);
            if(!prof_idList->get(cur_seqid, cur_len)){
                *prof_idList->set(cur_seqid, cur_len)=ihs;
            }
            if(noRefsYet && !idList.get(cur_seqid, cur_len)){
                *idList.set(cur_seqid, cur_len)=ihs;
            }
        }
    }
}

void algoIonAnnotMapperProc::retrieveSNPInfo(sBioseqSNP::SNPRecord & Line, real & freq, idx & iMutation) {
    real tmpFreq=0;
    freq=0;
    for ( idx ic=0; ic<4; ++ic) {
       if(ic!=(char)sBioseq::mapATGC[(idx)Line.letter]){
           tmpFreq=(real)Line.atgc[ic]/Line.coverage();
           if (tmpFreq>freq) {
               freq=tmpFreq;
               iMutation=ic;
           }
       }
    }
    for (idx ic=0; ic<2; ++ic){
        tmpFreq=(real)Line.indel[ic]/Line.coverage();
        if (tmpFreq>freq) {
           freq=tmpFreq;
           iMutation=4+ic;
        }
    }
}

idx algoIonAnnotMapperProc::getProfilerInfo(AnnotationOpenInformationCacheItem * annotEl, sDic < sDic <idx> > & dictForProfilers,const char * seqid, idx seqLen, idx start, idx end, sDic <idx> * hdrDic,sDic < sDic <idx> > * contentDic) {
    sDic <idx> * prof_idList =dictForProfilers.get(&annotEl->objid,sizeof(annotEl->objid));

    if (! prof_idList || ! annotEl->fil) {
        return 1;
    }
    idx *tmp_pm = prof_idList->get(seqid, seqLen);
    if (!tmp_pm) {
        return 1;
    }
    idx ref_index=*tmp_pm +1;

    sFil * prof = annotEl->fil;
    const char * startPos;

    if (ref_index == annotEl->prof_info.curSeqIndex) {
        startPos=prof->ptr()+annotEl->prof_info.curOffSet;
    }
    else {
        startPos = sBioseqSNP::binarySearchReference(sString::skipWords(prof->ptr(),0,1,sString_symbolsEndline),prof->ptr()+prof->length(),ref_index);
        annotEl->prof_info.curOffSet = startPos - prof->ptr();
        annotEl->prof_info.curSeqIndex = ref_index;
    }
    const char  * endSNP=prof->ptr()+prof->length();

    sBioseqSNP::SNPRecord Line; Line.position = (unsigned int)-1;
    const char * SNPline=sBioseqSNP::SNPConcatenatedRecordNext(startPos, &Line,endSNP, annotEl->prof_info.curSeqIndex );
    const char * prevSNPline = SNPline;

    sStr info; idx ip=0;
    idx iM=0; real varFreq=0;
    for (;  SNPline && SNPline < endSNP; SNPline=sBioseqSNP::SNPConcatenatedRecordNext(SNPline, &Line,endSNP, annotEl->prof_info.curSeqIndex ) ) {
        if ( Line.position < (udx)start) {
            continue;
        }
        if ( Line.position > (udx)end) {
            SNPline=prevSNPline;
            break;
        }
        retrieveSNPInfo(Line, varFreq, iM);
        if (ip) {
            info.printf("|");
        }
        info.printf("%c->%c:%.2lf:%" DEC "",Line.letter,(iM<4) ? sBioseq::mapRevATGC[iM] : ((iM==4) ? '+' : '-'),varFreq,Line.coverage());
        ++ip;
        prevSNPline=SNPline;
    }
    annotEl->prof_info.curOffSet = SNPline - prof->ptr();

    char ibuf[128]; idx ilen;
    sIPrintf(ibuf,ilen,annotEl->pann->Id().objId(),10);
    memcpy(ibuf+ilen,"_",1); ilen+=1;
    memcpy(ibuf+ilen,profilerHdr,sLen(profilerHdr));
    ilen+=sLen(profilerHdr);

    if (info.length() && hdrDic->find(ibuf,ilen)) {
        sDic <idx>  * conD= contentDic->set(ibuf,ilen);
        *(conD->set(info.ptr(),info.length()))=1;
    }

    return 0;
}

void algoIonAnnotMapperProc::headerPreparation(sVec <sHiveId> * oList,sDic <idx> * hdrDic, sStr * hdrOut) {

    sStr tmpPath;

    for (idx io=0; io < oList->dim(); ++io) {
        sUsrObj obj(*user, *oList->ptr(io));
        if( !obj.Id() )
            continue;
        const char * type=obj.getTypeName();
        if (sIs("svc-profiler",type)){
                tmpPath.printf(0,"%s_%s",obj.IdStr(),profilerHdr);
                if (hdrDic && !hdrDic->find(tmpPath.ptr(),tmpPath.length())){
                    *hdrDic->set(tmpPath.ptr(),tmpPath.length())=1;
                    if (hdrOut) {
                        hdrOut->printf(",%s",tmpPath.ptr());
                    }
                }
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
void algoIonAnnotMapperProc::prepareMyWanders(AnnotationOpenInformationCacheItem * annotEl,aParams & aP,sDic <idx> & idList){
    checkAnotSource(annotEl,aP);
    sStr path;
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
    annotEl->wander2->callbackFuncParam=&aP;
    annotEl->wander2->callbackFunc=algoIonAnnotMapperProc::myPosTraverserCallback;


    annotEl->wander3=new sIonWander();
    annotEl->wander3->attachIons(path);
    annotEl->wander3->traverseRecordSeparator = "@";
    annotEl->wander3->traverseFieldSeparator = ":";
    annotEl->wander3->traverseCompile("a=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);unique.1(a.record);b=find.annot(seqID=a.seqID,record=a.record);print(b.type,b.id)");
}

void algoIonAnnotMapperProc::checkAnotSource(AnnotationOpenInformationCacheItem * annotEl,aParams & aP){
    const char * aSrc = annotEl->pann->propGet("annot_source");
    if (aSrc && strncmp(aSrc,"genbank",7)==0) {
        aP.isGb = true;
    } else {
        aP.isGb = false;
    }
}

void algoIonAnnotMapperProc::collectRefToDict(sDic <idx> & idList, sHiveseq & ref){

    sStr refSeqs; formValue("referenceSubID",&refSeqs);
    sString::searchAndReplaceSymbols(refSeqs,0,"\n;",0,0,true,true,true,true);

    idx num=0,seqLen=0;
    const char * mySeqIDs;
    for ( idx iSub=0 ; iSub<ref.dim() ; ++iSub ) {
        mySeqIDs=ref.id(iSub);
        seqLen=getSeqIdLen(&mySeqIDs);
        if( refSeqs.length() && strcmp(refSeqs.ptr(),"all") != 0 ) {
            if( sString::compareChoice(mySeqIDs,refSeqs.ptr(0),&num,false,0,true)==sNotIdx)
                continue;
        }
        else num=iSub;
        *idList.set(mySeqIDs,seqLen)=num;
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

    sVec<sHiveId> objList;
    formHiveIdValues("annot", objList);
    sDic <idx> hdrD; sStr tmpHdr;
    headerPreparation(&objList,&hdrD,&tmpHdr);



    sDic < idx > idList;
    const char * refList=formValue("reference");
    sHiveseq ref(user, refList);
    if (ref.dim()) {
        collectRefToDict(idList, ref);
    }


    sStr resultPath;
    reqAddFile(resultPath, "crossingRanges.csv");
    sFil rangeFile(resultPath);
    if (tmpHdr.length()) {
        rangeFile.printf("Reference,start,end,length%s\n",tmpHdr.ptr());
    }
    else rangeFile.printf("Reference,start,end,length\n");

    idx noRefsYet=0;
    if(!idList.dim()) {
        *idList.set("no-reference-specified-just-a-dummy")=-2;
        noRefsYet=1;
    }


    const sUsrObjPropsTree * objPropsTree=objs[0].propsTree();
    const sUsrObjPropsNode * annotListList= objPropsTree->find("annotListList") ;

    sDic < AnnotationOpenInformationCacheItem >  annotationCache;
    sDic < sDic <idx> > dictForProfilers;
    aParams aP;
    sDic < sDic < idx > > outD;


    for ( idx iSub=0 ; iSub<idList.dim() ; ++iSub ) {
        idx seqIDLen = 0;
        const char * seqID=(const char * )idList.id(iSub, &seqIDLen);
        reqProgress(iSub,iSub,idList.dim());


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
            aP.aBM = BM;

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

                    annotEl->objid = annotNode->ivalue();
                    sHiveId annot(annotId);
                    annotEl->pann=new sUsrObj(*user, annot);
                    if(!annotEl->pann->Id()) {
                        continue;
                    }

                    const char * type=annotEl->pann->getTypeName();
                    annotEl->isProfiler=sIs("svc-profiler",type);




                    if(annotEl->isProfiler){
                        getSeqIdListFromProfiler(&iSub,annotEl,annotListBlockNum,noRefsYet,dictForProfilers,idList);
                        if (!iSub)
                            seqID=(const char * )idList.id(iSub,&seqIDLen);

                    }else {
                        prepareMyWanders(annotEl,aP,idList);

                        if(!annotListBlockNum && noRefsYet) {
                            if(idList[(idx)0]==-2) {
                                idList.empty();
                                iSub=0;
                            }
                            annotEl->wander1->traverse();
                            seqID=(const char * )idList.id(iSub,&seqIDLen);
                        }
                    }
                }


                if(annotEl->isProfiler){
                    idx * pm=idList.get(seqID,seqIDLen);

                    sDic <idx> * prof_idList = dictForProfilers.get(&annotEl->objid,sizeof(annotEl->objid));

                    if (pm && prof_idList) {
                        idx *tmp_pm = prof_idList->get(seqID,seqIDLen);
                        if (tmp_pm)
                            *pm=*tmp_pm +1;
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

                            if(crossMapMode>inAllSets)
                                BM->resize(ipos/64 + 1);
                            if (coverageThreshold){
                                if (Line.coverage() < coverageThreshold){
                                    continue;
                                }
                            }

                            if (!filtermyVariant(Line,freqThreshold)) {
                                continue;
                            }

                            if(crossMapMode==inAllSets )
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
                    annotEl->wander2->resetResultBuf();
                    annotEl->wander2->traverse();

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

        for (idx it=0; it<outD.dim(); ++it) {
            outD.ptr(it)->empty();
        }
        outD.empty();

        for ( idx ipos=0; ipos<cntBM; ++ ipos) {
            if(  bm[ipos/64] & ((idx)1)<<(ipos%64) )
                mode=1;
            else mode=0;
            if(mode!=prvmode){
                if( mode==1 ) {
                    start=ipos;
                } else{
                    end=ipos-1;
                    rangeFile.printf("\"%.*s\",%" DEC ",%" DEC ",%" DEC "",(int)seqIDLen,seqID,start,end,end-start+1);
                    for (idx it=0; it<outD.dim(); ++it) {
                        outD.ptr(it)->empty();
                    }
                    outD.empty();
                    for (idx iiA=0; iiA < annotationCache.dim(); ++iiA) {
                        if (annotationCache.ptr(iiA)->isProfiler){
                            getProfilerInfo(annotationCache.ptr(iiA),dictForProfilers,seqID,seqIDLen,start,end,&hdrD,&outD);
                            continue;
                        }
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
                            rangeFile.printf("%.*s",(int)ctLen,curContent);
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

        for (idx it=0; it<outD.dim(); ++it) {
            outD.ptr(it)->empty();
        }
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

    algoIonAnnotMapperProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "algo-ionAnnotMapper", argv[0]));
    return (int) backend.run(argc, argv);
}


