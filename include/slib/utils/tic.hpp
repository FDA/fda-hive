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
#ifndef sLib_utils_tic_h
#define sLib_utils_tic_h

#include <slib/core/algo.hpp>
#include <slib/core/vec.hpp>

namespace slib {
    template<class Tobj, int Tbits = 3> class sTic
    {
        private:
            struct Hdr
            {
                sAlgo::lix lst0;
                sAlgo::lix hax0;
                Hdr() { lst0 = hax0 = sAlgo::emptyLix(); }
            };
            sVec<sMex::Pos> _stack;
            sMex _mex;
            Hdr * _hdr(idx ilevel) { return static_cast<Hdr*>(_mex.ptr(_stack[ilevel].pos)); }
            const Hdr * _hdr(idx ilevel) const { return static_cast<const Hdr*>(_mex.ptr(_stack[ilevel].pos)); }

        public:
            sTic()
            {
                _mex.flags |= sMex::fAlignInteger;
                push();
            }
            ~sTic()
            {

                empty();
            }

            idx push()
            {
                if( dimStack() ) {
                    _stack[dimStack() - 1].size = _mex.pos() - _stack[dimStack() - 1].pos;
                }
                idx new_pos = _mex.add(0, sizeof(Hdr));
                _stack.add(1)->pos = new_pos;
                new (_mex.ptr(new_pos)) Hdr;
                return dimStack();
            }
            idx pop()
            {
                idx ilevel = dimStack() - 1;
                idx dim_top = dimTop();
                for(idx iobj = 0; iobj < dim_top; iobj++) {
                    ptr(ilevel, iobj)->~Tobj();
                }
                idx prev_mex_pos = ilevel ? _stack[ilevel - 1].pos + _stack[ilevel - 1].size : 0;
                _mex.cut(prev_mex_pos);
                _stack.cut(ilevel);
                return dimStack();
            }
            void empty()
            {
                if( dimStack() ) {
                    while( pop() );
                }
            }

            idx dimStack() const { return _stack.dim(); }
            idx dimLevel(idx ilevel) const
            {
                return sAlgo::lix_cnt(&_mex, _hdr(ilevel)->lst0);
            }
            idx dimTop() const { return dimStack() ? dimLevel(dimStack() - 1) : 0; }

            Tobj * setRaw(void * id, idx len_id, idx * piobj = 0)
            {
                idx ilevel = dimStack() - 1;
                idx iobj = sAlgo::hax_find(&_mex, id, len_id, 0, 0, _hdr(ilevel)->hax0) - 1;
                if( iobj < 0 ) {
                    iobj = dimLevel(ilevel);
                    sAlgo::lix_add(&_mex, 1, sizeof(Tobj), &(_hdr(ilevel)->lst0), Tbits);
                    new (_mex.ptr(_mex.add(0, sizeof(Tobj)))) Tobj;
                    idx id_pos = _mex.add(NULL, len_id);
                    memcpy(_mex.ptr(id_pos), id, len_id);
                    sAlgo::hax_map(&_mex, iobj, id_pos, len_id, 0, &(_hdr(ilevel)->hax0), Tbits, 4, Tbits);
                }
                if( piobj ) {
                    *piobj = iobj;
                }
                return ptr(ilevel, iobj);
            }
            Tobj * setString(const char * id, idx len_id = 0, idx * piobj = 0)
            {
                idx ilevel = dimStack() - 1;
                if( !len_id ) {
                    len_id = sLen(id);
                }
                idx iobj = sAlgo::hax_find(&_mex, id, len_id, 0, 0, _hdr(ilevel)->hax0) - 1;
                if( iobj < 0 ) {
                    iobj = dimLevel(ilevel);
                    sAlgo::lix_add(&_mex, 1, sizeof(Tobj), &(_hdr(ilevel)->lst0), Tbits);
                    new (_mex.ptr(_mex.add(NULL, sizeof(Tobj)))) Tobj;
                    idx id_pos = _mex.add(0, len_id + 1);
                    memcpy(_mex.ptr(id_pos), id, len_id);
                    *static_cast<char*>(_mex.ptr(id_pos + len_id)) = 0;
                    sAlgo::hax_map(&_mex, iobj, id_pos, len_id, 0, &(_hdr(ilevel)->hax0), Tbits, 4, Tbits);
                }
                if( piobj ) {
                    *piobj = iobj;
                }
                return ptr(ilevel, iobj);
            }
            Tobj * get(idx ilevel, const void * id, idx len_id = 0, idx * piobj = 0)
            {
                idx iobj = sAlgo::hax_find(&_mex, id, len_id ? len_id : sLen(static_cast<const char *>(id)), 0, 0, _hdr(ilevel)->hax0) - 1;
                if( piobj ) {
                    *piobj = iobj;
                }

                if( iobj >= 0 ) {
                    return ptr(ilevel, iobj);
                } else {
                    return 0;
                }
            }
            const Tobj * get(idx ilevel, const void * id, idx len_id = 0, idx * piobj = 0) const { return const_cast<sTic<Tobj, Tbits>*>(this)->get(ilevel, id, len_id, piobj); }
            Tobj * getTop(const void * id, idx len_id = 0, idx * piobj = 0) { return dimStack() ? get(dimStack() - 1, id, len_id, piobj) : 0; }
            const Tobj * getTop(const void * id, idx len_id = 0, idx * piobj = 0) const { return dimStack() ? get(dimStack() - 1, id, len_id, piobj) : 0; }

            Tobj * ptr(idx ilevel, idx iobj)
            {
                return static_cast<Tobj*>(sAlgo::lix_ptr(&_mex, iobj, sizeof(Tobj), (_hdr(ilevel)->lst0)));
            }
            const Tobj * ptr(idx ilevel, idx iobj) const
            {
                return static_cast<const Tobj*>(sAlgo::lix_ptr(&_mex, iobj, sizeof(Tobj), (_hdr(ilevel)->lst0)));
            }
            Tobj * ptrTop(idx iobj) { return dimStack() ? ptr(dimStack() - 1, iobj) : 0; }
            const Tobj * ptrTop(idx iobj) const { return dimStack() ? ptr(dimStack() - 1, iobj) : 0; }

            const void * id(idx ilevel, idx iobj, idx * plen_id = 0) const
            {
                return sAlgo::hax_identifier(&_mex, iobj, plen_id, _hdr(ilevel)->hax0) ;
            }
            const char * sid(idx ilevel, idx iobj, idx * plen_id = 0) const { return static_cast<const char*>(id(ilevel, iobj, plen_id)); }
            const void * idTop(idx iobj, idx * plen_id = 0) const { return dimStack() ? id(dimStack() -1, iobj, plen_id) : 0; }
            const char * sidTop(idx iobj, idx * plen_id = 0) const { return static_cast<const char*>(idTop(iobj)); }
    };
};

#endif
