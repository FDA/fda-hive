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
#include <qlib/QPrideProc.hpp>
#include <violin/alignparse.hpp>
#include <errno.h>

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

        bool checkSamUnaligned(sStr & inputSam, sStr & errmsg)
        {
            bool isSamUnaligned = true;

            const idx UNALIGNED_SAM_FLAG = 0x04;

            const char * cur = inputSam.ptr(0);
            const char * end = inputSam.ptr(0) + inputSam.length();

            while ( cur < end && cur != 0 ) {
                if ( *cur != '@' ) {
                    cur = sString::skipWords(cur, end - cur, 1, "\t");
                    if ( cur != 0 ) {
                        char * strtolEnd;
                        errno = 0;
                        idx flag = strtol(cur, &strtolEnd, 10);
                        if ( errno == 0  && strtolEnd != cur ) {
                            if ( !( flag & UNALIGNED_SAM_FLAG ) ) {
                                isSamUnaligned = false;
                                break;
                            }
                        } else {
                            errmsg.printf("Unable to interpret SAM flag.");
                            isSamUnaligned = false;
                            break;
                        }
                    } else {
                        errmsg.printf("SAM alignment row does not contain required number of tabs.");
                        isSamUnaligned = false;
                        break;
                    }
                }
                cur = sString::skipWords(cur, end - cur, 1, "\n");
            }

            return isSamUnaligned;
        }

        virtual idx OnExecute(idx req)
        {
            const sHiveId objID(formValue("obj"));
            std::unique_ptr<sUsrObj> obj(user->objFactory(objID));

            FileAlParser * parser = NULL;

            const idx reqtoWaitFor = formIValue("parser_reqid");
            if( reqtoWaitFor ) {
                const idx statreqStatus = reqGetStatus(reqtoWaitFor);
                if( statreqStatus == eQPReqStatus_Suspended ) {
                    reqSetStatus(req, statreqStatus);
                    return 0;
                }
                if( statreqStatus < eQPReqStatus_Suspended ) {
                    logOut(eQPLogType_Debug, "Waiting for request: %" DEC, reqtoWaitFor);
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

            const sFilePath ext(sourceSequenceFilePath, "%%ext");
            idx cntFound = 0;
            sFil fl(sourceSequenceFilePath, sMex::fReadonly);
            sourceSequenceFilePath.add0();
            sDic<idx> subIds;
            sDic<idx> * subIdsP = NULL;
            if( !fl.ok() ) {
                errmsg.printf(0, "error reading source file");
                cntFound = -5;
            } else if( strcmp(ext, "ma") == 0 ) {
                parser = new MultipleAlParser(*this, Qry, Qry);
            } else {
                const char * subform = formValue("sub", &localBuf);
                sHiveId::parseRangeSet(sids, subform);
                subform = sHiveId::printVec(localBuf, sids, ";", false);
                Sub.parse(subform, sBioseq::eBioModeLong, false, user);
                if( !sids.dim() || errmsg || !Sub.dim() ) {
                    bool allUnaligned = checkSamUnaligned(fl, errmsg);
                    if ( allUnaligned ) {
                        errmsg.printf("it only contains unaligned sequences");
                        cntFound = 0;
                    } else {
                        if ( errmsg.length() == 0 ) {
                            errmsg.printf("missing or empty reference with aligned sequences");
                        } else {
                            errmsg.printf(" malformed SAM input");
                        }
                        cntFound = -6;
                    }
                } else {
                    if( strcmp(ext, "sam") == 0 ) {
                        parser = new SAMParser(*this, Sub, Qry, true);
                        sFilterseq::parseDicBioseq(subIds, Sub, 0);
                        subIdsP = &subIds;
                    } else {
                        errmsg.printf("file format not recognized '.%s'", ext.ptr());
                        cntFound = -4;
                    }
                }
            }

            if (parser != NULL) {
                FileAlParser::WriteParams writeParams;
                writeParams.minMatchLength = 0;
                writeParams.maxMissQueryPercent = 100;
                writeParams.isMinMatchLengthPercentage = 0;
                writeParams.scoreFilter = 0;
                writeParams.flagSet = sBioseqAlignment::fAlignForward;
                if (sRC rc = parser->writeAls(sourceSequenceFilePath.ptr(), writeParams, cntFound, errmsg, 0, 0, subIdsP)) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Error encountered when writing vioals: %s", rc.print());
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    delete parser;
                    return 0;
                }
            }

            logOut(eQPLogType_Debug, "Parsed %" DEC " alignments", cntFound < 0 ? 0 : cntFound);

            if( cntFound == -3) {
                reqSetInfo(req, eQPInfoLevel_Info, "%s%s", filename.ptr(), errmsg.ptr());
            } else if( cntFound == 0) {
                delObject(obj.get(), reqtoWaitFor);
                progress100Start = 0;
                progress100End = 100;
                reqProgress(0, 100, 100);
                reqSetInfo(req, eQPInfoLevel_Warning, "%s not alignments found %s", filename.ptr(), errmsg.length() ? errmsg.ptr(): "");
                reqSetStatus(req, eQPReqStatus_Done);
                delete parser;
                return 0;
            } else if( cntFound < 0 ) {
                delObject(obj.get(), reqtoWaitFor);
                reqSetInfo(req, eQPInfoLevel_Error, "%s %s", filename.ptr(), errmsg.ptr());
                reqSetStatus(req, eQPReqStatus_ProgError);
                delete parser;
                return 0;
            }

            if( !reqProgress(cntFound, 100, 100) ) {
                delete parser;
                return 0;
            }
            progress100Start = 50;
            progress100End = 100;

            if (parser != NULL) {
                sVioal::digestParams params;
                params.flags = sBioseqAlignment::fAlignForward;
                params.countHiveAlPieces = 1000000;
                params.combineFiles = false;
                params.minFragmentLength = formIValue("fragmentLengthMin", 0);
                params.maxFragmentLength = formIValue("fragmentLengthMax", 0);

                if (sRC rc = parser->joinAls(params, obj.get())) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Error encountered when joining vioals into hiveal: %s", rc.print());
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    delete parser;
                    return 0;
                }
            }

            if (strcmp(ext, "ma") == 0) {
                obj->propSetHiveIds("subject", qids);
            } else {
                obj->propSetHiveIds("query", qids);
                if( sids.dim() ) {
                    obj->propSetHiveIds("subject", sids);
                }
            }
            progress100Start = 0;
            progress100End = 100;
            if( reqProgress(cntFound, 100, 100) ) {
                reqSetStatus(req, eQPReqStatus_Done);
            }
            delete parser;
            return 0;
        }
};

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    DnaAlignParser backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-align-parser", argv[0]));
    return (int) backend.run(argc, argv);
}
