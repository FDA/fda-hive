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
#ifndef sLib_tblqryx4_glue_hpp
#define sLib_tblqryx4_glue_hpp

#include <slib/utils/tbl.hpp>

namespace slib {
    namespace tblqryx4 {
        class sGluedTabular : public sTabular {
            private:
                struct SubTable {
                    sTabular * src;
                    bool owned;
                    idx col_map_offset;
                    sVec<idx> row_map; // map from output absrow to src row
                    sVec<idx> col_map; // map from output abscol - col_map_offset to src col

                    SubTable() : row_map(sMex::fExactSize), col_map(sMex::fExactSize)
                    {
                        src = 0;
                        owned = false;
                        col_map_offset = 0;
                    }

                    ~SubTable()
                    {
                        if( owned ) {
                            delete src;
                        }
                    }

                    void init(sTabular * src_, bool owned_, sVec<idx> & borrow_row_map, sVec<idx> & borrow_col_map)
                    {
                        if( owned ) {
                            delete src;
                        }
                        src = src_;
                        owned = owned_;
                        row_map.mex()->destroy();
                        row_map.mex()->borrow(borrow_row_map.mex());
                        col_map.mex()->destroy();
                        col_map.mex()->borrow(borrow_col_map.mex());
                    }
                    void setColMapOffset(idx offset)
                    {
                        col_map_offset = offset;
                    }

                    idx srcRow(idx out_absrow) const
                    {
                        return likely(out_absrow >= 0 && out_absrow < row_map.dim()) ? row_map[out_absrow] : -sIdxMax;
                    }
                    idx srcCol(idx out_abscol) const
                    {
                        idx abscol = out_abscol - col_map_offset;
                        return likely(abscol >= 0 && abscol < col_map.dim()) ? col_map[abscol] : -sIdxMax;
                    }
                    sVariant::eType srcColtype(idx out_abscol) const
                    {
                        return likely(src) ? src->coltype(srcCol(out_abscol)) : sVariant::value_NULL;
                    }
                    bool setSrcColtype(idx out_abscol, sVariant::eType t)
                    {
                        return likely(src) ? src->setColtype(srcCol(out_abscol), t) : false;
                    }
                    const char * printSrcCell(sStr & out, idx out_absrow, idx out_abscol, idx maxLen, const char * defValue, idx flags, bool isOutMissing) const
                    {
                        if( likely(src) ) {
                            const char * ret = src->printCell(out, srcRow(out_absrow), srcCol(out_abscol), maxLen, defValue, flags);
                            if( !ret && !isOutMissing ) {
                                ret = out.add0cut();
                            }
                            return ret;
                        } else {
                            return 0;
                        }
                    }
                    const char * srcCell(idx out_absrow, idx out_abscol, idx * cellLen, bool isOutMissing) const
                    {
                        if( likely(src) ) {
                            const char * ret = src->cell(srcRow(out_absrow), srcCol(out_abscol), cellLen);
                            if( !ret && !isOutMissing ) {
                                ret = sStr::zero;
                                if( cellLen ) {
                                    *cellLen = 0;
                                }
                            }
                            return ret;
                        } else {
                            if( cellLen ) {
                                *cellLen = 0;
                            }
                            return 0;
                        }
                    }
                    ECellEncoding srcCellEncoding(idx out_absrow, idx out_abscol) const
                    {
                        if( likely(src) ) {
                            return src->cellEncoding(srcRow(out_absrow), srcCol(out_abscol));
                        } else {
                            return sTabular::eEncodingUnspecified;
                        }
                    }
                    bool srcMissing(idx out_absrow, idx out_abscol) const
                    {
                        return likely(src) ? src->missing(srcRow(out_absrow), srcCol(out_abscol)) : false;
                    }
                    bool srcVal(sVariant & out, idx out_absrow, idx out_abscol, bool noReinterpret, bool blankIsNull, bool isOutMissing) const
                    {
                        if( likely(src) ) {
                            bool ret = src->val(out, srcRow(out_absrow), srcCol(out_abscol), noReinterpret, blankIsNull);
                            if( !ret && !isOutMissing ) {
                                ret = true;
                                if( blankIsNull ) {
                                    out.setNull();
                                } else {
                                    out.parseScalarType(sStr::zero, likely(out_absrow >= 0) ? srcColtype(out_abscol) : sVariant::value_STRING);
                                }
                            }
                            return ret;
                        } else {
                            out.setNull();
                            return false;
                        }
                    }
                    const char * getSrcTableMetadata(const char * name, idx out_absrow, idx out_abscol) const
                    {
                        return likely(src) ? src->getTableMetadata(name, srcRow(out_absrow), srcCol(out_abscol)) : 0;
                    }
                    bool setSrcTableMetadata(const char * name, const char * value, idx out_absrow, idx out_abscol)
                    {
                        return likely(src) ? src->setTableMetadata(name, value, srcRow(out_absrow), srcCol(out_abscol)) : false;
                    }
                };

                SubTable left, right;
                idx _dim_top_header, _dim_left_header;

            public:
                sGluedTabular()
                {
                    _dim_top_header = 0;
                    _dim_left_header = 0;
                }
                void setDimHeader(idx top, idx left=0)
                {
                    _dim_top_header = top;
                    _dim_left_header = left;
                }
                void setLeft(sTabular * src, bool owned, sVec<idx> & borrow_row_map, sVec<idx> & borrow_col_map)
                {
                    left.init(src, owned, borrow_row_map, borrow_col_map);
                    right.setColMapOffset(left.col_map.dim());
                }
                void setRight(sTabular * src, bool owned, sVec<idx> & borrow_row_map, sVec<idx> & borrow_col_map)
                {
                    right.init(src, owned, borrow_row_map, borrow_col_map);
                }

                idx rows() const { return sMax<idx>(left.row_map.dim(), right.row_map.dim()) - _dim_top_header; }
                idx cols() const { return left.col_map.dim() + right.col_map.dim() - _dim_left_header; }
                sVariant::eType coltype(idx icol) const
                {
                    idx abscol = icol + _dim_left_header;
                    return abscol < left.col_map.dim() ? left.srcColtype(abscol) : right.srcColtype(abscol);
                }

                bool setColtype(idx icol, sVariant::eType t)
                {
                    idx abscol = icol + _dim_left_header;
                    return abscol < left.col_map.dim() ? left.setSrcColtype(abscol, t) : right.setSrcColtype(abscol, t);
                }
                const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char * defValue=0, idx flags=0) const
                {
                    idx abscol = icol + _dim_left_header;
                    idx absrow = irow + _dim_top_header;
                    return abscol < left.col_map.dim() ? left.printSrcCell(out, absrow, abscol, maxLen, defValue, flags, missing(irow, icol)) : right.printSrcCell(out, absrow, abscol, maxLen, defValue, flags, missing(irow, icol));
                }
                const char * cell(idx irow, idx icol, idx * cellLen) const
                {
                    idx abscol = icol + _dim_left_header;
                    idx absrow = irow + _dim_top_header;
                    return abscol < left.col_map.dim() ? left.srcCell(absrow, abscol, cellLen, missing(irow, icol)) : right.srcCell(absrow, abscol, cellLen, missing(irow, icol));
                }
                virtual ECellEncoding cellEncoding(idx irow, idx icol) const
                {
                    idx abscol = icol + _dim_left_header;
                    idx absrow = irow + _dim_top_header;
                    return abscol < left.col_map.dim() ? left.srcCellEncoding(absrow, abscol) : right.srcCellEncoding(absrow, abscol);
                }
                bool missing(idx irow, idx icol) const
                {
                    return irow < -_dim_top_header || irow >= rows() || icol < -_dim_left_header || icol >= cols();
                }
                bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const
                {
                    idx abscol = icol + _dim_left_header;
                    idx absrow = irow + _dim_top_header;
                    return abscol < left.col_map.dim() ? left.srcVal(out, absrow, abscol, noReinterpret, blankIsNull, missing(irow, icol)) : right.srcVal(out, absrow, abscol, noReinterpret, blankIsNull, missing(irow, icol));
                }
                idx dimTopHeader() const
                {
                    return _dim_top_header;
                }
                idx dimLeftHeader() const
                {
                    return _dim_left_header;
                }
                const char * getTableMetadata(const char * name, idx irow=0, idx icol=0) const
                {
                    idx abscol = icol + _dim_left_header;
                    idx absrow = irow + _dim_top_header;
                    return abscol < left.col_map.dim() ? left.getSrcTableMetadata(name, absrow, abscol) : right.getSrcTableMetadata(name, absrow, abscol);
                }
                bool setTableMetadata(const char * name, const char * value, idx irow=0, idx icol=0)
                {
                    idx abscol = icol + _dim_left_header;
                    idx absrow = irow + _dim_top_header;
                    return abscol < left.col_map.dim() ? left.setSrcTableMetadata(name, value, absrow, abscol) : right.setSrcTableMetadata(name, value, absrow, abscol);
                }
        };
    };
};

#endif
