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
#include <qlib/QPrideBase.hpp>
#ifndef SLIB_WIN
#include <arpa/inet.h>
#include <ifaddrs.h>
#endif

using namespace slib;

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

//#define protect(_v_buf,_v_val) sString::encodeBase64(&(_v_buf),(const char *)(_v_val),sLen(_v_val),true);(_v_buf).add0();

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Constructors
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

sQPrideDB::sQPrideDB(sSql * lsql)
{
    db=lsql;dbdel=0;
}


sSql::gSettings __sQPrideDB_gDefaultSettings={HIVE_DB,HIVE_DB_HOST,HIVE_DB_USER,HIVE_DB_PWD,1,50,"random",0};

sQPrideDB::sQPrideDB(const char *defline)
{
    db=new sSql;
    db->gSet=__sQPrideDB_gDefaultSettings;

    if(db->connect(defline)==sSql::eDisconnected) {
        //printf("FATAL: Cannot connect to QPride\n");
    }
    dbdel=db;
    return ;
}

sQPrideDB::~sQPrideDB()
{
    // causes call for object which might be already deleted
    // and if object is owned ~sSql() will call disconnect anyway
    // db->disconnect();
    if(dbdel)delete dbdel;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Utilities
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//static
char * sQPrideDB::_QP_whereChoice(sSql * db, sStr * str, const char * choice00, bool single, bool quoted, const char * wherepar )
{
    if(!choice00) {
        return str->ptr();
    }

    str->addString(" WHERE ");
    protectName(*str, wherepar);

    if( strchr( choice00,'%' ) ) {
        str->addString(" LIKE ");
        protectOptional(*str, choice00, quoted);
    } else if(single==true) {
        str->addString(" = ");
        protectOptional(*str, choice00, quoted);
    } else {
        str->addString(" IN (");
        for(const char * choice = choice00; choice && *choice; choice = sString::next00(choice)) {
            protectOptional(*str, choice, quoted);
        }
        str->addString(")");
    }
    return str->ptr();
}

//static
idx sQPrideDB::_QP_SetIdxVar(sSql * db, idx req, idx val, const char * tblname, const char * idxname, const char * vname, const char * extraset)
{
    sStr sql;
    sql.addString("UPDATE ");
    protectName(sql, tblname);
    sql.addString(" SET ");
    protectName(sql, vname);
    sql.printf(" = %" DEC, val);
    if( extraset ) {
        sql.addString(extraset);
    }
    sql.addString(" WHERE ");
    protectName(sql, idxname);
    if( req < 0 ) {
        // group operation
        sql.printf(" IN (SELECT reqID FROM QPGrp WHERE grpID = %" DEC ")", -req);
    } else {
        sql.printf(" = %" DEC, req);
    }

    db->getTable(sql.ptr(), 0, 0);
    return val;
}

//static
idx sQPrideDB::_QP_GetIdxVar(sSql * db, idx req, const char * tblname,const char * idxname, const char * vname)
{
    idx ret = 0;
    sStr sql;
    sql.addString("SELECT ");
    protectName(sql, vname);
    sql.addString(" FROM ");
    protectName(sql, tblname);
    sql.addString(" WHERE ");
    protectName(sql, idxname);
    sql.printf(" = %" DEC, req);

    db->sscanfTable(sql.ptr(), "%" DEC, &ret);
    return ret;
}

//static
idx sQPrideDB::_QP_SetIdxVar(sSql * db, sVec<idx> * reqs, idx val, const char * tblname, const char * idxname, const char * vname, const char * extraset)
{
    sStr sql;
    sql.addString("UPDATE ");
    protectName(sql, tblname);
    sql.addString(" SET ");
    protectName(sql, vname);
    sql.printf(" = %" DEC, val);
    if( extraset ) {
        sql.addString(extraset);
    }
    sql.addString(" WHERE ");
    sSql::exprInList(sql, idxname, *reqs); // shorter SQL statement
    /*
    protectName(sql, idxname);
    sql.addString(" IN (");
    sString::printfIVec(&sql, reqs);
    sql.printf(")");
    */
    db->getTable(sql.ptr(), 0, 0);
    return val;
}

//static
idx sQPrideDB::_QP_GetIdxVar(sSql * db, sVec<idx> * reqs, sVec<idx> * vals, const char * tblname, const char * idxname, const char * vname)
{
    sStr sql;
    sql.addString("SELECT ");
    protectName(sql, vname);
    sql.addString(" FROM ");
    protectName(sql, tblname);
    sql.addString(" WHERE ");
    sSql::exprInList(sql, idxname, *reqs); // shorter SQL statement
    /*
    protectName(sql, idxname);
    sql.addString(" IN (");
    sString::printfIVec(&sql, reqs);
    sql.printf(")");
    */
    sVarSet res;
    db->getTable(sql.ptr(), &res);
    idx val = 0;
    for(idx i = 0; i < res.rows; ++i) {
        sscanf(res(i, 0), "%" DEC, &val);
        vals->vadd(1, val);
    }
    return vals->dim();
}


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Configuration
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

static sVar g_QPCfg;

void sQPrideDB::QP_flushCache()
{
    g_QPCfg.empty();
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

char * sQPrideDB::QP_configGet(sStr * vals00, const char * pars00, bool single)
{
    if( !g_QPCfg.dim() ) {
        sVarSet res;
        db->getTable("SELECT par, val FROM QPCfg", &res);
        for(idx i = 0; i < res.rows; ++i) {
            idx psz = 0, vsz = 0;
            const char * par = res.val(i, 0, &psz);
            const char * val = res.val(i, 1, &vsz);
            if( par && psz && val ) {
                g_QPCfg.inp(par, val, vsz);
            }
        }
    }
    const idx pos = vals00->length();
    if( pars00 ) {
        for(const char * p = pars00; p; p = single ? 0 : sString::next00(p)) {
            configGetVal(*vals00, g_QPCfg, p, -1);
        }
    } else {
        for(idx ipar = 0; ipar < (single ? 1 : g_QPCfg.dim()); ipar++) {
            configGetVal(*vals00, g_QPCfg, 0, ipar);
        }
    }
    if( vals00->length() ) {
        vals00->add0(2);
    }
    return vals00->ptr(pos);
}

bool sQPrideDB::QP_configSet(const char * par, const char * val)
{
    sSql::sqlProc sql(*db, "cfgIns");
    sql.Add(par).Add(val);

    if( sql.execute() ) {
        g_QPCfg.inp(par, val);
        return true;
    }
    return false;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/   Submission
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx sQPrideDB::QP_reqSubmit(const char * serviceName, idx subip, idx priority, idx usrid)
{
    sSql::sqlProc sql(*db, "reqSubmit");
    sql.Add(serviceName).Add((idx)0).Add(priority).Add(subip).Add(usrid);
    return sql.ivalue(0);
}

idx sQPrideDB::QP_reqGrab(const char * service, idx job, idx inBundle, idx status, idx action)
{
    sSql::sqlProc sql(*db, "reqGrab");
    sql.Add(service).Add(job).Add(status).Add(action).Add(inBundle);
    return sql.ivalue(0);
}

idx sQPrideDB::QP_reqReSubmit(idx * req, idx cnt, idx delaySeconds /* = 0 */)
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

idx sQPrideDB::QP_requestGetForGrp(idx grp, void * RList, const char * serviceName)
{
    sVec<sQPrideBase::Request> * rList = (sVec<sQPrideBase::Request> *) RList;
    idx count;
    sStr sql("SELECT COUNT(QPReq.reqID) FROM QPGrp JOIN QPReq USING (reqID)");
    if( serviceName ) {
        sql.addString(" JOIN QPSvc USING (svcID) WHERE name = ");
        protectString(sql, serviceName);
        sql.printf(" AND grpID = %" DEC, grp);
    } else {
        sql.printf(" WHERE grpID = %" DEC, grp);
    }
    db->sscanfTable(sql.ptr(), "%" DEC, &count, 1, sizeof(count));
    if( count > 0 ) {
        rList->add(count);
    } else { // this could be a single reqID without associated grpID
        sStr sql2("SELECT COUNT(QPReq.reqID) FROM QPReq WHERE reqID = %" DEC, grp);
        db->sscanfTable(sql2.ptr(), "%" DEC, &count, 1, sizeof(count));
        if( count > 0 ) {
            rList->add(count);
            return sQPrideDB::QP_requestGet(grp, rList->ptr(), false);
        } else {
            return 0; // not even a single request with reqID=grp and without grpID exists in DB
        }
    }
    return sQPrideDB::QP_requestGet(grp, rList->ptr(), true, serviceName);
}

idx sQPrideDB::QP_getReqByUserKey(idx userKey, const char * serviceName)
{
    sStr sql("SELECT reqID FROM QPReq WHERE userKey = %" DEC, userKey);
    if( serviceName ) {
        sql.addString(" AND svcID IN (SELECT svcID FROM QPSvc WHERE name = ");
        protectString(sql, serviceName);
        sql.addString(")");
    }

    idx req=0;
    db->sscanfTable(sql.ptr(), "%" DEC, &req, 1, sizeof(req));
    return req;
}

idx sQPrideDB::QP_requestGet(idx req , void * R, bool isGrp, const char * serviceName)
{
    sVarSet res;
    sQPrideBase::Request * r = (sQPrideBase::Request *)R;

    sStr sql("SELECT QPReq.reqID, QPReq.svcID, jobID, userID, subIp, cgiIp, stat, act, takenCnt,"
            "QPReq.priority, inParallel, progress, progress100, userKey,"
            "UNIX_TIMESTAMP(takenTm), UNIX_TIMESTAMP(QPReq.cdate), UNIX_TIMESTAMP(actTm), UNIX_TIMESTAMP(aliveTm),"
            "UNIX_TIMESTAMP(doneTm), UNIX_TIMESTAMP(purgeTm), QPSvc.title, UNIX_TIMESTAMP(QPReq.scheduleGrab)");

    if( isGrp ) {
        sql.printf(", QPGrp.grpID FROM QPGrp JOIN QPReq USING (reqID) JOIN QPSvc USING (svcID) WHERE grpID = %" DEC, req);
        if (serviceName) {
            sql.addString(" AND name = ");
            protectString(sql, serviceName);
        }
        sql.printf(" ORDER BY QPReq.reqID");
    } else {
        sql.printf(" FROM QPReq JOIN QPSvc USING (svcID) WHERE reqID = %" DEC, req);
    }

    db->getTable(sql.ptr(), &res);

    //if(res.rows<1)return req;
    for (idx i=0; i<res.rows; ++i ){
        idx o=0;
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
        strncpy(r->svcName, res.val(i, o++), sizeof(r->svcName) - 1);
        r->scheduleGrab = res.ival(i, o++);
        if( isGrp ) {
            r->grpID = res.ival(i, o++);
        } else {
            r->grpID = 0;
        }
        ++r;
    }
    return res.rows ? req : 0 ;
}


idx sQPrideDB::QP_reqSetProgress(idx req, idx progress, idx progress100)
{
    sStr sql("UPDATE QPReq SET progress = IF(%" DEC " >= 0, %" DEC ", progress)", progress, progress);
    if( progress100 >= 0 && progress100 <= 100 ) {
        sql.printf(", progress100 = %" DEC, progress100);
    }
    sql.printf(" WHERE reqID = %" DEC, req);
    db->executeString(sql);
    return progress;
}

idx sQPrideDB::QP_reqSetCgiIP(idx req, idx val)
{
    return _QP_SetIdxVar(db, req, val, "QPReq", "reqID", "cgiIp");
}
idx sQPrideDB::QP_reqSetSubIP(idx req, idx val){
    return _QP_SetIdxVar(db, req, val, "QPReq", "reqID", "subIp");
}
idx sQPrideDB::QP_reqSetAction(idx req, idx val){
    return _QP_SetIdxVar(db, req, val, "QPReq", "reqID", "act", ",actTm=NOW()");
}
idx sQPrideDB::QP_reqSetAction(sVec < idx > * reqs, idx val){
    return _QP_SetIdxVar(db, reqs,val,"QPReq","reqID","act",",actTm=NOW()");
}
idx sQPrideDB::QP_reqGetAction(idx req) {
    return _QP_GetIdxVar(db, req,"QPReq","reqID","act");
}

idx sQPrideDB::QP_reqGetAction(sVec < idx > * reqs, sVec <idx > * vals){
    return _QP_GetIdxVar(db, reqs,vals,"QPReq","reqID","act");
}

idx sQPrideDB::QP_reqGetUser(idx req) {
    return _QP_GetIdxVar(db, req,"QPReq","reqID","userID");
}

void sQPrideDB::QP_reqRegisterAlive(idx reqID)
{
    db->execute("UPDATE QPReq SET aliveTm = NOW() WHERE reqID = %" DEC, reqID);
}

/*
idx sQPrideDB::QP_reqGetObject(idx req) {
    return _QP_GetIdxVar(req,"QPReq","objID","userID");
}
idx sQPrideDB::QP_reqSetObject(idx req, idx val){
    return _QP_SetIdxVar(req,val,"QPReq","reqID","objID",0);
}
*/



idx sQPrideDB::QP_reqSetStatus(idx req, idx val)
{
    const char * more=(val>=sQPrideBase::eQPReqStatus_Done) ? ", doneTm = NOW()" : ((val==sQPrideBase::eQPReqStatus_Processing)? ", takenTm = NOW()" : 0);
    return _QP_SetIdxVar(db, req, val, "QPReq", "reqID", "stat", more);
}
idx sQPrideDB::QP_reqSetStatus(sVec < idx > * reqs, idx val)
{
    const char * more=(val>=sQPrideBase::eQPReqStatus_Done) ? ", doneTm = NOW()" : ((val==sQPrideBase::eQPReqStatus_Processing)? ", takenTm = NOW()" : 0);
    return _QP_SetIdxVar(db, reqs, val, "QPReq", "reqID", "stat", more);
}
idx sQPrideDB::QP_reqGetStatus(idx req)
{
    return _QP_GetIdxVar(db, req, "QPReq", "reqID", "stat");
}
idx sQPrideDB::QP_reqGetStatus(sVec < idx > * reqs, sVec <idx > * vals)
{
    return _QP_GetIdxVar(db, reqs, vals, "QPReq", "reqID", "stat");
}

idx sQPrideDB::QP_reqSetUserKey(idx req, idx val)
{
    return _QP_SetIdxVar(db, req, val, "QPReq", "reqID", "userKey", ", actTm = NOW()");
}
idx sQPrideDB::QP_reqSetUserKey(sVec < idx > * reqs, idx val)
{
    return _QP_SetIdxVar(db, reqs, val, "QPReq", "reqID", "userKey", ", actTm = NOW()");
}
idx sQPrideDB::QP_reqGetUserKey(idx req)
{
    return _QP_GetIdxVar(db, req, "QPReq", "reqID", "userKey");
}

idx sQPrideDB::QP_purgeReq(sVec<idx> * recVec, idx stat)
{
    return _QP_SetIdxVar(db, recVec, stat, "QPReq", "reqID", "stat", ", purgeTm = NOW()");
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Locks
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

bool sQPrideDB::QP_reqLock(idx req, const char * key, idx * preq_locked_by, idx max_lifetime, bool force)
{
    sVarSet res;
    idx req_locked_by = 0;
    bool ret = false;

    sSql::sqlProc sql(*db, "sp_req_lock");
    sql.Add(req).Add(key).Add(max_lifetime).Add((idx)force).Add((idx)sQPrideBase::eQPReqStatus_Done);
    if( sql.getTable(&res) && res.rows == 1 ) {
        req_locked_by = res.ival(0, res.colId("reqID"));
        ret = (req_locked_by == req);
    }
    if( preq_locked_by ) {
        *preq_locked_by = req_locked_by;
    }
    return ret;
}

bool sQPrideDB::QP_reqUnlock(idx req, const char * key, bool force)
{
    sSql::sqlProc sql(*db, "sp_req_unlock");
    sql.Add(req).Add(key).Add(force);
    return sql.ivalue(0);
}

idx sQPrideDB::QP_reqCheckLock(const char * key)
{
    sVarSet res;
    sSql::sqlProc sql(*db, "sp_req_lock");
    sql.Add((idx)0).Add(key).Add((idx)0).Add((idx)-1).Add((idx)sQPrideBase::eQPReqStatus_Done);
    sql.getTable(&res);
    return res.ival(0, res.colId("reqID"));
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Groups
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx sQPrideDB::QP_grpSubmit(const char * serviceName, idx subip, idx priority, idx numSubReqs, idx usrid, idx previousGrpSubmitCounter)
{
    sSql::sqlProc sql(*db, "grpSubmit");
    sql.Add(serviceName).Add(previousGrpSubmitCounter).Add(priority).Add(subip).Add(numSubReqs).Add(usrid);
    return sql.ivalue(0);
}

idx sQPrideDB::QP_grpAssignReqID(idx grp, idx req, idx jobIDSerial)
{
    sSql::sqlProc sql(*db, "grpIns");
    sql.Add(grp).Add(req).Add(jobIDSerial);
    return sql.ivalue(grp);
}

idx sQPrideDB::QP_grp2Req(idx grp, sVec< idx > * reqs, const char * svc, idx masterGroup)
{
    sStr sql;
    sVarSet res;
    idx v;
    sStr smaster;
    if( masterGroup ) {
        smaster.printf(" AND QPGrp.masterGrpID = %" DEC, masterGroup < 0 ? grp : masterGroup);
    }
    if( svc && *svc ) {
        sql.printf("SELECT QPGrp.reqID FROM QPGrp, QPReq WHERE QPReq.reqID=QPGrp.reqID %s AND QPGrp.grpID = %" DEC " AND svcID IN (SELECT svcID FROM QPSvc WHERE name", smaster ? smaster.ptr() : "", grp);
        if( strchr(svc, '%') ) {
            sql.addString(" LIKE ");
        } else {
            sql.addString(" = ");
        }
        protectString(sql, svc);
        sql.addString(") ORDER BY QPGrp.jobIDCollect, QPGrp.reqID");
             /*sql.printf("select QPGrp.reqID from QPGrp, QPReq where QPReq.reqID=QPGrp.reqID and QPGrp.grpID=%" DEC " and svcID in  ( select svcID from QPSvc  where name like ",grp);
        for( const char * p=svc; p; p=sString::next00(p)){
            if(p!=svc)sql.printf(" or");
            sql.printf("'%%%s%%' ",p);
        }
        sql.printf(" ) order by QPGrp.jobIDCollect, QPGrp.reqID ");
        */
    } else {
        sql.printf("SELECT reqID FROM QPGrp WHERE grpID = %" DEC " %s ORDER BY jobIDCollect, reqID",grp,smaster ? smaster.ptr() : "" );
    }
    db->getTable(sql.ptr(), &res);
    for(idx i=0; i<res.rows; ++i) {
        if( sscanf(res(i,0),"%" DEC,&v)!=1)break;
        reqs->vadd(1,v);
    }
    return reqs->dim() ? (*reqs)[0] : grp;
}

idx sQPrideDB::QP_req2Grp(idx req, sVec<idx> * grps, bool isMaster)
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

idx sQPrideDB::QP_req2GrpSerial(idx req, idx grp, idx * pcnt, idx svc)
{
    if(pcnt){
        if(svc!=sNotIdx) {
            *pcnt=db->ivalue(0, "SELECT COUNT(QPReq.reqID) FROM QPGrp, QPReq WHERE QPGrp.reqID=QPReq.reqID AND svcID=%" DEC " AND QPGrp.grpID=%" DEC, svc, grp);
        } else {
            *pcnt=db->ivalue(0, "SELECT COUNT(*) FROM QPGrp WHERE grpID=%" DEC, grp);
        }
    }
    return db->ivalue(0, "SELECT jobIDCollect FROM QPGrp WHERE grpID=%" DEC " AND reqID=%" DEC " LIMIT 1", grp, req);
}


idx sQPrideDB::QP_grpGetProgress(idx grp, idx * progress, idx * progress100)
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
/*
idx QP_grpSetCollector(idx grp, idx jobID)
{
    db->execute("update QPGrp set jobIDCollect=%" DEC " where grp=%" DEC " and jobIDCollect=%" DEC,grp,jobID);
    idx jo=db->ivalue(0,"select jobIDCollect from QPGrp where grp=%" DEC,grp);
    return jo==jobID ? jo : 0 ;
}

idx QP_grpGetCollector(idx grp)
{
    db->ivalue("select jobIDCollect from QPGrp where grp=%" DEC,grp);
    return grp;
}
*/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Data
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
bool sQPrideDB::QP_reqSetPar(idx req, idx type, const char * value,bool isOverwrite)
{
    sStr sql;
    if ( isOverwrite==0 || db->ivalue(-1, "SELECT reqID FROM QPReqPar WHERE reqID=%" DEC " AND type=%" DEC, req, type) ==-1) {
        sql.printf("INSERT INTO QPReqPar (reqID, type, val) VALUES(%" DEC ", %" DEC ", ", req, type);
        protectString(sql, value);
        sql.addString(")");
    } else {
        sql.addString("UPDATE QPReqPar SET val = ");
        protectString(sql, value);
        sql.printf(" WHERE reqID = %" DEC " AND type = %" DEC, req, type);
    }
    return db->executeString(sql.ptr());
}

char * sQPrideDB::QP_requestGetPar(idx req, idx type, sStr * val)
{
    db->getBlob(val->mex(), "SELECT val FROM QPReqPar WHERE reqID=%" DEC " AND type=%" DEC, req, type);
    val->add0();
    return val->ptr();
}

char * sQPrideDB::QP_reqDataGet(idx req, const char * dataName, sMex * data, idx * timestamp)
{
    sStr sql("SELECT dataBlob, UNIX_TIMESTAMP(modTm) FROM QPData WHERE reqID = %" DEC " AND dataName = ", req);
    protectString(sql, dataName);

    sMex enc;
    idx pos=data->pos();
    sVarSet res;

    if( db->getTable(&res, sql.ptr()) ) {
        idx encLen = 0;
        const char * enc = res.val(0, 0, &encLen);
        sString::decodeBase64(data, enc, encLen);
        if( timestamp ) {
            *timestamp = res.ival(0, 1, sIdxMax);
        }
        return (char*)data->ptr(pos);
    }
    return 0;
}

idx sQPrideDB::QP_reqDataGetTimestamp(idx req, const char * dataName)
{
    sStr sql("SELECT UNIX_TIMESTAMP(modTm) FROM QPData WHERE reqID = %" DEC " AND dataName = ", req);
    protectString(sql, dataName);

    return db->ivalue(sql.ptr(), sIdxMax);
}

bool sQPrideDB::QP_reqDataSet(idx req, const char * dataName, idx dsize, const void * data)
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

idx sQPrideDB::QP_reqDataGetAll(idx req, sVec < sStr > * dataVec, sStr * infos00, sVec < idx> * timestampVec )
{
    sStr sql;
    sVarSet res;

    db->getTable(&res, "SELECT dataName FROM QPData WHERE reqID = %" DEC, req);
    for(idx i=0; i<res.rows; ++i) {
        char * dataName=res(i,0);
        if( infos00 ){
            infos00->add(dataName,0);
        }

        if( dataVec ) {
            sStr * str=dataVec->add();
            idx stamp = sIdxMax;
            QP_reqDataGet( req, dataName, str, &stamp);
            if ( timestampVec ) {
                timestampVec->vadd(1, stamp);
            }
        } else if ( timestampVec ) {
            timestampVec->vadd(1, QP_reqDataGetTimestamp(req, dataName));
        }
    }
    if(infos00)infos00->add0(2);

    return res.rows;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Resources
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

char * sQPrideDB::QP_resourceGet(const char * service, const char * dataName, sMex * data, idx * tmstmp)
{
    sStr sql("SELECT dataBlob FROM QPResource WHERE svcName = ");
    protectString(sql, service);
    sql.addString(" AND dataName = ");
    protectString(sql, dataName);

    sMex enc;
    idx pos=data->pos();
    if( db->getTable(sql.ptr(), 0, &enc) ) {
        sString::decodeBase64(data , (char*)enc.ptr(), enc.pos());
        return (char*)data->ptr(pos);
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

bool sQPrideDB::QP_resourceSet(const char * service, const char * dataName, idx dsize, const void * data)
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

idx sQPrideDB::QP_resourceGetAll(const char * service, sStr * infos00 , sVec < sStr > * dataVec, sVec < idx > * tmStmpts )
{
    sStr sql;
    sVarSet res;

    sql.printf("SELECT dataName, UNIX_TIMESTAMP(modTm) FROM QPResource");
    if( service ) {
        sql.addString(" WHERE svcName = ");
        protectString(sql, service);
    }
    db->getTable(sql.ptr(), &res);
    for(idx i=0; i<res.rows; ++i) {
        char * dataName=res(i,0);
        if(infos00){
            infos00->add(dataName,0);
        }
        if(tmStmpts){
            tmStmpts->vadd(1, res.ival(i, 1));
        }
        if(dataVec) {
            sStr * str=dataVec->add();
            QP_resourceGet(service, dataName, str, 0);
        }
    }
    if( infos00 && infos00->length() )infos00->add0(2);

    return res.rows;
}

bool sQPrideDB::QP_resourceDel(const char * service, const char * dataName /* = 0 */)
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


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Service
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx sQPrideDB::QP_serviceID(const char * serviceName)
{
    sStr sql("SELECT svcID FROM QPSvc WHERE name = ");
    protectString(sql, serviceName);
    return db->ivalue(sql.ptr(), 0);
}

idx sQPrideDB::QP_serviceGet(void * Svc, const char * serviceName, idx svcId)
{
    sQPrideBase::Service * svc = (sQPrideBase::Service *)Svc;


    sStr sql;
    sql.printf( "SELECT "
        "svcID,permID,svcType,knockoutSec,maxJobs,nice,maxLoops,sleepTime"
        ",parallelJobs,delayLaunchSec,politeExitTimeoutSec,maxTrials,restartSec,priority"
        ",cleanUpDays,runInMT,noGrabDisconnect,noGrabExit,lazyReportSec,isUp,activeJobReserve,maxmemSoft,maxmemHard"
        ",UNIX_TIMESTAMP(cdate),name,title,cmdLine,hosts,emails,categories,capacity"
        " FROM QPSvc");
    if( serviceName ) {
        if( serviceName[0] == '?' ) {
            // TODO : is there a sane way to reimplement this API? Maybe via query language...
            fprintf(stderr, "%s:%u: ERROR: calling sQPrideDB::QP_serviceGet() with serviceName of the form \"?query\" is no longer supported\n", __FILE__, __LINE__);
            return 0;
        } else {
            sql.addString(" WHERE name = ");
            protectString(sql, serviceName);
        }
    } else if( svcId ) {
        sql.printf(" WHERE svcID = %" DEC,svcId);
    }

    sVarSet res;
    db->getTable(sql.ptr(), &res);
    svcId=0;
    for(idx ir=0; ir<res.rows; ++ir) {
        idx o=0;
        svc->svcID = res.ival(ir, o++);
        svc->permID = res.ival(ir, o++);
        svcId=svc->svcID;
        svc->svcType = res.ival(ir, o++);
        svc->knockoutSec = res.ival(ir, o++);
        svc->maxJobs = res.ival(ir, o++);
        svc->nice = res.ival(ir, o++);
        svc->maxLoops = res.ival(ir, o++);
        svc->sleepTime = res.ival(ir, o++);
        svc->parallelJobs = res.ival(ir, o++);
        svc->delayLaunchSec = res.ival(ir, o++);
        svc->politeExitTimeoutSec = res.ival(ir, o++);
        svc->maxTrials = res.ival(ir, o++);
        svc->restartSec = res.ival(ir, o++);
        svc->priority = res.ival(ir, o++);
        svc->cleanUpDays = res.ival(ir, o++);
        svc->runInMT = res.ival(ir, o++);
        svc->noGrabDisconnect = res.ival(ir, o++);
        svc->noGrabExit = res.ival(ir, o++);
        svc->lazyReportSec = res.ival(ir, o++);
        svc->isUp = res.ival(ir, o++);
        svc->activeJobReserve = res.ival(ir, o++);
        svc->maxmemSoft = res.ival(ir, o++);
        svc->maxmemHard = res.ival(ir, o++);
        svc->cdate = res.ival(ir, o++);
        strncpy(svc->name, res(ir,o++), sizeof(svc->name) - 1);
        strncpy(svc->title, res(ir,o++), sizeof(svc->title) - 1);
        strncpy(svc->cmdLine, res(ir,o++), sizeof(svc->cmdLine) - 1);
        strncpy(svc->hosts, res(ir,o++), sizeof(svc->hosts) - 1);
        strncpy(svc->emails, res(ir,o++), sizeof(svc->emails) - 1);
        strncpy(svc->categories, res(ir,o++), sizeof(svc->categories) - 1);
        svc->capacity = res.rval(ir, o++, 0);
        svc->hasReqToGrab=0;
        ++svc;
    }

    return svcId;//svc->svcID;
}

idx sQPrideDB::QP_serviceUp(const char * svc, idx isUpMask)
{
    sStr sql("UPDATE QPSvc SET isUp = %" DEC " WHERE name = ", isUpMask);
    protectString(sql, svc);
    db->executeString(sql.ptr());
    return isUpMask;
}

idx sQPrideDB::QP_serviceList(sStr * lst00, void * svcVecList)  // void * is sVec < Service >*
{
    sVec< sQPrideBase::Service > * svcvec = ( sVec < sQPrideBase::Service > * )svcVecList;
    idx svcCnt=0;
    if( svcvec) {
        svcCnt=db->ivalue("SELECT COUNT(name) FROM QPSvc", 0);
        if(svcCnt)
            QP_serviceGet(svcvec->add(svcCnt));
    }
    if(lst00) {
        sVarSet res;
        db->getTable("SELECT name FROM QPSvc", &res);
        for(idx i=0; i<res.rows; ++i){
            lst00->printf("%s",res(i,0));lst00->add0(1);
        }
        svcCnt=res.rows;
        lst00->add0(2);
    }
    return svcCnt;
}

void sQPrideDB::QP_getRegisteredIP(sVec <sStr> * ips, const char * equCmd)
{
    //escape ;
    if (strchr(equCmd,';')) return;

    const char * colon = strchr(equCmd, ':');
    idx hoursAgo = colon ? atoidx(colon + 1) : 0;

    sVarSet res;
    sStr sql("SELECT name, ip4 FROM QPHosts WHERE enabled = '1'");

    // equCmd is of the form "cat1, cat2 , cat3 :24"
    idx cat_count = 0;
    for(const char * cat = equCmd; cat && *cat && cat < colon; ) {
        // skip leading spaces
        for(; isspace(*cat); cat++);

        const char * comma = strchr(cat, ',');
        if( comma > colon ) {
            comma = 0;
        }

        // skip trailing spaces
        idx cat_len = comma ? comma - cat : sLen(cat);
        for(; cat_len > 0 && isspace(cat[cat_len - 1]); cat_len--);

        if( cat_count == 0 ) {
            sql.addString(" AND category IN (");
        } else {
            sql.addString(", ");
        }

        protectBuf(sql, cat, cat_len);

        cat_count++;
        cat = comma ? comma + 1 : 0;
    }
    if( cat_count ) {
        sql.addString(")");
    }

    // by default only hosts last active during the last hour are included
    // this can be changed by e.g. "qapp -shall=:24 cmd"
    if( hoursAgo > 0 ) {
        sql.printf(" AND (mdate > now() - INTERVAL %" DEC " hour)", hoursAgo);
    } else {
        sql.addString(" AND (mdate > now() - INTERVAL 1 hour)");
    }

    sql.addString(" ORDER BY name");

    db->getTable(sql.ptr(), &res);

    for ( idx i=0; i<res.rows ; ++i) {
        sStr * curHost=ips->add();
        curHost->printf("%s",res.val(i,0));
        curHost->add0();
        curHost->printf("%s",res.val(i,1));
    }
}

real sQPrideDB::QP_getHostCapacity(const char * hostname)
{
    sStr sql("SELECT capacity FROM QPHosts WHERE name = ");
    protectString(sql, hostname);
    return db->rvalue(sql.ptr(), 0);
}

void sQPrideDB::QP_registerHostIP(const char * sys)
{
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
                    struct in_addr* ptr = & ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
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

    if( !hostname[0] ) {
        strncpy(hostname, "hostname failed", sizeof(hostname) - 1);
    }
    if( !ip[0] ) {
        strncpy(ip, "ip missing", sizeof(ip) - 1);
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
            mem_sz = mem_pages * page_sz; // kilo-bytes
            const idx mem_sz_gb = mem_sz / 1024.0 / 1024.0 / 1024.0;
            capacity = mem_sz_gb / ncores; // giga-bytes/core
            // empirical 4GB per core rule
            const real GB_PER_CORE = 4.0;
            if( capacity - GB_PER_CORE > 0.1 ) {
                capacity = ncores;
            } else {
                capacity = mem_sz_gb / GB_PER_CORE;
            }
        }
    } else {
        ncores = 0;
    }
#endif
    sSql::sqlProc sql(*db, "sp_registerHostIP_v2");
    sql.Add(hostname).Add(ip).Add(sys).Add(capacity).Add(ncores).Add(mem_sz);
    sql.execute();
}


void sQPrideDB::QP_reqCleanTbl(sVec <sStr> *tblfiles, idx req, const char *dataname)
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

idx sQPrideDB::QP_workRegisterTime(const char * svc, const char * params, idx amount, idx time) {
    if( !svc ) return -1;
    if( amount <= 0 ) return 0;
    if( time <= 0 ) return 0;

    sVarSet res;

    //if (params) {sStr paramsStr(params); }
    //else {sStr paramsStr("null"); }
    sStr sql("SELECT amountOfWork, timeSeconds FROM QPPerform WHERE svcName = ");
    protectString(sql, svc);
    if( params ) {
        sql.addString(" AND paramset = ");
        protectString(sql, params);
    }
    db->getTable(sql.ptr(), &res);
    sql.cut(0);

    if( res.rows == 1 ) { // entry for (svcid,paramset) already exists in db, needs to be updated
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
    } else if( res.rows == 0 ) { //new entry for (svcid,paramset)
        if( params ) { //parameter set is not null
            sql.addString("INSERT INTO QPPerform (svcName, amountOfWork, timeSeconds, paramset) VALUES (");
            protectString(sql, svc);
            sql.printf(", %" DEC ", %" DEC ", ", amount, time);
            protectString(sql, params);
            sql.addString(")");
            db->executeString(sql.ptr());
        } else {//parameter set is null
            sql.printf("INSERT INTO QPPerform (svcName, amountOfWork, timeSeconds) VALUES (");
            protectString(sql, svc);
            sql.printf(", %" DEC ", %" DEC ")", amount, time);
            db->executeString(sql.ptr());
        }
        return time;
    } else {
        // no multiple entries for (svcid,paramset) are allowed in db
        return -1;
    }
}


idx sQPrideDB::QP_workEstimateTime(const char * svc, const char * params, idx amount) {
    if (!svc) return 0;
    if (amount <= 0) return 0;

    sVarSet res;

    sStr sql("SELECT amountOfWork, timeSeconds FROM QPPerform WHERE svcName = ");
    protectString(sql, svc);
    if( params ) {
        sql.addString(" AND paramset = ");
        protectString(sql, params);
    }

    db->getTable(sql.ptr(), &res);

    if (res.rows>1) return -1; // no multiple entries for (svcid,paramset) are allowed in db
    if (res.rows==1) { // entry for (svcid,paramset) already exists in db, needs to be updated
        idx amountOfWork = res.ival(0, 0, -1);
        if( amountOfWork <= 0 ) {
            return -1;
        }

        idx timeSeconds = res.ival(0, 1, -1);
        if( timeSeconds <= 0 ) {
            return -1;
        }

        idx estimated = (idx)(timeSeconds*amount/amountOfWork);
        return estimated;
    }

    return -1;
}


void sQPrideDB::QP_servicePurgeOld(sVec<idx> * reqList, const char * service, idx limit, bool no_delete)
{
    sVarSet res;
    sSql::sqlProc sql(*db, "sp_servicePurgeOld");
    sql.Add(service).Add(limit ? limit : -1).Add(
#if _DEBUG
        true
#else
        no_delete
#endif
        );

    sql.getTable(&res);

    for(idx i = 0; i < res.rows; ++i) {
        reqList->vadd(1, res.ival(i, 0));
    }
}

void sQPrideDB::QP_servicePath2Clean(sVarSet & res)
{
    db->getTable(&res, "SELECT val, cleanUpDays, nameMasks FROM QPCfg WHERE cleanUpDays IS NOT NULL OR nameMasks IS NOT NULL");
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  DB
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
idx sQPrideDB::QP_dbHasLiveConnection(void)
{
    if(db->status!=sSql::eConnected)return 0;
    return QP_serviceID("qm") ? 1 : 0 ;

}
void sQPrideDB::QP_dbDisconnect(void)
{
    db->disconnect();
}
idx sQPrideDB::QP_dbReconnect(void)
{
    return db->realConnect();
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Jobs
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
void sQPrideDB::QP_jobRegisterAlive(idx job)
{
    db->execute("UPDATE QPJob SET aliveTm=NOW() WHERE jobID=%" DEC, job);
}
idx sQPrideDB::QP_jobSetStatus(idx job, idx jobstat)
{
    db->execute("UPDATE QPJob SET stat=%" DEC " WHERE jobID=%" DEC, jobstat, job);
    return jobstat;
}

idx  sQPrideDB::QP_jobSetStatus(sVec < idx > * jobs, idx jobstat)
{
    return _QP_SetIdxVar(db, jobs, jobstat, "QPJob", "jobID", "stat", 0);
}

idx sQPrideDB::QP_jobSetAction(idx job, idx jobact)
{
    return _QP_SetIdxVar(db, job, jobact, "QPJob", "jobID", "act", ", actTm = NOW()");
}


idx sQPrideDB::QP_jobSetAction(sVec < idx > * jobs, idx jobact)
{
    return _QP_SetIdxVar(db, jobs, jobact, "QPJob", "jobID", "act", ", actTm = NOW()");
}


idx sQPrideDB::QP_jobRegister(const char * serviceName, const char * hostName, idx pid, idx inParallel)
{
    sSql::sqlProc sql(*db, "jobIns");
    sql.Add(serviceName).Add(hostName).Add(pid).Add(inParallel);
    return sql.ivalue(0);
}

idx sQPrideDB::QP_jobSetMem(idx job, idx curMemSize, idx maxMemSize)
{
    db->execute("UPDATE QPJob SET mem=%" DEC ", maxmem=%" DEC " WHERE jobID=%" DEC, curMemSize, maxMemSize, job);
    return curMemSize;
}

idx sQPrideDB::QP_jobGetAction(idx job)
{
    return db->ivalue(0, "SELECT act FROM QPJob WHERE jobID = %" DEC, job);
}

idx sQPrideDB::QP_jobSetReq(idx job, idx req)
{
    return db->uvalue(0, "UPDATE QPJob SET reqID = %" DEC " WHERE jobID = %" DEC "; UPDATE QPReq SET jobID = %" DEC " WHERE reqID = %" DEC "; SELECT 1;", req, job, job, req);
}

idx sQPrideDB::QP_jobGet(idx jobID, sQPrideBase::Job * jobs, idx jobCnt, const char * _wherecls)
{
    sQPrideBase::Job * job = jobs;

    sStr sql("SELECT "
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
    db->getTable( &res, sql.ptr());
    idx jobId=0;
    for(idx ir=0; ir < res.rows && ir < jobCnt; ++ir) {
        idx o=0;
        job->jobID = res.ival(ir, o++);
        jobId=job->jobID;
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
        strncpy(job->hostName, res.val(ir, o++), sizeof(job->hostName) - 1);
        ++job;
    }
    for(idx ir = res.rows; ir < jobCnt; ir++) {
        sSet<sQPrideBase::Job>(job++);
    }

    return jobId;
}


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  request and job info
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

bool sQPrideDB::QP_setLog(idx req, idx job, idx level, const char * txt)
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

idx sQPrideDB::QP_getLog(idx req, bool isGrp, idx job, idx level, sVarSet & log)
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


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  System Operations
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


idx sQPrideDB::QP_sysPeekOnHost(void * srvl,  const char * hostname )
{
    sVec < sQPrideBase::Service > * srvlst=(sVec < sQPrideBase::Service > *)srvl;

    QP_serviceList(0, srvl);  // void * is sVec < Service >*

    idx k=0;
    for ( k=0; k< srvlst->dim(); ++k) {
        if(hostNameListMatch(srvlst->ptr(k)->hosts, hostname)) // this service is supposed to run on this compute node
            continue;

        if(k!=srvlst->dim()-1)
            *srvlst->ptr(k)=*srvlst->ptr(srvlst->dim()-1);

        srvlst->cut(srvlst->dim()-1);
        --k;
    }

    //sStr s("select QPSvc.svcID, count(QPReq.reqId) from QPSvc , QPReq where QPSvc.svcID = QPReq.svcID and QPReq.stat=1 and QPReq.act=2 ");
    //s.printf(" group by svcID");

    sVarSet res;
    db->getTable(&res, "SELECT svcID, COUNT(reqId) FROM QPReq WHERE stat = 1 AND act = 2 AND scheduleGrab <= NOW() GROUP BY svcID");

    for ( idx ir=0; ir< res.rows; ++ir) {
        idx svcID = res.ival(ir, 0, -1);

        for ( idx k=0; k<srvlst->dim(); ++k) {
            if( svcID == srvlst->ptr(k)->svcID ) {
                srvlst->ptr(k)->hasReqToGrab = res.ival(ir, 1, srvlst->ptr(k)->hasReqToGrab);
                break;
            }
        }
    }
    return srvlst->dim();
}

idx sQPrideDB::QP_sysPeekReqOrder(idx req, const char * srv, idx * pRunning )
{
    sSql::sqlProc sql(*db, "reqPeekOrder");
    sql.Add(srv ? srv : "").Add(req).Add((idx)1).Add((idx)2);

    //idx order=db->ivalue(0,sql.ptr());
    sVarSet res;
    sql.getTable(&res);

    idx order=atoidx(res(0,0));
    if(pRunning)*pRunning=atoidx(res(0,1));

    return order;
}


idx sQPrideDB::QP_sysGetKnockoutJobs(sVec <sQPrideBase::Job> * jobs, sVec < idx > * svcIDs)
{
    sStr sql("SELECT COUNT(jobID) FROM QPJob");
    idx tail=sql.length();
    sql.printf(", QPSvc WHERE QPSvc.svcID = QPJob.svcID AND QPJob.stat = %i AND QPSvc.knockoutSec != 0 AND UNIX_TIMESTAMP(CURRENT_TIMESTAMP) - UNIX_TIMESTAMP(QPJob.aliveTm) > QPSvc.knockoutSec", sQPrideBase::eQPJobStatus_Running);
    if(svcIDs) { sql.printf(" AND QPSvc.svcID IN (");sString::printfIVec(&sql, svcIDs);sql.printf(")"); }
    idx jobCnt=db->ivalue(0,sql.ptr());
    sQPrideBase::Job * js=jobs->add(jobCnt);
    return sQPrideDB::QP_jobGet(0, js, jobCnt, sql.ptr(tail) );
}

idx sQPrideDB::QP_sysGetImpoliteJobs(sVec <sQPrideBase::Job> * jobs, sVec < idx > * svcIDs)
{
    sStr sql("SELECT COUNT(jobID) FROM QPJob");
    idx tail=sql.length();
    sql.printf(", QPSvc WHERE QPSvc.svcID=QPJob.svcID AND QPJob.stat = %i AND QPJob.act = %i AND QPSvc.politeExitTimeoutSec!=0 AND UNIX_TIMESTAMP(CURRENT_TIMESTAMP) - UNIX_TIMESTAMP(QPJob.actTm) > QPSvc.politeExitTimeoutSec" ,sQPrideBase::eQPJobStatus_Running,sQPrideBase::eQPJobAction_Kill);
    if(svcIDs) { sql.printf(" AND QPSvc.svcID IN (");sString::printfIVec(&sql, svcIDs);sql.printf(")"); }
    idx jobCnt=db->ivalue(0,sql.ptr());
    sQPrideBase::Job * js=jobs->add(jobCnt);
    return sQPrideDB::QP_jobGet(0, js, jobCnt, sql.ptr(tail) );
}

idx sQPrideDB::QP_sysJobsGetList(sVec <sQPrideBase::Job> * jobs , idx stat, idx act, const char * hostname )
{
    sStr sql("SELECT COUNT(jobID) FROM QPJob");
    idx tail=sql.length();
    sql.printf(" WHERE stat=%" DEC, stat);
    if( act != sQPrideBase::eQPJobAction_Any ) {
        sql.printf(" AND act = %" DEC, act );
    }
    if( hostname ) {
        sql.printf(" AND hostName = ");
        protectString(sql, hostname);
    }

    idx jobCnt=db->ivalue(sql.ptr(), 0);
    sQPrideBase::Job * js=jobs->add(jobCnt);
    return sQPrideDB::QP_jobGet(0, js, jobCnt, sql.ptr(tail));
}

idx sQPrideDB::QP_sysRecoverRequests(sVec <idx > * , sVec < idx > * )
{
    db->execute("CALL recoverReqs()");
    return 0;
}

idx sQPrideDB::QP_sysCapacityNeed(idx * capacity_total)
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
