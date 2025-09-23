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
#include <float.h>

#include <slib/std/string.hpp>

using namespace slib;

inline static char *_strnzcpy(char *dest, const char *src, size_t n)
{
    const char *s;
    char *d;
    for (d = dest, s = src; s < src+n && *s; s++, d++)
        *d = *s;
    *d = 0;
    return dest;
}

static const char *_strnpbrk(const char *s, const char *accept, size_t n)
{
    for (const char *t = s; t < s+n && *t; t++)
        for (const char *b = accept; *b; b++)
            if (*t == *b)
                return t;
    return NULL;
}


static idx scanAfter(sStr * wb,const char * scnFormat,const char * KeyFormat,char * pVal,const char * find,const char * scanto)
{
    wb->cut(0);
    const char *   pFormat;

    if(find)pFormat=strpbrk(KeyFormat,find);
    else pFormat=KeyFormat;
    if(pFormat){
        sString::copyUntil(wb,(pFormat+1),0,scanto);
    }else return 0;

    if(scnFormat)if(!sscanf(wb->ptr(),scnFormat,pVal))return 0;

    return 1;
}

idx sString::xvbufscanf(const char * textScan, const char *textScanEnd, const char * formatDescription, va_list marker)
{
    idx iChoice,is,isScanned,icnt;
    const char * pItem, * curT;
    const char *  toEndSym="=><; \t\r\n";
    const char * pFormat;
    const char * fmt;
    const char * typeOf;
    idx isLong,isLongLong;
    sStr wb;

    if (!textScan)
        return 0;

    if (!textScanEnd)
        textScanEnd = textScan + strlen(textScan);


    for(icnt=0, fmt=formatDescription; *fmt && textScan<=textScanEnd; ++fmt){
        if(*fmt!='%')continue;

        sStr bFormat;
        sString::copyUntil(&bFormat,fmt,0,toEndSym);
        sString::changeCase(bFormat.ptr(),0,sString::eCaseLo);
        pFormat=bFormat.ptr();

        isLong=0;isLongLong=0;
        for(typeOf=fmt+1; *typeOf  ; ++typeOf ){
            if(strchr("01234567890.+-",*typeOf))continue;
            else if(strncasecmp(typeOf,"i64",3)==0 ){isLongLong=1;continue;}
            else if(*typeOf=='l') {if(!isLong)isLong=1; else isLongLong=1;}
            else break;
        }

        switch(*typeOf){
            case 'i': case 'd': case 'u': case 'x': case 'c':
            case 'I': case 'D': case 'X': case 'C': {
                idx64 val=0, vmin,vmax;

                if( isLongLong) {vmin=-0x7FFFFFF; vmax=0x7FFFFFF; }
                else if(isLong) {vmin=-0x7FFFFFF; vmax=0x7FFFFFF; }
                else { val=(idx)val;vmin=-0x7FFFFFF; vmax=0x7FFFFFF; }
                isScanned=bufscanf(textScan,textScanEnd,pFormat,&val);
                if(isScanned!=1) scanAfter(&wb,pFormat,fmt,(char *)&val,"=",toEndSym);
                scanAfter(&wb,pFormat,fmt,(char *)&vmin,">",";" sString_symbolsBlank);
                scanAfter(&wb,pFormat,fmt,(char *)&vmax,"<",";" sString_symbolsBlank);

                if( isLongLong ) {
                } else if( isLong ) {
                    val = (idx64) val;
                    vmin = (idx64) vmin;
                    vmax = (idx64) vmax;
                } else {
                    val = (idx) val;
                    vmin = (idx) vmin;
                    vmax = (idx) vmax;
                }

                if(islower(*typeOf)){
                    if(val>vmax)val=vmax;
                    if(val<vmin)val=vmin;
                }

                if( isLongLong) * (va_arg(marker, idx64 * ))=val;
                else if(isLong) *(va_arg(marker, idx64 * ))=(idx64)val;
                else *(va_arg(marker, int * ))=(int)val;
                } ++icnt;break;
            case 'f': case 'g': case 'e':
            case 'F': case 'G': case 'E':{
                double val=0, vmin=-DBL_MAX, vmax=DBL_MAX;
                isScanned=bufscanf(textScan,textScanEnd,pFormat,&val);
                if(isScanned!=1) scanAfter(&wb,pFormat,fmt,(char *)&val,"=",toEndSym);
                scanAfter(&wb,pFormat,fmt,(char *)&vmin,">",";" sString_symbolsBlank);
                scanAfter(&wb,pFormat,fmt,(char *)&vmax,"<",";" sString_symbolsBlank);
                if(islower(*typeOf)){
                    if( val<vmin )val=vmin;
                    if( val>vmax )val=vmax;
                }
                *va_arg(marker, double * )=val;
                } ++icnt;break;

            case 's':
                if(textScan && (*textScan)) _strnzcpy(va_arg(marker, char * ), textScan, textScanEnd-textScan);
                else { scanAfter(&wb,0,fmt,0,"=",toEndSym);
                    if(wb.length())strcpy( va_arg(marker, char * ), wb.ptr());
                }
                ++icnt;break;

            case 'S':{
                sStr * str=va_arg(marker, sStr * );
                if(textScan && (*textScan))
                    str->add(textScan, textScanEnd-textScan+1);
                else
                    scanAfter(str,0,fmt,0,"=",toEndSym);
                }++icnt;break;
            case 'n':{
                idx val=0;
                if(strpbrk(fmt,"^;")==0)pItem="%n=0^FALSE^TRUE^OFF^ON^NO^YES^CANCEL^OK^0^1;";
                else pItem=fmt;
                isScanned=0;
                is=0;
                for(iChoice=0;;iChoice++){
                    pItem=strpbrk(pItem,"^;");if((*pItem)!='^')break;
                    pItem++;

                    is=sString::compareNUntil(textScan,pItem,textScanEnd-textScan,"=^;",0);
                    if(strchr(";^=",pItem[is])){isScanned=1;break;}
                }
                if(!isScanned) {
                    if(bufscanf(textScan,textScanEnd,"%" DEC,(idx *)&val)<1)
                        scanAfter(&wb,"%" DEC,fmt,(char *)&val,"=",";^");
                }
                else if(pItem[is]=='=') scanAfter(&wb,"%" DEC,pItem+is,(char *)&val,0,";^");
                else val=iChoice;
                *va_arg(marker, idx * )=val;
                } ++icnt;break;
            case 'b': {
                idx val=0;
                for(curT=textScan;curT && curT<textScanEnd && *curT;){
                    pItem=fmt;
                    isScanned=0;
                    is=0;
                    for(iChoice=0;;iChoice++){
                        pItem=strpbrk(pItem,"|;");if(!pItem || (*pItem)!='|')break;
                        pItem++;

                        is=sString::compareNUntil(curT,pItem,textScanEnd-curT,"=|;",0);
                        if(strchr(";|=",pItem[is])){isScanned=1;break;}
                    }
                    if(isScanned){
                        idx valScan=0;
                        if(pItem[is]=='='){scanAfter(&wb,"%" HEX,pItem+is,(char *)&valScan,0,"|;");val|=valScan;}
                        else val|=(((idx)1)<<iChoice);
                    }
                    curT=_strnpbrk(curT+is,"|;",textScanEnd-(curT+is));
                    if(curT){curT++;
                        while(curT<textScanEnd && *curT!=0 && (  (*curT==' ') || (*curT=='\t') || (*curT=='\r') || (*curT=='\r') )  )curT++;
                    }
                }

                if(!val) scanAfter(&wb,"%" HEX,fmt,(char *)&val,"=","|;");
                *va_arg(marker, idx * )=val;
                }++icnt;break;
            default:
                break;
        }
        fmt=strpbrk(fmt,";" sString_symbolsBlank);
        textScan=_strnpbrk(textScan,sString_symbolsBlank,textScanEnd-textScan);
        if(!textScan)break;
        ++textScan;
        if(!fmt)break;
        if(*fmt!=';')--fmt;
    }
  return icnt;
}

idx sString::xbufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, ...)
{
    va_list marker;
    idx ret;
    va_start(marker, formatDescription);
    ret = xvbufscanf(textScan, textScanEnd, formatDescription, marker);
    va_end(marker);
    return ret;
}



char *  sString::xvprint(sStr * str,bool isptr,const char * formatDescription,va_list marker)
{
    idx iChoice;
    idx iVal,iTmp,isFnd;
    const char *   toEndSym="%=><; \t\r\n";
    const char * pFormat;
    const char * pItem;
    const char * fmt, *it;
    const char * typeOf;
    idx isLong,isLongLong;
    sStr wb, bFormat;

    for(fmt = formatDescription; *fmt; fmt++) {
        if(*fmt!='%'){
            str->add(fmt, 1);
            continue;
        }
        if( fmt[1] == '%' ) {
            str->add(++fmt, 1);
            continue;
        }
        bFormat.cutAddString(0, fmt, 1);
        sString::copyUntil(&bFormat, fmt + 1, 0, toEndSym);
        sString::changeCase(bFormat.ptr(),0,sString::eCaseLo);
        bFormat.shrink00();
        pFormat=bFormat.ptr();

        isLong=0;isLongLong=0;
        for(typeOf=fmt+1; *typeOf  ; ++typeOf ){
            if(strchr("01234567890.+-",*typeOf))continue;
            else if(strncasecmp(typeOf,"i64",3)==0 ){isLongLong=1;continue;}
            else if(*typeOf=='l') {if(!isLong)isLong=1; else isLongLong=1;}
            else break;
        }

        idx crct= (*typeOf=='S') ? (*typeOf) : tolower(*typeOf);
        switch(crct){
            case 'i': case 'd': case 'u': case 'x':
                if(isptr) str->printf(pFormat, isLongLong ? *va_arg(marker, long long int * ) : ((isLong) ? *va_arg(marker, long int * ) : *va_arg(marker, int * ) ));
                else str->printf(pFormat, isLongLong ? va_arg(marker, idx64) : ((isLong) ? va_arg(marker, long int) : va_arg(marker, int) ));
                fmt += bFormat.length() - 1;
                break;
            case 'f': case 'g': case 'e':
                if(isptr)str->printf(pFormat,*va_arg(marker, double * ) );
                else str->printf(pFormat,va_arg(marker, double) );
                fmt += bFormat.length() - 1;
                break;
            case 's':
                str->printf(pFormat,va_arg(marker, char * ) );
                fmt += bFormat.length() - 1;
                break;
            case 'S':{
                const sStr * vr = va_arg(marker, const sStr * );
                if( vr->length() ) {
                    str->printf(pFormat, vr->ptr() );
                    fmt += bFormat.length() - 1;
                } else {
                    fmt = typeOf;
                }
                }break;
            case 'c':
                if(isptr)str->printf(pFormat,*va_arg(marker, char * ) );
                else str->printf(pFormat,va_arg(marker, int ) );
                fmt += bFormat.length() - 1;
                break;
            case 'p':
                if(isptr)str->printf(pFormat,*va_arg(marker, void * * ) );
                else str->printf(pFormat,va_arg(marker, void * ) );
                fmt += bFormat.length() - 1;
                break;
            case 'n':
                if(isptr)iVal=*va_arg(marker, idx * );
                else iVal=va_arg(marker, idx);
                if(strpbrk(fmt,"^;")==0)pItem="%n=0^FALSE^TRUE^OFF^ON^NO^YES^CANCEL^OK^0^1;";
                else pItem=fmt;
                isFnd=0;
                for(iChoice=0;;iChoice++){
                    pItem=strpbrk(pItem,"^;");if(!pItem || (*pItem)!='^')break;
                    pItem++;

                    pFormat = strpbrk(pItem, "=^;");
                    if( pFormat && pFormat[0] == '=' && scanAfter(&wb, "%" HEX, pFormat, (char *) &iTmp, 0, "^;") ) {
                    } else {
                        iTmp = (iChoice);
                    }
                    if( iTmp == iVal ) {
                        it = strpbrk(pItem, "=^;");
                        if( !it )
                            it = pItem + sLen(pItem);
                        str->add(pItem, (idx) (it - pItem));
                        isFnd = 1;
                        break;
                    }
                }
                if(!isFnd)str->printf("%" DEC,iVal);
                fmt = strpbrk(fmt, ";" sString_symbolsBlank);
                if( fmt && *fmt != ';' ) --fmt;
                break;

            case 'b':
                if(isptr)iVal= *va_arg(marker, idx * );
                else iVal=va_arg(marker, idx);
                pItem=fmt;
                isFnd=0;
                for(iChoice = 0; iChoice < (idx) (sizeof(idx) * 8); iChoice++) {
                    pItem=strpbrk(pItem,"|;");if(!pItem || (*pItem)!='|')break;
                    pItem++;

                    pFormat = strpbrk(pItem, "=|;");
                    if( pFormat && pFormat[0] == '=' && scanAfter(&wb, "%" HEX, pFormat, (char *) &iTmp, 0, "|;") ) {
                    } else {
                        iTmp = (((idx) 1) << iChoice);
                    }
                    if( iVal & iTmp ) {
                        if( isFnd )
                            str->add("|", 1);
                        it = strpbrk(pItem, "=|;");
                        if( !it )
                            it = pItem + sLen(pItem);
                        str->add(pItem, (idx) (it - pItem));
                        isFnd = 1;
                    }
                }
                fmt = strpbrk(fmt, ";" sString_symbolsBlank);
                if( fmt && *fmt != ';' ) --fmt;
                break;

            default:
                str->add(typeOf, 1);
                fmt += bFormat.length() - 1;
                break;
        }
        if( !fmt ) {
            break;
        }
    }
    str->add0cut();
    return str->ptr();
}







#ifdef WIN32
idx vsscanf(const char *s, const char *fmt, va_list ap)
{
  void * a[16];

  for (idx i=0; i<sDim(a); i++)
      a[i] = va_arg(ap, void *);
  return sscanf(s, fmt, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15]);
}
#endif







void sString::xscanSect(const char * src, idx len,SectVar * varIO, idx cnt)
{
    sStr buf;
    if(!src)src="";
    if(!len)len=sLen(src);
    if(!cnt)cnt=sIdxMax;
    const char * term;

    for(idx iv=0; iv<cnt && varIO[iv].loc ; ++iv) {

        if(varIO[iv].loc[sLen(varIO[iv].loc)+1]==0)term="[" __;
        else term="[" _ "\n" __;

        idx st=0,en=0;
        searchStruc(src, len, varIO[iv].loc, term,&st,&en);
        if(st!=en)cleanEnds(&buf,src+st,en-st,"=" sString_symbolsBlank,true);
        if(!buf.length())buf.add(__,2);

        xscanf(buf.ptr(),varIO[iv].fmtin,varIO[iv].val);
        buf.cut(0);
    }
}

idx sString::xprintSect(sStr * out, const char * doTabs, SectVar* varIO, idx cnt)
{
    idx rowVBS=0;
    sStr lastSect;
    if(!doTabs)doTabs="";
    if(!cnt)cnt=sIdxMax;

    for(idx iv=0; iv<cnt && varIO[iv].loc ; ++iv) {

        if(lastSect.length()==0 || strcmp(lastSect.ptr(),varIO[iv].loc)!=0 ){
            out->printf("%s%s\n",doTabs,varIO[iv].loc);
            rowVBS+=1;
            lastSect.printf(0,"%s",varIO[iv].loc);
        }

        if( varIO[iv].loc[sLen(varIO[iv].loc) + 1] == 0 ) {
        } else {
            out->printf("%s  %s = ", doTabs, next00(varIO[iv].loc));
        }
        xprintp(out,varIO[iv].fmtout ? varIO[iv].fmtout : varIO[iv].fmtin ,varIO[iv].val);
        if(varIO[iv].comment)out->printf(" %s",varIO[iv].comment);
        out->printf("\n");
        rowVBS+=1;
    }

    out->printf("%s[End]\n",doTabs); rowVBS+=1;
    return rowVBS;
}





idx sString::printHexArr( sStr * out, const void * Buf, idx siz , idx brk)
{
    idx i;
    const char * buf=(const char *)Buf;

    if(!siz)siz=sLen((const char *)buf);

    out->printf("0x");
    for (i=0; i<siz; ++i) {
        out->printf("%02X",buf[i]);
        if(brk) {
            if(((i+1)%(brk&0xFF))==0 )
                out->printf(" ");
            if( (brk>>8) && ((i+1)%(brk>>8))==0 )
                out->printf("\n");
        }
    }
    out->printf("\n");

    return i;
}



