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

//sPerf gPerf;
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


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  private utilities
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx maxHash=0;
idx sBioseqHash::compile(idx refseq, const char * seq, idx lenseq, idx flags, idx maxBin, idx startcompile, idx lencompile, idx complexityWindow, real complexityShannon, const char * qua, bool quabit, idx acceptNNNQuaTrheshold)
{
    ////PERF_START("HASH_PREPARATION");

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
            hashInd.cutM(0);// empty();
            hashInd.reserve(subjectIn, masterAl.bioHash.collisionReducer);
        #endif

        forceReset=0;
    }

    if(hList.dim()==0)
        hList.add(1);// we add a single element so the offset of the next ones is never zero
        

    
    idx cnt=0, ik=0;
    udx hash=0; // this is unsigned due to rotations sign digit overload
    if(lencompile==0)lencompile=lenseq;
    
    idx hC=hList.dim();
    hList.addM(lencompile*2); // a space for new hash items 
    HLitem * hL=hList.ptr(0);

    ////PERF_END();

    //sIndex < HLRefPos > tttInd;

    // compute ahead the complexity of the upcoming window
    idx atgc[4],newlet=-1,isS=0, oldlet=-2;
    real rEntr=0;
    if(complexityWindow){
        ////PERF_START("HASH_CPLX");
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
            // get the two bits necessary
            idx ibyte=isS/4;
            idx ishift=(isS%4)*2; // determine the byte number and the shift count
            idx val=(idx)((seq[ibyte]>>ishift)&0x3) ; // this particular base introduces this two bits

            if(flags&eCompileReverse)
                val=sBioseq::mapComplementATGC[val] ;

            ++atgc[val];
        }
        ik=isS;
        ////PERF_END();
    }

    ////PERF_START("HASH_COMPL");


    udx rememberedHash=0;
    idx iStp=0, lastUnacceptable=-hdr.lenUnit ;
    //for( idx is=startcompile; is+hdr.lenUnit<startcompile+lencompile ; ++is) {
    for( idx is=startcompile; is<startcompile+lencompile ; ++is) {
        hash=rememberedHash;
        //     prepare hash for this position    
        hash<<= ( 2 );
    
        idx isS=(flags&eCompileReverse) ? lenseq-1-is : is ;
        // get the two bits necessary 
        idx ibyte=isS/4;
        idx ishift=(isS%4)*2; // determine the byte number and the shift count

        idx val=(idx)((seq[ibyte]>>ishift)&0x3) ; // this particular base introduces this two bits

        if(flags&eCompileReverse)
            val=sBioseq::mapComplementATGC[val] ;

        if( qua  ){

            if( quabit ) {
                if( acceptNNNQuaTrheshold>0 &&  (qua[isS/8]&(((idx)1)<<(isS%8))) ==1 )
                    lastUnacceptable=isS;
            }else if( (qua[isS]) < acceptNNNQuaTrheshold )
                lastUnacceptable=isS;
        }


        hash|=val; //_seqBits(seq, is, flags );
        if(is<hdr.lenUnit-1){rememberedHash = hash;continue;} // we still didn't accumulate enough letters
        //if(is<24-1)continue; // we still didn't accumulate enough letters
        hash&=hdr.hashMask;
        rememberedHash = hash;
        #ifdef HASH_SINDEX
            hash=(hash<<8)| ((hash>>(2*(hdr.lenUnit-4)))&0xFF);
            hash&=hdr.hashMask;
        #endif



        if(hashStp>1 && iStp<(hashStp-1)){
            ++iStp;
            continue;//{PERF_END();continue;}
        }else iStp=0;


        if( lastUnacceptable > is-hdr.lenUnit ) {
            continue;
        }

        if(complexityWindow){
            ////PERF_START("HASH_CPLX");
            if(newlet!=oldlet){ // exactly because for low complexity regions we will have a lot of repeat letters .. .we do not need to recompute if old letter and new letter are the same
                rEntr=cplxW[atgc[0]]+cplxW[atgc[1]]+cplxW[atgc[2]]+cplxW[atgc[3]];

                //rEntr=0;
                /*for ( idx il=0; il<sDim(atgc) ; ++il ){
                    real p=((real)atgc[il])/complexityWindow;
                    if(p)rEntr-=p*(log(p)*1.442695040888963); // log 2 p
                }*/
            }

            // the next letter in the complexity window
            idx iprv=0;
            if(flags&eCompileReverse && ik>0) {--ik;iprv=ik+complexityWindow;}
            else if(ik<lenseq-1){++ik;iprv=ik-complexityWindow;}
            newlet=((seq[ ((ik)/4) ]>> (((ik)%4)*2) )&0x3) ;
            oldlet=((seq[ ((iprv)/4) ]>> (((iprv)%4)*2) )&0x3) ;
            if(newlet!=oldlet){
                --atgc[oldlet];
                ++atgc[newlet];
            }
            ////PERF_END();
            //if(rEntr<complexityShannon)
            if(rEntr<cplxSh)
                continue;
        }

        ////PERF_START("HASH_BLOOM");
        if(flags&eCompileBloom)
            hb[hash>>3]|=(((udx)1)<<(hash&0x7));
        ////PERF_END();
        if( flags&eCompileOnlyBloom) {
            continue;
        }

        ////PERF_START("HASH_INDEXING");
        HLitem * hi, * hi0;

        #ifdef HASH_SINDEX
            idx iref=hashInd.add(hash,0,collisionReducer);
            idx ofs=hashInd[iref].ofs;
        #else 
            idx ofs=hashTbl[hash];
        #endif

        if( ofs == 0 ) { // first time 
            #ifdef HASH_SINDEX
                hashInd[iref].ofs=hC;
            #else
                hashTbl[hash]=hC;
            #endif
        
            hi0= hL+ hC ;
            hi0->ref._pos=0; 
            hi0->next=hC; // the position of the last one is the next one for now 
            ++hC;
        } else {
            hi0=hL + ofs;
        
            if(maxBin!=0 && hi0->ref._pos>=maxBin){
                ++cnt;
                ////PERF_END();
                continue;
            }
        }

        hi=hL+ hi0->next; // pointer to the previously added one

        hi0->ref._pos++; // the toal count for this hash
        hi0->next=hC; // the position of the last one 
        
        hi->next=hC;  // each one point to the next 
        hi=hL+hC; //  now the one we are adding
        hi->next=0 ;// this is the last one in our list 
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
        ////PERF_END();
//if(cnt%1000000==0)
  //  PERF_PRINT();

    }

    hList.cut(hC);

    ////PERF_END();
    return cnt;    
}



idx  sBioseqHash::dumpCompiled(const char * flnm, idx * maxBin, idx * aveBin, idx * totBin)
{/*
    sFile::remove(flnm);
    FILE * fp=strcmp(flnm,"stdout")==0 ? stdout : fopen(flnm,"w");if (!fp) return 0;

    idx c=hdr.lenTbl, absO=2*c*sizeof(idx);
    sVec < idx > Ofs(sMex::fSetZero); Ofs.addM( c*2 ); idx * ofs=Ofs.ptr();
    idx cutBin=maxBin ? *maxBin : 0;

    if(maxBin)*maxBin=0;
    if(aveBin)*aveBin=0;
    idx iNotNull=0;

    for  ( idx i=0 ; i < c ; ++i ) {
        idx bins=this->cnt(i);//, lstdim2;
        if(cutBin && bins>cutBin )bins=0;

        ofs[2*i]=absO;
        ofs[2*i+1]=bins;

        if(maxBin && *maxBin<bins) *maxBin=bins;
        if(aveBin && bins){*aveBin+=bins;++iNotNull;}
        absO+=bins*sizeof(idx);
    }

    if(aveBin && totBin )*totBin=*aveBin;
    if(aveBin && iNotNull )*aveBin/=iNotNull;

    fwrite ( &hdr, sizeof(hdr), 1 , fp );
    fwrite( (void *) ofs ,c , 2*sizeof (idx),fp);
    
    for  ( idx i=0 ; i < c ; ++i ) {
        idx bins=ofs[2*i+1];//hashTbl[i].dim();
        if(cutBin && bins>cutBin )bins=0;
        if(!bins)continue;

        sBioseqHash::HashBinType * lst=this->lst(i);//, * list2;
        
        //RefPos * ip=hashTbl[i].ptr(0);
        //fwrite( (void*)ip, bins, sizeof( RefPos ) , fp);
        for (  idx il=0; il<bins ; ++il ) {
            sBioseqHash::RefPos * ref=&(*lst)[il];
            fwrite(ref,sizeof(RefPos),1,fp);
        }
    }

    if(fp!=stdout)fclose(fp);
    return c;
    */
    return 0;
}

void  sBioseqHash::bloom (void)
{
    /*
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
    */
}

/*
void sSeqAlign::printHashTbl(bool printlits)
{
    for (idx i=0; i<hashTbl.dim() ;++i){
        sLst< RefPos > & lst = hashTbl[i];
        if(lst.dim()==0)
            continue;
        printf("%" DEC " %" DEC,i,lst.dim());

        if(printlits){
            for (  idx il=0; il<lst.dim() ; ++il )
                printf(" %" DEC,lst[il]._pos);
        }printf("\n");
    }
}
*/
/**************************************
void sBioseqHash::fossilize(idx refseq, const char * seq, idx lenseq, idx granule, idx fosblock, idx fosstp, idx history, idx flags) 
{

    idx maxDiag=sMin(granule,(idx)sizeof(idx)*8);
    sVec < short int > Mat; Mat.resizeM(maxDiag*3);//(fosblock*fosblock*3+2*granule);
    static const short int costs[4]={5,-4,-12,-4};
    
    // prepare the array of fossils
    sVec <idx > * hfVec=hashFossil.add();

    idx stp=fosstp;// fosblock/2;
    idx start=0;
    *hfVec->add()=0; // a placeholder for count
    for ( idx ivalid =0 ; start<lenseq ; ++ivalid , start+=stp ){
        sPerf::gDebugCurrent=1000000*ivalid+refseq;

        //Vertable.set(0);

        idx lenfos=(lenseq-start > fosblock ) ? fosblock : lenseq-start ;
        DEBUG_START(1)
            ::printf("\n\nDIRECT refseq=%" DEC "  start=%" DEC "\n",refseq,start);
        DEBUG_END()
        idx bm=dynamicMatch( Mat.ptr(), seq, seq, start, lenfos, start,lenfos , maxDiag, history, costs );
        *hfVec->add()=bm;

        if( flags&sBioseq::eFossilizeReverse ){
            DEBUG_START(1)
                ::printf("\n\nREVERSE refseq=%" DEC "  start=%" DEC "\n",refseq,start);
            DEBUG_END()

            bm=dynamicMatch( Mat.ptr(), seq, seq, start, lenfos, start,lenfos , maxDiag, history, costs , lenseq );
            *hfVec->add()=bm;
        }

        if(start+lenfos>=lenseq)break;
        //if(refseq==23 && start==4500)
        //    exit(0);
    }

    if(refseq%1000==0){
        PERF_PRINT();}
    (*hfVec)[0]=hfVec->dim()-1 ;
}
********************************************/
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
        if(lenseq ) lenseq=lenseq-1-ilet1; // reversing
        char let1=((seq1[ ((ilet1)/4) ]>> (((ilet1)%4)*2) )&0x3) ; // this particular base introduces this two bits
        
        for( idx is2=1; is2<maxDiag && is2+is1<len2; ++is2 ) {
            short int * vt=matP+is2;
            short int * st=vt+maxDiag;
            short int * bt=st+maxDiag;

            idx ilet2=is2+is1+st1;
            if(lenseq ) lenseq=lenseq-1-ilet2; // reversing
            char let2=((seq2[ ((ilet2)/4) ]>> (((ilet2)%4)*2) )&0x3) ; // this particular base introduces this two bits

            // the next value for this column
            *vt+=(let1==let2 ? costs[0] : costs[1] );

            //if(let1!=let2){
            if(*vt<=0){
                *vt=0;// contains the current row of dynamic programming
                *st=0; // no sequence currently being tracked
            } else {
                if(*st==0) // it hasn't been started before : start it
                    *st=(short int)is1;
                if( is1-*st+1 > *bt ) // remember the length to the maximum
                    *bt=is1-*st+1;
            }

            DEBUG_START(1)
                if(is2==1)::printf("%3d>",(int)is1);//(int) mat(is1,is2));//    ::printf("%3" DEC ">",is1);
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

/*
    ::printf("\n   >");
    for ( idx is2=1; is2<maxDiag; ++is2) {
        ::printf(" %2d    |",(int)mx[is2]);
    }
    ::printf("\n   >");
    idx ave=0, tmax=0;
    static idx totCnt=0;
    static real totAve=0, totAveMax=0;
    for ( idx is2=1; is2<maxDiag; ++is2) {
        ave+=bt[is2];
        tmax=sMax(tmax,(idx)bt[is2]);
        ::printf(" %2d    |",(int)bt[is2]);
    }
    ave/=maxDiag;
    totAve=(totAve*totCnt+ave)/(totCnt+1);
    totAveMax=(totAveMax*totCnt+tmax)/(totCnt+1);
    ++totCnt;
    ::printf("\nbmask=%" HEX "  ave=%" DEC " max=%" DEC " totAve=%.3lf totAveMax=%.3lf totCnt=%" DEC " \n",bmask,ave,tmax,totAve,totAveMax,totCnt);
    //::printf("\n");

*/

    //exit(0);
    return bmask;
    #undef mat
    
}


idx  sBioseqHash::dumpFossilized (const char * flnm)
{

    //sFile::remove(flnm);
    FILE * fp=strcmp(flnm,"stdout")==0 ? stdout : fopen(flnm,"w");if (!fp) return 0;

    idx c= hashFossil.dim();
    sVec < idx > Ofs; idx * ofs=Ofs.add(1+c), absO=(c+1);
    ofs[0]=0;
    for (idx i=0; i<c; ++i ){
        sVec < idx > & hF=hashFossil[i];
        idx hCnt=hF[0];

        ofs[1+i]=absO;
        absO+=1+hCnt; // the valid size + count
    }
    ofs[0]=c; // we remember the count of reference sequences as well

    // write the count and offset table
    fwrite( (void*)ofs, 1+c, sizeof(idx) , fp);

    for (idx i=0; i<c; ++i ){
        sVec < idx > & hF=hashFossil[i];
        //idx * hf=hF.ptr(2);
        idx hCnt=hF[0];
        fwrite( (void*)hF.ptr(), 1+hCnt, sizeof( idx ) , fp); // write the fossil hashes


        /* printf("%" DEC " cnt=%" DEC "\n", i, hF[0]);
        for ( idx j=0; j<hCnt; ++j){
            printf("\t%" DEC ",%" HEX "\n",j,hF[j+1]);
        }
        printf("\n\n");
        */
    }


    fclose(fp);
    return c;
}






/*
idx sBioseqHash::fossilizeQuick(idx * fossillist, idx foscnt, const char * seq, idx lenseq, idx granule, idx granstep , idx fosblock, idx history, idx flags)
{
    // here are the two pointers we move in the buffer every granule letters
    idx fossil=0, ivalid=0;
    granule = lenseq;
    idx incr[4]={+3,+5,+7, +11 };
    idx maxDiffImportant=(incr[sDim(incr)-1]*incr[sDim(incr)-2])+incr[sDim(incr)-1]-1;
    
    
    idx  maxSignalOberton=10,lenSignal=granule+incr[sDim(incr)-1]*maxSignalOberton;
    idx sigImpendance=2, maxStrength=1;
    for ( idx is=0; is<maxSignalOberton; ++is) maxStrength*=sigImpendance;
    idx maxStrPoss=maxStrength*maxStrength*maxStrength*maxStrength;
    
    sVec < idx > Bitmask; Bitmask.resize(lenSignal); //Bitmask.resize( (granule-1)/(8*sizeof(idx))+1 );
    idx * bitmask=Bitmask.ptr();
    
    sVec < idx > Signal[8]; idx * signal[8];
    for ( idx i=0; i<sDim(signal); ++i) {
        Signal[i].resize(lenSignal);
        signal[i]=Signal[i].ptr();
    }
        
    for( idx ib=0, il=1; ib+granule<=lenseq ; ib+=granstep, ++il) {

        for ( idx i=0; i<sDim(signal); ++i) { // zero all signals 
            Signal[i].set();
        }
        Bitmask.set();
        
        idx dlen=0;
        for( idx is=ib, ip=0; ip<granule ; ++is, ip++) {

            // get the two bits necessary
            idx let=(idx)((seq[is/4]>>((is%4)*2))&0x3) ; // this particular base introduces this two bits
            
            idx strength=maxStrength;
            for ( idx si=0; si<=maxSignalOberton ; ++si, strength/=sigImpendance){
                idx ind=ip + si*incr[let]; 
            
                //idx ind=si*incr[let]; 
                if(ind>=lenSignal)break;
                signal[let][  ind ]+=strength;
                
                if(dlen<ind )dlen=ind;
            }
        }
        
        idx maxDlen=0, maxVal=0;
        for ( idx si=0; si<dlen; ++si ){
            signal[4][si]=1;
            idx s=0;
            for ( idx il=0; il<4; ++il ){
                s=signal[il][si];
                signal[4][si]*=s;
                if(s==0)break; // no point in multiplying zeros furtherr
            }
            if(maxVal<signal[4][ si ])maxVal=signal[4][ si ];
        }
        
        idx imsk=0;
        for ( idx si=0; si<dlen; ++si ){
            if( signal[4][ si ] >= maxVal/4 ){
                signal[5][si]=1;
                maxDlen=si;
                signal[6][imsk]=si;
                ++imsk;
            }
        }
        dlen=maxDlen;
        
        //idx isig=0;
        for( idx si=0; si<imsk-1 ; ++si ) {
            
            for ( idx ic=si+1; ic<imsk; ++ic ) { 
                idx dif= signal[6][ic]- signal[6][si];
                if(dif>maxDiffImportant)break; // further differences are repeats
                ++signal[7][dif];
                //++isig;
            }
        }
        
        {
            sFile::remove("W:/out.csv");
            sFil ff("W:/out.csv");
            for ( idx si=0; si<dlen; ++si ){
                ff.printf("%" DEC "",si);
                for ( idx il=0; il<sDim(signal); ++il ){
                    ff.printf(", %" DEC,signal[il][si] );
                }
                ff.printf("\n");
            }
        }

        if(il==fosblock){
            fossillist[ivalid++]=fossil;
            fossil=0;
            il=0;
            if(ivalid>=foscnt) // enough elements in ?
                break;
        }
    }
    return ivalid;
}
*/
/*
idx sBioseqHash::fossilizeQuick(idx * fossillist, idx foscnt, const char * seq, idx lenseq, idx granule, idx granstep , idx fosblock, idx , idx flags) // history
{
    // here are the two pointers we move in the buffer every granule letters
    idx fossil=0, ivalid=0;
    idx atgc[8];

    for( idx ib=0, il=1; ib+granule<=lenseq ; ib+=granstep, ++il) {

        //char prv[100];for(idx is=0; is<sDim(prv); ++is) prv[is]=1;// this is more than enough for history

        atgc[0]=0;atgc[1]=0;atgc[2]=0;atgc[3]=0;
        atgc[4]=0;atgc[5]=0;atgc[6]=0;atgc[7]=0;

        for( idx is=ib, ip=0; ip<granule ; ++is, ++ip) {

            // get the two bits necessary
            idx let=(idx)((seq[is/4]>>((is%4)*2))&0x3) ; // forward

            //idx val=(let+1)* prv[ ip%history ]; // compute the contribution from this letter
            idx val=1;//prv[ (ip+16)%history ]; // compute the contribution from this letter
            //prv[(is+1)%history]=let+1; // this letter is the previous letter for the next one

            atgc[let]+=val;
            if((flags&sBioseq::eFossilizeReverse)==0)continue;

            idx irev=(lenseq-is);
            let=(idx)((seq[irev/4]>>((irev%4)*2))&0x3) ; // reverse
            let=sBioseq::mapComplementATGC[let];
            val=1;//    prv[ (ip-1)%history ]; // second degree hash
            atgc[let+4]+=val;
        }


        for ( idx ih=0; ih<2; ++ih ){
            idx sft=ih*4;
            idx iBit=0, stp=6;

            for ( idx is1=0; is1<3; ++is1 ) {
                idx imax=0, iposmax=0;

                for ( idx is2=0, ipos=0; is2<4; ++is2 ) {
                    if(atgc[sft+is2]==-1) continue;
                    if (atgc[sft+is2] > atgc[sft+imax] ) {imax = is2; iposmax=ipos;}
                    ++ipos;
                }

                iBit+=iposmax*stp;
                if(stp>1)stp/=(3-is1);
                atgc[sft+imax]=-1; // we do not count this anymore
            }
            fossil|=((idx)1)<<(iBit + 24*ih);
        }




        if(il==fosblock){

            fossillist[ivalid++]=fossil;
            idx cntNon0=0;
            for ( idx ii=0; ii<24; ++ii) {
                if(fossil&0x01)++cntNon0;
                fossil>>=1;
            }
            fossil=0;
            il=0;
            if(ivalid>=foscnt) // enough elements in ?
                break;
        }
    }



    return ivalid;
}

*/


/*
idx sBioseqHash::fossilizeQuick(idx * fossillist, idx foscnt, const char * seq, idx lenseq, idx granule, idx granstep , idx fosblock, idx history, idx flags)
{
    // here are the two pointers we move in the buffer every granule letters
    idx fossil=0, ivalid=0;
    idx daubNum=0;
    if(daubNum)sMathNR::pwtset(daubNum);
    granule = lenseq;


    //idx ALLDIM=1024;
    idx k; // find the minimal size of the fft buffer enough to fit our data
    for(k=1; k<lenseq; k<<=1);

    sVec < double > Shape; Shape.resize(2*(k+1));
    double * shape=Shape.ptr();
    idx N=5;
    sVec < double > DBG; DBG.resize(N*(k+1));
    idx cutMin=0,cutMax=10000000;//2+lenseq/100, cutMax=4;//lenseq/4;

    for( idx ib=0, il=1; ib+granule<=lenseq ; ib+=granstep, ++il) {

        idx at=0,gc=0;
        Shape.set();
DBG.set();
        idx cumul=0;
        idx dlen=0;
        char incr[5]={+1,-1,+2,-2};
        for( idx is=ib, ip=0; ip<granule ; ++is, ip++) {

            // get the two bits necessary
            idx let=(idx)((seq[is/4]>>((is%4)*2))&0x3) ; // this particular base introduces this two bits
            //idx let=(idx)seq[is] ; // this particular base introduces this two bits

            //if(let&0x2) gc+=(let&0x1) ? 1 : -1; // for g or c  move gc
            //else at+=(let&0x1) ? 1 : -1; // for a or t move at

            shape[1+dlen]=cumul+incr[let];
            //shape[1+dlen]=let;
            cumul=shape[1+dlen];
            ++dlen;
            //cumul+=(let  == 0) ? 1 : 0 ;
            //shape[1+ip]=cumul;

            //double d2=at*at+gc*gc;
            //shape[1+ip]=d2 ? at/sqrt(d2) : 0;
            //shape[1+ip]=d2 ;
        }

        for(k=1; k<dlen; k<<=1);


            // original data
        for ( idx is=0; is<dlen; ++is )DBG[N*is]=shape[1+is];
        // perform wavelett or FT analysis
        if(daubNum)sMathNR::wt1(shape,k,1,sMathNR::pwt);
        else sMathNR::realft(shape,k,1);

        // coefficients
        //for(idx is=cutMax; is<2*k; ++is) shape[1+is] = 0;
        //for(idx is=0; is<cutMin; ++is) shape[1+is] = 0;
        double mm=0;
        for ( idx is=0; is<k; ++is ){
            DBG[N*is+1]= is==0 ? k : 1.*k/is;// *1./lenseq;
            DBG[N*is+2]=shape[1+2*is];
            DBG[N*is+3]=shape[1+2*is+1];
            DBG[N*is+4]=sqrt(DBG[N*is+2]*DBG[N*is+2]+DBG[N*is+3]*DBG[N*is+3]);
            if ( mm<DBG[N*is+4] )mm=DBG[N*is+4];
            //mm+=DBG[N*is+4];
        }
        //for ( idx is=0; is<k; ++is ){DBG[N*is+4]/=mm;}

        // recovered
        if(daubNum)sMathNR::wt1(shape,k,-1,sMathNR::pwt);
        else {
            sMathNR::realft(shape,k,-1);
            for(idx is=1; is<granule+1; is++)shape[is]*=2./k;
        }
        //for ( idx is=lenseq+1; is<k; ++is ){shape[1+is]=0;}
        //for ( idx is=0; is<k; ++is )DBG[N*is+4]=shape[1+is];


        //idx iBit=1;
        //fossil|=((idx)1)<<(iBit);

        {
            sFile::remove("/W/out.csv");
            sFil ff("/W/out.csv");
            for ( idx is=0; is<k; ++is ){
                ff.printf("%" DEC "",is);
                for ( idx in=0; in<N; ++in ){
                    ff.printf(", %lf",DBG[N*is+in] );
                }
                ff.printf("\n");
            }
        }

        if(il==fosblock){
            fossillist[ivalid++]=fossil;
            fossil=0;
            il=0;
            if(ivalid>=foscnt) // enough elements in ?
                break;
        }
    }
    return ivalid;
}

    */






//if(cv<=fv){
//    cv=fv;
    //bak(is1,is2)=3;
    //ext(is1,is2) = (is1>0 && is2>0) ?  ext(is1-1,is2-1)+1 : 1 ;
//}
//if(cv)
//    mat(is1,is2)=cv;

//if(fv + vertable[is2-is1]>0)
//    vertable[is2-is1]=fv;

// consider insertion
/*if(is1>0) {
    fv=mat(is1-1,is2)+ ( bak(is1-1,is2)==3 ? costs[2] : costs[3]);
    if( cv<fv ) {
        cv=fv; bak(is1,is2)=1;
        ext(is1,is2) = ext(is1-1,is2)+1;
    }
}
// consider deletion
if(is2>0){
    fv=mat(is1,is2-1)+( bak(is1,is2-1)==3 ? costs[2] : costs[3]);
    if( cv<fv ) {
        cv=fv; bak(is1,is2)=2;
        ext(is1,is2) = ext(is1,is2-1)+1;
    }
}*/

//if(cv)
//        mat(is1,is2)=cv;
//    if(is1==is2)
//        mat(is1, is2)=0;

//if(is1==is2)
    //    ::printf("%3d>",(int)is1);//(int) mat(is1,is2));//    ::printf("%3" DEC ">",is1);
//else
//if(mat(is1,is2)>vertable[is2-is1]){
//    vertable[is2-is1]=mat(is1,is2);
    //bittable[is2-is1]=ext(is1,is2);
//    bittable[is2-is1]=(is1>0 && is2>0) ?  ext(is1-1,is2-1)+1 : 1 ;
//    //::printf("+%2x(%d)|",(int) mat(is1,is2),(int)bak(is1,is2) );
//}
//else if ( mat(is1,is2)!=0)
//    ::printf(" %2x(%d)|",(int) mat(is1,is2),(int)bak(is1,is2));
//else ::printf("      |");




idx sBioseqHash::longestDynamicMatch( short int * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2, idx len2, idx maxDiag, idx lenseq,  idx * matchPos )
{

    memset(matP,0,maxDiag*3*sizeof(short int));

    for( idx is1=0; is1<len1; ++is1 ) {

        idx ilet1=is1+st1;
        if(lenseq ) lenseq=lenseq-1-ilet1; // reversing
        char let1=((seq1[ ((ilet1)/4) ]>> (((ilet1)%4)*2) )&0x3) ; // this particular base introduces this two bits

        for( idx is2=0; is2<maxDiag && is2<len2; ++is2 ) {
            short int * vt=matP+is2;
            short int * st=vt+maxDiag;
            short int * bt=st+maxDiag;

            idx ilet2=is2+st2;
            if(lenseq ) lenseq=lenseq-1-ilet2; // reversing
            char let2=((seq2[ ((ilet2)/4) ]>> (((ilet2)%4)*2) )&0x3) ; // this particular base introduces this two bits

            // the next value for this column
            if(let1==let2) ++(*vt);
            else (*vt)=0;

            if(*vt<=0){
                *vt=0;// contains the current row of dynamic programming
                *st=0; // no sequence currently being tracked
            } else {
                if(*st==0) // it hasn't been started before : start it
                    *st=(short int)is1;
                if( is1-*st+1 > *bt ) // remember the length to the maximum
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
        idx ishift=(isS%4)*2; // determine the byte number and the shift count
        idx val=(idx)((seq[ibyte]>>ishift)&0x3) ; // this particular base introduces this two bits
        if(flags&eCompileReverse)
            val=sBioseq::mapComplementATGC[val] ;


        hash|=val; //_seqBits(seq, is, flags );
        if(is<hdr.lenUnit-1){rememberedHash = hash;continue;} // we still didn't accumulate enough letters

        hash&=hdr.hashMask;
        rememberedHash = hash;

        if(hashStp>1 && iStp<(hashStp-1)){
            ++iStp;
            continue;//{PERF_END();continue;}
        }else iStp=0;

        suff_lst[hash]=is;

    }
    return 0;
}
