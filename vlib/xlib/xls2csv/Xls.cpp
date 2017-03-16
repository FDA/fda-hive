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
#include <slib/core/def.hpp>

#ifdef HAS_LIBXLS
    #include <xlib/xls2csv/Workbook.h>
    #include <xlib/xls2csv/Worksheet.h>
#endif

#include <slib/core/str.hpp>
#include <slib/std/file.hpp>

#include <string.h>
#include <alloca.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace Xls {

    const char * csvEncode(char *buf, const char *str, size_t buf_size)
    {
        char *_buf = (char *)alloca(buf_size);
        char *pt = _buf, *b2 = _buf + buf_size - 4, *b1 = _buf + buf_size - 3, *b = _buf + buf_size - 2;
        static char ch2escape[] = "\",", ch2quote[] = "";

        bool quoted = false;
        for ( ; *str; str++ ) {
            if ( strchr(ch2quote, *str) ) {
                if ( pt >= b2 )
                    return NULL;
                *pt++ = *str;
                        quoted = true;
            }
            if ( strchr(ch2escape, *str) ) {
                if ( pt >= b1 )
                    return NULL;
                *pt++ = '\\';
                *pt++ = *str;
            }
            else {
                if ( pt >= b )
                    return NULL;
                *pt++ = *str;
            }
        }

        if ( pt >= b1 )
            return NULL;
        *pt = '\0';

        size_t n = pt - _buf;
        pt = buf;
        if ( quoted )
            *pt++ = '"';
        strcpy(pt, _buf);
        pt += n;
        if ( quoted )
            *pt++ = '"';
        *pt = '\0';

        return buf;
    }

    int xls2csv(const char *xlsFile, const char *outDir, const char *reSheetNameFilter, slib::sStr * out_error)
    {
#ifdef HAS_LIBXLS
        Workbook wb;
        if ( !wb.Open(xlsFile) )
            return -1;

        struct stat st;
        if ( stat(outDir, &st) != 0 )
            mkdir(outDir, S_IRWXU);

        return wb.ExportCsvs(outDir);
#else // HAS_LIBXLS
        if( out_error ) {
            out_error->addString("This HIVE deployment was built without support for Excel 97/2000/2002/2003 .xls files.");
        }
        return 0;
#endif // HAS_LIBXLS
    }

    int xlsx2csv(const char *xlsFile, const char *outDir, const char *reSheetNameFilter, slib::sStr * out_error)
    {
        using namespace slib;

        bool caller_made_outdir = sDir::exists(outDir);

        sStr cmdline00;
        cmdline00.add("xlsx2csv.py");
        cmdline00.add("-a");
        cmdline00.add(xlsFile);
        cmdline00.add(outDir);
        if( sPS::execute00(cmdline00) != 0 ) {
            if( !caller_made_outdir ) {
                sDir::removeDir(outDir, false);
            }
            if( out_error ) {
                out_error->addString("XLSX to CSV converter failed.");
            }
            return 0;
        }

        sDir csv_list;
        csv_list.list(sFlag(sDir::bitFiles), outDir, "*.csv");
        int cnt = 0;
        for(idx i=0; i<csv_list.dimEntries(); i++) {
            if( sFile::size(csv_list.getEntryPath(i)) ) {
                cnt++;
            } else {
                // xlsx2csv.py generates 0-size files for unused sheets. Delete these.
                sFile::remove(csv_list.getEntryPath(i));
            }
        }
        if( cnt <= 0 && !caller_made_outdir ) {
            sDir::removeDir(outDir, false);
        }
        if( cnt == 0 && out_error ) {
            out_error->addString("Empty XLSX file.");
        }
        return cnt;
    }

    int excel2csv(const char *xlsFile, const char *outDir, const char *reSheetNameFilter, slib::sStr * out_error)
    {
        using namespace slib;

        const char * xls_ext = strrchr(xlsFile, '.');
        if( sIs(xls_ext, ".xls") ) {
            return xls2csv(xlsFile, outDir, reSheetNameFilter, out_error);
        } else if( sIs(xls_ext, ".xlsx") ) {
            return xlsx2csv(xlsFile, outDir, reSheetNameFilter, out_error);
        } else {
            if( out_error ) {
                out_error->printf("Unrecognized file extension '%s'", xls_ext ? xls_ext : "");
            }
            return 0;
        }
    }
}
