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
#ifndef sLib_core_algo_h
#define sLib_core_algo_h

#include <slib/core/mex.hpp>

namespace slib
{



    class sAlgo
    {

    public:

        static void * algo_alloc(sMex * mex, idx size, void * ptr ){
            if(mex) {
                if(size!=0) return mex->ptr(mex->add(0,size)) ;
                return 0;
            }
            return sNew(size,ptr);
        }
        static void * algo_ptr(sMex * mex, idx ofs) {
            if(mex) return mex->ptr(ofs);
            return sConvInt2Ptr(ofs, void);
        }
        static const void * algo_ptr(const sMex * mex, idx ofs) {
            if(mex) return mex->ptr(ofs);
            return sConvInt2Ptr(ofs, void);
        }

    public:
        struct lix_Hdr{idx _count, _total, _bits;};
        typedef lix_Hdr * lix;
        typedef const lix_Hdr * constlix;
        static idx lix_hdrsize;
        static lix emptyLix(void) { return sConvInt2Ptr(sNotIdx,sAlgo::lix_Hdr); }

        static idx * lix_deref(idx index, constlix hdr);
        static void * lix_add(sMex * mex, idx cntAdd, idx objsize, lix * phdr, idx bits);
        template <class TObj> static TObj * lix_add(sMex * mex, idx cntAdd, lix * phdr, idx bits){return (TObj * )lix_add(mex, cntAdd, sizeof(TObj), phdr, bits); }
        static void lix_unlink(sMex * mex, idx cntDel, lix * phdr);
        static void * lix_ptr(const sMex * mex , idx index, idx objsize, constlix hdr);
        template <class TObj> static TObj * lix_ptr(sMex * mex, idx index, constlix hdr){return (TObj * )lix_ptr(mex, index, sizeof(TObj), hdr); }
        static idx lix_cnt(const sMex * mex, constlix hdr);


    public:


    public:

        struct hax_id { idx _idx, _id, _len, _rev;};
        struct hax_hdr {idx _cnt, _bits, _rescollisions, _collisionReducer;};
        typedef idx (*hax_hashFunc)(const void * mem, idx len, idx bits, idx iNum, idx seed );


        static idx hax_hashfun(const void * mem, idx len, idx bits, idx iNum);
        static idx algo_murmurHash64 ( const void * key, idx len, idx bits, idx iNum, idx seed = 13 );
        static idx hax_find(const sMex * mex, const void * id, idx lenId, idx slotType, idx * pHash, constlix hdr);
        static idx hax_map(sMex * mex, idx index, idx id,idx lenId, idx * pFnd, lix * phdr , idx bits,idx  collisionReducer, idx hax_lix_bits);
        static void hax_unmap(sMex * mex, const void * id, idx lenId, constlix hdr);
        static void hax_unmap(sMex * mex, idx posDel, idx cntDel, lix * phdr);
        static void hax_rehash(sMex * mex,  idx howmany, constlix hdr );
        static void hax_shiftidx(sMex * mex, idx posDel, idx cntDel, constlix hdr );
        static const void * hax_identifier(const sMex * mex, idx iidx, idx * pLen, constlix hdr) ;
        static void * hax_identifier(sMex * mex, idx iids, idx * pLen, constlix hdr) { return const_cast<void *>(hax_identifier(const_cast<const sMex *>(mex), iids, pLen, hdr)); }

        static void lilist_add_base(sMex * mex, idx * baseofs, idx value);
        static idx lilist_add_item(sMex * mex, idx * baseofs, idx value, bool reversed=false, idx insIndex=sNotIdx);
        static idx lilist_next_item(sMex * mex, idx * pindex, bool jumpLast=false, idx cnt=1);
        static idx * lilist_pcnt_items(sMex * mex, idx baseofs){return ((idx*)mex->ptr(baseofs)+2);}
        static idx lilist_find_items(sMex * mex, idx * baseofs, idx * idexes, idx cntidexes, idx * pindexes ,bool jumpLast);
        static idx lilist_shift_items(sMex * mex, idx * baseofs, idx * start, idx * shift, bool jumpLast, idx * buf=0);
        static void lilist_delete_items(sMex *mex, idx * pindex, idx start, idx cnt);
        static udx * lilist_set_item(sMex * mex, idx * pindex, idx value);



        static const char * sizeHuman(real * sz);

    };

}


#endif 










