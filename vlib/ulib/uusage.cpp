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
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include <slib/std/file.hpp>
#include <slib/std/pipe2.hpp>
#include <slib/utils/sort.hpp>
#include <slib/utils/tbl.hpp>
#include <ulib/uusage.hpp>
#include <qlib/QPrideBase.hpp>
#include <xlib/sqlite.hpp>

#include "uperm.hpp"

using namespace slib;

static bool isUsageDbgWanted(bool trace)
{
    static enum {
        eNo,
        eDebug,
        eTrace,
        eUnknown = -1
    } state = eUnknown;
    if( state == eUnknown ) {
        if( const char * uusage_debug_str = getenv("UUSAGE_DEBUG") ) {
            if( sIsExactly(uusage_debug_str, "trace") ) {
                state = eTrace;
            } else if( sString::parseBool(uusage_debug_str) ) {
                state = eDebug;
            } else {
#if _DEBUG
                state = eDebug;
#else
                state = eNo;
#endif
            }
        }
    }
    return trace ? state >= eTrace : state >= eDebug;
}

static void usageDbg(bool trace, const char * fmt, ...) __attribute__((format(printf, 2, 3)));
static void usageDbg(bool trace, const char * fmt, ...)
{
    if( isUsageDbgWanted(trace) ) {
        sStr buf;
        va_list ap;
        va_start(ap, fmt);
        buf.vprintf(fmt, ap);
        va_end(ap);

        fprintf(stderr, "%s", buf.ptr());
        if( buf.length() && buf[buf.length() - 1] != '\n' ) {
            fprintf(stderr, "\n");
        }
    }
}


#define USAGEDBG(fmt, ...) usageDbg(false, fmt, ##__VA_ARGS__)
#define USAGETRACE(fmt, ...) usageDbg(true, fmt, ##__VA_ARGS__)
#define USAGETRACE_FUNC(fmt, ...) usageDbg(true, "%s:%d : " fmt, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define USAGETRACE_TYPE(type, fmt, ...) usageDbg(true, "%s : " fmt, sUsrUsage2::getTypeName(type), ##__VA_ARGS__)

#define USAGEDBG_SQLITE do { if( _db.hasFailed() ) { fprintf(stderr, "%s:%d : SQLite error %" UDEC ": %s\n", __PRETTY_FUNCTION__, __LINE__, _db.getErrno(), _db.getError()); } } while( 0 )
#define USAGEDBG_SQLITE_TRACE(fmt, ...) fprintf(stderr, "%s:%d : " fmt "\n", __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

static const struct {
        const char * filename;
        const char * column_change;
        const char * column_total;
} usage_type_names[] = {
    { "disk-usage", "disk usage change", "total disk usage" },
    { "temp-usage", "temp usage change", "total temp usage" },
    { "obj-count", "object count change", "total object count" },
    { "req-count", "request count change", "total request count" },
    { "file-count", "file count change", "total file count" },
    { "run-time", "run time", "total run time" },
    { "wait-time", "wait time", "total wait time" },
    { "completion-time", "process completion time", "total process completion time" },
    { "computation-obj-count", "computation object count change", "total computation object count" },
    { "data-loading-obj-count", "data-loading object count change", "total data-loading object count" },
    { "file-obj-count", "file object count change", "total file object count" },
    { "directory-obj-count", "directory object count change", "total directory object count" },
};

const char * sUsrUsage2::getTypeName(EUsageType t)
{
    if( likely(t >= 0 && t <= eUsageTypeLast) ) {
        return usage_type_names[t].filename;
    }
    return 0;
}

sUsrUsage2::EUsageType sUsrUsage2::parseTypeName(const char * s)
{
    if( !s ) {
        return eUsageTypeInvalid;
    }
    for(int i=0; i<=eUsageTypeLast; i++) {
        if( sIs(s, usage_type_names[i].filename) ) {
            return (EUsageType)i;
        }
    }
    return eUsageTypeInvalid;
}

class sUsrUsage2::UsageDb
{
    public:
        enum ESchemaPrefix {
            eMain,
            eTemp
        };
    private:
        sSqlite _db;
        ESchemaPrefix _pfx;

        static const char * schemaPrefix(ESchemaPrefix pfx)
        {
            if( pfx == eTemp ) {
                return "temp";
            } else {
                return "main";
            }
        }

        bool initSchemaV0(ESchemaPrefix pfx = eMain)
        {
            const char * schema_prefix = schemaPrefix(pfx);

            bool success =
                _db.execute("CREATE TABLE %s.user_usage (rowid INTEGER PRIMARY KEY AUTOINCREMENT, timestamp BIGINT NOT NULL, user_id BIGINT NOT NULL, usage_type INTEGER NOT NULL, value BIGINT NOT NULL, tag BIGINT);", schema_prefix) &&
                _db.execute("CREATE INDEX %s.user_usage_idx_timestamp_asc ON user_usage (timestamp ASC);", schema_prefix) &&
                _db.execute("CREATE INDEX %s.user_usage_idx_timestamp_desc ON user_usage (timestamp DESC);", schema_prefix) &&
                _db.execute("CREATE INDEX %s.user_usage_idx_type_tag ON user_usage (usage_type, tag);", schema_prefix) &&
                _db.execute("CREATE TABLE %s.req_usage (timestamp BIGINT NOT NULL, req_id BIGINT NOT NULL, usage_type INTEGER NOT NULL, value BIGINT NOT NULL);", schema_prefix) &&
                _db.execute("CREATE UNIQUE INDEX %s.req_usage_idx ON req_usage (req_id, usage_type);", schema_prefix) &&
                _db.execute("CREATE TABLE %s.obj_usage (timestamp BIGINT NOT NULL, domain_id BIGINT NOT NULL, obj_id BIGINT NOT NULL, usage_type INTEGER NOT NULL, value BIGINT NOT NULL);", schema_prefix) &&
                _db.execute("CREATE UNIQUE INDEX %s.obj_usage_idx ON obj_usage (domain_id, obj_id, usage_type);", schema_prefix);
            return success;
        }

        bool initSchemaV1(ESchemaPrefix pfx = eMain)
        {
            idx prev_version = schemaVersion(pfx);
            const char * schema_prefix = schemaPrefix(pfx);

            bool success =
                _db.execute("CREATE TABLE %s.req_info (timestamp BIGINT NOT NULL, req_id BIGINT NOT NULL, user_id BIGINT NOT NULL, on_behalf_user_id BIGINT DEFAULT NULL);", schema_prefix) &&
                _db.execute("CREATE UNIQUE INDEX %s.req_info_idx ON req_info (req_id);", schema_prefix) &&
                _db.execute("CREATE TABLE %s.obj_info (timestamp BIGINT NOT NULL, domain_id BIGINT NOT NULL, obj_id BIGINT NOT NULL, type_domain_id BIGINT NOT NULL, type_obj_id BIGINT NOT NULL, user_id BIGINT NOT NULL, on_behalf_user_id BIGINT DEFAULT NULL);", schema_prefix) &&
                _db.execute("CREATE UNIQUE INDEX %s.obj_info_idx ON obj_info (domain_id, obj_id);", schema_prefix) &&
                _db.execute("CREATE TABLE %s.settings (timestamp BIGINT NOT NULL, name VARCHAR(255) NOT NULL, value TEXT);", schema_prefix) &&
                _db.execute("CREATE UNIQUE INDEX %s.settings_idx ON settings (name);", schema_prefix) &&
                _db.execute("INSERT INTO %s.settings (timestamp, name, value) VALUES (%" DEC ", 'schema_version', 1);", schema_prefix, (idx)time(0));

            idx cnt_user_usage_rows = _db.getIValue(0, "SELECT COUNT(*) FROM %s.user_usage;", schema_prefix);
            if( cnt_user_usage_rows ) {
                _db.execute("INSERT OR IGNORE INTO %s.settings (timestmap, name, value) VALUES (%" DEC ", 'data_version', %" DEC ");", schema_prefix, (idx)time(0), prev_version);
            }

            return success;
        }

        void initSchema()
        {
            if( _db.startTransaction() ) {
                if( initSchemaV0() && initSchemaV1() ) {
                    _db.commit();
                } else {
                    USAGEDBG_SQLITE;
                    _db.rollback();
                }
            }
        }

        void dropTemp()
        {
            _db.executeExact("DROP TABLE IF EXISTS temp.req_info;");
            _db.executeExact("DROP TABLE IF EXISTS temp.obj_info;");
            _db.executeExact("DROP TABLE IF EXISTS temp.settings;");

            _db.executeExact("DROP TABLE IF EXISTS temp.user_usage;");
            _db.executeExact("DROP TABLE IF EXISTS temp.req_usage;");
            _db.executeExact("DROP TABLE IF EXISTS temp.obj_usage;");
        }

        void initTemp()
        {
            dropTemp();
            initSchemaV0(eTemp);
            initSchemaV1(eTemp);
        }

        struct UInfo {
            udx user_id;
            idx cnt_rows_in_range;
            sLst<idx> iuser_lists;

            UInfo()
            {
                user_id = 0;
                cnt_rows_in_range = 0;
            }
        };

    public:
        UsageDb(const char * path, bool readonly = false) : _db(path, readonly), _pfx(eMain)
        {
            if( _db.ok() && !readonly ) {
                idx nrecords = _db.getIValue(0, "SELECT COUNT(*) FROM user_usage;");
                if( !nrecords ) {
                    initSchema();
                } else if( schemaVersion(eMain) < expectedVersion() ) {
                    USAGEDBG("Usage db '%s' schema version %" DEC " is outdated", path, schemaVersion(eMain));
                    initTemp();
                }
            }
            if( schemaVersion() > expectedVersion() ) {
                fprintf(stderr, "%s:%d : unsupported schema version %" DEC "\n",  __PRETTY_FUNCTION__, __LINE__, schemaVersion());
                _db.reset(0, true);
            } else {
                _db.executeExact("PRAGMA journal_mode = WAL;");
                _db.executeExact("PRAGMA secure_delete = FALSE;");
            }
        }

        bool ok() const { return _db.ok(); }
        bool startTransaction() { return _db.startTransaction(); }
        bool commit() { return _db.commit(); }
        bool rollback() { return _db.rollback(); }

        idx schemaVersion(ESchemaPrefix pfx = eMain) {
            return _db.getIValue(0, "SELECT value FROM %s.settings WHERE name = 'schema_version';", schemaPrefix(pfx));
        }

        idx dataVersion(ESchemaPrefix pfx = eMain) {
            return _db.getIValue(schemaVersion(), "SELECT value FROM %s.settings WHERE name = 'data_version';", schemaPrefix(pfx));
        }

        idx expectedVersion() {
            return 1;
        }

        idx dimHistoryRecords(ESchemaPrefix pfx = eMain)
        {
            idx ret = 0;
            if( _db.ok() ) {
                ret = _db.getIValue(0, "SELECT COUNT(*) FROM %s.user_usage;", schemaPrefix(pfx));
                if( _db.hasFailed() ) {
                    USAGEDBG_SQLITE;
                }
            }
            return ret;
        }

        void setSchemaPrefix(ESchemaPrefix pfx)
        {
            _pfx = pfx;
        }

        void addRecord(udx user_id, sUsrUsage2::EUsageType type, idx tag, time_t timestamp, idx value)
        {
            udx timestamp_udx = timestamp;
            bool timestamp_is_last = true;
            if( _db.resultOpen("SELECT rowid FROM %s.user_usage WHERE user_id = %" UDEC " and usage_type = %d AND tag = %" DEC " and timestamp > %" UDEC " LIMIT 1;", schemaPrefix(_pfx), user_id, type, tag, timestamp_udx) ) {
                if( _db.resultNextRow() ) {
                    timestamp_is_last = false;
                }
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }

            if( timestamp_is_last ) {
                if( _db.resultOpen("SELECT rowid, value, timestamp FROM %s.user_usage WHERE user_id = %" UDEC " and usage_type = %d AND TAG = %" DEC " and timestamp <= %" UDEC " ORDER BY timestamp DESC LIMIT 2;", schemaPrefix(_pfx), user_id, type, tag, timestamp_udx) ) {
                    idx cnt_same_value = 0;
                    idx update_rowid = -sIdxMax;
                    if( _db.resultNextRow() ) {
                        update_rowid = _db.resultIValue(0);
                        if( _db.resultIValue(1) == value ) {
                            if( _db.resultUValue(2) == timestamp_udx ) {
                                _db.resultClose();
                                return;
                            }
                            cnt_same_value++;
                        }
                    }
                    if( _db.resultNextRow() ) {
                        if( _db.resultIValue(1) == value ) {
                            cnt_same_value++;
                        }
                    }

                    if( cnt_same_value == 2 ) {
                        if( !_db.execute("UPDATE %s.user_usage SET timestamp = %" UDEC " WHERE rowid = %" DEC ";", schemaPrefix(_pfx), timestamp_udx, update_rowid) ) {
                            USAGEDBG_SQLITE;
                        }
                        return;
                    }
                } else {
                    USAGEDBG_SQLITE;
                }
            }

            if( !_db.execute("INSERT INTO %s.user_usage (user_id, usage_type, tag, timestamp, value) VALUES(%" UDEC ", %d, %" DEC ", %" UDEC ", %" DEC ");", schemaPrefix(_pfx), user_id, (int)type, tag, timestamp_udx, value) ) {
                USAGEDBG_SQLITE;
            }
        }

        time_t getFirstTimestampAfter(time_t after, sUsrUsage2::EUsageType type, idx tag)
        {
            idx ret = sIdxMax;
            udx after_udx = after;
            if( _db.resultOpen("SELECT timestamp FROM %s.user_usage WHERE usage_type = %d AND tag = %" DEC " AND timestamp >= %" UDEC " ORDER BY timestamp ASC LIMIT 1;", schemaPrefix(_pfx), type, tag, after_udx) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, sIdxMax);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        time_t getFirstTimestamp(ESchemaPrefix pfx)
        {
            idx ret = sIdxMax;
            if( _db.resultOpen("SELECT timestamp FROM %s.user_usage ORDER BY timestamp ASC LIMIT 1;", schemaPrefix(pfx)) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, sIdxMax);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        time_t getLastTimestampBefore(time_t before, udx user_id, sUsrUsage2::EUsageType type, idx tag)
        {
            idx ret = 0;
            udx before_udx = before;
            if( _db.resultOpen("SELECT timestamp FROM %s.user_usage WHERE user_id = %" UDEC " AND usage_type = %d AND tag = %" DEC " AND timestamp <= %" UDEC " ORDER BY timestamp DESC LIMIT 1;", schemaPrefix(_pfx), user_id, type, tag, before_udx) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        time_t getLastTimestampBefore(time_t before, sUsrUsage2::EUsageType type, idx tag)
        {
            idx ret = 0;
            udx before_udx = before;
            if( _db.resultOpen("SELECT timestamp FROM %s.user_usage WHERE usage_type = %d AND tag = %" DEC " AND timestamp <= %" UDEC " ORDER BY timestamp DESC LIMIT 1;", schemaPrefix(_pfx), type, tag, before_udx) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        time_t getLastTimestampBefore(time_t before)
        {
            idx ret = 0;
            udx before_udx = before;
            if( _db.resultOpen("SELECT timestamp FROM %s.user_usage WHERE timestamp <= %" UDEC " ORDER BY timestamp DESC LIMIT 1;", schemaPrefix(_pfx), before_udx) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        typedef bool (*readCallback)(sUsrUsage2::EUsageType type, idx list_index, idx tag, time_t timestamp, idx value, sVec<idx> * psum_adjust, void * param);

        idx readRows(const sUsrUsage2::UserList * user_lists, idx num_user_lists, sUsrUsage2::EUsageType type, idx tag, time_t since, time_t to, readCallback cb, void * cb_param)
        {
            if( !num_user_lists ) {
                return 0;
            }

            sVec<udx> all_user_ids;
            sMex user_id2info_mex;
            sDic<UInfo> user_id2info;
            for(idx i = 0; i < num_user_lists; i++) {
                for(idx j = 0; j < user_lists[i].user_ids.dim(); j++) {
                    udx user_id = user_lists[i].user_ids[j];
                    *all_user_ids.add(1) = user_id;
                    UInfo * uinfo = user_id2info.set(&user_id, sizeof(user_id));
                    if( !uinfo->iuser_lists.mex() ) {
                        uinfo->iuser_lists.init(&user_id2info_mex);
                        uinfo->user_id = user_id;
                    }
                    *uinfo->iuser_lists.add(1) = i;
                }
            }

            const udx to_udx = to;

            const time_t first_timestamp = getFirstTimestampAfter(0, type, tag);

            idx cb_calls = 0;
            sVec<idx> iuser_list2start_value(sMex::fExactSize|sMex::fSetZero);
            sVec<idx> iuser_list2cnt_callbacks(sMex::fExactSize|sMex::fSetZero);
            iuser_list2start_value.resize(num_user_lists);
            iuser_list2cnt_callbacks.resize(num_user_lists);
            sStr sql("SELECT timestamp, user_id, value FROM %s.user_usage WHERE usage_type = %d AND tag = %" DEC " AND timestamp <= %" UDEC " AND ", schemaPrefix(_pfx), type, tag, to_udx);
            sSql::exprInList(sql, "user_id", all_user_ids);
            sql.addString(" ORDER BY timestamp ASC;");

            if( _db.resultOpenExact(sql.ptr()) ) {
                while( _db.resultNextRow() ) {
                    time_t timestamp = (time_t)_db.resultUValue(0);
                    udx user_id = _db.resultUValue(1);
                    idx cur_value = _db.resultIValue(2);

                    UInfo * uinfo = user_id2info.get(&user_id, sizeof(user_id));

                    for(idx iil=0; uinfo && iil<uinfo->iuser_lists.dim(); iil++) {
                        const idx il = uinfo->iuser_lists[iil];
                        if( timestamp < since ) {
                            iuser_list2start_value[il] += cur_value;
                            continue;
                        } else if( timestamp > to ) {
                            break;
                        } else {
                            bool cb_result = false;
                            if( unlikely(timestamp == first_timestamp) ) {
                                assert(iuser_list2start_value[il] == 0);
                                iuser_list2start_value[il] = cur_value;
                                cb_result = cb(type, il, tag, timestamp, 0, &iuser_list2start_value, cb_param);
                            } else {
                                cb_result = cb(type, il, tag, timestamp, cur_value, &iuser_list2start_value, cb_param);
                            }

                            iuser_list2cnt_callbacks[il]++;
                            cb_calls++;

                            if( !cb_result ) {
                                _db.resultClose();
                                return cb_calls;
                            }
                        }
                    }
                }
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return cb_calls;
        }

        idx getObjUsage(const sHiveId & id, sUsrUsage2::EUsageType type)
        {
            idx ret = 0;
            if( _db.resultOpen("SELECT value FROM %s.obj_usage WHERE domain_id = %" UDEC " AND obj_id = %" UDEC " AND usage_type = %d;", schemaPrefix(_pfx), id.domainId(), id.objId(), type) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        idx getReqUsage(const udx req, sUsrUsage2::EUsageType type)
        {
            idx ret = 0;
            if( _db.resultOpen("SELECT value FROM %s.req_usage WHERE req_id = %" UDEC " AND usage_type = %d;", schemaPrefix(_pfx), req, type) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

    private:
        struct getUserUsageSumWorkerParam {
            idx sum;
            idx rows_collected;
        };
        static bool getUserUsageSumWorker(sUsrUsage2::EUsageType type, idx list_index, idx tag, time_t timestamp, idx value, sVec<idx> * psum_adjust, void * param_)
        {
            getUserUsageSumWorkerParam * param = static_cast<getUserUsageSumWorkerParam*>(param_);
            param->sum += value;
            if( psum_adjust && param->rows_collected == 0 ) {
                param->sum += *psum_adjust->ptr(list_index);
            }
            param->rows_collected++;
            return true;
        }

    public:
        idx getUserUsageSum(udx user_id, sUsrUsage2::EUsageType type, idx tag, time_t since, time_t to)
        {
            UserList list;
            list.user_ids.init(sMex::fExactSize);
            *list.user_ids.add(1) = user_id;

            getUserUsageSumWorkerParam param;
            sSet(&param);
            readRows(&list, 1, type, tag, since, to, getUserUsageSumWorker, &param);
            return param.sum;
        }

        idx getUserReqs(sVec<udx> & out, udx user_id)
        {
            idx cnt = 0;
            if( _db.resultOpen("SELECT DISTINCT req_id FROM %s.req_usage WHERE (user_id = %" UDEC " AND (on_behalf_user_id IS NULL OR on_behalf_user_id = 0)) OR (on_behalf_user_id IS NOT NULL AND on_behalf_user_id = %" UDEC ");", schemaPrefix(_pfx), user_id, user_id) ) {
                while( _db.resultNextRow() ) {
                    *out.add(1) = _db.resultUValue(0);
                    cnt++;
                }
                _db.resultClose();
            }
            return cnt;
        }

        idx getUserObjs(sVec<sHiveId> & out, udx user_id)
        {
            idx cnt = 0;
            if( _db.resultOpen("SELECT DISTINCT domain_id, obj_id FROM %s.obj_info WHERE (user_id = %" UDEC " AND (on_behalf_user_id IS NULL OR on_behalf_user_id = 0)) OR (on_behalf_user_id IS NOT NULL AND on_behalf_user_id = %" UDEC ");", schemaPrefix(_pfx), user_id, user_id) ) {
                while( _db.resultNextRow() ) {
                    udx domain_id = _db.resultUValue(0);
                    idx obj_id = _db.resultUValue(1);
                    out.add(1)->set(domain_id, obj_id, (udx)0);
                    cnt++;
                }
                _db.resultClose();
            }
            return cnt;
        }

        udx getObjUser(const sHiveId & id)
        {
            udx ret = _db.getUValue(0, "SELECT user_id FROM %s.obj_info WHERE domain_id = %" UDEC " AND obj_id = %" UDEC ";", schemaPrefix(_pfx), id.domainId(), id.objId());
            if( _db.hasFailed() ) {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        udx getReqUser(udx req_id)
        {
            udx ret = _db.getUValue(0, "SELECT user_id FROM %s.req_info WHERE req_id = %" UDEC ";", schemaPrefix(_pfx), req_id);
            if( _db.hasFailed() ) {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        udx getObjOnBehalfUser(const sHiveId & id)
        {
            udx ret = _db.getUValue(0, "SELECT on_behalf_user_id FROM %s.obj_info WHERE domain_id = %" UDEC " AND obj_id = %" UDEC ";", schemaPrefix(_pfx), id.domainId(), id.objId());
            if( _db.hasFailed() ) {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        udx getReqOnBehalfUser(udx req_id)
        {
            udx ret = _db.getUValue(0, "SELECT on_behalf_user_id FROM %s.req_info WHERE req_id = %" UDEC ";", schemaPrefix(_pfx), req_id);
            if( _db.hasFailed() ) {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        bool hasObjInfo(const sHiveId & id)
        {
            idx ret = _db.getIValue(0, "SELECT COUNT(*) FROM %s.obj_info WHERE domain_id = %" UDEC " AND obj_id = %" UDEC ";", schemaPrefix(_pfx), id.domainId(), id.objId());
            if( _db.hasFailed() ) {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        bool hasReqInfo(udx req_id)
        {
            idx ret = _db.getIValue(0, "SELECT COUNT(*) FROM %s.req_info WHERE req_id = %" UDEC ";", schemaPrefix(_pfx), req_id);
            if( _db.hasFailed() ) {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        void setReqInfo(const udx req, time_t timestamp, udx user_id, udx on_behalf_user_id)
        {
            udx timestamp_udx = timestamp;
            sStr on_behalf_user_id_str;
            if( on_behalf_user_id ) {
                on_behalf_user_id_str.addNum(on_behalf_user_id);
            } else {
                on_behalf_user_id_str.addString("NULL");
            }
            if( !_db.execute("INSERT OR REPLACE INTO %s.req_info (timestamp, req_id, user_id, on_behalf_user_id) VALUES(%" UDEC ", %" UDEC ", %" UDEC ", %s);", schemaPrefix(_pfx), timestamp_udx, req, user_id, on_behalf_user_id_str.ptr()) ) {
                USAGEDBG_SQLITE;
            }
        }

        void setObjInfo(const sHiveId & id, time_t timestamp, const sHiveId & type_id, udx user_id, udx on_behalf_user_id)
        {
            udx timestamp_udx = timestamp;
            sStr on_behalf_user_id_str;
            if( on_behalf_user_id ) {
                on_behalf_user_id_str.addNum(on_behalf_user_id);
            } else {
                on_behalf_user_id_str.addString("NULL");
            }
            if( !_db.execute("INSERT OR REPLACE INTO %s.obj_info (timestamp, domain_id, obj_id, type_domain_id, type_obj_id, user_id, on_behalf_user_id) VALUES(%" UDEC ", %" UDEC ", %" UDEC ", %" UDEC ", %" UDEC ", %" UDEC ", %s);", schemaPrefix(_pfx), timestamp_udx, id.domainId(), id.objId(), type_id.domainId(), type_id.objId(), user_id, on_behalf_user_id_str.ptr()) ) {
                USAGEDBG_SQLITE;
            }
        }

        void setObjUsage(const sHiveId & id, time_t timestamp, idx disk_usage, idx file_count)
        {
            udx timestamp_udx = timestamp;
            if( !_db.execute("INSERT OR REPLACE INTO %s.obj_usage (timestamp, domain_id, obj_id, usage_type, value) VALUES(%" UDEC ", %" UDEC ", %" UDEC ", %d, %" DEC ");", schemaPrefix(_pfx), timestamp_udx, id.domainId(), id.objId(), sUsrUsage2::eDiskUsage, disk_usage) ) {
                USAGEDBG_SQLITE;
            }
            if( !_db.execute("INSERT OR REPLACE INTO %s.obj_usage (timestamp, domain_id, obj_id, usage_type, value) VALUES(%" UDEC ", %" UDEC ", %" UDEC ", %d, %" DEC ");", schemaPrefix(_pfx), timestamp_udx, id.domainId(), id.objId(), sUsrUsage2::eFileCount, file_count) ) {
                USAGEDBG_SQLITE;
            }
        }

        void setObjUsage2(const sHiveId & id, time_t timestamp, sUsrUsage2::EUsageType type, idx value)
        {
            udx timestamp_udx = timestamp;
            if( !_db.execute("INSERT OR REPLACE INTO %s.obj_usage (timestamp, domain_id, obj_id, usage_type, value) VALUES(%" UDEC ", %" UDEC ", %" UDEC ", %d, %" DEC ");", schemaPrefix(_pfx), timestamp_udx, id.domainId(), id.objId(), type, value) ) {
                USAGEDBG_SQLITE;
            }
        }

        void setReqUsage(const udx req, time_t timestamp, idx temp_usage)
        {
            udx timestamp_udx = timestamp;
            if( !_db.execute("INSERT OR REPLACE INTO %s.req_usage (timestamp, req_id, usage_type, value) VALUES(%" UDEC ", %" UDEC ", %d, %" DEC ");", schemaPrefix(_pfx), timestamp_udx, req, sUsrUsage2::eTempUsage, temp_usage) ) {
                USAGEDBG_SQLITE;
            }
        }

        void setReqUsage2(const udx req, time_t timestamp, sUsrUsage2::EUsageType type, idx value)
        {
            udx timestamp_udx = timestamp;
            if( !_db.execute("INSERT OR REPLACE INTO %s.req_usage (timestamp, req_id, usage_type, value) VALUES(%" UDEC ", %" UDEC ", %d, %" DEC ");", schemaPrefix(_pfx), timestamp_udx, req, type, value) ) {
                USAGEDBG_SQLITE;
            }
        }

        void delObjUsage(const sHiveId & id)
        {
            if( !_db.execute("DELETE FROM %s.obj_usage WHERE domain_id = %" UDEC " AND obj_id = %" UDEC ";", schemaPrefix(_pfx), id.domainId(), id.objId()) ) {
                USAGEDBG_SQLITE;
            }
            if( !_db.execute("DELETE FROM %s.obj_info WHERE domain_id = %" UDEC " AND obj_id = %" UDEC ";", schemaPrefix(_pfx), id.domainId(), id.objId()) ) {
                USAGEDBG_SQLITE;
            }
        }

        void delReqUsage(const udx req)
        {
            if( !_db.execute("DELETE FROM %s.obj_usage WHERE req_id = %" UDEC ";", schemaPrefix(_pfx), req) ) {
                USAGEDBG_SQLITE;
            }
            if( !_db.execute("DELETE FROM %s.req_info WHERE req_id = %" UDEC ";", schemaPrefix(_pfx), req) ) {
                USAGEDBG_SQLITE;
            }
        }

    private:
        struct UserTypeTag {
            udx user_id;
            EUsageType usage_type;
            udx tag;

            UserTypeTag() {
                sSet(this);
            }
        };

        struct UserUsageRecord {
            UserTypeTag user_type_tag;
            udx timestamp;
            udx value;

            UserUsageRecord() {
                sSet(this);
            }
        };

    public:
        bool upgradeMainData()
        {
            if( dataVersion() >= expectedVersion() ) {
                return true;
            }

            sStr escape_buf;
            if( startTransaction() ) {
                bool success = true;
                if( schemaVersion() < 1 ) {
                    success = success && initSchemaV1(eMain);
                }

                if( dimHistoryRecords(eTemp) ) {
                    success = success && _db.execute("DELETE FROM %s.req_info; INSERT INTO %s.req_info SELECT * FROM %s.req_info;", schemaPrefix(eMain), schemaPrefix(eMain), schemaPrefix(eTemp));
                    if( !success ) {
                        USAGEDBG_SQLITE;
                    }

                    success = success && _db.execute("DELETE FROM %s.obj_info; INSERT INTO %s.obj_info SELECT * FROM %s.obj_info;", schemaPrefix(eMain), schemaPrefix(eMain), schemaPrefix(eTemp));
                    if( !success ) {
                        USAGEDBG_SQLITE;
                    }


                    success = success && _db.execute("DELETE FROM %s.req_usage; INSERT INTO %s.req_usage SELECT * FROM %s.req_usage;", schemaPrefix(eMain), schemaPrefix(eMain), schemaPrefix(eTemp));
                    if( !success ) {
                        USAGEDBG_SQLITE;
                    }

                    success = success && _db.execute("DELETE FROM %s.obj_usage; INSERT INTO %s.obj_usage SELECT * FROM %s.obj_usage;", schemaPrefix(eMain), schemaPrefix(eMain), schemaPrefix(eTemp));
                    if( !success ) {
                        USAGEDBG_SQLITE;
                    }


                    sDic<idx> v0_sums;
                    if( _db.resultOpen("SELECT user_id, usage_type, tag, value FROM %s.user_usage;", schemaPrefix(eMain)) ) {
                        while( _db.resultNextRow() ) {
                            UserTypeTag user_type_tag;
                            user_type_tag.user_id = _db.resultUValue(0);
                            user_type_tag.usage_type = (slib::sUsrUsage2::EUsageType)_db.resultIValue(1);
                            user_type_tag.tag = _db.resultIValue(2);

                            idx value = _db.resultIValue(3);

                            if( idx * psum = v0_sums.get(&user_type_tag, sizeof(user_type_tag)) ) {
                                *psum += value;
                            } else {
                                *v0_sums.set(&user_type_tag, sizeof(user_type_tag)) = value;
                            }
                        }
                        _db.resultClose();
                    } else {
                        success = false;
                        USAGEDBG_SQLITE;
                    }

                    udx first_v1_timestamp = (udx)getFirstTimestamp(eTemp);
                    sDic<idx> v1_user_type_tags_cnt_migrated;
                    sVec<UserUsageRecord> v1_records;
                    if( _db.resultOpen("SELECT user_id, usage_type, tag, timestamp, value FROM %s.user_usage ORDER BY timestamp;", schemaPrefix(eTemp)) ) {
                        while( _db.resultNextRow() ) {
                            UserUsageRecord & user_usage_record = *v1_records.add(1);
                            user_usage_record.user_type_tag.user_id = _db.resultUValue(0);
                            user_usage_record.user_type_tag.usage_type = (slib::sUsrUsage2::EUsageType)_db.resultIValue(1);
                            user_usage_record.user_type_tag.tag = _db.resultIValue(2);
                            user_usage_record.timestamp = _db.resultUValue(3);
                            user_usage_record.value = _db.resultIValue(4);

                            *v1_user_type_tags_cnt_migrated.set(&user_usage_record.user_type_tag, sizeof(user_usage_record.user_type_tag)) = 0;
                        }
                        _db.resultClose();
                    } else {
                        success = false;
                        USAGEDBG_SQLITE;
                    }

                    for(idx ir = 0; success && ir < v0_sums.dim(); ir++) {
                        idx v0_sum = *v0_sums.ptr(ir);
                        const UserTypeTag * puser_type_tag = static_cast<const UserTypeTag*>(v0_sums.id(ir));

                        if( !v1_user_type_tags_cnt_migrated.get(puser_type_tag, sizeof(UserTypeTag)) ) {
                            if( puser_type_tag->usage_type != eRunTime && puser_type_tag->usage_type != eWaitTime && v0_sum ) {
                                USAGETRACE("Zeroing v0 value sum by adding value delta %" DEC " for user_id %" UDEC " type %s tag %" UDEC " timestamp %" UDEC " from user_usage;", -v0_sum, puser_type_tag->user_id, sUsrUsage2::getTypeName(puser_type_tag->usage_type), puser_type_tag->tag, first_v1_timestamp);
                                if( !_db.execute("INSERT INTO %s.user_usage (user_id, usage_type, tag, timestamp, value) VALUES(%" UDEC ", %d, %" DEC ", %" UDEC ", %" DEC ");", schemaPrefix(eMain), puser_type_tag->user_id, (int)puser_type_tag->usage_type, puser_type_tag->tag, first_v1_timestamp, -v0_sum) ) {
                                    success = false;
                                    USAGEDBG_SQLITE;
                                }
                            }
                        }
                    }

                    for(idx ir = 0; success && ir < v1_records.dim(); ir++) {
                        UserUsageRecord & user_usage_record = v1_records[ir];
                        idx * pcnt_migrated = v1_user_type_tags_cnt_migrated.get(&user_usage_record.user_type_tag, sizeof(user_usage_record.user_type_tag));
                        idx value = user_usage_record.value;
                        if( !*pcnt_migrated && user_usage_record.user_type_tag.usage_type != eRunTime && user_usage_record.user_type_tag.usage_type != eWaitTime ) {
                            if( idx * pv0_sum = v0_sums.get(&user_usage_record.user_type_tag, sizeof(user_usage_record.user_type_tag)) ) {
                                value -= *pv0_sum;
                            }
                        }
                        ++*pcnt_migrated;
                        USAGETRACE("Updating v0 value sum by adding value delta %" DEC " for user_id %" UDEC " type %s tag %" UDEC " timestamp %" UDEC " from user_usage;", value, user_usage_record.user_type_tag.user_id, sUsrUsage2::getTypeName(user_usage_record.user_type_tag.usage_type), user_usage_record.user_type_tag.tag, first_v1_timestamp);
                        if( !_db.execute("INSERT INTO %s.user_usage (user_id, usage_type, tag, timestamp, value) VALUES (%" UDEC ", %d, %" DEC ", %" UDEC ", %" DEC ");", schemaPrefix(eMain), user_usage_record.user_type_tag.user_id, (int) user_usage_record.user_type_tag.usage_type,
                            user_usage_record.user_type_tag.tag, user_usage_record.timestamp, value) ) {
                            success = false;
                            USAGEDBG_SQLITE;
                        }
                    }
                }

                success = success && _db.execute("INSERT OR REPLACE INTO %s.settings (timestamp, name, value) VALUES (%" DEC ", 'data_version', %" DEC ");", schemaPrefix(eMain), (idx)time(0), expectedVersion());
                if( !success ) {
                    USAGEDBG_SQLITE;
                }

                if( success ) {
                    dropTemp();
                    commit();
                    return true;
                } else {
                    rollback();
                }
            } else {
                USAGEDBG_SQLITE;
            }

            return false;
        }
};

static bool findUsrUsage2Id(sHiveId & result, const sUsr & user)
{
    sUsrObjRes obj_res;
    user.objs2("special", obj_res, 0, "meaning,version", "^user_usage$,^1$", "meaning,version");
    for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
        const char * meaning = obj_res.getValue(obj_res.get(*obj_res.get(it), "meaning"));
        const char * version = obj_res.getValue(obj_res.get(*obj_res.get(it), "version"));
        if( meaning && version && !strcmp(meaning, "user_usage") && atoidx(version) == 1 ) {
            result = *obj_res.id(it);
            return true;
        }
    }
    result = sHiveId::zero;
    return false;
}

#define NEW_SUSR_USAGE(user, id, admin, ret, prc) \
    new sUsrUsage2(user, id, admin); \
    do { \
        if( !ret ) { \
            RCSETP(prc, sRC::eAllocating, sRC::eMemory, sRC::eMemory, sRC::eExhausted); \
        } else if( !ret->Id() ) { \
            RCSETP(prc, sRC::eOpening, sRC::eObject, sRC::eOperation, sRC::eFailed); \
        } else { \
            *prc = sRC::zero; \
        } \
    } while( 0 )

const sUsrUsage2 * sUsrUsage2::getObj(const sUsr & user, sRC * prc)
{
    sRC temp_rc;
    if( !prc ) {
        prc = &temp_rc;
    }

    if( !user.Id() || user.isGuest() ) {
        RCSETP(prc, sRC::eFinding, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
        return 0;
    }

    sHiveId id;
    if( !findUsrUsage2Id(id, user) ) {
        RCSETP(prc, sRC::eFinding, sRC::eObject, sRC::eResult, sRC::eEmpty);
        return 0;
    }

    USAGETRACE("using existing object %s", id.print());
    const sUsrUsage2 * ret = NEW_SUSR_USAGE(user, id, 0, ret, prc);

    return ret;
}

sUsrUsage2 * sUsrUsage2::ensureObj(sUsr & admin, sRC * prc)
{
    sRC temp_rc;
    if( !admin.isAdmin() ) {
        RCSETP(prc, sRC::eFinding, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
        return 0;
    }

    sHiveId id;
    if( findUsrUsage2Id(id, admin) ) {
        USAGETRACE("using existing object %s", id.print());
        sUsrUsage2 * ret = NEW_SUSR_USAGE(admin, id, &admin, ret, prc);
        return ret;
    }

    USAGETRACE("creating new usage object");
    admin.updateStart();
    *prc = admin.objCreate(id, "special");
    if( prc->isSet() ) {
        admin.updateAbandon();
        return 0;
    }

    if( !admin.allow4admins(id) || !admin.allowRead4users(id) ) {
        admin.updateAbandon();
        RCSETP(prc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
        return 0;
    }

    sUsrUsage2 * ret = NEW_SUSR_USAGE(admin, id, &admin, ret, prc);
    if( !ret || !ret->Id() ) {
        admin.updateAbandon();
        return ret;
    }

    if( !ret->propSet("meaning", "user_usage") || !ret->propSet("title", "User usage statistics") || !ret->propSet("version", "1") ) {
        admin.updateAbandon();
        RCSETP(prc, sRC::eCreating, sRC::eProperty, sRC::eOperation, sRC::eFailed);
        delete ret;
        return 0;
    }

    admin.updateComplete();
    USAGETRACE("created new usage object %s", ret->Id().print());
    *prc = sRC::zero;

    return ret;
}

time_t sUsrUsage2::getFirstUpdateTime() const
{
    if( _usage_db ) {
        return _usage_db->getFirstTimestamp(UsageDb::eMain);
    }
    return 0;
}

time_t sUsrUsage2::getLastUpdateTime() const
{
    if( _usage_db ) {
        return _usage_db->getLastTimestampBefore(sIdxMax);
    }
    return 0;
}

class sUsrUsage2::IncrementalUpdates {
    public:
        struct Record {
            udx user_id;
            idx values[eUsageTypeLast + 1];
            Record() {
                sSet(this);
            }
        };

    private:
        sDic<idx> _row_map;
        sVec<Record> _records;

    public:
        Record * ensureRecord(udx user_id)
        {
            Record * r = getRecord(user_id);
            if( !r ) {
                *_row_map.set(&user_id, sizeof(user_id)) = _records.dim();
                r = _records.add(1);
                r->user_id = user_id;
            }
            return r;
        }

        Record * getRecord(udx user_id)
        {
            idx * pir = _row_map.get(&user_id, sizeof(user_id));
            return pir ? _records.ptr(*pir) : 0;
        }

        idx dim() const { return _records.dim(); }
        Record * ptr(idx i) { return _records.ptr(i); }
        const Record * ptr(idx i) const { return _records.ptr(i); }
};


#define PROGRESS_USER_ITEMS 100
#define REPORT_PROGRESS \
if( cb ) cb(cb_param, sMin<idx>(iuser * PROGRESS_USER_ITEMS + progress_items++, (iuser + 1) * PROGRESS_USER_ITEMS), sNotIdx, num_users * PROGRESS_USER_ITEMS)

static idx getTempUsage(udx req_id, sSql & db)
{
    sVarSet req_data_tbl;
    sStr sql_buf("SELECT dataName, LENGTH(dataBlob), IF(SUBSTRING(dataBlob, 1, 9) = 'ZmlsZTovL', dataBlob, NULL) FROM QPData WHERE reqID = %" UDEC, req_id);
    db.getTable(sql_buf, &req_data_tbl);
    idx temp_usage = 0;
    sStr data_path_buf;
    for(idx idat=0; idat<req_data_tbl.rows; idat++) {
        temp_usage += req_data_tbl.ival(idat, 1);
        USAGETRACE_TYPE(sUsrUsage2::eTempUsage, "req %" UDEC " '%s' db has %" DEC, req_id, req_data_tbl.val(idat, 0), req_data_tbl.ival(idat, 1));
        const char * data_path_encoded = req_data_tbl.val(idat, 2);
        if( data_path_encoded && *data_path_encoded ) {
            data_path_buf.cut0cut();
            sString::decodeBase64(&data_path_buf, data_path_encoded, sLen(data_path_encoded));
            if( data_path_buf.length() > 7 ) {
                const char * data_path = data_path_buf.ptr(7);
                temp_usage += sFile::size(data_path);
                USAGETRACE_TYPE(sUsrUsage2::eTempUsage, "req %" UDEC " '%s' file has %" DEC, req_id, data_path, sFile::size(data_path));
            }
        }
    }
    return temp_usage;
}

sRC sUsrUsage2::getUsageChange(IncrementalUpdates * value_updates, udx user_id, udx primary_group_id, time_t since[eUsageTypeLast + 1], time_t to, progressCb cb, void * cb_param, idx iuser, idx num_users, bool find_deleted)
{
#if HAVE_USAGEDBG
    {
        static sStr time_buf;
        static sVariant time_val;

        time_buf.cutAddString(0, "since ");
        bool one_since = true;
        for(udx i=1; i<=eUsageTypeLast; i++) {
            if( since[i] != since[0] ) {
                one_since = false;
                break;
            }
        }
        if( one_since ) {
            time_val.setDateTime(since[0]);
            time_val.print(time_buf, sVariant::eUnquoted);
        } else {
            time_buf.addString("[");
            for(udx i=0; i<=eUsageTypeLast; i++) {
                if( i ) {
                    time_buf.addString(", ");
                }
                time_buf.printf("%s: ", sUsrUsage2::getTypeName((EUsageType)i));
                time_val.setDateTime(since[i]);
                time_val.print(time_buf, sVariant::eUnquoted);
            }
            time_buf.addString("]");
        }
        time_buf.addString(" to ");
        time_val.setDateTime(to);
        time_val.print(time_buf, sVariant::eUnquoted);

        USAGETRACE_FUNC("user %" DEC ", group %" DEC " %s", user_id, primary_group_id, time_buf.ptr());
    }
#endif

    idx progress_items = 0;

    if( since[eDiskUsage] != since[eFileCount] || since[eTempUsage] != since[eWaitTime] || since[eTempUsage] != since[eRunTime] ) {
        return RC(sRC::eChecking, sRC::eParameter, sRC::eTimeStamp, sRC::eNotEqual);
    }

    sDic<bool> reqs_dic;
    sVarSet reqs_tbl;
    sSql::sqlProc sql_finished_reqs(m_usr.db(), "sp_req_by_time");
    idx completed_reqs_cnt = sql_finished_reqs.Add(user_id).Add((idx)(since[eTempUsage])).Add((idx)to).Add("completed").getTable(&reqs_tbl);
    USAGETRACE("User %" UDEC " completed requests : %" DEC, user_id, completed_reqs_cnt);
    REPORT_PROGRESS;

    sSql::sqlProc sql_created_reqs(m_usr.db(), "sp_req_by_time");
    idx created_reqs_cnt = sql_created_reqs.Add(user_id).Add((idx)(since[eReqCount])).Add((idx)to).Add("created").getTable(&reqs_tbl);
    USAGETRACE("User %" UDEC " new created requests : %" DEC, user_id, created_reqs_cnt);
    REPORT_PROGRESS;

    sDic<bool> objs_dic;
    sVarSet objs_tbl;
    sSql::sqlProc sql_completed_objs(m_usr.db(), "sp_obj_by_time");
    idx completed_objs_cnt = sql_completed_objs.Add(user_id).Add(primary_group_id).Add((idx)(since[eCompletionTime])).Add((idx)to).Add("completed").getTable(&objs_tbl);
    USAGETRACE("User %" UDEC " completed objects : %" DEC, user_id, completed_objs_cnt);

    sSql::sqlProc sql_modified_objs(m_usr.db(), "sp_obj_by_time");
    idx modified_objs_cnt = sql_modified_objs.Add(user_id).Add(primary_group_id).Add((idx)(since[eDiskUsage])).Add((idx)to).Add("modified").getTable(&objs_tbl);
    USAGETRACE("User %" UDEC " modifed objects : %" DEC, user_id, modified_objs_cnt);
    REPORT_PROGRESS;

    sSql::sqlProc sql_created_objs(m_usr.db(), "sp_obj_by_time");
    idx created_objs_cnt = sql_created_objs.Add(user_id).Add(primary_group_id).Add((idx)(since[eObjCount])).Add((idx)to).Add("created").getTable(&objs_tbl);
    USAGETRACE("User %" UDEC " new created objects : %" DEC, user_id, created_objs_cnt);
    REPORT_PROGRESS;

    sStr path;
    sDir dir;

    for(idx ir = 0; ir < objs_tbl.rows; ir++) {
        sHiveId id(objs_tbl.uval(ir, 0), objs_tbl.uval(ir, 1), objs_tbl.uval(ir, 2));
        path.cut(0);
        dir.cut();

        if( objs_dic.get(&id, sizeof(id)) ) {
            continue;
        }

        sUsrObj * uobj = _admin->objFactory(id);
        sHiveId type_id = uobj ? uobj->getType()->id() : sHiveId::zero;
        if( !type_id ) {
            fprintf(stderr, "Object %s type ID could not be determined\n", id.print());
        }
        udx on_behalf_user_id = uobj ? uobj->propGetU("onUserBehalf") : 0;
        if( on_behalf_user_id ) {
            USAGETRACE("Object %s onUserBehalf %" UDEC, id.print(), on_behalf_user_id);
        }
        udx effective_user_id = on_behalf_user_id ? on_behalf_user_id : user_id;

        udx prev_user_id = _usage_db->getObjUser(id);
        udx prev_on_behalf_user_id = _usage_db->getObjOnBehalfUser(id);
        udx prev_effective_user_id = prev_on_behalf_user_id ? prev_on_behalf_user_id : prev_user_id;

        bool is_new_object = !_usage_db->hasObjInfo(id);

        sUsrUsage2::IncrementalUpdates::Record * rec = value_updates->ensureRecord(effective_user_id);
        sUsrUsage2::IncrementalUpdates::Record * prev_rec = prev_effective_user_id && prev_effective_user_id != effective_user_id ? value_updates->ensureRecord(prev_effective_user_id) : 0;

        if( is_new_object || prev_user_id != user_id || prev_on_behalf_user_id != on_behalf_user_id ) {
            _usage_db->setObjInfo(id, to, type_id, user_id, on_behalf_user_id);
        }

        if( is_new_object ) {
            rec->values[eObjCount]++;

            if( uobj && uobj->isTypeOf("^svc-computations-base$+") ) {
                rec->values[eComputationObjCount]++;
                _usage_db->setObjUsage2(id, to, eComputationObjCount, 1);
            }
            if( uobj && uobj->isTypeOf("^svc-data-loading-base$+") ) {
                rec->values[eDataLoadingObjCount]++;
                _usage_db->setObjUsage2(id, to, eDataLoadingObjCount, 1);
            }
            if( uobj && uobj->isTypeOf("^file$+") ) {
                rec->values[eFileObjCount]++;
                _usage_db->setObjUsage2(id, to, eFileObjCount, 1);
            }
            if( uobj && uobj->isTypeOf("^directory$+") ) {
                rec->values[eDirectoryObjCount]++;
                _usage_db->setObjUsage2(id, to, eDirectoryObjCount, 1);
            }
        } else if( prev_rec ) {
            rec->values[eObjCount]++;
            prev_rec->values[eObjCount]--;

            static EUsageType types_to_update[] = { eComputationObjCount, eDataLoadingObjCount, eFileObjCount, eDirectoryObjCount };
            for(idx it = 0; it < sDim(types_to_update); it++) {
                if( idx v = _usage_db->getObjUsage(id, types_to_update[it]) ) {
                    rec->values[types_to_update[it]] += v;
                    prev_rec->values[types_to_update[it]] -= v;
                }
            }
        }
        REPORT_PROGRESS;

        if( !sUsrObj::getPath(path, id) ) {
            USAGETRACE("obj %s has empty storage path", id.print());
        }

        idx disk_usage = 0, file_count = 0;
        idx prev_disk_usage = _usage_db->getObjUsage(id, eDiskUsage);
        idx prev_file_count = _usage_db->getObjUsage(id, eFileCount);

        dir.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs) | sFlag(sDir::bitRecursive), path);
        USAGETRACE("obj %s has %" DEC " entries in storage path %s", id.print(), dir.dimEntries(), path.ptr());
        for(idx ient = 0; ient < dir.dimEntries(); ient++) {
            disk_usage += sFile::size(dir.getEntryPath(ient));
            if( !(dir.getEntryFlags(ient) & sDir::fIsDir) ) {
                file_count++;
            }
            REPORT_PROGRESS;
        }
        USAGETRACE("obj %s has %" DEC " bytes in storage path %s", id.print(), disk_usage, path.ptr());

        _usage_db->setObjUsage2(id, to, eDiskUsage, disk_usage);
        _usage_db->setObjUsage2(id, to, eFileCount, file_count);

        rec->values[eDiskUsage] += disk_usage - prev_disk_usage;
        rec->values[eFileCount] += file_count - prev_file_count;
        USAGETRACE_TYPE(eDiskUsage, "obj %s has change %" DEC " (from %" DEC ")", id.print(), disk_usage - prev_disk_usage, prev_disk_usage);
        USAGETRACE_TYPE(eFileCount, "obj %s has change %" DEC " (from %" DEC ")", id.print(), file_count - prev_file_count, prev_file_count);
        if( prev_rec ) {
            rec->values[eDiskUsage] -= prev_disk_usage;
            rec->values[eFileCount] -= prev_file_count;
        }

        if( ir < completed_objs_cnt ) {
            idx started_time = objs_tbl.ival(ir, 3);
            idx completed_time = objs_tbl.ival(ir, 4);

            idx prev_completion_time = _usage_db->getObjUsage(id, eCompletionTime);
            if( completed_time >= started_time ) {
                USAGETRACE_TYPE(eCompletionTime, "obj %s has %" DEC " (started %" DEC ", completed %" DEC ")", id.print(), completed_time - started_time, started_time, completed_time);
                rec->values[eCompletionTime] += completed_time - started_time - prev_completion_time;
                if( prev_rec ) {
                    prev_rec->values[eCompletionTime] -= prev_completion_time;
                }
            } else {
                USAGETRACE_TYPE(eCompletionTime, "obj %s has started %" DEC " > completed %" DEC " - not recording", id.print(), started_time, completed_time);
            }
        }

        delete uobj;
        *objs_dic.set(&id, sizeof(id)) = true;
        REPORT_PROGRESS;
    }

    if( find_deleted ) {
        _admin->allowExpiredObjects(true);
        sVec<sHiveId> objs;
        _usage_db->getUserObjs(objs, user_id);
        for(idx i = 0; i < objs.dim(); i++) {
            if( objs_dic.get(objs.ptr(i), sizeof(sHiveId)) ) {
                continue;
            }

            if( sUsrObj * uobj = _admin->objFactory(objs[i]) ) {
                delete uobj;
            } else {
                USAGETRACE("obj %s was deleted - adjusting usage records", objs[i].print());
                udx prev_user_id = _usage_db->getObjUser(objs[i]);
                udx prev_on_behalf_user_id = _usage_db->getObjOnBehalfUser(objs[i]);
                udx prev_effective_user_id = prev_on_behalf_user_id ? prev_on_behalf_user_id : prev_user_id;
                sUsrUsage2::IncrementalUpdates::Record * prev_rec = prev_effective_user_id ? value_updates->ensureRecord(prev_effective_user_id) : 0;

                if( prev_rec ) {
                    static EUsageType types_to_update[] = { eComputationObjCount, eDataLoadingObjCount, eFileObjCount, eDirectoryObjCount, eDiskUsage, eFileCount, eCompletionTime };
                    for(idx it = 0; it < sDim(types_to_update); it++) {
                        prev_rec->values[types_to_update[it]] -= _usage_db->getObjUsage(objs[i], types_to_update[it]);
                    }
                    prev_rec->values[eObjCount]--;
                    _usage_db->delObjUsage(objs[i]);
                }
            }
            REPORT_PROGRESS;
        }
        _admin->allowExpiredObjects(false);
    }

    sStr sql_buf, req_par_obj_str;
    sVec<sHiveId> req_par_obj_ids;
    for(idx ir=0; ir<reqs_tbl.rows; ir++) {
        udx req_id = reqs_tbl.uval(ir, 0);

        if( reqs_dic.get(&req_id, sizeof(req_id)) ) {
            continue;
        }

        req_par_obj_str.cut0cut();
        req_par_obj_ids.cut(0);
        idx grp_id = _admin->QPride()->req2Grp(req_id);
        _admin->QPride()->requestGetPar(grp_id, sQPrideBase::eQPReqPar_Objects, &req_par_obj_str, false);
        sHiveId::parseRangeSet(req_par_obj_ids, req_par_obj_str.ptr());

        udx on_behalf_user_id = 0;
        if( req_par_obj_ids.dim() ) {
            sUsrObj * uobj = _admin->objFactory(req_par_obj_ids[0]);
            on_behalf_user_id = uobj ? uobj->propGetU("onUserBehalf") : 0;
            delete uobj;
            if( on_behalf_user_id ) {
                USAGETRACE("req %" UDEC " grp %" UDEC " obj %s has onUserBehalf %" UDEC, req_id, grp_id, req_par_obj_str.ptr(), on_behalf_user_id);
            }
        }
        udx effective_user_id = on_behalf_user_id ? on_behalf_user_id : user_id;

        REPORT_PROGRESS;

        udx prev_user_id = _usage_db->getReqUser(req_id);
        udx prev_on_behalf_user_id = _usage_db->getReqOnBehalfUser(req_id);
        udx prev_effective_user_id = prev_on_behalf_user_id ? prev_on_behalf_user_id : prev_user_id;

        bool is_new_req = !_usage_db->hasReqInfo(req_id);

        sUsrUsage2::IncrementalUpdates::Record * rec = value_updates->ensureRecord(effective_user_id);
        sUsrUsage2::IncrementalUpdates::Record * prev_rec = prev_effective_user_id && prev_effective_user_id != effective_user_id ? value_updates->ensureRecord(prev_effective_user_id) : 0;

        if( is_new_req || prev_user_id != user_id || prev_on_behalf_user_id != on_behalf_user_id ) {
            _usage_db->setReqInfo(req_id, to, user_id, on_behalf_user_id);
        }

        if( is_new_req ) {
            rec->values[eReqCount]++;
        } else if( prev_rec ) {
            rec->values[eReqCount]++;
            prev_rec->values[eReqCount]--;
        }

        if( ir < completed_reqs_cnt ) {
            idx created_time = reqs_tbl.ival(ir, 2);
            idx taken_time = reqs_tbl.ival(ir, 3);
            idx alive_time = reqs_tbl.ival(ir, 4);
            idx done_time = reqs_tbl.ival(ir, 5);

            idx wait_time = 0;
            idx run_time = 0;
            idx prev_wait_time = _usage_db->getReqUsage(req_id, eWaitTime);
            idx prev_run_time = _usage_db->getReqUsage(req_id, eRunTime);
            idx prev_temp_usage = _usage_db->getReqUsage(req_id, eTempUsage);

            if( taken_time > created_time ) {
                wait_time = taken_time - created_time;
                rec->values[eWaitTime] += wait_time;
                USAGETRACE_TYPE(eWaitTime, "req %" DEC " has %" DEC, req_id, wait_time);
                idx end_time = sMax<idx>(alive_time, done_time);
                if( end_time > taken_time ) {
                    run_time = end_time - taken_time;
                    rec->values[eRunTime] += run_time;
                    USAGETRACE_TYPE(eRunTime, "req %" DEC " has %" DEC, req_id, run_time);
                }
            }
            _usage_db->setReqUsage2(req_id, to, eWaitTime, wait_time);
            _usage_db->setReqUsage2(req_id, to, eRunTime, run_time);
            if( prev_rec ) {
                prev_rec->values[eWaitTime] -= prev_wait_time;
                prev_rec->values[eRunTime] -= prev_run_time;
            }

            idx temp_usage = getTempUsage(req_id, m_usr.db());
            REPORT_PROGRESS;
            _usage_db->setReqUsage2(req_id, to, eTempUsage, temp_usage);
            rec->values[eTempUsage] += temp_usage;
            if( prev_rec ) {
                prev_rec->values[eTempUsage] -= prev_temp_usage;
            }
        }


        *reqs_dic.set(&req_id, sizeof(req_id)) = true;
    }

    if( find_deleted ) {
        sVec<udx> req_ids;
        _usage_db->getUserReqs(req_ids, user_id);
        for(idx i = 0; i < req_ids.dim(); i++) {
            if( reqs_dic.get(req_ids.ptr(i), sizeof(udx)) ) {
                continue;
            }

            sQPrideBase::Request r;
            sSet(&r);
            if( _admin->QPride()->requestGet(req_ids[i], &r) ) {
                continue;
            } else {
                USAGETRACE("req %" UDEC " was deleted - adjusting usage records", req_ids[i]);
                udx prev_user_id = _usage_db->getReqUser(req_ids[i]);
                udx prev_on_behalf_user_id = _usage_db->getReqOnBehalfUser(req_ids[i]);
                udx prev_effective_user_id = prev_on_behalf_user_id ? prev_on_behalf_user_id : prev_user_id;
                sUsrUsage2::IncrementalUpdates::Record * prev_rec = prev_effective_user_id ? value_updates->ensureRecord(prev_effective_user_id) : 0;

                if( prev_rec ) {
                    static EUsageType types_to_update[] = { eWaitTime, eRunTime, eTempUsage };
                    for(idx it = 0; it < sDim(types_to_update); it++) {
                        prev_rec->values[types_to_update[it]] -= _usage_db->getReqUsage(req_ids[i], types_to_update[it]);
                    }
                    prev_rec->values[eReqCount]--;
                    _usage_db->delReqUsage(req_ids[i]);
                }
            }
            REPORT_PROGRESS;
        }
        _admin->allowExpiredObjects(false);
    }

    return sRC::zero;
}

sRC sUsrUsage2::updateIncremental(progressCb cb, void * cb_param, time_t at_time, bool find_deleted)
{
    if( !m_usr.isAdmin() || !m_usr.isAllowed(m_id, ePermCanWrite|ePermCanAdmin) ) {
        return RC(sRC::eChecking, sRC::ePermission, sRC::eUser, sRC::eNotAuthorized);
    }

    if( !_usage_db || !_usage_db->ok() ) {
        return RC(sRC::eChecking, sRC::eFile, sRC::ePointer, sRC::eNull);
    }

    bool need_upgrade = (_usage_db->dataVersion() < _usage_db->expectedVersion());
    if( need_upgrade ) {
        _usage_db->setSchemaPrefix(sUsrUsage2::UsageDb::eTemp);
    }

    IncrementalUpdates value_updates;
    time_t since[eUsageTypeLast + 1];
    sVarSet groups_tbl;
    if( !at_time ) {
        at_time = time(0);
    }
    m_usr.db().getTable("SELECT DISTINCT userID, groupID FROM UPGroup WHERE flags = -1", &groups_tbl);
    _usage_db->startTransaction();
    for(idx ig = 0; ig < groups_tbl.rows; ig++) {
        udx user_id = groups_tbl.uval(ig, 0);
        udx primary_group_id = groups_tbl.uval(ig, 1);
        for(idx i = 0; i <= eUsageTypeLast; i++) {
            since[i] = _usage_db->getLastTimestampBefore((time_t) sUdxMax, user_id, (EUsageType) i, 0);
            if( since[i] >= at_time ) {
                return RC(sRC::eChecking, sRC::eParameter, sRC::eTimeStamp, sRC::eTooSmall);
            }
        }
        if( sRC rc = getUsageChange(&value_updates, user_id, primary_group_id, since, at_time, cb, cb_param, ig, groups_tbl.rows, find_deleted) ) {
            _usage_db->rollback();
            return rc;
        }
    }
    for(idx ir = 0; ir < value_updates.dim(); ir++) {
        udx user_id = value_updates.ptr(ir)->user_id;
        for(idx i = 0; i <= eUsageTypeLast; i++) {
            _usage_db->addRecord(user_id, (EUsageType) i, 0, at_time, value_updates.ptr(ir)->values[i]);
        }
    }
    _usage_db->commit();

    if( need_upgrade ) {
        if( !_usage_db->upgradeMainData() ) {
            return RC(sRC::eUpdating, sRC::eDatabase, sRC::eOperation, sRC::eFailed);
        }
        _usage_db->setSchemaPrefix(sUsrUsage2::UsageDb::eMain);
    }

    return sRC::zero;
}

inline static udx purgedObj2User(const sDic<udx> & creator2user, const sUsrHousekeeper::PurgedObj * objs, idx i)
{
    const udx * puser = creator2user.get(&(objs[i].creator_id), sizeof(udx));
    return puser ? *puser : 0;
}

static idx cmpPurgedObj(void * param, void * arr, idx i1, idx i2)
{
    const sUsrHousekeeper::PurgedObj * objs = static_cast<const sUsrHousekeeper::PurgedObj *>(arr);
    const sDic<udx> * creator2user = static_cast<const sDic<udx> *>(param);

    udx user1 = purgedObj2User(*creator2user, objs, i1);
    udx user2 = purgedObj2User(*creator2user, objs, i2);

    return user1 < user2 ? -1 : user1 > user2 ? 1 : 0;
}

static idx cmpPurgedReq(void * param, void * arr, idx i1, idx i2)
{
    const sUsrHousekeeper::PurgedReq * reqs = static_cast<const sUsrHousekeeper::PurgedReq *>(arr);

    udx user1 = reqs[i1].user_id;
    udx user2 = reqs[i2].user_id;

    return user1 < user2 ? -1 : user1 > user2 ? 1 : 0;
}

sRC sUsrUsage2::updateDeleted(const sVec<sUsrHousekeeper::PurgedObj> & objs, const sVec<sUsrHousekeeper::PurgedReq> & reqs, progressCb cb, void * cb_param, time_t at_time)
{
    if( !m_usr.isAdmin() || !m_usr.isAllowed(m_id, ePermCanWrite|ePermCanAdmin) ) {
        return RC(sRC::eChecking, sRC::ePermission, sRC::eUser, sRC::eNotAuthorized);
    }

    if( !_usage_db || !_usage_db->ok() ) {
        return RC(sRC::eChecking, sRC::eFile, sRC::ePointer, sRC::eNull);
    }

    sVarSet groups_tbl;
    if( !at_time ) {
        at_time = time(0);
    }
    m_usr.db().getTable("SELECT DISTINCT userID, groupID FROM UPGroup WHERE flags = -1", &groups_tbl);
    sDic<udx> creator2user;
    for(idx ig=0; ig<groups_tbl.rows; ig++) {
        udx user_id = groups_tbl.uval(ig, 0);
        udx primary_group_id = groups_tbl.uval(ig, 1);
        *creator2user.set(&primary_group_id, sizeof(udx)) = user_id;
    }

    sVec<idx> objs_order(sMex::fExactSize), reqs_order(sMex::fExactSize);
    objs_order.resize(objs.dim());
    reqs_order.resize(reqs.dim());

    sSort::sortSimpleCallback(cmpPurgedObj, &creator2user, objs.dim(), const_cast<sUsrHousekeeper::PurgedObj*>(objs.ptr()), objs_order.ptr());
    sSort::sortSimpleCallback(cmpPurgedReq, 0, reqs.dim(), const_cast<sUsrHousekeeper::PurgedReq*>(reqs.ptr()), reqs_order.ptr());

    idx values[eUsageTypeLast + 1];
    sSetArray(values, 0);

    for(idx ireq=0, iobj=0; ireq < reqs.dim() || iobj < objs.dim(); ) {
        udx user_from_req = ireq < reqs.dim() ? reqs[ireq].user_id : 0;
        udx user_from_obj = iobj < objs.dim() ? purgedObj2User(creator2user, objs.ptr(), iobj) : 0;
        const udx user_id = sMin<udx>(user_from_req, user_from_obj);
        while( user_id == user_from_req ) {
            values[eReqCount]--;
            values[eTempUsage] -= _usage_db->getReqUsage(reqs[ireq].req_id, eTempUsage);
            _usage_db->delReqUsage(reqs[ireq].req_id);
            ireq++;
            user_from_req = ireq < reqs.dim() ? reqs[ireq].user_id : 0;
        }
        while( user_id == user_from_obj ) {
            values[eObjCount]--;
            values[eDiskUsage] -= _usage_db->getObjUsage(objs[iobj].hive_id, eDiskUsage);
            values[eFileCount] -= _usage_db->getObjUsage(objs[iobj].hive_id, eFileCount);
            _usage_db->delObjUsage(objs[iobj].hive_id);
            iobj++;
            user_from_obj = iobj < objs.dim() ? purgedObj2User(creator2user, objs.ptr(), iobj) : 0;
        }
        for(idx i=0; i<=eUsageTypeLast; i++) {
            if( values[i] ) {
                _usage_db->addRecord(user_id, (EUsageType)i, 0, at_time, values[i]);
            }
        }
        sSetArray(values, 0);
    }

    return sRC::zero;
}

static void formatGroupPath(sStr & out, const char * group_path)
{
    idx len = sLen(group_path);
    if( !len || group_path[len - 1] == '/' ) {
        out.addString(group_path);
    } else if( const char * slash = strrchr(group_path, '/') ) {
        out.addString(slash + 1);
    } else {
        out.addString(group_path);
    }
}

static void getUsersByGroupPath(sVec<sUsrUsage2::UserList> & user_lists, sDic<udx> & all_user_ids, const sDic<udx> & except_user_ids, sSql & db, const char * group_path, bool with_all_selected, bool expand_users)
{
    sStr sql, group_path_buf;
    sVarSet user_tbl;

    if( !group_path ) {
        group_path = "";
    }

    idx ihead = user_lists.dim();
    user_lists.add(1);
    user_lists[ihead].label.init(sMex::fExactSize);
    user_lists[ihead].user_ids.init(sMex::fExactSize);

    sql.cutAddString(0, "SELECT DISTINCT g1.userID AS userID, g2.groupPath AS groupPath FROM UPGroup g1 LEFT JOIN UPGroup g2 USING (userID) WHERE g2.flags = -1");
    if( strcmp(group_path, "*") == 0 ) {
        user_lists[ihead].label.addString(group_path);
    } else {
        group_path_buf.cutAddString(0, group_path);
        if( group_path_buf.length() && group_path_buf[group_path_buf.length() - 1] == '/' ) {
            user_lists[ihead].label.addString(group_path);
            group_path_buf.addString("%");
            sql.addString(" AND g1.groupPath LIKE ");
        } else {
            if( const char * slash = strrchr(group_path, '/') ) {
                user_lists[ihead].label.addString(slash + 1);
            } else {
                user_lists[ihead].label.addString(group_path);
            }
            sql.addString(" AND g1.groupPath = ");
        }
        db.protectValue(sql, group_path_buf);
    }
    if( except_user_ids.dim() ) {
        sVec<udx> except_user_ids_lst(sMex::fExactSize);
        except_user_ids_lst.resize(except_user_ids.dim());
        for(idx i = 0; i < except_user_ids.dim(); i++) {
            except_user_ids_lst[i] = *static_cast<const udx*>(except_user_ids.id(i));
        }
        sql.addString(" AND ");
        db.exprInList(sql, "g1.userID", except_user_ids_lst, true);
    }

    user_tbl.empty();
    db.getTable(sql, &user_tbl);
    user_lists[ihead].user_ids.resize(user_tbl.rows);
    for(idx ir = 0; ir < user_tbl.rows; ir++) {
        udx user_id = user_tbl.uval(ir, 0);
        user_lists[ihead].user_ids[ir] = user_id;
        if( with_all_selected ) {
            *all_user_ids.set(&user_id, sizeof(udx)) = user_id;
        }
        if( expand_users ) {
            sUsrUsage2::UserList * expand = user_lists.add(1);
            expand->label.init(sMex::fExactSize);
            formatGroupPath(expand->label, user_tbl.val(ir, 1));
            expand->user_ids.init(sMex::fExactSize);
            *expand->user_ids.add(1) = user_id;
        }
    }
}

static void getUserByUserID(sVec<sUsrUsage2::UserList> & user_lists, sDic<udx> & all_user_ids, const sDic<udx> & except_user_ids, sSql & db, udx userID, bool with_all_selected)
{
    sStr sql;
    sVarSet user_tbl;

    idx ihead = user_lists.dim();
    user_lists.add(1);
    user_lists[ihead].label.printf("userID %" UDEC, userID);
    user_lists[ihead].user_ids.init(sMex::fExactSize);

    sql.printf(0, "SELECT userID, email FROM UPUser WHERE userID = %" UDEC, userID);
    if( except_user_ids.dim() ) {
        sVec<udx> except_user_ids_lst(sMex::fExactSize);
        except_user_ids_lst.resize(except_user_ids.dim());
        for(idx i = 0; i < except_user_ids.dim(); i++) {
            except_user_ids_lst[i] = *static_cast<const udx*>(except_user_ids.id(i));
        }
        sql.addString(" AND ");
        db.exprInList(sql, "userID", except_user_ids_lst, true);
    }
    user_tbl.empty();
    db.getTable(sql, &user_tbl);
    if( user_tbl.rows ) {
        *user_lists[ihead].user_ids.add(1) = userID;
        if( with_all_selected ) {
            *all_user_ids.set(&userID, sizeof(userID)) = userID;
        }
    }
}

namespace {
    struct getUserByBillableGroupParam {
        sVec<udx> * user_ids;
        sDic<udx> added_user_ids;
        const sDic<udx> * except_user_ids;
    };
};

static sUsrObjPropsNode::FindStatus getUserByBillableGroupPropTree_findCb(const sUsrObjPropsNode& node, void * param_)
{
    getUserByBillableGroupParam * param = static_cast<getUserByBillableGroupParam*>(param_);

    sVariant var;
    if( !node.value(var) ) {
        return sUsrObjPropsNode::eFindError;
    }

    udx user_id = var.asUInt();

    if( !param->except_user_ids || !param->except_user_ids->get(&user_id, sizeof(user_id)) ) {
        if( !param->added_user_ids.get(&user_id, sizeof(user_id)) ) {
            *(param->user_ids->add(1)) = user_id;
            *param->added_user_ids.set(&user_id, sizeof(user_id)) = user_id;
        }
    }
    return sUsrObjPropsNode::eFindContinue;
}

static void getUserByBillableGroupPropTree(sUsrUsage2::UserList & user_list, const sDic<udx> & except_user_ids, const sUsrObjPropsTree * tree)
{
    getUserByBillableGroupParam param;
    param.user_ids = &user_list.user_ids;
    param.except_user_ids = &except_user_ids;
    idx cut_len = user_list.user_ids.dim();
    if( !tree->find("user", getUserByBillableGroupPropTree_findCb, &param) ) {
        user_list.user_ids.cut(cut_len);
    }
}

static void getUserByBillableGroupObj(sVec<sUsrUsage2::UserList> & user_lists, sDic<udx> & all_user_ids, const sDic<udx> & except_user_ids, const sUsr & user, sSql & db, const sHiveId & billable_id, bool with_all_selected, bool expand_users)
{
    idx ihead = user_lists.dim();
    user_lists.add(1);
    user_lists[ihead].label.addString("billable_group_obj ");
    billable_id.print(user_lists[ihead].label);

    sUsrObj * billable_obj = user.objFactory(billable_id);
    if( billable_obj && billable_obj->Id() ) {
        if( const sUsrObjPropsTree * tree = billable_obj->propsTree() ) {
            getUserByBillableGroupPropTree(user_lists[ihead], except_user_ids, tree);
        }
    }
    delete billable_obj;
    billable_obj = 0;

    if( with_all_selected ) {
        for(idx i = 0; i < user_lists[ihead].user_ids.dim(); i++) {
            *all_user_ids.set(user_lists[ihead].user_ids.ptr(i), sizeof(udx)) = user_lists[ihead].user_ids[i];
        }
    }
    if( expand_users ) {
        for(idx i = 0; i < user_lists[ihead].user_ids.dim(); i++) {
            getUserByUserID(user_lists, all_user_ids, except_user_ids, db, user_lists[ihead].user_ids[i], false);
        }
    }
}

static void getUserByBillableGroupName(sVec<sUsrUsage2::UserList> & user_lists, sDic<udx> & all_user_ids, const sDic<udx> & except_user_ids, const sUsr & user, sSql & db, const char * billable_name, bool with_all_selected, bool expand_users)
{
    if( !billable_name ) {
        billable_name = "";
    }

    idx ihead = user_lists.dim();
    user_lists.add(1);
    user_lists[ihead].label.addString("billable_group_name ");
    user_lists[ihead].label.addString(billable_name);

    sUsrObjRes obj_res;
    sHiveId billable_id;
    user.objs2("HIVE_Development_Billable_Group", obj_res, 0, "name", billable_name, "name");
    for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
        const char * name = obj_res.getValue(obj_res.get(*obj_res.get(it), "name"));
        if( name && sIsExactly(name, billable_name) ) {
            billable_id = *obj_res.id(it);
            break;
        }
    }

    sUsrObj * billable_obj = user.objFactory(billable_id);
    if( billable_obj && billable_obj->Id() ) {
        if( const sUsrObjPropsTree * tree = billable_obj->propsTree() ) {
            getUserByBillableGroupPropTree(user_lists[ihead], except_user_ids, tree);
        }
    }
    delete billable_obj;
    billable_obj = 0;

    if( with_all_selected ) {
        for(idx i = 0; i < user_lists[ihead].user_ids.dim(); i++) {
            *all_user_ids.set(user_lists[ihead].user_ids.ptr(i), sizeof(udx)) = user_lists[ihead].user_ids[i];
        }
    }
    if( expand_users ) {
        for(idx i = 0; i < user_lists[ihead].user_ids.dim(); i++) {
            getUserByUserID(user_lists, all_user_ids, except_user_ids, db, user_lists[ihead].user_ids[i], false);
        }
    }
}

static void getUsersBySpecs(sVec<sUsrUsage2::UserList> & user_lists, sDic<udx> & all_user_ids, const sUsr & user, sSql & db, sUsrUsage2::GroupSpec * group_specs, idx num_group_specs, bool with_all_selected, bool expand_users)
{
    sHiveId billable_id;

    for(idx ispec = 0; ispec < num_group_specs; ispec++) {
        sUsrUsage2::GroupSpec & spec = group_specs[ispec];

        sDic<udx> except_user_ids;
        sStr except_label;

        if( spec.except.dim() ) {
            sVec<sUsrUsage2::UserList> dummy_except_user_lists;
            getUsersBySpecs(dummy_except_user_lists, except_user_ids, user, db, spec.except.ptr(), spec.except.dim(), true, false);
            if( dummy_except_user_lists.dim() ) {
                if( except_label ) {
                    except_label.addString(" ");
                }
                except_label.addString(dummy_except_user_lists[0].label.ptr());
            }
        }

        idx ihead = user_lists.dim();

        switch(spec.kind) {
            case sUsrUsage2::GroupSpec::eGroupPath:
                getUsersByGroupPath(user_lists, all_user_ids, except_user_ids, db, spec.value.asString(), with_all_selected, expand_users);
                break;
            case sUsrUsage2::GroupSpec::eUserID:
                getUserByUserID(user_lists, all_user_ids, except_user_ids, db, spec.value.asUInt(), with_all_selected);
                break;
            case sUsrUsage2::GroupSpec::eBillableGroupObj:
                spec.value.asHiveId(&billable_id);
                getUserByBillableGroupObj(user_lists, all_user_ids, except_user_ids, user, db, billable_id, with_all_selected, expand_users);
                break;
            case sUsrUsage2::GroupSpec::eBillableGroupName:
                getUserByBillableGroupName(user_lists, all_user_ids, except_user_ids, user, db, spec.value.asString(), with_all_selected, expand_users);
                break;
        }

        if( ihead < user_lists.dim() && except_label.length() ) {
            user_lists[ihead].label.addString(" except ");
            user_lists[ihead].label.addString(except_label.ptr());
        }
    }
}

sRC sUsrUsage2::exportGroupsTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, sUsrUsage2::GroupSpec * group_specs, idx num_group_specs, bool with_all_selected, bool expand_users) const
{
    if( !m_usr.isAllowed(m_id, ePermCanRead) ) {
        return RC(sRC::eAccessing, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
    }

    sDic<udx> all_user_ids;
    sVec<UserList> user_lists;

    getUsersBySpecs(user_lists, all_user_ids, m_usr, m_usr.db(), group_specs, num_group_specs, with_all_selected, expand_users);

    if( with_all_selected ) {
        UserList * user_list = user_lists.add(1);
        user_list->label.init(sMex::fExactSize);
        user_list->label.addString("All selected users/groups");
        user_list->user_ids.init(sMex::fExactSize);
        user_list->user_ids.resize(all_user_ids.dim());
        for(idx iu = 0; iu < all_user_ids.dim(); iu++) {
            user_list->user_ids[iu] = *all_user_ids.ptr(iu);
        }
    }

    return exportTable(out, usage_type, since, to, user_lists.ptr(), user_lists.dim());
}

sRC sUsrUsage2::exportGroupsTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const char * group_paths00, bool with_all_selected) const
{
    if( !m_usr.isAllowed(m_id, ePermCanRead) ) {
        return RC(sRC::eAccessing, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
    }

    sStr sql, group_path_buf;
    sVec<udx> user_ids;
    sDic<udx> all_user_ids;
    sVec<UserList> user_lists;
    sVarSet user_tbl;
    sDic<udx> dummy_except_user_ids;

    for(const char * group_path = group_paths00; group_path && *group_path; group_path = sString::next00(group_path) ) {
        getUsersByGroupPath(user_lists, all_user_ids, dummy_except_user_ids, m_usr.db(), group_path, with_all_selected, false);
    }

    if( with_all_selected ) {
        UserList * user_list = user_lists.add(1);
        user_list->label.init(sMex::fExactSize);
        user_list->label.addString("All selected users/groups");
        user_list->user_ids.init(sMex::fExactSize);
        user_list->user_ids.resize(all_user_ids.dim());
        for(idx iu = 0; iu < all_user_ids.dim(); iu++) {
            user_list->user_ids[iu] = *all_user_ids.ptr(iu);
        }
    }

    return exportTable(out, usage_type, since, to, user_lists.ptr(), user_lists.dim());
}

struct ExportTableWorker
{
    sTxtTbl & out;
    idx dim;

    sStr buf;
    bool have_cached;
    idx cnt_rows_printed;

    time_t timestamp;
    sVec<idx> values;
    sVec<idx> sums;
    sVec<idx> * psum_adjust;

    bool err;

    ExportTableWorker(sTxtTbl & out_, idx dim_) : out(out_)
    {
        dim = dim_;

        values.init(sMex::fExactSize|sMex::fSetZero);
        values.resize(dim);
        sums.init(sMex::fExactSize|sMex::fSetZero);
        sums.resize(dim);

        have_cached = false;
        cnt_rows_printed = 0;
        timestamp = -sIdxMax;
        psum_adjust = 0;
        err = false;
    }

    void flush()
    {
        if( have_cached ) {
            buf.cut(0);
            sString::printDateTime(buf, timestamp, sString::fISO8601);
            if( !out.addCell(buf.ptr(), buf.length(), sVariant::value_DATE_TIME) ) {
                err = true;
            }
            for( idx i=0; i<dim; i++ ) {
                if( !out.addICell(values[i]) ) {
                    err = true;
                }

                if( cnt_rows_printed == 0 && psum_adjust ) {
                    sums[i] += *psum_adjust->ptr(i);
                }
                if( !out.addICell(sums[i]) ) {
                    err = true;
                }

                values[i] = 0;
            }
            if( !out.addEndRow() ) {
                err = true;
            }
            cnt_rows_printed++;
        }
        have_cached = false;
    }

    void addFinalRow(time_t to)
    {
        flush();
        if( cnt_rows_printed && timestamp < to ) {
            timestamp = to;
            for(idx i = 0; i < dim; i++) {
                values[i] = 0;
            }
            have_cached = true;
            flush();
        }
    }

    static bool callback(slib::sUsrUsage2::EUsageType type, idx list_index, idx tag, time_t timestamp, idx value, sVec<idx> * psum_adjust_, void * param)
    {
        ExportTableWorker * self = static_cast<ExportTableWorker*>(param);

        if( psum_adjust_ ) {
            self->psum_adjust = psum_adjust_;
        }
        if( self->have_cached && timestamp == self->timestamp ) {
            self->values[list_index] += value;
            self->sums[list_index] += value;
        } else {
            self->flush();
            self->have_cached = true;
            self->timestamp = timestamp;
            self->values[list_index] = value;
            self->sums[list_index] += value;
        }

        return !self->err;
    }
};

sRC sUsrUsage2::exportTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const sUsrUsage2::UserList * user_lists, idx num_user_lists) const
{
    if( !m_usr.isAllowed(m_id, ePermCanRead) ) {
        return RC(sRC::eAccessing, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
    }

    enum {
        eFalse,
        eTrue,
        eUnknown = -1
    } can_see_others = eUnknown;

    for(idx il=0; il<num_user_lists; il++) {
        if( user_lists[il].user_ids.dim() > 1 || (user_lists[il].user_ids.dim() == 1 && user_lists[il].user_ids[0] != m_usr.Id())) {
            if( can_see_others == eUnknown ) {
                can_see_others = (m_usr.isAdmin() || m_usr.hasGroup("/Projects/Team/")) ? eTrue : eFalse;
            }
            if( can_see_others != eTrue ) {
               return RC(sRC::eAccessing, sRC::eUser, sRC::eUser, sRC::eNotAuthorized);
            }
        }
    }

    if( !out.initWritable(1 + 2 * num_user_lists, 0) ) {
        return RC(sRC::eInitializing, sRC::eTable, sRC::eMode, sRC::eReadOnly);
    }

    out.addCell("");
    sStr quoted_buf;
    for(idx il=0; il<num_user_lists; il++) {
        quoted_buf.cut(0);
        sString::escapeForCSV(quoted_buf, user_lists[il].label);
        out.addCell(quoted_buf);
        out.addCell(quoted_buf);
    }
    out.addEndRow();

    idx ihdr = 0;
    out.addCell("timestamp");
    out.setColtype(ihdr++, sVariant::value_DATE_TIME);
    for(idx il=0; il<num_user_lists; il++) {
        out.addCell(usage_type_names[usage_type].column_change);
        out.setColtype(ihdr++, sVariant::value_INT);

        out.addCell(usage_type_names[usage_type].column_total);
        out.setColtype(ihdr++, sVariant::value_INT);
    }
    out.addEndRow();

    out.setDimTopHeader(2);

    if( !_usage_db || !_usage_db->ok() ) {
        return RC(sRC::eReading, sRC::eData, sRC::eFile, sRC::eNotFound);
    }

    ExportTableWorker worker(out, num_user_lists);
    _usage_db->readRows(user_lists, num_user_lists, usage_type, 0, since, to, worker.callback, &worker);
    if( time_t final_row_timestamp = _usage_db->getLastTimestampBefore(to, usage_type, 0) ) {
        worker.addFinalRow(final_row_timestamp);
    }

    if( worker.err ) {
        return RC(sRC::eWriting, sRC::eTable, sRC::eOperation, sRC::eFailed);
    }

    return sRC::zero;
}

sUsrUsage2::sUsrUsage2(const sUsr& user, const sHiveId & objId, sUsr * admin) : sUsrObj(user, objId)
{
    sStr db_path;
    bool can_admin = m_usr.isAdmin() && m_usr.isAllowed(m_id, ePermCanWrite|ePermCanAdmin);
    bool can_read = m_usr.isAllowed(m_id, ePermCanRead);
    _usage_db = 0;
    _admin = 0;

    if( can_admin ) {
        if( getFilePathname(db_path, "usage.sqlite") || addFilePathname(db_path, false, "usage.sqlite") ) {
            USAGETRACE("Opening usage.sqlite for writing");
            _usage_db = new sUsrUsage2::UsageDb(db_path, false);
            if( _usage_db->ok() ) {
                _admin = admin;
            } else {
                USAGETRACE("Failed to read usage.sqlite");
                delete _usage_db;
                _usage_db = 0;
            }

        } else {
            USAGETRACE("Refusing to open usage.sqlite for writing");
        }
    } else if( can_read && getFilePathname(db_path, "usage.sqlite") ) {
        USAGETRACE("Opening usage.sqlite for reading");
        _usage_db = new sUsrUsage2::UsageDb(db_path, true);
        if( !_usage_db->ok() ) {
            USAGETRACE("Failed to read usage.sqlite");
            delete _usage_db;
            _usage_db = 0;
        }
    }
}

sUsrUsage2::~sUsrUsage2()
{
    delete _usage_db;
}
