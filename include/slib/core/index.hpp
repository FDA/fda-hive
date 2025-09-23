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
    
    struct sIndexStruc{idx defint;};
    struct sIndexIndex{idx defint; idx dstIndex;};
    template <class Tobj> class sIndex: public sVec < Tobj > 
    {
    public:
        sVec< idx > hashTbl;
        idx rehashcnt, collided, collidedTotal;
        idx bitness, valibit;



        
    public:
        sIndex (const char * flnm=0, idx openModeFlags=0) {
            init(flnm, openModeFlags);
        }

        sIndex * init( const char * flnm=0, idx openModeFlags=0 )
        {
            valibit=-1;
            collided=0;
            collidedTotal=0;
            bitness=1;
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
        
        void getValiBit()
        {
            if(!hashTbl.dim())return;
            idx * p=hashTbl.ptr(hashTbl.dim()-2);
            valibit=p[0];
            bitness=p[1];
        }
        void setValiBit()
        {
            idx * p=hashTbl.ptr(hashTbl.dim()-2);
            p[0]=valibit;
            p[1]=bitness;
        }

        void reserve(idx zestimate, idx collisionReducer=4)
        {
            ++valibit;
            idx oldbitness=bitness;
            for( ; ((udx)1<<bitness) <= (((udx)1<<zestimate)+1)*collisionReducer ; ++bitness);
            if(oldbitness<bitness){
                ++valibit;
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
            idx h=defint&(((udx)1<<bitness)-1) ;
            idx dh=7919;

            for(;;) { 
                idx ref=hashTbl[h];
                idx val=(ref>>56)&0xFF;
                if(val!=valibit)ref=0;
                else ref&=0x00FFFFFFFFFFFFFFll;
                

                if ( ref==0 ||  (udx)((*this)[ref-1].defint)==defint ){
                    if(pHash)*pHash=h;
                    return ref;
                }
                collided++;collidedTotal++;
                h += dh;
                h%=hashTbl.dim()-2;
            }
        }        

        idx add(udx defint, Tobj * myo, idx collisionReducer=4)
        {

            udx h=0;
            idx ref=find(defint,&h);
            if(ref) return ref-1;
            
            idx oldbitness=bitness;
            for( ; (idx) ((udx)1<<bitness) <= (sVec<Tobj>::dim()+1)*collisionReducer ; ++bitness);
            if(oldbitness<bitness){
                ++valibit;
                collided=0; 
                hashTbl.resizeM((((udx)1)<<bitness)+2);
                for( idx i=0; i< sVec<Tobj>::dim() ; ++i ) {
                    find((udx)((*this)[i].defint), &h );
                    hashTbl[h]=(i+1)|(valibit<<56);
                }
                h=0;
            }
            ref=sVec<Tobj>::dim();
            sVec<Tobj>::resizeM(ref+1);
            Tobj * po=sVec<Tobj>::ptr(ref);
            if(myo)*po=*myo;
            po->defint=defint;
            if(h==0)find(defint, &h );
            hashTbl[h]=(ref+1)|(valibit<<56);
            setValiBit();

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


#endif 












