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
#ifndef sLib_std_string_hpp
#define sLib_std_string_hpp


#include <time.h>

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <errno.h>

namespace slib { 
    class sString {
        public:
            #define sString_symbolsEndline  "\r\n" __
            #define sString_symbolsBlank    " \t\r\n" __
            #define sString_symbolsSpace    " \t" __
            #define sString_symbolsComment  ";!" __


        public:
            static bool empty(const char * ptr){return (ptr && *ptr) ? true : false; }
            static char * nonconst(const char * p) {return (((char *)0)+ (p-((const char*)0)));}
            static char * dup(const char * ptr, idx size=0){return (char *)sDup(ptr,size);}
            static char * glue(idx cnt,  const char * firstPtr, ... );
            static idx set00(char * src);
            static char * next00( const char * cont00,idx cnt=1);
            static idx cnt00( const char * cont00);
            static idx size00( const char * cont00);
            static char * glue00( sStr * dst, const char * cont00, const char * fmt, const char * separ);
            static char * inverse(char *text, idx n=0) ;
            static idx _strstr_QuickSearchPreprocBuf[256];
            static void strstr_quickSearchPreProcess(const char * find, idx m) ;
            static const char * strstr_quickSearch(const char *text, idx n, const char * find, idx m) ;

            static idx readChoiceList(idx * visCols, idx * sizCols, const char * cols00, const char * defline, const char * separ, bool isCaseInsensitive);
            static idx composeChoiceList(sStr * dfl,const char * cols00, idx * visCols, idx * sizCols, const char * separ);
            static idx scanRangeSet(const char * src,  idx len, sVec < idx > * range, idx shift, idx cidfirst, idx cidlast);
            static idx scanRangeSetSet(const char * src00, sVec < sVec < idx >  > * colset );
            static idx splitRange(real sVal, real fVal, idx cntTicks, sVec < real > * vec , bool filterBoundaries=false);
            static bool parseBool(const char *s)
            {
                return slib::parseBool(s);
            }
            static idx parseIBool(const char *s)
            {
                if (!s || !s[0] || !strcasecmp(s, "false") || !strcasecmp(s, "off") || !strcasecmp(s, "cancel"))
                    return 0;

                if (!strcasecmp(s, "true") || !strcasecmp(s, "on") || !strcasecmp(s, "ok"))
                    return 1;

                idx ival = 1;
                sscanf(s, "%" DEC, &ival);
                return ival;
            }
            static bool matchesBool(const char *s, bool *val=0)
            {
                if (!s)
                    return false;

                if (!strcmp(s, "1") || !strcasecmp(s, "on") || !strcasecmp(s, "true")) {
                    if (val)
                        *val = true;

                    return true;
                }

                if (!s[0] || !strcmp(s, "0") || !strcasecmp(s, "off") || !strcasecmp(s, "false")) {
                    if (val)
                        *val = false;

                    return true;
                }

                return false;
            }

            static int cmpNatural(const char * a, const char * b, bool caseSensitive);

            static idx parseTime(const char * s, idx * pLenParsed=0, idx * pExplicitUtcOffset=0);
            static idx parseTime(struct tm * result, const char * s, idx * pLenParsed=0, idx * pExplicitUtcOffset=0);
            static idx parseDateTime(struct tm * result, const char * s, idx * pLenParsed=0, idx * pExplicitUtcOffset=0);
            enum EPrintDateTimeFlags
            {
                fISO8601 = 1<<0,
                fNoDate = 1<<1,
                fNoTime = 1<<2,
                fNoTimezone = 1<<3
            };
            static const char * printDateTime(sStr & out, const struct tm * tm, idx flags=0);
            static const char * printDateTime(sStr & out, idx unix_time, idx flags=0);
            template <typename Tobj > static idx sscanfAnyVec(sVec < Tobj > * excl, const char * src, idx len, Tobj shift, Tobj rmin, Tobj rmax, const char * fmt, const char * separ=",;" sString_symbolsBlank ) 
            {
                if( !src ) return 0;
                if( len == 0 ) len = sLen(src);
                Tobj ex=0;
                char bb[128];
                idx i;
                for(const char * p=src; p && p<src+len && *p ; ) {
                    ex=0;
                    for( i=0; p<src+len && *p && strchr(separ,*p)==0 ; ++p, ++i)
                        bb[i]=(*p);
                    bb[i]=0;sscanf(bb,fmt,&ex);
                    for( ; p<src+len && *p && strchr(separ,*p)!=0 ; ++p)
                        ;

                    ex+=shift;
                    if(ex>rmax || ex<rmin) continue;
                    excl->vadd(1,ex);
                }return excl->dim();
            }
            static idx sscanfRVec(sVec < real > * excl, const char * src, idx len=0, real shift=0.0, real rmin=-REAL_MAX, real rmax=REAL_MAX) {return sscanfAnyVec<real>(excl, src,len, shift, rmin, rmax, "%lf",",;" sString_symbolsBlank);}
            static idx sscanfIVec(sVec < idx > * excl, const char * src, idx len=0, idx shift=0, idx imin=-sIdxMax, idx imax=sIdxMax)  {return sscanfAnyVec<idx>(excl, src,len, shift, imin, imax, "%" DEC,",;" sString_symbolsBlank);}
            static idx sscanfUVec(sVec < udx > * excl, const char * src, idx len=0, idx shift=0, udx umin=0, udx umax=sUdxMax)  {return sscanfAnyVec<udx>(excl, src,len, shift, umin, umax, "%" UDEC,",;" sString_symbolsBlank);}

            template <typename Tobj > static idx printfAnyVec(sStr * str, sVec < Tobj > * excl, const char * fmt, const char * separ=",") 
            {
                for(idx i=0 ; i< excl->dim() ; ++i ) { 
                    if(i) str->addString(separ);
                    str->printf(fmt,*(excl->ptr(i)));
                }return excl->dim();
            }
            static idx printfRVec(sStr * str, sVec < real > * excl, const char * separ="," ) {return printfAnyVec<real>(str, excl, "%lf", separ );}
            static idx printfIVec(sStr * str, sVec < idx > * excl, const char * separ=",")  {return printfAnyVec<idx>(str, excl, "%" DEC,separ);}
            static idx printfUVec(sStr * str, sVec < udx > * excl, const char * separ=",")  {return printfAnyVec<udx>(str, excl, "%" UDEC,separ);}

            enum eCase {
                eCaseNone=0,
                eCaseLo,
                eCaseHi
            } ;
        
            static idx compareChoice( const char *  src, const char * choice00,idx * numfnd,bool isCaseInSensitive, idx startNum, bool exactMatch=false, idx lenSrc=0);
            static idx compareNUntil( const char * str1, const char * str2, size_t n, const char * symblist,bool isCaseInSensitive);
            inline static idx compareUntil( const char * str1, const char * str2, const char * symblist,bool isCaseInSensitive)
            {
                return compareNUntil(str1, str2, (size_t)-1, symblist, isCaseInSensitive);
            }
            static char * searchSubstring( const char * src, idx lenSrc, const char * find00,idx occurence, const char * stopFind00,bool isCaseInSensitive);
            static char * searchStruc( const char * src, idx len, const char * begin00, const char * end00,idx * pst,idx * pfn);
            static char * searchBlock( const char * src, idx len, const char * begin00, const char * end00,idx * pst,idx * pfn);
            static char * skipWords( const char * src, idx slen, idx num=1,const char * separators=sString_symbolsBlank);

    
            static idx changeCase(sStr * dst, const char * TextStr,idx len, eCase CaseType);
            static idx copyUntil(sStr * dst, const char * src, idx len,const char * symblist);
            static char * extractSubstring(sStr * dst,const char * src,idx len, idx nextStp,const char * nextSepar00, bool isCaseInSensitive,bool isPreserveQuotes);
            static idx cleanEnds(sStr * dst, const char * src,idx len,const char * find,bool isMatch,idx maxNum=0);
            static char * cleanMarkup(sStr * dst, const char * src,idx len, const char * startTag00, const char * endTag00,const char * replacement00,idx maxTags,bool inside,bool isPreserveQuotes,bool isCaseInSensitive);
            static char * searchAndReplaceSymbols(sStr * dst , const char * src,idx len, const char * find, const char * replacement,idx maxTags,bool isMatch,bool isSkipMultiple,bool isPreserveQuotes, bool cleanTerminals=false, idx protectablequotes=0);
            static char * searchAndReplaceStrings(sStr * dst, const char * src,idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive);
            static char * searchAndReplaceStringsPaired(sStr * dst, const char * src,idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive, bool replacementPair=false);
            static char * searchandInvertStrings(sStr *dst, const char * src, idx len, const char * find, const char *replace);
            static idx replaceEscapeSequences(sStr * dst, const char * src, idx len=0);
            static bool extractNCBIInfoSeqID(sStr *ginum, sStr *accnum, const char *seqid, idx seqlen = 0);
            static bool stringEndsWith(const char *str, idx str_len, const char *suffix, idx suffix_len);

            static const char * escapeForCSV(sStr & dst, const char * src, idx len=0);
            static const char * unescapeFromCSV(sStr & dst, const char * src, idx len=0);

            static const char * escapeForShell(sStr & dst, const char * src, idx len=0);

            static const char * escapeForC(sStr & dst, const char * src, idx len=0);

            static const char * escapeForJSON(sStr & dst, const char * src, idx len=0);
            static const char * unescapeFromJSON(sStr & dst, const char * src, idx len=0);

            static const char * sepParseStr(const char *src, const char *srcend, char *buf, idx bn, unsigned char *row[], idx rn, idx sep, idx flags, idx *colnum = 0, sStr *err = 0);

            static idx changeCase(char * dst, idx len, eCase CaseType){return changeCase((sStr*)dst, dst,len, CaseType);}
            static idx copyUntil(char * dst, idx len,const char * symblist){return copyUntil((sStr*)dst, dst, len,symblist);}
            static char * extractSubstring(char * dst,idx len, idx nextStp,const char * nextSepar00, bool isCaseInSensitive,bool isPreserveQuotes){return extractSubstring((sStr*)dst,dst,len, nextStp,nextSepar00, isCaseInSensitive,isPreserveQuotes);}
            static idx cleanEnds(char * dst, idx len,const char * find,bool isMatch,idx maxNum=0){return cleanEnds((sStr*)dst, dst,len,find,isMatch,maxNum);}
            static char * cleanMarkup(char * dst, idx len, const char * startTag00, const char * endTag00,const char * replacement00,idx maxTags,bool inside,bool isPreserveQuotes,bool isCaseInSensitive){return cleanMarkup((sStr *)dst, dst,len, startTag00, endTag00,replacement00,maxTags,inside,isPreserveQuotes,isCaseInSensitive);}
            static char * searchAndReplaceSymbols(char * dst, idx len, const char * find, const char * replacement,idx maxTags,bool isMatch,bool isSkipMultiple,bool isPreserveQuotes, bool cleanTerminals=false, idx protectablequotes=0){return searchAndReplaceSymbols((sStr *)dst, dst,len, find, replacement,maxTags,isMatch,isSkipMultiple,isPreserveQuotes,cleanTerminals, protectablequotes);}
            static char * searchAndReplaceStrings(char * dst, idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive){return searchAndReplaceStrings((sStr*)dst, dst,len, find00, replacement00,maxTags,isCaseInSensitive);}
            static idx hungarianText(const char * TextStr,char * TextDest,idx sizeDest,eCase CaseType,bool IsName,bool IsRemIntBlanks);
            static idx  cStyle(sStr * dst,const char * src, idx len);
            static idx wrap(sStr * str,const char * src,const char * separ,idx charrayLen,sString::eCase caseChar, idx maxnum);

            static idx vbufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, va_list marker);

            static idx bufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, ...) __attribute__((format(scanf, 3, 4)));
            static idx xvbufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, va_list marker);
            static idx xbufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, ...);
            inline static idx xvscanf(const char * textScan, const char * formatDescription, va_list marker)
            {
                if (!textScan) return 0;
                return xvbufscanf(textScan, NULL, formatDescription, marker);
            }
            inline static idx xscanf(const char * textScan, const char * formatDescription, ...)
            {
                idx ret;
                sCallVargResPara(ret,sString::xvscanf,textScan,formatDescription);
                return ret;
            }
            static char *  xvprint(sStr * str, bool isptr, const char * formatDescription,va_list marker);
            static char *  xvprintf(sStr * str,const char * formatDescription,va_list marker){return xvprint(str,false,formatDescription,marker);}
            static char *  xvprintp(sStr * str,const char * formatDescription,va_list marker){return xvprint(str,true,formatDescription,marker);}
            static char *  xprintf(sStr * str,const char * formatDescription,...)
            {
                char * ret;
                sCallVargResPara(ret,sString::xvprintf,str,formatDescription);
                return ret;
            }
            static char *  xprintp(sStr * str,const char * formatDescription,...)
            {
                char * ret;
                sCallVargResPara(ret,sString::xvprintp,str,formatDescription);
                return ret;
            }
            static idx printHexArr( sStr * out, const void * Buf, idx siz , idx brk);

            struct SectVar {
                const char * visloc;
                const char * loc;
                const char * fmtin, *fmtout;
                void * val;
                idx granularity;
                const char * comment;
                SectVar * set(const char * lvisloc=0, const char * lloc=0 , const char * lfmtin=0, const char * lfmtout=0, void * lval=0,idx lgranularity=0, const char * lcomment=0){
                    visloc=lvisloc;
                    loc=lloc;
                    fmtin=lfmtin;
                    fmtout=lfmtout;
                    val=lval;
                    granularity=lgranularity;
                    comment=lcomment;
                    return this;
                }
            };
            static void xscanSect(const char * src, idx len,SectVar * varIO, idx cnt=0);
            static idx xprintSect(sStr * out, const char * doTabs, SectVar * varIO, idx cnt=0);

            static char encode64Char( unsigned char binary );
            static idx encodeBase64( sMex * dst, const char * src, idx len, const bool RFC2045_compliant=true);
            static idx decodeBase64( sMex * dst, const char * src, idx len );

            static idx fuzzyStringCompareDynamat(const char * string1, idx str1Len, const char * string2, idx str2Len, sVec <idx> * matrix );
            static idx IPDigest(const char * ptr);

            static void trimLeft(char * str, const char * trimChars = 0, bool trimMismatched = false);
            static void trimRight(char * str, const char * trimChars = 0, bool trimMismatched = false);
            static void trim(char * str, const char * trimChars = 0, bool trimMismatched = false);
            static bool convertStrToUdx(const char *str, udx *i);
    };
    
    
}

    #ifdef WIN32
        idx vsscanf(const char *s, const char *fmt, va_list ap);
    #endif
    
#endif
