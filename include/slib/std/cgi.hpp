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
#ifndef sLib_std_cgi_hpp
#define sLib_std_cgi_hpp

#include <slib/std/http.hpp>

namespace slib {

    class sCGI: public sHtml
    {

        public:
            sCGI(idx argc = 0, const char * * argv = 0, const char * * envp = 0, FILE * readfrom = stdin, bool isCookie = false, bool immediate = true, const char * forcedMethod = 0)
                : sHtml(immediate), m_headersPrinted(false), m_cookieSecureOverHTTP(false)
            {
                init(argc, argv, envp, readfrom, isCookie, forcedMethod);
            }
            sCGI * init(idx argc, const char * * argv, const char * * envp, FILE * readfrom = stdin, bool isCookie = false, const char * forcedMethod = 0);
            virtual ~sCGI()
            {
            }
        public:
            sStr execJS, dataForm, htmlBody, htmlDirs00, redirectURL;
            sVar form, *pForm;
            const char * cmd;
            const char * mangleNameChar, *keywordTmpltHtml;

            idx raw;
            static idx grabPost;
            FILE * flOut;
            enum eSectionsToHide
            {
                eSectionHideHeader = 0x00000001, eSectionHideTop = 0x00000002, eSectionHideBottom = 0x00000004, eSectionHideTail=0x00000008
            };
            idx sectionsToHide;

        public:
            virtual void outHeaders(sStr* str = 0);
            virtual void outHtml(sHtml * html = 0);
            virtual void headerSet(const char * nm, const char * fmt, ...) __attribute__((format(printf, 3, 4)));
            void headerSetContentDispositionAttachment(const char * flnm_fmt, ...) __attribute__((format(printf, 2, 3)));
            virtual void headerDelete(const char * nm);
            udx getStatus(void);

            enum {
                eCookie_None = 0,
                eCookie_Secure= 0x01,
                eCookie_HttpOnly = 0x02,
                eCookie_SameSiteStrict = 0x04,
                eCookie_SameSiteLax = 0x08,
                eCookie_SameSiteNone = 0x10
            };
            virtual void cookieSet(const char * nm, const idx flags, const char * fmt, ...) __attribute__((format(printf, 4, 5)));
            virtual void cookieSet(const char * nm, const char * fmt, ...) __attribute__((format(printf, 3, 4)));
            void cookieSecureOverHTTP(const bool & always) { m_cookieSecureOverHTTP = always; };
            virtual void cookieDelete(const char * nm);
            virtual void alert(const char * fmt, ...) __attribute__((format(printf, 2, 3)));
            virtual void error(const char * fmt, ...) __attribute__((format(printf, 2, 3)));
            virtual void warning(const char * fmt, ...) __attribute__((format(printf, 2, 3)));
            virtual void linkSelf(const char * cmd, const char * fmt, ...) __attribute__((format(printf, 3, 4)));
            void linkSelf(const char * cmd)
            {
                return linkSelf(cmd, 0);
            }
            virtual void executeJS(const char * fmt, ...) __attribute__((format(printf, 2, 3)));
            virtual void exportData(const char * dataname, const char * fmt, ...) __attribute__((format(printf, 3, 4)));
            virtual idx outSectionVa(sHtml * html, const char * sectionFmt, va_list marker);
            idx outSection(sHtml * html, const char * sectionFmt, ...) __attribute__((format(printf, 3, 4)))
            {
                idx ret;
                sCallVargResPara(ret, outSectionVa, html, sectionFmt);
                return ret;
            }
            idx outSection(const char * sectionFmt, ...) __attribute__((format(printf, 2, 3)))
            {
                idx ret;
                sCallVargResPara(ret, outSectionVa, 0, sectionFmt);
                return ret;
            }

            virtual idx outBinByteRange(sStr *flPath,char * flName,sHtml::sPartPair::TParts *ranges);
            virtual void outBin(const void * buf, idx len, idx timeStamp, bool asAttachment, const char * flnmFormat, ...) __attribute__((format(printf, 6, 7)));
            virtual void outBinHeaders(bool asAttachment, const char * flnmFormat, ...) __attribute__((format(printf, 3, 4)));
            virtual void outBinData(const void * buf, idx len) { fwrite(buf, len, 1, flOut); }
            virtual bool outFile(const char * flnmReal, bool asAttachment, const char * flnmFormat, ...) __attribute__((format(printf, 4, 5)));

            virtual const char* selfURL(sStr& url);

            void setFlOut(FILE * f)
            {
                if( !f ) {
                    f = stdout;
                }
                _funcCallbackParam = flOut = f;
            }
        public:

            virtual idx Cmd(const char *);
            virtual bool OnCGIInit(void)
            {
                return true;
            }
            virtual idx CmdStatic(const char *);

            virtual idx run(const char * rcmd = 0);

        public:
            void htmlDirs(const char * dirs = "./,lib/");

        protected:

            virtual bool checkETag(sStr & etagBuf, idx len, idx timeStamp);
            virtual void outBinCached(const char * etag);
            virtual void voutBinUncached(const void * buf, idx len, const char * etag, bool asAttachment, const char * flnmFormat, va_list marker);
            virtual bool resolveFile(const char * filename) {return false;}

        private:

            sVar m_headers;
            bool m_headersPrinted;
            sVar m_cookies;
            bool m_cookieSecureOverHTTP;
    };

}
#endif 