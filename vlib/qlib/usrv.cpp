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
#include <ulib/uobj.hpp>
#include <ulib/ufile.hpp>
#include <ulib/uusage.hpp>
#include <errno.h>
#include <fcntl.h>
#include "QPrideSrv.hpp"

namespace slib {

    class sUSrv: public sQPrideSrv
    {
            typedef sQPrideSrv TParent;
        public:
            sUSrv(const char * defline00, const char * srv)
                : TParent(defline00, srv), m_statisticsTimeLast(0), m_statisticsTimeFrequencySecs(0), m_statisticsTimeUnit(0)
            {
            }

            virtual bool OnCommand(const char * command, const char * value);
            virtual bool OnCommandUsage(const char * command, const char * value);
            virtual bool OnCommandAudit(const char * command, const char * value);

            virtual idx OnMaintain(void);

        protected:
            bool init(void);
            void purge(TPurgeData & data);

        private:

            sTime m_statisticsTimeLast;
            idx m_statisticsTimeFrequencySecs;
            idx m_statisticsTimeUnit;
    };
}
;

using namespace slib;

bool sUSrv::init(void)
{
    bool res = TParent::init();
    m_statisticsTimeFrequencySecs = cfgInt(0, "qm.statisticsTimeFrequencySecs", 300);
    m_statisticsTimeUnit = cfgInt(0, "qm.statisticsTimeUnit", 3600);
    return res;
}

bool sUSrv::OnCommand(const char * command, const char * value)
{
    idx ll;
#define jobIsCmd(_cmd) (!strncmp(command, _cmd, (ll = sLen(_cmd))))

    if( !init() ) {
        return false;
    } else if( jobIsCmd("usage")) {
        return OnCommandUsage(command, value);
    } else if( jobIsCmd("audit")) {
        return OnCommandAudit(command, value);
    }
    return TParent::OnCommand(command, value);
#undef jobIsCmd
}

idx sUSrv::OnMaintain(void)
{
    idx res = TParent::OnMaintain();

    if( m_statisticsTimeFrequencySecs ) {
        sTime t;
        idx timeHasPassedSinceLastTime = t.time(&m_statisticsTimeLast);
        if( timeHasPassedSinceLastTime > m_statisticsTimeFrequencySecs ) {
            m_statisticsTimeLast = t;
            OnCommand("audit", 0);
        }
    }
    return res;
}

bool sUSrv::OnCommandUsage(const char * command, const char * value)
{
    return false;
}

bool sUSrv::OnCommandAudit(const char * command, const char * value)
{
    sUsr qpride("qpride", true);
    if( !qpride.Id() ) {
        logOut(eQPLogType_Warning, "Cannot sign in\n");
    } else {
        const udx keep = cfgInt(0, "qm.auditKeepHours", 0);
        const udx chunk = cfgInt(0, "qm.auditChunkSize", 10000);
        if( keep ) {
            sStr path;
            cfgStr(&path, 0, "qm.auditDumpFileTmpl");
            const udx now = time(0) - keep * 60 * 60;
            udx maxID = 0;
            udx q = 1;
            if( path ) {
                path.printf(".%" UDEC, now);
                int dump = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                if( dump >= 0 ) {
                    sVarSet tbl;
                    sStr buf;
                    q = 0;
                    do {
                        tbl.empty();
                        qpride.db().getTable(&tbl, "SELECT historyID, createTm, sid, userID, operation, comment FROM UPHistory WHERE createTm < FROM_UNIXTIME(%" UDEC ")"
                                        " ORDER BY historyID ASC LIMIT %" UDEC ",%" UDEC, now, q, chunk);
                        if(qpride.db().Get_errno() != 0 ) {
                            logOut(eQPLogType_Error, "Database error '%s'\n", qpride.db().Get_error().ptr());
                            q = 0;
                            break;
                        }
                        if( tbl.rows ) {
                            const char * nm[] = { "id", "time", "session", "user", "operation", "details" };
                            buf.cut(0);
                            tbl.printCSV(buf, q ? 0 : 6, nm);

                            idx written = 0, togo = buf.length();
                            errno = 0;
                            while( togo - written > 0 && errno == 0 ) {
                                written += write(dump, buf.ptr(written), togo - written);
                            }
                            if( errno != 0 ) {
                                logOut(eQPLogType_Error, "Cannot write audit dump file '%s': %s\n", path.ptr(), strerror(errno));
                                q = 0;
                                break;
                            }
                            q += tbl.rows;
                            maxID = tbl.uval(tbl.rows - 1, 0);
                        }
                    } while( tbl.rows );
                    fdatasync(dump);
                    close(dump);
                    if( !q ) {
                        sFile::remove(path);
                    } else {
                        logOut(eQPLogType_Info, "Audit file '%s' %" UDEC " lines, histID %" UDEC, path.ptr(), q, maxID);
                    }
                } else {
                    logOut(eQPLogType_Error, "Cannot open audit dump file '%s': %s", path.ptr(), strerror(errno));
                }
            }
            if( q ) {
                if( maxID ) {
                    const udx qty = qpride.db().uvalue("SELECT COUNT(*) FROM UPHistory WHERE historyID <= %" UDEC, maxID);
                    for( udx i = 0; i < qty; ++i ) {
                        qpride.db().execute("DELETE FROM UPHistory WHERE historyID <= %" UDEC " LIMIT %" UDEC, maxID, chunk);
                    }
                    qpride.db().execute("DELETE FROM UPHistory WHERE historyID <= %" UDEC, maxID);
                } else {
                    qpride.db().execute("DELETE FROM UPHistory WHERE createTm < FROM_UNIXTIME(%" UDEC ")", now);
                }
            }
        }
    }
    return true;
}

void sUSrv::purge(TPurgeData & data)
{
    sUsr qpride("qpride", true);
    if( !qpride.Id() ) {
        logOut(eQPLogType_Warning, "Cannot sign in\n");
    } else {
        sVarSet res;
        idx limit = cfgInt(0, "qm.purgeObjectLimit", 1000);
        limit = limit ? limit : -1;
        idx expireAfter = cfgInt(0, "qm.ObjectExpireDays", 30);
        expireAfter = expireAfter >= 0 ? expireAfter : 30;
        qpride.db().getTable(&res, "SELECT domainID, objID FROM UPObj WHERE softExpiration IS NOT NULL AND DATEDIFF(NOW(), softExpiration) >= %" UDEC " LIMIT %" UDEC, expireAfter, limit);
        if( res.rows ) {
            sStr deleted("\t");
            sStr not_deleted("\t");
            idx cnt_deleted = 0, cnt_not_deleted = 0;
            qpride.allowExpiredObjects(true);
            for(idx i = 0; i < res.rows; ++i) {
                const udx domain_id = res.uval(i, 0);
                const udx oid = res.uval(i, 1);
                const sHiveId id(domain_id, oid, 0);
#if !_DEBUG
                sUsrObj obj(qpride, id);
#else
                sUsrObj obj;
#endif
                if( obj.Id() && obj.purge() ) {
                    if( !domain_id ) {
                        data.objs.set(&id, sizeof(id));
                    }
                    deleted.printf("%s%s", id.print(), ((cnt_deleted + 1) % 5) ? ", " : "\n\t");
                    cnt_deleted++;
                } else {
                    not_deleted.printf("%s%s", id.print(), ((cnt_not_deleted + 1) % 5) ? ", " : "\n\t");
                    cnt_not_deleted++;
                }
            }
            qpride.allowExpiredObjects(false);

            if( cnt_deleted ) {
                deleted.cut0cut(deleted.length() - 2);
            }
            if( cnt_not_deleted ) {
                not_deleted.cut0cut(not_deleted.length() - 2);
            }

            logOut(eQPLogType_Info, "following %" DEC " objects have been purged:\n%s\n", cnt_deleted, deleted.ptr());
            if( cnt_not_deleted ) {
                logOut(eQPLogType_Warning, "following %" DEC " objects could not be purged:\n%s\n", cnt_not_deleted, not_deleted.ptr());
            }
        }
    }
    TParent::purge(data);
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    sUSrv backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "qm", argv[0]));
    return (int) backend.run(argc, argv);
}
