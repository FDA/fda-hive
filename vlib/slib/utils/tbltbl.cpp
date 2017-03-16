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

#include <slib/utils/tbltbl.hpp>

using namespace slib;

#define DEFINE_S_CAT_TABULAR_ABSROW_SUB(error_ret) \
    if (unlikely(!_subs.dim() || irow >= _row2sub.dim())) \
        return (error_ret); \
    const SubTable & sub = irow < 0 ? _subs[0] : _subs[_row2sub[irow]]

idx sCatTabular::rows() const
{
    if (unlikely(!_subs.dim()))
        return 0;
    const SubTable & last = _subs[_subs.dim() - 1];
    return last.row_offset + last.tbl->rows();
}

idx sCatTabular::cols() const
{
    if (unlikely(!_subs.dim()))
        return 0;
    return _subs[0].tbl->cols();
}

sVariant::eType sCatTabular::coltype(idx icol) const
{
    if (unlikely(!_subs.dim()))
        return sVariant::value_NULL;
    return _subs[0].tbl->coltype(icol);
}

bool sCatTabular::setColtype(idx icol, sVariant::eType t)
{
    for (idx i=0; i<_subs.dim(); i++) {
        if (!_subs[i].tbl->setColtype(icol, t))
            return false;
    }
    return true;
}

sVariant::eType sCatTabular::type(idx irow, idx icol) const
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(sVariant::value_NULL);
    return sub.tbl->type(irow - sub.row_offset, icol);
}

idx sCatTabular::colId(const char * colname, idx colname_len, idx ic) const
{
    if (unlikely(!_subs.dim()))
        return -sIdxMax;
    return _subs[0].tbl->colId(colname, colname_len, ic);
}

idx sCatTabular::colIdDim(const char * colname, idx colname_len) const
{
    if (unlikely(!_subs.dim()))
        return -sIdxMax;
    return _subs[0].tbl->colIdDim(colname, colname_len);
}

const char * sCatTabular::printCell(sStr & out, idx irow, idx icol, idx maxLen, const char * defValue, idx flags) const
{
    if (unlikely(!_subs.dim() || irow >= _row2sub.dim())) {
        out.add0cut();
        return sStr::zero;
    }

    const SubTable & sub = irow < 0 ? _subs[0] : _subs[_row2sub[irow]];
    return sub.tbl->printCell(out, irow - sub.row_offset, icol, maxLen, defValue, flags);
}

const char * sCatTabular::cell(idx irow, idx icol, idx * cellLen) const
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(0);
    return sub.tbl->cell(irow - sub.row_offset, icol, cellLen);
}

sTabular::ECellEncoding sCatTabular::cellEncoding(idx irow, idx icol) const
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(sTabular::eEncodingUnspecified);
    return sub.tbl->cellEncoding(irow - sub.row_offset, icol);
}

bool sCatTabular::missing(idx irow, idx icol) const
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(true);
    return sub.tbl->missing(irow - sub.row_offset, icol);
}

bool sCatTabular::val(sVariant & out, idx irow, idx icol, bool noReinterpret/*=false*/, bool blankIsNull/*=false*/) const
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(false);
    idx abscol = icol + sub.tbl->dimLeftHeader();
    if (!noReinterpret && abscol >= 0 && abscol < _colReinterp.dim() && _colReinterp[abscol].dim() && icol >= 0) {
        if (likely(irow >= 0 && irow < _colReinterp[abscol].dim())) {
            out.setReal(_colReinterp[abscol][irow].r);
            return true;
        } else {
            out.setNull();
            return false;
        }
    }

    return sub.tbl->val(out, irow - sub.row_offset, icol, noReinterpret, blankIsNull);
}

const char * sCatTabular::getTableMetadata(const char * name, idx irow, idx icol) const
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(0);
    return sub.tbl->getTableMetadata(name, irow - sub.row_offset, icol);
}

bool sCatTabular::setTableMetadata(const char * name, const char * value, idx irow, idx icol)
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(false);
    return sub.tbl->setTableMetadata(name, value, irow - sub.row_offset, icol);
}

void sCatTabular::pushSubTable(sTabular * tbl, bool owned)
{
    SubTable * psub = _subs.add(1);
    psub->tbl = tbl;
    psub->owned = owned;
    if (_subs.dim() > 1) {
        SubTable * pprev = psub - 1;
        psub->row_offset = pprev->row_offset + pprev->tbl->rows();
    } else {
        _dim_top_header = tbl->dimTopHeader();
        _dim_left_header = tbl->dimLeftHeader();
    }
    idx dim_total = psub->row_offset + tbl->rows();
    idx isub = psub - _subs.ptr();
    _row2sub.resize(dim_total);
    for (idx i=psub->row_offset; i<dim_total; i++)
        _row2sub[i] = isub;
}

const sTabular * sCatTabular::getSubTableAtRow(idx irow) const
{
    DEFINE_S_CAT_TABULAR_ABSROW_SUB(0);
    return sub.tbl;
}

sReorderedTabular::sReorderedTabular(sTabular * src, bool src_owned)
{
    _src = src;
    _src_owned = src_owned;
    _transposed = false;
    _dim_top_header = src->dimTopHeader();
    _dim_left_header = src->dimLeftHeader();
    _rows_remapped = _cols_remapped = false;

    _rowtype = sVariant::value_NULL;
    for (idx icol=0; icol < src->cols(); icol++) {
        _rowtype = sTabular::mergeTypes(_rowtype, src->coltype(icol));
    }

    updateColIds();
}

void sReorderedTabular::setTransposed(bool t, idx topHeader, idx leftHeader)
{
    if (t == _transposed) {
        return;
    }
    if (!_cols_remapped) {
        initColsMap();
    }
    if (!_rows_remapped) {
        initRowsMap();
    }
    _transposed = t;
    _rowmap.mex()->swap(_colmap.mex());
    _dim_top_header = topHeader;
    _dim_left_header = leftHeader;
    updateColIds();
}

void sReorderedTabular::updateColIds()
{
    _colIds.empty();
    sStr buf;
    for (idx icol=-dimLeftHeader(); icol<cols(); icol++) {
        buf.cut(0);
        printTopHeader(buf, icol);
        if (buf.length()) {
            *_colIds.setString(buf.ptr())->add(1) = icol;
        }
    }
}

void sReorderedTabular::initRowsMapInto(sVec<sReorderedTabular::TypedSource> & rowmap) const
{
    if (_rows_remapped) {
        rowmap.resize(_rowmap.dim());
        memcpy(rowmap.ptr(), _rowmap.ptr(), _rowmap.mex()->pos());
    } else {
        idx header_dim = _src->dimTopHeader();
        idx rowmap_dim = _src->rows() + header_dim;
        rowmap.resize(rowmap_dim);
        rowmap.cut(rowmap_dim);
        for (idx absrow = 0; absrow < rowmap_dim; absrow++) {
            idx irow = absrow - header_dim;
            rowmap[absrow].src = irow;
            rowmap[absrow].type = irow >= 0 ? _rowtype : sVariant::value_STRING;
        }
    }
}

void sReorderedTabular::initColsMapInto(sVec<sReorderedTabular::TypedSource> & colmap) const
{
    idx header_dim = _src->dimLeftHeader();
    idx colmap_dim = _src->cols() + header_dim;
    colmap.resize(colmap_dim);
    colmap.cut(colmap_dim);
    for (idx abscol = 0; abscol < colmap_dim; abscol++) {
        idx icol = abscol - header_dim;
        colmap[abscol].src = icol;
        colmap[abscol].type = _src->coltype(icol);
    }
}

sEditedTabular::sEditedTabular(sTabular * src, bool src_owned)
{
    _src = src;
    _src_owned = src_owned;
    _dim_top_header = _src->dimTopHeader();
    _dim_left_header = _src->dimLeftHeader();
    _rows = _src->rows();
    _cols = _src->cols();
}

sVariant::eType sEditedTabular::type(idx irow, idx icol) const
{
    if( _src->missing(irow, icol) ) {
        return _src->coltype(icol);
    } else {
        return _src->type(irow, icol);
    }
}

idx sEditedTabular::colId(const char * colname, idx colname_len/* = 0*/, idx ic/* = 0*/) const
{
    if( unlikely(_colIds.dim()) ) {
        const sVec<idx> * ids = _colIds.get(colname, colname_len);
        if( likely(ids && ic >= 0 && ic < ids->dim()) ) {
            return *ids->ptr(ic);
        }
        return -sIdxMax;
    } else {
        return _src->colId(colname, colname_len, ic);
    }
}

idx sEditedTabular::colIdDim(const char * colname, idx colname_len/* = 0*/) const
{
    if( unlikely(_colIds.dim()) ) {
        const sVec<idx> * ids = _colIds.get(colname, colname_len);
        return likely(ids) ? ids->dim() : 0;
    } else {
        return _src->colIdDim(colname, colname_len);
    }
}

const char * sEditedTabular::printCell(sStr & out, idx irow, idx icol, idx maxLen/*=0*/, const char * defValue/*=0*/, idx flags/*=0*/) const
{
    idx ival = getIval(irow, icol);
    if( ival >= 0 ) {
        idx outStart = out.length();
        idx len = 0;
        const char * s = static_cast<const char *>(_values.id(ival, &len));
        if( maxLen ) {
            len = sMin<idx>(len, maxLen);
        }
        if( flags & fForCSV ) {
            sString::escapeForCSV(out, s, len);
        } else {
            out.addString(s, len);
        }
        return out.ptr(outStart);
    } else {
        return _src->printCell(out, irow, icol, maxLen, defValue, flags);
    }
}
const char * sEditedTabular::cell(idx irow, idx icol, idx * cellLen) const
{
    idx ival = getIval(irow, icol);
    if( ival >= 0 ) {
        const char * ret = static_cast<const char *>(_values.id(ival, cellLen));
        return ret;
    } else {
        return _src->cell(irow, icol, cellLen);
    }
}

bool sEditedTabular::val(sVariant & out, idx irow, idx icol, bool noReinterpret/*=false*/, bool blankIsNull/*=false*/) const
{
    idx ival = getIval(irow, icol);
    if( ival >= 0 ) {
        idx abscol = icol + dimLeftHeader();
        if( !noReinterpret && abscol >= 0 && abscol < _colReinterp.dim() && _colReinterp[abscol].dim() && icol >= 0 ) {
            if( likely(irow >= 0 && irow < _colReinterp[abscol].dim()) ) {
                out.setReal(_colReinterp[abscol][irow].r);
                return true;
            } else {
                out.setNull();
                return false;
            }
        }

        const char * s = static_cast<const char *>(_values.id(ival));

        if( blankIsNull ) {
            bool is_blank = true;
            for(idx i = 0; s[i]; i++) {
                if( !isspace(s[i]) ) {
                    is_blank = false;
                    break;
                }
            }
            if( is_blank ) {
                out.setNull();
                return true;
            }
        }

        out.parseScalarType(s, likely(irow >= 0) ? coltype(icol) : sVariant::value_STRING);
        return true;
    } else {
        return _src->val(out, irow, icol, noReinterpret, blankIsNull);
    }
}

void sEditedTabular::editCell(idx irow, idx icol, const char * decoded_value, idx val_len/* = 0 */)
{
    if( !decoded_value ) {
        decoded_value = sStr::zero;
    }

    idx ivalue = -sIdxMax;
    _values.setString(decoded_value, val_len, &ivalue);

    idx coord[2];
    coord[0] = irow;
    coord[1] = icol;
    *_rowcol2ival.set(coord, sizeof(coord)) = ivalue;

    if( irow < -_dim_top_header ) {
        _dim_top_header = -irow;
    } else if ( irow >= _rows ) {
        _rows = irow + 1;
    }

    if( icol < -_dim_left_header ) {
        _dim_left_header = -irow;
    } else if ( icol >= _cols ) {
        _cols = icol + 1;
    }

    if( irow < 0 ) {
        _colIds.empty();
        sStr header_buf;

        for (idx icol=-dimLeftHeader(); icol<cols(); icol++) {
            header_buf.cut(0);
            printTopHeader(header_buf, icol);
            if (header_buf.length()) {
                *_colIds.setString(header_buf.ptr())->add(1) = icol;
            }
        }
    }
}
