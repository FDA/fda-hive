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
#include <qlib/QPrideBase.hpp>
#include <ulib/uhousekeeper.hpp>
#include <ulib/uproc.hpp>

using namespace slib;

sRC sUsrHousekeeper::findObjsForPurge(const sUsr & admin, sVec<PurgedObj> & objs, idx max_cnt, idx max_age_days)
{
    if( !admin.Id() || !admin.isAdmin() ) {
        return RC(sRC::eFinding, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
    }

    sVarSet objs_tbl;
    admin.db().getTable(&objs_tbl, "SELECT domainID, objID, ionID, creatorID FROM UPObj WHERE softExpiration IS NOT NULL AND DATEDIFF(NOW(), softExpiration) >= %" UDEC " ORDER BY softExpiration LIMIT %" UDEC, max_age_days, max_cnt);

    const idx objs_initial_dim = objs.dim();
    objs.add(objs_tbl.rows);
    sStr objs_buf;

    for(idx ir=0; ir<objs_tbl.rows; ir++) {
        objs[objs_initial_dim + ir].hive_id.set(objs_tbl.uval(ir, 0), objs_tbl.uval(ir, 1), objs_tbl.uval(ir, 2));
        objs[objs_initial_dim + ir].creator_id = objs_tbl.uval(ir, 3);
        if( ir ) {
            objs_buf.addString(", ");
        }
        objs[objs_initial_dim + ir].hive_id.print(objs_buf);
    }

    sQPrideBase * qp = admin.QPride();
    if( qp ) {
        objs_buf.add0();
        qp->logOut(sQPrideBase::eQPLogType_Trace, "sUsrHousekeeper::findObjsForPurge() found %" DEC " object(s): %s", objs_tbl.rows, objs_buf.ptr());
    }

    return sRC::zero;
}

sRC sUsrHousekeeper::findReqsForPurge(const sUsr & admin, sVec<PurgedReq> & reqs, idx max_cnt)
{
    if( !admin.Id() || !admin.isAdmin() ) {
        return RC(sRC::eFinding, sRC::eRequest, sRC::eUser, sRC::eNotAuthorized);
    }

    sRC rc;
    sDic<bool> seen_uproc_id;

    sQPrideBase * qp = admin.QPride();
    sQPride * qpr = dynamic_cast<sQPride*>(qp);
    const bool uses_qpsvc = qpr && qpr->usesQPSvcTable();

    sSql::sqlProc p(admin.db(), "sp_svc_purge_v2");
    p.Add("*").Add(max_cnt).Add("find").Add(uses_qpsvc);
    sVarSet reqs_tbl;
    p.getTable(&reqs_tbl);

    if( qp ) {
        sStr buf;
        for(idx ir=0; ir<reqs_tbl.rows; ir++) {
            if( ir ) {
                buf.addString(", ");
            }
            buf.addString(reqs_tbl.val(ir, 0));
        }
        buf.add0();
        qp->logOut(sQPrideBase::eQPLogType_Warning, "sUsrHousekeeper::findReqsForPurge() found %" DEC " requests: %s", reqs_tbl.rows, buf.ptr());
    }

    const idx reqs_initial_dim = reqs.dim();
    reqs.add(reqs_tbl.rows);
    sStr sync_success_buf;
    idx sync_success_cnt = 0;
    for(idx ir=0; ir<reqs_tbl.rows; ir++) {
        udx req = reqs_tbl.uval(ir, 0);
        udx user_id = reqs_tbl.uval(ir, 1);
        reqs[reqs_initial_dim + ir].req_id = req;
        reqs[reqs_initial_dim + ir].user_id = user_id;
        sHiveId id((udx)0, reqs_tbl.uval(ir, 1), 0);
        if( !seen_uproc_id.get(&id, sizeof(sHiveId)) ) {
            seen_uproc_id.set(&id, sizeof(sHiveId));
            sUsrProc uproc(admin, id);
            if( uproc.Id() && (udx)uproc.reqID() == req ) {
                if( uproc.propSync() ) {
                    if( sync_success_buf.length() ) {
                        sync_success_buf.addString(",");
                    }
                    uproc.Id().print(sync_success_buf);
                    sync_success_cnt++;
                } else {
                    if( qp ) {
                        qp->logOut(sQPrideBase::eQPLogType_Error, "sUsrHousekeeper::findReqsForPurge() failed to sync process object %s", id.print());
                    }
                    RCSET(rc, sRC::eUpdating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
                }
            }
        }
    }

    if( qp ) {
        sync_success_buf.add0();
        qp->logOut(sQPrideBase::eQPLogType_Warning, "sUsrHousekeeper::findReqsForPurge() synced %" DEC " process object(s): %s", sync_success_cnt, sync_success_buf.ptr());
    }

    return rc;
}

sRC sUsrHousekeeper::purgeObjs(sUsr & admin, const sVec<PurgedObj> & objs)
{
    if( !admin.Id() || !admin.isAdmin() ) {
        return RC(sRC::eFinding, sRC::eRequest, sRC::eUser, sRC::eNotAuthorized);
    }

    sQPrideBase * qp = admin.QPride();

    sStr purged_success_buf;
    idx purged_success_cnt = 0;
    admin.allowExpiredObjects(true);
    for(idx i=0; i<objs.dim(); i++) {
        sUsrObj * obj = admin.objFactory(objs[i].hive_id);
        if( obj && obj->purge() ) {
            if( purged_success_cnt++ ) {
                purged_success_buf.addString(",");
            }
            objs[i].hive_id.print(purged_success_buf);
        } else if( qp ) {
            qp->logOut(sQPrideBase::eQPLogType_Error, "sUsrHousekeeper::purgeObjs() failed to delete object %s", objs[i].hive_id.print());
        }
        delete obj;
    }
    admin.allowExpiredObjects(false);

    if( qp ) {
        purged_success_buf.add0();
        qp->logOut(sQPrideBase::eQPLogType_Warning, "sUsrHousekeeper::purgeObjs() deleted %" DEC " object(s) : %s", purged_success_cnt, purged_success_buf.ptr());
    }

    return sRC::zero;
}

sRC sUsrHousekeeper::purgeReqs(const sUsr & admin, const sVec<PurgedReq> & reqs)
{
    if( !admin.Id() || !admin.isAdmin() ) {
        return RC(sRC::eFinding, sRC::eRequest, sRC::eUser, sRC::eNotAuthorized);
    }

    sQPrideBase * qp = admin.QPride();
    sQPride * qpr = dynamic_cast<sQPride*>(qp);
    const bool uses_qpsvc = qpr && qpr->usesQPSvcTable();

    if( !reqs.dim() ) {
        sSql::sqlProc p(admin.db(), "sp_svc_purge_v2");
        p.Add("*").Add((idx)0).Add("cleanup").Add(uses_qpsvc);
        p.execute();
        return sRC::zero;
    }

    sStr reqs_buf;
    for(idx i=0; i < reqs.dim(); i++) {
        reqs_buf.printf("%s%" UDEC, i ? "," : "", reqs[i].req_id);
    }
    reqs_buf.add0();

    sSql::sqlProc p(admin.db(), "sp_svc_purge_v2");
    p.Add("*").Add(reqs.dim()).Add("delete").Add(uses_qpsvc);
    if( p.execute() ) {
        if( qp ) {
            qp->logOut(sQPrideBase::eQPLogType_Warning, "sUsrHousekeeper::purgeReqs() deleted %" DEC " request(s) : %s", reqs.dim(), reqs_buf.ptr());
        }
        return sRC::zero;
    } else {
        if( qp ) {
            qp->logOut(sQPrideBase::eQPLogType_Error, "sUsrHousekeeper::purgeReqs() failed to delete %" DEC " request(s) : %s", reqs.dim(), reqs_buf.ptr());
        }
        return RC(sRC::eExecuting, sRC::eDatabase, sRC::eCommand, sRC::eFailed);
    }
}

sRC sUsrHousekeeper::purgeMisc(const sUsr & admin, idx limit)
{
    if( !admin.Id() || !admin.isAdmin() ) {
        return RC(sRC::eFinding, sRC::eRequest, sRC::eUser, sRC::eNotAuthorized);
    }

    sQPrideBase * qp = admin.QPride();
    sQPride * qpr = dynamic_cast<sQPride*>(qp);
    const bool uses_qpsvc = qpr && qpr->usesQPSvcTable();

    sSql::sqlProc p(admin.db(), "sp_misc_purge");
    p.Add(uses_qpsvc).Add(limit);
    sVarSet misc_tbl;
    if( p.getTable(&misc_tbl) ) {
        if( qp ) {
            sStr buf;
            const char * prev_label = 0;
            for(idx ir=0; ir<misc_tbl.rows; ir++) {
                const char * label = misc_tbl.val(ir, 0);
                if( !label ) {
                    label = "";
                }
                if( !prev_label || !label || strcmp(prev_label, label) != 0 ) {
                    if( prev_label ) {
                        buf.addString("; ");
                    }
                    buf.addString(label);
                    buf.addString(" ");
                    buf.addString(misc_tbl.val(ir, 1));
                } else {
                    buf.addString(",");
                    buf.addString(misc_tbl.val(ir, 1));
                }
                prev_label = label;
            }
            buf.add0();
            qp->logOut(sQPrideBase::eQPLogType_Warning, "sUsrHousekeeper::purgeMisc() : purged %s", buf.ptr());
        }
        return sRC::zero;
    } else {
        if( qp ) {
            qp->logOut(sQPrideBase::eQPLogType_Error, "sUsrHousekeeper::purgeMisc() : failed");
        }
        return RC(sRC::eExecuting, sRC::eDatabase, sRC::eCommand, sRC::eFailed);
    }
}

class NameMaskMatcher
{
    public:
        enum EMatchType {
            eMatchNone = 0,
            eMatchReq,
            eMatchObj
        };
    private:
        EMatchType _type;
        const char * _prefix;
        idx _prefix_len;
        const char * _suffix;
        idx _suffix_len;
        sHiveId _obj;
        udx _req;

    public:
        NameMaskMatcher()
        {
            _prefix = _suffix = 0;
            _prefix_len = _suffix_len = 0;
            _type = eMatchNone;
            _req = 0;
        }

        void init(const char * mask)
        {
            if( !mask ) {
                mask = "";
            }
            _prefix = mask;
            if( const char * pattern = strstr(mask, "@reqID@") ) {
                _prefix_len = pattern - mask;
                _type = eMatchReq;
                _suffix = pattern + sLen("@reqID@");
                _suffix_len = sLen(_suffix);
            } else if( const char * pattern = strstr(mask, "@objID@") ) {
                _prefix_len = pattern - mask;
                _type = eMatchObj;
                _suffix = pattern + sLen("@objID@");
                _suffix_len = sLen(_suffix);
            } else {
                _prefix_len = sLen(mask);
                _type = eMatchNone;
                _suffix = 0;
                _suffix_len = 0;
            }
        }

        bool match(const char * name, sDic<char> * reqs_dic, sDic<char> * objs_dic)
        {
            if( !name || !*name ) {
                return _prefix_len || _suffix_len || _type;
            }

            if( _prefix_len ) {
                if( strncmp(name, _prefix, _prefix_len) != 0 ) {
                    return false;
                }
                name += _prefix_len;
            }

            if( _type == eMatchReq ) {
                char * end = 0;
                _req = strtoudx(name, &end, 10);
                if( end == name ) {
                    return false;
                }
                if( reqs_dic && !reqs_dic->get(&_req, sizeof(udx)) ) {
                    return false;
                }
                name = end;
            } else if( _type == eMatchObj ) {
                idx len_parsed = _obj.parse(name);
                if( !len_parsed ) {
                    return false;
                }
                if( objs_dic && !objs_dic->get(&_obj, sizeof(sHiveId)) ) {
                    return false;
                }
                name += len_parsed;
            }

            if( _suffix_len ) {
                if( strncmp(name, _suffix, _suffix_len) != 0 ) {
                    return false;
                }
            }

            return true;
        }

        EMatchType type() const { return _type; }
        udx getReq() const { return _req; }
        const sHiveId & getObj() const { return _obj; }

        const char * print(sStr & out)
        {
            idx pos = out.length();
            switch( _type ) {
                case eMatchNone:
                    out.addString("prefix '");
                    if( _prefix_len ) {
                        out.addString(_prefix, _prefix_len);
                    }
                    out.addString("'");
                    break;
                case eMatchReq:
                    out.printf(0, "request %" DEC, _req);
                    break;
                case eMatchObj:
                    out.addString("object ");
                    _obj.print(out);
            }
            return out.ptr(pos);
        }
};

sRC sUsrHousekeeper::purgeTempFiles(const sUsr & admin, const sVec<PurgedObj> & objs, const sVec<PurgedReq> & reqs)
{
    if( !admin.Id() || !admin.isAdmin() ) {
        return RC(sRC::eFinding, sRC::eRequest, sRC::eUser, sRC::eNotAuthorized);
    }

    sQPrideBase * qp = admin.QPride();
    if( !qp ) {
        return RC(sRC::eFinding, sRC::ePath, sRC::ePointer, sRC::eNull);
    }

    sDic<char> objs_dic;
    for(idx i=0; i<objs.dim(); i++) {
        objs_dic.set(&(objs[i].hive_id), sizeof(sHiveId));
    }

    sDic<char> reqs_dic;
    for(idx i=0; i<reqs.dim(); i++) {
        reqs_dic.set(&(reqs[i].req_id), sizeof(udx));
    }

    sVarSet service_path_tbl;
    qp->servicePath2Clean(service_path_tbl);

    sVec<NameMaskMatcher> mask_matchers;
    idx cur_time = time(0);
    sRC rc;
    sStr buf;

    for(idx ir = 0; ir < service_path_tbl.rows; ir++) {
        const char * paths00 = service_path_tbl.val(ir, 0);
        const idx cleanup_days = service_path_tbl.ival(ir, 1);
        const idx cleanup_secs = cleanup_days * 24 * 60 * 60;
        const char * masks00 = service_path_tbl.val(ir, 2);
        for(const char * path = paths00; path && *path; path = sString::next00(path)) {
            mask_matchers.cut(0);
            for(const char * mask = masks00; mask && *mask; mask = sString::next00(mask)) {
                mask_matchers.add(1)->init(mask);
            }

            sDir files;
            files.list(sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs), path, "*");
            for(idx ie=0; ie<files.dimEntries(); ie++) {
                const char * filepath = files.getEntryPath(ie);
                const char * filename = sFilePath::nextToSlash(filepath);
                bool need_check_atime = true;
                for(idx im=0; im<mask_matchers.dim(); im++) {
                    if( mask_matchers[im].match(filename, &reqs_dic, &objs_dic) ) {
                        if( sFile::remove(filepath) || sDir::removeDir(filepath, true) ) {
                            buf.cut(0);
                            qp->logOut(sQPrideBase::eQPLogType_Warning, "sUsrHousekeeper::purgeTempFiles() : deleted '%s' matching purged %s", filepath,  mask_matchers[im].print(buf));
                        } else {
                            qp->logOut(sQPrideBase::eQPLogType_Error, "sUsrHousekeeper::purgeTempFiles() : failed to delete '%s'", filepath);
                            rc = RC(sRC::eRemoving, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                        need_check_atime = false;
                        break;
                    }
                }
                if( need_check_atime && cleanup_secs ) {
                    idx atime = sFile::atime(filepath);
                    if( cur_time > 0 && atime > 0 && atime < cur_time - cleanup_secs ) {
                        if( sFile::remove(filepath) || sDir::removeDir(filepath, true) ) {
                            buf.cut(0);
                            qp->logOut(sQPrideBase::eQPLogType_Warning, "sUsrHousekeeper::purgeTempFiles() : deleted '%s' for age (atime %s < now - %" DEC " days)", filepath, sString::printDateTime(buf, atime), cleanup_days);
                        } else {
                            qp->logOut(sQPrideBase::eQPLogType_Error, "sUsrHousekeeper::purgeTempFiles() : failed to delete '%s'", filepath);
                            rc = RC(sRC::eRemoving, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                    }
                }
            }
        }
    }

    return rc;
}
