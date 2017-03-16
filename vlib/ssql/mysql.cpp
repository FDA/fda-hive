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
// TODO be able to read configs for connect from another section : not just QPride

#define my_socket_defined
typedef int my_socket; /* File descriptor for sockets */

#include <limits.h> // for INT_MIN / INT_MAX
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>
#define conDB   ((MYSQL *)dbConn)


#include <slib/core/perf.hpp>
#include <slib/core/tim.hpp>
#include <slib/std/string.hpp>
#include <slib/std/file.hpp>
#include <slib/std/http.hpp>
#include <slib/utils/sort.hpp>
#include <ssql/mysql.hpp>

using namespace slib;

#if _DEBUG_off
#include <slib/core/tim.hpp>
static sTime slib_mysqldbg_wall_clock;
#define MYSQLDBG(fmt,...) ::fprintf(stderr, "%s:%u (%.2g s) : "fmt"\n", __FILE__, __LINE__, slib_mysqldbg_wall_clock.clock(0, 0, true), __VA_ARGS__)
#else
#define MYSQLDBG(...)
#endif

char * sSql::protect(sStr & to, const char* from, udx length)
{
    idx pos = to.length();

    if( !length ) {
        length = sLen(from);
    }

    if( !from ) {
        to.add0();
    } else if( status != eConnected ) {
        sString::searchAndReplaceStrings(&to, from, length, "'"__, "''"__, 0, false);
    } else {
        to.add(0, 4 * length);
        to.cut(pos + mysql_real_escape_string(conDB, to.ptr(pos), from, (unsigned long) length));
        to.add0();
    }
    return to.ptr(pos);
}

char * sSql::protectValue(sStr & to, const char* from, udx length)
{
    idx pos = to.length();

    to.addString("'", 1);
    protect(to, from, length);
    to.shrink00();
    to.addString("'", 1);
    return to.ptr(pos);
}

char * sSql::protectName(sStr & to, const char * from, udx length)
{
    idx pos = to.length();
    if( !length ) {
        length = sLen(from);
    }
    to.addString("`", 1);
    sString::searchAndReplaceStrings(&to, from, length, "`"__, "``"__, 0, false);
    to.shrink00();
    to.addString("`", 1);
    return to.ptr(pos);
}

char * sSql::protectSubstringLike(sStr& to, const char* from, udx length/* = 0*/)
{
    idx pos = to.length();
    if( !length ) {
        length = sLen(from);
    }
    to.addString("'%", 2);
    if( length && from[0] ) {
        for(udx i = 0; i < length; i++) {
            char c = from[i];
            switch(c) {
                case 0:
                    break;
                case '\'':
                    to.addString("''", 2);
                    break;
                case '\\':
                    to.addString("\\\\", 2);
                    break;
                case '%':
                    to.addString("\\%", 2);
                    break;
                case '_':
                    to.addString("\\_", 2);
                    break;
                default:
                    to.addString(&c, 1);
            }
        }
        to.addString("%", 1);
    }
    to.addString("'", 1);
    return to.ptr(pos);
}

char * sSql::protectBlob(sStr& to, const void * from, udx length)
{
    idx pos = to.length();
    to.addString("X'");
    idx hex_pos = to.length();
    if( length ) {
        to.add(0, length * 2 + 1); // extra byte required for snprintf's final 0
    }
    for(udx i = 0; i < length; i++) {
        unsigned int from_byte = ((unsigned char *)from)[i];
        snprintf(to.ptr(hex_pos + i * 2), 3, "%02X", from_byte);
    }
    to.cutAddString(hex_pos + length * 2, "'");
    return to.ptr(pos);
}

#define MIN_DENSE_RANGE 4
template<class T, class T_list, T(T_retr)(const T_list *, idx), T T_min, T T_max>
static char * static_exprInList(sStr & to, const char * lhs, const T_list * list, idx * ind, idx istart, idx ilast, bool negate, bool empty_list_is_true)
{
    idx pos = to.length();
    if( ilast > istart ) {
        to.addString("(", 1); // reserve space for outer parentheses

        idx istart_sparse = -1, iend_sparse = -1;
        idx istart_cur = istart, num_dense_spans = 0;
        bool need_outer_paren = false;

        // First pass: identify spans of at least MIN_DENSE_RANGE densely packged values.
        // Print them out as range expressions, mark their indices as negative to avoid
        // reusing in second pass.
        while( istart_cur < ilast ) {
            idx istart_dense = istart_cur, iend_dense = istart_cur;
            while( istart_dense < ilast ) {
                iend_dense = istart_dense;
                while( iend_dense + 1 < ilast && T_retr(list, ind[iend_dense + 1]) - T_retr(list, ind[iend_dense]) <= 1 ) {
                    iend_dense++;
                }
                if( T_retr(list, ind[iend_dense]) - T_retr(list, ind[istart_dense]) >= MIN_DENSE_RANGE ) {
                    // Found densely packed range
                    need_outer_paren = true;
                    bool need_inner_paren = num_dense_spans > 0 || iend_dense + 1 < ilast;
                    if( need_inner_paren ) {
                        if( num_dense_spans > 0 ) {
                            to.addString(negate ? " AND " : " OR ");
                        }
                        to.addString("(");
                    }

                    T min_val = T_retr(list, ind[istart_dense]);
                    T max_val = T_retr(list, ind[iend_dense]);

                    if( negate ) {
                        to.printf("%s < ", lhs);
                        to.addNum(min_val);
                        to.printf(" OR %s > ", lhs);
                        to.addNum(max_val);
                    } else {
                        // transform ">="/"<=" to ">"/"<" to save a byte, except when value is outside T's value range
                        if( min_val > T_min ) {
                            to.printf("%s > ", lhs);
                            to.addNum(min_val - 1);
                        } else {
                            to.printf("%s >= ", lhs);
                            to.addNum(min_val);
                        }

                        if( max_val < T_max ) {
                            to.printf(" AND %s < ", lhs);
                            to.addNum(max_val + 1);
                        } else {
                            to.printf(" AND %s <= ", lhs);
                            to.addNum(max_val);
                        }
                    }
                    if( need_inner_paren ) {
                        to.addString(")");
                    }

                    // Mark printed indices to avoid reuse
                    for(idx i = istart_dense; i <= iend_dense; i++) {
                        ind[i] = -sIdxMax;
                    }

                    num_dense_spans++;
                    break;
                } else {
                    // Found sparse range
                    if( istart_sparse < 0 ) {
                        istart_sparse = istart_cur;
                    }
                    iend_sparse = iend_dense;
                    istart_dense = iend_dense + 1;
                }
            }
            istart_cur = iend_dense + 1;
        }

        // Second pass: print sparse values (i.e. those whose indices were not marked as negative)
        if( istart_sparse >= 0 ) {
            if( iend_sparse < 0 ) {
                iend_sparse = ilast - 1;
            }
            if( num_dense_spans > 0 ) {
                need_outer_paren = true;
                to.addString(negate ? " AND " : " OR ");
            }
            if( T_retr(list, ind[istart_sparse]) == T_retr(list, ind[iend_sparse]) ) {
                // exactly one value in sparse span - equality expression
                to.printf("%s %s ", lhs, negate ? "!=" : "=");
                to.addNum(T_retr(list, ind[istart_sparse]));
            } else {
                // multiple values in sparse span - IN (...)
                to.printf("%s %s (", lhs, negate ? "NOT IN" : "IN");
                T cur_val = 0, prev_val = 0;
                for(idx i = istart_sparse; i <= iend_sparse; i++, prev_val = cur_val) {
                    if( ind[i] < 0 ) {
                        continue;
                    }
                    cur_val = T_retr(list, ind[i]);
                    if( i == istart_sparse || cur_val != prev_val ) {
                        if( i > istart_sparse ) {
                            to.addString(", ");
                        }
                        to.addNum(cur_val);
                    }
                }
                to.addString(")");
            }
        }

        if( need_outer_paren ) {
            to.addString(")");
        } else {
            idx len_no_paren = to.length() - pos - 1;
            memmove(to.ptr(pos), to.ptr(pos + 1), len_no_paren); // including terminal 0 not counted in length()
            to.cut0cut(pos + len_no_paren);
        }
    } else if( empty_list_is_true ) {
        to.addString(negate ? "FALSE" : "TRUE");
    } else {
        to.addString(negate ? "TRUE" : "FALSE");
    }
    return to.ptr(pos);
}

namespace {
    template<class T>
    inline T static_exprInList_retr(const T * list, idx i)
    {
        return list[i];
    }

    inline udx static_exprInList_objid_retr(const sHiveId * list, idx i)
    {
        return list[i].objId();
    }
};

// static
char * sSql::exprInList(sStr & to, const char * lhs, const idx * list, idx cnt, bool negate, bool empty_list_is_true/* = false*/)
{
    sVec<idx> ind(sMex::fExactSize);
    if( cnt ) {
        ind.resize(cnt);
        sSort::sort<const idx, idx>(cnt, list, ind.ptr());
    }
    return static_exprInList<idx, idx, static_exprInList_retr<idx>, -sIdxMax, sIdxMax>(to, lhs, list, ind.ptr(), 0, cnt, negate, empty_list_is_true);
}

// static
char * sSql::exprInList(sStr & to, const char * lhs, const int * list, idx cnt, bool negate, bool empty_list_is_true/* = false*/)
{
    sVec<idx> ind(sMex::fExactSize);
    if( cnt ) {
        ind.resize(cnt);
        sSort::sort<const int, idx>(cnt, list, ind.ptr());
    }
    return static_exprInList<int, int, static_exprInList_retr<int>, INT_MIN, INT_MAX>(to, lhs, list, ind, 0, cnt, negate, empty_list_is_true);
}

// static
char * sSql::exprInList(sStr & to, const char * lhs, const udx * list, idx cnt, bool negate, bool empty_list_is_true/* = false*/)
{
    sVec<idx> ind(sMex::fExactSize);
    if( cnt ) {
        ind.resize(cnt);
        sSort::sort<const udx, idx>(cnt, list, ind.ptr());
    }
    return static_exprInList<udx, udx, static_exprInList_retr<udx>, 0, sUdxMax>(to, lhs, list, ind, 0, cnt, negate, empty_list_is_true);
}

//static
char * sSql::exprInList(sStr& to, const char* domid_expr, const char* objid_expr, const sHiveId * list, idx cnt, bool negate/* = false*/, bool empty_list_is_true/* = false*/)
{
    if( domid_expr ) {
        idx pos = to.length();
        sVec<idx> ind(sMex::fExactSize);
        if( cnt ) {
            bool outer_paren = false;
            ind.resize(cnt);
            sSort::sort<const sHiveId, idx>(cnt, list, ind.ptr());
            for(idx i = 0; i < cnt; ) {
                udx domain_id = list[ind[i]].domainId();
                idx next_i = cnt; // by default, stop loop after this iteration
                for(idx j = i + 1; j < cnt; j++) {
                    if( list[ind[j]].domainId() != domain_id ) {
                        next_i = j;
                        break;
                    }
                }
                if( i == 0 && next_i < cnt ) {
                    to.addString("(");
                    outer_paren = true;
                }
                if( domain_id ) {
                    if( negate ) {
                        to.printf("%s(%s != %"UDEC" OR %s IS NULL OR ", i ? " AND " : "", domid_expr, domain_id, domid_expr);
                    } else {
                        to.printf("%s(%s = %"UDEC" AND ", i ? " OR " : "", domid_expr, domain_id);
                    }
                } else {
                    if( negate ) {
                        to.printf("%s((%s != 0 AND %s IS NOT NULL) OR ", i ? " AND " : "", domid_expr, domid_expr);
                    } else {
                        to.printf("%s((%s = 0 OR %s IS NULL) AND ", i ? " OR " : "", domid_expr, domid_expr);
                    }
                }
                static_exprInList<udx, sHiveId, static_exprInList_objid_retr, 0, sUdxMax>(to, objid_expr, list, ind, i, next_i, negate, false);
                to.addString(")");
                i = next_i;
            }
            if( outer_paren ) {
                to.addString(")");
            }
        } else if( empty_list_is_true ) {
            to.addString(negate ? "FALSE" : "TRUE");
        } else {
            to.addString(negate ? "TRUE" : "FALSE");
        }
        return to.ptr(pos);
    } else {
        sVec<udx> obj_ids(sMex::fExactSize);
        obj_ids.resize(cnt);
        for(idx i = 0; i < cnt; i++) {
            obj_ids[i] = list[i].objId();
        }
        return exprInList(to, objid_expr, obj_ids, negate, empty_list_is_true);
    }
}

bool sSql::hasError(void)
{
    if( m_had_deadlocked && m_in_transaction > 0 ) {
        // A deadlock in the middle of a transaction means all subsequent
        // queries until an explicit client-side rollback are unreliable -
        // they may either go into the failed transaction, or be executed in
        // auto-commit mode as new transactions, depending on exactly when an
        // automatic server-side rollback happens. Both alternatives are bad.
        return true;
    }

    m_mysql_errno = mysql_errno(conDB);
    if( m_mysql_errno ) {
        m_mysql_error.printf(0, "%s", mysql_error(conDB));
        errB.printf(0, "Error %"UDEC": %s\n", m_mysql_errno, m_mysql_error.ptr());
        MYSQLDBG("%s", errB.ptr());
    } else {
        errB.cut0cut();
        m_mysql_error.cut0cut();
    }
    return m_mysql_errno != 0;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ construction && connection
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

idx sSql::realConnect(void)
{
    if( !dbConn ) {
        dbConnDel = dbConn = (void*) mysql_init(NULL);
    }
    const char * db = dataB.ptr(ofsDB);
    const char * user = dataB.ptr(ofsUser);
    const char * pass = dataB.ptr(ofsPasswd);
    const char * serverlist = dataB.ptr(ofsServerList);

    sStr svl;
    sString::searchAndReplaceSymbols(&svl, serverlist, 0, ",", 0, 0, true, true, true);
    for(const char * server = svl.ptr(); server && status != eConnected; server = sString::next00(server)) {
        sStr s;
        sString::searchAndReplaceSymbols(&s, server, 0, ":", 0, 0, true, true, true);
        udx port = 0;
        char* p = sString::next00(s.ptr());
        if( p ) {
            sscanf(p, "%"UDEC, &port);
        }
        if( mysql_real_connect(conDB, s.ptr(), user, pass, db, (unsigned int) port, 0, CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS) ) {
            status = eConnected;
            unsigned int timeout = 3;
            mysql_options(conDB, MYSQL_OPT_READ_TIMEOUT, (const void *) &timeout);
            mysql_set_character_set(conDB, "utf8");
        }
        hasError();
    }
    return status;
}

idx sSql::connect(const char * db, const char * serverlist, const char * user, const char * pass) // , bool multistatement
{
    status =eDisconnected;
    //if(multistatement)conDB->set_option(new mysqlpp::MultiStatementsOption(true));
    errB.cut(0);
    if(!serverlist)serverlist="localhost"__;
    ofsDB=dataB.length(); dataB.add(db,sLen(db)+1);
    ofsUser=dataB.length(); dataB.add(user,sLen(user)+1);
    ofsPasswd=dataB.length(); dataB.add(pass,sLen(pass)+1);
    ofsServerList=dataB.length();dataB.add(serverlist,sLen(serverlist)+1);
    dataB.add0(1);
    if(permanentConnection) {
        realConnect();
        if(status != eConnected) {
            if( !errB ) {
                errB.printf(0, "Failure: cannot connect to any of the specified servers\n");
            }
            status = eFailure;
        }
    }
    else { return eConnected;}

    return status;
}

sString::SectVar sSql::gSetVars[]={
    {0,"[QPride]"_"db"__,"%s=sSql_db","%s",&sSql::gSet.db},
    {0,"[QPride]"_"server"__,"%s=localhost","%s",&sSql::gSet.server},
    {0,"[QPride]"_"user"__,"%s=sSql_user","%s",&sSql::gSet.user},
    {0,"[QPride]"_"pass"__,"%s=sSql_password","%s",&sSql::gSet.pass},
    {0,"[QPride]"_"permanent"__,"%n=0^false^true;",0,&sSql::gSet.permanent},
    {0,"[QPride]"_"pageSize"__,"%"DEC"=20","%s",&sSql::gSet.pageSize},
    {0,"[QPride]"_"rndTbl"__,"%s=random","%s",&sSql::gSet.rndTbl},
    {0,"[QPride]"_"debug"__,"%"DEC"=0",0,&sSql::gSet.debug},
    {0}
    };
sSql::gSettings sSql::gSet={"sSql_db","localhost","sSql_user","sSql_password",1,50,"random",0};

idx sSql::connect(const char * filenm, const char *) // , bool multistatement
{
    sFil inp(filenm, sFil::fReadonly);
    /**
     * temporary patch to cover for the problem that every piece of the code is reading
     * cfg file everywhere instead of having common cfg object read it once
     */
    if( !inp.ok() && strpbrk(filenm, "\\/") == 0 ) {
        // lookup in home
        const char * hm = getenv(
#ifdef WIN32
            "USERPROFILE"
#else
            "HOME"
#endif
            );
        if( hm ) {
            sStr home("%s/%s", hm, filenm);
            inp.destroy();
            inp.init(home, sFil::fReadonly);
        }
    }
    if( inp.ok() && inp.length() ) {
        sStr rst;
        sString::cleanMarkup(&rst, inp.ptr(), inp.length(), "//"_"/*"__, "\n"_"*/"__, "\n", 0, false, false, true);
        sString::xscanSect(rst.ptr(), rst.length(), gSetVars);
    }
    return connect(gSet.db, gSet.server, gSet.user, gSet.pass);
}

idx sSql::connect(const char * defline)
{
    char flnm[1024],sect[128];

    if( defline ){
        if ( sscanf(defline,"db=%s server=%s user=%s pass=%s",gSet.db,gSet.server,gSet.user,gSet.pass) ==4 ){
            return connect(gSet.db, gSet.server, gSet.user,gSet.pass);
        }else if(sscanf(defline,"config=%s section=%s",flnm,sect)>=1 ){
            return connect(flnm,sect);
        }
    }
    return connect(HIVE_DB, "localhost", HIVE_DB_USER, HIVE_DB_PWD);
}

void sSql::disconnect(void)
{
    if(dbConnDel){
        mysql_close((MYSQL * )dbConnDel);
        dbConnDel=dbConn=0;
        mysql_library_end();
    }
    status=eDisconnected;
    m_in_transaction = 0;
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ query execution
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

bool sSql::execute(const char * sqlfmt, ...  )
{
    sStr sql;
    sCallVarg(sql.vprintf, sqlfmt)
    return getTable(sql.ptr(),0,0) ? true : false;
}

bool sSql::executeString(const char * sql)
{
    return getTable(sql, 0, 0);
}

const idx sSql::max_deadlock_retries = 16;
const idx sSql::max_deadlock_wait_usec = 100 * 1000;

static bool isMysqlDeadlock(udx err)
{
    return err == ER_LOCK_DEADLOCK || err == ER_XA_RBDEADLOCK || err == ER_LOCK_WAIT_TIMEOUT;
}

// for testing purposes - set sSql object's state as if a deadlock had been detected
void sSql::pretendDeadlock()
{
    MYSQLDBG("Pretending a deadlock has occurred (transaction level %"UDEC")", m_in_transaction);
    m_mysql_errno = ER_LOCK_DEADLOCK;
    m_mysql_error.cutAddString(0, "Deadlock found when trying to get lock; try restarting transaction");
    m_had_deadlocked = true;
}

bool sSql::exec(const char * sql)
{
//PERF_START("SQL-ing");
    // above change '_DEBUG_off' to just '_DEBUG' to see sql and its result in STDERR
    if( !sql ) {
        sql = sqlB.ptr();
    }
    resultClose();
    MYSQLDBG("%s", sql);

    if( m_had_deadlocked && m_in_transaction > 0 ) {
        // A deadlock in the middle of a transaction means all subsequent
        // queries until an explicit client-side rollback are unreliable -
        // they may either go into the failed transaction, or be executed in
        // auto-commit mode as new transactions, depending on exactly when an
        // automatic server-side rollback happens. Both alternatives are bad.
        MYSQLDBG("In deadlocked transaction (level %"UDEC") - refusing query until rollback", m_in_transaction);
        return false;
    }

    // If not in the middle of a transaction and get a deadlock, simply wait a bit
    // and try to resubmit the query.
    // If not in the middle of a transaction and (maybe eventually) succeed,
    // unset the deadlocked flag.
    // If in the middle of a transaction and get a deadlock, break immediately:
    // subsequent queries will either go into the failed transaction (and fail),
    // or go into new auto-committed transaction if automatic server-side rollback
    // happens (causing inconsistent data). So the transaction must be rolled back,
    // but sSql's caller is responsible for handling that logic.
    for(idx i = 0; i < max_deadlock_retries; i++) {
        mysql_query(conDB, sql);
        m_mysql_errno = mysql_errno(conDB);
        if( isMysqlDeadlock(m_mysql_errno) ) {
            MYSQLDBG("Deadlock detected, mysql error code %"UDEC, m_mysql_errno);
            m_had_deadlocked = true;
        } else if( m_in_transaction == 0 ) {
            m_had_deadlocked = false;
        }
        if( !m_had_deadlocked || m_in_transaction > 0 ) {
            break;
        }
        MYSQLDBG("Retrying deadlocked query, attempt %"DEC"/%"DEC, i, max_deadlock_retries);
        sTime::randomSleep(max_deadlock_wait_usec);
    }
    return !hasError();
//PERF_END();
}

idx sSql::getTable(const char * sql, sVarSet * mresult, sMex * blb)
{
    if( !permanentConnection ) {
        realConnect();
    }
    if( status != eConnected ) {
        return 0;
    }
    idx totCnt = 0;
    if( exec(sql) ) {
//PERF_START("SQL-ROWS-READING");
        idx moreRes = 0;
        bool hasColIds = false;
        do {
            MYSQL_RES *result = mysql_store_result(conDB);
            if( result ) {
                idx num_fields = mysql_num_fields(result);
                if( mresult && !hasColIds ) {
                    MYSQL_FIELD* field;
                    idx c = 0;
                    while( (field = mysql_fetch_field(result)) ) {
                        mresult->setColId(c++, field->name);
                    }
                    hasColIds = true;
                }
                MYSQL_ROW row;
                // we need to make sure result set is completely read
                while( (row = mysql_fetch_row(result)) != 0 ) {
                    if( mresult ) {
                        mresult->addRow();
                        for(idx i = 0; i < num_fields; i++) {
                            const char * pp = row[i];
                            bool isnull = (pp && strcmp(pp, "NULL")) ? false : true;
                            mresult->addCol(isnull ? 0 : pp, isnull ? 0 : sLen(pp));
                            ++totCnt;
                        }
                    } else if( blb ) {
                        for(idx i = 0; i < num_fields; i++) {
                            blb->add(row[i], sLen(row[i]));
                            ++totCnt;
                        }
                    } else {
                        ++totCnt;
                    }
                }
                mysql_free_result(result);
            }
            moreRes = mysql_next_result(conDB);
        } while( moreRes == 0 );
        if( !m_mysql_errno && moreRes > 0 ) {
            hasError();
        } else {
            MYSQLDBG("rows: %"DEC" cols: %"DEC" cells: %"DEC"%s", mresult ? mresult->rows : -1, mresult ? mresult->cols : -1, totCnt, mresult ? ((mresult->rows * mresult->cols != totCnt) ? " RESULT IS NOT SQUARE" : "") : "");
        }
//PERF_END();
    }
    if( !permanentConnection ) {
        disconnect();
    }
    return totCnt;
}

bool sSql::resultOpen(const char * sqlfmt, ...)
{
    sStr tmp;
    if( sqlfmt ) {
        sCallVarg(tmp.vprintf, sqlfmt);
    }
    if( !permanentConnection ) {
        realConnect();
    }
    if( status != eConnected ) {
        return 0;
    }
    return exec(tmp);
}

void sSql::resultClose(void)
{
    while(resultNext());
}

#if _DEBUG
idx sSql_row_count = 0;
#endif
bool sSql::resultNext(void)
{
    if( m_res ) {
#if _DEBUG
        MYSQLDBG("result %p done %"DEC" rows read", m_res, sSql_row_count);
        sSql_row_count = 0;
#endif
        mysql_free_result((MYSQL_RES*)m_res);
        m_res_ids.empty();
        m_res = 0;
        if( mysql_next_result((MYSQL *) dbConn) > 0 ) {
            hasError();
            return false;
        }
    }
    m_res = mysql_store_result((MYSQL *) dbConn);
    if( m_res ) {
        MYSQLDBG("result %p stored %"DEC" columns", m_res, (idx)(mysql_num_fields((MYSQL_RES*) m_res)));
        MYSQL_FIELD * field = 0;
        while( (field = mysql_fetch_field((MYSQL_RES*) m_res)) ) {
            idx * cid = m_res_ids.setString(field->name, field->name_length);
            if( cid ) {
                *cid = m_res_ids.dim() - 1;
            }
        }
    } else {
        hasError();
    }
    return m_res;
}

bool sSql::resultNextRow()
{
    m_res_row = m_res ? mysql_fetch_row((MYSQL_RES*)m_res) : 0;
    m_res_lengths = m_res_row ? mysql_fetch_lengths((MYSQL_RES*)m_res) : 0;
#if _DEBUG
    sSql_row_count = sSql_row_count + (m_res_row ? 1 : 0);
#endif
    return m_res_row;
}

const char * sSql::resultColName(idx col) const
{
    if( m_res && col >= 0 && col < m_res_ids.dim() ) {
        return (const char *)m_res_ids.id(col);
    }
    return 0;
}

idx sSql::resultColId(const char * const name)
{
    idx * id = m_res_ids.get(name);
    return id ? *id : -1;
}

const char * sSql::resultValue(idx col, const char * defval /* = 0 */, idx * value_len/* = 0 */)
{
    if( m_res && col >= 0 && col < m_res_ids.dim() ) {
        //MYSQLDBG("col%"DEC" value '%s'", col, ((MYSQL_ROW)m_res_row)[col]);
        if( value_len && m_res_lengths ) {
            *value_len = ((unsigned long*)m_res_lengths)[col];
        }
        return ((MYSQL_ROW)m_res_row)[col];
    }
    return defval;
}

idx sSql::sscanfTable( const char * sql, const char * fmt00, void * pVal0, idx maxcnt, int sizeofval)
{
    if(!maxcnt)maxcnt=sIdxMax;
    sVarSet mres;
    getTable(sql,&mres);
    idx totCnt=0;
    char * pVal=(char *)pVal0;
    for ( idx ir=0; ir<mres.rows && totCnt<maxcnt; ++ir) {
        for ( idx ic=0; ic<mres.cols && totCnt<maxcnt; ++ic) {
            const char * fmt=sString::next00(fmt00,ic);if(!fmt)return totCnt;
            if(!strcmp(fmt,"%s"))strcpy(pVal,mres.val(ir,ic));
            else sscanf(mres.val(ir,ic),fmt,pVal);
            pVal+=sizeofval;
            ++totCnt;
        }
    }
    return totCnt;
}

idx sSql::ivalue(idx defval, const char * sqlfmt, ...)
{
    sStr tmp; sCallVarg(tmp.vprintf,sqlfmt);
    return ivalue(tmp.ptr(), defval);
}

idx sSql::ivalue(const char * sql, idx defval)
{
    idx ret=defval;
    sscanfTable(sql, "%"DEC, &ret, 1);
    return ret;
}

udx sSql::uvalue(udx defval, const char * sqlfmt, ...)
{
    sStr tmp; sCallVarg(tmp.vprintf,sqlfmt);
    return uvalue(tmp.ptr(), defval);
}

udx sSql::uvalue(const char * sql, udx defval)
{
    udx ret = defval;
    sscanfTable(sql, "%"UDEC, &ret, 1);
    return ret;
}

real sSql::rvalue(real defval, const char * sqlfmt, ...)
{
    sStr tmp; sCallVarg(tmp.vprintf,sqlfmt);
    return rvalue(tmp.ptr(), defval);
}

real sSql::rvalue(const char * sql, real defval)
{
    real ret = defval;
    sscanfTable(sql, "%lf", &ret, 1);
    return ret;
}

char * sSql::svalue(sStr & out, const char * sqlfmt, ...)
{
    idx pos = out.length();

    sStr tmp; sCallVarg(tmp.vprintf,sqlfmt);
    getTable(tmp, 0, &out);
    out.add0cut();
    return out.ptr(pos);
}

time_t sSql::time_value(time_t defval, const char * sqlfmt, ...)
{
    sStr tmp;
    sCallVarg(tmp.vprintf, sqlfmt);
    return time_value(tmp.ptr(), defval);
}

time_t sSql::time_value(const char * sql, time_t defval)
{
    time_t ret = defval;
    sscanfTable(sql, "%"UDEC, &ret, 1);
    return ret;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ tabled result outputs
_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

void sSql::outTblValues(sStr * str,sVarSet * valtbl, const char * fields00, const char * srch00, const char * srchflds00, idx maxfoundout)
{
    //const char * val=0;
    //idx pos=str->length();

    for ( idx ivis=0,i=valtbl->start ; i< valtbl->rows ; ++i) {
        sDic < sMex::Pos > * curRec=valtbl->tbl.ptr(i);
//const char * tmpID=(const char *)valtbl->tbl.id(i);

        bool isin=true;
        if(srch00){
            isin=false;
            for( const char * fld=fields00; fld; fld=sString::next00(fld) ){
                sMex::Pos * ppos=curRec->get(fld);if(!ppos)continue;
                if(!srchflds00 || sString::compareChoice( fld, srchflds00,0,false, 0)!=sNotIdx ) {
                    //if(strstr((const char * )(valtbl->buf.ptr(ppos->pos)),srch)){isin=true; break;}
                    if(sString::searchSubstring( (const char * )(valtbl->buf.ptr(ppos->pos)), 0, srch00,1, 0,srchCaseSensitive ? false : true )){isin=true; break;}
                }
            }
        }



        if(!isin)continue;
        if(ivis>0)str->printf(endlSymb);

        idx j;
        if(fields00){
            const char * fld;
            for( j=0,fld=fields00; fld; fld=sString::next00(fld) ,++j){
                //if(str->length()>pos)
                if(j)str->printf(separSymb);
                sMex::Pos * ppos=curRec->dictCnt() ? curRec->get(fld) : curRec->ptr(j);
                if(ppos && ppos->size){
                    if(isRawOut){
                        sStr t;sString::searchAndReplaceSymbols(&t,valtbl->buf.ptr(ppos->pos),0,rawFltOut," ",0,true,true,false);
                        str->printf("%s",t.ptr());
                    }
                    else str->printf("%s",valtbl->buf.ptr(ppos->pos));
                }
                else str->printf(nullSymb);

            }
        }else {
            for( j=0; j<curRec->dim(); ++j ){
                //if(str->length()>pos)
                if(j)str->printf(separSymb);
                sMex::Pos * ppos=curRec->ptr(j);
                if(ppos && ppos->size){
                    if(isRawOut){
                        sStr t;sString::searchAndReplaceSymbols(&t,valtbl->buf.ptr(ppos->pos),0,rawFltOut," ",0,true,true,false);
                        str->printf("%s",t.ptr());
                    }else str->printf("%s",valtbl->buf.ptr(ppos->pos));
                }
                else str->printf(nullSymb);

            }
        }
        //str->printf(",-");
        ++ivis;
        if(maxfoundout!=0 && ivis>maxfoundout)break;

    }str->add0(2);
}



idx sSql::outTblSql(sStr * str,const char * tblnm, const char * fields, const char * whereclause)
{
    sStr sql("select %s from %s", fields ? fields : "*" , tblnm);
    if(whereclause)sql.printf(" where %s",whereclause);
    sVarSet tbl;
    idx cnt=getTable(sql,&tbl);

    sStr fld;
    if(cnt){
        sString::searchAndReplaceSymbols(&fld,fields,0,",",0,0,true,false,false);
        outTblValues(str,&tbl,fld);
    }
    else {
        sString::searchAndReplaceSymbols(&fld,fields,0,",","-",0,false,true,false);
        sString::searchAndReplaceStrings(str,fld.ptr(),0,","__,"//"__,0,false);
    }
    return cnt;
}



idx sSql::updateTblSql(const char * tblnm, sVar * form, const char * fields00, const char * autoQuotes , const char * whereclause)
{
    sStr sql("update %s set ", tblnm);
    idx cntFld=sHtml::outFieldList(&sql,form,fields00,sHtml::fDoFieldName|sHtml::fDoFieldVal|sHtml::fDoFieldValRequired,autoQuotes,",");
    if(cntFld) {
        if(whereclause && *whereclause)sql.printf(" where %s",whereclause);
        execute(sql.ptr());
    }
    return cntFld;
}

idx sSql::parseFromCSV(const char * tblnm, const char * collist, const char * collistquote, const char * rowprfx,const char * tbl, idx tblen)
{
    sStr sql,res;
    idx icol,itot;
    const char * line,*p=0;

    sString::searchAndReplaceSymbols(&res,tbl,tblen,"\n\r",0,0,true,true,false);

    for ( itot=0, line=res.ptr(),p=0; line ; line=sString::next00(line), ++itot) {
        sql.printf(0,"insert into %s ",tblnm);
        if(collist)sql.printf(" (%s) ",collist);
        sql.printf(" values (");
        if(rowprfx)sql.printf("%s,",rowprfx);

        sStr bfr;sString::searchAndReplaceStrings(&bfr,line,0,","__," // "__,0,true); // do handle correctly emptynesses ,,
        sString::searchAndReplaceStrings(bfr.ptr(),0,"//"_,0,0,true);
        for( icol=0, p=bfr.ptr(); p ;  p=sString::next00(p) , ++icol) {
            if(icol)sql.printf(",");
            if(collistquote[icol]=='1')sql.printf("'");
            sStr tmp;
            sString::searchAndReplaceStrings(&tmp,p,0,"\'"_"\""__,"\\\'"_"\\\""__,0,false);
            sString::cleanEnds(tmp.ptr(),0,sString_symbolsBlank,true);
            //sString::cleanEnds(&tmp,p,0,sString_symbolsBlank,true);
            if(tmp.length())sql.printf("%s",tmp.ptr());
            if(collistquote[icol]=='1')sql.printf("'");
        }
        sql.printf(" )");
        if(icol)
            execute(sql);
    }
    return itot;
}




idx sSql::joinFromMultipleTables(sVarSet * pidlist, const char * sqllist00,const char * colnamelst00, idx recCol)
{
    const char * ppls=colnamelst00; //, * recIDcol=ppls;
    sMex::Pos * pp;
    sVarSet res;getTable(sqllist00,&res);
    pidlist->cols=1;
    for( idx i=0;i<res.rows; ++i) {
        sDic < sMex::Pos > * curRec=pidlist->tbl.set(res(i,recCol));

        ppls=colnamelst00;
        for( idx cc=0; cc<res.cols; ++cc) {
            //if(cc==recCol)recIDcol=ppls;
//            const char * dd=res(i,cc);
            pp=curRec->set(ppls);pp->pos=pidlist->buf.length();pidlist->buf.add( res(i,cc,&pp->size));pidlist->buf.add(__,2);
            ppls=sString::next00(ppls);
        }
        pidlist->cols=sMax(pidlist->cols,res.cols);
    }


    for (const char * sql2=sString::next00(sqllist00); sql2; sql2=sString::next00(sql2) ) {
        res.empty();getTable(sql2,&res);

        colnamelst00=ppls;
        for( idx i=0;i<res.rows; ++i) {
            //const char * llllrec=res(i,recCol);
            sDic < sMex::Pos > * curRec=pidlist->tbl.get(res(i,recCol));
            if(!curRec)continue;
            ppls=colnamelst00;
            for (idx j = 0; j < res.cols; ++j) {
                //const char * dde=res(i,j);

                if(j==recCol)continue;
                pp=curRec->set(ppls);pp->pos=pidlist->buf.length();pidlist->buf.add( res(i,j,&pp->size));pidlist->buf.add(__,2);
                ppls=sString::next00(ppls);
            }
            pidlist->cols=sMax(pidlist->cols,res.cols);
        }
        if(!res.rows) ppls=sString::next00(ppls);
    }

    pidlist->rows=pidlist->tbl.dim();

    return pidlist->rows;
}

idx sSql::outFromMultipleTables(sStr * out, const char * sqllist00,const char * colnamelst00, idx recCol, const char * srch,idx maxCount, idx pgStart)
{
    sVarSet idlist ;
    idx res=joinFromMultipleTables(&idlist, sqllist00,colnamelst00, recCol);

    sStr sql;
    if(srch){sql.cut(0);sString::searchAndReplaceSymbols(&sql, srch,0, ",;", 0,0,true,true,false);}

    if(pgStart)idlist.start=pgStart;
    idx orirows=idlist.rows;
    if(maxCount!=0 && idlist.rows>maxCount)idlist.rows=idlist.start+maxCount;
    if(idlist.rows>orirows)idlist.rows=orirows;

    outTblValues(out,&idlist,colnamelst00,srch ? sql.ptr() : 0 ,0);
    //out->printf("%s\n",bfr.ptr());
    return res;

}

void sSql::Schema::add( const char * tbln, const char * fldlist, const char * quotes , bool isupdateable)
{
    Table * td=tbl.set(tbln);
    td->quotes=dat.length();dat.printf("%s",quotes);dat.add0();
    td->fields=dat.length();dat.printf("%s",fldlist);dat.add0();
    td->fields00=dat.length();sString::searchAndReplaceSymbols(&dat,fldlist,0,",",0,0,true,true,false);
    td->isUpd=isupdateable;
}


void sSql::Schema::load( sSql * sql, const char * tables00, const char * exclColumns00 )
{
    sVarSet rs;
    sStr tmp;

    for(const char * tl=tables00; tl; tl=sString::next00(tl) ){
        sVarSet rs;
        Table * td=tbl.set(tl);

        sql->getTable( tmp.printf(0,"describe %s",tl), &rs);
        td->quotes=dat.length();
        dat.add(0,(rs.rows+1)); // 2*(rs.rows+1) if we reserve twice more for blob data lengths
        td->fields=dat.length();
        td->isUpd=true;

        idx ivis=0;
        for ( idx ir=0; ir<rs.rows; ++ir) {
            const char * qq=rs(ir,0);

            if(exclColumns00 && sString::compareChoice(qq,exclColumns00 ,0,false,0,true)!=-1)
                continue;

            char * qt=dat.ptr(td->quotes);
            const char * bb=rs(ir,1);
            if(strstr(bb,"char") || strstr(bb,"blob") || strstr(bb,"timestamp")){
                qt[ivis]='1';
            }else if(strstr(bb,"int") || strstr(bb,"double") || strstr(bb,"float")  ){
                qt[ivis]='0';
            }/*else if(strstr(bb,"blob") ){
                qt[ivis]='0';
                ++ivis;
                dat.printf("%s%s",ivis ? "," : "" , qq);
                qt[ivis]='1';

            }*/else continue;

            //const char * dlf=rs(ir,0);
            dat.printf("%s%s",ivis ? "," : "" , qq);
            ++ivis;
        }
        dat.add0(2);

        //const char * sss=dat.ptr(td->fields);
        tmp.printf(0,"%s",dat.ptr(td->fields));
        td->fields00=dat.length();
        sString::searchAndReplaceSymbols(&dat,tmp.ptr(),0,",",0,0,true,true,false);

        dat.add0(1);
    }

}




char * sSql::Schema::searchSql(sStr * srchSql, const char * srch,const char * anyExclusions00, sStr * participants)
{
    sSql::Schema * ligschema = this;
    sStr tmp;
    if(!srch)return 0;

    bool isAny=(*srch==':') ? false : true ;

    if( isAny ) {
        tmp.printf("any has ");
        sString::searchAndReplaceStrings(&tmp,srch,0," "_"\n"_"\r\n"__," and any has "__,0,false);
    }else ++srch;
    sString::cleanEnds(&tmp, srch,0, ":"sString_symbolsBlank, true);
    sString::searchAndReplaceSymbols(tmp.ptr(),0, sString_symbolsBlank, 0,0,true,true,true);

    sVec< sStr > fltrset; fltrset.add(ligschema->dim());
    const char  * opList = "is"_"has"_"isnt"_"hasnt"_"range"_;
    const char  * opSQL = "="_"like"_"!="_"not like "_"range"_;
    idx opNum=0,iTblD=1;

    for( const char * par=tmp.ptr(), *combine=0; par ; par=sString::next00(combine)){

        const char * op=sString::next00(par); if(!op)break;
            bool ishas=strstr(op,"has") ? true : false;
            bool isnot=strstr(op,"nt") ? true : false;
            if( sString::compareChoice(op,opList,&opNum,true,0,true)==-1 )continue;
            const char * prf=(ishas) ? "%" : "";
            //const char * prf="";
            const char * suff=(ishas) ? "%" : "";
        char * Vval=sString::next00(op); if(!Vval)break;
        sString::cleanEnds(Vval, 0, "\'\""sString_symbolsBlank, true);
        const char * val;
        if(strcmp((const char *)Vval,(const char *)"-")==0)val="";else val=Vval;

        bool isagregate=false;
        for( idx il=0; il<fltrset.dim(); ++il ) fltrset[il].cut(0);

        sStr ttPar;sString::cleanEnds(&ttPar,par,0,"([])",true);

        for( idx il=0; il<ligschema->dim(); ++il) {
            idx fldNum=0;
            const char * fldQuotes=ligschema->quotes(il);

            if( strcmp(par,"any")!=0 &&
                sString::compareChoice(par,ligschema->fields00(il),&fldNum,true,0,true)==-1 &&
                sString::compareChoice(ttPar.ptr(),ligschema->fields00(il),&fldNum,true,0,true)==-1
                )continue;

            const char * quo=((fldQuotes[fldNum]=='1') || ishas ) ? "\'" : "";
            if(par[0]=='(' || par[1]=='[' || strstr(par,"count(") || strstr(par,"group_concat(") )isagregate=true;


            if(!fltrset[il].length()){
                fltrset[il].printf("select %s from %s ", ligschema->idCol,ligschema->tblname(il) );
                if(isagregate)fltrset[il].printf(" group by %s having ",ligschema->idCol);
                else fltrset[il].printf("where ");
            }else {
                fltrset[il].printf(" %s ",combine);
            }

            idx somedone=false;
            if( strcmp(par,"any")==0) {
                const char * fl; idx ir;
                for ( ir=0, fl=ligschema->fields00(il) ; fl ; fl=sString::next00(fl), ++ir ) {
                    if(strstr(fl,"count(") || strstr(fl,"group_concat(") || strstr(fl,"length(") )continue; // we do not look this in agregate fields
                    if( anyExclusions00 && sString::compareChoice(fl,anyExclusions00,0,false,0,true)!=-1)continue; // exclude some fields from any
                    if(ir) fltrset[il].printf(" or ");
                    quo=((fldQuotes[ir]=='1') || ishas ) ? "\'" : "";
                    fltrset[il].printf("%s %s %s%s%s%s%s", fl ,  sString::next00(opSQL,opNum) , quo, prf,val,suff, quo);
                    somedone=true;
                }
            }
            else {
                const char * grp=(par[0]=='(') ? "group_concat" : (par[0]=='[' ? "count": "");
                if(participants){participants->printf("%s",par);participants->add0();}

                if(strcasecmp(val,"null")==0) {
                    fltrset[il].printf(" (%s%s %s null %s %s %s = '') ",grp, par , isnot ? "is not" : "is" , isnot ? "and" : "or" , isnot ? "NOT" : "" , par );
                }
                else if(strcmp(op, "range")==0) {
                    real vmin=-1.e+13, vmax=1.e+13;
                    if(val[0]==':')sscanf(val+1,"%lf",&vmax);
                    else sscanf(val,"%lf:%lf",&vmin,&vmax);
                    fltrset[il].printf(" (%s%s > %lf and %s < %lf) ",grp,par , vmin, par, vmax);
                }
                else {
                    fltrset[il].printf("%s%s %s %s%s%s%s%s",grp,par ,  sString::next00(opSQL,opNum) , quo, prf,val,suff, quo);
                }
                somedone=true;
            }
            if(!somedone)fltrset[il].cut(0);
            //break;
        }

        sStr loc;
        idx ivis,il,iall;
        // this composes a select statement chosing a single parameter
        for( il=0,iall=0; il<fltrset.dim(); ++il ) {if(fltrset[il].length()==0)continue;++iall;}
        if(iall) {
            for( il=0, ivis=0; il<fltrset.dim(); ++il ) {
                if(fltrset[il].length()==0)continue;
                if(ivis)loc.printf(" union " ); ++ivis;
                if(iall<2)loc.printf("%s", fltrset[il].ptr() );
                else loc.printf("(%s)", fltrset[il].ptr() );
            }
            if(combine) {
                if(!strcmp(combine ,"or")) {
                    srchSql->printf(" union %s ",loc.ptr() );
                }else { ////sStr oo;if(srchSql->ptr())oo.printf("%s",srchSql->ptr());
                    //srchSql->printf(" %s ",combine);
                    //srchSql->printf(" %s in ( select recID from %s where ",ligschema->idCol,ligschema->idTbl);
                    sStr old;if(srchSql->ptr())old.printf("%s",srchSql->ptr());
                    srchSql->printf(0,"select %s from (%s ",ligschema->idCol,old.ptr());
                    //if(strcmp(par,"any")!=0)srchSql->printf("group by %s ",ligschema->idCol);
                    srchSql->printf(") T%"DEC" where %s in (%s) ",iTblD, ligschema->idCol, loc.ptr());
                    iTblD+=2;
                }
            }
            else if(loc.ptr())
                srchSql->printf("%s",loc.ptr());
        }
        combine=sString::next00(val); if(!combine)break;
    }
    srchSql->printf(" group by %s ",ligschema->idCol);
    if(participants){participants->add0(2);}

    return srchSql->ptr();
}



char * sSql::Schema::dumpSql(sStr * dmp, const char * fltr, const char * fldlst)
{
    sSql::Schema * ligschema=this;
    sStr tmp;
    if(!fldlst)return 0;

    sString::cleanEnds(&tmp, fldlst,0, sString_symbolsBlank, true);
    sString::searchAndReplaceSymbols(tmp.ptr(),0, ","sString_symbolsBlank, 0,0,true,true,true);

    dmp->printf("select ");
    const char * par; idx ivis;
    bool isagregate=false;

    sStr ttPar;
    const char * grp;
    for( ivis=0,par=tmp.ptr(); par ; par=sString::next00(par), ++ivis){
        ttPar.cut(0);sString::cleanEnds(&ttPar,par,0,"([])",true);
        grp=(par[0]=='(') ? "group_concat" : (par[0]=='[' ? "count": "");


        for( idx il=0; il<ligschema->dim(); ++il) {
            if( sString::compareChoice(par,ligschema->fields00(il),0,true,0,true)!=-1  || sString::compareChoice(ttPar.ptr(),ligschema->fields00(il),0,true,0,true)!=-1 ){
                dmp->printf("%s%s%s",ivis ? "," : "" , grp,par);
                if(par[0]=='(' || par[1]=='[' || strstr(par,"count(") || strstr(par,"group_concat(") ) isagregate=true;
                break;
            }
        }
    }

    dmp->printf(" from %s ",fltr);
    ivis= fltr ? 1 : 0 ;
    for( idx il=0; il<ligschema->dim(); ++il) {
        for( par=tmp.ptr(); par ; par=sString::next00(par)){
            ttPar.cut(0);sString::cleanEnds(&ttPar,par,0,"([])",true);

            if( sString::compareChoice(par,ligschema->fields00(il),0,true,0,true)!=-1 || sString::compareChoice(ttPar.ptr(),ligschema->fields00(il),0,true,0,true)!=-1 ){
                dmp->printf("%s%s",ivis ? " natural left join " : "" , ligschema->tblname(il));
                ++ivis;
                break;
            }
        }
    }
    if(isagregate)
        dmp->printf(" group by %s ",ligschema->idCol);
    return dmp->ptr();
}



char * sSql::Schema::getAllFields( sStr * dmp)
{
    sSql::Schema * ligschema = this;
    for( idx il=0, ivis=0; il<ligschema->dim(); ++il) {
        dmp->printf("%s%s",ivis ? "," : "" , ligschema->fields(il));
        ++ivis;
    }
    return dmp->ptr();
}





void sSql::Schema::updateTbl(sSql * ligFam, idx recID, sVar * pForm, const char * whereclasue)
{
    sSql::Schema * ligschema=this;

    for( idx il=0; il<ligschema->dim(); ++il) {
        if(!ligschema->isUpdateable(il))continue;
        ensureIDRow(ligFam, recID,ligschema->tblname(il));

        ligFam->updateTblSql(ligschema->tblname(il),pForm, ligschema->fields00(il),ligschema->quotes(il),whereclasue);
    }
}



idx sSql::Schema::ensureIDRow(sSql * ligFam, idx recID,const char * tblnm)
{

    sSql::Schema * ligschema=this;
    //sStr sql;
    idx recIN=0;
    //ligFam->sscanfTable( sql.printf(0,"select %s from %s where recID=%"DEC"",ligschema->idCol,tblnm,recID),"%"DEC"",&recIN);
    recIN=ligFam->ivalue(0,"select %s from %s where recID=%"DEC"",ligschema->idCol,tblnm,recID);
    //if(!recIN){ligFam->execute(sql.printf(0,"insert into %s (%s) values (%"DEC")",tblnm,ligschema->idCol,recID));}
    if(!recIN)ligFam->execute("insert into %s (%s) values (%"DEC")",tblnm,ligschema->idCol,recID);
    return recIN;
}
