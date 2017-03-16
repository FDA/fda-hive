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
#ifndef sLib_core_index_h
#define sLib_core_index_h

#include <slib/core/vec.hpp>
#include <slib/core/perf.hpp>

namespace slib
{
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ class sAlgoTpl
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    
    struct sIndexStruc{idx defint;}; // minimalistic structure which must be part of Tobj
    struct sIndexIndex{idx defint; idx dstIndex;};
    template <class Tobj> class sIndex: public sVec < Tobj > 
    {
    public:
        sVec< idx > hashTbl;
        idx rehashcnt, collided, collidedTotal;
        idx bitness, valibit;



        
    public: // quick double 
        sIndex (const char * flnm=0, idx openModeFlags=0) {
            init(flnm, openModeFlags);
        }

        sIndex * init( const char * flnm=0, idx openModeFlags=0 )
        {
            valibit=-1;//1<<(sizeof(idx)*8-1);
            collided=0;
            collidedTotal=0;
            //rehashcnt=0;
            bitness=1;
            //sVec < Tobj >::setflag(sMex::fBlockDoubling);
            //hashTbl.setflag(sMex::fBlockDoubling|sMex::fSetZero);
            //hashTbl.setflag(sMex::fBlockNormal|sMex::fSetZero);
            if(flnm) {
                sStr t;
                hashTbl.init(flnm,openModeFlags|sMex::fBlockNormal|sMex::fSetZero);
                sVec<Tobj>::init(t.printf("%s_hash",flnm),openModeFlags|sMex::fBlockDoubling|sMex::fSetZero);
                getValiBit();
            }else {
                hashTbl.setflag(sMex::fBlockNormal|sMex::fSetZero|openModeFlags );
                this->setflag(sMex::fBlockDoubling|sMex::fSetZero|openModeFlags );
            }
            return this;
        }
        
        void getValiBit() // used for serialization when bitness and valibit are maintained as last two integers
        {
            if(!hashTbl.dim())return;
            idx * p=hashTbl.ptr(hashTbl.dim()-2);
            valibit=p[0];
            bitness=p[1];
        }
        void setValiBit() // used for serialization when bitness and valibit are maintained as last two integers
        {
            idx * p=hashTbl.ptr(hashTbl.dim()-2);
            p[0]=valibit;
            p[1]=bitness;
        }

        void reserve(idx zestimate, idx collisionReducer=4)
        {
            ++valibit;
            idx oldbitness=bitness;
            for( ; (idx) ((udx)1<<bitness) <= (((udx)1<<zestimate)+1)*collisionReducer ; ++bitness);
            if(oldbitness<bitness){ // rehash
                ++valibit; // now only those hashes whivh have this in highes bit are valid 
                hashTbl.resizeM((((udx)1)<<bitness)+2);
            }
            
            idx csize=sVec<Tobj>::dim();
            sVec<Tobj>::resizeM(zestimate);
            sVec<Tobj>::cutM(csize);
            setValiBit();
        }

        idx find(udx defint, udx * pHash=0 )
        {    

            if(!sVec<Tobj>::dim()) return 0;
            //idx mask=(1<<bitness)-1;
            //idx h=defint& (((udx)1<<bitness)-1) ;
            //static const idx mask=0x00FF00FF00FF00FFll;
            //defint=(defint&mask) | (sEndian(defint)&(mask>>8));
            idx h=defint&(((udx)1<<bitness)-1) ;
            idx dh=7919; //1; a primernumber
            //idx * ht=hashTbl.ptr();
            //idx totcnt=sVec<Tobj>::dim();

            //// idx sAlgo::hax_hashfun(const void * mem, idx len, idx bits, idx iNum)
            for(;;) { 
                idx ref=hashTbl[h];
                idx val=(ref>>56)&0xFF;
                if(val!=valibit)ref=0;
                else ref&=0x00FFFFFFFFFFFFFFll; // turn off the validity bits to get the pure reference 
                
                //if(ref!=0 && val!=valibit)
                //    valibit=valibit;

                if ( ref==0 ||  (udx)((*this)[ref-1].defint)==defint ){ // return if first occurence or found item 
                    if(pHash)*pHash=h;
                    return ref;
                }
                //*pHash = (*pHash + (defint%(hashTbl1.dim())|1) ) % (hashTbl.dim()); // double hashing resolution of collisions
                collided++;collidedTotal++;
                //idx prvdefint=(udx)((*this)[ref-1].defint);
                //if(!dh)dh = sAlgo::hax_hashfun(&defint,sizeof(defint),bitness,1); // double hashing resolution of collisions
                h += dh;
                // h++; // linear collision resolution
                //if(h >= hashTbl.dim() ) // over the limits ? rebound !
                //    h -=hashTbl.dim() ;
                h%=hashTbl.dim()-2;
            }
        }        

        idx add(udx defint, Tobj * myo, idx collisionReducer=4)
        {

//PERF_START("SRCH 1");
            udx h=0;
            idx ref=find(defint,&h);
            if(ref) return ref-1; // 1 based indexes are in the hash table 
            
            idx oldbitness=bitness;
            //idx totcnt=sVec<Tobj>::dim();
            for( ; (idx) ((udx)1<<bitness) <= (sVec<Tobj>::dim()+1)*collisionReducer ; ++bitness);
//PERF_NEXT("::REHASHH");
            if(oldbitness<bitness){ // rehash
                ++valibit; // now only those hashes whivh have this in highes bit are valid 
                //++rehashcnt; 
                collided=0; 
                //idx szresize=(1<<bitness);
                hashTbl.resizeM((((udx)1)<<bitness)+2);
                //hashTbl.set(0);
                for( idx i=0; i< sVec<Tobj>::dim() ; ++i ) {
                    find((udx)((*this)[i].defint), &h );
                    hashTbl[h]=(i+1)|(valibit<<56);
                }
                h=0;
            }
//PERF_NEXT("::SRCH 2");
            ref=sVec<Tobj>::dim();
            sVec<Tobj>::resizeM(ref+1);
            Tobj * po=sVec<Tobj>::ptr(ref);//(sVec<Tobj>::addM());
            if(myo)*po=*myo;
            po->defint=defint;
            if(h==0)find(defint, &h );
            hashTbl[h]=(ref+1)|(valibit<<56);
            setValiBit();

//PERF_END();
        return ref;
        }

        void empty(void)
        {
            hashTbl.destroy();
            sVec < Tobj >::destroy();
            bitness=1;
            valibit=-1;
        }


    
        
    };
    
}


#endif // sLib_core_vec_h













