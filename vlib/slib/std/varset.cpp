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

#include <slib/std/varset.hpp>
#include <slib/std/url.hpp>

using namespace slib;

const char * sVarSet::printPropUrl(sStr & dst, idx col_id, idx col_name, idx col_row, idx col_val, const char * overloadObjID, const char * excludeNames00) const
{
    idx start = dst.length();
    for(idx r = 0; r < rows; ++r) {
        const char * name = val(r, col_name);
        const char * row = val(r, col_row, 0);
        const char * objid = overloadObjID ? overloadObjID : val(r, col_id, 0);
        if( excludeNames00 && sString::compareChoice(name, excludeNames00, 0, false, 0, true) > 0 ) {
            continue;
        }
        dst.addString("&prop.");
        URLEncode(objid, dst, eUrlEncode_URIQueryNameEscapeQuotes);
        dst.addString(".");
        URLEncode(name, dst, eUrlEncode_URIQueryNameEscapeQuotes);
        if( row && *row ) {
            dst.addString(".");
            URLEncode(row, dst, eUrlEncode_URIQueryNameEscapeQuotes);
        }
        dst.addString("=");
        URLEncode(val(r, col_val), dst, eUrlEncode_URIQueryValueEscapeQuotes);
    }
    return dst.ptr(start);
}

idx sVarSet::reorderRows(idx * new_order, idx irow_start/* = 0 */, idx cnt/* = sIdxMax */, bool extract /* = false */)
{
    if( irow_start < 0 || irow_start > rows ) {
        return 0;
    }
    cnt = sMin<idx>(cnt, rows - irow_start);
    if( cnt <= 0 ) {
        return 0;
    }

    sDic<sVar> new_tbl;
    for(idx ir = 0; ir < rows; ir++) {
        idx ir_src = ir;
        if( ir >= irow_start && ir < irow_start + cnt ) {
            ir_src = new_order[ir - irow_start];
            if( ir_src < 0 || ir_src >= rows ) {
                return 0;
            }
        } else if ( extract )
            continue;

        sVar * src_row = tbl.ptr(ir_src);
        idx rowname_len = 0;
        const char * rowname = static_cast<const char *>(tbl.id(ir_src, &rowname_len));

        sVar * new_row = rowname ? new_tbl.set(rowname, rowname_len) : new_tbl.add(1);
        new_row->borrow(src_row);
    }
    if(extract) {
        rows = new_tbl.dim();
    }
    tbl.borrow(&new_tbl);

    return cnt;
}
