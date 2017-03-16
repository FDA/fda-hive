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
#include <slib/std/string.hpp>

using namespace slib;


// glues a list of strings given as arguments returns the destination address 
char * sString::glue(idx cnt,  const char * firstPtr, ... )
{
    idx len=0,i;
    const char * ptr = firstPtr;
    char *bfr,*bfrPtr;
    va_list marker;

    if(!cnt)cnt=0x7fffffff;

    va_start(marker, firstPtr );
    for(i=0;i<cnt && ptr;i++){
        if(ptr){len+=sLen(ptr);}
        ptr = va_arg(marker, char *);
    }
    va_end( marker );
    if(!len || (bfr=(char *)sNew(len+2))==0)return 0;

    bfrPtr=bfr;
    ptr=firstPtr;

    va_start(marker, firstPtr );
    for(i=0;i<cnt && ptr;i++){
        if(ptr){strcpy(bfrPtr,ptr);bfrPtr+=sLen(bfrPtr);}
        ptr = va_arg(marker, char *);
    }
    va_end( marker );

    return bfr;
}



// double the zero termination : asumes the string has enough buffer
idx sString::set00(char * src)
{
    idx len=sLen(src);

    src[len+1]=0;

    return len;
}

// returns string number cnt in the double-zero terminated string list 
// where strings are separated by \0 symbol.
char * sString::next00( const char * cont00,idx cnt)
{
    if(!cont00)return 0;
    if(cnt == -1) {
        idx cntChoice = sString::cnt00(cont00);
        if( cntChoice > 1 ) {
            cnt = (rand() * 1.) / (RAND_MAX + 1.) * cntChoice;
        } else {
            cnt = 0;
        }
    }
    while(cnt){
        while(*cont00){cont00++;}
        cont00++;
        if(!(*cont00))break;
        cnt--;
    }
    if(!(*cont00) || cnt)return 0;
    return nonconst(cont00);
}

// counts strings in double zero terminated string list 
idx sString::cnt00( const char * cont00)
{
    idx cnt;

    if(!cont00)return 0;
    for(cnt=0;(*cont00);cnt++){
        while(*cont00){cont00++;}
        cont00++;
    }
    return cnt;
}

idx sString::size00( const char * cont00)
{
    const char * zpos=cont00;
    if(!cont00)return 0;
    while((*cont00)){
        while(*cont00){cont00++;}
        cont00++;
    }
    return (idx )(cont00-zpos)+1 ;
}


char * sString::glue00( sStr * dst, const char * cont00, const char * fmt, const char * separ)
{
    idx pos=dst->length();
    for( const char * p=cont00; p; p=sString::next00(p) ) {
        if( p!=cont00 ) 
            dst->printf(separ);
        dst->printf(fmt, p);
    }
    return dst->ptr(pos);
}




idx sString::_strstr_QuickSearchPreprocBuf[256];

void sString::strstr_quickSearchPreProcess(const char * find, idx m) 
{
    idx i;

    /* Preprocessing */
    //for (i = 0; i < sizeof(sString::_strstr_QuickSearchPreprocBuf)/sizeof(sString::_strstr_QuickSearchPreprocBuf[0]); ++i)
    for (i = 0; i < sDim(sString::_strstr_QuickSearchPreprocBuf); ++i)
        sString::_strstr_QuickSearchPreprocBuf[i] = m + 1;
    for (i = 0; i < m; ++i)
        sString::_strstr_QuickSearchPreprocBuf[(int)find[i]] = m - i;
}

const char * sString::strstr_quickSearch(const char *text, idx n, const char * find, idx m) 
{
    idx j;

    // Searching
    j = 0;
    while (j <= n - m) {
        if (memcmp(find, text + j, m) == 0)
            return text+j;
        j += sString::_strstr_QuickSearchPreprocBuf[(int)text[j + m]]; //shift 
    }
    return 0;
}


char * sString::inverse(char *text, idx n ) 
{
    if(!n)n=sLen(text);
    for ( idx  i=0; i<n/2; ++i) { 
        char t=text[i];
        text[i]=text[n-1-i];
        text[n-1-i]=t;
    }
    return text;
}


