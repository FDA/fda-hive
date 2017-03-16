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
#include <slib/core/str.hpp>
#include <slib/std/cryptocipher.hpp>
#include <slib/std/cryptohash.hpp>
#include <slib/std/string.hpp>

#include "mbedtls/aes.h"

using namespace slib;

sAES::sAES()
{
    sSet(this, 0);
}

sAES::~sAES()
{
    mbedtls_aes_free((mbedtls_aes_context*)&_ctx);
}

void sAES::init(const void * key, sAES::EKeyBits key_len, bool for_encryption)
{
    mbedtls_aes_init((mbedtls_aes_context*)&_ctx);
    if( for_encryption ) {
        mbedtls_aes_setkey_enc((mbedtls_aes_context*)&_ctx, (const unsigned char *)key, key_len);
    } else {
        mbedtls_aes_setkey_dec((mbedtls_aes_context*)&_ctx, (const unsigned char *)key, key_len);
    }
    _for_encryption = for_encryption;
}

void sAES::processBlock(void * out, const void * in)
{
    if( _for_encryption ) {
        mbedtls_aes_encrypt((mbedtls_aes_context*)&_ctx, (const unsigned char *)in, (unsigned char *)out);
    } else {
        mbedtls_aes_decrypt((mbedtls_aes_context*)&_ctx, (const unsigned char *)in, (unsigned char *)out);
    }
}

sAESCBC::sAESCBC()
{
    sSetArray(_initial_iv);
    sSetArray(_current_iv);
}

void sAESCBC::initCtx(const void * key, sAES::EKeyBits key_len, bool for_encryption)
{
    _aes.init(key, key_len, for_encryption);
}

void sAESCBC::generateIV()
{
    sPassword::cryptoRandomBuf(_initial_iv, sAES::block_size);
    memcpy(_current_iv, _initial_iv, sAES::block_size);
}

const unsigned char * sAESCBC::getIV() const
{
    return _initial_iv;
}

void sAESCBC::setIV(const void * iv)
{
    memcpy(_initial_iv, iv, sAES::block_size);
    memcpy(_current_iv, _initial_iv, sAES::block_size);
}

void sAESCBC::processBlocks(void * out, const void * in, idx nblocks)
{
    mbedtls_aes_crypt_cbc((mbedtls_aes_context*)&_aes._ctx,
        _aes._for_encryption ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
        nblocks * sAES::block_size,
        _current_iv,
        (const unsigned char *)in,
        (unsigned char *)out);
}

//static
idx sBlockCrypto::encrypt(sMex * out, sBlockCrypto::ECryptoFormat out_mode, const void * in_plaintext, idx in_len, const void * key, idx key_len)
{
    if( !key_len ) {
        key_len = sLen(key);
    }
    if( !key_len ) {
        key = (unsigned char *)sStr::zero;
        key_len = 1;
    }
    if( !in_len ) {
        in_len = sLen(in_plaintext);
    }
    idx pos = out->pos();
    if( out_mode == eAES256_HMACSHA256 ) {
        sSHA256 key_hash(key, key_len);
        sHMAC<sSHA256> mac(key, key_len);

        idx nwhole_blocks = in_len / sAES::block_size;
        idx out_len = sAES::block_size * (nwhole_blocks + 2) + mac.getRawDigestSize();
        out->add(0, out_len);
        unsigned char * out_ptr = (unsigned char *)out->ptr(pos);

        sAESCBC aes_cbc;
        aes_cbc.initCtx(key_hash.getRawHash(), sAES::eAES256, true);

        // generate and write a random IV (must not be reused; must be saved for decryption; does not need to be secret)
        aes_cbc.generateIV();
        memcpy(out_ptr, aes_cbc.getIV(), sAES::block_size);
        out_ptr += sAES::block_size;

        // encrypt the whole blocks of ciphertext
        aes_cbc.processBlocks(out_ptr, in_plaintext, nwhole_blocks);
        out_ptr += nwhole_blocks * sAES::block_size;

        // create and encrypt the padding block - PKCS#7 algorithm
        unsigned char padding_block[sAES::block_size];
        int filler = sAES::block_size - in_len % sAES::block_size; // filler equals number of bytes of padding at the end of padding_block; always > 0
        memset(padding_block, filler, sAES::block_size);
        if( in_len % sAES::block_size ) {
            memcpy(padding_block, (const unsigned char *)in_plaintext + nwhole_blocks * sAES::block_size, in_len % sAES::block_size);
        }
        aes_cbc.processBlocks(out_ptr, padding_block, 1);
        out_ptr += sAES::block_size;

        // MAC : generated (effectively) from unhashed key + IV + ciphertext blocks
        mac.update(out->ptr(pos), sAES::block_size * (nwhole_blocks + 2));
        mac.finish();
        memcpy(out_ptr, mac.getRawDigest(), mac.getRawDigestSize());
        return out_len;
    } else {
        return -sIdxMax;
    }
}

//static
idx sBlockCrypto::decrypt(sMex * out, sBlockCrypto::ECryptoFormat out_mode, const void * in_ciphertext, idx in_len, const void * key, idx key_len)
{
    if( !key_len ) {
        key_len = sLen(key);
    }
    if( !key_len ) {
        key = (unsigned char *)sStr::zero;
        key_len = 1;
    }
    if( !in_len ) {
        in_len = sLen(in_ciphertext);
    }
    idx pos = out->pos();
    if( out_mode == eAES256_HMACSHA256 ) {
        sSHA256 key_hash(key, key_len);
        sHMAC<sSHA256> mac(key, key_len);

        // The input is expected to contain an IV block, 0 or more whole ciphertext blocks, a final ciphertext block
        // corresponding to padding, and a MAC. If the input is too short, it cannot possibly be valid
        if( in_len < 2 * sAES::block_size + mac.getRawDigestSize() || (in_len - mac.getRawDigestSize()) % sAES::block_size ) {
            return -sIdxMax;
        }

        // Verify that the MAC (generated from unhashed key + IV + ciphertext blocks) matches
        const unsigned char * in_mac = (const unsigned char *)in_ciphertext + in_len - mac.getRawDigestSize();
        mac.update(in_ciphertext, in_len - mac.getRawDigestSize());
        mac.finish();
        if( memcmp(in_mac, mac.getRawDigest(), mac.getRawDigestSize()) != 0 ) {
            return -sIdxMax;
        }

        idx nwhole_blocks = (in_len - 2 * sAES::block_size - mac.getRawDigestSize()) / sAES::block_size;
        out->add(0, nwhole_blocks * sAES::block_size);
        idx out_len = nwhole_blocks * sAES::block_size;
        unsigned char * out_ptr = (unsigned char *)out->ptr(pos);

        sAESCBC aes_cbc;
        aes_cbc.initCtx(key_hash.getRawHash(), sAES::eAES256, false);

        // read the IV
        aes_cbc.setIV(in_ciphertext);

        // decrypt the whole blocks of plaintext
        aes_cbc.processBlocks(out_ptr, (const unsigned char *)in_ciphertext + sAES::block_size, nwhole_blocks);

        // decrypt and read the padding block - PKCS#7 algorithm
        unsigned char padding_block[sAES::block_size];
        aes_cbc.processBlocks(padding_block, (const unsigned char *)in_ciphertext + (nwhole_blocks + 1 ) * sAES::block_size, 1);
        int filler = padding_block[sAES::block_size - 1]; // filler equals number of bytes of padding at the end of padding_block; always > 0
        if( filler < sAES::block_size ) {
            out->add(padding_block, sAES::block_size - filler);
            out_ptr = (unsigned char *)out->ptr();
            out_len += sAES::block_size - filler;
        }
        return out_len;
    } else {
        return -sIdxMax;
    }
}
