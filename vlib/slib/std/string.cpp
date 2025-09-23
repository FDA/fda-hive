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



idx sString::set00(char * src)
{
    idx len=sLen(src);

    src[len+1]=0;

    return len;
}

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
            dst->printf("%s", separ);
        dst->printf(fmt, p);
    }
    return dst->ptr(pos);
}




idx sString::_strstr_QuickSearchPreprocBuf[256];

void sString::strstr_quickSearchPreProcess(const char * find, idx m) 
{
    idx i;

    for (i = 0; i < sDim(sString::_strstr_QuickSearchPreprocBuf); ++i)
        sString::_strstr_QuickSearchPreprocBuf[i] = m + 1;
    for (i = 0; i < m; ++i)
        sString::_strstr_QuickSearchPreprocBuf[(int)find[i]] = m - i;
}

const char * sString::strstr_quickSearch(const char *text, idx n, const char * find, idx m) 
{
    idx j;

    j = 0;
    while (j <= n - m) {
        if (memcmp(find, text + j, m) == 0)
            return text+j;
        j += sString::_strstr_QuickSearchPreprocBuf[(int)text[j + m]];
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

void sString::trimLeft(char * str, const char * trimChars, bool trimMismatched)
{
    if(!str || *str == 0) { 
        return; 
    }
    const char *fltr = (trimChars && *trimChars) ? trimChars : sString_symbolsBlank;
    while (*str && (strchr(fltr, *str) || trimMismatched)) {
        ++str;
    }
    if (*str == 0) {
        *str = '\0';
    }
}

void sString::trimRight(char * str, const char * trimChars, bool trimMismatched)
{
    if(!str || *str == 0) { 
        return; 
    }
    const char *fltr = (trimChars && *trimChars) ? trimChars : sString_symbolsBlank;
    char *end = str + strlen(str) - 1;
    while (end > str && (strchr(fltr,*end) || trimMismatched)) {
        --end;
    }
    if (end == str && (strchr(fltr,*end) || trimMismatched)) {
        *str = '\0';
    } else {
        end[1] = '\0';
    }
}

void sString::trim(char * str, const char * trimChars, bool trimMismatched)
{
    trimLeft(str, trimChars, trimMismatched);
    trimRight(str, trimChars, trimMismatched);
}

bool sString::convertStrToUdx(const char *str, udx *i)
{
    char *end;
    errno = 0;
    udx x = strtoudx(str, &end, 10);

    if (end == str || *end != '\0' || errno == ERANGE)
    {
        errno = 0;
        return false;
    }

    *i = x;

    return true;
}
