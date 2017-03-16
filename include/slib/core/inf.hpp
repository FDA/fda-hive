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

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ class sWorld
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    template <int Tbits=3> class sInf { 
    private:
        // Extensible memory management class 
        sMex _mex; 
        

        // Bar structure signifies a Notion in the Information Space
        struct Bar {
            // The lixBase of data-references associated 
            // with the current notion.
            sAlgo::lix lstDat; 
            
            // For complex notions this ius the lixBase of notions 
            // this sub-notions is made of. One may think of this as a 
            // list of children notions in a hierarchical notion 
            // representation or as a list of outgoing nodes in 
            // drectional graph.
            sAlgo::lix lstSub; 

            // The lixBase for super-notions this notion is 
            // belong to. One may think of this as a list of parent 
            // notions in a hierarchical notion representation or as a 
            // list of incoming nodes in drectional graph.
            sAlgo::lix lstSup;


            Bar(void){
                lstDat=sAlgo::emptyLix();
                lstSub=sAlgo::emptyLix();
                lstSup=sAlgo::emptyLix();
            }
        };


        // Hdr is the Information Space header 
        struct Hdr { 

            // Major lixBase for notions 
            sAlgo::lix lstBar;  
            // Major lixBase for hashes of notions 
            sAlgo::lix haxBar; 

            Hdr(void){
                lstBar=sAlgo::emptyLix();
                haxBar=sAlgo::emptyLix();
            } 
        };

        Hdr * _hdr(void){return (Hdr *)_mex.ptr(0); }
        
    public: // Constructors/Destructors
        sInf() :_mex(sMex::fAlignInteger) { init( ); }
        ~sInf () { empty(); }
        sInf * init (const char * flnm=0)
        {
            if(flnm)
                _mex.init(flnm);
            if( _mex.pos()==0) {
                _mex.empty(); // this just makes sure that if mex was truncated (pos==0) its memory was released
                _mex.add(0, sizeof(Hdr)) ; // the very first intialization - we reserve a place fr the lix_hdr structures 
                new (_mex.ptr(0)) Hdr; // put a Hdr object in the beginning  constructor
            }
            return this;
        }
        
        // TODO implement wipeout of the data 
        void empty(void) {_mex.empty();init();}


    private: // low level lists manipulation routines

        #define lix_save idx lixofsinside=(idx)((char *)(plix)-(char*)(mex->ptr())); 
        #define lix_restore    plix=(lix *)mex->ptr(lixofsinside); // put back the pointer to lix header 

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
            for ( idx i=0 ; i < cntAdd ; ++i )new ( _ptr(cntall+i, plix , nullObj) ) Tobj; // contructors 
            return p;
        }
        template <typename Tobj > Tobj * _del( idx posDel, idx cntDel, sAlgo::lix * plix, Tobj * nullObj) { 
            idx cntall=_cnt(*plix,nullObj) ; if(!cntall)return ;
            for ( idx i=0 ; i < cntDel && posDel+i<cntall; ++i )_ptr(posDel+i,plix,nullObj)->~Tobj(); // destruct 
            for ( idx i=0 ; i < cntDel && posDel+i<cntall; ++i )*((Tobj*)_ptr(posDel+i,plix,nullObj))=*((Tobj*)_ptr(posDel+cntDel+i,plix,nullObj));
            sAlgo::lix_unlink(&_mex,cntDel,plix); // unlink the blocks 
        }
        
        #undef lix_save 
        #undef lix_restore    
    
    private: // low level hash manipulation routines
        
        idx _hash( idx iFnd, const void * id, idx lenId, sAlgo::lix * phax) 
        {
            idx stlen = lenId ? lenId : sLen((const char *)id) ; if(!lenId)lenId=stlen+1;
            idx idofs=_mex.add(0,lenId); // we allocate before getting the pointer since 'did' might be in the same buffer as idmex is 
            memmove( _mex.ptr(idofs) , (const void *)id, lenId ); // copy the content of id to the just allocated spot
            sAlgo::hax_map(&_mex, iFnd, idofs, stlen, 0, phax, Tbits, 4 , Tbits);
            return idofs;
        }
        void _unhash( idx posDel, idx cntDel, sAlgo::lix * phax)  { 
            sAlgo::hax_unmap(&_mex, posDel,cntDel, phax);
        }
        idx _find(const void * id, idx lenId, idx * pNum, sAlgo::lix phax) 
        {   
            idx iFnd=sAlgo::hax_find(&_mex,id, lenId ? lenId : sLen((const char *)id) , 0, 0, phax); // search if this item is present
            if(pNum)*pNum=iFnd-1;
            return iFnd;
        }
    
    public:     // Major notion list manipulation functions
        
        // The dimension of the notions' array
        idx dim(void) {
            return _cnt(_hdr()->lstBar, sNullPtr(Bar));
        }

        // Pointer to a particular notion
        Bar * ptr(idx index) {
            return _ptr(index,_hdr()->lstBar,sNullPtr(Bar));
        }
        
        // Create (or use existing) dictionarized notion
        Bar * set( const char * id, idx lenId=0, idx * pNum=0) 
        {
            idx iFnd=_find(id, lenId , pNum); // search if this item is present
            if( !iFnd-- ){ // add this since it is not yet in
                iFnd=dim();
                _add(1,&(_hdr()->lstBar)); 
                _hash(iFnd, id, lenId,&(o->haxBar));
                if(pNum)*pNum=iFnd;
            }
            return ptr(iFnd);
        }
        
        // Retrieve dictionarized item by its id
        Bar * get(const char * id, idx lenId=0, idx * pNum=0) // get an item from dictionary
        {
            idx iFnd=_find(id, lenId, pNum);
            return (Bar * )((iFnd--) ? _ptr(iFnd) : 0 ); 
        }

        // Remove a notion from notion list
        void cln(const char * id, idx lenId=0)
        {
            idx iFnd=_find(id, lenId ,0 ); // search if this item is present
            if( !iFnd-- )return ; 
            _unhash(iFnd,1,&(o->haxBar));
            _del(iFnd,&(_hdr()->lstBar));
        }
        


        idx learn(sMex * mex, idx cnt, sMex::Pos * poslist);
        idx learn(const char * src00 , const char * separ);

    //    template < typename Tdata > data(_)

    };
} // namespace slib

#endif //  sLib_obj_world_h
    /*

Notions

1. list of notions can be created 
    inf.set(notion);

2. notions can be retrieved 
    Notion * inf.get(notion)
3. notions can be forgotten 
    inf.cln(notion);

4. Each notion can have associated data array
    notion.data.add[i]
    data item can be either a blob or a reference 
    to another notion.
    

5. notions can have references to other notions
    Notion * notion[i];


    complex notions are called sentences
    they can be populated by learning the whole sentence at once 
    Notion * inf.learn(notion1, notion2, ...,0 );
    inf.learn("porsche color yellow");

6. sentences can be retrieved by positional complete or partial 
    specification of their subnotions  

    NotionRefList * get(notion1,0,notion3,...);
    sentences can be retrieved by non-positional specification 
    of their subnotions
    getAny(notion1,notion4,...);
    
    
5. searching all relati    ons having a bar in a certain position

every sentence

// objects can be added to the list 
// objects can be linked to eachother


// compactize function clears everything which is not used in mex


object -> property  -> value

    // when deleting ... empty if all are asked to be deleted
        //if(cntDel==cntall && posDel==0) {empty();return ;} // empty if all are being deleted 
            
    //template < class Tkey > Ref * get(Tkey * key){return get((const void*)key,sizeof(Tkey),0);}  
    //template < class Tkey > Ref * set(Tkey * key){return set((const void*)key,sizeof(Tkey),0);}  
    //template < class Tkey > Ref & get(Tkey & key){return *get((const void*)&key,sizeof(Tkey),0);}  
    //template < class Tkey > Ref & set(Tkey & key){return *set((const void*)&key,sizeof(Tkey),0);}  



    */

