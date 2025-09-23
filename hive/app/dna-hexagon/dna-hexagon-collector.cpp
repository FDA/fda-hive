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

#include <qpsvc/qpsvc-dna-hexagon.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>
#include <violin/alignparse.hpp>

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
            idx reverseEngine = formIValue("reverseEngine", 0);
            idx keepAllMatches = formIValue("keepAllMatches", 1);
            idx flagSet = sBioseqAlignment::fAlignForward;
            if( formIValue("isglobal") ) flagSet |= sBioseqAlignment::fAlignGlobal;
            if( formIValue("isbackward", 1) ) {
                flagSet|=sBioseqAlignment::fAlignBackward|sBioseqAlignment::fAlignBackwardComplement;
                if( formIValue("isbackward", 1) == 2 ) flagSet &= ~sBioseqAlignment::fAlignForward;
            }
            if( formIValue("isoptimize", 1) ) flagSet |= sBioseqAlignment::fAlignOptimizeDiagonal;
            if( formIValue("isextendtails") ) flagSet |= sBioseqAlignment::fAlignMaxExtendTail;
            if( formIValue("isCircular") ) flagSet |= sBioseqAlignment::fAlignCircular;
            if( formIValue("doubleHash") ) flagSet |= sBioseqAlignment::fAlignOptimizeDoubleHash;
            if( formIValue("alignmentEngine") == 0 ) flagSet |= sBioseqAlignment::fAlignIdentityOnly;

            idx resolveConflicts = formIValue("resolveConflicts", false);
            if( resolveConflicts ) {
                if(resolveConflicts == 1) flagSet |= sBioseqAlignment::fAlignKeepResolveMarkovnikov;
                if(resolveConflicts == 2) flagSet |= sBioseqAlignment::fAlignKeepResolveBalanced;

                idx resolveConflictsScore = formIValue("resolveConflictsScore", false);
                if( resolveConflictsScore == 1 ) flagSet |= sBioseqAlignment::fAlignKeepResolvedHits;
                if( resolveConflictsScore == 2 ) flagSet |= sBioseqAlignment::fAlignKeepResolvedSymmetry;
                if( formBoolValue("resolveConfictsUnique", false) ) flagSet |= sBioseqAlignment::fAlignKeepResolveUnique;
            }

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

    if( params.flags&(sBioseqAlignment::fAlignKeepResolveBalanced|sBioseqAlignment::fAlignKeepResolveMarkovnikov) ) {
        if( params.flags&(sBioseqAlignment::fAlignKeepRandomBestMatch|sBioseqAlignment::fAlignKeepUniqueMatch|sBioseqAlignment::fAlignKeepBestFirstMatch|sBioseqAlignment::fAlignKeepFirstMatch) ) {
            params.flags&=~(sBioseqAlignment::resolveConflicts);
            logOut(eQPLogType_Warning,"Please to choose keep more than one read in order to resolve conflicts");
            reqSetInfo(req, eQPInfoLevel_Warning,"Please choose to keep more than one read in order to resolve conflicts");
        }
    }


    sStr errS,sSubj;
    const char * subject = QPSvcDnaHexagon::getSubject00(objs[0],sSubj);
    sHiveseq Sub(user, subject, sBioseq::eBioModeShort, false, false, &errS);
    if( Sub.dim() == 0 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "dna-heptagon-collect: Reference '%s' sequences are missing or corrupted%s%s", subject ? subject : "unspecified", errS ? ": " : "", errS ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    sStr sQry;
    const char * query = QPSvcDnaHexagon::getQuery00(objs[0],sQry);
    sHiveseq Qry(sQPride::user, query, sBioseq::eBioModeShort, false, false, &errS);
    if( Qry.dim() == 0 ) {
        reqSetInfo(req, eQPInfoLevel_Error, "dna-heptagon-collect: Query '%s' sequences are missing or corrupted%s%s", query ? query : "unspecified", errS ? ": " : "", errS ? errS.ptr() : "");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    logOut(eQPLogType_Debug, "Concatenating results");
    const char * resultFileTemplate = formValue("resultFileTemplate", 0);
    if( !resultFileTemplate ) {
        resultFileTemplate = "";
    }
    params.countHiveAlPieces = 4000000;
    params.combineFiles = false;

    sStr hexagonIdBuf;
    if (reqGetData(reqId, "hexagonMasterId", &hexagonIdBuf)) {
        if (hexagonIdBuf.length()) {
            masterId = strtoll(hexagonIdBuf.ptr(0), 0, 10);
        }
    }

    AlignMapParser alignParser(*this, Sub, Qry);
    if (sRC rc = alignParser.joinAls(params, 0, resultFileTemplate)) {
        reqSetInfo(req, eQPInfoLevel_Error, "Error when combining alignments: %s", rc.print());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }


    if( reqProgress(-1, 100, 100) ) {
        reqSetProgress(req, -1, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    }
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    DnaHexagonCollector backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-hexagon-collector", argv[0]));
    return (int) backend.run(argc, argv);
}

