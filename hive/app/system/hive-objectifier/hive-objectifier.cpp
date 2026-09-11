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

#include <slib/std/app.hpp>
#include <qlib/QPrideProc.hpp>

using namespace slib;

class HiveObjectifier: public sQPrideProc
{
    private:
        void propSetHiveServiceObject(sUsrObj * obj, const sQPrideBase::Service & svc);
        bool getExistingObjectTypeByObjectName(sUsrObjRes & obj_res, sHiveId & existingId, const char * propField00, const char * propVal00);
        void propSetHiveHostObject(sUsrObj * obj, const sQPrideBase::Host & host);
        void propSetHiveCfgObject(sUsrObj * obj);
        
        sUsrObj * getMyObject(const char * objecType, bool isFound, sHiveId & existingId);

        bool applyHostFieldFromForm(sQPrideBase::Host & host, const char * col, const char * val);
        bool applySvcFieldFromForm(sQPrideBase::Service & svc, const char * col, const char * val);
    public:
        HiveObjectifier(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
            
        }

        virtual idx OnExecute(idx req);
};

bool HiveObjectifier::applyHostFieldFromForm(sQPrideBase::Host & host, const char * col, const char * val)
{
    if (!col || !val) return false;

    if (strcmp(col, "ip4") == 0) {
        strncpy(host.ip4, val, sizeof(host.ip4) - 1);
        host.updateMask |= sQPrideBase::HOST_UPDATE_IP4;
    } else if (strcmp(col, "htype") == 0) {
        strncpy(host.htype, val, sizeof(host.htype) - 1);
        host.updateMask |= sQPrideBase::HOST_UPDATE_HTYPE;
    } else if (strcmp(col, "category") == 0) {
        strncpy(host.category, val, sizeof(host.category) - 1);
        host.updateMask |= sQPrideBase::HOST_UPDATE_CATEGORY;
    } else if (strcmp(col, "enabled") == 0) {
        host.enabled = atoidx(val);
        host.updateMask |= sQPrideBase::HOST_UPDATE_ENABLED;
    } else if (strcmp(col, "capacity") == 0) {
        sscanf(val, "%lf", &host.capacity);
        host.updateMask |= sQPrideBase::HOST_UPDATE_CAPACITY;
    } else if (strcmp(col, "cores") == 0) {
        host.cores = atoidx(val);
        host.updateMask |= sQPrideBase::HOST_UPDATE_CORES;
    } else if (strcmp(col, "memory") == 0) {
        host.memory = atoidx(val);
        host.updateMask |= sQPrideBase::HOST_UPDATE_MEMORY;
    } else {
        return false;
    }
    return true;
}

bool HiveObjectifier::applySvcFieldFromForm(sQPrideBase::Service & svc, const char * col, const char * val)
{
    if (!col || !val) return false;

    if (strcmp(col, "svcType") == 0) {
        svc.svcType = atoidx(val);
    } else if (strcmp(col, "knockoutSec") == 0) {
        svc.knockoutSec = atoidx(val);
    } else if (strcmp(col, "maxJobs") == 0) {
        svc.maxJobs = atoidx(val);
    } else if (strcmp(col, "nice") == 0) {
        svc.nice = atoidx(val);
    } else if (strcmp(col, "sleepTime") == 0) {
        svc.sleepTime = atoidx(val);
    } else if (strcmp(col, "maxLoops") == 0) {
        svc.maxLoops = atoidx(val);
    } else if (strcmp(col, "parallelJobs") == 0) {
        svc.parallelJobs = atoidx(val);
    } else if (strcmp(col, "delayLaunchSec") == 0) {
        svc.delayLaunchSec = atoidx(val);
    } else if (strcmp(col, "politeExitTimeoutSec") == 0) {
        svc.politeExitTimeoutSec = atoidx(val);
    } else if (strcmp(col, "maxTrials") == 0) {
        svc.maxTrials = atoidx(val);
    } else if (strcmp(col, "restartSec") == 0) {
        svc.restartSec = atoidx(val);
    } else if (strcmp(col, "priority") == 0) {
        svc.priority = atoidx(val);
    } else if (strcmp(col, "cleanUpDays") == 0) {
        svc.cleanUpDays = atoidx(val);
    } else if (strcmp(col, "runInMT") == 0) {
        svc.runInMT = atoidx(val);
    } else if (strcmp(col, "noGrabDisconnect") == 0) {
        svc.noGrabDisconnect = atoidx(val);
    } else if (strcmp(col, "noGrabExit") == 0) {
        svc.noGrabExit = atoidx(val);
    } else if (strcmp(col, "lazyReportSec") == 0) {
        svc.lazyReportSec = atoidx(val);
    } else if (strcmp(col, "isUp") == 0) {
        svc.isUp = atoidx(val);
    } else if (strcmp(col, "maxmemSoft") == 0) {
        svc.maxmemSoft = atoidx(val);
    } else if (strcmp(col, "maxmemHard") == 0) {
        svc.maxmemHard = atoidx(val);
    } else if (strcmp(col, "title") == 0) {
        strncpy(svc.title, val, sizeof(svc.title) - 1);
    } else if (strcmp(col, "cmdLine") == 0) {
        strncpy(svc.cmdLine, val, sizeof(svc.cmdLine) - 1);
    } else if (strcmp(col, "hosts") == 0) {
        strncpy(svc.hosts, val, sizeof(svc.hosts) - 1);
    } else if (strcmp(col, "emails") == 0) {
        strncpy(svc.emails, val, sizeof(svc.emails) - 1);
    } else if (strcmp(col, "categories") == 0) {
        strncpy(svc.categories, val, sizeof(svc.categories) - 1);
    } else if (strcmp(col, "capacity") == 0) {
        sscanf(val, "%lf", &svc.capacity);
    } else {
        return false;
    }
    return true;
}


void HiveObjectifier::propSetHiveServiceObject(sUsrObj * obj, const sQPrideBase::Service & svc){
    obj->propSetI("svcID", svc.svcID);
    obj->propSetI("permID", svc.permID);
    obj->propSetI("svcType", svc.svcType);
    obj->propSetI("knockoutSec", svc.knockoutSec);
    obj->propSetI("maxJobs", svc.maxJobs);
    obj->propSetI("nice", svc.nice);            
    obj->propSetI("maxLoops", svc.maxLoops);
    obj->propSetI("sleepTime", svc.sleepTime);
    obj->propSetI("parallelJobs", svc.parallelJobs);
    obj->propSetI("delayLaunchSec", svc.delayLaunchSec);
    obj->propSetI("politeExitTimeoutSec", svc.politeExitTimeoutSec);
    obj->propSetI("maxTrials", svc.maxTrials);
    obj->propSetI("restartSec", svc.restartSec);
    obj->propSetI("priority", svc.priority);
    obj->propSetI("cleanUpDays", svc.cleanUpDays);
    obj->propSetI("runInMT", svc.runInMT);
    obj->propSetI("noGrabDisconnect", svc.noGrabDisconnect);
    obj->propSetI("noGrabExit", svc.noGrabExit);
    obj->propSetI("lazyReportSec", svc.lazyReportSec);
    obj->propSetI("isUp", svc.isUp);            
    obj->propSetI("maxmemSoft", svc.maxmemSoft);
    obj->propSetI("maxmemHard", svc.maxmemHard);
    obj->propSetI("cdate", svc.cdate);
    obj->propSet("name", svc.name);
    obj->propSet("title", svc.title);
    obj->propSet("cmdLine", svc.cmdLine);
    obj->propSet("hosts", svc.hosts);
    obj->propSet("emails", svc.emails);
    obj->propSet("categories", svc.categories);

    
}
void HiveObjectifier::propSetHiveHostObject(sUsrObj * obj, const sQPrideBase::Host & host){
    obj->propSet("name", host.name);
    obj->propSet("ip4", host.ip4);
    obj->propSet("htype", host.htype);
    obj->propSet("category", host.category);
    obj->propSetI("enabled", host.enabled);
    obj->propSetR("capacity", host.capacity);
    obj->propSetI("cores", host.cores);
    obj->propSetI("memory", host.memory);
    obj->propSetI("mdate", host.mdate);
}

void HiveObjectifier::propSetHiveCfgObject(sUsrObj * obj){

    obj->propSet("name", "qpcfg");

    sStr cfgBuf;
    configGetAllClean(&cfgBuf, 0);

    sStr groupBuf;
    sVec<const char *> groupList;
    sVec<const char *> parList;
    sVec<const char *> valList;
    idx rowCnt = 0;

    const char * par = 0;
    for (const char * val = cfgBuf.ptr(0); val; val = sString::next00(par)) {
        par = sString::next00(val);
        if (!par) {
            break;
        }

        idx pos = groupBuf.length();
        groupBuf.printf("%" DEC ".0", rowCnt++);
        groupBuf.add0();

        groupList.vadd(1, groupBuf.ptr(pos));
        parList.vadd(1, par);
        valList.vadd(1, val);
    }

    if (parList.dim()) {
        obj->propSet("par", groupList.ptr(), parList.ptr(), parList.dim());
        obj->propSet("val", groupList.ptr(), valList.ptr(), valList.dim());
    }
}

bool HiveObjectifier::getExistingObjectTypeByObjectName(sUsrObjRes & obj_res, sHiveId & existingId, const char * propField00, const char * propVal00) {
    for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
        sUsrObj * obj = user->objFactory(*obj_res.id(it));
        if (obj) {
            bool matched = true;
            const char * field = propField00;
            const char * expectedVal = propVal00;
            for (; field && expectedVal; field = sString::next00(field), expectedVal = sString::next00(expectedVal)) {
                const char * actualVal = obj->propGet(field);
                if (!actualVal || strcmp(actualVal, expectedVal) != 0) {
                    matched = false;
                    break;
                }
            }
            if (matched) {
                existingId = obj->Id();
                ::printf(" - Found existing object with ID %s\n", existingId.print());
            }
            delete obj;
            if (matched) return true;
        }
    }
    return false;
}

sUsrObj * HiveObjectifier::getMyObject(const char * objecType, bool isFound, sHiveId & existingId) {
    std::auto_ptr<sUsrObj>  newObj;
    sUsrObj * obj = 0;
    if (!isFound) {
            sHiveId nid; sRC rc;
            rc = user->objCreate(nid, objecType);
            if( !rc.isSet() ) {
                newObj.reset(user->objFactory(nid));
                obj = newObj.get();
                if( obj ) {
                    reqSetInfo(reqId, eQPInfoLevel_Info, "created object %s", obj->Id().print());
                }
                return newObj.release();
            }
            else {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                logOut(eQPLogType_Error, "Cannot find/create object : %s", rc.print());
                return 0;
            }
        } 
        else {
            obj = user->objFactory(existingId);
            if (!obj) {
                reqSetInfo(reqId, eQPInfoLevel_Error, "Internal error (%d)", __LINE__);
                logOut(eQPLogType_Error, "Cannot access existing object %s", existingId.print());
                return 0;
            }
            return obj;
        }
    return 0;
}

idx HiveObjectifier::OnExecute(idx req)
{
#ifdef _DEBUG
    fprintf(stderr, "qpride form for req %" DEC ":\n", req);
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
    fprintf(stderr, "vars for req %" DEC ":\n", req);
    for (idx i=0; i<vars.dim(); i++) {
        const char * key = static_cast<const char*>(vars.id(i));
        const char * value = vars.value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
#endif

    if( !user || !user->isAdmin() ) {
        reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Unauthorized or invalid user");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sUsr superuser("queen", true);
    if( !superuser.Id() ) {
        reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Failed to switch to superuser mode");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sUsr * cur_user = user;
    user = &superuser;

    const sUsrObjPropsTree* objFormTree = objs[0].propsTree();
    const sUsrObjPropsNode* colList = objFormTree ? objFormTree->find("col_list_list") : 0;

    if (colList) {
        sDic<sQPrideBase::Host> hostUpdates;
        sDic<sQPrideBase::Service> svcUpdates;
        sDic<sStr> cfgUpdates;

        logOut(eQPLogType_Info, "col_list has %" DEC " children", colList->dim());

        for (const sUsrObjPropsNode* colRow = colList->firstChild(); colRow; colRow = colRow->nextSibling()) {
            const sUsrObjPropsNode * tableNode = colRow->find("table");
            const sUsrObjPropsNode * colNode = colRow->find("col");
            const sUsrObjPropsNode * valNode = colRow->find("val");
            const sUsrObjPropsNode * keyFieldNode = colRow->find("key_field_value");

            const char * tbl = tableNode ? tableNode->value() : 0;
            const char * col = colNode ? colNode->value() : 0;
            const char * val = valNode ? valNode->value() : 0;

            logOut(eQPLogType_Info, "col_list row: table=%s col=%s val=%s",
                tbl ? tbl : "(null)", col ? col : "(null)", val ? val : "(null)");

            if (tableNode) objs[0].propDel(tableNode->name(), tableNode->path(), tableNode->value());
            if (colNode) objs[0].propDel(colNode->name(), colNode->path(), colNode->value());
            if (valNode) objs[0].propDel(valNode->name(), valNode->path(), valNode->value());
            if (keyFieldNode) objs[0].propDel(keyFieldNode->name(), keyFieldNode->path(), keyFieldNode->value());

            if (!tbl || !col || !val) continue;

            if (strcmp(tbl, "QPHosts") == 0) {
                const char * hostName = keyFieldNode ? keyFieldNode->value() : 0;
                if (!hostName || !hostName[0]) {
                    logOut(eQPLogType_Warning, "QPHosts update missing host name, skipping");
                    continue;
                }

                sQPrideBase::Host * h = hostUpdates.get(hostName, sLen(hostName));
                if (!h) {
                    h = hostUpdates.set(hostName, sLen(hostName));
                    sSet(h);
                    strncpy(h->name, hostName, sizeof(h->name) - 1);
                    h->updateMask = 0;
                }

                if (!applyHostFieldFromForm(*h, col, val)) {
                    logOut(eQPLogType_Warning, "Unknown QPHosts column: %s", col);
                }
            } else if (strcmp(tbl, "QPCfg") == 0) {
                const char * par = keyFieldNode ? keyFieldNode->value() : 0;
                if (!par || !par[0]) {
                    logOut(eQPLogType_Warning, "QPCfg update missing parameter name, skipping");
                    continue;
                }
                if (strcmp(col, "val") == 0) {
                    sStr * v = cfgUpdates.set(par, sLen(par));
                    v->cutAddString(0, val);
                } else {
                    logOut(eQPLogType_Warning, "Unsupported QPCfg column: %s", col);
                }
            } else if (strcmp(tbl, "QPSvc") == 0) {
                const char * svcName = keyFieldNode ? keyFieldNode->value() : 0;
                if (!svcName || !svcName[0]) {
                    logOut(eQPLogType_Warning, "QPSvc update missing service name, skipping");
                    continue;
                }

                sQPrideBase::Service * s = svcUpdates.get(svcName, sLen(svcName));
                if (!s) {
                    s = svcUpdates.set(svcName, sLen(svcName));
                    sSet(s);
                    if (!serviceGet(s, svcName)) {
                        logOut(eQPLogType_Warning, "QPSvc service not found: %s", svcName);
                        continue;
                    }
                }

                if (!applySvcFieldFromForm(*s, col, val)) {
                    logOut(eQPLogType_Warning, "Unknown QPSvc column: %s", col);
                }
            }
        }

        sStr updateLog;
        for (idx i = 0; i < hostUpdates.dim(); ++i) {
            sQPrideBase::Host * h = hostUpdates.ptr(i);
            if (h->updateMask) {
                hostSet(h, 1, &updateLog);
                logOut(eQPLogType_Info, "%s", updateLog.ptr());
                updateLog.cut(0);
            }
        }

        for (idx i = 0; i < svcUpdates.dim(); ++i) {
            sQPrideBase::Service * s = svcUpdates.ptr(i);
            if (!s->svcID) continue;
            updateLog.cut(0);
            serviceSet(s, 1, &updateLog);
            logOut(eQPLogType_Info, "%s", updateLog.ptr());
        }

        for (idx i = 0; i < cfgUpdates.dim(); ++i) {
            const char * par = (const char *)cfgUpdates.id(i);
            const char * val = cfgUpdates.ptr(i)->ptr();
            if (configSet(par, "%s", val)) {
                logOut(eQPLogType_Info, "QPCfg updated: %s = %s", par, val);
            } else {
                logOut(eQPLogType_Warning, "QPCfg update failed for: %s", par);
            }
        }
       
    } else {
        logOut(eQPLogType_Info, "No col_list found in form tree");
    }
   
    bool update = formBoolValue("update", true);

    const char * objType00 = "hive-service" _ "hive-hosts" _ "hive-config" __;

    for (const char *objType = objType00; objType; objType=sString::next00(objType)) {
        sUsrObjRes obj_res; user->objs2(objType, obj_res,(udx*)0);
        
        if (strncmp(objType,"hive-service",12) ==0) {
            sVec<sQPrideBase::Service> svcL;
            serviceList(0,&svcL);        
            for (idx i=0; i<svcL.dim(); i++) {
                const sQPrideBase::Service & svc = svcL[i];
                const char * serviceName = svc.name;
                
                sHiveId existingId;
                ::printf("Service: %s\n", serviceName);

                sStr svcValBuf;
                svcValBuf.printf("%s", serviceName); svcValBuf.add0();
                svcValBuf.printf("%" DEC "", svc.svcID); svcValBuf.add0();
                svcValBuf.add0();
                bool found = getExistingObjectTypeByObjectName(obj_res, existingId, "name" _ "svcID" __, svcValBuf.ptr());
                ::printf(" - Object '%s' for service \"%s\"\n", found ? "found" : "not found",serviceName);
                if (found && !update) {                    
                    continue;
                }
                ::printf(" - %s \n", update ? "Updating" : "Creating");
                sUsrObj * obj = getMyObject(objType, found, existingId);        
                
                if (obj) {
                    logOut(eQPLogType_Info, "Using object %s", obj->Id().print());
                    propSetHiveServiceObject(obj, svc);
                }
                delete obj;
            }
        }
        else if (strncmp(objType,"hive-hosts",10) ==0) {
            sVec<sQPrideBase::Host> hostL;
            hostList(0,&hostL);        
            for (idx i=0; i<hostL.dim(); i++) {
                const sQPrideBase::Host & host = hostL[i];
                const char * hostName = host.name;
                
                sHiveId existingId;
                ::printf("Host: %s\n", hostName);
                sStr hostValBuf;
                hostValBuf.printf("%s", hostName); hostValBuf.add0();
                hostValBuf.add0();
                bool found = getExistingObjectTypeByObjectName(obj_res, existingId, "name" __, hostValBuf.ptr());
                ::printf(" - Object '%s' for host \"%s\"\n", found ? "found" : "not found",hostName);
                if (found && !update) {
                    continue;
                }
                ::printf(" - %s \n", update ? "Updating" : "Creating");
                sUsrObj * obj = getMyObject(objType, found, existingId);        
                
                if (obj) {
                    logOut(eQPLogType_Info, "Using object %s", obj->Id().print());
                    propSetHiveHostObject(obj, host);
                }
                delete obj;
            }

        }
        else if (strncmp(objType,"hive-config",11) ==0) {
            sHiveId existingId;
            ::printf("Config name: %s\n", "qpcfg");
            bool found = getExistingObjectTypeByObjectName(obj_res, existingId, "name" __, "qpcfg" __);
            ::printf(" - Object '%s' for config \"%s\"\n", found ? "found" : "not found", "qpcfg");
             if (found && !update) {                
                continue;
            }
            ::printf(" - %s \n", update ? "Updating" : "Creating");
            sUsrObj * obj = getMyObject(objType, found, existingId);

            if (obj) {
                logOut(eQPLogType_Info, "Using object %s", obj->Id().print());
                propSetHiveCfgObject(obj);
            }
            delete obj;
        }
        
    }

   
    
    user = cur_user;

    reqProgress(0, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);

    reqReSubmit(req,10);

    return 0;
}


int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    HiveObjectifier backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "hive-objectifier", argv[0]));
    return (int) backend.run(argc, argv);
}
