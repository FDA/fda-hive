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
#ifndef sLib_std_gate_hpp
#define sLib_std_gate_hpp

#include <slib/std/file.hpp>

namespace slib 
{ 

    class sGate 
    {
    private: 
        idx gateWait;
        sStr dir;
    public:
        sGate ( idx lgateWait=0, const char * ldir=0) {init(lgateWait,ldir);}
        ~sGate( ){}

        void init  ( idx lgateWait=0, const char * ldir=0, ... ) 
        {
            if(ldir){sCallVarg(dir.vprintf,ldir);}
            gateWait=lgateWait;
        }
    private: 
        
        void keyname( sStr * nm, const void * key, idx keysize) ;

    public:
        idx lock ( const void * key, idx keysize, idx jobId=0 ) ;
        idx unlock ( const void * key, idx keysize, idx jobId=0) ;


        idx lock ( idx jobId, const char * fmt , ...  ) 
            {   sStr str;sCallVarg(str.vprintf,fmt);return lock ((const void*)str.ptr(), str.length() , jobId);}
        idx unlock ( idx jobId, const char * fmt, ... ) 
            {   sStr str;sCallVarg(str.vprintf,fmt);return unlock ( (const void*)str.ptr(), str.length() , jobId);}
        template <class Tobj >  idx lock ( Tobj & key, idx jobId ) // getpid() 
            {return lock ((const void*)&key, sizeof(key) , jobId);}
        template <class Tobj >  idx unlock ( Tobj & key, idx jobId ) 
            {return unlock ((const void*)&key, sizeof(key) , jobId);}
       

    };



}
#endif // sLib_std_gate_hpp




