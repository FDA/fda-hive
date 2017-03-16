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
#ifndef sLib_core_str_hpp
#define sLib_core_str_hpp

#include <slib/core/mex.hpp>

namespace slib
{

    inline bool parseBool(const char *s)
    {
        return s && s[0] && strcmp(s, "0") && strcasecmp(s, "false") && strcasecmp(s, "off") && strcasecmp(s, "cancel");
    }

    //! An sMex for storing character data
    class sStr : public sMex
    {
    public:
        //! standard empty string
        static const char zero[12];
        // construction 
        sStr(idx flags=sMex::fBlockDoubling) : sMex ( flags ) {}
        sStr(const char * formatDescription, ...) __attribute__((format(printf, 2, 3)))
        {
            sCallVarg(vprintf,formatDescription);
        }

    public:
        //! returns a pointer, doesn't check for boundaries
        const char * ptr(idx pos=0) const { return (const char *)sMex::ptr(pos); }
        //! returns a pointer, doesn't check for boundaries
        char * ptr(idx pos=0) { return (char *)sMex::ptr(pos); }
        //! if length() > 0, returns a pointer and doesn't check for boundaries; but if length() == 0, returns an empty string
        const char * ptrsafe(idx pos=0) const { return sMex::pos() ? static_cast<const char *>(sMex::ptr(pos)) : zero; }
        //! number of bytes marked as used in memory buffer
        /*! \warning may or may not include terminal 0 - use shrink00() to be sure it's not counted */
        idx length(void ) const { return sMex::pos();}
        //! pointer past end of used part of memory buffer
        const char * last(void) const { return ptr(length()); }
        //! pointer past end of used part of memory buffer
        char * last(void) { return ptr(length()); }
        operator const char *() const { return ptr(); } // operator typecast to const char *
        operator char *() { return ptr(); } // operator typecast to char *
        const char * operator *( ) const { return ptr(); } // const dereferencing operator
        char * operator *() { return ptr(); } // dereferencing operator
        //! true if length() > 0
        operator bool () const { return sMex::pos() != 0; }
        // non-const operator bool() needed because c++ prefers "operator char*()" over "operator bool() const" in non-const context
        // TODO: replace with "explicit operator char*()" when we switch to c++11
        operator bool () { return sMex::pos() != 0; }
        //! true if both strings are empty/null, or both point to same buffer, or match by strcmp
        bool operator==(const sStr &rhs) const { return !cmp(rhs.ptr(), rhs.length()); }
        bool operator!=(const sStr &rhs) const { return cmp(rhs.ptr(), rhs.length()); }
        //! true if both strings are empty/null, or both point to same buffer, or match by strcmp
        bool operator==(const char *rhs) const { return !cmp(rhs, sLen(rhs)); }
        bool operator!=(const char *rhs) const { return cmp(rhs, sLen(rhs)); }
        //! true if both strings are empty/null, or both point to same buffer, or match by strcmp.  Allows for left operand const char * and right operand sStr
        friend bool operator==(const char *lhs, const sStr& rhs) { return !rhs.cmp(lhs, sLen(lhs)); }
        friend bool operator!=(const char *lhs, const sStr& rhs) { return rhs.cmp(lhs, sLen(lhs)); }
        // ... and lots of const/non-const variations to avoid built-in operator==(char*,char*)
        // TODO: use "explicit" to punt most/all of these when we switch to c++11
        inline bool operator==(sStr &rhs) const { return !cmp(rhs.ptr(), rhs.length()); }
        inline bool operator==(const sStr &rhs) { return !cmp(rhs.ptr(), rhs.length()); }
        inline bool operator==(sStr &rhs) { return !cmp(rhs.ptr(), rhs.length()); }
        inline bool operator!=(sStr &rhs) const { return cmp(rhs.ptr(), rhs.length()); }
        inline bool operator!=(const sStr &rhs) { return cmp(rhs.ptr(), rhs.length()); }
        inline bool operator!=(sStr &rhs) { return cmp(rhs.ptr(), rhs.length()); }
        inline bool operator==(char *rhs) const { return !cmp(rhs, sLen(rhs)); }
        inline bool operator==(const char *rhs) { return !cmp(rhs, sLen(rhs)); }
        inline bool operator==(char *rhs) { return !cmp(rhs, sLen(rhs)); }
        inline bool operator!=(char *rhs) const { return cmp(rhs, sLen(rhs)); }
        inline bool operator!=(const char *rhs) { return cmp(rhs, sLen(rhs)); }
        inline bool operator!=(char *rhs) { return cmp(rhs, sLen(rhs)); }
        friend bool operator==(char *lhs, const sStr& rhs) { return !rhs.cmp(lhs, sLen(lhs)); }
        friend bool operator==(const char *lhs, sStr& rhs) { return !rhs.cmp(lhs, sLen(lhs)); }
        friend bool operator==(char *lhs, sStr& rhs) { return !rhs.cmp(lhs, sLen(lhs)); }
        friend bool operator!=(char *lhs, const sStr& rhs) { return rhs.cmp(lhs, sLen(lhs)); }
        friend bool operator!=(const char *lhs, sStr& rhs) { return rhs.cmp(lhs, sLen(lhs)); }
        friend bool operator!=(char *lhs, sStr& rhs) { return rhs.cmp(lhs, sLen(lhs)); }
        //! 0 if both strings are empty/null, or both point to same buffer; otherwise, compare by strcmp
        idx cmp(const char *rhs, idx rhsLen) const
        {
            if( ptr() == rhs || (length() == 0 && !rhsLen)  ) return 0;
            return strncmp(ptr(), rhs, rhsLen);
        }

    public:
        //! append character buffer at end of used memory buffer
        /*! \param sizeAdd number of bytes to append; if 0, addb is interpreted as 0-terminated string and is appended including the terminal 0
            \returns pointer to start of appended region in buffer
            \warning if \a addb == 0 and \a sizeAdd == 0, appends 1 byte of potentially uninitialized memory!
            \note In most cases, \ref addString() is better for appending strings */
        char * add(const char * addb, idx sizeAdd=0)  { return ptr ( sMex::add( (const void *) addb, sizeAdd ? sizeAdd : sLen(addb)+1)  );  }  // appends char * buffer to the end
        //! append string at end of used memory buffer, with terminal 0 added but not counted in length()
        /*! \param addb string to add; if 0, appends \a sizeAdd uninitialized bytes (plus terminal 0 not counted in length())
            \param sizeAdd number of bytes to apend; if \a sizeAdd is 0 and \a addb is non-zero, \a addb is interpreted as a 0-terminated string, all of which is appended
            \returns pointer to start of appended region in buffer
            \note A terminal 0 is always appended, but is not counted in length().
                  As a result, after addString(), ptr() is safe for use in libc
                  string handling functions such as strlen, strcmp, etc. */
        char * addString(const char * addb, idx sizeAdd=0)
        {
            if( !sizeAdd ) {
                sizeAdd = sLen(addb);
            }
            idx start = sMex::add(0, sizeAdd + 1);
            if( likely(addb) ) {
                memmove(ptr(start), addb, sizeAdd);
            }
            *ptr(start + sizeAdd) = 0;
            cut(start + sizeAdd);
            return ptr(start);
        }
        //! print number at end of used memory buffer
        /*! \param n number to print
            \param base number base in which to print (8, 10, 16 are supported)
            \returns pointer to start of appended region in buffer
            \note A terminal 0 is always appended, but is not counted in length() */
        char * addNum(idx n, udx base=10)
        {
            bool neg = false;
            if( n < 0 ) {
                neg = true;
                n = -n;
            }
            idx digits = neg ? 2 : 1;
            for(idx b = 1, b_max = n / base; b <= b_max; b *= base, digits++);
            idx end_pos = length() + digits;
            idx start_pos = sMex::add(NULL, digits + 1);
            for(idx i = digits - 1; i >= neg ? 1 : 0; i--, n /= base) {
                idx digit = n % base;
                *ptr(start_pos + i) = digit < 10 ? '0' + digit : 'A' - 10 + digit;
            }
            *ptr(end_pos) = 0;
            if( neg ) {
                *ptr(start_pos) = '-';
            }
            cut(end_pos);
            return ptr(start_pos);
        }
        //! print number at end of used memory buffer
        /*! \param n number to print
            \param base number base in which to print (8, 10, 16 are supported)
            \returns pointer to start of appended region in buffer
            \note A terminal 0 is always appended, but is not counted in length() */
        char * addNum(int n, udx base=10)
        {
            return addNum((idx)n, base);
        }
        //! print number at end of used memory buffer
        /*! \param n number to print
            \param base number base in which to print (8, 10, 16 are supported)
            \returns pointer to start of appended region in buffer
            \note A terminal 0 is always appended, but is not counted in length() */
        char * addNum(udx n, udx base=10)
        {
            idx digits = 1;
            for(udx b = 1, b_max = n / base; b <= b_max; b *= base, digits++);
            idx end_pos = length() + digits;
            idx start_pos = sMex::add(NULL, digits + 1);
            for(idx i = digits - 1; i >= 0; i--, n /= base) {
                idx digit = n % base;
                *ptr(start_pos + i) = digit < 10 ? '0' + digit : 'A' - 10 + digit;
            }
            *ptr(end_pos) = 0;
            cut(end_pos);
            return ptr(start_pos);
        }

        //! equivalent to cut(pos); addString(addb, sizeAdd);
        char * cutAddString(idx pos, const char * addb, idx sizeAdd=0)
        {
            cut(pos);
            return addString(addb, sizeAdd);
        }
        //char * resize(idx sizeAdd)  { return ptr ( sMex::add( 0, sizeAdd )  );  }  // appends char * buffer to the end
        //! expand the buffer if necessary to fit so many bytes total without changing length()
        /*! \returns last(), which is now the start of \a sizeAdd - length() bytes of unused memory */
        char * resize(idx sizeRequired)  { return ptr ( sMex::resize( sizeRequired )  );  }
        //! reallocate memory or remap a file, possibly using a new set of flags
        /*! \param setflags bitwise-or of sMex::bitFlags to set
            \param unsetflags bitwise-or of sMex::bitFlags to unset
            \param onlyIfFlagsChanged remap only if the new set of flags is different
            \returns pointer to new allocation or map, or 0 on allocation/map failure
            \warning unsetting sMex::fReadonly on a file map is not allowed due to operating system limitations */
        char * remap( idx setflags=0, idx unsetflags=0, bool onlyIfFlagsChanged=false , idx offset=0, idx sizemap=0) { return static_cast<char *>(sMex::remap(setflags, unsetflags, onlyIfFlagsChanged,offset,sizemap)); }
        //! estimate size of printf output
        static idx vprintfSizeOf(const char * formatDescription,va_list marker);
        char * vprintf( const char * formatDescription, va_list marker );
        //! append formatted string at the end of used part of the memory buffer; terminal zero is added, but length() is automatically adjusted to not count it
        /*! \returns start of added formatted string */
        char * printf(const char * formatDescription, ...) __attribute__((format(printf, 2, 3)))
        {
            char * ret;
            sCallVargRes(ret,vprintf,formatDescription);
            return ret;
        }
        //! add formatted string at a specified position; terminal zero is added, but length() is automatically adjusted to not count it
        /*! \returns start of added formatted string */
        char * printf(idx pos, const char * formatDescription, ...) __attribute__((format(printf, 3, 4)))
        {
            char * ret;
            cut(pos);
            sCallVargRes(ret,vprintf,formatDescription);
            return ret;
        }
        //! appends separators (0 characters by default) at end of used part of memory buffer
        /*! \param separator if 0, adds \a cntsep 0 characters; if empty string, does nothing; if non-empty string, adds \a cntsep copies of it (without terminal zeros)
            \returns pointer to added sequence of separators */
        char * addSeparator(const char * separator = 0, idx cntsep = 1)
        {
            idx retpos = length();
            idx len = 0;
            if( !separator ) {
                separator = _;
                len = 1;
            } else if( separator[0] ) {
                len = sLen(separator);
            }
            if( len ) {
                for(idx i = 0; i < cntsep; ++i) {
                    add(separator, len);
                }
            }
            return ptr(retpos);
        }
        //! appends \a cntsep 0 characters at end of used part of memory buffer
        char * add0(idx cntsep=1){return addSeparator(0,cntsep);}
        //! appends \a cntsep 0 characters without changing length()
        char * add0cut(idx cntsep=1)
        {
            idx l = length();
            add0(cntsep);
            return ptr(cut(l));
        }
        //! cuts at \a pos and then appends \a cntsep 0 characters without changing length()
        char * cut0cut(idx pos=0, idx cntsep=1)
        {
            cut(pos);
            add0(cntsep);
            return ptr(cut(pos));
        }
        //! decreases length() so as not to include terminal 0
        /*! \param separ if non-0, interpreted as a 0-terminated list of characters, and shrink length() until one of the characters in the list is found
            \param dozeros number of 0 characters (which \em will be counted in length()) to add after shrinking */
        void shrink00(const char * separ=0, idx  dozeros=0);
        void replace(const sStr& s) { replace(s ? s.ptr() : 0); }
        void replace(const char* s) { cut(0); if( s ) { add(s); } }

        //! Counts records in the buffer
        /*! \param skipEmtpy if true when separators are next to each other empty record assumed
         *  \param sep record separator character, if 0 - \n, \r, \r\n assumed
         *  \returns number of separator-terminated records */
        idx recCnt(bool skipEmpty = true, char sep = 0);
        //! Returns next record from position rec
        /*! \param rec if rec.pos == sIdxMax - from start (1st rec)
         *  \param skipEmtpy see ::recCnt
         *  \param sep see ::recCnt
         *  \returns true - rec has new value, false on EOF */
        bool recNext(sMex::Pos & rec, bool skipEmpty = true, char sep = 0);
        //! Returns next record from position rec
        /*! \param offset number of record to skip FORWARD from where rec points to
         *  \param rec see ::recCnt
         *  \param skipEmtpy see ::recCnt
         *  \param sep  see ::recCnt
         *  \returns true - rec has value, false on not found */
        bool recNth(const idx offset, sMex::Pos & rec, bool skipEmpty = true, char sep = 0);

        //! cast to sMex
        const sMex * mex() const { return static_cast<const sMex *>(this); }
        //! cast to sMex
        sMex * mex() { return (sMex *)this; } 
    
    public:
        //inline void _sOutf( void ) { _sOutDw;sMex::_sOutf();_sOutUp; }

        //block copying :

    private:

        sStr(const sStr&);
        sStr& operator =(const sStr&);
    };
    //sOutClass(sStr)

    //! Wrapper with a convenient constructor for using sStr to write or read files
    class sFil: public sStr {
        public:
            sFil( const char * flnm=0, idx flags=sMex::fBlockDoubling) : sStr (flags) { sStr::init(flnm) ; }
    };

    //! Wrapper with a convenient constructor for using sStr to write or read files; when writing, grows by chunks of 128 bytes
    class sStrT: public sStr {
        public:
            sStrT( const char * flnm=0, idx flags=sMex::fBlockCompact) : sStr (flags) { sStr::init(flnm) ; }
    };

}

#endif // sLib_core_str_hpp

