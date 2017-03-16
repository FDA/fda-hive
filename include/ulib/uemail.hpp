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
#ifndef sLib_usremail_h
#define sLib_usremail_h

#include <ulib/uobj.hpp>

namespace slib {

    class sUsrEmail: public sUsrObj
    {
        public:
            sUsrEmail(const sUsr& from, const sUsr& to, const char* subject = 0, const char* body = 0);
            sUsrEmail(sUsr& from, const char* to, const char* subject = 0, const char* body = 0);
            sUsrEmail(sUsr& usr, const sHiveId & objId);
            //! safe constructor for use in propset in case of write-only permissions
            sUsrEmail(sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission);
            ~sUsrEmail();

            enum ERecipientType {
                eTo = 1,
                eCc,
                eBcc,
            };

            udx addRecipient(ERecipientType type, const sUsr& usr);

            udx subject(const char* subject)
            { return propSet(sm_prop[ePropSubject], subject); }
            const char* subject(void) const
            { return propGet(sm_prop[ePropSubject]); }

            const char* from(void) const
            { return propGet(sm_prop[ePropFrom]); }
            udx to(sVarSet& res) const
            { return propGet(sm_prop[ePropTo], res); }
            udx cc(sVarSet& res) const
            { return propGet(sm_prop[ePropCc], res); }
            udx bcc(sVarSet& res) const
            { return propGet(sm_prop[ePropBcc], res); }

            udx body(const char* fmt, ...) __attribute__((format(printf, 2, 3)))
            {
                const char* p = body();
                sStr b("%s", p ? p : "");
                sCallVarg(b.vprintf, fmt);
                return propSet(sm_prop[ePropBody], b);
            }
            const char* body() const
            { return propGet(sm_prop[ePropBody]); }

            // technical fields
            bool isSent(void) const
            { return propGetBool(sm_prop[ePropSent]); }
            udx isSent(bool is_sent)
            { return propSetBool(sm_prop[ePropSent], is_sent); }

            time_t sent(void) const
            { return propGetDTM(sm_prop[ePropSentOn]); }
            udx sent(time_t when)
            { return propSetDTM(sm_prop[ePropSentOn], when); }

            udx retries(void) const
            { return propGetU(sm_prop[ePropTryCount]); }
            udx retries(udx count)
            { return propSetU(sm_prop[ePropTryCount], count); }

            udx errmsg(const char* errmsg)
            { return propSet(sm_prop[ePropErrMsg], errmsg); }
            const char* errmsg(void) const
            { return propGet(sm_prop[ePropErrMsg]); }

        private:

            udx addRecipient(ERecipientType type, const char* email);

            sVarSet m_recepients;
            static const char* sm_prop[];
            // must be in sync with above enum for recipients!!
            enum EProp {
                ePropFrom = 0,
                ePropTo = 1,
                ePropCc,
                ePropBcc,
                ePropSubject,
                ePropBody,
                ePropSent,
                ePropSentOn,
                ePropTryCount,
                ePropErrMsg
            };
    };
}

#endif // sLib_usremail_h
