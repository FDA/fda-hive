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

#include <slib/std/storage.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>
#include <slib/utils/json/printer.hpp>

#include <ctype.h>

using namespace slib;

// insert src_len bytes from src into buf (total length buf_len, used length buf_used_len) at position buf_pos,
// overwriting del_len original bytes from buf
static bool bufInsert(char * buf, idx buf_len, idx buf_used_len, idx buf_pos, const char * src, idx src_len, idx del_len)
{
    if( buf_used_len + src_len - del_len > buf_len ) {
        return false;
    }
    memmove(buf + buf_pos + src_len, buf + buf_pos + del_len, buf_used_len - buf_pos - del_len);
    memcpy(buf + buf_pos, src, src_len);
    return true;
}

namespace {
    static struct StorageSchemes {
        sVec<sStorageScheme*> list;
        sDic<sStorageScheme*> name_map;

        ~StorageSchemes()
        {
            for(idx i=0; i<list.dim(); i++) {
                delete list[i];
            }
        }
    } storage_schemes;
}

// static
idx sStorageScheme::extractSchemeName(char * buf_out, const char * uri_or_name, idx uri_len)
{
    if( buf_out ) {
        *buf_out = 0;
    }
    if( !uri_or_name || !isalpha(uri_or_name[0]) ) {
        return 0;
    }

    idx scan_len = uri_len ? sMin<idx>(uri_len, sStorageScheme::max_name_len) : sStorageScheme::max_name_len;
    idx scheme_len;

    for(scheme_len=0; scheme_len<scan_len; scheme_len++) {
        char c = uri_or_name[scheme_len];
        if( isalnum(c) || c == '+' || c == '-' || c == '.' ) {
            if( buf_out ) {
                buf_out[scheme_len] = tolower(c);
            }
        } else {
            break;
        }
    }

    if( buf_out ) {
        buf_out[scheme_len] = 0;
    }

    if( (uri_len == 0 || scheme_len + 3 <= uri_len) && uri_or_name[scheme_len] ) {
        if( strncmp(uri_or_name + scheme_len, "://", 3) != 0 ) {
            // uri_or_scheme is long enough to be a uri, but doesn't have "://" after scheme name
            if( buf_out ) {
                buf_out[0] = 0;
            }
            scheme_len = 0;
        } else {
            scheme_len += 3;
        }
    }

    return scheme_len;
}

sStorageScheme::Root::Root()
{
    sSet(this, 0);
#ifndef WIN32
    pthread_mutex_init(&usability_mutex, 0);
#ifdef __APPLE__
    // OSX has no pthread_condattr_setclock(), no CLOCK_REALTIME_COARSE, no clock_gettime() :/
    // https://github.com/nanomsg/nanomsg/issues/10
    pthread_cond_init(&usability_cond, 0);
#else
    pthread_condattr_t usability_condattr;
    pthread_condattr_init(&usability_condattr);
    pthread_condattr_setclock(&usability_condattr, CLOCK_REALTIME_COARSE);
    pthread_cond_init(&usability_cond, &usability_condattr);
    pthread_condattr_destroy(&usability_condattr);
#endif
#endif
}

sStorageScheme::Root::~Root()
{
#ifndef WIN32
    pthread_cond_destroy(&usability_cond);
    pthread_mutex_destroy(&usability_mutex);
#endif
}

sStorageScheme::sStorageScheme(const char * names00)
{
    char buf[max_name_len + 1];
    for(const char * name = names00; name && *name; name = sString::next00(name)) {
        if( extractSchemeName(buf, name) ) {
            _names.setString(buf);
        }
    }
}

bool sStorageScheme::matches(const char * uri_or_name, idx len /* = 0 */) const
{
    char buf[max_name_len + 1];
    if( extractSchemeName(buf, uri_or_name, len) ) {
        return _names.get(buf);
    }
    return false;
}

inline void addSysSep(sStr & s)
{
    if( !s.length() || s[s.length() - 1] != sDir::sysSep ) {
        s.addString(&sDir::sysSep, 1);
    }
}

bool sStorageScheme::updateRootIsDead(idx iroot)
{
    Root & r = _roots[iroot];
    if( !r.is_dead ) {
        r.is_dead = !sDir::exists(_root_paths00.ptr(r.canary_path_offset));
    }
    return r.is_dead;
}

#ifndef WIN32
//static
void * sStorageScheme::updateRootUsabilityWorker(void * param)
{
    sStorageScheme::Root * r = static_cast<sStorageScheme::Root*>(param);
    pthread_mutex_lock(&r->usability_mutex);

    if( !r->total_size ) {
        r->total_size = sDir::fileSystemSize(r->scheme->getRoot(r->index));
    }
    r->cur_free = sDir::freeSpace(r->scheme->getRoot(r->index));
    pthread_cond_broadcast(&r->usability_cond);
    pthread_mutex_unlock(&r->usability_mutex);
    pthread_exit(0);
}
#endif

// Root::cur_free is valid for 10 minutes
#define CUR_FREE_VALID_SECS 600

udx sStorageScheme::updateRootUsability(idx iroot)
{
    Root * r = _roots.ptr(iroot);
    if( updateRootIsDead(iroot) || r->weight <= 0 ) {
        r->usability = 0;
        return 0;
    }

    idx cur_seconds = 0;

#ifdef WIN32
    cur_seconds = ::time();
#else
    struct timespec abs_time;
#ifdef __APPLE__
    // OSX has no pthread_condattr_setclock(), no CLOCK_REALTIME_COARSE, no clock_gettime() :/
    // https://github.com/nanomsg/nanomsg/issues/10
    struct timeval abs_tv;
    gettimeofday(&abs_tv, 0);
    TIMEVAL_TO_TIMESPEC(&abs_tv, &abs_time);
#else
    clock_gettime(CLOCK_REALTIME_COARSE, &abs_time);
#endif
    cur_seconds = abs_time.tv_sec;
#endif

    if( !r->total_size || cur_seconds > r->cur_free_checked + CUR_FREE_VALID_SECS ) {
        r->cur_free_checked = cur_seconds;
        udx local_cur_free = r->cur_free; // thread-local copy for consistent math
#ifdef WIN32
        if( !r->total_size ) {
            r->total_size = sDir::fileSystemSize(getRoot(iroot));
        }
        local_cur_free = r->cur_free = sDir::freeSpace(getRoot(iroot));
#else
        if( pthread_mutex_trylock(&r->usability_mutex) == 0 ) {
            // no other threads are updating this root's usability at the moment
            if( pthread_create(&r->usability_tid, 0, sStorageScheme::updateRootUsabilityWorker, (void*) r) == 0 ) {
                pthread_detach(r->usability_tid);
                abs_time.tv_sec += 1; // wait 1 sec
                if( pthread_cond_timedwait(&r->usability_cond, &r->usability_mutex, &abs_time) == 0 ) {
                    local_cur_free = r->cur_free;
                }
            }
            pthread_mutex_unlock(&r->usability_mutex);
        }
#endif

        if( r->total_size > 0 && local_cur_free > r->min_free ) {
            r->usability = sMax<udx>(1, local_cur_free * 100 * r->weight / r->total_size);
        } else {
            r->usability = 0;
        }
    }
    return r->usability;
}

bool sStorageScheme::addRoot(const char * root_path, real weight /* = 1 */, udx min_free /* = 0 */, sStr * err_out /* = 0 */)
{
    if( !sDir::exists(root_path) ) {
        if( err_out ) {
            err_out->printf("Storage root '%s' does not exist", root_path ? root_path : "(null)");
        }
        return false;
    }

    idx iroot = _roots.dim();
    Root * r = _roots.add(1);
    r->scheme = this;
    r->index = iroot;
    r->weight = weight;
    r->min_free = min_free;
    r->root_path_offset = _root_paths00.length();
    _root_paths00.addString(root_path);
    addSysSep(_root_paths00);
    _root_paths00.add0();
    r->canary_path_offset = _root_paths00.length();
    _root_paths00.addString(root_path);
    addSysSep(_root_paths00);
    _root_paths00.add("_storage");

    if( updateRootIsDead(iroot) ) {
        if( err_out ) {
            err_out->printf("Storage root '%s' looks dead (e.g. no '_storage' subdir)", root_path ? root_path : "(null)");
        }
        _root_paths00.cut(r->root_path_offset);
        _roots.cut(iroot);
        return false;
    }

    return true;
}

void sStorageScheme::updateRootsUsability()
{
    for(idx i=0; i<dimRoots(); i++) {
        updateRootUsability(i);
    }
}

bool sStorageScheme::rootsOK(sStr * err_out /* = 0 */)
{
    sStr buf;
    for(idx i=0; i<dimRoots(); i++) {
        buf.cut(0);
        const char * root = getRoot(i);
        if( updateRootIsDead(i) ) {
            if( err_out ) {
                err_out->printf("Storage root '%s' looks dead (e.g. no '_storage' subdir)", root);
            }
            return false;
        }
    }
    return dimRoots();
}

const char * sStorageScheme::getRandomRoot()
{
    udx total_usability = 0, cumulative_usability = 0;
    for(idx i=0; i<_roots.dim(); i++) {
        total_usability += updateRootUsability(i);
    }
    real pick = ((real)rand() / RAND_MAX) * total_usability;
    for(idx i=0; i<_roots.dim(); i++) {
        cumulative_usability += _roots[i].usability;
        if( _roots[i].usability && (i == _roots.dim() - 1 || pick < cumulative_usability) ) {
            return getRoot(i);
        }
    }
    return 0;
}

static void useSysSlashes(char * buf, idx buf_len)
{
    if( sDir::sysSep != '/' ) {
        char sys_slash00[3];
        sys_slash00[0] = sDir::sysSep;
        sys_slash00[1] = sys_slash00[2] = 0;

        sString::searchAndReplaceSymbols(buf, buf_len, "/" __, sys_slash00, 0, true, false, false, false);
    }
}

const char * sStorageScheme::printPathSuffix(sStr & out, const char * uri, idx len, bool allow_empty_path) const
{
    idx out_start = out.length();
    if( !len ) {
        len = sLen(uri);
    }
    idx scheme_name_len = extractSchemeName(0, uri, len);
    if( !scheme_name_len || scheme_name_len >= len ) {
        out.cut0cut(out_start);
        return 0;
    }

    uri += scheme_name_len;
    len -= scheme_name_len;

    sHiveId id;
    idx id_len = id.parse(uri);
    if( !id || !id_len || (id_len < len && uri[id_len] != '/') ) {
        out.cut0cut(out_start);
        return 0;
    }

    if( !allow_empty_path && id_len + 1 >= len ) {
        // uri is of the form "scheme://1234567/" or "scheme://1234567"
        out.cut0cut(out_start);
        return 0;
    }

    udx levelId = id.objId();
    for(udx ilevel = 0; ilevel < print_path_suffix_max_levels; ++ilevel) {
        udx curLevel = levelId % print_path_suffix_level_base;
        out.printf("%03" UDEC "%c", curLevel, sDir::sysSep);
        levelId /= print_path_suffix_level_base;
    }
    id.print(out);
    if( id_len < len ) {
        idx path_start = out.length();
        out.addString(uri + id_len);
        useSysSlashes(out.ptr(path_start), out.length() - path_start);
    } else {
        out.addString(&sDir::sysSep, 1);
    }
    return out.ptr(out_start);
}

const char * sStorageScheme::printPathSuffix(char * buf_out, idx buf_out_len, const char * uri, idx len, bool allow_empty_path) const
{
    if( buf_out_len <= 0 ) {
        return 0;
    }
    if( !len ) {
        len = sLen(uri);
    }
    idx scheme_name_len = extractSchemeName(0, uri, len);
    if( !scheme_name_len || scheme_name_len >= len ) {
        buf_out[0] = 0;
        return 0;
    }

    uri += scheme_name_len;
    len -= scheme_name_len;

    idx buf_pos = 0;

    sHiveId id;
    idx id_len = id.parse(uri);
    if( !id || !id_len || (id_len < len && uri[id_len] != '/') ) {
        buf_out[0] = 0;
        return 0;
    }

    if( !allow_empty_path && id_len + 1 >= len ) {
        // uri is of the form "scheme://1234567/" or "scheme://1234567"
        buf_out[0] = 0;
        return 0;
    }

    udx levelId = id.objId();
    for(udx ilevel = 0; ilevel < print_path_suffix_max_levels; ++ilevel) {
        udx curLevel = levelId % print_path_suffix_level_base;
        buf_pos += snprintf(buf_out + buf_pos, buf_out_len - buf_pos, "%03" UDEC "%c", curLevel, sDir::sysSep);
        if( buf_pos + 1 >= buf_out_len ) {
            // buffer is too short
            buf_out[0] = 0;
            return 0;
        }
        levelId /= print_path_suffix_level_base;
    }
    if( buf_pos + S_HIVE_ID_SHORT_BUFLEN + len - id_len > buf_out_len ) {
        // buffer is too short
        buf_out[0] = 0;
        return 0;
    }
    buf_pos += id.print(buf_out + buf_pos, false);
    if( id_len < len ) {
        idx path_len = len - id_len;
        memcpy(buf_out + buf_pos, uri + id_len, path_len);
        buf_out[buf_pos + path_len] = 0;
        useSysSlashes(buf_out + buf_pos, path_len);
    } else {
        buf_out[buf_pos] = sDir::sysSep;
        buf_out[buf_pos + 1] = 0;
    }
    return buf_out;
}

const char * sStorageScheme::printDump(sStr & out, const char * newline) const
{
    idx out_start = out.length();
    sJSONPrinter j(&out, 0, newline);
    j.startObject();
    j.addKey("names");
    j.startArray();
    for(idx i=0; i<dimNames(); i++) {
        j.addValue(getName(i));
    }
    j.endArray();
    j.addKey("roots");
    j.startArray();
    for(idx i=0; i<_roots.dim(); i++) {
        j.startObject();
        j.addKey("path");
        j.addValue(getRoot(i));
        j.addKey("weight");
        j.addValue(_roots[i].weight);
        j.addKey("min_free");
        j.addValue(_roots[i].min_free);
        j.addKey("cur_free");
        j.addValue(_roots[i].cur_free);
        j.addKey("total_size");
        j.addValue(_roots[i].total_size);
        j.addKey("usability");
        j.addValue(_roots[i].usability);
        j.endObject();
    }
    j.endArray();
    j.endObject();
    j.finish();
    return out.ptr(out_start);
}

//static
bool sStorage::addScheme(sStorageScheme * scheme)
{
    if( !scheme ) {
        return false;
    }

    for(idx i=0; i<scheme->dimNames(); i++) {
        if( storage_schemes.name_map.get(scheme->getName(i)) ) {
            return false;
        }
    }

    *storage_schemes.list.add(1) = scheme;
    for(idx i=0; i<scheme->dimNames(); i++) {
        *storage_schemes.name_map.setString(scheme->getName(i)) = scheme;
    }

    // allow sMex to use globally registered schemes
    sMex::uri_callback = sMexUriCallback;

    return true;
}

//static
sStorageScheme * sStorage::getScheme(const char * uri_or_scheme, idx len /* = 0 */)
{
    char name_buf[sStorageScheme::max_name_len + 1];
    if( sStorageScheme::extractSchemeName(name_buf, uri_or_scheme, len) ) {
        sStorageScheme ** pret = storage_schemes.name_map.get(name_buf);
        if( pret ) {
            return *pret;
        }
    }
    return 0;
}

//static
idx sStorage::getSchemes(sVec<sStorageScheme*> & schemes_out)
{
    idx start = schemes_out.dim();
    idx dim = storage_schemes.list.dim();
    schemes_out.add(dim);
    for(idx i=0; i<dim; i++) {
        schemes_out[start + i] = storage_schemes.list[i];
    }
    return dim;
}

static bool createParentDirs(char * s, struct stat * stat_out)
{
    char * last_slash = strrchr(s, sDir::sysSep);
    *last_slash = 0;
    bool ret = sDir::makeDir(s);
    *last_slash = sDir::sysSep;
    if( ret && !last_slash[1] ) {
        // s ended in '/', so we created s itself - so need to fill in stat_out
        ret = (::stat(s, stat_out) == 0);
    }
    return ret;
}

//static
const char * sStorage::makePath(sStr & out, struct stat * stat_out, const char * uri, idx len, sStorage::EMakePathMode mode /* = eCreateDirs */)
{
    struct stat stat_local;
    if( !stat_out ) {
        stat_out = &stat_local;
    }

    idx out_start = out.length();
    sStorageScheme * scheme = getScheme(uri, len);
    if( !scheme ) {
        out.cut0cut(out_start);
        return 0;
    }

    if( !scheme->printPathSuffix(out, uri, len, false) ) {
        out.cut0cut(out_start);
        return 0;
    }

    idx suffix_start = out_start;
    idx dim_roots = scheme->dimRoots();

    for(idx i=0; i<dim_roots; i++) {
        const char * root = scheme->getRoot(i);
        idx root_len = sLen(root);
        out.mex()->replace(out_start, root, root_len, suffix_start - out_start);
        out.add0cut();
        suffix_start = out_start + root_len;
        if( ::stat(out.ptr(out_start), stat_out) == 0 ) {
            return out.ptr(out_start);
        }
    }

    if( mode != eOnlyExisting ) {
        if( const char * root = scheme->getRandomRootFor(uri, len) ) {
            idx root_len = sLen(root);
            out.mex()->replace(out_start, root, root_len, suffix_start - out_start);
            out.add0cut();
            suffix_start = out_start + root_len;
            memset(stat_out, 0, sizeof(struct stat));
            if( mode != eCreateDirs || createParentDirs(out.ptr(out_start), stat_out) ) {
                return out.ptr(out_start);
            }
        }
    }

    out.cut0cut(out_start);
    return 0;
}

//static
const char * sStorage::makePath(char * buf_out, idx buf_out_len, struct stat * stat_out, const char * uri, idx len, sStorage::EMakePathMode mode/* = eCreateDirs */)
{
    if( buf_out_len <= 2 ) {
        return 0;
    }

    struct stat stat_local;
    if( !stat_out ) {
        stat_out = &stat_local;
    }

    sStorageScheme * scheme = getScheme(uri, len);
    if( !scheme ) {
        buf_out[0] = 0;
        return 0;
    }

    if( !scheme->printPathSuffix(buf_out, buf_out_len, uri, len, false) ) {
        buf_out[0] = 0;
        return 0;
    }

    idx suffix_start = 0;
    idx suffix_len = sLen(buf_out);
    idx dim_roots = scheme->dimRoots();

    for(idx i=0; i<dim_roots; i++) {
        const char * root = scheme->getRoot(i);
        idx root_len = sLen(root);
        if( !bufInsert(buf_out, buf_out_len, suffix_start + suffix_len + 1, 0, root, root_len, suffix_start) ) {
            // buffer is too short :(
            buf_out[0] = 0;
            return 0;
        }

        suffix_start = root_len;
        if( ::stat(buf_out, stat_out) == 0 ) {
            return buf_out;
        }
    }

    if( mode != eOnlyExisting ) {
        if( const char * root = scheme->getRandomRootFor(uri, len) ) {
            idx root_len = sLen(root);
            if( !bufInsert(buf_out, buf_out_len, suffix_start + suffix_len + 1, 0, root, root_len, suffix_start) ) {
                // buffer is too short :(
                buf_out[0] = 0;
                return 0;
            }
            memset(stat_out, 0, sizeof(struct stat));
            if( mode != eCreateDirs || createParentDirs(buf_out, stat_out) ) {
                return buf_out;
            }
        }
    }

    buf_out[0] = 0;
    return 0;
}

void sStorage::FindResult::push(const char * uri_dir, idx uri_dir_len, const char * rel_uri, const char * disk_path, bool is_dir)
{
    FindResult::Entry * e = _entries.add(1);
    e->abs_offset = _buf00.length();
    _buf00.addString(uri_dir, uri_dir_len);
    if( _buf00[_buf00.length() - 1] != '/' ) {
        _buf00.addString("/", 1);
    }
    e->rel_offset = _buf00.length();
    _buf00.addString(rel_uri);
    _buf00.add0();
    e->disk_offset = _buf00.length();
    _buf00.addString(disk_path);
    _buf00.add0();
    e->is_dir = is_dir;
}

//static
idx sStorage::findPaths(sStorage::FindResult & out, const char * uri_dir, idx uri_dir_len/* = 0 */, const char * fileglob/* = "*" */, idx recurse_depth/* = 0 */, bool no_dirs/* = false */)
{
    if( !uri_dir_len ) {
        uri_dir_len = sLen(uri_dir);
    }

    if( recurse_depth < 0 ) {
        recurse_depth = sIdxMax;
    }

    sStorageScheme * scheme = getScheme(uri_dir, uri_dir_len);
    if( !scheme ) {
        return 0;
    }

    sStr suffix_buf;
    if( !scheme->printPathSuffix(suffix_buf, uri_dir, uri_dir_len, true) ) {
        return 0;
    }

    idx dim_roots = scheme->dimRoots();
    sDir result_lister, subdirs_lister_separate;

    bool result_lister_lists_subdirs = recurse_depth && sIs(fileglob, "*") && !no_dirs;
    sDir & subdirs_lister = result_lister_lists_subdirs ? result_lister : subdirs_lister_separate;
    idx result_lister_flags = sFlag(sDir::bitFiles);
    if( !no_dirs ) {
        result_lister_flags |= sFlag(sDir::bitSubdirs) | sFlag(sDir::bitEntryFlags);
    }

    idx cnt_all = 0;
    FindResult dirs;
    sStr buf;

    for(idx i=0; i<dim_roots; i++) {
        const char * root = scheme->getRoot(i);
        buf.cut0cut();
        buf.addString(root);
        buf.addString(suffix_buf);
        if( sDir::exists(buf) ) {
            dirs.push(uri_dir, uri_dir_len, "", buf, true);
        }
    }

    const idx uri_dir_len_with_slash = dirs.dim() ? sLen(dirs.absUri(0)) : 0;

    for(idx dirs_depth = 0, dirs_start = 0, dirs_cnt = dirs.dim(); dirs_cnt && dirs_depth <= recurse_depth; dirs_depth++) {
        for(idx idir = dirs_start; idir < dirs_start + dirs_cnt; idir++) {
            // buf will be used for composing relative URIs
            const char * rel_uri_dir = dirs.absUri(idir) + uri_dir_len_with_slash;
            buf.cut0cut();
            if( rel_uri_dir[0] && strcmp("/", rel_uri_dir) != 0 ) {
                buf.cutAddString(0, dirs.absUri(idir) + uri_dir_len_with_slash);
                if( buf[buf.length() - 1] != '/' ) {
                    buf.addString("/", 1);
                }
            }
            idx rel_uri_dir_len = buf.length();

            result_lister.cut();
            result_lister.list(result_lister_flags, dirs.diskPath(idir), fileglob, 0, dirs.diskPath(idir));
            for(idx i=0; i<result_lister.dimEntries(); i++) {
                buf.cutAddString(rel_uri_dir_len, result_lister.getEntryPath(i));
                out.push(uri_dir, uri_dir_len, buf, result_lister.getEntryAbsPath(i), result_lister.getEntryFlags(i) & sDir::fIsDir);
                cnt_all++;
            }

            if( dirs_depth < recurse_depth ) {
                if( !result_lister_lists_subdirs ) {
                    subdirs_lister.cut();
                    subdirs_lister.list(sFlag(sDir::bitSubdirs) | sFlag(sDir::bitEntryFlags), dirs.diskPath(idir), "*", 0, dirs.diskPath(idir));
                }

                for(idx i=0; i<subdirs_lister.dimEntries(); i++) {
                    buf.cutAddString(rel_uri_dir_len, subdirs_lister.getEntryPath(i));
                    dirs.push(uri_dir, uri_dir_len, buf, subdirs_lister.getEntryAbsPath(i), subdirs_lister.getEntryFlags(i) & sDir::fIsDir);
                }
            }
        }
        dirs_start += dirs_cnt;
        dirs_cnt = dirs.dim() - dirs_start;
    }

    return cnt_all;
}

//static
const char * sStorage::printDump(sStr & out)
{
    idx out_start = out.length();
    out.addString("[");
    for(idx i=0; i<storage_schemes.list.dim(); i++) {
        out.addString("\n    ");
        storage_schemes.list[i]->printDump(out, "\n    ");
        if( i + 1 < storage_schemes.list.dim() ) {
            out.addString(",");
        } else {
            out.addString("\n");
        }
    }
    out.addString("]");
    return out.ptr(out_start);
}
