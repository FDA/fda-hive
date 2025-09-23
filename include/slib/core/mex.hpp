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
#ifndef sLib_core_mex_hpp
#define sLib_core_mex_hpp

#include <slib/core/def.hpp>

namespace slib
{
    #define sMex_FileHandleShift    (40)
    class sMex
    {

    private:

        idx _curSize;
        idx _curPos;
        void * _buf;

    public:
        static const idx _aligner[];
        static const idx _extnder[];
        static const idx _zero;
        static const idx _defaultFlags;
        static idx _totMexSize;
        static const char * _hexCode;

        void * alloc(idx size, void *ptr, idx offset=0, idx sizemap=0);
        void * remap( idx setflags=0, idx unsetflags=0, bool onlyIfFlagsChanged=false , idx offset=0, idx sizemap=0);
        static idx newSize;

    public:
        idx flags;

        enum bitFlags{
            fAlignInteger=1,
            fAlignParagraph=2,
            fAlignPage=3,

            fBlockNormal=0<<2,
            fBlockCompact=1<<2,
            fBlockPage=2<<2,
            fBlockDoubling=3<<2,

            fSetZero=1<<5,
            fNoRealloc=1<<6,
            fExactSize=1<<7,
            fReadonly=1<<8,
            fZeroTerminateFile=1<<10,
            fPartialMap=1<<11,
            fMaintainAlignment=12,
            fDirectFileStream=1<<13,
            fMapRemoveFile=1<<15,
            fMapPreloadPages=1<<16,
            fCreatExcl=1<<17,
            fMapMemoryLazyFile=1<<18,
            fLast=1<<19,


            fForceRemapTruncate=((idx)1<<32)
        };

        typedef const char * (*UriCallback)(char * buf_out, idx buf_out_len, const char * uri_or_path, bool only_existing);
        static UriCallback uri_callback;

    public:
        sMex ( idx flg=_defaultFlags) {init(flg);}
        ~sMex (){destroy();}
        sMex * init(idx flg=_defaultFlags) {_buf=0;_curPos=_curSize=0;flags=flg;return this;}
        sMex * init(const char * flnm, idx flg=0, idx offset=0, idx sizemap=0);
        void destroy(void);

    public:
            inline void * ptr(idx pos = 0)
            {
                return (void *) (((char *) _buf) + pos);
            }
            inline const void * ptr(idx pos = 0) const
            {
                return (void *) (((char *) _buf) + pos);
            }
            template<class Tobj> Tobj * ptr(idx pos = 0, Tobj ** toptr = 0)
            {
                Tobj * p = (Tobj *) ptr(pos);
                if( toptr ) {
                    *toptr = p;
                }
                return p;
            }
            template<class Tobj> const Tobj * ptr(idx pos = 0, Tobj ** toptr = 0) const
            {
                Tobj * p = (Tobj *) ptr(pos);
                if( toptr ) {
                    *toptr = p;
                }
                return p;
            }

        inline void * ptrx(idx pos=0 ) { resize(pos+1); return ptr( pos ) ; }
        inline idx pos(void) const { return _curPos; }
        inline idx total(void) const { return _curSize; }
        bool ok (void) const { return _buf ? true : ((flags>>(sMex_FileHandleShift)) & 0xFFFF ? true : false); }
        idx mtime (void) const;
        inline void flagBlock(idx flag ) { flags=(flags&(~((idx)0x03)<<2))|flag; }

    public:


        idx replace(idx pozReplace,const void * add,idx sizeAdd,idx sizeDel);
        inline idx add(const void * addb, idx sizeAdd)
        {
            if(flags&fDirectFileStream) {
                idx hFile=(flags>>(sMex_FileHandleShift)) & 0xFFFF;
                if( hFile != 0 ) {
                    idx written = 0, remaining = sizeAdd;
                    do {
                        written = write(hFile, (const char *)addb + written, remaining);
                        if( written > 0 ) {
                            remaining -= written;
                            _curPos += written;
                        }
                    } while( written > 0 && remaining > 0 );
                }
                return 0;
            }
            if(sizeAdd > 0 && (!(flags&(sMex::fReadonly|sMex::fAlignParagraph|sMex::fAlignInteger|sMex::fAlignPage))) && _curPos+sizeAdd < _curSize) {
                if(addb != nullptr) {
                    memmove( ((char *)_buf)+_curPos, addb , sizeAdd );
                } else if(flags&(sMex::fSetZero)) {
                    memset( ((char *)_buf)+_curPos, 0, sizeAdd );
                }
                idx pos=_curPos;
                _curPos+=sizeAdd;
                return pos;
            }
            return replace(_curPos , addb, sizeAdd , 0);
        }


        inline idx insert( idx pos,const void * addb, idx sizeAdd)  { return replace( pos , addb , sizeAdd,0); }
        inline idx del(idx pos, idx sizeDel) { return replace( pos , 0 , 0 , sizeDel) ; }
        inline idx resize(idx sizeRequired)  { return (sizeRequired<_curPos) ? _curPos : add( 0, sizeRequired-_curPos);  }
        inline void empty( void ) { del(0,_curPos); }
        inline idx cut( idx pos) {if(pos<0)pos=_curPos+pos; return (_curPos=sAlign( pos, _aligner[flags&0x03] )) ;}
        inline void set(idx val=0) { memset(_buf,(int)val,_curPos);  }

        template <class Tobj> Tobj * add(Tobj & obj) { return  ptr(  (Tobj *)add((const char*)&obj, sizeof(obj)) ) ;}
        inline idx add(const char * addb)  { return add(addb, sLen(addb)+1);  }

        struct Pos {
            idx pos, size;
            Pos() {pos=0; size=0;}
        };

    public:
        inline sMex * borrow(sMex * from){
            flags = from->flags;
            _curPos = from->_curPos;
            _curSize = from->_curSize;
            _buf = from->_buf;
            from->flags = 0;
            from->_curPos = 0;
            from->_curSize = 0;
            from->_buf = 0;
            return this;
        }
        inline sMex * swap(sMex * from){
            sMex tmp;
            tmp.flags = from->flags;
            tmp._curPos = from->_curPos;
            tmp._curSize = from->_curSize;
            tmp._buf = from->_buf;
            from->flags = flags;
            from->_curPos = _curPos;
            from->_curSize = _curSize;
            from->_buf = _buf;
            flags = tmp.flags;
            _curPos = tmp._curPos;
            _curSize = tmp._curSize;
            _buf = tmp._buf;
            tmp.flags = 0;
            tmp._curPos = 0;
            tmp._curSize = 0;
            tmp._buf = 0;
            return this;
        }
        inline void * release(idx * pSize=0){
            void * r=_buf;
            if(pSize)*pSize=_curPos;
            flags = 0;
            _curPos = 0;
            _curSize = 0;
            _buf = 0;
            return r;
        }

    public:
        idx readIO(FILE * fp, const char * endCharList=0);
        idx readIO(int fh, const char * endCharList=0);
    };

    template <class Tobj, class Tarr> class sArr {
        public:
            virtual ~sArr(){};
        private:
            Tobj * _insert(idx pos, idx cntAdd) { Tobj * p=add( cntAdd ); sshift_op( pos, pos+cntAdd, cntAdd ); return p;}

        protected:

            template< class Tdummy > void snew_op(idx pos, idx ccnt, Tdummy * dummy)
            {
                for(idx i=0; i<ccnt;++i) {
                    new( static_cast<Tarr*>(this)->ptr(pos+i) ) Tobj;
                }
            }
            template< class Tdummy > void sdel_op(idx pos, idx ccnt, Tdummy * dummy)
            {
                for(idx i=0; i<ccnt;++i) {
                    static_cast<Tarr*>(this)->ptr(pos+i)->~Tobj();
                }
            }

            void sdel_op(idx pos, idx ccnt, idx * dummy) {}
            void sdel_op(idx pos, idx ccnt, udx * dummy) {}
            void sdel_op(idx pos, idx ccnt, real * dummy) {}
            void sdel_op(idx pos, idx ccnt, char * dummy) {}

            void snew_op(idx pos, idx ccnt, idx * dummy) {}
            void snew_op(idx pos, idx ccnt, udx * dummy) {}
            void snew_op(idx pos, idx ccnt, real * dummy) {}
            void snew_op(idx pos, idx ccnt, char * dummy) {}

            void sshift_op( idx posSrc, idx posDst, idx )
            {
                if (posDst<posSrc) {
                    for ( idx i=0 ; posSrc+i<static_cast<const Tarr*>(this)->dim(); ++i )
                        memmove( (void*) static_cast<Tarr*>(this)->ptr(posDst+i), (void*) static_cast<Tarr*>(this)->ptr(posSrc+i), sizeof(Tobj));
                }else {
                    for ( idx i=static_cast<const Tarr*>(this)->dim()-1-(posDst-posSrc) ; i>=posSrc; --i )
                        memmove( (void*) static_cast<Tarr*>(this)->ptr(posDst-posSrc+i), (void*) static_cast<Tarr*>(this)->ptr(i), sizeof(Tobj));
                }
            }

        public:

        typedef Tobj value_type;

        void flagOn(idx fl) { static_cast<Tarr*>(this)->mex()->flags|=fl; }
        void empty(void) { del(0, static_cast<const Tarr*>(this)->dim()); }
        void remap(idx setflags=0, idx unsetflags=0, bool onlyIfFlagsChanged=false) { static_cast<Tarr*>(this)->mex()->remap(setflags, unsetflags, onlyIfFlagsChanged); }
        void total(void) const { static_cast<const Tarr*>(this)->mex()->total()/sizeof(Tobj); }
        Tobj & operator []( idx index) { return *static_cast<Tarr*>(this)->ptr(index); }
        const Tobj & operator [](idx index) const { return *static_cast<const Tarr*>(this)->ptr(index); }
        Tobj * operator ()( idx index) { return static_cast<Tarr*>(this)->ptrx(index); }
        Tobj * ptrx(idx index=0) { resize(index+1); return static_cast<Tarr*>(this)->ptr(index); }

        Tobj * addva( idx cntAdd, va_list marker )
        {
            idx c=static_cast<Tarr*>(this)->dim();
            static_cast<Tarr*>(this)->_add(cntAdd);
            for(idx jj=0;  jj<(idx)cntAdd;  ++jj) {
                *((Tobj *)static_cast<Tarr*>(this)->ptr(c+jj))=va_arg(marker, Tobj );
            }
            return static_cast<Tarr*>(this)->ptr(c);
        }
        Tobj * addM( idx cntAdd=1)
        {
            idx c=static_cast<Tarr*>(this)->dim();
            static_cast<Tarr*>(this)->_add(cntAdd);
            return static_cast<Tarr*>(this)->ptr(c);
        }
        Tobj * add( idx cntAdd=1)
        {

            idx c=static_cast<Tarr*>(this)->dim();
            static_cast<Tarr*>(this)->_add(cntAdd);
            snew_op(c,(idx)cntAdd,(Tobj*)0);
            return static_cast<Tarr*>(this)->ptr(c);
        }
        Tobj * vadd( idx cntAdd, ...)
        {
            Tobj * res;
            sCallVargRes(res,addva,cntAdd);
            return res;
        }
        Tobj * resize(idx sizeRequired)
        {
            idx d = static_cast<Tarr*>(this)->dim();
            if( sizeRequired>d )
                add(sizeRequired-d);
            return static_cast<Tarr*>(this)->ptr(sizeRequired);
        }
        Tobj * resizeM(idx sizeRequired)
        {
            idx d = static_cast<Tarr*>(this)->dim();
            if( sizeRequired>d )
                addM( sizeRequired-d);
            return static_cast<Tarr*>(this)->ptr(sizeRequired);
        }
        void del( idx posDel, idx cntDel=1)
        {
            if( !static_cast<Tarr*>(this)->dim() ) return;
            sdel_op(posDel,cntDel, (Tobj *)0);
            static_cast<Tarr*>(this)->_del(posDel,cntDel);
        }
        void cut( idx cutPos=0 )
        {
            if( !static_cast<Tarr*>(this)->dim() ) return;
            if(cutPos<0)cutPos=static_cast<Tarr*>(this)->dim()+cutPos;
            sdel_op(cutPos,(static_cast<Tarr*>(this)->dim()-cutPos ), (Tobj *)0);
            static_cast<Tarr*>(this)->mex()->cut(sizeof(Tobj)*cutPos);
        }
        Tobj * insert(idx pos, idx cntAdd)
        {
            _insert( pos, cntAdd );
            snew_op(pos,cntAdd,(Tobj*)0);
            return static_cast<Tarr*>(this)->ptr(pos);
        }

        template < class Tother > sArr< Tobj, Tarr > * copy ( const Tother * o , bool doappend=false)
        {
            if(!doappend)cut();
            for ( idx i=0; i< ((Tother * )o)->dim() ; ++i) {
                vadd(1,(*(Tother*)o)[i]);
            }
            return this;
        }

        sArr< Tobj, Tarr > * copy ( const Tobj * o , idx dim, bool doappend=false)
        {
            if(!doappend)cut();
            for ( idx i=0 ; i< dim ; ++i) {
                vadd(1,((Tobj*)o)[i]);
            }
            return this;
        }

        template < class Tother > Tobj * glue ( const Tother * o )
        {
            idx ps = static_cast<Tarr*>(this)->dim();

            for ( idx i=0; i< ((Tother * )o)->dim() ; ++i) {
                vadd(1,(*(Tother*)o)[i]);
            }
            return static_cast<Tarr*>(this)->ptr(ps);
        }

        template < class Tother >  sArr< Tobj, Tarr > * borrow( Tother * o )
        {
            static_cast<Tarr*>(this)->mex()->borrow(o->mex());
            return this;
        }
        idx imax( void ) const
        {
            idx imax=0;
            idx d = static_cast<const Tarr*>(this)->dim();
            for(idx i=1; i<d; ++i) {
                if( *static_cast<const Tarr*>(this)->ptr(imax) < *static_cast<const Tarr*>(this)->ptr(i) ) imax=i;
            }
            return imax;
        }
        idx imin( void ) const
        {
            idx imin=0;
            idx d = static_cast<const Tarr*>(this)->dim();
            for(idx i=1; i<d; ++i) {
                if( *static_cast<const Tarr*>(this)->ptr(imin) > *static_cast<const Tarr*>(this)->ptr(i) ) imin=i;
            }
            return imin;
        }

    };






}

#endif 