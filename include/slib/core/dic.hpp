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
#ifndef sLib_core_dic_h
#define sLib_core_dic_h

#include <slib/core/lst.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/str.hpp>

namespace slib
{
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ class sDic
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    //! Key-value storage, optimized for addition/lookup, ordered by integer index, maintains internal copies of keys
    /*! Basic usage:
    \code
    sDic<idx> d; // dictionary with idx values and arbitrary-type keys
    real r = 123.4;
    *d.setString("Hello") = 42; // store value 42 under string key "Hello"
    *d.setString("Hello", 4) = 43; // store value 43 under string key "Hell"
    *d.set(&r, sizeof(real)) = 0; // store value 0 under binary key 123.4
    idx * myval = d.get("Bye"); // pointer to value under key "Bye"
    for (idx i=0; i<d.dim(); i++) {
        const char * key = static_cast<const char *>(d.id(i)); // iterate over all keys - you must know the key type!
        idx * val = d.ptr(i); // iterate over all values
    }
    \endcode */
    template < class Tobj , int Tbits=3 > class sDic: public sArr< Tobj, sDic<Tobj, Tbits> >
    {

    private:
        typedef sArr< Tobj, sDic<Tobj, Tbits> > Tparent;
        friend class sArr< Tobj, sDic<Tobj, Tbits> >;
        sMex _mex;
        struct Hdr { sAlgo::lix lst0,hax0; Hdr(void){lst0=hax0=sAlgo::emptyLix();} };
        Hdr * _hdr(void) const {return (Hdr *)_mex.ptr(0); };

    public: // construction/destruction

        // the constructor
        sDic(const char * flnm = 0, idx flgs = 0 ) { _mex.flags|=sMex::fAlignInteger; init( flnm, flgs); }
        ~sDic () {
            //undict( 0, dim()); // delete <cntDel> items from dictionary starting from <posDel> position
            //del(0,dim());
            idx hFile=(_mex.flags>>(sMex_FileHandleShift)) & 0xFFFF;
            if(hFile>0){
                destroy();
            } else {
                empty();
            }
        }
        //{ sdel_op(0,dim()); }
        sDic * init (const char * flnm=0, idx flgs = sMex::_defaultFlags)
        {
            if(flnm) {
                _mex.init(flnm,flgs);
            }
            if( _mex.pos()<= (idx)sizeof(Hdr)) {
                _mex.empty();
                _mex.add(0, sizeof(Hdr)) ; // the very first intialization - we reserve a place fr the lix_hdr structures
                new (_hdr()) Hdr; // call the sObj constructor
            }
            return this;
        }
        //! Free current buffer, take ownership of another sDic's buffer, giv e that sDic an unallocated buffer instead
        /*! \param src sDic whose buffer to take
            \warning src will become unusable due to no allocated buffer; call src->init() to be able to use it again */
        sDic * borrow(sDic<Tobj, Tbits> * src)
        {
            if( _hdr() ) {
                Tparent::empty();
            }
            _mex.empty();
            _mex.borrow(&src->_mex);
            return this;
        }
    // sArr implementations
    private:
        Tobj * _add(idx cntAdd=1){
            Tobj * p=(Tobj *)sAlgo::lix_add(&_mex,cntAdd,sizeof(Tobj),&(_hdr()->lst0),Tbits);
            return p;
        }
        void _del( idx posDel, idx cntDel) {
            if(cntDel==dim() && posDel==0) {
                _mex.empty();init();
            }
            else {
                undict( posDel, cntDel); // un-dictionarize
                Tparent::sshift_op( posDel+cntDel, posDel, cntDel);
                sAlgo::lix_unlink(&_mex,cntDel,&(_hdr()->lst0));
            }
        }
        //Tobj * _insert(idx pos, idx cntAdd) { Tobj * p=sArr<Tobj>::add( cntAdd ); sArr<Tobj>::sshift_op( pos, pos+cntAdd, cntAdd ); return p;}
    public:
        //! Get value with specified index
        Tobj * ptr( idx index=0) { return (Tobj *)sAlgo::lix_ptr(&_mex,index,sizeof(Tobj),(_hdr()->lst0)); }
        //! Get value with specified index
        const Tobj * ptr(idx index=0) const { return (const Tobj *)sAlgo::lix_ptr(&_mex,index,sizeof(Tobj),(_hdr()->lst0)); }
        //! Get number of values
        idx dim(void) const { return sAlgo::lix_cnt(&_mex,(_hdr()->lst0));}

        // sDic implmentations
    public:
        idx dictCnt(void) const // set an item to the dictionary
        {
            return sAlgo::lix_cnt(&_mex,(_hdr()->hax0));
        }
        //! Add a binary key with a specified index
        /*! \param iFnd value index for key
            \param id key to add
            \param lenId length of \a id in bytes; if 0, defaults to sLen(id)
            \warning If \a lenId is 0, then \a lenId + 1 bytes of \a id
                will be coped into key storage. So if \a lenId is 0, make certain
                that \a id is a 0-terminated string!
            \returns offset in _mex where key is stored */
        idx dict( idx iFnd, const void * id, idx lenId=0)
        {
            idx stlen = lenId ? lenId : sLen((const char *)id) ;
            if(!lenId)lenId=stlen+1;

            idx idofs=_mex.add(0,lenId); // we allocate before getting the pointer since 'did' might be in the same buffer as idmex is
            memmove( _mex.ptr(idofs) , (const void *)id, lenId ); // copy the content of id to the just allocated spot
            sAlgo::hax_map(&_mex, iFnd, idofs, stlen, 0, &(_hdr()->hax0), Tbits, 4 , Tbits);

            return idofs;
        }
        //! Add a string key (which will be stored in 0-terminated form, even if \a id is not 0-terminated) with a specified value index
        /*! \param iFnd value index for key
            \param id string key to add
            \param lenId length of \a id not including terminal 0; if 0, defaults to sLen(id)
            \returns offset in _mex where key is stored */
        idx dictString(idx iFnd, const char * id, idx lenId=0)
        {
            if( !lenId )
                lenId = sLen(id);

            idx idofs = _mex.add(0, lenId + 1);
            memmove(_mex.ptr(idofs), id, lenId);
            *(char *)_mex.ptr(idofs + lenId) = 0;
            sAlgo::hax_map(&_mex, iFnd, idofs, lenId, 0, &(_hdr()->hax0), Tbits, 4 , Tbits);

            return idofs;
        }

        void undict( idx posDel, idx cntDel=1, bool forceRehash=false) // delete <cntDel> items from dictionary starting from <posDel> position
        {
            sAlgo::hax_unmap(&_mex, posDel,cntDel, &(_hdr()->hax0));
            if(forceRehash)
                rehash();
        }
        void rehash( void )
        {
            sAlgo::hax_rehash(&_mex, 0, (_hdr()->hax0));
        }
        /*
        void del( idx posDel, idx cntDel=1) // delete <cntDel> items from dictionary starting from <posDel> position
        {
            undict(posDel, cntDel);
            sArr<Tobj>::del(posDel,cntDel);
        }
        */
        //! Get dictionary's underlying storage buffer
        sMex * mex(){ return &_mex; }
        //! Get dictionary's underlying storage buffer
        const sMex * mex() const { return &_mex; }
        //! Clear all keys/values from dictionary
        void empty(void) {
            if( _hdr() ) {
                Tparent::empty();
            }
            _mex.empty();
            init();
        }
        void destroy(void) {
            _mex.destroy();
        }
    public:
        //! Add a binary key to the dictionary if it's not already there
        /*! \param id string key to add
            \param lenId length of \a id not including terminal 0; if 0, defaults to sLen(id)
            \param[out] pNum index for value of \a id
            \warning If \a lenId is 0, then \a lenId + 1 bytes of \a id
                will be coped into key storage. So if \a lenId is 0, make certain
                that \a id is a 0-terminated string!
            \returns pointer to value stored under \a id (whether or not the value
                was already stored or has been just added by this call) */
        Tobj * set( const void * id, idx lenId=0, idx * pNum=0)
        {
            idx iFnd=find(id, lenId ? lenId : sLen((const char *)id) , pNum); // search if this item is present

            if( !iFnd-- ){ // add this since it is not yet in
                iFnd=dim();
                Tparent::add(1);
                dict(iFnd, id, lenId);
            }
            if(pNum)
                *pNum=iFnd;

            return (Tobj * )ptr(iFnd);
        }

        //! Add a string key to the dictionary if it's not already there, and which will be stored in 0-terminated form (even if \a id is not 0-terminated)
        /*! \param id string key to add
            \param lenId length of \a id not including terminal 0; if 0, defaults to sLen(id)
            \param[out] pNum index for value of \a id
            \returns pointer to value stored under \a id (whether or not the value
                was already stored or has been just added by this call) */
        Tobj * setString(const char * id, idx lenId=0, idx * pNum=0)
        {
            if( !lenId )
                lenId = sLen(id);

            idx iFnd=find(id, lenId, pNum); // search if this item is present

            if( !iFnd-- ) { // add this since it is not yet in
                iFnd = dim();
                Tparent::add(1);
                dictString(iFnd, id, lenId);
            }
            if(pNum)
                *pNum=iFnd;

            return (Tobj * )ptr(iFnd);
        }

        //! Get pointer to value stored under specified key
        /*! \param id key
            \param lenId length of \a id; if 0, defaults to sLen(id)
            \param[out] pNum index for value of \a id
            \returns pointer to value stored under \a id, or 0 if not found */
        Tobj * get(const void * id, idx lenId=0, idx * pNum =0) // get an item from dictionary
        {
            idx iFnd=find(id, lenId, pNum);
            return (Tobj * )((iFnd--) ? ptr(iFnd) : 0 );
        }

        //! Get pointer to value stored under specified key
        /*! \param id key
            \param lenId length of \a id (if 0, defaults to sLen(id))
            \param[out] pNum index for value for \a id
            \returns pointer to value stored under \a id, or 0 if not found */
        const Tobj * get(const void * id, idx lenId=0, idx * pNum =0) const // get an item from dictionary
        {
            idx iFnd=find(id, lenId, pNum);
            return (const Tobj * )((iFnd--) ? ptr(iFnd) : 0 );
        }

        //! Search for value with specified key
        /*! \param id key
            \param lenId length of \a id; if 0, defaults to sLen(id)
            \param[out] pNum index for value for \a id
            \returns index + 1 on success, 0 if not found */
        idx find(const void * id, idx lenId=0, idx * pNum =0) const
        {
            if(!_hdr())return 0;
            idx iFnd=sAlgo::hax_find(&_mex,id, lenId ? lenId : sLen((const char *)id) , 0, 0, (_hdr()->hax0)); // search if this item is present
            if(pNum)
                *pNum=iFnd-1;
            return iFnd;//_hax.find(id,lenId ? lenId : sLen((const char * )id),pNum);
        }

        //void * ido(idx idofs) { return _mex.ptr(idofs);}

        //! Get key with specified index
        /*! \param index index of key
            \param[out] pLen length of key (not including terminal 0 if the key was
                stored using dictString() / setString(), or dict() / set() with lenId == 0)
            \returns pointer to key
            \note You need to know the key's data type */
        void * id(idx index, idx * pLen=0)
        {
            return sAlgo::hax_identifier( &_mex, index, pLen, (_hdr()->hax0)) ;
        }

        //! Get key with specified index
        /*! \param index index of key
            \param[out] pLen length of key (not including terminal 0 if the key was
                stored using dictString() / setString(), or dict() / set() with lenId == 0)
            \returns pointer to key
            \note You need to know the key's data type */
        const void * id(idx index, idx * pLen=0) const
        {
            return sAlgo::hax_identifier( &_mex, index, pLen, (_hdr()->hax0)) ;
        }

        Tobj* operator ()(const char * nam)
        {
            return get((const void*)nam,0,0);
        }
        const Tobj* operator ()(const char * nam) const
        {
            return get((const void*)nam,0,0);
        }
        Tobj& operator [](const char * nam)
        {
            return *set((const void*)nam,0,0);
        }
        Tobj& operator [](char * nam)
        {
            return *set((const void*)nam,0,0);
        }

        idx find(const char * nam, idx * pNum) const
        {
            return find((void*)nam,0,pNum);
        }

        idx find(char * nam, idx * pNum) const
        {
            return find((void*)nam,0,pNum);
        }

        Tobj & operator []( idx index){return *ptr(index);} // subscript operator
        const Tobj & operator []( idx index) const {return *ptr(index);} // subscript operator
        Tobj * operator ()( idx index){return Tparent::ptrx(index);}// subscript operator: resizes automagically

        template < class Tkey > Tobj* operator ()(const Tkey * key)
        {
            return get((const void*)key,sizeof(Tkey),0);
        }
        template < class Tkey > const Tobj* operator ()(const Tkey * key) const
        {
            return get((const void*)key,sizeof(Tkey),0);
        }
        template < class Tkey > Tobj& operator [](const Tkey * key)
        {
            return *set((const void*)key,sizeof(Tkey),0);
        }
        template < class Tkey > idx find(const Tkey * key, idx * pNum) const
        {
            return find((const void*)key,sizeof(Tkey),pNum);
        }


        public:
            // these are to serialize a string dictionary
            idx parse00( const char * list00 , idx lenmax=0, idx cntmax=0)
            {
                if(!lenmax)lenmax=sIdxMax;
                if(!cntmax)cntmax=sIdxMax;
                const char * p; idx ivis;
                for( p=list00, ivis=0 ;p && p<(list00+lenmax) && ivis<cntmax ;p=(*p ? p+sLen(p)+1 : 0) , ++ivis ) {
                    *set(p,sLen(p))=ivis;
                    if(p+sLen(p)>=list00+lenmax)break;
                }
                return dim();
            }
            idx parseFile00( const char * flnm)
            {
                sFil fl(flnm,sFil::fReadonly);
                return parse00(fl.ptr(),fl.length());
            }

            idx serialIn(const void * data, idx lenmax)
            {
                const char * buf = (const char *) data;
                const idx sizeOfObj = *((const idx *) buf);
                idx key_size;
                Tobj * p = 0;
                const char * const end = buf + lenmax;
                buf += sizeof(idx);
                while( buf && buf < end ) {
                    key_size = *((const idx *) buf);
                    buf += sizeof(idx);
                    if( buf + key_size + sizeOfObj > end ) {
                        return -2;
                    }
                    p = set(buf, key_size);
                    if( !p ) {
                        return -1;
                    }
                    buf += key_size;
                    *p = *(Tobj *) buf;
                    buf += sizeOfObj;
                }
                return dim();
            }

            idx serialOut(sMex & buf) const
            {
                idx size = sizeof(Tobj);
                buf.add((const char *) &size, sizeof(size));

                for(idx i = 0; i < dim(); ++i) {
                    const void * key = id(i, &size);
                    buf.add((const char *) &size, sizeof(size));
                    buf.add((const char *) key, size);
                    buf.add((const char *) ptr(i), sizeof(Tobj));
                }
                return buf.pos();
            }
    };
}

#endif //  sLib_core_Dic.hpp
