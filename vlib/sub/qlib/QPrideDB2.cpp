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
#include <ctype.h>
#include <ssql/sql.hpp>
#include "QPrideDB.hpp"
#include "QPrideDB2.hpp"
#include "../ulib/uperm.hpp"
#include <qlib/QPrideBase.hpp>
#ifndef SLIB_WIN
#include <arpa/inet.h>
#include <ifaddrs.h>
#endif

#include <assert.h>


using namespace slib;

#define COL_ID 0
#define COL_NAME 1
#define COL_PATH 2
#define COL_VALUE 3

#define protectString(_v_prot, _v_val) db->protectValue((_v_prot), (_v_val), 0)

#define protectBuf(_v_prot, _v_val, _len) \
    do { \
        if( _len ) { \
            db->protectValue((_v_prot), (_v_val), (_len)); \
        } else { \
            db->protectValue((_v_prot), 0); \
        } \
        (_v_prot).shrink00(); \
    } while( 0 )

#define protectOptional(_v_prot, _v_val, _do_protect) \
    do { \
        if( _do_protect ) { \
            protectString((_v_prot), (_v_val)); \
        } else { \
            (_v_prot).addString((_v_val)); \
        } \
    } while( 0 )

#define protectName(_v_prot, _v_val) db->protectName((_v_prot), (_v_val), 0)

sSql::gSettings __sQPrideDB2_gDefaultSettings = {
    HIVE_DB,
    HIVE_DB_HOST,
    HIVE_DB_USER,
    HIVE_DB_PWD,
    1,
    50,
    "random",
    0 };

sUsr * sQPrideDB2::getUser()
{
    static std::auto_ptr<sUsr> _qpride;
    if( !_qpride.get() ) {
        _qpride.reset(new sUsr("queen"));
        if( _qpride.get() && _qpride->Id() ) {
            _qpride->m_SuperUserMode = true;
        } else {
#ifdef _DEBUG
            fprintf(stderr, "qpride user not get init\n");
#endif
        }
    }
    return _qpride.get();
}

sQPrideDB2::sQPrideDB2(sSql * lsql)
    : db(lsql), dbdel(0)
{

}

sQPrideDB2::sQPrideDB2(const char *defline)
    : db(new sSql)
{
    dbdel = db;
    if( db ) {
        db->gSet = __sQPrideDB2_gDefaultSettings;
        if( db->connect(defline) != sSql::eConnected ) {
#ifdef _DEBUG
            fprintf(stderr, "FATAL: Cannot connect to QPride\n");
#endif
        }
    }

}

sQPrideDB2::~sQPrideDB2()
{
    for(idx si = 0; si < _svcByName.dim(); si++) {
        if(_svcByName[si])
            delete _svcByName[si];
    }

    delete dbdel;
}


idx sQPrideDB2::_QP_populateCfgDic()
{
    if( !configCache.dim() ) {
        sVec<sUsrObj *> svcObjList;
        QP_serviceList(&svcObjList, "QPCfg_par");
        for(idx si = 0; si < svcObjList.dim(); si++) {
            const sUsrObjPropsTree * objPropsTree = svcObjList[si]->propsTree();
            sUsrObjPropsNode * arrayNode = objPropsTree ? (sUsrObjPropsNode *) objPropsTree->find("QPCfg") : 0;
            if( arrayNode ) {
                for(sUsrObjPropsNode * rowNode = arrayNode->firstChild(); rowNode; rowNode = rowNode->nextSibling()) {
                    sUsrObjPropsNode * parNode = rowNode->firstChild("QPCfg_par");
                    sUsrObjPropsNode * valNode = rowNode->firstChild("QPCfg_val");
                    if( parNode && valNode ) {
                        const char * parPtr = parNode->value(0);
                        const char * valPtr = valNode->value(0);
                        if( parPtr && valPtr ) {
                            configCache.inp(parPtr, valPtr);
                        }
                    } else {
#ifdef _DEBUG
                        fprintf(stderr, "node of props tree not get properly\n");
#endif
                    }
                }
            }
        }
    }
    return configCache.dim();
}

static void configGetVal(sStr & vals00, const sVar & cfg_cache, const char * par, idx ipar)
{
    idx parsz = 0;
    if( !par && ipar >= 0 ) {
        par = static_cast<const char *>(cfg_cache.id(ipar, &parsz));
    }
    if( *par ) {
        idx vsz = 0;
        const char * val = cfg_cache.value(par, 0, &vsz);
        if( val ) {
            vals00.add(val, vsz);
            vals00.add0();
            vals00.add(par, parsz);
            vals00.add0();
        }
    }
}

char * sQPrideDB2::QP_configGet(sStr * vals00, const char * pars00, bool single)
{
    if( vals00 ) {
        const idx pos = vals00->length();
        _QP_populateCfgDic();

        if( pars00 ) {
            for(const char * p = pars00; p; p = single ? 0 : sString::next00(p)) {
                configGetVal(*vals00, configCache, p, -1);
            }
        } else {
            for(idx ipar = 0; ipar < (single ? 1 : configCache.dim()); ipar++) {
                configGetVal(*vals00, configCache, 0, ipar);
            }
        }
        if( vals00->length() ) {
            vals00->add0(2);
        }
        return vals00->ptr(pos);
    }
    return 0;
}

bool sQPrideDB2::QP_configSet(const char * par, const char * val, const char * serviceName)
{

    if( par && val ) {
        sUsrObj * obj = QP_serviceGet(serviceName);
        if( obj ) {
            sVec<const char *> parmList;
            sVec<const char *> valList;
            sStr groupBuf;
            idx groupCnt = 1;
            sVec<const char *> groupList;
            parmList.vadd(1, par);
            valList.vadd(1, val);
            groupBuf.printf(0, "1.%" DEC, groupCnt++);
            groupList.vadd(1, groupBuf.ptr(0));

            _QP_populateCfgDic();
           configCache.inp(par,val);
            const sUsrObjPropsTree * objPropsTree = obj->propsTree();
            sUsrObjPropsNode * arrayNode = objPropsTree ? (sUsrObjPropsNode *) objPropsTree->find("QPCfg") : 0;
            if( arrayNode ) {
                for(sUsrObjPropsNode * rowNode = arrayNode->firstChild(); rowNode; rowNode = rowNode->nextSibling()) {
                    sUsrObjPropsNode * parNode = rowNode->firstChild("QPCfg_par");
                    const char * parPtr = parNode ? parNode->value(0) : 0;
                    if( parPtr ) {
                        if( strcmp(parNode->value(0), par) == 0 ) {
                            continue;
                        } else {
                            sUsrObjPropsNode * valNode = rowNode->firstChild("QPCfg_val");
                            if( valNode && valNode->value(0) ) {
                                parmList.vadd(1, parPtr);
                                valList.vadd(1, valNode->value(0));
                                groupBuf.printf(0, "1.%" DEC, groupCnt++);
                                groupList.vadd(1, groupBuf.ptr(0));
                            }
                        }
                    }
                }
            }

            if( parmList.dim() ) {
                reloadPropTree(serviceName, obj->Id());
                return obj->propSet("QPCfg_par", groupList.ptr(), parmList.ptr(), parmList.dim()) && obj->propSet("QPCfg_val", groupList.ptr(), valList.ptr(), valList.dim());
            }
        }
    }
    return false;
}

idx sQPrideDB2::QP_reqSubmit(const char * serviceName, idx subip, idx priority, idx usrid)
{
    idx req = 0;
    sUsrObj * obj = QP_serviceGet(serviceName);
    if( obj ) {
        const sUsrObjPropsTree * objPropsTree = obj->propsTree();
        idx cleanUpDays = 0;
        if( objPropsTree ) {
            cleanUpDays = objPropsTree->findIValue("cleanUpDays");
        }else{
            cleanUpDays = obj->propGetI("cleanUpDays");
        }
        if( !cleanUpDays ) {
            sUsrObj * qm = QP_serviceGet("qm");
            if( qm ) {
                const sUsrObjPropsTree * qmPropsTree = qm->propsTree();
                if( qmPropsTree ) {
                    cleanUpDays = qmPropsTree->findIValue("cleanUpDays");
                }else{
                    cleanUpDays = qm->propGetI("cleanUpDays");
                }
            }
        }
        sSql::sqlProc * sp = db->Proc("sp_req_submit");
        if( sp ) {
            sp->Add(obj->Id().objId()).Add(cleanUpDays).Add((idx) 0).Add(priority).Add(subip).Add(usrid);
            req = sp->ivalue(req);
            delete sp;
        }
    }
    return req;
}

idx sQPrideDB2::QP_reqGrab(const char * service, idx job, idx inBundle, idx status, idx action)
{
    idx req = 0;
    sUsrObj * obj = QP_serviceGet(service);
    if( obj ) {
        sSql::sqlProc sql(*db, "sp_req_grab");
        sql.Add(obj->Id().objId()).Add(job).Add(status).Add(action).Add(inBundle);
        req = sql.ivalue(0);
    }
    return req;
}

idx sQPrideDB2::QP_reqReSubmit(idx * req, idx cnt, idx delaySeconds)
{
    if( req && cnt ) {
        sStr sql;
        sql.addString("UPDATE QPReq SET stat = 1, takenCnt = 0, progress = 0, progress100 = 0, grabRand = 0, takenTm = cdate, doneTm = 0, actTm = NOW()");
        if( delaySeconds ) {
            sql.printf(", scheduleGrab = TIMESTAMPADD(SECOND, %" DEC ", NOW())", delaySeconds);
        }
        sql.printf(" WHERE ");
        db->exprInList(sql, "reqID", req, cnt, false);
        db->executeString(sql.ptr());
    }
    return req ? *req : 0;
}

idx sQPrideDB2::QP_requestGetForGrp(idx grp, void * RList, const char * serviceName)
{
    sVec<sQPrideBase::Request> * rList = (sVec<sQPrideBase::Request> *) RList;
    idx count;
    sStr sql;

    sql.printf("SELECT COUNT(QPReq.reqID) FROM QPGrp JOIN QPReq USING(reqID)");

    if( serviceName ) {
        idx svcId = QP_serviceGet(0, serviceName);
        sql.printf(" WHERE svcID = %" DEC " AND grpID = %" DEC, svcId, grp);
    } else {
        sql.printf(" WHERE grpID = %" DEC, grp);
    }
    count = db->ivalue(sql.ptr(), 0);
    if( count > 0 ) {
        rList->add(count);
    } else {
        sStr sql2;
        sql2.printf("SELECT COUNT(QPReq.reqID) FROM QPReq WHERE reqID = %" DEC, grp);
        count = db->ivalue(sql2.ptr(), 0);
        if( count > 0 ) {
            rList->add(count);
            return sQPrideDB2::QP_requestGet(grp, rList->ptr(), false);
        } else {
            return 0;
        }
    }
    return sQPrideDB2::QP_requestGet(grp, rList->ptr(), true, serviceName);
}

idx sQPrideDB2::QP_getReqByUserKey(idx userKey, const char * serviceName)
{
    sStr sql;
    sql.printf("SELECT reqID FROM QPReq WHERE userKey = %" DEC, userKey);
    if( serviceName ) {
        idx svcId = QP_serviceGet(0, serviceName);
        if( svcId )
            sql.printf(" AND svcID = %" DEC, svcId);
    }
    idx req = db->ivalue(sql.ptr(), 0);
    return req;
}

idx sQPrideDB2::QP_requestGet(idx req, void * R, bool isGrp, const char * serviceName)
{
    sStr sql("SELECT QPReq.reqID,QPReq.svcID,jobID,userID,subIp,cgiIp,stat,act,takenCnt,"
        "QPReq.priority,inParallel,progress,progress100,userKey,"
        "UNIX_TIMESTAMP(takenTm),UNIX_TIMESTAMP(QPReq.cdate),UNIX_TIMESTAMP(actTm), UNIX_TIMESTAMP(aliveTm),"
        "UNIX_TIMESTAMP(doneTm),UNIX_TIMESTAMP(purgeTm), QPReq.svcID, UNIX_TIMESTAMP(QPReq.scheduleGrab)");
    if( isGrp ) {
        sql.printf(",QPGrp.grpID FROM QPGrp JOIN QPReq USING(reqID) WHERE grpID = %" DEC, req);
        if( serviceName ) {
            idx svcID = QP_serviceGet(0, serviceName);
            sql.printf(" AND svcID = %" DEC " ", svcID);
        }
        sql.printf(" ORDER BY QPReq.reqID");
    } else {
        sql.printf(" FROM QPReq WHERE reqID = %" DEC, req);
    }

    sVarSet res;
    db->getTable(sql.ptr(), &res);

    sQPrideBase::Request * r = (sQPrideBase::Request *) R;
    for(idx i = 0; i < res.rows; ++i) {
        idx o = 0;
        r->reqID = res.ival(i, o++);
        r->svcID = res.ival(i, o++);
        r->jobID = res.ival(i, o++);
        r->userID = res.ival(i, o++);
        r->subIp = res.ival(i, o++);
        r->cgiIp = res.ival(i, o++);
        r->stat = res.ival(i, o++);
        r->act = res.ival(i, o++);
        r->takenCnt = res.ival(i, o++);
        r->priority = res.ival(i, o++);
        r->inParallel = res.ival(i, o++);
        r->progress = res.ival(i, o++);
        r->progress100 = res.ival(i, o++);
        r->userKey = res.ival(i, o++);
        r->takenTm = res.ival(i, o++);
        r->cdate = res.ival(i, o++);
        r->actTm = res.ival(i, o++);
        r->aliveTm = res.ival(i, o++);
        r->doneTm = res.ival(i, o++);
        r->purgeTm = res.ival(i, o++);
        idx svcID = res.ival(i, o++);
        const char * name = QP_serviceName(svcID);
        r->scheduleGrab = res.ival(i, o++);
        if( name ) {
            strncpy(r->svcName, name, sizeof(r->svcName) - 1);
        } else {
            strncpy(r->svcName, "unDefined", sizeof(r->svcName) - 1);
        }
        if( isGrp ) {
            r->grpID = res.ival(i, o++);
        } else {
            r->grpID = 0;
        }
        ++r;
    }
    return res.rows ? req : 0;
}

idx sQPrideDB2::QP_reqSetProgress(idx req, idx progress, idx progress100)
{
    sStr sql("UPDATE QPReq SET progress = IF(%" DEC " >= 0, %" DEC ", progress)", progress, progress);
    if( progress100 >= 0 && progress100 <= 100 ) {
        sql.printf(", progress100 = %" DEC, progress100);
    }
    sql.printf(" WHERE reqID = %" DEC, req);
    db->executeString(sql);
    return progress;
}

idx sQPrideDB2::QP_reqSetCgiIP(idx req, idx val)
{
    return sQPrideDB::_QP_SetIdxVar(db, req, val, "QPReq", "reqID", "cgiIp");
}
idx sQPrideDB2::QP_reqSetSubIP(idx req, idx val)
{
    return sQPrideDB::_QP_SetIdxVar(db, req, val, "QPReq", "reqID", "subIp");
}
idx sQPrideDB2::QP_reqSetAction(idx req, idx val)
{
    return sQPrideDB::_QP_SetIdxVar(db, req, val, "QPReq", "reqID", "act", ", actTm = NOW()");
}
idx sQPrideDB2::QP_reqSetAction(sVec<idx> * reqs, idx val)
{
    return sQPrideDB::_QP_SetIdxVar(db, reqs, val, "QPReq", "reqID", "act", ", actTm = NOW()");
}
idx sQPrideDB2::QP_reqGetAction(idx req)
{
    return sQPrideDB::_QP_GetIdxVar(db, req, "QPReq", "reqID", "act");
}

idx sQPrideDB2::QP_reqGetAction(sVec<idx> * reqs, sVec<idx> * vals)
{
    return sQPrideDB::_QP_GetIdxVar(db, reqs, vals, "QPReq", "reqID", "act");
}

idx sQPrideDB2::QP_reqGetUser(idx req)
{
    return sQPrideDB::_QP_GetIdxVar(db, req, "QPReq", "reqID", "userID");
}

void sQPrideDB2::QP_reqRegisterAlive(idx reqID)
{
    db->execute("UPDATE QPReq SET aliveTm=NOW() WHERE reqID = %" DEC, reqID);
}

idx sQPrideDB2::QP_reqSetStatus(idx req, idx val)
{
    const char * more=(val>=sQPrideBase::eQPReqStatus_Done) ? ", doneTm = NOW()" : ((val==sQPrideBase::eQPReqStatus_Processing)? ", takenTm = NOW()" : 0);
    return sQPrideDB::_QP_SetIdxVar(db, req, val, "QPReq", "reqID", "stat", more);
}
idx sQPrideDB2::QP_reqSetStatus(sVec<idx> * reqs, idx val)
{
    const char * more=(val>=sQPrideBase::eQPReqStatus_Done) ? ", doneTm = NOW()" : ((val==sQPrideBase::eQPReqStatus_Processing)? ", takenTm = NOW()" : 0);
    return sQPrideDB::_QP_SetIdxVar(db, reqs, val, "QPReq", "reqID", "stat", more);
}
idx sQPrideDB2::QP_reqGetStatus(idx req)
{
    return sQPrideDB::_QP_GetIdxVar(db, req, "QPReq", "reqID", "stat");
}
idx sQPrideDB2::QP_reqGetStatus(sVec<idx> * reqs, sVec<idx> * vals)
{
    return sQPrideDB::_QP_GetIdxVar(db, reqs, vals, "QPReq", "reqID", "stat");
}

idx sQPrideDB2::QP_reqSetUserKey(idx req, idx val)
{
    return sQPrideDB::_QP_SetIdxVar(db, req, val, "QPReq", "reqID", "userKey", ", actTm = NOW()");
}
idx sQPrideDB2::QP_reqSetUserKey(sVec<idx> * reqs, idx val)
{
    return sQPrideDB::_QP_SetIdxVar(db, reqs, val, "QPReq", "reqID", "userKey", ", actTm = NOW()");
}
idx sQPrideDB2::QP_reqGetUserKey(idx req)
{
    return sQPrideDB::_QP_GetIdxVar(db, req, "QPReq", "reqID", "userKey");
}

idx sQPrideDB2::QP_purgeReq(sVec<idx> * recVec, idx stat)
{
    return sQPrideDB::_QP_SetIdxVar(db, recVec, stat, "QPReq", "reqID", "stat", ", purgeTm = NOW()");
}


bool sQPrideDB2::QP_reqLock(idx req, const char * key, idx * preq_locked_by, idx max_lifetime, bool force)
{
    sVarSet res;
    idx req_locked_by = 0;
    bool ret = false;

    sSql::sqlProc sql(*db, "sp_req_lock");
    sql.Add(req).Add(key).Add(max_lifetime).Add(force).Add((idx)sQPrideBase::eQPReqStatus_Done);
    if( sql.getTable(&res) && res.rows == 1 ) {
        req_locked_by = res.ival(0, res.colId("reqID"));
        ret = (req_locked_by == req);
    }
    if( preq_locked_by ) {
        *preq_locked_by = req_locked_by;
    }
    return ret;
}

bool sQPrideDB2::QP_reqUnlock(idx req, const char * key, bool force)
{
    sSql::sqlProc sql(*db, "sp_req_unlock");
    sql.Add(req).Add(key).Add(force);
    return sql.ivalue(0);
}

idx sQPrideDB2::QP_reqCheckLock(const char * key)
{
    sVarSet res;
    sSql::sqlProc sql(*db, "sp_req_lock");
    sql.Add((idx)0).Add(key).Add((idx)0).Add((idx)-1).Add((idx)sQPrideBase::eQPReqStatus_Done);
    sql.getTable(&res);
    return res.ival(0, res.colId("reqID"));
}

idx sQPrideDB2::QP_grpSubmit(const char * serviceName, idx subip, idx priority, idx numSubReqs, idx usrid, idx previousGrpSubmitCounter)
{
    idx grp = 0;

    sUsrObj * obj = QP_serviceGet(serviceName);
    if( obj ) {
        const sUsrObjPropsTree * objPropsTree = obj->propsTree();
        idx cleanUpDays = 0;
        idx parallelJobs = 0;
        if( objPropsTree ) {
            cleanUpDays = objPropsTree->findIValue("cleanUpDays");
            parallelJobs = objPropsTree->findIValue("parallelJobs");
        }else{
            cleanUpDays = obj->propGetI("cleanUpDays");
            parallelJobs= obj->propGetI("parallelJobs");
        }
        if( !cleanUpDays ) {
            sUsrObj * qm = QP_serviceGet("qm");
            if( qm ) {
                const sUsrObjPropsTree * qmPropsTree = qm->propsTree();
                if( qmPropsTree ) {
                    cleanUpDays = qmPropsTree->findIValue("cleanUpDays");
                }else{
                    cleanUpDays = qm->propGetI("cleanUpDays");
                }
            }
        }
sSql::sqlProc * sp = db->Proc("sp_grp_submit_vBES");
        if( sp ) {
            sp->Add(obj->Id().objId()).Add(cleanUpDays).Add(parallelJobs).Add((idx) previousGrpSubmitCounter).Add(priority).Add(subip).Add(numSubReqs).Add(usrid);
            grp = sp->ivalue(grp);
            delete sp;
        }
    }
    return grp;
}

idx sQPrideDB2::QP_grpSubmit2(const char * serviceName, idx subip, const sQPrideBase::PriorityCnt * priority_cnts, idx num_priority_cnts, idx num_subreqs, idx user_id, idx prev_num_subreqs, idx grp_id)
{
    idx ret_grp_id = 0;
    if( const sUsrObj * obj = QP_serviceGet(serviceName) ) {
        const sUsrObjPropsTree * objPropsTree = obj->propsTree();
        idx cleanUpDays = 0;
        idx parallelJobs = 0;
        if( objPropsTree ) {
            cleanUpDays = objPropsTree->findIValue("cleanUpDays");
            parallelJobs = objPropsTree->findIValue("parallelJobs");
        } else {
            cleanUpDays = obj->propGetI("cleanUpDays");
            parallelJobs = obj->propGetI("parallelJobs");
        }
        if( !cleanUpDays ) {
            sUsrObj * qm = QP_serviceGet("qm");
            if( qm ) {
                const sUsrObjPropsTree * qmPropsTree = qm->propsTree();
                if( qmPropsTree ) {
                    cleanUpDays = qmPropsTree->findIValue("cleanUpDays");
                } else {
                    cleanUpDays = qm->propGetI("cleanUpDays");
                }
            }
        }

        sStr priority_cnt_csv;
        if( priority_cnts ) {
            for(idx i = 0; i < num_priority_cnts; i++) {
                if( i ) {
                    priority_cnt_csv.addString(",");
                }
                priority_cnt_csv.printf("%" DEC ",%" DEC, priority_cnts[i].priority, priority_cnts[i].cnt);
            }
        }

        sSql::sqlProc sql(*db, "sp_grp_submit_v2");
        sql.Add(obj->Id().objId()).Add(cleanUpDays).Add(prev_num_subreqs).Add(priority_cnt_csv.ptr()).Add(subip).Add(num_subreqs ? num_subreqs : parallelJobs).Add(user_id).Add(grp_id);
        ret_grp_id = sql.ivalue(0);
    }
    return ret_grp_id;
}

idx sQPrideDB2::QP_grpAssignReqID(idx grp, idx req, idx jobIDSerial)
{
    sSql::sqlProc sql(*db, "grpIns");
    sql.Add(grp).Add(req).Add(jobIDSerial);
    return sql.ivalue(grp);
}
idx sQPrideDB2::QP_grp2Req(idx grp, sVec<idx> * reqs, const char * svc, idx masterGroup)
{
    sStr sql;
    sVarSet res;
    idx v;
    sStr smaster;
    if( masterGroup )
        smaster.printf("AND QPGrp.masterGrpID=%" DEC, masterGroup < 0 ? grp : masterGroup);
    if( svc && *svc ) {
        idx svcId = QP_serviceGet(0, svc);
        if( svcId )
            sql.printf("SELECT QPGrp.reqID FROM QPGrp, QPReq WHERE QPReq.reqID=QPGrp.reqID %s AND QPGrp.grpID=%" DEC " AND svcID =%" DEC " ORDER BY QPGrp.jobIDCollect, QPGrp.reqID ", smaster ? smaster.ptr() : "", grp, svcId);
    } else
        sql.printf("SELECT reqID FROM QPGrp WHERE grpID=%" DEC " %s ORDER BY jobIDCollect, reqID", grp, smaster ? smaster.ptr() : "");
    db->getTable(sql.ptr(), &res);
    for(idx i = 0; i < res.rows; ++i) {
        if( sscanf(res(i, 0), "%" DEC, &v) != 1 ) {
            break;
        }
        reqs->vadd(1, v);
    }
    return reqs->dim() ? (*reqs)[0] : grp;
}

idx sQPrideDB2::QP_req2Grp(idx req, sVec<idx> * grps, bool isMaster)
{
    sVarSet res;
    idx v;
    if( isMaster ) {
        db->getTable(&res, "SELECT masterGrpID FROM QPGrp WHERE reqID=%" DEC " AND masterGrpID IS NOT NULL", req);
    } else {
        db->getTable(&res, "SELECT grpID FROM QPGrp WHERE reqID=%" DEC " ORDER BY grpID ASC", req);
    }
    if( !grps ) {
        return res.ival(0, 0, req);
    }
    for(idx i = 0; i < res.rows; ++i) {
        if( sscanf(res(i, 0), "%" DEC, &v) != 1 ) {
            break;
        }
        grps->vadd(1, v);
    }
    return grps->dim() ? (*grps)[0] : req;
}

idx sQPrideDB2::QP_req2GrpSerial(idx req, idx grp, idx * pcnt, idx svc)
{

    if( pcnt ) {
        if( svc != sNotIdx )
            *pcnt = db->ivalue(0, "SELECT COUNT(QPReq.reqID) FROM QPGrp, QPReq WHERE QPGrp.reqID=QPReq.reqID AND svcID=%" DEC " AND QPGrp.grpID=%" DEC, svc, grp);
        else
            *pcnt = db->ivalue(0, "SELECT COUNT(*) FROM QPGrp WHERE grpID=%" DEC, grp);
    }
    return db->ivalue(0, "SELECT jobIDCollect FROM QPGrp WHERE grpID=%" DEC " AND reqID=%" DEC " LIMIT 1", grp, req);
}

idx sQPrideDB2::QP_grpGetProgress(idx grp, idx * progress, idx * progress100)
{
    idx prg[2];
    sSetArray(prg);
    sSql::sqlProc * p = db->Proc("sp_grp_progress");
    if( p ) {
        sVarSet t;
        p->Add(grp);
        p->getTable(&t);
        if( t.rows > 0 ) {
            if( t.cols > 0 ) {
                prg[0] = t.ival(0, 0);
            }
            if( t.cols > 1 ) {
                prg[1] = t.ival(0, 1);
            }
        }
        delete p;
    }
    if( progress ) {
        *progress = prg[0];
    }
    if( progress100 ) {
        *progress100 = prg[1];
    }
    return prg[0];
}

bool sQPrideDB2::QP_reqSetPar(idx req, idx type, const char * value, bool isOverwrite)
{
    sStr sql;
    if( isOverwrite == 0 || db->ivalue(-1, "SELECT reqID FROM QPReqPar WHERE reqID=%" DEC " AND type=%" DEC, req, type) == -1 ) {
        sql.printf("INSERT INTO QPReqPar (reqID, type, val) VALUES (%" DEC ", %" DEC ", ", req, type);
        protectString(sql, value);
        sql.addString(")");
    } else {
        sql.addString("UPDATE QPReqPar SET val = ");
        protectString(sql, value);
        sql.printf(" WHERE reqID = %" DEC " AND type = %" DEC, req, type);
    }
    return db->executeString(sql.ptr());
}

char * sQPrideDB2::QP_requestGetPar(idx req, idx type, sStr * val)
{
    db->getBlob(val->mex(), "SELECT val FROM QPReqPar WHERE reqID=%" DEC " AND type=%" DEC, req, type);
    val->add0();
    return val->ptr();
}

char * sQPrideDB2::QP_reqDataGet(idx req, const char * dataName, sMex * data, idx * timestamp)
{
    sStr sql("SELECT dataBlob, UNIX_TIMESTAMP(modTm) FROM QPData WHERE reqID = %" DEC " AND dataName = ", req);
    protectString(sql, dataName);

    sMex enc;
    idx pos = data->pos();
    sVarSet res;

    if( db->getTable(&res, sql.ptr()) ) {
        idx encLen = 0;
        const char * enc = res.val(0, 0, &encLen);
        sString::decodeBase64(data, enc, encLen);
        if( timestamp ) {
            *timestamp = res.ival(0, 1, sIdxMax);
        }
        return (char*) data->ptr(pos);
    }
    return 0;
}

idx sQPrideDB2::QP_reqDataGetTimestamp(idx req, const char * dataName)
{
    sStr sql("SELECT UNIX_TIMESTAMP(modTm) FROM QPData WHERE reqID = %" DEC " AND dataName = ", req);
    protectString(sql, dataName);

    return db->ivalue(sql.ptr(), sIdxMax);
}

bool sQPrideDB2::QP_reqDataSet(idx req, const char * dataName, idx dsize, const void * data)
{
    sStr enc;
    if( data ) {
        if( dsize == 0 ) {
            dsize = sLen(data) + 1;
        }
        sString::encodeBase64(&enc, (const char *) data, dsize, true);
        enc.add0();
    }
    sSql::sqlProc sql(*db, "sp_req_data_set");
    sql.Add(req).Add(dataName).Add(data ? enc.ptr() : 0);
    return sql.execute();
}

idx sQPrideDB2::QP_reqDataGetAll(idx req, sVec<sStr> * dataVec, sStr * infos00, sVec<idx> * timestampVec)
{
    sStr sql;
    sVarSet res;

    db->getTable(&res, "SELECT dataName FROM QPData WHERE reqID = %" DEC, req);
    for(idx i = 0; i < res.rows; ++i) {
        char * dataName = res(i, 0);
        if( infos00 ) {
            infos00->add(dataName, 0);
        }

        if( dataVec ) {
            sStr * str = dataVec->add();
            idx stamp = sIdxMax;
            QP_reqDataGet(req, dataName, str, &stamp);
            if( timestampVec ) {
                timestampVec->vadd(1, stamp);
            }
        } else if( timestampVec ) {
            timestampVec->vadd(1, QP_reqDataGetTimestamp(req, dataName));
        }
    }
    if( infos00 )
        infos00->add0(2);

    return res.rows;
}


char * sQPrideDB2::QP_resourceGet(const char * service, const char * dataName, sMex * data, idx * tmstmp)
{
    sStr sql("SELECT dataBlob FROM QPResource WHERE svcName = ");
    protectString(sql, service);
    sql.addString(" AND dataName = ");
    protectString(sql, dataName);

    sMex enc;
    idx pos = data->pos();
    if( db->getTable(sql.ptr(), 0, &enc) ) {
        sString::decodeBase64(data, (char*) enc.ptr(), enc.pos());
        return (char*) data->ptr(pos);
    }
    if( tmstmp ) {
        sql.cutAddString(0, "SELECT UNIX_TIMESTAMP(modTm) FROM QPResource WHERE svcName = ");
        protectString(sql, service);
        sql.addString(" AND dataName = ");
        protectString(sql, dataName);
        *tmstmp = db->ivalue(sql.ptr(), 0);
    }
    return 0;
}

bool sQPrideDB2::QP_resourceSet(const char * service, const char * dataName, idx dsize, const void * data)
{
    sStr enc;
    if( dsize == 0 ) {
        dsize = sLen(data) + 1;
    }
    sString::encodeBase64(&enc, (const char *) data, dsize, true);
    enc.add0();
    sSql::sqlProc sql(*db, "resourceIns");
    sql.Add(service).Add(dataName).Add(enc.ptr());
    return sql.execute();
}

idx sQPrideDB2::QP_resourceGetAll(const char * service, sStr * infos00, sVec<sStr> * dataVec, sVec<idx> * tmStmpts)
{
    sStr sql;
    sVarSet res;

    sql.printf("SELECT dataName, UNIX_TIMESTAMP(modTm) FROM QPResource");
    if( service ) {
        sql.addString(" WHERE svcName = ");
        protectString(sql, service);
    }
    db->getTable(sql.ptr(), &res);
    for(idx i = 0; i < res.rows; ++i) {
        char * dataName = res(i, 0);
        if( infos00 ) {
            infos00->add(dataName, 0);
        }
        if( tmStmpts ) {
            tmStmpts->vadd(1, res.ival(i, 1));
        }
        if( dataVec ) {
            sStr * str = dataVec->add();
            QP_resourceGet(service, dataName, str, 0);
        }
    }
    if( infos00 && infos00->length() )
        infos00->add0(2);

    return res.rows;
}

bool sQPrideDB2::QP_resourceDel(const char * service, const char * dataName)
{
    if( service ) {
        sStr sql("DELETE FROM QPResource WHERE svcName = ");
        protectString(sql, service);
        if( dataName ) {
            sql.addString(" AND dataName = ");
            protectString(sql, dataName);
        }
        db->executeString(sql.ptr());
        return true;
    }
    return false;
}

idx sQPrideDB2::QP_serviceID(const char * serviceName)
{
    return QP_serviceGet(0, serviceName);
}

idx sQPrideDB2::QP_serviceGet(void * Svc, const char * serviceName, idx svcId)
{
    sUsrObj * obj = QP_serviceGet(serviceName, svcId);
    if( obj && Svc ) {
        sQPrideBase::Service * svc = (sQPrideBase::Service *) Svc;
        _QP_populateSvc(svc, obj);
    }
    return obj ? obj->Id().objId() : 0;
}

sUsrObj * const sQPrideDB2::QP_serviceGet(const char * serviceName, idx svcId)
{
    sUsrObj * obj = 0;
    sUsrObj ** objPtr = 0;
    if( svcId ) {
        objPtr = _svcById.get(&svcId, sizeof(svcId));
        obj = objPtr ? *objPtr : 0;
        if( !obj ) {
            if( !getUser() ) {
#ifdef _DEBUG
                fprintf(stderr, "qpride user not get init\n");
#endif
                return obj;
            }
            sHiveId objId(svcId, 0);
            obj = getUser()->objFactory(objId);
            if( obj ) {
                const sUsrObjPropsTree * objPropsTree = obj->propsTree();
                const char * namePtr = 0;
                if( objPropsTree ) {
                    namePtr = objPropsTree->findValue("name");
                }else{
                    namePtr = obj->propGet("name");
                }
                if( namePtr ) {
                    sUsrObj ** ptr1 = _svcById.set(&svcId, sizeof(svcId));
                    sUsrObj ** ptr2 = _svcByName.set(namePtr, 0);
                    if( ptr1 ) {
                        *ptr1 = obj;
                    }
                    if( ptr2 ) {
                        *ptr2 = obj;
                    }
                }
            }
        }
    } else if( serviceName ) {
        objPtr = _svcByName.get(serviceName);
        obj = objPtr ? *objPtr : 0;
        if( !obj ) {
            if( !getUser() ) {
#ifdef _DEBUG
                fprintf(stderr, "qpride user not get init\n");
#endif
                return obj;
            }
            sUsrObjRes objIdList;
            sStr nameBuf;
            nameBuf.printf("^%s$", serviceName);
            getUser()->objs2("qpsvc", objIdList, 0, "name", nameBuf.ptr(0));
            if( objIdList.dim() == 1 ) {
                obj = QP_serviceGet(0, objIdList.firstId()->objId());
            } else {
#ifdef _DEBUG
                fprintf(stderr, "For service '%s' found %" UDEC " objects\n", serviceName, objIdList.dim());
#endif
            }
        }
    }
    return obj;
}

const char * sQPrideDB2::QP_serviceName(idx svcId)
{
    if( svcId ) {
        sUsrObj * obj = QP_serviceGet(0, svcId);
        if( obj ) {
            const sUsrObjPropsTree * objPropsTree = obj->propsTree();
            if( objPropsTree ) {
                return objPropsTree->findValue("name");
            }else{
                return obj->propGet("name", 0);
            }
        }
    }
    return 0;
}

void sQPrideDB2::_QP_populateSvc(sQPrideBase::Service * svc, const sUsrObj * obj)
{
    const sUsrObjPropsTree * objPropsTree = 0;
    if( svc && obj ) {
        svc->svcID = obj->Id().objId();
        objPropsTree = obj->propsTree();
    }
    if( objPropsTree ) {
        svc->permID = objPropsTree->findIValue("permID");
        svc->svcType = objPropsTree->findIValue("svcType");
        svc->knockoutSec = objPropsTree->findIValue("knockoutSec");
        svc->maxJobs = objPropsTree->findIValue("maxJobs");
        svc->nice = objPropsTree->findIValue("nice");
        svc->maxLoops = objPropsTree->findIValue("maxLoops");
        svc->sleepTime = objPropsTree->findIValue("sleepTime");
        svc->parallelJobs = objPropsTree->findIValue("parallelJobs");
        svc->delayLaunchSec = objPropsTree->findIValue("delayLaunchSec");
        svc->politeExitTimeoutSec = objPropsTree->findIValue("politeExitTimeoutSec");
        svc->maxTrials = objPropsTree->findIValue("maxTrials");
        svc->restartSec = objPropsTree->findIValue("restartSec");
        svc->priority = objPropsTree->findIValue("priority");
        svc->cleanUpDays = objPropsTree->findIValue("cleanUpDays");
        svc->noGrabDisconnect = objPropsTree->findIValue("noGrabDisconnect");
        svc->noGrabExit = objPropsTree->findIValue("noGrabExit");
        svc->lazyReportSec = objPropsTree->findIValue("lazyReportSec");
        svc->maxmemHard = objPropsTree->findIValue("maxmemHard");
        svc->maxmemSoft = objPropsTree->findIValue("maxmemSoft");
        svc->capacity = objPropsTree->findRValue("capacity");
        svc->activeJobReserve = objPropsTree->findIValue("activeJobReserve");
        svc->runInMT = objPropsTree->findBoolValue("runInMT");
        svc->isUp = objPropsTree->findBoolValue("isUp");
        const char * name = objPropsTree->findValue("name");
        if( name ) {
            strncpy(svc->name, name, sizeof(svc->name) - 1);
        }
        const char * title = objPropsTree->findValue("title");
        if( title ) {
            strncpy(svc->title, title, sizeof(svc->title) - 1);
        }
        const char * cmdLine = objPropsTree->findValue("cmdLine");
        if( cmdLine ) {
            strncpy(svc->cmdLine, cmdLine, sizeof(svc->cmdLine) - 1);
        }
        const char * categories = objPropsTree->findValue("categories");
        if( categories ) {
            strncpy(svc->categories, categories, sizeof(svc->categories) - 1);
        }
        const char * hosts = objPropsTree->findValue("hosts");
        if( hosts ) {
            strncpy(svc->hosts, hosts, sizeof(svc->hosts) - 1);
        }
    } else {
#ifdef _DEBUG
        fprintf(stderr, "Missing objId or svc\n");
#endif
    }
}

idx sQPrideDB2::QP_serviceUp(const char * svc, idx isUpMask)
{
    sUsrObj * obj = QP_serviceGet(svc);
    if( obj ) {
        obj->propSetI("isUp", isUpMask);
        reloadPropTree(svc,obj->Id());
    }
    return isUpMask;
}

sVar * sQPrideDB2::QP_getVars(sStr * dst, const char * src, idx len)
{
    if(src ) {
        sStr par00,val00;
        for ( idx i=0; i<configCache.dim() ; ++i){
            const char * id=(const char *) configCache.id(i);
            par00.printf("$(%s)",id); par00.add0();
            val00.printf("%s",(const char *) configCache.value(id)); val00.add0();
        }
        sString::searchAndReplaceStrings(dst,src,len, par00.ptr(0), val00.ptr(),0,false);
    }
    return &configCache;
}


void sQPrideDB2::QP_flushCache()
{
    for(idx si = 0; si < _svcByName.dim(); ++si) {
        idx realModified = 0;
        idx cacheModified = 0;
        sUsrObj * obj = _svcByName[si];
        if(obj){
            const sUsrObjPropsTree * objPropsTree = obj->propsTree();
            if( objPropsTree ) {
                cacheModified = objPropsTree->findIValue("modified");
            }
            if( cacheModified ) {
                realModified = obj->propGetI("modified");
            }
            if( cacheModified && realModified && (realModified == cacheModified) ) {
                continue;
            }
            reloadPropTree(0, obj->Id());
        }
    }
    configCache.empty();
}

void sQPrideDB2::reloadPropTree(const char * svcName, const sHiveId & objID){
    sUsrObj ** objPtr = 0;
    if(svcName){
       objPtr =  _svcByName.get(svcName);
    }
    if(objID && (!objPtr || !(*objPtr))){
        idx svc_id = objID.objId();
        objPtr = _svcById.get(&svc_id, sizeof(svc_id));
    }

    if(objPtr && *objPtr){
        const sUsrObjPropsTree * objPropsTree = (*objPtr)->propsTree(0,0,true);
        if( !objPropsTree ) {
            fprintf(stderr, "failed to force reload the tree for obj %s\n", (*objPtr)->Id().print());
        }
    }
}



idx sQPrideDB2::QP_serviceList(sStr * lst00, void * svcVecList)
{
    sVec<sUsrObj *> objList;
    idx svcCnt = QP_serviceList(&objList);
    if( svcCnt ) {
        sVec<sQPrideBase::Service> * svcvec = (sVec<sQPrideBase::Service> *) svcVecList;
        for(idx si = 0; si < svcCnt; ++si) {
            if( svcvec ) {
                sQPrideBase::Service * ptr = svcvec->add(1);
                _QP_populateSvc(ptr, objList[si]);
            }
            if( lst00 ) {
                if( objList[si] ) {
                    const sUsrObjPropsTree * objPropsTree = objList[si]->propsTree();
                    const char * name = 0;
                    if( objPropsTree ) {
                        name = objPropsTree->findValue("name");
                    }else{
                        name = objList[si]->propGet("name", 0);
                    }
                    if( name ) {
                        lst00->printf("%s", name);
                        lst00->add0(1);
                    }
                }
            }
        }
        if( lst00 ) {
            lst00->add0(2);
        }
    }
    return svcCnt;
}

idx sQPrideDB2::QP_serviceList(sVec<sUsrObj *> * svcObjList, const char * prop, const char * val)
{
    if( !getUser() ) {
#ifdef _DEBUG
        fprintf(stderr, "qpride user not get init\n");
#endif
        return 0;
    }
    sUsrObjRes objIdList;
    idx svcCnt = 0;
    getUser()->objs2("qpsvc", objIdList, 0, prop ? prop : 0, val ? val : 0);
    for(sUsrObjRes::IdIter it = objIdList.first(); objIdList.has(it); objIdList.next(it)) {
        sUsrObj * obj = QP_serviceGet(0, objIdList.id(it)->objId());
        if( obj ) {
            ++svcCnt;
            if( svcObjList ) {
                svcObjList->vadd(1, obj);
            }
        } else {
#ifdef _DEBUG
            fprintf(stderr, "objId %s not get properly\n", objIdList.id(it)->print());
#endif
        }
    }
    return svcCnt;
}

void sQPrideDB2::QP_getRegisteredIP(sVec<sStr> * ips, const char * equCmd)
{
    if( !getUser() ) {
#ifdef _DEBUG
        fprintf(stderr, "qpride user not get init\n");
#endif
        return;
    }

    if( strchr(equCmd, ';') ) {
        return;
    }

    const char * colon = strchr(equCmd, ':');
    idx hoursAgo = colon ? atoidx(colon + 1) : 0;

    idx cat_count = 0;
    sStr catlist00;
    for(const char * cat = equCmd; cat && *cat && cat < colon; ) {
        for(; isspace(*cat); cat++);

        const char * comma = strchr(cat, ',');
        if( comma > colon ) {
            comma = 0;
        }

        idx cat_len = comma ? comma - cat : sLen(cat);
        for(; cat_len > 0 && isspace(cat[cat_len - 1]); cat_len--);

        if( cat_len ) {
            catlist00.add(cat, cat_len);
        }

        cat_count++;
        cat = comma ? comma + 1 : 0;
    }
    if( cat_count ) {
        catlist00.add0(2);
    }

    sUsrObjRes objIdList;
    getUser()->objs2("qphost", objIdList);

    for(sUsrObjRes::IdIter it = objIdList.first(); objIdList.has(it); objIdList.next(it)) {
        std::auto_ptr<sUsrObj> obj(getUser()->objFactory(*objIdList.id(it)));
        if( obj.get() ) {
            const sUsrObjPropsTree * objPropsTree = obj->propsTree();
            if( objPropsTree && objPropsTree->findBoolValue("enabled") ) {
                if( catlist00.length() > 0 ) {
                    const char * category = objPropsTree->findValue("category");
                    if( category ) {
                        if( sString::compareChoice(category, catlist00.ptr(), 0, false, 0, true) == -1 ) {
                            continue;
                        }
                    } else {
                        continue;
                    }
                }
                if( hoursAgo > 0 )
                    if( objPropsTree->findIValue("modified") <= (time(0) - hoursAgo * 3600) )
                        continue;

                if( !hoursAgo ) {
                    if( objPropsTree->findIValue("modified") <= (time(0) - 3600) )
                        continue;
                }
                if( objPropsTree->findValue("name") && objPropsTree->findValue("ip4") ) {
                    sStr * curHost = ips->add();
                    if( curHost ) {
                        curHost->printf("%s", objPropsTree->findValue("name"));
                        curHost->add0();
                        curHost->printf("%s", objPropsTree->findValue("ip4"));
                    }
                }
            }
        }
    }
}

real sQPrideDB2::QP_getHostCapacity(const char * hostname)
{
    sUsrObjRes hostList;
    sStr nameBuf("^%s$", hostname);
    if( !getUser() ) {
#ifdef _DEBUG
        fprintf(stderr, "qpride user not get init\n");
#endif
    } else {
        getUser()->objs2("qphost", hostList, 0, "name", nameBuf);
    }
    std::auto_ptr<sUsrObj> obj(hostList.dim() == 1 ? getUser()->objFactory(*hostList.firstId()) : 0);
    return obj.get() ? obj->propGetR("capacity") : 0;
}

void sQPrideDB2::QP_registerHostIP(const char * sys)
{
    if( !getUser() ) {
#ifdef _DEBUG
        fprintf(stderr, "qpride user not get init\n");
#endif
        return;
    }

    char hostname[1024];
    char ip[INET_ADDRSTRLEN];

    hostname[0] = '\0';
    ip[0] = '\0';

#ifndef SLIB_WIN
    if( gethostname(hostname, sizeof(hostname)) == 0 ) {
        struct ifaddrs *ifas;
        if( getifaddrs(&ifas) == 0 ) {
            for(struct ifaddrs * ifa = ifas; ifa; ifa = ifa->ifa_next) {
                if( ifa->ifa_addr->sa_family == AF_INET ) {
                    struct in_addr* ptr = &((struct sockaddr_in*) ifa->ifa_addr)->sin_addr;
                    char ip1[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, ptr, ip1, sizeof(ip));
                    if( strcmp(ip1, "127.0.0.1") != 0 && (!ip[0] || strstr(ip1, "192.168.") == 0) ) {
                        strcpy(ip, ip1);
                    }
                }
            }
            freeifaddrs(ifas);
        }
    }
#else
    strncpy(hostname, "win-host", sizeof(hostname) - 1);
    strncpy(ip, "not implemented", sizeof(ip) - 1);
#endif

    if( hostname[0] ) {
        sUsrObjRes hostList;
        sStr nameBuf("^%s$", hostname);
        getUser()->objs2("qphost", hostList, 0, "name", nameBuf);
        std::auto_ptr<sUsrObj> obj;
        for(sUsrObjRes::IdIter it = hostList.first(); it < hostList.last(); hostList.next(it)) {
            obj.reset(getUser()->objFactory(*hostList.id(it)));
            obj->actDelete();
        }
        if( hostList.dim() == 0 ) {
            obj.reset(new sUsrObj(*getUser(), "qphost"));
            if( obj.get() ) {
                obj->propSet("name", hostname);
                if( !getUser()->allow4admins(obj->Id()) ) {
#ifdef _DEBUG
                    fprintf(stderr, "failed to allow 4 admins\n");
#endif
                } else {
#ifdef _DEBUG
                    fprintf(stderr, "cannot create new host object\n");
#endif
                }
                real capacity = 0;
                idx ncores = 0;
                idx mem_sz = 0;
#ifdef WIN32
#else
                ncores = sysconf(_SC_NPROCESSORS_ONLN);
                if( ncores > 0 ) {
                    const idx mem_pages = sysconf(_SC_PHYS_PAGES);
                    const idx page_sz = sysconf(_SC_PAGESIZE);
                    if( mem_pages > 0 && page_sz > 0 ) {
                        mem_sz = mem_pages * page_sz;
                        capacity = mem_sz / 1024.0 / 1024.0 / 1024.0 / ncores;
                        if( capacity - 4.0 > 0.1 ) {
                            capacity = ncores;
                        } else {
                            capacity = mem_sz / 4;
                        }
                    }
                } else {
                    ncores = 0;
                }
#endif
                obj->propSet("htype", sys);
                obj->propSetR("capacity", capacity);
                obj->propSetI("cores", ncores);
                obj->propSetI("memory", mem_sz);
            }
        } else {
            obj.reset(getUser()->objFactory(*hostList.lastId()));
        }
        if( obj.get() ) {
            obj->propSet("ip4", ip);
        }
    }
}

void sQPrideDB2::QP_reqCleanTbl(sVec<sStr> *tblfiles, idx req, const char *dataname)
{
    sVarSet res;
    sStr sql("SELECT dataName FROM QPData WHERE reqID = %" DEC " AND dataName LIKE ", req);
    protectString(sql, dataname);
    db->getTable(sql.ptr(), &res);

    sql.printf(0, "DELETE FROM QPData WHERE reqID = %" DEC " AND dataName LIKE ", req);
    protectString(sql, dataname);
    db->executeString(sql.ptr());

    if( tblfiles ) {
        for(idx i = 0; i < res.rows; ++i) {
            sStr *onefile;
            onefile = tblfiles->add();
            onefile->printf("%s", res.val(i, 0));
        }
    }
}

idx sQPrideDB2::QP_workRegisterTime(const char * svc, const char * params, idx amount, idx time)
{
    if( !svc )
        return -1;
    if( amount <= 0 )
        return 0;
    if( time <= 0 )
        return 0;

    sVarSet res;

    sStr sql("SELECT amountOfWork, timeSeconds FROM QPPerform WHERE svcName = ");
    protectString(sql, svc);
    if( params ) {
        sql.addString(" AND paramset = ");
        protectString(sql, params);
    }
    db->getTable(sql.ptr(), &res);
    sql.cut(0);

    if( res.rows == 1 ) {
        idx amountOfWork = res.ival(0, 0);
        idx timeSeconds = res.ival(0, 1);

        amountOfWork += amount;
        timeSeconds += time;

        sql.printf("UPDATE QPPerform SET amountOfWork='%" DEC "', timeSeconds='%" DEC "' WHERE svcName = ", amountOfWork, timeSeconds);
        protectString(sql, svc);
        if( params ) {
            sql.addString(" AND paramset = ");
            protectString(sql, params);
        }

        db->executeString(sql.ptr());
        return timeSeconds;
    } else if( res.rows == 0 ) {
        if( params ) {
            sql.addString("INSERT INTO QPPerform (svcName, amountOfWork, timeSeconds, paramset) VALUES (");
            protectString(sql, svc);
            sql.printf(", %" DEC ", %" DEC ", ", amount, time);
            protectString(sql, params);
            sql.addString(")");
            db->executeString(sql.ptr());
        } else {
            sql.printf("INSERT INTO QPPerform (svcName, amountOfWork, timeSeconds) VALUES (");
            protectString(sql, svc);
            sql.printf(", %" DEC ", %" DEC ")", amount, time);
            db->executeString(sql.ptr());
        }
        return time;
    } else {
        return -1;
    }
}

idx sQPrideDB2::QP_workEstimateTime(const char * svc, const char * params, idx amount)
{
    if( !svc )
        return 0;
    if( amount <= 0 )
        return 0;

    sVarSet res;

    sStr sql("SELECT amountOfWork, timeSeconds FROM QPPerform WHERE svcName = ");
    protectString(sql, svc);
    if( params ) {
        sql.addString(" AND paramset = ");
        protectString(sql, params);
    }

    db->getTable(sql.ptr(), &res);

    if( res.rows > 1 )
        return -1;
    if( res.rows == 1 ) {
        idx amountOfWork = res.ival(0, 0, -1);
        if( amountOfWork <= 0 ) {
            return -1;
        }

        idx timeSeconds = res.ival(0, 1, -1);
        if( timeSeconds <= 0 ) {
            return -1;
        }

        idx estimated = (idx) (timeSeconds * amount / amountOfWork);
        return estimated;
    }

    return -1;
}

void sQPrideDB2::QP_servicePurgeOld(sVec<idx> * reqList, const char * service, idx limit, bool no_delete)
{
    sVarSet res;
    sUsrObj * obj = service ? QP_serviceGet(service) : 0;
    if( !service || obj ) {
#ifdef NDEBUG
#undef NDEBUG
        assert(false);
#define NDEBUG
#endif
        sSql::sqlProc * sp = db->Proc("sp_svc_purge_old");
        if( sp ) {
            sUsrObj * qm = QP_serviceGet("qm");
            idx objCleanUpDays=0;
            idx qmCleanUpDays = 0;
            if(obj){
                const sUsrObjPropsTree * objPropsTree = obj->propsTree();
                if( objPropsTree ) {
                    objCleanUpDays = objPropsTree->findIValue("cleanUpDays");
                }else{
                    objCleanUpDays = obj->propGetI("cleanUpDays");
                }
            }
            if(qm){
                const sUsrObjPropsTree * qmPropsTree = qm->propsTree();
                if( qmPropsTree ) {
                    qmCleanUpDays = qmPropsTree->findIValue("cleanUpDays");
                }else{
                    qmCleanUpDays = qm->propGetI("cleanUpDays");
                }
            }
            sp->Add(obj ? obj->Id().objId() : 0).Add(objCleanUpDays).Add(qmCleanUpDays).Add(limit).Add(
#if _DEBUG
                true
#else
                no_delete
#endif
                            );
            sp->getTable(&res);
            delete sp;
        }
        for(idx i = 0; i < res.rows; ++i) {
            reqList->vadd(1, res.ival(i, 0));
        }
    }
}

void sQPrideDB2::QP_servicePath2Clean(sVarSet & res)
{
    sVec<sUsrObj *> svcs;
    QP_serviceList(&svcs, "QPCfg_cleanUpDays,QPCfg_cleanUpMasks");
    for(idx i = 0; i < svcs.dim(); ++i) {
        const sUsrObjPropsTree * objPropsTree = svcs[i]->propsTree();

        sUsrObjPropsNode * arrayNode = objPropsTree ? (sUsrObjPropsNode *) objPropsTree->find("QPCfg") : 0;
        if( arrayNode ) {
            for(sUsrObjPropsNode * rowNode = arrayNode->firstChild(); rowNode; rowNode = rowNode->nextSibling()) {
                sUsrObjPropsNode * valNode = rowNode->firstChild("QPCfg_val");
                sUsrObjPropsNode * daysNode = rowNode->firstChild("QPCfg_cleanUpDays");
                sUsrObjPropsNode * masksNode = rowNode->firstChild("QPCfg_cleanUpMasks");
                if( valNode ) {
                    res.addRow().addCol(valNode->value(0)).addCol(daysNode ? daysNode->ivalue(0) : 0).addCol(masksNode ? masksNode->value(0) : "");
                } else {
#ifdef _DEBUG
                    fprintf(stderr, "node of props tree not get properly\n");
#endif
                }
            }
        }

    }
}

idx sQPrideDB2::QP_dbHasLiveConnection(void)
{
    if( db->status != sSql::eConnected )
        return 0;
    return QP_serviceID("qm") ? 1 : 0;

}
void sQPrideDB2::QP_dbDisconnect(void)
{
    db->disconnect();
}
idx sQPrideDB2::QP_dbReconnect(void)
{
    return db->realConnect();
}

void sQPrideDB2::QP_jobRegisterAlive(idx job)
{
    db->execute("UPDATE QPJob SET aliveTm=NOW() WHERE jobID=%" DEC, job);
}
idx sQPrideDB2::QP_jobSetStatus(idx job, idx jobstat)
{
    db->execute("UPDATE QPJob SET stat=%" DEC " WHERE jobID=%" DEC, jobstat, job);
    return jobstat;
}

idx sQPrideDB2::QP_jobSetStatus(sVec<idx> * jobs, idx jobstat)
{
    return sQPrideDB::_QP_SetIdxVar(db, jobs, jobstat, "QPJob", "jobID", "stat", 0);
}

idx sQPrideDB2::QP_jobSetAction(idx job, idx jobact)
{
    return sQPrideDB::_QP_SetIdxVar(db, job, jobact, "QPJob", "jobID", "act", ", actTm = NOW()");
}

idx sQPrideDB2::QP_jobSetAction(sVec<idx> * jobs, idx jobact)
{
    return sQPrideDB::_QP_SetIdxVar(db, jobs, jobact, "QPJob", "jobID", "act", ", actTm = NOW()");
}

idx sQPrideDB2::QP_jobRegister(const char * serviceName, const char * hostName, idx pid, idx inParallel)
{
    idx job = 0;
    sUsrObj * obj = QP_serviceGet(serviceName);
    if( obj ) {
        sSql::sqlProc * sp = db->Proc("sp_job_register");
        if( sp ) {
            sp->Add(obj->Id().objId()).Add(hostName).Add(pid).Add(inParallel);
            job = sp->ivalue(job);
            delete sp;
        }
    }
    return job;
}

idx sQPrideDB2::QP_jobSetMem(idx job, idx curMemSize, idx maxMemSize)
{
    db->execute("UPDATE QPJob SET mem=%" DEC ", maxmem=%" DEC " WHERE jobID=%" DEC, curMemSize, maxMemSize, job);
    return curMemSize;
}

idx sQPrideDB2::QP_jobGetAction(idx job)
{
    return db->ivalue(0, "SELECT act FROM QPJob WHERE jobID = %" DEC, job);
}

idx sQPrideDB2::QP_jobSetReq(idx job, idx req)
{
    return db->uvalue(0, "UPDATE QPJob SET reqID = %" DEC " WHERE jobID = %" DEC "; UPDATE QPReq SET jobID = %" DEC " WHERE reqID = %" DEC "; SELECT 1;", req, job, job, req);
}


idx sQPrideDB2::QP_jobGet(idx jobID, sQPrideBase::Job * jobs, idx jobCnt, const char * _wherecls)
{
    sQPrideBase::Job * job = jobs;

    sStr sql;
    sql.printf("SELECT "
        "QPJob.jobID, QPJob.svcID, QPJob.pid, QPJob.reqID"
        ",QPJob.stat, QPJob.act, QPJob.cntGrabbed, QPJob.inParallel"
        ",UNIX_TIMESTAMP(QPJob.cdate), UNIX_TIMESTAMP(QPJob.aliveTm), UNIX_TIMESTAMP(QPJob.actTm), UNIX_TIMESTAMP(QPJob.psTm)"
        ", QPJob.mem, QPJob.maxmem, QPJob.killCnt, QPJob.hostName"
        " FROM QPJob ");
    if( _wherecls ) {
        sql.printf(_wherecls);
    } else if( jobID ) {
        sql.printf("WHERE QPJob.jobID = %" DEC, jobID);
    }

    sVarSet res;
    db->getTable(&res, sql.ptr());
    idx jobId = 0;
    for(idx ir = 0; ir < res.rows && ir < jobCnt; ++ir) {
        idx o = 0;
        job->jobID = res.ival(ir, o++);
        jobId = job->jobID;
        job->svcID = res.ival(ir, o++);
        job->pid = res.ival(ir, o++);
        job->reqID = res.ival(ir, o++);
        job->stat = res.ival(ir, o++);
        job->act = res.ival(ir, o++);
        job->cntGrabbed = res.ival(ir, o++);
        job->inParallel = res.ival(ir, o++);
        job->cdate = res.ival(ir, o++);
        job->aliveTm = res.ival(ir, o++);
        job->actTm = res.ival(ir, o++);
        job->psTm = res.ival(ir, o++);
        job->mem = res.ival(ir, o++);
        job->maxmem = res.ival(ir, o++);
        job->killCnt = res.ival(ir, o++);
        strncpy(job->hostName, res(ir, o++), sizeof(job->hostName) - 1);
        ++job;
    }
    for(idx ir = res.rows; ir < jobCnt; ir++) {
        sSet<sQPrideBase::Job>(job++);
    }

    return jobId;
}

bool sQPrideDB2::QP_setLog(idx req, idx job, idx level, const char * txt)
{
    sSql::sqlProc * p = db->Proc("sp_log_set");
    if( p ) {
        p->Add(req).Add(job).Add(level).Add(txt);
        bool ret = p->execute() > 0;
        delete p;
        return ret;
    }
    return false;
}

idx sQPrideDB2::QP_getLog(idx req, bool isGrp, idx job, idx level, sVarSet & log)
{
    sSql::sqlProc * p = db->Proc("sp_log_get");
    if( p ) {
        p->Add(req).Add(isGrp).Add(job).Add(level);
        const idx sz = log.rows;
        p->getTable(&log);
        delete p;
        return log.rows - sz;
    }
    return 0;
}



idx sQPrideDB2::QP_sysPeekOnHost(void * srvl, const char * hostname)
{
    if( !srvl || !hostname ) {
        return 0;
    }
    sVarSet res;
    db->getTable(&res, "SELECT svcID, COUNT(reqId), SUM(IF(act=6,1,0)) FROM QPReq WHERE stat = 1 AND act IN (2, 6) AND scheduleGrab <= NOW() GROUP BY svcID");

    sVec<sQPrideBase::Service> * srvlst = (sVec<sQPrideBase::Service> *) srvl;
    sVec<sUsrObj *> objList;
    QP_serviceList(&objList);
    for(idx si = 0; si < objList.dim(); ++si) {
        const char * hosts = 0;
        const sUsrObjPropsTree * objPropsTree = objList[si]->propsTree();
        if( objPropsTree ) {
            hosts = objPropsTree->findValue("hosts");
        }else{
            hosts = objList[si]->propGet("hosts", 0);
        }
        if( !hostNameListMatch(hosts, hostname) ) {
            continue;
        }
        sQPrideBase::Service * ptr = srvlst->add(1);
        if( ptr ) {
            _QP_populateSvc(ptr, objList[si]);
            for(idx ir = 0; ir < res.rows; ++ir) {
                idx svcID = res.ival(ir, 0, -1);
                if( svcID == ptr->svcID ) {
                    ptr->hasReqToGrab = res.ival(ir, 1, 0);
                    ptr->hasReqToGrabForSplitting = res.ival(ir, 2, 0);
                    break;
                }
            }
        }
    }
    return srvlst->dim();
}

idx sQPrideDB2::QP_sysPeekReqOrder(idx req, const char * serviceName, idx * pRunning)
{
    idx order = 0;
    sUsrObj * obj = serviceName ? QP_serviceGet(serviceName) : 0;
    if( !serviceName || obj ) {
        sSql::sqlProc * sp = db->Proc("sp_req_peek_order");
        if( sp ) {
            sVarSet res;
            sp->Add(obj ? obj->Id().objId() : 0).Add(req).Add(1LL).Add(2LL);
            sp->getTable(&res);
            order = res.ival(0,0);
            if(pRunning){
                *pRunning=res.ival(0,1);
            }
            delete sp;
        }
    }
    return order;
}
idx sQPrideDB2::QP_sysGetKnockoutJobs(sVec <sQPrideBase::Job> * jobs, sVec<idx> * svcIDs)
{
    sStr sql;
    sql.printf("SELECT COUNT(jobID) FROM QPJob");
    idx tail = sql.length();
#ifdef NDEBUG
#undef NDEBUG
    assert(false);
#define NDEBUG
#endif
    sql.printf(
        ", (SELECT DISTINCT objID AS svcID, value AS knockoutSec FROM UPObjField F"
            " WHERE name = 'knockoutSec' AND (value!=0 AND value IS NOT NULL) AND objId IN (SELECT objID FROM UPObj WHERE objTypeID = (SELECT type_id FROM <DELETED> WHERE name ='qpsvc'))) svc_knoc WHERE svc_knoc.svcID=QPJob.svcID AND QPJob.stat=%i AND UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-UNIX_TIMESTAMP(QPJob.aliveTm)>svc_knoc.knockoutSec",
        sQPrideBase::eQPJobStatus_Running);
    if( svcIDs ) {
        sql.printf(" AND svc_knoc.svcID IN (");
        sString::printfIVec(&sql, svcIDs);
        sql.printf(")");
    }
    idx jobCnt = db->ivalue(sql.ptr(), 0);
    sQPrideBase::Job * js = jobs->add(jobCnt);
    return sQPrideDB2::QP_jobGet(0, js, jobCnt, sql.ptr(tail));
}
idx sQPrideDB2::QP_sysGetImpoliteJobs(sVec <sQPrideBase::Job> * jobs, sVec<idx> * svcIDs)
{
    sStr sql;
    sql.printf("SELECT COUNT(jobID) FROM QPJob");
    idx tail = sql.length();
#ifdef NDEBUG
#undef NDEBUG
    assert(false);
#define NDEBUG
#endif
    sql.printf(
        ", (SELECT DISTINCT objID AS svcID, value AS politeExitTimeoutSec FROM UPObjField F"
            " WHERE name = 'politeExitTimeoutSec' AND (value!=0 AND value IS NOT NULL) AND objId IN (SELECT objID FROM UPObj WHERE objTypeID = (SELECT type_id FROM <DELETED> WHERE name ='qpsvc'))) svc_pol WHERE svc_pol.svcID=QPJob.svcID AND QPJob.stat=%i AND QPJob.act=%i AND UNIX_TIMESTAMP(CURRENT_TIMESTAMP)-UNIX_TIMESTAMP(QPJob.actTm)>svc_pol.politeExitTimeoutSec",
        sQPrideBase::eQPJobStatus_Running, sQPrideBase::eQPJobAction_Kill);
    if( svcIDs ) {
        sql.printf(" AND svc_pol.svcID in (");
        sString::printfIVec(&sql, svcIDs);
        sql.printf(")");
    }
    idx jobCnt = db->ivalue(sql.ptr(), 0);
    sQPrideBase::Job * js = jobs->add(jobCnt);
    return sQPrideDB2::QP_jobGet(0, js, jobCnt, sql.ptr(tail));
}

idx sQPrideDB2::QP_sysJobsGetList(sVec <sQPrideBase::Job> * jobs, idx stat, idx act, const char * hostname)
{
    sStr sql("SELECT COUNT(jobID) FROM QPJob");
    idx tail = sql.length();
    sql.printf(" WHERE stat=%" DEC, stat);
    if( act != sQPrideBase::eQPJobAction_Any ) {
        sql.printf(" AND act=%" DEC, act);
    }
    if( hostname ) {
        sql.addString(" AND hostName=");
        protectString(sql, hostname);
    }
    idx jobCnt = db->ivalue(sql.ptr(), 0);
    sQPrideBase::Job * js = jobs->add(jobCnt);
    return sQPrideDB2::QP_jobGet(0, js, jobCnt, sql.ptr(tail));
}
idx sQPrideDB2::QP_sysRecoverRequests(sVec<idx> *, sVec<idx> *)
{
    db->executeString("CALL sp_reqs_recover()");
    return 0;
}

idx sQPrideDB2::QP_sysCapacityNeed(idx * capacity_total)
{
    idx need = 0;
    sSql::sqlProc * sp = db->Proc("sp_sys_capacity");
    if( sp ) {
        if( capacity_total ) {
            *capacity_total = 0;
        }
        sVarSet tbl;
        sp->getTable(&tbl);
        for(idx r = 0; r < tbl.rows; ++r) {
            if( capacity_total && tbl.ival(r, 0) > *capacity_total) {
                *capacity_total = tbl.ival(r, 0);
            }
            if( tbl.ival(r, 1) <= sQPrideBase::eQPReqStatus_Running) {
                need += tbl.ival(r, 2);
            }
        }
        delete sp;
    }
    return sp ? need : -1;
}
