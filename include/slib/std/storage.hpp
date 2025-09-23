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
#ifndef sLib_storage_h
#define sLib_storage_h

#include <slib/core/str.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/vec.hpp>

#include <sys/stat.h>

#ifndef WIN32
#include <pthread.h>
#endif

namespace slib {
    class sStorageScheme {
        public:
            sStorageScheme(const char * names00);
            virtual ~sStorageScheme() {}

            idx dimNames() const { return _names.dim(); }
            const char * getName(idx i = 0) const { return static_cast<const char *>(_names.id(i)); }

            bool matches(const char * uri_or_name, idx len = 0) const;

            bool addRoot(const char * root_path, real weight = 1, udx min_free = 0, sStr * err_out = 0);
            void updateRootsUsability();
            idx dimRoots() const { return _roots.dim(); }
            bool rootsOK(sStr * err_out = 0);
            const char * getRoot(idx iroot) const { return _root_paths00.ptr(_roots[iroot].root_path_offset); }
            const char * getRandomRoot();
            virtual const char * getRandomRootFor(const char * uri, idx len = 0) { return getRandomRoot(); }

            virtual const char * printPathSuffix(sStr & out, const char * uri, idx len = 0, bool allow_empty_path = false) const;
            virtual const char * printPathSuffix(char * buf_out, idx buf_out_len, const char * uri, idx len = 0, bool allow_empty_path = false) const;

            virtual const char * printDump(sStr & out, const char * newline = "\n") const;


            static const idx max_name_len = 8;

            static idx extractSchemeName(char * buf_out, const char * uri_or_name, idx len = 0);

        protected:
            struct Root {
                sStorageScheme * scheme;
                idx index;
                idx root_path_offset;
                idx canary_path_offset;
                real weight;
                udx min_free;
                udx cur_free;
                idx cur_free_checked;
                udx total_size;
                udx usability;
                bool is_dead;

#ifndef WIN32
                pthread_cond_t usability_cond;
                pthread_mutex_t usability_mutex;
                pthread_t usability_tid;
#endif
                Root();
                ~Root();
            };

            sDic<idx> _names;
            sVec<Root> _roots;
            sStr _root_paths00;

            static const udx print_path_suffix_max_levels = 2;
            static const udx print_path_suffix_level_base = 1000;

            udx updateRootUsability(idx iroot);
            bool updateRootIsDead(idx iroot);

        private:
#ifndef WIN32
            static void * updateRootUsabilityWorker(void * param);
#endif
    };

    class sStorage {
        public:
            static bool addScheme(sStorageScheme * scheme);
            static sStorageScheme * getScheme(const char * uri_or_scheme_name, idx len = 0);
            static idx getSchemes(sVec<sStorageScheme*> & schemes_out);
            enum EMakePathMode {
                eCreateDirs = 0,
                eNoCreateDirs,
                eOnlyExisting
            };
            static const char * makePath(sStr & out, struct stat * stat_out, const char * uri, idx len = 0, EMakePathMode = eCreateDirs);
            static const char * makePath(char * buf_out, idx buf_out_len, struct stat * stat_out, const char * uri, idx len = 0, EMakePathMode = eCreateDirs);
            static const char * sMexUriCallback(char * buf_out, idx buf_out_len, const char * uri_or_path, bool only_existing)
            {
                if( sStorageScheme::extractSchemeName(0, uri_or_path) ) {
                    return makePath(buf_out, buf_out_len, 0, uri_or_path, 0, only_existing ? eOnlyExisting : eCreateDirs);
                } else {
                    return uri_or_path;
                }
            }

            struct FindResult {
                public:
                    idx dim() const { return _entries.dim(); }
                    const char * absUri(idx i) const { return _buf00.ptr(_entries[i].abs_offset); }
                    const char * relUri(idx i) const { return _buf00.ptr(_entries[i].rel_offset); }
                    const char * diskPath(idx i) const { return _buf00.ptr(_entries[i].disk_offset); }
                    bool isDir(idx i) { return _entries[i].is_dir; }
                    void empty() {
                        _entries.empty();
                        _buf00.cut0cut();
                    }
                    sStr * buf00() { return &_buf00; }
                    void push(const char * uri_dir, idx uri_dir_len, const char * rel_uri, const char * disk_path, bool is_dir);
                private:
                    struct Entry {
                        idx abs_offset;
                        idx rel_offset;
                        idx disk_offset;
                        bool is_dir;
                    };
                    sVec<Entry> _entries;
                    sStr _buf00;
            };
            static idx findPaths(FindResult & out, const char * uri_dir, idx uri_dir_len = 0, const char * fileglob = "*", idx recurse_depth = 0, bool no_dirs = false);

            static const char * printDump(sStr & out);

        private:
            sStorage();
    };
};

#endif
