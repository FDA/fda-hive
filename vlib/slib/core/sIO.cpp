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
#include <slib/core/sIO.hpp>

using namespace slib;


void sIO::callback( char * cont)
{
    if(!_funcCallback)return ;

    void * docut=0;
    if( !_funcAccumulator ) {
        if(_funcCallback==(callbackFun)::printf){
            //fprintf((FILE * )_funcCallbackParam, "%s", cont);
            fwrite(cont,(idx)(last()-cont),1,_funcCallbackParam ? (FILE * )_funcCallbackParam : stdout );
            if(!(flags&fDualOutput))
                cut(0);
        }
        else if(_funcCallback(this,_funcCallbackParam,cont)!=0) {
            if(!(flags&fDualOutput))cut(0);
        }
    }
    else {
        while( length() && (docut=_funcAccumulator(this, _funcCallbackParam , cont))!=0 ){
            char ch=*(char*)docut;
            *(char*)docut=0;
            _funcCallback(this,_funcCallbackParam,ptr(0));
            *(char*)docut=ch;

            idx nlen=length()-(idx)((char *)docut-ptr(0));
            memmove(ptr(0),docut,nlen+1);
            cut(nlen);
            cont=ptr(1);
        }
    }

}

