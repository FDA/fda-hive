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

    class sStr : public sMex
    {
    public:
        static const char zero[12];
        sStr(idx flags=sMex::fBlockDoubling) : sMex ( flags ) {}
        sStr(const char * formatDescription, ...) __attribute__((format(printf, 2, 3)))
        {
            sCallVarg(vprintf,formatDescription);
        }

    public:
        const char * ptr(idx pos=0) const { return (const char *)sMex::ptr(pos); }
        char * ptr(idx pos=0) { return (char *)sMex::ptr(pos); }
        const char * ptrsafe(idx pos=0) const { return sMex::pos() ? static_cast<const char *>(sMex::ptr(pos)) : zero; }
        idx length(void ) const { return sMex::pos();}
        const char * last(void) const { return ptr(length()); }
        char * last(void) { return ptr(length()); }
        operator const char *() const { return ptr(); }
        operator char *() { return ptr(); }
        const char * operator *( ) const { return ptr(); }
        char * operator *() { return ptr(); }
        operator bool () const { return sMex::pos() != 0; }
        operator bool () { return sMex::pos() != 0; }
        bool operator==(const sStr &rhs) const { return !cmp(rhs.ptr(), rhs.length()); }
        bool operator!=(const sStr &rhs) const { return cmp(rhs.ptr(), rhs.length()); }
        bool operator==(const char *rhs) const { return !cmp(rhs, sLen(rhs)); }
        bool operator!=(const char *rhs) const { return cmp(rhs, sLen(rhs)); }
        friend bool operator==(const char *lhs, const sStr& rhs) { return !rhs.cmp(lhs, sLen(lhs)); }
        friend bool operator!=(const char *lhs, const sStr& rhs) { return rhs.cmp(lhs, sLen(lhs)); }
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
        idx cmp(const char *rhs, idx rhsLen) const
        {
            if( ptr() == rhs || (length() == 0 && !rhsLen)  ) return 0;
            return strncmp(ptr(), rhs, rhsLen);
        }

    public:
        char * add(const char * addb, idx sizeAdd=0)  { return ptr ( sMex::add( (const void *) addb, sizeAdd ? sizeAdd : sLen(addb)+1)  );  }
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
        char * addNum(int n, udx base=10)
        {
            return addNum((idx)n, base);
        }
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

        char * cutAddString(idx pos, const char * addb, idx sizeAdd=0)
        {
            cut(pos);
            return addString(addb, sizeAdd);
        }
        char * resize(idx sizeRequired)  { return ptr ( sMex::resize( sizeRequired )  );  }
        char * remap( idx setflags=0, idx unsetflags=0, bool onlyIfFlagsChanged=false , idx offset=0, idx sizemap=0) { return static_cast<char *>(sMex::remap(setflags, unsetflags, onlyIfFlagsChanged,offset,sizemap)); }
        static idx vprintfSizeOf(const char * formatDescription,va_list marker);
        char * vprintf( const char * formatDescription, va_list marker );
        char * printf(const char * formatDescription, ...) __attribute__((format(printf, 2, 3)))
        {
            char * ret;
            sCallVargRes(ret,vprintf,formatDescription);
            return ret;
        }
        char * printf(idx pos, const char * formatDescription, ...) __attribute__((format(printf, 3, 4)))
        {
            char * ret;
            cut(pos);
            sCallVargRes(ret,vprintf,formatDescription);
            return ret;
        }
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
        char * add0(idx cntsep=1){return addSeparator(0,cntsep);}
        char * add0cut(idx cntsep=1)
        {
            idx l = length();
            add0(cntsep);
            return ptr(cut(l));
        }
        char * cut0cut(idx pos=0, idx cntsep=1)
        {
            cut(pos);
            add0(cntsep);
            return ptr(cut(pos));
        }
        void shrink00(const char * separ=0, idx  dozeros=0);
        void replace(const sStr& s) { replace(s ? s.ptr() : 0); }
        void replace(const char* s) { cut(0); if( s ) { add(s); } }

        idx recCnt(bool skipEmpty = true, char sep = 0);
        bool recNext(sMex::Pos & rec, bool skipEmpty = true, char sep = 0);
        bool recNth(const idx offset, sMex::Pos & rec, bool skipEmpty = true, char sep = 0);

        const sMex * mex() const { return static_cast<const sMex *>(this); }
        sMex * mex() { return (sMex *)this; } 
    
    public:


    private:

        sStr(const sStr&);
        sStr& operator =(const sStr&);
    };

    class sFil: public sStr {
        public:
            sFil( const char * flnm=0, idx flags=sMex::fBlockDoubling) : sStr (flags) { sStr::init(flnm) ; }
    };

    class sStrT: public sStr {
        public:
            sStrT( const char * flnm=0, idx flags=sMex::fBlockCompact) : sStr (flags) { sStr::init(flnm) ; }
    };

}

#endif 
