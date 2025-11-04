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
#include <sys/stat.h>
#include <ion/sIon.hpp>
#include <ion/sIon-core.hpp>

using namespace slib;


#define string_search(_v_type, _v_strtosearch,_v_strinlist) for(ipos=0; (_v_strinlist)[ipos]!=0 && (notfound=strcmp((_v_strinlist)+ipos,(_v_strtosearch))); ipos+=sLen((_v_strinlist)+ipos)+1, ++(_v_type));
const char * sIon::ctypelist="idx" _ "real" _ "string" _ "bool" _ "other" _ "#" _ "irange" __;
const char * sIon::hashtypelist="none" _ "idx" _ "ordered" _ "string" _ "other" __;


idx sIon::constructRecordAndRelationTypes(const char * defTypes, idx len )
{

    char buf[4*1024];
    const char * fld[4*1024] = {};

    idx is,il, id;
    for (is=0; is<len && (defTypes[is]==' ' || defTypes[is]=='\t') ; ++is )
        ;

    for(il=0, id=0;is<len;is+=il+1) {
        idx ip=0, ipos;
        fld[ip]=buf;

        const char * ptr=defTypes+is;

        for ( il=0,id=0; is+il<len && ptr[il]!='\n' ; ++il ,++id) {
            if(ptr[il]==',') {buf[id++]=0;buf[id]=0;fld[++ip]=buf+id+1;}
            else if(ptr[il]=='|')buf[id]=0;
            else buf[id]=ptr[il];
        }
        buf[id]=0;
        buf[id+1]=0;
        fld[ip+1]=0;
        if(sIs(fld[0],"record")) {
            bool notfound=true;
            idx ctype=0;string_search(ctype, fld[2],ctypelist);if(notfound)continue;
            idx hashtype=0;string_search(hashtype, fld[3],hashtypelist);if(notfound)continue;

            idx baseRecordTypeIndex=addRecordType(fld[1],(eCType)(eCTypeIdx+ctype),(eHashType)(eHashTypeNone+hashtype), fld[4]);

            for( idx ifld=5; fld[ifld] ; ++ifld){
                idx aliasRecordTypeIndex=addRecordType(fld[ifld],(eCType)(eCTypeIdx+ctype),(eHashType)(eHashTypeNone+hashtype),fld[4]);
                aliasRecordTypes(baseRecordTypeIndex,aliasRecordTypeIndex);
            }

        }else if(sIs(fld[0],"relation")) {
            addRelationTypeVarg(0,fld[1], fld+2, 0);

        }
    }

    return 1;
}


idx sIon::providerLoad(providerCallback providerFunc, void * param , idx maxcnt, idx start,idx flags)
{
    if(!maxcnt)maxcnt=sIdxMax;
    idx cntRecordTypes=recordTypesArr.dim();
    idx cntRelationTypes=relationTypesArr.dim();

    idx record=start;


    sVec < idx > recordIndexes;recordIndexes.resize(2*(cntRecordTypes+1));
    idx * recordIndexSet=recordIndexes.ptr(cntRecordTypes);
    sStr bubuf;

    for(idx iR=0; iR<maxcnt; ++iR) {

        for( idx it=0; it<cntRecordTypes ; ++it) {
            recordIndexes[it]=sNotIdx;
        }
        for( idx it=0, iT=0; it<cntRecordTypes ; ++it) {
            RecordType * pRecordType=recordTypesArr.ptr(it);
            const char * recordTypeName=(const char * ) baseContainer.ptr(pRecordType->nameOfs);

            if(recordTypeName[0]=='#' && recordTypeName[2]==0) {
                if(recordTypeName[1]=='r'){
                    recordIndexes[it]=iR;
                    continue;
                }
                else if(recordTypeName[1]=='t'){
                    recordIndexes[it]=it;
                    continue;
                }
            }

            const void * recordBody=0;
            idx recordSize;

            if(dicLoadConst.dim()) {
                    struct DicRecord * precordBody=(struct DicRecord *) dicLoadConst.get(recordTypeName,0,&recordSize);
                    if(precordBody){
                        recordBody=precordBody->recBody;
                        recordSize=precordBody->recSize;
                    }
            }
            if(!recordBody)
                record=providerFunc(record, param, iR, iT, recordTypeName, &recordBody, &recordSize );

            if(flags&sFlag(fValueFormatting) &&  pRecordType->typeValueSynonyms00) {
                const char * fmtMaintain=(const char*)baseContainer.ptr(pRecordType->typeValueSynonyms00);
                if(fmtMaintain[0]!='*' || fmtMaintain[1]!=0) {
                    bubuf.printf(0,fmtMaintain,(int)recordSize, recordBody);
                    recordBody=bubuf;
                    recordSize=sLen(bubuf);
                }

            }

            ++iT;
            if(record==eProviderDestroy)
                break;
            if(!recordBody){
                if (flags&sFlag(fDoNotLoadIncompleteRelation))break;
                else continue;
            }

            if(sConvPtr2Int(recordBody)==sNotIdx) {
                recordBody=(const void*)&record;
                recordSize=sizeof(record);
            }
            if(!recordSize && (flags&sFlag(fDoNotLoadEmpty) ) )
                continue;
            recordIndexes[it]=addRecord(it,recordSize,recordBody);
        }
        if(record==eProviderDestroy)
            break;

        for( idx it=0; it<cntRelationTypes ; ++it) {
            RelationType * pRelationType=relationTypesArr.ptr(it);
            idx iRelType;
            for ( iRelType=0; iRelType<pRelationType->cntRelationTargets; ++iRelType ) {
                idx relationTarget=relationTargetsArr[pRelationType->ofsRelationTargets+iRelType];
                idx relationTargetRecordIndex=recordIndexes[relationTarget];
                if(relationTargetRecordIndex==sNotIdx)
                    break;
                recordIndexSet[iRelType]=relationTargetRecordIndex;
            }
            if( iRelType<pRelationType->cntRelationTargets)
                continue;
            recordIndexSet[iRelType]=0;

            addRelationVarg(it,sNotIdx, recordIndexSet,0);
        }

        if(record==eProviderDestroy)
            break;

    }

    return record;
}



idx sIon::mergeIons(sIon * fromdb)
{
    idx sirius=0;

    if( !fromdb || !fromdb->ok() ) {
        return sirius;
    }
    sVec < idx > recordTypeMatchIndex;
    sVec < sVec< idx > >  recordIndexMatchTable;

    idx cntRecordTypes=fromdb->recordTypesArr.dim();
    for ( idx it=0 ; it<cntRecordTypes; ++it ) {

        RecordType * pRecordType=fromdb->recordTypesArr.ptr(it);
        if(pRecordType->cType==eCTypeIndexOnly)
            continue;


        const char * recordTypeName=(const char * ) fromdb->baseContainer.ptr(pRecordType->nameOfs);
        idx recordTypeIndex=addRecordType(recordTypeName,pRecordType->cType,pRecordType->hashType,(const char *)baseContainer.ptr(pRecordType->typeValueSynonyms00));

        recordTypeMatchIndex.vadd(1,recordTypeIndex);
        sVec < idx > * pRecordIndexMatchTable=recordIndexMatchTable.add();

        sVec <Record> * recordsArr;sIon_ensureContainerIon(fromdb,pRecordType,recordsArr);
        sMex * recordBodyContainer;sIon_ensureContainerIon(fromdb,pRecordType,recordBodyContainer);
        for ( idx ir=0 , cnt=recordsArr->dim(); ir<cnt; ++ir ) {

            Record * pRecord=recordsArr->ptr(ir);
            const void * pRecordBody=(pRecord->size<=(idx)sizeof(pRecord->ofs)) ? (void*)&(pRecord->ofs) : recordBodyContainer->ptr(pRecord->ofs);

            idx recordIndex=addRecord(recordTypeIndex,pRecord->size,pRecordBody);

            pRecordIndexMatchTable->vadd(1,recordIndex);

            ++sirius;
        }

    }


    sVec <idx> arr;
    idx cntRelationTypes=fromdb->relationTypesArr.dim();
    for ( idx it=0 ; it<cntRelationTypes; ++it ) {

        RelationType * pRelationType=fromdb->relationTypesArr.ptr(it);
        const char * relationTypeName=(const char * )fromdb->baseContainer.ptr(pRelationType->nameOfs);

        idx relationTypeIndex=recordAndRelationTypesHash.find(eRARHash_relation,relationTypeName,sLen(relationTypeName)+1);
        idx * pRelationTargetTypes=fromdb->relationTargetsArr.ptr(pRelationType->ofsRelationTargets);

        if(!relationTypeIndex) {

            arr.resize(pRelationType->cntRelationHashTypes+1);
            idx findex=1;
            for ( idx i=0; i<pRelationType->cntRelationHashTypes; ++i) {
                if(i<pRelationType->cntRelationTargets){
                    arr[i]=recordTypeMatchIndex[pRelationTargetTypes[i]];
                }
                else arr[i]=pRelationTargetTypes[i];
            }

            if(!findex)
                continue;
            relationTypeIndex=addRelationTypeVarg(pRelationType->flags,relationTypeName, (const char **  ) arr.ptr(0), 0, pRelationType->cntRelationHashTypes);
            RelationType * pThisRelationType=relationTypesArr.ptr(relationTypeIndex);
            pThisRelationType->cntRelationHashTypes=pRelationType->cntRelationHashTypes;
            pThisRelationType->cntRelationTargets=pRelationType->cntRelationTargets;

        } else
            --relationTypeIndex;





        sMex * relationsArr;sIon_ensureContainerIon(fromdb,pRelationType,relationsArr);


        idx is32bit=0;
        arr.resize(pRelationType->cntRelationTargets);
        for (idx relationsOfs=0; relationsOfs*(idx)sizeof(unsigned int)<relationsArr->pos(); relationsOfs+=pRelationType->cntRelationTargets*(is32bit ? 1: 2 ) ){
            idx * pRelationTargets=(idx *)relationsArr->ptr(relationsOfs*sizeof(unsigned int));
            is32bit =((((idx*)pRelationTargets)[0])&1) ? true : false ;

            for( idx iRelTarget=0; iRelTarget<pRelationType->cntRelationTargets; ++iRelTarget ) {
                idx relationTarget=is32bit ? (idx)(((unsigned int * )pRelationTargets)[iRelTarget]) : pRelationTargets[iRelTarget];
                if(iRelTarget==0)
                    relationTarget>>=1;
                idx recordIndexInDst=recordIndexMatchTable[ pRelationTargetTypes[iRelTarget] ][ relationTarget ];
                arr[iRelTarget]=recordIndexInDst;
            }


            addRelationVarg(relationTypeIndex,sNotIdx, arr,0);
        }


    }

    return sirius;
}




const char * sIon::getIonAtachmentName(sStr * flnm,const char * relationName, const char * suffix)
{

    idx relationTypeIndex=recordAndRelationTypesHash.find(eRARHash_relation,relationName,sLen(relationName)+1);
    if( !relationTypeIndex )
        return 0;
    else
        --relationTypeIndex;

    RelationType * pRelationType=relationTypesArr.ptr(relationTypeIndex);


    flnm->init(sMex::fExactSize);
    flnm->printf(0,"%s-%s-#%s.ion",baseName.ptr(),(const char *)baseContainer.ptr(pRelationType->nameOfsContainer), suffix );
    return flnm->ptr();

}






idx sIon::deleteIonContainers(const char * baseName)
{
    sDir lst;
    sFilePath dir(baseName,"%%dir"), flnm(baseName,"%%flnm*.ion*");
    idx sirius=0;

    lst.list(sFlag(sDir::bitFiles),dir.ptr(), flnm.ptr(),0, 0);
    for(const char * o=lst.ptr(0); o; o=sString::next00(o)){
        sFile::remove(o);
        ++sirius;
    }

    return sirius;
}


