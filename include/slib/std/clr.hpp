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


    //typedef idx sCLR;
    //! 32-bit RGBA color
    class sClr 
    {   
            idx _rgb;
        public:

            enum eLumaMode {
                eRec709 = 0, //!< Rec. 709 luma/luminance measure, as used in sRGB and HDTV
                eRec601 = 1  //!< Rec. 601 luma/luminance measure, as used in MPEG
            };

            sClr(idx rgb=0) { set(rgb); }
            sClr(idx rgb, idx a=255) { set(rgb, a); }
            sClr(idx r,idx g,idx b, idx a=255) { set(r,g,b, a ); }
            #ifdef SLIB64
            sClr(int r,int g, int b, int a=255) { set((idx)r,(idx)g,(idx)b, (idx)a ); }
            #endif
            sClr(real lr,real lg,real lb, real la){  set(lr,lg,lb,la); }
            sClr(real hue1,real saturation1,real brightness1) { setHSB(hue1,saturation1,brightness1); }

            //! encoded rgba value
            idx rgb( void ) const {return _rgb;}
            //! blue component in [0, 255] range
            idx b( void )const { return b(_rgb); }
            //! green component in [0, 255] range
            idx g( void )const { return g(_rgb); }
            //! red component in [0, 255] range
            idx r( void )const { return r(_rgb); }
            //! alpha component in [0, 255] range
            idx a( void )const { return a(_rgb); }
            //! blue component in [0, 1] range
            real b1( void )const { return b(_rgb)/255.; }
            //! green component in [0, 1] range
            real g1( void )const { return g(_rgb)/255.; }
            //! red component in [0, 1] range
            real r1( void )const { return r(_rgb)/255.; }
            //! alpha component in [0, 1] range
            real a1( void )const { return a(_rgb)/255.; }

            //! hue in degrees, i.e. in [0, 360) range
            real hue360(void) const;
            //! hue in turns, i.e. in [0, 1) range
            real hue1(void) const { return hue360() / 360.; }
            //! chroma (perceived colorfulness) in [0, 1] range
            real chroma1(void) const { return value1() - minValue1(); }
            //! saturation as used in the HSV color model, in [0, 1] range
            real saturationHSV1(void) const
            {
                real maxChrom = value1();
                return maxChrom ? chroma1() / maxChrom : 0.;
            }
            //! saturation as used in the HSL color model, in [0, 1] range
            real saturationHSL1(void) const
            {
                real maxChrom = 1 - fabs(2 * lightness1() - 1);
                return maxChrom ? chroma1() / maxChrom : 0.;
            }
            //! lightness as used in the bi-hexcone model, in [0, 1] range, where 1 is white and 0 is black
            real lightness1(void) const { return (value1() + minValue1()) / 2.; }
            //! average of red, green, and blue components, in [0, 1] range
            real intensity1(void) const { return (r1() + g1() + b1()) / 3.; }
            //! luminance (perceived brightness) in [0, 1] range
            real luminance1(eLumaMode m = sClr::eRec709) const;
            //! largest of the 3 color components, in [0, 1] range
            real value1(void) const { return sMax<idx>(sMax<idx>(r(), g()), b()) / 255.; }
            //! smallest of the 3 color components, in [0, 1] range
            real minValue1(void) const { return sMin<idx>(sMin<idx>(r(), g()), b()) / 255.; }

            void getHSB(real * hue1, real * saturation1, real * brightness1) const;

            //! print color as \#RRGGBB or \#RRGGBBAA
            /*! \param forceAlpha if true, always print alpha component;
                                  if false, alpha will be printed only if not equal to 255
                \returns pointer to start of printed string in \a out */
            const char * printHex(sStr & out, bool forceAlpha=false) const;

            void set(idx rgb) {_rgb=rgb|0xFF000000;}
            void set(idx rgb, idx a) {_rgb=rgb|(a<<24);}
            void set(idx r,idx g,idx b, idx a=255) {_rgb=rgb(r,g,b,a);}
            #ifdef SLIB64
            void set(int r,int g,int b, int a=255) {_rgb=rgb((idx)r,(idx)g,(idx)b,(idx)a);}
            #endif
            void set(real r1, real g1, real b1, real a1) {set(idxrint(r1*255), idxrint(g1*255), idxrint(b1*255), idxrint(a1*255));}

            //! Set color using the hue-chroma-lightness bi-hexcone model
            /*! \param hue360 hue in degrees (0 = red, 120 = green, 240 = blue)
                \param chroma1 chroma in [0, 1] range (1 = colorful, 0 = grey); note that some
                               chroma values are invalid for a given lightness, so the chroma
                               parameter will be automatically clamped into a valid range.
                \param lightness1 lightness in [0, 1] range (1 = white, 0 = black). */
            void setHCL(real hue360, real chroma1, real lightness1);
            //! Set color using the hue-chroma-luminance perceptual model
            void setHCY(real hue360, real chroma1, real luminance1, eLumaMode m=eRec709)
            {
                setHCL(hue360, chroma1, chroma1 / 2);
                real min_value = luminance1 - this->luminance1(m);
                set(r1() + min_value, g1() + min_value, b1() + min_value, 1.0);
            }
            //! Set color using the hue-saturation-lightness bi-hexcone model
            void setHSL(real hue360, real saturation1, real lightness1)
            {
                setHCL(hue360, saturation1 * (1. - fabs(2 * lightness1 - 1.)), lightness1);
            }
            //! Set color using the hue-saturation-value hexcone model
            void setHSV(real hue360, real saturation1, real value1)
            {
                setHCL(hue360, value1 * saturation1, value1 - (value1 * saturation1) / 2);
            }
            void setHSB(real hue1, real saturation1, real brightness1);
            //! Set color by parsing an html-style hex string or a standard color name
            /*! \param s String in \#RRGGBB, \#RRGGBBAA, or \#RGB format, or a standard SVG
                         color name - see http://www.w3.org/TR/css3-color/#svg-color */
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

                //! name matching function for bsearch
                static int compareName(const void * name, const void * pig);
            };
        public:
            static const Pigment clrTable[];
            static const idx clrTableDim;
            //static sDic<idx> clrTbl; // 183
            ///static Pigment * pigment(const char * name);
            static sClr pigment(idx iclr=-1, idx a=255);
            static sClr inverse(const sClr & clr){ return sClr(~(clr.rgb()),255); }
        };


}  // end namespace slib

#endif // Vstd_clr_hpp


