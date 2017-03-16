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
#ifndef sLib_tblqryx4_rowscols_hpp
#define sLib_tblqryx4_rowscols_hpp

#include <slib/core/vec.hpp>
#include <slib/utils/tbl.hpp>
#include <slib/utils/tbltbl.hpp>
#include <ulib/uquery.hpp>

#include <regex.h>

namespace slib {
    namespace tblqryx4 {
        class ExecContext;

        // TODO : replace with shared_ptr<ast::Node> once we switch to C++11
        struct SharedFmla
        {
            public:
                const qlang::ast::Node * fmla;
            private:
                idx ref_cnt;
            public:
                SharedFmla(const qlang::ast::Node * fmla_, idx ref_cnt_)
                {
                    fmla = fmla_;
                    ref_cnt = ref_cnt_;
                }
                ~SharedFmla() { delete fmla; }
                static SharedFmla * incref(SharedFmla * f)
                {
                    if( f ) {
                        f->ref_cnt++;
                        fprintf(stderr, "incref %p to %"DEC"\n", f, f->ref_cnt);
                    }
                    return f;
                }
                static SharedFmla * decref(SharedFmla * f)
                {
                    if( f ) {
                        f->ref_cnt--;
                        fprintf(stderr, "decref %p to %"DEC"\n", f, f->ref_cnt);
                        if( f->ref_cnt <= 0 ) {
                            delete f;
                            return 0;
                        }
                    }
                    return f;
                }
        };
        /* Possibilities: range of rows (start to end), possibly split into groups;
         sparse list of rows, possibly split into groups by resolution or collapse */
        struct OutputColSource
        {
                idx input_col;
                SharedFmla * shared_fmla;

                OutputColSource()
                {
                    sSet(this, 0); // zero any padding bytes, allows using OutputColSource as sDic key
                    input_col = -sIdxMax;
                }
                OutputColSource & operator=(const OutputColSource & rhs)
                {
                    input_col = rhs.input_col;
                    shared_fmla = SharedFmla::incref(rhs.shared_fmla);
                    return *this;
                }
                ~OutputColSource()
                {
                    SharedFmla::decref(shared_fmla);
                }
                void reset()
                {
                    SharedFmla::decref(shared_fmla);
                    shared_fmla = 0;
                    input_col = -sIdxMax;
                }
                OutputColSource& swap(OutputColSource & rhs)
                {
                    sSwapI(input_col, rhs.input_col);
                    sSwap<SharedFmla*>(shared_fmla, rhs.shared_fmla);
                    return *this;
                }
                void setFormula(const sTabular * in_table, const qlang::ast::Node * fmla_);
                const qlang::ast::Node * getFormula() const { return shared_fmla ? shared_fmla->fmla : 0; }
        };

        class OutputColumns
        {
            public:
                typedef idx saveHandle;

            private:
                struct OutputCol
                {
                        idx iname;
                        OutputColSource src;
                        idx prev_homonym;
                        idx next_homonym;
                        mutable idx next_synonym; // only updated by getOutputCols
                        sVariant::eType itype;
                        sVariant::eType ifmt_type;
                        idx fmt_offset;
                        enum EMinmaxStatus
                        {
                            minmax_NONE = 0,
                            minmax_BASIC,
                            minmax_MAIN
                        } minmax_status;

                        OutputCol()
                        {
                            sSet(this, 0); // zero any padding bytes, allows using OutputCol as sDic key
                            reset();
                        }
                        OutputCol & swap(OutputCol & rhs)
                        {
                            sSwapI(iname, rhs.iname);
                            src.swap(rhs.src);
                            sSwapI(prev_homonym, rhs.prev_homonym);
                            sSwapI(next_homonym, rhs.next_homonym);
                            sSwapI(next_synonym, rhs.next_synonym);
                            sSwap<sVariant::eType>(itype, rhs.itype);
                            sSwap<sVariant::eType>(ifmt_type, rhs.ifmt_type);
                            sSwapI(fmt_offset, rhs.fmt_offset);
                            sSwap<EMinmaxStatus>(minmax_status, rhs.minmax_status);
                            return *this;
                        }

                        void reset()
                        {
                            iname = -1;
                            src.reset();
                            prev_homonym = next_homonym = next_synonym = -1;
                            minmax_status = minmax_NONE;
                            itype = ifmt_type = sVariant::value_NULL;
                            fmt_offset = -1;
                        }
                };

                sVec<OutputCol> _cols;
                mutable sVec<idx> _in2out; // maps input column to first output column
                sDic<idx> _savedCols; // preserve state of a column's source at a particular point in time
                sDic<idx> _colNames; // maps name to first _cols index with such name
                sStr _formats;
                mutable bool _need_update_in2out;
                idx _src_start;

                regex_t _format_checker;

                void updateIn2Out() const;

            public:
                OutputColumns();
                ~OutputColumns();

                void init(const sTabular * in_table, const char * rangeSet = 0);
                void setMinMax(const char * minmaxSet = 0, const char * minmaxMain = 0);

                void deleteAll()
                {
                    _cols.cut(0);
                    for(idx i = 0; i < _in2out.dim(); i++) {
                        _in2out[i] = -sIdxMax;
                    }
                    _formats.cut(0);
                    _colNames.init();
                    _need_update_in2out = false;
                }

                bool validCol(idx icol) const
                {
                    return icol >= 0 && icol < _cols.dim();
                }
                const OutputCol * getCol(idx icol, saveHandle keep) const
                {
                    if( keep ) {
                        return static_cast<const OutputCol *>(_savedCols.id(keep - 1));
                    } else if( validCol(icol) ) {
                        return &_cols[icol];
                    } else {
                        return 0;
                    }
                }
                bool validInsertPosition(idx icol) const
                {
                    return icol >= 0 && icol <= _cols.dim();
                }

                bool renameCol(idx icol, const char * name);
                bool deleteCol(idx icol);
                bool insertCol(idx icol, const sTabular * in_table, const char * name, const qlang::ast::Node * fmla, sVariant::eType itype = sVariant::value_NULL);
                bool moveCol(idx icol_from, idx icol_to);

                void appendCol(const char * name, const sTabular * in_table, const qlang::ast::Node * fmla, sVariant::eType itype = sVariant::value_NULL)
                {
                    idx icol = _cols.dim();
                    _cols.add(1);
                    renameCol(icol, name);
                    _cols[icol].src.setFormula(in_table, fmla);
                    _cols[icol].itype = itype;
                    _need_update_in2out = true;
                }

                bool setFormula(idx icol, const sTabular * in_table, const qlang::ast::Node * fmla)
                {
                    if( !validCol(icol) )
                        return false;

                    _cols[icol].src.setFormula(in_table, fmla);
                    _need_update_in2out = true;
                    return true;
                }
                bool setType(idx icol, sVariant::eType itype)
                {
                    if( !validCol(icol) )
                        return false;
                    _cols[icol].itype = itype;
                    return true;
                }
                bool setFormat(idx icol, const char * fmt);

                idx dim() const
                {
                    return _cols.dim();
                }
                saveHandle minSavedCol() const
                {
                    return 1;
                }
                saveHandle maxSavedCol() const
                {
                    return _savedCols.dim();
                }
                const char * getName(idx icol) const
                {
                    if( !validCol(icol) )
                        return 0;

                    idx iname = _cols[icol].iname;
                    return iname >= 0 ? static_cast<const char *>(_colNames.id(iname)) : 0;
                }
                const qlang::ast::Node * getFormula(idx icol, saveHandle keep=0) const
                {
                    const OutputCol * col = getCol(icol, keep);
                    if( !col )
                        return 0;

                    return col->src.getFormula();
                }
                idx getInputCol(idx icol, saveHandle keep=0) const
                {
                    const OutputCol * col = getCol(icol, keep);
                    if( !col )
                        return -sIdxMax;

                    return col->src.input_col;
                }
                bool getOutputCols(sVec<idx> & icols, idx input_col) const;
                sVariant::eType getType(idx icol, saveHandle keep=0) const
                {
                    const OutputCol * col = getCol(icol, keep);
                    if( !col )
                        return sVariant::value_NULL;

                    return col->itype;
                }
                sVariant::eType getFormatType(idx icol, saveHandle keep=0) const
                {
                    const OutputCol * col = getCol(icol, keep);
                    if( !col )
                        return sVariant::value_NULL;

                    return col->ifmt_type;
                }
                const char * getFormat(idx icol, saveHandle keep=0) const
                {
                    const OutputCol * col = getCol(icol, keep);
                    if( !col || col->fmt_offset < 0)
                        return 0;

                    return _formats.ptr(col->fmt_offset);
                }

                idx colIndex(const char * name, idx iname = 0) const;
                idx colIndices(sVec<idx> & out, const char * name) const;

                bool colIsMinmax(idx icol) const
                {
                    return _cols[icol].minmax_status == OutputCol::minmax_BASIC;
                }
                bool colIsMinmaxMain(idx icol) const
                {
                    return _cols[icol].minmax_status == OutputCol::minmax_MAIN;
                }

                saveHandle saveColumn(idx icol);

                const char * printDump(sStr & out, bool withType = true, const char * indent=0);
        };

        class OutputBuffer
        {
            private:
                struct Cell {
                    idx pos;
                    idx len;
                    idx in_row;
                    idx in_col;
                    sVariant val;
                    bool lazy_val_from_in_table;
                };
                const OutputColumns * _out_cols;
                const sTabular * _in_table;
                ExecContext * _ctx;

                sStr _buf;
                sVec<Cell> _cells;
                //sVec<sMex::Pos> _offsets;
                //sVec<sVariant> _values;
                //sVec<idx> _in_rows;
                sVec<OutputColumns::saveHandle> _icol2saved;
                idx _cols;
                idx _saved;

                bool setCell(idx icol, OutputColumns::saveHandle keep, idx in_row);

            public:
                OutputBuffer()
                {
                    _out_cols = 0;
                    _in_table = 0;
                    _ctx = 0;
                    _cols = _saved = 0;
                    _buf.cut0cut();
                }
                void init(OutputColumns * out_cols, const sTabular * in_table, tblqryx4::ExecContext * ctx);
                void reset()
                {
                    _buf.cut0cut();
                    _cells.resize(_saved);
                    for(idx i = 0; i < _saved; i++) {
                        _cells[i].pos = -sIdxMax;
                        _cells[i].len = 0;
                        _cells[i].in_row = -sIdxMax;
                        _cells[i].in_col = -sIdxMax;
                        _cells[i].lazy_val_from_in_table = false;
                        _cells[i].val.setNull();
                    }
                }
                inline idx cols() const
                {
                    return _cols;
                }
                inline sVariant & val(idx icol)
                {
                    Cell & c = _cells[_icol2saved[icol]];
                    if( c.lazy_val_from_in_table ) {
                        _in_table->val(c.val, c.in_row, c.in_col);
                        c.lazy_val_from_in_table = false;
                    }
                    return c.val;
                }
                inline sVariant & valForSaved(OutputColumns::saveHandle keep)
                {
                    Cell & c = _cells[keep];
                    if( c.lazy_val_from_in_table ) {
                        _in_table->val(c.val, c.in_row, c.in_col);
                        c.lazy_val_from_in_table = false;
                    }
                    return c.val;
                }
                const bool initialized(idx icol)
                {
                    return _cells[_icol2saved[icol]].pos >= 0;
                }
                const bool initializedForSaved(OutputColumns::saveHandle keep)
                {
                    return _cells[keep].pos >= 0;
                }
                const char * ptr(idx icol) const
                {
                    idx pos = _cells[_icol2saved[icol]].pos;
                    return pos >= 0 ? _buf.ptr(pos) : sStr::zero;
                }
                const char * ptrForSaved(OutputColumns::saveHandle keep) const
                {
                    idx pos = _cells[keep].pos;
                    return pos >= 0 ? _buf.ptr(pos) : sStr::zero;
                }
                idx length(idx icol) const
                {
                    return _cells[_icol2saved[icol]].len;
                }
                idx lengthForSaved(OutputColumns::saveHandle keep) const
                {
                    return _cells[keep].len;
                }
                bool setCell(idx icol, sVariant * in_val)
                {
                    return setCellForSaved(_icol2saved[icol], in_val);
                }
                bool setCellForSaved(OutputColumns::saveHandle keep, sVariant * in_val)
                {
                    Cell & c = _cells[keep];
                    c.pos = _buf.length();
                    c.lazy_val_from_in_table = false;
                    if( in_val ) {
                        in_val->print(_buf, sVariant::eCSV);
                        c.val = *in_val;
                    } else {
                        c.val.setNull();
                    }
                    c.len = _buf.length() - c.pos;
                    _buf.add0();
                    return true;
                }
                bool setCell(idx icol, idx in_row)
                {
                    return setCell(icol, _icol2saved[icol], in_row);
                }
                bool setCellForSaved(OutputColumns::saveHandle keep, idx in_row)
                {
                    return setCell(-sIdxMax, keep, in_row);
                }
                void printTblRow(sTxtTbl & out_tbl)
                {
                    for(idx icol = 0; icol < _cols; icol++) {
                        out_tbl.addCell(ptr(icol), length(icol));
                    }
                    out_tbl.addEndRow();
                }
        };

        class OutputFilter
        {
            public:
                enum EMethod
                {
                    eMethod_invalid = -1,
                    eMethod_list,
                    eMethod_inlist,
                    eMethod_equals,
                    eMethod_regex,
                    eMethod_substring,
                    eMethod_formula,
                    eMethod_range
                };

                enum EResult
                {
                    eResult_error = -1,
                    eResult_hide = 0,
                    eResult_hide_and_increment,
                    eResult_print
                };

            protected:
                const sTabular * _in_table;
                OutputColumns * _out_cols;
                OutputBuffer * _out_buf;
                ExecContext * _ctx;
                EMethod _method;

                struct ValueSpec
                {
                        enum EValueSpecType
                        {
                            eSpec_val,
                            eSpec_regex,
                            eSpec_range_inclusive,
                            eSpec_range_exclusive,
                            eSpec_ge,
                            eSpec_gt,
                            eSpec_le,
                            eSpec_lt
                        } type;
                        regex_t re;
                        const qlang::ast::Node * fmla;
                        sVariant val, val2;

                        ValueSpec()
                        {
                            type = eSpec_val;
                            fmla = 0;
                        }
                        ~ValueSpec()
                        {
                            delete fmla;
                            if( type == eSpec_regex )
                                regfree(&re);
                        }
                };
                sVec<ValueSpec> _specs;
                sVec<idx> _rows;
                idx _icur_row;
                sVariant _tmp_val, _tmp_val1, _tmp_val2;
                const char * _opname;
                static const char * _default_opnames[];

                bool _case_sensitive;
                bool _negate;
                bool _use_in_table;
                bool _all_rows;
                bool _conjunction;

                OutputFilter()
                {
                    _case_sensitive = false;
                    _negate = false;
                    _use_in_table = false;
                    _all_rows = false;
                    _conjunction = false;
                    _method = eMethod_invalid;

                    _in_table = 0;
                    _out_cols = 0;
                    _out_buf = 0;
                    _ctx = 0;
                    _opname = 0;
                    _icur_row = 0;
                }

                bool initValues(sVariant * values);
                EResult evalDataValue(sVariant * pdataval, bool allowFormula);
        };

        class OutputRowFilter: public OutputFilter
        {
            private:
                static const char * _default_opname;

                sVec<OutputColumns::saveHandle> _colHandles;
                sVec<idx> _cols;

            public:
                OutputRowFilter()
                {
                    _opname = _default_opname;
                }

                bool init(sVariant * arg, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx, const char * opname = 0);
                bool initSearch(const char * srch, bool srchRegExp, const char * srchRangeSet, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx);
                EResult eval(idx in_row, idx out_row);
        };

        class OutputColFilter: public OutputFilter
        {
            private:
                static const char * _default_opname;

                sVec<EResult> _results;
                bool _evaled;
                bool _all_rows;

            public:
                OutputColFilter()
                {
                    _evaled = false;
                    _all_rows = false;
                    _opname = _default_opname;
                }

                bool init(sVariant * arg, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx, const char * opname = 0);
                bool eval(idx in_row, idx out_row);
                const sVec<EResult> & getResults() const
                {
                    return _results;
                }
        };

        class OutputCategories
        {
            protected:
                virtual bool hasCat(const char * name) const = 0;
                virtual bool initNewCat(const char * name, sVariant * arg, sVariant * tqs_op, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx) = 0;
                virtual const char * getTypeName() const = 0;
                virtual const char * getOpName() const = 0;
                virtual const char * getCatName(idx icat) const = 0;
                virtual const sVec<idx> & getCatValues(idx icat) = 0;
                virtual const sVariant & getCatTqsOp(idx icat) const = 0;
                virtual void prePrint()
                {
                }

            public:
                virtual ~OutputCategories()
                {
                }
                bool add(sVariant * tqs_op, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx);
                const char * print(sStr & out, const char * indent);
                virtual idx dim() const = 0;
        };

        class OutputRowCategories: public OutputCategories
        {
            private:
                struct Cat
                {
                        OutputRowFilter categorizer;
                        sVariant tqs_op;
                        sVec<idx> values;
                };
                sDic<Cat> _cats;

                bool hasCat(const char * name) const
                {
                    return _cats.get(name);
                }
                bool initNewCat(const char * name, sVariant * arg, sVariant * tqs_op, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx);
                const char * getTypeName() const;
                const char * getOpName() const;
                const char * getCatName(idx icat) const
                {
                    return static_cast<const char *>(_cats.id(icat));
                }
                const sVec<idx> & getCatValues(idx icat)
                {
                    return _cats.ptr(icat)->values;
                }
                const sVariant & getCatTqsOp(idx icat) const
                {
                    return _cats.ptr(icat)->tqs_op;
                }
            public:
                bool categorize(idx in_row, idx out_row);
                idx dim() const
                {
                    return _cats.dim();
                }
        };

        class OutputColCategories: public OutputCategories
        {
            private:
                struct Cat
                {
                        OutputColFilter categorizer;
                        sVariant tqs_op;
                        sVec<idx> values;
                        bool values_prepared;

                        Cat()
                        {
                            values_prepared = false;
                        }
                };
                sDic<Cat> _cats;

                bool hasCat(const char * name) const
                {
                    return _cats.get(name);
                }
                bool initNewCat(const char * name, sVariant * arg, sVariant * tqs_op, const sTabular * in_table, OutputColumns * out_cols, OutputBuffer * out_buf, ExecContext * ctx);
                const char * getTypeName() const;
                const char * getOpName() const;
                const char * getCatName(idx icat) const
                {
                    return static_cast<const char *>(_cats.id(icat));
                }
                const sVec<idx> & getCatValues(idx icat);
                const sVariant & getCatTqsOp(idx icat) const
                {
                    return _cats.ptr(icat)->tqs_op;
                }
            public:
                bool categorize(idx in_row, idx out_row);
                idx dim() const
                {
                    return _cats.dim();
                }
        };
    };
};

#endif
