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
#ifndef sLib_viopop_hpp
#define sLib_viopop_hpp

#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>
#include <slib/utils.hpp>
#include <ssci/math.hpp>

#include <ssci/bio/bioseqpopul.hpp>
#include <ssci/bio/bioseq.hpp>

class sViopop
{
    private:
        sStr _t_buf;
        sVioDB vioDB;
        enum eVioPopTypes {
            eTypeClone=1,
            eTypeSeqCov,
            eTypeStats,
            eTypeSummary,
            eTypeSortArr
        };
        enum eCloneRlts
        {
            eClone2FatherClone = 1,
            eClone2MergeClone,
            eClone2Clone,
            eClone2SeqCov,
            eClone2Stats,
            eClone2Summary,
            eClone2SortArr
        };
        enum eSeqCovRlts
        {
            eSeqCov2Clone =1,
        };


    public:

        struct cloneRegion{
            idx contigInd, start, end;
            real freq;
            cloneRegion(){sSet(this,0);freq=1;}
        };

        struct cloneProfPos{
                idx pos[8];
                idx maxI() {
                    idx s_max = 0, i_max = 0;
                    for(idx i = 0 ; i < sDim(pos) ; ++i){
                        if( s_max < pos[i] ) {
                            s_max = pos[i];
                            i_max = i;
                        }
                    }
                    return i_max;
                }
                bool isMultiGap() {
                    return (maxI()==sBioseqpopul::baseGap);
                }
                bool isDeletion() {
                    return (maxI()==sBioseqpopul::baseDel);
                }
                bool isN() {
                    return maxI() == sBioseqpopul::baseN;
                }
                bool isAnyGap() {
                    return isDeletion() || isMultiGap();
                }
                idx sum() {
                    return pos[sBioseqpopul::baseA] + pos[sBioseqpopul::baseC] + pos[sBioseqpopul::baseG] + pos[sBioseqpopul::baseT] + pos[sBioseqpopul::baseDel] + pos[sBioseqpopul::baseN];
                }
                cloneProfPos(){sSet(this,0);}
        };

        struct contigComp{
            sVec< idx > contigInds;
            idx start, end;
            contigComp(){start = 0; end = 0;}
        };

        enum ePrintContig {
            ePrintContigSeq=                      0x00000001,
            ePrintContigComp=                     0x00000002,
            ePrintContigAl=                       0x00000004,
            ePrintContigCov=                      0x00000008,
            ePrintContigBreakpoints=              0x00000010,
            ePrintContigSummary=                  0x00000020,
            ePrintContigPrevalence=               0x00000040,
            ePrintContigDiversityMeasures=        0x00000080
        };

        sDic<idx> * simil, lsimil;
        sDic< sVec < idx> > * mergeDict;
        sDic< sVec < contigComp> > * mergeCompDict;
        sVec<idx> * gap2skipMap, lgap2skipMap;
        sVec<idx> * skip2gapMap, lskip2gapMap;
        idx gapFrameLevel;

        struct seqCovPosition{
                idx baseCov;
                idx base(){return (baseCov>>60)&0xF;}
                inline char baseChar(){ return isAnyGap() ? '-':(isN()?'N':sBioseq::mapRevATGC[base()]);}
                inline bool isN() {return base()==sBioseqpopul::baseN;}
                inline bool isMultiGap(){return base()==sBioseqpopul::baseGap;}
                inline bool isSupportedGap(){return base()==sBioseqpopul::baseDel;}
                bool isCoverageGap(){return coverage()==0; }
                inline bool isAnyGap(){return isSupportedGap() || isMultiGap() || isCoverageGap();}
                idx coverage(){return (baseCov&0xFFFFFFFFFFFFFFF);}
                void setCoverage(idx val){if(val<0)val=0;baseCov=(baseCov&0xF000000000000000)|(val&0xFFFFFFFFFFFFFFF);}
                void setBase(idx val){if(val<0)val=sBioseqpopul::baseGap;baseCov=(baseCov&0xFFFFFFFFFFFFFFF)|((val&0xF)<<60);}
                seqCovPosition() {
                    baseCov = 0;
                }
        };


        sViopop(const char * InputFilename=0, const char * similFlnm = 0, const char * gap2skiFlnm = 0, const char * skip2gapFlnm = 0,  idx mode=sMex::fReadonly)
        {
            progress_CallbackFunction = 0;
            progress_CallbackParam = 0;
            simil = 0;
            gap2skipMap = 0;
            skip2gapMap = 0;
            mergeDict = 0;
            mergeCompDict = 0;
            gapFrameLevel = 0;
            init(InputFilename, similFlnm, gap2skiFlnm, skip2gapFlnm, mode);
        }

        sViopop * init (const char * InputFilename=0, const char * similFlnm = 0, const char * gap2skiFlnm = 0, const char * skip2gapFlnm = 0, idx mode=sMex::fReadonly)
        {
            if( InputFilename && InputFilename[0] ) {
                vioDB.init(InputFilename, "viopop", 0, 0, mode);
            }
            if( similFlnm && similFlnm[0] ) {
                sFil similFl(similFlnm, mode);
                if( similFl.ok() ) {
                    lsimil.serialIn((const void *) similFl.ptr(), similFl.length());
                    simil = &lsimil;
                }
            }
            if( gap2skiFlnm && gap2skiFlnm[0] ) {
                lgap2skipMap.init(gap2skiFlnm, mode);
                gap2skipMap = lgap2skipMap.dim()?&lgap2skipMap:0;
            }
            if( skip2gapFlnm && skip2gapFlnm[0] ) {
                lskip2gapMap.init(skip2gapFlnm, mode);
                skip2gapMap = lskip2gapMap.dim()?&lskip2gapMap:0;
            }
            return this;
        }
        bool isok(void)
        {
            return (vioDB.isok("viopop")||vioDB.isok("viopop2"))? true : false;
        }

        bool isVersion2(void)
        {
            return vioDB.isok("viopop2")?true:false;
        }

        idx dimCl(void){
            return vioDB.GetRecordCnt(eTypeClone);
        }
        idx dimSeqCov(){
            return vioDB.GetRecordCnt(eTypeSeqCov);
        }

        void setMode(idx mode)
        {
            idx * userspace=(idx*)vioDB.userSpace8();
            if(mode) (*userspace)|=(idx)(0x01);
            else (*userspace)&=(idx)(~0x01);
        }

        idx getMode(void)
        {
            idx userspace=*(idx*)vioDB.userSpace8();
            return userspace&((idx)0x01) ? 1 : 0 ;
        }

        sBioseqpopul::cloneSummary * getCl(idx iAlIndex,idx * sortArr=0){
            return (sBioseqpopul::cloneSummary * )vioDB.Getbody (eTypeClone, iAlIndex+1, 0);
        }
        void getCov (idx iAlIndex,sVec<idx> & cov,idx * sortArr=0) {
            seqCovPosition * seqcov=0;
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeClone, (idx)iAlIndex+1, eClone2SeqCov, 0,0 ),bodysize=0;
            if(pMatch)
                seqcov=(seqCovPosition *)vioDB.Getbody (eTypeSeqCov, *pMatch, &bodysize);
            bodysize = bodysize / sizeof(idx);
            cov.empty();
            cov.add(bodysize);
            for(idx i=0;i<(bodysize);++i){
                cov[i]=seqcov[i].coverage();
            }
        }
        void getSeq (idx iAlIndex,sVec<idx> & seq,idx * sortArr=0) {
            seqCovPosition * seqcov=0;
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeClone, (idx)iAlIndex+1, eClone2SeqCov, 0,0 ),bodysize=0;
            if(pMatch)
                seqcov=(seqCovPosition *)vioDB.Getbody (eTypeSeqCov, *pMatch, &bodysize);

            bodysize = bodysize / sizeof(idx);
            seq.empty();
            seq.add( bodysize );
            for(idx i=0;i<(bodysize);++i){
                seq[i]=seqcov[i].base();
            }
        }
        seqCovPosition * getSeqCov (idx iAlIndex,idx * bodysize,idx * sortArr=0) {
            seqCovPosition * seqcov=0;
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeClone, (idx)iAlIndex+1, eClone2SeqCov, 0,0 );
            if(pMatch)
                seqcov=(seqCovPosition *)vioDB.Getbody (eTypeSeqCov, *pMatch, bodysize);
            if(bodysize){
                *bodysize = *bodysize/sizeof(seqCovPosition);
            }
            return seqcov;
        }

        idx getPositionSimil (idx cl, idx pos, idx refCnt, real * res, bool percentage = false, sVec < contigComp > * comp = 0) {
            if(!simil){
                return 0;
            }
            idx tot = 0;
            for(idx i = 0 ; i <  refCnt ; ++i ) {
                res[i] = getMergedPositionSingleSimil(cl,pos,i, comp);
                tot +=res[i];
            }
            if( percentage ) {
                for(idx i = 0 ; i <  refCnt ; ++i ) {
                    if(res[i]>0)res[i]/=tot;
                }
            }
            return tot;
        }

        real getMergedPosSimils(idx clID, idx pos, idx iS, idx cov, sVec<contigComp> * comp) {
            sVec < idx> * contigs = 0;
            real res = 0;
            if( comp && comp->dim() ) {
                for( idx i = 0 ; i < comp->dim() ; ++i ) {
                    if( pos>= comp->ptr(i)->start && pos < comp->ptr(i)->end) {
                        contigs = &comp->ptr(i)->contigInds;
                        break;
                    }
                }
            }
            if( contigs ) {
                for( idx j = 0 ; j < contigs->dim() ; ++j ){
                    res += (real)getPositionSingleSimil(*(contigs->ptr(j)), pos, iS);
                }
                res = res/cov;
            }
            else {
                res = ((real)getPositionSingleSimil(clID, pos, iS))/cov;
            }
            return res;
        }

        bool getSimilAboveThrshld ( sVec< real > &simil, sStr & res, real thrshld, sBioseq * Sub = 0  ) {
            res.cut(0);
            bool ret=false;
            for(idx i = 0 ; i < simil.dim() ; ++i ) {
                if( simil[i] >= thrshld) {
                    ret=true;
                    if( res.length() ) {
                        res.printf("/");
                    }
                    if ( Sub ) {
                        res.printf("\"%s\"",Sub->id(i));
                    }
                    else {
                        res.printf("%" DEC,i);
                    }
                }
            }
            if(!ret) res.printf("-");
            return ret;
        }

        bool isBreakpoint (sStr &prev, sStr &curr) {
            if( !prev.length() ){
                return false;
            }
            if( prev.length() && curr.length() )
            {
                return strcmp(prev.ptr(), curr.ptr());
            }
            return true;
        }


        idx getPositionSingleSimil (idx cl, idx pos, idx ref) {
            if(!simil){
                return 0;
            }
            _t_buf.printf(0,"%s",contructSimilaritiesID(cl, pos,ref));
            idx * res = simil->get( (const void *)_t_buf.ptr(), _t_buf.length() );
            if(!res) {
                return 0;
            }
            return *res;
        }
        idx getMergedPositionSingleSimil (idx cl, idx pos, idx ref, sVec<contigComp> * comp = 0 ) {
            sVec < idx> * contigs = 0;
            idx res = 0;
            if( comp && comp->dim() ) {
                for( idx i = 0 ; i < comp->dim() ; ++i ) {
                    if( pos>= comp->ptr(i)->start && pos < comp->ptr(i)->end) {
                        contigs = &comp->ptr(i)->contigInds;
                        break;
                    }
                }
            }
            if( contigs ) {
                for( idx j = 0 ; j < contigs->dim() ; ++j ){
                    res += getPositionSingleSimil(*(contigs->ptr(j)), pos, ref);
                }
            }
            else if( !comp ){
                res = getPositionSingleSimil(cl, pos, ref);
            }
            return res;
        }

        idx getGap2SkipContigPos ( sBioseqpopul::cloneSummary *cl , idx pos, sVec<idx> * contigMap = 0, bool forceValid = false) {
            idx res = 0;
            if( contigMap && contigMap->dim()) {
                if(pos>=0 && pos<contigMap->dim()) {
                    res =  *contigMap->ptr(pos);
                } else {
                    res =-1;
                    if(forceValid) {
                        if(pos<0) {
                            pos = 1;
                        }
                        else {
                            pos = contigMap->dim();
                        }
                    }
                }
                while ( res < 0 && --pos >= 0 && forceValid) {
                    res =  *contigMap->ptr(pos);
                }
            }
            else {
                if(!gap2skipMap || cl->clID > gap2skipMap->dim() ) {
                    res = pos;

                }
                res = *gap2skipMap->ptr(pos+ *gap2skipMap->ptr(cl->clID) );
                if( res < -1 && pos == 0) {
                    while ( res < 0 && ++pos + cl->start < cl->end && forceValid) {
                        res =  *gap2skipMap->ptr(pos+ *gap2skipMap->ptr(cl->clID) );
                    }
                    while (res < 0 && --pos >= 0 && forceValid) {
                        res =  *gap2skipMap->ptr(pos+ *gap2skipMap->ptr(cl->clID) );
                    }
                }
                else {
                    while ( res < 0 && --pos >= 0 && forceValid) {
                        res =  *gap2skipMap->ptr(pos+ *gap2skipMap->ptr(cl->clID) );
                    }
                }

            }
            if( res < 0 && forceValid ) res = 0;
            return res;
        }

        idx getSkip2GapContigPos( sBioseqpopul::cloneSummary *cl , idx pos, sVec<idx> * contigMap = 0) {
            if( contigMap && contigMap->dim()) {
                return *contigMap->ptr(pos);
            }
            else {
                if(!skip2gapMap || cl->clID > skip2gapMap->dim() ) {
                    return pos;
                }
                return *skip2gapMap->ptr(pos+*skip2gapMap->ptr(cl->clID));
            }

        }

        idx getFramePos( sBioseqpopul::cloneSummary * cl, idx pos, sVec<idx> * contigMap = 0, bool forceValid = false) {
            if( (!gap2skipMap || cl->clID > gap2skipMap->dim()) && (!contigMap || !contigMap->dim()) ) {
                return pos;
            }
            else {
                return getGap2SkipFramePos(cl, pos, contigMap, forceValid);
            }
        }
        inline idx getContigPos(sBioseqpopul::cloneSummary * cl, idx pos, sVec<idx> * contigMap = 0, bool forceValid = false ) {
            return getContigDistance(cl,pos,0,contigMap, forceValid);
        }

        idx getGap2SkipFramePos(sBioseqpopul::cloneSummary * cl, idx pos, sVec<idx> * contigMap = 0, bool forceValid = false);

        idx getContigDistance(sBioseqpopul::cloneSummary * cl, idx end, idx start = 0, sVec<idx> * contigMap = 0, bool forceValid = false);

        inline bool isMultiGap(sBioseqpopul::cloneSummary * cl, idx pos, sVec<idx> * contigMap = 0) {
            if( contigMap && contigMap->dim() ) {
                return *contigMap->ptr(pos) == -1;
            }
            else {
                    seqCovPosition * p = getSeqCov(cl->clID,0);
                    return p[pos].isMultiGap();
            }
        }

        inline bool isSupportedGap(sBioseqpopul::cloneSummary * cl, idx pos, sVec<idx> * contigMap = 0) {
            if( contigMap && contigMap->dim() ) {
                return *contigMap->ptr(pos) == -2;
            }
            else {
                    seqCovPosition * p = getSeqCov(cl->clID,0);
                    return p[pos].isSupportedGap();
            }
        }

        inline bool isAnyGap(sBioseqpopul::cloneSummary * cl, idx pos, sVec<idx> * contigMap = 0) {
            if( contigMap && contigMap->dim() ) {
                return *contigMap->ptr(pos) == -2;
            }
            else {
                    seqCovPosition * p = getSeqCov(cl->clID,0);
                    return p[pos].isAnyGap();
            }
        }

        sBioseqpopul::cloneStats * getStats (idx iAlIndex,idx * sortArr=0) {
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeClone, (idx)iAlIndex+1, eClone2Stats, 0,0 );
            return pMatch?(sBioseqpopul::cloneStats*)vioDB.Getbody (eTypeStats, *pMatch, 0):(sBioseqpopul::cloneStats*)0;
        }
        idx * getArrSort () {
            return (idx*)vioDB.Getbody (eTypeSortArr, 1, 0);
        }
        sBioseqpopul::cloneStats * getGenStats () {
            return (sBioseqpopul::cloneStats *)vioDB.Getbody (eTypeSummary, 1, 0);
        }

        sBioseqpopul::cloneSummary * getFatherCl(idx iAlIndex,idx * sortArr=0){
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeClone, (idx)iAlIndex+1, eClone2FatherClone, 0,0 );
            return pMatch?(sBioseqpopul::cloneSummary*)vioDB.Getbody (eTypeClone, *pMatch, 0):(sBioseqpopul::cloneSummary*)0;
        }
        sBioseqpopul::cloneSummary * getMergedCl(idx iAlIndex,idx * sortArr=0){
            idx * pMatch=(idx*)vioDB.GetRelationPtr(eTypeClone, (idx)iAlIndex+1, eClone2MergeClone, 0,0 );
            return pMatch?(sBioseqpopul::cloneSummary*)vioDB.Getbody (eTypeClone, *pMatch, 0):(sBioseqpopul::cloneSummary*)0;
        }

        idx * listBifCl(idx iAlIndex, idx * relCount,idx * sortArr=0){

            return (idx*)vioDB.GetRelationPtr(eTypeClone, iAlIndex+1, eTypeClone, (idx*)relCount,0 );
        }

        idx Digest(const char* outputfilename, sFrameConsensus * rawPopulation,idx * arrSortClon,sBioseqpopul::cloneStats * sumStats,idx * sortedSubjInd);


    public:

        typedef idx (*callbackType)(void * param, idx countDone, idx progressCur, idx percentMax);
        callbackType progress_CallbackFunction;
        void * progress_CallbackParam;

        enum fViewCloneFlags {
            clPrintSummary              =0x0001,
            clPrintTreeMode             =0x0002,
            clPrintCoverage             =0x0004,
            clPrintConsensus            =0x0008,
            clPrintRegionsConsensus     =0x0010,
            clPrintStats                =0x0020,
            clPrintTree                 =0x0040,
            clPrintContigsInMutualFrame =0x0080,
            clPrintNoGapsFrame          =0x0100,
            clSkipSupportedDeletions    =0x0200,
            clPrintLowDiversityBreaks   =0x0400,
            clPrintFastaTitleComposition=0x0800,
            clPrintFastaTitleSimple     =0x1000,
            clPrintFastaTitleNumbersOnly=0x2000,
            clPrintGlobal               =0x4000

        } ;
        struct ParamCloneIterator{
            idx flags,iCln,step,resolution,minCov,minLen,minSup,* normCov,mergeHidden,showSimil,sStart,sEnd,wrap,breaksminLen,covThrs,mc_iters;
            const char * hiddenClones;
            bool isNormCov,minDiv,collapse;
            void * userPointer;
            real * frequencies,minF;
            sStr * out;
            const char * fastaTmplt;

            real similarity_threshold;
            idx similarity_cnt, cnt;
            ParamCloneIterator(sStr * xstr=0 ){
                sSet(this,0);
                cnt = sIdxMax;
                out=xstr;
                step=50;
                resolution=300;
                similarity_threshold=1;
                breaksminLen = 0;
                wrap=120;
                minF = ((real)1)/pow(10,4);
            }
        };


        typedef idx (*typeCallbackIteratorFunction)(sViopop * viopop, ParamCloneIterator * param, idx cloneIndex);

        idx iterateClones(idx * iVis, idx start, idx cnt, typeCallbackIteratorFunction callbackFunc,ParamCloneIterator * params,idx * arrSortClon=0);

        idx printAllCoverageClones(idx * iVis, idx start, idx cnt,sStr & out,idx * arrSortClon=0);
        idx printAllSequenceClones(idx * iVis, idx start, idx cnt,sStr & out,idx * arrSortClon=0);

        idx getPermutationsCompositions(const char * clones, sVec< sVec <sViopop::cloneRegion> > & out, sVec< idx > & clIds, ParamCloneIterator * params = 0 );

        idx getPredictedGlobal(sVec< sVec <sViopop::cloneRegion> > & out, sVec< real > & clIds, ParamCloneIterator * params = 0);

        idx getExtendedCompositions(const char * clones, sVec< sVec <sViopop::cloneRegion> > & out, sVec< idx > & clIds, ParamCloneIterator * params = 0);
        idx getExtendedComposition( sBioseqpopul::cloneSummary * cl, sVec<sViopop::cloneRegion> * c_Out, ParamCloneIterator * params = 0);
        idx getContigCompositions( const char * clones_input, sVec< sVec <sViopop::cloneRegion> > & out, sVec< idx > & clIds, ParamCloneIterator * params = 0);
        idx getContigComposition( sBioseqpopul::cloneSummary * cl, sVec<sViopop::cloneRegion> * c_Out, ParamCloneIterator * params = 0);

        const char * printTitle(sStr & title, ParamCloneIterator * param, idx contgId, idx start = 0, idx end = 0, const char * sep = "," );
        void printTitleFreq(sStr & title, ParamCloneIterator * param, idx contgId) { if(param && param->frequencies)title.printf(" Freq=%lf",param->frequencies[contgId]);};

        idx printContigsPrevalence(sVec< sVec< sViopop::cloneRegion > > & composition, sVec<idx> & clIds, sStr & out, ParamCloneIterator * params);
        void getDiversityMeasureUnits(sVec< sVec< sViopop::cloneRegion > > & compositions, ParamCloneIterator * param, sVec<real> & avCovs, sVec<sVec<seqCovPosition> > & all_seqs);
        void printContigsDiversityMeasurements(sVec< sVec< sViopop::cloneRegion > > & composition, sVec<idx> & clIds, sStr & out, ParamCloneIterator * params);

        idx printContig( sVec< sVec< sViopop::cloneRegion > > & compositions, sVec<idx> & clIds, sStr & out, ParamCloneIterator * params, idx print_type );
        idx printContigSequences( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * params, idx curclID, idx offset = 0 );
        idx printContigAlignments( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx curclID, idx offset = 0 );
        idx printContigCoverages( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx curclID, idx offset = 0 );
        idx printContigComposition( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx curclID, idx offset = 0 );
        idx printContigBreakpoints( sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx curclID, idx offset = 0 );
        idx printContigSummary(sVec< sViopop::cloneRegion > & composition, sStr & out, ParamCloneIterator * param, idx offset = 0);

        idx printStackedStats(sStr * out, ParamCloneIterator * params = 0 );

        bool cloneOverlap(sBioseqpopul::cloneSummary * t_sum, sDic<bool> & clones);
        bool cloneOverlap(sBioseqpopul::cloneSummary * clA, sBioseqpopul::cloneSummary * clB);

        bool isCloneAncestor(sBioseqpopul::cloneSummary * t_sum, sDic<bool> & clones);
        bool isCloneAncestor(sBioseqpopul::cloneSummary * t_sum1, sBioseqpopul::cloneSummary * t_sum2);

        bool printSimilaritiesinJSON(sStr & out, idx cl, idx pos, idx cov, idx rfCnt, ParamCloneIterator * params, sVec < contigComp > * cComp = 0 );

        void printPositionCoverageinJSON(sStr & out, idx pos, char let , idx cov, const char * sim, bool abs, idx t_cov );

        const char * contructSimilaritiesID ( idx icl, idx pos, idx reference ) {
            return _t_buf.printf(0,"%" DEC "-%" DEC "-%" DEC, icl,pos,reference);
        }

        bool buildMergeDictionary( ParamCloneIterator * params, sDic< sVec<idx> > & mergeTree, idx clCnt );
        idx buildMergeCompositionDictionary( ParamCloneIterator * params, sDic< sVec<contigComp> > & mergeComp );
        void constructCloneComposition(idx iCl, sVec< contigComp > * comp);

        inline bool isValidClone (idx iCl) {
            if(mergeDict) {
                _t_buf.printf(0,"%" DEC,iCl);
                return mergeDict->get(_t_buf.ptr());
            }
            else {
                return true;
            }
        }

        idx getValidCloneID( idx iCl, bool followOnNonMergedContigs = false);
        sBioseqpopul::cloneSummary * getValidCloneSummary( idx iCl, bool followOnNonMergedContigs = false);
        sBioseqpopul::cloneSummary * getValidParentSummary(sBioseqpopul::cloneSummary * cl);
        sBioseqpopul::cloneSummary * getValidMergingSummary(sBioseqpopul::cloneSummary * cl);
        bool getValidCloneComposition(sBioseqpopul::cloneSummary * cl, sVec < contigComp > & c_comp );
        bool mergeClone(sBioseqpopul::cloneSummary * cl, sBioseqpopul::cloneSummary &resCl, sVec < contigComp > & comp, ParamCloneIterator * params = 0, sVec< seqCovPosition > * seqCov =0 , sVec<idx> * gap2skpMap= 0 , sVec<idx> * skp2gapMap = 0);

        idx getMergedFramePos(sBioseqpopul::cloneSummary * cl, idx pos, ParamCloneIterator * params = 0, bool forceValid = false);
        idx getMergedContigPos(sBioseqpopul::cloneSummary * cl, idx pos, ParamCloneIterator * params = 0, bool forceValid = false);

        idx clonesCoverage(ParamCloneIterator * params, sBioseqpopul::cloneSummary * Sum, sVec< seqCovPosition > * seqCov, idx refCnt, sVec < contigComp > * cComp = 0, sVec<idx> * skp2gapMap = 0, sVec<idx> * gap2skpMap = 0 );
        idx clonesDifferences(ParamCloneIterator * params, sBioseqpopul::cloneSummary * parentCl, sBioseqpopul::cloneSummary * childCl, sVec<seqCovPosition> * pSC = 0,  sVec<seqCovPosition> * cSC = 0, sVec<idx> * child_gap2skpMap = 0, sBioseqpopul::cloneStats * c_stats = 0 );

        static idx printAllClones(sViopop * viopop, ParamCloneIterator * params, idx clIndex);
        static idx printHierarchySingle(sViopop * viopop, ParamCloneIterator * params, idx clIndex);
        static idx printCoverageSingle(sViopop * viopop, ParamCloneIterator * params, idx clIndex);
        static idx getClonesOnPosition(sViopop * viopop, idx pos, sVec<idx> * clInds);
        idx getBifurcatedClonesOnPosition( idx pos, sVec<idx> * clInds, idx clId );
}; 

class popGraph {

    public:
        popGraph(sViopop * vp) {
            _vp = vp ;
            _tot_sum = 0;
            _max_sum = 0;
        }
        struct node {
                idx start,end,irng_s,irng_e;
                sBioseqpopul::cloneSummary clone;
                real cov;
                sVec<idx> next, previous;
                idx length() {return end - start;}
                real getSim(real ref_cov) {
                    if(!ref_cov) return cov;
                    if(ref_cov == cov) return REAL_MAX/100;
                    return (cov/sAbs(cov - ref_cov));
                }
                node(){irng_s = irng_e = start = end = cov = 0;sSet(&clone,0);}
        };

    private:
        sVec<node> _nodes;
        sViopop * _vp;
        sVec<idx> _max_contigs;
        sVec<real> _max_contigs_freqs;
        real _max_sum;
        real _tot_sum;

        real _getNextSum(node* n, real ref_cov) {
            real sum = 0;
            for(idx i = 0 ; i < n->next.dim() ; ++i) {
                sum += _nodes[n->next[i]].getSim(ref_cov);

            }
            return sum;
        }

        real _getPrevSum(node* n, real ref_cov) {
            real sum = 0;
            for(idx i = 0 ; i < n->previous.dim() ; ++i) {
                sum += _nodes[n->previous[i]].getSim(ref_cov);

            }
            return sum;
        }
    public:
        idx buildGraph(sViopop::ParamCloneIterator * params);

        idx findMerged(idx end, sBioseqpopul::cloneSummary * clone, sVec<idx> & res) {
            node * nd = 0;
            for( idx i = 0 ; i < _nodes.dim() ; ++i ) {
                nd = _nodes.ptr(i);
                if( nd->clone.hasMerged() && nd->clone.end == end && nd->end == end && nd->clone.mergeclID == clone->clID ) {
                    res.vadd(1, i);
                }
            }
            return res.dim();
        }

        node * addNode(idx start,idx end,idx irng, sBioseqpopul::cloneSummary & cls) {
            node * n = _nodes.add();
            n->start = start;
            n->end = end;
            n->irng_s = irng;
            n->irng_e = irng + 1;
            n->clone = cls;
            return n;
        }
        void extendFrw(node * n, idx end, idx irng){
            n->end = end;
            n->irng_e = irng+1;
        }
        void extendRvr(node * n, idx start, idx irng){
            n->start = start;
            n->irng_e = irng;
        }
        void connectNodes(node * prev, node * next) {
            idx in = next -_nodes.ptr();
            idx ip = prev -_nodes.ptr();
            prev->next.vadd(1,in);
            next->previous.vadd(1,ip);
        }
        bool isSubrange(idx end, sBioseqpopul::cloneSummary * clone) {
            if( clone->start == end )
                return false;
            node * nd = 0;
            for( idx i = 0 ; i < _nodes.dim() ; ++i ) {
                nd = _nodes.ptr(i);
                if( nd->end == end && nd->clone.clID == clone->clID ) {
                    return true;
                }
            }
            return false;
        }
        idx getNodeIndFromRange(idx iCl, idx end)
        {
            node * nd = 0;
            for(idx i = 0; i < _nodes.dim(); ++i) {
                nd = _nodes.ptr(i);
                if( nd->end >= end && nd->clone.clID == iCl ) {
                    return i;
                }
            }
            return -1;
        }
        idx getSeed(bool min_div) {
                real cml= 0, rand_v = _tot_sum * sRand::random1();
                for(idx i = 0 ; i < _nodes.dim() ; ++i ) {
                    cml += _nodes[i].cov*_nodes[i].length() ;
                    if (cml > rand_v) {
                        return i;
                    }
                }
            return -1;
        }
        node * getNextNode(node * n, real freq) {
            if(!n->next.dim()) return 0;
            if(n->next.dim()==1) return _nodes.ptr(n->next[0]);

            node * nn = 0;
            real sum = _getNextSum(n,freq), cml = 0;

            real rand_v = sum * sRand::random1();
            for(idx i = 0 ; i < n->next.dim() ; ++i ) {
                nn = _nodes.ptr(n->next[i]);
                cml += nn->getSim(freq);
                if (cml > rand_v) {
                    return nn;
                }
            }
            return 0;
        }
        node * getPrevNode(node * n, real freq) {
            if(!n->previous.dim()) return 0;
            if(n->previous.dim()==1) return _nodes.ptr(n->previous[0]);

            node * nn = 0;
            real sum = _getPrevSum(n,freq), cml = 0;

            real rand_v = sum * sRand::random1();

            for(idx i = 0 ; i < n->previous.dim() ; ++i ) {
                nn = _nodes.ptr(n->previous[i]);
                cml += nn->getSim(freq);
                if (cml > rand_v) {
                    return nn;
                }
            }
            return 0;
        }

        idx traverse(idx seed_ind, sVec<sViopop::cloneRegion> & out, bool min_diversity);

};

#endif
