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
#pragma once
#ifndef sLib_tblqryx4_exec_context_hpp
#define sLib_tblqryx4_exec_context_hpp

#include <slib/core/lst.hpp>
#include <slib/utils/tbl.hpp>
#include <qlang/parser.hpp>
#include <qlib/QPrideProc.hpp>

#include "qlang.hpp"

namespace slib {
    namespace tblqryx4 {
        class TblQryX4;
        class Command;

        class ExecContext
        {
            protected:
                sQPrideProc & _proc;
                idx _out_req, _waiting_for_req;
                qlang::Parser _qlang_parser;
                TblQueryContext _qlang_ctx;

            public:
                sQPrideProc & qproc() { return _proc; }
                qlang::Parser & qlangParser() { return _qlang_parser; }
                TblQueryContext & qlangCtx() { return _qlang_ctx; }
                idx reqID() const { return _proc.reqId; }
                idx outReqID() const { return _out_req; }


                idx allocateLoaderHandle();

                bool hasLoaderHandle(idx loader_handle) const;

                void requestLoadReqTable(idx loader_handle, idx dataReqID, bool isgrp, const char * tblname, const char * idx_suffix=0, const char * colsep=0, const char * commentPrefix=0, idx parseStart = 0, idx parseCnt=sIdxMax, idx initialOffset=0, idx headerOffset=-1, idx maxLen=sIdxMax);
                void requestLoadObjTable(idx loader_handle, const sHiveId & objID, const char * tblname, const char * idx_suffix=0, bool fallback_default_name=false, const char * colsep=0, const char * commentPrefix=0, idx parseStart = 0, idx parseCnt=sIdxMax, idx initialOffset=0, idx headerOffset=-1, idx maxLen=sIdxMax);
                idx allocateRequestLoadReqTables(sVec<idx> & loader_handles, idx dataReqID, bool isgrp, const char * glob);
                idx allocateRequestLoadObjTables(sVec<idx> & loader_handles, const sHiveId & objID, const char * glob, bool fallback_default_name=false);

                void requestLoadObjqryTable(idx loader_handle, qlang::ast::Node * qry_node);
                sTabular * getLoadedTable(idx loader_handle);
                bool isLoadedTable(const sTabular * tbl) const;
                sTabular * releaseLoadedTable(idx loader_handle);


                void logError(const char * fmt, ...) __attribute((format(printf, 2, 3)))
                {
                    sCallVargPara2(_proc.vreqSetInfo, _proc.reqId, sQPrideBase::eQPInfoLevel_Error, fmt);
                }
                void logWarning(const char * fmt, ...) __attribute((format(printf, 2, 3)))
                {
                    sCallVargPara2(_proc.vreqSetInfo, _proc.reqId, sQPrideBase::eQPInfoLevel_Warning, fmt);
                }
                void logInfo(const char * fmt, ...) __attribute((format(printf, 2, 3)))
                {
                    sCallVargPara(this->_proc.vlogOut, sQPrideBase::eQPLogType_Info, fmt);
                }
                void logDebug(const char * fmt, ...) __attribute((format(printf, 2, 3)))
                {
                    sCallVargPara(this->_proc.vlogOut, sQPrideBase::eQPLogType_Debug, fmt);
                }
                void logTrace(const char * fmt, ...) __attribute((format(printf, 2, 3)))
                {
                    sCallVargPara(this->_proc.vlogOut, sQPrideBase::eQPLogType_Trace, fmt);
                }


                idx reportSubProgress(idx items, idx progress, idx progressMax);
                static idx reportSubProgressStatic(void * param, idx items, idx progress, idx progressMax);

                idx curCmdIndex() { return _cur_icmd; }

            private:
                friend class TblQryX4;

                ExecContext(sQPrideProc * proc);
                ~ExecContext();

                sDic<bool> _tbl_source_strings;
                sMex _tbl_source_mex;
                struct InputTableSource {
                    enum EMode {
                        eReq,
                        eObj,
                        eObjQry
                    } mode;
                    idx data_req_id;
                    bool data_is_grp;
                    sHiveId obj_id;
                    idx tblname_index;
                    idx idx_suffix_index;
                    idx colsep_index;
                    idx comment_prefix_index;
                    idx parse_start;
                    idx parse_cnt;
                    idx initial_offset;
                    idx header_offset;
                    idx max_len;
                    qlang::ast::Node * qry_node;
                    bool fallback_default_name;

                    InputTableSource();
                };
                struct InputTable {
                    sTabular * tbl;
                    bool owned;
                    sLst<InputTableSource> sources;

                    InputTable();
                    ~InputTable();
                };
                sVec<InputTable> _in_tables;
                sVec<sVariant> _in_table_qry_results;
                bool _missing_tbl_nonfatal;

                const char * getLoaderTblSourceString(idx index) const;

                idx _progressItems;
                idx _subProgressItems;
                idx _subProgressNum;
                idx _subProgressCur;

                sVariant _tqs;
                qlang::ast::Node * _root_objQry;
                sVariant _objQryResult;

                sVec<Command*> _commands;
                idx _cur_icmd;

                bool _top_header, _left_header;
                sTabular *_out_table;

                bool parseForm();
                bool loadInput();
                bool prepareCommands();
                bool processCommands();
                bool saveResult();

                void setProgressItems(idx n) { _progressItems = n; }
                void startSubProgress();
                void endSubProgress();
                void setOutputReq(idx r) { _out_req = r; }
                idx waitingForReq() const { return _waiting_for_req; }

                bool substituteTemplate(sVariant * arg);

                void pushInputTable(idx loader_handle, sTabular * tbl, bool owned = true);
                sTxtTbl * newCSVorVCF(const char * name, const char * tblIdxPath, const char * tblDataPath, sFil * tblFil, idx tblFilTime, sTxtTbl::ParseOptions & opts);
                bool loadFileObject(idx loader_handle, InputTableSource * source);
                bool loadRequestFile(idx loader_handle, InputTableSource * source);

                bool appendTqs(const char * tqs_string, idx len = 0, idx tqsMaxCnt = sIdxMax);

                void cleanupCommandOutputs(idx start, idx cnt);
        };
    };
};

#endif
