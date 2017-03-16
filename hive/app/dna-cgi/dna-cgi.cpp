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
#include <qlib/QPrideCGI.hpp>
#include <qlib/QPrideProc.hpp>
#include <violin/violin.hpp>
#include <qpsvc/archiver.hpp>
#include <qpsvc/dna-qc.hpp>
#include <qpsvc/screening.hpp>
#include <ulib/uquery.hpp>
#include <ulib/utype2.hpp>
#include <regex.h>
#include <errno.h>

class DnaCGI: public sQPrideCGI, sVioTools
{
    public:

        DnaCGI(const char * defline00, const char * service, idx argc, const char * * argv, const char * * envp, FILE * readfrom, bool isCookie, bool immediate)
            : sQPrideCGI(defline00, service, argc, argv, envp, readfrom, isCookie, immediate)
        {
        }

        virtual idx Cmd(const char * cmd);
        idx CmdAlignment(idx cmdnum);
        idx CmdProfile(idx cmdnum);
        idx CmdRecomb(idx cmdnum);
        idx CmdPopulation(idx cmdnum);
        idx CmdAnnotation(idx cmdnum);
        idx CmdIon(idx cmdnum);
        idx CmdIonBio(idx cmdnum);

        idx customizeSubmission(sVar * pForm, sUsr * user, sUsrProc * obj, Service * pSvc, sStr * log, sMex **pFileSlices)
        {
            return sHiveTools::customizeSubmission(pForm, user, obj, pSvc, log, pFileSlices);
        }

        virtual bool checkETag(sStr & etagBuf, idx len, idx timeStamp)
        {
            return runAsBackEnd() ? false : sQPrideCGI::checkETag(etagBuf, len, timeStamp);
        }

        virtual void voutBinUncached(const void * buf, idx len, const char * etag, bool asAttachment, const char * flnmFormat, va_list marker)
        {
            if( runAsBackEnd() ) {
                if( len ) {
                    outBinData(buf, len);
                }
            } else {
                sQPrideCGI::voutBinUncached(buf, len, etag, asAttachment, flnmFormat, marker);
            }
        }

        bool loadScreeningFile(sTxtTbl *tbl, sHiveId &screenId, const char *screenType, const char *screenResult, sStr *err, sStr *objname, bool tabSeparated = false)
        {
            if( !screenId.objId() && !screenResult ) {
                err->printf("Please specify correct screen ID");
                return false;
            }
            std::auto_ptr<sUsrObj> obj(user->objFactory(screenId));
            if( !obj.get() || !obj->Id() ) {
                err->printf("File object %s not found or access denied", screenId.print());
                return false;
            }
            obj->propGet("name", objname);

            sStr screenPath;
            if( screenType ) {
                obj->getFilePathname00(screenPath, screenType);
            } else if( sUsrFile * ufile = dynamic_cast<sUsrFile*>(obj.get()) ) {
                ufile->getFile(screenPath);
            }

            if( !screenPath || !sFile::exists(screenPath) ) {
                err->printf("Could not access original source file for object %s", screenId.print());
                return false;
            }

            if( screenPath.length() > 4 && sIsExactly(screenPath.ptr(screenPath.length() - 4), ".tab") ) {
                tabSeparated = true;
            }

            if( sFile::size(screenPath) ) {
                tbl->setFile(screenPath);
            } else if( screenResult && *screenResult ) {
                tbl->setBuf(screenResult);
            }
            if(tabSeparated){
                tbl->parseOptions().colsep = "\t";
            }
            if( !tbl->parse() || !tbl->rows() ) {
                return false;
            }

            return true;

        }

        // overrides sUsrCGI implementation
        qlang::sUsrEngine * queryEngineFactory(idx flags/* = 0 */)
        {
            return new qlang::sHiveEngine(m_User, flags);
        }
};

class DnaCGIProc: public sQPrideProc
{
        typedef sQPrideProc TParent;
    public:

        DnaCGIProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
            dnaCGI_qapp = 0;
        }
        virtual idx OnExecute(idx);
        virtual ~DnaCGIProc()
        {
            delete dnaCGI_qapp;
        }
        void initIfNeeded();
        virtual bool OnInit(void)
        {
            initIfNeeded();
            return TParent::OnInit();
        }
//        std::auto_ptr<DnaCGI> dnaCGI_qapp;
        DnaCGI * dnaCGI_qapp;

//        idx functionToName(sIonWander &iWander, sTxtTbl &tbl, const char *ionQuery, const char *defaultValue = 0);
};

enum enumCommands
{
    eSeqList,
    eSeqQC,
    eLaunchSvc,
    eArchive,
    eIdMap,

    eAlCount,eAlMatch,eAlView,eAlSam,eAlBed,eAlStack,eAlStackSubj,eAlConsensus,eAlMutBias,eAlSaturation,eAlIon,eAlFastq,eAlFasta,eAlFailed,
    eProfSNP,eProfNoiseIntegral,eProfFreqIntegral,eProfWith0,eProfVCF,eProfSummary,eProfSummaryAll,eProfContig,eProfSNPcalls,eProfProtGen,eProfGWatch,eProfConsensus,
    eRecombCross,
    eDiSeqSubList,
    ePopHierarchy,ePopSummary,ePopCoverage,ePopPredictedGlobal,ePopConsensus,ePopContig,ePopExtended,ePopPermutations,ePopStackedStats,ePopClones,
    eAnotDefinition, eAnotNumberOfRange, eAnotRange, eAnotIdsByRangeNumber, eAnotIdByKindOfId, eAnotGetAnnotFile,eAnotFiles,eIngestGeneList,eAnotGetIdTypesFromAnotFile, eAnotSearch ,eAnotBrowser, eAnotDumper,  eAnotMapperResults, eAnotSeqIDs,
    eGenCount,
//    eIon,//AnnotInfo,eIonAnnotIdMap,
    eClock,
    eTranlsateValt,

    eLast
};

#include "ion-cgi.cpp"

const char * listCommands = "seqList"_
"seqQC"_
"launchSvc"_
"archive"_
"idMap"_


    "alCount"_"alMatch"_"alView"_"alSam"_"alBed"_"alStack"_"alStackSubj"_"alConsensus"_"alMutBias"_"alSaturation"_"alIon"_"alFastq"_"alFasta"_"alFailed"_
    "profSNP"_"profNoiseIntegral"_"profFreqIntegral"_"profWithZeros"_"profVCF"_"profSummary"_"profSummaryAll"_"profContig"_"profSNPcalls"_"profProtGen"_"profGWatch"_"profConsensus"_
    "recombCross"_
    "diSeqSubList"_
    "popHierarchy"_"popSummary"_"popCoverage"_"popPredictedGlobal"_"popConsesus"_"popContig"_"popExtended"_"popPermutations"_"popStackedStats"_"popClones"_
    "anotDefinition"_"anotNumberOfRange"_"anotRange"_"anotIdsByRangeNumber"_"anotIdByKindOfId"_"anotFile"_"anotFiles"_"ingestGeneList"_"anotGetIdTypesFromAnotFile"_"anotSearch"_"anotBrowser"_"anotDumper"_"anotMapperResults"_"anotSeqIDs"_
    "genCount"_
//    "ion"_
//    "ionAnnotInfo"_"ionAnnotIdMap"_
"clock"_
"translatevalt"_
_;

const char * listIonCommands = "ionncbiTax"_"ionTaxInfo"_"ionTaxDownInfo"_"ionTaxParent"_"ionTaxPathogenInfo"_
"ionAnnotInfo"_"ionAnnotIdMap"_"ionTaxidCollapse"_"ionTaxidByName"_"ionCodonDB"_"extendCodonTable"_"ionWander"_
_;

const char * listIonBioCommands = "ionGenBankAnnotPosMap"_"ionAnnotTypes"_"ionAnnotInfoAll"_""
_;

idx DnaCGI::Cmd(const char * cmd)
{
    if( !cmd ) {
        return 0;
    }

    idx cmdnum = -1;
    sString::compareChoice(cmd, listCommands, &cmdnum, false, 0, true);

    sStr tmp, t;

    const char * svc = pForm->value("svc");
    if( svc ) {
        sQPrideProc::QPrideSrvName(&tmp, svc, sApp::argv[0]);
        char * pcgi = strstr(tmp.ptr(0), ".cgi");
        if( pcgi )
            *pcgi = 0;
        vars.inp("serviceName", tmp.ptr());
        svc = vars.value("serviceName");
        tmp.cut(0);
    }

    switch(cmdnum) {
        case eClock: {
            udx waitTime = pForm->uvalue("msecs", 1000);
            sStr * out = &dataForm;
            sleepMS(waitTime);
            out->printf("waited for %"DEC"msecs successfully", waitTime);
            outHtml();
            return 1;
        }
        case eTranlsateValt: {
            sHiveId objid(pForm->value("objID"));
            idx mySubject = pForm->ivalue("subID", 0);

            std::auto_ptr<sUsrObj> obj(sQPride::user->objFactory(objid));
            if( !obj.get() || !obj->Id() ) {
                break;
            }
            sStr path;
            obj->getFilePathname(path, "SNPprofile-%" DEC ".csv", mySubject);
            sTxtTbl tbl;
            tbl.setFile(path);
            tbl.parse();
            sStr buffer;
            if( tbl.dim() ) {
                tbl.printCell(buffer, 10, 10); // prints cell from data row 10 (row -1 is header), column 10
            }

            return 1;
        }
        case eArchive: {
            raw = 1;
            sVec<sHiveId> ids;
            sHiveId::parseRangeSet(ids, pForm->value("ids"));
            if( !ids.dim() ) {
                error("Missing object id(s)");
                break;
            }
            const char * objtype = pForm->value("objtype");
            const char * ext = pForm->value("ext");

            if( objtype && objtype[0] && ids.dim() != 1 ) {
                error("Type conversion supported only for a single object");
                break;
            }
            sStr dest;
            cfgStr(&dest, 0, "user.download");
            dest.printf("cnv-%"DEC"/", reqId ? reqId : getpid() + rand()); // slash at the end important!
            if( !sDir::makeDir(dest) ) {
                error("Staging area access error: %s", strerror(errno));
                break;
            }
            const idx dpos = dest.length();
            bool hasErrors = false;
            sStr nmBuf, path, extBuf, orig_objtypes00;
            sDic<sHiveId> map;
            for(idx i = 0; i < ids.dim(); ++i) {
                std::auto_ptr<sUsrObj> obj(user->objFactory(ids[i]));
                sUsrFile* infile = obj.get() ? dynamic_cast<sUsrFile*>(obj.get()) : 0;
                if( !infile ) {
                    error("File object %s not found or access denied", ids[i].print());
                    hasErrors = true;
                    continue;
                }
                path.cut(0);
                infile->getFile(path);
                if( !path || !sFile::exists(path) ) {
                    error("Could not access original source file for object %s", ids[i].print());
                    hasErrors = true;
                    continue;
                }
                nmBuf.cut(0);
                // use file name as provided by user: right extension might get set, etc
                const char * fnm = infile->propGet("name", &nmBuf);
                if( !fnm || !fnm[0] ) {
                    // fall back to original upload name
                    fnm = infile->propGet("orig_name", &nmBuf);
                    if( !fnm || !fnm[0] ) {
                        fnm = "_";
                    }
                }
                char * x = (char *) strchr(fnm, '.');
                if( x ) {
                    *x = '\0';
                }
                if( !ext ) {
                    extBuf.cut(0);
                    infile->propGet("ext", &extBuf);
                } else {
                    extBuf.printf(0, "%s", ext);
                }
                sStr uniq;
                sDir::uniqueName(uniq, "%s%s%s%s", dest.ptr(), fnm, extBuf ? "." : "", extBuf ? extBuf.ptr() : "");
                if( !sFile::symlink(path, uniq) ) {
                    error("Could not establish destination: %s", strerror(errno));
                    hasErrors = true;
                    continue;
                }
                sHiveId * id = map.set(sFilePath::nextToSlash(uniq)); // indication that file comes from existing object
                if( id ) {
                    *id = ids[i];
                    orig_objtypes00.add(infile->getType()->name());
                } else {
                    error("Insufficient resources");
                    hasErrors = true;
                    continue;
                }
            }
            orig_objtypes00.add0(2);
            if( hasErrors ) {
                dest.cut(dpos);
                sDir::removeDir(dest);
                break;
            }
            sStr src("obj://%s", pForm->value("ids"));
            dmArchiver arch(*this, dest, src, ext ? 0 : pForm->value("datatype"));
            arch.addObjProperty("hierarchy", "%s", pForm->value("hierarchy", ""));
            arch.addObjProperty("category", "%s", pForm->value("category", ""));
            arch.setSubject(pForm->value("subject"));
            idx isExpr = pForm->ivalue("isExpr", 0);
            if( isExpr ) {
                arch.setVar("experiment", pForm->value("experiment"));
                arch.setVar("hasdata", pForm->value("hasdata"));
            }
            sStr smap;
            if( map.dim() ) {
                map.serialOut(smap);
                arch.setVar("existing_map", smap);
            }
            idx areq = 0;
            sHiveId aobjID;
            if( objtype && objtype[0] && !arch.convertObj(ids[0], objtype) ) {
                error("conversion request failed");
            } else if( !(areq = arch.launch(*user, grpId, &aobjID)) ) {
                error("could not submit processing request");
            } else {
                sJSONPrinter json_printer(&dataForm);
                json_printer.startObject();
                const char * orig_objtype = orig_objtypes00.ptr();
                for(idx i = 0; i < ids.dim(); ++i, orig_objtype = sString::next00(orig_objtype)) {
                    json_printer.addKey(ids[i].print());
                    json_printer.startObject();
                    {
                        json_printer.addKey("signal");
                        json_printer.addValue("cast");
                        json_printer.addKey("data");
                        json_printer.startObject();
                        {
                            json_printer.addKey("from");
                            json_printer.addValue(orig_objtype);
                            json_printer.addKey("to");
                            json_printer.addValue(objtype);
                            json_printer.addKey("background");
                            json_printer.startObject();
                            {
                                json_printer.addKey("req");
                                json_printer.addValue(areq);
                                json_printer.addKey("obj");
                                json_printer.addValue(aobjID);
                            }
                            json_printer.endObject();
                        }
                        json_printer.endObject();
                    }
                    json_printer.endObject();
                }
                json_printer.endObject();
                json_printer.finish();
                dataForm.printf("\n");
            }
            break;
        }
        case eSeqList: {
            if( raw < 1 )
                raw = 1;
//            idx hsindex=pForm->ivalue("hsindex",1);
#define     DEFAULT_MAX_TXT_LEN 128
            idx maxTxtLen = pForm->ivalue("maxTxtLen", DEFAULT_MAX_TXT_LEN);
            sUsrQueryEngine engine(*user);

            sVec<sHiveId> ids;
            if( const char * sformids = pForm->value("ids") ) {
                if( strstr(sformids, "qry(") == sformids ) {
                    sformids += 4;
                    sVariant result;
                    if( engine.parse(sformids, sLen(sformids) - 1) && engine.eval(result) ) {
                        if( result.isScalar() ) {
                            result.asHiveId(ids.add(1));
                        } else if( result.isList() ) {
                            for(idx i = 0; i < result.dim(); i++) {
                                result.getListElt(i)->asHiveId(ids.add(1));
                            }
                        }
                    }
                } else {
                    sHiveId::parseRangeSet(ids, sformids);
                }
            }
            if( !ids.dim() ) {
                error("object ids must be specified");
                break;
            }

            idx iStart = pForm->ivalue("start");
            idx iCnt = pForm->ivalue("cnt", 20);
            if (iCnt < 0){
                iCnt = sIdxMax;
            }
            idx rStart = pForm->ivalue("rangeStart", 0);
            idx rLen = pForm->ivalue("rangeLen", 0);
            const sBioseq::EBioMode biomode = pForm->boolvalue("long", true) ? sBioseq::eBioModeLong : sBioseq::eBioModeShort;

            idx formatToPrint=0;
            const char *formats = "csv"_"fasta"_"fastq"_"fastx"__;
            sString::compareChoice(pForm->value("fformat", "csv"), formats, &formatToPrint, false, 0, true);

            bool isHeader = (formatToPrint == 0) && pForm->boolvalue("header", true);
            idx iVisible = 0;
            sStr * out = &dataForm;
            regex_t re;
            const char * srch = pForm->value("search", 0);
            if( srch && *srch == 0 )
                srch = 0;
            idx regerr = srch ? regcomp(&re, srch, REG_EXTENDED | REG_ICASE) : true;

            sVec<idx> rowList;
            if( const char * srowlist = pForm->value("rows") ) {
                if( strstr(srowlist, "qry(") == srowlist ) {
                    srowlist += 4;
                    sVariant result;
                    if( engine.parse(srowlist, sLen(srowlist) - 1) && engine.eval(result) ) {
                        if( result.isScalar() ) {
                            rowList.vadd(1, result.asInt());
                        } else if( result.isList() ) {
                            for(idx i = 0; i < result.dim(); i++) {
                                rowList.vadd(1, result.getListElt(i)->asInt());
                            }
                        }
                    }
                } else {
                    sString::scanRangeSet(srowlist, 0, &rowList, 0, 0, 0);
                }
            }

            // ensure sids is separated by ';' not by ',' - it matters for sHiveseq
            sStr sids;
            sHiveId::printVec(sids, ids, ";");
            sHiveseq hs(user, sids, biomode);
            //if(hsindex)hs.reindex();
            sBioseq * bioseq = &hs;

            // Determine if we are going to print in fasta or fastq
            bool isFastq = false;
            switch(formatToPrint){
                case 1:
                    isFastq = false;
                    break;
                case 2:
                    isFastq = true;
                    break;
                case 3:
                    isFastq = (bioseq->isFastA() == true) ? false : true;
                    break;
            }
            if( isHeader ) {
                out->printf("#,len,rpt,id,seq\n");
            }
            if( formatToPrint != 0 ) {
                sStr outfilename;
                // Construct the filename
                if( ids.dim() == 1 ) {
                    std::auto_ptr<sUsrObj> obj(sQPride::user->objFactory(ids[0]));
                    if( !obj.get() ) {
                        break;
                    }
                    sFilePath nm;
                    sStr fnm;
                    if( obj->propGet("name", &fnm) ) {
                        // use fnm in the name
                        nm.makeName(fnm, "%%flnm.%s", isFastq ? "fastq" : "fasta");
                    }
                    outfilename.addString(nm.ptr());
                }
                if( !outfilename ) {
                    const char * outflnm = pForm->value("outflnm",0);
                    outfilename.printf("%s.%s", outflnm?outflnm:sids.ptr(), isFastq ? "fastq" : "fasta");
                }
                // Print the header content
                headerSetContentDispositionAttachment("%s", outfilename.ptr());
                sStr headers;
                outHeaders(&headers);
                outBinData(headers.ptr(), headers.length());
            }
            if( rowList.dim() ) {
                iStart = 0;
            }
            if( rStart < 0 ) {
                rStart = 0;
            }
            if( rLen < 0 ) {
                rLen = 0;
            }
            idx iSeqEnd = rowList.dim() ? rowList.dim() : bioseq->dim();

            static idx maxDumpContainer = 100 * 1024 * 1024 ; //2 << 23;
            for(idx iss = iStart; iss < iSeqEnd; ++iss) {
                idx isValid = 0;
                idx is = iss;
                if( rowList.dim() ) {
                    is = rowList[iss] - 1;
                    if( is < 0 || is >= bioseq->dim() )
                        continue;
                }
                const char * id = bioseq->id(is);
                if( id[0] == '>' )
                    ++id;
                bool match = srch ? false : true;
                if( srch && !regerr ) {
                    match = (regexec(&re, id, 0, NULL, 0) == 0) ? true : false;
                }
                if( !match )
                    continue;

                idx len = bioseq->len(is);
                idx seqlen = len;
                // We must print from bioseq 'is' row
                // in fastaformat
                if( rStart >= seqlen ) {
                    continue;
                }
                if( rLen ) {
                    if( rStart + rLen > seqlen ) {
                        seqlen -= rStart;
                    } else {
                        seqlen = rLen;
                    }
                }
                else {
                    seqlen -= rStart;
                }
                if (formatToPrint != 0){
                    isValid = bioseq->printFastXRow(out,isFastq,is,rStart,seqlen,false,true,false,0,false,0,true,false,false);
                    if (out->length() > maxDumpContainer){
                        outBinData(out->ptr(), out->length());
                        out->cut(0);
                    }
                }
                else {
                    bool ellipsize = false;
                    if( maxTxtLen > 0 && seqlen > maxTxtLen ) {
                        ellipsize = true;
                        seqlen = maxTxtLen;
                    }
                    idx rpt = bioseq->rpt(is);
                    const char * seq = bioseq->seq(is);
                    sStr ss;
                    ss.cut(0);
                    sBioseq::uncompressATGC(&ss, seq, rStart, seqlen, true, 0);
                    // restore N based on quality 0
                    const char * seqqua = bioseq->qua(is);
                    bool quaBit = bioseq->getQuaBit(is);
                    if( seqqua) {
                        idx NCount=0;// count all Ns in one read if it is bigger than filterNs reject the sequence
                        char *seqpos = ss.ptr();
                        for(idx i = rStart, pos = 0; i < rStart + seqlen; ++i, ++pos) {
                            if( sBioseq::Qua(seqqua, i, quaBit) == 0 ) {
                                seqpos[pos] = 'N';
                                NCount++;
                            }
                        }
                    }

                    t.cut(0);
                    sString::escapeForCSV(t, id);
                    out->printf("%"DEC",%"DEC",%"DEC",%s,%s%s\n", (is + 1), len, rpt, t.ptr(), ss.ptr(0), ellipsize ? "..." : "");
                    isValid = 1;
                }
                iVisible += isValid;
                if( iVisible >= iCnt ){
                    break;
                }

            }

            if (formatToPrint != 0){
                outBinData(out->ptr(), out->length()); //, 0, true, outfilename.ptr());
            }
            else {
                if( srch )
                    regfree(&re);
                if( !iVisible ) {
                    out->printf("no available sequences");
                }
                out->add0();
                outHtml();
            }
        }
            return 1;

        case eSeqQC: {
            //idx recompute=pForm->ivalue("recompute",0);
            vars.inp("serviceName", "dna-qc");
            const char * qcNeeded = pForm->value("qc", "sumLetterTable.csv");

            const char* objName = pForm->value("query");
            sHiveId objId(objName && strncmp(objName, "obj://", 6) == 0 ? objName + 6 : objName);
            std::auto_ptr<sUsrObj> obj(sQPride::user->objFactory(objId));
            if( !obj.get() || !obj->Id() ) {
                break;
            }
            sStr path;
            obj->getFilePathname(path, ".qc2.%s", qcNeeded);
            sFil fl(path, sMex::fReadonly);
            if( !fl.ok() ) {
                path.cut(0);
                obj->getFilePathname(path, ".qc.%s", qcNeeded);
                fl.init(path, sMex::fReadonly);
            }
            if( fl.ok() ) {
                outBin(fl.ptr(), fl.length(), sFile::time(path), true, "%s", path.ptr());
            }
            return 1;
        }
        case eLaunchSvc: {
            // Will launch a service
            const sHiveId objID(pForm->value("query"));
            const char *key = pForm->value("key");
            if( !objID ) {
                error("Invalid objID: %"DEC, objID.objId());
                break;
            }
            sStr lockBuf;

            idx keynum = -1;
            const char * listSvcKey = "dna-qc"_"dna-screening"__;

            sString::compareChoice(key, listSvcKey, &keynum, false, 0, true);

            std::auto_ptr<sQPSvc> svc;
            if (keynum == 0){
                svc.reset(new DnaQC(*this, objID));
            }
            else if (keynum == 1){
                svc.reset(new DnaScreening(*this, objID, DnaScreening::eBlastVsNT));
            }
            const char *lockString = svc->getLockString(lockBuf, objID);
            idx reqLock = reqCheckLock(lockString);
            idx priority = 1000;
            if (reqLock){
                dataForm.printf("Waiting for %s request=%"DEC"\n", svc->getSvcName(), reqLock);
            }
            else {
                idx reqsubmitedQC = svc->launch(*user, grpId, 0, priority);
                dataForm.printf("Submitted %s request=%"DEC"\n", svc->getSvcName(), reqsubmitedQC);
            }
            outHtml();
            return 1;
        }
        case eIdMap: {
            // raw=1;
            const char * idlines = pForm->value("ids", 0);
            const char * idTypeTo = pForm->value("idTypeTo", 0);
            const char * idtypeFrom = pForm->value("idTypeFrom", 0);
            const char * mode = pForm->value("mode", "list");

            if( !idlines ) {
                error("ids must be specified");
                break;
            }
            idx iStart = pForm->ivalue("start", 0);
            idx defaultCnt = 1;
            if( strcmp(mode, "list") )
                defaultCnt = sIdxMax;
            idx iCnt = pForm->ivalue("cnt", defaultCnt);

            sStr * out = &dataForm;
            sHiveIdMap im(*user);
            sStr idDigest;
            sString::searchAndReplaceSymbols(&idDigest, idlines, 0, ",", 0, 0, true, true, true, true);
            const char * ptr = idDigest.ptr();
            if( strcmp(mode, "list") )
                out->printf("id,type\n");
            for(idx i = 0; ptr; i++) {
                if( strcmp(mode, "list") ) {
//                    out->printf("id,type\n");
                    sStr buf;
                    im.findId(idlines, idTypeTo, buf, idtypeFrom, iCnt, iStart);
                    sStr line;
                    sString::searchAndReplaceSymbols(&line, buf.ptr(), 0, ",", 0, 0, true, true, false, true);
                    const char * ptr1 = line.ptr();
                    for(; ptr1;) {
                        if( idTypeTo )
                            out->printf("%s,%s\n", ptr1, idTypeTo);
                        else
                            out->printf("%s,unspecified\n", ptr1);
                        ptr1 = sString::next00(ptr1);
                    }
                } else {
                    if( i )
                        out->printf(",");
                    im.findId(idlines, idTypeTo, *out, idtypeFrom, iCnt, iStart);
                }

                ptr = sString::next00(ptr);
            }
            out->add0();
            outHtml();
        }
            return 1;
    }

    if( cmdnum >= eAlCount && cmdnum <= eAlFasta ) {        //idx res=DnaCGI::CmdAlignment(cmdnum);
        return CmdAlignment(cmdnum);
    }

    if( cmdnum >= eProfSNP && cmdnum <= eProfConsensus ) {
        return CmdProfile(cmdnum);
    }

    if( cmdnum >= eRecombCross && cmdnum <= eRecombCross ) {
        return CmdRecomb(cmdnum);
    }

    if( cmdnum == eDiSeqSubList ) {
        if( !objs.dim() ) {
            error("object is missing");
            outHtml();
            return 1;
        }
        sUsrObj& obj = objs[0];
        sVec<sHiveId> parent_proc_ids;
        obj.propGetHiveIds("parent_proc_ids", parent_proc_ids);
        std::auto_ptr<sUsrObj> al(user->objFactory(parent_proc_ids.dim() ? parent_proc_ids[0] : sHiveId::zero));

        sStr cr_path;
        al->getFilePathname00(cr_path, "alignment.hiveal"_"alignment.vioal"__);
        sHiveal hiveal(user, cr_path);
        sBioal * bioal = &hiveal;

        sHiveseq Qry(user, al->propGet00("query",0,";"), hiveal.getQryMode());
        if( !Qry.dim() ) {
            error("alignment query invalid or not accessible");
            outHtml();
            return 1;
        }
        bioal->Qry = &Qry;
        if( !al.get() || !al->Id() ) {
            error("alignment object is missing or not accessible");
            outHtml();
            return 1;
        }

        cr_path.cut0cut();
        obj.getFilePathname00(cr_path, "_.dic"__);
        sFil dicbuf(cr_path,sMex::fReadonly);
        if( !dicbuf.ok() ) return 1;
        sDic<sMex::Pos> dicLU;

        if(!dicbuf.length()) return 1;
        dicLU.serialIn(dicbuf,dicbuf.length());
        const char * di_key = pForm->value("dikey", 0);
        if( !di_key ) return 1;
        sMex::Pos * lupos = dicLU.get(di_key,sLen(di_key));

        if(!lupos)return 1;

        cr_path.cut0cut();
        obj.getFilePathname00(cr_path, "_.vec"__);
        sVec<idx> allreads(sMex::fReadonly,cr_path.ptr());
        if( !allreads.dim() ) {
            error("read container is missing");
            outHtml();
            return 1;
        }

        bool isFastq = pForm->boolvalue("fastq",false);
        sStr defaultFileName;
        defaultFileName.printf("o%s_reads_supporting_di(%s)",obj.IdStr(),di_key);
        sStr outFileName;
        outFileName.printf("%s.%s", pForm->value("outflnm",defaultFileName.ptr()),isFastq?"fastq":"fastq");

        outBinHeaders(true, outFileName);

        sStr out;
        sBioal::ParamsAlignmentIterator par(&out);
        if( isFastq )
            par.navigatorFlags |= sBioal::alPrintQualities;
        bioal->iterateAlignments(0, 0, lupos->size, -2, sBioal::printFastXSingle, &par,0,0,allreads.ptr(lupos->pos));
        outBinData(out.ptr(0), out.length());
    }

    if( cmdnum >= ePopHierarchy && cmdnum <= ePopClones ) {
        return CmdPopulation(cmdnum);
    }

    if( cmdnum >= eAnotDefinition && cmdnum <= eGenCount ) {
        return CmdAnnotation(cmdnum);
    }

    // Check if it is an Ion Command

    if( cmdnum == -1 ) {
        idx ioncmdnum = -1;
        sString::compareChoice(cmd, listIonCommands, &ioncmdnum, false, 0, true);

        if( ioncmdnum != -1 ) {
            return CmdIon(ioncmdnum);
        }
        sString::compareChoice(cmd, listIonBioCommands, &ioncmdnum, false, 0, true);
        if( ioncmdnum != -1 ) {
            return CmdIonBio(ioncmdnum);
        }
        return sQPrideCGI::Cmd(cmd);
    } else {
        outHtml();
        return 1;
    }

}

idx DnaCGI::CmdAlignment(idx cmdnum)
{

    if( !objs.dim() ) {
        error("alignment ids invalid or not accessible");
        outHtml();
        return 1;
    }
    sStr pathAl;
    objs[0].getFilePathname00(pathAl, "alignment.hiveal"_"alignment.vioal"__);
    sHiveal hiveal(user, pathAl);
    sBioal * bioal = &hiveal;

    //sStr pathList;listObjOrReqFiles( &pathList, "alignment.vioal"_"alignment-slice.vioal"__, 0 ,0); // "dna-hexagon"

    idx start = formIValue("start", 0), cnt = formIValue("cnt", sIdxMax), mySubID = formIValue("mySubID", 0);
    --mySubID;
    //idx cntAls=sString::cnt00(pathList.ptr()), iAls=0;
    /*
     const char * path=pathList.ptr();
     idx viochunk=(start>>32)&0xFFFFFFFF;
     if(viochunk)path=sString::next00(path,viochunk);
     idx vioaltMode=0;
     for( ; path; path=sString::next00(path)) {
     sVioal vioalt(path);
     if(!bioal->isok())continue;
     vioaltMode=(bioal->getMode()&1);
     break;
     }
     */

    sStr out, *outP;
    outP = &out;
    sVec<sBioal::Stat> stat;
    sBioal::ParamsAlignmentIterator par(outP);
    sHiveseq Sub(user, formValue("subject"), hiveal.getSubMode());        //Sub.reindex();
    sHiveseq Qry(user, formValue("query"), hiveal.getQryMode());        //Qry.reindex();
    if( !Sub.dim() && Qry.dim() && formBoolValue("multiple") ) {
        Sub.parse(formValue("query"), hiveal.getSubMode());
    }
    if( !Sub.dim() ) {
        error("alignment subject invalid or not accessible");
        outHtml();
        return 1;
    }
    if( !Qry.dim() ) {
        error("alignment query invalid or not accessible");
        outHtml();
        return 1;
    }
    bioal->Sub = &Sub;
    bioal->Qry = &Qry;
    if( runAsBackEnd() ) {
        bioal->progress_CallbackFunction = sQPrideProc::reqProgressStatic;
        bioal->progress_CallbackParam = proc_obj;
    }

    //If downloading has been requested then pass to stdout for faster response of the server
    //avoiding connection time outs
    if( formIValue("down", 0) || runAsBackEnd() )
        par.outF = flOut;
    //Determine flags for Stack and AlView
    par.wrap = pForm->ivalue("wrap", 0);
    par.navigatorFlags = formIValue("flags", (sBioal::alPrintSubject | sBioal::alPrintUpperInterm | sBioal::alPrintQuery));
    par.rangestart = formIValue("rangeStart", 0);
    par.alignmentStickDirectional = 2;
    if( formBoolValue("multiple") )
        par.navigatorFlags |= sBioal::alPrintMultiple;
    par.rangeend = formIValue("rangeEnd", (par.navigatorFlags & sBioal::alPrintMultiple) ? 100 : 0);
    par.winSize = formIValue("alWinSize", 100);
    par.winTailLimit = formIValue("winTailLimit", par.winSize);
    if( par.winTailLimit < 0 )
        par.winTailLimit = sIdxMax;
    par.pageRevDir = formIValue("pageRevDir", 0);
    par.High = formIValue("alHigh", -2) - 1;
    par.rightTailPrint = formBoolIValue("filterTails", 0);
    par.leftTailPrint = formBoolIValue("filterTails", 0);
    if(par.rightTailPrint || par.leftTailPrint) {
        par.navigatorFlags |= sBioal::alPrintFilterTail;
        idx tails_display= formIValue("printTailsOnly", 0);
        if(tails_display < 2 ) {
            par.navigatorFlags |= sBioal::alPrintTailDisplayAlignment;
        }
        if(tails_display > 0){
            par.navigatorFlags |= sBioal::alPrintTailDisplayTail;
        }
        //hide subject and match line
        if( tails_display >= 2 ) {
            par.navigatorFlags &= (~((idx)sBioal::alPrintSubject | (idx)sBioal::alPrintUpperInterm));
        }
    }
    if( formIValue("shuffle", 0) ) {
        par.navigatorFlags |= sBioal::alPrintInRandom;
        idx maxAlLen = 0;
        sStr histflnm;
        objs[0].getFilePathname(histflnm, "histogram.csv");
        sFil histfl(histflnm, sMex::fReadonly);
        if( histfl.ok() && histfl.length() ) {
            sTbl hisTbl;
            hisTbl.parse(histfl, histfl.length());
            maxAlLen = hisTbl.ivalue(hisTbl.rows() - 1, 0);
        } else {
            maxAlLen = Qry.getMaxLen() * 1.25;
        }
        par.maxAlLen = maxAlLen;
    }
    if( formIValue("collapseRpts") )
        par.navigatorFlags |= sBioal::alPrintCollapseRpt;
    if( formIValue("printNs", 1) )
        par.navigatorFlags |= sBioal::alPrintNs;
    if( formIValue("readsAsFasta") ) {
        par.navigatorFlags |= sBioal::alPrintReadsInFasta;
    }
    if( par.High >= 0 || cmdnum == eAlStack || cmdnum == eAlMutBias ) {
        if( !formBoolValue("multiple") ) {
            if( par.High < 0 )
                par.High = par.winSize / 2;
            par.rangestart = par.High - par.winSize / 2;
            par.rangeend = par.High + par.winSize / 2;
            if( par.rangestart < 0 ) {
                par.rangestart = 0;
                par.rangeend = par.winSize;
            }
            if( par.rangeend > Sub.len(mySubID) ) {
                par.rangestart = Sub.len(mySubID) - par.winSize;
                par.rangeend = par.rangestart + par.winSize;
            }
        }
        if( formIValue("alTouch", 1) && !formBoolValue("multiple") )
            par.navigatorFlags |= sBioal::alPrintTouchingOnly;
        if( cmdnum == eAlStack ) {
            par.navigatorFlags &= (~((idx)sBioal::alPrintSubject | (idx)sBioal::alPrintUpperInterm));
            par.navigatorFlags |= (sBioal::alPrintQuery | (formBoolValue("multiple") ? sBioal::alPrintMultiple : sBioal::alPrintDotsFormat) | (formIValue("header", 1) ? sBioal::alPrintMode : 0));
        }
        if( cmdnum == eAlStack || cmdnum == eAlView ) {
            if( formIValue("alVarOnly", 0) )
                par.navigatorFlags |= sBioal::alPrintVariationOnly;
            if( formIValue("alNonPerfOnly", 0) )
                par.navigatorFlags |= sBioal::alPrintNonPerfectOnly;
        }
    }
    if( formIValue("queryAsFasta") ) {
        par.navigatorFlags |= sBioal::alPrintAsFasta;
        par.navigatorFlags |= sBioal::alPrintSequenceOnly;
        par.navigatorFlags &= (~((idx)sBioal::alPrintSubject | (idx)sBioal::alPrintUpperInterm));
    }
    if( formBoolValue("printForward") ) {
        par.navigatorFlags |= sBioal::alPrintForward;
    }
    if( formBoolValue("printReverse") ) {
        par.navigatorFlags |= sBioal::alPrintReverse;
    }

    regex_t regp;
    sStr regStr;
    if( formValue("search") )
        regStr.printf("%s", formValue("search"));
    else if( formValue("prop_val") )
        regStr.printf("%s", formValue("prop_val"));
    if( regStr.ptr() && (cmdnum == eAlView || cmdnum == eAlStack || cmdnum == eAlCount || cmdnum == eAlSaturation) ) {
        if( par.High >= 0 && formIValue("alPosSearch", 0) )
            par.navigatorFlags |= sBioal::alPrintPositionalRegExp;
        if( regStr.ptr()[0] == '-' )
            par.navigatorFlags |= sBioal::alPrintPosInDelRegExp;
        else if( regStr.length() > 1 ) {
            if( regStr.ptr()[1] == '-' && regStr.ptr()[0] == '\\' )
                par.navigatorFlags |= sBioal::alPrintPosInDelRegExp;
        }
        idx regerr = regStr ? regcomp(&regp, regStr.ptr(), REG_EXTENDED | REG_ICASE) : true;
        if( regerr ) {
            error("bad regular expression value for id!");
            return 1;
        }
        par.navigatorFlags |= sBioal::alPrintRegExpQry;
        par.alCol = formIValue("alignmentColumn", 4);
        if( cmdnum == eAlView ) {
            if( formIValue("rgSub", 0) )
                par.navigatorFlags |= sBioal::alPrintRegExpSub;
            else
                par.navigatorFlags &= (~((idx)sBioal::alPrintRegExpSub));
            if( formIValue("rgInt", 0) )
                par.navigatorFlags |= sBioal::alPrintRegExpInt;
            else
                par.navigatorFlags &= (~(sBioal::alPrintRegExpInt));
            if( formIValue("rgQry", 0) )
                par.navigatorFlags |= sBioal::alPrintRegExpQry;
            else
                par.navigatorFlags &= (~(sBioal::alPrintRegExpQry));
        }
        par.regp = &regp;
    }

    idx reportZeroHits = formIValue("zeroHits"), reportFailed = formIValue("zeroHits", 1), reportTotals = formIValue("reportTotals", 1);

    // retrieve sequences and alignments

    // determine filenames
    const char * extension = "csv";
    //if(cmdnum==eAlView )extension="txt";
    if( cmdnum == eAlFasta || (par.navigatorFlags & sBioal::alPrintReadsInFasta) )
        extension = "fa";
    else if( cmdnum == eAlFastq)
        extension = "fastq";
    else if( cmdnum == eAlSam )
        extension = "sam";
    else if( cmdnum == eAlBed )
        extension = "bed";
    sStr outFileName;
    idx qty = formIValue("qty", 0);
    if( qty == -1 )
        mySubID = -2;
    if( objs.dim() ) {
        if( qty == -1 ) {
            outFileName.printf("o%s-%s-All", objs[0].Id().print(), cmd);
        } else {
            if( mySubID == sNotIdx ) {
                outFileName.printf("o%s-%s-Unaligned", objs[0].Id().print(), cmd);
            }
            outFileName.printf("o%s-%s-%"DEC, objs[0].Id().print(), cmd, mySubID);
        }
    } else {
        outFileName.printf("r%"DEC"-%s-%"DEC, reqId, cmd, mySubID);
    }
    if( par.High > 0 )
        outFileName.printf("-%"DEC, par.High);
    outFileName.printf(".%s", extension);
    outBinHeaders(true, "%s", outFileName.ptr());
    if( cmdnum == eAlMatch ) {
        out.printf(0, "Alignment #,Reference #,Reference Identifier,Read #,Read Identifier,Score,Direction,Length,Reference Start,Reference End,Read Start,Read End\n");
        outBinData(out.ptr(0), out.length());
    }

    /*
     path=pathList.ptr();
     viochunk=(start>>32)&0xFFFFFFFF;
     if(viochunk)path=sString::next00(path,viochunk);
     start&=0xFFFFFFFF;
     par.currentChunk=viochunk;
     */
    idx res, iVis = 0;

    out.cut(0);

    res = 0;
    if( cmdnum == eAlCount ) {
        const char * childProcessedList;

        sVec<idx> processedSubs;
        if( (childProcessedList = formValue("childProcessedList", 0)) ) {
            sString::scanRangeSet(childProcessedList, 0, &processedSubs, 0, 0, 0);
            sStr childPath, childFilePath, childFileNames;
//                         objs[is].generatePathStoreManager(&childPath, false);
            for(idx iS = 0; iS < Sub.dim(); ++iS) {
                childFileNames.printf(0, "SNPprofile-%"DEC".csv", iS);
                childFilePath.printf("%s", childFileNames.ptr());
                if( sFile::size(childFilePath) ) {
//                             childProcessedList.vadd(1,iS);
                }
            }
        }
        res = bioal->countAlignmentSummaryBySubject(stat);
        if( !res ) {
            stat[0].found = bioal->Qry->dim();
            stat[0].foundRpt = bioal->Qry->getlongCount();
        }
        sStr cov_flnm;
        objs[0].getFilePathname(cov_flnm, "coverage_dict");
        sVec<idx> cov_dicT;
        sVec<idx> * cov_dic = 0;
        if( cov_flnm.length() ) {
            cov_dicT.init(cov_flnm.ptr(), sMex::fSetZero | sMex::fReadonly);
            if( cov_dicT.dim() )
                cov_dic = &cov_dicT;
        }
        //if(iAls==cntAls-1)
        sStr listOfIon;
        const char * ionProfiler = pForm->value("profilerID", 0);
        if( ionProfiler ) {
            listOfIon.addString(ionProfiler, sLen(ionProfiler));
        }
        const char * ionObjIDs = pForm->value("ionObjs", 0);
        if( ionObjIDs ) {
            if( ionProfiler || listOfIon.length() ) {
                listOfIon.addString(",", 1);
            }
            listOfIon.addString(ionObjIDs, sLen(ionObjIDs));
        }

        idx annotationExtensionRequested = pForm->ivalue("extendAnnot");

        if( annotationExtensionRequested ) {
            const char * ionType = pForm->value("ionType", "u-ionAnnot");
            const char * fileNameTemplate = pForm->value("file", "ion");
            sHiveIonSeq hi(user, listOfIon.ptr(0), ionType, fileNameTemplate);

            //hi.addIonWander("simpleIDLookup", "a=find.annot(id='$id'); b=find.annot(record=a.record); dict(b.type,b.id); print(b.seqID,b.id,b.type);");
            if( ionProfiler ) {
                hi.addIonWander("profilerIDLookup",
                    "a=foreach('total_contig_length','total_number_of_contigs','mapped_coverage(percentage_reference)','average_coverage_of_contigs'); b=find.annot(seqID='$id',type=a.1)[0:1]; dict(b.type,b.id);print(b.seqID,b.id,b.type);");
                if( ionObjIDs ) {
                    sStr ionQry;
                    if( hi.ionCnt > 2 ) {
                        ionQry.printf(0, "a=2-%"DEC":find.annot(id='$id', type='$type'); b=2-%"DEC":find.annot(record=a.record); dict(b.type,b.id);print(b.seqID, b.id,b.type);c=2-%"DEC":find.annot(seqID=b.seqID,type='FEATURES',id='source');d=2-%"DEC":find.annot(record=c.record);dict(d.type,d.id);", hi.ionCnt, hi.ionCnt,hi.ionCnt,hi.ionCnt);
                    } else
                        ionQry.printf(0, "a=2:find.annot(id='$id', type='$type');b=2:find.annot(record=a.record);dict(b.type,b.id);print(b.seqID, b.id,b.type);c=2:find.annot(seqID=b.seqID,type='FEATURES',id='source');d=2:find.annot(record=c.record);dict(d.type,d.id);");
                    hi.addIonWander("complexIDLookup", ionQry.ptr(0));
                }
            } else {
                hi.addIonWander("simpleIDLookup", "a=find.annot(id='$id'); b=find.annot(record=a.record); dict(b.type,b.id); print(b.seqID,b.id,b.type);c=find.annot(seqID=b.seqID,type='FEATURES',id='source');d=find.annot(record=c.record);dict(d.type,d.id);");
                hi.addIonWander("complexIDLookup", "a=find.annot(id='$id', type='$type'); b=find.annot(record=a.record); dict(b.type,b.id);  print(b.seqID, b.id,b.type);c=find.annot(seqID=b.seqID,type='FEATURES',id='source');d=find.annot(record=c.record);dict(d.type,d.id);");
                //hi.addIonWander("simpleIDLookup", "a=find.annot(seqID='$id'); b=foreach('total_contig_length','total_number_of_contigs');c=find.annot(id=a.id,type=b.1);dict(c.type,c.id);");//print(a.seqID,a.id,a.type);");

            }
            res = bioal->printAlignmentSummaryBySubject(stat, &out, &par, reportZeroHits, reportTotals, reportFailed, start, cnt, processedSubs.dim() ? &processedSubs : 0, cov_dic, sHiveIonSeq::annotMap, (void*) &hi);
            // res = bioal->printAlignmentSummaryBySubject(stat, &out,&par,reportZeroHits,reportTotals,reportFailed,start,cnt,processedSubs.dim()?&processedSubs:0,cov_dic,sHiveIonSeq::annotMap,(void*)&hiVec);
        } else {
            res = bioal->printAlignmentSummaryBySubject(stat, &out, &par, reportZeroHits, reportTotals, reportFailed, start, cnt, processedSubs.dim() ? &processedSubs : 0, cov_dic);
        }
        if( formBoolValue("info") ) {
            out.printf("info,%"DEC",%"DEC"\n", start, res);
        }
    } else if( cmdnum == eAlMatch && (mySubID >= 0 || qty == -1) ) {
        res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printMatchSingle, &par);
    } else if( cmdnum == eAlBed && (mySubID >= 0 || qty == -1) ) {
        res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printBEDSingle, &par);
    } else if( cmdnum == eAlView && (mySubID >= 0 || qty == -1) ) {
        res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printAlignmentSingle, &par);
    } else if( cmdnum == eAlSam ) {
        bool userOriginal = formBoolValue("useOriginalID", false);
        if( qty == -1 ) {
            sViosam::convertVioaltIntoSam(bioal, -1, 0, 0, userOriginal, 0, par.outF); // Print all references' alignments into file
        } else {
            sViosam::convertVioaltIntoSam(bioal, mySubID, 0, 0, userOriginal, 0, par.outF); // Print only single reference specified in SAM format
        }
    } else if( cmdnum == eAlFasta ) {
        if( par.navigatorFlags & sBioal::alPrintMultiple ) {
            par.navigatorFlags = sBioal::alPrintMultiple | sBioal::alPrintSequenceOnly | sBioal::alPrintQuery;
            res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printFastaSingle, &par);
        } else {
    //        par.navigatorFlags |= sBioal::alPrintAsFasta;
            par.navigatorFlags &= ~((idx)sBioal::alPrintQualities);
            res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printFastXSingle, &par);
        }
    } else if( cmdnum == eAlFastq ) {
        par.navigatorFlags |= sBioal::alPrintQualities;
        res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printFastXSingle, &par);
    } else if( cmdnum == eAlConsensus ) {
        sStr alName;
        if( !objs[0].propGet("name", &alName) ) {
            objs[0].IdStr(&alName);
        }
        out.printf(">Consensus from alignment %s\n", alName.ptr());
        idx mode = 0;
        if( formBoolIValue("overlap") )
            mode |= sBioal::alConsensusOverlap;
        res = bioal->getConsensus(out, par.wrap, mode);
    } else if( (cmdnum == eAlStack || cmdnum == eAlStackSubj) && (mySubID >= 0 || formBoolValue("multiple")) ) {
        //PRINT SUBJECT
        if( (par.navigatorFlags & sBioal::alPrintMode) || cmdnum == eAlStackSubj ) {
            const char * sub = bioal->Sub->seq(mySubID);
            //if(par.rangestart>Sub.len(mySubID) || !sub)
            //    break;
            if( !formBoolValue("multiple") ) {
                if( par.rangestart < Sub.len(mySubID) && sub ) {
                    sStr tSub;
                    sString::searchAndReplaceSymbols(&tSub, bioal->Sub->id(mySubID), 0, ",", " ", 0, true, true, false, false);
                    out.printf(0, "Element #,%5"DEC", (+) ,%5"DEC",", mySubID + 1, par.rangestart + 1);

                    for(idx isx = par.rangestart; isx <= par.rangeend; ++isx)
                        out.printf("%c", sBioseq::mapRevATGC[sBioseq::mapComplementATGC[(idx) sBioseqAlignment::_seqBits(sub, isx, ~(sBioseqAlignment::fAlignBackwardComplement))]]);
                    out.printf(",%"DEC",%s,Repeats,%"DEC",%"DEC, par.rangeend + 1, tSub.ptr(), par.High + 1, par.High - par.rangestart + 1);
                    if( &par.regp )
                        out.printf(",,");
                    out.printf("\n");
                }
            } else {
                out.printf(0, "Element #,Subject #, directionality,Start,Alignments,End,Subject,Repeats,Highlight,Range");
                if( &par.regp )
                    out.printf(",,");
                out.printf("\n");
            }
        }
        //-----------
        if( cmdnum == eAlStack )
            res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printAlignmentSingle, &par);
    } else if( (cmdnum == eAlMutBias) && (mySubID >= 0) ) {
        par.navigatorFlags = (sBioal::alPrintMutBiasOnly | sBioal::alPrintTouchingOnly | sBioal::alPrintNonFlippedPosforRev);
        sVec<idx> mutBias(sMex::fSetZero);
        mutBias.add(1);
        par.userPointer = &mutBias;
        cnt = 0;
        res = bioal->iterateAlignments(&iVis, start, cnt, mySubID, sBioal::printAlignmentSingle, &par);
        for(idx i = 0; i < mutBias.dim() / 4; ++i)
            out.printf("%"DEC",%"DEC",%"DEC",%"DEC",%"DEC"\n", i + 1, mutBias[4 * i], mutBias[4 * i + 1], mutBias[4 * i + 2], mutBias[4 * i + 3]);
    } else if ( cmdnum == eAlSaturation ) {
        par.navigatorFlags |= sBioal::alPrintInRandom;
        if(par.regp) {
            par.navigatorFlags |= sBioal::alPrintRegExpSub;
        }
        if( formBoolValue("reducedSaturation",false) ) {
            par.navigatorFlags |= sBioal::alPrintSaturationReduced;
        }

        //we use 'currentChunk' to serve as minReads filter
        par.currentChunk = formIValue("minReads", 0);
        sDic< idx > refHits;
//        refHits.mex()->init(sMex::fSetZero);
        par.userPointer = (void *)&refHits;
        par.alCol = 2;
        par.wrap = formIValue("wrap",10000);
        if( bioal->dimAl()/par.wrap > 1000 ) {
            par.wrap = bioal->dimAl()/1000;
        }

        res = bioal->iterateAlignments(&iVis, start, 0, -2, sBioal::printSubSingle, &par,  sBioal::getSaturation, &par);
        out.printf("reads,transcripts\n");
        idx * hits = 0, binHits = 1;
        idx maxCnt = bioal->dimAl();
        sStr tk_buf;

        hits = refHits.get( tk_buf.ptr() );

        if( (par.navigatorFlags&sBioal::alPrintSaturationReduced) ) {
            maxCnt = res;
        } else {
            res = bioal->countAlignmentSummaryBySubject(stat);
            maxCnt = 0;

            for(idx is = 0 ; is < bioal->Sub->dim() ; ++is) {
                if( par.navigatorFlags&sBioal::alPrintCollapseRpt )
                    maxCnt+=stat[is+1].found;
                else
                    maxCnt+=stat[is+1].foundRpt;
            }
        }

        idx binCnt = maxCnt/par.wrap;
        for(idx ih = 0 ; ih < binCnt ;  ++ih) {
            tk_buf.printf(0,"bin-%"DEC,ih+1);
            hits = refHits.get( tk_buf.ptr() );
            if(hits) {
                binHits = *hits;
            }
            out.printf("%"DEC", %"DEC"\n", par.wrap*(ih+1), binHits);
        }

        out.printf("%"DEC", %"DEC"\n", maxCnt, refHits.dim() - binCnt - 1 );
    } else if( (cmdnum == eAlIon) ) {

//        const char * ionObjIDs = pForm->value("ionObjs",0);
//        const char * ionType=pForm->value("ionType","u-ionAnnot");
//        const char * fileNameTemplate=pForm->value("file","ion");
//        sHiveIonSeq hi(user,ionObjIDs, ionType,fileNameTemplate);
//
//        hi.addIonWander("simpleIDLookup", "a=find.annot(id='$id'); b=find.annot(record=a.record); dict(b.type,b.id); print(b.seqID,b.id,b.type);");
//        hi.addIonWander("complexIDLookup", "a=find.annot(id='$id', type='$type'); b=find.annot(record=a.record); dict(b.type,b.id);  print(b.seqID, b.id,b.type);");
//
//        sIO io(0,(sIO::callbackFun)::printf);
//
//        sDic <sStr > dic;
//        hi.annotMap(&io,0, &dic,"167006432"  _ "gi|000111|refSeq|NC_001447.1| wkuf" _ "gi|388570767|kokokokokok kwlkefnlkn"__ );

    }

    if( out ) {
        outBinData(out.ptr(0), out.length());
    }

    if( par.regp )
        regfree(par.regp);
    if( !iVis ) {
        out.printf("\n\nNothing Found\n");
        outBinData(out.ptr(), dataForm.length());
    }

//::printf("JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ\n\n");
//exit(0);
    //outHtml();
    //if(res)return res;
    return 1;
}

idx DnaCGI::CmdProfile(idx cmdnum)
{
    if( !objs.dim() ) {
        return 1;
    }
    sUsrObj& obj = objs[0];
    sVec<sHiveId> parent_proc_ids;
    obj.propGetHiveIds("parent_proc_ids", parent_proc_ids);
    std::auto_ptr<sUsrObj> al(user->objFactory(parent_proc_ids.dim() ? parent_proc_ids[0] : sHiveId::zero));

    if( !al.get() || !al->Id() ) {
        return 1;
    }
    sStr parentAlignmentPath;
    al->getFilePathname00(parentAlignmentPath, "alignment.hiveal"_"alignment.vioal"__);
    if( !parentAlignmentPath ) {
        return 1;
    }
    sHiveal hiveal(user);
    ;

    sHiveseq Sub(user, al->propGet00("subject", 0, ";"), hiveal.parseSubMode(parentAlignmentPath));
    if( !Sub.dim() && cmdnum != eProfSNPcalls ) {
        return 1;
    }
    idx mySubID = pForm->ivalue("idSub") - 1;
    sStr profPath;
    // initialize the SNP profile table
    // If it is -1, that means 0 was passed in an all SNPs need to be grabbed for the summary

    bool isConcatenated = true;
    obj.getFilePathname(profPath, "SNPprofile.csv");
    if( !sFile::exists(profPath) ) {
        isConcatenated = false;
        obj.getFilePathname(profPath, "SNPprofile-%"DEC".csv", mySubID + 1);
    }
    sFil SNPprofile(profPath, sMex::fReadonly); // SNPprofile will be empty if 0 is passed in for mySubID (for all references)

    if( mySubID != -1 && cmdnum != eProfGWatch ) {
        if( !SNPprofile.length() && (cmdnum != eProfSNPcalls && cmdnum != eProfVCF) )
            return 1;
    }

    idx start = formIValue("start");
    idx cnt = formIValue("cnt");

    sStr out;

    switch(cmdnum) {
        case eProfSNP:
            return 1;
        case eProfNoiseIntegral:
        case eProfFreqIntegral: {

            //TODO fix downloadable file name
            sStr outFileName;
            outFileName.printf("o%s-s%"DEC"-%sProfileIntegrals.csv", obj.Id().print(), mySubID + 1,((cmdnum==eProfNoiseIntegral)?"Noise":"Frequency"));
            outBinHeaders(true,"%s",outFileName.ptr());
            real resolution = obj.propGetR("noiseProfileResolution");
            if(!resolution) resolution = sBioseqSNP::noiseProfileResolution;
            if( cmdnum == eProfFreqIntegral ) {
                resolution = obj.propGetR("freqProfileResolution");
                if(!resolution) resolution = sBioseqSNP::freqProfileResolution;
            }
            sStr freqFile;
            obj.getFilePathname(freqFile, "%s.csv", ( cmdnum == eProfFreqIntegral )?"FreqProfile":"Noise");
            sFil fl(freqFile, sMex::fReadonly);
            if( fl.length() ) {
                sDic<sVec<real> > integrals;
                const char * buf = fl.ptr(), * bufend = fl.ptr()+ fl.length();
                buf = sBioseqSNP::binarySearchReferenceNoise( buf, bufend, mySubID+1, false );
                bufend = sBioseqSNP::binarySearchReferenceNoise( buf, bufend, mySubID+1,true );
                sBioseqSNP::integralFromProfileCSV( buf, bufend, integrals, resolution );
                sBioseqSNP::snpOutIntegrals(out,integrals,0,true);
            }

            outBinData(out.ptr(0), out.length());
        } return 1;

        case eProfWith0: {
            idx end = cnt ? start + cnt : Sub.len(mySubID);

            sStr path;
            if( !obj.getFilePathname(path, "SNPprofile-%"DEC".csv", mySubID + 1) ) {
                return 1;
            }
            sFil SNPprofileT(path, sMex::fReadonly);
            if( !SNPprofileT.length() ) {
                return 1;
            }

            sStr outFileName;
            if( !start && !cnt ) {
                outFileName.printf("o%s-SNPprofile_withzeros-%"DEC".csv", obj.Id().print(), mySubID + 1);
            } else {
                // use random prefix since data will vary as user changes start/cnt
                outFileName.printf("o%s-%"DEC"-SNPprofile_withzeros-%"DEC".csv", obj.Id().print(), reqId ? reqId : getpid() + rand(), mySubID + 1);
            }
            outBinHeaders(true, "%s", outFileName.ptr());

            sBioseqSNP::ParamsProfileIterator params(&out);
            params.outF = stdout;

            const char * SNPline = SNPprofileT.ptr(), *endSNP = SNPprofileT.ptr() + SNPprofileT.length();
            while( SNPline < endSNP && *SNPline && *SNPline != '\r' && *SNPline != '\n' )
                ++SNPline;
            fwrite(SNPprofileT.ptr(), 1, SNPline - SNPprofileT.ptr(), params.outF);
            fwrite("\n", 1, 1, params.outF);

            idx iVis = 0;
            idx snpCmp = obj.propGetI("snpCompare");
            const char * sub_seq = Sub.seq(mySubID);
            sStr uncompressedSeq;
            sBioseq::uncompressATGC(&uncompressedSeq, sub_seq, 0, Sub.len(mySubID));

            params.userPointer = (void *) &uncompressedSeq;
            params.userIndex = snpCmp;
            params.isCoverageThrs = 0;
            params.coverageThrs = 0;
            sBioseqSNP::iterateProfile(&SNPprofileT, end, &iVis, start, cnt, sBioseqSNP::printCSV, &params);

        }
            return 1;
        case eProfVCF: {
            sStr outFileName;
            idx subStart = mySubID, subEnd = mySubID + 1;

            if( mySubID < 0 ) {
                // all subjects
                subStart = 0;
                subEnd = Sub.dim();
                start = cnt = 0;
                outFileName.printf("o%s-SNPprofile-all.vcf", obj.Id().print());
            } else if( !start && !cnt ) {
                outFileName.printf("o%s-SNPprofile-%"DEC".vcf", obj.Id().print(), mySubID + 1);
            } else {
                // use random prefix since data will vary as user changes start/cnt
                outFileName.printf("o%s-%"DEC"-SNPprofile-%"DEC".vcf", obj.Id().print(), reqId ? reqId : getpid() + rand(), mySubID + 1);
            }
            outBinHeaders(true, "%s", outFileName.ptr());

            sBioseqSNP::ParamsProfileIterator params(&out);
            params.outF = stdout;
            params.threshold = formRValue("cutOffCall", 0.05);

            sStr subNames;
            for(idx curSub = subStart; curSub < subEnd; ++curSub)
                subNames.printf("%s ", Sub.id(curSub));

            sViosam::createVCFheader(params.outF, subNames.ptr(), params.threshold);

            for(idx curSub = subStart; curSub < subEnd; ++curSub) {

                //sStr path;
                //if( !obj.getFilePathname(path, "SNPprofile-%"DEC".csv", curSub + 1) ) {
                //    continue;
                //}
                //sFil SNPprofileT(path,sMex::fReadonly);
                //if(!SNPprofileT.length()){
                //    return 1;
                //}
                idx iVis = 0, end = Sub.len(curSub);
                params.userPointer = (void *) &Sub;
                params.userIndex = curSub;
                params.isCoverageThrs = 1;
                params.coverageThrs = 0;
                params.chrName.cut(0);
                params.chrName.printf("%s", (const char *) (Sub.id(curSub)));
                params.seq = Sub.seq(curSub);

                if( isConcatenated ) {
                    params.iSub = curSub + 1;
                    sBioseqSNP::iterateProfile(&SNPprofile, end, &iVis, start, cnt, sViosam::convertSNPintoVCF, &params);
                } else {
                    profPath.cut(0);
                    obj.getFilePathname(profPath, "SNPprofile-%"DEC".csv", curSub + 1);
                    sFil SNPprofileOld(profPath, sMex::fReadonly);
                    sBioseqSNP::iterateProfile(&SNPprofileOld, end, &iVis, start, cnt, sViosam::convertSNPintoVCF, &params);
                }

            }

        }
            return 1;

        case eProfContig:
        case eProfSummary: {

            idx gapWindowSize = formIValue("gapWindowSize", 30);
            idx gapThreshold = formIValue("gapThreshold", 1);
            idx minGapLength = formIValue("minGapLength", 10);
            idx whatToPrint= formIValue("whatToPrint", 3);

            //sBioseqSNP::ProfileStat summaryps;
            //summaryps.initilize();

            // Grab the sBioseq object based on the subject pased in
            sBioseq& qry = Sub;

            bool all = false;

            idx end;
            if( mySubID == -1 ) {
                end = qry.dim();
                mySubID++; // Correct for subtraction above...passed in as 0 but loop should start at 0 not -1
                all = true;
            } else{
                end = mySubID + 1;
            }

            // If printing all, prepare the file header
            if( all && cmdnum == eProfSummary ) {
                out.addString(
                    "SeqID,Reference Name,Total Reference Genome Length,Total Contig Length,Mapped Coverage (% Reference),Average Coverage of Contigs, RPCM (Reads Per Contig base per Million mapped reads),Total Number of Contigs,Total Length of them Unmapped Regions,Unmapped Regions (% Reference),Average Coverage of Gaps,Total Number of Gaps Found\n");
            }
            if ( cmdnum == eProfContig ) {
                if( all ) out.printf("seq,");
                out.printf("#,start,end,length,Average Coverage,Has Coverage\n");
            }

            // Loop through each chromosome/segment for data
            for(; mySubID < end; mySubID++) {
//                float covg = 0.0;
//                float gapcovg = 0.0;

                sVec<sBioseqSNP::ProfileGap> pg;
                sStr tmp_profPath;
                if( isConcatenated )
                    obj.getFilePathname(tmp_profPath, "SNPprofile.csv");
                else
                    obj.getFilePathname(tmp_profPath, "SNPprofile-%"DEC".csv", mySubID + 1);
                if( !sFile::exists(tmp_profPath) ) {
                    return 1;
                }

                sFil SNPprofileSummary(tmp_profPath, sMex::fReadonly);
                if( !SNPprofileSummary.ok() )
                    continue;
                // Grab the current Profile Stat for the current reference

                sBioseqSNP::ProfileStat ps;
                sBioseqSNP::snpDetectGaps(&ps, &pg, &SNPprofileSummary, Sub.len(mySubID), gapWindowSize, gapThreshold, minGapLength, isConcatenated ? mySubID + 1 : 0);
                ps.totalContigLength = ps.reflen - ps.totalGapLength;

                sVec < sBioal::Stat >  alStatistics;
                idx totAlRpts = 0;
                hiveal.Sub = &Sub;
                hiveal.parse(parentAlignmentPath.ptr());
                idx res = hiveal.countAlignmentSummaryBySubject(alStatistics);
                if( !res ) { //if nothing found then being in profiler is already bad
                    return 1;
                } else {
                    for( idx iAS = 1 ; iAS < alStatistics.dim() ; ++iAS ) {
                        totAlRpts += alStatistics[iAS].foundRpt;
                    }
                }

                if( cmdnum == eProfSummary ) {
                    if( !all ) {
                        out.printf("- General Information -,\n");
                        out.printf("Total Reference Genome Length,%"DEC"\n", ps.reflen);
                        out.printf("Number of Reference Genomes,%"DEC"\n", Sub.dim());
                        out.printf("- Mapped Regions -,\n");
                        out.printf("Total Contig Length,%"DEC"\n", ps.totalContigLength);
                        out.printf("Mapped Coverage (%% Reference),%.2lf\n", ps.contigsPart);
                        out.printf("Average Coverage of Contigs,%"DEC"\n", ps.averageContigCoverage);
                        out.printf("RPCM (Reads Per Contig base per Million mapped reads),%"DEC"\n", (idx)(1000000*((real)ps.averageContigCoverage/totAlRpts)));
                        out.printf("Total Number of Contigs,%"DEC"\n", ps.totalContigsNumber);
                        out.printf("- Unmapped Regions -,\n");
                        out.printf("Total Length of the Unmapped Regions,%"DEC"\n", ps.totalGapLength);
                        out.printf("Unmapped Regions (%% Reference),%.2lf\n", ps.gapsPart);
                        out.printf("Average Coverage of Gaps,%"DEC"\n", ps.averageGapCoverage);
                        out.printf("Total Number of Gaps Found,%"DEC"\n", ps.totalGapsNumber);
                    } else {
                        out.printf("%"DEC",", mySubID);
                        out.printf("\"%s\",", Sub.id(mySubID));
                        out.printf("%"DEC",", ps.reflen);
                        out.printf("%"DEC",", ps.totalContigLength);
                        out.printf("%.2lf,", ps.contigsPart);
                        out.printf("%"DEC",", ps.averageContigCoverage);
                        out.printf("%"DEC",", (idx)( 1000000*((real)ps.averageContigCoverage/totAlRpts)));
                        out.printf("%"DEC",", ps.totalContigsNumber);
                        out.printf("%"DEC",", ps.totalGapLength);
                        out.printf("%.2lf,", ps.gapsPart);
                        out.printf("%"DEC",", ps.averageGapCoverage);
                        out.printf("%"DEC"\n", ps.totalGapsNumber);
                    }
                } else if ( cmdnum == eProfContig ){
                    if( !cnt )
                        cnt = pg.dim();
                    idx end = start + cnt;
                    if( end > pg.dim() )
                        end = pg.dim();
                    for(idx i = start; i < end; ++i) {
                        sBioseqSNP::ProfileGap & spg = pg[i]; //sStr * p=(spg.hasCoverage) ? &contigTable: &gapTable ;
/*
                        print-false     print-true      Value
                        0               0               0
                        0               1               1
                        1               0               2
                        1               1               3
*/


                        if(spg.hasCoverage && !(whatToPrint&01)  )
                            continue;
                        if(!spg.hasCoverage && !(whatToPrint&02)  )
                            continue;

                        if( all ) {
                            out.printf("\"%s\",",Sub.id(mySubID));
                        }
                        out.printf("%"DEC",%"DEC",%"DEC",%"DEC",%"DEC",%s\n", i + 1, spg.start + 1, spg.end + 1, spg.length, spg.averageCoverage, spg.hasCoverage ? "true" : "false");
                    }
                }
            }
            if( cmdnum == eProfSummary ) {
                if( all ) {
                    outBin(out.ptr(), out.length(), 0, true, "o%s-Summary-all.csv", obj.Id().print());
                } else
                    outBin(out.ptr(), out.length(), 0, true, "o%s-Summary-%"DEC".csv", obj.Id().print(), mySubID + 1);
                return 1;

            } else if( cmdnum == eProfContig ) {
                if( all ) {
                    outBin(out.ptr(), out.length(), 0, true, "o%s-Contigs-all.csv", obj.Id().print());
                } else
                    outBin(out.ptr(), out.length(), 0, true, "o%s-Contigs-%"DEC".csv", obj.Id().print(), mySubID + 1);
            }
        }
            return 1;
        case eProfGWatch: {

            out.printf("Start,End,Type,Signal,Description,Reference\n");

            idx length;
            if( mySubID == -1 ) {
                length = Sub.dim() - 1;
            } else {
                length = mySubID;
            }

            for(; mySubID <= length; mySubID++) {
                profPath.cut(0);
                obj.getFilePathname(profPath, "SNPprofile-%" DEC ".csv", mySubID);

                sFil SNPprofile(profPath, sMex::fReadonly);
                sBioseqSNP::SNPRecord Line;

                Line.position = (unsigned int) -1;
                for(const char * SNPline = SNPprofile.ptr(), *endSNP = SNPprofile.ptr() + SNPprofile.length(); SNPline && SNPline < endSNP; SNPline = sBioseqSNP::SNPRecordNext(SNPline, &Line, endSNP)) {
                    if( Line.position == (unsigned int) -1 ) {
                        continue;
                    }
                    const char * seq = Sub.seq(mySubID - 1);
                    idx one = (idx) sBioseqAlignment::_seqBits(seq, Line.position - 1, 0);

                    for(idx ic = 0; ic < 4; ++ic) {
                        if( one == ic )
                            continue;
                        if( Line.freq(ic) < 0.01 || Line.atgc[ic] == 0 )
                            continue;
                        sStr refPreservedQuotes;
                        sString::escapeForCSV(refPreservedQuotes, Sub.id(mySubID - 1));
                        out.printf("%"DEC",%"DEC",%c,%.2f,-,%s\n", Line.position, Line.position, sBioseq::mapRevATGC[ic], Line.freq(ic), refPreservedQuotes.ptr());
                    }
                }
            }
            outBin(out.ptr(), out.length(), 0, true, "gwatch.csv");
        }
            return 1;

        case eProfSNPcalls: {
            sBioseqSNP::ProfileSNPcallParams SPC;

            // Check if meant to download or display to screen
            if( pForm->ivalue("down", 0) )
                SPC.outF = stdout;

            SPC.snpCallCutoff = formRValue("cutOffCall", 0.05);
            SPC.isORF = formIValue("isORF", 0);
            SPC.consensusAAMode = formIValue("consensusAAMode", 0);
            SPC.codonScale = formIValue("codonScale", 0);
            idx start = formIValue("start", 0), cnt = formIValue("cnt", -1), iSubStart = 0, iSubEnd = Sub.dim();
//            idx end = formIValue("end",0);
            idx sub_start = formIValue("sub_start", 0);
            idx sub_end = formIValue("sub_end", 0);
            SPC.nsSNVs = SPC.isORF ? formIValue("nsSNV", 0) : 0;
            idx resolution = formIValue("resolution", 0);
            resolution = resolution ? resolution : 0;
            idx rsID = formIValue("rsID", 0);
            sStr snpCalls;

            const char * aaOutPut = pForm->value("AAcode", 0);
            if( aaOutPut ) {
                sStr fmt("%%b=0|single=%x|triple=%x|full=%x;", sBioseq::eBioAAsingle, sBioseq::eBioAAtriple, sBioseq::eBioAAfull);
                sString::xscanf(aaOutPut, fmt, &SPC.AAcode);
            }

            const char * fls = formValue("Files", 0, "Not found");

            if( SPC.snpCallCutoff < 0 )
                SPC.snpCallCutoff = 0.05;
            else if( SPC.snpCallCutoff > 1 )
                SPC.snpCallCutoff = 1;

            snpCalls.printf("Subject,Position,Relative letter,SNP,Frequency,Coverage,Entropy,Length");
            if( rsID )
                snpCalls.printf(",rsID");
            if( SPC.isORF )
                snpCalls.printf(",ProteinId,start,end,AA Position,AA Ref,AA Sub,Annotation File");
            snpCalls.printf("\n");

            if( mySubID >= 0 ) {
                iSubStart = mySubID;
                iSubEnd = mySubID + 1;
                outBinHeaders(true, "o%s-SNPcall-%"DEC"-%1.4f.csv",obj.IdStr(), mySubID + 1, SPC.snpCallCutoff);
            } else {
                outBinHeaders(true, "o%s-SNPcall-all-%1.4f.csv",obj.IdStr(), SPC.snpCallCutoff);
            }
            sStr path;
            sFil SNPprofilePerR;
            if( isConcatenated ) {
                obj.getFilePathname(path, "SNPprofile.csv");
                SNPprofilePerR.init(path, sMex::fReadonly);
            }

            sHiveId ionObj(fls);
            sHiveIon hionAnnot(user, ionObj.print(), "u-ionAnnot", "ion");

            for(idx iSub = iSubStart; iSub < iSubEnd; ++iSub) {
                if( isConcatenated ) {
                    SPC.iSub = iSub + 1;
                } else {
                    SNPprofilePerR.destroy();
                    path.cut(0);
                    obj.getFilePathname(path, "SNPprofile-%"DEC".csv", iSub + 1);
                    SNPprofilePerR.init(path, sMex::fReadonly);
                }
                if( !SNPprofilePerR.length() ) {
                    continue;
                }
                sStr subName;
                if( formIValue("hideSubjectName", 0) ) {
                    subName.printf("ref#%"DEC, iSub + 1);
                } else {
                    subName.printf(Sub.id(iSub));
                }
                // ion wander
                sIonWander * hiWander=0;
                sIonWander * hiWanderComplex=0;

                if( fls && sLen(fls) > 1 && !sIs(fls, "Not found") ) {

                   hiWander = hionAnnot.addIonWander("rangesLookUp","seq=foreach(\"%s\");a=find.annot(#range=possort-max,seq.1,$start,seq.1,$end);unique.1(a.record);restriction=find.annot(seqID=a.seqID,record=a.record,id=CDS);proteinId=find.annot(seqID=restriction.seqID,record=restriction.record,type=protein_id);askStrand=find.annot(seqID=restriction.seqID,record=restriction.record,type=strand);askJoin=find.annot(seqID=restriction.seqID,record=restriction.record,type=listOfConnectedRanges);",subName.ptr(0));
                   SPC.iWander = hiWander;

                   hiWanderComplex = hionAnnot.addIonWander("rangesLookUpComplex","k=find.annot(id='$id', type='$type');a=find.annot(#range=possort-max,k.seqID,$start,k.seqID,$end);unique.1(a.record);restriction=find.annot(seqID=a.seqID,record=a.record,id=CDS);proteinId=find.annot(seqID=restriction.seqID,record=restriction.record,type=protein_id);askStrand=find.annot(seqID=restriction.seqID,record=restriction.record,type=strand);askJoin=find.annot(seqID=restriction.seqID,record=restriction.record,type=listOfConnectedRanges);");
                   //hiWanderComplex = hionAnnot.addIonWander("rangesLookUpComplex","k=find.annot(id='304308996',type='gi');a=find.annot(#range=possort-max,k.seqID,$start,k.seqID,$end);unique.1(a.record);restriction=find.annot(seqID=a.seqID,record=a.record,id=CDS);proteinId=find.annot(seqID=restriction.seqID,record=restriction.record,type=protein_id);askJoin=find.annot(seqID=restriction.seqID,record=restriction.record,type=listOfConnectedRanges);");
                   SPC.iWanderComplex = hiWanderComplex;
                }

                sBioseqSNP::snpCalls(&SNPprofilePerR, Sub.seq(iSub), subName, sub_start, sub_end ? sub_end : Sub.len(iSub) - 1, start, Sub.len(iSub), cnt, &snpCalls, &SPC, resolution, rsID);
            }
            outBinData(snpCalls.ptr(), snpCalls.length());
        }
            return 1;

        case eProfProtGen: {
//            const char * locus=formValue("locus");
//            idx considerMutStart=formIValue("considerMutStart",0);
//            idx considerMutEnd=formIValue("considerMutEnd",0);

            const char * annotFileObjId = formValue("Files", 0);
            sStr protSeq;
            sVec<sHiveId> annotIDListToUse;
            sHiveId::parseRangeSet(annotIDListToUse, annotFileObjId);

            sVec<sVioAnnot> annotList;
            sHiveannot::InitAnnotList(user, annotList, &annotIDListToUse);

            if( annotList.dim() ) {
                idx iSub = 0;
                sStr subName;
                subName.printf(Sub.id(iSub));
                sBioseqSNP::protSeqGeneration(Sub.seq(iSub), subName, Sub.len(iSub), &protSeq);
            }

            outBinData(protSeq.ptr(), protSeq.length());
        }
            return 1;

        case eProfConsensus: {
            idx defline = pForm->ivalue("defline", 1);
            idx iVis = 0;
            outBinHeaders(true, "o%s-Consensus-%"DEC".fa", obj.Id().print(), mySubID + 1);

            sStr fasta_header;

            sBioseqSNP::ParamsProfileIterator params(&out);
            if( pForm->ivalue("down", 0) )
                params.outF = stdout;
            if( isConcatenated )
                params.iSub = mySubID + 1;
            params.wrap = pForm->ivalue("wrap", 120);
            params.consensusThrs = formRValue("consensus_thrshld", 0) / 100;
            const char * gaps_input = formValue("gaps", 0);
            idx gaps_choice = 0;
            if( gaps_input ) {
                fasta_header.printf("%%b=0|fill=%i|skip=%i|split=%i;", sBioseqSNP::ePIreplaceGaps, sBioseqSNP::ePIskipGaps, sBioseqSNP::ePIsplitOnGaps);
                sString::xscanf(gaps_input, fasta_header, &gaps_choice);
            }
            fasta_header.printf(0, ">%s Consensus Sequence", Sub.id(mySubID));
            params.iter_flags = gaps_choice;

            if( defline ) {
                if( params.iter_flags != sBioseqSNP::ePIsplitOnGaps ) {
                    out.printf("%s\n", fasta_header.ptr());
                } else {
                    params.userPointer = (void *) fasta_header.ptr();
                }
            }
            if( gaps_choice == sBioseqSNP::ePIreplaceGaps )
                params.userPointer = (void *) Sub.seq(mySubID);
//            params.userIndex = 1;
            idx t_cnt = cnt > 0 ? cnt + 1 : cnt;
            idx bp_found = sBioseqSNP::iterateProfile(&SNPprofile, Sub.len(mySubID), &iVis, start, t_cnt, sBioseqSNP::snpOutConsensus, &params);
            if( cnt > 0 && bp_found >= t_cnt ) {
                out.printf(out.length() - 1, "...");
            }
            outBinData(out.ptr(), out.length());
        }
            return 1;
    };
    return 1;
}

idx DnaCGI::CmdRecomb(idx cmdnum)
{

     sUsrObj& obj = objs[0];

     sVec<sHiveId> parent_proc_ids;
     obj.propGetHiveIds("parent_proc_ids", parent_proc_ids);
     std::auto_ptr<sUsrObj> al(user->objFactory(parent_proc_ids.dim() ? parent_proc_ids[0] : sHiveId::zero));

     if( !al.get() || !al->Id() ) {
     return 1;
     }
     sStr parentAlignmentPath;
     al->getFilePathname00(parentAlignmentPath, "alignment.hiveal"_"alignment.vioal"__);
     if( !parentAlignmentPath ) {
     return 1;
     }
     sHiveal hiveal(user);

     sHiveseq Sub(user, al->propGet00("subject", 0,";"), hiveal.parseSubMode(parentAlignmentPath));
     if( !Sub.dim() ) {
     return 1;
     }


     sStr statement;
     switch(cmdnum) {
         case eRecombCross:{
             sDic<sStr> cross1;
             sDic<sStr> cross2;
             idx mySub1 = pForm->ivalue("sub1",0) - 1;
             idx mySub2 = pForm->ivalue("sub2",0) - 1;
             outBinHeaders(true,"o%s-cross-%"DEC"-%"DEC".csv",obj.IdStr(),mySub1+1,mySub2+1);
             if(mySub1 < 0 || mySub2 < 0 || mySub1 == mySub2) {
                 return 1;
             }
             const char * id_sub1 = Sub.id(mySub1);
             const char * id_sub2 = Sub.id(mySub2);
             sStr subId1;
             sString::searchAndReplaceSymbols(&subId1, id_sub1, 0, ",", " ", 0, true, true, false, false);
             sStr subId2;
             sString::searchAndReplaceSymbols(&subId2, id_sub2, 0, ",", " ", 0, true, true, false, false);
             sStr out;out.printf(0,"position,%s,%s\n",subId1.ptr(),subId2.ptr());

             idx len1 = Sub.len(mySub1);
             idx len2 = Sub.len(mySub2);

             sHiveIon ionWander(user,objs[0].IdStr(),0,"ionRecombinant");
             sIonWander * iWander_1 =0;
             sIonWander * iWander_2 =0;

             statement.printf(0,"b=find.annot(seqID=\"%s\",type=\"%s\");print(b.seqID);dict(b.pos,b.id);", id_sub1, id_sub2);
             iWander_1 = ionWander.addIonWander("query1",statement.ptr());
             iWander_1->resultCumulator = &cross1;
             iWander_1->traverse();

             statement.printf(0,"b=find.annot(seqID=\"%s\",type=\"%s\");print(b.seqID);dict(b.pos,b.id);", id_sub2, id_sub1);
             iWander_2 = ionWander.addIonWander("query2",statement.ptr());
             iWander_2->resultCumulator = &cross2;
             iWander_2->traverse();

             idx last = sMax(len1,len2);
             sIonAnnot::sIonPos ion_p;
             bool isPrevValid=true, isPrevZero = true;
             for (idx iS=0; iS < last; ++iS){
                 ion_p.s32.start = iS;
                 ion_p.s32.end = iS;

                 sStr * value1 = cross1.get((void *)&ion_p,sizeof(ion_p));if(value1)value1->add0();
                 sStr * value2 = cross2.get((void *)&ion_p,sizeof(ion_p));if(value2)value2->add0();
                 if( value1 || value2 ) {
                     if(!isPrevValid && !isPrevZero){
                         out.printf("%"DEC",0,0\n",iS-1);
                     }
                     out.printf("%"DEC",%s,%s\n",iS,(value1?value1->ptr():"0"),(value2?value2->ptr():"0"));
                     isPrevZero = false;
                     isPrevValid=true;
                 } else {
                     if(isPrevValid) {
                         out.printf("%"DEC",0,0\n",iS);
                         isPrevZero = true;
                     } else {
                         isPrevZero = false;
                     }
                     isPrevValid = false;
                 }
             }

             outBinData(out.ptr(), out.length());
         } break;
     }
    return 1;

}

idx DnaCGI::CmdPopulation(idx cmdnum)
{
    if( !objs.dim() ) {
        return 1;
    }
    sUsrObj& obj = objs[0];
    sVec<sHiveId> parent_proc_ids;
    obj.propGetHiveIds("parent_proc_ids", parent_proc_ids);
    std::auto_ptr<sUsrObj> al(user->objFactory(parent_proc_ids.dim() ? parent_proc_ids[0] : sHiveId::zero));
    if( !al.get() || !al->Id() ) {
        return 1;
    }
    sStr parentAlignmentPath;
    al->getFilePathname00(parentAlignmentPath, "alignment.hiveal"_"alignment.vioal"__);
    if( !parentAlignmentPath ) {
        return 1;
    }
    sHiveal hiveal(user, parentAlignmentPath);
    sHiveseq Sub(user, al->propGet00("subject", 0, ";"), hiveal.getSubMode());
    if( !Sub.dim() && cmdnum != eProfSNPcalls ) {
        return 1;
    }


    idx mySubID = formIValue("mySubID", 0);
    --mySubID;


    const char * contig_print = formValue("contig_print",0);
    idx c_print=0;

    sStr cp_fmt("%%b=0|seq=%x|al=%x|cov=%x|comp=%x|break=%x|sum=%x|prev=%x;", sViopop::ePrintContigSeq, sViopop::ePrintContigAl, sViopop::ePrintContigCov, sViopop::ePrintContigComp, sViopop::ePrintContigBreakpoints, sViopop::ePrintContigSummary, sViopop::ePrintContigPrevalence);

    if (contig_print){
        sString::xscanf(contig_print, cp_fmt, &c_print);
    }



    sStr outFileName;
    if( objs.dim() ) {
        outFileName.printf("o%s-%s", objs[0].Id().print(), cmd);
    } else {
        outFileName.printf("r%"DEC"-%s", reqId, cmd);
    }

    if( cmdnum>=ePopContig && cmdnum<=ePopPermutations ) {
        if( c_print == sViopop::ePrintContigSeq || c_print == sViopop::ePrintContigAl ) {
            outFileName.printf(".fasta");
        } else if ( c_print&(sViopop::ePrintContigCov|sViopop::ePrintContigSummary) ){
            outFileName.printf(".csv");
        }
        else {
            outFileName.printf(".txt");
        }
    }
    else if ( cmdnum == ePopClones ) {
        outFileName.printf(".fasta");
    } else {
        outFileName.printf(".csv");
    }

    outBinHeaders(true, "%s", outFileName.ptr());
    idx iVis = 0;
    sStr out, *outP;
    outP = &out;

    sViopop::ParamCloneIterator params(outP);
    params.minCov = formIValue("minCloneCov", 0);
    params.minLen = formIValue("minCloneLen", 0);
    params.minSup = formIValue("minCloneSup", 0);
    params.isNormCov = formIValue("normCloneCov", 0);
    params.hiddenClones = formValue("hideClones", 0);
    params.similarity_cnt = formIValue("simCnt", 10);
    params.similarity_threshold = formRValue("simThrs", 0)/100;
    params.mergeHidden = formIValue("mergeHidden", 0);
    params.resolution = formIValue("resolution", 0);
    params.sStart = formIValue("pos_start", 0) - 1;
    params.sEnd = formIValue("pos_end", 0) - 1;
    params.fastaTmplt = formValue("fasta_tmplt", 0);
    params.showSimil = formIValue("showSimil", 1);
    params.covThrs = formIValue("covThrs",0);
    params.cnt = formIValue("clcnt",0);
    idx t_cnt = params.cnt?params.cnt:sIdxMax;
    if( c_print == sViopop::ePrintContigPrevalence ) {
        params.userPointer = (void*)&t_cnt;
        params.cnt = 0;
    }
    if( !params.cnt ) {
        params.cnt = sIdxMax;
    }
    sStr ft_fmt("%%b=0|composition=%x|simple=%x|numbers=%x;", sViopop::clPrintFastaTitleComposition, sViopop::clPrintFastaTitleSimple, sViopop::clPrintFastaTitleNumbersOnly);
    const char * fastatitle_print = formValue("fasta_title",0,"composition");
    if (fastatitle_print){
        idx ft_print=0;
        sString::xscanf(fastatitle_print, ft_fmt, &ft_print);
        if( !ft_print )
            ft_print = sViopop::clPrintFastaTitleComposition;
        params.flags |=ft_print;
    }

    if( !formBoolValue("maskLowDiversity",true) ) {
        params.flags |= sViopop::clPrintLowDiversityBreaks;
    }

    if( c_print == sViopop::ePrintContigBreakpoints ) {
        params.userPointer = (void *)&Sub;
    }
    idx gapsFrame = formIValue("noGapsFrame");
    if( gapsFrame > 0 && c_print != sViopop::ePrintContigAl ) {
        params.flags |= sViopop::clPrintNoGapsFrame;
        if( gapsFrame > 1 ) {
            params.flags |= sViopop::clSkipSupportedDeletions;
        }
    }

    if( params.similarity_threshold < 0 ) {
        params.similarity_threshold = 0;
    }
    else if( params.similarity_threshold > 1 ) {
        params.similarity_threshold = 1;
    }

    sStr pathList_buf, pathSimil_buf, pathSkp2Gps_buf, pathGps2Skp_buf;
    const char *pathList = 0, *pathSimil = 0, *pathSkp2Gps = 0, *pathGps2Skp = 0;

    pathList = obj.getFilePathname00(pathList_buf, "clones.viopop"__);
    if (params.showSimil) {
        pathSimil = obj.getFilePathname00(pathSimil_buf, "clones.simil"__);
    }
    if ( (params.flags&sViopop::clSkipSupportedDeletions) && !(c_print&sViopop::ePrintContigSummary) ) {
        pathSkp2Gps = obj.getFilePathname00(pathSkp2Gps_buf, "skip2gapSupprted.map"__);
        pathGps2Skp = obj.getFilePathname00(pathGps2Skp_buf, "gap2skipSupprted.map"__);
    }
    else if( (params.flags&sViopop::clPrintNoGapsFrame) && !(c_print&sViopop::ePrintContigSummary) ) {
        pathSkp2Gps = obj.getFilePathname00(pathSkp2Gps_buf, "skip2gap.map"__);
        pathGps2Skp = obj.getFilePathname00(pathGps2Skp_buf, "gap2skip.map"__);
    }

    sFil similFl(pathList, sMex::fReadonly);
    sViopop viopop(pathList, pathSimil, pathGps2Skp, pathSkp2Gps);
    if( !viopop.isok() ) {
        return 1;
    }
    if( !(c_print&sViopop::ePrintContigSummary) ) {
        viopop.gapFrameLevel = params.flags;
    }
    if( runAsBackEnd() ) {
        viopop.progress_CallbackFunction = sQPrideProc::reqProgressStatic;
        viopop.progress_CallbackParam = proc_obj;
    }

    sDic< sVec<idx> > mergeTree;
    sDic< sVec<sViopop::contigComp> > mergeCompTree;
    viopop.buildMergeDictionary(&params, mergeTree, viopop.dimCl());
    viopop.buildMergeCompositionDictionary( &params, mergeCompTree);

    const char * clone_ids = formValue("cloneIDs",0);

    //ePopExtended     <-   ePopRegionCons
    //ePopContig       <-   ePopRegions
    //ePopPermutations <-   ePopCloneCons

    switch (cmdnum) {
        case ePopHierarchy:{
            params.flags|=sViopop::clPrintSummary|sViopop::clPrintTreeMode;
            viopop.iterateClones(&iVis,0,0,sViopop::printHierarchySingle,&params);

            outBinData(out.ptr(), out.length());

        }return 1;
        case ePopClones: {
            params.flags |= sViopop::clPrintSummary | sViopop::clPrintTreeMode;
            params.out->printf("Clone ID,Start,End,showStart,showEnd,Bifurcated ID,Merged ID,Bifurcation Pos,Merge Pos,Max Coverage,Coverage,First Diff,Last Diff,# of points of support,Differences,Bifurcation Statistics");
            viopop.iterateClones(&iVis, 0, 0, sViopop::printAllClones, &params);

            outBinData(out.ptr(), out.length());

        }return 1;

        case ePopContig:{
            params.flags|=sViopop::clPrintSummary|sViopop::clPrintConsensus;
            sStack < sVec< sViopop::cloneRegion > > compositions;
            sVec<idx> clIds;
            viopop.getContigCompositions(clone_ids, compositions, clIds, &params);
            if( compositions.dim() ) {
                viopop.printContig(compositions, clIds, out, &params, c_print);
            }
            outBinData(out.ptr(), out.length());
        }
        return 1;

        case ePopExtended:{
            params.flags|=sViopop::clPrintSummary|sViopop::clPrintConsensus|sViopop::clPrintRegionsConsensus;
            sVec < sVec< sViopop::cloneRegion > > compositions;
            sVec<idx> clIds;
            viopop.getExtendedCompositions(clone_ids, compositions, clIds, &params);;
            if( compositions.dim() ) {
                viopop.printContig(compositions, clIds, out, &params, c_print);
            }
            outBinData(out.ptr(), out.length());
        }
            return 1;

        case ePopPermutations:{
            sStack < sVec< sViopop::cloneRegion > > compositions;
            sVec<idx> clIds;
            viopop.getPermutationsCompositions(clone_ids, compositions, clIds, &params);
            if( compositions.dim() ) {
                viopop.printContig(compositions, clIds, out, &params, c_print);
            }
            outBinData(out.ptr(), out.length());
        }
            return 1;

        case ePopPredictedGlobal:{
            sVec < sVec< sViopop::cloneRegion > > compositions;
            sVec<real> frequencies;
            params.minF = formRValue("minFrequency",((real)1)/pow(10,2))/100;
            params.minDiv = formBoolIValue("minDiversity", 0);
            params.mc_iters = formBoolIValue("mc_iterations", 10000);
            viopop.getPredictedGlobal(compositions, frequencies, &params);
            sVec<idx> clIds(sMex::fExactSize);clIds.add(compositions.dim());
            for(idx i = 0 ; i < clIds.dim() ; clIds[i] = i , ++i);
            params.frequencies = frequencies.ptr();
            if( compositions.dim() ) {
                viopop.printContig(compositions, clIds, out, &params, c_print);
            }
            outBinData(out.ptr(), out.length());
        }
            return 1;

        case ePopStackedStats:{
            params.flags|=sViopop::clPrintSummary|sViopop::clPrintConsensus|sViopop::clPrintRegionsConsensus;
            viopop.printStackedStats(&out);
            outBinData(out.ptr(), out.length());
        }
            return 1;

        case ePopConsensus:{
            params.flags|=sViopop::clPrintSummary|sViopop::clPrintConsensus;
            viopop.printAllSequenceClones(&iVis,0,0,out);
            outBinData(out.ptr(), out.length());
        }
            return 1;
        case ePopCoverage:{
            params.flags|=sViopop::clPrintSummary|sViopop::clPrintCoverage;
            viopop.printAllCoverageClones(&iVis,0,0,out);
            outBinData(out.ptr(), out.length());
        }
            return 1;
    }
    return 1;
}

idx DnaCGI::CmdAnnotation(idx cmd)
{
    sStr line; //line.printf(0,"Locus,start,end,id\n");
    sStr dtaBlobName;
    dtaBlobName.printf(0, "result.csv");  // rename the output from CGI cmd

    if( cmd == eAnotRange ) { //eAnotNumberOfRange
        const char * idToSearch = pForm->value("idToSearch");
        const char * idTypeToSearch = pForm->value("idTypeToSearch");
        const char * isProfiler = pForm->value("isProfiler");
        sVec<sVioAnnot> myAnnotList;
        line.cut(0);
        line.printf("id,name,created\n");
        sHiveannot::getAnnotListFromIdAndIdType(user, idTypeToSearch, idToSearch, &myAnnotList, &line);
        if( isProfiler && myAnnotList.dim() == 0 ) {
            sHiveannot::getAnnotListFromIdAndIdType(user, "gi", idToSearch, &myAnnotList, &line);
        }
    } else if( cmd == eAnotMapperResults ) {
        const char * anotFiles = pForm->value("anotFiles"); // it might be the mix of anot file and profiler
        idx isIon = pForm->ivalue("isIon",0); // it might be the mix of anot file and profiler

        idx showStat = pForm->ivalue("showStat",0); // it might be the mix of anot file and profiler
        const char * typeToShow = pForm->value("typeToShow",0); // it might be the mix of anot file and profiler

        idx cnt = pForm->ivalue("cnt",1000);
        if (cnt==-1) cnt = sIdxMax;
        idx start = pForm->ivalue("start",0);

        sHiveId processID(pForm->value("procID"));

        line.cut(0);
        dtaBlobName.printf(0, "Annotation_Mapper_Results.csv");

        sUsrObj processFolder(*user, processID);
        sStr pathToCrossRange;
        processFolder.getFilePathname(pathToCrossRange, "crossingRanges.csv");


        if (isIon) {
            sTxtTbl tb;
            tb.setFile(pathToCrossRange.ptr(0));
            tb.parse();

            //line.printf("Reference,Start,End,Type,Id\n");
            line.printf("Reference,Pos-Start,Pos-End,Annot-Reference,Annot-Start,Annot-End,Annot-Type,Annot-Id\n");
            sHiveIon hionAnnot(user, anotFiles, "u-ionAnnot", "ion");
            sIonWander * hiWander =0;
            sStr myIonQL("r=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);unique.1(r.pos);printCSV(r.seqID,r.pos,r.type,r.id);");
            if (showStat) {
                myIonQL.printf(0,"r=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);dict(r.id,r.type,\"%s\")",typeToShow);
            }

            hiWander=hionAnnot.addIonWander("myion", myIonQL.ptr(0));

            //sIonWander * hiWander=hionAnnot.addIonWander("myion","r=find.annot(#range=possort-max,$seqID1,$start,$seqID2,$end);dict(r.type)");
            idx tblRowLen = tb.rows();
            idx seqIDLen=0, startLen=0, endLen=0;
            char szStart[128],szEnd[128];szEnd[0]='0';szEnd[1]=':';

            char tmp_seqid[128]; memcpy(tmp_seqid,"chr",3);
            for (idx irow = 0; irow < tblRowLen; irow++) { // START of loop over the rows of the crossing ranges file

                const char * seqID = tb.cell(irow,0,&seqIDLen);  // (idx irow, idx icol, idx * cellLen)
                const char * startString = tb.cell(irow,1,&startLen);
                const char * endString = tb.cell(irow,1,&endLen);
                idx start = -1, end =-1;
                sIScanf(start, startString, startLen, 10); // (_v_r, _v_src, _v_len , _v_base )
                sIScanf(end, endString, endLen, 10);

                sIPrintf(szStart,startLen,start,10); memcpy(szStart+startLen,":0",3); startLen+=2;
                sIPrintf(szEnd+2,endLen,end,10); endLen+=2;
                if (seqID[0] == '"' && seqID[seqIDLen-1]=='"') {
                    seqID = seqID+1; seqIDLen-=2;
                }
                for(idx ikind=0; ikind<2; ++ikind){

                    const char * seqid=seqID;

                    if(ikind==1) {
                        idx sl = seqIDLen;
                        if( strncasecmp(seqid,"chr",3)==0 ) {
                            seqid=seqid+3; // removing 'chr' in seqID
                            seqIDLen-=3;
                        }
                        else if(isdigit(seqid[0])) {
                            memcpy(tmp_seqid+3,seqid,sl);
                            seqid=tmp_seqid;
                            seqIDLen+=3;
                        }
                        else break;
                    }

                    hiWander->setSearchTemplateVariable("$seqID1",7,seqid,seqIDLen);
                    hiWander->setSearchTemplateVariable("$seqID2",7,seqid,seqIDLen);
                    hiWander->setSearchTemplateVariable("$start",6,szStart,startLen);
                    hiWander->setSearchTemplateVariable("$end",4,szEnd,endLen);
                    hiWander->resetResultBuf();
                    hiWander->traverse();
                    if (hiWander->traverseBuf.length() && !showStat) {
                        line.printf("\"%.*s\",%"DEC",%"DEC",",(int)seqIDLen,seqID,start, end);
                        for (idx ipt=0; ipt< hiWander->traverseBuf.length(); ++ipt) {
                            char * myChar = hiWander->traverseBuf.ptr(ipt);
                            line.addString(myChar,1);
                            if (*myChar =='\n' && ipt < hiWander->traverseBuf.length() -1) {
                                line.printf("\"%.*s\",%"DEC",%"DEC",",(int)seqIDLen,seqID,start, end);
                            }

                        }
                        //line.addString(hiWander->traverseBuf.ptr(0), hiWander->traverseBuf.length());
                    }
                }
            } // END of loop
            if (showStat && hiWander->resultCounter.dim()) {
                line.printf(0,"Type,Count\n");
                idx cur_id_len =0;
                idx iCnt=0;
                for(idx i=0; i<hiWander->resultCounter.dim() ; ++i){
                    if (i<start) continue;
                    if (iCnt > cnt-1) break;
                    const char * cur_id = (const char*)hiWander->resultCounter.id(i,&cur_id_len);
                    line.printf("\"%.*s\",%"DEC"\n",(int)cur_id_len, cur_id,*hiWander->resultCounter.ptr(i));
                    iCnt++;
                }
            }

        }
        else {
            // Grab vector of sHiveIds from the file list sent in in csv format
            sVec<sHiveId> idAnotRef;
            sHiveId::parseRangeSet(idAnotRef, anotFiles); //convert and parse list of objID to the vector of sHiveId

            sVec<sVioAnnot> anotList;
            sHiveannot::InitAnnotList(user, anotList, &idAnotRef);
            sHiveannot::outInfo(line, pathToCrossRange.ptr(), anotList);
        }

    } else if( cmd == eIngestGeneList ) { // ingesting gene list of u-idList
        idx fromConvert = formIValue("convert", 0);
        sHiveId objIDToConvert = formValue("objToConvert", 0);
        sFilePath pathToFile;
        sStr fn;
        if( fromConvert ) { // Converting mode
            sStr myExt;
            sUsrObj objFolder(*user, objIDToConvert);
            if( !objFolder.Id() ) {
                error("object is corrupted !");
                return 0;
            }
            objFolder.propGet("ext", &myExt);
            fn.addString("_.", 2);
            fn.addString(myExt.ptr(0), myExt.length());
            objFolder.getFilePathname(pathToFile, fn.ptr(0)); // _.txt
            if( !pathToFile.length() ) {
                error("file not found !");
                return 0;
            }
            sTxtTbl tb;
            tb.setFile(pathToFile.ptr(0));
            tb.parseOptions().colsep = ",\t";
            tb.parse();
            if( tb.cols() != 1 ) {
                error("file should be one column table !");
                return 0;
            }
            if( strcmp(myExt.ptr(0), "genelist") != 0 ) {
                sFilePath newName(pathToFile, "%%dir/_.genelist");
                sFile::rename(pathToFile.ptr(0), newName.ptr(0));
                objFolder.propSet("ext", "genelist");
            }
            objFolder.cast("u-idList");
            line.printf(0, "{\"%"DEC"\":{\"signal\":\"default\",\"data\":{\"from\":\"u-file\",\"to\":\"u-idList\"}}}", objIDToConvert.objId());
            outBin(line.ptr(), line.length(), 0, true, dtaBlobName.ptr(0));
            return 0;
        }
        // ingesting mode
        idx asNew = formIValue("asNew", 1);
        sHiveId objIDToMerge = formValue("objToMerge", 0);
        const char * newFilename = pForm->value("filename", "gene_list");
        const char * geneList = pForm->value("geneList", 0);

        // Creating new object and get the path to the new location
        sStr tmp_dest;
        cfgStr(&tmp_dest, 0, "user.download");
        tmp_dest.printf("geneList-%"DEC"/", reqId ? reqId : getpid() + rand()); // slash at the end important!
        if( !sDir::makeDir(tmp_dest) ) {
            error("Staging area access error: %s", strerror(errno));
            return 0;
        }
        fn.addString(newFilename, sLen(newFilename));
        if( !strstr(newFilename, ".genelist") ) {
            fn.addString(".genelist", 9);
        }
        pathToFile.printf(0, "%s%s", tmp_dest.ptr(0), fn.ptr(0));
        sFil myFile(pathToFile, sMex::fBlockDoubling);
        if( asNew )
            myFile.addString("gene_name\n", 10);
        // if an object specified in order to get the the list of gene
        sFilePath pathToObjMerge;
        sUsrObj objFolder(*user, objIDToMerge);
        if( objFolder.Id() ) {
            objFolder.getFilePathname(pathToObjMerge, "_.genelist"); // _.txt
            sFil myObjFile(pathToObjMerge, sMex::fReadonly);
            myFile.addString(myObjFile.ptr(0), myObjFile.length());
            myObjFile.destroy();
        }
        sStr geneListBy00;
        sString::searchAndReplaceSymbols(&geneListBy00, geneList, 0, ",", 0, 0, true, true, false, true);
        for(const char * p = geneListBy00.ptr(0); p; p = sString::next00(p)) {
            myFile.addString(p, sLen(p));
            myFile.addString("\n", 1);
        }
        myFile.destroy();
        sStr src("file://%s", fn.ptr(0)); // add the source to the metadata
        dmArchiver archHS(*this, pathToFile, src, 0, fn.ptr(0)); // create the dmArchiver object in order to launch the file internally
        idx archReqId = archHS.launch(*user, grpId); // start launching the dmArchiver
        logOut(eQPLogType_Info, "Launching dmArchiver request %"DEC" sequences\n", archReqId);
        line.printf(0, "{\"%s\":{\"signal\":\"default\",\"data\":{\"from\":\"u-file\",\"to\":\"u-idList\"}}}", newFilename);
    } else if( cmd == eAnotDumper ) {
        //const char * anotIDs = pForm->value("anot_ids");
        const char * anotIDs = pForm->value("ionObjs");
        const char * isProfiler = pForm->value("isProfiler");
        sVec<sHiveId> idAnotRef;
        sHiveId::parseRangeSet(idAnotRef, anotIDs);
        sVec<sVioAnnot> annotList;
        sHiveannot::InitAnnotList(user, annotList, &idAnotRef);

        const char * hits_file = pForm->value("hits_file");
        const char * seqIDRaw = pForm->value("seqID");
        sStr seqID;
        if( isProfiler ) {
            sVioAnnot::cleanIdFromProfiler(seqIDRaw, seqID);
        } else
            seqID.printf(0, "%s", seqIDRaw);
        sVec<idx> start, end;
        //idx idIndex = 0 ;
        sVioAnnot::searchOutputParams out_params(",", "\n");
        out_params.outBuf = &line;
        const char * annot_format = formValue("annot_format", 0);
        //const char * whatToPrint = formValue("whatToPrint",0);

        sStr fmt("%%b=0|posID=%x|seqID=%x|annotRange=%x|annotID=%x|annotRangeSingleCell=%x|singleHitRow=%x|singleAnnotRangeRow=%x;", sVioAnnot::ePrintIDpos, sVioAnnot::ePrintSeqID, sVioAnnot::ePrintAnnotRange, sVioAnnot::ePrintAnnotID,
            sVioAnnot::ePrintAnnotRangeInOneColumn, sVioAnnot::ePrintSingleHitRow, sVioAnnot::ePrintSingleAnnotRangeRow);

        if( annot_format ) {
            sString::xscanf(annot_format, fmt, &out_params.rowParams);
        }

        if( hits_file ) {
            /*!
             * Get ranges from file
             */
        } else {
            const char * query_ranges = pForm->value("query_ranges");
            if( query_ranges ) {
                sStr ranges_str;
                sString::searchAndReplaceSymbols(&ranges_str, query_ranges, 0, ",", 0, 0, true, true, false, true);
                sVec<sVec<idx> > ranges;
                sString::scanRangeSetSet(ranges_str.ptr(), &ranges);
                for(idx i = 0; i < ranges.dim(); ++i) {
                    if( ranges[i].dim() ) {
                        start.vadd(1, ranges[i][0]);
                        end.vadd(1, ranges[i][ranges[i].dim() - 1]);
                    }
                }
            }
        }
        if( start.dim() != end.dim() || !start.dim() ) {
            return 1;
        }
        idx output_format = out_params.rowParams;

        if( !(out_params.rowParams & (sVioAnnot::ePrintSingleHitRow | sVioAnnot::ePrintSingleAnnotRangeRow)) ) {
            if( (output_format & sVioAnnot::ePrintIDpos) ) {
                out_params.outBuf->printf("query");
                out_params.outBuf->printf("%s", out_params.column_delim);
            }
            if( (output_format & sVioAnnot::ePrintSeqID) ) {
                out_params.outBuf->printf("sequenceID");
                out_params.outBuf->printf("%s", out_params.column_delim);
            }
            if( (output_format & sVioAnnot::ePrintAnnotRange) ) {
                if( output_format & sVioAnnot::ePrintAnnotRangeInOneColumn ) {
                    out_params.outBuf->printf(" range ");
                } else {
                    out_params.outBuf->printf("start");
                    out_params.outBuf->printf("%s", out_params.column_delim);
                    out_params.outBuf->printf("end");
                    out_params.outBuf->printf("%s", out_params.column_delim);
                }
            }
            out_params.outBuf->printf("id");
            out_params.outBuf->printf("%s", out_params.column_delim);
            out_params.outBuf->printf("idType");

            out_params.outBuf->printf("%s", out_params.row_delim);
        }
        for(idx i = 0; i < annotList.dim(); ++i) {
            sVioAnnot * annotFile = annotList.ptr(i);
            if( annotFile->printRangeSetOnSeqIDSearch(start, end, seqID.ptr(0), out_params) ) {
                out_params.outBuf->printf("%s", out_params.row_delim);
            }
        }
    }

    else if( cmd == eAnotSearch ) {
        const char * whatToPrint = pForm->value("whatToPrint");
        const char * objIDList = pForm->value("objIDs");
        const char * isProfiler = pForm->value("isProfiler");

        sVec<sHiveId> idAnotRef;
        sHiveId::parseRangeSet(idAnotRef, objIDList);
        sVec<sVioAnnot> annotList;
        sHiveannot::InitAnnotList(user, annotList, &idAnotRef);

        sStr idToSearch;
        if( isProfiler ) {
            sVioAnnot::cleanIdFromProfiler(pForm->value("search"), idToSearch);
        } else
            idToSearch.printf(0, pForm->value("search"));
        const char * idTypeToSearch = pForm->value("searchField");
        idx start = pForm->ivalue("start", 0);
        idx end = pForm->ivalue("end", 0);
        idx cnt = pForm->ivalue("cnt", 0);
        if( !cnt )
            cnt = sIdxMax;
        if( !end )
            end = sIdxMax;
        //if (end && start) cnt = end - start;

        dtaBlobName.printf(0, "result.csv");
        if( !objIDList && !idToSearch )
            return 0;

        sStr printElement;
        sString::searchAndReplaceSymbols(&printElement, whatToPrint, 0, "|", 0, 0, true, true, true, true);

        sStr hdr;
        hdr.cut(0); // constructing the Header
        idx sp = 0;
        sVec<sStr> idTypeListToCompare;
        for(const char * ptr = printElement.ptr(0); ptr; ptr = sString::next00(ptr)) { // printing the header based on whatToPrint
            if( sp != 0 )
                hdr.printf(",");
            hdr.printf("%s", ptr);
            sp++;
            sStr * idType = idTypeListToCompare.add(1);
            idType->printf(0, "%s", ptr);

        }

        line.cut(0);
        line.printf(0, "%s\n", hdr.ptr()); // add the header to datablob
        sStr outPut;
        idx nbOfLinePrinted = 0;
        for(idx i = 0; i < annotList.dim(); ++i) {
            sVioAnnot * annotFile = annotList.ptr(i); //
            if( annotFile->printInformationBasedOnIdAndIdType(idTypeToSearch, idToSearch.ptr(), idTypeListToCompare, outPut, nbOfLinePrinted, start, end, cnt) ) {
                line.printf("%s", outPut.ptr());
            }
            if( nbOfLinePrinted > cnt )
                break;
        }

    } else if( cmd == eAnotBrowser ) {
        const char * search = pForm->value("srch", 0);
        const char * refSeqID = pForm->value("refSeqID", 0);
        const char * refObjID = pForm->value("refObjID", 0);

        idx pos_start = pForm->ivalue("pos_start", 0);
        idx pos_end = pForm->ivalue("pos_end", 0);

        idx start = pForm->ivalue("start", 0); // for pagination
        idx end = pForm->ivalue("end", 0);
        idx cnt = pForm->ivalue("cnt", 0);

        idx resolution = pForm->ivalue("resolution", 200);
        idx width = pForm->ivalue("width", 600);
        idx density = pForm->ivalue("density", 20);
        idx maxLayers = pForm->ivalue("maxLayers", 10);

        idx length = 0;
        if( refObjID && refSeqID ) {
            if( sLen(refObjID) > 0 && sLen(refSeqID) > 0 ) {
                sHiveseq ref(user, refObjID);
                for(idx i = 0; i < ref.dim(); ++i) {
                    const char * refId = ref.id(i);
                    if( strcmp(refId, refSeqID) == 0 ) {
                        length = ref.len(i);
                        break;
                    }
                }
            }
        }

        if( !cnt )
            cnt = sIdxMax;
        if( !end )
            end = start + cnt;
        if( !pos_end && !length ) {
            pos_end = sIdxMax;
        } else if( !pos_end && length ) {
            pos_end = length;
        }
        if( !search ) {
            return 0;
        }
        sStr String, objId;
        // src =  301452[UniProt|RefSeq]
        sDic<sVec<sStr> > dico;
        sString::searchAndReplaceSymbols(&String, search, 0, ","_, 0, 0, true, true, true, true);
        for(const char * ptr = String; ptr; ptr = sString::next00(ptr)) {
            objId.cut(0);
            idx len = sString::copyUntil(&objId, ptr, sLen(ptr), "["); // objId = 301452
            if( !len ) {
                return 0;
            }
            idx fnd = dico.find(objId.ptr());
            sVec<sStr> * idType = 0;
            if( len && !fnd ) {
                idType = dico.set(objId.ptr());
            }
            const char * sub = sString::searchStruc(ptr, sLen(ptr), "["__, "]"__, 0, 0);
            if( !sub ) {
                return 0;
            }
            sStr t;
            sString::copyUntil(&t, sub, sLen(sub), "]");
            sStr tt;
            sString::searchAndReplaceSymbols(&tt, t.ptr(), 0, "|"_, 0, 0, true, true, true, true);
            for(const char * pptr = tt; pptr; pptr = sString::next00(pptr)) {
                idType->add()->printf("%s", pptr);
            }
        }

        sStr headerWithReference;
        headerWithReference.printf(0, "seqID,start,end,virtual_start,virtual_end,source,idType-id\n");
        if( refObjID && refSeqID && length ) {
            idx newStart = 0;
            idx newEnd = length;
            if( pos_start )
                newStart = pos_start;
            if( pos_end )
                newEnd = pos_end;
            headerWithReference.printf("ref,%"DEC",%"DEC",%"DEC",%"DEC",reference genome,", newStart, newEnd, newStart, newEnd);
            sStr composeCell("\"seqID\" %s", refSeqID);
            sString::escapeForCSV(headerWithReference, composeCell.ptr(), composeCell.length());
            headerWithReference.printf("\n");
        }

        idx nbOfLinePrinted = 0;
        sStr outPut;
        line.cut(0);
        for(idx i = 0; i < dico.dim(); ++i) {
            const char * key = (const char *) dico.id(i);
            sVec<sStr> * myVec = dico.get(key);
            sHiveId objHiveId(key);
            sUsrObj obj(*user, objHiveId);
            sStr path;
            obj.getFilePathname00(path, ".vioannot"__);
            if( !path.length() )
                continue;
            sVioAnnot a;
            a.init(path, sMex::fReadonly);
            // const char * sourceFileName, sVec < sStr > & idTypeFilterList, sStr & outPut, idx & nbOfLinePrinted, idx start, idx end, idx cnt)
            const char * sourceFileName = obj.propGet("name");
            if( a.printInformationBasedOnIdTypeList(sourceFileName, refSeqID, *myVec, outPut, nbOfLinePrinted, pos_start, pos_end, start, end, cnt) ) {
                line.printf("%s", outPut.ptr());
            }
            if( nbOfLinePrinted > cnt )
                break;
        }

        //const char * contentInCSVFormat,sStr & tableOut, idx _referenceStart, idx _referenceEnd, idx _width, idx _resolution, idx _annotationDensity, idx _maxLayers
        sStr tmpOut;
        idx error = sVioAnnot::runBumperEngine(line.ptr(), line.length(), tmpOut, pos_start, pos_end, width, resolution, density, maxLayers);

        if( tmpOut.length() ) {
            line.cut(0);
            line.printf("%"DEC"\n", error);
            line.printf("%s", headerWithReference.ptr());
            line.printf("%s", tmpOut.ptr());
        }

    } else if( cmd == eAnotFiles ) {
        line.cut(0);
        const char * type = pForm->value("type", "u-annot"); // u-ionAnnot
        idx start = formIValue("start", 0);
        idx cnt = formIValue("cnt", 10);
        if( cnt == -1 )
            cnt = sIdxMax;
        idx iCnt = 0, inum = 0;

        sUsrObjRes annotIDList;
        user->objs2(type, annotIDList);
        line.printf("id,name,created\n");
        for(sUsrObjRes::IdIter it = annotIDList.first(); annotIDList.has(it); annotIDList.next(it)) {
            if( inum < start ) {
                inum++;
                continue;
            }
            sUsrObj ann(*user, *annotIDList.id(it));
            sStr path;
            if( strcasecmp(type, "u-ionAnnot") == 0 ) {
                ann.getFilePathname00(path, "ion.ion"__); // full name by default for ion
            } else
                ann.getFilePathname00(path, ".vioannot"__);
            if( !path.length() )
                continue;
            ann.Id().print(line);
            line.printf(",%s,%"DEC"\n", ann.propGet("name"), sFile::time(path));
            iCnt++;
            if( iCnt > cnt )
                break;
        }
    } else if( cmd == eAnotSeqIDs ) {
        //idx myRef = pForm->ivalue("reference");

        const char * refList = formValue("reference");
        sHiveseq ref(user, refList);

        sDic<idx> idList;
        const char * seqID;
        sStr returnValue;
        for(idx iSub = 0; iSub < ref.dim(); ++iSub) { /* Start loop for the iSub*/
            seqID = ref.id(iSub);
            if( iSub > 0 )
                returnValue.printf("|");
            //returnValue.printf("%s", seqID);
            sString::escapeForCSV(returnValue, seqID);
        }
        //::printf("\n\n%s\n\n", returnValue.ptr());
        line.printf("%s", returnValue.ptr());
    } else if( cmd == eAnotGetIdTypesFromAnotFile ) {
        line.cut(0);

        sHiveId objID(pForm->value("objID"));

        idx start = pForm->ivalue("start", 0);
        idx cnt = pForm->ivalue("cnt", 20);

        if( !objID )
            return 0;

        sUsrObj myAnotFile(*user, objID);

        const char * type = myAnotFile.getTypeName();

        if( strcmp(type, "svc-profiler") == 0 ) {
            line.printf("1,coverage\n2,frequency\n");
        } else {
            sStr path;
            sStr buf;
            sVec<sMex::Pos> bufposes;
            myAnotFile.getFilePathname00(path, ".vioannot"__);

            if( !path.length() )
                return 1;

            sVioAnnot annotObj;

            annotObj.init(path, sMex::fReadonly);

            if( cmd == eAnotGetIdTypesFromAnotFile ) {
                sVec<sStr> listOfIdTypes;
                buf.cut(0);
                bufposes.cut(0);
                annotObj.getAllIdTypes(buf, &bufposes);
                if( cnt == -1 )
                    cnt = bufposes.dim();
                for(idx ii = 0; ii < bufposes.dim(); ++ii) {
                    if( ii < start )
                        continue;
                    if( ii == (cnt + start) )
                        break;
                    line.printf("%"DEC",%s\n", ii + 1, buf.ptr(bufposes[ii].pos));
                }
            }
        }
    } else {
        enum ePrint
        {
            ePrintGi = 0x00000001,
            ePrintLocus = 0x00000002,
            ePrintRangeStart = 0x00000004,
            ePrintRangeEnd = 0x00000008,
            ePrintIdType = 0x00000010,
            ePrintId = 0x00000020
        };
//    if( !objs.dim() ) {
//        return 1;
//    }
        sUsrObj& obj = objs[0];

//    sUsrObj al(*sQPride::user,objs[0].propGetI("parent_proc_ids"));
        sVec<sHiveId> parent_proc_ids;
        obj.propGetHiveIds("parent_proc_ids", parent_proc_ids);
        std::auto_ptr<sUsrObj> al(user->objFactory(parent_proc_ids.dim() ? parent_proc_ids[0] : sHiveId::zero));

        idx subID = pForm->ivalue("subID", 0);
        sHiveseq Sub(user, al->propGet00("subject", 0, ";"));
        idx lengthSeq = Sub.len(subID - 1);
        const char * rawlocus = Sub.id(subID - 1);
        sStr locus;
        locus.cut(0);

        sVioAnnot::cleanIdFromProfiler(rawlocus, locus);
        //
        sStr outFileNameA;
        if( objs.dim() ) {
            outFileNameA.printf("o%s-%"DEC"", objs[0].Id().print(), cmd);
        } else {
            outFileNameA.printf("r%"DEC"-%"DEC"", reqId, cmd);
        }
        outFileNameA.printf(".csv");

        outBinHeaders(true, "%s", outFileNameA.ptr());

        idx giNumber = pForm->uvalue("giNumber");

//        const char * dataName = pForm -> value("dataName",0);
//        const char * idtype = pForm->value("idType", 0);
//        const char * id = pForm->value("id");

        const char * whatToOutPut = pForm->value("whatToOutPut", 0);  // list of what to print in comma separated
        idx whatToSearch = 0;
        sStr fmt("%%b=0|giNumber=%x|locus=%x|rangeStart=%x|rangeEnd=%x|idType=%x|id=%x;", ePrintGi, ePrintLocus, ePrintRangeStart, ePrintRangeEnd, ePrintIdType, ePrintId);

        if( whatToOutPut ) {
            sString::xscanf(whatToOutPut, fmt, &whatToSearch);
        }

        // CDS or mat_peptide --------------------------------
        sStr myDataName;
        myDataName.printf(0, "CDS");
//    idx getCDS = pForm ->uvalue("CDS");
        idx getMatPeptide = pForm->uvalue("mat_peptide");
        if( getMatPeptide == 1 )
            myDataName.printf(0, "mat_peptide");
        //---------------------------------------------------

        // if AnotFile specify -----------------------------------------------
        const char * getFileAnot = pForm->value("Files");

        sVec<sHiveId> idAnotRefRemoveZero;
        if( getFileAnot ) {
            sVec<sHiveId> idAnotRef;
            sHiveId::parseRangeSet(idAnotRef, getFileAnot);
            for(idx aa = 0; aa < idAnotRef.dim(); ++aa) {
                if( idAnotRef[aa] )
                    *idAnotRefRemoveZero.add(1) = idAnotRef[aa];
            }
        }
        sVec<sHiveId> annotIDList;
        sVec<sVioAnnot> annotList;
        sHiveannot::InitAnnotList(user, annotList, &idAnotRefRemoveZero);

        // ------------------------------------------------------------------

        idx recordStart = pForm->ivalue("recordStart", 1);
        if( recordStart < 0 )
            recordStart = 1;
        idx cnt = pForm->ivalue("count", 50);
        if( cnt <= 0 )
            cnt = 50;
        idx position = pForm->ivalue("position", 0);
        if( !position || position < -1 )
            position = 0;
        idx numberRangeStop = pForm->ivalue("numberRangeStop", 0);
        if( !numberRangeStop || numberRangeStop < -1 )
            numberRangeStop = 0;
        idx resolution = pForm->ivalue("resolution", 0);
        if( !resolution || resolution < -1 )
            resolution = 0;

        // iv points to the annot list which is the first to be considered
        // recordStart shows its offset inside of that particular annot

        // positioning
        idx iv, irec;
        for(iv = 0; iv < annotList.dim(); ++iv) {
            idx cntInThis = 0;
            if( annotList[iv].isGBstructure() ) {
                cntInThis = annotList[iv].getTotalRecord();
            } else {
                annotList[iv].getIdByIdType("seqID", &cntInThis);
            }
            if( recordStart <= cntInThis )
                break;
            recordStart -= cntInThis;
        }

        sVec<idx> accumulatedRecords;

        // filtering

        idx cntPassedFilter = 0;
        sVec<idx> subsetRec;
        for(; iv < annotList.dim(); ++iv) {

            idx cntInThis = 0;
            idx recordEnd = 0;
            //-------check what the structure of annotation file, if old structure Continue ------------------
            idx found = 0;
            if( annotList[iv].isGBstructure() ) {

            } else {
                idx * idPtr = annotList[iv].getIdByIdType("seqID", &cntInThis);
                recordEnd = cntInThis;
                sStr buf;
                subsetRec.cut(0);
                for(idx is = 0; is < cntInThis; ++is) {
                    buf.cut(0);
                    annotList[iv].getIdByIdIndex(buf, idPtr[is]);
                    if( locus || giNumber ) {
                        // get the irec if any and push to array
                        sStr gN("%"DEC"", giNumber);
                        if( strcmp(locus, buf.ptr()) == 0 )
                            found = is;
                        else if( strcmp(gN.ptr(), buf.ptr()) == 0 )
                            found = is;
                        if( found < recordStart )
                            continue;

                        recordStart = found;
                        recordEnd = recordStart + 1;
                    }
                }

            }
            for(idx iRec = recordStart; iRec < recordEnd; ++iRec) {
                if( subsetRec.dim() )
                    irec = subsetRec[iRec];
                else
                    irec = iRec;

                ++cntPassedFilter;
                idx * pElement = accumulatedRecords.add(1);
                *pElement = ((iv << 32) | (irec));

                if( cntPassedFilter >= cnt )
                    break;

            }
            if( cntPassedFilter >= cnt )
                break;
            recordStart = 1;
        }

        if( whatToSearch & ePrintGi )
            line.printf("Gi,");
        if( whatToSearch & ePrintLocus )
            line.printf("Locus,");
        if( whatToSearch & ePrintRangeStart )
            line.printf("Start,");
        if( whatToSearch & ePrintRangeEnd )
            line.printf("End,");
        /*  if( whatToSearch & ePrintIdType )
         line.printf("TypeOfId,");*/
        if( whatToSearch & ePrintId )
            line.printf("LocusTag,Product,ProteinId,");
        line.printf("Length,Feature\n");

        switch(cmd) {

            case eAnotDefinition: {

                for(idx iel = 0; iel < accumulatedRecords.dim(); ++iel) {

                    iv = ((accumulatedRecords[iel]) >> 32) & 0xFFFFFFFF;
                    irec = (accumulatedRecords[iel]) & 0xFFFFFFFF;
                    sVioAnnot & Annotation = annotList[iv];
                    if( Annotation.isGBstructure() ) {
                        // nothing for old structure
                    } else {
                        idx cntRanges = 0;
                        idx * indexRangePtr = Annotation.getNumberOfRangesByIdTypeAndId("seqID", locus.ptr(0), cntRanges);
                        sStr locusTag, product, proteinId;   // line.printf("LocusTag,Product,ProteinId,");
                        line.printf("%s,", locus.ptr(0));
                        for(idx irange = 0; irange < cntRanges; ++irange) {
                            locusTag.cut(0);
                            product.cut(0);
                            proteinId.cut(0);
                            idx cntIDsForRange = Annotation.getNberOfIdsByRangeIndex(indexRangePtr[irange]);
                            for(idx iid = 0; iid < cntIDsForRange; ++iid) { /* loop for id list */
                                const char * idPtr, *idTypePtr;
                                Annotation.getIdTypeByRangeIndexAndIdIndex(indexRangePtr[irange], iid, &idPtr, 0, &idTypePtr, 0);

                                if( strcasecmp("locus_tag", idTypePtr) == 0 ) {
                                    locusTag.printf(0, "%s", idPtr);
                                }
                                if( strcasecmp("product", idTypePtr) == 0 ) {
                                    product.printf(0, "%s", idPtr);
                                }
                                if( strcasecmp("ProteinId", idTypePtr) == 0 ) {
                                    proteinId.printf(0, "%s", idPtr);
                                }
                            }
                            idx cntRangeJoints = 0;
                            sVioAnnot::startEnd * rangePtr = Annotation.getRangeJointsByRangeIndex(indexRangePtr[irange], &cntRangeJoints);  // LAM
                            cntRangeJoints /= sizeof(sVioAnnot::startEnd);
                            for(idx irj = 0; irj < cntRangeJoints; ++irj) {
                                line.printf("%"DEC",%"DEC",%s,%s,%s,%"DEC",%s\n", rangePtr[irj].start, rangePtr[irj].end, locusTag.ptr(), product.ptr(0), proteinId.ptr(0), lengthSeq, "CDS");
                            }
                        }
                        if( !cntRanges )
                            line.printf("%d,%d,%s,%s,%s,%"DEC",%s\n", 0, 0, locusTag.ptr(), product.ptr(0), proteinId.ptr(0), lengthSeq, "CDS");
                    }
                }
            }
                break;
            case eAnotNumberOfRange: {

            }
                break;
            default:
                break;

        };
    }
    outBin(line.ptr(), line.length(), 0, true, dtaBlobName.ptr(0));
    return 1; // let the underlying Management code to deal with untreated commands
}

//idx DnaCGI::CmdIon(idx cmd){
//
//    /********************************
//     *  CGI command for ionAnnot files
//     ********/
//    //const char * ionObjIDs = pForm->value("objs",0);
//    //const char * ionType=pForm->value("ionType","u-ionAnnot");
//    //const char * fileNameTemplate=pForm->value("file",0);
//
//
//    sIonWander iWander;
//    /*
//    if( !sHiveIon::InitalizeIonWander(user, &iWander, ionObjIDs, ionType,fileName ))
//        return 0;  // {ops !}
//    */
//    iWander.addIon(0)->ion->init("/home/vsim/data-stage/tax/dst/ncbiTaxonomy",sMex::fReadonly);
//
//    sStr statement;
//
//    switch(cmd) {
//        case eIonAnnotInfo :{
//            statement.printf("a=foreach(9606,2); b=find.taxid_parent(taxid==a.); print(b.taxid,b.parent,b.rank); ");
//        } break;
//        case eIonAnnotIdMap :{
//            statement.printf("a=foreach(9606,2); b=find.taxid_parent(taxid==a.); print(b.taxid,b.parent,b.rank); ");
//        } break;
//        default: {
//            //outPutLine.printf("Nothing found");
//        } break;
//    }
//
//    //iWander.debug=1;
//    iWander.traverseCompile(statement.ptr());
//    //sStr t;iWander.printPrecompiled(&t);//::printf("%s\n\n",t.ptr());
//    iWander.traverse();
//
//    //sStr outPutLine;
//    //outBin(outPutLine.ptr(), outPutLine.length(), 0, true, dtaBlobName.ptr(0));
//    return 1; // let the underlying Management code to deal with untreated commands
//}
/*
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/
 _/  Initialization
 _/
 _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 */

idx DnaCGIProc::OnExecute(idx req)
{
    initIfNeeded();
    dnaCGI_qapp->pForm = pForm;


#ifdef _DEBUG
    fprintf(stderr, "qpride form:\n");
    for (idx i=0; i<dnaCGI_qapp->pForm->dim(); i++) {
        const char * key = static_cast<const char*>(dnaCGI_qapp->pForm->id(i));
        const char * value = dnaCGI_qapp->pForm->value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
#endif

//    dnaCGI_qapp->pForm->value("cmd");
    sStr s_cmd(dnaCGI_qapp->pForm->value("cmd"));
    dnaCGI_qapp->cmd = s_cmd.ptr();

    dnaCGI_qapp->reqId = req;
    //bool doDigest = false;
    sStr out;
    dnaCGI_qapp->outP = &out;


    dnaCGI_qapp->proc_obj = this;

    const char * risky_cmds00 = "-qpSubmit"_"-qpRawSubmit"_"-qpProcSubmit"__;
    if( !dnaCGI_qapp->cmd || sString::compareChoice(dnaCGI_qapp->cmd, risky_cmds00, 0, true, 0, true) >= 0 ) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    bool isArchive = dnaCGI_qapp->pForm->boolvalue("arch");

    sStr cgi_dstnamebuf;
    const char * cgi_dstname = dnaCGI_qapp->pForm->value("cgi_dstname","cgi_output");
    cgi_dstnamebuf.addString(cgi_dstname);
    sStr datasource("file://%s",cgi_dstnamebuf.ptr());
    sStr cgi_output_path;

    reqSetData(req, datasource, 0, 0);
    reqDataPath(req, datasource.ptr(7), &cgi_output_path);

    dnaCGI_qapp->setFlOut(fopen(cgi_output_path.ptr(), "w"));
    dnaCGI_qapp->raw = 2; // no headers and no html

    dnaCGI_qapp->pForm->inp("-daemon", "1");
    sStr reqbuf("%"DEC, req);
    dnaCGI_qapp->pForm->inp("req", reqbuf.ptr());

    dnaCGI_qapp->run();

    fclose(dnaCGI_qapp->flOut);
    dnaCGI_qapp->setFlOut(0);

    if( isArchive ) {
        if( !sFile::size(cgi_output_path) ) {
            reqProgress(1, 100, 100);
            reqSetStatus(req, eQPReqStatus_Done);
            return 0;
        }
        const char * dstName = dnaCGI_qapp->pForm->value("arch_dstname");
        const char * fmt = dnaCGI_qapp->pForm->value("ext");
        datasource.printf(0, "file://%"DEC"-%s", req, dstName);
        dmArchiver arch(*this, cgi_output_path, datasource, fmt, dstName);
        arch.addObjProperty("source", "%s", datasource.ptr());
        idx arch_reqId = arch.launch(*user);
        logOut(eQPLogType_Info, "Launching dmArchiver request %"DEC" \n", arch_reqId);
        if( !arch_reqId ) {
            reqProgress(1, 100, 100);
            reqSetStatus(req, eQPReqStatus_Done);
            return 0;
        }
        datasource.printf(0,"arch_%s",cgi_dstnamebuf.ptr());
        dnaCGI_qapp->reqSetData(req, datasource, "%"DEC, arch_reqId);
    } else {
        dnaCGI_qapp->reqRepackData(req, cgi_dstnamebuf.ptr());
    }

    reqProgress(1, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}

void DnaCGIProc::initIfNeeded()
{
    if( dnaCGI_qapp )
        return;
    dnaCGI_qapp = new DnaCGI("config=qapp.cfg"__, "dnaCGI", sApp::argc, sApp::argv, sApp::envp, stdin, true, true);
}

int main(int argc, const char *argv[], const char *envp[])
{
    sApp::args(argc, argv, envp);
    sBioseq::initModule(sBioseq::eACGT);

    sCmdLine cmd;
    if( argc > 1 ) {
        cmd.init(argc, argv);
    }

    if( sString::parseBool(cmd.next("-daemon")) ) {
        sStr tmp;

        DnaCGIProc backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "dnaCGI", sApp::argv[0]));
        return (int) backend.run(argc, argv);
    }

    DnaCGI qapp("config=qapp.cfg"__, "dnaCGI", argc, argv, envp, stdin, true, true);
    qapp.run();
    return 0;
}
