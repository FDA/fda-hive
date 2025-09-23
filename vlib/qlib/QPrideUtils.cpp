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
#include <slib/utils/json/parser.hpp>
#include <qlib/QPrideBase.hpp>

#include "QPrideConnection.hpp"
#include <ulib/usr.hpp>
#include <time.h>
#include <limits.h>

using namespace slib;

struct TimeSlice {
    idx start;
    idx end;
};

class TSAggregator {
    sVec<TimeSlice> slices;

    public:

        int add(idx start, idx end) {
            TimeSlice dummy = {start, end};
            return add(dummy);
        }

        int add(TimeSlice slice) {

            idx res = -1;
            do { 
                res = addWorker(slice);
                if (res >= 0)
                {
                    slice = slices[res];
                    slices.del(res);
                }        
            } while (res >= 0);
            
            return res;
        }

        idx getSum() {
            idx ret = 0;
            for(idx i = 0; i < slices.dim(); i++) {
                ret += slices[i].end - slices[i].start;
            }
            return ret;
        }

    protected:
        int addWorker(TimeSlice& slice) 
        {    

            if( slices.dim() <= 0) {
                *slices.add() = slice;
                return -1;
            }
            for(idx i = 0; i < slices.dim(); i++) {

                if( slices[i].start <= slice.start && slices[i].end >= slice.end ) {
                    return -1;
                }
                if(slices[i].end < slice.start) {
                    continue;
                }
                if(slices[i].start > slice.end) {
                    continue;
                }
                if( slices[i].start <= slice.start ) {
                    slices[i].end = slice.end;
                    return i;
                }
                else {
                    slices[i].start = slice.start;
                    return i;
                }
            }
            *slices.add() = slice;
            return -1;
        }
};

struct ProgressItem
{
        idx reqID, grpID, svcID, objID;
        idx takenTm, actTm, doneTm;
        idx stat, act;
        idx orderBefore, runningBefore;
        idx orderCnt, runningCnt;
        idx reportLevel, cntReportLevel;
        idx progress, progress100;
        idx waitTime;
        char parent[256];
        idx cnt;
        bool printed;

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
            objID = 0;
            printed = false;
            parent[0] ='\0';
        }
        ProgressItem(const char * l_parent, idx l_grpID, idx l_reqID, idx l_svcID, idx l_takenTm, idx l_actTm, idx l_doneTm, idx l_stat, idx l_act, idx l_progress, idx l_progress100, idx l_waitTime, idx l_orderBefore, idx l_runningBefore, idx l_reportLevel)
        {
            update(l_parent, l_grpID, l_reqID, l_svcID, l_takenTm, l_actTm, l_doneTm, l_stat, l_act, l_progress, l_progress100, l_waitTime, l_orderBefore, l_runningBefore, l_reportLevel);
        }

        ProgressItem * update(const char * l_parent, idx l_grpID, idx l_reqID, idx l_svcID, idx l_takenTm, idx l_actTm, idx l_doneTm, idx l_stat, idx l_act, idx l_progress, idx l_progress100, idx l_waitTime, idx l_orderBefore, idx l_runningBefore, idx l_reportLevel)
        {
            if(l_parent) {
                strncpy(parent, l_parent, sizeof(parent));
                parent[sizeof(parent) - 1] = '\0';
            }
            else {
                strncpy(parent, "none", sizeof(parent));
            }

            reqID = l_reqID;
            if( !grpID )
                grpID = l_grpID;
            svcID = l_svcID;

            if( orderBefore > l_orderBefore )
                orderBefore = l_orderBefore;
            if( runningBefore > l_runningBefore )
                runningBefore = l_runningBefore;


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

        void print(const char * title, sJSONPrinter& printer)
        {
            if( printed )
                return;
            printer.addKey("name");     printer.addValue(title);
            printer.addKey("parent");   printer.addValue(parent);
            printer.addKey("count");      (reqID > 0 && cnt > 0)? printer.addValue(cnt-1):printer.addValue(cnt);
            printer.addKey("reqID");    printer.addValue(reqID);
            printer.addKey("grpID");    printer.addValue(grpID);
            printer.addKey("objID");    printer.addValue(objID);
            printer.addKey("svcID");    printer.addValue(svcID);
            
            printer.addKey("status");
            switch(stat) {
                case sQPrideBase::eQPReqStatus_Any:
                    printer.addValue("Any"); break;
                case sQPrideBase::eQPReqStatus_Waiting:
                    printer.addValue("Waiting"); break;
                case sQPrideBase::eQPReqStatus_Processing:
                    printer.addValue("Processing"); break;
                case sQPrideBase::eQPReqStatus_Running:
                    printer.addValue("Running"); break;
                case sQPrideBase::eQPReqStatus_Suspended:
                    printer.addValue("Suspended"); break;
                case sQPrideBase::eQPReqStatus_Done:
                    printer.addValue("Done"); break;
                case sQPrideBase::eQPReqStatus_Killed:
                    printer.addValue("Killed"); break;
                case sQPrideBase::eQPReqStatus_ProgError:
                    printer.addValue("ProgError"); break;
                case sQPrideBase::eQPReqStatus_SysError:
                    printer.addValue("SysError"); break;

                case sQPrideBase::eQPReqStatus_Max:
                default:
                    printer.addValue("Unknown"); break;
            }

            printer.addKey("progress");     printer.addValue(progress);
            printer.addKey("progress100");  printer.addValue(progress100);
            printer.addKey("actTime");      printer.addValue(actTm);
            printer.addKey("takenTime");    printer.addValue(takenTm);
            printer.addKey("doneTime");     printer.addValue(doneTm);
            printer.addKey("waitTime");     printer.addValue(waitTime);
            printer.addKey("execTime");     printer.addValue(doneTm - (takenTm == sIdxMax ? doneTm : takenTm));
            printer.addKey("reportType");   printer.addValue(reportLevel);
            if( orderBefore ) {
                printer.addKey("orderExecute");    printer.addValue(orderBefore);
            }
            if( runningBefore ) {
                printer.addKey("runningBefore");    printer.addValue(runningBefore);
            }
            printer.addKey("act");    printer.addValue(act);
            printed = true;
        }
};

class TreeProgressItem {
    
    idx takenTm, actTm, doneTm;
    idx orderBefore, runningBefore;
    idx orderCnt, runningCnt;
    idx reportLevel, cntReportLevel;
    idx progress, progress100;
    idx waitTime;
    idx execTime;
    idx cnt;

    sQPrideBase::Request sr;
    sVec<TreeProgressItem> partList;
    sVec<TreeProgressItem> chList;
    sVec<sQPrideBase::QPLogMessage> messages; 

    static sQPrideBase::eQPReqStatus partStatusToPrint;
    static bool printParts; 
    static idx printMsg; 
    static sDic<idx> req2group; 

    public:
        bool setPrintParts(bool bIn = true) 
        {
            bool ret = printParts;
            printParts = bIn;
            return ret;
        }

        idx setPrintMsg(idx bIn = 0) 
        {
            idx ret = printMsg;
            printMsg = bIn;
            return ret;
        }

        sQPrideBase::eQPReqStatus setPartsFilter(sQPrideBase::eQPReqStatus nSt = sQPrideBase::eQPReqStatus_Any) 
        {
            sQPrideBase::eQPReqStatus ret = partStatusToPrint;
            partStatusToPrint = nSt;
            return ret;
        }

    public:
        TreeProgressItem() 
        {
            takenTm = sIdxMax;
            actTm = sIdxMax;
            doneTm = 0;
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
            execTime = 0;
        }

        TreeProgressItem(sQPrideBase::Request& r) : sr(r)
        {
            takenTm = sr.takenTm;
            actTm = sr.actTm;
            doneTm = sr.doneTm;
            progress = sr.progress;
            progress100 = sr.progress100;
            waitTime = sr.cdate ? ((sr.takenTm ? sr.takenTm : time(0)) - sr.cdate) : 0;
            if( sr.scheduleGrab ) {
                waitTime = 0;
                sr.takenTm = sr.cdate;
                sr.actTm = sr.cdate;
            }
            orderBefore = sIdxMax;
            runningBefore = sIdxMax;
            orderCnt = 0;
            runningCnt = 0;
            reportLevel = 0;
            cntReportLevel = 0;
            cnt = 0;
            execTime = 0;
        }

        TreeProgressItem(sQPrideBase::Request& r, sVec<sQPrideBase::QPLogMessage>* msgs ) : 
            sr(r)
        {
            if(msgs) {
                for(int i = 0; i < msgs->dim(); i++) {
                    sQPrideBase::QPLogMessage* val = messages.add();
                    *val = (*msgs)[i];
                    if( sr.maxLogErrLevel < val->level ) {
                        sr.maxLogErrLevel = val->level;
                    }
                }
            }
            takenTm = sr.takenTm;
            actTm = sr.actTm;
            doneTm = sr.doneTm;
            progress = sr.progress;
            progress100 = sr.progress100;
            waitTime = sr.cdate ? ((sr.takenTm ? sr.takenTm : time(0)) - sr.cdate) : 0;
            if( sr.scheduleGrab ) {
                waitTime = 0;
                sr.takenTm = sr.cdate;
                sr.actTm = sr.cdate;
            }
            orderBefore = sIdxMax;
            runningBefore = sIdxMax;
            orderCnt = 0;
            runningCnt = 0;
            reportLevel = 0;
            cntReportLevel = 0;
            cnt = 0;
            execTime = 0;
        }
        
        TreeProgressItem(sVariant& tqs_arg) {
            takenTm = sIdxMax;
            actTm = sIdxMax;
            doneTm = 0;
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
            execTime = 0;

            sVariant* dummy = tqs_arg.getDicElt("name");
            if( dummy && dummy->isString() ) {
                strncpy(sr.svcName, dummy->asString(), 128);
                sr.svcName[127] = '\0';
            }
            else {
                strncpy(sr.svcName, "unknown", 128);
            }

            setIdxValue("grpID",     sr.grpID,          tqs_arg); 
            setIdxValue("reqID",     sr.reqID,          tqs_arg); 
            setIdxValue("objID",     sr.objID,          tqs_arg); 
            setIdxValue("svcID",     sr.svcID,          tqs_arg); 

            setSeverity(tqs_arg.getDicElt("severity"));

            setStatus(tqs_arg.getDicElt("status"));

            setIdxValue("progress",    progress,    tqs_arg); 
            setIdxValue("progress100", progress100, tqs_arg); 
            setIdxValue("actTime",     actTm,       tqs_arg); 
            setIdxValue("takenTime",   takenTm,     tqs_arg); 
            setIdxValue("doneTime",    doneTm,      tqs_arg); 
            setIdxValue("waitTime",    waitTime,    tqs_arg); 
            setIdxValue("execTime",    execTime,    tqs_arg); 

            setIdxValue("orderExecute", orderBefore,  tqs_arg); 
            setIdxValue("runningBefore",runningBefore,tqs_arg); 
            
            setIdxValue("act", sr.act, tqs_arg); 

            readArray(tqs_arg.getDicElt("messages"), messages); 
            readArray(tqs_arg.getDicElt("parts"),    partList); 
            readArray(tqs_arg.getDicElt("children"), chList);

        }

        TreeProgressItem& operator = (const TreeProgressItem &in) {
            sr = in.sr;
            takenTm = in.takenTm;
            actTm = in.actTm;
            doneTm = in.doneTm;
            progress = in.progress;
            progress100 = in.progress100;
            waitTime = in.waitTime;
            orderBefore = in.orderBefore;
            runningBefore = in.runningBefore;
            orderCnt = in.orderCnt;
            runningCnt = in.runningCnt;
            reportLevel = in.reportLevel;
            cntReportLevel = in.cntReportLevel;
            cnt = in.cnt;
            execTime = in.execTime;

            for(int i = 0; i < in.messages.dim(); i++) {
                sQPrideBase::QPLogMessage* val = messages.add();
                *val = in.messages[i];
            }
            
            return *this; 
        } 

        idx getSvcID() { return sr.svcID; }
        idx getReqID() { return sr.reqID; }

        TreeProgressItem* getChildByReqID( idx reqID ) 
        {
            for(idx i = 0; i < chList.dim(); ++i) {
                if(chList[i].getReqID() == reqID) {
                    return chList.ptr(i);
                }
                TreeProgressItem* ret = chList[i].getChildByReqID(reqID);
                if( ret )
                    return ret;
            }
            return NULL;
        }

        void processParallelParts(sVec<sQPrideBase::Request>& rList, idx& index, sDic < sVec<sQPrideBase::QPLogMessage> >& infos) 
        {
            int partIndex = 0;
            idx prevPar = 0;
            while ( index < rList.dim() && rList.ptr(index)->inParallel >= prevPar && rList.ptr(index)->svcID == sr.svcID )
            {
                sQPrideBase::Request* r = rList.ptr(index++);
                prevPar = r->inParallel;
                TreeProgressItem svc_item(*r, infos.get(&r->reqID, sizeof(idx)));
                if( svc_item.sr.maxLogErrLevel > sr.maxLogErrLevel) {
                    sr.maxLogErrLevel = svc_item.sr.maxLogErrLevel;
                }
                partList.add();                    
                partList[partIndex++] = svc_item;
            }
            if(partIndex>0) {
                index--;
            }

            calcProgress();
        }

        void setLogLevel(TreeProgressItem& svc_item, sDic<sQPrideBase::QPLogMessage>& infos) {
            sQPrideBase::QPLogMessage* msg = infos.get(&svc_item.sr.reqID, sizeof(idx));
            if( msg ) {
                if( msg->level > reportLevel ) {
                    svc_item.reportLevel = (enum sQPrideBase::eQPInfoLevel) msg->level;
                }
            }
        }

        void skipDiplayParts(sVec<sQPrideBase::Request>& rList, idx& index)
        {
            idx prevPar = 1;
            while ( (index+1) < rList.dim() && rList.ptr(index+1)->inParallel > prevPar && rList.ptr(index+1)->svcID != sr.svcID )
            {
                index++;
                prevPar++;                               
            }
        }

        void processGroup( sVec<sQPrideBase::Request>& rList, idx& index, sDic < sVec<sQPrideBase::QPLogMessage> >& infos, sQPrideBase* base) 
        {
            int chIndex = 0;
            for(; index < rList.dim(); ++index)
            {
                sQPrideBase::Request* r = rList.ptr(index);
                if(r->grpID == sr.reqID) 
                {
                    if(r->inParallel > 0 && r->reqID != r->grpID && r->svcID == sr.svcID) {
                        skipDiplayParts(rList, index);
                        continue;
                    }

                    if( r->reqID == r->grpID && 
                        index+1 < rList.dim() && 
                        rList[index+1].grpID == r->reqID && 
                        rList[index+1].inParallel > 0) {
                            processParallelParts(rList, index, infos);
                            continue;
                    }

                    TreeProgressItem svc_item(*r, infos.get(&r->reqID, sizeof(idx)));



                    if(sr.stat <= sQPrideBase::eQPReqStatus_Done) {
                        if(sr.stat == sQPrideBase::eQPReqStatus_Done && sr.stat > r->stat) 
                            sr.stat = r->stat;

                        if(r->stat > sQPrideBase::eQPReqStatus_Done )
                            sr.stat = r->stat;
                    }

                    chList.add();                    
                    chList[chIndex++] = svc_item;
                    
                    skipDiplayParts(rList, index);
                    continue;
                }
                else
                {
                    TreeProgressItem* child = getChildByReqID(r->grpID);
                    if (child) {
                        child->processGroup(rList, index, infos, base);
                        index--;
                        
                        if(child->chList.dim() > 0) {
                            for(idx k = child->chList.dim()-1; k >= 0; k--) {
                                idx req = child->chList[k].sr.reqID;
                                for(idx k = chList.dim()-1; k >= 0; k--) {
                                    if(chList[k].sr.reqID < req)
                                        break;
                                    else
                                    if(chList[k].sr.reqID == req && chList[k].chList.dim() <= 0) {
                                        chList.del(k);
                                        break;
                                    }
                                }
                            }
                        }
                        
                        continue;
                    }
                    else {
                        break;
                    }
                }
                break;
            } 
            if(chList.dim() > 0) {
                int progressSum = 0;
                for(int i = 0; i < chList.dim(); i++) {
                    progressSum += chList[i].progress100;
                }
                progress100 = progressSum/chList.dim();
            }
        }

        void cleanUpChildList()
        {
            if(chList.dim() > 0) {
                for(idx k = chList.dim()-1; k >= 0; k--) {
                    if( chList[k].chList.dim() <= 0 ) {
                        idx* grp = TreeProgressItem::req2group.get(&chList[k].sr.reqID, sizeof(chList[k].sr.reqID));
                        if (grp == NULL || *grp == chList[k].sr.grpID)
                            continue;
                        chList.del(k);
                    }
                }
            }
        }

        void cleanUpChildList2()
        {
            if(chList.dim() > 0) {
                for(idx k = chList.dim()-1; k >= 0; k--) {
                    if( chList[k].chList.dim() <= 0 ) {
                        TreeProgressItem* child = getChildByReqID(chList[k].getReqID());
                        if( child != NULL ) {
                            if(child->sr.grpID != chList[k].sr.grpID) {
                                chList.del(k);
                            }
                        }
                    }
                }
            }
        }

    protected:
        void setIdxValue(const char* name, idx& value, sVariant& tqs_arg)
        {
            sVariant* dummy = tqs_arg.getDicElt(name);
            if(dummy && dummy->isInt())  
                value = dummy->asInt();
        }

        idx setSeverity(sVariant* varIn)
        {
            sr.maxLogErrLevel = -1;
            if(varIn && varIn->isString()) {
                const char* level = varIn->asString(); 
                if( !level ) return sr.maxLogErrLevel;
                else if( !strncmp(level, "Trace",     5) ) sr.maxLogErrLevel = sQPrideBase::eQPInfoLevel_Trace;
                else if( !strncmp(level, "Debug",     5) ) sr.maxLogErrLevel = sQPrideBase::eQPInfoLevel_Debug;
                else if( !strncmp(level, "Info",      4) ) sr.maxLogErrLevel = sQPrideBase::eQPInfoLevel_Info;
                else if( !strncmp(level, "Warning",   7) ) sr.maxLogErrLevel = sQPrideBase::eQPInfoLevel_Warning;
                else if( !strncmp(level, "Error",     5) ) sr.maxLogErrLevel = sQPrideBase::eQPInfoLevel_Error;
                else if( !strncmp(level, "Fatal",     5) ) sr.maxLogErrLevel = sQPrideBase::eQPLogType_Fatal;
            }
            return sr.maxLogErrLevel;
        }

        idx setStatus(sVariant* varIn) {
            sr.stat = -1;
            if(varIn && varIn->isString()) {
                const char* status = varIn->asString();
                if( !status ) return sr.stat;
                else if( !strncmp(status, "Any",       3) ) sr.stat = sQPrideBase::eQPReqStatus_Any;
                else if( !strncmp(status, "Waiting",   7) ) sr.stat = sQPrideBase::eQPReqStatus_Waiting;
                else if( !strncmp(status, "Processing", 10) ) sr.stat = sQPrideBase::eQPReqStatus_Processing;
                else if( !strncmp(status, "Running",   7) ) sr.stat = sQPrideBase::eQPReqStatus_Running;
                else if( !strncmp(status, "Suspended", 9) ) sr.stat = sQPrideBase::eQPReqStatus_Suspended;
                else if( !strncmp(status, "Done",      4) ) sr.stat = sQPrideBase::eQPReqStatus_Done;
                else if( !strncmp(status, "Killed",    6) ) sr.stat = sQPrideBase::eQPReqStatus_Killed;
                else if( !strncmp(status, "ProgError", 9) ) sr.stat = sQPrideBase::eQPReqStatus_ProgError;
                else if( !strncmp(status, "SysError",  8) ) sr.stat = sQPrideBase::eQPReqStatus_SysError;
            }
            return sr.stat;
        }

        void readArray(sVariant* arrayIn, sVec<sQPrideBase::QPLogMessage>& dest) 
        {
            if(!arrayIn) return;
            if( arrayIn->isList() ) {
                for(idx i=0; i < arrayIn->dim(); i++) {
                    sVariant* partElm = arrayIn->getListElt(i);
                    if(partElm) {
                        sQPrideBase::QPLogMessage* dummy = dest.add();
                        dummy->req = this->sr.reqID;
                        dummy->init(*partElm);
                    }
                }
            }
        }

        void readArray(sVariant* arrayIn, sVec<TreeProgressItem>& dest) 
        {
            if(!arrayIn) return;
            if( arrayIn->isList() ) {
                for(idx i=0; i < arrayIn->dim(); i++) {
                    sVariant* partElm = arrayIn->getListElt(i);
                    TreeProgressItem dummy(*partElm);
                    dest.add();                    
                    dest[i] = dummy;
                }
            }
        }
        void printHead(sJSONPrinter& printer)
        {
            printer.addKey("name");      printer.addValue(sr.svcName); 
            printer.addKey("grpID");     printer.addValue(sr.grpID);
            printer.addKey("reqID");     printer.addValue(sr.reqID);
            printer.addKey("objID");     printer.addValue(sr.objID);
            printer.addKey("svcID");     printer.addValue(sr.svcID);
            if( sr.maxLogErrLevel > 0) {
                printer.addKey("severity"); printer.addValue( sQPrideBase::getLevelName(sr.maxLogErrLevel) );
            }
            printer.addKey("childCount");   printer.addValue(chList.dim());
            printer.addKey("pieces");       printer.addValue(partList.dim());

            if( sr.stat > 0 && sr.stat < sQPrideBase::eQPReqStatus_Max ) {
                printer.addKey("status");
                switch( sr.stat ) {
                    case sQPrideBase::eQPReqStatus_Waiting:
                        printer.addValue("Waiting"); break;
                    case sQPrideBase::eQPReqStatus_Processing:
                        printer.addValue("Processing"); break;
                    case sQPrideBase::eQPReqStatus_Running:
                        printer.addValue("Running"); break;
                    case sQPrideBase::eQPReqStatus_Suspended:
                        printer.addValue("Suspended"); break;
                    case sQPrideBase::eQPReqStatus_Done:
                        printer.addValue("Done"); break;
                    case sQPrideBase::eQPReqStatus_Killed:
                        printer.addValue("Killed"); break;
                    case sQPrideBase::eQPReqStatus_ProgError:
                        printer.addValue("ProgError"); break;
                    case sQPrideBase::eQPReqStatus_SysError:
                        printer.addValue("SysError"); break;
                    case sQPrideBase::eQPReqStatus_Max:
                    default:
                        printer.addValue("Unknown"); break;
                }
            }
            printer.addKey("progress");     printer.addValue(progress);
            printer.addKey("progress100");  printer.addValue(progress100);
            printer.addKey("actTime");      printer.addValue(actTm);
            printer.addKey("takenTime");    printer.addValue(takenTm);
            printer.addKey("doneTime");     printer.addValue(doneTm );
            printer.addKey("waitTime");     printer.addValue(waitTime);

            if(doneTm == 0) 
                doneTm = time( NULL );

            if(execTime) {
                printer.addKey("execTime");     printer.addValue(execTime);
            }
            else {
                printer.addKey("execTime");     printer.addValue(doneTm - (takenTm == 0? doneTm : takenTm));
            }
            printer.addKey("reportType");   printer.addValue( sQPrideBase::getLevelName(reportLevel) );
            printer.addKey("act");    printer.addValue(sr.act);
        }

        void printPrts(sJSONPrinter& printer, idx start=0, idx cnt= LONG_MAX) 
        {
            if(partList.dim() > 0) {
                
                if( start != 0 ) {
                    printer.addKey("pStart");     printer.addValue(start);
                    printer.addKey("pCount");     printer.addValue(cnt);
                }
                
                printer.addKey("parts");
                printer.startArray();
                for(idx k = 0, j = 0; k < partList.dim(); ++k) {
                    if( partStatusToPrint == sQPrideBase::eQPReqStatus_Any ) {
                        if (j >= start)  {
                            partList[k].print(printer);
                            cnt--;
                        }
                        j++;
                    }
                    else
                    if( partList[k].sr.stat == partStatusToPrint )
                    {
                        if (j >= start)  {
                            partList[k].print(printer);
                            cnt--;
                        }
                        j++;
                    }
                    
                    if(cnt <= 0)
                        break;
                }
                printer.endArray();
            }
        }

        void printMessages(sJSONPrinter & printer)
        {
            if( printMsg && messages.dim() ) {
                printer.addKey("messages");
                printer.startArray();
                for( idx k = 0; k < messages.dim(); ++k ) {
#if _DEBUG
                    if( true ) {
#else
                    if( messages[k].level >= sQPrideBase::eQPInfoLevel_Min ) {
#endif
                        if( messages[k].level >= printMsg ) {
                            messages[k].print(printer);
                        }
                    }
                }
                printer.endArray();
            }
        }

    public:
        void printPartsOnly(sJSONPrinter & printer, idx start, idx cnt)
        {
            printer.startObject();
            printHead(printer);
            if( partList.dim() <= 0 ) {
                printMessages(printer);
            }
            printPrts(printer, start, cnt);
            printer.endObject();
        }

        void print(sJSONPrinter& printer)
        {
            printer.startObject();

            printHead( printer );
            
            if(partList.dim() <= 0) {
                printMessages( printer );
            }
            if(printParts) {
                printPrts( printer );
            }
            if(chList.dim() > 0) {
                printer.addKey("children");
                printer.startArray();
                for(idx k = 0; k < chList.dim(); ++k) {
                    chList[k].print(printer);
                }
                printer.endArray();
            }

            printer.endObject();
        }

        void calcProgress() 
        {
            idx calcProgress    = 0;
            idx calcProgress100 = 0;
            TSAggregator pExecTime;

            if(partList.dim() > 0) {
                for(idx k = 0; k < partList.dim(); ++k) {

                    if( sr.stat >= sQPrideBase::eQPReqStatus_Done ) {
                        if( ( sr.stat > partList[k].sr.stat && partList[k].sr.stat < sQPrideBase::eQPReqStatus_Done ) || 
                            ( sr.stat < partList[k].sr.stat && partList[k].sr.stat >= sQPrideBase::eQPReqStatus_Done) || 
                            sr.stat == sQPrideBase::eQPReqStatus_Max ) {
                            sr.stat = partList[k].sr.stat;
                        }
                    }
                    else if( partList[k].sr.stat < sQPrideBase::eQPReqStatus_Suspended && sr.stat < partList[k].sr.stat ) {
                            sr.stat = partList[k].sr.stat;
                    }

                    calcProgress    += partList[k].progress;
                    calcProgress100 += partList[k].progress100;
                    
                    if(doneTm < partList[k].doneTm) {  
                        doneTm = partList[k].doneTm;
                    }
                    if( takenTm >= partList[k].takenTm && partList[k].takenTm > 0 ) { 
                        takenTm = partList[k].takenTm;
                    }
                    idx partDoneTm = partList[k].doneTm==0? time(NULL) : partList[k].doneTm;

                    pExecTime.add(partList[k].takenTm==0 ? partDoneTm : partList[k].takenTm, partDoneTm);
                }
                progress    = calcProgress;
                progress100 = calcProgress100/partList.dim();
                execTime = pExecTime.getSum();
            }
        }
};

sDic<idx> TreeProgressItem::req2group;
bool TreeProgressItem::printParts = false;
idx TreeProgressItem::printMsg = 0;
sQPrideBase::eQPReqStatus TreeProgressItem::partStatusToPrint = sQPrideBase::eQPReqStatus_Any;

 
void sQPrideBase::getAllReqInfoOldStyle(sDic<ProgressItem>& prgList, sVec<Request>& rList, idx minLevelInfo) 
{
    sVec<QPLogMessage> infos;
    grpGetInfo(rList[0].grpID, minLevelInfo, infos);
    sDic<idx> pReqs;

    idx now = time(0);
    sStr tmp;
    for(idx i = 0; i < rList.dim(); ++i) {
        Request * sr = rList.ptr(i);

        if( pReqs.find( &(sr->reqID), sizeof(sr->reqID) ) <= 0 ) {
            *pReqs.set(&sr->reqID, sizeof(sr->reqID)) = sr->grpID;
        }
        else {
            continue;
        }

        ProgressItem & svc_item = prgList[tmp.printf(0, "__svc-%s", sr->svcName)];
        if( svc_item.orderBefore == sIdxMax) {
            svc_item.orderCnt = sysPeekReqOrder(sr->reqID, 0, &svc_item.runningCnt);
        }
        if( sr->stat == sQPrideBase::eQPReqStatus_Waiting ) {
            sr->progress100 = 0;
            ++svc_item.orderCnt;
        }
        idx runningBefore = svc_item.runningCnt;
        idx orderBefore = svc_item.orderCnt;
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
        svc_item.update("Total Progress", sr->grpID, 0, sr->svcID, 
                         sr->takenTm, sr->actTm, sr->doneTm, sr->stat, sr->act, 
                         sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);
        prgList[tmp.printf(0, "__req-%" DEC, sr->reqID)].update(sr->svcName, sr->grpID, sr->reqID, 
                                                                sr->svcID, sr->takenTm, sr->actTm, sr->doneTm, sr->stat, 
                                                                sr->act, sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);
        prgList["__root"].update("root", sr->grpID, rList.dim() == 1 ? sr->reqID : 0 , 
                                    0, sr->takenTm, sr->actTm, sr->doneTm, sr->stat, 
                                    sr->act, sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);
    }
}


void sQPrideBase::getAllReqInfo(sDic<ProgressItem>& prgList, sVec<Request>& rList, idx minLevelInfo) 
{
    sVec<QPLogMessage> infos;
    grpGetInfo(rList[0].grpID, minLevelInfo, infos);

    ProgressItem rootItem;

    idx now = time(0);
    sStr tmp;
    for(idx i = 0; i < rList.dim(); ++i) {
        Request * sr = rList.ptr(i);
        ProgressItem & svc_item = prgList[tmp.printf(0, "__svc-%s", sr->svcName)];
        if( svc_item.orderBefore == sIdxMax) {
            svc_item.orderCnt = sysPeekReqOrder(sr->reqID, 0, &svc_item.runningCnt);
        }
        if( sr->stat == sQPrideBase::eQPReqStatus_Waiting ) {
            sr->progress100 = 0;
            ++svc_item.orderCnt;
        }
        idx runningBefore = svc_item.runningCnt;
        idx orderBefore = svc_item.orderCnt;
        if( sr->takenTm && ( !sr->doneTm || sr->doneTm < sr->takenTm) ) {
            sr->doneTm = now;
        }
        idx waitTime = sr->cdate ? ((sr->takenTm ? sr->takenTm : now) - sr->cdate) : 0;
        if( sr->scheduleGrab ) {
            waitTime = 0;
            sr->takenTm = sr->cdate;
            sr->actTm = sr->cdate;
        }
        
        if(sr->objID != 0) svc_item.objID = sr->objID;

        idx reportLevel = 0;
        for(idx l = 0; l < infos.dim(); ++l) {
            if( sr->reqID == infos[l].req && infos[l].level > reportLevel ) {
                reportLevel = (enum eQPInfoLevel) infos[l].level;
            }
        }

        rootItem.update("root", sr->grpID, rList.dim() == 1 ? sr->reqID : 0 , 
                                    0, sr->takenTm, sr->actTm, sr->doneTm, sr->stat, 
                                    sr->act, sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);

        tmp.printf(0, "__req-%" DEC, sr->grpID);
        svc_item.update( ( sr->grpID == sr->reqID )? "Total Progress" : prgList[tmp.ptr()].parent, sr->grpID, sr->reqID, sr->svcID, 
                        sr->takenTm, sr->actTm, sr->doneTm, sr->stat, sr->act, 
                        sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);
        svc_item.cnt = sr->chldCnt;

        prgList[tmp.printf(0, "__req-%" DEC, sr->reqID)].update(sr->svcName, sr->grpID, sr->reqID, 
                                 sr->svcID, sr->takenTm, sr->actTm, sr->doneTm, sr->stat, 
                                 sr->act, sr->progress, sr->progress100, waitTime, orderBefore, runningBefore, reportLevel);        
    }
    prgList["__root"] = rootItem;
}

void sQPrideBase::fillGroupAndList(sStr * group, sStr * list, idx start, idx cnt, sDic<idx> * svcIDs, sDic<ProgressItem>& prgList) 
{
    if( group ) {
        prgList["__root"].print("Total Progress", group);
        for(idx id = 0; id < prgList.dim(); ++id) {
            const char * key = (const char *) prgList.id(id);
            if( sIs("__svc-", key) ) {
                prgList[id].print(key + 6, group);
            }
        }
    }
    if( list ) {
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
    }
}

idx sQPrideBase::reqProgressReport2(sStr * group, sStr * list, idx req, idx start, idx cnt, idx minLevelInfo, const char * svcName, sDic<idx> * svcIDs)
{
    if( !cnt ) {
        cnt = sIdxMax;
    }
    sVec<Request> rList(sMex::fSetZero);
    requestGetForGrp2 (req, &rList, svcName);
    if( !rList.dim() && !requestGet(req, rList.add()) ) {
        return 0; 
    }
    if( rList.dim() == 1 ) {
        rList[0].grpID = rList[0].reqID;
    }

    (group ? group : list)->printf("name,parent,cnt,reqID,grpID,svcID,stat,progress,progress100,actTime,takenTime,doneTime,waitTime,execTime,reportType,orderExecute,runningBefore,act\n");
    
    sDic<ProgressItem> prgList;
    getAllReqInfo(prgList, rList, minLevelInfo);

    fillGroupAndList(group, list, start, cnt, svcIDs, prgList);

    return rList.dim();
}

const char * const obj_progress_json_name = ".progress.json";

idx sQPrideBase::reqProgressReport2(sJSONPrinter& printer, idx req, bool showReqs, idx showMsg, sUsrObj* obj)
{
    sVec<Request> rList(sMex::fSetZero);
    requestGetForGrp2 (req, &rList, 0);
    if( !rList.dim() && !requestGet(req, rList.add()) ) {
        if( obj ) {
            sStr dest;
            obj->getFilePathname(dest, obj_progress_json_name);

            sFil dest_fil(dest, sMex::fExactSize | sMex::fReadonly);
            if( !dest_fil.ok() ) {
                return 0;
            }

            sJSONParser json_parser;
            json_parser.parse(dest_fil.ptr(), dest_fil.length()); 
            sVariant * head = json_parser.result().getDicElt("Head"); 
            if( !head ) return 0;

            TreeProgressItem tree(*head);
            tree.setPrintParts(showReqs);
            tree.setPrintMsg(showMsg);

            printer.startObject();
            printer.addKey("Head");
            tree.print(printer);
            printer.finish();
            return 1;
        }
        return 0; 
    }

    sDic < sVec<QPLogMessage> > info;
    if(showMsg || !rList[0].grpID) { 
        sVec<QPLogMessage> infos;
        if(rList[0].grpID) {
            grpGetInfo2(rList[0].grpID, eQPIPStatus_Any, infos);
        }
        else {
            getLog(rList[0].reqID, false, 0, 0, infos);
        }

        for(idx i=0; i< infos.dim(); i++) {
            idx index = infos[i].req;

            sVec<QPLogMessage> *dummy = info.set( &index, sizeof(idx) );
            if(dummy) {
                for(; i<infos.dim() && infos[i].req == index; i++) {
                    if(dummy->dim() > 0) {
                        if( (*dummy)[dummy->dim()-1] == infos[i] )
                            continue; 
                    }
                    QPLogMessage* val = dummy->add();
                    if(val) {
                        val->init(infos[i].req, infos[i].job, infos[i].level, infos[i].cdate, infos[i].message());
                    }
                } 
            }
        }
    }
    TreeProgressItem tree(rList[0] , info.get(&rList[0].reqID, sizeof(idx)) );
    tree.setPrintParts(showReqs);
    tree.setPrintMsg(showMsg);

    idx index = rList[0].chldCnt && rList.dim() > 1 &&  rList[1].inParallel  ? 0 : 1;

    tree.processGroup(rList, index, info, this);
    
    printer.startObject();
    printer.addKey("Head");
    tree.print(printer);
    printer.finish();

    return rList.dim();
}

idx sQPrideBase::reqProgressReportReqOnly(sJSONPrinter & printer, idx req, idx showMsg, idx start, idx cnt, idx status, sUsrObj * obj)
{
    sVec<Request> rList(sMex::fSetZero);
    requestGetForGrp2(req, &rList, 0);
    if( !rList.dim() && !requestGet(req, rList.add()) ) {
        if( obj ) {
            sStr dest;
            obj->getFilePathname(dest, obj_progress_json_name);
            sFil dest_fil(dest, sMex::fExactSize | sMex::fReadonly);
            if( dest_fil.ok() ) {
                sJSONParser json_parser;
                json_parser.parse(dest_fil.ptr(), dest_fil.length());
                sVariant * head = json_parser.result().getDicElt("Head");
                if( head ) {
                    TreeProgressItem tree(*head);
                    tree.setPartsFilter((sQPrideBase::eQPReqStatus)status);
                    tree.setPrintMsg(showMsg);
                    printer.startObject();
                    printer.addKey("Head");
                    tree.printPartsOnly(printer, start, cnt);
                    printer.finish();
                    return 1;
                }
            }
        }
        return 0;
    }
    sDic<sVec<QPLogMessage>> info;
    if( showMsg ) {
        sVec<QPLogMessage> msgs;
        if( rList[0].grpID ) {
            grpGetInfo2(rList[0].grpID, 0, msgs);
        } else {
            getLog(rList[0].reqID, false, 0, 0, msgs);
        }
        for( idx i = 0; i < msgs.dim(); ++i ) {
            if( !info.get(&msgs[i].req, sizeof(msgs[i].req)) ) {
                info.set(&msgs[i].req, sizeof(msgs[i].req));
            }
        }
        for( idx i = 0; i < msgs.dim(); ++i ) {
            sVec<QPLogMessage> * m = info.get(&msgs[i].req, sizeof(msgs[i].req));
            QPLogMessage * log = m->add();
            if( log ) {
                log->init(msgs[i].req, msgs[i].job, msgs[i].level, msgs[i].cdate, msgs[i].message());
            }
        }
    }
    TreeProgressItem tree(rList[0], info.get(&rList[0].reqID, sizeof(idx)));
    tree.setPartsFilter((sQPrideBase::eQPReqStatus)status);
    tree.setPrintMsg(showMsg);
    idx index = 0;
    tree.processGroup(rList, index, info, this);
    printer.startObject();
    printer.addKey("Head");
    tree.printPartsOnly(printer, start, cnt);
    printer.finish();
    return rList.dim();
}

void sQPrideBase::saveProgress(idx req, sUsrObj& obj)
{
    sVec<Request> rList(sMex::fSetZero);
    requestGetForGrp2 (req, &rList, 0);
    if( !rList.dim() && !requestGet(req, rList.add()) ) {
        return; 
    }

    sVec<QPLogMessage> infos;
    if(rList[0].grpID) {
        grpGetInfo2(rList[0].grpID, eQPIPStatus_Any, infos);
    }
    else {
        getLog(rList[0].reqID, false, 0, 0, infos);
    }
    sDic < sVec<QPLogMessage> > info;

    for(idx i=0; i< infos.dim(); i++) {
        idx index = infos[i].req;

        sVec<QPLogMessage> *dummy = info.set( &index, sizeof(idx) );
        if(dummy) { 
            for(;i < infos.dim() && infos[i].req == index; i++) {
                if(dummy->dim() > 0) {
                    if( (*dummy)[dummy->dim()-1] == infos[i] ) {
                        continue; 
                    }
                }
                QPLogMessage* val = dummy->add();
                if(val) {
                    *val = infos[i];
                }
            } 
        }
    }

    TreeProgressItem tree(rList[0], info.get(&rList[0].reqID, sizeof(idx)));
    tree.setPrintParts(true);
    tree.setPrintMsg( 1 );

    idx index = 0;
    tree.processGroup(rList, index, info, this);
    tree.cleanUpChildList();

    sStr dest;
    obj.addFilePathname(dest, true, obj_progress_json_name);

    sFil dest_fil(dest);
    if( !dest_fil.ok() ) {
        sFile::remove(dest_fil);
        return;
    }

    sJSONPrinter printer(&dest_fil
#if !_DEBUG
        ,"", ""
#endif
    );
    printer.startObject();
    printer.addKey("Head");
    tree.print(printer);
    printer.finish();
}
idx sQPrideBase::reqProgressReport(sStr * group, sStr * list, idx req, idx start, idx cnt, idx minLevelInfo, const char * svcName, sDic<idx> * svcIDs)
{
    if( !cnt ) {
        cnt = sIdxMax;
    }
    sVec<Request> rList(sMex::fSetZero);
    requestGetForGrp2(req, &rList, svcName);

    if( !rList.dim() && !requestGet(req, rList.add()) ) {
        return 0;
    }
    if( rList.dim() == 1 ) {
        rList[0].grpID = rList[0].reqID;
    }
    sVec<QPLogMessage> infos;
    grpGetInfo(rList[0].grpID, minLevelInfo, infos);

    (group ? group : list)->printf("name,parent,cnt,reqID,grpID,svcID,stat,progress,progress100,actTime,takenTime,doneTime,waitTime,execTime,reportType,orderExecute,runningBefore,act\n");

    sDic<ProgressItem> prgList;
    getAllReqInfoOldStyle(prgList, rList, minLevelInfo);

    fillGroupAndList(group, list, start, cnt, svcIDs, prgList);
    
    return rList.dim();
}


