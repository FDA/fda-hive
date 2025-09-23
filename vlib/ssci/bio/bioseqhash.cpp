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
#include <math.h>
#include <slib/core.hpp>
#include <slib/std.hpp>

#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqhash.hpp>

using namespace slib;

char * sBioseqHash::dbgH(idx hash, idx len)
{
    static  char buf[64];

    sBioseq::uncompressATGC(buf,(const char *)&hash,0, len);
    return buf;
}


sBioseqHash * sBioseqHash::init(idx llenunit , idx llenalphabet)
{
    hdr.hashMask=0;
    hdr.lenTbl=1;
    hdr.lenAlpha=llenalphabet;
    hdr.lenUnit=llenunit;
        
    if(hdr.lenAlpha==0 || hdr.lenUnit==0)return 0;
    
    for( idx il=0; il<hdr.lenUnit; ++il )
        hdr.lenTbl*=hdr.lenAlpha;
    
    if(hdr.lenAlpha==4) {
        for( idx il=0; il<hdr.lenUnit; ++il ) {
            hdr.hashMask<<=2;
            hdr.hashMask|=0x03;
        }
    }else {
        for( idx il=0; il<hdr.lenUnit; ++il ) {
            hdr.hashMask<<=8;
            hdr.hashMask|=0x0F;
        }
    }


    return this;
}


idx sBioseqHash::initFile(const char * filename, idx allowSave)
{


    sStr t;
    t.printf(0,"%s.list",filename);
    idx s=sFile::size(t);
    if(!allowSave && !s )
        return 0;

    hList.destroy();
    hList.init(t,s ? sMex::fReadonly : sMex::fSetZero);


    hashBloom.destroy();
    #ifdef HASH_SINDEX
        hashInd.destroy();
        hashInd.hashTbl.destroy();
        hashInd.bitness=1;
    #else
        hashTbl.destroy();
        t.printf(0,"%s.hash",filename);
        hashTbl.init(t,sFile::size(t) ? sMex::fReadonly : sMex::fSetZero|sMex::fExactSize);
    #endif


    t.printf(0,"%s.bloom",filename);
    hashBloom.init(t,sFile::size(t) ? sMex::fReadonly : sMex::fSetZero|sMex::fExactSize);
    hb= hashBloom.ptr();
    #ifdef HASH_SINDEX
        t.printf(0,"%s.index",filename);
        hashInd.init(t,sFile::size(t) ? sMex::fReadonly : sMex::fSetZero|sMex::fBlockDoubling);

        t.printf(0,"%s.hashindex",filename);
        hashInd.hashTbl.init(t,sFile::size(t) ? sMex::fReadonly : sMex::fSetZero|sMex::fBlockNormal);
        if(hashInd.hashTbl.dim())
            hashInd.getValiBit();

    #endif
        return hList.dim();
}


idx maxHash=0;
idx sBioseqHash::compile(idx refseq, const char * seq, idx lenseq, idx flags, idx maxBin, idx startcompile, idx lencompile, idx complexityWindow, real complexityShannon, const char * qua, bool quabit, idx acceptNNNQuaTrheshold)
{

    static idx stComplexityWindow=0;
    static idx cplxW[1024];
    idx cplxSh=complexityShannon*1000;

    #ifndef HASH_SINDEX
        {
            if(hashTbl.dim()!=hdr.lenTbl){
                hashTbl.mex()->flags|=sMex::fExactSize|sMex::fSetZero;
                hashTbl.empty();
                hashTbl.addM(hdr.lenTbl);
                for(idx i=0; i<hashTbl.dim(); ++i) {
                    hashTbl[i]=0;
                }
            }else if(flags&eCompileReset || forceReset ){
                hashTbl.set(0);
            }
        }
    #endif


    if(flags&eCompileBloom) { 
        if( hashBloom.dim()!=(hdr.lenTbl+1)/8 +1 ) {
            hashBloom.resizeM( (hdr.lenTbl+1)/8 +1  );
            hashBloom.set(0);
            hb= hashBloom.ptr();
        }
        else if(flags&eCompileReset || forceReset){
            hashBloom.set(0);
        }
    }
    if(forceReset) { 
        hList.cutM(0);
        #ifdef HASH_SINDEX
            hashInd.cutM(0);
            hashInd.reserve(subjectIn, masterAl.bioHash.collisionReducer);
        #endif

        forceReset=0;
    }

    if(hList.dim()==0)
        hList.add(1);
        

    
    idx cnt=0, ik=0;
    udx hash=0;
    if(lencompile==0)lencompile=lenseq;
    
    idx hC=hList.dim();
    hList.addM(lencompile*2);
    HLitem * hL=hList.ptr(0);



    idx atgc[4],newlet=-1,isS=0, oldlet=-2;
    real rEntr=0;
    if(complexityWindow){
        if(stComplexityWindow!=complexityWindow) {
            stComplexityWindow=complexityWindow;
            cplxW[0]=0;cplxW[sDim(cplxW)-1]=0;
            for ( idx i=1; i<sDim(cplxW); ++i){
                real p=((real)i)/complexityWindow;
                cplxW[i]=(idx)(-p*(log(p)*1.442695040888963)*1000);
            }
        }

        atgc[0]=0;atgc[1]=0;atgc[2]=0;atgc[3]=0;

        for( idx is=startcompile; ik<complexityWindow && is<startcompile+lencompile  ; ++is, ++ik) {

            isS=(flags&eCompileReverse) ? lenseq-1-is : is ;
            idx ibyte=isS/4;
            idx ishift=(isS%4)*2;
            idx val=(idx)((seq[ibyte]>>ishift)&0x3) ;

            if(flags&eCompileReverse)
                val=sBioseq::mapComplementATGC[val] ;

            ++atgc[val];
        }
        ik=isS;
    }



    udx rememberedHash=0;
    idx iStp=0, lastUnacceptable=-hdr.lenUnit ;
    for( idx is=startcompile; is<startcompile+lencompile ; ++is) {
        hash=rememberedHash;
        hash<<= ( 2 );
    
        idx isS=(flags&eCompileReverse) ? lenseq-1-is : is ;
        idx ibyte=isS/4;
        idx ishift=(isS%4)*2;

        idx val=(idx)((seq[ibyte]>>ishift)&0x3) ;

        if(flags&eCompileReverse)
            val=sBioseq::mapComplementATGC[val] ;

        if( qua  ){

            if( quabit ) {
                if( acceptNNNQuaTrheshold>0 &&  (qua[isS/8]&(((idx)1)<<(isS%8))) ==1 )
                    lastUnacceptable=isS;
            }else if( (qua[isS]) < acceptNNNQuaTrheshold )
                lastUnacceptable=isS;
        }


        hash|=val;
        if(is<hdr.lenUnit-1){rememberedHash = hash;continue;}
        hash&=hdr.hashMask;
        rememberedHash = hash;
        #ifdef HASH_SINDEX
            hash=(hash<<8)| ((hash>>(2*(hdr.lenUnit-4)))&0xFF);
            hash&=hdr.hashMask;
        #endif



        if(hashStp>1 && iStp<(hashStp-1)){
            ++iStp;
            continue;
        }else iStp=0;


        if( lastUnacceptable > is-hdr.lenUnit ) {
            continue;
        }

        if(complexityWindow){
            if(newlet!=oldlet){
                rEntr=cplxW[atgc[0]]+cplxW[atgc[1]]+cplxW[atgc[2]]+cplxW[atgc[3]];

            }

            idx iprv=0;
            if(flags&eCompileReverse && ik>0) {--ik;iprv=ik+complexityWindow;}
            else if(ik<lenseq-1){++ik;iprv=ik-complexityWindow;}
            newlet=((seq[ ((ik)/4) ]>> (((ik)%4)*2) )&0x3) ;
            oldlet=((seq[ ((iprv)/4) ]>> (((iprv)%4)*2) )&0x3) ;
            if(newlet!=oldlet){
                --atgc[oldlet];
                ++atgc[newlet];
            }
            if(rEntr<cplxSh)
                continue;
        }

        if(flags&eCompileBloom)
            hb[hash>>3]|=(((udx)1)<<(hash&0x7));
        if( flags&eCompileOnlyBloom) {
            continue;
        }

        HLitem * hi, * hi0;

        #ifdef HASH_SINDEX
            idx iref=hashInd.add(hash,0,collisionReducer);
            idx ofs=hashInd[iref].ofs;
        #else 
            idx ofs=hashTbl[hash];
        #endif

        if( ofs == 0 ) {
            #ifdef HASH_SINDEX
                hashInd[iref].ofs=hC;
            #else
                hashTbl[hash]=hC;
            #endif
        
            hi0= hL+ hC ;
            hi0->ref._pos=0; 
            hi0->next=hC;
            ++hC;
        } else {
            hi0=hL + ofs;
        
            if(maxBin!=0 && hi0->ref._pos>=maxBin){
                ++cnt;
                continue;
            }
        }

        hi=hL+ hi0->next;

        hi0->ref._pos++;
        hi0->next=hC;
        
        hi->next=hC;
        hi=hL+hC;
        hi->next=0 ;
        ++hC;

        if(hi0->ref._pos>maxHash)
            maxHash=hi0->ref._pos;

        hi->ref._pos=is-hdr.lenUnit+1;
        #ifdef SLIB64 
            hi->ref._pos|=( (refseq&0xFFFFFFFF)<<32);
        #else 
            hi->ref._pos|=( ( (refseq&0xFFF) &0xFFFFFFFF)<<20);
        #endif
        
        
        ++cnt;

    }

    hList.cut(hC);

    return cnt;    
}



idx  sBioseqHash::dumpCompiled(const char * flnm, idx * maxBin, idx * aveBin, idx * totBin)
{
    return 0;
}

void  sBioseqHash::bloom (void)
{
}

#define _costMatch(_v_sbits, _v_qbits) (((_v_sbits) != (_v_qbits)) ? costMismatch : costMatch)

idx sBioseqHash::dynamicMatch( short int * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2, idx len2, idx maxDiag, idx nMereKeep, const short int * costs, idx lenseq)
{

PERF_START("NULL");
    memset(matP,0,maxDiag*3*sizeof(short int));

PERF_NEXT("DYNAMIC");

    DEBUG_START(1)
        ::printf("    ");
    DEBUG_END()
    for( idx is1=1; is1<maxDiag; ++is1 ) {
        DEBUG_START(1)
                ::printf("   %2d  |", (int)is1 );
        DEBUG_END()
    }

    DEBUG_START(1)
        ::printf("\n");
    DEBUG_END()

    for( idx is1=0; is1<len1; ++is1 ) {

        idx ilet1=is1+st1;
        if(lenseq ) lenseq=lenseq-1-ilet1;
        char let1=((seq1[ ((ilet1)/4) ]>> (((ilet1)%4)*2) )&0x3) ;
        
        for( idx is2=1; is2<maxDiag && is2+is1<len2; ++is2 ) {
            short int * vt=matP+is2;
            short int * st=vt+maxDiag;
            short int * bt=st+maxDiag;

            idx ilet2=is2+is1+st1;
            if(lenseq ) lenseq=lenseq-1-ilet2;
            char let2=((seq2[ ((ilet2)/4) ]>> (((ilet2)%4)*2) )&0x3) ;

            *vt+=(let1==let2 ? costs[0] : costs[1] );

            if(*vt<=0){
                *vt=0;
                *st=0;
            } else {
                if(*st==0)
                    *st=(short int)is1;
                if( is1-*st+1 > *bt )
                    *bt=is1-*st+1;
            }

            DEBUG_START(1)
                if(is2==1)::printf("%3d>",(int)is1);
                if(*vt<=0)::printf("   %c%c  |",sBioseq::mapRevATGC[(idx)let1],sBioseq::mapRevATGC[(idx)let2]);
                else ::printf(" %2x%c%c%2d|", (int)*vt, sBioseq::mapRevATGC[(idx)let1],sBioseq::mapRevATGC[(idx)let2],(int)(is1-*st+1) );
            DEBUG_END()
        }
        DEBUG_START(1)
            ::printf("\n");
        DEBUG_END()
    }

PERF_NEXT("MASK");
    idx bmask=0;
    short int * bt=matP+2*maxDiag;

    for ( idx is2=1; is2<maxDiag; ++is2) {
        if(bt[is2]>=nMereKeep)
            bmask|=((idx)1)<<is2;
    }

    DEBUG_START(1)
        ::printf("    ");
        for ( idx is2=1; is2<maxDiag; ++is2)
            ::printf("     %d |", (bt[is2]>=nMereKeep) ? 1 : 0 );
        ::printf("\n    ");
        for ( idx is2=1; is2<maxDiag; ++is2)
            ::printf(" %2d    |",(int)bt[is2]);
        ::printf("\n    ");
        for ( idx is2=1; is2<maxDiag; ++is2)
            ::printf("%d", (bt[is2]>=nMereKeep) ? 1 : 0 );
        ::printf("\n    %" HEX "\n\n\n",bmask);
    DEBUG_END()

PERF_END();


    return bmask;
    #undef mat
    
}


idx  sBioseqHash::dumpFossilized (const char * flnm)
{

    FILE * fp=strcmp(flnm,"stdout")==0 ? stdout : fopen(flnm,"w");if (!fp) return 0;

    idx c= hashFossil.dim();
    sVec < idx > Ofs; idx * ofs=Ofs.add(1+c), absO=(c+1);
    ofs[0]=0;
    for (idx i=0; i<c; ++i ){
        sVec < idx > & hF=hashFossil[i];
        idx hCnt=hF[0];

        ofs[1+i]=absO;
        absO+=1+hCnt;
    }
    ofs[0]=c;

    fwrite( (void*)ofs, 1+c, sizeof(idx) , fp);

    for (idx i=0; i<c; ++i ){
        sVec < idx > & hF=hashFossil[i];
        idx hCnt=hF[0];
        fwrite( (void*)hF.ptr(), 1+hCnt, sizeof( idx ) , fp);


    }


    fclose(fp);
    return c;
}






















idx sBioseqHash::longestDynamicMatch( short int * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2, idx len2, idx maxDiag, idx lenseq,  idx * matchPos )
{

    memset(matP,0,maxDiag*3*sizeof(short int));

    for( idx is1=0; is1<len1; ++is1 ) {

        idx ilet1=is1+st1;
        if(lenseq ) lenseq=lenseq-1-ilet1;
        char let1=((seq1[ ((ilet1)/4) ]>> (((ilet1)%4)*2) )&0x3) ;

        for( idx is2=0; is2<maxDiag && is2<len2; ++is2 ) {
            short int * vt=matP+is2;
            short int * st=vt+maxDiag;
            short int * bt=st+maxDiag;

            idx ilet2=is2+st2;
            if(lenseq ) lenseq=lenseq-1-ilet2;
            char let2=((seq2[ ((ilet2)/4) ]>> (((ilet2)%4)*2) )&0x3) ;

            if(let1==let2) ++(*vt);
            else (*vt)=0;

            if(*vt<=0){
                *vt=0;
                *st=0;
            } else {
                if(*st==0)
                    *st=(short int)is1;
                if( is1-*st+1 > *bt )
                    *bt=is1-*st+1;
            }
        }
    }

    short int * bt=matP+2*maxDiag;
    short int * st=matP+maxDiag;
    idx maxMatch=0;
    for ( idx is2=1; is2<maxDiag; ++is2) {
        if(bt[is2]>=maxMatch){
            maxMatch=bt[is2];
            if(matchPos)*matchPos=st[is2];
        }
    }



    return maxMatch;
}


idx sBioseqHash::compileSuffix(idx refseq, const char * seq, idx lenseq, idx flags, idx maxBin, idx startcompile, idx lencompile, idx complexityWindow, real complexityShannon)
{
    suff_lst._mex=&suff;
    suff_lst.resize( ((idx)1)<<(2*hdr.lenUnit));

    idx hash, rememberedHash = 0, iStp = 0;
    for( idx is=startcompile; is<startcompile+lencompile ; ++is) {
        hash=rememberedHash;
        hash<<= ( 2 );

        idx isS=(flags&eCompileReverse) ? lenseq-1-is : is ;
        idx ibyte=isS/4;
        idx ishift=(isS%4)*2;
        idx val=(idx)((seq[ibyte]>>ishift)&0x3) ;
        if(flags&eCompileReverse)
            val=sBioseq::mapComplementATGC[val] ;


        hash|=val;
        if(is<hdr.lenUnit-1){rememberedHash = hash;continue;}

        hash&=hdr.hashMask;
        rememberedHash = hash;

        if(hashStp>1 && iStp<(hashStp-1)){
            ++iStp;
            continue;
        }else iStp=0;

        suff_lst[hash]=is;

    }
    return 0;
}
