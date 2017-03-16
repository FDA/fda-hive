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
#include <violin/violin.hpp>

class DnaAlignParser: public sQPrideProc
{
    public:
        DnaAlignParser(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        void delObject(sUsrObj * obj, const idx reqtoWaitFor)
        {
            if( reqtoWaitFor ) {
#if _DEBUG
                const bool deleted = true;
#else
                const bool deleted = obj ? obj->actDelete() : false;
#endif
                if( deleted ) {
                    logOut(eQPLogType_Debug, "Object %s has been deleted", obj->Id().print());
                } else {
                    reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to delete object %s", obj ? obj->Id().print() : "unspecified");
                }
            }
        }

        virtual idx OnExecute(idx req)
        {
            const sHiveId objID(formValue("obj"));
            std::auto_ptr<sUsrObj> obj(user->objFactory(objID));

            const idx reqtoWaitFor = formIValue("parser_reqid");
            if( reqtoWaitFor ) {
                // GET STATUS OF A RUNNING PARSER
                // there is more than one request possible if more than on reads
                // has been listed or chunks of a single read
                const idx statreqStatus = reqGetStatus(reqtoWaitFor);
                if( statreqStatus == eQPReqStatus_Suspended ) {
                    reqSetStatus(req, statreqStatus);
                    return 0;
                }
                if( statreqStatus < eQPReqStatus_Suspended ) {
                    logOut(eQPLogType_Debug, "Waiting for request: %"DEC, reqtoWaitFor);
                    reqReSubmit(req, 60);
                    return 0;
                }
                if( statreqStatus > eQPReqStatus_Done ) {
                    delObject(obj.get(), reqtoWaitFor);
                    reqSetInfo(req, eQPInfoLevel_Error, "Reads file Indexing failed");
                    reqSetStatus(req, statreqStatus);
                    return 0;
                }
                if( statreqStatus == eQPReqStatus_Done ) {
                    logOut(eQPLogType_Debug, "processing object %s", objID.print());
                }
            }
            if( !obj.get() ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", objID.print());
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 1;
            }

            progress100Start = 0;
            progress100End = 50;

            sStr sourceSequenceFilePath;
            formValue("sourceSequenceFilePath", &sourceSequenceFilePath);
            if( !sFile::size(sourceSequenceFilePath) ) {
                reqSetInfo(req, eQPLogType_Error, "Source file not found or empty");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }

            sStr localBuf, errmsg, filename("File ");
            formValue("userFileName", &filename);
            if( filename.length() <= 5 ) {
                filename.cut0cut();
            } else {
                filename.shrink00();
                filename.printf(": ");
            }
            sVec<sHiveId> qids, sids;
            const char * qryform = formValue("qry", &localBuf);
            sHiveId::parseRangeSet(qids, qryform);
            qryform = sHiveId::printVec(localBuf, qids, ";", false);
            sHiveseq Sub, Qry(sQPride::user, qryform, sBioseq::eBioModeLong, false, false, &errmsg);
            if( !qids.dim() || errmsg || !Qry.dim() ) {
                delObject(obj.get(), reqtoWaitFor);
                reqSetInfo(req, eQPInfoLevel_Error, "%sreads %s: %s", filename.ptr(), qids.dim() ? qryform : "unspecified", qids.dim() ? errmsg.ptr() : "is empty");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }

            sVec<idx> alignmentMap;
            //idx cntFound = parseAligner(sourceSequenceFilePath, Sub, Qry, &alignmentMap, errmsg);
            const sFilePath ext(sourceSequenceFilePath, "%%ext");
            idx cntFound = 0;
            sFil fl(sourceSequenceFilePath, sMex::fReadonly);
            if( !fl.ok() ) {
                errmsg.printf(0, "error reading source file");
                cntFound = -5;
            } else if( strcmp(ext, "ma") == 0 ) {
                cntFound = sBioseqAlignment::readMultipleAlignment(&alignmentMap, fl.ptr(), fl.length(), sBioseqAlignment::eAlRelativeToMultiple, 0, false);
            } else {
                const char * subform = formValue("sub", &localBuf);
                sHiveId::parseRangeSet(sids, subform);
                subform = sHiveId::printVec(localBuf, sids, ";", false);
                Sub.parse(subform, sBioseq::eBioModeLong, false, user);
                if( !sids.dim() || errmsg || !Sub.dim() ) {
                    errmsg.printf(0, "missing or empty reference");
                    cntFound = -6;
                } else {
                    sDic<idx> subIds;
                    // > 1 million reference sequences, mmap to disk to avoid heap memory abuse
                    if( Sub.dim() > 1000000 ) {
                        sStr tempSubDir;
                        cfgStr(&tempSubDir, 0, "qm.tempDirectory");
                        tempSubDir.printf("%"DEC"-%s", reqId, "subids.dic");
                        subIds.init(tempSubDir);
                    }
                    // Create dictionary for Sub Id's
                    sFilterseq::parseDicBioseq(subIds, Sub, 0);
                    const idx minMatchLength = 0, maxMissQueryPercent = 100, scoreFilter = 0;
                    if( strcmp(ext, "sam") == 0 ) {
                        cntFound = sVioseq2::convertSAMintoAlignmentMap(fl.ptr(), fl.length(), &alignmentMap, 0, minMatchLength, maxMissQueryPercent, &subIds, 0, true);
                        if( cntFound < 0 ) {
                            errmsg.printf(0, "One or more reference ids were not resolved using reference list provided");
                        } else if( cntFound != Qry.dim() ) { // should they??
                            errmsg.printf("Number of reads %"DEC" do not correlate with number of alignments %"DEC, Qry.dim(), cntFound);
                            cntFound = -3;
                        }
                    } else if( strcmp(ext, "blast_out") == 0 ) {
                        // Untested code down here
                        sIO log((idx) 0, (sIO::callbackFun) ::printf);
                        cntFound = sBioAlBlast::SSSParseAlignment(&log, fl.ptr(), fl.length(), &alignmentMap, scoreFilter, minMatchLength, maxMissQueryPercent, &subIds);
                    } else {
                        errmsg.printf("file format not recognized '.%s'", ext.ptr());
                        cntFound = -4;
                    }
                }
            }
            logOut(eQPLogType_Debug, "Parsed %"DEC" alignments", cntFound);
            if( cntFound == -3 ) {
                // No error, just report the message
                reqSetInfo(req, eQPInfoLevel_Info, "%s%s", filename.ptr(), errmsg.ptr());
            } else if( cntFound == 0 ) {
                delObject(obj.get(), reqtoWaitFor);
                reqSetInfo(req, eQPInfoLevel_Warning, "%s not alignments found", filename.ptr());
                reqSetStatus(req, eQPReqStatus_Done);
                return 0;
            } else if( cntFound < 0 ) {
                delObject(obj.get(), reqtoWaitFor);
                reqSetInfo(req, eQPInfoLevel_Error, "%s %s", filename.ptr(), errmsg.ptr());
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }

            idx flagSet = sBioseqAlignment::fAlignForward;
            if( alignmentMap.dim() ) {
                sStr pathT;
                reqSetData(req, "file://alignment-slice.vioalt", 0, 0); // create and empty file for this request
                reqDataPath(req, "alignment-slice.vioalt", &pathT);
                sFile::remove(pathT);
                sFil ff(pathT);
                sBioseqAlignment::filterChosenAlignments(&alignmentMap, 0, flagSet, &ff);
            }
            if( !reqProgress(cntFound, 100, 100) ) {
                return 0;
            }
            progress100Start = 50;
            progress100End = 100;

            sStr srcAlignmentsT, dstAlignmentsT;
            if( !obj->addFilePathname(dstAlignmentsT, true, "alignment.hiveal") ) {
                reqSetInfo(req, eQPLogType_Error, "failed to create destination");
                return 0;
            }

            sVioal vioAltAAA(0, Sub.dim() ? &Sub : &Qry, &Qry);
            vioAltAAA.myCallbackFunction = sQPrideProc::reqProgressStatic;
            vioAltAAA.myCallbackParam = this;
            sVioal::digestParams params;
            params.flags = flagSet;
            params.countHiveAlPieces = 1000000;
            params.combineFiles = false;
            params.minFragmentLength = formIValue("fragmentLengthMin", 0);
            params.maxFragmentLength = formIValue("fragmentLengthMax", 0);
            grpDataPaths(req, "alignment-slice.vioalt", &srcAlignmentsT, vars.value("serviceName"));
            vioAltAAA.DigestCombineAlignmentsRaw(dstAlignmentsT, srcAlignmentsT, params);

            obj->propSetHiveIds("query", qids);
            if( sids.dim() ) {
                obj->propSetHiveIds("subject", sids);
            }
            progress100Start = 0;
            progress100End = 100;
            if( reqProgress(req, cntFound, 100) ) {
                reqSetStatus(req, eQPReqStatus_Done); // change the status
            }
            return 0;
        }
};

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future

    DnaAlignParser backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "dna-align-parser", argv[0]));
    return (int) backend.run(argc, argv);
}
