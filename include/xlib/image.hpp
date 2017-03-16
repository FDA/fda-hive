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
#ifndef xLib_image_hpp
#define xLib_image_hpp

#include <slib/core.hpp>
#include <slib/utils/heatmap.hpp>

namespace slib {
    //! Image file manipulation
    class sImage
    {
        public:
            sImage(const char* filename);
            ~sImage();

            //! image was loaded successfully
            bool ok(void) const;
            //! width in pixels
            udx width(void) const;
            //! height in pixels
            udx height(void) const;
            //! horizontal resolution in pixels per inch
            real xResolution(void) const;
            //! vertical resolution in pixels per inch
            real yResolution(void) const;
            //! image type (e.g. "PNG", "JPEG", "GIF")
            const char* type(void) const;
            //! image filename
            const char* filename(void) const;
            time_t created(void) const;
            time_t modified(void) const;
            //! time photo was taken, as recorded in EXIF metadata
            time_t taken(void) const;

            typedef enum
            {
                eAspectExact = 1,
                eAspectWidth,
                eAspectHeight
            } EAspect;



            sImage * convert(const char* pic_dst, const char* new_type);
            sImage * resize(const char* pic_dst, udx width, udx height, EAspect keepAspect);
            //! Crop an image to form a new image.
            /*! The image to be cropped is the object. After cropping the image, a new image is formed and is stored in \a pic_dst.
             * \param pic_dst path where to save the cropped image
             * \param x left offset of cropped image
             * \param y top offset of cropped image
             * \param width width of cropped image
             * \param height height of cropped image
             * \returns new allocated cropped sImage object, or 0 on failure */
            sImage * crop(const char* pic_dst, udx x, udx y, udx width, udx height);

            //! Generate a heatmap image
            /*! \param filename where to write the heatmap image
             *  \param values vector of vectors of heat values, from 0 to 1; -1 means missing data
             *  \param cx width of each output cell in pixels
             *  \param cy height of each output cell in pixels
             *  \param limits min/max HSL parameters for the heatmap, for turning a 0-1 value into a color
             *  \returns true on success */
            static bool generateHeatmap(const char * filename, const sVec < sVec< real > > * values, idx cx, idx cy, const sHeatmap::ColorLimits * limits);


        private:
            udx m_width;
            udx m_height;
            const char * m_imgtype;
            const char * m_filename;
            sStr _buf;
            time_t m_date_create;
            time_t m_date_modify;
            time_t m_date_taken;
            real m_xRes;
            real m_yRes;
    };
};

#endif // xLib_image_hpp
