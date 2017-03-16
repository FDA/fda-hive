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
#ifndef sLib_core_vec_h
#define sLib_core_vec_h

#include <slib/core/mex.hpp>
//#include <assert.h>

#ifdef WIN32
#pragma warning(disable:4189)
#endif

namespace slib
{

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ class sVec
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    template< class Tobj > class sVec : public sArr< Tobj, sVec<Tobj> >
    {

    private:
        typedef sArr< Tobj, sVec<Tobj> > Tparent;
        friend class sArr< Tobj, sVec<Tobj> >;
        sMex _mex;

    public: // constructor/destructor
        sVec( idx lflags=sMex::fBlockDoubling, const char * flnm=0, idx offset=0, idx sizemap=0 )  : _mex( lflags ) {if(flnm) _mex.init(flnm, 0, offset, sizemap*sizeof(Tobj));}
        sVec( const char * flnm, idx offset=0, idx sizemap=0)  { if(flnm) _mex.init(flnm, 0 , offset,sizemap*sizeof(Tobj)); }
        sVec ( const sVec <Tobj > & v){ Tparent::add(v.dim());memcpy(ptr(), v.ptr(), v.dim()*sizeof(Tobj));}

        ~sVec () { this->sdel_op(0,dim(),(Tobj*) 0); }

        sVec * init(const char * flnm,idx flags=0) {_mex.init(flnm,flags);return this;}
        sVec * init(idx lflags, const char * flnm, idx offset, idx sizemap) {_mex.init(flnm, lflags,offset,sizemap*sizeof(Tobj)); return this;}
        sVec * init(idx flags) {_mex.init(flags);return this;}

        inline idx setflag(idx flg=sNotIdx) { if(flg!=sNotIdx) _mex.flags=flg; return _mex.flags;}

    // sArr implementations
    private: 
        inline Tobj * _add(idx cntAdd=1) {return (Tobj * )_mex.ptr(_mex.add(0, sizeof(Tobj)*cntAdd));}
        inline void _del( idx posDel, idx cntDel) {_mex.del( posDel*sizeof(Tobj), cntDel*sizeof(Tobj) );}
        inline Tobj * _insert(idx pos, idx cntAdd) { return (Tobj*)_mex.ptr( _mex.insert(pos*sizeof(Tobj),0,cntAdd*sizeof(Tobj)) );}

    public:
        typedef Tobj value_type;

        inline const Tobj * ptr(idx index=0) const { return static_cast<const Tobj*>(_mex.ptr(index*sizeof(Tobj))); }
        inline Tobj * last(idx index=-1) { return static_cast<Tobj*>(_mex.ptr(_mex.pos()+(index)*sizeof(Tobj))); }
        inline Tobj * ptr(idx index=0) { return static_cast<Tobj*>(_mex.ptr(index*sizeof(Tobj))); }
        inline idx dim(void) const {return  _mex.pos()/sizeof(Tobj) ;}// { return _cnt; }
        inline sMex * mex(void) {return &_mex;}
        inline const sMex * mex(void) const {return &_mex;}

    public: // operations
        inline operator Tobj *() { return ptr();} // operator typecast to Tobj *
        inline operator const Tobj *() const { return ptr(); }
        inline void set(idx val=0) { _mex.set(val);  } // set to certain value
        inline void destroy(void) { _mex.destroy();  } //
        inline void cutM(idx pos) { _mex.cut(pos*sizeof(Tobj));}

        bool ok (void) const { return  _mex.ok();}
    public: // serialization

    };
    //sOutClass(sVec)

    template< class Tobj > class sStack : public sVec<Tobj> {
    public:
        virtual ~sStack<Tobj>() {}

        virtual inline Tobj * push( void) {
            return this->add();
        }
        virtual inline Tobj * top(void) {
            // assert( this->dim() );
            return sVec<Tobj>::ptr( this->dim()-1);
        }
        virtual inline Tobj * pop(void) {
            // assert( this->dim() );
            Tobj * t = sVec<Tobj>::ptr( this->dim()-1 );
            sVec<Tobj>::cutM( this->dim()-1 );
            return t;
        }
    };
}



#endif // sLib_core_vec_h











/*

    template< class Tobj > class sVec : protected sMex : public sArr<Tobj>
    {

    public: // constructor/destructor
        sVec( idx lflags=0, const char * flnm=0 )  : sMex ( lflags ) {if(flnm) sMex::init(flnm);}
        sVec( const char * flnm)  : sMex ( ) { if(flnm) sMex::init(flnm); }
        sVec ( sVec <Tobj > & v){ add(v.dim());memcpy(ptr(), v.ptr(), v.dim()*sizeof(Tobj));}
    
        ~sVec () { Tobj * p=ptr(); sDel_op(p,dim()); }
        sVec * init(const char * flnm){sMex::init(flnm);return this;}
        sVec * init(idx flags){sMex::init(flags);return this;}
        
    public: // information

        Tobj * ptr(idx index=0) { return (Tobj *)sMex::ptr(index*sizeof(Tobj)); }
        Tobj * ptrx(idx index=0) { sMex::resize((index+1)*sizeof(Tobj)); return ptr(index);}
        idx cnt(void) {return  pos()/sizeof(Tobj) ;}// { return _cnt; }
        //Tobj & operator []( idx index){return *ptr(index);} // subscript operator:
        Tobj * operator ()( idx index){return ptrx(index);}// subscript operator: resizes automagically
        operator Tobj *(){ return ptr();} // operator typecast to Tobj * 
        
        
    public: // operations
        Tobj * resize(idx totalCnt){return ptrx(totalCnt);}
        void empty(void){del(0,dim());}

        Tobj * addva( idx cntAdd, va_list marker) // request element(s) , return  pointer and the index 
        {   
            Tobj * p=(Tobj * )sMex::ptr(sMex::add(0, sizeof(Tobj)*cntAdd));
            for(idx jj=0;  jj<(idx)cntAdd;  ++jj) 
                *(p+jj)=va_arg(marker, Tobj ); 
            return p;
        }

        Tobj * add( idx cntAdd=1) // request element(s) , return  pointer and the index 
        {   
            Tobj * p=(Tobj * )sMex::ptr(sMex::add(0, sizeof(Tobj)*cntAdd));
            sNew_op(p,(idx)cntAdd);
            
            return p;
        }

        Tobj * vadd( idx cntAdd=1, ...) // request element(s) , return  pointer and the index 
        {
            Tobj * res;
            sCallVargRes(res,addva,cntAdd);
            return res;
        }

        void del( idx pos, idx cntDel=1 ) // delete element(s) 
        {   
            sDel_op(ptr(pos),cntDel);
            sMex::del( pos*sizeof(Tobj), cntDel*sizeof(Tobj) );
        }
        
        Tobj * insert(idx pos, idx cntAdd=1)
        {
            sMex::insert(pos*sizeof(Tobj),0,cntAdd*sizeof(Tobj));
            Tobj * p=ptr(pos);
            sNew_op(p,cntAdd);
            return p;
        }

        void set(idx val=0) { sMex::set(val);  } // set to certain value 
        void destroy(void) { sMex::destroy();  } //  
        void cut(idx pos=0) { del(pos, dim()-pos);}//{ sMex::cut(pos*sizeof(Tobj));  } // cut
    

        //Tobj * resize(idx totalCnt){if(totalCnt*sizeof(Tobj)>sMex::pos())return vadd(totalCnt-dim(),0);else return sMex::ptr(sMex::pos());}

       

    public:
        inline void _sOutf( void ) { 
            _sOutDw;
                sMex::_sOutf();
            _sOutUp;
        }

    };

*/

