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

#include <slib/core.hpp>
#include <slib/std/cryptohash.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace slib;

static void cryptoRandomStringNonLinux(unsigned char * out, idx len, const char * charlist, idx charlist_len)
{
    static bool have_seeded = false;
    if( !have_seeded ) {
        sSHA256 sha;
        time_t t;
        time(&t);
        sha.update((const char *)&t, sizeof(t));
        pid_t p = getpid();
        sha.update((const char *)&p, sizeof(p));
        const char* e = getenv("UNIQUE_ID");
        if( e && *e ) {
            sha.update(e);
        }
        sha.finish();
        unsigned int seed = 0;
        unsigned char * sha_raw_hash = (unsigned char *)sha.getRawHash();
        for(idx i = 0; i < (idx)sizeof(seed); i++) {
            seed |= sha_raw_hash[i] << (i * 8);
        }
        srand(seed);
        have_seeded = true;
    }

    for(idx i=0; i<len; i++) {
        if( charlist ) {
            out[i] = charlist[rand() % charlist_len];
        } else {
            out[i] = rand() % 256;
        }
    }
}

//static
const char * sPassword::cryptoRandomString(sStr & out, idx len, const char * charlist/* = 0 */)
{
    idx charlist_len;
    if( charlist ) {
        charlist_len = sLen(charlist);
    } else {
        static const char nice_salt_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
        static const idx num_nice_salt_chars = sizeof(nice_salt_chars) - 1;

        charlist = nice_salt_chars;
        charlist_len = num_nice_salt_chars;
    }

    idx pos = out.length();
    out.add(0, len);

#ifdef SLIB_LINUX
    int urandom = open("/dev/urandom", O_RDONLY);
    if( urandom >= 0 ) {
        udx total = 0;

        for(idx i=0; i<len; i++) {
            unsigned char c;
            read(urandom, &c, 1);
            total += c;
            out[pos + i] = charlist[total % charlist_len];
        }
        close(urandom);
    } else {
        cryptoRandomStringNonLinux((unsigned char*)out.ptr(pos), len, charlist, charlist_len);
    }
#else
    cryptoRandomStringNonLinux((unsigned char*)out.ptr(pos), len, chars, chars_len);
#endif
    out.add0cut();
    return out.ptr(pos);
}

//static
const unsigned char * sPassword::cryptoRandomBuf(unsigned char * out, idx len)
{
#ifdef SLIB_LINUX
    int urandom = open("/dev/urandom", O_RDONLY);
    if( urandom >= 0 ) {
        idx total = 0;
        while( total < len ) {
            idx bytes_read = read(urandom, out + total, len - total);
            if( bytes_read > 0 ) {
                total += bytes_read;
            } else {
                cryptoRandomStringNonLinux(out + total, len - total, 0, 0);
                break;
            }
        }
        close(urandom);
    } else {
        cryptoRandomStringNonLinux(out, len, 0, 0);
    }
#else
    cryptoRandomStringNonLinux(out, len, 0, 0);
#endif
    return out;
}

//static
const char * sPassword::encodePassword(sStr & out, const char * pass, const char * salt/* = 0 */)
{
    idx pos = out.length();
    idx iterations = sPBKDF2_HMAC<sSHA256>::default_iterations;
    out.printf("pbkdf2_sha256$%"DEC"$", iterations);
    idx salt_pos = out.length();
    idx salt_len = sLen(salt);
    if( salt_len ) {
        out.addString(salt);
    } else {
        salt_len = 12;
        cryptoRandomString(out, salt_len);
    }
    out.addString("$");
    sPBKDF2_HMAC<sSHA256> pbkdf2(pass, out.ptr(salt_pos), salt_len, iterations);
    pbkdf2.printBase64(out);
    return out.ptr(pos);
}

//static
bool sPassword::checkPassword(const char * hash, idx hash_len, const char * pass, bool * out_need_upgrade/* = 0 */)
{
    // characters that cannot occur within a password hash, and that separate multiple password hashes in a list
    if( !hash ) {
        return false;
    }

    static const char * hash_seps = " \t\r\n";
    if( !hash_len ) {
        hash_len = strcspn(hash, hash_seps);
    }

    // Format: "pbkdf2_sha256" '$' iterations '$' salt '$' digest; assume > 32 bytes long
    if( hash_len <= 32 || strncmp(hash, "pbkdf2_sha256$", 14) != 0 ) {
        return false;
    }

    const char * s = hash + 14;
    idx iterations = atoidx(s);
    if( iterations < 1 ) {
        return false;
    }
    if( out_need_upgrade ) {
        *out_need_upgrade = (iterations < sPBKDF2_HMAC<sSHA256>::default_iterations);
    }
    s = (const char *)memchr(s, '$', hash + hash_len - s);
    if( !s ) {
        return false;
    }
    const char * salt = ++s;
    if( salt >= hash + hash_len || !*salt ) {
        return false;
    }
    s = (const char *)memchr(s, '$', hash + hash_len - s);
    if( !s ) {
        return false;
    }
    const char * digest = ++s;
    if( digest >= hash + hash_len || !*digest ) {
        return false;
    }
    idx digest_len = hash + hash_len - digest;
    sPBKDF2_HMAC<sSHA256> pbkdf2(pass, salt, digest - 1 - salt, iterations);
    sStr buf;
    if( strncmp(pbkdf2.printBase64(buf), digest, digest_len) == 0 ) {
        return true;
    }

    return false;
}
