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
 #include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqalign.hpp>
#include <ssci/bio/bioseqhash.hpp>
#include <slib/std/string.hpp>
#include <ssci/math/rand/rand.hpp>

#include <ctype.h>

using namespace slib;

#define _costMatch(_v_sbits, _v_qbits) (((_v_sbits) != (_v_qbits)) ? ( prevCost<=costMismatch ?  costMismatchNext : costMismatch) : costMatch)
#define QIDX(_v_iq, _v_len)    ((flags&sBioseqAlignment::fAlignBackward) ? ((_v_len)-1-(_v_iq)) : (_v_iq))
#define SIDX(_v_is, _v_len)    ( (_v_is) )
//#define SIDX(_v_is, _v_len)    ( ( (!(flags&fAlignCircular)) || (_v_is)<(_v_len) ) ? (_v_is) : ( (_v_is) -(_v_len) ) )
#define FIXFQFS(_v_fq, _v_fs)   { \
    _v_fq=iq; \
    if(_v_fq<(scanQPos[refid]&0xFFFFFFFF) ) \
        _v_fq=(scanQPos[refid]&0xFFFFFFFF); \
    iQForQPosFiltering=iq; \
    if(_v_fs<((scanQPos[refid]>>((idx)32))&0xFFFFFFFF) ) \
        _v_fs=((scanQPos[refid]>>((idx)32))&0xFFFFFFFF); \
    if(scanQPos[refid]==0) { \
        scanQIdxes[cntScanQPos]=refid; \
        cntScanQPos++; \
    } \
    scanQPos[refid]= fq|(fs<<((idx)32)); \
    }




idx cntHashLookup=0,cntBloomLookup=0,cntAlignmentSW=0,cntExtension=0, cntTotLetters=0,  cntCrossPass=0, cntSuccessSW=0;
//idx maxHashBinFound=0;
//idx falseNeg=0,falsePos=0, trueNeg=0,truePos=0, noFos=0;

//sPerf gPerf;
//#define DEBUGGING_SMITHWATERMANN 1
idx sBioseqAlignment::zeroAlignment[2]={0,0};
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Major Alignment Procedures
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

idx sBioseqAlignment::alignSeq(sVec < idx > * allHits , sBioseq * Subs, const char * qry, idx qrylen, idx idSub, idx idQry, idx flagset, idx qrysim, idx *  , idx * ) // subFosList qryfos
{


    if(!computeDiagonalWidth)computeDiagonalWidth=bioHash.hdr.lenUnit;// /2;

    idx oldHashList[32];

    idx flags;

    // here we maintain what position is already scanned on a query for each subject

    SubScanQPos.resizeM(Subs->dim()*2); // make sure there is enough space
    idx * scanQPos=SubScanQPos;//+Subs->dim();
    idx * scanQIdxes=scanQPos+Subs->dim(); // we remember the refids and the number of scanQPos-es hit-to zero them later
    //idx * ssim=SimilarityBuf.ptr();

    idx * extensionGapBuff=ExtensionGapBuff.ptr();
    idx cntIn=0,fq,fs;
//    sBioseqHash::HLitem * hi2_0=0;
    //sBioseqHash::HLitem * hi2=0;
//    idx lstdim2;
    idx iQForQPosFiltering=0;

    ///#ifdef _DEBUG
    ///::printf("\n\n\n\n\n--- #################################################### \n ");
    ///#endif

PERF_START("ALIGNSEQ");

    for ( idx idir=0; idir < 2; ++idir)  {

        PERF_START("ALIGN-MEM");
        //SubScanQPos.set(0);
        for(idx isqp=0; isqp<cntScanQPos; ++isqp)
            scanQPos[scanQIdxes[isqp]]=0;
        cntScanQPos=0;
        PERF_END();

        idx mask = (idir==0) ? fAlignForward : fAlignBackward; // direction considering now
        if( !(flagset&mask) ) continue;  // this does not seem to be required to compute now
        flags=( flagset&(~(fAlignForward|fAlignBackward)) ) | mask;

        udx chash=0,chash2=0; // this is unsigned due to rotations sign digit overload
        idx iStp=0;
        udx crememberHash=0;

        ///#ifdef _DEBUG
        ///::printf("\n\n\n\n\n--- DIR \n ");
        ///#endif


        for( idx iq=0; iq<qrylen; ++iq ) {
            if(maxSeedSearchQueryPos && iq>maxSeedSearchQueryPos)break;

            ////PERF_START("PRE_FILT");

            //if(iq && callbackFunc && (iq%1000000)==0 ){
                //if( callbackFunc(callbackParam, cntIn, qrylen , 0)==1 )return cntIn; // (iq+idir*qrylen)/2
            //    PERF_PRINT();
            //}

//            ++cntTotLetters;

            // prepare the hash for this position
            chash=crememberHash;
            chash <<= 2;
            idx iqx=QIDX(iq,qrylen);
            char let=_seqBits(qry, iqx , flags);
            chash|=let;
            chash&=bioHash.hdr.hashMask;
            crememberHash = chash;
            #ifdef HASH_SINDEX
                chash=(chash<<8)| ((chash>>(2*(bioHash.hdr.lenUnit-4)))&0xFF);
                chash&=bioHash.hdr.hashMask;
            #else
            #ifdef _DEBUG
                char dbg[1024],dbg2[1024],dbg3[1024];
                sBioseq::uncompressATGC(dbg, (const char * ) &crememberHash, 0,bioHash.hdr.lenUnit, 0);
            #endif
            if(chash>=(udx)bioHash.hashTbl.dim())
                {////PERF_END();
                continue;
                }
            #endif

            bool useDoubleHash=false;
            if( flags&fAlignOptimizeDoubleHash ){
                chash2=oldHashList[iq%bioHash.hdr.lenUnit];
                oldHashList[iq%bioHash.hdr.lenUnit]=chash;

                #ifdef _DEBUG
                    sBioseq::uncompressATGC(dbg2, (const char * ) &chash2, 0,bioHash.hdr.lenUnit, 0);
                #endif

                if(iq<2*bioHash.hdr.lenUnit-1)
                    ;
                else
                    useDoubleHash=true;

            }

            // this is the position where the sequence has a hit of N-mer
            idx qrypos=iq-bioHash.hdr.lenUnit+1, refid=0;
            if(!(flagset&fAlignSearchRepeats)) {
                idx cntSub=Subs->dim();
                if(cntSub<128){
                    for( ; refid<cntSub ; ++refid ) {
                        if(qrypos < (scanQPos[refid]&0xFFFFFFFF) )
                            break;
                    }
                    if (refid<Subs->dim() )
                        continue;
                }
            }

            if( hashStp>1 && iStp<(hashStp-1)){
                ++iStp;
                {////PERF_END();
                continue;
                }
            }else iStp=0;
            if(iq<bioHash.hdr.lenUnit-1)
                {////PERF_END();
                continue;
                }

            // bloom filter if this hash even exists in table
            ++cntBloomLookup;
            if( bioHash.hb &&  ( (bioHash.hb[chash>>3] & (((udx)1)<<(chash&0x7))   ) ==0)  )
                {////PERF_END();
                continue;
                }


            if( useDoubleHash ){ // bloom
                if( bioHash.hb && (bioHash.hb [chash2>>3] & (((udx)1)<<(chash2&0x7))) ==0 ) // bloom
                    {////PERF_END();
                    continue;
                    }
            }


            ++cntHashLookup;
            #ifdef HASH_SINDEX
                idx iref=bioHash.hashInd.find(chash)-1; // bloom will never let 0 to be returned
                if( iref<0 )
                    {////PERF_END();
                    continue;
                    }

                idx ofs=bioHash.hashInd[iref].ofs;
            #else
                idx ofs=bioHash.hashTbl[chash];
            #endif
            if( !ofs)
                {////PERF_END();
                continue;
                }

            sBioseqHash::HLitem * hi0=bioHash.hList.ptr(ofs);
            sBioseqHash::HLitem * hi=hi0+1;
            idx lstdim=hi0->ref._pos;


/*
            if( useDoubleHash ){ // bloom
                #ifdef HASH_SINDEX
                    idx iref2=bioHash.hashInd.find(chash2)-1; // bloom will never let 0 to be returned
                    if( iref2<0 )continue;//{PERF_END();continue;}  // since bloom might not be used
                    idx ofs2=bioHash.hashInd[iref2].ofs;
                #else
                    idx ofs2=bioHash.hashTbl[chash2];
                #endif
                if( !ofs2 )
                    {PERF_END();continue;}
                    //continue;

                hi2_0=bioHash.hList.ptr(ofs2);
                hi2=hi0+1;
                lstdim2=hi2_0->ref._pos;
            }
            PERF_END();
*/
            // for all of the hits at this position
            for (  idx il=0; il<lstdim ; ++il ) {

                // get the match position
                sBioseqHash::RefPos & ref=hi->ref;
                hi=bioHash.hList.ptr(hi->next);

                idx refid=ref.id();

                if(idSub!=sNotIdx && refid!=idSub)
                    continue; // this reference is to a different sequen

                if( (flagset & fAlignKeepFirstMatch) && (flagset & fAlignReverseEngine) && SHitBitmask && (SHitBitmask[refid / 8] & (((idx) 1) << (refid % 8))) )
                    continue;


                idx subpos=ref.pos();


                /* idx *ss=ssim ? ssim+(refid*selfSimilairityBufferSize+iq*2+idir) : 0 ;
                if(ss && iq<selfSimilairityBufferSize ) {
                    // the previous sequence on this reference for this iq has not have a successful extension until the subpos=
                    idx prevExt=(*ss);
                    if(qrysim<prevExt)// up to the position of extension the previous and this sequence are identical - hence no extension is possible
                        continue;

                }*/

                if(flagset&fAlignSearchRepeats) {
                    //if((flagset&fAlignSearchTranspositions)) {
                    if(qrypos>(scanQPos[refid]&0xFFFFFFFF)) // if transpositions are searched: the right part of the query may still hit the left part of the subject
                        scanQPos[refid]&=0xFFFFFFFF; // shut of subject scan position
                //}
                }

                if(!(flagset&fAlignSearchRepeats)) {
                    if(iq>iQForQPosFiltering && qrypos < (scanQPos[refid]&0xFFFFFFFF) )
                        continue;
                }

                {
                    if(subpos<(((scanQPos[refid]>>((idx)32))&0xFFFFFFFF))+1)
                        continue; //  this position is a definite miss - we tried extending this before
                }

                ///#ifdef _DEBUG
                ///::printf(" --- considering qryID=%" DEC " iqry=%" DEC " idir=%" DEC " refid=%" DEC " subpos=%" DEC " il/lstdim=%" DEC "/%" DEC " \n", idQry, iq, idir, refid, subpos,il,lstdim);
                ///#endif
                const char * sub=Subs->seq(refid);


                if( useDoubleHash ) {
                    /*
                    idx i2;
                    for( hi2=hi2_0+1, i2=0; i2<lstdim2 ; ++i2){
                        sBioseqHash::RefPos & ref2=hi2->ref;
                        hi2=bioHash.hList.ptr(hi2->next);
                        if( ref2.id() == refid && ref2.pos()==subpos-(bioHash.hdr.lenUnit) )break;
                    }
                    if(i2==lstdim2) continue; /// didn't find the second piece of a hash
                    */
                    idx p=subpos-(bioHash.hdr.lenUnit);
                    if( p < 0 ) // this hit is too early , cannot be the second hash
                        continue;
                    udx  h2ash=_seqBytes(sub, p, bioHash.hdr.lenUnit );
                    #ifdef _DEBUG
                        sBioseq::uncompressATGC(dbg3, (const char * ) &h2ash, 0,bioHash.hdr.lenUnit, 0);
                    #endif

                    if(chash2!=h2ash)
                        continue;

                }


                idx sublen=Subs->len(refid);
                idx curofs=allHits->dim(); // remember the working position

                idx matchesAll=sIdxMax;
                idx qryposN=qrypos;
                if( maxMissQueryPercent >= 0 ){

                    PERF_START("EXTENDER");
                    // extend on the left and right
                    matchesAll=0;
                    idx inShift=0;
                    iqx=QIDX(qrypos,qrylen);
                    char prvSubLet=_seqBits(sub, subpos , 0);
                    char prvQryLet=_seqBits(qry, iqx , flags); // the last matching position letters

                    idx isub=0,iqry=0, isubfirstnonmatch=-1,iqryfirstnonmatch=-1;
                    idx isubleft=subpos,iqryleft=qrypos;
                    idx isubright=subpos+bioHash.hdr.lenUnit,iqryright=qrypos+bioHash.hdr.lenUnit;
                    ++cntExtension;
                    idx iGapMinus=extGapMidPos,iGapPlus=extGapMidPos;

                    for ( idx ext = -1; ext<2 ; ext+=2 ) { // extension directions left and right

                        if(ext<0){
                            isub=subpos-1;iqry=qrypos-1;
                            if( useDoubleHash ) {
                                isub-=(bioHash.hdr.lenUnit-1);
                                iqry-=(bioHash.hdr.lenUnit-1);
                                isubleft-=(bioHash.hdr.lenUnit);
                                iqryleft-=(bioHash.hdr.lenUnit);


                            }
                        } // the next position under consideration
                        else {
                            isub=subpos+bioHash.hdr.lenUnit;
                            //if( useDoubleHash ) {
                            //    isub+=bioHash.hdr.lenUnit;
                            //}
                            iqry=qrypos+bioHash.hdr.lenUnit;
                        }
                        idx diffs=0 ,gaps=0 ;//, matches= (ext==-1) ? bioHash.hdr.lenUnit : 0 ;//, isubshift=0, iqryshift=0 ;
                        idx matches=bioHash.hdr.lenUnit ;
                        if( useDoubleHash && ext<0)
                            matches+=bioHash.hdr.lenUnit ;

//                        char qryLet = 0;
//                        char subLet = 0;

                        while( gaps<=(maxExtensionGaps+1) && diffs*100 <= sMin((real)looseExtenderMismatchesPercent,maxMissQueryPercent)*((considerGoodSubalignments == 2)?qrylen:((considerGoodSubalignments == 1)?minMatchLen:(matches +diffs))) ) { // maxExtensionGaps+1 in gaps because we are comparing a differential

                            if(ext>0 && (isub>=sublen || iqry>=qrylen) ){isubright=isub;iqryright=iqry;break;}
                            if(ext<0 && (isub<0 || iqry<0) )break;
                            //if(ext<0 && (isub<0 || iqry<0) )break;

                            iqx=QIDX(iqry,qrylen);
                            char qryLet = _seqBits(qry, iqx , flags);
                            char subLet = _seqBits(sub, isub , 0);

                            if(qryLet!=subLet){
                                // QRY TO SUB filtering
                                if( iqryfirstnonmatch==-1 && ext>0 )iqryfirstnonmatch=iqry;
                                if( isubfirstnonmatch==-1 && ext>0 )isubfirstnonmatch=isub;

                                if(gaps==1){ // the previous letter is a no match ?
                                    idx gapPos=0;
                                    if( prvSubLet==qryLet) {
                                        if(!maxExtensionGaps)
                                            break;
                                        gapPos=-(iqry+(ext>0 ? 1 : 1 ) );
                                        isub-=ext; subLet=prvSubLet;
                                    }
                                    else if( prvQryLet==subLet ) {
                                        if(!maxExtensionGaps)
                                            break;
                                        gapPos=(isub+(ext>0 ? 1 : 1 )); // it can be zero as well that is why we keep one more and negatives mean on qry
                                        iqry-=ext;qryLet=prvQryLet;
                                    }

                                    if(gapPos){
                                        if(ext<0){
                                            if(iGapMinus>0){
                                                --iGapMinus;
                                                extensionGapBuff[iGapMinus]=gapPos;

                                            }
                                        }else {
                                            if(iGapPlus<extGapMidPos*2-1){
                                                extensionGapBuff[iGapPlus]=gapPos;
                                                ++iGapPlus;
                                            }
                                        }
                                    }
                                }

                                if( qryLet==subLet ){
                                    ++matches;
                                    gaps=0;
                                    if(inShift==0)++diffs;
                                    inShift=1;
                                    //++diffs;
                                }
                                else {
                                    ++diffs;++gaps;
                                    inShift=0;
                                }

                            }
                            else { // if(qryLet==subLet){
                                ++matches;
                                gaps=0;
                                inShift=0;
                                if(qryLet==subLet){
                                    if(ext<0){
                                        isubleft=isub;
                                        iqryleft=iqry;
                                    }
                                    else {
                                        isubright=isub;
                                        iqryright=iqry;
                                    }
                                }
                            }

                            prvSubLet=subLet;
                            prvQryLet=qryLet;

                            isub+=ext; iqry+=ext;
                        }
                        matchesAll+=matches;
                        if(ext!=-1)matchesAll-=bioHash.hdr.lenUnit ;
                        else --matchesAll;
                        /*if(ext<0) {
                            if(qryLet==subLet){
                                isubleft=isub;
                                iqryleft=iqry;
                            }
                        }*/
                        if( iqryfirstnonmatch==-1 && ext>0 )iqryfirstnonmatch=iqry;
                        if( isubfirstnonmatch==-1 && ext>0 )isubfirstnonmatch=isub;
                    }


                    if(isubleft<0)isubleft = 0 ;
                    if(iqryleft<0)iqryleft = 0;
                    iqry=iqryright;
                    isub=isubright;

                    subpos=isubleft  ;
                    qryposN=iqryleft ;

                    PERF_END();


                    if(selfSubjectPosJumpInNonPerfectAlignment)
                        fs = (isubfirstnonmatch==-1) ? (subpos+bioHash.hdr.lenUnit) : isubfirstnonmatch; // last considered query extension position on this subject
                    else fs=-1;
                    if(selfQueryPosJumpInNonPerfectAlignment)
                        fq=(iqryfirstnonmatch==-1) ? (qryposN+bioHash.hdr.lenUnit) : iqryfirstnonmatch; // last considered query extension position on this subject
                    else fq=-1;
                    if(fs!=-1 || fq!=-1)
                        FIXFQFS(fq,fs);

                    // QRY TO SUB filtering
                    idx compareLength=(flags&fAlignIdentityOnly) ? minMatchLen : minMatchLen/2;
                    if( matchesAll < compareLength){  // do not attempt to align smith watermann
                        if( matchesAll>=allowShorterEnds && ((idir==0 && isub>=sublen-3) || (idir==1 && subpos<3)) ) { // if the length is short because of the reference boundary ?? do not cut it
                        } else {
                            continue;
                        }
                    }

                    // remember the length we were able to extend for this sequence from the seed at position iq
                    /*
                    if(ss && ( ( (*ss)>>32 )&0xFFFFFFFF) ) {
                        *ss=(iqryright-ref.pos())<<32;
                        qrysim=qrylen;
                    }*/
                    ///#ifdef _DEBUG
                    ///::printf(" ----- extension ok  \n");
                    ///#endif

                    fs = (isubfirstnonmatch==-1) ? (subpos+bioHash.hdr.lenUnit) : isubfirstnonmatch; // last considered query extension position on this subject
                    fq= (iqryfirstnonmatch==-1) ? (qryposN+bioHash.hdr.lenUnit) : iqryfirstnonmatch; // last considered query extension position on this subject
                    //fs = (subpos+bioHash.hdr.lenUnit) ; // last considered query extension position on this subject
                    //fq= (qryposN+bioHash.hdr.lenUnit) ; // last considered query extension position on this subject
                    FIXFQFS(fq,fs);

//

                    if( matchesAll==qrylen || flags&fAlignIdentityOnly){ // we do not need to run smith watermann here
                    //if( flags&fAlignIdentityOnly){ // we do not need to run smith watermann here
                        //iGapPlus=iGapMinus;
                        if(matchesAll<minMatchLen*looseExtenderMinimumLengthPercent/100) {
                            continue;

                        }
                        idx gpos=0;
                        for( idx iG=iGapMinus; iG<iGapPlus; ++iG){
                            gpos = extensionGapBuff[iG];
                            if(gpos<0)
                                gpos = -gpos;
                            if( gpos < iqryleft ) {
                                iGapMinus = iG + 1 ;
                            }
                            else if( gpos > iqryright ) {
                                iGapPlus = iG;
                                break;
                            }
                        }

                        PERF_START("Identities");
                        idx dimal=3;
                        if(matchesAll!=qrylen)
                            dimal+=(iGapPlus-iGapMinus)*5;
                        allHits->addM(sizeof(Al)/sizeof(idx)+dimal); // 3 for alignment train
                        Al * hdr=(Al*)allHits->ptr(curofs);
                        sSet(hdr);

                        hdr->setFlags(flags|sBioseqAlignment::fAlignCompressed);
                        hdr->setDimAlign(dimal);
                        hdr->setSubStart(subpos);
                        hdr->setQryStart(qryposN);
                        //hdr->subEnd=isub;
                        //hdr->qryEnd=iqry;
                        // //hdr->setLenAlign(iqry-qryposN+1);
                        hdr->setLenAlign(iqry-qryposN);
                        hdr->setIdSub(refid);
                        hdr->setIdQry(idQry);
                        // // hdr->setScore((iqry-qryposN+1)*costMatch); //
                        hdr->setScore((iqry-qryposN)*costMatch); //


                        idx * m=(idx*)sShift(hdr,sizeof(Al) );
                        m[0]=0;m[1]=0;
                        m[2]=-matchesAll;//-(iqry-qryposN+1);

                        if(matchesAll!=qrylen) {
                            idx extent,ig=2;

                            for( idx iG=iGapMinus; iG<iGapPlus; ++iG){
                                if(extensionGapBuff[iG]<0)
                                    extent=-extensionGapBuff[iG]-iqryleft;
                                else
                                    extent=extensionGapBuff[iG]-isubleft;

                                m[ig++]=-extent;

                                isubleft+=extent;
                                iqryleft+=extent;

                                if(extensionGapBuff[iG]>0){
                                    m[ig++]=isubleft-subpos;
                                    m[ig++]=-1;
                                    ++isubleft;
                                }
                                else {
                                    m[ig++]=-1;
                                    m[ig++]=iqryleft-qryposN;
                                    ++iqryleft;
                                }

                                m[ig++]=isubleft-subpos;
                                m[ig++]=iqryleft-qryposN;
                            }
                            // // m[ig]=-(iqry-iqryleft+1);
                            m[ig]=-(iqry-iqryleft);
                        }


                        // QRY TO SUB filtering
                        fq=iqry;
                        fs=isub;
                        FIXFQFS(fq, fs);

                        ++cntIn;
                        if(flags&fAlignReverseEngine)
                            reverseAlignment(hdr, 0);

                        PERF_END();

                        if(QHitBitmask)
                            QHitBitmask[idQry/8]|=(((idx)1)<<(idQry%8));
                        if(SHitBitmask)
                            SHitBitmask[refid/8]|=(((idx)1)<<(refid%8));
                        if( (flagset&fAlignKeepFirstMatch) && (flagset&fAlignReverseEngine)==0 ){  // if we are interested only in the first hit
                            ////PERF_END();
                            return cntIn; // for direct engine ... once we find a hit - break it
                        }
                        if(maxHitsPerRead && cntIn>maxHitsPerRead){
                            ////PERF_END();
                            return cntIn;
                        }

                        continue;
                    }
                } // if (looseExtenderMismatchesPercent)


                PERF_START("ALIGNSMITHWATERMAN");

                idx lenBeforeNmerMatch=sMin ( qryposN, subpos) ; // the letters before the match cann not be on a negative side of either sequecne
                idx qryStart=qryposN-lenBeforeNmerMatch;
                idx subStart=subpos-lenBeforeNmerMatch;
                idx qryLenAlign= computeDiagonalWidth+sMin ( (sublen-subStart) , (qrylen-qryStart) ); // the length to be aligned cannot be behind the end of either sequence
                idx subLenAlign = qryLenAlign;
                //if( qryLenAlign>qrylen)qryLenAlign=qrylen;
                //if( subLenAlign>sublen)subLenAlign=sublen;
                if( qryStart+qryLenAlign>qrylen)qryLenAlign=qrylen-qryStart;
                if( subStart+subLenAlign>sublen)subLenAlign=sublen-subStart;

                //sVec <idx > * al=allHits->add();
                //idx  matchcounters[4]; sSet(matchcounters,0,sizeof(idx)*4);

                idx lastQryEnd=qryStart, lastSubEnd=subStart, okhit=false;
                if(matchesAll>=minMatchLen*looseExtenderMinimumLengthPercent/100) {

                    ++cntAlignmentSW;
                    okhit = alignSmithWaterman(allHits,sub, subStart, subLenAlign, qry, qryStart, qryLenAlign,  flags , sublen, qrylen, lenBeforeNmerMatch+computeDiagonalWidth, &lastQryEnd, &lastSubEnd );

                }
                Al * hdr=(Al*)allHits->ptr(curofs);


                // QRY TO SUB filtering
                if(okhit) {
                    fq=lastQryEnd-bioHash.hdr.lenUnit+1; // last considered query extension position on this subject
                    fs=lastSubEnd-bioHash.hdr.lenUnit+1;
                    FIXFQFS(fq, fs);
                //A }
                //A fq = lastSubEnd-bioHash.hdr.lenUnit+1; // last considered query extension position on this subject
                //A if(fq>subScanQPos[refid] && !(flagset&fAlignSearchRepeats)  )
                //A     subScanQPos[refid]=fq;
                } // A
                PERF_END();


                if(okhit) {
                    ++cntSuccessSW;
                    ///#ifdef _DEBUG
                    ///::printf(" ---- smit ok \n");
                    ///#endif
                    hdr->setIdSub ( (flags&fAlignReverseEngine) ? idQry : refid );
                    hdr->setIdQry ( (flags&fAlignReverseEngine) ? refid : idQry ) ;


                    //if(flags&fAlignReverseEngine) viewAlignment(&dst,hdr, hdr->match() ,qry, sub, qrylen , sublen , "-SM-", 0, 0,0);
                    //else
                    //sStr dst;
                    //viewAlignment(&dst,hdr, hdr->match() ,sub, qry, sublen , qrylen , "-SM-", 0, 0,0);
                    //::printf("%s\n",dst.ptr());

                    ++cntIn;
                    if(QHitBitmask)
                        QHitBitmask[idQry/8]|=(((idx)1)<<(idQry%8));
                    if(SHitBitmask)
                        SHitBitmask[refid/8]|=(((idx)1)<<(refid%8));

                    if( (flagset&fAlignKeepFirstMatch) && (flagset&fAlignReverseEngine)==0 )  // if we are interested only in the first hit
                        return cntIn;  // for direct engine (reads are queries) ... once we find a hit - break it
                    if(maxHitsPerRead && cntIn>maxHitsPerRead)
                        return cntIn;

                }else {
                    allHits->cut(curofs);
                }

            }

        }

    }
    PERF_END();

    return cntIn;
}


//#define DEBUGGING_SMITHWATERMANN

/*! Align using the Smith-Waterman algorithm. */

idx sBioseqAlignment::alignSmithWaterman( sVec < idx > * al, const char * sub, idx substart, idx sublen, const char * qry, idx qrystart, idx qrylen, idx flags , idx , idx qrybuflen, idx startFloatingDiagonal, idx * pLastQryEnd,  idx * pLastSubEnd ) // subbuflen
{
//vioPerf.start("alignSW1");

//    #ifndef sBIO_ATGC_SEQ_2BIT
//        idx isComp=( (flags&fAlignForwardComplement) && (flags&fAlignForward) ) || ( (flags&fAlignBackwardComplement) &&(flags&fAlignBackward) ) ? true : false;
//    #endif
PERF_START("SMITHWATERMANN-PREPARATION");
    if(!qrybuflen)qrybuflen=qrylen;


    idx reserve=1024*1024;
    idx maxSeq=qrylen*2+computeDiagonalWidth; /// the maximum number of sequence letters to be considered cannot be bigger than this.
    idx SWMatrixWidth=qrylen ;
    ///idx ofsMatrixS=0;
    if(compactSWMatrix && computeDiagonalWidth) {
        // // // 2015/Feb/14 SWMatrixWidth=computeDiagonalWidth*2;
        //SWMatrixWidth=computeDiagonalWidth;
        SWMatrixWidth=computeDiagonalWidth*2;
    }

    idx sz=(maxSeq+2)*(SWMatrixWidth+1);if(sz<reserve)sz=reserve;
    // // MatSW.resizeM(sz*2+sublen*2); // make sure there is enough space to hold the matrix
    // // short int * matP=(short int *)MatSW.ptr();
    // // short int * floatDiag=matP+sz;
    MatSW.resizeM(sz*sizeof(long)+sublen*sizeof(long)); // make sure there is enough space to hold the matrix
    long * matP=(long *)MatSW.ptr();
    long * floatDiag=matP+sz;
//memset(matP,0,sz*sizeof(short int));
    sz=(maxSeq+1)*SWMatrixWidth;if(sz<reserve)sz=reserve;
    MatBR.resizeM(sz); // this matrix doesn't have the zeroth elements , so it is one less
    char * matB=MatBR.ptr();
//memset(matB,0,sz*sizeof(char));

    // we keep a matrix of for smith waterman algorithm
    /// /// #define mat(_v_i, _v_j) matP[(SWMatrixWidth+1)*(_v_i)+(_v_j-ofsMatrixS)]
    /// /// #define bak(_v_i, _v_j) matB[(SWMatrixWidth)*(_v_i)+(_v_j-ofsMatrixS)]

    #define mat(_v_i, _v_j) matP[(SWMatrixWidth+1)*(_v_i)+(_v_j)]
    #define bak(_v_i, _v_j) matB[(SWMatrixWidth)*(_v_i)+(_v_j)]

    idx fv,cv=0;//,bestCasePotential;
    idx maxAll=0, maxAllLastLetter=0; // the maximal score
    idx maxS=0, maxQ=0, maxRowLastLetter=0, maxColLastLetter=0; // maxS,maxQ: the position of the maxAll

//    cellsComputed=0;
    idx qStart=0, qEnd=(flags&fAlignOptimizeDiagonal ) ? computeDiagonalWidth : qrylen; // definition of the diagonal width where we compute

    //DEBUG_OUT_SMITHWATERMANN_TABLE_HEADER

    idx is, iq ,iFloatingDiagonal;
    //idx nonCompFlags=(flags&(~(fAlignBackwardComplement|fAlignForwardComplement)));
    idx prevCost=0;

PERF_END();

//vioPerf.end();
//vioPerf.start("alignSW2");
#ifdef DEBUGGING_SMITHWATERMANN
::printf("   \n\n");
#endif

    idx qbqs=qrybuflen-qrystart-1;
    bool complementQ=( ( (flags&fAlignForwardComplement) && (flags&fAlignForward) ) ||
            ( (flags&fAlignBackwardComplement) &&(flags&fAlignBackward) ) ) ?
            true : false;

PERF_START("SMITHWATERMANN-ACTUAL-ALGORITHM");
    for(  is=0, iFloatingDiagonal=0; is<sublen; ++is, ++iFloatingDiagonal ) {
    //vioPerf.start("alignSW2 - 1 ");

        #ifdef old_stile
            idx isx=SIDX( (substart+is) , subbuflen);
            idx sBits=_seqBits(sub, isx, nonCompFlags ); // this is the current sequence letter (bits) on the subject
        #else
            idx isx=substart+is;
            idx sBits=(idx)((sub[(isx>>2)]>>((isx&3)<<1))&0x3) ; // this particular base introduces this two bits
        #endif // oldstyle

        //#else
        //    idx sBits=sub[isx];
        //#endif

        idx maxRow=0;
        idx maxRowPos=0;
#ifdef DEBUGGING_SMITHWATERMANN
::printf( "%" DEC " %c  :" , is , sBioseq::mapRevATGC[sBits]);
#endif
//vioPerf.end();
//vioPerf.start("alignSW2 - 2 ");

        for( iq=qStart; iq<qEnd; ++iq ) {



            #ifdef old_stile
                idx iqx=QIDX((qrystart+iq),qrybuflen);
                idx qBits=_seqBits(qry, iqx, flags)  ; // this is the current sequence letter (bits) on the query
            #else
                idx iqx=((flags&sBioseqAlignment::fAlignBackward) ? (qbqs-iq) : (qrystart+iq));
                idx qBits=(idx)((qry[(iqx>>2)]>>((iqx&3)<<1))&0x3) ; // this particular base introduces this two bits
                if( complementQ)
                    qBits=sBioseq::mapComplementATGC[qBits];
            #endif
            //#else
            //    idx qBits= ( isComp ) ?  sBioseq::mapComplementATGC[(idx)qry[iqx]] : qry[iqx];
            //#endif


            cv=mat(is+1,iq+1);

            // consider a match
            idx costMMatch=_costMatch( sBits , qBits   );
            fv=mat(is,iq)+costMMatch;
            prevCost=costMMatch;
            if( cv<=fv ) {cv=fv; bak(is,iq)=3; };// if(is && iq)back(is-1,iq-1)|=0x30; }
            // consider insertion
            fv=mat(is,iq+1)+ ( (is==0 || bak(is-1,iq)==3) ? costGapOpen : costGapNext);//_costMatch(-1, sBits);
            if( cv<fv ) {cv=fv; bak(is,iq)=1; }// if(is)back(is-1,iq)|=0x30; }
            // consider deletion
            fv=mat(is+1,iq)+( (iq==0 || bak(is,iq-1)==3) ? costGapOpen : costGapNext);//_costMatch(sBits,-1);
            if( cv<fv ) {cv=fv; bak(is,iq)=2;};// if(iq)back(is,iq-1)|=0x20; }

            // is still better to be aligned or start a new one ?
            if(cv>0){
                // // mat(is+1,iq+1)=(short int)cv;
                mat(is+1,iq+1)=(long)cv;

                if(cv>=maxRow){ // we remember the Rows maximum value and its position for local alignment
                    maxRow=cv; // it will be useful for exit criteria
                    maxRowPos=iq;
                }
            }
#ifdef DEBUGGING_SMITHWATERMANN
::printf("%c %4" DEC " | ",cv>=maxRow ? sBioseq::mapRevATGC[qBits] : sBioseq::mapRevATGC[qBits]+'a'-'A' ,cv);
#endif

        }

#ifdef DEBUGGING_SMITHWATERMANN
::printf("\n");
#endif
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
            if(startFloatingDiagonal!=0 && is>startFloatingDiagonal) {
                if(maxRowPos>iFloatingDiagonal)++iFloatingDiagonal;
                if(maxRowPos<iFloatingDiagonal)--iFloatingDiagonal;
            }
            //floatDiag[is]=(short int)iFloatingDiagonal;
            floatDiag[is]=(long)iFloatingDiagonal;

            //qStart=is-computeDiagonalWidth;
            qStart=iFloatingDiagonal-computeDiagonalWidth;
            if(qStart<0)qStart=0;
            //qEnd=is+computeDiagonalWidth;
            qEnd=iFloatingDiagonal+computeDiagonalWidth;
            if(qEnd>qrylen)qEnd=qrylen;
        }

        //DEBUG_OUT_SMITHWATERMANN_TABLE_ROW

        if(qStart>=qrylen-1)
            break;

    }
//vioPerf.end();
//vioPerf.start("alignSW3");
PERF_END();

PERF_START("SMITHWATERMANN-COINTAINERIZATION");
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
    hdr->setFlags(flags);

    if(flags&fAlignGlobal) {
        maxAll=maxAllLastLetter;
        maxS=maxRowLastLetter;
        maxQ=maxColLastLetter;    // qrylen-1
        hdr->setScore(maxAllLastLetter);//hdr->scoreGlobal;
    }else
        hdr->setScore(maxAll);//hdr->scoreLocal;


    hdr->setDimAlign(0); // to be counted a little later
    hdr->setSubStart(substart);
    hdr->setQryStart(qrystart);
    //hdr->subEnd=maxS+hdr->subStart; // the maximum subject window used for alignment : +1 is because maxS was an index
    //hdr->qryEnd=maxQ+hdr->qryStart; // qryLen // the maximum query window used for alignment
    if(pLastQryEnd)*pLastQryEnd=maxQ+qrystart+1;
    if(pLastSubEnd)*pLastSubEnd=maxS+substart+1;

    if(flags&fAlignReverseEngine){
        //idx t=hdr->subStart;hdr->subStart=hdr->qryStart;hdr->qryStart=t;
        hdr->starts=sSwapHiLo(hdr->starts);
//        t=hdr->subEnd;hdr->subEnd=hdr->qryEnd;hdr->qryEnd=t;
    }

    //
    // first we determine the length
    //
    idx  matchcounters[4]; sSet(matchcounters,0,sizeof(idx)*4);
    idx dlen =1+( (minMatchLen<=0?1:minMatchLen))/(sizeof(idx)*8);
    bool oklocal = false, match = false;
    localBitMatch.resize(dlen);sSet(localBitMatch.ptr(),0,sizeof(idx)*dlen);
//    idx insideIsGood=0;
PERF_END();

PERF_START("SMITHWATERMANN-BACKTRACKING");
    for(dlen=0, is=maxS, iq=maxQ, cv=maxAll;  ; cv=mat(is+1,iq+1) , dlen+=2) {  // start walking back on backtrace pointers
        if(flags&fAlignGlobal){ if(iq<0 || is<0) break;} // break when we reach the first letter in global alignment
        else if(cv<=0) break;// break if we reach the 0 score in local alignment
        //else if(cv<=0 || is<0 || iq<0) break;// break if we reach the 0 score in local alignment
        match = false;
        char bw=bak(is,iq);
        if( (!(flags&fAlignMaxExtendTail)) && cv<costMatch ){ break;} // if we want compact alignment ... no reason to walk left without gaining anything

        if(bw==0x02)++matchcounters[2];
        else if(bw==0x01)++matchcounters[3];
        else if(cv<mat(is,iq))++matchcounters[1];
        else { ++matchcounters[0];match=true;}

        if( considerGoodSubalignments == 1 && !oklocal){
            if( matchcounters[0]*100 < minMatchLen*(100-maxMissQueryPercent) ){// because maximum score may have more than allowed number of missmatches : we want to know that inside we have a region satisfying our missmatch criteria
                idx bitptr = (( dlen / 2)%minMatchLen) / (sizeof(idx) * 8), bitpos = ((( dlen / 2)%minMatchLen) % (sizeof(idx) * 8));
                if( localBitMatch[bitptr] & ((idx)1 << bitpos ) ) {
                    --matchcounters[0];
                    if(!match)
                        localBitMatch[bitptr] &= ~((idx)1 << bitpos );
                }
                localBitMatch[bitptr] |= (idx)match << bitpos;
            }
            else oklocal = true;
        }
        if(!bw) break;
        if(bw & 0x1)--is; // backstep
        if(bw & 0x2)--iq;
    }
PERF_END();

PERF_START("SMITHWATERMANN-FETCHING");

    bool okhit=true;
    if( dlen<2*minMatchLen) {
        if( !(flags&fAlignBackward) && maxS+substart+1>=sublen-3 && dlen>=2*allowShorterEnds )
            okhit=true;
        else if( (flags&fAlignBackward) && substart<3 && dlen>=2*allowShorterEnds )
            okhit=true; // if the length is short because of the reference boundary ?? do not cut it
        else okhit=false;
    }
    else if(scoreFilter && hdr->score()<scoreFilter ) okhit=false;

     else {
        if(considerGoodSubalignments == 1 ) {
            if( !oklocal ) okhit=false;
        } else if(considerGoodSubalignments == 2){
            if( matchcounters[0]*200 < 2*qrylen*(100-maxMissQueryPercent)) okhit=false;
        } else {
            if( matchcounters[0]*200 < dlen*(100-maxMissQueryPercent)  ) okhit=false;
        }
    }
    if(!okhit) {
        //al->cut(curofs);
    } else
    {

        idx * m=al->addM(dlen);//+(sizeof(Alignment::Hdr)/sizeof(idx)) ); // this is the real number of elements needed
        hdr=(Al*)al->ptr(curofs); // because it may have been reallocated

        bak(0,0)=3;
        hdr->setDimAlign(dlen);

        idx qq=0,ss=1;
        if(flags&fAlignReverseEngine) {
            qq=1;ss=0;
        }

        for(dlen-=2, is=maxS, iq=maxQ, cv=maxAll;  dlen>=0 ; cv=mat(is+1,iq+1) , dlen-=2) {  // start walking back on backtrace pointers

            char bw=bak(is,iq);

            m[dlen+qq]= bw==0x02 ? -1 : is; // here we remember the matching pairs: the fact that subject position goes after
            m[dlen+ss]= bw==0x01 ? -1 : iq; // query position is reverted in the next loop during reversal of the array to forward order

            if(bw & 0x1)--is; // backstep
            if(bw & 0x2)--iq;
        }
        hdr->setLenAlign(hdr->dimAlign()/2);

        hdr->setDimAlign(compressAlignment(hdr, m, m ));
        al->cut(curofs+sizeof(sBioseqAlignment::Al)/sizeof(idx)+hdr->dimAlign());

//        hdr->missMatches= ((matchcounters[1]&0xFFFF)) | ((matchcounters[2]&0xFFFF)<<16) | ((matchcounters[3]&0xFFFF)<<32);

    }

PERF_END();

//vioPerf.end();
//vioPerf.start("alignSW4");

PERF_START("SMITHWATERMANN-RECOVERY");

    //set back things to zero
    qStart=0;
    qEnd=(flags&fAlignOptimizeDiagonal ) ? computeDiagonalWidth : qrylen; // definition of the diagonal width where we compute
    for(  is=0; is<sublen; ++is ) {
        for( iq=qStart; iq<qEnd; ++iq ) {
            mat(is+1,iq+1)=0;
            bak(is,iq)=0;

            if(flags&fAlignOptimizeDiagonal) {
                //qStart=is-computeDiagonalWidth;
                qStart=floatDiag[is]-computeDiagonalWidth;
                if(qStart<0)qStart=0;
                //qEnd=is+computeDiagonalWidth;
                qEnd=floatDiag[is]+computeDiagonalWidth;
                if(qEnd>qrylen)qEnd=qrylen;

            }
        }
    }
PERF_END();

//vioPerf.end();


    return okhit ? 1 : 0 ;
    #undef mat
}

idx sBioseqAlignment::remapAlignment(Al * from, Al * to, idx * mfrom0, idx * mto0 ,idx maxlen,idx readonly,sVec<idx> * lookup )
{
    //idx * mfrom0=from->match();
    //idx * mto0=to->match();
    if(!mfrom0)mfrom0=from->match();
    if(!mto0)mto0=to->match();
    if(!maxlen)maxlen=sIdxMax;
    idx cntfrom=2*from->lenAlign(), cntto=2*to->lenAlign();
    idx ofsfrom=from->subStart(), ofsto=to->qryStart();

    idx * mfrom, * mto=mto0+1, *mfromEnd=mfrom0+cntfrom, * mtoEnd=mto0+cntto,cntr=0;
//    for( mfrom=mfrom0, mto=mto0+1; mfrom < mfromEnd && mto<mtoEnd && cntr<maxlen; mfrom+=2 , ++cntr) { // mto0+1 because in master alignment to amplicon , amplicon is the query
    for( mfrom=mfrom0, mto=mto0+1; mfrom < mfromEnd && mto<mtoEnd && cntr<maxlen; mfrom+=2 , ++cntr) {
        // find the position  where we have and overlap of the two mappings on amplicon
        while ( mfrom<mfromEnd &&  (*mfrom)+ofsfrom < (*mto)+ofsto)   // TODO : sortsearch instead of scanning
          mfrom+=2;
        if(mfrom>=mfromEnd)break;


        if(lookup){   //if((*mfrom)+ofsfrom>(to->dimAlign()-cntto))break;
            mto=mto0+( (*(lookup->ptr( (*mfrom)+ofsfrom )) )*2)+1;
            if(mto>=mtoEnd)break;
        }
        while ( mto<mtoEnd && (*mfrom)+ofsfrom > (*mto)+ofsto)
          mto+=2;
        if(mto>=mtoEnd)break;

        if ( (*mfrom)+ofsfrom!=(*mto)+ofsto) // this position is not aligned (?in the multiple alignment?)
            *(mfrom+1)=-1; // query points to -1
        else {
            *mfrom=*(mto-1);
            if(*mfrom!=-1) {
                *mfrom+=-ofsfrom;
//                from->subEnd=*mfrom;
                if(!readonly)from->setLenAlign((mfrom-mfrom0)/2);
            }
        }

        if( *(mfrom+1)!=-1){
//             from->qryEnd=*(mfrom+1);
            if(!readonly)from->setLenAlign((mfrom-mfrom0+2)/2);
         }

    }

    if(!readonly)from->setIdSub(to->idSub());// now we are aligned with master
    if(!readonly)from->setSubStart(to->subStart()+from->subStart());
//    from->subEnd+=from->subStart;
//  from->qryEnd+=from->qryStart;

    return from->lenAlign(); // how many elements in a new array
}

//static
idx sBioseqAlignment::remapSubjectPosition(Al * hdr, idx * to0, idx pos, idx valid /*= 0*/ )
{
    if(!hdr || !to0) {
        return 0;
    }

    idx cntTo=2*hdr->lenAlign(), ofS=hdr->subStart(), ofQ = hdr->qryStart();
    idx * toEnd = to0 + cntTo, l_valid = -1, * to=to0;

    idx m_pos = (*(to+1) ) + ofQ;

    while ( to<toEnd &&  (*to)+ofS < pos) {
        m_pos = (*(to+1) ) + ofQ;
        if( m_pos >= 0 ) {
            l_valid = m_pos;
        }
        to+=2;
    }
    if( to < toEnd )
        m_pos = (*(to+1) ) + ofQ;
    else
        m_pos = -1;

    if( (*to)+ofS != pos )
        m_pos =  -1; // query points to -1
    else {
        if( m_pos < 0 && valid > 0) {
            if( valid == 1 ) {
                m_pos = l_valid;
            }
            else {
                while ( to<toEnd && m_pos < 0) {
                    m_pos = (*(to+1) ) + ofQ;
                    to+=2;
                }
            }
        }
    }
    return m_pos;
}

//static
idx sBioseqAlignment::remapQueryPosition(Al * hdr, idx * to0, idx pos, idx valid /*= 0*/ )
{
    if(!hdr || !to0) {
        return 0;
    }

    idx cntTo=2*hdr->lenAlign(), ofS=hdr->subStart(), ofQ = hdr->qryStart();
    idx * toEnd = to0 + cntTo, l_valid = -1, * to=to0;

    idx m_pos = (*(to) ) + ofS;

    while ( to<toEnd &&  (*(to+1))+ofQ < pos) {
        m_pos = (*(to) ) + ofS;
        if( m_pos >= 0 ) {
            l_valid = m_pos;
        }
        to+=2;
    }
    if( to < toEnd )
        m_pos = (*(to) ) + ofS;
    else
        m_pos = -1;

    if( (*(to+1))+ofQ != pos )
        m_pos =  -1; // query points to -1
    else {
        if( m_pos < 0 && valid > 0) {
            if( valid == 1 ) {
                m_pos = l_valid;
            }
            else {
                while ( to<toEnd && m_pos < 0) {
                    m_pos = (*(to) ) + ofS;
                    to+=2;
                }
            }
        }
    }
    return m_pos;
}
/*! Truncate the alignment */

idx sBioseqAlignment::truncateAlignment(Al * hdr, idx maskBefore, idx maskAfter, idx issuborqry)
{
    idx * m=hdr->match();

    idx subend=hdr->subStart()+m[0]+1+hdr->lenAlign();
    if ( issuborqry==0)  {
        //if(maskAfter<0)maskAfter=hdr->subEnd()+maskAfter;
        if(maskAfter<0)maskAfter=subend+maskAfter;

        if( subend<=maskBefore ) return 0;
        if( hdr->subStart()>maskAfter )return 0; // this is before the requested region
    }

    idx len=hdr->lenAlign()*2, idst, isrc;
    idx newsubstart=-1, newqrystart=-1;

    for ( isrc=0, idst=0; isrc<len; isrc+=2 ) {
        if ( issuborqry==0)  {
            if( m[isrc]+hdr->subStart()<maskBefore ) continue; // this is before the requested region
            if( m[isrc]+hdr->subStart()>=maskAfter ) break; // this is before the requested region
        } else if ( issuborqry==1)  {
            if( m[isrc+1]+hdr->qryStart()<maskBefore ) continue; // this is before the requested region
            if( m[isrc+1]+hdr->qryStart()>=maskAfter ) break; // this is before the requested region
        }


        if(newsubstart==-1)
            newsubstart=hdr->subStart()+m[isrc]; // we remember the new start position
        if(newqrystart==-1)
            newqrystart=hdr->qryStart()+m[isrc+1];

/*
        if(m[isrc]!=-1)
            hdr->subEnd=m[idst]=hdr->subStart+m[isrc]-newsubstart;
        if(m[idst+1]!=-1)
            hdr->qryEnd=m[idst+1]=hdr->qryStart+m[isrc+1]-newqrystart;
*/
        idst+=2;

    }
    hdr->setLenAlign(idst/2);
    hdr->setSubStart(newsubstart);
//    hdr->subEnd+=newsubstart;
    hdr->setQryStart(newqrystart);
//    hdr->qryEnd+=newqrystart;

    return hdr->lenAlign(); // how many elements in a new array
}

idx sBioseqAlignment::getLUTMultipleAlignment(sVec < Al * > * alSub,sVec< sVec<idx> > & lutAlSub){
    if(!alSub)return 0;
    if(lutAlSub.dim())lutAlSub.cut(0);
    lutAlSub.add(alSub->dim());
    sVec < idx > uncompressMM;
    idx i,iA,it,ip,qS;
    for(iA=0; iA<alSub->dim();++iA){
        Al * hdr=*(alSub->ptr(iA));
        idx * m =hdr->match();qS=hdr->qryStart();
        if(hdr->flags()&sBioseqAlignment::fAlignCompressed){ // deal with compressed alignments
            uncompressMM.resize(2*hdr->lenAlign());
            sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
            m=uncompressMM.ptr();
        }
        lutAlSub.ptr(iA)->add(hdr->lenAlign()+qS);
        lutAlSub.ptr(iA)->set(-1);
        it=0;ip=0;
        for( i=0 ; i<2*hdr->lenAlign(); i+=2) {
            if( m[i + 1] >= 0 ) {
                ip = qS + m[i + 1];
                *(lutAlSub.ptr(iA)->ptr(ip)) = it;
            }
            ++it;
        }
        lutAlSub.ptr(iA)->cut(ip+1);
    }
    return iA;
}


idx sBioseqAlignment::prepareMultipleAlignmentSrc(sStr * tempStr, const char * src, idx len,  bool withIdlines)
{
    if( withIdlines ) {
        sStr tempStrWOids;


        sString::cleanMarkup(&tempStrWOids, src, len, "\r\n>" _ "\n>" _ "\r>" _ ">" __, "\r\n" _ "\n" _ "\r" __, "$", 0, false, false, false);
        idx start = (*src == '>') ? 1 : 0;
        sStr tempStrWOeols;
        if( !tempStrWOids ) return 0;
        sString::searchAndReplaceSymbols(&tempStrWOeols, tempStrWOids.ptr(start), tempStrWOids.length() - start, "\r\n" _ "\n" _ "\r" __, "", 0, true, true, false, false);
        if( !tempStrWOeols ) return 0;
        sString::searchAndReplaceSymbols(tempStr, tempStrWOeols.ptr(), tempStrWOeols.length(), "$", "\n", 0, true, true, false, false);
    } else {
        tempStr->printf("%s", src);
    }
    return tempStr->length();
}

idx sBioseqAlignment::readMultipleAlignment(sVec<idx> * alSub, const char * src, idx len, idx flags, idx * alLen, bool withIdlines, sBioseq * qry)
{
    if( !src || !len )
        return 0;
    if( !len )
        len = sLen(src);

    sStr DAA;
    sStr tempStr;
    withIdlines = (*src == '>') ? true : withIdlines;
    idx prep = sBioseqAlignment::prepareMultipleAlignmentSrc(&tempStr, src, len, withIdlines);
    if( !(prep > 0) )
        return 0;
    sString::searchAndReplaceSymbols(&DAA, tempStr.ptr(), tempStr.length(), "\r\n", 0, 0, true, true, false, false);
    const char * p0 = DAA.ptr();
    idx il, ib, maxlen = 0;
    for(ib = 0; p0; ++ib, p0 = sString::next00(p0)) {
        const char * p1 = p0;
        if( !(flags & eAlRelativeToMultiple) )
            p1 = sString::next00(p1); // in multiple mode we keep alignments of all to the block
        for( il = ib + ((flags & eAlRelativeToMultiple) ? 0 : 1); p1; ++il, p1 = sString::next00(p1)) {
            idx curOfs = alSub->dim();
            sBioseqAlignment::Al * hdr = (sBioseqAlignment::Al *) alSub->add(sizeof(sBioseqAlignment::Al) / sizeof(idx));

            hdr->setIdSub((flags & eAlRelativeToMultiple) ? -1 : ib);
            hdr->setIdQry(qry?qry->long2short(il):il);
            hdr->setScore(0);
            hdr->setSubStart(0);
            hdr->setQryStart(0);
            hdr->setFlags(0);

            idx i0 = 0, i1 = 0, it = 0;
            for(const char * f0 = p0, *f1 = p1; *f0 && *f1; ++f0, ++f1) {

                if( flags & eAlRelativeToMultiple ) {
                    alSub->vadd(1, i0++);
                } else {
                    if( *f0 == '-' )
                        alSub->vadd(1, sNotIdx);
                    else
                        alSub->vadd(1, i0++);
                }

                if( *f1 == '-' )
                    alSub->vadd(1, sNotIdx);
                else
                    alSub->vadd(1, i1++);
                ++it;
            }
            hdr = (sBioseqAlignment::Al *) alSub->ptr(curOfs);
            hdr->setLenAlign(it);
            hdr->setDimAlign(2 * it);
            if( maxlen < it )
                maxlen = it;
        }
        if( flags & eAlRelativeToMultiple )
            ib = il;
            break;
    }
    if( alLen )
        *alLen = maxlen;
    return ib;
}

idx sBioseqAlignment::selectBestAlignments(sBioseqAlignment::Al * alignmentMap,idx alignmentMapSize, idx qryOrSub, idx flags)
{
    // find start and end of the alignment map
    sBioseqAlignment::Al  * hdr=(sBioseqAlignment::Al  *)(alignmentMap);
    sBioseqAlignment::Al  * hdre=hdr ? sShift(hdr,alignmentMapSize*sizeof(idx)) : hdr ;

    sVec < idx > bestHit(sMex::fSetZero);
    sVec < idx > hitCnt(sMex::fSetZero);

    // accumulate best hits in the bestHit array and meanwhile computes statistics for All mode
    idx ofsPrv,idQry;
    sBioseqAlignment::Al * hdrFirst = 0;

    for( ; hdr<hdre ; hdr=sShift(hdr,hdr->sizeofFlat())  ) { // get the list of best alignments
        idQry = qryOrSub==0 ? hdr->idQry() : hdr->idSub();

        // make sure we have enough space
        hitCnt.resizeM(idQry+1);
        bestHit.resizeM(idQry+1);

        // here we remember the best hit for each query
        if( ++hitCnt[idQry] > 1 && (flags&sBioseqAlignment::fAlignKeepUniqueMatch) ) {
            bestHit[idQry] = 0;
            continue;
        }

        ofsPrv = bestHit[idQry];

        if( ofsPrv ){
            sBioseqAlignment::Al * hdrPrv = (sBioseqAlignment::Al *)((idx*)alignmentMap+(ofsPrv-1));
            if( hdrPrv->score() > hdr->score() ) // the previous alignment is better than the current one
                continue;
            else if( hdrPrv->score() == hdr->score() ) {
                if(flags&sBioseqAlignment::fAlignKeepAllBestMatches){
                    hdrPrv->setFlagsOn(sBioseqAlignment::fSelectedAlignment);
                }else if(flags&sBioseqAlignment::fAlignKeepRandomBestMatch){
                    //if((rand())%2) // generate a random number and decide if we switch or not
                    //if( (sRand::rand_xorshf96()%RAND_MAX)*hitCnt[idQry]<RAND_MAX ) // generate a random number and decide if we switch or not
                    if( ((sRand::rand_xorshf96()&0x7FFFFFFF)-1)*hitCnt[idQry]<RAND_MAX ) // generate a random number and decide if we switch or not
                        continue;
                }
                else
                    continue;
            }
            else { // the new alignment has better score ...
                if(flags&sBioseqAlignment::fAlignKeepAllBestMatches){ // remove previous selected flag
                    while(hdrFirst && hdrFirst<hdr){
                        hdrFirst->setFlagsOff(sBioseqAlignment::fSelectedAlignment);
                        hdrFirst=sShift(hdrFirst,hdrFirst->sizeofFlat());
                    }
                    hdrFirst = hdr;
                }
                if(flags&sBioseqAlignment::fAlignKeepRandomBestMatch) {
                    hitCnt[idQry] = 1;
                }
            }
        }
        else {
            hdrFirst = hdr;
        }
        bestHit[idQry]=(((idx*)hdr)-((idx *)alignmentMap)) + 1 ; // + 1 signifies the fact that this is a hit (for zero-th element)
    }

    idx iCnt=0;

    for ( idx iFnd = 0 ; iFnd < bestHit.dim() ;  ++iFnd ){ // print the best alignments and failed alignments
        idx ofs=bestHit[iFnd];if(!ofs )continue;
        --ofs;  /// remember +1 above, when accumulating the best hits  ?

        sBioseqAlignment::Al * hdr = (sBioseqAlignment::Al *)((idx*)alignmentMap+(ofs));
        hdr->setFlagsOn(sBioseqAlignment::fSelectedAlignment);
        ++iCnt;
    }
    return iCnt;
}

idx sBioseqAlignment::filterChosenAlignments(sVec< idx> * alignmentMap,idx qStart, idx flagSet, sStr * dst )
{

    bool doSelect = sBioseqAlignment::doSelectMatches(flagSet);
    idx cnt = 0;
    if(doSelect)
        sBioseqAlignment::selectBestAlignments((sBioseqAlignment::Al *)alignmentMap->ptr(),alignmentMap->dim(), 0 ,flagSet );
    sBioseqAlignment::Al * hdr0=(sBioseqAlignment::Al *)(alignmentMap->ptr(0)), * hdre=sShift(hdr0,alignmentMap->dim()*sizeof(idx)), * hdr=hdr0;
    for( hdr=hdr0; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()), ++cnt ) {
        hdr->setIdQry( hdr->idQry()+qStart);
    }
    ::printf("%" DEC "\n",cnt);
    idx newcnt = 0;
    if(dst->ok()){
        if( doSelect ) {
            for( hdr=hdr0; hdr<hdre; hdr=sShift(hdr,hdr->sizeofFlat()) ) {
                if( !((hdr->flags())&(sBioseqAlignment::fSelectedAlignment) ) )
                    continue;
                dst->add((const char *) hdr,hdr->sizeofFlat());
                ++newcnt;
            }
        }
        else {
            dst->add((const char *) hdr0,sizeof(idx)*alignmentMap->dim());
            newcnt = cnt;
        }

    }
    return newcnt;
}


idx sBioseqAlignment::compressAlignment(Al * hdr, idx * m, idx * mdst )
{
    idx i,idst, iprv ;
    idx counter=0;

    idx dimAlign=hdr->dimAlign();
    for( i=2,iprv=idst=0 ; i<dimAlign; i+=2) {

        if(i==dimAlign-2)++counter;
        if( m[i]!=m[i-2]+1 || m[i+1]!=m[i-1]+1 || i==dimAlign-2){

            mdst[idst++]=m[iprv];
            mdst[idst++]=m[iprv+1];
            if(counter>0){
                mdst[idst++]=-counter-1;
            }
            iprv=i;
            counter=0;
        }else {
            ++counter;
        }

    }
    hdr->setFlagsOn(fAlignCompressed);

    return idst;
}

idx sBioseqAlignment::uncompressAlignment(Al * hdr, idx * m, idx * mdst, bool reverse )
{
    idx i, idst;
    idx rs=reverse ? 1 : 0 ;
    idx rq=reverse ? 0 : 1 ;

    for( i=0,idst=0 ; i<hdr->dimAlign();) {
        if( m[i]<-1) {
            idx counter=-m[i]-1;
            for(idx k=0; k<counter; ++k ){
                mdst[idst+rs]=mdst[idst-2]+1;
                mdst[idst+rq]=mdst[idst-1]+1;
                idst+=2;
            }
            ++i;
            continue;
        }
        mdst[idst+rs]=m[i];
        mdst[idst+rq]=m[i+1];
        idst+=2;
        i+=2;
    }
    return idst;
}

idx sBioseqAlignment::reverseAlignment(Al * hdr, idx * m)
{
    idx i, t;

    hdr->ids=sSwapHiLo(hdr->ids);
    hdr->starts=sSwapHiLo(hdr->starts);
//          t=hdr->idQry;hdr->idQry=hdr->idSub;hdr->idSub=t;
//          t=hdr->subStart;hdr->subStart=hdr->qryStart;hdr->qryStart=t;
//          t=hdr->subEnd;hdr->subEnd=hdr->qryEnd;hdr->qryEnd=t;

    if(!m)return 0;
    for( i=0 ; i<hdr->dimAlign();) {
        if( m[i]<-1)
            ++i;
        else {
            t=m[i];
            m[i]=m[i+1];
            m[i+1]=t;
            i+=2;
        }
    }
    return i;
}

bool sBioseqAlignment::align2bioseqDefaultParameters (idx *flags)
{
    idx seed = 11;
    MatSW.addM(4*1024*1024);// reserve few megs at the beginning
    MatBR.addM(4*1024*1024);

    costMatch=5;
    costMismatch=-4;
    costMismatchNext=-6;
    costGapOpen=-12;
    costGapNext=-4;
    computeDiagonalWidth=0;
    maxMissQueryPercent=15;
    minMatchLen=11;

    scoreFilter=0;
    allowShorterEnds=minMatchLen;
    maxExtensionGaps=0;
    hashStp=1;
    bioHash.hashStp=1;
    looseExtenderMismatchesPercent=25;
    looseExtenderMinimumLengthPercent=66;

    maxHitsPerRead=20;
    compactSWMatrix=1;
    maxSeedSearchQueryPos=512;
    selfSubjectPosJumpInNonPerfectAlignment=0;
    selfQueryPosJumpInNonPerfectAlignment=1;

    bioHash.init(seed,4ll);
    idx isBloom = (seed <= 16) ? sBioseqHash::eCompileBloom  : 0 ; // ( ((udx)1<<(2*seed))/8/1024/1024/1024 <= 2 )  ? sBioseqHash::eCompileBloom  : 0 ;// the size neede for bloom in GB
    if(!isBloom)bioHash.collisionReducer=4;

    {
        idx flagSet=sBioseqAlignment::fAlignForward;
        flagSet|=sBioseqAlignment::fAlignGlobal;//else flagSet|=sBioseqAlignment::fAlignLocal;
        flagSet|=sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement;
        flagSet|=sBioseqAlignment::fAlignOptimizeDiagonal;
        flagSet|=sBioseqAlignment::fAlignIdentityOnly;
        flagSet|=sBioseqAlignment::fAlignKeepBestFirstMatch;
        *flags = flagSet;
    }
    return true;
}
