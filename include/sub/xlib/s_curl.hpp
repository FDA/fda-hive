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
#ifndef xLib_s_curl_hpp
#define xLib_s_curl_hpp



#include <curl/curl.h>
#include <slib/core/str.hpp>
#include <ion/vax.hpp>



namespace slib {
class sCurl
{
    private:
        CURL * curl;
        const char * cookiefile;
        struct curl_slist * headers;
        struct curl_httppost *post,*last;
        sStr fieldList;
        sIO myIO;


    public:
        idx http_status_code;
        idx skipCertificate;

            sCurl(sIO * lio=0)
            {
                curl_global_init(CURL_GLOBAL_ALL);

                curl =0;
                io=lio ? lio : &myIO;
                cookiefile=0;
                headers=0;
                post=0;
                last=0;
                http_status_code=0;
                skipCertificate=0;
            }


            ~sCurl()
            {
                if (curl) {
                    curl_easy_cleanup(curl);
                }
                if(post)
                    curl_formfree(post);

                if(headers)
                    curl_slist_free_all(headers);

                curl_global_cleanup();
            }

        static void * malloc_callback( size_t size);
        static void free_callback( void * ptr);
        static char * strdup_callback( const char * ptr);
        static void * realloc_callback( void * ptr, size_t size);
        static void * calloc_callback( size_t nmemb, size_t size);

        void cookie_setFile(const char * filePath) {
            cookiefile=filePath;
        }

        void setIO(sIO *lio = 0){
            io=lio ? lio : &myIO;
        }

        struct FieldValue {
                const char * field;
                const char * value;
                const char * type;
                idx size;
        };
        struct HeaderValue {
                const char * field;
                const char * value;
        };
    public:
        struct curl_slist * setHeader( const HeaderValue * hdrs, const char * fmt=0, va_list marker=0);
        struct curl_slist * setHeader( const char * field, ...){
            va_list ap;
            va_start(ap,field);
            struct curl_slist * hds=setHeader(0,field,ap);
            va_end(ap);
            return hds;
        }
        struct curl_httppost * setPost( const FieldValue * pairs, const char * fmt=0, va_list marker=0);
        struct curl_httppost * setPost( const char * field, ...) {
            va_list ap;
            va_start(ap,field);
            struct curl_httppost * ps=setPost(0,field,ap);
            va_end(ap);
            return ps;
        }
        idx setTimeout(idx timeout=60);

    public:

        sIO * io;

        void setCipher( const char * cipher);
        idx Get(const char * url, idx port=0, bool recookie=true);
        idx Post(const char * url, idx port=0, bool recookie=true);
        const char * UrlEncode(sStr * str,const char * src, idx cnt=0);

        sJax json;
        idx jsonGet(const char * url, sJax * pjson=0,idx port=0){
            io->cut(0);
            idx res=Get(url, port);
            if(res==CURLE_OK){
                if(!pjson)pjson=&json;
                pjson->empty();
                pjson->init(0,io->ptr(0),io->length());pjson->parseJson=true;
        pjson->parse();
            }
            return res;
        }
        idx jsonPost(const char * url, sJax * pjson=0, idx port=0){
            io->cut(0);
            idx res=Post(url, port);
            if(res==CURLE_OK){
                if(!pjson)pjson=&json;
                pjson->empty();
                pjson->init(0,io->ptr(0),io->length());pjson->parseJson=true;
                pjson->parse();
            }
            return res;
        }


    

};
}


#endif
