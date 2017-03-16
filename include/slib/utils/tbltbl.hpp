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
#ifndef sLib_utils_tbltbl_hpp
#define sLib_utils_tbltbl_hpp

#include <slib/core/vec.hpp>
#include <slib/utils/tbl.hpp>

/*! \file Tables made of other tables */

namespace slib
{
    //! A table made by stacking a sequence of tables one after another, and using only the first one's header
    class sCatTabular: public sTabular
    {
    protected:
        struct SubTable {
            sTabular * tbl;
            idx row_offset;
            bool owned;

            SubTable() { sSet(this, 0); }
            ~SubTable()
            {
                if (owned)
                    delete tbl;
            }
        };

        sVec<SubTable> _subs;
        sVec<idx> _row2sub; // map from sCatTabular row to index into _subs
        idx _dim_top_header;
        idx _dim_left_header;

    public:
        sCatTabular()
        {
            _dim_top_header = _dim_left_header = 0;
        }
        virtual ~sCatTabular() {}

        // sTabular interface
        virtual idx rows() const;
        virtual idx cols() const;
        virtual sVariant::eType coltype(idx icol) const;
        virtual bool setColtype(idx icol, sVariant::eType t);
        virtual sVariant::eType type(idx irow, idx icol) const;
        virtual idx colId(const char * colname, idx colname_len=0, idx ic=0) const;
        virtual idx colIdDim(const char * colname, idx colname_len=0) const;
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char * defValue=0, idx flags=0) const;
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const;
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const;
        virtual bool missing(idx irow, idx icol) const;
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const;
        virtual idx dimTopHeader() const { return _dim_top_header; }
        virtual idx dimLeftHeader() const { return _dim_left_header; }
        virtual const char * getTableMetadata(const char * name, idx irow=0, idx icol=0) const;
        virtual bool setTableMetadata(const char * name, const char * value, idx irow=0, idx icol=0);

        void pushSubTable(sTabular * tbl, bool owned=true);
        idx dimSubTables() const { return _subs.dim(); }
        const sTabular * getSubTable(idx i) const { return i > 0 && i < _subs.dim() ? _subs[i].tbl : 0; }
        const sTabular * getSubTableAtRow(idx irow) const;
    };

    //! A table made by reordering another table's rows or columns
    class sReorderedTabular: public sTabular
    {
    public:
        struct TypedSource {
            idx src; //!< source row or column
            sVariant::eType type;
        };
    protected:
        sTabular * _src;
        bool _src_owned;
        sVec<TypedSource> _rowmap; // map from output absrow to source row/col
        sVec<TypedSource> _colmap; // map from output abscol to source col/row
        bool _rows_remapped, _cols_remapped;
        idx _dim_top_header;
        idx _dim_left_header;
        bool _transposed;
        sVariant::eType _rowtype;

        inline bool validRow(idx irow) const { return irow >= -_dim_top_header && irow < rows(); }
        inline bool validCol(idx icol) const { return icol >= -_dim_left_header && icol < cols(); }

    public:
        sReorderedTabular(sTabular * src, bool src_owned);

        virtual ~sReorderedTabular()
        {
            if (_src_owned)
                delete _src;
        }

        // sTabular interface
        virtual idx rows() const { return (_rows_remapped ? _rowmap.dim() : _src->rows() + _src->dimTopHeader()) - _dim_top_header; }
        virtual idx cols() const { return (_cols_remapped ? _colmap.dim() : _src->cols() + _src->dimLeftHeader()) - _dim_left_header; }
        virtual sVariant::eType coltype(idx icol) const
        {
            if (unlikely(!validCol(icol))) {
                return sVariant::value_NULL;
            }
            if (_cols_remapped) {
                return _colmap[icol + _dim_left_header].type;
            } else {
                return _src->coltype(icol + _dim_left_header - _src->dimLeftHeader());
            }
        }
        virtual bool setColtype(idx icol, sVariant::eType t)
        {
            if (unlikely(!validCol(icol))) {
                return false;
            }
            if (unlikely(!_cols_remapped)) {
                initColsMap();
            }
            _colmap[icol + _dim_left_header].type = t;
            return true;
        }
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char * defValue=0, idx flags=0) const
        {
            idx src_row = mapRow(irow);
            idx src_col = mapCol(icol);
            if (_transposed) {
                sSwapI(src_row, src_col);
            }
            return _src->printCell(out, src_row, src_col, maxLen, defValue, flags);
        }
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const
        {
            idx src_row = mapRow(irow);
            idx src_col = mapCol(icol);
            if (_transposed) {
                sSwapI(src_row, src_col);
            }
            return _src->cell(src_row, src_col, cellLen);
        }
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const
        {
            idx src_row = mapRow(irow);
            idx src_col = mapCol(icol);
            if (_transposed) {
                sSwapI(src_row, src_col);
            }
            return _src->cellEncoding(src_row, src_col);
        }
        virtual bool missing(idx irow, idx icol) const
        {
            idx src_row = mapRow(irow);
            idx src_col = mapCol(icol);
            if (_transposed) {
                sSwapI(src_row, src_col);
            }
            return _src->missing(src_row, src_col);
        }
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const
        {
            idx src_row = mapRow(irow);
            idx src_col = mapCol(icol);
            if (_transposed) {
                sSwapI(src_row, src_col);
            }
            return _src->val(out, src_row, src_col, noReinterpret, blankIsNull);
        }
        virtual idx dimTopHeader() const
        {
            return _dim_top_header;
        }
        virtual idx dimLeftHeader() const
        {
            return _dim_left_header;
        }
        virtual const char * getTableMetadata(const char * name, idx irow=0, idx icol=0) const
        {
            idx src_row = mapRow(irow);
            idx src_col = mapCol(icol);
            if (_transposed) {
                sSwapI(src_row, src_col);
            }
            return _src->getTableMetadata(name, src_row, src_col);
        }
        virtual bool setTableMetadata(const char * name, const char * value, idx irow=0, idx icol=0)
        {
            idx src_row = mapRow(irow);
            idx src_col = mapCol(icol);
            if (_transposed) {
                sSwapI(src_row, src_col);
            }
            return _src->setTableMetadata(name, value, src_row, src_col);
        }

        inline idx mapRow(idx irow) const
        {
            if (_rows_remapped) {
                return likely(validRow(irow)) ? _rowmap[irow + _dim_top_header].src : -sIdxMax;
            } else {
                return irow;
            }
        }

        inline idx mapCol(idx icol) const {
            if (_cols_remapped) {
                return likely(validCol(icol)) ? _colmap[icol + _dim_left_header].src : -sIdxMax;
            } else {
                return icol;
            }
        }

        virtual sVariant::eType rowtype(idx irow) const
        {
            if (unlikely(!validRow(irow))) {
                return sVariant::value_NULL;
            }
            if (_rows_remapped) {
                return _rowmap[irow + _dim_left_header].type;
            } else {
                return irow >= 0 ? _rowtype : sVariant::value_STRING;
            }
        }

        //! \warning Call this after modifying size of top header, moving header row, or reordering columns
        void updateColIds();

        bool isTransposed() const { return _transposed; }
        void setTransposed(bool t, idx topHeader=1, idx leftHeader=0);
        void setDimTopHeader(idx i)
        {
            if (i != _dim_top_header) {
                _dim_top_header = i;
                updateColIds();
            }
        }
        void setDimLeftHeader(idx i)
        {
            if (i != _dim_left_header) {
                _dim_left_header = i;
                updateColIds();
            }
        }

        bool hasRowsRemapped() const { return _rows_remapped; }
        bool hasColsRemapped() const { return _cols_remapped; }
        void initRowsMap()
        {
            _rows_remapped = false;
            initRowsMapInto(_rowmap);
            _rows_remapped = true;
        }
        void initColsMap()
        {
            _cols_remapped = false;
            initColsMapInto(_colmap);
            _cols_remapped = true;
        }
        void initRowsMapInto(sVec<TypedSource> & rowmap) const;
        void initColsMapInto(sVec<TypedSource> & colmap) const;
        void borrowRowsMap(sVec<TypedSource> & from_rowmap)
        {
            _rows_remapped = true;
            _rowmap.destroy();
            _rowmap.mex()->borrow(from_rowmap.mex());
        }
        void borrowColsMap(sVec<TypedSource> & from_colmap)
        {
            _cols_remapped = true;
            _colmap.destroy();
            _colmap.mex()->borrow(from_colmap.mex());
        }
        sVec<TypedSource> & getRowsMap() { return _rowmap; }
        sVec<TypedSource> & getColsMap() { return _colmap; }
        const sVec<TypedSource> & getRowsMap() const { return _rowmap; }
        const sVec<TypedSource> & getColsMap() const { return _colmap; }
    };

    //! A table made by editing (overriding) a small number of cells in another table
    /*! \warning For the moment, there is no provision for garbage-collecting old values of a cell which is edited multiple times; the expectation is that the total number of edits will be small relative to the size of the source table.
     */
    class sEditedTabular : public sTabular
    {
    protected:
        sTabular * _src;
        bool _src_owned;
        idx _dim_top_header;
        idx _dim_left_header;
        idx _rows;
        idx _cols;
        sDic<idx> _values; // buffer of uniquified new values
        sDic<idx> _rowcol2ival; // map from (row, column) to index in _values (or -1 to undo the edit)

        idx getIval(idx irow, idx icol) const
        {
            idx coord[2];
            coord[0] = irow;
            coord[1] = icol;
            const idx * pret = _rowcol2ival.get(coord, sizeof(coord));
            return pret ? *pret : -1;
        }

    public:
        sEditedTabular(sTabular * src, bool src_owned);

        virtual ~sEditedTabular()
        {
            if( _src_owned )
                delete _src;
        }

        // sTabular interface
        virtual idx rows() const { return _rows; }
        virtual idx cols() const { return _cols; }
        virtual sVariant::eType coltype(idx icol) const { return _src->coltype(icol); }
        virtual bool setColtype(idx icol, sVariant::eType t) { return _src->setColtype(icol, t); }
        virtual sVariant::eType type(idx irow, idx icol) const;
        virtual idx colId(const char * colname, idx colname_len=0, idx ic=0) const;
        virtual idx colIdDim(const char * colname, idx colname_len=0) const;
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char * defValue=0, idx flags=0) const;
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const;
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const
        {
            idx ival = getIval(irow, icol);
            return ival >= 0 ? eEncodingNone : _src->cellEncoding(irow, icol);
        }
        virtual bool missing(idx irow, idx icol) const
        {
            idx ival = getIval(irow, icol);
            return ival >= 0 ? true : _src->missing(irow, icol);
        }
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const;
        virtual idx dimTopHeader() const { return _dim_top_header; }
        virtual idx dimLeftHeader() const { return _dim_left_header; }
        virtual const char * getTableMetadata(const char * name, idx irow=0, idx icol=0) const { return _src->getTableMetadata(name, irow, icol); }
        virtual bool setTableMetadata(const char * name, const char * value, idx irow=0, idx icol=0) { return _src->setTableMetadata(name, value, irow, icol); }

        //! Write a value into a specific cell
        /*! \param irow row index of cell to edit
            \param icol column index of cell to edit
            \param decoded_value value to write; must be decoded, original data (*not* CSV-encoded!)
            \param decoded_value_len length of decoded_value, or 0 if decoded_value is 0-terminated string */
        void editCell(idx irow, idx icol, const char * decoded_value, idx decoded_value_len = 0);
    };
};

#endif
