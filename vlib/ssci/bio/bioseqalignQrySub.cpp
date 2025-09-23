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
#include <slib/std/file.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ctype.h>

using namespace slib;

#define _costMatch(_v_sbits, _v_qbits) (((_v_sbits) != (_v_qbits)) ? costMismatch : costMatch)
#define QIDX(_v_iq, _v_len)    ((flags&fAlignBackward) ? ((_v_len)-1-(_v_iq)) : (_v_iq))
#define SIDX(_v_is, _v_len)    ( ( (!(flags&fAlignCircular)) || (_v_is)<(_v_len) ) ? (_v_is) : ( (_v_is) -(_v_len) ) )

idx cntHashLookup=0,cntBloomLookup=0,cntHSPHit=0,cntAlignmentSW=0,cntTotLetters=0;

idx sBioseqAlignment::findHSPs(sIndex <HSP > & hspTbl, const char * qry, idx qrylen, idx id, idx flagset, idx * subfos, idx * qryfos)
{
    idx flags=flagset;
    idx hspPara=(bioHash.hdr.lenUnit);

    idx oldHashList[32];

    for ( idx idir=0; idir < 2; ++idir)  {

        idx mask = (idir==0) ? fAlignForward : fAlignBackward;
        if( !(flagset&mask) ) continue;
        flags=( flagset&(~(fAlignForward|fAlignBackward)) ) | mask;

        udx chash=0,chash2=0;

        for( idx iq=0; iq<qrylen; ++iq ) {
            ++cntTotLetters;

            chash <<= 2;
            idx iqx=QIDX(iq,qrylen);
            chash|=_seqBits(qry, iqx , flags);

            if(iq<bioHash.hdr.lenUnit-1)continue;
            chash&=bioHash.hdr.hashMask;

            if( flags&fAlignOptimizeDoubleHash ){
                chash2=oldHashList[iq%bioHash.hdr.lenUnit];
                oldHashList[iq%bioHash.hdr.lenUnit]=chash;
                if(iq<2*bioHash.hdr.lenUnit-1)continue;
            }

            ++cntBloomLookup;
            if( (bioHash.hb [chash/8] & (((udx)1)<<(chash%8))) ==0 )
                continue;
            ++cntHashLookup;
            idx lstdim=bioHash.cnt(chash) ,lstdim2=0;


            sBioseqHash::RefPos * lst=bioHash.lst(chash), * list2=0;

            if( flags&fAlignOptimizeDoubleHash ){
                if( (bioHash.hb [chash2/8] & (((udx)1)<<(chash2%8))) !=0 ){
                    lstdim2=bioHash.cnt(chash2);
                    list2=bioHash.lst(chash2);
                }
                else continue;
            }

            idx qpos=iq-bioHash.hdr.lenUnit+1;

            for (  idx il=0; il<lstdim ; ++il ) {
                sBioseqHash::RefPos & ref=lst[il];
                idx refid=ref.id();
                idx refpos=ref.pos();
                if(id!=sNotIdx && refid!=id) continue;

                idx matchShape=0;
                if(subfos ){
                    idx * thissubfos=sBioseqHash::fos(subfos, refid );
                    idx pos=(refpos-1)/500+1;
                        idx sfos=thissubfos[pos];
                        idx qfos=qryfos[1];
                        if(idir)qfos>>=24;
                            matchShape=sfos;
                }

                if( flags&fAlignOptimizeDoubleHash ) {
                    idx i2;
                    for( i2=0; i2<lstdim2 ; ++i2){
                        if( list2[i2]._pos == ref._pos-(bioHash.hdr.lenUnit) )break;
                    }
                    if(i2==lstdim2) continue;
                }

                ++cntHSPHit;
                HSP hsp;
                hsp.subPosExact=refpos;
                hsp.qryPosExact=qpos;
                hsp.refid=refid;
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






idx sBioseqAlignment::alignSmithWaterman( sVec < idx > * al, const char * sub, idx substart, idx sublen, const char * qry, idx qrystart, idx qrylen, idx flags , idx subbuflen, idx qrybuflen)
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

    al->resizeM(sizeof(Al)/sizeof(idx));
    Al * hdr=(Al*)al->ptr();
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

        if(!bw) break;
        if(bw & 0x1)--is;
        if(bw & 0x2)--iq;
    }

    idx * m=al->addM(dlen);
    hdr=(Al*)al->ptr();

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



    return 1;
    #undef mat
}

















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

    idx isInsDel=0;
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





idx sBioseqAlignment::viewAlignment(sStr * dst, Al * hdr, idx * m, const char * sub, const char * qry, idx subbuflen, idx qrybuflen, const char * prefix, const char * id, const char * qua)
{
    idx i, is , iq ;

    idx flags=hdr->flags;
    idx nonCompFlags=((hdr->flags)&(~(fAlignBackwardComplement|fAlignForwardComplement)));

    sStr l1, l2, l3;
    sStr l4;

    if(id){
        dst->printf("%s[%"_d_"]<->Query[%"_d_"] %s: score=%"_d_" matches=%"_d_" miss=%"_d_" ins=%"_d_" del=%"_d_"\n",
            hdr->idSub>0 ? "Amplicon" :"Master", hdr->idSub,hdr->idQry, id,
            hdr->score,hdr->matches, hdr->mismatches, hdr->inserts, hdr->deletions );
    }

    if(prefix) {
        l1.printf("%s",prefix);
        l2.printf("%s",prefix);
        l3.printf("%s",prefix);
        if(qua) {
            l4.printf("%s",prefix);
        }
    }

    l1.printf("subject[%7"_d_"] %7"_d_"-%7"_d_": ",hdr->idSub,hdr->subStart+m[0]+1,hdr->subEnd+1);
    l2.printf("                                : ");
    if( hdr->flags&fAlignBackward ) l3.printf("query[%7"_d_"](-)%7"_d_"-%7"_d_": ", hdr->idQry,hdr->qryEnd+1,hdr->qryStart+m[1]+1);
    else l3.printf("query[%7"_d_"](+)%7"_d_"-%7"_d_": ", hdr->idQry,hdr->qryStart+m[1]+1,hdr->qryEnd+1);
    if(qua){
        l4.printf("                                : ");
    }

    for( i=0 ; i<hdr->lenAlign; i+=2) {
        is=m[i];
        iq=m[i+1];

        idx isx=SIDX( (hdr->subStart+is) , subbuflen);
        char chS=sBioseq::mapRevATGC[(idx)_seqBits(sub, isx,nonCompFlags)];
        idx iqx=QIDX( hdr->qryStart+iq, qrybuflen );
        char chQ=sBioseq::mapRevATGC[(idx)_seqBits(qry, iqx, hdr->flags)];

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

        if(qua) {

            if(qua[iqx/8]&(((idx)1)<<(iqx%8)))l4.printf("+");
            else l4.printf("-");
        }
    }

    dst->printf("%s\n%s\n%s\n",l1.ptr(),l2.ptr(),l3.ptr());
    if(qua)dst->printf("%s\n",l4.ptr());

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

    idx i, is, iq;
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

    return i;
}



