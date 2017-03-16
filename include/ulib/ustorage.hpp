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
#ifndef sLib_usrstorage_h
#define sLib_usrstorage_h

#include <slib/core.hpp>
#include <qlib/QPrideBase.hpp>

namespace slib {
    class sUsrStorage {
        public:
            static void initManager(sQPrideBase * qpb);
            static const sUsrStorage * manager() { return _s_manager; }

            // keep in sync with scheme_names in ustorage.cpp
            enum EScheme {
                eObjData, //! permanent object file - stored in globally-accessible location for as long as the owner object exists
                eReqData, //! large request data (small request data is stored in db) - stored in globally-accessible location for as long as owner request exists
                eCached, //! cached result - stored in globally-accessible location for some amount of time
                eLocalTmp //! machine-local temporary file - stored on a local disk for fast i/o, not accessible from other machines
            };

            static const char * getSchemeString(EScheme scheme);
            static const char * getSchemeString(const char * scheme_or_uri);
            static bool validScheme(const char * scheme_or_uri) { return getSchemeString(scheme_or_uri); }
            //! returns scheme code of valid user-storage URI or scheme name; return value for unknown or invalid scheme is undefined
            static EScheme getScheme(const char * scheme_or_uri);

            static const char * vmakeURI(sStr & out_uri, EScheme scheme, const sHiveId & id, const char * name_fmt, va_list ap);
            static const char * makeURI(sStr & out_uri, EScheme scheme, const sHiveId & id, const char * name_fmt, ...);
            static const char * decomposeURI(EScheme & out_scheme, sHiveId & out_id, const char * uri);

            bool open(sMex * file_out, idx mex_flags, const char * uri_fmt, ...) const;
            bool mkdir(const char * uri_fmt, ...) const;
            bool exists(const char * uri_fmt, ...) const;
            bool isDir(const char * uri_fmt, ...) const;
            bool isSymLink(const char * uri_fmt, ...) const;
            const char * followSymLink(sStr & out, const char * uri_fmt, ...) const;
            idx size(const char * uri_fmt, ...) const;
            idx time(const char * uri_fmt, ...) const;
            bool copy(const char * to, const char * from_glob, bool recurse = true) const;
            bool move(const char * to, const char * from_glob) const;
            bool remove(const char * uri_glob, bool recurse = true) const;

            bool open(sMex * file_out, idx mex_flags, EScheme scheme, const sHiveId & id, const char * name) const;
            bool mkdir(EScheme scheme, const sHiveId & id, const char * name) const;
            bool exists(EScheme scheme, const sHiveId & id, const char * name) const;
            bool isDir(EScheme scheme, const sHiveId & id, const char * name) const;
            bool isSymLink(EScheme scheme, const sHiveId & id, const char * name) const;
            const char * followSymLink(sStr & out, EScheme scheme, const sHiveId & id, const char * name) const;
            idx size(EScheme scheme, const sHiveId & id, const char * name) const;
            idx time(EScheme scheme, const sHiveId & id, const char * name) const;
            bool remove(EScheme scheme, const sHiveId & id, const char * name_glob, bool recurse = true) const;

            struct ListResult {
                struct Entry {
                    EScheme scheme;
                    sHiveId id;
                    const char * uri;
                    const char * name;
                    idx size;
                    idx mtime;
                    bool is_dir;
                    bool is_symlink;

                    Entry();
                };

                sVec<Entry> entries;
                sStr buf;
            };
            idx list(ListResult & out, const char * uri_glob_fmt, ...) const;
            idx list(ListResult & out, EScheme scheme, const sHiveId * ids, idx num_ids, const char * name_glob) const;

            // low-level API
            const char * vdiskPath(sStr & out, const char * uri_fmt, va_list ap, bool no_generate_new = false, bool make_dirs = false) const;
            const char * diskPath(sStr & out, const char * uri_fmt, ...) const;
            const char * diskPath(sStr & out, EScheme scheme, const sHiveId & id, const char * name, bool no_generate_new = false, bool make_dirs = false, const char * uri = 0) const;
            bool vstat(struct stat & out, sStr & out_path, const char * uri_fmt, va_list ap) const;
            bool stat(struct stat & out, sStr & out_path, const char * uri_fmt, ...) const;
            bool stat(struct stat & out, sStr & out_path, EScheme scheme, const sHiveId & id, const char * name) const;

        protected:
            sUsrStorage();

            static sUsrStorage * _s_manager;
            sQPrideBase * _qpb;
            mutable sVar _cache;
    };
};

#endif
