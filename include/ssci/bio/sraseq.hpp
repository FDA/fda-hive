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
#ifndef sSRA_seq_hpp
#define sSRA_seq_hpp

#include <ssci/bio/bioseq.hpp>

namespace slib {

    class sSRASeq: public sBioseq
    {
        public:
            sSRASeq(const char* path, udx cache_size = 200 * 1024 * 1024);
            virtual ~sSRASeq();
            virtual idx dim(void);
            virtual idx num(idx num, idx readtypes = 0);
            virtual idx len(idx num, idx iread = 1);
            virtual const char * seq(idx num, idx iread=0, idx ipos = 0, idx ilen = 0);
            virtual const char * id(idx num, idx iread = 0);
            virtual const char * qua(idx num, idx iread=0, idx ipos = 0, idx ilen = 0);

        private:
            const void* m_handle;
            sStr m_seq;
            sStr m_id;
            sStr m_qual;
    };
}

#endif 