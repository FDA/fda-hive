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

#include <slib/core/inf.hpp>

using namespace slib;


template <int Tbits> idx sInf<Tbits>::learn(sMex * mex, idx cnt, sMex::Pos * poslist)
{   
    // the order of the hasher runs from 1 (single word)
    // to cnt ... which means all notions are used 
    for(idx iO=1; iO<cnt; +iO)  
    //for( const char * s=src;
    return len+1;
}


template <int Tbits> idx sInf<Tbits> ::learn(const char * src00 , const char * separ)
{   
    sMex t;
    sMex::Pos br;
    const char * s=src;
    if(separ) // if separator is selected (like blanks) ... we skip the ones coming first
        while(strpbrk(*s,separ))++s;
    
    // loop over characters in the string 
    while( *s ){
        br.pos=(idx)(s-src00);
        if(separ) // if separator is given ... scan until next of those characters
            while(!strpbrk(*s,separ))++s;
        else  // otherwise just position to the next 00 string
            while(*s++);

        br.size=(idx)(s-src00)-pos;
        t.add(pos);

        if(separ) // if separator is selected ... we skip multiple occurences of those
            while(strpbrk(*s,separ))++s;
    }
    // now call the real learning procedure 
    return learn(&t,t.pos()/sizeof(sMex::Pos));
}




