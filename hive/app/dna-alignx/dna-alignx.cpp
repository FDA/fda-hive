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
#include <ssci/math/rand/rand.hpp>
#include <xlib/md5.hpp>
#include <violin/hiveproc.hpp>

#include "dna-alignx.hpp"

DnaAlignX::~DnaAlignX()
{
#if !_DEBUG
    sDir::removeDir(getWorkDir(0, true));
#endif
    delete algorithm;
}

void DnaAlignX::readParams(sVar * pForm, sUsr * user, sVec<sUsrProc> & objs)
{
    keepRefNs = qp.formBoolValue("keepRefNs", keepRefNs);
    minMatchLength = qp.formIValue("minMatchLen", minMatchLength);
    isMinMatchPercentage=(bool)qp.formIValue("minMatchUnit",0);
    maxMissQueryPercent = qp.formIValue("maxMissQueryPercent", maxMissQueryPercent);
    separateHiveseqs = qp.formBoolValue("splitSingle", separateHiveseqs);
    scoreFilter = qp.formIValue("scoreFilter", scoreFilter);
    seedSize = qp.formIValue("seedSize", seedSize);
    evalueFilter = qp.formRValue("evlaueFilter", 0, evalueFilter);
    keepOriginalQryId = qp.formBoolValue("keepOriginalQryId", keepOriginalQryId);
    subbiomode = sBioseq::isBioModeLong( qp.formIValue("subbiomode", subbiomode) ) ? sBioseq::eBioModeLong : sBioseq::eBioModeShort;
    qrybiomode = sBioseq::isBioModeLong( qp.formIValue("qrybiomode", qrybiomode) ) ? sBioseq::eBioModeLong : sBioseq::eBioModeShort;
    sHiveId gtfFileId(qp.formValue("GTFfile"));
    sUsrFile uf(gtfFileId, qp.user);
    if( uf.Id() ) {
        uf.getFile(referenceAnnotationFile);
    }
}

bool DnaAlignX::evalScriptTmplt(const sHiveId & id, sUsrQueryEngine & ql, sStr & scriptName, const idx cnt)
{
    sStr errorMsg, buf;
    sVariant evaled;

    ql.registerBuiltinStringProperty(id, "algorithm_version", algorithm->propGet("version", &buf));
    bool retval = ql.evalTemplate(scriptTemplate, 0, evaled, &errorMsg);
    if( !retval ) {
#if _DEBUG
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "%s", errorMsg.ptr());
#else
        qp.logOut(qp.eQPLogType_Error, "%s", errorMsg.ptr());
#endif
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Invalid script template");
        qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
        return false;
    }
    if( cnt >= 0 ) {
        scriptName.printf("%s%" DEC "-hive-script.sh", getWorkDir(&ql), cnt);
    } else {
        scriptName.printf("%s-hive-final-script.sh", getWorkDir(&ql));
    }
    sFil sf(scriptName.ptr());
    if( !sf.ok() ) {
#if _DEBUG
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Failed to write script file '%s'", scriptName.ptr());
#else
        qp.logOut(qp.eQPLogType_Error, "Failed to write script file '%s'", scriptName.ptr());
#endif
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Script creation failed");
        qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
        return false;
    }
    sf.cut0cut();
    const char *p = evaled.asString();
    sf.add(p, sLen(p));
    sf.destroy();
    sFile::chmod(scriptName);
    return true;
}

static
const char * progressWatchFilename(const char * in)
{
    static sFilePath watch;
    if( !in || !in[0] ) {
        watch.cut0cut(0);
    } else if( !sFile::exists(in) ) {
        watch.makeName(in, "%%dir");
    } else {
        watch.printf(0, "%s", in);
    }
    return watch;
}

DnaAlignX::EIndexerMessage DnaAlignX::subjectDump(const char * subjectPath, idx start, idx cnt)
{
    subjectGet();
    if( !Sub.dim() ) {
        qp.logOut(qp.eQPLogType_Error, "Reference sequences are not specified or corrupted");
        return eIndexerMessage_Error;
    }
    const idx indexChunkSize = subjectGetChunkSize();
    idx totalDim = cnt ? cnt : (Sub.dim() - start), dumpedSize = 0, posInChunk = 0;
    totalDim += start;
    sFil faFile;
    bool doDump = true;
    for(idx chunkCnt = 0, iS = start; iS < totalDim; ++iS) {
        idx seqlen = Sub.len(iS);

        if( indexChunkSize && seqlen > indexChunkSize ) {
            qp.logOut(qp.eQPLogType_Warning, "Subject sequence #%" DEC " length %" DEC " is > %s index chunk size %" DEC, iS, seqlen, getAlgorithmName(), indexChunkSize);
        }
        if( indexChunkSize && posInChunk > 0 && (dumpedSize + seqlen) > indexChunkSize ) {
            faFile.destroy();
        }
        if( !faFile.ok() ) {
            const char * subjectFile = subjectFile00.printf("%s%s%" DEC ".fa", subjectPath, getAlgorithmName(), chunkCnt);
            subjectFile00.add0();
            faFile.init(subjectFile);
            if( !faFile.ok() ) {
                qp.logOut(qp.eQPLogType_Error, "Failed to create file '%s'", subjectFile);
                return eIndexerMessage_Error;
            }
#if _DEBUG
            doDump = faFile.length() ? false : true;
#else
            faFile.cut(0);
#endif
            posInChunk = 0;
            dumpedSize = 0;
            ++chunkCnt;
        }
        const idx seq_line_max = 1024 * 1024L;
        const idx seqlenX = seq_line_max ? seqlen + ((seqlen - 1) / seq_line_max) : seqlen;
        if( doDump ) {
            const char * seqs = Sub.seq(iS, 0);
            if( keepOriginalSubId ) {
                const char * id = Sub.id(iS);
                id += id[0] == '>' ? 1 : 0;
                faFile.printf(">%s\n", id);
                const char * idEnd = strchr(id, ' ');
                idx idLen = idEnd ? idEnd - id : sLen(id);
                idx * pid = idMap.setString(id, idLen);
                if( !pid ) {
                    qp.logOut(qp.eQPLogType_Error, "Out of memory for id mapping: %" DEC " -> [%" DEC "]'%s'", iS, idLen, id);
                    return eIndexerMessage_Error;
                }
                *pid = iS;
            } else {
                faFile.printf(">%" DEC "\n", iS);
            }
            char * const buf = faFile.add(0, seqlenX + 1);
            sBioseq::uncompressATGC(buf, seqs, 0, seqlen, seq_line_max);
            if( keepRefNs ) {
                const char * qua = Sub.qua(iS, 0);
                if( qua ) {
                    const bool quaBit = Sub.getQuaBit(iS);
                    if( quaBit ) {
                        for(idx j = 0, k = 0; j < seqlen; ++j) {
                            if( j > 0 && (j % seq_line_max) == 0 ) {
                                ++k;
                            }
                            if( Sub.Qua(qua, j, true) == 0 ) {
                                buf[j + k] = 'N';
                            }
                        }
                    } else {
                        for(idx j = 0, k = 0; j < seqlen; ++j) {
                            if( j > 0 && (j % seq_line_max) == 0 ) {
                                ++k;
                            }
                            if( qua[j] == 0 ) {
                                buf[j + k] = 'N';
                            }
                        }
                    }
                }
            }
            buf[seqlenX] = '\n';
        }
        dumpedSize += seqlenX;
        ++posInChunk;
        if( !qp.reqProgress(iS, iS, totalDim) ) {
            return eIndexerMessage_Aborted;
        }
    }
    faFile.destroy();
    subjectFile00.add0();
    return eIndexerMessage_Done;
}

static void reSubmitGrp(sQPrideProc & qp, const idx lockReqId)
{
    qp.grpReSubmit(qp.grpId, qp.vars.value("serviceName"), 60, lockReqId);
}

static void grpSetStatus(sQPrideProc & qp, const idx grpid, const idx status, const char * svc)
{
    sVec <idx> reqs;
    qp.grp2Req(grpid, &reqs, svc);
    qp.reqSetStatus(&reqs, status);
}

DnaAlignX::EIndexerMessage DnaAlignX::IndexSubject(sStr & subjectIndexDir)
{
    sStr suffix("%s", getAlgorithmName());
    const idx indexChunkSize = subjectGetChunkSize();
    if( indexChunkSize > 0 ) {
        if( indexChunkSize < (1 * 1024 * 1024L) ) {
            qp.logOut(qp.eQPLogType_Error, "subject chunk size too small: '%" DEC, indexChunkSize);
            return eIndexerMessage_Error;
        }
        suffix.printf("-%" DEC "M", indexChunkSize / 1024 / 1024);
    }
    suffix.printf("%s%s", keepOriginalSubId ? "-Id" : "", keepRefNs ? "-N" : "");
    switch (subbiomode) {
        case sBioseq::eBioModeLong:
            suffix.printf("-long");
            break;
        case sBioseq::eBioModeShort:
            break;
        default:
            qp.logOut(qp.eQPLogType_Error, "subject mode unrecognized: %i", subbiomode);
            return eIndexerMessage_Error;
            break;
    }
    sStr sublist, taxonomy, err;
    sVec<sHiveId> subIdList;
    subjectGet(&sublist, &subIdList);
    subjectIndexDir.cut0cut();
    sStr tokenizedSubList;
    sString::searchAndReplaceSymbols(&tokenizedSubList, sublist, 0, "\n;,", 0, 0, true, true, false, true);
    for(const char * id = tokenizedSubList; id; id = sString::next00(id)) {
        sUsrObj * obj = qp.user->objFactory(sHiveId(id));
        if( !obj ) {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Object %s not accessible", id);
            delete obj;
            return eIndexerMessage_Error;
        }
        obj->propGet("taxonomy", &taxonomy);
        taxonomy.add0(1);
        if( id == tokenizedSubList.ptr() ) {
            obj->getFilePathname(subjectIndexDir, "%s", suffix.ptr());
            if( !subjectIndexDir ) {
                sUsr qpride("qpride", true);
                sUsrObj * tobj = qpride.objFactory(obj->Id());
                if( tobj ) {
                    tobj->addFilePathname(subjectIndexDir, true, "%s", suffix.ptr());
                }
                delete tobj;
            }
        } else {
            subjectIndexDir.cut0cut();
        }
        delete obj;
    }
    taxonomy.add0(2);
    if( !subjectIndexDir || !subjectIndexPersistent() ) {
        sStr prefix("%s-sub-", qp.svc.name), subIdListConcated;
        sHiveId::printVec(subIdListConcated, subIdList, "_", false);
        if( subIdListConcated.length() + prefix.length() + suffix.length() > 250 ) {
            sMD5 sum(subIdListConcated, subIdListConcated.length());
            subIdListConcated.printf(0, "%s_%s_%s", subIdList[0].print(), subIdList[subIdList.dim() - 1].print(), sum.sum());
        }
        sStr varnm("%s.indexDirectory", qp.svc.name);
        subjectIndexDir.cut0cut();
        qp.cfgStr(&subjectIndexDir, 0, varnm);
        suffix.printf("-%s", subIdListConcated.ptr());
        subjectIndexDir.printf("%s%s", prefix.ptr(), suffix.ptr());
    }
    subjectIndexDir.printf("/");

    typedef sDic<idx> TAlgoIdx;

    sStr triggerFinished("%sidx.finished", subjectIndexDir.ptr());
    sStr subjectListFile("%ssubjects.00", subjectIndexDir.ptr());
    bool doIndex = true;
    if( sFile::exists(triggerFinished) ) {
        sFil ftbl(triggerFinished, sMex::fReadonly);
        if( !ftbl.ok() ) {
            qp.logOut(qp.eQPLogType_Error, "Cannot read file '%s'", triggerFinished.ptr());
            return eIndexerMessage_Error;
        }
        TAlgoIdx tbl;
        idx i = 0;
        if( tbl.serialIn(ftbl.ptr(), ftbl.length()) < 0 ) {
            qp.logOut(qp.eQPLogType_Info, "File '%s' is corrupt", triggerFinished.ptr());
            sFile::remove(triggerFinished);
            tbl.empty();
        } else {
            sStr buf;
            idx plen = 0;
            while( i < tbl.dim() ) {
                const char * nm = (const char *) tbl.id(i, &plen);
                buf.printf(0, "%s%.*s", subjectIndexDir.ptr(), (int) plen, nm);
                const idx sz = sFile::size(buf);
                if( !sFile::exists(buf) || sz != *tbl.ptr(i) ) {
                    qp.logOut(qp.eQPLogType_Debug, "Miss file '%s' %" DEC " vs %" DEC " in index file list", buf.ptr(), sz, *tbl.ptr(i));
                    sFile::remove(triggerFinished);
                    break;
                }
                ++i;
            }
        }
        doIndex = !tbl.dim() || i < tbl.dim();
    }
    sStr idMapFile;
    if( keepOriginalSubId ) {
        idMapFile.printf(0, "%sid_map.sdic", subjectIndexDir.ptr());
    }
    if( doIndex || qp.formBoolValue("force_reindex", false) ) {
        if( !subjectVerify(taxonomy, &err) ) {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "%s with %s algorithm", err ? err.ptr() : "Selected reference(s) cannot be used", getAlgorithmName());
            return eIndexerMessage_Error;
        }
        sStr subjectWriteLock("%sindex.lock", subjectIndexDir.ptr());
        idx lreq;
        if( !qp.reqLock(subjectWriteLock, &lreq) ) {
            qp.logOut(qp.eQPLogType_Debug, "Waiting for req %" DEC " to finish indexing '%s'", lreq, subjectIndexDir.ptr());
            reSubmitGrp(qp, lreq);
            return eIndexerMessage_Wait;
        }
        sDir::removeDir(subjectIndexDir);
        if( !sDir::makeDir(subjectIndexDir) ) {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Could not create index directory");
            qp.logOut(qp.eQPLogType_Error, "Could not create index directory '%s'", subjectIndexDir.ptr());
            return eIndexerMessage_Error;
        }
        reSubmitGrp(qp, lreq);
        qp.logOut(qp.eQPLogType_Info, "Started indexing in '%s', lock %" DEC, subjectIndexDir.ptr(), lreq);
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Info, "Indexing subject(s)");
        subjectIdxLockId = lreq;
        subjectFile00.empty();
        idMap.empty();
        const EIndexerMessage ret = subjectDump(subjectIndexDir, 0, Sub.dim());
        if( ret != eIndexerMessage_Done ) {
            if( ret == eIndexerMessage_Error ) {
                qp.logOut(qp.eQPLogType_Error, "dumping indexing dump files in '%s'", subjectIndexDir.ptr());
            }
            return ret;
        }
        if( idMapFile ) {
            sFil fidMap(idMapFile);
            if( !fidMap.ok() ) {
                qp.logOut(qp.eQPLogType_Error, "Cannot create id mapping file '%s'", idMapFile.ptr());
                return eIndexerMessage_Error;
            }
            idMap.serialOut(fidMap);
        }
        const sHiveId & id = qp.objs[0].Id();
        idx cnt = 0;
        for(const char * p = subjectFile00; p; p = sString::next00(p)) {
            sUsrQueryEngine * ql = qp.queryEngineFactory();
            if( ql ) {
                ql->registerBuiltinStringProperty(id, "indexPath", p);
                ql->registerBuiltinStringProperty(id, "referenceFile", p);
                if( referenceAnnotationFile ) {
                    ql->registerBuiltinStringProperty(id, "annotationFile", referenceAnnotationFile.ptr());
                }
                sStr scriptName;
                if( evalScriptTmplt(id, *ql, scriptName, cnt++) ) {
                    sPipe2::CmdLine cmdline;
                    cmdline.exe(scriptName.ptr());
                    cmdline.arg("index");
                    qp.logOut(qp.eQPLogType_Debug, "INDEXING: %s", cmdline.printBash());
                    sIO log;
                    if( qp.exec(cmdline.printBash(), 0, progressWatchFilename(subjectIndexDir), &log) ) {
                        qp.logOut(qp.eQPLogType_Error, "failed to index file '%s'", p);
                        delete ql;
                        return eIndexerMessage_Error;
                    }
                } else {
                    qp.logOut(qp.eQPLogType_Error, "failed to evaluate script");
                    delete ql;
                    return eIndexerMessage_Error;
                }
                delete ql;
            } else {
                qp.logOut(qp.eQPLogType_Error, "failed to obtain query engine");
                return eIndexerMessage_Error;
            }
        }
        {{
            sDir ex;
            ex.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive), subjectIndexDir, "*");
            TAlgoIdx tbl;
            for(const char * f = ex; f; f = sString::next00(f)) {
                idx * sz = tbl.setString(&f[subjectIndexDir.length()]);
                if( sz ) {
                    *sz = sFile::size(f);
                } else {
                    qp.logOut(qp.eQPLogType_Error, "index table cannot add element '%s'\n", f);
                    return eIndexerMessage_Error;
                }
            }
            sFile::remove(triggerFinished);
            sFil ftbl(triggerFinished);
            if( !ftbl.ok() ) {
                qp.logOut(qp.eQPLogType_Error, "Cannot create id mapping file '%s'", triggerFinished.ptr());
                return eIndexerMessage_Error;
            }
            tbl.serialOut(ftbl);
            sFile::remove(subjectListFile);
            sFil slf(subjectListFile);
            if( !slf.ok() ) {
                qp.logOut(qp.eQPLogType_Error, "Cannot create subject chunk list file '%s'", subjectListFile.ptr());
                return eIndexerMessage_Error;
            }
            for(const char * p = subjectFile00; p; p = sString::next00(p)) {
                slf.printf("%s", &p[subjectIndexDir.length()]);
                slf.add0(1);
            }
            slf.add0(1);
        }}
        if( !qp.reqUnlock(subjectWriteLock) ) {
            qp.logOut(qp.eQPLogType_Error, "lock re-grabbed by another request, waiting");
            return eIndexerMessage_Wait;
        }
    }
    subjectFile00.empty();
    sFil slf(subjectListFile, sMex::fReadonly);
    if( !slf.ok() ) {
        qp.logOut(qp.eQPLogType_Error, "Cannot read subject chunk list file '%s'", subjectListFile.ptr());
        return eIndexerMessage_Error;
    }
    for(const char * p = slf; p; p = sString::next00(p)) {
        if( p[0] == '/' || p[0] == '\\' ) {
            const char * p1 = strstr(p, suffix);
            if( !p1 ) {
                qp.logOut(qp.eQPLogType_Error, "Path adjustment failed '%s' suffix '%s'", p, suffix.ptr());
                return eIndexerMessage_Error;
            }
            p = p1 + suffix.length() + 1;
        }
        const char * r = subjectFile00.printf("%s%s", subjectIndexDir.ptr(), p);
        subjectFile00.add0(1);
        if( !sFile::exists(r) ) {
            qp.logOut(qp.eQPLogType_Error, "Missing subject (chunk) file '%s' in '%s'", p, subjectIndexDir.ptr());
            return eIndexerMessage_Error;
        }
    }
    subjectFile00.add0(1);
    if( idMapFile ) {
        sFil fidMap(idMapFile, sMex::fReadonly);
        if( !fidMap.ok() ) {
            qp.logOut(qp.eQPLogType_Error, "Cannot read id mapping file '%s'", idMapFile.ptr());
            return eIndexerMessage_Error;
        }
        idMap.empty();
        if( idMap.serialIn(fidMap.ptr(), fidMap.length()) < 0 ) {
            qp.logOut(qp.eQPLogType_Error, "Id mapping file '%s' is invalid", idMapFile.ptr());
            return eIndexerMessage_Error;
        }
    }
    qp.logOut(qp.eQPLogType_Debug, "Using index id maps %" DEC " in '%s'", idMap.dim(), subjectIndexDir.ptr());
    return eIndexerMessage_Done;
}

idx queryDumpCallback(void *param, sStr * buf, idx initid, idx initseq, idx initqua, idx seqlen)
{

    sQPrideProc* qp = (sQPrideProc*)param;
    if( qp ) {
        qp->reqProgress(buf->pos(), 12, 100);
    }
    return 1;
}

DnaAlignX::EIndexerMessage DnaAlignX::queryIndexChunk(sStr & queryFiles, idx reqSliceId, idx reqSliceCnt, bool get_pair)
{
    sStr qqStr;
    queryGet(&qqStr, get_pair);
    const char * qry;
    idx iq;
    EIndexerMessage res = eIndexerMessage_Error;

    if( !get_pair ) {
        QryList.empty();
    } else {
        res = isPairedEndMandatory() ? eIndexerMessage_Error : eIndexerMessage_Done;
    }
    idx qryListPrevLen = QryList.dim();

    sStr tokenizedQueryList;
    sString::searchAndReplaceSymbols(&tokenizedQueryList, qqStr.ptr(), qqStr.length(), separateHiveseqs ? "\n;" : "\xFF", 0, 0, true, true, false, true);
    idx qrylistdim=0;
    for(iq = 0, qry = tokenizedQueryList.ptr(0); qry; qry = sString::next00(qry), ++iq) {
        sFilePath queryFile(getWorkDir(0), separateHiveseqs ? "%%dir/query%s-%" DEC ".fa" : "%%dir/query%s.fa", get_pair ? "-pair" : "", iq + 1);
        sHiveseq * hs = QryList.set(queryFile.ptr());
        new (hs) sHiveseq(qp.user, qry, qrybiomode);
        if( !hs->dim() ) {
            qp.logOut(qp.eQPLogType_Error, "query is empty: '%s' in mode %i", qry, qrybiomode);
            return eIndexerMessage_Error;
        }
        qrylistdim += hs->dim();
        if( !separateHiveseqs ) {
            break;
        }
    }
    idx slice = (qrylistdim - 1) / reqSliceCnt + 1;
    idx qStart = slice * reqSliceId;
    idx qEnd = qStart + slice;
    sStr err;
    if( !queryVerify((sDic<sBioseq> *) &QryList, &err) ) {
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "%s with %s algorithm", err ? err.ptr() : "Selected read(s) cannot be used", getAlgorithmName());
        return eIndexerMessage_Error;
    }
    idx maxNumberQuery = qp.formIValue("maxNumberQuery", sIdxMax);
    if( maxNumberQuery <= 0 ) {
        maxNumberQuery = sIdxMax;
    }
    const bool filterNsPercent = qp.formBoolValue("filterNs", false);
    const bool keepQryNs = qp.formBoolValue("keepQryNs", true);
    idx qryCumulativeDim = 0;
    frmProduceRandomReadsForNT = qp.formBoolValue("produceRandomReadsForNT", frmProduceRandomReadsForNT);
    for(idx i = qryListPrevLen; i < QryList.dim(); ++i) {
        sHiveseq * Qry1 = &(QryList[i]);
        Qry1->print_callback = queryDumpCallback;
        Qry1->print_callbackParam = &qp;
        sStr qryFile("%s", (const char *) QryList.id(i));

        if( qEnd > Qry1->dim() || qEnd <= 0) {
            qEnd = Qry1->dim();
        }

        if( frmProduceRandomReadsForNT ) {
            idx max_len = Qry1->getMaxLen();
            if (!max_len){
                for(iq = 0; iq < Qry1->dim() && Qry1->len(iq) < 1000; ++iq) {
                }
            }
            else if (max_len < 1000){
                iq = Qry1->dim();
            }
            if( iq >= Qry1->dim() ) {
                frmProduceRandomReadsForNT = 0;
            }
        }

        if( frmProduceRandomReadsForNT ) {
            sFilterseq::randomizer(*Qry1, qryFile, maxNumberQuery);
            res = sFile::size(qryFile) ? DnaAlignX::eIndexerMessage_Done : DnaAlignX::eIndexerMessage_Error;
        } else {
            sFile::remove(qryFile.ptr());
            sFil fileDst(qryFile.ptr());
            if( maxNumberQuery < qEnd - qStart ) {
                idx count = qEnd - qStart;
                sVec<idx> samplingSet;
                samplingSet.resize(count);
                for(idx irnd = 0; irnd < samplingSet.dim(); ++irnd) {
                    samplingSet[irnd] = irnd;
                }
                idx validCount = 0;
                idx avoidInfiniteLoop = sMax(count - (maxNumberQuery * 10), (idx)0);
                do {
                    idx choice = rand() % count;
                    idx randomQuery = samplingSet[choice];
                    samplingSet[choice] = samplingSet[--count];
                    const char *qseqtemp = Qry1->seq(randomQuery);
                    idx qseqlen = Qry1->len(randomQuery);
                    if( qseqtemp && qseqlen != 0 ) {
                        idx isValid = sFilterseq::complexityFilter(qseqtemp, qseqlen, 32, 1.2);
                        if( isValid == 0 ) {
                            isValid = Qry1->printFastXRow(&fileDst, qryInFastQ, randomQuery, 0, 0, qryCumulativeDim, keepOriginalQryId, false, 0, 0, sBioseq::eSeqForward, keepQryNs, filterNsPercent ? 50 : 0);
                            if( isValid ) {
                                ++validCount;
                            }
                        }
                    }
                } while( count > 0 && validCount < maxNumberQuery && (avoidInfiniteLoop < count));
                if( validCount == 0 ) {
                    qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Query didn't pass low complexity Filter");
                    res = eIndexerMessage_Error;
                } else {
                    res = eIndexerMessage_Done;
                }
            } else {
                Qry1->printFastX(&fileDst, qryInFastQ, qStart, qEnd, qryCumulativeDim, keepOriginalQryId, false, 0, 0, keepQryNs, filterNsPercent ? 50 : 0);
                res = eIndexerMessage_Done;
            }
            qryCumulativeDim += Qry1->dim();
        }
        queryFiles.printf("%s", queryFiles ? " " : "");
        sString::escapeForShell(queryFiles, qryFile.ptr());
        if( res != DnaAlignX::eIndexerMessage_Done ) {
            break;
        }
    }
    for(idx i = qryListPrevLen; i < QryList.dim(); ++i) {
        Qry.attach((sBioseq*) &(QryList[i]), 1, QryList[i].dim());
    }
    return res;
}

DnaAlignX::EIndexerMessage DnaAlignX::Align(const char * query, const char * query_pair)
{
    const idx q = sString::cnt00(subjectFile00);
    idx cnt = 0;
    const sHiveId & id = qp.objs[0].Id();
    for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
        sUsrQueryEngine * ql = qp.queryEngineFactory();
        if( ql ) {
            if( !qp.reqProgress(cnt, cnt, q) ) {
                delete ql;
                return eIndexerMessage_Aborted;
            }
            sStr out("%s%" DEC, getWorkDir(ql), cnt);
            ql->registerBuiltinStringProperty(id, "outPath", out.ptr());
            ql->registerBuiltinStringProperty(id, "indexPath", p);
            if( referenceAnnotationFile ) {
                ql->registerBuiltinStringProperty(id, "annotationFile", referenceAnnotationFile.ptr());
            }
            ql->registerBuiltinStringProperty(id, "queryFiles", query);
            if( query_pair ) {
                ql->registerBuiltinStringProperty(id , "paired_queryFiles", query_pair);
            }
            sStr scriptName;
            if( evalScriptTmplt(id, *ql, scriptName, cnt) ) {
                sPipe2::CmdLine cmdline;
                cmdline.exe(scriptName.ptr());
                cmdline.arg("align");
                qp.logOut(qp.eQPLogType_Debug, "ALIGNING: %s", cmdline.printBash());
                sIO log;
                if( qp.exec(cmdline.printBash(), 0, progressWatchFilename(getWorkDir(0, true)), &log) ) {
                    qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Aligner produced an error");
                    qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
                    delete ql;
                    return eIndexerMessage_Error;
                }
            } else {
                delete ql;
                return eIndexerMessage_Error;
            }
            delete ql;
        } else {
            qp.logOut(qp.eQPLogType_Error, "failed to obtain query engine");
            return eIndexerMessage_Error;
        }
    }
    return eIndexerMessage_Done;
}

idx DnaAlignX::ParseAlignment(const idx keepAllMatches, sDic<idx> * unalignedList, FileAlParser * alignParser)
{

    sStr fileList00;
    idx cnt = 0;
    for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
        fileList00.printf("%s%" DEC ".%s", getWorkDir(0), cnt, resultExtension());
        fileList00.add0();
    }
    fileList00.add0();

    flagSet = sBioseqAlignment::fAlignForward;
    if( frmProduceRandomReadsForNT ) {
        flagSet |= sBioseqAlignment::fAlignKeepRandomBestMatch;
    } else {
        if( keepAllMatches == 0 ) {
            flagSet |= sBioseqAlignment::fAlignKeepFirstMatch;
        } else if( keepAllMatches == 1 ) {
            flagSet |= sBioseqAlignment::fAlignKeepBestFirstMatch;
        } else if( keepAllMatches == 3 ) {
            flagSet |= sBioseqAlignment::fAlignKeepAllBestMatches;
        } else if( keepAllMatches == 4 ) {
            flagSet |= sBioseqAlignment::fAlignKeepRandomBestMatch;
        }
    }

    if (alignParser == NULL) {
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Uninitialized align parser");
        qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
        return -1;
    }

    idx countAls = 0;
    sStr errBuf;
    FileAlParser::WriteParams writeParams;
    writeParams.minMatchLength = minMatchLength;
    writeParams.maxMissQueryPercent = maxMissQueryPercent;
    writeParams.isMinMatchLengthPercentage = isMinMatchPercentage;
    writeParams.scoreFilter = scoreFilter;
    writeParams.flagSet = flagSet;
    if (sRC rc = alignParser->writeAls(fileList00.ptr(), writeParams, countAls, errBuf, idMap.dim() ? &idMap : 0, unalignedList)) {
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Failed when converting alignment files to vioals: %s", rc.print());
        qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
        return -1;
    }

    qp.logOut(qp.eQPLogType_Debug, "Parsed %" DEC " alignments\n", countAls);
    return countAls;
}

idx DnaAlignX::FinalProcessing(FileAlParser * alignParser)
{
    if (alignParser == NULL) {
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Uninitialized align parser");
        qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
        return -1;
    }

    const char * resultFileTemplate = qp.formValue("resultFileTemplate", 0);
    if( !resultFileTemplate ) {
        resultFileTemplate = "";
    }

    sVioal::digestParams params;
    params.flags= flagSet;
    params.countHiveAlPieces = 1000000;
    params.combineFiles = false;
    params.minFragmentLength = qp.formIValue("fragmentLengthMin", 0);
    params.maxFragmentLength = qp.formIValue("fragmentLengthMax", 0);

    idx resolveConflicts = qp.formIValue("resolveConflicts", false);
    if( resolveConflicts ) {
        if(resolveConflicts == 1) params.flags |= sBioseqAlignment::fAlignKeepResolveMarkovnikov;
        if(resolveConflicts == 2) params.flags |= sBioseqAlignment::fAlignKeepResolveBalanced;

        idx resolveConflictsScore = qp.formIValue("resolveConflictsScore", false);
        if( resolveConflictsScore == 1 ) params.flags |= sBioseqAlignment::fAlignKeepResolvedHits;
        if( resolveConflictsScore == 2 ) params.flags |= sBioseqAlignment::fAlignKeepResolvedSymmetry;
        if( qp.formBoolValue("resolveConfictsUnique", false) ) params.flags |= sBioseqAlignment::fAlignKeepResolveUnique;
    }

    if( qp.formBoolValue("keepPairedOnly", false) ) params.flags |= sBioseqAlignment::fAlignKeepPairedOnly;
    if( qp.formBoolValue("keepPairOnSameSubject", false) ) params.flags |= sBioseqAlignment::fAlignKeepPairOnSameSubject;
    if( qp.formBoolValue("keepPairOnOppositeStrand", false) ) params.flags |= sBioseqAlignment::fAlignKeepPairDirectionality;

    if( params.minFragmentLength || params.maxFragmentLength  ||
        (params.flags&(sBioseqAlignment::fAlignKeepPairDirectionality|sBioseqAlignment::fAlignKeepPairOnSameSubject|sBioseqAlignment::fAlignKeepPairedOnly) ) ) {
        params.flags |= sBioseqAlignment::fAlignIsPairedEndMode;
    }

    if (sRC rc = alignParser->joinAls(params, 0, resultFileTemplate)) {
        qp.logOut(qp.eQPLogType_Error, "Failed to combine the results: %s", rc.print());
        return 0;
    }

    sStr finalPaths;
    if( getPathsForFinalProcessing(finalPaths) && finalPaths ) {
        sStr finalDst;
        if( finalPaths.length() > (3 * 1024) ) {
            sStr listName("%s%s-finalize.lst", getWorkDir(0), qp.svc.name);
            sFil list(listName);
            if( !list.ok() ) {
                qp.logOut(qp.eQPLogType_Error, "Failed to write final file list to '%s'", listName.ptr());
                return 0;
            }
            sStr tokenizedSubList;
            sString::searchAndReplaceSymbols(&tokenizedSubList, finalPaths, 0, "\r\n\t;, ", 0, 0, true, true, false, true);
            for(const char * f = tokenizedSubList; f; f = sString::next00(f)) {
                list.printf("%s\n", f);
            }
            finalPaths.printf(0, "@%s", listName.ptr());
        }
        qp.reqAddFile(finalDst, "%s", getAlgorithmName());
        sUsrQueryEngine * ql = qp.queryEngineFactory();
        const sHiveId & id = qp.objs[0].Id();
        ql->registerBuiltinStringProperty(id, "outPath", getWorkDir(ql));
        ql->registerBuiltinStringProperty(id, "resultPath", finalDst.ptr());
        if( referenceAnnotationFile ) {
            ql->registerBuiltinStringProperty(id, "annotationFile", referenceAnnotationFile.ptr());
        }
        ql->registerBuiltinStringProperty(id, "finalFiles", finalPaths.ptr());
        sStr scriptName;
        if( evalScriptTmplt(id, *ql, scriptName, -1) ) {
            sPipe2::CmdLine cmdline;
            cmdline.exe(scriptName.ptr());
            cmdline.arg("finalize");
            qp.logOut(qp.eQPLogType_Debug, "FINALIZING: %s", cmdline.printBash());
            sIO log;
            if( qp.exec(cmdline.printBash(), 0, progressWatchFilename(getWorkDir(0)), &log) ) {
                return 0;
            }
        } else {
            return 0;
        }
    }
    return 1;
}

class DnaAlignXProc: public sHiveProc
{
        idx * subjectIdxLockId;
        DnaAlignX * alignx;
        sUsrQueryEngine * ql;

    public:

        DnaAlignXProc(const char * defline00, const char * srv)
            : sHiveProc(defline00, srv), subjectIdxLockId(0), alignx(0), ql(0)
        {
        }
        ~DnaAlignXProc()
        {
            delete alignx;
            delete ql;
        }

        virtual bool OnProgress(idx reqId)
        {
            if( subjectIdxLockId && *subjectIdxLockId ) {
                reSubmitGrp(*this, *subjectIdxLockId);
            }
            return true;
        }
        virtual idx OnExecute(idx req)
        {
            delete alignx;
            alignx = 0;
            delete ql;
            ql = 0;

            sUsrObj * algo = 0;
            sVec<sHiveId> algoids;
            formHiveIdValues("alignSelector", algoids);
            if( algoids.dim() == 1 ) {
                algo = user->objFactory(algoids[0]);
            } else {
                sStr tool;
                formValue("alignSelector", &tool);
                if( sIs("svc-align-", tool.ptr()) ) {
                    tool.printf(0, "^%s$", tool.ptr(10));
                }
                sUsrObjRes res;
                user->objs2("^algorithm-alignment$+", res, 0, "name", tool);
                if( res.dim() > 0 ) {
                    const sHiveId * o = res.firstId();
                    if( o ) {
                        algo = user->objFactory(*o);
                    }
                }
            }
            FileAlParser * alignParser = 0;
            const char * algorithm = 0;
            if( algo ) {
                algorithm = algo->propGet("name");
                if( algorithm && sIs("svc-align-", algorithm) ) {
                    algorithm = &algorithm[sizeof("svc-align-") - 1];
                }
                if( sIsExactly(algorithm, "bowtie") ) {
                    alignx = new DnaAlignXBowtie(*this, algo);
                    alignParser = new SAMParser(*this, alignx->Sub, alignx->Qry);
                } else if( sIsExactly(algorithm, "bowtie2") ) {
                    alignx = new DnaAlignXBowtie2(*this, algo);
                    alignParser = new SAMParser(*this, alignx->Sub, alignx->Qry);
                } else if( sIsExactly(algorithm, "blast") ) {
                    alignx = new DnaAlignXBlast(*this, algo);
                    alignParser = new BLASTParser(*this, alignx->Sub, alignx->Qry, sBioAlBlast::eBlastStandardOut);
                } else if( sIsExactly(algorithm, "blastx") ) {
                    alignx = new DnaAlignXBlastX(*this, algo);
                    alignParser = new BLASTParser(*this, alignx->Sub, alignx->Qry, sBioAlBlast::eBlastProteinOut);
                } else if( sIsExactly(algorithm, "tblastx") ) {
                    alignx = new DnaAlignXTBlastX(*this, algo);
                    alignParser = new BLASTParser(*this, alignx->Sub, alignx->Qry, sBioAlBlast::eBlastProteinOut);
                } else if( sIsExactly(algorithm, "tophat") ) {
                    alignx = new DnaAlignXTophat(*this, algo);
                    alignParser = new SAMParser(*this, alignx->Sub, alignx->Qry);
                } else if( sIsExactly(algorithm, "hisat2") ) {
                    alignx = new DnaAlignXHisat2(*this, algo);
                    alignParser = new SAMParser(*this, alignx->Sub, alignx->Qry);
                } else if( sIsExactly(algorithm, "bwa") ) {
                    alignx = new DnaAlignXBWA(*this, algo);
                    alignParser = new SAMParser(*this, alignx->Sub, alignx->Qry);
                } else if( sIsExactly(algorithm, "clustal") ) {
                    alignx = new DnaAlignXClustal(*this, algo);
                    alignParser = new MultipleAlParser(*this, alignx->Sub, alignx->Qry);
                } else if( sIsExactly(algorithm, "mafft") ) {
                    alignx = new DnaAlignXMafft(*this, algo);
                    alignParser = new MultipleAlParser(*this, alignx->Sub, alignx->Qry);
                } else if( sIsExactly(algorithm, "blat") ) {
                    alignx = new DnaAlignXBlat(*this, algo);
                    alignParser = new BLASTParser(*this, alignx->Sub, alignx->Qry, sBioAlBlast::eBlastStandardOut);
                }
            }
            if( alignx == 0 || alignParser == 0) {
                reqSetInfo(req, eQPInfoLevel_Error, "Unknown algorithm '%s'", algorithm ? algorithm : "unspecified");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            sUsrQueryEngine *ql = queryEngineFactory();
            if( !ql ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Failed to initialize query engine");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            alignx->readParams(pForm, user, objs);

            sStr buf,scriptSrc("%sdna-alignx-%s-%s.sh.os" SLIB_PLATFORM, cfgStr(&buf, 0, "qm.resourceRoot"), alignx->getAlgorithmName(), algo->propGet("version", &buf));
            sFil scriptFil(scriptSrc.ptr(), sMex::fReadonly);
            if( !scriptFil.ok() ) {
                logOut(eQPLogType_Error, "Failed to read script file '%s'", scriptSrc.ptr());
                reqSetInfo(req, eQPInfoLevel_Error, "Failed to initialize script");
                reqSetStatus(req, eQPReqStatus_ProgError);
            }
            alignx->scriptTemplate.add(scriptFil.ptr(), scriptFil.length());
            alignx->scriptTemplate.add0(2);
            scriptFil.destroy();

            logOut(eQPLogType_Debug, "Sequence Preparation and Indexing\n");
            progress100Start = 0;
            progress100End = 10;
            sStr subjectPath;
            subjectIdxLockId = &alignx->subjectIdxLockId;
            DnaAlignX::EIndexerMessage res = alignx->IndexSubject(subjectPath);
            subjectIdxLockId = 0;
            if( res == DnaAlignX::eIndexerMessage_Error ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Reference sequences could not be indexed");
                grpSetStatus(*this, grpId, eQPReqStatus_ProgError, svc.name);
                return 0;
            }
            if( res != DnaAlignX::eIndexerMessage_Done ) {
                return 0;
            }
            reqSetProgress(req, 1, 10);
            progress100Start = 10;
            progress100End = 14;

            sStr queryFiles;
            res = alignx->queryIndexChunk(queryFiles, reqSliceId, reqSliceCnt);
            if( res == DnaAlignX::eIndexerMessage_Error ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Query sequences are missing, corrupt or could not be indexed");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            sStr queryFiles_pair;
            res = alignx->queryIndexChunk(queryFiles_pair, reqSliceId, reqSliceCnt, true);
            if( res == DnaAlignX::eIndexerMessage_Error ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Paired query sequences are missing, corrupt or could not be indexed");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            reqSetProgress(req, 2, 11);
            progress100Start = 15;
            progress100End = 69;
            if( alignx->Align(queryFiles, queryFiles_pair) != DnaAlignX::eIndexerMessage_Done ) {
                return 0;
            }

            logOut(eQPLogType_Info, "Analyzing results...");
            progress100Start = 69;
            progress100End = 80;
            bool keepUnalignedReads = formBoolValue("keepUnalignedReads", false);
            sDic<idx> unalignedList;
            const idx keepAllMatches = formIValue("keepAllMatches", 2);
            const idx cntFound = alignx->ParseAlignment(keepAllMatches, keepUnalignedReads ? &unalignedList : 0, alignParser);

            sStr outUnalignedReads;
            if( keepUnalignedReads && !reqAddFile(outUnalignedReads, "req-unaligned.fa") ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Cannot save unaligned results");
                keepUnalignedReads = false;
            }
            if( keepUnalignedReads ) {
                sFil dst(outUnalignedReads);
                if( !dst.ok() ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Cannot save unaligned results");
                    logOut(eQPLogType_Error, "Failed to create file '%s'", outUnalignedReads.ptr());
                }
                const idx q = sString::cnt00(alignx->subjectFile00);
                for(idx i = 0; i < alignx->QryList.dim(); ++i) {
                    sHiveseq * Qry1 = &(alignx->QryList[i]);
                    for (idx j = 0; j < unalignedList.dim(); ++j){
                        idx cntAligned = *unalignedList.ptr(j);
                        if(cntAligned == q) {
                            idx * key = (idx *) (unalignedList.id(j));
                            Qry1->printFastXRow(&dst, alignx->qryInFastQ, *key, 0, 0, 0, true, false, 0, 0, sBioseq::eSeqForward, true, 0);
                        }
                    }
                }
            }
            if( cntFound == 0 ) {
                reqSetInfo(req, eQPInfoLevel_Warning, "Aligner produced empty result");

            } else if( cntFound < 0 ) {
                return 0;
            }

            if( isLastInMasterGroup() ) {
                alignx->updateSubjectProp();
                progress100Start = 81;
                progress100End = 95;
                if( !alignx->FinalProcessing(alignParser) ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Results collection failed");
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return 0;
                }
            }
            delete alignx;
            alignx = 0;
            delete ql;
            ql = 0;
            delete alignParser;
            alignParser = 0;
            reqSetProgress(req, cntFound, 100);
            reqSetStatus(req, eQPReqStatus_Done);
            return 0;
        }
};

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);

    DnaAlignXProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, 0, argv[0]));
    return (int) backend.run(argc, argv);
}
