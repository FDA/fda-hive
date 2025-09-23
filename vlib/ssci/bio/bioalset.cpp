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
#include <ssci/bio/bioal.hpp>

using namespace slib;


void sBioalSet::attach(sBioal * bioal, idx alNum, idx alCnt)
{
    if(!alCnt || alCnt>bioal->dimAl() )alCnt=bioal->dimAl();

    idx ib=0;
    for ( ib=biosR.dim()-1; ib>=0 && biosR[ib]!=bioal; --ib)
        ;
    if(ib<0){
        biosR.vadd(1,bioal);
        ib=biosR.dim()-1;


        idx cntSubs;
        for( idx it=0, ct=bioal->dimStat(); it<ct; ++it ){
            Stat * pS=bioal->getStat(it,0,&cntSubs);

            if(allStat.dim()<cntSubs)
                allStat.resize(cntSubs);

            for(idx is=0; is<cntSubs; ++is ) {
                if(pS[is].found) {
                    allStat[is].found+=pS[is].found;
                    allStat[is].foundRpt+=pS[is].foundRpt;
                }
            }

        }

        --cntSubs;
        if( dimSubCnt.dim()*2 < cntSubs ) {
            dimSubCnt.resize(2*cntSubs);
        }
        for(idx is=0; is<cntSubs; ++is ) {
            idx subCnt=0;
            idx plst=bioal->listSubAlIndex(is, &subCnt);
            if(subCnt ) {
                dimSubCnt[2*is]+=subCnt;
                if(dimSubCnt[2*is+1]==0)
                    dimSubCnt[2*is+1]=totDim+1+plst-1;
            }
        }


    }

    RefAl * rs=0;

    if( refs.dim()>0 ){
        rs=refs.ptr(refs.dim()-1);
        if( rs->bioNum==ib &&
            rs->alNum+rs->alCnt==alNum
            ){
            rs->alCnt+=alCnt;
        }
        else rs=0;
    }

    if( !rs ){
        rs=refs.add();
        rs->bioNum=ib;
        rs->alNum=alNum;
        rs->alCnt= alCnt;
    }

    totDim+=alCnt;

    needsReindex=true;
}


sBioal * sBioalSet::ref(idx * inum, idx iSub)
{
    idx ir;
    RefAl * rs;
    if( alInd.dim()) {
        ir=alInd[(*inum)];
        (*inum)=(ir&0xFFFFFFFF);
        rs=refs.ptr( (ir>>32)&0xFFFFFFFF );
        return biosR[rs->bioNum];
    }
    else {
        idx tDim=0;
        for(ir=0; ir<refs.dim(); ++ir ) {
            rs=refs.ptr(ir);
            if( (*inum)<tDim+rs->alCnt){
                (*inum)-=tDim;
                return biosR[rs->bioNum];
            }
            tDim+=rs->alCnt;
        }
    }

    return 0;
}


void sBioalSet::reindex(void)
{
    if(!needsReindex)return ;

    totDim=0;
    alInd.cut(0);

    for(idx ir=0; ir<refs.dim(); ++ir ) {
        RefAl * br=refs.ptr(ir);
        totDim+=br->alCnt;
    }

    idx * ind=alInd.add(totDim), inum=0;

    totDim=0;
    for(idx ir=0; ir<refs.dim(); ++ir ) {
        RefAl * rs=refs.ptr(ir);
        for( idx is=0 ; is<rs->alCnt ; ++is ) {
            ind[inum]=((ir&0xFFFFFFFF)<<32)|((rs->alNum+is-1)&0xFFFFFFFF);
            ++inum;
        }
        totDim+=rs->alCnt;
    }
    
    needsReindex=false;
}


