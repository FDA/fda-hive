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
    //! Base class for a storage scheme that distributes file paths over multiple storage root directories; intended for extension by child classes, and for internal use in slib::sStorage and low-level code
    /*! It is expected that high-level code will simply pass URIs to sMex/sFil/sStr, make use of sStorage's
        static methods for corner cases, and won't need to directly use sStorageScheme's API at all.

        Basic usage:
        \code
            sStorageScheme s = new sStorageScheme("foo"_"bar"__); // to handle "foo://" and "bar://" uris
            s->addRoot("/mnt/foobar1");
            s->addRoot("/mnt/foobar2");
            s->updateRootsUsability();
            sStorage::add(s);
            sStr buf;
            printf("%s", sStorage::makePath(buf, 0, "foo://123456/hello.txt", false)); // will print "/mnt/foobar1/456/123456/hello.txt" or similar
        \endcode
     */
    class sStorageScheme {
        public:
            //! Create storage scheme with given scheme names
            sStorageScheme(const char * names00);
            virtual ~sStorageScheme() {}

            //! number of names that the scheme has
            idx dimNames() const { return _names.dim(); }
            //! get scheme's name
            const char * getName(idx i = 0) const { return static_cast<const char *>(_names.id(i)); }

            //! Check if the storage scheme uses the scheme name (alone or given as part of a uri)
            bool matches(const char * uri_or_name, idx len = 0) const;

            //! Add a storage root directory to the scheme. Must contain special "_storage" subdirectory.
            /*! \param root_path directory on disk under which files will be stored
                \param weight preference factor for picking this root in getRandomRoot(); 0 means do not pick
                \param min_free do not pick this root if disk free space in bytes drops below this level
                \note you must call updateRootsUsability() after finishing adding all roots */
            bool addRoot(const char * root_path, real weight = 1, udx min_free = 0, sStr * err_out = 0);
            //! Must be called after all roots are added to update their free space etc.
            void updateRootsUsability();
            //! Number of storage root directories
            idx dimRoots() const { return _roots.dim(); }
            //! Check if all successfully added roots still exist, and weren't lost e.g. due to network mount failure
            bool rootsOK(sStr * err_out = 0);
            //! Get a specific storage root directory, guaranteed to be '/'-terminated
            const char * getRoot(idx iroot) const { return _root_paths00.ptr(_roots[iroot].root_path_offset); }
            //! Pick a random storage root directory (directories with more free space and higher weight are preferred)
            const char * getRandomRoot();
            //! Pick a random storage root directory suitable for a given URI, using scheme-specific logic
            /*! \param uri URI which is valid for the current scheme, but *not* present on disk
                \returns pointer to '/'-terminated root storage directory, or 0 on failure */
            virtual const char * getRandomRootFor(const char * uri, idx len = 0) { return getRandomRoot(); }

            //! From a uri, extract/mangle the part of the disk path that goes after the root directory, using scheme-specific logic
            /*! The default sStorageScheme implementation assumes uri is in the form schemename://hiveID/foo.ext
                and transforms "scheme://1234567/foo.txt" to "567/234/1234567/foo.txt"
                \param[out] out buffer to which to append the path suffix
                \param allow_empty_path allow uris consisting only of "domain" component, e.g. "scheme://1234567/"
                \returns pointer to printed suffix in out on success, or 0 on failure (e.g. if uri is invalid) */
            virtual const char * printPathSuffix(sStr & out, const char * uri, idx len = 0, bool allow_empty_path = false) const;
            //! From a uri, extract/mangle the part of the disk path that goes after the root directory, using scheme-specific logic
            /*! The default sStorageScheme implementation assumes uri is in the form schemename://hiveID/foo.ext
                transforms "schemename://1234567/foo.ext" to "567/234/1234567/foo.ext"
                \param[out] buf_out buffer in which to print the path suffix
                \param buf_out_len size of buf_out, generally expected to be PATH_MAX
                \param allow_empty_path allow uris consisting only of "domain" component, e.g. "scheme://1234567/"
                \returns pointer to start of buf_out on success, or 0 on failure (e.g. if uri is invalid or
                         buf_out_len is too short to hold the printed suffix) */
            virtual const char * printPathSuffix(char * buf_out, idx buf_out_len, const char * uri, idx len = 0, bool allow_empty_path = false) const;

            //! For debugging
            virtual const char * printDump(sStr & out, const char * newline = "\n") const;

            // utility functions

            static const idx max_name_len = 8; //!< maximum allowed length of a scheme name (not including "://")

            //! Extract and lowercase a scheme name from a uri
            /*! \param[out] buf_out buffer of sStorageScheme::max_name_len + 1 bytes to hold the lowercased name and
                                    terminal 0 (note that "://" will *not* be extracted); or 0 to not use
                \param uri_or_name uri (or bare scheme name, or scheme name + "://") from which to extract the name
                \param len length of uri_or_name, or 0 to interpret uri_or_name as 0-terminated string
                \returns length of scheme name, including "://" if any; or 0 on failure */
            static idx extractSchemeName(char * buf_out, const char * uri_or_name, idx len = 0);

        protected:
            struct Root {
                sStorageScheme * scheme; //!< owner scheme
                idx index; //!< own index in parent scheme
                idx root_path_offset; //!< offset for root path in _root_paths
                idx canary_path_offset; //!< offset for "_storage" canary subdir path in _root_paths
                real weight; //!< requested preference for picking this root in getRandomRoot() / getRandomRootFor() (0 to avoid)
                udx min_free; //!< minimum required free space in bytes in mount containing root directory
                udx cur_free; //!< current free space in bytes in bytes in mount containing root directory
                idx cur_free_checked; //!< time when cur_free was last checked
                udx total_size; //!< current total size in bytes of mount containing root directory
                udx usability; //!< actual preference for picking this root in getRandomRoot() / getRandomRootFor(), range 0..100
                bool is_dead; //!< true if "_storage" canary subdir disappeared (so mount got unmounted)

#ifndef WIN32
                // Updating cur_free could be slow on a busy network mount, so do it in a separate thread
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

            static const udx print_path_suffix_max_levels = 2; //!< number of disk path dir levels to prefix an object ID "host": e.g. "1234567" -> "567/234/1234567" means 2 levels
            static const udx print_path_suffix_level_base = 1000; //!< disk dir prefixes to an object ID "host" are produced modulo this base

            udx updateRootUsability(idx iroot);
            bool updateRootIsDead(idx iroot);

        private:
#ifndef WIN32
            static void * updateRootUsabilityWorker(void * param);
#endif
    };

    //! Translate URIs into file paths distributed over multiple storage roots depending on custom storage schemes
    /*! All methods are static; the class is an interface to an internal singleton. */
    class sStorage {
        public:
            //! Register a storage scheme to the global storage list
            /*! \note global storage list takes ownership of the scheme, and will destruct it at program exit
                \note adding a scheme automatically sets sMex::uri_callback = sStorage::sMexUriCallback */
            static bool addScheme(sStorageScheme * scheme);
            //! Get a storage scheme appropriate for a given uri or scheme name
            static sStorageScheme * getScheme(const char * uri_or_scheme_name, idx len = 0);
            //! Get all registered storage schemes
            static idx getSchemes(sVec<sStorageScheme*> & schemes_out);
            enum EMakePathMode {
                eCreateDirs = 0, //!< If path doesn't exist on disk, create parent directories (and requested directory itself if it ends in '/')
                eNoCreateDirs, //!< If path doesn't exist on disk, print the path but don't create its parent directories
                eOnlyExisting //!< If path doesn't exist on disk, print nothing
            };
            //! Make a disk path for a uri
            /*! \param[out] out buffer in which to print the disk path
                \param[out] stat_out will be filled with struct stat for the disk file/directory, if it exists; or 0 to not use
                \param len length of uri, or 0 to interpret uri as 0-terminated string
                \param only_existing whether to retrieve path only if it already exists on disk, and whether to
                                     auto-create parent directories if it doesn't exist on disk
                \returns pointer to start of printed path in out, or 0 on failure */
            static const char * makePath(sStr & out, struct stat * stat_out, const char * uri, idx len = 0, EMakePathMode = eCreateDirs);
            //! Make a disk path for a uri
            /*! \param[out] buf_out buffer in which to print the disk path
                \param buf_out_len size of buf_out, generally expected to be PATH_MAX
                \param[out] stat_out will be filled with struct stat for the disk file/directory, if it exists; or 0 to not use
                \param len length of uri, or 0 to interpret uri as 0-terminated string
                \param only_existing whether to retrieve path only if it already exists on disk, and whether to
                                     auto-create parent directories if it doesn't exist on disk
                \returns pointer to start of printed path in buf_out, or 0 on failure */
            static const char * makePath(char * buf_out, idx buf_out_len, struct stat * stat_out, const char * uri, idx len = 0, EMakePathMode = eCreateDirs);
            //! Implementation for sMex::uri_callback
            /*! \param[out] buf_out buffer to hold printed disk path
                \param buf_out_len size of buf_out, generally expected to be PATH_MAX
                \param uri_or_path either a URI to resolve into a path, or an already resolved disk file path
                \param only_existing if true, return 0 for uris that do not resolve to an existing file on disk;
                                     if false, and uri is valid but resolves but to a file that does not exist,
                                     return the path *and* create parent directories to allow the file to be
                                     immediately created/opened.
                \returns uri_or_path if it already looks like a disk path; or printed disk path in buf_out; or 0 on failure */
            static const char * sMexUriCallback(char * buf_out, idx buf_out_len, const char * uri_or_path, bool only_existing)
            {
                if( sStorageScheme::extractSchemeName(0, uri_or_path) ) {
                    return makePath(buf_out, buf_out_len, 0, uri_or_path, 0, only_existing ? eOnlyExisting : eCreateDirs);
                } else {
                    return uri_or_path;
                }
            }

            //! Result of sStorage::find()
            struct FindResult {
                public:
                    //! number of files/dirs found
                    idx dim() const { return _entries.dim(); }
                    //! absolute uri of found file/dir
                    const char * absUri(idx i) const { return _buf00.ptr(_entries[i].abs_offset); }
                    //! relative uri (relative to uri_dir in sStorage::find() call) of found file/dir
                    const char * relUri(idx i) const { return _buf00.ptr(_entries[i].rel_offset); }
                    //! disk path of found file/dir
                    const char * diskPath(idx i) const { return _buf00.ptr(_entries[i].disk_offset); }
                    //! whether the entry is a directory
                    bool isDir(idx i) { return _entries[i].is_dir; }
                    //! clear the results
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
            //! Find files/directories matching a fileglob within a specified uri path
            /*! \param[out] out list of retrieved uris and disk paths for found files/directories
                \param fileglob case-insensitive shell-style glob matching names of files/directories to retrieve
                \param recurse_depth max depth to recurse; non-recursive if 0; infinite if negative
                \param no_dirs find only files, no directories */
            static idx findPaths(FindResult & out, const char * uri_dir, idx uri_dir_len = 0, const char * fileglob = "*", idx recurse_depth = 0, bool no_dirs = false);

            //! for debugging
            static const char * printDump(sStr & out);

        private:
            sStorage(); //!< This class should not be constructed - static methods only
    };
};

#endif
