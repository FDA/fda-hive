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
#ifndef DNA_DENOVOEXTENSION_HPP_
#define DNA_DENOVOEXTENSION_HPP_

#include <slib/std.hpp>
#include <ssci/bio.hpp>
#include <qlib/QPrideProc.hpp>


class DnaDenovoAssembly
{
        struct seqfrag
        {
                idx seqpos;
                idx seqlen;
                idx seqrevpos;
                bool isReverse;
        };

        struct longSeq
        {
                sVec<seqfrag> contigs;
                idx contiglen;
                real thickness;
                idx isValid;
                idx nodeStraight;
                idx nodeReverse;
                idx link;
        };

        sVec <longSeq> seqOrder;

        struct seqlinkage
        {
                idx start;
                idx end;
                idx len;
                idx next;
                idx prev;
        };

        struct ContigRepresentation
        {
                idx acgt [4];
                idx consensus;
                idx sumcoverage;
                idx quality;
                void updateConsensus (){
                    idx max = 0;
                    for (idx ilet = 1; ilet < 4; ++ilet){
                        if (acgt[ilet] > acgt[max]){
                            max = ilet;
                        }
                    }
                    consensus = max;
                }
                idx freq(){
                    return ((acgt[consensus]*100)/sumcoverage);
                }
        };
        sVec <seqlinkage> seqLink;
        sVec <idx> seqContig;
        sVec <ContigRepresentation> seqContigContainer;
        idx sizeContigs;

        idx seqCount, seqLinkCount;
        idx finalseqCount;
        idx sizemer;
        idx lenfilter;
        idx rptfilter;
        bool firstStageOnly;
        idx outLengthFilter;
        idx missmatchesPercent;
        bool useNewContigContainer;


    protected:
        sQPrideProc * qp;

    public:
        sBioseq *QrySrc;
        idx dnaDenovoExtension(sBioseq &Qry, BioseqTree &tree, idx sizem, idx sizef, idx rptf, bool firstStage = false);
        idx dnaContigExtension(sBioseq &Qry, BioseqTree &tree, idx sizem, idx sizef, idx rptf, bool firstStage = false, sStr *err = 0);
        idx dnaDenovoExtension(sBioseq &Qry, BioseqTree &tree);

        DnaDenovoAssembly(sQPrideProc * qp)
        : qp(qp)
        {
            sBioseq::initModule(sBioseq::eACGT);
            sizemer = 15;
            lenfilter = 0;
            rptfilter = 0;
            firstStageOnly = false;
            useNewContigContainer = false;

        }

        void registerBioseq (sBioseq *Q){
            QrySrc = Q;
        }
        bool getValues(idx, idx, idx *, idx *, bool *);
        char seqB(sBioseq *Qry, idx contig, idx pos, idx *rpt = 0, idx *qua = 0);
        char seqsetRpt(sBioseq *, idx , idx, idx);
        void addnewSequence(idx pos, idx len, idx rpt, idx strNode, idx revNode = 0, idx seqRev = 0, idx validation = 1);
        void mergeSeq(idx prevpos, idx nextpos, idx sizemer, BioseqTree *revNode = 0, idx thisNode = -1);

        idx printResult(sFil * outFile, sBioseq *vio, idx limit = 0, bool printFromQuery = false, const char *seqid = 0);
        void printSequenceID(sFil * outFile, idx position, sBioseq *vio, bool printFromQuery = false, const char *seqid = 0);
        idx getfinalCount(){ return finalseqCount;}
        void cleanSeqs(void);
        idx seqsLength(idx);
        idx scoring(const char *qry, const char *sub, idx iqry, idx isub, idx ext);
        idx alignExtension(sBioseq *, idx, idx, idx);
        idx compareBit (char let1, char let2, idx value);
        idx getSubNode(sBioseq *Qry, BioseqTree *tree,  idx contigpos, idx start, idx sizemer);
        idx getSubNodeContig(BioseqTree *tree,  idx contigpos, idx start, idx sizemer);
        void getSequence (sStr *, sBioseq *, idx);

        void initContigs(){
            seqContig.mex()->flags |= sMex::fSetZero;
            seqContig.cut(0);
            sizeContigs = 2000;
            seqLinkCount = 0;
        }
        void initNewContigs(){
            useNewContigContainer = true;
            seqContigContainer.mex()->flags |= sMex::fSetZero;
            seqContigContainer.cut(0);
            sizeContigs = 2000;
            seqLinkCount = 0;
        }
        void initseqLinkage();
        void addSeqContig(sBioseq *Qry, idx contig);
        void addNewSeqContig(sBioseq *Qry, idx contig);
        void extendSeqContig(sBioseq *Qry, idx contig, idx nextcontig, idx startpos);
        void mergeonlySeqContig (sBioseq *Qry, idx contig, idx nextcontig, idx startpos);
        idx getContigRpt(idx contig, idx pos);
        char getContigLet(idx contig, idx pos);
        void setContigRpt(idx contig, idx pos, idx newrpt);
        void setContigLet(idx contig, idx pos, char newlet);
        void addNewContigLet(idx contig, idx pos, ContigRepresentation *r2);
        void setInitialParameters (idx sizem, idx sizef = 0, idx rptfil = 0, bool firstSt = false, idx len = 0){
            sizemer = sizem;
            lenfilter = sizef;
            rptfilter = rptfil;
            firstStageOnly = firstSt;
            outLengthFilter = len;
            setMissmatchesPercent (90);
        }
        void setMissmatchesPercent (idx mp){
            if (mp >= 0 && mp <= 100){
                missmatchesPercent = mp;
            }
            else {missmatchesPercent = 90;}
        }

        ContigRepresentation *getNewContigLet(idx contig, idx pos);

        void someStats(sFil *out, idx limit)
        {
            sVec<idx> resArr, indxArr;
            {
                sVec<idx> idxArr;
                idxArr.cut(0);
                indxArr.cut(0);
                idx count = 0;
                for(idx contig = 0; contig < seqCount; ++contig) {
                    if( seqOrder[contig].isValid && (seqOrder[contig].contiglen > limit) ) {
                        idxArr.vadd(1, seqOrder[contig].contiglen * -1);
                        indxArr.vadd(1, contig);
                        ++count;
                    }
                }
                resArr.resize(count);
                sSort::sort(count, idxArr.ptr(), resArr.ptr(0));
            }

            if (!resArr.dim()){
                return;
            }
            for(idx contig = 0; contig < 1; contig++) {
                idx icontig = indxArr[resArr[contig]];
                for(idx i = 0; i < seqOrder[icontig].contiglen; i++) {
                    ContigRepresentation *rep = getNewContigLet(icontig, i);
                    out->printf("%" DEC ",%" DEC ",%c,%" DEC ",%" DEC ",%" DEC ",%" DEC "\n", i + 1, rep->freq(), sBioseq::mapRevATGC[rep->consensus], rep->acgt[0], rep->acgt[1], rep->acgt[2], rep->acgt[3]);
                }
                return;
            }

        }

};



#endif 