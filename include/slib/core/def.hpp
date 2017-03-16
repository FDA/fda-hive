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
#ifndef sLib_core_def_hpp
#define sLib_core_def_hpp

#include <new>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <slib/core/os.hpp>

/*! \file
 *  \brief Basic types, macros, and memory manipulation functions
 */

#ifdef SLIB64
    //! default signed integer type for #slib; #idx64 on 64-bit platforms, \a int on 32-bit ones
    typedef long long idx;
    //! default unsigned integer type for #slib; #udx64 on 64-bit platforms, <em>unsigned int</em> on 32-bit ones
    typedef unsigned long long udx;
    //! printf() code for #idx, e.g. printf("x = %"DEC"\n", (idx)x)
    #define    DEC    "lli"
    //! printf() code for #udx, e.g. printf("x = %"UDEC"\n", (udx)x)
    #define UDEC "llu"
    //! printf() code for #udx as hex, e.g. printf("x = %"HEX"\n", (udx)x)
    #define    HEX    "llx"
    //! wrapper for atoi() / atoll() depending on size of #idx
    #define atoidx atoll
    //! wrapper for strtoul(a, NULL, 10) / strtoull(a, NULL, 10) depending on size of #udx
    #define atoudx(a) strtoull((a), 0, 10)
    //! wrapper for strtoi() / strtoll() depending on size of #idx
    #define strtoidx strtoll
    //! wrapper for strtoul() / strtoull() depending on size of #udx
    #define strtoudx strtoull
    //! wrapper for lrint() / llrint() depending on size of #idx
    #define idxrint llrint
    //! wrapper for lround() / llround() depending on size of #idx
    #define idxround llround
#else
    typedef int idx;
    typedef unsigned int udx;
    #define    DEC    "d"
    #define UDEC "u"
    #define    HEX    "x"
    #define atoidx atoi
    #define atoudx(u) strtoul((u), 0, 10)
    #define strtoidx strtol
    #define strtoudx strtoul
    #define idxrint lrint
    #define idxround lround
#endif
#define NUMAlphabet "0123456789ABCDEF"
//! signed 64-bit integer
typedef long long idx64;
//! unsigned 64-bit integer
typedef unsigned long long udx64;

#ifdef __GNUC__
//! mark x as a likely condition for optimization
#define likely(x) __builtin_expect(!!(x), 1)
//! mark x as an unlikely condition for optimization
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#define __attribute__(x) /*no-op*/
#endif

//! Core HIVE API namespace
namespace slib
{
    //! -1, used as guard value in various contexts
    #define sNotIdx                 ((idx)-1)
    //#define sNotPtr                 (0xFFFFFFFFFFFFFFFFll)
    //! max address, used as guard value in various contexts
    #define sNotPtr                 (((char * )0)-1)
    //! max signed value
    #define sIdxMax                 (~(((idx)1)<<((sizeof(idx)<<3)-1)))
    //! max unsigned value
    #define sUdxMax                 (~((udx)0))
    //! 0 pointer of specified type
    #define sNullPtr(_v_type)          ((_v_type*)(0x0ll))
    //! default floating point type used in #slib
    typedef double real;
    #define REAL_MAX  DBL_MAX
    #define REAL_MIN  DBL_MIN

    #define sIdxHighBit ((((idx)1)<<((sizeof(idx)<<3)-1)))
    #define sIdxHighMask (~sIdxHighBit)


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ NUMERALS
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    //! number of elements in an array
    #define sDim(vrn_vn)           ((int)( sizeof( (vrn_vn) ) / sizeof( (vrn_vn)[0] ) ) )
    //! maximum value of two variables
    template< class Tobj > inline Tobj sMax( Tobj v1, Tobj v2) {return v1>v2 ? v1 : v2;}
    //! minimum value from two variables
    template< class Tobj > inline Tobj sMin( Tobj v1, Tobj v2) {return v1<v2 ? v1 : v2;}
    //! clamp v within specified range
    template< class Tobj > inline Tobj sClamp( Tobj v, Tobj range_min, Tobj range_max ) {return range_min>range_max ? sClamp(v, range_max, range_min) : v<range_min ? range_min : v>range_max ? range_max : v; }
    //! if v is not within specified range, set to default_v
    template< class Tobj > inline Tobj sClamp( Tobj v, Tobj range_min, Tobj range_max, Tobj default_v ) {return range_min>range_max ? sClamp(v, range_max, range_min, default_v) : v<range_min ? default_v : v>range_max ? default_v : v; }
    //! absolute value of variable
    template< class Tobj > inline Tobj sAbs( Tobj v) {return v<0 ? -v : v ;}
    //! the sign of a value (1 or -1)
    template< class Tobj > inline Tobj sSign( Tobj v) {return v<0 ? -1 : 1 ;}
    //! the sign of a value, or 0 if the value equals zero
    template< class Tobj > inline Tobj sSig0( Tobj v) {return v==0 ? 0 : sSign(v) ;}
    //! the square of a value
    template< class Tobj > inline Tobj sSqr( Tobj v) {return v==0 ? v : v*v ;}
    //! if a1 and a2 overlaps with b1 and b2
    #define sOverlap(a1, a2, b1, b2) ( (a1 <= b1)? ((b1 <= a2)?1:0) : ((a1 <= b2)?1:0) )
    #define sSwapI(vrn_vn1,vrn_vn2)    {idx itmp=vrn_vn1; vrn_vn1=vrn_vn2; vrn_vn2=itmp; }
    //! swap two scalars using a temp
    template< class Tobj > inline void sSwap( Tobj & v1, Tobj & v2 ) { Tobj tmp = v1; v1 = v2; v2 = tmp; }

    //! change endianness
    inline idx sEndian(idx num) {
        return (
            (((num)&0xFF00000000000000ull)>>56) |
            (((num)&0x00FF000000000000ull)>>40) |
            (((num)&0x0000FF0000000000ull)>>24) |
            (((num)&0x000000FF00000000ull)>>8)  |
            (((num)&0x00000000FF000000ull)<<8)  |
            (((num)&0x0000000000FF0000ull)<<24) |
            (((num)&0x000000000000FF00ull)<<40) |
            (((num)&0x00000000000000FFull)<<56)
            )  ;
    }
    //! swap 32-bit halves of a 64-bit value
    inline idx sSwapHiLo(idx num) {
        return (
                ((num&0xFFFFFFFF)<<32) | ((num>>32)&0xFFFFFFFF)
            )  ;
    }
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ STRINGS
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    //! stringification macro
    #define sToStr(_v_var)         ( #_v_var)
    //! two zero characters, used for terminating 0-separated string lists
    #define __ "\0\0"
    #undef _
    //! zero character, used as separator in 00-terminated string lists in #slib
    #define _ "\0"


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ TYPES
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    // callback function typedefs
    typedef void * (* sCallbackUniversal)(...);
    typedef bool (* sCallbackLogical)(void * param,...);



    // pointer convertions macros
    //! cast an integer to a pointer of given type
    #define sConvInt2Ptr( _v_int, _v_typ )      ((_v_typ * )(((char *)0)+(_v_int)))
    //! cast a pointer to an integer
    #define sConvPtr2Int( _v_ptr)               ((idx)(((const char*)(_v_ptr))-((char *)0)))
    //! cast a pointer to a pointer of another type
    #define sConvPtr( _v_ptr, _v_typ)           sConvInt2Ptr( (sConvPtr2Int( (_v_ptr))) , _v_typ )
    template< class Tobj > inline Tobj *         sNonConst(const Tobj * p) {return (((Tobj *)0)+ (p-((const Tobj*)0)));}

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ VARARGS
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    //! call specified va_list handler
    #define sCallVarg(v_fun,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_fun)((v_frmt),ap);va_end(ap);}
    //! call specified va_list handler with given parameter as first argument
    #define sCallVargPara(v_fun,v_para,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_fun)((v_para),(v_frmt),ap);va_end(ap);}
    //! call specified va_list handler with given parameter as first argument, and save the result
    #define sCallVargPara2(v_fun,v_para1,v_para2,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_fun)((v_para1),(v_para2),(v_frmt),ap);va_end(ap);}
    //! call specified va_list handler and save the result
    #define sCallVargRes(v_res,v_fun,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_res)=(v_fun)((v_frmt),ap);va_end(ap);}
    //! call specified va_list handler with given parameter as first argument, and save the result
    #define sCallVargResPara(v_res,v_fun,v_para,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_res)=(v_fun)((v_para),(v_frmt),ap);va_end(ap);}
    //! call specified va_list handler with given parameter as first argument, and save the result
    #define sCallVargResPara2(v_res,v_fun,v_para1,v_para2,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_res)=(v_fun)((v_para1),(v_para2),(v_frmt),ap);va_end(ap);}

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ BITS AND FLAGS
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    //! 64-bit value with specified bit set
    inline idx sFlag(idx num) { return (((idx)1)<<num); }
    //! set specified bit set to 1 for a 64-bit idx
    inline void sBitON(idx &flag, idx num) { flag |= (((idx)1)<<num); }
    //! set specified bit set to 0 for a 64-bit idx
    inline void sBitOFF(idx &flag, idx num) { flag &= ~((((idx)1)<<num)); }
    //! set specified bit set to 0 for a 64-bit idx
    inline void sBit(idx &flag, idx num, bool mode) { if(mode) sBitON(flag,num); else sBitOFF(flag,num);}
    //! check if specified bit is set
    inline idx isFlag(idx var,idx num ) { return (var&sFlag(num)); }


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ MEMORY
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    //! return the length of a 0-terminated string or a zero for 0 pointers
    inline idx sLen(const void * ptr) {
        return ptr ? (idx ) strlen((const char *)ptr) : 0;
    }
    //! shift a pointer by a specified number of bytes (not by multiple of sizeof(Tobj))
    template< class Tobj > inline Tobj * sShift(Tobj * ptr, idx shift) {
       return (Tobj *)(((char * )ptr)+shift);
    }

    //! deletes the memory pointed by ptr, alway returns zero for ease of coding ptr=sDel(ptr);
    template< class Tobj > inline Tobj * sDel(Tobj * ptr) {
        if ( ptr) free ((void *)ptr);
        return 0;
    }
    //! equivalent of malloc()/realloc()/free()
    /*! \param ptr if zero - allocates memory of a given size;
     *             if non-zero - reallocates that memory
     *  \param size if zero - deletes \a ptr;
     *              if non-zero - amount of memory to (re)allocate
     *  \returns pointer to allocation if any allocated, or 0 otherwise */
    inline void * sNew(idx size, void * ptr=0)  {
        return size ? realloc(ptr,size) : sDel(ptr)  ;
    }

    //! make a memory copy of a string; equivalent of malloc()/strncpy()
    /*! \param size if positive - exact number of bytes to allocate and copy;
     *              if zero - the string is assumed to be 0-terminated (and is duplicated including terminal 0);
     *              if negative - the string is assumed to be 0-terminated, and -size extra bytes are allocated and copied
     * \returns pointer to copied string, or 0 on allocation failure */
    inline char * sDup(const char * ptr, idx size=0) {
        if(size==0) size=sLen(ptr)+1;
        else if(size<0) size=sLen(ptr)+1-size;
        void * dst=sNew(size);
        if(dst)memmove(dst,ptr,size);
        return (char *)dst;
    }

    //! make a memory copy of an arbitrary object; equivalent of malloc()/memcpy()
    /*! \param size if positive - exact number of bytes to allocate and copy;
     *              if zero - size is taken as sizeof(Tobj);
     *              if negative - -size extra bytes are allocated and copied
     * \returns pointer to copied object, or 0 on allocation failure */
    template< class Tobj > inline Tobj * sDup ( const Tobj * ptr, idx size=0)  {
        if(size==0) size=sizeof(Tobj) ;
        else if(size<0) size=sizeof(Tobj)-size;
        void * dst=sNew(size);
        if(dst)memmove(dst,ptr,size);
        return (Tobj*)dst;
    }

    //! fills a single memory object with a value; equivalent of memset()
    /*! \warn Use sSetArray() on arrays; sSet() will only initialize an array's first element */
    template< class Tobj > inline Tobj * sSet( Tobj * ptr, idx val=0,idx size=0)  {
        if(size==0) size=sizeof(Tobj) ;
        else if(size<0) size=sizeof(Tobj)-size;
        memset((void *)ptr,(int)val,size);
        return ptr;
    }

    //! fills an array of objects with a value; equivalent of memset()
    template< class Tobj, int n > inline Tobj * sSetArray( Tobj (& ary)[n], idx val=0,idx size=0)  {
        if(size==0) size=sizeof(ary) ;
        else if(size<0) size=sizeof(ary)-size;
        memset((void *)ary,(int)val,size);
        return ary;
    }

    #define sIPrintf(_v_buf, _v_len, _v_num, _v_base ) { \
        _v_len=0; \
        if( _v_num==0 ) { \
            (_v_buf)[(_v_len)]='0'; \
            ++(_v_len); \
        } else { \
            idx b=1, n=(_v_num); \
            while(b<=n) \
                {b*=(_v_base);}\
            while((b=b/(_v_base))>0){ \
                (_v_buf)[(_v_len)]=(n/b)+'0'; \
                n%=b; \
                ++(_v_len); \
            } \
        } \
        (_v_buf)[(_v_len)]=0; \
    }

    #define sNUMPrintf(_v_buf, _v_len, _v_num, _v_base, _v_alphabet ) { \
        _v_len=0; \
        if( _v_num==0 ) { \
            (_v_buf)[(_v_len)]='0'; \
            ++(_v_len); \
        } else { \
            idx b=1, n=(_v_num); \
            while(b<=n) \
                {b*=(_v_base);}\
            while((b=b/(_v_base))>0){ \
                (_v_buf)[(_v_len)]=(_v_alphabet)[(n/b)]; \
                n%=b; \
                ++(_v_len); \
            } \
        } \
        (_v_buf)[(_v_len)]=0; \
    }

    #define sFlPrintf(_v_buf, _v_len, _v_num, _v_base, _v_num_decimal ) { \
          ll=0; \
          if( _v_num==0 ) { \
              (_v_buf)[(ll)]='0'; \
              ++(ll); \
          } else { \
              idx b=1, n=(_v_num), nn = (_v_num); \
              while(b<=n){ \
                  b*=(_v_base); \
              } \
              while((b=b/(_v_base))>0){ \
                  (_v_buf)[(ll)]=(n/b)+'0'; \
                  n%=b; \
                  ++(ll); \
              } \
              float k = _v_num - nn; \
              if (k>0) { \
                  body[(ll)] = '.'; \
                  ++ll; \
                  idx num_decimal = 100; \
                  if (_v_num_decimal) num_decimal = _v_num_decimal; \
                  idx kk = k * num_decimal; \
                  b=1; n =kk; \
                  while(b<=n){ \
                     b*=(_v_base); \
                  } \
                  while((b=b/(_v_base))>0){ \
                     (_v_buf)[(ll)]=(n/b)+'0'; \
                     n%=b; \
                     ++(ll); \
                  } \
              } \
          } \
          (_v_buf)[(ll)]=0; \
      }

    #define sIPrintfFixBuf(_v_buf, _v_len, _v_num, _v_base , forceBufSize ) { \
        _v_len=0; \
        if( _v_num==0 ) { \
            (_v_buf)[(_v_len)]='0'; \
            ++(_v_len); \
        } else { \
            idx b=1, n=(_v_num); \
            while(b<=n) \
                {b*=(_v_base);}\
            while((b=b/(_v_base))>0){ \
                (_v_buf)[(_v_len)]=(n/b)+'0'; \
                n%=b; \
                ++(_v_len); \
            } \
        } \
        (_v_buf)[(_v_len)]=0; \
        for( ; (_v_len)<forceBufSize ; ++(_v_len) ) \
            (_v_buf)[(_v_len)]=0; \
    }

    #define sTPrintf(_v_buf, _v_len, _v_num, _v_base ) { \
        _v_len=0; \
        if( _v_num==0 ) { \
            (_v_buf)[(_v_len)]='0'; \
            ++(_v_len); \
        } else { \
            idx b=1, n=(_v_num); \
            while(b<=n) \
                {b*=(_v_base);}\
            while((b=b/(_v_base))>0){ \
                (_v_buf)[(_v_len)]=(n/b)+'a'; \
                n%=b; \
                ++(_v_len); \
            } \
        } \
        (_v_buf)[(_v_len)]=0; \
    }

    #define sIScanf(_v_r, _v_src, _v_len , _v_base ) { \
        _v_r=0; \
        char ch; \
        for ( idx i=0; i< (_v_len) && (isdigit(   (ch=(((const char*)(_v_src))[i]))    ) ) ; ++i ) { \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
        } \
    }
    #define sIScanf_Mv(_v_r, _v_src, _v_len , _v_base, _v_mv ) { \
        _v_r=0; \
        char ch; \
        for ( (_v_mv)=0; (_v_mv)< (_v_len) && (isdigit(   (ch=(((const char*)(_v_src))[(_v_mv)]))    ) ) ; ++(_v_mv) ) { \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
        } \
    }

    #define sIHiLoScanf(_v_r, _v_src, _v_len , _v_base ) { \
        _v_r=0; \
        char ch; \
        idx hi=0; \
        for ( idx i=0; i< (_v_len) && (isdigit(   (ch=(((const char*)(_v_src))[i]))    ) || ch==':' ) ; ++i ) { \
            if(ch==':') {hi=(_v_r); (_v_r)=0; continue;} \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
        } \
        (_v_r)|=(hi<<32); \
    }

    #define sIRngScanf(_v_r, _v_src, _v_len , _v_base , _v_mov) { \
        _v_r=isdigit((_v_src)[0]) ? 0 : 0x7FFFFFFF; \
        char ch, chP=0; \
        idx hi=0, i; \
        for ( i=0; i< (_v_len) && (isdigit(   (ch=(((const char*)(_v_src))[i]))    ) || ch=='+' || ch=='-' ) ; ++i ) { \
            if(ch=='+' || ch=='-' ) { hi=(_v_r); (_v_r)=(isdigit((_v_src)[i+1])) ? 0 : 0x7FFFFFFF; chP=ch;continue;} \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
        } \
        (_v_mov)+=i; \
        if( hi==0x7FFFFFFF){ (_v_r)=0x7FFFFFFF00000000ll; } \
        else { \
            if(chP==0) { hi=(_v_r);_v_r=0;} \
            else if(chP=='-' && _v_r!=0x7FFFFFFF) { (_v_r)-=hi;} \
             (_v_r)|=(hi<<32); \
        } \
    }

    #define sRScanf(_v_r, _v_src, _v_len, _v_base ) { \
        _v_r=0; \
        real vorder=0; \
        char ch; \
        for ( idx i=0; i< (_v_len) && (isdigit(   (ch=(((const char*)(_v_src))[i]))    )  || ch=='.')  ; ++i ) { \
            if(ch=='.'){vorder=1; continue;} \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
            if(vorder)vorder*=(_v_base); \
        } \
        if(vorder)_v_r/=vorder; \
    }

    //! aligns an integer (representing size, pointer, etc.) to the given alignment unit in bytes
    /*! used for pointer and memory size alignments */
    inline idx sAlign(idx pos,idx align) {
        return  (  align>1 && (pos&(align-1)) ) ?  ( pos + align ) & ( ( align-1 )^sNotIdx ) : pos;
    }
    //! fully random 64-bit value
    #define sRand64() (((((idx64)rand())&0xFFFF)<<48) | ((((idx64)rand())&0xFFFF)<<32) | ((((idx64)rand())&0xFFFF)<<16) | ((((idx64)rand())&0xFFFF)<<0))



    #define sNew_op(_v_p, _v_cnt)  for(idx jj=0;  jj<(_v_cnt);  ++jj) { new( ((_v_p)+jj) ) Tobj;}
    #define sDel_op(_v_p, _v_cnt)  for(idx jj=0;  jj<(_v_cnt);  ++jj) {  ((_v_p)+jj)->~Tobj();}

    //! case-insensitive string equality, matching prefix of \a what
    inline bool sIs(const char * cmd,  const char * what ){
        return (cmd && strncasecmp(what,cmd,sLen(cmd) )==0) ? true : false;
    }
    //! case-insensitive string equality, matching full value of \a what
    inline bool sIsExactly(const char * cmd,  const char * what ){
        idx len_cmd = sLen(cmd);
        idx len_what = sLen(what);
        return (cmd && len_cmd == len_what && strncasecmp(what,cmd,len_cmd)==0) ? true : false;
    }

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ DIAGNOSTICS
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    // debug output macros
    #define _sOut       printf
    #define _sOutDw     _sOut( "{\n")
    #define _sOutUp     _sOut( "}")

    // outputing values
/*    inline void sOutf( const int & obj)                { _sOut("%"DEC"",obj); }
    inline void sOutf( const bool & obj)            { _sOut("%s",obj ? "true" : "false"); }
    inline void sOutf( const unsigned int & obj)    { _sOut("%"DEC"",obj); }
    inline void sOutf( const double & obj)            { _sOut("%lf",obj); }
    inline void sOutf( const float & obj)            { _sOut("%f",obj); }
    inline void sOutf( const char & obj)            { _sOut("%c",obj); }
    inline void sOutf( const char * obj)            { _sOut("%s",obj); }
    inline void sOutx( const int & obj)                { _sOut("%x",obj); }
    inline void sOutf( const sCallbackUniversal & obj) { _sOut("%p",obj); }
    template <class Tobj> inline void sOutf( const Tobj * obj)    { _sOut("%p",obj); }
  */
    //inline const char * sType( const __int64 & /*obj*/)                { return "__int64"; }
    inline const char * sType( const int & /*obj*/)                { return "int"; }
    inline const char * sType( const bool & /*obj*/)            { return "bool"; }
    inline const char * sType( const unsigned int & /*obj*/)    { return "unsigned int"; }
    inline const char * sType( const double & /*obj*/)            { return "double"; }
    inline const char * sType( const float & /*obj*/)            { return "float"; }
    inline const char * sType( const char & /*obj*/)            { return "char"; }
    inline const char * sType( const char * /*obj*/)            { return "char *"; }
    inline const char * sType( const void * /*obj*/)            { return "void *"; }
    inline const char * sType( sCallbackUniversal & /*obj*/)    { return "function *"; }

    #define sOut(_v_var)     {_sOut("%s " #_v_var "=",sType(_v_var) ); sOutf(_v_var); _sOut(";\n"); }

    #define sOutClass(_v_cls) inline const char *  sType( const _v_cls & ) { return #_v_cls ; } inline const char *  sType( const _v_cls  * ) { return #_v_cls " *" ; } inline void sOutf( _v_cls & obj) { obj._sOutf();}

} // namespace slib







#endif // sLib_core_def_hpp



