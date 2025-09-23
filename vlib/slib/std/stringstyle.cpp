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

#include <ctype.h>
#include <slib/std/string.hpp>

using namespace slib;



idx sString::hungarianText(const char * TextStr,char * TextDest,idx sizeDest,sString::eCase CaseType,bool IsName,bool IsRemIntBlanks)
{
    char chR,chW;
    idx iScan,iWrite;
    bool SkipBlank=true;
    idx UpCasechar=1;

    if(!sizeDest)sizeDest=sIdxMax;

    for(iScan=0,iWrite=0;iWrite<sizeDest && TextStr[iScan]!=0;iScan++){

        chR=TextStr[iScan];
        chW=0;

        if(strchr((sString_symbolsBlank),chR)){
            if(!SkipBlank){
                chW=(sString_symbolsBlank)[0];
                SkipBlank=true;
            }
        }
        else    {
            chW=chR;
            if(!IsRemIntBlanks)SkipBlank=false;
        }

        if(CaseType==eCaseHi)chW=(char)toupper((int)chW);
        if(CaseType==eCaseLo)chW=(char)tolower((int)chW);
        if(IsName && UpCasechar)chW=(char)toupper((int)chW);

        if(chW!=0)TextDest[iWrite++]=chW;

        if( isalpha((unsigned char)chW) )UpCasechar=0;

        else UpCasechar=1;
    }

    if(iWrite>0)if(strchr((sString_symbolsBlank),TextDest[iWrite-1]))iWrite--;
    TextDest[iWrite]=0;

    return iWrite;
}




idx sString::cStyle(sStr * dst, const char * src, idx len)
{
    #define addS(_v_sym)          dst->add(&(ch=_v_sym),1)
    idx levelCnt=4,i,j;
    char ch;
    bool isSep=false;
    idx level=0;
    
    #define lvlS()              for(j=0;j<level*levelCnt;j++){addS(' ');}

    for(i=0;i<len && src[i] ;i++){
        if(src[i]=='{'){
            addS('{');
            addS('\n');
            level++;lvlS();
            isSep=true;
            continue;
        }
        else if(src[i]=='}'){
            level--;
            addS('\n');lvlS();
            addS('}');
            addS('\n');
            isSep=true;
            continue;
        }
        else if(src[i]==',' || src[i]==';'){
            if(!isSep) {addS(' ');addS(src[i]);addS(' ');}
            isSep=true;
            continue;
        }
        else if(src[i]==' '){
            if(!isSep)addS(' ');
            isSep=true;
        }
        else if(src[i]=='\t'){
            if(!isSep){for(idx t=0; t<levelCnt; ++t) addS(' ');}
            isSep=true;
        }

        else {
            addS(src[i]);
            isSep=false;
        }
        
    }
    addS(0);addS(0);

    if(level<0)level=0;

    return 1;
}


idx sString::wrap(sStr * str,const char * src,const char * separ,idx charrayLen,sString::eCase caseChar, idx maxnum)
{
    idx i,k,iy=0,lastspace=0,lastpos=0,anySpace=0;
    char dst[1024],ch;

    lastspace=0;
    lastpos=0;

    if(!maxnum)maxnum=sIdxMax;
    
    for(k=0,i=0;i<maxnum && (ch=src[i])!=0;i++){

        if((separ && strchr(separ,ch)) || strchr(sString_symbolsEndline,ch) ){
            lastspace = k; lastpos = i;anySpace=1;
        }
        if(strchr(sString_symbolsEndline,ch) || k>=charrayLen){
            if(anySpace)dst[lastspace]=0;
            else dst[k]=0;

            str->printf("%s\n",dst);
            
            if(!anySpace){
                i--;lastpos=i;
            }else {
                i=lastpos;
            }
            
            lastspace=k=0;
            iy++;
            anySpace=0;
            continue;
        }
        if(caseChar==eCaseHi)ch=(char)toupper((int)src[i]);
        else if(caseChar==eCaseLo)ch=(char)tolower((int)src[i]);
        else ch=src[i];

        dst[k]=ch;k++;
    }
    if(k){
        dst[k]=0;
        str->printf("%s\n",dst);
    }
    return iy;
}


