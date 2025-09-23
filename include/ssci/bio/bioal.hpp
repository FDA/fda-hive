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
#ifndef sLib_bioal_hpp
#define sLib_bioal_hpp

#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>
#include <ssci/bio/tax-ion.hpp>
#include <ssci/bio/bioseqalign.hpp>
#include <regex.h>

class sBioal
{
    public:
        sBioseq * Sub;
        sBioseq * Qry;

        sBioal ()
        {
            progress_CallbackFunction = 0;
            progress_CallbackParam = 0;
            Qry=Sub=0;
        }

        virtual ~sBioal ()
        {
        }

        typedef enum EBioModeBits_enum {
            eBioModeShortBoth=0x00,
            eBioModeLongBoth=0x01,
            eBioModeLongSub=0x02,
            eBioModeLongQry=0x03
        } EBioModeBits;

        bool isok(void)
        {
            return true;
        }
        virtual idx dimAl(void)
        {
            return 0;
        }
        virtual sBioseqAlignment::Al * getAl(idx iAlIndex)
        {
            return 0;
        }
        virtual idx * getMatch(idx iAlIndex)
        {
            return 0;
        }
        virtual idx getRpt(idx iAlIndex)
        {
            return 0;
        }
        virtual idx dimSub()
        {
            return 0;
        }
        virtual const char * getSAMContent(idx iAlIndex, idx * size)
        {
            return 0;
        }
        virtual idx listSubAlIndex(idx idSub, idx * relCount)
        {
            return 0;
        }
        struct Stat
        {
                idx found;
                idx foundRpt;
                Stat() { sSet(this); }
        };
        virtual idx dimStat()
        {
            return 0;
        }
        virtual Stat * getStat(idx iStat = 0, idx iSub = 0, idx * size = 0)
        {
            return 0;
        }
        virtual void setMode(sBioseq::EBioMode qrymode, sBioseq::EBioMode sub)
        {
        }
        virtual sBioseq::EBioMode getSubMode(void)
        {
            return sBioseq::eBioModeShort;
        }
        virtual sBioseq::EBioMode getQryMode(void)
        {
            return sBioseq::eBioModeShort;
        }
        virtual bool isPairedEnd(void)
        {
            return false;
        }


    public:
        typedef idx (*callbackTypeProgress)(void * param, idx countDone, idx curPercent, idx maxPercent);

        typedef idx (*HitListExtensionCallback)(void * param, sIO * io, sBioseq * sub, sDic <sStr > * dic,const char * seqidFrom00, idx countResultMax, idx startResult, idx contSequencesMax, idx outPutHeader,sStr * hdr );

        struct ParamsAlignmentIterator{
            sStr * str;
            idx navigatorFlags;
            regex_t * regp;
            idx rangestart, rangeend,High,winSize,pageRevDir,subRealS,padding,indels,currentChunk,winTailLimit, maxAlLen;
            idx alCol;
            const char * id;
            FILE * outF;
            idx wrap;
            idx alignmentStickDirectional;
            idx rightTailPrint,leftTailPrint;

            void * userPointer;
            idx userIndex;

            ParamsAlignmentIterator(sStr * lstr=0){
                sSet(this,0);
                str=lstr;
                regp=0;
                High=-1;
                alignmentStickDirectional=0;
            }
        };
        struct ParamsAlignmentSummary {
            bool reportZeroHits, reportTotals, reportFailed;
            idx start, cnt;
            sVec<idx> * processedSubs, * coverage;
            HitListExtensionCallback callBackExtension;
            sTaxIon *taxion;
            regex_t * regp;
            void * param;
            ParamsAlignmentSummary(){
                sSet(this, 0);
            }
        };

        typedef idx (*callbackType)(void * param, idx countDone, idx progressCur, idx percentMax);
        callbackType progress_CallbackFunction;
        void * progress_CallbackParam;

        enum fViewAlignmentFlags {
            alPrintSubject              =0x1,
            alPrintUpperInterm          =0x2,
            alPrintQuery                =0x4,
            alPrintLowerInterm          =0x08,
            alPrintDotsFormat           =0x10,
            alPrintVariationOnly        =0x20,
            alPrintTouchingOnly         =0x40,
            alPrintNonFlippedPosforRev  =0x80,
            alPrintMirroredPos          =0x100,
            alPrintRegExpSub            =0x200,
            alPrintRegExpQry            =0x400,
            alPrintBasedOnRange         =0x800,
            alPrintMode                 =0x1000,
            alPrintRegExpInt            =0x2000,
            alPrintMutBiasOnly          =0x4000,
            alPrintNonPerfectOnly       =0x8000,
            alPrintPositionalRegExp     =0x10000,
            alPrintPosInDelRegExp       =0x20000,
            alPrintInRandom             =0x40000,
            alPrintSequenceOnly         =0x80000,
            alPrintExcludeDeletions     =0x100000,
            alPrintExcludeInsertions    =0x200000,
            alPrintIgnoreCaseMissmatches=0x400000,
            alPrintMultiple             =0x800000,
            alPrintCollapseRpt          =0x1000000,
            alConsensusOverlap          =0x2000000,
            alConsensusIgnoreGaps       =0x4000000,
            alPrintAsFasta              =0x8000000,
            alPrintNs                   =0x10000000,
            alPrintUpperCaseOnly        =0x20000000,
            alPrintSaturationReduced    =0x40000000,
            alPrintRepeatsOnly          =0x80000000,
            alPrintForward              =0x100000000,
            alPrintReverse              =0x200000000,
            alPrintReadsInFasta         =0x400000000,
            alPrintQualities            =0x800000000,
            alPrintFilterTail           =0x1000000000,
            alPrintTailDisplayTail      =0x2000000000,
            alPrintTailDisplayAlignment =0x4000000000
        };


        enum eSAMFlags
        {
           eSF_ReadPaired = 0x1,
           eSF_ReadMappedProperPair = 0x2,
           eSF_ReadUnmapped = 0x4,
           eSF_MateUnmapped = 0x8,
           eSF_ReadReverseStrand = 0x10,
           eSF_MateReverseStrand = 0x20,
           eSF_FirstInPair = 0x40,
           eSF_SecondInPair = 0x80,
           eSF_NotPrimaryAlignment = 0x100,
           eSF_ReadFailsQualityChecks = 0x200,
           eSF_ReadPCROpticalDuplicate = 0x400,
           eSF_SupplementaryAlignment = 0x800
        };

        struct LenHistogram
        {
                idx cntRead, cntSeq, cntFailed, cntAl, cntLeft, cntRight,maxLeft,maxRight, lenAnisotropy;
                LenHistogram()
                {
                    sSet(this, 0);
                }
        };

        typedef idx (*typeCallbackIteratorFunction)(sBioal * bioal, ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd);
        idx iterateAlignments(idx * iVis, idx start, idx cnt, idx iSub, typeCallbackIteratorFunction callbackFunc, ParamsAlignmentIterator * callbackParam=0, typeCallbackIteratorFunction secondaryCallbackFunc = 0, ParamsAlignmentIterator * secondaryCallbackParam = 0, sVec<idx> * sortArr = 0);

        static idx rngComparator (void * param, idx * position, sBioal * arr, idx iAl) {
            if ( sOverlap( *position, *position, arr->getAl(iAl)->getSubjectStart(arr->getMatch(iAl)), arr->getAl(iAl)->getSubjectEnd(arr->getMatch(iAl))) )
                return 0;
            return *position - arr->getAl(iAl)->getSubjectStart(arr->getMatch(iAl));
        }
        idx getConsensus(sStr &out,idx wrap = 0, idx mode = 0);


        idx countMatches(sBioseqAlignment::Al * hdr, idx * m, const char * sub, const char * qry, idx qrylen);

        idx printAlignmentSummaryBySubject(sVec < Stat > & statistics, sStr * str,ParamsAlignmentSummary * params);
        idx countAlignmentSummaryBySubject( sVec < Stat >  & statistics);
        sBioal::Stat getTotalAlignmentStats();
        sBioal::Stat getSubjectAlignmentStats(idx iSub);
        static idx printAlignmentHistogram(sStr * out , sDic < sBioal::LenHistogram > * lenHistogram );
        static idx printAlignmentCoverage(sStr * out , sDic < idx > * subCoverage);

        static idx countAlignmentLetters(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd);
        static idx getSaturation(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd);
        static idx printFastXSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd);
        static idx printSubSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd);
        static idx printMatchSingle(sBioal * bioal, ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iSub, idx iAlInd);
        static idx printAlignmentSingle(sBioal * bioal, ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iSub, idx iAlInd);
        static bool regexAlignmentSingle(sStr &compStr, sStr &out, idx start, idx end, ParamsAlignmentIterator * callbackParam);

        static idx printBEDSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd);

        static idx BioseqAlignmentComparator(void * parameters, void * hdrA, void * hdrB, void * arr,idx i1,idx i2);

        idx bSearchAlignments(idx iAlPoint,idx iAlmin, idx iAlmax, sSort::sSearchHitType hitType = sSort::eSearch_First);
        idx iterUnAligned(idx * iVis,idx start, idx cnt,ParamsAlignmentIterator * params);

        idx remap( sBioal * mutual, sVec<idx> &remappedHits, idx * alSortList = 0);
        idx stableRemap( sBioal * mutual, sVec<idx> &remappedHits, idx start = 0, idx cnt = 0, idx * alSortList = 0, sBioseqAlignment * seqAl = 0);

        typedef idx (* sBioalSorterFunction)(void * param, void * ob1, void * obj2 , void * objSrc,idx i1,idx i2);

        struct ParamsAlignmentSorter{
            idx flags;
            sBioal * bioal;
            void * extCompParams;

            sBioalSorterFunction extComparator;
            ParamsAlignmentSorter(){
                flags=alSortByPosStart;
                bioal=0;
                extCompParams=0;extComparator=0;
            }

        };

        enum fSortAlignmentFlags {
            alSortByPosEnd          =0x001,
            alSortByPosStart        =0x002,
            alSortBySubID           =0x004,
            alSortByQryID           =0x008,
            alSortByPosONSubID      =0x020,
            alSortBySubIdONPos      =0x040
        };

        sBioseqAlignment::Al & operator []( idx index){return *getAl(index);}

};

class sBioalSet : public sBioal
{
        struct RefAl {
            idx bioNum;
            idx alNum;
            idx alCnt;
        };

        sVec < sBioal * > biosR;
        sVec < RefAl > refs;

        sVec < idx > alInd;
        idx totDim;
        idx totSubjects;
        bool needsReindex;

        sVec < Stat > allStat;
        sVec < idx > dimSubCnt;


    public:

        sBioalSet()
            : totDim(0), totSubjects(0), needsReindex(false), allStat(sMex::fSetZero), dimSubCnt(sMex::fSetZero)
        {
        }

        void attach(sBioal * bioal, idx alNum = 0, idx alCnt = sIdxMax);
        void reindex(void);
        sBioal * ref(idx * inum, idx iSub = sNotIdx);

   public:
        virtual idx dimAl(void){return totDim;}
        virtual sBioseqAlignment::Al * getAl(idx iAlIndex){
            return ref(&iAlIndex)->getAl(iAlIndex);
        }
        virtual idx * getMatch (idx iAlIndex) {
            return ref(&iAlIndex)->getMatch(iAlIndex);
        }
        virtual const char * getSAMContent(idx iAlIndex, idx * size){
            return ref(&iAlIndex)->getSAMContent(iAlIndex, size);
        }
        virtual idx getRpt (idx iAlIndex) {
            idx rpt = ref(&iAlIndex)->getRpt(iAlIndex);
            if( rpt ) return rpt;
            if( Qry ) return Qry->rpt(getAl(iAlIndex)->idQry());
            return 0;
        }
        virtual idx dimStat()
        {
            return 1;
        }
        virtual Stat * getStat(idx iStat = 0, idx iSub = 0, idx * size = 0)
        {
            if( iSub > allStat.dim() )
                return 0;
            Stat * pI = (Stat *) allStat.ptr(iSub);
            if( size )
                *size = allStat.dim();
            return pI;
        }
        virtual idx listSubAlIndex(idx idSub, idx * relCount )
        {
            if( idSub >= dimSubCnt.dim() / 2 ) {
                if( relCount )
                    *relCount = 0;
                return 0;
            }
            if( relCount ) {
                *relCount = dimSubCnt[(idSub) * 2];
            }
                return dimSubCnt[(idSub)*2+1];
        }
        virtual void setMode(sBioseq::EBioMode qrymode, sBioseq::EBioMode submode)
        {
            for(idx ir = 0; ir < biosR.dim(); ++ir) {
                biosR[ir]->setMode(qrymode, submode);
            }
        }
        virtual sBioseq::EBioMode getQryMode(void)
        {
            for(idx ir = 0; ir < biosR.dim(); ++ir) {
                if( biosR[0]->getQryMode() == sBioseq::eBioModeLong ) {
                    return sBioseq::eBioModeLong;
                }
            }
            return sBioseq::eBioModeShort;
        }
        virtual sBioseq::EBioMode getSubMode(void)
        {
            for(idx ir = 0; ir < biosR.dim(); ++ir) {
                if( biosR[0]->getSubMode() == sBioseq::eBioModeLong ) {
                    return sBioseq::eBioModeLong;
                }
            }
            return sBioseq::eBioModeShort;
        }
        virtual bool isPairedEnd()
        {
            if( Sub ) {
                idx size;
                getStat(0, 0, &size);
                return size == 2 * Sub->dim() + 1;
            }
            bool rs = true;
            for(idx ir = 0; ir < biosR.dim(); ++ir) {
                rs &= biosR[ir]->isPairedEnd();
            }
            return rs;
        }
};

class sBioAlBlast
{
    public:
        typedef enum eBlastOutmode_enum
        {
            eBlastStandardOut = 0,
            eBlastProteinOut = 1,
            eBlastBlatOut = 2
        } eBlastOutmode;

        static idx SSSParseAlignment(sIO * log, char * fileContent, idx filesize, sVec<idx> * alignOut, idx scoreFilter, idx minMatchLength, bool isMinMatchPercentage,  idx maxMissQueryPercent, sDic<idx> * rgm, sDic<idx> * sub = 0, sDic<idx> * qry = 0, idx blastOutMode = eBlastStandardOut, sDic<
            idx> *unalignedQry = 0);
        static const char * readVal(char * src, const char * find, const char * fmt, void * pVal);
        static const char * getNextLine(sStr * Buf, const char * const startPtr, const idx sizeLeftover);
};

typedef struct
{
        idx row;
        bool aligned;
        bool paired;
        bool firstRead;
        bool mateAligned;

        idx samFlag() {
            idx flag = 0;
            if ( !aligned ) {
                flag |= sBioal::eSF_ReadUnmapped;
            }
            if ( paired ) {
                flag |= sBioal::eSF_ReadPaired;
                if ( firstRead ) {
                     flag |= sBioal::eSF_FirstInPair;
                } else {
                     flag |= sBioal::eSF_SecondInPair;
                }
                if ( aligned && mateAligned ) {
                    flag |= sBioal::eSF_ReadMappedProperPair;
                }
                if ( !mateAligned ) {
                    flag |= sBioal::eSF_MateUnmapped;
                }
            }
            return flag;
        }
} AlRead;

class AlBitMap
{
    protected:
        sVec<char> bitMap;
        sBioseq::EBioMode mode;
    public:
        AlBitMap() : bitMap(), mode(sBioseq::eBioModeShort)
        {
        }
        void build(sBioal & al, idx qryDim)
        {
            mode = al.getQryMode();
            bitMap.cut(0);
            bitMap.addM(qryDim/8+1);
            bitMap.set(0);
            idx qy=0;
            for(idx iAl=0; iAl < al.dimAl(); ++iAl){
                qy = al.getAl(iAl)->idQry();
                bitMap[qy/8] |= ((idx(1))<<((qy%8)));
            }
        }
        bool isAligned(idx qryId, sBioseq & qry) {
            if ( getMode() == sBioseq::eBioModeShort ) {
                qryId = qry.long2short(qryId);
            }
            return bitMap[qryId/8] & (((idx)1)<<(qryId%8));
        }
        sBioseq::EBioMode getMode() { return mode; }
};

class ReadIter
{
    public:
        ReadIter(sBioseq & qry, AlBitMap & alBitMap) : _qry(qry), _alBitMap(alBitMap), _pos(0) {}
        virtual ~ReadIter() {}
        idx pos() { return _pos; }
        idx next() { return ++_pos; }
        virtual idx dim() = 0;
        virtual void get(sVec<AlRead> & out) = 0;
    protected:
            sBioseq & _qry;
            AlBitMap & _alBitMap;
            idx _pos;
};

class UnpairedReadIter : public ReadIter
{
    public:
        UnpairedReadIter(sBioseq & qry, AlBitMap & alBitMap) : ReadIter(qry, alBitMap) {}
        virtual ~UnpairedReadIter() {}
        virtual idx dim() { return _qry.dim(); }
        virtual void get(sVec<AlRead> & out)
        {
            out.cut(0);
            out.add();
            out[0].paired = false;
            out[0].row = pos();
            out[0].aligned = _alBitMap.isAligned(pos(), _qry);
        }
};

class PairedReadIter : public ReadIter
{
    public:
        PairedReadIter(sBioseq & qry, AlBitMap & alBitMap) : ReadIter(qry, alBitMap) {}
        virtual ~PairedReadIter() {}
        virtual idx dim() { return _qry.dim() / 2; }
        virtual void get(sVec<AlRead> & out)
        {
            out.cut(0);
            out.add(2);

            out[0].row = pos();
            out[1].row = out[0].row + ( _qry.dim() / 2 );
            for (idx i = 0; i < out.dim(); i++) {
                out[i].paired = true;
                out[i].firstRead = ( i == 0 );
                out[i].aligned = _alBitMap.isAligned(out[i].row, _qry);
            }
            out[0].mateAligned = out[1].aligned;
            out[1].mateAligned = out[0].aligned;
        }
};

#endif

