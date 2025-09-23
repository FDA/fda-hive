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
#include <slib/core/tim.hpp>
#include <slib/std/cryptohash.hpp>
#include <slib/std/cryptocipher.hpp>
#include <ulib/usr.hpp>
#include <ulib/utype2.hpp>
#include <ulib/uquery.hpp>
#include <ulib/ufile.hpp>
#include <ulib/uproc.hpp>
#include <ulib/uemail.hpp>
#include <ulib/ufolder.hpp>
#include <ulib/uusage.hpp>
#include <ulib/upropset.hpp>
#include <qlib/QPride.hpp>
#include <slib/std/crypt.hpp>
#include <slib/std/url.hpp>
#include <xlib/md5.hpp>

#include "uperm.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <regex.h>

using namespace slib;

sQPrideBase * sUsr::sm_qpride = 0;
sSql sUsr::sm_cfg_db;
sSql * sUsr::sm_actual_db = 0;
sUsr::EAuditMode g_Audit = sUsr::eUserAuditOff;
sStr g_key;

static const char * canonicalCase(sStr & buf, const char * str, idx len = 0)
{
    if( !str ) {
        return sStr::zero;
    }
    if( !len ) {
        len = sLen(str);
    }
    for(idx i=0; i<len; i++) {
        if( str[i] >= 'A' && str[i] <= 'Z' ) {
            buf.cut0cut();
            sString::changeCase(&buf, str, 0, sString::eCaseLo);
            return buf.ptr(0);
        }
    }
    return str;
}

namespace {

udx projRoleToPerm(sUsr::EProjectRole role) {
    switch( role ) {
        case sUsr::eProjectViewer:
            return ePermCanBrowse | ePermCanRead;
        case sUsr::eProjectDataHandler:
            return ePermCanBrowse | ePermCanRead | ePermCanWrite | ePermCanDownload;
        case sUsr::eProjectContributer:
            return ePermCanBrowse | ePermCanRead | ePermCanWrite | ePermCanDownload | ePermCanExecute;
        case sUsr::eProjectAdmin:
            return ePermCompleteAccess;
    }
    return 0;
}

char projRoleToChar(sUsr::EProjectRole role) {
    switch( role ) {
        case sUsr::eProjectAdmin: return 'A';
        case sUsr::eProjectContributer: return 'C';
        case sUsr::eProjectDataHandler: return 'D';
        case sUsr::eProjectViewer: return 'V';
    }
    return 0;
}

sUsr::EProjectRole projCharToRole(char c) {
    switch( c ) {
        case 'A': return sUsr::eProjectAdmin;
        case 'C': return sUsr::eProjectContributer;
        case 'D': return sUsr::eProjectDataHandler;
        case 'V': return sUsr::eProjectViewer;
    }
    return sUsr::eProjectViewer;
}

}

sUsr::EAuditMode sUsr::audit() const
{
    return g_Audit;
}

bool sUsr::audit(sUsr::EAuditMode mode, const char * oper, const char * fmt, ...) const
{
    if( mode <= audit() ) {
#if _DEBUG
        sStr tmp("pid='%i'; ", getpid());
#else
        sStr tmp;
#endif
        sCallVarg(tmp.vprintf, fmt);
        std::unique_ptr<sSql::sqlProc> p(db().Proc("sp_user_audit"));
        if( p.get() ) {
            p->Add(m_SID).Add(m_Id).Add(oper).Add(tmp);
            return p->execute();
        }
    }
    return false;
}

const char* const sUsr::getKey(void)
{
    return "";
}

const char* sUsr::encodeSID(sStr & sid, sStr & buf)
{
    return "";
}

static
const char* encodePasswordOld(const char* s, const char* salt, sStr& buf)
{
    return "";
}

static const char * hash_seps = " \t\r\n";

static
bool checkPassword(const char * cur_hash, idx cur_hash_len, const char * pass, const char * email, const char * user_id, sStr * upgraded_hash = 0)
{
    return true;
}



bool sUsr::encodeField(sStr * out_encoded_value, sMex * out_encoded_blob, idx encoding, const void * orig_value, idx len) const
{
    return false;
}

bool sUsr::decodeField(sMex * out, idx encoding, const void * encoded_value, idx value_len, const void * encoded_blob, idx blob_len) const
{
    return false;
}

sSql* sUsr::pdb(bool initIfUndefined) const
{
    if( !sm_actual_db && initIfUndefined ) {
        static struct sCfg
        {
            char db[64];
            char server[128];
            char user[64];
            char pass[256];
            udx debug;
            udx rwTimeout;
        } cfg;
        sString::SectVar cfgVars[] = {
            {0, "[HIVE]" _ "db" __, "%s=" HIVE_DB, "%s", &cfg.db},
            {0, "[HIVE]" _ "server" __, "%s=" HIVE_DB_HOST, "%s", &cfg.server},
            {0, "[HIVE]" _ "user" __, "%s=" HIVE_DB_USER, "%s", &cfg.user},
            {0, "[HIVE]" _ "pass" __, "%s=" HIVE_DB_PWD, "%s", &cfg.pass},
            {0, "[HIVE]" _ "debug" __, "%" UDEC "=0", "%s", &cfg.debug},
            {0, "[HIVE]" _ "rwTimeoutSec" __, "%" UDEC "=120", 0, &cfg.rwTimeout}
        };
        const char * hm = getenv(
#ifdef WIN32
            "USERPROFILE"
#else
            "HOME"
#endif
        );
        if( !hm ) {
            hm = "~";
        }
        const char * cfgs00 = "hive.cfg" _ "qapp.cfg" __;
        const char * pfx[] = { ".", hm };
        for( const char * p = cfgs00; p && *p; p = sString::next00(p) ) {
            for(idx i = 0; i < sDim(pfx); ++i) {
                sStr nm("%s/%s", pfx[i], p);
                sFil inp(nm.ptr(), sFil::fReadonly);
                if( inp.ok() && inp.length() ) {
                    sStr rst;
                    sString::cleanMarkup(&rst, inp, inp.length(), "//" _ "/*" __, "\n" _ "*/" __, "\n", 0, false, false, true);
                    sString::xscanSect(rst.ptr(), rst.length(), cfgVars, sDim(cfgVars));
                    if( sm_cfg_db.connect(cfg.db, cfg.server, cfg.user, cfg.pass, cfg.rwTimeout) == sSql::eConnected ) {
                        break;
                    }
                }
            }
        }
        sm_actual_db = &sm_cfg_db;
        if( sm_cfg_db.status != sSql::eConnected ) {
            sQPride* qp = dynamic_cast<sQPride*>(sm_qpride);
            if( qp && qp->sql() && qp->sql()->status == sSql::eConnected) {
                sm_actual_db = qp->sql();
            }
        }
    }
    return sm_actual_db;
}

void sUsr::setQPride(sQPrideBase * qpride)
{
    if( sm_qpride && sm_qpride != qpride ) {
        if( sm_actual_db != &sm_cfg_db ) {
            sm_actual_db = 0;
        }
    }
    if( qpride && sm_qpride != qpride ) {
        sStr buf;
        qpride->cfgStr(&buf, 0, "qm.audit", "off");
        if( strcasecmp(buf, "admin") == 0 ) {
            g_Audit = eUserAuditAdmin;
        } else if( strcasecmp(buf, "login") == 0 ) {
            g_Audit = eUserAuditLogin;
        } else if( strcasecmp(buf, "action") == 0 ) {
            g_Audit = eUserAuditActions;
        } else if( strcasecmp(buf, "full") == 0 ) {
            g_Audit = eUserAuditFull;
        } else {
            g_Audit = eUserAuditOff;
        }
        g_key.cut0cut();
        qpride->cfgStr(&g_key, 0, "qm.session");
    }
    sm_qpride = qpride;
}

void sUsr::session(udx sid, udx uid, idx key, const char* ipaddr)
{
    reset();
#ifdef _DEBUG
#endif
    {
        std::unique_ptr<sSql::sqlProc> p(db().Proc("sp_user_session_v2"));
        if( p.get() ) {
            time_t t = time(0);
            const udx now = mktime(gmtime(&t));
            p->Add(sid).Add(uid).Add(now).Add(key);
            uid = p->uvalue(0);
            if( uid && !init(uid) ) {
                uid = 0;
            } else {
                m_SID = sid;
                m_SIDrnd = key;
            }
        } else {
            uid = 0;
        }
    }
    if( uid == 0 ) {
        if( sid != 0 ) {
            audit(eUserAuditLogin, __func__, "sessionID='%" UDEC "'; rnd='%" DEC "'; result='expired?'", sid, key);
        }
        loginAsGuest();
    }
}

void sUsr::batch(const char * ipaddr)
{
    if( Id() ) {
        time_t t = time(0);
        const udx now = mktime(gmtime(&t));
        sSql::sqlProc* p = db().Proc("sp_user_login");
        sStr log("forked from %" UDEC "%s%s", m_SID, ipaddr ? " " : "", ipaddr ? ipaddr : "");
        p->Add(Id()).Add(log).Add(now).Add(true);
        sVarSet tbl;
        p->getTable(&tbl);
        if( !db().HasFailed() && tbl.rows > 0 && tbl.cols > 1 ) {
            m_SID = tbl.uval(0, 0, 0);
            m_SIDrnd = tbl.ival(0, 1, 0);
        } else {
            err.printf(0, "Persistent session cannot be established, try again later");
        }
        audit(eUserAuditLogin, __func__, "sessionID='%" UDEC "'; user_id=%" UDEC "; rnd='%" DEC "'; source='%s'; result='%s'", m_SID, Id(), m_SIDrnd, ipaddr, err ? err.ptr() : "ok");
    }
}

sRC sUsr::initProjectFolders() const
{
    sDic<udx> groupPerms;
    sVec<Project> projRoles;
    if( sRC rc = allProjectRoles(projRoles, m_currProj.projectId) ) {
        return rc;
    }
    for (idx i = 0; i < projRoles.dim(); i++) {
        udx grpId = projRoles[i].groupId;
        *groupPerms.set(&grpId, sizeof(udx)) = projRoleToPerm(projRoles[i].role);
    }
    const bool su = m_SuperUserMode;
    std::unique_ptr<sUsrFolder> sysFolder(sSysFolder::Inbox(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        sRC rc = clearAndSetPermissions(sysFolder->Id(), groupPerms);
        m_SuperUserMode = su;
        if( rc ) {
            return rc;
        }
    }
    sysFolder.reset(sSysFolder::Trash(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        sRC rc = clearAndSetPermissions(sysFolder->Id(), groupPerms);
        m_SuperUserMode = su;
        if( rc ) {
            return rc;
        }
    }
    return sRC::zero;
}

void sUsr::initFolders(bool keepHierarchy)
{
    const udx group = m_PrimaryGroup;
    const bool su = m_SuperUserMode;
    std::unique_ptr<sUsrFolder> sysFolder(sSysFolder::Home(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        setPermission(group, sysFolder->Id(), ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        m_SuperUserMode = su;
    }
    sysFolder.reset(sSysFolder::Inbox(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        setPermission(group, sysFolder->Id(), ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        m_SuperUserMode = su;
    }
    sysFolder.reset(sSysFolder::Trash(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        setPermission(group, sysFolder->Id(), ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        m_SuperUserMode = su;
    }
    sUsrObjRes us;
    objs2("user-settings", us);
    if( ! us.dim() ) {
        m_SuperUserMode = true;
        sHiveId usid;
        sRC rc = objCreate(usid, "user-settings");
        if( !rc ) {
            setPermission(group, usid, ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        }
        m_SuperUserMode = su;
    }
}

sUsr::ELoginResult sUsr::init2(const udx userId, const char * ipaddr, idx * plogCount)
{
    ELoginResult status = eUserOperational;
    time_t t = time(0);
    udx gmt_time = mktime(gmtime(&t));
    sSql::sqlProc* p = db().Proc("sp_user_login");
    p->Add(userId).Add(ipaddr).Add(gmt_time).Add(false);
    sVarSet tbl;
    p->getTable(&tbl);
    if( !db().HasFailed() && tbl.rows > 0 && tbl.cols > 1) {
        if( init(userId) ) {
            m_SID = tbl.uval(0, 0, 0);
            m_SIDrnd = tbl.ival(0, 1, 0);
            if( plogCount ) {
                *plogCount = tbl.ival(0, 2, 0);
            }
            if( tbl.ival(0, 2, 0) < 2 ) {
                initFolders(true);
            }
        } else {
            status = eUserInternalError;
        }
    } else {
        status = eUserInternalError;
    }
    delete p;
    return status;
}

sUsr::ELoginResult sUsr::login(const char * email, const char * pswd, const char * ipaddr, idx * plogCount)
{
    ELoginResult status = eUserBlocked;
    sVarSet t;
    sStr tmp;
    const char * lemail = db().protect(tmp, email);
    tmp.add0(2);
    sStr sql("SELECT is_active_fg, userID, is_email_valid_fg, is_admin_fg, pswd"
        ", loginTm + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.accountExpireDays') DAY < NOW() AS account_expired"
        ", IFNULL(pswd_changed, NOW() - INTERVAL 200 YEAR) + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.pswdExpireDays') DAY < NOW() AS pswd_expired"
        ", UNIX_TIMESTAMP(NOW()) - UNIX_TIMESTAMP(IFNULL(login_failed_date, NOW() - INTERVAL 1 YEAR)) AS login_failed_secs_ago"
        ", (SELECT IF(val REGEXP '^[0-9]+$', val * 60, 0) FROM QPCfg WHERE par = 'user.loginAttemptsInMinutes') As login_lock_period_sec"
        ", (SELECT IF(val REGEXP '^[0-9]+$', val, 0) != 0 FROM QPCfg WHERE par = 'user.loginAttemptsWarnLockLast') As login_failed_warn"
        ", login_failed_count, (SELECT IF(val REGEXP '^[0-9]+$', val, 0) FROM QPCfg WHERE par = 'user.loginAttemptsMax') AS login_failed_count_max"
        ", (SELECT IF(val REGEXP '^[0-9]+$', val, 0) FROM QPCfg WHERE par = 'user.loginAttemptsLockAccount') AS login_failed_lock_acc"
        " FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
    db().getTable(&t, "%s", sql.ptr());
    if( t.rows == 1 ) {
        const bool lock_account = t.boolval(0, t.colId("login_failed_lock_acc"));
        const char * pp = t.val(0, t.colId("pswd"));
        sStr upgraded_pp;
        if( checkPassword(pp, 0, pswd, email, t.val(0, t.colId("userID")), &upgraded_pp) ) {
            if( upgraded_pp.length() ) {
                db().execute("UPDATE UPUser SET pswd='%s', modifTm = CURRENT_TIMESTAMP WHERE userID=%" UDEC, upgraded_pp.ptr(), t.uval(0, t.colId("userID")));
                audit(eUserAuditLogin, "upgrade_password", "userID=%" UDEC "; from='%s'; to='%s'", t.uval(0, t.colId("userID")), pp, upgraded_pp.ptr());
                pp = upgraded_pp.ptr();
            }
        } else {
            if( t.boolval(0, t.colId("login_failed_warn")) && (t.ival(0, t.colId("login_failed_count_max")) - t.ival(0, t.colId("login_failed_count")) == 2) ) {
                status = lock_account ? eUserLoginAttemptsWarn1Left : eUserNotFound;
            } else if( lock_account && t.boolval(0, t.colId("login_failed_warn")) && (t.ival(0, t.colId("login_failed_count_max")) - t.ival(0, t.colId("login_failed_count")) == 1) ) {
                status = lock_account ? eUserLoginAttemptsNowLocked : eUserNotFound;
            } else {
                status = eUserNotFound;
            }
            if( t.uval(0, t.colId("login_failed_secs_ago")) <= t.uval(0, t.colId("login_lock_period_sec")) ) {
                db().execute("UPDATE UPUser SET login_failed_date = IFNULL(login_failed_date, NOW()), login_failed_count = login_failed_count + 1 WHERE userID=%" UDEC, t.uval(0, t.colId("userID")));
            } else {
                db().execute("UPDATE UPUser SET login_failed_date = NOW(), login_failed_count = 1 WHERE userID=%" UDEC, t.uval(0, t.colId("userID")));
            }
        }
        if( status == eUserBlocked ) {
            const bool is_admin = t.boolval(0, t.colId("is_admin_fg"));
            if( !t.boolval(0, t.colId("is_email_valid_fg")) && !is_admin ) {
                status = eUserEmailNotValidated;
            } else if( t.boolval(0, t.colId("account_expired")) && !is_admin ) {
                status = eUserAccountExpired;
            } else if( t.boolval(0, t.colId("pswd_expired")) ) {
                status = eUserPswdExpired;
            } else if( t.uval(0, t.colId("login_failed_count_max")) > 0 && t.uval(0, t.colId("login_failed_count")) >= t.uval(0, t.colId("login_failed_count_max")) ) {
                if( lock_account ) {
                    status = eUserLoginAttemptsTooMany;
                } else {
                    if( t.uval(0, t.colId("login_failed_secs_ago")) <= t.uval(0, t.colId("login_lock_period_sec")) ) {
                        status = eUserLoginAttemptsTooManyTryLater;
                    } else if( t.boolval(0, t.colId("is_active_fg")) ) {
                        status = eUserOperational;
                    }
                }
            } else if( t.boolval(0, t.colId("is_active_fg")) ) {
                status = eUserOperational;
            }
        }
    } else {
        status = eUserNotFound;
    }
    if( status == eUserOperational ) {
        status = init2(t.uval(0, t.colId("userID")), ipaddr, plogCount);
    }
    audit(eUserAuditLogin, __func__, "email='%s'; sessionID='%" UDEC "'; rnd='%" DEC "'; source='%s'; result='%" DEC, email, m_SID, m_SIDrnd, ipaddr, (idx)status);
    return status;
}

void sUsr::logout(const char * ipaddr)
{
    time_t t = time(0);
    const udx now = mktime(gmtime(&t));
    sSql::sqlProc* p = db().Proc("sp_user_logout");
    p->Add(m_Id).Add(m_SID).Add(now);
    p->execute();
    delete p;
    audit(eUserAuditLogin, __func__, "source='%s'; result='%" UDEC ": %s'", ipaddr, db().Get_errno(), db().Get_error().ptr());
    loginAsGuest();
}

sUsr::ELoginResult sUsr::loginByEmail(const char * email, const char * ipaddr, idx * plogCount, const char * log_data)
{
    sVarSet t;
    sStr tmp;
    db().getTable(&t, "SELECT is_active_fg, is_email_valid_fg, userID"
        ", (loginTm + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.accountExpireDays') DAY < NOW() AND !is_admin_fg) AS account_expired"
        ", IFNULL(pswd_changed, NOW() - INTERVAL 200 YEAR) + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.pswdExpireDays') DAY < NOW() AS pswd_expired"
        ", login_failed_count >= (SELECT IF(val REGEXP '^[0-9]+$', val, login_failed_count - 1) FROM QPCfg WHERE par = 'user.loginAttemptsMax')"
        "FROM UPUser WHERE email = '%s' AND `type` = 'user'",  db().protect(tmp, email));
    ELoginResult res = eUserNotFound;
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) ) {
            res = eUserBlocked;
        } else if( !t.uval(0, 1) ) {
            res = eUserEmailNotValidated;
        } else if( t.boolval(0, 3) ) {
            res = eUserAccountExpired;
        } else if( t.boolval(0, 4) ) {
            res = eUserPswdExpired;
        } else if( t.boolval(0, 5) ) {
            res = eUserLoginAttemptsTooMany;
        } else {
            res = init2(t.uval(0, t.colId("userID")), ipaddr, plogCount);
        }
    }
    audit(eUserAuditLogin, __func__, "email='%s'; result='%" UDEC "'; auth='%s'", email, (udx)res, log_data ? log_data : "" );
    return res;
}

sUsr::ELoginResult sUsr::loginByToken(const char * email, const char * token, const char * ipaddr, idx * plogCount, const char * log_data)
{
    sVarSet t;
    sStr tmp;
    db().getTable(&t, "SELECT u.is_active_fg, u.is_email_valid_fg, u.userID, g.groupID AS creatorID"
        ", (u.loginTm + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.accountExpireDays') DAY < NOW() AND !u.is_admin_fg) AS account_expired"
        " FROM UPUser u JOIN UPGroup g USING(userID) WHERE u.email = '%s' AND u.`type` = 'user' AND g.flags = -1",  db().protect(tmp, email));
    ELoginResult res = eUserNotFound;
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) ) {
            res = eUserBlocked;
        } else if( !t.uval(0, 1) ) {
            res = eUserEmailNotValidated;
        } else if( t.boolval(0, 4) ) {
            res = eUserAccountExpired;
        } else {
            const char * creatorID = t.val(0, t.colId("creatorID"));
            if ( creatorID && sLen(creatorID) ) {
                sUsrObjRes token_res;
                bool old_SuperUserMode = m_SuperUserMode;
                m_SuperUserMode = true;
                objs2("^user-settings$", token_res, 0, "_creator", creatorID);
                m_SuperUserMode = old_SuperUserMode;

                if ( token_res.dim() == 1 ) {
                    const sHiveId * id = NULL;
                    sUsrObj * o = NULL;
                    if ( ( id = token_res.firstId() ) && ( o = objFactory(*id) ) ) {
                        sUsrObjPropsTree props_tree(*this, o->getTypeName());
                        o->propBulk(props_tree.getTable(), 0, "account_token_hash" _ "account_token_expiration" __);
                        props_tree.useTable(props_tree.getTable());

                        const sUsrObjPropsNode * node = props_tree.firstChild("account_token");
                        while ( node ) {
                            const char * hash = node->findValue("account_token_hash");
                            if ( hash && sPassword::checkPassword(hash, 0, token) ) {
                                const sUsrObjPropsNode * expir_node = node->find("account_token_expiration");
                                if ( expir_node && expir_node->hasValue() && time(0) > expir_node->ivalue() ) {
                                    res = eUserTokenExpired;
                                } else {
                                    res = init2(t.uval(0, t.colId("userID")), ipaddr, plogCount);
                                }
                                break;
                            }
                            node = node->nextSibling("account_token");
                        }
                    } else {
                        res = eUserInternalError;
                    }
                }
            } else {
                res = eUserInternalError;
            }
        }
    }
    audit(eUserAuditLogin, __func__, "email='%s'; result='%" UDEC "'; auth='%s'", email, (udx)res, log_data ? log_data : "" );
    return res;
}

sUsr::ELoginResult sUsr::loginAsGuest(void)
{
    udx g = db().uvalue(0, "SELECT userID FROM UPUser WHERE email = '%s' AND `type` = 'system'", "guest");
    if( g && !init(g) ) {
        g = 0;
    }
    if( !g ) {
        reset();
        audit(eUserAuditLogin, __func__, "result='guest corrupt'");
    }
    return g ? eUserOperational : eUserNotFound;
}

sUsr::sUsr(idx usrid)
{
    m_SuperUserMode = false;
    reset();
    init(usrid);
}

sUsr::sUsr(const char* service_name, bool su_mode)
{
    m_SuperUserMode = false;
    udx srv = 0;
    if( service_name && service_name[0] ) {
        sStr tmp;
        srv = db().uvalue(0, "SELECT userID FROM UPUser WHERE email = '%s' AND `type` = 'service'", db().protect(tmp, service_name));
        if( srv ) {
            reset();
            if( su_mode ) {
                m_SuperUserMode = true;
            }
            if( !init(srv) ) {
                srv = 0;
            }
            if( su_mode ) {
                m_SuperUserMode = false;
            }
        }
    }
    if( !srv ) {
        reset();
        audit(eUserAuditLogin, "loginAsService", "result='service not found [%s]'", service_name);
    } else {
        m_SuperUserMode = su_mode;
    }
}

const char* sUsr::Name(sStr* buf) const
{
    static sStr sbuf;
    if( !buf ) {
        buf = &sbuf;
        sbuf.empty();
    }
    idx pos = buf->pos();
    bool f = m_First && *m_First.ptr();
    bool l = m_Last && *m_Last.ptr();
    buf->printf("%s%s%s", f ? m_First.ptr() : "", f && l ? " " : "", l ? m_Last.ptr() : "");
    return buf->ptr(pos);
}

void sUsr::reset(void)
{
    if( m_ObjPermission.get() && audit() >= eUserAuditFull ) {
        sStr log;
        for(idx k = 0; k < m_ObjPermission->dim(); ++k) {
            const sHiveId * id = static_cast<const sHiveId*>(m_ObjPermission->id(k));
            log.printf(",%s", id->print());
            if( log.length() > 256 ) {
                audit(audit(), "accessed", "objID='%s'", log.ptr(1));
                log.cut(0);
            }
        }
        if( log.length() > 1 ) {
            audit(audit(), "accessed", "objID='%s'", log.ptr(1));
        }
    }
    m_Id = 0;
    m_IsAdmin = false;
    m_IsGuest = false;
    m_IsEmailValid = false;
    m_Email.destroy();
    m_First.destroy();
    m_Last.destroy();
    m_PrimaryGroup = 0;
    m_membership.destroy();
    m_SID = 0;
    m_SIDrnd = 0;
    m_ObjPermission.reset();
    m_AllowExpiredObjects = false;
    err.cut0cut(0);
    m_currProj.projectId = 0;
    m_projMembership.destroy();
}

bool sUsr::init(udx userId, udx projectId)
{
    if( userId == 0 ) {
        reset();
        return true;
    }

    if( m_Id != userId || projectId ) {
        sSql::sqlProc* p = db().Proc("sp_user_init");
        if( p ) {
            sVarSet usr;
            p->Add(userId);
            p->getTable(&usr);
            if( !db().HasFailed() && usr.rows > 1 ) {
                reset();
                m_Id = userId;
                m_IsAdmin = usr.uval(0, usr.colId("is_admin_fg"));
                m_IsGuest = sIs(usr.val(0, usr.colId("type")), "system") && sIs(usr.val(0, usr.colId("email")), "guest");
                m_IsEmailValid = usr.uval(0, usr.colId("is_email_valid_fg"));
                m_Email.replace(usr.val(0, usr.colId("email")));
                m_First.replace(usr.val(0, usr.colId("first_name")));
                m_Last.replace(usr.val(0, usr.colId("last_name")));

                setProject(projectId);
                sHiveId::mapDomainReset();
                getDomainIdDescr(0);
            }
            delete p;
        }
    }
    return m_Id != 0 && m_Id == userId;
}

sSql::sqlProc* sUsr::getProc(const char* sp_name) const
{
    sSql::sqlProc* p = (sp_name && sp_name[0]) ? db().Proc(sp_name) : 0;
    if( p ) {
        if( m_currProj.projectId ) {
            p->Add(m_currProj.groupId).Add(m_SuperUserMode ? " TRUE " : m_projMembership.ptr());
        } else {
            p->Add(m_PrimaryGroup).Add(m_SuperUserMode ? " TRUE " : m_membership.ptr());
        }
    }
    return p;
}



bool sUsr::sendEmailValidation(const char* baseURL, const char* email, const char* firstName, const char* lastName)
{
    sVarSet t;
    sStr tmp;
    if( !firstName || !lastName ) {
        db().getTable(&t, "SELECT first_name, last_name, email FROM UPUser WHERE email = '%s' AND `type` = 'user'",  db().protect(tmp, email));
        if( t.rows == 1 ) {
            firstName = t.val(0, t.colId("first_name"));
            lastName = t.val(0, t.colId("last_name"));
            email = t.val(0, t.colId("email"));
        }
    }
    if( email && firstName && lastName ) {
        sUsr admin("qpride");
        sStr body("%s %s,\n\n"
                "Your account on HIVE was successfully created.\n"
                "Now, in order to verify your email address, please, click the link below:\n"
                "%s?cmd=userV1&emailAct=%s\n"
                "\nHIVE Team.\n", firstName, lastName, baseURL, URLEncode(email, tmp, eUrlEncode_ProcessMarkChars));
        sUsrEmail eml(admin, email, "HIVE registration");
        eml.body("%s", body.ptr());
        sQPrideBase * qp = QPride();
        if( qp ) {
            sStr adminEmail;
            qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
            if( adminEmail ) {
                eml.addRecipient(sUsrEmail::eBcc, adminEmail);
            }
        }
        eml.draft(false);
    } else {
        err.printf(0, "Email address is not recognized");
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
    return !err;
}

bool sUsr::sendEmailOnFinish(const idx status, sUsrObj& proc_obj)
{
    sVarSet t;
    sStr tmp;

    const char* email = Email();

    if( email ) {
        sQPrideBase * qp = QPride();
        if( qp ) {
            bool send_email = false;

            sStr process_notify;
            proc_obj.propGet("notify", &process_notify);

            if ( !process_notify ) {
                process_notify.printf(0,"Use value from user settings");
            }

            sStr user_notify;
            sUsrObjRes us;
            objs2("user-settings", us);
            if( us.dim() ) {
                const sHiveId * hive_id = us.firstId();
                if ( hive_id ) {
                    sUsrObj setting_obj(*this, *hive_id);
                    setting_obj.propGet("notify", &user_notify);
                }
            }

            if ( !user_notify ) {
                user_notify.printf(0,"Failures Only");
            }

            if ( sIsExactly(process_notify, "Use value from user settings") ) {
                if ( sIsExactly(user_notify, "Always") ) {
                    send_email = true;
                }
                else if ( sIsExactly(user_notify, "Failures Only") && status > sQPrideBase::eQPReqStatus_Done ) {
                    send_email = true;
                }
            }
            else if ( sIsExactly(process_notify, "Always") ) {
                send_email = true;
            }
            else if ( sIsExactly(process_notify, "Failures Only") && status > sQPrideBase::eQPReqStatus_Done ) {
                send_email = true;
            }


            if (send_email) {
                const char* name = Name();
                name = name ? name : "Dear User";

                const char* fin_status;
                const char* body_phrase;
                switch(status) {
                    case sQPrideBase::eQPReqStatus_Suspended:
                        fin_status = "Failed";
                        body_phrase = "was suspended by a user or system";
                        break;
                    case sQPrideBase::eQPReqStatus_Done:
                        fin_status = "Finished";
                        body_phrase = "finished successfully";
                        break;
                    default:
                        fin_status = "Failed";
                        body_phrase = "finished unsuccessfully";
                }

                sStr url;
                qp->cfgStr(&url, 0, "baseURL");

                sStr oid;
                proc_obj.IdStr(&oid);
                oid.add0(2);

                sStr submitter;
                sUsrObjRes actions;
                objs2("^action$+", actions, 0, "name", "open", "url");

                for(sUsrObjRes::IdIter it = actions.first(); actions.has(it); actions.next(it)) {
                    sUsrObj action(*this, *actions.id(it));
                    const char* type_name = action.propGet("type_name");
                    if( action.Id() && type_name && strcmp(type_name, proc_obj.getTypeName()) == 0 ) {
                        sString::searchAndReplaceStrings(&submitter, action.propGet("url"), 0, "$(ids)" __, oid.ptr(), sIdxMax, true);
                        if( submitter ) {
                            break;
                        }
                    }
                }
                if ( !submitter ) {
                    submitter.printf("?cmd=home");
                }

                sStr subject_line("Process %s %s", oid.ptr(), fin_status);
                sStr body("%s,\n\n"
                        "Your computation %s.\n"
                        "%sdna.cgi%s\n"
                        "\nHIVE Team.\n", name, body_phrase, url.ptr(), submitter.ptr());

                sUsrEmail eml(*this, email, subject_line.ptr(), body.ptr());
                const idx on = qp->cfgInt(0, "ccAdminOnProcFail", 0);
                if( on ) {
                    sStr adminEmail;
                    qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
                    if( adminEmail ) {
                        eml.addRecipient(sUsrEmail::eBcc, adminEmail);
                    }
                }
                eml.draft(false);
            }
        }
    } else {
        err.printf(0, "Email address is not recognized");
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email ? email : "", err ? err.ptr() : "ok");
    return !err;
}

bool sUsr::verifyEmail(const char* baseURL, const char* email)
{
    sVarSet t;
    sStr tmp;
    db().getTable(&t, "SELECT is_active_fg, is_email_valid_fg FROM UPUser WHERE email = '%s' AND `type` = 'user'",  db().protect(tmp, email));
    if( t.rows == 1 ) {
        if( !t.uval(0, 1) ) {
            db().execute("UPDATE UPUser SET is_email_valid_fg = TRUE WHERE email = '%s' AND `type` = 'user'", tmp.ptr());
        }
        if( !t.uval(0, 0) ) {
            sendAccountActivation(baseURL, email);
            err.printf(0, "Account activation request was submitted");
        }
    } else {
        err.printf(0, "Email address is not recognized");
        return false;
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
    return true;
}

bool sUsr::sendAccountActivation(const char* baseURL, const char* email)
{
    sVarSet t;
    sStr tmp;
    db().getTable(&t, "SELECT first_name, last_name, email FROM UPUser WHERE email = '%s' AND `type` = 'user'",  db().protect(tmp, email));
    if( t.rows == 1 ) {
        sUsr admin("qpride");
        sStr body("Dear Admin,\n\n"
                "%s %s has applied for access to HIVE from email %s.\n"
                "To activate this account, please, click the link below:\n"
                "%s?cmd=userV3&emailAct=%s\n"
                "\nHIVE Team.\n", t.val(0, 0), t.val(0, 1), t.val(0, 2), baseURL, URLEncode(t.val(0, 2), tmp, eUrlEncode_ProcessMarkChars));
        sQPrideBase * qp = QPride();
        sStr adminEmail;
        if( qp ) {
            qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
        }
        if( adminEmail ) {
            sUsrEmail eml(admin, adminEmail, "HIVE account activation request");
            eml.body("%s", body.ptr());
            eml.draft(false);
        } else {
            err.printf(0, "Admin email address is not configured");
        }
    } else {
        err.printf(0, "Email address is not recognized");
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
    return !err;
}

bool sUsr::accountActivate(const char* baseURL, const char* email)
{
    sVarSet t;
    sStr tmp;
    db().getTable(&t, "SELECT is_active_fg, "
        "loginTm + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.accountExpireDays') DAY < NOW() AS account_expired "
        "FROM UPUser WHERE email = '%s' AND `type` = 'user'",  db().protect(tmp, email));
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) || t.boolval(0, 1) ) {
            db().execute("UPDATE UPUser SET is_active_fg = TRUE, loginTm = NULL WHERE email = '%s' AND `type` = 'user'", tmp.ptr());
            t.empty();
            db().getTable(&t, "SELECT is_active_fg, first_name, last_name, email FROM UPUser WHERE email = '%s' AND `type` = 'user'",  tmp.ptr());
            if( t.rows == 1 && t.uval(0, 0) ) {
                sUsr admin("qpride");
                sStr body("%s %s,\n\n"
                        "Your account on HIVE is now activated.\n"
                        "Please click here to login: %s?cmd=login&login=%s\n"
                        "\nHIVE Team.\n", t.val(0, 1), t.val(0, 2), baseURL, URLEncode(email, tmp, eUrlEncode_ProcessMarkChars));
                sUsrEmail eml(admin, t.val(0, 3), "HIVE account activation confirmation");
                eml.body("%s", body.ptr());
                sQPrideBase * qp = QPride();
                if( qp ) {
                    sStr adminEmail;
                    qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
                    if( adminEmail ) {
                        eml.addRecipient(sUsrEmail::eBcc, adminEmail);
                    }
                }
                eml.draft(false);
            } else {
                err.printf(0, "Activation unsuccessful");
            }
        } else {
            err.printf(0, "Was active already");
        }
    } else {
        err.printf(0, "Email address is not recognized");
        return false;
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
    return true;
}

bool sUsr::groupActivate(idx groupId)
{
    bool res = false;
    sVarSet t;
    db().getTable(&t, "SELECT is_active_fg FROM UPGroup WHERE groupID = %" UDEC,  groupId);
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) ) {
            db().execute("UPDATE UPGroup SET is_active_fg = TRUE WHERE groupID = %" UDEC, groupId);
            t.empty();
            db().getTable(&t, "SELECT is_active_fg, groupPath FROM UPGroup WHERE groupID = %" UDEC, groupId);
            if( t.rows == 1 && t.uval(0, 0) ) {
                err.printf(0, "Group membership activation successful");
                res = true;
            } else {
                err.printf(0, "Activation unsuccessful");
            }
        } else {
            err.printf(0, "Was active already");
            res = true;
        }
    } else {
        err.printf(0, "group id %" UDEC " is not found", groupId);
    }
    audit(eUserAuditAdmin, __func__, "groupID='%" UDEC "'; result='%s'", groupId, !res ? err.ptr() : "ok");
    return res;
}

bool sUsr::groupCreate(const char* name, const char* abbr, const char* parent)
{
    bool res = false;
    if( !name || !name[0] || !abbr || !abbr[0] ) {
        err.printf("Missing group name and/or abbreviation");
    } else if( !parent || !parent[0] || parent[0] != '/' || parent[strlen(parent) - 1] != '/' ) {
        err.printf("Invalid parent group path");
    } else {
        sStr protectedName, protectedAbbr, protectedParent;
        db().protect(protectedName, name);
        db().protect(protectedAbbr, abbr);
        db().protect(protectedParent, parent);

        if( db().uvalue(0, "SELECT userID FROM UPUser WHERE first_name = '%s%s/' OR (first_name LIKE '%s%%' AND last_name LIKE '%s') AND `type` = 'group'", protectedParent.ptr(), protectedAbbr.ptr(), protectedParent.ptr(), protectedName.ptr()) != 0 ) {
            err.printf("Group with same name and/or abbreviation already exists.");
        } else if( updateStart() ) {
            db().execute("INSERT INTO UPUser (first_name, last_name, email, is_active_fg, is_email_valid_fg, pswd, `type`, createTm)"
                    " VALUES ('%s%s/', '%s', '%s', TRUE, TRUE, '--not an account--', 'group', CURRENT_TIMESTAMP)", protectedParent.ptr(), protectedAbbr.ptr(), protectedName.ptr(), m_Email.ptr());
            if( !db().HasFailed() ) {
                db().execute("INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath)"
                        " VALUES ((SELECT userID FROM UPUser WHERE first_name = '%s%s/' AND `type` = 'group'), -1, TRUE, '%s%s/')", protectedParent.ptr(), protectedAbbr.ptr(), protectedParent.ptr(), protectedAbbr.ptr());
            }
            if( !db().HasFailed() ) {
                res = updateComplete();
            }
            if( !res ) {
#ifdef _DEBUG
                err.printf("SQL error: [%" UDEC "] '%s'", db().Get_errno(), db().Get_error().ptr());
#endif
                updateAbandon();
            }
        }
    }
    audit(eUserAuditAdmin, __func__, "name='%s'; abbr='%s'; result='%s'", name, abbr, res ? "ok" : err.ptr());
    return res;
}

bool sUsr::contact(const char * from_email, const char * subject, const char * body, const char * autoreply)
{
    err.cut(0);
    if( !from_email || strchr(from_email, '@') == 0 || strchr(from_email, '.') == 0 ) {
        err.printf(0, "Invalid email address!");
    } else {
        sUsr admin("qpride");
        sQPrideBase * qp = QPride();
        sStr adminEmail, msg("%s", (body && body[0]) ? body : "empty message body");
        if( qp ) {
            qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
        }
        if( autoreply && autoreply[0] ){
           sUsrEmail emlr(admin, from_email, (subject && subject[0]) ? subject : "No subject", autoreply);
           emlr.draft(false);
        }
        if( adminEmail ) {
            sUsrEmail eml(admin, adminEmail, (subject && subject[0]) ? subject : "No subject", msg);
            eml.draft(false);
        } else {
            err.printf(0, "Admin email address is not configured");
        }
    }
    audit(eUserAuditAdmin, __func__, "sessionID='%" UDEC "'; user_id=%" UDEC "; from='%s'; result='%s'", m_SID, Id(), from_email, err ? err.ptr() : "ok");
    return !err;
}

udx sUsr::addPasswordResetID(const char* email)
{
    udx pswd_reset_id = 0;
    sVarSet t;
    sStr tmp;
    if( !email || !email[0] ) {
        email = m_Email.ptr();
    }
    db().getTable(&t, "SELECT is_active_fg, is_email_valid_fg FROM UPUser WHERE email = '%s' AND `type` = 'user'",  db().protect(tmp, email));
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) ) {
            err.printf(0, "Account is disabled.");
        } else if( !t.uval(0, 1) ) {
            err.printf(0, "Email address not verified, try to login again and click on link to resend verification email.");
        } else {
            sUsr admin("qpride");
            udx r = rand(), r1 = rand();
            db().execute("UPDATE UPUser SET pswd_reset_id = IF(pswd_reset_id = %" UDEC ", %" UDEC ", %" UDEC ") WHERE email = '%s' AND `type` = 'user'", r, r1, r, tmp.ptr());
            t.empty();
            db().getTable(&t, "SELECT pswd_reset_id FROM UPUser WHERE email = '%s' AND `type` = 'user'",  tmp.ptr());
            if( t.rows == 1 && (t.uval(0, 0) == r || t.uval(0, 0) == r1) ) {
                pswd_reset_id = t.uval(0, 0);
            } else {
                err.printf(0, "Request unsuccessful, try again later.");
            }
        }
    } else {
        err.printf(0, "Email address not recognized.");
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; pswd_reset_id=%" UDEC "; result='%s'", email, pswd_reset_id, err ? err.ptr() : "ok");
    return err ? 0 : pswd_reset_id;
}

bool sUsr::sendForgotten(const char* baseURL, const char* email)
{
    sStr tmp;
    db().protect(tmp, email);
    if( udx pswd_reset_id = addPasswordResetID(email) ) {
        sVarSet t;
        db().getTable(&t, "SELECT first_name, last_name, email, pswd_reset_id FROM UPUser WHERE email = '%s' AND `type` = 'user'",  tmp.ptr());
        if( t.rows == 1 && t.uval(0, 3) == pswd_reset_id ) {
            sUsr admin("qpride");
            sStr body("%s %s,\n\n"
                      "To complete your request to reset password click the link below:\n"
                      "%s?cmd=pswdSet&login=%s&pswd=%" UDEC "&x=%" UDEC "\n"
                      "\nHIVE Team.\n", t.val(0, 0), t.val(0, 1), baseURL, URLEncode(t.val(0, 2), tmp, eUrlEncode_ProcessMarkChars), t.uval(0, 3), (udx)(time(0) + 24 * 60 * 60));
            sUsrEmail eml(admin, email, "HIVE password notification");
            eml.body("%s", body.ptr());
            sQPrideBase * qp = QPride();
            if( qp ) {
                sStr adminEmail;
                qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
                if( adminEmail ) {
                    eml.addRecipient(sUsrEmail::eBcc, adminEmail);
                }
            }
            eml.draft(false);
        } else {
            err.printf(0, "Unexpected system state, try again later.");
        }
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
    return !err;
}

bool sUsr::passwordCheckQuality(const char * mod, const char * mod1)
{
    sQPrideBase * qp = QPride();
    const bool checkComplexity = qp ? qp->cfgInt(0, "user.pswdCheckComplexity", 1) : true;
    const idx minLen = qp ? qp->cfgInt(0, "user.pswdMinLength", 15) : 15;
    const bool useSymb = qp ? qp->cfgInt(0, "user.pswdUseSymbols", 0) : false;
    bool res = false;

    if( !mod ) {
        mod = "\x01";
    }
    if( !mod1 ) {
        mod1 = "\x02";
    }
    idx cAlphaLow = 0, cAlphaCap = 0, cNum = 0, cLen = 0, cSymb = 0;
    for(const char * p = mod; *p; ++p) {
        ++cLen;
        if( *p >= 'a' && *p <= 'z' ) {
            ++cAlphaLow;
        } else if( *p >= 'A' && *p <= 'Z' ) {
            ++cAlphaCap;
        } else if( *p >= '0' && *p <= '9' ) {
            ++cNum;
        } else if( strchr("~`!@#$%^&*()_-+={[}]|\\:;\"\'<,>.?/", *p) ) {
            ++cSymb;
        }
    }
    if( strcmp(mod, mod1) ) {
        err.printf(0, "new password and confirmation do not match!");
    } else if( checkComplexity && (cLen < minLen || cAlphaLow == 0 || cAlphaCap == 0 || cNum == 0 || (useSymb && cSymb == 0)) ) {
        err.printf(0, "new password does not satisfy minimum complexity criteria: more than %" DEC " symbols, with one lower, one caps, one numeric%s", minLen, useSymb ? ", one specail" : "");
    } else {
        res = true;
    }
    return res;
}

bool sUsr::passwordReset(const char* email, udx pswd_reset_id, const char * mod, const char * mod1)
{
    bool res = false;
    udx userId = 0;
    if( pswd_reset_id ) {
        sVarSet t;
        sStr tmp;
        db().getTable(&t, "SELECT userID, is_active_fg, is_email_valid_fg FROM UPUser WHERE email = '%s' AND pswd_reset_id = %" UDEC " AND `type` = 'user'",
            db().protect(tmp, email), pswd_reset_id);
        if(t.rows == 1 && t.uval(0, 0) && t.uval(0, 1) && t.uval(0, 2) ) {
            userId = t.uval(0, 0);
            res = passwordReset(userId, mod, mod1);
        }
    }
    if( !userId ) {
        err.printf(0, "password reset request is invalid or has expired");
        audit(eUserAuditLogin, __func__, "result='%s'", err.ptr());
    }
    return res;
}

bool sUsr::passwordReset(udx userId, const char * mod, const char * mod1)
{
    bool res = false;
    if( passwordCheckQuality(mod, mod1) ) {
        sVarSet t;
        db().getTable(&t, "SELECT pswd, email FROM UPUser WHERE userID = %" UDEC, userId);
        if( t.rows == 1 ) {
            const char * cur_hash = t.val(0, 0);
            const char * email = t.val(0, 1);
            sStr hashes_buf("NULL");
            bool password_reused = false;

            if( idx num_keep_old = db().ivalue("SELECT val FROM QPCfg WHERE par = 'user.pswdKeepOldQty'", 0) ) {
                sStr userID_str("%" UDEC, userId);
                hashes_buf.printf(0, "'%s ", cur_hash);
                idx cur_hash_pos = 1;
                db().svalue(hashes_buf, "SELECT pswd_prev_list FROM UPUser WHERE userID = %" UDEC, userId);
                for(idx ihash = 0; ihash < num_keep_old; ihash++) {
                    idx ws = strspn(hashes_buf.ptr(cur_hash_pos), hash_seps);
                    idx cur_hash_len = strcspn(hashes_buf.ptr(cur_hash_pos + ws), hash_seps);
                    if( !cur_hash_len ) {
                        break;
                    }
                    if( checkPassword(hashes_buf.ptr(cur_hash_pos + ws), cur_hash_len, mod, email, userID_str, 0) ) {
                        password_reused = true;
                        break;
                    }
                    cur_hash_pos += ws + cur_hash_len;
                    cur_hash_len = 0;
                }
                hashes_buf.cutAddString(cur_hash_pos, "'", 1);
            }

            if( password_reused ) {
                err.printf(0, "reusing old passwords is not permitted!");
            } else {
                sStr tmp;
                db().execute("UPDATE UPUser SET pswd='%s', logCount = IF(logCount <= 0, 1, logCount), pswd_reset_id = NULL, "
                             "pswd_changed = NOW(), pswd_prev_list=%s, modifTm = CURRENT_TIMESTAMP WHERE userID=%" UDEC, sPassword::encodePassword(tmp, mod), hashes_buf.ptr(), userId);
                res = true;
            }
        }
    }
    audit(eUserAuditLogin, __func__, "result='%s'", err.ptr());
    return res;
}

idx sUsr::update(const bool isnew, const char * email, const char * password, const char * newpass1, const char * newpass2, idx statusNeed,
        const char * firstName, const char * lastName, sVec<idx>& groups, idx softExpiration, idx hardExpiration, const char* baseURL)
{
    sStr log;
    bool result = true;

    err.cut(0);
    udx userId = 0;

    sStr email_protected, firstName_protected, lastName_protected;

    db().protect(email_protected, email);
    const char * lemail = email_protected.ptr();

    db().protect(firstName_protected, firstName);
    const char * lfirstName = firstName_protected.ptr();

    db().protect(lastName_protected, lastName);
    const char * llastName = lastName_protected.ptr();

    while( updateStart() ) {
        if( isnew ) {
            const bool prohibitSelfRegistration = QPride() ? QPride()->cfgInt(0, "user.prohibitSelfRegistration", 0) : false;
            if( prohibitSelfRegistration && !m_IsAdmin ) {
                err.printf(0, "Need admin privileges to register a new user");
                break;
            }
            if( !email || strchr(email ,'@') == 0 || strchr(email ,'.') == 0 ) {
                err.printf(0, "Invalid email address!");
                break;
            }
            if( !lastName || !lastName[0] || !firstName || !firstName[0] ) {
                err.printf(0, "First and Last names are required!");
                break;
            }
            if( !passwordCheckQuality(newpass1, newpass2) ) {
                break;
            }
            userId = db().uvalue(0, "SELECT userID FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
            if( userId > 0 ) {
                err.printf(0, "Email address is already in use!");
                break;
            } else {
                db().execute("INSERT INTO UPUser (email, is_active_fg, pswd, logCount, `type`, first_name, last_name, createTm)"
                                    " VALUES ('%s', FALSE, '--TBD--', %u, 'user', '%s', '%s', CURRENT_TIMESTAMP)",
                                        lemail, m_IsAdmin ? 0 : 1, lfirstName, llastName);
                userId = db().uvalue(0, "SELECT userID FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
                if( userId ) {
                    sStr tmp("%" UDEC, userId);
                    tmp.add0(2);
                    const char * pswd = sPassword::encodePassword(tmp, newpass1);
                    db().execute("UPDATE UPUser SET pswd = '%s', pswd_changed = NOW() WHERE userID = %" UDEC, pswd, userId);
                    db().execute("insert into UPGroup (userID, flags, is_active_fg, groupPath) values(%" UDEC ", -1, TRUE, '/everyone/users/%s')", userId, lemail);
                    if( !db().HasFailed() ) {
                        log.printf("new_user='%s'; ", email);
                        sendEmailValidation(baseURL, email, firstName, lastName);
                        firstName = lastName = 0;
                    } else {
                        err.printf(0, "Registration failed, please, come back later!");
#if _DEBUG
                        err.printf(" mysql: %" UDEC " '%s'", db().Get_errno(), db().Get_error().ptr());
#endif
                        break;
                    }
                } else {
                    err.printf(0, "Registration failed, please, come back later!");
                    break;
                }
            }
        } else {
            sVarSet t;
            sStr tmp;
            db().getTable(&t, "SELECT userID, pswd FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
            if( t.rows != 1 || !checkPassword(t.val(0, t.colId("pswd")), 0, password, email, t.val(0, t.colId("userID")), 0) ) {
                err.printf(0, "Current password is not recognized!");
                break;
            }
            userId = t.uval(0, 0);
            if( (newpass1 || newpass2) && !passwordReset(userId, newpass1, newpass2) ) {
                break;
            }
        }
        break;
    }
    if( !err ) {
        if( result && firstName && firstName[0] && strcmp(m_First, firstName) != 0 ) {
            log.addString("old_first='");
            db().svalue(log, "SELECT first_name FROM UPUser WHERE userID = %" UDEC, userId);
            log.addString("'; ");
            db().execute("update UPUser set first_name = '%s', modifTm = CURRENT_TIMESTAMP where userID = %" UDEC, lfirstName, userId);
            result = !db().HasFailed();
        }
        if( result && lastName && lastName[0] && strcmp(m_Last, lastName) != 0 ) {
            log.addString("old_last='");
            db().svalue(log, "SELECT last_name FROM UPUser WHERE userID = %" UDEC, userId);
            log.addString("'; ");
            db().execute("update UPUser set last_name = '%s', modifTm = CURRENT_TIMESTAMP where userID = %" UDEC, llastName, userId);
            result = !db().HasFailed();
        }
        if( result && groups.dim() ) {
            log.printf("old_groups='%s'; ", groupList());
            sStr g;
            sString::printfIVec(&g, &groups, ",");
            db().execute("DELETE FROM UPGroup WHERE userID = %" UDEC " AND flags != -1 AND groupPath NOT IN (SELECT CONCAT(first_name, '%s') FROM UPUser WHERE userID IN (%s))", userId, lemail, g.ptr());
            result = !db().HasFailed();
            if( result ) {
                db().execute("INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath) "
                         "SELECT %" UDEC ", 0, FALSE, CONCAT(first_name, '%s') FROM UPUser WHERE userID IN (%s) AND userID NOT IN ("
                           "SELECT userID FROM UPUser WHERE first_name IN ("
                                "SELECT SUBSTRING(groupPath, 1, LENGTH(groupPath) - LENGTH('%s')) FROM UPGroup WHERE userID = %" UDEC " AND flags != -1 AND groupPath IN ("
                                       "SELECT CONCAT(first_name, '%s') FROM UPUser WHERE userID in(%s))))"
                        , userId, lemail, g.ptr(), lemail, userId, lemail, g.ptr());
                result = !db().HasFailed();
            }
        }
    }
    if( err || !result ) {
        updateAbandon();
    } else if( !updateComplete() ) {
        result = false;
    } else if( !init(userId) ) {
        err.printf(0, "user corrupt");
    } else if( isnew ) {
        initFolders(true);
    }
    if( !result && !err ) {
        err.printf(0, "System temporarily unavailable. Try again later.");
        log.printf("dberr='%s';", db().Get_error().ptr());
    }
    log.printf("result='%s';", err.ptr());
    audit(eUserAuditLogin, isnew ? "account-register" : "account-update", "%s", log.ptr());
    return result;
}

const char* sUsr::groupList(bool inactive) const
{
    static sStr buf;
    buf.cut0cut();
    sStr protectedEmail;
    db().protect(protectedEmail, m_Email.ptr());
    return db().svalue(buf, "SELECT GROUP_CONCAT(userID) FROM UPUser WHERE `type` = 'group' AND CONCAT(first_name, '%s') IN (SELECT groupPath FROM UPGroup WHERE userID = %" UDEC " AND is_active_fg IN (%s))",
        protectedEmail.ptr(), m_Id, inactive ? "TRUE, FALSE" : "TRUE");
}

bool sUsr::hasGroup(const char * path_prefix, bool direct_member_only, bool with_inactive) const
{
    idx len = sLen(path_prefix);
    if( !len || path_prefix[len - 1] != '/' ) {
        return false;
    }
    sStr sql;
    sql.printf("SELECT COUNT(*) FROM UPGroup WHERE `userID` = %" UDEC " AND `groupPath` LIKE '", m_Id);
    db().protect(sql, path_prefix);
    sql.shrink00();
    sql.addString("%'");
    if( !with_inactive ) {
        sql.addString(" AND `is_active_fg` = TRUE");
    }
    if( direct_member_only ) {
        sql.printf(" AND SUBSTRING(`groupPath`, %" DEC ") NOT LIKE '%%/%%'", sLen(path_prefix) + 1);
    }

    return db().ivalue(sql, "%s", sql.ptr());
}


const sHiveId * sUsr::getDomainIdDescr(udx domain_id) const
{
    const char * url = sHiveId::domainIdAsUrl(domain_id);
    if( !url ) {
        sUsrObjRes res;
        objs2("^domain-id-descr$+", res, 0, 0, 0, "domain-id-descr_ascii,domain-id-descr_uri", 0, 0, 0, true);
        for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
            const sUsrObjRes::TObjProp * props = res.get(it);
            const sUsrObjRes::TPropTbl * tbl_ascii = res.get(*props, "domain-id-descr_ascii");
            const char * ascii = res.getValue(tbl_ascii);
            const sUsrObjRes::TPropTbl * tbl_uri = res.get(*props, "domain-id-descr_uri");
            const char * uri = res.getValue(tbl_uri);
            if( !sHiveId::mapDomainIdUrl(ascii, uri, *(res.id(it))) ) {
                continue;
            }
        }
    }
    return sHiveId::domainObjId(domain_id);
}

sRC sUsr::objCreate(sHiveId & out_id, const char* type_name, const udx in_domainID, const udx in_objID) const
{
    sRC rc;
    out_id.reset();
    if( type_name && type_name[0] ) {
        const sUsrType2 * tp = sUsrType2::ensure(*this, type_name);
        if( tp && !tp->isVirtual() ) {
            if( m_SuperUserMode || m_IsAdmin || !tp->isSystem() ) {
                std::unique_ptr<sSql::sqlProc> p;
                if( !Id() || (isGuest() && !tp->nameMatch("HIVE_public_account_request") && !tp->nameMatch("HIVE-Development_Project_Request") && !tp->nameMatch("email")) ) {
                    RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
                } else if( m_currProj.projectId && ( m_currProj.role == eProjectViewer || m_currProj.role == eProjectDataHandler ) ) {
                    RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
                } else if( m_currProj.projectId && tp->nameMatch("user-settings") ) {
                    RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eGroup, sRC::eIncompatible);
                } else if( in_domainID && !in_objID ) {
                    const sHiveId * domain_id_descr = getDomainIdDescr(in_domainID);
                    if( !domain_id_descr ) {
                        RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eDomain, sRC::eNotFound);
                    } else if(  m_SuperUserMode || isAllowed(*domain_id_descr, ePermCanExecute) ) {
                        p.reset(getProc("sp_obj_create_new_in_domain"));
                        p->Add(tp->id().domainId()).Add(tp->id().objId()).Add(in_domainID).Add((udx) ePermCompleteAccess).Add((udx) eFlagDefault).
                            Add(domain_id_descr->domainId()).Add(domain_id_descr->objId()).Add((udx) ePermCanExecute);
                    } else {
                        RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eDomain, sRC::eNotAuthorized);
                    }
                } else {
                    const sHiveId * domain_id_descr = getDomainIdDescr(0);
                    if( !domain_id_descr ) {
                        RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eDomain, sRC::eNotFound);
                    } else if( m_SuperUserMode || isAllowed(*domain_id_descr, ePermCanExecute) ) {
                        p.reset(getProc("sp_obj_create_v2"));
                        p->Add(tp->id().domainId()).Add(tp->id().objId()).Add(in_domainID).Add(in_objID).Add((udx) ePermCompleteAccess).Add((udx) eFlagDefault);
                    } else {
                        RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eDomain, sRC::eNotAuthorized);
                    }
                }
                sVarSet tbl;
                if( rc.isUnset() && p.get() && p->getTable(&tbl) && tbl.rows == 1 ) {
                    out_id.set(tbl.uval(0, 0), tbl.uval(0, 1), 0);
                    if( out_id ) {
                        sUsrObj * o = objFactory(out_id);
                        if( o ) {
                            sStr hack;
                            o->addFilePathname(hack, true, ".deleteme");
                            o->delFilePathname("%s", ".deleteme");
                            if( !rc.isSet() ) {
                                const sUsrType2 * type_type = sUsrType2::ensureTypeType(*this);
                                if( type_type && tp->id() == type_type->id() ) {
                                    const udx system_grp_id = getGroupId("/system/");
                                    if( !system_grp_id || !setPermission(system_grp_id, out_id, ePermCompleteAccess, eFlagInheritDown) ) {
                                        RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                                    } else if( in_domainID == type_type->id().domainId() ) {
                                        const udx everyone_grp_id = getGroupId("/everyone/");
                                        if( !everyone_grp_id || !setPermission(everyone_grp_id, out_id, ePermCanBrowse | ePermCanRead, eFlagInheritDown) ) {
                                            RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                                        }
                                    }
                                } else if( tp->nameMatch("HIVE_Development_Timelog") ) {
                                    if( m_currProj.projectId ) {
                                        RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eOperation, sRC::eProhibited);
                                    } else {
                                        const udx hive_grp_id = getGroupId("/Projects/Team/");
                                        if( !hive_grp_id || !setPermission(hive_grp_id, out_id, ePermCanBrowse | ePermCanRead, eFlagInheritDown) ) {
                                            RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                                        }
                                    }
                                } else if( tp->nameMatch("HIVE_public_account_request") || (isGuest() && tp->nameMatch("email")) ) {
                                    if( const udx hive_grp_id = getGroupId("/Projects/Team/") ) {
                                        if( !setPermission(hive_grp_id, out_id, ePermCanBrowse | ePermCanRead | ePermCanShare, eFlagInheritDown) ) {
                                            RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                                        } else {
                                            m_SuperUserMode = true;
                                            if( !setPermission(m_PrimaryGroup, out_id, ePermCanWrite, eFlagNone) ) {
                                                RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                                            }
                                            m_SuperUserMode = false;
                                        }
                                    }
                                } else if( tp->nameMatch("project+") ) {
                                    sStr errBuf;
                                    sVec<Project> projRoles;
                                    const Project oldProject = m_currProj;

                                    if( !updateStart() ) {
                                        RCSET(rc, sRC::eStarting, sRC::eSequence, sRC::eDatabase, sRC::eFailed);
                                    }
                                    if( !rc ) {
                                        sStr projPath;
                                        const char * roleChars = "ACDV";
                                        for (idx i = 0; i < 4; i++) {
                                            projPath.cut(0);
                                            projPath.printf("%" UDEC "-%c", out_id.objId(), roleChars[i]);
                                            const bool success = const_cast<sUsr *>(this)->groupCreate(projPath.ptr(0), projPath.ptr(0), "/HIVEPROJ/");
                                            if( !success || err ) {
                                                RCSET(rc, sRC::eCreating, sRC::eGroup, sRC::eDatabase, sRC::eFailed);
                                                break;
                                            }
                                        }
                                    }
                                    if( !rc ) {
                                        m_currProj.projectId = out_id.objId();
                                        m_currProj.role = eProjectAdmin;
                                        rc = addToProject(m_Id, eProjectAdmin);
                                    }
                                    if( !rc ) {
                                        rc = allProjectRoles(projRoles, out_id.objId());
                                    }
                                    if( !rc ) {
                                        rc = setProject(out_id.objId());
                                    }
                                    if( !rc ) {
                                        rc = initProjectFolders();
                                    }
                                    if( !rc) {
                                        rc = setProject(oldProject.projectId);
                                    }
                                    if( !rc ) {
                                        sDic<udx> groupPerms;
                                        for (idx i = 0; i < projRoles.dim(); i++) {
                                            udx grpId = projRoles[i].groupId;
                                            *groupPerms.set(&grpId, sizeof(udx)) = projRoleToPerm(projRoles[i].role);
                                        }
                                        const bool oldSuperUserMode = m_SuperUserMode;
                                        m_SuperUserMode = true;
                                        rc = clearAndSetPermissions(out_id, groupPerms);
                                        if( !rc ) {
                                            const udx hive_grp_id = getGroupId("/Projects/Team/");
                                            if( !hive_grp_id || !setPermission(hive_grp_id, out_id, ePermCompleteAccess, eFlagInheritDown) ) {
                                                RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                                            }
                                        }
                                        m_SuperUserMode = oldSuperUserMode;
                                    }
                                    if( !rc ) {
                                        if( !updateComplete() ) {
                                            RCSET(rc, sRC::eCommitting, sRC::eSequence, sRC::eDatabase, sRC::eFailed);
                                        }
                                    } else {
                                        if( !updateAbandon() ) {
                                            RCSET(rc, sRC::eCleaningUp, sRC::eSequence, sRC::eDatabase, sRC::eFailed);
                                        }
                                    }
                                }

                                if ( !rc && m_currProj.projectId && !tp->nameMatch("project+") ) {
                                    sStr err;
                                    sVec<Project> roles;
                                    rc = allProjectRoles(roles, m_currProj.projectId);
                                    if( !rc ) {
                                        sDic<udx> groupPerms;
                                        for (idx i = 0; i < roles.dim(); i++) {
                                            *groupPerms.set(&roles[i].groupId, sizeof(udx)) = projRoleToPerm(roles[i].role);
                                        }
                                        const bool oldSuperUserMode = m_SuperUserMode;
                                        m_SuperUserMode = true;
                                        rc = clearAndSetPermissions(out_id, groupPerms);
                                        m_SuperUserMode = oldSuperUserMode;
                                    }
                                }
                            }
                            delete o;
                        }
                    } else {
                        RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eID, sRC::eNull);
                    }
                } else if( rc.isUnset() ) {
                    RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eOperation, sRC::eFailed);
                    if( db().HasFailed() ) {
                        fprintf(stderr, "objCreate() DB error %" UDEC ": %s\n", db().Get_errno(), db().Get_error().ptr());
                        if( QPride() ) {
                            QPride()->logOut(db().in_transaction() ? sQPrideBase::eQPLogType_Warning : sQPrideBase::eQPLogType_Error,
                                                "objCreate() DB error %" UDEC ": %s", db().Get_errno(), db().Get_error().ptr());
                        }
                    }
                }
            } else {
                RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
            }
        } else {
            RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eType, sRC::eInvalid);
        }
    } else {
        RCSET(rc, sRC::eCreating, sRC::eObject, sRC::eType, sRC::eEmpty);
    }
    if( rc.isSet() ) {
        out_id.reset();
    }
    audit(eUserAuditActions, __func__, "type='%s'; result='%s'", type_name, rc.isSet() ? rc.print() : out_id.print());
    return rc;
}

bool sUsr::objGet(const sHiveId & id) const
{
    return isAllowed(id, ePermCanRead | ePermCanBrowse);
}

bool sUsr::objGet(const sHiveId & id, const sHiveId * ptypeHiveId, udx permission) const
{
    if( !permission ) {
        permission = ePermCanRead | ePermCanBrowse;
    }
    return isAllowedAndHasType(id, ptypeHiveId, permission);
}

bool sUsr::isAllowed(const sHiveId & objHiveId, udx permission) const
{
    return (objPermEffective(objHiveId) & permission) != 0;
}

bool sUsr::isAllowedAndHasType(const sHiveId & objHiveId, const sHiveId * ptypeHiveId, udx permission) const
{
    if( !isAllowed(objHiveId, permission) ) {
        return false;
    }

    sObjPerm * op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    if( op && ptypeHiveId && op->type != *ptypeHiveId ) {
        return false;
    }
    return op;
}

const sUsrType2 * sUsr::objGetType(const sHiveId & objHiveId, udx permission) const
{
    if( !isAllowed(objHiveId, permission) ) {
        return 0;
    }

    sObjPerm * op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    return op ? sUsrType2::ensure(*this, op->type) : 0;
}

void sUsr::objPermAll(sVec<sHiveId>& ids, sVarSet& tbl, bool expand_grp) const
{
    sVec<sHiveId> allowed;
    for(idx i = 0; i < ids.dim(); ++i) {
        if( isAllowed(ids[i], ePermCanBrowse) ) {
            sHiveId * id = allowed.add(1);
            if( id ) {
                *id = ids[i];
            }
        }
    }
    if( allowed.dim() ) {
        sStr sid;
        sSql::exprInList(sid, "domainID", "objID", allowed, false);
        if( sid ) {
            std::unique_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_all_v2"));
            p->Add(sid).Add(expand_grp);
            p->getTable(&tbl);
        }
    }
}

udx sUsr::objPermEffective(const sHiveId & objHiveId) const
{
    udx p = ePermNone;
    sObjPerm * op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    if( !op ) {
        sVec<sHiveId> out;
        objs(&objHiveId, 1, out);
        op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    }
    if( op && m_SuperUserMode && !m_AllowExpiredObjects && op->expiration == sObjPerm::eMaybeExpired ) {
        cacheRemove(objHiveId);
        sVec<sHiveId> out;
        objs(&objHiveId, 1, out);
        op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
        if( !op ) {
            m_AllowExpiredObjects = true;
            objs(&objHiveId, 1, out);
            m_AllowExpiredObjects = false;
            op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
            if( op ) {
                op->expiration = sObjPerm::eExpired;
            }
        }
    }
    if( op && (m_AllowExpiredObjects || op->expiration == sObjPerm::eUnexpired) ) {
            p = op->allow & ~op->deny;
    }
    return p;
}

sHiveId sUsr::propExport(sVec<sHiveId>& ids, sVarSet & v, sUsr::EPermExport permissions) const
{
    for(idx i = 0; i < ids.dim(); ++i) {
        std::unique_ptr<sUsrObj> obj(objFactory(ids[i]));
        if( !obj.get() ) {
            return ids[i];
        }
        const idx rows1 = v.rows;
        obj->propBulk(v);
        if( v.rows > rows1 ) {
            v.addRow().addCol(ids[i]).addCol("_type").addCol((const char*)0).addCol(obj->getTypeName());
        }
    }
    if( permissions != ePermExportNone ) {
        sVarSet tbl;
        sStr perm;
        objPermAll(ids, tbl, true);
        sHiveId toid;
        idx n = 0;
        for(idx r = 0; r < tbl.rows; ++r) {
            const char * grp = tbl.val(r, 6);
            if( grp && (grp[sLen(grp) - 1] == '/' || permissions == ePermExportAll) ) {
                perm.cut0cut();
                permPrettyPrint(perm, tbl.uval(r, 2), tbl.uval(r, 3));
                const char* vw = tbl.val(r, 5);
                sHiveId cc(tbl.uval(r, 0), tbl.uval(r, 1), 0);
                if( toid != cc ) {
                    n = 0;
                }
                v.addRow().addCol(cc).addCol("_perm").addCol(++n).printCol("%s,%s,%s", grp, vw ? vw : "", perm.ptr());
                toid.set(tbl.uval(r, 0), tbl.uval(r, 1), 0);
            }
        }
    }
    return sHiveId();
}

sHiveId sUsr::propExport(sVec<sHiveId>& ids, sJSONPrinter & printer, EPermExport permissions, const char * rootDir, const bool flatten, const bool upsert, const char * upsert_qry, const char * prop_filter00, const char * prop_exclude00) const
{
    sStr sid;
    sHiveId::printVec(sid, ids, ",");
    sUsrObjRes res;
    sVarSet perm_tbl;
    if( permissions != ePermExportNone ) {
        objPermAll(ids, perm_tbl, true);
    }
    printer.startObject();
    for(idx i = 0, c = 1; i < ids.dim(); ++i, ++c) {
        sUsrObj * obj = objFactory(ids[i]);
        if( !obj ) {
            return ids[i];
        }
        res.empty();
        if( !objs2("*", res, 0, "_id", obj->Id().print()) ) {
            delete obj;
            return ids[i];
        }
        sStr x("%" DEC, c);
        printer.addKey(x.ptr());
        printer.startObject();
        res.json(*this, res.first(), printer, true, flatten, upsert, upsert_qry, prop_filter00, prop_exclude00);
        if( rootDir && *rootDir ) {
            sDir xd;
            obj->files(xd, sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs));
            if( xd.dimEntries() ) {
                printer.addKey("_file");
                printer.startArray(
#if _DEBUG
    true
#else
    false
#endif
                );
                sStr fldir("%s/%" DEC "/", rootDir, c);
                sDir::removeDir(fldir);
                if( !sDir::exists(fldir) && !sDir::makeDir(fldir) ) {
                    delete obj;
                    return ids[i];
                }
                sFilePath p, dst;
                for(idx j = 0; j < xd.dimEntries(); ++j) {
                    const char * src = xd.getEntryAbsPath(j);
                    const char * nm = p.makeName(src, "%%flnm");
                    dst.makeName(fldir, "%%dir/%s", nm);
                    if( sFile::exists(dst) && !sFile::remove(dst) ) {
                        delete obj;
                        return ids[i];
                    }
                    if( sFile::symlink(src, dst) ) {
                        printer.addValue(p.printf("%" DEC "/%s", c, nm));
                    } else {
                        delete obj;
                        return ids[i];
                    }
                }
                printer.endArray();
            }
        }
        bool have_grp_perm = false;
        for(idx ir = 0; ir < perm_tbl.rows; ir++) {
            sHiveId perm_id(perm_tbl.uval(ir, 0), perm_tbl.uval(ir, 1), 0);
            if( perm_id == ids[i] ) {
                const char * grp = perm_tbl.val(ir, 6);
                if( (grp && permissions == ePermExportGroups && grp[sLen(grp) - 1] == '/') || (permissions == ePermExportAll) ) {
                    if( !have_grp_perm ) {
                        printer.addKey("_perm");
                        printer.startArray();
                        have_grp_perm = true;
                    }
                    permPretty2JSON(printer, 0, grp, 0, perm_tbl.uval(ir, 2), perm_tbl.uval(ir, 3));
                }
            }
        }
        if( have_grp_perm ) {
            printer.endArray();
        }
        printer.endObject();
        delete obj;
    }
    printer.endObject();
    return sHiveId();
}

bool sUsr::objFilesExport(sVec<sHiveId> & ids, sVarSet & v, const char * dstdir, const char * mask) const
{
    if( !dstdir || !dstdir[0] ) {
        return false;
    }
    sFilePath p, dst;
    for(idx i = 0; i < ids.dim(); i++) {
        sUsrObj * obj = objFactory(ids[i]);
        if( !obj ) {
            return false;
        }
        sDir xd;
        obj->files(xd, sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs), mask);
        idx flcnt = 0;
        sStr fldir("%s/%s/", dstdir, obj->Id().print());
        sDir::removeDir(fldir);
        if( !sDir::exists(fldir) && !sDir::makeDir(fldir) ) {
            delete obj;
            return false;
        }
        for(idx i = 0; i < xd.dimEntries(); i++) {
            const char * src = xd.getEntryAbsPath(i);
            p.cut(0);
            p.makeName(src, "%%flnm");
            dst.cut(0);
            dst.makeName(fldir, "%%dir/%s", p.ptr());
            if( sFile::exists(dst) && !sFile::remove(dst) ) {
                delete obj;
                return false;
            }
            if( sFile::symlink(src, dst) ) {
                v.addRow().addCol(obj->IdStr()).addCol("_file").addCol(++flcnt).printCol("%s/%s", obj->IdStr(), p.ptr());
            } else {
                delete obj;
                return false;
            }
        }
        delete obj;
    }
    return true;
}

sRC sUsr::objHivepack(sVec<sHiveId>& ids, const char * dstName, sPipe::callbackFuncType callback, void * callbackParam) const
{
    sRC rc;
    if( ids.dim() ) {
        sStr dst("%s.tmp/", dstName ? dstName : "");
        if( !sDir::exists(dst) && !sDir::makeDir(dst) ) {
            rc = RC(sRC::eCreating, sRC::eDirectory, sRC::eOperation, sRC::eFailed);
        } else {
            const sHiveId * domid = getDomainIdDescr(ids[0].domainId());
            if( domid ) {
                const bool domMode = sHiveId::setDomainUrlPrintMode(true);
                sJSONPrinter printer;
                sStr prop("%shivepack-%03u.json", dst.ptr(), 1);
                if( !sFile::exists(prop) || sFile::remove(prop) ) {
                    sFil ofil(prop);
                    if( ofil.ok() ) {
                        printer.init(&ofil);
                        printer.startObject();
                        sHiveId oerr = propExport(ids, printer, ePermExportGroups, dst,
        #if _DEBUG
                            false
        #else
                            true
        #endif
                                , false, 0, 0, 0);
                        if( oerr ) {
                            rc = RC(sRC::eWriting, sRC::eProperty, sRC::eObject, sRC::eNotFound);
                        } else {
                            printer.endObject();
                            printer.finish();
                            ofil.destroy();
                            sStr hpack("%s%s", strchr("/\\", dst[0]) ? "" : "../", dstName);
                            sStr tmp("cd \"%s\" && zip -rp \"%s\" *", dst.ptr(), hpack.ptr());
                            sFilePath cbpath(hpack, "%%dir");
                            const idx res = sPipe::exeFS(0, tmp, 0, callback, callbackParam, cbpath.ptr());
                            if( res != 0 ) {
                                rc = RC(sRC::eExecuting, sRC::eCommandLine, sRC::eOperation, res < 0 ? sRC::eInterrupted : sRC::eFailed);
                            }
                        }
                    } else {
                        rc = RC(sRC::eWriting, sRC::eFile, sRC::eOperation, sRC::eFailed);
                    }
                } else {
                    rc = RC(sRC::eRemoving, sRC::eFile, sRC::eOperation, sRC::eFailed);
                }
                sHiveId::setDomainUrlPrintMode(domMode);
            } else {
                rc = RC(sRC::eWriting, sRC::eObject, sRC::eDomain, sRC::eNotFound);
            }
        }
        sDir::removeDir(dst);
    } else {
        rc = RC(sRC::eCreating, sRC::eFile, sRC::eList, sRC::eEmpty);
    }
    return rc;
}

void sUsr::permPrettyScanf(const char * group, const char * view, const char* sperm, const char* sflags, const sUsrType2 * type, udx * groupId, sHiveId * viewId, udx * perm, udx * flags) const
{
    if( (group && group[0] && groupId) || (view && view[0] && type && viewId) ) {
        std::unique_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_scanf"));
        p->Add(group).Add(type ? type->name() : 0).Add(view);
        sVarSet tbl;
        p->getTable(&tbl);
        const bool sized = tbl.rows >= 1 && tbl.cols >= 2;
        if( groupId ) {
            *groupId = sized ? tbl.uval(0, 0) : 0;
        }
        if( viewId ) {
            if( sized ) {
                viewId->set(tbl.uval(0, 2), tbl.uval(0, 1), 0);
            } else {
                viewId->reset();
            }
        }
    }
    *perm = ePermNone;
    if( sperm && sperm[0] ) {
        sStr fmt("%%b=%" HEX "|browse=%x|read=%x|write=%x|exec=%x|del=%x|admin=%x|share=%x|download=%x;",
            *perm, ePermCanBrowse, ePermCanRead, ePermCanWrite, ePermCanExecute, ePermCanDelete, ePermCanAdmin, ePermCanShare, ePermCanDownload);
        sString::xscanf(sperm, fmt, perm);
    }
    *flags = eFlagNone;
    if( sflags && sflags[0] ) {
        sStr fmt("%%b=%" HEX "|allow=0|active=0|deny=%x|down=%x|up=%x|hold=%x|revoke=%x;",
            *flags, eFlagRestrictive, eFlagInheritDown, eFlagInheritUp, eFlagOnHold, eFlagRevoked);
        sString::xscanf(sflags, fmt, flags);
    }
}

bool sUsr::objSetHidden(const sHiveId & id, const bool hidden)
{
    bool ok = isAllowed(id, ePermCanAdmin);
    if( ok ) {
        std::unique_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_flip"));
        p->Add(id.domainId()).Add(id.objId()).Add(hidden).Add((idx)ePermCanBrowse);
        ok = p->uvalue(0) > 0;
        if( ok ) {
            cacheRemove(id);
        }
    }
    audit(eUserAuditAdmin, __func__, "obj='%s'; result='%s'", id.print(), ok ? "ok" : "failed");
    return ok;
}

bool sUsr::setPermission(udx groupId, const sHiveId & objHiveId, udx permission, udx flags, sHiveId * viewId, const char * forObjID) const
{
    bool ok = m_SuperUserMode, admin = false, share = false;
    if( !ok ) {
        if( getProject() ) {
            ok = false;
        } else {
            admin = isAllowed(objHiveId, ePermCanAdmin);
            share = isAllowed(objHiveId, ePermCanShare);
            ok = (admin | share);
        }
    }
    if( ok ) {
        std::unique_ptr<sUsrObj> obj(objFactory(objHiveId));
        if( obj.get() ) {
            const char * tpnm = obj->getTypeName();
            if( !m_SuperUserMode && !getProject() && groupId != m_PrimaryGroup && tpnm && (strcasecmp(tpnm, "folder") == 0 || strcasecmp(tpnm, "sysfolder") == 0) ) {
                permission &= (ePermCanBrowse | ePermCanRead | ePermCanShare);
            }
            if( admin || m_SuperUserMode ) {
                if( groupId == m_PrimaryGroup && !m_SuperUserMode ) {
                    permission |= ePermCanBrowse | ePermCanAdmin;
                    flags = eFlagDefault;
                }
            } else if( share ) {
                sObjPerm * op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
                ok = op;
                if( op ) {
                    if( flags & eFlagRestrictive ) {
                        permission |= op->deny;
                    } else {
                        permission &= op->allow;
                    }
                }
            }
            if( ok && !m_SuperUserMode ) {
                sVarSet res;
                db().getTable(&res, "SELECT groupID FROM UPGroup WHERE groupPath like '/HIVEPROJ/%%' AND groupID=%" UDEC, groupId);
                ok = res.rows < 1;
            }
            if( ok ) {
                std::unique_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_set_v2"));
                p->Add(groupId);
                p->Add(objHiveId.domainId()).Add(objHiveId.objId()).Add(viewId ? viewId->domainId() : (udx)0).Add(viewId ? viewId->objId() : (udx)0);
                p->Add(permission).Add(flags).Add(true);
                ok = p->uvalue(0) > 0;
                if( ok ) {
                    cacheRemove(objHiveId);
                }
            }
        } else {
            ok = false;
        }
    }
    sStr perm_log;
    permPrettyPrint(perm_log, permission, flags);
    if( sLen(forObjID) ) {
        audit(eUserAuditAdmin, __func__, "groupID='%" UDEC "'; objID='%s'; forObjID='%s'; view=%s; perm='%s'; result='%s'", groupId, objHiveId.print(), forObjID, viewId ? viewId->print() : "", perm_log.ptr(), ok ? "ok" : "failed");
    } else {
        audit(eUserAuditAdmin, __func__, "groupID='%" UDEC "'; objID='%s'; view=%s; perm='%s'; result='%s'", groupId, objHiveId.print(), viewId ? viewId->print() : "", perm_log.ptr(), ok ? "ok" : "failed");
    }
    return ok;
}

bool sUsr::copyPermission(const sHiveId & objHiveIdFrom, const sHiveId & objHiveIdTo) const
{
    bool ok = isAllowed(objHiveIdFrom, ePermCanAdmin) &&
              isAllowed(objHiveIdTo, ePermCanAdmin);
    if( ok ) {
        std::unique_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_copy_v2"));
        p->Add(objHiveIdFrom.domainId()).Add(objHiveIdFrom.objId()).Add(objHiveIdTo.domainId()).Add(objHiveIdTo.objId());
        ok = p->uvalue(0) > 0;
        if( ok ) {
            cacheRemove(objHiveIdTo);
        }
    }
    audit(eUserAuditAdmin, __func__, "objFrom='%s'; objTo='%s'; result='%s'", objHiveIdFrom.print(), objHiveIdTo.print(), ok ? "ok" : "failed");
    return ok;
}

bool sUsr::allow4admins(const sHiveId & objHiveId) const
{
    bool ok = false;
    if( isAllowed(objHiveId, ePermCanAdmin) ) {
        udx adm = db().uvalue(0, "select groupID from UPGroup where groupPath = '/system/admins/'");
        if( adm ) {
            ok = setPermission(adm, objHiveId, ePermCompleteAccess, eFlagInheritDown);
        }
    }
    return ok;
}

bool sUsr::allowRead4users(const sHiveId & objHiveId) const
{
    bool ok = false;
    if( isAllowed(objHiveId, ePermCanAdmin) ) {
        udx adm = db().uvalue(0, "select groupID from UPGroup where groupPath = '/everyone/users/'");
        if( adm ) {
            ok = setPermission(adm, objHiveId, ePermCanBrowse|ePermCanRead, eFlagInheritDown);
        }
    }
    return ok;
}

class TPropCtx
{
    public:
        TPropCtx(const sUsr & user, sStr & log)
            : user(user), log(log)
        {
        }
        bool hasError(idx pos = 0) const
        {
            return log && sString::searchSubstring(log.ptr(pos), 0, "\nerr." __, sNotIdx, 0, false) != 0;
        }
        std::unique_ptr<sUsrQueryEngine> qengine;
        const sUsr & user;
        sStr & log;
};


#define RM_TRAIL_SPACE(v,l) {{ while(strchr("\n\r\t ", v[--l]) != 0 ) {} ++l; }}

static const char * printRelativeFilePath(sStr & outbuf, const char * path, idx len, sFilePath & tmp)
{
    if( !path || !len ) {
        return 0;
    }

    tmp.addString(path, len);
    tmp.simplifyPath();
    tmp.shrink00();

    if( !tmp.length() || !tmp[0] ) {
        return 0;
    }
    if( tmp[0] == '/' ) {
        return 0;
    }
    for(idx i = 0; i + 1 < len; i++) {
        if( path[i] == '.' && path[i + 1] == '.' && (i == 0 || path[i - 1] == '/') && (i + 2 == len || path[i + 2] == '/') ) {
            return 0;
        }
    }
    idx outbuf_start = outbuf.length();
    outbuf.addString("./", 2);
    outbuf.addString(tmp.ptr(), tmp.length());
    if( outbuf[outbuf.length() - 1] == '/' ) {
        outbuf.cut0cut(outbuf.length() - 1);
    }
    if( !sFile::exists(outbuf.ptr(outbuf_start)) ) {
        outbuf.cut0cut(outbuf_start);
        return 0;
    }
    return outbuf.ptr(outbuf_start);
}

class TPropObj
{
    public:
        enum EPrefix {
            ePrefix_none = -1,
            ePrefixProp = 0,
            ePrefixInfo,
            ePrefixErr,
            ePrefixWarn,
            ePrefixDbg
        };
        struct PrefixStr {
            EPrefix prefix;
            const char * str;
            idx len;
        };
        static const PrefixStr prefixes[];
        static const char * scanPrefix(const char * buf, idx buf_len, idx scan_len = sIdxMax, bool no_newline = false, TPropObj::EPrefix * out_prefix = 0, idx * out_prefix_len = 0);

        struct TVecPair
        {
                TVecPair()
                {
                    ptr.mex()->flags |= sMex::fSetZero | sMex::fExactSize;
                    len.mex()->flags |= sMex::fSetZero | sMex::fExactSize;
                }
                const char * add(const char * val, const udx val_len)
                {
                    const char ** v = ptr.add();
                    udx * l = len.add();
                    if( v && l ) {
                        *v = val;
                        *l = val_len ? val_len : sLen(val);
                    }
                    return (v && l) ? 0 : "insufficient resources (1)";
                }
                sVec<const char*> ptr;
                sVec<udx> len;
        };
        struct TPropVals
        {
                TPropVals()
                    : append(false),
                      prefix(sMex::fSetZero | sMex::fExactSize)
                {
                }
                bool append;
                TVecPair path;
                TVecPair value;
                sVec<EPrefix> prefix;
                const char * addPrefix(EPrefix pref)
                {
                    EPrefix * ppref = prefix.add(1);
                    if( ppref ) {
                        *ppref = pref;
                    }
                    return ppref ? 0 : "insufficient resources (1.1)";
                }
        };

        sDic<TPropVals> prop;
        sHiveId id;
        udx in_domain_id;
        sStr del;
        sStr file;
        sFilePath tmp_file_buf;
        sVec<sMex::Pos> perms;
        typedef struct {
            udx groupId, perm, flags;
            sHiveId viewId;
        } TPerm;
        sVec<TPerm> perm;

        TPropObj()
            : del(sMex::fExactSize), file(sMex::fExactSize)
        {
            prop.mex()->flags |= sMex::fSetZero | sMex::fExactSize;
            in_domain_id = 0;
        }
        bool isEmpty() const
        {
            return prop.dim() == 0 && !file && perm.dim() == 0;
        }
        const char * setType(const char * val, udx val_len, const sUsr & user)
        {
            type_id.reset();
            RM_TRAIL_SPACE(val, val_len);
            if( val && val_len ) {
                char buf[256];
                if( val_len < sizeof(buf) ) {
                    bool no_prefetch_types = false;
                    if( val_len == 4 && strncasecmp(val, "type", 4) == 0 ) {
                        no_prefetch_types = true;
                    }
                    const sUsrType2 * utype = sUsrType2::ensure(user, val, val_len, no_prefetch_types);
                    if( utype ) {
                        type_id = utype->id();
                        return 0;
                    } else {
                        return "type name not found";
                    }
                } else {
                    return "type name too long";
                }
            }
            return "empty type name";
        }
        const char * setInDomainId(const char * val, udx val_len)
        {
            in_domain_id = 0;
            RM_TRAIL_SPACE(val, val_len);
            if( val && val_len ) {
                in_domain_id = sHiveId::encodeDomainId(val, val_len);
                if( !in_domain_id ) {
                    return "invalid domain";
                }
            }
            return 0;
        }
        const sUsrType2 * getType() const
        {
            return sUsrType2::get(type_id);
        }
        bool val2id(sHiveId & out_id, const char * val, udx val_len, const char *& err, bool test, bool allow_zero_id = false) const
        {
            out_id.reset();
            RM_TRAIL_SPACE(val, val_len);
            if( val && val_len ) {
                idx parse_len = 0;
                if ( val_len > S_HIVE_ID_MAX_BUFLEN ) {
                    err = test ? 0 : "object id is too long";
                } else {
                    parse_len = out_id.parse(val, val_len);
                    if( !parse_len || (udx)parse_len != val_len ) {
                        err = test ? 0 : "object id is invalid or malformed";
                    } else if ( !out_id && !allow_zero_id ) {
                        err = test ? 0 : "object id is zero";
                    }
                }
            }
            return out_id;
        }
        const char * addProp(EPrefix prefix, const char * pnm, udx pnm_len, const char * path, udx path_len, const char * val, udx val_len, TPropCtx & ctx, bool not_for_db)
        {
            const char * err = 0;
            if( pnm[0] == '_' ) {
                if( strncasecmp(pnm, "_id", pnm_len) == 0 ) {
                    sHiveId _id;
                    if( val_len >= 8 && strncasecmp(val, "query://", 8) == 0 ) {
                        if( !ctx.qengine.get() ) {
                            ctx.qengine.reset(new sUsrQueryEngine(const_cast<sUsr&>(ctx.user)));
                            if( !ctx.qengine.get() ) {
                                err = "insufficient resources (2)";
                            }
                        }
                        if( !err ) {
                            static sStr lerr;
                            lerr.cut(0);
                            ctx.qengine->parse(&val[8], val_len - 8, &lerr);
                            if( lerr ) {
                                err = lerr.ptr();
                            } else {
                                sVariant * v = ctx.qengine->run(&lerr);
                                if( lerr ) {
                                    err = lerr.ptr();
                                } else {
                                    if( v && v->isList() && v->dim() == 1 ) {
                                        v->getListElt(0)->asHiveId(&_id);
                                    } else {
                                        _id.reset();
                                    }
                                    if( !_id ) {
                                        err = "query must return single object id != 0 as result";
                                    }
                                }
                            }
                        }
                    } else if( val && val_len ) {
                        val2id(_id, val, val_len, err, false, not_for_db);
                    }
                    if( !err ) {
                        if( id && id != _id ) {
                            err = "_id value is different from the id following 'prop.'";
                        } else {
                            id = _id;
                        }
                    }
                } else if( strncasecmp(pnm, "_domain", pnm_len) == 0 ) {
                    err = setInDomainId(val, val_len);
                } else if( strncasecmp(pnm, "_type", pnm_len) == 0 ) {
                    err = setType(val, val_len, ctx.user);
                } else if( strncasecmp(pnm, "_delete", pnm_len) == 0 ) {
                    if( del ) {
                        del.shrink00();
                        del.add0();
                    }
                    del.printf("%.*s", (int)val_len, val);
                    del.add0(2);
                } else if( strncasecmp(pnm, "_file", pnm_len) == 0 ) {
                    RM_TRAIL_SPACE(val, val_len);
                    tmp_file_buf.cut0cut();
                    if( file ) {
                        file.shrink00();
                        file.add0();
                    }
                    if( printRelativeFilePath(file, val, val_len, tmp_file_buf) ) {
                        file.add0(2);
                    } else {
                        err = not_for_db ? 0 : "invalid filename or path";
                    }
                } else if( strncasecmp(pnm, "_comment", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_dir", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_brief", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_summary", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_info", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_action", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_perm", pnm_len) == 0 ) {
                    sMex::Pos * pos = perms.add();
                    if( pos ) {
                        pos->pos = (idx)val;
                        pos->size = val_len;
                    } else {
                        err = "insufficient resources (6)";
                    }
                } else if( strncasecmp(pnm, "_effperm", pnm_len) == 0) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_info", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_warn", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "_err", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : "file contains _err";
                } else {
                    err = "unrecognized '_' (underscore) directive";
                }
            } else {
                err = addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx);
            }
            return err;
        }
    private:
        const char * addPropLowLevel(EPrefix prefix, const char * pnm, udx pnm_len, const char * path, udx path_len, const char * val, udx val_len, TPropCtx & ctx)
        {
            const char * err = 0;
            char buf[256];
            bool append = pnm[pnm_len - 1] == '+';
            if( append ) {
                pnm_len--;
            }
            if( pnm_len < sizeof(buf) ) {
                snprintf(buf, sizeof(buf), "%.*s", (int) pnm_len, pnm);
                TPropVals * pp = prop.get(buf);
                if( !pp ) {
                    pp = prop.set(buf);
                    if( pp ) {
                        pp->append = append;
                    } else {
                        err = "insufficient resources (3)";
                    }
                } else if( pp->append != append ) {
                    err = "mixed append and overwrite";
                }
                if( !err ) {
                    if( val_len > 16 * 1024 * 1024 ) {
                        err = "value too long";
                    } else if( path_len > 255 ) {
                        err = "path too long";
                    } else {
                        err = pp->path.add(path, path_len);
                        err = err ? err : pp->value.add(val, val_len);
                        err = err ? err : pp->addPrefix(prefix);
                    }
                }
            } else {
                err = "property name too long";
            }
            return err;
        }
        sHiveId type_id;
};

const TPropObj::PrefixStr TPropObj::prefixes[] = {
    { ePrefixProp, "prop.", 5 },
    { ePrefixInfo, "info.", 5 },
    { ePrefixErr, "err.", 4 },
    { ePrefixWarn, "warn.", 5 },
    { ePrefixDbg, "dbg.", 4 },
};

const char * TPropObj::scanPrefix(const char * buf, idx buf_len, idx scan_len, bool no_newline, TPropObj::EPrefix * out_prefix, idx * out_prefix_len)
{
    if( buf ) {
        for(idx i = 0, newline_len = 0; i < buf_len && i < scan_len; i += newline_len? newline_len : 1 ) {
            newline_len = 0;
            if( !no_newline ) {
                if( buf[i] == '\r' ) {
                    newline_len++;
                }
                if( i + newline_len < buf_len && i + newline_len < scan_len && buf[i + newline_len] == '\n' ) {
                    newline_len++;
                }
            }
            if( newline_len || no_newline ) {
                for(idx ip = 0; ip < sDim(prefixes); ip++) {
                    if( i + newline_len + prefixes[ip].len <= buf_len && strncasecmp(buf + i + newline_len, prefixes[ip].str, prefixes[ip].len) == 0 ) {
                        if( out_prefix ) {
                            *out_prefix = prefixes[ip].prefix;
                        }
                        if( out_prefix_len ) {
                            *out_prefix_len = prefixes[ip].len + newline_len;
                        }
                        return buf + i;
                    }
                }
            }
        }
    }
    if( out_prefix_len ) {
        *out_prefix_len = 0;
    }
    if( out_prefix ) {
        *out_prefix = TPropObj::ePrefix_none;
    }
    return 0;
}

class TProp
{
    public:
        TProp(const sUsr & user, sStr & log)
            : ctx(user, log)
        {
        }
        bool parse(const char * nm, udx nm_len, const char * val, const udx val_len, bool not_for_db = false)
        {
            nm_len = nm_len ? nm_len : sLen(nm);
            TPropObj::EPrefix prefix = TPropObj::ePrefixProp;
            idx prefix_len = 0;
            if( !TPropObj::scanPrefix(nm, nm_len, 1, true, &prefix, &prefix_len) ) {
                return true;
            }
            const char * err = 0, *nm_end = nm + nm_len;
            do {
                const char * id = &nm[prefix_len];
                const char * id_end = id;
                while(true) {
                    id_end = (const char *) memchr(id_end, '.', nm_end - id);
                    if(id_end < nm_end && !isdigit(id_end[1])) {
                        break;
                    }
                    ++id_end;
                }
                TPropObj * pobj = 0;
                if( id_end ) {
                    pobj = all.get(id, id_end - id);
                    if( !pobj ) {
                        sStr sid("%.*s", (int)(id_end - id), id);
                        pobj = all.set(sid.ptr());
                        if( pobj ) {
                            pobj->val2id(pobj->id, sid, id_end - id, err, true, not_for_db);
                        } else {
                            err = "insufficient resources (4)";
                        }
                    }
                } else {
                    err = id < nm_end ? "expected property name" : "expected object id";
                }
                if( !err ) {
                    const char * pnm = id_end + 1, *pnm_end = 0;
                    if( pnm >= nm_end ) {
                        err = "expected property name";
                    } else {
                        pnm_end = (const char *) memchr(pnm, '.', nm_end - pnm);
                        pnm_end = pnm_end ? pnm_end : nm_end;
                        const char * path = pnm_end + 1;
                        const char * path_end = (path < nm_end) ? nm_end : 0;
                        err = pobj->addProp(prefix, pnm, pnm_end - pnm, path_end ? path : 0, path_end ? path_end - path : 0, val, val_len, ctx, not_for_db);
                    }
                }
            } while( false );
            if( err ) {
                ctx.log.printf("\nerr.%.*s=%s", (int) (nm_len - prefix_len), &nm[prefix_len], err);
            }
            return !err;
        }
        bool validate(void)
        {
            const idx logpos = ctx.log.pos();
            for(idx o = 0; o < all.dim(); ++o) {
                const char * oid = (const char *) all.id(o);
                TPropObj * pobj = all.ptr(o);
                if( !pobj->prop.dim() ) {
                    continue;
                }
                if( pobj->id ) {
                    std::unique_ptr<sUsrObj> obj(ctx.user.objFactory(pobj->id));
                    if( !obj.get() ) {
                        ctx.log.printf("\nerr.%s._id=object not found ", oid);
                        pobj->id.print(ctx.log);
                        continue;
                    } else if( pobj->getType() ) {
                        const char * file = pobj->getType()->name();
                        const char * real = obj->getTypeName();
                        if( strcasecmp(file, real) != 0 ) {
                            ctx.log.printf("\nerr.%s._type=object type mismatch '%s' != '%s'", oid, file, real);
                            continue;
                        }
                    } else {
                        const char * real = obj->getTypeName();
                        pobj->setType(real, sLen(real), ctx.user);
                    }
                } else if( !pobj->getType() ) {
                    ctx.log.printf("\nerr.%s._type=missing object type", oid);
                    continue;
                }
                if( pobj->del ) {
                    for(const char * p = pobj->del; p; p = sString::next00(p)) {
                        if( pobj->getType()->getFieldType(ctx.user, p) == sUsrTypeField::eInvalid ) {
                            ctx.log.printf("\nerr.%s._delete=invalid property name '%s'", oid, p);
                        }
                    }
                }
                if( pobj->file ) {
                }
                if( pobj->perms.dim() ) {
                    sStr tmp;
                    for(idx i = 0; i < pobj->perms.dim(); ++i) {
                        const sMex::Pos p = pobj->perms[i];
                        tmp.printf(0, "%.*s", (int)p.size, (const char *)(p.pos));
                        char * grp = tmp ? tmp.ptr() : 0;
                        char * view = 0, * sperm = 0, * sflags = 0;
                        if( grp && (view = strchr(grp, ',')) != 0 ) {
                            view[0] = '\0';
                            if( (++view)[0] != '\0' && (sperm = strchr(view, ',')) != 0 ) {
                                sperm[0] = '\0';
                                if( (++sperm)[0] != '\0' && (sflags = strchr(sperm, ',')) != 0 ) {
                                    sflags[0] = '\0';
                                    ++sflags;
                                }
                            }
                        }
                        if( !grp ) {
                            ctx.log.printf("\nerr.%s._perm=invalid format: '%.*s'", oid, (int)p.size, (const char *)(p.pos));
                        } else {
                            TPropObj::TPerm ip;
                            sSet(&ip);
                            ctx.user.permPrettyScanf(grp, view, sperm, sflags, pobj->getType(), &ip.groupId, &ip.viewId, &ip.perm, &ip.flags);
                            if( ip.groupId ) {
                                TPropObj::TPerm * pp = pobj->perm.add();
                                if( pp ) {
                                    *pp = ip;
                                } else {
                                    ctx.log.printf("\nerr.%s._perm=insufficient resources (7)", oid);
                                }
                            } else {
                                ctx.log.printf("\nwarn.%s._perm=group not found: '%s'", oid, grp);
                            }
                        }
                    }
                    pobj->perms.empty();
                }
                for(idx p = 0; p < pobj->prop.dim(); ++p) {
                    const char * pnm = (const char *) pobj->prop.id(p);
                    if( pobj->getType()->getFieldType(ctx.user, pnm) == sUsrTypeField::eInvalid ) {
                        ctx.log.printf("\nerr.%s.%s=invalid property name", oid, pnm);
                    }
                }
            }
            return !ctx.hasError(logpos);
        }
        void purgeNewObjs(sDic<sHiveId> * new_ids_map)
        {
            for(idx o = 0; o < new_ids_map->dim(); ++o) {
                sUsrObj * obj = ctx.user.objFactory(*(new_ids_map->ptr(o)));
                if( obj ) {
                    obj->purge();
                    delete obj;
                }
            }
        }
        bool db(sVec<sHiveId> * new_ids, sVec<sHiveId> * updated_ids, sDic<sHiveId> * new_ids_map = 0)
        {
            sDic<sHiveId> local_map;
            bool isok = false;
            idx ctx_log_start = ctx.log.length();
            for(idx itry = 0; itry < sSql::max_deadlock_retries; itry++) {
                bool is_our_transaction = !ctx.user.getUpdateLevel();
                if( ctx.user.updateStart() ) {
                    new_ids_map = new_ids_map ? new_ids_map : &local_map;
                    for(idx o = 0; o < all.dim(); ++o) {
                        TPropObj * pobj = all.ptr(o);
                        if( pobj->isEmpty() ) {
                            continue;
                        }
                        const char * oid = (const char *)all.id(o);
                        if( !pobj->id ) {
                            if( const sUsrType2 * otype = all.ptr(o)->getType() ) {
                                sRC rc = ctx.user.objCreate(pobj->id, otype->name(), pobj->in_domain_id);
                                if( !rc ) {
                                    sHiveId * u = new_ids_map->set(oid);
                                    if( u ) {
                                        *u = pobj->id;
                                    }
                                    if( new_ids ) {
                                        u = new_ids->add(1);
                                        if( u ) {
                                            *u = pobj->id;
                                        }
                                    }
                                } else {
                                    ctx.log.printf("\nerr.%s._err=%s", oid, rc.print());
                                    break;
                                }
                            } else {
                                ctx.log.printf("\nerr.%s._err=object type not specified or not found", oid);
                            }
                        } else if( updated_ids ) {
                            sHiveId * u = updated_ids->add(1);
                            if( u ) {
                                *u = pobj->id;
                            }
                        }
                    }
                    sStr valueSubstituteBuffer;
                    sStr objFiles00;
                    for(idx o = 0; !ctx.hasError() && o < all.dim(); ++o) {
                        TPropObj * pobj = all.ptr(o);
                        if( !pobj->id ) {
                            continue;
                        }
                        const sUsrType2 * tp = pobj->getType();
                        sUsrObj * obj = ctx.user.objFactory(pobj->id, tp ? &tp->id() : 0, ePermCanWrite);
                        if( obj ) {
                            if( pobj->del ) {
                                for(const char * p = pobj->del; p; p = sString::next00(p)) {
                                    obj->propDel(p, 0, 0);
                                }
                            }
                            for(idx p = 0; p < pobj->prop.dim(); ++p) {
                                valueSubstituteBuffer.cut(0);
                                TPropObj::TPropVals * pp = pobj->prop.ptr(p);
                                for(idx ipp = 0; ipp < pp->value.ptr.dim(); ++ipp) {
                                    const char * value = pp->value.ptr[ipp];
                                    if( strncmp(value, "${", 2) == 0 ) {
                                        idx valueLen = pp->value.len[ipp];
                                        if( value[valueLen - 1] == '}' ) {
                                            sHiveId * newid = new_ids_map->get(value + 2, valueLen - 3);
                                            if( newid ) {
                                                const idx stlen = valueSubstituteBuffer.length();
                                                pp->value.ptr[ipp] = newid->print(valueSubstituteBuffer);
                                                pp->value.len[ipp] = valueSubstituteBuffer.length() - stlen;
                                            }
                                        }
                                    }
                                }
                                const char * pnm = (const char *) pobj->prop.id(p);
                                if( obj->propSet(pnm, &(pp->path.ptr[0]), &(pp->value.ptr[0]), pp->value.ptr.dim(), pp->append, &(pp->path.len[0]), &(pp->value.len[0])) - pp->value.ptr.dim() != 0 ) {
                                    for(idx ipv = 0; ipv < pp->value.ptr.dim(); ++ipv) {
                                        ctx.log.printf("\nerr.%s.%s", (const char *)(all.id(o)), pnm);
                                        if( pp->path.len[ipv] ) {
                                            ctx.log.printf(".%.*s", (int)(pp->path.len[ipv]), pp->path.ptr[ipv]);
                                        }
                                        udx len = pp->value.len[ipv];
                                        len = len > 20 ? 20 : len;
                                        ctx.log.printf("=failed to save value: '%.*s%s'", (int)len, pp->value.ptr[ipv], len < pp->value.len[ipv] ? "..." : "");
                                    }
                                }
                            }
                            if( pobj->file ) {
                                for(const char * p = pobj->file; p; p = sString::next00(p)) {
                                    const char * nm = sFilePath::nextToSlash(p);
                                    if( obj->addFilePathname(objFiles00, false, "%s", nm) ) {
                                        if( !ctx.log ) {
                                            objFiles00.add0();
                                            objFiles00.printf("%s", p);
                                            objFiles00.add0();
                                            objFiles00.printf("%s", (const char *)(all.id(o)));
                                            objFiles00.add0();
                                        }
                                    } else {
                                        ctx.log.printf("\nerr.%s._file=cannot add file '%s'", (const char *)(all.id(o)), nm);
                                    }
                                }
                            }
                            if( pobj->perm.dim() ) {
                                for(idx i = 0; i < pobj->perm.dim(); ++i) {
                                    TPropObj::TPerm pp = pobj->perm[i];
                                    if( !ctx.user.setPermission(pp.groupId, obj->Id(), pp.perm, pp.flags, &pp.viewId) ) {
                                        ctx.log.printf("\nerr.%s._file=cannot set permissions for group %" UDEC, (const char *)(all.id(o)), pp.groupId);
                                    }
                                }
                            }
                            delete obj;
                        } else {
                            ctx.log.printf("\nerr.%s._id=object not found or access denied", pobj->id.print());
                        }
                    }
                    if( !ctx.hasError() ) {
                        if( !ctx.user.updateComplete() ) {
                            ctx.log.printf("\nerr.0._err=system is busy transaction cannot be finished");
                        } else {
                            if( objFiles00 ) {
                                objFiles00.add0(2);
                                for(const char * dst = objFiles00, * src, * oid; dst; dst = sString::next00(oid)) {
                                    src = sString::next00(dst);
                                    oid = sString::next00(src);
                                    const char * nm = sFilePath::nextToSlash(src);
                                    if( sDir::exists(src) ) {
                                        if( !sDir::copyDir(src, dst, true) ) {
                                            ctx.log.printf("\nerr.%s._file=cannot copy directory '%s'", oid, nm);
                                        }
                                    } else if( !sFile::copy(src, dst, false, true) ) {
                                        ctx.log.printf("\nerr.%s._file=cannot copy file '%s'", oid, nm);
                                    }
                                }
                            }
                            if( !ctx.hasError() ) {
                                for(idx i = 0; i < new_ids_map->dim(); ++i) {
                                    idx sz = 0;
                                    const void * p = new_ids_map->id(i, &sz);
                                    ctx.log.printf("\nprop.%.*s._id=%s", (int) sz, (const char *)p, new_ids_map->ptr(i)->print());
                                }
                                isok = true;
                            }
                        }
                    } else {
                        if( ctx.user.hadDeadlocked() && is_our_transaction ) {
                            ctx.user.updateAbandon();
                            isok = false;
                            for(idx o = 0; o < all.dim(); ++o) {
                                TPropObj * pobj = all.ptr(o);
                                if( pobj->isEmpty() ) {
                                    continue;
                                }
                                const char * oid = (const char *)all.id(o);
                                sHiveId * pnew_id = new_ids_map->get(oid);
                                if( pnew_id && *pnew_id == pobj->id ) {
                                    pobj->id.reset();
                                }
                            }
                            purgeNewObjs(new_ids_map);
                            new_ids_map->empty();
                            if( updated_ids ) {
                                updated_ids->empty();
                            }
                            ctx.log.cut0cut(ctx_log_start);
                            sTime::randomSleep(sSql::max_deadlock_wait_usec);
                            continue;
                        } else {
                            ctx.user.updateAbandon();
                        }
                    }
                } else {
                    ctx.log.printf("\nerr.0._err=system is busy transaction cannot be started");
                }
                break;
            }
            if( !isok ) {
                purgeNewObjs(new_ids_map);
            }
            return isok;
        }
        bool varset(sVarSet & tbl)
        {
            idx q = 0;
            for(idx o = 0; o < all.dim(); ++o) {
                const char * oid = (const char *) all.id(o);
                TPropObj * pobj = all.ptr(o);
                if( const sUsrType2 * otype = pobj->getType() ) {
                    ++q;
                    tbl.addRow().addCol(oid).addCol("_type").addCol((char *)0).addCol(otype->name());
                }
                for(idx p = 0; p < pobj->prop.dim(); ++p) {
                    const char * pnm = (const char *) pobj->prop.id(p);
                    TPropObj::TPropVals * pp = pobj->prop.ptr(p);
                    for(idx ipv = 0; ipv < pp->value.ptr.dim(); ++ipv) {
                        ++q;
                        tbl.addRow().addCol(oid).addCol(pnm).addCol(pp->path.ptr[ipv], pp->path.len[ipv]).addCol(pp->value.ptr[ipv], pp->value.len[ipv]);
                    }
                }
            }
            return (tbl.rows * tbl.cols) == (q * 4);
        }
        bool hasError() const
        {
            return ctx.hasError();
        }
private:
        sDic<TPropObj> all;
        TPropCtx ctx;
};

bool sUsr::propSet(const char * propFileName, sStr& log, sVec<sHiveId>* new_ids, sVec<sHiveId>* updated_ids, sDic<sHiveId> * new_ids_map)
{
    bool ret = false;
    sFilePath curr, ext(propFileName, "%%ext"), dir(propFileName, "%%dir");
    if( strcasecmp(ext, "prop") == 0 ) {
        sFil propFile(propFileName, sMex::fReadonly);
        if( !propFile.ok() ) {
            log.printf("err.null._err=prop file not found '%s'", propFileName);
        } else {
            if( dir ) {
                curr.curDir();
                sDir::chDir(dir);
            }
            ret = propFile.length() ? propSet(propFile.ptr(), propFile.length(), log, new_ids, updated_ids, new_ids_map) : true;
            if( dir && curr ) {
                sDir::chDir(curr);
            }
        }
    } else if( strcasecmp(ext, "json") == 0 ) {
        sDic<sUsrPropSet::Obj> modified_objs;
        sUsrPropSet upropset(*this);
        upropset.setSrcFile(propFileName);
        upropset.setFileRoot(dir.ptr());
        sJSONPrinter printer(&log);
        printer.startObject();
        if( !(ret = upropset.run(&modified_objs, sUsrPropSet::fInvalidUserGroupNonFatal | sUsrPropSet::fOverwriteExistingSameType)) ) {
            printer.addKeyValue("error", upropset.getErr());
        }
        const char * p = upropset.getWarn00();
        if( p ) {
            printer.addKey("warning");
            printer.startArray();
            for(; p; p = sString::next00(p) ) {
                if( *p ) {
                    printer.addValue(p);
                }
            }
        }
        for(idx imod = 0; imod < modified_objs.dim(); imod++) {
            sHiveId * id = (modified_objs.ptr(imod)->is_new ? new_ids : updated_ids)->add();
            if( id ) {
                *id = modified_objs.ptr(imod)->id;
            }
        }
    } else {
        ret = false;
    }
    return ret;
}

static bool propBufParse(const char * srcbuf, idx len, sStr & log, TProp & prop, bool not_for_db)
{
    if( !srcbuf ) {
        log.printf("err.null._err=missing input data");
    }
    if( !len ) {
        len = sLen(srcbuf);
    }
    const char * lastpos = srcbuf + len;
    for(const char * nm = srcbuf; nm < lastpos;) {
        nm = sString::skipWords(nm, 0, 0, sString_symbolsEndline);
        const char * nm_end = (const char *)memchr(nm, '=', lastpos - nm);
        if( !nm_end ) {
            log.printf("err.null._err='=' symbol expected");
            break;
        }
        const char * val = nm_end + 1;
        const char * val_end = TPropObj::scanPrefix(val, lastpos - val);
        if( !val_end ) {
            val_end = lastpos;
        }
        if( !prop.parse(nm, nm_end - nm, val, val_end - val, not_for_db) ) {
            break;
        }
        nm = val_end + 1;
    }
    return !prop.hasError();
}

bool sUsr::propSet(const char * srcbuf, idx len, sStr & log, sVec<sHiveId> * new_ids, sVec<sHiveId> * updated_ids, sDic<sHiveId> * new_ids_map)
{
    TProp prop(*this, log);
    if( propBufParse(srcbuf, len, log, prop, false) && prop.validate() ) {
        return prop.db(new_ids, updated_ids, new_ids_map);
    }
    return false;
}

static bool propFormParse(const sVar & form, TProp & prop, bool not_for_db)
{
    bool isok = true;
    for(idx k = 0; k < form.dim(); ++k) {
        const char * nm = (const char*) form.id(k);
        const char * val = (const char*) form.value(nm);
        isok &= prop.parse(nm, sLen(nm), val, sLen(val), not_for_db);
    }
    return isok;
}

bool sUsr::propSet(const sVar & form, sStr & log, sVarSet & result, bool not_for_db) const
{
    TProp prop(*this, log);
    if( propFormParse(form, prop, not_for_db) ) {
        return prop.varset(result);
    }
    return false;
}

bool sUsr::propSet(const char * srcbuf, idx len, sStr & log, sVarSet & result, bool not_for_db) const
{
    TProp prop(*this, log);
    if( propBufParse(srcbuf, len, log, prop, not_for_db) ) {
        return prop.varset(result);
    }
    return false;
}

bool sUsr::propSet(const sVar & form, sStr & log, sVec<sHiveId>* new_ids, sVec<sHiveId>* updated_ids, sDic<sHiveId> * new_ids_map) const
{
    TProp prop(*this, log);
    if( propFormParse(form, prop, false)  && prop.validate() ) {
        return prop.db(new_ids, updated_ids, new_ids_map);
    }
    return false;
}

const sUsrType2 * sUsr::objType(const sHiveId & objHiveId, sHiveId * out_objTypeId) const
{
    sObjPerm * op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    if( !op ) {
        sVec<sHiveId> out;
        objs(&objHiveId, 1, out);
        op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    }
    if( out_objTypeId ) {
        if( op ) {
            *out_objTypeId = op->type;
        } else {
            out_objTypeId->reset();
        }
    }
    return op ? sUsrType2::ensure(*this, op->type, false, false, true) : 0;
}

void sUsr::cacheRemove(const sHiveId & objHiveId) const
{
    if( m_ObjPermission.get() ) {
        TPermCache old(m_ObjPermission.release());
        m_ObjPermission.reset(new TPermCache::element_type);
        if( m_ObjPermission.get() ) {
            for(idx k = 0; k < old->dim(); ++k) {
                sHiveId keyId(*static_cast<const sHiveId*>(old->id(k)));
                if( objHiveId != keyId ) {
                    sObjPerm * op = m_ObjPermission->set(&keyId, sizeof(sHiveId));
                    sObjPerm * pp = old->ptr(k);
                    if( op && pp ) {
                        op->type = pp->type;
                        op->allow = pp->allow;
                        op->deny = pp->deny;
                        op->expiration = pp->expiration;
                    }
                }
            }
        }
    }
}

bool sUsr::cacheObj(const sHiveId & id, const sHiveId * type, udx flags, udx bits) const
{
    if( id ) {
        if( !m_ObjPermission.get() ) {
            m_ObjPermission.reset(new TPermCache::element_type);
        }
        if( !m_ObjPermission.get() ) {
            return false;
        }
        sObjPerm * op = m_ObjPermission->get(&id, sizeof(id));
        if( !op ) {
            op = m_ObjPermission->set(&id, sizeof(id));
            if( op ) {
                op->type.reset();
                op->allow = ePermNone;
                op->deny = ePermNone;
                op->expiration = m_AllowExpiredObjects ? sObjPerm::eMaybeExpired : sObjPerm::eUnexpired;
            }
        }
        if( op ) {
            if( type ) {
                op->type = *type;
            }
            bits &= ePermMask;
            if( bits && !(flags & (eFlagOnHold | eFlagRevoked))) {
                if( flags & eFlagRestrictive ) {
                    op->deny |= bits;
                } else {
                    op->allow |= bits;
                }
            }
        } else {
            return false;
        }
    }
    return true;
}

bool sUsr::cacheObjPerm(const sVarSet& tbl) const
{
    enum
    {
        eColObjId = 0, eColType, eColPermission, eColFlags, eColViewName
    };
    for(idx r = 0; r < tbl.rows; ++r) {
        udx flags = tbl.uval(r, eColFlags, eFlagNone);
        if( flags & (eFlagOnHold | eFlagRevoked) ) {
            continue;
        }
        sHiveId objHiveId(tbl.uval(r, eColObjId), 0);
        const sUsrType2 * utype = sUsrType2::ensure(*this, tbl.val(r, eColType));
        if( utype ) {
            sHiveId tid(utype->id());
            if( tid.valid() ) {
                if( !cacheObj(objHiveId, tid ? &tid : 0, tbl.uval(r, eColFlags, eFlagNone), tbl.uval(r, eColPermission)) ) {
                    return false;
                }
            }
        }
    }
    return true;
}

#if _DEBUG_off
#define SRCHDBG(...) ::fprintf(stderr, __VA_ARGS__)
#else
#define SRCHDBG(...)
#endif

bool sUsr::copy2res(sUsrObjRes & res) const
{
    sStr tmp;
    while( db().resultNext() ) {
        idx pid = 0;
        while( db().resultNextRow() ) {
            sHiveId id(db().resultUValue(0), db().resultUValue(1), db().resultUValue(2));
            if( id ) {
                sUsrObjRes::TObjProp * p = res.add(id);
                if( p ) {
                    const char * nm = db().resultValue(3);
                    if( nm && nm[0] == '_' ) {
                        if( strcasecmp(&nm[1], "type") == 0 ) {
                            sHiveId tid;
                            tid.set(db().resultUValue(4), db().resultUValue(5), 0);
                            if( tid && !cacheObj(id, &tid, eFlagNone, ePermNone) ) {
                                return false;
                            }
                        } else if( strcasecmp(&nm[1], "acl") == 0 ) {
                            if( !cacheObj(id, 0, db().resultUValue(4), db().resultUValue(5)) ) {
                                return false;
                            }
                        } else if( strcasecmp(&nm[1], "perm") == 0 ) {
                            const char * path = tmp.printf(0, "1.%" DEC, ++pid);
                            tmp.add0(2);
                            const char * val;
                            sHiveId vw(db().resultUValue(7), db().resultUValue(8));
                            val = tmp.printf("%" UDEC ",%s,", db().resultUValue(4), vw ? vw.print() : "");
                            permPrettyPrint(tmp, db().resultUValue(6), db().resultUValue(5));
                            res.add(*p, nm, path, sLen(path), val, sLen(val));
                        }
                    } else {
                        idx path_len = 0, val_len = 0;
                        const char * path = db().resultValue(4, 0, &path_len);
                        const char * val = db().resultValue(5, 0, &val_len);
                        if( idx encoding = db().resultIValue(6) ) {
                            tmp.cut0cut();
                            idx blob_len = 0;
                            const void * blob_value = db().resultValue(7, 0, &blob_len);
                            if( decodeField(&tmp, encoding, val, val_len, blob_value, blob_len) ) {
                                val = tmp.ptr();
                                val_len = tmp.length();
                            } else {
                                val = 0;
                                val_len = 0;
                            }
                        }
                        res.add(*p, nm, path, path_len, val, val_len);
                    }
                }
            }
        }
    }
    return true;
}

udx sUsr::objsLowLevel(const char * type_names, const char * obj_filter_sql, const char * prop_filter_sql, const char * prop_name_csv,
                       bool permissions, const udx start, const udx count, sUsrObjRes * res, udx * total_qty, bool allowSysInternal, bool withTrash) const
{
    if( !res && !total_qty ) {
        return 0;
    }
    SRCHDBG("SEARCH QUERY %s%s FROM type(s): '%s' WHERE [[%s]] AND [[%s]] LIMIT %" UDEC ", %" UDEC " with%s total\n", prop_name_csv ? prop_name_csv : "NULL", permissions ? " +flag:_perm" : "", type_names, obj_filter_sql ? obj_filter_sql : "", prop_filter_sql ? prop_filter_sql : "", start, count, total_qty ? "" : "out");
    std::unique_ptr<sSql::sqlProc> p(getProc("sp_obj_get_v5"));
    if( total_qty ) {
        *total_qty = 0;
    }
    if( p.get() ) {
        sStr filter00, typeids;
        sVec<const sUsrTypeField*> fields;
        sDic<bool> props;
        if( type_names == 0 || strcmp(type_names, "*") != 0 ) {
            sVec<const sUsrType2 *> tout;
            sVec< sHiveId > tids;
            sUsrType2::find(*this, & tout, type_names && type_names[0] ? type_names : 0, 0, 0, 0, true);
            tids.resize(tout.dim());
            for(idx i = 0; i < tout.dim(); ++i ) {
                tids[i] = tout[i]->id();
            }
            if( tids.dim() ) {
                sSql::exprInList(typeids, "o.objTypeDomainID", "o.objTypeID", tids, false);
            } else {
                return 0;
            }
        }
        if( prop_name_csv ) {
            sStr tmp;
            sString::searchAndReplaceSymbols(&tmp, prop_name_csv, 0, ",", 0, 0, true, true, true, true);
            tmp.add0(2);
            for(const char * p = tmp; p; p = sString::next00(p)) {
                if( strcasecmp(p, "_perm") == 0 ) {
                    permissions = true;
                } else if( p[0] && !props.get(p) ) {
                    props.setString(p);
                    filter00.printf("%s", p);
                    filter00.add0(1);
                }
            }
        }
        if( filter00 ) {
            if( permissions ) {
                filter00.printf("%s", "_perm");
                filter00.add0(1);
            }
            filter00.add0(2);
        }
        p->Add(typeids.length() ? typeids.ptr() : "").Add(obj_filter_sql).Add(prop_filter_sql);
        if( m_AllowExpiredObjects ) {
            if( m_SuperUserMode ) {
                p->Add((idx)-1);
            } else {
                p->Add(idx(withTrash ? 2 : 0));
            }
        } else {
            p->Add(idx(withTrash ? 2 : 0));
        }
        p->Add(start).Add(count).Add(permissions);
        if( p->resultOpen() && db().resultNext() ) {
            db().resultNextRow();
            if( total_qty ) {
                *total_qty = db().resultUValue(0, 0);
            }
            const udx sid = db().resultUValue(1, 0);
            if( res ) {
                res->_total = db().resultUValue(0, 0);
                res->_start = start;
                if( !copy2res(*res) ) {
                    res->empty();
                    return 0;
                }
                if( res->dim() && (!prop_name_csv || props.dim()) ) {
                    if( filter00 && m_ObjPermission.get() ) {
                        for(sUsrObjRes::IdIter it = res->first(); res->has(it); res->next(it)) {
                            sObjPerm * op = m_ObjPermission->get(res->id(it), sizeof(sHiveId));
                            if( op ) {
                                if( const sUsrType2 * utype = sUsrType2::ensure(*this, op->type) ) {
                                    fields.cut(0);
                                    utype->findFields(*this, fields, filter00);
                                    for(idx k = 0; k < fields.dim(); ++k) {
                                        props.setString(fields[k]->name());
                                    }
                                }
                            }
                        }
                    }
                    sStr propsSql;
                    bool has_virtual_props = permissions;
                    for(idx i = 0; i < props.dim(); ++i) {
                        const char * nm = (const char*)props.id(i);
                        if( nm ) {
                            if( nm[0] == '_' ) {
                                has_virtual_props = true;
                            } else {
                                propsSql.printf(",%s", nm);
                            }
                        }
                    }
                    p.reset(getProc("sp_obj_prop_v2_2"));
                    if( p.get() ) {
                        p->Add(sid);
                        p->Add(propsSql ? propsSql.ptr(1) : "");
                        if( !p->resultOpen() || !copy2res(*res) ) {
                            res->empty();
                            return 0;
                        }
                    }
                    for(sUsrObjRes::IdIter it = res->first(); res->has(it); res->next(it)) {
                        const sHiveId * id = res->id(it);
                        std::unique_ptr<sUsrObj> obj(objFactory(*id));
                        if( obj.get() ) {
                            if( propsSql || has_virtual_props ) {
                                obj->propEval(*res, filter00, allowSysInternal);
                            }
                        } else {
                            res->del(it);
                        }
                    }
                }
            }
        }
    }
    return res ? res->dim() : 0;
}

udx sUsr::objs2(const char* type_names, sUsrObjRes & res, udx * total_qty, const char* prop, const char* value, const char * prop_name_csv, bool permissions, const udx start, const udx count, bool allowSysInternal, bool withTrash) const
{
    SRCHDBG("SEARCH QUERY type(s): '%s' --> '%s'[%s]\n", type_names, value ? value : "", prop ? prop : "");
    sStr v00, p00, o_flt, v_flt;
    sVec<sHiveId> id_incl, id_excl;
    sString::searchAndReplaceSymbols(&v00, value, 0, ",", 0, 0, true, true, true, true);
    sString::searchAndReplaceSymbols(&p00, prop, 0, ",", 0, 0, true, true, true, true);
    p00.add0(p00 ? 1 : 0);
    v00.add0(v00 ? 1 : 0);
    const char* pn_last = 0, * pv_last = 0;
#define OBJCMP "REGEXP"
#define OBJNOT "NOT "
    for(const char* pn = p00, * pv = v00; pn || pv; pn = pn ? sString::next00(pn) : 0, pv = pv ? sString::next00(pv) : 0) {
        pn = pn ? pn : pn_last;
        pv = pv ? pv : pv_last;
        pn_last = pn;
        pv_last = pv;
        const bool not_pn = pn && (pn[0] == '!');
        const bool not_pv = pv && (pv[0] == '!');
        pn = not_pn ? &pn[1] : pn;
        pv = not_pv ? &pv[1] : pv;
        if( pn && pn[0] && strcmp(pn, "*") != 0 ) {
            if( (strcmp(pn, "id") == 0 || strcmp(pn, "_id") == 0) ) {
                if( pv && pv[0] ) {
                    sHiveId * objid = not_pn ? id_excl.add(1) : id_incl.add(1);
                    if( objid ) {
                        objid->parse(pv);
                    }
                }
            } else if( (strcmp(pn, "_creator") == 0) ) {
                o_flt.printf(o_flt ? " AND (" : "(");
                o_flt.printf("o.creatorID %s ", (not_pn || not_pv) ? "!=" : "=");
                db().protect(o_flt, pv);
                o_flt.shrink00();
                o_flt.printf(")");
            } else {
                if( v_flt ) {
                    v_flt.addString(" OR ");
                }
                v_flt.printf("(f.name %s '", not_pn ? "<>" : "=");
                db().protect(v_flt, pn);
                v_flt.shrink00();
                v_flt.printf("'");
                if( pv && pv[0] ) {
                    v_flt.printf(" AND f.value %s" OBJCMP " '", not_pv ? OBJNOT : "");
                    db().protect(v_flt, pv);
                    v_flt.shrink00();
                    v_flt.printf("'");
                }
                v_flt.printf(")");
            }
        } else if( pv && pv[0] && strcmp(pv, "*") != 0 && strcmp(pv, ".*") != 0 ) {
            if( v_flt ) {
                v_flt.addString(" OR ");
            }
            v_flt.printf("((f.value %s" OBJCMP " '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("') OR (CHAR(f.domainID USING ASCII) %s" OBJCMP " '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("') OR (f.objID %s" OBJCMP " '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("'))");
        }
    }
    if( id_incl ) {
        o_flt.printf(o_flt ? " AND (" : "(");
        sSql::exprInList(o_flt, "o.domainID", "o.objID", id_incl, false);
        o_flt.printf(")");
    }
    if( id_excl ) {
        o_flt.printf(o_flt ? " AND (" : "(");
        sSql::exprInList(o_flt, "o.domainID", "o.objID", id_excl, true);
        o_flt.printf(")");
    }
#undef OBJCMP
#undef OBJNOT
    return objsLowLevel(type_names, o_flt, v_flt.ptr(0), prop_name_csv, permissions, start, count, &res, total_qty, allowSysInternal, withTrash);
}

udx sUsr::objs(const sHiveId * ids, const udx cnt_ids, sVec<sHiveId>& out) const
{
    sStr pn, pv;
    for(udx i = 0; i < cnt_ids; ++i) {
        if( !m_ObjPermission.get() || !m_ObjPermission->get(ids + i, sizeof(sHiveId)) ) {
            pn.printf(",_id");
            pv.printf(",");
            ids[i].print(pv);
        }
    }
    if( pv ) {
        sUsrObjRes res;
        objs2("*", res, 0, pn.ptr(1), pv.ptr(1), "", false, 0, 0);
    }
    for(udx i = 0, k = 0; m_ObjPermission.get() && i < cnt_ids; ++i) {
        if( m_ObjPermission->get(ids + i, sizeof(sHiveId)) ) {
            (*out.ptrx(k++)) = ids[i];
        }
    }
    return out.dim();
}

void sUsr::propBulk(sVec<sHiveId> & ids, sVarSet & list, const char* view_name, const char* filter00, bool allowSysInternal) const
{
    if( ids.dim() ) {
        sStr idcsv(",");
        sSql::exprInList(idcsv, "domainID", "objID", ids, false);
        const bool hasBrief = filter00 && (sString::compareChoice("_brief", filter00, 0, true, 0, true) != sNotIdx);
        const bool hasSummary = filter00 && (sString::compareChoice("_summary", filter00, 0, true, 0, true) != sNotIdx);
        sStr prp, pkey;
        if( hasBrief || hasSummary ) {
            sDic<bool> uniq;
            for(const char * f = filter00; f; f = sString::next00(f)) {
                uniq.set(f, sLen(f) + 1);
                pkey.printf("%s", f);
                pkey.add0();
            }
            const char * pp[2];
            pp[0] = hasBrief ? "_brief" : 0;
            pp[1] = hasSummary ? "_summary" : 0;
            sVarSet props;
            for(idx i = 0; i < ids.dim() ; ++i) {
                std::unique_ptr<sUsrObj> obj(objFactory(ids[i]));
                if( obj.get() ) {
                    for(idx p = 0; p < sDim(pp); ++p) {
                        if( pp[p] ) {
                            props.empty();
                            if( const sUsrType2 * utype = obj->getType() ) {
                                utype->props(*this, props, pp[p]);
                            }
                            idx cnm = props.colId("name");
                            idx cvrt = props.colId("is_virtual_fg");
                            for(idx r = 0; r < props.rows; ++r) {
                                const char * nm = props.val(r, cnm);
                                if( !props.uval(r, cvrt) ) {
                                    uniq.set(nm, sLen(nm) + 1);
                                }
                                if( p > 0 ) {
                                    pkey.printf("%s", nm);
                                    pkey.add0();
                                }
                            }
                        }
                    }
                }
            }
            for(idx i = 0; i < uniq.dim(); ++i) {
                const char * nm = (const char *)(uniq.id(i));
                prp.printf("%s,", nm);
            }
            prp.cut0cut(-1);
            pkey.add0(2);
            filter00 = pkey;
        } else {
            sString::glue00(&prp, filter00, "%s", ",");
        }
        std::unique_ptr<sSql::sqlProc> p(getProc("sp_obj_prop_list_v2"));
        p->Add(idcsv.ptr(1)).Add(prp);
        sVarSet vtmp;
        p->getTable(&vtmp);
        for(idx i = 0; i < ids.dim(); ++i) {
            std::unique_ptr<sUsrObj> obj(objFactory(ids[i]));
            if( obj.get() ) {
                obj->propBulk(vtmp, list, view_name, filter00, allowSysInternal);
            }
        }
    }
}

udx sUsr::all(sDic<udx> & list, const char* types) const
{
    sUsrObjRes out;
    if( objs2(types, out, 0, 0, 0, "") ) {
        list.mex()->flags |= sMex::fSetZero;
        for(sUsrObjRes::IdIter it = out.first(); out.has(it); out.next(it)) {
            const sHiveId * id = out.id(it);
            if( id ) {
                std::unique_ptr<sUsrObj> obj(objFactory(*id));
                if( obj.get() ) {
                    const char * tnm = obj->getTypeName();
                    if( tnm ) {
                        udx * q = list.setString(tnm);
                        if( q ) {
                            *q = *q + 1;
                        }
                    }
                }
            }
        }
    }
    return list.dim();
}

sUsrObj* sUsr::objFactory(const sHiveId & id) const
{
    std::unique_ptr<sUsrObj> obj;
    obj.reset(id ? new sUsrObj(*this, id) : 0);
    if( !obj.get() || !obj->Id() ) {
        obj.reset();
    } else {
        const sUsrType2 * typ = obj->getType();
        const char * tpnm = typ ? typ->name() : 0;
        if( typ && tpnm ) {
            if( typ->isDescendentOf("process") ) {
                obj.reset(new sUsrProc(const_cast<sUsr&>(*this), id));
            } else if( typ->isDescendentOf("file") ) {
                obj.reset(new sUsrFile(id, const_cast<sUsr*>(this)));
            } else if( strcasecmp(tpnm, "email") == 0 ) {
                obj.reset(new sUsrEmail(const_cast<sUsr&>(*this), id));
            } else if( strcasecmp(tpnm, "folder") == 0 ) {
                obj.reset(new sUsrFolder(const_cast<sUsr&>(*this), id));
            } else if( strcasecmp(tpnm, "sysfolder") == 0 ) {
                obj.reset(new sSysFolder(const_cast<sUsr&>(*this), id));
            }
        } else if( !m_AllowExpiredObjects ) {
            obj.reset();
        }
    }
    return obj.release();
}

sUsrObj* sUsr::objFactory(const sHiveId & id, const sHiveId * ptypeId, udx permission) const
{
    std::unique_ptr<sUsrObj> obj;
    obj.reset(id ? new sUsrObj(*this, id, ptypeId, permission) : 0);
    if( !obj.get() || !obj->Id() ) {
        obj.reset();
    } else {
        const sUsrType2 * typ = ptypeId ? sUsrType2::ensure(*this, *ptypeId) : obj->getType();
        const char * tpnm = typ ? typ->name() : 0;
        if( typ && tpnm ) {
            if( typ->isDescendentOf("process") ) {
                obj.reset(new sUsrProc(const_cast<sUsr&>(*this), id, ptypeId, permission));
            } else if( typ->isDescendentOf("file") ) {
                obj.reset(new sUsrFile(const_cast<sUsr&>(*this), id, ptypeId, permission));
            } else if( strcasecmp(tpnm, "email") == 0 ) {
                obj.reset(new sUsrEmail(const_cast<sUsr&>(*this), id, ptypeId, permission));
            } else if( strcasecmp(tpnm, "folder") == 0 ) {
                obj.reset(new sUsrFolder(const_cast<sUsr&>(*this), id, ptypeId, permission));
            } else if( strcasecmp(tpnm, "sysfolder") == 0 ) {
                obj.reset(new sSysFolder(const_cast<sUsr&>(*this), id, ptypeId, permission));
            }
        } else if( !m_AllowExpiredObjects ) {
            obj.reset();
        }
    }
    return obj.release();
}

bool sUsr::allowExpiredObjects(bool allowed)
{
    if( !m_SuperUserMode ) {
#if _DEBUG
        fprintf(stderr, "WARNING: sUsr API misuse: fetching expired objects is permitted only in superuser mode\n");
#endif
        return false;
    }
    m_AllowExpiredObjects = allowed;
    return true;
}

idx sUsr::listUsr(sVec < sStr > * userList, idx isgrp, bool allUsr, bool active, const char * search, bool primaryGrpOnly, bool billable, bool with_system) const
{
    sStr sql("SELECT IF(`type` = 'group', last_name, CONCAT(first_name, ' ', last_name)), groupPath, u.userID"
            " FROM UPUser u JOIN UPGroup g USING(userID) WHERE");
    if( with_system ) {
        sql.addString(" (`type` NOT IN ('system','service') OR groupPath NOT LIKE '%/')");
    } else {
        sql.addString(" `type` NOT IN ('system','service')");
    }
    if( active ) {
        sql.printf(" AND u.is_active_fg = TRUE AND g.is_active_fg = TRUE");
    }
    if( billable ) {
        sql.printf(" AND u.is_billable_fg = TRUE");
    }
    if( !allUsr && !primaryGrpOnly ) {
        sql.printf(" AND g.groupPath NOT LIKE '/everyone/%%'");
    }
    sql.printf(" AND g.groupPath %sLIKE '%%/'", isgrp ? "" : "NOT ");
    if( search && search[0] ) {
        sStr tmp("%%");
        db().protect(tmp, search);
        tmp.shrink00();
        tmp.printf("%%");
        sql.printf(" AND (u.first_name LIKE '%s' OR u.last_name LIKE '%s')", tmp.ptr(), tmp.ptr());
    }
    if( primaryGrpOnly ) {
        sql.addString(" AND (g.flags = -1)");
    }
    sql.printf(" ORDER BY IF(u.`type` = 'group', '', u.last_name), IF(u.`type` = 'group', '', u.first_name), u.userID");
    sVarSet tbl;
    db().getTable(sql,&tbl);
    if(!tbl.rows)return 0;

    sStr * ul=userList->add(tbl.rows);

    for ( idx ir=0; ir<tbl.rows; ++ir) {
        for ( idx ic=0; ic<tbl.cols; ++ic) {
            ul[ir].printf("%s",tbl.val(ir,ic));
            ul[ir].add0();
        }
    }

    return userList->dim();
}

idx sUsr::listGrp(sVec < sStr > * userList, idx isgrp, idx usrOnly, const char * search, bool with_system, bool with_service) const
{
    sStr sql("SELECT IF(u.`type` = 'group', u.last_name, CONCAT(u.first_name, ' ', u.last_name)), g.groupPath, g.groupID"
            " FROM UPGroup g JOIN UPUser u USING(userID) WHERE u.is_active_fg = TRUE AND g.is_active_fg = TRUE");
    if( !with_system || !with_service) {
        sql.addString(" AND u.`type` NOT IN (");
        if( !with_system ) {
            sql.addString("'system'");
        }
        if( !with_service ) {
            if( sql[sql.length() - 1] != '(' ) {
                sql.addString(",");
            }
            sql.addString("'service'");
        }
        sql.addString(")");
    }
    if( isgrp || usrOnly ) {
        sql.printf(" AND (");
        if(isgrp) sql.printf(" g.groupPath LIKE '%%/'");
        sql.printf(" %s", isgrp && usrOnly ? "OR" : "");
        if(usrOnly) sql.printf(" (g.flags = -1 AND u.`type` = 'user')");
        sql.printf(")");
    }
    if( search && search[0] ) {
        sStr tmp("%%");
        db().protect(tmp, search);
        tmp.shrink00();
        tmp.printf("%%");
        sql.printf(" AND (u.first_name LIKE '%s' OR u.last_name LIKE '%s')", tmp.ptr(), tmp.ptr());
    }
    sql.printf(" ORDER BY IF(u.`type` = 'group', '', u.last_name), IF(u.`type` = 'group', '', u.first_name), u.userID");
    sVarSet tbl;
    db().getTable(sql,&tbl);
    if(!tbl.rows)return 0;

    sStr * ul=userList->add(tbl.rows);

    for ( idx ir=0; ir<tbl.rows; ++ir) {
        for ( idx ic=0; ic<tbl.cols; ++ic) {
            ul[ir].printf("%s",tbl.val(ir,ic));
            ul[ir].add0();
        }
    }
    return userList->dim();
}

static idx groupPathPrefixLen(const char * group_path)
{
    const char * email = sFilePath::nextToSlash(group_path);
    return email > group_path ? email - group_path : 0;
}

idx sUsr::printUserInfo(sJSONPrinter & out, bool into_object, const sVec<udx> * user_ids, bool without_current, const char * path_prefix, bool with_inactive, bool with_system)
{
    if( !Id() || isGuest() ) {
        user_ids = 0;
        path_prefix = 0;
        with_inactive = false;
    }
    sStr sql("SELECT u.userID, u.is_active_fg, u.is_admin_fg, u.is_email_valid_fg, u.email, u.first_name, u.last_name, u.createTm, u.modifTm, u.loginTm, u.max_sessions, u.is_billable_fg, "
        "g.groupID, g.flags, g.is_active_fg AS group_is_active_fg, g.groupPath, g.createTm FROM UPUser u JOIN UPGroup g USING(userID)");

    if( path_prefix ) {
        sql.addString(" JOIN UPGroup g2 USING(userID)");
    }
    sql.addString(" WHERE (u.type = 'user' OR (u.type = 'system' AND u.email = 'guest')");
    if( with_system ) {
        sql.addString(" OR g.groupPath NOT LIKE '%/'");
    }
    sql.addString(")");

    if( !with_inactive ) {
        sql.addString(" AND u.is_active_fg AND g.is_active_fg");
        if( path_prefix ) {
            sql.addString("  AND g2.is_active_fg");
        }
    }

    bool disjunct = false;
    if( user_ids && user_ids->dim() ) {
        if( disjunct ) {
            sql.addString(" OR ");
        } else {
            sql.addString(" AND (");
            disjunct = true;
        }
        sSql::exprInList(sql, "u.userID", *user_ids);
    }
    if( !without_current ) {
        if( disjunct ) {
            sql.addString(" OR ");
        } else {
            sql.addString(" AND (");
            disjunct = true;
        }
        sql.printf("u.userID = %" UDEC, this->Id());
    }
    if( path_prefix ) {
        if( disjunct ) {
            sql.addString(" OR ");
        } else {
            sql.addString(" AND (");
            disjunct = true;
        }
        sql.addString("g2.groupPath LIKE '");
        db().protect(sql, path_prefix);
        sql.shrink00();
        sql.addString("%'");
    }

    if( disjunct ) {
        sql.addString(")");
    }

    if( path_prefix ) {
        sql.addString(" GROUP BY g.groupID");
    }
    sql.addString(" ORDER BY u.userID ASC, g.flags ASC;");
    sVarSet tbl;
    db().getTable(sql, &tbl);
    sDic<idx> seen_ids;

    if( !into_object ) {
        out.startObject();
    }
    if( tbl.rows ) {
        const idx user_id_icol = tbl.colId("userID");
        const idx email_icol = tbl.colId("email");
        const idx first_name_icol = tbl.colId("first_name");
        const idx last_name_icol = tbl.colId("last_name");
        const idx user_is_active_fg_icol = tbl.colId("is_active_fg");
        const idx group_is_active_fg_icol = tbl.colId("group_is_active_fg");
        const idx group_id_icol = tbl.colId("groupID");
        const idx group_path_icol = tbl.colId("groupPath");
        const idx group_flags_icol = tbl.colId("flags");
        if( !without_current ) {
            out.addKey("current_user");
            out.startObject();
            out.addKey("_id");
            out.addValue(Id());
            out.addKeyValue("email", Email());
            out.addKeyValue("first_name", firstName());
            out.addKeyValue("last_name", lastName());
            for (idx ir = 0; ir < tbl.rows; ir++) {
                udx user_id = tbl.uval(ir, user_id_icol);
                if( user_id == Id() ) {
                    out.addKeyValue("is_active_fg", tbl.boolval(ir, user_is_active_fg_icol));
                    break;
                }
            }
            if( isAdmin() ) {
                out.addKeyValue("is_admin_fg", isAdmin());
            }
            if( isGuest() ) {
                out.addKeyValue("_is_guest_fg", isGuest());
            }
            out.addKeyValue("_primary_group_id", groupId());
            out.addKey("_groups");
            out.startArray();
            for (idx ir = 0; ir < tbl.rows; ir++) {
                udx user_id = tbl.uval(ir, user_id_icol);
                if( user_id == Id() ) {
                    seen_ids.set(&user_id, sizeof(user_id));
                    out.startObject();
                    out.addKeyValue("_id", tbl.uval(ir, group_id_icol));
                    const char * group_path = tbl.val(ir, group_path_icol);
                    out.addKeyValue("path", group_path);
                    out.addKeyValue("_path_prefix", group_path, groupPathPrefixLen(group_path));
                    out.addKeyValue("is_active_fg", tbl.boolval(ir, group_is_active_fg_icol));
                    out.endObject();
                }
            }
            out.endArray();
            out.endObject();
        }

        for (idx ir = 0; ir < tbl.rows; ir++) {
            udx user_id = tbl.uval(ir, user_id_icol);
            if( seen_ids.get(&user_id, sizeof(user_id)) ) {
                continue;
            }
            seen_ids.set(&user_id, sizeof(user_id));
            out.addKey(tbl.val(ir, user_id_icol));
            out.startObject();
            out.addKeyValue("_id", user_id);
            out.addKeyValue("first_name", tbl.val(ir, first_name_icol));
            out.addKeyValue("last_name", tbl.val(ir, last_name_icol));
            out.addKeyValue("is_active_fg", tbl.boolval(ir, user_is_active_fg_icol));
            if( isAdmin() ) {
                out.addKeyValue("email", tbl.val(ir, email_icol));
            }

            if( sIsExactly(tbl.val(ir, email_icol), "guest") ) {
                out.addKeyValue("_is_guest_fg", true);
            }
            if( tbl.ival(ir, group_flags_icol) == -1 ) {
                out.addKeyValue("_primary_group_id",  tbl.ival(ir, group_id_icol));
            }
            out.addKey("_groups");
            out.startArray();
            while(ir < tbl.rows) {
                out.startObject();
                out.addKeyValue("_id", tbl.uval(ir, group_id_icol));
                const char * group_path = tbl.val(ir, group_path_icol);
                out.addKeyValue("_path_prefix", group_path, groupPathPrefixLen(group_path));
                out.addKeyValue("is_active_fg", tbl.boolval(ir, group_is_active_fg_icol));
                out.endObject();
                if( ir + 1 < tbl.rows && tbl.uval(ir + 1, user_id_icol) == user_id ) {
                    ir++;
                } else {
                    break;
                }
            }

            out.endArray();
            out.endObject();
        }
    }
    if( !into_object ) {
        out.endObject();
    }
    return seen_ids.dim();
}

idx sUsr::listUserGroups(sVarSet & tbl, udx user_id, bool active) const
{
    sStr sql("SELECT groupID, userID, flags, is_active_fg, groupPath, createTm FROM UPGroup WHERE userID = %" UDEC, user_id);
    if( active ) {
        sql.addString(" AND is_active_fg");
    }
    sql.addString(" ORDER BY groupID");
    return db().getTable(sql, &tbl);
}

idx sUsr::listInactive(sVec<sStr> * userList, idx isgrp)
{
    sStr sql;

    if( isgrp ) {
        sql.printf("SELECT groupID, groupPath FROM UPGroup WHERE is_active_fg = FALSE");
    } else {
        sql.printf("SELECT userID, first_name, last_name, email FROM UPUser WHERE is_active_fg = FALSE AND `type` = 'user'");
    }
    sVarSet tbl;
    db().getTable(sql, &tbl);
    if( !tbl.rows ) {
        return 0;
    }
    sStr * ul = userList->add(tbl.rows);
    for(idx ir = 0; ir < tbl.rows; ++ir) {
        for(idx ic = 0; ic < tbl.cols; ++ic) {
            if( ic != 0 ) {
                ul[ir].printf(",");
            }
            ul[ir].printf("%s", tbl.val(ir, ic));
        }
    }
    return userList->dim();
}

static const char * nonEmptyVal(sVarSet & tbl, idx ir, idx ic)
{
    const char * s = tbl.val(ir, ic);
    return s && s[0] ? s : 0;
}

idx sUsr::exportUsrGrp4Ion(sJSONPrinter & out)
{
    sStr sql, buf;
    sql.printf("SELECT userID, is_active_fg, is_admin_fg, is_email_valid_fg, type, email, pswd, pswd_reset_id, pswd_changed, pswd_prev_list, first_name, last_name, logCount, UNIX_TIMESTAMP(createTm) AS createTm, UNIX_TIMESTAMP(modifTm) AS modifTm, UNIX_TIMESTAMP(loginTm) AS loginTm, max_sessions FROM UPUser WHERE `type` != 'group'");
    sVarSet utbl;
    db().getTable(sql, &utbl);
    if( !utbl.rows ) {
        return 0;
    }

    sql.printf(0, "SELECT groupID, userID, flags, is_active_fg, groupPath, UNIX_TIMESTAMP(createTm) AS createTm FROM UPGroup");
    sVarSet gtbl;
    db().getTable(sql, &gtbl);
    if( !gtbl.rows ) {
        return 0;
    }

    out.startObject();
    out.addKey("users");
    out.startObject();
    for(idx ir = 0; ir < utbl.rows; ir++) {
        out.addKey(buf.printf(0, "u%" DEC, utbl.ival(ir, utbl.colId("userID"))));
        out.startObject();
        out.addKeyValue("_type", "hc_user");
        out.addKeyValue("_id", "$newid()");
        if( const char * first_name = nonEmptyVal(utbl, ir, utbl.colId("first_name")) ) {
            out.addKeyValue("fst_name", first_name);
        }
        if( const char * last_name = nonEmptyVal(utbl, ir, utbl.colId("last_name")) ) {
            out.addKeyValue("lst_name", last_name);
        }
        out.addKeyValue("email", utbl.val(ir, utbl.colId("email")), 0, true);
        if( utbl.boolval(ir, utbl.colId("is_email_valid_fg")) ) {
            out.addKeyValue("ok_email", true);
        }
        if( !utbl.boolval(ir, utbl.colId("is_active_fg")) ) {
            out.addKeyValue("inactive", true);
        }
        if( utbl.boolval(ir, utbl.colId("is_admin_fg")) ) {
            out.addKeyValue("is_admin", true);
        }
        out.addKeyValue("max_sess", utbl.ival(ir, utbl.colId("max_sessions")));
        if( const char * pswd = utbl.val(ir, utbl.colId("pswd")) ) {
            if( pswd[0] && pswd[0] != '-' ) {
                out.addKeyValue("pswd", pswd);
            }
        }
        if( const char * pswd_reset_id = nonEmptyVal(utbl, ir, utbl.colId("pswd_reset_id")) ) {
            out.addKeyValue("pswd_rst", pswd_reset_id);
        }
        if( const char * pswd_prev_list = nonEmptyVal(utbl, ir, utbl.colId("pswd_prev_list")) ) {
            out.addKey("pswd_prv");
            out.startArray();
            while( 1 ) {
                pswd_prev_list += strspn(pswd_prev_list, hash_seps);
                idx len = strcspn(pswd_prev_list, hash_seps);
                if( !len ) {
                    break;
                }
                out.addValue(pswd_prev_list, len);
                pswd_prev_list += len;
            }
            out.endArray();
        }

        if( idx cnt = utbl.ival(ir, utbl.colId("logCount")) ) {
            out.addKeyValue("log_cnt", cnt);
        }
        if( idx t = utbl.ival(ir, utbl.colId("loginTm")) ) {
            sVariant v;
            v.setDateTime(t);
            out.addKeyValue("logtime", v);
        }
        if( idx t = utbl.ival(ir, utbl.colId("createTm")) ) {
            sVariant v;
            v.setDateTime(t);
            out.addKeyValue("ctime", v);
        }
        if( idx t = utbl.ival(ir, utbl.colId("modifTm")) ) {
            sVariant v;
            v.setDateTime(t);
            out.addKeyValue("mtime", v);
        }
        out.addKeyValue("category", utbl.val(ir, utbl.colId("type")), 0, true);
        out.endObject();
    }
    out.endObject();


    out.addKey("groups");
    out.startObject();
    for(idx ir = 0; ir < gtbl.rows; ir++) {
        const char * path = gtbl.val(ir, gtbl.colId("groupPath"));
        idx path_len = sLen(path);
        if( !path_len ) {
            continue;
        }
        if( path[path_len - 1] != '/' ) {
            continue;
        }
        const char * name = path;
        for(idx ip = path_len - 2; ip >= 0; ip--) {
            if( path[ip] == '/' ) {
                name = path + ip + 1;
                break;
            }
        }

        out.addKey(buf.printf(0, "g%" DEC, gtbl.ival(ir, gtbl.colId("groupID"))));
        out.startObject();
        out.addKeyValue("_type", "hc_group");
        out.addKeyValue("_id", "$newid()");

        out.addKeyValue("name", name, sLen(name) - 1, true);
        if( !gtbl.boolval(ir, gtbl.colId("is_active_fg")) ) {
            out.addKeyValue("inactive", true);
        }

        out.addKey("members");
        out.startArray();
        for(idx jr = 0; jr < gtbl.rows; jr++) {
            const char * subpath = gtbl.val(jr, gtbl.colId("groupPath"));
            idx subpath_len = sLen(subpath);
            if( subpath_len <= path_len ) {
                continue;
            }
            if( strncmp(path, subpath, path_len) != 0 ) {
                continue;
            }
            const char * next_slash = strchr(subpath + path_len, '/');
            if( next_slash ) {
                if( next_slash[1] ) {
                    continue;
                } else {
                    out.addValue(buf.printf(0, "$root.groups.g%" DEC, gtbl.ival(jr, gtbl.colId("groupID"))));
                }
            } else {
                out.addValue(buf.printf(0, "$root.users.u%" DEC, gtbl.ival(jr, gtbl.colId("userID"))));
            }
        }
        out.endArray();

        if( idx t = gtbl.ival(ir, gtbl.colId("createTm")) ) {
            sVariant v;
            v.setDateTime(t);
            out.addKeyValue("ctime", v);
        }

        out.endObject();
    }
    out.endObject();

    out.endObject();

    return utbl.rows + gtbl.rows;
}

udx sUsr::getGroupId(const char * grp_name, bool reset_cache) const
{
    sStr case_buf;
    static sDic<udx> group_ids;
    if( reset_cache ) {
        group_ids.empty();
    }
    if( !group_ids.dim() ) {
        sVec<sStr> table;
        listGrp(&table, 0, 0, 0, true, true);
        for(idx i = 0; i < table.dim(); i++) {
            const char * user_name = table[i].ptr();
            const char * group_path = sString::next00(user_name);
            const char * group_id_str = sString::next00(group_path);
            udx group_id = group_id_str ? atoudx(group_id_str) : 0;
            if( group_id && group_path ) {
                case_buf.cut(0);
                *group_ids.set(canonicalCase(case_buf, group_path)) = group_id;
            }
        }
    }

    case_buf.cut(0);
    grp_name = canonicalCase(case_buf, grp_name);
    if( const udx * pgroup_id = group_ids.get(grp_name) ) {
        return *pgroup_id;
    }
    return 0;
}

bool sUsrObjRes::del(TObjProp & obj, const char * name) const
{
    idx * t = obj.get(name, sLen(name) + 1);
    if( t && *t > 0 ) {
        *t = -(*t);
        return true;
    }
    return false;
}

bool sUsrObjRes::del(TPropTbl * tbl) const
{
    if( tbl && tbl->path > 0 ) {
        tbl->path = -tbl->path;
        return true;
    }
    return false;
}

void sUsrObjRes::csv(const sUsrObjRes::IdIter & it, sStr & buf) const
{
    const sHiveId * oid = id(it);
    if( oid ) {
        const TObjProp * prop = get(it);
        for(idx p = 0; prop && p < prop->dim(); ++p) {
            const char * prop_name = (const char *) prop->id(p);
            const TPropTbl * tbl = get(*prop, prop_name);
            while( tbl ) {
                oid->print(buf);
                buf.printf(",%s,", prop_name);
                const char * path = getPath(tbl);
                if( path && path[0] ) {
                    sString::escapeForCSV(buf, path);
                }
                buf.printf(",");
                sString::escapeForCSV(buf, getValue(tbl));
                buf.printf("\n");
                tbl = getNext(tbl);
            }
        }
    }
}

void sUsrObjRes::prop(const sUsrObjRes::IdIter & it, sStr & buf) const
{
    const sHiveId * oid = id(it);
    if( oid ) {
        const TObjProp * prop = get(it);
        for(idx p = 0; prop && p < prop->dim(); ++p) {
            const char * prop_name = (const char *) prop->id(p);
            const TPropTbl * tbl = get(*prop, prop_name);
            while( tbl ) {
                buf.printf("\nprop.");
                oid->print(buf);
                const char * path = getPath(tbl);
                path = (path && path[0]) ? path : 0;
                buf.printf(".%s%s%s=%s", prop_name, path ? "." : "", path ? path : "", getValue(tbl));
                tbl = getNext(tbl);
            }
        }
    }
}

static void jsonPerm(sJSONPrinter & printer, const sUsrObjRes * res, const sUsrObjRes::TObjProp * prop, const char * key)
{
    printer.addKey(key);
    printer.startArray();

    const sUsrObjRes::TPropTbl * tbl = res->get(*prop, key);
    while( tbl ) {
        const char * grp_perm_pretty_print = res->getValue(tbl);
        udx num_group = 0;
        int num_group_nbytes = 0;
        if( sscanf(grp_perm_pretty_print, "%" UDEC ",,%n", &num_group, &num_group_nbytes) ) {
            permPretty2JSON(printer, num_group, 0, grp_perm_pretty_print + num_group_nbytes);
        }
        tbl = res->getNext(tbl);
    }
    printer.endArray();
}

void sUsrObjRes::json(const sUsr & user, const sUsrObjRes::IdIter & it, sJSONPrinter & printer, bool into_object, bool flatten, bool upsert, const char * upsert_qry, const char * prop_filter00, const char * prop_exclude00) const
{
    const sHiveId * oid = id(it);
    if( oid ) {
        if( !into_object ) {
            printer.startObject();
        }
        sVarSet tree_table;
        sStr upsert_qry_buf;
        const TObjProp * prop = get(it);
        idx iprop_perm = sIdxMax, iprop_effperm = sIdxMax;
        for(idx p = 0; prop && p < prop->dim(); ++p) {
            const char * prop_name = (const char *) prop->id(p);
            if( prop_name && prop_filter00 && sString::compareChoice(prop_name, prop_filter00, 0, false, 0, true) < 0 ) {
                continue;
            }
            if( prop_name && prop_exclude00 && sString::compareChoice(prop_name, prop_exclude00, 0, false, 0, true) >= 0 ) {
                continue;
            }
            if( prop_name && strcmp(prop_name, "_perm") == 0 ) {
                iprop_perm = sMin<idx>(iprop_perm, p);
            } else if( prop_name && strcmp(prop_name, "_effperm") == 0 ) {
                iprop_effperm = sMin<idx>(iprop_effperm, p);
            } else {
                const TPropTbl * tbl = get(*prop, prop_name);
                while( tbl ) {
                    tree_table.addRow().addCol(oid->print()).addCol(prop_name).addCol(getPath(tbl)).addCol(getValue(tbl));
                    tbl = getNext(tbl);
                }
            }
        }
        if( prop ) {
            printer.addKey("_id");
            if( upsert ) {
                if( upsert_qry && upsert_qry[0] ) {
                    upsert_qry_buf.printf(0, "$upsert_qry(%s)", upsert_qry);
                    printer.addValue(upsert_qry_buf.ptr());
                } else {
                    printer.addValue("$upsert()");
                }
            } else {
                printer.addValue(*oid);
            }
            const sUsrType2 * utype = user.objType(*oid);
            if( utype ) {
                printer.addKey("_type");
                printer.addValue(utype->name());
                sUsrObjPropsTree tree(user, utype, tree_table);
                tree.printJSON(printer, true, flatten);
            }
            if( iprop_perm < sIdxMax ) {
                jsonPerm(printer, this, prop, "_perm");
            }
            if( iprop_effperm < sIdxMax ) {
                jsonPerm(printer, this, prop, "_effperm");
            }
        }
        if( !into_object ) {
            printer.endObject();
        }
    }
}

sUsrObjRes::TObjProp * sUsrObjRes::add(const sHiveId & id)
{
    idx prev_table_dim = _table.dim();
    idx index = 0;
    Optional<TObjProp> * opt = _table.set(&id, sizeof(id), &index);
    if( index == prev_table_dim || !opt->exists ) {
        _table_cnt++;
        resetFirstLast();
    }
    opt->exists = true;
    return opt->get();
}
bool sUsrObjRes::add(TObjProp & obj, const char * prop, const char * path, const idx path_len, const char * value, const idx value_len)
{
    const idx vnew_offset = _buf.mex()->add((const char*) 0, sizeof(TPropTbl));
    if( vnew_offset != sNotIdx ) {
        idx vp, vv, * q = 0;
        q = _buf.setString(path, path_len, &vp);
        if( q ) {
            *q = 1;
        }
        q = _buf.setString(value, value_len, &vv);
        if( q ) {
            *q = 1;
        }
        TPropTbl * tbl = (TPropTbl *) _buf.mex()->ptr(vnew_offset);
        tbl->path = vp;
        tbl->value = vv;
        tbl->next = 0;
        const idx nmlen = sLen(prop) + 1;
        idx * tail = obj.get(prop, nmlen);
        if( tail ) {
            do {
                tbl = (TPropTbl *) _buf.mex()->ptr(*tail);
                tail = &(tbl->next);
            } while( *tail > 0 );
        } else {
            tail = obj.set(prop, nmlen);
        }
        if( tail ) {
            *tail = vnew_offset;
            return true;
        }
    }
    return false;
}

sRC sUsr::createProject(sHiveId & outId, const char * name, const char * description, const char * typeName) {
    if( m_IsGuest ) {
        return RC(sRC::eCreating, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
    }
    const sUsrType2 * type = sUsrType2::ensure(*this, typeName);
    if( !type || !type->nameMatch("project+") ) {
        return RC(sRC::eCreating, sRC::eObject, sRC::eType, sRC::eInvalid);
    }
    if( sRC rc = objCreate(outId, typeName) ) {
        return rc;
    }
    const udx oldProject = m_currProj.projectId;
    if( sRC rc = setProject(outId.objId()) ) {
        return rc;
    }
    sUsrObj newProject(*this, outId);
    if( !newProject.Id().valid() ) {
        return RC(sRC::eAccessing, sRC::eObject, sRC::eID, sRC::eInvalid);
    }
    udx propSetSuccess = 0;
    propSetSuccess |= newProject.propSet("Title", name);
    propSetSuccess |= newProject.propSet("Description_full", description);
    if( propSetSuccess == 0 ) {
        setProject(oldProject);
        return RC(sRC::eSetting, sRC::eProperty, sRC::eOperation, sRC::eFailed);
    }
    if( sRC rc = setProject(oldProject) ) {
        return rc;
    }
    return sRC::zero;
}

sRC sUsr::setProject(udx projectId) const {
    bool foundProject = false;
    if( !projectId ) {
        m_currProj.projectId = 0;
        m_projMembership.cut(0);
        foundProject = true;
    } else {
        sVec<Project> projects;
        if( sRC rc = availableProjects(projects) ) {
            return rc;
        }
        for (idx i = 0; i < projects.dim(); i++) {
            if( projects[i].projectId == projectId ) {
                m_currProj = projects[i];
                char roleChar = projRoleToChar(m_currProj.role);
                m_projMembership.printf("(  ( (g.groupPath in ('/HIVEPROJ/%" UDEC "-%c/')) AND ((p.flags & 6) = 0) )  OR ( (g.groupPath in ('/HIVEPROJ/%" UDEC "-%c/', '/everyone/users/', '/everyone/')) AND ((p.flags & 2) != 0) ) )", m_currProj.projectId, roleChar, m_currProj.projectId, roleChar);
                foundProject = true;
                break;
            }
        }
    }
    if( !foundProject ) {
        return RC(sRC::eSearching, sRC::eGroup, sRC::eGroup, sRC::eNotFound);
    }
    return sRC::zero;
}

sRC sUsr::availableProjects(sVec<Project> & outProjects) const
{
    sRC rc;
    sVarSet res;
    const sStr query("SELECT groupID, groupPath FROM UPGroup WHERE is_active_fg = true AND userID = %" UDEC " AND groupPath like '/HIVEPROJ/%%';", m_Id);
    db().getTable(&res, query.ptr());
    for (idx i = 0; i < res.rows; ++i) {
        const char * projPath = res.val(i, res.colId("groupPath"));
        udx projId;
        char roleChar;
        const idx varsFilled = sscanf(projPath, "/HIVEPROJ/%" UDEC "-%c/", &projId, &roleChar);
        if( varsFilled == 2 ) {
            Project *newProj = outProjects.add();
            newProj->projectId = projId;
            newProj->userId = m_Id;
            newProj->role = projCharToRole(roleChar);
            const sStr parentIds("SELECT groupID FROM UPGroup WHERE is_active_fg=true AND groupPath='/HIVEPROJ/%" UDEC "-%c/';", projId, roleChar);
            sVarSet idRes;
            db().getTable(&idRes, parentIds.ptr(0));
            if( idRes.rows == 1 ) {
                newProj->groupId = idRes.uval(0, res.colId("groupID"));
            } else if( idRes.rows < 1 ) {
                RCSET(rc, sRC::eSelecting, sRC::eGroup, sRC::eList, sRC::eTooShort);
            } else if( idRes.rows > 1 ) {
                RCSET(rc, sRC::eSelecting, sRC::eGroup, sRC::eList, sRC::eTooLong);
            }
        } else {
            RCSET(rc, sRC::eScanning, sRC::eString, sRC::eItem, sRC::eIncomplete);
        }
    }
    return rc;
}

sRC sUsr::projectMembers(sVec<ProjectMember> &outMembers) const
{
    const sStr sqlQry("SELECT userID, groupPath FROM UPGroup WHERE is_active_fg=true AND flags=0 AND groupPath like '/HIVEPROJ/%" UDEC "-_/%%';", m_currProj.projectId);
    sVarSet res;
    db().getTable(&res, sqlQry.ptr(0));
    for (idx i = 0; i < res.rows; i++) {
        const char * projPath = res.val(i, res.colId("groupPath"));
        udx projId;
        char roleChar;
        const idx varsFilled = sscanf(projPath, "/HIVEPROJ/%" UDEC "-%c/", &projId, &roleChar);
        if( varsFilled != 2 ) {
            return RC(sRC::eScanning, sRC::eString, sRC::eItem, sRC::eIncomplete);
        }
        ProjectMember *newProj = outMembers.add();
        newProj->userId = res.uval(i, res.colId("userID"));
        newProj->role = projCharToRole(roleChar);
    }
    return sRC::zero;
}

sRC sUsr::allProjectRoles(sVec<Project> & out, udx projectId) const
{
    sVarSet res;
    db().getTable(&res, "SELECT groupID, userID, is_active_fg, groupPath FROM UPGroup WHERE is_active_fg=true AND groupPath like '/HIVEPROJ/%" UDEC "-_/';", projectId);
    if( res.rows != 4 ) {
        return RC(sRC::eSelecting, sRC::eGroup, sRC::eList, sRC::eUnexpected);
    }
    for (idx i = 0; i < res.rows; i++) {
        const char * projPath = res.val(i, res.colId("groupPath"));
        udx projId;
        char roleChar;
        const idx varsFilled = sscanf(projPath, "/HIVEPROJ/%" UDEC "-%c/", &projId, &roleChar);
        if( varsFilled == 2 ) {
            Project *newProj = out.add();
            newProj->projectId = projId;
            newProj->role = projCharToRole(roleChar);
            newProj->groupId = res.uval(i, res.colId("groupID"));
            newProj->userId =  res.uval(i, res.colId("userID"));
        } else {
            return RC(sRC::eScanning, sRC::eString, sRC::eItem, sRC::eIncomplete);
        }
    }

    return sRC::zero;
}


sRC sUsr::removeFromProject(udx userId)
{
    if( m_currProj.role != eProjectAdmin ) {
        return RC(sRC::eRemoving, sRC::eGroup, sRC::eUser, sRC::eNotAuthorized);
    }
    if( userId == m_Id ) {
        return RC(sRC::eRemoving, sRC::eGroup, sRC::eID, sRC::eProhibited);
    }
    const sStr query("SELECT groupID FROM UPGroup WHERE is_active_fg=true AND userID=%" UDEC " AND groupPath like '/HIVEPROJ/%" UDEC "-_/%%';", userId, m_currProj.projectId);
    sVarSet res;
    db().getTable(&res, query.ptr(0));
    if( res.rows < 1 ) {
        return RC(sRC::eSearching, sRC::eGroup, sRC::eGroup, sRC::eNotFound);
    } else if ( res.rows > 1 ) {
        return RC(sRC::eSearching, sRC::eGroup, sRC::eList, sRC::eTooLong);
    }
    const udx groupId = res.uval(0, res.colId("groupID"));
    db().execute("UPDATE UPGroup SET is_active_fg=false WHERE groupID=%" UDEC ";", groupId);
    return sRC::zero;
}

sRC sUsr::addToProject(udx userId, sUsr::EProjectRole role) const
{
    if( m_currProj.role != eProjectAdmin ) {
        return RC(sRC::eUpdating, sRC::eGroup, sRC::eUser, sRC::eNotAuthorized);
    }
    sVarSet res;
    const sStr usrSqlQry("SELECT email FROM UPUser WHERE is_active_fg=true AND userID=%" UDEC " AND type='user';", userId);
    db().getTable(&res, usrSqlQry.ptr(0));
    sStr emailBuf;
    if( res.rows != 1 ) {
        return RC(sRC::eSearching, sRC::eUser, sRC::eID, sRC::eNotFound);
    }
    db().protect(emailBuf, res.val(0, res.colId("email")));
    const char roleChar = projRoleToChar(role);
    res.empty();
    db().getTable(&res, "SELECT groupID FROM UPGroup WHERE is_active_fg=true AND userID=%" UDEC " AND groupPath like '/HIVEPROJ/%" UDEC "-_/%%';", userId, m_currProj.projectId);
    if( res.rows > 0 ) {
        return RC(sRC::eCreating, sRC::eGroup, sRC::eGroup, sRC::eExists);
    }
    res.empty();
    db().getTable(&res, "SELECT groupID FROM UPGroup WHERE is_active_fg=false AND userID=%" UDEC " AND groupPath like '/HIVEPROJ/%" UDEC "-%c/%%';", userId, m_currProj.projectId, roleChar);
    if( res.rows > 1 ) {
        return RC(sRC::eSelecting, sRC::eGroup, sRC::eList, sRC::eTooLong);
    } else if( res.rows == 1 ) {
        const udx groupId = res.uval(0, res.colId("groupID"));
        db().execute("UPDATE UPGroup SET is_active_fg=true WHERE groupID=%" UDEC ";", groupId);
    } else if( res.rows == 0 ) {
        db().execute("INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath) values(%" UDEC ", 0, 1, '/HIVEPROJ/%" UDEC "-%c/%s');", userId, m_currProj.projectId, roleChar, emailBuf.ptr(0));
    }
    return sRC::zero;
}

bool sUsr::moveCheck(const sVec<sHiveId> & inIds, sVec<sHiveId> & outIds, sStr & err) const
{






    return false;
}

bool sUsr::moveToProject(sVec<sHiveId> & ids, sStr & err) {




    return false;
}

bool sUsr::moveToHome(sVec<sHiveId> & ids, sStr & err) {



    return false;
}

sRC sUsr::clearAndSetPermissions(const sHiveId & objID, sDic<udx> & groupPermissions) const
{
    sRC rc;
    if( !updateStart() ) {
        RCSET(rc, sRC::eStarting, sRC::eSequence, sRC::eDatabase, sRC::eFailed);
    }

    if( !rc ) {
        for(idx i = 0; i < groupPermissions.dim(); i++) {
            const udx * groupId = static_cast<const udx *>(groupPermissions.id(i));
            const udx perms = *groupPermissions.ptr(i);
            if( !setPermission(*groupId, objID, perms, eFlagNone) ) {
                RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                break;
            }
        }
    }
    if( !rc ) {
        sVarSet res;
        sVec<sHiveId> ids;
        *ids.add() = objID;
        objPermAll(ids, res);
        for(idx i = 0; i < res.rows; i++) {
            const udx grp = res.uval(i,  res.colId("grp"));
            if( !groupPermissions.get(&grp, sizeof(udx), 0) ) {
                if( !setPermission(grp, objID, ePermNone, eFlagNone) ) {
                    RCSET(rc, sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                    break;
                }
            }
        }
    }

    if( !rc ) {
        if( !updateComplete() ) {
            RCSET(rc, sRC::eCommitting, sRC::eSequence, sRC::eDatabase, sRC::eFailed);
        }
    } else {
        if( !updateAbandon() ) {
            RCSET(rc, sRC::eCleaningUp, sRC::eSequence, sRC::eDatabase, sRC::eFailed);
        }
    }
    return rc;
}
