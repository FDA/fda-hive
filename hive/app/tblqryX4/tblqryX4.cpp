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
#include <slib/utils/tbl.hpp>
#include <ulib/uquery.hpp>
#include <violin/violin.hpp>
#include <violin/hiveion.hpp>
#include <qpsvc/archiver.hpp>

#include "rowscols.hpp"
#include "utils.hpp"
#include "qlang.hpp"
#include "sort.hpp"
#include "glue.hpp"
#include "tblqryX4_cmd.hpp"
#include "tblqryX4_cmdlist.hpp"

using namespace slib;
using namespace slib::qlang;
using namespace slib::tblqryx4;

/*
 tqs parameter / tqsId file object:

 { "op": "filter",
     "arg": {
         "col": 10,
         "method": "range",
         "value": { "min": 0, "max": 100, "exclusive": true } } }
 { "op": "filter",
     "arg": {
         "col": 123 // {"name": "ABC", "num": 1, "optional": true}, or "cols": [1,2,3] or "incol" / "incols"
         "method": "equals" // or "range" or "regex" or "substring" or "formula" or "rowlist" (values is a list of output rows) or "inrowlist" (values is a list of input rows)
         "value": "hello" // or 42 or {"min": 1, "max": 10, "exclusive": true // false by default}
             // or "values": ["hello", "bye"] // or list of ranges
         "negate": true // false by default
         "caseSensitive": true // false by default
         "colConjunction": true // false by default; used for multiple columns } }
 { "op": "colfilter",
     "arg": {
         "row": 123 // or "rows": [1,2,3], or "inrow" / "inrows"
         "method": "equals" // same as for filter
         "value": "hello" // same as for filter, except formulas operate on this == current cell
         // etc same as filter } }
 { "op": "slice",
     "arg": {
         "start" : {
             "col": 123,
             "value": "ABC",
             "method": "firstEquals" // or "firstLessThan" or "firstGreaterThan"; can replace "first with "last" }
         },
         "end": {
             "col": 123,
             "value": "ABC",
             "method": "lastEquals"
         } } }
 { "op": "predefine",
     "arg": {
         "name": "foo",
         "formula": "$1 + $2 + $3" // or "value": 12345; "ask": "int" to instruct UI to ask user for an integer; will substitute for {"tmpl": "..."} constructions} }
 { "op": "definecol",
     "arg": {
         "col": 123,
         "formula": "$1 + $2 + $3",
         "type": "string" // "integer", "real",
         "format": "0.3f" } }
 { "op": "insertcol",
     "arg": {
         "col": 123,
         "name": "ABC", // or { "formula": "cat($123, ' copy')" } } ; default name is name of source column (if formula is of form $N or ${name}), else ""
         formula: "$42" } }
 { "op": "appendcol",
     "arg": {
         "name": "ABC", // or { "formula": "cat($123, ' copy')" } } ; default name is name of source column (if formula is of form $N or ${name}), else ""
         "formula": "$1" } }
 { "op": "hidecol",
     "arg": {
         "col": 123 // or "cols": [123, 456, {"name": "foo"}], or "cols": "*", or "cols": { "regex": "^blah", "caseSensitive": true, "negate": true } },
         "negate": true // false by default }
 { "op": "renamecol",
     "arg": {
         "col": 123,
         "to": "New Name" // or { "formula": "cat($123, ' renamed')" } }
 { "op": "movecol",
     "arg": {
         "col": 123,
         "to": 124 } }
 { "op": "setcell",
     "arg": {
         "row": 0,
         "col": 123,
         "value": "hello world" } }
 { "op": "rowcategory",
     "arg": {
         "formula": "$1 > 10 && $2 == 'total'",
         "name": "abc" },
     "color": "#ff0000" }
 { "op": "rowcategory",
     "color": "red",
     "arg": {
         "method": "inrowlist" // or any other filter method,
         "values": [5, 10, 16], "name": "_selected" } }
 { "op": "colcategory", ... }
 { "op": "sort",
     "arg": {
         "formula": "$1", // or "formulas": [...]
         "reverse": false.
         "maxCountEach": 10 // max number of rows with any one formula result } }
 { "op": "transpose",
     "arg": {
         "topHeader": 1, // how many top header rows after transpose; default : same as in source table
         "leftHeader": 1 // how many left header cols after transpose; default : 0
    } }
 { "op": "glue",
     "arg": {
         "rhs" : {
             "obj": 12345,
             "req": 56789,
             "tbl": "foo.csv",
             "formula": "abs($2)",
             "hidecol": 2 // or "*" or [1,2,3] or {"name" : "foo"} etc.
          },
          "lhs" : {
              "formula": "$1"
          }
          // order is taken from left
    } }
 { "op": "ionwander",
     "arg": {
         "ionId": 12345, // or [12345, 67890] or { "objQry": "alloftype('whatever').filter(...)" }
         "ionFile": "foo*.ion", // "ion.ion" by default
         // query can be specified as an ionql file in an object ...
         "qryId": 12345,
         "qryFile": "hello.ionql",
         // ... or as an explicit text string
         "qry": "o=find.taxid_name(taxid=9606, tag='scientific name'); printCSV(o.taxid, o.name)"
         "cnt": 1, // 200 by default
         "qryPrintsHdr": true // false by default (default assumption is ION queries don't print headers)
    } }
 { "op": "addmissingrows",
     "arg": {
         "abscissa": {
             "col": 0,
             "maxGap": 1 // default: determine automagically
             "minValue": { "formula": "2 + 2" }, // or 2 etc., default is automatic
             "maxValue": { "formula": "min(100, blah())" }, // or 100 etc., default is automatic
         },
         "add": [
             { "col": 0, "value": 1},
             { "col": 1, "value": {"formula": "abc()"}},
             { "col": "*", "value": 0 }
         ]
   } }
 { "op": "load",
   "arg": {
       "concat": [
           {
               "obj": 1234, // or "req" or "grp"
               "tbl": "foobar.csv" // "_.csv" by default; or "glob": "*.csv"
           },
           {
               "obj": 1235, tbl: "wombat.csv"
           }
       ]
   } }
 { "op": "collapserows" // TODO }

 params: inputStart, inputCnt, parseCnt, resolution, search, searchRegExp, searchCols. tqsCnt
 dataCmd
 */

const idx RowSorter::_hash_bits = 10;
const idx RowSorter::_irow_mask = ((idx) 1 << RowSorter::_hash_bits) - 1;

#if _DEBUG

static void stripTerminalNewline(sStr & s)
{
    idx l = s.length();
    while( l && (s[l - 1] == '\r' || s[l - 1] == '\n') ) {
        s[--l] = 0;
    }
    s.cut(l);
}

#define PRINT_DEBUG_MAX_ROWS 50
#define PRINT_DEBUG_MAX_COLS 20
static void printDebugTable(idx istage, idx loader_handle, const sTabular * cur_table, const sTabular * prev_table = 0)
{
    if( istage >= 0 ) {
        fprintf(stderr, "Output table at stage %"DEC" ", istage);
    } else {
        fprintf(stderr, "Input table %"DEC" ", loader_handle);
    }
    if( !cur_table ) {
        if( istage >= 0 && !prev_table ) {
            fprintf(stderr, "is NULL, same as previous table\n");
        } else {
            fprintf(stderr, "is NULL\n");
        }
        return;
    }
    fprintf(stderr, "(%"DEC" x %"DEC" data cells, %"DEC" header rows, %"DEC" header cols):", cur_table->rows(), cur_table->cols(), cur_table->dimTopHeader(), cur_table->dimLeftHeader());
    if( cur_table == prev_table ) {
        fprintf(stderr, " same as previous table\n");
        return;
    } else {
        fprintf(stderr, "\n");
    }

    sStr buf;
    const idx istart1 = -cur_table->dimTopHeader();
    const idx iend1 = cur_table->rows() > PRINT_DEBUG_MAX_ROWS ? PRINT_DEBUG_MAX_ROWS / 2 : cur_table->rows();
    const idx istart2 = cur_table->rows() > PRINT_DEBUG_MAX_ROWS ? cur_table->rows() - PRINT_DEBUG_MAX_ROWS / 2 : cur_table->rows();
    const idx iend2 = cur_table->rows();

    for(idx irow = istart1; irow < iend1; irow++) {
        buf.cut(0);
        cur_table->printCSV(buf, irow, -cur_table->dimLeftHeader(), 1, cur_table->dimLeftHeader() + PRINT_DEBUG_MAX_COLS, false);
        stripTerminalNewline(buf);
        if( cur_table->cols() > PRINT_DEBUG_MAX_COLS ) {
            if( irow == -cur_table->dimTopHeader() ) {
                buf.printf(",<%"DEC" columns skipped...>", cur_table->cols() - PRINT_DEBUG_MAX_COLS);
            } else {
                buf.addString(",<...>");
            }
        }
        fprintf(stderr, "%s\n", buf.ptr());
    }

    if( istart2 > iend1 ) {
        fprintf(stderr, "<%"DEC" rows skipped...>\n", cur_table->rows() - PRINT_DEBUG_MAX_ROWS);
    }

    for(idx irow = istart2; irow < iend2; irow++) {
        buf.cut(0);
        cur_table->printCSV(buf, irow, -cur_table->dimLeftHeader(), 1, cur_table->dimLeftHeader() + PRINT_DEBUG_MAX_COLS, false);
        stripTerminalNewline(buf);
        if( cur_table->cols() > PRINT_DEBUG_MAX_COLS ) {
            buf.addString(",<...>");
        }
        fprintf(stderr, "%s\n", buf.ptr());
    }
}

#endif

class Builtin_cell: public BuiltinFunction
{
    public:
        Builtin_cell()
        {
            _name.printf("builtin cell() function");
        }
        bool call(sVariant &result, Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            if( !checkNArgs(ctx, nargs, 2) || !checkTopicNone(ctx, topic) )
                return false;
            if( const sTabular * tbl = ctx.getTable() ) {
                idx icol = 0;
                if( args[1].isNumeric() ) {
                    icol = args[1].asInt();
                } else {
                    icol = tbl->colId(args[1].asString());
                }
                tbl->val(result, args[0].asInt(), icol);
            } else {
                result.setNull();
            }
            return true;
        }
};

class Builtin_colcells: public BuiltinFunction
{
    public:
        Builtin_colcells()
        {
            _name.printf("builtin colcells() function");
        }
        bool call(sVariant &result, Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            const sTabular * tbl = ctx.getTable();
            if( ctx.isOutputPhase() ) {
                ctx.setError(EVAL_SYNTAX_ERROR, "%s can only be used in output phases", getName());
                return false;
            }
            if( !checkNArgs(ctx, nargs, 1, 3) || !checkTopicNone(ctx, topic) )
                return false;

            if( nargs == 2 ) {
                ctx.setError(EVAL_SYNTAX_ERROR, "%s requires either 1 or 3 arguments", getName());
                return false;
            }

            result.setList();
            if( tbl ) {
                idx icol = 0;
                if( args[0].isNumeric() ) {
                    icol = args[0].asInt();
                } else {
                    icol = tbl->colId(args[0].asString());
                    if( icol == -sIdxMax ) {
                        return true;
                    }
                }
                sVariant v;

                if( nargs == 3 ) {
                    idx start_row = args[1].asInt();
                    idx last_row = args[2].asInt();
                    idx step = start_row <= last_row ? 1 : -1;
                    if( step > 0 ) {
                        start_row = sMax<idx>(start_row, -tbl->dimTopHeader());
                        last_row = sMin<idx>(last_row, tbl->rows() - 1);
                    } else {
                        start_row = sMin<idx>(start_row, tbl->rows() - 1);
                        last_row = sMax<idx>(last_row, -tbl->dimTopHeader());
                    }
                    for(idx irow = start_row; irow * step <= last_row * step; irow += step) {
                        tbl->val(v, irow, icol);
                        result.push(v);
                    }
                } else {
                    for(idx irow = 0; irow < tbl->rows(); irow++) {
                        tbl->val(v, irow, icol);
                        result.push(v);
                    }
                }
            }

            return true;
        }
};

class Builtin_colId: public BuiltinFunction
{
    public:
        Builtin_colId()
        {
            _name.printf("builtin colId() function");
        }
        bool call(sVariant &result, Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            if( !checkNArgs(ctx, nargs, 1, 2) || !checkTopicNone(ctx, topic) )
                return false;

            if( const sTabular * tbl = ctx.getTable() ) {
                if( nargs >= 2 && args[1].isString() && !strcmp(args[1].asString(), "*") ) {
                    // get all ids
                    result.setList();
                    for(idx i = 0; 1; i++) {
                        idx id = tbl->colId(args[0].asString(), 0, i);
                        if( id == -sIdxMax )
                            break;
                        else
                            result.push(id);
                    }

                    return true;
                }

                idx id = tbl->colId(args[0].asString(), 0, nargs >= 2 ? args[1].asInt() : 0);
                if( id == -sIdxMax )
                    result.setNull();
                else
                    result.setInt(id);
            } else {
                result.setNull();
            }

            return true;
        }
};

class Builtin_headercell: public BuiltinFunction
{
    public:
        Builtin_headercell()
        {
            _name.printf("builtin headercell() function");
        }
        bool call(sVariant &result, Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            if( !checkNArgs(ctx, nargs, 1) || !checkTopicNone(ctx, topic) )
                return false;

            if( const sTabular * tbl = ctx.getTable() ) {
                if( tbl->dimTopHeader() > 1 ) {
                    result.setList();
                    for(idx irow = -tbl->dimTopHeader(); irow < 0; irow++) {
                        sVariant v;
                        tbl->val(v, irow, args[0].asInt());
                        result.push(v);
                    }
                } else {
                    tbl->val(result, -1, args[0].asInt());
                }
            } else {
                result.setNull();
            }

            return true;
        }
};

class Builtin_incoltype: public BuiltinFunction
{
    public:
        Builtin_incoltype()
        {
            _name.printf("builtin incoltype() function");
        }
        bool call(sVariant &result, Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            if( !checkNArgs(ctx, nargs, 1, 2) || !checkTopicNone(ctx, topic) )
                return false;

            if( const sTabular * tbl = ctx.getTable() ) {
                if( nargs >= 2 && args[1].isString() && !strcmp(args[1].asString(), "*") ) {
                    // get all coltypes
                    result.setList();
                    for(idx i = 0; 1; i++) {
                        idx icol = tbl->colId(args[0].asString(), 0, i);
                        if( icol == -sIdxMax )
                            break;
                        else
                            result.push(sVariant::getTypeName(tbl->coltype(icol)));
                    }

                    return true;
                }

                idx icol = -sIdxMax;
                if( args[0].isNumeric() ) {
                    icol = args[0].asInt();
                } else {
                    icol = tbl->colId(args[0].asString(), 0, nargs >= 2 ? args[1].asInt() : 0);
                }

                if( icol == -sIdxMax )
                    result.setNull();
                else
                    result.setString(sVariant::getTypeName(tbl->coltype(icol)));
            } else {
                result.setNull();
            }

            return true;
        }
};

class Builtin_setincoltype: public BuiltinFunction
{
    public:
        Builtin_setincoltype()
        {
            _name.printf("builtin setincoltype() function");
        }
        bool call(sVariant &result, Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            if( !checkNArgs(ctx, nargs, 2, 3) || !checkTopicNone(ctx, topic) )
                return false;

            sVariant::eType itype = sVariant::parseTypeName(nargs == 2 ? args[1].asString() : args[2].asString());
            if( sTabular * tbl = ctx.getTable() ) {
                if( nargs >= 3 && args[1].isString() && !strcmp(args[1].asString(), "*") ) {
                    // set all coltypes
                    idx i = 0;
                    while( 1 ) {
                        idx icol = tbl->colId(args[0].asString(), 0, i);
                        if( icol == -sIdxMax )
                            break;

                        tbl->setColtype(icol, itype);
                        i++;
                    }

                    result.setInt(i);
                    return true;
                }

                idx icol = -sIdxMax;
                if( args[0].isNumeric() ) {
                    icol = args[0].asInt();
                } else {
                    icol = tbl->colId(args[0].asString(), 0, nargs >= 2 ? args[1].asInt() : 0);
                }

                if( icol == -sIdxMax ) {
                    result.setInt(0);
                } else {
                    result.setInt(tbl->setColtype(icol, itype));
                }
            } else {
                result.setInt(0);
            }

            return true;
        }
};

class Builtin_rowcells: public BuiltinFunction
{
    public:
        Builtin_rowcells()
        {
            _name.printf("builtin rowcells() function");
        }
        bool call(sVariant &result, Context &qlctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            if( unlikely(ctx.isOutputPhase()) ) {
                ctx.setError(EVAL_SYNTAX_ERROR, "%s can only be used in output phases", getName());
                return false;
            }
            if( !checkNArgs(ctx, nargs, 1, 3) || !checkTopicNone(ctx, topic) )
                return false;

            if( nargs == 2 ) {
                ctx.setError(EVAL_SYNTAX_ERROR, "%s requires either 1 or 3 arguments", getName());
                return false;
            }

            idx irow = args[0].asInt();
            result.setList();
            sVariant v;

            if( const sTabular * tbl = ctx.getTable() ) {
                if( nargs == 3 ) {
                    idx start = args[1].asInt();
                    idx last = args[2].asInt();
                    idx step = start <= last ? 1 : -1;
                    for(idx icol = start; icol * step <= last * step; icol += step) {
                        tbl->val(v, irow, icol);
                        result.push(v);
                    }
                } else {
                    for(idx i = 0; i < ctx.getCols()->dim(); i++) {
                        idx icol = ctx.getCols()->getInputCol(i);
                        if( icol == -sIdxMax )
                            continue;
                        tbl->val(v, irow, icol);
                        result.push(v);
                    }
                }
            }

            return true;
        }
};

class BuiltinTaxFunction: public BuiltinFunction
{
    public:
        bool getTaxInfoCol(sVariant & result, Context &qlctx, udx tax_id, idx icol)
        {
            TblQueryContext & ctx = static_cast<TblQueryContext &>(qlctx);
            if( !ctx.ensureTaxIon() ) {
                return false;
            }
            sStr tax_id_buf("%"UDEC, tax_id);
            const char * tax_info_txt = ctx.getTaxIon()->getTaxIdInfo(tax_id_buf.ptr(), tax_id_buf.length());
            if( !tax_info_txt || !tax_info_txt[0] ) {
                // tax ID not found
                result.setNull();
                return true;
            }
            sTxtTbl tax_info_tbl;
            tax_info_tbl.setBuf(tax_info_txt);
            if( !tax_info_tbl.parse() || tax_info_tbl.cols() < 5 ) {
                ctx.setError(EVAL_SYSTEM_ERROR, "Taxonomy database result in unexpected format");
                return false;
            }
            tax_info_tbl.val(result, 0, icol);
            return true;
        }
};

class Builtin_refSeqLen : public BuiltinFunction {
    public:
        Builtin_refSeqLen() { _name.printf("builtin refSeqLen() function"); }
        virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            if( !checkTopicObjectId(ctx, topic) || !checkNArgs(ctx, nargs, 1, 1) ) {
                return false;
            }

            if( sHiveseq * sub = static_cast<TblQueryContext&>(ctx).getRefSeq(*topic) ) {
                idx isub = args[0].asInt();
                if( isub > 0 && isub <= sub->dim() ) {
                    result.setInt(sub->len(isub - 1));
                }
            }

            return true;
        }
};

class Builtin_refSeqLetter : public BuiltinFunction {
    public:
        Builtin_refSeqLetter() { _name.printf("builtin refSeqLetter() function"); }
        virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            if( !checkTopicObjectId(ctx, topic) || !checkNArgs(ctx, nargs, 2, 2) ) {
                return false;
            }

            if( sHiveseq * sub = static_cast<TblQueryContext&>(ctx).getRefSeq(*topic) ) {
                idx isub = args[0].asInt();
                if( isub > 0 && isub <= sub->dim() ) {
                    idx len = sub->len(isub - 1);
                    idx pos = args[1].asInt();
                    if( pos > 0 && pos <= len ) {
                        // equivalent to sBioseq::uncompressATGC(s, sub->seq(isub - 1), pos, 1, true, 0);
                        // low-level for efficiency
                        char s[2];
                        s[0] = sBioseq::mapRevATGC[(int)sBioseqAlignment::_seqBits(sub->seq(isub - 1), pos, 0)];
                        s[1] = 0;
                        result.setString(s);
                    }
                }
            }

            return true;
        }
};

#define REGISTER_BUILTIN_FUNC(name) \
static Builtin_ ## name builtin_ ## name; \
registerBuiltinFunction(#name, builtin_ ## name)

void TblQueryContext::registerDefaultBuiltins()
{
    registerBuiltinGetter("cur_row", getCurRow, this);
    registerBuiltinGetter("input_name", getInTableName, this);
    registerBuiltinGetter("input_obj", getInTableObj, this);

    REGISTER_BUILTIN_FUNC(cell);
    REGISTER_BUILTIN_FUNC(colcells);
    REGISTER_BUILTIN_FUNC(colId);
    REGISTER_BUILTIN_FUNC(incoltype);
    REGISTER_BUILTIN_FUNC(headercell);
    REGISTER_BUILTIN_FUNC(rowcells);

    REGISTER_BUILTIN_FUNC(refSeqLen);
    REGISTER_BUILTIN_FUNC(refSeqLetter);
}

namespace {
    class LoadStage: public tblqryx4::Command
    {
        private:
            sVec<idx> loader_handles;
            bool _wraps_in_table;

        public:
            LoadStage(ExecContext & ctx)
            : tblqryx4::Command(ctx)
            {
                _wraps_in_table = false;
            }
            bool wrapsInTable() { return _wraps_in_table; }
            bool hasProgress() { return false; }
            const char * getName() { return "load"; }
            bool init(const char * op_name, sVariant * tqs_arg);
            bool sub_init(sVariant * arg);
            bool compute(sTabular * in_table);
    };

    class IonWanderStage: public tblqryx4::Command
    {
        private:
            sVar form;
            sVec<sHiveId> ion_ids;
            sHiveId qry_id;
            bool need_qry_id;
            bool qry_prints_hdr;
            ast::Node * ion_ids_qry_root, * qry_ids_qry_root;

        public:
            IonWanderStage(ExecContext & ctx)
            : tblqryx4::Command(ctx)
            {
                ion_ids_qry_root = qry_ids_qry_root = 0;
                need_qry_id = true;
                qry_prints_hdr = false;
            }

            ~IonWanderStage()
            {
                delete ion_ids_qry_root;
                delete qry_ids_qry_root;
            }

            const char * getName() { return "ionwander"; }
            bool init(const char * op_name, sVariant * tqs_arg);
            bool compute(sTabular * in_table);
    };

    class SortStage: public tblqryx4::Command
    {
        private:
            RowSorter sorter;

        public:
            SortStage(ExecContext & ctx)
            : tblqryx4::Command(ctx) {}

            const char * getName() { return "sort"; }
            bool wrapsInTable() { return true; }
            bool init(const char * op_name, sVariant * tqs_arg)
            {
                return sorter.init(tqs_arg, &_ctx);
            }
            bool compute(sTabular * in_table);
    };

    class SliceStage: public tblqryx4::Command
    {
        public:
            enum EMethod
            {
                eMethod_unknown = -2,
                eMethod_none,
                eMethod_first_eq,
                eMethod_first_lt,
                eMethod_first_gt,
                eMethod_last_eq,
                eMethod_last_lt,
                eMethod_last_gt
            } start_method, end_method;
            static EMethod parseMethodName(sVariant * method_arg);

        private:
            static const char * method_names;
            sVariant start_value, end_value;
            sVariant start_col_arg, end_col_arg;

            idx findRow(sTabular * in_table, EMethod method, sVariant & col_arg, sVariant & value, idx irow_default);

        public:
            SliceStage(ExecContext & ctx)
            : tblqryx4::Command(ctx)
            {
                start_method = end_method = eMethod_none;
            }

            const char * getName() { return "slice"; }
            bool wrapsInTable() { return true; }
            bool init(const char * op_name, sVariant * tqs_arg);
            bool compute(sTabular * in_table);
    };

    class TransposeStage: public tblqryx4::Command
    {
        private:
            idx top_header, left_header;

        public:
            TransposeStage(ExecContext & ctx)
            : tblqryx4::Command(ctx)
            {
                top_header = left_header = -sIdxMax;
            }

            const char * getName() { return "transpose"; }
            bool wrapsInTable() { return true; }
            bool init(const char * op_name, sVariant * tqs_arg);
            bool compute(sTabular * in_table);
    };

    class GlueStage: public tblqryx4::Command
    {
        private:
            struct SourceDef {
                bool uses_loader_handle;
                idx loader_handle;
                OutputColSource key_def;
                sVariant * hidecols_arg;

                SourceDef()
                {
                    hidecols_arg = 0;
                    uses_loader_handle = false;
                    loader_handle = -1;
                }
            } left_def, right_def;
            bool _wraps_in_table;

            bool init(SourceDef & def, sVariant * arg);
            sTabular * loadInputTable(SourceDef & def, sTabular * in_table);
            bool makeColMap(sVec<idx> & out_col_map, SourceDef & def, const sTabular * tbl);
            bool evalKey(sVariant & out_val, SourceDef & def, sTabular * tbl, idx ir);

        public:
            GlueStage(ExecContext & ctx)
            : tblqryx4::Command(ctx)
            {
                _wraps_in_table = false;
            }

            const char * getName() { return "glue"; }
            bool wrapsInTable() { return _wraps_in_table; }
            bool init(const char * op_name, sVariant * tqs_arg);
            bool compute(sTabular * in_table);
    };

    class SetCellStage: public tblqryx4::Command
    {
        private:
            idx irow;
            sVariant * col_arg;
            sVariant * val_arg;

        public:
            SetCellStage(ExecContext & ctx)
            : tblqryx4::Command(ctx)
            {
                irow = 0;
                col_arg = 0;
                val_arg = 0;
            }

            const char * getName() { return "setcell"; }
            bool wrapsInTable() { return true; }
            bool hasProgress() { return false; }
            bool init(const char * op_name, sVariant * tqs_arg);
            bool compute(sTabular * in_table);
    };

    class PrintStage: public tblqryx4::Command
    {
        private:
            struct MinMaxCol
            {
                idx out_col;
                sVariant min_val;
                idx min_in_row;
                sVariant max_val;
                idx max_in_row;

                MinMaxCol()
                {
                    out_col = -sIdxMax;
                    min_in_row = max_in_row = 0;
                }
            };

            sVec<OutputRowFilter> rowfilters;
            bool has_rowfilters;
            sVec<OutputColFilter> colfilters;

            OutputRowCategories rowcats;
            OutputColCategories colcats;

            OutputColumns out_cols;
            OutputBuffer out_buf;

            sVec<MinMaxCol> minmax_cols;
            sVec<idx> out_col2imm; // map from output column # to index in minmax_cols

            sVec<sVariant *> tqs_ops;

            bool findCellPair(idx icol, idx in_row1, idx in_row2);
            bool getOutValue(sVariant & result, const sTabular * in_table, idx out_icol, idx in_row) const;
            bool initRowsCols(sTabular * in_table);

            bool basicPrintOutRow(sTxtTbl * out_csv_table, idx in_row, idx out_row_ifno_filter, idx out_row_ifno_start);
            bool minmaxPrintOutRow(sTxtTbl * out_csv_table, idx in_row_default, idx out_row_ifno_filter, idx out_row_ifno_start, bool is_min);

            bool op_definecol(const sTabular * in_table, sVariant * arg);
            bool op_insertcol(const sTabular * in_table, sVariant * arg);
            bool op_appendcol(const sTabular * in_table, sVariant * arg);
            bool op_hidecol(const sTabular * in_table, sVariant * arg);
            bool op_renamecol(const sTabular * in_table, sVariant * arg);
            bool op_movecol(const sTabular * in_table, sVariant * arg);

        public:
            idx out_start, out_cnt, out_resolution;
            idx out_abscissa_col;
            const char * formstr_cols, *formstr_minmaxCols, *formstr_minmaxMainCol;
            const char * formstr_srch, *formstr_srchCols;
            bool srchRegExp; //! formstr_srch is a regular expression (as opposed to substring search)
            bool print_top_header;

            PrintStage(ExecContext & ctx) :
                tblqryx4::Command(ctx),
                out_col2imm(sMex::fExactSize)
            {
                out_start = out_cnt = out_resolution = 0;
                out_abscissa_col = -sIdxMax;
                formstr_cols = formstr_minmaxCols = formstr_minmaxMainCol = 0;
                formstr_srch = formstr_srchCols = 0;
                srchRegExp = false;
                print_top_header = true;
                has_rowfilters = false;
            }

            const char * getName() { return "print"; }
            bool init(const char * op_name, sVariant * tqs_arg) { /* not used */ return false; }
            void addOpArg(sVariant * tqs_arg, bool is_filter = false)
            {
                *tqs_ops.add(1) = tqs_arg;
                if( is_filter ) {
                    has_rowfilters = true;
                }
            }

            bool hasRowFilters() { return has_rowfilters; }

            bool compute(sTabular * in_table);
            OutputRowCategories & getRowCats() { return rowcats; }
            OutputColCategories & getColCats() { return colcats; }
    };

    class ReinterpretStage: public tblqryx4::Command
    {
        private:
            sVec<idx> binary_cols;
            bool unreinterpret;

        public:
            ReinterpretStage(ExecContext & ctx, bool unreinterpret_ = false)
            : tblqryx4::Command(ctx)
            {
                unreinterpret = unreinterpret_;
            }

            const char * getName() { return "reinterpret"; }
            bool wrapsInTable() { return true; }
            bool init(const char * op_name, sVariant * tqs_arg) { /* not used */ return false; }
            void init(const char * binary_cols_str)
            {
                if( binary_cols_str ) {
                    sString::scanRangeSet(binary_cols_str, 0, &binary_cols, 0, 0, 0);
                }
            }
            bool compute(sTabular * in_table);
    };
};

namespace slib {
    namespace tblqryx4 {
        class TblQryX4: public sQPrideProc
        {
            private:

        #if 0
                class ExecContext: public ErrorLogger, public InputLoader
                {
                    private:
                        Parser _parser;
                        TblQueryContext _query_ctx;
                        idx _output_req;
                        bool _top_header, _left_header;

                        idx _waiting_for_req;
                        sTabular *_out_table;

                        sVariant _tqs;
                        ast::Node * _root_objQry;
                        sVariant _objQryResult;

                        sDic<CommandPlugin> _cmdPlugins;

                        sVec<ExecStage*> _stages;

                        bool substituteTemplate(sVariant * arg);

                        void pushInputTable(idx loader_handle, sTabular * tbl, bool owned = true);
                        sTxtTbl * newCSVorVCF(const char * name, const char * tblIdxPath, const char * tblDataPath, sFil * tblFil, idx tblFilTime, sTxtTbl::ParseOptions & opts);
                        bool loadFileObject(idx loader_handle, InputTableSource * source);
                        bool loadRequestFile(idx loader_handle, InputTableSource * source);

                        bool appendTqs(const char * tqs_string, idx len = 0, idx tqsMaxCnt = sIdxMax);

                        void cleanupStageOutputs(idx start, idx cnt);

                    public:
                        ExecContext(sQPrideProc & proc, idx req);
                        virtual ~ExecContext();
                        // query stages
                        bool parseForm();
                        bool loadInput();
                        bool prepareStages();
                        bool processStages();
                        bool saveResult();

                        void addCommandPlugin(const char * name, initCmdCallback initCb, parseCmdCallback parseCb, runCmdCallback runCb, destroyCmdCallback destroyCb, bool outputsTable = false, bool needReinterpret=true, bool outTableUsesInTable=false);

                        idx waitingForReq() const
                        {
                            return _waiting_for_req;
                        }
                };
        #endif

            public:
                TblQryX4(const char * defline00, const char * srv)
                    : sQPrideProc(defline00, srv)
                {
                }
                idx OnExecute(idx req);
        };
    };
};

bool PrintStage::op_definecol(const sTabular * in_table, sVariant * arg)
{
    if( !arg ) {
        _ctx.logError("Missing or invalid arg parameter for definecol operation");
        return false;
    }

    sVec<idx> icols;
    if( !ParseUtils::parseColsArg(icols, arg->getDicElt("col"), &out_cols, _ctx, false) ) {
        _ctx.logError("Invalid col argument for definecol operation");
        return false;
    }

    if( icols.dim() == 1 ) {
        idx icol = icols[0];

        ast::Node * fmla = ParseUtils::parseFormulaArg(arg->getDicElt("formula"), _ctx, "formula argument for definecol operation");
        if( unlikely(!fmla) )
            return false;

        if( unlikely(!out_cols.setFormula(icol, in_table, fmla)) ) {
            _ctx.logError("Failed to set formula for definecol operation");
            return false;
        }

        sVariant::eType itype = ParseUtils::parseTypeArg(arg->getDicElt("type"));
        if( unlikely(itype != sVariant::value_NULL && !out_cols.setType(icol, itype)) ) {
            _ctx.logError("Failed to set column type for definecol operation");
            return false;
        }

        if( sVariant * fmt_val = arg->getDicElt("format") ) {
            if( unlikely(!out_cols.setFormat(icol, fmt_val->asString())) ) {
                _ctx.logError("Invalid format argument for definecol operation");
                return false;
            }
        }
    } else {
        _ctx.logDebug("Optional col argument for definecol operation failed to match; operation is no-op");
    }

    return true;
}

bool PrintStage::op_insertcol(const sTabular * in_table, sVariant * arg)
{
    if( !arg ) {
        _ctx.logError("Missing or invalid arg parameter for insertcol operation");
        return false;
    }

    idx icol = -sIdxMax;
    if( sVariant * col_val = arg->getDicElt("col") )
        icol = col_val->asInt();

    if( !out_cols.validInsertPosition(icol) ) {
        _ctx.logError("Invalid col argument for insertcol operation");
        return false;
    }

    ast::Node * fmla = ParseUtils::parseFormulaArg(arg->getDicElt("formula"), _ctx, "formula argument for insertcol operation");
    if( unlikely(!fmla) )
        return false;

    const char * name = 0;
    sStr name_buf;
    if( sVariant * name_val = arg->getDicElt("name") ) {
        name = ParseUtils::evalStringOrFormulaArg(name_buf, name_val, _ctx);
    } else {
        const char * input_col_name;
        idx input_col_num;
        if( fmla->isDollarCall(&input_col_name, &input_col_num) ) {
            if( input_col_name ) {
                name = input_col_name;
            } else {
                name = in_table->printTopHeader(name_buf, input_col_num);
            }
        }
    }

    sVariant::eType itype = ParseUtils::parseTypeArg(arg->getDicElt("type"));

    if( !out_cols.insertCol(icol, in_table, name, fmla, itype) ) {
        _ctx.logError("Failed to set formula for insertcol operation");
        return false;
    }

    if( sVariant * fmt_val = arg->getDicElt("format") ) {
        if( unlikely(!out_cols.setFormat(icol, fmt_val->asString())) ) {
            _ctx.logError("Invalid format argument for insertcol operation");
            return false;
        }
    }

    return true;
}

bool PrintStage::op_appendcol(const sTabular * in_table, sVariant * arg)
{
    if( !arg ) {
        _ctx.logError("Missing or invalid arg parameter for appendcol operation");
        return false;
    }

    ast::Node * fmla = ParseUtils::parseFormulaArg(arg->getDicElt("formula"), _ctx, "formula argument for appendcol operation");
    if( unlikely(!fmla) )
        return false;

    const char * name = 0;
    sStr name_buf;
    if( sVariant * name_val = arg->getDicElt("name") ) {
        name = ParseUtils::evalStringOrFormulaArg(name_buf, name_val, _ctx);
    } else {
        const char * input_col_name;
        idx input_col_num;
        if( fmla->isDollarCall(&input_col_name, &input_col_num) ) {
            if( input_col_name ) {
                name = input_col_name;
            } else {
                name = in_table->printTopHeader(name_buf, input_col_num);
            }
        }
    }

    sVariant::eType itype = ParseUtils::parseTypeArg(arg->getDicElt("type"));

    out_cols.appendCol(name, in_table, fmla, itype);

    if( sVariant * fmt_val = arg->getDicElt("format") ) {
        if( unlikely(!out_cols.setFormat(out_cols.dim() - 1, fmt_val->asString())) ) {
            _ctx.logError("Invalid format argument for appendcol operation");
            return false;
        }
    }

    return true;
}

bool PrintStage::op_hidecol(const sTabular * in_table, sVariant * arg)
{
    if( !arg ) {
        _ctx.logError("Missing or invalid arg parameter for hidecol operation");
        return false;
    }

    sVec<idx> icols;
    const char * argname = "col";

    if( sVariant * cols_val = arg->getDicElt("cols") ) {
        argname = "cols";
        if( cols_val->isString() && !strcmp(cols_val->asString(), "*") ) {
            out_cols.deleteAll();
        } else if( cols_val->isList() ) {
            for(idx i = 0; i < cols_val->dim(); i++) {
                if( !ParseUtils::parseColsArg(icols, cols_val->getListElt(i), &out_cols, _ctx, true) ) {
                    _ctx.logError("Invalid cols argument element %"DEC" for hidecol operation", i);
                    return false;
                }
            }
        } else if( !ParseUtils::parseColsArg(icols, cols_val, &out_cols, _ctx, true) ) {
            _ctx.logError("Invalid cols argument for hidecol operation");
            return false;
        }
    } else {
        if( !ParseUtils::parseColsArg(icols, arg->getDicElt("col"), &out_cols, _ctx, false) ) {
            _ctx.logError("Invalid col argument for hidecol operation");
            return false;
        }
    }

    bool negate = false;
    if( sVariant * negate_val = arg->getDicElt("negate") ) {
        negate = negate_val->asBool();
    }
    if( negate ) {
        sSort::sort<idx>(icols.dim(), icols.ptr());

        sVec<idx> negated_icols;

        for(idx icol = 0, i = 0; icol < out_cols.dim(); icol++) {
            if( i < icols.dim() && icol == icols[i] ) {
                do {
                    i++;
                } while( i < icols.dim() && icol == icols[i] );
                continue;
            }
            *negated_icols.add(1) = icol;
        }
        icols.destroy();
        icols.borrow(&negated_icols);
    }

    // we must go through deletion list from highest to lowest so each deletion does not invalidate following column numbers
    if( icols.dim() ) {
        sSort::sort<idx>(icols.dim(), icols.ptr());
        idx prev_icol = -sIdxMax;
        for(idx i = icols.dim() - 1; i >= 0; i--) {
            if( icols[i] == prev_icol ) {
                continue;
            }
#if 0
            {
                sStr buf;
                out_cols.printDump(buf, false, "    ");
                fprintf(stderr, "out_cols = %s\n", buf.ptr(0));
                buf.cut(0);
            }
#endif
            _ctx.logTrace("hidecol operation: hiding column #%"DEC" (name \"%s\")", icols[i], out_cols.getName(icols[i]));
            if( !out_cols.validCol(icols[i]) ) {
                _ctx.logError("Invalid %s argument for hidecol operation", argname);
                return false;
            }
            if( !out_cols.deleteCol(icols[i]) ) {
                _ctx.logError("Failed hidecol operation");
                return false;
            }

            prev_icol = icols[i];
        }
    }

    return true;
}

bool PrintStage::op_renamecol(const sTabular * in_table, sVariant * arg)
{
    if( !arg ) {
        _ctx.logError("Missing or invalid arg parameter for renamecol operation");
        return false;
    }

    sVec<idx> icols;
    if( !ParseUtils::parseColsArg(icols, arg->getDicElt("col"), &out_cols, _ctx, false) ) {
        _ctx.logError("Invalid col argument for renamecol operation");
        return false;
    }

    if( icols.dim() == 1 ) {
        idx icol = icols[0];

        const char * name_to = 0;
        sStr name_buf;
        if( sVariant * name_val = arg->getDicElt("to") ) {
            name_to = ParseUtils::evalStringOrFormulaArg(name_buf, name_val, _ctx);
        }

        if( !out_cols.renameCol(icol, name_to) ) {
            _ctx.logError("Failed renamecol operation");
            return false;
        }
    } else {
        _ctx.logDebug("Optional col argument for renamecol operation failed to match; operation is no-op");
    }

    return true;
}

bool PrintStage::op_movecol(const sTabular * in_table, sVariant * arg)
{
    if( !arg ) {
        _ctx.logError("Missing or invalid arg parameter for movecol operation");
        return false;
    }

    sVec<idx> icols_from;
    if( !ParseUtils::parseColsArg(icols_from, arg->getDicElt("col"), &out_cols, _ctx, false) ) {
        _ctx.logError("Invalid col argument for movecol operation");
        return false;
    }

    if( icols_from.dim() == 1 ) {
        idx icol_from = icols_from[0];
        idx icol_to = -sIdxMax;
        if( sVariant * to_val = arg->getDicElt("to") )
            icol_to = to_val->asInt();

        if( !out_cols.validCol(icol_to) ) {
            _ctx.logError("Invalid to argument for movecol operation");
            return false;
        }

        if( !out_cols.moveCol(icol_from, icol_to) ) {
            _ctx.logError("Failed movecol operation");
            return false;
        }
    } else {
        _ctx.logDebug("Optional col argument for movecol operation failed to match; operation is no-op");
    }

    return true;
}

static const char * opNames = "predefine"_
"load"_
"ionwander"_
"sort"_
"inputsort"_
"collapserows"_
"filter"_
"colfilter"_
"slice"_
"transpose"_
"glue"_
"setcell"_
"definecol"_
"insertcol"_
"appendcol"_
"hidecol"_
"renamecol"_
"movecol"_
"rowcategory"_
"colcategory"__;

enum EOpName
{
    eOp_unknown = -1,
    eOp_predefine,
    eOp_load,
    eOp_ionwander,
    eOp_sort,
    eOp_inputsort_deprecated,
    eOp_collapserows,
    eOp_filter,
    eOp_colfilter,
    eOp_slice,
    eOp_transpose,
    eOp_glue,
    eOp_setcell,
    eOp_definecol,
    eOp_insertcol,
    eOp_appendcol,
    eOp_hidecol,
    eOp_renamecol,
    eOp_movecol,
    eOp_rowcategory,
    eOp_colcategory,
};

static EOpName parseOpName(sVariant * op_val)
{
    idx op = eOp_unknown;
    if( op_val && sString::compareChoice(op_val->asString(), opNames, &op, true, 0, true) >= 0 )
        return static_cast<EOpName>(op);

    return eOp_unknown;
}

#define ENSURE_UNREINTERPRET_STAGE \
do { \
    if( reinterpret_stage ) { \
        reinterpret_stage = 0; \
        *_commands.add(1) = new ReinterpretStage(*this, true); \
    } \
} while (0)

#define ENSURE_PRINT_STAGE \
do { \
    ENSURE_UNREINTERPRET_STAGE; \
    if( !print_stage ) { \
        print_stage = new PrintStage(*this); \
        *_commands.add(1) = print_stage; \
        reinterpret_stage = 0; \
    } \
} while (0)

bool ExecContext::prepareCommands()
{
    PrintStage * print_stage = 0;
    ReinterpretStage * reinterpret_stage = 0;

    // before doing anything else - evaluate predefines, and substitute { "tmpl" : "..." }
    sStr name_buf, json_buf;
    for(idx i = 0; i < _tqs.dim(); i++) {
        sVariant * _tqs_op = _tqs.getListElt(i);
        if( !_tqs_op || !_tqs_op->isDic() )
            continue;

        if( !substituteTemplate(_tqs_op) ) {
            return false;
        }

        if( parseOpName(_tqs_op->getDicElt("op")) == eOp_predefine ) {
#ifdef _DEBUG
            fprintf(stderr, "Predefine operation\n");
#endif
            sVariant * arg_val = _tqs_op->getDicElt("arg");
            if( !arg_val ) {
                logError("Missing or invalid arg parameter for predefine operation");
                return false;
            }

            name_buf.cut0cut();
            if( sVariant * name_val = arg_val->getDicElt("name") ) {
                name_val->print(name_buf, sVariant::eUnquoted);
            }
            idx name_len = name_buf.length();
            for(idx i = 0; i < name_len; i++) {
                if( (name_buf[i] >= 'a' && name_buf[i] <= 'z') || (name_buf[i] >= 'A' && name_buf[i] <= 'Z') || name_buf[i] == '_' )
                    continue;
                if( i && name_buf[i] >= '0' && name_buf[i] <= '9' )
                    continue;
                name_len = 0;
            }

            if( !name_len ) {
                logError("Missing or invalid name argument \"%s\" for predefine operation", name_buf.ptr());
                return false;
            }

            sVariant value;
            if( sVariant * pvalue = arg_val->getDicElt("value") ) {
                value = *pvalue;
            } else {
                ast::Node * fmla = ParseUtils::parseFormulaArg(arg_val->getDicElt("formula"), *this, "formula argument for predefine operation");
                if( unlikely(!fmla) ) {
                    return false;
                }
                if( unlikely(!fmla->eval(value, _qlang_ctx)) ) {
                    sStr err;
                    _qlang_ctx.printError(err);
                    logError("Query language error when evaluating formula for predefine operation: %s\n", err.ptr());
                    _qlang_ctx.clearError();
                    return false;
                }
                delete fmla;
            }
            if( unlikely(!_qlang_ctx.registerBuiltinValue(name_buf, value)) ) {
                logError("Failed predefine operation for %s", name_buf.ptr());
                return false;
            }

            json_buf.cut0cut();
            logDebug("/* predefine */ %s = %s;\n", name_buf.ptr(), value.print(json_buf, sVariant::eJSON));
        }
    }

    for(idx i = 0; i < _tqs.dim(); i++) {
        sVariant * _tqs_op = _tqs.getListElt(i);
        if( !_tqs_op || !_tqs_op->isDic() )
            continue;

        sVariant * op_name_val = _tqs_op->getDicElt("op");
        sVariant * arg_val = _tqs_op->getDicElt("arg");
        switch(parseOpName(op_name_val)) {
            case eOp_predefine:
                // already handled done in previous pass
                break;
            case eOp_load: {
                LoadStage * stage = new LoadStage(*this);
                *_commands.add(1) = stage;
                if( unlikely(!stage->init(op_name_val->asString(), arg_val)) ) {
                    return false;
                }
            }
                break;
            case eOp_ionwander: {
                print_stage = 0;
                IonWanderStage * stage = new IonWanderStage(*this);
                *_commands.add(1) = stage;
                if( unlikely(!stage->init(op_name_val->asString(), arg_val)) ) {
                    return false;
                }
            }
                break;
            case eOp_sort:
            case eOp_inputsort_deprecated: {
                ENSURE_UNREINTERPRET_STAGE;
                print_stage = 0;

                SortStage * stage = new SortStage(*this);
                *_commands.add(1) = stage;
                if( unlikely(!stage->init(op_name_val->asString(), arg_val)) ) {
                    return false;
                }
            }
                break;
            case eOp_collapserows:
                // TODO
                ENSURE_UNREINTERPRET_STAGE;
                print_stage = 0;
                break;
            case eOp_slice:
                ENSURE_UNREINTERPRET_STAGE;
                print_stage = 0;
                {
                    SliceStage * stage = new SliceStage(*this);
                    *_commands.add(1) = stage;
                    if( unlikely(!stage->init(op_name_val->asString(), arg_val)) ) {
                        return false;
                    }
                }
                break;
            case eOp_transpose:
                ENSURE_UNREINTERPRET_STAGE;
                print_stage = 0;
                {
                    TransposeStage * stage = new TransposeStage(*this);
                    *_commands.add(1) = stage;
                    if( unlikely(!stage->init(op_name_val->asString(), arg_val)) ) {
                        return false;
                    }
                }
                break;
            case eOp_glue:
                ENSURE_UNREINTERPRET_STAGE;
                print_stage = 0;
                {
                    GlueStage * stage = new GlueStage(*this);
                    *_commands.add(1) = stage;
                    if( unlikely(!stage->init(op_name_val->asString(), arg_val)) ) {
                        return false;
                    }
                }
                break;
            case eOp_setcell:
                print_stage = 0;
                {
                    SetCellStage * stage = new SetCellStage(*this);
                    *_commands.add(1) = stage;
                    if( unlikely(!stage->init(op_name_val->asString(), arg_val)) ) {
                        return false;
                    }
                }
                break;
            case eOp_filter:
                // involves column names and formulas which will need to be parsed when print stage's input table becomes available
                ENSURE_PRINT_STAGE;
                print_stage->addOpArg(_tqs_op, true);
                break;
            case eOp_colfilter:
            case eOp_definecol:
            case eOp_insertcol:
            case eOp_appendcol:
            case eOp_hidecol:
            case eOp_renamecol:
            case eOp_movecol:
            case eOp_rowcategory:
            case eOp_colcategory:
                // these all involve column names and formulas which will need to be parsed when print stage's input table becomes available
                ENSURE_PRINT_STAGE;
                print_stage->addOpArg(_tqs_op);
                break;
            default:
                if( Command * cmd = Command::extFactory(op_name_val->asString(), *this) ) {
                    print_stage = 0;

                    if( cmd->needsInTableReinterpret() && !reinterpret_stage ) {
                        reinterpret_stage = new ReinterpretStage(*this, false);
                        *_commands.add(1) = reinterpret_stage;
                        reinterpret_stage->init(_proc.pForm->value("binary"));
                    }
                    *_commands.add(1) = cmd;

                    if( unlikely(!cmd->init(op_name_val->asString(), arg_val)) ) {
                        return false;
                    }

                    if( cmd->computesOutTable() ) {
                        reinterpret_stage = 0;
                    }
                }
#ifdef _DEBUG
                else {
                    fprintf(stderr, "Unexpected TQS command: '%s'\n", _tqs_op->asString());
                }
#endif
                break;
        }
    }

    idx out_start = sMax<idx>(0, _proc.pForm->ivalue("start"));
    idx out_cnt = sMax<idx>(0, _proc.pForm->ivalue("cnt"));
    idx out_resolution = _proc.pForm->ivalue("resolution");
    idx out_abscissa_col = _proc.pForm->ivalue("abscissaCol", -sIdxMax);
    const char * formstr_cols = _proc.pForm->value("cols");
    const char * formstr_minmaxCols = _proc.pForm->value("minmaxCols");
    const char * formstr_minmaxMainCol = _proc.pForm->value("minmaxMainCol");
    const char * formstr_srch = _proc.pForm->value("search");
    bool srchRegExp = _proc.pForm->boolvalue("searchRegExp", _proc.pForm->boolvalue("searchRegEx"));
    const char * formstr_srchCols = _proc.pForm->value("searchCols");

    // start/cnt/resolution parameters go into the final print stage
    if( out_start || out_cnt || out_resolution || sLen(formstr_cols) || sLen(formstr_minmaxCols) || sLen(formstr_minmaxMainCol) || sLen(formstr_srch) || !_top_header ) {
        // filtering must be done in a separate stage before resolution and minmaxing
        if( print_stage && print_stage->hasRowFilters() && out_resolution ) {
            if( sLen(formstr_srch) ) {
                // search is another synonym for filter
                ENSURE_PRINT_STAGE;
                print_stage->formstr_srch = formstr_srch;
                print_stage->srchRegExp = srchRegExp;
                print_stage->formstr_srchCols = formstr_srchCols;
                formstr_srch = formstr_srchCols = 0;
                srchRegExp = false;
            }
            print_stage = 0;
        }
        ENSURE_PRINT_STAGE;
        print_stage->out_start = out_start;
        print_stage->out_cnt = out_cnt;
        print_stage->out_resolution = out_resolution;
        print_stage->out_abscissa_col = out_abscissa_col;
        print_stage->formstr_cols = formstr_cols;
        print_stage->formstr_minmaxCols = formstr_minmaxCols;
        print_stage->formstr_minmaxMainCol = formstr_minmaxMainCol;
        print_stage->formstr_srch = formstr_srch;
        print_stage->srchRegExp = srchRegExp;
        print_stage->formstr_srchCols = formstr_srchCols;
        print_stage->print_top_header = _top_header;
    }

    return true;
}

bool LoadStage::sub_init(sVariant * arg)
{
    sHiveId obj_id;
    idx req = 0;
    idx is_grp = false;
    const char * tbl_name = 0;
    bool is_glob = false;
    if( sVariant * obj_arg = arg->getDicElt("obj") ) {
        obj_arg->asHiveId(&obj_id);
    }
    if( sVariant * req_arg = arg->getDicElt("req") ) {
        req = req_arg->asInt();
        is_grp = false;
    } else if( sVariant * grp_arg = arg->getDicElt("grp") ) {
        req = grp_arg->asInt();
        is_grp = true;
    }
    if( sVariant * glob_arg = arg->getDicElt("glob") ) {
        tbl_name = glob_arg->asString();
        is_glob = true;
    } else if( sVariant * tbl_arg = arg->getDicElt("tbl") ) {
        tbl_name = tbl_arg->asString();
    } else {
        tbl_name = "_.csv";
    }
    if( obj_id ) {
        if( is_glob ) {
            if( _ctx.allocateRequestLoadObjTables(loader_handles, obj_id, tbl_name) < 0 ) {
                return false;
            }
        } else {
            idx loader_handle = *loader_handles.add(1) = _ctx.allocateLoaderHandle();
            _ctx.requestLoadObjTable(loader_handle, obj_id, tbl_name);
        }
    } else if( req ) {
        if( is_glob ) {
            if( _ctx.allocateRequestLoadReqTables(loader_handles, req, is_grp, tbl_name) < 0 ) {
                return false;
            }
        } else {
            idx loader_handle = *loader_handles.add(1) = _ctx.allocateLoaderHandle();
            _ctx.requestLoadReqTable(loader_handle, req, is_grp, tbl_name);
        }
    } else {
        return false;
    }
    return true;
}

bool LoadStage::init(const char * op_name, sVariant * arg)
{
    if( sVariant * concat_val = arg->getDicElt("concat") ) {
        if( !concat_val->isList() ) {
            _ctx.logError("Invalid concat arg for %s operation", op_name);
            return false;
        }
        for(idx i=0; i<concat_val->dim(); i++) {
            if( !sub_init(concat_val->getListElt(i)) ) {
                _ctx.logError("Invalid concat arg %"DEC" for %s operation", i, op_name);
                return false;
            }
        }
    } else {
        if( !sub_init(arg) ) {
            _ctx.logError("Invalid arg for %s operation", op_name);
            return false;
        }
    }
    return true;
}

bool LoadStage::compute(sTabular * in_table)
{
#ifdef _DEBUG
    fprintf(stderr, "Load stage\n");
#endif

    sCatTabular * cat = 0;

    if( in_table && in_table->rows() + in_table->dimTopHeader() ) {
        _wraps_in_table = true;
        cat = dynamic_cast<sCatTabular*>(in_table);
        if( !cat ) {
            cat = new sCatTabular;
            cat->pushSubTable(in_table, false);
        }
        _out_table = cat;
    } else {
        _wraps_in_table = false;
        in_table = 0;
    }

    if( loader_handles.dim() == 1 ) {
        if( cat ) {
            cat->pushSubTable(_ctx.releaseLoadedTable(loader_handles[0]), true);
        } else {
            _out_table = _ctx.releaseLoadedTable(loader_handles[0]);
        }
    } else if ( loader_handles.dim() > 1 ) {
        if( !cat ) {
            _out_table = cat = new sCatTabular;
        }
        for(idx i=0; i<loader_handles.dim(); i++) {
            cat->pushSubTable(_ctx.releaseLoadedTable(loader_handles[i]), true);
        }
    } else {
        return false;
    }
    return true;
}

bool IonWanderStage::init(const char * op_name, sVariant * arg)
{
    sStr buf;
    if( !arg ) {
        _ctx.logError("%s operation: missing argument", op_name);
        return false;
    }

    if( const sVariant * params_val = arg->getDicElt("qryParams") ) {
        if( params_val->isDic() ) {
            for(idx i=0; i<params_val->dim(); i++) {
                const char * param_name = params_val->getDicKey(i);
                const sVariant * param_val = params_val->getDicElt(i);
                buf.cut(0);
                param_val->print(buf, sVariant::eUnquoted);
                buf.add0(2);
                form.inp(param_name, buf.ptr(), buf.length());
            }
        }
    }

    if( sVariant * ids_val = arg->getDicElt("ionId") ) {
        if( ids_val->isScalar() || ids_val->isList() ) {
            ids_val->asHiveIds(ion_ids);
        } else if( ids_val->isDic() && ids_val->getDicElt("objQry") ) {
            const char * qry = ids_val->getDicElt("objQry")->asString();
            if( qry && *qry ) {
                if( !_ctx.qlangParser().parse(qry) ) {
                    _ctx.logError("%s operation: failed to parse ionId objQry argument: %s", op_name, _ctx.qlangParser().getErrorStr());
                    return false;
                }
                ion_ids_qry_root = _ctx.qlangParser().releaseAstRoot();
            }
        } else {
            _ctx.logError("%s operation: expected list or query for ionId argument", op_name);
            return false;
        }
    } else {
        _ctx.logError("%s operation: missing ionId argument", op_name);
        return false;
    }

    if( const sVariant * file_val = arg->getDicElt("ionFile") ) {
        buf.cut(0);
        file_val->print(buf, sVariant::eUnquoted);
        buf.add0(2);
        form.inp("ionfile", buf.ptr(), buf.length());
    }

    if( sVariant * qry_val = arg->getDicElt("qry") ) {
        need_qry_id = false;
        buf.cut(0);
        qry_val->print(buf, sVariant::eUnquoted);
        buf.add0(2);
        form.inp("qry", buf.ptr(), buf.length());
    } else if( sVariant * id_val = arg->getDicElt("qryId") ) {
        if( id_val->isScalar() ) {
            id_val->asHiveId(&qry_id);
        } else if( id_val->isDic() && id_val->getDicElt("objQry") ) {
            const char * qry_qry = id_val->getDicElt("objQry")->asString();
            if( qry_qry && *qry_qry ) {
                if( !_ctx.qlangParser().parse(qry_qry) ) {
                    _ctx.logError("%s operation: failed to parse qryId objQry argument: %s", op_name, _ctx.qlangParser().getErrorStr());
                    return false;
                }
                qry_ids_qry_root = _ctx.qlangParser().releaseAstRoot();
            }
        } else {
            _ctx.logError("%s operation: expected single ID or query for qryId argument", op_name);
            return false;
        }

        if( const sVariant * file_val = arg->getDicElt("qryFile") ) {
            buf.cut(0);
            file_val->print(buf, sVariant::eUnquoted);
            buf.add0(2);
            form.inp("qryfile", buf.ptr(), buf.length());
        } else {
            _ctx.logError("%s operation: missing qryFile argument", op_name);
            return false;
        }
    } else {
        _ctx.logError("%s operation: missing qryId argument", op_name);
        return false;
    }

    if( const sVariant * cnt_val = arg->getDicElt("cnt") ) {
        buf.cut(0);
        buf.addNum(cnt_val->asInt());
        buf.add0(2);
        form.inp("cnt", buf.ptr(), buf.length());
    }

    return true;
}

bool IonWanderStage::compute(sTabular * in_table)
{
    sStr buf;

    if( ion_ids_qry_root ) {
        sVariant ion_ids_val;
        if( !ion_ids_qry_root->eval(ion_ids_val, _ctx.qlangCtx()) ) {
            sStr err_buf;
            _ctx.logError("ionwander operation: failed to evaluate ionId objQry argument: %s", _ctx.qlangCtx().printError(err_buf));
            _ctx.qlangCtx().clearError();
            return false;
        }
        ion_ids_val.asHiveIds(ion_ids);
    }

    sHiveId::printVec(buf, ion_ids);
    buf.add0(2);
    form.inp("ionid", buf.ptr(), buf.length());

    if( need_qry_id ) {
        if( qry_ids_qry_root ) {
            sVariant qry_id_val;
            if( !ion_ids_qry_root->eval(qry_id_val, _ctx.qlangCtx()) ) {
                sStr err_buf;
                _ctx.logError("ionwander operation: failed to evaluate qryId objQry argument: %s", _ctx.qlangCtx().printError(err_buf));
                _ctx.qlangCtx().clearError();
                return false;
            }
            qry_id_val.asHiveId(&qry_id);
        }

        buf.cut(0);
        qry_id.print(buf);
        buf.add0(2);
        form.inp("qryid", buf.ptr(), buf.length());
    }

#ifdef _DEBUG
    fprintf(stderr, "sHiveIon::Cmd form:\n");
    for (idx i=0; i<form.dim(); i++) {
        const char * key = static_cast<const char*>(form.id(i));
        const char * value = form.value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
#endif

    sIO csv_buf;
    sHiveIon hive_ion(_ctx.qproc().user);
    if( hive_ion.Cmd(&csv_buf, "wander", &form) != 1 ) {
        _ctx.logError("ionwander operation: wander failed");
        return false;
    }
    if( !csv_buf.length() ) {
        _ctx.logError("ionwander operation: wander output is empty");
        return false;
    }

    sTxtTbl * tbl = new sTxtTbl;
    tbl->borrowFile(csv_buf.mex());
    tbl->parseOptions().flags = sTblIndex::fSaveRowEnds;
    if( qry_prints_hdr ) {
        tbl->parseOptions().flags |= sTblIndex::fTopHeader; // most queries do not print a header
        tbl->parseOptions().dimTopHeader = 1; // ... and that header has 1 row
    }
    if( !tbl->parse() ) {
        _ctx.logError("ionwander operation: failed to parse output of wander");
        return false;
    }

    _out_table = tbl;
    return true;
}

bool SortStage::compute(sTabular * in_table)
{
    if( !in_table ) {
        _out_table = 0;
        _ctx.logInfo("Missing input table for sort operation");
        return false;
    }

    _ctx.qlangCtx().setTable(in_table);

    sReorderedTabular * reordered_table = dynamic_cast<sReorderedTabular*>(in_table);
    if( !reordered_table ) {
        reordered_table = new sReorderedTabular(in_table, false);
    }
    _out_table = reordered_table;
    return sorter.sort(reordered_table);
}

const char * SliceStage::method_names = "firstEquals"_
"firstLessThan"_
"firstGreaterThan"_
"lastEquals"_
"lastLessThan"_
"lastGreaterThan"__;

SliceStage::EMethod SliceStage::parseMethodName(sVariant * method_arg)
{
    if( !method_arg || method_arg->isNull() ) {
        return eMethod_none;
    }
    idx op = eMethod_unknown;
    if( sString::compareChoice(method_arg->asString(), method_names, &op, true, 0, true) >= 0 )
        return static_cast<EMethod>(op);
    return eMethod_unknown;
}

bool SliceStage::init(const char * op_name, sVariant * arg)
{
    if( !arg ) {
        return false;
    }
    sVariant * start_arg = arg->getDicElt("start");
    sVariant * end_arg = arg->getDicElt("end");
    if( !start_arg && !end_arg ) {
        return false;
    }
    if( start_arg ) {
        start_method = parseMethodName(start_arg->getDicElt("method"));
        if( start_method == eMethod_unknown ) {
            _ctx.logError("Missing 'method' value for start argument of %s operation", op_name);
            return false;
        } else if( start_method != eMethod_none ) {
            if( !start_arg->getElt("col", start_col_arg) || !start_arg->getElt("value", start_value) ) {
                _ctx.logError("Missing 'col' or 'value' fields for start argument of %s operation", op_name);
                return false;
            }
        }
    }
    if( end_arg ) {
        end_method = parseMethodName(end_arg->getDicElt("method"));
        if( end_method == eMethod_unknown ) {
            _ctx.logError("Missing 'method' value for end argument of %s operation", op_name);
            return false;
        } else if( end_method != eMethod_none ) {
            if( !end_arg->getElt("col", end_col_arg) || !end_arg->getElt("value", end_value) ) {
                _ctx.logError("Missing 'col' or 'value' fields for end argument of %s operation", op_name);
                return false;
            }
        }
    }
    return true;
}

idx SliceStage::findRow(sTabular * in_table, SliceStage::EMethod method, sVariant & col_arg, sVariant & value, idx irow_default)
{
    if( method == eMethod_none || in_table->rows() == 0 ) {
        return irow_default;
    }

    sVec<idx> parsed_colnums;
    ParseUtils::parseColsArg(parsed_colnums, &col_arg, in_table, _ctx, false);
    if( !parsed_colnums.dim() ) {
        _ctx.logError("Invalid or missing col field for slice operation");
        return false;
    }
    idx icol = parsed_colnums[0];

    // is the column ascending or descending?
    sVariant rowval, rowval2;
    in_table->val(rowval, 0, icol);
    in_table->val(rowval2, in_table->rows() - 1, icol);
    bool descending = rowval > rowval2;

    idx imin = 0;
    idx imax = in_table->rows() - 1;
    idx ibest = -sIdxMax;

    while( imax >= imin ) {
        idx imid = (imin + imax) / 2;
        in_table->val(rowval, imid, icol);
        idx cmp_res = rowval.cmp(value);
        idx next_search = -cmp_res;

        switch(method) {
            case eMethod_first_eq:
                if( cmp_res == 0 ) {
                    next_search = -1;
                    ibest = imid;
                }
                break;
            case eMethod_first_lt:
                if( cmp_res < 0 ) {
                    next_search = -1;
                    ibest = imid;
                } else {
                    next_search = 1;
                }
                break;
            case eMethod_first_gt:
                if( cmp_res > 0 ) {
                    next_search = -1;
                    ibest = imid;
                } else {
                    next_search = 1;
                }
                break;
            case eMethod_last_eq:
                if( cmp_res == 0 ) {
                    next_search = 1;
                    ibest = imid;
                }
                break;
            case eMethod_last_lt:
                if( cmp_res < 0 ) {
                    next_search = 1;
                    ibest = imid;
                } else {
                    next_search = -1;
                }
                break;
            case eMethod_last_gt:
                if( cmp_res > 0 ) {
                    next_search = 1;
                    ibest = imid;
                } else {
                    next_search = -1;
                }
                break;
            default:
                return -sIdxMax;
        }

        if( descending ) {
            next_search = -next_search;
        }

        if( next_search == 0 ) {
            // found!
            return imid;
        } else if( next_search < 0 ) {
            imax = imid - 1;
        } else {
            imin = imid + 1;
        }
    }

    return ibest;
}

bool SliceStage::compute(sTabular * in_table)
{
    if( !in_table ) {
        _out_table = 0;
        _ctx.logInfo("Missing input table for slice operation");
        return false;
    }

    _ctx.qlangCtx().setTable(in_table);

    idx start_row = findRow(in_table, start_method, start_col_arg, start_value, 0);
    _ctx.reportSubProgress(1, -1, 3);
    idx end_row = findRow(in_table, end_method, end_col_arg, end_value, in_table->rows() - 1);
    _ctx.reportSubProgress(2, -1, 3);

    sReorderedTabular * reordered_table = dynamic_cast<sReorderedTabular*>(in_table);
    if( !reordered_table ) {
        reordered_table = new sReorderedTabular(in_table, false);
    }

    _out_table = reordered_table;

    idx top_header = in_table->dimTopHeader();

    sVec<sReorderedTabular::TypedSource> rows_map;
    if( start_row >= 0 && end_row >= start_row ) {
        rows_map.resize(top_header + sMax<idx>(0, 1 + end_row - start_row));
    } else {
        rows_map.resize(top_header);
    }
    for(idx absrow = 0; absrow < top_header; absrow++) {
        rows_map[absrow].src = reordered_table->mapRow(absrow - top_header);
        rows_map[absrow].type = sVariant::value_STRING;
    }

    for(idx ir = 0; ir + top_header < rows_map.dim() && ir + start_row < reordered_table->rows(); ir++) {
        rows_map[ir + top_header].src = reordered_table->mapRow(ir + start_row);
        rows_map[ir + top_header].type = reordered_table->rowtype(ir + start_row);
    }

    reordered_table->borrowRowsMap(rows_map);

    return true;
}

bool TransposeStage::init(const char * op_name, sVariant * arg)
{
    if( !arg ) {
        return true;
    }
    if( sVariant * top_header_arg = arg->getDicElt("topHeader") ) {
        top_header = top_header_arg->asInt();
    }
    if( sVariant * left_header_arg = arg->getDicElt("leftHeader") ) {
        top_header = left_header_arg->asInt();
    }
    return true;
}

bool TransposeStage::compute(sTabular * in_table)
{
    if( !in_table ) {
        _out_table = 0;
        _ctx.logInfo("Missing input table for transpose operation");
        return false;
    }

    idx prev_top_header = in_table->dimTopHeader();

    sReorderedTabular * reordered_table = dynamic_cast<sReorderedTabular*>(in_table);
    if( !reordered_table ) {
        reordered_table = new sReorderedTabular(in_table, false);
    }

    _out_table = reordered_table;

    idx want_top_header = top_header < 0 ? prev_top_header : top_header;
    idx want_left_header = left_header < 0 ? 0 : left_header;
    reordered_table->setTransposed(!reordered_table->isTransposed(), want_top_header, want_left_header);

    return true;
}

bool GlueStage::init(const char * op_name, sVariant * arg)
{
    if( !arg ) {
        return true;
    }
    bool ret = true;
    if( !init(left_def, arg->getDicElt("lhs")) ) {
        ret = false;
    }
    if( !init(right_def, arg->getDicElt("rhs")) ) {
        ret = false;
    }
    return ret;
}

bool GlueStage::init(GlueStage::SourceDef & def, sVariant * arg)
{
    sVec<sHiveId> obj_ids;
    sVec<idx> data_reqs;
    sVec<idx> tablename_pos;
    sStr tablename_buf;

    *tablename_pos.add(1) = -1;

    if( arg ) {
        if( sVariant * objs_val = arg->getDicElt("obj") ) {
            def.uses_loader_handle = true;
            if( !objs_val->asHiveIds(obj_ids) ) {
                _ctx.logError("Empty or invalid objs argument for glue operation");
                return false;
            }
        }
        if( sVariant * reqs_val = arg->getDicElt("req") ) {
            def.uses_loader_handle = true;
            if( reqs_val->isScalar() ) {
                *data_reqs.add(1) = reqs_val->asInt();
            } else if( reqs_val->isList() ) {
                data_reqs.add(reqs_val->dim());
                for(idx i=0; i<reqs_val->dim(); i++) {
                    data_reqs[i] = reqs_val->getListElt(i)->asInt();
                }
            } else {
                _ctx.logError("Empty or invalid reqs argument for glue operation");
                return false;
            }
        }

        if( sVariant * tbls_val = arg->getDicElt("tbl")) {
            if( tbls_val->isScalar() ) {
                tablename_pos[0] = 0;
                tbls_val->print(tablename_buf, sVariant::eUnquoted);
            } else if( tbls_val->isList() ) {
                tablename_pos.resize(tbls_val->dim());
                for(idx i=0; i<tbls_val->dim(); i++) {
                    tablename_pos[i] = tablename_buf.length();
                    tbls_val->print(tablename_buf, sVariant::eUnquoted);
                    tablename_buf.add0();
                }
            }
        }
    }

    if( def.uses_loader_handle ) {
        def.loader_handle = _ctx.allocateLoaderHandle();
        for(idx i = 0; i < data_reqs.dim(); i++) {
             for(idx j=0; j<tablename_pos.dim(); j++) {
                 _ctx.requestLoadReqTable(def.loader_handle, data_reqs[i], false, tablename_pos[j] >= 0 ? tablename_buf.ptr(tablename_pos[j]) : 0);
             }
         }

         for(idx i = 0; i < obj_ids.dim(); i++) {
             if( !obj_ids[i] ) {
                 continue;
             }
             for(idx j=0; j<tablename_pos.dim(); j++) {
                 _ctx.requestLoadObjTable(def.loader_handle, obj_ids[i], tablename_pos[j] >= 0 ? tablename_buf.ptr(tablename_pos[j]) : 0);
             }
         }
    }

    const char * fmla_text = "$0";
    if( arg ) {
        def.hidecols_arg = arg->getDicElt("hidecol");
        if( sVariant * fmla_val = arg->getDicElt("formula") ) {
            fmla_text = fmla_val->asString();
        }
    }

    def.key_def.shared_fmla = new SharedFmla(ParseUtils::parseFormulaArg(fmla_text, _ctx, "formula argument for glue operation"), 1);
    if( !def.key_def.getFormula() ) {
        return false;
    }

    return true;
}

sTabular * GlueStage::loadInputTable(GlueStage::SourceDef & def, sTabular * in_table)
{
    sTabular * tbl = def.uses_loader_handle ? _ctx.getLoadedTable(def.loader_handle) : in_table;
    if( tbl ) {
        def.key_def.setFormula(tbl, def.key_def.getFormula());
    }
    return tbl;
}

bool GlueStage::evalKey(sVariant & out_val, GlueStage::SourceDef & def, sTabular * tbl, idx ir)
{
    if( def.key_def.getFormula() ) {
        _ctx.qlangCtx().setTable(tbl);
        _ctx.qlangCtx().setInRow(ir);
        if( unlikely(!def.key_def.getFormula()->eval(out_val, _ctx.qlangCtx())) ) {
            sStr err_str;
            // a formula failing is a non-fatal event; we log and continue
            _ctx.logInfo("Failed formula in glue operation: %s", _ctx.qlangCtx().printError(err_str));
            out_val.setNull();
            _ctx.qlangCtx().clearError();
            return false;
        }
        return true;
    } else {
        return tbl->val(out_val, ir, def.key_def.input_col);
    }
}

bool GlueStage::makeColMap(sVec<idx> & out_col_map, GlueStage::SourceDef & def, const sTabular * tbl)
{
    sVariant * arg = def.hidecols_arg;
    sVec<idx> hide_list;

    if( arg ) {
        if( arg->isString() && !strcmp(arg->asString(), "*") ) {
            // no cols!
            out_col_map.cut(0);
            return true;
        } else if( arg->isList() ) {
            for(idx i = 0; i < arg->dim(); i++) {
                if( !ParseUtils::parseColsArg(hide_list, arg->getListElt(i), tbl, _ctx, true) ) {
                    _ctx.logError("Invalid hidecols argument for glue operation");
                    return false;
                }
            }
        } else if( !ParseUtils::parseColsArg(hide_list, arg, tbl, _ctx, true) ) {
            _ctx.logError("Invalid hidecols argument for glue operation");
            return false;
        }
    }

    sDic<bool> hide_dic;
    for(idx i=0; i<hide_list.dim(); i++) {
        if( hide_list[i] >= -tbl->dimLeftHeader() && hide_list[i] < tbl->cols() ) {
            hide_dic.set(hide_list.ptr(i), sizeof(idx));
        }
    }

    idx num_abscols = tbl->dimLeftHeader() + tbl->cols() - hide_dic.dim();
    out_col_map.resize(num_abscols);
    out_col_map.cut(num_abscols);

    idx abscol = 0;
    for(idx ic = -tbl->dimLeftHeader(); ic < tbl->cols(); ic++) {
        if( !hide_dic.get(&ic, sizeof(idx)) ) {
            out_col_map[abscol++] = ic;
        }
    }
    assert(abscol == out_col_map.dim());

    return true;
}

bool GlueStage::compute(sTabular * in_table)
{
    if( !in_table ) {
        _out_table = 0;
        _ctx.logInfo("Missing input table for glue operation");
        return false;
    }

    if( left_def.uses_loader_handle && right_def.uses_loader_handle ) {
        _wraps_in_table = false;
    } else {
        _wraps_in_table = true;
    }

    sTabular * left_table = loadInputTable(left_def, in_table);
    if( !left_table ) {
        _ctx.logError("glue stage failed: no left hand side table");
        return false;
    }
    sTabular * right_table = loadInputTable(right_def, in_table);
    if( !right_table ) {
        _ctx.logError("glue stage failed: no right hand side table");
        return false;
    }

    sVec<idx> left_col_map(sMex::fExactSize), right_col_map(sMex::fExactSize);
    makeColMap(left_col_map, left_def, left_table);
    makeColMap(right_col_map, right_def, right_table);

    sVec<idx> left_row_map(sMex::fExactSize), right_row_map(sMex::fExactSize);
    idx out_top_header = sMax<idx>(left_table->dimTopHeader(), right_table->dimTopHeader());
    left_row_map.resize(out_top_header + left_table->rows());
    right_row_map.resize(out_top_header + sMax<idx>(left_table->rows(), right_table->rows()));

    for(idx ir=-out_top_header; ir<0; ir++) {
        idx absrow = ir + out_top_header;
        left_row_map[absrow] = right_row_map[absrow] = ir;
    }

    for(idx ir=0; ir<left_row_map.dim() - out_top_header; ir++) {
        idx absrow = ir + out_top_header;
        left_row_map[absrow] = ir;
    }

    for(idx ir=0; ir<right_row_map.dim() - out_top_header; ir++) {
        idx absrow = ir + out_top_header;
        right_row_map[absrow] = -sIdxMax;
    }

    sDic<idx> real_key_absrows, string_key_absrows;
    idx null_key_absrow = -sIdxMax;
    sStr buf;
    sVariant key_val;
    for(idx ir=left_table->rows() - 1; ir >= 0; ir--) {
        idx absrow = ir + out_top_header;
        evalKey(key_val, left_def, left_table, ir);
#if 0
        fprintf(stderr, "Left ir=%"DEC" key=%s\n", ir, key_val.asString());
#endif
        if( unlikely(key_val.isNull()) ) {
            null_key_absrow = absrow;
        } else if( key_val.isNumeric() ) {
            real r = key_val.asReal();
            *real_key_absrows.set(&r, sizeof(r)) = absrow;
        } else {
            const char * s = key_val.asString();
            *string_key_absrows.setString(s) = absrow;
        }
    }

    sVec<idx> unmatched_right_rows;
    for(idx ir=right_table->rows() - 1; ir >= 0; ir--) {
        idx absrow = -sIdxMax;
        evalKey(key_val, right_def, right_table, ir);
#if 0
        fprintf(stderr, "Right ir=%"DEC" key=%s\n", ir, key_val.asString());
#endif
        if( unlikely(key_val.isNull()) ) {
            absrow = null_key_absrow;
        } else if( key_val.isNumeric() ) {
            real r = key_val.asReal();
            idx * pabsrow = real_key_absrows.get(&r, sizeof(r));
            if( pabsrow ) {
                absrow = *pabsrow;
            }
        } else {
            const char * s = key_val.asString();
            idx * pabsrow = string_key_absrows.get(s);
            if( pabsrow ) {
                absrow = *pabsrow;
            }
        }
        if( absrow >= 0 && absrow < right_row_map.dim() ) {
            right_row_map[absrow] = ir;
        } else {
            *unmatched_right_rows.add(1) = ir;
        }
    }
    if( unmatched_right_rows.dim() ) {
        right_row_map.resize(unmatched_right_rows.dim() + left_row_map.dim());
        for(idx i=0; i<unmatched_right_rows.dim(); i++) {
            idx absrow = i + left_row_map.dim();
            right_row_map[absrow] = unmatched_right_rows[unmatched_right_rows.dim() - 1 - i]; // unmatched_right_rows is in reversed order
        }
    }

    sGluedTabular * glued = new sGluedTabular;
    glued->setDimHeader(out_top_header, left_table->dimLeftHeader());
    glued->setLeft(left_table, false, left_row_map, left_col_map);
    glued->setRight(right_table, false, right_row_map, right_col_map);
    _out_table = glued;

    return true;
}

bool SetCellStage::init(const char * op_name, sVariant * arg)
{
    if( !arg ) {
        _ctx.logError("Missing arg for %s operation", getName());
        return false;
    }

    if( sVariant * row_arg = arg->getDicElt("row") ) {
        irow = row_arg->asInt();
    } else {
        _ctx.logError("Missing row arg for %s operation", getName());
        return false;
    }

    col_arg = arg->getDicElt("col");
    if( !col_arg ) {
        _ctx.logError("Missing col arg for %s operation", getName());
        return false;
    }

    val_arg = arg->getDicElt("value");
    if( !val_arg ) {
        _ctx.logError("Missing col arg for %s operation", getName());
        return false;
    }

    return true;
}

bool SetCellStage::compute(sTabular * in_table)
{
    if( !in_table ) {
        setOutTable(0);
        _ctx.logInfo("Missing input table for %s operation", getName());
        return false;
    }

    sVec<idx> cols;
    if( !ParseUtils::parseColsArg(cols, col_arg, in_table, _ctx, false) ) {
        return false;
    }

    sEditedTabular * edited_table = dynamic_cast<sEditedTabular*>(in_table);
    if( !edited_table ) {
        edited_table = new sEditedTabular(in_table, false);
    }

    edited_table->editCell(irow, cols[0], val_arg->asString());

    setOutTable(edited_table);
    return true;
}

bool PrintStage::initRowsCols(sTabular * in_table)
{
    out_cols.init(in_table, formstr_cols);

    for(idx i = 0; i < tqs_ops.dim(); i++) {
        sVariant * op_name_val = tqs_ops[i]->getDicElt("op");
        sVariant * arg_val = tqs_ops[i]->getDicElt("arg");
        switch(parseOpName(op_name_val)) {
            case eOp_definecol:
                if( !op_definecol(in_table, arg_val) )
                    return false;
                break;
            case eOp_insertcol:
                if( !op_insertcol(in_table, arg_val) )
                    return false;
                break;
            case eOp_appendcol:
                if( !op_appendcol(in_table, arg_val) )
                    return false;
                break;
            case eOp_hidecol:
                if( !op_hidecol(in_table, arg_val) )
                    return false;
                break;
            case eOp_renamecol:
                if( !op_renamecol(in_table, arg_val) )
                    return false;
                break;
            case eOp_movecol:
                if( !op_movecol(in_table, arg_val) )
                    return false;
                break;
            case eOp_filter:
                has_rowfilters = true;
                if( !rowfilters.add(1)->init(arg_val, in_table, &out_cols, &out_buf, &_ctx, op_name_val->asString()) )
                    return false;
                break;
            case eOp_colfilter:
                if( !colfilters.add(1)->init(arg_val, in_table, &out_cols, &out_buf, &_ctx, op_name_val->asString()) )
                    return false;
                break;
            case eOp_rowcategory:
                if( !rowcats.add(tqs_ops[i], in_table, &out_cols, &out_buf, &_ctx) )
                    return false;
                break;
            case eOp_colcategory:
                if( !colcats.add(tqs_ops[i], in_table, &out_cols, &out_buf, &_ctx) )
                    return false;
                break;
            default:
                fprintf(stderr, "Invalid TQS operation %s\n", op_name_val->asString());
                return false;
        }
    }

    out_cols.setMinMax(formstr_minmaxCols, formstr_minmaxMainCol);

    // search is applied as the last filter
    if( sLen(formstr_srch) ) {
        if( unlikely(!rowfilters.add(1)->initSearch(formstr_srch, srchRegExp, formstr_srchCols, in_table, &out_cols, &out_buf, &_ctx)) ) {
            return false;
        }
    }

    out_start = sMax<idx>(out_start, 0);

    if( out_cnt <= 0 ) {
        out_cnt = sMax<idx>(in_table->rows() - out_start, 0);
    }

    if( out_resolution < 0 || out_resolution >= in_table->rows() ) {
        out_resolution = 0;
    }

    // row filtering and resolution are incompatible
    return !rowfilters.dim() || !out_resolution;
}

bool PrintStage::getOutValue(sVariant & result, const sTabular * in_table, idx out_icol, idx in_row) const
{
    if( const ast::Node * fmla = out_cols.getFormula(out_icol) ) {
        if( unlikely(!fmla->eval(result, _ctx.qlangCtx())) ) {
            // a formula failing is a non-fatal event; we log and continue
            sStr err_buf;
            _ctx.logInfo("Failed formula on output column %"DEC", input row %"DEC": %s", out_icol, in_row, _ctx.qlangCtx().printError(err_buf));

            result.setNull();
            _ctx.qlangCtx().clearError();
            return false;
        }
    } else {
        idx in_col = out_cols.getInputCol(out_icol);
        in_table->val(result, in_row, in_col);
    }
    return true;
}

bool PrintStage::basicPrintOutRow(sTxtTbl * out_csv_table, idx in_row, idx out_row_ifno_filter, idx out_row_ifno_start)
{
    _ctx.qlangCtx().setInRow(in_row);

    for(idx ifilt = 0; ifilt < colfilters.dim(); ifilt++) {
        if( unlikely(!colfilters[ifilt].eval(in_row, out_row_ifno_start)) ) {
            return false;
        }
    }

    if( unlikely(!colcats.categorize(in_row, out_row_ifno_filter)) ) {
        return false;
    }

    if( unlikely(!rowcats.categorize(in_row, out_row_ifno_start)) ) {
        return false;
    }

    out_buf.reset();
    const idx cols = out_buf.cols();
    for(idx out_col = 0; out_col < cols; out_col++) {
        _ctx.qlangCtx().setCol(out_col);
        out_buf.setCell(out_col, in_row);
    }
    out_buf.printTblRow(*out_csv_table);
    return true;
}

bool PrintStage::minmaxPrintOutRow(sTxtTbl * out_csv_table, idx in_row_default, idx out_row_ifno_filter, idx out_row_ifno_start, bool is_min)
{
    _ctx.qlangCtx().setInRow(in_row_default);

    for(idx ifilt = 0; ifilt < colfilters.dim(); ifilt++) {
        if( unlikely(!colfilters[ifilt].eval(in_row_default, out_row_ifno_start)) ) {
            return false;
        }
    }

    if( unlikely(!colcats.categorize(in_row_default, out_row_ifno_filter)) ) {
        return false;
    }

    if( unlikely(!rowcats.categorize(in_row_default, out_row_ifno_start)) ) {
        return false;
    }

    out_buf.reset();
    const idx cols = out_buf.cols();
    for(idx out_col = 0; out_col < cols; out_col++) {
        idx imm = out_col2imm[out_col];
        idx in_row;
        if( imm >= 0 ) {
            in_row = is_min ? minmax_cols[imm].min_in_row : minmax_cols[imm].max_in_row;
        } else {
            in_row = in_row_default;
        }
        _ctx.qlangCtx().setInRow(in_row);
        _ctx.qlangCtx().setCol(out_col);
        out_buf.setCell(out_col, in_row);
    }
    out_buf.printTblRow(*out_csv_table);
    return true;
}

bool PrintStage::compute(sTabular * in_table)
{
    sStr buf;
    sVariant result;

    if( !initRowsCols(in_table) ) {
        return false;
    }

#ifdef _DEBUG
    out_cols.printDump(buf, false, "    ");
    fprintf(stderr, "out_cols = %s\n", buf.ptr(0));
    buf.cut(0);
#endif

    sTxtTbl * out_csv_table = new sTxtTbl();
    _out_table = out_csv_table;
    out_csv_table->initWritable(1, sTblIndex::fTopHeader | sTblIndex::fRaggedEdge);

    _ctx.qlangCtx().setTable(in_table);
    _ctx.qlangCtx().setCols(&out_cols);
    _ctx.qlangCtx().setOutputPhase(true);
    _ctx.qlangCtx().setInRow(0);
    _ctx.qlangCtx().setCol(0);

    out_buf.init(&out_cols, in_table, &_ctx);
    out_buf.reset();

    idx out_cols_dim = out_cols.dim();

    idx progress_items_max = 1 + out_cnt + colfilters.dim();
    idx progress_items = 1;

    if( print_top_header ) {
        out_csv_table->setDimTopHeader(in_table->dimTopHeader());
        for(idx in_row = -in_table->dimTopHeader(); in_row < 0; in_row++) {
            _ctx.qlangCtx().setInRow(in_row);
            for(idx out_col = 0; out_col < out_cols_dim; out_col++) {
                _ctx.qlangCtx().setCol(out_col);
                buf.cut(0);
                sString::escapeForCSV(buf, out_cols.getName(out_col));
                out_csv_table->addCell(buf, buf.length());
            }
            out_csv_table->addEndRow();
        }
        _ctx.reportSubProgress(progress_items, -1, progress_items_max);
    }

    idx minmax_main = -sIdxMax;

    // For resolution calculation
    struct {
        idx inrow_pre; // singleton first row - print without resampling
        idx inrow_start_resample; // start of resample region
        idx inrow_last_resample; // end of resampling region
        idx cnt_resampled_bins; // number of resampling bins
        idx inrow_post; // singleton last row - print without resampling

        real start_resample_val;
        real last_resample_val;

        bool desc;
    } resolution_lim;
    memset(&resolution_lim, 0, sizeof(resolution_lim));

    if( out_resolution > 0 ) {
        assert(out_resolution < in_table->rows());

        if( out_resolution == 1 ) {
            // resample the whole input table into 1 bin
            resolution_lim.inrow_pre = resolution_lim.inrow_post = -sIdxMax;
            resolution_lim.inrow_start_resample = 0;
            resolution_lim.inrow_last_resample = in_table->rows() - 1;
            resolution_lim.cnt_resampled_bins = 1;
        } else if( out_resolution == 2 ) {
            // resample the whole input table except for last row into 1 bin, then print last row
            resolution_lim.inrow_pre = -sIdxMax;
            resolution_lim.inrow_post = in_table->rows() - 1;
            resolution_lim.inrow_start_resample = 0;
            resolution_lim.inrow_last_resample = sMax<idx>(0, in_table->rows() - 2);
            resolution_lim.cnt_resampled_bins = 1;
        } else {
            // print first row, then resample rows 1...n-1 into out_resolution-2 bins, then print last row
            resolution_lim.inrow_pre = 0;
            resolution_lim.inrow_post = in_table->rows() - 1;
            resolution_lim.inrow_start_resample = 1;
            resolution_lim.inrow_last_resample = sMax<idx>(0, in_table->rows() - 2);
            resolution_lim.cnt_resampled_bins = out_resolution - 2;
        }
        out_col2imm.resize(out_cols_dim);
        for(idx out_col = 0; out_col < out_cols_dim; out_col++) {
            if( out_cols.colIsMinmaxMain(out_col) ) {
                minmax_main = out_col2imm[out_col] = minmax_cols.dim();
                minmax_cols.add(1)->out_col = out_col;
            } else if( out_cols.colIsMinmax(out_col) ) {
                out_col2imm[out_col] = minmax_cols.dim();
                minmax_cols.add(1)->out_col = out_col;
            } else {
                out_col2imm[out_col] = -sIdxMax;
            }
        }

        if( out_abscissa_col >= 0 ) {
            _ctx.qlangCtx().setCol(out_abscissa_col);
            _ctx.qlangCtx().setInRow(resolution_lim.inrow_start_resample);
            getOutValue(result, in_table, out_abscissa_col, resolution_lim.inrow_start_resample);
            resolution_lim.start_resample_val = result.asReal();

            _ctx.qlangCtx().setInRow(resolution_lim.inrow_last_resample);
            getOutValue(result, in_table, out_abscissa_col, resolution_lim.inrow_last_resample);
            resolution_lim.last_resample_val = result.asReal();

            resolution_lim.desc = resolution_lim.start_resample_val > resolution_lim.last_resample_val;
        }
#if _DEBUG
        fprintf(stderr, "resolution = %"DEC" (resampling bin cnt = %"DEC")\n", out_resolution, resolution_lim.cnt_resampled_bins);
        fprintf(stderr, "singleton first row = %"DEC"\n", resolution_lim.inrow_pre);
        fprintf(stderr, "resample start row = %"DEC, resolution_lim.inrow_start_resample);
        if( out_abscissa_col >= 0 ) {
            fprintf(stderr, " (abscissa col %"DEC" start value = %g)", out_abscissa_col, resolution_lim.start_resample_val);
        }
        fprintf(stderr, "\nresample last row = %"DEC, resolution_lim.inrow_last_resample);
        if( out_abscissa_col >= 0 ) {
            fprintf(stderr, " (abscissa col %"DEC" last value = %g)", out_abscissa_col, resolution_lim.last_resample_val);
        }
        fprintf(stderr, "\nsingleton last row = %"DEC"\n", resolution_lim.inrow_post);
        if( minmax_cols.dim() ) {
            fprintf(stderr, "minmax cols: ");
            for(idx imm=0; imm<minmax_cols.dim(); imm++) {
                fprintf(stderr, "%s%"DEC"%s", imm ? ", " : "", minmax_cols[imm].out_col, minmax_main == imm ? " (main)" : "");
            }
            fprintf(stderr, "\n");
        }
#endif
    }

    // intended order of operations to emulate
    // 1. filters
    // 2. resolution, minmax
    // 3. start, cnt

    if( out_resolution > 0 ) {
        // resample
        idx out_row = 0;
        idx out_row_ifno_start = 0;

        // print first row
        if( resolution_lim.inrow_pre == 0 ) {
            if( out_row_ifno_start >= out_start && out_row < out_cnt ) {
                _ctx.reportSubProgress(progress_items++, -1, progress_items_max);
                if( unlikely(!basicPrintOutRow(out_csv_table, 0, 0, 0)) ) {
                    return false;
                }
                out_row++;
            }
            out_row_ifno_start++;
        }

        // resample resolution
#ifdef _DEBUG_TBLQRY_RESOLUTION
        real abscissa_cur_bin = resolution_lim.start_resample_val;
#endif
        for(idx ibin = 0, in_row = resolution_lim.inrow_start_resample; out_row < out_cnt && ibin < resolution_lim.cnt_resampled_bins && in_row <= resolution_lim.inrow_last_resample; ibin++) {
            _ctx.reportSubProgress(progress_items++, -1, progress_items_max);

            real abscissa_next_bin = HUGE_VAL;
            idx in_row_bin_start = in_row;
            idx in_row_next_bin_start = resolution_lim.inrow_last_resample + 1;
            // last bin's one-past-end is always resolution_lim.inrow_last_resample + 1; earlier bins' one-past-end needs to be calculated
            if( ibin + 1 < resolution_lim.cnt_resampled_bins ) {
                if( out_abscissa_col >= 0 ) {
                    real range_abscissa = resolution_lim.last_resample_val - resolution_lim.start_resample_val;
                    abscissa_next_bin = resolution_lim.start_resample_val + (ibin + 1) * range_abscissa / resolution_lim.cnt_resampled_bins;
                } else {
                    // cnt = sMax<idx>(1, ((i + 1) * (dim() - 1)) / (res - 1) + _input_start - _groups[i].start);
                    real cnt_resampled_in_rows = resolution_lim.inrow_last_resample - resolution_lim.inrow_start_resample + 1;
                    in_row_next_bin_start = sMax<idx>(in_row_bin_start + 1, (idx)(floor(resolution_lim.inrow_start_resample + (ibin + 1) * (cnt_resampled_in_rows / resolution_lim.cnt_resampled_bins))));
                    if( unlikely(in_row_next_bin_start > resolution_lim.inrow_last_resample) ) {
                        in_row_next_bin_start = resolution_lim.inrow_last_resample + 1;
                    }
                }
            }

            for(idx imm = 0; imm < minmax_cols.dim(); imm++) {
                minmax_cols[imm].min_in_row = minmax_cols[imm].max_in_row = -sIdxMax;
            }

            for(in_row = in_row_bin_start; in_row < in_row_next_bin_start; in_row++) {
                _ctx.qlangCtx().setInRow(in_row);
                // if binning by abscissa and this is not the last bin, check if curent row's abscissa value satisfies < abscissa_next_bin criterion
                if( ibin + 1 < resolution_lim.cnt_resampled_bins && out_abscissa_col >= 0 ) {
                    _ctx.qlangCtx().setCol(out_abscissa_col);
                    getOutValue(result, in_table, out_abscissa_col, in_row);
                    real abscissa_val = result.asReal();
                    if( (resolution_lim.desc && abscissa_val <= abscissa_next_bin) || (!resolution_lim.desc && abscissa_val >= abscissa_next_bin) ) {
                        in_row_next_bin_start = in_row;
                        break;
                    }
                }
                for(idx imm = 0; imm < minmax_cols.dim(); imm++) {
                    MinMaxCol & mm = minmax_cols[imm];
                    _ctx.qlangCtx().setCol(mm.out_col);
                    getOutValue(result, in_table, mm.out_col, in_row);
                    if( mm.min_in_row < 0 || result < mm.min_val ) {
                        mm.min_in_row = in_row;
                        mm.min_val = result;
                    }
                    if( mm.max_in_row < 0 || result > mm.max_val ) {
                        mm.max_in_row = in_row;
                        mm.max_val = result;
                    }
                }
            }

#ifdef _DEBUG_TBLQRY_RESOLUTION
            fprintf(stderr, "processed resampling bin %"DEC": input rows %"DEC"...%"DEC, ibin, in_row_bin_start, in_row_next_bin_start - 1);
            if( out_abscissa_col >= 0 ) {
                fprintf(stderr, ", expect abscissa column %"DEC" value range %g %s x %s %g\n", out_abscissa_col, abscissa_cur_bin, resolution_lim.desc ? ">=" : "<=", resolution_lim.desc ? ">" : "<", abscissa_next_bin);
            } else {
                fprintf(stderr, "\n");
            }
            for(idx imm = 0; imm < minmax_cols.dim(); imm++) {
                MinMaxCol & mm = minmax_cols[imm];
                fprintf(stderr, "col %"DEC" min = %s (inrow %"DEC"), max = %s (inrow %"DEC")\n", mm.out_col, mm.min_val.asString(), mm.min_in_row, mm.max_val.asString(), mm.max_in_row);
            }
#endif

            // fill table
            if( minmax_cols.dim() && in_row_next_bin_start > in_row_bin_start + 1 ) {
                idx in_row1 = in_row_bin_start + (in_row_next_bin_start - in_row_bin_start) / 3;
                idx in_row2 = in_row_bin_start + 2 * (in_row_next_bin_start - in_row_bin_start) / 3;
                bool min_is_first = true;
                if( minmax_main >= 0 && minmax_cols[minmax_main].min_in_row != minmax_cols[minmax_main].max_in_row ) {
                    in_row1 = sMin<idx>(minmax_cols[minmax_main].min_in_row, minmax_cols[minmax_main].max_in_row);
                    in_row2 = sMax<idx>(minmax_cols[minmax_main].min_in_row, minmax_cols[minmax_main].max_in_row);
                    min_is_first = minmax_cols[minmax_main].min_in_row <= minmax_cols[minmax_main].max_in_row;
                }

                if( out_row_ifno_start >= out_start && out_row < out_cnt ) {
                    if( unlikely(!minmaxPrintOutRow(out_csv_table, in_row1, out_row_ifno_start, out_row_ifno_start, min_is_first)) ) {
                        return false;
                    }
                    out_row++;
                }
                out_row_ifno_start++;

                if( out_row_ifno_start >= out_start && out_row < out_cnt ) {
                    if( unlikely(!minmaxPrintOutRow(out_csv_table, in_row2, out_row_ifno_start, out_row_ifno_start, !min_is_first)) ) {
                        return false;
                    }
                    out_row++;
                }
                out_row_ifno_start++;

            } else {
                if( in_row_bin_start < in_row_next_bin_start && out_row_ifno_start >= out_start && out_row < out_cnt ) {
                    in_row = in_row_bin_start + (in_row_next_bin_start - in_row_bin_start) / 2;
                    if( unlikely(!basicPrintOutRow(out_csv_table, in_row, out_row_ifno_start, out_row_ifno_start)) ) {
                        return false;
                    }
                    out_row++;
                }
                out_row_ifno_start++;
            }

            in_row = in_row_next_bin_start;
#ifdef _DEBUG_TBLQRY_RESOLUTION
            abscissa_cur_bin = abscissa_next_bin;
#endif
        }

        // print final row
        if( resolution_lim.inrow_post > 0 ) {
            if( out_row_ifno_start >= out_start && out_row < out_cnt ) {
                _ctx.reportSubProgress(progress_items++, -1, progress_items_max);
                if( unlikely(!basicPrintOutRow(out_csv_table, resolution_lim.inrow_post, out_row_ifno_start, out_row_ifno_start)) ) {
                    return false;
                }
                out_row++;
            }
            out_row_ifno_start++;
        }
    } else {
        for(idx in_row = 0, out_row = 0, out_row_ifno_start = 0, out_row_ifno_filter = 0; in_row < in_table->rows() && out_row < out_cnt; in_row++) {
            _ctx.reportSubProgress(progress_items++, -1, progress_items_max);
            out_buf.reset();
            _ctx.qlangCtx().setInRow(in_row);

            bool pass_filters = true;
            bool incr_out_row_ifno_filter = true;
            for(idx ifilt = 0; ifilt < rowfilters.dim() && pass_filters; ifilt++) {
                switch(rowfilters[ifilt].eval(in_row, out_row_ifno_filter)) {
                    case OutputFilter::eResult_print:
                        incr_out_row_ifno_filter = true;
                        pass_filters = true;
                        break;
                    case OutputFilter::eResult_hide:
                        incr_out_row_ifno_filter = false;
                        pass_filters = false;
                        break;
                    case OutputFilter::eResult_hide_and_increment:
                        incr_out_row_ifno_filter = true;
                        pass_filters = false;
                        break;
                    case OutputFilter::eResult_error:
                        return false;
                }
            }

            if( incr_out_row_ifno_filter ) {
                out_row_ifno_filter++;
            }

            if( !pass_filters )
                continue;

            if( out_row_ifno_start < out_start ) {
                out_row_ifno_start++;
                continue;
            }

            if( unlikely(!basicPrintOutRow(out_csv_table, in_row, out_row_ifno_filter, out_row_ifno_start)) ) {
                return false;
            }
            out_row_ifno_start++;
            out_row++;
        }
    }

    for(idx out_col = 0; out_col < out_cols.dim(); out_col++) {
        sVariant::eType itype = out_cols.getType(out_col);
        if( itype != sVariant::value_NULL ) {
            out_csv_table->setColtype(out_col, itype);
        }
    }

    if( colfilters.dim() ) {
        sVec<sReorderedTabular::TypedSource> col_map;
        for(idx icol = 0; icol < out_cols.dim(); icol++) {
            bool printable = true;
            for(idx ifilt = 0; ifilt < colfilters.dim(); ifilt++) {
                if( colfilters[ifilt].getResults()[icol] != OutputFilter::eResult_print ) {
                    printable = false;
                    break;
                }
            }
            if( printable ) {
                sReorderedTabular::TypedSource * s = col_map.add(1);
                s->src = icol;
                s->type = out_cols.getType(icol);
            }
        }
        if( col_map.dim() < out_cols.dim() ) {
            sReorderedTabular * colfiltered_table = new sReorderedTabular(out_csv_table, true);
            colfiltered_table->borrowColsMap(col_map);
            _out_table = colfiltered_table;
        }
    }

    return true;
}

bool ReinterpretStage::compute(sTabular * in_table)
{
    _out_table = in_table;
    if( !in_table ) {
        // In theory, null input table might not be an error - e.g. if reinterpret was requested
        // by a command that has an input table only optionally. However, the situation is unusual.
        _ctx.logInfo("Missing input table for reinterpret operation");
        return true;
    }

    for(idx i = 0; i < _out_table->cols(); i++) {
        if( unreinterpret ) {
            _out_table->reinterpretCol(i, sTabular::eNone);
        } else {
            idx coltype = _out_table->coltype(i);
            if( coltype == sVariant::value_REAL ) {
                _out_table->reinterpretCol(i, sTabular::eUnique, sTabular::fMissingAsNaN);
            } else if( coltype == sVariant::value_STRING ) {
                idx j;
                for(j = 0; j < binary_cols.dim(); j++)
                    if( binary_cols[j] == i )
                        break;

                if( j < binary_cols.dim() ) {
                    _out_table->reinterpretCol(i, sTabular::eBool, sTabular::fMissingAsNaN);
                } else {
                    _out_table->reinterpretCol(i, sTabular::eUnique, sTabular::fMissingAsNaN);
                }
            }
        }
    }
    return true;
}

bool ExecContext::processCommands()
{
#ifdef _DEBUG
    for(idx ii=0; ii<_in_tables.dim(); ii++) {
        printDebugTable(-1, ii, getLoadedTable(ii), 0);
    }
    fprintf(stderr, "Will run %"DEC" commands\n", _commands.dim());
#endif

    idx start_clean = 0;
    sTabular * cur_table = getLoadedTable(0);

    for(_cur_icmd = 0; _cur_icmd < _commands.dim(); _cur_icmd++) {
        logDebug("Running %s command (#%"DEC" of %"DEC")", _commands[_cur_icmd]->getName(), _cur_icmd, _commands.dim());
        if( _commands[_cur_icmd]->hasProgress() ) {
            startSubProgress();
        }
        _qlang_ctx.setTable(cur_table);
        if( !_commands[_cur_icmd]->compute(cur_table) ) {
            return false;
        }
        if( !_commands[_cur_icmd]->wrapsInTable() ) {
            cleanupCommandOutputs(start_clean, _cur_icmd - start_clean);
            start_clean = _cur_icmd;
        }
#ifdef _DEBUG
        const sTabular * prev_table = cur_table;
#endif
        if( _commands[_cur_icmd]->computesOutTable() ) {
            cur_table = _commands[_cur_icmd]->getOutTable();
        }
        if( _commands[_cur_icmd]->hasProgress() ) {
            endSubProgress();
        }
#ifdef _DEBUG
        printDebugTable(_cur_icmd, -1, cur_table, prev_table);
#endif
    }
    _out_table = cur_table;
    return true;
}

void printTableMetadata(sStr & out, const sTabular * tbl, OutputCategories * rowcats, OutputCategories * colcats, const char * indent)
{
    sStr subindent("%s    ", indent);
    sStr subsubindent("%s    ", subindent.ptr());

    out.printf("{\r\n%s\"columns\": {\r\n%s\"total\": %"DEC", \"header\": %"DEC", \"data\": %"DEC",\r\n%s\"types\": [", subindent.ptr(), subsubindent.ptr(), tbl->cols() + tbl->dimLeftHeader(), tbl->dimLeftHeader(), tbl->cols(), subsubindent.ptr());
    for(idx i = -tbl->dimLeftHeader(); i < tbl->cols(); i++) {
        if( i > -tbl->dimLeftHeader() ) {
            out.printf(", ");
        }
        out.printf("\"%s\"", sVariant::getTypeName(tbl->coltype(i)));
    }
    out.printf("]");

    if( colcats && colcats->dim() ) {
        out.printf(",\r\n%s", subsubindent.ptr());
        colcats->print(out, subsubindent.ptr());
    }

    out.printf("\r\n%s},\r\n", subindent.ptr());
    out.printf("%s\"rows\": {\r\n%s\"total\": %"DEC", \"header\": %"DEC", \"data\": %"DEC"", subindent.ptr(), subsubindent.ptr(), tbl->rows() + tbl->dimTopHeader(), tbl->dimTopHeader(), tbl->rows());
    if( rowcats && rowcats->dim() ) {
        out.printf(",\r\n%s", subsubindent.ptr());
        rowcats->print(out, subsubindent.ptr());
    }
    out.printf("\r\n%s}\r\n%s}", subindent.ptr(), indent);
}

bool ExecContext::saveResult()
{
    sStr outpath_buf;

    {
        const char * outpath = _proc.reqAddFile(outpath_buf, "_.csv");
        sFil outfil(outpath);
        if( !outpath || !outfil.ok() ) {
            logError("Failed to create _.csv req file for request %"DEC, _out_req);
            return false;
        }

        logDebug("Writing %s", outpath);
        _out_table->printCSV(outfil);
        outfil.destroy();
    }

    if( _proc.pForm->boolvalue("arch") ) {
        const char * dstName = _proc.pForm->value("arch_dstname");
        sStr datasource("file://%"DEC"-%s", _out_req, dstName ? dstName : "output.csv");
        sStr arch_src;
        _proc.reqDataPath(_out_req, "_.csv", &arch_src);
        dmArchiver arch(_proc, arch_src, datasource, "csv", dstName);
        arch.addObjProperty("source", "%s", datasource.ptr());
        idx arch_reqId = arch.launch(*_proc.user);
        if( !arch_reqId )
            return false;
        if( !_proc.reqSetData(_out_req, "arch-req.txt", "%"DEC, arch_reqId) )
            return false;
    } else {
        // repack _.csv only if not archiving to obj - otherwise, archiver won't pick up the file
        _proc.reqRepackData(_out_req, "_.csv");
    }

    {
        const char * metadata_outpath = _proc.reqAddFile(outpath_buf, "metadata.json");
        sFil metadata(metadata_outpath);
        if( !metadata_outpath || !metadata.ok() ) {
            logError("Failed to create metadata.json req file for request %"DEC, _out_req);
            return false;
        }

        metadata.printf("{\r\n    \"input\": ");
        printTableMetadata(metadata, getLoadedTable(0), 0, 0, "    ");

        // use output categories from last print stage
        OutputCategories * rowcats = 0, *colcats = 0;
        for(idx icmd = _commands.dim() - 1; icmd >= 0; icmd--) {
            if( PrintStage * print_stage = dynamic_cast<PrintStage *>(_commands[icmd]) ) {
                rowcats = &print_stage->getRowCats();
                colcats = &print_stage->getColCats();
                break;
            }
        }
        metadata.printf(",\r\n    \"output\": ");
        printTableMetadata(metadata, _out_table, rowcats, colcats, "    ");
        metadata.printf("\r\n}\r\n");

#ifdef _DEBUG
        int metadata_length = metadata.length();
        fprintf(stderr, "metadata.json:\n%*s", metadata_length, metadata.ptr());
#endif
        metadata.destroy();
    }
    _proc.reqRepackData(_out_req, "metadata.json");
    return true;
}

idx TblQryX4::OnExecute(idx req)
{
#ifdef _DEBUG
    fprintf(stderr, "qpride form for req %"DEC":\n", req);
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
#endif

    // FIXME : for debugging in release mode
    // setupLog(true, sQPrideBase::eQPLogType_Trace);

    ExecContext exec_ctx(this);

    if( !exec_ctx.parseForm() ) {
        if( exec_ctx.waitingForReq() > 0 && exec_ctx.waitingForReq() != req ) {
            // we need a subrequest's output to continue, so wait a bit
            reqReSubmit(req, 5);
            return 0;
        }
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    if( !exec_ctx.prepareCommands() ) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    if( !exec_ctx.loadInput() ) {
        if( exec_ctx.waitingForReq() > 0 && exec_ctx.waitingForReq() != req ) {
            // another request is already parsing this table, so wait a bit
            reqReSubmit(req, 30);
            return 0;
        }
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    if( !exec_ctx.processCommands() ) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    if( !(exec_ctx.saveResult()) ) {
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future
    TblQryX4 backend("config=qapp.cfg"__, sQPrideProc::QPrideSrvName(&tmp, "tblqryx4", argv[0]));
    return (int) backend.run(argc, argv);
}
