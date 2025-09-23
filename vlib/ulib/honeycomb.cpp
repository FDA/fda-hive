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

#include <ulib/honeycomb.hpp>

#define sHoneyComb_allIndexesHash 5
#define sHoneyComb_objNameHash 12
#define sHoneyComb_objNameValueHash 15
#define sHoneyComb_nameHash 19
#define sHoneyComb_nameValueHash 21
#define sHoneyComb_valueHash 24

#define sHoneyComb_objTypeHash 5
#define sHoneyComb_typeHash 8

sHoneyCombIon * sHoneyCombIon::init(const char * baseName, idx openMode )
{

    ion->init(baseName,openMode);

    objType=ion->addRecordType("obj",sIon::eCTypeIdx,sIon::eHashTypeIdx);
    typeType=ion->addRecordType("type",sIon::eCTypeString,sIon::eHashTypeString);
    nameType=ion->addRecordType("name",sIon::eCTypeString,sIon::eHashTypeString);
    pathType=ion->addRecordType("path",sIon::eCTypeString,sIon::eHashTypeString);
    valueType=ion->addRecordType("value",sIon::eCTypeString,sIon::eHashTypeString);


    objToTypeRelationshipIndex=ion->addRelationType("obj_type","obj" _ "type" __,
        "obj" __,
        "obj" _ "type" __,
        "type" __,
        (const char * )0);
    objToValueRelationshipIndex=ion->addRelationType("obj_prop","obj" _ "name" _ "path" _ "value" __,
        "#obj"_"#name"_"#path"_"#value" __,
        "obj" __,
        "obj" _ "name" __,
        "obj" _ "name" _ "value" __,
        "name" __,
        "name" _ "value" __,
        "value" __,
        (const char * )0);

    return this;
}


idx sHoneyCombIon::createObject(idx objID, idx * pObjIndex)
{
    return ion->addRecord(objType, sizeof(objID),(void*)&objID);
}
bool sHoneyCombIon::propSet(idx objID, const char * name, const char * path, const char * value)
{
    idx objIndex,nameIndex,pathIndex,valueIndex;
    idx objToValue;
    objIndex=ion->addRecord(objType, sizeof(objID), &objID);
    nameIndex=ion->addRecord(nameType, sLen(name)+1, (void*)name);
    pathIndex=ion->addRecord(pathType, sLen(path)+1, (void*)path);
    valueIndex=ion->addRecord(valueType, sLen(value)+1, (void*)value);


    objToValue=ion->addRelation(objToValueRelationshipIndex, objIndex, nameIndex, pathIndex, valueIndex);

    return true;
}





idx sHoneyCombIon::objListIterate(Locator * locator, bool isMult, const char * type, const char * name, const char * value , sIO * buf)
{

    idx relationshipToUse = (name || value ) ? objToValueRelationshipIndex : objToTypeRelationshipIndex ;
    if(locator->bucket==sNotIdx) {
        if(name || value ) {
            relationshipToUse=objToValueRelationshipIndex;
            if(name && value)
                locator->bucket=ion->getRelationBucketByHash(relationshipToUse,sHoneyComb_nameHash,  0,name,sLen(name)+1,value, sLen(value)+1,(const void * )0);
            else if(name)
                locator->bucket=ion->getRelationBucketByHash(relationshipToUse,sHoneyComb_nameValueHash,  0,name,sLen(name)+1,(const void * )0);
            else if(value)
                locator->bucket=ion->getRelationBucketByHash(relationshipToUse,sHoneyComb_valueHash,  0,name,sLen(name)+1,(const void * )0);
        } else if(type ) {
            locator->bucket=ion->getRelationBucketByHash(relationshipToUse,sHoneyComb_typeHash,  0,type,sLen(type)+1,(const void * )0);
        }
        if(locator->bucket==sNotIdx)
            return 0;
    }

    idx next_bucket;
    const idx * pObjID=0;

    do {

        const char * nname, * nvalue;

        if( isMult ) {

            nname=name;
            nvalue=value;

            do {
                bool more=false;
                const char * pnam=sString::next00(nname);
                const char * pval=sString::next00(nvalue);
                if(pnam){nname=pnam;more=true;}
                if(pval){nvalue=pval;more=true;}
                if(!more)
                    break;

                pObjID= (const idx * )ion->getRecordBody(objType, locator->indexObjId, 0);
                next_bucket=ion->getRelationBucketByHash(objToValueRelationshipIndex,sHoneyComb_objNameValueHash,  0,pObjID, sizeof(*pObjID), nname,sLen(nname)+1, nvalue, sLen(nvalue)+1, (const void * )0);

                if(next_bucket==sNotIdx) {
                    return sNotIdx;
                }
            }while(true);

        }

        if(type && relationshipToUse==objToValueRelationshipIndex) {
            next_bucket=ion->getRelationBucketByHash(objToTypeRelationshipIndex,sHoneyComb_objTypeHash,  0,pObjID, sizeof(*pObjID), type,sLen(type)+1, (const void * )0);
            if(next_bucket==sNotIdx) {
                continue;
            }
        }
        break;

    }while(locator->bucket);

    if(buf) {
        idx size;
        const char * nname,* nvalue, * npath;
        if(!pObjID)
            pObjID= (const idx * )ion->getRecordBody(objType, locator->indexObjId, &size);
        nname= (const char * )ion->getRecordBody(nameType, locator->indexName, &size);
        npath= (const char * )ion->getRecordBody(pathType, locator->indexPath, &size);
        nvalue= (const char * )ion->getRecordBody(valueType, locator->indexValue, &size);

        buf->printf("\nprop.%" DEC ".%s.%s=%s",*pObjID,nname,npath,nvalue);
    }
    return locator->bucket;


}



const char * sHoneyCombIon::propGet(sIO * buf, idx objID, const char * name, const char * path)
{

    idx bucket=0;

    if(bucket==sNotIdx) return 0;

    idx arr[12];
    const char * ret=0;
    do{

        idx size;
        const idx * objID= (const idx * )ion->getRecordBody(objType, arr[0], &size);
        const char * name= (const char * )ion->getRecordBody(nameType, arr[1], &size);
        const char * path= (const char * )ion->getRecordBody(pathType, arr[2], &size);
        const char * value= (const char * )ion->getRecordBody(valueType, arr[3], &size);
        if(buf)
            buf->printf("\nprop.%" DEC ".%s.%s=%s",*objID,name,path,value);
        if(!ret)ret=value;
    }while(bucket!=sNotIdx);
    return ret;
}






sHoneyCombSet * sHoneyCombSet::init(const char * baseNameList00, idx openMode)
{
    for(const char * p=baseNameList00; p ;p=sString::next00(p) ) {
        sHoneyComb * hc=hcList.set(p);
        if(!hc->ok())
            hc->init(p,openMode);
    }
    return this;
}


idx sHoneyCombSet::objListIterate(Locator * locator, bool isMult, const char * type, const char * name, const char * value , sIO * buf)
{

    for (idx i=locator->indexIon; i< hcList.dim() ; ++i ) {
        if(locator->bucket==sNotIdx)
            locator->indexIon=i;
        idx res=hcList[locator->indexIon].objListIterate(locator, isMult, type, name, value , buf);
        if(res!=sNotIdx)
            return sNotIdx;
        locator->bucket=sNotIdx;
    }
    return sNotIdx;
}





const char * sHoneyCombSet::propGet(sIO * buf, idx objID, const char * name, const char * path)
{
    for (idx i=0; i< hcList.dim() ; ++i ) {

    }
    return 0;
}



