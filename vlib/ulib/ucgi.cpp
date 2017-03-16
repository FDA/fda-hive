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
#include <ulib/ucgi.hpp>
#include <ulib/uobj.hpp>
#include <ulib/ufile.hpp>
#include <ulib/ufolder.hpp>
#include <ulib/uquery.hpp>
#include <ulib/utype2.hpp>
#include <ulib/upropset.hpp>
#include <ulib/honeycomb.hpp>

#include <slib/core/algo.hpp>
#include <slib/std/cryptocipher.hpp>
#include "uperm.hpp" // TODO remove this tmp include
#include <qlib/QPrideBase.hpp>

#include <slib/std/url.hpp>
#include <slib/std/file.hpp>

#include <slib/utils/json/printer.hpp>

#include <memory>
#include <ctype.h>

#ifndef WIN32
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

using namespace slib;

static
bool decodeCookie(sUsr & usr, const char* s, udx & key, idx & key2, udx& uid, sStr& sid, udx expire_secs)
{
    sStr buf, tmp;
    if( s && *s ) {
        const char* u = strchr(s, '@');
        sMex cryptbin;
        idx b64len;
        if( u ) {
            sid.printf(0, "%s", &u[1]);
            b64len = u - s;
        } else {
            b64len = sLen(s);
        }
        if( b64len ) {
            sString::decodeBase64(&cryptbin, s, b64len);
        }
        if( cryptbin.pos() ) {
            if( sBlockCrypto::decrypt(&buf, sBlockCrypto::eAES256_HMACSHA256, cryptbin.ptr(), cryptbin.pos(), usr.getKey(), sLen(usr.getKey())) > 0 ) {
                buf.add0(2);
            }
        }
    }
    udx ok = 0;
    if( buf ) {
        sString::searchAndReplaceSymbols(&tmp, buf.ptr(), 0, "|", 0, 0, true, true, true, true);
        idx keylen = 0;
        udx chksum = 0, timestamp = 0;
        char* p = tmp.ptr();
        for(idx i = 0; p; p = sString::next00(p), ++i) {
            if( i == 0 ) {
                ok += sscanf(p, "%"UDEC, &key) == 1 ? 1 : 0;
            } else if( i == 1 ) {
                ok += sscanf(p, "%"DEC, &key2);
            } else if( i == 2 ) {
                ok += sscanf(p, "%"UDEC, &uid);
            } else if( i == 3 ) {
                ok += sscanf(p, "%"UDEC, &timestamp);
            } else if( i == 4 ) {
                keylen = p - tmp.ptr() - 1;
                ok += sscanf(p, "%"UDEC, &chksum);
            }
        }
        time_t t = time(0);
        udx now = mktime(gmtime(&t));
        if( (now < timestamp) || (expire_secs && (now - timestamp) > expire_secs) ) {
            uid = 0;
            ok--;
        }
        udx chksum1 = sAlgo::algo_murmurHash64(buf.ptr(), keylen, 32, 0, timestamp);
        if( chksum1 == chksum ) {
            ok++;
        }
    }
    return ok == 6;
}

static
void setUserCookie(sCGI & cgi, sUsr & usr, sStr & sid)
{
    sStr buf;
    cgi.cookieSet("sessionID", "%s", usr.encodeSID(sid, buf));
    cgi.cookieSet("userName", "%s", usr.Name());
    const char* email = usr.Email();
    cgi.cookieSet("email", "%s", email ? email : "");
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Initializations
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

bool sUsrCGI::OnCGIInit(void)
{
    m_apiVersion = pForm->uvalue("api", sUdxMax);
    // this cookie is _always_ present to track sessions even for guests!
    sStr sid("%s", pForm->value("sessionID", ""));
    const char * ip = pForm->value("ADDR");
    udx key = 0, uid = 0;

    // FIXME - temporary switch for new ulib code paths with UPObj-ification of types
    if( const char * useTypeDomainIdStr = pForm->value("useTypeUPObj") ) {
        sStr buf("%i", sString::parseBool(useTypeDomainIdStr)); // don't trust the string value - convert to "0" or "1"
        setenv("TYPE_UPOBJ", buf.ptr(), 1);
    }

    m_SID.destroy();
    {
        sQPrideBase * qp = m_User.QPride();
        udx cookie_expires = 1L * 24 * 60 * 60;
        if( qp ) {
            cookie_expires = qp->cfgInt(0, "user.Auth_ExpirationSecs", cookie_expires);
        }
        idx rnd = 0;
        decodeCookie(m_User, sid.ptr(), key, rnd, uid, m_SID, cookie_expires);
        m_User.session(key, uid, rnd, ip);
    }
    setUserCookie(*this, m_User, m_SID);
    udx userPerspective = pForm->uvalue("userPerspective", 0);
    if( m_User.isAdmin() && userPerspective ) {
        m_User.init(userPerspective);
    }
#if _LOG_REQUEST_TO_AUDIT_LOG
        sStr buf;
        for(idx i = 0; i < pForm->dim(); ++i) {
            const char * nm = (const char *)pForm->id(i);
            buf.printf("%s='%s'; ", nm, pForm->value(nm));
        }
        m_User.audit(sUsr::eUserAuditLogin, __func__, "%s", buf.ptr());
#endif
    return TParent::OnCGIInit();
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Login/Logout
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void sUsrCGI::logout(void)
{
    m_User.logout(pForm->value("ADDR"));
    setUserCookie(*this, m_User, m_SID);
}

void sUsrCGI::login(void)
{
    const char * email = pForm->value("login");
    const char * pswd = pForm->value("pswd");
    const char * pswd2 = pForm->value("pswd2");
    idx logCount = -1;

    sQPrideBase * qp = m_User.QPride();
    sUsr::ELoginResult eres = sUsr::eUserNotSet;
    sUsr tokenized;
    do {
        udx token = 0;
        if( sLen(pswd2) ) {
            udx uid = 0;
            idx rnd = 0;
            if( !decodeCookie(m_User, pswd2, token, rnd, uid, m_SID, 0) || !tokenized.init(uid) ) {
                redirectURL.cut(0);
            }
            email = tokenized.Email();
        }
        if( email && (pswd || token) ) {
            headerSet("Status", "401");
            eres = m_User.login(email, pswd, token, pForm->value("ADDR"), &logCount);
            switch(eres) {
                case sUsr::eUserOperational:
                    headerSet("Status", "200");
                    if( logCount == 0 && !token ) {
                        // initial password was set by admin, user must now change it
                        warning("You must change your password now.");
                        sStr t("pswdSet&followTo=%s", redirectURL.ptr());
                        redirectURL.replace(t);
                        cookieSet("preset", "%"UDEC, m_User.addPasswordResetID(email));
                        cookieSet("emailAct", "%s", email);
                    }
                    cookieSet("last_login", "%s", email);
                    break;
                case sUsr::eUserEmailNotValidated:
                    redirectURL.cut(0);
                    outSection("user_email_inactive");
                    warning("Email address on the account is not validated. See web page for more information.");
                    cookieSet("emailAct", "%s", email);
                    break;
                case sUsr::eUserBlocked:
                    redirectURL.cut(0);
                    outSection("user_account_inactive");
                    warning("Your account is deactivated. Request activation on web page.");
                    cookieSet("emailAct", "%s", email);
                    break;
                case sUsr::eUserAccountExpired:
                    redirectURL.cut(0);
                    outSection("user_account_expired");
                    warning("Your account has expired. Request activation on web page.");
                    cookieSet("emailAct", "%s", email);
                    break;
                case sUsr::eUserPswdExpired:
                    redirectURL.printf(0, "user_pswd");
                    warning("Your password has expired. You must change your password now.");
                    cookieSet("emailAct", "%s", email);
                    break;
                case sUsr::eUserNotFound:
                    cookieSet("followTo", redirectURL.ptr());
                    error("Email or password is not recognized");
                    redirectURL.cut(0);
                    break;
                case sUsr::eUserNotSet:
                case sUsr::eUserInternalError:
                    redirectURL.printf(0, "login");
                    error("Authentication failed. See web page.");
                    redirectURL.cut(0);
                    break;
            }
        } else {
            if( redirectURL ) {
                cookieSet("followTo", redirectURL.ptr());
                redirectURL.cut(0);
            }
        }
    } while(false);

    if( tokenized.Id() ) {
        cookieSet("exname", "%s", tokenized.Name());
        cookieSet("exemail", "%s", tokenized.Email());
    }
    setUserCookie(*this, m_User, m_SID);

    if( !raw ) {
        sStr login_opts, tmp;
        if( qp ) {
            qp->cfgStr(&login_opts, 0, "user.LoginTmpltList");
        }
        if( !login_opts ) {
            login_opts.printf("user_login_pswd");
        }
        sString::searchAndReplaceSymbols(&tmp, login_opts, 0, ";,", 0, 0, true, true, true, true);
        for(char* p = tmp.ptr(); p; p = sString::next00(p)) {
            outSection(p);
        }
    } else {
        if( eres == sUsr::eUserNotSet ) {
            headerSet("Status", "401");
        }
    }
}

void sUsrCGI::userEmailAuth()
{
    raw = 1;
    const char * email = pForm->value("email");
    if( !email || !email[0] ) {
        headerSet("Status", "400");
        error("missing url parameter 'email'");
    } else {
        switch( m_User.token(email, dataForm) ) {
            case sUsr::eUserOperational:
                break;
            case sUsr::eUserEmailNotValidated:
                headerSet("Status", "401");
                error("Email is not validated");
                break;
            case sUsr::eUserBlocked:
                headerSet("Status", "401");
                error("User inactive");
                break;
            case sUsr::eUserAccountExpired:
                headerSet("Status", "401");
                error("Account expired, reactivate");
                break;
            case sUsr::eUserPswdExpired:
                headerSet("Status", "401");
                error("Password expired, change");
                break;
            case sUsr::eUserNotSet:
            case sUsr::eUserInternalError:
            case sUsr::eUserNotFound:
                headerSet("Status", "404");
                error("Email not found");
                break;
        }
    }
}

void sUsrCGI::batch(void)
{
    m_User.batch(pForm->value("ADDR"));
    setUserCookie(*this, m_User, m_SID);
    error("%s", m_User.err.ptr());
    return;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Account settings
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void sUsrCGI::userResendEmailVerify()
{
    const char* email = pForm->value("emailAct", pForm->value("email"));
    sStr baseURL;
    selfURL(baseURL);
    if( email && email[0] ) {
        if( m_User.sendEmailValidation(baseURL, email) ) {
            alert("Email was sent to %s.", email);
        } else {
            if( !m_User.err ) {
                m_User.err.printf(0, "Unknown error has occurred.");
            }
            error("%s", m_User.err.ptr());
        }
    } else {
        error("Resend failed");
    }
}

void sUsrCGI::userVerifyEmail()
{
    const char* email = pForm->value("emailAct");
    sStr baseURL;
    selfURL(baseURL);
    if( email && email[0] ) {
        if( m_User.verifyEmail(baseURL, email) ) {
            alert("Email was successfully verified. %s", m_User.err.ptr());
            redirectURL.printf(0, "login");
        } else {
            if( !m_User.err ) {
                m_User.err.printf(0, "Unknown error has occurred.");
            }
            error("%s", m_User.err.ptr());
        }
    } else {
        error("Verification failed");
    }
}

void sUsrCGI::userResendAccountActivate()
{
    const char* email = pForm->value("emailAct", pForm->value("email"));
    sStr baseURL;
    selfURL(baseURL);
    if( email && email[0] ) {
        if( m_User.sendAccountActivation(baseURL, email) ) {
            alert("Email was sent to administrator.");
        } else {
            if( !m_User.err ) {
                m_User.err.printf(0, "Unknown error has occurred.");
            }
            error("%s", m_User.err.ptr());
        }
    } else {
        error("Resend failed");
    }
}

void sUsrCGI::userAccountActivate()
{
    const char* email = pForm->value("emailAct");
    if( m_User.isAdmin() ) {
        sStr baseURL;
        selfURL(baseURL);
        if( email && email[0] ) {
            if( m_User.accountActivate(baseURL, email) ) {
                alert("Account was successfully activated. %s", m_User.err ? m_User.err.ptr() : "");
            } else {
                if( !m_User.err ) {
                    m_User.err.printf(0, "Unknown error has occurred.");
                }
                error("%s", m_User.err.ptr());
            }
        } else {
            error("Activation failed");
        }
        redirectURL.printf(0, "home");
    } else {
        error("You must be logged in as administrator to perform this action.");
        redirectURL.printf(0, "logout&follow=login%%26follow%%3DuserV3%%2526e%%253D%s", email);
    }
}

void sUsrCGI::userGroupActivate()
{
    udx groupId = pForm->uvalue("ugrpid");
    if( m_User.isAdmin() ) {
        if( !m_User.groupActivate(groupId) ) {
            if( !m_User.err ) {
                m_User.err.printf(0, "Unknown error has occurred.");
            }
            error("%s", m_User.err.ptr());
        } else if( m_User.err ) {
            alert("%s", m_User.err.ptr());
        }
    } else {
        error("You must be logged in as administrator to perform this action.");
        redirectURL.printf(0, "logout&follow=login%%26follow%%3DuserV4%%2526grpid%%253D%"UDEC, groupId);
    }
}

void sUsrCGI::userMgr()
{
    if( m_User.isAdmin() ) {
        outSection("user_mgr");
    } else {
        error("You must be logged in as administrator to access this page.");
        redirectURL.printf(0, "logout&follow=login%%26follow%%3Dmgr");
    }
}

void sUsrCGI::contact()
{
    const char * email = pForm->value("emailAct", pForm->value("email"));
    const char * subj = pForm->value("subject");
    const char * body = pForm->value("message");

    bool ok = true;
    if( !email || !email[0] ) {
        error("Email address is required to submit a request.");
    }
    if( !subj || !subj[0] ) {
        error("Please specify a brief description of the request on the subject line.");
    }
    if( !body || !body[0] ) {
        error("Message body should contain the description of the request.");
    }
    if( ok ) {
        if( !m_User.contact(email, subj, body) ) {
            error("%s", m_User.err.ptr());
        } else {
            alert("The message was successfully sent");
        }
    }
}

void sUsrCGI::userForgot()
{
    const char * email = pForm->value("login");
    if( email && email[0] ) {
        sStr baseURL;
        selfURL(baseURL);
        if( m_User.sendForgotten(baseURL, email) ) {
            alert("Email was sent to %s", email);
        } else {
            if( !m_User.err ) {
                m_User.err.printf(0, "Uknown error has occured.");
            }
            error("%s", m_User.err.ptr());
            redirectURL.cut(0);
        }
    } else {
        redirectURL.cut(0);
    }
    outSection("user_forgot");
}

void sUsrCGI::passwordReset(void)
{
    const char* email = pForm->value("login");
    udx pswd_reset_id = pForm->uvalue("pswd");
    const char* mod = pForm->value("newpass1");
    const char* mod1 = pForm->value("newpass2");
    udx expires = pForm->uvalue("x", 0);

    email = (email && email[0]) ? email : 0;

    if( email && (expires > 0 && expires < (udx)time(0)) ) {
        error("This URL has expired.");
        redirectURL.printf(0, "forgot");
    } else if( mod || mod1 ) {
        if( !m_User.passwordReset(email, pswd_reset_id, mod, mod1) ) {
            error("%s", m_User.err.ptr());
            redirectURL.cut(0);
        }
    } else {
        redirectURL.cut(0);
    }
    if( !redirectURL && email ) {
        if( pswd_reset_id ) {
            // On first login, user is redirected to cmd=pswdSet with "preset" cookie
            // already filled in, and no pswd cgi param. In this case, we do not want
            // to overwrite the cookie, since empty "preset" would result in password
            // change UI instead of password reset UI.
            cookieSet("preset", "%"UDEC, pswd_reset_id);
        }
        if( expires ) {
            cookieSet("xp", "%"UDEC, expires);
        }
        cookieSet("emailAct", "%s", email);
    }
    outSection("user_pswd");
}

void sUsrCGI::passwordChange(void)
{
    const char* email = pForm->value("login");
    if( email && !email[0] ) {
        email = 0;
    }
    const char * password = pForm->value("pswd");
    if( password && !*password ) {
        password = 0;
    }
    const char * newpass1 = pForm->value("newpass1");
    if( newpass1 && !*newpass1 ) {
        newpass1 = 0;
    }
    const char * newpass2 = pForm->value("newpass2");
    if( newpass2 && !*newpass2 ) {
        newpass2 = 0;
    }

    sVec<idx> no_new_groups;
    sStr baseURL;
    selfURL(baseURL);
    m_User.update(false, email, password, newpass1, newpass2, sUsr::eUserOperational, 0, 0, no_new_groups, -1, -1, baseURL.ptr());
    if( m_User.err.length() ) {
        error("%s", m_User.err.ptr());
        redirectURL.cut(0);
        cookieSet("emailAct", "%s", email);
        outSection("user_pswd");
        return;
    } else {
        warning("User password has been updated.");
        pForm->inp("pswd", newpass1 ? newpass1 : password);
        return login();
    }
}

void sUsrCGI::userReg(void)
{
    cookieSet("newreg", "1");
    outSection("user");
    return;
}

void sUsrCGI::userSet(bool show)
{
    const char * firstName = 0;
    const char * lastName = 0;
    if( !show ) {
        const char* email = pForm->value("login");
        if( email && !email[0] ) {
            email = 0;
        }
        sVec<idx> groups;
        sString::scanRangeSet(pForm->value("groups", ""), 0, &groups, 0, 0, 0);

        firstName = pForm->value("firstName");
        if( firstName && !firstName[0] ) {
            firstName = 0;
        }
        lastName = pForm->value("lastName");
        if( lastName && !lastName[0] ) {
            lastName = 0;
        }
        const char * password = pForm->value("pswd");
        if( password && !*password )
            password = 0;
        const char * newpass1 = pForm->value("newpass1");
        if( newpass1 && !*newpass1 )
            newpass1 = 0;
        const char * newpass2 = pForm->value("newpass2");
        if( newpass2 && !*newpass2 )
            newpass2 = 0;
        idx statusNeed = sUsr::eUserOperational; //pForm->ivalue("statusNeed",-1);
        idx softExpiration = pForm->ivalue("softExpiration", -1);
        idx hardExpiration = pForm->ivalue("hardExpiration", -1);
        bool isnew = pForm->boolvalue("newreg", false);

        sStr baseURL;
        selfURL(baseURL);
        m_User.update(isnew, email, password, newpass1, newpass2, statusNeed, firstName, lastName, groups, softExpiration, hardExpiration, baseURL.ptr());
        if( m_User.err.length() ) {
            error("%s", m_User.err.ptr());
            redirectURL.cut(0);
        } else {
            if( isnew ) {
                redirectURL.cut(0);
                outSection("user_registered");
                return;
            }
            if( (firstName && firstName[0]) || (lastName && lastName[0]) ) {
                warning("%s %s profile has been modified!", firstName ? firstName : "", lastName ? lastName : "");
            } else if( email && email[0]) {
                warning("%s profile has been modified!", email);
            } else {
                warning("Profile has been modified!");
            }
            pForm->inp("pswd", newpass1 ? newpass1 : password);
            return login();
        }
    } else {
        firstName = m_User.firstName();
        lastName = m_User.lastName();
    }
    cookieSet("fn", firstName ? firstName : "");
    cookieSet("ln", lastName ? lastName : "");
    cookieSet("ugrp", m_User.groupList(false));
    cookieSet("ugrp_all", m_User.groupList(true));
    if( !m_User.Id() || m_User.isGuest() ) {
        cookieSet("newreg", "1");
    }
    outSection("user");
    return;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Object Info
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

static void objsAddInfoRows(sVarSet &x, udx total, udx start)
{
    x.addRow().addCol("info").addCol("total").addCol("").addCol(total);
    x.addRow().addCol("info").addCol("start").addCol("").addCol(start);
}

struct typeTreeFindParam {
    const sUsr * user;
    sTxtTbl * tbl;
    bool force_print;
    sHiveId search_id;
    regex_t * search_re;
    sDic<idx> printed_ids;
};

static void typeTreeFindCb(const sUsrType2 * utype, const sUsrType2 * recurse_start, idx depth_from_start, void * param_)
{
    sStr csv_buf;
    typeTreeFindParam * param = static_cast<typeTreeFindParam*>(param_);
    sTxtTbl * tbl = param->tbl;

    if( !param->force_print ) {
        if( param->search_id ) {
            // if search key looks like an ID, skip types not matching the ID (or the object ID part, if search key does not specify domain)
            if( utype->id() != param->search_id && (param->search_id.domainId() != 0 || param->search_id.objId() != utype->id().objId()) ) {
                return;
            }
        } else if( param->search_re ) {
            const char * name = utype->name() ? utype->name() : sStr::zero;
            const char * title = utype->title() ? utype->title() : sStr::zero;
            const char * descr = utype->description() ? utype->description() : sStr::zero;
            if( regexec(param->search_re, name, 0, 0, 0) != 0 && regexec(param->search_re, title, 0, 0, 0) != 0 && regexec(param->search_re, descr, 0, 0, 0) != 0) {
                return;
            }
        }
    }

    *param->printed_ids.set(&utype->id(), sizeof(utype->id())) = 1;

    // "main" inheritance path
    sDic<idx> parent_ids;
    for(const sUsrType2 * t = utype; t; ) {
        sHiveId id(t->id());
        idx * pseen = parent_ids.get(&id, sizeof(id));
        if( pseen && *pseen ) {
            // avoid infinite loop in case of circular inheritance (shouldn't happen, but might)
            break;
        }
        *parent_ids.set(&id, sizeof(id)) = 1;
        const sUsrType2 * par = 0;
        // pick the first non-virtual parent, or if none, just the first parent
        for(idx iparent = 0; iparent < t->dimParents(); iparent++) {
            if( iparent == 0 ) {
                par = t->getParent(iparent);
            } else if( par->isVirtual() && !t->getParent(iparent)->isVirtual() ) {
                par = t->getParent(iparent);
                break;
            }
        }
        t = par;
    }
    sStr path_buf;
    for(idx ipath = parent_ids.dim() - 1; ipath >= 0; ipath--) {
        sHiveId * id = static_cast<sHiveId*>(parent_ids.id(ipath));
        path_buf.addString("/");
        id->print(path_buf);
        // ensure parents of current type get printed first
        if( !param->printed_ids.get(id, sizeof(*id)) ) {
            const sUsrType2 * par = sUsrType2::ensure(*param->user, *id, false, true);
            if( par ) {
                bool force_print = param->force_print;
                param->force_print = true;
                typeTreeFindCb(par, 0, 0, param);
                param->force_print = force_print;
            }
        }
    }

    tbl->addCell(utype->id());
    tbl->addCell(sString::escapeForCSV(csv_buf, utype->name()));
    tbl->addCell(sString::escapeForCSV(csv_buf, utype->title()));
    tbl->addBoolCell(utype->isVirtual());
    tbl->addCell(sString::escapeForCSV(csv_buf, utype->description()));

    // parents
    sVec<sHiveId> ids;
    sStr ids_buf;
    for(idx iparent = 0; iparent < utype->dimParents(); iparent++) {
        *ids.add(1) = utype->getParent(iparent)->id();
    }
    sHiveId::printVec(ids_buf, ids);
    tbl->addCell(sString::escapeForCSV(csv_buf, ids_buf));

    // includes
    ids.cut(0);
    ids_buf.cut0cut();
    for(idx iinc = 0; iinc < utype->dimIncludes(); iinc++) {
        *ids.add(1) = utype->getInclude(iinc)->id();
    }
    sHiveId::printVec(ids_buf, ids);
    tbl->addCell(sString::escapeForCSV(csv_buf, ids_buf));

    // "main" inheritance path
    tbl->addCell(sString::escapeForCSV(csv_buf, path_buf));

    tbl->addEndRow();
}

void sUsrCGI::typeTree(void)
{
    sTxtTbl tbl;
    tbl.initWritable(7, sTblIndex::fTopHeader);
    tbl.addCell("id");
    tbl.addCell("name");
    tbl.addCell("title");
    tbl.addCell("is_virtual_fg");
    tbl.addCell("description");
    tbl.addCell("parents");
    tbl.addCell("includes");
    tbl.addCell("path");
    tbl.addEndRow();

    regex_t search_re;
    typeTreeFindParam param;
    param.user = &m_User;
    param.force_print = false;
    param.tbl = &tbl;
    param.search_re = 0;
    if( const char * search = pForm->value("search") ) {
        param.search_id.parse(search);
        if( !param.search_id ) {
            if( regcomp(&search_re, search, REG_EXTENDED | REG_NOSUB | REG_ICASE) == 0 ) {
                param.search_re = &search_re;
            }
        }
    }
    sUsrType2::find(m_User, 0, pForm->value("type", ".*"), typeTreeFindCb, &param, 0, true); // find types lazily - we only need their hierarchy info
    tbl.finish();
    tbl.printCSV(dataForm);
    if( param.search_re ) {
        regfree(param.search_re);
    }
}

void sUsrCGI::objs(void)
{
    const char* type_name = pForm->value("type");
    const char* props = pForm->value("prop");
    const char* propN = pForm->value("prop_name");
    const char* propV = pForm->value("prop_val");
    const char* search = pForm->value("search");
    const bool show_info = pForm->boolvalue("info");
    const udx from = pForm->uvalue("start", 0);
    const udx cnt = pForm->uvalue("cnt", 1000);
    udx qty = cnt;
    const bool showTrash = pForm->boolvalue("showTrashed", false) || pForm->boolvalue("showtrash", false);
    const char * pIds = pForm->value("parIds");
    const char * pProp = pForm->value("parP");
    const char * pVal = pForm->value("parV");

    sStr prop_name, filter;
    if( pIds && pIds[0] && pProp && pProp[0] ) {
        sVec<sHiveId> pis;
        sHiveId::parseRangeSet(pis, pIds);
        pVal = (pVal && pVal[0]) ? pVal : "_id";
        for(idx i = 0; i < pis.dim(); ++i) {
            std::auto_ptr<sUsrObj> obj(m_User.objFactory(pis[i]));
            if( obj.get() ) {
                sVarSet pvs;
                obj->propGet(pProp, pvs);
                for(idx r = 0; r < pvs.rows; ++r) {
                    const char * pv = pvs.val(r, 0);
                    if( pv && pv[0] ) {
                        filter.printf(",%s", pv);
                        prop_name.printf(",%s", pVal);
                    }
                }
            }
        }
        if(!filter) {
            qty = 0; // empty result
        }
    }
    sStr typeBuf;
    if( search ) {
        const char * p = search;
        do {
            // search is a comma-delimited sequence of entries of the following form:
            // 1. optional whitespace, "[type:", type name, "]", optional whitespace (e.g. "[type:u-file]"); or
            // 2. a string (for matching a property value)
            const char * non_space = p;
            const char * comma = 0;
            const char * type_name_end = 0, * type_name_start = 0;

            for(; isspace(*non_space); non_space++);
            if( strncmp(non_space, "[type:", 6) == 0 ) {
                type_name_start = non_space + 6;
                type_name_end = strchr(type_name_start, ']');
            }

            if( type_name_start && type_name_end ) {
                typeBuf.addString(type_name_start, type_name_end - type_name_start);
                type_name = typeBuf.ptr(0);
                for(comma = type_name_end + 1; isspace(*comma); comma++);
            } else {
                comma = strchr(p, ',');
                prop_name.addString(",*", 2);
                filter.addString(",", 1);
                if( !comma ) {
                    filter.addString(p);
                } else if( comma != p ) {
                    filter.addString(p, comma - p);
                }
            }
            p = (comma && *comma == ',') ? comma + 1 : comma;
        } while( p && *p );
    }
    if( propN && propN[0] ) {
        prop_name.printf(",%s", propN);
    }
    if( propV && propV[0] ) {
        filter.printf(",%s", propV);
    }
    sUsrObjRes v;
    udx total_qty = 0, fstart = from;
    do {
        // collect full page (qty)
        if( qty ) {
            const idx r = v.dim();
            const idx nq = qty - v.dim();
            m_User.objs2(type_name, v, (show_info && (fstart == from)) ? &total_qty : 0, prop_name ? prop_name.ptr(1) : 0, filter ? filter.ptr(1) : 0, props, pForm->boolvalue("perm"), fstart, nq);
            if( r == v.dim() ) {
                // no new items
                break;
            }
            fstart += nq;
        }
        if( !showTrash && pIds ) {
            total_qty -= m_User.removeTrash(v);
        }
    } while( (udx)(v.dim()) < qty );
    if( !showTrash && !pIds ) {
        total_qty -= m_User.removeTrash(v, true);
    }

    sJSONPrinter json_printer;
    const bool is_json = strcasecmp("json", pForm->value("mode", "")) == 0;
    if( is_json ) {
        json_printer.init(&dataForm);
        json_printer.startObject();
        json_printer.addKey("objs");
    }
    propGet(&v, pForm->value("view"), props, is_json ? &json_printer : 0);
    if( show_info ) {
        if( strcasecmp("csv", pForm->value("mode", "")) == 0 ) {
            sVarSet x;
            objsAddInfoRows(x, total_qty, from);
            x.printCSV(dataForm, 0);
        } else if( is_json ) {
            json_printer.addKey("info");
            json_printer.startObject();
            json_printer.addKey("total");
            json_printer.addValue(total_qty);
            json_printer.addKey("start");
            json_printer.addValue(from);
            json_printer.addKey("cnt");
            json_printer.addValue(qty);
            json_printer.addKey("page");
            json_printer.addValue(cnt);
        } else {
            dataForm.printf("\ninfo.0.total=%"UDEC"\ninfo.0.start=%"UDEC, total_qty, from);
        }
    }
    if( is_json ) {
        json_printer.finish();
    }
}

#ifndef WIN32
static void rlimitHandler(int sig, siginfo_t *si, void * unused)
{
    printf("Content-type: text/plain\n\nerror : query takes too much time\n");
    exit(-1);
}
#endif

qlang::sUsrEngine * sUsrCGI::queryEngineFactory(idx flags/* = 0 */)
{
    return new qlang::sUsrEngine(m_User, flags);
}

void sUsrCGI::objQry(sVariant * dst, const char * qry_text)
{
    if( !qry_text ) {
        qry_text = pForm->value("qry");
    }
    if( !qry_text )
        return;

    bool silentErr = sString::parseBool(pForm->value("silentErr"));
    const bool is_json = strcasecmp("json", pForm->value("mode", "")) == 0;

    idx ctx_flags = 0;
    if( pForm->boolvalue("_id") ) {
        ctx_flags |= qlang::sUsrContext::fUnderscoreId;
    }

    std::auto_ptr<qlang::sUsrEngine> engine(queryEngineFactory(ctx_flags));
    sVariant local_result;
    sVariant * presult = dst ? dst : &local_result;
    sStr error;

#ifndef WIN32
    // If a query takes >30 sec or >2 GB heap or stack, something is very wrong
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = rlimitHandler;
    sigaction(SIGXCPU, &sa, NULL);

    struct rlimit rlim;
    rlim.rlim_cur = 30; rlim.rlim_max = rlim.rlim_cur + 10;
    setrlimit(RLIMIT_CPU, &rlim);
    rlim.rlim_cur = 1<<31; rlim.rlim_max = rlim.rlim_cur + (1<<29);
    setrlimit(RLIMIT_DATA, &rlim);
    setrlimit(RLIMIT_STACK, &rlim);
#endif

    if( !engine->parse(qry_text, 0, &error) ) {
        if (!silentErr)
            dataForm.printf("error : failed to parse query : %s", error.ptr());
        return;
    }

    if( !engine->eval(*presult, &error) ) {
        if (!silentErr)
            dataForm.printf("error : failed while running query : %s", error.ptr());
        return;
    }

    if( !dst ) {
        if( is_json ) {
            presult->print(dataForm, sVariant::eJSON);
        } else {
            // Default formating: a single object ID as an integer; an object ID list as comma-separated integers; strings and more complex data structures as themselves, with C-style escapes
            dataForm.printf("%s", presult->asString());
        }
    }
}

void sUsrCGI::objDel(void)
{

    sQPrideBase * qp = m_User.QPride();
    idx reqId = qp?qp->reqId:0;

    const char* obj_ids = pForm->value("ids");
    if( obj_ids && obj_ids[0] ) {
        sUsrObjRes parents;
        m_User.objs2("sysfolder,folder", parents, 0, "child", obj_ids);
        sVec<sHiveId> ids;
        sHiveId::parseRangeSet(ids, obj_ids);
        for(idx i = 0; i < ids.dim(); ++i) {
            std::auto_ptr<sUsrObj> obj(m_User.objFactory(ids[i]));
            if( obj.get() && obj->actDelete() ) {
                for(sUsrObjRes::IdIter it = parents.first(); parents.has(it); parents.next(it)) {
                    std::auto_ptr<sUsrObj> p_obj(m_User.objFactory(*parents.id(it)));
                    sUsrFolder * p_src_obj = ((sUsrFolder*) p_obj.get());
                    if(reqId) {
                        p_src_obj->progress_CallbackFunction = sQPrideBase::reqProgressStatic;
                        p_src_obj->progress_CallbackParamObject = qp;
                        p_src_obj->progress_CallbackParamReqID = &reqId;
                    }
                    p_src_obj->detach(*obj.get());
                }
            }
            if(reqId)qp->reqProgress(reqId,2,i,i,ids.dim()); //hardcoded lazyreport seconds (no svc accessible here).
        }
    }
}

void sUsrCGI::objRemove(void)
{
    const char* obj_ids = pForm->value("ids");
    const char* src_id = pForm->value("src");

    sVec<sHiveId> s_ids;
    if( src_id && strcmp(src_id, "root") == 0 ) {
        s_ids.add();
    } else if( src_id && strcmp(src_id, "all") == 0 ) {
        sUsrObjRes res;
        m_User.objs2("sysfolder,folder", res, 0, "child", obj_ids);
        for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
            *(s_ids.add()) = *res.id(it);
        }
    } else if( src_id ) {
        s_ids.add()->parse(src_id);
    }
    if(!s_ids.dim()) {
        s_ids.add();
    }

    sQPrideBase * qp = m_User.QPride();
    idx reqId = qp?qp->reqId:0;

    sVec<sHiveId> ids, copied_ids;
    sHiveId::parseRangeSet(ids, obj_ids);
    sVec<sHiveId> *p_ids = &ids;
    dataForm.printf("{");
    for(idx i = 0; i < s_ids.dim(); ++i) {
        if( !s_ids[i] ) {  // Copy objects that you don't have permissions to their parents to your tree (e.g.) and revoke permissions from old links
            sUsrFolder * _inbox = sSysFolder::Inbox(m_User);
            if( _inbox ) {
                _inbox->attachCopy(p_ids, _inbox, 0, &copied_ids, true);
    //            _inbox->actRemove(p_ids, 0, true, true);
                p_ids = &copied_ids;
                s_ids[i] = _inbox->Id();
                delete _inbox;
            }
        }
        std::auto_ptr<sUsrObj> src_obj(m_User.objFactory(s_ids[i]));
        if( src_obj.get() && src_obj->isTypeOf("(folder|sysfolder)") ) {
            sUsrFolder * p_src_obj = ((sUsrFolder*) src_obj.get());
            if(reqId) {
                p_src_obj->progress_CallbackFunction = sQPrideBase::reqProgressStatic;
                p_src_obj->progress_CallbackParamObject = qp;
                p_src_obj->progress_CallbackParamReqID = &reqId;
            }
            p_src_obj->actRemove(p_ids, &dataForm);
        } else {
            dataForm.printf("\"%s\" : { \"signal\" : \"delete\", \"data\" : { \"error\":\"failed to access source folder\"} } \n,", s_ids[i].print());
        }

        if(reqId)qp->reqProgress(reqId,2,i,i,s_ids.dim()); //hardcoded lazyreport seconds (no svc accessible here).
    }
    dataForm.cut(dataForm.length() - 1);
    dataForm.printf("}");
}

void sUsrCGI::objMove(bool isCopy)
{
    const char* obj_ids = pForm->value("ids");
    const char* src_id = pForm->value("src");
    sHiveId dst_id(pForm->value("dest"));
    sVec<sHiveId> ids;
    sHiveId::parseRangeSet(ids, obj_ids);
    if(ids.dim()) {
        sVec<sHiveId> copied_ids, s_ids, *p_ids;
        p_ids = &ids;
        if( src_id && strcmp(src_id, "all") == 0 ) {
            //if no parental id is provided then perform the same action for all parents;
            sUsrObjRes res;
            m_User.objs2("folder,sysfolder,", res, 0, "child", obj_ids);
            for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
                *(s_ids.add(1)) = *res.id(it);
            }
            if( !s_ids.dim() ) {
                s_ids.add();
            }
        } else if( src_id && strcmp(src_id, "root") == 0 ) {
            s_ids.add();
        } else if( src_id ) {
            s_ids.add()->parse(pForm->value("src"));
        }
        dataForm.printf("{");
        std::auto_ptr<sUsrObj> dst_obj(m_User.objFactory(dst_id));
        if( dst_obj.get() && dst_obj->isTypeOf("(folder|sysfolder)") && ids.dim() ) {
            sUsrFolder * _inbox = sSysFolder::Inbox(m_User);
            sStr ret_buf, failed_Ids;
            bool no_src = false;
            for( idx i = 0 ; i < s_ids.dim() ; ++i ) {
                no_src=false;
                if( !s_ids[i] ) {
                    s_ids[i] = _inbox ? _inbox->Id() : sHiveId();
                    no_src = true;
                }
                std::auto_ptr<sUsrObj> src_obj(m_User.objFactory(s_ids[i]));
                if( src_obj.get() && src_obj->isTypeOf("(folder|sysfolder)") ) {
                    if( isCopy ) {
                        ((sUsrFolder*) src_obj.get())->attachCopy(p_ids, (sUsrFolder*) dst_obj.get(), &dataForm, 0, no_src);
                    } else {
                        ((sUsrFolder*) src_obj.get())->attachMove(p_ids, (sUsrFolder*) dst_obj.get(), &dataForm, no_src);
                    }
                } else {
                    dataForm.printf("\"");
                    s_ids[i].print(dataForm);
                    dataForm.printf("\" : { \"signal\" : \"%s\", \"data\" : { \"error\":\"cannot access source folder \" } }\n,", isCopy ? "copy" : "move");
                }
            }
            delete _inbox;
        }
        else{
            dataForm.printf("\"");
            dst_id.print(dataForm);
            dataForm.printf("\" : { \"signal\" : \"%s\", \"data\" : { \"error\":\"cannot access destination folder\"} }\n", isCopy?"copy":"move" );
        }
    }
    else {
        dataForm.printf("\"");
        dst_id.print(dataForm);
        dataForm.printf("\" : { \"signal\" : \"%s\", \"data\" : { \"error\":\"unknown object\"} }\n", isCopy ? "copy" : "move");
    }
    dataForm.cut(dataForm.length()-1);
    dataForm.printf("}");
}

void sUsrCGI::folderCreate(void)
{
    const char* name = pForm->value("name");
    sHiveId id(pForm->value("ids"));
    sUsrFolder * c_obj = 0;
    if( id && name ) {
        std::auto_ptr<sUsrObj> src_obj(m_User.objFactory(id));
        if( src_obj.get() ) {
            if( src_obj->isTypeOf("(folder|sysfolder)") ) {
                c_obj = ((sUsrFolder*) src_obj.get())->createSubFolder(name);
                dataForm.printf("{\"");
                c_obj->IdStr(&dataForm);
                dataForm.printf("\" : { \"signal\" : \"folder\", \"data\" : { \"to\":\"");
                src_obj->IdStr(&dataForm);
                dataForm.printf("\"} } }");
            }
        }
    }
    if( !c_obj ) {
        dataForm.printf("{\"%s\" : { \"signal\" : \"folder\", \"data\" : { \"error\":\"failed to create folder\"} } }", id.print());
    } else {
        delete c_obj;
    }
}

void sUsrCGI::allStat(void)
{
    sDic<udx> list;
    m_User.all(list, pForm->value("type"));
    sVarSet res;
    res.setColId(0, "id");
    res.setColId(1, "name");
    res.setColId(2, "path");
    res.setColId(3, "value");
    for(idx i = 0; i < list.dim(); ++i) {
        res.addRow().addCol("all").addCol("type_count").addCol((const char*) list.id(i)).addCol(*list.ptr(i));
    }
    res.printCSV(dataForm);
}

#if 0
static bool useTypeUPObj()
{
    static bool use_type_upobj = false;
    static bool first_call = true;

    if( first_call ) {
        // TYPE_UPOBJ env variable can be set via sUsrCGI::OnCGIInit() by &useTypeUPObj=1 url param
        if( const char * s = getenv("TYPE_UPOBJ") ) {
            use_type_upobj = sString::parseBool(s);
        }
        first_call = false;
    }
    return use_type_upobj;
}
#endif

void sUsrCGI::propSpec(const char* type_name, const char* view_name, const char* props)
{
    enum {
        eCSV,
        eJSON
    } mode = sIs(pForm->value("mode"), "json") ? eJSON : eCSV;
    if( !type_name ) {
        type_name = pForm->value("type");
    }

    sStr autoType;
    if(!type_name) {
        sHiveId id(pForm->value("ids"));
        if( id ) {
            sUsrObj * pobj = m_User.objFactory(id);
            if(pobj && pobj->Id())
                type_name=pobj->getTypeName();
            if(type_name)
                type_name=autoType.printf("%s",type_name);
            delete pobj;
        }
        if(!type_name)
            return;
    }
    if( !view_name ) {
        view_name = pForm->value("view");
    }
    if( !props ) {
        props = pForm->value("prop");
    }
    sStr filter00;
    if( props ) {
        sString::searchAndReplaceSymbols(&filter00, props, 0, ",", 0, 0, true, true, true, true);
        filter00.add0();
    }
    sJSONPrinter json_printer;
    if( const sUsrType2 * type = sUsrType2::ensure(m_User, type_name) ) {
        if( mode == eCSV ) {
            sVarSet list;
            type->props(m_User, list, filter00.ptr());
            list.printCSV(dataForm);
        } else if( mode == eJSON ) {
            json_printer.init(&dataForm);
            json_printer.startObject();
            type->printJSON(m_User, json_printer, true);
        }
        if( sLen(pForm->value("actions", "")) > 0 ) {
            sVec<const sUsrAction*> actions;
            for(idx ia = 0; ia < type->dimActions(m_User); ia++) {
                const sUsrAction * act = type->getAction(m_User, ia);
                if( act && !act->isObjAction() ) {
                    *actions.add(1) = act;
                }
            }
            if( actions.dim() ) {
                if( mode == eCSV ) {
                    dataForm.printf("%s,_action", type->name());
                    for(idx ia = 0; ia < actions.dim(); ia++) {
                        dataForm.addString(",");
                        sString::escapeForCSV(dataForm, actions[ia]->name());
                    }
                    dataForm.addString("\n");
                } else if( mode == eJSON ) {
                    json_printer.addKey("_action");
                    json_printer.startArray();
                    for(idx ia = 0; ia < actions.dim(); ia++) {
                        json_printer.addValue(actions[ia]->name());
                    }
                    json_printer.endArray();
                }
            }
        }
        if( pForm->boolvalue("types", false) ) {
            if( mode == eCSV ) {
                dataForm.printf("%s,_type,", type->name());
                sString::escapeForCSV(dataForm, type->title());
                dataForm.add("\n");
                dataForm.add0cut();
            } else if( mode == eJSON ) {
                json_printer.addKey("_type");
                json_printer.startObject();
                json_printer.addKey(type->name());
                json_printer.addValue(type->title());
                json_printer.endObject();
            }
        }
    }
    if( mode == eJSON ) {
        json_printer.endObject();
    }
}

namespace {
    // allow iterating over sUsrObjRes either by its default sort order, or explicitly by a vector of object ids
    struct PropGetResIter {
        sUsrObjRes::IdIter it;
        const sUsrObjRes * res;
        const sVec<sHiveId> & explicit_ids;
        idx iexplicit;

        PropGetResIter(const sUsrObjRes * res_, const sVec<sHiveId> & explicit_ids_):
            res(res_), explicit_ids(explicit_ids_)
        {
            if( explicit_ids.dim() ) {
                iexplicit = 0;
                res->resetIter(it, explicit_ids[0]);
            } else {
                iexplicit = -1;
                it = res->first();
            }
        }

        PropGetResIter & operator++()
        {
            if( explicit_ids.dim() ) {
                while( ++iexplicit < explicit_ids.dim() ) {
                    res->resetIter(it, explicit_ids[iexplicit]);
                    if( res->has(it) ) {
                        break;
                    }
                }
            } else {
                res->next(it);
            }
            return *this;
        }
        operator bool() const { return iexplicit < explicit_ids.dim() && res->has(it); }
        operator sUsrObjRes::IdIter & () { return it; }
    };
};

static void propGet_printTypeNamesCb(const sUsrType2 * utype, const sUsrType2 * recurse_start, idx depth_from_start, void * param)
{
    sStr * buf = static_cast<sStr*>(param);
    if( buf->length() ) {
        buf->addString(",");
    }
    // exact match regexp
    buf->printf("^%s$", utype->name());
}

void sUsrCGI::propGet(sUsrObjRes * obj_ids, const char* view_name, const char* props, sJSONPrinter * json_printer)
{
    enum {
        eCSV,
        eJSON,
        eProp
    } mode = sIs(pForm->value("mode"), "json") ? eJSON : sIs(pForm->value("mode"), "csv") ? eCSV : eProp;
    if( !view_name ) {
        view_name = pForm->value("view");
    }
    const bool show_actions = pForm->boolvalue("actions");
    const bool show_perm = pForm->boolvalue("perm");
    const bool flatten_tree = pForm->boolvalue("flatten"); // simplify json by omitting decorative list nodes
    sUsrObjRes res;
    sVec<sHiveId> explicit_ids;
    if( !obj_ids ) {
        const char * ii = pForm->value("ids");
        const char * obj_qry = pForm->value("objQry");
        if( ii && ii[0] ) {
            sHiveId::parseRangeSet(explicit_ids, ii);
        } else if( obj_qry && obj_qry[0] ) {
            sVariant qry_result;
            objQry(&qry_result, obj_qry);
            qry_result.asHiveIds(explicit_ids);
        }

        sStr sid;
        sHiveId::printVec(sid, explicit_ids, ",");

        if( sid ) {
            props = props ? props : pForm->value("prop");
            sStr type_names_buf;
            const char * type_names = "*";
            const char * type_names_qry = pForm->value("type");
            if( type_names_qry && strcmp(type_names_qry, "*") != 0 ) {
                // resolve types in c++ rather than sql, both for speed and security
                sUsrType2::find(m_User, 0, type_names_qry, propGet_printTypeNamesCb, &type_names_buf, false, true); // find types lazily - we only need their names
                type_names = type_names_buf.ptr();
            }
            if( type_names && *type_names ) {
                m_User.objs2(type_names, res, 0, "_id", sid.ptr(), props, show_perm, 0, 0);
            }
            if( res.dim() ) {
                obj_ids = &res;
            }
        }
    }
    const char * show_files_wildcard = pForm->value("files", 0);
    sJSONPrinter my_json_printer;
    if( !json_printer ) {
        json_printer = &my_json_printer;
    }
    if( mode == eCSV ) {
        dataForm.addString("id,name,path,value\n");
    } else if( mode == eJSON && json_printer == &my_json_printer) {
        json_printer->init(&dataForm);
    }
    if( obj_ids ) {
        if( mode == eJSON && obj_ids->dim() > 1 ) {
            json_printer->startArray();
        }
        for(PropGetResIter it(obj_ids, explicit_ids); it; ++it) {
            switch( mode ) {
                case eCSV:
                    obj_ids->csv(it, dataForm);
                    break;
                case eJSON:
                    json_printer->startObject();
                    obj_ids->json(m_User, it, *json_printer, true, flatten_tree);
                    break;
                case eProp:
                    obj_ids->prop(it, dataForm);
                    break;
            }
            if( show_actions || show_files_wildcard ) {
                const sHiveId * id = obj_ids->id(it);
                std::auto_ptr<sUsrObj> obj(m_User.objFactory(*id));
                if( obj.get() ) {
                    if( show_actions ) {
                        sVarSet acts;
                        sVec<sStr> vec;
                        obj->actions(vec);
                        if( mode == eJSON ) {
                            if( vec.dim() ) {
                                json_printer->addKey("_action");
                                json_printer->startArray();
                                for(idx i = 0; i < vec.dim(); ++i) {
                                    json_printer->addValue(vec[i].ptr());
                                }
                                json_printer->endArray();
                            }
                        } else {
                            sStr a;
                            for(idx i = 0; i < vec.dim(); ++i) {
                                a.printf(",%s", vec[i].ptr());
                            }
                            if( a ) {
                                acts.addRow().addCol(obj->Id()).addCol("_action").addCol("").addCol(a.ptr(1));
                            }
                            if( mode == eCSV ) {
                                acts.printCSV(dataForm, 0);
                            } else {
                                acts.printProp(dataForm, 0, 1, 2, 3);
                            }
                        }
                    }
                    if( show_files_wildcard ) {
                        switch( mode ) {
                            case eCSV:
                                obj->fileProp(dataForm, true, show_files_wildcard);
                                break;
                            case eJSON:
                                obj->fileProp(*json_printer, show_files_wildcard, true);
                                break;
                            case eProp:
                                obj->fileProp(dataForm, false, show_files_wildcard);
                                break;
                        }
                    }
                }
            }
            if( mode == eJSON ) {
                json_printer->endObject();
            }
        }
        if( mode == eJSON && obj_ids->dim() > 1 ) {
            json_printer->endArray();
        }
    }
}

void sUsrCGI::propSet(sVar* form)
{
    m_User.propSet(form ? *form : *pForm, dataForm);
}

void sUsrCGI::propSet2(sVar* form/* = 0 */)
{
    if( !form ) {
        form = pForm;
    }
    idx json_parse_txt_len = 0;
    const char * json_parse_txt = form->value("parse", 0, &json_parse_txt_len);

    if( m_User.updateStart() ) {
        sDic<sUsrPropSet::Obj> modified_objs;
        sUsrPropSet upropset(m_User);
        upropset.setSrc(json_parse_txt, json_parse_txt_len).disableFileRoot();
        if( upropset.run(&modified_objs) ) {
            if( m_User.updateComplete() ) {
                sJSONPrinter printer(&dataForm);
                printer.startObject();
                for(idx i = 0; i < modified_objs.dim(); i++) {
                    idx key_len = 0;
                    const char * key = static_cast<const char *>(modified_objs.id(i, &key_len));
                    printer.addKey(key, key_len);
                    printer.startObject();
                    printer.addKeyValue("_id", modified_objs[i].id);
                    printer.addKeyValue("_type", modified_objs[i].utype->name());
                    printer.endObject();
                }
                printer.endObject();
            } else {
                // updateComplete failed
                dataForm.addString("error: database connection error 2\n");
            }
        } else {
            // json error
            dataForm.printf("error: %s\n", upropset.getErr());
        }
    } else {
        // updateStart failed
        dataForm.addString("error: database connection error 1\n");
    }
}

static bool is_potential_settable_prop_name(const char * s)
{
    if( !s ) {
        return false;
    }
    if( !isalpha(s[0]) ) {
        return false;
    }
    for(idx i = 1; s[i]; i++) {
        if( !isalnum(s[i]) && s[i] != '_' && s[i] != '-' ) {
            return false;
        }
    }
    return true;
}

void sUsrCGI::propBulkSet(void)
{
    // parameters:
    // ids = list of object ids
    // objQry = query language expression evaluating to a list of object ids (appended to ids and uniquified)
    // fmla = formula appled to each object as topic, expected to produce a string
    // mapfile = object ID of csv table with 2 columns, from / to (for mapping formula result)
    // prop = name of field to set with the result
    const char * ids = pForm->value("ids");
    const char * obj_qry = pForm->value("objQry");
    const char * fmla = pForm->value("fmla");
    const char * mapfile_id = pForm->value("mapfile");
    const char * prop_name = pForm->value("prop");

    std::auto_ptr<qlang::sUsrEngine> engine(queryEngineFactory(qlang::sUsrContext::fUnderscoreId));
    sStr error, result, cell, path;
    sVariant value;

#ifndef WIN32
    // If a query takes >30 sec or >2 GB heap or stack, something is very wrong
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = rlimitHandler;
    sigaction(SIGXCPU, &sa, NULL);

    struct rlimit rlim;
    rlim.rlim_cur = 30; rlim.rlim_max = rlim.rlim_cur + 10;
    setrlimit(RLIMIT_CPU, &rlim);
    rlim.rlim_cur = 1<<31; rlim.rlim_max = rlim.rlim_cur + (1<<29);
    setrlimit(RLIMIT_DATA, &rlim);
    setrlimit(RLIMIT_STACK, &rlim);
#endif

    sVec<sHiveId> ids_list;
    sHiveId::parseRangeSet(ids_list, ids);

    if( obj_qry && obj_qry[0] ) {
        if( !engine->parse(obj_qry, 0, &error) ) {
            dataForm.printf("error : failed to parse objQry : %s", error.ptr());
            return;
        }
        if( !engine->eval(value, &error) ) {
            dataForm.printf("error : failed while running objQry : %s", error.ptr());
            return;
        }

        value.asHiveIds(ids_list);
        value.setNull();
    }

    sDic<bool> unique_ids;
    for(idx i = 0; i < ids_list.dim(); i++) {
        unique_ids.set(ids_list.ptr(i), sizeof(sHiveId));
    }
    ids_list.cut(0);
    ids_list.resize(unique_ids.dim());
    for(idx i = 0; i < unique_ids.dim(); i++) {
        ids_list[i] = *static_cast<const sHiveId*>(unique_ids.id(i));
    }

    if( !fmla || !fmla[0] ) {
        dataForm.printf("error : missing fmla");
        return;
    }

    if( !engine->parse(fmla, 0, &error) ) {
        dataForm.printf("error : failed to parse fmla : %s", error.ptr());
        return;
    }

    sTxtTbl mapfile_tbl;
    sDic<idx> mapfile_dic; // map from cell content to row number for first column ("from") of mapfile_tbl
    if (mapfile_id && mapfile_id[0]) {
        const sUsrFile * ufile = dynamic_cast<const sUsrFile*>(m_User.objFactory(mapfile_id));
        if( !ufile ) {
            dataForm.printf("error : failed to open mapfile %s", mapfile_id);
            return;
        }
        sStr filename;
        if( !ufile->getFile(filename) ) {
            dataForm.printf("error : failed to determine filename for mapfile %s", mapfile_id);
            delete ufile;
            return;
        }

        mapfile_tbl.parseOptions().flags = sTblIndex::fSaveRowEnds | sTblIndex::fTopHeader;
        if( ufile->isTypeOf("csv-table") ) {
            mapfile_tbl.parseOptions().colsep = ",";
        } else if( ufile->isTypeOf("tsv-table") ) {
            mapfile_tbl.parseOptions().colsep = "\t";
        } else if( filename.length() > 4 && (sIs(filename.ptr(filename.length() - 4), ".tsv") || sIs(filename.ptr(filename.length() - 4), ".tab")) ) {
            mapfile_tbl.parseOptions().colsep = "\t";
        } else {
            mapfile_tbl.parseOptions().colsep = ",";
        }

        delete ufile;
        ufile = 0;

        mapfile_tbl.setFile(filename);
        if( !mapfile_tbl.parse() ) {
            dataForm.printf("error : failed to parse mapfile %s", mapfile_id);
            return;
        }

        for(idx ir = 0; ir < mapfile_tbl.rows(); ir++) {
            cell.cut0cut();
            mapfile_tbl.printCell(cell, ir, 0);
            if( !mapfile_dic.get(cell) ) {
                *mapfile_dic.setString(cell) = ir;
            }
        }
    }

    sUsrObjRes res;
    if( !is_potential_settable_prop_name(prop_name) ) {
        dataForm.printf("error : missing or invalid prop parameter");
        return;
    }
    m_User.objs2(0, res, 0, 0, 0, prop_name);

    for(idx i = 0; i < ids_list.dim(); i++) {
        const sHiveId & id = ids_list[i];
        if( !engine->registerBuiltinThis(ids_list[i]) ) {
            dataForm.printf("error : failed to register id %s as query language topic", id.print());
            return;
        }

        value.setNull();
        if( !engine->eval(value, &error) ) {
            dataForm.printf("error : failed while running fmla on id %s : %s", id.print(), error.ptr());
            return;
        }

        if( const idx * pir = mapfile_dic.get(value.asString()) ) {
            mapfile_tbl.val(value, *pir, 1);
        }

        path.cut0cut();
        bool have_path = false;

        if( const sUsrObjRes::TObjProp * oprop = res.get(id) ) {
            if( const sUsrObjRes::TPropTbl * proptbl = res.get(*oprop, prop_name) ) {
                path.addString(res.getPath(proptbl));
                have_path = true;
            }
        }

        if( !have_path ) {
            // if there is no such field in the object, we have to construct the entire prop tree
            // to create a reasonable path
            sUsrObj * uobj = m_User.objFactory(id);
            if( !uobj ) {
                dataForm.printf("error : object %s could not be opened", id.print());
                return;
            }
            sUsrObjPropsTree tree(m_User, uobj->getTypeName());
            uobj->propBulk(tree.getTable());
            tree.useTable(tree.getTable());

            delete uobj;
            uobj = 0;

            if( const sUsrObjPropsNode * node = tree.push(prop_name, value) ) {
                path.addString(node->path());
                have_path = true;
            } else {
                dataForm.printf("error : failed to add property '%s' to object %s", prop_name, id.print());
                return;
            }
        }

        result.addString("\nprop.");
        id.print(result);
        result.printf(".%s%s%s=%s", prop_name, path.length() ? "." : "", path.ptr(), value.asString());
    }

    dataForm.addString(result);
}

void sUsrCGI::propDel(void)
{
    const char* objIds = pForm->value("ids");
    const char* prop = pForm->value("prop");
    const char* path = pForm->value("path");
    const char* value = pForm->value("value");

    sVec<sHiveId> os;
    sHiveId::parseRangeSet(os, objIds);
    for(idx o = 0; o < os.dim(); ++o) {
        std::auto_ptr<sUsrObj> obj(m_User.objFactory(os[o]));
        if( obj.get() ) {
            if( !obj->propDel(prop, path, value) ) {
                dataForm.printf("err.");
                obj->Id().print(dataForm);
                dataForm.printf("._err=failed to delete property '%s'", prop);
                if( value && value[0] ) {
                    if( path && path[0] ) {
                        dataForm.printf(" path '%s' value '%s'\n", path, value);
                    } else {
                        dataForm.printf(" value '%s'\n", value);
                    }
                } else if( path && path[0] ) {
                    dataForm.printf(" path '%s'\n", path);
                } else {
                    dataForm.printf("\n");
                }
            }
        }
    }
}

void sUsrCGI::simpleCast(void)
{
    const char* objIds = pForm->value("ids");
    const char* type = pForm->value("type");

    if( type && type[0] ) {
        sVec<sHiveId> os;
        sHiveId::parseRangeSet(os, objIds);
        dataForm.printf("{");
        for(idx o = 0; o < os.dim(); ++o) {
            dataForm.printf("%s\n \"%s\" : ", o > 0 ? "," : "", os[o].print());
            std::auto_ptr<sUsrObj> obj(m_User.objFactory(os[o]));
            if( obj.get() ) {
                sStr type_from("%s", obj->getTypeName());
                sUsrObj * o = obj->cast(type);
                if( o && o != obj.get() ) {
                    dataForm.printf("{ \"signal\" : \"cast\", \"data\" : { \"from\" : \"%s\", \"to\" : \"%s\" } }", type_from.ptr(), type);
                    delete o;
                    continue;
                }
            }
            dataForm.printf("{ \"signal\" : \"cast\", \"data\" : { \"error\" : \"cast failed\" } }");
        }
        dataForm.printf("\n}");
    }
}

void sUsrCGI::permSet(void)
{
    const char* objIds = pForm->value("ids");
    const char* optObjIds = pForm->value("optIds");
    const char* groupIds = pForm->value("groups");

    bool ok = m_User.updateStart();
    if( ok ) {
        sVec<sHiveId> os, opt_os;
        sVec<idx> gs;
        sHiveId::parseRangeSet(os, objIds);
        sHiveId::parseRangeSet(opt_os, optObjIds);
        sString::scanRangeSet(groupIds, 0, &gs, 0, 0, 0);
        ok = (os.dim() || opt_os.dim()) && gs.dim();
        if( ok ) {
            udx perm = ePermNone, flags = eFlagNone;
            const char * sviewId = pForm->value("view");
            const char * sperm = pForm->value("perm", "0");
            const char * sflags = pForm->value("flag", "0");
            m_User.permPrettyScanf(0, sviewId, sperm, sflags, 0, 0, 0, &perm, &flags);
            for(idx o = 0; ok && o < os.dim(); ++o) {
                for(idx g = 0; ok && g < gs.dim(); ++g) {
                    sHiveId viewId;
                    if( sviewId ) {
                        if( const sUsrType2 * utype = m_User.objGetType(os[o], ePermCanAdmin) ) {
                            if( const sUsrView * view = utype->getView(m_User, sviewId) ) {
                                viewId = view->id();
                            }
                        }
                    }

                    ok &= m_User.setPermission(gs[g], os[o], perm, flags, &viewId);
                }
            }
            udx opt_perm = ePermNone, opt_flags = eFlagNone;
            const char * sopt_perm = pForm->value("optPerm", "0");
            const char * sopt_flags = pForm->value("optFlag", "0");
            m_User.permPrettyScanf(0, sviewId, sopt_perm, sopt_flags, 0, 0, 0, &opt_perm, &opt_flags);
            for(idx o = 0; ok && o < opt_os.dim(); ++o) {
                for(idx g = 0; ok && g < gs.dim(); ++g) {
                    sHiveId viewId;
                    if( sviewId ) {
                        if( const sUsrType2 * utype = m_User.objGetType(opt_os[o], ePermCanAdmin) ) {
                            if( const sUsrView * view = utype->getView(m_User, sviewId) ) {
                                viewId = view->id();
                            }
                        }
                    }

                    m_User.setPermission(gs[g], opt_os[o], opt_perm, opt_flags, &viewId, objIds);
                }
            }
        }
        if( ok ) {
            ok = m_User.updateComplete();
        } else {
            m_User.updateAbandon();
        }
    }
    dataForm.printf("%s", ok ? "ok" : "failed");
}

void sUsrCGI::permCopy(void)
{
    const sHiveId objFrom(pForm->value("from"));
    const char* objIds = pForm->value("ids");

    bool ok = m_User.updateStart();
    if( ok ) {
        sVec<sHiveId> objIdsTo;
        sHiveId::parseRangeSet(objIdsTo, objIds);

        for(idx o = 0; ok && o < objIdsTo.dim(); ++o) {
            ok &= m_User.copyPermission(objFrom, objIdsTo[o]);
        }
        if( ok ) {
            ok = m_User.updateComplete();
        } else {
            m_User.updateAbandon();
        }
    }
    dataForm.printf("%s", ok ? "ok" : "failed");
}

void sUsrCGI::groupAdd(void)
{
    const char* nm = pForm->value("name");
    const char* abbr = pForm->value("abbr");
    const char* parent = pForm->value("parent");

    if( !m_User.groupCreate(nm, abbr, parent) && !m_User.err ) {
        m_User.err.printf("Request unsuccessful, try again later.");
    }
    dataForm.printf("%s", m_User.err ? m_User.err.ptr() : "ok");
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ User Info
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void sUsrCGI::userList(void)
{
    idx isGrp = pForm->ivalue("ugrp");
    idx allUsers = pForm->ivalue("usr");
    idx active = pForm->ivalue("active");
    const char * search = pForm->value("search");
    bool primaryGrpOnly = pForm->boolvalue("primaryGrpOnly");

    sVec<sStr> lst;
    m_User.listUsr(&lst, isGrp, allUsers, active, search, primaryGrpOnly);
    dataForm.printf("id,path,name\n");
    for(idx i = 0; i < lst.dim(); ++i) {
        const char * userName = lst[i].ptr();
        const char * groupPath = sString::next00(userName);
        const char * id = sString::next00(groupPath);
        dataForm.printf("%s,%s,%s\n", id, groupPath, userName);
    }
}

void sUsrCGI::groupList(void)
{
    idx isGrp = pForm->ivalue("ugrp");
    idx usersOnly = pForm->ivalue("usr");
    const char * search = pForm->value("search");

    sVec<sStr> lst;
    m_User.listGrp(&lst, isGrp, usersOnly, search);
    dataForm.printf("id,path,name\n");
    for(idx i = 0; i < lst.dim(); ++i) {
        const char * userName = lst[i].ptr();
        const char * groupPath = sString::next00(userName);
        const char * id = sString::next00(groupPath);
        dataForm.printf("%s,%s,%s\n", id, groupPath, userName);
    }
}

void sUsrCGI::inactList()
{
    idx isGrp = pForm->ivalue("ugrp");

    sVec<sStr> lst;
    m_User.listInactive(&lst, isGrp);
    for(idx i = 0; i < lst.dim(); ++i) {
        dataForm.printf("%s\n", lst[i].ptr());
    }
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Files
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
void sUsrCGI::file(void)
{
    const char * obj_ids = pForm->value("ids");
    const char * filename = pForm->value("filename");
    const char * propname = pForm->value("propname");
    idx maxSize = pForm->ivalue("maxSize", sIdxMax);
    idx offset = pForm->ivalue("offset", 0);
    const char * ellipsize = pForm->value("ellipsize");
    idx prefix=pForm->ivalue("prefix",1);

    sVec<sHiveId> ids;
    sHiveId::parseRangeSet(ids, obj_ids);
    sStr log;
    for(idx i = 0; i < ids.dim(); ++i) {
        std::auto_ptr<sUsrObj> obj(m_User.objFactory(ids[i], 0, ePermCanDownload));
        if( obj.get() ) {
            sFilePath nm;
            if( prefix ) {
                nm.printf("o%s-", obj->Id().print());
            }
            sStr path;
            if( filename && filename[0] ) {
                if( obj->getFilePathname(path, "%s", filename) ) {
                    nm.printf("%s", filename);
                }
            } else if( propname && propname[0] ) {
                obj->propGet(propname, &nm);
                obj->getFilePathname(path, "%s", nm.ptr());
            } else if( const sUsrFile * u_file = dynamic_cast<const sUsrFile*>(obj.get()) ) {
                u_file->getFile(path);
                sStr fnm, ext;
                if( path && u_file->propGet("path") ) {
                    // old ways
                    nm.makeName(path, "%%flnm");
                } else {
                    u_file->propGet("ext", &ext);
                    u_file->propGet("name", &fnm);
                    if( sLen(ext) ) {
                        nm.makeName(fnm, "%%flnmx%s%s", ext ? "." : "", ext ? ext.ptr() : "");
                    } else {
                        nm.makeName(fnm, "%%flnm");
                    }
                }
            } else {
                continue;
            }
            if( path ) {
                sFil fl(path, sMex::fReadonly);
                if( fl.ok() ) {
                    if(raw<1)raw = 1;
                    sStr fl_ellipsized;

                    const char * fl_buf = fl.ptr();
                    idx fl_buf_len = fl.length();
                    if( offset > 0 ) {
                        offset = sMin<idx>(offset, fl_buf_len);
                        fl_buf += offset;
                        fl_buf_len -= offset;
                    } else if( offset < 0 && -offset <= fl_buf_len ) {
                        // negative offsets are from the end of the file
                        fl_buf += fl_buf_len + offset;
                        fl_buf_len = -offset;
                    }

                    if( fl_buf_len>maxSize )
                        fl_buf_len=maxSize;

                    bool cutoffFront = fl_buf > fl.ptr();
                    bool cutoffBack = fl_buf + fl_buf_len < fl.ptr() + fl.length();

                    if( ellipsize && (cutoffFront || cutoffBack) ) {
                        fl_ellipsized.printf("%s%.*s%s", cutoffFront ? ellipsize : "", (int)fl_buf_len, fl_buf, cutoffBack ? ellipsize : "");
                        fl_buf = fl_ellipsized.ptr();
                        fl_buf_len = fl_ellipsized.length();
                    }
                    outBin(fl_buf, fl_buf_len, fl_buf_len == fl.length() ? sFile::time(path) : 0, true, "%s", nm.ptr());
                    m_User.audit(sUsr::eUserAuditActions, "download", "objID='%s'; file='%s'; bytes='%"DEC"'", obj->Id().print(), nm.ptr(), fl_buf_len);
                } else {
                    error("Referred file '%s' is not found.", nm.ptr());
                }
            }
        }
    }
}

void sUsrCGI::dropboxList(void)
{
    // initialize user setting object
    sUsrObjRes dropboxList;
    m_User.objs2("dropbox", dropboxList, 0, 0, 0, "dropbox_path,dropbox_name");
    if( !dropboxList.dim() ) {
        return;
    }

    const char * whichdropbox = pForm->value("dropbox");
    const char * wildcard = pForm->value("wildcard", "*");
    idx outs = 3;
    sString::xscanf(pForm->value("out", "title|files"), "%b=3|title|files;", &outs);
    if( !outs ) {
        outs = 3;
    }
    sVarSet res;
    res.setColId(0, "id");
    res.setColId(1, "title");
    res.setColId(2, "path");
    res.setColId(3, "existing_id");
    sStr pathS, buf;
    sDir lst;
    for(sUsrObjRes::IdIter it = dropboxList.first(); dropboxList.has(it); dropboxList.next(it)) {
        sHiveId dropbox_id = *dropboxList.id(it);
        const char * dropbox_name = 0;
        pathS.cut0cut();
        buf.cut0cut();
        lst.cut();
        if( const sUsrObjRes::TPropTbl * t = dropboxList.get(*dropboxList.get(it), "dropbox_path") ) {
            pathS.addString(dropboxList.getValue(t));
        }
        if( const sUsrObjRes::TPropTbl * t = dropboxList.get(*dropboxList.get(it), "dropbox_name") ) {
            dropbox_name = dropboxList.getValue(t);
        }
        if( !pathS || !dropbox_name || !*dropbox_name ) {
            continue;
        }
        if( whichdropbox && strcmp(whichdropbox, dropbox_name) != 0 ) { // this dropbox has not been asked for
            continue;
        }

        res.addRow();
        res.addCol(dropbox_id); // tree node id
        if( outs & 1 ) {
            res.addCol(dropbox_name); // tree node title - use readable name
        } else {
            res.addCol(""); // tree node title - use default (same as id)
        }
        buf.cut(0);
        buf.addString("/", 1);
        dropbox_id.print(buf);
        buf.addString("/", 1);
        idx buf_id_len = buf.length();
        res.addCol(buf);
        if( outs == 1 ) {
            // only dropbox titles are requested
            continue;
        }
        const char * dir = strstr(pathS, "://");
        if( dir ) {
            dir += 3;
        } else {
            dir = pathS;
        }
        lst.find(sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive), dir, wildcard, 0, 0, dir);
        for(idx ie = 0; ie < lst.dimEntries(); ie++) {
            const char * relative_path = lst.getEntryPath(ie);
            res.addRow();
            res.addCol(relative_path); // tree node id
            res.addCol(""); // tree node title - use default (same as id)
            if( outs & 2 ) {
                // print relative file path
                buf.cut(buf_id_len);
                buf.addString(relative_path);
                res.addCol(buf);
            } else {
                // nothing
                res.addCol("");
            }
            sUsrObjRes existing;
            // TODO fix invalid way to find 'existing file' just!!! by its name
            m_User.objs2("u-file+", existing, 0, "orig-name", relative_path, 0, 1);
            if( existing.dim() ) {
                res.addCol(existing.firstId()->print());
            }
        }
    }
    res.printCSV(dataForm);
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Ion Commands
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
void sUsrCGI::ionObjList(void)
{
    /*
    const char * ids=pForm->value("ids");
    const char * filename=pForm->value("filename");
    sVec<sHiveId> ion_ids;
    if(ids)sHiveId::parseRangeSet(ion_ids,ids);


    sStr path00;
    for ( idx iio=0; iio<ion_ids.dim(); ++iio) {
        sUsrObj us(m_User, ion_ids[iio]);
        if( !us.Id() ) {
            continue;
        }
        us.getFilePathname(path00, "%s", filename ? filename : 0);
    }
    if(!path00.length()){
        error("No valid ions specified.");
    }
    path00.add0(2);
    sHoneyCombSet hc(path00.ptr(),sMex::fReadonly);
    if(!hc.ok()){
        error("Referred ions are corrupt of missing.");
    }

    const char * type=pForm->value("type");
    const char * name=pForm->value("name");
    const char * value=pForm->value("value");
    sHoneyComb::Locator loc;
    sIO buf(sMex::fBlockDoubling,(sIO::callbackFun)::printf);
    while ( hc.objListIterate(&loc, true, type, name, value , &buf) ) {
    }

    //sUsrObj us(m_User, userSetList[0]);
    //sVec<sHiveId> ionList;m_User.objs("ions", ions);
*/
}

void sUsrCGI::ionPropGet(void)
{
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Main Command Function
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx sUsrCGI::Cmd(const char * cmd)
{
    idx retval = 1;
    enum enumCommands
    {
        eLogin, eLogout, eBatch, ePswdSet, ePswdChange, eUserGet, eUserSet, eUserReg, eUsrList, eGrpList, eGrpAdd, ePermSet,
        ePropSpec, ePropGet, eObjs, eObjQry, ePropSet, ePropSet2, ePropBulkSet, ePropDel, eSimpleTypeCast, eContactUs, eTypeTree,
        eFile, eDropboxList, eUsrV0, eUsrV1, eUsrV2, eUsrV3, eUsrFgt, eObjDel, eInactiveList, eUsrV4, eUsrMgr, ePermCopy, eAllStat,
        eObjRemove,eObjCopy,eObjCut,eFolderCreate,eUsrEmAuth,
        eIonObjList,eIonPropGet,
        eLast
    };

    const char * listCommands = "login"_"logout"_"batch"_"pswdSet"_"pswdChange"_"user"_"userSet"_"userReg"_"usrList"_"grpList"_"grpAdd"_"permset"_
    "propspec"_"propget"_"objList"_"objQry"_"propset"_"propset2"_"propBulkSet"_"propDel"_"scast"_"sendmail"_"typeTree"_
    "objFile"_"dropboxlist"_"userV0"_"userV1"_"userV2"_"userV3"_"forgot"_"objDel"_"inactive"_"userV4"_"mgr"_"permcopy"_"allStat"_
    "objRemove"_"objCopy"_"objCut"_"folderCreate"_"emauth"_
    "ionObjList"_"ionObjGet"_
    __;

    idx cmdnum = -1;
    if(cmd)
        sString::compareChoice(cmd, listCommands, &cmdnum, false, 0, true);

    switch(cmdnum) {
        case eLogin:
            login();
            break;
        case eLogout:
            logout();
            break;
        case eBatch:
            batch();
            break;
        case ePswdSet:
            passwordReset();
            break;
        case ePswdChange:
            passwordChange();
            break;
        case eUserGet:
            userSet(true);
            break;
        case eUserSet:
            userSet(false);
            break;
        case eUserReg:
            userReg();
            break;
        case eUsrList:
            userList();
            break;
        case eGrpList:
            groupList();
            break;
        case eGrpAdd:
            groupAdd();
            break;
        case eUsrV0:
            userResendEmailVerify();
            break;
        case eUsrV1:
            userVerifyEmail();
            break;
        case eUsrV2:
            userResendAccountActivate();
            break;
        case eUsrV3:
            userAccountActivate();
            break;
        case eUsrV4:
            userGroupActivate();
            break;
        case eUsrEmAuth:
            userEmailAuth();
            break;
        case eUsrFgt:
            userForgot();
            break;
        case eInactiveList:
            inactList();
            break;
        case eUsrMgr:
            userMgr();
            break;
        case eContactUs:
            contact();
            break;
        case ePermSet:
            permSet();
            break;
        case ePermCopy:
            permCopy();
            break;
        case eTypeTree:
            typeTree();
            break;
        case eObjs:
            objs();
            break;
        case eObjQry:
            objQry();
            break;
        case eObjDel:
            objDel();
            break;
        case ePropSpec:
            propSpec();
            break;
        case ePropGet:
            propGet();
            break;
        case ePropSet:
            propSet();
            break;
        case ePropSet2:
            propSet2();
            break;
        case ePropBulkSet:
            propBulkSet();
            break;
        case ePropDel:
            propDel();
            break;
        case eSimpleTypeCast:
            simpleCast();
            break;
        case eFile:
            file();
            break;
        case eDropboxList:
            dropboxList();
            break;
        case eObjRemove:
            objRemove();
            break;
        case eObjCopy:
            objMove(true);
            break;
        case eObjCut:
            objMove();
            break;
        case eFolderCreate:
            folderCreate();
            break;
        case eAllStat:
            allStat();
            break;

        case eIonObjList:
            ionObjList();
            break;
        case eIonPropGet:
            ionPropGet();
            break;

        default:
            cmdnum = -1;
            break;
    }
    if( cmdnum == -1 ) {
        retval = TParent::Cmd(cmd);
    } else {
        TParent::outHtml();
    }
    return retval;
}
