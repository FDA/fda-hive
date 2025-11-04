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
#include <xlib/sha256.hpp>

using namespace slib;

#define sSHA256_NEW 0
#define sSHA256_ADD 1
#define sSHA256_SUM 2
#define sSHA256_ERR 3

sSHA_256::~sSHA_256()
{
    if( m_state < sSHA256_SUM ) {
        sum();
    }
}

bool sSHA_256::parse_stream(FILE * stream)
{
    if( m_state <= sSHA256_ADD ) {
        const idx blk_sz = 4096 * 64;
        char buffer[blk_sz];
        size_t sum;
        while( true ) {
            size_t n;
            sum = 0;
            do {
                n = fread(buffer + sum, 1, blk_sz - sum, stream);
                sum += n;
            } while( sum < blk_sz && n != 0 );
            if( n == 0 && ferror(stream) ) {
                m_state = sSHA256_ERR;
                return false;
            }
            if( !parse_buffer(buffer, sum) ) {
                return false;
            }
            if( n == 0 ) {
                break;
            }
        }
        return true;
    }
    return false;
}

bool sSHA_256::parse_file(const char * filename)
{
    FILE * fp = fopen(filename, "r");
    bool res = fp != NULL;
    if( res ) {
        res = parse_stream(fp);
    }
    fclose(fp);
    return res;
}

bool sSHA_256::parse_buffer(const char * buffer, idx len)
{
    switch( m_state ) {
        case sSHA256_NEW:
            if( SHA256_Init(&m_ctx) != 1 ) {
                m_state = sSHA256_ERR;
                break;
            }
            m_state = sSHA256_ADD;
        case sSHA256_ADD:
            if( SHA256_Update(&m_ctx, buffer, len) != 1 ) {
                m_state = sSHA256_ERR;
            }
    }
    return m_state != sSHA256_ERR;
}

const char * sSHA_256::sum(sStr * buffer)
{
    idx pos = 0;
    if( buffer ) {
        pos = buffer->pos();
    }
    static char buf[SHA256_DIGEST_LENGTH * 2 + 10];
    switch( m_state ) {
        case sSHA256_NEW:
            if( SHA256_Init(&m_ctx) != 1 ) {
                m_state = sSHA256_ERR;
                break;
            }
            m_state = sSHA256_ADD;
        case sSHA256_ADD:
            if( SHA256_Final(m_hash, &m_ctx) != 1 ) {
                m_state = sSHA256_ERR;
                break;
            }
            m_state = sSHA256_SUM;
        case sSHA256_SUM:
            for( idx i = 0; i < SHA256_DIGEST_LENGTH; ++i ) {
                sprintf(&buf[i * 2], "%02x", m_hash[i]);
            }
            buf[SHA256_DIGEST_LENGTH * 2] = '\0';
            if( buffer ) {
                buffer->printf("%s", buf);
            }
    }
    return (m_state != sSHA256_ERR) ? (buffer ? buffer->ptr(pos) : buf) : NULL;
}
