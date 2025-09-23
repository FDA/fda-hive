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

    template < class Tobj , int Tbits=3 > class sDic: public sArr< Tobj, sDic<Tobj, Tbits> >
    {

    private:
        typedef sArr< Tobj, sDic<Tobj, Tbits> > Tparent;
        friend class sArr< Tobj, sDic<Tobj, Tbits> >;
        sMex _mex;
        struct Hdr { sAlgo::lix lst0,hax0; Hdr(void){lst0=hax0=sAlgo::emptyLix();} };
        Hdr * _hdr(void) const {return (Hdr *)_mex.ptr(0); };

    public:

        sDic(const char * flnm = 0, idx flgs = 0 ) { _mex.flags|=sMex::fAlignInteger; init( flnm, flgs); }
        ~sDic () {
            idx hFile=(_mex.flags>>(sMex_FileHandleShift)) & 0xFFFF;
            if(hFile>0){
                destroy();
            } else {
                empty();
            }
        }
        sDic * init (const char * flnm=0, idx flgs = sMex::_defaultFlags)
        {
            if(flnm) {
                _mex.init(flnm,flgs);
            }
            if( _mex.pos()<= (idx)sizeof(Hdr)) {
                _mex.empty();
                _mex.add(0, sizeof(Hdr)) ;
                new (_hdr()) Hdr;
            }
            return this;
        }
        sDic * borrow(sDic<Tobj, Tbits> * src)
        {
            if( _hdr() ) {
                Tparent::empty();
            }
            _mex.empty();
            _mex.borrow(&src->_mex);
            return this;
        }
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
                undict( posDel, cntDel);
                Tparent::sshift_op( posDel+cntDel, posDel, cntDel);
                sAlgo::lix_unlink(&_mex,cntDel,&(_hdr()->lst0));
            }
        }
    public:
        Tobj * ptr( idx index=0) { return (Tobj *)sAlgo::lix_ptr(&_mex,index,sizeof(Tobj),(_hdr()->lst0)); }
        const Tobj * ptr(idx index=0) const { return (const Tobj *)sAlgo::lix_ptr(&_mex,index,sizeof(Tobj),(_hdr()->lst0)); }
        idx dim(void) const { return sAlgo::lix_cnt(&_mex,(_hdr()->lst0));}

    public:
        idx dictCnt(void) const
        {
            return sAlgo::lix_cnt(&_mex,(_hdr()->hax0));
        }
        idx dict( idx iFnd, const void * id, idx lenId=0)
        {
            idx stlen = lenId ? lenId : sLen((const char *)id) ;
            if(!lenId)lenId=stlen+1;

            idx idofs=_mex.add(0,lenId);
            memmove( _mex.ptr(idofs) , (const void *)id, lenId );
            sAlgo::hax_map(&_mex, iFnd, idofs, stlen, 0, &(_hdr()->hax0), Tbits, 4 , Tbits);

            return idofs;
        }
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

        void undict( idx posDel, idx cntDel=1, bool forceRehash=false)
        {
            sAlgo::hax_unmap(&_mex, posDel,cntDel, &(_hdr()->hax0));
            if(forceRehash)
                rehash();
        }
        void rehash( void )
        {
            sAlgo::hax_rehash(&_mex, 0, (_hdr()->hax0));
        }
        sMex * mex(){ return &_mex; }
        const sMex * mex() const { return &_mex; }
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
        Tobj * set( const void * id, idx lenId=0, idx * pNum=0)
        {
            idx iFnd=find(id, lenId ? lenId : sLen((const char *)id) , pNum);

            if( !iFnd-- ){
                iFnd=dim();
                Tparent::add(1);
                dict(iFnd, id, lenId);
            }
            if(pNum)
                *pNum=iFnd;

            return (Tobj * )ptr(iFnd);
        }

        Tobj * setString(const char * id, idx lenId=0, idx * pNum=0)
        {
            if( !lenId )
                lenId = sLen(id);

            idx iFnd=find(id, lenId, pNum);

            if( !iFnd-- ) {
                iFnd = dim();
                Tparent::add(1);
                dictString(iFnd, id, lenId);
            }
            if(pNum)
                *pNum=iFnd;

            return (Tobj * )ptr(iFnd);
        }

        Tobj * get(const void * id, idx lenId=0, idx * pNum =0)
        {
            idx iFnd=find(id, lenId, pNum);
            return (Tobj * )((iFnd--) ? ptr(iFnd) : 0 );
        }

        const Tobj * get(const void * id, idx lenId=0, idx * pNum =0) const
        {
            idx iFnd=find(id, lenId, pNum);
            return (const Tobj * )((iFnd--) ? ptr(iFnd) : 0 );
        }

        idx find(const void * id, idx lenId=0, idx * pNum =0) const
        {
            if(!_hdr())return 0;
            idx iFnd=sAlgo::hax_find(&_mex,id, lenId ? lenId : sLen((const char *)id) , 0, 0, (_hdr()->hax0));
            if(pNum)
                *pNum=iFnd-1;
            return iFnd;
        }


        void * id(idx index, idx * pLen=0)
        {
            return sAlgo::hax_identifier( &_mex, index, pLen, (_hdr()->hax0)) ;
        }

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

        Tobj & operator []( idx index){return *ptr(index);}
        const Tobj & operator []( idx index) const {return *ptr(index);}
        Tobj * operator ()( idx index){return Tparent::ptrx(index);}

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

#endif 