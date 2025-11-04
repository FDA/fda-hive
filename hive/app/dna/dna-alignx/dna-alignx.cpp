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
    sDir::removeDir(getWorkDir());
#endif
}

void DnaAlignX::readParams(sVar * pForm, sUsr * user, sVec<sUsrProc> & objs, const char * svcname)
{
    algorithm.printf(0, "%s", &svcname[sizeof("svc-align-") - 1]);
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
    sStr buf;sStr tbuf;qp.replaceObjMacros(tbuf, "$(obj)/dna-alignx.sh");
    m_exe.vexe(tbuf.ptr(0)).arg(algorithm);


    const sUsrType2 * svctype = sUsrType2::get(svcname);
    if( objs.dim() && user ) {
        sVarSet list;
        sStr filter00;
        svctype->props(*user, list);
        const idx colName = list.colId("name");
        for(idx r = 1; r < list.rows; ++r) {
            filter00.printf("%s", list.val(r, colName));
            filter00.add0();
        }
        filter00.add0(2);
        list.empty();
        objs[0].propBulk(list, 0, filter00, false);
        const char * prev_prop = "";
        for(idx r = 1; r < list.rows; ++r) {
            const char * key = list.val(r, 1);
            if( strcasecmp(prev_prop, key) != 0 ) {
                m_commonArgs.varg("--%s", key);
                prev_prop = key;
            }
            m_commonArgs.arg(list.val(r, 3));
        }
    }
    for(idx ip = 0; pForm && ip < pForm->dim(); ++ip) {
        const char * key = (const char *) pForm->id(ip);
        if( svctype && user && !svctype->getField(*user, key)) {
            continue;
        }
        if( sIs("prop.", key) ) {
            continue;
        }
        if( sIs(sHtml::headerStartMarker, key) ) {
            break;
        }
        const char * val = pForm->value(key);
        m_commonArgs.varg("--%s", key).arg(val);
    }
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
            qp.logOut(qp.eQPLogType_Warning, "Subject sequence #%" DEC " length %" DEC " is > %s index chunk size %" DEC, iS, seqlen, algorithm.ptr(), indexChunkSize);
        }
        if( indexChunkSize && posInChunk > 0 && (dumpedSize + seqlen) > indexChunkSize ) {
            faFile.destroy();
        }
        if( !faFile.ok() ) {
            const char * subjectFile = subjectFile00.printf("%s%s%" DEC ".fa", subjectPath, algorithm.ptr(), chunkCnt);
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
    sStr suffix("%s", algorithm.ptr());
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
        std::auto_ptr<sUsrObj> obj(qp.user->objFactory(sHiveId(id)));
        if( !obj.get() ) {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Object %s not accessible", id);
            return eIndexerMessage_Error;
        }
        obj->propGet("taxonomy", &taxonomy);
        taxonomy.add0(1);
        if( id == tokenizedSubList.ptr() ) {
            obj->getFilePathname(subjectIndexDir, "%s", suffix.ptr());
            if( !subjectIndexDir ) {
                sUsr qpride("queen", true);
                std::auto_ptr<sUsrObj> tobj(qpride.objFactory(obj->Id()));
                if( tobj.get() ) {
                    tobj->addFilePathname(subjectIndexDir, true, "%s", suffix.ptr());
                }
            }
        } else {
            subjectIndexDir.cut0cut();
        }
    }
    taxonomy.add0(2);
    if( !subjectIndexDir || !subjectIndexPersistent() ) {
        sStr prefix("%s-sub-", qp.svc.name), subIdListConcated;
        sHiveId::printVec(subIdListConcated, subIdList, "_", false);
        if( subIdListConcated.length() + prefix.length() + suffix.length() > 250 ) {
            sMD5 sum(subIdListConcated, subIdListConcated.length());
            subIdListConcated.printf(0, "%s_%s_%s", subIdList[0].print(), subIdList[subIdList.dim() - 1].print(), sum.sum);
        }
        subjectIndexDir.cut0cut();
        qp.cfgStr(&subjectIndexDir, 0, "dna-alignx.indexDirectory");
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
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "%s with %s algorithm", err ? err.ptr() : "Selected reference(s) cannot be used", algorithm.ptr());
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
        for(const char * p = subjectFile00; p; p = sString::next00(p)) {
            sPipe2::CmdLine cmdline;
            cmdline = m_exe;
            cmdline.arg("build");
            cmdline.copyArgs(m_commonArgs);
            cmdline.arg("--indexPath").arg(p);
            cmdline.arg("--referenceFile").arg(p);
            if( referenceAnnotationFile ) {
                cmdline.arg("--annotationFile").arg(referenceAnnotationFile);
            }
            qp.logOut(qp.eQPLogType_Debug, "INDEXING: %s", cmdline.printBash());
            sIO log;
            if( qp.exec2(cmdline, progressWatchFilename(subjectIndexDir), &log) ) {
                qp.logOut(qp.eQPLogType_Error, "failed to index file '%s'", p);
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
        sStr queryFile(separateHiveseqs ? "%s%s-%" DEC ".fa" : "%s%s.fa", getWorkDir(true), get_pair ? "-pair" : "", iq + 1);
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
        qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "%s with %s algorithm", err ? err.ptr() : "Selected read(s) cannot be used", algorithm.ptr());
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
            for(iq = 0; iq < Qry1->dim() && Qry1->len(iq) < 1000; ++iq) {
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
    for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
        if( !qp.reqProgress(cnt, cnt, q) ) {
            return eIndexerMessage_Aborted;
        }
        sPipe2::CmdLine cmdline;
        cmdline = m_exe;
        cmdline.arg("align");
        cmdline.copyArgs(m_commonArgs);
        cmdline.arg("--outPath").varg("%s%" DEC "", getWorkDir(true), cnt);
        cmdline.arg("--indexPath").arg(p);
        if( referenceAnnotationFile ) {
            cmdline.arg("--annotationFile").arg(referenceAnnotationFile);
        }
        cmdline.arg("--queryFiles").arg(query);
        if( query_pair ) {
            cmdline.arg("--paired_queryFiles").arg(query_pair);
        }
        qp.logOut(qp.eQPLogType_Debug, "ALIGNING: %s", cmdline.printBash());
        sIO log;
        if( qp.exec2(cmdline, progressWatchFilename(getWorkDir()), &log) ) {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Warning, "Aligner produced an error");
            qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
            return eIndexerMessage_Error;
        }
    }
    return eIndexerMessage_Done;
}

idx DnaAlignX::ParseAlignment(const idx keepAllMatches, sDic<idx> * unalignedList)
{

    sVec<idx> alignmentMap;
    idx cnt = 0, cntFoundAll = 0;
    for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
        sStr fn("%s%" DEC ".%s", getWorkDir(true), cnt, resultExtension());
        if( sFile::size(fn) ) {
            sFil fl(fn, sMex::fReadonly);
            if( !fl.ok() ) {
                qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Failed to access slice result");
                qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
                return -1;
            }
            const idx cntFound = fillAlignmentMap(fl, alignmentMap, unalignedList);
            qp.logOut(qp.eQPLogType_Debug, "Found %" DEC " aligner results in file '%s' for %s", cntFound, fn.ptr(), algorithm.ptr());
            cntFoundAll += cntFound;
        }
    }
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
    if( alignmentMap.dim() ) {
        sStr pathT;
        qp.reqSetData(qp.reqId, "file://alignment-slice.vioalt", 0, 0);
        qp.reqDataPath(qp.reqId, "alignment-slice.vioalt", &pathT);
        sFil ff(pathT);
        if( ff.ok() ) {
            cntFoundAll = sBioseqAlignment::filterChosenAlignments(&alignmentMap, 0, flagSet, &ff);
        } else {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Failed to save slice result");
            qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
            return -2;
        }
    }
    qp.logOut(qp.eQPLogType_Debug, "Parsed %" DEC " alignments\n", alignmentMap.dim());
    return cntFoundAll;
}

idx DnaAlignX::FinalProcessing()
{
    sStr srcAlignmentsT;
    if( qp.reqSliceCnt == 1 ) {
        qp.reqDataPath(qp.reqId, "alignment-slice.vioalt", &srcAlignmentsT);
    } else {
        qp.grpDataPaths(qp.grpId, "alignment-slice.vioalt", &srcAlignmentsT, qp.vars.value("serviceName"));
    }
    const char * resultFileTemplate = qp.formValue("resultFileTemplate", 0);
    if( !resultFileTemplate ) {
        resultFileTemplate = "";
    }
    sStr resultFileName("%salignment.hiveal", resultFileTemplate);
    sStr dstAlignmentsT;
    if(!qp.reqAddFile(dstAlignmentsT, resultFileName.ptr())){
        qp.logOut(qp.eQPLogType_Error, "Failed to write results to the destination object (%s)",qp.objs[0].IdStr());
        return 0;
    }

    sVioal vioAltAAA(0, &Sub, &Qry);
    vioAltAAA.myCallbackFunction = sQPrideProc::reqProgressStatic;
    vioAltAAA.myCallbackParam = &qp;
    sDic<sBioal::LenHistogram> lenHistogram;
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

    if(!vioAltAAA.DigestCombineAlignmentsRaw(dstAlignmentsT, srcAlignmentsT, params, &lenHistogram)) {
        qp.logOut(qp.eQPLogType_Error, "Failed to combine the results");
    }

    if( lenHistogram.dim() ) {
        resultFileName.printf(0, "%shistogram.csv", resultFileTemplate);
        dstAlignmentsT.cut(0);
        qp.reqAddFile(dstAlignmentsT, "%s", resultFileName.ptr());
        sFil hist(dstAlignmentsT);
        if( hist.ok() ) {
            sBioal::printAlignmentHistogram(&hist, &lenHistogram);
        }
    }
    sStr finalPaths;
    if( getPathsForFinalProcessing(finalPaths) && finalPaths ) {
        sStr finalDst;
        if( finalPaths.length() > (3 * 1024) ) {
            sStr listName("%s%s-finalize.lst", getWorkDir(), qp.svc.name);
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
        qp.reqAddFile(finalDst, "%s", algorithm.ptr());
        sPipe2::CmdLine cmdline;
        cmdline = m_exe;
        cmdline.arg("finalize");
        cmdline.copyArgs(m_commonArgs);
        cmdline.arg("--outPath").arg(getWorkDir());
        cmdline.arg("--resultPath").arg(finalDst);
        if( referenceAnnotationFile ) {
            cmdline.arg("--annotationFile").arg(referenceAnnotationFile);
        }
        cmdline.arg("--finalFiles").arg(finalPaths);
        qp.logOut(qp.eQPLogType_Debug, "FINALIZING: %s", cmdline.printBash());
        sIO log;
        if( qp.exec2(cmdline, progressWatchFilename(getWorkDir()), &log) ) {
            return 0;
        }

    }
    return 1;
}

class DnaAlignXProc: public sHiveProc
{
        idx * subjectIdxLockId;
    public:

        DnaAlignXProc(const char * defline00, const char * srv)
            : sHiveProc(defline00, srv), subjectIdxLockId(0)
        {
        }
        ~DnaAlignXProc()
        {
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
            std::auto_ptr<DnaAlignX> alignx;
            sStr algorithm;
            formValue("alignSelector", &algorithm);
            if( sIs(algorithm, "svc-align-bowtie") ) {
                alignx.reset(new DnaAlignXBowtie(*this));
            } else if( sIs(algorithm, "svc-align-bowtie2") ) {
                alignx.reset(new DnaAlignXBowtie2(*this));
            } else if( sIs(algorithm, "svc-align-blast") ) {
                alignx.reset(new DnaAlignXBlast(*this));
            } else if( sIs(algorithm, "svc-align-blastx") ) {
                alignx.reset(new DnaAlignXBlastX(*this));
            } else if( sIs(algorithm, "svc-align-tblastx") ) {
                alignx.reset(new DnaAlignXTBlastX(*this));
            } else if( sIs(algorithm, "svc-align-tophat") ) {
                alignx.reset(new DnaAlignXTophat(*this));
            } else if( sIs(algorithm, "svc-align-hisat2") ) {
                alignx.reset(new DnaAlignXHisat2(*this));
            } else if( sIs(algorithm, "svc-align-bwa") ) {
                alignx.reset(new DnaAlignXBWA(*this));
            } else if( sIs(algorithm, "svc-align-clustal") ) {
                alignx.reset(new DnaAlignXClustal(*this));
            } else if( sIs(algorithm, "svc-align-mafft") ) {
                alignx.reset(new DnaAlignXMafft(*this));
            } else if( sIs(algorithm, "svc-align-blat") ) {
                alignx.reset(new DnaAlignXBlat(*this));
            } else {
                alignx.reset(0);
                reqSetInfo(req, eQPInfoLevel_Error, "Unknown algorithm '%s'", algorithm ? algorithm.ptr() : "unspecified");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            alignx->readParams(pForm, user, objs, algorithm);

            const char * const wd = alignx->getWorkDir();
            if( !wd ) {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Missing configuration");
                logOut(eQPLogType_Error, "Could not create working directory");
                reqSetStatus(reqId, eQPReqStatus_ProgError);
                return 0;
            }
            sDir::removeDir(wd);
            if( !sDir::makeDir(wd) ) {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Cannot establish work directory");
                reqSetStatus(reqId, eQPReqStatus_ProgError);
                return 0;
            }

            logOut(eQPLogType_Debug, "Sequence Preparation and Indexing\n");

            progress100Start = 0;
            progress100End = 10;

            sUsrObj SS(*user,sHiveId(formValue("subject")));
            sStr ff;SS.getFilePathname(ff,"%s","preindex");
            DnaAlignX::EIndexerMessage res;
            if(ff.length()) {
                sFil fl(ff.ptr());if(fl.length()){
                    alignx->subjectFile00.printf("%.*s%.*s",(int)ff.length()-8,ff.ptr(),(int)fl.length(),fl.ptr(0));
                    alignx->subjectFile00.add0(2);
                    res=DnaAlignX::eIndexerMessage_Done;
                }
            }
            else {
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
            const idx cntFound = alignx->ParseAlignment(keepAllMatches, keepUnalignedReads ? &unalignedList : 0);

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

            if( isLastInGroup() ){
                alignx->updateSubjectProp();
            }
            if( isLastInMasterGroup() ) {
                progress100Start = 81;
                progress100End = 95;
                if( !alignx->FinalProcessing() ) {
                    reqSetInfo(req, eQPInfoLevel_Error, "Results collection failed");
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return 0;
                }
            }
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

    DnaAlignXProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-alignx", argv[0]));
    return (int) backend.run(argc, argv);
}

