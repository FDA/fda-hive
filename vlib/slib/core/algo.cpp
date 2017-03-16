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

#include <slib/core/algo.hpp>

using namespace slib;

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Lix container algorithms
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

idx sAlgo::lix_hdrsize=sizeof(sAlgo::lix_Hdr);

#define phdr_save idx phdrofsinside=(mex && phdr>=mex->ptr() && phdr< mex->ptr(mex->pos()) ) ? (idx)((char *)(phdr)-(char*)(mex->ptr())) : sNotIdx ;  // if phdr itself is inside of the mex
#define phdr_restore    if(phdrofsinside!=sNotIdx) phdr=(lix *)mex->ptr(phdrofsinside); // put back the pointer to lix header

// IMPORTANT : this is highly optimized code. Do not modify without considering performance issues
idx * sAlgo::lix_deref(idx index, constlix hdr) // returns pointer to the block which contains this index
{
    //idx * buf0=(idx * )(hdr+1);
    idx  * buf=&sAlgo::lix_hdrsize;
    idx mask = (1<<(hdr->_bits))-1;

    if( (index&(~mask)) ) {
        idx orderBit;
        for ( orderBit=0 ; (index>>orderBit)>mask ;  orderBit+=(hdr->_bits)) ;
        for ( ;  orderBit>0 ; orderBit-=(hdr->_bits) )
            buf = (idx*)( ((char *)hdr) + ( *buf ) + sizeof(idx)*((index>>orderBit)&mask)  );
            //buf = (_buf0+ ( *buf ) + ((index>>orderBit)&_mask)  );
    }
    return buf;
}

void * sAlgo::lix_ptr(const sMex * mex, idx index, idx objsize, constlix hdr)
{
    if(hdr==emptyLix())return 0;
    if(mex) hdr=( lix ) mex->ptr( sConvPtr2Int(hdr) );

    return (void *)(
                (char *)(hdr)+
                *lix_deref(index,hdr)+
                (sizeof(idx)<<(hdr->_bits))+sizeof(idx)+(index&((1<<(hdr->_bits))-1))*objsize
                );
}

idx sAlgo::lix_cnt(const sMex * mex, constlix hdr)
{
    if(hdr==emptyLix() )return 0;
    if(mex) hdr=( lix ) mex->ptr( sConvPtr2Int(hdr) );
    return hdr->_count;
}

void * sAlgo::lix_add(sMex * mex, idx cntAdd, idx objsize, lix * phdr, idx bits) // request element(s) , return  pointer and the index
{
    idx newblocks, curtot;

    lix hdr = *phdr;
    if(mex && hdr!=emptyLix()) hdr=( lix ) mex->ptr( sConvPtr2Int(hdr) );

    if( hdr==emptyLix() ) { // the very first allocation ?
        newblocks=cntAdd;
        curtot=0;
    }else {
        bits=hdr->_bits;
        newblocks=hdr->_count+cntAdd-hdr->_total;
        curtot=hdr->_count;
        if(newblocks<=0){hdr->_count+=cntAdd; return lix_ptr(mex, curtot, objsize, *phdr );}
    }

    idx blk=1<<bits;
    newblocks=((newblocks-1)>>bits)+1;

    // the current allocated blocks do not have enough space for accomodating requested items
    // perform allocation of the memory for the necessary number of blocks
    idx block=((objsize+sizeof(idx))<<bits)+sizeof(idx); // space for linking idx-es and for block flag
    idx sz=newblocks*block + (hdr==emptyLix() ? sizeof(lix_Hdr) : 0);
        phdr_save //idx phdrofsinside=(mex && phdr>=mex->ptr() && phdr< mex->ptr(mex->pos()) ) ? (idx)((char *)(phdr)-(char*)(mex->ptr())) : sNotIdx ;  // if phdr itself is inside of the mex
    idx * buf= (idx *)algo_alloc(mex, sz,0); // the very first block has a header
        phdr_restore //if(phdrofsinside!=sNotIdx) phdr=(lix *)mex->ptr(phdrofsinside); // put back the pointer to lix header
    if(mex && hdr!=emptyLix() ) hdr=( lix ) mex->ptr( sConvPtr2Int(*phdr) ); // the previous allocation may have reallocated the mex buffer - so we get it again.
    memset(buf,0,sz);  // we zerofy the memory for the block linking _service part
    if(hdr==emptyLix()){
        hdr=(lix)buf;
        idx ofs=mex ? (char*)hdr-(char*)(mex->ptr()) : 0;
        *phdr = mex ? sConvInt2Ptr(ofs, lix_Hdr) : hdr ;
        hdr->_bits=bits;
        hdr->_total=0;
        hdr->_count=0;
        buf=(idx *)((char * )buf+sizeof(lix_Hdr));
    }
    buf[((idx)1)<<(hdr->_bits)]=1; // end of service block flag tells that this block was allocated independently and needs to be deleted during destructions

    // attach/link newly allocated blocks together with the existing blocks
    for ( ; ; )  {
        //memset(buf,0,sizeof(idx)<<(hdr->_bits));  // we zerofy the memory for the block linking _service part

        if(hdr->_total)
            *lix_deref( hdr->_total , hdr ) = (idx)((char*)buf-(char*)hdr) ; // attach the block to the corresponding derefernced block address

        hdr->_total+=(((idx)1)<<(hdr->_bits)); // increase the number of available items
        if( hdr->_total>=cntAdd+hdr->_count ) break; // stop if this is enough

        buf=(idx*)(((char*)buf)+block); // position to the next block
        buf[blk]=0; // this block is not allocated independedly, do not delete !
    }

    hdr->_count+=cntAdd;
    return lix_ptr(mex,curtot,objsize, (*phdr) );
}




// unlink and free blocks of items from the end
void sAlgo::lix_unlink(sMex * mex, idx cntDel, lix * phdr)
{
    lix hdr = *phdr;
    if(hdr==emptyLix())return ;
    if(mex) hdr=( lix ) mex->ptr( sConvPtr2Int(hdr) );

    idx blk=1<<(hdr->_bits);
    if(cntDel==sNotIdx)
        cntDel=hdr->_count;

    for ( ; hdr->_total>=hdr->_count-cntDel+blk;  ) {
        idx * buf = (idx*)((char*)hdr + *lix_deref(hdr->_total-1, hdr ) );
        hdr->_total-=blk; // mark so much as already removed
        if (buf[blk]) {  // this was allocated separately, not in a bunch with other blocks
            if(hdr->_total>0){  // we do not delete the very last one here
                    phdr_save
                algo_alloc(mex, 0, buf);
                    phdr_restore
            }
        }
        //alloc(0, sConvInt2Ptr( buf, void ) );
    }

    if(hdr->_count>cntDel)
        hdr->_count-=cntDel;
    else {
        hdr->_count=0;
            phdr_save
        algo_alloc(mex, 0,hdr);
            phdr_restore
        (*phdr)=emptyLix();
    }
    return ;
}







// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Hax container algorithms
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/



#define HASHNONE        (0)
#define HASHID            HASHNONE
#define HASHAVAILABLE    ((idx)0xFFFFFFFFFFFFFFFF)


// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Lix container algorithms
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// searches the hash table for a given <id><lenId> for a slot of a given type
// returns the ordinal of the cell+1 and initializes *pHash by the value of the hash

#ifdef _DEBUG
idx globalCollisionsCounter=0;
#endif

idx sAlgo::hax_find(const sMex * mex, const void * id, idx lenId, idx slotType, idx * pHash, constlix hdr)
{
    if(hdr==emptyLix())return 0;
    hax_hdr * hd=(hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), hdr) ;
    idx dh=0,h=hax_hashfun(id,lenId,hd->_bits,0);

    idx iidx =0;


    for(;;)
    {
        hax_id * clh=(hax_id *)lix_ptr(mex, (h+1) , sizeof(hax_id), hdr) ;

        iidx = clh->_idx;
        if (iidx==HASHNONE) // never occupied hash slot
            break;
        if(iidx==HASHAVAILABLE ) // delted item
        {
            if(slotType==HASHAVAILABLE ) // and we are searching for available slots.
                break;
        }
        else if(slotType==HASHID) // not a deleted item check for identity
        {
            const void * didid=algo_ptr(mex, clh->_id);

            /*if( _difffun ){
                if( !_difffun(didid,clh->_len, id,lenId)  )
                    break;
            }
            else */if( clh->_len==lenId && ( didid==id || (!memcmp(didid,id,lenId)) )  )
                break;
        }

        // collision
        if(!dh)dh = hax_hashfun(id,lenId,hd->_bits,1 ); // double hashing resolution of collisions
        h += dh;
        // h++; // linear collision resolution
        if(h >= (idx)(1<<(hd->_bits)) ) // over the limits ? rebound !
            h -=(idx)(1<<(hd->_bits)) ;
        #ifdef _DEBUG
            ++globalCollisionsCounter;
        #endif
        //if(_doCollisions)
  //          hd->_collisions++; // one more collision
    }

    if(pHash)
        *pHash=h;
    return iidx;
}



// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  Lix container algorithms
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

// returns bits limited hash of the id in the memory <mem> of length <len>
// if iNum uses a different key and makes an odd number
idx sAlgo::hax_hashfun(const void * mem, idx len, idx bits, idx iNum )
{
    return algo_murmurHash64 ( mem, len, bits, iNum, 13 );
    /*
    idx i ;
    idx j, x = 0 ;
    idx rotate = iNum ? 21 : 13 ;
    idx leftover = 8*sizeof(idx) - rotate ;
    char * src=(char * )mem;

    // preparing the hash
    for(i=0;i<len;i++)
        x = src[i] ^ (( x >> leftover) | (x << rotate)) ;

    // compress down to given number of bits
    for (j = x, i = bits ; i < (idx)(sizeof(idx)) ; i += bits)j ^= (x >> i) ;
    j &= (1 << bits) - 1 ;

    if(iNum)j|=1;
    return j ;
    */
}

// 64-bit hash for 64-bit platforms
idx sAlgo::algo_murmurHash64 ( const void * key, idx len, idx bits, idx iNum, idx seed )
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
    /* no break */
    case 6: h ^= udx(data2[5]) << 40;
    /* no break */
    case 5: h ^= udx(data2[4]) << 32;
    /* no break */
    case 4: h ^= udx(data2[3]) << 24;
    /* no break */
    case 3: h ^= udx(data2[2]) << 16;
    /* no break */
    case 2: h ^= udx(data2[1]) << 8;
    /* no break */
    case 1: h ^= udx(data2[0]);
            h *= m;
            /* no break */
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




// adds the <index>-th item <id><lenID> into the dictionary. returns 0 if the item is already in
// otherwise returns index+1. Also initializes pFnd by the hash slot number for that id
idx sAlgo::hax_map(sMex * mex, idx index, idx id,idx lenId, idx * pFnd, lix * phdr , idx bits, idx collisionReducer,idx hax_lix_bits)
{
    hax_hdr * hd= (hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), (*phdr)) ;
    hax_id * clh;

    idx  h=0, oldbits,iidx=0;

    if ( hd ) {
        if( hd->_cnt) // try to find if this item is already in the dictionary
            iidx=hax_find( mex, algo_ptr(mex, id), lenId, HASHID, &h, (*phdr));
        if ( iidx ) {// is it in the dictionary
            if(pFnd)  // yes it is ... return
                *pFnd=iidx-1;
            // map to the newest location
            clh=(hax_id *)lix_ptr(mex, (index+1) , sizeof(hax_id), (*phdr) ) ;
            clh->_rev=h;

            return 0; // already the item is inside
        }
        bits=hd->_bits;
        for( oldbits=bits; (idx) (1<<bits) <= hd->_cnt*hd->_collisionReducer; ++bits);// compute the new _bit-ness of the hash table neccessary to hold so much
    }else {
        oldbits=0;
    }

    // see if hash table needs reallocation
    if( bits != oldbits ) // in case if the size was changed : reserve enough space for dictionary and rehash all
    {
        idx oldcnt=hd ? hd->_cnt : 0;
        iidx=lix_cnt(mex,(*phdr));
        idx memsize=1+(1<< bits);  // +1 is for hax_hdr
            phdr_save
        lix_add(mex, memsize-iidx, sizeof(hax_id), phdr, hax_lix_bits); // request element(s) , return  pointer and the index
            phdr_restore

        hd=(hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), (*phdr)) ;
        hd->_cnt=oldcnt;
        hd->_bits=bits;
        hd->_collisionReducer=collisionReducer;

        if(hd->_cnt && oldbits)hax_rehash( mex, (1<<oldbits),(*phdr) ); // rehash all the previous items
        h=0;
    }

    // find an available spot in the hash table and add the item into it
    if(!h)
        hax_find(mex,algo_ptr(mex,id), lenId, HASHAVAILABLE, &h, (*phdr));
    clh=(hax_id *)lix_ptr(mex, h+1, sizeof(hax_id), (*phdr)) ;
    clh->_idx=index+1; // remember where this hash points to
    clh->_id=id;
    clh->_len=lenId;
    clh=(hax_id *)lix_ptr(mex, index+1, sizeof(hax_id), (*phdr)) ;
    clh->_rev=h;
    ++(hd->_cnt);//(*_phaxcnt()); // one more item in dictionary

    if(pFnd)  // return
        *pFnd=index;

    //_doCollisions=0;
    return index+1;
}

// delete <cntDel> items from dictionary starting from <posDel> position
void sAlgo::hax_unmap(sMex * mex, const void * id, idx lenId, constlix hdr)
{
    //sHaxHead * hd=_hdptr();
    hax_hdr * hd=(hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), hdr) ;
    if(!hd )return;// if(!_hd) return;

    idx h;
    if( !hax_find(mex, id, lenId, HASHID, &h, hdr ) )
        return ;
    //sHaxId * cl=_clptr();
    hax_id * clh=(hax_id *)lix_ptr(mex, h+1, sizeof(hax_id), hdr) ;
    idx posDel=clh->_idx;
    clh->_idx=(HASHAVAILABLE); // declare their hash spots available
    clh->_len=0; // remove id not to be copied later
    --(hd->_cnt); // decrease the number of hashed items

    hax_shiftidx(mex,posDel, 1, hdr);
}

void sAlgo::hax_unmap(sMex * mex, idx posDel, idx cntDel, lix * phdr)
{
    hax_hdr * hd=(hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), (*phdr) ) ;
    if(!hd) return ;

    //sHaxId * cl=_clptr();

    if(posDel==0 && cntDel==hd->_cnt ){
            phdr_save
        lix_unlink(mex, sNotIdx , phdr);
            phdr_restore
    }
    else if(hd->_cnt){ // not an empty dictionary
        idx h;
        for(idx i=0 ; i<cntDel; ++i )
        {
            hax_id * clh=(hax_id *)lix_ptr(mex, i+posDel+1, sizeof(hax_id), (*phdr)) ;
            h=clh->_rev;//cl[i+posDel]._rev;
            clh=(hax_id *)lix_ptr(mex, h+1, sizeof(hax_id), (*phdr)) ;
            if( hax_find(  mex, algo_ptr(mex,clh->_id) , clh->_len, HASHID, &h, (*phdr)) ){ // free their allocated ids
                clh=(hax_id *)lix_ptr(mex, h+1, sizeof(hax_id), (*phdr)) ;
                clh->_idx=(HASHAVAILABLE); // declare their hash spots available
                clh->_len=0; // remove id not to be copied later
                --(hd->_cnt); // decrease the number of hashed items
            }
        }
    }

    hax_shiftidx(mex,posDel, cntDel, *phdr);
}


void sAlgo::hax_shiftidx(sMex * mex, idx posDel, idx cntDel, constlix hdr )
{
    hax_hdr * hd=(hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), hdr ) ;

    idx howmany=(1<<(hd->_bits));
    hax_id * clh;
    for(idx i=0 ; i<howmany; ++i )  {
        clh=(hax_id *)lix_ptr(mex, i+1, sizeof(hax_id), hdr );
        if(clh->_idx>posDel+cntDel) {
            clh->_idx-=cntDel;
        }

   }

    for(idx i=posDel+cntDel; i<hd->_cnt; ++i )  {
        clh=(hax_id *)lix_ptr(mex, i+1, sizeof(hax_id), hdr );
        hax_id * clr = (hax_id *)lix_ptr(mex, i+1-cntDel, sizeof(hax_id), hdr );
        clr->_rev=clh->_rev;
   }

}



// rehashes toHash items in the dictionary
void sAlgo::hax_rehash(sMex * mex,  idx howmany, constlix hdr )
{

    hax_hdr * hd=(hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), hdr ) ;
    idx i,h;

    if(!howmany)
        howmany=(1<<(hd->_bits));

    /// copy the hash table into temporary location
    hax_id * _cr=(hax_id * ) sNew ( sizeof(hax_id)*(((idx)1)<<(hd->_bits)) );
    hax_id * clh;
    for(i=0 ; i<howmany; ++i )  {
        clh=(hax_id *)lix_ptr(mex, i+1, sizeof(hax_id), hdr );
        _cr[i]=*clh;
        memset((void*)clh,0,sizeof(hax_id));
    }

    // rehash to the reserv
    for(i=0 ; i<howmany; ++i )  // scan all items
    {
        hax_id * cr=_cr+i;
        if(cr->_idx==HASHNONE || cr->_idx==HASHAVAILABLE)continue; // rehash only hashed cells

        hax_find( mex, algo_ptr(mex,cr->_id) , cr->_len, HASHID, &h , hdr) ; // lookup for hashspots of this item

        clh=(hax_id *)lix_ptr(mex, h+1, sizeof(hax_id), hdr );

        clh->_idx=cr->_idx; // set the hashes in the reserv part
        clh->_id=cr->_id;
        clh->_len=cr->_len;
        if( _cr[cr->_idx-1]._rev==i){ // here was where reverse hash was pointing
            //cl[ci->_idx-1]._rev=h;
            clh=(hax_id *)lix_ptr(mex, clh->_idx-1+1, sizeof(hax_id), hdr );
            clh->_rev=h;
        }
     }

    #ifdef _DEBUG
        globalCollisionsCounter=0;
    #endif
    sDel(_cr);
}


const void * sAlgo::hax_identifier(const sMex * mex, idx iidx, idx * pLen, constlix hdr)
{
    hax_hdr * hd=(hax_hdr *)lix_ptr(mex, 0, sizeof(hax_id), hdr ) ;
    if(!hd)return 0;

    hax_id * clh=(hax_id *)lix_ptr(mex, iidx+1, sizeof(hax_id), hdr) ;
    idx h=clh->_rev;
    clh=(hax_id *)lix_ptr(mex, h+1, sizeof(hax_id), hdr) ;

    if(pLen)*pLen=clh->_len;
    return  algo_ptr( mex, clh->_id );
}





// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/  optimized linked list container algorithms
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void sAlgo::lilist_add_base(sMex * mex, idx * baseofs, idx value)
{
    udx pos=mex->add(0,4*sizeof(idx));
    udx * base=(udx*)mex->ptr(pos);

    base[0]=value|sIdxHighBit; // this value
    base[1]=sNotIdx; // the next element
    base[2]=(udx)1; // count
    base[3]=pos; // the last element

    *baseofs=pos;

}


idx sAlgo::lilist_add_item(sMex * mex, idx * baseofs, idx value, bool reversed, idx insIndex)
{
    bool is64=false;
    if( value>(idx)0x7FFFFFFF || mex->pos()+3*sizeof(idx) >= 0x7FFFFFFF ) // we can't use the highest bit here because that bit switches 32->64 bitness
        is64=true;

    udx * base=(udx*)mex->ptr(*baseofs);
    udx lastpos=base[3];
    udx * last=(udx*)mex->ptr(lastpos);

    if(!(last[0]&sIdxHighBit)) { // this was in 32 bit mode now it must link to 64 bit mode
        if(is64) { // turn it into 64
            int val=*(int*)last;
            lastpos=mex->add(0,2*sizeof(udx));
            base=(udx*)mex->ptr(*baseofs);
            last = (udx*)mex->ptr(base[3]);
            *last=lastpos|sIdxHighBit|(sIdxHighBit>>1); // in 64 mode highest two bits for the value signify that this is not a value but rather an absolute position jump
            last=(udx * ) mex->ptr(lastpos);
            last[0]=(udx)(val|sIdxHighBit); // remember the old value and signify its 64 bitness
            base[3]=lastpos; // the last element
            //last[1]=-1;
        }
    }

    udx nextpos=mex->add(0,2*(is64 ? sizeof(udx): sizeof(int) ));
    if(is64) {
        udx * next=(udx*)mex->ptr(nextpos);
        next[0]=value|sIdxHighBit; // remember the value and signify the fact that the value is 64 bit value
        if(reversed)next[1]=lastpos; // the previous element link
        else next[1]=(udx)-1; // the next element is the last
    } else {
        unsigned int * next=(unsigned int *)mex->ptr(nextpos);
        next[0]=value; // remember the value
        if(reversed)next[1]=(unsigned int)lastpos; // the previous element link
        else next[1]=(unsigned int)0x7FFFFFFF; // the next element is the last // in 32 bit mode we can't use -1 as the last becuase of the highest bit means something else  (64 bit form)
    }

    base=(udx*)mex->ptr(*baseofs);
    last=(udx*)mex->ptr(base[3]); // the previous allocation may have altered the pointers
    idx ret=base[3];

    if(!reversed) {
        if(!(last[0]&sIdxHighBit)) { // this is still 32 bit
            ((unsigned int*)last)[1]=nextpos;
        }else
            last[1]=nextpos;
    }

    ++base[2]; // count
    base[3]=nextpos; // the last element

    return ret;

}

idx sAlgo::lilist_find_items(sMex * mex, idx * baseofs, idx * idexes, idx cntidexes, idx * pindexes ,bool jumpLast)
{
    idx pind=*baseofs, zeros;
    lilist_next_item(mex, &pind, jumpLast);

    for( idx ie=0; pind!=sNotIdx; ++ie ) {
        zeros=0;
        for(idx ic=0; ic<cntidexes; ++ic ) {
            if(pindexes[ie]==ie)pindexes[ic]=pind;
            if(pindexes[ie]==0)++zeros;
        }
        if(!zeros)break;
        lilist_next_item(mex, &pind, jumpLast);
    }
    return pind;
}

udx * sAlgo::lilist_set_item(sMex * mex, idx * pindex, idx value)
{
    udx * base=(udx *)mex->ptr(*pindex);
    bool is64=(base[0]&sIdxHighBit) ? true : false ;
    if(is64) { base[0]= value|sIdxHighBit; }
    else { ((unsigned int* )base)[0]= (unsigned int)(value);  }
    return base;
}


void sAlgo::lilist_delete_items(sMex *mex, idx * pindex, idx start, idx cnt)
{
    start+=cnt;
    cnt=-cnt;
    sAlgo::lilist_shift_items(mex, pindex, &start, &cnt,false,0);
    *sAlgo::lilist_pcnt_items(mex, *pindex)+=cnt;

    udx * base=(udx *)mex->ptr(cnt);
    bool is64=(base[0]&sIdxHighBit) ? true : false ;
    if(is64) { base[0]= (udx)-1; }
    else { ((unsigned int* )base)[1]= 0x7FFFFFFF;  }
}


idx sAlgo::lilist_shift_items(sMex * mex, idx * baseofs, idx * start, idx * shift, bool jumpLast, idx * buf)
{
    idx pind=*baseofs;
    lilist_next_item(mex, &pind, jumpLast);
    idx pEnd=0, pStart=0;
    idx ie;
    for( ie=0; pind!=sNotIdx; ++ie ) {
        if(*start==ie)pStart=pind;
        if((*start)+(*shift)==ie)pEnd=pind;
        if(pStart && pEnd)
            break;
        lilist_next_item(mex, &pind, jumpLast);
    }
    if(!pStart || !pEnd)
        return 0;
    idx sft=*shift;
    *start=pStart;
    *shift=pEnd;


    for( ie=0; pStart!=sNotIdx && pEnd!=sNotIdx ; ++ie ) {
        idx * posStart=(idx * )mex->ptr(pStart);
        idx * posEnd=(idx * )mex->ptr(pEnd);

        bool is64Start=(posStart[0]&sIdxHighBit) ? true : false ;
        bool is64End=(posEnd[0]&sIdxHighBit) ? true : false ;
        idx valRemember;

        if(sft>0) {
            valRemember = is64End ? posEnd[0] : (udx) ((unsigned int* )posEnd)[0]; // remember the previous value
        }

        if(sft>0 && ie>=sft ){
            if(is64End) { posEnd[0]= buf[ie%sft]|sIdxHighBit; }
            else { ((unsigned int* )posEnd)[0]= (unsigned int)(buf[ie%sft]);  }
        }
        else {
            if(is64End) { posEnd[0]= ((is64Start ? posStart[0] : (udx) ((unsigned int* )posStart)[0]))|sIdxHighBit; }
            else { ((unsigned int* )posEnd)[0]=  (unsigned int)(is64Start ? posStart[0] : ((unsigned int* )posStart)[0]);  }
        }

        if(sft>0)
            buf[ie%sft]=valRemember;

        lilist_next_item(mex, &pStart, jumpLast);
        lilist_next_item(mex, &pEnd, jumpLast);
    }

    return pStart;
}


idx sAlgo::lilist_next_item(sMex * mex, idx * pindex, bool jumpLast, idx cnt)
{
    udx * base;
    bool is64=false;
    for ( idx ic=0; ic<cnt; ++ic ) {
        base=(udx *)mex->ptr(*pindex);
        is64=(base[0]&sIdxHighBit) ? true : false ;
        if(is64) {
            if( (base[0]&(((udx)sIdxHighBit)>>1)) ){ // this link doesn't carry a value but rather is a jump between 32 and 64 bits
                base=(udx*)mex->ptr(base[0]&(~(sIdxHighBit|(sIdxHighBit>>1))) ) ;
            }
            *pindex=base[jumpLast ? 3 : 1 ];
            return base[0]&(sIdxHighMask);
        }

        *pindex=((int*)base)[jumpLast ? 3 : 1 ];
        if(*pindex==0x7FFFFFFF){*pindex=sNotIdx; break;}
    }

    return is64 ? base[0] : (idx)((int*)base)[0]&(0x7FFFFFFF);
}





const char * sAlgo::sizeHuman(real * sz)
{
    idx unit=0;
    static const char * units[]={"","kB","MB","GB","TB","PB","EB"};
    for(unit=0; (*sz)>1024 ; ++unit ){
        (*sz)/=1024;
    }
    return units[unit];
}



