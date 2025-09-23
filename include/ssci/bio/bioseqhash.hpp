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
#include <slib/utils/sort.hpp>


namespace slib
{



    class sBioseqHash {

    public:

        struct RefPos { 
            idx _pos;
            idx pos(void){return (_pos&0x000FFFFFFFF); }
            idx id(void){
                #ifdef SLIB64 
                    return ((_pos>>32)&0xFFFFFFFF);  
                #else
                    return ((_pos>>20)&0x00000FFF);
                #endif
            }
        };

        struct Hdr {
            idx lenUnit,lenAlpha,lenTbl, hashMask;
        };
    
        struct HLitem{RefPos ref; int next;};
        sVec < HLitem > hList; 

        #ifdef HASH_SINDEX
            struct HLRefPos { udx defint; idx ofs;};
            sIndex < HLRefPos > hashInd;
        #else
            sVec < int > hashTbl;
        #endif

        idx collisionReducer;
        



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

        sBioseqHash( idx llenunit=11 , idx llenalphabet=4 ) {
            ofs=0;rFl=0;rFos=0;forceReset=1;hb=0;
            collisionReducer=4;
            hashStp=1;
            init(llenunit ,llenalphabet);
        }
        sBioseqHash ( const char * flnm ) {
            ofs=0;rFl=0;rFos=0;forceReset=1;hb=0;
            collisionReducer=4;
            hashStp=1;
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
            if(hashBloom.dim())hashBloom.destroy();
            if(rFl)delete rFl;
        }

    public:


        enum eCompileFlags{ eCompileReset=0x01, eCompileDic=0x02, eCompileBloom=0x04, eCompileOnlyBloom=0x08,eCompileReverse=0x10 };
        void reset(void){forceReset=1;}
        idx compile(idx refseq, const char * seq, idx lenseq, idx flags=0, idx maxBin=0, idx startcompile=0, idx lencompile=0 , idx complexityWindow=0, real complexityShannon=0 , const char * qua=0, bool quabit=0, idx acceptNNNQuaTrheshold=0);
        idx fospile(idx refseq, const char * seq, idx lenseq, idx flags=0, idx startcompile=0, idx lencompile=0 );
        void bloom(void);
        void fossilize(idx refseq, const char * seq, idx lenseq, idx granule,idx fosblock, idx fosstp, idx history, idx flags=0);
        idx  dumpCompiled (const char * flnm, idx * maxBin=0, idx * aveBin=0, idx  * totBin=0);
        idx  dumpFossilized (const char * flnm);
        static idx dynamicMatch( short int * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2 , idx len2, idx maxDiag, idx nMereKeep, const short int * costs, idx lenseq=0);
        static char * dbgH(idx hash, idx len);
        static idx longestDynamicMatch( short int * matP, const char * seq1, const char * seq2, idx st1, idx len1, idx st2, idx len2, idx maxDiag, idx lenseq,  idx * matchPos=0 );

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

    class sBioHashHits {
        private:
            idx _kmer, _avHashHits, _strlen;
            idx _rng_cnt, _hp_cnt;

            struct hrng {
                int ref,isHit,start,end;
                void setrng(idx lref, idx rngstart, idx rngend, bool lisHit = false) {
                    ref= lref;
                    isHit= lisHit;
                    start= rngstart;
                    end= rngend;
                }

                void updaterng(idx rngstart, idx rngend, bool lisHit) {
                    isHit= lisHit;
                    start= rngstart;
                    end= rngend;
                }

            };

            hrng _cmp;

            inline idx _get_hashpos(idx ihp){
                return ((_hpvec[ihp]>>48)&0xFFFF);
            }
            inline idx _get_hashcnt(idx ihp) {
                return ((_hpvec[ihp]>>32)&0xFFFF);
            }
            inline idx _get_hashoffs(idx ihp) {
                return (_hpvec[ihp]&0xFFFFFFFF);
            }


            inline void _increase_hashcnt( idx inc = 1) {
                _hpvec[_hp_cnt-1] += (inc << 32);
            }
            inline void _set_hp(idx pos) {
                _hpvec.resizeM(++_hp_cnt);
                _hpvec[_hp_cnt-1]= (pos<<48)|((idx)1<<32)|((_rng_cnt)&0xFFFFFFFF);
            }


            static idx rngComparator (void * param, hrng * current, hrng * arr) {
                idx diff = current->ref - arr->ref;
                if( diff )
                    return diff<<32;
                if(arr->start <= current->start && current->end <= arr->end )
                    return 0;
                diff = current->start - arr->start;
                return diff;
            }
            sVec<idx> _hpvec;
            sVec<hrng> _rngvec;

        public:
            sBioHashHits(int lkmer, int lavHasHits = 2, int lstrlen = 0 ) {
                init(lkmer, lavHasHits, lstrlen);
            }
            void init(int lkmer, int lavHasHits = 2, int lstrlen = 0) {
                _avHashHits = lavHasHits;
                _strlen = lstrlen;
                _kmer = lkmer;
                reset();
            }
            void reset(idx lstrlen = 0) {
                if(lstrlen > _strlen) {
                    _strlen = lstrlen;
                }
                _hp_cnt = _rng_cnt = 0;
                _hpvec.resizeM(_strlen - _kmer);
                _rngvec.resizeM(_hpvec.dim() * _avHashHits);
            }
            idx addHashHit(idx hashpos, idx ref, idx hitpos, idx rangeend, bool isHit) {
                if(_hp_cnt && hashpos == _get_hashpos(_hp_cnt-1)) {
                    _increase_hashcnt();
                } else {
                    _set_hp(hashpos);
                }
                _rngvec.resizeM(++_rng_cnt);
                _rngvec[_rng_cnt-1].setrng(ref,hitpos,rangeend,isHit);
                return _get_hashcnt(_hp_cnt - 1);
            }
            inline void updateHashHit(idx hitpos, idx rangeend,bool isHit) {
                _rngvec[_rng_cnt-1].updaterng(hitpos,rangeend,isHit);
            }
            inline idx hashOverlaps(idx hpos) {
                if(!_hp_cnt)return false;
                return ( hpos - _kmer <=  _get_hashpos(_hp_cnt -1) );
            }

            inline idx hitExists(idx ref, idx hpos, idx ihpodend,idx iq = -1, idx delta =-1) {
                if(!_hp_cnt)return false;
                _cmp.setrng(ref,hpos,ihpodend);
                for (idx i = _hp_cnt -1 ; i >= 0 ; --i) {
                    idx res = sSort::binarySearch((sSort::sCallbackSearch)rngComparator,0,&_cmp,_get_hashcnt(i),_rngvec.ptr(_get_hashoffs(i)));
                    if( res >= 0 && (iq < 0 || _rngvec[_get_hashoffs(i)+res].isHit || ((iq - _get_hashpos(i)) - (hpos - _rngvec[_get_hashoffs(i)+res].start)) == delta) )
                        return true;
                }
                return false;
            }

    };

}
#endif 
