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




sStr sCurl_str;
void * sCurl::malloc_callback( size_t size)
{

    void * p=(void*)sCurl_str.add(0,(idx)size+sizeof(idx));
    *((idx*)p)=size;
    p=sShift(p,sizeof(idx));
    return p;
}
void sCurl::free_callback( void * ptr)
{

}

char * sCurl::strdup_callback( const char * ptr)
{
    ::printf("strdup " );
    size_t orisize=sLen(ptr);
    void * p=(char*)malloc_callback(orisize);
    memcpy(p,ptr,orisize);
    return (char*)p;
}

void * sCurl::calloc_callback( size_t nmemb, size_t size)
{
    ::printf("calloc " );
    return malloc_callback(size*nmemb);
}

void * sCurl::realloc_callback( void * ptr, size_t size)
{
    size_t orisize=*(idx*)sShift(ptr,-sizeof(idx));
    void * p=malloc_callback(size);
    memcpy(p,ptr,sMin(orisize,size));
    return p;
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize=size*nmemb;
    sCurl * mycls=(sCurl * )userp;
    if(realsize)mycls->io->add((const char *)contents, (idx)realsize);
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

idx sCurl::setTimeout(idx timeout)
{
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    return 0;
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
            resM=(CURLFORMcode)CURLE_OK;
        }
        if((idx)resM!=(idx)CURLE_OK || res!=(idx)CURLE_OK)
            return 0;
    }

    return post ;
}



idx sCurl::Get(const char * url, idx port, bool recookie)
{
    CURLcode res;
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);

    if(cookiefile) {
        if(recookie)
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiefile);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiefile);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);

    if(skipCertificate) {
        curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    if(port)curl_easy_setopt(curl, CURLOPT_PORT, port);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    if(headers)
        res=curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


    res = curl_easy_perform(curl);



    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    http_status_code=http_code;

    if(headers){
        curl_slist_free_all(headers);
        headers=0;
    }


    curl_easy_cleanup(curl);
    curl=0;
    return (idx)res;
}

void sCurl::setCipher( const char * cipher)
{
    curl_easy_setopt(curl, CURLOPT_SSL_CIPHER_LIST, cipher);
}

idx sCurl::Post(const char *  url, idx port, bool recookie)
{
    CURLcode res;
    curl = curl_easy_init();

    const char * pars=strchr(url,'?');
    sStr t,u;
    if(pars){

        sString::searchAndReplaceSymbols(&t,pars+1,0,"&",0,0,true,true,false,true);
        for (char * p=t.ptr(), * nxt; p;  p=nxt){
            nxt=sString::next00(p);
            char * equ=strchr(p,'=');
            if(equ){*equ=0;++equ;}
            curl_formadd(&post, &last, CURLFORM_COPYNAME, p,
                CURLFORM_PTRCONTENTS, equ ? equ : "",
                CURLFORM_END);

        }
        u.add(url,pars-url);
        u.add0(1);
        url=u.ptr(0);
    }


    curl_easy_setopt(curl, CURLOPT_URL, url);

    if(cookiefile) {
        if(recookie)curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiefile);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiefile);
    }
    if(skipCertificate) {
        curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
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
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    http_status_code=http_code;


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

const char * sCurl::UrlEncode(sStr * str,const char * src, idx cnt)
{
    char * s=curl_easy_escape(curl, src, cnt ? cnt : sLen (src) );
    if(s){
        str->printf("%s",s);
        curl_free(s);
    }
    return str->ptr(0);
}
