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
#ifndef xLib_sqlite_hpp
#define xLib_sqlite_hpp

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <slib/std/varset.hpp>

namespace slib {
    //! sSql-like convenience wrapper for SQLite 3
    class sSqlite {
        public:
            sSqlite();
            /*! \param readonly if true, opens file in readonly mode;
                       if false, opens file in read-write mode (and creates the file if it does not exist) */
            sSqlite(const char * filepath, bool readonly = false);
            ~sSqlite();

            //! open a database file (and close the currently opened one, if any)
            /*! \param readonly if true, opens file in readonly mode;
                       if false, opens file in read-write mode (and creates the file if it does not exist) */
            bool reset(const char * filepath, bool readonly = false);

            //! check if the database has been successfully opened
            bool ok() const { return _db; }
            //! execute a sql statement without retrieving its result
            /*! \returns true if statement executed successfully */
            bool execute(const char * sqlfmt, ...) __attribute__((format(printf, 2, 3)));
            //! execute a sql statement without retrieving its result
            /*! \returns true if statement executed successfully */
            bool executeExact(const char * sql);

            //! execute sql statement and retrieve its result into a varset table
            /*! \param[out] out_tbl table to which results will be appended;
                            column names will be set if out_tbl was initially empty
                \returns number of rows retrieved (and appended to out_tbl) */
            idx getTable(sVarSet & out_tbl, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            //! execute sql statement and retrieve its result into a varset table
            /*! \param[out] out_tbl table to which results will be appended;
                            column names will be set if out_tbl was initially empty
                \returns number of rows retrieved (and appended to out_tbl) */
            idx getTableExact(sVarSet & out_tbl, const char * sql);

            //! prepare a sql statement for retrieving results row by row, using resultValue() and related methods
            /*! \returns true if the sql statement compiled successfully */
            bool resultOpen(const char * sqlfmt, ...) __attribute__((format(printf, 2, 3)));
            //! prepare a sql statement for retrieving results row by row, using resultValue() and related methods
            /*! \returns true if the sql statement compiled successfully */
            bool resultOpenExact(const char * sql);
            //! retrieve a row (including the first row!) from a statement prepared by resultOpen()
            /*! \returns true if row was retrieved, false if there are no more rows to retrieve or if there was an error */
            bool resultNextRow(void);
            //! number of columns in the result output of a prepared sql statement
            /*! This method may be called any time between resultOpen() and resultClose() */
            idx resultColDim() const { return _res_ncols; }
            //! name of a column in the result output of a prepared sql statement
            /*! This method may be called any time between resultOpen() and resultClose() */
            const char * resultColName(idx icol) const;
            //! string value of a column in a row that was loaded by resultNextRow()
            /*! \param defval default return value if the column contains no data or on error
                \param[out] plen if non-0, retrieves the length of the string value
                \returns string value of the column, or defval on error
                \warning the returned pointer is invalidated by *any* subsequent result*() call */
            const char * resultValue(idx icol, const char * defval = 0, idx * plen = 0);
            //! integer value of a column in a row that was loaded by resultNextRow()
            /*! \param defval default return value if the column contains no data or on error */
            idx resultIValue(idx icol, idx defval = 0);
            //! unsigned value of a column in a row that was loaded by resultNextRow()
            /*! \param defval default return value if the column contains no data or on error */
            udx resultUValue(idx icol, udx defval = 0);
            //! real value of a column in a row that was loaded by resultNextRow()
            /*! \param defval default return value if the column contains no data or on error */
            real resultRValue(idx icol, real defval = 0);
            //! close the statement which was prepared by resultOpen()
            void resultClose(void);

            //! start a transaction
            /*! \note SQLite does not support nested transactions; calling this method twice increments
                an emulated transaction level counter, but the inner transaction will not roll back
                or commit independently of the outer.
                \returns true on success, false on error */
            bool startTransaction();
            //! commit a transaction
            /*! \note SQLite does not support nested transactions; if startTransaction() was called twice,
                calling commit() once will decrement an emulated transaction counter, but won't really
                commit the inner transaction.
                \returns true on success, false on error */
            bool commit();
            //! roll back a transaction
            /*! \note SQLite does not support nested transactions; if startTransaction() was called twice,
                calling rollback() once will decrement an emulated transaction counter, but won't really
                roll back the inner transaction.
                \returns true on success, false on error */
            bool rollback();
            //! current transaction level counter (0 if not in a transaction, 1 in the outermost transaction)
            udx inTransaction() const { return _in_transaction; }
            //! check if the database encountered a deadlock
            /*! Possible only if two processes opened the same database file in non-readonly mode */
            bool hadDeadlocked() const { return _had_deadlocked; }
            //! number of times to retry an operation if the database deadlocked; 16 by default
            static const idx max_deadlock_retries; // = 16;

            //! return true if an error occurred
            bool hasFailed() const { return _errno || _err_msg.length(); }
            //! return most recent sqlite error string
            const char * getError() const { return _err_msg.length() ? _err_msg.ptr() : 0; }
            //! return most recent sqlite error code, or 0 meaning success
            udx getErrno() const { return _errno; }

            idx getLastInsertRowid();

        private:
            void * _db;
            void * _stmt;
            bool _stmt_done;
            udx _in_transaction;
            udx _errno;
            sStr _err_msg;
            bool _had_deadlocked;
            bool _have_result_row;

            sStr _res_colnames;
            sVec<idx> _res_colname_pos;
            idx _res_ncols;

            void clearError();
            void clearAll();
    };
};

#endif
