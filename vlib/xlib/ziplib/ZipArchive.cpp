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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <zip.h>
#include <slib/utils/ziplib/ZipArchive.h>

using namespace std;

namespace ZipLib {

    ZipArchive::ZipArchive(const char *zipFile)
    {
        int err;
        _pz = zip_open(zipFile, 0, &err);
    }

    ZipArchive::~ZipArchive()
    {
        if ( _pz )
            zip_close(_pz);
    }

    list<ZipArchive::Entry> ZipArchive::List() const
    {
        list<Entry> entries;

        struct zip_stat st;
        zip_stat_init(&st);

        zip_int64_t n = zip_get_num_entries(_pz, 0);

        for (int i = 0; i<n; i++) {
            if (zip_stat_index(_pz, i, 0, &st) == 0){
                Entry ent;
                ent.Name = st.name;
                ent.Size = st.size;
                ent.CompressedSize = st.comp_size;
                ent.ModificationTime = st.mtime;
                ent.Index = st.index;
                entries.push_back(ent);
            }
        }

        return entries;
    }

    void ZipArchive::List(FindCallback callback) const
    {
        struct zip_stat st;
        zip_stat_init(&st);

        zip_int64_t n = zip_get_num_entries(_pz, 0);

        for ( int i = 0; i<n; i++ ) {
            if ( zip_stat_index(_pz, i, 0, &st) == 0 ){
                Entry ent;
                ent.Name = st.name;
                ent.Size = st.size;
                ent.CompressedSize = st.comp_size;
                ent.ModificationTime = st.mtime;
                ent.Index = st.index;

                callback(ent);
            }
        }
    }

    ZipArchive::Entry ZipArchive::Find(const char *fileName) const
    {
        zip_int64_t index = zip_name_locate(_pz, fileName, ZIP_FL_NOCASE);
        if (index == -1)
            return Entry();

        struct zip_stat st;
        if (zip_stat_index(_pz, index, 0, &st) == -1)
            return Entry();

        Entry entry;
        entry.Name = st.name;
        entry.Size = st.size;
        entry.CompressedSize = st.comp_size;
        entry.ModificationTime = st.mtime;
        entry.Index = st.index;

        return entry;
    }

    bool exists(const char *path)
    {
        struct stat buf;
        int result = stat(path, &buf);
        return result != -1;
    }

    bool createDir(const char *dir)
    {
        if ( exists(dir) )
            return true;

        char path[MAX_PATH];
        strcpy(path, dir);
        char *p = path;
        while ( p = strchr(p, '/'), p ) {
            *p = '\0';
            if ( !exists(path) ) {
                if ( mkdir(path
#ifndef _WIN32
                    , S_IRUSR | S_IWUSR
#endif // !_WIN32
                    ) == -1 )
                    return false;
            }
            *p++ = '/';
        }

        if ( !exists(dir) ) {
            if ( mkdir(dir
#ifndef _WIN32
                , S_IRUSR | S_IWUSR
#endif // !_WIN32
                ) == -1 )
                return false;
        }

        return true;
    }

    bool ZipArchive::unzipFile(int64_t fileIndex, const char *dir) const
    {
        struct zip_stat st;
        zip_stat_init(&st);
        if ( zip_stat_index(_pz, fileIndex, 0, &st) == -1 )
            return false;

        zip_file *zf = zip_fopen_index(_pz, fileIndex, 0);
        if ( zf == NULL )
            return false;

        char filePath[MAX_PATH];
        strcpy(filePath, dir);
        strcat(filePath, "/");
        strcat(filePath, st.name);
        char *pSlash = strrchr(filePath, '/');
        *pSlash = '\0';
        if ( !createDir(filePath) ){
            zip_fclose(zf);
            return false;
        }
        *pSlash = '/';

        FILE *f = fopen(filePath, "wb");
        if ( f == NULL ) {
            zip_fclose(zf);
            return false;
        }

        bool result = true;
        int n;
        char buf[1024];
        while ( (n = zip_fread(zf, buf, sizeof(buf))) > 0 ) {
            if ( fwrite(buf, n, 1, f) != 1 ) {
                result = true;
                break;
            }
        }

        fclose(f);
        zip_fclose(zf);

        return result;
    }

    bool ZipArchive::UnzipFile(const char *file, const char *dir) const
    {
        zip_int64_t index = zip_name_locate(_pz, file, ZIP_FL_NOCASE);
        if ( index == -1 )
            return false;

        return unzipFile(index, dir);
    }

    bool ZipArchive::Unzip(const char *dir) const
    {
        struct zip_stat st;
        zip_stat_init(&st);

        zip_int64_t n = zip_get_num_entries(_pz, 0);
        for (int i = 0; i<n; i++) {
            if ( !unzipFile(i, dir) )
                return false;
        }

        return true;
    }

    string ZipArchive::Error() const
    {
        int ze, se;
        zip_error_get(_pz, &ze, &se);

        char buf[128];
        zip_error_to_str(buf, sizeof(buf), ze, se);

        return string(buf);
    }
}
