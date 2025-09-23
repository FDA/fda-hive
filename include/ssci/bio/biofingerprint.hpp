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
#ifndef sLib_sBioFingerprint_hpp
#define sLib_sBioFingerprint_hpp

#include <slib/core/vec.hpp>
#include <slib/std/file.hpp>
#include <ssci/bio/bioseq.hpp>

namespace slib
{
    class sBioFingerPrint {

        private:

            struct Chromosome
            {
                idx chromosomeID;
                idx _tableOffset;
                idx startCompile, cntCompile;
            };

            struct ChromInfo {
                    Chromosome * chr0;
                    udx * pattern;
                    idx *bitsCountOffset;
            };

            sVec <ChromInfo> chromList;
            sFil * cFl;
            sFil * c2Fl;

            sVec <udx> container;
            udx *contPointer;

            sStr tFl;
            ChromInfo tList;
            sVec <idx> distMatrix;
            sVec <idx> bitC;
            idx chrToSort, lenOfSegs, numOfSegs;

            ChromInfo * addChromosome (idx chrID, idx seqlen);

            idx initPopulation (ChromInfo *chr1, idx num);
            idx initIndividual (ChromInfo *chr, udx * ind, bool useBioseq = false);
            udx initMask (idx num);
            idx populateTable (ChromInfo *chr0, idx num);
            idx countBits (ChromInfo *chr, idx num);
            idx generateNewSolution (ChromInfo *chr, bool randomgenerated = true, sVec <idx> * listPop = 0);
            idx getPatRepresentation (sStr *buf, ChromInfo *chr, idx num);
            idx calcPopFitness (ChromInfo *chr, sVec <idx> * listPop = 0);
            idx sortPopulation (ChromInfo *chr, sVec <idx> * listPop);

            idx calcDistance (udx *i1, udx *i2, idx len);

            real getFitness (ChromInfo *chr, idx ind);
            idx printPopulation (sStr *buf, ChromInfo *chr, sVec <idx> *listPop = 0, idx start = 0, idx cnt = 0);
            idx printIndividual (sStr *dest, ChromInfo *chr, idx ind);
            idx selection (idx popNum, idx *listPop);
            idx generateNewPopulation(ChromInfo *chr, idx *listPop);
            idx recombination (udx *parent1, udx *parent2, udx *child1, udx *child2);
            idx mutate (udx *individual, real pmut);

            static idx bioFingerComparator(sBioFingerPrint * myThis, void * arr, udx i1, udx i2);
            inline static udx read32letters(idx position, const idx * binarysequence, idx len)
            {
                if( position % 32 == 0 && (len - position + 1) > 31 )
                    return binarysequence[position / 32];

                udx k = binarysequence[position / 32];
                udx l = binarysequence[(position / 32) + 1];
                udx shift_k = (position % 32) * 2;

                k >>= shift_k;
                l <<= (64 - shift_k);

                k |= l;

                return k;
            }
            idx segsCount(Chromosome *chr1){
                return (chr1->cntCompile - chr1->startCompile - 1) / hdr->segmentSize + 1;
            }

            idx moveIndividual (ChromInfo *chr, idx dst, idx src);
            void selectBest(ChromInfo *chr, idx *sortList, idx best, idx total);
            void transpose32(udx *A);
            void transpose64(udx *A);
            idx translateTable (ChromInfo *chr, udx *pat_table, idx numSegs, idx lenPat);

            idx readChromosomes();
        public:
            struct Hdr {
                idx version;
                idx reserved[32];
                idx chromCnt;
                idx sizeofPattern;
                idx segmentSize;
                idx numPatterns;
                idx lenPatterns;
                idx cntbitmask;
            };

            Hdr *hdr;
            sBioseq *Sub;

            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);

            sBioFingerPrint()
                : cFl(0), c2Fl(0), contPointer(0), chrToSort(0), lenOfSegs(0), numOfSegs(0), hdr(0), Sub(0)
            {
            }

            sBioFingerPrint(sBioseq *sub, idx start=0, idx cnt=0){
                idx chrStart;
                idx chrCnt;
                if (start > sub->dim() || start < 0){
                    return ;
                }
                if (cnt == 0){
                    chrCnt = sub->dim() - start;
                }
                if (start + chrCnt > sub->dim()){
                    return;
                }

                init(sub, chrStart, chrCnt);
            }

            sBioFingerPrint(const char *filename, sBioseq *s, idx numP){
                init(filename, s, 0, numP);
            }

            ~sBioFingerPrint(void){
                if(cFl)delete cFl;
                if(c2Fl)delete c2Fl;
            }

            void initHdr (Hdr *hd, idx numP, idx segSize){
                hd->version = 0;
                hd->chromCnt = 0;
                hd->numPatterns = numP;
                hd->segmentSize = segSize;
                hd->lenPatterns = 32;
                hd->cntbitmask = 8;
                hd->sizeofPattern = ((hdr->lenPatterns*2 - 1) / 64) + 1;

            }
            sBioFingerPrint * init(sBioseq * s, idx start, idx cnt);
            sBioFingerPrint * init (const char *filename, sBioseq * s, idx readorwrite, idx numP){
                Sub = s;
                idx fflags = readorwrite ? 0 : sMex::fReadonly;
                if (Sub->dim() == 0){return 0;}

                cFl = new sFil(filename,fflags);
                if (!cFl || !cFl->ok() ){ return 0; }

                sFilePath file2 (filename, "%%dir/%%flnmx_tables.bfp");
                c2Fl = new sFil(file2.ptr(),fflags);
                if (!c2Fl || !c2Fl->ok() ){ return 0; }

                container.mex()->flags |= sMex::fSetZero;
                if (readorwrite == 1){
                    cFl->cut(0);
                    c2Fl->cut(0);
                    idx segSize = 10000;
                    hdr = (Hdr *)cFl->add(0, sizeof(Hdr));
                    initHdr(hdr, numP, segSize);
                    contPointer = container.ptr(0);
                }
                else {
                    hdr=(Hdr*)cFl->ptr(0);
                    contPointer=(udx*)c2Fl->ptr(0);
                    bitC.cut(0);
                    readChromosomes ();
                }

                return this;
            }


            idx bcmin(idx a, idx b) {
                return a<b?a:b;
            }

            idx dim (){
                return hdr->chromCnt;
            }

            idx getSegmentCount (idx num){
                return segsCount(chromList.ptr(num)->chr0);
            }

            idx compileFingerPrint (idx ){
                idx success = 0;
                return success;
            }

            idx compileChromosome(idx chrID, idx generationNum);

            ChromInfo * getChromosome (idx num)
            {
                return chromList.ptr(num);
            }

            idx getLenPatterns (ChromInfo * chr)
            {
                return hdr->lenPatterns;
            }

            udx * getPattern (ChromInfo *chr0, idx ipat)
            {
                udx *i1 = chr0->pattern;
                idx t = ipat*(hdr->sizeofPattern * 2);
                return i1+t;
            }

            udx * getPMask (ChromInfo *chr0, idx ipat)
            {
                return getPattern(chr0,ipat)+hdr->sizeofPattern;
            }

            udx * getRowTable (ChromInfo *chr, idx num)
            {
                return contPointer + (chr->chr0->_tableOffset + num * ((segsCount(chr->chr0) - 1) / 64 + 1));
            }

            idx bitSetCount64(idx i);
            idx finalize();

            idx getNumPatterns(idx i){
                return hdr->numPatterns;
            }

            const char * printPatterns (sStr * buf, idx i){
                ChromInfo *chr = getChromosome(i);
                printPopulation(buf, chr);\
                return buf->ptr();
            }

            bool findStringPattern(udx *pat, const char * seq, idx seqlen);

            idx findPatRepresentation(sStr *str, idx num, udx *result, bool isStrCompress = true){
                ChromInfo *chr = getChromosome(num);
                idx numP = getNumPatterns(0);

                char * dst = str->ptr();
                idx seqlen = str->length();
                if (!isStrCompress){
                    sStr buf;
                    buf.resize(seqlen/4+1);
                    dst = buf.ptr();
                    sBioseq::compressATGC_2Bit(dst, str->ptr(), seqlen);
                }

                idx getsizePatterns = numP / 64 + 1;

                for (idx i = 0; i < getsizePatterns; ++i){
                    result[i]= 0;
                }
                idx bitCount = 0;
                for (idx ip = 0; ip < numP; ++ip){
                    udx *pat = getPattern(chr, ip);
                    if (findStringPattern(pat, dst, seqlen))
                        {
                        idx ibyte = ip / 64;
                        idx ishift = ip % 64;
                        result[ibyte] |= (udx) (0x1) << ishift;
                        ++(bitCount);
                    }
                }
                return bitCount;
            }

            real presentInSegment (idx num, udx *result, idx seg)
            {
                ChromInfo *chr = getChromosome(num);
                udx *segsTable = getRowTable (chr, num);
                idx patLen = getLenPatterns(chr);
                idx option = 3;
                real res;
                if (option == 0){
                }
                else if(option == 1){
                }
                else {
                    for (idx i = 0; i < patLen; ++i){
                        res += bitSetCount64(result[i] & segsTable[i]);
                    }
                }

                return res;
            }
            bool getPosition (idx num, idx seg, idx *start, idx *cnt){
                return true;
            }
    };

}

#endif 