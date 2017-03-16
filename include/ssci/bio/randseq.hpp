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
#ifndef sLib_utils_randseq_hpp
#define sLib_utils_ranseq_hpp

#include <slib/core/vec.hpp>
#include <slib/core/sIO.hpp>
#include <slib/utils/tbl.hpp>
#include <slib/utils/sort.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/filterseq.hpp>
#include <ssci/bio/vioseq2.hpp>
#include <ion/sIon-core.hpp>
#include <ion/sIon.hpp>
#include <ssci/math/rand/rand.hpp>


namespace slib
{
    class sRandomSeq {
        public:
            enum eInSilicoFlags
            {
                eSeqNone = 0x00000000,
                eSeqNoId = 0x00000001,
                eSeqRev = 0x00000002,
                eSeqComp = 0x00000004,
                eSeqLowComplexity = 0x00000008,
                eSeqPrimers = 0x00000010,
                eSeqFastA = 0x00000020,
                eSeqFastQ = 0x00000040,
                eSeqPrintRandomReads = 0x00000080,
                eSeqPrintRecombinantsOnly = 0x00000100,
                eSeqPrintPairedEnd = 0x00000200,
                eSeqMutations = 0x00000400,
                eSeqNoise = 0x00000800,
                eSeqComplexityFilter = 0x00001000,
                eSeqParseMutationFile = 0x00002000,
                eSeqParseAnnotFile = 0x00004000,
                eSeqParseCNVFile = 0x00008000,
                eSeqParseTargetSeq = 0x000010000,
                eSeqFilterNs = 0x00020000,
                eSeqMimicQuality = 0x00040000,
                eSeqGenerateRandomMut = 0x00080000,
                eSeqPreloadedGenomeFile = 0x00100000,
                eSeqSetRandomSeed0 = 0x00200000
            };

            struct RefMutation
            {
                    RefMutation(){
                        mutationOffset = refBase = refNumSeq = 0;
                        refLength = position = frequency = 0;
                        quality = allele = mutBiasStart = 0;
                        mutBiasEnd = 0;
                    }
                    RefMutation operator= (const RefMutation& x){
                        mutationOffset = x.mutationOffset;
                        refBase = x.refBase;
                        refNumSeq = x.refNumSeq;
                        refLength = x.refLength;
                        position = x.position;
                        frequency = x.frequency;
                        quality = x.quality;
                        allele = x.allele;
                        mutBiasStart = x.mutBiasStart;
                        mutBiasEnd = x.mutBiasEnd;
                        groupid = x.groupid;
                        return *this;
                    }
                    idx mutationOffset;
                    char refBase;
                    idx refNumSeq;
                    idx refLength; // (default =1 ) The length of the reference that the mutation is pointing to.
                    idx position, frequency;
                    idx quality, allele;
                    idx mutBiasStart, mutBiasEnd;
                    idx groupid;
                    idx count, miss;
            };
            struct RangeSeq
            {
                    RangeSeq(){
                        hiveseqListOffset = sourceSeqnum = -1;
                        sourceStartRange = sourceEndRange = 0;
                        orientation = coverage = tandem = 0;
                        destSeqnum = destPosition = -1;
                        posidAppend = -1;
                        deleteRange = false;
                    }
                    bool loadRange(RangeSeq range){
                        memcpy(this, &range, sizeof(RangeSeq));
                        return true;
//                        return true;
                        hiveseqListOffset = range.hiveseqListOffset;
                        sourceSeqnum = range.sourceSeqnum;
                        sourceStartRange = range.sourceStartRange;
                        sourceEndRange = range.sourceEndRange;
                        orientation = range.orientation;
                        coverage = range.coverage;
                        tandem = range.tandem;
                        deleteRange = range.deleteRange;
                        destSeqnum = range.destSeqnum;
                        destPosition = range.destPosition;
                        return true;
                    }
                    idx hiveseqListOffset;  // pathfilename or object ID number
                    idx sourceSeqnum;  // row in the bioseq
                    idx sourceStartRange, sourceEndRange;
                    idx orientation;  // look at translateRevComp (0, 1, 2 or 3)
                    idx coverage;   //
                    idx tandem;     // repeated sequence, use for copy number variant
                    bool deleteRange;
                    idx destSeqnum;
                    idx destPosition;
                    idx posidAppend;
            };
            struct RefInfo
            {
    //            idx fileID, seqID;
                    idx rangeseqsOffset;
                    idx rangecnt;
    //            sHiveseq *refhiveseq;
                    idx coverage;
    //                idx minLength, maxLength;
                    idx mutInfoOffset, mutcnt;
                    idx diploidicity;
    //                idx quaType, numOfSegs;
            };
            struct AnnotQuery
            {
                    idx seqoffset;
                    idx idoffset;
                    idx typeoffset;
                    idx tandem;
                    idx before;
                    idx after;
                    void *extraInfo;
            };

        public:
            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);
            callbackType m_callback;
            void * m_callbackParam;

            sRandomSeq(): m_callback(0), m_callbackParam(0)
            {
                init();
            }
            ~sRandomSeq()
            {
//                destroy();
            }

        public:
            idx inSilicoFlags;
            sStr lowComplexityString;
            idx lowComplexityMin, lowComplexityMax, lowComplexityFreq, lowComplexityCnt;
            idx complexityWindow;
            real complexityEntropy;
            idx numReads;
            idx minLength, maxLength, lengthDistributionType;
            sStr primersString;
            idx primersMin, primersFreq, primersCnt;
            idx strandedness;
            idx minPEread, maxPEread;
            idx quaType;
            sTxtTbl *qualityTable;
            sTxtTbl *rangeExtraction;
            real noisePercentage;
            real noiseOriginalTable[5][6]; // 5 (ACGTN) and 6 (ACGT, insertion, deletion)
            real noiseAccumTable[5][6]; // 5 (ACGTN)
            idx qryFileCnt;
            idx filterNperc;
            char prefixID[100];

        public:
            bool useAllAnnotRanges;
            sVec <AnnotQuery> annotRanges;
            sVec <AnnotQuery> cnvRanges;
            sStr annotStringBuf;

            // Variables to generate random Mutations
            idx randMutNumber, randMutStringLen, randMutFreq;
            idx randseed0, idum;

            idx quaMinValue, quaMaxValue;
            sVec<RefInfo> refSeqs;  // Container for the references to create recombinants
            sVec<RangeSeq> rangeContainer;  // Container of ranges in targeting sequencing
            sVec<RefMutation> mutContainer; // Container for all the mutations to include
            sVec<idx> mutSortList;  // List of sorted mutations
            sDic<idx> idlist;       // Dictionary of id's of sBioseq object

            sStr mutationStringContainer;   // buffer to store mutations
            sStr hiveseqListContainer;  // buffer to store hiveseq string associated to refSeqs

            sBioseq *Sub;   // sBioseq object
            sIO auxbuf;

        public:
            void init()
            {
                lowComplexityString.cut(0);
                primersString.cut(0);
                refSeqs.cut(0);
                rangeContainer.cut(0);
                mutContainer.cut(0);
                mutSortList.cut(0);
                idlist.cut(0);
                mutationStringContainer.cut(0);
                hiveseqListContainer.cut(0);
                auxbuf.cut(0);
                annotRanges.cut(0);
                cnvRanges.cut(0);
                annotStringBuf.cut(0);
                Sub = 0;
                inSilicoFlags = 0;
                quaType = 0;
                qualityTable = 0;
                rangeExtraction = 0;
                prefixID[0] = 0;
                setRandSeed();
            }

            sTxtTbl * getrangeTable (){
                return rangeExtraction;
            }
            bool emptyIdDictionary(){
                idlist.cut(0);
                idlist.init();
                return true;
            }
            idx validate(idx num, idx low, idx high);
            bool validate(const char * str);
            bool mutationCSVoutput(sFil *out, sBioseq &sub, sStr &err);
            bool mutationVCFoutput(sFil *out, const char *refID, sBioseq &sub, sStr &err);
            bool normalizeTable();
            bool noiseDefaults();
//            idx parseAnnotation(sBioseq &sub, sStr &err);
            bool generateRecombination(sFil &out, sStr &err);
            bool generateQualities(const char *seq, sStr &qua, idx seqlen);
            void mutationFillandFix(sBioseq &sub);
            bool generateRandomMutations(sBioseq &sub, sStr &err);
            bool launchParser(const char *out, const char *flnm, sVioseq2 &v, sStr &errmsg);

            idx selectRandRow(real *prob, idx num, real ra = 0);
            idx randomize(sFil *out, sBioseq &sub, sStr &err, sFil *out2 = 0, const char * lazyLenPath = 0, sDic<idx> * cntProbabilityInfo = 0);

            static idx ionWanderCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults);
    //        idx traverserCallback(sIon * ion, sIonWander *ts, sIonWander::StatementHeader * traverserStatement, sIon::RecordResult * curResults);

            void translateRevComp(idx orientation, bool *isrev, bool *iscomp);
            idx preparePrimers();
            idx prepareLowComplexity();
            void applyNoise(sStr &buf, idx buflen);
            bool addPrimers(sStr &buf, idx buflen);
            int applySortMutations(sStr &buf, sStr &qua, idx buflen, idx irow, idx seqStart, sStr *id, idx seqlenDump = 0);
            bool addLowComplexity(sStr &buf, idx buflen);
            bool printFastXData(sFil * outFile, idx seqlen, const char *seqid, const char *seq, const char *seqqua, idx subrpt);
            real randDist(real rmin = 0, real rmax = 1, void *dist = 0);
            bool flipCoin(real pr, idx maxValue = 1);
            const char * generateRandString(sStr &auxbuf, idx randlength, idx tabu = -1);
            sTxtTbl * tableParser(const char * tblDataPath, const char * colsep, bool header, idx parseCnt);


//            idx getrownum(const char *id);
            void addMutation (RefMutation *dst, RefMutation *src, idx pos);
            bool readTableMutations(const char * tblDataPath, sBioseq *sub, sStr *errmsg);

            // Ascendent Order:
            // 1, if i1 is better than i2
            // -1, if i2 is better than i1
            static idx mutationComparator(sRandomSeq * myThis, void * arr, udx i1, udx i2);
            idx binarySearch(real * array, idx num, real target, bool returnIndexbeforeTarget = false);
            idx binarySearchMutation(idx target);

//            bool loadGeneralAttributes(const sUsrObjPropsTree *propsTree, sStr *err, const char *quaMimicTable = 0, sBioseq *qryPreloadedGenome = 0, bool useMutFile = false, const char *flnmTargetFile = 0, bool useIonAnnot = false);
            bool loadParticularAttributes (sVar &options, sStr *err, sBioseq &qry, bool useMutFile = false, bool useIonAnnot = false);
            bool addAnnotRange (AnnotQuery * annot, sStr &stringContainer, const char *seqid, const char *id, const char *type, idx tandem = 0, idx before = 0, idx after = 0, void *ptr = 0);
            bool addRefs(sStr *err);

            idx getFlags () {
                return inSilicoFlags;
            }

            void setFlags (idx flags){
                inSilicoFlags = flags;
            }

            void addFlags (idx flags){
                inSilicoFlags |= flags;
            }

            bool setPrefixID (const char *preid){
                idx idlen = sLen(preid);
                if (idlen > 100-1){
                    return false;
                }
                strcpy(prefixID, preid);
                prefixID[idlen] = 0;
                return true;
            }

            void setRandSeed (idx seed0 = 0)
            {
                if (seed0 <= 0){
                    seed0 = time(NULL);
                }
                srand(seed0);
                randseed0 = seed0;
                idum = randseed0;
            }

            bool outrowProbInfo (sFil *out, sDic<idx> *cntRowProb)
            {
                idx cnt;
                out->printf(0, "id, len, cnt\n");
                idx seqLen;
                for(idx icnt = 0; icnt < cntRowProb->dim(); ++icnt) {

                    cnt = *cntRowProb->ptr(icnt);
                    idx *key = (idx *) (cntRowProb->id(icnt));
                    RangeSeq *range = rangeContainer.ptr(*key);
                    idx irow = range->sourceSeqnum; // use the real irow from sBioseq
                    seqLen = range->sourceEndRange - range->sourceStartRange;
                    out->printf("%s, %" DEC ", %" DEC "\n", Sub->id(irow), seqLen, cnt);
                }
                return true;
            }
    };
}
#endif // sLib_utils_randseq_hpp
