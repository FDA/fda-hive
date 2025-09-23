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
#ifndef Vstd_clr_hpp
#define Vstd_clr_hpp

#include <slib/core/def.hpp>
#include <slib/core/str.hpp>
#include <math.h>

namespace slib { 


    class sClr 
    {   
            idx _rgb;
        public:

            enum eLumaMode {
                eRec709 = 0,
                eRec601 = 1
            };

            sClr(idx rgb=0) { set(rgb); }
            sClr(idx rgb, idx a=255) { set(rgb, a); }
            sClr(idx r,idx g,idx b, idx a=255) { set(r,g,b, a ); }
            #ifdef SLIB64
            sClr(int r,int g, int b, int a=255) { set((idx)r,(idx)g,(idx)b, (idx)a ); }
            #endif
            sClr(real lr,real lg,real lb, real la){  set(lr,lg,lb,la); }
            sClr(real hue1,real saturation1,real brightness1) { setHSB(hue1,saturation1,brightness1); }

            idx rgb( void ) const {return _rgb;}
            idx b( void )const { return b(_rgb); }
            idx g( void )const { return g(_rgb); }
            idx r( void )const { return r(_rgb); }
            idx a( void )const { return a(_rgb); }
            real b1( void )const { return b(_rgb)/255.; }
            real g1( void )const { return g(_rgb)/255.; }
            real r1( void )const { return r(_rgb)/255.; }
            real a1( void )const { return a(_rgb)/255.; }

            real hue360(void) const;
            real hue1(void) const { return hue360() / 360.; }
            real chroma1(void) const { return value1() - minValue1(); }
            real saturationHSV1(void) const
            {
                real maxChrom = value1();
                return maxChrom ? chroma1() / maxChrom : 0.;
            }
            real saturationHSL1(void) const
            {
                real maxChrom = 1 - fabs(2 * lightness1() - 1);
                return maxChrom ? chroma1() / maxChrom : 0.;
            }
            real lightness1(void) const { return (value1() + minValue1()) / 2.; }
            real intensity1(void) const { return (r1() + g1() + b1()) / 3.; }
            real luminance1(eLumaMode m = sClr::eRec709) const;
            real value1(void) const { return sMax<idx>(sMax<idx>(r(), g()), b()) / 255.; }
            real minValue1(void) const { return sMin<idx>(sMin<idx>(r(), g()), b()) / 255.; }

            void getHSB(real * hue1, real * saturation1, real * brightness1) const;

            const char * printHex(sStr & out, bool forceAlpha=false) const;

            void set(idx rgb) {_rgb=rgb|0xFF000000;}
            void set(idx rgb, idx a) {_rgb=rgb|(a<<24);}
            void set(idx r,idx g,idx b, idx a=255) {_rgb=rgb(r,g,b,a);}
            #ifdef SLIB64
            void set(int r,int g,int b, int a=255) {_rgb=rgb((idx)r,(idx)g,(idx)b,(idx)a);}
            #endif
            void set(real r1, real g1, real b1, real a1) {set(idxrint(r1*255), idxrint(g1*255), idxrint(b1*255), idxrint(a1*255));}

            void setHCL(real hue360, real chroma1, real lightness1);
            void setHCY(real hue360, real chroma1, real luminance1, eLumaMode m=eRec709)
            {
                setHCL(hue360, chroma1, chroma1 / 2);
                real min_value = luminance1 - this->luminance1(m);
                set(r1() + min_value, g1() + min_value, b1() + min_value, 1.0);
            }
            void setHSL(real hue360, real saturation1, real lightness1)
            {
                setHCL(hue360, saturation1 * (1. - fabs(2 * lightness1 - 1.)), lightness1);
            }
            void setHSV(real hue360, real saturation1, real value1)
            {
                setHCL(hue360, value1 * saturation1, value1 - (value1 * saturation1) / 2);
            }
            void setHSB(real hue1, real saturation1, real brightness1);
            void parse(const char * s);

            static sClr none;
        public:
            static idx rgb(idx r,idx g,idx b, idx a=255) { return ( (b&0xFF) | ((g&0xFF)<<8) | ((r&0xFF)<<16) | ((a&0xFF)<<24) ); }
            static idx b(idx lrgb){ return ( (((idx)(lrgb)) )&0xFF ); }
            static idx g(idx lrgb){ return ( (((idx)(lrgb)) >> 8)&0xFF );}
            static idx r(idx lrgb){ return ( (((idx)(lrgb)) >> 16)&0xFF );}
            static idx a(idx lrgb){ return ( (((idx)(lrgb)) >> 24)&0xFF );}

        private:
            struct Pigment{
                char name[64];
                idx rgb;

                static int compareName(const void * name, const void * pig);
            };
        public:
            static const Pigment clrTable[];
            static const idx clrTableDim;
            static sClr pigment(idx iclr=-1, idx a=255);
            static sClr inverse(const sClr & clr){ return sClr(~(clr.rgb()),255); }
        };


}

#endif 

