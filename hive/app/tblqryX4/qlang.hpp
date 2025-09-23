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
#ifndef sLib_tblqryx4_qlang_hpp
#define sLib_tblqryx4_qlang_hpp

#include <slib/utils/tbl.hpp>
#include <ulib/uquery.hpp>
#include <violin/violin.hpp>
#include "rowscols.hpp"

using namespace slib::qlang;

namespace slib {
    namespace tblqryx4 {
        class TblQueryContext: public sHiveQueryContext
        {
            protected:
                sTabular * _in_table;
                const OutputColumns * _out_cols;
                idx _in_row;
                idx _out_col;
                bool _output_phase;

                sTaxIon * _tax_ion;
                bool _tax_ion_failed;
                sDic<sHiveseq*> _refSeqs;

                virtual void registerDefaultBuiltins();

            public:
                TblQueryContext(sUsr & usr)
                    : sHiveQueryContext(usr)
                {
                    _in_table = 0;
                    _out_cols = 0;
                    _output_phase = false;
                    _in_row = -1;
                    _out_col = 0;
                    _tax_ion = 0;
                    registerDefaultBuiltins();
                }
                virtual ~TblQueryContext()
                {
                    delete _tax_ion;
                    for(idx i = 0; i < _refSeqs.dim(); i++) {
                        delete _refSeqs[i];
                        _refSeqs[i] = 0;
                    }
                }

                void setTable(sTabular * in_tbl)
                {
                    _in_table = in_tbl;
                    _in_row = -1;
                }
                void setCols(const OutputColumns * out_cols)
                {
                    _out_cols = out_cols;
                }
                void setInRow(idx in_row)
                {
                    _in_row = in_row;
                }
                void setCol(idx out_col)
                {
                    _out_col = out_col;
                }
                void setObjs(const sHiveId * ids, idx num)
                {
                    sVariant objs;
                    objs.setList();

                    for(idx i = 0; i < num; i++) {
                        sVariant val;
                        val.setHiveId(ids[i]);
                        objs.push(val);
                    }
                    registerBuiltinValue("objs", objs);
                }

                inline const OutputColumns* getCols() const
                {
                    return _out_cols;
                }
                inline const sTabular* getTable() const
                {
                    return _in_table;
                }
                inline sTabular* getTable()
                {
                    return _in_table;
                }

                virtual bool evalGetDollarNumValue(sVariant & out, idx icol)
                {
                    sTabular * tbl = getTable();
                    if( likely(tbl) ) {
                        if( likely(tbl->val(out, _in_row, icol)) ) {
                            return true;
                        } else if( likely(_in_row >= -tbl->dimTopHeader() && _in_row < tbl->rows() && icol >= -tbl->dimLeftHeader() && icol < tbl->cols()) ) {
                            out.setNull();
                            return true;
                        }
                    }
                    return false;
                }

                virtual bool evalGetDollarNameValue(sVariant & out, const char * colname)
                {
                    sTabular * tbl = getTable();
                    if( likely(tbl) ) {
                        return evalGetDollarNumValue(out, tbl->colId(colname));
                    } else {
                        return false;
                    }
                }

                static bool getCurRow(sVariant & out, void * param)
                {
                    TblQueryContext * tc = static_cast<TblQueryContext*>(param);
                    if( unlikely(!tc->_in_table) ) {
                        out.setNull();
                        return false;
                    }
                    out.setInt(tc->_in_row);
                    return true;
                }

                static bool getInTableName(sVariant & out, void * param)
                {
                    TblQueryContext * tc = static_cast<TblQueryContext*>(param);
                    if( likely(tc->_in_table) ) {
                        idx in_col = tc->_out_cols ? sMax<idx>(0, tc->_out_cols->getInputCol(tc->_out_col)) : 0;
                        out.setString(tc->_in_table->getTableMetadata("name", tc->_in_row, in_col));
                    } else {
                        out.setNull();
                    }
                    return true;
                }

                static bool getInTableObj(sVariant & out, void * param)
                {
                    TblQueryContext * tc = static_cast<TblQueryContext*>(param);
                    if( likely(tc->_in_table) ) {
                        idx in_col = tc->_out_cols ? sMax<idx>(0, tc->_out_cols->getInputCol(tc->_out_col)) : 0;
                        out.setHiveId(tc->_in_table->getTableMetadata("obj", tc->_in_row, in_col));
                    } else {
                        out.setNull();
                    }
                    return true;
                }

                inline bool isOutputPhase() const
                {
                    return _output_phase;
                }
                void setOutputPhase(bool b)
                {
                    _output_phase = b;
                }

                void setCurInputRow(idx irow)
                {
                    _in_row = irow;
                }
                idx getCurInputRow() const
                {
                    return _in_row;
                }

                sHiveseq* getRefSeq(sVariant & heptagon_val)
                {
                    sHiveId heptagon_id;
                    heptagon_val.asHiveId(&heptagon_id);
                    if( sHiveseq ** psub = _refSeqs.get(&heptagon_id, sizeof(sHiveId)) ) {
                        return *psub;
                    }

                    *_refSeqs.set(&heptagon_id, sizeof(sHiveId)) = 0;

                    sUsrObj * heptagon_obj = 0;
                    if( evalUsrObj(&heptagon_obj, heptagon_val) != qlang::EVAL_SUCCESS || !heptagon_obj ) {
                        return 0;
                    }

                    sUsrType2::ensure(*_usr, "svc-profiler");
                    const sUsrType2 * utype = heptagon_obj->getType();
                    if( !utype || !utype->isDescendentOf("svc-profiler") ) {
                        return 0;
                    }

                    sVec<sHiveId> parent_proc_ids;
                    heptagon_obj->propGetHiveIds("parent_proc_ids", parent_proc_ids);
                    if( parent_proc_ids.dim() ) {
                        sUsrObj al(*_usr, parent_proc_ids[0]);
                        if( al.Id() ) {
                            sStr parentAlignmentPath;
                            al.getFilePathname00(parentAlignmentPath, "alignment.hiveal" _ "alignment.vioal" __);
                            if( parentAlignmentPath ) {
                                sHiveal hiveal(_usr);
                                sHiveseq * sub = new sHiveseq(_usr, al.propGet00("subject", 0, ";"), hiveal.parseSubMode(parentAlignmentPath));
                                *_refSeqs.set(&heptagon_id, sizeof(sHiveId)) = sub;
                                return sub;
                            }
                        }
                    }

                    return 0;
                }
        };
    }
    ;
}
;

#endif
