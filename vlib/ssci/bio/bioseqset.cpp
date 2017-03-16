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
#include <slib/core.hpp>
#include <ssci/bio/bioseq.hpp>

using namespace slib;

void sBioseqSet::attach(sBioseq * bioseq, idx seqNum, idx seqCnt, idx partialRangeStart, idx partialRangeEnd)
{
    // adjust the number of sequences being pushed here
    idx seqlongCnt = seqCnt, seqshortCnt = seqCnt;
    if(!seqCnt || seqCnt>bioseq->dim() ){
        seqCnt=bioseq->dim();
    }
    seqlongCnt = bioseq->getlongCount();
    seqshortCnt = bioseq->getshortCount();
//    }

    // attach a new sBioseq to our sBioseqSet
    // we do it in reverse because the last element
    // will frequently be the one which is adding more sequences
    idx ib=0;
    for ( ib=biosR.dim()-1; ib>=0 && biosR[ib]!=bioseq; --ib)
        ;
    if(ib<0){ // couldn't find this sBioseq ... a new one, perhaps
        biosR.vadd(1,bioseq);
        ib=biosR.dim()-1;
    }

    // for select its last element
    RefSeq  * rs=0;

    if( refs.dim()>0 ){
        // compare if we can just concatenate this with the last element
        rs=refs.ptr(refs.dim()-1);
        if( rs->bioNum==ib &&  // same sequence
            rs->seqNum+rs->seqCnt==seqNum && // the next sequence in range is being added  ?
            rs->partialRangeStart==partialRangeStart && // same range start
            rs->partialRangeEnd==partialRangeEnd // same range end
            ){
            rs->seqCnt+=seqCnt;
            rs->longseqCnt += seqlongCnt;
            rs->shortseqCnt += seqshortCnt;
        }
        else rs=0;
    }

    if( !rs ){ // otherwise we add the value
        rs=refs.add();
        rs->bioNum=ib;
        rs->seqNum=seqNum;
        rs->seqCnt= seqCnt;
        rs->longseqCnt = seqlongCnt;
        rs->shortseqCnt = seqshortCnt;
        rs->partialRangeStart=partialRangeStart;
        rs->partialRangeEnd=partialRangeEnd;
    }

    totDim+=seqCnt;

    needsReindex=true;
}


sBioseq * sBioseqSet::ref(idx * inum, idx *seqCnt, idx mode)
{
    idx ir,cnt;
    RefSeq * rs;
    if( seqInd.dim() && mode<0 ) { // if it has been indexed
        ir=seqInd[(*inum)];
        (*inum)=(ir&0xFFFFFFFF);
        rs=refs.ptr( (ir>>32)&0xFFFFFFFF );
        return biosR[rs->bioNum];
    }
    else { // if it has not been indexed
        idx tDim=0;
        for(ir=0; ir<refs.dim(); ++ir ) {
            rs=refs.ptr(ir);
            cnt = ( (mode<0)?rs->seqCnt:(mode?rs->longseqCnt:rs->shortseqCnt));
            if( (*inum)<tDim+cnt){
                (*inum)-=tDim;
                (*inum)+= (rs->seqNum-1);
                return biosR[rs->bioNum];
            }
            tDim+=cnt;
            if (seqCnt){
                *seqCnt = tDim;
            }
        }
    }
    // If I could not find it, but is a vioseqlist (cause totDimVioseqlist is not 0) , then return pointer to biosR[0]
    if (totDimVioseqlist > 0){
        return biosR[0];
    }
    return 0;
}


void sBioseqSet::reindex(void)
{
    if(!needsReindex)return ;

    totDim=0;
    seqInd.cut(0);

    for(idx ir=0; ir<refs.dim(); ++ir ) {
        RefSeq * br=refs.ptr(ir);
        totDim+=br->seqCnt;
    }

    idx * ind=seqInd.add(totDim), inum=0;

    totDim=0;
    for(idx ir=0; ir<refs.dim(); ++ir ) {
        RefSeq * rs=refs.ptr(ir);
        for( idx is=0 ; is<rs->seqCnt ; ++is ) {
            ind[inum]=((ir&0xFFFFFFFF)<<32)|((rs->seqNum+is-1)&0xFFFFFFFF);
            ++inum;
        }
        totDim+=rs->seqCnt;
    }
    
    needsReindex=false;
}

idx sBioseqSet::refsList(idx inum)
{
    idx ir;
    RefSeq * rs;
    idx tDim=0;
    for(ir=0; ir<refs.dim(); ++ir ) {
        rs = refs.ptr(ir);
        if( (inum)<tDim+rs->seqCnt){
            return ir;
        }
        tDim+=rs->seqCnt;
    }
    return 0;
}
