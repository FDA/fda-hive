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
#include <slib/core.hpp>
#include <slib/std/cryptohash.hpp>
#include <slib/std/file.hpp>
#include <slib/std/http.hpp>
#include <slib/utils/json/printer.hpp>
#include <slib/utils/sort.hpp>
#include <qlib/QPrideClient.hpp>
#include <ulib/uproc.hpp>
#include <ulib/uquery.hpp>
#include <ulib/usr.hpp>
#include <ulib/upropset.hpp>

static idx __on_help(sQPrideClient * QP, const char * cmd, const char *, const char *, sVar * pForm);

static idx dataint = 0;
static enum {
    eUpsertNone,
    eUpsertAuto,
    eUpsertQry
} want_upsert = eUpsertNone;
static bool want_flatten = false;
static sUsr::EPermExport export_permissions = sUsr::ePermExportGroups;
static sStr update_list_qry;
static sStr prop_exclude00;
static std::unique_ptr<sUsr> g_user;
static sStr respondCommand;
static sStr lastObjList;

static idx __on_vars(sQPrideClient * QP, const char * cmd, const char * arg, const char *, sVar * pForm)
{
    if( sIs(cmd, "-req") ) {
        QP->reqId = pForm->ivalue("req");
    }
    if( sIs(cmd, "-grp") ) {
        QP->grpId = pForm->ivalue("grp");
    }
    if( sIsExactly(cmd, "-job") ) {
        QP->jobId = pForm->ivalue("job");
    }
    if( sIs(cmd, "-user") ) {
        const char * login = pForm->value("login");
        const char * pswd = pForm->value("password");
        if( login ) {
            if( strcasecmp(login, "qpride") == 0 && !pswd) {
                g_user.reset(new sUsr(login));
            } else if( strcasecmp(login, "qapp") == 0 && !pswd) {
                g_user.reset(new sUsr("qpride", true));
            } else {
                g_user.reset(new sUsr);
                g_user->login(login, pswd, "command-line");
            }
        }
        if( g_user.get() && g_user->Id() ) {
            QP->user = g_user.get();
            sStr rootPath;
            sRC rc = sUsrObj::initStorage(QP->cfgStr(&rootPath, 0, "user.rootStoreManager"), QP->cfgInt(0, "user.storageMinKeepFree", (udx)20 * 1024 * 1024 * 1024));
            if( rc ) {
                QP->printf("%s\n", rc.print());
                return 1;
            }
        } else {
            QP->printf("Login failed for user %s\n", login);
            return 1;
        }
    }
    if( sIs(cmd, "-os") ) {
        QP->vars.inp("os", pForm->value("platform"));
    }
    if( sIs(cmd, "-host") ) {
        QP->printf("%s\n%s\n", QP->vars.value("thisHostName"), QP->vars.value("os"));
    }
    if( sIs(cmd, "-logLevel") ) {
        const idx log_level = pForm->ivalue("level", 0);
        QP->setupLog(true, log_level);
    }
    if( sIs(cmd, "-version") ) {
        QP->printf("r$Id$\n");
    }
    if( sIs(cmd, "-dataint") ) {
        dataint = 1 - dataint;
    }
    if( sIs(cmd, "-respond") ) {
        respondCommand.printf(0, "%s", pForm->value("command"));
    }
    if( sIsExactly(cmd, "-upsert") ) {
        if( const char * upsert_how = pForm->value("upsert_how") ) {
            if(strcasecmp(upsert_how, "auto") == 0 ) {
                want_upsert = eUpsertAuto;
            } else if( strcasecmp(upsert_how, "qry") == 0 ) {
                want_upsert = eUpsertQry;
            } else if( strcasecmp(upsert_how, "0") == 0 || strcasecmp(upsert_how, "false") == 0 ) {
                want_upsert = eUpsertNone;
            } else {
                QP->printf("Unknown value for -upsert command; expected 'auto', 'qry', or '0' / 'false'\n");
                return 1;
            }
        }
    }
    if( sIsExactly(cmd, "-flatten") ) {
        want_flatten = true;
    }
    if( sIsExactly(cmd, "-permUser") ) {
        export_permissions = sUsr::ePermExportAll;
    }
    if( sIsExactly(cmd, "-updateList") ) {
        update_list_qry.cutAddString(0, pForm->value("hiveids"));
        update_list_qry.add0(2);
    }
    if( sIsExactly(cmd, "-excludeProp") ) {
        prop_exclude00.shrink00();
        if( prop_exclude00.length() ) {
            prop_exclude00.add0(1);
        }
        prop_exclude00.addString(arg);
        prop_exclude00.add0(2);
    }
    return 0;
}


static idx __on_general(sQPrideClient * QP, const char * cmd, const char * arg, const char * equCmd,sVar * pForm)
{
    if(sIs(cmd,"-shell")){
        sStr t;sPipe::exeSys(&t,arg);t.add0();
        QP->printf("%s",t.ptr());
     }else if(sIs(cmd,"-shall")){
        sVec <sStr> ips;
        QP->getRegisteredIP(&ips,equCmd);
        sStr cmdLine;
        sStr catlist00; sString::searchAndReplaceSymbols(&catlist00,equCmd,0,",",0,0,true,true,true);
        for(idx i = 0; i < ips.dim(); ++i) {
            const char * hostname = ips[i].ptr();
            const char * ipnumeric = sString::next00(hostname);
            QP->printf("HOST <%s %s>: ssh %s '%s'\n", hostname, ipnumeric, ipnumeric, arg);
            cmdLine.printf(0, "ssh %s '%s' ", ipnumeric, arg);
            sStr retval;
            sPipe::exeSys(&retval, cmdLine.ptr(), respondCommand ? respondCommand.ptr() : 0);
            retval.add0();
            if( retval ) {
                QP->printf("%s\n", retval.ptr());
            }
        }
    } else if( sIs(cmd, "-shellW") ) {
        sStr t;
        sPipe::exeSys(&t, pForm->value("exec"));
        t.add0();
        QP->printf("%s", t.ptr());
    } else if( sIs(cmd, "-file") ) {
        sFil ff(pForm->value("flnm"));
        if( ff.length() ) {
            QP->printf("%s", ff.ptr());
        }
    } else if( sIsExactly(cmd, "-init") || sIsExactly(cmd, "-init2") ) {
        if( sIsExactly(cmd, "-init2") ) {
            pForm->inp("login", "qapp");
            __on_vars(QP, "-user", 0, 0, pForm);
        }
        sStr rootPath;
        sRC rc = sUsrObj::initStorage(QP->cfgStr(&rootPath, 0, "user.rootStoreManager"), QP->cfgInt(0, "user.storageMinKeepFree", (udx)20 * 1024 * 1024 * 1024));
        if( rc ) {
            QP->printf("%s\n", rc.print());
            return 1;
        }
        const char * dir = pForm->value("dir", "/QPride/bin/");
        const char * os = QP->vars.value("os");
        QP->resourceSync(dir, "qm", os);
        QP->printf("#Add one the following line to crontab on ALL nodes:\n");
        QP->printf("* * * * * %sqpstart.sh.os%s \n", dir, os);
        QP->printf("#For the daemon running on a farm submit node add \"broadcast 1 psman launcher.sh\"\n");
        QP->printf("* * * * * %sqpstart.sh.os%s \"broadcast 1 psman launcher.sh\"\n", dir, os);
        QP->printf("#On maintenance node add to crontab:\n");
        QP->printf("*/15 * * * * %sqm_maintain.sh.os%s\n", dir, os);
    } else if( sIs(cmd, "-endian") ) {
        sVec<idx> fl(pForm->value("file"));
        idx *f = fl.ptr();
        for(idx i = 0; i < fl.dim(); ++i)
            f[i] = sEndian(f[i]);
    }
    return 0;
}


static idx __on_html(sQPrideClient * QP, const char * cmd, const char * , const char * ,sVar * pForm)
{
    if(sIs(cmd,"-form")){
        sVar rForm;
        QP->reqGetData(QP->reqId,"formT.qpride",&rForm ) ;

        const char * var=pForm->value("variable");
        idx maxsize=pForm->ivalue("maxsize",1024);
        if(var && strcmp(var,"all")==0)var=0;

        for ( idx i=0; i<rForm.dim(); ++i) {
            idx valsize = 0, tailsize = 0;
            const char * variable=(const char *)rForm.id(i);
            const char * value=rForm.value(variable,"(null)",&valsize) ;
            if( var && strcmp(var,variable)!=0 )continue;
            if(valsize>maxsize+3){
                tailsize = valsize - maxsize;
                valsize = maxsize - 3;
                if(tailsize>10)tailsize=10;
            }
            sStr tt;sString::searchAndReplaceSymbols(&tt,value,valsize,"\n"," ",0,true,true,false,true);
            if(tt.length())value=tt.ptr();
            if(var)QP->printf("%s", value);
            else QP->printf("%s = %s",variable, value);
            if(tailsize){
                const char * value2 = rForm.value(variable,"(null)",&valsize);
                sStr ttt;sString::searchAndReplaceSymbols(&ttt,&value2[valsize-tailsize],tailsize,"\n"," ",0,true,true,false,true);
                QP->printf("...%s",ttt.ptr(0));
            }
            QP->printf("\n");
        }
    } else if( sIs(cmd, "-setForm") || sIs(cmd, "-resetForm") ) {
        sVar new_rForm;
        const char * url_query_string = pForm->value("url_query_string");

        if( url_query_string ) {
            sStr buf;
            sVec<sMex::Pos> ofs;
            const char * argv[] = { "qapp", url_query_string };
            sHtml::inputCGI(&buf, &ofs, 0, sDim(argv), argv, &new_rForm, 0, 0, "GET", true);
        }

        if( !sIs(cmd, "-resetForm") ) {
            sVar old_rForm;
            QP->reqGetData(QP->reqId, "formT.qpride", &old_rForm);
            for(idx i=0; i<old_rForm.dim(); i++) {
                const char * key = static_cast<const char*>(old_rForm.id(i));
                if( !new_rForm.is(key) ) {
                    idx size = 0;
                    const char * value = old_rForm.value(key, 0, &size);
                    new_rForm.inp(key, value, size);
                }
            }
        }

        QP->reqSetData(QP->reqId, "formT.qpride", &new_rForm);
    }
    return 0;
}


static idx __on_message(sQPrideClient * QP, const char * cmd, const char * , const char ,sVar * pForm)
{

    if(sIs(cmd,"-messageSubmit")){
        QP->messageSubmit(pForm->value("server"), pForm->value("service"),false, "%s", pForm->value("message"));
    }else if(sIs(cmd,"-messageSubmitToDomainHeader")){
        QP->messageSubmitToDomainHeader("%s", pForm->value("message"));
    }else if(sIs(cmd,"-messageSubmitToDomain")){
        QP->messageSubmitToDomain(pForm->value("service"),"%s",pForm->value("message"));
    }else if(sIs(cmd,"-messageWakePulljob")){
        QP->messageWakePulljob(pForm->value("service"));
    }
    return 0;
}


static idx __on_config(sQPrideClient * QP, const char * cmd, const char * , const char * ,sVar * pForm)
{

    if(sIs(cmd,"-configGet")){
        sStr str;
        QP->cfgStr(&str ,0, pForm->value("parameter"), 0 );
        if(str.length())QP->printf("%s",str.ptr());
    } else if(sIs(cmd,"-configSet")){
        QP->configSet(pForm->value("parameter"), pForm->value("value"));
    }
    else if(sIs(cmd,"-configGetAll")){
        const char * par=pForm->value("parameter");
        if( par && strcmp(par,"all")==0) par=0;
        sStr vals00, csvBuf;
        QP->configGetAll( &vals00, par );
        for( const char * p=vals00.ptr(0) ; p; p=sString::next00(p)){
            const char * nxt=sString::next00(p);
            if(!nxt)break;
            csvBuf.cut(0);
            QP->printf("%s,%s\n", sString::escapeForCSV(csvBuf, p), nxt);
            p=nxt;
        }
    }
    return 0;
}


static idx objQuery(sQPrideClient * QP, const char * args, sDic<bool> & ids, sStr * out_qry = 0) {
    idx rc = 0;
    sUsrInternalQueryEngine qengine(QP, *g_user.get());
    for(const char * id = args; id; id = sString::next00(id)) {
        if( sIs("query://", id) ) {
            sStr lerr;
            const char * qry_txt = id + 8;
            if( !qengine.parse(qry_txt, 0, &lerr) ) {
                rc = 2;
                QP->logOut(QP->eQPLogType_Error, "parse error: '%s', query %s\n", lerr.ptr(), id);
            } else {
                lerr.cut0cut();
                sVariant * v = qengine.run(&lerr);
                if( lerr ) {
                    rc = 3;
                    QP->logOut(QP->eQPLogType_Error, "parse execution: '%s', query %s\n", lerr.ptr(), id);
                } else if( v ) {
                    idx cnt = 0;
                    if( v->isList() ) {
                        for(idx k = 0; k < v->dim(); ++k) {
                            if( v->getListElt(k)->isHiveId() ) {
                                ids.set(v->getListElt(k)->asHiveId(), sizeof(sHiveId));
                                cnt++;
                            }
                        }
                    } else if( v->isHiveId() ) {
                        ids.set(v->asHiveId(), sizeof(sHiveId));
                        cnt++;
                    }

                    if( out_qry ) {
                        out_qry->addString(qry_txt);
                    }
                }
            }
        } else {
            sHiveId oid;
            if( !oid.parse(id) ) {
                rc = 4;
                QP->logOut(QP->eQPLogType_Error, "invalid id '%s'\n", id);
            } else {
                ids.set(&oid, sizeof(oid));
            }
        }
    }
    return rc;
}



static idx __on_udb(sQPrideClient * QP, const char * cmd, const char * args, const char *, sVar * pForm)
{
    idx rc = 0;
    if( !g_user.get() ) {
        rc = 1;
    } else if( sIs("-propJson2Csv", cmd) ) {
        sUsrPropSet upropset(*g_user);
        sStr buf;
        for(const char * flnm00 = args; flnm00; flnm00 = sString::next00(flnm00)) {
            if( flnm00[0] ) {
                upropset.setSrcFile(flnm00);
                sVarSet tbl;
                if( upropset.pretend(tbl, 0, sUsrPropSet::fInvalidUserGroupNonFatal) ) {
                    buf.cut0cut();
                    tbl.printCSV(buf);
                    QP->printf("%s", buf.ptr());
                } else {
                    QP->printf("error: %s\n", upropset.getErr());
                    rc = 14;
                }
            }
        }
    } else if( sIs("-propJson2Prop", cmd) ) {
        sUsrPropSet upropset(*g_user);
        sStr buf;
        for(const char * flnm00 = args; flnm00; flnm00 = sString::next00(flnm00)) {
            if( flnm00[0] ) {
                upropset.setSrcFile(flnm00);
                buf.cut0cut();
                if( upropset.pretendPropFmt(buf, 0, sUsrPropSet::fInvalidUserGroupNonFatal) ) {
                    QP->printf("%s", buf.ptr());
                } else {
                    QP->printf("error: %s\n", upropset.getErr());
                    rc = 14;
                }
            }
        }
    } else if( sIs("-propJson", cmd) ) {
        sDic<bool> update_list;
        if( update_list_qry.length() && update_list_qry[0] ) {
            sStr qry_buf;
            rc = objQuery(QP, update_list_qry, update_list, &qry_buf);
        }

        if( !rc ) {
            for(idx itry = 0; itry < sSql::max_deadlock_retries; itry++) {
                sDic<sUsrPropSet::Obj> modified_objs;
                sUsrPropSet upropset(*g_user);
                bool is_our_transaction = !g_user->getUpdateLevel();
                if( !g_user->updateStart() ) {
                    rc = 11;
                } else {
                    for(const char * flnm00 = args; flnm00; flnm00 = sString::next00(flnm00) ) {
                        if( flnm00[0] ) {
                            upropset.setSrcFile(flnm00);
                            if( !upropset.run(&modified_objs, sUsrPropSet::fInvalidUserGroupNonFatal | sUsrPropSet::fOverwriteExistingSameType) ) {
                                QP->printf("error: %s\n", upropset.getErr());
                                rc = 14;
                            }
                            const char * warn = upropset.getWarn00();
                            if( warn && warn[0] ) {
                                QP->printf("warning: %s\n", warn);
                            }
                        }
                    }
                    if( rc == 0 ) {
                        for(idx iupd = 0; iupd < update_list.dim(); iupd++) {
                            sHiveId id = *static_cast<const sHiveId *>(update_list.id(iupd));
                            bool is_modified_id = false;
                            for(idx imod = 0; imod < modified_objs.dim(); imod++) {
                                if( modified_objs.ptr(imod)->id == id ) {
                                    is_modified_id = true;
                                    break;
                                }
                            }
                            if( !is_modified_id ) {
                                sUsrObj * obj = g_user->objFactory(id);
                                if( obj ) {
                                    if( !obj->purge() ) {
                                        rc = 15;
                                    }
                                    delete obj;
                                }
                            }
#if _DEBUG
                            QP->printf("info: object %s - %s\n", id.print(), is_modified_id ? "updated" : "deleted");
#endif
                        }
#if _DEBUG
                        for(idx imod = 0; imod < modified_objs.dim(); ++imod) {
                            bool found = false;
                            for(idx iupd = 0; iupd < update_list.dim(); ++iupd) {
                                sHiveId id = *static_cast<const sHiveId *>(update_list.id(iupd));
                                if( modified_objs.ptr(imod)->id = id ) {
                                    found = true;
                                    break;
                                }
                            }
                            if( !found ) {
                                QP->printf("info: object %s - created\n", modified_objs.ptr(imod)->id.print());
                            }
                        }
#endif
                    }
                    if( rc == 0 && !g_user->updateComplete() ) {
                        rc = 13;
                    }
                    if( rc != 0 ) {
                        if( g_user->hadDeadlocked() && is_our_transaction ) {
                            g_user->updateAbandon();
                            QP->logOut(QP->eQPLogType_Warning, "DB deadlock detected, retrying attempt %" DEC "/%" DEC "\n", itry + 1, sSql::max_deadlock_retries);
                            sTime::randomSleep(sSql::max_deadlock_wait_usec);
                            continue;
                        } else {
                            g_user->updateAbandon();
                        }
                    }
                }
                break;
            }
        }
    } else if( sIs("-prop", cmd) ) {
        for(idx itry = 0; itry < sSql::max_deadlock_retries; itry++) {
            bool is_our_transaction = !g_user->getUpdateLevel();
            if( !g_user->updateStart() ) {
                rc = 11;
            } else {
                idx fqty = 0;
                for(const char * flnm00 = args; flnm00; flnm00 = sString::next00(flnm00) ) {
                    if( flnm00[0] ) {
                        sStr log;
                        fqty++;
                        if( !g_user->propSet(flnm00, log) ) {
                            rc = 12;
                        }
                        if( log ) {
                            QP->printf("%s", log.ptr());
                        }
                    }
                }
                if( rc == 0 && !g_user->updateComplete() ) {
                    rc = 13;
                }
                if( rc != 0 ) {
                    if( g_user->hadDeadlocked() && is_our_transaction ) {
                        g_user->updateAbandon();
                        QP->logOut(QP->eQPLogType_Warning, "DB deadlock detected, retrying attempt %" DEC "/%" DEC "\n", itry + 1, sSql::max_deadlock_retries);
                        sTime::randomSleep(sSql::max_deadlock_wait_usec);
                        continue;
                    } else {
                        g_user->updateAbandon();
                    }
                }
            }
            break;
        }
    } else if( sIs("-query", cmd) ) {
        sUsrInternalQueryEngine qengine(QP, *g_user.get());
        sStr lerr;
        qengine.parse(pForm->value("query"), 0, &lerr);
        if( lerr ) {
            QP->printf("%s", lerr.ptr());
            rc = 21;
        } else {
            sVariant * v = qengine.run(&lerr);
            if( lerr ) {
                QP->printf("%s", lerr.ptr());
                rc = 22;
            } else if( v ) {
                v->print(lerr);
                QP->printf("%s", lerr.ptr());
            }
        }
    } else if( sIs("-qcd", cmd) || sIs("-qcdExt", cmd) ) {
        const bool extend = sIs("-qcdExt", cmd);
        sDic<bool> ids;
        rc = objQuery(QP, args, ids);
        if( rc == 0 && ids.dim() ) {
            sStr paths;
            for(idx j = 0; j < ids.dim(); ++j) {
                const sHiveId * hid = (const sHiveId *)ids.id(j);
                std::unique_ptr<sUsrObj> obj(g_user->objFactory(*hid));
                if( !obj.get() ) {
                    rc = 34;
                    QP->logOut(QP->eQPLogType_Error, "object not found %s\n", hid->print());
                } else {
                    paths.cut(0);
                    obj->propGet00("_dir", &paths, 0, true);
                    for(const char * p = paths; p; p = sString::next00(p)) {
                        if( extend ) {
                            QP->printf("%s ", obj->Id().print());
                        }
                        QP->printf("%s\n", p);
                    }
                }
            }
        }
    } else if( sIs("-exportJson", cmd) ) {
        sDic<bool> ids;
        sStr qry_buf;
        rc = objQuery(QP, args, ids, &qry_buf);
        if( !ids.dim() ) {
            QP->logOut(QP->eQPLogType_Error, "no objects found - query or ID list missing or invalid\n");
            rc = 5;
        }
        if( rc == 0 ) {
            sVec<sHiveId> vids;
            vids.resize(ids.dim());
            for(idx j = 0; j < ids.dim(); ++j) {
                vids[j] = *((sHiveId *) ids.id(j));
            }

            const char * upsert_qry = 0;
            if( want_upsert == eUpsertQry ) {
                if( !qry_buf.length() ) {
                    QP->logOut(QP->eQPLogType_Error, "upsert query not provided\n");
                    rc = 49;
                }
                if( vids.dim() != 1 ) {
                    QP->logOut(QP->eQPLogType_Error, "upsert query does not return a unique result\n");
                    rc = 49;
                }
                upsert_qry = qry_buf.ptr();
            }

            if( rc == 0 ) {
                sStr buf;
                sJSONPrinter printer(&buf);
                sHiveId err1 = g_user->propExport(vids, printer, export_permissions, 0, want_flatten, want_upsert != eUpsertNone, upsert_qry, 0, prop_exclude00.length() ? prop_exclude00.ptr() : 0);
                if( err1.objId() ) {
                    QP->logOut(QP->eQPLogType_Error, "object not found %s\n", err1.print());
                } else {
                    QP->printf("%s\n", buf.ptr());
                }
            }
        }
    } else if( sIsExactly("-export", cmd) || sIsExactly("-exportUrl", cmd) ) {
        const bool asUrl = sIs("-exportUrl", cmd);
        sFilePath wDir;
        sDic<bool> ids;
        rc = objQuery(QP, args, ids);
        if( !ids.dim() ) {
            QP->logOut(QP->eQPLogType_Error, "no objects found - query or ID list missing or invalid\n");
            rc = 5;
        }
        if( rc == 0 ) {
            sVec<sHiveId> vids;
            vids.resize(ids.dim());
            for(idx j = 0; j < ids.dim(); ++j) {
                vids[j] = *((sHiveId *) ids.id(j));
            }
            sVarSet v;
            sHiveId err1 = g_user->propExport(vids, v, export_permissions);
            if( err1.objId() ) {
                rc = 41;
                if( err1.objId() ) {
                    QP->logOut(QP->eQPLogType_Error, "object not found %s\n", err1.print());
                }
            } else {
                sStr out, out2;
                sFil ofil;
                bool toFile = false;
                if( asUrl ) {
                    v.printPropUrl(out);
                    sString::searchAndReplaceStrings(&out2, out, out.length(), "&prop." __, "&prop.src-" __, 0, false);
                } else {
                    v.addRow().addCol((udx) 0).addCol("_comment").addCol((const char*) 0).printCol("%" DEC " objects exported on %s", ids.dim(), (*QP->vars["thisHostName"]));
                    v.printProp(out);
                    sString::searchAndReplaceStrings(&out2, out, out.length(), "\nprop." __, "\nprop.src-" __, 0, false);
                }
                if( toFile ) {
                    ofil.printf("%s", out2.ptr());
                } else {
                    QP->printf("%s", out2.ptr());
                }
            }
        }
    } else if( sIsExactly("-exportHivepack", cmd) ) {
        sStr hivepackName("%s.hivepack", args);
        args = sString::next00(args);
        sFilePath wDir(hivepackName, "%%dir/");
        if( !sDir::exists(wDir) && !sDir::makeDir(wDir) ) {
            QP->logOut(QP->eQPLogType_Error, "failed to create directory %s\n", wDir.ptr());
            rc = 5;
        }
        sDic<bool> ids;
        rc = objQuery(QP, args, ids);
        if( !ids.dim() ) {
            QP->logOut(QP->eQPLogType_Error, "no objects found - query or ID list missing or invalid\n");
            rc = 6;
        }
        if( rc == 0 ) {
            sVec<sHiveId> vids;
            vids.resize(ids.dim());
            for(idx j = 0; j < ids.dim(); ++j) {
                vids[j] = *((sHiveId *) ids.id(j));
            }
            sVarSet v;
            sRC RC = g_user->objHivepack(vids, hivepackName, QP->reqId ? sQPrideClient::reqProgressFSStatic : 0, (void*) QP);
            if( RC ) {
                rc = 41;
                QP->logOut(QP->eQPLogType_Error, "%s\n", RC.print());
            }
        }
    } else if( sIs("-delobj", cmd) ) {
        sDic<bool> ids;
        rc = objQuery(QP, args, ids);
        if( rc == 0 ) {
            sStr newid;
            for(idx j = 0; j < ids.dim(); ++j) {
                const sHiveId * hid = (const sHiveId *)ids.id(j);
                std::unique_ptr<sUsrObj> obj(g_user->objFactory(*hid));
                if( obj.get() && obj->purge() ) {
                    newid.cut0cut();
                    hid->print(newid);
                    QP->printf("%s\n", newid.ptr());
                }
            }
        }
    }
    return rc;
}

static void print_call_type_XXX_prop_set_cb(const sUsrObjPropsNode &node, sQPrideClient * QP, const char * typevar)
{
    if( node.value() &&  node.namecmp("created") && node.namecmp("modified") && !(sIs(node.value(), "0") && node.namecmp("field_default_value") && node.namecmp("field_order")) ) {
        sStr buf(",");
        QP->sql()->protectValue(buf, node.name());
        buf.addString(",");
        if( node.path() && *node.path() ) {
            QP->sql()->protectValue(buf, node.path());
        } else {
            buf.addString("NULL");
        }
        buf.addString(",");
        QP->sql()->protectValue(buf, node.value());
        buf.addString(",NULL,NULL)");
        sStr buf2;
        QP->sql()->protectValue(buf2, buf);
        QP->printf("CALL sp_obj_prop_set_v3(@system_group_id, @system_membership, @type_domain, %s, @system_permission, CONCAT('(', @type_domain, ',', %s, %s), NULL);\n", typevar, typevar, buf2.ptr());
    } else if( node.parentNode() && node.parentNode()->namecmp("fields") == 0 ) {
        QP->printf("\n");
    }
}

static sUsrObjPropsNode::FindStatus print_call_type_type_prop_set_cb(const sUsrObjPropsNode &node, void *param)
{
    sQPrideClient * QP = static_cast<sQPrideClient*>(param);
    print_call_type_XXX_prop_set_cb(node, QP, "@type_type_id");
    return sUsrObjPropsNode::eFindContinue;
}

static idx __on_db_init_data_type_sql(sQPrideClient * QP, const char * cmd, const char * args, const char *, sVar * pForm) {
    const sUsrType2 * utype = sUsrType2::ensure(*g_user, "type");
    sUsrObj * uobj = utype ? g_user->objFactory(utype->id()) : 0;
    const sUsrObjPropsTree * tree = uobj ? uobj->propsTree() : 0;
    if( !tree ) {
        return 1;
    }
    QP->printf(
"\n" \
"-- Generated by: qapp -user qapp -%s\n\n" \
"START TRANSACTION;\n" \
"\n" \
"source db_init_data_include.sql;\n\n", &__func__[5]);
    QP->printf("SET @type_domain = %" UDEC ";\n", utype->id().domainId());
    QP->printf("SET @type_type_id = %" UDEC ";\n", utype->id().objId());
    QP->printf("\n" \
"DELETE FROM UPPerm WHERE objID = @type_type_id AND domainID = @type_domain;\n" \
"DELETE FROM UPObjField WHERE objID = @type_type_id AND domainID = @type_domain;\n" \
"DELETE FROM UPObj WHERE objID = @type_type_id AND domainID = @type_domain;\n"\
"\n"\
"CALL sp_obj_create_v2(@system_group_id, @system_membership, @type_domain, @type_type_id, @type_domain, @type_type_id, @system_permission, @system_flags);\n\n");

    tree->find(0, print_call_type_type_prop_set_cb, QP);

    QP->printf("\n\n");
    QP->printf("CALL sp_obj_perm_set_v2(@system_group_id, @system_membership, @system_group_id, @type_domain, @type_type_id, 0, 0, @system_permission, @system_flags, TRUE);\n");
    QP->printf("CALL sp_obj_perm_set_v2(@system_group_id, @system_membership, @everyone_group_id, @type_domain, @type_type_id, 0, 0, @everyone_permission, @everyone_flags, TRUE);\n");

    QP->printf("\n\nCOMMIT;\n");
    delete uobj;
    return 0;
}

static sUsrObjPropsNode::FindStatus print_call_type_domain_prop_set_cb(const sUsrObjPropsNode &node, void *param)
{
    sQPrideClient * QP = static_cast<sQPrideClient*>(param);
    print_call_type_XXX_prop_set_cb(node, QP, "@type_domain_id");
    return sUsrObjPropsNode::eFindContinue;
}

static idx __on_db_init_data_domain_sql(sQPrideClient * QP, const char * cmd, const char * args, const char *, sVar * pForm) {
    const sUsrType2 * Type = sUsrType2::ensure(*g_user, "type");
    const sUsrType2 * utype = sUsrType2::ensure(*g_user, "domain-id-descr");
    sUsrObj * uobj = utype ? g_user->objFactory(utype->id()) : 0;
    const sUsrObjPropsTree * tree = uobj ? uobj->propsTree() : 0;
    if( !tree ) {
        return 1;
    }
    QP->printf(
"\n" \
"-- Generated by: qapp -user qapp -%s\n\n" \
"START TRANSACTION;\n" \
"\n" \
"source db_init_data_include.sql;\n\n", &__func__[5]);
    QP->printf("SET @type_domain = %" UDEC ";\n", utype->id().domainId());
    QP->printf("SET @type_type_id = %" UDEC ";\n", Type->id().objId());
    QP->printf("SET @type_domian_id = %" UDEC ";\n", utype->id().objId());
    QP->printf("\n" \
"DELETE FROM UPPerm WHERE objID = @type_domian_id AND domainID = @type_domain;\n" \
"DELETE FROM UPObjField WHERE objID = @type_domian_id AND domainID = @type_domain;\n" \
"DELETE FROM UPObj WHERE objID = @type_domian_id AND domainID = @type_domain;\n"\
"\n"\
"CALL sp_obj_create_v2(@system_group_id, @system_membership, @type_domain, @type_type_id, @type_domain, @type_domian_id, @system_permission, @system_flags);\n\n");

    tree->find(0, print_call_type_domain_prop_set_cb, QP);

    QP->printf("\n\n");
    QP->printf("CALL sp_obj_perm_set_v2(@system_group_id, @system_membership, @system_group_id, @type_domain, @type_domian_id, 0, 0, @system_permission, @system_flags, TRUE);\n");
    QP->printf("CALL sp_obj_perm_set_v2(@system_group_id, @system_membership, @everyone_group_id, @type_domain, @type_domian_id, 0, 0, @everyone_permission, @everyone_flags, TRUE);\n");

    QP->printf("\n\nCOMMIT;\n");
    delete uobj;
    return 0;
}

static idx cmp_types_by_id(void * param, void * arr, idx i1, idx i2)
{
    const sUsrType2 ** types = static_cast<const sUsrType2**>(arr);
    if (types[i1]->id() < types[i2]->id()) {
        return -1;
    } else if( types[i1]->id() > types[i2]->id()) {
        return 1;
    }
    return 0;
}

static idx cmp_typefields_by_name(void * param, void * arr, idx i1, idx i2)
{
    const sUsrTypeField ** fields = static_cast<const sUsrTypeField**>(arr);
    sStr * buf = static_cast<sStr*>(param);

    buf->cut(0);
    sString::changeCase(buf, fields[i1]->name(), 0, sString::eCaseHi);
    buf->add0();

    idx pos2 = buf->length();
    sString::changeCase(buf, fields[i2]->name(), 0, sString::eCaseHi);
    buf->add0();


    return strcmp(buf->ptr(0), buf->ptr(pos2));
}

static char * sqlProtectValueOrNull(const sUsr & user, sStr & to, const char * from, idx len = 0)
{
    idx pos = to.length();
    if( !len ) {
        len = sLen(from);
    }

    if( len ) {
        return user.dbProtectValue(to, from, len);
    }
    to.addString("NULL");
    return to.ptr(pos);
}

static idx __on_uptype_data_sql(sQPrideClient * QP, const char * cmd, const char * args, const char *, sVar * pForm) {
    QP->printf(
"\n" \
"TRUNCATE `UPType`;\n" \
"\n");

    sVec<const sUsrType2 *> types;
    sUsrType2::find(*g_user, &types, ".*");

    sSort::sortSimpleCallback(cmp_types_by_id, 0, types.dim(), types.ptr());
    sHiveId prev_id;
    sStr parents_buf, sql_buf;
    for(idx itype = 0; itype < types.dim(); itype++) {
        const sUsrType2 * utype = types[itype];
        if( utype->id().domainId() != sUsrType2::type_type_domain_id ) {
            continue;
        }
        if( prev_id ) {
            for(udx deleted_id = prev_id.objId() + 1; deleted_id < utype->id().objId(); deleted_id++) {
                QP->printf("INSERT INTO `UPType` VALUES (%" UDEC ",NULL,0,0,'--DELETED%" UDEC "','DELETED TYPE %" UDEC "');\n", deleted_id, deleted_id, deleted_id);
            }
        }
        QP->printf("INSERT INTO `UPType` VALUES (%" UDEC ",", utype->id().objId());

        parents_buf.cut0cut();
        sql_buf.cut0cut();
        for(idx ipar = 0; ipar < utype->dimParents(); ipar++) {
            if (ipar) {
                parents_buf.addString(",");
            }
            parents_buf.addString(utype->getParent(ipar)->name());
        }
        QP->printf(sqlProtectValueOrNull(*g_user, sql_buf, parents_buf));

        QP->printf(",%d,%d,", utype->isVirtual(), utype->isPrefetch());
        QP->printf("%s,", g_user->dbProtectValue(sql_buf, utype->name()));
        QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, utype->title()));
        QP->printf("%s", sqlProtectValueOrNull(*g_user, sql_buf, utype->description()));
        QP->printf(");\n");

        prev_id = utype->id();
    }

    return 0;
}

static idx __on_uptypefield_data_sql(sQPrideClient * QP, const char * cmd, const char * args, const char *, sVar * pForm) {
    QP->printf(
"\n" \
"TRUNCATE `UPTypeField`;\n" \
"\n");

    sVec<const sUsrType2 *> types;
    sVec<const sUsrTypeField *> fields;
    sUsrType2::find(*g_user, &types, ".*");

    sSort::sortSimpleCallback(cmp_types_by_id, 0, types.dim(), types.ptr());

    sStr buf, sql_buf;
    for(idx itype = 0; itype < types.dim(); itype++) {
        const sUsrType2 * utype = types[itype];
        if( utype->id().domainId() != sUsrType2::type_type_domain_id ) {
            continue;
        }

        fields.empty();
        buf.cut0cut();
        utype->getFields(*g_user, fields);
        sSort::sortSimpleCallback(cmp_typefields_by_name, &buf, fields.dim(), fields.ptr());

        for(idx ifield = 0; ifield < fields.dim(); ifield++) {
            const sUsrTypeField * field = fields[ifield];
            if( field->name()[0] == '_' || field->definerType() != utype ) {
                continue;
            }

            buf.cut0cut();
            sql_buf.cut0cut();
            QP->printf("INSERT INTO `UPTypeField` VALUES (%" UDEC ",", utype->id().objId());
            QP->printf("%s,", g_user->dbProtectValue(sql_buf, field->name()));
            QP->printf("%s,", g_user->dbProtectValue(sql_buf, field->title()));
            if( field->includedFromType() ) {
                buf.addString("type2");
                buf.addString(field->typeName());
                QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, buf.ptr()));
            } else {
                QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->typeName()));
            }
            const sUsrTypeField * non_array_row_parent = field->parent();
            if( non_array_row_parent && non_array_row_parent->isArrayRow() ) {
                non_array_row_parent = non_array_row_parent->parent();
            }
            QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, non_array_row_parent ? non_array_row_parent->name() : 0));
            switch(field->role()) {
                case sUsrTypeField::eRole_input:
                    QP->printf("in,");
                    break;
                case sUsrTypeField::eRole_output:
                    QP->printf("out,");
                    break;
                default:
                    QP->printf(",");
                    break;
            }

            QP->printf("%d,", field->isKey());
            QP->printf("%d,", field->readonly());
            QP->printf("%d,", field->isOptional());
            QP->printf("%d,", field->isMulti());
            QP->printf("%d,", field->isHidden());
            QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->brief()));
            QP->printf("%d,", field->isSummary());
            QP->printf("%d,", field->isVirtual());
            QP->printf("%d,", field->isBatch());
            QP->printf("%s,", field->orderString() && field->orderString()[0] ? field->orderString() : "NULL");
            QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->defaultValue()));
            if( field->defaultEncoding() ) {
                QP->printf("%" DEC ",", field->defaultEncoding());
            } else {
                QP->printf("NULL,");
            }
            QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->linkUrl()));
            QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->constraint()));
            if( field->includedFromType() ) {
                QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->includedFromType()->name()));
            } else {
                QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->constraintData()));
            }
            QP->printf("%s,", sqlProtectValueOrNull(*g_user, sql_buf, field->constraintDescription()));
            QP->printf("%s", sqlProtectValueOrNull(*g_user, sql_buf, field->description()));
            QP->printf(");\n");
        }
    }

    return 0;
}


static idx __on_service_helper(sQPrideClient * QP, idx stage)
{
    idx rc = 0;
    if( !g_user.get() ) {
        rc = 11;
    } else if( stage == 1 ) {
        if( !g_user->updateStart() ) {
            rc = 12;
        }
    } else if( stage == 2 ) {
        if( !g_user->updateComplete() ) {
            g_user->updateAbandon();
            rc = 13;
        }
    } else if( stage == 3 ) {
        g_user->updateAbandon();
        rc = 14;
    } else {
        rc = 15;
    }
    return rc;
}

static idx __on_service(sQPrideClient * QP, const char * cmd, const char *, const char *, sVar * pForm)
{
    idx rc = 0;
    sStr log;
    if( sIs("-serviceCreate", cmd) ) {
        const char * flnm = pForm->value("flnm");
        if( flnm && flnm[0] ) {
            sVec<sHiveId> new_ids, updated;
            if( (rc = __on_service_helper(QP, 1)) != 0 ) {
            } else if( !g_user->propSet(flnm, log, &new_ids, &updated) ) {
                QP->printf("%s", log.ptr());
                rc = 1;
            } else if( updated.dim() ) {
                QP->printf("error: %s: updates are not allowed for this command\n", cmd);
                rc = 3;
            } else if( !new_ids.dim() ) {
                QP->printf("error: %s: service was not created%s\n", cmd, log.ptr());
                rc = 2;
            } else {
                for(idx i = 0; i < new_ids.dim(); i++) {
                    sUsrObj * obj = g_user->objFactory(new_ids[i]);
                    if( !obj ) {
                        QP->printf("error: %s: new object %s access denied\n", cmd, new_ids[i].print());
                        rc = 4;
                    } else if( !obj->isTypeOf("qpsvc") ) {
                        QP->printf("error: %s: new object %s is not of service type\n", cmd, obj->Id().print());
                        rc = 5;
                    } else if(!g_user->allow4admins(obj->Id()) ) {
                        QP->printf("error: %s: cannot assign to 'admins' object %s\n", cmd, obj->Id().print());
                        rc = 6;
                    } else {
                        QP->printf("%s\n", obj->Id().print());
                    }
                    delete obj;
                }
            }
            rc = __on_service_helper(QP, rc ? 3 : 2);
        } else {
            QP->printf("error: %s: missing input file\n", cmd);
            rc = 7;
        }

    } else if( sIs("-serviceUpdate", cmd) ) {
        const char * flnm = pForm->value("flnm");
        if( flnm && flnm[0] ) {
            sVec<sHiveId> new_ids, updated;
            if( (rc = __on_service_helper(QP, 1)) != 0 ) {
            } else if( !g_user->propSet(flnm, log, &new_ids, &updated) ) {
                QP->printf("%s", log.ptr());
                rc = 1;
            } else if( new_ids.dim() ) {
                QP->printf("error: %s: object creation is not allowed for this command\n", cmd);
                rc = 3;
            } else if( !updated.dim() ) {
                QP->printf("error: %s: service was not updated%s\n", cmd, log.ptr());
                rc = 2;
            } else {
                for(idx i = 0; i < updated.dim(); i++) {
                    sUsrObj * obj = g_user->objFactory(updated[i]);
                    if( !obj ) {
                        QP->printf("error: %s: updated object %s access denied\n", cmd, updated[i].print());
                        rc = 4;
                    } else if( !obj->isTypeOf("qpsvc") ) {
                        QP->printf("error: %s: updated object %s is not of service type\n", cmd, obj->Id().print());
                        rc = 5;
                    } else {
                        QP->printf("%s\n", obj->Id().print());
                    }
                    delete obj;
                }
            }
            rc = __on_service_helper(QP, rc ? 3 : 2);
        } else {
            QP->printf("error: %s: missing service name\n", cmd);
            rc = 6;
        }

    } else if( sIs("-serviceDelete", cmd) ) {
        const char * svcName = pForm->value("name");
        if( svcName && svcName[0] ) {
            sStr nm("^%s$", svcName);
            sUsrObjRes out;
            sUsrObj * obj = 0;
            if( (rc = __on_service_helper(QP, 1)) != 0 ) {
            } else if( !g_user->objs2("qpsvc", out, 0, "name", nm) || out.dim() == 0 ) {
                QP->printf("error: %s: service not found '%s'\n", cmd, svcName);
                rc = 1;
            } else if( (obj = g_user->objFactory(*out.firstId())) == 0 ) {
                QP->printf("error: %s: service '%s' object is not accessible\n", cmd, svcName);
                rc = 2;
            } else {
                if( obj->actDelete() ) {
                    delete obj;
                } else {
                    QP->printf("error: %s: service '%s' object was not deleted\n", cmd, svcName);
                    rc = 2;
                }
            }
            rc = __on_service_helper(QP, rc ? 3 : 2);
        } else {
            QP->printf("error: %s: missing service name\n", cmd);
            rc = 3;
        }

    } else if( sIs("-up", cmd) ) {
        const char * service = pForm->value("service");
        sQPrideBase::Service sv;
        if( !QP->serviceGet(&sv, service, 0) ) {
            if( service ) {
                QP->printf("error: %s: invalid service name \"%s\"\n", cmd, service);
            } else {
                QP->printf("error: %s: missing service name\n", cmd);
            }
            rc = 4;
        } else {
            const char * up = pForm->value("isup");
            if( up && up[0] ) {
                idx isup = -1;
                sscanf(up, "%" HEX, &isup);
                if( up[0] == '+' ) {
                    isup = (sv.isUp) | (((idx) 1) << (idx) atol(up + 1));
                } else if( up[0] == '-' ) {
                    isup = (sv.isUp) & ~(((idx) 1) << (idx) atol(up + 1));
                } else {
                    isup = sAbs(isup);
                }
                QP->serviceUp(service, isup);
            } else {
                QP->printf("error: %s: missing isup value\n", cmd);
                rc = 4;
            }
        }
    } else {
        const char * service = pForm->value("service");
        idx svcCnt = 0;
        sVec<sQPrideBase::Service> svcL;
        sQPride::Service svcI, *svc = 0;

        if( sIs(cmd, "-serviceList") ) {
            QP->serviceList(0, &svcL);
            svcCnt = svcL.dim();
            svc = svcL.ptr(0);
        }
        if( sIs(cmd, "-serviceGet") ) {
            QP->serviceGet(&svcI, service, 0);
            svcCnt = 1;
            svc = &svcI;
        }

#define ifPrtD(_par) if(!strcmp(a,"all") || !strcmp(a,#_par) ){QP->printf("%s%" DEC,comma,svc->_par);comma=QP->comma;}
#define ifPrtS(_par) if(!strcmp(a,"all") || !strcmp(a,#_par) ){QP->printf("%s%s",comma,*svc->_par ? svc->_par  : "-");comma=QP->comma;}
#define ifPrtX(_par) if(!strcmp(a,"all") || !strcmp(a,#_par) ){QP->printf("%s%" HEX,comma,svc->_par);comma=QP->comma;}
#define ifPrtR(_par) if(!strcmp(a,"all") || !strcmp(a,#_par) ){QP->printf("%s%f",comma,svc->_par);comma=QP->comma;}

        for(idx i = 0; i < svcCnt; ++i) {
            sStr t;
            sString::searchAndReplaceSymbols(&t, pForm->value("variable"), 0, ",", 0, 0, true, true, true);
            for(const char * a = t.ptr(), *comma = ""; a; a = sString::next00(a)) {
                ifPrtD(svcID);
                ifPrtS(name);
                ifPrtS(title);
                ifPrtX(isUp);
                ifPrtD(svcType);
                ifPrtD(knockoutSec);
                ifPrtD(maxJobs);
                ifPrtD(nice);
                ifPrtD(sleepTime);
                ifPrtD(maxLoops);
                ifPrtD(parallelJobs);
                ifPrtD(delayLaunchSec);
                ifPrtD(politeExitTimeoutSec);
                ifPrtD(maxTrials);
                ifPrtD(restartSec);
                ifPrtD(priority);
                ifPrtD(cleanUpDays);
                ifPrtD(runInMT);
                ifPrtD(noGrabDisconnect);
                ifPrtD(noGrabExit);
                ifPrtD(lazyReportSec);
                ifPrtS(cmdLine);
                ifPrtS(hosts);
                ifPrtS(emails);
                ifPrtS(categories);
                ifPrtX(maxmemSoft);
                ifPrtX(maxmemHard);
                ifPrtD(activeJobReserve);
                ifPrtR(capacity);
                ifPrtD(cdate);
                ifPrtD(permID);
            }
            if( i < svcCnt - 1 )
                QP->printf("%s", QP->endl);
            ++svc;
        }
#undef ifPrtD
#undef ifPrtS
#undef ifPrtX
    }
    return rc;
}

static idx __on_submit(sQPrideClient * QP, const char * cmd, const char *, const char *, sVar * pForm)
{
    if( sIs(cmd, "-reqSubmit") ) {
        QP->reqId = QP->reqSubmit(pForm->value("service"), 0, 0, g_user.get() ? g_user->Id() : 0);
        if( QP->reqId ) {
            const idx grpId = pForm->ivalue("group", 0);
            if( grpId ) {
                QP->grpAssignReqID(QP->reqId, grpId);
                const idx userId = QP->reqGetUser(grpId);
                if( userId ) {
                    QP->reqSetUser(QP->reqId, userId);
                }
            }
        }

    } else if( sIs(cmd, "-grpSubmit") ) {
        QP->grpId = QP->reqId = QP->grpSubmit(pForm->value("service"), 0, 0, pForm->ivalue("count", 0), g_user.get() ? g_user->Id() : 0);
    } else if( sIs(cmd, "-reqReSubmit") ) {
        sVec<idx> reqlist;
        sString::scanRangeSet(pForm->value("req"), 0, &reqlist, 0, 0, 0);
        for(idx i = 0; i < reqlist.dim(); ++i) {
            QP->reqId = QP->reqReSubmit(reqlist[i]);
        }
    } else if( sIs(cmd, "-reqCache") ) {
        QP->reqId = QP->reqCache();
    }
    QP->printf("%" DEC "\n", QP->reqId);
    return 0;
}

static idx __on_data(sQPrideClient * QP, const char * cmd, const char * , const char * equCmd, sVar * pForm)
{
    if(sIs(cmd,"-reqSetData")){
        const char * dataName=pForm->value("dataName");
        const char * data=pForm->value("data");
        idx dsize=pForm->ivalue("dsize");
        if(!strncmp(data,"file://",7)) {
            sFil ff(data+7,sMex::fReadonly);
            if(ff.length()) {
                QP->reqSetData(QP->reqId, dataName, dsize ? dsize : ff.length(), (const void *)ff.ptr());
            }
        } else {
            QP->reqSetData(QP->reqId, dataName, dsize ? dsize : sLen(data)+1, (const void *)data );
        }
    } else if(sIs(cmd,"-reqGetData")){
        const char * dataName=pForm->value("dataName");
        const char * dstFile=pForm->value("flnm");
        idx dsize=pForm->ivalue("dsize");

        if(!dstFile || !strcmp(dstFile,"stdout")) {
            sMex data;
            QP->reqGetData(QP->reqId, dataName, &data);
            if(data.pos()) {
                fwrite(data.ptr(),dsize ? dsize : data.pos(),1,stdout);
            }
        } else {
            sFil ff(dstFile);
            QP->reqGetData(QP->reqId, dataName, ff.mex());
            if(dsize) {
                ff.cut(dsize);
            }
        }
    } else if(sIs(cmd,"-grpGetData")){
        const char * dataName=pForm->value("dataName");
        const char * dstFile=pForm->value("flnm");
        idx dsize=pForm->ivalue("dsize");
        idx cntlimit=pForm->ivalue("cntlimit"); if (!cntlimit)cntlimit=sIdxMax;
        const char * comma=pForm->value("comma","");

        sVec < idx > reqs;QP->grp2Req(QP->grpId, &reqs) ;


        FILE * fp= (!dstFile || strcmp(dstFile,"stdout")==0) ? stdout : fopen (dstFile,"w");
        sMex data;
        for( idx i=0; i<reqs.dim() && i<cntlimit; ++i){
            if(i && ! dataint)fwrite(comma,sLen(comma),1,stdout);
            QP->reqGetData(reqs[i], dataName, &data , true) ;
            if(!data.pos())continue;
            if(dataint) {
                sVec < idx > alInt;
                sString::sscanfIVec(&alInt, (const char *)data.ptr(), data.pos());
                fwrite(alInt.ptr(),alInt.dim()*sizeof(idx),1,stdout);
            }
            else
                fwrite(data.ptr(),dsize ? dsize : data.pos(),1,stdout);
            QP->logOut(QP->eQPLogType_Info, "%" DEC "/%" DEC "\r",i,reqs.dim());
        }
        if(fp!=stdout)
            fclose(stdout);
    }
    else if(sIs(cmd,"-dataGetNames")){
        sStr infos00;
        QP->dataGetAll(QP->reqId, 0, &infos00);
        for( const char * nm=infos00.ptr(); nm ; nm=sString::next00(nm)){
            QP->printf("%s\n",nm);
        }
    }
    else if(sIs(cmd,"-dataGetAll")){
        const char * ext=equCmd;
        idx doZip=pForm->ivalue("doZip",0);
        const char * dstPath=pForm->value("dstPath",0);
        const char * filter=pForm->value("filter",0);

        sStr tmpDir, path, tmp;
        if(!dstPath || strcmp(dstPath,"/tmp/")==0 ){
            QP->cfgStr(&tmpDir,pForm, "qm.tempDirectory");
            dstPath="";
        }
        char * sl=sFilePath::nextToSlash(dstPath);
        sStr dirname; dirname.printf("QPData-%" DEC,QP->reqId);
        if(sl && *(sl)==0) dstPath=path.printf("%s%s%s",tmpDir.ptr(),dstPath,dirname.ptr());

        if(doZip==2) {
            sFil test(tmp.printf("%s.tar.gz",dstPath));
            if(test.length()>0){
                QP->printf("%s exists\n",tmp.ptr());
                return 0;
            }
        }

        sl=sFilePath::nextToSlash(dstPath);
        if(sl>dstPath){
            *(sl-1)=0;
            sDir::makeDir(dstPath);
            sFile::chmod(dstPath);
            *(sl-1)='/';
        }

        if(doZip) {
            sDir::makeDir(dstPath);
            sFile::chmod(dstPath);
        }

        sStr infos00;
        QP->dataGetAll(QP->reqId, 0, &infos00);

        for( const char * nm=infos00.ptr(); nm; nm=sString::next00(nm)){


            if (filter) {
                if ( !strstr(nm,filter) ) continue;
            }

            tmp.cut(0);
            if(dstPath && *dstPath) tmp.printf("%s%s",dstPath, doZip ? "/" : "");
            tmp.printf("%s",nm);
            if(ext && *ext)tmp.printf(".%s",ext);

            if(doZip==1)
                sFile::remove(tmp.ptr());

            sFil ff(tmp.ptr());
            if(ff.length()) {
                QP->printf("%s exists\n",tmp.ptr());
                continue;
            }


            QP->reqGetData(QP->reqId, nm, ff.mex());

            QP->printf("%s\n",nm);
        }

        if(doZip ){
            sStr outLog;
            tmp.printf(0,"cd %s; tar cvf %s.tar -C %s %s; gzip %s.tar; rm -r %s ",dstPath, dstPath ,tmpDir.ptr(),dirname.ptr(), dstPath, dstPath);
            sFile::remove(path.printf(0,"%s.tar.gz", dstPath));
            sPipe::exeSys(&outLog, tmp.ptr(0));
            if(outLog.length())QP->printf("%s\n",outLog.ptr());
            if(tmp.length())QP->printf("%s\n",tmp.ptr());

        }
    }
    return 0;
}

static idx __on_resource(sQPrideClient * QP, const char * cmd, const char * , const char * , sVar * pForm)
{
    const char * service = pForm->value("service");
    if(sIs(cmd,"-resourceSet")){
        idx rsize=pForm->ivalue("rsize");
        const char * flnm=pForm->value("flnm");
        sFil ff;
        if(flnm && strncmp(flnm,"content://", 10) == 0) {
            ff.printf("%s",flnm+10);
        }else {
            ff.init( flnm,sMex::fReadonly );
        }
        return !QP->resourceSet(service,pForm->value("resourceName"), rsize ? rsize : ff.length(), (const void *)ff.ptr());

    } else if(sIs(cmd,"-resourceGet")){
        const char * dstFile=pForm->value("flnm");
        idx rsize=pForm->ivalue("rsize");

        sMex data;
        QP->resourceGet(service,pForm->value("resourceName"), &data, 0 );

        if( !dstFile || strcmp(dstFile, "stdout") == 0 ) {
            fwrite(data.ptr(), rsize ? rsize : data.pos(), 1, stdout);
        } else {
            sFil ff(dstFile);
            ff.add((const char *) data.ptr(), rsize ? rsize : data.pos());
        }
    } else if( sIs(cmd, "-resourceGetAll") ) {
        const char * dir = pForm->value("dir");
        sStr infos00;
        sVec<idx> tmStmpts;

        QP->resourceGetAll(service, &infos00, 0, &tmStmpts);
        for(const char * nm = infos00.ptr(); nm; nm = sString::next00(nm)) {
            sStr tmp;
            if( dir && *dir ) {
                tmp.printf("%s/", dir);
            }
            tmp.printf("%s", nm);
            sFile::remove(tmp);
            sFil ff(tmp.ptr());
            if( ff.ok() ) {
                QP->reqGetData(QP->reqId, nm, ff.mex());
                QP->resourceGet(service, nm, ff.mex(), 0);
                QP->printf("%s\n", nm);
            }
        }
    } else if(sIs(cmd,"-resourceDel")){
        QP->resourceDel(service, pForm->value("resourceName"));
    } else if(sIs(cmd,"-resourceSync")){
        QP->resourceSync(pForm->value("dir"), service, QP->vars.value("os"));
    } else if(sIs(cmd,"-resourceWipe")){

    }else if(sIs(cmd,"-resourceDiff")){

    }else if(sIs(cmd,"-resourceList")){

    }

    return 0;
}





static const char * _QPReqActionList="unknown" _ "none" _ "run" _ "kill" _ "suspend" _ "resume" __;
static const char * _QPReqStatusList="unknown" _ "waiting" _ "processing" _ "running" _ "suspended" _ "done" _ "killed" _ "progerror" _ "syserror" __;
static const char * _QPInfoLevelList="trace" _ "debug" _ "info" _ "warning" _ "error" _ "fatal" __;

static idx __on_request(sQPrideClient * QP, const char * cmd, const char * arg, const char * ,sVar * pForm)
{
    idx num,pos;

    if(sIs(cmd,"-requestGet")){
        sQPride::Request Req;
        QP->requestGet(QP->reqId, &Req) ;

        #define ifArtD(_par) if(!strcmp(arg,"all") || !strcmp(arg,#_par) )QP->printf("%" DEC "\n",Req._par)
        #define ifArtT(_par) if(!strcmp(arg,"all") || !strcmp(arg,#_par) )QP->printf("%" DEC "\n",Req._par)
        #define ifArtX(_par) if(!strcmp(arg,"all") || !strcmp(arg,#_par) )QP->printf("%" HEX"\n",Req._par)

        sStr t;sString::searchAndReplaceSymbols(&t,pForm->value("variable"),0,",",0,0,true,true,true);
        for ( const char * a=t.ptr(); a ; a=sString::next00(a)) {


            ifArtX(reqID);
            ifArtX(svcID);
            ifArtX(jobID);
            ifArtX(subIp);
            ifArtX(cgiIp);
            ifArtD(svcID);
            ifArtD(stat);
            ifArtD(act);
            ifArtD(takenCnt);
            ifArtD(priority);
            ifArtD(inParallel);
            ifArtD(progress);
            ifArtD(progress100);
            ifArtT(takenTm);
            ifArtT(cdate);
            ifArtT(actTm);
            ifArtT(doneTm);
            ifArtT(purgeTm);
        }
        #undef ifArtD
        #undef ifArtT
        #undef ifArtX

    } else if( sIs(cmd, "-reqGetInfo") ) {
        sVec<sQPrideBase::QPLogMessage> infos;
        sStr buf;
        if( QP->grpId ) {
            QP->grpGetInfo(QP->grpId, QP->eQPInfoLevel_Min, infos);
        } else {
            QP->reqGetInfo(QP->reqId, QP->eQPInfoLevel_Min, infos);
        }
        for(idx i = 0; i < infos.dim(); ++i) {
            buf.cut0cut();
            QP->printf("%s\t%s\t%s\n", QP->getLevelName(infos[i].level), sString::printDateTime(buf, infos[i].cdate, sString::fISO8601), infos[i].message());
        }
    } else if( sIsExactly(cmd, "-reqGetLog") ) {
        sVec<sQPrideBase::QPLogMessage> logs;
        sStr buf;
        QP->getLog(QP->grpId ? QP->grpId : QP->reqId, QP->grpId, QP->jobId, 0, logs);
        for(idx i = 0; i < logs.dim(); i++) {
            buf.cut0cut();
            QP->printf("req %" DEC "\tjob %" DEC "\t%s.%s\t%s\t%s\n", logs[i].req, logs[i].job, logs[i].level >= sQPrideBase::eQPInfoLevel_Min ? "info" : "log", QP->getLevelName(logs[i].level), sString::printDateTime(buf, logs[i].cdate, sString::fISO8601), logs[i].message());
        }
    } else if(sIs(cmd,"-reqSetInfo")){
        num = sNotIdx;
        if ( pForm->value("severity") ) {
            pos = sString::compareChoice(pForm->value("severity"), _QPInfoLevelList,&num,true, 0, true);
            if ( pos != sNotIdx ) {
                num = (num + 1) * 100;
            }
        }
        if ( num == sNotIdx ) {
            num = QP->eQPInfoLevel_Info;
        }
        QP->reqSetInfo(QP->reqId, num, pForm->value("text") );
    } else if(sIs(cmd,"-reqSetAction")){
        pos=sString::compareChoice(arg, _QPReqActionList,&num,true, 0, true);
        if(pos==sNotIdx)sscanf(pForm->value("action"),"%" DEC,&num);
           QP->reqSetAction(QP->reqId, num);
    }else if(sIs(cmd,"-grpSetAction")){
        pos=sString::compareChoice(pForm->value("action"), _QPReqActionList,&num,true, 0, true);
        if(pos==sNotIdx)sscanf(arg,"%" DEC,&num);
        QP->reqSetAction(-QP->grpId, num);
    } else if(sIs(cmd,"-reqGetAction")){
        num=QP->reqGetAction(QP->reqId);
        QP->printf("%" DEC " %s\n",num,sString::next00(_QPReqActionList,num));
    } else if(sIs(cmd,"-reqSetStatus")){
        pos=sString::compareChoice(pForm->value("status"), _QPReqStatusList,&num,true, 0, true);
        if(pos==sNotIdx)sscanf(arg,"%" DEC,&num);
        QP->reqSetStatus(QP->reqId, num);
    } else if(sIs(cmd,"-grpSetStatus")){
        pos=sString::compareChoice(pForm->value("status"), _QPReqStatusList,&num,true, 0, true);
        if(pos==sNotIdx)sscanf(arg,"%" DEC,&num);
        QP->reqSetStatus( - QP->grpId, num);
    } else if(sIs(cmd,"-reqGetStatus")){
        num=QP->reqGetStatus(QP->reqId);
        QP->printf("%" DEC " %s\n",num,sString::next00(_QPReqStatusList,num));
    } else if( sIs(cmd, "-reqSetProgress") ) {
        idx progress = 0, progress100 = 0;
        progress = pForm->ivalue("progress");
        progress100 = pForm->ivalue("progress100");
        QP->reqProgress(QP->reqId, 0, progress, progress100,100);
    }
    return 0;
}


static idx __on_grp(sQPrideClient * QP, const char * cmd, const char * arg, const char * ,sVar * pForm)
{
    if(sIs(cmd,"-grpAssignReqID")){
        if( !QP->grpId ) {
            QP->grpId = pForm->ivalue("grp");
        }
        QP->grpAssignReqID(QP->reqId, QP->grpId) ;
    } else if(sIs(cmd,"-getReq2Grp")) {
        sVec<idx> grpIds;
        QP->req2Grp(QP->reqId, &grpIds);
        for( idx i=0; i<grpIds.dim(); ++i) {
            QP->printf("%" DEC "\n",grpIds[i]);
        }
    } else if(sIs(cmd,"-getGrp2Req")) {
        sVec<idx> reqIds;
        QP->grp2Req(QP->grpId, &reqIds);
        for( idx i=0; i<reqIds.dim(); ++i) {
            QP->printf("%" DEC "\n",reqIds[i]);
        }
    }
    return 0;
}


static idx __on_acct(sQPrideClient * QP, const char * cmd, const char * arg, const char * , sVar * pForm)
{
    sStr passbuf, outbuf;

    if( sIs(cmd, "-pswdHash") ) {
        const char * pass = pForm->value("password");

        if( pass ) {
            QP->printf("%s\n", sPassword::encodePassword(outbuf, pass));
        } else {
            fprintf(stderr, "pass argument required\n");
            return 1;
        }
    } else if( sIs(cmd, "-pswdCheck") ) {
        const char * hash = pForm->value("hash");
        const char * pass = pForm->value("password");

        if( pass && hash ) {
            if( sPassword::checkPassword(hash, 0, pass) ) {
                return 0;
            } else {
                fprintf(stderr, "password and hash do not match\n");
                return 1;
            }
        } else {
            fprintf(stderr, "hash and password arguments required\n");
            return 1;
        }
    } else if( sIs(cmd, "-exportUsrGrp4Ion") ) {
        if( g_user.get() ) {
            sStr buf;
            sJSONPrinter json(QP->outP ? QP->outP : &buf);
            g_user.get()->exportUsrGrp4Ion(json);
            json.finish();
            if( !QP->outP ) {
                QP->printf("%s", buf.ptr());
            }
        } else {
            fprintf(stderr, "You need to run -user qpride -exportUsrGrp4Ion\n");
        }
    }

    return 0;
}



void sQPrideClient::printf(const char * formatDescription , ...)
{
    if(outP){sCallVarg(outP->vprintf,formatDescription);}
    else {sCallVargPara(vfprintf,stdout,formatDescription);}
}

idx sQPrideClient::CmdForm(const char * cmd , sVar * pForm)
{
    if( !cmd ) {
        return 0;
    }
    if( !reqId ) {
        reqId = pForm->ivalue("req");
    }
    if( reqId < 0 ) {
        reqId = getReqByUserKey(reqId);
    }
    grpId = reqId;
    if( pForm->is("grp") ) {
        grpId = pForm->ivalue("grp");
    }
    for(idx i = 0; cmdExes[i].param != sNotPtr; ++i) {
        if( cmdExes[i].cmd == 0 || cmdExes[i].cmdFun == 0 )
            continue;
        if( strcmp(cmdExes[i].cmd, cmd) == 0 ) {
            sCmdLine cmdline;
            char equCmd[128];
            equCmd[0] = 0;
            cmdline.exeFunCaller(&cmdExes[i], cmd, 0, equCmd, pForm);
            return 1;
        }
    }
    return 0;
}


sCmdLine::exeCommand sQPrideClient::cmdExes[]={
    {0,0,0,"","List of available commands"},

    {0,0,0,      "\t","\n\nSettings variables\n"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-req"," req // set request id to work with "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-grp"," grp // set group id to work with "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-job"," job // set job id to work with "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argAllSpacedList,     "-user"," login password // set user to work with "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-os"," platform // set the platform "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-host"," // get the host name and platform  "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-logLevel"," level // verbose output level"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-version"," // show the version of the program "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-dataint"," // if set all data is translated to ints "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-respond"," command // define the password for shall command "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-upsert"," upsert_how // for -exportJson command, set _id to $upsert() (if upsert_how == 'auto'),\n\t\t $upsert_qry() (if upsert_how == 'qry'), or leave as is (if upsert_how == '0' or 'false') "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-flatten"," // flatten decorative nodes in -exportJson command "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,              "-permUser"," // print user permissions in -exportJson / -export command "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-updateList"," hiveids // for -propJson command, from specified ids (or query:// results),\n\t\t delete those which were not updated by json "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,          "-excludeProp"," prop_excl // for -exportJson command, exclude specified property "},


    {0,0,0,      "\t","\n\nGeneral commands\n"},
    {0,(sCmdLine::exeFunType)&__on_general,sCmdLine::argAllSpacedList,  "-shell"," commandline // run arbitrary command "},
    {0,(sCmdLine::exeFunType)&__on_general,sCmdLine::argAllSpacedList,  "-shall"," commandline // run arbitrary command on a list of hosts/time range in hours , e.g. shall=bee,fee:24 or shall=bee or -shall=:7 (default is 1 hour)"},
    {0,(sCmdLine::exeFunType)&__on_general,sCmdLine::argAllSpacedList,  "-shellW"," exec // run arbitrary command "},
    {0,(sCmdLine::exeFunType)&__on_general,sCmdLine::argAllZeroList,    "-file"," flnm // show file content "},
    {0,(sCmdLine::exeFunType)&__on_general,sCmdLine::argAllZeroList,    "-init"," dir          // initialize the machine "},
    {0,(sCmdLine::exeFunType)&__on_general,sCmdLine::argAllZeroList,    "-init2"," dir          // initialize the machine (objectified)"},
    {0,(sCmdLine::exeFunType)&__on_general,sCmdLine::argAllZeroList,    "-endian", " file          // initialize the machine "},

    {0,0,0,      "\t","\n\nHTML commands\n"},
    {0,(sCmdLine::exeFunType)&__on_html,sCmdLine::argAllZeroList,  "-form"," variable // show submission form content "},
    {0,(sCmdLine::exeFunType)&__on_html,sCmdLine::argAllZeroList,  "-setForm"," url_query_string // add values from URL query string to form "},
    {0,(sCmdLine::exeFunType)&__on_html,sCmdLine::argAllZeroList,  "-resetForm"," url_query_string // delete form, replace with values from URL query string "},


    {0,0,0,      "\t","\n\nMessaging commands\n"},
    {0,(sCmdLine::exeFunType)&__on_message,sCmdLine::argAllZeroList,    "-messageSubmit"," server service message // submit a message to a particular service "},
    {0,(sCmdLine::exeFunType)&__on_message,sCmdLine::argAllZeroList,    "-messageSubmitToDomainHeader"," message // submit a message to a domain headers "},
    {0,(sCmdLine::exeFunType)&__on_message,sCmdLine::argAllZeroList,    "-messageSubmitToDomain"," service message // submit a message to a service within domain "},
    {0,(sCmdLine::exeFunType)&__on_message,sCmdLine::argAllZeroList,    "-messageWakePulljob"," service message // submit a wakeup message to a service "},


    {0,0,0,      "\t","\n\nConfig commands\n"},
    {0,(sCmdLine::exeFunType)&__on_config,sCmdLine::argAllZeroList,        "-configGet"," parameter // retrieve the value of the config parameter "},
    {0,(sCmdLine::exeFunType)&__on_config,sCmdLine::argAllZeroList,        "-configGetAll"," parameter // retrieve the list of the config parameters "},
    {0,(sCmdLine::exeFunType)&__on_config,sCmdLine::argAllZeroList,        "-configSet"," parameter value // set the value of the parameter "},

    {0,0,0,      "\t","\n\nService commands\n"},
    {0,(sCmdLine::exeFunType)&__on_service,sCmdLine::argAllSpacedList,  "-up"," service isup // start/stop the service by name "},
    {0,(sCmdLine::exeFunType)&__on_service,sCmdLine::argAllSpacedList,  "-serviceList"," variable // retrieve the value of the service parameter "},
    {0,(sCmdLine::exeFunType)&__on_service,sCmdLine::argAllSpacedList,  "-serviceGet"," service variable // retrieve the value of the service parameter "},
    {0,(sCmdLine::exeFunType)&__on_service,sCmdLine::argAllZeroList,    "-serviceCreate"," flnm // Create service from prop file "},
    {0,(sCmdLine::exeFunType)&__on_service,sCmdLine::argAllZeroList,    "-serviceUpdate"," flnm // Update service from prop files "},
    {0,(sCmdLine::exeFunType)&__on_service,sCmdLine::argAllZeroList,    "-serviceDelete"," name // Delete service "},

    {0,0,0,      "\t","\n\nSubmission commands\n"},
    {0,(sCmdLine::exeFunType)&__on_submit,sCmdLine::argAllZeroList,        "-reqSubmit"," service group // submit a request of particular service (to group)"},
    {0,(sCmdLine::exeFunType)&__on_submit,sCmdLine::argAllZeroList,        "-grpSubmit"," service count // submit a group request of particular service "},
    {0,(sCmdLine::exeFunType)&__on_submit,sCmdLine::argAllZeroList,        "-reqReSubmit"," req // resubmit existing request "},
    {0,(sCmdLine::exeFunType)&__on_submit,sCmdLine::argAllZeroList,        "-reqCache"," // create a cache request "},

    {0,0,0,      "\t","\n\nData commands\n"},
    {0,(sCmdLine::exeFunType)&__on_data,sCmdLine::argAllZeroList,        "-reqSetData","    dataName data dsize // set the data blob, can be file:datafilename "},
    {0,(sCmdLine::exeFunType)&__on_data,sCmdLine::argAllZeroList,        "-reqGetData","    dataName flnm dsize // retrieve data blob, can use a filename "},
    {0,(sCmdLine::exeFunType)&__on_data,sCmdLine::argAllZeroList,        "-grpGetData","    dataName flnm cntlimit comma dsize // retrieve data blob, can use a filename "},
    {0,(sCmdLine::exeFunType)&__on_data,sCmdLine::argAllZeroList,        "-dataGetAll","=extension dstPath doZip filter // get all data blobs into appropriately named files "},
    {0,(sCmdLine::exeFunType)&__on_data,sCmdLine::argNone,               "-dataGetNames"," // get the list of datanames "},

    {0,0,0,      "\t","\n\nResource commands\n"},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceSet"," service resourceName flnm rsize // set the resource blob, can be file:datafilename "},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceGet"," service resourceName flnm rsize // retrieve resource blob, can use a filename "},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceGetAll"," service dir // get all resource blobs into appropriately named files "},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceDel"," service resourceName // remove resource blob(s) by service name and optional resourceName"},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceSync"," service dir // syncronize all resource blobs"},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceWipe"," dir // wipe all unneeded resources "},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceDiff"," dir // list all diffed resources "},
    {0,(sCmdLine::exeFunType)&__on_resource,sCmdLine::argAllZeroList,   "-resourceList"," // list the resource names "},


    {0,0,0,      "\t","\n\nUser DB commands\n"},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,     "-prop"," propfile // load prop file (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,     "-propJson"," propjson // load JSON prop file (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,     "-propJson2Csv"," propjson // convert JSON prop file to CSV (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,     "-propJson2Prop"," propjson // convert JSON prop file to old prop format (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllSpacedList,   "-query"," query // execute object query (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,   "-qcd"," hiveids // print object(s) or query:// directories, one per line "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,   "-qcdExt"," hiveids // print object(s) or query:// directories, one per line: id path"},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllSpacedList,   "-export"," hiveids // prop output object(s) or query:// (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllSpacedList,   "-exportJson"," hiveids // json output object(s) or query:// (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,   "-exportUrl"," hiveids // prop output object(s) or query:// (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllZeroList,   "-exportHivepack"," directory hiveids // prop output object(s) or query:// (requires user) -e.g. 'query://((obj)<id>).allusedby({recurse:true,with_topic:true})' "},
    {0,(sCmdLine::exeFunType)&__on_udb,sCmdLine::argAllSpacedList,   "-delobj"," hiveids // delete object(s) or query:// (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_db_init_data_type_sql,sCmdLine::argNone, "-db_init_data_type_sql"," // generate db_init_data_type.sql (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_db_init_data_domain_sql,sCmdLine::argNone, "-db_init_data_domain_sql"," // generate db_init_data_domain.sql (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_uptype_data_sql,sCmdLine::argNone, "-UPType_data.sql"," // generate old-style UPType_data.sql (requires user) "},
    {0,(sCmdLine::exeFunType)&__on_uptypefield_data_sql,sCmdLine::argNone, "-UPTypeField_data.sql"," // generate old-style UPTypeField_data.sql (requires user) "},


    {0,0,0,      "\t","\n\nRequest commands\n"},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argAllZeroList,   "-requestGet"," variable // retrieve the value of the request parameter, can be variable=all "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argNone,          "-reqGetInfo"," // retrieve the list of messages associated with a request or group (specified by -req or -grp) "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argNone,          "-reqGetLog"," // retrieve list of both info messages and debugging log (for request/group/job specified by -req or -grp or -job) "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argAllSpacedList, "-reqSetInfo"," text severity // add a new message to the request log "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argAllZeroList,   "-reqSetAction"," action // set action for this request "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argAllZeroList,   "-grpSetAction"," action // set action for this request "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argNone,          "-reqGetAction"," // get action of the request "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argAllZeroList,   "-reqSetStatus"," status // set status for this request "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argAllZeroList,   "-grpSetStatus"," status // set status for this group "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argNone,          "-reqGetStatus"," // get status of the request "},
    {0,(sCmdLine::exeFunType)&__on_request,sCmdLine::argAllZeroList,   "-reqSetProgress"," progress progress100 // set progress of the request "},


    {0,0,0,      "\t","\n\nGroup commands\n"},
    {0,(sCmdLine::exeFunType)&__on_grp,sCmdLine::argAllZeroList,     "-grpAssignReqID"," grp // assign to a group ID"},
    {0,(sCmdLine::exeFunType)&__on_grp,sCmdLine::argAllZeroList,     "-getReq2Grp"," // get the lit of groups for this request"},
    {0,(sCmdLine::exeFunType)&__on_grp,sCmdLine::argAllZeroList,     "-getGrp2Req"," // get the list of requests for this group"},

    {0,0,0,      "\t","\n\nUser account and user group commands\n"},
    {0,(sCmdLine::exeFunType)&__on_acct,sCmdLine::argAllZeroList,     "-pswdHash"," password // generate HIVE format password hash from a raw password"},
    {0,(sCmdLine::exeFunType)&__on_acct,sCmdLine::argAllZeroList,     "-pswdCheck"," hash password // check if password matches a hash"},
    {0,(sCmdLine::exeFunType)&__on_acct,sCmdLine::argNone,            "-exportUsrGrp4Ion"," // export user and group info for ION in JSON format"},

    {0,0,0,      "\t","\n\nHelp commands\n"},
    {0,(sCmdLine::exeFunType)&__on_help,sCmdLine::argAllZeroList,     "-help"," command // help"},

    {sNotPtr,0}
};


static idx __on_help(sQPrideClient * QP, const char * cmd, const char * , const char * ,sVar * pForm)
{
    if(sIs(cmd,"-help")){
        const char * which=pForm->value("command");
        for ( idx i=0; sQPrideClient::cmdExes[i].param!=sNotPtr ; ++i ) {
            if( which && strcmp(which,sQPrideClient::cmdExes[i].cmd+1)!=0)continue;
            QP->printf("\t%s%s\n", sQPrideClient::cmdExes[i].cmd,sQPrideClient::cmdExes[i].descr);
        }
    }
    return 0;
}
