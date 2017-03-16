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

    /////////////////////////////////////////////////////////////////////////////
    ///
    /// IUrlEncoder::
    ///
    /// URL parts encoder/decoder interface. Used by CUrl.
    ///
    enum EUrlEncode
    {
        eUrlEncode_SkipMarkChars, ///< Do not convert chars like '!', '(' etc.
        eUrlEncode_ProcessMarkChars, ///< Convert all non-alphanum chars,
                                  ///< spaces are converted to '+'
        eUrlEncode_PercentOnly, ///< Convert all non-alphanum chars including
                             ///< space and '%' to %## format
        eUrlEncode_Path, ///< Same as ProcessMarkChars but preserves
                      ///< valid path characters ('/', '.')

        eUrlEncode_URIScheme, ///< Encode scheme part of an URI.
        eUrlEncode_URIUserinfo, ///< Encode userinfo part of an URI.
        eUrlEncode_URIHost, ///< Encode host part of an URI.
        eUrlEncode_URIPath, ///< Encode path part of an URI.
        eUrlEncode_URIQueryName, ///< Encode query part of an URI, arg name; ' is left unencoded.
        eUrlEncode_URIQueryNameEscapeQuotes, ///< Encode query part of an URI, arg name; ' is percent-encoded.
        eUrlEncode_URIQueryValue, ///< Encode query part of an URI, arg value; ' is left unencoded.
        eUrlEncode_URIQueryValueEscapeQuotes, ///< Encode query part of an URI, arg value; ' is percent-encoded.
        eUrlEncode_URIFragment, ///< Encode fragment part of an URI.

        eUrlEncode_ExtValue, ///< Encode RFC-2231 'extended-other-values', or RFC-5987 'ext-value' WITHOUT the "UTF-8''" prefix

        eUrlEncode_None
    ///< Do not encode
    };

    /// URL decode flags
    enum EUrlDecode
    {
        eUrlDecode_All, ///< Decode '+' to space
        eUrlDecode_Percent
    ///< Decode only %XX
    };

    /// Ampersand encoding for composed URLs
    enum EAmpEncoding
    {
        eAmp_Char, ///< Use & to separate arguments
        eAmp_Entity
    ///< Encode '&' as "&amp;"
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

            /// Encode user name
            virtual const char* EncodeUser(const char* user, sStr& dst) const = 0;
            /// Decode user name
            virtual const char* DecodeUser(const char* user, sStr& dst) const = 0;
            /// Encode password
            virtual const char* EncodePassword(const char* password, sStr& dst) const = 0;
            /// Decode password
            virtual const char* DecodePassword(const char* password, sStr& dst) const = 0;
            /// Encode path on server
            virtual const char* EncodePath(const char* path, sStr& dst) const = 0;
            /// Decode path on server
            virtual const char* DecodePath(const char* path, sStr& dst) const = 0;
            /// Encode URL argument name
            virtual const char* EncodeArgName(const char* name, sStr& dst) const = 0;
            /// Decode URL argument name
            virtual const char* DecodeArgName(const char* name, sStr& dst) const = 0;
            /// Encode URL argument value
            virtual const char* EncodeArgValue(const char* value, sStr& dst) const = 0;
            /// Decode URL argument value
            virtual const char* DecodeArgValue(const char* value, sStr& dst) const = 0;
            /// Encode fragment
            virtual const char* EncodeFragment(const char* value, sStr& dst) const = 0;
            /// Decode fragment
            virtual const char* DecodeFragment(const char* value, sStr& dst) const = 0;
    };

    /// Primitive encoder - all methods return the argument value.
    /// Used as base class for other encoders.
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

    /// Default encoder, uses the selected encoding for argument names/values
    /// and eUrlEncode_Path for document path. Other parts of the URL are
    /// not encoded.
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

    /////////////////////////////////////////////////////////////////////////////
    ///
    /// CUrl::
    ///
    /// URL parser. Uses CUrlArgs to parse arguments.
    ///
    class CUrlArgs;

    class CUrl
    {
        public:
            /// Default constructor
            CUrl(void);

            /// Parse the URL.
            ///
            /// @param url
            ///   String to parse as URL:
            ///   Generic: [scheme://[user[:password]@]]host[:port][/path][?args]
            ///   Special: scheme:[path]
            ///   The leading '/', if any, is included in path value.
            /// @param encoder
            ///   URL encoder object. If not set, the default encoder will be used.
            ///   @sa CDefaultUrlEncoder
            CUrl(const char* url, const IUrlEncoder* encoder = 0);

            ~CUrl();

            /// Parse the URL.
            ///
            /// @param url
            ///   String to parse as URL
            /// @param encoder
            ///   URL encoder object. If not set, the default encoder will be used.
            ///   @sa CDefaultUrlEncoder
            bool SetUrl(const char* url, const IUrlEncoder* encoder = 0);

            /// Compose the URL.
            ///
            /// @param amp_enc
            ///   Method of encoding ampersand.
            ///   @sa EAmpEncoding
            /// @param encoder
            ///   URL encoder object. If not set, the default encoder will be used.
            ///   @sa CDefaultUrlEncoder
            const char* ComposeUrl(EAmpEncoding amp_enc, sStr& dst, const IUrlEncoder* encoder = 0) const;

            // Access parts of the URL

            const sStr& GetScheme(void) const
            {
                return m_Scheme;
            }
            void SetScheme(const sStr& value)
            {
                m_Scheme.replace(value);
            }

            /// Generic schemes use '//' after scheme name and colon.
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

            /// Get the original (unparsed and undecoded) query sStr
            const sStr& GetOriginalArgsString(void) const
            {
                return m_OrigArgs;
            }

            /// Construct and return complete query string. Use selected amp
            /// and name/value encodings.
            const char* GetQueryString(EAmpEncoding amp_enc, sStr& dst, EUrlEncode encode) const;
            /// Construct and return complete query string. Use selected amp
            /// and name/value encodings.
            const char* GetQueryString(EAmpEncoding amp_enc, sStr& dst, const IUrlEncoder* encoder = 0) const;

            /// Check if an argument with the given name exists.
            bool IsSetValue(const char* name) const;

            /// Get value for the given name. finds first of the arguments with the
            /// given name. If the name does not exist, is_found is set to false.
            /// If is_found is null, CUrlArgsException is thrown.
            const char* GetValue(const char* name) const;

            /// Set new value for the first argument with the given name or
            /// add a new argument.
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

            /// Check if the URL contains any arguments
            bool HaveArgs(void) const;

            CUrl(const CUrl& url);
            CUrl& operator=(const CUrl& url);

            /// Return default URL encoder.
            ///
            /// @sa CDefaultUrlEncoder
            static IUrlEncoder* GetDefaultEncoder(void);

        private:
            /// Get const list of arguments
            const CUrlArgs* GetArgs(void) const;

            /// Get list of arguments
            CUrlArgs* GetArgs(void);

            // Set values with verification
            void x_SetScheme(const char* scheme, const IUrlEncoder& /* encoder */)
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
            void x_SetHost(const char* host, const IUrlEncoder& /* encoder */)
            {
                m_Host.replace(host);
            }
            void x_SetPort(const char* port, const IUrlEncoder& /* encoder */)
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
            bool m_IsGeneric; // generic schemes include '//' delimiter
            sStr m_User;
            sStr m_Password;
            sStr m_Host;
            sStr m_Port;
            sStr m_Path;
            sStr m_Fragment;
            sStr m_OrigArgs;
            std::auto_ptr<CUrlArgs> m_ArgsList;
    };
}

#endif  // sLib_std_url_hpp
