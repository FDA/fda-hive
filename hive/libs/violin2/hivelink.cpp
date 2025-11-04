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

#include <violin/hivelink.hpp>
#include <qlib/QPrideProc.hpp>
#include <stdarg.h>

using namespace slib;
using namespace sviolin;

void HIVELink::initFromProcess(sQPrideProc * proc)
{
    sStrT b1,b2,b3;
    proc->formValue("pswd",&b1,"Blue134711Hive");
    proc->formValue("login",&b2,"queen");
    proc->formValue("session",&b3,0);
    setCredentials(b2, b1);
    setSession(b3);
}

char * HIVELink::vcmdr(sIO * io, const char * fmt, va_list marker)
{
    sIO * oldIO;
    if(io) { oldIO=curl.io;    curl.io=io;}

    url.printf(0,"%s/dna.cgi?",HIVEdomain.ptr());
    if(session.length())url.printf("sessionID=%s&",session.ptr());
    url.printf("cmdr=");
    url.vprintf(fmt,marker);

    curl.io->cut(0);
    curl.Get(url.ptr(0));
    lenResponse=curl.io->length();
    response=lenResponse ? curl.io->ptr() : 0;

    if(io)curl.io=oldIO;
    return response;
}


const char * HIVELink::loginToHIVE(const char * domain, idx domainLen)
{
    if(domain)setDomain(domain,domainLen);
    sStr buf,bur;
    if(!session.length() || cmdri("loginInfo")==0) {
        char * sessionID=cmdr("login&login=%s&pswd=%s",email.ptr(),password.ptr());
        if(sessionID) {
            char * p=strchr(sessionID,'\n');if(p)*p=0;
            setSession(sessionID);
        }
    }
    return session.ptr();
}


const char * HIVELink::getFile(sIO * dst, const char * objId, const char * srcflnm, idx objIdLen, idx lenFilename)
{
    if(!objId)return 0;
    loginToHIVE();
    return cmdr(dst,"objFile&ids=%.*s&filename=%.*s",(int)(objIdLen ? objIdLen : sLen(objId)),objId,(int)(lenFilename ? lenFilename : sLen(srcflnm)),srcflnm);
}


