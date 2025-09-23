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
#ifndef sLib_viosam_hpp
#define sLib_viosam_hpp

#include <slib/core.hpp>
#include <slib/utils.hpp>

#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqalign.hpp>
#include <ssci/bio/bioal.hpp>
#include <ssci/bio/bioseqsnp.hpp>

namespace slib {
    class sViosam
    {

        public:
            enum eSamFlags
            {
                 eSamMultisegs    = 0x1
                ,eSamAligned      = 0x2
                ,eSamUnmapped     = 0x4
                ,eSamNextSeg      = 0x8
                ,eSamRevComp      = 0x10
                ,eSamNextRevComp  = 0x20
                ,eSamFirstNextSeg = 0x40
                ,eSamLastSeg      = 0x80
                ,eSamSecAlign     = 0x100
                ,eSamFilters      = 0x200
                ,eSamPCR          = 0x400
                ,eSamSupAlign     = 0x800
            };
            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);

            idx ParseAlignment(const char * fileContent, idx filesize, sVioDB &db, sFil & baseFile, sVec<idx> * alignOut, sDic<idx> * rgm);
            idx ParseSamFile(char * fileContent, idx filesize, sVec<idx> * alignOut, const char * vioseqFilename = "", sDic<idx> *rgm = 0);
            idx convertVarScan2OutputintoCSV(const char * varscanFile, const char * csvFileTmplt, sBioseq * Sub, bool skipHeader = false);
            idx convertVCFintoCSV(const char * vcfFile, const char * csvFileTmplt, sBioseq * Sub, bool skipHeader = false);
            sVioDB vioDB;
            enum eRecTypes
            {
                eRecID_TYPE = 1, eRecREC_TYPE, eRecSEQ_TYPE, eRecQUA_TYPE
            };
            static idx convertVioaltIntoSam(sBioal *bioal, sFil & samHeader, sFil & samFooter, idx subId = -1, sBioseq *Qry = 0, sBioseq *Sub = 0, bool originalrefIds = true, const char * outputFilename = 0, FILE * fstream = 0, void * myCallbackParam=0,callbackType myCallbackFunction=0);

            static void convertDIProfIntoSam(sBioal *bioal, bool originalrefIds, const char * outputFilename, FILE * fstream, sVec<idx> * readsArray); 

            static idx convertSNPintoVCF(sBioseqSNP::SNPRecord * snpRecord, sBioseqSNP::ParamsProfileIterator * params, idx iNum = 0);
            static idx createVCFheader(FILE * stream, const char * refname = 0, real threshold = 0.5);

            static void printSam ( sBioseq *Qry, sStr * samData, sStr *outFile);

            static void printHeaderHD(sStr & out) { out.printf("@HD\tVN:1.0\tSO:unsorted\n"); }

            static void printHeaderSQ(sStr & out, sBioseq & Sub, idx i, bool originalIds) {
                sStr buf;
                if ( originalIds )
                    sString::copyUntil(&buf, Sub.id(i), 0, " ");
                else
                    buf.addNum(i + 1);
                buf.add0(2);
                out.printf("@SQ\tSN:%s\tLN:%" DEC "\n", buf.ptr(0), Sub.len(i)); 
            }

        private:

            struct RecSam
            {
                    idx lenSeq;
                    idx countSeq;
                    idx ofsSeq;
            };

            char * cigar_parser(const char * ptr, const char * lastpos, sVec<idx> * alignOut, idx * lenalign, idx * qryStart);
            char * scanUntilLetterOrSpace(const char * ptr, idx * pVal, const char * lastpos);
            char * scanNumUntilEOL(const char * ptr, idx * pVal, const char * lastpos);
            char * skipBlanks(const char * ptr, const char * lastpos);
            static char * skipUntilEOL(const char * ptr, const char * lastpos);
            char * scanAllUntilSpace(const char * ptr, sStr * strVal, const char * lastpos);
            char * scanAllUntilSpace(const char * ptr, const char * lastpos);

            static idx vioaltIteratorFunction(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum, idx iAl = 0 );

    };

}
#endif

