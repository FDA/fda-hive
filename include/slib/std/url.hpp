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
#ifndef sLib_std_url_hpp
#define sLib_std_url_hpp

#include <slib/core/str.hpp>
#include <memory>

namespace slib {

    enum EUrlEncode
    {
        eUrlEncode_SkipMarkChars,
        eUrlEncode_ProcessMarkChars,
        eUrlEncode_PercentOnly,
        eUrlEncode_Path,

        eUrlEncode_URIScheme,
        eUrlEncode_URIUserinfo,
        eUrlEncode_URIHost,
        eUrlEncode_URIPath,
        eUrlEncode_URIQueryName,
        eUrlEncode_URIQueryNameEscapeQuotes,
        eUrlEncode_URIQueryValue,
        eUrlEncode_URIQueryValueEscapeQuotes,
        eUrlEncode_URIFragment,

        eUrlEncode_ExtValue,

        eUrlEncode_None
    };

    enum EUrlDecode
    {
        eUrlDecode_All,
        eUrlDecode_Percent
    };

    enum EAmpEncoding
    {
        eAmp_Char,
        eAmp_Entity
    };

    bool NeedsURLEncoding(const char* str, EUrlEncode flag = eUrlEncode_SkipMarkChars);
    const char* URLEncode(const char* str, sStr& dst, EUrlEncode flag = eUrlEncode_SkipMarkChars);
    const char* URLDecode(const char* str, sStr& dst, EUrlDecode flag = eUrlDecode_All);

    class IUrlEncoder
    {
        public:
            virtual ~IUrlEncoder(void)
            {
            }

            virtual const char* EncodeUser(const char* user, sStr& dst) const = 0;
            virtual const char* DecodeUser(const char* user, sStr& dst) const = 0;
            virtual const char* EncodePassword(const char* password, sStr& dst) const = 0;
            virtual const char* DecodePassword(const char* password, sStr& dst) const = 0;
            virtual const char* EncodePath(const char* path, sStr& dst) const = 0;
            virtual const char* DecodePath(const char* path, sStr& dst) const = 0;
            virtual const char* EncodeArgName(const char* name, sStr& dst) const = 0;
            virtual const char* DecodeArgName(const char* name, sStr& dst) const = 0;
            virtual const char* EncodeArgValue(const char* value, sStr& dst) const = 0;
            virtual const char* DecodeArgValue(const char* value, sStr& dst) const = 0;
            virtual const char* EncodeFragment(const char* value, sStr& dst) const = 0;
            virtual const char* DecodeFragment(const char* value, sStr& dst) const = 0;
    };

    class CEmptyUrlEncoder: public IUrlEncoder
    {
        public:
            virtual const char* EncodeUser(const char* user, sStr& dst) const
            {
                return dst.add(user);
            }
            virtual const char* DecodeUser(const char* user, sStr& dst) const
            {
                return dst.add(user);
            }
            virtual const char* EncodePassword(const char* password, sStr& dst) const
            {
                return dst.add(password);
            }
            virtual const char* DecodePassword(const char* password, sStr& dst) const
            {
                return dst.add(password);
            }
            virtual const char* EncodePath(const char* path, sStr& dst) const
            {
                return dst.add(path);
            }
            virtual const char* DecodePath(const char* path, sStr& dst) const
            {
                return dst.add(path);
            }
            virtual const char* EncodeArgName(const char* name, sStr& dst) const
            {
                return dst.add(name);
            }
            virtual const char* DecodeArgName(const char* name, sStr& dst) const
            {
                return dst.add(name);
            }
            virtual const char* EncodeArgValue(const char* value, sStr& dst) const
            {
                return dst.add(value);
            }
            virtual const char* DecodeArgValue(const char* value, sStr& dst) const
            {
                return dst.add(value);
            }
            virtual const char* EncodeFragment(const char* value, sStr& dst) const
            {
                return dst.add(value);
            }
            virtual const char* DecodeFragment(const char* value, sStr& dst) const
            {
                return dst.add(value);
            }
    };

    class CDefaultUrlEncoder: public CEmptyUrlEncoder
    {
        public:
            CDefaultUrlEncoder(EUrlEncode encode = eUrlEncode_SkipMarkChars)
                    : m_Encode(EUrlEncode(encode))
            {
                return;
            }
            virtual const char* EncodePath(const char* path, sStr& dst) const
            {
                return URLEncode(path, dst, eUrlEncode_URIPath);
            }
            virtual const char* DecodePath(const char* path, sStr& dst) const
            {
                return URLDecode(path, dst);
            }
            virtual const char* EncodeArgName(const char* name, sStr& dst) const
            {
                return URLEncode(name, dst, m_Encode);
            }
            virtual const char* DecodeArgName(const char* name, sStr& dst) const
            {
                return URLDecode(name, dst, m_Encode == eUrlEncode_PercentOnly ? eUrlDecode_Percent : eUrlDecode_All);
            }
            virtual const char* EncodeArgValue(const char* value, sStr& dst) const
            {
                return URLEncode(value, dst, m_Encode);
            }
            virtual const char* DecodeArgValue(const char* value, sStr& dst) const
            {
                return URLDecode(value, dst, m_Encode == eUrlEncode_PercentOnly ? eUrlDecode_Percent : eUrlDecode_All);
            }
            virtual const char* EncodeFragment(const char* value, sStr& dst) const
            {
                return URLEncode(value, dst, eUrlEncode_URIFragment);
            }
            virtual const char* DecodeFragment(const char* value, sStr& dst) const
            {
                return URLDecode(value, dst, eUrlDecode_All);
            }
        private:
            EUrlEncode m_Encode;
    };

    class CUrlArgs;

    class CUrl
    {
        public:
            CUrl(void);

            CUrl(const char* url, const IUrlEncoder* encoder = 0);

            ~CUrl();

            bool SetUrl(const char* url, const IUrlEncoder* encoder = 0);

            const char* ComposeUrl(EAmpEncoding amp_enc, sStr& dst, const IUrlEncoder* encoder = 0) const;


            const sStr& GetScheme(void) const
            {
                return m_Scheme;
            }
            void SetScheme(const sStr& value)
            {
                m_Scheme.replace(value);
            }

            bool GetIsGeneric(void) const
            {
                return m_IsGeneric;
            }
            void SetIsGeneric(bool value)
            {
                m_IsGeneric = value;
            }

            const sStr& GetUser(void) const
            {
                return m_User;
            }
            void SetUser(const sStr& value)
            {
                m_User.replace(value);
            }

            const sStr& GetPassword(void) const
            {
                return m_Password;
            }
            void SetPassword(const sStr& value)
            {
                m_Password.replace(value);
            }

            const sStr& GetHost(void) const
            {
                return m_Host;
            }
            void SetHost(const sStr& value)
            {
                m_Host.replace(value);
            }

            const sStr& GetPort(void) const
            {
                return m_Port;
            }
            void SetPort(const sStr& value)
            {
                m_Port.replace(value);
            }

            const sStr& GetPath(void) const
            {
                return m_Path;
            }
            void SetPath(const sStr& value)
            {
                m_Path.replace(value);
            }

            const sStr& GetFragment(void) const
            {
                return m_Fragment;
            }
            void SetFragment(const sStr& value)
            {
                m_Fragment.replace(value);
            }

            const sStr& GetOriginalArgsString(void) const
            {
                return m_OrigArgs;
            }

            const char* GetQueryString(EAmpEncoding amp_enc, sStr& dst, EUrlEncode encode) const;
            const char* GetQueryString(EAmpEncoding amp_enc, sStr& dst, const IUrlEncoder* encoder = 0) const;

            bool IsSetValue(const char* name) const;

            const char* GetValue(const char* name) const;

            void SetValue(const char* name, const char* value);
            void SetValue(const char* name, const idx value)
            {
                sStr tmp("%" DEC, value);
                SetValue(name, tmp.ptr());
            }
            void SetValue(const char* name, const udx value)
            {
                sStr tmp("%" UDEC, value);
                SetValue(name, tmp.ptr());
            }

            bool HaveArgs(void) const;

            CUrl(const CUrl& url);
            CUrl& operator=(const CUrl& url);

            static IUrlEncoder* GetDefaultEncoder(void);

        private:
            const CUrlArgs* GetArgs(void) const;

            CUrlArgs* GetArgs(void);

            void x_SetScheme(const char* scheme, const IUrlEncoder&)
            {
                m_Scheme.replace(scheme);
            }
            void x_SetUser(const char* user, const IUrlEncoder& encoder)
            {
                encoder.DecodeUser(user, m_User);
            }
            void x_SetPassword(const char* password, const IUrlEncoder& encoder)
            {
                encoder.DecodePassword(password, m_Password);
            }
            void x_SetHost(const char* host, const IUrlEncoder&)
            {
                m_Host.replace(host);
            }
            void x_SetPort(const char* port, const IUrlEncoder&)
            {
                m_Port.replace(port);
            }
            void x_SetPath(const char* path, const IUrlEncoder& encoder)
            {
                encoder.DecodePath(path, m_Path);
            }
            void x_SetArgs(const char* args, const IUrlEncoder& encoder);

            void x_SetFragment(const char* fragment, const IUrlEncoder& encoder)
            {
                encoder.DecodeFragment(fragment, m_Fragment);
            }

            sStr m_Scheme;
            bool m_IsGeneric;
            sStr m_User;
            sStr m_Password;
            sStr m_Host;
            sStr m_Port;
            sStr m_Path;
            sStr m_Fragment;
            sStr m_OrigArgs;
            std::unique_ptr<CUrlArgs> m_ArgsList;
    };
}

#endif  