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
#include <slib/core/hash2.hpp>
#include <slib/core/algo.hpp>

using namespace slib;

#define HASHNONE        (0)
#define HASHID          HASHNONE
#define HASHAVAILABLE   ((idx)0xFFFFFFFF)

idx sHash2::hashfun( hashtypeIdx typeindex, const void * key, idx len, idx bits, idx iNum )
{

    udx seed=(udx)typeindex;

    {
        const udx m = 0xc6a4a7935bd1e995;
        const idx r = 47;

        udx h = seed ^ (len * m);

        const udx * data = (const udx *)key;
        const udx * end = data + (len/8);

        while(data != end)
        {
            udx k = *data++;

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        const unsigned char * data2 = (const unsigned char*)data;

        switch(len & 7)
        {
        case 7: h ^= udx(data2[6]) << 48;
        case 6: h ^= udx(data2[5]) << 40;

        case 5: h ^= udx(data2[4]) << 32;
        case 4: h ^= udx(data2[3]) << 24;
        case 3: h ^= udx(data2[2]) << 16;
        case 2: h ^= udx(data2[1]) << 8;
        case 1: h ^= udx(data2[0]);
                h *= m;
        };

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        idx j,x=h,i;
        for (j = x, i = bits ; i < (idx)(sizeof(idx)) ; i += bits)j ^= (x >> i) ;
        j &= (((idx)1) << bits) - 1 ;

        if(iNum)j|=1;

        return (idx)j;
    }


}


idx sHash2::find(HashRecord * hashPtrLocal, hashtypeIdx typeindex, const void * key, idx lenKey, idx slotType, idx * pHash)
{
    if(!hashPtrLocal)return 0;
    HashHeader * h0=(HashHeader * )hashPtrLocal+(0);

    idx dh=0,h=hashfun(typeindex, key,lenKey,h0->bits(),0);
    idx iidx =0;

    HashRecord * hd;
    for(;;)
    {
        hd=hashPtrLocal+(h+1);

        iidx = hd->recordindex();
        if (iidx==HASHNONE)
            break;
        if(iidx==HASHAVAILABLE ) {
            if(slotType==HASHAVAILABLE )
                break;
        } else if(slotType==HASHID) {

            idx keyIdx, keysize;
            void* keyid;
            if(bucketContainer && (hd->recordindex()&(sHash2_idxFilterBit)) ) {
                idx baseOfs1Based=((hd->recordindex())&(sHash2_idxFilterMask))-1;
                keyIdx=(*(idx*)bucketContainer->ptr(baseOfs1Based))&(sHash2_idxFilterMask);
                do {
                    keyid=keyfunc(keyParam,hd->typeindex(), keyIdx, &keysize,&keyBody);
                    if(keysize)break;
                    keyIdx=sAlgo::lilist_next_item(bucketContainer,&baseOfs1Based,reversed);
                }while(keysize==0);
            }
            else {
                keyIdx=hd->recordindex()-1;
                keyid=keyfunc(keyParam,hd->typeindex(), keyIdx, &keysize,&keyBody);
            }


            if(keyid==sNotPtr)keyid=(&keyIdx);

            keyIdx&=~(sIdxHighBit);
            if( hd->typeindex()==typeindex && lenKey==keysize && ( keyid==key || (!memcmp(keyid,key,lenKey)) ) )
                break;
        }

        if(!dh)dh = hashfun(typeindex, key,lenKey,h0->bits(),1);
        h += dh;
        if(h >= (idx)(((idx)1)<<(h0->bits())) )
            h -=(idx)(((idx)1)<<(h0->bits())) ;
    }

    if(pHash)
        *pHash=h;
    return iidx;
}


idx sHash2::map(idx recordindex, hashtypeIdx typeindex, const void * key,idx lenKey, idx * pFnd , idx insIndex)
{

    idx  h=0, oldbits,iidx=0,bits=defaultInitialBitness;

    HashHeader * h0=0;
    if ( hashPtr ) {
        h0=(HashHeader * )hashPtr+(0);

        iidx=find( hashPtr, typeindex, key, lenKey, HASHID, &h);
        if ( iidx ) {

            if(bucketContainer) {
                HashRecord * hd=hashPtr+(h+1);


                    if(!bucketContainer->pos()) {
                       bucketContainer->add(&sMex::_zero,sizeof(sMex::_zero));
                    }

                    if( ! (hd->recordindex()&(sHash2_idxFilterBit)) ) {
                        idx baseOfs1Based;
                        sAlgo::lilist_add_base(bucketContainer, &baseOfs1Based, hd->recordindex()-1);
                        hd->recordindexSet((baseOfs1Based+1)|sHash2_idxFilterBit);
                    }
                    {
                        idx baseOfs1Based=((hd->recordindex())&(sHash2_idxFilterMask))-1;
                        sAlgo::lilist_add_item(bucketContainer, &baseOfs1Based, recordindex, reversed,insIndex);
                        if(insIndex!=sNotIdx){
                            idx pIns=insIndex,pShift=1;
                            idx buf[1];
                            sAlgo::lilist_shift_items(bucketContainer,&baseOfs1Based, &pIns, &pShift, reversed,buf);
                            sAlgo::lilist_set_item(bucketContainer, &pIns, recordindex);
                        }
                        hd->recordindexSet((baseOfs1Based+1)|sHash2_idxFilterBit);
                    }
           }
            if(pFnd)
                *pFnd=iidx-1;
            return 0;
        }
        for( oldbits=h0->bits(); (idx) (((idx)1)<<(h0->bits())) <= h0->mapCount()*collisionReducer; h0->bitsSet(h0->bits()+1));
        bits=h0->bits();
    }else {
        bits=defaultInitialBitness;
        oldbits=0;
    }



    if( bits != oldbits )
    {
        rehash( bits );
        h=0;
    }

    if(!h)
        find(hashPtr, typeindex,key, lenKey, HASHAVAILABLE, &h);

    HashRecord * hd=hashPtr+(h+1);
    hd->recordindexSet(recordindex+1);
    hd->typeindexSet(typeindex);

    h0=(HashHeader * )hashPtr+(0);
    h0->mapCountSet(h0->mapCount()+1);
    if(pFnd)
        *pFnd=recordindex;

    return recordindex+1;
}



void sHash2::unmap( idx hash )
{

    HashRecord * hd=hashPtr+(hash+1);
    hd->_val=(HASHAVAILABLE);
}


void sHash2::rehash(idx bits)
{
    sVec < HashRecord > tmpTable;
    sMex keyBody(sMex::fExactSize);

    for(idx i=1 ; i<hashTableVec.dim(); ++i )
    {
        HashRecord * hd=hashPtr+(i);
        if(!hd->recordindex())
            continue;
        *tmpTable.add(1)=*hd;
    }

    idx mapCount=hashPtr ? ((HashHeader*)(hashPtr+(0)))->mapCount() : 0 ;
    hashTableVec.resizeM((((idx)1)<<bits)+1);
    hashTableVec.set(0);
    hashPtr=hashTableVec.ptr();
    HashHeader * h0=(HashHeader * )hashPtr+(0);
    h0->mapCountSet(mapCount);
    h0->bitsSet(bits);


    for(idx i=0 ; i<tmpTable.dim(); ++i )
    {
        HashRecord * hd=tmpTable.ptr(i);

        idx keysize,keyIdx;
        if(bucketContainer && hd->recordindex()&(sHash2_idxFilterBit) ) keyIdx=*(idx*)bucketContainer->ptr(((hd->recordindex())&(sHash2_idxFilterMask))-1);
        else keyIdx=hd->recordindex()-1;
        keyIdx&=~(sIdxHighBit);

        void* keyid=keyfunc(keyParam,hd->typeindex(), keyIdx, &keysize,&keyBody);
        if(keyid==sNotPtr)keyid=(&keyIdx);

        idx h;
        find(hashPtr, hd->typeindex(), keyid, keysize, HASHID, &h);
        HashRecord * hn=hashPtr+(h+1);
        hn->recordindexSet(hd->recordindex());
        hn->typeindexSet(hd->typeindex());


    }
}
