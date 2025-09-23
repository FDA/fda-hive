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
#include <ssci/math.hpp>
#include <ssci/bio/bioseq.hpp>

using namespace slib;


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




idx sBioseqHash::compile(idx refseq, const char * seq, idx lenseq, idx flags, idx maxBin, idx startcompile, idx lencompile)
{
PERF_START("Init");
    if( (flags&eCompileOnlyBloom)==0 && (flags&eCompileDic)==0 ){
        if(hashTbl.dim()!=hdr.lenTbl){
            hashTbl.mex()->flags=sMex::fBlockCompact;
            hashTbl.empty();
            hashTbl.add(hdr.lenTbl);
            for(idx i=0; i<hashTbl.dim(); ++i) {
                hashTbl[i].mex()->flags=sMex::fBlockCompact;
                hashTbl[i]._mex=&lstMex;
            }
        }else if(flags&eCompileReset || forceReset ){
            for(idx i=0; i<hashTbl.dim(); ++i)
                hashTbl[i].cut(0);
        }
    }
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
    forceReset=1;
PERF_END();


    idx cnt=0;
    udx hash=0;
    
    if(lencompile==0)lencompile=lenseq;

    for( idx is=startcompile; is+hdr.lenUnit<startcompile+lencompile ; ++is) {
            
        hash<<= ( 2 );
    
        idx isS=(flags&eCompileReverse) ? lenseq-1-is : is ;
        idx ibyte=isS/4;
        idx ishift=(isS%4)*2;

        idx val=(idx)((seq[ibyte]>>ishift)&0x3) ;
        
        if(flags&eCompileReverse)
            val=sBioseq::mapComplementATGC[val] ;

        hash|=val;
        if(is<hdr.lenUnit-1)continue;
        hash&=hdr.hashMask;

PERF_START("Bloom");

        if(flags&eCompileBloom)
            hb[hash/8]|=((udx)1<<(hash%8));
        if( flags&eCompileOnlyBloom)
            continue;
PERF_NEXT("DIC");

        HashBinType * lst;
        if( (flags&eCompileDic) ) {
            idx iref=hashDic.add(hash,0);
            lst=&(hashDic[iref].list);
            lst->_mex=&lstMex;
        }
        else lst=&(hashTbl[hash]);
PERF_END();

        if(maxBin!=0 && lst->dim()>=maxBin){
            ++cnt;
            continue;
        }
        if( (flags&eCompileNoList) && lst->dim() ) {
            ++cnt;
            continue;
        }
PERF_START("BINADD");
        RefPos * rp=lst->add();
        
        rp->_pos=is-hdr.lenUnit+1;
        #ifdef SLIB64 
            rp->_pos|=( (refseq&0xFFFFFFFF)<<32);
        #else 
            rp->_pos|=( ( (refseq&0xFFF) &0xFFFFFFFF)<<20);
        #endif
        
PERF_END();

        ++cnt;
    }
    return cnt;    
}


idx  sBioseqHash::dumpCompiled (const char * flnm, idx * maxBin, idx * aveBin)
{
    sFile::remove(flnm);
    FILE * fp=strcmp(flnm,"stdout")==0 ? stdout : fopen(flnm,"w");if (!fp) return 0;
    
    idx c=hashTbl.dim(), absO=2*c*sizeof(idx);
    sVec < idx > ofs; ofs.add( c*2 );
    idx cutBin=maxBin ? *maxBin : 0;

    if(maxBin)*maxBin=0;
    if(aveBin)*aveBin=0;
    idx iNotNull=0;

    for  ( idx i=0 ; i < c ; ++i ) {
        idx bins=hashTbl[i].dim();
        if(cutBin && bins>cutBin )bins=0;

        ofs[2*i]=absO;
        ofs[2*i+1]=bins;

        if(maxBin && *maxBin<bins) *maxBin=bins;
        if(aveBin && bins){*aveBin+=bins;++iNotNull;}
        absO+=bins*sizeof(idx);
    }
    if(aveBin && iNotNull )*aveBin/=iNotNull;

    fwrite ( &hdr, sizeof(hdr), 1 , fp );
    fwrite( (void *) ofs.ptr() ,c , 2*sizeof (idx),fp);
    
    for  ( idx i=0 ; i < c ; ++i ) {
        idx bins=hashTbl[i].dim();
        if(cutBin && bins>cutBin )bins=0;
        if(!bins)continue;

        RefPos * ip=hashTbl[i].ptr(0);
        fwrite( (void*)ip, bins, sizeof( RefPos ) , fp);
    }

    if(fp!=stdout)fclose(fp);
    return c;
}

void  sBioseqHash::bloom (void)
{
    hashBloom.resizeM( (hdr.lenTbl+1)/8 +1  );
    hashBloom.set(0);
    hb= hashBloom.ptr();

    for  ( idx i=0 ; i < hdr.lenTbl ; ++i ) {
        idx c=0;
        if(ofs)c=ofs[i*2+1];
        else if(hashTbl.dim() )c=hashTbl[i].dim();
        else { 
            idx idxH=0;
            c=hashDic.find(i,&idxH);
        }

        if( c )
            hb[i/8]|=((udx)1<<(i%8));
    }
}



void sBioseqHash::fossilize(idx , const char * seq, idx lenseq, idx granule, idx fosblock, idx history, idx flags)
{
    sVec < char > Mat(sMex::fSetZero); Mat.resizeM(fosblock*fosblock);
    char * matP=Mat.ptr();

    sVec <idx > * hfVec=hashFossil.add();


    sVec < idx > Vertable; Vertable.resizeM(granule);idx * vertable=Vertable.ptr();

    idx stp=fosblock/2;
    idx start=0;
    *hfVec->add()=0;
    for ( idx ivalid =0 ; start<lenseq ; ++ivalid , start+=stp ){
        Vertable.set(0);

        idx lenfos=(lenseq-start > fosblock ) ? fosblock : lenseq-start ;

        idx bm=dynamicMatch( matP, seq, seq, start, lenfos, start,lenfos , sMin(granule,(idx)sizeof(idx)*8), vertable, history );
        *hfVec->add()=bm;
        if(start+lenfos>=lenseq)break;
    }

    (*hfVec)[0]=hfVec->dim()-1 ;
}


idx sBioseqHash::dynamicMatch( char * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2, idx len2, idx maxDiag, idx * vertable, idx nMereKeep)
{
    #define mat(_v_i, _v_j) matP[len2*(_v_i)+(_v_j)]

    for( idx is1=0; is1<len1; ++is1 ) {

        char let1=((seq1[ ((is1+st1)/4) ]>> (((is1+st1)%4)*2) )&0x3) ;

        for( idx is2=is1; is2<len2 && is2<is1+maxDiag; ++is2 ) {

            char let2=((seq2[ ((is2+st2)/4) ]>> (((is2+st2)%4)*2) )&0x3) ;


            char prv=(is1>0 && is2>0) ? mat(is1-1,is2-1) : 0;
            if(let1==let2) {
                if(prv<maxDiag)
                    mat(is1, is2)=prv+1;
            }


            if(mat(is1,is2)>vertable[is2-is1])
                vertable[is2-is1]=mat(is1,is2);
        }
    }


    idx bmask=0;
    for ( idx is2=1; is2<maxDiag; ++is2) {
        if(vertable[is2]>=nMereKeep)
            bmask|=((idx)1)<<is2;
    }



    for( idx is1=0; is1<len1; ++is1 ) {
        for( idx is2=is1; is2<len2 && is2<is1+maxDiag; ++is2 ) {
            mat(is1,is2)=0;
        }
    }
    return bmask;
    #undef mat
}


idx  sBioseqHash::dumpFossilized (const char * flnm)
{

    sFile::remove(flnm);
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









