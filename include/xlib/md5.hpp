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
#ifndef xLib_md5_h
#define xLib_md5_h

#include <slib/core/def.hpp>

namespace slib {
    class sMD5
    {
            static const idx MD5size = (16);

        public:
            sMD5(const char * filename)
            {
                sSet(bin);
                sum[0] = 0;
                parse_file(filename, 0);
            }
            sMD5(FILE * fp)
            {
                sSet(bin);
                sum[0] = 0;
                parse_stream(fp, 0);
            }
            sMD5(const char * buffer, idx len)
            {
                sSet(bin);
                sum[0] = 0;
                parse_buffer(buffer, len, 0);
            }

            char bin[MD5size];
            char sum[MD5size * 4];

            void * parse_buffer(const char *buffer, idx len, void * resblock = 0);
            int parse_stream(FILE *stream, void *resblock = 0);
            int parse_file(const char * filename, void * resblock = 0);

        private:
            void * md5_finish_ctx(void *ctx, void *resbuf);
            void * md5_read_ctx(const void *ctx, void *resbuf);

            static void md5_init_ctx(void *ctx);
            static void md5_process_block(const void *buffer, idx len, void *ctx);
            static void md5_process_bytes(const void *buffer, idx len, void *ctx);
    };

}
;

#endif // xLib_md5_h

