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
#include <slib/utils/vtree.hpp>

using namespace slib;


struct RelationVTreeStruct {
    sIon::RecordResult * start_recs, * end_recs, * start, * end;
    sVec<sVTree::vTreeNode> state;
    idx srtStart,srtEnd,srtCurrent;
    sIon::RelationIterator iter;
    sIon * ion;
    sVec<idx> res;
    sVTree * vtree;
    idx * max;
    idx cnt,prev_ind;
    RelationVTreeStruct() {vtree=0;max=0;cnt=0;start_recs=0;end_recs=0;srtStart=0;srtEnd=0;srtCurrent=0;ion=0;start=0;end=0;prev_ind=sNotIdx;}
    ~RelationVTreeStruct(){
        if(vtree)
            delete vtree;
    }
};

bool sIon::getIonVTreeNode( void * rngPtr, void * maxPtr, idx ind, void * vnode, void * params ) {
    RelationVTreeStruct * treeParam = (RelationVTreeStruct*)params;
    sVTree::rangeNode * node=(sVTree::rangeNode * )vnode;
    --ind;

    if(node->start) {
        if( !treeParam->ion->getRelationBodyBySortIterator( &treeParam->iter, treeParam->start_recs, treeParam->cnt , 1 , ind ) )
            return false;
        node->start = (void *)treeParam->start_recs;
    }

    if(node->end) {
        if(treeParam->prev_ind!=sNotIdx && treeParam->prev_ind!=ind) {
            RecordResult * trs = treeParam->start_recs;
            treeParam->start_recs = treeParam->end_recs;
            treeParam->end_recs = trs;
        }
        if( !treeParam->ion->getRelationBodyBySortIterator( &treeParam->iter, treeParam->end_recs, treeParam->cnt , 1 , ind ) )
            return false;
        node->end = (void *)treeParam->end_recs;
    }

    if( node->max ){
        if(treeParam->max) {
            node->max = treeParam->max[ind];
        }
    }

    if(treeParam->prev_ind!=sNotIdx) {
        treeParam->prev_ind = ind;
    }
    return true;
}


idx sIon::ionVTreeComparator( void * vr1, void * vr2, void * params, idx compType = 0 ) {
    RelationVTreeStruct * treeParam = (RelationVTreeStruct*)params;

    RecordResult * r1 = (RecordResult *)vr1;
    RecordResult * r2 = (RecordResult *)vr2;

    idx res = 0;
    for( idx ir = 0 ; ir<treeParam->cnt;  ++ir, ++r1, ++r2 ){

        if(r1->ccType()==eCTypeString){
            if(r1->flags()&sFlag(sIon::bSortRelationNumeric) ) {
                {sIon_numericCmpHex(res,r1,r2);}
            } else {
                {sIon_bodyCmp(res,r1,r2);}
            }

        }else if(r1->ccType()==eCTypeIdx || r1->ccType()==eCTypeIndexOnly ) {
            res=(*(idx*)(r1->body))-(*(idx*)(r2->body));
        }else if (r1->ccType()==eCTypeIdxRange) {
            switch (compType){
                case sVTree::eNC_SS:
                    res=((*(idx*)(r1->body)>>32))-((*(idx*)(r2->body)>>32));
                    break;
                case sVTree::eNC_SE:
                    res=((*(idx*)(r1->body)>>32))-((*(idx*)(r2->body))&0xFFFFFFFF);
                    break;
                case sVTree::eNC_ES:
                    res=((*(idx*)(r1->body))&0xFFFFFFFF)-((*(idx*)(r2->body)>>32));
                    break;
                case sVTree::eNC_EE:
                    res=((*(idx*)(r1->body))&0xFFFFFFFF)-((*(idx*)(r2->body))&0xFFFFFFFF);
                    break;
                default:
                    res = 0;
                    break;
            }
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
    return res;
}

void sIon::setIonVTreeMax( void * rngPtr, void * maxPtr, idx ind, idx max_ind, void * params ) {
    --ind;
    RelationVTreeStruct * treeParam = (RelationVTreeStruct*)params;
    treeParam->max[ind] = max_ind;
}

idx sIon::buildVTree(const char * relationName, const char * sortName, const char * vtreeName, RecordResult * rStart, RecordResult * rEnd,idx cnt )
{

    if(!vtreeName)vtreeName = "vtree";
    sStr flnm;
    getIonAtachmentName(&flnm,relationName, vtreeName);
    sVec<idx> maxInd( sMex::fExactSize, flnm);


    sIon::RelationIterator it;
    getRelationSorterIterator(relationName, sortName, &it);
    maxInd.resize(it.cntRelations);

    RelationVTreeStruct treeParam;
    treeParam.cnt = cnt;
    treeParam.start_recs = rStart;
    treeParam.prev_ind = sIdxMax;
    treeParam.end_recs = rEnd;
    treeParam.iter = it;
    treeParam.ion = this;
    treeParam.max = maxInd.ptr();

    sVTree::vTreeCallbacksStruct treecallbacks;
    treecallbacks.compare = ionVTreeComparator;
    treecallbacks.setMax = setIonVTreeMax;
    treecallbacks.getNode = (sVTree::sCallbackVTreeGetNode)getIonVTreeNode;

    sVTree vtree(0,it.cntRelations, maxInd.ptr(), &treecallbacks);
    return vtree.fixMax( (void*)&treeParam );
}



idx sIon::getRelationBucketByRange(Bucket * bucket, idx relationTypeIndex, const char * engineID00, idx * pCnt, const idx * searchTraj, va_list marker, sVec < RecordResult > * resultSet,sDic < sMex > * ac)
{

    bucket->relationBucketPos=sNotIdx;
    sMex * buf=ac->ptr(0);
    bool found = true;

    RelationVTreeStruct * vt=sConvInt2Ptr( bucket->relationBucketPos, RelationVTreeStruct);
    if((void*)vt==sNotPtr){
        vt=new RelationVTreeStruct();
    }
    if( ac->dim()>1 ) {
        vt->iter.mex=ac->ptr(1);
    }
    SortRelationsHeader * sh=getRelationSorterIterator(relationTypeIndex,&vt->iter);

    resultSetForEngines.cut(0);
    idx cntRes=getResultSetFromSearchTrajectory(&resultSetForEngines,searchTraj, 0, resultSet);
    cntRes/=2;
    idx cntInquire=sh ? cntRes : -cntRes;
    resultSetForEngines.add(cntRes*3);



    idx iMin=0,iMax=vt->iter.cntRelations-1, iMid, cmp, iStart=iMin, iEnd=iMax;
    RecordResult * start=resultSetForEngines.ptr(0);
    RecordResult * end=start+cntRes;
    RecordResult * rMin=end+cntRes;
    RecordResult * rMid=rMin+cntRes;
    RecordResult * rMax=rMid+cntRes;


    iMin=0;getRelationBodyBySortIterator(&vt->iter, rMin, cntInquire , 1, iMin);

    for (idx i=0; i<2  ; ++ i ) {
        RecordResult * current= (i==0) ? start : end ;
        for(idx it=0; it<cntRes ; ++it ) {
            sIon_numerizeInBufBasedOnType(rMin[it].ccType(),&(current[it]),10,(buf),(sFlag(sIon::bSortCastNeeded)<<32) );
        }
    }
    for (idx i=0; i<2  ; ++ i ) {
        RecordResult * current= (i==0) ? start : end ;
        for(idx it=0; it<cntRes ; ++it ) {
            if(current[it].flags()&sFlag(sIon::bSortCastNeeded) ) {
                current[it].body=buf->ptr( sConvPtr2Int(current[it].body) );
                current[it].cType=current[it].ccType();
                if(current[it].ccType()==eCTypeIdxRange) {
                    unsigned int * hl=(unsigned int *)current[it].body;
                    hl[1]=hl[0];
                }
            }

        }
    }

    if( ac->dim()<=2 ) {
        for (idx i=0; i<2  ; ++ i ) {

            RecordResult * current= (i==0) ? start : end ;

            if(i)
                {iMin=0;getRelationBodyBySortIterator(&vt->iter, rMin, cntInquire , 1, iMin);}
            iMax=vt->iter.cntRelations-1;getRelationBodyBySortIterator(&vt->iter, rMax, cntInquire , 1, iMax);
            if(i==0)
                { cmp=sortRecordComparator(start, rMax, cntRes ) ; if(cmp>0)return sNotIdx; }

            do{
                iMid=(iMin+iMax)/2;
                getRelationBodyBySortIterator(&vt->iter, rMid, cntInquire, 1, iMid);
                cmp=sortRecordComparator(current, rMid, cntRes );

                if(cmp<0 || (i==0 && cmp==0) ){
                    if(iMax-iMin==1 )
                        { iMid=(i^(bool)cmp)?iMax:iMin; break;}
                    iMax=iMid;
                    rMax=rMid;
                }
                else if(cmp>0 || (i==1 && cmp==0 )){
                    if(iMax-iMin==1 )
                        { iMid= (i^(bool)cmp)?iMax:iMin; break;}
                    iMin=iMid;
                    rMin=rMid;
                }
            }while(iMin<iMax);
            if(i==0)iStart = iMid;
            else iEnd = iMid;

        }
        if(pCnt) {
            *pCnt=iEnd-iStart;
            if(*pCnt<0)*pCnt=0;
        }
        if(iStart>iEnd)
            found = false;
        else {
            vt->srtStart=iStart;
            vt->srtEnd=iEnd;
            vt->srtCurrent=vt->srtStart-1;
        }
    } else {
        sVTree::vTreeCallbacksStruct treecallbacks;
        treecallbacks.compare = ionVTreeComparator;
        treecallbacks.setMax = setIonVTreeMax;
        treecallbacks.getNode = (sVTree::sCallbackVTreeGetNode)getIonVTreeNode;

        sVTree * vtree = new sVTree(0,vt->iter.cntRelations, ac->ptr(2), &treecallbacks);
        vt->start = start;
        vt->end = end;
        vt->start_recs = rMin;
        vt->end_recs = rMax;
        vt->ion = this;
        vt->vtree = vtree;
        vt->max = (idx *)ac->ptr(2)->ptr(0);
        vt->cnt = cntRes;

        vt->vtree->search( (void *)start, (void *)end, vt->res, 1, &vt->state, (void *)vt );
        if(!vt->res.dim())
            found = false;
    }

    if(found)
        bucket->relationBucketPos=sConvPtr2Int(vt);
    else {
        found = false;
        if((void*)vt!=sNotPtr){
            delete vt;
        }
        bucket->relationBucketPos=sNotIdx;
    }

    return  bucket->relationBucketPos;
}




idx sIon::getRangeNextBucket(Bucket * bucket)
{
    RelationVTreeStruct * vt=sConvInt2Ptr( bucket->relationBucketPos, RelationVTreeStruct);
    idx relOfs = sNotIdx;
    bool isLast = false;
    if(vt->vtree) {
        if(vt->res.dim()){
            relOfs = sIon::getRelationOffsetsBySortIterator(&vt->iter, vt->res[vt->res.dim()-1]-1);
            vt->res.cut(vt->res.dim()-1);
            if(vt->state.dim()) {
                vt->vtree->search(vt->start,vt->end,vt->res,1,&vt->state,vt);
            }
        }
        if( !vt->res.dim() ) {
            isLast = true;
        }
    }
    else{
        ++vt->srtCurrent;
        if(vt->srtStart <= vt->srtEnd) {
            relOfs = sIon::getRelationOffsetsBySortIterator(&vt->iter, vt->srtCurrent);
        }
        if(vt->srtCurrent>=vt->srtEnd) {
            isLast = true;
        }
    }
    if(isLast) {
        if((void*)vt!=sNotPtr){
            delete vt;
        }
        bucket->relationBucketPos=sNotIdx;
    }
    return relOfs;
}






