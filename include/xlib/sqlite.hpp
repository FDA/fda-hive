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
    class sSqlite {
        public:
            sSqlite();
            sSqlite(const char * filepath, bool readonly = false);
            ~sSqlite();

            bool reset(const char * filepath, bool readonly = false);

            bool ok() const { return _db; }
            bool execute(const char * sqlfmt, ...) __attribute__((format(printf, 2, 3)));
            bool executeExact(const char * sql);

            idx getTable(sVarSet & out_tbl, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            idx getTableExact(sVarSet & out_tbl, const char * sql);

            const char * getValue(sStr & out, const char * defval, const char * sqlfmt, ...) __attribute__((format(printf, 4, 5)));
            const char * getValueExact(sStr & out, const char * sql, const char * defval = 0);
            idx getIValue(idx defval, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            idx getIValueExact(const char * sql, idx defval = 0);
            udx getUValue(udx defval, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            udx getUValueExact(const char * sql, udx defval = 0);
            real getRValue(real defval, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            real getRValueExact(const char * sql, real defval = 0);

            bool resultOpen(const char * sqlfmt, ...) __attribute__((format(printf, 2, 3)));
            bool resultOpenExact(const char * sql);
            bool resultNextRow(void);
            idx resultColDim() const { return _res_ncols; }
            const char * resultColName(idx icol) const;
            const char * resultValue(idx icol, const char * defval = 0, idx * plen = 0);
            idx resultIValue(idx icol, idx defval = 0);
            udx resultUValue(idx icol, udx defval = 0);
            real resultRValue(idx icol, real defval = 0);
            void resultClose(void);

            bool startTransaction();
            bool commit();
            bool rollback();
            udx inTransaction() const { return _in_transaction; }
            bool hadDeadlocked() const { return _had_deadlocked; }
            static const idx max_deadlock_retries;

            bool hasFailed() const { return _errno || _err_msg.length(); }
            const char * getError() const { return _err_msg.length() ? _err_msg.ptr() : 0; }
            udx getErrno() const { return _errno; }

            idx getLastInsertRowid();

            static char * protectValue(sStr & to, const char* from, udx length = 0);

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
            udx saveError(sStr * out_err_msg) const;
            void restoreError(udx saved_errno, const sStr * err_msg);
            void clearAll();
    };
};

#endif
