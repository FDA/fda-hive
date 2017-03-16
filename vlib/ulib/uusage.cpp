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
#include <slib/std/file.hpp>
#include <slib/std/pipe2.hpp>
#include <slib/utils/sort.hpp>
#include <ulib/uusage.hpp>
#include <qlib/QPrideBase.hpp>
#include <xlib/sqlite.hpp>

#include "uperm.hpp"

#include <assert.h>

using namespace slib;

#if _DEBUG_off
#define HAVE_USAGEDBG 1
#define USAGEDBG(fmt, ...) fprintf(stderr, fmt"\n", ##__VA_ARGS__)
#define USAGEDBG_FUNC(fmt, ...) fprintf(stderr, "%s:%d : "fmt"\n", __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define USAGEDBG_TYPE(type, fmt, ...) fprintf(stderr, "%s : "fmt"\n", sUsrUsage2::getTypeName(type), ##__VA_ARGS__)
#else
#define HAVE_USAGEDBG 0
#define USAGEDBG(...)
#define USAGEDBG_FUNC(...)
#define USAGEDBG_TYPE(...)
#endif

#define USAGEDBG_SQLITE do { if( _db.hasFailed() ) { fprintf(stderr, "%s:%d : SQLite error %"UDEC": %s\n", __PRETTY_FUNCTION__, __LINE__, _db.getErrno(), _db.getError()); } } while( 0 )
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
    { "completion-time", "process completion time", "total process completion time" }
};

//static
const char * sUsrUsage2::getTypeName(EUsageType t)
{
    if( likely(t >= 0 && t <= eUsageTypeLast) ) {
        return usage_type_names[t].filename;
    }
    return 0;
}

//static
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

static void ionapp2csv(sTxtTbl & out_tbl, sIO & out_io, const char * ion_path, const char * relname)
{
    sPipe2 ionapp;
    sIO ionapp_io;
    ionapp.setExe("ionapp.os" SLIB_PLATFORM).addArg("-ionRead").addArg(ion_path).addArg("-ionExport").addArg(relname).setStdOut(&out_io);
    ionapp.execute();

    if( out_io.length() ) {
        out_tbl.parseOptions().comment = "# ";
        out_tbl.setBuf(out_io.ptr(), out_io.length());
        out_tbl.parse();
        USAGEDBG_SQLITE_TRACE("%"DEC" rows read from ionapp -ionRead %s -ionExport %s", out_tbl.rows(), ion_path, relname);
    }
}

static unsigned char readHexDigit(char c)
{
    unsigned char ret = 0;
    if( c >= '0' && c <= '9' ) {
        ret += c - '0';
    } else if( c >= 'a' && c <= 'f' ) {
        ret += c - 'a' + 10;
    } else if( c >= 'A' && c <= 'F' ) {
        ret += c - 'A' + 10;
    }
    return ret;
}

// read two hex digits as a byte
static unsigned char readHexByte(const char * txt)
{
    return (readHexDigit(txt[0]) << 4) + readHexDigit(txt[1]);
}

static bool noCSVErrorCells(sTxtTbl & tbl, idx irow, sStr & buf)
{
    for(idx ic = 0; ic < tbl.cols(); ic++) {
        buf.cut0cut();
        tbl.printCell(buf, irow, ic);
        if( strcasestr(buf.ptr(), "err") ) {
            return false;
        }
    }
    return true;
}

class sUsrUsage2::UsageDb
{
    private:
        sSqlite _db;

        void initSchema()
        {
            if( _db.startTransaction() ) {
                if( // user_usage table: historical log for each (user, type, tag) tuple, including historical data
                    // rowid (explicit to use as foreign key) / timestamp / user_id / usage_type (EUsageType) / value / tag
                    _db.executeExact("CREATE TABLE user_usage (rowid INTEGER PRIMARY KEY AUTOINCREMENT, timestamp BIGINT NOT NULL, user_id BIGINT NOT NULL, usage_type INTEGER NOT NULL, value BIGINT NOT NULL, tag BIGINT);") &&
                    _db.executeExact("CREATE INDEX user_usage_idx_timestamp_asc ON user_usage (timestamp ASC);") &&
                    _db.executeExact("CREATE INDEX user_usage_idx_timestamp_desc ON user_usage (timestamp DESC);") &&
                    _db.executeExact("CREATE INDEX user_usage_idx_type_tag ON user_usage (usage_type, tag);") &&
                    // req_usage table: current usage for each (req, type, tag) tuple, without historical data
                    // timestamp / req_id / usage_type / value (+ sqlite's implicit rowid column)
                    _db.executeExact("CREATE TABLE req_usage (timestamp BIGINT NOT NULL, req_id BIGINT NOT NULL, usage_type INTEGER NOT NULL, value BIGINT NOT NULL);") &&
                    _db.executeExact("CREATE UNIQUE INDEX req_usage_idx ON req_usage (req_id, usage_type);") &&
                    // obj_usage table: current usage for each (obj, type, tag) tuple, without historical data
                    // timestamp / domain_id / obj_id / usage_type / value (+ sqlite's implicit rowid column)
                    _db.executeExact("CREATE TABLE obj_usage (timestamp BIGINT NOT NULL, domain_id BIGINT NOT NULL, obj_id BIGINT NOT NULL, usage_type INTEGER NOT NULL, value BIGINT NOT NULL);") &&
                    _db.executeExact("CREATE UNIQUE INDEX obj_usage_idx ON obj_usage (domain_id, obj_id, usage_type);") ) {
                    _db.commit();
                } else {
                    USAGEDBG_SQLITE;
                    _db.rollback();
                }
            }
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
        UsageDb(const char * path, bool readonly = false) : _db(path, readonly)
        {
            if( _db.ok() && !readonly && !_db.executeExact("SELECT COUNT(*) FROM user_usage;") ) {
                initSchema();
            }
            _db.executeExact("PRAGMA journal_mode = WAL;"); // nfs optimization
            _db.executeExact("PRAGMA secure_delete = FALSE;"); // nfs optimization
        }

        bool ok() const { return _db.ok(); }
        bool startTransaction() { return _db.startTransaction(); }
        bool commit() { return _db.commit(); }
        bool rollback() { return _db.rollback(); }

        idx dimHistoryRecords()
        {
            idx ret = 0;
            if( _db.ok() && _db.resultOpenExact("SELECT COUNT(*) FROM user_usage;") ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        //! this MUST be called in chronological order (earliest to latest) - it's assumed timestamps ascend with rowid!
        void addRecord(udx user_id, sUsrUsage2::EUsageType type, idx tag, time_t timestamp, idx value)
        {
            udx timestamp_udx = timestamp;
            bool timestamp_is_last = true;
            if( _db.resultOpen("SELECT rowid FROM user_usage WHERE user_id = %"UDEC" and usage_type = %d AND tag = %"DEC" and timestamp > %"UDEC" LIMIT 1;", user_id, type, tag, timestamp_udx) ) {
                if( _db.resultNextRow() ) {
                    timestamp_is_last = false;
                }
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }

            if( timestamp_is_last ) {
                if( _db.resultOpen("SELECT rowid, value, timestamp FROM user_usage WHERE user_id = %"UDEC" and usage_type = %d AND TAG = %"DEC" and timestamp <= %"UDEC" ORDER BY timestamp DESC LIMIT 2;", user_id, type, tag, timestamp_udx) ) {
                    idx cnt_same_value = 0;
                    idx update_rowid = -sIdxMax;
                    if( _db.resultNextRow() ) {
                        update_rowid = _db.resultIValue(0);
                        if( _db.resultIValue(1) == value ) {
                            if( _db.resultUValue(2) == timestamp_udx ) {
                                // no-op: the latest row in the database is *exactly equal* to what we are trying to write : same user, type, tag, timestamp, value
                                // (this can legitimately happen only during migration from ion)
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
                        // optimization: the latest 2 entries for (user, type, tag) have the same value; we just
                        // need to update the second one's timestamp
                        if( !_db.execute("UPDATE user_usage SET timestamp = %"UDEC" WHERE rowid = %"DEC";", timestamp_udx, update_rowid) ) {
                            USAGEDBG_SQLITE;
                        }
                        return;
                    }
                } else {
                    USAGEDBG_SQLITE;
                }
            }

            if( !_db.execute("INSERT INTO user_usage (user_id, usage_type, tag, timestamp, value) VALUES(%"UDEC", %d, %"DEC", %"UDEC", %"DEC");", user_id, (int)type, tag, timestamp_udx, value) ) {
                USAGEDBG_SQLITE;
            }
        }

        time_t getFirstTimestampAfter(time_t after, sUsrUsage2::EUsageType type, idx tag)
        {
            idx ret = sIdxMax;
            udx after_udx = after;
            if( _db.resultOpen("SELECT timestamp FROM user_usage WHERE usage_type = %d AND tag = %"DEC" AND timestamp >= %"UDEC" ORDER BY timestamp ASC LIMIT 1;", type, tag, after_udx) ) {
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
            if( _db.resultOpen("SELECT timestamp FROM user_usage WHERE user_id = %"UDEC" AND usage_type = %d AND tag = %"DEC" AND timestamp <= %"UDEC" ORDER BY timestamp DESC LIMIT 1;", user_id, type, tag, before_udx) ) {
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
            if( _db.resultOpen("SELECT timestamp FROM user_usage WHERE usage_type = %d AND tag = %"DEC" AND timestamp <= %"UDEC" ORDER BY timestamp DESC LIMIT 1;", type, tag, before_udx) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        typedef bool (*readCallback)(sUsrUsage2::EUsageType type, idx list_index, idx tag, time_t timestamp, idx value, idx sum_adjust, void * param);

        idx readRows(const sUsrUsage2::UserList * user_lists, idx num_user_lists, sUsrUsage2::EUsageType type, idx tag, time_t since, time_t to, readCallback cb, void * cb_param)
        {
            if( !num_user_lists ) {
                return 0;
            }

            sVec<udx> all_user_ids;
            sMex user_id2info_mex;
            sDic<UInfo> user_id2info;
            for(idx i = 0; i < num_user_lists; i++) {
                for(idx j = 0; j < user_lists[i].dim; j++) {
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
            sStr sql("SELECT timestamp, user_id, value FROM user_usage WHERE usage_type = %d AND tag = %"DEC" AND timestamp <= %"UDEC" AND ", type, tag, to_udx);
            sSql::exprInList(sql, "user_id", all_user_ids);
            sql.addString(" ORDER BY timestamp ASC;");

            if( _db.resultOpenExact(sql.ptr()) ) {
                while( _db.resultNextRow() ) {
                    time_t timestamp = (time_t)_db.resultUValue(0);
                    udx user_id = _db.resultUValue(1);
                    idx cur_value = _db.resultIValue(2);

                    UInfo * uinfo = user_id2info.get(&user_id, sizeof(user_id));

                    for(idx iil=0; uinfo && iil<uinfo->iuser_lists.dim(); iil++) {
                        const idx il = uinfo->iuser_lists[iil]; // index into user_lists and iuser_list2start_value
                        if( timestamp < since ) {
                            iuser_list2start_value[il] += cur_value;
                            continue;
                        } else if( timestamp > to ) {
                            break;
                        } else {
                            bool cb_result = false;
                            if( unlikely(timestamp == first_timestamp) ) {
                                // treat first records in db as a historical baseline, not as a change relative to previous
                                assert(iuser_list2start_value[il] == 0);
                                cb_result = cb(type, il, tag, timestamp, 0, cur_value, cb_param);
                            } else if( unlikely(!iuser_list2cnt_callbacks[il]) ) {
                                cb_result = cb(type, il, tag, timestamp, cur_value, iuser_list2start_value[il], cb_param);
                            } else {
                                cb_result = cb(type, il, tag, timestamp, cur_value, 0, cb_param);
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
            if( _db.resultOpen("SELECT value FROM obj_usage WHERE domain_id = %"UDEC" AND obj_id = %"UDEC" AND usage_type = %d;", id.domainId(), id.objId(), type) ) {
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
            if( _db.resultOpen("SELECT value FROM req_usage WHERE req_id = %"UDEC" AND usage_type = %d;", req, type) ) {
                _db.resultNextRow();
                ret = _db.resultIValue(0, 0);
                _db.resultClose();
            } else {
                USAGEDBG_SQLITE;
            }
            return ret;
        }

        void setObjUsage(const sHiveId & id, time_t timestamp, idx disk_usage, idx file_count)
        {
            udx timestamp_udx = timestamp;
            if( !_db.execute("INSERT OR REPLACE INTO obj_usage (timestamp, domain_id, obj_id, usage_type, value) VALUES(%"UDEC", %"UDEC", %"UDEC", %d, %"DEC");", timestamp_udx, id.domainId(), id.objId(), sUsrUsage2::eDiskUsage, disk_usage) ) {
                USAGEDBG_SQLITE;
            }
            if( !_db.execute("INSERT OR REPLACE INTO obj_usage (timestamp, domain_id, obj_id, usage_type, value) VALUES(%"UDEC", %"UDEC", %"UDEC", %d, %"DEC");", timestamp_udx, id.domainId(), id.objId(), sUsrUsage2::eFileCount, file_count) ) {
                USAGEDBG_SQLITE;
            }
        }

        void setReqUsage(const udx req, time_t timestamp, idx temp_usage)
        {
            udx timestamp_udx = timestamp;
            if( !_db.execute("INSERT OR REPLACE INTO req_usage (timestamp, req_id, usage_type, value) VALUES(%"UDEC", %"UDEC", %d, %"DEC");", timestamp_udx, req, sUsrUsage2::eTempUsage, temp_usage) ) {
                USAGEDBG_SQLITE;
            }
        }

        void delObjUsage(const sHiveId & id)
        {
            if( !_db.execute("DELETE FROM obj_usage WHERE domain_id = %"UDEC" AND obj_id = %"UDEC";", id.domainId(), id.objId()) ) {
                USAGEDBG_SQLITE;
            }
        }

        void delReqUsage(const udx req)
        {
            if( !_db.execute("DELETE FROM obj_usage WHERE req_id = %"UDEC";", req) ) {
                USAGEDBG_SQLITE;
            }
        }

        void migrateFromIon(const char * usage_db_path)
        {
            sFilePath usage_ion_path(usage_db_path, "%%dir/usage.ion");
            sFilePath prev_vals_ion_path(usage_db_path, "%%dir/prev_vals.ion");
            usage_ion_path.shrink00();
            prev_vals_ion_path.shrink00();
            if( !sFile::exists(usage_ion_path) || !sFile::exists(prev_vals_ion_path) ) {
                return;
            }
            USAGEDBG_SQLITE_TRACE("Migrating from %s and %s to %s", usage_ion_path.ptr(), prev_vals_ion_path.ptr(), usage_db_path);

            sTxtTbl normal_rows_tbl, end_rows_tbl, obj_usage_tbl, req_usage_tbl;
            sIO normal_rows_io, end_rows_io, obj_usage_io, req_usage_io;
            sStr print_buf;

            // remove ".ion" extension for ionapp
            usage_ion_path.cut0cut(usage_ion_path.length() - 4);
            prev_vals_ion_path.cut0cut(prev_vals_ion_path.length() - 4);

            ionapp2csv(normal_rows_tbl, normal_rows_io, usage_ion_path, "normal-row");
            ionapp2csv(end_rows_tbl, end_rows_io, usage_ion_path, "end-row");
            ionapp2csv(obj_usage_tbl, obj_usage_io, prev_vals_ion_path, "obj-usage");
            ionapp2csv(req_usage_tbl, req_usage_io, prev_vals_ion_path, "req-usage");

            if( startTransaction() ) {
                USAGEDBG_SQLITE_TRACE("Migrating %"DEC" user usage records ...", normal_rows_tbl.rows());
                for(idx ir = 0; ir < normal_rows_tbl.rows(); ir++) {
                    if( !noCSVErrorCells(normal_rows_tbl, ir, print_buf) ) {
                        USAGEDBG_SQLITE_TRACE("Skipping row %"DEC" : %s", ir, normal_rows_tbl.printCSV(print_buf, ir, 0, 1));
                        continue;
                    }
                    // time, value, user-id, usage-type, tag
                    udx user_id = normal_rows_tbl.uval(ir, normal_rows_tbl.colId("user-id"));
                    sUsrUsage2::EUsageType type = (sUsrUsage2::EUsageType)normal_rows_tbl.ival(ir, normal_rows_tbl.colId("usage-type"));
                    idx tag = normal_rows_tbl.uval(ir, normal_rows_tbl.colId("tag"));
                    time_t timestamp = (time_t)normal_rows_tbl.uval(ir, normal_rows_tbl.colId("time"));
                    idx value = normal_rows_tbl.uval(ir, normal_rows_tbl.colId("value"));

                    addRecord(user_id, type, tag, timestamp, value);
                }
                USAGEDBG_SQLITE_TRACE("Migrating %"DEC" user usage end records ...", end_rows_tbl.rows());
                for(idx ir = 0; ir < end_rows_tbl.rows(); ir++) {
                    if( !noCSVErrorCells(end_rows_tbl, ir, print_buf) ) {
                        USAGEDBG_SQLITE_TRACE("Skipping row %"DEC" : %s", ir, end_rows_tbl.printCSV(print_buf, ir, 0, 1));
                        continue;
                    }
                    // unhashed-time, unhashed-value, user-id, usage-type, tag
                    udx user_id = end_rows_tbl.uval(ir, end_rows_tbl.colId("user-id"));
                    sUsrUsage2::EUsageType type = (sUsrUsage2::EUsageType)end_rows_tbl.ival(ir, end_rows_tbl.colId("usage-type"));
                    idx tag = end_rows_tbl.uval(ir, end_rows_tbl.colId("tag"));
                    time_t timestamp = (time_t)end_rows_tbl.uval(ir, end_rows_tbl.colId("unhashed-time"));
                    idx value = end_rows_tbl.uval(ir, end_rows_tbl.colId("unhashed-value"));

                    addRecord(user_id, type, tag, timestamp, value);
                }
                USAGEDBG_SQLITE_TRACE("done.\nMigrating %"DEC" object usage records ...", obj_usage_tbl.rows());
                sStr id_encoded;
                for(idx ir = 0; ir < obj_usage_tbl.rows(); ir++) {
                    if( !noCSVErrorCells(obj_usage_tbl, ir, print_buf) ) {
                        USAGEDBG_SQLITE_TRACE("Skipping row %"DEC" : %s", ir, obj_usage_tbl.printCSV(print_buf, ir, 0, 1));
                        continue;
                    }
                    // obj, time, disk-usage, file-count
                    id_encoded.cut0cut();
                    obj_usage_tbl.printCell(id_encoded, ir, obj_usage_tbl.colId("obj"));
                    // obj is hex-encoded binary dump of sHiveId structure :(
                    sHiveId id;
                    if( id_encoded.length() == sizeof(id) * 2 + 2 && id_encoded[0] == '0' && id_encoded[1] == 'x' ) {
                        unsigned char id_decoded[sizeof(sHiveId)];
                        memset(id_decoded, sizeof(sHiveId), 0);
                        for(udx ic = 0; ic < sizeof(sHiveId); ic++) {
                            id_decoded[ic] = readHexByte(id_encoded.ptr(2 + 2 * ic));
                        }
                        memcpy(&id, id_decoded, sizeof(sHiveId));
                    }
                    time_t timestamp = (time_t)obj_usage_tbl.uval(ir, obj_usage_tbl.colId("time"));
                    idx disk_usage = obj_usage_tbl.ival(ir, obj_usage_tbl.colId("disk-usage"));
                    idx file_count = obj_usage_tbl.ival(ir, obj_usage_tbl.colId("file-count"));

                    if( id ) {
                        setObjUsage(id, timestamp, disk_usage, file_count);
                    }
                }
                USAGEDBG_SQLITE_TRACE("done.\nMigrating %"DEC" request usage records ...", req_usage_tbl.rows());
                for(idx ir = 0; ir < req_usage_tbl.rows(); ir++) {
                    if( !noCSVErrorCells(req_usage_tbl, ir, print_buf) ) {
                        USAGEDBG_SQLITE_TRACE("Skipping row %"DEC" : %s", ir, req_usage_tbl.printCSV(print_buf, ir, 0, 1));
                        continue;
                    }
                    // req, time, temp-usage
                    udx req = req_usage_tbl.uval(ir, req_usage_tbl.colId("req"));
                    time_t timestamp = (time_t)req_usage_tbl.uval(ir, req_usage_tbl.colId("time"));
                    idx temp_usage = req_usage_tbl.ival(ir, req_usage_tbl.colId("temp-usage"));

                    if( req ) {
                        setReqUsage(req, timestamp, temp_usage);
                    }
                }
                USAGEDBG_SQLITE_TRACE("%s", "done");
                commit();
            }
        }
};

// Needed because objs2() only searches by meaning=x OR version=y
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

#define NEW_SUSR_USAGE(user, id, ret, prc) \
    new sUsrUsage2(user, id); \
    do { \
        if( !ret ) { \
            prc->set(sRC::eAllocating, sRC::eMemory, sRC::eMemory, sRC::eExhausted); \
        } else if( !ret->Id() ) { \
            prc->set(sRC::eOpening, sRC::eObject, sRC::eOperation, sRC::eFailed); \
        } else { \
            *prc = sRC::zero; \
        } \
    } while( 0 )

// static
const sUsrUsage2 * sUsrUsage2::getObj(const sUsr & user, sRC * prc/* = 0*/)
{
    sRC temp_rc;
    if( !prc ) {
        prc = &temp_rc;
    }

    if( !user.Id() || user.isGuest() ) {
        prc->set(sRC::eFinding, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
        return 0;
    }

    sHiveId id;
    if( !findUsrUsage2Id(id, user) ) {
        prc->set(sRC::eFinding, sRC::eObject, sRC::eResult, sRC::eEmpty);
        return 0;
    }

    USAGEDBG("using existing object %s", id.print());
    const sUsrUsage2 * ret = NEW_SUSR_USAGE(user, id, ret, prc);

    return ret;
}

// static
sUsrUsage2 * sUsrUsage2::ensureObj(const sUsr & admin, sRC * prc/* = 0*/)
{
    sRC temp_rc;
    if( !admin.isAdmin() ) {
        prc->set(sRC::eFinding, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
        return 0;
    }

    sHiveId id;
    if( findUsrUsage2Id(id, admin) ) {
        USAGEDBG("using existing object %s", id.print());
        sUsrUsage2 * ret = NEW_SUSR_USAGE(admin, id, ret, prc);
        return ret;
    }

    USAGEDBG("creating new usage object");
    admin.updateStart();
    if( !admin.objCreate(id, "special") ) {
        admin.updateAbandon();
        prc->set(sRC::eCreating, sRC::eObject, sRC::eOperation, sRC::eFailed);
        return 0;
    }

    if( !admin.allow4admins(id) || !admin.allowRead4users(id) ) {
        admin.updateAbandon();
        prc->set(sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
        return 0;
    }

    sUsrUsage2 * ret = NEW_SUSR_USAGE(admin, id, ret, prc);
    if( !ret || !ret->Id() ) {
        admin.updateAbandon();
        return ret;
    }

    if( !ret->propSet("meaning", "user_usage") || !ret->propSet("title", "User usage statistics") || !ret->propSet("version", "1") ) {
        admin.updateAbandon();
        prc->set(sRC::eCreating, sRC::eProperty, sRC::eOperation, sRC::eFailed);
        delete ret;
        return 0;
    }

    admin.updateComplete();
    USAGEDBG("created new usage object %s", ret->Id().print());
    *prc = sRC::zero;

    return ret;
}

#define PROGRESS_USER_ITEMS 100
#define REPORT_PROGRESS \
if( cb ) cb(cb_param, sMin<idx>(iuser * PROGRESS_USER_ITEMS + progress_items++, (iuser + 1) * PROGRESS_USER_ITEMS), sNotIdx, num_users * PROGRESS_USER_ITEMS)

sRC sUsrUsage2::getUsageChange(idx values[eUsageTypeLast + 1], udx user_id, udx primary_group_id, time_t since[eUsageTypeLast + 1], time_t to, progressCb cb, void * cb_param, idx iuser, idx num_users)
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

        USAGEDBG_FUNC("user %"DEC", group %"DEC" %s", user_id, primary_group_id, time_buf.ptr());
    }
#endif

    idx progress_items = 0;
    for(idx i=0; i <= eUsageTypeLast; i++) {
        values[i] = 0;
    }

    // sanity check: disk usage and file count must be synchronized (on objs); wait time, run time, temp usage must be synchronized (on reqs)
    if( since[eDiskUsage] != since[eFileCount] || since[eTempUsage] != since[eWaitTime] || since[eTempUsage] != since[eRunTime] ) {
        return sRC(sRC::eChecking, sRC::eParameter, sRC::eTimeStamp, sRC::eNotEqual);
    }

    sSql::sqlProc sql_created_objs(m_usr.db(), "sp_obj_by_time");
    values[eObjCount] = sql_created_objs.Add(user_id).Add(primary_group_id).Add((idx)(since[eObjCount])).Add((idx)to).Add("created").getTable(0);
    USAGEDBG_TYPE(eObjCount, "%"DEC, values[eObjCount]);
    REPORT_PROGRESS;

    sSql::sqlProc sql_created_reqs(m_usr.db(), "sp_req_by_time");
    values[eReqCount] = sql_created_reqs.Add(user_id).Add((idx)(since[eReqCount])).Add((idx)to).Add("created").getTable(0);
    USAGEDBG_TYPE(eReqCount, "%"DEC, values[eReqCount]);
    REPORT_PROGRESS;

    sVarSet modified_objs_tbl;
    sSql::sqlProc sql_modified_objs(m_usr.db(), "sp_obj_by_time");
    sql_modified_objs.Add(user_id).Add(primary_group_id).Add((idx)(since[eDiskUsage])).Add((idx)to).Add("modified").getTable(&modified_objs_tbl);
    USAGEDBG("modifed objects : %"DEC,modified_objs_tbl.rows);
    REPORT_PROGRESS;

    sVarSet completed_objs_tbl;
    sSql::sqlProc sql_completed_objs(m_usr.db(), "sp_obj_by_time");
    sql_completed_objs.Add(user_id).Add(primary_group_id).Add((idx)(since[eCompletionTime])).Add((idx)to).Add("completed").getTable(&completed_objs_tbl);
    USAGEDBG("completed objects : %"DEC, completed_objs_tbl.rows);

    sStr path;
    sDir dir;
    for(idx ir=0; ir<modified_objs_tbl.rows; ir++) {
        sHiveId id(modified_objs_tbl.uval(ir, 0), modified_objs_tbl.uval(ir, 1), modified_objs_tbl.uval(ir, 2));
        path.cut(0);
        dir.cut();
        if( !sUsrObj::getPath(path, id) ) {
            USAGEDBG("obj %s has empty storage path", id.print());
            continue;
        }

        idx disk_usage = 0, file_count = 0;

        dir.find(sFlag(sDir::bitFiles)|sFlag(sDir::bitSubdirs)|sFlag(sDir::bitRecursive), path);
        USAGEDBG("obj %s has %"DEC" entries in storage path %s", id.print(), dir.dimEntries(), path.ptr());
        for(idx ient=0; ient<dir.dimEntries(); ient++) {
            disk_usage += sFile::size(dir.getEntryPath(ient));
            if( !(dir.getEntryFlags(ient) & sDir::fIsDir) ) {
                file_count++;
            }
            REPORT_PROGRESS;
        }

        values[eDiskUsage] += disk_usage - _usage_db->getObjUsage(id, eDiskUsage);
        values[eFileCount] += file_count - _usage_db->getObjUsage(id, eFileCount);
        USAGEDBG_TYPE(eDiskUsage,"obj %s has %"DEC" bytes (was %"DEC" bytes)", id.print(), disk_usage, _usage_db->getObjUsage(id, eDiskUsage));
        USAGEDBG_TYPE(eFileCount, "obj %s has %"DEC" (was %"DEC")", id.print(), file_count, _usage_db->getObjUsage(id, eFileCount));
        _usage_db->setObjUsage(id, to, disk_usage, file_count);
    }

    for(idx ir=0; ir<completed_objs_tbl.rows; ir++) {
        sHiveId id(completed_objs_tbl.uval(ir, 0), completed_objs_tbl.uval(ir, 1), completed_objs_tbl.uval(ir, 2));
        idx started_time = completed_objs_tbl.ival(ir, 3);
        idx completed_time = completed_objs_tbl.ival(ir, 4);
        if( completed_time >= started_time ) {
            values[eCompletionTime] += completed_time - started_time;
            USAGEDBG_TYPE(eCompletionTime, "obj %s has %"DEC" (started %"DEC", completed %"DEC")", id.print(), completed_time - started_time, started_time, completed_time);
        } else {
            USAGEDBG_TYPE(eCompletionTime, "obj %s has started %"DEC" > completed %"DEC" - not recording", id.print(), started_time, completed_time);
        }
    }

    // completed requests which last did something during the specified time interval
    sVarSet finished_reqs_tbl;
    sSql::sqlProc sql_finished_reqs(m_usr.db(), "sp_req_by_time");
    sql_finished_reqs.Add(user_id).Add((idx)(since[eTempUsage])).Add((idx)to).Add("completed").getTable(&finished_reqs_tbl);
    USAGEDBG("finished requests : %"DEC, finished_reqs_tbl.rows);
    REPORT_PROGRESS;

    sStr sql_buf;
    for(idx ir=0; ir<finished_reqs_tbl.rows; ir++) {
        udx req_id = finished_reqs_tbl.uval(ir, 0);
        idx created_time = finished_reqs_tbl.ival(ir, 2);
        idx taken_time = finished_reqs_tbl.ival(ir, 3);
        idx alive_time = finished_reqs_tbl.ival(ir, 4);
        idx done_time = finished_reqs_tbl.ival(ir, 5);
        if( taken_time > created_time ) {
            values[eWaitTime] += taken_time - created_time;
            USAGEDBG_TYPE(eWaitTime, "req %"DEC" has %"DEC, req_id, taken_time - created_time);
            idx end_time = sMax<idx>(alive_time, done_time);
            if( end_time > taken_time ) {
                values[eRunTime] += end_time - taken_time;
                USAGEDBG_TYPE(eRunTime, "req %"DEC" has %"DEC, req_id, end_time - taken_time);
            }
        }
        sVarSet req_data_tbl;
        sql_buf.printf(0, "SELECT dataName, LENGTH(dataBlob), IF(SUBSTRING(FROM_BASE64(SUBSTRING(dataBlob, 1, 12)), 1, 7) = 'file://', FROM_BASE64(dataBlob), NULL) FROM QPData WHERE reqID = %"UDEC, req_id);
        m_usr.db().getTable(sql_buf, &req_data_tbl);
        REPORT_PROGRESS;
        idx temp_usage = 0;
        for(idx idat=0; idat<req_data_tbl.rows; idat++) {
            temp_usage += req_data_tbl.ival(idat, 1);
            USAGEDBG_TYPE(eTempUsage, "req %"DEC" '%s' db has %"DEC, req_id, req_data_tbl.val(idat, 0), req_data_tbl.ival(idat, 1));
            const char * data_path = req_data_tbl.val(idat, 2);
            if( data_path && *data_path ) {
                data_path += 7; // strlen("file://")
                temp_usage += sFile::size(data_path);
                USAGEDBG_TYPE(eTempUsage, "req %"DEC" '%s' file has %"DEC, req_id, data_path, sFile::size(data_path));
            }
            REPORT_PROGRESS;
        }
        values[eTempUsage] += temp_usage;
        _usage_db->setReqUsage(req_id, to, temp_usage);
    }
    return sRC::zero;
}

sRC sUsrUsage2::updateIncremental(progressCb cb, void * cb_param, time_t at_time)
{
    if( !m_usr.isAdmin() || !m_usr.isAllowed(m_id, ePermCanWrite|ePermCanAdmin) ) {
        return sRC(sRC::eChecking, sRC::ePermission, sRC::eUser, sRC::eNotAuthorized);
    }

    if( !_usage_db || !_usage_db->ok() ) {
        return sRC(sRC::eChecking, sRC::eFile, sRC::ePointer, sRC::eNull);
    }

    idx values[eUsageTypeLast + 1];
    time_t since[eUsageTypeLast + 1];
    sVarSet groups_tbl;
    if( !at_time ) {
        at_time = time(0);
    }
    m_usr.db().getTable("SELECT DISTINCT userID, groupID FROM UPGroup WHERE flags = -1", &groups_tbl); // flags = -1 means primary group
    _usage_db->startTransaction();
    for(idx ig=0; ig<groups_tbl.rows; ig++) {
        udx user_id = groups_tbl.uval(ig, 0);
        udx primary_group_id = groups_tbl.uval(ig, 1);
        for(idx i=0; i<=eUsageTypeLast; i++) {
            since[i] = _usage_db->getLastTimestampBefore((time_t)sUdxMax, user_id, (EUsageType)i, 0);
            if( since[i] >= at_time ) {
                return sRC(sRC::eChecking, sRC::eParameter, sRC::eTimeStamp, sRC::eTooSmall);
            }
        }
        if( sRC rc = getUsageChange(values, user_id, primary_group_id, since, at_time, cb, cb_param, ig, groups_tbl.rows) ) {
            _usage_db->rollback();
            return rc;
        }
        for(idx i=0; i<=eUsageTypeLast; i++) {
            _usage_db->addRecord(user_id, (EUsageType)i, 0, at_time, values[i]);
        }
    }
    _usage_db->commit();
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
        return sRC(sRC::eChecking, sRC::ePermission, sRC::eUser, sRC::eNotAuthorized);
    }

    if( !_usage_db || !_usage_db->ok() ) {
        return sRC(sRC::eChecking, sRC::eFile, sRC::ePointer, sRC::eNull);
    }

    sVarSet groups_tbl;
    if( !at_time ) {
        at_time = time(0);
    }
    // FIXME : put primary group in PurgedObj
    m_usr.db().getTable("SELECT DISTINCT userID, groupID FROM UPGroup WHERE flags = -1", &groups_tbl); // flags = -1 means primary group
    sDic<udx> creator2user;
    for(idx ig=0; ig<groups_tbl.rows; ig++) {
        udx user_id = groups_tbl.uval(ig, 0);
        udx primary_group_id = groups_tbl.uval(ig, 1);
        *creator2user.set(&primary_group_id, sizeof(udx)) = user_id;
    }

    // need to sort objs and reqs by their user id
    sVec<idx> objs_order(sMex::fExactSize), reqs_order(sMex::fExactSize);
    objs_order.resize(objs.dim());
    reqs_order.resize(reqs.dim());

    // const casts needed because sortSimpleCallback's comparator callback uses non-const ptr to array in its signature;
    // safe since our cmpPurged* callbacks treat the array ptr as constant
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
            _usage_db->setReqUsage(reqs[ireq].req_id, at_time, 0);
            _usage_db->delReqUsage(reqs[ireq].req_id);
            ireq++;
            user_from_req = ireq < reqs.dim() ? reqs[ireq].user_id : 0;
        }
        while( user_id == user_from_obj ) {
            values[eObjCount]--;
            values[eDiskUsage] -= _usage_db->getObjUsage(objs[iobj].hive_id, eDiskUsage);
            values[eFileCount] -= _usage_db->getObjUsage(objs[iobj].hive_id, eFileCount);
            _usage_db->setObjUsage(objs[iobj].hive_id, at_time, 0, 0);
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

sRC sUsrUsage2::exportGroupsTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const char * group_paths00, bool with_all_selected) const
{
    if( !m_usr.isAllowed(m_id, ePermCanRead) ) {
        return sRC(sRC::eAccessing, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
    }

    sStr sql, group_path_buf;
    sVec<udx> user_ids;
    sDic<udx> all_user_ids;
    sVec<UserList> user_lists;
    sVarSet user_tbl;

    for(const char * group_path = group_paths00; group_path && *group_path; group_path = sString::next00(group_path) ) {
        UserList * user_list = user_lists.add(1);
        sql.cutAddString(0, "SELECT DISTINCT userID FROM UPGroup");
        if( strcmp(group_path, "*") == 0 ) {
            user_list->label = group_path;
        } else {
            sql.addString(" WHERE ");
            group_path_buf.cutAddString(0, group_path);
            if( group_path_buf[group_path_buf.length() - 1] == '/' ) {
                // match group prefix
                user_list->label = group_path;
                group_path_buf.addString("%");
                sql.addString("groupPath LIKE ");
            } else {
                // exact group name
                if( const char * slash = strrchr(group_path, '/') ) {
                    user_list->label = slash + 1;
                } else {
                    user_list->label = group_path;
                }
                sql.addString("groupPath = ");
            }
            m_usr.db().protectValue(sql, group_path_buf);
        }

        user_tbl.empty();
        m_usr.db().getTable(sql, &user_tbl);
        idx prev_tot_dim = user_ids.dim();
        user_list->user_ids = user_ids.add(user_tbl.rows);
        user_list->dim = user_tbl.rows;
        for(idx ir=0; ir<user_tbl.rows; ir++) {
            udx user_id = user_tbl.uval(ir, 0);
            user_ids[prev_tot_dim + ir] = user_id;
            if( with_all_selected ) {
                *all_user_ids.set(&user_id, sizeof(udx)) = user_id;
            }
        }
    }

    if( with_all_selected ) {
        UserList * user_list = user_lists.add(1);
        user_list->label = "All selected users/groups";
        user_list->dim = all_user_ids.dim();
        idx prev_tot_dim = user_ids.dim();
        user_list->user_ids = user_ids.add(user_list->dim);
        for(idx iu=0; iu<user_list->dim; iu++) {
            user_ids[prev_tot_dim + iu] = *all_user_ids.ptr(iu);
        }
    }

    // fix up user_lists' user_ids pointers, which became invalidated as user_ids vector grew
    for(idx il=0, prev_tot_dim=0; il<user_lists.dim(); il++) {
        user_lists[il].user_ids = user_ids.ptr(prev_tot_dim);
        prev_tot_dim += user_lists[il].dim;
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
        err = false;
    }

    void flush()
    {
        if( have_cached ) {
            buf.cut(0);
            sString::printDateTime(buf, timestamp, sString::fISO8601); // strict ISO8601 mode for javascript
            if( !out.addCell(buf.ptr(), buf.length(), sVariant::value_DATE_TIME) ) {
                err = true;
            }
            for( idx i=0; i<dim; i++ ) {
                if( !out.addICell(values[i]) ) {
                    err = true;
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
            // preserve sums from the previous printed row, set values (delta change) to zero
            timestamp = to;
            for(idx i = 0; i < dim; i++) {
                values[i] = 0;
            }
            have_cached = true;
            flush();
        }
    }

    static bool callback(slib::sUsrUsage2::EUsageType type, idx list_index, idx tag, time_t timestamp, idx value, idx sum_adjust, void * param)
    {
        ExportTableWorker * self = static_cast<ExportTableWorker*>(param);

        // If we see a series of identical timestamps, sum them in cache before flushing to output table
        if( self->have_cached && timestamp == self->timestamp ) {
            self->values[list_index] += value;
            self->sums[list_index] += value + sum_adjust;
        } else {
            self->flush();
            self->have_cached = true;
            self->timestamp = timestamp;
            self->values[list_index] = value;
            self->sums[list_index] += value + sum_adjust;
        }

        return !self->err;
    }
};

sRC sUsrUsage2::exportTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const sUsrUsage2::UserList * user_lists, idx num_user_lists) const
{
    if( !m_usr.isAllowed(m_id, ePermCanRead) ) {
        return sRC(sRC::eAccessing, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
    }

    if( !m_usr.isAdmin() ) {
        // only admins can see other user's usage
        for(idx il=0; il<num_user_lists; il++) {
            if( user_lists[il].dim > 1 || (user_lists[il].dim == 1 && user_lists[il].user_ids[0] != m_usr.Id())) {
                return sRC(sRC::eAccessing, sRC::eUser, sRC::eUser, sRC::eNotAuthorized);
            }
        }
    }

    if( !out.initWritable(1 + 2 * num_user_lists, 0) ) {
        return sRC(sRC::eInitializing, sRC::eTable, sRC::eMode, sRC::eReadOnly);
    }

    // first header line: user list labels
    out.addCell("");
    sStr quoted_buf;
    for(idx il=0; il<num_user_lists; il++) {
        quoted_buf.cut(0);
        sString::escapeForCSV(quoted_buf, user_lists[il].label);
        out.addCell(quoted_buf);
        out.addCell(quoted_buf);
    }
    out.addEndRow();

    // second header line: column contents
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
        return sRC(sRC::eReading, sRC::eData, sRC::eFile, sRC::eNotFound);
    }

    ExportTableWorker worker(out, num_user_lists);
    _usage_db->readRows(user_lists, num_user_lists, usage_type, 0, since, to, worker.callback, &worker);
    if( time_t final_row_timestamp = _usage_db->getLastTimestampBefore(to, usage_type, 0) ) {
        worker.addFinalRow(final_row_timestamp);
    }

    if( worker.err ) {
        return sRC(sRC::eWriting, sRC::eTable, sRC::eOperation, sRC::eFailed);
    }

    return sRC::zero;
}

sUsrUsage2::sUsrUsage2(const sUsr& user, const sHiveId & objId) : sUsrObj(user, objId)
{
    sStr db_path;
    bool can_admin = m_usr.isAdmin() && m_usr.isAllowed(m_id, ePermCanWrite|ePermCanAdmin);
    bool can_read = m_usr.isAllowed(m_id, ePermCanRead);
    _usage_db = 0;

    if( can_admin ) {
        if( getFilePathname(db_path, "usage.sqlite") || addFilePathname(db_path, false, "usage.sqlite") ) {
            USAGEDBG("Opening usage.sqlite for writing");
            _usage_db = new UsageDb(db_path, false);
            if( _usage_db->ok() ) {
                if( !_usage_db->dimHistoryRecords() ) {
                    _usage_db->migrateFromIon(db_path.ptr());
                }
            } else {
                USAGEDBG("Failed to read usage.sqlite");
                delete _usage_db;
                _usage_db = 0;
            }

        } else {
            USAGEDBG("Refusing to open usage.sqlite for writing");
        }
    } else if( can_read && getFilePathname(db_path, "usage.sqlite") ) {
        USAGEDBG("Opening usage.sqlite for reading");
        _usage_db = new UsageDb(db_path, true);
        if( !_usage_db->ok() ) {
            USAGEDBG("Failed to read usage.sqlite");
            delete _usage_db;
            _usage_db = 0;
        }
    }
}

sUsrUsage2::~sUsrUsage2()
{
    delete _usage_db;
}
