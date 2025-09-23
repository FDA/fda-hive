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

#include <xlib/sqlite.hpp>
#include <sqlite3.h>

#define sSqlite_db static_cast<sqlite3*>(_db)
#define sSqlite_pdb reinterpret_cast<sqlite3**>(&_db)
#define sSqlite_stmt static_cast<sqlite3_stmt*>(_stmt)
#define sSqlite_pstmt reinterpret_cast<sqlite3_stmt**>(&_stmt)

using namespace slib;

const idx sSqlite::max_deadlock_retries = 16;

void sSqlite::clearAll()
{
    _db = 0;
    _stmt = 0;
    _in_transaction = 0;
    _errno = 0;
    _had_deadlocked = false;
    _have_result_row = false;
    _stmt_done = false;
    _res_ncols = 0;
    if( _err_msg.ptr() ) {
        _err_msg.cut0cut();
    }
}

sSqlite::sSqlite()
{
    clearAll();
}

sSqlite::sSqlite(const char * filepath, bool readonly)
{
    clearAll();
    reset(filepath, readonly);
}

bool sSqlite::reset(const char * filepath, bool readonly)
{
    resultClose();
    sqlite3_close(sSqlite_db);

    clearAll();

    if( filepath ) {
        _errno = sqlite3_open_v2(filepath, sSqlite_pdb, readonly ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
        if( _errno == SQLITE_OK ) {
            return true;
        } else {
            if( _db ) {
                _err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
            } else {
                _err_msg.cutAddString(0, sqlite3_errstr(_errno));
            }
            sqlite3_close(sSqlite_db);
            _db = 0;
        }
    }
    return false;
}

sSqlite::~sSqlite()
{
    resultClose();
    sqlite3_close(sSqlite_db);
    _db = 0;
}

bool sSqlite::execute(const char * sqlfmt, ...)
{
    sStr sql;
    va_list ap;
    va_start(ap, sqlfmt);
    sql.vprintf(sqlfmt, ap);
    va_end(ap);

    return executeExact(sql.ptr());
}

bool sSqlite::executeExact(const char * sql)
{
    resultClose();

    char * errmsg = 0;
    _errno = sqlite3_exec(sSqlite_db, sql, 0, 0, &errmsg);
    if( errmsg ) {
        _err_msg.cutAddString(0, errmsg);
        sqlite3_free(errmsg);
    } else if( _errno != SQLITE_OK ) {
        _err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
    }

    return _errno == SQLITE_OK;
}

idx sSqlite::getTable(sVarSet & out_tbl, const char * sqlfmt, ...)
{
    sStr sql;
    va_list ap;
    va_start(ap, sqlfmt);
    sql.vprintf(sqlfmt, ap);
    va_end(ap);

    return getTableExact(out_tbl, sql.ptr());
}

int getTable_callback(void * param, int ncols, char ** values, char ** colnames)
{
    sVarSet & out_tbl = *static_cast<sVarSet*>(param);
    if( !out_tbl.rows ) {
        for(idx i = 0; i < ncols; i++) {
            out_tbl.setColId(i, colnames[i]);
        }
    }
    out_tbl.addRow();
    for(idx i = 0; i < ncols; i++) {
        out_tbl.addCol(values[i]);
    }
    return 0;
}

idx sSqlite::getTableExact(sVarSet & out_tbl, const char * sql)
{
    resultClose();

    char * errmsg = 0;
    idx start_rows = out_tbl.rows;
    _errno = sqlite3_exec(sSqlite_db, sql, getTable_callback, &out_tbl, &errmsg);
    if( errmsg ) {
        _err_msg.cutAddString(0, errmsg);
        sqlite3_free(errmsg);
    } else if( _errno != SQLITE_OK ) {
        _err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
    }

    return out_tbl.rows - start_rows;
}

const char * sSqlite::getValue(sStr & out, const char * defval, const char * sqlfmt, ...)
{
    sStr sql;
    va_list ap;
    va_start(ap, sqlfmt);
    sql.vprintf(sqlfmt, ap);
    va_end(ap);

    return getValueExact(out, sql.ptr(), defval);
}

const char * sSqlite::getValueExact(sStr & out, const char * sql, const char * defval)
{
    const char * val = 0;
    idx len = 0;
    idx start_pos = out.length();

    if( resultOpenExact(sql) ) {
        udx saved_errno = SQLITE_OK;
        sStr saved_err_msg;

        if( resultNextRow() ) {
            val = resultValue(0, 0, &len);
            if( val && len ) {
                out.addString(val, len);
            }
        } else {
            saved_errno = saveError(&saved_err_msg);
        }
        resultClose();
        if( saved_errno ) {
            restoreError(saved_errno, &saved_err_msg);
        }
    }

    if( !val || !len ) {
        val = defval;
        len = sLen(defval);

        if( val && len ) {
            out.addString(val, len);
        } else if( val ) {
            out.add0cut();
        }
    }

    return val ? out.ptr(start_pos) : 0;
}

idx sSqlite::getIValue(idx defval, const char * sqlfmt, ...)
{
    sStr sql;
    va_list ap;
    va_start(ap, sqlfmt);
    sql.vprintf(sqlfmt, ap);
    va_end(ap);

    return getIValueExact(sql.ptr(), defval);
}

idx sSqlite::getIValueExact(const char * sql, idx defval)
{
    idx ret = defval;
    if( resultOpenExact(sql) ) {
        udx saved_errno = SQLITE_OK;
        sStr saved_err_msg;

        if( resultNextRow() ) {
            ret = resultIValue(0);
        } else {
            saved_errno = saveError(&saved_err_msg);
        }
        resultClose();
        if( saved_errno ) {
            restoreError(saved_errno, &saved_err_msg);
        }
    }
    return ret;
}

udx sSqlite::getUValue(udx defval, const char * sqlfmt, ...)
{
    sStr sql;
    va_list ap;
    va_start(ap, sqlfmt);
    sql.vprintf(sqlfmt, ap);
    va_end(ap);

    return getIValueExact(sql.ptr(), defval);
}

udx sSqlite::getUValueExact(const char * sql, udx defval)
{
    udx ret = defval;
    if( resultOpenExact(sql) ) {
        udx saved_errno = SQLITE_OK;
        sStr saved_err_msg;

        if( resultNextRow() ) {
            ret = resultUValue(0);
        } else {
            saved_errno = saveError(&saved_err_msg);
        }
        resultClose();
        if( saved_errno ) {
            restoreError(saved_errno, &saved_err_msg);
        }
    }
    return ret;
}

real sSqlite::getRValue(real defval, const char * sqlfmt, ...)
{
    sStr sql;
    va_list ap;
    va_start(ap, sqlfmt);
    sql.vprintf(sqlfmt, ap);
    va_end(ap);

    return getIValueExact(sql.ptr(), defval);
}

real sSqlite::getRValueExact(const char * sql, real defval)
{
    real ret = defval;
    if( resultOpenExact(sql) ) {
        udx saved_errno = SQLITE_OK;
        sStr saved_err_msg;

        if( resultNextRow() ) {
            ret = resultRValue(0);
        } else {
            saved_errno = saveError(&saved_err_msg);
        }
        resultClose();
        if( saved_errno ) {
            restoreError(saved_errno, &saved_err_msg);
        }
    }
    return ret;
}

bool sSqlite::resultOpen(const char * sqlfmt, ...)
{
    sStr sql;
    va_list ap;
    va_start(ap, sqlfmt);
    sql.vprintf(sqlfmt, ap);
    va_end(ap);

    return resultOpenExact(sql.ptr());
}

bool sSqlite::resultOpenExact(const char * sql)
{
    resultClose();

    _errno = sqlite3_prepare_v2(sSqlite_db, sql, -1, sSqlite_pstmt, 0);
    if( _errno ) {
        sStr saved_err_msg;
        udx saved_errno = _errno;
        saved_err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
        resultClose();
        restoreError(saved_errno, &saved_err_msg);
        return false;
    } else {
        _res_ncols = sqlite3_column_count(sSqlite_stmt);
        _res_colnames.cut0cut();
        _res_colname_pos.resize(_res_ncols);
        for(idx icol = 0; icol < _res_ncols; icol++) {
            _res_colname_pos[icol] = _res_colnames.length();
            _res_colnames.addString(sqlite3_column_name(sSqlite_stmt, icol));
            _res_colnames.add0();
        }
    }
    return true;
}

bool sSqlite::resultNextRow()
{
    _have_result_row = false;

    if( !sSqlite_stmt ) {
        _errno = SQLITE_MISUSE;
        _err_msg.cutAddString(0, "resultOpen() was not called");
        return false;
    }

    for(idx itry = 0; !_errno && itry < max_deadlock_retries; itry++) {
        int code = sqlite3_step(sSqlite_stmt);
        switch(code) {
            case SQLITE_ROW:
                _have_result_row = true;
                return true;
            case SQLITE_DONE:
                return false;
            case SQLITE_BUSY:
                if( inTransaction() ) {
                    _had_deadlocked = true;
                    _errno = code;
                    break;
                } else {
                    continue;
                }
            default:
                _errno = code;
                break;
        }
    }

    sStr saved_err_msg;
    udx saved_errno = _errno;
    saved_err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
    resultClose();
    restoreError(saved_errno, &saved_err_msg);
    return false;
}

const char * sSqlite::resultColName(idx icol) const
{
    if( sSqlite_stmt && icol >= 0 && icol < _res_ncols ) {
        return _res_colnames.ptr(_res_colname_pos[icol]);
    }
    return 0;
}

const char * sSqlite::resultValue(idx icol, const char * defval, idx * plen)
{
    const char * val = 0;
    idx len = 0;
    if( sSqlite_stmt && _have_result_row && icol >= 0 && icol < _res_ncols ) {
        val = (const char *)sqlite3_column_text(sSqlite_stmt, icol);
        len = val ? sqlite3_column_bytes(sSqlite_stmt, icol) : 0;
    }
    if( !len || !val ) {
        val = defval;
        len = sLen(defval);
    }
    if( plen ) {
        *plen = len;
    }
    return val;
}

idx sSqlite::resultIValue(idx icol, idx defval)
{
    if( const char * val = resultValue(icol) ) {
        return atoidx(val);
    } else {
        return defval;
    }
}

udx sSqlite::resultUValue(idx icol, udx defval)
{
    if( const char * val = resultValue(icol) ) {
        return atoudx(val);
    } else {
        return defval;
    }
}

real sSqlite::resultRValue(idx icol, real defval)
{
    if( const char * val = resultValue(icol) ) {
        return strtod(val, 0);
    } else {
        return defval;
    }
}

void sSqlite::resultClose()
{
    sqlite3_finalize(sSqlite_stmt);
    _stmt = 0;
    _have_result_row = false;
    _res_ncols = 0;
    clearError();
}

bool sSqlite::startTransaction()
{
    resultClose();

    if( _in_transaction > 0 ) {
        _in_transaction++;
        return true;
    } else {
        char * errmsg = 0;
        _errno = sqlite3_exec(sSqlite_db, "BEGIN TRANSACTION;", 0, 0, &errmsg);
        if( errmsg ) {
            _err_msg.cutAddString(0, errmsg);
            sqlite3_free(errmsg);
            return false;
        } else if( _errno != SQLITE_OK ) {
            _err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
            return false;
        } else {
            _in_transaction = 1;
        }
    }
    return true;
}

bool sSqlite::commit()
{
    resultClose();

    if( _in_transaction == 0 ) {
        clearError();
        return true;
    } else if( --_in_transaction == 0 ) {
        char * errmsg = 0;
        _errno = sqlite3_exec(sSqlite_db, "COMMIT;", 0, 0, &errmsg);
        if( errmsg ) {
            _err_msg.cutAddString(0, errmsg);
            sqlite3_free(errmsg);
            return false;
        } else if( _errno != SQLITE_OK ) {
            _err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
            return false;
        }
    }
    return true;
}

bool sSqlite::rollback()
{
    resultClose();

    if( _in_transaction == 0 ) {
        clearError();
        return true;
    } else if( --_in_transaction == 0 ) {
        char * errmsg = 0;
        _errno = sqlite3_exec(sSqlite_db, "ROLLBACK;", 0, 0, &errmsg);
        if( errmsg ) {
            _err_msg.cutAddString(0, errmsg);
            sqlite3_free(errmsg);
            return false;
        } else if( _errno != SQLITE_OK ) {
            _err_msg.cutAddString(0, sqlite3_errmsg(sSqlite_db));
            return false;
        }
    }
    return true;
}

void sSqlite::clearError()
{
    _errno = 0;
    _err_msg.cut0cut();
}

udx sSqlite::saveError(sStr * out_err_msg) const
{
    if( _err_msg.length() ) {
        out_err_msg->addString(_err_msg.ptr(), _err_msg.length());
    }
    return _errno;
}

void sSqlite::restoreError(udx saved_errno, const sStr * saved_err_msg)
{
    _errno = saved_errno;
    if( saved_err_msg->length() ) {
        _err_msg.addString(saved_err_msg->ptr(), saved_err_msg->length());
    } else {
        _err_msg.cut0cut();
    }
}

idx sSqlite::getLastInsertRowid()
{
    return sqlite3_last_insert_rowid(sSqlite_db);
}

char * sSqlite::protectValue(sStr & to, const char* from, udx length)
{
    idx pos = to.length();

    if( !length ) {
        length = sLen(from);
    }

    to.addString("'", 1);
    if( length ) {
        sString::searchAndReplaceStrings(&to, from, length, "'" __, "''" __, 0, false);
        to.shrink00();
    }
    to.addString("'", 1);
    return to.ptr(pos);
}
