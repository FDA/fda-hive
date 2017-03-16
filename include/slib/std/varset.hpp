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
#ifndef sLib_std_varset_hpp
#define sLib_std_varset_hpp

#include <slib/core/str.hpp>
#include <slib/core/var.hpp>
#include <slib/std/string.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/id.hpp>

namespace slib
{
    class sVarSet {

        sVar* m_cur_row;
        sDic< idx > m_names; // column names => col id - duplicate column names not supported!

    public:
        sDic< sVar > tbl;
        sStr buf;
        idx cols, rows, start;

        sVarSet()
        {
            empty();
        }
        sVarSet& addRow(const char * rowname=0, idx rowsize=0)
        {
            if(rowname) m_cur_row = tbl.set(rowname, rowsize);
            else m_cur_row = tbl.add(1);
            ++rows;
            return *this;
        }
        sVarSet& printCol(const char* fmt, ...) __attribute__((format(printf, 2, 3)))
        {
            sStr buf;
            sCallVarg(buf.vprintf, fmt);
            return addCol(buf.ptr(), buf.length());
        }
        sVarSet& addCol(const sStr& v)
        {
            return addCol(v.ptr(), v.length());
        }
        sVarSet& addCol(const char* p, idx size = 0)
        {
            if( !m_cur_row ) {
                addRow();
            }
            if( m_cur_row ) {
                if( !size && p ) {
                    size = sLen(p);
                }
                sMex::Pos ps;
                ps.size = size;
                ps.pos = buf.length();
                if( size ) {
                    buf.add(p, ps.size);
                }
                buf.add0();
                *m_cur_row->add(1) = ps;
                if( rows == 1 ) {
                    ++cols;
                }
            }
            return *this;
        }
        sVarSet& addCol(idx value)
        {
            return printCol("%"DEC, value);
        }
        sVarSet& addCol(udx value)
        {
            return printCol("%"UDEC, value);
        }
        sVarSet& addCol(bool value)
        {
            return addCol(value ? "1" : "0");
        }
        sVarSet& addCol(const sHiveId & idvalue)
        {
            sStr buf;
            idvalue.print(buf);
            return addCol(buf.ptr(), buf.length());
        }
        //! warning: each updateVal() call will usually grow sVarSet's memory use!
        bool updateVal(idx irow, idx icol, const char* p, idx size = 0)
        {
            if(irow < 0 || irow >= tbl.dim())
                return false;

            sVar* currow = tbl.ptr(irow);
            if(icol < 0 || icol >= currow->dim())
                return false;

            sMex::Pos* ps = currow->ptr(icol);
            if(!ps)
                return false;

            if(!size)
                size = sLen(p);

            if(size <= ps->size) {
                memmove(buf.ptr(ps->pos), p, size);
                if(size < ps->size)
                    buf[ps->pos + size] = 0;
                ps->size = size;
            } else {
                ps->pos = buf.length();
                if(size)
                    buf.add(p, size);
                buf.add0();
                ps->size = size;
            }
            return true;
        }
        sVarSet& setColId(idx col, const char* col_name)
        {
            idx* i = m_names.set(col_name);
            if( i ) { *i = col; }
            return *this;
        }
        idx colId(const char* col_name) const
        {
            const idx* c = col_name ? m_names.get(col_name) : 0;
            return (!c || *c < 0 || *c >= cols) ? -1 : *c;
        }

        idx rowId(const char* row_name) const
        {
            return tbl.find(row_name);
        }
        const char* colName(idx col) const
        {
            if(col >= 0 || col < cols) {
                for(idx i = 0; i < m_names.dim(); ++i) {
                    if( col == *(m_names.ptr(i)) ) {
                        return (const char*)m_names.id(i);
                    }
                }
            }
            return 0;
        }
        const char* val(idx irow, idx icol, idx* psize = 0) const
        {
            if(irow < 0 || irow >= tbl.dim()) {
                return 0;
            }
            const sVar* currow = tbl.ptr(irow);
            if(icol < 0 || icol >= currow->dim()) {
                return 0;
            }
            const sMex::Pos* p = currow->ptr(icol);
            if(!p) {
                return 0;
            }
            if(psize) {
                *psize = p->size;
            }
            return buf.ptr(p->pos);
        }
        char* val(idx irow, idx icol, idx* psize = 0)
        {
            return const_cast<char *>(const_cast<const sVarSet&>(*this).val(irow, icol, psize));
        }
        char* operator()(idx irow, idx icol, idx* psize = 0)
        {
            return val(irow, icol, psize);
        }
        const char* operator()(idx irow, idx icol, idx* psize = 0) const
        {
            return val(irow, icol, psize);
        }
        idx ival(idx irow, idx icol, idx default_value = 0) const
        {
            const char* p = val(irow, icol);
            if( p ) {
                sscanf(p, "%"DEC, &default_value);
            }
            return default_value;
        }
        udx uval(idx irow, idx icol, udx default_value = 0) const
        {
            const char* p = val(irow, icol);
            if( p ) {
                sscanf(p, "%"UDEC, &default_value);
            }
            return default_value;
        }
        real rval(idx irow, idx icol, real default_value = 0) const
        {
            const char* p = val(irow, icol);
            if( p ) {
                sscanf(p, "%lf", &default_value);
            }
            return default_value;
        }
        bool boolval(idx irow, idx icol, bool default_value = false) const
        {
            const char * p=val(irow, icol);
            return p ? sString::parseBool(p) : default_value;
        }
        idx boolival(idx irow, idx icol, idx default_value = 0) const
        {
            const char * p=val(irow, icol);
            return p ? sString::parseIBool(p) : default_value;
        }
        void empty(void)
        {
            m_cur_row = 0;
            m_names.empty();
            tbl.empty();
            buf.empty();
            rows = cols = start = 0;
        }
        // cntColNames < 0 - use own header
        // cntColNames == 0 - no header
        void printCSV(sStr& dst, idx cntColNames = -1, const char* colNames[] = 0) const
        {
            if( cntColNames > 0 ) {
                for(idx c = 0; c < cols; ++c) {
                    if( c > 0 ) {
                        dst.printf(",");
                    }
                    if( c < cntColNames && colNames && colNames[c] && colNames[c][0] ) {
                        sString::escapeForCSV(dst, colNames[c]);
                    } else {
                        dst.printf("col%"DEC, c + 1);
                    }
                }
                dst.printf("\n");
            } else if( cntColNames < 0 ) {
                for(idx c = 0; c < cols; ++c) {
                    if( c > 0 ) {
                        dst.printf(",");
                    }
                    if( c < m_names.dim() ) {
                        idx sz;
                        const char* p = (const char* )m_names.id(c, &sz);
                        sString::escapeForCSV(dst, p, sz);
                    } else {
                        dst.printf("col%"DEC, c + 1);
                    }
                }
                dst.printf("\n");
            }
            for(idx r = 0; r < rows; ++r) {
                for(idx c = 0; c < cols; ++c) {
                    if( c > 0 ) {
                        dst.printf(",");
                    }
                    idx sz = 0;
                    const char* p = val(r, c, &sz);
                    sString::escapeForCSV(dst, p, sz);
                }
                dst.printf("\n");
            }
        }
        static const char* escape4JSON(sStr& dst, const char* p, idx size)
        {
            if( p == 0 ) {
                dst.printf("null");
            } else if( size > 0 ) {
                sString::searchAndReplaceStrings(&dst, p, size,
                    "\""_"\\"_"/"_"\b"_"\f"_"\n"_"\r"_"\t"__,
                    "\\\""_"\\\\"_"\\/"_"\\b"_"\\f"_"\\n"_"\\r"_"\\t"__, 0, true);
            } else {
                dst.add0cut();
            }
            return dst.ptr();
        }
#ifdef _DEBUG
#define NL "\n"
#define INDENT "  "
#else
#define NL ""
#define INDENT ""
#endif
        // cntColNames < 0 - use own header
        // cntColNames == 0 - no header
        void printJSON(sStr& dst, bool force_array = false, idx cntColNames = -1, const char* colNames[] = 0) const
        {
            dst.printf("%s", (force_array || rows > 1) ? "[" : "");
            for(idx r = 0; r < rows; ++r) {
                if( r > 0 ) {
                    dst.printf(",");
                }
                dst.printf("%s", cntColNames ? NL INDENT"{"NL : "["NL);
                for(idx c = 0; c < cols; ++c) {
                    if( c > 0 ) {
                        dst.printf(",%s", cntColNames ? NL : "");
                    }
                    if( cntColNames != 0 ) {
                        sStr tmp;
                        if( cntColNames > 0 ) {
                            if( c < cntColNames && colNames[c] && colNames[c][0] ) {
                                escape4JSON(tmp, colNames[c], strlen(colNames[c]));
                            } else {
                                tmp.printf("col%"UDEC, c + 1);
                            }
                        } else if( cntColNames < 0 ) {
                            if( c < m_names.dim() ) {
                                idx sz;
                                const char* p = (const char*) m_names.id(c, &sz);
                                escape4JSON(tmp, p, sz);
                            } else {
                                tmp.printf("col%"UDEC, c + 1);
                            }
                        }
                        dst.printf(INDENT INDENT"\"%s\":", tmp.ptr());
                    }
                    sStr t;
                    idx sz = 0;
                    const char* p = val(r, c, &sz);
                    dst.printf("\"%s\"", escape4JSON(t, p, sz));
                }
                dst.printf("%s", cntColNames ? NL INDENT"}" : "]");
            }
            dst.printf("%s", (force_array || rows > 1) ? NL"]" : "");
        }
#undef NL
#undef INDENT
        void printProp(sStr& dst, idx col_id = 0, idx col_name = 1, idx col_row = 2, idx col_val = 3, const char * overloadObjID = 0) const
        {
            for(idx r = 0; r < rows; ++r) {
                const char * row = val(r, col_row, 0);
                const char * oid = overloadObjID ? overloadObjID : val(r, col_id);
                if( row && *row ) {
                    dst.printf("\nprop.%s.%s.%s=%s", oid, val(r, col_name), row, val(r, col_val));
                } else {
                    dst.printf("\nprop.%s.%s=%s", oid, val(r, col_name), val(r, col_val));
                }
            }
        }
        //! Print as "&prop.id.foo.1=hello&prop.id.bar.2=bye" etc. string, with names/values encoded for use in a url
        /*! \param dst where to print
         *  \param col_id column with hive ids
         *  \param col_name column with prop names
         *  \param col_row column with prop paths
         *  \param col_val column with prop values
         *  \param overloadObjID use this as id instead of col_id's value
         *  \paran excludeCols00 0-delimeted, 00-terminated list of prop names to ignore; or 0 to print everything
         *  \returns pointer to start of printed string in dst */
        const char * printPropUrl(sStr & dst, idx col_id = 0, idx col_name = 1, idx col_row = 2, idx col_val = 3, const char * overloadObjID = 0, const char * excludeCols00 = 0) const;
        idx reorderRows(idx * new_order, idx irow_start = 0, idx cnt = sIdxMax, bool extract = false);
    };
}
#endif
