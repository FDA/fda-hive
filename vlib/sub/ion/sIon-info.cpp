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
using namespace slib;


idx sIon::getRecordCount(idx recordTypeIndex)
{
    RecordType * pRecordType = recordTypesArr.ptr(recordTypeIndex);
    if(pRecordType->cType==eCTypeIndexOnly)
        return 0;

    sVec<Record> * recordsArr;sIon_ensureContainer(pRecordType, recordsArr);

    return recordsArr->dim();
}

const char  * sIon::info(idx whatToPrint, sIO * buf, const char * type, const char * useSort, idx start, idx cnt, sIon_InfoCallbacks * infoCallbacks)
{
    idx pos=buf->length();

    idx is=start;
    idx ie=cnt ? start+cnt : sIdxMax;

    idx donebycb;
    bool isCB=false;
    bool isCBT=false;
    bool isCBL=false;
    bool isCBLT=false;
    if(infoCallbacks){
        if (infoCallbacks->callbackRecord) isCB=true;
        if( infoCallbacks->callbackRecordType) isCBT = true;
        if (infoCallbacks->callbackRelation) isCBL=true;
        if( infoCallbacks->callbackRelationType) isCBLT = true;
    }


    if( isFlag(whatToPrint,fInfoRecordTypes) ) {
        idx cntRecordTypes=recordTypesArr.dim();



        for( idx i=0; i<cntRecordTypes; ++i) {
            RecordType * pRecordType=recordTypesArr.ptr(i);
            const char * tpname=(const char *)baseContainer.ptr(pRecordType->nameOfs);
            if(type && strcmp(tpname,type))
                continue;



            donebycb=0;
            if(isCBT){
                donebycb=infoCallbacks->callbackRecordType(this,buf,infoCallbacks->params,pRecordType);
            }
            if(!donebycb){
                buf->printf("record,%s,%s,%s",tpname,sString::next00(ctypelist,pRecordType->cType),sString::next00(hashtypelist,pRecordType->hashType));
            }

            if( isFlag(whatToPrint,fInfoRecordSummary) && pRecordType->cType!=eCTypeIndexOnly ) {
                sVec <Record> * recordsArr;sIon_ensureContainer(pRecordType,recordsArr);
                sMex * recordBodyContainer;sIon_ensureContainer(pRecordType,recordBodyContainer);
                real sz=(real)recordBodyContainer->pos();

                if(!donebycb){
                    const char * unit=sAlgo::sizeHuman(&sz);
                    buf->printf(",%" DEC ",%.1lf%s",recordsArr->dim(),sz,unit);
                }

                if( isFlag(whatToPrint,fInfoRecords) ) {

                    if(!isCBT) {
                        buf->printf("\n");
                    }

                    for( idx ir = is ; ir<recordsArr->dim() && ir<ie;++ir ){

                        Record * pRecord=recordsArr->ptr(ir);

                        const void * pRecordBody;
                        if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                            pRecordBody=(const void * ) &(pRecord->ofs);
                        } else {
                            pRecordBody=recordBodyContainer->ptr(pRecord->ofs);
                        }
                        if(isCB) {
                            infoCallbacks->callbackRecord(this,buf, infoCallbacks->params, pRecordType, pRecordBody, pRecord->size, ir, sNotIdx );
                        } else {
                            idx sz=sMin(pRecord->size,(idx)32);

                            idx hash=0;
                            getRecordByHash(pRecordType, pRecordBody, sz , &hash);

                            buf->printf("\t %" DEC "/#%" DEC " ",ir,hash);

                            sIon_outTextBodyInternal(buf,pRecordType->cType, pRecordBody, sz );
                            buf->printf("\n");
                        }
                    }
                }

            }
            if(!donebycb) {
                buf->printf("\n");
            }

        }

    }

    sStrT bb;
    if( isFlag(whatToPrint,fInfoRelationTypes) ) {

        idx cntRelationTypes=relationTypesArr.dim();
        for( idx i=0; i<cntRelationTypes; ++i) {
            RelationType * pRelationType=relationTypesArr.ptr(i);

            void * relationsOfsSet=0;
            sMex relOfs;
            if( useSort && isFlag(whatToPrint,fInfoRelations) ) {
                sStr flnm(sMex::fExactSize);
                flnm.printf(0,"%s-%s-#%s%s",baseName.ptr(),(const char *)baseContainer.ptr(pRelationType->nameOfsContainer), useSort, extension );
                relOfs.init( flnm.ptr() , openMode);
                if(relOfs.ok()) {
                    SortRelationsHeader * sh=(SortRelationsHeader * )relOfs.ptr(0);
                    relationsOfsSet=(idx*)relOfs.ptr(sh->shiftRelationOffsets);
                }
            }

            const char * tpname = (const char *)baseContainer.ptr(pRelationType->nameOfsContainer);
            if(type && strcmp(tpname,type))
                continue;

            idx * pRelationTargets=relationTargetsArr.ptr(pRelationType->ofsRelationTargets);
            RecordType * pRecordTypeInRelation;
            sMex * relationsArr;sIon_ensureContainer(pRelationType,relationsArr);

            donebycb=false;
            if(isCBLT) {
                donebycb=infoCallbacks->callbackRelationType(this,buf,infoCallbacks->params,pRelationType);
            }
            if(!donebycb){
                buf->printf("relation,%s",tpname);

                buf->printf(",");
                idx iRel=0;
                for (iRel=0; iRel<pRelationType->cntRelationTargets; ++iRel) {
                    pRecordTypeInRelation=recordTypesArr.ptr(pRelationTargets[iRel] );
                    if(iRel!=0)buf->printf("|");
                    buf->printf("%s",(const char *)baseContainer.ptr(pRecordTypeInRelation->nameOfs));
                }
                ++iRel;
                for (idx ic=0, it; pRelationTargets[iRel]!=sNotIdx; ++ic) {
                    buf->printf(",");
                    for(idx ik=0; (it=pRelationTargets[iRel])!=sNotIdx; ++ik,++iRel) {
                        if(ik!=0)buf->printf("|");
                        pRecordTypeInRelation=recordTypesArr.ptr(pRelationTargets[it]);
                        buf->printf("%s",(const char *)baseContainer.ptr(pRecordTypeInRelation->nameOfs));
                    }
                    ++iRel;
                }


                if( isFlag(whatToPrint,fInfoRecordSummary) ) {
                    real sz=(real)relationsArr->pos();
                    const char * unit=sAlgo::sizeHuman(&sz);
                    buf->printf(",%" DEC ",%.1lf%s",pRelationType->cntRelationsAll,sz,unit);
                }
            }
            bool is64Bit= (relationsArr->pos() >= (idx)(sizeof(int)*((idx)1<<32)) ) ? true : false ;


            if( isFlag(whatToPrint,fInfoRelations) ) {
                if( !donebycb){
                    buf->printf("\n");
                }

                idx is=start;
                idx ie=cnt ? start+cnt : sIdxMax;

                for( idx irPos = 0, il=relationsOfsSet ? is : 0 ; il<pRelationType->cntRelationsAll && il<ie; ++il ){
                    if( relationsOfsSet ) {
                        irPos=is64Bit ? ((idx * )relationsOfsSet)[il] : ((unsigned int * )relationsOfsSet)[il];
                        irPos*=sizeof(int);
                    }

                    idx * pRecordTargetIndexes=(idx*)relationsArr->ptr(irPos);
                    bool is32bit =((((unsigned int*)pRecordTargetIndexes)[0])&1) ? true : false ;


                    if(il>=is) {
                        idx  donebycbb=0;
                        if(isCBL) {
                            donebycbb=infoCallbacks->callbackRelation(this,buf, infoCallbacks->params, pRelationType, pRelationTargets, pRecordTargetIndexes , il-is );
                        }
                        if(!donebycbb){
                            bb.cut(0);
                            for( idx ir = 0 ; ir<pRelationType->cntRelationTargets;  ++ir ){
                                bool isdel=is32bit  ? ((idx)((((int * )pRecordTargetIndexes)[ir]))==((int)-1)) : (pRecordTargetIndexes[ir]==sNotIdx) ;
                                if(isdel) {
                                    bb.printf("//-deleted-");
                                    continue;
                                }

                                idx relationTargetIndex= is32bit ?  (idx)((((unsigned int * )pRecordTargetIndexes)[ir])) :pRecordTargetIndexes[ir] ;
                                if(ir==0)relationTargetIndex=((udx)relationTargetIndex)>>1;

                                RecordType * pRecordType=recordTypesArr.ptr(pRelationTargets[ir] );
                                if(pRecordType->cType==eCTypeIndexOnly){
                                    if(isCB) {
                                        infoCallbacks->callbackRecord(this,buf, infoCallbacks->params, pRecordType, &relationTargetIndex, (idx)sizeof(relationTargetIndex), il-is, ir);
                                    } else {
                                        buf->printf("//%" DEC,relationTargetIndex);
                                        bb.printf("// #%" DEC "[%" DEC "]",relationTargetIndex,(idx)sizeof(relationTargetIndex));
                                    }
                                    continue;
                                }
                                sVec <Record> * recordsArr;sIon_ensureContainer(pRecordType,recordsArr);

                                Record * pRecord=recordsArr->ptr(relationTargetIndex);
                                const void * pRecordBody;
                                if(relationTargetIndex <recordsArr->dim() ) {
                                    if(pRecord->size<=(idx)sizeof(pRecord->ofs)){
                                        pRecordBody=(const void * ) &(pRecord->ofs);
                                    } else {
                                        sMex * recordBodyContainer;sIon_ensureContainer(pRecordType,recordBodyContainer);
                                        pRecordBody=recordBodyContainer->ptr(pRecord->ofs);
                                    }
                                }
                                else {
                                    if(isCB) {
                                        infoCallbacks->callbackRecord(this,buf, infoCallbacks->params, pRecordType, 0, 0, il-is, ir );
                                    } else {
                                        buf->printf("//");
                                        sIon_outTextBodyInternal(buf,pRecordType->cType, "!\n", 2 );
                                        bb.printf("//#%" DEC "[%" DEC "]",relationTargetIndex,(idx)0);
                                    }
                                    continue;
                                }

                                if(isCB) {
                                    infoCallbacks->callbackRecord(this,buf, infoCallbacks->params, pRecordType, pRecordBody, pRecord->size, il-is, ir );
                                }else {

                                    idx sz=sMin(pRecord->size,(idx)32);

                                    buf->add("//",2);
                                    sIon_outTextBodyInternal(buf,pRecordType->cType, pRecordBody, sz );
                                    bb.printf("//#%" DEC "[%" DEC "]",relationTargetIndex,pRecord->size);
                                }
                            }
                        }
                    }
                    if(!relationsOfsSet)
                        irPos+=pRelationType->cntRelationTargets * (is32bit ? sizeof(int) : sizeof(idx));
                        if(il>=is) {
                            if(!isCBL && !isCB) {
                                buf->printf("// %" DEC "\t|\t%s\n",il,bb.ptr());
                            }
                        }
                }

            }
            if(!donebycb)
                buf->printf("\n");

        }

    }


    return buf->ptr(pos);
}

idx sIon::exportRecordTypeVax(sIon * ion, sIO * io, void * param, RecordType * pRecordType )
{
    io->add("# $ion ",7);
    return 0;

}
idx sIon::exportRelationTypeVax(sIon * ion, sIO * io, void * param, RelationType * pRelationType )
{
    io->add("# $ion ",7);
    return 0;
}
idx sIon::exportRecordVax(sIon * ion, sIO * io, void * param, RecordType * pRecordType, const void * pRecordBody, idx recordSize, idx iLine, idx iRecOrderInRelation )
{

    if(iRecOrderInRelation==0 ){
        if(iLine==0) {

        } else {
            io->add("\n",1);
        }
    }
    else if(iRecOrderInRelation>0)
        io->add(",",1);

    if(recordSize==0)
        io->add("*err*",5);
    else {
        sIon_outTextBodyInternal(io, pRecordType->cType, pRecordBody, recordSize );
    }

    return 0;
}
idx sIon::exportRelationVax(sIon * ion, sIO * io, void * param, RelationType * pRelationType , idx * pRelationTargets,idx * pRecordTargetIndexes , idx iLine )
{
    if(iLine!=0)return 0;

    idx iRel=0;
    for (iRel=0; iRel<pRelationType->cntRelationTargets; ++iRel) {
        RecordType * pRecordTypeInRelation=ion->recordTypesArr.ptr(pRelationTargets[iRel] );
        if(iRel!=0)io->add(",",1);
        const char * ptr=(const char *)ion->baseContainer.ptr(pRecordTypeInRelation->nameOfs);
        io->add(ptr,sLen(ptr));
    }
    io->add("\n",1);

    return 0;
}

sIon::sIon_InfoCallbacks sIon::vaxExporter(sIon::exportRecordTypeVax,sIon::exportRelationTypeVax,sIon::exportRecordVax,sIon::exportRelationVax,0);































































