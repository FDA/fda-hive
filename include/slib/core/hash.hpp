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
#ifndef sLib_hash_h
#define sLib_hash_h

#include <slib/core/mex.hpp>
#include <slib/core/vec.hpp>

namespace slib
{


    class sHash {
    public:
            struct HashHeader{
                idx mapCount;
                idx bits;
            };

            struct HashRecord{
                idx typeindex;
                idx recordindex;
            };




        sMex * bucketContainer;
        idx bucket_lstBits;

        sVec < HashRecord > hashTableVec;
        HashRecord * hashPtr;
        idx collisionReducer,mapCount;
        const static idx defaultInitialBitness=4;

        void rehash(idx sizeTable);
        idx hashfun(idx typeindex, const void * mem, idx len, idx bits, idx iNum);


        public:

        sHash (const char * filename=0, idx flags=sMex::fSetZero|sMex::fExactSize){
            hashPtr=0;
            bucketContainer=0;
            init(filename,flags);
            collisionReducer=4;
            bucket_lstBits=3;
            keyfunc=0;
            keyParam=0;
        }
        sHash * init(const char * filename=0 , idx flags=sMex::fSetZero|sMex::fExactSize ){
            hashTableVec.init(filename,flags|sMex::fSetZero|sMex::fExactSize);
            hashPtr=hashTableVec.ptr();
            return this;
        }

        idx find(idx typeindex, const void * key, idx lenKey, idx slotType=0, idx * pHash=0, idx * bucketStart=0);
        idx map(idx recordindex, idx typeindex, const void * key,idx lenKey, idx * pFnd=0 );


        typedef void * (* keyFuncType)(void * param, idx typeindex, idx recordindex,idx * sizeKey);

        keyFuncType keyfunc;
        void * keyParam;

        idx hashMemSize(void){
            if(hashPtr)
                return  (((udx)1)<<(((HashHeader *)hashPtr)->bits))*sizeof(HashRecord)+sizeof(HashHeader);
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
        }

    };
}

#endif 