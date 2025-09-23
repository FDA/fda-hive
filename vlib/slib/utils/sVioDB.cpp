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
#include <slib/utils/sVioDB.hpp>
#include <slib/utils/sort.hpp>
#include <slib/utils/tbl.hpp>

#define sizeofTYPELIST(__additional)  (sizeof(sVioDB::TypeList)/sizeof(idx)+((__additional)-1)*sizeof(sVioDB::RLST)/sizeof(idx))

#define sizeofRECORDTYPE(__additional)  (sizeof(sVioDB::RecordType)/sizeof(idx)+((__additional)-1)*sizeof(sVioDB::LST)/sizeof(idx))

idx sVioDB::spaceForFlags=4;
idx sVioDB::pointerForCntType=sVioDB::spaceForFlags+0;
idx sVioDB::pointerForMaxRel=sVioDB::spaceForFlags+1;
idx sVioDB::pointerForMapSize=sVioDB::spaceForFlags+2;
idx sVioDB::pointerForMapOfs=sVioDB::spaceForFlags+3;
idx sVioDB::spaceForConstructInfo=16;
const char * sVioDB::vioDBFileMarker= "vioDB1.0-";

sVioDB::~sVioDB()
{
}



void sVioDB::AddType(sVioDB::ctype type,idx relCnt,idx * relationlist,const char * tname,idx indexOneBased,sVioDB::hashType hashtype )
{
    TypeList * typelist_type = (TypeList *)arr.add(sizeofTYPELIST(arr[pointerForMaxRel]));
    sVec <idx> * recordContainer=recordContainerArray.add(1);
    sVec <idx> * recordBodyContainer=recordBodyContainerArray.add(1);


    typelist_type->ctype=type;
    typelist_type->relCnt=relCnt;
    typelist_type->index=indexOneBased;
    sStr filenamebuf;
    filenamebuf.printf(0,"%s_recordContainer_%s",filename.ptr(), tname);
    recordContainer->init(filenamebuf.ptr(0),sMex::fSetZero|sMex::fBlockDoubling);
    recordContainer->cut(0);

    filenamebuf.printf(0,"%s_recordBodyContainer_%s",filename.ptr(), tname);
    recordBodyContainer->init(filenamebuf.ptr(0),sMex::fSetZero|sMex::fBlockDoubling);
    recordBodyContainer->cut(0);

    for(idx i=0; i<arr[pointerForMaxRel]; i++){
        if(i<relCnt)
        typelist_type->rels[i].typeIndex=*(relationlist+i);
        else
        typelist_type->rels[i].typeIndex=0;
        filenamebuf.printf(0,"%s_recordRelationContainer_%s_%" DEC "",filename.ptr(), tname,i);
        sVec <idx> * recordRelationContainer=recordRelationContainerArray.add(1);
        recordRelationContainer->init(filenamebuf.ptr(0),sMex::fSetZero|sMex::fBlockDoubling);
        recordRelationContainer->cut(0);
    }

    idx tnameSize=strlen(tname)+1;
    if(hashtype)    typelist_type->tnameSizeOrHash=hashtype;
    else typelist_type->tnameSizeOrHash=tnameSize;
    if(tnameSize <= (idx)sizeof(idx)) {
        strncpy((char *)&(typelist_type->tnameOfs), (const char *)tname,7);
    }

}

idx sVioDB::AddRecord(idx typeIndexOneBased, const void * record, const idx recordsize)
{
    TypeList * typelist_type = sVioDB::GetTypePointer(typeIndexOneBased);
    const idx myIndex = typelist_type->index - 1;
    sVec<idx> * recordContainer = recordContainerArray.ptr(myIndex);
    RecordType * recordtype_type = (RecordType *) recordContainer->add(sizeofRECORDTYPE(typelist_type->relCnt));
    switch(typelist_type->ctype) {
        case eInt:
            recordtype_type->ofs = *((idx *) record);
            break;
        case eString:
            {{
                void * ptr;
                const idx zeroAdd = ((char*) record)[recordsize - 1] == '\0' ? 0 : 1;
                if( recordsize <= (idx) (sizeof(idx) - zeroAdd) ) {
                    ptr = &(recordtype_type->ofs);
                } else {
                    const idx lenInIntegersAlloc = ((recordsize + zeroAdd - 1) / sizeof(idx)) + 1;
                    recordtype_type->ofs = typelist_type->bodyCnt + 1;
                    typelist_type->bodyCnt += lenInIntegersAlloc;
                    sVec<idx> * recordBodyContainer = recordBodyContainerArray.ptr(myIndex);
                    ptr = recordBodyContainer->add(lenInIntegersAlloc);
                }
                memcpy(ptr, record, recordsize);
            }}
            break;
        case eOther:
            {{
                void * ptr;
                if( recordsize <= (idx) sizeof(idx) ) {
                    ptr = &(recordtype_type->ofs);
                } else {
                    const idx lenInIntegersAlloc = ((recordsize - 1) / sizeof(idx)) + 1;
                    recordtype_type->ofs = typelist_type->bodyCnt + 1;
                    typelist_type->bodyCnt += lenInIntegersAlloc;
                    sVec<idx> * recordBodyContainer = recordBodyContainerArray.ptr(myIndex);
                    ptr = recordBodyContainer->add(lenInIntegersAlloc);
                }
                memcpy(ptr, record, recordsize);
            }}
            break;
    }
    recordtype_type->size = recordsize;
    typelist_type->cnt++;
    return typelist_type->cnt;
}

idx  sVioDB::AddRecordRelationshipCounter(idx typeIndexOneBased,idx recordIndexOneBased, idx relationIndexOneBased)
{

    TypeList * typelist_type = sVioDB::GetTypePointer(typeIndexOneBased);

    if(!recordIndexOneBased) recordIndexOneBased = typelist_type->cnt;
    idx myIndex=typelist_type->index;
    sVec <idx> * recordContainer=recordContainerArray.ptr(myIndex-1);

    RecordType * recordtype_type=(RecordType *)recordContainer->ptr((recordIndexOneBased-1)*sizeofRECORDTYPE(typelist_type->relCnt));
    recordtype_type->rels[relationIndexOneBased-1].cnt++;

    return recordtype_type->rels[relationIndexOneBased-1].cnt;
}


bool sVioDB::AllocRelation()
{
    idx typeCnt= *arr.ptr(pointerForCntType);
    idx MaxRel = *arr.ptr(pointerForMaxRel);

    for(idx i = 0; i < typeCnt; i++){
        TypeList * typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));

        for(idx j=0; j < typelist_type->relCnt; j++){
            idx totalcount=0;
            for ( idx k=0; k<typelist_type->cnt; ++k) {
                if(recordContainerArray.ptr(i)->dim()==0)continue;

                RecordType * recordtype_from=(RecordType *)recordContainerArray.ptr(i)->ptr(k*sizeofRECORDTYPE(typelist_type->relCnt));
                if(!recordtype_from->rels[j].cnt || recordtype_from->rels[j].cnt==1) continue;
                recordtype_from->rels[j].ofs=totalcount+1;
                totalcount+=recordtype_from->rels[j].cnt;
                recordtype_from->rels[j].cnt=0;
             }
             typelist_type->rels[j].cnt=totalcount;
             recordRelationContainerArray.ptr(i*MaxRel+j)->add(typelist_type->rels[j].cnt);
          }
    }
    return true;
}

bool sVioDB::AddRelation(idx typeIndexOneBased_from, idx typeIndexOneBased_to, idx reocrdIndexOneBased_from, idx recordIndexOneBased_to)
{
    TypeList * typelist_from = sVioDB::GetTypePointer(typeIndexOneBased_from);

    idx myIndex_from=typelist_from->index;

    sVec <idx> * recordContainer_from=recordContainerArray.ptr(myIndex_from-1);
    sVec <idx> * recordRelationContainer_from=recordRelationContainerArray.ptr((myIndex_from-1)*arr[pointerForMaxRel]+typeIndexOneBased_to-1);

    RecordType * recordtype_from=(RecordType *)recordContainer_from->ptr((reocrdIndexOneBased_from-1)*sizeofRECORDTYPE(typelist_from->relCnt));

    LST * r= (LST * ) &(recordtype_from->rels[typeIndexOneBased_to-1]);
    if(r->cnt==1 && !r->ofs){
        r->ofs=recordIndexOneBased_to;
    }
    else{
        *recordRelationContainer_from->ptr(r->ofs+r->cnt-1)=recordIndexOneBased_to;
        r->cnt++;
    }

    return true;
}

bool sVioDB::deleteAllJobs(bool ifJustUnmap)
{
    idx typeCnt = *arr.ptr(pointerForCntType);
    idx MaxRel = *arr.ptr(pointerForMaxRel);
    sStr filenamebuf;
    for(idx i = 0; (i < typeCnt) && (i < recordContainerArray.dim()); i++){
        TypeList * typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));

        const char * tname = (const char *)&typelist_type->tnameOfs;

        if(!ifJustUnmap){
            recordContainerArray[i].cut(0);
        }
        recordContainerArray[i].destroy();

        if(!ifJustUnmap){
            filenamebuf.printf(0,"%s_recordContainer_%s",filename.ptr(),tname);
            sFile::remove(filenamebuf.ptr(0));
        }

        if (i < recordBodyContainerArray.dim()){
            if(!ifJustUnmap){
                recordBodyContainerArray[i].cut(0);
            }
            recordBodyContainerArray[i].destroy();
            if(!ifJustUnmap){
                filenamebuf.printf(0,"%s_recordBodyContainer_%s",filename.ptr(),tname);
                sFile::remove(filenamebuf.ptr(0));
            }
        }
        for(idx k=0; k < MaxRel && k<recordRelationContainerArray.dim(); k++){

            if(!ifJustUnmap){
                recordRelationContainerArray[i*MaxRel+k].cut(0);
            }
            recordRelationContainerArray[i*MaxRel+k].destroy();

            if(!ifJustUnmap){
                filenamebuf.printf(0,"%s_recordRelationContainer_%s_%" DEC "",filename.ptr(), tname,k);
                sFile::remove(filenamebuf.ptr(0));
            }
        }

    }
    if(!ifJustUnmap){
        hashTable.cut(0);
    }
    hashTable.destroy();
    if(!ifJustUnmap){
        filenamebuf.printf(0,"%s_hash",filename.ptr());
        sFile::remove(filenamebuf.ptr(0));
    }

    if(!ifJustUnmap){
        arr.cut(0);
        arr.destroy();
        filenamebuf.printf(0,"%s",filename.ptr());
        sFile::remove(filenamebuf.ptr(0));
    }
    return true;
}

bool sVioDB::readonlyAllRecord(void)
{
    idx typeCnt = *arr.ptr(pointerForCntType);
    for(idx i = 0; (i < typeCnt) && (i < recordContainerArray.dim()); i++){

        recordContainerArray[i].remap(sMex::fReadonly);

        if (i < recordBodyContainerArray.dim()){
            recordBodyContainerArray[i].remap(sMex::fReadonly);
        }


    }
    hashTable.hashTableVec.remap(sMex::fReadonly);
    arr.remap(sMex::fReadonly);

    return true;
}
bool sVioDB::renameAllFiles(const char *newfilename)
{
    idx typeCnt = *arr.ptr(pointerForCntType);
    idx MaxRel = *arr.ptr(pointerForMaxRel);
    sStr filenamebuf;
    sStr newfilenamebuf;

    for(idx i = 0; i < typeCnt; i++){
        TypeList * typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));

        const char * tname = (const char *)&typelist_type->tnameOfs;

        filenamebuf.printf(0,"%s_recordContainer_%s",filename.ptr(),tname);
        newfilenamebuf.printf(0,"%s_recordContainer_%s",newfilename,tname);
        sFile::rename(filenamebuf.ptr(0), newfilenamebuf.ptr(0));

        filenamebuf.printf(0,"%s_recordBodyContainer_%s",filename.ptr(),tname);
        newfilenamebuf.printf(0,"%s_recordBodyContainer_%s",newfilename,tname);
        sFile::rename(filenamebuf.ptr(0), newfilenamebuf.ptr(0));

        for(idx k=0; k < MaxRel; k++){
            filenamebuf.printf(0,"%s_recordRelationContainer_%s_%" DEC "",filename.ptr(), tname,k);
            newfilenamebuf.printf(0,"%s_recordRelationContainer_%s_%" DEC "",newfilename, tname,k);
            sFile::rename(filenamebuf.ptr(0), newfilenamebuf.ptr(0));
        }

    }
    filenamebuf.printf(0,"%s_hash",filename.ptr());
    newfilenamebuf.printf(0,"%s_hash",newfilename);
    sFile::rename(filenamebuf.ptr(0), newfilenamebuf.ptr(0));
    filenamebuf.printf(0,"%s",filename.ptr());
    newfilenamebuf.printf(0,"%s",newfilename);
    sFile::rename(filenamebuf.ptr(0), newfilenamebuf.ptr(0));
    return true;
}

bool sVioDB::Finalize(bool ifGlueTheFile)
{
    idx typeCnt = *arr.ptr(pointerForCntType);
    idx MaxRel = *arr.ptr(pointerForMaxRel);
    if(ifGlueTheFile){
        arr[0]=1;
        idx totalsize=arr.dim();

        for(idx i = 0; i < typeCnt; i++){
            TypeList * typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));
            if(!typelist_type->cnt) continue;
            typelist_type->ofs=totalsize;

            arr.glue(recordContainerArray.ptr(i));
            typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));
             totalsize+=(typelist_type->cnt)*sizeofRECORDTYPE(typelist_type->relCnt);
             recordContainerArray[i].cut(0);
             recordContainerArray[i].destroy();
        }
        for(idx i = 0; i < typeCnt; i++){
            for(idx k=0; k < MaxRel; k++){
                TypeList * typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));
                if(!typelist_type->rels[k].cnt) continue;
                typelist_type->rels[k].ofs=totalsize;
                arr.glue(recordRelationContainerArray.ptr(i*MaxRel+k));
                typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));
                 totalsize+=typelist_type->rels[k].cnt;
                 recordRelationContainerArray[i*MaxRel+k].cut(0);
                 recordRelationContainerArray[i*MaxRel+k].destroy();
            }
        }
        arr[pointerForMapOfs]=totalsize;
        arr[pointerForMapSize]=hashTable.hashMemSize()/sizeof(idx);
        if(arr[pointerForMapSize]) {
            idx * ptr=arr.add(arr[pointerForMapSize]);
            if(ptr)memcpy((void*)ptr,(void*)hashTable.hashMem(),arr[pointerForMapSize]*sizeof(idx));
            totalsize+=arr[pointerForMapSize];
        }


        for(idx i = 0; i < typeCnt; i++){
            TypeList * typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));
            if(!typelist_type->bodyCnt) continue;
            arr.glue(recordBodyContainerArray.ptr(i));
            typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));
            typelist_type->bodyofs=totalsize;
            totalsize+=recordBodyContainerArray.ptr(i)->dim();
            recordBodyContainerArray[i].cut(0);
            recordBodyContainerArray[i].destroy();
        }
    }

    for(idx i = 0; i < typeCnt; i++){
        TypeList * typelist_type=(TypeList *)arr.ptr(spaceForConstructInfo+i*sizeofTYPELIST(MaxRel));
        if(!typelist_type->tnameOfs) continue;
        const char * tname = (const char *)&typelist_type->tnameOfs;
        sStr filenamebuf;
        filenamebuf.printf(0,"%s_recordContainer_%s",filename.ptr(),tname);
        if(ifGlueTheFile || !sFile::size(filenamebuf.ptr(0))) sFile::remove(filenamebuf.ptr(0));
        filenamebuf.printf(0,"%s_recordBodyContainer_%s",filename.ptr(),tname);
        if(ifGlueTheFile || !sFile::size(filenamebuf.ptr(0))) sFile::remove(filenamebuf.ptr(0));
        for(idx k=0; k < MaxRel; k++){
            filenamebuf.printf(0,"%s_recordRelationContainer_%s_%" DEC "",filename.ptr(), tname,k);
            if(ifGlueTheFile || !sFile::size(filenamebuf.ptr(0))) sFile::remove(filenamebuf.ptr(0));
        }
    }
    sStr filenamebuf;
    filenamebuf.printf(0,"%s_hash",filename.ptr());
    if(ifGlueTheFile || !sFile::size(filenamebuf.ptr(0))) sFile::remove(filenamebuf.ptr(0));
    return true;
}

idx sVioDB::GetRecordIndexByBody(const void * body, idx typeIndexOneBased, idx bodysize)
{
    TypeList * typelist_type = sVioDB::GetTypePointer(typeIndexOneBased);
    if(!bodysize) {
       if(typelist_type->ctype==eInt)
           bodysize=sizeof(idx);
       else bodysize=sLen(body);
    }
    if(typelist_type->tnameSizeOrHash<hNone){

        if(!hashTable.ok()){
            sStr filenamebuf; filenamebuf.printf("%s_hash",filename.ptr());
            hashTable.init(filenamebuf,fileOpenMode);
        }
        hashTable.keyParam=(void*)this;
        idx retval=hashTable.find(typeIndexOneBased, body, bodysize);
        return retval>0 ? retval-1 : 0;
    }
    else if(typelist_type->tnameSizeOrHash==hNone)  return 0;
    else if(typelist_type->tnameSizeOrHash==hInt){
        idx midIndex=0;
        for(idx lowerBound=1,uperBound = typelist_type->cnt;lowerBound<=uperBound;){
            midIndex=(lowerBound+uperBound)/2;
            idx bodyMid = *(idx *)Getbody(typeIndexOneBased,midIndex,0);
            if(*(idx *)body<bodyMid){
                uperBound=midIndex-1;
            }
            else if(*(idx *)body>bodyMid){
                lowerBound=midIndex+1;
            }
            else return midIndex;
        }
    }
    return 0;
}

bool sVioDB::SetRecordIndexByBody(const void * body,idx typeIndexOneBased, idx * recordIndexOneBased, idx bodysize)
{
    TypeList * typelist_type = sVioDB::GetTypePointer(typeIndexOneBased);

    if(!bodysize) {
        if(typelist_type->ctype==eInt)
            bodysize=sizeof(idx);
        else bodysize=sLen(body)+1;
    }

    if(!hashTable.ok()){
        sStr filenamebuf; filenamebuf.printf("%s_hash",filename.ptr());
        hashTable.init(filenamebuf,fileOpenMode);
    }
    idx recordindex=(recordIndexOneBased && *recordIndexOneBased) ? *recordIndexOneBased : typelist_type->cnt+1;
    hashTable.keyParam=(void*)this;
    idx res=hashTable.map(recordindex,(idx)typeIndexOneBased, body, bodysize, &recordindex );
    if(recordIndexOneBased)*recordIndexOneBased=recordindex;
    return res ? true : false ;

    return true;
}


sVioDB::RecordType * sVioDB::GetRecordPointerByIndex(idx typeindex, idx recordIndex)
{
    TypeList * typelist_type=sVioDB::GetTypePointer(typeindex);
    RecordType * recordtype_type=0;

    sVec <idx> * recordContainer=recordContainerArray.ptr(typeindex-1);
    if(typelist_type && recordIndex>0 && recordIndex <=typelist_type->cnt){
        if(arr[0])    recordtype_type=(RecordType *)arr.ptr(typelist_type->ofs+(recordIndex-1)*sizeofRECORDTYPE(typelist_type->relCnt));
        else{
            sFil filenamebuf;
            if(recordContainerArray.dim()<(idx)typeindex){
                recordContainerArray.resize(typeindex);
            }
            recordContainer=recordContainerArray.ptr(typeindex-1);
            if(recordContainer->dim()==0){
                filenamebuf.printf(0,"%s_recordContainer_%s",filename.ptr(), (const char *)&typelist_type->tnameOfs);
                recordContainer->init(filenamebuf.ptr(0),fileOpenMode);
            }
            if((idx)recordContainer->dim()<(idx)((recordIndex-1)*sizeofRECORDTYPE(typelist_type->relCnt)))
                return 0;
            recordtype_type=(RecordType *)recordContainer->ptr((recordIndex-1)*sizeofRECORDTYPE(typelist_type->relCnt));
        }
    }
    return recordtype_type;
}

void * sVioDB::Getbody(idx typeindex, idx recordIndex, idx * bodysize)
{
    void * body=0;
    TypeList * typelist_type=sVioDB::GetTypePointer(typeindex);
    RecordType * recordtype_type=GetRecordPointerByIndex(typeindex, recordIndex);
    sVec <idx> * recordBodyContainer=recordBodyContainerArray.ptr(typeindex-1);

    if(typelist_type && recordtype_type){
        if(bodysize ) *bodysize = recordtype_type->size;
        if(recordtype_type->size <= 8) body = (void *)&recordtype_type->ofs;
        else {
            if(arr[0]) body = (void *)arr.ptr(typelist_type->bodyofs+recordtype_type->ofs-1);
            else {
                if(recordBodyContainerArray.dim()<(idx)typeindex){
                    recordBodyContainerArray.resize(typeindex);
                }
                recordBodyContainer=recordBodyContainerArray.ptr(typeindex-1);
                if(recordBodyContainer->dim()==0){
                    sStr filenamebuf;
                    filenamebuf.printf(0,"%s_recordBodyContainer_%s",filename.ptr(), (const char *)&typelist_type->tnameOfs);
                    recordBodyContainer->init(filenamebuf.ptr(0),fileOpenMode);
                }
                if((idx)recordBodyContainer->dim()<recordtype_type->ofs-1)
                    return 0;
                body = (void *)recordBodyContainer->ptr(recordtype_type->ofs-1);
            }
        }
    }
    return body;
}

idx * sVioDB::GetRelationPtr(idx typeindex, idx recordIndex, idx relationIndex, idx * relationCnt, idx * relationTypeIndex)
{
    idx * relationPtr=0;
    TypeList * typelist_type=sVioDB::GetTypePointer(typeindex);
    RecordType * recordtype_type=GetRecordPointerByIndex(typeindex, recordIndex);
    if(!recordtype_type)return 0;
    if(recordtype_type->rels[relationIndex-1].cnt==0){
        return 0;
    }

    idx containerPtr = (typeindex-1)*(*arr.ptr(pointerForMaxRel))+ relationIndex-1;
    sVec <idx> * recordRelationContainer=recordRelationContainerArray.dim ()  ? recordRelationContainerArray.ptr(containerPtr) : 0;
    if(typelist_type && recordtype_type){
        if(relationCnt)*relationCnt = recordtype_type->rels[relationIndex-1].cnt;
           if(relationTypeIndex)*relationTypeIndex= typelist_type->rels[relationIndex-1].typeIndex;
           if(recordtype_type->rels[relationIndex-1].cnt == 1) relationPtr = &recordtype_type->rels[relationIndex-1].ofs;
        else{

            if(arr[0])    relationPtr = arr.ptr(typelist_type->rels[relationIndex-1].ofs+recordtype_type->rels[relationIndex-1].ofs-1);
            else{
                if(recordRelationContainerArray.dim() < (idx)containerPtr+1){
                    sStr filenamebuf;
                    recordRelationContainerArray.resize(containerPtr+1);
                    recordRelationContainer=recordRelationContainerArray.ptr(containerPtr);
                    filenamebuf.printf(0,"%s_recordRelationContainer_%s_%" DEC "",filename.ptr(), (const char*)&typelist_type->tnameOfs,relationIndex-1);
                    recordRelationContainer->init(filenamebuf.ptr(0),fileOpenMode);
                }
                if((idx)recordRelationContainer->dim()<recordtype_type->rels[relationIndex-1].ofs-1)
                    return 0;
                relationPtr=recordRelationContainer->ptr(recordtype_type->rels[relationIndex-1].ofs-1);
            }
        }
    }
    return relationPtr;
}

sVioDB::TypeList * sVioDB::GetTypePointer(idx typeindex)
{
    TypeList * typelist_type=0;
    if(typeindex <= arr[pointerForCntType]) typelist_type = (TypeList *)arr.ptr(spaceForConstructInfo+(typeindex-1)*sizeofTYPELIST(arr[pointerForMaxRel]));
    else return 0;
    return typelist_type;
}
idx sVioDB::GetTypeCnt(){
    return arr[pointerForCntType];
};

idx sVioDB::GetRecordCnt(idx typeIndexOneBased)
{
    TypeList * typelist_type=sVioDB::GetTypePointer(typeIndexOneBased);
    return typelist_type->cnt;
}

idx sVioDB::GetRelationCnt(idx typeIndexOneBased, idx recordIndexOneBased, idx relationIndexOneBased)
{
    idx relationCnt=0;
    TypeList * typelist_type=sVioDB::GetTypePointer(typeIndexOneBased);
    RecordType * recordtype_type=GetRecordPointerByIndex(typeIndexOneBased, recordIndexOneBased);
    if(typelist_type && recordtype_type)
        relationCnt = recordtype_type->rels[relationIndexOneBased-1].cnt;
    return relationCnt;
}






idx sVioDB::concatTypes(sVioDB * src )
{

    sVioDB * dst=this;


    idx typeCount=src->GetTypeCnt();
    for ( idx it=0; it<typeCount; ++it) {

        TypeList * typelist_type=src->GetTypePointer(it+1);
        sVec < idx > rels;rels.add(typelist_type->relCnt);

        for( idx j=0 ; j < typelist_type->relCnt; ++j )
            rels[j]=typelist_type->rels[j].typeIndex;

        dst->AddType((ctype)typelist_type->ctype,typelist_type->relCnt, (idx*)&(rels[0]) , (const char * )&(typelist_type->tnameOfs), typelist_type->index );

    }

    return typeCount;
}

idx sVioDB::concatRecordsAndRelationshipCounts(sVioDB * src , idx * countsForFileTypes )
{
    sVioDB * dst=this;

    idx typeCount=src->GetTypeCnt();
    idx mySrcRecID,myDstRecID;
    for ( idx it=0; it<typeCount; ++it) {

        TypeList * typelist_type=src->GetTypePointer(it+1);

        idx recCount=src->GetRecordCnt(typelist_type->index);

        for ( idx ir=0; ir<recCount; ++ir) {

            idx bodysize=0;
            void * bodyptr=src->Getbody(typelist_type->index, ir+1, &bodysize);
            mySrcRecID=src->GetRecordIndexByBody(bodyptr, typelist_type->index ,bodysize);
            myDstRecID=0;

            if(mySrcRecID) {
                if( dst->SetRecordIndexByBody((const void *)bodyptr, typelist_type->index, &myDstRecID, bodysize) ){
                    dst->AddRecord(typelist_type->index,(const void *)bodyptr, bodysize);
                    if(countsForFileTypes)++countsForFileTypes[it];
                }
            }
            else {
                myDstRecID=dst->AddRecord(typelist_type->index,(const void *)bodyptr, bodysize);;
                mySrcRecID=ir+1;
                if(countsForFileTypes)++countsForFileTypes[it];
            }

            for ( idx il=0; il<typelist_type->relCnt; ++il ){
                idx relationCnt=src->GetRelationCnt(typelist_type->index, mySrcRecID, il+1);

                for( idx ik=0; ik<relationCnt; ++ik ){
                    dst->AddRecordRelationshipCounter(typelist_type->index, myDstRecID, il+1);
                }
            }
        }


    }
    return 1;
}


idx sVioDB::concatRelationships(sVioDB * src , idx * recordOffsetsForFileTypes )
{
    sVioDB * dst=this;

    idx typeCount=src->GetTypeCnt();
    for ( idx it=0; it<typeCount; ++it) {

        TypeList * typelist_type=src->GetTypePointer(it+1);

        idx recCount=src->GetRecordCnt(typelist_type->index);


        for ( idx ir=0; ir<recCount; ++ir) {

            for ( idx il=0; il<typelist_type->relCnt; ++il ){


                idx relationCnt=0;
                idx * relPtr=src->GetRelationPtr(typelist_type->index, ir+1, il+1, &relationCnt);

                idx bodysize=0;
                void * bodyptr=src->Getbody(typelist_type->index, ir+1, &bodysize);
                idx myNewFromRecID=dst->GetRecordIndexByBody(bodyptr,typelist_type->index, bodysize);
                if(!myNewFromRecID)myNewFromRecID=recordOffsetsForFileTypes[it]+ir+1;

                for( idx ik=0; ik<relationCnt; ++ik ){
                    idx myNewToRecID;
                    bodysize=0;
                    if(relPtr[ik]<0)
                        myNewToRecID=-relPtr[ik];
                    else {
                        bodyptr=src->Getbody(typelist_type->rels[il].typeIndex, relPtr[ik], &bodysize);
                        myNewToRecID=dst->GetRecordIndexByBody(bodyptr, typelist_type->rels[il].typeIndex, bodysize);
                        if(!myNewToRecID)
                            myNewToRecID=relPtr[ik]+recordOffsetsForFileTypes [typelist_type->rels[il].typeIndex-1];
                    }

                    dst->AddRelation(typelist_type->index, il+1, myNewFromRecID, myNewToRecID );
                }

            }
        }
    }
    return 1;
}


idx sVioDB::concatFiles( const char * dstfile, const char * filenames, const char * fileMarker,  bool ifGlueTheFile, bool removeOriginals)
{
    sStr filenames00;
    sString::searchAndReplaceSymbols(&filenames00,filenames,0,"," sString_symbolsBlank,(const char *)0,0,true,true,true,true);
    idx icnt=0;


    {
        sVec < sVioDB > srcList;


        for( const char * flnm=filenames00; flnm; flnm=sString::next00(flnm) ){
            srcList.add()->init(flnm,0,0,0,sMex::fReadonly);
            ++icnt;
        }
        sFile::remove(dstfile);
        this->fileOpenMode=sMex::fSetZero|sMex::fBlockDoubling;
        this->init(dstfile,fileMarker,srcList.ptr(0)->arr[pointerForCntType],srcList.ptr(0)->arr[pointerForMaxRel],sMex::fSetZero|sMex::fBlockDoubling);
        idx typeCount=concatTypes(srcList.ptr(0));

        *(this->userSpace8())=*(srcList.ptr(0)->userSpace8());

        sVec < idx > recordCounts(sMex::fSetZero|sMex::fBlockDoubling),  recordOffsets(sMex::fSetZero|sMex::fBlockDoubling);
        recordCounts.add(srcList.dim()*typeCount);
        recordOffsets.add(typeCount);

        for( idx i=0; i< srcList.dim(); ++i)
            concatRecordsAndRelationshipCounts(srcList.ptr(i), recordCounts+typeCount*i );

        AllocRelation();

        for( idx i=0; i< srcList.dim(); ++i) {
            concatRelationships(srcList.ptr(i),recordOffsets);
            for(idx io=0; io<recordOffsets.dim();++io )
                recordOffsets[io]+=recordCounts[typeCount*i+io];

        }

        Finalize(ifGlueTheFile);

        if(removeOriginals){
            for( const char * flnm=filenames00; flnm; flnm=sString::next00(flnm) ){
                sFile::remove(flnm);
            }
            for( idx i=0; i< srcList.dim(); ++i) {
                srcList.ptr(i)->deleteAllJobs();
            }
        }
    }
    return icnt;
}

idx sVioDB::ComparatorWrapper(void * param, void * arr, idx i1,idx i2 )
{
    sVioDB::MyParamStructure * MyparaStr=(sVioDB::MyParamStructure * )param;
    void * o1 =  MyparaStr->viodb->Getbody(MyparaStr->typeTo,*(((idx *)arr)+i1),0);
    void * o2 =  MyparaStr->viodb->Getbody(MyparaStr->typeTo,*(((idx *)arr)+i2),0);
    void * objSrc= MyparaStr->fromType ? MyparaStr->viodb->Getbody(MyparaStr->fromType,MyparaStr-> recordFrom,0) : 0 ;
    return MyparaStr->externalCallback(MyparaStr->externalParam, o1, o2 ,objSrc,*(((idx *)arr)+i1)-1,*(((idx *)arr)+i2)-1);
}

void sVioDB::viodDBSorter( idx typeIndexOneBased, idx relationIndexOneBased, sVioDBSorterFunction myCallback , void * myParam )
{

    idx cntRecord = GetRecordCnt(typeIndexOneBased);
    for(idx cr=0; cr < cntRecord; cr++){
        idx relationCnt=0;
        idx relationTypeIndex=0;
        idx * relPtr = GetRelationPtr(typeIndexOneBased, cr+1, relationIndexOneBased, &relationCnt, &relationTypeIndex);
        if(relationCnt<2)continue;

        vioDBRelationSorter(relPtr,myCallback,myParam,typeIndexOneBased,relationTypeIndex,cr+1,relationCnt);
    }
}


void sVioDB::vioDBRelationSorter (idx * relPtr, sVioDBSorterFunction myCallback , void * myParam, idx fromType, idx typeTo, idx recordFrom, idx relationCnt )
{
    MyParamStructure MyparaStr;
    MyparaStr.viodb=this;
    MyparaStr.fromType=fromType;
    MyparaStr.externalCallback = myCallback;
    MyparaStr.externalParam=myParam;

    MyparaStr.typeTo=typeTo;
    MyparaStr.recordFrom=recordFrom;

    sSort::sortSimpleCallback<idx>(ComparatorWrapper, &MyparaStr, relationCnt, relPtr);
}


void sVioDB::viodDBSorter( idx typeIndexOneBased, idx * indexes, sVioDBSorterFunction myCallback , void * myParam )
{
    MyParamStructure MyparaStr;
    MyparaStr.viodb=this;
    MyparaStr.fromType=typeIndexOneBased;
    MyparaStr.externalCallback = myCallback;
    MyparaStr.externalParam=myParam;

    idx cntRecord = MyparaStr.viodb->GetRecordCnt(typeIndexOneBased);
    sVec<idx> rPtr;
    rPtr.add(cntRecord);
    idx * relPtr=rPtr.ptr(0);
    for(idx cr=0; cr < cntRecord; cr++)
        relPtr[cr]=cr+1;

        MyparaStr.typeTo=typeIndexOneBased;
        MyparaStr.recordFrom=0;
        sSort::sortSimpleCallback<idx>(ComparatorWrapper,&MyparaStr, cntRecord, relPtr, indexes);
}

void * sVioDB::genericTableFileInitForVioDB(const char * fileBody, idx fileLength, const char * separtor, const char * filename)
{
    if (!separtor) separtor = "\t|\t" __;
    sTbl * tbl=new sTbl();
    tbl->parse((char *)fileBody, fileLength, sTbl::fSeparatorIsListStrings|sTbl::fAllowEmptyCells, separtor);
    return (void*)tbl;
}
void sVioDB::genericTableFileDestoryForVioDB(void* usrObj)
{
    sTbl * tbl=(sTbl*)usrObj;
    delete tbl;
}

idx sVioDB::genericTableParserForVioDB(void * usrObj, sVec<sVar> &buf, idx usrIndex, sDic< sVec<sStr> > & columnListDic, void * param)
{
    sTbl * tbl=(sTbl*)usrObj;
    idx row=usrIndex;

    if(row >= tbl->rows())
        return sNotIdx;
    buf.cut(0);
    buf.add(1);
    for ( idx icol=0;  icol<tbl->cols(); ++icol) {
        sStr n;
        sStr c;
        idx size=0,namesize=0;
        const void * cl=tbl->cell(row+1,icol,&size);
        const char * name=tbl->cell(0,icol,&namesize);
        if(name) n.add(name,namesize);
        n.add0();

        if(row+1==1 && !strncmp("parentC",name,namesize)) continue;

        if(!strncmp("rankC",n.ptr(),namesize) || !strncmp("nameC",n.ptr(),namesize)){
            c.add((const char *)cl,size);
            c.add0();
            buf[0].inp(n.ptr(),c.ptr(),size+1);
        }
        else buf[0].inp(n.ptr(),cl,size);
    }

    return row+1;


}


void sVioDB::parsefileToVioDBFormat(const char* controlFileName, const char * sVioDBType,  const char* inputfilename, bool combinedFile, sVec <genericParserForVioDB> * ParserFunctionList, sVec <genericFileInitForVioDB> * fileInitList, sVec <genericFileDestoryForVioDB> * fileDestroyList, const char * srcFileNameListCommaSep, const char * sep, void * param)
{



    sFil controlFile(controlFileName,sMex::fReadonly);
    if(!controlFile.length()){
        ::printf("Control file %s is unreadable\n",controlFileName);
        return;
    }
    sTbl controlTable;
    controlTable.parse(controlFile.ptr(), controlFile.length() , true,",");

    idx cntRows= controlTable.rows();
    sDic <COLUMN> colNameMap;
    sDic <TYPE> typeNameMap;
    sDic< sVec<sStr> > columnListDic;
    idx typeMaxRel =1;
    COLUMN *cPtr=0;
    TYPE *tPtr=0;

    const char * listCtype="eNone" _ "eInt" _ "eDouble" _ "eString" _ "eBool" _ "eOther" __;


    for(idx i=1;i<cntRows;i++){

        sStr buf1;
        controlTable.get(&buf1,i,1);
        cPtr =  colNameMap.set(buf1.ptr());
        sVec<sStr> * columnListDicPtr = columnListDic.set(buf1.ptr(),sLen(buf1.ptr()));

        idx relCnt=1;
        sStr buf2;
        controlTable.get(&buf2,i,4);
        sscanf(buf2.ptr(),"%" DEC,&relCnt);
        colNameMap[buf1.ptr(0)].colIndLIST.add(relCnt);
        colNameMap[buf1.ptr(0)].relIndLIST.add(relCnt);
        cPtr->relCnt = relCnt;

        sStr buf3;
        controlTable.get(&buf3,i,2);
        tPtr = typeNameMap.set(buf3.ptr());
        tPtr->relCnt += relCnt;
        if(tPtr->relCnt > typeMaxRel) typeMaxRel = tPtr->relCnt;
        cPtr->typeIndex = typeNameMap.find(buf3.ptr())-1;

        sStr buf4;
        idx ctype=-1;
        controlTable.get(&buf4,i,3);
        sString::compareChoice(buf4, listCtype,&ctype,false, 0,true);
        if(ctype==-1)   ctype=1;
        tPtr->ctype=ctype;
        cPtr->ctype=ctype;

        sStr buf5;
        controlTable.get(&buf5,i,5);


        sStr buf;
        sString::searchAndReplaceSymbols(&buf,buf5.ptr(),0,";\r",0, 0,true , true, false , true);
        const char * ptr=buf.ptr();
        idx j=0;
        for(; ptr ; ptr = sString::next00(ptr),j++){
            if(colNameMap.find(ptr)>0) colNameMap[buf1.ptr()].colIndLIST[j]=colNameMap.find(ptr)-1;
            else {
                colNameMap.set(ptr);
                colNameMap[buf1.ptr()].colIndLIST[j]=colNameMap.dim()-1;
            }
        }
        if(j<relCnt){
            if(relCnt == 1){
               colNameMap[buf1.ptr()].colIndLIST[j] = -1;
            }
        }
        sStr buf6;
        controlTable.get(&buf6,i,6);
        if(strncmp(buf6.ptr(0),"hash",4)==0)  colNameMap[buf1.ptr(0)].ifHash = hReg;
        else  if(strncmp(buf6.ptr(0),"no",2)==0)colNameMap[buf1.ptr(0)].ifHash = hNone;
        else  if(strncmp(buf6.ptr(0),"intHash",6)==0)colNameMap[buf1.ptr(0)].ifHash = hInt;
        tPtr->ifHash=colNameMap[buf1.ptr(0)].ifHash;

        sStr buf7;
        controlTable.get(&buf7,i,7);

        if(buf7.ptr(0) && buf7.length()){
            buf.cut(0);
            sString::searchAndReplaceSymbols(&buf,buf7.ptr(),0,";\r",0, 0,true , true, false , true);
            ptr=buf.ptr();
            j=0;
            for(; ptr ; ptr = sString::next00(ptr),j++){
                if(columnListDicPtr){
                    sStr * sStrPtr = columnListDicPtr->add(1);
                    sStrPtr->printf(0, "%s", ptr);
                }
            }
        }
    }


    idx colCnt = colNameMap.dim();
    for(idx i=0;i<colCnt;i++){
        COLUMN * colPtr = (COLUMN *)colNameMap.ptr(i);
        TYPE * typePtr = (TYPE *)typeNameMap.ptr(colPtr->typeIndex);
        idx relCnt = colPtr->colIndLIST.dim();
        for(idx j=0;j<relCnt;j++){
            idx relColIndex = colPtr->colIndLIST[j];
            if(relColIndex!=-1){
                COLUMN * relColPtr = (COLUMN *)colNameMap.ptr(relColIndex);
                idx * temPtr = typePtr->typeIndLIST.add(1);
                *temPtr = relColPtr->typeIndex+1;
                colPtr->relIndLIST[j]= typePtr->typeIndLIST.dim();
            }
            else {
                idx * temPtr = typePtr->typeIndLIST.add(1);
                *temPtr = 0+1;
                 colPtr->relIndLIST[j] = 1;
            }
        }
    }

    idx typeCnt=  typeNameMap.dim();


    sVioDB db(inputfilename,sVioDBType,typeCnt,typeMaxRel);

    for(idx i=0;i<typeCnt;i++){
        TYPE * typePtr = (TYPE *)typeNameMap.ptr(i);
        idx ctype = typePtr->ctype;
        idx relCnt = typePtr->relCnt;
        const char * typeName = (const char *)typeNameMap.id(i);
        idx index = i+1;
        db.AddType((sVioDB::ctype)ctype,relCnt,typePtr->typeIndLIST,typeName,index,(sVioDB::hashType)(typePtr->ifHash));
    }


     sStr nameListbuf;
     sString::searchAndReplaceSymbols(&nameListbuf,srcFileNameListCommaSep,0,",",0, 0,true , true, false , true);
     const char * srcName=nameListbuf.ptr();
     sVec<sVar> lineArray;


     idx runNumber = 1;

     for(idx initFileIndex=0,destroyFileIndex=0,parseFileIndex=0;srcName!=NULL;){
        sFil srcFile(srcName,sMex::fReadonly);
        const char * src=srcFile.ptr();


        void * usrObj = (*fileInitList->ptr(initFileIndex))(src, srcFile.length(),sep, srcName, param);
        if(initFileIndex < fileInitList->dim()-1) initFileIndex++;

        idx offset=0;
        idx index=0;
PERF_START("FIRST GO");
        for (  ; offset!=sNotIdx ;index++)
        {
            PERF_START("ParserFunction");

            lineArray.cut(0);
            offset=(*ParserFunctionList->ptr(parseFileIndex))(usrObj,lineArray,offset,columnListDic, runNumber, param);
            PERF_END();
            PERF_START("lineArray");

            for(idx vi = 0;vi<lineArray.dim();vi++){

                sVar &line = lineArray[vi];
                idx cntVar = line.dim();
                for(int j=0;j<cntVar;j++){
                    const char * vName = (char *)line.id(j);
                    COLUMN * cPtr = colNameMap.get(vName);
                    idx size=0;
                    const void * body= line.value(vName,0,&size);
                    idx buf=0;
                    if(body && cPtr->ctype==eInt) {
                        char buf1[60];
                        strncpy(buf1,(char*)body,size);
                        buf1[size]=0;
                        buf=atoi(buf1);
                        size = sizeof(buf);
                        body = (const void*)&buf;
                    }
                    if(body){
                        idx myRecordID =0;
                        if(cPtr->ifHash==hReg){
                            idx ifSet = db.SetRecordIndexByBody(body,cPtr->typeIndex+1, &myRecordID,size);
                            if(ifSet) db.AddRecord(cPtr->typeIndex+1,body,size);
                        }
                        else {
                            db.AddRecord(cPtr->typeIndex+1 ,body, size);
                            TypeList * typelist_type = db.GetTypePointer(cPtr->typeIndex+1);
                            myRecordID=typelist_type->cnt;
                        }

                        for(int k=0;k<cPtr->relCnt;k++){
                            idx colIndex = cPtr->colIndLIST[k];
                            if(colIndex==-1) break;
                            const char *c1Name = (const char *)colNameMap.id(colIndex);
                            const void * body1 = line.value(c1Name,0,0);
                            if(body1){
                                db.AddRecordRelationshipCounter(cPtr->typeIndex+1, myRecordID, cPtr->relIndLIST[k]);
                            }
                        }
                    }
                }
            }
            PERF_END();

        }
        if(parseFileIndex < ParserFunctionList->dim()-1) parseFileIndex++;

        (*fileDestroyList->ptr(destroyFileIndex))(usrObj);
       if(destroyFileIndex < fileDestroyList->dim()-1) destroyFileIndex++;

        srcName = sString::next00(srcName);
    }
PERF_END();
PERF_PRINT();
    db.AllocRelation();

    sString::searchAndReplaceSymbols(&nameListbuf,srcFileNameListCommaSep,0,",",0, 0,true , true, false , true);
    srcName=nameListbuf.ptr();
    runNumber = 2;

    for(idx initFileIndex=0,destroyFileIndex=0,parseFileIndex=0;srcName!=NULL;){
        sFil srcFile(srcName,sMex::fReadonly);
        const char * src=srcFile.ptr();


        void * usrObj = (*fileInitList->ptr(initFileIndex))(src, srcFile.length(),sep,srcName, param);
        if(initFileIndex < fileInitList->dim()-1) initFileIndex++;

        for ( idx offset=0, i=0, iRec=0 ; offset!=sNotIdx; i++)

        {
            lineArray.cut(0);
            offset=(*ParserFunctionList->ptr(parseFileIndex))(usrObj,lineArray,offset,columnListDic, runNumber, param);
            for(idx vi = 0;vi<lineArray.dim();vi++, ++iRec){
                sVar &line = lineArray[vi];
                idx cntVar = line.dim();
                for(int j=0;j<cntVar;j++){
                    idx size =0 ;
                    const char * vName = (char *)line.id(j);
                    COLUMN * cPtr = colNameMap.get(vName);
                    const void * body= line.value(vName,0,&size);
                    idx buf=0;
                    if(body && cPtr->ctype==eInt) {
                        char buf1[60];
                        strncpy(buf1,(char*)body,size);
                        buf1[size]=0;
                        buf=atoi(buf1);
                        size = sizeof(buf);
                        body = (const void*)&buf;
                    }
                    if(body){
                        idx myRecordID =0;
                        if(cPtr->ifHash==hReg){
                            myRecordID=db.GetRecordIndexByBody(body, cPtr->typeIndex+1,size);
                        }
                        else {
                            myRecordID = iRec+1 > db.GetRecordCnt(cPtr->typeIndex+1)?db.GetRecordCnt(cPtr->typeIndex+1):iRec+1;
                        }
                        for(int k=0;k<cPtr->relCnt;k++){
                            idx myRecordID1 =0;
                            idx size1=0;
                            idx colIndex = cPtr->colIndLIST[k];
                            if(colIndex==-1) break;

                            const char *c1Name = (const char *)colNameMap.id(colIndex);
                            COLUMN * c1Ptr = colNameMap.ptr(colIndex);

                            const void * body1 = line.value(c1Name,0,&size1);
                            idx buf1=0;

                            if(body1 && c1Ptr->ctype==eInt) {
                                char buf2[60];
                                strncpy(buf2,(char*)body1,size1);
                                buf2[size1]=0;
                                buf1=atoi(buf2);
                                size1 = sizeof(buf1);
                                body1 = (const void*)&buf1;
                            }

                           if(body1){
                                if(c1Ptr->ifHash==hReg){
                                    myRecordID1=db.GetRecordIndexByBody(body1, c1Ptr->typeIndex+1,size1);
                                }
                                else myRecordID1 = (iRec+1) > db.GetRecordCnt(c1Ptr->typeIndex+1) ? db.GetRecordCnt(c1Ptr->typeIndex+1):(iRec+1);
                                db.AddRelation(cPtr->typeIndex+1, cPtr->relIndLIST[k], myRecordID , myRecordID1 );
                            }
                        }
                    }
                }
            }
        }
        if(parseFileIndex < ParserFunctionList->dim()-1) parseFileIndex++;

        (*fileDestroyList->ptr(destroyFileIndex))(usrObj);
        if(destroyFileIndex < fileDestroyList->dim()-1) destroyFileIndex++;
        srcName = sString::next00(srcName);
    }
    db.Finalize(combinedFile);
}






