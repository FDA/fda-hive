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
#ifndef sBio_seqhash_hpp
#define sBio_seqhash_hpp

#include <slib/core/def.hpp>
#include <slib/core/index.hpp>

//#define HASH_SINDEX

namespace slib
{


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Sequence Collection Hashing class
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    class sBioseqHash {

        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ construction/destruction
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    public:

        struct RefPos { 
            idx _pos; // the position on a reference sequence
            idx pos(void){return (_pos&0x000FFFFFFFF); }
            idx id(void){
                #ifdef SLIB64 
                    return ((_pos>>32)&0xFFFFFFFF);  
                #else
                    return ((_pos>>20)&0x00000FFF);
                #endif
            }
        };

        struct Hdr { // bioseq Hash file/blob header
            idx lenUnit,lenAlpha,lenTbl, hashMask;
        };
    
        struct HLitem{RefPos ref; int next;}; // the first keeps either the count (for the first one in the list) and the position to the next  in the list
        sVec < HLitem > hList; 

        #ifdef HASH_SINDEX
            struct HLRefPos { udx defint; idx ofs;};
            sIndex < HLRefPos > hashInd;
        #else
            sVec < int > hashTbl;
        #endif

        idx collisionReducer;
        
        //typedef sLst < RefPos , 2 > HashBinType;
        //sMex lstMex; /// here we hold all the hash bins
        //typedef sLst < RefPos , 2 > HashBinType;

        //sVec < HashBinType >  hashTbl;
        //struct HashRefPos{ idx defint; HashBinType list;};
        //sIndex < HashRefPos  > hashDic;


        sVec < char > hashBloom;
                
        sVec < sVec < idx > > hashFossil;
        sFil  * rFl;
        sFil * rFos;
        idx * ofs;
        Hdr hdr;
        char * hb;
        idx forceReset;
        idx hashStp;

    public:

        // sBioseqHash
        sBioseqHash( idx llenunit=11 , idx llenalphabet=4 ) {
            ofs=0;rFl=0;rFos=0;forceReset=1;hb=0;
            collisionReducer=4;
            hashStp=1;
            //hList.add(1);// we add a single element so the offset of the next ones is never zero
            //lookHash=0, lookBloom=0;
            init(llenunit ,llenalphabet);
        }
        sBioseqHash ( const char * flnm ) {
            ofs=0;rFl=0;rFos=0;forceReset=1;hb=0;
            collisionReducer=4;
            hashStp=1;
            //hList.add(1);// we add a single element so the offset of the next ones is never zero
            //lookHash=0, lookBloom=0;
            if(!flnm)return ;
            init( flnm);
        }

        sBioseqHash * init(idx llenunit , idx llenalphabet) ;
        sBioseqHash * init(const char * flnm)
        {
            rFl=new sFil(flnm,sMex::fReadonly);
            if(rFl->length()==0)return 0;
            hdr=*((Hdr*)rFl->ptr(0));
            ofs=(idx *)rFl->ptr(sizeof(Hdr));
            bloom ();
            return this;
        }
        idx initFile(const char * filename, idx flags);

        void destroy(void){
            //if(hashInList.dim())hashInList.empty();
            /////if(hashDic.dim())hashDic.empty();
            //if(hashTbl.dim())hashTbl.destroy();
            if(hashBloom.dim())hashBloom.destroy();
            if(rFl)delete rFl;
        }

        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
        // _/
        // _/ hash compilation and access functions
        // _/
        // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    public:

                // hash is being manipulated at sBioSeqhash::compile
                // to match manipulated hash keys we have to perform the same manipulation here
        /*idx find (idx hash) {
            #ifdef HASH_SINDEX
                hash=(hash<<8)| ((hash>>(2*(hdr.lenUnit-4)))&0xFF);
                hash&=hdr.hashMask;
            #endif
            return hashInd.find(hash);
        }*/

        enum eCompileFlags{ eCompileReset=0x01, eCompileDic=0x02, eCompileBloom=0x04, eCompileOnlyBloom=0x08,eCompileReverse=0x10 };
        void reset(void){forceReset=1;}
        idx compile(idx refseq, const char * seq, idx lenseq, idx flags=0, idx maxBin=0, idx startcompile=0, idx lencompile=0 , idx complexityWindow=0, real complexityShannon=0 , const char * qua=0, bool quabit=0, idx acceptNNNQuaTrheshold=0);
        idx fospile(idx refseq, const char * seq, idx lenseq, idx flags=0, idx startcompile=0, idx lencompile=0 );
        void bloom(void);
        void fossilize(idx refseq, const char * seq, idx lenseq, idx granule,idx fosblock, idx fosstp, idx history, idx flags=0);
        idx  dumpCompiled (const char * flnm, idx * maxBin=0, idx * aveBin=0, idx  * totBin=0);
        idx  dumpFossilized (const char * flnm);
        //static idx fossilizeQuick(idx * fossillist, idx foscnt, const char * seq, idx lenseq, idx granule, idx granstep , idx fosblock, idx history, idx flags);
        static idx dynamicMatch( short int * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2 , idx len2, idx maxDiag, idx nMereKeep, const short int * costs, idx lenseq=0);
        static char * dbgH(idx hash, idx len);
        static idx longestDynamicMatch( short int * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2, idx len2, idx maxDiag, idx lenseq,  idx * matchPos=0 );

        /*idx cnt(idx hash){
            //++lookBloom;
            //if( (hb [hash/8] & (((udx)1)<<(hash%8))) ==0 ) return 0;
            //++lookHash;
            //if(ofs) return ofs[hash*2+1];
            //else 
            if(hashTbl.dim() )return hashTbl[hash].dim();
            //else if(hashInList.dim() )return hashInList[&hash];
            else { 
                idx iFnd=hashDic.find(hash);
                return iFnd ? hashDic[iFnd-1].list.dim() : 0 ;
            }
        }
        HashBinType * lst(idx hash){
            //if( hb [hash/8]& ((udx)1<<hash%8)  ) return 0;
            //++lookHash;
            //if(ofs) return (RefPos*)((char *)ofs+ofs[hash*2]) ;
            //else 
            if(hashTbl.dim()) return &hashTbl[hash];//.ptr(0);
            else {
                idx iFnd=hashDic.find(hash);
                return iFnd ? &(hashDic[iFnd-1].list) : 0 ;
            }
        }
        */
        static idx * fos(idx * pstart, idx seqnum ){
            if(!pstart)return 0;
            idx ofs=pstart[seqnum+1];
            return pstart+ofs;
        }
        


        sLst <int> suff_lst;
        sMex suff;
        sMex suff_bins;
        idx compileSuffix(idx refseq, const char * seq, idx lenseq, idx flags, idx maxBin, idx startcompile, idx lencompile, idx complexityWindow, real complexityShannon);


    };



} // namespace 

#endif // sBio_seqhash_hpp

