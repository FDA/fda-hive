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

#ifdef WIN32
#pragma warning(disable:4189)
#endif

namespace slib
{


    template< class Tobj > class sVec : public sArr< Tobj, sVec<Tobj> >
    {

    private:
        typedef sArr< Tobj, sVec<Tobj> > Tparent;
        friend class sArr< Tobj, sVec<Tobj> >;
        sMex _mex;

    public:
        sVec( idx lflags=sMex::fBlockDoubling, const char * flnm=0, idx offset=0, idx sizemap=0 )  : _mex( lflags ) {if(flnm) _mex.init(flnm, 0, offset, sizemap*sizeof(Tobj));}
        sVec( const char * flnm, idx offset=0, idx sizemap=0)  { if(flnm) _mex.init(flnm, 0 , offset,sizemap*sizeof(Tobj)); }
        sVec ( const sVec <Tobj > & v){ Tparent::add(v.dim());memcpy(ptr(), v.ptr(), v.dim()*sizeof(Tobj));}

        ~sVec () { this->sdel_op(0,dim(),(Tobj*) 0); }

        sVec * init(const char * flnm,idx flags=0) {_mex.init(flnm,flags);return this;}
        sVec * init(idx lflags, const char * flnm, idx offset, idx sizemap) {_mex.init(flnm, lflags,offset,sizemap*sizeof(Tobj)); return this;}
        sVec * init(idx flags) {_mex.init(flags);return this;}

        inline idx setflag(idx flg=sNotIdx) { if(flg!=sNotIdx) _mex.flags=flg; return _mex.flags;}

    private: 
        inline Tobj * _add(idx cntAdd=1) {return (Tobj * )_mex.ptr(_mex.add(0, sizeof(Tobj)*cntAdd));}
        inline void _del( idx posDel, idx cntDel) {_mex.del( posDel*sizeof(Tobj), cntDel*sizeof(Tobj) );}
        inline Tobj * _insert(idx pos, idx cntAdd) { return (Tobj*)_mex.ptr( _mex.insert(pos*sizeof(Tobj),0,cntAdd*sizeof(Tobj)) );}

    public:
        typedef Tobj value_type;

        inline const Tobj * ptr(idx index=0) const { return static_cast<const Tobj*>(_mex.ptr(index*sizeof(Tobj))); }
        inline Tobj * last(idx index=-1) { return static_cast<Tobj*>(_mex.ptr(_mex.pos()+(index)*sizeof(Tobj))); }
        inline Tobj * ptr(idx index=0) { return static_cast<Tobj*>(_mex.ptr(index*sizeof(Tobj))); }
        inline idx dim(void) const {return  _mex.pos()/sizeof(Tobj) ;}
        inline sMex * mex(void) {return &_mex;}
        inline const sMex * mex(void) const {return &_mex;}

    public:
        inline operator Tobj *() { return ptr();}
        inline operator const Tobj *() const { return ptr(); }
        inline void set(idx val=0) { _mex.set(val);  }
        inline void destroy(void) { _mex.destroy();  }
        inline void cutM(idx pos) { _mex.cut(pos*sizeof(Tobj));}

        bool ok (void) const { return  _mex.ok();}
    public:

    };

    template< class Tobj > class sStack : public sVec<Tobj> {
    public:
        virtual ~sStack<Tobj>() {}

        virtual inline Tobj * push( void) {
            return this->add();
        }
        virtual inline Tobj * top(void) {
            return sVec<Tobj>::ptr( this->dim()-1);
        }
        virtual inline Tobj * pop(void) {
            Tobj * t = sVec<Tobj>::ptr( this->dim()-1 );
            sVec<Tobj>::cutM( this->dim()-1 );
            return t;
        }
    };
}



#endif 











