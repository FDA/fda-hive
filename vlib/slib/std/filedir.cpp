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
#include <ctype.h> // toupper
#ifdef WIN32
#include <direct.h> // _getdrive
//#include <io.h> // _findfirst
#else
//#include <unistd.h>
#include <dirent.h> // for DIR
#endif
#include <fcntl.h> // O_RDONLY
#include <glob.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <errno.h>

#include <slib/std/file.hpp>
#include <slib/std/string.hpp>

using namespace slib;

#ifdef WIN32
const char sDir::sysSep = '\\';
#else
const char sDir::sysSep = '/';
#endif

const char sDir::sep = '/';

// glob "?" means "any character, as long as it's not a '.' at the beginning of a filename"
#define GLOB_ANY_CHAR "(^[^.]|.)"

static void escapeReSpecialChar(sStr & out, char c)
{
    switch(c) {
        case '.':
        case '[':
        case ']':
        case '\\':
        case '(':
        case ')':
        case '*':
        case '+':
        case '?':
        case '{':
        case '}':
        case '|':
        case '^':
        case '$':
            out.addString("\\");
            /* no break */
        default:
            out.addString(&c, 1);
            /* no break */
    }
}

bool sFileGlob::compile(const char * glob /* = "*" */, bool caseSensitive /* = false */)
{
    if( compiled ) {
        regfree(&re);
        compiled = false;
    }

    if( !glob || !*glob || strcmp(glob, "*") == 0 ) {
        trivial = true;
        return true;
    }

    // translate a bash-like glob pattern into a POSIX regular expression
    trivial = false;
    sStr sre("^");
    idx curly = 0;
    for(char c = *glob; c; c = *(++glob)) {
        switch(c) {
            case '\\':
                c = *(++glob);
                if( !c )
                    return false;

                escapeReSpecialChar(sre, c);
                break;
            case '?':
                sre.addString(GLOB_ANY_CHAR);
                break;
            case '*':
                sre.addString(GLOB_ANY_CHAR "*");
                break;
            case '[': {
                sre.addString("[");
                idx len = 0;
                for(char cc = *(++glob); cc && cc != ']'; cc = *(++glob), len++) {
                    if( len == 0 ) {
                        if( cc == '!' ) {
                            sre.addString("^");
                            cc = *(++glob);
                        }
                        if( !cc )
                            return false;
                        if( cc == ']' ) {
                            sre.addString("]");
                            cc = *(++glob);
                        }
                        if( !cc )
                            return false;
                    }
                    sre.addString(&cc, 1);
                }

                if( *glob != ']' )
                    return false;

                sre.addString("]");
            }
                break;
            case '{':
                sre.addString("(");
                curly++;
                break;
            case ',':
                sre.addString(curly ? "|" : ",");
                break;
            case '}':
                if( curly ) {
                    sre.addString(")");
                    curly--;
                } else {
                    escapeReSpecialChar(sre, c);
                }
                break;
            default:
                escapeReSpecialChar(sre, c);
                break;
        }
    }

    sre.addString("$");

    if( curly )
        return false;

    int flags = REG_EXTENDED | REG_NOSUB;
    if( !caseSensitive )
        flags |= REG_ICASE;

    if( regcomp(&re, sre.ptr(), flags) != 0 )
        return false;

    compiled = true;
    return true;
}

bool sFileGlob::match(const char * path) const
{
    if( trivial ) {
        if( !path || !*path )
            return false;

        path = sFilePath::nextToSlash(path);
        // Hidden files are not matched by "*" !
        if( !path || !*path || *path == '.' )
            return false;

        return true;
    }
    return compiled && regexec(&re, path, 0, NULL, 0) == 0;
}

class sDirLinkFollower : public sFile::LinkFollower
{
protected:
    sDic<bool> & _seen;

    virtual bool valid()
    {
#ifdef WIN32
        return false;
#else
        sFile::InodeKey k(_st);
        bool * p = _seen.set(&k, sizeof(k));
        bool isNewKey = !*p;
        *p = true;
        return isNewKey;
#endif
    }

public:
    sDirLinkFollower(sStr & path, struct stat * st, sDic<bool> & seen): LinkFollower(path, st), _seen(seen) {}
};

struct StatStat
{
    struct stat st;
    enum EFlags {
        f_got_stat = 1,
        f_got_lstat = 1<<1
    };
    idx got_ops;
    int result;
    const char * path;
    const char * dir_entry_name;
    int dir_fd;

    inline void init(const char * path_, const char * dir_entry_name_, int dir_fd_)
    {
        sSet(&st);
        got_ops = 0;
        result = 0;
        path = path_;
        dir_entry_name = dir_entry_name_;
        dir_fd = dir_fd_;
    }

    inline bool getStat()
    {
        if( !(got_ops & f_got_stat) ) {
            got_ops = f_got_stat;
#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L || defined(_ATFILE_SOURCE)
            result = fstatat(dir_fd, dir_entry_name, &st, 0);
#else
            result = stat(path, &st);
#endif
        }
        return result == 0;
    }

    inline bool getLStat()
    {
        if( !(got_ops & f_got_lstat) ) {
            got_ops = f_got_lstat;
#if _POSIX_C_SOURCE >= 200809L || defined(_ATFILE_SOURCE)
            result = fstatat(dir_fd, dir_entry_name, &st, AT_SYMLINK_NOFOLLOW);
#else
            result = lstat(path, &st);
#endif
            if( result == 0 && !S_ISLNK(st.st_mode) ) {
                got_ops |= f_got_stat;
            }
        }
        return result == 0;
    }

    inline bool err() const
    {
        return got_ops && result;
    }

    StatStat() { init(0, 0, 0); }
};

#ifndef WIN32
static inline bool dir_islink_unix(const struct dirent * dt, StatStat & fst)
{
#ifdef _DIRENT_HAVE_D_TYPE
    if( dt->d_type != DT_UNKNOWN ) {
        // dirent d_type is not supported on some filesystems
        return dt->d_type == DT_LNK;
    }
#endif
    if( fst.getLStat() ) {
        return S_ISLNK(fst.st.st_mode);
    }
    return false;
}

static inline bool dir_isdir_unix(const struct dirent * dt, StatStat & fst)
{
#ifdef _DIRENT_HAVE_D_TYPE
    if( dt->d_type != DT_UNKNOWN ) {
        // dirent d_type is not supported on some filesystems
        if( dt->d_type == DT_DIR ) {
            return true;
        } else if( dt->d_type == DT_LNK && fst.getStat() ) {
            // this might be a symlink to a directory...
            return S_ISDIR(fst.st.st_mode);
        } else {
            return false;
        }
    }
#endif
    if( fst.getStat() ) {
        return S_ISDIR(fst.st.st_mode);
    }
    return false;
}
#endif

idx sDir::list(idx flags, const char * dirini, const char * wildcard, const char * separ, const char * relativePath)
{
    idx cnt = 0;
#define dir_symb "/"
    //sFilePath dCur();
    if( !dirini )
        dirini = ".";
    if( !wildcard )
        wildcard = "*";

    if( isFlag(flags, bitFollowLinks) && isFlag(flags, bitNamesOnly) )
    {
#ifdef _DEBUG
        fprintf(stderr, "%s:%u: DEVELOPER WARNING: bitFollowLinks makes no sense with bitNamesOnly\n", __FILE__, __LINE__);
#endif
        flags &= ~(sFlag(bitFollowLinks));
    }

    // If we are following links, go through non-link entries first to cache their inodes as seen
    bool onlyLinks = false;
    if( isFlag(flags, bitFollowLinks) && !isFlag(flags, bitNoLinks) )
    {
        cnt += list(flags | sFlag(bitNoLinks), dirini, wildcard, separ, relativePath);
        onlyLinks = true;
    }

    if( (isFlag(flags, bitSubdirs) != 0) != (isFlag(flags, bitFiles) != 0) ||
        isFlag(flags, bitRecursive) ||
        isFlag(flags, bitOpenable) ||
        isFlag(flags, bitSubdirSlash) ||
        isFlag(flags, bitFollowLinks) ||
        isFlag(flags, bitNoLinks) )
    {
        flags |= sFlag(bitEntryFlags); // need to record entry flags to distinguish files, dirs, and symlinks
    }

#ifdef WIN32
    if(isFlag(flags,bitDrives)) { // drive list
        //  Save current drive.
        idx drive, curdrive = _getdrive();
        // If we can switch to the drive, it exists.
        for( drive = 3; drive <= 26; drive++ ) {
            if( !_chdrive( (int)drive ) ) {
                if(cnt) sStr::separ(separ); //if(sStr::length())
                sStr::printf("%c:%c",drive + 'A' - 1,dir_symb[0]);
                ++cnt;
                //printf( "%c: ", drive + 'A' - 1 );
            }
        }

        // Restore original drive.
        _chdrive( (int)curdrive );
    }
#endif
    {
        idx fh;
        char quote = 0;
        idx len = sLen(dirini);
        sStr drl, tmpBuf;
        sStr entry_path, entry_abspath, entry_targetpath;
        const char * flnm;
        StatStat fst;
        char curd[16] = "./";
        struct stat followst;
        sDirLinkFollower linkFollower(tmpBuf, &followst, _seen);

        // quoted names are treated a little differently
        if( dirini[0] == '\"' || dirini[0] == '\'' ) {
            quote = dirini[0];
            ++dirini;
            --len;
            if( dirini[len - 1] == quote )
                --len;
        }
        drl.add(dirini, len);
        if( dirini[len - 1] != '/' && dirini[len - 1] != '\\' ) {
            drl.addString("/");
        }
        drl.addString(wildcard);
        flnm = sFilePath::nextToSlash(wildcard);
        //if( (flnm=strrchr(wildcard,'/'))==0 && (flnm=strrchr(wildcard,'\\'))==0 )flnm=wildcard;  // the pointer to the position of filename in the wildcard which may contain subdirectory path
        //else ++flnm;

#ifdef WIN32
#define dir_filename c_file.name
#define dir_isdir    (c_file.attrib&_A_SUBDIR)
#define dir_islink   0
        struct _finddata_t c_file;
        intptr_t hFile;
        idx res;

        if( (hFile=_findfirst(drl.ptr(), &c_file ))!=-1 ) {
            char * dir=drl.ptr(0), * ptr;if( (ptr=strrchr(dir,'/'))==0 && (ptr=strrchr(ptr,'\\'))==0 )dir=drl.printf(0,"."); else *ptr=0;        // the pointer to the position of filename in the wildcard which may contain subdirectory path
            len=sLen(dir);
            for(res=0;res==0;res=_findnext( hFile, &c_file )) {
#else
        sFileGlob glob;
        glob.compile(flnm);
#define dir_filename dt->d_name
#define dir_isdir    dir_isdir_unix(dt, fst)
#define dir_islink   dir_islink_unix(dt, fst)
        struct dirent * dt;
        DIR * mdr;

        char * dir = drl.ptr(0);
        char * ptr;
        if( (ptr = strrchr(dir, '/')) == 0 && (ptr = strrchr(dir, '\\')) == 0 )
            dir = curd;        //"./";
        else
            *ptr = 0;        // the pointer to the position of filename in the wildcard which may contain subdirectory path
        len = sLen(dir);
        if( (mdr = opendir(dir)) != 0 ) {
            int dir_fd = dirfd(mdr);
            // cache directory inode to make sure we don't reopen it via symlink loops
            if( isFlag(flags, bitFollowLinks) ) {
                linkFollower.follow(dir);
            }

            for(; (dt = readdir(mdr)) != 0;) {
                if( !glob.match(dir_filename) )
                    continue;
#endif
                idx entry_flags = 0;
                entry_path.cut(0);
                entry_abspath.cut(0);
                entry_targetpath.cut(0);

                entry_abspath.addString(drl.ptr(), len);
                entry_abspath.addString(&sDir::sep, 1);
                entry_abspath.addString(dir_filename);

                char * outpath = entry_abspath.ptr();

                fst.init(outpath, dir_filename, dir_fd);
                if( isFlag(flags, bitEntryFlags) ) {
                    if( dir_islink ) {
                        entry_flags |= fIsSymLink;
                    }
                    if( fst.err() )
                        continue;
                    if( isFlag(flags, bitNoLinks) && (entry_flags & fIsSymLink))
                        continue;
                    if( onlyLinks && !(entry_flags & fIsSymLink) )
                        continue;

                    if( isFlag(flags, bitFollowLinks) ) {
                        sSet(&followst);
                        if( linkFollower.follow(outpath) /* updates followst */ ) {
                            if( entry_flags & fIsSymLink )
                                entry_targetpath.addString(linkFollower.getPath().ptr());
                            //if( strcmp(outpath, linkFollower.getPath().ptr()) )
                                //printf("%s -> %s (%d)\n", outpath, targetpath.ptr(), dir_isdir)
                        } else {
                            entry_flags |= fIsDuplicate;
                        }
                    }
                    if( dir_isdir ) {
                        entry_flags |= fIsDir;
                    }
                    if( entry_flags & fIsSymLink && !fst.getStat() ) {
                        entry_flags |= fIsDangling;
                    }

                    if( !fst.err() && (entry_flags & fIsDir) ) {
                        if( !isFlag(flags, bitSubdirs) ) {
                            continue;
                        }
                        if( !isFlag(flags, bitSelf) && !strcmp(dir_filename, ".") ) {
                            continue;
                        }
                        if( !isFlag(flags, bitUpper) && !strcmp(dir_filename, "..") ) {
                            continue;
                        }
                    } else {
                        // file or symlink that does not point to a directory
                        if( !isFlag(flags, bitFiles) ) {
                            continue;
                        }
                        if( isFlag(flags, bitOpenable) ) {
                            if( (fh = open(outpath, O_RDONLY, S_IREAD)) > 0 )
                                close((int) fh); // try opening and close the file
                            if( fh < 1 )
                                continue;
                        }
                    }
                }

                // prepare the filename
                if( quote ) {
                    entry_path.addString(&quote, 1);
                }
                //sStr::add(dirini,len);
                //sStr::printf("%s%s",(dirini[len-1]!='/' && dirini[len-1]!='\\') ? dir_symb : "", (flags&sFile_DIRLIST_SINGLEPATH) ? wildcard : dir_filename );
                if( relativePath && *relativePath && strstr(outpath, relativePath) == outpath ) {
                    outpath += sLen(relativePath);
                    while( *outpath && (*outpath == '/' || *outpath == '\\') )
                        ++outpath;
                }
                if( isFlag(flags, bitNoExtension) ) {
                    char * ext = strrchr(outpath, '.');
                    if( ext )
                        *ext = 0;
                }
                entry_path.addString(isFlag(flags, bitNamesOnly) ? sFilePath::nextToSlash(outpath) : outpath);                //drl.ptr(0));
                if( isFlag(flags, bitSubdirSlash) && dir_isdir )
                    entry_path.addString(dir_symb, 1);
                if( quote )
                    entry_path.addString(&quote, 1);

                Entry * entry = _entries.add();
                entry->flags = entry_flags;
                bool entry_path_is_abspath = (entry_path == entry_abspath);
                if( entry_path.length() && !entry_path_is_abspath ) {
                    entry->path_index = _entryBuf.length();
                    _entryBuf.addString(entry_path.ptr());
                    _entryBuf.add0();
                }
                if( entry_abspath.length() ) {
                    entry->abspath_index = _entryBuf.length();
                    _entryBuf.addString(entry_abspath.ptr());
                    _entryBuf.add0();
                    if( entry_path_is_abspath ) {
                        entry->path_index = entry->abspath_index;
                    }
                }
                if( entry_targetpath.length() ) {
                    entry->targetpath_index = _entryBuf.length();
                    _entryBuf.addString(entry_targetpath.ptr());
                    _entryBuf.add0();
                }
                cnt++;
            }
#ifdef WIN32
            _findclose( hFile );
#else
            closedir(mdr);
#endif
        }
    }

    updateListSepar(separ);
    if( cnt )
        _listUpdated = false;

    return cnt;
}

idx sDir::find(idx flags, const char * dirini, const char * wildcard, const char * separ, idx maxFind, const char * relativePath)
{
    idx sflags = 0;
    if( isFlag(flags, bitRecursive) && !isFlag(flags, bitSubdirs) ) {
        sflags = sFlag(bitSubdirs);
        if( isFlag(flags, bitFollowLinks) ) {
            sflags |= sFlag(bitFollowLinks);
        }
        if( !isFlag(flags, bitFollowLinks) || isFlag(flags, bitNoLinks) ) {
            sflags |= sFlag(bitNoLinks);
        }
    }
    sDir slist;
    sStr dirs00;
    sDir * subdir_list = sflags ? &slist : this;

    dirs00.add(dirini);
    dirs00.add0(2);

    idx cntAll = 0;
    idx cntNewSubdirs = 0;

    do {
        cntNewSubdirs = 0;
        idx subdir_list_start = subdir_list->dimEntries();

        for( const char * dir = dirs00.ptr(); dir && *dir; dir = sString::next00(dir) ) {
            cntAll += list(flags, dir, wildcard, separ, relativePath);
            if( maxFind && cntAll >= maxFind )
                return cntAll;
            if( !isFlag(flags, bitRecursive) )
                return cntAll;
            // If we did not list subdirectories, slist needs to list them now
            if( sflags )
                slist.list(sflags, dir, "*", 0);
        }
        dirs00.cut(0);

        for( idx i=subdir_list_start; i<subdir_list->dimEntries(); i++ ) {
            idx entry_flags = subdir_list->getEntryFlags(i);
            if( !(entry_flags & fIsDir) || (entry_flags & fIsDuplicate) || ((entry_flags & fIsSymLink) && !isFlag(flags, bitFollowLinks)) )
                continue;

            cntNewSubdirs++;
            dirs00.add(dirini ? subdir_list->getEntryAbsPath(i) : subdir_list->getEntryPath(i));
        }
        dirs00.add0(2);
    } while( cntNewSubdirs );

    return cntAll;
}

const char * sDir::findMany(idx flags, const char * dirs, const char * flnm, const char * separ, idx maxfind)
{
    const char * srch, *sfl;

    sStr tmp, dfl, dst, path;
    idx cnt = 0, cntDone = 0;

    for(sfl = flnm; sfl && *sfl;) {

        // extract filename
        tmp.cut(0);
        dfl.cut(0);
        sfl = sString::extractSubstring(&tmp, sfl, 0, 0, " " _ "," _ "\t" __, true, true);
        sString::cleanEnds(&dfl, tmp, 0, sString_symbolsBlank, 1);
        if( !dfl.length() )
            continue;

        for(srch = dirs; srch && *srch;) {

            // extract dirname
            tmp.cut(0);
            dst.cut(0);
            srch = sString::extractSubstring(&tmp, srch, 0, 0, " " _ "," _ "\t" __, 1, 1);
            sString::cleanEnds(&dst, tmp, 0, sString_symbolsBlank, 1);
            if( !dst.length() )
                continue;

            path.cut(0);
            path.addString(dst.ptr());
            path.addString(dfl.ptr());
            sFilePath dir(path, "%%dir");
            sFilePath fln(path, "%%flnm");

            // search for the filename
            if( cnt != cntDone ) {
                cntDone = cnt;
            }
            //cnt+=sDir::find(flags, dst, dfl, separ, maxfind);
            cnt += sDir::find(flags, dir, fln, separ, maxfind);
        }
    }
    return ptr();
}

const char * sDir::includes(const char * incdir, const char * flnm, idx maxlen, const char * incstart, const char * incend, const char * incsymbs, bool skipRepeat, bool isRecursive)
{
    sFil fil(flnm, sFil::fReadonly);
    sStr tmp;

    if( !maxlen )
        maxlen = fil.length();
    else if( maxlen > fil.length() )
        maxlen = fil.length();
    char * content = fil.ptr(), *s = content, *e = s + maxlen, *p;
    idx slen = sLen(incstart);
    idx elen = sLen(incend);

    // find the include statements filename
    sDic<bool> seenPaths;
    for(; s < e; ++s) {

        if( strncmp(s, incstart, slen) ) // not found here
            continue;

        // found the start
        s += slen;
        p = s;

        while( s++ < e && strncmp(s, incend, elen) )
            ; // scan until the end is found
        if( s == e ) // scanned to the end and was not able to locate an include statement end
            break;

        tmp.cut(0);
        char * docopy = tmp.add(p, (idx) (s - p));
        tmp.add(_, 1);
        sString::searchAndReplaceSymbols(docopy, 0, incsymbs, "", 0, true, true, true);
        sString::cleanEnds(docopy, 0, " \t\r\n", 1);

        idx prevDimEntries = dimEntries();
        findMany(sFlag(bitSubdirs) | sFlag(bitFiles), incdir, docopy, "", 1);
        if( dimEntries() == prevDimEntries )
            continue;

        const char * foundPath = getEntryPath(dimEntries() - 1);

        if( skipRepeat ) {
            bool * seenPath = seenPaths.set(foundPath);
            if( *seenPath )
                continue;

            *seenPath = true;
        }

        if( isRecursive ) {
            includes(incdir, foundPath, maxlen, incstart, incend, incsymbs, skipRepeat, isRecursive);
        }
    }

    updateListSepar(" ");
    return ptr();
}

bool sDir::chDir(const char* dir)
{
#ifdef WIN32
    if(dir[1] == ':') {
        ::_chdrive(toupper(dir[0]) - 'A' + 1);
    }
#endif
    return chdir(dir) == 0;
}

bool sDir::removeDir(const char* dir, bool recursive /* = true */)
{
    sDir slt;
    const char * ptr;

    if( isSymLink(dir) ) {
        return sFile::remove(dir);
    }
    if( recursive ) {
        slt.find(sFlag(bitFiles), dir, "*");
        for(ptr = slt.ptr(); ptr && *ptr; ptr = sString::next00(ptr, 1)) {
            sFile::remove(ptr);
        }
        slt.cut();
        slt.find(sFlag(bitSubdirs) | sFlag(bitSubdirSlash), dir, "*");
        for(ptr = slt.ptr(); ptr && *ptr; ptr = sString::next00(ptr, 1)) {
            removeDir(ptr);
        }
    }
    return rmdir(dir) == 0;
}

bool sDir::makeDir(const char* dir, idx mode)
{
    int r = -1;
    if( dir ) {
        sStr path;
        sString::searchAndReplaceSymbols(&path, dir, 0, "/\\", 0, 0, true, true, false, false);
        if( path.length() ) {
            r = 0;
            for(char* p = path.ptr(); p && r == 0; p = sString::next00(p)) {
                if( p > path.ptr() ) {
                    *--p = '/';
                } else if( p == path.ptr() && !p[0] ) {
                    // skip root directory since it always exists
                    continue;
                }
#ifdef WIN32
                r = ::_mkdir(path.ptr()); mode = 0;
#else
                errno = 0;
                r = ::mkdir(path.ptr(), mode);
#endif
                // FIXME: under unclear conditions (NFS bug? cosmic ray?), errno == EEPERM instead of EEXIST
                // is returned for an existing directory after mkdir(). Try to work around it.
                if( r != 0 && (errno == EEXIST || sDir::exists(path.ptr())) ) {
                    r = 0;
                }
            }
        }
    }
    return r == 0;
}

// static
bool sDir::exists(const char * path)
{
    struct stat buf;
    int r;
    r = stat(path, &buf);
    return r == 0 && (buf.st_mode & S_IFDIR);
}

// static
udx sDir::freeSpace(const char * path)
{
    udx r = 0;
    if( path && path[0] ) {
#ifdef WIN32
        GetDiskFreeSpaceEx // TODO
#else
        struct statvfs buf;
        if( statvfs(path, &buf) == 0 ) {
            r = buf.f_bavail * buf.f_frsize;
        }
#endif
    }
    return r;
}

// static
udx sDir::fileSystemSize(const char * path)
{
    udx r = 0;
    if( path && path[0] ) {
#ifdef WIN32
        // TODO
#else
        struct statvfs buf;
        if( statvfs(path, &buf) == 0 ) {
            r = buf.f_blocks * buf.f_frsize;
        }
#endif
    }
    return r;
}

//static
bool sDir::copyDir(const char * src, const char * dst, bool follow_link /* = false */, idx * pnumcopied /* = 0 */)
{
#ifdef _DEBUG_SDIR_COPY
    fprintf(stderr, "sDir::copyDir(\"%s\", \"%s\", %d)\n", src, dst, follow_link);
#endif

    idx numcopied = 0;
    if( pnumcopied ) {
        *pnumcopied = 0;
    } else {
        pnumcopied = &numcopied;
    }

    if( !sDir::exists(src) )
        return false;

    if( sDir::exists(dst) ) {
        // copying a directory onto itself is pointless
        if( sFile::sameInode(src, dst) )
            return false;
    } else {
        if( isSymLink(dst) && !sFile::remove(dst) )
            return false;
        if( sFile::exists(dst) )
            return false;

        // trivial case: src is a link, and we can create a link at dst pointing to the same target
        if( !follow_link && isSymLink(src) ) {
            *pnumcopied = sFile::copy(src, dst, false, false) ? 1 : 0;
            return *pnumcopied;
        }

        if( makeDir(dst) ) {
            (*pnumcopied)++;
        } else {
            return false;
        }
    }

    idx src_len = sLen(src);
    while( src_len > 0 && (src[src_len-1] == sep || src[src_len-1] == sysSep) )
        src_len--;

    sDir srcDir;
    idx flags = sFlag(bitFiles) | sFlag(bitSubdirs) | sFlag(bitRecursive) | sFlag(bitEntryFlags);
    if( follow_link )
        flags |= sFlag(bitFollowLinks);
    srcDir.find(flags, src);

    sStr dst_entry_path;

    for( idx i=0; i<srcDir.dimEntries(); i++ ) {
        dst_entry_path.cut(0);
        dst_entry_path.addString(dst);
        dst_entry_path.addString(srcDir.getEntryPath(i) + src_len);
        idx entry_flags = srcDir.getEntryFlags(i);
        if( (entry_flags & fIsSymLink) && (!follow_link || (entry_flags & fIsDangling) || (entry_flags & fIsDuplicate)) ) {
#ifdef _DEBUG_SDIR_COPY
            fprintf(stderr, "\tlink => trying sFile::copy(\"%s\", \"%s\", 0, 0)\n", srcDir.getEntryPath(i), dst_entry_path.ptr());
#endif
            if( sFile::copy(srcDir.getEntryPath(i), dst_entry_path, false, false) ) {
                (*pnumcopied)++;
            } else {
                return false;
            }
        } else if( entry_flags & fIsDir ) {
#ifdef _DEBUG_SDIR_COPY
            fprintf(stderr, "\tdir => trying sDir::mkdir(\"%s\")\n", dst_entry_path.ptr());
#endif
            if( makeDir(dst_entry_path) ) {
                (*pnumcopied)++;
                struct stat st;
                if( stat(srcDir.getEntryPath(i), &st) == 0 ) {
                    sFile::setAttributes(dst_entry_path, &st);
                }
            } else {
                return false;
            }
        } else {
#ifdef _DEBUG_SDIR_COPY
            fprintf(stderr, "\tfile => trying sFile::copy(\"%s\", \"%s\", 0, %d)\n", srcDir.getEntryPath(i), dst_entry_path.ptr(), follow_link);
#endif
            if( sFile::copy(srcDir.getEntryPath(i), dst_entry_path, false, follow_link) ) {
                (*pnumcopied)++;
            } else {
                return false;
            }
        }
    }
    // fix up attributes at the end to preserve timestamps
    for(idx i=0; i<srcDir.dimEntries(); i++) {
        struct stat st;
        idx entry_flags = srcDir.getEntryFlags(i);
        if( (entry_flags & fIsDir) && stat(srcDir.getEntryPath(i), &st) == 0 ) {
            dst_entry_path.cut(0);
            dst_entry_path.addString(dst);
            dst_entry_path.addString(srcDir.getEntryPath(i) + src_len);
            // ignore failure to set attributes, since destination filesystem may not allow some
            sFile::setAttributes(dst_entry_path, &st);
        }
    }
    return true;
}

//static
sRC sDir::copyContents(const char * src, const char * wildcard, const char * dst, bool follow_link /* = false */, idx * pnumcopied /* = 0 */)
{
#ifdef _DEBUG_SDIR_COPY
    fprintf(stderr, "sDir::copyContents(\"%s\", \"%s\", \"%s\", %d)\n", src, wildcard, dst, follow_link);
#endif

    idx numcopied = 0;
    if( pnumcopied ) {
        *pnumcopied = 0;
    } else {
        pnumcopied = &numcopied;
    }

    if( !sDir::exists(src) ) {
        return sRC(sRC::eCopying, sRC::eDirectory, sRC::eSource, sRC::eInvalid);
    }

    if( !sDir::exists(dst) ) {
        if( makeDir(dst) ) {
            (*pnumcopied)++;
        } else {
            switch(errno) {
                case EACCES:
                    return sRC(sRC::eCreating, sRC::eDestination, sRC::ePath, sRC::eNotPermitted);
                case ENOMEM:
                    return sRC(sRC::eCreating, sRC::eDestination, sRC::eMemory, sRC::eInsufficient);
                case ENOSPC:
                    return sRC(sRC::eCreating, sRC::eDestination, sRC::eDiskSpace, sRC::eInsufficient);
                case EPERM:
                    return sRC(sRC::eCreating, sRC::eDestination, sRC::eOperation, sRC::eNotSupported);
                case EROFS:
                    return sRC(sRC::eCreating, sRC::eDestination, sRC::eFileSystem, sRC::eReadOnly);
                default:
                    return sRC(sRC::eCreating, sRC::eDestination, sRC::ePath, sRC::eInvalid);
            }
        }
    }

    idx src_len = sLen(src);
    while( src_len > 0 && (src[src_len-1] == sep || src[src_len-1] == sysSep) )
        src_len--;

    sDir srcDir;
    srcDir.list(sFlag(bitFiles) | sFlag(bitSubdirs) | sFlag(bitEntryFlags), src, wildcard);

    sStr dst_entry_path;

    for( idx i=0; i<srcDir.dimEntries(); i++ ) {
        dst_entry_path.cut(0);
        dst_entry_path.addString(dst);
        dst_entry_path.addString(srcDir.getEntryPath(i) + src_len);
        if( srcDir.getEntryFlags(i) & fIsDir ) {
#ifdef _DEBUG_SDIR_COPY
            fprintf(stderr, "\tdir => trying sDir::copyDir(\"%s\", \"%s\", %d)\n", srcDir.getEntryPath(i), dst_entry_path.ptr(), follow_link);
#endif
            idx subnumcopied = 0;
            bool subcopied = sDir::copyDir(srcDir.getEntryPath(i), dst_entry_path.ptr(), follow_link, &subnumcopied);
            *pnumcopied += subnumcopied;
            if( !subcopied ) {
                return sRC(sRC::eCopying, sRC::eDirectory, sRC::eOperation, sRC::eFailed);
            }
        } else {
#ifdef _DEBUG_SDIR_COPY
            fprintf(stderr, "\tfile => trying sFile::copy(\"%s\", \"%s\", 0, %d)\n", srcDir.getEntryPath(i), dst_entry_path.ptr(), follow_link);
#endif
            if( sFile::copy(srcDir.getEntryPath(i), dst_entry_path, false, follow_link) ) {
                (*pnumcopied)++;
            } else {
                return sRC(sRC::eCopying, sRC::eFile, sRC::eOperation, sRC::eFailed);
            }
        }
    }
    // fix up attributes at the end to preserve timestamps
    for(idx i=0; i<srcDir.dimEntries(); i++) {
        struct stat st;
        idx entry_flags = srcDir.getEntryFlags(i);
        if( (entry_flags & fIsDir) && stat(srcDir.getEntryPath(i), &st) == 0 ) {
            dst_entry_path.cut(0);
            dst_entry_path.addString(dst);
            dst_entry_path.addString(srcDir.getEntryPath(i) + src_len);
            // ignore failure to set attributes, since destination filesystem may not allow some
            sFile::setAttributes(dst_entry_path, &st);
        }
    }
    return sRC::zero;
}

// static
bool sDir::rename(const char * src, const char * dst)
{
    if( ::rename(src, dst) == 0 )
        return true;

    // check that we are not trying to move src into its own subdirectory
    if( errno == EINVAL )
        return false;

    if( copyDir(src, dst, false) ) {
        removeDir(src, true);
        return true;
    }
    return false;
}

// static
idx sDir::size(const char * dir, bool recursive /* = true */, bool follow_link /* = false */)
{
    sDir slt;

    idx sum = sFile::size(dir);
    idx flags = sFlag(bitFiles) | sFlag(bitSubdirs) | sFlag(bitEntryFlags);
    if( recursive ) {
        flags |= sFlag(bitRecursive);
    }
    if( follow_link ) {
        flags |= sFlag(bitFollowLinks);
    }
    slt.find(flags, dir, "*");
    for( idx i=0; i<slt.dimEntries(); i++ ) {
        // even if follow_link is true, find() may have returned some dangling links
        idx entry_flags = slt.getEntryFlags(i);
        sum += sFile::size(slt.getEntryPath(i), follow_link && !(entry_flags & fIsDangling) && !(entry_flags & fIsDuplicate));
#ifdef _DEBUG_SDIR_SIZE
        fprintf(stderr, "size %s == %" DEC " (%s%s%s%s)\n", slt.getEntryPath(i), sFile::size(slt.getEntryPath(i), follow_link && !(entry_flags & fIsDangling) && !(entry_flags & fIsDuplicate)), entry_flags & fIsDir ? "dir " : "", entry_flags & fIsSymLink ? "lnk " : "", entry_flags & fIsDangling ? "dgl " : "", entry_flags & fIsDuplicate ? "dup" : "");
#endif
    }
    return sum;
}

idx resolveGlob(sStr & out00, idx out00_start, const char * globsrc, bool ensureExist)
{
    glob_t glb;
    idx cnt = 0;
    if( glob(globsrc, ensureExist ? 0 : GLOB_NOMAGIC, 0, &glb) == 0 ) {
        out00.cut(out00_start);
        cnt = glb.gl_pathc;
        for(idx i=0; i<cnt; i++) {
            out00.add(glb.gl_pathv[i]);
        }
        globfree(&glb);
    } else {
        out00.cut0cut(out00_start);
    }
    return cnt;
}

const char * sDir::aliasResolve(sStr & filenameResolved, const char * configFile, const char * section, const char * filenamesrc, bool ensureExist, bool filenamesrcIsGlob)
{
    idx pos = filenameResolved.length();
    idx cntok = 0;
    if( filenamesrc ) {
        if( configFile && section ) {
            sFil inp(configFile, sMex::fReadonly);
            if( inp.ok() ) {
                sStr rst;
                sString::cleanMarkup(&rst, inp.ptr(), inp.length(), "//" _ "/*" __, "\n" _ "*/" __, "\n", 0, false, false, true);

                sStr redir("%s", section);
                redir.add0(1);
                const char * slash = strchr(filenamesrc, '/');
                if( !slash ) {
                    slash = strchr(filenamesrc, '\\');
                }
                if( slash ) {
                    sString::copyUntil(&redir, filenamesrc, slash - filenamesrc, "/\\");
                    ++slash;
                    redir.cut(-2);
                } else {
                    slash = filenamesrc;
                }
                redir.addString("/");
                redir.add0(2);

                sStr dirList;
                sString::SectVar var;
                var.loc = redir;
                var.fmtin = "%S";
                var.val = &dirList;

                sString::xscanSect(rst.ptr(), rst.length(), &var, 1);
                if( dirList ) {
                    sStr buf;
                    sString::searchAndReplaceSymbols(&buf, dirList.ptr(), 0, ";\n", 0, 0, true, true, true, true);
                    for(const char * p = buf.ptr(0); p; p = sString::next00(p)) {
                        idx blen = filenameResolved.length();
                        filenameResolved.addString(p);
                        filenameResolved.addString(slash);
                        if( filenamesrcIsGlob ) {
                            cntok += resolveGlob(filenameResolved, blen, filenameResolved.ptr(blen), ensureExist);
                        } else if( ensureExist && !sFile::exists(filenameResolved.ptr(blen)) ) {
                            filenameResolved.cut(blen);
                            continue;
                        } else {
                            // single file resolved
                            ++cntok;
                            filenameResolved.add0(1);
                        }
                    }
                }
            }
        }
        if( filenamesrcIsGlob ) {
            cntok += resolveGlob(filenameResolved, filenameResolved.length(), filenamesrc, ensureExist);
        } else if( !ensureExist || sFile::exists(filenamesrc) ) {
            filenameResolved.addString(filenamesrc);
            ++cntok;
        }
        if(cntok) {
            filenameResolved.add0(2);
        }
    }
    return cntok ? filenameResolved.ptr(pos) : 0;
}

void sDir::updateListSepar(const char * separ) const
{
    if( separ ) {
        if( !_listSepar || strcmp(separ, _listSepar.ptr()) != 0 ) {
            _listSepar.cutAddString(0, separ);
            _listUpdated = false;
        }
    } else {
        if( _listSepar ) {
            _listSepar.cut(0);
            _listUpdated = false;
        }
    }
}

void sDir::updateList() const
{
    if( _listUpdated )
        return;

    _list00.cut(0);

    for( idx i=0; i<dimEntries(); i++ ) {
        _list00.addString(getEntryPath(i));
        _list00.addSeparator(_listSepar);
    }

    _list00.add0(2);
    _listUpdated = true;
}

// static
const char * sDir::cleanUpName(const char * path, sStr & buf, bool applyToDisk /* = false */)
{
    if( !path || !path[0] || (applyToDisk && !exists(path) && !sFile::exists(path)) ) {
        return 0;
    }
    const idx pos = buf.length();
    const char * c = sFilePath::nextToSlash(path);
    if( c != path ) {
        buf.add(path, c - path);
    }
    idx dot = -1; // 1st dot in name - point of counter injection when name collide on disk
    for(; *c; ++c) {
        if( *c == '-' || *c == '.' || *c == '_' || (*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') ) {
            if( dot < 0 && *c == '.' ) {
                dot = buf.length();
            }
            buf.add(c, 1);
        } else {
            buf.add("_", 1);
        }
    }
    buf.add0();
    if( applyToDisk && strcmp(path, buf.ptr(pos)) ) {
        bool isFile = !exists(path);
        //check for dup names!!!
        udx q = 0;
        dot = dot < 0 ? buf.length() - 1 : dot;
        sStr ext("%s", buf.ptr(dot));
        while( exists(buf.ptr(pos)) || sFile::exists(buf.ptr(pos)) ) {
            buf.printf(dot, "_%" UDEC "%s", ++q, ext.ptr());
        }
        if( isFile && sFile::rename(path, buf.ptr(pos)) ) {
        } else if( rename(path, buf.ptr(pos)) ) {
        } else {
            // failed to rename - no result!
            *buf.ptr(pos) = '\0';
            buf.cut(pos);
        }
    }
    return buf.length() != pos ? buf.ptr(pos) : 0;
}

// static
const char * sDir::uniqueName(sStr & buf, const char * path, ...)
{
    const idx pos = buf.length();
    if( path && path[0] ) {
        sCallVarg(buf.vprintf, path);
        const char * p = sFilePath::nextToSlash(buf.ptr(pos));
        p = strchr(p ? p : buf.ptr(pos), '.');
        const idx dot = p ? p - buf.ptr() : buf.length() - 1;
        sStr ext("%s", buf.ptr(dot));
        udx q = 0;
        while( exists(buf.ptr(pos)) || sFile::exists(buf.ptr(pos)) ) {
            buf.printf(dot, "_%" UDEC "%s", ++q, ext.ptr());
        }
    }
    return buf.length() != pos ? buf.ptr(pos) : 0;
}

static bool tempDirTester(const char * path)
{
    struct stat st;

    if( stat(path, &st) == 0 )
        return false;

    if( !sDir::makeDir(path) )
        return false;

    return true;
}

// static
const char * sDir::mktemp(sStr & outPath, const char * dir, const char * pattern/*=0*/)
{
    return sFile::mktemp(outPath, dir, 0, pattern, tempDirTester);
}

static void escapeForGlob(sStr & out, const char * s, idx len)
{
    for( idx i=0; i < len; i++ ) {
        switch(s[i]) {
        case '\\':
        case '?':
        case '*':
        case '[':
        case '{':
            out.add("\\", 1);
        default:
            out.add(s + i, 1);
        }
    }
    out.add0cut();
}

// static
idx sDir::cleantemp(const char * dir, const char * pattern/*=0*/)
{
    if( !pattern )
        pattern = DEFAULT_MKTEMP_PATTERN;

    const char * xxx = strstr(pattern, "XXXXXX");
    if( !xxx ) {
#ifdef _DEBUG
        fprintf(stderr, "%s:%u: DEVELOPER WARNING: sDir::cleantemp requires pattern with \"XXXXXX\" substring\n", __FILE__, __LINE__);
#endif
        return 0;
    }

    // create a shell glob that matches pattern followed by any extension
    sStr wildcard;
    escapeForGlob(wildcard, pattern, xxx - pattern);
    wildcard.addString("*");
    escapeForGlob(wildcard, xxx + 6, strlen(xxx + 6));
    wildcard.addString("{,.*}");

    sDir temps;
    temps.list(sFlag(bitSubdirs)|sFlag(bitFiles)|sFlag(bitEntryFlags), dir, wildcard.ptr());

    idx count = 0;

    for( idx i=0; i<temps.dimEntries(); i++ ) {
        if( temps.getEntryFlags(i) & fIsDir ) {
            count += removeDir(temps.getEntryPath(i));
        } else {
            count += sFile::remove(temps.getEntryPath(i));
        }
    }

    return count;
}

idx sDir::totalMemUse() const
{
    return sizeof(*this) + _entries.mex()->total() + _seen.mex()->total() + _entryBuf.mex()->total() + _list00.mex()->total() + _listSepar.mex()->total();
}
