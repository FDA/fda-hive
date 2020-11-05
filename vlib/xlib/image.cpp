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
#include <slib/std.hpp>
#include <slib/utils/heatmap.hpp>
#include <xlib/image.hpp>

#include <wand/MagickWand.h>

#include <errno.h>
#include <time.h>

static udx wand_genesis_cnt = 0;

static void safeWandGenesis()
{
    if( wand_genesis_cnt == 0 ) {
        MagickWandGenesis();
    }
    wand_genesis_cnt++;
}

static void safeWandTerminus()
{
    if( wand_genesis_cnt && --wand_genesis_cnt == 0 ) {
        MagickWandTerminus();
    }
}

static time_t str2time_t(const char * t_str)
{
    time_t t = 0;
    const idx len = sLen(t_str);
    if( len ) {
        sStr in("%s", t_str);
        char fmt[] = "%Y-%m-%dT%H:%M:%S%z";
        if( strchr("-+", in[len - 6]) && in[len - 3] == ':' ) {
            in[len - 3] = in[len - 2];
            in[len - 2] = in[len - 1];
            in[len - 1] = '\0';
        } else if( strchr("-+", in[len - 5]) ) {
        } else {
            fmt[sizeof(fmt) - 3] = '\0';
        }
        struct tm tmp_tm, *tm_cur;

        time_t t_cur = time(0);
        tm_cur = localtime(&t_cur);

        strptime(in.ptr(), fmt, &tmp_tm);

        tmp_tm.tm_isdst = tm_cur->tm_isdst;

        int sec = tm_cur->tm_gmtoff - tmp_tm.tm_gmtoff;
        //t = mktime(&tmp_tm);

        //return mktime(gmtime(mktime(tmp_tm)));

        t = sec + mktime(&tmp_tm); // gmt time
    }
    return t;
}

sImage::sImage(const char* filename)
    : m_width(0), m_height(0), m_imgtype(0), m_filename(0)
{
    MagickWand * wand = NULL;

    safeWandGenesis();

    wand = NewMagickWand();

    if( MagickReadImage(wand, filename) == MagickTrue ) {
        m_width = MagickGetImageWidth(wand);
        m_height = MagickGetImageHeight(wand);
        MagickGetImageResolution(wand, &m_xRes, &m_yRes);

        _buf.addString(MagickGetImageFormat(wand));
        _buf.add0();
        _buf.addString(filename);
        _buf.add0();
        m_imgtype = _buf.ptr(0);
        m_filename = sString::next00(m_imgtype);

        const char * p = MagickGetImageProperty(wand, "date:create");
        m_date_create = str2time_t(p);
        MagickRelinquishMemory((void *)p);

        p = MagickGetImageProperty(wand, "date:modify");
        m_date_modify = str2time_t(p);
        MagickRelinquishMemory((void *)p);

        m_date_taken = std::min(m_date_create, m_date_modify);

        p = MagickGetImageProperty(wand, "exif:DateTime");
        if( p ) {
            char fmt[] = "%Y:%m:%d %H:%M:%S";
            struct tm tmp_tm;
            strptime(p, fmt, &tmp_tm);
            time_t t = mktime(&tmp_tm); // gmt time
            if( m_date_taken > t ) {
                m_date_taken = t;
            }
            MagickRelinquishMemory((void *)p);
        }
    }
    if( wand ) {
        wand = DestroyMagickWand(wand);
    }
}

sImage::~sImage()
{
    safeWandTerminus();
}

bool sImage::ok(void) const
{
    return m_filename != 0;
}

udx sImage::width() const
{
    return m_width;
}

udx sImage::height() const
{
    return m_height;
}

real sImage::xResolution() const
{
    return m_xRes;
}

real sImage::yResolution() const
{
    return m_yRes;
}

const char* sImage::type() const
{
    return m_imgtype;
}

const char* sImage::filename() const
{
    return m_filename;
}

time_t sImage::created() const
{
    return m_date_create;
}

time_t sImage::modified() const
{
    return m_date_modify;
}

time_t sImage::taken() const
{
    return m_date_taken;
}

sImage * sImage::convert(const char* pic_dst, const char* new_type)
{
    sImage * retval = 0;
    if( pic_dst && pic_dst[0] && new_type && new_type[0] ) {
        MagickWand * wand = NewMagickWand();
        if( wand ) {
            MagickBooleanType rc = MagickReadImage(wand, m_filename);
            if( rc == MagickTrue ) {
                rc = MagickSetImageFormat(wand, new_type);
                if( rc == MagickTrue ) {
                    rc = MagickWriteImage(wand, pic_dst);
                    if( rc == MagickTrue ) {
                        retval = new sImage(pic_dst);
                    }
                }
            }
            wand = DestroyMagickWand(wand);
        }
    }
    return retval;
}

sImage * sImage::resize(const char* pic_dst, udx width, udx height, EAspect keepAspect)
{
    sImage * retval = 0;
    if( pic_dst && pic_dst[0] && (width || height) ) {
        MagickWand * wand = NewMagickWand();
        if( wand ) {
            MagickBooleanType rc = MagickReadImage(wand, m_filename);
            if( rc == MagickTrue ) {
                if( keepAspect == eAspectWidth ) {
                    height = m_height * width / m_width;
                } else if( keepAspect == eAspectHeight ) {
                    width = m_width * height / m_height;
                }
                rc = MagickResizeImage(wand, width, height, LanczosFilter);
                if( rc == MagickTrue ) {
                    rc = MagickWriteImage(wand, pic_dst);
                    if( rc == MagickTrue ) {
                        retval = new sImage(pic_dst);
                    }
                }
            }
            wand = DestroyMagickWand(wand);
        }
    }
    return retval;
}

sImage * sImage::crop(const char* pic_dst, udx x, udx y, udx width, udx height)
{
    sImage * retval = 0;
    MagickWand * wand = NULL;

    wand = NewMagickWand();

    MagickBooleanType rc = MagickReadImage(wand, m_filename);
    if( rc == MagickTrue ) {
        rc = MagickCropImage(wand, width, height, x, y);
        if( rc == MagickTrue ) {
            rc = MagickWriteImage(wand, pic_dst);
            if( rc == MagickTrue ) {
                retval = new sImage(pic_dst);
            }
        }
    }
    if( wand ) {
        wand = DestroyMagickWand(wand);
    }
    return retval;
}

static inline void pixelSetColor(PixelWand * pixel, sClr & clr)
{
    PixelSetRed(pixel, clr.r1());
    PixelSetGreen(pixel, clr.g1());
    PixelSetBlue(pixel, clr.b1());
}

static inline real linear(real val, real min, real max)
{
    return min + val * (max - min);
}

//static
bool sImage::generateHeatmap(const char * filename, const sVec < sVec< real > > * values, idx cx, idx cy, const sHeatmap::ColorLimits * limits)
{
    if( !values || !values->dim()) {
        return false;
    }

    idx height = cy * values->dim();
    idx width = 0;
    for( idx ir=0; ir<values->dim(); ir++ ) {
        width = sMax<idx>(width, cx * values->ptr(ir)->dim());
    }
    if( !width ) {
        return false;
    }

    safeWandGenesis();

    MagickWand * wand = NewMagickWand();
    PixelWand * pixel = NewPixelWand();

    sClr clr = limits->missing;
    pixelSetColor(pixel, clr);
    PixelIterator * pixel_iter = 0;
    MagickBooleanType status = MagickNewImage(wand, width, height, pixel); // initialize grey image
    if( status == MagickFalse ) {
#ifdef _DEBUG
        // FIXME - replace with proper logging functions
        ExceptionType severity;
        char * err = MagickGetException(wand, &severity);
        fprintf(stderr, "%s:%u: ImageMagick error: %s\n", __FILE__, __LINE__, err);
        MagickRelinquishMemory(err);
#endif
        goto CLEANUP;
    }

    pixel_iter = NewPixelIterator(wand);
    for( idx ir=0; ir<values->dim(); ir++ ) {
        for( idx icy=0; icy<cy; icy++ ) {
            size_t num_pixels = 0;
            PixelWand ** pixels = PixelGetNextIteratorRow(pixel_iter, &num_pixels);
            for( idx ic=0; ic<values->ptr(ir)->dim(); ic++ ) {
                real val = *values->ptr(ir)->ptr(ic);
                if( val < 0 ) {
                    continue; // keep pre-existing grey pixel
                }
                limits->makeColor(clr, val);
                for( idx icx=0; icx<cx; icx++ ) {
                    pixelSetColor(pixels[ic * cx + icx], clr);
                }
            }
            // write pixels into the image
            status = PixelSyncIterator(pixel_iter);
            if( status == MagickFalse ) {
#ifdef _DEBUG
                // FIXME - replace with proper logging functions
                ExceptionType severity;
                char * err = PixelGetIteratorException(pixel_iter, &severity);
                fprintf(stderr, "%s:%u: ImageMagick error: %s\n", __FILE__, __LINE__, err);
                MagickRelinquishMemory(err);
#endif
                goto CLEANUP;
            }
        }
    }

    status = MagickWriteImage(wand, filename);
    if( status == MagickFalse ) {
#ifdef _DEBUG
        // FIXME - replace with proper logging functions
        ExceptionType severity;
        char * err = MagickGetException(wand, &severity);
        fprintf(stderr, "%s:%u: ImageMagick error: %s\n", __FILE__, __LINE__, err);
        MagickRelinquishMemory(err);
#endif
        goto CLEANUP;
    }

  CLEANUP:

    pixel_iter = DestroyPixelIterator(pixel_iter);
    wand = DestroyMagickWand(wand);
    pixel = DestroyPixelWand(pixel);

    safeWandTerminus();

    return status == MagickTrue;
}
