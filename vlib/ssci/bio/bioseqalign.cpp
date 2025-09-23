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

#define _costMatch(_v_sbits, _v_qbits) (((_v_sbits) != (_v_qbits)) ? ( prevCost<=costMismatch ?  costMismatchNext : costMismatch) : (prevCost!=costMatch?(prevCost==costMatchFirst?costMatchSecond:(prevCost==costMatchSecond?costMatch:costMatchFirst)):costMatch))
#define _profile_costMatch(_v_sbits, _v_qbits) (((_v_sbits) != (_v_qbits)) ?costMismatch : costMatch)
#define QIDX(_v_iq, _v_len)    ((flags&sBioseqAlignment::fAlignBackward) ? ((_v_len)-1-(_v_iq)) : (_v_iq))
#define SIDX(_v_is, _v_len)    ( (_v_is) )
#define FIXFQFS(_v_fqh, _v_fsh) {\
    if(docontinueOLD) {\
        _v_fqh=iq; \
        if(_v_fqh<(scanQPos[refid]&0xFFFFFFFF) ) \
            _v_fqh=(scanQPos[refid]&0xFFFFFFFF); \
        iQForQPosFiltering=iq; \
        if(_v_fsh<((scanQPos[refid]>>((idx)32))&0xFFFFFFFF) ) \
            _v_fsh=((scanQPos[refid]>>((idx)32))&0xFFFFFFFF)-1; \
        if(scanQPos[refid]==0) { \
            scanQIdxes[cntScanQPos]=refid; \
            cntScanQPos++; \
        } \
        scanQPos[refid]= fq|((fs+1)<<((idx)32)); \
    }\
}
#define FIXHHS(_v_fqh, _v_fsh, _v_fse, _vNew, _vfsV) { \
    if( docontinueNEW && _v_fse>=0  && _v_fsh>=0 && _v_fqh>=0) {\
        if(_vNew) \
            hashHits.addHashHit(_v_fqh,refid,_v_fsh, _v_fse, _vfsV); \
        else \
            hashHits.updateHashHit(_v_fsh, _v_fse,_vfsV);\
        _vNew = false; \
    } \
}

#if defined(scanHitsMethodNEW) && defined(scanHitsMethodOLD)
    #ifdef printMethodComparisonHits
sStr cmpMethodsOutput;
    #endif
idx cntAlignmentsNEWonly = 0, cntAlignmentsOLDonly = 0, cntAlignmentsBOTH = 0, cntAlignmentsNONE = 0, cntFilteredNEWonly = 0, cntFilteredOLDonly = 0, cntFilteredBOTH = 0, cntFilteredNONE = 0;
bool cmpMethodsOutputISDIFF = false;
#define CNTMETHODFILTERS() { \
    if(docontinueNEW && docontinueOLD){++cntFilteredBOTH;}\
    else if(docontinueNEW) {++cntFilteredNEWonly;cmpMethodsOutputISDIFF=true;}\
    else if(docontinueOLD) {++cntFilteredOLDonly;cmpMethodsOutputISDIFF=true;} \
    else {++cntFilteredNONE;} \
}

#define CNTMETHODALIGNMENTS() { \
    if(docontinueNEW && docontinueOLD){++cntAlignmentsBOTH;}\
    else if(docontinueNEW){++cntAlignmentsNEWonly;cmpMethodsOutputISDIFF=true;}\
    else if(docontinueOLD) {++cntAlignmentsOLDonly;cmpMethodsOutputISDIFF=true;}\
    else {++cntAlignmentsNONE;}\
}
#endif


idx cntHashLookup=0,cntBloomLookup=0,cntAlignmentSW=0,cntExtension=0, cntTotLetters=0,  cntCrossPass=0, cntSuccessSW=0;

idx sBioseqAlignment::zeroAlignment[2]={0,0};
idx sBioseqAlignment::resolveConflicts =(sBioseqAlignment::fAlignKeepResolvedHits|sBioseqAlignment::fAlignKeepResolvedSymmetry);

idx sBioseqAlignment::alignSeq(sVec < idx > * allHits , sBioseq * Subs, const char * qry, idx qrylen, idx idSub, idx idQry, idx flagset, idx qrysim, idx *  , idx * )
{
    if(!computeDiagonalWidth)computeDiagonalWidth=bioHash.hdr.lenUnit;

    idx oldHashList[32];

    idx flags;
#ifdef scanHitsMethodOLD
        SubScanQPos.resizeM(Subs->dim()*2);
#endif
    idx * scanQPos=SubScanQPos;
    idx * scanQIdxes=scanQPos+Subs->dim();

    idx * extensionGapBuff=ExtensionGapBuff.ptr();
    idx cntIn=0,fq,fs, hh_delta = 0;
    idx iQForQPosFiltering=0;


PERF_START("ALIGNSEQ");

    for ( idx idir=0; idir < 2; ++idir)  {

        PERF_START("ALIGN-MEM");

#ifdef scanHitsMethodOLD
        for(idx isqp=0; isqp<cntScanQPos; ++isqp)
            scanQPos[scanQIdxes[isqp]]=0;
        cntScanQPos=0;
#endif

#ifdef scanHitsMethodNEW
            hashHits.reset(qrylen);
#endif
        PERF_END();

        idx mask = (idir==0) ? fAlignForward : fAlignBackward;
        if( !(flagset&mask) ) continue;
        flags=( flagset&(~(fAlignForward|fAlignBackward)) ) | mask;

        udx chash=0,chash2=0;
        idx iStp=0;
        udx crememberHash=0;



        for( idx iq=0; iq<qrylen; ++iq ) {
            if(maxSeedSearchQueryPos && iq>maxSeedSearchQueryPos)break;




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
                {
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

            idx qrypos=iq-bioHash.hdr.lenUnit+1;
#if defined(scanHitsMethodOLD)             if(!(flagset&fAlignSearchRepeats)) {
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
#endif

#ifdef scanHitsMethodNEW
            if(!(flagset&fAlignSearchRepeats) && hashHits.hashOverlaps(iq)){
                continue;
            }
#endif
            if( hashStp>1 && iStp<(hashStp-1)){
                ++iStp;
                {
                continue;
                }
            }else iStp=0;
            if(iq<bioHash.hdr.lenUnit-1)
                {
                continue;
                }

            ++cntBloomLookup;
            if( bioHash.hb &&  ( (bioHash.hb[chash>>3] & (((udx)1)<<(chash&0x7))   ) ==0)  )
                {
                continue;
                }


            if( useDoubleHash ){
                if( bioHash.hb && (bioHash.hb [chash2>>3] & (((udx)1)<<(chash2&0x7))) ==0 )
                    {
                    continue;
                    }
            }


            ++cntHashLookup;
            #ifdef HASH_SINDEX
                idx iref=bioHash.hashInd.find(chash)-1;
                if( iref<0 )
                    {
                    continue;
                    }

                idx ofs=bioHash.hashInd[iref].ofs;
            #else
                idx ofs=bioHash.hashTbl[chash];
            #endif
            if( !ofs)
                {
                continue;
                }

            sBioseqHash::HLitem * hi0=bioHash.hList.ptr(ofs);
            sBioseqHash::HLitem * hi=hi0+1;
            idx lstdim=hi0->ref._pos;


            for (  idx il=0; il<lstdim ; ++il ) {

                sBioseqHash::RefPos & ref=hi->ref;
                hi=bioHash.hList.ptr(hi->next);

                idx refid=ref.id();

                if(idSub!=sNotIdx && refid!=idSub)
                    continue;

                if( (flagset & fAlignKeepFirstMatch) && (flagset & fAlignReverseEngine) && SHitBitmask && (SHitBitmask[refid / 8] & (((idx) 1) << (refid % 8))) )
                    continue;


                idx subpos=ref.pos();
                idx ihsubpos = subpos;



                bool docontinueOLD = false, docontinueNEW = false;
                bool doAddNEWHit = true;

#ifdef scanHitsMethodNEW
                docontinueNEW=true;
#endif
#ifdef scanHitsMethodOLD
                docontinueOLD=true;
#endif

#ifdef scanHitsMethodOLD
                if(flagset&fAlignSearchRepeats) {
                    if(qrypos>(scanQPos[refid]&0xFFFFFFFF) )
                        scanQPos[refid]&=0xFFFFFFFF;
                } else {
                    if(iq>iQForQPosFiltering && qrypos < (scanQPos[refid]&0xFFFFFFFF) ) {
    #ifdef scanHitsMethodNEW
                        docontinueOLD = false;
    #else
                        continue;
    #endif
                    }
                }

                {
                    if(subpos<(((scanQPos[refid]>>((idx)32))&0xFFFFFFFF)))
    #ifdef scanHitsMethodNEW
                        docontinueOLD = false;
    #else
                        continue;
    #endif
                }
#endif
#ifdef scanHitsMethodNEW

                fs = ihsubpos + (bioHash.hdr.lenUnit);
                if(hashHits.hitExists(refid,ihsubpos, fs, (flagset&fAlignSearchRepeats)?iq:-1, hh_delta)) {
#    ifdef scanHitsMethodOLD
                    docontinueNEW = false;
#    else
                    continue;
#    endif
                }
#endif

#if defined(scanHitsMethodNEW) && defined(scanHitsMethodOLD)
    #if defined(printMethodComparisonHits)
                cmpMethodsOutput.printf("FILTER HIT : Read=%10" DEC " idir=%" DEC " iq=%3" DEC " refid=%4" DEC " subpos=%10" DEC " OLD=\'%c\' NEW=\'%c\' \t%s\n",idQry,idir,iq,refid,ihsubpos, (docontinueOLD?'+':'-'), (docontinueNEW?'+':'-'), (docontinueNEW!=docontinueOLD)?"DIFFFF":"");
    #endif
                CNTMETHODFILTERS();
                if( !(docontinueNEW || docontinueOLD) ) {
                    continue;
                }
#endif
                const char * sub=Subs->seq(refid);


                if( useDoubleHash ) {
                    idx p=subpos-(bioHash.hdr.lenUnit);
                    if( p < 0 )
                        continue;
                    udx  h2ash=_seqBytes(sub, p, bioHash.hdr.lenUnit );
                    #ifdef _DEBUG
                        sBioseq::uncompressATGC(dbg3, (const char * ) &h2ash, 0,bioHash.hdr.lenUnit, 0);
                    #endif

                    if(chash2!=h2ash)
                        continue;

                }


                idx sublen=Subs->len(refid);
                idx curofs=allHits->dim();

                idx matchesAll=sIdxMax;
                idx qryposN=qrypos;
                if( maxMissQueryPercent >= 0 ){

                    PERF_START("EXTENDER");
                    matchesAll=0;
                    idx inShift=0;
                    iqx=QIDX(qrypos,qrylen);
                    char prvSubLet=_seqBits(sub, subpos);
                    char prvQryLet=_seqBits(qry, iqx , flags);

                    idx isub=0,iqry=0, isubfirstnonmatch=-1,iqryfirstnonmatch=-1;
                    idx isubleft=subpos,iqryleft=qrypos;
                    idx isubright=subpos+bioHash.hdr.lenUnit,iqryright=qrypos+bioHash.hdr.lenUnit;
                    ++cntExtension;
                    idx iGapMinus=extGapMidPos,iGapPlus=extGapMidPos;

                    for ( idx ext = -1; ext<2 ; ext+=2 ) {

                        if(ext<0){
                            isub=subpos-1;iqry=qrypos-1;
                            if( useDoubleHash ) {
                                isub-=(bioHash.hdr.lenUnit-1);
                                iqry-=(bioHash.hdr.lenUnit-1);
                                isubleft-=(bioHash.hdr.lenUnit);
                                iqryleft-=(bioHash.hdr.lenUnit);
                            }
                        }
                        else {
                            isub=subpos+bioHash.hdr.lenUnit;
                            iqry=qrypos+bioHash.hdr.lenUnit;
                        }
                        idx diffs=0 ,gaps=0 ;
                        idx matches=bioHash.hdr.lenUnit ;
                        if( useDoubleHash && ext<0)
                            matches+=bioHash.hdr.lenUnit ;


                        while( gaps<=(maxExtensionGaps+1) && diffs*100 <= sMin((real)looseExtenderMismatchesPercent,maxMissQueryPercent)*((considerGoodSubalignments == 2)?qrylen:((considerGoodSubalignments == 1)?minMatchLen:(matches +diffs))) ) {

                            if(ext>0 && (isub>=sublen || iqry>=qrylen) ){isubright=isub;iqryright=iqry;break;}
                            if(ext<0 && (isub<0 || iqry<0) )break;

                            iqx=QIDX(iqry,qrylen);
                            char qryLet = _seqBits(qry, iqx , flags);
                            char subLet = _seqBits(sub, isub);

                            if(qryLet!=subLet){
                                if( iqryfirstnonmatch==-1 && ext>0 )iqryfirstnonmatch=iqry;
                                if( isubfirstnonmatch==-1 && ext>0 )isubfirstnonmatch=isub;

                                if(gaps==1){
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
                                        gapPos=(isub+(ext>0 ? 1 : 1 ));
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
                                }
                                else {
                                    ++diffs;++gaps;
                                    inShift=0;
                                }

                            }
                            else {
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

#ifdef scanHitsMethodOLD
                    if(ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment) {
                        fs = (isubfirstnonmatch==-1) ? (subpos+bioHash.hdr.lenUnit) : isubfirstnonmatch;
                    }
                    else fs=-1;
                    if(ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment)
                        fq=(iqryfirstnonmatch==-1) ? (qryposN+bioHash.hdr.lenUnit) : iqryfirstnonmatch;
                    else fq=-1;
                    if(fs!=-1 || fq!=-1) {
                        FIXFQFS(fq,fs);
                    }
#endif

#ifdef scanHitsMethodNEW
                    if(isubfirstnonmatch<ihsubpos) {
                        isubright = ihsubpos+bioHash.hdr.lenUnit;
                    }
                    fs = (isubfirstnonmatch==-1) ? isubright : isubfirstnonmatch;
                    FIXHHS(iq,subpos ,fs,doAddNEWHit,false);
#endif

                    idx compareLength=(flags&fAlignIdentityOnly) ? minMatchLen : 0;
                    if( matchesAll < compareLength){
                        if( matchesAll>=allowShorterEnds && ((idir==0 && isub>=sublen-3) || (idir==1 && subpos<3)) ) {
                        } else {
                            continue;
                        }
                    }

#ifdef scanHitsMethodOLD
                    fs = (isubfirstnonmatch==-1) ? (subpos+bioHash.hdr.lenUnit) : isubfirstnonmatch;
                    fq= (iqryfirstnonmatch==-1) ? (qryposN+bioHash.hdr.lenUnit) : iqryfirstnonmatch;
                    FIXFQFS(fq,fs);
#endif


                    if( matchesAll==qrylen || flags&fAlignIdentityOnly){
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
                        allHits->addM(sizeof(Al)/sizeof(idx)+dimal);
                        Al * hdr=(Al*)allHits->ptr(curofs);
                        sSet(hdr);

                        hdr->setFlags(flags|sBioseqAlignment::fAlignCompressed);
                        hdr->setDimAlign(dimal);
                        hdr->setSubStart(subpos);
                        hdr->setQryStart(qryposN);
                        hdr->setLenAlign(iqry-qryposN);
                        hdr->setIdSub(refid);
                        hdr->setIdQry(idQry);
                        hdr->setScore((iqry-qryposN)*costMatch);

                        idx * m=(idx*)sShift(hdr,sizeof(Al) );
                        m[0]=0;m[1]=0;
                        m[2]=-matchesAll;
                        if(matchesAll!=qrylen) {
                            idx extent, ig = 2;

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
                            m[ig]=-(iqry-iqryleft);
                            isubright = hdr->subStart() + m[ig-2];
                        }


                        fq=iqry;
                        fs=isub;
#ifdef scanHitsMethodNEW
                        FIXHHS(iq,hdr->subStart() ,isubright,doAddNEWHit,true);
#endif
#ifdef scanHitsMethodOLD
                        FIXFQFS(fq,fs);
#endif

#if defined(scanHitsMethodNEW) && defined(scanHitsMethodOLD)
    #ifdef printMethodComparisonHits
                    cmpMethodsOutput.printf("FAST ALIGN : Read=%10" DEC " idir=%" DEC " iq=%3" DEC " refid=%4" DEC " ihsubpos=%10" DEC " substart=%10" DEC " subend=%10" DEC " OLD=\'%c\' NEW=\'%c\' \t%s\n",idQry,idir,iq,refid,ihsubpos, subpos, isubright,(docontinueOLD?'+':'-'), (docontinueNEW?'+':'-'), (docontinueNEW!=docontinueOLD)?"DIFFFF":"");
    #endif
                    CNTMETHODALIGNMENTS();
#endif
                        ++cntIn;
                        if(flags&fAlignReverseEngine)
                            reverseAlignment(hdr, 0);

                        PERF_END();

                        if(QHitBitmask)
                            QHitBitmask[idQry/8]|=(((idx)1)<<(idQry%8));
                        if(SHitBitmask)
                            SHitBitmask[refid/8]|=(((idx)1)<<(refid%8));
                        if( (flagset&fAlignKeepFirstMatch) && (flagset&fAlignReverseEngine)==0 ){
                            return cntIn;
                        }
                        if(maxHitsPerRead && cntIn>maxHitsPerRead){
                            return cntIn;
                        }

                        continue;
                    }
                }


                PERF_START("ALIGNSMITHWATERMAN");

                idx lenBeforeNmerMatch=sMin ( qryposN, subpos) ;
                idx qryStart=qryposN-lenBeforeNmerMatch;
                idx subStart=subpos-lenBeforeNmerMatch;
                idx qryLenAlign= computeDiagonalWidth+sMin ( (sublen-subStart) , (qrylen-qryStart) );
                idx subLenAlign = qryLenAlign;
                if( qryStart+qryLenAlign>qrylen)qryLenAlign=qrylen-qryStart;
                if( subStart+subLenAlign>sublen)subLenAlign=sublen-subStart;


                idx lastQryEnd=qryStart, lastSubEnd=subStart, okhit=false;
                if(matchesAll>=minMatchLen*looseExtenderMinimumLengthPercent/100) {

                    ++cntAlignmentSW;
                    okhit = alignSmithWaterman(allHits,sub, subStart, subLenAlign, qry, qryStart, qryLenAlign,  flags , sublen, qrylen, lenBeforeNmerMatch+computeDiagonalWidth, &lastQryEnd, &lastSubEnd );

                }
                Al * hdr=(Al*)allHits->ptr(curofs);


                if(okhit) {
                    fq=lastQryEnd-bioHash.hdr.lenUnit+1;
                    fs=lastSubEnd-bioHash.hdr.lenUnit+1;
                    FIXHHS(iq,hdr->getSubjectStart(hdr->match()),lastSubEnd,doAddNEWHit,true);
                    FIXFQFS(fq,fs);
#if defined(scanHitsMethodNEW) && defined(scanHitsMethodOLD)
    #ifdef printMethodComparisonHits
                    cmpMethodsOutput.printf("SLOW ALIGN : Read=%10" DEC " idir=%" DEC " iq=%3" DEC " refid=%4" DEC " ihsubpos=%10" DEC " substart=%10" DEC " subend=%10" DEC " OLD=\'%c\' NEW=\'%c\' \t%s\n",idQry,idir,iq,refid,ihsubpos,subStart, lastSubEnd, (docontinueOLD?'+':'-'), (docontinueNEW?'+':'-'), (docontinueNEW!=docontinueOLD)?"DIFFFF":"");
    #endif
                    CNTMETHODALIGNMENTS();
#endif
                }
                PERF_END();


                if(okhit) {
                    ++cntSuccessSW;
                    hdr->setIdSub ( (flags&fAlignReverseEngine) ? idQry : refid );
                    hdr->setIdQry ( (flags&fAlignReverseEngine) ? refid : idQry ) ;



                    ++cntIn;
                    if(QHitBitmask)
                        QHitBitmask[idQry/8]|=(((idx)1)<<(idQry%8));
                    if(SHitBitmask)
                        SHitBitmask[refid/8]|=(((idx)1)<<(refid%8));

                    if( (flagset&fAlignKeepFirstMatch) && (flagset&fAlignReverseEngine)==0 )
                        return cntIn;
                    if(maxHitsPerRead && cntIn>maxHitsPerRead)
                        return cntIn;

                }else {
                    allHits->cut(curofs);
                }

            }

        }

    }
    PERF_END();

    if(iQForQPosFiltering)
        iQForQPosFiltering=0;

    return cntIn;
}




idx sBioseqAlignment::alignSmithWaterman( sVec < idx > * al, const char * sub, idx substart, idx sublen, const char * qry, idx qrystart, idx qrylen, idx flags , idx , idx qrybuflen, idx startFloatingDiagonal, idx * pLastQryEnd,  idx * pLastSubEnd )
{

PERF_START("SMITHWATERMANN-PREPARATION");
    if(!qrybuflen)qrybuflen=qrylen;

    if(2*computeDiagonalWidth + 1 > qrylen) {
        flags &= ~((idx)fAlignOptimizeDiagonal);
    }

    idx reserve=1024*1024;
    idx maxSeq=qrylen*2+computeDiagonalWidth;
    idx SWMatrixWidth=qrylen ;
    if((flags&fAlignOptimizeDiagonal) && computeDiagonalWidth) {
        SWMatrixWidth=computeDiagonalWidth*2;
    }

    idx sz=(maxSeq+2)*(SWMatrixWidth+1);if(sz<reserve)sz=reserve;
    MatSW.resizeM(sz*sizeof(long)+(sublen+1)*sizeof(long));
    long * matP=(long *)MatSW.ptr();
    long * floatStart = matP+sz;
    sz=(maxSeq+1)*SWMatrixWidth;if(sz<reserve)sz=reserve;
    MatBR.resizeM(sz);
    char * matB=MatBR.ptr();



    #define mat(_v_i, _v_j) matP[(SWMatrixWidth+1)*(_v_i)+((_v_j)-floatStart[(_v_i)])]
    #define bak(_v_i, _v_j) matB[(SWMatrixWidth)*(_v_i)+((_v_j)-floatStart[(_v_i+1)])]


    idx fv,cv=0;
    idx maxAll=0, maxAllLastLetter=0;
    idx maxS=0, maxQ=0, maxRowLastLetter=0, maxColLastLetter=0;



    idx is, iq ,iFloatingDiagonal;
    idx prevCost=0;

PERF_END();

#ifdef DEBUGGING_SMITHWATERMANN
::printf("   \n\n");
#endif

    idx qbqs=qrybuflen-qrystart-1;
    bool complementQ=( ( (flags&fAlignForwardComplement) && (flags&fAlignForward) ) ||
            ( (flags&fAlignBackwardComplement) &&(flags&fAlignBackward) ) ) ?
            true : false;

PERF_START("SMITHWATERMANN-RECOVERY");

    idx qStart=0, qEnd=(flags&fAlignOptimizeDiagonal ) ? (SWMatrixWidth+1) : qrylen;
    static idx last_qEnd=(flags&fAlignGlobal)?0:qEnd;

    floatStart[0]=0;
    static long initBoundValue = (flags&fAlignGlobal)?costGapNext:0;
    long initSWvalue = (sublen+qrylen)*initBoundValue;

    for( is=0; is<sublen; ++is ) {
        floatStart[is+1]=0;
        for( iq=qStart; iq<qEnd; ++iq ) {
            mat(is+1,iq+1)=initSWvalue;
            bak(is,iq)=0;
        }
    }
    if( last_qEnd != qEnd ) {
        for( iq=qStart; iq<qEnd; ++iq ) {
            mat(0,iq+1)=(iq+1)*initBoundValue;
        }
        for( is=0; is<sublen; ++is ) {
            mat(is+1,0)=(is+1)*initBoundValue;
        }
        last_qEnd = qEnd;
    }
PERF_END();

    qEnd=(flags&fAlignOptimizeDiagonal ) ? (1+computeDiagonalWidth) : qrylen;


    idx lastqEnd = qEnd;
PERF_START("SMITHWATERMANN-ACTUAL-ALGORITHM");
    for(  is=0, iFloatingDiagonal=0; is<sublen; ++is, ++iFloatingDiagonal ) {

        #ifdef old_stile
            idx isx=SIDX( (substart+is) , subbuflen);
            idx sBits=_seqBits(sub, isx, nonCompFlags );
        #else
            idx isx=substart+is;
            idx sBits=(idx)((sub[(isx>>2)]>>((isx&3)<<1))&0x3) ;
        #endif 

        idx maxRow=0;
        idx maxRowPos=0;
#ifdef DEBUGGING_SMITHWATERMANN
::printf( "%" DEC " %c  :" , is , sBioseq::mapRevATGC[sBits]);
#endif
        floatStart[is+1]=qStart;

        for( iq=qStart; iq<qEnd; ++iq ) {



            #ifdef old_stile
                idx iqx=QIDX((qrystart+iq),qrybuflen);
                idx qBits=_seqBits(qry, iqx, flags)  ;
            #else
                idx iqx=((flags&sBioseqAlignment::fAlignBackward) ? (qbqs-iq) : (qrystart+iq));
                idx qBits=(idx)((qry[(iqx>>2)]>>((iqx&3)<<1))&0x3) ;
                if( complementQ)
                    qBits=sBioseq::mapComplementATGC[qBits];
            #endif


            cv=mat(is+1,iq+1);
            idx costMMatch=_costMatch( sBits , qBits   );
            fv=mat(is,iq)+costMMatch;
            prevCost=costMMatch;
            if( cv<=fv ) {cv=fv; bak(is,iq)=3; };
            fv=(( iq+1 < lastqEnd )?mat(is,iq+1):0) + ( (is==0 || bak(is-1,iq)==3) ? costGapOpen : costGapNext);
            if( cv<fv ) {cv=fv; bak(is,iq)=1; }
            fv=((iq > qStart)?mat(is+1,iq):0)+( (iq==0 || bak(is,iq-1)==3) ? costGapOpen : costGapNext);
            if( cv<fv ) {cv=fv; bak(is,iq)=2;};

            if(cv>0){
                mat(is+1,iq+1)=(long)cv;

                if(cv>=maxRow){
                    maxRow=cv;
                    maxRowPos=iq;
                }
            }
#ifdef DEBUGGING_SMITHWATERMANN
::printf("%c %4" DEC " | ",cv>=maxRow ? sBioseq::mapRevATGC[qBits] : sBioseq::mapRevATGC[qBits]+'a'-'A' ,cv);
#endif

        }
        lastqEnd = qEnd;
#ifdef DEBUGGING_SMITHWATERMANN
::printf("\n");
#endif

        if( flags&fAlignGlobal) {
            if( maxAllLastLetter< cv ) {
                maxAllLastLetter=cv;
                maxRowLastLetter=is;
                maxColLastLetter=qEnd-1;
            }
        }

        if(maxAll<maxRow || ( (flags&fAlignMaxExtendTail) && maxAll==maxRow) ) {
            maxAll=maxRow;
            maxS=is;
            maxQ=maxRowPos;
        }



        if(flags&fAlignOptimizeDiagonal) {
            if(startFloatingDiagonal!=0 && is>startFloatingDiagonal) {
                if(maxRowPos>iFloatingDiagonal)++iFloatingDiagonal;
                if(maxRowPos<iFloatingDiagonal)--iFloatingDiagonal;
                if((flags&fAlignGlobal) && iFloatingDiagonal+computeDiagonalWidth < is) {
                    iFloatingDiagonal++;
                }
            }

            qStart=iFloatingDiagonal-computeDiagonalWidth;
            if(qStart<0)qStart=0;
            qEnd=iFloatingDiagonal+computeDiagonalWidth;
            if(qEnd>qrylen)qEnd=qrylen;
        }


        if(qStart>=qrylen-1)
            break;

    }
PERF_END();

PERF_START("SMITHWATERMANN-COINTAINERIZATION");
    idx curofs=al->dim();
    al->addM(sizeof(Al)/sizeof(idx));
    Al * hdr=(Al*)al->ptr(curofs);
    sSet(hdr);

    hdr->setFlags(flags);

    if(flags&fAlignGlobal) {
        maxAll=maxAllLastLetter;
        maxS=maxRowLastLetter;
        maxQ=maxColLastLetter;
    }else


    hdr->setDimAlign(0);
    hdr->setSubStart(substart);
    hdr->setQryStart(qrystart);
    if(pLastQryEnd)*pLastQryEnd=maxQ+qrystart+1;
    if(pLastSubEnd)*pLastSubEnd=maxS+substart+1;

    if(flags&fAlignReverseEngine){
        hdr->starts=sSwapHiLo(hdr->starts);
    }

    idx  matchcounters[4]; sSet(matchcounters,0,sizeof(idx)*4);
    idx dlen =1+( (minMatchLen<=0?1:minMatchLen))/(sizeof(idx)*8);
    bool oklocal = false, match = false;
    localBitMatch.resize(dlen);sSet(localBitMatch.ptr(),0,sizeof(idx)*dlen);

    static sVec<idx> tails_window_m(sMex::fSetZero|sMex::fExactSize),tails_window_iq(sMex::fSetZero|sMex::fExactSize),tails_window_is(sMex::fSetZero|sMex::fExactSize);
    tails_window_m.resize(trimLowScoreEnds);tails_window_iq.resize(trimLowScoreEnds);tails_window_is.resize(trimLowScoreEnds);
    tails_window_m.set(0);tails_window_iq.set(0);tails_window_is.set(0);
    idx tails_window_score = 0, iqL = -1, iqR = -1, isL = -1, isR = -1, idL = -1, idR = -1, itw = 0;
    for(dlen=0, is=maxS, iq=maxQ, cv=maxAll;  ; cv=mat(is+1,iq+1) , dlen+=2) {
        if(flags&fAlignGlobal){ if(iq<0 || is<0) break;}
        else if(cv<=0) break;
        match = false;
        char bw=bak(is,iq);
        if( ((!(flags&fAlignMaxExtendTail)) && !(flags&fAlignGlobal)) && cv<costMatchFirst ){ break;}

        if(bw==0x02)++matchcounters[2];
        else if(bw==0x01)++matchcounters[3];
        else if(cv<mat(is,iq)+costMatchFirst)++matchcounters[1];
        else { ++matchcounters[0];match=true;}

        if( trimLowScoreEnds > 0 ) {
            itw = (dlen/2)%trimLowScoreEnds;
            if( dlen>=2*trimLowScoreEnds )
                tails_window_score -= tails_window_m[itw];
            tails_window_m[itw]=match?1:0;
            tails_window_score += tails_window_m[itw];
            tails_window_iq[itw]=iq;
            tails_window_is[itw]=is;

            if( dlen>=2*trimLowScoreEnds ) {
                if( tails_window_score*100 > trimLowScoreEndsMaxMM*trimLowScoreEnds) {
                    if(iqL < 0 && tails_window_m[(itw+1)%trimLowScoreEnds] > 0 ) {
                        idL = dlen-2*(trimLowScoreEnds-1);
                        iqL = tails_window_iq[(itw+1)%trimLowScoreEnds];
                        isL = tails_window_is[(itw+1)%trimLowScoreEnds];
                    }
                    if( match ) {
                        idR = dlen; iqR = iq; isR = is;
                    }
                }
            }
            if( iqL < 0 && match ) {
                idR = dlen;iqR = iq;isR = is;
            }
        }
        if( considerGoodSubalignments == 1 && !oklocal){
            idx bitptr = (( dlen / 2)%minMatchLen) / (sizeof(idx) * 8), bitpos = ((( dlen / 2)%minMatchLen) % (sizeof(idx) * 8));
            if( localBitMatch[bitptr] & ((idx)1 << bitpos ) ) {
                --matchcounters[0];
                if(!match) {
                    localBitMatch[bitptr] &= ~((idx)1 << bitpos );
                }
            }
            localBitMatch[bitptr] |= (idx)match << bitpos;
            if( matchcounters[0]*100 >= minMatchLen*(100-maxMissQueryPercent) ){
                oklocal = true;
            }
        }
        if(!bw) break;
        if(bw & 0x1)--is;
        if(bw & 0x2)--iq;
    }
    idx firstIS = is + 1, firstIQ = iq + 1;

PERF_END();

PERF_START("SMITHWATERMANN-FETCHING");

    bool okhit=true;

    if(trimLowScoreEnds > 0 ) {
        if(idL < 0 ) {
            dlen = 0;
            okhit = false;
        } else {
            dlen = idR-idL+2;
            maxQ = iqL;
            maxS = isL;
            maxAll = mat(maxS,maxQ) - mat(isR,iqR);
        }
    }

    if( dlen<2*minMatchLen) {
        if( !(flags&fAlignBackward) && maxS+substart+1>=sublen-3 && dlen>=2*allowShorterEnds )
            okhit=true;
        else if( (flags&fAlignBackward) && substart<3 && dlen>=2*allowShorterEnds )
            okhit=true;
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
    } else
    {

        idx * m=al->addM(dlen);
        hdr=(Al*)al->ptr(curofs);

        bak(0,0)=3;
        hdr->setDimAlign(dlen);

        idx qq=0,ss=1;
        if(flags&fAlignReverseEngine) {
            qq=1;ss=0;
        }

        for(dlen-=2, is=maxS, iq=maxQ, cv=maxAll;  dlen>=0 ; cv=mat(is+1,iq+1) , dlen-=2) {

            char bw=bak(is,iq);

            m[dlen+qq]= bw==0x02 ? -1 : (is - firstIS);
            m[dlen+ss]= bw==0x01 ? -1 : (iq - firstIQ);

            if(bw & 0x1)--is;
            if(bw & 0x2)--iq;
        }

        hdr->setSubStart(substart+firstIS);
        hdr->setQryStart(qrystart+firstIQ);

        hdr->setScore(maxAll);
        hdr->setLenAlign(hdr->dimAlign()/2);

        hdr->setDimAlign(compressAlignment(hdr, m, m ));
        al->cut(curofs+sizeof(sBioseqAlignment::Al)/sizeof(idx)+hdr->dimAlign());


    }

PERF_END();





    return okhit ? 1 : 0 ;
    #undef mat
}


typedef real SWMType;
#define DEBUGGING_SMITHWATERMANN
sBioseqAlignment::Al * sBioseqAlignment::alignSWProfile (sVec < idx > & al, const char ** subs, idx ** sub_ms, idx substart, idx sublen, idx subcnt, const char ** qrys, idx ** qry_ms, idx qrybuflen, idx qrystart, idx qrylen, idx qrycnt, idx flags)
{
    if(2*computeDiagonalWidth + 1 > qrylen) {
        flags &= ~((idx)fAlignOptimizeDiagonal);
    }
    idx startFloatingDiagonal = computeDiagonalWidth;

    idx reserve=1024*1024;
    idx maxSeq=sMax(qrylen,sublen)*2+computeDiagonalWidth;
    idx SWMatrixWidth=qrylen ;
    if((flags&fAlignOptimizeDiagonal) && computeDiagonalWidth) {
        SWMatrixWidth=computeDiagonalWidth*2;
    }

    idx sz=(maxSeq+2)*(SWMatrixWidth+1);if(sz<reserve)sz=reserve;


    MatSW.resizeM(sz*sizeof(SWMType)+(sublen+1)*sizeof(SWMType));
    SWMType * matP=(SWMType *)MatSW.ptr();
    long * floatStart=(long*)(matP+sz);

    sz=(maxSeq+1)*SWMatrixWidth;if(sz<reserve)sz=reserve;
    MatBR.resizeM(sz);
    char * matB=MatBR.ptr();

    #define mat(_v_i, _v_j) matP[(SWMatrixWidth+1)*(_v_i)+((_v_j)-floatStart[(_v_i)])]
    #define bak(_v_i, _v_j) matB[(SWMatrixWidth)*(_v_i)+((_v_j)-floatStart[(_v_i+1)])]

    SWMType fv,cv=0, costMMatch = 0;;
    idx maxAll=0, maxAllLastLetter=0;
    idx maxS=0, maxQ=0, maxRowLastLetter=0, maxColLastLetter=0;


    idx is, isx, iis, * sub_m, iq, iqx, iiq, * qry_m;
    const char * sub, *qry;
    idx iFloatingDiagonal;

    idx qStart=0, qEnd=(flags&fAlignOptimizeDiagonal ) ? (SWMatrixWidth+1) : qrylen;
    static idx last_qEnd=(flags&fAlignGlobal)?0:qEnd;

    floatStart[0]=0;
    static long initBoundValue = (flags&fAlignGlobal)?costGapNext:0;
    idx initSWvalue = (flags&fAlignGlobal)?-(REAL_MAX/2):0;

    for( is=0; is<sublen; ++is ) {
        floatStart[is+1]=0;
        for( iq=qStart; iq<qEnd; ++iq ) {
            mat(is+1,iq+1)=initSWvalue;
            bak(is,iq)=0;
        }
    }
    if( last_qEnd != qEnd ) {
        for( iq=qStart; iq<qEnd; ++iq ) {
            mat(0,iq+1)=(iq+1)*initBoundValue;
        }
        for( is=0; is<sublen; ++is ) {
            mat(is+1,0)=(is+1)*initBoundValue;
        }
        last_qEnd = qEnd;
    }

    qEnd=(flags&fAlignOptimizeDiagonal ) ? computeDiagonalWidth : qrylen;
    idx qbqs = qrybuflen - qrystart - 1;
    bool complementQ=( ( (flags&fAlignForwardComplement) && (flags&fAlignForward) ) ||
            ( (flags&fAlignBackwardComplement) &&(flags&fAlignBackward) ) ) ?
            true : false;

#ifdef _DBG_PROF_SW
    sStr dbgout;
#endif
    real profSub[5];idx sBits;
    for(  is=0, iFloatingDiagonal=0; is<sublen; ++is, ++iFloatingDiagonal ) {
        idx maxRow = -sIdxMax;
        idx maxRowPos=0;
        floatStart[is+1]=qStart;
#ifdef _DBG_PROF_SW
        for(iq=0; iq < qStart ; ++iq) {
            dbgout.printf(",");
        }
#endif
        sSet(profSub,0,5*sizeof(real));
        for( iis = 0 ; iis < subcnt ; ++iis) {
            sub = subs[iis];
            sBits = 0;
            isx=substart+is;
            if( !sub_ms ) {
                sBits=(idx)((sub[(isx>>2)]>>((isx&3)<<1))&0x3) ;
            } else {
                sub_m = sub_ms[iis];
                if( sub_m[2*isx+1] < 0 ) {
                    sBits = 4;
                } else {
                    isx = sub_m[2*isx+1];
                    sBits=(idx)((sub[(isx>>2)]>>((isx&3)<<1))&0x3) ;
                }
            }
            profSub[sBits]++;
        }
        for(idx isp = 0 ; isp < 5 ; ++isp)profSub[isp]/=subcnt;

        for( iq=qStart; iq<qEnd; ++iq ) {
            costMMatch = 0;
            for( iis = 0 ; iis < 5 ; ++iis) {
                if(profSub[iis]==0)continue;

                for ( iiq = 0 ; iiq < qrycnt ; ++iiq )  {
                    qry = qrys[iiq];
                    idx qBits = -1;
                    iqx=((flags&sBioseqAlignment::fAlignBackward) ? (qbqs-iq) : (qrystart+iq));
                    if( !qry_ms ) {
                        qBits=(idx)((qry[(iqx>>2)]>>((iqx&3)<<1))&0x3) ;
                    } else {
                        qry_m = qry_ms[iiq];
                        if( qry_m[2*iqx+1] < 0 ) {
                            qBits = -2;
                        } else {
                            iqx = qry_m[2*iqx+1];
                            qBits=(idx)((qry[(iqx>>2)]>>((iqx&3)<<1))&0x3) ;
                        }
                    }
                    if( complementQ)
                        qBits=sBioseq::mapComplementATGC[qBits];

                    costMMatch += _profile_costMatch(iis , qBits)*profSub[iis];
                }

            }
            costMMatch = costMMatch/qrycnt;
            cv=mat(is+1,iq+1);
            fv=mat(is,iq)+costMMatch;
            if( cv<=fv ) {cv=fv; bak(is,iq)=3; };
            fv=mat(is,iq+1) + ( (is==0 || bak(is-1,iq)==3) ? costGapOpen : costGapNext);
            if( cv<fv ) {cv=fv; bak(is,iq)=1; }
            fv=mat(is+1,iq)+( (iq==0 || bak(is,iq-1)==3) ? costGapOpen : costGapNext);
            if( cv<fv ) {cv=fv; bak(is,iq)=2;};
#ifdef _DBG_PROF_SW
            dbgout.printf(",%3.2lf",cv);
#endif
            if(cv>0 || (flags&fAlignGlobal)){
                mat(is+1,iq+1)=(SWMType)cv;

                if(cv>=maxRow){
                    maxRow=cv;
                    maxRowPos=iq;
                }
            }
        }
#ifdef _DBG_PROF_SW
        dbgout.printf("\n");
#endif
        if( flags&fAlignGlobal) {
                maxAllLastLetter=maxRow;
                maxRowLastLetter=is;
                maxColLastLetter=qEnd-1;
        }

        if(maxAll<maxRow || ( (flags&fAlignMaxExtendTail) && maxAll==maxRow) ) {
            maxAll=maxRow;
            maxS=is;
            maxQ=maxRowPos;
        }

        if(flags&fAlignOptimizeDiagonal) {
            if(startFloatingDiagonal!=0 && is>startFloatingDiagonal) {
                if(maxRowPos>iFloatingDiagonal)++iFloatingDiagonal;
                if(maxRowPos<iFloatingDiagonal)--iFloatingDiagonal;
                if((flags&fAlignGlobal) && iFloatingDiagonal+computeDiagonalWidth < is) {
                    iFloatingDiagonal++;
                }
            }
            qStart=iFloatingDiagonal-computeDiagonalWidth;
            if(qStart<0)qStart=0;
            qEnd=iFloatingDiagonal+computeDiagonalWidth;
            if(qEnd>qrylen)qEnd=qrylen;
        }


        if(qStart>=qrylen-1)
            break;

    }

    if(flags&fAlignGlobal) {
        maxAll=maxAllLastLetter;
        maxS=maxRowLastLetter;
        maxQ=maxColLastLetter;
    }

    return backTracking(&al,substart,sublen,qrystart,qrylen,SWMatrixWidth,(idx)(floatStart - (long*)matP),maxS,maxQ,maxAll,flags);
}

sBioseqAlignment::Al * sBioseqAlignment::backTracking(sVec < idx > * al, idx substart, idx sublen, idx qrystart, idx qrylen, idx SWMatrixWidth, idx floatSZ, idx maxS, idx maxQ, idx maxScore, idx flags) {
    SWMType * matP=(SWMType *)MatSW.ptr();
    char * matB=MatBR.ptr();

    long * floatStart = (long *)(matP+floatSZ);
    #define mat(_v_i, _v_j) matP[(SWMatrixWidth+1)*(_v_i)+((_v_j)-floatStart[(_v_i)])]
    #define bak(_v_i, _v_j) matB[(SWMatrixWidth)*(_v_i)+((_v_j)-floatStart[(_v_i+1)])]

    idx curofs=al->dim();
    al->addM(sizeof(Al)/sizeof(idx));
    Al * hdr=(Al*)al->ptr(curofs);
    sSet(hdr);

    hdr->setFlags(flags);


    hdr->setDimAlign(0);
    hdr->setSubStart(substart);
    hdr->setQryStart(qrystart);

    if(flags&fAlignReverseEngine){
        hdr->starts=sSwapHiLo(hdr->starts);
    }

    idx  matchcounters[4]; sSet(matchcounters,0,sizeof(idx)*4);
    idx dlen =1+( (minMatchLen<=0?1:minMatchLen))/(sizeof(idx)*8),  is=0, iq=0;
    SWMType cv=0;
    bool oklocal = false, match = false;
    localBitMatch.resize(dlen);sSet(localBitMatch.ptr(),0,sizeof(idx)*dlen);

    static sVec<idx> tails_window_m(sMex::fSetZero|sMex::fExactSize),tails_window_iq(sMex::fSetZero|sMex::fExactSize),tails_window_is(sMex::fSetZero|sMex::fExactSize);
    tails_window_m.resize(trimLowScoreEnds);tails_window_iq.resize(trimLowScoreEnds);tails_window_is.resize(trimLowScoreEnds);
    tails_window_m.set(0);tails_window_iq.set(0);tails_window_is.set(0);
    idx tails_window_score = 0, iqL = -1, iqR = -1, isL = -1, isR = -1, idL = -1, idR = -1, itw = 0;
    for(dlen=0, is=maxS, iq=maxQ, cv=maxScore;  ; cv=mat(is+1,iq+1) , dlen+=2) {
        if(flags&fAlignGlobal){ if(iq<0 || is<0) break;}
        else if(cv<=0) break;

        match = false;
        char bw=bak(is,iq);
        if( (!(flags&fAlignMaxExtendTail) && !(flags&fAlignGlobal)) && cv<costMatchFirst ){ break;}

        if(bw==0x02)++matchcounters[2];
        else if(bw==0x01)++matchcounters[3];
        else if(cv<mat(is,iq)+costMatchFirst)++matchcounters[1];
        else { ++matchcounters[0];match=true;}

        if( trimLowScoreEnds > 0 ) {
            itw = (dlen/2)%trimLowScoreEnds;
            if( dlen>=2*trimLowScoreEnds )
                tails_window_score -= tails_window_m[itw];
            tails_window_m[itw]=match?1:0;
            tails_window_score += tails_window_m[itw];
            tails_window_iq[itw]=iq;
            tails_window_is[itw]=is;

            if( dlen>=2*trimLowScoreEnds ) {
                if( tails_window_score*100 > trimLowScoreEndsMaxMM*trimLowScoreEnds) {
                    if(iqL < 0 && tails_window_m[(itw+1)%trimLowScoreEnds] > 0 ) {
                        idL = dlen-2*(trimLowScoreEnds-1);
                        iqL = tails_window_iq[(itw+1)%trimLowScoreEnds];
                        isL = tails_window_is[(itw+1)%trimLowScoreEnds];
                    }
                    if( match ) {
                        idR = dlen; iqR = iq; isR = is;
                    }
                }
            }
            if( iqL < 0 && match ) {
                idR = dlen;iqR = iq;isR = is;
            }
        }
        if( considerGoodSubalignments == 1 && !oklocal){
            if( matchcounters[0]*100 < minMatchLen*(100-maxMissQueryPercent) ){
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
        if(bw & 0x1)--is;
        if(bw & 0x2)--iq;
    }

    bool okhit=true;

    if(trimLowScoreEnds > 0 ) {
        if(idL < 0 ) {
            dlen = 0;
            okhit = false;
        } else {
            dlen = idR-idL+2;
            maxQ = iqL;
            maxS = isL;
            maxScore = mat(maxS,maxQ) - mat(isR,iqR);
        }
    }

    if( dlen<2*minMatchLen) {
        if( !(flags&fAlignBackward) && maxS+substart+1>=sublen-3 && dlen>=2*allowShorterEnds )
            okhit=true;
        else if( (flags&fAlignBackward) && substart<3 && dlen>=2*allowShorterEnds )
            okhit=true;
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
    if(okhit) {
        idx * m=al->addM(dlen);
        hdr=(Al*)al->ptr(curofs);

        bak(0,0)=3;
        hdr->setDimAlign(dlen);

        idx qq=0,ss=1;
        if(flags&fAlignReverseEngine) {
            qq=1;ss=0;
        }

        for(dlen-=2, is=maxS, iq=maxQ;  dlen>=0 ; dlen-=2) {

            char bw=bak(is,iq);

            m[dlen+qq]= bw==0x02 ? -1 : is;
            m[dlen+ss]= bw==0x01 ? -1 : iq;

            if(bw & 0x1)--is;
            if(bw & 0x2)--iq;
        }
        hdr->setScore(maxScore);
        hdr->setLenAlign(hdr->dimAlign()/2);

        hdr->setDimAlign(compressAlignment(hdr, m, m ));
        al->cut(curofs+sizeof(sBioseqAlignment::Al)/sizeof(idx)+hdr->dimAlign());
    }
    return okhit?hdr:0;
}

idx sBioseqAlignment::remapAlignment(Al * from, Al * to, idx * mfrom0, idx * mto0 ,idx maxlen,idx readonly,sVec<idx> * lookup )
{
    if(!mfrom0)mfrom0=from->match();
    if(!mto0)mto0=to->match();
    if(!maxlen)maxlen=sIdxMax;
    idx cntfrom=2*from->lenAlign(), cntto=2*to->lenAlign();
    idx ofsfrom=from->subStart(), ofsto=to->qryStart();

    idx * mfrom, * mto=mto0+1, *mfromEnd=mfrom0+cntfrom, * mtoEnd=mto0+cntto,cntr=0;
    for( mfrom=mfrom0, mto=mto0+1; mfrom < mfromEnd && mto<mtoEnd && cntr<maxlen; mfrom+=2 , ++cntr) {
        while ( mfrom<mfromEnd &&  (*mfrom)+ofsfrom < (*mto)+ofsto)
          mfrom+=2;
        if(mfrom>=mfromEnd)break;


        if(lookup){
            mto=mto0+( (*(lookup->ptr( (*mfrom)+ofsfrom )) )*2)+1;
            if(mto>=mtoEnd)break;
        }
        while ( mto<mtoEnd && (*mfrom)+ofsfrom > (*mto)+ofsto)
          mto+=2;
        if(mto>=mtoEnd)break;

        if ( (*mfrom)+ofsfrom!=(*mto)+ofsto)
            *(mfrom+1)=-1;
        else {
            *mfrom=*(mto-1);
            if(*mfrom!=-1) {
                *mfrom+=-ofsfrom;
                if(!readonly)from->setLenAlign((mfrom-mfrom0)/2);
            }
        }

        if( *(mfrom+1)!=-1){
            if(!readonly)from->setLenAlign((mfrom-mfrom0+2)/2);
         }

    }

    if(!readonly)from->setIdSub(to->idSub());
    if(!readonly)from->setSubStart(to->subStart()+from->subStart());

    return from->lenAlign();
}

idx sBioseqAlignment::remapSubjectPosition(Al * hdr, idx * to0, idx pos, idx valid)
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
        m_pos =  -1;
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

idx sBioseqAlignment::remapQueryPosition(Al * hdr, idx * to0, idx pos, idx valid)
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
        m_pos =  -1;
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

idx sBioseqAlignment::truncateAlignment(Al * hdr, idx maskBefore, idx maskAfter, idx issuborqry)
{
    idx * m=hdr->match();

    idx subend=hdr->getSubjectEnd(m);
    if ( issuborqry==0)  {
        if(maskAfter<0)maskAfter=subend+maskAfter;

        if( subend<=maskBefore ) return 0;
        if( hdr->subStart()>maskAfter )return 0;
    }

    idx len=hdr->lenAlign(), idst, isrc;
    idx newsubstart=-1, newqrystart=-1;

    for ( isrc=0, idst=0; isrc<len; isrc++ ) {
        if ( issuborqry==0)  {
            if( hdr->getSubjectPosition(m,isrc)<maskBefore ) continue;
            if( hdr->getSubjectPosition(m,isrc)>=maskAfter ) break;
        } else if ( issuborqry==1)  {
            if( hdr->getQueryPositionRaw(m,isrc)<maskBefore ) continue;
            if( hdr->getQueryPositionRaw(m,isrc)>=maskAfter ) break;
        }

        if(newsubstart==-1)
            newsubstart=hdr->getSubjectPosition(m,isrc);
        if(newqrystart==-1)
            newqrystart=hdr->getQueryPositionRaw(m,isrc);
        idst++;

    }
    hdr->setLenAlign(idst);
    hdr->setSubStart(newsubstart);
    hdr->setQryStart(newqrystart);

    return hdr->lenAlign();
}

idx sBioseqAlignment::getLUTMultipleAlignment(sVec < Al * > * alSub,sVec< sVec<idx> > & lutAlSub){
    if(!alSub)return 0;
    if(lutAlSub.dim())lutAlSub.cut(0);
    lutAlSub.add(alSub->dim());
    idx i,iA,it,ip,qS;
    for(iA=0; iA<alSub->dim();++iA){
        Al * hdr=*(alSub->ptr(iA));
        idx * m =hdr->match();qS=hdr->qryStart();
        if(hdr->flags()&sBioseqAlignment::fAlignCompressed){
            m = sBioseqAlignment::uncompressAlignment(hdr,m);
        }
        lutAlSub.ptr(iA)->add(hdr->lenAlign()+qS);
        lutAlSub.ptr(iA)->set(-1);
        it=0;ip=0;
        for( i=0 ; i<hdr->lenAlign(); i++) {
            ip = hdr->getQueryPosition(m,i,0);
            if( ip >= 0 ) {
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
            p1 = sString::next00(p1);
        for( il = ib + ((flags & eAlRelativeToMultiple) ? 0 : 1); p1; ++il, p1 = sString::next00(p1)) {
            if (strncmp("mafft", p1, 5) == 0) {
                return -1;
            }
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
    sBioseqAlignment::Al  * hdr=(sBioseqAlignment::Al  *)(alignmentMap);
    sBioseqAlignment::Al  * hdre=hdr ? sShift(hdr,alignmentMapSize*sizeof(idx)) : hdr ;

    sVec < idx > bestHit(sMex::fSetZero);
    sVec < idx > hitCnt(sMex::fSetZero);

    idx ofsPrv,idQry;
    sBioseqAlignment::Al * hdrFirst = 0;

    for( ; hdr<hdre ; hdr=sShift(hdr,hdr->sizeofFlat())  ) {
        idQry = qryOrSub==0 ? hdr->idQry() : hdr->idSub();

        hitCnt.resizeM(idQry+1);
        bestHit.resizeM(idQry+1);

        if( ++hitCnt[idQry] > 1 && (flags&sBioseqAlignment::fAlignKeepUniqueMatch) ) {
            bestHit[idQry] = 0;
            continue;
        }

        ofsPrv = bestHit[idQry];

        if( ofsPrv ){
            sBioseqAlignment::Al * hdrPrv = (sBioseqAlignment::Al *)((idx*)alignmentMap+(ofsPrv-1));
            if( hdrPrv->score() > hdr->score() )
                continue;
            else if( hdrPrv->score() == hdr->score() ) {
                if(flags&sBioseqAlignment::fAlignKeepAllBestMatches){
                    hdrPrv->setFlagsOn(sBioseqAlignment::fSelectedAlignment);
                }else if(flags&sBioseqAlignment::fAlignKeepRandomBestMatch){
                    if( ((sRand::rand_xorshf96()&0x7FFFFFFF)-1)*hitCnt[idQry]<RAND_MAX )
                        continue;
                }
                else
                    continue;
            }
            else {
                if(flags&sBioseqAlignment::fAlignKeepAllBestMatches){
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
        bestHit[idQry]=(((idx*)hdr)-((idx *)alignmentMap)) + 1 ;
    }

    idx iCnt=0;

    for ( idx iFnd = 0 ; iFnd < bestHit.dim() ;  ++iFnd ){
        idx ofs=bestHit[iFnd];if(!ofs )continue;
        --ofs;

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
        if( m[i]!=m[i-2]+1 || m[i+1]!=m[i-1]+1 ){

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
    mdst[idst++] = m[iprv];
    mdst[idst++] = m[iprv+1];
    if(counter) {
        mdst[idst++]=-counter-1;
    }
    hdr->setFlagsOn(fAlignCompressed);

    return idst;
}

sVec<idx> uncompressAlignment_buffer;

idx * sBioseqAlignment::uncompressAlignment(Al * hdr, idx * m, idx * mdst, bool reverse )
{
    if (unlikely(!hdr || !m)) {
        return 0;
    }

    idx i;
    idx rs=reverse ? 1 : 0 ;
    idx rq=reverse ? 0 : 1 ;

    if( !mdst ) {
        uncompressAlignment_buffer.resize(2*hdr->lenAlign());
        mdst = uncompressAlignment_buffer.ptr();
    }

    if( !hdr->dimAlign() ) {
        for( i = 0 ; i < hdr->lenAlign() ; ++i ) {
            mdst[2*i+rs] = i;
            mdst[2*i+rq] = i;
        }
        return mdst;
    }

    idx idst;
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
    return mdst;
}

idx sBioseqAlignment::reverseAlignment(Al * hdr, idx * m)
{
    idx i, t;

    hdr->ids=sSwapHiLo(hdr->ids);
    hdr->starts=sSwapHiLo(hdr->starts);

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
    MatSW.addM(4*1024*1024);
    MatBR.addM(4*1024*1024);

    costMatch=5;
    costMismatch=-4;
    costMismatchNext=-6;
    costGapOpen=-12;
    costGapNext=-4;
    computeDiagonalWidth=0;
    maxMissQueryPercent=15;
    minMatchLen=11;
    isMinMatchPercentage = false;

    scoreFilter=0;
    allowShorterEnds=minMatchLen;
    maxExtensionGaps=0;
    hashStp=1;
    bioHash.hashStp=1;
    looseExtenderMismatchesPercent=25;
    looseExtenderMinimumLengthPercent=66;

    maxHitsPerRead=20;
    maxSeedSearchQueryPos=512;
    ignoreOverlappingSeedsInSubjectPosInNonPerfectAlignment=0;
    ignoreOverlappingSeedsInQueryPosInNonPerfectAlignment=1;

    bioHash.init(seed,4ll);
    idx isBloom = (seed <= 16) ? sBioseqHash::eCompileBloom  : 0 ;
    if(!isBloom)bioHash.collisionReducer=4;

    {
        idx flagSet=sBioseqAlignment::fAlignForward;
        flagSet|=sBioseqAlignment::fAlignGlobal;
        flagSet|=sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement;
        flagSet|=sBioseqAlignment::fAlignOptimizeDiagonal;
        flagSet|=sBioseqAlignment::fAlignIdentityOnly;
        flagSet|=sBioseqAlignment::fAlignKeepBestFirstMatch;
        *flags = flagSet;
    }
    return true;
}


