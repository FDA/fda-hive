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
#include <ssci/bio/filterseq.hpp>
#include <slib/std/string.hpp>
#include <slib/utils.hpp>
#include <ssci/bio/vioseq2.hpp>
//#include <violin/violin.hpp>
#include <math.h>

#define RAND1 (real)rand()/RAND_MAX
using namespace slib;

bool sFilterseq::qualityFilter ( const char * qua, idx len, idx threshold, real percentage100)
{
    idx sum = 0;
    idx count = 0;
    for (idx ik = 0; ik < len; ++ik){
        sum += qua[ik];
        if (qua[ik] >= threshold){
            count += 1;
        }
    }
    if ( ((count*100)/len) < (percentage100)){
        return false;
    }
    return (threshold < (real)(sum/len)) ? true : false;
}

idx sFilterseq::trimQuality(const char *qua, idx len, idx threshold, idx minlen)
{
    idx ik;
    for (ik = len-1; 0 <= ik && qua[ik] < threshold; --ik);
    return ik < minlen ? ik : 0;
}

bool sFilterseq::trimPosition(idx startseq, idx endseq, idx start, idx end)
{
    if (startseq >= start && endseq <= end){
        return true;
    }
    return false;
}

idx sFilterseq::complexityFilter( const char * seq, idx len ,idx complexityWindow, real complexityEntropy, bool isCompressed, idx optiontoCut, bool considerNs)
{

    idx atgc[5];atgc[0]=0;atgc[1]=0;atgc[2]=0;atgc[3]=0;atgc[4]=0;
    char let;
    idx counter;
    idx cplx=(idx)(complexityEntropy*1000);
//    if(complexityFilterPrecomputedArray.dim()!=complexityWindow+1)
//        sFilterseq::precomputePLogPScaled(complexityWindow, 1000);
    bool reverse = false;
    bool cutCorner = false;

    if (optiontoCut != 0){
        cutCorner = true;
    }
    if (optiontoCut==2){
        reverse = true;
    }

    // precompute to avoid real math in the bigger loop
    static idx stComplexityWindow=0;
    static idx cplxW[1024];
    if(stComplexityWindow!=complexityWindow) {
        stComplexityWindow=complexityWindow;
        cplxW[0]=0;cplxW[sDim(cplxW)-1]=0;
        for ( idx i=1; i<sDim(cplxW); ++i){
            real p=((real)i)/complexityWindow;
            cplxW[i]=(idx)(-p*(log(p)*1.442695040888963)*1000);
        }
    }

    if( isCompressed) {
        if (!reverse){
            counter = 0;
            for ( idx ik=0 ; ik <len ; ++ik ){
                // adjust the accumulation matrix
                if(ik>complexityWindow-1){ // subtract the oldest letter
                    let=((seq[ ((ik-complexityWindow)/4) ]>> (((ik-complexityWindow)%4)*2) )&0x3) ;
                    --atgc[(idx)let];
                }
                let=((seq[ (ik/4) ]>> ((ik%4)*2) )&0x3) ; //add the newest letter

                ++atgc[(idx)let];

                if(ik<complexityWindow-1)
                    continue; // not enough letters accumulated in this window;
                idx rEntr=cplxW[atgc[0]]+cplxW[atgc[1]]+cplxW[atgc[2]]+cplxW[atgc[3]];
                if(rEntr<cplx) {  // if this window is ok .. just move along to the next position
                    if (!cutCorner){
                        return -1;
                    }
                    counter++;
                }
                else if (cutCorner) {
                    if (counter == 0){return 0;}
                    return ik;
                }
            }
        }
        else {
            complexityWindow = len - complexityWindow;
            counter = 0;
            for ( idx ik=len-1 ; ik >= 0 ; --ik ){
                // adjust the accumulation matrix
                if(complexityWindow-1 > ik){ // subtract the oldest letter
                    let=((seq[ ((ik+len-complexityWindow)/4) ]>> (((ik+len-complexityWindow)%4)*2) )&0x3) ;
                    --atgc[(idx)let];
                }
                let=((seq[ (ik/4) ]>> ((ik%4)*2) )&0x3) ; //add the newest letter

                ++atgc[(idx)let];

                if(complexityWindow-1 < ik)
                    continue; // not enough letters accumulated in this window;
                idx rEntr=cplxW[atgc[0]]+cplxW[atgc[1]]+cplxW[atgc[2]]+cplxW[atgc[3]];
                if(rEntr<cplx) {  // if this window is ok .. just move along to the next position
                    if (!cutCorner){
                        return -1;
                    }
                    ++counter;
                }
                else {
                    if (counter == 0){return len;}
                    return ik+1;
                }
            }
        }
    }
    else {
        for ( idx ik=0 ; ik <len ; ++ik ){
            // adjust the accumulation matrix
            if(ik>complexityWindow-1){ // subtract the oldest letter
                let=sBioseq::mapATGC[(idx)seq[ik-complexityWindow]];
                --atgc[(idx)let];
            }
            let=sBioseq::mapATGC[(idx)seq[ik]];

            ++atgc[(idx)let];

            if(ik<complexityWindow-1)
                continue; // not enough letters accumulated in this window;
            idx rEntr=cplxW[atgc[0]]+cplxW[atgc[1]]+cplxW[atgc[2]]+cplxW[atgc[3]];
            if (considerNs){
                rEntr += cplxW[atgc[4]];
            }
            if(rEntr<cplx) {  // if this window is ok .. just move along to the next position
                return -1;
            }
        }
    }
    return 0;
}

bool sFilterseq::complexityFilter_wholeSeq_ChunkSize( const char * seq, idx len , idx chunkSize, real complexityEntropy,  bool considerNs)
{
    if (len < chunkSize || len > 1024){
        // report an error
        return true;
    }
    idx combSize = 1;
//    real pi = 0.25;
    considerNs = false;

    idx atgc[257];
    for (idx i = 0; i < chunkSize; i++){
        combSize *= 4;
    }
    for (idx i = 0; i < combSize; i++){
        atgc[i] = 0;
    }

    idx let;

    idx cplx=(idx)(complexityEntropy*1000);


    // precompute to avoid real math in the bigger loop
//    static idx stComplexityWindow=0;
    static idx cplxW[1024];
//    static idx cplxWA[1024];
    cplxW[0] = 0;
    cplxW[len/chunkSize] = 0;

    for ( idx i=1; i<len/chunkSize; ++i){
        real p=((real)i)/(len/chunkSize);
        cplxW[i]=(idx)(-p*(log(p)*1.442695040888963)*1000);
    }
//    for ( idx i=1; i< len/chunkSize; ++i){
//        real p = 1.0;
//        for (idx j = 1; j <= i; j++){
//            p *= (real)(len+1-j)/j;
//            p *= pi;
//        }
//        for (idx j = 1; j<=len-i; j++){
//            p *= (1.0-pi);
//        }
//
//        cplxW[i]=(idx)(-p*(log(p))*1000);
//    }



    for ( idx ik=0 ; ik <len-chunkSize ; ik+=chunkSize ){
        // adjust the accumulation matrix
//        if(ik>complexityWindow-1){ // subtract the oldest letter
//            let=((seq[ ((ik-complexityWindow)/4) ]>> (((ik-complexityWindow)%4)*2) )&0x3) ;
//            --atgc[(idx)let];
//        }

        let=((seq[ (ik/4) ]>> ((ik%4)*2) )&0x3) ; //add the newest letter
        for (idx i = 1; i < chunkSize; i++){
            let = (let) |  ((seq[ ((ik+i)/4) ]>> (((ik+i)%4)*2) )&0x3) << 2*i;
        }

        ++atgc[let];

//        if(ik<len-chunkSize)
//            continue; // not enough letters accumulated in this window;

//                idx rEntr=cplxW[atgc[0]]+cplxW[atgc[1]]+cplxW[atgc[2]]+cplxW[atgc[3]];


    }
    idx rEntr = 0;
//    idx maxEntr = cplxW[len/combSize]*combSize;
    for (idx i = 0; i < combSize; i++){
        rEntr += cplxW[atgc[i]];
    }

    if(rEntr<cplx)
        return false;
    else
        return true;


}

//bool sFilterseq::complexityFilter2( const char * seq, idx len ,idx complexityWindow, real complexityEntropy , idx complexityCharLen)
//{
//
//    idx atgc[4];
//    idx lenATGCBuf=((idx)1)<<(complexityCharLen*2);
//    //idx mask=lenATGCBuf-1;
//    sSet(atgc,0,sizeof(idx)*sDim(atgc));
//
//    idx gs=0;
//    char let;
//    idx cplx=(idx)(complexityEntropy*1000);
//
//    // precompute to avoid real math in the bigger loop
//    static idx stComplexityWindow=0;
//    static idx cplxW[1024];
//    if(stComplexityWindow!=complexityWindow) {
//        stComplexityWindow=complexityWindow;
//        cplxW[0]=0;cplxW[sDim(cplxW)-1]=0;
//        for ( idx i=1; i<sDim(cplxW); ++i){
//            real p=((real)i)/complexityWindow;
//            cplxW[i]=(idx)(-p*(log(p)*1.442695040888963)*1000);
//        }
//    }
//
//    for ( idx ik=0 ; (ik+1)*complexityCharLen<=len ; ++ik ){
//        // adjust the accumulation matrix
//        if(ik>gs+complexityWindow-1){ // subtract the oldest letter
//            let=0;
//            for( idx il=0;  il<complexityCharLen; ++il )
//                    let=let<<(complexityCharLen*2) | ((seq[ ((ik*complexityCharLen+il-complexityWindow)/4) ]>> (((ik*complexityCharLen+il-complexityWindow)%4)*2) )&0x3) ;
//            --atgc[(idx)let];
//        }
//        let=0;
//        for( idx il=0;  il<complexityCharLen; ++il )
//            let=let<<(complexityCharLen*2) | ((seq[ ((ik*complexityCharLen+il)/4) ]>> (((ik*complexityCharLen+il)%4)*2) )&0x3) ; //add the newest letter
//        ++atgc[(idx)let];
//
//        if(ik<gs+complexityWindow-1)
//            continue; // not enough letters accumulated in this window;
//        //idx rEntr=cplxW[atgc[0]]+cplxW[atgc[1]]+cplxW[atgc[2]]+cplxW[atgc[3]];
//        idx rEntr=0;
//        for( idx ib=0;  ib<lenATGCBuf; ++ib )
//            rEntr+=cplxW[atgc[ib]];
//
//        if(rEntr<cplx) {  // if this window is ok .. just move along to the next position
//            return false;
//        }
//
//    }
//    //::printf("%s #####################OK f\n",t.ptr(0));
//    return true;
//}

idx sFilterseq::k_match(const char* s1, const char* q1, idx len1, const char* s2, const char* q2, idx len2, idx max_mismatch, char q_cut)
{
    idx mismatch = 0;
    idx match = 0;
    for(idx i = 0; ((i < len1) && (i < len2)); i++) {
        //if we have a match, or at least good quality bases...
        if( (s1[i] == s2[i] && s1[i] != 'N') || ((q1[i] >= q_cut) && (q2[i] >= q_cut)) ) {
            if( s1[i] != s2[i] || s1[i] == 'N' ) {
                mismatch++;
                if( mismatch > max_mismatch )
                    return 0;
            } else {
                match++;
            }
        }
    }
    return match;
}

idx sFilterseq::primersFilter(const char *seq, idx seqlen, const char *primer, idx prlen, idx minLength, idx maxMissMatches, bool reverse, bool keepIfMid, bool keepIfEnd, idx windowLen)
{
    idx ispos = 0, ippos=0, ik=0;
    idx matches = 0, missMatches = 0;
    if ((windowLen == 0) || (windowLen > seqlen)){
        windowLen = seqlen;
    }
    if(!reverse) {
            for ( ispos=windowLen-1 ; ispos>=0 ; --ispos) {
                missMatches=0;
                matches = 0;
                for ( ik=0, ippos=prlen-1; ispos-ik>=0 && ippos>=0; --ippos, ++ik ) {
                    //char let=sBioseq::mapRevATGC[((seq[ ((ispos-ik)/4) ]>> (((ispos-ik)%4)*2) )&0x3)];
                    char let=seq[ispos - ik];
                    if(primer[ippos]=='.')continue;
                    if(let!=primer[ippos]){
                        ++missMatches;
                    }
                    else {
                        ++matches;
                    }
                    if(missMatches>maxMissMatches){
                        break;
                    }
                }
                if( ippos<=0 || ispos-ik<0 || ik >= minLength)
                    break;
            }
            if(ispos < 0) {return -1; } // no matches
            if(ik >= minLength){ // enough characters matched from primer
                bool doKeep=false;
                if( (ispos-ik<0) && keepIfEnd ) doKeep=true;// primer matched to the end : cut the sequence or keep it ?
                if( (ispos-ik>=0) && keepIfMid ) doKeep=true;// // primer matched in the middle : remove the sequence

                if(doKeep) { // if(ispos-ik<0){ // primer matched to the end : cut the sequence
                    idx movpos=prlen-1-ippos +(ispos-ik+1);
                    return movpos;
                }
                else // primer matched in the middle : remove the sequence
                    return 0;
            }

        }else {
            for ( ispos= (seqlen - windowLen); ispos<seqlen ; ++ispos) {
                idx missMatches=0;
                for ( ik=0, ippos=0; ippos<prlen && ispos+ik<seqlen; ++ippos , ++ik) {
                    //char let=sBioseq::mapRevATGC[((seq[ ((ispos+ik)/4) ]>> (((ispos+ik)%4)*2) )&0x3)];
                    char let=seq[ispos + ik];
                    if(primer[ippos]=='.')continue;
                    if(let!=primer[ippos]){
                        ++missMatches;
                    }
                    if(missMatches>maxMissMatches){
                        break;
                    }
                }
                if( ippos>=prlen || ispos+ik==seqlen || ippos >= minLength)
                    break;
            }
            if(ispos>=seqlen) {return -1;} // no matches
            if(ippos >= minLength){ // enough characters matched from primer
                bool doKeep=false;
                if( (ispos+ik>=seqlen) && keepIfEnd ) doKeep=true;// primer matched to the end : cut the sequence or keep it ?
                if( (ispos+ik<seqlen) && keepIfMid ) doKeep=true;// // primer matched in the middle : remove the sequence

                if(doKeep){// if(ispos+ik>=rec.lenSeq){ // primer matched to the end : cut the sequence
                    return ispos;
                }
                else // primer matched in the middle : remove the sequence
                    return 0;
            }
        }
    return -1;
}

//    bool quaBit = getQuaBit(row);
//    if (quaBit){
    //    const char * seqqua = Q.qua(row);
//        idx len = Q.len (row)
    //    bool isValid = true;
    //    if( seqqua && *seqqua ) {
    //        isValid = filterNoise(seqqua, 0, len, value);
    //    }
    //}

//bool sFilterseq::filterNoise (const char *qua, idx start, idx end, idx limit)
//{
////    bool quaBit = getQuaBit(row);
////    const char * seqqua = qua(row, iread);
//    idx count = 0;
//    if( seqqua && *seqqua ) {
//        for(idx i = start; i < end; ++i) {
//            if( qua[i/8]&(((idx)1)<<(i%8)) == 0 ) {
//                ++count;
//            }
//        }
//    }
//    return
//}

void sFilterseq::randomizer2vioseq(sBioseq &sub,const char * output, idx numReads, real noise, idx minValue, idx maxValue, bool isNrevComp, bool isQual, const char * strsettings, const char *strmutations)
{
    sStr tmpfasta_output;
    tmpfasta_output.printf("%s_tmp.fasta",output);
    sFil fp(tmpfasta_output);
    if( fp.ok() ) {
        fp.cut(0);
        randomizer(sub,tmpfasta_output.ptr(),numReads,noise,minValue,maxValue,isNrevComp,isQual,strsettings,strmutations);
        sVioseq2 v;
        v.parseSequenceFile(output, tmpfasta_output.ptr() );
    }
}

void sFilterseq::randomizer(sBioseq &sub,const char * output, idx numReads, real noise, idx minValue, idx maxValue, bool isNrevComp, bool isQual, const char * strsettings, const char *strmutations)
{
    sStr ratioSettings;
    bool isLengthNormalized = false;
    if( strsettings ) {
        ratioSettings.printf("%s", strsettings);
    } else {
//        idx totLength = 0;
//        for(idx i = 0; i < sub.dim(); ++i) {
//            totLength += sub.len(i);
//        }
//        for(idx i = 0; i < sub.dim(); ++i) {
//            real ratio = (real)sub.len(i)/totLength;
//            ratioSettings.printf("%"DEC":%f",i,ratio);
//            if(i)
//                ratioSettings.printf(";");
//        }
        isLengthNormalized = true;
    }
    sFil fp(output);

    if( fp.ok() ) {
        fp.cut(0);
        randomizer(fp,sub,numReads,noise,minValue,maxValue,isNrevComp,isQual,isLengthNormalized,ratioSettings,strmutations);
    }
}

void sFilterseq::randomizer(sFil &QP, sBioseq &sub, idx numReads, real noise, idx minValue, idx maxValue, bool isNrevComp, bool isQual, bool isLengthNormalized, const char * strsettings, const char *strmutations,idx complexityWindow, real complexityEntropy,void * myCallbackParam,callbackType myCallbackFunction )
{

    //From what subject should I generate the reads from------------------------------------//
//    const char * subSet = pForm->value("subset","all");
//    real mutfreq=pForm->rvalue("mutfreq",0.5);
    idx m_progressLast=0,m_progressUpdate=0;
    sVec<idx> subrange;
    sVec<real> pp;pp.resize(sub.dim());pp.set(0);
    real * sus=pp.ptr();//idx ist=0;
    sVec <real> mutfreqRand;
    mutfreqRand.add(sub.dim());
    mutfreqRand.set(0);
    idx accumLength = 0;
    // Parse settings string
    if (sLen(strsettings)){
        sStr strSubSet;
        for(idx ii=0;ii<sub.dim();++ii)
            sus[ii]=0;
        sString::searchAndReplaceSymbols(&strSubSet,strsettings, 0, ";",0,0,true , true, false, true);
        const char * strSet=strSubSet.ptr();
        for (strSet=strSubSet.ptr(); strSet; strSet=sString::next00(strSet)){
            sStr stSub; sVec<idx> ids;
            sString::searchAndReplaceSymbols(&stSub,strSet, 0, ":",0,0,true , true, false, true);
            const char * sbinf=stSub.ptr();
            sString::scanRangeSet(sbinf, 0, &ids, 0, 0, 0);

            sbinf=sString::next00(sbinf);
            real prop=-2;
            if(sbinf)
                prop=atof(sbinf);
            for(idx c_id=0; c_id < ids.dim() ;++c_id){
                if( ids[c_id]>=sub.dim() ){
                    continue;
                }
                sus[ ids[c_id] ]=prop;
                if(isLengthNormalized){
                    accumLength += sub.len( ids[c_id] );
                }
//                ist=1;
            }
        }
        real accprop=0;
        for(idx ii=0;ii<sub.dim();++ii){
            if(sus[ii]!=-2 && sus[ii]){
                if(isLengthNormalized){
                    sus[ii] *= ( (real)sub.len(ii)/accumLength );
                }
                accprop+=sus[ii];
            }
        }
        idx rest=0;
        for(idx ii=0;ii<sub.dim();++ii){
            if(sus[ii]<=0)++rest;
        }
        for(idx ii=0;ii<sub.dim();++ii){
            if(accprop<1 && rest){
                if(sus[ii]==-2)
                    sus[ii]=(real)(1-accprop)/rest;
            }
            else
            {
                if(sus[ii]!=-2 && sus[ii])
                    sus[ii]=sus[ii]/accprop;
                else if(sus[ii]==-2)
                    sus[ii]=0;
            }
            if(ii){
                sus[ii] += sus[ii-1];
            }
        }
    }
    else{
        if(isLengthNormalized){
            for(idx ii=0;ii<sub.dim();++ii){
                accumLength += sub.len(ii);
            }
            for(idx ii=0;ii<sub.dim();++ii){
                sus[ii] = (real)sub.len(ii)/accumLength;
                if(ii) {
                    sus[ii] += sus[ii-1];
                }
            }
        }
    }
//    else
//        ist=1;

    //Should I generate mutations or noise? positions/rate and frequency provided-----------//
//    const char * strvar=pForm->value("mutations");
    sVec< sDic< idx > > varpos;
//    sVec<sVec<idx> >  varmut;varmut.add(sub.dim());

    // Parse mutation string
    if(strmutations)
    {
        varpos.add(sub.dim());
        sStr subfm;
        sString::searchAndReplaceSymbols(&subfm, strmutations, 0, ";",0,0,true , true, false, true);
        const char * sbfpt=subfm.ptr();
        for (sbfpt=subfm.ptr(); sbfpt; sbfpt=sString::next00(sbfpt))
        {
            sStr subidfm,mutIDfmt,tmp;
            sString::searchAndReplaceSymbols(&subidfm, sbfpt, 0, ":",0,0,true , true, false, true);
            sString::searchAndReplaceSymbols(&mutIDfmt, subidfm,0, ">",0,0,true , true, false, true);

            sVec<idx> ids;
            sString::scanRangeSet(mutIDfmt, 0, &ids, 0, 0, 0);

            sString::searchAndReplaceSymbols(&tmp, sString::next00(mutIDfmt.ptr()),0, ",",0,0,true , true, false, true);
            const char * catchsub = tmp.ptr();
            idx varnum=atol(catchsub);
            catchsub=sString::next00(catchsub);
            real mut_freq = atof(catchsub);



            catchsub=sString::next00(subidfm.ptr());
            sStr stsfm,mutPat;
            if(varnum>0 ){
                idx mutfr = 0,mutst=0,maxLen=0;
                sString::searchAndReplaceSymbols(&mutPat, catchsub,0, ",",0,0,true , true, false, true);
                const char * mutpat=mutPat.ptr();
                if(mutpat && strcmp(mutpat,"")!=0){
                    if(mutpat && mutpat[0]=='n')                                    //mutate at a given rate (eg. 0-10 > 1000, 0.5 : n30, 50 ; Next pattern)
                    {                                                               //                            |        |    |     |    |
                        mutfr=atol(++mutpat);                                       //                          subID      | mut freq |  mut. start
                                                                       //                                              mut #     mut. step
                        const char * mutstart = sString::next00(mutPat.ptr());
                        if(mutstart) {
                            mutst = atol(mutstart);
                            if(mutst<0)mutst = 0;
                        }
                    }
                }
                else{                                                                   //if positions are not specified choose positions randomly
                    for(idx ii=0;ii<sub.dim();++ii){
                        if(sub.len(ii)>maxLen)
                            maxLen=sub.len(ii);
                    }

                }
                idx mysubid = 0;
                for( idx is = 0 ; is < ids.dim() ; ++is ) {
                    if( ids[is]>=sub.dim() ){
                        continue;
                    }
                    mysubid = ids[is];
                    mutfreqRand[mysubid] = mut_freq;
                    sDic<idx> * crDic = varpos.ptrx(mysubid);
                    if(mutfr != 0) {
                        for(idx m=mutst;m<sub.len(mysubid)  && varnum>0  && mutfr;m+=mutfr){
                            *crDic->set(&m, sizeof(idx))=(idx)(RAND1*3)+1;
//                            crDic = varpos.ptrx(mysubid);
                            --varnum;
                        }
                    }
                    else{
                        for(idx ii=0; ii<varnum;++ii){
                            crDic = varpos.ptrx(mysubid);
                            idx m=(idx)(RAND1*maxLen);
                            *crDic->set(&m, sizeof(idx))=(idx)(RAND1*3)+1;//varpos[mysubid].vadd(1,(idx)(RAND1*maxLen));
                        }
                    }
                }
            }
        }
    }

//    if(!ist)numReads=0; //if none subset was selected
    sVec<idx> cntV,ranV;
    for(idx ps=0;ps<sub.dim();++ps){
        cntV.vadd(1,0);
        ranV.vadd(1,-(ps*10));
    }

    if ((maxValue < minValue) && (maxValue != -1)){
        idx temp = minValue;
        minValue = maxValue;
        maxValue = temp;
    }
//    idx noisyMutations=0;
    sStr hdr;
    for( idx ii=0; ii<numReads ; ++ii ){
        idx i=-1;
        if(!sLen(strsettings) && !isLengthNormalized)
            i=(idx)(RAND1*sub.dim());
        else{
            idx ps=0;
            real ra=RAND1;
           while(ra > sus[ps] && ps<sub.dim()){
               ++ps;
           }
           i = ps;
        }
        // Here I put the condition

        if(i>=sub.dim())
            i=sub.dim()-1;

        idx subLen=sub.len(i);
        const char * id=sub.id(i);if(id[0]=='>')++id;
//        idx minValue, idx maxValue, idx start,  idx end, const char * strsettings, const char *strmutations)
        idx seqLen;
        if(maxValue==-1)seqLen=subLen;
        else seqLen=minValue+(idx)(RAND1*(maxValue-minValue));
        if(seqLen>subLen)seqLen=subLen;

        idx seqStart=(idx)(RAND1*(subLen - seqLen));

//        if (seqStart < 0) {
//            seqStart = 0;
//        }

        bool doRev=false;
        if(!isNrevComp)
        {
            if(RAND1<0.5)
                doRev=true;
        }
        if(isQual){ hdr.printf(0,"@");}
        else {hdr.printf(0,">");}
        hdr.printf("%"DEC" pos=%"DEC" len=%"DEC,i,doRev?(seqStart+seqLen):(seqStart+1),seqLen);
        if(doRev)hdr.printf(" REV");
        hdr.printf(" ori=%s",id);

        const char * seq=sub.seq(i);
        sStr t; sBioseq::uncompressATGC(&t,seq, seqStart, seqLen,true,0);

        real tocomp=RAND1;
        for(idx ll=0;ll<t.length();++ll)
        {
            char * tt=t.ptr(ll);
            idx curpos=ll+seqStart;
            if(strmutations && varpos[i].dim()){
                    idx * mut=varpos[i].get(&curpos,sizeof(idx));
                    if(mut){
                        if(tocomp<=mutfreqRand[i])
                        {
                            char init=sBioseq::mapRevATGC[(sBioseq::mapATGC[(idx)tt[0]])%4];
                            tt[0]=sBioseq::mapRevATGC[(sBioseq::mapATGC[(idx)tt[0]]+*mut)%4];
                            hdr.printf(" mut=%"DEC"%c>%c",curpos+1,init,tt[0]);
                        }
//                        ++jj;
//                    }
                }
            }
            if(RAND1<=noise){
                tt[0]=sBioseq::mapRevATGC[(sBioseq::mapATGC[(idx)tt[0]]+(idx)(RAND1*3)+1)%4];
//                ++noisyMutations;
            }
        }
        if(doRev){
            idx revll=t.length()-1;
            sStr revt;revt.printf("%s",t.ptr());
            char * rev=revt.ptr(0);
            char * tt=t.ptr(0);
            for(idx ll = 0; ll< t.length(); ++ll, --revll) {
                tt[ll]=sBioseq::mapRevATGC[sBioseq::mapComplementATGC[sBioseq::mapATGC[(idx)rev[revll]]]];
            }
        }
        if( !complexityWindow || (complexityFilter(t.ptr(),t.length(),complexityWindow,complexityEntropy,false)==0) ){
            if( myCallbackFunction ) {
                time_t t0 = time(0);
                real percent = (real)(100*ii)/numReads;
                if( percent > m_progressUpdate || (t0 - m_progressLast) > 60 ) {
                    m_progressUpdate = percent;
                    m_progressLast = t0;
                    myCallbackFunction(myCallbackParam, ii, percent, 100);
                }
            }
            QP.printf("%s\n",hdr.ptr());

            QP.printf("%s\n",t.ptr());
            if(isQual){
                QP.printf("+\n");
                for(idx ll=0 ; ll < (t.length()) ;++ll)
                    QP.printf("A");
                QP.printf("\n");
            }
        }
        else{
            --ii;
        }
    }
//    ::printf("Number of noisy mutations: %"DEC"\n",noisyMutations);

}

/*
idx sFilterseq::posMatrix(idx i, idx j, idx len, idx *mat)
{
    // 8 x 9
    // 0 1  2  3  4  5  6  7  8
    // 9 10 11 12 13 14 15 16 17
    // 18
    idx pos = ((i+1) * (len+1)) + (j+1);
    return mat[pos];
}
*/

#define posMatrix( _v_i, _v_j, _v_len, _v_mat) _v_mat[(((_v_i)+1) * ((_v_len)+1)) + ((_v_j)+1)]


idx sFilterseq::adaptiveAlignment (const char *sub, idx sublen, const char *qry, idx qrylen, idx cfmatch, idx cfmismatch, idx cfindel, idx *resPos, idx *costMatrix, idx *dirMatrix)
{
    idx p, match = 0;
//    idx match2;
    idx i, j;
    idx case1, case2, case3, maxi, maxj;
    idx lenalign = 0;
    for (i = 0; i < sublen+1; ++i){
        costMatrix[i]=0; dirMatrix[i]=0;
    }
    for (i = 0; i < qrylen+1; ++i){
        costMatrix[i*(sublen+1)]=0; dirMatrix[i*(sublen+1)]=0;
    }

    idx qrylenminus1=qrylen-1;
    maxi = 0; maxj = 0;
    for (i = 0; i < qrylen; ++i){
//        char qry1 = sBioseqAlignment::_seqBits(qry, qrylenminus1-i, sBioseqAlignment::fAlignForwardComplement | sBioseqAlignment::fAlignForward);
        idx is = qrylenminus1-i;
//        idx val=(idx)((seq[is/4]>>((is%4)*2)&0x3) ; // this particular base introduces this two bits
        char qry1 = (char)sBioseq::mapComplementATGC[(idx)((qry[is/4]>>((is%4)*2)&0x3))];
        idx ppp=((i+1) * (sublen+1))+1;

        for (j = 0; j < sublen; ++j){
            // p = ((i+1) * (sublen+1)) + (j+1);
            p=ppp+j;
//            char sub1 = sBioseqAlignment::_seqBits(sub, j, 0);
            char sub1 = (char)((sub[j/4]>>((j%4)*2)&0x3)) ;
            match = (sub1 == qry1) ? cfmatch : cfmismatch;

//            if (match != match2){
//                match2 = match;
//            }
            case1 = posMatrix (i-1, j-1, sublen, costMatrix) + match;
            case2 = posMatrix (i-1, j, sublen, costMatrix) + cfindel;
            case3 = posMatrix (i, j-1, sublen, costMatrix) + cfindel;
            if (case1 >= case2 && case1>=case3){ // It comes from the top left diagonal
                costMatrix[p] = case1;
                dirMatrix[p] = 0;
                if (posMatrix(maxi, maxj, sublen, costMatrix) < case1){
                    maxi = i; maxj = j;
                }
            } else if (case2 >= case3){ // It comes from the top
                costMatrix[p] = case2;
                dirMatrix[p] = 1;
            } else { // It comes from the left
                costMatrix[p] = case3;
                dirMatrix[p] = 2;
            }
        }
    }

    // We have maxi and maxj to start local alignment regression
    resPos[1] = maxj;
    resPos[3] = maxi;
    idx direction;
    i = maxi; j = maxj;
    while (posMatrix (i, j, sublen, costMatrix) > 0){
        direction = posMatrix (i, j, sublen, dirMatrix);
        lenalign++;
        if (direction == 0){
            --i; --j;
        }
        else if (direction == 1){
            --i;
        }
        else {
            --j;
        }
    }
    resPos[0] = j+1;
    resPos[2] = i+1;

    return lenalign;
}

// Create a dictionary with id's of a bioseq object
// sDic<idx> myidDictionary
// parseDicBioseq(myidDictionary, Sub)
// idx myrow = getrownum (myidDictionary, "chr7", 4);
bool sFilterseq::parseDicBioseq(sDic <idx> &idDic, sBioseq &sub, sStr *errmsg, bool attemptSubStrings)
{
    sStr aux1, aux2;
//    bool attemptDicSubStrings = (attemptSubStrings && (sub.dim() < 1000000)) ? true : false;
    for(idx ir = 0; ir < sub.dim(); ++ir) {
        aux2.cut(0);
        const char *currId = sub.id(ir);
//        sString::searchAndReplaceSymbols(&dst, currId, sLen(currId), "|", 0, 0, true, true, false, false);
        idx aux2len = sString::copyUntil(&aux2, currId, sLen(currId), " ");
        idx * IDbuf = idDic.get(aux2.ptr(), aux2len);
        if( !IDbuf ) {
            // Create the dictionary of all the parts
            {
                // Get the row of id and divide it by '|'
                aux1.cut(0);
                sString::searchAndReplaceSymbols(&aux1, aux2.ptr(), aux2len, "|", 0, 0, true, true, false, false);
                if (sString::cnt00(aux1) > 1){
                    idx it = 1;
                    for(const char * p = aux1; p; p = sString::next00(p), ++it) {
                        if( it % 2 == 0 ) {
                            // Remove spaces the id
                            aux2.cut(0);
                            aux2len = sString::copyUntil(&aux2, p, 0, " .");

                            // put 'p' in the dictionary
                            IDbuf = idDic.set(aux2.ptr(), aux2len);
                            *IDbuf = ir;
                        }
                    }
                }
                else {
                    // include the whole 'ID' into the dictionary
                    IDbuf = idDic.set(aux2.ptr(), aux2len);
                    *IDbuf = ir;
                }
            }
        };
    }
    if( idDic.dim() == 0  && errmsg) {
        errmsg->printf("Can not create a dictionary from Id's");
        return false;
    }
    return true;
}

idx sFilterseq::getrownum(sDic <idx> &idDic, const char *id, idx idlen)
{
    static sStr aux1, aux2;
    if (!idlen){
        idlen = sLen(id);
    }
    // 1) First try the whole id
    idx *irow = idDic.get(id, idlen);
    if( irow ) {
        return (*irow);
    } else {
        // We couldn't find it !!!!
        // 2) Let's add chr and try one more time
        char chr[11] = "chr";
        idx minlen = sMin < idx > (idlen, 7);
        memcpy(chr + 3, id, minlen);
        chr[3 + minlen] = 0;

        irow = idDic.get(chr, minlen);
        if( irow ) {
            return (*irow);
        }
        // No, we haven't found it
        // 3) Remove spaces and try again
        aux1.cut(0);
        idx aux1len = sString::copyUntil(&aux1, id, idlen, " ");

        irow = idDic.get(aux1.ptr(),aux1len);
        if( irow ) {
            return (*irow);
        }
        // No, we haven't found it
        // Get the row of id and divide it by '|'
        aux2.cut(0);
        sString::searchAndReplaceSymbols(&aux2, aux1.ptr(), aux1len, "|", 0, 0, true, true, false, false);
        if (sString::cnt00(aux2) > 1){
            // 4) If it has '|', we will try all the pairs
            idx ir = 1;
            for(const char * p = aux2; p; p = sString::next00(p), ++ir) {
                if( ir % 2 == 0 ) {
                    irow = idDic.get(p, sLen(p));
                    if( irow ) {
                        return (*irow);
                    }
                }
            }
        }
    }
    return -1;
}
