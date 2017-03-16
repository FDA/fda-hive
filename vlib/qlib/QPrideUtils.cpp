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
#include <slib/core/index.hpp>
#include <slib/std/online.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>
#include <qlib/QPrideBase.hpp>
#include "QPrideConnection.hpp"
#include <ulib/usr.hpp>
#include <time.h>

using namespace slib;


idx sQPrideBase::reqProcSubmit( idx cntParallel, sVar * pForm, const char * svc , idx grp, idx autoAction /* = sQPrideBase::eQPReqAction_Run */, bool requiresGroupSubmission /* = false */, idx priority , idx previousGrpSubmitCounter)  // returns reqId
{
    idx req=0;
    if( cntParallel > 1 || requiresGroupSubmission ) {
        req=grpSubmit(svc,0,priority,cntParallel,0,previousGrpSubmitCounter);
        if(grp) {
            sVec < idx > reqList;
            grp2Req(req,&reqList,0,req);
            for (idx ig=0; ig< reqList[ig]; ++ig ) {
                grpAssignReqID(reqList[ig], grp, ig);
            }
        }
    } else {
        req = reqSubmit(svc,0,priority);
        idx g = grp ? grp : (pForm ? pForm->ivalue("grp", 0) : 0);
        if( g ) {
            grpAssignReqID(req, g, 0);
        }
    }

    if(req){
       // associate HTML for with the request
        if(pForm) reqSetData(req,"formT.qpride",pForm);

        // set the action to run
        if( cntParallel > 1 || requiresGroupSubmission ) {
            sVec < idx > reqIds;
            grp2Req(req, &reqIds);
            reqSetAction(&reqIds, autoAction);
        } else {
            reqSetAction(req, autoAction);
        }
    }
    return req;
}



struct ProgressItem
{
        idx reqID, grpID, svcID;
        idx takenTm, actTm, doneTm;
        idx stat, act;
        idx orderBefore, runningBefore;
        idx orderCnt, runningCnt;
        idx reportLevel, cntReportLevel;
        idx progress, progress100;
        idx waitTime;
        char parent[256];
        idx cnt;
        ProgressItem()
        {
            takenTm = sIdxMax;
            actTm = sIdxMax;
            doneTm = 0;
            stat = sQPrideBase::eQPReqStatus_Max;
            act = sIdxMax;
            progress = 0;
            progress100 = 0;
            waitTime = 0;
            orderBefore = sIdxMax;
            runningBefore = sIdxMax;
            orderCnt = 0;
            runningCnt = 0;
            reportLevel = 0;
            cntReportLevel = 0;
            cnt = 0;
            svcID = 0;
            reqID = 0;
            grpID = 0;
        }
        ProgressItem(const char * l_parent, idx l_grpID, idx l_reqID, idx l_svcID, idx l_takenTm, idx l_actTm, idx l_doneTm, idx l_stat, idx l_act, idx l_progress, idx l_progress100, idx l_waitTime, idx l_orderBefore, idx l_runningBefore, idx l_reportLevel)
        {
            update(l_parent, l_grpID, l_reqID, l_svcID, l_takenTm, l_actTm, l_doneTm, l_stat, l_act, l_progress, l_progress100, l_waitTime, l_orderBefore, l_runningBefore, l_reportLevel);
        }

        ProgressItem * update(const char * l_parent, idx l_grpID, idx l_reqID, idx l_svcID, idx l_takenTm, idx l_actTm, idx l_doneTm, idx l_stat, idx l_act, idx l_progress, idx l_progress100, idx l_waitTime, idx l_orderBefore, idx l_runningBefore, idx l_reportLevel)
        {
            strncpy(parent, l_parent, sizeof(parent) - 1);

            reqID = l_reqID;
            if( !grpID )
                grpID = l_grpID;
            svcID = l_svcID;
//        execTime+=l_execTime;
//        waitTime+=l_waitTime;

            if( orderBefore > l_orderBefore )
                orderBefore = l_orderBefore;
            if( runningBefore > l_runningBefore )
                runningBefore = l_runningBefore;

            //if(l_stat>sQPrideBase::eQPReqStatus_Running || stat>sQPrideBase::eQPReqStatus_Running )stat=sMax(stat,l_stat);
            //else stat=sMin(stat,l_stat);

            if( stat >= sQPrideBase::eQPReqStatus_Done ){
                if( ( stat > l_stat && l_stat < sQPrideBase::eQPReqStatus_Done ) || ( stat < l_stat && l_stat >= sQPrideBase::eQPReqStatus_Done) || stat == sQPrideBase::eQPReqStatus_Max ) {
                    stat = l_stat;
                }
            }
            else if( l_stat < sQPrideBase::eQPReqStatus_Suspended && stat < l_stat ) {
                    stat = l_stat;
            }


            progress += l_progress;
            progress100 += l_progress100;

            if( act > l_act ) {
                act = l_act;
            }
            if( reportLevel < l_reportLevel ) {
                reportLevel = l_reportLevel;
                cntReportLevel = 0;
            } else {
                ++cntReportLevel;
            }
            if( !reqID ) {
                if( doneTm < l_doneTm ) {
                    doneTm = l_doneTm;
                }
                if( takenTm >= l_takenTm && l_takenTm > 0 ) {
                    takenTm = l_takenTm;
                }
            } else {
                doneTm = l_doneTm;
                if( l_takenTm > 0 )
                    takenTm = l_takenTm;
            }
            if( l_actTm && actTm >= l_actTm ) {
                actTm = l_actTm;
            }
            waitTime += l_waitTime;

            ++cnt;
            return this;
        }

        void print(const char * title, sStr * out)
        {
            out->printf("\"%s\"", title);
            out->printf(",\"%s\"", parent);
            if( cnt > 1 ) {
                out->printf(",%" DEC, cnt);
            } else {
                out->printf(",");
            }
            out->printf(",%" DEC, reqID);
            out->printf(",%" DEC, grpID);
            out->printf(",%" DEC, svcID);
            out->printf(",%" DEC, stat);
            if( progress ) {
                out->printf(",%" DEC, progress);
            } else {
                out->printf(",");
            }
            out->printf(",%" DEC, progress100 / (cnt ? cnt : 1));
            out->printf(",%" DEC, actTm);
            out->printf(",%" DEC, (takenTm == sIdxMax ? doneTm : takenTm));
            out->printf(",%" DEC, doneTm);
            out->printf(",%" DEC, waitTime);
            out->printf(",%" DEC, doneTm - (takenTm == sIdxMax ? doneTm : takenTm));
            out->printf(",%s", sQPrideBase::getLevelName(reportLevel));
            if( orderBefore ) {
                out->printf(",%" DEC, orderBefore);
            } else {
                out->printf(",");
            }
            if( runningBefore ) {
                out->printf(",%" DEC, runningBefore);
            } else {
                out->printf(",");
            }
            out->printf(",%" DEC, act);
            out->printf("\n");
        }
};

idx sQPrideBase::reqProgressReport(sStr * group, sStr * list, idx req, idx start, idx cnt, idx minLevelInfo, const char * svcName, sDic<idx> * svcIDs)
{
    if( !cnt ) {
        cnt = sIdxMax;
    }
    sVec<Request> rList(sMex::fSetZero);
    requestGetForGrp(req, &rList, svcName);

    if( !rList.dim() && !requestGet(req, rList.add()) ) {
        return 0;
    }
    if( rList.dim() == 1 ) {
        rList[0].grpID = rList[0].reqID;
    }
    sVec<QPLogMessage> infos;
    grpGetInfo(rList[0].grpID, minLevelInfo, infos);

    (group ? group : list)->printf("name,parent,cnt,reqID,grpID,svcID,stat,progress,progress100,actTime,takenTime,doneTime,waitTime,execTime,reportType,orderExecute,runningBefore,act\n");
    idx now = time(0);
    sDic<ProgressItem> prgList;
    sStr tmp;
    for(idx i = 0; i < rList.dim(); ++i) {
        Request * sr = rList.ptr(i);
        ProgressItem & svc_item = prgList[tmp.printf(0, "__svc-%s", sr->svcName)];
        if( svc_item.orderBefore == sIdxMax) {
            // go to db only once when new service type encountered
            svc_item.orderCnt = sysPeekReqOrder(sr->reqID, 0, &svc_item.runningCnt);
        }
        if( sr->stat == sQPrideBase::eQPReqStatus_Waiting ) {
            sr->progress100 = 0;
            ++svc_item.orderCnt;
        }
        idx runningBefore = svc_item.runningCnt;
        idx orderBefore = svc_item.orderCnt; //if(svc_item.orderBefore>=1)++svc_item.orderCnt;
        if( sr->takenTm && ( !sr->doneTm || sr->doneTm < sr->takenTm) ) {
            sr->doneTm = now;
        }
        idx waitTime = sr->cdate ? ((sr->takenTm ? sr->takenTm : now) - sr->cdate) : 0;
        if( sr->scheduleGrab ) {
            waitTime = 0;
            sr->takenTm = sr->cdate;
            sr->actTm = sr->cdate;
        }

        idx reportLevel = 0;
        for(idx l = 0; l < infos.dim(); ++l) {
            if( sr->reqID == infos[l].req && infos[l].level > reportLevel ) {
                reportLevel = (enum eQPInfoLevel) infos[l].level;
            }
        }
        svc_item.update("Total Progress", sr->grpID, 0, sr->svcID, sr->takenTm, sr->actTm, sr->doneTm, sr->stat, sr->act, sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);
        prgList[tmp.printf(0, "__req-%" DEC, sr->reqID)].update(sr->svcName, sr->grpID, sr->reqID, sr->svcID, sr->takenTm, sr->actTm, sr->doneTm, sr->stat, sr->act, sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);
        prgList["__root"].update("root", sr->grpID, rList.dim() == 1 ? sr->reqID : 0 , 0, sr->takenTm, sr->actTm, sr->doneTm, sr->stat, sr->act, sr->progress, sr->progress100, waitTime, orderBefore, runningBefore,
            reportLevel);
    }
    if( group ) {
        prgList["__root"].print("Total Progress", group);
        for(idx id = 0; id < prgList.dim(); ++id) {
            const char * key = (const char *) prgList.id(id);
            if( sIs("__svc-", key) ) {
                prgList[id].print(key + 6, group);
            }
        }
    }
    if( !list ) {
        return rList.dim();
    }
    for(idx id = start; id < start + cnt && id < prgList.dim(); ++id) {
        const char * key = (const char *) prgList.id(id);
        if( sIs("__req-", key) ) {
            if( svcIDs ) {
                sStr svcID(sMex::fBlockNormal);
                if( !svcIDs->get(svcID.printf("%" DEC, prgList[id].svcID)) ) {
                    continue;
                }
            }
            prgList[id].print(key + 6, list);
        }
    }
    if( start + cnt < prgList.dim() ) {
        list->printf("... %" DEC " more ...,root,,,,,,,,,,,,,,,,\n", prgList.dim() - start - cnt);
    }
    return rList.dim();

}


