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
    class sFile
    {
        public:
            static bool remove(const char * file);

            typedef bool(copyCallback)(void * callbackParam, const char * buffer, const idx len);

            static bool copy(const char * filesrc, const char * filedst, bool doAppend = false, bool follow_link = true, copyCallback func = NULL, void * callbackParam = NULL);
            static bool symlink(const char *pathname, const char *slink);
            static bool rename(const char * filesrc, const char * filedst, copyCallback func = NULL, void * callbackParam = NULL);
            static void chmod(const char * file, idx mode = 0);
            static bool exists(const char * file, bool follow_link = true);
            static bool isSymLink(const char * file);
            static bool sameInode(const char * file1, const char * file2, bool follow_link = true);
            static const char * followSymLink(const char * file, sStr & outPath, struct stat * outSt = 0, idx maxLevels = sIdxMax);

            typedef idx pid;
            static idx time(const char * flnm, bool follow_link = true);
            static idx atime(const char * flnm, bool follow_link = true);
            static bool touch(const char * file, idx acc_and_mod_time = 0);
            static bool setAttributes(const char * file, const struct stat * st);
            static bool fsetAttributes(int fd, const struct stat * st);
            static idx size(const char * flnm, bool follow_link = true);

            typedef bool (tempPathTester)(const char * path);
            static const char * mktemp(sStr & outPath, const char * dir, const char * extension=0, const char * pattern=0, tempPathTester tester=0);

            class LinkFollower
            {
            protected:
                idx _level;
                sStr & _path;
                struct stat * _st;

                virtual bool finished() { return false; }

                virtual bool valid() { return true; }

            public:
                LinkFollower(sStr & path, struct stat * st) : _level(0), _path(path), _st(st) {}
                virtual ~LinkFollower() {}
                const char * follow(const char * file);
                struct stat * getStat() { return _st; }
                sStr & getPath() { return _path; }
            };

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
            };

            static const char sysSep;
            static const char sep;

            enum EEntryFlags
            {
                fIsDir = 1<<0,
                fIsSymLink = 1<<1,
                fIsDangling = 1<<2,
                fIsDuplicate = 1<<3
            };

            sDir(void) : _listSepar(sMex::fExactSize)
            {
                _listUpdated = false;
            }
            idx list(idx flags, const char * dirini = 0, const char * wildcard = 0, const char * separ = 0, const char * relativePath = 0);
            idx find(idx flags, const char * dirini = 0, const char * wildcard = 0, const char * separ = 0, idx maxFind = 0, const char * relativePath = 0);
            const char * findMany(idx flags, const char * dirs, const char * flnms, const char * separ = 0, idx maxfind = 0);
            const char * includes(const char * incdir, const char * flnm, idx maxlen, const char * incstart, const char * incend, const char * incsymbs, bool skipRepeat, bool isRecursive);
            static const char * aliasResolve(sStr & filenameResolved, const char * configFile, const char * section, const char * filenamesrc, bool ensureExist = false, bool pathIsGlob = false);
            static bool chDir(const char* dir);
            static bool removeDir(const char * dir, bool recursive = true, sFile::copyCallback func = NULL, void * callbackParam = NULL);
            static bool makeDir(const char* dir, idx mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IWOTH);
            static bool copyDir(const char * src, const char * dst, bool follow_symlink = false, idx * pnumcopied = 0, sFile::copyCallback func = NULL, void * callbackParam = NULL);
            static sRC copyContents(const char * src, const char * wildcard, const char * dst, bool follow_symlink = false, idx * pnumcopied = 0, sFile::copyCallback func = NULL, void * callbackParam = NULL);
            static bool rename(const char * src, const char * dst, sFile::copyCallback func = NULL, void * callbackParam = NULL);
            static bool exists(const char * path);
            inline static bool isSymLink(const char * path)
            {
                return sFile::isSymLink(path);
            }
            static udx freeSpace(const char * path);
            static udx fileSystemSize(const char * path);
            static idx size(const char * path, bool recursive = true, bool follow_link = false);

            static const char * mktemp(sStr & outPath, const char * dir, const char * pattern=0);

            static idx cleantemp(const char * dir, const char * pattern = 0, sFile::copyCallback func = NULL, void * callbackParam = NULL);
            inline const char * ptr(idx pos = 0) const
            {
                updateList();
                return _list00.ptr(pos);
            }
            inline idx length(void) const
            {
                updateList();
                return _list00.length();
            }
            inline const char * operator*() const
            {
                return ptr();
            }
            inline operator const char *() const
            {
                return ptr();
            }
            inline operator bool() const
            {
                return dimEntries();
            }
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

            idx totalMemUse() const;
            inline idx dimEntries() const
            {
                return _entries.dim();
            }
            inline idx getEntryFlags(idx i) const
            {
                return _entries[i].flags;
            }
            inline const char * getEntryPath(idx i) const
            {
                idx index = _entries[i].path_index;
                return index >= 0 ? _entryBuf.ptr(index) : 0;
            }
            inline const char * getEntryAbsPath(idx i) const
            {
                idx index = _entries[i].abspath_index;
                return index >= 0 ? _entryBuf.ptr(index) : 0;
            }

            inline const char * getEntryTargetPath(idx i) const
            {
                idx index = _entries[i].targetpath_index;
                return index >= 0 ? _entryBuf.ptr(index) : 0;
            }

            static const char * cleanUpName(const char * path, sStr & buf, bool applyToDisk = false);
            static const char * uniqueName(sStr & buf, const char * path, ...);

            mutable sStr _list00;

        protected:
            struct Entry
            {
                idx path_index;
                idx abspath_index;
                idx targetpath_index;
                idx flags;

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

    class sFilePath: public sStr
    {
        public:
            static const idx sSizeMax = sSizePage;

            typedef idx (*funCallback)(char * buf, const char * wherepos, const char * tmplt);
            funCallback funcall;

            sFilePath(funCallback lfuncall = 0)
                : sStr()
            {
                funcall = lfuncall;
            }
            sFilePath(const char * filename, const char * fmt, ...) __attribute__((format(printf, 3, 4)))
                : sStr()
            {
                funcall = 0;
                sCallVargPara2(vmakeName, 0, filename, fmt);
            }
            char * makeName(const char * tmplt, const char * fmt, ...) __attribute__((format(printf, 3, 4)))
            {
                char * res;
                sCallVargResPara2(res, vmakeName, 0, tmplt, fmt);
                return res;
            }
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
            char * simplifyPath(eSimplifyPath spacehandle = eSimplifySpaceDoNotTouch, const char * srcBuf = 0);
            static char * nextToSlash(const char * path)
            {
                char * lastch = sString::searchSubstring(path, 0, "/" _ "\\" __, sNotIdx, 0, 1);
                if( !lastch )
                    return sString::nonconst(path);
                else
                    return lastch + 1;
            }
            char * pathReSlash(void)
            {
                char * path = ptr();
                cut(0);
                sString::searchAndReplaceSymbols(this, path, 0, "\\", "/", 0, true, true, false, false);
                return path;
            }
            char * composeWinStyleFileListPaths(const char * flnmlst, bool iszerolist);
            char * curDir(void);
            char * processPath(sFile::pid pid, bool justdir);
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

            idx search(idx gi);
            idx search(const char * acc, const char * separ);
            idx searchReverse(idx pos, idx gi);

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
                eExec_BG,
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
                    idx args;
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
