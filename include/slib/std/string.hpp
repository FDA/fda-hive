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

/*! \file
 *  \brief A collection of string utility functions */

#include <time.h>

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>

namespace slib { 
    //! Collection of string utility functions
    class sString {
        public:
            //! 00-terminated list of newline characters
            #define sString_symbolsEndline  "\r\n" __
            //! 00-terminated list of whitespace symbols
            #define sString_symbolsBlank    " \t\r\n" __
            //! 00-terminated list of non-newline whitespace symbols
            #define sString_symbolsSpace    " \t" __
            //! \def sString_symbolsComment
            #define sString_symbolsComment  ";!" __


        public:
            // basics of strings
            //! check for NULL or empty string
            static bool empty(const char * ptr){return (ptr && *ptr) ? true : false; }
            //! hack for turning a const char pointer into non-const
            static char * nonconst(const char * p) {return (((char *)0)+ (p-((const char*)0)));}
            //! copy a string into a newly allocated buffer; equivalent of malloc()/strncpy()
            /*! \returns pointer to newly allocated string, or 0 on allocation failure */
            static char * dup(const char * ptr, idx size=0){return (char *)sDup(ptr,size);}
            //! concatenate strings into a newly allocated buffer
            /*! \returns pointer to newly allocated string, or 0 on allocation failure or if all strings were null/empty */
            static char * glue(idx cnt,  const char * firstPtr, ... );// glues a list of strings given as arguments returns the destination address
            //! Doubles the terminal 0 of a 0-terminated string
            /*! \param src 0-terminated string with enough buffer to append an additional 0
             *  \returns length of \a src    */
            static idx set00(char * src);
            //! Iterates over 0-terminated substrings of a 00-terminated string
            /*! \param cont00 00-terminated string
             *  \cnt index of 0-terminated substring in \a cont00, cnt == -1 brings random choice
             *  \returns 0-terminated substring number \a cnt of \a cont00 on success, or NULL on failure  */
            static char * next00( const char * cont00,idx cnt=1);
            //! Counts 0-terminated substrings of a 00-terminated string
            /*! \param cont00 00-terminated string
             *  \returns number of 0-terminated substrings in \a cont00; or 0 if \a cont00 is NULL */
            static idx cnt00( const char * cont00);
            //! Measures buffer size of a 00-terminated string
            /*! \param cont00 00-terminated string
             *  \returns buffer size for \a cont00 (including the terminal 00) if \a *cont00 is not 0;
                         1 if \a *cont00 is 0; and 0 if \a cont00 is NULL. */
            static idx size00( const char * cont00);
            //! Formats and concatenates 0-terminated substrings of a 00-terminated string
            /*! \param[in,out] dst concatenation result
             *  \param cont00 00-terminated string
             *  \param fmt printf() format string which will be used for each substring of \a cont00
             *  \param separ separator between formated substrings in result
             *  \returns pointer to the concatenated result in \a dst's buffer */
            static char * glue00( sStr * dst, const char * cont00, const char * fmt, const char * separ);
            //! reverse a string in place
            /*! \param n if zero, \a text is assumed to be 0-terminated, and all of it will be reversed;
                         if non-zero, the first \a n bytes will be reversed
             * \returns pointer to start of (now reversed) \a text */
            static char * inverse(char *text, idx n=0) ;
            // quicksearch 
            static idx _strstr_QuickSearchPreprocBuf[256];
            static void strstr_quickSearchPreProcess(const char * find, idx m) ;
            static const char * strstr_quickSearch(const char *text, idx n, const char * find, idx m) ;

            // string parsing functions
            static idx readChoiceList(idx * visCols, idx * sizCols, const char * cols00, const char * defline, const char * separ, bool isCaseInsensitive);
            static idx composeChoiceList(sStr * dfl,const char * cols00, idx * visCols, idx * sizCols, const char * separ);
            static idx scanRangeSet(const char * src,  idx len, sVec < idx > * range, idx shift, idx cidfirst, idx cidlast);
            static idx scanRangeSetSet(const char * src00, sVec < sVec < idx >  > * colset );
            static idx splitRange(real sVal, real fVal, idx cntTicks, sVec < real > * vec , bool filterBoundaries=false);
            //! parse a string as a boolean value; "false", "off", "cancel", "0", null pointer and empty strings are false; everything else is true
            static bool parseBool(const char *s)
            {
                // was made global in core/str.hpp for use in sVar to avoid numerous errors
                //while accessing boolean form values in backend implementations
                return slib::parseBool(s);
            }
            //! parse a string as an integer or boolean value
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
            //! check whether a string holds a boolean value
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

            //! "natural" string comparison: "x10.csv" > "x9.csv"
            static int cmpNatural(const char * a, const char * b, bool caseSensitive);

            //! parse time string in any common format: 13:45, 13:45:01, 1:45 pm, 13:45:01.123456Z, 13:45:01+01:00, 13:34:01-0100
            /*! \param s string to parse
                \param[out] pLenParsed if non-0, retrieves number of bytes successfully parsed
                \param[out] pExplicitUtcOffset if non-0, retrieves the UTC offset in seconds of the timezone
                    in the parsed string, or -sIdxMax if the timezone is not explicitly given
                \returns result as number of seconds since midnight, or -sIdxMax on parse failure */
            static idx parseTime(const char * s, idx * pLenParsed=0, idx * pExplicitUtcOffset=0);
            //! parse time string in any common format: 13:45, 13:45:01, 1:45 pm, 13:45:01.123456Z, 13:45:01+01:00, 13:34:01-0100
            /*! \param[inout] result tm structure with the month, day, year already filled in;
                                will be normalized to time in the system's timezone.
                \param s string to parse
                \param[out] pLenParsed if non-0, retrieves number of bytes successfully parsed
                \param[out] pExplicitUtcOffset if non-0, retrieves the UTC offset in seconds of the timezone
                    in the parsed string, or -sIdxMax if the timezone is not explicitly given
                \returns result as a Unix epoch time, or -sIdxMax on parse failure
                \note if the string doesn't have timezone information, it's assumed to be in the system's timezone */
            static idx parseTime(struct tm * result, const char * s, idx * pLenParsed=0, idx * pExplicitUtcOffset=0);
            //! parse date followed by optional timestamp; understands Unix epoch time, US-style M/D/Y dates, RFC 3339 (ISO 8601) and variations, RFC 2822, YYYY or YYYY-MM
            /*! \param[out] result tm structure, retrieves parsed time, normalized to the system's timezone.
                \param s string to parse.
                \param[out] pLenParsed if non-0, retrieves number of bytes successfully parsed
                \param[out] pExplicitUtcOffset if non-0, retrieves the UTC offset in seconds of the timezone
                    in the parsed string, or -sIdxMax if the timezone is not explicitly given
                \returns result as a Unix epoch time, or -sIdxMax on parse failure
                \note if the string is not a simple Unix epoch time and doesn't have timezone information,
                      it's assumed to be in the system's timezone */
            static idx parseDateTime(struct tm * result, const char * s, idx * pLenParsed=0, idx * pExplicitUtcOffset=0);
            //! Flags for printing date/time
            enum EPrintDateTimeFlags
            {
                fISO8601 = 1<<0, //!< use 'T' as date-time separator (instead of ' ')
                fNoDate = 1<<1, //!< do not print date component
                fNoTime = 1<<2, //!< do not print time or timezone components
                fNoTimezone = 1<<3 //!< do not print timezone component
            };
            //! Print date and time in RFC 3339 format
            /*! \param tm time to be printed; note that timezone information will be respected if present
                \param flags bitwise-or of sString::EPrintDateTimeFlags
                \returns pointer to start of printed string */
            static const char * printDateTime(sStr & out, const struct tm * tm, idx flags=0);
            //! Print date and time in RFC 3339 format
            /*! \param unix_time Unix epoch time to be printed
                \param flags bitwise-or of sString::EPrintDateTimeFlags
                \returns pointer to start of printed string */
            static const char * printDateTime(sStr & out, idx unix_time, idx flags=0);
            template <typename Tobj > static idx sscanfAnyVec(sVec < Tobj > * excl, const char * src, idx len, Tobj shift, Tobj rmin, Tobj rmax, const char * fmt, const char * separ=",;" sString_symbolsBlank ) 
            {
                if(!src) return 0;if(len==0)len=sLen(src);
                Tobj ex=0;
                char bb[128];
                idx i;
                for(const char * p=src; p && p<src+len && *p ; ) { //set global max_allowed_packet = 512 * 1024 * 1024;
                    ex=0;
                    for( i=0; p<src+len && *p && strchr(separ,*p)==0 ; ++p, ++i)
                        bb[i]=(*p);
                    bb[i]=0;sscanf(bb,fmt,&ex);
                    /*
                    for( ; p<src+len && *p && strchr(separ,*p)==0 ; ++p)
                        ex=ex*10+((*p)-'0');
                        */
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
                //if(str->length()) str->printf(separ);
                for(idx i=0 ; i< excl->dim() ; ++i ) { 
                    if(i) str->addString(separ);
                    str->printf(fmt,*(excl->ptr(i)));
                }return excl->dim();
            }
            static idx printfRVec(sStr * str, sVec < real > * excl, const char * separ="," ) {return printfAnyVec<real>(str, excl, "%lf", separ );}
            static idx printfIVec(sStr * str, sVec < idx > * excl, const char * separ=",")  {return printfAnyVec<idx>(str, excl, "%" DEC,separ);}
            static idx printfUVec(sStr * str, sVec < udx > * excl, const char * separ=",")  {return printfAnyVec<udx>(str, excl, "%" UDEC,separ);}

            enum eCase {
                eCaseNone=0, //!< no case modification
                eCaseLo, //!< lowercase
                eCaseHi //!< uppercase
            } ;
        
            // string searching functions 
            static idx compareChoice( const char *  src, const char * choice00,idx * numfnd,bool isCaseInSensitive, idx startNum, bool exactMatch=false, idx lenSrc=0);
            //! Compares up to \a n characters from two strings until a symbol from a list is reached.
            /*! The two strings are scanned until either \a n identical characters are found, or one of the strings
             * terminates with a 0, or a character from \a symblist is encountered.
             * \param str1, str2 strings to be compared
             * \param n max number of characters from \a str1 and \a str2 to compare
             * \param symblist symbol list
             * \param isCaseInSensitive whether comparison between \a str1 and \a str2 is case-insensitive
             * \returns 0 if the two strings were found to differ before scanning stopped; or the number of characters
             *          scanned and found identical if no difference was found.
             */
            static idx compareNUntil( const char * str1, const char * str2, size_t n, const char * symblist,bool isCaseInSensitive);
            //! Compares two strings until a symbol from a list is reached.
            /*! The two strings are scanned until either one of the strings terminates with a 0, or a character
             * from \a symblist is encountered.
             * \param str1, str2 strings to be compared
             * \param symblist symbol list
             * \param isCaseInSensitive whether comparison between \a str1 and \a str2 is case-insensitive
             * \returns 0 if the two strings were found to differ before scanning stopped; or the number of characters
             *          scanned and found identical if no difference was found.
             */
            inline static idx compareUntil( const char * str1, const char * str2, const char * symblist,bool isCaseInSensitive)
            {
                return compareNUntil(str1, str2, (size_t)-1, symblist, isCaseInSensitive);
            }
            static char * searchSubstring( const char * src, idx lenSrc, const char * find00,idx occurence, const char * stopFind00,bool isCaseInSensitive); // ,idx lenSrc
            static char * searchStruc( const char * src, idx len, const char * begin00, const char * end00,idx * pst,idx * pfn);
            static char * searchBlock( const char * src, idx len, const char * begin00, const char * end00,idx * pst,idx * pfn);
            //static char * skipWords( const char * src, idx num,const char * separators);
            static char * skipWords( const char * src, idx slen, idx num=1,const char * separators=sString_symbolsBlank);

    
            // string content editing functions 
            static idx changeCase(sStr * dst, const char * TextStr,idx len, eCase CaseType);
            static idx copyUntil(sStr * dst, const char * src, idx len,const char * symblist);
            static char * extractSubstring(sStr * dst,const char * src,idx len, idx nextStp,const char * nextSepar00, bool isCaseInSensitive,bool isPreserveQuotes);
            static idx cleanEnds(sStr * dst, const char * src,idx len,const char * find,bool isMatch,idx maxNum=0);
            static char * cleanMarkup(sStr * dst, const char * src,idx len, const char * startTag00, const char * endTag00,const char * replacement00,idx maxTags,bool inside,bool isPreserveQuotes,bool isCaseInSensitive);
            static char * searchAndReplaceSymbols(sStr * dst , const char * src,idx len, const char * find, const char * replacement,idx maxTags,bool isMatch,bool isSkipMultiple,bool isPreserveQuotes, bool cleanTerminals=false, idx protectablequotes=0);
            static char * searchAndReplaceStrings(sStr * dst, const char * src,idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive);
            static char * searchandInvertStrings(sStr *dst, const char * src, idx len, const char * find, const char *replace);
            static idx replaceEscapeSequences(sStr * dst, const char * src, idx len=0);
            static bool extractNCBIInfoSeqID(sStr *ginum, sStr *accnum, const char *seqid, idx seqlen = 0);

            //! Escape string for printing as a CSV cell (RFC-4180)
            static const char * escapeForCSV(sStr & dst, const char * src, idx len=0);
            //! Unescape and unquote a string from a CSV cell (RFC-4180)
            static const char * unescapeFromCSV(sStr & dst, const char * src, idx len=0);

            //! Escape string for safe use in Bourne Shell
            static const char * escapeForShell(sStr & dst, const char * src, idx len=0);

            //! Escape and quote a string for safe use as a string literal in C-like languages
            static const char * escapeForC(sStr & dst, const char * src, idx len=0);

            //! Escape and quote a UTF8 string for safe use as a JSON string value
            static const char * escapeForJSON(sStr & dst, const char * src, idx len=0);
            //! Unescape and unquote to a UTF8 string from a JSON string value
            static const char * unescapeFromJSON(sStr & dst, const char * src, idx len=0);

  /*          static idx changeCase(char * dst, const char * TextStr,idx len, eCase CaseType){return changeCase((sStr*)dst, TextStr,len, CaseType);}
            static idx copyUntil(char * dst, const char * src, idx len,const char * symblist){return copyUntil((sStr*)dst, src, len,symblist);}
            static char * extractSubstring(char * dst,const char * src,idx len, idx nextStp,const char * nextSepar00, bool isCaseInSensitive,bool isPreserveQuotes){return extractSubstring((sStr*)dst,src,len, nextStp,nextSepar00, isCaseInSensitive,isPreserveQuotes);}
            static idx cleanEnds(char * dst, const char * src,idx len,const char * find,bool isMatch){return cleanEnds((sStr*)dst, src,len,find,isMatch);}
            static char * cleanMarkup(char * dst, const char * src,idx len, const char * startTag00, const char * endTag00,const char * replacement00,idx maxTags,bool inside,bool isCaseInSensitive){return cleanMarkup((sStr *)dst, src,len, startTag00, endTag00,replacement00,maxTags,inside,isCaseInSensitive);}
            static char * searchAndReplaceSymbols(char * dst, const char * src,idx len, const char * find, const char * replacement,idx maxTags,bool isMatch,bool isSkipMultiple,bool isPreserveQuotes){return searchAndReplaceSymbols((sStr *)dst, src,len, find, replacement,maxTags,isMatch,isSkipMultiple,isPreserveQuotes);}
            static char * searchAndReplaceStrings(char * dst, const char * src,idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive){return searchAndReplaceStrings((sStr*)dst, src,len, find00, replacement00,maxTags,isCaseInSensitive);}
*/
            static idx changeCase(char * dst, idx len, eCase CaseType){return changeCase((sStr*)dst, dst,len, CaseType);}
            static idx copyUntil(char * dst, idx len,const char * symblist){return copyUntil((sStr*)dst, dst, len,symblist);}
            static char * extractSubstring(char * dst,idx len, idx nextStp,const char * nextSepar00, bool isCaseInSensitive,bool isPreserveQuotes){return extractSubstring((sStr*)dst,dst,len, nextStp,nextSepar00, isCaseInSensitive,isPreserveQuotes);}
            static idx cleanEnds(char * dst, idx len,const char * find,bool isMatch,idx maxNum=0){return cleanEnds((sStr*)dst, dst,len,find,isMatch,maxNum);}
            static char * cleanMarkup(char * dst, idx len, const char * startTag00, const char * endTag00,const char * replacement00,idx maxTags,bool inside,bool isPreserveQuotes,bool isCaseInSensitive){return cleanMarkup((sStr *)dst, dst,len, startTag00, endTag00,replacement00,maxTags,inside,isPreserveQuotes,isCaseInSensitive);}
            static char * searchAndReplaceSymbols(char * dst, idx len, const char * find, const char * replacement,idx maxTags,bool isMatch,bool isSkipMultiple,bool isPreserveQuotes, bool cleanTerminals=false, idx protectablequotes=0){return searchAndReplaceSymbols((sStr *)dst, dst,len, find, replacement,maxTags,isMatch,isSkipMultiple,isPreserveQuotes,cleanTerminals, protectablequotes);}
            static char * searchAndReplaceStrings(char * dst, idx len, const char * find00, const char * replacement00,idx maxTags,bool isCaseInSensitive){return searchAndReplaceStrings((sStr*)dst, dst,len, find00, replacement00,maxTags,isCaseInSensitive);}
//            static char * searchandInvertStrings(char * dst, idx len, const char * src, idx len, const char * find, const char *replace){{return searchAndInvertStrings((sStr*)dst, dst,len, find, replace);}};
            // string styling functions 
            static idx hungarianText(const char * TextStr,char * TextDest,idx sizeDest,eCase CaseType,bool IsName,bool IsRemIntBlanks);
            static idx  cStyle(sStr * dst,const char * src, idx len);
            static idx wrap(sStr * str,const char * src,const char * separ,idx charrayLen,sString::eCase caseChar, idx maxnum);

            //! vsscanf for non-0-terminated buffers; may not respect locale
            /*! \param textScan    text buffer to scan
             * \param textScanEnd pointer past end of buffer (e.g. to the terminal 0 in a 0-terminated string);
             *                    if \a textScanEnd is NULL, \a textScan is assumed to be 0-terminated.
             * \param formatDescription standard scanf format string
             * \returns number of items matched
             */
            static idx vbufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, va_list marker);

            //! sscanf for non-0-terminated buffers; may not respect locale
            /*! \param textScan    text buffer to scan
             * \param textScanEnd pointer past end of buffer (e.g. to the terminal 0 in a 0-terminated string);
             *                    if \a textScanEnd is NULL, \a textScan is assumed to be 0-terminated.
             * \param formatDescription standard scanf format string
             * \returns number of items matched
             */
            static idx bufscanf(const char *textScan, const char *textScanEnd, const char *formatDescription, ...) __attribute__((format(scanf, 3, 4)));
            // extended sprintf/sscanf functions 
            /*
            Struct xPrintfScanfCallbacks{
                scanfInt();
                scanf
            };*/
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
            //! Extended printf for enums and bitfields
            /*! Format specifiers \%i, \%d, \%u, \%x, \%p, \%f, \%g, \%e, \%c, \%s, \%% work like in printf();
                l and ll length modifiers are supported.

                Format specifier \%S prints content of sStr * s if s->length() > 0.

                Format specifier \%n formats enums; the \%n must be followed by '^', a list of
                names delimeted by '^', and terminated by ';' or end of string. Names have values
                correspond to their (zero-based) order, or an explicit value in hex can be provided. An
                idx argument is converted to the first name with a matching value, and if there's
                no match, the argument is printed as an integer.
                For example: \code
                    sString::xprintf(&s, "%n^a^b^c=3;\n", 1) // result is "b\n"
                    sString::xprintf(&s, "%n^a^b^c=3", 3) // result is "c"
                \endcode

                Format specifier \%b formats bitfields; the \%b must be followd by '|', a list of
                names delimeted by '|', and terminated by ';' or end of string. Names have values
                corresponding to the set bit of their (zero-based) order, or an explicit value in hex can
                be provied. An idx argument is convereted to a '|'-delimeted list of names that
                match the argument.
                For example: \code sString::xprintf(&s, "%b|a|b|c=3", 2); // result is "b|c" \endcode */
            static char *  xprintf(sStr * str,const char * formatDescription,...)
            {
                char * ret;
                sCallVargResPara(ret,sString::xvprintf,str,formatDescription);
                return ret;
            }
            //! Extended print for enums and bitfields that dereferences pointers
            /*! Like xprintf(), but \%i, \%d, \%u, \%x, \%p, \%f, \%g, \%e, \%c, \%n, and \%b format
                specifiers dereference pointers to their corresponding data types.
                For example: \code
                    idx val = 123;
                    sString::xprintp(&s, "%" DEC " %n^a^b^x=123", &val, &val); // result is "123 x" \endcode */
            static char *  xprintp(sStr * str,const char * formatDescription,...)
            {
                char * ret;
                sCallVargResPara(ret,sString::xvprintp,str,formatDescription);
                return ret;
            }
            static idx printHexArr( sStr * out, const void * Buf, idx siz , idx brk);

            struct SectVar {
                const char * visloc;
                const char * loc; // location of the variable in the input file 
                const char * fmtin, *fmtout;  // input and output Xformats for the variable : read and written by vString::xPrintf and vString:xScanf
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

    };
    //#define sSVR    sString::SectVar
    //inline const char * s00(_v_var) {return  sString::next00((_v_var)); } 
    
    
}

    #ifdef WIN32
        idx vsscanf(const char *s, const char *fmt, va_list ap);
    #endif
    
#endif
