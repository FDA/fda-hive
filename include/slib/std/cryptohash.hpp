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
#ifndef sLib_std_cryptohash_hpp
#define sLib_std_cryptohash_hpp

#include <slib/core/def.hpp>
#include <slib/core/str.hpp>
#include <slib/std/string.hpp>

#include <stdint.h>

namespace slib {
    //! CRTP-style base class for cryptographic hash functions
    /*! \tparam Thash CRTP child class implementing *_impl() methods, and providing output_size and block_size static values */
    template <class Thash>
    class sCryptoHash
    {
        public:
            void init()
            {
                sSetArray(static_cast<const Thash*>(this)->_output_bin);
                static_cast<Thash*>(this)->init_impl();
            }
            void update(const void * buffer, idx len)
            {
                static_cast<Thash*>(this)->update_impl(buffer, len);
            }
            void update(const char * buffer)
            {
                update(buffer, sLen(buffer));
            }
            void finish()
            {
                static_cast<Thash*>(this)->finish_impl();
            }
            void parseBuffer(const void * buffer, idx len)
            {
                init();
                update(buffer, len);
                finish();
            }
            void parseString(const char * str)
            {
                parseBuffer(str, sLen(str));
            }
            bool parseFile(const char * path)
            {
                init();
                sFil fil(path, sMex::fReadonly);
                if( fil.ok() ) {
                    if( fil.length() ) {
                        update(fil.ptr(), fil.length());
                    }
                    finish();
                    return true;
                } else {
                    return false;
                }
            }
            static idx getRawHashSize()
            {
                return Thash::output_size;
            }
            //! Get pointer to hash in binary form
            const void * getRawHash() const
            {
                return static_cast<const Thash*>(this)->_output_bin;
            }
            const char * printHex(sStr & out) const
            {
                idx pos = out.length();
                for(idx i=0; i<Thash::output_size; i++) {
                    int b = static_cast<const Thash*>(this)->_output_bin[i];
                    out.printf("%02x", b);
                }
                return out.ptr(pos);
            }
    };

    //! SHA-1 hash (FIPS 180-4)
    /*! Basic usage:
     * \code
         sStr buf;
         sSHA1 sha("/tmp/foobar.fasta");
         printf("sha1sum = %s\n", sha.printHex(buf));
     * \endcode
     */
    class sSHA1 : public sCryptoHash<sSHA1>
    {
        public:
            static const idx output_size = 20; //!< output raw checksum length in bytes
            static const idx block_size = 64; //!< input block size in bytes

            sSHA1()
            {
                init();
            }
            sSHA1(const char * filename)
            {
                parseFile(filename);
            }
            //! \warning len must be specified to ensure safe usage by consumers; len = 0 is interpreted as empty buffer
            sSHA1(const void * buffer, idx len)
            {
                parseBuffer(buffer, len);
            }

            struct Context {
                union {
                    uint8_t    b8[20];
                    uint32_t   b32[5];
                } h;
                union {
                    uint8_t    b8[8];
                    uint64_t   b64[1];
                } c;
                union {
                    uint8_t    b8[64];
                    uint32_t   b32[16];
                } m;
                uint8_t count;
            };

        private:
            friend class sCryptoHash<sSHA1>;

            unsigned char _output_bin[output_size];
            Context _context;

            void init_impl();
            void update_impl(const void * buffer, idx len);
            void finish_impl();
    };

    //! SHA-256 (SHA-2 family) hash (FIPS 180-4)
    /*! Basic usage:
     * \code
         sStr buf;
         sSHA256 sha("/tmp/foobar.fasta");
         printf("sha256sum = %s\n", sha.printHex(buf));
     * \endcode
     */
    class sSHA256 : public sCryptoHash<sSHA256>
    {
        public:
            static const idx output_size = 32; //!< output raw checksum length in bytes
            static const idx block_size = 64; //!< input block size in bytes

            sSHA256()
            {
                init();
            }
            sSHA256(const char * filename)
            {
                parseFile(filename);
            }
            //! \warning len must be specified to ensure safe usage by consumers; len = 0 is interpreted as empty buffer
            sSHA256(const void * buffer, idx len)
            {
                parseBuffer(buffer, len);
            }

            struct Context {
                uint32_t state[8];
                uint64_t count;
                uint8_t buf[64];
            };

        private:
            friend class sCryptoHash<sSHA256>;

            unsigned char _output_bin[output_size];
            Context _context;

            void init_impl();
            void update_impl(const void * buffer, idx len);
            void finish_impl();
    };

    //! Hash-based message authentication code (RFC 2104)
    /*! \tparam Thash hash function (derived from sCryptoHash)
     *
     * Basic usage:
     * \code
         sStr buf;
         sHMAC<> hmac("secret", 6);
         hmac.update("message part 1");
         hmac.update("message part 2");
         hmac.finish();
         printf("hmac digest = %s\n", hmac.printBase64(buf));
     * \endcode
     */
    template <class Thash = sSHA256>
    class sHMAC
    {
        public:
            sHMAC<Thash>(const void * key, idx key_len)
            {
                init(key, key_len);
            }
            sHMAC<Thash>(const void * key, idx key_len, const void * msg, idx msg_len)
            {
                init(key, key_len);
                update(msg, msg_len);
                finish();
            }
            sHMAC<Thash>(const void * key, idx key_len, const char * msg)
            {
                init(key, key_len);
                update(msg);
                finish();
            }

            void init(const void * key, idx key_len)
            {
                if( key_len > Thash::block_size ) {
                    _ohash.parseBuffer((const char *)key, key_len);
                    key = _ohash.getRawHash();
                    key_len = _ohash.getRawHashSize();
                }

                unsigned char key_pad[Thash::block_size];
                sSetArray(key_pad);
                memcpy(key_pad, key, key_len);
                for(idx i=0; i<Thash::block_size; i++) {
                    key_pad[i] ^= 0x36;
                }
                _ihash.init();
                _ihash.update(key_pad, Thash::block_size);

                sSetArray(key_pad);
                memcpy(key_pad, key, key_len);
                for(idx i=0; i<Thash::block_size; i++) {
                    key_pad[i] ^= 0x5c;
                }
                _ohash.init();
                _ohash.update(key_pad, Thash::block_size);
            }
            //! \warning msg_len must be specified to ensure safe usage by consumers; msg_len = 0 is interpreted as empty buffer
            void update(const void * msg, idx msg_len)
            {
                _ihash.update(msg, msg_len);
            }
            void update(const char * msg)
            {
                _ihash.update(msg, sLen(msg));
            }
            void finish()
            {
                _ihash.finish();
                _ohash.update(_ihash.getRawHash(), _ihash.getRawHashSize());
                _ohash.finish();
            }

            static idx getRawDigestSize()
            {
                return Thash::output_size;
            }
            const void * getRawDigest() const
            {
                return _ohash.getRawHash();
            }
            const char * printHex(sStr & out) const
            {
                return _ohash.printHex(out);
            }
            const char * printBase64(sStr & out) const
            {
                idx pos = out.length();
                sString::encodeBase64(&out, (const char *)getRawDigest(), getRawDigestSize());
                out.add0cut(2);
                return out.ptr(pos);
            }

        private:
            Thash _ihash, _ohash;
    };

    //! RSA Password-Based Key Derivation Function 2 (RFC 2898)
    /*! \tparam Thash hash function (derived from sCryptoHash)
     *
     * Basic usage:
     * \code
         sStr buf;
         sPBKDF2_HMAC<> pbkdf2("password", "salt", 4);
         printf("pbkdf2 digest = %s\n", pbkdf2.printBase64(buf));
     * \endcode
     */
    template <class Thash = sSHA256>
    class sPBKDF2_HMAC
    {
        public:
            // https://docs.python.org/3/library/hashlib.html : "as of 2013, at least 100,000 rounds of SHA-256 is suggested"
            static const idx default_iterations = 100000;

            sPBKDF2_HMAC<Thash>()
            {
                _salt_buf.init(sMex::fExactSize);
            }

            sPBKDF2_HMAC<Thash>(const char * pass, idx pass_len, const void * salt, idx salt_len, idx iterations = default_iterations)
            {
                _salt_buf.init(sMex::fExactSize);
                encode(pass, pass_len, salt, salt_len, iterations);
            }

            sPBKDF2_HMAC<Thash>(const char * pass, const void * salt, idx salt_len, idx iterations = default_iterations)
            {
                _salt_buf.init(sMex::fExactSize);
                encode(pass, salt, salt_len, iterations);
            }

            void encode(const char * pass, idx pass_len, const void * salt, idx salt_len, idx iterations = default_iterations)
            {
                _salt_buf.resize(salt_len + 4);
                memcpy(_salt_buf.ptr(), salt, salt_len);
                _salt_buf[salt_len + 0] = (((uint32_t)1) >> 24) & 0xff;
                _salt_buf[salt_len + 1] = (((uint32_t)1) >> 16) & 0xff;
                _salt_buf[salt_len + 2] = (((uint32_t)1) >> 8) & 0xff;
                _salt_buf[salt_len + 3] = ((uint32_t)1) & 0xff;
                sHMAC<Thash> hmac(pass, pass_len, _salt_buf.ptr(), salt_len + 4);
                memcpy(_output_bin, hmac.getRawDigest(), Thash::output_size);
                unsigned char prev_bin[Thash::output_size];
                memcpy(prev_bin, hmac.getRawDigest(), Thash::output_size);
                for(idx i=1; i<iterations; i++) {
                    hmac.init(pass, pass_len);
                    hmac.update((const char *)prev_bin, Thash::output_size);
                    hmac.finish();
                    memcpy(prev_bin, hmac.getRawDigest(), Thash::output_size);
                    for(idx j=0; j<Thash::output_size; j++) {
                        _output_bin[j] ^= prev_bin[j];
                    }
                }
            }
            void encode(const char * pass, const void * salt, idx salt_len, idx iterations = default_iterations)
            {
                return encode(pass, sLen(pass), salt, salt_len, iterations);
            }

            static idx getRawDigestSize()
            {
                return Thash::output_size;
            }
            const void * getRawDigest() const
            {
                return _output_bin;
            }
            const char * printHex(sStr & out) const
            {
                idx pos = out.length();
                for(idx i=0; i<Thash::output_size; i++) {
                    int b = _output_bin[i];
                    out.printf("%02x", b);
                }
                return out.ptr(pos);
            }
            const char * printBase64(sStr & out) const
            {
                idx pos = out.length();
                sString::encodeBase64(&out, (const char *)_output_bin, Thash::output_size);
                out.add0cut(2);
                return out.ptr(pos);
            }

        private:
            sStr _salt_buf;
            unsigned char _output_bin[Thash::output_size];
    };

    class sPassword {
        public:
            //! Generate cryptographically strong random string of specified length, e.g. for password hash salt
            /*! \param charset 0-terminated list of characters to use for the string; a-zA-Z0-9_ by default */
            static const char * cryptoRandomString(sStr & out, idx len, const char * charlist = 0);
            //! Generate cryptographically strong random binary buffer of specified length, e.g. for password hash salt
            /*! \param out buffer into which to print (must be large enough!)
                \param len number of bytes to retrieve */
            static const unsigned char * cryptoRandomBuf(unsigned char * out, idx len);
            //! Hash a password using PBKDF2-SHA256
            /*! Output is in the usual $-delimited format, as used in e.g. Django:
               "pbkdf2_sha256" '$' iterations '$' salt '$' digest
               \param[out] out buffer into which to print
               \param pass plain-text password
               \param salt use specified salt, or generate new random salt if 0 */
            static const char * encodePassword(sStr & out, const char * pass, const char * salt = 0);
            //! Verify whether a password matches a PBKDF2-SHA256 hash
            /*! Output format: "pbkdf2_sha256" '$' iterations '$' salt '$' digest
               \param hash password hash in the usual $-delimited format ("pbkdf2_sha256" '$' iterations '$' salt '$' digest)
               \param hash_len length of hash (or 0 to assume it's a whitespace-delimeted list)
               \param pass plain-text password
               \param[out] out_need_upgrade will be set to true if hash is detected as old or weak and needs to be re-hashed */
            static bool checkPassword(const char * hash, idx hash_len, const char * pass, bool * out_need_upgrade = 0);
    };
};

#endif
