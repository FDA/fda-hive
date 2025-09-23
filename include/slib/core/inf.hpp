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
#ifndef sLib_obj_world_h
#define sLib_obj_world_h

#include <slib/core/mex.hpp>
#include <slib/core/algo.hpp>

namespace slib
{


    template <int Tbits=3> class sInf { 
    private:
        sMex _mex; 
        

        struct Bar {
            sAlgo::lix lstDat; 
            
            sAlgo::lix lstSub; 

            sAlgo::lix lstSup;


            Bar(void){
                lstDat=sAlgo::emptyLix();
                lstSub=sAlgo::emptyLix();
                lstSup=sAlgo::emptyLix();
            }
        };


        struct Hdr { 

            sAlgo::lix lstBar;  
            sAlgo::lix haxBar; 

            Hdr(void){
                lstBar=sAlgo::emptyLix();
                haxBar=sAlgo::emptyLix();
            } 
        };

        Hdr * _hdr(void){return (Hdr *)_mex.ptr(0); }
        
    public:
        sInf() :_mex(sMex::fAlignInteger) { init( ); }
        ~sInf () { empty(); }
        sInf * init (const char * flnm=0)
        {
            if(flnm)
                _mex.init(flnm);
            if( _mex.pos()==0) {
                _mex.empty();
                _mex.add(0, sizeof(Hdr)) ;
                new (_mex.ptr(0)) Hdr;
            }
            return this;
        }
        
        void empty(void) {_mex.empty();init();}


    private:

        #define lix_save idx lixofsinside=(idx)((char *)(plix)-(char*)(mex->ptr())); 
        #define lix_restore    plix=(lix *)mex->ptr(lixofsinside);

        template <typename Tobj > idx _cnt(sAlgo::lix plix, Tobj * nullObj) { 
            return sAlgo::lix_cnt(&_mex,lix); 
        }
        template <typename Tobj > Bar * _ptr(idx index, sAlgo::lix plix, Tobj * nullObj) { 
            return (Tobj *)sAlgo::lix_ptr(&_mex,index,sizeof(Tobj),lix);
        }
        template <typename Tobj > Tobj * _add(idx cntAdd, sAlgo::lix * plix, idx bits, Tobj * nullObj){ 
            idx cntall=_cnt(*plix, nullObj);
                lix_save    
            Bar * p=(Bar *)sAlgo::lix_add(&_mex,cntAdd,sizeof(Tobj),plix,bits);
                lix_restore
            for ( idx i=0 ; i < cntAdd ; ++i )new ( _ptr(cntall+i, plix , nullObj) ) Tobj;
            return p;
        }
        template <typename Tobj > Tobj * _del( idx posDel, idx cntDel, sAlgo::lix * plix, Tobj * nullObj) { 
            idx cntall=_cnt(*plix,nullObj) ; if(!cntall)return ;
            for ( idx i=0 ; i < cntDel && posDel+i<cntall; ++i )_ptr(posDel+i,plix,nullObj)->~Tobj();
            for ( idx i=0 ; i < cntDel && posDel+i<cntall; ++i )*((Tobj*)_ptr(posDel+i,plix,nullObj))=*((Tobj*)_ptr(posDel+cntDel+i,plix,nullObj));
            sAlgo::lix_unlink(&_mex,cntDel,plix);
        }
        
        #undef lix_save 
        #undef lix_restore    
    
    private:
        
        idx _hash( idx iFnd, const void * id, idx lenId, sAlgo::lix * phax) 
        {
            idx stlen = lenId ? lenId : sLen((const char *)id) ; if(!lenId)lenId=stlen+1;
            idx idofs=_mex.add(0,lenId);
            memmove( _mex.ptr(idofs) , (const void *)id, lenId );
            sAlgo::hax_map(&_mex, iFnd, idofs, stlen, 0, phax, Tbits, 4 , Tbits);
            return idofs;
        }
        void _unhash( idx posDel, idx cntDel, sAlgo::lix * phax)  { 
            sAlgo::hax_unmap(&_mex, posDel,cntDel, phax);
        }
        idx _find(const void * id, idx lenId, idx * pNum, sAlgo::lix phax) 
        {   
            idx iFnd=sAlgo::hax_find(&_mex,id, lenId ? lenId : sLen((const char *)id) , 0, 0, phax);
            if(pNum)*pNum=iFnd-1;
            return iFnd;
        }
    
    public:
        
        idx dim(void) {
            return _cnt(_hdr()->lstBar, sNullPtr(Bar));
        }

        Bar * ptr(idx index) {
            return _ptr(index,_hdr()->lstBar,sNullPtr(Bar));
        }
        
        Bar * set( const char * id, idx lenId=0, idx * pNum=0) 
        {
            idx iFnd=_find(id, lenId , pNum);
            if( !iFnd-- ){
                iFnd=dim();
                _add(1,&(_hdr()->lstBar)); 
                _hash(iFnd, id, lenId,&(o->haxBar));
                if(pNum)*pNum=iFnd;
            }
            return ptr(iFnd);
        }
        
        Bar * get(const char * id, idx lenId=0, idx * pNum=0)
        {
            idx iFnd=_find(id, lenId, pNum);
            return (Bar * )((iFnd--) ? _ptr(iFnd) : 0 ); 
        }

        void cln(const char * id, idx lenId=0)
        {
            idx iFnd=_find(id, lenId ,0 );
            if( !iFnd-- )return ; 
            _unhash(iFnd,1,&(o->haxBar));
            _del(iFnd,&(_hdr()->lstBar));
        }
        


        idx learn(sMex * mex, idx cnt, sMex::Pos * poslist);
        idx learn(const char * src00 , const char * separ);


    };
}

#endif 
