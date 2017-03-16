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
#include <slib/std/file.hpp>
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

// static
const char* const sUsr::getKey(void)
{
    // normally this key comes from configuration, see setQpride function
    if( !g_key ) {
        // in case it is missing somewhat random text is generated here
        // relaying on this part will cause all session invalidation upon new binaries build
        g_key.printf("%sx%sx%s", "$Id$", __FILE__, __DATE__);
    }
    return g_key.ptr();
}

const char* sUsr::encodeSID(sStr & sid, sStr & buf)
{
    time_t t = time(0);
    udx gmt_time = mktime(gmtime(&t));
    sStr str("q%"UDEC"|%"DEC"|%"UDEC"|%"UDEC, m_SID, m_SIDrnd, m_Id, gmt_time);
    str.printf("|%"UDEC, sAlgo::algo_murmurHash64(str.ptr(1), str.length() - 1, 32, 0, gmt_time));
    idx pos = buf.length();
    sMex cryptbin;
    idx cryptlen = sBlockCrypto::encrypt(&cryptbin, sBlockCrypto::eAES256_HMACSHA256, str.ptr(1), str.length() - 1, getKey(), sLen(getKey()));
    if( cryptlen > 0 ) {
        sString::encodeBase64(&buf, (const char *)cryptbin.ptr(), cryptlen, false);
        buf.add0cut(2);
    }
    if( !sid ) {
        const char* e = getenv("UNIQUE_ID");
        if( e && e[0] ) {
            sid.printf("%s", e);
        } else {
            sid.printf("r%"UDEC"%"UDEC, static_cast<udx>(rand()), gmt_time);
        }
    }
    buf.printf("@%s", sid.ptr());
    return buf.ptr(pos);
}

// characters that cannot occur within a password hash, and that separate multiple password hashes in a list
static const char * hash_seps = " \t\r\n";

// Check password. If the current hash uses an outdated algorithm and the password matches,
// automatically upgrade to a better algorithm
static
bool checkPassword(const char * cur_hash, idx cur_hash_len, const char * pass, const char * email, const char * user_id, sStr * upgraded_hash = 0)
{
    bool need_upgrade = false;
    bool match = false;
    if( cur_hash && pass ) {
        if( !cur_hash_len ) {
            cur_hash_len = strcspn(cur_hash, hash_seps);
        }
        match = sPassword::checkPassword(cur_hash, cur_hash_len, pass, &need_upgrade);
    }
    if( need_upgrade && upgraded_hash ) {
        sPassword::encodePassword(*upgraded_hash, pass);
    }
    // invalid
    return match;
}

static const idx field_encryption_secret_len = 64;

// return XOR of builtin 64-byte secret and hex-encoded value of qm.encoder in QPCfg
static void * ensureFieldEncryptionSecret(sQPrideBase * qpb)
{
    static const unsigned char builtin_field_encryption_secret[field_encryption_secret_len] = {
        0x27, 0xd4, 0xc7, 0x49, 0x67, 0x7f, 0x6c, 0xc3, 0xce, 0xdc, 0x11, 0x3e, 0xb9, 0xde, 0x3a, 0xde,
        0x40, 0xb7, 0xdf, 0xb3, 0x22, 0x5c, 0x22, 0xf8, 0x4b, 0xa2, 0xca, 0x1f, 0x13, 0xb6, 0x64, 0x64,
        0x6d, 0x4a, 0xad, 0x6a, 0x4c, 0xc6, 0x0d, 0xe7, 0x18, 0x0b, 0x8e, 0xb3, 0x91, 0xd5, 0x56, 0x51,
        0x83, 0x15, 0xb4, 0x4a, 0x36, 0x72, 0x87, 0x6e, 0x40, 0xc8, 0xa7, 0xc4, 0x27, 0x2c, 0xf1, 0xc3
    };
    static unsigned char field_encryption_secret[field_encryption_secret_len] = { 0 };
    static enum {
        eUncached,
        eCached,
        eError
    } field_encryption_secret_status = eUncached;

    if( field_encryption_secret_status == eUncached ) {
        sStr qm_encoder_hex;
        qpb->configGet(&qm_encoder_hex, 0, "qm.encoder", 0, 0);
        if( qm_encoder_hex.length() ) {
            memcpy(field_encryption_secret, builtin_field_encryption_secret, field_encryption_secret_len);
            for(idx i = 0; i < field_encryption_secret_len && i < qm_encoder_hex.length() * 2; i++ ) {
                unsigned int qm_encoder_byte = 0;
                sscanf(qm_encoder_hex.ptr(2 * i), "%02x", &qm_encoder_byte);
                field_encryption_secret[i] ^= (unsigned char)qm_encoder_byte;
            }
            field_encryption_secret_status = eCached;
        } else {
            field_encryption_secret_status = eError;
        }
    }

    return field_encryption_secret_status == eCached ? field_encryption_secret : 0;
}

enum FieldEncodings {
    eField_eAES256_HMACSHA256 = 1
};


bool sUsr::encodeField(sStr * out_encoded_value, sMex * out_encoded_blob, idx encoding, const void * orig_value, idx len) const
{
    if( encoding == eField_eAES256_HMACSHA256 ) {
        const void * secret = ensureFieldEncryptionSecret(QPride());
        if( secret && sBlockCrypto::encrypt(out_encoded_blob, sBlockCrypto::eAES256_HMACSHA256, orig_value, len, secret, field_encryption_secret_len) >= 0 ) {
            return true;
        }
    }
    return false;
}

bool sUsr::decodeField(sMex * out, idx encoding, const void * encoded_value, idx value_len, const void * encoded_blob, idx blob_len) const
{
    if( encoding == eField_eAES256_HMACSHA256 ) {
        const void * secret = ensureFieldEncryptionSecret(QPride());
        if( secret && sBlockCrypto::decrypt(out, sBlockCrypto::eAES256_HMACSHA256, encoded_blob, blob_len, secret, field_encryption_secret_len) >= 0 ) {
            return true;
        }
    }
    return false;
}

sSql* sUsr::pdb(bool initIfUndefined) const
{
    if( !sm_actual_db && initIfUndefined ) {
        // preserves single connection when no configs found and has qpdb!!!
        static struct sCfg
        {
            char db[64];
            char server[128];
            char user[64];
            char pass[256];
            udx debug;
        } cfg;
        sString::SectVar cfgVars[] = {
            { 0, "[HIVE]"_"db"__, "%s="HIVE_DB, "%s", &cfg.db },
            { 0, "[HIVE]"_"server"__, "%s="HIVE_DB_HOST, "%s", &cfg.server },
            { 0, "[HIVE]"_"user"__, "%s="HIVE_DB_USER, "%s", &cfg.user },
            { 0, "[HIVE]"_"pass"__, "%s="HIVE_DB_PWD, "%s", &cfg.pass },
            { 0, "[HIVE]"_"debug"__, "%"UDEC"=0", "%s", &cfg.debug }
        };
        const char* cfgs[] = { "hive.cfg", "~/hive.cfg", "qapp.cfg", "~/qapp.cfg", "~/.my.cnf"};
        for(idx i = 0; i < sDim(cfgs); ++i) {
            sFil inp(cfgs[i], sFil::fReadonly);
            if( inp.length() ) {
                sStr rst;
                sString::cleanMarkup(&rst, inp, inp.length(), "//"_"/*"__, "\n"_"*/"__, "\n", 0, false, false, true);
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
                // fail over to qpride db connection
                sm_actual_db = qp->sql();
            }
        }
        // TODO call connection setup stored procedure
    }
    return sm_actual_db;
}

// static
void sUsr::setQPride(sQPrideBase * qpride)
{
    // May be called by ~sQPrideBase, so we cannot use dynamic_cast<sQPride*>(sm_qpride)
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
            audit(eUserAuditLogin, __func__, "sessionID='%"UDEC"'; rnd='%"DEC"'; result='expired?'", sid, key);
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
        sStr log("forked from %"UDEC"%s%s", m_SID, ipaddr ? " " : "", ipaddr ? ipaddr : "");
        p->Add(Id()).Add(log).Add(now).Add(true);
        sVarSet tbl;
        p->getTable(&tbl);
        if( !db().HasFailed() && tbl.rows > 0 && tbl.cols > 1 ) {
            m_SID = tbl.uval(0, 0, 0);
            m_SIDrnd = tbl.ival(0, 1, 0);
        } else {
            err.printf(0, "Persistent session cannot be established, try again later");
        }
        audit(eUserAuditLogin, __func__, "sessionID='%"UDEC"'; user_id=%"UDEC"; rnd='%"DEC"'; source='%s'; result='%s'", m_SID, Id(), m_SIDrnd, ipaddr, err ? err.ptr() : "ok");
    }
}

void sUsr::initFolders(bool keepHierarchy)
{
    // TODO should be done from admin account
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
#if 0
        sVec<sHiveId> orph;
        sUsrFolder::orphans(*this, orph, "");
        for(idx i = 0; i < orph.dim(); ++i) {
            std::auto_ptr<sUsrObj> o(objFactory(orph[i]));
            if( o.get() && o->Id() ) {
                const char * h = o->propGet("hierarchy");
                if( keepHierarchy && h && h[0] && strncasecmp(h, "/http", 5) != 0 ) {
                    std::auto_ptr<sUsrFolder> f(sysFolder->createSubFolder("%s", h));
                    if( f.get() ) {
                        f->attach(*o.get());
                    }
                } else {
                    sysFolder->attach(*o.get());
                }
            }
        }
#endif
    }
    sysFolder.reset(sSysFolder::Trash(*this, true));
    if( sysFolder.get() ) {
        m_SuperUserMode = true;
        setPermission(m_PrimaryGroup, sysFolder->Id(), ePermCanRead | ePermCanBrowse | ePermCanWrite, eFlagDefault, 0);
        m_SuperUserMode = su;
    }
#if 0
    // find all user visible objects, given criteria
    if( false ) {
        sUsrObjRes all;
        objs2("folder,sysfolder", all);
        for(sUsrObjRes::IdIter it = all.first(); all.has(it); all.next(it)) {
            std::auto_ptr<sUsrObj> o(objFactory(*(all.id(it))));
            if( o.get() ) {
                ((sUsrFolder*)o.get())->fixChildrenPath();
            }
        }
    }
#endif
}

sUsr::ELoginResult sUsr::login(const char * email, const char * pswd, const udx token, const char * ipaddr, idx * plogCount)
{
    ELoginResult status = eUserBlocked;
    sVarSet t;
    sStr tmp;
    const char * lemail = db().protect(tmp, email);
    tmp.add0(2);
    sStr sql("SELECT is_active_fg, userID, is_email_valid_fg, is_admin_fg, pswd, pswd_reset_id, "
        "loginTm + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.accountExpireDays') DAY < NOW() AS account_expired, "
        "IFNULL(pswd_changed, NOW() - INTERVAL 200 YEAR) + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.pswdExpireDays') DAY < NOW() AS pswd_expired "
        "FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
    db().getTable(&t, "%s", sql.ptr());
    if( t.rows == 1 ) {
        if( token ) {
            status = (token == t.uval(0, t.colId("pswd_reset_id"))) ? eUserOperational : eUserNotFound;
        } else {
            const char * pp = t.val(0, t.colId("pswd"));
            sStr upgraded_pp;
            if( checkPassword(pp, 0, pswd, email, t.val(0, t.colId("userID")), &upgraded_pp) ) {
                if( upgraded_pp.length() ) {
                    db().execute("UPDATE UPUser SET pswd='%s', modifTm = CURRENT_TIMESTAMP WHERE userID=%"UDEC, upgraded_pp.ptr(), t.uval(0, t.colId("userID")));
                    audit(eUserAuditLogin, "upgrade_password", "userID=%"UDEC"; from='%s'; to='%s'", t.uval(0, t.colId("userID")), pp, upgraded_pp.ptr());
                    pp = upgraded_pp.ptr();
                }
            } else {
                status = eUserNotFound;
            }
        }
        if( status == eUserBlocked ) {
            // admin can get in even with invalid email or expired account!
            const bool is_admin = t.boolval(0, t.colId("is_admin_fg"));
            if( !t.boolval(0, t.colId("is_email_valid_fg")) && !is_admin ) {
                status = eUserEmailNotValidated;
            } else if( t.boolval(0, t.colId("account_expired")) && !is_admin ) {
                status = eUserAccountExpired;
            } else if( t.boolval(0, t.colId("pswd_expired")) ) {
                status = eUserPswdExpired;
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
                // ~~first time login only, to prevent sysfolder dupes, this should completely go away if there no new accounts registered before this fix
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
    audit(eUserAuditLogin, __func__, "email='%s'; sessionID='%"UDEC"'; rnd='%"DEC"'; source='%s'; result='%"DEC, email, m_SID, m_SIDrnd, ipaddr, (idx)status);
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
    audit(eUserAuditLogin, __func__, "source='%s'; result='%"UDEC": %s'", ipaddr, db().Get_errno(), db().Get_error().ptr());
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
    prohibitSelfRegistration = 0;
    checkComplexity = 1;
    m_SuperUserMode = false;
    reset();
    init(usrid);
}

sUsr::sUsr(const char* service_name, bool su_mode /* = false */)
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
            if( log.length() > 256 ) { // split into multiple lines
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
                if( m_IsAdmin && m_SuperUserMode ) {
                    m_membership.printf(" TRUE ");
                    for(idx ir = 1; ir < usr.rows; ++ir) {
                        if( usr.ival(ir, 1) == -1 ) {
                            m_PrimaryGroup = usr.uval(ir, 0);
                            break;
                        }
                    }
                } else {
                    sDic<idx> parDic;
                    sStr directParents;
                    m_membership.printf("((p.groupID IN (");
                    for(idx ir = 1; ir < usr.rows; ++ir) {
                        if( m_PrimaryGroup == 0 && usr.ival(ir, 1) == -1 ) {
                            m_PrimaryGroup = usr.uval(ir, 0);
                        }
                        const char * p = usr.val(ir, 2);
                        m_membership.printf("%s,", usr.val(ir, 0));
                        const char * curSlash;
                        sStr t;
                        for(curSlash = sString::skipWords(p + 1, 0, 1, "/"__); curSlash; curSlash = sString::skipWords(curSlash + 1, 0, 1, "/"__)) {
                            t.cut(0);
                            t.add(p, curSlash - p);
                            t.add0();
                            parDic.set(t.ptr());
                        }
                        if( directParents.length() > 0 ) {
                            directParents.printf(" OR ");
                        }
                        if( t.ptr() ) {
                            directParents.printf("groupPath like '%s%%'", t.ptr());
                        }
                        parDic.set(p);
                    }
                    // direct membership rule
                    m_membership.cut(-1);
                    m_membership.printf(") AND (p.flags & %"UDEC") = 0) OR ", (udx)(eFlagInheritDown | eFlagInheritUp));
                    // inherit down
                    m_membership.printf("((g.groupPath in (");
                    for(idx i = 0; i < parDic.dim(); ++i) {
                        if( i > 0 ) {
                            m_membership.printf(",");
                        }
                        m_membership.printf("'%s'", (const char *)(parDic.id(i)));
                    }
                    m_membership.printf(")) AND (p.flags & %"UDEC") != 0)", (udx)eFlagInheritDown);
                    if( directParents ) {
                        // inherit up
                        m_membership.printf(" OR ((%s) AND (p.flags & %"UDEC") != 0) )", directParents.ptr(), (udx)eFlagInheritUp);
                    }
                }
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


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Account settings
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

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
        // TODO add checksum
        sStr body("%s %s,\n\n"
                "Your account on HIVE was successfully created.\n"
                "Now, in order to verify your email address, please, click the link below:\n"
                "%s?cmd=userV1&emailAct=%s\n"
                "\nHIVE Team.\n", firstName, lastName, baseURL, email);
        sUsrEmail eml(admin, email, "HIVE registration", body.ptr());
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
            // TODO add checksum validation here
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
        sUsr admin("qpride");
        // TODO add checksum
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
        sUsrEmail eml(admin, adminEmail, "HIVE account activation request", body.ptr());
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
            // TODO add checksum validation here
            db().execute("UPDATE UPUser SET is_active_fg = TRUE, loginTm = NULL WHERE email = '%s' AND `type` = 'user'", tmp.ptr());
            t.empty();
            db().getTable(&t, "SELECT is_active_fg, first_name, last_name, email FROM UPUser WHERE email = '%s' AND `type` = 'user'",  tmp.ptr());
            if( t.rows == 1 && t.uval(0, 0) ) {
                sUsr admin("qpride");
                // TODO add checksum
                sStr body("%s %s,\n\n"
                        "Your account on HIVE is now activated.\n"
                        "Please click here to login: %s?cmd=login&login=%s\n"
                        "\nHIVE Team.\n", t.val(0, 1), t.val(0, 2), baseURL, email);
                sUsrEmail eml(admin, t.val(0, 3), "HIVE account activation confirmation", body.ptr());
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

bool sUsr::groupActivate(idx groupId)
{
    bool res = false;
    sVarSet t;
    db().getTable(&t, "SELECT is_active_fg FROM UPGroup WHERE groupID = %"UDEC,  groupId);
    if( t.rows == 1 ) {
        if( !t.uval(0, 0) ) {
            db().execute("UPDATE UPGroup SET is_active_fg = TRUE WHERE groupID = %"UDEC, groupId);
            t.empty();
            db().getTable(&t, "SELECT is_active_fg, groupPath FROM UPGroup WHERE groupID = %"UDEC, groupId);
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
        err.printf(0, "group id %"UDEC" is not found", groupId);
    }
    audit(eUserAuditAdmin, __func__, "groupID='%"UDEC"'; result='%s'", groupId, !res ? err.ptr() : "ok");
    return res;
}

bool sUsr::groupCreate(const char* name, const char* abbr, const char* parent)
{
    bool res = false;
    if( !name || !name[0] || !abbr || !abbr[0] ) {
        err.printf("Missing group name and/or abbreviation:\nName: '%s'\nAbbreviation: '%s'\n", name, abbr);
    } else if( !parent || !parent[0] || parent[0] != '/' || parent[strlen(parent) - 1] != '/' ) {
        err.printf("Invalid parent group path '%s'.", parent);
    } else {
        if( db().uvalue(0, "SELECT userID FROM UPUser WHERE first_name = '%s%s/' OR (first_name LIKE '%s%%' AND last_name LIKE '%s') AND `type` = 'group'", parent, abbr, parent, name) != 0 ) {
            err.printf("Group with same name and/or abbreviation already exists.");
        } else if( updateStart() ) {
            // email now determines if a person can add new sub groups to a group
            db().execute("INSERT INTO UPUser (first_name, last_name, email, is_active_fg, is_email_valid_fg, pswd, `type`)"
                    " VALUES ('%s%s/', '%s', '%s', TRUE, TRUE, '--not an account--', 'group')", parent, abbr, name, m_Email.ptr());
            if( !db().HasFailed() ) {
                db().execute("INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath)"
                        " VALUES ((SELECT userID FROM UPUser WHERE first_name = '%s%s/' AND `type` = 'group'), -1, TRUE, '%s%s/')", parent, abbr, parent, abbr);
            }
            if( !db().HasFailed() ) {
                res = updateComplete();
            }
            if( !res ) {
#ifdef _DEBUG
                err.printf("SQL error: [%"UDEC"] '%s'", db().Get_errno(), db().Get_error().ptr());
#endif
                updateAbandon();
            }
        }
    }
    audit(eUserAuditAdmin, __func__, "name='%s'; abbr='%s'; result='%s'", name, abbr, res ? "ok" : err.ptr());
    return res;
}

bool sUsr::contact(const char * from_email, const char * subject, const char * body)
{
    err.cut(0);
    if( !from_email || strchr(from_email, '@') == 0 || strchr(from_email, '.') == 0 ) {
        err.printf(0, "Invalid email address!");
    } else {
        sUsr admin("qpride");
        sQPrideBase * qp = QPride();
        sStr adminEmail, msg("From: %s\n%s", from_email, (body && body[0]) ? body : "empty message body");
        if( qp ) {
            qp->cfgStr(&adminEmail, 0, "emailAddr", 0);
        }
        sUsrEmail eml(admin, adminEmail, (subject && subject[0]) ? subject : "No subject", msg);
    }
    audit(eUserAuditAdmin, __func__, "sessionID='%"UDEC"'; user_id=%"UDEC"; from='%s'; result='%s'", m_SID, Id(), from_email, err ? err.ptr() : "ok");
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
            sUsr admin("qpride");
            udx r = rand(), r1 = rand();
            db().execute("UPDATE UPUser SET pswd_reset_id = IF(pswd_reset_id = %"UDEC", %"UDEC", %"UDEC") WHERE email = '%s' AND `type` = 'user'", r, r1, r, tmp.ptr());
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
    audit(eUserAuditAdmin, __func__, "email='%s'; pswd_reset_id=%"UDEC"; result='%s'", email, pswd_reset_id, err ? err.ptr() : "ok");
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
            // TODO add checksum
            sUsr admin("qpride");
            sStr body("%s %s,\n\n"
                      "To complete your request to reset password click the link below:\n"
                      "%s?cmd=pswdSet&login=%s&pswd=%"UDEC"&x=%"UDEC"\n"
                      "\nHIVE Team.\n", t.val(0, 0), t.val(0, 1), baseURL, t.val(0, 2), t.uval(0, 3), (udx)(time(0) + 24 * 60 * 60));
            sUsrEmail eml(admin, email, "HIVE password notification", body.ptr());
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
    db().getTable(&t, "SELECT is_active_fg, is_email_valid_fg, userID, "
        "(loginTm + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.accountExpireDays') DAY < NOW() AND !is_admin_fg) AS account_expired, "
        "IFNULL(pswd_changed, NOW() - INTERVAL 200 YEAR) + INTERVAL (SELECT IF(val REGEXP '^[0-9]+$', val, NULL) FROM QPCfg WHERE par = 'user.pswdExpireDays') DAY < NOW() AS pswd_expired "
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
        } else {
            udx r = 0;
            while(r == 0) { // Repeat to make sure new value is set
                r = rand();
                db().execute("UPDATE UPUser SET pswd_reset_id = IF(pswd_reset_id = %"UDEC", NULL, %"UDEC") WHERE email = '%s' AND `type` = 'user'", r, r, tmp.ptr());
                t.empty();
                db().getTable(&t, "SELECT pswd_reset_id, userID FROM UPUser WHERE email = '%s' AND `type` = 'user'",  tmp.ptr());
                r = (t.rows == 1 && t.cols == 2) ? t.uval(0, 0) : 0;
            }
            // a little hack here
            const udx s = m_SID, i = m_Id;
            m_SID = r; m_Id = t.uval(0, 1);
            tmp.cut(0);
            encodeSID(tmp, token);
            m_SID = s; m_Id = i;
            res = token ? eUserOperational : eUserNotFound;
        }
    }
    audit(eUserAuditLogin, "piv-auth", "email='%s'; result='%"UDEC"'", email, (udx)res);
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
    idx cSymb = -sIdxMax; // do not check
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
        db().getTable(&t, "SELECT userID, is_active_fg, is_email_valid_fg FROM UPUser WHERE email = '%s' AND pswd_reset_id = %"UDEC" AND `type` = 'user'",
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
    // if valid userId was retrieved, audit() was done by passwordReset(userId, mod, mod1)
    return res;
}

bool sUsr::passwordReset(udx userId, const char * mod, const char * mod1)
{
    bool res = false;
    if( passwordCheckQuality(mod, mod1) ) {
        sVarSet t;
        db().getTable(&t, "SELECT pswd, email FROM UPUser WHERE userID = %"UDEC, userId);
        if( t.rows == 1 ) {
            const char * cur_hash = t.val(0, 0);
            const char * email = t.val(0, 1);
            sStr hashes_buf("NULL");
            bool password_reused = false;

            if( idx num_keep_old = db().ivalue("SELECT val FROM QPCfg WHERE par = 'user.pswdKeepOldQty'", 0) ) {
                sStr userID_str("%"UDEC, userId);
                hashes_buf.printf(0, "'%s ", cur_hash);
                idx cur_hash_pos = 1; // skips initial quote
                db().svalue(hashes_buf, "SELECT pswd_prev_list FROM UPUser WHERE userID = %"UDEC, userId); // whitespace-delimeted list of hashes, most recent is first
                for(idx ihash = 0; ihash < num_keep_old; ihash++) {
                    idx ws = strspn(hashes_buf.ptr(cur_hash_pos), hash_seps); // skip any whitespace
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
                             "pswd_changed = NOW(), pswd_prev_list=%s, modifTm = CURRENT_TIMESTAMP WHERE userID=%"UDEC, sPassword::encodePassword(tmp, mod), hashes_buf.ptr(), userId);
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
            // this is a new user creation
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
                // insert a new user
                db().execute("INSERT INTO UPUser (email, is_active_fg, pswd, logCount, `type`, first_name, last_name, createTm)"
                                    " VALUES ('%s', FALSE, '--TBD--', %u, 'user', '%s', '%s', CURRENT_TIMESTAMP)",
                                        lemail, m_IsAdmin ? 0 : 1, lfirstName, llastName);
                userId = db().uvalue(0, "SELECT userID FROM UPUser WHERE email = '%s' AND `type` = 'user'", lemail);
                if( userId ) {
                    sStr tmp("%"UDEC, userId);
                    tmp.add0(2);
                    const char * pswd = sPassword::encodePassword(tmp, newpass1);
                    db().execute("UPDATE UPUser SET pswd = '%s', pswd_changed = NOW() WHERE userID = %"UDEC, pswd, userId);
                    // create a personal group (flag = -1) for new user
                    db().execute("insert into UPGroup (userID, flags, is_active_fg, groupPath) values(%"UDEC", -1, TRUE, '/everyone/users/%s')", userId, lemail);
                    if( !db().HasFailed() ) {
                        log.printf("new_user='%s'; ", email);
                        sendEmailValidation(baseURL, email, firstName, lastName);
                        firstName = lastName = 0; // avoid reset later in this function
                    } else {
                        err.printf(0, "Registration failed, please, come back later!");
#if _DEBUG
                        err.printf(" mysql: %"UDEC" '%s'", db().Get_errno(), db().Get_error().ptr());
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
            db().svalue(log, "SELECT first_name FROM UPUser WHERE userID = %"UDEC, userId);
            log.addString("'; ");
            db().execute("update UPUser set first_name = '%s', modifTm = CURRENT_TIMESTAMP where userID = %"UDEC, lfirstName, userId);
            result = !db().HasFailed();
        }
        if( result && lastName && lastName[0] && strcmp(m_Last, lastName) != 0 ) {
            log.addString("old_last='");
            db().svalue(log, "SELECT last_name FROM UPUser WHERE userID = %"UDEC, userId);
            log.addString("'; ");
            db().execute("update UPUser set last_name = '%s', modifTm = CURRENT_TIMESTAMP where userID = %"UDEC, llastName, userId);
            result = !db().HasFailed();
        }
        if( result && groups.dim() ) {
            // update membership
            log.printf("old_groups='%s'; ", groupList());
            sStr g;
            sString::printfIVec(&g, &groups, ",");
            // delete unselected
            db().execute("DELETE FROM UPGroup WHERE userID = %"UDEC" AND flags != -1 AND groupPath NOT IN (SELECT CONCAT(first_name, '%s') FROM UPUser WHERE userID IN (%s))", userId, lemail, g.ptr());
            result = !db().HasFailed();
            if( result ) {
                // insert new but ignore existing
                db().execute("INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath) "
                         "SELECT %"UDEC", 0, FALSE, CONCAT(first_name, '%s') FROM UPUser WHERE userID IN (%s) AND userID NOT IN ("
                           "SELECT userID FROM UPUser WHERE first_name IN ("
                                "SELECT SUBSTRING(groupPath, 1, LENGTH(groupPath) - LENGTH('%s')) FROM UPGroup WHERE userID = %"UDEC" AND flags != -1 AND groupPath IN ("
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
        initFolders(true); // create basic folder structure for new user
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
    return db().svalue(buf, "SELECT GROUP_CONCAT(userID) FROM UPUser WHERE `type` = 'group' AND CONCAT(first_name, '%s') IN (SELECT groupPath FROM UPGroup WHERE userID = %"UDEC" AND is_active_fg IN (%s))",
        m_Email.ptr(), m_Id, inactive ? "TRUE, FALSE" : "TRUE");
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Object and Permission
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

sRC sUsr::objCreate(sHiveId & out_id, const char* type_name, const udx in_domainID/* = 0 */, const udx in_objID/* = 0*/) const
{
    sRC rc;
    out_id.reset();
    if( type_name && type_name[0] ) {
        const sUsrType2 * tp = sUsrType2::ensure(*this, type_name);
        // only admins/superuser can create objects descending from base_system_type
        if( tp && !tp->isVirtual() ) {
            if( m_SuperUserMode || m_IsAdmin || !tp->isSystem() ) {
                static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
                std::auto_ptr<sSql::sqlProc> p(getProc( use_type_upobj ? "sp_obj_create_v2" : "sp_obj_create"));
                if( use_type_upobj ) {
                    p->Add(tp->id().domainId()).Add(tp->id().objId()).Add(in_domainID).Add(in_objID);
                } else {
                    p->Add(type_name);
                }
                p->Add((udx) ePermCompleteAccess).Add((udx) eFlagDefault);
                sVarSet tbl;
                if( p->getTable(&tbl) && tbl.rows == 1 ) {
                    // TODO support ion_id?
                    if( use_type_upobj ) {
                        out_id.set(tbl.uval(0, 0), tbl.uval(0, 1), 0);
                    } else {
                        out_id.setObjId(tbl.uval(0, 0));
                    }
                }
                if( out_id ) {
                    sUsrObj * o = objFactory(out_id);
                    if( o ) {
                        // TODO this is temp fix to avoid duplicate obj dirs, in future storeManager must handle it
                        sStr hack;
                        o->addFilePathname(hack, true, ".deleteme");
                        delete o;
                    }
                } else {
                    if( db().HasFailed() ) {
                        // Serious DB failure that should not happen - need to log with details
                        // fprintf needed because a DB connection failure or deadlock might cause QPride()->logOut() to fail too
                        fprintf(stderr, "objCreate() DB error %"UDEC": %s\n", db().Get_errno(), db().Get_error().ptr());
                        if( QPride() ) {
                            QPride()->logOut(sQPrideBase::eQPLogType_Error, "objCreate() DB error %"UDEC": %s", db().Get_errno(), db().Get_error().ptr());
                        }
                    }
                    rc.set(sRC::eCreating, sRC::eObject, sRC::eOperation, sRC::eFailed);
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
    udx p = ePermNone;
    sObjPerm * op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    if( !op ) {
        sVec<sHiveId> out;
        objs(&objHiveId, 1, out);
        op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
    }
    if( op && m_SuperUserMode && !m_AllowExpiredObjects && op->expiration == sObjPerm::eMaybeExpired ) {
        // refetch permissions from db to check whether maybe-expired object is expired or not
        cacheRemove(objHiveId);
        sVec<sHiveId> out;
        objs(&objHiveId, 1, out);
        op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
        if( !op ) {
            // object was retrievable before, was maybe-expired before, failed to retrieve now when requiring unexpired;
            // so we conclude that if it is still retrieveable in allow-expired mode, then it is definitely expired
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
        if( m_SuperUserMode ) {
            // ignore all permissions in su mode
            p = ePermCompleteAccess & ePermMask;
        } else {
            p = op->allow & ~op->deny;
        }
    }
    return (p & permission) != 0;
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

void sUsr::objPermAll(sVec<sHiveId>& ids, sVarSet& tbl, bool expand_grp /* = false */) const
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

sHiveId sUsr::propExport(sVec<sHiveId>& ids, sVarSet & v, bool permissions) const
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
            if( grp && grp[sLen(grp) - 1] == '/' ) {
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

sHiveId sUsr::propExport(sVec<sHiveId>& ids, sJSONPrinter & printer, bool permissions, bool flatten/* = false */, bool upsert/* = false*/, const char * upsert_qry/* = 0*/) const
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

        res.json(*this, res.first(), printer, true, flatten, upsert, upsert_qry);

        bool have_grp_perm = false;
        for(idx ir = 0; ir < perm_tbl.rows; ir++) {
            sHiveId perm_id(perm_tbl.uval(ir, 0), perm_tbl.uval(ir, 1), 0);
            if( perm_id == ids[i] ) {
                // special handling for _prop for ION json format compatibility
                const char * grp = perm_tbl.val(ir, 6);
                if( grp && grp[sLen(grp) - 1] == '/' ) {
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

sHiveId sUsr::objFilesExport(sVec<sHiveId>& ids, sVarSet & v, const char * dstdir) const
{
    sFilePath p,dst;
    if(!dstdir) return 0;
    for(idx i = 0; i < ids.dim(); ++i) {
        std::auto_ptr<sUsrObj> obj(objFactory(ids[i]));
        if( !obj.get() ) {
            return ids[i];
        }

        sDir fileList00;
        obj->files(fileList00, sFlag(sDir::bitFiles)|sFlag(sDir::bitSubdirs));
        idx flcnt = 0;
        sStr fldir("%s/%s/",dstdir,obj->IdStr());
        sDir::removeDir(fldir);
        for( idx i=0 ; i < fileList00.dimEntries(); ++i){
            const char * src=fileList00.getEntryAbsPath(i);
            p.cut(0);p.makeName(src,"%%flnm");
            if( !sDir::exists(fldir) ) {
                sDir::makeDir(fldir);
            }
            dst.cut(0);dst.makeName(fldir,"%%dir/%s",p.ptr());
            sFile::remove(dst);
            if( sFile::symlink(src,dst) ) {
                v.addRow().addCol(obj->IdStr()).addCol("_file").addCol(++flcnt).printCol("%s/%s",obj->IdStr(),p.ptr());
            }
        }
    }
    return sHiveId();
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
        sStr fmt("%%b=%"HEX"|browse=%x|read=%x|write=%x|exec=%x|del=%x|admin=%x|share=%x|download=%x;",
            *perm, ePermCanBrowse, ePermCanRead, ePermCanWrite, ePermCanExecute, ePermCanDelete, ePermCanAdmin, ePermCanShare, ePermCanDownload);
        sString::xscanf(sperm, fmt, perm);
    }
    *flags = eFlagNone;
    if( sflags && sflags[0] ) {
        sStr fmt("%%b=%"HEX"|allow=0|active=0|deny=%x|down=%x|up=%x|hold=%x|revoke=%x;",
            *flags, eFlagRestrictive, eFlagInheritDown, eFlagInheritUp, eFlagOnHold, eFlagRevoked);
        sString::xscanf(sflags, fmt, flags);
    }
}

bool sUsr::setPermission(udx groupId, const sHiveId & objHiveId, udx permission, udx flags, sHiveId * viewId /* = 0 */, const char * forObjID) const
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
            if( !m_SuperUserMode && groupId != m_PrimaryGroup && (strcasecmp(tpnm, "folder") == 0 || strcasecmp(tpnm, "sysfolder") == 0 ) ) {
                permission &= (ePermCanBrowse | ePermCanRead | ePermCanShare);
            }
            if( admin || m_SuperUserMode ) {
                if( groupId == m_PrimaryGroup && !m_SuperUserMode ) {
                    // ensure it is visible to self
                    permission |= ePermCanBrowse | ePermCanAdmin;
                    // and cannot be altered or inherited since it is your own access not a group!
                    flags = eFlagDefault;
                }
            } else if( share ) {
                sObjPerm * op = m_ObjPermission.get() ? m_ObjPermission->get(&objHiveId, sizeof(sHiveId)) : 0;
                ok = op;
                if( op ) {
                    // do not elevate permissions
                    if( flags & eFlagRestrictive ) {
                        permission |= op->deny;
                    } else {
                        permission &= op->allow;
                    }
                }
            }
            if( ok ) {
                static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
                std::auto_ptr<sSql::sqlProc> p(getProc(use_type_upobj ? "sp_obj_perm_set_v2" : "sp_obj_perm_set"));
                p->Add(groupId);
                if( use_type_upobj ) {
                    p->Add(objHiveId.domainId()).Add(objHiveId.objId()).Add(viewId ? viewId->domainId() : (udx)0).Add(viewId ? viewId->objId() : (udx)0);
                } else {
                    p->Add(objHiveId.objId()).Add(viewId ? viewId->objId() : (udx)0);
                }
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
        audit(eUserAuditAdmin, __func__, "groupID='%"UDEC"'; objID='%s'; forObjID='%s'; view=%s; perm='%s'; result='%s'", groupId, objHiveId.print(), forObjID, viewId ? viewId->print() : "", perm_log.ptr(), ok ? "ok" : "failed");
    } else {
        audit(eUserAuditAdmin, __func__, "groupID='%"UDEC"'; objID='%s'; view=%s; perm='%s'; result='%s'", groupId, objHiveId.print(), viewId ? viewId->print() : "", perm_log.ptr(), ok ? "ok" : "failed");
    }
    return ok;
}

bool sUsr::copyPermission(const sHiveId & objHiveIdFrom, const sHiveId & objHiveIdTo) const
{
    bool ok = isAllowed(objHiveIdFrom, ePermCanAdmin) &&
              isAllowed(objHiveIdTo, ePermCanAdmin);
    if( ok ) {
        static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
        std::auto_ptr<sSql::sqlProc> p(getProc(use_type_upobj ? "sp_obj_perm_copy_v2" : "sp_obj_perm_copy"));
        // TODO support domain_id and ion_id
        if( use_type_upobj ) {
            p->Add(objHiveIdFrom.domainId()).Add(objHiveIdFrom.objId()).Add(objHiveIdTo.domainId()).Add(objHiveIdTo.objId());
        } else {
            p->Add(objHiveIdFrom.objId()).Add(objHiveIdTo.objId());
        }
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
            return log && sString::searchSubstring(log.ptr(pos), 0, "\nerr."__, sNotIdx, 0, false) != 0;
        }
        std::auto_ptr<sUsrQueryEngine> qengine;
        const sUsr & user;
        sStr & log;
};


#define RM_TRAIL_SPACE(v,l) {{ while(strchr("\n\r\t ", v[--l]) != 0 ) {} ++l; }}

// Check that a path is (1) non-empty, (2) non-absolute, (3) has no .. elements, (4) exists.
// If everything looks good, print to outbuf.
static const char * printRelativeFilePath(sStr & outbuf, const char * path, idx len, sFilePath & tmp)
{
    if( !path || !len ) {
        // empty path
        return 0;
    }

    tmp.addString(path, len);
    tmp.simplifyPath();
    tmp.shrink00();

    if( !tmp.length() || !tmp[0] ) {
        // empty path after simplification
        return 0;
    }
    if( tmp[0] == '/' ) {
        // absolute path
        return 0;
    }
    for(idx i = 0; i + 1 < len; i++) {
        if( path[i] == '.' && path[i + 1] == '.' && (i == 0 || path[i - 1] == '/') && (i + 2 == len || path[i + 2] == '/') ) {
            // path equals "..", or starts with "../", or ends with "/..", or contains "/../"
            return 0;
        }
    }
    idx outbuf_start = outbuf.length();
    outbuf.addString("./", 2);
    outbuf.addString(tmp.ptr(), tmp.length());
    if( outbuf[outbuf.length() - 1] == '/' ) {
        // clean up terminal '/' to avoid breaking nextToSlash()
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
        //! find first "\rprop." or similar in buf; returns pointer to start of prefix string in buf, or 0 if not found
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
                // path/value pairs and their lengths
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

        sDic<TPropVals> prop; // prop_name => values
        sHiveId id; // object id, if available
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
                        // special case: for type objects, fetch only type "type", no other types - otherwise,
                        // that could spuriously break "make install" when installing types out of order
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
                ++pnm; --pnm_len;
                if( strncasecmp(pnm, "id", pnm_len) == 0 ) {
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
                } else if( strncasecmp(pnm, "type", pnm_len) == 0 ) {
                    err = setType(val, val_len, ctx.user);
                } else if( strncasecmp(pnm, "delete", pnm_len) == 0 ) {
                    del.printf("%.*s", (int)val_len, val);
                    del.add0();
                } else if( strncasecmp(pnm, "file", pnm_len) == 0 ) {
                    RM_TRAIL_SPACE(val, val_len);
                    tmp_file_buf.cut0cut();
                    if( printRelativeFilePath(file, val, val_len, tmp_file_buf) ) {
                        file.add0();
                    } else {
                        err = not_for_db ? 0 : "invalid filename or path";
                    }
                } else if( strncasecmp(pnm, "comment", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "dir", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "brief", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "summary", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "info", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "action", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                    // these prop are ignored now
                } else if( strncasecmp(pnm, "perm", pnm_len) == 0 ) {
                    sMex::Pos * pos = perms.add();
                    if( pos ) {
                        pos->pos = (idx)val;
                        pos->size = val_len;
                    } else {
                        err = "insufficient resources (6)";
                    }
                } else if( strncasecmp(pnm, "info", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "warn", pnm_len) == 0 ) {
                    err = not_for_db ? addPropLowLevel(prefix, pnm, pnm_len, path, path_len, val, val_len, ctx) : 0;
                } else if( strncasecmp(pnm, "err", pnm_len) == 0 ) {
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
            char buf[256]; // see name column size from UPObjField
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
                    if( val_len > 16 * 1024 * 1024 ) { // see UPObjField table: MEDIUMTEXT -> 2^24
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

//static
const TPropObj::PrefixStr TPropObj::prefixes[] = {
    { ePrefixProp, "prop.", 5 },
    { ePrefixInfo, "info.", 5 },
    { ePrefixErr, "err.", 4 },
    { ePrefixWarn, "warn.", 5 },
    { ePrefixDbg, "dbg.", 4 },
};

//static
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
                return true; // skip random http / env values in pForm
            }
            const char * err = 0, *nm_end = nm + nm_len;
            do {
                const char * id = &nm[prefix_len];
                const char * id_end = id;
                // initial element might be domainId if true, go on till parts are numeric
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
                    continue; // skip empty object
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
                    pobj->del.add0();
                    for(const char * p = pobj->del; p; p = sString::next00(p)) {
                        if( pobj->getType()->getFieldType(ctx.user, p) == sUsrTypeField::eInvalid ) {
                            ctx.log.printf("\nerr.%s._delete=invalid property name '%s'", oid, p);
                        }
                    }
                }
                if( pobj->file ) {
                    pobj->file.add0();
                    /// TODO validation here might see if anything is returned by the mask
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
            // cleanup obj directories for new objects only
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
                    // create objects if needed
                    for(idx o = 0; o < all.dim(); ++o) {
                        TPropObj * pobj = all.ptr(o);
                        if( pobj->isEmpty() ) {
                            continue; // skip empty object
                        }
                        const char * oid = (const char *)all.id(o);
                        if( !pobj->id ) {
                            if( const sUsrType2 * otype = all.ptr(o)->getType() ) {
                                sRC rc = ctx.user.objCreate(pobj->id, otype->name());
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
                    //save properties
                    sStr valueSubstituteBuffer;
                    sStr objFiles00; // obj files to copy, in triplets: dst, src, objid
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
                                // perform value to id substitution
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
                                        if( !ctx.log ) { // do not add if there were errors before
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
                                        ctx.log.printf("\nerr.%s._file=cannot set permissions for group %"UDEC, (const char *)(all.id(o)), pp.groupId);
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
                                // return new obj ids mapping
                                for(idx i = 0; i < new_ids_map->dim(); ++i) {
                                    idx sz = 0;
                                    const void * p = new_ids_map->id(i, &sz);
                                    ctx.log.printf("\nprop.%.*s._id=%s", (int) sz, (const char *)p, new_ids_map->ptr(i)->print());
                                }
                                isok = true; // since now log contains new ids;
                            }
                        }
                    } else {
                        if( ctx.user.hadDeadlocked() && is_our_transaction ) {
                            // DB deadlock detected, and our own ctx.user.updateStart() call had
                            // started the current DB transaction. Wait a bit and retry.
                            ctx.user.updateAbandon();
                            isok = false;
                            // Reset id in elements of all dictionary for newly created objects
                            // (to ensure objCreate will be re-run on next retry iteration)
                            for(idx o = 0; o < all.dim(); ++o) {
                                TPropObj * pobj = all.ptr(o);
                                if( pobj->isEmpty() ) {
                                    continue; // skip empty object
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
                // cleanup obj directories for new objects only
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
        sDic<TPropObj> all; // idstr -> props
        TPropCtx ctx;
};

bool sUsr::propSet(const char * propFileName, sStr& log, sVec<sHiveId>* new_ids /* = 0 */, sVec<sHiveId>* updated_ids /* = 0 */, sDic<sHiveId> * new_ids_map /* = 0 */)
{
    sFil propFile(propFileName, sMex::fReadonly);
    bool ret = false;
    if( !propFile.ok() ) {
        log.printf("err.null._err=prop file not found '%s'", propFileName);
    } else {
        // _file props are relative to prop file location to avoid security breaches
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
            val_end = lastpos; // till end of buffer
        }
        if( !prop.parse(nm, nm_end - nm, val, val_end - val, not_for_db) ) {
            break;
        }
        nm = val_end + 1;
    }
    return !prop.hasError();
}

bool sUsr::propSet(const char * srcbuf, idx len, sStr & log, sVec<sHiveId> * new_ids, sVec<sHiveId> * updated_ids, sDic<sHiveId> * new_ids_map /* = 0 */)
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

bool sUsr::propSet(const sVar & form, sStr & log, sVec<sHiveId>* new_ids /* = 0 */, sVec<sHiveId>* updated_ids /* = 0 */, sDic<sHiveId> * new_ids_map /* = 0 */) const
{
    TProp prop(*this, log);
    if( propFormParse(form, prop, false) ) {
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
    return op ? sUsrType2::ensure(*this, op->type) : 0;
}

void sUsr::cacheRemove(const sHiveId & objHiveId) const
{
    if( m_ObjPermission.get() ) {
        // remove this id from cache
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
    { // expected first 4 columns in that order:
        eColObjId = 0, eColType, eColPermission, eColFlags, eColViewName
    };
    for(idx r = 0; r < tbl.rows; ++r) {
        udx flags = tbl.uval(r, eColFlags, eFlagNone);
        if( flags & (eFlagOnHold | eFlagRevoked) ) {
            continue;
        }
        // TODO: support domain_id and ion_id
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
    static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
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
                            if( use_type_upobj ) {
                                tid.set(db().resultUValue(4), db().resultUValue(5), 0);
                            } else {
                                tid.set(&nm[1], db().resultUValue(7), 0);
                            }
                            if( tid && !cacheObj(id, &tid, eFlagNone, ePermNone) ) {
                                return false;
                            }
                        } else if( strcasecmp(&nm[1], "acl") == 0 ) {
                            if( !cacheObj(id, 0, db().resultUValue(4), db().resultUValue(5)) ) { // TODO cols 6,7 are view ID
                                return false;
                            }
                        } else if( strcasecmp(&nm[1], "perm") == 0 ) {
                            const char * path = tmp.printf(0, "1.%"DEC, ++pid);
                            tmp.add0(2);
                            const char * val;
                            if( use_type_upobj ) {
                                sHiveId vw(db().resultUValue(7), db().resultUValue(8));
                                val = tmp.printf("%"UDEC",%s,", db().resultUValue(4), vw ? vw.print() : "");
                                permPrettyPrint(tmp, db().resultUValue(6), db().resultUValue(5));
                            } else {
                                const udx vw = db().resultUValue(5);
                                val = tmp.printf("%"UDEC",%s,", db().resultUValue(4), vw ? db().resultValue(5) : "");
                                permPrettyPrint(tmp, db().resultUValue(7), db().resultUValue(6));
                            }
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

udx sUsr::objsLowLevel(const char * type_names, const char * obj_filter_sql, const char * prop_filter_sql, const char * prop_name_csv, bool permissions, const udx start, const udx count, sUsrObjRes * res /* = 0 */, udx * total_qty /* = 0 */) const
{
    if( !res && !total_qty ) {
        return 0;
    }
    static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
    SRCHDBG("SEARCH QUERY %s%s FROM type(s): '%s' WHERE [[%s]] AND [[%s]] LIMIT %"UDEC", %"UDEC" with%s total\n", prop_name_csv ? prop_name_csv : "NULL", permissions ? " +flag:_perm" : "", type_names, obj_filter_sql ? obj_filter_sql : "", prop_filter_sql ? prop_filter_sql : "", start, count, total_qty ? "" : "out");
    std::auto_ptr<sSql::sqlProc> p(getProc(use_type_upobj ? "sp_obj_get_v4_1" : "sp_obj_get_v3"));
    if( total_qty ) {
        *total_qty = 0;
    }
    if( p.get() ) {
        // unique-ified lists
        sStr filter00, typeids;
        sVec<const sUsrTypeField*> fields;
        sDic<bool> props;
        if( use_type_upobj && type_names && type_names[0] && strcmp(type_names, "*") != 0 ) {
            sVec<const sUsrType2 *> tout;
            sVec< sHiveId > tids;
            sUsrType2::find(*this, & tout, type_names, 0, 0, 0, true); // fetch types lazily - we only need their IDs
            tids.resize(tout.dim());
            for(idx i = 0; i < tout.dim(); ++i ) {
                tids[i] = tout[i]->id();
            }
            if( tids.dim() ) {
                sSql::exprInList(typeids, "o.objTypeDomainID", "o.objTypeID", tids, false);
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
        if( permissions ) {
            filter00.printf("%s", "_perm");
            filter00.add0(1);
        }
        filter00.add0(filter00 ? 2 : 0);
        if( use_type_upobj ) {
            p->Add(typeids.length() ? typeids.ptr() : "");
        } else {
            p->Add(type_names);
        }
        p->Add(obj_filter_sql).Add(prop_filter_sql);
        if( m_AllowExpiredObjects ) {
            if( m_SuperUserMode ) {
                p->Add((idx)-1); // any (expired or unexpired)
            } else {
#if _DEBUG
                fprintf(stderr, "WARNING: sUsr API misuse: fetching expired objects is permitted only in superuser mode\n");
#endif
                p->Add((idx)0); // unexpired only
            }
        } else {
            p->Add((idx)0); // unexpired only
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
                    for(idx i = 0; i < props.dim(); ++i) {
                        const char * nm = (const char*)props.id(i);
                        if( nm && nm[0] != '_' ) {
                            propsSql.printf(",%s", nm);
                        }
                    }
                    p.reset(getProc(use_type_upobj ? "sp_obj_prop_v2_2" : "sp_obj_prop_v1_1"));
                    if( p.get() ) {
                        if( !use_type_upobj ) {
                            p->Add((char*)0);
                        } else {
                            p->Add(sid);
                        }
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
                            if( propsSql ) {
                                obj->propEval(*res, filter00);
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

udx sUsr::objs2(const char* type_names, sUsrObjRes & res, udx * total_qty, const char* prop, const char* value, const char * prop_name_csv, bool permissions, const udx start, const udx count) const
{
    SRCHDBG("SEARCH QUERY type(s): '%s' --> '%s'[%s]\n", type_names, value ? value : "", prop ? prop : "");
    sStr v00(sMex::fExactSize), p00(sMex::fExactSize);
    sStr o_flt(sMex::fExactSize), v_flt(sMex::fExactSize);
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
                    sHiveId objid(pv);
                    // if obj is 0, we still want to print it to o_flt (if we are filtering by _id == 0, we want to retrieve no objects, not all objects)
                    o_flt.printf(o_flt ? (not_pn ? "AND NOT" : "OR") : "(");
                    objid.printSQL(o_flt, "o");
                }
            } else {
                if( v_flt ) {
                    v_flt.addString(" OR ");
                }
                v_flt.printf("(f.name %s '", not_pn ? "<>" : "=");
                db().protect(v_flt, pn);
                v_flt.shrink00();
                v_flt.printf("'");
                if( pv && pv[0] ) {
                    v_flt.printf(" AND f.value %s"OBJCMP" '", not_pv ? OBJNOT : "");
                    db().protect(v_flt, pv);
                    v_flt.shrink00();
                    v_flt.printf("'");
                }
                v_flt.printf(")");
            }
        } else if( pv && pv[0] && strcmp(pv, "*") != 0 && strcmp(pv, ".*") != 0 ) {
            // !* in prop list means value != ...
            if( v_flt ) {
                v_flt.addString(" OR ");
            }
            v_flt.printf("((f.value %s"OBJCMP" '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("') OR (CHAR(o.domainID USING ASCII) %s"OBJCMP" '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("') OR (o.objID %s"OBJCMP" '", not_pv ? OBJNOT : "");
            db().protect(v_flt, pv);
            v_flt.shrink00();
            v_flt.printf("'))");
        }
    }
    o_flt.printf(o_flt ? ")" : "");
#undef OBJCMP
#undef OBJNOT
    return objsLowLevel(type_names, o_flt, v_flt.ptr(0), prop_name_csv, permissions, start, count, &res, total_qty);
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

void sUsr::propBulk(sVec<sHiveId> & ids, sVarSet & list, const char* view_name, const char* filter00) const
{
    if( ids.dim() ) {
        static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
        sStr idcsv;
        if( use_type_upobj ) {
            idcsv.printf(",");
            sSql::exprInList(idcsv, "domainID", "objID", ids, false);
        } else {
            // truncate list to fit into MEDIUMTEXT storeproc param
            for(idx i = 0; i < ids.dim() && idcsv.length() < ((2 << 24) - 20); ++i) {
                idcsv.printf(",%"UDEC, ids[i].objId());
            }
        }
        const bool hasBrief = filter00 && (sString::compareChoice("_brief", filter00, 0, true, 0, true) != sNotIdx);
        const bool hasSummary = filter00 && (sString::compareChoice("_summary", filter00, 0, true, 0, true) != sNotIdx);
        sStr prp, pkey;
        if( hasBrief || hasSummary ) {
            // expand filter00 with _brief and/or _summary props for given id types
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
                                //utype->props(*this, props, view_name, pp[p]);
                                utype->props(*this, props, pp[p]);
                            }
                            idx cnm = props.colId("name");
                            idx cvrt = props.colId("is_virtual_fg");
                            for(idx r = 0; r < props.rows; ++r) {
                                // skip virtual columns
                                const char * nm = props.val(r, cnm);
                                if( !props.uval(r, cvrt) ) {
                                    uniq.set(nm, sLen(nm) + 1);
                                }
                                if( p > 0 ) { // not _brief (fields for _brief excluded from output unless requested directly
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
        std::auto_ptr<sSql::sqlProc> p(getProc(use_type_upobj ? "sp_obj_prop_list_v2" : "sp_obj_prop_list"));
        if( use_type_upobj ) {
            p->Add(idcsv.ptr(1)).Add(prp);
        } else {
            p->Add(idcsv.ptr(1)).Add(prp).Add((udx) (ePermCanRead | ePermCanBrowse));
        }
        sVarSet vtmp;
        p->getTable(&vtmp);
        // TODO fix data in memory duplication
        for(idx i = 0; i < ids.dim(); ++i) {
            std::auto_ptr<sUsrObj> obj(objFactory(ids[i]));
            if( obj.get() ) {
                obj->propBulk(vtmp, list, view_name, filter00);
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

udx sUsr::removeTrash(sUsrObjRes & res, bool return_total_count /* = false */) const
{
    static sDic<idx> inTrashObjects;
    if( !inTrashObjects.dim() ) {
        // determine trash objects
        sUsrObjRes trash;
        objs2("sysfolder", trash, 0, "name", "Trash", "child");
        // TODO recurr into trashed folders too, but might be too heavy
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
        obj.reset(); // miss
    } else {
        const char * tpnm = obj->getTypeName();
        const sUsrType2 * typ = tpnm ? sUsrType2::ensure(*this, tpnm) : 0;
        if( typ ) {
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
        // ... but if m_AllowExpiredObjects == true, then we allow objects of unknown/removed type - so we can purge them
    }
    return obj.release();
}

sUsrObj* sUsr::objFactory(const sHiveId & id, const sHiveId * ptypeId, udx permission) const
{
    std::auto_ptr<sUsrObj> obj;
    obj.reset(id ? new sUsrObj(*this, id, ptypeId, permission) : 0);
    if( !obj.get() || !obj->Id() ) {
        obj.reset(); // miss
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
        // ... but if m_AllowExpiredObjects == true, then we allow objects of unknown/removed type - so we can purge them
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

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ User Info Handling
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx sUsr::listUsr(sVec < sStr > * userList, idx isgrp, bool allUsr, bool active, const char * search, bool primaryGrpOnly) const
{
    sStr sql("SELECT IF(`type` = 'group', last_name, CONCAT(first_name, ' ', last_name)), groupPath, u.userID"
            " FROM UPUser u JOIN UPGroup g USING(userID) where `type` NOT IN ('system','service')");
    if( active ) {
        sql.printf(" AND u.is_active_fg = TRUE AND g.is_active_fg = TRUE");
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
    // sort groups by ID, but humans by their names
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

idx sUsr::listGrp(sVec < sStr > * userList, idx isgrp, idx usrOnly, const char * search, bool with_system/* = false*/, bool with_service/* = false */) const
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
    // sort groups by ID, but humans by their names
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
        out.addKey(buf.printf(0, "u%"DEC, utbl.ival(ir, utbl.colId("userID"))));
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
                pswd_prev_list += strspn(pswd_prev_list, hash_seps); // skip any whitespace
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
            // skip leaf groups - we express them as direct membership of user in group
            continue;
        }
        const char * name = path;
        for(idx ip = path_len - 2; ip >= 0; ip--) {
            if( path[ip] == '/' ) {
                name = path + ip + 1;
                break;
            }
        }

        out.addKey(buf.printf(0, "g%"DEC, gtbl.ival(ir, gtbl.colId("groupID"))));
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
                    // grandchild - skip
                    continue;
                } else {
                    // branch child
                    out.addValue(buf.printf(0, "$root.groups.g%"DEC, gtbl.ival(jr, gtbl.colId("groupID"))));
                }
            } else {
                // leaf child
                out.addValue(buf.printf(0, "$root.users.u%"DEC, gtbl.ival(jr, gtbl.colId("userID"))));
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

void sUsrObjRes::json(const sUsr & user, const sUsrObjRes::IdIter & it, sJSONPrinter & printer, bool into_object, bool flatten/* = false */, bool upsert/* = false */, const char * upsert_qry/* = 0 */) const
{
    const sHiveId * oid = id(it);
    if( oid ) {
        if( !into_object ) {
            printer.startObject();
        }
        // FIXME this is inefficient; need a way to generate tree without intermediate varset (and better yet, skip the tree too)
        sVarSet tree_table;
        sStr upsert_qry_buf;
        const TObjProp * prop = get(it);
        idx iprop_perm = sIdxMax;
        for(idx p = 0; prop && p < prop->dim(); ++p) {
            const char * prop_name = (const char *) prop->id(p);
            if( prop_name && strcmp(prop_name, "_perm") == 0 ) {
                // special handling for _prop for ION json format compatibility
                iprop_perm = sMin<idx>(iprop_perm, p);
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
                    // $upsert_qry(<query language expression that specifies the object uniquely>)
                    // we assume that the object will be uniquely specified by its type and is_key_fg properties
                    upsert_qry_buf.printf(0, "$upsert_qry(%s)", upsert_qry);
                    printer.addValue(upsert_qry_buf.ptr());
                } else {
                    // $upsert() - upsert object uniquely specified by type name and globally single-valued key fields
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
                // special handling for _prop for ION json format compatibility
                printer.addKey("_perm");
                printer.startArray();

                const TPropTbl * tbl = get(*prop, "_perm");
                while( tbl ) {
                    const char * grp_perm_pretty_print = getValue(tbl);
                    udx num_group = 0;
                    int num_group_nbytes = 0;
                    if( sscanf(grp_perm_pretty_print, "%"UDEC",,%n", &num_group, &num_group_nbytes) ) {
                        permPretty2JSON(printer, num_group, 0, grp_perm_pretty_print + num_group_nbytes);
                    }
                    tbl = getNext(tbl);
                }
                printer.endArray();
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
        // don't double-count if the table was already added (and not yet deleted)
        _table_cnt++;
        resetFirstLast();
    }
    opt->exists = true;
    return opt->get();
}
bool sUsrObjRes::add(TObjProp & obj, const char * prop, const char * path, const idx path_len, const char * value, const idx value_len)
{
    // allocate new element
    const idx vnew_offset = _buf.mex()->add((const char*) 0, sizeof(TPropTbl));
    if( vnew_offset != sNotIdx ) {
        TPropTbl * vnew = (TPropTbl *) _buf.mex()->ptr(vnew_offset);
        vnew->next = 0; // tail
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
                // set() calls may have moved vnew in memory
                vnew = (TPropTbl *) _buf.mex()->ptr(vnew_offset);
                vnew->path = vp;
                vnew->value = vv;
                const idx nmlen = sLen(prop) + 1;
                idx * tail = obj.get(prop, nmlen);
                if( tail ) {
                    // find linked list end
                    do {
                        TPropTbl * tbl = (TPropTbl *) _buf.mex()->ptr(*tail);
                        tail = &tbl->next;
                    } while( *tail > 0);
                } else {
                    // make new head node
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
