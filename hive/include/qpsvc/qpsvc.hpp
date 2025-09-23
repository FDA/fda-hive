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
#ifndef sQPSvc_hpp
#define sQPSvc_hpp

#include <qlib/QPride.hpp>
#include <ulib/uproc.hpp>
#include <ulib/usr.hpp>

using namespace slib;

class sQPSvc
{
    public:

        sQPSvc(sQPride& qp)
            : m_qp(qp)
        {
        }
        virtual ~sQPSvc()
        {
        }

        virtual const char* getSvcName() const = 0;
        virtual void setobjDefaultParameters(){ }
        void getReqIds(sVec<idx>& vec) const;

        void setForm(sVar * form, const bool overwrite);

        idx launch(sUsr& user, idx grpID = 0, sHiveId * out_uprocID = 0, idx priority = 0, idx sliceId = 0);

        void setVar(const char* name, const char * value, ...) __attribute__((format(printf, 3, 4)));
        void setVar(const char* name, const sMex & data);

        bool setSessionID(const sVar * form);
        const char *getLockString(sStr &buf, const sHiveId &objId);

    protected:
        const char * getVar(const char* name) const;
        bool setData(const char* name, sMex * value);
        char * getData(sMex & buf, const char* name) const;

        virtual sUsrProc * makeObj(sUsr& user, sUsrProc * p = 0) const
        {
            return 0;
        }


    private:

        friend class svcHMI;

        sVar m_form;
        sQPride & m_qp;
        sVec<idx> m_reqs;

        sQPSvc(const sQPSvc& );
};

#endif 