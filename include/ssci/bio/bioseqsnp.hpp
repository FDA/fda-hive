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
#pragma once
#ifndef sBio_seqsnp_hpp
#define sBio_seqsnp_hpp

#include <ssci/bio/bioseqalign.hpp>
#include <ssci/bio/bioal.hpp>
#include <ssci/bio/ion-bio.hpp>
#include <ssci/bio/sVioAnnot.hpp>
#include <regex.h>
#include <limits.h>

namespace slib
{




    class sBioseqSNP {


    public:

        struct SNPParams {
            idx maxRptIns;
            idx cutEnds;
            idx lenPercCutoff;

            idx disbalanceFR;
            idx minCover;
            real minFreqPercent;
            idx minFreqIgnoreSNP;
            idx snpCompare;
            idx maxLowQua;
            idx useQuaFilter;
            idx rangeSize;
            real minImportantEntropy;
            real maxMissmatchPercCutoff;

            real entrCutoff;
            real noiseCutoffs[6][6];

            real noiseProfileResolution,freqProfileResolution, histProfileResolution,noiseProfileMax;
            idx filterZeros;

            bool countAAs, computeAAs, directionalityInfo, supportedDeletions, collapseRpts;

            SNPParams (){
                sSet(this,0);
            }

            SNPParams operator= (const SNPParams& x){
                maxRptIns = x.maxRptIns;
                cutEnds = x.cutEnds;
                lenPercCutoff = x.lenPercCutoff;

                disbalanceFR = x.disbalanceFR;
                minCover = x.minCover;
                minFreqPercent = x.minFreqPercent;
                minFreqIgnoreSNP = x.minFreqIgnoreSNP;
                snpCompare = x.snpCompare;
                maxLowQua = x.maxLowQua;
                useQuaFilter = x.useQuaFilter;
                rangeSize = x.rangeSize;
                minImportantEntropy = x.minImportantEntropy;
                maxMissmatchPercCutoff = x.maxMissmatchPercCutoff;

                entrCutoff = x.entrCutoff;
                
                memcpy(noiseCutoffs, x.noiseCutoffs, sizeof(noiseCutoffs)*sizeof(real));

                noiseProfileResolution = x.noiseProfileResolution;
                freqProfileResolution = x.freqProfileResolution;
                histProfileResolution = x.histProfileResolution;
                noiseProfileMax = x.noiseProfileMax;
                filterZeros = x.filterZeros;

                countAAs = x.countAAs;
                computeAAs = x.computeAAs;
                directionalityInfo = x.directionalityInfo;
                supportedDeletions = x.supportedDeletions;
                collapseRpts = x.collapseRpts;
                
                return *this;
            }

        };

        enum SNPParamsDefaults {
            snpCompare_DEFAULT = 1,
            noiseCutoffThresholds_NUM = 6,
            noiseCutoffThresholdsIdx_DEFAULT = 4,
        };

        static const real noiseCutoffThresholds[noiseCutoffThresholds_NUM];
        static const real freqProfileResolution;
        static const real histCoverResolution;
        static const real noiseProfileResolution;
            struct ATGCcount
            {
                    int countFwd[4], countRev[4], iqx;
                    ATGCcount()
                        : iqx(0)
                    {
                        sSet(countFwd);
                        sSet(countRev);
                    }
            };

            class PosInDel
            {
                public:
                    struct info
                    {
                            int fwd, rev, vquafwd, vquarev;
                            info()
                            {
                                sSet(this);
                            }
                            void add(info & t_info)
                            {
                                if( t_info.fwd ) {
                                    vquafwd += (vquarev * fwd + t_info.vquafwd * t_info.fwd) / (fwd + t_info.fwd);
                                    fwd += t_info.fwd;
                                }
                                if( t_info.rev ) {
                                    vquarev += (vquarev * rev + t_info.vquarev * t_info.rev) / (rev + t_info.rev);
                                    rev += t_info.rev;
                                }
                            }
                            idx cnt() {
                                return fwd + rev;
                            }
                            real freq(idx cov) {
                                return (real)cnt()/cov;
                            }
                    };

                    sDic<info> insertions;
                    sDic<info> deletions;
                    PosInDel()
                        : insertions(), deletions()
                    {
                    }

                    info * addInsertion(const char * seq, const idx length)
                    {
                        return insertions.setString(seq, length);
                    }

                    info * addInsertion(sStr &seq)
                    {
                        seq.shrink00();
                        return addInsertion(seq.ptr(), seq.length());
                    }

                    info * addDeletion(idx cnt)
                    {
                        return deletions.set((const void *) &cnt, (idx) sizeof(cnt));
                    }

                    idx getDeletionAllele(idx index)
                    {
                        return *(idx*) deletions.id(index);
                    }
                    info * getDeletionInfo(idx index)
                    {
                        return deletions.ptr(index);
                    }

                    const char * getInsertionAllele(idx index)
                    {
                        return (const char *) insertions.id(index);
                    }
                    info * getInsertionInfo(idx index)
                    {
                        return insertions.ptr(index);
                    }
                    const char * serialIn(const char * data)
                    {
                        const char * buf = (const char *) data;
                        const idx sizeofIns = *((const idx *) buf);
                        buf += sizeof(sizeofIns);
                        insertions.serialIn(buf, sizeofIns);
                        buf += sizeofIns;
                        const idx sizeofDel = *((const idx *) buf);
                        buf += sizeof(sizeofDel);
                        deletions.serialIn(buf, sizeofDel);
                        return buf + sizeofDel;
                    }

                    idx serialOut(sMex &buf)
                    {
                        sStr tmp;
                        idx size = insertions.serialOut(tmp);
                        buf.add((const char *) &size, (idx) sizeof(size));
                        buf.add(tmp.ptr(), size);

                        tmp.cut0cut();

                        size = deletions.serialOut(tmp);
                        buf.add((const char *) &size, (idx) sizeof(size));
                        buf.add(tmp.ptr(), size);
                        return buf.pos();
                    }

                    void merge(PosInDel & t_posindel)
                    {
                        idx pLen = 0;
                        for(idx i = 0; i < t_posindel.insertions.dim(); ++i) {
                            const char * key = (const char *) t_posindel.insertions.id(i, &pLen);
                            addInsertion(key, pLen)->add(t_posindel.insertions[i]);
                        }
                        for(idx i = 0; i < t_posindel.deletions.dim(); ++i) {
                            idx * ikey = (idx*) t_posindel.deletions.id(i);
                            addDeletion(*ikey)->add(t_posindel.deletions[i]);
                        }
                    }
                    idx dim()
                    {
                        return insertions.dim() + deletions.dim();
                    }
            };

            class InDels
            {
                private:
                    sDic<PosInDel> _pos;
                    idx _sub_pos[2];

                public:
                    InDels()
                        : _pos()
                    {
                    }

                    idx serialIn(const void * data, idx lenmax)
                    {
                        const char * buf = (const char *) data;
                        idx key_size = 2*sizeof(idx);
                        PosInDel * p = 0;
                        const char * const end = buf + lenmax;
                        while( buf && buf < end ) {
                            p = _pos.set(buf, key_size);

                            if( !p ) {
                                return -1;
                            }
                            buf += key_size;

                            buf = p->serialIn(buf);
                        }
                        return dim();
                    }

                    idx serialOut(sMex &buf)
                    {
                        idx size = 0;
                        for(idx i = 0; i < dim(); ++i) {
                            const void * key = _pos.id(i, &size);
                            buf.add((const char *) key, size);
                            _pos.ptr(i)->serialOut(buf);
                        }
                        return buf.pos();
                    }

                    PosInDel::info * addInsertion(idx i_sub, idx pos, sStr &seq)
                    {
                        seq.shrink00();
                        return addInsertion(i_sub, pos, seq.ptr(), seq.length());
                    }

                    PosInDel::info * addInsertion(idx i_sub, idx i_pos, const char * seq, idx length)
                    {
                        return setInDel(i_sub, i_pos)->addInsertion(seq, length);
                    }
                    PosInDel::info * addDeletion(idx i_sub, idx i_pos, idx cnt)
                    {
                        return setInDel(i_sub, i_pos)->addDeletion(cnt);
                    }

                    PosInDel * setInDel(idx i_sub, idx i_pos)
                    {
                        _sub_pos[0] = i_sub;
                        _sub_pos[1] = i_pos;
                        return _pos.set((const void *) &_sub_pos, (idx) sizeof(_sub_pos));
                    }

                    PosInDel * getInDel(idx i_sub, idx i_pos)
                    {
                        _sub_pos[0] = i_sub;
                        _sub_pos[1] = i_pos;
                        return _pos.get((const void *) &_sub_pos, (idx) sizeof(_sub_pos));
                    }

                    sDic<PosInDel::info> * getInsertions(idx i_sub, idx i_pos)
                    {
                        PosInDel * my_indels = getInDel(i_sub, i_pos);
                        return my_indels ? &my_indels->insertions : 0;
                    }

                    sDic<PosInDel::info> * getDeletions(idx i_sub, idx i_pos)
                    {
                        PosInDel * my_indels = getInDel(i_sub, i_pos);
                        return my_indels ? &my_indels->deletions : 0;
                    }
                    idx dim()
                    {
                        return _pos.dim();
                    }

                    void merge(InDels & t_indels)
                    {
                        idx * pos = 0;
                        for(idx i = 0; i < t_indels.dim(); ++i) {
                            pos = (idx*) t_indels._pos.id(i);
                            setInDel(*pos, *(pos + 1))->merge(t_indels._pos[i]);
                        }
                    }

            };

        struct SNPFreq{
                int atgcFwd[6], atgcRev[6];
                int ventr,vsnpentr,vquafwd, vquarev;

                enum {ENTRMAX=255};
                inline idx entr(idx inu){return (ventr>>(8*inu))&0xFF;}
                inline idx snpentr(idx inu){return (vsnpentr>>(8*inu))&0xFF;}
                inline idx quaFwd(){return vquafwd;}
                inline idx quaRev(){return vquarev;}
                idx quaTot(){return quaRev() + quaFwd();}

                idx getAvQua(){return acgtCoverage()?(quaTot())/acgtCoverage():0;}

                void setEntr(idx val, idx inu){ventr&=~(0xFF<<(8*inu)); ventr|=(val&0xFF)<<(8*inu);}
                void setSnpentr(idx val, idx inu){vsnpentr&=~(0xFF<<(8*inu)); vsnpentr|=(val&0xFF)<<(8*inu);}
                void setQuaFwd(idx val){ vquafwd=val;}
                void setQuaRev(idx val){ vquarev=val;}

                idx coverage()
                {
                    idx totIn=0;
                    for(idx i=0; i<sDim(atgcFwd);++i){totIn+=atgcFwd[i];}
                    for(idx i=0; i<sDim(atgcRev);++i){totIn+=atgcRev[i];}
                    return totIn;
                }
                idx acgtCoverage()
                {
                    idx tot = 0 ;
                    for(idx i=0; i<4;++i){tot+=atgcFwd[i];}
                    for(idx i=0; i<4;++i){tot+=atgcRev[i];}
                    return tot;
                }
                inline idx inFwd(){return atgcFwd[4];}
                inline idx inRev(){return atgcRev[4];}
                inline idx delFwd(){return atgcFwd[5];}
                inline idx delRev(){return atgcRev[5];}
                inline idx insertions(){return inFwd()+inRev();}
                inline idx deletions(){return delFwd()+delRev();}
                idx readCoverage()
                {
                    idx tot = 0 ;
                    for(idx i=0; i<4;++i){tot+=atgcFwd[i];}
                    for(idx i=0; i<4;++i){tot+=atgcRev[i];}
                    tot+=deletions();
                    return tot;
                }

                real minFreq(idx relativeLetter)
                {
                    idx tot = acgtCoverage();
                    tot = tot?tot:1;
                    real min=1, vv = 0;
                    for (idx ic=0; ic<4 ; ++ic ) {
                        vv=(ic==relativeLetter) ? 0 : ((real)(atgcFwd[ic]+atgcRev[ic]))/tot ;
                        if(vv < min) min = vv;
                    }
                    return min;
                }
                real maxFreq(idx relativeLetter)
                {
                    idx tot = acgtCoverage();
                    tot = tot?tot:1;
                    real max=0, vv = 0;
                    for (idx ic=0; ic<4 ; ++ic ) {
                        vv=(ic==relativeLetter) ? 0 : ((real)(atgcFwd[ic]+atgcRev[ic]))/tot ;
                        if(vv > max) max = vv;
                    }
                    return max;
                }


                void add(SNPFreq * other){
                    idx totIn=0,totNew=0;
                    for(idx i=0; i<sDim(atgcFwd);++i){totIn+=atgcFwd[i];totNew+=other->atgcFwd[i];atgcFwd[i]+=other->atgcFwd[i];}
                    for(idx i=0; i<sDim(atgcRev);++i){totIn+=atgcRev[i];totNew+=other->atgcRev[i];atgcRev[i]+=other->atgcRev[i];}
                    setQuaFwd(quaFwd()+other->quaFwd());
                    setQuaRev(quaRev()+other->quaRev());


                    if(totIn+totNew){
                        for ( idx in=0; in<4; ++in){
                            setEntr((entr(in)*totIn+other->entr(in)*totNew)/(totIn+totNew),in);
                            setSnpentr((snpentr(in)*totIn+other->snpentr(in)*totNew)/(totIn+totNew),in);
                        }
                    }
                }
        };

        struct ProfilePosInfo{
                sBioal::LenHistogram lenHistogram;
                int quaFwdATGC[4],quaRevATGC[4];
                idx score;


                void sum(ProfilePosInfo * other){
                    lenHistogram.cntAl+=other->lenHistogram.cntAl;
                    lenHistogram.cntLeft+=other->lenHistogram.cntLeft;
                    lenHistogram.cntRead+=other->lenHistogram.cntRead;
                    lenHistogram.cntRight+=other->lenHistogram.cntRight;
                    lenHistogram.lenAnisotropy+=other->lenHistogram.lenAnisotropy;
                    lenHistogram.cntSeq+=other->lenHistogram.cntSeq;
                    lenHistogram.maxRight=sMax(lenHistogram.maxRight,other->lenHistogram.maxRight);
                    lenHistogram.maxLeft=sMax(lenHistogram.maxLeft,other->lenHistogram.maxLeft);
                    score+=other->score;
                    for ( idx in=0; in<4; ++in){
                        quaFwdATGC[in]+=other->quaFwdATGC[in];
                        quaRevATGC[in]+=other->quaRevATGC[in];
                    }
                }
                idx coverage()
                {
                    return lenHistogram.cntAl;
                }
        };


        struct ProfileAAInfo {
                int aa[22];
                int ref;
                int pos;
                void sum(ProfileAAInfo * other){
                    for ( idx in=0; in<sDim(aa); ++in){
                        aa[in]+=other->aa[in];
                        aa[in]+=other->aa[in];
                    }
                }
                idx coverage()
                {
                    idx totIn=0;
                    for(idx i=0; i<sDim(aa);++i){totIn+=aa[i];}
                    return totIn;
                }
                ProfileAAInfo(){sSet(this,0);}
        };

        struct ProfileExtraInfo {
            sVec < sDic < sBioseqSNP::ATGCcount > > EntroMap;
        };

        struct SNPminmax {
                idx bucket_size, last_valid_bucket, max_pos, min_pos, isub, * multAlPosMatch,num_of_points,tot_length;
                real maxFreq[4], minFreq[4], * curFreq;

                SNPFreq freqMin,freqMax;
                ProfilePosInfo outInfoMin,outInfoMax;
                struct posTot{
                        idx tot_withInDelsMin;
                        idx tot_withInDelsMax;
                };
                struct posCnts {
                        idx totR,totF,total,total_withIndels;
                };
                posTot info;
                posCnts posCntMin,posCntMax, * posCntCur;
                SNPParams * SP;
                const char * subseq;
                sStr * out,* outInfo;
                bool isOutFirstLine,isPrint;

                SNPminmax(){
                    reset();
                    num_of_points = 1000;
                    out = 0;outInfo = 0;multAlPosMatch = 0;SP = 0;
                    bucket_size = 0;
                    last_valid_bucket = -1;
                    subseq = 0;tot_length = 0;
                    isub = sIdxMax;
                    isPrint = false;
                }
                void reset_freqs() {
                    reset_min();
                    reset_max();
                }
                void reset_posCnt(){
                    posCntMin.total = sIdxMax;posCntMin.totR = sIdxMax;posCntMin.totF = sIdxMax;posCntMin.total_withIndels = sIdxMax;
                    sSet(&posCntMax,0);
                }
                void reset_min() {
                    freqMin.ventr = INT_MAX;
                    freqMin.vsnpentr = INT_MAX;
                    freqMin.vquafwd = INT_MAX;
                    freqMin.vquarev = INT_MAX;
                    for(idx i = 0 ; i < 6 ; ++i ){
                        freqMin.atgcFwd[i] = 0;
                        freqMin.atgcRev[i] = 0;
                    }
                    minFreq[0] = minFreq[1] = minFreq[2] = minFreq[3] = REAL_MAX;
                    min_pos=0;
                }
                void reset_max() {
                    sSet(&freqMax);
                    sSet(&maxFreq);
                    max_pos=0;
                }

                void reset_Info() {
                    reset_minInfo();
                    reset_maxInfo();
                }
                void reset_minInfo() {
                    outInfoMin.lenHistogram.cntAl = sIdxMax;
                    outInfoMin.lenHistogram.cntFailed = sIdxMax;
                    outInfoMin.lenHistogram.cntLeft = sIdxMax;
                    outInfoMin.lenHistogram.cntRead = sIdxMax;
                    outInfoMin.lenHistogram.cntRight = sIdxMax;
                    outInfoMin.lenHistogram.cntSeq = sIdxMax;
                    outInfoMin.lenHistogram.lenAnisotropy = sIdxMax;
                    for(idx i = 0 ; i < 4 ; ++i ){
                        outInfoMin.quaFwdATGC[i] = INT_MAX;
                        outInfoMin.quaRevATGC[i] = INT_MAX;
                    }
                }
                void reset_maxInfo() {
                    sSet(&outInfoMax);
                }

                void reset() {
                    reset_freqs();
                    reset_Info();
                    reset_posCnt();
                }

                SNPminmax * initBucket (idx length, idx start = 0) {
                    tot_length=length;
                    bucket_size = length/num_of_points;
                    last_valid_bucket = start/tot_length - 1;
                    if( bucket_size < 10 ) {
                        return 0;
                    }
                    return this;
                }
                void updateQua(SNPFreq & cfreq, idx new_tot) {
                    if( cfreq.acgtCoverage() ) {
                        real qua_correction = (real) new_tot/cfreq.acgtCoverage() ;

                        cfreq.setQuaFwd(qua_correction * cfreq.quaFwd());
                        cfreq.setQuaRev(qua_correction * cfreq.quaRev());
                    }
                }
                inline void updateMinMaxQua(SNPFreq * line) {
                    if( line->acgtCoverage() && ( freqMax.getAvQua() < line->getAvQua() || !freqMax.quaTot() ) ) {
                        real qua_correction = 1;
                        if( freqMax.acgtCoverage() ) {
                            qua_correction = (real) freqMax.acgtCoverage() / line->acgtCoverage();
                        }

                        freqMax.setQuaFwd(qua_correction * line->quaFwd());
                        freqMax.setQuaRev(qua_correction * line->quaRev());
                    }


                    if( line->acgtCoverage() && ( freqMin.getAvQua() > line->getAvQua() || !freqMin.quaTot() ) ) {
                        real qua_correction = 1;
                        if( freqMin.acgtCoverage() ) {
                            qua_correction = (real) freqMin.acgtCoverage() / line->acgtCoverage();
                        }
                        freqMin.setQuaFwd(qua_correction * line->quaFwd());
                        freqMin.setQuaRev(qua_correction * line->quaRev());
                    }
                }
                inline void updateMinMaxACGTFreq(SNPFreq * line, idx relativeLetter, idx tot) {
                    idx v; real vv;
                    for(idx ic = 0; ic < 4; ++ic) {
                        v = line->atgcFwd[ic] + line->atgcRev[ic];
                        vv = (tot != 0 && ic != relativeLetter) ? (((real) v) / tot) : 0;
                        if( vv > maxFreq[ic] ) {
                            maxFreq[ic] = vv;
                        }
                        if( vv < minFreq[ic] ) {
                            minFreq[ic] = vv;
                        }
                    }
                }
                inline void updateMinMaxTotals(idx tot,idx withIndels_tot,idx totF, idx totR) {
                    if( posCntMax.total < tot) {
                        posCntMax.total = tot;
                        posCntMax.totF = totF;
                        posCntMax.totR = totR;
                    }
                    if( posCntMax.total_withIndels < withIndels_tot)
                        posCntMax.total_withIndels = withIndels_tot;

                    if( posCntMin.total > tot) {
                        posCntMin.total = tot;
                        posCntMin.totF = totF;
                        posCntMin.totR = totR;
                    }
                    if( posCntMin.total_withIndels > withIndels_tot)
                        posCntMin.total_withIndels = withIndels_tot;
                }
                inline void updateMinMaxEntropy(SNPFreq * line) {
                    idx l_entr = 0,l_snp_entr = 0;
                    for(idx in = 0; in < 4; ++in) {
                        l_entr = line->entr(in);
                        l_snp_entr = line->snpentr(in);
                        if( freqMax.entr(in) < l_entr ) {
                            freqMax.setEntr(l_entr, in);
                        }
                        if( freqMax.snpentr(in) < l_snp_entr ) {
                            freqMax.setSnpentr(l_snp_entr, in);
                        }

                        if( freqMin.entr(in) > l_entr ) {
                            freqMin.setEntr(l_entr, in);
                        }
                        if( freqMin.snpentr(in) > l_snp_entr ) {
                            freqMin.setSnpentr(l_snp_entr, in);
                        }
                    }
                }
                inline void updateMinMaxInDels(SNPFreq * line) {
                    if( freqMax.insertions() < line->insertions() ) {
                        freqMax.atgcFwd[4] = line->inFwd();
                        freqMax.atgcRev[4] = line->inRev();
                    }
                    if( freqMax.deletions() < line->deletions() ) {
                        freqMax.atgcFwd[5] = line->delFwd();
                        freqMax.atgcRev[5] = line->delRev();
                    }
                    if( freqMin.insertions() > line->insertions() ) {
                        freqMin.atgcFwd[4] = line->inFwd();
                        freqMin.atgcRev[4] = line->inRev();
                    }
                    if( freqMin.deletions() > line->deletions() ) {
                        freqMin.atgcFwd[5] = line->delFwd();
                        freqMin.atgcRev[5] = line->delRev();
                    }
                }
                void updateMinMax(SNPFreq * line, idx cur_pos, idx relativeLetter, idx tot, idx withIndels_tot,idx totF,idx totR) {

                    if( tot > posCntMax.total ) {
                        max_pos = cur_pos;
                        updateQua(freqMax, line->acgtCoverage());
                        for(idx ic = 0; ic < 4; ++ic) {
                            freqMax.atgcFwd[ic] = line->atgcFwd[ic];
                            freqMax.atgcRev[ic] = line->atgcRev[ic];
                        }
                    }
                    if( tot < posCntMin.total ) {
                        min_pos = cur_pos;
                        updateQua(freqMin, line->acgtCoverage());
                        for(idx ic = 0; ic < 4; ++ic) {
                            freqMin.atgcFwd[ic] = line->atgcFwd[ic];
                            freqMin.atgcRev[ic] = line->atgcRev[ic];
                        }
                    }

                    updateMinMaxQua(line);
                    updateMinMaxTotals(tot,withIndels_tot,totF,totR);
                    updateMinMaxACGTFreq(line,relativeLetter,tot);
                    updateMinMaxInDels(line);
                    updateMinMaxEntropy(line);
                };

                idx outCSV()
                {
                    isPrint = true;
                    if(!posCntMax.total) {
                        freqMin = freqMax;
                        outInfoMin = outInfoMax;
                        posCntMin = posCntMax;
                        for(idx i = 0 ; i < sDim(minFreq) ; ++i ) minFreq[i] = 0;
                    }

                    SNPFreq * outFirst = &freqMin, *  outSecond = &freqMax;

                    ProfilePosInfo * outInfoFirst = &outInfoMin, *  outInfoSecond = &outInfoMax;

                    idx * first_pos = &min_pos, * second_pos = &max_pos;
                    idx first_tot = info.tot_withInDelsMin, second_tot = info.tot_withInDelsMax;
                    if( !freqMax.coverage() ){
                        idx cur_bucket_size = tot_length - bucket_size*(last_valid_bucket+1);
                        if( cur_bucket_size > bucket_size )
                            cur_bucket_size = bucket_size;
                        min_pos = cur_bucket_size/3 + bucket_size*(last_valid_bucket+1);
                        max_pos = 2*cur_bucket_size/3 + bucket_size*(last_valid_bucket+1);
                        outFirst = &freqMax;
                    }
                    idx t_filt_zeros = SP->filterZeros;
                    SP->filterZeros = 0;

                    for(idx i = 0 ; i < sDim(minFreq) ; ++i ) {
                        if( minFreq[i] == REAL_MAX) minFreq[i] = 0;
                    }
                    if(*first_pos < * second_pos){
                        curFreq = &minFreq[0];
                        posCntCur = &posCntMin;
                        snpOutTable(out,outInfo,0,0,isub,subseq,outFirst,outInfoFirst,0,*first_pos,1,SP,multAlPosMatch, this);
                    }
                    info.tot_withInDelsMin = second_tot;

                    curFreq = &maxFreq[0];
                    posCntCur = &posCntMax;
                    snpOutTable(out,outInfo,0,0,isub,subseq,outSecond,outInfoSecond,0,*second_pos,1,SP,multAlPosMatch, this);
                    if(*first_pos > * second_pos){
                        info.tot_withInDelsMin = second_tot;
                        info.tot_withInDelsMin = first_tot;
                        curFreq = &minFreq[0];
                        posCntCur = &posCntMin;
                        snpOutTable(out,outInfo,0,0,isub,subseq,outFirst,outInfoFirst,0,*first_pos,1,SP,multAlPosMatch, this);
                    }
                    SP->filterZeros = t_filt_zeros;
                    isPrint = false;
                    reset();
                    return 1;
                }
        };


        struct SNPRecord {
            idx iSub;
            udx position;
            char letter;
            char consensus;
            idx atgc[4];
            idx indel[2];
            idx countFwd;
            idx countRev;
            idx qua;
            real entrScaled;
            real snpentrScaled;

            enum {
                symbol_First = 0,
                symbol_FirstBase = symbol_First,
                symbol_A = symbol_FirstBase,
                symbol_C,
                symbol_G,
                symbol_T,
                symbol_LastBase = symbol_T,
                symbol_Ins,
                symbol_Del,
                symbol_Last = symbol_Del,
                symbol_Dim
            } eAlphabet;

             SNPRecord()
             {
                 sSet(this);
             }

             void reset()
             {
                 sSet(this);
             }

            inline bool isMatch() const {return coverage() && letter==consensus;};
            inline idx coverage() const {return countFwd+countRev;}

            inline char relativeLetterChar(idx snpCompare) const {return snpCompare == 0 ? consensus : letter;}

            inline idx relativeLetterIndex(idx snpCompare) const {return sBioseq::mapATGC[(idx)relativeLetterChar(snpCompare)];}

            inline real freq(idx i, idx snpCompare=snpCompare_DEFAULT) const
            {
                return (i == relativeLetterIndex(snpCompare)) ? 0 : freqRaw(i);
            }

            inline real freqRaw(idx i) const
            {
                assert (i >= 0 && i < alphabetSize() );

                switch( i ) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        return ((real)atgc[i]) / (coverage() ? coverage() : 1);
                    case 4:
                    case 5:
                        return ((real)indel[i-4]) / (coverage() ? coverage() : 1);
                }

                return 0;
            }

            static inline  idx alphabetSize() { return symbol_Dim; }

            static inline  bool isIndexABase(idx i) { return i<=symbol_LastBase && i>=symbol_FirstBase; }

            static inline  bool isIndexAnIndel(idx i) { return i==symbol_Ins || i>=symbol_Del; }

            inline idx getMutations(real thrsld, sVec<idx> * letter_res = 0, sVec<real> * freq_res = 0, idx snpCompare=snpCompare_DEFAULT) const
            {
                idx cnt = 0;
                real c_freq = 0;
                for (idx i = 0 ; i < alphabetSize() ; ++i)
                {
                    c_freq = freq(i,snpCompare);
                    if( c_freq >= thrsld ) {
                        ++cnt;
                        if( letter_res )
                            letter_res->vadd(1,i);
                        if( freq_res )
                            freq_res->vadd(1,c_freq);
                    }
                }
                return cnt;
            }

            void printCSV(sStr &s, idx snpCompare=snpCompare_DEFAULT) const;


            void printCSV_withSubject(sStr &s, idx snpCompare=snpCompare_DEFAULT) const;

            bool parseCSV(const char *buf, const char *bufend, idx icolMax=0, bool withSub = false);

            bool parseCSV_withSubject(const char *buf, const char *bufend, idx icolMax=0);
        };
        struct startEndAnnot {
                idx start, end;
                startEndAnnot(){start=end=-1;};
        };
        struct ionRange {
                idx start, end;
                bool forward;
                sStr proteinId;
                idx recordIndex;
                sVec < startEndAnnot > connectedRanges;
                ionRange() {start=end=recordIndex=-1; forward=true; proteinId.cut(0); connectedRanges.empty();}
        };

        struct AnnotAlRange{
            idx iRec, iSub, Start, End, iAlStart, iAlEnd;
            bool getStrand(void) {return Start <= End;}
            void reverse(void){idx t = Start; Start = End; End = t;}
            idx getStart(void){return getStrand()?Start:End;}
            idx getEnd(void){return !getStrand()?Start:End;}
            idx getLength(void) { return getEnd() - getStart() +1;}
            idx iAlDim(void) {return iAlEnd - iAlStart;}
            AnnotAlRange(){
                sSet(this,0);
            }
        };
        static idx snpCountSingleSeq(SNPFreq * freq, ProfilePosInfo * pinf, ProfileAAInfo * ainf, idx substart, idx sublen,  const char * sub, InDels  & my_indels,
            idx subbuflen, const char * qry, idx qrylen,const char * qua,
            bool quabit, sBioseqAlignment::Al * hdr, idx * m, SNPParams * SP, idx qrpt, ProfileExtraInfo * extraInf,
            idx idQry=0, sBioseqSNP::ionRange * range=0);
        static idx snpCountPosInfo(SNPFreq * freq, ProfilePosInfo * pinf, const char * subseq, idx substart, idx sublen, SNPParams * SP, ProfileExtraInfo * extraInf );
        static idx * tryAlternativeWay(sIonWander * myWander, const char * orignal_id, idx * recDim);
        static idx snpCleanTable( SNPFreq * freq, const char * subseq, idx substart, idx sublen, SNPParams * SP) ;
        static idx snpOutTable(sStr * out, sStr * xinf, sStr * aainf, sStr * cons, idx isub, const char * subseq, SNPFreq * freq, ProfilePosInfo * pinf, ProfileAAInfo * ainf, idx substart, idx sublen, SNPParams * SP , idx * multAlPosMatch, SNPminmax * MinMaxParams = 0 , sIonAnnot * ionAnnot=0);

        static idx snpOutTable_version2( const char * subseq,SNPFreq * freq, idx substart, idx sublen, SNPParams * SP , sIonAnnot * iannot );

        static idx snpComputeNoiseThresholds(real noiseProfileResolution, sDic < sVec < idx > > * noiseProfile, real * ctof, idx lenct, sVec<sVec<real> > * pct);


        struct HistogHistog { sDic <idx > countForCoverage[4]; };
        static idx snpOutHistog(sVec <HistogHistog> & histogramCoverage, real step, sFil & histProf,idx iSub=0, bool printHeader = true );
        static idx snpCountNoise(SNPFreq * freq, const char * subseq,  idx substart, idx sublen,  sDic < sVec < idx > > * noiseProfile, real noiseProfileResolution, real noiseProfileMax, sVec < HistogHistog > * histogramCoverage = 0, idx minCoverage = 0);
        static idx snpOutNoise(sStr * out, sStr * noiseCuts, real noiseProfileResolution, sDic < sVec < idx > > * noiseProfile, real Ctof=0, sDic<sVec<real> > * pct=0, idx iSub=0, bool printHeaders = true);
        static idx snpComputeIntegrals( sDic < sVec < idx > > & noiseProfile, sVec<idx> & noiseSum, sDic<sVec<real> > & noiseThreshold, real noiseProfileResolution, const real * Ctof, idx Ctof_size );
        static idx snpSumNoisePrint(sStr * out, idx iSub, real noiseProfileResolution, sDic < sVec < idx > > & noiseProfile, sVec<idx> &noiseSum, bool printHeaders);
        static idx snpSumNoise( sDic < sVec < idx > > & noiseProfile, sVec<idx> &noiseSum, real noiseProfileResolution ){ return snpSumNoisePrint(0,0,noiseProfileResolution,noiseProfile,noiseSum,0); };
        static idx snpOutIntegrals(sStr & out, sDic<sVec<real> > & integrals, idx iSub, bool printHeaders);

            struct ProfileSNPcallParams
            {
                    idx isORF, nsSNVs, consensusAAMode;
                    real minRngFreq, maxRngFreq;
                    bool codonScale;
                    bool minmax;
                    sBioseq::EBioAAcode AAcode;
                    real snpCallCutoff;
                    idx hideSubjectName, iSub;
                    sVec<sVioAnnot> *annotList;
                    sIonWander * iWander;
                    sIonWander * iWanderComplex;
                    FILE * outF;
                    ProfileSNPcallParams()
                    {
                        sSet(this, 0);
                    }
            };
        static idx snpCalls(sFil * prof, const char * subseq, const char * subName, idx substart, idx sublen,idx start,idx end,idx cnt, sStr * snpOut,ProfileSNPcallParams * SPC,idx resolution, idx rsID);

        static idx traverserCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist );
        static bool createAnnotationIonRangeQuery( sStr & qry, const char * seqID, const char * record );
        static bool createAnnotationSearchIonRangeQueries(sStr & qry1 , sStr & qry2, const char * seqID, const char * start="start", const char * end="$end" );
        static idx launchIonAnnot(sIonWander * wander, sIonWander * wander2, sVec < ionRange > * rangeDic, idx position, const char * sequenceIdFromProfiler);

        struct ProfileStat {
                idx reflen;
                idx totalGapLength;
                idx totalContigLength;
                real averageContigCoverage;
                real averageGapCoverage;
                idx totalGapsNumber;
                idx totalContigsNumber;
                idx totalMatches;
                float gapsPart;
                float contigsPart;

                void initilize () {
                    reflen = 0;
                    totalGapLength= 0;
                    totalContigLength=0;
                    averageContigCoverage=0;
                    averageGapCoverage=0;
                    totalGapsNumber=0;
                    totalContigsNumber=0;
                    gapsPart=0.0;
                    contigsPart=0.0;
                }
        };
        struct ProfileGap{ bool hasCoverage; idx start , end , averageCoverage, length;};
        static idx snpDetectGaps(ProfileStat * ps, sVec < ProfileGap > * pg, sFil * prof, idx sublen, idx gapWindowSize, idx gapThreshold, idx minGapLength,idx iSub=0);

        static idx protSeqGeneration(const char * subseq, const char * subName,idx sublen,sStr * seqOut);

        enum eProfileIteratorFlags {
                    ePIskipGaps    =    0x01,
                    ePIreplaceGaps =    0x02,
                    ePIsplitOnGaps =    0x03,
                    eCollapseConsecutiveDeletions = 0x04
                };


        struct ParamsProfileIterator{
            sStr * str;
            idx flags;
            SNPRecord prev_rec;
            regex_t * regp;
            idx isCoverageThrs,coverageThrs,pageRevDir;
            idx alCol;
            real consensusThrs;
            const char * id;
            FILE * outF;
            real threshold;
            sStr chrName;
            const char * seq;

            idx wrap,iSub;
            const char * current_row, * end_row;

            void * userPointer;
            idx userIndex;

            ParamsProfileIterator(sStr * lstr=0){
                sSet(this,0);
                str=lstr;
                chrName.cut(0);
            }
            idx get_gap_flags() {
                return flags&3;
            }
            void set_gap_flags(idx t_flg) {
                flags = (flags&(~(idx)3))|(t_flg&3);
            }
        };

        static idx snpOutConsensus(SNPRecord  * rec,ParamsProfileIterator * params,idx iNum);
        static idx getNextRecord(ParamsProfileIterator * param, SNPRecord & rec, idx i_pos, idx i_last_valid);
        typedef idx (*typeCallbackIteratorFunction)(SNPRecord *rec, ParamsProfileIterator * param, idx iNum);
        static idx iterateProfile(sFil * profile,idx subLen,idx * piVis, idx start, idx cnt, sBioseqSNP::typeCallbackIteratorFunction callbackFunc,ParamsProfileIterator * callbackParam, bool isProfVCF = false);
        static const char * binarySearchReference(const char * buf, const char * bufend, idx iSub, bool wantEnd = false);
        static const char * binarySearchReferenceNoise(const char * buf, const char * bufend, idx iSub,bool wantEnd);

        static idx printCSV(sBioseqSNP::SNPRecord * snpRecord, sBioseqSNP::ParamsProfileIterator * params, idx iNum = 0);

        static idx aminoacidDecode( const char * seq, idx position, ionRange * ranges,char * AA, idx * threeCodeOffset=0) ;

        static idx baseFrameDecode( idx &base, idx position, ionRange * ranges,idx & AA, idx * threeCodeOffset=0);

        static const char * SNPRecordNext(const char *buf, SNPRecord *rec, const char *bufend);
        static const char * SNPRecordPrevious(const char *buf, SNPRecord *rec, const char *bufstart);

        static const char * SNPConcatenatedRecordNext(const char *buf, SNPRecord *rec, const char *bufend,idx iSub = 0 , idx icolMax=0);
        static const char * SNPConcatenatedRecordPrevious(const char *buf, SNPRecord *rec, const char *bufstart,idx iSub = 0, idx icolMax=0);


        static bool snpNoiseCutoffsFromIntegralCSV(const char *buf, const char *bufend, real noiseCutoffs[6][6], idx thresholdIdx=noiseCutoffThresholdsIdx_DEFAULT);
        static const char * snpConcatenatedNoiseCutoffsFromIntegralCSV(const char *buf, const char *bufend, real noiseCutoffs[6][6], idx iSub = 0, idx thresholdIdx=noiseCutoffThresholdsIdx_DEFAULT, idx * piSub = 0);
        static idx noiseProfileFromCSV(const char *buf, const char *bufend, sDic< sVec<idx> > * noiseProfile, real noiseProfileResolution);
        static idx noiseIntegralFromCSV(const char *buf, const char *bufend, sDic< sVec<idx> > * noiseProfile, real noiseProfileResolution);
        static idx histogramProfileFromCSV(const char *buf, const char *bufend, sVec<HistogHistog> * histProfile, real noiseProfileResolution);
        static const char * noiseProfileFreqFromConcatenatedCSV(const char * buf, const char * bufend, const char * bufNext, idx * iSub, real * fr, idx noise[6][6]);
        static const char * noiseProfileSubFromConcatenatedCSV(const char * buf, const char * bufend, const char * bufNext, idx * iSub);
        static idx integralFromProfileCSV(const char *buf, const char *bufend, sDic<sVec<real> > & noiseThreshold, real noiseProfileResolution, const real * Ctof = noiseCutoffThresholds , idx Ctof_size = noiseCutoffThresholds_NUM );
    };

    class sSNPRecordIter: public sIter<const sBioseqSNP::SNPRecord*, sSNPRecordIter>
    {
    protected:
        const char *_buf, *_bufend;
        bool _buf_concatenated;
        idx _i;
        sBioseqSNP::SNPRecord _rec;
        bool _validRec;
        idx _snpCompare;

        inline bool isEof() const {return !_buf || _buf+1 >= _bufend || !*_buf;}

        inline void readRec()
        {
            _validRec = true;
            if( isEof() ) {
                _validRec = false;
            } else {
                _buf = _buf_concatenated ? sBioseqSNP::SNPConcatenatedRecordNext(_buf, &_rec, _bufend, _rec.iSub) : sBioseqSNP::SNPRecordNext(_buf, &_rec, _bufend);
                if( unlikely(!_buf) ) {
                    _validRec = false;
                }
            }
        }

        void init(const char *buf, const char *bufend, bool concatenated, idx snpCompare)
        {
            _buf = buf;
            _bufend = !buf ? NULL : bufend ? bufend : buf + strlen(buf);
            _buf_concatenated = concatenated;
            _i = 0;
            _snpCompare = snpCompare;
            _rec.reset();
            readRec();
        }

    public:
        inline void requestData_impl() {}
        inline void releaseData_impl() {}
        inline bool readyData_impl() const { return true; }
        inline bool validData_impl() const { return _validRec; }
        inline idx pos_impl() const { return _i; }
        inline idx segmentPos_impl() const { return _rec.position; }
        sSNPRecordIter(const char *buf = NULL, const char *bufend = NULL, bool concatenated = true, idx snpCompare=sBioseqSNP::snpCompare_DEFAULT) { init(buf, bufend, concatenated, snpCompare); }
        sSNPRecordIter(const sFil *f, bool concatenated = true, idx snpCompare=sBioseqSNP::snpCompare_DEFAULT) { init(f->ptr(), f->last(), concatenated, snpCompare); }
        sSNPRecordIter(const sFil &f, bool concatenated = true, idx snpCompare=sBioseqSNP::snpCompare_DEFAULT) { init(f.ptr(), f.last(), concatenated, snpCompare); }

        sSNPRecordIter(const sSNPRecordIter &rhs): _buf(rhs._buf), _bufend(rhs._bufend), _buf_concatenated(rhs._buf_concatenated), _i(rhs._i), _rec(rhs._rec), _validRec(rhs._validRec), _snpCompare(rhs._snpCompare) {}

        inline sSNPRecordIter* clone_impl() const { return new sSNPRecordIter(*this); }
        inline sSNPRecordIter& operator++()
        {
            ++_i;
            readRec();
            return *this;
        }
        inline bool equals_impl(const sSNPRecordIter &rhs) const { return _buf == rhs._buf; }
        inline bool lessThan_impl(const sSNPRecordIter &rhs) const { return _buf - rhs._buf; }
        inline bool greaterThan_impl(const sSNPRecordIter &rhs) const { return _buf - rhs._buf; }
        inline const sBioseqSNP::SNPRecord* dereference_impl() const { return &_rec; }
    };

    struct sSNPCSVFreqIterParams
    {
        sBioseqSNP::SNPParams _snpparams;

        real _maxThreshold;
        idx _thresholdAutoTarget;

        void init(const sBioseqSNP::SNPParams * snpparams = NULL)
        {
            if (snpparams) {
                if (snpparams != &_snpparams)
                    _snpparams = *snpparams;
            } else {
                _snpparams.snpCompare = sBioseqSNP::snpCompare_DEFAULT;
                _snpparams.noiseCutoffs[0][0] = -1;
            }
            _maxThreshold = 1;
            _thresholdAutoTarget = -1;
        }

        sSNPCSVFreqIterParams(const sBioseqSNP::SNPParams * snpparams = NULL) { init(snpparams); }
        sSNPCSVFreqIterParams(const sSNPCSVFreqIterParams &rhs)
        {
            if (this != &rhs) {
                _snpparams = rhs._snpparams;
                _maxThreshold = rhs._maxThreshold;
                _thresholdAutoTarget = rhs._thresholdAutoTarget;
            }
        }
        sSNPCSVFreqIterParams& operator= (const sSNPCSVFreqIterParams &rhs)
        {
            if (this != &rhs) {
                _snpparams = rhs._snpparams;
                _maxThreshold = rhs._maxThreshold;
                _thresholdAutoTarget = rhs._thresholdAutoTarget;
            }
            return *this;
        }

        inline bool hasNoiseCutoffs() const { return _snpparams.noiseCutoffs[0][0] >= 0; }
        inline real getNoiseCutoff(const sBioseqSNP::SNPRecord *rec, idx atgcIndex) const
        {
            return _snpparams.noiseCutoffs[atgcIndex][rec->relativeLetterIndex(_snpparams.snpCompare)];
        }

        inline real getMinThreshold() const { return _snpparams.minFreqPercent / 100; }
        inline void setMinThreshold(real f) { if (f >= 0 && f <= 1) _snpparams.minFreqPercent = f * 100; }
        inline real getMaxThreshold() const { return _maxThreshold; }
        inline void setMaxThreshold(real f) { if (f > 0 && f <= 1) _maxThreshold = f; }
        inline idx getMinCoverage() const { return _snpparams.minCover; }
        inline void setMinCoverage(idx c) { _snpparams.minCover = c; }
    };

    template <typename Titer=void>
    class sSNPCSVFreqIter: public sIter<real, sSNPCSVFreqIter<Titer> >
    {
    protected:
        const char *_buf, *_bufend;
        bool _buf_concatenated;
        idx _i, _j;
        sBioseqSNP::SNPRecord _rec;
        real _totalFreq;
        real _maxFreq;
        bool _validRec;
        sSNPCSVFreqIterParams _params;

        inline bool isEof() const { return !_buf || _buf+1 >= _bufend || !*_buf; }

        inline void readRec() { static_cast<Titer *>(this)->readRec_impl(); }
        inline void readRec_default()
        {
            _validRec = true;
            _totalFreq = _maxFreq = 0;
            if( isEof() ) {
                _validRec = false;
            } else {
                _buf = _buf_concatenated ? sBioseqSNP::SNPConcatenatedRecordNext(_buf, &_rec, _bufend, _rec.iSub) : sBioseqSNP::SNPRecordNext(_buf, &_rec, _bufend);
                if( !_buf ) {
                    _validRec = false;
                }
            }
            if( _validRec ) {
                for (idx j=0; j<6; j++) {
                    _totalFreq += getDenoisedFreq(j);
                    _maxFreq = sMax(_maxFreq, getDenoisedFreq(j));
                }
            }
        }

        void init(const sSNPCSVFreqIter &rhs)
        {
            _buf = rhs._buf;
            _bufend = rhs._bufend;
            _buf_concatenated = rhs._buf_concatenated;
            _i = rhs._i;
            _j = rhs._j;
            _rec = rhs._rec;
            _validRec = rhs._validRec;
            _params = rhs._params;
        }

        void init(const char *buf, const char *bufend, bool concatenated, const sSNPCSVFreqIterParams * params)
        {
            _buf = buf;
            _bufend = !buf ? NULL : bufend ? bufend : buf + strlen(buf);
            _buf_concatenated = concatenated;
            _i = _j = 0;
            _rec.reset();
            if (params) {
                _params = *params;
            } else {
                _params.init();
            }
            readRec_default();
        }
        inline bool validIndices() const { return _j >= 0 && _j < 6; }
        inline bool meetsThreshold(real f) const
        {
            return f >= _params.getMinThreshold() && f <= _params.getMaxThreshold();
        }
        inline bool meetsCoverage() const
        {
            return _rec.coverage() >= _params.getMinCoverage();
        }

    public:
        inline void requestData_impl() { static_cast<Titer *>(this)->requestData_impl(); }
        inline void releaseData_impl() { static_cast<Titer *>(this)->releaseData_impl(); }
        inline bool readyData_impl() const { return static_cast<const Titer *>(this)->readyData_impl(); }
        inline bool validData_impl() const { return static_cast<const Titer *>(this)->validData_impl(); }
        inline bool validData_default() const { return validIndices() && _validRec; }
        inline idx pos_impl() const { return static_cast<const Titer *>(this)->pos_impl(); }
        inline idx pos_default() const { return 6*_i + _j; }
        inline idx segment_impl() const { return static_cast<const Titer *>(this)->segment_impl(); }
        inline idx segment_default() const { return 0; }
        inline idx segmentPos_impl() const { return static_cast<const Titer *>(this)->segmentPos_impl(); }
        inline idx segmentPos_default() const { return _rec.position; }
        sSNPCSVFreqIter(const char *buf = NULL, const char *bufend = NULL, bool concatenated = true, const sSNPCSVFreqIterParams * params = NULL) { init(buf, bufend, concatenated, params); }
        sSNPCSVFreqIter(const sFil *f, bool concatenated = true, const sSNPCSVFreqIterParams * params = NULL) { init(f->ptr(), f->last(), concatenated, params); }
        sSNPCSVFreqIter(const sFil &f, bool concatenated = true, const sSNPCSVFreqIterParams * params = NULL) { init(f.ptr(), f.last(), concatenated, params); }

        sSNPCSVFreqIter(const sSNPCSVFreqIter &rhs) { init(rhs); }
        sSNPCSVFreqIter& operator=(const sSNPCSVFreqIter &rhs) { init(rhs); return *this; }

        inline sSNPCSVFreqIter<Titer>* clone_impl() const { return static_cast<const Titer *>(this)->clone_impl(); }
        sSNPCSVFreqIter<Titer>& increment_impl() { return static_cast<Titer*>(this)->increment_impl(); }
        sSNPCSVFreqIter<Titer>& increment_default()
        {
            if (_j < 5)
                ++_j;
            else {
                _j = 0;
                ++_i;
                readRec();
            }
            return *this;
        }
        sSNPCSVFreqIter<Titer>& nextRecord()
        {
            _j = 0;
            ++_i;
            readRec();
            return *this;
        }
        inline const sBioseqSNP::SNPRecord& getSNPRecord() const { return _rec; }
        real getDenoisedFreq(idx j=-1) const
        {
            if (j < 0 || j >= 6) j = _j;

            real f = _rec.freq(j, _params._snpparams.snpCompare);
            if (_params.hasNoiseCutoffs())
                f = sMax<real>(0., f - _params.getNoiseCutoff(&_rec, j));

            return f;
        }
        inline bool equals_impl(const sSNPCSVFreqIter &rhs) const {return _buf == rhs._buf && _j == rhs._j;}
        inline bool lessThan_impl(const sSNPCSVFreqIter &rhs) const {return 6 * (_buf - rhs._buf) + (idx)_j - (idx)rhs._j < 0;}
        inline bool greaterThan_impl(const sSNPCSVFreqIter &rhs) const {return 6 * (_buf - rhs._buf) + (idx)_j - (idx)rhs._j > 0;}
        real dereference_impl() const
        {
            real f = getDenoisedFreq();
            if (!meetsThreshold(_maxFreq) || !meetsCoverage())
                return 0;

            return f;
        }
    };

    template<> inline void sSNPCSVFreqIter<void>::readRec() { readRec_default(); }
    template<> inline void sSNPCSVFreqIter<void>::requestData_impl() {}
    template<> inline void sSNPCSVFreqIter<void>::releaseData_impl() {}
    template<> inline bool sSNPCSVFreqIter<void>::readyData_impl() const { return true; }
    template<> inline bool sSNPCSVFreqIter<void>::validData_impl() const { return validData_default(); }
    template<> inline idx sSNPCSVFreqIter<void>::pos_impl() const { return pos_default(); }
    template<> inline idx sSNPCSVFreqIter<void>::segment_impl() const { return segment_default(); }
    template<> inline idx sSNPCSVFreqIter<void>::segmentPos_impl() const { return segmentPos_default(); }
    template<> inline sSNPCSVFreqIter<void>* sSNPCSVFreqIter<void>::clone_impl() const { return new sSNPCSVFreqIter(*this); }
    template<> inline sSNPCSVFreqIter<void>& sSNPCSVFreqIter<void>::increment_impl() { return increment_default(); }

}

#endif 
