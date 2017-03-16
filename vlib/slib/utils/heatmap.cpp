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

//tbl->printCell(val, iR, iC);


#define TBLEND }\
        }

static const sClr default_min_clr(0, 0, 255); // blue
static const sClr default_mid_clr(0, 0, 0); // black
static const sClr default_max_clr(255, 0, 0); // red
static const sClr default_missing_clr(0xEE, 0xEE, 0xEE); // light-grey

void sHeatmap::ColorLimits::set(const sClr & min_clr, const sClr & mid_clr, const sClr & max_clr, const sClr & missing_clr)
{
    min_hue = min_clr.hue360();
    mid_hue_min = mid_hue_max = mid_clr.hue360();
    max_hue = max_clr.hue360();

    min_chroma = min_clr.chroma1();
    mid_chroma = mid_clr.chroma1();
    max_chroma = max_clr.chroma1();

    // special handing for grayscale-to-color transition
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

/*color method = 0: every cell is evaluated separately, so the max and min are computed over the entire table provided)
 *color method = 1: every row is evaluated separately, so the max and min are computed per row
 *color method = 2: every column is evaluated separately, so the max and the min are computed per column
 */

idx sHeatmap::generateRangeMap( sVec < sVec< real > > * rgbs, sTabular * tbl, sVec < idx > * columnsToUse, sVec <idx > * rowsToUse, idx colorMethod)
{
    //MinMax span; //!holds the current maximums and minimums of all the rows
    sVec < MinMax > Span; //!holds the current maximums and minimums of all the rows

    idx rowCnt=(rowsToUse && rowsToUse->dim()) ? rowsToUse->dim() : tbl->rows(); //! the total number of rows to go through
    idx colCnt=(columnsToUse && columnsToUse->dim()) ? columnsToUse->dim() : tbl->cols()-1;//! the total number of columns to go through

    rgbs->add(rowCnt);
    for (idx i=0; i < rowCnt; i++)
        rgbs->ptr(i)->add(colCnt);


    //Allocates the proper amount of memory based on the coloring method
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

    // iterator of your table
    sStr val;
    real v;

    //!this for loop will find the minimums and the maximums in the entire table
    for ( idx ir=0; ir<rowCnt; ++ir)
    {
            idx iR=(rowsToUse && rowsToUse->dim()) ? *rowsToUse->ptr(ir) : ir;
            for ( idx ic=0; ic<colCnt; ++ic)
            {
                idx iC= (columnsToUse && columnsToUse->dim()) ? *columnsToUse->ptr(ic) : ic;
                if (iC > tbl->cols()-1 || iR > tbl->rows())
                    return -1;
                v=tbl->rval(iR,iC, NAN, false, true);
                //accumulate minimums and maximums
                // can use iR, iC, val
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

    //! this for loop will generate the correct number for the slot
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
                (*rgbs)[ir][ic]=(v-span->min)/(span->max-span->min); //this will make the value in the slot iR, iC between 0 and 1
                val.cut(0);
            }
    }

    return 1;

}

/*
void sHeatmap_setRGB(png_byte *ptr, sClr & clr)
{
    ptr[0] = clr.r();
    ptr[1] = clr.g();
    ptr[2] = clr.b();
}

idx sHeatmap::createImage (const char * filename, idx height, idx width, const char * title, sVec < sVec < sClr > > & colors, idx cx, idx cy)
{
       idx code = 0;
       FILE * imageFile;
       imageFile = fopen(filename, "wb");
       // creating structures to generate the png
       png_structp png_ptr;
       png_infop info_ptr;
       png_bytep row;

       //char buffer [4096];

      // Initialize write structure
      png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      if (png_ptr == NULL) {
         ::printf("Could not allocate write struct\n");
         return -1;
      }

      // Initialize info structure
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL) {
         ::printf("Could not allocate info struct\n");
         code = 1;
         return -1;
      }

      png_init_io(png_ptr, imageFile);

     // Write header (8 bit colour depth)
     png_set_IHDR(png_ptr, info_ptr, width*cx, height*cy,
           8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
           PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

     // Set title
     if (title != NULL) {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = sConvPtr( "Title" , char );
        title_text.text = sConvPtr( title, char );
        png_set_text(png_ptr, info_ptr, &title_text, 1);
     }

     png_write_info(png_ptr, info_ptr);

    // Allocate memory for one row (3 bytes per pixel - RGB)
    row = (png_bytep) malloc(3 * width*cx * sizeof(png_byte));

    // Write image data
    idx x, y;
    for (y=0 ; y<height ; y++) {
        for (x=0 ; x<width ; x++) {
            for (idx icx=0 ; icx<cx ; icx++) {
                sHeatmap_setRGB(&(row[cx*x*3+3*icx]), colors [y][x]);
            }
        }
        for( idx icy=0; icy<cy; icy++) {
            png_write_row(png_ptr, row);
        }
    }

    // End write
    png_write_end(png_ptr, NULL);

    if (imageFile != NULL) fclose(imageFile);
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (info_ptr != NULL) png_destroy_info_struct(png_ptr, &info_ptr);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

    if (row != NULL) free(row);

   return 1;
}*/

//can also pass rgbs by refference to make more readable
idx sHeatmap::generatePNG(const char * filename,  sVec < sVec< real > > * rgbs, idx cx, idx cy , sHeatmap::ColorLimits * limits, sVec < sVec < sClr > > * colors)
{
   colors->add(rgbs->dim());
   for (idx i=0; i < rgbs->dim(); i++)
       colors->ptr(i)->add((*rgbs)[i].dim());

    for ( idx ir=0; ir<rgbs->dim(); ++ir) {
        for ( idx ic=0; ic<(*rgbs)[ir].dim(); ++ic) {
            real val=(*rgbs)[ir][ic];
            sClr clr;

            //if there was no value for a cell in the table, the color will be set to gray
            if (val == -1)
            {
                clr.setHCY(89,0,50);
                (*colors) [ir][ic]=clr;
                continue;
            }

            limits->makeColor(clr, val);

            (*colors) [ir][ic]=clr;
            //sStr buf;
            //::printf ("color == %s\n", clr.printHex(buf));
        }

        //::printf ("\n");
    }

    //sHeatmap::createImage (filename, rgbs->dim(), (*rgbs)[0].dim(), "HIVE-heatmap", *colors, cx, cy);

    return 1;
}
