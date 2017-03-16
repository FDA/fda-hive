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
#ifndef sLib_std_file_hpp
#define sLib_std_file_hpp

#include <slib/core/dic.hpp>
#include <slib/core/rc.hpp>
#include <slib/core/sIO.hpp>
#include <slib/std/string.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>

#define DEFAULT_MKTEMP_PATTERN "_tmp_XXXXXX"

namespace slib {
    //! Mostly OS-independent functions for filesystem operations on individual files
    class sFile
    {
        public:
            //! Remove a file
            /*! \returns true on success */
            static bool remove(const char * file);
            //! Copy a file from \a filesrc to \a filedst
            /*! \param doAppend if destination exists, append to it instead of replacing it
                \param follow_link if source is a symlink, link destination to source's target instead of
                                   copying source's target into destination
                \returns true if copying succeeded. Attempting to copy a file to itself is treated as failure. */
            static bool copy(const char * filesrc, const char * filedst, bool doAppend = false, bool follow_link = true);
            //! Create a symbolic link at \a slink pointing to \a pathname
            /*! \returns true if creating the link succeeded or if this link already existed */
            static bool symlink(const char *pathname, const char *slink);
            //! Move a file from \a filesrc to \a filedst
            /*! \note when moving a file between filesystems, the file will be copied and the source then deleted
                \returns true if move succeeded */
            static bool rename(const char * filesrc, const char * filedst);
            //! Change a file's permissions
            /*! \param mode permissions flags; 0 is interpreted as 0777 (rwx for everyone) */
            static void chmod(const char * file, idx mode = 0);
            //! Check if a file exists
            /*! \param follow_link if true and file is a symbolic link, check whether the link's target exists */
            static bool exists(const char * file, bool follow_link = true);
            //! Check if a file is a symbolic link
            static bool isSymLink(const char * file);
            //! Check if two paths represent the same inode on disk
            /*! \param follow_link if true and one or both files are symbolic links, link targets will be compared */
            static bool sameInode(const char * file1, const char * file2, bool follow_link = true);
            //! Resolve a symbolic link
            /*! \param file link path; must <em>not</em> be allocated inside outPath
                \param[out] outPath resolved link target path
                \param[out] outSt if non-0, stat of resolved link target
                \param maxLevels number of levels of links to resolve, in case of links pointing to links
                \returns outPath.ptr() on success, or 0 on failure */
            static const char * followSymLink(const char * file, sStr & outPath, struct stat * outSt = 0, idx maxLevels = sIdxMax);

            typedef idx pid;
            //! Modification timestamp of file
            static idx time(const char * flnm, bool follow_link = true);
            //! Access timestamp of file
            static idx atime(const char * flnm, bool follow_link = true);
            //! Set an existing file's access and modification timestamp
            /*! \param acc_and_mod_time timestamp to set; if 0, current time will be used
                \warning if the file does not exist, it will not be created */
            static bool touch(const char * file, idx acc_and_mod_time = 0);
            //! Set file attributes (owner, mode, etc.)
            static bool setAttributes(const char * file, const struct stat * st);
            static bool fsetAttributes(int fd, const struct stat * st);
            //! File size in bytes
            /*! \param follow_link if true and file is a symbolic link, will find size of link target
                                   rather than the size of the symlink itself
                \returns file size in bytes, or 0 on failure */
            static idx size(const char * flnm, bool follow_link = true);

            //! Callback for use in sFile::mktemp
            /*! \returns true if \a path is a satisfactory temp file (e.g. the file exists, is writable, and the filename is appropriate) */
            typedef bool (tempPathTester)(const char * path);
            //! Creates a new temporary 0-size file in given directory, and returns its path
            /*! \param[out] outPath created file's path
                \param dir directory where to create the file
                \param extension create file with given extension, or without extension if 0
                \param pattern filename format. If non-0, must be a string with a "XXXXXX" substring. The "XXXXXX" substring will be replaced with random characters.
                \param tester callback to test whether a generated path is satisfactory; if 0, no test is performed
                \returns pointer to start of \a outPath, or 0 on error */
            static const char * mktemp(sStr & outPath, const char * dir, const char * extension=0, const char * pattern=0, tempPathTester tester=0);

            //! Can be used to reimplement followSymLink() with different semantics
            class LinkFollower
            {
            protected:
                idx _level;
                sStr & _path;
                struct stat * _st;

                //! Check whether to finish following early
                virtual bool finished() { return false; }

                //! Check that the current path and stat info are valid
                virtual bool valid() { return true; }

            public:
                LinkFollower(sStr & path, struct stat * st) : _level(0), _path(path), _st(st) {}
                virtual ~LinkFollower() {}
                const char * follow(const char * file);
                struct stat * getStat() { return _st; }
                sStr & getPath() { return _path; }
            };

            //! Used for determining if two files have the same inode, but ignoring atime/mtime etc.
            class InodeKey
            {
            protected:
#ifdef WIN32
#else
                dev_t st_dev;
                ino_t st_ino;
#endif

            public:
                InodeKey() { sSet(this); }
                InodeKey(const struct stat * st)
                {
                    set(st);
                }
                void set(const struct stat * st)
                {
#ifdef WIN32
#else
                    st_dev = st->st_dev;
                    st_ino = st->st_ino;
#endif
                }

                inline bool operator==(const InodeKey & rhs) const
                {
#ifdef WIN32
                    return true;
#else
                    return st_dev == rhs.st_dev && st_ino == rhs.st_ino;
#endif
                }
                inline bool operator!=(const InodeKey & rhs) const
                {
                    return !operator==(rhs);
                }
            };
    };

    //! Mostly OS-independent functions for filesystem operations on directories
    class sDir
    {
        public:
            enum bitFlags
            {
                bitSubdirs = 0,
                bitFiles,
                bitSelf,
                bitUpper,
                bitRecursive,
                bitOpenable,
                bitSubdirSlash,
                bitDrives,
                bitNamesOnly,
                bitNoExtension,
                bitFollowLinks,
                bitNoLinks,
                bitEntryFlags,
            //bitRelative,
            //bitSinglePath,
            };

            //! The operating system's native directory separator symbol
            static const char sysSep;
            //! The directory separator symbol used when printing/parsing paths
            static const char sep;

            enum EEntryFlags
            {
                fIsDir = 1<<0, //!< entry is a directory
                fIsSymLink = 1<<1, //!< entry is a symbolic link
                fIsDangling = 1<<2, //!< entry is an invalid symbolic link
                fIsDuplicate = 1<<3 //!< same disk node was already found earlier by list()/find()
            };

            sDir(void) : _listSepar(sMex::fExactSize)
            {
                _listUpdated = false;
            }
            //! Non-recusively list the contents of a directory
            idx list(idx flags, const char * dirini = 0, const char * wildcard = 0, const char * separ = 0, const char * relativePath = 0);
            //! Potentially recursively list the contents of a directory
            idx find(idx flags, const char * dirini = 0, const char * wildcard = 0, const char * separ = 0, idx maxFind = 0, const char * relativePath = 0);
            const char * findMany(idx flags, const char * dirs, const char * flnms, const char * separ = 0, idx maxfind = 0);
            const char * includes(const char * incdir, const char * flnm, idx maxlen, const char * incstart, const char * incend, const char * incsymbs, bool skipRepeat, bool isRecursive);
            static const char * aliasResolve(sStr & filenameResolved, const char * configFile, const char * section, const char * filenamesrc, bool ensureExist = false, bool pathIsGlob = false);
            //! Change current working directory
            static bool chDir(const char* dir);
            //! Delete a directory
            /*! \param recursive delete the directory's contents too */
            static bool removeDir(const char* dir, bool recursive = true);
            //! Create a directory
            static bool makeDir(const char* dir, idx mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IWOTH);
            //! Copy an entire directory
            /*! \param src source directory or valid symlink to directory
             *  \param dst destination directory or valid symlink to directory; will be created if it does not exist
             *  \param follow_symlink when copying symlinks, instead of creating a new link in dst pointing to same target, copy the target itself
             *  \param[out] pnumcopied number of filesystem entries (files, directories, symlinks) copied or created
             *  \returns true on success; false if src is not a directory, or src and dst are the same directory, or some part of src could not be copied */
            static bool copyDir(const char * src, const char * dst, bool follow_symlink = false, idx * pnumcopied = 0);
            //! Copy part of a directory; like "mkdir -p '$dst' && cp -r '$src'/$wildcard '$dst'"
            /*! \param src source directory or valid symlink to directory
             *  \param wildcard shell-style glob for matching contents of src to copy
             *  \param dst destination directory or valid symlink to directory; will be created if it does not exist
             *  \param follow_symlink when copying symlinks, instead of creating a new link in dst pointing to same target, copy the target itself
             *  \param[out] pnumcopied number of filesystem entries (files, directories, symlinks) copied or created
             *  \returns sRC::zero on success; appropriate RC code if src is not a directory, or src and dst are the same directory, or some part of '$src'/$wildcard could not be copied */
            static sRC copyContents(const char * src, const char * wildcard, const char * dst, bool follow_symlink = false, idx * pnumcopied = 0);
            //! Rename (move) a directory
            static bool rename(const char * src, const char * dst);
            //! Check whether a path is a directory that exists on disk
            static bool exists(const char * path);
            //! Check whether a path is a symbolic link
            inline static bool isSymLink(const char * path)
            {
                return sFile::isSymLink(path);
            }
            //! Free space in bytes in the filesystem containing specified path
            static udx freeSpace(const char * path);
            //! Total size (used + free) in bytes of the filesystem containing specified path
            static udx fileSystemSize(const char * path);
            //! Total size in bytes of the contents of a directory
            /*! \param follow_link Set to true for network transmission size, and to false for disk usage */
            static idx size(const char * path, bool recursive = true, bool follow_link = false);

            //! Create a new temporary subdirectory in given directory, and returns its path
            static const char * mktemp(sStr & outPath, const char * dir, const char * pattern=0);

            //! Delete temporary files and subdirectories (as specified by pattern) in given directory.
            //! \returns number of top-level items cleaned
            static idx cleantemp(const char * dir, const char * pattern=0);
            //! Pointer into buffer of concatenated paths found by list() or find()
            inline const char * ptr(idx pos = 0) const
            {
                updateList();
                return _list00.ptr(pos);
            }
            //! Length of buffer of concatenated paths found by list() or find()
            inline idx length(void) const
            {
                updateList();
                return _list00.length();
            }
            //! Pointer into buffer of concatenated paths found by list() or find()
            inline const char * operator*() const
            {
                return ptr();
            }
            //! Pointer into buffer of concatenated paths found by list() or find()
            inline operator const char *() const
            {
                return ptr();
            }
            //! Check whether list() or find() found something
            inline operator bool() const
            {
                return dimEntries();
            }
            //! Empty list of entries found by list() or find()
            void cut()
            {
                _seen.empty();
                _entries.empty();
                _listUpdated = false;
                _list00.cut(0);
            }
            inline void shrink00(const char * separ = 0, idx dozeros = 0)
            {
                updateListSepar(separ);
                updateList();
                _list00.shrink00(separ, dozeros);
            }

            //! for debugging - bytes of memory used by sDir object
            idx totalMemUse() const;
            //! Number of entries found by list() or find()
            inline idx dimEntries() const
            {
                return _entries.dim();
            }
            //! Get bitwise-or list of sDir::EEntryFlags for a given entry from list() or find()
            inline idx getEntryFlags(idx i) const
            {
                return _entries[i].flags;
            }
            //! Get path for a given entry from list() or find(), in the format as specified by flags to list()/find()
            inline const char * getEntryPath(idx i) const
            {
                idx index = _entries[i].path_index;
                return index >= 0 ? _entryBuf.ptr(index) : 0;
            }
            //! Get absolute filesystem path for a given entry from list() or find()
            inline const char * getEntryAbsPath(idx i) const
            {
                idx index = _entries[i].abspath_index;
                return index >= 0 ? _entryBuf.ptr(index) : 0;
            }

            //! If entry found by list() or find() is a symbolic link and list()/find() was called with bitFollowLinks, retrieve the link's target
            inline const char * getEntryTargetPath(idx i) const
            {
                idx index = _entries[i].targetpath_index;
                return index >= 0 ? _entryBuf.ptr(index) : 0;
            }

            //! Clean up file name from characters which can break commands line
            /*
                \param applyToDisk true than actual file is renamed on disk, returns 0 if file is not found
             */
            static const char * cleanUpName(const char * path, sStr & buf, bool applyToDisk = false);
            //! Modifies path to be end with unique name by appending _## before FIRST extension ex: /a/b/c/a.fastq.gz -> /a/b/c/a_1.fastq.gz
            /*
                \return 0 on error on empty path
             */
            static const char * uniqueName(sStr & buf, const char * path, ...);

            // Must be public for non-HIVE code for Vahan
            mutable sStr _list00;

        protected:
            struct Entry
            {
                idx path_index;
                idx abspath_index;
                idx targetpath_index;
                idx flags; // bitwise-or of sDir::EEntryFlags

                void reset()
                {
                    path_index = abspath_index = targetpath_index = -1;
                    flags = 0;
                }

                Entry() { reset(); }
            };

            sVec<Entry> _entries;
            sDic<bool> _seen;
            sStr _entryBuf;
            mutable sStr _listSepar;
            mutable bool _listUpdated;

            void updateListSepar(const char * separ) const;
            void updateList() const;
    };

    //! Matches bash-like wildcard (glob) patterns for a filename. Multi-level path globs (foo*bar/*.txt) are currently not supported.
    class sFileGlob
    {
        public:
            sFileGlob()
            {
                sSet(this, 0);
            }
            bool compile(const char * glob = "*", bool caseSensitive = false);
            bool match(const char * path) const;
            virtual ~sFileGlob()
            {
                if( compiled )
                    regfree(&re);
            }
        protected:
            bool compiled;
            bool trivial;
            regex_t re;
    };

    //! Filesystem path formatting and mangling
    /*! Basic usage:
        \code
            const char * some_path = "/tmp/foo/bar.txt";
            const char * filename = sFilePath::nextToSlash(some_path);
            sFilePath new_path(some_path, "%%dir/%s.txt", "new_name"); // new_path is "/tmp/foo/new_name.txt"
            sFil f(new_path);
        \endcode */
    class sFilePath: public sStr
    {
        public:
            static const idx sSizeMax = sSizePage;

            typedef idx (*funCallback)(char * buf, const char * wherepos, const char * tmplt);
            funCallback funcall; //<! Custom interpreter for %% codes in makeName() / vmakeName()

            sFilePath(funCallback lfuncall = 0)
                : sStr()
            {
                funcall = lfuncall;
            }
            //! Create a file path using an existing path as template; see makeName() documentation
            sFilePath(const char * filename, const char * fmt, ...) __attribute__((format(printf, 3, 4)))
                : sStr()
            {
                funcall = 0;
                sCallVargPara2(vmakeName, 0, filename, fmt);
            }
            //! Extract and format components from a provided template path
            /*! Basic example:
             * \code
                 sFilePath p;
                 p.makeName("dir/subdir/file.txt", "%%dir/%%flnmx-%d.%%ext.bck", 2); // p is "dir/subdir/file-2.txt.bck"
             * \endcode
             * \param tmplt template file path, or "" to ignore; if tmplt is 0, makeName() is a no-op
             * \param fmt printf-like format string. Accepts the usual % codes, plus the following special %% codes:
             *  - path - entire template path
             *  - pathx - template path without file extension
             *  - dir - directory component of template path, *without* the final directory separator;
             *          or "" given a template path without any directory separators
             *  - dirx - final subdirectory component of template path, *with* terminal directory separator;
             *           or the entire template path if it is without any directory separators
             *  - flnm - filename component of template path
             *  - flnmx - filename component of template path without extension
             *  - ext - file extension of template path
             *  - st_size, st_atime, st_ctime, st_mtime, st_dev, st_attrib - stat() data for template path
             *  - Additional %% codes may be interpreted by this->funcall
             * \warning Avoid repeated directory separators in the template, these may be handled in surprising ways
             * \returns pointer to start of buffer */
            char * makeName(const char * tmplt, const char * fmt, ...) __attribute__((format(printf, 3, 4)))
            {
                char * res;
                sCallVargResPara2(res, vmakeName, 0, tmplt, fmt);
                return res;
            }
            //! Like makeName(), but prints at a position of your choice, not just at start of buffer
            /*! \param cutlen position where to print in buffer; or sIdxMax to append to current buffer */
            char * makeNameAt(idx cutlen, const char * tmplt, const char * fmt, ...) __attribute__((format(printf, 4, 5)))
            {
                char * res;
                sCallVargResPara2(res, vmakeName, cutlen, tmplt, fmt);
                return res;
            }

            char * vmakeName(idx cut, const char * tmplt, const char * fmt, va_list marker);

            enum eSimplifyPath
            {
                eSimplifySpaceDoNotTouch = 0,
                eSimplifySpaceRemove = 1,
                eSimplifySpaceReplaceWithUnderscore = 2
            };
            //! Eliminate ".." directory components as much as possible, eliminate "." components, simplify multiple directory separators to '/'
            char * simplifyPath(eSimplifyPath spacehandle = eSimplifySpaceDoNotTouch, const char * srcBuf = 0);
            //! Return substring after the last directory separator in path
            static char * nextToSlash(const char * path)
            {
                char * lastch = sString::searchSubstring(path, 0, "/"_"\\"__, sNotIdx, 0, 1);
                if( !lastch )
                    return sString::nonconst(path);
                else
                    return lastch + 1;
            }
            //! Replace windows-style \ directory separators with unix-style /
            char * pathReSlash(void)
            {
                char * path = ptr();
                cut(0);
                sString::searchAndReplaceSymbols(this, path, 0, "\\", "/", 0, true, true, false, false);
                return path;
            }
            char * composeWinStyleFileListPaths(const char * flnmlst, bool iszerolist);
            //! Append current working directory (with terminal '/') to buffer
            char * curDir(void);
            //! Replace buffer with path to executable for a given process id
            /*! \param pid process id whose executable to find if non-zero, or use current pid if 0
             *  \param justdir return just the directory of the executable
             *  \returns pointer to start of buffer */
            char * processPath(sFile::pid pid, bool justdir);
            //! Naively parse a command line and append executable name to the buffer
            /*! Basic example:
                \code
                    sFilePath p;
                    p.procNameFromCmdLine("/usr/bin/foobar-2.0 a.txt > b.csv"); // p now contains "foobar-2.0"
                \endcode
                \warning Spaces in executable name are parsed incorrectly. */
            char * procNameFromCmdLine(const char * cmdline);

    };

    class sFileSorted
    {
        public:
            FILE * fl;
            idx firstGI;
            idx lastGI;
            idx size;
            idx lastPos;
            idx backuplength;

            sFileSorted(const char * flnm, idx lbackuplength = 1024)
            {
                init(flnm, lbackuplength);
            }
            sFileSorted * init(const char * flnm, idx lbackuplength = 1024);

            idx search(idx gi); // searched for a gi in a sorted file
            idx search(const char * acc, const char * separ); // searches for the acc string in the sorted file
            idx searchReverse(idx pos, idx gi); // searches for the gi in reverse order from starting position

    };

    class sPipe
    {
        public:
            static idx exeFile(const char * commandLine, const char * stdinFile, const char * stdoutFile, const char * stderrFile);
            static idx exeFile00(const char * commandLine00, const char * stdinFile, const char * stdoutFile, const char * stderrFile);
            static char * exePipe(sIO * dst, const char * commandLine, const char * inputBuf, idx * retCode = 0);
            static idx exePipeBiDir(sIO * dst, const char *command, const char *inBuf, idx inSize);
            static char * exeSys(sStr * dst, const char * commandLine, const char * stdinBuf = 0, idx lenbuf = 0, idx * retCode = 0);
            typedef idx callbackFuncType(void * callbackParam, idx FSsize);
            //! execute a command in a thread while reporting back size of the given path/file
            /*
                \param callbackFunc returns 0 to indicate that stop was requested
            */
            static idx exeFS(sIO * dst, const char * cmdline, const char * inputBuf, callbackFuncType callbackFunc, void * callbackParam, const char * FSpath, const udx sleepSec = 1);
    };

    class sPS
    {
        public:
            typedef void callbackSetUp(void * callbackParam);

            static idx execute(const char * cmdline);
            static idx execute00(const char * cmdline00, callbackSetUp setupChild = 0, void * param = 0);
            static idx executeBG(const char * cmdline);
            static idx executeBG00(const char * cmdline00, callbackSetUp setupChild = 0, void * param = 0);

            enum eExecMode
            {
                eExec_Local,
                eExec_BG, // only affects exec()
                eExec_Extern
            };

            sPS(eExecMode execMode = eExec_Local, const char * ps_sript = 0)
                : m_mode(eExec_Local), m_extern(sMex::fExactSize)
            {
                setMode(execMode, ps_sript);
            }

            struct Stat
            {
                    idx pid;
                    char cmd[16 * 1024];
                    idx args; // index inside cmd
            };

            idx setMode(eExecMode mode, const char * ps_sript = 0);
            eExecMode getMode(void) const
            {
                return m_mode;
            }
            const char * const getScript(void) const
            {
                return m_extern.ptr();
            }
            //! List of processes, potentially filtered by user, command, or command-line arguments
            /*! \param[out] pi process list
                \param proc if non-0, output only processes exactly matching this command name
                \param currentUserOnly if true, output only processes for current user ID
                \param arguments if non-0, output only processes which have this string in command-line arguments
                \returns pi->dim() on success, or -1 on failure */
            idx getProcList(sVec<Stat> * pi, const char * proc, bool currentUserOnly, const char * arguments = 0) const;


            struct StatList
            {
                struct Stat
                {
                    idx pid;
                    idx cmdline_offset;
                    idx mem;

                    Stat() { pid = cmdline_offset = mem = -1; }
                };
                sVec<Stat> list;
                sStr buf;

                void empty()
                {
                    list.empty();
                    buf.empty();
                }
                idx dim() const { return list.dim(); }
                idx getPid(idx i) const { return list[i].pid; }
                idx getMem(idx i) const { return list[i].mem; }
                const char * getCmdline(idx i) const { return list[i].cmdline_offset >= 0 ? buf.ptr(list[i].cmdline_offset) : 0; }
            };
            enum EGetProcListFlags
            {
                fCurrentUserOnly = 1<<0,
                fGetMem = 1<<1,
                fMatchCaseInsensitive = 1<<2
            };
            idx getProcList(StatList & pi, idx flags = 0, const char * proc = 0, const char * argument = 0) const;
            idx killProcess(idx pid, idx signal = -9) const;
            idx getMem(idx pid) const;
            idx exec(const char * cmdline) const;
            idx exec00(const char * cmdline00, callbackSetUp setupChild = 0, void * param = 0) const;

        private:
            eExecMode m_mode;
            sStr m_extern;
            sStr m_extern_path;
    };
}
#endif

