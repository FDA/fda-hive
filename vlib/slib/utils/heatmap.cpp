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
#include <slib/utils/heatmap.hpp>

using namespace slib;

struct MinMax { real min, max; MinMax(){min=REAL_MAX; max=REAL_MIN;} };

#define TBLSTART     for ( idx ir=0; ir<rowCnt; ++ir) { \
        idx iR=(rowsToUse && rowsToUse->dim()) ? *rowsToUse->ptr(ir) : ir; \
        for ( idx ic=0; ic<colCnt; ++ic) { \
            idx iC= (columnsToUse && columnsToUse->dim()) ? *columnsToUse->ptr(ic) : ic; \
            if (iC > tbl->cols()-1 || iR > tbl->rows()) return -1;\
            v=tbl->rval(iR,iC);



#define TBLEND }\
        }

static const sClr default_min_clr(0, 0, 255);
static const sClr default_mid_clr(0, 0, 0);
static const sClr default_max_clr(255, 0, 0);
static const sClr default_missing_clr(0xEE, 0xEE, 0xEE);

void sHeatmap::ColorLimits::set(const sClr & min_clr, const sClr & mid_clr, const sClr & max_clr, const sClr & missing_clr)
{
    min_hue = min_clr.hue360();
    mid_hue_min = mid_hue_max = mid_clr.hue360();
    max_hue = max_clr.hue360();

    min_chroma = min_clr.chroma1();
    mid_chroma = mid_clr.chroma1();
    max_chroma = max_clr.chroma1();

    if (!min_chroma) {
        min_hue = mid_hue_min;
    }
    if (!max_chroma) {
        max_hue = mid_hue_max;
    }
    if (!mid_chroma) {
        mid_hue_min = min_hue;
        mid_hue_max = max_hue;
    }

    min_lightness = min_clr.lightness1();
    mid_lightness = mid_clr.lightness1();
    max_lightness = max_clr.lightness1();

    missing = missing_clr;
}

sHeatmap::ColorLimits::ColorLimits()
{
    set(default_min_clr, default_mid_clr, default_max_clr, default_missing_clr);
}


idx sHeatmap::generateRangeMap( sVec < sVec< real > > * rgbs, sTabular * tbl, sVec < idx > * columnsToUse, sVec <idx > * rowsToUse, idx colorMethod)
{
    sVec < MinMax > Span;

    idx rowCnt=(rowsToUse && rowsToUse->dim()) ? rowsToUse->dim() : tbl->rows();
    idx colCnt=(columnsToUse && columnsToUse->dim()) ? columnsToUse->dim() : tbl->cols()-1;

    rgbs->add(rowCnt);
    for (idx i=0; i < rowCnt; i++)
        rgbs->ptr(i)->add(colCnt);


    if(colorMethod==0)
    {
        Span.add(1);
    }
    else if(colorMethod==1)
    {
        Span.add(rowCnt);
    }
    else if(colorMethod==2)
    {
        Span.add(colCnt);
    }

    sStr val;
    real v;

    for ( idx ir=0; ir<rowCnt; ++ir)
    {
            idx iR=(rowsToUse && rowsToUse->dim()) ? *rowsToUse->ptr(ir) : ir;
            for ( idx ic=0; ic<colCnt; ++ic)
            {
                idx iC= (columnsToUse && columnsToUse->dim()) ? *columnsToUse->ptr(ic) : ic;
                if (iC > tbl->cols()-1 || iR > tbl->rows())
                    return -1;
                v=tbl->rval(iR,iC, NAN, false, true);
                if (isnan(v))
                    continue;

                MinMax* span = Span.ptr( colorMethod==0 ? 0 : (colorMethod==1 ? ir : ic) );

                if (span->min > v)
                    span->min = v;
                if (span->max < v)
                    span->max = v;
                val.cut(0);
            }
    }

    for ( idx ir=0; ir<rowCnt; ++ir)
    {
            idx iR=(rowsToUse && rowsToUse->dim()) ? *rowsToUse->ptr(ir) : ir;
            for ( idx ic=0; ic<colCnt; ++ic)
            {
                idx iC= (columnsToUse && columnsToUse->dim()) ? *columnsToUse->ptr(ic) : ic;
                if (iC > tbl->cols()-1 || iR > tbl->rows()) return -1;
                v=tbl->rval(iR,iC, NAN, false, true);

                if (isnan(v))
                {
                    (*rgbs)[ir][ic] = -1;
                    val.cut(0);
                    continue;
                }

                MinMax* span = Span.ptr( colorMethod==0 ? 0 : (colorMethod==1 ? ir : ic) );
                (*rgbs)[ir][ic]=(v-span->min)/(span->max-span->min);
                val.cut(0);
            }
    }

    return 1;

}


idx sHeatmap::generatePNG(const char * filename,  sVec < sVec< real > > * rgbs, idx cx, idx cy , sHeatmap::ColorLimits * limits, sVec < sVec < sClr > > * colors)
{
   colors->add(rgbs->dim());
   for (idx i=0; i < rgbs->dim(); i++)
       colors->ptr(i)->add((*rgbs)[i].dim());

    for ( idx ir=0; ir<rgbs->dim(); ++ir) {
        for ( idx ic=0; ic<(*rgbs)[ir].dim(); ++ic) {
            real val=(*rgbs)[ir][ic];
            sClr clr;

            if (val == -1)
            {
                clr.setHCY(89,0,50);
                (*colors) [ir][ic]=clr;
                continue;
            }

            limits->makeColor(clr, val);

            (*colors) [ir][ic]=clr;
        }

    }


    return 1;
}
