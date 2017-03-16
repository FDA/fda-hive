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
#include <slib/std/file.hpp>
#include <ctype.h>

using namespace slib;

#define _costMatch(_v_sbits, _v_qbits) (((_v_sbits) != (_v_qbits)) ? costMismatch : costMatch)
#define QIDX(_v_iq, _v_len)    ((flags&fAlignBackward) ? ((_v_len)-1-(_v_iq)) : (_v_iq))
//#define SIDX(_v_is, _v_len)    ( ( (!(flags&fAlignCircular)) || (_v_is)<(_v_len) ) ? (_v_is) : ( (_v_is) -(_v_len) ) )
#define SIDX(_v_is, _v_len)    ( (_v_is) )

//sPerf bioPerf;
idx cntHashLookup=0,cntBloomLookup=0,cntAlignmentSW=0,cntTotLetters=0,  cntCrossPass=0;
idx maxHashBinFound=0;

//sPerf gPerf;

idx sBioseqAlignment::zeroAlignment[2]={0,0};
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Major Alignment Procedures
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx sBioseqAlignment::alignSeq(sVec < idx > * allHits , sBioseq * Subs, const char * qry, idx qrylen, idx idSub, idx idQry, idx flagset, idx * , idx * ) // subfos qryfos
{
    if(!computeDiagonalWidth)computeDiagonalWidth=bioHash.hdr.lenUnit/2;
    idx flags;

    // here we maintain what position is already scanned on a query for each subject 

    SubScanQPos.resizeM(Subs->dim()); // make sure there is enough space
    idx * subScanQPos=SubScanQPos.ptr();
    idx cntIn=0;

    for ( idx idir=0; idir < 2; ++idir)  {
        
        

PERF_START("PREP");
        
        idx leftToDo=0;
        for( idx iFnd=0; iFnd<SubScanQPos.dim(); ++ iFnd) {
            if( (flagset&fAlignKeepFirstMatch) && subScanQPos[iFnd]==sIdxMax )continue; // if we are intetrested in the first single hit - we do not wipe this flag out
            subScanQPos[iFnd]=0;
            ++leftToDo;
        }
        if(flagset&fAlignKeepFirstMatch && leftToDo==0)
            continue;
        //    ::printf("WARNING : %"_d_" left ot find\n",leftToDo);
        //if(!leftToDo)continue;
PERF_END();
        

        //SubScanQPos.set(0);

        idx mask = (idir==0) ? fAlignForward : fAlignBackward; // direction considering now
        if( !(flagset&mask) ) continue;  // this does not seem to be required to compute now
        flags=( flagset&(~(fAlignForward|fAlignBackward)) ) | mask;

        udx chash=0;//,chash2=0; // this is unsigned due to rotations sign digit overload
                
        for( idx iq=0; iq<qrylen; ++iq ) {

            if(iq && callbackFunc && (iq%100000)==0 ){
                if( callbackFunc(callbackParam, cntIn, iq*50/qrylen )==1 )return cntIn;
            }
            ++cntTotLetters;
            

            // prepare the hash for this position
            chash <<= 2;
            idx iqx=QIDX(iq,qrylen);
            char let=_seqBits(qry, iqx , flags);
            chash|=let;
            chash&=bioHash.hdr.hashMask;
            if(iq<bioHash.hdr.lenUnit-1)continue; // we still didn't accumulate enough letters
            
            // bloom filter if this hash even exists in table
            ++cntBloomLookup;
            if( (bioHash.hb [chash/8] & (((udx)1)<<(chash%8))) ==0 ){
                continue;
            }

            ++cntHashLookup;
            sBioseqHash::HashBinType * lst=bioHash.lst(chash);//, * list2;
            idx lstdim=bioHash.cnt(chash);//, lstdim2;

            if(maxHashBinFound<lstdim)maxHashBinFound=lstdim;

            // this is the position where the sequence has a hit of N-mer
            idx qrypos=iq-bioHash.hdr.lenUnit+1;
            // for all of the hits at this position
            for (  idx il=0; il<lstdim ; ++il ) {
                
                // get the match position
                sBioseqHash::RefPos & ref=(*lst)[il];
                idx refid=ref.id();
                
                if(idSub!=sNotIdx && refid!=idSub) continue; // this reference is to a different sequen
                if(qrypos<subScanQPos[refid])continue; //  this position is a defint miss - we tried extending this before 
                
                idx subpos=ref.pos();
                
PERF_START("LOAD");

                const char * sub=Subs->seq(refid);
                idx sublen=Subs->len(refid);
PERF_END();


PERF_START("EXTEND");

// extend on the left and right
                idx matchesAll=0, inShift=0;
                iqx=QIDX(qrypos,qrylen);
                char prvSubLet=_seqBits(sub, subpos , 0), prvQryLet=_seqBits(qry, iqx , flags); // the last matching position letters

                idx isub,iqry, iqryfirstnonmatch=qrypos+bioHash.hdr.lenUnit;
                for ( idx ext = -1; ext<2 ; ext+=2 ) { // extension directions left and right
                    
                    if(ext<0){isub=subpos-1;iqry=qrypos-1;} // the next position under consideration
                    else {isub=subpos+bioHash.hdr.lenUnit;iqry=qrypos+bioHash.hdr.lenUnit;}
                    idx diffs=0 ,gaps=0 , matches =bioHash.hdr.lenUnit, isubshift=0, iqryshift=0 ;

                    while( gaps<=(maxExtensionGaps+1) && diffs*100 < looseExtenderMismatchesPercent*(matches +diffs) ) { // maxExtensionGaps+1 in gaps because we are comparing a differential
                                                                        
                        if(ext>0 && (isub>=sublen || iqry>=qrylen) )break;
                        if(ext<0 && (isub<=0 || iqry<=0) )break;

                        iqx=QIDX(iqry,qrylen);
                        char qryLet = _seqBits(qry, iqx , flags);
                        char subLet = _seqBits(sub, isub , 0);

                        if(qryLet==subLet){
                            ++matches;gaps=0;
                            inShift=0;
                            iqryshift=isubshift=0;
                        }
                        else {
                            idx idxNextLet=iqry+ext;
                            char nextLet=0x0F;
                            if( iqryshift < maxExtensionGaps ) { 
                                if( idxNextLet>=0 && idxNextLet<qrylen ) // see if we can recover by an inserts or deletion ? 
                                    nextLet = _seqBits(qry, QIDX(( idxNextLet ) ,qrylen)  , flags);
                                if ( subLet == nextLet ) { // a match in the next letter of a query
                                    iqry=idxNextLet;
                                    qryLet=nextLet;
                                    ++iqryshift;
                                }
                            }
                            if( subLet!=nextLet && isubshift < maxExtensionGaps  ){ // try incrementing the subject 
                                nextLet=0x0F;
                                idxNextLet=isub+ext;
                                if( idxNextLet>=0 && idxNextLet<sublen ) // see if we can recover by an inserts or deletion ? 
                                    nextLet = _seqBits(sub, idxNextLet , 0);
                                if ( qryLet == nextLet ) { // a match in the next letter of a subject
                                    subLet=nextLet;
                                    isub=idxNextLet;
                                    ++isubshift;
                                }
                            }

                            if(qryLet==subLet){
                                ++matches;gaps=0;
                                if(inShift==0)++diffs; 
                                inShift=1;
                                ++diffs;
                            }
                            else { 
                                ++diffs;++gaps;
                                inShift=0;
                                iqryshift=isubshift=0;
                            }
                            if( ext>0 && iqryfirstnonmatch==qrypos+bioHash.hdr.lenUnit)iqryfirstnonmatch=iqry;
                        }
                        prvSubLet=subLet;
                        prvQryLet=qryLet;

                        isub+=ext; iqry+=ext;
                    }
                    matchesAll+=matches;
                }
                subScanQPos[refid]= iqryfirstnonmatch; // last considered query extension position on this subject 
                
PERF_END();

                    
                if( matchesAll < minMatchLen ){ // do not attempt to align smith watermann
                    //++filt; 
                    continue ;
                }
                
                idx curofs=allHits->dim(); // remember the working position
PERF_START("IDENT");
                if(flags&fAlignIdentityOnly){ // we do not need to run smith watermann here
                    allHits->addM(sizeof(Al)/sizeof(idx));
                    Al * hdr=(Al*)allHits->ptr(curofs);
                    sSet(hdr);
                    hdr->flags=flags;
                    hdr->dimAlign=0;
                    hdr->subStart=subpos;
                    hdr->qryStart=qrypos;
                    hdr->subEnd=isub;
                    hdr->qryEnd=iqry;
                    hdr->lenAlign=hdr->qryEnd-hdr->qryStart;
                    hdr->idSub=refid;
                    hdr->idQry=idQry;
                    ++cntIn;
                    subScanQPos[refid] = (flagset&fAlignKeepFirstMatch) ? sIdxMax: iqry; // last considered query extension position on this subject
                    continue;
                }
PERF_END();

PERF_START("SMWAT");


                idx lenBeforeNmerMatch=sMin ( qrypos, subpos) ; // the letters before the match cann not be on a negative side of either sequecne
                idx qryStart=qrypos-lenBeforeNmerMatch;
                idx subStart=subpos-lenBeforeNmerMatch;
                idx qryLenAlign= computeDiagonalWidth+sMin ( (sublen-subStart) , (qrylen-qryStart) ); // the length to be aligned cannot be behind the end of either sequence
                idx subLenAlign = qryLenAlign;
                if( qryLenAlign>qrylen)qryLenAlign=qrylen;
                if( subLenAlign>sublen)subLenAlign=sublen;

                //sVec <idx > * al=allHits->add();
                ++cntAlignmentSW;
                idx  matchcounters[4]; sSet(matchcounters,0,sizeof(idx)*4);
                idx okhit = alignSmithWaterman(allHits,sub, subStart, subLenAlign, qry, qryStart, qryLenAlign,  flags , sublen, qrylen , matchcounters);
                Al * hdr=(Al*)allHits->ptr(curofs);
                subScanQPos[refid] = hdr->qryEnd+1; // last considered query extension position on this subject
                
                if(okhit) {
                    hdr->idSub=refid;
                    hdr->idQry=idQry;
                    if(flagset&fAlignKeepFirstMatch) subScanQPos[refid]=sIdxMax ;
                    ++cntIn;
                }else {
                    allHits->cut(curofs);
                }


PERF_END();

            }

        }

    }
    return cntIn;
}



idx sBioseqAlignment::alignSmithWaterman( sVec < idx > * al, const char * sub, idx substart, idx sublen, const char * qry, idx qrystart, idx qrylen, idx flags , idx , idx qrybuflen, idx  * matchcounters) // subbuflen
{
//vioPerf.start("alignSW1");

//    #ifndef sBIO_ATGC_SEQ_2BIT
//        idx isComp=( (flags&fAlignForwardComplement) && (flags&fAlignForward) ) || ( (flags&fAlignBackwardComplement) &&(flags&fAlignBackward) ) ? true : false;
//    #endif
    if(!qrybuflen)qrybuflen=qrylen;


    idx reserve=1024*1024;
    idx maxSeq=qrylen*2+computeDiagonalWidth; /// the maximum number of sequence letters to be considered cannot be bigger than this.
    idx sz=(maxSeq+2)*(qrylen+1);if(sz<reserve)sz=reserve;
    MatSW.resizeM(sz*2); // make sure there is enough space to hold the matrix
    short int * matP=(short int *)MatSW.ptr();
//memset(matP,0,sz*sizeof(short int));
    sz=(maxSeq+1)*qrylen;if(sz<reserve)sz=reserve;
    MatBR.resizeM(sz); // this matrix doesn't have the zeroth elements , so it is one less
    char * matB=MatBR.ptr();
//memset(matB,0,sz*sizeof(char));

    // we keep a matrix of for smith waterman algorithm
    #define mat(_v_i, _v_j) matP[(qrylen+1)*(_v_i)+(_v_j)]
    #define bak(_v_i, _v_j) matB[(qrylen)*(_v_i)+(_v_j)]

    idx fv,cv=0;//,bestCasePotential;
    idx maxAll=0, maxAllLastLetter=0; // the maximal score
    idx maxS=0, maxQ=0, maxRowLastLetter=0, maxColLastLetter=0; // maxS,maxQ: the position of the maxAll

//    cellsComputed=0;
    idx qStart=0, qEnd=(flags&fAlignOptimizeDiagonal ) ? computeDiagonalWidth : qrylen; // definition of the diagonal width where we compute

    //DEBUG_OUT_SMITHWATERMANN_TABLE_HEADER

    idx is, iq;
    idx nonCompFlags=(flags&(~(fAlignBackwardComplement|fAlignForwardComplement)));

//vioPerf.end();
//vioPerf.start("alignSW2");

    for(  is=0; is<sublen; ++is ) {
//vioPerf.start("alignSW2 - 1 ");

        idx isx=SIDX( (substart+is) , subbuflen);
        #ifdef sBIO_ATGC_SEQ_2BIT
            idx sBits=_seqBits(sub, isx, nonCompFlags ); // this is the current sequence letter (bits) on the subject
        #else
            idx sBits=sub[isx];
        #endif

        idx maxRow=0;
        idx maxRowPos=0;

//vioPerf.end();
//vioPerf.start("alignSW2 - 2 ");
        for( iq=qStart; iq<qEnd; ++iq ) {

            idx iqx=QIDX((qrystart+iq),qrybuflen);

            //#ifdef sBIO_ATGC_SEQ_2BIT
            idx qBits=_seqBits(qry, iqx, flags)  ; // this is the current sequence letter (bits) on the query
            //#else
            //    idx qBits= ( isComp ) ?  sBioseq::mapComplementATGC[(idx)qry[iqx]] : qry[iqx];
            //#endif


            cv=mat(is+1,iq+1);

            // consider a match
            fv=mat(is,iq)+_costMatch( sBits , qBits   );
            if( cv<=fv ) {cv=fv; bak(is,iq)=3; };// if(is && iq)back(is-1,iq-1)|=0x30; }
            // consider insertion
            fv=mat(is,iq+1)+ ( (is==0 || bak(is-1,iq)==3) ? costGapOpen : costGapNext);//_costMatch(-1, sBits);
            if( cv<fv ) {cv=fv; bak(is,iq)=1; }// if(is)back(is-1,iq)|=0x30; }
            // consider deletion
            fv=mat(is+1,iq)+( (iq==0 || bak(is,iq-1)==3) ? costGapOpen : costGapNext);//_costMatch(sBits,-1);
            if( cv<fv ) {cv=fv; bak(is,iq)=2;};// if(iq)back(is,iq-1)|=0x20; }

            // is still better to be aligned or start a new one ?
            if(cv>0){
                mat(is+1,iq+1)=(short int)cv;

                if(cv>=maxRow){ // we remember the Rows maximum value and its position for local alignment
                    maxRow=cv; // it will be useful for exit criteria
                    maxRowPos=iq;
                }
            }
        }
//vioPerf.end();
//vioPerf.start("alignSW2 - 3 ");

        // for global alignments we remember the max score and row for the last letter (all sequence is aligned)
        if( flags&fAlignGlobal) {
            if( maxAllLastLetter< cv ) {
                maxAllLastLetter=cv;
                maxRowLastLetter=is; // qEnd-1; //is
                maxColLastLetter=qEnd-1; // qEnd-1; //is
            }
        }

        // we remember where the maximum score alignment starts
        if(maxAll<maxRow || ( (flags&fAlignMaxExtendTail) && maxAll==maxRow) ) { // for local alignment
            maxAll=maxRow;
            maxS=is;
            maxQ=maxRowPos;
        }

        if(flags&fAlignOptimizeDiagonal) {
            qStart=is-computeDiagonalWidth;
            if(qStart<0)qStart=0;
            qEnd=is+computeDiagonalWidth;
            if(qEnd>qrylen)qEnd=qrylen;
        }

        //DEBUG_OUT_SMITHWATERMANN_TABLE_ROW

        if(qStart>=qrylen-1)break;

    }
//vioPerf.end();
//vioPerf.start("alignSW3");

    //
    // traceback mechanism
    //
    //al->resizeM(sizeof(Al)/sizeof(idx)); // initialize the header
    idx curofs=al->dim();
    al->addM(sizeof(Al)/sizeof(idx));
    Al * hdr=(Al*)al->ptr(curofs);
    sSet(hdr);

    //hdr->scoreLocal=maxAll;
    //hdr->scoreGlobal=maxAllLastLetter;
    hdr->flags=flags;

    if(flags&fAlignGlobal) {
        maxAll=maxAllLastLetter;
        maxS=maxRowLastLetter;
        maxQ=maxColLastLetter;    // qrylen-1
        hdr->score=maxAllLastLetter;//hdr->scoreGlobal;
    }else
        hdr->score=maxAll;//hdr->scoreLocal;


    hdr->dimAlign=0; // to be counted a little later
    hdr->subStart=substart;
    hdr->qryStart=qrystart;
    hdr->subEnd=maxS+hdr->subStart; // the maximum subject window used for alignment : +1 is because maxS was an index
    hdr->qryEnd=maxQ+hdr->qryStart; // qryLen // the maximum query window used for alignment

    //
    // first we determine the length
    //
    idx dlen;
    for(dlen=0, is=maxS, iq=maxQ, cv=maxAll;  ; cv=mat(is+1,iq+1) , dlen+=2) {  // start walking back on backtrace pointers
        if(flags&fAlignGlobal){ if(iq<0 || is<0) break;} // break when we reach the first letter in global alignment
        else if(cv<=0) break;// break if we reach the 0 score in local alignment
        //else if(cv<=0 || is<0 || iq<0) break;// break if we reach the 0 score in local alignment

        char bw=bak(is,iq);
        if( (!(flags&fAlignMaxExtendTail)) && cv<costMatch ){ break;} // if we want compact alignment ... no reason to walk left without gaining anything

        if(bw==0x02)++matchcounters[2];
        else if(bw==0x01)++matchcounters[3];
        else if(cv<mat(is,iq))++matchcounters[1];
        else ++matchcounters[0];

        if(!bw) break;
        if(bw & 0x1)--is; // backstep
        if(bw & 0x2)--iq;
    }
    
    bool okhit=true;
    if( dlen<2*minMatchLen) okhit=false;
    else if(scoreFilter && hdr->score<scoreFilter ) okhit=false;
    else if( matchcounters[0]*200 < dlen*(100-maxMissQueryPercent)  ) okhit=false;
    if(!okhit) {
        //al->cut(curofs);
    } else {  

        idx * m=al->addM(dlen);//+(sizeof(Alignment::Hdr)/sizeof(idx)) ); // this is the real number of elements needed
        hdr=(Al*)al->ptr(curofs); // because it may have been reallocated

        bak(0,0)=3;
        hdr->dimAlign=dlen;


        for(dlen-=2, is=maxS, iq=maxQ, cv=maxAll;  dlen>=0 ; cv=mat(is+1,iq+1) , dlen-=2) {  // start walking back on backtrace pointers

            char bw=bak(is,iq);

            m[dlen]= bw==0x02 ? -1 : is; // here we remember the matching pairs: the fact that subject position goes after
            m[dlen+1]= bw==0x01 ? -1 : iq; // query position is reverted in the next loop during reversal of the array to forward order

            if(bw & 0x1)--is; // backstep
            if(bw & 0x2)--iq;
        }
        hdr->lenAlign=hdr->dimAlign;

        hdr->dimAlign=compressAlignment(hdr, m, m );
        al->cut(curofs+sizeof(sBioseqAlignment::Al)/sizeof(idx)+hdr->dimAlign);
    }

//vioPerf.end();
//vioPerf.start("alignSW4");


    //set back things to zero
    qStart=0;
    qEnd=(flags&fAlignOptimizeDiagonal ) ? computeDiagonalWidth : qrylen; // definition of the diagonal width where we compute
    for(  is=0; is<sublen; ++is ) {
        for( iq=qStart; iq<qEnd; ++iq ) {
            mat(is+1,iq+1)=0;
            bak(is,iq)=0;

            if(flags&fAlignOptimizeDiagonal) {
                qStart=is-computeDiagonalWidth;
                if(qStart<0)qStart=0;
                qEnd=is+computeDiagonalWidth;
                if(qEnd>qrylen)qEnd=qrylen;

            }
        }
    }

//vioPerf.end();


    return okhit ? 1 : 0 ;
    #undef mat
}


/*
idx sBioseqAlignment::findHSPs(sIndex <HSP > & hspTbl, const char * qry, idx qrylen, idx id, idx flagset, idx * subfos, idx * qryfos)
{
    idx flags=flagset;
    idx hspPara=(bioHash.hdr.lenUnit);

    idx oldHashList[32];

    //
    // now build HSPs
    //
    for ( idx idir=0; idir < 2; ++idir)  {

        idx mask = (idir==0) ? fAlignForward : fAlignBackward; // direction considering now
        if( !(flagset&mask) ) continue;  // this does not seem to be required to compute now
        flags=( flagset&(~(fAlignForward|fAlignBackward)) ) | mask;

        udx chash=0,chash2=0; // this is unsigned due to rotations sign digit overload

        for( idx iq=0; iq<qrylen; ++iq ) {
            ++cntTotLetters;

            // prepare the hash for this position
            chash <<= 2;
            idx iqx=QIDX(iq,qrylen);
            chash|=_seqBits(qry, iqx , flags);

            if(iq<bioHash.hdr.lenUnit-1)continue; // we still didn't accumulate enough letters
            chash&=bioHash.hdr.hashMask;

            if( flags&fAlignOptimizeDoubleHash ){
                chash2=oldHashList[iq%bioHash.hdr.lenUnit];
                oldHashList[iq%bioHash.hdr.lenUnit]=chash;
                if(iq<2*bioHash.hdr.lenUnit-1)continue; // we still didn't accumulate enough letters
            }

            //retrieve the list of hits from subject
            ++cntBloomLookup;
            if( (bioHash.hb [chash/8] & (((udx)1)<<(chash%8))) ==0 )
                continue;
            ++cntHashLookup;
            idx lstdim=bioHash.cnt(chash) ,lstdim2=0;
            //if(lstdim==0)continue; filtered by bloom;


            sBioseqHash::RefPos * lst=bioHash.lst(chash), * list2=0;

            if( flags&fAlignOptimizeDoubleHash ){ // bloom
                if( (bioHash.hb [chash2/8] & (((udx)1)<<(chash2%8))) !=0 ){ // bloom
                    lstdim2=bioHash.cnt(chash2);
                    list2=bioHash.lst(chash2);
                }
                else continue;
                //if(!lstdim2) continue;// filtered by bloom
            }

            idx qpos=iq-bioHash.hdr.lenUnit+1;

            // for all of the hits at this position
            for (  idx il=0; il<lstdim ; ++il ) {
                // get the match position
                sBioseqHash::RefPos & ref=lst[il];
                idx refid=ref.id();
                idx refpos=ref.pos();
                if(id!=sNotIdx && refid!=id) continue; // this reference is to a different sequence

                idx matchShape=0;
                if(subfos ){
                    idx * thissubfos=sBioseqHash::fos(subfos, refid );
                    //idx cntfos=*subfos; ++subfos;
                    idx pos=(refpos-1)/500+1;//, ps=pos-1, pe=pos+1;
                    //if(ps<0)ps=0;if(pe>=cntfos)pe=cntfos;
                    //for ( idx ip=ps ; ip<pe; ++ip ){ // check three fossils
                        idx sfos=thissubfos[pos];
                        idx qfos=qryfos[1];
                        if(idir)qfos>>=24;
                        //if(qryfos[1]&sfos)
                            matchShape=sfos;
                    //}
                }

                if( flags&fAlignOptimizeDoubleHash ) {
                    idx i2;
                    for( i2=0; i2<lstdim2 ; ++i2){
                        if( list2[i2]._pos == ref._pos-(bioHash.hdr.lenUnit) )break;
                    }
                    if(i2==lstdim2) continue; /// didn't find the second piece of a hash
                }

                ++cntHSPHit;
                HSP hsp;
                hsp.subPosExact=refpos;//ref.pos(); // the position where the first unit would have been aligned (not necessarily through an exact match)
                hsp.qryPosExact=qpos;
                hsp.refid=refid;//ref.id();
                hsp.flags=flags;
                hsp.defint=ref._pos-qpos;
                if(hsp.defint<0)hsp.defint=0;
                hsp.refcnt=0;
                hsp.defint=(hsp.defint/hspPara)*hspPara;
                if((flags&fAlignBackward))hsp.defint=-1-hsp.defint;

                hsp.matchShape=matchShape;



                idx r=hspTbl.add(hsp.defint,hsp);
                ++hspTbl[r].refcnt;

            }
        }
    }

    return hspTbl.dim();
}



idx sBioseqAlignment::alignfromHSPs(HSP * pr, sVec < sVec < idx >  > * hits , const char * sub, idx sublen , const char * qry, idx qrylen, idx id, idx flagset)
{
    idx pos=pr->subPosExact-pr->qryPosExact;
    idx revshift=0;
    if(pos<0) {revshift=-pos; pos=0;}

    // run Smith Waterman
    sVec <idx > * al=hits->add();
    idx flags=( flagset&(~(fAlignForward|fAlignBackward)) ) | (pr->defint<0 ? fAlignBackward : fAlignForward );

    if(!computeDiagonalWidth)computeDiagonalWidth=bioHash.hdr.lenUnit/2;

    idx slen=sublen-pos;
    if(slen>qrylen+computeDiagonalWidth)slen=qrylen+computeDiagonalWidth;
    ++cntAlignmentSW;
    alignSmithWaterman(al,sub, pos, slen, qry, revshift, qrylen-revshift,  flags , sublen, qrylen);

    Al * hdr=(Al *)al->ptr();
    hdr->idSub=id;

    return hits->dim();
}


*/









// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Manipulating Alignments
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


/*
idx sBioseqAlignment::alignmentSplit(const char * prefix, sVec < FILE* > * fps , Al * hdr, Al  * hdre, idx cntBestHit, idx subNum, idx start, idx cnt)
{
    sVec < sStr >  * strL = prefix  ? 0 : (sVec < sStr > * ) fps  ;// the pointer is FILE pointer and we work in file mode
    idx MM[2]={0,0};

    idx cntCurHit=0,idLastHit=-1,i;
    sStr t1;

    for ( i=0; hdr<hdre && i<cnt+start ; ++i) {
        if(i<start)continue;

        if(subNum!=sNotIdx && hdr->idSub!=subNum) continue; // particular subid is required
        if(hdr->idQry!=idLastHit) {idLastHit=hdr->idQry;cntCurHit=1;} // a new query sequence hits are here
        else ++cntCurHit;
        if(cntBestHit && cntCurHit>cntBestHit )
            continue;

        idx * m =hdr->lenAlign ? hdr->match() : MM;

        // open file or stream
        if(prefix){
            fps->resize(hdr->idSub+1);
            if( (*fps)[hdr->idSub]==0 ) {
                sFilePath mk("queryMap.vioal","%s%%pathx.%"_d_".%%ext",prefix,hdr->idSub);
                (*fps)[hdr->idSub]=fopen(mk.ptr(),"wrt");
                if(!(*fps)[hdr->idSub])(*fps)[hdr->idSub]=(FILE *)sNotPtr;
            }
            if((*fps)[hdr->idSub]==0 && (*fps)[hdr->idSub]==(FILE *)sNotPtr)continue;

            t1.cut(0);sBioseqAlignment::printf(&t1, hdr, m);
            if(t1.length())fprintf((*fps)[hdr->idSub],"%s", t1.ptr(0));
        }
        else {
            strL->resize(hdr->idSub+1);
            sBioseqAlignment::printf(strL->ptr(hdr->idSub), hdr, m);
        }

        hdr=sShift(hdr,hdr->sizeofFlat());
    }
    return i;
}
*/





/*
idx sBioseqAlignment::alignPostCompute(Al * hdr, const char * sub, const char * qry, idx subbuflen, idx qrybuflen)
{
    idx i, is , iq ;

    idx * m =hdr->match();
    hdr->matches=0;
    hdr->mismatches=0;
    hdr->inserts=0;
    hdr->deletions=0;
    idx flags=hdr->flags;
    idx nonCompFlags=((hdr->flags)&(~(fAlignBackwardComplement|fAlignForwardComplement)));

    idx isInsDel=0; // we keep track of the previous state so we count gap of multiple as one
    for( i=0 ; i<hdr->lenAlign; i+=2) {
        is=m[i];
        iq=m[i+1];

        if(is<0) {
            if(!isInsDel)
                ++hdr->inserts;
            ++isInsDel;
        }
        else if(iq<0){
            if(!isInsDel){
                ++hdr->deletions;
            }
            ++isInsDel;
        }
        else {
            isInsDel=0;
            idx iqx=QIDX( hdr->qryStart+iq, qrybuflen );
            idx isx=SIDX( (hdr->subStart+is) , subbuflen);

            char chS=sBioseq::mapRevATGC[(idx)_seqBits(sub, isx, nonCompFlags)];
            char chQ=sBioseq::mapRevATGC[(idx)_seqBits(qry, iqx , hdr->flags)];

            if( chS != chQ )
                ++hdr->mismatches;
            else
                ++hdr->matches;
        }
    }

    return hdr->lenAlign/2;
}

*/



idx sBioseqAlignment::viewAlignment(sStr * dst, Al * hdr, idx * m, const char * sub, const char * qry, idx , idx qrybuflen, const char * prefix, const char * id, const char * subqua, const char * qryqua) // subbuflen
{
    sVec < idx > uncompressMM;
    if(hdr->flags&fAlignCompressed){ // deal with compressed alignments
        uncompressMM.resize(hdr->lenAlign);
        sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
        m=uncompressMM.ptr();
    }

    idx i, is , iq ;

    idx flags=hdr->flags;
    idx nonCompFlags=((hdr->flags)&(~(fAlignBackwardComplement|fAlignForwardComplement)));
    
    sStr l1, l2, l3, l3comp;
    sStr l4,l5; // for qualities

    if(id){
        dst->printf("%s[%"_d_"]<->Query[%"_d_"] %s: score=%"_d_"\n",
            hdr->idSub>0 ? "Amplicon" :"Master", hdr->idSub,hdr->idQry, id,
            hdr->score);//,hdr->matches, hdr->mismatches, hdr->inserts, hdr->deletions );
    }

    if(prefix) {
        l1.printf("%s",prefix);
        l2.printf("%s",prefix);
        l3comp.printf("%s",prefix);
        l3.printf("%s",prefix);
        if(subqua) {
            l4.printf("%s",prefix);
        }if(qryqua) {
            l5.printf("%s",prefix);
        }
    }

    l1.printf("subject[%7"_d_"] %7"_d_"-%7"_d_": ",hdr->idSub,hdr->subStart+m[0]+1,hdr->subEnd+1);
    l2.printf("                                : ");
    if( hdr->flags&fAlignBackward ) {
        l3.printf("query[%7"_d_"](-)%7"_d_"-%7"_d_": ", hdr->idQry,(qrybuflen-(hdr->qryStart+m[1])-1)+1 ,(qrybuflen-hdr->qryEnd-1)+1);
        l3comp.printf("query[%7"_d_"](-)%7"_d_"-%7"_d_": ", hdr->idQry,(qrybuflen-(hdr->qryStart+m[1])-1)+1 ,(qrybuflen-hdr->qryEnd-1)+1);
    }
    else l3.printf("query[%7"_d_"](+)%7"_d_"-%7"_d_": ", hdr->idQry,hdr->qryStart+m[1]+1,hdr->qryEnd+1);
    if(subqua){
        l4.printf("                                : ");
    }
    if(qryqua){
        l5.printf("                                : ");
    }

    for( i=0 ; i<hdr->lenAlign; i+=2) {
        is=m[i];
        iq=m[i+1];

        idx isx=SIDX( (hdr->subStart+is) , subbuflen);
        char chS=sBioseq::mapRevATGC[(idx)_seqBits(sub, isx,nonCompFlags)];
        idx iqx=QIDX( hdr->qryStart+iq, qrybuflen );
        char chQ=sBioseq::mapRevATGC[(idx)_seqBits(qry, iqx, hdr->flags)];
        char chQcomp=sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx)_seqBits(qry, iqx, hdr->flags)]];

        l1.printf("%c", is>=0 ? ( (iq<0 || chQ!=chS) ? tolower(chS) : chS ) :'-' );

        if(is<0)
            l2.printf("-");
        else if(iq<0)
            l2.printf(".");
        else if(chS==chQ)
            l2.printf("|");
        else
            l2.printf(" ");

        l3.printf("%c",iq>=0 ? ( (is<0 || chQ!=chS) ? tolower(chQ) : chQ) : '.' );
        l3comp.printf("%c",iq>=0 ? ( (is<0 || chQ!=chS) ? tolower(chQcomp) : chQcomp) : '.' );
        
        if(subqua) {
            if(sub[isx/8]&(((idx)1)<<(isx%8)))l4.printf("+");
            else l4.printf("-");
        }
        if(qryqua) {
            if(qryqua[iqx/8]&(((idx)1)<<(iqx%8)))l4.printf("+");
            else l5.printf("-");
        }
    }

    if(subqua)dst->printf("%s\n",l4.ptr());
    dst->printf("%s\n%s\n%s\n",l1.ptr(),l2.ptr(),l3.ptr());
    if( hdr->flags&fAlignBackward )
        dst->printf("%s\n",l3comp.ptr());
    if(qryqua)dst->printf("%s\n",l5.ptr());

    return hdr->lenAlign/2;
}





idx sBioseqAlignment::remapAlignment(Al * from, Al * to )
{
    idx * mfrom0=from->match();
    idx * mto0=to->match();
    idx cntfrom=from->lenAlign, cntto=to->lenAlign;
    idx ofsfrom=from->subStart, ofsto=to->qryStart;

    idx * mfrom, * mto, *mfromEnd=mfrom0+cntfrom, * mtoEnd=mto0+cntto;
    for( mfrom=mfrom0, mto=mto0+1; mfrom < mfromEnd && mto<mtoEnd; mfrom+=2 ) { // mto0+1 because in master alignment to amplicon , amplicon is the query

        // find the position  where we have and overlap of the two mappings on amplicon
        while ( mfrom<mfromEnd &&  (*mfrom)+ofsfrom < (*mto)+ofsto)   // TODO : sortsearch instead of scanning
            mfrom+=2;
        if(mfrom>=mfromEnd)break;
        while ( mto<mtoEnd && (*mfrom)+ofsfrom > (*mto)+ofsto)
            mto+=2;
        if(mto>=mtoEnd)break;

        if ( (*mfrom)+ofsfrom!=(*mto)+ofsto) // this position is not aligned
            *(mfrom+1)=-1; // query points to -1
        else {
            *mfrom=*(mto-1);
            if(*mfrom!=-1) {
                *mfrom+=-from->subStart;
                from->subEnd=*mfrom;
                from->lenAlign=mfrom-mfrom0;
            }
        }

        if( *(mfrom+1)!=-1){
             from->qryEnd=*(mfrom+1);
            from->lenAlign=mfrom-mfrom0+2;
         }


    }

    from->idSub=to->idSub;// now we are aligned with master
    from->subStart=to->subStart+from->subStart;
    from->subEnd+=from->subStart;
    from->qryEnd+=from->qryStart;

    return from->lenAlign; // how many elements in a new array
}

idx sBioseqAlignment::truncateAlignment(Al * hdr, idx maskBefore, idx maskAfter, idx issuborqry)
{
    idx * m=hdr->match();


    if ( issuborqry==0)  {
        if(maskAfter<0)maskAfter=hdr->subEnd+maskAfter;

        if( hdr->subEnd<=maskBefore ) return 0;
        if( hdr->subStart>maskAfter )return 0; // this is before the requested region
    }

    idx len=hdr->lenAlign, idst, isrc;
    idx newsubstart=-1, newqrystart=-1;

    for ( isrc=0, idst=0; isrc<len; isrc+=2 ) {
        if ( issuborqry==0)  {
            if( m[isrc]+hdr->subStart<maskBefore ) continue; // this is before the requested region
            if( m[isrc]+hdr->subStart>=maskAfter ) break; // this is before the requested region
        } else if ( issuborqry==1)  {
            if( m[isrc+1]+hdr->qryStart<maskBefore ) continue; // this is before the requested region
            if( m[isrc+1]+hdr->qryStart>=maskAfter ) break; // this is before the requested region
        }


        if(newsubstart==-1)
            newsubstart=hdr->subStart+m[isrc]; // we remember the new start position
        if(newqrystart==-1)
            newqrystart=hdr->qryStart+m[isrc+1];

        if(m[isrc]!=-1)
            hdr->subEnd=m[idst]=hdr->subStart+m[isrc]-newsubstart;
        if(m[idst+1]!=-1)
            hdr->qryEnd=m[idst+1]=hdr->qryStart+m[isrc+1]-newqrystart;
        idst+=2;

    }
    hdr->lenAlign=idst;
    hdr->subStart=newsubstart;
    hdr->subEnd+=newsubstart;
    hdr->qryStart=newqrystart;
    hdr->qryEnd+=newqrystart;

    return hdr->lenAlign; // how many elements in a new array
}





// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  SNP counting
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/



idx sBioseqAlignment::countSNPSingleSeq(idx * freq, idx substart, idx sublen, const char * qrybits, idx qrybuflen,const char * qua, Al * hdr, idx * m, idx maxRptIns)
{
    if( hdr->subEnd < substart || hdr->subStart+m[0] > substart+sublen) // if the range requestes is beyond mapping of this query
        return 0;

    idx i, is, iq,cntMapped=0;
    idx ddd=SNPtableColSize;

    // retrieve the query sequence
    idx lastSubOK=0, ch, flags=hdr->flags;
    char let=0;

    for( i=0 ; i<hdr->lenAlign; i+=2) {

        is=m[i];
        iq=m[i+1];

        if(qua) {
            if( (qua[i/8]&(((idx)1)<<(i%8))) ==0 ) // quality bit is not set
                continue; // ignore the bases with low Phred score
        }

        if(is<0){ ch=4; is=lastSubOK;}  // we remember inserts on the last mapped position
        else if(iq<0)ch=5; // deletions
        else {
            idx iqx=QIDX( hdr->qryStart+iq, qrybuflen );
            ch=let=_seqBits(qrybits, iqx , hdr->flags) ;  // mapped ok
        }

        if( is +hdr->subStart < substart)continue; // before the range requested
        else if( is+hdr->subStart >= sublen+substart)break; // after the range requested

        if(maxRptIns && ch==4 && hdr->qryStart+iq>maxRptIns) { // check if this is a valid insert
            idx irp, iqx;
            char qlet, prvlet=-1;
            for ( irp=0 ; irp<=maxRptIns; ++irp ){
                iqx=QIDX( hdr->qryStart+iq-irp, qrybuflen );
                qlet=_seqBits(qrybits, iqx , hdr->flags);
                if( irp>0 && prvlet!=qlet)break ;  // scan back untill the letters are the same
                prvlet=qlet;
            }
            if(irp>maxRptIns)
                continue; // this is a device error repeat insert
        }

        ++cntMapped;
        if( (hdr->flags&fAlignCircular) && is>=sublen)is-=sublen;

        idx line=ddd*(hdr->subStart+is-substart);

        if(hdr->flags&fAlignBackward) ch+=ddd/2;
        ++(freq)[line+ch];

        if(qua && freq){
            idx iQua=6; if (hdr->flags&fAlignBackward) iQua=13 ;
            (freq)[line+iQua]+=( (qua[i/8]&(((idx)1)<<(i%8))) ==0 ) ? 40 : 10 ; // qua[i];
        }

        lastSubOK=is;

    }

    return cntMapped;
}

/*
idx sBioseqAlignment::countSNP(sVec < idx > * freq, idx substart, idx sublen, sBioseq * qry, sBioseqAlignment * qryMap, idx id, idx maxRptIns)
{
    idx iQry;
    idx ddd=SNPtableColSize;
    freq->resizeM(ddd*sublen);freq->set(0); // memory for frequencies

    // walk on queries one by one
    for ( iQry=0; qryMap->next(); ++iQry ) {
        //idx * m=sSeqAlign::Alignment::match(hdr);
        Al * hdr=qryMap->hdr();
        idx * m=qryMap->match();

        if(hdr->idSub!=id) // hitting not this id
            continue;
        if( (hdr->subEnd)<substart || hdr->subStart+m[0]>substart+sublen) // if the range requestes is beyond mapping of this query
            continue;


        // retrieve the query sequence
        const char * qrybits=qry->seq(hdr->idQry);
        const char * qua=qry->qua(hdr->idQry);
        idx qrybuflen=qry->len(hdr->idQry);

        countSNPSingleSeq(freq->ptr(), substart, sublen, qrybits, qrybuflen,qua, hdr, m, maxRptIns);
    }
    return 0;
}

*/






// find this item in the list of HSPs
/*
idx ih, id;
for ( ih=0; ih < cH ; ++ ih ){
    if(hsp.defint==ht[ih].defint) break;
}
if(ih==cH){  // not found: need to add it
    ht[cH]=hsp;
    ++ht[cH].refcnt;
    ++cH;
}else
    ++ht[ih].refcnt;
if(cH>=maxH*3) { // wipe out some elements which have no potential of scoring higher hits and have low refcount now
    for ( id=0, ih=maxH ; ih < 2*maxH ; ++ih ){ //only the best scoring hits of the third ones are getting into first 1/3rd
        if(id>=maxH)id=0; // we restart filling first third from the second third
        for ( ; id<maxH; ++id ) {
            if(  ht[id].refcnt<=ht[ih].refcnt ) {
                ht[id]=ht[ih];
                ++id;
                break;
            }
        }
    }

    id=maxH;

    for ( ; ih < cH ; ++ih ){ // the second half just didn't cut it the third takes over even if has lower scores
        ht[id]=ht[ih];
        ++id;
    }
    cH=id;
}
*/    //for (idx ih=0; ih<maxH ; ++ ih){
//    hspTbl.add(ht[ih].defint,ht[ih]);
//}


                /*
                if( hitsOn [refid] ) { // this sequences already has a hit
                    Al * hdr=(Al *)allHits->ptr( hitsOn[refid]-1 ) ;
                    idx * m=hdr->match();
                    if(subpos >=(hdr->subStart+m[0]+1) && subpos<=hdr->subEnd+1){
                        samehit++;
//PERF_END();
                        continue;
                    }
                }*/



/*else if( (incrN[(idx)qryLet]-incrP[(idx)prvQryLet]) == (incrN[(idx)subLet]-incrP[(idx)prvSubLet]) ) { //char diffQry=incrN[(idx)qryLet]-incrP[(idx)prvQryLet]; char diffSub=incrN[(idx)subLet]-incrP[(idx)prvSubLet];
                            ++matches;gaps=0;
                        } */




/*


PERF_START("CROSS-LOOKUP");
                idx cntCrossHit=0;
                for ( idx ipos=0; ipos<sublen-secHashUnit; ipos+=secHashUnit ) {
                    idx sechash=0;
                    for ( idx ik=0; ik<secHashUnit; ++ik ) {
                        char seclet=_seqBits(sub, ipos+ik , 0 );
                        sechash <<= 2;
                        sechash|=seclet;
                    }
                    sechash&=crossHash.hdr.hashMask;
                    if( (crossHash.hb [sechash/8] & (((udx)1)<<(sechash%8))) ==0 )
                        continue;
                    ++cntCrossHit;
                }
                //::printf("CROSSSSSHIT  = %"_d_"\n",cntCrossHit);
PERF_END();
*/



/*
    sBioseqHash crossHash;  // crossHash
    crossHash.init(secHashUnit,4);
    idx crossReHashLen=Subs->len(0), lastRehashLen=0; // so we can position in char buffer 
    idx cntPassedCrossBoundary=0, crossBoundary=0;


    */

            /*if( iq%crossReHashLen==0 && lastRehashEnd<qrylen && iq-crossReHashLen!=lastRehashStart ) { 
                idx fl=sBioseqHash::eCompileBloom|sBioseqHash::eCompileReset|sBioseqHash::eCompileOnlyBloom| (idir ? sBioseqHash::eCompileReverse : 0 );
                lastRehashStart=iq-crossReHashLen; if(lastRehashStart<0)lastRehashStart=0;
                lastRehashEnd=lastRehashStart+3*crossReHashLen;
                if(lastRehashEnd>qrylen-crossReHashLen)lastRehashEnd=qrylen;
                //::printf("LLLLLLLLLLLLLLLLLAST CRRS POS %"_d_": %"_d_"-%"_d_" \n",iq, lastRehashStart, lastRehashEnd);
                crossHash.compile(idQry, qry, qrylen, fl, 0, lastRehashStart, lastRehashEnd-lastRehashStart );
            }
            */



