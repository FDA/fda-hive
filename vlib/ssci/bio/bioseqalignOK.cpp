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
#define SIDX(_v_is, _v_len)    ( (_v_is) )

idx cntHashLookup=0,cntBloomLookup=0,cntAlignmentSW=0,cntTotLetters=0,  cntCrossPass=0;
idx maxHashBinFound=0;


idx sBioseqAlignment::zeroAlignment[2]={0,0};
idx sBioseqAlignment::alignSeq(sVec < idx > * allHits , sBioseq * Subs, const char * qry, idx qrylen, idx idSub, idx idQry, idx flagset, idx * , idx * )
{
    if(!computeDiagonalWidth)computeDiagonalWidth=bioHash.hdr.lenUnit/2;
    idx flags;


    SubScanQPos.resizeM(Subs->dim());
    idx * subScanQPos=SubScanQPos.ptr();
    idx cntIn=0;

    for ( idx idir=0; idir < 2; ++idir)  {
        
        

PERF_START("PREP");
        
        idx leftToDo=0;
        for( idx iFnd=0; iFnd<SubScanQPos.dim(); ++ iFnd) {
            if( (flagset&fAlignKeepFirstMatch) && subScanQPos[iFnd]==sIdxMax )continue;
            subScanQPos[iFnd]=0;
            ++leftToDo;
        }
        if(flagset&fAlignKeepFirstMatch && leftToDo==0)
            continue;
PERF_END();
        


        idx mask = (idir==0) ? fAlignForward : fAlignBackward;
        if( !(flagset&mask) ) continue;
        flags=( flagset&(~(fAlignForward|fAlignBackward)) ) | mask;

        udx chash=0;
                
        for( idx iq=0; iq<qrylen; ++iq ) {

            if(iq && callbackFunc && (iq%100000)==0 ){
                if( callbackFunc(callbackParam, cntIn, iq*50/qrylen )==1 )return cntIn;
            }
            ++cntTotLetters;
            

            chash <<= 2;
            idx iqx=QIDX(iq,qrylen);
            char let=_seqBits(qry, iqx , flags);
            chash|=let;
            chash&=bioHash.hdr.hashMask;
            if(iq<bioHash.hdr.lenUnit-1)continue;
            
            ++cntBloomLookup;
            if( (bioHash.hb [chash/8] & (((udx)1)<<(chash%8))) ==0 ){
                continue;
            }

            ++cntHashLookup;
            sBioseqHash::HashBinType * lst=bioHash.lst(chash);
            idx lstdim=bioHash.cnt(chash);

            if(maxHashBinFound<lstdim)maxHashBinFound=lstdim;

            idx qrypos=iq-bioHash.hdr.lenUnit+1;
            for (  idx il=0; il<lstdim ; ++il ) {
                
                sBioseqHash::RefPos & ref=(*lst)[il];
                idx refid=ref.id();
                
                if(idSub!=sNotIdx && refid!=idSub) continue;
                if(qrypos<subScanQPos[refid])continue;
                
                idx subpos=ref.pos();
                
PERF_START("LOAD");

                const char * sub=Subs->seq(refid);
                idx sublen=Subs->len(refid);
PERF_END();


PERF_START("EXTEND");

                idx matchesAll=0, inShift=0;
                iqx=QIDX(qrypos,qrylen);
                char prvSubLet=_seqBits(sub, subpos , 0), prvQryLet=_seqBits(qry, iqx , flags);

                idx isub,iqry, iqryfirstnonmatch=qrypos+bioHash.hdr.lenUnit;
                for ( idx ext = -1; ext<2 ; ext+=2 ) {
                    
                    if(ext<0){isub=subpos-1;iqry=qrypos-1;}
                    else {isub=subpos+bioHash.hdr.lenUnit;iqry=qrypos+bioHash.hdr.lenUnit;}
                    idx diffs=0 ,gaps=0 , matches =bioHash.hdr.lenUnit, isubshift=0, iqryshift=0 ;

                    while( gaps<=(maxExtensionGaps+1) && diffs*100 < looseExtenderMismatchesPercent*(matches +diffs) ) {
                                                                        
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
                                if( idxNextLet>=0 && idxNextLet<qrylen )
                                    nextLet = _seqBits(qry, QIDX(( idxNextLet ) ,qrylen)  , flags);
                                if ( subLet == nextLet ) {
                                    iqry=idxNextLet;
                                    qryLet=nextLet;
                                    ++iqryshift;
                                }
                            }
                            if( subLet!=nextLet && isubshift < maxExtensionGaps  ){
                                nextLet=0x0F;
                                idxNextLet=isub+ext;
                                if( idxNextLet>=0 && idxNextLet<sublen )
                                    nextLet = _seqBits(sub, idxNextLet , 0);
                                if ( qryLet == nextLet ) {
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
                subScanQPos[refid]= iqryfirstnonmatch;
                
PERF_END();

                    
                if( matchesAll < minMatchLen ){
                    continue ;
                }
                
                idx curofs=allHits->dim();
PERF_START("IDENT");
                if(flags&fAlignIdentityOnly){
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
                    subScanQPos[refid] = (flagset&fAlignKeepFirstMatch) ? sIdxMax: iqry;
                    continue;
                }
PERF_END();

PERF_START("SMWAT");


                idx lenBeforeNmerMatch=sMin ( qrypos, subpos) ;
                idx qryStart=qrypos-lenBeforeNmerMatch;
                idx subStart=subpos-lenBeforeNmerMatch;
                idx qryLenAlign= computeDiagonalWidth+sMin ( (sublen-subStart) , (qrylen-qryStart) );
                idx subLenAlign = qryLenAlign;
                if( qryLenAlign>qrylen)qryLenAlign=qrylen;
                if( subLenAlign>sublen)subLenAlign=sublen;

                ++cntAlignmentSW;
                idx  matchcounters[4]; sSet(matchcounters,0,sizeof(idx)*4);
                idx okhit = alignSmithWaterman(allHits,sub, subStart, subLenAlign, qry, qryStart, qryLenAlign,  flags , sublen, qrylen , matchcounters);
                Al * hdr=(Al*)allHits->ptr(curofs);
                subScanQPos[refid] = hdr->qryEnd+1;
                
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



idx sBioseqAlignment::alignSmithWaterman( sVec < idx > * al, const char * sub, idx substart, idx sublen, const char * qry, idx qrystart, idx qrylen, idx flags , idx , idx qrybuflen, idx  * matchcounters)
{

    if(!qrybuflen)qrybuflen=qrylen;


    idx reserve=1024*1024;
    idx maxSeq=qrylen*2+computeDiagonalWidth;
    idx sz=(maxSeq+2)*(qrylen+1);if(sz<reserve)sz=reserve;
    MatSW.resizeM(sz*2);
    short int * matP=(short int *)MatSW.ptr();
    sz=(maxSeq+1)*qrylen;if(sz<reserve)sz=reserve;
    MatBR.resizeM(sz);
    char * matB=MatBR.ptr();

    #define mat(_v_i, _v_j) matP[(qrylen+1)*(_v_i)+(_v_j)]
    #define bak(_v_i, _v_j) matB[(qrylen)*(_v_i)+(_v_j)]

    idx fv,cv=0;
    idx maxAll=0, maxAllLastLetter=0;
    idx maxS=0, maxQ=0, maxRowLastLetter=0, maxColLastLetter=0;

    idx qStart=0, qEnd=(flags&fAlignOptimizeDiagonal ) ? computeDiagonalWidth : qrylen;


    idx is, iq;
    idx nonCompFlags=(flags&(~(fAlignBackwardComplement|fAlignForwardComplement)));


    for(  is=0; is<sublen; ++is ) {

        idx isx=SIDX( (substart+is) , subbuflen);
        #ifdef sBIO_ATGC_SEQ_2BIT
            idx sBits=_seqBits(sub, isx, nonCompFlags );
        #else
            idx sBits=sub[isx];
        #endif

        idx maxRow=0;
        idx maxRowPos=0;

        for( iq=qStart; iq<qEnd; ++iq ) {

            idx iqx=QIDX((qrystart+iq),qrybuflen);

            idx qBits=_seqBits(qry, iqx, flags)  ;


            cv=mat(is+1,iq+1);

            fv=mat(is,iq)+_costMatch( sBits , qBits   );
            if( cv<=fv ) {cv=fv; bak(is,iq)=3; };
            fv=mat(is,iq+1)+ ( (is==0 || bak(is-1,iq)==3) ? costGapOpen : costGapNext);
            if( cv<fv ) {cv=fv; bak(is,iq)=1; }
            fv=mat(is+1,iq)+( (iq==0 || bak(is,iq-1)==3) ? costGapOpen : costGapNext);
            if( cv<fv ) {cv=fv; bak(is,iq)=2;};

            if(cv>0){
                mat(is+1,iq+1)=(short int)cv;

                if(cv>=maxRow){
                    maxRow=cv;
                    maxRowPos=iq;
                }
            }
        }

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
            qStart=is-computeDiagonalWidth;
            if(qStart<0)qStart=0;
            qEnd=is+computeDiagonalWidth;
            if(qEnd>qrylen)qEnd=qrylen;
        }


        if(qStart>=qrylen-1)break;

    }

    idx curofs=al->dim();
    al->addM(sizeof(Al)/sizeof(idx));
    Al * hdr=(Al*)al->ptr(curofs);
    sSet(hdr);

    hdr->flags=flags;

    if(flags&fAlignGlobal) {
        maxAll=maxAllLastLetter;
        maxS=maxRowLastLetter;
        maxQ=maxColLastLetter;
        hdr->score=maxAllLastLetter;
    }else
        hdr->score=maxAll;


    hdr->dimAlign=0;
    hdr->subStart=substart;
    hdr->qryStart=qrystart;
    hdr->subEnd=maxS+hdr->subStart;
    hdr->qryEnd=maxQ+hdr->qryStart;

    idx dlen;
    for(dlen=0, is=maxS, iq=maxQ, cv=maxAll;  ; cv=mat(is+1,iq+1) , dlen+=2) {
        if(flags&fAlignGlobal){ if(iq<0 || is<0) break;}
        else if(cv<=0) break;

        char bw=bak(is,iq);
        if( (!(flags&fAlignMaxExtendTail)) && cv<costMatch ){ break;}

        if(bw==0x02)++matchcounters[2];
        else if(bw==0x01)++matchcounters[3];
        else if(cv<mat(is,iq))++matchcounters[1];
        else ++matchcounters[0];

        if(!bw) break;
        if(bw & 0x1)--is;
        if(bw & 0x2)--iq;
    }
    
    bool okhit=true;
    if( dlen<2*minMatchLen) okhit=false;
    else if(scoreFilter && hdr->score<scoreFilter ) okhit=false;
    else if( matchcounters[0]*200 < dlen*(100-maxMissQueryPercent)  ) okhit=false;
    if(!okhit) {
    } else {  

        idx * m=al->addM(dlen);
        hdr=(Al*)al->ptr(curofs);

        bak(0,0)=3;
        hdr->dimAlign=dlen;


        for(dlen-=2, is=maxS, iq=maxQ, cv=maxAll;  dlen>=0 ; cv=mat(is+1,iq+1) , dlen-=2) {

            char bw=bak(is,iq);

            m[dlen]= bw==0x02 ? -1 : is;
            m[dlen+1]= bw==0x01 ? -1 : iq;

            if(bw & 0x1)--is;
            if(bw & 0x2)--iq;
        }
        hdr->lenAlign=hdr->dimAlign;

        hdr->dimAlign=compressAlignment(hdr, m, m );
        al->cut(curofs+sizeof(sBioseqAlignment::Al)/sizeof(idx)+hdr->dimAlign);
    }



    qStart=0;
    qEnd=(flags&fAlignOptimizeDiagonal ) ? computeDiagonalWidth : qrylen;
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



    return okhit ? 1 : 0 ;
    #undef mat
}





















idx sBioseqAlignment::viewAlignment(sStr * dst, Al * hdr, idx * m, const char * sub, const char * qry, idx , idx qrybuflen, const char * prefix, const char * id, const char * subqua, const char * qryqua)
{
    sVec < idx > uncompressMM;
    if(hdr->flags&fAlignCompressed){
        uncompressMM.resize(hdr->lenAlign);
        sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
        m=uncompressMM.ptr();
    }

    idx i, is , iq ;

    idx flags=hdr->flags;
    idx nonCompFlags=((hdr->flags)&(~(fAlignBackwardComplement|fAlignForwardComplement)));
    
    sStr l1, l2, l3, l3comp;
    sStr l4,l5;

    if(id){
        dst->printf("%s[%"_d_"]<->Query[%"_d_"] %s: score=%"_d_"\n",
            hdr->idSub>0 ? "Amplicon" :"Master", hdr->idSub,hdr->idQry, id,
            hdr->score);
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
    for( mfrom=mfrom0, mto=mto0+1; mfrom < mfromEnd && mto<mtoEnd; mfrom+=2 ) {

        while ( mfrom<mfromEnd &&  (*mfrom)+ofsfrom < (*mto)+ofsto)
            mfrom+=2;
        if(mfrom>=mfromEnd)break;
        while ( mto<mtoEnd && (*mfrom)+ofsfrom > (*mto)+ofsto)
            mto+=2;
        if(mto>=mtoEnd)break;

        if ( (*mfrom)+ofsfrom!=(*mto)+ofsto)
            *(mfrom+1)=-1;
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

    from->idSub=to->idSub;
    from->subStart=to->subStart+from->subStart;
    from->subEnd+=from->subStart;
    from->qryEnd+=from->qryStart;

    return from->lenAlign;
}

idx sBioseqAlignment::truncateAlignment(Al * hdr, idx maskBefore, idx maskAfter, idx issuborqry)
{
    idx * m=hdr->match();


    if ( issuborqry==0)  {
        if(maskAfter<0)maskAfter=hdr->subEnd+maskAfter;

        if( hdr->subEnd<=maskBefore ) return 0;
        if( hdr->subStart>maskAfter )return 0;
    }

    idx len=hdr->lenAlign, idst, isrc;
    idx newsubstart=-1, newqrystart=-1;

    for ( isrc=0, idst=0; isrc<len; isrc+=2 ) {
        if ( issuborqry==0)  {
            if( m[isrc]+hdr->subStart<maskBefore ) continue;
            if( m[isrc]+hdr->subStart>=maskAfter ) break;
        } else if ( issuborqry==1)  {
            if( m[isrc+1]+hdr->qryStart<maskBefore ) continue;
            if( m[isrc+1]+hdr->qryStart>=maskAfter ) break;
        }


        if(newsubstart==-1)
            newsubstart=hdr->subStart+m[isrc];
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

    return hdr->lenAlign;
}








idx sBioseqAlignment::countSNPSingleSeq(idx * freq, idx substart, idx sublen, const char * qrybits, idx qrybuflen,const char * qua, Al * hdr, idx * m, idx maxRptIns)
{
    if( hdr->subEnd < substart || hdr->subStart+m[0] > substart+sublen)
        return 0;

    idx i, is, iq,cntMapped=0;
    idx ddd=SNPtableColSize;

    idx lastSubOK=0, ch, flags=hdr->flags;
    char let=0;

    for( i=0 ; i<hdr->lenAlign; i+=2) {

        is=m[i];
        iq=m[i+1];

        if(qua) {
            if( (qua[i/8]&(((idx)1)<<(i%8))) ==0 )
                continue;
        }

        if(is<0){ ch=4; is=lastSubOK;}
        else if(iq<0)ch=5;
        else {
            idx iqx=QIDX( hdr->qryStart+iq, qrybuflen );
            ch=let=_seqBits(qrybits, iqx , hdr->flags) ;
        }

        if( is +hdr->subStart < substart)continue;
        else if( is+hdr->subStart >= sublen+substart)break;

        if(maxRptIns && ch==4 && hdr->qryStart+iq>maxRptIns) {
            idx irp, iqx;
            char qlet, prvlet=-1;
            for ( irp=0 ; irp<=maxRptIns; ++irp ){
                iqx=QIDX( hdr->qryStart+iq-irp, qrybuflen );
                qlet=_seqBits(qrybits, iqx , hdr->flags);
                if( irp>0 && prvlet!=qlet)break ;
                prvlet=qlet;
            }
            if(irp>maxRptIns)
                continue;
        }

        ++cntMapped;
        if( (hdr->flags&fAlignCircular) && is>=sublen)is-=sublen;

        idx line=ddd*(hdr->subStart+is-substart);

        if(hdr->flags&fAlignBackward) ch+=ddd/2;
        ++(freq)[line+ch];

        if(qua && freq){
            idx iQua=6; if (hdr->flags&fAlignBackward) iQua=13 ;
            (freq)[line+iQua]+=( (qua[i/8]&(((idx)1)<<(i%8))) ==0 ) ? 40 : 10 ;
        }

        lastSubOK=is;

    }

    return cntMapped;
}























