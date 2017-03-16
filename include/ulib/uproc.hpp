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
#ifndef sLib_usrproc_h
#define sLib_usrproc_h

#include <ulib/uobj.hpp>
#include <qlib/QPride.hpp>

namespace slib {

    class sUsrProc: public sUsrObj
    {
            typedef sUsrObj TParent;

        public:
            // we must have a default constructor for this object if we ever want to use it inside of containers
            sUsrProc()
                : m_req(0)
            {
            }

            sUsrProc(const sUsr& usr, const char * svc_objtype)
                : sUsrObj(usr, svc_objtype), m_req(0)
            {
            }
            sUsrProc(const sUsr& usr, const sHiveId & objId)
                : sUsrObj(usr, objId), m_req(0)
            {
            }
            //! safe constructor for use in propset in case of write-only permissions
            sUsrProc(const sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission)
                : sUsrObj(usr, objId, ptypeId, permission), m_req(0)
            {
            }

            virtual udx propSet(const char* prop, const char** groups, const char** values, udx cntValues, bool isAppend = false, const udx * path_lens = 0, const udx * value_lens = 0);
            virtual udx propGet(const char* prop, sVarSet& res, bool sort=false) const;
            bool propSync(void);

            idx reqID(void) const
            {
                return m_req ? m_req : (m_req = propGetI(sm_prop[ePropReqID]));
            }
            udx reqID(idx reqID)
            {
                return propSetU(sm_prop[ePropReqID], reqID);
            }

            const char* service(void) const
            {
                return TParent::propGet(sm_prop[ePropServiceName]);
            }
            udx service(const char* svc)
            {
                return TParent::propSet(sm_prop[ePropServiceName], svc);
            }

            time_t started(void) const
            {
                return propGetDTM(sm_prop[ePropStarted]);
            }
            const char* sstarted(sStr* buf = 0) const
            {
                return TParent::propGet(sm_prop[ePropStarted], buf);
            }

            idx progress(void) const
            {
                return propGetI(sm_prop[ePropProgress]);
            }
            const char* sprogress(sStr* buf = 0) const
            {
                return TParent::propGet(sm_prop[ePropProgress], buf);
            }

            idx act(void) const
            {
                return propGetI(sm_prop[ePropAction]);
            }
            const char* sact(sStr* buf = 0) const
            {
                return TParent::propGet(sm_prop[ePropAction], buf);
            }

            idx progress100(void) const
            {
                return propGetI(sm_prop[ePropProgress100]);
            }
            const char* sprogress100(sStr* buf = 0) const
            {
                return TParent::propGet(sm_prop[ePropProgress100], buf);
            }

            idx status(void) const
            {
                return propGetI(sm_prop[ePropStatus]);
            }
            const char* sstatus(sStr* buf = 0) const
            {
                return TParent::propGet(sm_prop[ePropStatus], buf);
            }

            time_t completed(void) const
            {
                return propGetDTM(sm_prop[ePropCompleted]);
            }
            const char* scompleted(sStr* buf = 0) const
            {
                return TParent::propGet(sm_prop[ePropCompleted], buf);
            }

            enum errSubmission
            {
                errHiveTools_NoInbox = 1,
                errHiveTools_ProcessCreation,
                errHiveTools_SubmissionCustomization,
                errhiveTools_SubmissionTooLarge,
                errhiveTools_SubmissionFaled,
                errHiveTools_last
            };
            static idx createProcesForsubmission(sQPrideBase * qp, sVar * pForm, sUsr * user, sVec<sUsrProc> & procObjs, sQPride::Service * pSvc, sStr * strObjList, sStr * log);
            static idx standardizedSubmission(sQPrideBase * qp, sVar * pForm, sUsr * user, sVec<sUsrProc> & procObjs, idx cntParallel, idx * pReq, sQPride::Service * pSvc, idx previousGrpSubmitCounter, sStr * strObjList, sStr * log);

        protected:
            bool onDelete(void);
            bool onPurge(void);

        private:

            typedef struct sUsrProcReq sUsrProcReq;

            static udx isQprideProp(const char* prop);
            bool propGet(udx propId, sUsrProcReq& ur, idx& res) const;
            bool propSync(sUsrProcReq& ur);

            static const char* sm_prop[];
            enum EProp
            {
                ePropReqID = 1,
                ePropServiceName,
                ePropCompleted,
                ePropProgress,
                ePropStarted,
                ePropStatus,
                ePropAction,
                ePropProgress100
            };
            mutable idx m_req;

    };
}

#endif // sLib_usrproc_h
