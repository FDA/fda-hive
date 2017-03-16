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
#include <ulib/uproc.hpp>
#include <ulib/ufolder.hpp>
#include "uperm.hpp"
#include <qlib/QPride.hpp>

using namespace slib;

// order important as must follow prop enum in class declaration!!!
const udx s_custom_prop = 3; // start of custom props in array
const char* sUsrProc::sm_prop[] = {"not used", "reqID", "svc", "completed", "progress", "started", "status", "action", "progress100" };

struct slib::sUsrProcReq {
    idx grpCnt;
    bool isGrpId;
    sQPride::Request R;

    sUsrProcReq()
        : grpCnt(0), isGrpId(false)
    {
        sSet(&R);
    }
};

// static
udx sUsrProc::isQprideProp(const char* prop)
{
    for(udx i = s_custom_prop; i < sDim(sm_prop); ++i) {
        if( strcasecmp(prop, sm_prop[i]) == 0 ) {
            return i;
        }
    }
    return 0;
}

udx sUsrProc::propSet(const char* prop, const char** groups, const char** values, udx cntValues, bool isAppend /* = false */, const udx * path_lens /* = 0 */, const udx * value_lens /* = 0 */)
{
    if( m_usr.isAllowed(Id(), ePermCanWrite) ) {
        if( prop && strcasecmp(prop, "folder") == 0 ) {
            /* The "folder" field in type "process" exists for UI to specify in which folder to create the
               process object. (The default is inbox.) We do not want to preserve this field in the process
               object in db to avoid pulling in the folder as a dependency in sharing etc., especially if the
               original folder into which the process was created is later deleted. Instead, interpret this
               field purely in the opposite direction - add a "child" field to the folder and point it at our
               process object. */
            if( !isAppend || !sUsrFolder::attachedTo(0, m_usr, Id()) ) {
                sUsrFolder * ufolder = 0;
                if( cntValues && values[0] && values[0][0] && strcmp(values[0], "0") != 0 ) {
                    sHiveId ufolder_id(values[0]);
                    sUsrObj * uobj = m_usr.objFactory(ufolder_id);
                    ufolder = dynamic_cast<sUsrFolder*>(uobj);
                    if( !ufolder ) {
                        delete uobj;
                        uobj = ufolder = 0;
                    }
                }
                if( !ufolder ) {
                    ufolder = sSysFolder::Inbox(m_usr);
                }
                if( ufolder && ufolder->Id() ) {
                    ufolder->attach(*this);
                }
                delete ufolder;
            }
            return 1; // always success or you can't launch anything
        } else if( !isQprideProp(prop) ) {
            return TParent::propSet(prop, groups, values, cntValues, isAppend, path_lens, value_lens);
        }
    }
    return cntValues;
}

bool sUsrProc::propGet(udx propId, sUsrProcReq& ur, idx& res) const
{
    sQPrideBase * qp = m_usr.QPride();
    if( ur.grpCnt == 0 ) {
        qp->req2GrpSerial(ur.R.reqID, ur.R.reqID, &ur.grpCnt);
        if( ur.grpCnt < 1 ) {
            ur.grpCnt = 1;
        }
        ur.isGrpId = (qp->req2Grp(ur.R.reqID) == ur.R.reqID && ur.grpCnt > 1) ? true : false;
    }
    bool retval = true;
    switch(propId) {
        case ePropStarted:
            res = ur.R.takenTm;
            retval = res > 0;
            break;
        case ePropCompleted:
            res = ur.R.doneTm;
            retval = res > 0;
            break;
        case ePropProgress:
            if( ur.isGrpId ) {
                qp->grpGetProgress(ur.R.reqID, &res, 0);
            } else {
                res = ur.R.progress;
            }
            break;
        case ePropStatus:
            res = ur.R.stat;
            if( ur.isGrpId ) {
                sVec<idx> stat;
                qp->grpGetStatus(ur.R.reqID, &stat);
                idx sQ = 0, sR = 0, sD = 0, sS = 0, sK = 0, sE = 0;
                for(idx i = 0; i < stat.dim(); ++i) {
                    if( stat[i] <= qp->eQPReqStatus_Waiting) {
                        ++sQ;
                    } else if( stat[i] == qp->eQPReqStatus_Running || stat[i] == qp->eQPReqStatus_Processing ) {
                        ++sR;
                    } else if( stat[i] == qp->eQPReqStatus_Done) {
                        ++sD;
                    } else if( stat[i] == qp->eQPReqStatus_Suspended) {
                        ++sS;
                    } else if( stat[i] == qp->eQPReqStatus_Killed) {
                        ++sK;
                    } else {
                        ++sE;
                    }
                }
                res = sR ? qp->eQPReqStatus_Running :
                        (sQ ? qp->eQPReqStatus_Waiting :
                            (sE ? qp->eQPReqStatus_ProgError :
                                (sS ? qp->eQPReqStatus_Suspended :
                                    (sK ? qp->eQPReqStatus_Killed : (sD ? qp->eQPReqStatus_Done : qp->eQPReqStatus_Waiting)))));
            }
            break;
        case ePropAction:
            res = ur.R.act;
            break;
        case ePropProgress100:
            if( ur.isGrpId ) {
                qp->grpGetProgress(ur.R.reqID, 0, &res);
                res /= ur.grpCnt;
            } else {
                res = ur.R.progress100;
            }
            break;
        default:
            // fail if name is not recognize, normally this never happens
            return false;
    }
    return retval;
}


udx sUsrProc::propGet(const char* prop, sVarSet& res, bool sort/* = false */) const
{
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        bool isRetrieved = false;
        sQPrideBase * qp = m_usr.QPride();
        udx propId;
        if( qp && (propId = isQprideProp(prop)) ) {
            idx req = reqID();
            if( req ) {
                sUsrProcReq ur;
                if( qp->requestGet(req, &ur.R) ) {
                    isRetrieved = true;
                    idx val;
                    if( propGet(propId, ur, val) ) {
                        res.addRow().addCol(val).addCol((char*) 0);
                        //if( propGet(ePropStatus, ur, val) && val == qp->eQPReqStatus_Done ) {
                        //    const_cast<sUsrProc&>(*this).propSync(ur);
                        //}
                    } else {
                        isRetrieved = false;
                    }
                }
            }
        }
        if( !isRetrieved ) {
            return TParent::propGet(prop, res, sort);
        }
    }
    return res.rows;
}

bool sUsrProc::onDelete(void)
{
    if( TParent::onDelete() ) {
        sQPrideBase * qp = m_usr.QPride();
        if( qp ) {
            qp->killGrp(reqID());
        }
        return true;
    }
    return false;
}

bool sUsrProc::onPurge(void)
{
    if( TParent::onPurge() ) {
        sQPrideBase * qp = m_usr.QPride();
        if( qp ) {
            qp->purgeReq(reqID());
        }
        return true;
    }
    return false;
}

bool sUsrProc::propSync(void)
{
    sQPrideBase * qp = m_usr.QPride();
    if( qp ) {
        idx req = reqID();
        if( req ) {
            sUsrProcReq ur;
            if( qp->requestGet(req, &ur.R) ) {
                return propSync(ur);
            }
        }
    }
    return false;
}

bool sUsrProc::propSync(sUsrProcReq& ur)
{
    udx q = 0;
    is_auditable = false;
    for(udx i = s_custom_prop; i < sDim(sm_prop) ; ++i) {
        idx res;
        if( propGet(i, ur, res) ) {
            char strBuf[64];
            sprintf(strBuf, "%"DEC, res);
            const char * strBufPtr = strBuf;
            q += TParent::propSet(sm_prop[i], (const char**) 0, &strBufPtr, 1, false);
        }
    }
    is_auditable = true;
    return q == sDim(sm_prop) - s_custom_prop;
}






idx sUsrProc::createProcesForsubmission(sQPrideBase * qp , sVar * pForm , sUsr * user, sVec< sUsrProc > & procObjs, sQPride::Service * pSvc, sStr * strObjList, sStr * log)
{
    // check that inbox exists before trying to create any objects
    std::auto_ptr<sUsrFolder> inbox(sSysFolder::Inbox(*user));
    if( !inbox.get() ) {
        return  errHiveTools_NoInbox;
    }

    //if(doCreateProcesses){
        log->cut(0);
        if(!user->propSet(*pForm, *log, procObjs, strObjList)){
            return  errHiveTools_ProcessCreation;
        }
    //}

        /*
    if( procObjs.dim() > 1 ) {
        sUsrFolder * sub = inbox->createSubFolder("submission %"DEC, req);
        if( sub ) {
            inbox->reset(sub);
        }
    }*/

    // if the process objects weren't attached to a specific folder, attach them to the inbox
    for(idx i = 0; i < procObjs.dim(); ++i) {
        if( procObjs[i].Id() && !sUsrFolder::attachedTo(0, *user, procObjs[i].Id()) ) {
            inbox->attach(procObjs[i]);
        }
    }

    return 0;
}

idx sUsrProc::standardizedSubmission(sQPrideBase * qp, sVar * pForm, sUsr * user, sVec<sUsrProc> & procObjs, idx cntParallel, idx * pReq, sQPride::Service * pSvc, idx previousGrpSubmitCounter, sStr * strObjList, sStr * log)
{
    idx reqPriority = 0;
    if( procObjs.dim() ) {
        reqPriority = ((sUsrObj *) &(procObjs[0]))->propGetI("reqPriority");
        const char * svcFromProc = ((sUsrObj *) &(procObjs[0]))->propGet("svc");
        if( svcFromProc ) {
            qp->serviceGet(pSvc, svcFromProc, 0); // otherwise retrieve it from the reqID
        } else {
            const char * algoFromProc = ((sUsrObj *) &(procObjs[0]))->propGet("algo");
            if( algoFromProc ) {
                sUsrObj uo(*user, sHiveId(algoFromProc));
                if( uo.Id() ) {
                    const char * svcFromAlgo = uo.propGet("qpsvc");
                    if( svcFromAlgo ) {
                        qp->serviceGet(pSvc, svcFromAlgo, 0); // otherwise retrieve it from the reqID
                    }
                }
            }
        }
    }
    const idx requiresGroupSubmission = false;
    bool postpone = pForm->boolvalue("isPostponed", false);
    if( procObjs.dim() ) {
        postpone = ((sUsrObj *) &(procObjs[0]))->propGetBool("isPostponed");
    }
    // only actions seems to make sense upon submission
    // ALWAYS HOLD TO MAKE SURE DB IS SETUP COMPLETELY FOR REQUEST(s) TO RUN
    const idx req = qp->reqProcSubmit(cntParallel, pForm, pSvc->name, 0, sQPrideBase::eQPReqAction_Suspend, requiresGroupSubmission, reqPriority);
    if( req ) {
        for(idx ip = 0; ip < procObjs.dim(); ++ip) {
            sUsrObj * so = (sUsrObj *) (&(procObjs[ip]));
            so->propSetI("reqID", req);
            so->propSet("svcTitle", pSvc->title);
            so->propSet("svc", pSvc->name);
            procObjs[ip].propSync();
        }
        if( strObjList && strObjList->length() ) {
            qp->reqSetPar(req, sQPrideBase::eQPReqPar_Objects, strObjList->ptr());
        }
        idx userKey = pForm->ivalue("userKey", 0);
        if( userKey ) {
            qp->reqSetUserKey(req, userKey);
        }
        if( cntParallel > 1 ) {
            sVec<idx> reqIds;
            qp->grp2Req(req, &reqIds);
            if( postpone ) {
                qp->reqSetStatus(&reqIds, sQPrideBase::eQPReqStatus_Suspended);
            } else {
                qp->reqSetAction(&reqIds, sQPrideBase::eQPReqAction_Run);
            }
        } else {
            if( postpone ) {
                qp->reqSetStatus(req, sQPrideBase::eQPReqStatus_Suspended);
            } else {
                qp->reqSetAction(req, sQPrideBase::eQPReqAction_Run);
            }
        }
    }
    if( !req ) {
        for(idx ip = 0; ip < procObjs.dim(); ++ip) {
            procObjs[ip].actDelete();
        }
        if( log ) {
            log->printf("Request could not be submitted!");
        }
        return errhiveTools_SubmissionFaled;
    }
    if( pReq ) {
        *pReq = req;
    }
    return 0;
}
