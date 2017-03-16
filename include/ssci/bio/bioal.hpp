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

#include <ssci/bio/bioseqalign.hpp>
#include <regex.h>

class sBioal
{

    public:
        sBioseq * Sub; //!< The reference sequence in a sBioseq wrapper.
        sBioseq * Qry; //!< The query sequence in a sBioseq wrapper.

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
            eBioModeShortBoth=0x00, // as if bits...
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
        virtual idx listSubAlIndex(idx idSub, idx * relCount)
        {
            return 0;
        } // , sVec <idx> * indexList
        struct Stat
        {
                idx found;
                idx foundRpt;
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
        // For reporting progress
        typedef idx (*callbackTypeProgress)(void * param, idx countDone, idx curPercent, idx maxPercent);

        //! The parameters for the alignment iterator.
        struct ParamsAlignmentIterator{
            sStr * str; //!< An sStr parameter
            idx navigatorFlags;
            regex_t * regp;
            idx rangestart, rangeend,High,winSize,pageRevDir,subRealS,padding,indels,currentChunk,winTailLimit, maxAlLen;
            idx alCol; //!< which column holds the alignment (for regexping purposes)
//            sVec<idx> * SNPentr;
            const char * id;
            FILE * outF; //!< An output file stream.
            //void (* Bin)(const void * buf, idx len, const char * filename, ...);
            idx wrap;
            idx alignmentStickDirectional;
            idx rightTailPrint,leftTailPrint;

            void * userPointer; //!< Open pointer.
            idx userIndex; //!< Open int value.
            callbackTypeProgress reqProgressFunction;
            void * reqProgressParam;

            ParamsAlignmentIterator(sStr * lstr=0){
                sSet(this,0); // fill with zeros
                str=lstr;
                regp=0;
                High=-1;
                alignmentStickDirectional=0;
            }
        };

        typedef idx (*callbackType)(void * param, idx countDone, idx progressCur, idx percentMax);
        callbackType progress_CallbackFunction;
        void * progress_CallbackParam;

        // Dinos-TODO : need naming of these flags in vioalt.cpp
        enum fViewAlignmentFlags {
            alPrintSubject              =0x1,            //print Subject                                            AATGATCTT
            alPrintUpperInterm          =0x2,            //show intermediate row (alignment notation)               |||| ||||
            alPrintQuery                =0x4,            //print queries                                            ....G....
            alPrintLowerInterm          =0x08,           //reserved for similarity notations in amino acids (.:*)
            alPrintDotsFormat           =0x10,           //print dots/letters (for match/mismatch)
            alPrintVariationOnly        =0x20,           //print only reads with variation in the selected position
            alPrintTouchingOnly         =0x40,           //print only reads that touch the selected positionand not just in the window
            alPrintNonFlippedPosforRev  =0x80,           //print the position of a reverse alignment without flipping it
            alPrintMirroredPos          =0x100,          //print the position after mirroring in the center of the alignment
            alPrintRegExpSub            =0x200,          //print alignments matching regexp in their Subject
            alPrintRegExpQry            =0x400,          //print alignments matching regexp in their Query
            alPrintBasedOnRange         =0x800,          //print alignments based on range
            alPrintMode                 =0x1000,         //print in stack to file //include subject
            alPrintRegExpInt            =0x2000,         //print alignments matching regexp in their Intermediate -> || | |
            alPrintMutBiasOnly          =0x4000,         //create vector of entropy only //will be printed in CGI
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
            alConsensusOverlap          =0x2000000,      //Generate overlap, NOT consensus. Gaps will be substituted with the next most prominent letter.
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

        struct LenHistogram
        {
                idx cntRead, cntSeq, cntFailed, cntAl, cntLeft, cntRight,maxLeft,maxRight, lenAnisotropy;
                LenHistogram()
                {
                    sSet(this, 0);
                }
        };

        typedef idx (*typeCallbackIteratorFunction)(sBioal * bioal, ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd);
        idx iterateAlignments(idx * iVis, idx start, idx cnt, idx iSub, typeCallbackIteratorFunction callbackFunc, ParamsAlignmentIterator * callbackParam=0, typeCallbackIteratorFunction secondaryCallbackFunc = 0, ParamsAlignmentIterator * secondaryCallbackParam = 0, idx * sortArr = 0);
        idx getConsensus(sStr &out,idx wrap = 0, idx mode = 0);

        //char * printAlignmentSummaryBySubject(sVec < idx >  & statistics, sStr * str, bool reportZeroHits=false);
        typedef idx (*HitListExtensionCallback)(void * param, sIO * io, sBioseq * sub, sDic <sStr > * dic,const char * seqidFrom00, idx countResultMax, idx startResult, idx contSequencesMax, idx outPutHeader,sStr * hdr );

        idx printAlignmentSummaryBySubject(sVec < Stat > & statistics, sStr * str,ParamsAlignmentIterator * params,bool reportZeroHits, bool reportTotals, bool reportFailed,idx start=0,idx cnt=0,sVec<idx> * childPath=0, sVec<idx> * coverage = 0, sBioal::HitListExtensionCallback callBackExtension=0,  void * param=0);
        idx countAlignmentSummaryBySubject( sVec < Stat >  & statistics);
        static idx printAlignmentHistogram(sStr * out , sDic < sBioal::LenHistogram > * lenHistogram );
        static idx printAlignmentCoverage(sStr * out , sDic < idx > * subCoverage);

        static idx countAlignmentLetters(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd);
        static idx getSaturation(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd);
        static idx printFastaSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAlInd);
        static idx printFastXSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd);
        static idx printSubSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd);
        static idx printMatchSingle(sBioal * bioal, ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iSub, idx iAlInd);
        static idx printAlignmentSingle(sBioal * bioal, ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iSub, idx iAlInd);
        //static idx stackAlignmentSingle(sVioalt * vioalt, ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m , idx iNum);
        static bool regexAlignmentSingle(sStr &compStr, sStr &out, idx start, idx end, ParamsAlignmentIterator * callbackParam);

        // Print function for BED file output
        static idx printBEDSingle(sBioal * bioal, ParamsAlignmentIterator * params, sBioseqAlignment::Al * al, idx * m, idx iNum, idx iAlInd);

        static idx BioseqAlignmentComparator(void * parameters, void * hdrA, void * hdrB, void * arr,idx i1,idx i2);

        idx bSearchAlignments(idx iAlPoint,idx iAlmin, idx iAlmax, idx  sortIndFirst, idx isEnd=0);
        idx iterUnAligned(idx * iVis,idx start, idx cnt,ParamsAlignmentIterator * params);

        idx remap( sBioal * mutual, sVec<idx> &remappedHits, idx * alSortList = 0);

        typedef idx (* sBioalSorterFunction)(void * param, void * ob1, void * obj2 , void * objSrc,idx i1,idx i2);

        //! The parameters for the alginment sorter.
        struct ParamsAlignmentSorter{
            /*  0000 0000: sort by start position       0000 0100: sort by subId            0001 0000: sort first by start/end position/length alignment then query/sub Id
            *   0000 0001: sort by end position         0000 1000: sort by queryId
            */
            idx flags;
            sBioal * bioal;
            void * extCompParams;

            sBioalSorterFunction extComparator;
            ParamsAlignmentSorter(){
                flags=alSortByPosStart;
                bioal=0;
                extCompParams=0;extComparator=0;
            }

            /*sDic < sVec < idx > > *rIdMap;*/
        };

        enum fSortAlignmentFlags {
            //  0000 0000: sort by start position       0000 0100: sort by subId            0001 0000: sort first by start/end position/length aligment then query/sub Id
            //  0000 0001: sort by end position         0000 1000: sort by queryId
            alSortByPosEnd          =0x001,
            alSortByPosStart        =0x002,
            alSortBySubID           =0x004,
            alSortByQryID           =0x008,
            alSortByPosONSubID      =0x020,
            alSortBySubIdONPos      =0x040
        };


};

class sBioalSet : public sBioal
{
        // each record defines a multiple or a single sequences in the container
        struct RefAl {
            idx bioNum; // the serial number of sBioal in the biosR this alignment originates from
            idx alNum; // the first alignment being pushed
            idx alCnt; // how many is being pushed
        };

        sVec < sBioal * > biosR; // managed sBioal objects;
        sVec < RefAl > refs; // list of references from this bioseq

        sVec < idx > alInd;
        idx totDim; // total dimension
        idx totSubjects;
        bool needsReindex; // flag signifying the need to reindex the sBioseqSet

        sVec < Stat > allStat;
        sVec < idx > dimSubCnt;


    public:

        sBioalSet()
            : totDim(0), totSubjects(0), needsReindex(false), allStat(sMex::fSetZero), dimSubCnt(sMex::fSetZero)
        {
        }

        void attach(sBioal * bioal, idx alNum = 0, idx alCnt = sIdxMax);
        void reindex(void); // re-indexing function
        sBioal * ref(idx * inum, idx iSub = sNotIdx); // returns the sBioal * object and its offset

   public:
        virtual idx dimAl(void){return totDim;}
        virtual sBioseqAlignment::Al * getAl(idx iAlIndex){
            return ref(&iAlIndex)->getAl(iAlIndex);
        }
        virtual idx * getMatch (idx iAlIndex) {
            return ref(&iAlIndex)->getMatch(iAlIndex);
        }

        virtual idx getRpt (idx iAlIndex) {
            idx rpt = ref(&iAlIndex)->getRpt(iAlIndex);
            if( rpt ) return rpt;
            if( Qry ) return Qry->rpt(getAl(iAlIndex)->idQry());
            return 0;
        }
        virtual idx dimSub()
        {
            return dimSubCnt.dim() / 2;
        }
        virtual idx dimStat()
        {
            return 1; //allStat.dim();
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
        virtual idx listSubAlIndex(idx idSub, idx * relCount ) // , sVec < idx > * alIndexes
        {
            if( idSub >= dimSubCnt.dim() / 2 ) {
                if( relCount )
                    *relCount = 0;
                return 0;
            }
            if( relCount ) {
                *relCount = dimSubCnt[(idSub) * 2];
            }
            // if no array is provided for data - just put the pointer to the first
            //if( !alIndexes || *relCount==0)
                return dimSubCnt[(idSub)*2+1];
/*
            idx first=dimSubCnt[idSub*2+1];
            idx * cpy=alIndexes->add(*relCount);
            for(idx ip=0; ip<*relCount; ++ip )
                cpy[ip]=first+ip;
            return cpy;
            */
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
        //If at least one bioal is does not include paired end then hiveal is NOT an eligible
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

        static idx SSSParseAlignment(sIO * log, char * fileContent, idx filesize, sVec<idx> * alignOut, idx scoreFilter, idx minMatchLength, idx maxMissQueryPercent, sDic<idx> * rgm, sDic<idx> * sub = 0, sDic<idx> * qry = 0, idx blastOutMode = eBlastStandardOut, sDic<
            idx> *unalignedQry = 0);
        static const char * readVal(char * src, const char * find, const char * fmt, void * pVal);
        static const char * getNextLine(sStr * Buf, const char * currentPtr, idx sizeLeftover);
};

#endif

