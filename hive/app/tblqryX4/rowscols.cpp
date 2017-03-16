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

#include <assert.h>

#include <slib/utils/sort.hpp>

#include "rowscols.hpp"
#include "utils.hpp"

using namespace slib;
using namespace slib::qlang;
using namespace slib::tblqryx4;

void OutputColSource::setFormula(const sTabular * in_table, const ast::Node * fmla)
{
    // is this formula a simple $n / ${name} expression? Then we can simplify
    const char * input_col_name = 0;
    if( in_table && fmla && fmla->isDollarCall(&input_col_name, &input_col) ) {
        if( input_col_name ) {
            input_col = in_table->colId(input_col_name);
        }

        if( shared_fmla && shared_fmla->fmla != fmla ) {
            delete fmla;
        }
        SharedFmla::decref(shared_fmla);
        shared_fmla = 0;
    } else {
        input_col = -sIdxMax;
        if( !shared_fmla || shared_fmla->fmla != fmla ) {
            SharedFmla::decref(shared_fmla);
            shared_fmla = new SharedFmla(fmla, 1);
        }
    }
}

OutputColumns::OutputColumns()
{
    _need_update_in2out = false;
    _src_start = 0;
    regcomp(&_format_checker, "^[ #0+-]*([-0-9][0-9]+)?(\\.[0-9]+)?([diufFeEgGxXo])$", REG_EXTENDED | REG_NOSUB);
}

OutputColumns::~OutputColumns()
{
    regfree(&_format_checker);
    for(idx i = 0; i < _savedCols.dim(); i++) {
        static_cast<OutputCol *>(_savedCols.id(i))->src.reset();
    }
}

bool OutputColumns::setFormat(idx icol, const char * fmt)
{
    if( !validCol(icol) )
        return false;

    if( !fmt || !*fmt ) {
        _cols[icol].ifmt_type = sVariant::value_NULL;
        _cols[icol].fmt_offset = -1;
        return true;
    }

    if( regexec(&_format_checker, fmt, 0, 0, 0) != 0 ) {
        // unsupported format
        _cols[icol].ifmt_type = sVariant::value_NULL;
        _cols[icol].fmt_offset = -1;
        return false;
    }

    idx len = sLen(fmt);

    switch(fmt[len - 1]) {
        case 'd':
        case 'i':
            _cols[icol].ifmt_type = sVariant::value_INT;
            _cols[icol].fmt_offset = _formats.length();
            _formats.add("%", 1);
            _formats.add(fmt, len - 1);
#ifdef SLIB64
            _formats.add("ll", 2);
#endif
            _formats.add(fmt + len - 1, 2);
            break;
        case 'u':
        case 'x':
        case 'X':
        case 'o':
            _cols[icol].ifmt_type = sVariant::value_UINT;
            _cols[icol].fmt_offset = _formats.length();
            _formats.add("%", 1);
            _formats.add(fmt, len - 1);
#ifdef SLIB64
            _formats.add("ll", 2);
#endif
            _formats.add(fmt + len - 1, 2);
            break;
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            _cols[icol].ifmt_type = sVariant::value_REAL;
            _cols[icol].fmt_offset = _formats.length();
            _formats.add("%", 1);
            _formats.add(fmt);
            break;
        default:
            return false;
    }

    return true;
}

void OutputColumns::init(const sTabular * in_table, const char * rangeSet)
{
    sVec<idx> scan;
    sStr buf;

    _src_start = -in_table->dimLeftHeader();
    idx src_end = in_table->cols();
    idx src_cnt = src_end - _src_start;
    static sStr namebuf;
    _in2out.resize(src_cnt);
    if( rangeSet && *rangeSet ) {
        // set of *input* column IDs
        for(idx icol = 0; icol < src_cnt; icol++) {
            _in2out[icol] = -sIdxMax;
        }
        sString::scanRangeSet(rangeSet, 0, &scan, 0, 0, 0);
        for(idx i = 0; i < scan.dim(); i++) {
            if( scan[i] >= _src_start && scan[i] < src_end ) {
                buf.cut(0);
                in_table->printTopHeader(buf, scan[i]);
                _cols.add(1);
                idx icol = _cols.dim() - 1;
                renameCol(icol, buf.ptr());
                _cols[icol].src.input_col = scan[i];
                _in2out[scan[i] - _src_start] = icol;
                _need_update_in2out = false;
            }
        }
    } else {
        _cols.resize(src_cnt);
        for(idx icol = 0; icol < src_cnt; icol++) {
            idx input_col = icol + _src_start;
            buf.cut(0);
            in_table->printTopHeader(buf, input_col);
            renameCol(icol, buf.ptr());
            _cols[icol].src.input_col = input_col;
            _in2out[icol] = icol;
            _need_update_in2out = false;
        }
    }
}

void OutputColumns::setMinMax(const char * minmaxSet/* = 0*/, const char * minmaxMain/* = 0*/)
{
    sVec<idx> scan;

    if( minmaxSet && *minmaxSet ) {
        // set of *output* column IDs
        scan.cut(0);
        sString::scanRangeSet(minmaxSet, 0, &scan, 0, 0, 0);
        for(idx i = 0; i < scan.dim(); i++) {
            if( scan[i] >= 0 && scan[i] < _cols.dim() ) {
                _cols[scan[i]].minmax_status = OutputCol::minmax_BASIC;
            }
        }
    }

    if( minmaxMain && *minmaxMain ) {
        // *output* column ID
        idx icol = atoidx(minmaxMain);
        if( icol >= 0 && icol < _cols.dim() ) {
            _cols[icol].minmax_status = OutputCol::minmax_MAIN;
        }
    }
}

bool OutputColumns::renameCol(idx icol, const char * name)
{
    if( !validCol(icol) )
        return false;

    // clear the old name
    idx prev_iname = _cols[icol].iname;
    if( prev_iname >= 0 ) {
        // clear the old name
        idx first_homonym = -1;
        if( _cols[icol].prev_homonym < 0 && _cols[icol].next_homonym >= 0 ) {
            first_homonym = _cols[icol].next_homonym;
        }
        _colNames[prev_iname] = first_homonym;
    }

    if( !name )
        name = "";

    idx *pfirst_homonym = _colNames.get(name);
    if( pfirst_homonym && *pfirst_homonym >= 0 ) {
        // this name is already used by another column
        _cols[icol].iname = _cols[*pfirst_homonym].iname;
        idx prev_homonym = *pfirst_homonym;
        while( prev_homonym < icol && _cols[prev_homonym].next_homonym >= 0 && _cols[prev_homonym].next_homonym < icol )
            prev_homonym = _cols[prev_homonym].next_homonym;

        _cols[icol].prev_homonym = prev_homonym;
        _cols[icol].next_homonym = _cols[prev_homonym].next_homonym;
        if( _cols[prev_homonym].next_homonym >= 0 ) {
            _cols[_cols[prev_homonym].next_homonym].prev_homonym = icol;
        }
        _cols[prev_homonym].next_homonym = icol;
    } else {
        *(_colNames.setString(name, 0, &(_cols[icol].iname))) = icol;
        _cols[icol].prev_homonym = -1;
        _cols[icol].next_homonym = -1;
    }

    return true;
}

bool OutputColumns::deleteCol(idx icol)
{
    if( icol < 0 || icol >= _cols.dim() )
        return false;

    OutputCol deleted_col;
    deleted_col.swap(_cols[icol]);

    for(idx i = 0; i < _cols.dim() - 1; i++) {
        if( i >= icol ) {
            _cols[i].swap(_cols[i + 1]);

            if( _cols[i].prev_homonym == icol ) {
                _cols[i].prev_homonym = deleted_col.prev_homonym;
            } else if( _cols[i].prev_homonym > icol ) {
                _cols[i].prev_homonym--;
            } else if( _cols[i].prev_homonym < 0 ) {
                _colNames[_cols[i].iname] = i;
            }
        }
        if( _cols[i].next_homonym == icol ) {
            _cols[i].next_homonym = deleted_col.next_homonym;
        } else if( _cols[i].next_homonym > icol ) {
            _cols[i].next_homonym--;
        }
    }

    for(idx i = 0; i < _in2out.dim(); i++) {
        if( _in2out[i] == icol )
            _in2out[i] = -sIdxMax;
    }

    _cols.cut(_cols.dim() - 1);
    _colNames[deleted_col.iname] = deleted_col.next_homonym;
    _need_update_in2out = true;

    return true;
}

bool OutputColumns::insertCol(idx icol, const sTabular * in_table, const char * name, const ast::Node * fmla, sVariant::eType itype)
{
    if( icol == _cols.dim() ) {
        appendCol(name, in_table, fmla);
        return true;
    }

    if( !validInsertPosition(icol) )
        return false;

    _cols.add(1);

    for(idx i = _cols.dim() - 1; i >= 0; i--) {
        if( i > icol ) {
            _cols[i].swap(_cols[i - 1]);
            if( _cols[i].prev_homonym < 0 ) {
                _colNames[_cols[i].iname] = i;
            }
        }
        if( i != icol ) {
            if( _cols[i].prev_homonym >= icol ) {
                _cols[i].prev_homonym++;
            }
            if( _cols[i].next_homonym >= icol ) {
                _cols[i].next_homonym++;
            }
        }
    }

    renameCol(icol, name);
    _cols[icol].src.setFormula(in_table, fmla);
    _cols[icol].itype = itype;
    _need_update_in2out = true;
    return true;
}

bool OutputColumns::moveCol(idx icol_from, idx icol_to)
{
    if( !validCol(icol_from) || !validCol(icol_to) )
        return false;

    if( icol_from == icol_to )
        return true;

    if( icol_from < icol_to ) {
        for(idx icol = icol_from; icol < icol_to; icol++) {
            idx icol2 = icol + 1;
            idx input_col = _cols[icol].src.input_col;
            idx input_col2 = _cols[icol2].src.input_col;
            _cols[icol].swap(_cols[icol2]);
            if( input_col >= 0 )
                _in2out[input_col] = icol2;
            if( input_col2 >= 0 )
                _in2out[input_col2] = icol;
            if( _cols[icol].iname == _cols[icol2].iname ) {
                _cols[icol2].next_homonym = _cols[icol].next_homonym;
                _cols[icol].next_homonym = icol2;
                _cols[icol].prev_homonym = _cols[icol2].prev_homonym;
                _cols[icol2].prev_homonym = icol;
            }
        }
    } else {
        for(idx icol = icol_from; icol > icol_to; icol--) {
            idx icol2 = icol - 1;
            idx input_col = _cols[icol].src.input_col;
            idx input_col2 = _cols[icol2].src.input_col;
            _cols[icol].swap(_cols[icol2]);
            if( input_col >= 0 )
                _in2out[input_col] = icol2;
            if( input_col2 >= 0 )
                _in2out[input_col2] = icol;
            if( _cols[icol].iname == _cols[icol2].iname ) {
                _cols[icol2].prev_homonym = _cols[icol].prev_homonym;
                _cols[icol].prev_homonym = icol2;
                _cols[icol].next_homonym = _cols[icol2].next_homonym;
                _cols[icol2].next_homonym = icol;
            }
        }
    }

    _need_update_in2out = true;

    return true;
}

idx OutputColumns::colIndex(const char * name, idx iname) const
{
    if( !name )
        name = "";

    const idx * picol = _colNames.get(name);
    if( !picol )
        return -1;

    idx icol = *picol;
    if( icol < 0 )
        return -1;

    while( iname > 0 && _cols[icol].next_homonym >= 0 ) {
        icol = _cols[icol].next_homonym;
        iname--;
    }
    return icol;
}

idx OutputColumns::colIndices(sVec<idx> & out, const char * name) const
{
    out.cut(0);

    if( !name )
        name = "";

    const idx * picol = _colNames.get(name);
    if( !picol )
        return 0;

    for(idx icol = *picol; icol >= 0; icol = _cols[icol].next_homonym) {
        *out.add() = icol;
        icol = _cols[icol].next_homonym;
    }
    return out.dim();
}

void OutputColumns::updateIn2Out() const
{
    sVec<idx> in2out_last;
    in2out_last.resize(_in2out.dim());

    for(idx i = 0; i < _in2out.dim(); i++) {
        _in2out[i] = in2out_last[i] = -sIdxMax;
    }

    for(idx icol = 0; icol < _cols.dim(); icol++) {
        idx input_col = _cols[icol].src.input_col;
        if( input_col < _src_start ) {
            continue;
        } else if( _in2out[input_col - _src_start] < 0 ) {
            _in2out[input_col - _src_start] = in2out_last[input_col - _src_start] = icol;
        } else {
            _cols[in2out_last[input_col - _src_start]].next_synonym = icol;
            in2out_last[input_col - _src_start] = icol;
        }
    }
}

bool OutputColumns::getOutputCols(sVec<idx> & icols, idx input_col) const
{
    if( _need_update_in2out ) {
        updateIn2Out();
        _need_update_in2out = false;
    }

    if( input_col < 0 || input_col >= _in2out.dim() ) {
        return false;
    }

    idx icol = _in2out[input_col];
    if( icol < 0 ) {
        return false;
    }

    for(; icol >= 0; icol = _cols[icol].next_synonym) {
        *icols.add(1) = icol;
    }
    return true;
}

OutputColumns::saveHandle OutputColumns::saveColumn(idx icol)
{
    if( !validCol(icol) ) {
        return 0;
    }
    idx saveIndex;
    // save only the source and type/format info
    OutputCol saved_col;
    saved_col.src = _cols[icol].src;
    SharedFmla::incref(saved_col.src.shared_fmla); // don't delete the formula when saved_col goes out of scope and gets destructed
    saved_col.itype = _cols[icol].itype;
    saved_col.ifmt_type = _cols[icol].ifmt_type;
    saved_col.fmt_offset = _cols[icol].fmt_offset;
    (*_savedCols.set(&saved_col, sizeof(saved_col), &saveIndex))++;
    return saveIndex + 1;
}

#define SPACES "    "

const char * OutputColumns::printDump(sStr & out, bool withType, const char * indent)
{
    idx start = out.length();
    if( !indent ) {
        indent = sStr::zero;
    }

    out.printf("{\n%s"SPACES"\"cols\": [\n", indent);
    for(idx icol = 0; icol < _cols.dim(); icol++) {
        out.printf("%s" SPACES SPACES "{ \"icol\": %"DEC", \"name\": ", indent, icol);
        const char * name = getName(icol);
        if( name ) {
            sString::escapeForJSON(out, name);
        } else {
            out.addString("null");
        }
        out.addString(", \"src\": ");
        if( getFormula(icol) ) {
            out.addString("\"formula\"");
        } else {
            out.printf("%"DEC, getInputCol(icol));
        }
        if( withType ) {
            out.printf(", \"type\": \"%s\"", sVariant::getTypeName(getType(icol)));
        }
        if( getFormatType(icol) ) {
            out.printf(", \"fmt_type\": \"%s\"", sVariant::getTypeName(getFormatType(icol)));
        }
        if( getFormat(icol) ) {
            out.addString(", \"fmt\": ");
            sString::escapeForJSON(out, getFormat(icol));
        }
        if( _cols[icol].minmax_status ) {
            out.addString(", \"minmax\": true");
            if( _cols[icol].minmax_status == OutputCol::minmax_MAIN ) {
                out.addString(", \"minmaxMain\": true");
            }
        }
        out.addString("}");
        out.addString(icol + 1 < _cols.dim() ? ",\n" : "\n");
    }
    out.printf("%s" SPACES "],\n", indent);
    out.printf("%s"SPACES"\"saved\": [\n", indent);
    for(saveHandle keep = minSavedCol(); keep <= maxSavedCol(); keep++) {
        out.printf("%s" SPACES SPACES "{ \"handle\": %"DEC", \"src\": ", indent, keep);
        if( getFormula(-sIdxMax, keep) ) {
            out.addString("\"formula\"");
        } else {
            out.printf("%"DEC, getInputCol(-sIdxMax, keep));
        }
        if( getType(-sIdxMax, keep) != sVariant::value_NULL ) {
            out.printf(", \"type\": \"%s\"", sVariant::getTypeName(getType(-sIdxMax, keep)));
        }
        if( getFormatType(-sIdxMax, keep) ) {
            out.printf(", \"fmt_type\": \"%s\"", sVariant::getTypeName(getFormatType(-sIdxMax, keep)));
        }
        if( getFormat(-sIdxMax, keep) ) {
            out.addString(", \"fmt\": ");
            sString::escapeForJSON(out, getFormat(-sIdxMax, keep));
        }
        out.printf(", \"refs\": %"DEC"}", *_savedCols.ptr(keep - 1));
        out.addString(keep < maxSavedCol() ? ",\n" : "\n");
    }
    out.printf("%s" SPACES "]\n%s}", indent, indent);
    return out.ptr(start);
}

void OutputBuffer::init(OutputColumns * out_cols, const sTabular * in_table, tblqryx4::ExecContext * ctx)
{
    _out_cols = out_cols;
    _in_table = in_table;
    _ctx = ctx;
    _cols = _out_cols->dim();

    // set up association between column indices and source indices
    _icol2saved.resize(_cols);
    for(idx icol=0; icol < _cols; icol++) {
        _icol2saved[icol] = out_cols->saveColumn(icol);
    }
    _saved = 1 + out_cols->maxSavedCol();

    reset();
}

static void printFormatedValue(sStr & out, const sVariant & val, const char * fmt, idx itype)
{
    switch(itype) {
        case sVariant::value_INT:
            out.printf(fmt, val.asInt());
            break;
        case sVariant::value_UINT:
            out.printf(fmt, val.asUInt());
            break;
        case sVariant::value_REAL:
            out.printf(fmt, val.asReal());
            break;
    }
}

bool OutputBuffer::setCell(idx icol, OutputColumns::saveHandle keep, idx in_row)
{
    Cell & c = _cells[keep];
    c.pos = _buf.length();
    c.in_row = in_row;
    c.in_col = -sIdxMax;
    c.lazy_val_from_in_table = false;

    if( const ast::Node * fmla = _out_cols->getFormula(icol, keep) ) {
        if( unlikely(!fmla->eval(c.val, _ctx->qlangCtx())) ) {
            sStr err_buf;
            _ctx->qlangCtx().printError(err_buf);
            if( icol == -sIdxMax ) {
                _ctx->logInfo("Formula failed on input row %"DEC": %s", in_row, err_buf.ptr());
            } else {
                _ctx->logInfo("Formula failed for column %"DEC" on input row %"DEC": %s", icol, in_row, err_buf.ptr());
            }
            // a formula failing is a non-fatal event; we log and continue
            c.val.setNull();
            c.len = 0;
            _buf.add0();
            _ctx->qlangCtx().clearError();
            return false;
        }
        if( const char * fmt = _out_cols->getFormat(icol, keep) ) {
            printFormatedValue(_buf, c.val, fmt, _out_cols->getFormatType(icol, keep));
        } else {
            c.val.print(_buf, sVariant::eCSV);
        }
    } else {
        idx in_col = c.in_col = _out_cols->getInputCol(icol, keep);
        if( const char * fmt = _out_cols->getFormat(icol, keep) ) {
            _in_table->val(c.val, in_row, in_col);
            printFormatedValue(_buf, c.val, fmt, _out_cols->getFormatType(icol, keep));
        } else {
            c.lazy_val_from_in_table = true;
            _in_table->printCell(_buf, in_row, in_col, 0, 0, sTabular::fForCSV);
        }
    }

    c.len = _buf.length() - c.pos;
    _buf.add0();
    return true;
}

static const char * rowMethodNames = "rowlist"_
"inrowlist"_
"equals"_
"regex"_
"substring"_
"formula"_
"range"__;

static const char * colMethodNames = "collist"_
"incollist"_
"equals"_
"regex"_
"substring"_
"formula"_
"range"__;

const char * OutputRowFilter::_default_opname = "filter";
const char * OutputColFilter::_default_opname = "colfilter";

static OutputFilter::EMethod parseMethodName(sVariant * val, const char * methodNames)
{
    idx meth = OutputFilter::eMethod_invalid;
    if( val && sString::compareChoice(val->asString(), methodNames, &meth, true, 0, true) >= 0 )
        return static_cast<OutputFilter::EMethod>(meth);
    return OutputFilter::eMethod_invalid;
}

#define INVALID_PARAMETER(param) \
    _ctx->logError("%s parameter for %s operation", param, _opname); \
    return false

bool OutputFilter::initValues(sVariant * values)
{
    sStr formula_op_desc("formula value for %s operation", _opname);

    for(idx i = 0; i < values->dim(); i++) {
        sVariant * value_val = values->getListElt(i);

        switch(_method) {
            case eMethod_list:
            case eMethod_inlist:
                // should be handled by specific filter
                break;
            case eMethod_equals:
            case eMethod_substring:
                _specs[i].val = *value_val;
                _specs[i].type = ValueSpec::eSpec_val;
                break;
            case eMethod_regex: {
                int flags = REG_EXTENDED | REG_NOSUB;
                if( !_case_sensitive )
                    flags |= REG_ICASE;
                _specs[i].type = ValueSpec::eSpec_regex;
                if( regcomp(&_specs[i].re, value_val->asString(), flags) != 0 ) {
                    _ctx->logError("Invalid regular expression value argument for %s operation", _opname);
                    return false;
                }
            }
                break;
            case eMethod_formula: {
                _specs[i].fmla = ParseUtils::parseFormulaArg(value_val, *_ctx, formula_op_desc.ptr());
                if( unlikely(!_specs[i].fmla) )
                    return false;
                _specs[i].type = ValueSpec::eSpec_val;
            }
                break;
            case eMethod_range: {
                sVariant * min_val = value_val->getDicElt("min");
                sVariant * max_val = value_val->getDicElt("max");
                bool exclusive = false;
                if( sVariant * excl_val = value_val->getDicElt("exclusive") ) {
                    exclusive = excl_val->asBool();
                }

                if( min_val ) {
                    if( max_val ) {
                        _specs[i].type = exclusive ? ValueSpec::eSpec_range_exclusive : ValueSpec::eSpec_range_inclusive;
                        _specs[i].val = *min_val;
                        _specs[i].val2 = *max_val;
                    } else {
                        _specs[i].type = exclusive ? ValueSpec::eSpec_gt : ValueSpec::eSpec_ge;
                        _specs[i].val = *min_val;
                    }
                } else if( max_val ) {
                    _specs[i].type = exclusive ? ValueSpec::eSpec_lt : ValueSpec::eSpec_le;
                    _specs[i].val = *max_val;
                } else {
                    _ctx->logError("Invalid range value argument for %s operation: need min and/or max", _opname);
                    return false;
                }
            }
                break;
            default:
                _ctx->logError("Missing or invalid method argument for %s operation", _opname);
                return false;
        }
    }

    return true;
}

bool OutputRowFilter::init(sVariant * arg, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx, const char * opname/*=0*/)
{
    _in_table = in_table;
    _use_in_table = false;
    _out_cols = out_cols;
    _out_buf = out_buf;
    _ctx = ctx;
    _icur_row = 0;
    _opname = opname ? opname : _default_opname;

    if( !arg ) {
        INVALID_PARAMETER("Missing or invalid arg");
    }

    sVariant values_val_holder, *values_val = 0;
    if( sVariant * value_val = arg->getDicElt("value") ) {
        values_val_holder.setList();
        values_val_holder.push(*value_val);
        values_val = &values_val_holder;
    } else {
        values_val = arg->getDicElt("values");
    }

    if( !values_val ) {
        INVALID_PARAMETER("Missing or invalid value/values");
    }

    if( sVariant * case_val = arg->getDicElt("caseSensitive") ) {
        _case_sensitive = case_val->asBool();
    }

    if( sVariant * negate_val = arg->getDicElt("negate") ) {
        _negate = negate_val->asBool();
    }

    if( sVariant * conj_val = arg->getDicElt("colConjunction") ) {
        _conjunction = conj_val->asBool();
    }

    _method = parseMethodName(arg->getDicElt("method"), rowMethodNames);

    if( _method == eMethod_list || _method == eMethod_inlist ) {
        sVec<idx> rows_raw;
        rows_raw.resize(values_val->dim());
        for(idx i = 0; i < rows_raw.dim(); i++) {
            rows_raw[i] = values_val->getListElt(i)->asInt();
        }
        sSort::sort(rows_raw.dim(), rows_raw.ptr());
        // record unique rows
        _rows.resize(rows_raw.dim());
        idx nrows = 0;
        for(idx i = 0; i < rows_raw.dim(); i++) {
            if( nrows == 0 || rows_raw[i] != _rows[nrows - 1] ) {
                _rows[nrows++] = rows_raw[i];
            }
        }
        _rows.cut(nrows);
    } else if( _method == eMethod_formula ) {
        _specs.resize(values_val->dim());
    } else {
        _specs.resize(values_val->dim());

        if( sVariant * col_val = arg->getDicElt("col") ) {
            if( !ParseUtils::parseColsArg(_cols, col_val, _out_cols, *_ctx, false) ) {
                INVALID_PARAMETER("Invalid col");
            }
        } else if (sVariant * cols_val = arg->getDicElt("cols")) {
            if (cols_val->isString() && !strcmp(cols_val->asString(), "*")) {
                _cols.resize(out_cols->dim());
                for (idx i=0; i<_cols.dim(); i++) {
                    _cols[i] = i;
                }
            } else if (cols_val->isList()) {
                for (idx i=0; i<cols_val->dim(); i++) {
                    // parseColsArg will resize _cols
                    if (!ParseUtils::parseColsArg(_cols, cols_val->getListElt(i), _out_cols, *_ctx, true)) {
                        INVALID_PARAMETER("Invalid cols");
                    }
                }
            } else if (!ParseUtils::parseColsArg(_cols, cols_val, _out_cols, *_ctx, true)) {
                INVALID_PARAMETER("Invalid cols");
            }
        } else if (sVariant * incol_val = arg->getDicElt("incol")) {
            _use_in_table = true;
            if (!ParseUtils::parseColsArg(_cols, incol_val, in_table, *_ctx, false)) {
                INVALID_PARAMETER("Invalid incol");
            }
        } else if (sVariant * incols_val = arg->getDicElt("incols")) {
            _use_in_table = true;
            if (incols_val->isString() && !strcmp(incols_val->asString(), "*")) {
                _cols.resize(in_table->cols());
                for (idx i=0; i<_cols.dim(); i++) {
                    _cols[i] = i;
                }
            } else if (incols_val->isList()) {
                for (idx i=0; i<cols_val->dim(); i++) {
                    // parseColsArg will resize _cols
                    if (!ParseUtils::parseColsArg(_cols, incols_val->getListElt(i), in_table, *_ctx, true)) {
                        INVALID_PARAMETER("Invalid incols");
                    }
                }
            } else if (!ParseUtils::parseColsArg(_cols, incols_val, _out_cols, *_ctx, true)) {
                INVALID_PARAMETER("Invalid incols");
            }
        } else {
            _cols.resize(out_cols->dim());
            for (idx i=0; i<_cols.dim(); i++) {
                _cols[i] = i;
            }
        }

        if( !_use_in_table ) {
            _colHandles.resize(_cols.dim());
            for (idx i=0; i<_cols.dim(); i++) {
                _colHandles[i] = _out_cols->saveColumn(_cols[i]);
            }
        }
    }

    return initValues(values_val);
}

bool OutputColFilter::init(sVariant * arg, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx, const char * opname/*=0*/)
{
    _in_table = in_table;
    _use_in_table = false;
    _out_cols = out_cols;
    _out_buf = out_buf;
    _ctx = ctx;
    _evaled = false;
    _all_rows = false;
    _icur_row = 0;
    _opname = opname ? opname : _default_opname;

    if( !arg ) {
        INVALID_PARAMETER("Missing or invalid arg");
    }

    sVariant values_val_holder, *values_val = 0;
    if( sVariant * value_val = arg->getDicElt("value") ) {
        values_val_holder.setList();
        values_val_holder.push(*value_val);
        values_val = &values_val_holder;
    } else {
        values_val = arg->getDicElt("values");
    }

    if( !values_val ) {
        INVALID_PARAMETER("Missing or invalid value/values");
    }

    if( sVariant * case_val = arg->getDicElt("caseSensitive") ) {
        _case_sensitive = case_val->asBool();
    }

    if( sVariant * negate_val = arg->getDicElt("negate") ) {
        _negate = negate_val->asBool();
    }

    if( sVariant * conj_val = arg->getDicElt("rowConjunction") ) {
        _conjunction = conj_val->asBool();
    }

    _results.resize(_out_cols->dim());
    for(idx i = 0; i < _results.dim(); i++) {
        _results[i] = eResult_print;
    }

    _method = parseMethodName(arg->getDicElt("method"), colMethodNames);

    if( _method == eMethod_list || _method == eMethod_inlist ) {
        if( !_negate ) {
            for(idx i = 0; i < _results.dim(); i++) {
                _results[i] = eResult_hide;
            }
        }
        sVec<idx> icols;
        for(idx i = 0; i < values_val->dim(); i++) {
            idx icol = values_val->getListElt(i)->asInt();
            if( _method == eMethod_inlist ) {
                out_cols->getOutputCols(icols, icol);
            } else {
                *icols.add(1) = icol;
            }
        }
        for(idx i = 0; i < icols.dim(); i++) {
            if( icols[i] >= 0 && icols[i] < _results.dim() ) {
                _results[icols[i]] = _negate ? eResult_hide : eResult_print;
            }
        }
    } else if( _method == eMethod_formula ) {
        _specs.resize(values_val->dim());
    } else {
        _specs.resize(values_val->dim());

        if( sVariant * row_val = arg->getDicElt("row") ) {
            _rows.resize(1);
            _rows[0] = row_val->asInt();
        } else if( sVariant * rows_val = arg->getDicElt("rows") ) {
            if( rows_val->isString() && !strcmp(rows_val->asString(), "*") ) {
                _all_rows = true;
            } else if( rows_val->isList() ) {
                _rows.resize(rows_val->dim());
                for(idx i = 0; i < _rows.dim(); i++) {
                    _rows[i] = rows_val->getListElt(i)->asInt();
                }
            } else {
                INVALID_PARAMETER("Invalid rows");
            }
        } else if (sVariant * inrow_val = arg->getDicElt("inrow")) {
            _use_in_table = true;
            _rows.resize(1);
            _rows[0] = inrow_val->asInt();
        } else if (sVariant * inrows_val = arg->getDicElt("inrows")) {
            _use_in_table = true;
            if (inrows_val->isString() && !strcmp(inrows_val->asString(), "*")) {
                _all_rows = true;
            } else if (inrows_val->isList()) {
                _rows.resize(rows_val->dim());
                for (idx i=0; i<_rows.dim(); i++) {
                    _rows[i] = rows_val->getListElt(i)->asInt();
                }
            } else {
                INVALID_PARAMETER("Invalid inrows");
            }
        } else {
            _all_rows = true;
        }
    }

    return OutputFilter::initValues(values_val);
}

bool OutputRowFilter::initSearch(const char * srch, bool srchRegExp, const char * srchRangeSet, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx)
{
    _in_table = in_table;
    _out_cols = out_cols;
    _out_buf = out_buf;
    _ctx = ctx;
    _conjunction = false;
    _method = srchRegExp ? eMethod_regex : eMethod_substring;
    _icur_row = 0;

    if( !srch || !*srch ) {
        _ctx->logError("Missing or invalid search parameter");
        return false;
    }

    _specs.resize(1);
    _specs[0].val.setString(srch);
    if( srchRegExp ) {
        int flags = REG_EXTENDED | REG_NOSUB | REG_ICASE;
        _specs[0].type = ValueSpec::eSpec_regex;
        if( regcomp(&_specs[0].re, srch, flags) != 0 ) {
            _ctx->logError("Invalid regular expression value argument for %s operation", _opname);
            _specs.cut(0);
            return false;
        }
    } else {
        _specs[0].type = ValueSpec::eSpec_val;
    }

    _use_in_table = true;

    if( srchRangeSet && *srchRangeSet ) {
        sString::scanRangeSet(srchRangeSet, 0, &_cols, 0, 0, 0);
    } else {
        _cols.resize(in_table->cols());
        for(idx i = 0; i < in_table->cols(); i++)
            _cols[i] = i;
    }

    _colHandles.resize(_cols.dim());
    for (idx i=0; i<_cols.dim(); i++) {
        _colHandles[i] = _out_cols->saveColumn(_cols[i]);
    }

    return true;
}

static sVariant * destringifiedVal(sVariant & spec_val, sVariant * pdataval, sVariant & tmp_val, OutputFilter::EMethod method)
{
    sVariant * ret = &spec_val;
    if( (method == OutputFilter::eMethod_equals || method == OutputFilter::eMethod_range) && spec_val.isString() && !pdataval->isString() ) {
        tmp_val.parseScalarType(spec_val.asString(), pdataval->getType());
        ret = &tmp_val;
    }
    return ret;
}

OutputFilter::EResult OutputFilter::evalDataValue(sVariant * pdataval, bool allowFormula)
{
    bool pass = false;

    if( allowFormula && _method == eMethod_formula ) {
        _ctx->qlangCtx().registerBuiltinThis(*pdataval);
    }

    for(idx is = 0; !pass && is < _specs.dim(); is++) {
        sVariant * pspecval = destringifiedVal(_specs[is].val, pdataval, _tmp_val1, _method);
        sVariant * pspecval2 = destringifiedVal(_specs[is].val2, pdataval, _tmp_val2, _method);

        switch(_method) {
            case eMethod_equals:
                pass = *pdataval == *pspecval;
                break;
            case eMethod_regex:
                pass = !regexec(&(_specs[is].re), pdataval->asString(), 0, 0, 0);
                break;
            case eMethod_substring:
                if( _case_sensitive )
                    pass = strstr(pdataval->asString(), _specs[is].val.asString());
                else
                    pass = strcasestr(pdataval->asString(), _specs[is].val.asString());
                break;
            case eMethod_formula:
                if( !allowFormula )
                    goto ERROR;
                if( unlikely(!_specs[is].fmla->eval(_tmp_val, _ctx->qlangCtx())) ) {
                    // non-fatal
                    sStr err_buf;
                    _ctx->logInfo("Failed formula on filtering input row %"DEC": %s", _ctx->qlangCtx().getCurInputRow(), _ctx->qlangCtx().printError(err_buf));
                    _ctx->qlangCtx().clearError();
                    return eResult_error;
                }
                pass = _tmp_val.asBool();
                break;
            case eMethod_range:
                switch(_specs[is].type) {
                    case ValueSpec::eSpec_range_inclusive:
                        pass = *pdataval >= *pspecval && *pdataval <= *pspecval2;
                        break;
                    case ValueSpec::eSpec_range_exclusive:
                        pass = *pdataval > *pspecval && *pdataval < *pspecval2;
                        break;
                    case ValueSpec::eSpec_ge:
                        pass = *pdataval >= *pspecval;
                        break;
                    case ValueSpec::eSpec_gt:
                        pass = *pdataval > *pspecval;
                        break;
                    case ValueSpec::eSpec_le:
                        pass = *pdataval <= *pspecval;
                        break;
                    case ValueSpec::eSpec_lt:
                        pass = *pdataval < *pspecval;
                        break;
                    default:
                        // should not happen, fall through to error below
                        break;
                }
                break;
            default:
                ERROR:
                // should not happen!
                fprintf(stderr, "%s:%u: unexpected filter method\n", __FILE__, __LINE__);
                assert(1);
        }

        if( _negate )
            pass = !pass;

        if( pass )
            break;
    }

    return pass ? eResult_print : eResult_hide;
}

OutputRowFilter::EResult OutputRowFilter::eval(idx in_row, idx out_row)
{
    bool pass = false;

    switch(_method) {
        case eMethod_formula:
            for(idx is = 0; is < _specs.dim(); is++) {
                if( unlikely(!_specs[is].fmla->eval(_tmp_val, _ctx->qlangCtx())) ) {
                    // non-fatal
                    sStr err_buf;
                    _ctx->logInfo("Failed formula on filtering input row %"DEC": %s", _ctx->qlangCtx().getCurInputRow(), _ctx->qlangCtx().printError(err_buf));
                    _ctx->qlangCtx().clearError();
                    return eResult_error;
                }
                pass = _tmp_val.asBool();
                if( _negate )
                    pass = !pass;

                if( pass )
                    return eResult_print;
            }
            return eResult_hide;
        case eMethod_list:
        case eMethod_inlist: {
            bool always_increment = _method == eMethod_list;

            if( _icur_row < _rows.dim() ) {
                idx spec_row = _rows[_icur_row];
                idx cur_row = _method == eMethod_list ? out_row : in_row;
                if( cur_row == spec_row ) {
                    _icur_row++;
                    pass = true;
                } else if( cur_row > spec_row ) {
                    _icur_row++;
                }
            } else {
                always_increment = false;
            }

            if( _negate ) {
                pass = !pass;
            }
            if( pass ) {
                return eResult_print;
            } else {
                return always_increment ? eResult_hide_and_increment : eResult_hide;
            }
        }
        default:
            break;
    }

    const idx cdim = _use_in_table ? _cols.dim() : _colHandles.dim();

    if( !cdim )
        return eResult_print;

    for(idx ic = 0; ic < cdim; ic++) {
        sVariant * pdataval = 0;
        if( _use_in_table ) {
            _in_table->val(_tmp_val, in_row, _cols[ic]);
            pdataval = &_tmp_val;
        } else {
            if( !_out_buf->initializedForSaved(_colHandles[ic]) && !_out_buf->setCellForSaved(_colHandles[ic], in_row) )
                return eResult_error;

            pdataval = &(_out_buf->valForSaved(_colHandles[ic]));
        }

        pass = false;

        switch(evalDataValue(pdataval, false)) {
            case eResult_error:
                return eResult_error;
            case eResult_print:
                pass = true;
                break;
            default:
                break;
        }

        if( _conjunction && !pass ) {
            return eResult_hide;
        } else if( !_conjunction && pass ) {
            return eResult_print;
        }
    }

    return _conjunction ? eResult_print : eResult_hide;
}

bool OutputColFilter::eval(idx in_row, idx out_row)
{
    if( _method == eMethod_list || _method == eMethod_inlist ) {
        // trivial case, _results already prepared in init()
        _evaled = true;
        return true;
    }

    if( _icur_row < _rows.dim() ) {
        idx spec_row = _rows[_icur_row];
        idx cur_row = _use_in_table ? in_row : out_row;
        if( cur_row == spec_row ) {
            _icur_row++;
        } else if( cur_row > spec_row ) {
            _icur_row++;
            return true;
        }
    } else if( !_all_rows ) {
        return true;
    }

    for(idx icol = 0; icol < _results.dim(); icol++) {
        if( !_conjunction && _results[icol] == eResult_hide )
            continue;

        sVariant * pdataval = 0;
        if( _use_in_table ) {
            idx in_col = _out_cols->getInputCol(icol);
            if( unlikely(in_col < 0) ) {
                _results[icol] = eResult_hide;
                continue;
            }

            _in_table->val(_tmp_val, in_row, icol);
            pdataval = &_tmp_val;
        } else {
            if( !_out_buf->initialized(icol) && !_out_buf->setCell(icol, in_row) )
                return eResult_error;

            pdataval = &(_out_buf->val(icol));
        }

        bool pass = false;
        switch(evalDataValue(pdataval, false)) {
            case eResult_error:
                return eResult_error;
            case eResult_print:
                pass = true;
                break;
            default:
                break;
        }

        _results[icol] = pass ? eResult_print : eResult_hide;
    }

    _evaled = true;
    return true;
}

const char * OutputCategories::print(sStr & out, const char * indent)
{
    idx out_start = out.length();

    out.printf("\"categories\": {");
    for(idx ic = 0; ic < dim(); ic++) {
        const char * cat_name = getCatName(ic);
        const sVec<idx> cat_values = getCatValues(ic);
        const sVariant & cat_tqs_op = getCatTqsOp(ic);

        out.printf("%s\r\n%s    ", ic ? "," : "", indent);
        sString::escapeForJSON(out, cat_name);
        out.printf(": {\r\n%s        \"%s\": [", indent, getTypeName());
        for(idx ir = 0; ir < cat_values.dim(); ir++) {
            out.printf("%s%"DEC, ir ? ", " : "", cat_values[ir]);
        }
        out.printf("]");
        for(idx ik = 0; ik < cat_tqs_op.dim(); ik++) {
            const char * key = cat_tqs_op.getDicKey(ik);
            if( key && strcmp(key, "op") && strcmp(key, "arg") ) {
                out.printf(",\r\n%s        ", indent);
                sString::escapeForJSON(out, key);
                out.printf(": ");
                cat_tqs_op.getDicElt(key)->print(out, sVariant::eJSON);
            }
        }
        out.printf("\r\n%s    }", indent);
    }
    out.printf("\r\n%s}", indent);

    return out.ptr(out_start);
}

const char * OutputRowCategories::getTypeName() const
{
    static const char * typeName = "rows";
    return typeName;
}

const char * OutputColCategories::getTypeName() const
{
    static const char * typeName = "cols";
    return typeName;
}

const char * OutputRowCategories::getOpName() const
{
    static const char * opName = "rowcategory";
    return opName;
}

const char * OutputColCategories::getOpName() const
{
    static const char * opName = "colcategory";
    return opName;
}

bool OutputCategories::add(sVariant * tqs_op, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx)
{
    if( !tqs_op ) {
        ctx->logError("Missing %s operation", getOpName());
        return false;
    }

    sVariant * arg = tqs_op->getDicElt("arg");
    if( !arg ) {
        ctx->logError("Missing or invalid arg parameter for %s operation", getOpName());
        return false;
    }

    const char * name = 0;
    if( sVariant * name_val = arg->getDicElt("name") ) {
        name = name_val->asString();
    } else {
        ctx->logError("Missing or invalid name argument for %s operation", getOpName());
        return false;
    }

    if( hasCat(name) ) {
        ctx->logError("Duplicate %s category with name \"%s\"", getOpName(), name);
        return false;
    }

    return initNewCat(name, arg, tqs_op, in_table, out_cols, out_buf, ctx);
}

bool OutputRowCategories::initNewCat(const char * name, sVariant * arg, sVariant * tqs_op, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx)
{
    Cat * cat = static_cast<Cat*>(_cats.set(name));
    if( !cat->categorizer.init(arg, in_table, out_cols, out_buf, ctx, getOpName()) )
        return false;
    cat->tqs_op = *tqs_op;
    return true;
}

bool OutputColCategories::initNewCat(const char * name, sVariant * arg, sVariant * tqs_op, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx)
{
    Cat * cat = static_cast<Cat*>(_cats.set(name));
    if( !cat->categorizer.init(arg, in_table, out_cols, out_buf, ctx, getOpName()) )
        return false;
    cat->tqs_op = *tqs_op;
    return true;
}

bool OutputRowCategories::categorize(idx in_row, idx out_row)
{
    for(idx ic = 0; ic < _cats.dim(); ic++) {
        Cat * cat = _cats.ptr(ic);
        switch(cat->categorizer.eval(in_row, out_row)) {
            case OutputFilter::eResult_print:
                *cat->values.add(1) = out_row;
                break;
            case OutputFilter::eResult_hide:
            case OutputFilter::eResult_hide_and_increment:
                break;
            case OutputFilter::eResult_error:
                return false;
        }
    }
    return true;
}

bool OutputColCategories::categorize(idx in_row, idx out_row)
{
    for(idx ic = 0; ic < _cats.dim(); ic++) {
        Cat * cat = _cats.ptr(ic);
        if( !cat->categorizer.eval(in_row, out_row) )
            return false;
    }
    return true;
}

const sVec<idx> & OutputColCategories::getCatValues(idx icat)
{
    Cat * cat = _cats.ptr(icat);
    if( !cat->values_prepared ) {
        const sVec<OutputFilter::EResult> & results = cat->categorizer.getResults();
        for(idx icol = 0; icol < results.dim(); icol++) {
            if( results[icol] == OutputFilter::eResult_print ) {
                *cat->values.add(1) = icol;
            }
        }
        cat->values_prepared = true;
    }
    return cat->values;
}
