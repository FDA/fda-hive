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

#ifndef HIVE_INCLUDE_VIOLIN_ALIGNPARSE_HPP_
#define HIVE_INCLUDE_VIOLIN_ALIGNPARSE_HPP_

#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <violin/hiveproc.hpp>

namespace sviolin
{

    void SAMHeaders(sUsrObj &obj, sFil & samHeaderPP, sFil & samFooterPP);

    class AlignParser
    {

        public:
            typedef sVec<idx> AlignMap;
            typedef sVec<idx> CoverageDict;

            AlignParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads) :
                _qp(qp), _subjects(subjects), _reads(reads) {}
            virtual ~AlignParser() {}

            virtual sRC joinAls(sVioal::digestParams & vioalParams, sUsrObj * obj = 0, const char * resultFileTemplate = 0) const;

        protected:
            sQPrideProc & _qp;

            sHiveseq & _subjects;
            sHiveseq & _reads;

            sRC filterAndWriteAlignMap(AlignMap & alignMap, idx queryStart, idx flagSet) const;

            sRC joinAlsHelper(sVioal::digestParams & vioalParams, sUsrObj * obj = 0, const char * resultFileTemplate = 0, const char * samFilelist00 = 0, bool generateCoverageDict = true) const;
    };

    class AlignMapParser : public AlignParser
    {

        public:
            AlignMapParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads) :
                AlignParser(qp, subjects, reads) {}
            virtual ~AlignMapParser() {}
            sRC writeAls(AlignMap & inMap, idx alignMapQueryStart, idx flagSet, idx & countAls, sStr & errMsgBuf) const;

    };

    class FileAlParser : public AlignParser
    {

        public:

            struct WriteParams {
                idx minMatchLength;
                idx maxMissQueryPercent;
                idx isMinMatchLengthPercentage;
                idx scoreFilter;
                idx flagSet;
            };

            FileAlParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads) :
                AlignParser(qp, subjects, reads) {}
            virtual ~FileAlParser() {}
            virtual sRC writeAls(const char * fileList00, const WriteParams & writeParams, idx & countAls, sStr & errMsgBuf, sDic<idx> * idMap = NULL, sDic<idx> * unalignedList = NULL, sDic<idx> * subIds = NULL) const = 0;

    };

    class MultipleAlParser : public FileAlParser
    {

        public:
            MultipleAlParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads, bool withIdlines)  :
                FileAlParser(qp, subjects, reads), _withIdlines(withIdlines) {}
            MultipleAlParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads)  :
                FileAlParser(qp, subjects, reads), _withIdlines(true) {}
            virtual ~MultipleAlParser() {}
            virtual sRC writeAls(const char * fileList00, const WriteParams & writeParams, idx & countAls, sStr & errMsgBuf, sDic<idx> * idMap = NULL, sDic<idx> * unalignedList = NULL, sDic<idx> * subIds = NULL) const;
            virtual sRC joinAls(sVioal::digestParams & vioalParams, sUsrObj * obj = 0, const char * resultFileTemplate = 0) const;

        protected:
            const bool _withIdlines;

    };

    class SAMParser : public FileAlParser
    {

        public:
            SAMParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads, bool useRowInformationtoExtractQry)  :
                FileAlParser(qp, subjects, reads), _useRowInformationtoExtractQry(useRowInformationtoExtractQry) {}
            SAMParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads)  :
                FileAlParser(qp, subjects, reads), _useRowInformationtoExtractQry(false) {}
            virtual ~SAMParser() {}
            virtual sRC writeAls(const char * fileList00, const WriteParams & writeParams, idx & countAls, sStr & errMsgBuf, sDic<idx> * idMap = NULL, sDic<idx> * unalignedList = NULL, sDic<idx> * subIds = NULL) const;
            virtual sRC joinAls(sVioal::digestParams & vioalParams, sUsrObj * obj = 0, const char * resultFileTemplate = 0) const;

        protected:
            const bool _useRowInformationtoExtractQry;
            sRC joinAndDumpSAMHeaders(const char * srcSamFiles00, sFil & outFile, sUsrObj * obj) const;
            sRC concatAndDumpSAMFooters(sUsrObj * obj) const;

    };

    class BLASTParser : public FileAlParser
    {

        public:
            BLASTParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads, sBioAlBlast::eBlastOutmode blastMode) :
                FileAlParser(qp, subjects, reads), _blastMode(blastMode) {}
            BLASTParser(sQPrideProc & qp, sHiveseq & subjects, sHiveseq & reads) :
                FileAlParser(qp, subjects, reads), _blastMode(sBioAlBlast::eBlastStandardOut) {}
            virtual ~BLASTParser() {}
            virtual sRC writeAls(const char * fileList00, const WriteParams & writeParams, idx & countAls, sStr & errMsgBuf, sDic<idx> * idMap = NULL, sDic<idx> * unalignedList = NULL, sDic<idx> * subIds = NULL) const;

        protected:
            const sBioAlBlast::eBlastOutmode _blastMode;

    };

}



#endif 