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
#ifndef sLib_cryptocipher_hpp
#define sLib_cryptocipher_hpp

#include <stdint.h>

namespace slib {
    class sAESCBC;
    //! Low-level interfaace to the AES block cipher. Not recommended for direct use; just use sEncrypter
    class sAES {
        public:
            enum EKeyBits {
                eAES128 = 128,
                eAES192 = 192,
                eAES256 = 256
            };
            static const idx block_size = 16; //!< block size in bytes

            sAES();
            ~sAES();
            void init(const void * key, EKeyBits key_len, bool for_encryption);
            void processBlock(void * out, const void * in);

        private:
            // keep in sync with aes.h !!!
            struct {
                int nr;
                uint32_t *rk;
                uint32_t buf[68];
            } _ctx;
            // our own fields follow below
            bool _for_encryption;

            friend class sAESCBC;
    };
    //! Low-level interface to AES-CBC. Not recommended for direct use; just use sEncrypter
    class sAESCBC {
        public:
            sAESCBC();
            //! set the key
            /*! \param key binary key of appropriate length (128, 192, or 256 bits) */
            void initCtx(const void * key, sAES::EKeyBits key_len, bool for_encryption);
            //! set initialization vector to a random value from /dev/urandom
            void generateIV();
            //! get the initial initialization vector
            const unsigned char * getIV() const;
            //! change the initialization vector
            void setIV(const void * iv);
            //! encrypt/decrypt some blocks of plaintext into the output buffer
            /*! \param[out] out buffer into which to write
                \param in data to encrypt/decrypt
                \param nblocks number of blocks to encrypt/decrypt */
            void processBlocks(void * out, const void * in, idx nblocks);

        private:
            unsigned char _initial_iv[sAES::block_size], _current_iv[sAES::block_size];
            sAES _aes;
    };

    //! High-level block encryption interface
    class sBlockCrypto {
        public:
            enum ECryptoFormat {
                eAES256_HMACSHA256 //!< 16-byte IV, then cryptotext in 16-byte blocks (plaintext is padded using PKCS#7, see RFC-5652), then HMAC-SHA256 of (unashed key + IV + cryptotext)
            };

            //! Encrypt some input in specified format
            /*! \param[out] out mex to which to append the encrypted result
                \param fmt encrypted data format
                \param plaintext input data to encrypt
                \param plaintext_len length of input, or 0 if the plaintext is a 0-terminated string
                \param key encryption key (of arbitrary length; will be hashed using SHA-256 to get 256 bits)
                \param key_len encryption key length
                \returns number of bytes encrypted, or negative value on failure; note that 0 is a valid, non-error return value */
            static idx encrypt(sMex * out, ECryptoFormat fmt, const void * in_plaintext, idx in_len, const void * key, idx key_len);

            //! Decrypt some input in specified format
            /*! \param[out] out mex to which to append the plaintext result
                \param fmt encrypted data format
                \param in_encrypted input data to decrypt
                \param in_len length of input, or 0 if the input is a 0-terminated string
                \param key decryption key (of arbitrary length; will be hashed using SHA-256 to get 256 bits)
                \param key_len decryption key length, or 0 if it is a 0-terminated string
                \returns number of bytes decrypted, or negative value on failure; note that 0 is a valid, non-error return value */
            static idx decrypt(sMex * out, ECryptoFormat fmt, const void * in_encrypted, idx in_len, const void * key, idx key_len);
    };
};
#endif
