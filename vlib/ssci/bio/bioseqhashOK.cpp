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

//sPerf gPerf;

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



// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  private utilities
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

idx sBioseqHash::compile(idx refseq, const char * seq, idx lenseq, idx flags, idx maxBin, idx startcompile, idx lencompile)
{
PERF_START("Init");
    if( (flags&eCompileOnlyBloom)==0 && (flags&eCompileDic)==0 ){
        if(hashTbl.dim()!=hdr.lenTbl){
            hashTbl.mex()->flags=sMex::fBlockCompact;//sMex::fExactSize;
            hashTbl.empty();
            hashTbl.add(hdr.lenTbl);
            for(idx i=0; i<hashTbl.dim(); ++i) {
                hashTbl[i].mex()->flags=sMex::fBlockCompact;//fExactSize;
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

    //sVec < idx > ofs; ofs.add( hdr.lenTbl );//ofs.set(0);

    idx cnt=0;
    udx hash=0; // this is unsigned due to rotations sign digit overload
    //bool hardCut=maxBin>0 ? false : true; maxBin=sAbs(maxBin);
    
    if(lencompile==0)lencompile=lenseq;

    for( idx is=startcompile; is+hdr.lenUnit<startcompile+lencompile ; ++is) {
            
        //     prepare hash for this position    
        hash<<= ( 2 );
    
        idx isS=(flags&eCompileReverse) ? lenseq-1-is : is ;
        // get the two bits necessary 
        idx ibyte=isS/4;
        idx ishift=(isS%4)*2; // determine the byte number and the shift count

        idx val=(idx)((seq[ibyte]>>ishift)&0x3) ; // this particular base introduces this two bits
        
        if(flags&eCompileReverse)
            val=sBioseq::mapComplementATGC[val] ;

        hash|=val; //_seqBits(seq, is, flags );
        if(is<hdr.lenUnit-1)continue; // we still didn't accumulate enough letters
        hash&=hdr.hashMask;

PERF_START("Bloom");

        if(flags&eCompileBloom)
            hb[hash/8]|=((udx)1<<(hash%8));
        if( flags&eCompileOnlyBloom)
            continue;
PERF_NEXT("DIC");

        // add a reference in this hash position 
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
    //bloom();
    return cnt;    
}


idx  sBioseqHash::dumpCompiled (const char * flnm, idx * maxBin, idx * aveBin)
{
    sFile::remove(flnm);
    FILE * fp=strcmp(flnm,"stdout")==0 ? stdout : fopen(flnm,"w");if (!fp) return 0;
    
    idx c=hashTbl.dim(), absO=2*c*sizeof(idx);//+sizeof(Hdr);
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
        //idx ibyte=i/8;
        //idx ibit=((udx)1<<(i%8));
        idx c=0;
        if(ofs)c=ofs[i*2+1];
        else if(hashTbl.dim() )c=hashTbl[i].dim();
        //else if(hashInList.dim() )c=hashInList[&i];
        else { 
            idx idxH=0;
            c=hashDic.find(i,&idxH);
        }

        if( c )
            hb[i/8]|=((udx)1<<(i%8));
    }
}

/*
void sSeqAlign::printHashTbl(bool printlits)
{
    for (idx i=0; i<hashTbl.dim() ;++i){
        sLst< RefPos > & lst = hashTbl[i];
        if(lst.dim()==0)
            continue;
        printf("%"_d_" %"_d_,i,lst.dim());

        if(printlits){
            for (  idx il=0; il<lst.dim() ; ++il )
                printf(" %"_d_,lst[il]._pos);
        }printf("\n");
    }
}
*/


void sBioseqHash::fossilize(idx , const char * seq, idx lenseq, idx granule, idx fosblock, idx history, idx flags) // refseq
{
    sVec < char > Mat(sMex::fSetZero); Mat.resizeM(fosblock*fosblock);
    char * matP=Mat.ptr();

    // prepare the array of fossils
    sVec <idx > * hfVec=hashFossil.add();
    //idx foscnt = (((lenseq-granule)/granstep)+1)/fosblock+1; // how much array do we need
    //idx ddim=1+foscnt; // first for count , the rest fossils
    //if(ddim<=1)ddim=1;
    //idx * hf=hfVec->add(ddim);hfVec->set();
    //*((idx*)hf)=refseq;

    //idx * fossillist=hfVec->ptr(1);
    //idx ivalid=fossilizeQuick(fossillist, foscnt, seq, lenseq, granule, granstep , fosblock, history, flags);
    //idx ivalid;

    sVec < idx > Vertable; Vertable.resizeM(granule);idx * vertable=Vertable.ptr();

    idx stp=fosblock/2;
    idx start=0;
    *hfVec->add()=0; // a placeholder for count
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

        char let1=((seq1[ ((is1+st1)/4) ]>> (((is1+st1)%4)*2) )&0x3) ; // this particular base introduces this two bits

        for( idx is2=is1; is2<len2 && is2<is1+maxDiag; ++is2 ) {

            char let2=((seq2[ ((is2+st2)/4) ]>> (((is2+st2)%4)*2) )&0x3) ; // this particular base introduces this two bits


            char prv=(is1>0 && is2>0) ? mat(is1-1,is2-1) : 0;
            if(let1==let2) {
                if(prv<maxDiag)
                    mat(is1, is2)=prv+1;
            }

            //::printf(" %2x",(int) mat(is1,is2));

            if(mat(is1,is2)>vertable[is2-is1])
                vertable[is2-is1]=mat(is1,is2);
        }
    //    ::printf("\n");
    }

    //for ( idx is2=0; is2<maxDiag; ++is2) {
    //    printf(" %"_d_,vertable[is2]);
    //}
    //::printf("\n");

    idx bmask=0;
    for ( idx is2=1; is2<maxDiag; ++is2) {
        //if(vertable[is2]<nMereKeep)
        //vertable[is2]=0;
        //else if(is2< (idx)(8*sizeof(bmask)))
        if(vertable[is2]>=nMereKeep)
            bmask|=((idx)1)<<is2;
    }
//    vertable[0]=0;


    //for ( idx is2=0; is2<maxDiag; ++is2) {
    //    printf(" %"_d_,vertable[is2]);
    //}
    //::printf("\nbmask=%"_x_"  |=%"_x_"\n",bmask , bmask&0x21000000000021);
    //::printf("\n");

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


        /* printf("%"_d_" cnt=%"_d_"\n", i, hF[0]);
        for ( idx j=0; j<hCnt; ++j){
            printf("\t%"_d_",%"_x_"\n",j,hF[j+1]);
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
                ff.printf("%"_d_"",si);
                for ( idx il=0; il<sDim(signal); ++il ){
                    ff.printf(", %"_d_,signal[il][si] );
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
                ff.printf("%"_d_"",is);
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

