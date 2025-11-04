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
#include <slib/utils/sort.hpp>
#include <ion/sIon-core.hpp>
using namespace slib;



idx sIon::sortRelationsCallbackFunction(sIon::RelationSortStruct * param, void * relOffsets, idx i1, idx i2 )
{

    if(i1==i2)
        return 0;

    idx * p1Rels,* p2Rels;
    if(param->is64Bit) {
        p1Rels=(idx*)sShift(param->relationsArr, ((idx * )relOffsets)[i1]*sizeof(int));
        p2Rels=(idx*)sShift(param->relationsArr, ((idx * )relOffsets)[i2]*sizeof(int));
    }else {
        p1Rels=(idx*)sShift(param->relationsArr, (idx)(((unsigned int * )relOffsets)[i1])*sizeof(int));
        p2Rels=(idx*)sShift(param->relationsArr, (idx)(((unsigned int * )relOffsets)[i2])*sizeof(int));
    }


    bool is1_32bit =((((unsigned int*)p1Rels)[0])&1) ? true : false ;
    bool is2_32bit =((((unsigned int*)p2Rels)[0])&1) ? true : false ;

#ifdef IMPLEMENT_CACHE
    idx r1LocInCache,r2LocInCache;
    if(param->sortBufCacheSize>1) {
        r1LocInCache=(2*(i1%(param->sortBufCacheSize)));
        r2LocInCache=(2*(i2%(param->sortBufCacheSize))+1);
    }else {
        r1LocInCache=0;
        r2LocInCache=1;
    }

    RecordResult * r1=param->pairRelBuf.ptr(r1LocInCache*param->cntSorted);
    RecordResult * r2=param->pairRelBuf.ptr(r2LocInCache*param->cntSorted);
#else
    RecordResult * r1=param->pairRelBuf.ptr(0);
    RecordResult * r2=param->pairRelBuf.ptr(param->cntSorted);
#endif


    for( idx ir = 0 ; ir<param->cntSorted;  ++ir , ++r1, ++r2  ){
        idx iR=r1->typeIndex>>32;

        #ifdef IMPLEMENT_CACHE
        if( param->cacheIndexes[r1LocInCache]!=i1)
        #else
        if( param->i1Prev!=i1)
        #endif
        {
            idx relationTargetIndex= is1_32bit ?  (idx)((((unsigned int * )p1Rels)[iR])) :p1Rels[iR] ;
            if(iR==0)relationTargetIndex=((udx)relationTargetIndex)>>1;
            r1->index=relationTargetIndex;

            if(r1->ccType()==eCTypeIndexOnly){
                r1->body=(void*)&relationTargetIndex;
                r1->size=sizeof(relationTargetIndex);
            } else {
                Record * pRecord=param->recordMexs[ir]->ptr(relationTargetIndex);
                if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                    r1->body=(const void * ) &(pRecord->ofs);
                } else {
                    r1->body=param->recordBodyMexs[ir]->ptr(pRecord->ofs);
                }
                r1->size=pRecord->size;
            }
        }

        #ifdef IMPLEMENT_CACHE
        if( param->cacheIndexes[r2LocInCache]!=i2)
        #else
        if( param->i2Prev!=i2)
        #endif
        {
            idx relationTargetIndex= is2_32bit ?  (idx)((((unsigned int * )p2Rels)[iR])) :p2Rels[iR] ;
            if(iR==0)relationTargetIndex=((udx)relationTargetIndex)>>1;
            r2->index=relationTargetIndex;
            if(r2->ccType()==eCTypeIndexOnly){
                r2->body=(void*)&relationTargetIndex;
                r2->size=sizeof(relationTargetIndex);
            } else {
                Record * pRecord=param->recordMexs[ir]->ptr(relationTargetIndex);
                if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                    r2->body=(const void * ) &(pRecord->ofs);
                } else {
                    r2->body=param->recordBodyMexs[ir]->ptr(pRecord->ofs);
                }
                r2->size=pRecord->size;
            }
        }
    }


#ifdef IMPLEMENT_CACHE
    param->cacheIndexes[r1LocInCache]=i1;
    param->cacheIndexes[r2LocInCache]=i2;
    r1=param->pairRelBuf.ptr(r1LocInCache*param->cntSorted);
    r2=param->pairRelBuf.ptr(r2LocInCache*param->cntSorted);
#else
    param->i1Prev=i1;
    param->i2Prev=i2;
    r1=param->pairRelBuf.ptr(0);
    r2=param->pairRelBuf.ptr(param->cntSorted);
#endif



    if(param->sorterFunc)
        return param->sorterFunc(param->ion,param->sorterParam, r1, r2,i1,i2);
    return sortRecordComparator(r1,r2,param->cntSorted,i1,i2);
}


idx sIon::sortRecordComparator(RecordResult * r1, RecordResult * r2, idx cnt, idx i2, idx i1 )
{
    idx res = 0;
    for( idx ir = 0 ; ir<cnt;  ++ir, ++r1, ++r2 ){

        if(r1->ccType()==eCTypeString){
            if(r1->flags()&sFlag(sIon::bSortRelationNumeric) ) {
                {sIon_numericCmpHex(res,r1,r2);}
            } else {
                {sIon_bodyCmp(res,r1,r2);}
            }

        }else if(r1->ccType()==eCTypeIdx || r1->ccType()==eCTypeIndexOnly  || r1->ccType()==eCTypeIdxRange  ) {
            res=(*(idx*)(r1->body))-(*(idx*)(r2->body));
        }else if(r1->ccType()==eCTypeReal) {
            res=(*(real*)(r1->body))-(*(real*)(r2->body));
        }else {
            res=memcmp(r1->body,r2->body,sMin(r1->size,r2->size));
        }

        if(r1->flags()&sFlag(sIon::bSortRelationReverse) )
            res=-res;
        if(res)
            break;
    }

    if(!res)
        res=i2-i1;

    return res;
}



idx sIon::analyzeUseRecordSortLine(RelationType * pRelationType, const char * useRecords, sVec < idx > * recordTypeIndexes)
{
    idx * pRelationTargetTypes=relationTargetsArr.ptr(pRelationType->ofsRelationTargets);
    idx cntSorted;

    sStr buf;
    if(useRecords && *useRecords ) {
        for (const char *  ptr=useRecords; *ptr;  ){

            const char * nxt=ptr, *flgs=0;
            while( *nxt ){
                if(*nxt==' ')break;
                if(*nxt==':') flgs=nxt;
                ++nxt;
            }
            if(!flgs)
                flgs=nxt;

            buf.cut(0);buf.addString(ptr,flgs-ptr);
            idx recordTypeIndex=recordAndRelationTypesHash.find(eRARHash_record,buf.ptr(),buf.length()+1);
            ptr=nxt+1;
            if(!recordTypeIndex)
                continue;
            --recordTypeIndex;

            idx ia;
            for ( ia=0; ia<pRelationType->cntRelationTargets; ++ia)
                if( recordTypeIndex == pRelationTargetTypes[ia] )
                    break;
            if(ia==pRelationType->cntRelationTargets)
                continue;

            idx flags=0;
            if(flgs!=nxt)
                sString::xscanf(flgs+1,"%b=0|reverse|numeric", &flags);

            recordTypeIndexes->vadd(3,recordTypeIndex,ia, flags);
        }
        cntSorted= recordTypeIndexes->dim()/3;
    } else {
        cntSorted=pRelationType->cntRelationTargets;
    }

    return cntSorted;
}

idx sIon::sortRelations(const char * relationName, const char * useRecords, const char * sortName, sIon::sCallbackRelationSorter sorterFunc, void * sorterParam, sVec<idx> * tRecordTypeIndex , idx extraMem, idx sortCache)
{

    idx relationTypeIndex=recordAndRelationTypesHash.find(eRARHash_relation,relationName,sLen(relationName)+1);
    if( !relationTypeIndex )
        return 0;
    else
        --relationTypeIndex;


    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);
    sMex * relationsArr;sIon_ensureContainer(pRelationType,relationsArr);
    bool is64Bit= (relationsArr->pos() >= (idx)(sizeof(int)*((idx)1<<32)) ) ? true : false ;
    idx sizeOne= is64Bit ? sizeof(idx) : sizeof(int);
    idx size=   sizeOne * pRelationType->cntRelationsAll ;
    extraMem*=pRelationType->cntRelationsAll;
    sVec < idx > recordTypeIndexes;
    idx cntSorted=analyzeUseRecordSortLine(pRelationType, useRecords, &recordTypeIndexes);



    sStr flnm(sMex::fExactSize);
    flnm.printf(0,"%s-%s-#%s%s",baseName.ptr(),(const char *)baseContainer.ptr(pRelationType->nameOfsContainer), sortName , extension);
    sMex RelOfses( sMex::fExactSize ); RelOfses.init( flnm.ptr(0) );RelOfses.cut(0);

    RelOfses.add(0,
        sizeof(SortRelationsHeader)+(cntSorted-1)*sizeof(RecordResult)
        +size+
        extraMem+
        ( (((idx)sorterFunc)!=sNotIdx) ? size*2  : 0 )
        );

    idx oofs=0;
    SortRelationsHeader * sortCfg=(SortRelationsHeader * )RelOfses.ptr(oofs); oofs+=sizeof(SortRelationsHeader)+(cntSorted-1)*sizeof(RecordResult);
    idx * finalOfses=(idx*)RelOfses.ptr( oofs );oofs+=size+extraMem ;
    idx * relOfses=(((idx)sorterFunc)!=sNotIdx) ? (idx*)RelOfses.ptr( oofs ) : finalOfses; oofs+=size;
    idx * sindexes=(((idx)sorterFunc)!=sNotIdx) ? (idx*)RelOfses.ptr( oofs ) : 0 ;


    for( idx irPos = 0, il=0 ; il<pRelationType->cntRelationsAll ; ++il ){
        if(is64Bit) ( (idx*)relOfses )[il] = (irPos /sizeof(int));
        else ( (unsigned int *)relOfses )[il] = (unsigned int)(irPos /sizeof(int));

        idx * pRecordTargetIndexes=(idx*)relationsArr->ptr(irPos);
        bool is32bit =((((unsigned int*)pRecordTargetIndexes)[0])&1) ? true : false ;
        irPos+=pRelationType->cntRelationTargets * (is32bit ? sizeof(int) : sizeof(idx));
    }

    if(((idx)sorterFunc)==sNotIdx)
        return pRelationType->cntRelationsAll;




    sIon::RelationSortStruct par;
    #ifdef IMPLEMENT_CACHE
        par.sortBufCacheSize=sortCache>0  ? sortCache : 64 ;
    #else
        par.sortBufCacheSize=1;
    #endif

    par.recordMexs.add(pRelationType->cntRelationTargets);
    par.recordBodyMexs.add(pRelationType->cntRelationTargets);
    par.i1Prev=sNotIdx;
    par.i2Prev=sNotIdx;
    par.ion=this;
    par.sorterFunc=sorterFunc;
    par.sorterParam=sorterParam;
    par.pRelationType=pRelationType;
    par.is64Bit=is64Bit;
    par.relationsArr=relationsArr->ptr();

    idx * pRelationTargetTypes=relationTargetsArr.ptr(par.pRelationType->ofsRelationTargets);

    par.cntSorted=cntSorted;
    par.pRelationType=pRelationType;
    par.pairRelBuf.add( (2 * par.cntSorted)*par.sortBufCacheSize );
    par.cacheIndexes.add(2*par.sortBufCacheSize ); par.cacheIndexes.set(sNotIdx);


    sVec<idx> recordTypeIndex, * pRecordTypeIndex;
    pRecordTypeIndex = &recordTypeIndex;
    if(tRecordTypeIndex) pRecordTypeIndex = tRecordTypeIndex;
    pRecordTypeIndex->resize(par.cntSorted);
    for( idx iRelTarget = 0 ; iRelTarget < par.cntSorted;  ++iRelTarget ){
        idx flags, orderInRelation;
        if( recordTypeIndexes.dim() ) {
            *pRecordTypeIndex->ptr(iRelTarget)=recordTypeIndexes[3*iRelTarget];
            orderInRelation=recordTypeIndexes[3*iRelTarget+1];
            flags=recordTypeIndexes[3*iRelTarget+2];
        } else {
            *pRecordTypeIndex->ptr(iRelTarget)=pRelationTargetTypes[iRelTarget];
            orderInRelation=iRelTarget;
            flags=0;
        }


        sIon::RecordType * pRecordTypeInRelation=recordTypesArr.ptr(*pRecordTypeIndex->ptr(iRelTarget));

        par.pairRelBuf[ iRelTarget ].body=0;
        par.pairRelBuf[ iRelTarget ].size=0;
        par.pairRelBuf[ iRelTarget ].cType=pRecordTypeInRelation->cType | ( (flags<<32) );
        par.pairRelBuf[ iRelTarget ].typeIndex=pRecordTypeInRelation->typeIndex |  ((orderInRelation << 32)) ;

        sortCfg->usedRecords[iRelTarget].cType=pRecordTypeInRelation->cType | ( (flags<<32) );
        sortCfg->usedRecords[iRelTarget].typeIndex=pRecordTypeInRelation->typeIndex |  ((orderInRelation << 32)) ;

        for (idx ic=iRelTarget + par.cntSorted, ma=par.sortBufCacheSize *2*par.cntSorted; ic< ma; ic+=par.cntSorted ) {
            par.pairRelBuf[ic].body=par.pairRelBuf[iRelTarget].body;
            par.pairRelBuf[ic].size=par.pairRelBuf[iRelTarget].size;
            par.pairRelBuf[ic].cType=par.pairRelBuf[iRelTarget].cType;
            par.pairRelBuf[ic].typeIndex=par.pairRelBuf[iRelTarget].typeIndex;
        }

        sVec <Record> * recordsArr;sIon_ensureContainer(pRecordTypeInRelation,recordsArr);
        par.recordMexs[iRelTarget]=recordsArr;
        sMex * recordBodyContainer;sIon_ensureContainer(pRecordTypeInRelation,recordBodyContainer);
        par.recordBodyMexs[iRelTarget]=recordBodyContainer;

    }





    if(is64Bit)
        sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sortRelationsCallbackFunction,(void*)&par,par.pRelationType->cntRelationsAll , (idx*)relOfses , (idx*)sindexes);
    else
        sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sortRelationsCallbackFunction,(void*)&par,par.pRelationType->cntRelationsAll , (int *)relOfses, (int*)sindexes  );



    if(is64Bit) {
        idx * src=(idx*)relOfses;
        idx * dst=(idx*)finalOfses;
        idx * ind=(idx*)sindexes;
        for( idx il=0 ; il<pRelationType->cntRelationsAll ; ++il )
            dst[il]=src[ind[il]];

    }else {
        int * src=(int*)relOfses;
        int * dst=(int*)finalOfses;
        int * ind=(int*)sindexes;
        for( idx il=0 ; il<pRelationType->cntRelationsAll ; ++il )
            dst[il]=src[ind[il]];
    }

    RelOfses.cut(sizeof(SortRelationsHeader)+(cntSorted-1)*sizeof(RecordResult)+size+extraMem);

    sortCfg->shiftRelationOffsets=sizeof(SortRelationsHeader)+(cntSorted-1)*sizeof(RecordResult);
    sortCfg->cntSorted=cntSorted;
    sortCfg->countSortedRelationOffsets=pRelationType->cntRelationsAll;

    return par.pRelationType->cntRelationsAll;

}




sIon::SortRelationsHeader * sIon::getRelationSorterIterator(const char * relationName, const char * sortName, RelationIterator * iter)
{
    idx relationTypeIndex=recordAndRelationTypesHash.find(eRARHash_relation,relationName,sLen(relationName)+1);
    if( !relationTypeIndex )
        return 0;
    else
        --relationTypeIndex;

    if(sortName) {

        sStr flnm(sMex::fExactSize);
        flnm.printf(0,"%s-%s-#%s%s",baseName.ptr(),(const char *)baseContainer.ptr(relationTypesArr.ptr(relationTypeIndex)->nameOfsContainer), sortName , extension);
        iter->mex->init( flnm.ptr() );
    }
    return getRelationSorterIterator(relationTypeIndex, iter);
}

sIon::SortRelationsHeader * sIon::getRelationSorterIterator(idx relationTypeIndex, RelationIterator * iter)
{
    iter->pRelationType=relationTypesArr.ptr(relationTypeIndex);
    sMex * relationsArr;sIon_ensureContainer(iter->pRelationType,relationsArr);
    iter->is64Bit= (relationsArr->pos() >= (idx)(sizeof(int)*((idx)1<<32)) ) ? true : false ;

    iter->relationsArr=relationsArr;
    iter->pRelationTargets=relationTargetsArr.ptr(iter->pRelationType->ofsRelationTargets);

    SortRelationsHeader * sh=0;
    if(iter->mex && iter->mex->pos()) {
        sh=(SortRelationsHeader * )iter->mex->ptr(0);
        iter->relationsOffsets=(idx*)iter->mex->ptr(sh->shiftRelationOffsets);
        iter->cntRelations=sh->countSortedRelationOffsets;
    } else {
        iter->relationsOffsets=0;
        iter->cntRelations=iter->pRelationType->cntRelationsAll;
    }

    return sh;
}


idx sIon::getRelationOffsetsBySortIterator(sIon::RelationIterator* iter, idx serno)
{
    if( iter->relationsOffsets ) {
        iter->irPos=iter->is64Bit ? ((idx * )iter->relationsOffsets)[serno] : ((int * )iter->relationsOffsets)[serno];
        iter->irPos*=sizeof(int);
    } else {
        iter->irPos=serno*iter->pRelationType->cntRelationTargets * (iter->is64Bit ? sizeof(idx) : sizeof(int));
    }
    return iter->irPos/sizeof(int);
}

idx sIon::getRelationBodyBySortIterator(sIon::RelationIterator* iter, sIon::RecordResult * resultSet , idx cntRelationBodies, idx countToProduce , idx serno)
{
    if(serno!=sNotIdx)
        iter->serno=serno;
    if(iter->serno>=iter->pRelationType->cntRelationsAll)
        return 0;

    RecordResult * r=resultSet;

    bool autoAll=false;
    if(cntRelationBodies<=0) {
        if(!cntRelationBodies)cntRelationBodies=iter->pRelationType->cntRelationTargets;
        else cntRelationBodies=-cntRelationBodies;
        autoAll=true;
    }


    idx il=0;

    SortRelationsHeader * sh=(SortRelationsHeader * )iter->mex->ptr(0);

    for( il=0; il<countToProduce; ++il, ++r) {

        if( iter->relationsOffsets ) {
            iter->irPos=iter->is64Bit ? ((idx * )iter->relationsOffsets)[iter->serno] : ((unsigned int * )iter->relationsOffsets)[iter->serno];
            iter->irPos*=sizeof(int);
        } else if(serno !=sNotIdx ) {
            iter->irPos=iter->serno*iter->pRelationType->cntRelationTargets * (iter->is64Bit ? sizeof(idx) : sizeof(int));
        }

        idx * pRecordTargetIndexes=(idx*)(iter->relationsArr->ptr(iter->irPos));
        bool is32bit =((((unsigned int*)pRecordTargetIndexes)[0])&1) ? true : false ;

        for( idx ir = 0 ; ir<cntRelationBodies;  ++ir , ++r){
            idx iR= autoAll ? ir : sh->usedRecords[ir].recordTypeIndex();

            r->index= is32bit ?  (idx)((((unsigned int * )pRecordTargetIndexes)[iR])) :pRecordTargetIndexes[iR] ;
            if(iR==0)r->index=((udx)r->index)>>1;

            RecordType * pRecordType=recordTypesArr.ptr(iter->pRelationTargets[iR] );
            r->cType=pRecordType->cType;
            if(pRecordType->cType==eCTypeIndexOnly){
                r->body=&r->index;
                r->size=sizeof(r->index);
            }
            else {
                sVec <Record> * recordsArr;sIon_ensureContainer(pRecordType,recordsArr);
                Record * pRecord=recordsArr->ptr(r->index);

                if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                    r->body=(const void * ) &(pRecord->ofs);
                } else {
                    sMex * recordBodyContainer;sIon_ensureContainer(pRecordType,recordBodyContainer);
                    r->body=recordBodyContainer->ptr(pRecord->ofs);
                }
                r->size=pRecord->size;

            }
            if(!iter->relationsOffsets)
                iter->irPos+=iter->pRelationType->cntRelationTargets * (is32bit ? sizeof(int) : sizeof(idx));
        }

        ++iter->serno;
    }
    return il;
}



