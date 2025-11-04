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

#include <slib/core/str.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/vec.hpp>
#include <ulib/uobj.hpp>
#include <violin/hiveseq.hpp>

#include <bitset>

using namespace slib;

namespace diu_utils {
    struct Tier2DB
    {
        sFil seqFile;
    };

    void mapAcc2Tier2DB(const char* fileName, sVar * pAccToDBName);
    void mapGenomesToAccDB( sUsr* user ,sVar * pAccToDBName);
    void fillSequenceFiles(const char* accesionsFile, const char * basePath, sVar & acc2DB, sDic<Tier2DB>& dbNameToFileDic, sviolin::sHiveseq& sf);
    const char * closeDbFiles(sStr & list,sDic<Tier2DB>& dbNameToFileDic);

    idx createEventJson(const char * fl, JSNode & resinf, sStr & b, idx & cntEvents, idx & highestSeverity, bool isTax=true, sVar * acc2DB=0);
    idx createDIEventJson(const char * fl, JSNode & resinf, sStr & b, idx & cntEvents, sVar * acc2DB=0);

    struct  LLNL_BULL_INFO {
        idx refIdLen, linLen, detailsLen;
        idx score, taxId, serial;
        const char * refId, * lineage,*dbType, *details;
        char sev;
    };

    struct LLNL_BULL { 
        LLNL_BULL_INFO bi[2];
        idx readIdLen;
        const char * * readId;
    };

    idx llnl_tempDic(sTbl & tbl, sDic< LLNL_BULL > & dicReads, sVar * acc2DB, const char * dbType) ;
    idx llnl_tempOut(sFil & tsvFile, sVec < sDic< LLNL_BULL > > & dicReadsSet, sviolin::sHiveseq & hs, idx isPaired);

    class FormatOutput {
        public:

            struct ReadInfo {
            char * label;
            idx length;
            idx ref_num;
            idx tax_id;
            idx alignment_score;
            bool engineered = false;
            bool novel = false;
            bool concern = false;
            bool called = false;
            bool processed = false;
            char * lineage;
            char * details;
            idx direction;
            real max_score = 0.0;
        };


        FormatOutput() {};

        bool processFiles(sStr & outputFilePath, sStr & alMatchFilePath, sStr & concernFilePath, sviolin::sHiveseq &reads);

        private:
            const char * _hdr = "Read ID\tRead Length\tResult Code\tTax ID\tConfidence\tResult Details";
            sTbl csvTable;
            sTbl concernTable;

            void processCSV(const char * csvFilePath, const char * concernFilePath, const char * tsvFilePath, const char * errorFilePath, sDic<ReadInfo> &dic, idx &errCount, sviolin::sHiveseq & all_reads);
            void validateInitialRows(const char * filePath, idx rowsToCheck);
            idx constructResultCode(const ReadInfo &info, bool called);
            const char * getResultDetails(idx ResultInt, const char *existingDetails);
            idx findLastCommonAncestor(const char *lineage1, const char *lineage2);
            void outputSingleAlignedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info, idx combinedLength);
            void outputPairedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2, idx commonTaxID);
            void outputUnalignedRead(sFil &tsvFile, const char *readID, idx readLength);
            void outputUnalignedReadPair(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2);
    };

}
