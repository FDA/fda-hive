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

using namespace slib;


#include <ctype.h>

#define textSIZEOFINT           64
#define textSIZEOFREAL          64
#define textSIZEOFADDRESS       64
#define textSIZEOFCHAR          1
#define textSIZEOFPERCENT       1
#define textSIZEOFDEFAULT       1


const char sStr::zero[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

idx sStr::vprintfSizeOf(const char * formatDescription,va_list marker)
{   
    idx len=0,lenAdd,width,stopCountingWidth,isLong,isLongLong;
    const char *fmt;
    const char * typeOf;
    void *pVal;


    if(!formatDescription)return 0;

    for(fmt=formatDescription;*fmt;fmt++){
        if(*fmt=='%'){
            bool have_width = false;
            width=0; stopCountingWidth=0;
            for(typeOf=fmt+1; *typeOf && strchr("01234567890.+-*",*typeOf) ; typeOf++ ){ 
                if(*typeOf=='*' ){
                    have_width = true;
                    width+=va_arg(marker, int );
                }
                else if(*typeOf=='.') {
                    stopCountingWidth=1;
                    if(typeOf[1]=='*' ){
                        have_width = true;
                        width+=va_arg(marker, int );
                        ++typeOf;
                    }
                }
                if( isdigit((int)(*typeOf)) && (!stopCountingWidth)) {
                    have_width = true;
                    width=width*10+(*typeOf)-'0';
                }
            }
            width = sMax<idx>(0, width);

            isLong=0;isLongLong=0;
            if(*typeOf=='l' && *(typeOf+1)=='l'){isLongLong=1;typeOf+=2;}
            else if(*typeOf=='l'){isLong=1;typeOf++;}

            switch(tolower((int)*typeOf)){
            case 'i': case 'd': case 'x': case 'u':
                if(isLongLong)va_arg(marker, long long int);
                else if(isLong)va_arg(marker, long int);
                else va_arg(marker, int );
                lenAdd=textSIZEOFINT;
                break;
            case 'f': case 'g': case 'e':
                if(isLong)va_arg(marker, double );
                else va_arg(marker, double );
                lenAdd=textSIZEOFREAL;
                break;
            case 's':
                pVal=va_arg(marker, char * );
                if(!pVal) lenAdd=sLen("(NULL)")+16;
                else if( have_width ) lenAdd=width;
                else lenAdd=sLen(pVal);
                break;
            case 'p':
                pVal=va_arg(marker, void * );
                lenAdd=textSIZEOFADDRESS;
                break;
            case 'c':
va_arg(marker, int );
                lenAdd=textSIZEOFCHAR;
                break;
            case '%':
                lenAdd=textSIZEOFPERCENT;
                break;
            default: 
                typeOf--;
                lenAdd=textSIZEOFDEFAULT;
                break;
            }
            if(width)lenAdd=lenAdd > width ? lenAdd : width ;
            fmt=typeOf;
                len+=lenAdd;
        }
        else len++;
    }

    return len+1;
}



char * sStr::vprintf( const char * formatDescription, va_list marker )
{

    if((flags&fReadonly))return last();
    
    va_list marker0;va_copy(marker0,marker);
    idx len = vprintfSizeOf(formatDescription,marker0);
    va_end(marker0);

    if(!len)return last();
    
    char * cont;
    idx ps=pos();
    if(flags&fDirectFileStream) {
        cont=(char*)malloc(len+1);
    } else {
        resize(ps+len+1);
        cont=(char *)ptr(ps);
    }
    if(cont) {
        vsprintf(cont,formatDescription,marker) ;
    }
    idx realLen=sLen(cont);
    if(flags&fDirectFileStream) {
        if(cont){
            add(cont,realLen);
            free(cont);
        }
    } else {
        cut(ps+realLen);
    }

    if(realLen>len){
        ::printf("FATAL ERROR in slib: estimation of vprintfSizeOf=%" DEC "<%" DEC " has failed.\nFORMAT=\"%s\"\nRESULT=\"%s\"\n\n",len,realLen,formatDescription,cont);
#ifdef SLIB32
        __asm {
            int 3;
        };
#endif
        exit(0);
    }

    return cont;
}



void sStr::shrink00( const char * separ, idx dozeros)
{
    idx len=length(); if(!len) return ;
    for (char * p=ptr(0) ; len>0 ;  --len){
        if( !separ) {if( p[len-1]!=0) break;}
        else { if( p[len-1]!=0 && strchr(separ, p[len-1])==0 ) break;}
    }
    cut(len);
    if(dozeros)add0(dozeros);

}

idx sStr::recCnt(bool skipEmpty, char sep)
{
    idx qty = 0;
    sMex::Pos p;
    for(p.pos = sIdxMax; ; ++qty) {
        if( !recNext(p, skipEmpty, sep) ) {
            break;
        }
    }
    return qty;
}

bool sStr::recNext(sMex::Pos & rec, bool skipEmpty, char sep)
{
    const char * p;
    if( rec.pos == sIdxMax ) {
        p = ptr(0);
        if( skipEmpty ) {
            while( p < last() && (sep ? *p == sep : (*p == '\n' || *p == '\r')) ) {
                ++p;
            }
            if( p >= last() ) {
                return false;
            }
        }
    } else {
        p = ptr(rec.pos + rec.size);
        for(; p < last(); ++p ) {
            if( sep && *p == sep ) {
            } else if( *p == '\n' || *p == '\r' ) {
                if( *p == '\r' && (p < last()) && p[1] == '\n' ) {
                    ++p;
                }
            } else {
                break;
            }
            if( !skipEmpty ) {
                ++p;
                break;
            }
        }
        if( p >= last() ) {
            return false;
        }
    }
    rec.pos = p - ptr(0);
    while( p < last() && (sep ? *p != sep : (*p != '\n' && *p != '\r')) ) {
        ++p;
    }
    rec.size = p - ptr(rec.pos);
    return length() > 0;
}

bool sStr::recNth(const idx offset, sMex::Pos & rec, bool skipEmpty, char sep)
{
    idx qty = -1;
    while(offset >= 0) {
        if( !recNext(rec, skipEmpty, sep) || ++qty >= offset ) {
            break;
        }
    }
    return offset >= 0 && qty >= offset;
}
