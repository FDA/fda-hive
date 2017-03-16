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
#include <slib/std.hpp>
#include <qlib/QPrideProc.hpp>

#include <slib/utils.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>

class DnaHexagonCollector: public sQPrideProc
{
    public:
        DnaHexagonCollector(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);

        idx getTheFlags(void)
        {
            //idx isDoubleHash = formIValue("doubleHash") ? true : false;
            idx reverseEngine = formIValue("reverseEngine", 0);
            idx keepAllMatches = formIValue("keepAllMatches", 1);
            idx fragmentScore = formIValue("fragmentScore", 0);
            idx flagSet = sBioseqAlignment::fAlignForward;
            if( formIValue("isglobal") ) flagSet |= sBioseqAlignment::fAlignGlobal; //else flagSet|=sBioseqAlignment::fAlignLocal;
            if( formIValue("isbackward", 1) ) {
                flagSet|=sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement;
                if( formIValue("isbackward", 1) == 2 ) flagSet &= ~sBioseqAlignment::fAlignForward;
            }
            if( formIValue("isoptimize", 1) ) flagSet |= sBioseqAlignment::fAlignOptimizeDiagonal;
            if( formIValue("isextendtails") ) flagSet |= sBioseqAlignment::fAlignMaxExtendTail;
            if( formIValue("isCircular") ) flagSet |= sBioseqAlignment::fAlignCircular;
            if( formIValue("doubleHash") ) flagSet |= sBioseqAlignment::fAlignOptimizeDoubleHash;
            if( formIValue("alignmentEngine") == 0 ) flagSet |= sBioseqAlignment::fAlignIdentityOnly;

            if( formBoolValue("keepMarkovnikovMatches", false) ) flagSet |= sBioseqAlignment::fAlignKeepMarkovnikovMatch;

            if( formBoolValue("keepPairedOnly", false) ) flagSet |= sBioseqAlignment::fAlignKeepPairedOnly;
            if( formBoolValue("keepPairOnSameSubject", false) ) flagSet |= sBioseqAlignment::fAlignKeepPairOnSameSubject;
            if( formBoolValue("keepPairOnOppositeStrand", false) ) flagSet |= sBioseqAlignment::fAlignKeepPairDirectionality;

            if( formIValue("fragmentLengthMin", 0) || formIValue("fragmentLengthMax", 0) ||
                (flagSet&(sBioseqAlignment::fAlignKeepPairDirectionality|sBioseqAlignment::fAlignKeepPairOnSameSubject|sBioseqAlignment::fAlignKeepPairedOnly) ) ) {
                flagSet |= sBioseqAlignment::fAlignIsPairedEndMode;
            }

            if( keepAllMatches == 0 ) flagSet |= sBioseqAlignment::fAlignKeepFirstMatch;
            if( keepAllMatches == 1 ) flagSet |= sBioseqAlignment::fAlignKeepBestFirstMatch;
            if( keepAllMatches == 3 ) flagSet |= sBioseqAlignment::fAlignKeepAllBestMatches;
            if( keepAllMatches == 4 ) flagSet |= sBioseqAlignment::fAlignKeepRandomBestMatch;
            if( keepAllMatches == 5 ) flagSet |= sBioseqAlignment::fAlignKeepUniqueMatch;
            if( reverseEngine )flagSet |= sBioseqAlignment::fAlignReverseEngine;

            idx searchRepeatsAndTrans = formIValue("searchRepeatsAndTrans", 0);
            if( searchRepeatsAndTrans == 1 ) flagSet |= sBioseqAlignment::fAlignSearchRepeats;
            if( searchRepeatsAndTrans == 2 ) flagSet |= sBioseqAlignment::fAlignSearchTranspositions | sBioseqAlignment::fAlignSearchRepeats;
            return flagSet;
        }
};

idx DnaHexagonCollector::OnExecute(idx req)
{
    sVioal::digestParams params;
    params.flags= getTheFlags();
    params.minFragmentLength = formIValue("fragmentLengthMin", 0);
    params.maxFragmentLength = formIValue("fragmentLengthMax", 0);
    params.seed = rand_seed;


    sStr errS;
    const char * subject = formValue("subject");
    sHiveseq Sub(user, subject, sBioseq::eBioModeShort, false, false, &errS);
    if( Sub.dim() == 0 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Reference '%s' sequences are missing or corrupted%s%s", subject ? subject : "unspecified", errS ? ": " : "", errS ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    const char * query = formValue("query");
    sHiveseq Qry(sQPride::user, query, sBioseq::eBioModeShort, false, false, &errS);
    if( Qry.dim() == 0 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Query '%s' sequences are missing or corrupted%s%s", query ? query : "unspecified", errS ? ": " : "", errS ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    sStr srcAlignmentsT00;

    grpDataPaths(grpId, "alignment-slice.vioalt", &srcAlignmentsT00, "dna-hexagon");
    //srcAlignmentsT.shrink00();
    //grpDataPaths(grpId, "alignment-slice.vioalt", &srcAlignmentsT00, "dna-alignx");
    logOut(eQPLogType_Debug, "Alignment slice list count %"DEC, sString::cnt00(srcAlignmentsT00));

    const char * resultFileTemplate = formValue("resultFileTemplate", 0);
    if( !resultFileTemplate ) {
        resultFileTemplate = "";
    }
    sStr pathBuf;
    reqAddFile(pathBuf, "%salignment.hiveal", resultFileTemplate);
    pathBuf.add0();
    const char * coverT = reqAddFile(pathBuf, "%s", "coverage_dict");
    const char * dstAlignmentsT = pathBuf.ptr();
    if( !dstAlignmentsT || !coverT ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Failed to add %s", dstAlignmentsT ? "coverage" : "combined alignments");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sVioal vioAltAAA(0, &Sub, &Qry);
    vioAltAAA.myCallbackFunction = sQPrideProc::reqProgressStatic;
    vioAltAAA.myCallbackParam = this;
    sDic<sBioal::LenHistogram> lenHistogram;
    sVec<idx> subCoverage(coverT);

    logOut(eQPLogType_Debug, "Concatenating results '%s', '%s'", dstAlignmentsT, coverT);

    params.countHiveAlPieces = 4000000;
    params.combineFiles = false;
    vioAltAAA.DigestCombineAlignmentsRaw(dstAlignmentsT, srcAlignmentsT00, params, &lenHistogram, &subCoverage);

    if( lenHistogram.dim() ) {
        const char * histPath = reqAddFile(pathBuf, "%shistogram.csv", resultFileTemplate);
        if( histPath ) {
            logOut(eQPLogType_Debug, "Preparing histogram '%s'", histPath);
            if( !sFile::exists(histPath) || sFile::remove(histPath) ) {
                sFil hist(histPath);
                if( hist.ok() ) {
                    sBioal::printAlignmentHistogram(&hist, &lenHistogram);
                    histPath = 0; // success!
                }
            }
        }
        if( histPath ) {
            reqSetInfo(req, eQPInfoLevel_Error, "Failed to save histogram");
            logOut(eQPLogType_Error, "Cannot operate on '%s'", histPath);
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }
    }
    /*
     sHivealtax * idAlTax = new sHivealtax(user, objs[0].Id(), (idx)1, (idx)0);
     sDic < idx > taxCnt;
     char buf[1024];sprintf(buf,"%"DEC,objs[0],objs[0].Id());
     idAlTax->CurateResult(taxCnt, 0, subject, buf, "leaf");
     idx newSpeciesFound = idAlTax->analyzeResults(&taxCnt,0, query, "alignment");
     */

#ifndef _DEBUG
    sStr filenames00;
    sString::searchAndReplaceSymbols(&filenames00,srcAlignmentsT00,0,","sString_symbolsBlank,(const char *)0,0,true,true,true,true);
    for( const char * flnm=filenames00; flnm; flnm=sString::next00(flnm) ) {
        if(sFile::exists(flnm) && !sFile::remove(flnm)) {
            logOut(eQPLogType_Warning,"Failed to delete alignment file : \"%s\"\n",flnm);
        }
    }
#endif

    if( reqProgress(-1, 100, 100) ) { //Do not change status if request was stopped.
        reqSetProgress(req, -1, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    }
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future

    DnaHexagonCollector backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "dna-hexagon-collector", argv[0]));
    return (int) backend.run(argc, argv);
}

