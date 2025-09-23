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
#include <stdint.h>
#include <slib/std/string.hpp>

using namespace slib;




idx sString::compareChoice( const char *  src, const char * choice00,idx * numfnd,bool isCaseInSensitive, idx startNum, bool exactMatch, idx lenSrc)
{
    idx sc,i,cnt;

    if(!lenSrc)lenSrc=sLen(src);
    if(!choice00[0]){if(numfnd)*numfnd=startNum;return 0;}

    for(cnt=0,sc=0;choice00[sc];cnt++) {

        if(!isCaseInSensitive){
            for(i=0;choice00[sc] && i<lenSrc && src[i]==choice00[sc] ;i++,sc++)
                ;
        }
        else
            for(i=0;choice00[sc] && i<lenSrc && toupper((idx)src[i])==toupper((idx)choice00[sc]);i++,sc++)
                ;

      if(!choice00[sc] && (exactMatch==false || !src[i]) ){
          if(numfnd)*numfnd=cnt+startNum;
          return i;
      }
      while(choice00[sc])sc++;
      sc++;
    }
    return (idx)(-1);
}

idx sString::compareNUntil( const char * str1, const char * str2, size_t n, const char * symblist,bool isCaseInSensitive)
{
    idx  i=0,j;

    for(j=0; (udx)j<n && (*str1) && (*str2); j++){

        if(symblist){
            for(i=0;symblist[i];i++){if(symblist[i]==(*str2))break;}
            if(symblist[i])break;
        }

        if(!isCaseInSensitive && ((*str1)-(*str2)))break;
        else if(((toupper((idx)(*str1)))-(toupper((idx)(*str2)))))break;

        str1++;str2++;
    }
    if(symblist)return (symblist[i] || (udx)j>=n || !(*str2) || !(*str1)) ? j : 0 ;
    return j ;

}



char * sString::searchSubstring( const char * src, idx lenSrc, const char * find00,idx occurence, const char * stopFind00,bool isCaseInSensitive)
{
    idx i,iFound=sNotIdx,ifnd=0,pos;
    if(!src || !find00)return 0;
    if(!lenSrc ) lenSrc=sLen(src);



    for(i=0;i<lenSrc && src[i];i++){

        pos=compareChoice(src+i,find00,0,isCaseInSensitive,0,false,lenSrc-i);
        if(pos!=sNotIdx){
            if(ifnd+1==occurence || occurence==sNotIdx)iFound=i;
            else ifnd++;
            if(occurence!=sNotIdx && iFound!=sNotIdx)break;
        }

        if(stopFind00 && stopFind00[0] && compareChoice(src+i,stopFind00,0,isCaseInSensitive,0,false,lenSrc-i)!=sNotIdx)
            break;
    }
    if(iFound==sNotIdx)return 0;
    return nonconst(src+iFound);
}




char * sString::searchStruc( const char * src, idx len, const char * begin00, const char * end00,idx * pst,idx * pfn)
{
    const char * st, *fn,*bg,*ed,*fnp;

    if(!len)len=sLen(src);

    st=src;fn=fnp=st+len;
    bg=begin00;ed=end00;
     while(*bg || *ed){
        if(*bg){
            st=strstr(st,bg);if(!st)return 0;
            if(st>=fnp)return 0;
            st+=sLen(bg);
        }
        if(*ed){
            fn=strstr(st,ed);if(!fn)return 0;
        }
        bg+=sLen(bg)+1;
        ed+=sLen(ed)+1;
        fnp=fn;
    }
    if(pst)*pst=(idx)(st-src);
    if(pfn)*pfn=(idx)(fn-src);

    return nonconst(st);
}

char * sString::searchBlock( const char * src, idx len, const char * begin00, const char * end00,idx * pst,idx * pfn)
{
    const char *bg,*ed;
    len=0;

    bg=sString::searchSubstring(src,0,begin00,1,0, 1) ;
    if(!bg) return 0;
    idx num=0;
    sString::compareChoice(bg,begin00,&num,1,0);
    bg+=sLen(sString::next00(begin00,num));

    ed=sString::searchSubstring(bg+1,0,end00,1,0, 1) ;

    if(pst)*pst=(idx)(bg-src);
    if(pfn)*pfn=(idx)(ed-src);

    return nonconst(ed);
}

char * sString::skipWords( const char * src, idx slen, idx num,const char * separators)
{
    idx i;
    const char * end=slen ? src+slen : 0 ;

    if(!separators)separators=(sString_symbolsBlank);
    while(*src && strchr(separators,*src))src++;
    for(i=0;(end==0 || src<end ) && *src && i<num;i++){
        while((end==0 || src<end ) && *src && !strchr(separators,*src))src++;
        while((end==0 || src<end ) && *src && strchr(separators,*src))src++;
    }
    if( (end!=0 && src>=end )) return 0;
    if(!(*src))return 0;
    return nonconst(src);
}





#define isIN  ((void *)dst==(void *)src)
#define isNT  ((void *)dst!=(void *)src)
#define SYM(_v_i,_v_ch)  if((void *)dst==(void *)src)((char*)dst)[(_v_i)]=*(_v_ch); else dst->add((_v_ch),1)
#define ZER(_v_i)  if(isIN)((char*)dst)[(_v_i)]=0; else dst->add(__,2)
#define DST ((char * )dst)

idx sString::changeCase(sStr * dst, const char * src,idx len, eCase CaseType)
{

    idx i;
    char ch;
    if(!len)len=sLen(src);

    for(i=0;i<len && src[i];i++){
        if(CaseType==(eCaseLo))ch=(char)tolower((idx)src[i]);
        else if(CaseType==(eCaseHi))ch=(char)toupper((idx)src[i]);
        else ch=src[i];

        SYM(i,&ch);
    }

    ZER(i);
    return i;
}



idx sString::copyUntil(sStr * dst, const char * src, idx len,const char * symblist)
{
    idx i,j;

    if(!len)len=sIdxMax;

    for(j=0;j<len && src[j];j++){

        if(symblist){
            for(i=0;symblist[i];i++){if(symblist[i]==(src[j]))break;}
            if(symblist[i])break;
        }
        SYM(j,&src[j]);
    }

    ZER(j);

    return j;
}



char * sString::extractSubstring(sStr * dst,const char * src,idx len, idx nextStp,const char * nextSepar00, bool isCaseInSensitive,bool isPreserveQuotes)
{
    idx iNxt=0,i,k,pos;
    char  inquote=0;

    if(!src || !src[0])return 0;
    if(!len)len=sIdxMax;

    for(i=0,k=0;i<len && iNxt<=nextStp && src[i];i++){

        if(isPreserveQuotes){
            if(!inquote && (src[i]=='\"' || src[i]=='\'') )inquote=src[i];
            else if(src[i]==inquote)inquote=0;
        }
        if(!inquote)pos=compareChoice(src+i,nextSepar00,0,isCaseInSensitive,0);
        else pos=sNotIdx;

        if(pos!=sNotIdx){i+=pos-1;iNxt++;continue;}
        if(iNxt==nextStp){SYM(k,&src[i]); ++k; }
        if(iNxt>nextStp)break;
    }


    ZER(k);

    return nonconst(src+i);
}


idx sString::cleanEnds(sStr * dst, const char * src, idx len, const char * find, bool isMatch, idx maxNum)
{
    if( !src ) {
        return 0;
    }
    if( !len ) {
        len = sIdxMax;
    }
    if(!maxNum)
        maxNum=sIdxMax;

    idx i, k, sc;
    for(i = 0; src[i] && i<maxNum; i++) {
        for(sc = 0; src[i] && find[sc] && src[i] != find[sc]; sc++){}
        if( (find[sc] && !isMatch) || (!find[sc] && isMatch) ) {
            break;
        }
    }
    for(k = 0; i < len && src[i]; i++, k++) {
        SYM(k, &src[i]);
    }
    if( k ) {
        --k;
        char * d = isIN ? DST + k : dst->last() - 1;
        for(idx j=0; k > 0 && j<maxNum; --k, --d, ++j) {
            for(sc = 0; find[sc] && *d != find[sc]; sc++)
                {}
            if( (find[sc] && !isMatch) || (!find[sc] && isMatch) ) {
                break;
            }
        }
        if( !isIN) {
            dst->cut((idx) (d - dst->ptr() + 1));
        }
    }
    ZER(k + 1);
    return k;
}



char * sString::cleanMarkup(sStr * dst, const char * src,idx len, const char * startTag00, const char * endTag00,const char * replacement00,idx maxTags,bool inside, bool isPreserveQuotes,bool isCaseInSensitive)
{
    idx i,k,pos;
    idx notreplaced=0;
    bool inMark=false;
    idx iEnd=sNotIdx,iStart=sNotIdx;
    idx howmany=0,isin=0;
    char  inquote=0;

    if(!maxTags)maxTags=sIdxMax;
    if(!len)len=sLen(src);

    for(i=0,k=0; i<len && src[i] && howmany<maxTags;i++){
        if(isPreserveQuotes){
            if(!inquote && (src[i]=='\"' || src[i]=='\'') )inquote=src[i];
            else if(src[i]==inquote)inquote=0;
        }


        if(!inquote) {
            pos=compareChoice(src+i,startTag00,0,isCaseInSensitive,0,false,len-i);
            if(pos!=sNotIdx && (!isin)){iStart=i;if(inside)iStart+=pos-1;if(iStart<0)iStart=0;}

            if(pos==sNotIdx){
                pos= compareChoice(src+i,endTag00,0,isCaseInSensitive,0,false,len-i);
                if(pos!=sNotIdx&& (isin) ){iEnd=i;isin=0;if(!inside)iEnd+=pos-1;}
            }
        }

        if(i==iStart){inMark=true;isin=1;notreplaced=0;if(startTag00[0])continue;}
        if(i==iEnd){inMark=false;notreplaced=0;howmany++;continue;}

        if(inside==inMark){SYM(k,&src[i]);++k;}
        else if(!notreplaced){notreplaced=1;
            if(!replacement00){SYM(k,_);++k;}
            else {
                if(isIN){ for(idx ir=0;replacement00[ir];ir++){DST[k]=replacement00[ir];k++;} }
                else dst->add(replacement00,sLen(replacement00));
            }
        }

    }
    ZER(k);


    return nonconst(src+i);
}





char * sString::searchAndReplaceStrings(sStr * dst, const char * src,idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive)
{
    idx i,k;
    idx howmany=0,fnum;
    idx pos;
    const char * rpl;

    if(!src || !src[0])return nonconst(src);
    if(!maxTags)maxTags=sIdxMax;
    if(!len)len=sIdxMax;

    for(i=0,k=0;i<len && src[i] ;i++){

        if( howmany<maxTags &&  (pos=compareChoice(src+i,find00,&fnum,isCaseInSensitive,0,0, len - i))!=sNotIdx){
            if( !replacement00 ) {
                SYM(k, _);
                ++k;
            } else if( *replacement00 != 0 ) {
                rpl = sString::next00(replacement00, fnum);
                if( !rpl )
                    rpl = replacement00;
                if( isIN ) {
                    for(idx ir = 0; rpl[ir]; ir++) {
                        DST[k] = rpl[ir];
                        k++;
                    }
                } else {
                    dst->add(rpl, sLen(rpl));
                }
            }
            ++howmany;
            i += pos - 1;
        }else {
            SYM(k,&src[i]);++k;
        }

    }
    ZER(k);

    ++k;ZER(k);

    return nonconst(src+i);
}

char * sString::searchAndReplaceStringsPaired(sStr * dst, const char * src,idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive, bool replacementPair)
{
    idx i,k;
    idx howmany=0,fnum;
    idx pos;
    const char * rpl;

    if(!src || !src[0])return nonconst(src);
    if(!maxTags)maxTags=sIdxMax;
    if(!len)len=sIdxMax;

    for(i=0,k=0;i<len && src[i] ;i++){

        if( howmany<maxTags &&  (pos=compareChoice(src+i,find00,&fnum,isCaseInSensitive,0,0, len - i))!=sNotIdx){
            if( !replacement00 ) {
                SYM(k, _);
                ++k;
            } else if( *replacement00 != 0 ) {
                rpl = sString::next00(replacement00, fnum);

                if( rpl || !replacementPair){
                    if (!rpl) {
                        rpl = replacement00;
                    }
                    if( isIN ) {
                        for(idx ir = 0; rpl[ir]; ir++) {
                            DST[k] = rpl[ir];
                            k++;
                        }
                    } else {
                        dst->add(rpl, sLen(rpl));
                    }
                }
            }
            ++howmany;
            i += pos - 1;
        }else {
            SYM(k,&src[i]);++k;
        }

    }
    ZER(k);

    ++k;ZER(k);

    return nonconst(src+i);
}

char * sString::searchAndReplaceSymbols(sStr * dst , const char * src,idx len, const char * find, const char * replacement,idx maxTags,bool isMatch,bool isSkipMultiple,bool isPreserveQuotes, bool cleanTerminals, idx protectablequotes)
{
    idx i,k;
    idx sc;
    idx replaced=0;
    idx howmany=0;
    idx somecopied=0;
    idx ir;
    char  inquote=0;

    if(!src || !src[0])return nonconst(src);
    if(!maxTags)maxTags=sIdxMax;
    if(!len)len=sIdxMax;

    for(i=0,k=0;i<len && src[i] && howmany<maxTags;i++){

        if(isPreserveQuotes){
            bool ignoreThisQuote=(protectablequotes && (i>0 && src[i-1]=='\\') ) ;
            if(!inquote && (src[i]=='\"' || src[i]=='\'')  && !ignoreThisQuote )inquote=src[i];
            else if(src[i]==inquote && !ignoreThisQuote  )inquote=0;
            if(ignoreThisQuote && protectablequotes ==2 ) {
                if((void *)dst==(void *)src)--k;
                else dst->cut(-1);
            }
        }

        if(!inquote) {
            for(sc=0;src[i] && find[sc] && src[i]!=find[sc];sc++);
            if((find[sc] && isMatch) || (!find[sc] && !isMatch)){if( (replaced && isSkipMultiple) || (cleanTerminals && somecopied==0) )continue;
                if(!replacement){SYM(k,_);++k;}
                else {
                    for(ir=0;replacement[ir];ir++) {
                        if(isIN) {SYM(k,&replacement[ir]); ++k;}
                        else {dst->add(&replacement[ir],1);}
                    }
                }
                replaced=1;howmany++;
                continue;
            }
        }
        SYM( k, &src[i]); ++k;
        replaced=0;
        ++somecopied;
    }
    ZER(k);

    if(!isIN){++k;ZER(k);}
    return nonconst(src+i);
}

char *sString::searchandInvertStrings(sStr *dst, const char * src, idx len, const char * find, const char *replace)
{
    idx i,j;

    if(!src || !src[0])return nonconst(src);
    if(!len)len=sLen(src);
    idx lenfind = sLen(find);
    idx lenreplace = sLen (replace);
    idx endRange = len-1;

    for(i=len-1;i>0;--i){
        if(src[i]==find[0]){
            for(j=0;src[i+j] && find[j] && src[i+j]==find[j];j++);
            if (j==lenfind){
                if ((len - lenfind) > i){
                    if (endRange - (i+lenfind) > 0){
                        dst->add(&src[i+lenfind], endRange-(i+lenfind));
                        dst->add(replace, lenreplace);
                        endRange = i;
                    }
                }
                endRange = i;
            }
        }
    }
    dst->addString(src, endRange);

    return nonconst (src);;
}

static bool parseHexChar(char * dst, const char * src)
{
    *dst = 0;
    for( idx i=0; i<2; i++ ) {
        char c = 0;
        if( src[i] >= '0' && src[i] <= '9' ) {
            c = src[i] - '0';
        } else if ( src[i] >= 'a' && src[i] <= 'f' ) {
            c = 10 + src[i] - 'a';
        } else if ( src[i] >= 'A' && src[i] <= 'F' ) {
            c = 10 + src[i] - 'A';
        } else {
            return false;
        }

        *dst += c << (4 * (1-i));
    }
    return true;
}

static bool parseOctChar(char * dst, const char * src)
{
    *dst = 0;
    for( idx i=0; i<3; i++ ) {
        char c = 0;
        if( src[i] >= '0' && src[i] <= '7' ) {
            c = src[i] - '0';
        } else {
            return false;
        }

        *dst += c << (3 * (2-i));
    }
    return true;
}

idx sString::replaceEscapeSequences(sStr * dst, const char * src, idx len)
{
    if( !len ) len = sLen(src);
    idx i, j;

    for( i=0, j=0; i<len && src[i]; i++, j++ ) {
        if( src[i] == '\\' && i+1<len && src[i+1] ) {
            char c;
            switch( src[i+1] ) {
            case '\\':
            case '\'':
            case '"':
            case '?':
                dst->add(src+i+1, 1);
                i++;
                break;
            case 'a':
                dst->add("\a", 1);
                i++;
                break;
            case 'b':
                dst->add("\b", 1);
                i++;
                break;
            case 'f':
                dst->add("\f", 1);
                i++;
                break;
            case 'n':
                dst->add("\n", 1);
                i++;
                break;
            case 'r':
                dst->add("\r", 1);
                i++;
                break;
            case 't':
                dst->add("\t", 1);
                i++;
                break;
            case 'v':
                dst->add("\v", 1);
                i++;
                break;
            case '0':
                if( i+3<len && parseOctChar(&c, src+i+1) ) {
                    dst->add(&c, 1);
                    i += 3;
                    break;
                } else {
                    dst->add0();
                    i++;
                    break;
                }
            case 'x':
                if( i+3<len && parseHexChar(&c, src+i+2) ) {
                    dst->add(&c, 1);
                    i += 3;
                    break;
                }
            default:
                if( i+3<len && parseOctChar(&c, src+i+1) ) {
                    dst->add(&c, 1);
                    i+=3;
                } else {
                    dst->add(src+i, 1);
                }
            }
        } else {
            dst->add(src+i, 1);
        }
    }

    ZER(j);
    return j;
}

const char * sString::escapeForCSV(sStr & dst, const char * src, idx len)
{
    if( !len )
        len = sLen(src);

    idx dstStart = dst.length();

    bool needEscape = false;
    idx nquotes = 0;
    for(idx i = 0; i < len; i++) {
        char c = src[i];
        if( !c ) {
            len = i;
            break;
        }

        if( c == '"' ) {
            nquotes++;
            needEscape = true;
        } else if( c == ',' || c == '\r' || c == '\n' ) {
            needEscape = true;
            break;
        }
    }

    if( !len ) {
        dst.add0cut();
    } else if( needEscape ) {
        dst.resize(dst.length() + len + 3 + nquotes);
        dst.cut(dstStart);
        dst.add("\"", 1);
        for(const char * quote = (const char *) memchr(src, '"', len); len && quote; len -= quote + 1 - src, src = quote + 1, quote = (const char *) memchr(src, '"', len)) {
            if( quote > src ) {
                dst.add(src, quote - src);
            }
            dst.add("\"\"", 2);
        }
        if( len ) {
            dst.add(src, len);
        }
        dst.addString("\"", 1);
    } else {
        dst.addString(src, len);
    }
    return dst.ptr(dstStart);
}

const char * sString::unescapeFromCSV(sStr & dst, const char * src, idx len)
{
    if (!len)
        len = sLen(src);

    idx dstStart = dst.length();

    if (len == 0 || (len == 2 && src[0] == '"' && src[1] == '"')) {
        dst.add0cut();
    } else if (len > 2 && src[0] == '"' && src[len-1] == '"') {
        sString::searchAndReplaceStrings(&dst, src+1, len-2, "\"\"" __, "\"" __, 0, true);
        dst.shrink00();
    } else {
        dst.add(src, len);
        dst.add0cut();
    }
    return dst.ptr(dstStart);
}

const char * sString::escapeForShell(sStr & dst, const char * src, idx len)
{
    if (!src)
        src = "";

    if (!len)
        len = sLen(src);

    idx dstStart = dst.length();

    bool needEscape = false;
    for (idx i=0; i<len; i++) {
        char c = src[i];
        if (!c) {
            len = i;
            break;
        }

        if (c == '-' || c == '+' || c == '_' || c == '.' || c == ',' || c == '/' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
            continue;

        needEscape = true;
        break;
    }

    if( !len ) {
        dst.addString("''", 2);
    } else if( needEscape ) {
        bool inside_quote = false;
        for (idx i=0; i<len; i++) {
            idx j=0;
            while (i+j < len && src[i+j] != '\'')
                j++;

            if (j) {
                if( !inside_quote ) {
                    dst.addString("'", 1);
                    inside_quote = true;
                }
                dst.addString(src+i, j);
            }

            if (i+j < len) {
                if( inside_quote ) {
                    dst.add("'", 1);
                    inside_quote = false;
                }
                dst.addString("\\'", 2);
            }

            i+=j;
        }
        if( inside_quote ) {
            dst.addString("'", 1);
        }
        dst.add0cut();
    } else {
        dst.addString(src, len);
    }

    return dst.ptr(dstStart);
}

const char * sString::escapeForC(sStr & dst, const char * src, idx len)
{
    if (!src)
        src = "";

    if (!len)
        len = sLen(src);

    idx dstStart = dst.length();

    dst.add("\"", 1);

    for (idx i=0; i<len; i++) {
        switch (src[i]) {
        case '\0':
            dst.add("\\0", 2);
            break;
        case '\a':
            dst.add("\\a", 2);
            break;
        case '\b':
            dst.add("\\b", 2);
            break;
        case '\t':
            dst.add("\\t", 2);
            break;
        case '\n':
            dst.add("\\n", 2);
            break;
        case '\v':
            dst.add("\\v", 2);
            break;
        case '\f':
            dst.add("\\f", 2);
            break;
        case '\r':
            dst.add("\\r", 2);
            break;
        case '"':
            dst.add("\\\"", 2);
            break;
        case '\\':
            dst.add("\\\\", 2);
            break;
        default:
            if (src[i] < ' ' || src[i] > '~') {
                unsigned int u = (unsigned char)src[i];
                dst.printf("\\x%02X", u);
            } else {
                dst.add(src + i, 1);
            }
        }
    }

    dst.add("\"", 1);
    dst.add0cut();

    return dst.ptr(dstStart);
}

const char * sString::escapeForJSON(sStr & dst, const char * src, idx len)
{
    if (!src)
        src = "";

    if (!len)
        len = sLen(src);

    idx dstStart = dst.length();

    dst.add("\"", 1);

    for (idx i=0; i<len; i++) {
        switch (src[i]) {
        case '\b':
            dst.add("\\b", 2);
            break;
        case '\t':
            dst.add("\\t", 2);
            break;
        case '\n':
            dst.add("\\n", 2);
            break;
        case '\f':
            dst.add("\\f", 2);
            break;
        case '\r':
            dst.add("\\r", 2);
            break;
        case '"':
            dst.add("\\\"", 2);
            break;
        case '\\':
            dst.add("\\\\", 2);
            break;
        default:
            if( (src[i] >= 0 && src[i] < ' ') || src[i] == 0x7f ) {
                unsigned int u = (unsigned char)src[i];
                dst.printf("\\u%04X", u);
            } else {
                dst.add(src + i, 1);
            }
        }
    }

    dst.add("\"", 1);
    dst.add0cut();

    return dst.ptr(dstStart);
}

static bool parseUTF16Hex(sStr & dst, const char * src)
{
    uint16_t utf16 = 0;

    for(idx i=0; i<4; i++) {
        uint16_t nibble = 0;
        if( src[i] >= '0' && src[i] <= '9' ) {
            nibble = src[i] - '0';
        } else if ( src[i] >= 'a' && src[i] <= 'f' ) {
            nibble = 10 + src[i] - 'a';
        } else if ( src[i] >= 'A' && src[i] <= 'F' ) {
            nibble = 10 + src[i] - 'A';
        } else {
            return false;
        }

        utf16 += nibble << (4 * (3 - i));
    }

    unsigned char buf[3];
    if( utf16 < 0x80 ) {
        buf[0] = (unsigned char)utf16;
        dst.add((char*)buf, 1);
    } else if( utf16 < 0x800 ) {
        buf[0] = 0xC0 | (utf16 >> 6);
        buf[1] = 0x80 | (utf16 & 0x3F);
        dst.add((char*)buf, 2);
    } else {
        buf[0] = 0xE0 | (utf16 >> 12);
        buf[1] = 0x80 | ((utf16 >> 6) & 0x3F);
        buf[2] = 0x80 | (utf16 & 0x3F);
        dst.add((char*)buf, 3);
    }

    return true;
}

const char *  sString::unescapeFromJSON(sStr & dst, const char * src, idx len)
{
    if( !len ) {
        len = sLen(src);
    }
    idx i, j;

    idx dstStart = dst.length();

    if( len && src[0] == '"' ) {
        src++;
        len -= 2;
    }

    for( i=0, j=0; i<len && src[i]; i++, j++ ) {
        bool unrecognized = false;
        if( src[i] == '\\' && i+1<len && src[i+1] ) {
            switch( src[i+1] ) {
            case '"':
            case '\\':
            case '/':
                dst.add(src + i + 1, 1);
                i++;
                break;
            case 'b':
                dst.add("\b", 1);
                i++;
                break;
            case 'f':
                dst.add("\f", 1);
                i++;
                break;
            case 'n':
                dst.add("\n", 1);
                i++;
                break;
            case 'r':
                dst.add("\r", 1);
                i++;
                break;
            case 't':
                dst.add("\t", 1);
                i++;
                break;
            case 'u':
                if( i + 5 >= len || !parseUTF16Hex(dst, src + i + 2) ) {
                    unrecognized = true;
                }
                i += 5;
                break;
            default:
                unrecognized = true;
                i++;
            }
        } else if( (unsigned char)src[i] >= (unsigned char)' ' ){
            dst.add(src + i, 1);
        }

        if( unrecognized ) {
            dst.add("\xEF\xBF\xBD", 3);
        }
    }
    dst.add0cut();

    return dst.ptr(dstStart);
}

#define ST_START     1
#define ST_COLLECT   2
#define ST_TAILSPACE 3
#define ST_END_QUOTE 4

const char * sString::sepParseStr(const char *src, const char *srcend, char *buf, idx bn, unsigned char *row[], idx rn, idx sep, idx flags, idx *colnum, sStr *err)
{
    idx trim, quotes, ch, state, r, j, t, inquotes;

    trim = 0;
    quotes = flags;
    state = ST_START;
    inquotes = 0;
    ch = r = j = t = 0;

    while ( src<srcend && *src && *src != '\r' && *src != '\n'){
        ch = *src;

        ++src;
        switch(state) {
            case ST_START:
                if( ch != '\n' && ch != sep && isspace(ch) ) {
                    if( !trim ) {
                        buf[j++] = ch;
                        bn--;
                        t = j;
                    }
                    break;
                } else if( quotes && ch == '"' ) {
                    j = t = 0;
                    state = ST_COLLECT;
                    inquotes = 1;
                    break;
                }
                state = ST_COLLECT;
            case ST_COLLECT:
                if( inquotes ) {
                    if( ch == '"' ) {
                        state = ST_END_QUOTE;
                        break;
                    }
                } else if( ch == sep || ch == '\n' ) {
                    row[r++] = (unsigned char *) buf;
                    rn--;
                    if( ch == '\n' && t && buf[t - 1] == '\r' ) {
                        t--;
                        bn++;
                    }
                    buf[t] = '\0';
                    bn--;
                    buf += t + 1;
                    j = t = 0;
                    state = ST_START;
                    inquotes = 0;
                    if( ch == '\n' ) {
                        rn = 0;
                    }
                    break;
                } else if( quotes && ch == '"' ) {
                    return 0;
                }
                buf[j++] = ch;
                bn--;
                if( !trim || isspace(ch) == 0 ) {
                    t = j;
                }
                break;
            case ST_TAILSPACE:
            case ST_END_QUOTE:
                if( ch == sep || ch == '\n' ) {
                    row[r++] = (unsigned char *) buf;
                    rn--;
                    buf[j] = '\0';
                    bn--;
                    buf += j + 1;
                    j = t = 0;
                    state = ST_START;
                    inquotes = 0;
                    if( ch == '\n' ) {
                        rn = 0;
                    }
                    break;
                } else if( quotes && ch == '"' && state != ST_TAILSPACE ) {
                    buf[j++] = '"';
                    bn--;
                    t = j;
                    state = ST_COLLECT;
                    break;
                } else if( isspace(ch) ) {
                    state = ST_TAILSPACE;
                    break;
                }
                if (err){
                    err->printf("bad end quote in element %" DEC, (r + 1));
                }
                return 0;
        }
    }
    if( ch == -1 ) {
        return 0;
    }
    if( bn == 0 ) {
        if (err){
            err->printf("not enough space in buffer to store data");
        }
        return 0;
    }
    if( rn ) {
        if( inquotes && state != ST_END_QUOTE ) {
            if (err){
                err->printf("bad end quote in element");
            }
            return 0;
        }
        row[r] = (unsigned char *) buf;
        buf[t] = '\0';
    }

    if (colnum){
        *colnum = r+1;
    }
    while( (*src == '\r' || *src == '\n') && src < srcend ){
        ++src;
    }
    return src;
}

