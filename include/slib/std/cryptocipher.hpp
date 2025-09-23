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
    class sAES {
        public:
            enum EKeyBits {
                eAES128 = 128,
                eAES192 = 192,
                eAES256 = 256
            };
            static const idx block_size = 16;

            sAES();
            ~sAES();
            void init(const void * key, EKeyBits key_len, bool for_encryption);
            void processBlock(void * out, const void * in);

        private:
            struct {
                int nr;
                uint32_t *rk;
                uint32_t buf[68];
            } _ctx;
            bool _for_encryption;

            friend class sAESCBC;
    };
    class sAESCBC {
        public:
            sAESCBC();
            void initCtx(const void * key, sAES::EKeyBits key_len, bool for_encryption);
            void generateIV();
            const unsigned char * getIV() const;
            void setIV(const void * iv);
            void processBlocks(void * out, const void * in, idx nblocks);

        private:
            unsigned char _initial_iv[sAES::block_size], _current_iv[sAES::block_size];
            sAES _aes;
    };

    class sBlockCrypto {
        public:
            enum ECryptoFormat {
                eAES256_HMACSHA256
            };

            static idx encrypt(sMex * out, ECryptoFormat fmt, const void * in_plaintext, idx in_len, const void * key, idx key_len);

            static idx decrypt(sMex * out, ECryptoFormat fmt, const void * in_encrypted, idx in_len, const void * key, idx key_len);
    };
};
#endif
