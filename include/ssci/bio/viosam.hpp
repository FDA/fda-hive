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
    //! Class containing members to work with SAM and VCF formats, or to convert to these formats.
    /*!
     *  Uses vioalt internal objects as well as reference and query sequences to generate both SAM and VCF formats.
     *  Can also parse both SAM files and Alignments.
     */
    class sViosam
    {

        public:
            enum eSamFlags
            {
                 eSamMultisegs    = 0x1 // template having multiple segments in sequencing
                ,eSamAligned      = 0x2 // each segment properly aligned according to the aligner
                ,eSamUnmapped     = 0x4 // segment unmapped
                ,eSamNextSeg      = 0x8 // next segment in the template unmapped
                ,eSamRevComp      = 0x10 // SEQ being reverse complemented
                ,eSamNextRevComp  = 0x20 // SEQ of the next segment in the template being reverse complemented
                ,eSamFirstNextSeg = 0x40 // the first segment in the template
                ,eSamLastSeg      = 0x80 // the last segment in the template
                ,eSamSecAlign     = 0x100 // secondary alignment
                ,eSamFilters      = 0x200 // not passing filters, such as platform/vendor quality controls
                ,eSamPCR          = 0x400 // PCR or optical duplicate
                ,eSamSupAlign     = 0x800 // supplementary alignment
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
            /*!
             * Converts the Vioalt format into SAM format for export.
             * \param vioal The vioalt format alignment file.
             * \param subId The reference sequence ID to use.  A value of -1 means use all available reference sequences.  For example, a value of 2 might
             * refer to the second chromosome (depending on how the ID structure is set up for a particular reference).  Upon passing in the human genome as a reference,
             * the value of -1 would tell the program to use all chromosomes (or however it is broken down by ID in the \a Sub object.
             * \param Qry A pointer to the query sequence in sBioseq format.
             * \param Sub A pointer to the reference sequence in sBioseq format.
             * \param outputFilename The filename that the resulting SAM file should be output to.
             * \param fstream A FILE type that is not currently used (this is what the parameter's outF is set to in the event there is no outputFilename specified).
             * \param myCallbackParam A callback parameter that can be typecasted as a sQPrideProc object containing the reqID.
             * \param myCallbackFunction A callback function that meets the requirements of callbackType typedef.
             * \returns 1 if successful.
             */
            static idx convertVioaltIntoSam(sBioal *bioal, idx subId = -1, sBioseq *Qry = 0, sBioseq *Sub = 0, bool originalrefIds = true, const char * outputFilename = 0, FILE * fstream = 0, void * myCallbackParam=0,callbackType myCallbackFunction=0);
            /*! Converts a vioalt object into a VCF formatted file for export.
             * \param snpRecord A SNPRecord object that contains the SNPs at a particular position on a reference sequence.
             * \param params A parameter object of type ParamsProfileIterator.  This is used to pass in a number of parameter values to the
             * function as needed.  \a param.userIndex is used to denote the ID of the used reference sequence (an ID for a chromosome perhaps),
             * \a param.userPointer is a pointer to the sBioseq object representing the reference sequence.
             * \param iNum An optional argument that is currently not used.
             * \returns 1 if successful, -1 on failure.
             */
            static idx convertSNPintoVCF(sBioseqSNP::SNPRecord * snpRecord, sBioseqSNP::ParamsProfileIterator * params, idx iNum = 0);
            /*! Creates the header to a VCF file
             * \param stream A pointer to the stream that the header should be written to.
             * \param refname An optional field that captures where the data came from (NCBI, etc.)
             * \returns 1 if successful.
             */
            static idx createVCFheader(FILE * stream, const char * refname = 0, real threshold = 0.5);


        private:

            struct RecSam
            {
                    idx lenSeq;
                    idx countSeq;
                    idx ofsSeq; // used for record tracking first and then used as user data
            };

            char * cigar_parser(const char * ptr, const char * lastpos, sVec<idx> * alignOut, idx * lenalign, idx * qryStart);
            char * scanUntilLetterOrSpace(const char * ptr, idx * pVal, const char * lastpos);
            char * scanNumUntilEOL(const char * ptr, idx * pVal, const char * lastpos);
            char * skipBlanks(const char * ptr, const char * lastpos);
            char * skipUntilEOL(const char * ptr, const char * lastpos);
            char * scanAllUntilSpace(const char * ptr, sStr * strVal, const char * lastpos);
            char * scanAllUntilSpace(const char * ptr, const char * lastpos);

            static idx vioaltIteratorFunction(sBioal * bioal, sBioal::ParamsAlignmentIterator * param, sBioseqAlignment::Al * hdr, idx * m, idx iNum );

    };

}
#endif

