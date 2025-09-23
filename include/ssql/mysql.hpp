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
#ifndef sLib_mysql_h
#define sLib_mysql_h

#include <slib/core/vec.hpp>
#include <slib/core/str.hpp>
#include <slib/std/varset.hpp>

namespace slib
{
    class sSql
    {
        public:
            struct gSettings {
                char db[128];
                char server[128];
                char user[128];
                char pass[128];
                idx permanent;
                idx pageSize;
                char rndTbl[128];
                idx debug;
                udx rwTimeout;
            };
            static gSettings gSet;
            enum eStatus { eDisconnected = 0, eConnected, eFailure };
            idx status;
            bool permanentConnection;

        public:
            sSql(const char * db = 0, const char * serverlist00 = 0, const char * user = 0, const char * pass = 0)
                : m_res(0), m_res_row(0), m_res_lengths(0), m_lib_init(0)
            {
                permanentConnection = true;
                isRawOut = false;
                separSymb = ",";
                endlSymb = "\n//";
                nullSymb = "-";
                rawFltOut = ", \t\r\n";
                srchCaseSensitive = false;
                rwTimeout = 120;
                m_mysql_errno = 0;
                m_in_transaction = 0;
                m_had_deadlocked = false;
                init(db, serverlist00, user, pass);
            }
            sSql* init(const char* db = 0, const char* serverlist00 = 0, const char* user = 0, const char* pass = 0)
            {
                dbConn = 0;
                dbConnDel = 0;
                status = eDisconnected;
                if( db ) {
                    connect(db, serverlist00, user, pass, rwTimeout);
                    if( status == eConnected ) {
                        return 0;
                    }
                }
                return this;
            }
            sSql* init(sSql* reuse)
            {
                dbConn = reuse->dbConn;
                dbConnDel = 0;
                return this;
            }
            ~sSql()
            {
                disconnect();
            }

        public:
            idx realConnect(void);

            idx connect(const char * db, const char * serverlist00, const char * user, const char * pass, const udx rwtmout);
            idx connect(const char * filenm, const char * section) ;
            idx connect(const char * defline) ;

            void disconnect(void);

            const sStr& GetError() const
            { return errB; }
            udx Get_errno() const
            { return m_mysql_errno; }
            bool HasFailed() const
            { return m_mysql_errno != 0; }
            bool hadDeadlocked() const
            { return m_had_deadlocked; }
            const sStr& Get_error() const
            { return m_mysql_error; }

            static const idx max_deadlock_retries;
            static const idx max_deadlock_wait_usec;
            void pretendDeadlock();

        public:

            bool execute(const char * sqlfmt, ... ) __attribute__((format(printf, 2, 3)));
            bool executeString(const char * sql);

            bool start_transaction(void)
            {
                if( m_in_transaction > 0 ) {
                    m_in_transaction++;
                    return true;
                } else if( executeString("START TRANSACTION; SELECT TRUE;") ) {
                    m_in_transaction = 1;
                    return true;
                }
                return false;
            }
            bool commit(void)
            {
                if( m_in_transaction == 0 ) {
                    return true;
                } else if( --m_in_transaction == 0 ) {
                    return executeString("COMMIT; SELECT TRUE;");
                }
                return true;
            }
            bool rollback(void)
            {
                if( m_in_transaction == 0 ) {
                    return true;
                } else if( --m_in_transaction == 0 ) {
                    return executeString("ROLLBACK; SELECT TRUE;");
                }
                return true;
            }
            udx in_transaction() const
            {
                return m_in_transaction;
            }

            class sqlProc {
                private:
                    sqlProc& operator=( const sqlProc& );

                public:
                    sqlProc(sSql& sql, const char* name) : m_sql(sql), m_qty(0)
                        { m_stmt.printf(0, "CALL %s(", name); }
                    ~sqlProc()
                        {}

                    sqlProc& Add(const idx value)
                        { m_stmt.printf("%s%" DEC, m_qty++ ? ", ": "", value); return *this; }
                    sqlProc& Add(const udx value)
                        { m_stmt.printf("%s%" UDEC, m_qty++ ? ", ": "", value); return *this; }
                    sqlProc& Add(const real value, const char * fmt = "%g")
                        { if( m_qty++ ) { m_stmt.addString(", "); } m_stmt.printf(fmt, value); return *this; }
                    sqlProc& Add(const bool value)
                        { m_stmt.printf("%s%s", m_qty++ ? ", ": "", value ? "TRUE" : "FALSE"); return *this; }
                    sqlProc& Add(const sStr& value)
                        { return Add(value.ptr()); }
                    sqlProc& Add(const char* value, udx value_len = 0)
                        {
                            if( m_qty++ ) {
                                m_stmt.addString(", ");
                            }
                            if( value ) {
                                m_sql.protectValue(m_stmt, value, value_len);
                            } else {
                                m_stmt.addString("NULL");
                            }
                            return *this;
                        }

                    const char* statement(void)
                        { return close(); }
                    const char * rawStatement(void) const
                        { return m_stmt.ptr(); }
                    bool execute()
                        { return m_sql.executeString(close()); }
                    idx getTable(sVarSet * ofs, sMex * blb = 0)
                        { blb=0;return m_sql.getTable(close(), ofs, blb); }
                    idx getBlob(sMex* blb)
                        { return m_sql.getBlob(blb, "%s", close()); }
                    bool resultOpen(void)
                        { return m_sql.exec(close()); }
                    idx ivalue(idx defval)
                        { return m_sql.ivalue(close(), defval); }
                    udx uvalue(udx defval)
                        { return m_sql.uvalue(close(), defval); }
                    real rvalue(real defval)
                        { return m_sql.rvalue(close(), defval); }
                    char* svalue(sStr& out)
                        { idx pos = out.length(); m_sql.getTable(close(), 0, &out); out.add0cut(); return out.ptr(pos); }
                    time_t time_value(time_t defval)
                        { return m_sql.time_value(close(), defval); }

                protected:
                    const char* close()
                        { m_stmt.printf(")"); return m_stmt.ptr(); }

                    sSql& m_sql;
                    sStr m_stmt;
                    idx m_qty;

            };

            sqlProc* Proc(const char* name)
                { return new sqlProc(*this, name); }

            idx getTable( const char * sql, sVarSet * ofs=0, sMex * blb=0);
            idx getTable( sVarSet  * ofs,const char * sqlfmt, ... ) __attribute__((format(printf, 3, 4)))
            {
                sStr tmp;
                sCallVarg(tmp.vprintf,sqlfmt);
                return getTable(tmp.ptr(),ofs);
            }
            idx getBlob( sMex * blb, const char * sqlfmt, ... ) __attribute__((format(printf, 3, 4)))
            {
                sStr tmp;
                sCallVarg(tmp.vprintf,sqlfmt);
                return getTable(tmp.ptr(),0,blb);
            }

            bool resultOpen(const char * sqlfmt, ...) __attribute__((format(printf, 2, 3)));
            void resultClose(void);
            bool resultNext(void);
            idx resultColDim() const
            {
                return m_res_ids.dim();
            }
            const char * resultColName(idx col) const;
            idx resultColId(const char * const name);
            bool resultNextRow();
            const char * resultValue(idx col, const char * defval = 0, idx * out_len = 0);
            idx resultIValue(idx col, idx defval = 0)
            {
                const char * v = resultValue(col);
                if( v ) {
                    sscanf(v, "%" DEC, &defval);
                }
                return defval;
            }
            udx resultUValue(idx col, udx defval = 0)
            {
                const char * v = resultValue(col);
                if( v ) {
                    sscanf(v, "%" UDEC, &defval);
                }
                return defval;
            }
            real resultRVvalue(idx col, real defval = 0)
            {
                const char * v = resultValue(col);
                if( v ) {
                    sscanf(v, "%lf", &defval);
                }
                return defval;
            }

            idx sscanfTable( const char * sql, const char * fmt00, void * pVal0, idx maxcnt, int sizeofval=0);
            template < typename Tobj > idx sscanfTable( const char * sql, const char * fmt00, Tobj * pVal0, idx maxcnt=0){
                return sscanfTable( sql, fmt00, (void *) pVal0, maxcnt, sizeof(Tobj));
            }
            idx ivalue(idx defval, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            idx ivalue(const char * sql, idx defval);
            udx uvalue(udx defval, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            udx uvalue(const char * sql, udx defval);
            real rvalue(real defval, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            real rvalue(const char * sql, real defval);
            char * svalue(sStr & out, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            time_t time_value(time_t defval, const char * sqlfmt, ...) __attribute__((format(printf, 3, 4)));
            time_t time_value(const char * sql, time_t defval);
            char * protect(sStr& to, const char* from, udx length = 0);
            char * protectValue(sStr& to, const char* from, udx length = 0);
            char * protectName(sStr& to, const char* from, udx length = 0);
            char * protectSubstringLike(sStr& to, const char* from, udx length = 0);
            char * protectBlob(sStr& to, const void * from, udx length);
            static char * exprInList(sStr& to, const char* domid_expr, const char* objid_expr, const sHiveId * list, idx cnt, bool negate = false, bool empty_list_is_true = false);
            static char * exprInList(sStr& to, const char* domid_expr, const char* objid_expr, sVec<const sHiveId> & list, bool negate = false, bool empty_list_is_true = false)
            {
                return exprInList(to, domid_expr, objid_expr, list.ptr(), list.dim(), negate, empty_list_is_true);
            }
            static char * exprInList(sStr& to, const char* domid_expr, const char* objid_expr, sVec<sHiveId> & list, bool negate = false, bool empty_list_is_true = false)
            {
                return exprInList(to, domid_expr, objid_expr, list.ptr(), list.dim(), negate, empty_list_is_true);
            }
            static char * exprInList(sStr& to, const char* lhs_expr, const idx * list, idx cnt, bool negate = false, bool empty_list_is_true = false);
            static char * exprInList(sStr& to, const char* lhs_expr, const sVec<idx> & list, bool negate = false, bool empty_list_is_true = false)
            {
                return exprInList(to, lhs_expr, list.ptr(), list.dim(), negate, empty_list_is_true);
            }
            static char * exprInList(sStr& to, const char* lhs_expr, const int * list, idx cnt, bool negate = false, bool empty_list_is_true = false);
            static char * exprInList(sStr& to, const char* lhs_expr, const udx * list, idx cnt, bool negate = false, bool empty_list_is_true = false);
            static char * exprInList(sStr& to, const char* lhs_expr, const sVec<udx> & list, bool negate = false, bool empty_list_is_true = false)
            {
                return exprInList(to, lhs_expr, list.ptr(), list.dim(), negate, empty_list_is_true);
            }



    public:
        const char * separSymb, *endlSymb, *nullSymb, * rawFltOut;
        bool srchCaseSensitive, isRawOut;
        void outTblValues(sStr * str,sVarSet * valtbl, const char * fields00=0, const char * srch00=0, const char * srchflds00=0, idx maxfoundout=0);

        idx outTblSql(sStr * str,const char * tbl, const char * fields=0, const char * whereclause=0);
        idx updateTblSql(const char * tblnm, sVar * form, const char * fields00, const char * autoQuotes , const char * whereclause);
        idx parseFromCSV(const char * tblnm, const char * collist, const char * collistquote, const char * rowprfx,const char * tbl, idx tblen=0);


        idx joinFromMultipleTables(sVarSet  * pidlist, const char * sqllist00,const char * colnamelst00, idx recCol=0);
        idx outFromMultipleTables(sStr * out, const char * sqllist00,const char * colnamelst00, idx recCol=0, const char * srch=0,idx maxCount=0, idx pgStart=0);
    public:
        class Schema {
            sStr dat;
            struct Table {
                idx fields, fields00, quotes;
                bool isUpd;
            };
            sDic < Table > tbl;

            public:
            const char * idCol, * idTbl;
            Schema(const char * lidtbl, const char * lidcol){idTbl=lidtbl; idCol=lidcol;}

            void load( sSql * sql, const char * tables00, const char * exclColumns00=0);
            idx dim(){return tbl.dim();}
            const char * tblname(idx inum){return (const char *)tbl.id(inum);}
            const char * fields(idx inum){return dat.ptr(tbl[inum].fields);}
            const char * fields00(idx inum){return dat.ptr(tbl[inum].fields00);}
            const char * quotes(idx inum){return dat.ptr(tbl[inum].quotes);}
            bool isUpdateable(idx inum){return tbl[inum].isUpd;}

            void add(const char * tbln, const char * fldlist, const char * quotes,bool isupdateable=true);

            char * searchSql(sStr * srchSql, const char * srch,const char * anyExclusions00=0, sStr * participants=0);
            char * searchSqlRecomb(sStr * srchSql, const char * srch,const char * anyExclusions00=0, sStr * participants=0);
            char * dumpSql(sStr * dmp, const char * fltr, const char * fldlst);
            char * getAllFields( sStr * dmp);

            idx ensureIDRow(sSql * ligFam, idx recID,const char * tblnm);
            void updateTbl(sSql * ligFam, idx recID, sVar * pForm, const char * whereclasue);

        };
    private:

        static sString::SectVar gSetVars[];

        void * dbConn, *dbConnDel;
        sStr errB;
        sStr dataB;
        sStr sqlB;
        idx ofsServerList, ofsDB, ofsUser, ofsPasswd;
        udx rwTimeout;
        udx m_mysql_errno;
        sStr m_mysql_error;
        udx m_in_transaction;
        bool m_had_deadlocked;

        void * m_res;
        void * m_res_row;
        void * m_res_lengths;
        sDic<idx> m_res_ids;

        bool exec(const char * sql);
        bool ihasError(const char * func, unsigned int line);

        idx m_lib_init;
    };

};

#endif 