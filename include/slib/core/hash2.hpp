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
#ifndef sLib_hash2_h
#define sLib_hash2_h

#include <slib/core/mex.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/algo.hpp>


namespace slib
{

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ class sHash
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    class sHash2 {

    public:
            typedef short int hashtypeIdx;
            #define sHash2_idxBits (48)
            #define sHash2_idxMask ((((idx)1)<<sHash2_idxBits)-1)
            #define sHash2_highMask ((((idx)1)<<(sizeof(idx)*8-sHash2_idxBits))-1)

            #define sHash2_idxFilterBit  (((idx)1)<<(sHash2_idxBits-1))
            #define sHash2_idxFilterMask (sHash2_idxFilterBit-1)


            struct HashHeader{
            //    hashtypeIdx mapCount;
                idx _val;
                idx mapCount(void) {return _val&sHash2_idxMask;}
                idx mapCountSet(idx lval) {return _val=(_val&(~sHash2_idxMask))|lval;}

                idx bits(void) {return (_val>>sHash2_idxBits)&sHash2_highMask;}
                idx bitsSet(idx lval) {return _val=(_val&(sHash2_idxMask))|(lval<<sHash2_idxBits);}
            };

            struct HashRecord{
                idx _val;
                idx recordindex(void) {return _val&sHash2_idxMask;}
                idx recordindexSet(idx lval) {return _val=(_val&(~sHash2_idxMask))|lval;}

                idx typeindex(void) {return (_val>>sHash2_idxBits)&sHash2_highMask;}
                idx typeindexSet(idx lval) {return _val=(_val&(sHash2_idxMask))|(lval<<sHash2_idxBits);}

            };




    public:
        sMex * bucketContainer;
        sMex baseFileName;
        sMex keyBody;

        sVec < HashRecord > hashTableVec;
        HashRecord * hashPtr;
        idx collisionReducer;//,mapCount;
        const static idx defaultInitialBitness=4;
        bool reversed;

        void rehash(idx sizeTable);
        void unmap(idx hash);
        idx mapCount(void){return ((HashHeader *)hashPtr)->mapCount(); }
        static idx hashfun(hashtypeIdx typeindex, const void * mem, idx len, idx bits, idx iNum);
        idx find(HashRecord * hashPtrLocal, hashtypeIdx typeindex, const void * key, idx lenKey, idx slotType=0, idx * pHash=0); // , idx * bucketStart=0

    public:

        sHash2 (const char * filename=0, idx flags=sMex::fSetZero|sMex::fExactSize){
            hashPtr=0;
            bucketContainer=0;
            init(filename,flags);
            collisionReducer=4;
            //bucket_lstBits=3;
            keyfunc=0;
            keyParam=0;
            baseFileName.flags=sMex::fExactSize;
            keyBody.flags=sMex::fBlockCompact;
            reversed=0;
        }
        sHash2 * init(const char * filename=0 , idx flags=sMex::fSetZero|sMex::fExactSize ){
            baseFileName.flags=sMex::fExactSize;
            if(filename)baseFileName.add(filename,sLen(filename)+1);
            hashTableVec.init(filename,flags|sMex::fSetZero|sMex::fExactSize);
            hashPtr=hashTableVec.ptr();

            return this;
        }
        ~sHash2()
        {
            //if(hashTableVec.mex()->flags&sMex::fReadonly)
            hashTableVec.destroy(); // to avoid calling stupid destructors
        }

        idx map(idx recordindex, hashtypeIdx typeindex, const void * key,idx lenKey, idx * pFnd=0, idx insIndex=sNotIdx);
        idx find(hashtypeIdx typeindex, const void * key, idx lenKey, idx slotType=0, idx * pHash=0){
            return find(hashPtr,typeindex, key, lenKey, slotType, pHash);
        }


        typedef void * (* keyFuncType)(void * param, hashtypeIdx  typeindex, idx recordindex,idx * sizeKey, sMex * keybody);
        //typedef void * (* hashFuncType)(idx typeindex, idx recordindex,idx * sizeKey);

        keyFuncType keyfunc;
        void * keyParam;

        idx hashMemSize(void){
            //return (((HashHeader *)hashPtr)->mapCount)*sizeof(HashRecord)+sizeof(HashHeader);
            //return hashTableVec.dim(); //(((HashHeader *)hashPtr)->mapCount)*sizeof(HashRecord)+sizeof(HashHeader);
            if(hashPtr)
                return  (((udx)1)<<(((HashHeader *)hashPtr)->bits()))*sizeof(HashRecord)+sizeof(HashHeader);
            else return 0;
        }
        void * hashMem(void){
            return ((void *)hashPtr);
        }
        bool ok (void) const { return  hashPtr || hashTableVec.ok();}
        void cut(idx pos){
            hashTableVec.cut(pos);
        }
        void destroy(){
            hashTableVec.destroy();
            baseFileName.destroy();
        }

        idx bucketCount(idx index)
        {
            idx * base;
            if(index&(sHash2_idxFilterBit) ){
                base=(idx*)bucketContainer->ptr(index&(sHash2_idxFilterMask));
                return base[2];
            }
            else return 1;
        }

        idx bucketNextIndex(idx * pindex , bool jumpLast=false, idx cnt=1)
        {
            if( (*pindex)&(sHash2_idxFilterBit) ){
                (*pindex)&=(sHash2_idxFilterMask);
                idx ret=sAlgo::lilist_next_item(bucketContainer, pindex, jumpLast , cnt);
                *pindex|=sHash2_idxFilterBit;
                if(jumpLast)
                    return bucketNextIndex(pindex , false);
                return ret&(sIdxHighMask);
            }
            else {
                idx ret=*pindex;
                *pindex=sNotIdx;
                return ret;
            }
        }
        void bucketDeleteElements(idx * pindex, idx start, idx cnt)
        {
            if( (*pindex)&(sHash2_idxFilterBit) ){
                (*pindex)&=(sHash2_idxFilterMask);
                sAlgo::lilist_delete_items(bucketContainer, pindex, start, cnt);
            }
        }

        void bucketDelete(idx * pindex, idx buckethash)
        {
            *pindex=sNotIdx;
            unmap(buckethash);
        }

        idx bucketCnt(idx pindex)
        {
            if( (pindex)&(sHash2_idxFilterBit) ){
                (pindex)&=(sHash2_idxFilterMask);
                return *sAlgo::lilist_pcnt_items(bucketContainer, pindex);
            }
            else {
                return 1;
            }
        }

    };
} // namespace slib

#endif //  sLib_hash2_h
