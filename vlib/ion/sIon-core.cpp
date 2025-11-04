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
#include <slib/core/word.hpp>

const idx sIon::sizeofIdx=(idx)sizeof(idx);
const void * sIon::Link_BodyByIndex=sNotPtr-1;




sIon * sIon::init(const char * lBaseName, idx fileOpenMode)
{

    if(recordTypesArr.ok() && recordTypesArr.dim()){
        destroy();
    }
    if(!lBaseName)return this;

    baseName.printf("%s",lBaseName);
    baseName.add0(2);
    extension=baseName.printf(".ion");
    openMode=fileOpenMode;


    sStr flnm(sMex::fExactSize);flnm.printf("%s",baseName.ptr());
    idx flen=flnm.length();
    flnm.printf("%s",extension);

    if(!(openMode&sMex::fReadonly)) {
        baseContainer.init(flnm,openMode|sMex::fSetZero|sMex::fBlockCompact);
        if(!baseContainer.pos())
            baseContainer.add(0,sizeof(Header));
    } else {
        baseContainer.init(flnm,openMode|sMex::fSetZero|sMex::fBlockCompact);
    }


    flnm.printf(flen,"-#-recordTypes%s",extension);
    recordTypesArr.init(flnm,openMode|sMex::fSetZero|sMex::fBlockCompact);

    flnm.printf(flen,"-#-relationTypes%s",extension);
    relationTypesArr.init(flnm,openMode|sMex::fSetZero|sMex::fBlockCompact);

    flnm.printf(flen,"-#-relationTargetTypes%s",extension);
    relationTargetsArr.init(flnm,openMode|sMex::fBlockCompact);

    flnm.printf(flen,"-#-recordAndRelationTargetTypesHash%s",extension);
    recordAndRelationTypesHash.init(flnm,openMode|sMex::fBlockCompact);
    recordAndRelationTypesHash.keyParam=this;
    recordAndRelationTypesHash.keyfunc=getRecordTypeKeyFunction;


    idx cntRecordTypes=recordTypesArr.dim();
    typeContainers.init(sMex::fSetZero);
    if(cntRecordTypes){
        typeContainers.resize(cntRecordTypes);
    }

    return this;
}

void sIon::expect(const char * record_or_relationName, idx size)
{
    configuration[record_or_relationName].sizeExpectation=size;
}


idx sIon::addRecordType(const char * typeName,sIon::eCType cType,sIon::eHashType hashType, const char * tvs)
{

    idx recordTypeIndex=recordAndRelationTypesHash.find(eRARHash_record,typeName,sLen(typeName)+1);
    if( recordTypeIndex ){
        --recordTypeIndex;
        return recordTypeIndex;
    }
    RecordType * pRecordType=recordTypesArr.add(1);

    pRecordType->typeIndex=recordTypesArr.dim()-1;
    pRecordType->typeIndexContainer=pRecordType->typeIndex;
    pRecordType->cType=cType;
    pRecordType->hashType=hashType;
    pRecordType->nameOfs=baseContainer.add(typeName,sLen(typeName)+1);
    pRecordType->nameOfsContainer=pRecordType->nameOfs;
    pRecordType->typeValueSynonyms00=0;
    if(tvs && *tvs) {
        pRecordType->typeValueSynonyms00=baseContainer.add(tvs,sString::size00(tvs));
        baseContainer.add(__,1);
    }


    recordAndRelationTypesHash.map(pRecordType->typeIndex,eRARHash_record,typeName,sLen(typeName)+1);

    typeContainers.resize(pRecordType->typeIndex+1);

    return pRecordType->typeIndex;
}

void sIon::aliasRecordTypes(idx dstRecordTypeIndex,idx srcRecordTypeIndex)
{
    RecordType * pSrcRecordType=recordTypesArr(srcRecordTypeIndex);
    RecordType * pDstRecordType=recordTypesArr(dstRecordTypeIndex);
    pSrcRecordType->typeIndexContainer=pDstRecordType->typeIndexContainer;
    pSrcRecordType->nameOfsContainer=pDstRecordType->nameOfsContainer;
    pSrcRecordType->typeIndexAlias=pDstRecordType->typeIndex+1;
}

void * sIon::getRecordTypeKeyFunction(void * param, sHash2::hashtypeIdx typeIndexAs0, idx recordIndex,idx * pRecordTypeNameSize, sMex * body)
{
    sIon * ion=(sIon *) param;

    void * typeName;
    if(typeIndexAs0==eRARHash_record){
        RecordType * pRecordType=ion->recordTypesArr(recordIndex);
        typeName = ion->baseContainer.ptr(pRecordType->nameOfs);
    }else {
        RelationType * pRelationType=ion->relationTypesArr.ptr(recordIndex);
        typeName=ion->baseContainer.ptr(pRelationType->nameOfs);
    }

    if(pRecordTypeNameSize)
        *pRecordTypeNameSize=sLen((const char*)typeName)+1;
    return typeName;
}


idx sIon::autoNumber(idx recordTypeIndex, idx range)
{
    if(range<=0)return sNotIdx;

    RecordType * pRecordType=recordTypesArr.ptr(recordTypeIndex);
    idx toReturn=pRecordType->autoIncrement;
    do {
        pRecordType->autoIncrement+=range;
    }while(pRecordType->autoIncrement!=toReturn+range);
    return toReturn;

}


idx sIon::addRelationTypeVarg(idx relationTypeFlags , const char * relationName, const char * * markerArr, va_list marker, idx directCnt)
{
    idx relationTypeIndex=recordAndRelationTypesHash.find(eRARHash_relation,relationName,sLen(relationName)+1);
    if( relationTypeIndex ){
        --relationTypeIndex;
        return relationTypeIndex;
    }

    RelationType * pRelationType=relationTypesArr.add(1);

    pRelationType->typeIndex=relationTypesArr.dim()-1;
    pRelationType->typeIndexContainer=pRelationType->typeIndex;
    pRelationType->nameOfs=baseContainer.add(relationName,sLen(relationName)+1);
    pRelationType->nameOfsContainer=pRelationType->nameOfs;
    pRelationType->ofsRelationTargets=relationTargetsArr.dim();
    pRelationType->cntRelationTargets=0;
    pRelationType->cntRelationHashTypes=0;
    pRelationType->flags=0;
    pRelationType->cntRelationsAll=0;
    pRelationType->flags=relationTypeFlags&(sFlag(RelationType::bRelationReverseLookup));

    recordAndRelationTypesHash.map(pRelationType->typeIndex,eRARHash_relation,relationName,sLen(relationName)+1);

    idx iBlock=0;
    const char * vc=0,* firstChoiceSet=0;
    do{
        if(directCnt) {
            if(iBlock>=directCnt)
                break;
            relationTargetsArr.vadd(1,((idx*)markerArr)[iBlock]);
        } else {
            if(markerArr)
                vc=markerArr[iBlock];
            else
                vc=va_arg(marker,const char * );
            if(!vc)
                break;

            if(iBlock==0)
                firstChoiceSet=vc;


            for ( const char * t=vc; *t; t=t+sLen(t)+1 ) {

                idx recordTypeIndex=-1;
                if(iBlock==0){
                    recordTypeIndex=recordAndRelationTypesHash.find(eRARHash_record,t,sLen(t)+1);
                } else {
                    if(sString::compareChoice( t ,firstChoiceSet,&recordTypeIndex,false,1,true)==-1) {
                        if( t[0]=='#' ){
                            if(sString::compareChoice(t+1 ,firstChoiceSet,&recordTypeIndex,false,1,true)==-1)
                                break;
                            recordTypeIndex|=fRelationTypeIndex;
                        }
                    }
                }
                if(recordTypeIndex<=0)
                    break;
                --recordTypeIndex;
                relationTargetsArr.vadd(1,recordTypeIndex);
            }


            if(iBlock==0) {
                pRelationType->cntRelationTargets=relationTargetsArr.dim()-pRelationType->ofsRelationTargets;
            }

            relationTargetsArr.vadd(1,(idx)-1);
        }
        ++iBlock;
    }while(vc || directCnt );

    relationTargetsArr.vadd(1,(idx)-1);

    pRelationType->cntRelationHashTypes=relationTargetsArr.dim()-pRelationType->ofsRelationTargets;

    return pRelationType->typeIndex;
}




idx sIon::getRecordByHash(sIon::RecordType * pRecordType, const void * recordBody, idx recordSize , idx * pHash)
{
    idx recordIndex=0;
    sIon::Record * pRecord=0;

    if(pRecordType->cType==eCTypeIndexOnly)
        return *(idx * )recordBody;
    sVec < Record> * recordsArr=0;sIon_ensureContainer(pRecordType,recordsArr);


    switch ( pRecordType->hashType ) {

        case sIon::eHashTypeIdx:{
            sIndex< sIndexStruc> * recordIndexContainer;sIon_ensureContainer(pRecordType,recordIndexContainer);
            recordIndex=recordIndexContainer->find(*((udx*)recordBody),(udx * )pHash);
        }break;

        case sIon::eHashTypeOther:
        case sIon::eHashTypeString:{
            sHash2 * recordHashContainer;sIon_ensureHashContainer(pRecordType,recordHashContainer,this,getRecordKeyFunction,0);
            recordIndex=recordHashContainer->find(pRecordType->typeIndex,recordBody,recordSize, 0,pHash);
        }break;


        case sIon::eHashTypeOrdered:{
            *pHash=0;
            sMex * recordBodyContainer;sIon_ensureContainer(pRecordType,recordBodyContainer);
            idx cntRecordBatches=recordsArr->dim(),res=0;
            recordIndex = cntRecordBatches/2;
            for(idx left=0, right=cntRecordBatches-1;right>left; recordIndex=(left+right)/2 )
            {
                pRecord=recordsArr->ptr(recordIndex);
                void * curRecordBody=(pRecord->size<=(idx)sizeof(pRecord->ofs)) ?
                    (void*)&(pRecord->ofs) :
                    recordBodyContainer->ptr(pRecord->ofs);

                res=0;
                switch ( pRecordType->cType ) {

                    case eCTypeIdxRange:
                    case eCTypeIdx:
                        if( *((idx*)curRecordBody) > *((idx*)recordBody) )
                            res=1;
                        else if( *((idx*)curRecordBody) > *((idx*)recordBody) )
                            res=-1;
                        break;

                    case eCTypeReal:
                        if( *((real*)curRecordBody) > *((real*)recordBody) )
                            res=1;
                        else if( *((real*)curRecordBody) > *((real*)recordBody) )
                            res=-1;
                        break;

                    case eCTypeString:
                    case eCTypeOther:
                    default:
                        idx cmpSize=sMin(recordSize,pRecord->size);
                        res=memcmp((const char * )curRecordBody,(const char * )recordBody,cmpSize);
                        break;
                }


                if(res==0)
                    break;
                if(res>0)
                    left=recordIndex;
                if(res<0)
                    right=recordIndex;
            }

            if(res)
                return 0;
            else
                ++recordIndex;
        }break;
        default:
            break;
    };

    if(!recordIndex)
        return sNotIdx;
    else
        --recordIndex;

    return recordIndex;
}

idx sIon::setRecordHash(sIon::RecordType * pRecordType, const void * recordBody, idx recordSize, idx recordIndex)
{


    switch ( pRecordType->hashType ) {

        case sIon::eHashTypeIdx:{
            sIndex< sIndexStruc> * recordIndexContainer;sIon_ensureContainer(pRecordType,recordIndexContainer);
                recordIndexContainer->add(*((udx*)recordBody),(sIndexStruc *)(&recordIndex));
            Configuration * cfg=configuration.get((const char *)baseContainer.ptr((pRecordType)->nameOfsContainer));
            if(cfg) recordIndexContainer->reserve(cfg->sizeExpectation);
        }break;

        case sIon::eHashTypeOther:
        case sIon::eHashTypeString:{
            sMex * recordBodyContainer;sIon_ensureContainer(pRecordType,recordBodyContainer);
            sHash2 * recordHashContainer;sIon_ensureHashContainer(pRecordType,recordHashContainer,this,getRecordKeyFunction,0);
                recordHashContainer->map(recordIndex, pRecordType->typeIndex,recordBody,recordSize);
                if( recordHashContainer->mapCount()==1 ){
                    Configuration * cfg=configuration.get((const char *)baseContainer.ptr((pRecordType)->nameOfsContainer));
                    if(cfg) recordHashContainer->rehash(cfg->sizeExpectation);
                }

        }break;
        default:
            break;
    };

    return recordIndex;
}

void * sIon::getRecordKeyFunction(void * param, sHash2::hashtypeIdx recordTypeIndex, idx recordIndex,idx * pRecordSize, sMex * body)
{
    sIon * ion=(sIon *) param;
    RecordType * pRecordType=ion->recordTypesArr.ptr(recordTypeIndex);

    if(pRecordType->cType==eCTypeIndexOnly){
        if(pRecordSize)
            *pRecordSize=sizeof(recordIndex);
        return sNotPtr;
    }

    sVec < Record> * recordsArr;sIon_ensureContainerIon(ion,pRecordType,recordsArr);
    Record * pRecord=recordsArr->ptr(recordIndex);
    if(pRecordSize)
        *pRecordSize=pRecord->size;
    if( (pRecord->size<=(idx)sizeof(pRecord->ofs))  )
        return (void*)&(pRecord->ofs);

    sMex * recordBodyContainer;sIon_ensureContainerIon(ion,pRecordType,recordBodyContainer);
    return recordBodyContainer->ptr(pRecord->ofs);
}


idx sIon::addRecord(idx recordTypeIndex, idx recordSize, const void * recordBody)
{

    sIon::RecordType * pRecordType=recordTypesArr.ptr(recordTypeIndex);
    if(pRecordType->typeIndexAlias)
        pRecordType=recordTypesArr.ptr(pRecordType->typeIndexAlias-1);

    if(pRecordType->cType==eCTypeIndexOnly)
        return *(idx *) recordBody;

    char voidBuf[1024];

    if(recordSize<0){
        recordSize=-recordSize;

        sIon_fixRecordFromCharBuf(pRecordType,recordBody, recordSize, voidBuf);
        idx val;
        if(pRecordType->typeIndex==1){

            sIHiLoScanf(val, recordBody, recordSize, 10 ) ;
        }

    }
    if(transactionFile.ok()){
        idx rt=transactionFile.vadd(3, sIonTransaction::eTransactionAddRecord,recordTypeIndex,recordSize)-transactionFile.ptr();
        idx b= (recordSize/sizeof(idx)); if(recordSize%sizeof(idx))b+=1;
        memcpy((void*)transactionFile.add(b),(const void*)recordBody,recordSize);
        return -rt;
    }

    sVec <Record> * recordsArr;sIon_ensureContainer(pRecordType,recordsArr);

    if(!recordSize){
        recordSize=1;
        recordBody=&sMex::_zero;
    }

    idx recordIndex;

    Record * pRecord=0;
    if( pRecordType->hashType > eHashTypeNone ) {
        if( recordSize==sIdxMax || recordSize==sIdxMax-1 ) {
            char bauto[1024];bauto[0]=(recordSize==sIdxMax) ? '@' : '#';recordBody=bauto;
            for ( idx ir=0; ir< 32; ++ir ) {


                const char * rnd=(const char * ) sWord::getWord(random());
                for( recordSize=0; *rnd; ++rnd ) { bauto[ ++recordSize ]=*rnd;}
                bauto[ ++recordSize ]='@';++recordSize;
                char * bb=bauto+recordSize;
                idx autoRecord=((recordTypeIndex+1))|((recordsArr->dim()+1)<<8), r;
                sNUMPrintf(bb,r,autoRecord,16,NUMAlphabet);recordSize+=r;
                recordIndex=getRecordByHash(pRecordType, recordBody, recordSize );
                if(recordIndex==sNotIdx)
                    break;
            }
        }else {
            recordIndex=getRecordByHash(pRecordType, recordBody, recordSize );

            if( recordIndex!=sNotIdx ) {
                return recordIndex;
            }
        }
    }


    if(!pRecord) {
        recordIndex=recordsArr->dim();
        pRecord=recordsArr->add(1);
    }

    if(recordSize){
        if(recordSize<=(idx)sizeof(pRecord->ofs)){
            pRecord->ofs=0;
            memcpy((void*)(&pRecord->ofs),(const void *)(recordBody),recordSize);
        } else {
            sMex * recordBodyContainer;sIon_ensureContainer(pRecordType,recordBodyContainer);
            pRecord->ofs=recordBodyContainer->add(recordBody, recordSize);
        }
        pRecord->size=recordSize;
        if(pRecordType->hashType > eHashTypeNone ) {
            setRecordHash(pRecordType,recordBody,recordSize,recordIndex);
        }
    }


    return recordIndex;
}

const void * sIon::getRecordBody(idx recordTypeIndex, idx recordIndex , idx ** pRecordSize, sStr * bodyText)
{
    sIon::RecordType * pRecordType=recordTypesArr.ptr(recordTypeIndex);
    if(pRecordType->cType==eCTypeIndexOnly){
        if(pRecordSize)
            *pRecordSize=(idx *)&sizeofIdx;
        return sNotPtr;
    }

    sVec <Record> * recordsArr;sIon_ensureContainer(pRecordType,recordsArr);
    Record * pRecord=recordsArr->ptr(recordIndex);
    if(pRecordSize)
        *pRecordSize=&(pRecord->size);

    sMex * recordBodyContainer;
    const void * recordBody;

    if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
        recordBody=(const void * )(&(pRecord->ofs));
    } else {
        sIon_ensureContainer(pRecordType,recordBodyContainer);
        recordBody=recordBodyContainer->ptr(pRecord->ofs);
    }

    if(bodyText) {
        if(pRecordType->cType==eCTypeReal ) {
            bodyText->printf("%lf",*(real*)recordBody);
        }
        else if(pRecordType->cType==eCTypeIdx || pRecordType->cType==eCTypeIdxRange ) { \
            bodyText->printf("%" DEC,*(idx *)recordBody);
        }
        else {
            bodyText->add((const char * )recordBody,pRecord->size);
            bodyText->add0(1);
        }
    }

    return recordBody;

}



idx sIon::getHashBodyForRelation(sMex * toHash, RelationType * pRelationType, idx relationOfsInInt, sMex * relationsArr, idx relStart)
{

    idx iRelType=relStart;
    idx * pRelationTargets=(idx *)relationsArr->ptr(relationOfsInInt*sizeof(unsigned int));
    bool is32bit =((((unsigned int*)pRelationTargets)[0])&1) ? true : false ;

    for( ; iRelType<pRelationType->cntRelationHashTypes;++iRelType){
        idx relationTargetInHash=relationTargetsArr[pRelationType->ofsRelationTargets+iRelType];
        if(relationTargetInHash==(idx)-1)
            return iRelType;

        bool indexOnlyMode=false;
        if(relationTargetInHash&fRelationTypeIndex) {
            indexOnlyMode=true;
            relationTargetInHash&=~fRelationTypeIndex;
        }


        idx relationTargetType=relationTargetsArr[pRelationType->ofsRelationTargets+relationTargetInHash];
        RecordType * pRecordTypeToHash=recordTypesArr.ptr(relationTargetType);


        idx relationTargetIndex= is32bit ?  (idx)((((unsigned int * )pRelationTargets)[relationTargetInHash])) :pRelationTargets[relationTargetInHash] ;
        if(relationTargetInHash==0)relationTargetIndex=((udx)relationTargetIndex)>>1;


        toHash->add(&relationTargetType,sizeof(relationTargetType));
        if(indexOnlyMode || pRecordTypeToHash->cType==eCTypeIndexOnly) {
            toHash->add(&relationTargetIndex,sizeof(relationTargetIndex));
        }else {
            sVec < Record> * recordsArr;sIon_ensureContainer(pRecordTypeToHash,recordsArr);
            sMex * recordBodyContainer;sIon_ensureContainer(pRecordTypeToHash,recordBodyContainer);

            idx isdel=false;
            if(is32bit) {if(relationTargetIndex==0xFFFFFFFF || relationTargetIndex==0x7FFFFFFF)isdel=true;}
            else {if(relationTargetIndex==(idx)-1)isdel=true;}
            if(isdel) {
                toHash->cut(0);
                return iRelType;
            }
            else {
                Record * pRecordToHash=recordsArr->ptr(relationTargetIndex);
                const void * pRecordBodyToHash=(pRecordToHash->size<=(idx)sizeof(pRecordToHash->ofs)) ? (void*)&(pRecordToHash->ofs) : recordBodyContainer->ptr(pRecordToHash->ofs);
                toHash->add(pRecordBodyToHash,pRecordToHash->size);
            }
        }
    }
    return iRelType;
}



idx sIon::prepareSearchValset(idx relationTypeIndex, sIon::SearchElement * seList, idx maxSeList, const idx * markerArr, va_list marker,sVec < RecordResult > * stackRecords)
{
    idx iVarCnt;
    for ( iVarCnt=0; iVarCnt<maxSeList; ++iVarCnt) {
        SearchElement * se=seList+iVarCnt;


        if(markerArr) {
            se->iRecordTypeTocompare=markerArr[iVarCnt*3];
            se->val=sConvInt2Ptr((markerArr[iVarCnt*3+1]),void);
            se->valSize=markerArr[iVarCnt*3+2];
        }
        else {
            se->iRecordTypeTocompare=va_arg(marker, idx );
            se->val=va_arg(marker, const void * );
            se->valSize=va_arg(marker, idx ) ;
        }

        if(!se->val || se->iRecordTypeTocompare==sNotIdx )
            break;

        idx recordIndex;
        if(stackRecords && (sConvPtr2Int(se->val)==sNotIdx || sConvPtr2Int(se->val)==sNotIdx-2) ) {
            RecordResult * rr=stackRecords->ptr(se->valSize>=0 ? se->valSize: (stackRecords->dim()+se->valSize) );
            se->val=rr->body;
            se->valSize=rr->size;
        } else if( se->val==Link_BodyByIndex ){
            recordIndex=se->valSize;
            idx * pValSize;
            se->val=getRecordBody(se->iRecordTypeTocompare,recordIndex, &pValSize, 0);
            se->valSize=*pValSize;
            if(se->val==sNotPtr)se->val=&recordIndex;
        }
    }
    return iVarCnt;
}

idx sIon::getNextRelationBySearchValset(idx relationTypeIndex, sIon::SearchElement * seList, idx iVarCnt, idx * pirPos, idx * pStartFrom)
{
    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);

    sMex * relationsArr;sIon_ensureContainer(pRelationType,relationsArr);
    idx * pRelationTargets=relationTargetsArr.ptr(pRelationType->ofsRelationTargets);

    void * relationsOfsSet=0;
    bool is64Bit= (relationsArr->pos() >= (idx)(sizeof(int)*((idx)1<<32)) ) ? true : false ;
    idx irPos=pirPos ? *pirPos : 0, il ;
    for( il=* pStartFrom ; il<pRelationType->cntRelationsAll; ++il ){
        if( relationsOfsSet ) {
            irPos=is64Bit ? ((idx * )relationsOfsSet)[il] : ((unsigned int * )relationsOfsSet)[il];
            irPos*=sizeof(int);
        }

        idx * pRecordTargetIndexes=(idx*)relationsArr->ptr(irPos);
        bool is32bit =((((unsigned int*)pRecordTargetIndexes)[0])&1) ? true : false ;
        idx ivar;


        for( ivar=0; ivar<iVarCnt; ++ivar  ) {
            SearchElement * se=seList+ivar;

            bool approx=false;
            idx irTypeToCompare=se->iRecordTypeTocompare;
            if(irTypeToCompare&(0x80000000ll)){
                approx=true;
                irTypeToCompare&=(~(0x80000000ll));
            }
            idx regofs=(irTypeToCompare>>32)&0xFFFFFFFF;
            if(regofs){
                irTypeToCompare&=0xFFFFFFFF;
            }

            bool isdel=is32bit  ? (((((int * )pRecordTargetIndexes)[irTypeToCompare]))==((int)-1)) : (pRecordTargetIndexes[irTypeToCompare]==sNotIdx) ;
            if(isdel) {
                continue;
            }

            idx relationTargetIndex= is32bit ?  (idx)((((unsigned int * )pRecordTargetIndexes)[irTypeToCompare])) :pRecordTargetIndexes[irTypeToCompare] ;
            if(irTypeToCompare==0)relationTargetIndex=((udx)relationTargetIndex)>>1;

            RecordType * pRecordType=recordTypesArr.ptr(pRelationTargets[irTypeToCompare] );
            const void * pRecordBody;
            idx pRecordSize;
            if(pRecordType->cType==eCTypeIndexOnly){
                pRecordBody=&relationTargetIndex;
                pRecordSize=sizeof(idx);
            }else {
                sVec <Record> * recordsArr;sIon_ensureContainer(pRecordType,recordsArr);
                Record * pRecord=recordsArr->ptr(relationTargetIndex);
                if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                    pRecordBody=(const void * ) &(pRecord->ofs);
                } else {
                    sMex * recordBodyContainer;sIon_ensureContainer(pRecordType,recordBodyContainer);
                    pRecordBody=recordBodyContainer->ptr(pRecord->ofs);
                }
                pRecordSize=pRecord->size;
                if((regofs || approx) && pRecordSize>1024){
                    pRecordSize=1024;
                }
            }



            if(regofs || approx) {
                regBuf.cut(0);
                regBuf.add(pRecordBody,pRecordSize);
                regBuf.add("\0\0",1);
            }
            idx match=0;
            if(regofs){
                match=regexec((regex_t*)regexpList.ptr(regofs-1),(const char*) regBuf.ptr(),0,0,0) ? 0 : 1 ;
            }else {
                if(approx)
                    match= strstr((const char*)regBuf.ptr(),(const char*)se->val) ? 1 : 0;
                else {
                    match = (se->valSize==pRecordSize && (memcmp(pRecordBody,(const char*)se->val,se->valSize)==0) ) ? 1 : 0 ;
                }
            }
            if(!match)
                break;
        }

        idx nxtPos=irPos+pRelationType->cntRelationTargets * (is32bit ? sizeof(int) : sizeof(idx));
        if(!relationsOfsSet && pirPos)
            *pirPos=nxtPos;
        if(ivar==iVarCnt){
            ++il;
            break;
        }
        irPos=nxtPos;
    }


    if( pStartFrom )
        *pStartFrom=il;

    return (il >= pRelationType->cntRelationsAll) ? sNotIdx : irPos ;
}


idx sIon::getRelationBucketByHashVarg(sIon::Bucket * bucket, idx relationTypeIndex, idx relationHasherIndexType, idx * pCnt, const idx * markerArr, va_list marker0,sVec < RecordResult > * stackRecords)
{
    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    idx * pRelationTargetsInHash=relationTargetsArr.ptr(pRelationType->ofsRelationTargets);

    sIon_fixHasherIndex(relationHasherIndexType, pRelationTargetsInHash);

    idx howManyHaveSyn=1;


    char buf[4*1024];
    idx xhash,relationIndex=0;
    sHash2 * relationHashContainer=0;

    for(idx iSyn=0; iSyn<howManyHaveSyn ; iSyn++) {
        bucket->toHash->cut(0);
        idx ivar;

        va_list marker;
        if(marker0)
            va_copy(marker,marker0);

        idx iSynThisLoop=iSyn;
        for ( ivar=0; true; ++ivar) {

            idx relationTargetType=pRelationTargetsInHash[relationHasherIndexType+ivar];
            if(relationTargetType==sNotIdx)break;
            relationTargetType=pRelationTargetsInHash[relationTargetType];

            const void * val;
            idx valSize=0;
            if(markerArr) {
                val=sConvInt2Ptr((markerArr[ivar*2]),void);
                valSize=markerArr[ivar*2+1];
            }
            else {
                val=va_arg(marker, const void * );
                valSize=va_arg(marker, idx ) ;
            }
            if(!val)
                return sNotIdx;

            idx recordIndex;
            if(stackRecords && (sConvPtr2Int(val)==sNotIdx || sConvPtr2Int(val)==sNotIdx-2) ) {
                RecordResult * rr=stackRecords->ptr(valSize>=0 ? valSize : (stackRecords->dim()+valSize) );
                val=rr->body;
                valSize=rr->size;
            } else if( val==Link_BodyByIndex ){
                recordIndex=valSize;
                idx * pValSize;
                val=getRecordBody(relationTargetType,recordIndex, &pValSize, 0);
                valSize=*pValSize;
                if(val==sNotPtr)val=&recordIndex;
            }

            bucket->toHash->add(&relationTargetType,sizeof(relationTargetType));


            RecordType * pRecordType=recordTypesArr.ptr(relationTargetType);
            const char * fmt=(const char * ) baseContainer.ptr(pRecordType->typeValueSynonyms00);
            idx cntSynPerThisType=sString::cnt00(fmt)+1;
            if(fmt && *fmt=='*' && *(fmt+1)==0 ) {
                fmt=sString::next00(fmt);
                --cntSynPerThisType;
            }
            if(iSyn==0){
                howManyHaveSyn*=cntSynPerThisType;
            }

            idx indexToPick=iSynThisLoop%cntSynPerThisType;
            if(indexToPick!=0) {
                const char * pp=sString::next00(fmt,indexToPick-1);
                valSize=sprintf(buf,pp,(int)valSize,val);
                val=buf;
            }
            bucket->toHash->add(val,valSize);
            iSynThisLoop/=cntSynPerThisType;

        }
        if(marker0)
            va_end(marker);


        sMex * relationBucketContainer;sIon_ensureContainer(pRelationType,relationBucketContainer);
        sIon_ensureHashContainer(pRelationType,relationHashContainer,this,getRelationKeyFunction,relationBucketContainer);

        sHash2::hashtypeIdx hashTypeIndex=(((pRelationType->typeIndex)&0xFF) )|(((relationHasherIndexType)&0xFF)<<8);
        relationIndex=relationHashContainer->find(hashTypeIndex,bucket->toHash->ptr(),bucket->toHash->pos(),0,&xhash);

        if(relationIndex)
            break;
    }

    if(!relationIndex)
        return sNotIdx;
    if(pCnt)
        *pCnt=relationHashContainer->bucketCnt(relationIndex-1);

    if(bucket) {
        bucket->relationBucketPos=relationIndex-1;
        bucket->xhash=xhash;
    }
    return relationIndex-1;
}

idx sIon::getRelationResultsByRelationTargets(idx relationTypeIndex, idx * pRelationTargets, RecordResult * pRecordResults )
{
    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    idx * pRelationTargetTypes=relationTargetsArr.ptr(pRelationType->ofsRelationTargets);
    bool is32bit =((((idx*)pRelationTargets)[0])&1) ? true : false ;

    bool anydel=false;
    for( idx iRelTarget=0; iRelTarget<pRelationType->cntRelationTargets; ++iRelTarget ) {
        bool isdel=is32bit  ? ((idx)((((int * )pRelationTargets)[iRelTarget]))==((int)-1)) : (pRelationTargets[iRelTarget]==sNotIdx) ;
        if(isdel){
            pRecordResults->size=-1;
            pRecordResults->body=(const void*) (&sMex::_zero);
            ++pRecordResults;
            anydel=true;
            continue;
        }

        if(iRelTarget==0)
            pRecordResults->index=is32bit ? (idx)(((unsigned int * )pRelationTargets)[0])>>1 : pRelationTargets[0];
        else
            pRecordResults->index=is32bit ? (idx)(((unsigned int * )pRelationTargets)[iRelTarget]) : pRelationTargets[iRelTarget];

        pRecordResults->typeIndex=pRelationTargetTypes[iRelTarget];


        sIon::RecordType * pRecordTypeInRelation=recordTypesArr.ptr(pRecordResults->typeIndex);
        pRecordResults->cType=pRecordTypeInRelation->cType;
        if(pRecordTypeInRelation->cType==eCTypeIndexOnly){
            pRecordResults->size=sizeof(pRecordResults->index);
            pRecordResults->body=&pRecordResults->index;
        } else{
            sVec <Record> * recordsArr;sIon_ensureContainer(pRecordTypeInRelation,recordsArr);
            Record * pRecord=recordsArr->ptr(pRecordResults->index);
            pRecordResults->size=pRecord->size;

            if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                pRecordResults->body=(const void * )(&(pRecord->ofs));
            } else {
                sMex * recordBodyContainer;sIon_ensureContainer(pRecordTypeInRelation,recordBodyContainer);
                pRecordResults->body=recordBodyContainer->ptr(pRecord->ofs);
            }
        }
        ++pRecordResults;
    }
    return anydel ? -1 : 1;
}

idx sIon::getNextRelationInBucket(Bucket * bucket, idx relationTypeIndex, idx num)
{
    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    sHash2 * relationHashContainer=&(typeContainers[pRelationType->typeIndex].relationHashContainer);
    sMex * relationsArr=&(typeContainers[pRelationType->typeIndex].relationsArr);
    idx relationOfs=0;

    for(idx i=0; bucket->found() && i<num; ) {
        relationOfs=relationHashContainer->bucketNextIndex(&(bucket->relationBucketPos), false, 1);
        idx * pRelationTargets=(idx *)relationsArr->ptr(relationOfs*sizeof(unsigned int));
        bool is32bit =((((idx*)pRelationTargets)[0])&1) ? true : false ;
        bool isdel=false;
        for( idx iRelTarget=0; iRelTarget<pRelationType->cntRelationTargets; ++iRelTarget ) {
            isdel=is32bit  ? ((idx)((((int * )pRelationTargets)[iRelTarget]))==((int)-1)) : (pRelationTargets[iRelTarget]==sNotIdx) ;
            if(isdel)break;
        }
        if(isdel)
            continue;
        ++i;
    }
    return relationOfs;
}

idx sIon::getResultSetFromSearchTrajectory(sVec < sIon::RecordResult > * resultFinal, const idx * searchTraj, va_list marker, sVec < RecordResult > * resultSet )
{
    idx val,valSize;
    idx cnt=0;
    idx iVar=1+sizeof(sIonWander::StatementHeader)/sizeof(idx);
    for ( ; iVar<searchTraj[0]; iVar+=2 ) {
        if(searchTraj) {
            val=searchTraj[iVar];
            valSize=searchTraj[iVar+1];
        } else if(marker) {
            val=va_arg(marker, idx );
            valSize=va_arg(marker, idx ) ;
        } if(!val)
            return sNotIdx;

        if( resultSet && val==sNotIdx ){
            RecordResult * rr=resultSet->ptr(valSize>=0 ? valSize : (resultSet->dim()+valSize) );
            val=sConvPtr2Int(rr->body);
            valSize=rr->size;
            ++cnt;
        }else if( resultSet && val==sNotIdx-1 ){
            cnt+=getResultSetFromSearchTrajectory(resultFinal,searchTraj+valSize,marker, resultSet);
            continue;
        } else {
            ++cnt;
        }

        RecordResult * rr=resultFinal->add();
        rr->body=sConvInt2Ptr(val,void);
        rr->size=valSize;

    }
    return cnt;
}


idx sIon::getRelationsByBucketAndIndex(Bucket * bucket, idx relationTypeIndex, RecordResult * pRecordResults , idx * pRelCnt, bool moveToNext)
{
    idx relationBucketPos=bucket->relationBucketPos;

    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    sHash2 * relationHashContainer=&(typeContainers[pRelationType->typeIndex].relationHashContainer);
    sMex * relationsArr=&(typeContainers[pRelationType->typeIndex].relationsArr);



    idx iRel, relCnt=(pRelCnt && *pRelCnt) ? *pRelCnt : 1;

    for( iRel=0; relationBucketPos!=sNotIdx && iRel<relCnt; ++iRel) {

        idx relationOfs=relationHashContainer->bucketNextIndex(&relationBucketPos);


        idx * pRelationTargets=(idx *)relationsArr->ptr(relationOfs*sizeof(unsigned int));

        getRelationResultsByRelationTargets(relationTypeIndex, pRelationTargets, pRecordResults);


    }

    if(pRelCnt)
        *pRelCnt=relationHashContainer->bucketCount(bucket->relationBucketPos);

    if(moveToNext)
        bucket->relationBucketPos=relationBucketPos;

   return relationBucketPos;
}


void * sIon::getRelationKeyFunction(void * param, sHash2::hashtypeIdx typeRelationIndex, idx relationIndex,idx * pRelationHashBodySize, sMex * body)
{
    sIon * ion=(sIon *) param;

    idx relationTypeIndex=(typeRelationIndex)&0xFF;
    idx hasherStartIndex=(typeRelationIndex>>8)&0xFFFF;


    body->cut(0);
    RelationType * pRelationType=ion->relationTypesArr.ptr(relationTypeIndex);
    sMex * relationsArr;sIon_ensureContainerIon(ion,pRelationType,relationsArr);
    ion->getHashBodyForRelation(body, pRelationType, relationIndex, relationsArr, hasherStartIndex);

    *pRelationHashBodySize=body->pos();
    return body->ptr();
}


idx sIon::addRelationVarg(idx relationTypeIndex,idx insertIndex, const idx * markerArr, va_list marker )
{
    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);



    idx iRelType=0;

    if( transactionFile.ok() ) {
        idx rT= transactionFile.vadd(4, sIonTransaction::eTransactionAddRelation,relationTypeIndex,insertIndex, pRelationType->cntRelationTargets)-transactionFile.ptr();
        for ( iRelType=0; iRelType<pRelationType->cntRelationTargets; ++iRelType ) {
            idx targetTypeIndex=markerArr ? markerArr[iRelType] : va_arg(marker, idx );
            transactionFile.vadd(1, targetTypeIndex );
        }
        return -rT;
    }

    sMex * relationsArr;sIon_ensureContainer(pRelationType,relationsArr);
    idx relationsOfs=relationsArr->pos();
    bool isThereIdx=false;

    for ( iRelType=0; iRelType<pRelationType->cntRelationTargets; ++iRelType ) {
        idx targetTypeIndex=markerArr ? markerArr[iRelType] : va_arg(marker, idx );

        relationsArr->add(&targetTypeIndex,sizeof(targetTypeIndex));
        if(targetTypeIndex>=0x7FFFFFFF)
            isThereIdx=true;
    }

    idx * relations=(idx*)relationsArr->ptr(relationsOfs);
    sMex * relationBucketContainer;sIon_ensureContainer(pRelationType,relationBucketContainer);
    sHash2 * relationHashContainer;sIon_ensureHashContainer(pRelationType,relationHashContainer,this,getRelationKeyFunction,relationBucketContainer);

    ++pRelationType->cntRelationsAll;



    if(isThereIdx==false ) {
        unsigned int * relations32=(unsigned int *)relationsArr->ptr(relationsOfs);

        relations32[0]=(relations[0]<<1)|1;
        for ( idx iR=1;iR<iRelType; ++iR ) {
            relations32[iR]=relations[iR];
        }

        relationsArr->cut(relationsOfs+iRelType*sizeof(unsigned int));
    }
    else {
        pRelationType->flags|=sFlag(RelationType::bRelation64Bit);
    }



    sMex toHash(sMex::fBlockCompact);
    for( ++iRelType; iRelType<pRelationType->cntRelationHashTypes-1;++iRelType){
        idx hasherStartIndex=iRelType;
        iRelType=sIon::getHashBodyForRelation(&toHash, pRelationType, relationsOfs/sizeof(int), relationsArr,hasherStartIndex);

        if(toHash.pos()==0)
            break;

        sHash2::hashtypeIdx hashTypeIndex=(((pRelationType->typeIndex)&0xFF) )|(((hasherStartIndex)&0xFF)<<8);
        relationHashContainer->map((relationsOfs/sizeof(int)),hashTypeIndex,toHash.ptr(),toHash.pos(),0,insertIndex);

        toHash.cut(0);

    }

    if( pRelationType->cntRelationsAll==1 ){
        Configuration * cfg=configuration.get((const char *)baseContainer.ptr((pRelationType)->nameOfsContainer));
        if(cfg) relationHashContainer->rehash(cfg->sizeExpectation);
    }
    return relationsOfs;


}


idx sIon::deleteRelationsByBucketAndIndex(Bucket * bucket, idx relationTypeIndex, idx iStart, idx * pRelCnt, RecordResult * pRecordResults)
{

    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    sMex * relationsArr=&(typeContainers[pRelationType->typeIndex].relationsArr);



    idx iRel, relCnt=(pRelCnt && *pRelCnt) ? *pRelCnt : 1, iRelCounted=0;

    for( iRel=0; bucket->found() && iRelCounted<relCnt; ++iRel) {

        idx relationOfs=getNextRelationInBucket(bucket,relationTypeIndex,1);
        if(iRel<iStart)
            continue;


        if( pRecordResults ) {
            idx * pRelationTargets=(idx *)relationsArr->ptr(relationOfs*sizeof(unsigned int));
            getRelationResultsByRelationTargets(relationTypeIndex, pRelationTargets, pRecordResults);
        }
        deleteRelation(relationTypeIndex, relationOfs);
        ++iRelCounted;
    }

    if(pRelCnt)
        *pRelCnt=iRel;


    return iRelCounted;
}

void sIon::deleteRelation(idx relationTypeIndex, idx relationOffset )
{

    if( transactionFile.ok() ) {
        transactionFile.vadd(3, sIonTransaction::eTransactionDeleteRelation,relationTypeIndex,relationOffset);
        return;
    }
    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    sMex * relationsArr;sIon_ensureContainer(pRelationType,relationsArr);
    idx * pRecordTargetIndexes=(idx*)relationsArr->ptr(relationOffset*sizeof(unsigned int));
    bool is32bit =((((unsigned int*)pRecordTargetIndexes)[0])&1) ? true : false ;

    if(is32bit) {
        for( idx ir = 0 ; ir<pRelationType->cntRelationTargets;  ++ir ){
            ((int * )pRecordTargetIndexes)[ir]=(int)(-1);
        }
    }else {
        for( idx ir = 0 ; ir<pRelationType->cntRelationTargets;  ++ir ){
            pRecordTargetIndexes[ir]=-1;
        }
    }

}
void sIon::deleteRelationBucket(idx relationTypeIndex, Bucket * bucket )
{
    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    sMex * relationBucketContainer;sIon_ensureContainer(pRelationType,relationBucketContainer);
    sHash2 * relationHashContainer;sIon_ensureHashContainer(pRelationType,relationHashContainer,this,getRelationKeyFunction,relationBucketContainer);
    relationHashContainer->bucketDelete(&bucket->relationBucketPos,bucket->xhash);
}




void sIon::destroyDependencies(void)
{
    for(idx poo=0; poo<regexpList.pos(); poo+=sizeof(regex_t) ){
        regfree((regex_t*)regexpList.ptr(poo));
    }
}

