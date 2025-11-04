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
#include <violin/alignparse.hpp>

const char * VIOAL_NAME = "file://alignment-slice.vioalt";
const char * HIVEAL_NAME = "file://alignment.hiveal";
const char * SAM_HEADER_NAME = "file://samheader";
const char * SAM_HEADER_SLICE_NAME = "file://samheader-slice";
const char * LOSSLESS_SAM_INFO_NAME = "file://samcontent";
const char * LOSSLESS_SAM_FOOTER_NAME = "file://samfooter";
const char * HIST_FILE_NAME = "file://histogram.csv";
const char * COV_DICT_NAME = "file://coverage_dict";

const idx MIN_SUBJECTS_FOR_MMAP = 1000000;

using namespace sviolin;

void sviolin::SAMHeaders(sUsrObj &obj, sFil & samHeader, sFil & samFooter) {
    sStr samNameBuf;
    samHeader.destroy();
    samFooter.destroy();
    if (obj.getFilePathname(samNameBuf, "samheader")) {
        samHeader.init(samNameBuf, sMex::fReadonly);
    }
    samNameBuf.cut(0);
    if (obj.getFilePathname(samNameBuf, "samfooter")) {
        samFooter.init(samNameBuf, sMex::fReadonly);
    }
}

sRC AlignParser::filterAndWriteAlignMap(AlignMap & alignMap, idx queryStart, idx flagSet) const
{
    if( alignMap.dim() ) {
        sStr pathBuf;
        if (!_qp.reqSetData(_qp.reqId, VIOAL_NAME, 0, 0)) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
        if (!_qp.reqDataPath(_qp.reqId, &VIOAL_NAME[7], &pathBuf)) {
            return RC(sRC::eReceiving, sRC::ePath, sRC::eFunction, sRC::eFailed);
        }
        if (sFile::exists(pathBuf) && !sFile::remove(pathBuf)) {
            return RC(sRC::eDeleting, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
        sFil ff(pathBuf);
        sBioseqAlignment::filterChosenAlignments(&alignMap, queryStart, flagSet, &ff);
    }

    return sRC::zero;
}

sRC AlignParser::joinAls(sVioal::digestParams & vioalParams, sUsrObj * obj, const char * resultFileTemplate) const
{
    return joinAlsHelper(vioalParams, obj, resultFileTemplate, 0, true);
}

sRC AlignParser::joinAlsHelper(sVioal::digestParams & vioalParams, sUsrObj * obj, const char * resultFileTemplate, const char * samFilelist00, bool generateCoverageDict) const
{
    sStr srcVioals;
    sStr dstHiveal;
    sStr dstCoverage;

    if (!_qp.grpDataPaths(_qp.masterId ?_qp.masterId : _qp.grpId, &VIOAL_NAME[7], &srcVioals)) {
        return RC(sRC::eReceiving, sRC::ePath, sRC::eFunction, sRC::eFailed);
    }

    if (obj != NULL) {
        if (!obj->addFilePathname(dstHiveal, true, &HIVEAL_NAME[7])) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
        if (!obj->addFilePathname(dstCoverage, true, &COV_DICT_NAME[7])) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
    } else {
        if (!_qp.reqAddFile(dstHiveal, "%s%s", resultFileTemplate ? resultFileTemplate : "", &HIVEAL_NAME[7])) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
        if (!_qp.reqAddFile(dstCoverage, "%s", &COV_DICT_NAME[7])) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
    }

    sVioal vioAltAAA(0, &_subjects, &_reads);
    vioAltAAA.myCallbackFunction = sQPrideProc::reqProgressStatic;
    vioAltAAA.myCallbackParam = &_qp;

    sDic<sBioal::LenHistogram> lenHist;
    CoverageDict subCoverage(dstCoverage.ptr(0));

    if (!vioAltAAA.DigestCombineAlignmentsRaw(dstHiveal, srcVioals, vioalParams, &lenHist, generateCoverageDict ? &subCoverage : 0, 0, samFilelist00)) {
        return RC(sRC::eJoining, sRC::eData, sRC::eFunction, sRC::eFailed);
    }

    if (lenHist.dim()) {
        sStr dstHist;
        if (obj != NULL) {
            if (!obj->addFilePathname(dstHist, true, &HIST_FILE_NAME[7])) {
                return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
            }
        } else {
            if (!_qp.reqAddFile(dstHist, "%s%s", resultFileTemplate ? resultFileTemplate : "", &HIST_FILE_NAME[7])) {
                return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
            }
        }
        sFil histFile(dstHist);
        if( !histFile.ok() ) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
        sBioal::printAlignmentHistogram(&histFile, &lenHist);
    }

    sStr filenames00;
    sString::searchAndReplaceSymbols(&filenames00,srcVioals,0,"," sString_symbolsBlank,(const char *)0,0,true,true,true,true);
    for( const char * flnm = filenames00.ptr(0); flnm; flnm=sString::next00(flnm) ) {
        if ( sFile::exists(flnm) && !sFile::remove(flnm) ) {
            return RC(sRC::eDeleting, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
    }

    return sRC::zero;
}


sRC MultipleAlParser::writeAls(const char * fileList00, const WriteParams & writeParams, idx & countAls, sStr & errMsgBuf, sDic<idx> * idMap, sDic<idx> * unalignedList, sDic<idx> * subIds) const
{
    AlignMap alignmentMap;
    countAls = 0;

    idx cnt = 0;
    for (const char * p = fileList00; p && *p; p = sString::next00(p), ++cnt) {
        if (sFile::size(p)) {
            sFil fl(p, sMex::fReadonly);
            if( !fl.ok() ) {
                return RC(sRC::eAccessing, sRC::eFile, sRC::eFile, sRC::eInvalid);
            }
            countAls += sBioseqAlignment::readMultipleAlignment(&alignmentMap, fl.ptr(), fl.length(), sBioseqAlignment::eAlRelativeToMultiple, 0, _withIdlines);
        }
    }

    return filterAndWriteAlignMap(alignmentMap, 0, writeParams.flagSet);
}

sRC MultipleAlParser::joinAls(sVioal::digestParams & vioalParams, sUsrObj * obj, const char * resultFileTemplate) const
{
    return joinAlsHelper(vioalParams, obj, resultFileTemplate, 0, false);
}

sRC AlignMapParser::writeAls(AlignMap & inMap, idx alignMapQueryStart, idx flagSet, idx & countAls, sStr & errMsgBuf) const
{
    return filterAndWriteAlignMap(inMap, alignMapQueryStart, flagSet);
}

sRC SAMParser::writeAls(const char * fileList00, const WriteParams & writeParams, idx & countAls, sStr & errMsgBuf, sDic<idx> * idMap, sDic<idx> * unalignedList, sDic<idx> * subIds) const
{
    AlignMap alignmentMap;
    countAls = 0;

    sStr samPathBuf;
    if(!_qp.reqSetData(_qp.reqId, LOSSLESS_SAM_INFO_NAME, 0, 0) || !_qp.reqDataPath(_qp.reqId, &LOSSLESS_SAM_INFO_NAME[7], &samPathBuf)) {
        _qp.reqSetInfo(_qp.reqId, _qp.eQPLogType_Error, "failed to create sam content destination");
        countAls = 0;
        return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
    }
    if (sFile::exists(samPathBuf) && !sFile::remove(samPathBuf)) {
        return RC(sRC::eDeleting, sRC::eFile, sRC::eFunction, sRC::eFailed);
    }
    sFil samcontent(samPathBuf);

    samPathBuf.cut(0);
    if(!_qp.reqSetData(_qp.reqId, LOSSLESS_SAM_FOOTER_NAME, 0, 0) || !_qp.reqDataPath(_qp.reqId, &LOSSLESS_SAM_FOOTER_NAME[7], &samPathBuf)) {
        _qp.reqSetInfo(_qp.reqId, _qp.eQPLogType_Error, "failed to create sam footer destination");
        countAls = 0;
        return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
    }
    if (sFile::exists(samPathBuf) && !sFile::remove(samPathBuf)) {
        return RC(sRC::eDeleting, sRC::eFile, sRC::eFunction, sRC::eFailed);
    }
    sFil samFooter(samPathBuf);

    samPathBuf.cut(0);
    if(!_qp.reqSetData(_qp.reqId, SAM_HEADER_SLICE_NAME, 0, 0) || !_qp.reqDataPath(_qp.reqId, &SAM_HEADER_SLICE_NAME[7], &samPathBuf)) {
        _qp.reqSetInfo(_qp.reqId, _qp.eQPLogType_Error, "failed to create sam header destination");
        countAls = 0;
        return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
    }
    if (sFile::exists(samPathBuf) && !sFile::remove(samPathBuf)) {
        return RC(sRC::eDeleting, sRC::eFile, sRC::eFunction, sRC::eFailed);
    }
    sFil samHeaderSlice(samPathBuf);

    idx cnt = 0;
    sStr samHeaderPartNames;
    for (const char * p = fileList00; p && *p; p = sString::next00(p), ++cnt) {
        if (sFile::size(p)) {
            sFil fl(p, sMex::fReadonly);
            if( !fl.ok() ) {
                return RC(sRC::eAccessing, sRC::eFile, sRC::eFile, sRC::eInvalid);
            }

            const char * headerName = 0;
            if (!(headerName = _qp.reqAddFile(samHeaderPartNames, "req-samheader-%lld", cnt))) {
                return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
            }
            samHeaderPartNames.add0();
            sFil headerFile(headerName);

            countAls += sVioseq2::convertSAMintoAlignmentMap(fl.ptr(), fl.length(), &alignmentMap, idMap, writeParams.minMatchLength, writeParams.isMinMatchLengthPercentage,  writeParams.maxMissQueryPercent, subIds, 0, _useRowInformationtoExtractQry, &headerFile, &samcontent, &samFooter);
        }
    }
    samHeaderPartNames.add0();

    if (sRC rc = joinAndDumpSAMHeaders(samHeaderPartNames.ptr(0), samHeaderSlice, 0)) {
        return rc;
    }

    if( countAls < 0 ) {
        errMsgBuf.printf(0, "One or more reference ids were not resolved using reference list provided");
    } else if( countAls != _reads.dim() ) {
        errMsgBuf.printf("Number of reads %" DEC " do not correlate with number of alignments %" DEC, _reads.dim(), countAls);
    }

    return filterAndWriteAlignMap(alignmentMap, 0, writeParams.flagSet);
}

sRC SAMParser::joinAndDumpSAMHeaders(const char * srcSamFiles00, sFil & outFile, sUsrObj * obj) const
{
    sDic<idx> samHeader;
    for (const char * p = srcSamFiles00; p && *p; p = sString::next00(p)) {
        sFil fl(p, sMex::fReadonly);
        if( !fl.ok() ) {
            return RC(sRC::eAccessing, sRC::eFile, sRC::eFile, sRC::eInvalid);
        }

        const char * end = fl.last();
        const char * fp = fl.ptr(0);
        char fc;
        while ( fp && fp < end && (fc = *fp) && fc == '@') {
            const char * lp = fp;
            char lc;
            while ( lp && lp < end && (lc = *lp) && lc != '\n') {
                ++lp;
            }
            idx carriageRetFix = 0;
            if (lp > fp && *(lp - 1) == '\r') {
                carriageRetFix = -1;
            }
            samHeader.setString(fp, lp - fp + carriageRetFix);
            fp = lp;
            if (fp && fp < end && *fp) {
                ++fp;
            }
        }
    }

    for (idx i = 0; i < samHeader.dim(); ++i) {
        idx len = 0;
        const char * headerLine = static_cast<const char *>(samHeader.id(i, &len));
        outFile.add(headerLine, len);
        outFile.add("\n", 1);
    }

    return sRC::zero;
}

sRC SAMParser::concatAndDumpSAMFooters(sUsrObj * obj) const
{
    sStr srcFooters00;
    if (_qp.grpDataPaths(_qp.masterId ?_qp.masterId : _qp.grpId, &LOSSLESS_SAM_FOOTER_NAME[7], &srcFooters00, _qp.vars.value("serviceName"), 0)) {
        sStr dstFooter;
        if (obj != NULL) {
            if (!obj->addFilePathname(dstFooter, true, &LOSSLESS_SAM_FOOTER_NAME[7])) {
                return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
            }
        } else {
            if (!_qp.reqAddFile(dstFooter, &LOSSLESS_SAM_FOOTER_NAME[7])) {
                return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
            }
        }
        sFil samFooter(dstFooter);
        if ( !samFooter.ok() ) {
            return RC(sRC::eAccessing, sRC::eFile, sRC::eFile, sRC::eInvalid);
        }

        for (const char * p = srcFooters00; p && *p; p = sString::next00(p)) {
            if (sFile::size(p)) {
                sFil fl(p, sMex::fReadonly);
                samFooter.add(fl.ptr(0), fl.length());
            }
        }
    }

    return sRC::zero;
}

sRC SAMParser::joinAls(sVioal::digestParams & vioalParams, sUsrObj * obj, const char * resultFileTemplate) const
{
    sStr srcSamFiles00;
    if (!_qp.grpDataPaths(_qp.masterId ?_qp.masterId : _qp.grpId, &LOSSLESS_SAM_INFO_NAME[7], &srcSamFiles00, _qp.vars.value("serviceName"), 0)) {
        return RC(sRC::eReceiving, sRC::ePath, sRC::eFunction, sRC::eFailed);
    }
    if (sRC rc = joinAlsHelper(vioalParams, obj, resultFileTemplate, srcSamFiles00.ptr(0), true)) {
        return rc;
    }

    sStr srcSamHeaders00;
    if (!_qp.grpDataPaths(_qp.masterId ?_qp.masterId : _qp.grpId, &SAM_HEADER_SLICE_NAME[7], &srcSamHeaders00, _qp.vars.value("serviceName"), 0)) {
        return RC(sRC::eReceiving, sRC::ePath, sRC::eFunction, sRC::eFailed);
    }
    sStr dstHeader;
    if (obj != NULL) {
        if (!obj->addFilePathname(dstHeader, true, &SAM_HEADER_NAME[7])) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
    } else {
        if (!_qp.reqAddFile(dstHeader, &SAM_HEADER_NAME[7])) {
            return RC(sRC::eAllocating, sRC::eFile, sRC::eFunction, sRC::eFailed);
        }
    }
    sFil samHeader(dstHeader);
    if( !samHeader.ok() ) {
        return RC(sRC::eAccessing, sRC::eFile, sRC::eFile, sRC::eInvalid);
    }
    if (sRC rc = joinAndDumpSAMHeaders(srcSamHeaders00.ptr(0), samHeader, obj)) {
        return rc;
    }

    if (sRC rc = concatAndDumpSAMFooters(obj)) {
        return rc;
    }

    return sRC::zero;
}

sRC BLASTParser::writeAls(const char * fileList00, const WriteParams & writeParams, idx & countAls, sStr & errMsgBuf, sDic<idx> * idMap, sDic<idx> * unalignedList, sDic<idx> * subIds) const
{
    AlignMap alignmentMap;
    countAls = 0;

    idx cnt = 0;
    for (const char * p = fileList00; p && *p; p = sString::next00(p), ++cnt) {
        if (sFile::size(p)) {
            sFil fl(p, sMex::fReadonly);
            if( !fl.ok() ) {
                return RC(sRC::eAccessing, sRC::eFile, sRC::eFile, sRC::eInvalid);
            }
            sIO log;
            countAls += sBioAlBlast::SSSParseAlignment(&log, fl.ptr(), fl.length(), &alignmentMap, writeParams.scoreFilter, writeParams.minMatchLength, writeParams.isMinMatchLengthPercentage, writeParams.maxMissQueryPercent, idMap, 0, 0, _blastMode, unalignedList);
            if( log ) {
                _qp.logOut(_qp.eQPLogType_Debug, "%s", log.ptr());
            }
        }
    }

    return filterAndWriteAlignMap(alignmentMap, 0, writeParams.flagSet);
}
