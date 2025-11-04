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
#include <slib/utils/cron.hpp>
#include <ulib/uusage.hpp>
#include <qlib/QPrideProc.hpp>

using namespace slib;

class Housekeeper: public sQPrideProc
{
    private:
        static const char * _lock_path;

        void updateUsage(idx req, sUsrUsage2 * usage_obj);
        void purge(idx req, sUsrUsage2 * usage_obj);
        bool lock(idx req);
        bool unlock(idx req);

    public:
        Housekeeper(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }

        virtual idx OnExecute(idx req);
};

const char * Housekeeper::_lock_path = "QPLock://housekeeper";

bool Housekeeper::lock(idx req)
{
    idx lock_req;
    if( reqLock(_lock_path, &lock_req) ) {
        logOut(sQPrideBase::eQPLogType_Trace, "Took lock %s\n", _lock_path);
        return true;
    } else {
        logOut(sQPrideBase::eQPLogType_Debug, "Lock %s is held by req %" UDEC "\n", _lock_path, lock_req);
        reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Another housekeeper (req %" DEC ") is still running", lock_req);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return false;
    }
}

bool Housekeeper::unlock(idx req)
{
    if( reqUnlock(_lock_path) ) {
        logOut(sQPrideBase::eQPLogType_Trace, "Released lock %s\n", _lock_path);
        return true;
    } else {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to release lock %s\n", _lock_path);
        return false;
    }
}

idx Housekeeper::OnExecute(idx req)
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

    sUsr * requester = user;
    user = &superuser;

    if( !lock(req) ) {
        user = requester;
        return 0;
    }

    sRC rc;
    sUsrUsage2 * usage_obj = sUsrUsage2::ensureObj(*user, &rc);
    if( !usage_obj ) {
        unlock(req);
        logOut(sQPrideBase::eQPLogType_Error, "Failed to load/create sUsrUsage2 special object: %s\n", rc.print());
        reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Failed to load/create object");
        reqSetStatus(req, eQPReqStatus_ProgError);
        user = requester;
        return 0;
    }

    logOut(sQPrideBase::eQPLogType_Debug, "Loaded sUsrUsage2 special object %s\n", usage_obj->Id().print());

    const char * cmd = pForm->value("cmd");
    if( !cmd ) {
        cmd = vars.value("form.cmd");
    }
    if( sIs(cmd, "update-usage") ) {
        updateUsage(req, usage_obj);
    } else if( sIs(cmd, "purge") ) {
        purge(req, usage_obj);
    } else {
        logOut(sQPrideBase::eQPLogType_Error, "Unknown command: \"%s\"\n", cmd ? cmd : "");
    }

    unlock(req);

    delete usage_obj;
    user = requester;
    return 0;
}

void Housekeeper::updateUsage(idx req, sUsrUsage2 * usage_obj)
{
    const char * every = 0;
    sCronTime cron_every;
    sVariant time_val_formatter;
    sStr buf;

    if( (every = pForm->value("every")) || (every = vars.value("form.every")) ) {
        if( !cron_every.parse(every) ) {
            reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Invalid 'every' parameter");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return;
        }
    }

    time_t at_time = 0;
    const char * t = 0;
    if( (t = pForm->value("time")) || (t = vars.value("form.time")) ) {
        time_val_formatter.setDateTime(t);
        at_time = time_val_formatter.asDateTime();
    } else if( !every ) {
        at_time = time(0);
    }

    sVec<time_t> times;

    if( every ) {
        time_t until_time = at_time ? at_time : time(0);
        if( time_t from_time = usage_obj->getLastUpdateTime() ) {
            for(time_t cron_time = cron_every.nextMatch(from_time); cron_time && cron_time < until_time; cron_time = cron_every.nextMatch(cron_time)) {
                *times.add(1) = cron_time;
            }
        } else {
            *times.add(1) = cron_every.matches(until_time) ? until_time : cron_every.prevMatch(until_time);
        }

        if( at_time && (times.dim() == 0 || times[times.dim() - 1] != at_time) ) {
            *times.add(1) = at_time;
        }
    } else if( at_time ) {
        *times.add(1) = at_time;
    } else {
        *times.add(1) = time(0);
    }

    for(idx i = 0; i < times.dim(); i++) {
        buf.cut0cut();
        time_val_formatter.setDateTime(times[i]);
        logOut(sQPrideBase::eQPLogType_Debug, "Will run sUsrUsage2::updateIncremental() at time %s: #%" DEC "/%" DEC "\n", time_val_formatter.print(buf, sVariant::eUnquoted), i + 1, times.dim());
        if( sRC rc = usage_obj->updateIncremental(reqProgressStatic, this, times[i], true) ) {
            logOut(sQPrideBase::eQPLogType_Error, "Failed to update usage: %s\n", rc.print());
            reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Failed to update usage");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return;
        }
    }

    reqSetStatus(req, eQPReqStatus_Done);
}

void Housekeeper::purge(idx req, sUsrUsage2 * usage_obj)
{
    const idx obj_purge_limit = sClamp<idx>(cfgInt(0, "qm.purgeObjectLimit", 1000), 0, sIdxMax, 1000);
    const idx obj_purge_days = sClamp<idx>(cfgInt(0, "qm.ObjectExpireDays", 30), 0, sIdxMax, 30);
    const idx req_purge_limit = sClamp<idx>(cfgInt(0, "qm.purgeReqLimit", 1000), 0, sIdxMax, 1000);

    sVec<sUsrHousekeeper::PurgedObj> old_objs;
    sVec<sUsrHousekeeper::PurgedReq> old_reqs;

    sRC overall_rc;
    if( sRC rc = sUsrHousekeeper::findObjsForPurge(*user, old_objs, obj_purge_limit, obj_purge_days) ) {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to find objects for purge: %s\n", rc.print());
        overall_rc = rc;
    }
    if( sRC rc = sUsrHousekeeper::findReqsForPurge(*user, old_reqs, req_purge_limit) ) {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to find requests for purge: %s\n", rc.print());
        overall_rc = rc;
    }

    time_t at_time = time(0);

    if( sRC rc = usage_obj->updateDeleted(old_objs, old_reqs, reqProgressStatic, this, at_time) ) {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to update usage for deleted requests: %s\n", rc.print());
        overall_rc = rc;
    }


    if( sRC rc = sUsrHousekeeper::purgeObjs(*user, old_objs) ) {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to purge objects: %s\n", rc.print());
        overall_rc = rc;
    }
    if( sRC rc = sUsrHousekeeper::purgeReqs(*user, old_reqs) ) {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to purge requests: %s\n", rc.print());
        overall_rc = rc;
    }
    if( sRC rc = sUsrHousekeeper::purgeMisc(*user, req_purge_limit) ) {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to purge miscellaneous: %s\n", rc.print());
        overall_rc = rc;
    }
    if( sRC rc = sUsrHousekeeper::purgeTempFiles(*user, old_objs, old_reqs) ) {
        logOut(sQPrideBase::eQPLogType_Error, "Failed to purge temp files: %s\n", rc.print());
        overall_rc = rc;
    }

    if( overall_rc ) {
        reqSetInfo(req, sQPrideBase::eQPInfoLevel_Error, "Problems encountered while purging");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return;
    }

    reqSetStatus(req, eQPReqStatus_Done);
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    Housekeeper backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "housekeeper", argv[0]));
    return (int) backend.run(argc, argv);
}
