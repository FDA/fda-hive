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
#include <ulib/uemail.hpp>
#include "uperm.hpp"

using namespace slib;

const char* sUsrEmail::sm_prop[] = {"from", "to", "cc", "bcc", "subject", "body", "sent", "sent_dtm", "try_count", "err_msg", "draft" };

sUsrEmail::sUsrEmail(const sUsr& from, const sUsr& to, const char* subject, const char* body)
    : sUsrObj(from, "email")
{
    draft(true);
    addRecipient((enum ERecipientType) ePropFrom, from);
    addRecipient(eTo, to);
    if( subject ) {
        this->subject(subject);
    }
    if( body ) {
        this->body(body);
    }
}

sUsrEmail::sUsrEmail(sUsr& from, const char* to, const char* subject, const char* body)
    : sUsrObj(from, "email")
{
    draft(true);
    addRecipient((enum ERecipientType) ePropFrom, from);
    if( to ) {
        addRecipient(eTo, to);
    }
    if( subject ) {
        this->subject(subject);
    }
    if( body ) {
        this->body(body);
    }
}

sUsrEmail::sUsrEmail(sUsr& usr, const sHiveId & objId)
    : sUsrObj(usr, objId)
{
}

sUsrEmail::sUsrEmail(sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission)
    : sUsrObj(usr, objId, ptypeId, permission)
{
}

sUsrEmail::~sUsrEmail()
{
    if( m_recepients.rows ) {
        for(udx type = ePropFrom; type <= ePropBcc; ++type) {
            sVec<const char*> v;
            for(idx r = 0; r < m_recepients.rows; ++r) {
                if( m_recepients.uval(r, 0) == type ) {
                    *(v.add(1)) = m_recepients.val(r, 1);
                    if( type == ePropFrom && v.dim() > 0 ) {
                        break;
                    }
                }
            }
            if( v.dim() ) {
                propSet(sm_prop[type], v);
            }
        }
    }
}

udx sUsrEmail::addRecipient(ERecipientType type, const sUsr& usr)
{
    return usr.isEmailValid() ? addRecipient(type, usr.Email()) : 0;
}

udx sUsrEmail::addRecipient(ERecipientType type, const char* email)
{
    if( m_usr.isAllowed(Id(), ePermCanWrite) && email ) {
        m_recepients.addRow().addCol((udx)type).addCol(email);
        return 1;
    }
    return 0;
}
