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
#include <ion/sIon-core.hpp>
#include <sys/types.h>
#include <regex.h>

const char * sIonWander::traverseCommands=
    "find" _
    "count" _
    "search" _
    "delete" _
    "add" _
    "print" _
    "printCSV" _
    "body" _
    "blob" _
    "serial" _
    "return" _
    "set" _
    "foreach" _
    "jump!" _
    "jump" _
    "dict!" _
    "dict" _
    "ddict!" _
    "ddict" _
    "sum!" _
    "sum" _
    "unique!" _
    "unique" _
    "check_on" _
    "check_off" _
    "itraverse" _
    "skip" _
    "halt" __;




#define traverseJump( _v_where ) \
        {\
            if(callbackFunc) \
                res=callCalbackFunc(curIon,this,shdr,stackResults.ptr(curResultCnt) ); \
            if(res) { \
                if( (_v_where) != sIdxMax ) \
                    ofsTrajectory=(_v_where)>0 ? _v_where : curOfs+_v_where; \
                else \
                    ofsTrajectory+=sizeInstruction+1; \
                ++levelOperation; \
                res=traverse(0, listStart==-1 ? myListIndex : listStart, listEnd==-1 ? myListIndex+1 : listEnd ); \
                listCallingIon=myListIndex; \
                ofsTrajectory=curOfs; \
                if(this->debug){ \
                    debugStatementBuf.cut(ofsStatementBuf); \
                } \
                stackResults.cut(curResultCnt); \
                --levelOperation; \
            } \
        }
#define traverseContinue( ) traverseJump( sIdxMax )


idx sIonWander::traverseReal(sIonRef * ionref, idx start, idx myListIndex)
{

    sIon * curIon=ionref->ion;
    idx * searchTrajectory=ionref->SearchTrajectory.ptr();


    if(start)
        ofsTrajectory=start;
    StatementHeader * shdr=(StatementHeader * )((idx*)searchTrajectory+ofsTrajectory+1);
    idx relationTypeIndex=shdr->relationTypeIndex;

    idx trCommand=relationTypeIndex>>32;
    if(trCommand>=eTraverseSkipOperation)
        return 1;
    idx res=1,transientCommand=0;
    idx sizeInstruction=searchTrajectory[ofsTrajectory+0];

    relationTypeIndex=relationTypeIndex&0xFFFFFFFFll;
    idx relationHasherIndexType=shdr->relationHasherIndexType;
    idx relationSearchEngineIndex=relationHasherIndexType>>32;
    relationHasherIndexType&=0xFFFFFFFF;

    sIon::RelationType * pRelationType = 0;
    idx cntResultTargets = 0;
    if(trCommand<=eTraverseRelationBased) {
        pRelationType=curIon->relationTypesArr.ptr(relationTypeIndex);
        cntResultTargets=pRelationType->cntRelationTargets;
    }
    idx posStatementBuf=debugStatementBuf.length();
    idx ofsStatementBuf=0;
    char intBuf[128] = "0";


    idx curOfs=ofsTrajectory, curResultCnt=stackResults.dim();
    listCallingIon=myListIndex;

    if(this->debug){
        idx iss;
        debugStatementBuf.printf("\n#%s %4" DEC " ",shdr->label ? shdr->label :" " ,ionref->ionSerial);
        for( idx it=1; it<levelOperation;++it)
            debugStatementBuf.printf("  ");
        debugStatementBuf.printf("%s",sString::next00(traverseCommands,trCommand) );
        if( trCommand < eTraverseRelationBased )
            debugStatementBuf.printf(".%s",(const char * )curIon->baseContainer.ptr(curIon->relationTypesArr.ptr(relationTypeIndex)->nameOfs) );
        debugStatementBuf.printf("(");
        for( iss=0; iss<=sizeInstruction; ++iss) {
            if(iss)debugStatementBuf.printf(",");
            if(iss==1)debugStatementBuf.printf("%" DEC ":%" DEC "",searchTrajectory[curOfs+iss]>>32,searchTrajectory[curOfs+iss]&0xFFFFFFFF);
            else if(iss<=(idx)(sizeof(StatementHeader)/sizeof(idx))) debugStatementBuf.printf("%" DEC "",searchTrajectory[curOfs+iss]);
            else {
                debugStatementBuf.printf("%" DEC "",searchTrajectory[curOfs+iss]);
                bool doParVal=false;
                if(trCommand==eTraverseSearch ){
                    if( iss%3==2) doParVal=true;
                } else {
                    if( iss%2==1)doParVal=true;
                }
                if(doParVal){
                    idx vali=searchTrajectory[curOfs+iss];
                    const char * val=sConvInt2Ptr(vali,char );
                    idx valSize=searchTrajectory[curOfs+iss+1];
                    if(vali==sNotIdx ) {
                        sIon::RecordResult * rr=stackResults.ptr(valSize>=0 ? valSize  : (stackResults.dim()+valSize) );
                        val=(const char*)rr->body;
                        valSize=rr->size;
                    }
                    if( valSize>0 ) debugStatementBuf.printf("=%.*s",(int)valSize,val);
                }
            }
        }
        debugStatementBuf.printf(") ========> ");
        debugStatementBuf.add0(2);
        ofsStatementBuf=debugStatementBuf.length();
        ::printf("%s",debugStatementBuf.ptr(posStatementBuf));

    }


    idx ivar, isMatch=1 , varCnt=0, vnum;
    if(trCommand>eTraverseRelationHashBased && trCommand!=eTraverseReturn) {
        transientCommand=1;
        varCnt=sizeInstruction-sizeof(StatementHeader)/sizeof(idx);
    }

    char cntWuf[256];
    char cntSet[256];
    idx recordIndexArr[1024];
    idx travLimitStart=0, travLimitEnd=sIdxMax;
    idx dictOnCntValues=relationTypeIndex ? relationTypeIndex*2 : 2 ;
    idx valSizePrev=0;
    const void * vBodyPrev="";
    if( varCnt ) {
        isMatch=0;
        idx val,valSize;
        const void * vBody;
        idx * bodies=searchTrajectory+ofsTrajectory+1+sizeof(StatementHeader)/sizeof(idx);

        for ( vnum=0, ivar=0; ivar < varCnt; ++ivar, ++vnum) {
            idx valType=-1;
            bool comparisonNeeded=true;
            if(trCommand==eTraverseSearch){++ivar;}
            val=bodies[ivar];++ivar;
            vBody=sConvInt2Ptr(val,void);
            valSize=bodies[ivar];
            idx iVarInComparison=ivar;

            if(val==sNotIdx-1)
                continue;

            sIon::RecordResult * rr=stackResults.ptr(valSize>=0 ? valSize  : (stackResults.dim()+valSize) );
            if(val==sNotIdx || val==sNotIdx-2) {

                if( val==sNotIdx-2 ) {
                    vBody=(void*)(&(rr->index));
                    valSize=sizeof(rr->index);
                    valType=rr->cType;
                } else {
                    vBody=rr->body;
                    valSize=rr->size;
                    valType=rr->cType;
                }
            }
            else if(((const char * ) vBody)[0]=='#'){

                idx * pCnt=resultCounter.get((char*)vBody+1, valSize-1);
                if(pCnt){
                    sIPrintf(cntWuf, valSize, (*pCnt), 10 );
                    vBody=cntWuf;
                }
            }

            if(trCommand==eTraverseAdd ) {
                idx * pRelationTargetTypes=curIon->relationTargetsArr.ptr(pRelationType->ofsRelationTargets);
                idx rnum=(vnum)%pRelationType->cntRelationTargets;
                recordIndexArr[rnum]=curIon->addRecord(pRelationTargetTypes[rnum],valSize,vBody);
                if(rnum==(pRelationType->cntRelationTargets-1) )
                    curIon->addRelationVarg(relationTypeIndex,sNotIdx, recordIndexArr,0);
                comparisonNeeded=false;
            } else if( trCommand==eTraversePrint || trCommand==eTraverseBody  ) {
                if(someInRecord && traverseFieldSeparator){
                    if(*traverseFieldSeparator == 0){pTraverseBuf->add0();}
                    else {pTraverseBuf->add(traverseFieldSeparator,sLen(traverseFieldSeparator));}
                }
                if(valType!=-1 && trCommand==eTraversePrint) {
                    if(recordProtectQuote && valType==sIon::eCTypeString )
                        pTraverseBuf->add(&recordProtectQuote,1);
                    sIon_outTextBody(pTraverseBuf,valType,vBody, valSize , internalSeparator);
                    if(recordProtectQuote && valType==sIon::eCTypeString )
                        pTraverseBuf->add(&recordProtectQuote,1);

                }else
                    pTraverseBuf->add((const char* )vBody,valSize);
                someInRecord=true;
                traverseOutputting=true;
                comparisonNeeded=false;

            } else if( trCommand==eTraversePrintCSV ) {
                if( someInRecord ) {
                    if(traverseFieldSeparator){
                        if(*traverseFieldSeparator == 0){pTraverseBuf->add0();}
                        else {pTraverseBuf->add(traverseFieldSeparator,sLen(traverseFieldSeparator));}
                    }else
                        pTraverseBuf->addString(",", 1);
                }
                if( valType == sIon::eCTypeString ) {
                    const char * sBody = (const char *)vBody;
                    if( idx len = valSize && *sBody ? valSize : 0 ) {
                        idx bufpos = pTraverseBuf->length();
                        sString::escapeForCSV(*pTraverseBuf, sBody, len);
                        pTraverseBuf->callback(pTraverseBuf->ptr(bufpos));
                    }
                } else if( valType >= 0 ) {
                    sIon_outTextBody(pTraverseBuf,valType,vBody, valSize, internalSeparator );
                }else
                    pTraverseBuf->add((const char* )vBody,valSize);
                someInRecord=true;
                traverseOutputting=true;
                comparisonNeeded=false;
            } else if( trCommand==eTraverseSerial) {
                idx s=sizeof(rr->index);
                pTraverseBuf->add((const char * )&s,sizeof(s));
                pTraverseBuf->add((const char * )&(rr->index),sizeof(rr->index));
            }else if( trCommand==eTraverseBlob) {

                {
                    pTraverseBuf->add((const char * )&valSize,sizeof(valSize));
                    pTraverseBuf->add((const char * )&vBody,sizeof(vBody));
                }
                comparisonNeeded=false;
            }
            else if( trCommand==eTraverseDict || trCommand==eTraverseDictNot || trCommand==eTraverseDDict || trCommand==eTraverseDDictNot || trCommand==eTraverseSum || trCommand==eTraverseSumNot ) {

                if ( ivar<dictOnCntValues ) {
                    if(ivar==1)dictBuf.cut(0);
                    else dictBuf.add0(1);
                    dictBuf.add((const char* )vBody,valSize);
                    if(varCnt<=1+dictOnCntValues+2)
                        isMatch=1;
                    comparisonNeeded=false;
                }
                else
                    iVarInComparison-=dictOnCntValues+1;
            }
            else if( trCommand==eTraverseIonLimit ) {

                if ( ivar==1 ) {
                    if( valSize==4 && strcmp((const char*) vBody,"self")==0){
                        travLimitStart=-1;
                    }else if( valSize==4 && strcmp((const char*) vBody,"this")==0){
                        travLimitStart=myListIndex;
                    }else  {
                        sIScanf(travLimitStart,(const char* )vBody,valSize,10);
                    }
                }
                else if( ivar==3) {

                    if( valSize==4 && strcmp((const char*) vBody,"self")==0){
                        travLimitEnd=-1;
                    }else if( valSize==4 && strcmp((const char*) vBody,"this")==0){
                        travLimitEnd=myListIndex;
                    }else if( valSize==3 && strcmp((const char*) vBody,"all")==0){
                        travLimitEnd=sIdxMax;
                    }else  {
                        sIScanf(travLimitEnd,(const char*) vBody,valSize,10);
                    }
                }
            }
            else if( trCommand==eTraverseUniqueNot || trCommand==eTraverseUnique ) {
                if(ivar==1)uniqBuf.cut(0);
                uniqBuf.add((const char* )vBody,valSize);
                comparisonNeeded=false;
            }
            else if( trCommand==eTraverseForeach ) {

                stackResults.resize(curResultCnt+1);
                sIon::RecordResult * pRR=stackResults.ptr(curResultCnt);

                pRR->body=vBody;
                pRR->size=valSize;
                pRR->cType=sIon::eCTypeString;
                pRR->index=ivar;
                pRR->typeIndex=sNotIdx;


                if(this->debug){
                    if(ivar>1)
                        ::printf("%s",debugStatementBuf.ptr(posStatementBuf));
                    ::printf("//%.*s",(int)pRR->size,(const char * )pRR->body);
                }
                if(relationTypeIndex) {
                    continueScope=true;
                        ofsTrajectory+=sizeInstruction+1;
                        ++levelOperation;

                        sIon::RecordType * pRecordType=curIon->recordTypesArr.ptr(relationTypeIndex-1);
                        sVec <sIon::Record> * recordsArr;sIon_ensureContainerIon(curIon,pRecordType,recordsArr);
                        sMex * recordBodyContainer;sIon_ensureContainerIon(curIon,pRecordType,recordBodyContainer);
                        for( idx ir = 0 ; res && ir<recordsArr->dim();  ++ir ){
                            if(ir<shdr->limitStart) continue;
                            if(shdr->limitEnd && ir>=shdr->limitEnd) break;
                            sIon::Record * pRecord=recordsArr->ptr(ir);
                            const void * pRecordBody;
                            if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                                pRecordBody=(const void * ) &(pRecord->ofs);
                            } else {
                                pRecordBody=recordBodyContainer->ptr(pRecord->ofs);
                            }
                            pRR->body=pRecordBody;
                            pRR->size=pRecord->size;
                            if(callbackFunc)
                                res=callCalbackFunc(curIon,this,shdr,stackResults.ptr(curResultCnt) );
                            if(res)
                                res=traverse();

                        }
                        ofsTrajectory=curOfs;
                        stackResults.cut(curResultCnt);
                        --levelOperation;

                } else {
                    traverseContinue( ) ;
                }
                if(!res)
                    break;

                comparisonNeeded=false;
            }
            else if( trCommand==eTraverseSet ) {
                idx rc=curResultCnt+1+((ivar-1)/2+1);
                stackResults.resize(rc);
                sIon::RecordResult * pRR;
                if(ivar==1) {
                    pRR=stackResults.ptr(rc-2);
                    sIPrintf(cntSet, pRR->size, varCnt/2, 10 );
                    pRR->body=cntSet;
                    pRR->cType=sIon::eCTypeString;
                    pRR->index=0;
                    pRR->typeIndex=sNotIdx;
                }

                pRR=stackResults.ptr(rc-1);
                pRR->body=vBody;
                pRR->size=valSize;
                pRR->cType=sIon::eCTypeString;
                pRR->index=ivar;
                pRR->typeIndex=sNotIdx;

                if(this->debug){
                    ::printf("//%.*s",(int)pRR->size,(const char * )pRR->body);
                }

                comparisonNeeded=false;
            }

            if(comparisonNeeded){
                if( ((iVarInComparison)%4)==3 ) {
                    if(((const char*)vBody)[0]=='^'){
                        if( (vBody==vBodyPrev ||  (memcmp(sShift(vBody,1),vBodyPrev,valSize-1))==0) ){++isMatch;break;}
                    }
                    else if(((const char*)vBody)[valSize-1]=='$'){
                        if( (vBody==vBodyPrev ||  (memcmp(vBody,sShift(vBodyPrev,valSizePrev-valSize+1),valSize-1))==0) ){++isMatch;break;}
                    }

                    else if( valSize==valSizePrev && (vBody==vBodyPrev ||  (memcmp(vBody,vBodyPrev,valSize))==0) ){
                        ++isMatch;
                    }
                    if(this->debug){
                        ::printf("//%.*s == %.*s ",(int)valSizePrev,(char*)vBodyPrev,(int)valSize,(char*)vBody);
                    }

                }
                vBodyPrev=vBody;
                valSizePrev=valSize;
            }
            if(!res)break;
        }

    }

    if(trCommand==eTraverseAdd )
        return res;

    if(trCommand==eTraverseForeach )
        return res;

    if(trCommand==eTraverseSet ){
        traverseContinue( ) ;
        return res;
    }
    if(trCommand==eTraverseCheckOn) {
        bool myCheck=checkOn;
        checkOn=true;
        traverseContinue( ) ;
        checkOn=myCheck;
        return res;
    }
    if(trCommand==eTraverseCheckOff){
        bool myCheck=checkOn;
        checkOn=false;
        traverseContinue( ) ;
        checkOn=myCheck;
        return res;
    }


    else if(trCommand==eTraversePrint || trCommand==eTraversePrintCSV || trCommand==eTraverseBody  || trCommand==eTraverseBlob ) {
        traverseContinue();
        return res;
    }

    if( trCommand==eTraverseIonLimit) {
        idx prvTraverseLimitStart=this->listStart;
        idx prvTraverseLimitEnd=this->listEnd;
        this->listStart=travLimitStart;
        this->listEnd=travLimitEnd<sIdxMax ? travLimitEnd+1 : sIdxMax ;
        traverseContinue();
        this->listStart=prvTraverseLimitStart;
        this->listEnd=prvTraverseLimitEnd;
        return res;
    }
    if( trCommand==eTraverseDict || trCommand==eTraverseDictNot || trCommand==eTraverseDDict || trCommand==eTraverseDDictNot ) {
        if( ( (trCommand==eTraverseDict || trCommand==eTraverseSum || trCommand==eTraverseDDict ) && isMatch ) || ((trCommand==eTraverseDictNot || trCommand==eTraverseSumNot || trCommand==eTraverseDDictNot ) && !isMatch)) {

        if(trCommand == eTraverseSum || trCommand ==  eTraverseSumNot) {
                real * pSum=resultSummator.set(dictBuf.ptr(), dictBuf.length());
               real val;sRScanf(val,(const char *)vBodyPrev, valSizePrev,10 );
                (*pSum)+=val;
            } else {
        idx * pCnt=resultCounter.set(dictBuf.ptr(), dictBuf.length());
                ++(*pCnt);
        }
            if(this->debug){
                ::printf("%.*s[%" DEC "]",(int)dictBuf.length(),dictBuf.ptr(),*pCnt);
            }

            if(bigDicCumulator && valSizePrev && (trCommand==eTraverseDDict || trCommand==eTraverseDDictNot) ){

                idx ofs=bigDicBuffer.add((const char * )vBodyPrev,valSizePrev);
                sMex::Pos* pp=bigDicCumulator->set(dictBuf.ptr(), dictBuf.length());
                pp->pos=ofs;
                pp->size=valSizePrev;

                dictBuf.add0(2);
                idx ik=0;
                for( const char * pk=dictBuf.ptr(); pk ; pk=sString::next00(pk) ) {
                    keysForDic.resize(ik+1);
                    keysForDic[ik].set(pk,0);
                    ++ik;
                }

            }

            if(resultCumulator && valSizePrev && (trCommand==eTraverseDict || trCommand==eTraverseDictNot) ){
                sStr * pStr=resultCumulator->set(dictBuf.ptr(), dictBuf.length());
                if(pStr->length()){
                    if(*traverseFieldSeparator == 0){pStr->add0();}
                    else {pStr->add(traverseFieldSeparator,sLen(traverseFieldSeparator));}
                }
                pStr->add((const char * )vBodyPrev,valSizePrev);
                dictBuf.add0(2);
                idx ik=0;
                for( const char * pk=dictBuf.ptr(); pk ; pk=sString::next00(pk) ) {
                    keysForDic.resize(ik+1);
                    keysForDic[ik].set(pk,0);
                    ++ik;
                }
            }


        }
        traverseContinue();
        return res;
    }


    if( trCommand==eTraverseUnique  || trCommand==eTraverseUniqueNot  ) {
        idx * pCnt=uniqResultCounters.set(uniqBuf.ptr(), uniqBuf.length());
        ++(*pCnt);
        if(this->debug){
            ::printf("%.*s[%" DEC "]",(int)uniqBuf.length(),uniqBuf.ptr(),*pCnt);
        }
        if(*pCnt>relationTypeIndex)
            return res;
        traverseContinue();
        return res;
    }


    if(trCommand==eTraverseReturn || levelOperation>=maxLevelOperations || cntResults>=maxNumberResults){

        ++cntResults;
        if(traverseOutputting) {
            if(someInRecord) {
                if(traverseRecordSeparator){
                    if(*traverseRecordSeparator == 0){pTraverseBuf->add0();}
                    else{pTraverseBuf->addString(traverseRecordSeparator);}
                }
                someInRecord=false;
            }
        }
        if(this->debug){
            ::printf("\n\n");
        }

        if(callbackFunc)
            res=callCalbackFunc(curIon,this,shdr,stackResults.ptr(curResultCnt));


        return res;
    }


    if(trCommand==eTraverseJump || trCommand==eTraverseJumpNot ) {
        if((trCommand==eTraverseJump && isMatch) || (trCommand==eTraverseJumpNot && !isMatch))
        {
            if(debug)
                ::printf("ok");
            traverseJump( relationTypeIndex);

            return res;
        }else {
            if(debug)
                ::printf("!");
            traverseContinue();
            return res;
        }
    }





    sIon::Bucket searchBucket, rememberBucket0;
    searchBucket.toHash=&toHushBuf;
    sIon::RecordResult * pRecordResults=0;


    idx cntResultSet=0;
    const char * engineID=0;
    sIon::SearchElement searchElementList[1024];
    idx searchElementCnt=0;
    if ( trCommand<=eTraverseRelationHashBased || trCommand==eTraverseSearch )
    {

        if ( trCommand==eTraverseSearch ) {
            idx varCnt=(sizeInstruction-sizeof(StatementHeader)/sizeof(idx))/3;
            if(varCnt>sDim(searchElementList))
                varCnt=sDim(searchElementList);
            searchElementCnt=curIon->prepareSearchValset(relationTypeIndex, searchElementList, varCnt,(searchTrajectory+ofsTrajectory+1+sizeof(StatementHeader)/sizeof(idx)), 0,&(stackResults)) ;
            cntResultSet=1;
        } else {
            if(relationSearchEngineIndex==0) {
                curIon->getRelationBucketByHashVarg(&searchBucket,relationTypeIndex, relationHasherIndexType, &cntResultSet, (searchTrajectory+ofsTrajectory+1+sizeof(StatementHeader)/sizeof(idx)), 0,&(stackResults));
            } else {

                engineID=compileBuf.ptr(relationSearchEngineIndex);
                const char * containerLabels=engineID+sLen(engineID)+1;
                sDic < sMex > * ac;sIon_ensureAssociatedContainersIon( curIon , ac, pRelationType, containerLabels );

                if( memcmp(engineID,"range",6)==0) {
                    curIon->getRelationBucketByRange(&searchBucket,relationTypeIndex, engineID, &cntResultSet, (searchTrajectory+ofsTrajectory), 0,&(stackResults),ac);
                }
            }
        }
        if( trCommand==eTraverseDelete )
            rememberBucket0=searchBucket;


        if(pCnt)
            *pCnt=cntResultSet;
        if(  (trCommand == eTraverseFind && (!searchBucket.found() && checkOn==true) ) ||  (trCommand == eTraverseSearch && !searchElementCnt ) ) {
            if(this->debug && !searchBucket.found()){
                ::printf("!");
            }
            return res;
        }


        if( trCommand==eTraverseSearch || (searchBucket.found() || checkOn==false) ||  !transientCommand) {
            stackResults.add(1);
            pRecordResults=stackResults.ptr(curResultCnt);
            sIPrintf(intBuf, pRecordResults->size, cntResultSet, 10 );

            pRecordResults->body=intBuf;
            pRecordResults->index=0;
            pRecordResults->typeIndex=sNotIdx;
            pRecordResults->cType=sIon::eCTypeString;
        }

        if(trCommand==eTraverseCount ){
            traverseContinue();
            return res;
        }
        if(trCommand==eTraverseFind  || trCommand==eTraverseSearch ){
            stackResults.add(cntResultTargets);
        }
    }


    pRecordResults=stackResults.ptr(curResultCnt);
    ++levelOperation;

    if( checkOn==false && (!searchBucket.found()) ) {
        for ( idx ir=0; ir<1+cntResultTargets; ++ir)  {
            sIon::RecordResult * rr=pRecordResults+ir;
            rr->body="";
            rr->size=8;
            rr->cType=sIon::eCTypeString;
            rr->typeIndex=0;
            rr->index=0;
        }

        traverseContinue();
        return res;
    }


    ofsTrajectory+=sizeInstruction+1;
    idx inum=0, idbg=0;
    idx relationOfs=0,currentIRel=0, relOfsSearchNext=0;



    while( searchBucket.found() || trCommand==eTraverseSearch ) {


        idx * pRelationTargets=0;
        if(trCommand==eTraverseFind  || trCommand==eTraverseDelete || trCommand==eTraverseSearch) {
            if( relationSearchEngineIndex ==0 ) {
                if(trCommand==eTraverseSearch ) {
                    relationOfs=curIon->getNextRelationBySearchValset(pRelationType->typeIndex, searchElementList, searchElementCnt, &relOfsSearchNext, &currentIRel);
                    if(relationOfs==sNotIdx)
                        break;
                    relationOfs/=sizeof(unsigned int);
                } else {
                    sHash2 * relationHashContainer=&(curIon->typeContainers[pRelationType->typeIndex].relationHashContainer);
                    bool startFromLast=(inum==0  && relationHashContainer->reversed ) ? true: false;
                    relationOfs=relationHashContainer->bucketNextIndex(&(searchBucket.relationBucketPos),startFromLast);
                }
            }else  {
                if( memcmp(engineID,"range",6)==0) {
                    relationOfs=curIon->getRangeNextBucket(&searchBucket);
                } else {
                    relationOfs = 0;
                }
            }

            if(trCommand==eTraverseDelete) {
                curIon->deleteRelation(relationTypeIndex, relationOfs);
            }else {
                sMex * relationsArr=&(curIon->typeContainers[pRelationType->typeIndex].relationsArr);
                pRelationTargets=(idx *)relationsArr->ptr(relationOfs*sizeof(unsigned int));
            }
        }

        bool cancontinue=true;
        if(trCommand!=eTraverseDelete) {
            cancontinue = curIon->getRelationResultsByRelationTargets(relationTypeIndex, pRelationTargets, pRecordResults+1)>0 ? true : false ;
        }


        if(cancontinue) {
            if(inum<( shdr->limitStart >=0 ? shdr->limitStart : cntResultSet+shdr->limitStart ))
                cancontinue=false;
            else if(shdr->limitEnd ) {
                if( inum>= ( shdr->limitEnd >=0 ? shdr->limitEnd : cntResultSet+shdr->limitEnd ) ){
                    cancontinue=false;
                    res=0;
                }
            }
        }
        ++inum;

        if(cancontinue && callbackFunc)
            res=callCalbackFunc(curIon,this,shdr,stackResults.ptr(curResultCnt));


        if(this->debug){
            if(idbg)::printf("%s",debugStatementBuf.ptr(posStatementBuf));
            ++idbg;
            for ( idx ir=0; ir<1+cntResultTargets; ++ir)  {
                sIon::RecordResult * rr=pRecordResults+ir;
                idx valSize=rr->size;
                const char * body=(const char* ) rr->body;

                if(sConvPtr2Int(body)==sNotIdx-1) {
                    sIon::RecordResult * r=stackResults.ptr(valSize>=0 ? valSize  : (stackResults.dim()+valSize) );
                    body =(const char * ) r->body;
                    valSize=r->size;
                }
                if(rr->cType==sIon::eCTypeIdx || rr->cType==sIon::eCTypeIndexOnly )
                    ::printf("//%" DEC,*(idx*)body);
                else if( rr->cType==sIon::eCTypeIdxRange) {
                    ::printf("//%" DEC "-%" DEC,*(idx*)body>>32,*(idx*)body&0xFFFFFFFF);
                }
                else
                    ::printf("//%.*s",(int)valSize,body);
            }
            fflush(0);
        }

        if(cancontinue) {
            res*=traverse(0, listStart==-1 ? myListIndex : listStart, listEnd==-1 ? myListIndex+1 : listEnd );
            listCallingIon=myListIndex;
            pRecordResults=stackResults.ptr(curResultCnt);
        }

        if(!res)
            break;
    }

    if( trCommand==eTraverseDelete ) {
        curIon->deleteRelationBucket(relationTypeIndex,&rememberBucket0);
    }

    ofsTrajectory=curOfs;
    stackResults.cut(curResultCnt);
    --levelOperation;

    return res;

}



const char * sIonWander::traverseCompileReal(sIonRef * ionRef, const char * rules, idx len, sIO * errB, bool acceptInvalid)
{

    oriRules=rules;


    sIon * curIon=ionRef->ion;
    sVec < idx > * searchTrajectory=&(ionRef->SearchTrajectory);
    sVec < idx > * fixBodyPos=&(ionRef->fixBodyPos);

    if(!len)len=sLen(rules);
    sStr t1,t2,t3;

    sString::cleanMarkup(&t2,rules,len,"//" __,"\n" __," ",0,false,true,false);
    sString::searchAndReplaceSymbols(&t1,*t2.ptr() ? t2.ptr() : t2.ptr(1),0,sString_symbolsBlank,"",0,true,true,true,true,(idx)1);
    sString::searchAndReplaceSymbols(t1.ptr(),0,";",0,0,true,true,true,true,1);
    char rr[4]="-1\00";
    sVec <idx> hashTargets;

    idx *cntInstruction=0, cntResultsUpToPoint=0, cntInstructionOfs=0;

    resultIndexesIncrement=0;



    t2.cut(0);
    for(const char * ptr0=t1.ptr(); ptr0; ptr0=sString::next00(ptr0)) {
        bool invalid=false;

        t2.cut(0),t3.cut(0);
        const char * ptr=ptr0;
        if( (ptr[0]=='/' && ptr[1]=='/'))continue;

        char *label=0,* field, * value, * refer, *equ, * func , * dot, * args, * scope, * limit;
        idx iscope=0,limitStart=0,limitEnd=0;




        sString::searchAndReplaceSymbols(&t2,ptr,0,"()",0,0,true,true,true,true,2);

        func=t2.ptr(0);
        args=sString::next00(func);
        limit=sString::next00(args);



        equ=strchr(func,'=');
        if(equ){
            *equ=0;
            label=t2.ptr();
            func=equ+1;
        }
        dot=strchr(func,'.');
        if(dot) {
            *dot=0;
            ++dot;
        }
        scope=strchr(func,':');
        if( scope ) {
            *scope=0;
            iscope=0;
            idx hiscope=0;
            bool shift=0;
            bool negate=false;
            if(*func=='!') {
                negate=true;
                ++func;
            }
            while( func<scope ) {
                if(*func=='-')
                    shift=true;
                else {
                    if(shift)
                        hiscope=hiscope*(10)+(*func-'0');
                    else
                        iscope=iscope*(10)+(*func-'0');
                }

                ++func;
            }
            iscope=(hiscope<<32)|iscope;
            if(negate)iscope=-iscope;
            ++func;
        }
        if( limit && *limit) {
            ++limit;
            for( limitStart=0; isdigit(*limit) ; ++limit ) {
                limitStart=limitStart*(10)+(*limit-'0');
            }
            while( *limit!=0 && *limit!=':')
                ++limit;
            if(*limit)
                ++limit;
            idx sign=1;
            if(*limit=='-') {
                sign=-1;
                ++limit;
            }
            for( limitEnd=0; isdigit(*limit) ; ++limit ) {
                limitEnd=limitEnd*(10)+(*limit-'0');
            }
            limitEnd*=sign;
        }


        *(cntInstruction=searchTrajectory->add())=searchTrajectory->dim();
        cntInstructionOfs=cntInstruction-searchTrajectory->ptr();

        cntResultsUpToPoint=resultIndexesIncrement;
        idx labelOfs=-1;
        if(label){ labelOfs=compileBuf.length(); compileBuf.add(label); }

        sDic < idx > * functionIndexes=label ? resultIndexes.set(label) : resultIndexes.add();
        *functionIndexes->set(".")=searchTrajectory->dim()-1;

        idx relationTypeIndex=curIon->recordAndRelationTypesHash.find(sIon::eRARHash_relation,func,sLen(func)+1), lfnd=0;
        idx relationHasherIndex=0,relationSearchEngineIndex=0;
        idx trCommand=eTraverseFind;

        if(!relationTypeIndex) {
            if( (lfnd=sString::compareChoice(func,traverseCommands,&relationTypeIndex,false,0,true))==-1) {
                if(errB)errB->printf("%s\n wrong special command or relation type '%.*s'\n",ptr,(int)sLen(func),func);
                return ptr;
            }
            trCommand=relationTypeIndex;
            if(trCommand==eTraverseParseStop) {
                searchTrajectory->cut(*cntInstruction-1);
                break;
            }
            if(trCommand<=eTraverseRelationBased ) {
                relationTypeIndex=sNotIdx;
                if(dot)
                    relationTypeIndex=curIon->recordAndRelationTypesHash.find(sIon::eRARHash_relation,dot,sLen(dot)+1);
                if(relationTypeIndex<=0) {
                    if(errB)errB->printf("%s\nwrong relation type '%.*s'\n",ptr,(int)sLen(dot),dot);
                    if(!acceptInvalid)return ptr;
                    else invalid=true;
                }
                --relationTypeIndex;
            }
            else relationTypeIndex=0;

        }
        else --relationTypeIndex;


        idx * pRelationTargetsInHash=0, * pRelationTargets=0;
        if(trCommand<=eTraverseRelationBased ) {
            sIon::RelationType * pRelationType=curIon->relationTypesArr.ptr(relationTypeIndex);
            pRelationTargets=curIon->relationTargetsArr.ptr(pRelationType->ofsRelationTargets);
        }

        if(trCommand<=eTraverseRelationHashBased  || trCommand==eTraverseSearch ) {

            pRelationTargetsInHash=pRelationTargets;
            *functionIndexes->set("#")=resultIndexesIncrement;
            ++resultIndexesIncrement;
            while(*pRelationTargetsInHash!=sNotIdx) {
                const char * name=(const char * ) curIon->baseContainer.ptr((curIon->recordTypesArr.ptr(*pRelationTargetsInHash))->nameOfs);
                if( trCommand==eTraverseFind ||  trCommand==eTraverseSearch) {
                    *functionIndexes->set(name)=resultIndexesIncrement;
                    ++resultIndexesIncrement;
                }
                ++pRelationTargetsInHash;
            }
            ++pRelationTargetsInHash;
        }
        else {

            if(trCommand==eTraverseForeach ) {
                *functionIndexes->add()=resultIndexesIncrement;
                ++resultIndexesIncrement;
            }
            else if(trCommand==eTraverseSet) {
                *functionIndexes->set("#")=resultIndexesIncrement;
                ++resultIndexesIncrement;
            }
            relationTypeIndex=0;
            idx * refIndex=0;
            if(dot) {
                if( trCommand==eTraverseForeach ){
                    relationTypeIndex = curIon->recordAndRelationTypesHash.find(sIon::eRARHash_record,dot,sLen(dot)+1);
                    if(relationTypeIndex<0)
                        relationTypeIndex=0;

                } else if(trCommand==eTraverseJump || trCommand==eTraverseJumpNot ){
                    functionIndexes=resultIndexes.get(dot);

                    if(!functionIndexes){
                        relationTypeIndex=atoidx(dot);
                        *(forwardLabels.set(dot,0,&relationTypeIndex)->add())=searchTrajectory->dim();
                    }else
                        refIndex=functionIndexes->get(".");
                    if(refIndex)
                        relationTypeIndex=*refIndex;
                } else {
                    relationTypeIndex=atoidx(dot);
                }
            }
        }

        idx actualValidCommandOfs=searchTrajectory->dim();
        StatementHeader * hdr=(StatementHeader * )searchTrajectory->add(sizeof(StatementHeader)/sizeof(idx));
        hdr->limitStart=limitStart;hdr->limitEnd=limitEnd;
        hdr->scope=iscope;
        hdr->relationTypeIndex=relationTypeIndex|(trCommand<<32);
        hdr->relationHasherIndexType=0;
        if(labelOfs!=-1) {
            hdr->label=sConvInt2Ptr(labelOfs,const char);
            fixBodyPos->vadd(1,searchTrajectory->dim()-(sizeof(StatementHeader)-( sConvPtr2Int(&(hdr->label))-sConvPtr2Int(hdr) ))/sizeof(idx) );
        } else {
            hdr->label = 0;
        }


        hashTargets.cut(0);
        sString::searchAndReplaceSymbols(&t3,args,0,",",0,0,true,true,true,true);
        for(char * parval=t3.ptr(),*nxt=sString::next00(parval); parval; parval=nxt, nxt=sString::next00(parval)) {


            bool approx=false;
            field=parval;
            refer=0;
            equ=strchr(field,'=');
            if(!equ)equ=strchr(field,'~');
            value=0;
            if(equ){
                if(*equ=='~'){
                    approx=true;
                }

                *equ=0;++equ;
                if(*equ=='=' || *equ=='~') {
                    ++equ;
                }

                value=equ;
            }else {
                if(field[0]!='#') {
                    value=field;
                    field=0;
                } else value=field+sLen(field);
            }
            if(value) {
                char inquote=0;
                for ( equ=value; *equ; ++equ ) {
                    if(*equ=='\'' || *equ=='\"'){
                        if(inquote && inquote==*equ && equ[1]==0)
                            inquote=0;
                        else if(equ==value)
                            inquote=*equ;
                    }
                    if(!inquote && *equ=='.')
                        break;
                }
                if(*equ){
                    *equ=0;
                    refer=value;
                    value=equ+1;
                }
            }else {
                refer=rr;
                value=rr+2;
            }

            sIon::RecordType * pRecordType=0;
            idx regExValOfs=searchTrajectory->dim();
            if(field){
                idx ridx=curIon->recordAndRelationTypesHash.find(sIon::eRARHash_record,field,sLen(field)+1);
                if(!ridx){
                    if(field[0]=='#' ){
                        relationSearchEngineIndex=compileBuf.length();
                        if(value &&  *value)
                            compileBuf.add(field+1,value+sLen(value)-field);
                        else  {
                            compileBuf.add(field+1);
                            compileBuf.add0(2);
                        }
                        continue;

                    }else {
                        if(errB)errB->printf("%s\nwrong reference to a field '%.*s'\n",ptr,(int)sLen(field),field);
                        if(!acceptInvalid)return ptr;
                        else invalid=true;
                    }
                }
                --ridx;
                pRecordType=curIon->recordTypesArr.ptr(ridx);
                hashTargets.vadd(1,ridx);

                if(trCommand==eTraverseSearch ) {
                    idx iRR;
                    for ( iRR=0; pRelationTargets[iRR]!=sNotIdx && pRelationTargets[iRR]!=ridx ; ++iRR) {}
                    if( pRelationTargets[iRR]==sNotIdx) {
                        if(!acceptInvalid)return ptr;
                        else invalid=true;
                        iRR=sNotIdx;
                    }
                    searchTrajectory->vadd(1,iRR);
                    if(approx){
                        idx a=*searchTrajectory->ptr(regExValOfs);
                        *searchTrajectory->ptr(regExValOfs)=a|(0x80000000);
                    }
                }

            }


            idx val=0;
            bool reverse=true;
            if(refer){
                idx isSerialOnly=0;
                idx * refIndex=0;
                sDic < idx > * reffunctionIndexes=0;

                if(refer[0]=='-'){
                    reverse=false;++refer;
                }
                if(strcmp(refer,"$")==0) {
                    reffunctionIndexes=resultIndexes.ptr(resultIndexes.dim()-1);
                }
                else
                    reffunctionIndexes=resultIndexes.get(refer);
                if(reffunctionIndexes) {
                    if(value[0]=='#' && value[1]!=0 ) {
                        isSerialOnly=2;
                    }
                    refIndex=reffunctionIndexes->get(value);
                }
                if(!refIndex){

                    if(value[0]>='0' && value[0]<='9' ){
                        val=atoidx(value);
                        refIndex=reffunctionIndexes->ptr(val);
                    }else {
                        *(value-1)='.';
                        value=refer;
                        refer=0;
                    }
                }
                if(refIndex) {
                    val=*refIndex;
                    if(!reverse)
                        searchTrajectory->vadd(2,sNotIdx-isSerialOnly,val);
                    else
                        searchTrajectory->vadd(2,sNotIdx-isSerialOnly,-(cntResultsUpToPoint-val));
                }
            }


            if(!refer){
                idx vpos=compileBuf.length();

                sDic < idx > * labelIndex =resultIndexes.get(value);
                idx * labelOfs=labelIndex ? labelIndex->get(".") : 0 ;
                if(labelOfs) {
                    if(!reverse) {
                        searchTrajectory->vadd(2,sNotIdx-1,*labelOfs);
                    } else {
                        searchTrajectory->vadd(2,sNotIdx-1,-(cntInstructionOfs-(*labelOfs)));

                    }
                } else {
                    idx lastLen=sLen(value);
                    if(value[0]!='$' && pRecordType && pRecordType->cType==sIon::eCTypeReal ) {
                        real r; sRScanf( r , value , lastLen, 10 );
                        compileBuf.add((const char*)&r,(lastLen=sizeof(r)));
                    }else if( value[0]!='$' && pRecordType && ( pRecordType->cType==sIon::eCTypeIdx || pRecordType->cType==sIon::eCTypeIndexOnly || pRecordType->cType==sIon::eCTypeIdxRange ))  {
                        idx r; sIHiLoScanf( r , value , lastLen, 10 );
                        compileBuf.add((const char*)&r,(lastLen=sizeof(r)));
                    } else {
                        sString::cleanEnds(&compileBuf,value,0,"\"\'",true,0);
                        value=compileBuf.ptr(vpos);
                        lastLen=sLen(value);
                    }
                    fixBodyPos->vadd(1,searchTrajectory->dim());
                    if(value[0]=='$')
                        *parametricArguments.set(value,lastLen)=searchTrajectory->dim();
                    searchTrajectory->vadd(2,vpos,lastLen);

                    if(trCommand==eTraverseSearch && strncmp(value,"regex:",6)==0) {
                        idx ofsReg=curIon->regexpList.add(0,sizeof(regex_t));
                        regcomp((regex_t*)curIon->regexpList.ptr(ofsReg), value+6, REG_EXTENDED|REG_ICASE);
                        idx iRR=*searchTrajectory->ptr(regExValOfs);
                        iRR|=(ofsReg+1)<<32;
                        *searchTrajectory->ptr(regExValOfs)=iRR;
                    }
                }

            }

            if(trCommand==eTraverseSet) {
                *functionIndexes->add()=resultIndexesIncrement;
                ++resultIndexesIncrement;
            }
        }

        cntInstruction=searchTrajectory->ptr(cntInstructionOfs);


        if( (!relationSearchEngineIndex) && trCommand<=eTraverseRelationHashBased){
            bool found=0;
            relationHasherIndex=0;
            for ( idx i=0, j=0; pRelationTargetsInHash[i+j]!=sNotIdx; ++relationHasherIndex, i+=j, j=0) {
                for ( j=0 ; j<hashTargets.dim() && pRelationTargetsInHash[i+j]!=sNotIdx; ++j ) {
                    if(hashTargets[j]!=pRelationTargets[pRelationTargetsInHash[i+j]])
                        break;
                }
                if(j==hashTargets.dim() && pRelationTargetsInHash[i+j]==sNotIdx ) {
                    found=true;
                    break;
                }
                while( pRelationTargetsInHash[i+j]!=sNotIdx)
                    ++j;
                ++j;
            }
            if(!found){
                if(errB)errB->printf("%s\nwrong hasher type referred\n",ptr);
                if(!acceptInvalid)return ptr;
                else invalid=true;
            }


        }
        *(cntInstruction+2)=(relationSearchEngineIndex<<32)|relationHasherIndex;

        *cntInstruction=(searchTrajectory->dim()-(*cntInstruction));
        if(invalid)
            *searchTrajectory->ptr(actualValidCommandOfs)=((idx)eTraverseSkipOperation<<32);
    }

    searchTrajectory->vadd(1,sizeof(StatementHeader)/sizeof(idx));
    StatementHeader * hdr=(StatementHeader * )searchTrajectory->add(sizeof(StatementHeader)/sizeof(idx));
    hdr->limitStart=0;hdr->limitEnd=0;
    hdr->scope=0;
    hdr->relationTypeIndex=((idx)eTraverseReturn)<<32;
    hdr->relationHasherIndexType=0;

    idx labelOfs=compileBuf.length();compileBuf.add("_return");
    hdr->label=sConvInt2Ptr(labelOfs,const char );
    fixBodyPos->vadd(1,searchTrajectory->dim()-(sizeof(StatementHeader)-( sConvPtr2Int(&(hdr->label))-sConvPtr2Int(hdr) ))/sizeof(idx) );




    for ( idx i =0,size ; i<forwardLabels.dim() ; ++i) {
        const char * label=(const char * )forwardLabels.id(i,&size);
        sDic < idx > * functionIndexes=resultIndexes.get(label,size);
        if(!functionIndexes)
            continue;
        idx * refIndex=functionIndexes->get(".");
        if(!refIndex)
            continue;
        sVec < idx > * v=forwardLabels.ptr(i);
        for (idx r=0; r<v->dim(); ++r) {
            idx * prCommand=searchTrajectory->ptr(*v->ptr(r));
            idx trCommand=(*prCommand)>>32;
            *prCommand=(*refIndex)|(trCommand<<32);
        }

    }

    return 0;
}

const char * sIonWander::printPrecompiledReal(sIonWander::sIonRef * ionRef, sStr * buf)
{
    sIon * curIon=ionRef->ion;
    sVec < idx > * searchTrajectory=&(ionRef->SearchTrajectory);

    idx ic=0,c=0;
    idx scope=0;
    StatementHeader * shdr = 0;
    for(idx is=0; is<searchTrajectory->dim() ; ++is , ++ic) {
        if(ic>c){
            buf->printf(")");
                buf->printf("[%" DEC ":%" DEC "]",shdr->limitStart, shdr->limitEnd);
            buf->printf("\n");
            ic=0;
        }
        if(ic==0) {
            c=(*searchTrajectory)[is];
        }
        else if(ic==1){
            shdr=(StatementHeader *)searchTrajectory->ptr(is);
            scope=shdr->scope;
            idx trCommand=shdr->relationTypeIndex>>32;
            const char * trCommandText=sString::next00(traverseCommands,trCommand);
            idx loWord=shdr->relationTypeIndex&0xFFFFFFFF;
            const char * secondWord="";
            if( trCommand <= eTraverseRelationBased) {
                sIon::RelationType * pRelationType=curIon->relationTypesArr.ptr(loWord);
                secondWord=(const char * )curIon->baseContainer.ptr(pRelationType->nameOfs);
            }else if(trCommand==eTraverseForeach) {
                if(loWord>0 && loWord<=curIon->recordTypesArr.dim()) {
                    sIon::RecordType * pRecordType=curIon->recordTypesArr(loWord-1);
                    secondWord=(const char * )curIon->baseContainer.ptr(pRecordType->nameOfs);
                }
            }
            if(scope)
                buf->printf("0x%llx:",scope);
            buf->printf("%s.%s[%" DEC ":%" DEC "]", trCommandText, secondWord,trCommand,loWord);

            idx relationSearchEngineIndex=shdr->relationHasherIndexType;
            idx relationHasherIndex=relationSearchEngineIndex&0xFFFFFFFF;
            buf->printf(":[%" DEC ":%" DEC "] (", relationSearchEngineIndex,relationHasherIndex);

            ic+=sizeof(StatementHeader)/sizeof(idx)-1;
            is+=sizeof(StatementHeader)/sizeof(idx)-1;
            continue;
        }
        {
            buf->printf("%" DEC "%s", (*searchTrajectory)[is],ic==c ? "" : ",");
        }

    }
    buf->printf(")\n");


    return buf->ptr(0);
}


const char * sIonWander::traverseView(void)
{
    idx pos=pTraverseBuf->length();
    for( idx id=0,valsize=0;id<resultCounter.dim(); ++id) {
        const void * val=resultCounter.id(id,&valsize);
        pTraverseBuf->add((const char* )val,valsize);
        pTraverseBuf->printf("%s%" DEC "%s",traverseFieldSeparator ? traverseFieldSeparator : " ",resultCounter[id],traverseRecordSeparator);
    }
    return pTraverseBuf->ptr(pos);

}

const char * sIonWander::traverseViewVal(void)
{
    if(!resultCumulator)return 0;
    idx pos=pTraverseBuf->length();
  

    for( idx id=0,valsize=0;id<resultCumulator->dim(); ++id) {
        const void * val=resultCumulator->id(id,&valsize);
        sStr * pStr=resultCumulator->ptr(id);
        pTraverseBuf->add((const char* )val,valsize);
        if(!pStr->length())continue;
        if(traverseFieldSeparator && *traverseFieldSeparator)
            pTraverseBuf->add(traverseFieldSeparator,1);
        pTraverseBuf->add(pStr->ptr(),pStr->length());
        if(traverseRecordSeparator && *traverseRecordSeparator)
            pTraverseBuf->add(traverseRecordSeparator,1);

    }

    return pTraverseBuf->ptr(pos);

}

const char * sIonWander::traverseViewValTbl(void){
    if(!resultCumulator)return 0;
    if (!keysForDic.dim() || keysForDic.dim() <2) return 0;
    idx pos=pTraverseBuf->length();
    for( idx id=0,valsize=0;id<keysForDic[1].dim(); ++id) {
        if(traverseFieldSeparator && *traverseFieldSeparator)
            pTraverseBuf->add(traverseFieldSeparator,1);
        const void * val=keysForDic[1].id(id,&valsize);
        pTraverseBuf->add((const char *)val,valsize);
    }
    if(traverseRecordSeparator&& *traverseRecordSeparator){pTraverseBuf->add(traverseRecordSeparator,1);}

    sStr buf;
    for( idx id=0,valsize=0;id<keysForDic[0].dim(); ++id) {
        const void * val=keysForDic[0].id(id,&valsize);
        pTraverseBuf->add((const char *)val,valsize);
        buf.cut(0);
        buf.add((const char *)val,valsize);
        idx vv=valsize+1;
        buf.add0(1);
    for( idx ii=0,valsize=0;ii<keysForDic[1].dim(); ++ii) {

        if(traverseFieldSeparator && *traverseFieldSeparator)
                pTraverseBuf->add(traverseFieldSeparator,1);

            val=keysForDic[1].id(ii,&valsize);
            buf.add((const char*)val,valsize);

            sStr * pStr=resultCumulator->get(buf.ptr(),buf.length());
            buf.cut(vv);
        if(!pStr || !pStr->length())continue;


        pTraverseBuf->add("\"",1);
        pTraverseBuf->add(pStr->ptr(),pStr->length());
        pTraverseBuf->add("\"",1);
    
        }

        if(traverseRecordSeparator&& *traverseRecordSeparator)
                pTraverseBuf->add(traverseRecordSeparator,1);

    }
    return pTraverseBuf->ptr(pos);
}

const char * sIonWander::traverseViewBigDic2D(bool quotes)
{
    if(!bigDicCumulator || keysForDic.dim()<2)return 0;
    idx pos=pTraverseBuf->length();

    for( idx id=0,valsize=0;id<keysForDic[1].dim(); ++id) {
        if(traverseFieldSeparator && *traverseFieldSeparator)
            pTraverseBuf->add(traverseFieldSeparator,1);
        const void * val=keysForDic[1].id(id,&valsize);
    if (quotes)pTraverseBuf->add("\"",1);
        pTraverseBuf->add((const char *)val,valsize);
    if (quotes)pTraverseBuf->add("\"",1);
    }
    if(traverseRecordSeparator&& *traverseRecordSeparator)
        pTraverseBuf->add(traverseRecordSeparator,1);

    sStr buf;
    for( idx id=0,valsize=0;id<keysForDic[0].dim(); ++id) {
        const void * val=keysForDic[0].id(id,&valsize);
    if (quotes)pTraverseBuf->add("\"",1);
        pTraverseBuf->add((const char *)val,valsize);
    if (quotes)pTraverseBuf->add("\"",1);
        buf.cut(0);
        buf.add((const char *)val,valsize);
        idx vv=valsize+1;
        buf.add0(1);
        for( idx ii=0,valsize=0;ii<keysForDic[1].dim(); ++ii) {

            if(traverseFieldSeparator && *traverseFieldSeparator)
                pTraverseBuf->add(traverseFieldSeparator,1);

            val=keysForDic[1].id(ii,&valsize);
            buf.add((const char*)val,valsize);

            sMex::Pos * pps=bigDicCumulator->get(buf.ptr(),buf.length());
            buf.cut(vv);
            if(!pps)
                continue;

            const void * ptr=bigDicBuffer.ptr(pps->pos);
        if (quotes)pTraverseBuf->add("\"",1);
            pTraverseBuf->add((const char *)ptr,pps->size);
        if (quotes)pTraverseBuf->add("\"",1);

        }

        if(traverseRecordSeparator&& *traverseRecordSeparator)
                pTraverseBuf->add(traverseRecordSeparator,1);
    }

    return pTraverseBuf->ptr(pos);

}

sIonWander::TraverseParamReference * sIonWander::getSearchArgumentPointer(idx serno, idx ionNum)
{
    idx pos=0;
    for(idx i=0; i<serno; ++i){
        pos+=ionList[ionNum].SearchTrajectory[pos]+1;
    }
    return (TraverseParamReference *)(ionList[ionNum].SearchTrajectory+pos+1+sizeof(StatementHeader)/sizeof(idx));
}

idx sIonWander::setSearchTemplateVariable(const char * templateName, idx templateLen, const void * value, idx valSize)
{
    idx cntDone=0;
    for(idx i=0; i<ionList.dim() ; ++i ) {
        TraverseParamReference * tplt=getSearchDictionaryPointer(templateName, templateLen ? templateLen : sLen(templateName), i);
        if(!tplt)continue;
        tplt->body=(void *)value;
        tplt->size=(valSize || sConvPtr2Int(value)<0) ? valSize : sLen(value);
        ++cntDone;
    }
    return cntDone;
}
sIonWander::TraverseParamReference * sIonWander::getSearchDictionaryPointer(const char *serno, idx serlen, idx ionNum)
{
    if (!serlen){
        serlen=sLen(serno);
    }
    idx * ptre=parametricArguments.get(serno,serlen);
    if(!ptre)return 0;
    idx tra = *ptre;
    return (TraverseParamReference *)(ionList[ionNum].SearchTrajectory.ptr(0) + tra);
}

sIonWander::TraverseParamReference * sIonWander::getSearchDictionaryPointer(idx serial, idx ionNum)
{
    idx * ptre=parametricArguments.ptr(serial);
    idx tra = *ptre;
    return (TraverseParamReference *)(ionList[ionNum].SearchTrajectory.ptr(0) + tra);
}

idx sIonWander::retrieveParametricWander(sVar * pForm, idx ionNum)
{
    idx len,notfound=0,siz;
    for(idx ip=0; ip<parametricArguments.dim(); ++ip ) {
        const char * par=(const char * )parametricArguments.id(ip,&len);
        const char * val=pForm->value(par+1,0,&siz, len-1);
        if(!val) {
            ++notfound;
            continue;
        }
        idx * ptre=parametricArguments.ptr(ip);
        idx tra = *ptre;
        idx startIn= (ionNum==sNotIdx) ? 0 : ionNum;
        idx endIn= (ionNum==sNotIdx) ? ionList.dim() : ionNum+1;
        for( idx iN=startIn; iN<endIn ; ++iN) {
            TraverseParamReference * p=(TraverseParamReference *)(ionList[iN].SearchTrajectory.ptr(0) + tra);
            p->body=(void *) val;
            p->size=sLen(val);
        }
    }
    return notfound;
}




const char * sIonWander::printPrecompiled(sStr * buf)
{
    const char * res=0;
    for(idx i=0; i<ionList.dim() ; ++i ) {
        buf->printf("-------- %" DEC " --------\n",i);
        res=printPrecompiledReal(ionList.ptr(i), buf );
    }
    return res;
}

idx sIonWander::traverse(idx start, idx iListStart, idx iListEnd)
{
    if(transactionFile.ok()){
        transactionFile.vadd(2, sIonTransaction::eTransactionTraverse,start);
        return 0;
    }
    bool prvContinueScope=continueScope;
    idx res=0;

    for(idx i=iListStart; !breakMode && i<ionList.dim() && i<iListEnd ; ++i) {
        sIonRef * iref=ionList.ptr(i);
        if(iref->precompBuf) {
            traverseCompileReal(iref, iref->precompBuf, iref->precompBufLen, 0, true );
            for(idx i = 0 ; i< iref->fixBodyPos.dim() ; ++i) {
                idx * vpos=iref->SearchTrajectory.ptr(iref->fixBodyPos[i]);
                const void * b=compileBuf.ptr(*vpos);
                *vpos=sConvPtr2Int(b);
            }
            iref->precompBuf=0;
            iref->precompBufLen=0;
        }

        StatementHeader * shdr=(StatementHeader * )((idx*)iref->SearchTrajectory.ptr(ofsTrajectory+1));
        idx ionSerial=iref->ionSerial-1;

        if( shdr->scope!=0 ) {
            bool negate=false;
            idx iscope=shdr->scope;
            if(iscope<0){iscope=-iscope;negate=true;}
            idx hiscope=(iscope>>32)&0xFFFFFFFF;
            iscope&=0xFFFFFFFF;
            bool isok=false;
            if(hiscope==0 && iscope-1==ionSerial)
                isok=true;
            else if(iscope-1<=ionSerial && ionSerial<=hiscope-1)
                isok=true;
            if(negate)
                isok=!isok;
            if(!isok)
                continue;
        }
        continueScope=false;

        idx cmd=(shdr->relationTypeIndex>>32);
        if((cmd>eTraverseRelationHashBased && cmd!=eTraverseForeach) && i!=listCallingIon)
            ;
        else
            res+=traverseReal(iref,start, i );
        if(continueScope)
        continue;
    }
    continueScope=prvContinueScope;
    return res;
}


const char * sIonWander::traverseCompile(const char * rules, idx len, sIO * errb, bool acceptInvalid, bool lazyCompile)
{

    if(!len)len=sLen(rules);
    if(transactionFile.ok()){
        transactionFile.vadd(3, sIonTransaction::eTransactionTraverseCompile,len,acceptInvalid ? 1 : 0 );
        idx b= (len/sizeof(idx)); if(len%sizeof(idx))b+=1;
        memcpy((void*)transactionFile.add(b),(const void*)rules,len);
        return 0;
    }

    const char * res=0;
    idx l,lerr;
    for(idx i=0; i<ionList.dim() ; ++i ) {
        if(errb){
            l=errb->length();
            if(!errb->_funcCallback)
                errb->printf("-------- %" DEC " --------\n",i);
            lerr=errb->length();
        }
        sIonRef * iref=ionList.ptr(i);
        if(lazyCompile){
            iref->precompBuf=rules;
            iref->precompBufLen=len;
        }
        else {
            iref->precompBuf=0;
            iref->precompBufLen=0;
            res=traverseCompileReal(ionList.ptr(i), rules, len, errb, acceptInvalid );
        }

        if(errb && errb->length()==lerr)
            errb->cut(l);

        if(!acceptInvalid && res)
            break;
    }
    updateCompileBufPointers();

    if(errb && errb->length())
        errb->add0(1);
    return res;
}


void sIonWander::updateCompileBufPointers (void)
{
    sIonRef * ionRef = 0;
    for(idx l=0; l<ionList.dim(); ++l) {
        ionRef = ionList.ptr(l);
        for(idx i = 0 ; i< ionRef->fixBodyPos.dim() ; ++i) {
            idx * vpos=ionRef->SearchTrajectory.ptr(ionRef->fixBodyPos[i]);
            const void * b=compileBuf.ptr(*vpos);
            *vpos=sConvPtr2Int(b);
        }
    }
}

void sIonWander::resetCompileBuf (void)
{
    for(idx i=0; i<ionList.dim(); ++i) {
        ionList[i].SearchTrajectory.cut(0);
        ionList[i].fixBodyPos.cut(0);
    }
    parametricArguments.empty();
    compileBuf.cut(0);
    pTraverseBuf->cut(0);
    forwardLabels.empty();
    cntResults=0;
    levelOperation=0;
    someInRecord = 0;
    traverseFieldSeparator=",";
    traverseRecordSeparator="\n";
    toHushBuf.cut(0);

    uniqBuf.empty();
}


void sIonWander::resetResultBuf (void)
{
    pTraverseBuf->cut(0);
    cntResults=0;
    someInRecord = 0;
    uniqResultCounters.empty();
    uniqBuf.cut(0);
    dictBuf.empty();
    toHushBuf.cut(0);
}





