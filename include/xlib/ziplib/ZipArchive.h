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

#define MAX_PATH    1024

#include <stdint.h>
#include <string>
#include <list>

struct zip;

namespace ZipLib {

    class ZipArchive
    {
    public:
        struct Entry {
            std::string Name;
            size_t Size;
            size_t CompressedSize;
            time_t ModificationTime;
            int64_t Index;

            Entry()
            {
                Size = 0;
                CompressedSize = 0;
                ModificationTime = 0;
                Index = -1;
            }

            bool IsValid() const { return Index >= 0; }
            operator bool() const { return IsValid(); }
        };

        typedef void(*FindCallback)(const Entry &entry);

    private:
        zip *_pz;

    private:
        bool unzipFile(int64_t fileIndex, const char *dir) const;

    public:
        ZipArchive(const char *zipFile);
        virtual ~ZipArchive();

        std::list<Entry> List() const;
        void List(FindCallback callback) const;

        Entry Find(const char *fileName) const;

        bool UnzipFile(const char *file, const char *dir) const;
        bool Unzip(const char *dir) const;

        std::string Error() const;
    };
}
