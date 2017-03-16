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
#ifndef sLib_uhousekeeper_h
#define sLib_uhousekeeper_h

#include <slib/core/rc.hpp>
#include <ulib/usr.hpp>

namespace slib {
    class sUsrHousekeeper
    {
        public:
            struct PurgedObj {
                sHiveId hive_id;
                udx creator_id;
            };
            struct PurgedReq {
                udx req_id;
                udx user_id;
            };

            static sRC findObjsForPurge(const sUsr & admin, sVec<PurgedObj> & objs, idx max_cnt=1000, idx max_age_days=30);
            static sRC findReqsForPurge(const sUsr & admin, sVec<PurgedReq> & reqs, idx max_cnt=1000);

            static sRC purgeObjs(sUsr & admin, const sVec<PurgedObj> & objs);
            static sRC purgeReqs(const sUsr & admin, const sVec<PurgedReq> & reqs);
            static sRC purgeTempFiles(const sUsr & admin, const sVec<PurgedObj> & objs, const sVec<PurgedReq> & reqs);
            static sRC purgeMisc(const sUsr & admin, idx limit=1000);
    };
};

#endif /* ifdef sLib_uhousekeeper_h */
