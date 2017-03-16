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

#include "exec-context.hpp"
#include "../../vlib/ulib/uperm.hpp"

using namespace slib;
using namespace slib::tblqryx4;

ExecContext::InputTableSource::InputTableSource()
{
    mode = eObj;
    data_req_id = 0;
    data_is_grp = false;
    tblname_index = -1;
    idx_suffix_index = -1;
    colsep_index = -1;
    comment_prefix_index = -1;
    parse_start = 0;
    parse_cnt = sIdxMax;
    qry_node = 0;
    fallback_default_name = false;
    initial_offset = 0;
    header_offset = -1;
    max_len = sIdxMax;
}

ExecContext::InputTable::InputTable()
{
    tbl = 0;
    owned = true;
}

ExecContext::InputTable::~InputTable()
{
    if( owned ) {
        delete tbl;
    }
}

const char * ExecContext::getLoaderTblSourceString(idx index) const
{
    return index >= 0 ? static_cast<const char *>(_tbl_source_strings.id(index)) : 0;
}

idx ExecContext::allocateLoaderHandle()
{
    _in_tables.add(1)->sources.init(&_tbl_source_mex);
    return _in_tables.dim() - 1;
}

bool ExecContext::hasLoaderHandle(idx loader_handle) const
{
    return loader_handle >= 0 && loader_handle < _in_tables.dim();
}

void ExecContext::requestLoadReqTable(idx loader_handle, idx dataReqID, bool isgrp, const char * tblname, const char * idx_suffix, const char * colsep, const char * commentPrefix, idx parseStart, idx parseCnt, idx initialOffset, idx headerOffset, idx maxLen)
{
    InputTableSource * src = _in_tables[loader_handle].sources.add(1);
    src->mode = InputTableSource::eReq;
    src->data_req_id = dataReqID;
    src->data_is_grp = isgrp;
    if( tblname ) {
        _tbl_source_strings.setString(tblname, 0, &src->tblname_index);
    }
    if( idx_suffix ) {
        _tbl_source_strings.setString(idx_suffix, 0, &src->idx_suffix_index);
    }
    if( colsep ) {
        _tbl_source_strings.setString(colsep, 0, &src->colsep_index);
    }
    if( commentPrefix ) {
        _tbl_source_strings.setString(commentPrefix, 0, &src->comment_prefix_index);
    }
    src->parse_cnt = parseCnt;
    src->initial_offset = initialOffset;
    src->header_offset = headerOffset;
    src->parse_start = parseStart;
    src->max_len = maxLen;
}

void ExecContext::requestLoadObjTable(idx loader_handle, const sHiveId & objID, const char * tblname, const char * idx_suffix, bool fallback_default_name, const char * colsep, const char * commentPrefix, idx parseStart, idx parseCnt, idx initialOffset, idx headerOffset, idx maxLen)
{
    InputTableSource * src = _in_tables[loader_handle].sources.add(1);
    src->mode = InputTableSource::eObj;
    src->obj_id = objID;
    if( tblname ) {
        _tbl_source_strings.setString(tblname, 0, &src->tblname_index);
    }
    if( idx_suffix ) {
        _tbl_source_strings.setString(idx_suffix, 0, &src->idx_suffix_index);
    }
    if( colsep ) {
        _tbl_source_strings.setString(colsep, 0, &src->colsep_index);
    }
    if( commentPrefix ) {
        _tbl_source_strings.setString(commentPrefix, 0, &src->comment_prefix_index);
    }
    src->fallback_default_name = fallback_default_name;
    src->parse_cnt = parseCnt;
    src->initial_offset = initialOffset;
    src->header_offset = headerOffset;
    src->parse_start = parseStart;
    src->max_len = maxLen;
}

idx ExecContext::allocateRequestLoadReqTables(sVec<idx> & loader_handles, idx dataReqID, bool isgrp, const char * glob)
{
    sFileGlob glb;
    glb.compile(glob);

    sVec <idx> reqs;
    if( isgrp ) {
        _proc.grp2Req(dataReqID, &reqs);
    } else {
        *reqs.add(1) = dataReqID;
    }
    sStr data_names00;
    idx cnt = 0;
    for(idx i=0; i<reqs.dim(); i++) {
        data_names00.cut(0);
        _proc.dataGetAll(dataReqID, 0, &data_names00);
        for(const char * dname = data_names00.ptr(); dname && *dname; dname = sString::next00(dname)) {
            if( glb.match(dname) ) {
                idx loader_handle = allocateLoaderHandle();
                *loader_handles.add(1) = loader_handle;
                requestLoadReqTable(loader_handle, reqs[i], false, dname);
                cnt++;
            }
        }
    }
    return cnt;
}

idx ExecContext::allocateRequestLoadObjTables(sVec<idx> & loader_handles, const sHiveId & objID, const char * glob)
{
    sUsrObj obj(*_proc.user, objID);
    if( !obj.Id() ) {
        logError("Failed to open object %s", objID.print());
        return -1;
    }

    sDir files;
    obj.files(files, sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive) | sFlag(sDir::bitSubdirSlash), glob);
    for(idx i=0; i<files.dimEntries(); i++) {
        idx loader_handle = allocateLoaderHandle();
        *loader_handles.add(1) = loader_handle;
        requestLoadObjTable(loader_handle, objID, files.getEntryPath(i));
    }
    return files.dimEntries();
}

//! can only be called during op init stage
void ExecContext::requestLoadObjqryTable(idx loader_handle, qlang::ast::Node * qry_node)
{
    InputTableSource * src = _in_tables[loader_handle].sources.add(1);
    src->mode = InputTableSource::eObjQry;
    src->qry_node = qry_node;
}

//! can only be called during op process stage
sTabular * ExecContext::getLoadedTable(idx loader_handle)
{
    return hasLoaderHandle(loader_handle) ? _in_tables[loader_handle].tbl : 0;
}

bool ExecContext::isLoadedTable(const sTabular * tbl) const
{
    for(idx loader_handle = 0; loader_handle < _in_tables.dim(); loader_handle++) {
        if( hasLoaderHandle(loader_handle) && _in_tables[loader_handle].tbl == tbl ) {
            return true;
        }
    }
    return false;
}

//! can only be called during op process stage
sTabular * ExecContext::releaseLoadedTable(idx loader_handle)
{
    if( hasLoaderHandle(loader_handle) ) {
        sTabular * ret = _in_tables[loader_handle].tbl;
        _in_tables[loader_handle].tbl = 0;
        _in_tables[loader_handle].owned = false;
        return ret;
    } else {
        return 0;
    }
}

sTxtTbl * ExecContext::newCSVorVCF(const char * name, const char * tblIdxPath, const char * tblDataPath, sFil * tblFil, idx tblFilTime, sTxtTbl::ParseOptions & opts)
{
    sFileGlob vcfGlob, tsvGlob;
    vcfGlob.compile("*.vcf", false);
    tsvGlob.compile("*.{tsv,tab}", false);
    bool vcf = name ? vcfGlob.match(name) : false;
    bool tsv = name ? tsvGlob.match(name) : false;
    sTxtTbl * tbl = vcf ? new sVcfTbl(tblIdxPath, true) : new sTxtTbl(tblIdxPath, true);

    if( !opts.colsep ) {
        opts.colsep = (vcf || tsv) ? "\t" : ",";
    }

    if( tblDataPath ) {
        tbl->setFile(tblDataPath);
    } else {
        tbl->borrowFile(tblFil, tblFilTime);
    }

    if( tbl->isParsed() ) {
        return tbl;
    }

    if( opts.absrowCnt && opts.absrowCnt < sIdxMax ) {
        // if we are only parsing the top of the table, we don't want to save the index
        delete tbl;
        tbl = vcf ? new sVcfTbl() : new sTxtTbl();
        if( tblDataPath ) {
            tbl->setFile(tblDataPath);
        } else {
            tbl->borrowFile(tblFil, tblFilTime);
        }
    }

    tbl->parseOptions().flags = opts.flags;
    tbl->parseOptions().colsep = opts.colsep;
    tbl->parseOptions().comment = opts.comment;
    tbl->parseOptions().progressCallback = reportSubProgressStatic;
    tbl->parseOptions().progressParam = this;

    tbl->parseOptions().absrowStart = opts.absrowStart;
    if( opts.absrowCnt )
        tbl->parseOptions().absrowCnt = opts.absrowCnt;

    tbl->parseOptions().initialOffset = opts.initialOffset;
    tbl->parseOptions().headerOffset = opts.headerOffset;
    tbl->parseOptions().maxLen = opts.maxLen;

    if( !_proc.reqLock(tblIdxPath, &_waiting_for_req) ) {
        delete tbl;
        return 0;
    }

    if( !tbl->parse() ) {
        delete tbl;
        _proc.reqUnlock(tblIdxPath);
        return 0;
    }

    tbl->remapReadonly();

    _proc.reqUnlock(tblIdxPath);
    return tbl;
}

void ExecContext::pushInputTable(idx loader_handle, sTabular * tbl, bool owned)
{
    if( loader_handle >= _in_tables.dim() || !_in_tables[loader_handle].tbl ) {
        _in_tables.resize(loader_handle + 1);
        _in_tables[loader_handle].tbl = tbl;
        _in_tables[loader_handle].owned = owned;
        return;
    }

    sCatTabular * cat = dynamic_cast<sCatTabular*>(_in_tables[loader_handle].tbl);
    if( !cat ) {
        cat = new sCatTabular;
        cat->pushSubTable(_in_tables[loader_handle].tbl, _in_tables[loader_handle].owned);
        _in_tables[loader_handle].tbl = cat;
        _in_tables[loader_handle].owned = true; // cat-table is owned by the exec context
    }

    cat->pushSubTable(tbl, owned);
}

static bool saneFilePath(const char * path)
{
    return path && *path && sFile::exists(path) && !sDir::exists(path);
}

bool ExecContext::loadFileObject(idx loader_handle, InputTableSource * source)
{
    sStr tblDataPath, tblIdxName, tblIdxPath;
    sStr objIDStr;
    source->obj_id.print(objIDStr);

    sUsrObj * obj = _proc.user->objFactory(source->obj_id, 0, ePermCanRead | ePermCanDownload);
    if( !obj || !obj->Id() ) {
        logError("Failed to open object %s", objIDStr.ptr());
        delete obj;
        return false;
    }

    sUsrFile * ufile = dynamic_cast<sUsrFile*>(obj);
    const char * tblname = getLoaderTblSourceString(source->tblname_index);

    obj->getFilePathname(tblDataPath, "%s", tblname);
    if( !saneFilePath(tblDataPath) && source->fallback_default_name && ufile ) {
        tblDataPath.cut0cut();
        if( ufile->isTypeOf("excel-file") ) {
            // use the first sheet (sort by natural string comparison)
            sDir files;
            if( ufile->files(files, sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive) | sFlag(sDir::bitSubdirSlash), "*.csv") ) {
                sVec<const char *> entries;
                sVec<idx> ind_entries;
                entries.resize(files.dimEntries());
                ind_entries.resize(files.dimEntries());
                for(idx i=0; i<files.dimEntries(); i++) {
                    entries[i] = files.getEntryPath(i);
                }
                sSort::sortStringsNaturalCaseInd(files.dimEntries(), entries.ptr(), ind_entries.ptr());
                obj->getFilePathname(tblDataPath, "%s", entries[ind_entries[0]]);
            }
        } else {
            // fall back to u-file's default file
            ufile->getFile(tblDataPath);
        }
    }
    if( !saneFilePath(tblDataPath) ) {
        logError("Object %s doesn't have file \"%s\"", objIDStr.ptr(), tblname);
        delete obj;
        return false;
    }
    if( !tblname ) {
        tblname = sFilePath::nextToSlash(tblDataPath);
    }

    if( const char * idx_suffix = getLoaderTblSourceString(source->idx_suffix_index) ) {
        tblIdxName.printf("obj%s-%s.%s.idx2", objIDStr.ptr(), tblname, idx_suffix);
    } else {
        tblIdxName.printf("obj%s-%s.idx2", objIDStr.ptr(), tblname);
    }
    _proc.cfgPath(tblIdxPath, 0, tblIdxName, "tblqryx.tableRepository");

    sTxtTbl::ParseOptions opts;
    opts.flags = sTblIndex::fSaveRowEnds;
    if( _top_header )
        opts.flags |= sTblIndex::fTopHeader;
    if( _left_header )
        opts.flags |= sTblIndex::fLeftHeader;

    if( ufile && ufile->isTypeOf("csv-table") ) {
        opts.colsep = ",";
    } else if( ufile && ufile->isTypeOf("tsv-table") ) {
        opts.colsep = "\t";
    } else {
        opts.colsep = getLoaderTblSourceString(source->colsep_index);
    }
    opts.comment = getLoaderTblSourceString(source->comment_prefix_index);
    opts.absrowCnt = source->parse_cnt;
    opts.initialOffset = source->initial_offset;
    opts.headerOffset = source->header_offset;
    opts.absrowStart = source->parse_start;
    opts.maxLen = source->max_len;

    _waiting_for_req = 0;
    sTxtTbl * tbl = newCSVorVCF(tblDataPath, tblIdxPath, tblDataPath, 0, 0, opts);
    if( !tbl ) {
        if( _waiting_for_req ) {
            logDebug("Index for table %s for object %s locked by request %" DEC, tblDataPath.ptr(), objIDStr.ptr(), _waiting_for_req);
        } else {
#ifdef _DEBUG
            // Don't show storage paths to user!
            fprintf(stderr, "Failed to parse table %s for object %s\n", tblDataPath.ptr(), objIDStr.ptr());
#endif
            logError("Failed to parse table %s for object %s\n", tblname, objIDStr.ptr());
        }
        delete obj;
        return false;
    }

#ifdef _DEBUG
    fprintf(stderr, "Loading table %s for object %s (index file %s)\n", tblDataPath.ptr(), objIDStr.ptr(), tblIdxPath.ptr(0));
#endif

    sStr pretty_name;
    if( ufile ) {
        ufile->propGet("name", &pretty_name);
        pretty_name.shrink00();
    }

    tbl->setTableMetadata("name", pretty_name.length() ? pretty_name.ptr() : tblname);
    tbl->setTableMetadata("obj", objIDStr.ptr());
    pushInputTable(loader_handle, tbl);

    delete obj;
    return true;
}

bool ExecContext::loadRequestFile(idx loader_handle, InputTableSource * source)
{
    sStr tblIdxName, tblIdxPath;
    sFil tblFil;
    idx tblFilTime = sIdxMax;
    const char * tblname = getLoaderTblSourceString(source->tblname_index);

    if( !_proc.reqAuthorized(source->data_req_id) ) {
        logError("Not authorized to read request %" UDEC " data", source->data_req_id);
        return false;
    }

    if( const char * idx_suffix = getLoaderTblSourceString(source->idx_suffix_index) ) {
        tblIdxName.printf("req%" DEC "-%s.%s.idx2", source->data_req_id, tblname, idx_suffix);
    } else {
        tblIdxName.printf("req%" DEC "-%s.idx2", source->data_req_id, tblname);
    }
    _proc.cfgPath(tblIdxPath, 0, tblIdxName, "tblqryx.tableRepository");

    if( source->data_is_grp ) {
        if( !_proc.grpGetData(source->data_req_id, tblname, &tblFil, true, 0, &tblFilTime) ) {
            logError("Request group %" UDEC " doesn't have table %s", source->data_req_id, tblname);
            return false;
        }
    } else {
        if( !_proc.reqGetData(source->data_req_id, tblname, &tblFil, true, &tblFilTime) ) {
            logError("Request %" UDEC " doesn't have table %s", source->data_req_id, tblname);
            return false;
        }
    }

    sTxtTbl::ParseOptions opts;
    opts.flags = sTblIndex::fSaveRowEnds;
    if( _top_header )
        opts.flags |= sTblIndex::fTopHeader;
    if( _left_header )
        opts.flags |= sTblIndex::fLeftHeader;

    opts.colsep = getLoaderTblSourceString(source->colsep_index);
    opts.comment = getLoaderTblSourceString(source->comment_prefix_index);
    opts.absrowCnt = source->parse_cnt;
    opts.initialOffset = source->initial_offset;
    opts.headerOffset = source->header_offset;
    opts.absrowStart = source->parse_start;
    opts.maxLen = source->max_len;

    _waiting_for_req = 0;
    sTxtTbl * tbl = newCSVorVCF(tblname, tblIdxPath, 0, &tblFil, tblFilTime, opts);
    if( !tbl ) {
        if( _waiting_for_req ) {
            logDebug("Index for table %s for request %" UDEC " locked by request %" UDEC, tblname, source->data_req_id, _waiting_for_req);
        } else {
            logError("Failed to parse table %s for request %" UDEC, tblname, source->data_req_id);
        }
        return false;
    }

#ifdef _DEBUG
    fprintf(stderr, "Loading table %s for request %" UDEC " (index file %s)\n", tblname, source->data_req_id, tblIdxPath.ptr(0));
#endif

    tbl->setTableMetadata("name", tblname);
    pushInputTable(loader_handle, tbl);

    return true;
}
