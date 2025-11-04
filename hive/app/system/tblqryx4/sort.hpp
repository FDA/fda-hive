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
#ifndef sLib_tblqryx4_sort_hpp
#define sLib_tblqryx4_sort_hpp

#include <slib/utils/tbltbl.hpp>
#include "rowscols.hpp"
#include "qlang.hpp"
#include "utils.hpp"

namespace slib {
    namespace tblqryx4 {
        class RowSorter
        {
        private:
            struct KeyValue {
                union {
                    idx s;
                    real r;
                } val;
                enum eKeyType {
                    eUndefined,
                    eString,
                    eReal
                } type;

                KeyValue() { reset(); }
                void reset() { val.r = 0; type = eUndefined; }
            };

            struct KeyRow {
                idx irow;
                sStr buf;

                KeyRow() { reset(-1); }
                void reset(idx irow_=0) { irow = irow_; buf.cut(0); }
            };

            sVec<KeyValue> _key_values;
            sVec<KeyRow> _key_rows;
            sVec<OutputColSource> _key_specs;
            bool _reverse;
            idx _max_cnt_each;

            sReorderedTabular * _tbl;
            ExecContext * _ctx;

            idx _comparator_call_count, _comparator_call_max;

            sVariant _key_val_tmp_result;
            bool _no_reinterpret;
            bool _err;

            static const idx _hash_bits;
            static const idx _irow_mask;

            KeyRow * ensureKeyRow(idx irow, idx preserve_irow=-sIdxMax)
            {
                idx ikey = irow & _irow_mask;
                KeyRow * key_row = _key_rows.ptr(ikey);
                if (key_row->irow == irow)
                    return key_row;

                if (key_row->irow == preserve_irow) {
                    ikey = _key_rows.dim() - 1;
                    key_row = _key_rows.ptr(ikey);
                }

                key_row->reset(irow);

                idx specs_dim = _key_specs.dim();

                for (idx ival=0; ival<specs_dim; ival++)
                    _key_values[ikey * specs_dim + ival].reset();

                return key_row;
            }

            KeyValue * ensureKeyValue(KeyRow * row, const idx irow, idx ival)
            {
                _ctx->qlangCtx().setCurInputRow(irow);
                idx specs_dim = _key_specs.dim();
                idx ikey = row - _key_rows.ptr();
                KeyValue * val = _key_values.ptr(ikey * specs_dim + ival);
                if (val->type == KeyValue::eUndefined) {
                    _key_val_tmp_result.setNull();
                    OutputColSource * key_spec = _key_specs + ival;
                    if (key_spec->getFormula()) {
                        if (unlikely(!key_spec->getFormula()->eval(_key_val_tmp_result, _ctx->qlangCtx()))) {
                            sStr err_buf;
                            _ctx->logError("Query language error when sorting at table row %" DEC ", sortkey #%" DEC ": %s", irow, ival, _ctx->qlangCtx().printError(err_buf));
                            _ctx->qlangCtx().clearError();
                            _err = true;
                        }
                    } else {
                        _tbl->val(_key_val_tmp_result, irow, key_spec->input_col, _no_reinterpret);
                    }

                    if (_key_val_tmp_result.isNumeric() || _key_val_tmp_result.isDateOrTime()) {
                        val->type = KeyValue::eReal;
                        val->val.r = _key_val_tmp_result.asReal();
                    } else {
                        val->type = KeyValue::eString;
                        val->val.s = row->buf.length();
                        _key_val_tmp_result.print(row->buf);
                        row->buf.add0();
                    }
                }


                return val;
            }

            static idx compareKeyValues(const KeyRow * key_row1, const KeyValue * val1, const KeyRow * key_row2, const KeyValue * val2)
            {
                idx ret = 0;
                if (val1->type == KeyValue::eString) {
                    const char * s1 = key_row1->buf.ptr(val1->val.s);
                    if (val2->type == KeyValue::eString) {
                        const char * s2 = key_row2->buf.ptr(val2->val.s);
                        ret = strcasecmp(s1, s2);
                    } else {
                        ret = *s1 ? -1 : 1;
                    }
                } else {
                    real r1 = val1->val.r;
                    if (val2->type == KeyValue::eString) {
                        const char * s2 = key_row2->buf.ptr(val2->val.s);
                        ret = *s2 ? 1 : -1;
                    } else {
                        real r2 = val2->val.r;
                        ret = isnan(r1) && isnan(r2) ? 0 : sSig0<real>(r1 - r2);
                    }
                }
                return ret;
            }

            static idx comparator(void * param, void * unused, idx irow1, idx irow2)
            {
                if (irow1 == irow2) {
                    return 0;
                }

                RowSorter * sorter = static_cast<RowSorter*>(param);

                if (sorter->_err) {
                    return 0;
                }

                sorter->_comparator_call_max = sMax<idx>(sorter->_comparator_call_max, sorter->_comparator_call_count);
                ExecContext::reportSubProgressStatic(sorter->_ctx, sorter->_comparator_call_count++, -1, sorter->_comparator_call_max);

                KeyRow * key_row1 = sorter->ensureKeyRow(irow1);
                KeyRow * key_row2 = sorter->ensureKeyRow(irow2, irow1);
                idx ret = 0;
                idx specs_dim = sorter->_key_specs.dim();
                for (idx ival=0; ival<specs_dim; ival++) {
                    KeyValue * val1 = sorter->ensureKeyValue(key_row1, irow1, ival);
                    KeyValue * val2 = sorter->ensureKeyValue(key_row2, irow2, ival);

                    if (sorter->_err) {
                        return 0;
                    }

                    ret = compareKeyValues(key_row1, val1, key_row2, val2);

                    if (ret) {
                        return sorter->_reverse ? -ret : ret;
                    }
                }
                if (ret == 0) {
                    ret = irow1 - irow2;
                }
                return sorter->_reverse ? -ret : ret;
            }

        public:
            RowSorter()
            {
                _reverse = false;
                _max_cnt_each = -1;
                _tbl = 0;
                _ctx = 0;
                _err = false;
                _no_reinterpret = false;
                _comparator_call_count = 0;
                _comparator_call_max = 1;
                _key_rows.resize((1 << _hash_bits) + 1);
            }

            bool init(sVariant * arg, ExecContext * ctx, const char * opname="inputsort", bool no_reinterpret=false)
            {
                _ctx = ctx;
                _no_reinterpret = no_reinterpret;

                if (!arg) {
                    _ctx->logError("Missing or invalid arg parameter for %s operation", opname);
                    return false;
                }

                if (sVariant * reverse_val = arg->getDicElt("reverse")) {
                    _reverse = reverse_val->asBool();
                }

                if (sVariant * max_cnt_each_val = arg->getDicElt("maxCountEach")) {
                    _max_cnt_each = max_cnt_each_val->asInt();
                }

                if (sVariant * fmla_val = arg->getDicElt("formula")) {
                    _key_specs.resize(1);
                    sStr fmla_msg("formula argument for %s operation", opname);
                    ast::Node * fmla = ParseUtils::parseFormulaArg(fmla_val, *_ctx, fmla_msg.ptr());
                    if (unlikely(!fmla))
                        return false;
                    _key_specs(0)->setFormula(0, fmla);
                } else if (sVariant * fmlas_val = arg->getDicElt("formulas")) {
                    idx specs_dim = fmlas_val->dim();
                    if (!specs_dim) {
                        _ctx->logError("Empty formulas argument for %s operation", opname);
                        return false;
                    }
                    _key_specs.resize(specs_dim);
                    sStr fmla_msg("formulas argument for %s operation", opname);
                    for (idx i=0; i<specs_dim; i++) {
                        ast::Node * fmla = ParseUtils::parseFormulaArg(fmlas_val->getListElt(i), *_ctx, fmla_msg.ptr());
                        if (unlikely(!fmla))
                            return false;
                        _key_specs[i].setFormula(0, fmla);
                    }
                } else {
                    _ctx->logError("Missing formula or formulas argument for %s operation", opname);
                    return false;
                }

                _key_values.resize(((1 << _hash_bits) + 1) * _key_specs.dim());
                return true;
            }

            bool sort(sReorderedTabular * tbl)
            {
                _tbl = tbl;
                if (!_key_specs.dim())
                    return true;

                for (idx is=0; is<_key_specs.dim(); is++) {
                    _key_specs[is].setFormula(tbl, _key_specs[is].getFormula());
                }

                _err = false;

                idx nrows = _tbl->rows();
                idx top_header = _tbl->dimTopHeader();

                _comparator_call_count = 0;
                _comparator_call_max = nrows * nrows;

                sVec<idx> sorted_index;
                sorted_index.resize(nrows);
                sSort::sortSimpleCallback<idx>(RowSorter::comparator, this, nrows, 0, sorted_index.ptr());

                if (_err)
                    return false;

                if (tbl->hasRowsRemapped()) {
                    sVec<sReorderedTabular::TypedSource> sorted_map;
                    sorted_map.resize(nrows + top_header);
                    for (idx absrow=0; absrow<top_header; absrow++) {
                        sorted_map[absrow] = tbl->getRowsMap()[absrow];
                    }
                    for (idx absrow=top_header; absrow < nrows + top_header; absrow++) {
                        sorted_map[absrow] = tbl->getRowsMap()[top_header + sorted_index[absrow - top_header]];
                    }
                    tbl->borrowRowsMap(sorted_map);
                } else {
                    tbl->initRowsMap();
                    for (idx absrow=top_header; absrow < nrows + top_header; absrow++) {
                        tbl->getRowsMap()[absrow].src = sorted_index[absrow - top_header];
                    }
                }

                if (_max_cnt_each > 0 && nrows) {
                    const sVec<sReorderedTabular::TypedSource> & sorted_map = tbl->getRowsMap();
                    sVec<sReorderedTabular::TypedSource> sliced_map;
                    sliced_map.resize(nrows);
                    for (idx absrow = 0; absrow <= top_header; absrow++) {
                        sliced_map[absrow] = sorted_map[absrow];
                    }
                    idx sliced_map_dim = top_header + 1;
                    idx specs_dim = _key_specs.dim();

                    KeyRow * prev_row = ensureKeyRow(sorted_map[top_header].src), * cur_row = 0;
                    sVec<KeyValue*> values;
                    values.resize(specs_dim);
                    for (idx ival=0; ival<specs_dim; ival++) {
                        values[ival] = ensureKeyValue(prev_row, sorted_map[top_header].src, ival);
                    }
                    idx cur_cnt = 1;
                    for (idx absrow = top_header + 1; absrow < nrows + top_header; absrow++) {
                        cur_row = ensureKeyRow(sorted_map[absrow].src);
                        for (idx ival=0; ival<specs_dim; ival++) {
                            KeyValue * cur_value = ensureKeyValue(cur_row, sorted_map[absrow].src, ival);
                            if (cur_cnt && !_err && compareKeyValues(cur_row, cur_value, prev_row, values[ival])) {
                                cur_cnt = 0;
                            }
                            if (cur_cnt == 0) {
                                values[ival] = cur_value;
                            }
                        }
                        if (cur_cnt < _max_cnt_each) {
                            sliced_map[sliced_map_dim++] = sorted_map[absrow];
                        }
                        prev_row = cur_row;
                        cur_cnt++;
                    }
                    sliced_map.cut(sliced_map_dim);
                    _tbl->borrowRowsMap(sliced_map);
                }
                return true;
            }
        };

    };
};
#endif
