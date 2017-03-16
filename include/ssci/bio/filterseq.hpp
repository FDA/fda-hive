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
#ifndef sLib_utils_filter_hpp
#define sLib_utils_filter_hpp

#include <slib/core/vec.hpp>
#include <slib/core/dic.hpp>
#include <ssci/bio/bioseq.hpp>

namespace slib
{
    class sFilterseq {
        public:
            struct filterParams{
                idx complexityWindow;
                real complexityEntropy;
                // developer: feel free to add more variables to this structure if necessary
            };
            typedef idx (*callbackType)(void * param, idx countDone, idx curPercent, idx maxPercent);

            static idx complexityFilter( const char * seq, idx len ,idx complexityWindow, real complexityEntropy, bool isCompressed = true, idx optiontoCut = 0, bool considerNs = false );
//            static idx complexityStartFilter( const char * seq, idx len ,idx complexityWindow, real complexityEntropy, bool isCompressed);
//            static idx complexityEndFilter( const char * seq, idx len ,idx complexityWindow, real complexityEntropy, bool isCompressed);
//            static bool complexityFilter2( const char * seq, idx len ,idx complexityWindow, real complexityEntropy , idx complexityCharLen=1);
            static bool complexityFilter_wholeSeq_ChunkSize( const char * seq, idx len , idx chunkSize, real complexityEntropy, bool considerNs = false);


            static bool qualityFilter ( const char * qua, idx len ,idx threshold, real percentage = 1.0);
            static idx trimQuality(const char *qua, idx len, idx threshold, idx minlen = 1);
            static bool trimPosition(idx startseq, idx endseq, idx start, idx end);
            static idx primersFilter(const char *seq, idx seqlen, const char *primer, idx prlen, idx minLength, idx maxMissMatches, bool reverse, bool keepIfMid, bool keepIfEnd, idx windowLen);
//            static void randomizer(sFil &fp, sBioseq &Sub, idx num, idx noise, idx minValue, idx maxValue, const char * strsettings, const char *strmutations);
            static void randomizer(sFil &QP, sBioseq &sub, idx numReads, real noise, idx minValue, idx maxValue, bool isNrevComp, bool isQual, bool isLengthNormalized, const char * strsettings, const char *strmutations,idx complexityWindow = 0, real complexityEntropy = 0,void * myCallbackParam = 0,callbackType myCallbackFunction = 0  );
            //static sVec <idx> complexityFilterPrecomputedArray;
            static void randomizer2vioseq(sBioseq &sub, const char * output, idx numReads, real noise=0, idx minValue=0, idx maxValue=0, bool isNrevComp=false, bool isQual=false, const char * strsettings=0, const char *strmutations=0);
            static void randomizer(sBioseq &sub, const char * output, idx numReads, real noise=0, idx minValue=100, idx maxValue=100, bool isNrevComp=false, bool isQual=false, const char * strsettings=0, const char *strmutations=0);
            static idx k_match(const char* s1, const char* q1, idx len1, const char* s2, const char* q2, idx len2, idx max_mismatch, char q_cut);
//            static void randomizer(const char * input, const char * output, idx numReads, real noise=0, idx minValue=0, idx maxValue=0, bool isNrevComp=false, bool isQual = false, const char * strsettings=0, const char *strmutations=0);
            static idx adaptiveAlignment (const char *sub, idx sublen, const char *qry, idx qrylen, idx cfmatch, idx cfmismatch, idx cfindel, idx *resPos, idx *costMatrix, idx *dirMatrixconst);
            static idx posMatrix(idx i, idx j, idx len, idx *mat);

            // Create a dictionary with id's of the bioseq object
            static bool parseDicBioseq(sDic <idx> &idDic, sBioseq &sub, sStr *errmsg = 0, bool attemptSubStrings = true);
            static idx getrownum(sDic <idx> &idDic, const char *id, idx idlen = 0);

            static idx sequenceFilterStatic(void *param, sStr * buf, idx initid, idx initseq, idx initqua, idx seqlen)
            {
                // developer: feel free to add more options to the filter
                filterParams * par = static_cast<filterParams*>(param);
                const char *seq = buf->ptr(initseq);
                idx retval = complexityFilter(seq, seqlen, par->complexityWindow, par->complexityEntropy, false);
                return retval == -1 ? 0 : 1;
            }

    };

}

#endif // sLib_utils_filter_hpp
