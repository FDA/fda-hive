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
    //! Generalized extensible memory container
    /*! sMex has multiple memory allocation strategies optimized for
    different usage scenarios. In a lazy mode it prefers
    over-allocation to minimize number of alloc/realloc interaction
    with OS. In active mode it can be used for more
    accurate and less wasteful usage for smaller set of containerized
    objects. This container can be used in a memory mapping
    mode where all memory operations are synchronized to disk
    mount and thus it can serve also as a shared memory container
    to organize interactions between different processed on a same or
    different compute units over networks. All other containers in the
    system can use this particular extensible memory container
    to implement their custom functionalities. */
    #define sMex_FileHandleShift    (40)
    class sMex
    {

    private:

        idx _curSize; //!< the current size of allocated memory
        idx _curPos; //!< the current size of the used part of allocted memory
        void * _buf; //!< the pointer to the memory chunk: this can be freed externally by vmemDel
//        char Fl[1024];

    public:
        static const idx _aligner[]; //!< list of alignments in bytes, indexed by bits 0-1 of flags
        static const idx _extnder[]; //!< list of growth sizes in bytes, indexed by bits 2-3 of flags
        static const idx _zero;
        static const idx _defaultFlags;
        static idx _totMexSize;
        static const char * _hexCode;

        //! allocate memory or map a file
        /*! \param size bytes to allocate or map; if a file is not mapped readonly and \a offset and \a sizemap are not set, the file will be truncated to this value
            \param ptr previous allocation (will be realloc-ed or unmapped)
            \param offset start of map in file
            \param sizemap non-zero if this is a partial map
            \returns pointer to new allocation or map, or 0 on allocation/map failure */
        void * alloc(idx size, void *ptr, idx offset=0, idx sizemap=0);
        //! reallocate memory or remap a file, possibly using a new set of flags
        /*! \param setflags bitwise-or of sMex::bitFlags to set
            \param unsetflags bitwise-or of sMex::bitFlags to unset
            \param onlyIfFlagsChanged remap only if the new set of flags is different
            \returns pointer to new allocation or map, or 0 if either the allocation/map failed,
                     or if there had not been any allocation yet
            \warning unsetting sMex::fReadonly on a file map is not allowed due to operating system limitations
            \warning can return 0 if the operation succeeded! (This can happen if there was no underlying
                     buffer allocated before remap() was called.) */
        void * remap( idx setflags=0, idx unsetflags=0, bool onlyIfFlagsChanged=false , idx offset=0, idx sizemap=0);
        static idx newSize; //!< last failed allocation size if not 0

    public:
        //! flags determining the behavior of the class
        /*! layout: bits 0-1 control allocation alignment;
            bits 2-3 control allocation growth rate;
            bit 4 is unused;
            bits 5-7 control allocation behavior;
            bits 8-14 are other assorted flags;
            bit 15 is unused;
            bits 16-31 store filehandles */
        idx flags;  // flags

        enum bitFlags{
            fAlignInteger=1, //!< align to sizeof(idx)
            fAlignParagraph=2, //!< align to 16 bytes
            fAlignPage=3, //!< align to page size

            // next 2 bits determine the size of the increment block:
            fBlockNormal=0<<2, //!< grow by 1KB
            fBlockCompact=1<<2, //!< grow by chunks of 128 bytes
            fBlockPage=2<<2, //!< grow by page size
            fBlockDoubling=3<<2, //!< grow by doubling the existing size

            fSetZero=1<<5, //!< fill allocated memory with zeros
            fNoRealloc=1<<6, //!< single allocation, no extensions allowed
            fExactSize=1<<7, //!< do not allocate more memory than required : used for files
            fReadonly=1<<8, //!< this is to make the container readonly
            fZeroTerminateFile=1<<10, //!< this is to make the container zero terminated \warning not yet implemented
            fPartialMap=1<<11, //!< map only start or middle part of a file, not all the way to end
            fMaintainAlignment=12, //!< maintain alignment inside a file when destroying the object
            fDirectFileStream=1<<13, //!< memory does not accumulate through map, directly writes to a file
            //fLockedForChange=14, //!< container can only be read but not modified
            fMapRemoveFile=1<<15, //!< if file exists, remove it before map creation
            fMapPreloadPages=1<<16, //!< whole map should be pre-loaded immediately
            fCreatExcl=1<<17, //!< file should be created, but not opened if it already exists (i.e. O_CREAT|O_EXCL in POSIX)
            fMapMemoryLazyFile=1<<18, //!< while working memory is in RAM only when destroying it writes to disk
            fLast=1<<19,


            fForceRemapTruncate=((idx)1<<32) // this option forces truncation during remaps even for containers to be readonly
            // next sixteen bits are for filehandles
        };

        //! Signature for uri_callback
        /*! \param[out] buf_out buffer to hold printed disk path
            \param buf_out_len size of buf_out, generally expected to be PATH_MAX
            \param uri_or_path either a URI to resolve into a path, or an already resolved disk file path
            \param only_existing if true, return 0 for a URI which does not resolve to an already existing path on disk;
                                 if false, but the uri is valid, the sStorage implementation of the callback creates
                                 parent directories for the path, allowing a new file to be immediately created.
            \return uri_or_path if it already looks like a disk path; or printed path in buf_out; or 0 on failure */
        typedef const char * (*UriCallback)(char * buf_out, idx buf_out_len, const char * uri_or_path, bool only_existing);
        //! Optional URI resolver callback, for resolving custom storage URIs to file paths on disk; not used if 0
        /*! Implemented using sStorage::makePath() if slib is built with sStorage support */
        static UriCallback uri_callback;

    public: // construction/destruction
        sMex ( idx flg=_defaultFlags) {init(flg);}
        ~sMex (){destroy();}
        /*! \warning call destroy() first to avoid leaking this sMex's buffer or filehandle */
        sMex * init(idx flg=_defaultFlags) {_buf=0;_curPos=_curSize=0;flags=flg;return this;}
        /*! \warning call destroy() first to avoid leaking this sMex's buffer or filehandle */
        sMex * init(const char * flnm, idx flg=0, idx offset=0, idx sizemap=0);
        //! free allocated buffer or close mapped filehandle
        void destroy(void);

    public: // information
            //! returns a pointer, doesn't check for boundaries
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

        //! returns a pointer, makes sure the buffer is big enough
        inline void * ptrx(idx pos=0 ) { resize(pos+1); return ptr( pos ) ; }
        //! returns the current used memory size
        inline idx pos(void) const { return _curPos; }
        //! returns the current allocated memory size (some of it may be unused)
        inline idx total(void) const { return _curSize; }
        //! tells if the container has a buffer allocated or a file opened
        bool ok (void) const { return _buf ? true : ((flags>>(sMex_FileHandleShift)) & 0xFFFF ? true : false); }
        //! timestamp of mapped file, or #sIdxMax if this is not a file
        idx mtime (void) const;
        //! set allocation strategy
        inline void flagBlock(idx flag ) { flags=(flags&(~((idx)0x03)<<2))|flag; }

    public: // operations

        // void * manipulations

        //! replaces \a sizeDel bytes of internal buffer starting from \a pozReplace by  \a sizeAdd bytes from \a add
        /*! \param pozReplace where to start replacing/allocating/deleting
            \param add source to copy from; if 0, memory is allocated but nothing is copied into it
            \param sizeAdd length to allocate and (if \a add is not zero) to copy from \a add
            \param sizeDel length of internal buffer following \a pozReplace which will be deleted before data from \a add is inserted
            \returns \a pozReplace, or sNotIdx on failure */
        idx replace(idx pozReplace,const void * add,idx sizeAdd,idx sizeDel);
        //! append starting at the current end of used memory
        /*! \returns pos() */
        inline idx add(const void * addb, idx sizeAdd)
        {
            if(flags&fDirectFileStream) {
                idx hFile=(flags>>(sMex_FileHandleShift)) & 0xFFFF; // this is where file handles are kept; no file handles means in memory operations
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
            if(sizeAdd && (!(flags&(sMex::fReadonly|sMex::fAlignParagraph|sMex::fAlignInteger|sMex::fAlignPage))) &&_curPos+sizeAdd < _curSize) {
                if(addb) {
                    memmove( ((char *)_buf)+_curPos, addb , sizeAdd ); // copy to the space allocated for it
                } else if(flags&(sMex::fSetZero)) {
                    memset( ((char *)_buf)+_curPos, 0, sizeAdd );
                }
                idx pos=_curPos;
                _curPos+=sizeAdd;
                return pos;
            }
            return replace(_curPos , addb, sizeAdd , 0);
        }


        //! insert at a specified position
        /*! \param pos where to insert
            \param addb source to copy from; if 0, nothing is copied into newly allocated memory
            \param sizeAdd length to allocate and (if \a addb is not 0) to copy from \a addb
            \returns \a pos */
        inline idx insert( idx pos,const void * addb, idx sizeAdd)  { return replace( pos , addb , sizeAdd,0); }
        //! delete a piece from the buffer
        /*! \returns \a pos */
        inline idx del(idx pos, idx sizeDel) { return replace( pos , 0 , 0 , sizeDel) ; }
        //! expand the buffer if necessary to fit so many bytes total without moving pos()
        /*! \returns pos() */
        inline idx resize(idx sizeRequired)  { return (sizeRequired<_curPos) ? _curPos : add( 0, sizeRequired-_curPos);  }
        //! empty the internal memory buffer
        inline void empty( void ) { del(0,_curPos); }
        //! set the used size of memory buffer without changing the buffer itself
        /*! \param pos if >= 0, set buffer used size to this value; if < 0, decrement buffer used size by this value
            \note will keep buffer used size aligned
            \returns new buffer used size */
        inline idx cut( idx pos) {if(pos<0)pos=_curPos+pos; return (_curPos=sAlign( pos, _aligner[flags&0x03] )) ;}
        //! set the used part of the buffer to a certain value
        inline void set(idx val=0) { memset(_buf,(int)val,_curPos);  } // set to certain value

        //! append arbitrary object at the current end of used memory
        template <class Tobj> Tobj * add(Tobj & obj) { return  ptr(  (Tobj *)add((const char*)&obj, sizeof(obj)) ) ;}
        //! append 0-terminated string, including terminal 0, at the current end of used memory
        inline idx add(const char * addb)  { return add(addb, sLen(addb)+1);  }

        //! position and length of an allocation
        struct Pos {
            idx pos, size;
            Pos() {pos=0; size=0;}
        };

    public:
        //! take ownership of another sMex's buffer or file
        /*! \warning call destroy() first to avoid leaking this sMex's buffer or filehandle */
        inline sMex * borrow(sMex * from){
            memcpy(this,from,sizeof(sMex));
            memset(from,0,sizeof(sMex));
            return this;
        }
        //! swap buffer or file with another sMex
        inline sMex * swap(sMex * from){
            sMex tmp;
            memcpy(&tmp,from,sizeof(sMex));
            memcpy(from,this,sizeof(sMex));
            memcpy(this,&tmp,sizeof(sMex));
            memset(&tmp,0,sizeof(sMex));
            return this;
        }
        //! release the buffer of this container
        inline void * release(idx * pSize=0){
            void * r=_buf;
            if(pSize)*pSize=_curPos;
            memset(this,0,sizeof(sMex));
            return r;
        }

    public:
        //! append content of file stream at the current end of used memory
        /*! \param fp FILE pointer from which IO is streamed
            \param endCharList list of characters preventing to continue the streaming process
            \returns the number of the bytes appended */
        idx readIO(FILE * fp, const char * endCharList=0);
        idx readIO(int fh, const char * endCharList=0);
        /*
    public:
        inline void _sOutf( void ) {
            _sOutDw;
                sOut(_curSize);
                sOut(_curPos);
                sOut(_buf);
                sOut(flags);
            _sOutUp;
        }*/
    };
//    sOutClass(sMex)

    //#define SLIB_DECLARE_ARRAY_FUNCTIONS
    template <class Tobj, class Tarr> class sArr {
        public:
            virtual ~sArr(){};
            //sArr(const sArr & o){copy(o);};
        private:
            /* Child classes must implement:
            friend class sArr<Tobj,foo>;
            Tobj * _add(idx cntAdd);
            void _del(idx posDel,idx cntDel);
            Tobj * ptr(idx index=0);
            const Tobj * ptr( idx index=0) const;
            idx dim(void) const=0;
            sMex * mex(void);
            */
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

            //void snew_op(idx pos, idx ccnt, idx * dummy=0) {}
            void sshift_op( idx posSrc, idx posDst, idx ) //cntAll
            {
                if (posDst<posSrc) {
                    //for ( idx i=0 ; i != cntAll && posSrc+i<dim(); ++i )
                    for ( idx i=0 ; posSrc+i<static_cast<const Tarr*>(this)->dim(); ++i )
                        memmove( (void*) static_cast<Tarr*>(this)->ptr(posDst+i), (void*) static_cast<Tarr*>(this)->ptr(posSrc+i), sizeof(Tobj));
                }else {
                    //for ( idx i=cntAll-1 ; i >=0 ; ++i )
                    for ( idx i=static_cast<const Tarr*>(this)->dim()-1-(posDst-posSrc) ; i>=posSrc; --i )
                        memmove( (void*) static_cast<Tarr*>(this)->ptr(posDst-posSrc+i), (void*) static_cast<Tarr*>(this)->ptr(i), sizeof(Tobj));
                }
            }

        public:

        typedef Tobj value_type;

        void flagOn(idx fl) { static_cast<Tarr*>(this)->mex()->flags|=fl; }
        void empty(void) { del(0, static_cast<const Tarr*>(this)->dim()); }
        void remap(idx setflags=0, idx unsetflags=0, bool onlyIfFlagsChanged=false) { static_cast<Tarr*>(this)->mex()->remap(setflags, unsetflags, onlyIfFlagsChanged); }
        //void cut(idx pos=0) { del(pos, static_cast<const Tarr*>(this)->dim()-pos); }
        void total(void) const { static_cast<const Tarr*>(this)->mex()->total()/sizeof(Tobj); }
        Tobj & operator []( idx index) { return *static_cast<Tarr*>(this)->ptr(index); }
        const Tobj & operator [](idx index) const { return *static_cast<const Tarr*>(this)->ptr(index); }
        Tobj * operator ()( idx index) { return static_cast<Tarr*>(this)->ptrx(index); }
        Tobj * ptrx(idx index=0) { resize(index+1); return static_cast<Tarr*>(this)->ptr(index); }

        Tobj * addva( idx cntAdd, va_list marker )
        {
            // assert(cntAdd >= 0); // removed by Vahan because it is impossible to debug anything with Big Data because of this stupid checks .
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
            if(!doappend)empty();
            for ( idx i=0; i< ((Tother * )o)->dim() ; ++i) {
                vadd(1,(*(Tother*)o)[i]);
            }
            return this;
//            return copy ( (*(Tother*)o)(0) , ((Tother * )o)->dim(), doappend);
        }

        sArr< Tobj, Tarr > * copy ( const Tobj * o , idx dim, bool doappend=false)
        {
            if(!doappend)empty();
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

#endif // sLib_core_mex_hpp




