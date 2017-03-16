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
#include <slib/core/hash.hpp>
#include <slib/core/algo.hpp>

using namespace slib;

#define HASHNONE        (0)
#define HASHID          HASHNONE
#define HASHAVAILABLE   ((idx)0xFFFFFFFF)


//idx sHash::hashfun(idx typeindex, const void * mem, idx len, idx bits, idx iNum)
idx sHash::hashfun( idx typeindex, const void * key, idx len, idx bits, idx iNum )
// algo_murmurHash64
{

    udx seed=(udx)typeindex;
    // 64-bit hash for 64-bit platforms

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

        // compress down to given number of bits
        idx j,x=h,i;
        for (j = x, i = bits ; i < (idx)(sizeof(idx)) ; i += bits)j ^= (x >> i) ;
        j &= (((idx)1) << bits) - 1 ;

        if(iNum)j|=1;

        return (idx)j;
    }


}

idx sHash::find(idx typeindex, const void * key, idx lenKey, idx slotType, idx * pHash, idx * bucketStart)
{
    if(!hashPtr)return 0;
    HashHeader * h0=(HashHeader * )hashPtr+(0);

    idx dh=0,h=hashfun(typeindex, key,lenKey,h0->bits,0);
    idx iidx =0;

    HashRecord * hd;
    for(;;)
    {
        hd=hashPtr+(h+1);

        iidx = hd->recordindex;
        if (iidx==HASHNONE) // never occupied hash slot
            break;
        if(iidx==HASHAVAILABLE ) { // deleted item
            if(slotType==HASHAVAILABLE ) // and we are searching for available slots.
                break;
        } else if(slotType==HASHID) { // not a deleted item check for identity

            //idx bodysize=0;
            //void * keyid=sVioDB::Getbody(hd->typeindex, hd->recordindex, &bodysize);
            idx keyIdx, keysize;
            if(bucketContainer)keyIdx=*sAlgo::lix_ptr<idx>(bucketContainer,0,(sAlgo::lix )(hd->recordindex));
            else keyIdx=hd->recordindex-1;
            void* keyid=keyfunc(keyParam,hd->typeindex, keyIdx, &keysize);

            if( hd->typeindex==typeindex && lenKey==keysize && ( keyid==key || (!memcmp(keyid,key,lenKey)) ) )
                break;
        }

        // collision
        if(!dh)dh = hashfun(typeindex, key,lenKey,h0->bits,1); // double hashing resolution of collisions
        h += dh;
        if(h >= (idx)(((idx)1)<<(h0->bits)) ) // over the limits ? rebound !
            h -=(idx)(((idx)1)<<(h0->bits)) ;
    }

    if(pHash)
        *pHash=h;
    if(bucketStart)
        *bucketStart=hd->recordindex;
    return iidx;
}

// adds the <index>-th item <key><lenKey> into the hash. returns 0 if the item is already in
// otherwise returns index+1. Also initializes pFnd by the hash slot number for that key
idx sHash::map(idx recordindex, idx typeindex, const void * key,idx lenKey, idx * pFnd )
{
    idx  h=0, oldbits,iidx=0,bits=defaultInitialBitness;

    HashHeader * h0=0;
    if ( hashPtr ) {
        h0=(HashHeader * )hashPtr+(0);

        iidx=find( typeindex, key, lenKey, HASHID, &h);
        if ( iidx ) {// is it in the dictionary

            if(bucketContainer) {
                HashRecord * hd=hashPtr+(h+1);
                idx * pref=sAlgo::lix_add<idx>(bucketContainer,1,(sAlgo::lix *)(&hd->recordindex),bucket_lstBits);
                *pref=recordindex;
            }
            if(pFnd)  // yes it is ... return
                *pFnd=iidx-1;
            return 0; // already the item is inside
        }
        for( oldbits=h0->bits; (idx) (((idx)1)<<(h0->bits)) <= h0->mapCount*collisionReducer; ++(h0->bits));// compute the new _bit-ness of the hash table neccessary to hold so much
        bits=h0->bits;
    }else {
        bits=defaultInitialBitness;
        oldbits=0;
    }



    // see if hash table needs reallocation
    if( bits != oldbits ) // in case if the size was changed : reserve enough space for dictionary and rehash all
    {
        //if(hashTable.dim() && oldbits)
        rehash( bits ); // rehash all the previous items
        h=0;
    }

    // find an available spot in the hash table and add the item into it
    if(!h)
        find(typeindex,key, lenKey, HASHAVAILABLE, &h);

    HashRecord * hd=hashPtr+(h+1);
    if(bucketContainer) {
        hd->recordindex=(idx)sAlgo::emptyLix();
        *sAlgo::lix_add<idx>(bucketContainer,1,(sAlgo::lix *)(&hd->recordindex),bucket_lstBits)=recordindex;
    }
    else hd->recordindex=recordindex+1;
    hd->typeindex=typeindex;

    h0=(HashHeader * )hashPtr+(0);
    ++h0->mapCount;
    if(pFnd)  // return
        *pFnd=recordindex;

    //_doCollisions=0;
    return recordindex+1;
}


// rehashes items in the dictionary
void sHash::rehash(idx bits)
{
    sVec < HashRecord > tmpTable;

    for(idx i=1 ; i<hashTableVec.dim(); ++i )  // scan all items but the header
    {
        HashRecord * hd=hashPtr+(i);
        if(!hd->recordindex)
            continue;
        *tmpTable.add(1)=*hd;
    }

    idx mapCount=hashPtr ? ((HashHeader*)(hashPtr+(0)))->mapCount : 0 ;
    hashTableVec.resize((((idx)1)<<bits)+1);
    hashTableVec.set(0);
    hashPtr=hashTableVec.ptr();
    HashHeader * h0=(HashHeader * )hashPtr+(0);
    h0->mapCount=mapCount ;
    h0->bits=bits;


    // rehash to the reserv
    for(idx i=0 ; i<tmpTable.dim(); ++i )  // scan all items
    {
        HashRecord * hd=tmpTable.ptr(i);
        //if(hd->recordindex<0)
        //    continue;

        idx keysize,keyIdx;
        if(bucketContainer)keyIdx=*sAlgo::lix_ptr<idx>(bucketContainer,0,(sAlgo::lix )(hd->recordindex));
        else keyIdx=hd->recordindex-1;
        void* keyid=keyfunc(keyParam,hd->typeindex, keyIdx, &keysize);
        //map(hd->recordindex, hd->typeindex, keyid, keysize, 0);

        idx h;
        find( hd->typeindex, keyid, keysize, HASHID, &h);
        HashRecord * hn=hashPtr+(h+1);
        hn->recordindex=hd->recordindex;
        hn->typeindex=hd->typeindex;


    }
}



