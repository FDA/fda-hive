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
#include <ctype.h>
#include <float.h>
#include <slib/core/os.hpp>


#ifdef SLIB64
    typedef long long idx;
    typedef unsigned long long udx;
    #define    DEC    "lli"
    #define UDEC "llu"
    #define    HEX    "llx"
    #define atoidx atoll
    #define atoudx(a) strtoull((a), 0, 10)
    #define strtoidx strtoll
    #define strtoudx strtoull
    #define idxrint llrint
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
typedef long long idx64;
typedef unsigned long long udx64;

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#define __attribute__(x)
#endif

#define UNUSED_VAR(expr) do { (void)(expr); } while (0)

namespace slib
{
    #define sNotIdx                 ((idx)-1)
    #define sNotPtr                 (((char * )0)-1)
    #define sIdxMax                 (~(((idx)1)<<((sizeof(idx)<<3)-1)))
    #define sUdxMax                 (~((udx)0))
    #define sNullPtr(_v_type)          ((_v_type*)(0x0ll))
    typedef double real;
    #define REAL_MAX  DBL_MAX
    #define REAL_MIN  DBL_MIN

    #define sIdxHighBit ((((idx)1)<<((sizeof(idx)<<3)-1)))
    #define sIdxHighMask (~sIdxHighBit)



    #define sDim(vrn_vn)           ((int)( sizeof( (vrn_vn) ) / sizeof( (vrn_vn)[0] ) ) )
    template< class Tobj > inline Tobj sMax( Tobj v1, Tobj v2) {return v1>v2 ? v1 : v2;}
    template< class Tobj > inline Tobj sMin( Tobj v1, Tobj v2) {return v1<v2 ? v1 : v2;}
    template< class Tobj > inline Tobj sClamp( Tobj v, Tobj range_min, Tobj range_max ) {return range_min>range_max ? sClamp(v, range_max, range_min) : v<range_min ? range_min : v>range_max ? range_max : v; }
    template< class Tobj > inline Tobj sClamp( Tobj v, Tobj range_min, Tobj range_max, Tobj default_v ) {return range_min>range_max ? sClamp(v, range_max, range_min, default_v) : v<range_min ? default_v : v>range_max ? default_v : v; }
    template< class Tobj > inline Tobj sAbs( Tobj v) {return v<0 ? -v : v ;}
    template< class Tobj > inline Tobj sSign( Tobj v) {return v<0 ? -1 : 1 ;}
    template< class Tobj > inline Tobj sSig0( Tobj v) {return v==0 ? 0 : sSign(v) ;}
    template< class Tobj > inline Tobj sSqr( Tobj v) {return v==0 ? v : v*v ;}
    #define sOverlap(a1, a2, b1, b2) ( (a1 <= b1)? ((b1 <= a2)?1:0) : ((a1 <= b2)?1:0) )
    #define sSwapI(vrn_vn1,vrn_vn2)    {idx itmp=vrn_vn1; vrn_vn1=vrn_vn2; vrn_vn2=itmp; }
    template< class Tobj > inline void sSwap( Tobj & v1, Tobj & v2 ) { Tobj tmp = v1; v1 = v2; v2 = tmp; }

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
    inline idx sSwapHiLo(idx num) {
        return (
                ((num&0xFFFFFFFF)<<32) | ((num>>32)&0xFFFFFFFF)
            )  ;
    }
    #define sToStr(_v_var)         ( #_v_var)
    #define __ "\0\0"
    #undef _
    #define _ "\0"



    typedef void * (* sCallbackUniversal)(...);
    typedef bool (* sCallbackLogical)(void * param,...);



    #define sConvInt2Ptr( _v_int, _v_typ )      ((_v_typ * )(((char *)0)+(_v_int)))
    #define sConvPtr2Int( _v_ptr)               ((idx)(((const char*)(_v_ptr))-((char *)0)))
    #define sConvPtr( _v_ptr, _v_typ)           sConvInt2Ptr( (sConvPtr2Int( (_v_ptr))) , _v_typ )
    template< class Tobj > inline Tobj *         sNonConst(const Tobj * p) {return (((Tobj *)0)+ (p-((const Tobj*)0)));}


    #define sCallVarg(v_fun,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_fun)((v_frmt),ap);va_end(ap);}
    #define sCallVargPara(v_fun,v_para,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_fun)((v_para),(v_frmt),ap);va_end(ap);}
    #define sCallVargPara2(v_fun,v_para1,v_para2,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_fun)((v_para1),(v_para2),(v_frmt),ap);va_end(ap);}
    #define sCallVargRes(v_res,v_fun,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_res)=(v_fun)((v_frmt),ap);va_end(ap);}
    #define sCallVargResPara(v_res,v_fun,v_para,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_res)=(v_fun)((v_para),(v_frmt),ap);va_end(ap);}
    #define sCallVargResPara2(v_res,v_fun,v_para1,v_para2,v_frmt) {va_list ap;va_start(ap,v_frmt);(v_res)=(v_fun)((v_para1),(v_para2),(v_frmt),ap);va_end(ap);}

    inline idx sFlag(idx num) { return (((idx)1)<<num); }
    inline void sBitON(idx &flag, idx num) { flag |= (((idx)1)<<num); }
    inline void sBitOFF(idx &flag, idx num) { flag &= ~((((idx)1)<<num)); }
    inline void sBit(idx &flag, idx num, bool mode) { if(mode) sBitON(flag,num); else sBitOFF(flag,num);}
    inline idx isFlag(idx var,idx num ) { return (var&sFlag(num)); }



    inline idx sLen(const void * ptr) {
        return ptr ? (idx ) strlen((const char *)ptr) : 0;
    }
    template< class Tobj > inline Tobj * sShift(Tobj * ptr, idx shift) {
       return (Tobj *)(((char * )ptr)+shift);
    }

    template< class Tobj > inline Tobj * sDel(Tobj * ptr) {
        if ( ptr) free ((void *)ptr);
        return 0;
    }
    inline void * sNew(idx size, void * ptr=0)  {
        return size ? realloc(ptr,size) : sDel(ptr)  ;
    }

    inline char * sDup(const char * ptr, idx size=0) {
        if(size==0) size=sLen(ptr)+1;
        else if(size<0) size=sLen(ptr)+1-size;
        void * dst=sNew(size);
        if(dst)memmove(dst,ptr,size);
        return (char *)dst;
    }

    template< class Tobj > inline Tobj * sDup ( const Tobj * ptr, idx size=0)  {
        if(size==0) size=sizeof(Tobj) ;
        else if(size<0) size=sizeof(Tobj)-size;
        void * dst=sNew(size);
        if(dst)memmove(dst,ptr,size);
        return (Tobj*)dst;
    }

    template< class Tobj > inline Tobj * sSet( Tobj * ptr, idx val=0,idx size=0)  {
        if(size==0) size=sizeof(Tobj) ;
        else if(size<0) size=sizeof(Tobj)-size;
        memset((void *)ptr,(int)val,size);
        return ptr;
    }

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
        idx sign=1, i=0; \
        if( ((const char*)(_v_src))[0]=='-') {sign=-1;++i;} \
        for ( ; i< (_v_len) && (isdigit(   (ch=(((const char*)(_v_src))[i]))    ) ) ; ++i ) { \
            (_v_r)=(_v_r)*(_v_base)+(ch-'0'); \
        } \
        if(sign==-1)(_v_r)=-(_v_r); \
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

    inline idx sAlign(idx pos,idx align) {
        return  (  align>1 && (pos&(align-1)) ) ?  ( pos + align ) & ( ( align-1 )^sNotIdx ) : pos;
    }
    #define sRand64() (((((idx64)rand())&0xFFFF)<<48) | ((((idx64)rand())&0xFFFF)<<32) | ((((idx64)rand())&0xFFFF)<<16) | ((((idx64)rand())&0xFFFF)<<0))



    #define sNew_op(_v_p, _v_cnt)  for(idx jj=0;  jj<(_v_cnt);  ++jj) { new( ((_v_p)+jj) ) Tobj;}
    #define sDel_op(_v_p, _v_cnt)  for(idx jj=0;  jj<(_v_cnt);  ++jj) {  ((_v_p)+jj)->~Tobj();}

    inline bool sIs(const char * cmd,  const char * what ){
        return (cmd && strncasecmp(what,cmd,sLen(cmd) )==0) ? true : false;
    }
    inline bool sIsExactly(const char * cmd,  const char * what ){
        idx len_cmd = sLen(cmd);
        idx len_what = sLen(what);
        return (cmd && len_cmd == len_what && strncasecmp(what,cmd,len_cmd)==0) ? true : false;
    }


    #define _sOut       printf
    #define _sOutDw     _sOut( "{\n")
    #define _sOutUp     _sOut( "}")

    inline const char * sType( const int &)                { return "int"; }
    inline const char * sType( const bool &)            { return "bool"; }
    inline const char * sType( const unsigned int &)    { return "unsigned int"; }
    inline const char * sType( const double &)            { return "double"; }
    inline const char * sType( const float &)            { return "float"; }
    inline const char * sType( const char &)            { return "char"; }
    inline const char * sType( const char *)            { return "char *"; }
    inline const char * sType( const void *)            { return "void *"; }
    inline const char * sType( sCallbackUniversal &)    { return "function *"; }

    #define sOut(_v_var)     {_sOut("%s " #_v_var "=",sType(_v_var) ); sOutf(_v_var); _sOut(";\n"); }

    #define sOutClass(_v_cls) inline const char *  sType( const _v_cls & ) { return #_v_cls ; } inline const char *  sType( const _v_cls  * ) { return #_v_cls " *" ; } inline void sOutf( _v_cls & obj) { obj._sOutf();}

}







#endif 


