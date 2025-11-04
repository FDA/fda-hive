
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
#include <qlib/QPride.hpp>
#include <slib/std/crypt.hpp>
#include <xlib/md5.hpp>
#include <ulib/upropset.hpp>

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
        std::auto_ptr<sSql::sqlProc> p(db().Proc("sp_user_audit"));
        if( p.get() ) {
            p->Add(m_SID).Add(m_Id).Add(oper).Add(tmp);
            return p->execute();
        }
    }
    return false;
}

const char* const sUsr::getKey(void)
{
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
    return "";
}

const char* sUsr::encodeSID(sStr & sid, sStr & buf)
{
    /* requires site-specific implementation */
    return "";
}

static
const char* encodePasswordOld(const char* s, const char* salt, sStr& buf)
{
    /* requires site-specific implementation */
    return "";
}

static const char * hash_seps = " \t\r\n";

static
bool checkPassword(const char * cur_hash, idx cur_hash_len, const char * pass, const char * email, const char * user_id, sStr * upgraded_hash = 0)
{
    /* requires site-specific implementation */
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
        } cfg;
        sString::SectVar cfgVars[] = {
            { 0, "[HIVE]" _ "db" __, "%s=" HIVE_DB, "%s", &cfg.db },
            { 0, "[HIVE]" _ "server" __, "%s=" HIVE_DB_HOST, "%s", &cfg.server },
            { 0, "[HIVE]" _ "user" __, "%s=" HIVE_DB_USER, "%s", &cfg.user },
            { 0, "[HIVE]" _ "pass" __, "%s=" HIVE_DB_PWD, "%s", &cfg.pass },
            { 0, "[HIVE]" _ "debug" __, "%" UDEC "=0", "%s", &cfg.debug }
        };
        const char* cfgs[] = { "hive.cfg", "~/hive.cfg", "qapp.cfg", "~/qapp.cfg", "~/.my.cnf"};
        sStr b;
        for(idx i = 0; i < sDim(cfgs); ++i) {
            const char * fp=cfgs[i];
            if(cfgs[i][0]=='~' && cfgs[i][1]=='/' ) {
                b.cut(0);
                sDir::homedir(&b);
                b.shrink00();
                b.add(cfgs[i]+1);
                fp=b.ptr(0);
            }

            sFil inp(fp, sFil::fReadonly);
            if( inp.length() ) {
                sStr rst;
                sString::cleanMarkup(&rst, inp, inp.length(), "//" _ "/*" __, "\n" _ "*/" __, "\n", 0, false, false, true);
                sString::xscanSect(rst.ptr(), rst.length(), cfgVars, sDim(cfgVars));
                if( sm_cfg_db.connect(cfg.db, cfg.server, cfg.user, cfg.pass) == sSql::eConnected ) {
                    break;
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
    /* requires site-specific implementation */
#endif
    {
        std::auto_ptr<sSql::sqlProc> p(db().Proc("sp_user_session_v2"));
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

void sUsr::initFolders(bool keepHierarchy)
{
    const bool su = m_SuperUserMode;
    std::auto_ptr<sUsrFolder> sysFolder(sSysFolder::Home(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        setPermission(m_PrimaryGroup, sysFolder->Id(), ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        m_SuperUserMode = su;
    }
    sysFolder.reset(sSysFolder::Inbox(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        setPermission(m_PrimaryGroup, sysFolder->Id(), ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        m_SuperUserMode = su;
    }
    sysFolder.reset(sSysFolder::Trash(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        setPermission(m_PrimaryGroup, sysFolder->Id(), ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        m_SuperUserMode = su;
    }
}

idx sUsr::setAdmin(const char * email, idx super)
{
    sStr sql("update UPUser set is_admin_fg=%" DEC " where email='%s'",super,email);
    db().execute(sql);
    return super;
}

sUsr::ELoginResult sUsr::login(const char * email, const char * pswd, const udx token, const char * ipaddr, idx * plogCount)
{
    ELoginResult status = eUserBlocked;
    sVarSet t;
    sStr tmp;
    const char * lemail = db().protect(tmp, email);
    tmp.add0(2);
    sStr sql("SELECT is_active_fg, userID, is_email_valid_fg, is_admin_fg, pswd, pswd_reset_id"
        ", loginTm + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.accountExpireDays') DAY < NOW() AS account_expired"
        ", IFNULL(pswd_changed, NOW() - INTERVAL 200 YEAR) + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.pswdExpireDays') DAY < NOW() AS pswd_expired"
        ", UNIX_TIMESTAMP(NOW()) - UNIX_TIMESTAMP(IFNULL(login_failed_date, 0)) AS login_failed_secs_ago"
        ", (SELECT IF(val REGEXP '^[0-9]+$', val * 60, 0) FROM QPCfg WHERE par = 'user.loginAttemptsInMinutes') As login_lock_period_sec"
        ", (SELECT IF(val REGEXP '^[0-9]+$', val, 0) != 0 FROM QPCfg WHERE par = 'user.loginAttemptsWarnLockLast') As login_failed_warn"
        ", login_failed_count, (SELECT IF(val REGEXP '^[0-9]+$', val, 0) FROM QPCfg WHERE par = 'user.loginAttemptsMax') AS login_failed_count_max"
        ", (SELECT IF(val REGEXP '^[0-9]+$', val, 0) FROM QPCfg WHERE par = 'user.loginAttemptsLockAccount') AS login_failed_lock_acc"
        " FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
    db().getTable(&t, "%s", sql.ptr());
    if( t.rows == 1 ) {
        const bool lock_account = t.boolval(0, t.colId("login_failed_lock_acc"));
        if( token ) {
            status = (token == t.uval(0, t.colId("pswd_reset_id"))) ? eUserOperational : eUserNotFound;
        } else {
            const char * pp = t.val(0, t.colId("pswd"));
            sStr upgraded_pp;
            if( checkPassword(pp, 0, pswd, email, t.val(0, t.colId("userID")), &upgraded_pp) ) {
                if( upgraded_pp.length() ) {
                    db().execute("UPDATE UPUser SET pswd='%s', modifTm = CURRENT_TIMESTAMP WHERE userID=%" UDEC, upgraded_pp.ptr(), t.uval(0, t.colId("userID")));
                    audit(eUserAuditLogin, "upgrade_password", "userID=%" UDEC "; from='%s'; to='%s'", t.uval(0, t.colId("userID")), pp, upgraded_pp.ptr());
                    pp = upgraded_pp.ptr();
                }
    /* requires site-specific implementation */
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
        }
        if( status == eUserBlocked ) {
            const bool is_admin = t.boolval(0, t.colId("is_admin_fg"));
            if( !t.boolval(0, t.colId("is_email_valid_fg")) && !is_admin ) {
                status = eUserEmailNotValidated;
            } else if( t.boolval(0, t.colId("account_expired")) && !is_admin ) {
                status = eUserAccountExpired;
            } else if( t.boolval(0, t.colId("pswd_expired")) ) {
                status = eUserPswdExpired;
            } else if( t.uval(0, t.colId("login_failed_count")) >= t.uval(0, t.colId("login_failed_count_max")) ) {
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
        udx userId = t.uval(0, t.colId("userID"));
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
    pForm=0;
    prohibitSelfRegistration = 0;
    checkComplexity = 1;
    m_SuperUserMode = false;
    reset();
    init(usrid);
}

sUsr::sUsr(const char* service_name, bool su_mode)
{
    pForm=0;
    if(strcmp(service_name,"qpride")==0)service_name="queen";

    m_SuperUserMode = false;
    udx srv = 0;
    if( service_name && service_name[0] ) {
        sStr tmp;
        srv = db().uvalue(0, "SELECT userID FROM UPUser WHERE email = '%s' ", db().protect(tmp, service_name));
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
    m_printAutoType=0;
    err.cut0cut(0);
}

bool sUsr::init(udx userId)
{
    if( userId == 0 ) {
        reset();
        return true;
    }
    if( m_Id != userId ) {
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
    /* requires site-specific implementation */
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
        p->Add(m_PrimaryGroup).Add(m_SuperUserMode ? " TRUE " : m_membership.ptr());
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
        pForm->inp("firstName",firstName,0,0);
        pForm->inp("lastName",lastName,0,0);
        pForm->inp("email",email,0,0);
        pForm->inp("BASE_URL",baseURL,0,0);

        sUsr admin("queen");
        admin.pForm = pForm;
        sStr body("%s %s,\n\n"
                "Your account on HIVE was successfully created.\n"
                "Now, in order to verify your email address, please, click the link below:\n"
                "%s?cmd=userV1&emailAct=%s\n"
                "\nHIVE Team.\n", firstName, lastName, baseURL, email);
        sUsrEmail eml(admin, email, "HIVE registration", body.ptr());
        sQPrideBase * qp = QPride();
        if( qp ) {
            sStr adminEmail;
            qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
            if( adminEmail ) {
                eml.addRecipient(sUsrEmail::eCc, adminEmail);
            }
        }
    } else {
        err.printf(0, "Email address %s is not recognized", email);
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
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
        err.printf(0, "Email address %s is not recognized", email);
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
        pForm->inp("firstName",t.val(0, 0),0,0);
        pForm->inp("lastName",t.val(0, 1),0,0);
        pForm->inp("email",t.val(0, 2),0,0);
        pForm->inp("BASE_URL",baseURL,0,0);

        sUsr admin("queen");
        admin.pForm = pForm;
        sStr body("Dear Admin,\n\n"
                "%s %s has applied for access to HIVE from email %s.\n"
                "To activate this account, please, click the link below:\n"
                "%s?cmd=userV3&emailAct=%s\n"
                "\nHIVE Team.\n", t.val(0, 0), t.val(0, 1), t.val(0, 2), baseURL, t.val(0, 2));
        sQPrideBase * qp = QPride();
        sStr adminEmail;
        if( qp ) {
            qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
        }
        if( adminEmail ) {
            sUsrEmail eml(admin, adminEmail, "HIVE account activation request", body.ptr());
        } else {
            err.printf(0, "Admin email address is not configured");
        }
    } else {
        err.printf(0, "Email address %s is not recognized", email);
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
            db().execute("UPDATE UPUser SET is_active_fg = TRUE, is_email_valid_fg = TRUE, loginTm = NULL WHERE email = '%s' AND `type` = 'user'", tmp.ptr());
            t.empty();
            db().getTable(&t, "SELECT is_active_fg, first_name, last_name, email FROM UPUser WHERE email = '%s' AND `type` = 'user'",  tmp.ptr());
            if( t.rows == 1 && t.uval(0, 0) ) {
                pForm->inp("firstName",t.val(0, 1),0,0);
                pForm->inp("lastName",t.val(0, 2),0,0);
                pForm->inp("email",email,0,0);
                pForm->inp("BASE_URL",baseURL,0,0);

                sUsr admin("queen");
                admin.pForm = pForm;
                sStr body("%s %s,\n\n"
                        "Your account on HIVE is now activated.\n"
                        "Please click here to login: %s?cmd=login&login=%s\n"
                        "\nHIVE Team.\n", t.val(0, 1), t.val(0, 2), baseURL, email);
                sUsrEmail eml(admin, t.val(0, 3), "HIVE account activation confirmation", body.ptr());
                sQPrideBase * qp = QPride();
                if( qp ) {
                    sStr adminEmail;
                    qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
                    if( adminEmail ) {
                        eml.addRecipient(sUsrEmail::eCc, adminEmail);
                    }
                }
            } else {
                err.printf(0, "Activation unsuccessful");
            }
        } else {
            err.printf(0, "Was active already");
        }
    } else {
        err.printf(0, "Email address %s is not recognized", email);
        return false;
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
    return true;
}

idx sUsr::groupIdFromPath(const char * groupList, sVec<idx> * groups, const char * mygroup, sVec <idx> * guids)
{
    if(isdigit(groupList[0])) {
        if(groups)sString::scanRangeSet(groupList, 0, groups, 0, 0, 0);
        if(guids)sString::scanRangeSet(groupList, 0, guids, 0, 0, 0);
    } else {
        sStr dst;
        if(!mygroup) {
            dst.printf("'");
            sString::searchAndReplaceStrings(&dst, groupList, 0, " " _ "," __, "','" __ , 0, false);
            dst.shrink00();
            dst.printf("'");
        }
        else  {
            sStr klp;
            sString::searchAndReplaceStrings(&klp, groupList, 0, " " _ "," __, 0 , 0, false);
            for ( const char * k, * k0=klp.ptr(); k0; k0=sString::next00(k0)) {
                sStr mg;mg.addString(mygroup);
                bool found=false;
                for(k=k0;sLen(k)>=3 && memcmp(k,"../",3)==0;){
                    k+=3;
                    char * parent=strrchr(mg.ptr(),'/');
                    if(parent) {
                        *(parent)=0;
                        found=true;
                    }
                    else break;
                }
                if(dst.length())dst.printf(",");
                if(found)dst.printf("'%s/%s'",mg.ptr(0),k);
                else dst.printf("'%s'",k0);
            }
        }

        sVarSet t;
        db().getTable(&t,"SELECT groupID, userID from UPGroup where groupPath like %s" ,dst.ptr());
        for(idx it=0; it<t.rows; ++it) {
            if(groups)groups->vadd(1,(idx)t.uval(it, 0));
            if(guids)guids->vadd(1,(idx)t.uval(it, 1));
        }
    }
    return groups ? groups->dim() : guids->dim();
}

idx sUsr::groupList( sVec<idx> * groups, udx usrID, sVec <idx > * guids)
{
    sVarSet t;
    db().getTable(&t,"SELECT groupID, userID from UPGroup where userID = %" UDEC , usrID ? usrID : Id() );
    for(idx it=0; it<t.rows; ++it) {
        if(groups)groups->vadd(1,(idx)t.uval(it, 0));
        if(guids)guids->vadd(1,(idx)t.uval(it, 1));
    }
    return groups ? groups->dim() : guids->dim();
}


const char * sUsr::groupName(idx groupId, sStr * groupName, bool isGroupUsrID)
{
    sVarSet t;
    static sStr gN;
    if(!groupName)
        groupName=&gN;
    groupName->cut(0);

    if(isGroupUsrID)db().getTable(&t, "SELECT groupPath from UPGroup WHERE userID=%" UDEC,  groupId);
    else db().getTable(&t, "SELECT groupPath from UPGroup WHERE groupID=%" UDEC,  groupId);
    if(t.rows ==1 ) {
        if(t.val(0,0)) groupName->add(t.val(0,0));
    }
    if(groupName->length())
        return groupName->ptr(0);
    return 0;


}

bool sUsr::groupActivate(idx groupId, bool isUsr)
{
    bool res = false;
    sVarSet t;
    if(isUsr){
        db().execute("UPDATE UPGroup SET is_active_fg = TRUE WHERE userID = %" UDEC, groupId);
        audit(eUserAuditAdmin, __func__, "userID='%" UDEC "'; result='%s'", groupId, "ok");
        return true;
    }

    db().getTable(&t, "SELECT is_active_fg FROM UPGroup WHERE groupID = %" UDEC,  groupId);
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) ) {
            db().execute("UPDATE UPGroup SET is_active_fg = TRUE WHERE groupID = %" UDEC, groupId);
            t.empty();
            db().getTable(&t, "SELECT is_active_fg, groupPath FROM UPGroup WHERE groupID = %" UDEC, groupId);
            if( t.rows == 1 && t.uval(0, 0) ) {
                err.printf(0, "Group membership '%s' activation successful", t.val(0, 1));
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

bool sUsr::groupActivate(const char * groupPath)
{
    db().execute("UPDATE UPGroup SET is_active_fg = TRUE WHERE groupPath = '%s'", groupPath);
    audit(eUserAuditAdmin, __func__, "groupPath='%s'; result='%s'", groupPath, "ok");
    return true;
}


bool sUsr::groupCreate(const char* name, const char* abbr, const char* parent, const char * email)
{
    bool res = false;
    if( !name || !name[0] || !abbr || !abbr[0] ) {
        err.printf("Missing group name and/or abbreviation:\nName: '%s'\nAbbreviation: '%s'\n", name, abbr);
    } else if( !parent || !parent[0] || parent[0] != '/' || parent[strlen(parent) - 1] != '/' ) {
        err.printf("Invalid parent group path '%s'.", parent);
    } else {
        if( db().uvalue(0, "SELECT userID FROM UPGroup WHERE groupPath = '%s%s/'", parent, abbr) != 0 ) {
             err.printf("Group with same parent + abbreviation already exists.");
        } else 
        if( updateStart() ) {
            db().execute("INSERT INTO UPUser (first_name, last_name, email, is_active_fg, is_email_valid_fg, pswd, `type`, createTm)"
                    " VALUES ('%s%s/', '%s', '%s', TRUE, TRUE, '--not an account--', 'group', NOW())", parent, abbr, name, email ? email : m_Email.ptr());
            if( !db().HasFailed() ) {
                db().execute("INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath)"
                        " VALUES ((SELECT userID FROM UPUser WHERE first_name = '%s%s/' AND `type` = 'group'), -1, TRUE, '%s%s/')", parent, abbr, parent, abbr);
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

bool sUsr::groupEdit(const char* path, const char* firstName, const char * lastName)
{
    bool res = false, doDelete=false;
    if( !path || !path[0] ) {
        err.printf("Missing group path \n");
    } else {
        if( !firstName && !lastName ) {
            doDelete=true;
        }

        if( db().uvalue(0, "SELECT groupID FROM UPGroup WHERE groupPath = '%s'", path ) == 0 ) {
            err.printf("Group with the path '%s' does not exist.",path);
        } else if( updateStart() ) {
            sStr sql;

            if(doDelete) {
                sql.printf("DELETE from UPUser ");
            } else {
                sql.printf("UPDATE UPUser set ");
                if(firstName) sql.printf("first_name='%s' ",firstName);
                if(lastName) {
                    if(firstName)sql.printf(",");
                    sql.printf("last_name='%s' ",lastName);
                }
            }
            sql.printf("where first_name='%s' and `type` = 'group'",path);
            db().execute(sql.ptr());
            if(doDelete) {
                if( !db().HasFailed() ) {
                    sql.printf(0, "DELETE from UPGroup where groupPath='%s'",path);
                    db().execute(sql.ptr());
                }
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
    audit(eUserAuditAdmin, __func__, "groupPath='%s'; first_name='%s'; last_name='%s'; result='%s'", path, firstName, lastName, res ? "ok" : err.ptr());
    return res;
}

bool sUsr::contact(const char * from_email, const char * subject, const char * body)
{
    err.cut(0);
    if( !from_email || strchr(from_email, '@') == 0 || strchr(from_email, '.') == 0 ) {
        err.printf(0, "Invalid email address!");
    } else {
        sUsr admin("queen");
        sQPrideBase * qp = QPride();
        sStr adminEmail, msg("From: %s\n%s", from_email, (body && body[0]) ? body : "empty message body");
        if( qp ) {
            qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
        }
        if( adminEmail ) {
            sUsrEmail eml(admin, adminEmail, (subject && subject[0]) ? subject : "No subject", msg);
        } else {
            err.printf(0, "Admin email address is not configured");
        }
    }
    audit(eUserAuditAdmin, __func__, "sessionID='%" UDEC "'; user_id=%" UDEC "; from='%s'; result='%s'", m_SID, Id(), from_email, err ? err.ptr() : "ok");
    return err;
}

udx sUsr::addPasswordResetID(const char* email)
{
    udx pswd_reset_id = 0;
    sVarSet t;
    sStr tmp;
    db().getTable(&t, "SELECT is_active_fg, is_email_valid_fg FROM UPUser WHERE email = '%s' AND `type` = 'user'",  db().protect(tmp, email));
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) ) {
            err.printf(0, "Account is disabled.");
        } else if( !t.uval(0, 1) ) {
            err.printf(0, "Email address %s is not verified, try to login again and click on link to resent verification email.", email);
        } else {
            sUsr admin("queen");
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
        err.printf(0, "Email address %s is not recognized.", email);
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
            pForm->inp("firstName",t.val(0, 0),0,0);
            pForm->inp("lastName",t.val(0, 1),0,0);
            pForm->inp("email",t.val(0, 2),0,0);
            pForm->inp("BASE_URL",baseURL,0,0);
            pForm->inp("pswd",t.val(0, 3),0,0);
            pForm->inp("login",t.val(0, 2),0,0);
            pForm->inpv("x","%" DEC ,(idx)(time(0) + 24 * 60 * 60));
            sUsr admin("queen");
            admin.pForm = pForm;
            sStr body("%s %s,\n\n"
                      "To complete your request to reset password click the link below:\n"
                      "%s?cmd=pswdSet&login=%s&pswd=%" UDEC "&x=%" UDEC "\n"
                      "\nHIVE Team.\n", t.val(0, 0), t.val(0, 1), baseURL, t.val(0, 2), t.uval(0, 3), (udx)(time(0) + 24 * 60 * 60));
            sUsrEmail eml(admin, t.val(0, 2), "HIVE password notification", body.ptr());
            sQPrideBase * qp = QPride();
            if( qp ) {
                sStr adminEmail;
                qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
                if( adminEmail ) {
                    eml.addRecipient(sUsrEmail::eCc, adminEmail);
                }
            }

            sVec < idx > groups;sStr gg;
            qp->cfgStr(&gg, 0, "emailWatcher", "9");
            if(gg.length()) {
                groupIdFromPath(gg.ptr(), &groups);
            }
            for ( idx i=0; i<groups.dim(); ++i) {
                idx gidEmailWatcher=groups[i];
                admin.setPermission(gidEmailWatcher, eml.m_id, 3, 2);
            }
        } else {
            err.printf(0, "Unexpected system state, try again later.");
        }
    }
    audit(eUserAuditAdmin, __func__, "email='%s'; result='%s'", email, err ? err.ptr() : "ok");
    return !err;
}

sUsr::ELoginResult sUsr::token(const char * email, sStr & token)
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
            udx r = 0;
            while(r == 0) {
                r = rand();
                db().execute("UPDATE UPUser SET pswd_reset_id = IF(pswd_reset_id = %" UDEC ", NULL, %" UDEC ") WHERE email = '%s' AND `type` = 'user'", r, r, tmp.ptr());
                t.empty();
                db().getTable(&t, "SELECT pswd_reset_id, userID FROM UPUser WHERE email = '%s' AND `type` = 'user'",  tmp.ptr());
                r = (t.rows == 1 && t.cols == 2) ? t.uval(0, 0) : 0;
            }
            const udx s = m_SID, i = m_Id;
            m_SID = r; m_Id = t.uval(0, 1);
            tmp.cut(0);
            encodeSID(tmp, token);
            m_SID = s; m_Id = i;
            res = token ? eUserOperational : eUserNotFound;
        }
    }
    audit(eUserAuditLogin, "piv-auth", "email='%s'; result='%" UDEC "'", email, (udx)res);
    return res;
}

#define PASSWORD_CHECK_SYMB_no

bool sUsr::passwordCheckQuality(const char * mod, const char * mod1)
{
    bool res = false;

    if( !mod ) {
        mod = "\x01";
    }
    if( !mod1 ) {
        mod1 = "\x02";
    }
    idx cAlphaLow = 0, cAlphaCap = 0, cNum = 0, cLen = 0;

#ifdef PASSWORD_CHECK_SYMB
    idx cSymb = 0;
#else
    idx cSymb = -sIdxMax;
#endif

    for(const char * p = mod; *p; ++p) {
        ++cLen;
        if( *p >= 'a' && *p <= 'z' ) {
            ++cAlphaLow;
        } else if( *p >= 'A' && *p <= 'Z' ) {
            ++cAlphaCap;
        } else if( *p >= '0' && *p <= '9' ) {
            ++cNum;
#ifdef PASSWORD_CHECK_SYMB
        } else if( strchr("~`!@#$%^&*()_-+={[}]|\\:;\"\'<,>.?/", *p) ) {
            ++cSymb;
#endif
        }
    }
    if( strcmp(mod, mod1) ) {
        err.printf(0, "new password and confirmation do not match!");
    } else if( checkComplexity && (cLen < 8 || cAlphaLow == 0 || cAlphaCap == 0 || cNum == 0 || cSymb == 0) ) {
        err.printf(0, "new password does not satisfy minimum complexity criteria: more than 8 symbols, with one lower, one caps, one numeric");
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
        const char * firstName, const char * lastName, sVec<idx>& groups, idx softExpiration, idx hardExpiration, const char* baseURL, bool checkPwd)
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
            if(checkPwd){
                if( t.rows != 1 || !checkPassword(t.val(0, t.colId("pswd")), 0, password, email, t.val(0, t.colId("userID")), 0) ) {
                    err.printf(0, "Current password is not recognized!");
                    break;
                }
                
            } 
            userId = t.uval(0, 0);
            if(!userId)
                userId = db().uvalue(0, "SELECT userID FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
                        
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
    return db().svalue(buf, "SELECT GROUP_CONCAT(userID) FROM UPUser WHERE `type` = 'group' AND CONCAT(first_name, '%s') IN (SELECT groupPath FROM UPGroup WHERE userID = %" UDEC " AND is_active_fg IN (%s))",
        m_Email.ptr(), m_Id, inactive ? "TRUE, FALSE" : "TRUE");
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


const char * sUsr::getSingletonVar(const char * type_name, const char * var )
{
    sUsrObjRes usobj;
    udx cnt;
    objs2(type_name, usobj, &cnt);
    if(!cnt){
        return 0;
    }

    const sHiveId * usid=usobj.firstId();
    sUsrObj * obj = objFactory(*usid);
    const char * token=obj->propGet(var);
    if(!token){
        return 0;
    }
    return token;
}

bool sUsr::setSingletonVar(const char * type_name, ... )
{
    sHiveId usid;
    bool oldsu=m_SuperUserMode , su = strcmp(type_name,"user-settings")==0 ? true : false;
    if(su)m_SuperUserMode=true;
    if(objCreate(usid, "user-settings") ) {
        return 0;
    }
    sUsrObj * obj = objFactory(usid);


    {
        va_list ap;
        va_start(ap,type_name);
        for(idx i=0 ; true ; ++i) {
            const char * var = va_arg(ap, const char *);
            if(!var)break;
            const char * val = va_arg(ap, const char *);

            obj->propSet(var, val);
        }
        va_end(ap);
    }

    if(su)m_SuperUserMode=oldsu;
    return true;
}


udx sUsr::getObjOwner(const sHiveId & obj, udx * userId)
{
    sVec < sHiveId > hids;
    new (hids.add(1)) sHiveId(obj.objId(),obj.domainId());
    udx dataOwnerGid=0;

    sVarSet tbl;
    objPermAll(hids, tbl, false);
    for(idx r = 0; r < tbl.rows; ++r) {
        if(atoidx(tbl.val(r,2))==ePermCompleteAccess){
            dataOwnerGid=atoidx(tbl.val(r, 6));
            break;
        }
    }
    if(!dataOwnerGid){
        return 0;
    }

    if(userId){
        sVec<sStr> lst;
        listUsr(&lst, false,true,true);
        sVar gidMap;
        for(idx i = 0; i < lst.dim(); ++i) {
            const char * userName = lst[i].ptr();
            const char * groupPath = sString::next00(userName);
            const char * id = sString::next00(groupPath);
            const char * gid = sString::next00(id);
            const char * email=strrchr(groupPath,'/');
            if(!email)continue;

            if(atoudx(gid)==dataOwnerGid) {
                *userId=atoudx(id);
                break;
            }
        }
    }

    return dataOwnerGid;
}

udx sUsr::getGroupUser(udx groupID)
{
    sVec<sStr> lst;
    listUsr(&lst, false,true,true);
    sVar gidMap;
    for(idx i = 0; i < lst.dim(); ++i) {
        const char * userName = lst[i].ptr();
        const char * groupPath = sString::next00(userName);
        const char * id = sString::next00(groupPath);
        const char * gid = sString::next00(id);
        const char * email=strrchr(groupPath,'/');
        if(!email)continue;

        if(atoudx(gid)==groupID) {
            return atoudx(id);
        }
    }
    return 0;
}




sUsrObj * sUsr::getDomainIdDescr(udx domain_id) const
{
    sUsrObjRes domain_id_descr_res;
    char domain_id_ascii[sizeof(udx) + 1];
    domain_id_ascii[0] = 0;
    if( sHiveId::decodeDomainId(domain_id_ascii, domain_id) ) {
        sStr domain_id_ascii_csv;
        sString::escapeForCSV(domain_id_ascii_csv, domain_id_ascii);
        objs2("^domain-id-descr$+", domain_id_descr_res, 0, "domain-id-descr_ascii", domain_id_ascii_csv.ptr(), "domain-id-descr_ascii", 0, 0, 1);
    }
    sUsrObj * domain_id_descr = domain_id_descr_res.firstId() ? objFactory(*domain_id_descr_res.firstId()) : 0;
    const char * domain_id_descr_domain = domain_id_descr && domain_id_descr->isTypeOf("^domain-id-descr$+") ? domain_id_descr->propGet("domain-id-descr_ascii") : 0;

    if( domain_id_descr && domain_id_descr_domain && strcmp(domain_id_descr_domain, domain_id_ascii) == 0 ) {
        return domain_id_descr;
    } else {
        delete domain_id_descr;
        return 0;
    }
}

sRC sUsr::objCreate(sHiveId & out_id, const char* type_name, const udx in_domainID, const udx in_objID) const
{
    sRC rc;
    out_id.reset();
    if( type_name && type_name[0] ) {
        const sUsrType2 * tp = sUsrType2::ensure(*this, type_name);
        if( tp && !tp->isVirtual() ) {
            if( m_SuperUserMode || m_IsAdmin || !tp->isSystem() ) {
                std::auto_ptr<sSql::sqlProc> p;
                if( !Id() || isGuest() ) {
                    rc.set(sRC::eCreating, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
                } else if( in_domainID && !in_objID ) {
                    std::auto_ptr<sUsrObj> domain_id_descr(getDomainIdDescr(in_domainID));
                    if( domain_id_descr.get() && isAllowed(domain_id_descr->Id(), ePermCanExecute) ) {
                        p.reset(getProc("sp_obj_create_new_in_domain"));
                        p->Add(tp->id().domainId()).Add(tp->id().objId()).Add(in_domainID).Add((udx) ePermCompleteAccess).Add((udx) eFlagDefault).
                            Add(domain_id_descr->Id().domainId()).Add(domain_id_descr->Id().objId()).Add((udx) ePermCanExecute);
                    } else {
                        rc.set(sRC::eCreating, sRC::eObject, sRC::eDomain, sRC::eNotAuthorized);
                    }
                } else {
                    p.reset(getProc("sp_obj_create_v2"));
                    p->Add(tp->id().domainId()).Add(tp->id().objId()).Add(in_domainID).Add(in_objID).Add((udx) ePermCompleteAccess).Add((udx) eFlagDefault);
                }
                sVarSet tbl;
                if( p.get() && p->getTable(&tbl) && tbl.rows == 1 ) {
                    out_id.set(tbl.uval(0, 0), tbl.uval(0, 1), 0);
                    const sUsrType2 * type_type = sUsrType2::ensureTypeType(*this);
                    if( type_type && tp->id() == type_type->id() ) {
                        udx system_grp_id = getGroupId("/system/");
                        udx everyone_grp_id = getGroupId("/everyone/");
                        if( !system_grp_id || !out_id || !setPermission(system_grp_id, out_id, ePermCompleteAccess, eFlagInheritDown) ) {
                            rc.set(sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                            out_id.reset();
                        }
                        if( in_domainID == type_type->id().domainId() && (!everyone_grp_id || !out_id || !setPermission(everyone_grp_id, out_id, ePermCanBrowse | ePermCanRead, eFlagInheritDown) ) ) {
                            rc.set(sRC::eSetting, sRC::ePermission, sRC::eOperation, sRC::eFailed);
                            out_id.reset();
                        }
                    } else if( tp->nameMatch("HIVE_Development_Timelog") ) {
                        if( udx hive_grp_id = getGroupId("/Projects/HIVE/") ) {
                            setPermission(hive_grp_id, out_id, ePermCanBrowse | ePermCanRead, eFlagInheritDown);
                        }
                    }
                }
                if( out_id ) {
                    sUsrObj * o = objFactory(out_id);
                    if( o ) {
                        sStr hack;
                        o->addFilePathname(hack, true, ".deleteme");
                        delete o;
                    }
                } else {
                    if( db().HasFailed() ) {
                        fprintf(stderr, "objCreate() DB error %" UDEC ": %s\n", db().Get_errno(), db().Get_error().ptr());
                        if( QPride() ) {
                            QPride()->logOut(sQPrideBase::eQPLogType_Error, "objCreate() DB error %" UDEC ": %s", db().Get_errno(), db().Get_error().ptr());
                        }
                    }
                    if( !rc ) {
                        rc.set(sRC::eCreating, sRC::eObject, sRC::eOperation, sRC::eFailed);
                    }
                }
            } else {
                rc.set(sRC::eCreating, sRC::eObject, sRC::eUser, sRC::eNotAuthorized);
            }
        } else {
            rc.set(sRC::eCreating, sRC::eObject, sRC::eType, sRC::eInvalid);
        }
    } else {
        rc.set(sRC::eCreating, sRC::eObject, sRC::eType, sRC::eEmpty);
    }
    if( rc.isSet() || !objGet(out_id) ) {
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

#define TYPE_TYPE_PERMS ePermCanRead | ePermCanBrowse

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
            std::auto_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_all_v2"));
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
    /* requires site-specific implementation */
            p = op->allow & ~op->deny;
    /* requires site-specific implementation */
    }
    return p;
}

sHiveId sUsr::propExport(sVec<sHiveId>& ids, sVarSet & v, sUsr::EPermExport permissions) const
{
    for(idx i = 0; i < ids.dim(); ++i) {
        std::auto_ptr<sUsrObj> obj(objFactory(ids[i]));
        if( !obj.get() ) {
            return ids[i];
        }
        const idx rows1 = v.rows;
        obj->propBulk(v);
        if( v.rows > rows1 ) {
            v.addRow().addCol(ids[i]).addCol("_type").addCol((const char*)0).addCol(obj->getTypeName());
        }
    }
    if( permissions ) {
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

sHiveId sUsr::propExport(sVec<sHiveId>& ids, sJSONPrinter & printer, sUsr::EPermExport permissions, bool flatten, bool upsert, const char * upsert_qry, const char * prop_filter00, const char * prop_exclude00) const
{
    sStr sid;
    sHiveId::printVec(sid, ids, ",");
    sUsrObjRes res;
    objs2("*", res, 0, "_id", sid.ptr());

    sVarSet perm_tbl;
    if( permissions ) {
        objPermAll(ids, perm_tbl, true);
    }

    printer.startObject();
    for(idx i = 0; i < ids.dim(); ++i) {
        res.empty();
        if( !objs2("*", res, 0, "_id", ids[i].print()) ) {
            return ids[i];
        }

        printer.addKey(ids[i].print());
        printer.startObject();

        res.json(*this, res.first(), printer, true, flatten, upsert, upsert_qry, prop_filter00, prop_exclude00);

        bool have_grp_perm = false;
        for(idx ir = 0; ir < perm_tbl.rows; ir++) {
            sHiveId perm_id(perm_tbl.uval(ir, 0), perm_tbl.uval(ir, 1), 0);
            if( perm_id == ids[i] ) {
                const char * grp = perm_tbl.val(ir, 6);
                if( grp && (grp[sLen(grp) - 1] == '/' || permissions == ePermExportAll) ) {
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
        std::auto_ptr<sUsrObj> obj(objFactory(ids[i]));
        if( !obj.get() ) {
            return false;
        }
        sStr fileList00;
        if( mask && mask[0] ) {
            obj->files2(fileList00, sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs) | sFlag(sDir::bitRecursive), mask);
        } else {
            sDir xd;
            obj->files(xd, sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs));
            for(idx i = 0; i < xd.dimEntries(); i++) {
                fileList00.printf("%s", xd.getEntryAbsPath(i));
                fileList00.add0();
            }
            fileList00.add0(2);
        }
        idx flcnt = 0;
        sStr fldir("%s/%s/", dstdir, obj->Id().print());
        sDir::removeDir(fldir);
        for(const char * src = fileList00; src; src = sString::next00(src) ) {
            p.cut(0);
            p.makeName(src, "%%flnm");
            if( !sDir::exists(fldir) ) {
                sDir::makeDir(fldir);
            }
            dst.cut(0);
            dst.makeName(fldir, "%%dir/%s", p.ptr());
            sFile::remove(dst);
            if( sFile::symlink(src, dst) ) {
                v.addRow().addCol(obj->IdStr()).addCol("_file").addCol(++flcnt).printCol("%s/%s", obj->IdStr(), p.ptr());
            }
        }
    }
    return true;
}

sRC sUsr::objHivepack(sVec<sHiveId>& ids, const char * dstName, sPipe::callbackFuncType callback, void * callbackParam) const
{
    sStr dst("%s.tmp/", dstName ? dstName : "");
    sStr hpack("%s%s", strchr("/\\", dst[0]) ? "" : "../", dstName);
    sRC rc;
    sVarSet v;
    sHiveId oerr = propExport(ids, v, ePermExportGroups);
    if( oerr ) {
        rc = sRC(sRC::eWriting, sRC::eProperty, sRC::eObject, sRC::eNotFound);
    } else {
        if( !sDir::exists(dst) && !sDir::makeDir(dst) ) {
            rc = sRC(sRC::eCreating, sRC::eDirectory, sRC::eOperation, sRC::eFailed);
        } else {
            if( !objFilesExport(ids, v, dst) ) {
                rc = sRC(sRC::eWriting, sRC::eFile, sRC::eObject, sRC::eNotFound);
            } else {
                sVec<idx> new_order(sMex::fSetZero);
                bool isOrdered = true;
                for(idx r = 0; r < v.rows; ++r) {
                    sHiveId pvl(v.val(r, 3, 0));
                    sHiveId prop_oid(v.val(r, 0, 0));
                    std::auto_ptr<sUsrObj> obj(objFactory(prop_oid));
                    if( !obj.get() ) {
                        rc = sRC(sRC::eAccessing, sRC::eObject, sRC::eObject, sRC::eNotFound);
                        break;
                    }
                    const char * prop_name = v.val(r, 1, 0);
                    const sUsrTypeField * tp = obj->propGetTypeField(prop_name);
                    if( tp && (tp->isVirtual() && tp->name()[0] != '_') ) {
                        isOrdered = false;
                        continue;
                    }
                    new_order.vadd(1, r);
                    idx j = 0;
                    while( j < ids.dim() ) {
                        if( ids[j++] == pvl ) {
                            --j;
                            break;
                        }
                    }
                    if( j < ids.dim() ) {
                        if( tp && tp->type() == sUsrTypeField::eObj ) {
                            sStr linkvalue("${src-%s}", pvl.print());
                            v.updateVal(r, 3, linkvalue.ptr());
                        }
                    }
                }
                if( !rc ) {
                    if( !isOrdered ) {
                        v.reorderRows(new_order.ptr(), 0, new_order.dim(), true);
                    }
                    {{
                        sStr prop("%sobjects.prop", dst.ptr());
                        if( !sFile::exists(prop) || sFile::remove(prop) ) {
                            v.addRow().addCol((udx) 0).addCol("_comment").addCol((const char*) 0).printCol("%" DEC " objects exported on %s", ids.dim(), QPride() ? *(QPride()->vars["thisHostName"]) : "--");
                            sStr out, out2;
                            v.printProp(out);
                            sString::searchAndReplaceStrings(&out2, out, out.length(), "\nprop." __, "\nprop.src-" __, 0, false);
                            sFil ofil(prop);
                            if( ofil.ok() ) {
                                ofil.printf("%s", out2.ptr());
                            } else {
                                rc = sRC(sRC::eWriting, sRC::eFile, sRC::eOperation, sRC::eFailed);
                            }
                        } else {
                            rc = sRC(sRC::eRemoving, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                    }}
                    if( !rc ) {
                        sStr tmp("cd \"%s\" && zip -mrp \"%s\" *", dst.ptr(), hpack.ptr());
                        sFilePath cbpath(hpack, "%%dir");
                        const idx res = sPipe::exeFS(0, tmp, 0, callback, callbackParam, cbpath.ptr());
                        if( res != 0 ) {
                            rc = sRC(sRC::eExecuting, sRC::eCommandLine, sRC::eOperation, res < 0 ? sRC::eInterrupted : sRC::eFailed);
                        }
                    }
                }
            }
        }
    }
    sDir::removeDir(dst);
    return rc;
}

void sUsr::permPrettyScanf(const char * group, const char * view, const char* sperm, const char* sflags, const sUsrType2 * type, udx * groupId, sHiveId * viewId, udx * perm, udx * flags) const
{
    if( (group && group[0] && groupId) || (view && view[0] && type && viewId) ) {
        std::auto_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_scanf"));
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

bool sUsr::setPermission(udx groupId, const sHiveId & objHiveId, udx permission, udx flags, sHiveId * viewId, const char * forObjID) const
{
    bool ok = m_SuperUserMode, admin = false, share = false;
    if( !ok ) {
        admin = isAllowed(objHiveId, ePermCanAdmin);
        share = isAllowed(objHiveId, ePermCanShare);
        ok = admin | share;
    }
    if( ok ) {
        std::auto_ptr<sUsrObj> obj(objFactory(objHiveId));
        if( obj.get() ) {
            const char * tpnm = obj->getTypeName();
            if( !m_SuperUserMode && groupId != m_PrimaryGroup && tpnm && (strcasecmp(tpnm, "folder") == 0 || strcasecmp(tpnm, "sysfolder") == 0 ) ) {
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
            if( ok ) {
                std::auto_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_set_v2"));
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
        std::auto_ptr<sSql::sqlProc> p(getProc("sp_obj_perm_copy_v2"));
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
        std::auto_ptr<sUsrQueryEngine> qengine;
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
            } else if( val_len==0 ) {
                if( del ) {
                    del.shrink00();
                    del.add0();
                }
                del.printf("%.*s", (int)pnm_len, pnm);
                del.add0(2);

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
                    std::auto_ptr<sUsrObj> obj(ctx.user.objFactory(pobj->id));
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
                        sUsrObj * obj = ctx.user.objFactory(pobj->id);
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
public:
        sDic<TPropObj> all;
        TPropCtx ctx;
};

bool sUsr::propSet(const char * propFileName, sStr& log, sVec<sHiveId>* new_ids, sVec<sHiveId>* updated_ids, sDic<sHiveId> * new_ids_map)
{
    sFil propFile(propFileName, sMex::fReadonly);
    bool ret = false;
    if( !propFile.ok() ) {
        log.printf("err.null._err=prop file not found '%s'", propFileName);
    } else {
        sFilePath dir(propFileName, "%%dir"), curr;
        if( dir ) {
            curr.curDir();
            sDir::chDir(dir);
        }
        ret = propFile.length() ? propSet(propFile.ptr(), propFile.length(), log, new_ids, updated_ids, new_ids_map) : true;
        if( dir && curr ) {
            sDir::chDir(curr);
        }
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

static bool propFormParse(sVar & form, TProp & prop, bool not_for_db)
{

    bool isok = true;
    sStr d;
    for(idx k = 0; k < form.dim(); ++k) {
        idx nlen;const char * nm = (const char*) form.id(k,&nlen);
        idx vlen;const char * val = (const char*) form.value(nm,0,&vlen);
        isok &= prop.parse(nm, sLen(nm), val, sLen(val), not_for_db);

    }
    return isok;
}

bool sUsr::propSet(sVar & form, sStr & log, sVarSet & result, bool not_for_db) const
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

bool sUsr::propSetJson(sJson * jsonObj, const char * jsonText, sStr * log)
{
    sStr jsonTextBuf("[");
    if(jsonObj)jsonObj->print("$root",&jsonTextBuf,0);
    else if (jsonText) jsonTextBuf.printf("%s",jsonText);
    jsonTextBuf.printf("]");

    sVar form;form.inp("_json",jsonTextBuf);
    if(log)log->cut(0);
    sStr Log;
    bool ret=propSet(form,log ? *log : Log);


    return ret;

}

bool sUsr::propSet(sVar & form, sStr & log, sVec<sHiveId>* new_ids, sVec<sHiveId>* updated_ids, sDic<sHiveId> * new_ids_map) const
{
    const char * pJson=form.value("_json");
    if(pJson) {
        sUsrPropSet upropset(*this);
        upropset.pretendForm(&form, pJson);
        upropset.getErr(&log);
    }

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

udx sUsr::objsLowLevel(const char * type_names, const char * obj_filter_sql, const char * prop_filter_sql, const char * prop_name_csv, bool permissions, const udx start, const udx count, sUsrObjRes * res, udx * total_qty, bool allowSysInternal, const char * roles) const
{
    if( !res && !total_qty ) {
        return 0;
    }
    SRCHDBG("SEARCH QUERY %s%s FROM type(s): '%s' WHERE [[%s]] AND [[%s]] LIMIT %" UDEC ", %" UDEC " with%s total\n", prop_name_csv ? prop_name_csv : "NULL", permissions ? " +flag:_perm" : "", type_names, obj_filter_sql ? obj_filter_sql : "", prop_filter_sql ? prop_filter_sql : "", start, count, total_qty ? "" : "out");
    std::auto_ptr<sSql::sqlProc> p(getProc("sp_obj_get_v4_1"));
    if( total_qty ) {
        *total_qty = 0;
    }
    if( p.get() ) {
        sStr filter00, typeids, roles00;
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
        if(roles) {
            sString::searchAndReplaceSymbols(&roles00, roles, 0, ",", 0, 0, true, true, true, true);
            filter00.cut(0);
        }

        p->Add(typeids.length() ? typeids.ptr() : "").Add(obj_filter_sql).Add(prop_filter_sql);
        if( m_AllowExpiredObjects ) {
            if( m_SuperUserMode ) {
                p->Add((idx)-1);
            } else {
#if _DEBUG
                fprintf(stderr, "WARNING: sUsr API misuse: fetching expired objects is permitted only in superuser mode\n");
#endif
                p->Add((idx)0);
            }
        } else {
            p->Add((idx)0);
        }
        p->Add(start).Add(count).Add(total_qty != 0).Add(permissions);
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
                    if( (filter00 || roles00) && m_ObjPermission.get() ) {
                        for(sUsrObjRes::IdIter it = res->first(); res->has(it); res->next(it)) {
                            sObjPerm * op = m_ObjPermission->get(res->id(it), sizeof(sHiveId));
                            if(op){
                                if( const sUsrType2 * utype = sUsrType2::ensure(*this, op->type) ) {
                                    fields.cut(0);

                                    utype->findFields(*this, fields, filter00);
                                    for(idx k = 0; k < fields.dim(); ++k) {

                                        if(roles00) {
                                            const char * fld_rlnm =fields[k]->roleName();
                                            if(!fld_rlnm )fld_rlnm ="null";
                                            if( sString::compareChoice(fld_rlnm ,roles00,0,false,0,true)==-1) {
                                                continue;
                                            }
                                            filter00.addString(fields[k]->name());
                                            filter00.add0(1);
                                        }
                                        props.setString(fields[k]->name());
                                    }

                                }
                            }
                        }
                    }
                    if(roles) {
                        if(!filter00.length()) {
                            filter00.printf(".");filter00.add0(1);
                        }
                        filter00.add0(1);

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
                        std::auto_ptr<sUsrObj> obj(objFactory(*id));
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

udx sUsr::objs2(const char* type_names, sUsrObjRes & res, udx * total_qty, const char* prop, const char* value, const char * prop_name_csv, bool permissions, const udx start, const udx count, bool allowSysInternal, const char * roles) const
{
    SRCHDBG("SEARCH QUERY type(s): '%s' --> '%s'[%s]\n", type_names, value ? value : "", prop ? prop : "");
    sStr v00(sMex::fExactSize), p00(sMex::fExactSize), v_flt(sMex::fExactSize);
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
            } else {

                if( v_flt ) {
                    if( pn[0]=='&' ){v_flt.addString(" AND ");++pn;}
                    else v_flt.addString(" OR ");
                }
                v_flt.printf("(f.name %s '", not_pn ? "<>" : "=");
                db().protect(v_flt, pn);
                v_flt.shrink00();
                v_flt.printf("'");
                if( pv && pv[0] ) {
                    if(pv[0]=='>' || pv[0]=='<' )v_flt.printf(" AND f.value ");
                    else v_flt.printf(" AND f.value %s" OBJCMP " '", not_pv ? OBJNOT : "");
                    db().protect(v_flt, pv);
                    v_flt.shrink00();
                    if(pv[0]!='>' && pv[0]!='<' )v_flt.printf("'");
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
            v_flt.printf("') OR (CHAR(o.domainID USING ASCII) %s" OBJCMP " '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("') OR (o.objID %s" OBJCMP " '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("'))");
        }
    }
    sStr o_flt;
    if( id_incl ) {
        o_flt.printf("(");
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
    return objsLowLevel(type_names, o_flt, v_flt.ptr(0), prop_name_csv, permissions, start, count, &res, total_qty, allowSysInternal,roles);
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
                std::auto_ptr<sUsrObj> obj(objFactory(ids[i]));
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
        std::auto_ptr<sSql::sqlProc> p(getProc("sp_obj_prop_list_v2"));
        p->Add(idcsv.ptr(1)).Add(prp);
        sVarSet vtmp;
        p->getTable(&vtmp);
        for(idx i = 0; i < ids.dim(); ++i) {
            std::auto_ptr<sUsrObj> obj(objFactory(ids[i]));
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
        removeTrash(out);
        list.mex()->flags |= sMex::fSetZero;
        for(sUsrObjRes::IdIter it = out.first(); out.has(it); out.next(it)) {
            const sHiveId * id = out.id(it);
            if( id ) {
                std::auto_ptr<sUsrObj> obj(objFactory(*id));
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

udx sUsr::removeTrash(sUsrObjRes & res, bool return_total_count) const
{
    static sDic<idx> inTrashObjects;
    if( !inTrashObjects.dim() ) {
        sUsrObjRes trash;
        objs2("sysfolder", trash, 0, "name", "Trash", "child");
        if( trash.dim() ) {
            const sUsrObjRes::TObjProp * obj = trash.getFirst();
            const sUsrObjRes::TPropTbl * tbl = trash.get(*obj, "child");
            sVec<sHiveId> ids, out;
            while( tbl ) {
                const char * s = trash.getValue(tbl);
                if( s ) {
                    sHiveId id(s), * idp = 0;
                    if( id ) {
                        idp = ids.add(1);
                        if( idp ) {
                            *idp = id;
                        }
                    }
                }
                tbl = trash.getNext(tbl);
            }
            objs(ids, out);
            for(idx i = 0; i < out.dim(); ++i) {
                inTrashObjects.set(out.ptr(i), sizeof(sHiveId));
            }
        }
    }
    udx total_qty = 0;
    if( inTrashObjects.dim() ) {
        for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
            const sHiveId * id = res.id(it);
            if( inTrashObjects.find(id, sizeof(*id)) ) {
                res.del(it);
                ++total_qty;
            }
        }
    }
    return return_total_count ? inTrashObjects.dim() : total_qty;
}

sUsrObj* sUsr::objFactory(const sHiveId & id) const
{
    std::auto_ptr<sUsrObj> obj;
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
    std::auto_ptr<sUsrObj> obj;
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
    sStr sql("SELECT IF(`type` = 'group', last_name, CONCAT(first_name, ' ', last_name)), groupPath, u.userID, groupID "
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
        sql.printf(" AND (u.first_name LIKE '%s' OR u.last_name LIKE '%s' OR u.email LIKE '%s')", tmp.ptr(), tmp.ptr(),tmp.ptr());
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

void sUsrObjRes::dcsv(const sUsrObjRes::IdIter & it, sStr & buf, sDic < idx > * propDic) const
{
    const sHiveId * oid = id(it);
    if( !oid ) return ;
    idx sz,ss;
    const TObjProp * prop = get(it);

    oid->print(buf);

    for(idx idd=0; idd< propDic->dim(); ++idd) {
        const char * pid=(const char *) propDic->id(idd,&sz);

        buf.printf(",");

        for(idx p = 0; prop && p < prop->dim(); ++p) {
            const char * prop_name = (const char *) prop->id(p,&ss);

            if(ss!=sz || memcmp(prop_name,pid,sz)!=0)
                continue;

            const TPropTbl * tbl = get(*prop, prop_name);
            buf.printf("\"");
            idx some=0;
            while( tbl ) {
                if(some)buf.printf(";");
                sString::escapeForCSV(buf, getValue(tbl));
                tbl = getNext(tbl);
                ++some;
            }
            buf.printf("\"");
        }


    }
    buf.printf("\n");
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
    sStr t;
    if( oid ) {
        const sUsrType2 * utype = user.objType(*oid);
        const char * tpnm=utype->name();

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

            if(user.m_printAutoType ) {
                const TPropTbl * tbl = get(*prop, prop_name);
                const sUsrTypeField * fld = utype->getField(user, prop_name);
                if(fld){
                    while( tbl ) {
                        const char * gv=getValue(tbl);
                        sHiveId tid(gv);
                        const sUsrType2 * tp= (fld->type()==sUsrTypeField ::eObj) ? sUsrType2::get(tid) : 0;
                        if ( tp){
                            const char * pp=tp->name();
                            t.printf(0,"$type.%s",pp);
                            tree_table.addRow().addCol(oid->print()).addCol(prop_name).addCol(getPath(tbl)).addCol(t.ptr(0));
                        }
                        else tree_table.addRow().addCol(oid->print()).addCol(prop_name).addCol(getPath(tbl)).addCol(gv);
                        tbl = getNext(tbl);
                    }
                }
            }
            else if( prop_name && strcmp(prop_name, "_perm") == 0 ) {
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
            if(user.m_printAutoType && (!(oid->domainId())) && sLen(tpnm)==4 && strncmp(tpnm,"type",4)==0) {
                const TPropTbl * tbl = get(*prop, "name");
                const char * pp=this->getValue(tbl);
                t.printf(0,"$type.%s",pp);
                printer.addValue(t.ptr(0));
            }
            else if( upsert ) {
                if( upsert_qry && upsert_qry[0] ) {
                    upsert_qry_buf.printf(0, "$upsert_qry(%s)", upsert_qry);
                    printer.addValue(upsert_qry_buf.ptr());
                } else {
                    printer.addValue("$upsert()");
                }
            } else {
                printer.addValue(*oid);
            }
            if( utype ) {
                printer.addKey("_type");
                printer.addValue(tpnm);
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
        TPropTbl * vnew = (TPropTbl *) _buf.mex()->ptr(vnew_offset);
        vnew->next = 0;
        idx vp, vv;
        sStr t(sMex::fExactSize);
        if( path && path_len ) {
            t.add(path, path_len);
        }
        t.add0(2);
        idx * q = _buf.setString(t, path_len, &vp);
        if( q ) {
            *q = *q + 1;
            t.cut0cut(0);
            if( value && value_len ) {
                t.add(value, value_len);
            }
            t.add0(2);
            q = _buf.setString(t, value_len, &vv);
            if( q ) {
                *q = *q + 1;
                vnew = (TPropTbl *) _buf.mex()->ptr(vnew_offset);
                vnew->path = vp;
                vnew->value = vv;
                const idx nmlen = sLen(prop) + 1;
                idx * tail = obj.get(prop, nmlen);
                if( tail ) {
                    do {
                        TPropTbl * tbl = (TPropTbl *) _buf.mex()->ptr(*tail);
                        tail = &tbl->next;
                    } while( *tail > 0);
                } else {
                    tail = obj.set(prop, nmlen);
                }
                if( tail ) {
                    *tail = vnew_offset;
                    return true;
                }
            }
        }
    }
    return false;
}

idx sUsr::usrExpand(const char * hugoObjList, sVar * pForm, sVec <sHiveId> & newObjList) {
    sUsrObjRes obj_res; idx cnt;
    sVec <sHiveId> hugoListIdx;
    sUsr * curUsr = this;
    if (!hugoObjList) {
        curUsr->objs2("^hugo$", obj_res,(udx*)&cnt);
        for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
            *hugoListIdx.add() = *obj_res.id(it);
        }
    }else {
        sHiveId::parseRangeSet(hugoListIdx, hugoObjList);
    }

    sVec <sUsrObj> hugoObjVec;
    for (idx iPVec=0; iPVec < hugoListIdx.dim(); ++iPVec) {
        sUsrObj * uObj = hugoObjVec.add();
        new (uObj) sUsrObj (*curUsr,hugoListIdx[iPVec]);
    }

    sVarSet curUserGroupList;
    curUsr->listUserGroups(curUserGroupList,curUsr->Id(),false);

    for (idx ihugo=0; ihugo < hugoObjVec.dim(); ++ihugo) {
        const sUsrObjPropsTree * objPropsTree=hugoObjVec[ihugo].propsTree();
        const sUsrObjPropsNode * groupList= objPropsTree->find("groups");

        for(const sUsrObjPropsNode * groupRow = groupList->firstChild(); groupRow; groupRow = groupRow->nextSibling()) {
            const sUsrObjPropsNode * group_name = groupRow->find("group_name");
            const sUsrObjPropsNode * formList = groupRow->find("forms");

            bool okGroup=true;
            sStr myGroupbuf;
            sString::searchAndReplaceSymbols(&myGroupbuf,group_name->value(),0,";",0,0,true,false,true,0);

            for( const char * pGroup=myGroupbuf.ptr();pGroup;pGroup=sString::next00(pGroup)){
                okGroup=false;
                for (idx iig=0; iig < curUserGroupList.rows; ++iig) {
                    idx size; const char * myGroup = curUserGroupList.val(iig,4,&size);
                    if( size >6 && memcmp(pGroup,"regex:",6)==0) {
                        regex_t re;
                        if( regcomp(&re, pGroup+6, REG_EXTENDED | REG_ICASE) == 0 ) {
                            if( regexec(&re, myGroup, 0, NULL, 0) == 0 ) {
                                okGroup = true;
                                regfree(&re);
                                break;
                            }
                            regfree(&re);
                        }
                    }
                    else if (strstr(myGroup,pGroup)){
                        okGroup=true;
                        break;
                    }
                }
            }

            if (!okGroup) continue;

            for(const sUsrObjPropsNode * formRow = formList->firstChild(); formRow; formRow = formRow->nextSibling()) {
                const sUsrObjPropsNode * form_name = formRow->find("form_name");
                const sUsrObjPropsNode * var_groups = formRow->find("var_groups");

                const char * formName = form_name->value();
                if (!formName)
                    continue;

                sHiveId myFormObj;
                curUsr->objCreate(myFormObj, formName);

                if (!myFormObj.valid())
                    continue;

                sUsrObj obj(*curUsr,myFormObj);

                for(const sUsrObjPropsNode * varRow = var_groups->firstChild(); varRow; varRow = varRow->nextSibling()) {
                    const sUsrObjPropsNode * var_from = varRow->find("var_from");
                    const sUsrObjPropsNode * var_to = varRow->find("var_to");

                    const char * arg_name = var_from->value();
                    const char * field_name = var_to->value();
                    const char * arg_val = (arg_name[0]=='$') ? pForm->value(arg_name+1,0) : arg_name ;
                    if (!arg_val || !*arg_val)
                      continue;

                    obj.propSet(field_name,arg_val);
                }
                *newObjList.add()= myFormObj;
            }
        }

    }

    return 1;
}

idx sUsr::permPropagate(const char * objIdList,const char * permObjIdList, const char * objTypeID,const char * sharerOverride) {
    sUsrObjRes obj_res; idx cnt;
    sVec <sHiveId> ppoListIdx;
    if (!permObjIdList) {
        objs2("^hppo$", obj_res,(udx*)&cnt);
        for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
            *ppoListIdx.add() = *obj_res.id(it);
        }
    }
    else {
        sHiveId::parseRangeSet(ppoListIdx, permObjIdList);
    }
    
    sVec <sUsrObj> ppoObjVec;
    for (idx iPVec=0; iPVec < ppoListIdx.dim(); ++iPVec) {
        sUsrObj * uObj = ppoObjVec.add();
        new (uObj) sUsrObj (*this,ppoListIdx[iPVec]);
    }

    sVec <idx> objListIdx;
    sUsrObjRes objLLL_res;
    if(!objIdList) {
        objs2(objTypeID ? objTypeID : 0, objLLL_res,(udx*)&cnt);
        for(sUsrObjRes::IdIter it = objLLL_res.first(); objLLL_res.has(it); objLLL_res.next(it)) {
            *objListIdx.add() = objLLL_res.id(it)->objId();
        }
    } else {
        sString::scanRangeSet(objIdList,0,&objListIdx,0,0,0);
    }
            
    for(idx iObj=0; iObj < objListIdx.dim(); ++iObj){
        const sHiveId objId(objListIdx[iObj],0);
        sUsrObj myObj(*this,objId); 
        const char * mytype=myObj.getTypeName();
        if (!mytype)
          continue; 
        for (idx ippo=0; ippo < ppoObjVec.dim(); ++ippo) {
            
            const sUsrObjPropsTree * objPropsTree=ppoObjVec[ippo].propsTree();
            const sUsrObjPropsNode * ruleList= objPropsTree->find("rule") ;
            
            for(const sUsrObjPropsNode * ruleRow = ruleList->firstChild(); ruleRow; ruleRow = ruleRow->nextSibling()) {
                const sUsrObjPropsNode * ruleTypes = ruleRow->find("types");
                const sUsrObjPropsNode * ruleGroups = ruleRow->find("groups");
                const sUsrObjPropsNode * ruleFlags = ruleRow->find("flags");
                const sUsrObjPropsNode * ruleBits = ruleRow->find("bits");
                const sUsrObjPropsNode * ruleSharer = ruleRow->find("sharer");

                bool okGroup=true;
                const char * groupSelected=0;
                sVarSet sharerList;
                sStr myGroupbuf;
                if(ruleSharer){
                    

                    listUserGroups(sharerList,Id(),false);
                    
                    const char * realSharer=sharerOverride ? sharerOverride : ruleSharer->value();

                    sString::searchAndReplaceSymbols(&myGroupbuf,realSharer,0,";",0,0,true,false,true,0);

                    for( const char * pGroup=myGroupbuf.ptr();pGroup;pGroup=sString::next00(pGroup)){
                        okGroup=false;
                        for (idx iig=0; iig < sharerList.rows; ++iig) {
                            idx size;
                            const char * myGroup = sharerList.val(iig,4,&size);
                            if( size >6 && memcmp(pGroup,"regex:",6)==0) {
                                regex_t re;
                                if( regcomp(&re, pGroup+6, REG_EXTENDED | REG_ICASE) == 0 ) {
                                    if( regexec(&re, myGroup, 0, NULL, 0) == 0 ) {
                                        okGroup = true;
                                        regfree(&re);
                                        groupSelected=myGroup;
                                        break;
                                    }
                                    regfree(&re);
                                }
                            } else if(strcmp(myGroup,pGroup)==0){
                                okGroup=true;
                                groupSelected=myGroup;
                                break;
                            }
                        }
                        if(okGroup == true)
                            break;
                    }
                }
                if(!okGroup)continue;
            
                bool okrule=false;
                sStr tbuf;
                sString::searchAndReplaceSymbols(&tbuf,ruleTypes->value(),0,";",0,0,true,false,true,0);
                for( const char * ptype=tbuf.ptr();ptype;ptype=sString::next00(ptype)){
                    if(memcmp(ptype,"regex:",6)==0) { 
                        regex_t reg;
                        if( regcomp(&reg, ptype+6, REG_EXTENDED | REG_ICASE) == 0 ) {
                            if( regexec(&reg, mytype, 0, NULL, 0) == 0 ) {
                                okrule = true;
                                regfree(&reg);
                                break;
                            }
                            regfree(&reg);
                        }
                    } else if(strcmp(ptype,mytype)==0){
                        okrule=true;
                        break;
                    }
                }

                if(!okrule)continue;


                sVec <idx > groupList;
                sStr gbuf;

                sString::searchAndReplaceSymbols(&gbuf,ruleGroups->value(),0,";",0,0,true,false,true,0);
                for( const char * pgroup= gbuf.ptr();pgroup;pgroup=sString::next00(pgroup)){
                    sVec<idx> gs;
                    if ( groupIdFromPath(pgroup, &gs,groupSelected) ) {
                        groupList.vadd(1,gs[0]) ;
                    }   
                }

                for( idx ig=0;ig<groupList.dim(); ++ig) {
                    bool ok = updateStart();
                    ok &= setPermission(groupList[ig], objId, ruleBits->uvalue(), ruleFlags->uvalue(),0);
                    if( ok ) {
                        ok = updateComplete();
                    } else {
                        updateAbandon();
                    }
                }
            }
        }
    }
    return 0;
}

const char * sUsr::findUniqueObject(sUsrObj * objDst, const char * objType, const char * uniqueVar,const char * uniqueVal)
{
    sUsrObjRes obj_res;
    sStrT t;t.printf("^%s$",objType);

    if( !objs2(t.ptr(0), obj_res,(udx*)0,uniqueVar,uniqueVal) )
        return 0;
    new (objDst) sUsrObj(*this,*obj_res.id(obj_res.first()));
    return objDst->IdStr();
}
const char * sUsr::ensureUniqueObjectProvided(sUsrObj * objSrc, const char * objType, const char * uniqueVar,const char * uniqueVal, const char * typeSearch)
{
    if(!typeSearch)typeSearch=objType;
    const char * objS=objSrc->propGet(objType);
    const char * val=0;
    if(!objS) {
        sUsrObj od;
        val=findUniqueObject(&od, typeSearch, uniqueVar,uniqueVal);
        if(!val)return 0;
        objSrc->propSet(objType,val);
    }
    return val;
}
const char * sUsr::createSharedUniqueObject(sUsrObj * objDst, const char * objType, const char * uniqueVar,const char * uniqueVal,  const char * folder, const char * shareTo )
{
    sUsrObjRes obj_res;
    sHiveId hid;
    sStrT t;t.printf("^%s$",objType);

    bool isnew=true;
    if( objs2(t.ptr(0), obj_res,(udx*)0,uniqueVar,uniqueVal) )  {
        hid=*obj_res.id(obj_res.first());
        isnew=false;
    }else {
        objCreate(hid, objType);
        if(shareTo && *shareTo) {
            sVec<idx> gs;
            groupIdFromPath(shareTo, &gs);
            for(idx g = 0; g < gs.dim(); ++g) {
                setPermission(gs[g],hid,7,2);
            }
        }
    }

    new (objDst) sUsrObj(*this,hid);
    if(isnew) {
        objDst->propSet(uniqueVar, uniqueVal);
    }
    if(folder)
        objDst->propSet("folder", folder);


    return objDst->IdStr();
}






idx sUsr::replaceVarsFromObjForm(sStr * dst, const char * script,sUsrObj * obj, sVar * pForm, sStr * log, bool jsonMode) const
{
    sStrT var,pro,val,d;
    const char * varStart, * varEnd;
    for( const char * p=script; *p ; script=varEnd+1){
        idx quoted=0; char toquote=0;
        varStart=strstr(script,"${");if(!varStart)break;
        if(varStart>script && *(varStart-1)=='\"' )  quoted=1;

        if(varStart!=script)dst->add(script,(idx)(varStart-script-quoted));
        varStart+=2;
        varEnd=strstr(varStart+1,"}");if(!varEnd)break;

        const char * prop=varStart, * value;
        if(*prop=='\'' || *prop=='\"'){toquote=*prop;++prop;}
        if(*prop=='.')value=obj->IdStr() ;
        else {
            var.cut(0);while(prop<varEnd && *prop!='.'){var.add(prop,1);++prop;};var.add0(1);
            val.cut(0);
            value=0;
            const char * vv=var.ptr(0);
            if(vv && vv[0]=='?' && vv[1]=='?' ) {
                sUsrObjRes obj_res;
                d.cut(0);sString::searchAndReplaceSymbols(&d,vv+2,0,":",0,0,true,true,true,true,0);
                const char * type=d.ptr(),* pars=sString::next00(type), * rvals=sString::next00(pars), * rprop=sString::next00(rvals);
                if( !objs2(type, obj_res,(udx*)0,pars,rvals) )
                    value="";
                else {
                    d.cut(0);
                    for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
                        if(d.length())d.add(";");
                        if(!rprop)
                            d.printf("%" UDEC ,obj_res.id(it)->objId() );
                        else  {
                            sUsrObj o(*this,sHiveId(obj_res.id(it)->objId(),0));
                            if(o.Id()) {
                                d.printf("%s" ,o.propGet(rprop) );
                            }
                        }
                    }
                    value = d.length() ? d.ptr() : "";
                }
            }
            else {
                if(obj)value=obj->propGet(var.ptr(),&val);
                if(!value && pForm) {
                    idx vallen=0;
                    value=pForm->value(var.ptr(),0,&vallen);
                    value=val.printf(0,"%.*s",(int)vallen,value);
                }
            }
        }

        if(prop<varEnd-1) {
            pro.cut(0);pro.add(prop+1,varEnd-prop-1);pro.add0(1);
            sUsrObj ov(*this,sHiveId(value));
            val.cut(0);
            if(ov.Id()) {
                value=ov.propGet(pro.ptr(),&val,true);
            } else {
                if(strcmp(pro.ptr(),"_dir")==0){
                    value=val.printf(0,"/tmp");
                }

            }
        }


        if(value && sLen(value)) {
            if(toquote)dst->add("\"",1);
            dst->add(value);dst->shrink00();
            if(toquote)dst->add("\"",1);
        }else {
            if (jsonMode && dst->length()>0) {
                idx ipos=dst->length()-1;
                while( ipos>0 && strchr(" \r\n,\t",*dst->ptr(ipos)) ) ipos--; 
                dst->cut(ipos+1);
            }
        }
        varEnd+=quoted;
    }
    dst->add(script);
    dst->add0(1);

    return 1;
}


const char * sUsr::uniqueObjectAndPath( const char * objType, const char * idorname, sUsrObj * obj, sStr * pathAlgo)
{
    sUsrObjRes obj_res;
    sUsrObjRes obr;

    sStrT buf;if(!isdigit(idorname[0]) )buf.printf("^%s$",idorname);
    if( !objs2(objType, obr,(udx*)0, isdigit(idorname[0]) ? "_id" : "name", isdigit(idorname[0]) ? idorname : buf.ptr(0)) ) {
        return 0;
    }

    sStrT PathAlgo;
    if(!pathAlgo)pathAlgo=&PathAlgo;
    sUsrObj oo;if(!obj)obj=&oo;

    new (obj) sUsrObj(*this, *obr.id(obj_res.first())  );
    obj->getFilePathname(*pathAlgo);

    return pathAlgo ? pathAlgo->ptr(0) : 0 ;
}



const char * sUsr::objJS(idx obj, sStr * json, idx * plengh, bool respectArr)
{
    static sStr Json;
    sUsrObjRes obj_res;
    if(!json){Json.cut(0);json=&Json;}
    sStr obl("%" DEC, obj ) ;
    if( !objs2(0, obj_res,(udx*)0,"_id",obl.ptr() ) )
        return 0;

    sJSONPrinter pr;pr.init(json);pr.respectArrays=true;
    pr.startObject();
    for( sUsrObjRes::IdIter it=obj_res.first(); obj_res.has(it); it=obj_res.next(it)) {
        obj_res.json(*this, it, pr, true, false);
    }
    pr.endObject();
    return json->ptr(0);
}


sJson * sUsr::objJson(idx obj,sJson * json, bool respectArr)
{
    static sJson JS;
    if (!json){json=&JS; json->cln();}
    return json->initMem(objJS(obj,0,0,respectArr));
}
