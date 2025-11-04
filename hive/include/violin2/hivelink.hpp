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
#ifndef violin_hivelink_hpp
#define violin_hivelink_hpp

#include <slib/core.hpp>
#include <ulib/usr.hpp>
#include <xlib/s_curl.hpp>
#include <slib/std/url.hpp>
#include <qlib/QPrideProc.hpp>

namespace sviolin {
    class HIVELink {

        sStrT HIVEdomain,session,password, email,url;
        sCurl curl;



        public:
            idx lenResponse;
            char * response;

            void initFromProcess(sQPrideProc * proc);

            const char * setDomain(const char * domain, idx domainLen=0){
                if(strncmp(domain,"http://",7)==0 )domain+=7;
                else if(strncmp(domain,"https://",8)==0)domain+=7;
                const char * domainEnd=strchr(domain,'/');
                if(!domainLen)domainLen=domainEnd ? domainEnd-domain : sLen(domain);
                return HIVEdomain.printf(0,"%.*s",(int)(domainLen ? domainLen : sLen(domain)),domain);
            }
            void setCredentials(const char * login,  const char * pswd, idx loginLen=0, idx pswdLen=0){
                if(pswd)password.printf(0,"%.*s",(int)(pswdLen ? pswdLen : sLen(pswd)),pswd);
                if(login)email.printf(0,"%.*s",(int)(loginLen ? loginLen : sLen(login)),login);
            }
            void setSession(const char * sessionID,  idx sessionIDLen=0){
                if(sessionID){
                    sStr tt;
                    tt.printf("%.*s",(int)(sessionIDLen ? sessionIDLen : sLen(sessionID)),sessionID);
                    session.cut(0);
                    URLEncode(tt, session);
                }
            }

            char * vcmdr(sIO * io, const char * fmt, va_list marker);
            char * cmdr(sIO * io,const char * fmt, ... ){char * res;sCallVargResPara(res,vcmdr,io,fmt);    return res;}
            char * cmdr(const char * fmt, ... ){char * res;sCallVargResPara(res,vcmdr,0,fmt);return res;}
            idx cmdri(const char * fmt, ... ){    char * res;sCallVargResPara(res,vcmdr,0,fmt);    return res ? atoidx(res) : 0;}
            char * cmdrf(const char * dstflnm,const char * fmt, ... ){sIO destFile;destFile.init(dstflnm);char * res;sCallVargResPara(res,vcmdr,&destFile,fmt);return res;}

            const char * loginToHIVE(const char * domain=0, idx domainLen=0);
            const char * getFile (sIO * dst, const char * objId, const char * srcflnm, idx objIdLen=0, idx lenFilename=0);
            const char * getFile (const char * objId, const char * srcflnm, idx objIdLen=0, idx lenFilename=0){return getFile((sIO*)0,objId,srcflnm,objIdLen,lenFilename);}
            void getFile(const char * dstflnm, const char * objId, const char * srcflnm, idx objIdLen=0, idx lenFilename=0){sIO destFile;sFile::remove(dstflnm);destFile.init(dstflnm);getFile(&destFile,objId,srcflnm,objIdLen,lenFilename);}
    };
};

#endif
