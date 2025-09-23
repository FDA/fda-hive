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
#ifndef sLib_core_synonym_hpp
#define sLib_core_synonym_hpp

#include <slib/core/str.hpp>
#include <slib/core/dic.hpp>

namespace slib 
{ 

    class sSynonym {

    private:
        sMex _buf;
        sDic< idx> _ofs;
    public:    
        sSynonym() {}

        void set(const char * data00, idx datasize, const char * dicOn00, sString::eCase caseLOUP=sString::eCaseNone)
        {
            sStr tmp;
            idx ofs=_buf.pos();
            _buf.add(data00,datasize ? datasize : sString::size00(data00));
            idx num=_ofs.dim();
            *(_ofs.add(1))=ofs;
            for(const char * ptr=dicOn00; ptr ; ptr=sString::next00(ptr,1)) {
                if(caseLOUP!=sString::eCaseNone){
                    sString::changeCase(&tmp,ptr,0,caseLOUP);
                    _ofs.dict(num,tmp.ptr());tmp.cut(0);
                }else _ofs.dict(num,ptr);
            }
        }

        char * get(idx which, const char * nam, idx len=0, sString::eCase caseLOUP=sString::eCaseNone)
        {
            idx*p;
            if(caseLOUP!=sString::eCaseNone){
                sStr tmp; sString::changeCase(&tmp,nam,len,caseLOUP);
                p=_ofs.get(tmp.ptr(),len);
            }
            else p=_ofs.get((const char *)nam,len);
            if(!p)return 0;
            return sString::next00((const char *)_buf.ptr(*p),which);
        }
        idx cnt(void){return _ofs.dim();}

    };            


}
#endif




