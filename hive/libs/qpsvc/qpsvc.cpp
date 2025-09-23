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
#include <qpsvc/qpsvc.hpp>
#include <qlib/QPrideClient.hpp>
#include <ulib/ufolder.hpp>

using namespace slib;

void sQPSvc::getReqIds(sVec<idx>& vec) const
{
    idx c = vec.dim();
    if( vec.add(m_reqs.dim()) ) {
        for(idx i = 0; i < m_reqs.dim(); ++i) {
            vec[c + i] = m_reqs[i];
        }
    }
}

void sQPSvc::setForm(sVar * form, const bool overwrite)
{
    for(idx i = 0; i < form->dim(); ++i) {
        const char * id = (const char *)form->id(i);
        if( id ) {
            if( overwrite || (!overwrite && !m_form.value(id)) ) {
                idx vlen = 0;
                const char * v = form->value(id, 0, &vlen);
                m_form.inp(id, v, vlen);
            }
        }
    }
}

void sQPSvc::setVar(const char* name, const char * value, ...)
{
    sStr s;
    sCallVarg(s.vprintf, value);
    m_form.inp(name, s, s.length() + 1);
}

void sQPSvc::setVar(const char* name, const sMex & data)
{
    m_form.inp(name, data.ptr(), data.pos());
}

const char * sQPSvc::getVar(const char* name) const
{
    return m_form.value(name, "");
}

bool sQPSvc::setData(const char* name, sMex * value)
{
    bool retval = m_reqs.dim();
    if( retval ) {
        void * ptr = value ? value->ptr() : 0;
        idx pos = value ? value->pos() : 0;
        for(idx i = 0; i < m_reqs.dim(); ++i) {
            retval &= m_qp.reqSetData(m_reqs[i], name, pos, ptr);
        }
    }
    return retval;
}

char * sQPSvc::getData(sMex & buf, const char* name) const
{
    char * retval = 0;
    if( m_reqs.dim() ) {
        retval = m_qp.reqGetData(m_reqs[0], name, &buf, false);
    }
    return retval;
}

const char * sQPSvc::getLockString(sStr &buf, const sHiveId &objId)
{
    udx bufpos = buf.length();
    buf.printf("%s:%s", getSvcName(), objId.print());
    return buf.ptr(bufpos);
}

idx sQPSvc::launch(sUsr & user, idx grpID, sHiveId * out_uprocID, idx priority, idx sliceId)
{
    if( !m_reqs.dim() ) {

        sVec<sUsrProc> procObjs;
        sQPrideBase::Service c_svc;
        m_qp.serviceGet(&c_svc, getSvcName(), 0);
        idx reqId = 0;

        if( const udx projID = user.getProject() ) {
            sStr sp("%" UDEC, projID);
            m_form.inp("projectID", sp.ptr());
        }
        
        sStr log, strObjList;
        idx err = 0;
        if (!grpID) {
            sUsrProc * new_proc = makeObj(user, procObjs.add());
            if( new_proc ) {
                new_proc->IdStr(&strObjList);
                strObjList.add0();
                new_proc->reqID(reqId);
                new_proc->service(getSvcName());
                sUsrFolder * inbox = sSysFolder::Inbox(user);
                if( inbox ) {
                    inbox->attach(*new_proc);
                    delete inbox;
                }
                if( out_uprocID ) {
                    *out_uprocID = new_proc->Id();
                }
            }
        }
        err = sUsrProc::standardizedSubmission(&m_qp, &m_form, &user, procObjs, 1, &reqId, &c_svc, 0, &strObjList, &log);
        if( err ) {
            if( grpID ) {
                m_qp.reqSetInfo(grpID, sQPrideBase::eQPInfoLevel_Error, "Failure to submit.\n%s", log.ptr());
            }
            m_qp.logOut(sQPrideBase::eQPLogType_Error, "Failure to submit.\n%s", log.ptr());
            return 0;
        }
        if( reqId ) {
            m_qp.grp2Req(reqId, &m_reqs, 0, 0);
            if( grpID ) {
                for(idx i = 0; i < m_reqs.dim(); ++i) {
                    m_qp.grpAssignReqID(m_reqs[i], grpID, sliceId);
                }
            }
        }
    }
    return m_reqs.dim() ? m_reqs[0] : 0;
}

bool sQPSvc::setSessionID(const sVar * form)
{
    if( !form ) {
        return false;
    }
    const char * val = form->value("sessionID");
    if( !val ) {
        return false;
    }
    setVar("sessionID", "%s", val);

    val = form->value("userName");
    if( val ) {
        setVar("userName", "%s", val);
    }

    val = form->value("email");
    if( val ) {
        setVar("email", "%s", val);
    }
    return true;
}
