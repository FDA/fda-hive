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
#include <slib/std/crypt.hpp>
#include <slib/std/cryptocipher.hpp>
#include "uperm.hpp"#include <qlib/QPrideBase.hpp>

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
    return true;
}

static
void setUserCookie(sCGI & cgi, sUsr & usr, sStr & sid)
{
    sStr buf;
    cgi.cookieSet("sessionID", sCGI::eCookie_Secure | sCGI::eCookie_HttpOnly | sCGI::eCookie_SameSiteStrict, "%s", usr.encodeSID(sid, buf));
    cgi.cookieSet("userName", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", usr.Name());
    const char* email = usr.Email();
    cgi.cookieSet("email", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email ? email : "");
}


bool sUsrCGI::OnCGIInit(void)
{
    m_apiVersion = pForm->uvalue("api", sUdxMax);
    sStr sid("%s", pForm->value("sessionID", ""));
    const char * ip = pForm->value("ADDR");
    udx key = 0, uid = 0;
    m_SID.destroy();
#ifdef _DEBUG
#endif
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
    udx projectId = pForm->uvalue("projectID", 0);
    m_User.init(m_User.m_Id, projectId);
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

const char* sUsrCGI::selfURL(sStr& url)
{
    sQPrideBase * qp = m_User.QPride();
    if( qp ) {
        qp->cfgStr(&url, 0, "baseURL");
        if( url ) {
            return url.ptr();
        }
    }
    return TParent::selfURL(url);
}

void sUsrCGI::logout(void)
{
    m_User.logout(pForm->value("ADDR"));
    setUserCookie(*this, m_User, m_SID);
}

void sUsrCGI::loginInfo(void)
{
    raw |= 1;
    if( !m_User.Id() || m_User.isGuest() ) {
        dataForm.printf("0");
    } else {
        dataForm.printf("1");
    }
}

void sUsrCGI::loginCommon(const sUsr::ELoginResult eres, const char * email, const idx logCount)
{
    headerSet("Status", "200");
    do {
        if( !email || !email[0] ) {
            email = m_User.Email();
        }
        switch(eres) {
            case sUsr::eUserOperational:
                if( logCount == 0 ) {
                    warning("You must change your password now.");
                    sStr t("pswdSet&followTo=%s", redirectURL.ptr());
                    redirectURL.replace(t);
                    cookieSet("preset", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%" UDEC, m_User.addPasswordResetID());
                    cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                }
                cookieSet("last_login", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserEmailNotValidated:
                redirectURL.cut(0);
                outSection("user_email_inactive");
                warning("Email address on the account is not validated. See web page for more information.");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserBlocked:
                redirectURL.cut(0);
                outSection("user_account_inactive");
                warning("Your account is deactivated. Request activation on web page.");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserAccountExpired:
                redirectURL.cut(0);
                outSection("user_account_expired");
                warning("Your account has expired. Request activation on web page.");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserPswdExpired:
                redirectURL.printf(0, "user_pswd");
                warning("Your password has expired. You must change your password now.");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserLoginAttemptsWarn1Left:
                redirectURL.cut(0);
                warning("You have one last attempt to login before your will be locked.");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserLoginAttemptsNowLocked:
                redirectURL.cut(0);
                warning("Your account is now locked. Contact administrator.");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserLoginAttemptsTooMany:
                redirectURL.cut(0);
                warning("Your account is locked. Contact administrator.");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserLoginAttemptsTooManyTryLater:
                redirectURL.cut(0);
                warning("Too many failed attempts. Please try again later");
                cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
                break;
            case sUsr::eUserNotFound:
                cookieSet("followTo", "%s", redirectURL.ptr());
                error("Email or password is not recognized");
                redirectURL.cut(0);
                break;
            case sUsr::eUserNotSet:
                redirectURL.cut(0);
                break;
            case sUsr::eUserInternalError:
                redirectURL.printf(0, "login");
                error("Authentication failed. See web page.");
                redirectURL.cut(0);
                break;
            case sUsr::eUserTokenExpired:
                error("Token is expired");
                redirectURL.cut(0);
                break;
        }
    } while( false );
    setUserCookie(*this, m_User, m_SID);

    if( !raw ) {
        sStr login_opts, tmp;
        sQPrideBase * qp = m_User.QPride();
        if( qp ) {
            qp->cfgStr(&login_opts, 0, "user.LoginTmpltList");
            sStr sso_url;
            qp->cfgStr(&sso_url, 0, "user.sso.url", 0);
            if( sso_url ) {
                cookieSet("sso_url", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", sso_url.ptr());
            }
        }
        if( !login_opts ) {
            login_opts.printf(0, "user_login_pswd");
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

void sUsrCGI::login(void)
{
    const char * email = pForm->value("login");
    const char * pswd = pForm->value("pswd");
    idx logCount = -1;
    sUsr::ELoginResult eres = sUsr::eUserNotSet;
    if( email && pswd ) {
        eres = m_User.login(email, pswd, pForm->value("ADDR"), &logCount);
    }
    loginCommon(eres, email, logCount);
}

void sUsrCGI::SSO()
{
    sStr log;
    const char * email = 0;
    sQPrideBase * qp = m_User.QPride();
    while( qp ) {
        sStr auth_type, idp_var, idp, email_var;
        qp->cfgStr(&auth_type, 0, "user.sso.auth_type", 0);
        qp->cfgStr(&idp_var, 0, "user.sso.env_var_provider_id", 0);
        qp->cfgStr(&idp, 0, "user.sso.provider_id", 0);
        qp->cfgStr(&email_var, 0, "user.sso_env_var_email", 0);
        if( !auth_type || !idp_var || !idp || !email_var) {
            cookieSet("sso_on", "%s", "false");
            break;
        }
        const char * v = getenv("AUTH_TYPE");
        if( v && strcasecmp(auth_type, v) == 0 && idp_var ) {
            log.printf("AUTH_TYPE='%s'", v);
            v = getenv(idp_var);
            if( v && strcasecmp(idp, v) == 0 ) {
                log.printf("; %s='%s'", idp_var.ptr(), v);
                email = getenv(email_var);
                if( email ) {
                    log.printf("; %s='%s'", email_var.ptr(), email);
                }
            }
        }
        break;
    }
    idx logCount = -1;
    sUsr::ELoginResult eres = sUsr::eUserNotSet;
    if( email && email[0] ) {
        eres = m_User.loginByEmail(email, pForm->value("ADDR"), &logCount, log);
    }
    loginCommon(eres, email, logCount);
}

void sUsrCGI::batch(void)
{
    raw |= 1;
    m_User.batch(pForm->value("ADDR"));
    setUserCookie(*this, m_User, m_SID);
    error("%s", m_User.err.ptr());
    return;
}


void sUsrCGI::userResendEmailVerify()
{
    const char* email = pForm->value("emailAct", pForm->value("email"));
    sStr baseURL;
    selfURL(baseURL);
    if( email && email[0] ) {
        if( m_User.sendEmailValidation(baseURL, email) ) {
            alert("Email sent.");
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
        redirectURL.printf(0, "logout&follow=login%%26follow%%3DuserV4%%2526grpid%%253D%" UDEC, groupId);
    }
}

static void user_pswd(sUsrCGI & ucgi, sQPrideBase * qp, const char * section)
{
    sStr buf;
    const char * checkComplexity = qp ? qp->cfgStr(&buf, 0, "user.pswdCheckComplexity", "1") : "1";
    ucgi.cookieSet("cmplx", "%s", checkComplexity);
    const char * minLen = qp ? qp->cfgStr(&buf, 0, "user.pswdMinLength", "15") : "15";
    ucgi.cookieSet("mlen", "%s", minLen);
    const char * useSymb = qp ? qp->cfgStr(&buf, 0, "user.pswdUseSymbols", "0") : "0";
    ucgi.cookieSet("usym", "%s", useSymb);
    if( section ) {
        ucgi.outSection(section);
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

void sUsrCGI::sendmail()
{
    const char * email = pForm->value("emailAct", pForm->value("email"));
    const char * subj = 0;
    const char * body = pForm->value("message");
    const char * autoreply = pForm->value("autoreply");
    const udx reqid = pForm->uvalue("reqid", 0);

    sStr reponse2sender;
    if( sIsExactly(autoreply, "contact") ) {
        subj = "HIVE contact request";
        reponse2sender.printf(0, "Thank you for submitting the question. You will be contacted shortly by our team."
            " We look forward to working with you.\n\nThank you,\nHIVE at the U.S. Food and Drug Administration (FDA)\n");
    } else if( sIsExactly(autoreply, "projreq") ) {
        subj = "Project Request";
        reponse2sender.printf(0, "Thank you for submitting project request #%" UDEC ". You will be contacted shortly regarding the next steps in the process by our team."
            " We look forward to working with you.\n\nThank you,\nHIVE at the U.S. Food and Drug Administration (FDA)\n", reqid);
    } else if( sIsExactly(autoreply, "accreq") ) {
        subj = "Account Request";
        reponse2sender.printf(0, "Thank you for submitting account request #%" UDEC ". You will be contacted shortly regarding the next steps in the process by our team."
            " We look forward to working with you.\n\nThank you,\nHIVE at the U.S. Food and Drug Administration (FDA)\n", reqid);
    }
    bool ok = true;
    if( !email || !email[0] ) {
        error("From email address is required to submit a request.");
        ok = false;
    }
    if( !subj || !subj[0] ) {
        error("Please specify a brief description of the request on the subject line.");
        ok = false;
    }
    if( !body || !body[0] ) {
        error("Message body should contain the description of the request.");
        ok = false;
    }
    if( ok ) {
        if( !m_User.contact(email, subj, body, reponse2sender.ptr()) ) {
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
            if (!raw) {
                alert("Email was sent to %s", email);
            }
        } else {
            if( !m_User.err ) {
                m_User.err.printf(0, "Uknown error has occured.");
            }
            if (!raw) {
                error("%s", m_User.err.ptr());
            }
            redirectURL.cut(0);
        }
    } else {
        redirectURL.cut(0);
    }
    if (raw) {
        sJSONPrinter printer(&dataForm);
        printer.startObject();
        if( !m_User.err ) {
            sStr msg("Email was sent to %s", email);
            printer.addKeyValue("message", msg.ptr());
        } else {
            printer.addKeyValue("error", m_User.err.ptr());
        }
        printer.endObject();
        printer.finish();
    } else {
        outSection("user_forgot");
    }
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
            cookieSet("preset", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%" UDEC, pswd_reset_id);
        }
        if( expires ) {
            cookieSet("xp", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%" UDEC, expires);
        }
        cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
    }
    user_pswd(*this, m_User.QPride(), "user_pswd");
}

void sUsrCGI::passwordEdit(void)
{
    user_pswd(*this, m_User.QPride(), "user_pswd");
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
        cookieSet("emailAct", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, "%s", email);
        user_pswd(*this, m_User.QPride(), "user_pswd");
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
    user_pswd(*this, m_User.QPride(), "user");
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
        idx statusNeed = sUsr::eUserOperational;
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
    cookieSet("fn", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, firstName ? firstName : "");
    cookieSet("ln", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, lastName ? lastName : "");
    cookieSet("ugrp", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, m_User.groupList(false));
    cookieSet("ugrp_all", sCGI::eCookie_Secure | sCGI::eCookie_SameSiteStrict, m_User.groupList(true));
    if( !m_User.Id() || m_User.isGuest() ) {
        cookieSet("newreg", "1");
    }
    user_pswd(*this, m_User.QPride(), "user");
    return;
}


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

    sDic<idx> parent_ids;
    for(const sUsrType2 * t = utype; t; ) {
        sHiveId id(t->id());
        idx * pseen = parent_ids.get(&id, sizeof(id));
        if( pseen && *pseen ) {
            break;
        }
        *parent_ids.set(&id, sizeof(id)) = 1;
        const sUsrType2 * par = 0;
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

    sVec<sHiveId> ids;
    sStr ids_buf;
    for(idx iparent = 0; iparent < utype->dimParents(); iparent++) {
        *ids.add(1) = utype->getParent(iparent)->id();
    }
    sHiveId::printVec(ids_buf, ids);
    tbl->addCell(sString::escapeForCSV(csv_buf, ids_buf));

    ids.cut(0);
    ids_buf.cut0cut();
    for(idx iinc = 0; iinc < utype->dimIncludes(); iinc++) {
        *ids.add(1) = utype->getInclude(iinc)->id();
    }
    sHiveId::printVec(ids_buf, ids);
    tbl->addCell(sString::escapeForCSV(csv_buf, ids_buf));

    tbl->addCell(sString::escapeForCSV(csv_buf, path_buf));

    tbl->addEndRow();
}

void sUsrCGI::typeTree(void)
{
    raw |= 1;
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
    sUsrType2::find(m_User, 0, pForm->value("type", ".*"), typeTreeFindCb, &param, 0, true);
    tbl.finish();
    tbl.printCSV(dataForm);
    if( param.search_re ) {
        regfree(param.search_re);
    }
}

void sUsrCGI::objs(void)
{
    raw |= 1;
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
        sUsrFolder * shared = (strcmp(pProp, "child") == 0 ? sSysFolder::SharedWithMe(m_User, false) : 0);
        for(idx i = 0; i < pis.dim(); ++i) {
            sUsrObj * obj = m_User.objFactory(pis[i]);
            if( obj ) {
                if( shared && obj->Id() == shared->Id() ) {
                    sVec<sHiveId> ids;
                    sStr gid("%" UDEC, m_User.groupId());
                    shared->orphans(this->m_User, ids, 0, "!_creator", gid.ptr());
                    for(idx i = 0; i < ids.dim(); ++i) {
                        filter.printf(",%s", ids[i].print());
                        prop_name.printf(",%s", pVal);
                    }
                } else {
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
                delete obj;
            }
        }
        delete shared;
        if(!filter) {
            qty = 0;
        }
    }
    sStr typeBuf;
    if( search ) {
        const char * p = search;
        do {
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
    udx total_qty = 0;
    if( qty ) {
        m_User.objs2(type_name, v, show_info ? &total_qty : 0, prop_name ? prop_name.ptr(1) : 0, filter ? filter.ptr(1) : 0, props, pForm->boolvalue("perm"), from, cnt, false, showTrash);
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
            dataForm.printf("\ninfo.0.total=%" UDEC "\ninfo.0.start=%" UDEC, total_qty, from);
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

sUsrQueryEngine* sUsrCGI::queryEngineFactory(idx flags)
{
#if _DEBUG
    return new sUsrInternalQueryEngine(m_User.QPride(), m_User, flags);
#else
    return new sUsrQueryEngine(m_User, flags);
#endif
}

void sUsrCGI::objQry(sVariant *dst, const char *qry_text)
{
    raw |= 1;
    if( !qry_text ) {
        qry_text = pForm->value("qry");
    }
    if( !qry_text ) {
        return;
    }
    const bool silentErr = sString::parseBool(pForm->value("silentErr"));
    const bool is_json = strcasecmp("json", pForm->value("mode", "")) == 0;

    idx ctx_flags = 0;
    if( pForm->boolvalue("_id") ) {
        ctx_flags |= qlang::sUsrContext::fUnderscoreId;
    }

    sUsrQueryEngine *engine = queryEngineFactory(ctx_flags);
    sVariant local_result;
    sVariant *presult = dst ? dst : &local_result;
    sStr error;

#ifndef WIN32
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = rlimitHandler;
    sigaction(SIGXCPU, &sa, NULL);

    struct rlimit rlim;
    rlim.rlim_cur = 30;
    rlim.rlim_max = rlim.rlim_cur + 10;
    setrlimit(RLIMIT_CPU, &rlim);
    rlim.rlim_cur = 1 << 31;
    rlim.rlim_max = rlim.rlim_cur + (1 << 29);
    setrlimit(RLIMIT_DATA, &rlim);
    setrlimit(RLIMIT_STACK, &rlim);
#endif

    if( !engine->parse(qry_text, 0, &error) ) {
        if (!silentErr) {
#ifdef _DEBUG
            dataForm.printf("error : failed to parse query : %s", error.ptr());
#else
            dataForm.printf("error : failed to parse query");
#endif
        }
        delete engine;
        return;
    }
    if( !engine->eval(qry_text, 0, *presult, &error) ) {
        if (!silentErr) {
#ifdef _DEBUG
            dataForm.printf("error : failed while running query : %s", error.ptr());
#else
            dataForm.printf("error : failed while running query");
#endif
        }
        delete engine;
        return;
    }

    if( !dst ) {
        if( is_json ) {
            presult->print(dataForm, sVariant::eJSON);
        } else {
            dataForm.printf("%s", presult->asString());
        }
    }
    delete engine;
}

void sUsrCGI::objDel(void)
{
    raw |= 1;
    sQPrideBase * qp = m_User.QPride();
    idx reqId = qp?qp->reqId:0;

    const char* obj_ids = pForm->value("ids");
    if( obj_ids && obj_ids[0] ) {
        sUsrObjRes parents;
        m_User.objs2("sysfolder,folder", parents, 0, "child", obj_ids);
        sVec<sHiveId> ids;
        sHiveId::parseRangeSet(ids, obj_ids);
        for(idx i = 0; i < ids.dim(); ++i) {
            std::unique_ptr<sUsrObj> obj(m_User.objFactory(ids[i]));
            if( obj.get() && obj->actDelete() ) {
                for(sUsrObjRes::IdIter it = parents.first(); parents.has(it); parents.next(it)) {
                    std::unique_ptr<sUsrObj> p_obj(m_User.objFactory(*parents.id(it)));
                    sUsrFolder * p_src_obj = ((sUsrFolder*) p_obj.get());
                    if(reqId) {
                        p_src_obj->progress_CallbackFunction = sQPrideBase::reqProgressStatic;
                        p_src_obj->progress_CallbackParamObject = qp;
                        p_src_obj->progress_CallbackParamReqID = &reqId;
                    }
                    p_src_obj->detach(*obj.get());
                }
            }
            if(reqId)qp->reqProgress(reqId,2,i,i,ids.dim());
        }
    }
}

void sUsrCGI::objRemove(void)
{
    raw |= 1;
    const char * obj_ids = pForm->value("ids");
    const char * src_id = pForm->value("src");

    sVec<sHiveId> s_ids;
    if( src_id && strcmp(src_id, "root") == 0 ) {
        s_ids.add();
    } else if( src_id && strcmp(src_id, "all") == 0 ) {
        sUsrObjRes res;
        m_User.objs2("sysfolder,folder", res, 0, "child", obj_ids);
        for( sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it) ) {
            *(s_ids.add()) = *res.id(it);
        }
    } else if( src_id ) {
        s_ids.add()->parse(src_id);
    }
    if( !s_ids.dim() ) {
        s_ids.add();
    }

    sQPrideBase * qp = m_User.QPride();
    idx reqId = qp ? qp->reqId : 0;

    sVec<sHiveId> ids, copied_ids;
    sHiveId::parseRangeSet(ids, obj_ids);
    sVec<sHiveId> * p_ids = &ids;
    dataForm.printf("{");
    for( idx i = 0; i < s_ids.dim(); ++i ) {
        if( !s_ids[i] ) {
            sUsrFolder * _inbox = sSysFolder::Inbox(m_User);
            if( _inbox ) {
                _inbox->attachCopy(p_ids, _inbox, 0, &copied_ids, true);
                p_ids = &copied_ids;
                s_ids[i] = _inbox->Id();
                delete _inbox;
            }
        }
        std::unique_ptr<sUsrObj> src_obj(m_User.objFactory(s_ids[i]));
        if( src_obj.get() && src_obj->isTypeOf("(folder|sysfolder)") ) {
            sUsrFolder * p_src_obj = ((sUsrFolder *)src_obj.get());
            if( reqId ) {
                p_src_obj->progress_CallbackFunction = sQPrideBase::reqProgressStatic;
                p_src_obj->progress_CallbackParamObject = qp;
                p_src_obj->progress_CallbackParamReqID = &reqId;
            }
            p_src_obj->actRemove(p_ids, &dataForm);
        } else {
            dataForm.printf("\"%s\" : { \"signal\" : \"delete\", \"data\" : { \"error\":\"failed to access source folder\"} } \n,", s_ids[i].print());
        }
        if( reqId ) {
            qp->reqProgress(reqId, 2, i, i, s_ids.dim());
        }
    }
    dataForm.cut(dataForm.length() - 1);
    dataForm.printf("}");
}

void sUsrCGI::objMove(bool isCopy)
{
    raw |= 1;
    const char* obj_ids = pForm->value("ids");
    const char* src_id = pForm->value("src");
    sHiveId dst_id(pForm->value("dest"));
    sVec<sHiveId> ids;
    sHiveId::parseRangeSet(ids, obj_ids);
    if(ids.dim()) {
        sVec<sHiveId> copied_ids, s_ids, *p_ids;
        p_ids = &ids;
        if( src_id && strcmp(src_id, "all") == 0 ) {
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
        std::unique_ptr<sUsrObj> dst_obj(m_User.objFactory(dst_id));
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
                std::unique_ptr<sUsrObj> src_obj(m_User.objFactory(s_ids[i]));
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
    raw |= 1;
    const char* name = pForm->value("name");
    sHiveId id(pForm->value("ids"));
    sUsrFolder * c_obj = 0;
    if( id && name ) {
        std::unique_ptr<sUsrObj> src_obj(m_User.objFactory(id));
        if( src_obj.get() ) {
            if( src_obj->isTypeOf("(folder|sysfolder)") ) {
                c_obj = ((sUsrFolder*) src_obj.get())->createSubFolder(name);
                if( c_obj ) {
                    dataForm.printf("{\"");
                    c_obj->IdStr(&dataForm);
                    dataForm.printf("\" : { \"signal\" : \"folder\", \"data\" : { \"to\":\"");
                    src_obj->IdStr(&dataForm);
                    dataForm.printf("\"} } }");
                }
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
    raw |= 1;
    sDic<udx> list;
    m_User.all(list, pForm->value("type"));
    sVarSet res;
    if( sIs(pForm->value("mode"), "json") ) {
        sJSONPrinter printer(&dataForm);
        printer.startObject();
        printer.addKeyValue("_id", "0");
        printer.addKeyValue("_type", "sysfolder");
        printer.addKey("type_count");
        printer.startObject();
        for(idx i = 0; i < list.dim(); ++i) {
            printer.addKeyValue((const char*) list.id(i), *list.ptr(i));
        }
        printer.endObject();
        printer.endObject();
        printer.finish();
    } else {
        res.setColId(0, "id");
        res.setColId(1, "name");
        res.setColId(2, "path");
        res.setColId(3, "value");
        for(idx i = 0; i < list.dim(); ++i) {
            res.addRow().addCol("all").addCol("type_count").addCol((const char*) list.id(i)).addCol(*list.ptr(i));
        }
        res.printCSV(dataForm);
    }
}

void objCountAdd(sVariant & o, sUsrObj & child, const bool listIds)
{
    sVariant * count = o.getDicElt("count");
    if( !count ) {
        count = o.setElt("count", (idx) 0);
    }
    if( count ) {
        *count += (idx)1;
    }
    if( listIds ) {
        sVariant * ids = o.getDicElt("ids");
        if( !ids) {
            ids = o.setElt("ids", true);
            if( ids ) {
                ids->setList();
            }
        }
        if( ids ) {
            ids->push(child.IdStr());
        }
    }
}

void objCountObj(sUsr & user, const sHiveId & id, sVariant & counts, const bool extend, const bool listIds)
{
    sUsrObj * child = user.objFactory(id);
    bool found = false;
    for(idx i = 0; child && i < counts.dim(); ++i) {
        sVariant * o = counts.getListElt(i);
        if( o && o->isDic() ) {
            sVariant * tt = o->getDicElt("types");
            bool typeNameNotEmpty = false;
            if( tt && tt->isList() ) {
                for(idx j = 0; j < tt->dim(); ++j) {
                    const char * tn = tt->getListElt(j)->asString();
                    typeNameNotEmpty |= tn && tn[0];
                    if( tn && child->isTypeOf(tn) ) {
                        objCountAdd(*o, *child, listIds);
                        found = true;
                    }
                }
            }
            if( !typeNameNotEmpty ) {
                objCountAdd(*o, *child, listIds);
            }
        }
    }
    if( extend && !found && child ) {
        sVariant x;
        x.setDic();
        x.setElt("title", child->getType()->title());
        sVariant tt;
        tt.setList();
        tt.push(child->getTypeName());
        x.setElt("types", tt);
        objCountAdd(x, *child, listIds);
        counts.push(x);
    }
    delete child;
}

void sUsrCGI::objCount(void)
{
    bool success = false;
    sVariant result, rtype_buf;

    sJSONParser parser;
    idx json_parse_txt_len = 0;
    const char * json_parse_txt = pForm->value("parse", 0, &json_parse_txt_len);
    while( parser.parse(json_parse_txt, json_parse_txt_len, 0, 0) ) {
        sVariant & res = parser.result();
        if( res.isDic() ) {
            const char * parent_field = "child";
            sVariant * f = res.getDicElt("parent_field");
            if( f && !f->isNullish() ) {
                parent_field = f->asString();
            }
            f = res.getDicElt("show_other");
            bool extend = true;
            if( f ) {
                extend = f->asBool();
            }
            f = res.getDicElt("list_ids");
            bool listIds = false;
            if( f && !f->isNullish() ) {
                listIds = f->asBool();
            }
            sVariant * rtypes = res.getDicElt("rtypes");
            if( rtypes ) {
                if( !rtypes->isList() ) {
                    break;
                }
                for(idx i = 0; i < rtypes->dim(); ++i) {
                    sVariant * o = rtypes->getListElt(i);
                    if( o && o->isDic() ) {
                        sVariant * count = o->setElt("count", (udx) 0);
                        if( !count ) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            } else {
                rtype_buf.setList();
                rtypes = &rtype_buf;
            }
            if( !rtypes->dim() ) {
                sVariant t;
                t.setDic();
                if( !t.setElt("title", "all") || !t.setElt("count", (udx) 0) ) {
                    break;
                }
            }
            sVec<sHiveId> children;
            sVariant * parents = res.getDicElt("parents");
            if( parents && parents->isList() ) {
                result.setList();
                sDic<char> seen;
                for(idx i = 0; i < parents->dim(); ++i) {
                    sHiveId id(parents->getListElt(i)->asString());
                    if( seen.get(&id, sizeof(sHiveId)) ) {
                        continue;
                    }
                    seen.set(&id, sizeof(sHiveId));
                    sVariant * par = result.push(true);
                    if( !par ) {
                        break;
                    }
                    par->setDic();
                    sVariant * tcount = par->setElt("type_count", true);
                    if( !tcount ) {
                        break;
                    }
                    tcount->clone(*rtypes);
                    if( id ) {
                        sUsrObj * parent = m_User.objFactory(id);
                        if( parent ) {
                            par->setElt("_id", parent->IdStr());
                            par->setElt("_type", parent->getTypeName());
                            sVarSet pvs;
                            children.empty();
                            parent->propGet(parent_field, pvs);
                            for(idx r = 0; r < pvs.rows; ++r) {
                                const char * pv = pvs.val(r, 0);
                                if( pv && pv[0] ) {
                                    sHiveId::parseRangeSet(children, pv);
                                }
                            }
                            for(idx i = 0; i < children.dim(); ++i) {
                                objCountObj(m_User, children[i], *tcount, extend, listIds);
                            }
                            delete parent;
                        } else {
                            break;
                        }
                    } else {
                        par->setElt("_id", 0);
                        par->setElt("_type", "sysfolder");
                        sUsrObjRes out;
                        if( m_User.objs2(0, out, 0, 0, 0, "") ) {
                            for(sUsrObjRes::IdIter it = out.first(); out.has(it); out.next(it)) {
                                const sHiveId * id = out.id(it);
                                if( id ) {
                                    objCountObj(m_User, *id, *tcount, extend, listIds);
                                }
                            }
                        }
                    }
                }
                success = true;
            }
        }
        break;
    }
    if( !success ) {
        headerSet("Status", "400");
    } else {
        const sVariant::Whitespace * ws = 0;
#if _DEBUG
        ws = sVariant::getDefaultWhitespace(sVariant::eJSON);
#else
        sVariant::Whitespace ws1;
        ws1.indent = ws1.space = ws1.newline  = "";
        ws = &ws1;
#endif
        result.print(dataForm, sVariant::eJSON, ws);
    }
}

void sUsrCGI::propSpec(const char* type_name, const char* view_name, const char* props)
{
    raw |= 1;
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
        json_printer.finish();
    }
}

void sUsrCGI::propSpec2(const char* type_name, const char* view_name, const char* props)
{
    raw |= 1;
    bool pretty = pForm->boolvalue("pretty",
#ifdef _DEBUG
        true
#else
        false
#endif
        );

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
        json_printer.init(&dataForm, pretty ? 0 : "", pretty ? 0 : "");
        json_printer.startObject();
        type->printJSON2(m_User, json_printer, true);

        if( sLen(pForm->value("actions", "")) > 0 ) {
            sVec<const sUsrAction*> actions;
            for(idx ia = 0; ia < type->dimActions(m_User); ia++) {
                const sUsrAction * act = type->getAction(m_User, ia);
                if( act && !act->isObjAction() ) {
                    *actions.add(1) = act;
                }
            }
            if( actions.dim() ) {
                json_printer.addKey("_action");
                json_printer.startArray();
                for(idx ia = 0; ia < actions.dim(); ia++) {
                    json_printer.addValue(actions[ia]->name());
                }
                json_printer.endArray();
            }
        }
        json_printer.endObject();
    }
    json_printer.finish();
}

void sUsrCGI::propSpec3(const char * types)
{
    raw |= 1;
    if( !types ) {
        types = pForm->value("types");
    }
    const bool print_actions = sLen(pForm->value("actions", "")) > 0;

    sStr types00;
    if( !types ) {
        sVec<sHiveId> ids;
        sHiveId::parseRangeSet(ids, pForm->value("ids"));
        for(idx i = 0; i < ids.dim(); ++i) {
            sUsrObj * pobj = m_User.objFactory(ids[i]);
            if( pobj && pobj->Id() ) {
                types00.printf("%s", pobj->getTypeName());
                types00.add0();
            }
            delete pobj;
        }
    } else {
        sString::searchAndReplaceSymbols(&types00, types, 0, ",", 0, 0, true, true, true, true);
    }
    types00.add0(2);

    sDic<bool> printed;
    sJSONPrinter json_printer;
    json_printer.init(&dataForm);
    headerSet("Content-Type", "%s", contentTypeByExt(".json"));
    json_printer.startArray();
    for(const char * t = types00; t; t = sString::next00(t) ) {
        if( const sUsrType2 * type = sUsrType2::ensure(m_User, t) ) {
            if( !printed.get(t) ) {
                printed.set(t);
                json_printer.startObject();
                type->printJSON(m_User, json_printer, true);
                if( print_actions ) {
                    sVec<const sUsrAction*> actions;
                    for(idx ia = 0; ia < type->dimActions(m_User); ia++) {
                        const sUsrAction * act = type->getAction(m_User, ia);
                        if( act && !act->isObjAction() ) {
                            *actions.add(1) = act;
                        }
                    }
                    if( actions.dim() ) {
                        json_printer.addKey("_action");
                        json_printer.startArray();
                        for(idx ia = 0; ia < actions.dim(); ia++) {
                            json_printer.addValue(actions[ia]->name());
                        }
                        json_printer.endArray();
                    }
                }
            }
        }
        json_printer.endObject();
    }
    json_printer.endArray();
    json_printer.finish();
}

namespace {
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
    raw |= 1;
    const udx show_actions = pForm->uvalue("actions", 0);
    const udx show_js_components = pForm->uvalue("js_components", 0);
    const bool show_perm = pForm->boolvalue("perm");
    const bool flatten_tree = pForm->boolvalue("flatten");
    idx dim_explicit_objs = 0;
    sUsrObjRes res;
    sVec<sHiveId> explicit_ids;
    if( !obj_ids ) {
        const char * ii = pForm->value("ids");
        const char * obj_qry = pForm->value("objQry");
        if( ii && ii[0] ) {
            sHiveId::parseRangeSet(explicit_ids, ii);
            dim_explicit_objs = explicit_ids.dim();
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
                sUsrType2::find(m_User, 0, type_names_qry, propGet_printTypeNamesCb, &type_names_buf, false, true);
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
    const bool show_files_size = pForm->boolvalue("files_size", false);
    sJSONPrinter my_json_printer;
    if( !json_printer ) {
        json_printer = &my_json_printer;
    }
    if( mode == eCSV ) {
        dataForm.addString("id,name,path,value");
        if( show_files_size ) {
            dataForm.addString(",size");
        }
        dataForm.addString("\n");
    } else if( mode == eJSON && json_printer == &my_json_printer) {
        json_printer->init(&dataForm);
    }
    if( obj_ids ) {
        if( mode == eJSON && dim_explicit_objs != 1 ) {
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
                std::unique_ptr<sUsrObj> obj(m_User.objFactory(*id));
                if( obj.get() ) {
                    if( show_actions ) {
                        sVec<sStr> vec;
                        obj->actions(vec, show_actions > 1);
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
                            sVarSet acts;
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
                    if( show_js_components ) {
                        sVec<sStr> vec;
                        obj->jscomponents(vec, show_js_components > 1);
                        if( mode == eJSON ) {
                            if( vec.dim() ) {
                                json_printer->addKey("_js_component");
                                json_printer->startArray();
                                for(idx i = 0; i < vec.dim(); ++i) {
                                    json_printer->addValue(vec[i].ptr());
                                }
                                json_printer->endArray();
                            }
                        } else {
                            sVarSet jscos;
                            sStr a;
                            for(idx i = 0; i < vec.dim(); ++i) {
                                a.printf(",%s", vec[i].ptr());
                            }
                            if( a ) {
                                jscos.addRow().addCol(obj->Id()).addCol("_js_component").addCol("").addCol(a.ptr(1));
                            }
                            if( mode == eCSV ) {
                                jscos.printCSV(dataForm, 0);
                            } else {
                                jscos.printProp(dataForm, 0, 1, 2, 3);
                            }
                        }
                    }
                    if( show_files_wildcard ) {
                        switch( mode ) {
                            case eCSV:
                                obj->fileProp(dataForm, true, show_files_wildcard, show_files_size);
                                break;
                            case eJSON:
                                obj->fileProp(*json_printer, show_files_wildcard, true, show_files_size) ;
                                break;
                            case eProp:
                                obj->fileProp(dataForm, false, show_files_wildcard, show_files_size);
                                break;
                        }
                    }
                }
            }
            if( mode == eJSON ) {
                json_printer->endObject();
            }
        }
        if( mode == eJSON && dim_explicit_objs != 1 ) {
            json_printer->endArray();
        }
    }
}

void sUsrCGI::propSet(sVar* form)
{
    raw |= 1;
    m_User.propSet(form ? *form : *pForm, dataForm);
}

void sUsrCGI::propSet2(sVar* form)
{
    raw |= 1;
    if( !form ) {
        form = pForm;
    }
    idx json_parse_txt_len = 0;
    const char * json_parse_txt = form->value("parse", 0, &json_parse_txt_len);

    if( m_User.updateStart() ) {
        sDic<sUsrPropSet::Obj> modified_objs;
        sUsrPropSet upropset(m_User);
        upropset.setSrc(json_parse_txt, json_parse_txt_len).disableFileRoot();
        if( upropset.run(&modified_objs, sUsrPropSet::fOverwriteExistingSameType) ) {
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
                dataForm.addString("error: database connection error 2\n");
            }
        } else {
            dataForm.printf("error: %s\n", upropset.getErr());
        }
    } else {
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
    const char * ids = pForm->value("ids");
    const char * obj_qry = pForm->value("objQry");
    const char * fmla = pForm->value("fmla");
    const char * mapfile_id = pForm->value("mapfile");
    const char * prop_name = pForm->value("prop");

    std::unique_ptr<sUsrQueryEngine> engine(queryEngineFactory(qlang::sUsrContext::fUnderscoreId));
    sStr error, result, cell, path;
    sVariant value;
    raw |= 1;

#ifndef WIN32
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
        if( !engine->eval(obj_qry, 0, value, &error) ) {
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
    sTxtTbl mapfile_tbl;
    sDic<idx> mapfile_dic;
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
        if( !engine->eval(fmla, 0, value, &error) ) {
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
    raw |= 1;
    const char* objIds = pForm->value("ids");
    const char* prop = pForm->value("prop");
    const char* path = pForm->value("path");
    const char* value = pForm->value("value");

    sVec<sHiveId> os;
    sHiveId::parseRangeSet(os, objIds);
    for(idx o = 0; o < os.dim(); ++o) {
        std::unique_ptr<sUsrObj> obj(m_User.objFactory(os[o]));
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
    raw |= 1;
    const char* objIds = pForm->value("ids");
    const char* type = pForm->value("type");

    if( type && type[0] ) {
        sVec<sHiveId> os;
        sHiveId::parseRangeSet(os, objIds);
        dataForm.printf("{");
        for(idx o = 0; o < os.dim(); ++o) {
            dataForm.printf("%s\n \"%s\" : ", o > 0 ? "," : "", os[o].print());
            std::unique_ptr<sUsrObj> obj(m_User.objFactory(os[o]));
            if( obj.get() ) {
                sStr type_from;
                type_from.addString(obj->getTypeName());
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
    raw |= 1;
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
    raw |= 1;
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
    raw |= 1;
    if( !m_User.isGuest() ) {
        const char* nm = pForm->value("name");
        const char* abbr = pForm->value("abbr");
        const char* parent = pForm->value("parent");
        if( !m_User.groupCreate(nm, abbr, parent) && !m_User.err ) {
            m_User.err.printf("Request unsuccessful, try again later.");
        }
        dataForm.printf("%s", m_User.err ? m_User.err.ptr() : "ok");
    }
}


void sUsrCGI::userList(void)
{
    raw |= 1;
    sJSONPrinter json_printer;
    const bool printJson = pForm->boolvalue("json");
    if( printJson ) {
        json_printer.init(&dataForm);
        json_printer.startArray();
    } else {
        dataForm.printf("id,path,name\n");
    }
    idx isGrp = pForm->ivalue("ugrp");
    if( !m_User.isGuest() || isGrp ) {
        idx allUsers = pForm->ivalue("usr");
        idx active = pForm->ivalue("active");
        bool billable = pForm->boolvalue("billable");
        bool with_system = pForm->boolvalue("withSystem");
        const char * search = pForm->value("search");
        bool primaryGrpOnly = pForm->boolvalue("primaryGrpOnly");
        sVec<sStr> lst;
        m_User.listUsr(&lst, isGrp, allUsers, active, search, primaryGrpOnly, billable, with_system);
        for(idx i = 0; i < lst.dim(); ++i) {
            const char * userName = lst[i].ptr();
            const char * groupPath = sString::next00(userName);
            const char * id = sString::next00(groupPath);
            if( printJson ) {
                json_printer.startObject();
                json_printer.addKeyValue("id", id);
                json_printer.addKeyValue("path", groupPath);
                json_printer.addKeyValue("name", userName);
                json_printer.endObject();
            } else {
                dataForm.printf("%s,%s,%s\n", id, groupPath, userName);
            }
        }
    }
    if( printJson ) {
        json_printer.endArray();
        json_printer.finish();
    }
}

void sUsrCGI::userInfo(void)
{
    raw |= 1;
    bool pretty = pForm->boolvalue("pretty",
#ifdef _DEBUG
        true
#else
        false
#endif
        );
    const char * prefix = pForm->value("prefix");
    bool with_inactive = pForm->boolvalue("active");
    bool with_system = pForm->boolvalue("withSystem");

    sJSONPrinter json_printer;
    json_printer.init(&dataForm, pretty ? 0 : "", pretty ? 0 : "");
    json_printer.startObject();
    m_User.printUserInfo(json_printer, true, 0, false, prefix, with_inactive, with_system);
    json_printer.endObject();
    json_printer.finish();
}

void sUsrCGI::groupList(void)
{
    raw |= 1;
    if( !m_User.isGuest() ) {
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
}

void sUsrCGI::inactList()
{
    raw |= 1;
    if( !m_User.isGuest() ) {
        idx isGrp = pForm->ivalue("ugrp");

        sVec<sStr> lst;
        m_User.listInactive(&lst, isGrp);
        for(idx i = 0; i < lst.dim(); ++i) {
            dataForm.printf("%s\n", lst[i].ptr());
        }
    }
}

void sUsrCGI::file2(void)
{
    const char * r = pForm->value("HTTP_RANGE", NULL);

    if( !r || !r[0] ) {
        file();
        return;
    }
    sStr rangeStr("%s", r);
    const char * obj_ids = pForm->value("ids");
    const char * filename = pForm->value("filename");
    const char * propname = pForm->value("propname");
    idx prefix = pForm->ivalue("prefix", 1);

    sVec<sHiveId> ids;
    sHiveId::parseRangeSet(ids, obj_ids);
    sStr log;
    for( idx i = 0; i < ids.dim(); ++i ) {
        std::unique_ptr<sUsrObj> obj(m_User.objFactory(ids[i], 0, ePermCanDownload));
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
            } else if( const sUsrFile * u_file = dynamic_cast<const sUsrFile *>(obj.get()) ) {
                u_file->getFile(path);
                sStr fnm, ext;
                if( path && u_file->propGet("path") ) {
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
                    if( raw < 1 )
                        raw = 1;

                    sHtml::sPartPair::TParts ranges = sHtml::parseRange(rangeStr, fl.length());
                    if( ranges.dim() == 0 ) {
                        headerSet("Status", "416");
                        return;
                    } else {
                        idx totalBufferSize = outBinByteRange(&path, nm.ptr(), &ranges);
                        if( totalBufferSize != -1 ) {
                            m_User.audit(sUsr::eUserAuditActions, "download", "objID='%s'; file='%s'; bytes='%" DEC "'", obj->Id().print(), nm.ptr(), totalBufferSize);
                        }
                    }
                } else {
                    error("Referred file '%s' is not found.", nm.ptr());
                }
            }
        }
    }
}

void sUsrCGI::file(void)
{
    const char * obj_ids = pForm->value("ids");
    const char * filename = pForm->value("filename");
    const char * propname = pForm->value("propname");
    idx maxSize = pForm->ivalue("maxSize", sIdxMax);
    idx offset = pForm->ivalue("offset", 0);
    const char * ellipsize = pForm->value("ellipsize");
    idx prefix = pForm->ivalue("prefix",1);

    sVec<sHiveId> ids;
    sHiveId::parseRangeSet(ids, obj_ids);
    sStr log;
    for(idx i = 0; i < ids.dim(); ++i) {
        std::unique_ptr<sUsrObj> obj(m_User.objFactory(ids[i], 0, ePermCanDownload));
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
                    if( raw < 1 ) {
                        raw |= 1;
                    }
                    sStr fl_ellipsized;

                    const char * fl_buf = fl.ptr();
                    idx fl_buf_len = fl.length();
                    if( offset > 0 ) {
                        offset = sMin<idx>(offset, fl_buf_len);
                        fl_buf += offset;
                        fl_buf_len -= offset;
                    } else if( offset < 0 && -offset <= fl_buf_len ) {
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
                    m_User.audit(sUsr::eUserAuditActions, "download", "objID='%s'; file='%s'; bytes='%" DEC "'", obj->Id().print(), nm.ptr(), fl_buf_len);
                } else {
                    error("Referred file '%s' is not found.", nm.ptr());
                }
            }
        }
    }
}

void sUsrCGI::dropboxList(void)
{
    raw |= 1;
    const sHiveId dbid(pForm->value("dropbox"));
    const char * path = dbid ? pForm->value("path") : 0;
    const char * search = pForm->value("search");

    sVarSet res;
    res.setColId(0, "id");
    res.setColId(1, "name");
    res.setColId(2, "size");
    sUsrObjRes dropboxList;
    m_User.objs2("dropbox", dropboxList, 0, 0, 0, "dropbox_path,dropbox_name");
    sStr dbpath, dbname;
    sDir lst;
    for(sUsrObjRes::IdIter it = dropboxList.first(); dropboxList.has(it); dropboxList.next(it)) {
        const sHiveId * id = dropboxList.id(it);
        if( dbid && id && *id && dbid != *id ) {
            continue;
        }
        const sUsrObjRes::TObjProp * prop = dropboxList.get(it);
        if( prop ) {
            dbname.cut0cut();
            const sUsrObjRes::TPropTbl * dbp = dropboxList.get(*prop, "dropbox_path");
            if( dbp ) {
                dbpath.printf(0, "%s", dropboxList.getValue(dbp));
            }
            if( const sUsrObjRes::TPropTbl * t = dropboxList.get(*prop, "dropbox_name") ) {
                sStr x("%s", dropboxList.getValue(t));
                sString::searchAndReplaceStrings(&dbname, x, x.length(), "/" __, "\\/" __, 0, false);
            }
            if( !dbp || !dbpath || !dbname ) {
                continue;
            }
            if( path ) {
                dbpath.printf("/%s", path);
            }
            idx flags = sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs) | sFlag(sDir::bitSubdirSlash);
            if( search && search[0] ) {
                flags |= sFlag(sDir::bitRecursive);
            }
            lst.cut();
            lst.find(flags, dbpath.ptr(), search, 0, 0, dropboxList.getValue(dbp));
            for(idx ie = 0; ie < lst.dimEntries(); ++ie) {
                dbpath.printf(0, "/%s/%s", dbname.ptr(), lst.getEntryPath(ie));
                const char * abs_path = lst.getEntryAbsPath(ie);
                res.addRow();
                res.addCol(*id);
                res.addCol(dbpath);
                if( sDir::exists(abs_path) ) {
                    res.addCol((idx)-1);
                } else {
                    res.addCol(sFile::size(abs_path));
                }
            }
        }
    }
    if( !res.rows ) {
        res.cols = 3;
    }
    res.printCSV(dataForm);

}

static bool validDate(const char * s) {
    struct tm tmp;
    return sLen(s) == 10 && sString::parseDateTime(&tmp, s);
}

static bool validHours(const char * s) {
    char * endptr = 0;
    real val = s ? strtod(s, &endptr) : 0;
    return val > 0 && *endptr == 0;
}

static bool timelogAddEntry_pushValue(sUsrObj * obj, sUsrObjPropsTree & tree, idx inode_parent, const char * prop, const char * value)
{
    bool success = false;
    if( sUsrObjPropsNode * parent_node = tree.getNodeByIndex(inode_parent) ) {
        if( const sUsrObjPropsNode * node = parent_node->push(prop, value) ) {
            parent_node = 0;
            const char * path = node->path();
            success = obj->propSet(prop, &path, &value, 1, true);
        }
    }
    return success;
}

void sUsrCGI::timelogAddEntry(void)
{
    const char * start_date = pForm->value("start_date");
    if( !validDate(start_date) ) {
        error("Invalid start_date");
        return;
    }
    const char * end_date = pForm->value("end_date");
    if( !validDate(end_date) || strcmp(start_date, end_date) > 0 ) {
        error("Invalid end_date");
        return;
    }
    const char * hours = pForm->value("hours");
    if( !validHours(hours) ) {
        error("Invalid hours");
        return;
    }
    const char * comment = pForm->value("comment");
    sHiveId project_id(pForm->value("project_id"));
    std::unique_ptr<sUsrObj> obj(m_User.objFactory(project_id));
    if( !obj.get() || !obj->isTypeOf("HIVE_Development_Project_List+") ) {
        error("Invalid project");
        return;
    }
    sUsrObjPropsTree props_tree(m_User, obj->getTypeName());

    m_User.updateStart();
    obj->propBulk(props_tree.getTable(), 0, "time_log_reporter" _ "time_log_date" _ "time_log_end_date" _ "time_log_hours" _ "time_log_comment" __);
    props_tree.useTable(props_tree.getTable());

    bool success = false;
    if( const sUsrObjPropsNode * node_parent = props_tree.push("_row_time_log") ) {
        idx inode_parent = node_parent->treeIndex();
        node_parent = 0;
        success = timelogAddEntry_pushValue(obj.get(), props_tree, inode_parent, "time_log_reporter", m_User.Email());
        success = success && timelogAddEntry_pushValue(obj.get(), props_tree, inode_parent, "time_log_date", start_date);
        success = success && timelogAddEntry_pushValue(obj.get(), props_tree, inode_parent, "time_log_end_date", end_date);
        success = success && timelogAddEntry_pushValue(obj.get(), props_tree, inode_parent, "time_log_hours", hours);
        if( comment ) {
            success = success && timelogAddEntry_pushValue(obj.get(), props_tree, inode_parent, "time_log_comment", comment);
        }
    }
    if( success ) {
        m_User.updateComplete();
        dataForm.printf("%s", project_id.print());
    } else {
        m_User.updateAbandon();
        error("Failed to add log entry");
        return;
    }

    printf("%s", project_id.print());
}

void sUsrCGI::loginToken(void)
{
    const char * email = pForm->value("email");
    const char * token = pForm->value("token");

    idx logCount = -1;
    sUsr::ELoginResult eres = sUsr::eUserNotSet;
    if( email && token ) {
        eres = m_User.loginByToken(email, token, pForm->value("ADDR"), &logCount);
    }
    loginCommon(eres, email, logCount);
}

void sUsrCGI::genToken(void)
{
    const char * name = pForm->value("name", "Token");
    const char * expir = pForm->value("expir");
    idx expir_unix = -sIdxMax;

    if( expir ) {
        struct tm tmp;
        expir_unix = sString::parseDateTime(&tmp, expir);
        if ( expir_unix == -sIdxMax ) {
            error("Invalid token expiratation date");
            return;
        }
    }

    idx keyLen = 30;
    sStr keyBuf;
    keyBuf.printf("%s|%s|%s|", m_User.IdStr(), m_User.firstName(), m_User.lastName());
    sPassword::cryptoRandomString(keyBuf, keyLen);

    sStr saltBuf;
    idx saltLen = 12;
    sPassword::cryptoRandomString(saltBuf, saltLen);

    sStr tokenBuf;
    idx iterations = sPBKDF2_HMAC<sSHA256>::default_iterations;
    sPBKDF2_HMAC<sSHA256> pbkdf2(keyBuf.ptr(), saltBuf.ptr(0), saltLen, iterations);
    pbkdf2.printBase64(tokenBuf);

    sStr hashBuf;
    sPassword::encodePassword(hashBuf, tokenBuf.ptr());

    sUsrObjRes us;
    m_User.objs2("user-settings", us);
    if( us.dim() != 1 ) {
        error("Unable to find user-settings object");
        return;
    }
    const sHiveId * id = us.firstId();
    if ( !id ) {
        error("Unable to retreive user-settings object id");
        return;
    }
    sUsrObj * uobj = m_User.objFactory(*id);
    if ( !uobj ) {
        error("Unable to create user-settings object");
        return;
    }

    sUsrObjPropsTree props_tree(m_User, uobj->getTypeName());
    m_User.updateStart();
    uobj->propBulk(props_tree.getTable(), 0, "account_token_name" _ "account_token_hash" _ "account_token_expiration" __);
    props_tree.useTable(props_tree.getTable());

    bool success = false;
    if( const sUsrObjPropsNode * node_parent = props_tree.push("account_token") ) {
        idx inode_parent = node_parent->treeIndex();
        node_parent = 0;
        success = timelogAddEntry_pushValue(uobj, props_tree, inode_parent, "account_token_name", name);
        success = success && timelogAddEntry_pushValue(uobj, props_tree, inode_parent, "account_token_hash", hashBuf.ptr());
        if ( expir ) {
            sStr expirBuf;
            expirBuf.printf("%lld", expir_unix);
            success = success && timelogAddEntry_pushValue(uobj, props_tree, inode_parent, "account_token_expiration", expirBuf.ptr(0));
        }
        sStr time_buf;
        time_t curr_time = time(0);
        time_buf.printf("%ld", curr_time);
        success = success && timelogAddEntry_pushValue(uobj, props_tree, inode_parent, "account_token_created", time_buf.ptr());
    }

    if( success ) {
        m_User.updateComplete();
        dataForm.printf("%s", tokenBuf.ptr(0));
    } else {
        m_User.updateAbandon();
        error("Failed to add token to user-settings");
        return;
    }
}

void sUsrCGI::ionObjList(void)
{
}

void sUsrCGI::ionPropGet(void)
{
}

void sUsrCGI::createProject(void)
{
    sStr errBuf;
    sHiveId outId = 0;
    const char * name = pForm->value("name", 0);
    const char * description = pForm->value("description", 0);
    const char * type = pForm->value("type", 0);
    if( !name || !name[0] ) {
        errBuf.printf("Must give non-empty parameter 'name'. ");
        headerSet("Status", "400");
    }
    if( !description || !description[0] ) {
        errBuf.printf("Must give non-empty parameter 'description'. ");
        headerSet("Status", "400");
    }
    if( !type || !type[0] ) {
        errBuf.printf("Must give non-empty parameter 'type'. ");
        headerSet("Status", "400");
    }
    if( !errBuf ) {
        if( sRC rc = m_User.createProject(outId, name, description, type) ) {
            if( rc.val.parts.bad_entity == sRC::eUser && rc.val.parts.state == sRC::eNotAuthorized ) {
                errBuf.printf("Not authorized to create projects. ");
                headerSet("Status", "403");
            } else if( rc.val.parts.bad_entity == sRC::eType && rc.val.parts.state == sRC::eInvalid ) {
                errBuf.printf("Given type must descend from type 'project'. ");
                headerSet("Status", "400");
            } else {
                errBuf.printf("Internal server error: %s ", rc.print());
                headerSet("Status", "500");
            }
        }
    }
    sJSONPrinter printer(&dataForm);
    printer.startObject();
    printer.addKeyValue("projectID", outId.print());
    if( errBuf ) {
        printer.addKeyValue("error", errBuf.ptr(0));
    }
    printer.endObject();
    printer.finish();
}

void sUsrCGI::availableProjects(void)
{
    sStr errBuf;
    sStr buf;
    sVec<sUsr::Project> projs;
    sJSONPrinter printer(&dataForm);
    printer.startObject();

    if( sRC rc = m_User.availableProjects(projs) ) {
        errBuf.printf("Internal server error: %s ", rc.print());
        headerSet("Status", "500");
    }
    printer.addKey("data");
    printer.startObject();
    for (int i = 0; !errBuf && i < projs.dim(); i++) {
        buf.cut(0);
        buf.printf("%" UDEC, projs[i].projectId);
        printer.addKey(buf.ptr(0));
        buf.cut(0);
        printer.startObject();

        printer.addKey("role");
        switch( projs[i].role ) {
            case sUsr::eProjectViewer:      printer.addKeyValue("role", "Viewer"); break;
            case sUsr::eProjectDataHandler: printer.addKeyValue("role", "DataHandler"); break;
            case sUsr::eProjectContributer: printer.addKeyValue("role", "Contributor"); break;
            case sUsr::eProjectAdmin:       printer.addKeyValue("role", "Admin"); break;
            default:
                errBuf.printf("Internal server error: Unknown role for project %" UDEC, projs[i].projectId);
                headerSet("Status", "500");
                break;
        }
        const bool oldSUMode = m_User.m_SuperUserMode;
        m_User.m_SuperUserMode = true;
        sUsrObj projObj(m_User, sHiveId(projs[i].projectId, 0));
        printer.addKeyValue("Title", projObj.propGet("Title"));
        printer.addKeyValue("Description_full", projObj.propGet("Description_full"));
        printer.addKeyValue("created", projObj.propGet("created"));
        m_User.m_SuperUserMode = oldSUMode;
        printer.endObject();
    }
    printer.endObject();

    if( errBuf ) {
        printer.addKeyValue("error", errBuf.ptr(0));
    }
    printer.endObject();
    printer.finish();
}

void sUsrCGI::projectMembers(void)
{
    sStr buf;
    sStr errBuf;
    sVec<sUsr::ProjectMember> projMembers;
    if( sRC rc = m_User.projectMembers(projMembers) ) {
        errBuf.printf("Internal server error: %s ", rc.print());
        headerSet("Status", "500");
    }
    sJSONPrinter printer(&dataForm);
    printer.startObject();
    printer.addKey("data");
    printer.startObject();
    for (int i = 0; !errBuf && i < projMembers.dim(); i++) {
        buf.cut(0);
        buf.printf("%" UDEC, projMembers[i].userId);
        printer.addKey(buf.ptr(0));
        buf.cut(0);
        switch( projMembers[i].role) {
            case sUsr::eProjectViewer:      buf.printf("Viewer"); break;
            case sUsr::eProjectDataHandler: buf.printf("DataHandler"); break;
            case sUsr::eProjectContributer: buf.printf("Contributor"); break;
            case sUsr::eProjectAdmin:       buf.printf("Admin"); break;
            default:
                errBuf.printf("Internal server error: Unknown role for project %" UDEC, m_User.getProject());
                headerSet("Status", "500");
                break;
        }
        printer.addValue(buf.ptr(0));
    }
    printer.endObject();
    if( errBuf ) {
        printer.addKeyValue("error", errBuf.ptr(0));
    }
    printer.endObject();
    printer.finish();
}

void sUsrCGI::addToProject(void)
{
    const udx userId = pForm->uvalue("userId", 0);
    const char * roleStr = pForm->value("role", 0);
    sStr errBuf;
    sUsr::EProjectRole role = sUsr::eProjectViewer;

    if( !userId ) {
        errBuf.printf("Must give non-zero parameter 'userId'. ");
        headerSet("Status", "400");
    }
    if( !roleStr || !roleStr[0] ) {
        errBuf.printf("Must give non-empty parameter 'role'. ");
        headerSet("Status", "400");
    }
    if( !errBuf ) {
        switch( roleStr[0] ) {
            case 'A': role = sUsr::eProjectAdmin; break;
            case 'C': role = sUsr::eProjectContributer; break;
            case 'D': role = sUsr::eProjectDataHandler; break;
            case 'V': role = sUsr::eProjectViewer; break;
            default:
                errBuf.printf("Parameter 'role' must be one of 'A','C','D', or 'V'");
                headerSet("Status", "400");
                break;
        }
    }
    if( !errBuf ) {
        if( sRC rc = m_User.addToProject(userId, role) ) {
            if( rc.val.parts.bad_entity == sRC::eUser && rc.val.parts.state == sRC::eNotAuthorized ) {
                errBuf.printf("Not authorized to add users to project. ");
                headerSet("Status", "403");
            } else if( rc.val.parts.bad_entity == sRC::eID && rc.val.parts.state == sRC::eNotFound ) {
                errBuf.printf("User ID not found. ");
                headerSet("Status", "400");
            } else if( rc.val.parts.bad_entity == sRC::eGroup && rc.val.parts.state == sRC::eExists ) {
                errBuf.printf("User is already a member of the project. Remove user first. ");
                headerSet("Status", "400");
            } else {
                errBuf.printf("Internal server error: %s ", rc.print());
                headerSet("Status", "500");
            }
        }
    }

    sJSONPrinter printer(&dataForm);
    printer.startObject();
    if( errBuf ) {
        printer.addKeyValue("error", errBuf.ptr(0));
    }
    printer.endObject();
    printer.finish();
}

void sUsrCGI::removeFromProject(void)
{
    sStr errBuf;
    const udx userId = pForm->uvalue("userId", 0);

    if( !userId ) {
        errBuf.printf("Must give non-zero parameter 'userId'. ");
        headerSet("Status", "400");
    }
    if( !errBuf ) {
        if( sRC rc = m_User.removeFromProject(userId) ) {
            if( rc.val.parts.bad_entity == sRC::eUser && rc.val.parts.state == sRC::eNotAuthorized ) {
                errBuf.printf("Not authorized to remove users from project. ");
                headerSet("Status", "403");
            } else if( rc.val.parts.bad_entity == sRC::eGroup && rc.val.parts.state == sRC::eNotFound ) {
                errBuf.printf("Given user is not a member of project. ");
                headerSet("Status", "400");
            } else if( rc.val.parts.bad_entity == sRC::eID && rc.val.parts.state == sRC::eProhibited ) {
                errBuf.printf("Not allowed to remove self from project. Another project administrator can remove you. ");
                headerSet("Status", "400");
            } else {
                errBuf.printf("Internal server error: %s ", rc.print());
                headerSet("Status", "500");
            }
        }
    }

    sJSONPrinter printer(&dataForm);
    printer.startObject();
    if( errBuf ) {
        printer.addKeyValue("error", errBuf.ptr(0));
    }
    printer.endObject();
    printer.finish();
}

void sUsrCGI::moveCheck(void)
{




}

void sUsrCGI::moveToProject(void)
{


}

void sUsrCGI::moveToHome(void)
{


}

bool sUsrCGI::resolveFile(const char * filename)
{

    if( sIs("help/", filename) ) {
        const char * ver = strrchr(filename, '-');
        if( ver ) {
            sStr app("%.*s/%.*s", (int)(ver - filename - 5), &filename[5], (int)(sLen(ver) - 6), &ver[1]);
            return loadAppPage(app.ptr());
        }
    }
    return false;
}

bool sUsrCGI::loadAppPage(const char * app)
{
    const char * help = "hlp.svc-";
    const bool is_help = sIs(help, app);
    sStr app_ver;
    sString::searchAndReplaceSymbols(&app_ver, &app[is_help ? sLen(help) : 0], 0, "/", 0, 0, true, false, true, true);
    app_ver.add0(2);
    if( sString::cnt00(app_ver) >= 2 ) {
        sUsrObjRes v;
        const char * version = sString::next00(app_ver.ptr());
        m_User.objs2("^algorithm$+", v, 0, "name", app_ver.ptr(), "version");
        if( v.dim() ) {
            for( sUsrObjRes::IdIter it = v.first(); v.has(it); v.next(it) ) {
                const sUsrObjRes::TObjProp * prop = v.get(it);
                if( prop ) {
                    if( const sUsrObjRes::TPropTbl * w = v.get(*prop, "version") ) {
                        if( sIs(version, v.getValue(w)) ) {
                            std::unique_ptr<sUsrObj> obj(m_User.objFactory(*v.id(it)));
                            if( obj.get() ) {
                                const char * t = obj->getFilePathname(app_ver, is_help ? "app_help.html" : "app_tmplt.html");
                                if( t ) {
                                    sFil f(t, sMex::fReadonly);
                                    if( f.ok() ) {
                                        if( is_help ) {
                                            dataForm.cutAddString(0, f.ptr(), f.length());
                                            raw = 1;
                                        } else {
                                            htmlBody.cutAddString(0, f.ptr(), f.length());
                                            raw = 0;
                                        }
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

void sUsrCGI::CBERConnect(void)
{
    const char * atype = pForm->value("applicationtype");
    const char * stn = pForm->value("STN");
    if( atype && atype[0] && stn && stn[0] ) {
        IUrlEncoder * enc = CUrl::GetDefaultEncoder();
        if( enc ) {
            if( !m_User.isGuest() ) {
                sHiveId projId;
                sUsrObjRes projs;
                sStr wunit, stn2;
                const bool oldSUMode = m_User.m_SuperUserMode;
                m_User.m_SuperUserMode = true;
                m_User.objs2("^project$+", projs, 0, 0, 0, "work_unit,stn", false);
                m_User.m_SuperUserMode = oldSUMode;
                for(sUsrObjRes::IdIter it = projs.first(); projs.has(it); projs.next(it)) {
                    const sHiveId * id = projs.id(it);
                    if( id && *id == projId ) {
                        projId.reset();
                        break;
                    }
                    const sUsrObjRes::TObjProp * prop = projs.get(it);
                    if( prop ) {
                        if( const sUsrObjRes::TPropTbl * w = projs.get(*prop, "work_unit") ) {
                            wunit.printf(0, "%s", projs.getValue(w));
                        } else {
                            wunit.cut0cut();
                        }
                        if( const sUsrObjRes::TPropTbl * t = projs.get(*prop, "stn") ) {
                            stn2.printf(0, "%s", projs.getValue(t));
                        } else {
                            stn2.cut0cut();
                        }
                        if( !wunit || !stn2 ) {
                            continue;
                        }
                        if( strcasecmp(atype, wunit.ptr()) == 0 && strcasecmp(stn, stn2.ptr()) == 0 ) {
                            projId = *id;
                        }
                    }
                }
                if( projId ) {
                    sVec<sUsr::Project> projects;
                    sRC rc = m_User.availableProjects(projects);
                    bool found = false;
                    if( !rc ) {
                        for(idx i = 0; i < projects.dim(); i++) {
                            if( projects[i].projectId == projId.objId() ) {
                                found = true;
                                break;
                            }
                        }
                    }
                    if( !found ) {
                        projId.reset();
                    }
                }
                if( !projId ) {
                    error("Project %s%s not found or permission denied", atype, stn);
                    executeJS("funcLink('?cmd=home&follow=///r/home');");
                } else {
                    executeJS("projectHandler.setProjectID(%s); funcLink('?cmd=home&follow=///r/home');", projId.print());
                }
            } else {
                redirectURL.printf(0, "login&follow=");
                enc->EncodeArgName("cberconnect&applicationtype=", redirectURL);
                enc->EncodeArgValue(atype, redirectURL);
                enc->EncodeArgName("&STN=", redirectURL);
                enc->EncodeArgValue(stn, redirectURL);
            }
        } else {
            headerSet("Status", "503");
        }
    } else {
        headerSet("Status", "400");
    }
}

idx sUsrCGI::Cmd(const char * cmd)
{
    idx retval = 1;
    enum enumCommands
    {
        eLogin, eLogout, eLoginInfo, eBatch, ePswdSet, ePswdEdit, ePswdChange,
        eUserGet, eUserSet, eUserReg, eUsrList, eUserInfo, eGrpList, eGrpAdd, ePermSet,
        ePropSpec, ePropSpec2, ePropSpec3, ePropGet, eObjs, eObjQry, ePropSet, ePropSet2, ePropBulkSet, ePropDel,
        eSimpleTypeCast, eSendMail, eTypeTree, eFile, eFile2, eDropboxList,
        eUsrV0, eUsrV1, eUsrV2, eUsrV3, eUsrFgt, eObjDel, eInactiveList, eUsrV4, eUsrMgr, ePermCopy, eAllStat,
        eObjRemove, eObjCopy, eObjCut, eObjCount, eFolderCreate, eUsrSSO,
        eIonObjList, eIonPropGet,
        eTimelogAddEntry,
        eGenToken, eLoginToken,
        eAvailableProjects, eProjectMembers, eAddToProject, eRemoveFromProject, eMoveCheck, eMoveToProject, eMoveToHome, eCBERConnect,
        eCreateProject,
        eLast
    };
    const char * listCommands =
        "login" _ "logout" _ "loginInfo" _ "batch" _ "pswdSet" _ "user_pswd" _ "pswdChange" _
        "user" _ "userSet" _ "userReg" _ "usrList" _ "userInfo" _ "grpList" _ "grpAdd" _ "permset" _
        "propspec" _ "propspec2" _ "propspec3" _ "propget" _ "objList" _ "objQry" _ "propset" _ "propset2" _ "propBulkSet" _ "propDel" _
        "scast" _ "sendmail" _ "typeTree" _ "objFile" _ "objFile2" _ "dropboxlist" _
        "userV0" _ "userV1" _ "userV2" _ "userV3" _ "forgot" _ "objDel" _ "inactive" _ "userV4" _ "mgr" _ "permcopy" _ "allStat" _
        "objRemove" _ "objCopy" _ "objCut" _ "objcount" _ "folderCreate" _ "sso" _
        "ionObjList" _ "ionObjGet" _
        "timelogAddEntry" _
        "genToken" _ "loginToken" _
        "availableProjects" _ "projectMembers" _ "addToProject" _ "removeFromProject" _ "moveCheck" _ "moveToProject" _
        "moveToHome" _ "cberconnect" _ "createProject"
        __;

    idx cmdnum = -1;
    if(cmd) {
        sString::compareChoice(cmd, listCommands, &cmdnum, false, 0, true);
    }
    switch(cmdnum) {
        case eLogin:
            login();
            break;
        case eLogout:
            logout();
            break;
        case eLoginInfo:
            loginInfo();
            break;
        case eBatch:
            batch();
            break;
        case ePswdSet:
            passwordReset();
            break;
        case ePswdEdit:
            passwordEdit();
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
        case eUserInfo:
            userInfo();
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
        case eUsrSSO:
            SSO();
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
        case eSendMail:
            sendmail();
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
        case ePropSpec2:
            propSpec2();
            break;
        case ePropSpec3:
            propSpec3();
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
        case eFile2:
            file2();
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
        case eObjCount:
            objCount();
            break;

        case eIonObjList:
            ionObjList();
            break;
        case eIonPropGet:
            ionPropGet();
            break;

        case eTimelogAddEntry:
            timelogAddEntry();
            break;

        case eGenToken:
            genToken();
            break;
        case eLoginToken:
            loginToken();
            break;

        case eAvailableProjects:
            availableProjects();
            break;
        case eProjectMembers:
            projectMembers();
            break;
        case eAddToProject:
            addToProject();
            break;
        case eRemoveFromProject:
            removeFromProject();
            break;
        case eMoveCheck:
            moveCheck();
            break;
        case eMoveToProject:
            moveToProject();
            break;
        case eMoveToHome:
            moveToHome();
            break;
        case eCBERConnect:
            CBERConnect();
            break;
        case eCreateProject:
            createProject();
            break;
        default:
            if( sIs("app/", cmd) && loadAppPage( &cmd[4] ) ) {
                cmdnum = 0;
            } else {
                cmdnum = -1;
            }
            break;
    }
    if( cmdnum == -1 ) {
        retval = TParent::Cmd(cmd);
    } else {
        TParent::outHtml();
    }
    return retval;
}
