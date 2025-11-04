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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include <xlib/s_curl.hpp>

using namespace slib;
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize=size*nmemb;
    sCurl * mycls=(sCurl * )userp;
    mycls->io->add((const char *)contents, (idx)realsize);
    return realsize;
}

struct curl_slist * sCurl::setHeader( const HeaderValue * hdrs, const char * fmt, va_list marker)
{
    sStr t;

    for(idx i=0 ; true ; ++i) {
        const char * fld = marker ? (i==0 ? fmt : va_arg(marker, char *)) : hdrs[i].field;
        if(fld==0)
            break;
        const char * val = marker ? va_arg(marker, char *) : hdrs[i].value;

        t.printf(0,"%s: %s", fld, val);
        headers=curl_slist_append(headers,t.ptr(0));
    }

    return headers;
}


struct curl_httppost *sCurl::setPost( const FieldValue * pairs, const char * fmt, va_list marker )
{

    for(idx i=0 ; true ; ++i) {
        const char * fld = marker ? (i==0 ? fmt : va_arg(marker, const char *) ) : pairs[i].field;
        if(fld==0)
            break;
        const char * val = marker ? va_arg(marker, const char *) : pairs[i].value;
        const char * typ = marker ? va_arg(marker, const char *) : pairs[i].type;
        idx size = marker ? va_arg(marker, idx ) : pairs[i].size;

        CURLFORMcode resM;
        CURLcode res=CURLE_OK;

        if(typ) {
            if( *typ ) {
                resM=curl_formadd(&post, &last, CURLFORM_COPYNAME, fld,
                    CURLFORM_FILE, (const char *) val ,
                    CURLFORM_CONTENTTYPE, typ,
                    CURLFORM_END);
            } else {
                resM=curl_formadd(&post, &last, CURLFORM_COPYNAME, fld,
                   CURLFORM_PTRCONTENTS, val,
                   CURLFORM_CONTENTSLENGTH, size,
                   CURLFORM_CONTENTTYPE, "text/html",
                   CURLFORM_END);
            }
        } else {
            if(fieldList.length())fieldList.add("&",1);
            fieldList.add(fld,size);
        }
        if((idx)resM!=(idx)CURLE_OK || res!=(idx)CURLE_OK)
            return 0;
    }

    return post ;
}


idx sCurl::Get(const char * url, idx port)
{
    CURLcode res;
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);

    if(cookiefile) {
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiefile);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiefile);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);

    if(port)curl_easy_setopt(curl, CURLOPT_PORT, port);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    if(headers) {
        res=curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }


    res = curl_easy_perform(curl);

    if(headers){
        curl_slist_free_all(headers);
        headers=0;
    }


    curl_easy_cleanup(curl);
    curl=0;
    return (idx)res;
}

idx sCurl::Post(const char *  url, idx port)
{
    CURLcode res;
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);

    if(cookiefile) {
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiefile);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiefile);
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
    

    if(post)
        res=curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    if(fieldList.length()) {
        fieldList.add0();
        res=curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fieldList.ptr(0));
    }

    if(headers)
        res=curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


    if(port)curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    res = curl_easy_perform(curl);
  

    if(post){
        curl_formfree(post);
        post=0;
        last=0;
    }
    if(headers){
        curl_slist_free_all(headers);
        headers=0;
    }

    curl_easy_cleanup(curl);
    curl=0;
    


    return (idx)res;
}

