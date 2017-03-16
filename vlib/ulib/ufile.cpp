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
#include <ulib/ufile.hpp>
#include <slib/std/file.hpp>
#include <sys/stat.h>
#include <sys/types.h>

using namespace slib;

void sUsrFile::fixOldPath(void) const
{
    // fix data for old imported files when implementation was poor
    sStr path(sMex::fExactSize);
    const char * prop = "path";
    propGet(prop, &path);
    if( path ) {
        const char * fnm = sFilePath::nextToSlash(path);
        if( const_cast<sUsrFile*>(this)->propSet("orig_name", fnm) == 1 ) {
            bool gz = false;
            if( strcmp(path.ptr(sLen(path) - 3), ".gz") == 0 ) {
                path.cut0cut(-4);
                gz = true;
            }
            sFilePath nm(path, "%%flnmx.*"), ext(path, "%%ext%s", gz ? ".gz" : "");
            if( const_cast<sUsrFile*>(this)->propSet("ext", ext) == 1 ) {
                sStr p;
                propGet("_dir", &p);
                sDir files;
                files.find(sFlag(sDir::bitFiles), p, nm);
                const idx offset = p.length() + nm.length() - 3;
                bool ok = true;
                for(const char * pnm = files.ptr(); pnm; pnm = sString::next00(pnm)) {
                    if( pnm[0] ) {
                        sStr e("%s_.%s", p.ptr(), &pnm[offset]);
                        ok &= sFile::rename(pnm, e);
                    }
                }
                if( ok ) {
                    const_cast<sUsrFile*>(this)->propDel(prop, 0, 0);
                }
            }
        }
    }
}

const char * sUsrFile::addFilePathname(sStr & buf, bool overwrite, const char* key, ...) const
{
    fixOldPath();
    sStr added, reserved, k;
    sCallVarg(k.vprintf, key);
    makeFileName(added, k);
    getFile(reserved);
    const char * r = sFilePath::nextToSlash(reserved);
    return (r && strcmp(added, r) == 0) ? 0 : TParent::addFilePathname(buf, overwrite, "%s", k.ptr());
}

bool sUsrFile::delFilePathname(const char* key, ...) const
{
    fixOldPath();
    sStr del, reserved, k;
    sCallVarg(k.vprintf, key);
    makeFileName(del, k);
    getFile(reserved);
    const char * r = sFilePath::nextToSlash(reserved);
    return (r && strcmp(del, r) == 0) ? false : TParent::delFilePathname("%s", k.ptr());
}

// TEMP !! for old obj conversion only!!!
const char * sUsrFile::getFilePathnameX(sStr & buf, const sStr & key, bool check_existence) const
{
    fixOldPath();
    return TParent::getFilePathnameX(buf, key, check_existence);
}

const char* sUsrFile::getFile(sStr & buf) const
{
    fixOldPath();
    sStr ext(".");
    propGet("ext", &ext);
    return TParent::getFilePathname(buf, "%s", ext.ptr());
}

bool sUsrFile::setFile(const char * file_path, const char* name /* = 0 */, const char * ext /* = 0 */, udx file_sz /* = 0 */)
{
    udx q = 0;
    if( file_path && file_path[0] ) {
        const char* n = sFilePath::nextToSlash((name && name[0]) ? name : file_path);
        q += propSet("orig_name", n);
        sStr e(".");
        if( !ext ) {
            ext = strchr(n, '.');
        }
        if( ext && ext[0] ) {
            // cut leading dotS
            while( ext[0] == '.' && ext[0] != '\0' ) {
                ++ext;
            }
            if( ext[0] ) {
                e.printf("%s", ext);
            }
        }
        q += propSet("ext", &e[1]); //extension w/o dot
        if( !file_sz ) {
            file_sz = sFile::size(file_path);
        }
        q += propSetU("size", file_sz);
        q += propSet("name", n);
        if( q == 4 ) {
            sStr buf;
            if( TParent::addFilePathname(buf, true, "%s", e.ptr()) ) {
                if( sFile::rename(file_path, buf) || file_sz == 0 ) {
                    ++q;
                }
            }
        }
    }
    return q == 5;
}
