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
#ifndef sLib_utils_heatmap_hpp
#define sLib_utils_heatmap_hpp

#include <slib/utils/tbl.hpp>
#include <slib/std/clr.hpp>

namespace slib {

    class sHeatmap
    {
        public:
            //! Min/max HSL parameters for a heatmap, for turning a 0-1 heat value into a color
            struct ColorLimits
            {
                real min_hue, mid_hue_min, mid_hue_max, max_hue;
                real min_chroma, mid_chroma, max_chroma;
                real min_lightness, mid_lightness, max_lightness;
                sClr missing;

                void set(const sClr & min_clr, const sClr & mid_clr, const sClr & max_clr, const sClr & missing_clr);

                /*! \param[out] result resulting color
                    \param val heat value; 0 is mininum, 1 is maximum, and values
                               outside 0-1 range will be colored as missing */
                void makeColor(sClr & result, real val) const
                {
                    val *= 2;
                    if (val >= 0 && val <= 1) {
                        real h = min_hue + val * (mid_hue_min - min_hue);
                        real c = min_chroma + val * (mid_chroma - min_chroma);
                        real l = min_lightness + val * (mid_lightness - min_lightness);
                        result.setHCL(h, c, l);
                    } else if (val >= 1 && val <= 2) {
                        val -= 1;
                        real h = mid_hue_max + val * (max_hue - mid_hue_max);
                        real c = mid_chroma + val * (max_chroma - mid_chroma);
                        real l = mid_lightness + val * (max_lightness - mid_lightness);
                        result.setHCL(h, c, l);
                    } else {
                        result = missing;
                    }
                }

                ColorLimits();
            };
            static idx generateRangeMap(sVec<sVec<real> > * rgbs, sTabular * tbl, sVec<idx> * columnsToUse, sVec<idx> * rowsToUse, idx colorMethod);
            //static idx createImage (const char * filename, idx height, idx width, const char * title, sVec < sVec < sClr > > & colors, idx cx, idx cy);
            static idx generatePNG(const char * filename, sVec<sVec<real> > * rgbs, idx cx, idx cy, ColorLimits * limits, sVec<sVec<sClr> > * colors);
            //static idx create (const char * filename, idx height, idx width, const char * title, sVec < sVec < sClr > > & colors, idx cx, idx cy);

            static idx generatePNG(const char * filename, sTabular * tbl, sVec<idx> * columnsToUse, sVec<idx> * rowsToUse, idx colorMethod, idx cx, idx cy, sHeatmap::ColorLimits * limits, sVec<sVec<real> > * pvals = 0)
            {
                sVec<sVec<real> > vals;
                sVec<sVec<sClr> > colors;

                if( !pvals ) {
                    pvals = &vals;
                }
                sHeatmap::generateRangeMap(pvals, tbl, columnsToUse, rowsToUse, colorMethod);
                sHeatmap::generatePNG(filename, pvals, cx, cy, limits, &colors);
                return 1;
            }

    };

}
#endif // sLib_utils_heatmap_hpp
