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
#include <slib/std/gate.hpp>

using namespace slib;

void sGate::keyname( sStr * nm, const void * key, idx keysize) 
{
    nm->add(dir);
    const char * hx="0123456789ABCDEF";
    idx i;
    char * ls=nm->last();
    if( (*ls)!='/' && *(ls)!='\\') nm->add("/",1);
    for(i=0; (i<keysize) ; ++i ) {
        char ch=((char*)key)[i];
        nm->add(&hx[(ch&0x0F)],1);
        nm->add(&hx[((ch>>4)&0x0F)],1);
    }
    nm->add(_,1);
}

        
idx sGate::lock ( const void * key, idx keysize, idx jobId) 
{
    if(jobId==0)jobId=getpid();
    sStr nm;keyname( &nm,  key, keysize) ;
    
    for(idx it=0; it<gateWait+1; ++it) {
        sFil fl(nm);
        
        if(fl.length()) continue;

        fl.add((const char *)&jobId,sizeof(jobId));
        fl.destroy();
        fl.init(nm,sMex::fReadonly);
        if(  *(idx * )(fl.ptr())==jobId )
            return jobId;
    }
    return 0;
}

idx sGate::unlock ( const void * key, idx keysize, idx jobId) 
{
    if(jobId==0)jobId=getpid();
    sStr nm;keyname( &nm,  key, keysize) ;
    
    {
        sFil fl(nm,sFil::fReadonly);if(!fl.length()) return 0;
        if(jobId && *(idx  * )(fl.ptr())!=jobId )
            return 0;
        jobId=*(idx * )(fl.ptr());
    }
    sFile::remove(nm);
    return jobId;
}


