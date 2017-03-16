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

#include <slib/utils/json/parser.hpp>
#include <qpsvc/dna-cgi.hpp>

#include "exec-context.hpp"
#include "tblqryX4_cmd.hpp"

using namespace slib;
using namespace slib::tblqryx4;

ExecContext::ExecContext(sQPrideProc * proc) :
    _proc(*proc),
    _qlang_ctx(*proc->user)
{
    _out_req = proc->reqId;
    _subProgressCur = 0;
    _subProgressNum = 1;
    _progressItems = _subProgressItems = 0;

    _out_table = 0;
    _waiting_for_req = 0;
    _top_header = true;
    _left_header = false;
    _root_objQry = 0;

    _cur_icmd = -1;

    allocateLoaderHandle();
}

ExecContext::~ExecContext()
{
    cleanupCommandOutputs(0, _commands.dim());

    for(idx i = _commands.dim() - 1; i >= 0; i--) {
        delete _commands[i];
        _commands[i] = 0;
    }

    delete _root_objQry;
}

void ExecContext::startSubProgress()
{
    _subProgressItems = 0;
    _proc.progress100Start = 100.0 * _subProgressCur / _subProgressNum;
    _proc.progress100End = 100.0 / _subProgressNum;
    _proc.reqProgress(-1, 0, 100);
}

void ExecContext::endSubProgress()
{
    _progressItems += _subProgressItems;
    _subProgressItems = 0;
    _subProgressCur++;
    _subProgressNum = sMax<idx>(_subProgressCur, _subProgressNum);
    _proc.progress100Start = 100.0 * _subProgressCur / _subProgressNum;
    _proc.progress100End = 100.0 / _subProgressNum;
    _proc.reqProgress(-1, 0, 100);
}

idx ExecContext::reportSubProgress(idx items, idx progress, idx progressMax)
{
    if (progress == sNotIdx) {
        progress = items;
    }
    if (items > -1) {
        _subProgressItems = items;
        return _proc.reqProgress(_progressItems + _subProgressItems, progress, progressMax);
    } else if (items < -1) {
        _subProgressItems = -items;
        return _proc.reqProgress(-(_progressItems + _subProgressItems), progress, progressMax);
    } else {
        return _proc.reqProgress(-1, progress, progressMax);
    }
}

// static
idx ExecContext::reportSubProgressStatic(void * param, idx items, idx progress, idx progressMax)
{
    if (ExecContext * self = static_cast<ExecContext*>(param)) {
        return self->reportSubProgress(items, progress, progressMax);
    } else {
        return 0;
    }
}

bool ExecContext::appendTqs(const char * tqs_string, idx len, idx tqsMaxCnt)
{
    if( !tqs_string || !*tqs_string ) {
        return true;
    }

    sJSONParser json_parser;
    sStr error;
    if( unlikely(!json_parser.parse(tqs_string, len)) ) {
        json_parser.printError(error);
        logError("JSON parser failed to parse TQS query: %s", error.ptr());
        return false;
    }

    if( !json_parser.result().isList() ) {
        logError("TQS query must be a JSON list, not %s", json_parser.result().getTypeName());
        return false;
    }

    if( !_tqs.isList() ) {
        _tqs.setList();
    }

    idx resultCnt = sMin<idx>(tqsMaxCnt - _tqs.dim(), json_parser.result().dim());

    for(idx i = 0; i < resultCnt; i++) {
        _tqs.push(*json_parser.result().getListElt(i));
    }

    return true;
}

bool ExecContext::parseForm()
{
    sVar * pForm = _proc.pForm;
    sStr error, buf;

    _waiting_for_req = 0;

    // dataCmd: need to launch dna.cgi backend
    if( const char * dataCmd = pForm->value("dataCmd", 0) ) {
        sVariant dataInfo;
        buf.cut(0);
        const char * dataInfoName = "data-info.json";
        _proc.reqGetData(_proc.reqId, dataInfoName, &buf, false);
        if( !buf.length() ) {
            dmDnaCgi dna_cgi(_proc, dataCmd);
            dna_cgi.setSessionID(_proc.pForm);
            _waiting_for_req = dna_cgi.launch(*_proc.user, _proc.reqId);
            logInfo("launched dna.cgi?%s as request %" DEC, dataCmd, _waiting_for_req);
            if( _waiting_for_req <= 0 ) {
                logError("Failed to launch dna.cgi?%s", dataCmd);
                return false;
            }
            dataInfo.setDic();
            dataInfo.setElt("dataID", _waiting_for_req);
            dataInfo.setElt("tbl", "cgi_output");
            buf.cut(0);
            dataInfo.print(buf, sVariant::eJSON);
            _proc.reqSetData(_proc.reqId, dataInfoName, buf.mex());
            logInfo("Waiting for dna.cgi subrequest %" DEC, _waiting_for_req);
            return false;
        } else {
            sJSONParser json_parser;
            if( unlikely(!json_parser.parse(buf, buf.length())) ) {
                json_parser.printError(error);
                logError("Failed to read data-info.json: %s", error.ptr());
                _waiting_for_req = 0;
                return false;
            }

            sVariant * pval = json_parser.result().getDicElt("dataID");
            _waiting_for_req = pval ? pval->asInt() : 0;
            if( _waiting_for_req <= 0 ) {
                logError("Unexpected dataID=%" DEC " in read data-info.json", _waiting_for_req);
                return false;
            }

            switch(_proc.reqGetStatus(_waiting_for_req)) {
                case sQPrideBase::eQPReqStatus_Waiting:
                case sQPrideBase::eQPReqStatus_Processing:
                case sQPrideBase::eQPReqStatus_Running:
                case sQPrideBase::eQPReqStatus_Suspended:
                    // suspend and wait
                    logInfo("Still waiting for dna.cgi subrequest %" DEC, _waiting_for_req);
                    return false;
                case sQPrideBase::eQPReqStatus_Done:
                    // subrequest is done, we can continue!
                    buf.printf(0, "%" DEC, _waiting_for_req);
                    _proc.pForm->inp("dataID", buf.ptr());
                    pval = json_parser.result().getDicElt("tbl");
                    _proc.pForm->inp("tbl", pval ? pval->asString() : 0);
                    logDebug("dna.cgi subrequest done; setting dataID=%" DEC " and tbl=%s", _waiting_for_req, pval ? pval->asString() : "");
                    _waiting_for_req = 0;
                    break;
                default:
                    // some sort of error or unknown code
                    logError("dna.cgi subrequest %" DEC " failed or was killed", _waiting_for_req);
                    _waiting_for_req = 0;
                    return false;
            }
        }
    }

    _top_header = pForm->boolvalue("hdr", true);
    _out_req = pForm->ivalue("resID", _proc.reqId);

    idx tqsMaxCnt = pForm->ivalue("tqsCnt", sIdxMax);

    sHiveId tqsId(pForm->value("tqsId"));

    if( tqsId ) {
        sUsrFile tqsFileObj(tqsId, _proc.user);
        const char * tqsFilename = tqsFileObj.Id() ? tqsFileObj.getFile(buf) : 0;
        sFil tqsFile(tqsFilename, sMex::fReadonly);
        if( !tqsFile.length() ) {
            logError("Failed to load TQS query file from object ID %s", tqsId.print());
            return false;
        }
        if( !appendTqs(tqsFile.ptr(), tqsFile.length(), tqsMaxCnt) ) {
            return false;
        }
    }

    if( !appendTqs(pForm->value("tqs"), 0, tqsMaxCnt) ) {
        return false;
    }

    if( _tqs.isList() && _tqs.dim() ) {
        sStr tqs_file;
        _tqs.print(tqs_file, sVariant::eJSON);
#ifdef _DEBUG
        fprintf(stderr, "tqs.json:\n%s\n", tqs_file.ptr());
#endif

        _proc.reqSetData(_out_req, "tqs.json", tqs_file.mex());
    }

    // static inputs declaration

    if( const char * obj_qry_string = pForm->value("objQry") ) {
        if( obj_qry_string[0] ) {
            sStr err_buf;
            if (unlikely(!_qlang_parser.parse(obj_qry_string, 0, qlang::Parser::fDollarValues))) {
                _qlang_parser.printError(err_buf);
                logError("Query language failed to parse objQry param: %s", error.ptr());
                return false;
            }
            _root_objQry = _qlang_parser.releaseAstRoot();
            if( _root_objQry ) {
                requestLoadObjqryTable(0, _root_objQry);
            }
        }
    }

    sStr tblnames00;
    sString::searchAndReplaceSymbols(&tblnames00, _proc.pForm->value("tbl"), 0, "\n", 0, 0, true, true, false, false);
    tblnames00.add0cut(2);
    bool fallback_default_name = sString::cnt00(tblnames00.ptr()) < 2;

    bool isgrp;
    idx dataID = _proc.pForm->ivalue("dataGrpID");
    if( dataID ) {
        isgrp = true;
    } else {
        dataID = _proc.pForm->ivalue("dataID");
        isgrp = false;
    }

    sVec<sHiveId> objIDs;
    sHiveId::parseRangeSet(objIDs, _proc.pForm->value("objs"));
    _qlang_ctx.setObjs(objIDs.ptr(), objIDs.dim());

    const char * colsep = _proc.pForm->value("colsep", 0);
    const char * commentPrefix = _proc.pForm->value("commentPrefix", 0);
    idx parseCnt = _proc.pForm->ivalue("parseCnt");
    idx parseStart = sMax<idx>(0, _proc.pForm->ivalue("parseStart", 0));

    if( dataID ) {
        for(const char * tblname = tblnames00.ptr(); tblname; tblname = sString::next00(tblname)) {
            requestLoadReqTable(0, dataID, isgrp, tblname, 0, colsep, commentPrefix, parseStart, parseCnt, 0, -1);
        }
    }

    for(idx i = 0; i < objIDs.dim(); i++) {
        if( !objIDs[i] ) {
            continue;
        }
        for(const char * tblname = tblnames00.ptr(); tblname; tblname = sString::next00(tblname)) {
            requestLoadObjTable(0, objIDs[i], tblname, 0, fallback_default_name, colsep, commentPrefix, parseStart, parseCnt, 0, -1);
        }
    }

    return true;
}

bool ExecContext::loadInput()
{
    // count the number of progress-reporting stages
    _subProgressNum = _commands.dim();
    for(idx i=0; i<_commands.dim(); i++) {
        if( _commands[i]->hasProgress() ) {
            _subProgressNum++;
        }
    }

    for(idx ii=0; ii<_in_tables.dim(); ii++) {
        _subProgressNum += _in_tables[ii].sources.dim();
    }

    sStr eval_node_error;

    for(idx ii=0; ii<_in_tables.dim(); ii++) {
        InputTable & in_table = _in_tables[ii];
        for(idx is=0; is<in_table.sources.dim(); is++) {
            startSubProgress();
            InputTableSource * source = in_table.sources.ptr(is);
            switch (source->mode) {
                case InputTableSource::eReq:
                    if( !loadRequestFile(ii, source) ) {
                        return false;
                    }
                    break;
                case InputTableSource::eObj:
                    if( !loadFileObject(ii, source) ) {
                        return false;
                    }
                    break;
                case InputTableSource::eObjQry:
                    sVariant * pqry_result = _in_table_qry_results.add(1);
                    if( !source->qry_node || !source->qry_node->eval(*pqry_result, _qlang_ctx) ) {
                        _qlang_ctx.printError(eval_node_error);
                        logError("Failed to evaluate object query: %s", eval_node_error.ptr());
                        return false;
                    }
                    sVariantTblData * tbld = dynamic_cast<sVariantTblData*>(pqry_result->asData());
                    if( !tbld ) {
                        logError("Object query did not produce a table");
                        return false;
                    }
                    // the table is owned by _in_table_qry_results
                    pushInputTable(ii, &tbld->getTable(), false);
            }
            endSubProgress();
        }
    }

    if( !getLoadedTable(0) ) {
        _in_tables.resize(1);
        _in_tables[0].tbl = new sTxtTbl;
        _in_tables[0].owned = true;
    }

    _qlang_ctx.setTable(getLoadedTable(0));

    return true;
}

bool ExecContext::substituteTemplate(sVariant * arg)
{
    if( arg->isDic() ) {
        if( sVariant * tmplt_val = arg->getDicElt("tmpl") ) {
            qlang::ast::Node * fmla = ParseUtils::parseFormulaArg(tmplt_val, *this, "template formula");
            if( unlikely(!fmla) ) {
                return false;
            }
            if( unlikely(!fmla->eval(*arg, _qlang_ctx)) ) {
                sStr err, buf;
                _qlang_ctx.printError(err);
                logError("Query language error when substituting template formula %s: %s\n", tmplt_val->print(buf, sVariant::eJSON), err.ptr());
                return false;
            }
            delete fmla;
            return true;
        }

        for (idx i=0; i<arg->dim(); i++) {
            if( !substituteTemplate(arg->getDicElt(i)) ) {
                return false;
            }
        }
    } else if( arg->isList() ) {
        for (idx i=0; i<arg->dim(); i++) {
            if( !substituteTemplate(arg->getListElt(i)) ) {
                return false;
            }
        }
    }
    return true;
}

void ExecContext::cleanupCommandOutputs(idx start, idx cnt)
{
    for(idx istage = cnt + start - 1; istage >= start; istage--) {
        sTabular * out_table = _commands[istage]->getOutTable();
        if( !out_table ) {
            continue;
        }

        sTabular * in_table = istage ? _commands[istage - 1]->getOutTable() : getLoadedTable(0);
        if( in_table != out_table && !isLoadedTable(out_table) ) {
            delete out_table;
        }
        _commands[istage]->setOutTable(0);
    }
}
