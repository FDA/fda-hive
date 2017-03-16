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
    nm->add(dir); // ,sLen(dir)
    const char * hx="0123456789ABCDEF";
    idx i;
    char * ls=nm->last();
    if( (*ls)!='/' && *(ls)!='\\') nm->add("/",1); // add slash if necessary
    for(i=0; (i<keysize) ; ++i ) {
        char ch=((char*)key)[i];
        //nm[2*i]=hx[(ch&0x0F)];
        //nm[2*i+1]=hx[((ch>>4)&0x0F)];
        nm->add(&hx[(ch&0x0F)],1);
        nm->add(&hx[((ch>>4)&0x0F)],1);
    }
    //nm[2man *i]=0;
    nm->add(_,1);
}

        
idx sGate::lock ( const void * key, idx keysize, idx jobId) 
{
    if(jobId==0)jobId=getpid();
    sStr nm;keyname( &nm,  key, keysize) ;
    
    for(idx it=0; it<gateWait+1; ++it) {
        sFil fl(nm);
        
        if(fl.length()) continue; // the file is busy ... the gate is closed 

        fl.add((const char *)&jobId,sizeof(jobId)); // we add ourselves ... which will close the gate
        fl.destroy();
        fl.init(nm,sMex::fReadonly);
        // we still check one more time if between checking and writing someone else didn't write into it.
        if(  *(idx * )(fl.ptr())==jobId ) // if we were the only ones to write into that file we accept this 
            return jobId;
        // sleep( 1 );
    }
    return 0;//*(int64 * )(fl.ptr()); // return  the id of the job who has the gate 
}

idx sGate::unlock ( const void * key, idx keysize, idx jobId) 
{
    if(jobId==0)jobId=getpid();
    sStr nm;keyname( &nm,  key, keysize) ;
    
    { // we block this to unmap the file 
        sFil fl(nm,sFil::fReadonly);if(!fl.length()) return 0; // nothing to unlock ... the gate was open
        if(jobId && *(idx  * )(fl.ptr())!=jobId ) // if jobId is specified ... only the locker can unlock it.
            return 0;
        jobId=*(idx * )(fl.ptr()); // return  the locker
    }
    sFile::remove(nm);
    return jobId;
}


