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
#pragma once
#ifndef sLib_usrusage_h
#define sLib_usrusage_h

#include <slib/core/rc.hpp>
#include <ulib/uobj.hpp>
#include <ulib/uhousekeeper.hpp>
#include <slib/utils/tbl.hpp>

namespace slib {
    class sUsrUsage2: public sUsrObj
    {
        public:
            //*! Same interface as sQPrideProc::reqProgress()
            typedef idx (*progressCb)(void * param, idx items, idx progress, idx progressMax);

            enum EUsageType
            {
                eUsageTypeInvalid = -1,
                eDiskUsage = 0, //! permanent (object data) disk usage in bytes; type name is "disk-usage"
                eTempUsage, //! qpdata db and temporary disk usage in bytes; type name is "temp-usage"
                eObjCount, //! number of objects; type name is "obj-count"
                eReqCount, //! number of requests; type name is "req-count"
                eFileCount, //! number of permament (object data) files; type name is "file-count"
                eRunTime, //! number of seconds requests were running; type name is "run-time"
                eWaitTime, //! number of seconds requests waited to be grabbed; type name is "wait-time"
                eCompletionTime, //! number of seconds process objects took to complete; type name is "completion-time"
                eUsageTypeLast = eCompletionTime
            };
            static const char * getTypeName(EUsageType t);
            static EUsageType parseTypeName(const char * name);

            //! Get the global usage object.
            static const sUsrUsage2 * getObj(const sUsr & user, sRC * prc = 0);

            //! Get the global usage object. If it doesn't exist, create it.
            static sUsrUsage2 * ensureObj(const sUsr & admin, sRC * prc = 0);

            //! Typical usage update
            /*! \param at_time if non-0, attempt to emulate what would have happened if called at given time
             *  \warning at_time must be greater than effective time of any previous call */
            sRC updateIncremental(progressCb cb=0, void * cb_param = 0, time_t at_time=0);
            //! call before removing objects or requests
            sRC updateDeleted(const sVec<sUsrHousekeeper::PurgedObj> & objs, const sVec<sUsrHousekeeper::PurgedReq> & reqs, progressCb cb=0, void * cb_param = 0, time_t at_time=0);

            struct UserList
            {
                const char * label; //! email or group prefix
                idx dim; //! number of \a user_ids
                const udx * user_ids; //! pointer into array of user ids
            };

            //! Get usage for the current user
            sRC exportTable(sTxtTbl & out, EUsageType usage_type, time_t start, time_t end) const
            {
                return exportTable(out, usage_type, start, end, m_usr.Id());
            }
            //! Get usage for a specified user
            sRC exportTable(sTxtTbl & out, EUsageType usage_type, time_t start, time_t end, udx user_id) const
            {
                UserList list;
                list.label = m_usr.Email();
                list.dim = 1;
                list.user_ids = &user_id;
                return exportTable(out, usage_type, start, end, &list, 1);
            }
            //! Get usage sum for the list of unique users matched by each given group path (or "*" to get sum for all users)
            sRC exportGroupsTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const char * group_paths00, bool with_all_selected=false) const;
            //! Get usage sum for each given list of user accounts
            sRC exportTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const UserList * user_lists, idx num_user_lists) const;

            virtual ~sUsrUsage2();

        private:
            // constructor is private - use sUsrUsage::getObj() or ensureObj() to create
            sUsrUsage2(const sUsr& user, const sHiveId & objId);
            sRC getUsageChange(idx values[eUsageTypeLast + 1], udx user_id, udx primary_group_id, time_t since[eUsageTypeLast + 1], time_t to, progressCb cb, void * cb_param, idx iuser, idx num_users);

            class UsageDb;
            UsageDb * _usage_db;
    };
}

#endif // sLib_usrusage_h
