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
#include "tblqryX4_cmd.hpp"
#include "tblqryX4_cmd_addmissingrows.hpp"
#include "utils.hpp"

using namespace slib;
using namespace slib::qlang;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        Command * cmdAddMissingRowsFactory(ExecContext & ctx) { return new CmdAddMissingRows(ctx); }

        class sAddedRowsTabular : public sTabular {
            private:
                friend struct CmdAddMissingRows;
                sTabular * _src;
                sTxtTbl _varying_cols, _fixed_cols;
                struct RowCoord {
                    idx isrc;
                    idx ivarying;
                };
                sVec<RowCoord> _row_map;
                sVec<idx> _col_varying_map;

            public:
                sAddedRowsTabular(sTabular * src)
                {
                    _src = src;
                }

                idx rows() const { return _row_map.dim(); }
                idx cols() const { return _src->cols(); }
                sVariant::eType coltype(idx icol) const { return _src->coltype(icol); }
                bool setColtype(idx icol, sVariant::eType t) { return _src->setColtype(icol, t); }
                idx colId(const char * colname, idx colname_len=0, idx ic=0) const { return _src->colId(colname, colname_len, ic); }
                idx colIdDim(const char * colname, idx colname_len=0) const { return _src->colIdDim(colname, colname_len); }
                const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char * defValue=0, idx flags=0) const
                {
                    if( unlikely(icol < 0 || icol >= cols() || irow < 0 || irow >= rows()) ) {
                        return _src->printCell(out, irow, icol, maxLen, defValue, flags);
                    }
                    if( likely(_row_map[irow].isrc >= 0) ) {
                        return _src->printCell(out, _row_map[irow].isrc, icol, maxLen, defValue, flags);
                    }
                    if( _col_varying_map[icol] >= 0 ) {
                        return _varying_cols.printCell(out, _row_map[irow].ivarying, _col_varying_map[icol], maxLen, defValue, flags);
                    } else {
                        return _fixed_cols.printCell(out, 0, icol, maxLen, defValue, flags);
                    }
                }
                const char * cell(idx irow, idx icol, idx * cellLen) const
                {
                    if( unlikely(icol < 0 || icol >= cols() || irow < 0 || irow >= rows()) ) {
                        return _src->cell(irow, icol, cellLen);
                    }
                    if( likely(_row_map[irow].isrc >= 0) ) {
                        return _src->cell(_row_map[irow].isrc, icol, cellLen);
                    }
                    if( _col_varying_map[icol] >= 0 ) {
                        return _varying_cols.cell(_row_map[irow].ivarying, _col_varying_map[icol], cellLen);
                    } else {
                        return _fixed_cols.cell(0, icol, cellLen);
                    }
                }
                virtual ECellEncoding cellEncoding(idx irow, idx icol) const
                {
                    if( unlikely(icol < 0 || icol >= cols() || irow < 0 || irow >= rows()) ) {
                        return _src->cellEncoding(irow, icol);
                    }
                    if( likely(_row_map[irow].isrc >= 0) ) {
                        return _src->cellEncoding(_row_map[irow].isrc, icol);
                    }
                    if( _col_varying_map[icol] >= 0 ) {
                        return _varying_cols.cellEncoding(_row_map[irow].ivarying, _col_varying_map[icol]);
                    } else {
                        return _fixed_cols.cellEncoding(0, icol);
                    }
                }
                bool missing(idx irow, idx icol) const
                {
                    if( unlikely(icol < 0 || icol >= cols() || irow < 0 || irow >= rows()) ) {
                        return _src->missing(irow, icol);
                    }
                    if( likely(_row_map[irow].isrc >= 0) ) {
                        return _src->missing(_row_map[irow].isrc, icol);
                    }
                    return icol >= 0 && icol < cols();
                }
                bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const
                {
                    if( unlikely(icol < 0 || icol >= cols() || irow < 0 || irow >= rows()) ) {
                        return _src->val(out, irow, icol, noReinterpret, blankIsNull);
                    }
                    if( likely(_row_map[irow].isrc >= 0) ) {
                        return _src->val(out, _row_map[irow].isrc, icol, noReinterpret, blankIsNull);
                    }
                    if( _col_varying_map[icol] >= 0 ) {
                        return _varying_cols.val(out, _row_map[irow].ivarying, _col_varying_map[icol], noReinterpret, blankIsNull);
                    } else {
                        return _fixed_cols.val(out, 0, icol, noReinterpret, blankIsNull);
                    }
                }
                idx dimTopHeader() const
                {
                    return _src->dimTopHeader();
                }
                idx dimLeftHeader() const
                {
                    return _src->dimLeftHeader();
                }
                const char * getTableMetadata(const char * name, idx irow=0, idx icol=0) const
                {
                    if( unlikely(icol < 0 || icol >= cols() || irow < 0 || irow >= rows()) ) {
                        return _src->getTableMetadata(name, irow, icol);
                    }
                    if( likely(_row_map[irow].isrc >= 0) ) {
                        return _src->getTableMetadata(name, _row_map[irow].isrc, icol);
                    }
                    return _src->getTableMetadata(name, 0, 0);
                }
                bool setTableMetadata(const char * name, const char * value, idx irow=0, idx icol=0)
                {
                    if( unlikely(icol < 0 || icol >= cols() || irow < 0 || irow >= rows()) ) {
                        return _src->setTableMetadata(name, value, irow, icol);
                    }
                    if( likely(_row_map[irow].isrc >= 0) ) {
                        return _src->setTableMetadata(name, value, _row_map[irow].isrc, icol);
                    }
                    return _src->setTableMetadata(name, value, 0, 0);
                }
        };

    }
}

bool CmdAddMissingRows::parseParam(CmdAddMissingRows::ParsedParam & param, const char * name, sVariant * arg)
{
    param.val.setNull();
    param.fmla = 0;

    if( !arg ) {
        return true;
    }

    if( arg->isScalar() ) {
        param.val = *arg;
        return true;
    }

    if( arg->isDic() ) {
        if( sVariant * fmla_arg = arg->getDicElt("formula") ) {
            if( unlikely(!_ctx.qlangParser().parse(fmla_arg->asString(), 0, qlang::Parser::fDollarValues)) ) {
                _ctx.logError("addMissingRows command: invalid fmla in %s : %s", name, _ctx.qlangParser().getErrorStr());
                return false;
            }
            param.fmla = _ctx.qlangParser().releaseAstRoot();
            return true;
        }
    }
    _ctx.logError("addMissingRows command: invalid %s arg", name);
    return false;
}

bool CmdAddMissingRows::evalParam(CmdAddMissingRows::ParsedParam & param, const char * name)
{
    if( param.fmla && !param.fmla->eval(param.val, _ctx.qlangCtx()) ) {
        sStr log_buf;
        _ctx.logError("addMissingRows command: failed to evaluate fmla in %s: %s", name, _ctx.qlangCtx().printError(log_buf));
        _ctx.qlangCtx().clearError();
        return false;
    }
    return true;
}


bool CmdAddMissingRows::init(const char * op_name, sVariant * tqs_arg)
{
    if( !tqs_arg ) {
        _ctx.logError("addMissingRows command: missing arg");
        return false;
    }

    if( sVariant * abscissa_arg = tqs_arg->getDicElt("abscissa") ) {
        if( !(_abscissa_col_arg = abscissa_arg->getDicElt("col")) ) {
            _ctx.logError("addMissingRows command: missing abscissa col arg");
            return false;
        }
        if( !parseParam(_abscissa_max_gap, "abscissa maxGap arg", abscissa_arg->getDicElt("maxGap")) ||
            !parseParam(_abscissa_min_value, "abscissa minValue arg", abscissa_arg->getDicElt("minValue")) ||
            !parseParam(_abscissa_max_value, "abscissa maxValue arg", abscissa_arg->getDicElt("maxValue")) )
        {
            return false;
        }

        if( sVariant * abscissa_dense_val = abscissa_arg->getDicElt("dense") ) {
            _abscissa_dense = abscissa_dense_val->asBool();
        }
    } else {
        _ctx.logError("addMissingRows command: missing abscissa arg");
        return false;
    }


    sVariant * add_arg = tqs_arg->getDicElt("add");
    if( add_arg ) {
        if( add_arg->isList() ) {
            sStr param_desc;
            for(idx i=0; i<add_arg->dim(); i++) {
                bool is_default = false;
                if( sVariant * col_arg = add_arg->getListElt(i)->getDicElt("col") ) {
                    if( col_arg->isString() && strcmp(col_arg->asString(), "*") == 0 ) {
                        is_default = true;
                    } else {
                        *_add_cols_args.add(1) = col_arg;
                    }
                } else {
                    _ctx.logError("addMissingRows command: missing col key in add arg element %" DEC, i);
                    return false;
                }
                if( sVariant * value_arg = add_arg->getListElt(i)->getDicElt("value") ) {
                    ParsedParam & param = is_default ? _add_default_value : *_add_values.add(1);
                    param_desc.printf(0, "value in add arg element %" DEC, i);
                    if( !parseParam(param, param_desc.ptr(), value_arg) ) {
                        return false;
                    }
                } else {
                    _ctx.logError("addMissingRows command: missing value key in add arg element %" DEC, i);
                    return false;
                }
            }
        } else {
            _ctx.logError("addMissingRows command: add arg is expected to be a list of objects");
            return false;
        }
    }

    return true;
}

static idx estimateAbscissaMaxGap(sTabular * tbl, idx icol)
{
    sDic<idx> gaps;
    idx nrows = sMin<idx>(tbl->rows() - 1, 10000);
    for(idx ir=0; ir<nrows; ir++) {
        idx gap = sAbs(tbl->ival(ir + 1, icol) - tbl->ival(ir, icol));
        if( idx * cnt = gaps.get(&gap, sizeof(gap)) ) {
            (*cnt)++;
        } else {
            *gaps.set(&gap, sizeof(gap)) = 1;
        }
    }
    idx mode_gaps = 1;
    idx mode_gaps_cnt = 0;
    for(idx i=0; i<gaps.dim(); i++) {
        idx cnt = *gaps.ptr(i);
        if( cnt > mode_gaps_cnt ) {
            mode_gaps_cnt = cnt;
            mode_gaps = *static_cast<idx*>(gaps.id(i));
        }
    }
    return sMax<idx>(1, 2 * mode_gaps - 1);
}

static idx findStartEndRow(sTabular * tbl, idx icol, bool is_descending, idx val, bool is_start)
{
    if( tbl->rows() == 0 ) {
        return -sIdxMax;
    }

    idx imin = 0;
    idx imax = tbl->rows() - 1;
    idx ibest = -sIdxMax;

    while( imax >= imin ) {
        idx imid = (imin + imax) / 2;
        idx res = tbl->ival(imid, icol);
        idx next_search;

        if( is_start ) {
            if( is_descending ) {
                if( res <= val ) {
                    ibest = imid;
                    next_search = -1;
                } else {
                    next_search = 1;
                }
            } else {
                if( res >= val ) {
                    ibest = imid;
                    next_search = -1;
                } else {
                    next_search = 1;
                }
            }
        } else {
            if( is_descending ) {
                if( res >= val ) {
                    ibest = imid;
                    next_search = 1;
                } else {
                    next_search = -1;
                }
            } else {
                if( res <= val ) {
                    ibest = imid;
                    next_search = 1;
                } else {
                    next_search = -1;
                }
            }
        }

        if( next_search < 0 ) {
            imax = imid - 1;
        } else {
            imin = imid + 1;
        }
    }

    return ibest;
}

void CmdAddMissingRows::addMissingRow(sAddedRowsTabular * out, idx abscissa_value, const sVec<idx> & varying2iadd, idx non_abscissa_varying_cols, sVariant & tmp_val)
{
    sAddedRowsTabular::RowCoord * coord = out->_row_map.add(1);
    coord->isrc = -sIdxMax;
    coord->ivarying = out->_varying_cols.rows();

    out->_varying_cols.addICell(abscissa_value);

    if( non_abscissa_varying_cols ) {
        _cur_abscissa_val = abscissa_value;
        for(idx iia=0; iia < non_abscissa_varying_cols; iia++) {
            if( !_add_values[varying2iadd[iia]].fmla->eval(tmp_val, _ctx.qlangCtx()) ) {
                tmp_val.setNull();
                _ctx.qlangCtx().clearError();
            }
            out->_varying_cols.addCell(tmp_val);
        }
    }

    out->_varying_cols.addEndRow();
}

void CmdAddMissingRows::addMissingRowSpan(sAddedRowsTabular * out, idx abscissa_first, bool include_first, idx abscissa_last, bool include_last, idx abscissa_max_gap, const sVec<idx> & varying2iadd, idx non_abscissa_varying_cols, sVariant & tmp_val)
{
    if( abscissa_first == abscissa_last && (!include_first || !include_last) ) {
        return;
    }

    bool omit_first = false, omit_last = false;
    if( _abscissa_descending ) {
        if( abscissa_first < abscissa_last ) {
            return;
        }

        if( include_first && !include_last ) {
            abscissa_last += abscissa_max_gap;
            if( abscissa_last >= abscissa_first ) {
                omit_last = true;
            }
        } else if( !include_first && include_last ) {
            abscissa_first -= abscissa_max_gap;
            if( abscissa_last >= abscissa_first ) {
                omit_first = true;
            }
        } else if( !include_first && !include_last ) {
            abscissa_first -= abscissa_max_gap;
            abscissa_last += abscissa_max_gap;
            if( abscissa_last > abscissa_first ) {
                return;
            } else if( abscissa_last == abscissa_first ) {
                omit_last = true;
            }
        }

        if( !omit_first ) {
            addMissingRow(out, abscissa_first, varying2iadd, non_abscissa_varying_cols, tmp_val);
        }

        if( _abscissa_dense ) {
            for(idx abscissa_cur = abscissa_first - abscissa_max_gap; abscissa_cur > abscissa_last; abscissa_cur -= abscissa_max_gap) {
                addMissingRow(out, abscissa_cur, varying2iadd, non_abscissa_varying_cols, tmp_val);
            }
        }

        if( !omit_last ) {
            addMissingRow(out, abscissa_last, varying2iadd, non_abscissa_varying_cols, tmp_val);
        }
    } else {
        if( abscissa_first > abscissa_last ) {
            return;
        }

        if( include_first && !include_last ) {
            abscissa_last -= abscissa_max_gap;
            if( abscissa_last <= abscissa_first ) {
                omit_last = true;
            }
        } else if( !include_first && include_last ) {
            abscissa_first += abscissa_max_gap;
            if( abscissa_last <= abscissa_first ) {
                omit_first = true;
            }
        } else if( !include_first && !include_last ) {
            abscissa_first += abscissa_max_gap;
            abscissa_last -= abscissa_max_gap;
            if( abscissa_last < abscissa_first ) {
                return;
            } else if( abscissa_last == abscissa_first ) {
                omit_last = true;
            }
        }

        if( !omit_first ) {
            addMissingRow(out, abscissa_first, varying2iadd, non_abscissa_varying_cols, tmp_val);
        }

        if( _abscissa_dense ) {
            for(idx abscissa_cur = abscissa_first + abscissa_max_gap; abscissa_cur < abscissa_last; abscissa_cur += abscissa_max_gap) {
                addMissingRow(out, abscissa_cur, varying2iadd, non_abscissa_varying_cols, tmp_val);
            }
        }

        if( !omit_last ) {
            addMissingRow(out, abscissa_last, varying2iadd, non_abscissa_varying_cols, tmp_val);
        }
    }
}

bool CmdAddMissingRows::compute(sTabular * in_table)
{
    _ctx.qlangCtx().registerBuiltinIdxPtr("cur_abscissa_val", &_cur_abscissa_val, true);

    if (!in_table) {
        _ctx.logError("addMissingRows command: internal error, no input table");
        return 0;
    }
    if( in_table->cols() < 1 || in_table->rows() < 1 ) {
        setOutTable(in_table);
        return true;
    }

    sVec<idx> tmp_vec;
    idx abscissa_col;
    if( ParseUtils::parseColsArg(tmp_vec, _abscissa_col_arg, in_table, _ctx, false) && tmp_vec.dim() == 1 ) {
        abscissa_col = tmp_vec[0];
        tmp_vec.cut(0);
    } else {
        _ctx.logError("addMissingRows command: invalid abscissa col arg");
        return 0;
    }

    if( !evalParam(_abscissa_max_gap, "abscissa maxGap arg") ) {
        return 0;
    }
    idx abscissa_max_gap = _abscissa_max_gap.val.asInt() > 0 ? _abscissa_max_gap.val.asInt() : estimateAbscissaMaxGap(in_table, abscissa_col);
    _ctx.logDebug("addMissingRows command: using %" DEC " for abscissa maxGap", abscissa_max_gap);

    if( !evalParam(_abscissa_min_value, "abscissa start arg") || !evalParam(_abscissa_max_value, "abscissa end arg") ) {
        return 0;
    }

    bool abscissa_descending = in_table->ival(0, abscissa_col) > in_table->ival(in_table->rows() - 1, abscissa_col);
    idx abscissa_start, abscissa_end;
    if( abscissa_descending ) {
        abscissa_start = _abscissa_max_value.val.isNull() ? in_table->ival(0, abscissa_col) : _abscissa_max_value.val.asInt();
        abscissa_end = _abscissa_min_value.val.isNull() ? in_table->ival(in_table->rows() - 1, abscissa_col) : _abscissa_min_value.val.asInt();
    } else {
        abscissa_start = _abscissa_min_value.val.isNull() ? in_table->ival(0, abscissa_col) : _abscissa_min_value.val.asInt();
        abscissa_end = _abscissa_max_value.val.isNull() ? in_table->ival(in_table->rows() - 1, abscissa_col) : _abscissa_max_value.val.asInt();
    }

    _ctx.logDebug("addMissingRows command: using %" DEC " for abscissa start value", abscissa_start);
    _ctx.logDebug("addMissingRows command: using %" DEC " for abscissa end value", abscissa_end);

    _cur_abscissa_val = abscissa_start;

    if( !evalParam(_add_default_value, "value in add default arg") ) {
        return 0;
    }

    sAddedRowsTabular * out = new sAddedRowsTabular(in_table);
    out->_col_varying_map.resize(in_table->cols());

    sVec<idx> out2iadd;
    out2iadd.resize(in_table->cols());
    for(idx ic=0; ic<in_table->cols(); ic++) {
        out->_col_varying_map[ic] = -sIdxMax;
        out2iadd[ic] = -sIdxMax;
    }

    for(idx i=0; i<_add_cols_args.dim(); i++) {
        if( ParseUtils::parseColsArg(tmp_vec, _add_cols_args[i], in_table, _ctx, false) && tmp_vec.dim() == 1 ) {
            if( tmp_vec[0] >= 0 && tmp_vec[0] < in_table->cols() ) {
                out2iadd[tmp_vec[0]] = i;
            } else {
                _ctx.logDebug("addMissingRows command: skipping add column argument '%s' - column not found in table\n", _add_cols_args[i]->asString());
            }
            tmp_vec.cut(0);
        } else {
            _ctx.logDebug("addMissingRows command: skipping add column argument '%s' - column not found or not unique in table\n", _add_cols_args[i]->asString());
        }
    }

    idx non_abscissa_varying_cols = 0;
    sVec<idx> varying2iadd;
    out->_fixed_cols.initWritable(in_table->cols(), sTblIndex::fSaveRowEnds);
    for(idx ic=0; ic<in_table->cols(); ic++) {
        sVariant fixed_val;
        if( ic == abscissa_col ) {
            out->_col_varying_map[abscissa_col] = 0;
        } else if( out2iadd[ic] < 0 ) {
            fixed_val = _add_default_value.val;
        } else if( !_add_values[out2iadd[ic]].fmla ) {
            fixed_val = _add_values[out2iadd[ic]].val;
        } else {
            out->_col_varying_map[ic] = 1 + non_abscissa_varying_cols++;
            *varying2iadd.add(1) = out2iadd[ic];
        }
        out->_fixed_cols.addCell(fixed_val);
    }
    if( non_abscissa_varying_cols + 1 < in_table->cols() ) {
        out->_fixed_cols.addEndRow();
    }
    out->_fixed_cols.finish();

    out->_varying_cols.initWritable(non_abscissa_varying_cols + 1, sTblIndex::fSaveRowEnds);

    idx start_row = findStartEndRow(in_table, abscissa_col, abscissa_descending, abscissa_start, true);
    idx end_row = findStartEndRow(in_table, abscissa_col, abscissa_descending, abscissa_end, false);

    _ctx.logDebug("addMissingRows command: using %" DEC " for abscissa start row", start_row);
    _ctx.logDebug("addMissingRows command: using %" DEC " for abscissa end row", end_row);

    if( start_row != -sIdxMax && end_row != -sIdxMax ) {
        sVariant varying_val;
        idx start_val = in_table->ival(start_row, abscissa_col);
        idx end_val = in_table->ival(end_row, abscissa_col);

        addMissingRowSpan(out, abscissa_start, true, start_val, false, abscissa_max_gap, varying2iadd, non_abscissa_varying_cols, varying_val);

        for(idx ir=start_row; ir<=end_row; ir++) {
            sAddedRowsTabular::RowCoord * coord = out->_row_map.add(1);
            coord->isrc = ir;
            coord->ivarying = -sIdxMax;

            if( ir < end_row ) {
                idx cur_val = in_table->ival(ir, abscissa_col);
                idx next_val = in_table->ival(ir + 1, abscissa_col);
                addMissingRowSpan(out, cur_val, false, next_val, false, abscissa_max_gap, varying2iadd, non_abscissa_varying_cols, varying_val);
            }
        }

        addMissingRowSpan(out, end_val, false, abscissa_end, true, abscissa_max_gap, varying2iadd, non_abscissa_varying_cols, varying_val);
    }

    if( out->_varying_cols.rows() || start_row != 0 || end_row != in_table->rows() - 1 ) {
        out->_varying_cols.finish();
        setOutTable(out);
    } else {
        delete out;
        setOutTable(in_table);
    }

    return true;
}
