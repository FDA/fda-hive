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
#include <ulib/uquery.hpp>
#include <violin/violin.hpp>
#include <ssci/math/rand/rand.hpp>
#include <xlib/md5.hpp>

#include "dna-alignx.hpp"

DnaAlignX::~DnaAlignX()
{
#if !_DEBUG
    sDir::removeDir(getWorkDir());
#endif
}

void DnaAlignX::readParams(sVar * pForm, sUsr * user, sVec<sUsrProc> & objs)
{
    keepRefNs = qp.formBoolValue("keepRefNs", keepRefNs);
    minMatchLength = qp.formIValue("minMatchLen", minMatchLength);
    maxMissQueryPercent = qp.formIValue("maxMissQueryPercent", maxMissQueryPercent);
    // if we need to keep multiple elements of query hiveseq input separated
    separateHiveseqs = qp.formBoolValue("splitSingle", separateHiveseqs);
    scoreFilter = qp.formIValue("scoreFilter", scoreFilter);
    seedSize = qp.formIValue("seedSize", seedSize);
    evalueFilter = qp.formRValue("evlaueFilter", 0, evalueFilter);
    keepOriginalQryId = qp.formBoolValue("keepOriginalQryId", keepOriginalQryId);
    subbiomode = sBioseq::isBioModeLong( qp.formIValue("subbiomode", subbiomode) ) ? sBioseq::eBioModeLong : sBioseq::eBioModeShort;
    qrybiomode = sBioseq::isBioModeLong( qp.formIValue("qrybiomode", qrybiomode) ) ? sBioseq::eBioModeLong : sBioseq::eBioModeShort;

    qp.formValue("cmdLine", &additionalCommandLineParameters);
    sHiveId gtfFileId(qp.formValue("GTFfile"));
    sUsrFile uf(gtfFileId, qp.user); // for single references we also turn this into object
    if( uf.Id() ) {
        uf.getFile(referenceAnnotationFile);
    }
    if( objs.dim() && user ) {
        sUsrInternalQueryEngine engine(*user);
        sStr formula("argstring((obj)\"%s\", {assign: \" \"})", objs[0].Id().print());
        if( engine.parse(formula, 0, 0) ) {
            sVariant result;
            if( engine.eval(result, 0) ) {
                additionalArguments.printf("%s", result.asString());
            }
        }
    }
    for(idx ip = 0; pForm && ip < pForm->dim(); ++ip) {
        const char * key = (const char *) pForm->id(ip);
        if( sIs("prop.", key) ) {
            continue;
        }
        if( sIs(sHtml::headerStartMarker, key) ) {
            break;
        }
        const char * val = pForm->value(key);
        additionalArguments.printf(" --%s '%s' ", key, val);
    }
}

//! Return internal pointer to a path for monitoring during background execution
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

DnaAlignX::EIndexerMessage DnaAlignX::subjectDump(const char * subjectPath, idx start /* = 0 */, idx cnt /* = 0 */)
{
    subjectGet(); //(question by Dinos) why do we get the subject again? it is done already, outside of the function.
    if( !Sub.dim() ) {
        qp.logOut(qp.eQPLogType_Error, "Reference sequences are not specified or corrupted");
        return eIndexerMessage_Error;
    }
    //
    // prepare reference sequences
    //
    const idx indexChunkSize = subjectGetChunkSize();
    idx totalDim = cnt ? cnt : (Sub.dim() - start), dumpedSize = 0, posInChunk = 0;
    totalDim += start;
    sFil faFile;
    bool doDump = true;
    for(idx chunkCnt = 0, iS = start; iS < totalDim; ++iS) {
        idx seqlen = Sub.len(iS);

        if( indexChunkSize && seqlen > indexChunkSize ) {
            qp.logOut(qp.eQPLogType_Warning, "Subject sequence #%"DEC" length %"DEC" is > %s index chunk size %"DEC, iS, seqlen, algorithm.ptr(), indexChunkSize);
        }
        if( indexChunkSize && posInChunk > 0 && (dumpedSize + seqlen) > indexChunkSize ) {
            //qp.logOut(qp.eQPLogType_Trace, "NEW FILE %"DEC":  posInChunk %"DEC" dumpedSize %"DEC" seqlen %"DEC, chunkCnt + 1, posInChunk, dumpedSize, seqlen);
            faFile.destroy();
        }
        // make sure there is a file to write into
        if( !faFile.ok() ) {
            const char * subjectFile = subjectFile00.printf("%s%s%"DEC".fa", subjectPath, algorithm.ptr(), chunkCnt);
            subjectFile00.add0();
            faFile.init(subjectFile);
            if( !faFile.ok() ) {
                qp.logOut(qp.eQPLogType_Error, "Failed to create file '%s'", subjectFile);
                return eIndexerMessage_Error;
            }
#if _DEBUG
            // in debug not to wait for fasta dump, in release always dump
            doDump = faFile.length() ? false : true;
#else
            faFile.cut(0); // always overwrite
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
                id += id[0] == '>' ? 1 : 0; //skip leading '>' from old hiveseqs
                faFile.printf(">%s\n", id);
                const char * idEnd = strchr(id, ' ');
                idx idLen = idEnd ? idEnd - id : sLen(id);
                idx * pid = idMap.setString(id, idLen);
                if( !pid ) {
                    qp.logOut(qp.eQPLogType_Error, "Out of memory for id mapping: %"DEC" -> [%"DEC"]'%s'", iS, idLen, id);
                    return eIndexerMessage_Error;
                }
                *pid = iS;
            } else {
                faFile.printf(">%"DEC"\n", iS);
            }
            char * const buf = faFile.add(0, seqlenX + 1);
            sBioseq::uncompressATGC(buf, seqs, 0, seqlen, seq_line_max);
            if( keepRefNs ) {
                const char * qua = Sub.qua(iS, 0);
                if( qua ) {
                    //! check the quality of FASTQ sequence, it will return zero if given is FASTA
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
    // push all in group into future, except indexer (indexer might be not in my group)
    qp.grpReSubmit(qp.grpId, qp.vars.value("serviceName"), 60, lockReqId);
}

static void grpSetStatus(sQPrideProc & qp, const idx grpid, const idx status)
{
    sVec <idx> reqs;
    qp.grp2Req(grpid, &reqs);
    qp.reqSetStatus(&reqs, status);
}

DnaAlignX::EIndexerMessage DnaAlignX::IndexSubject(sStr & subjectIndexDir)
{
    // index directory suffix
    sStr suffix("%s", algorithm.ptr());
    const idx indexChunkSize = subjectGetChunkSize();
    if( indexChunkSize > 0 ) { // append chunk size if defined
        if( indexChunkSize < (1 * 1024 * 1024L) ) {
            // can't be less than that because of the suffix text below
            qp.logOut(qp.eQPLogType_Error, "subject chunk size too small: '%"DEC, indexChunkSize);
            return eIndexerMessage_Error;
        }
        suffix.printf("-%"DEC"M", indexChunkSize / 1024 / 1024);
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
        if( id == tokenizedSubList.ptr() ) { // use first subject location
            obj->getFilePathname(subjectIndexDir, "%s", suffix.ptr());
            if( !subjectIndexDir ) {
                sUsr qpride("qpride", true); // have to create this via sudo
                std::auto_ptr<sUsrObj> tobj(qpride.objFactory(obj->Id()));
                if( tobj.get() ) {
                    tobj->addFilePathname(subjectIndexDir, true, "%s", suffix.ptr());
                }
            }
        } else { // move to common area if more than 1 subject
            subjectIndexDir.cut0cut();
        }
    }
    taxonomy.add0(2);
    // multi-subject indexing destination is separate for each algorithm to avoid storing unnecessary indexes
    if( !subjectIndexDir || !subjectIndexPersistent() ) {
        sStr prefix("%s-sub-", qp.svc.name), subIdListConcated;
        sHiveId::printVec(subIdListConcated, subIdList, "_", false);
        if( subIdListConcated.length() + prefix.length() + suffix.length() > 250 ) {
            // directory name limit minus some
            sMD5 sum(subIdListConcated, subIdListConcated.length());
            subIdListConcated.printf(0, "%s_%s_%s", subIdList[0].print(), subIdList[subIdList.dim() - 1].print(), sum.sum);
        }
        subjectIndexDir.cut0cut();
        qp.cfgStr(&subjectIndexDir, 0, "dna-alignx.indexDirectory");
        suffix.printf("-%s", subIdListConcated.ptr());
        subjectIndexDir.printf("%s%s", prefix.ptr(), suffix.ptr());
    }
    subjectIndexDir.printf("/");

    typedef sDic<idx> TAlgoIdx; // filename -> size

    // check index exist and valid
    sStr triggerFinished("%sidx.finished", subjectIndexDir.ptr());
    sStr subjectListFile("%ssubjects.00", subjectIndexDir.ptr());
    bool doIndex = true;
    if( sFile::exists(triggerFinished) ) {
        // validate index content
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
                    qp.logOut(qp.eQPLogType_Debug, "Miss file '%s' %"DEC" vs %"DEC" in index file list", buf.ptr(), sz, *tbl.ptr(i));
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
        // validate that these references can be used for this algorithm
        // not exactly effective if for an example two NTs are chosen
        if( !subjectVerify(taxonomy, &err) ) {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "%s with %s algorithm", err ? err.ptr() : "Selected reference(s) cannot be used", algorithm.ptr());
            return eIndexerMessage_Error;
        }
        sStr subjectWriteLock("%sindex.lock", subjectIndexDir.ptr());
        idx lreq; // lock indexing directory
        if( !qp.reqLock(subjectWriteLock, &lreq) ) {
            qp.logOut(qp.eQPLogType_Debug, "Waiting for req %"DEC" to finish indexing '%s'", lreq, subjectIndexDir.ptr());
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
        qp.logOut(qp.eQPLogType_Info, "Started indexing in '%s', lock %"DEC, subjectIndexDir.ptr(), lreq);
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
        // save ids map if needed
        if( idMapFile ) {
            sFil fidMap(idMapFile);
            if( !fidMap.ok() ) {
                qp.logOut(qp.eQPLogType_Error, "Cannot create id mapping file '%s'", idMapFile.ptr());
                return eIndexerMessage_Error;
            }
            idMap.serialOut(fidMap);
        }
        for(const char * p = subjectFile00; p; p = sString::next00(p)) {
            sStr cmdLine("\"%sdna-alignx.sh.os%s\" %s build %s --indexPath \"%s\" --referenceFile \"%s\" --annotationFile \"%s\"", resourceRoot.ptr(), SLIB_PLATFORM, algorithm.ptr(), (additionalArguments ? additionalArguments.ptr() : " "), p, p,
                referenceAnnotationFile ? referenceAnnotationFile.ptr() : "n/a");
            qp.logOut(qp.eQPLogType_Info, "INDEXING: %s\n", cmdLine.ptr());
            sIO log;
            const idx execret = qp.exec(cmdLine, 0, progressWatchFilename(subjectIndexDir), &log);
            qp.logOut(execret ? qp.eQPLogType_Error : qp.eQPLogType_Debug, "retcode %"DEC" %s", execret, log ? log.ptr() : "");
            if( execret != 0 ) {
                qp.logOut(qp.eQPLogType_Error, "failed to index file '%s'", p);
                return eIndexerMessage_Error;
            }
        }
        {{
            sDir ex;
            ex.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive), subjectIndexDir, "*");
            TAlgoIdx tbl;
            for(const char * f = ex; f; f = sString::next00(f)) {
                idx * sz = tbl.setString(&f[subjectIndexDir.length()]); // relative path
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
            // save subject file names *only* w/o indexDir
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
    // extend with subjectIndexDir
    for(const char * p = slf; p; p = sString::next00(p)) {
        if( p[0] == '/' || p[0] == '\\' ) {
            // adjust for old bug with storing full paths to subject chunks
            // find index dir name and remove it with everything preceeding it:
            // [/hive/qnest1/vol1/qnest2/vol2/store1/366/106/106366/blast-N-32768/]blast0.fa
            p = strstr(p, suffix);
            p += suffix.length() + 1;
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
    qp.logOut(qp.eQPLogType_Debug, "Using index id maps %"DEC" in '%s'", idMap.dim(), subjectIndexDir.ptr());
    return eIndexerMessage_Done;
}

DnaAlignX::EIndexerMessage DnaAlignX::queryIndexChunk(sStr & queryFiles, idx qStart, idx qEnd)
{
    sStr qqStr;
    queryGet(&qqStr);
    const char * qry;
    idx iq;
    EIndexerMessage res = eIndexerMessage_Error;

    QryList.empty();
    sStr tokenizedQueryList; //  if requested to maintain in multiple files queries will be tokenized, otherwise in one buffer
    sString::searchAndReplaceSymbols(&tokenizedQueryList, qqStr.ptr(), qqStr.length(), separateHiveseqs ? "\n;" : "\xFF", 0, 0, true, true, false, true);
    for(iq = 0, qry = tokenizedQueryList.ptr(0); qry; qry = sString::next00(qry), ++iq) {
        sStr queryFile(separateHiveseqs ? "%s-%"DEC".fa" : "%s.fa", getWorkDir(true), iq + 1);
        sHiveseq * hs = QryList.set(queryFile.ptr());
        new (hs) sHiveseq(qp.user, qry, qrybiomode);
        if( !hs->dim() ) {
            qp.logOut(qp.eQPLogType_Error, "query is empty: '%s' in mode %i", qry, qrybiomode);
            return eIndexerMessage_Error;
        }
        if( !separateHiveseqs ) {
            break; // just in case if the name of the id has \xff inside
        }
    }

    // validate that these reads can be used for this algorithm
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
    // prepare and index query files
    for(idx i = 0; i < QryList.dim(); ++i) {
        sHiveseq * Qry1 = &(QryList[i]);
        sStr qryFile("%s", (const char *) QryList.id(i));

        if( qEnd > Qry1->dim() || qEnd <= 0) {
            qEnd = Qry1->dim();
        }

        // for genomes' screening, produce random reads from genomes
        if( frmProduceRandomReadsForNT ) {
            for(iq = 0; iq < Qry1->dim() && Qry1->len(iq) < 1000; ++iq) {
                // scan until found a long sequence
            }
            if( iq >= Qry1->dim() ) {
                // found any ?
                frmProduceRandomReadsForNT = 0;
            }
        }

        if( frmProduceRandomReadsForNT ) {
            sFilterseq::randomizer(*Qry1, qryFile, maxNumberQuery);
            res = sFile::size(qryFile) ? DnaAlignX::eIndexerMessage_Done : DnaAlignX::eIndexerMessage_Error;
        } else { //produce query files
            sFile::remove(qryFile.ptr());
            sFil fileDst(qryFile.ptr());
            if( maxNumberQuery < qEnd - qStart ) {
                // Print maxNumberQuery elements from query
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
                        // Returns false if it didn't pass the complexity test
                        if( isValid == 0 ) {
                            isValid = Qry1->printFastXRow(&fileDst, qryInFastQ, randomQuery, 0, 0, qryCumulativeDim, keepOriginalQryId, false, 0, 0, 0, keepQryNs, filterNsPercent ? 50 : 0);
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
                // Print the whole file
                Qry1->printFastX(&fileDst, qryInFastQ, qStart, qEnd, qryCumulativeDim, keepOriginalQryId, false, 0, 0, keepQryNs, filterNsPercent ? 50 : 0);
                res = eIndexerMessage_Done;
            }
            qryCumulativeDim += Qry1->dim();
        }
        queryFiles.printf("\"%s\" ", qryFile.ptr());
        if( res != DnaAlignX::eIndexerMessage_Done ) {
            break;
        }
    }
    Qry.setmode(qrybiomode);
    for(idx i = 0; i < QryList.dim(); ++i) {
        // Attaching a new bioseq to Hiveseq has to be 1 based
        Qry.attach((sBioseq*) &(QryList[i]), 1, QryList[i].dim());
    }
    return res;
}

DnaAlignX::EIndexerMessage DnaAlignX::Align(const char * query)
{
    const idx q = sString::cnt00(subjectFile00);
    idx cnt = 0;
    for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
        if( !qp.reqProgress(cnt, cnt, q) ) {
            return eIndexerMessage_Aborted;
        }
        sStr cmdLine("\"%sdna-alignx.sh.os%s\" %s align %s --outPath \"%s%"DEC"\" --indexPath \"%s\" --annotationFile \"%s\" --queryFiles %s", resourceRoot.ptr(0), SLIB_PLATFORM, algorithm.ptr(),
            (additionalArguments ? additionalArguments.ptr() : " "), getWorkDir(true), cnt, p, referenceAnnotationFile ? referenceAnnotationFile.ptr() : "n/a", query);
        qp.logOut(qp.eQPLogType_Info, "ALIGNMENT %s: \n%s\n\n", algorithm.ptr(), cmdLine.ptr());
        sIO log;
        const idx execret = qp.exec(cmdLine, 0, progressWatchFilename(getWorkDir()), &log, 1);
        qp.logOut(execret ? qp.eQPLogType_Error : qp.eQPLogType_Debug, "retcode %"DEC" %s", execret, log.ptr());
        if( execret ) {
            qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Warning, "Aligner produced an error");
            qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
            return eIndexerMessage_Error;
        }
    }
    return eIndexerMessage_Done;
}

idx DnaAlignX::ParseAlignment(const idx keepAllMatches, sDic<idx> * unalignedList)
{

    // Load query sequences into one Hiveseq even if separatehiveseqs=1
    // This is required for parsing the results
    sVec<idx> alignmentMap;
    idx cnt = 0, cntFoundAll = 0;
    for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
        sStr fn("%s%"DEC".%s", getWorkDir(true), cnt, resultExtension());
        if( sFile::size(fn) ) {
            sFil fl(fn, sMex::fReadonly);
            if( !fl.ok() ) {
                qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Failed to access slice result");
                qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
                return -1;
            }
            const idx cntFound = fillAlignmentMap(fl, alignmentMap, unalignedList);
            qp.logOut(qp.eQPLogType_Debug, "Found %"DEC" aligner results in file '%s' for %s", cntFound, fn.ptr(), algorithm.ptr());
            cntFoundAll += cntFound;
        }
    }
    flagSet = sBioseqAlignment::fAlignForward;
    if( frmProduceRandomReadsForNT ) {
        flagSet |= sBioseqAlignment::fAlignKeepAllBestMatches;
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
        qp.reqSetData(qp.reqId, "file://alignment-slice.vioalt", 0, 0); // create and empty file for this request
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
    qp.logOut(qp.eQPLogType_Debug, "Parsed %"DEC" alignments\n", alignmentMap.dim());
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
            // pass file list in a file and pass that file name as @list.txt or command line might be too long
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
        qp.reqAddFile(finalDst, "%s", algorithm.ptr()); // path to obj dir with algorithm suffix: /store12/123255/blast
        sStr cmdLine("\"%sdna-alignx.sh.os%s\" %s finalize %s --outPath \"%s\" --resultPath \"%s\" --annotationFile \"%s\" --finalFiles %s",
            resourceRoot.ptr(), SLIB_PLATFORM, algorithm.ptr(), (additionalArguments ? additionalArguments.ptr() : " "),
            getWorkDir(), finalDst.ptr(), referenceAnnotationFile ? referenceAnnotationFile.ptr() : "n/a", finalPaths.ptr());
        qp.logOut(qp.eQPLogType_Debug, "FINALIZE %s: \n%s\n\n", algorithm.ptr(), cmdLine.ptr());
        sIO log;
        const idx execret = qp.exec(cmdLine, 0, progressWatchFilename(getWorkDir()), &log, 0);
        qp.logOut(execret ? qp.eQPLogType_Error : qp.eQPLogType_Debug, "retcode %"DEC" %s", execret, log ? log.ptr() : "");
        if( execret ) {
            return 0;
        }
    }
    return 1;
}

class DnaAlignXProc: public sQPrideProc
{
        idx * subjectIdxLockId;
    public:

        DnaAlignXProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv), subjectIdxLockId(0)
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
            std::auto_ptr<DnaAlignX> alignx; // provides workDir cleanup upon destruction!
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
            alignx->algorithm.printf(0, "%s", algorithm.ptr(sizeof("svc-align-") - 1));
            cfgStr(&alignx->resourceRoot, 0, "qm.resourceRoot");
            const idx keepAllMatches = formIValue("keepAllMatches", 2);
            alignx->readParams(pForm, user, objs);

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

            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
            // _/
            // _/ load the subject sequences
            // _/
            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
            logOut(eQPLogType_Debug, "Sequence Preparation and Indexing\n");

            progress100Start = 0;
            progress100End = 10;
            // perform subject indexing
            sStr subjectPath;
            subjectIdxLockId = &alignx->subjectIdxLockId;
            DnaAlignX::EIndexerMessage res = alignx->IndexSubject(subjectPath);
            subjectIdxLockId = 0; // forget the lock to avoid resubmitting OnProgress
            if( res == DnaAlignX::eIndexerMessage_Error ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Reference sequences could not be indexed");
                // killall or they will wake up and try to index forever
                grpSetStatus(*this, grpId, eQPReqStatus_ProgError);
                return 0;
            }
            if( res != DnaAlignX::eIndexerMessage_Done ) { // still waiting it to be done or it was aborted
                return 0;
            }
            reqSetProgress(req, 1, 10);
            progress100Start = 10;
            progress100End = 11;

            idx slice = formIValue("slice", sHiveseq::defaultSliceSizeI);
            if( slice < 0 ) {
                slice = sHiveseq::defaultSliceSizeI;
            }
            idx qStart = slice * reqSliceId;
            idx qEnd = qStart + slice;

            sStr queryFiles;
            res = alignx->queryIndexChunk(queryFiles, qStart, qEnd);
            if( res == DnaAlignX::eIndexerMessage_Error ) {
                reqSetInfo(req, eQPInfoLevel_Error, "Query sequences are missing, corrupt or could not be indexed");
                reqSetStatus(req, eQPReqStatus_ProgError);
                return 0;
            }
            reqSetProgress(req, 2, 11);
            progress100Start = 11;
            progress100End = 69;
            logOut(eQPLogType_Debug, "Aligning...");
            if( alignx->Align(queryFiles) != DnaAlignX::eIndexerMessage_Done ) {
                return 0;
            }
            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
            // _/
            // _/ Parse the results
            // _/
            // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
            logOut(eQPLogType_Info, "Analyzing results...");
            progress100Start = 69;
            progress100End = 80;
            bool keepUnalignedReads = formBoolValue("keepUnalignedReads", false);
            sDic<idx> unalignedList;
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
                // Dump only the ones in the list
                const idx q = sString::cnt00(alignx->subjectFile00);
                for(idx i = 0; i < alignx->QryList.dim(); ++i) {
                    sHiveseq * Qry1 = &(alignx->QryList[i]);
                    for (idx j = 0; j < unalignedList.dim(); ++j){
                        idx cntAligned = *unalignedList.ptr(j);
                        if(cntAligned == q) {
                            idx * key = (idx *) (unalignedList.id(j));
                            Qry1->printFastXRow(&dst, alignx->qryInFastQ, *key, 0, 0, 0, true, false, 0, 0, 0, true, 0);
                        }
                    }
                }
            }
            if( cntFound == 0 ) {
                reqSetInfo(req, eQPInfoLevel_Warning, "Aligner produced empty result");

            } else if( cntFound < 0 ) {
                return 0;
            }

            //Regardless the master group all requests use the same subject and only one needs to update
            //the subject field in case it is reorded. This is done as last in a group.
            //Last in a group is another way of say last request of this svc for this object
            if( isLastInGroup() ){
                alignx->updateSubjectProp();
            }
            //We always collect and digest the results of each master group.
            //Dna-screening is launching multiple alignxs under one group but each one in each own master group
            //FinalProcessing here is acting as a collector step which should performed per master group
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
    sApp::args(argc, argv); // remember arguments in global for future

    DnaAlignXProc backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "dna-alignx", argv[0]));
    return (int) backend.run(argc, argv);
}

