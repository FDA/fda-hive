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
#ifndef sLib_core_lst_h
#define sLib_core_lst_h

#include <slib/core/mex.hpp>
#include <slib/core/algo.hpp>

namespace slib
{



    template< class Tobj , int Tbits=8 > class sLst : public sArr< Tobj, sLst<Tobj, Tbits> >
    {
    public:
        typedef sArr< Tobj, sLst<Tobj, Tbits> > Tparent;
        friend class sArr< Tobj, sLst<Tobj, Tbits> >;
        sAlgo::lix lst0;
        sMex * _mex;
        sLst( sMex * mex=0)  { init(mex)  ; }
        ~sLst () { this->sdel_op(0,dim(),(Tobj * )0); this->empty(); }
        sLst * init(sMex * mex=0) { _mex=mex; lst0=sAlgo::emptyLix(); return this;}

    private:
        Tobj * _add(idx cntAdd=1) __attribute__((used)) {return (Tobj *)sAlgo::lix_add(_mex,cntAdd,sizeof(Tobj),&lst0,Tbits);}
        void _del( idx posDel, idx cntDel) __attribute__((used)) { Tparent::sshift_op( posDel+cntDel, posDel, cntDel); sAlgo::lix_unlink(_mex,cntDel,&lst0);}
    public:
        const Tobj * ptr( idx index=0) const __attribute__((used)) { return static_cast<const Tobj *>(sAlgo::lix_ptr(_mex,index,sizeof(Tobj),lst0));}
        Tobj * ptr( idx index=0) __attribute__((used)) { return static_cast<Tobj*>(sAlgo::lix_ptr(_mex,index,sizeof(Tobj),lst0));}
        idx dim(void) const __attribute__((used)) { return sAlgo::lix_cnt(_mex,lst0);}
        sMex * mex(void) __attribute__((used)) {return _mex;}
        const sMex * mex(void) const __attribute__((used)) { return _mex; }


    };

}


#endif 










