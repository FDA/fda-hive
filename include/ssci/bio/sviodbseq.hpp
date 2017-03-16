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
#ifndef sBio_sVioDBseq_hpp
#define sBio_sVioDBseq_hpp

#include <slib/core.hpp>
#include <slib/std.hpp>
#include <slib/utils.hpp>

#define ID_TYPE 1
#define REC_TYPE 2 
#define SEQ_TYPE 3
#define QUA_TYPE 4 

class sVioDBseq
{
    private:
        sVioDB *db;
        idx     mode;
        idx     length;

    public:
        sVioDBseq (sVioDB *database){
            db = database;
            mode = 0;    // This means that the sorted and short list is reference by default
        };
        void setmode(idx mod){
            mode = mod;
            return ;
        };
        idx getmode (){
            return mode;
        };
        idx getnumseq(){
            if (mode)
                length = db->GetRecordCnt(ID_TYPE);
            else
                length = db->GetRecordCnt(REC_TYPE);
            return  length;
        }

        idx getLen(idx i);
        idx getCount(idx i);
        const char *getSeq(idx i);
        const char *getQua(idx i);
        const char *getID(idx i);
        
};  
#endif // sBio_sVioDBseq_hpp

