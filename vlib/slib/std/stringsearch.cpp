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

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ String search functions
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


// compares if the curent src string is equal to one of strings in double-zero string <choice00>
// returns -1 if not found, or the length of found string otherwise
// puts numfnd to the ordinal of found string if not zero

idx sString::compareChoice( const char *  src, const char * choice00,idx * numfnd,bool isCaseInSensitive, idx startNum, bool exactMatch, idx lenSrc)
{
    idx sc,i,cnt;

    if(!lenSrc)lenSrc=sLen(src);
    if(!choice00[0]){if(numfnd)*numfnd=startNum;return 0;} // empty list

    for(cnt=0,sc=0;choice00[sc];cnt++) { // check if current string is equal to one of column separators in the separator stringList

        if(!isCaseInSensitive){
            for(i=0;choice00[sc] && i<lenSrc && src[i]==choice00[sc] ;i++,sc++) // compare while equal
                ;
        }
        else
            for(i=0;choice00[sc] && i<lenSrc && toupper((idx)src[i])==toupper((idx)choice00[sc]);i++,sc++) // compare while equal case insensitive
                ;

      if(!choice00[sc] && (exactMatch==false || !src[i]) ){// correposndence
          if(numfnd)*numfnd=cnt+startNum; // return the ordinal  of the found string
          return i;
      }
      while(choice00[sc])sc++; // check next string in stringlist for correspondence
      sc++;
    }
    return (idx)(-1);
}

//  compares up to n characters from two strings str1,str2 until a symbol from the symblist string
//    is reached.    Returns  zero if not equal otherwise returns the length of equal part
idx sString::compareNUntil( const char * str1, const char * str2, size_t n, const char * symblist,bool isCaseInSensitive)
{
    idx  i=0,j;

    for(j=0; (udx)j<n && (*str1) && (*str2); j++){ // until the end of the strings

        if(symblist){
            for(i=0;symblist[i];i++){if(symblist[i]==(*str2))break;} // look for symbol until the end of the symbol list
            if(symblist[i])break; // if found
        }

        if(!isCaseInSensitive && ((*str1)-(*str2)))break;
        else if(((toupper((idx)(*str1)))-(toupper((idx)(*str2)))))break;

        str1++;str2++;
    }
    if(symblist)return (symblist[i] || (udx)j>=n || !(*str2) || !(*str1)) ? j : 0 ;
    return j ;

}



//  looks for n-th occurence of one of given "find" 00 stringlist before stopFind00 is reached
//  src -source string
//  find - start of the tag to look for (is a list ending with two zero symbols )
//  stopFind - end of the tag to look for (is a list ending with two zero symbols )
//              if stopFind=="" search will be performed until the end of the string
//  occurence  - wich occurence to pick up
//  returns the found position or zero if not found
char * sString::searchSubstring( const char * src, idx lenSrc, const char * find00,idx occurence, const char * stopFind00,bool isCaseInSensitive) // ,idx lenSrc
{
    idx i,iFound=sNotIdx,ifnd=0,pos;
    if(!src || !find00)return 0;
    if(!lenSrc ) lenSrc=sLen(src);



    // find the string find if that is before the string endTag
    for(i=0;i<lenSrc && src[i];i++){ // for(i=0;src[i] && i<srcLen;i++){

        //* scan if the current position corresponds to the find
        pos=compareChoice(src+i,find00,0,isCaseInSensitive,0);
        if(pos!=sNotIdx){ // found find let's see if that is the occurence we need
            if(ifnd+1==occurence || occurence==sNotIdx)iFound=i;
            else ifnd++;
            if(occurence!=sNotIdx && iFound!=sNotIdx)break;
        }

        // scan if the current position corresponds to the endTag
        if(stopFind00 && stopFind00[0] && compareChoice(src+i,stopFind00,0,isCaseInSensitive,0)!=sNotIdx)
            break;
    }
    if(iFound==sNotIdx)return 0;
    return nonconst(src+iFound);
}



//  performs structured search
//    char * stringStructuredSearch(char * src,       source
//        char * begin,                               the stringlist for structure beginning tags
//        char * end,                                 the stringlist for structure endinning tags
//        idx * pst                                   points to the beginning of the found block
//        idx * pfn                                   points to the end of the found block
//        returns the beginning of the found block
//        example:
//        {
//            begin
//                start
//                      what I am looking for
//                finish
//            end
//        }
//        stringStructuredSearch (src,"{\0begin\0start\0\0","}\0end\0finish\0\0",&st,&fn);
//

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

//  skips several words
//    char * stringSkipWords(char * src,       source
//        idx num                             how many words to skip
//        char * separators ,                 separators between words
//        returns the position of the found word
//
char * sString::skipWords( const char * src, idx slen, idx num,const char * separators)
{
    idx i;
    const char * end=slen ? src+slen : 0 ;

    if(!separators)separators=(sString_symbolsBlank);
    while(*src && strchr(separators,*src))src++; // pass spaces
    for(i=0;(end==0 || src<end ) && *src && i<num;i++){
        while((end==0 || src<end ) && *src && !strchr(separators,*src))src++; // pass 1st non spaces
        while((end==0 || src<end ) && *src && strchr(separators,*src))src++; // pass spaces
    }
    if( (end!=0 && src>=end )) return 0;
    if(!(*src))return 0;
    return nonconst(src);
}




// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ String content editing functions
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

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
        // dst->add(&ch,1);
    }

    ZER(i);
    // dst->add(__,2);
    return i;
}



//  copies two strings str1,str2 until a symbol from the symblist string
//    is reached.
//    Returns the length of the copied buffer
idx sString::copyUntil(sStr * dst, const char * src, idx len,const char * symblist)
{
    idx i,j;

    if(!len)len=sIdxMax;

    for(j=0;j<len && src[j];j++){ // untill the end of the strings

        if(symblist){
            for(i=0;symblist[i];i++){if(symblist[i]==(src[j]))break;} // look for symbol until the end of the symbol list
            if(symblist[i])break; // if found
        }
        SYM(j,&src[j]);
        //dst->add(&src[j],1);
    }

    // dst->add(__,2);
    ZER(j);

    return j;
}


//  returns string from array of strings separated by some markup
//    char * stringGetValueFromArray(char * src,      source
//        char * dst,                                 destination
//        idx nextStp,                                how many separators to pass
//        char * nextSepar)                             separator stringlist

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

        if(pos!=sNotIdx){i+=pos-1;iNxt++;continue;}  // if current string is in the list
        // if we have finally reached what we needed
        if(iNxt==nextStp){SYM(k,&src[i]); ++k; }
        //if(iNxt==nextStp){dst->add(&src[i],1);}
        if(iNxt>nextStp)break;
    }


    ZER(k);
    //dst->add(__,2);

    return nonconst(src+i);
}


//  cleans start and end of string from the symbols of given set
//  char * stringCleanEnds(char * src,            source  string
//                char * dst,                     destination where to copy
//                char * find,                    look for these symbols
//                idx isMatch)                    1 if match is requred 0 otherwise
//    returns the length of resulting string
idx sString::cleanEnds(sStr * dst, const char * src, idx len, const char * find, bool isMatch, idx maxNum)
{
    // i= current symbol to read from src, k at destination
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
        // scan if the current position is in find string
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



//  cleans markup in the text
//    src -source string
//    dst -destination string (can be the same as source)
//    startTag - start of the tag to look for (is a list ending with two zero symbols )
//    endTag - end of the tag to look for (is a list ending with two zero symbols )
//    replacement - whatever is found between tags will be replaced by this
//    maxTags - maximum number of tag pairs to process
//    inside - = 1 if tags internal content is necessary to leave untouched 0 otherwise
//    returns the last treated position to continue from
char * sString::cleanMarkup(sStr * dst, const char * src,idx len, const char * startTag00, const char * endTag00,const char * replacement00,idx maxTags,bool inside, bool isPreserveQuotes,bool isCaseInSensitive)
{
    idx i,k,pos;        // i= current symbol to read from src, k at destination
    idx notreplaced=0;  // index for replacement string
    bool inMark=false;   // =1 inside of the markup tags; otherwise 0
    idx iEnd=sNotIdx,iStart=sNotIdx;      // where to change the mode
    idx howmany=0,isin=0;
    char  inquote=0;

    if(!maxTags)maxTags=sIdxMax; // default maxtags
    if(!len)len=sIdxMax;

    for(i=0,k=0; i<len && src[i] && howmany<maxTags;i++){
        if(isPreserveQuotes){
            if(!inquote && (src[i]=='\"' || src[i]=='\'') )inquote=src[i];
            else if(src[i]==inquote)inquote=0;
        }

        //if(inquote)
            //continue;

        // check if the current position corresponds to the startTag list
        if(!inquote) {
            pos=compareChoice(src+i,startTag00,0,isCaseInSensitive,0);
            if(pos!=sNotIdx && (!isin)){iStart=i;if(inside)iStart+=pos-1;if(iStart<0)iStart=0;}  // if inside is asked through away the markup tag recognizers

            // check if the current position corresponds to the endTag
            if(pos==sNotIdx){
                pos= compareChoice(src+i,endTag00,0,isCaseInSensitive,0);
                if(pos!=sNotIdx&& (isin) ){iEnd=i;isin=0;if(!inside)iEnd+=pos-1;} // if inside is asked through away the markup tag recognizers
            }
        }

        // determine if current position is inside of the Markup tag or outside
        if(i==iStart){inMark=true;isin=1;notreplaced=0;if(startTag00[0])continue;}
        if(i==iEnd){inMark=false;notreplaced=0;howmany++;continue;}

        // copy inside of markup if inside is asked or copy outside of markup if non-inside
        //if(inside==inMark){dst->add(&src[i],1);}
        if(inside==inMark){SYM(k,&src[i]);++k;}
        else if(!notreplaced){notreplaced=1;// copy the replacement to the found position
            //if(!replacement00){dst->add(_,1);}
            // else dst->add(replacement00,0);
            if(!replacement00){SYM(k,_);++k;}
            else {
                if(isIN){ for(idx ir=0;replacement00[ir];ir++){DST[k]=replacement00[ir];k++;} }
                else dst->add(replacement00,sLen(replacement00));
            }
        }

    }
    ZER(k);
    //dst->add(__,2);


    return nonconst(src+i);
}





//  find strings from given stringlist and replaces them
//    char * stringFindReplaceStrings(char * src,     source
//            char * dst,                             destination where to copy matches
//            char * find00,                          on of the strings in this stringlist will be found
//            char * replacement00,                   found will be replaced by corresponding number string from this string
//            idx maxTags,                            how many times perform the search&replace
//    returns the last treated position to continue from
char * sString::searchAndReplaceStrings(sStr * dst, const char * src,idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive)
{
    idx i,k;        // i= current symbol to read from src, k at destination
    idx howmany=0,fnum;
    idx pos; // index for replacement string
    const char * rpl;

    if(!src || !src[0])return nonconst(src);
    if(!maxTags)maxTags=sIdxMax; // default maxtags
    if(!len)len=sIdxMax;

    for(i=0,k=0;i<len && src[i] ;i++){

        // scan if the current position corresponds to the startTag
        if( howmany<maxTags &&  (pos=compareChoice(src+i,find00,&fnum,isCaseInSensitive,0,0, len - i))!=sNotIdx){
            //if(!replacement00){dst->add(_,1);}
            if( !replacement00 ) {
                SYM(k, _);
                ++k;
            } else if( *replacement00 != 0 ) {
                // find the replacer string fnum and replace
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
            //dst->add(&src[i],1);
        }

    }
    ZER(k);

    ++k;ZER(k);
    //dst->add(__,2);

    return nonconst(src+i);
}


//    find symbol from given symbol set and replaces them
//    char * stringFindReplaceSymbols(char * src,     source
//            char * dst,                             destination where to copy matches
//            char * find,                            symbols to find
//            char * replacement,                     symbols will be replaced by this string
//            idx maxTags,                            how many times perform the search&replace
//            idx isMatch,                            1 if match is required 0 otherweise
//            idx isSkipMultiple,                     1 skip multiple repetitions 0 otherwise
//            idx isPreserveQuotes)                   1 preserve the characters in quotes otherwise treat all
//    returns the last treated position to continue from
char * sString::searchAndReplaceSymbols(sStr * dst , const char * src,idx len, const char * find, const char * replacement,idx maxTags,bool isMatch,bool isSkipMultiple,bool isPreserveQuotes, bool cleanTerminals, idx protectablequotes)
{
    idx i,k;        // i= current symbol to read from src, k at destination
    idx sc;         // index for searching markUp tags
    idx replaced=0; // index for replacement string
    idx howmany=0;
    idx somecopied=0;
    idx ir; // index for replacement string
    char  inquote=0;

    if(!src || !src[0])return nonconst(src);
    if(!maxTags)maxTags=sIdxMax; // default maxtags
    if(!len)len=sIdxMax;

//    for(i=0,k=0;i<len && src[i] ;i++){
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

        // scan if the current position is in find string
        if(!inquote) {
            for(sc=0;src[i] && find[sc] && src[i]!=find[sc];sc++);
            if((find[sc] && isMatch) || (!find[sc] && !isMatch)){if( (replaced && isSkipMultiple) || (cleanTerminals && somecopied==0) )continue;
                //if(!replacement){dst->add(_,1);}
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
        // dst->add(&src[i],1);
        SYM( k, &src[i]); ++k;
        replaced=0;
        ++somecopied;
    }
    //dst->add(__,2);
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
                // We found it
                if ((len - lenfind) > i){// is it at the end of the string ??
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
    idx i, j; // i counts src, j counts dst

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

const char * sString::escapeForCSV(sStr & dst, const char * src, idx len/*=0*/)
{
    if (!len)
        len = sLen(src);

    idx dstStart = dst.length();

    // Do not use strcspn because src might not be 0-terminated
    bool needEscape = false;
    for (idx i=0; i<len; i++) {
        char c = src[i];
        if (!c) {
            len = i;
            break;
        }

        if (c == ',' || c == '\r' || c == '\n' || c == '"') {
            needEscape = true;
            break;
        }
    }

    if (!len) {
        dst.add0cut();
    } else if (needEscape) {
        dst.add("\"", 1);
        sString::searchAndReplaceStrings(&dst, src, len, "\"" __, "\"\"" __, 0, true);
        dst.shrink00();
        dst.add("\"", 1);
        dst.add0cut();
    } else {
        dst.add(src, len);
        dst.add0cut();
    }
    return dst.ptr(dstStart);
}

const char * sString::unescapeFromCSV(sStr & dst, const char * src, idx len/*=0*/)
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

const char * sString::escapeForShell(sStr & dst, const char * src, idx len/*=0*/)
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
            // null characters never allowed in shell
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
            // Replace single quotes with ' then \' then '
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

const char * sString::escapeForC(sStr & dst, const char * src, idx len/*=0*/)
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

const char * sString::escapeForJSON(sStr & dst, const char * src, idx len/*=0*/)
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
    idx i, j; // i counts src, j counts dst

    idx dstStart = dst.length();

    if( len && src[0] == '"' ) {
        // remove quotes
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
