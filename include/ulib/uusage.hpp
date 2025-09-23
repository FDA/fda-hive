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
            typedef idx (*progressCb)(void * param, idx items, idx progress, idx progressMax);

            enum EUsageType
            {
                eUsageTypeInvalid = -1,
                eDiskUsage = 0,
                eTempUsage,
                eObjCount,
                eReqCount,
                eFileCount,
                eRunTime,
                eWaitTime,
                eCompletionTime,
                eComputationObjCount,
                eDataLoadingObjCount,
                eFileObjCount,
                eDirectoryObjCount,
                eUsageTypeLast = eDirectoryObjCount
            };
            static const char * getTypeName(EUsageType t);
            static EUsageType parseTypeName(const char * name);

            static const sUsrUsage2 * getObj(const sUsr & user, sRC * prc = 0);

            static sUsrUsage2 * ensureObj(sUsr & admin, sRC * prc = 0);

            time_t getFirstUpdateTime() const;
            time_t getLastUpdateTime() const;

            sRC updateIncremental(progressCb cb=0, void * cb_param = 0, time_t at_time = 0, bool find_deleted = false);
            sRC updateDeleted(const sVec<sUsrHousekeeper::PurgedObj> & objs, const sVec<sUsrHousekeeper::PurgedReq> & reqs, progressCb cb=0, void * cb_param = 0, time_t at_time=0);

            struct UserList
            {
                sStr label;
                sVec<udx> user_ids;
            };

            sRC exportTable(sTxtTbl & out, EUsageType usage_type, time_t start, time_t end) const
            {
                return exportTable(out, usage_type, start, end, m_usr.Id());
            }
            sRC exportTable(sTxtTbl & out, EUsageType usage_type, time_t start, time_t end, udx user_id) const
            {
                UserList list;
                list.label.init(sMex::fExactSize);
                list.label.addString(m_usr.Email());
                list.user_ids.init(sMex::fExactSize);
                *list.user_ids.add(1) = user_id;
                return exportTable(out, usage_type, start, end, &list, 1);
            }

            struct GroupSpec {
                enum {
                    eGroupPath,
                    eUserID,
                    eBillableGroupObj,
                    eBillableGroupName
                } kind;
                sVariant value;
                sVec<GroupSpec> except;
            };
            sRC exportGroupsTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, GroupSpec * group_specs, idx num_group_specs, bool with_all_selected=false, bool expand_users=false) const;

            sRC exportGroupsTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const char * group_paths00, bool with_all_selected=false) const;
            sRC exportTable(sTxtTbl & out, EUsageType usage_type, time_t since, time_t to, const UserList * user_lists, idx num_user_lists) const;

            virtual ~sUsrUsage2();

        private:
            class IncrementalUpdates;

            sUsrUsage2(const sUsr& user, const sHiveId & objId, sUsr * admin);
            sRC getUsageChange(IncrementalUpdates * value_updates, udx user_id, udx primary_group_id, time_t since[eUsageTypeLast + 1], time_t to, progressCb cb, void * cb_param, idx iuser, idx num_users, bool find_deleted);

            class UsageDb;
            UsageDb * _usage_db;
            sUsr * _admin;
    };
}

#endif 