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
#include <ulib/uobj.hpp>
#include <slib/core/str.hpp>
#include <slib/std/file.hpp>
#include <slib/utils/sort.hpp>
#include <qlib/QPrideBase.hpp>
#include "uperm.hpp"

using namespace slib;
#include <memory>
#include <ctype.h>
#include <regex.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

typedef struct {
    sStr path;
    real weight;
    udx min_free;
    udx from, to;
    udx free;
} sRoots;

static sVec< sRoots > s_roots;

pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static
void * diskFree_thread(void * locR )
{
    sRoots * r = (sRoots *) locR;
    if( r->path ) {
        r->free = sDir::freeSpace(r->path);
        pthread_mutex_unlock(&g_lock);
    }
    pthread_exit(0);
}

static udx diskFree(sRoots * r)
{
    udx fsz = 0;

#ifdef WIN32
    fsz = sDir::freeSpace(path);
#else
    pthread_t tid;
    pthread_mutex_lock(&g_lock);
    if( pthread_create(&tid, 0, diskFree_thread, (void*) r) == 0 ) {
        pthread_detach(tid);
        struct timespec abs_time;
#ifdef __APPLE__
#warning "IMPLEMENT THIS FOR MAC"
#else
        clock_gettime(CLOCK_REALTIME, &abs_time);
        abs_time.tv_sec += 1; // wait 1 sec
        if( pthread_mutex_timedlock(&g_lock, &abs_time) == 0 ) {
            fsz = r->free;
        }
#endif
    }
    pthread_mutex_unlock(&g_lock);

#endif
    return fsz;
}

//static
char * sUsrObj::getPath(sStr & path, const sHiveId & id, bool create/* = false*/)
{
    if( id && !path ) {
        sStr suffix;
        // TODO: what about ion_id?
        udx levelId = id.objId();
        const udx maxLevels = 2, levelBase = 1000;
        for(udx ilevel = 0; ilevel < maxLevels; ++ilevel) {
            udx curLevel = levelId % levelBase;
            suffix.printf("%03"UDEC"/", curLevel);
            levelId /= levelBase;
        }
        // TODO support domain!
        suffix.printf("%"UDEC, id.objId());
        for(idx i = 0; i < s_roots.dim(); ++i) {
            const char * realStoragePath = sString::next00(s_roots.ptr(i)->path, -1); // random
            path.printf("%s%s/", realStoragePath, suffix.ptr());
            if( sDir::exists(path) ) {
                break;
            }
            path.cut(0);
        }
        if( !path && create && s_roots.dim() ) {
            udx total = 0;
            for(idx i = 0; i < s_roots.dim(); ++i) {
                sRoots * r = s_roots.ptr(i);
                r->from = total + 1;
                r->free = 0;
                udx df = diskFree(r);
                if( df > r->min_free ) {
                    total += df * r->weight;
                }
                r->to = total;
            }
            total = (rand() * 1.) / RAND_MAX * total + 1;
            idx found = 0;
            for(; found < s_roots.dim() - 1; ++found) {
                if( s_roots.ptr(found)->from <= total && total <= s_roots.ptr(found)->to ) {
                    break;
                }
            }
            const char * realStoragePath = sString::next00(s_roots.ptr(found)->path, -1); // random
            path.printf("%s%s/", realStoragePath, suffix.ptr());
            if( !sDir::makeDir(path) ) {
#if _DEBUG
                ::printf("mkdir failed: '%s' %d - %s\n", path.ptr(), errno, strerror(errno));
#endif
                path.cut(0);
            }
        }
    }
    return path ? path.ptr() : 0;
}

// static
void sUsrObj::initStorage(const char * pathList, const udx default_min_free_space_per_volume)
{
    sStr path00, mpath;
    sString::searchAndReplaceSymbols(&path00, pathList, 0, ";,", 0, 0, true, true, false, true);
    path00.add0();
    s_roots.empty();
    for(char * p = path00.ptr(); p; p = sString::next00(p)) {
        sRoots * r = s_roots.add(1);
        if( r ) {
            mpath.cut0cut(0);
            sString::searchAndReplaceSymbols(&mpath, p, 0, ":~", 0, 0, true, false, false, false);
            mpath.add0(2);
            r->min_free = default_min_free_space_per_volume;
            r->weight = 1;
            char * x = sString::next00(mpath.ptr());
            if( x ) {
                sscanf(x, "%lf", &r->weight);
                *x = 0; // terminate path list
                x = sString::next00(x);
                if( x ) {
                    sscanf(x, "%"UDEC, &r->min_free);
                }
            }
            sString::searchAndReplaceSymbols(mpath.ptr(), 0, "|", 0, 0, true, false, false, false);
            for(const char * mp = mpath; mp; mp = sString::next00(mp)) {
                if( sDir::exists(mp) ) {
                    r->path.printf("%s", mp);
                    r->path.add0();
                }
            }
            r->path.add0(2);
        }
    }
}

sUsrObj::~sUsrObj()
{
}

udx sUsrObj::propSet(const char* prop, const char** paths, const char** values, udx cntValues, bool isAppend /* = false */, const udx * path_lens /* = 0 */, const udx * value_lens /* = 0 */)
{
    udx qty = 0;
    const sUsrTypeField * tf = propGetTypeField(prop);
    static bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
    bool err = !tf;
    if( m_id.domainId() && !use_type_upobj ) {
        fprintf(stderr, "Error: propSet() on objects with non-zero domain ID (obj = %s) works only in TYPE_UPOBJ mode\n", m_id.print());
        err = true;
    }
    if( m_usr.isAllowed(Id(), ePermCanWrite) && !err ) {
        if( sIsExactly(prop, "created") || sIsExactly(prop, "modified") ) {
            // special fields which must only be set automatically by stored procedures on object creation or modification
            qty = 1;
        } else {
            const bool single = !tf->isMulti();
            err = sLen(prop) <= 0 || sLen(prop) >= 256;
            const char * const s_multi_value_separator = "^=|=$";
            // see sp_obj_prop_set stored procedure parameters for maximum length here
            // its less than sp can accept to accommodate escaping
            const udx max_value_len = 23 * 1024 * 1024;
            udx skip_transaction = 0;
            for(udx i = 0; !err && i < cntValues; ++i ) {
                udx path_len = path_lens ? path_lens[i] : (paths ? sLen(paths[i]) : 0);
                udx value_len = value_lens ? value_lens[i] : (values ? sLen(values[i]) : 0);
                skip_transaction += path_len + value_len + sizeof(s_multi_value_separator) + 5;
                if( skip_transaction > max_value_len ) {
                    skip_transaction = 0;
                    break;
                }
            }
            for(idx itry = 0; !err && itry < sSql::max_deadlock_retries; itry++) {
                bool is_our_transaction = !skip_transaction && !m_usr.getUpdateLevel();
                if( !err && (skip_transaction || m_usr.updateStart()) ) {
                    sStr vsql, value_buf;
                    sMex blob_buf;
                    udx chunk = 0;
                    while( !err && chunk < cntValues ) {
                        udx i = chunk;
                        vsql.cut(0);
                        do {
                            const char * value = values[i];
                            udx value_len = value_lens ? value_lens[i] : sLen(value);
                            blob_buf.cut(0);
                            value_buf.cut(0);
                            if( tf->defaultEncoding() ) {
                                if( !m_usr.encodeField(&value_buf, &blob_buf, tf->defaultEncoding(), value, value_len) ) {
                                    err = true;
                                    break;
                                }
                                if( value_buf.length() ) {
                                    value = value_buf.ptr();
                                    value_len = value_buf.length();
                                } else {
                                    value = NULL;
                                    value_len = 0;
                                }
                            }

                            static const udx MAX_VALUE_LENGTH = 16 * 1024 *1024; // see UPObjField table: MEDIUMTEXT -> 2^24
                            if( value_len >= MAX_VALUE_LENGTH ) {
                                err = true;
                                break;
                            }
                            if( blob_buf.pos() >= (idx)MAX_VALUE_LENGTH ) {
                                err = true;
                                break;
                            }

                            if( i > chunk ) {
                                vsql.addString(",");
                            }
                            // (domainID, objID, name, group, value, encoding, blob_value)
                            vsql.addString("(");

                            // domainID
                            if( m_id.domainId() ) {
                                vsql.printf("%"UDEC",", m_id.domainId());
                            } else {
                                // UPObjField: NULL domainID means 0
                                vsql.addString("NULL,");
                            }

                            // objID
                            vsql.printf("%"UDEC",", m_id.objId());

                            // name
                            m_usr.db().protectValue(vsql, prop);
                            vsql.addString(",");

                            // path
                            if( paths && paths[i] && (path_lens ? path_lens[i] : paths[i][0]) ) {
                                udx path_len = path_lens ? path_lens[i] : sLen(paths[i]);
                                if( path_len > 255 ) {
                                    err = true;
                                    break;
                                }
                                m_usr.db().protectValue(vsql, paths[i], path_lens ? path_lens[i] : 0);
                                vsql.addString(",");
                            } else if( !single ) {
                                vsql.printf("'1.%"UDEC"',", chunk + i + 1);
                            } else {
                                vsql.addString("NULL,");
                            }

                            // value
                            if( value ) {
                                m_usr.db().protectValue(vsql, value, value_len);
                                vsql.addString(",");
                            } else {
                                vsql.addString("'',"); // value column cannot be NULL
                            }

                            // encoding
                            if( tf->defaultEncoding() ) {
                                vsql.printf("%"DEC",", tf->defaultEncoding());
                            } else {
                                vsql.addString("NULL,");
                            }

                            // blob
                            if( blob_buf.pos() ) {
                                m_usr.db().protectBlob(vsql, blob_buf.ptr(), blob_buf.pos());
                            } else {
                                vsql.addString("NULL");
                            }

                            vsql.addString(")");
                        } while( ++i < cntValues );
                        if( !err ) {
                            // TODO remove temp hack for types
                            std::auto_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_prop_set_v3"));
                            p->Add(m_id.domainId()).Add(m_id.objId()).Add((udx) ePermCanWrite).Add(vsql).Add(!isAppend && chunk == 0 ? prop : 0);
                            udx q = p->uvalue(0);
                            if( q == 0 ) {
                                err = true;
                                if( m_usr.db().HasFailed() && !(is_our_transaction && m_usr.hadDeadlocked()) ) {
                                    // Serious DB failure that should not happen - need to log with details
                                    // fprintf() needed because a DB connection failure or deadlock might cause QPride()->logOut() to fail too
                                    fprintf(stderr, "propSet() DB error %"UDEC" on objID = %s, prop = '%s' : %s\n", m_usr.db().Get_errno(), m_id.print(), prop, m_usr.db().Get_error().ptr());
                                    if( m_usr.QPride() ) {
                                        m_usr.QPride()->logOut(sQPrideBase::eQPLogType_Error, "propSet() DB error %"UDEC" on objID = %s, prop = '%s' : %s", m_usr.db().Get_errno(), m_id.print(), prop, m_usr.db().Get_error().ptr());
                                    }
                                }
                            } else {
                                chunk = i;
                                qty += q;
                            }
                        }
                    }
                    if( !err && qty ) {
                        if( skip_transaction || m_usr.updateComplete() ) {
                            if( m_propsTree.get() ) {
                                m_propsTree->is_default = false;
                            }
                        } else {
                            qty = 0;
                        }
                    }
                    if( (err || !qty) && !skip_transaction ) {
                        if( m_usr.hadDeadlocked() && is_our_transaction ) {
                            // DB deadlock detected, and our own m_usr.updateStart() call had
                            // started the current DB transaction. Wait a bit and retry.
                            m_usr.updateAbandon();
                            err = false;
                            qty = 0;
    #if 0
                            fprintf(stderr, "%s:%u - restarting deadlocked transaction, attempt %"DEC"/%"DEC"\n", __FILE__, __LINE__, itry + 1, sSql::max_deadlock_retries);
    #endif
                            sTime::randomSleep(sSql::max_deadlock_wait_usec);
                            continue;
                        } else {
                            m_usr.updateAbandon();
                        }
                    }
                }
                break;
            }
        }
    }
    if( qty && !err && is_auditable ) {
        m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; prop='%s'; updated='%"UDEC"'", Id().print(), prop, qty);
    }
    return qty;
}

udx sUsrObj::propDel(const char * prop, const char * group, const char * value)
{
    if( !prop || !prop[0] || !m_usr.isAllowed(Id(), ePermCanWrite) ) {
        return 0;
    }
    static bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
    std::auto_ptr<sSql::sqlProc> p(m_usr.getProc(use_type_upobj ? "sp_obj_prop_del_v2" : "sp_obj_prop_del"));
    if( use_type_upobj ) {
        p->Add(m_id.domainId());
    }
    p->Add(m_id.objId()).Add((udx)ePermCanWrite).Add(prop).Add(group).Add(value);
    udx qty = p->uvalue(0);
    if( qty ) {
        m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; prop='%s'; path='%s'; value='%s'; updated='%"UDEC"'", Id().print(), prop, group ? group : "", value ? value : "", qty);
    }
    return qty;
}

//static
bool sUsrObj::propInitInternal(const sUsr& usr, sUsrObj * uobj, const sHiveId & id, bool keep_autofill)
{
    if( usr.isAllowed(id, ePermCanWrite) ) {
        sStr autofill_fld_csv;
        if( keep_autofill ) {
            bool own_uobj = false;
            if( !uobj ) {
                uobj = usr.objFactory(id);
                own_uobj = true;
            }
            const sUsrType2 * utype = uobj ? uobj->getType() : 0;
            for(idx ifld = 0; utype && ifld < utype->dimFields(usr); ifld++) {
                const sUsrTypeField * fld = utype->getField(usr, ifld);
                // "created" and "modified" are handled specially by the stored procedure
                if( fld->readonly() == sUsrTypeField::eReadOnlyAutofill && !sIsExactly(fld->name(), "created") && !sIsExactly(fld->name(), "modified") ) {
                    if( autofill_fld_csv.length() ) {
                        autofill_fld_csv.addString(",");
                    }
                    usr.db().protectValue(autofill_fld_csv, fld->name());
                }
            }
            if( own_uobj ) {
                delete uobj;
                uobj = 0;
            }
        }
        std::auto_ptr<sSql::sqlProc> p(usr.getProc("sp_obj_prop_init_v2"));
        p->Add(id.domainId()).Add(id.objId()).Add((udx)ePermCanWrite).Add(autofill_fld_csv.ptr());
        sVarSet tbl;
        p->getTable(&tbl);
        // result: ( # of objects, # of deleted/changed properties )
        if( tbl.rows && tbl.cols && tbl.ival(0, 0) > 0 ) {
            return true;
        }
    }
    return false;
}

bool sUsrObj::propInit(bool keep_autofill/* = false */)
{
    return propInitInternal(m_usr, this, m_id, keep_autofill);
}

//static
bool sUsrObj::propInit(const sUsr& usr, const sHiveId & id, bool keep_autofill/* = false */)
{
    return propInitInternal(usr, 0, id, keep_autofill);
}

//static
idx sUsrObj::readPathElt(const char * path, const char ** next)
{
    if( !path ) {
        *next = 0;
        return -sIdxMax;
    }

    idx elt = strtoidx(path, (char**)next, 10);
    if( **next == '.' ) {
        // integer followed by expedcted path separator
        (*next)++;
    } else if( **next ) {
        // unexpected path separator
        *next = 0;
        elt = -sIdxMax;
    } else if( path == *next ) {
        // no path separator, this is the final path element
        *next = 0;
    }
    return elt;
}

static idx sortPropsCallback(void * param, void * arr_param, idx i1, idx i2)
{
    sVarSet * res = static_cast<sVarSet*>(param);
    idx * arr = static_cast<idx*>(arr_param);

    // first compare by ID - if available (4-column format)
    if( res->cols == 4 ) {
        sHiveId id1(res->val(arr[i1], 0));
        sHiveId id2(res->val(arr[i2], 0));
        if( idx diff = id1.cmp(id2) ) {
            return diff;
        }
    }

    idx path_col = res->cols == 4 ? 2 : 1;

    // then by path
    const char * path1 = res->val(arr[i1], path_col);
    const char * path2 = res->val(arr[i2], path_col);
    do {
        idx cur_elt1 = sUsrObj::readPathElt(path1, &path1);
        idx cur_elt2 = sUsrObj::readPathElt(path2, &path2);
        if( cur_elt1 != cur_elt2 ) {
            return cur_elt1 - cur_elt2;
        }
    } while( path1 && path2 );

    // finally by name - if available (4-column format)
    if( res->cols == 4 ) {
        const char * name1 = res->val(arr[i1], 1);
        const char * name2 = res->val(arr[i2], 1);
        return strcasecmp(name1 ? name1 : "", name2 ? name2 : "");
    }
    return 0;
}

//static
idx sUsrObj::sortProps(sVarSet & res, idx start_row /* = 0 */, idx cnt /* = sIdxMax */)
{
    if( start_row < 0 || start_row > res.rows ) {
        return 0;
    }

    cnt = sMin<idx>(cnt, res.rows - start_row);
    if( cnt <= 0 || (res.cols != 4 && res.cols != 2) ) {
        return 0;
    }

    if( cnt > 1 ) {
        sVec<idx> rows_sorted(sMex::fExactSize);
        rows_sorted.resize(cnt);

        for(idx i=0; i<cnt; i++) {
            rows_sorted[i] = start_row + i;
        }
        sSort::sortSimpleCallback<idx>(sortPropsCallback, &res, rows_sorted.dim(), rows_sorted.ptr());
        idx ret = res.reorderRows(rows_sorted.ptr(), start_row, cnt);
        return ret;
    } else {
        return cnt;
    }
}

udx sUsrObj::propGet(const char* prop, sVarSet& res, bool sort) const
{
    if( prop && prop[0] == '_' ) {
        if( strcasecmp(&prop[1], "type") == 0 ) {
            // "_type" and "_id" are the only properties that can be read for write-only objects
            res.addRow().addCol(getTypeName()).addCol((const char*)0);
        } else if( m_usr.isAllowed(Id(), ePermCanRead) ) {
            if( strcasecmp(&prop[1], "parent") == 0 ) {
                sUsrObjRes p;
                m_usr.objs2("directory+", p, 0, "child", IdStr());
                idx cnt = 0;
                for(sUsrObjRes::IdIter it = p.first(); p.has(it); p.next(it)) {
                    res.addRow().addCol(p.id(it)->print()).addCol(cnt++);
                }
            } else if( strcasecmp(&prop[1], "dir") == 0 ) {
                if( getPath(m_path, m_id) ) {
                    res.addRow().addCol(m_path).addCol((const char*)0);
                }
            }
        }
    } else if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
        std::auto_ptr<sSql::sqlProc> p(m_usr.getProc(use_type_upobj ? "sp_obj_prop_get_v2_1" : "sp_obj_prop_get_v1_1"));
        // TODO: support domain_id and ion_id
        idx start_row = res.rows;
        if ( use_type_upobj ) {
            p->Add(m_id.domainId());
        }
        p->Add(m_id.objId()).Add(prop).Add((udx)(ePermCanRead | ePermCanBrowse));

        sSql & sql = m_usr.db();
        sStr decode_buf;
        if( p->resultOpen() ) {
            while( sql.resultNext() ) {
                static const idx colValue = sql.resultColId("value");
                static const idx colGroup = sql.resultColId("group");
                static const idx colEncoding = sql.resultColId("encoding");
                static const idx colBlobValue = sql.resultColId("blob_value");

                while( sql.resultNextRow() ) {
                    idx group_len = 0, value_len = 0;
                    const char * group = sql.resultValue(colGroup, 0, &group_len);
                    const char * value = sql.resultValue(colValue, 0, &value_len);
                    if( idx encoding = sql.resultIValue(colEncoding) ) {
                        decode_buf.cut0cut();
                        idx blob_len = 0;
                        const void * blob_value = sql.resultValue(colBlobValue, 0, &blob_len);
                        if( m_usr.decodeField(&decode_buf, encoding, value, value_len, blob_value, blob_len) ) {
                            value = decode_buf.ptr();
                            value_len = decode_buf.length();
                        } else {
                            value = 0;
                            value_len = 0;
                        }
                    }
                    res.addRow().addCol(value, value_len).addCol(group, group_len);
                    if( res.rows == 1 ) {
                        res.setColId(0, "value");
                        res.setColId(1, "group");
                    }
                }
            }
            sql.resultClose();
        }

        if( sort ) {
            sortProps(res, start_row, res.rows - start_row);
        }
    }
    return res.rows;
}

const char* sUsrObj::propGet(const char* prop, sStr* buffer) const
{
    sVarSet res;
    propGet(prop, res);
    // TODO validate to be single value, need to return error somehow!!!
    if( res.rows == 1 ) {
        if(!buffer) {
            buffer = &locBuf;
            locBuf.cut(0);
        }
        idx pos = buffer->length();
        idx size = 0;
        const char* val = res.val(0, 0, &size);
        if(val) {
            buffer->add(val, size);
        }
        buffer->add0();
        return buffer->ptr(pos);
    }
    return 0;
}

const char* sUsrObj::propGet00(const char* prop, sStr* buffer00, const char * altSeparator) const
{
    sVarSet res;
    propGet(prop, res, true);
    if( res.rows ) {
        if( !buffer00 ) {
            buffer00 = &locBuf;
            locBuf.cut(0);
        }
        idx pos = buffer00->length();
        for(idx i = 0; i < res.rows; ++i) {

            if( pos != buffer00->length()){
                if(altSeparator)buffer00->add(altSeparator,sLen(altSeparator));
                else buffer00->add0();
            }
            idx size = 0;
            const char* val = res.val(i, 0, &size);
            buffer00->add(val, size);
        }
        buffer00->add0(2);
        return buffer00->ptr(pos);
    }
    return 0;
}

idx sUsrObj::propGetHiveIds(const char * prop, sVec<sHiveId> & res) const
{
    sVarSet tbl;
    propGet(prop, tbl, true);
    if( tbl.rows ) {
        sHiveId * id = res.add(tbl.rows);
        if( id ) {
            for(idx i = 0; i < tbl.rows; ++i, ++id) {
                id->parse(tbl.val(i, 0));
            }
            return tbl.rows;
        }
    }
    return 0;
}

sUsrTypeField::EType sUsrObj::propGetValueType(const char* prop) const
{
    const sUsrTypeField * fld = propGetTypeField(prop);
    return fld ? fld->type() : sUsrTypeField::eInvalid;
}

const sUsrTypeField * sUsrObj::propGetTypeField(const char* prop) const
{
    const sUsrType2 * utype = getType();
    return utype ? utype->getField(m_usr, prop) : 0;
}

bool sUsrObj::isTypeOf(const char* pattern) const
{
    if( const sUsrType2 * utype = getType() ) {
        return utype->nameMatch(pattern);
    }
    return false;
}

const char* sUsrObj::getTypeName(void) const
{
    const sUsrType2 * utype = getType();
    return utype ? utype->name() : 0;
}

sUsrObj* sUsrObj::cast(const char* type_name)
{
    const sUsrType2 * typTo = sUsrType2::ensure(m_usr, type_name);
    const sUsrType2 * typFrom = getType();
    if( m_usr.isAllowed(Id(), ePermCanWrite) && typTo && typFrom && strcasecmp(typFrom->name(), typTo->name()) != 0 ) {
        std::auto_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_cast"));
        p->Add(m_id.domainId()).Add(m_id.objId()).Add(typFrom->id().domainId()).Add(typFrom->id().objId()).Add(typTo->id().domainId()).Add(typTo->id().objId());
        if( p->execute() ) {
            m_usr.cacheRemove(m_id);
            m_usr.audit(sUsr::eUserAuditActions, __func__, "objID='%s'; type='%s'", Id().print(), type_name);
            return m_usr.objFactory(m_id);
        }
        return 0;
    }
    return this;
}

bool sUsrObj::actDelete(void)
{
    static bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
    if( m_usr.isAllowed(Id(), ePermCanDelete) && onDelete() ) {
        std::auto_ptr<sSql::sqlProc> p(m_usr.getProc(use_type_upobj ? "sp_obj_erase_v2" : "sp_obj_erase"));
        if( use_type_upobj ) {
            p->Add(m_id.domainId());
        }
        p->Add(m_id.objId()).Add((udx) ePermCanDelete).Add((udx) (ePermCanBrowse | ePermCanRead)).Add((udx) eFlagRestrictive);
        if( p->execute() ) {
            m_usr.audit(sUsr::eUserAuditActions, __func__, "objID='%s'", Id().print());
            m_id.reset();
        }
    }
    return !m_id;
}

void sUsrObj::cleanup(void)
{
    if( (m_usr.isAllowed(Id(), ePermCanWrite) || m_usr.isAllowed(Id(), ePermCanDelete)) && getPath(m_path, m_id) ) {
        sDir d;
        d.find(sFlag(sDir::bitFiles), m_path, "~");
        for(const char* ptr = d.ptr(); ptr && *ptr; ptr = sString::next00(ptr, 1)) {
            sFile::remove(ptr);
        }
        d.find(sFlag(sDir::bitSubdirs), m_path, "~");
        for(const char* ptr = d.ptr(); ptr && *ptr; ptr = sString::next00(ptr, 1)) {
            sDir::removeDir(ptr, true);
        }
    }
}

bool sUsrObj::purge(void)
{
    static bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
    if( m_usr.isAllowed(Id(), ePermCanDelete) && onPurge() ) {
        if( m_usr.updateStart() ) {
            std::auto_ptr<sSql::sqlProc> p(m_usr.db().Proc(use_type_upobj ?  "sp_obj_delete_v2" : "sp_obj_delete"));
            if( use_type_upobj ) {
                p->Add(m_id.domainId());
            }
            p->Add(m_id.objId());
            if( p->execute() && m_usr.updateComplete() ) {
                if( getPath(m_path, m_id) ) {
                    sDir::removeDir(m_path);
                }
                m_usr.audit(sUsr::eUserAuditActions, __func__, "objID='%s'", Id().print());
                m_id.reset();
            } else {
                m_usr.updateAbandon();
            }
        }
    }
    return !m_id;
}

udx sUsrObj::propBulk(sVarSet & list, const char* view_name, const char* filter00) const
{
    filter00 = (filter00 && sLen(filter00)) ? filter00 : 0;
    sVarSet props;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        if( const sUsrType2 * utype = getType() ) {
            // t->props(m_usr, props, view_name, filter00);
            utype->props(m_usr, props, filter00);
        }
        if( props.rows > 0 ) {
            sVarSet tmp;
            if( filter00 ) {
                sStr prp;
                idx cnm = props.colId("name");
                for(idx r = 0; r < props.rows; ++r) {
                    prp.printf("%s", props.val(r, cnm));
                    prp.add0();
                }
                prp.add0(2);
                propBulk(prp, tmp);
            } else {
                propBulk(0, tmp);
            }
            propBulk(tmp, list, view_name, filter00);
        }
    }
    return list.rows;
}

void sUsrObj::propEval(sUsrObjRes & res, const char * filter00) const
{
    sUsrObjRes::TObjProp * self =  res.get(Id());
    if( self ) {
        filter00 = (filter00 && sLen(filter00)) ? filter00 : 0;
        sVarSet props;
        if( const sUsrType2 * utype = getType() ) {
            utype->props(m_usr, props, filter00);
        }
        if( props.rows > 0 ) {
            const idx cnm = props.colId("name");
            const idx cvrt = props.colId("is_virtual_fg");
            const idx cmlt = props.colId("is_multi_fg");
            // first inject all virtual properties
            sVarSet vtmp;
            for(idx r = 0; r < props.rows; ++r) {
                const char * nm = props.val(r, cnm);
                if( props.uval(r, cvrt) ) {
                    if( props.uval(r, cmlt) || !res.get(*self, nm) ) {
                        vtmp.empty();
                        propGet(nm, vtmp);
                        for(idx t = 0; t < vtmp.rows; ++t) {
                            idx pl, vl;
                            const char * p = vtmp.val(t, 1, &pl);
                            const char * v = vtmp.val(t, 0, &vl);
                            res.add(*self, nm, p, pl, v, vl);
                        }
                    }
                }
            }
            if( filter00 && sString::compareChoice("_brief", filter00, 0, true, 0, true) != sNotIdx && !res.get(*self, "_brief")) {
                sStr brief, tmp;
                idx cbrf = props.colId("brief");
                for(idx f = 0; f < props.rows; ++f) { // preserves order of fields in brief!
                    const char * b = props.val(f, cbrf);
                    if( b && b[0] ) {
                        const sUsrObjRes::TPropTbl * tbl = res.get(*self, props.val(f, cnm));
                        if( tbl ) { // take only top,brief field should not be multi-value actually
                            brief.shrink00();
                            brief.printf(" ");
                            tmp.cut0cut(0);
                            sString::searchAndReplaceStrings(&tmp, b, 0, "$_(v)"__, res.getValue(tbl), 0, false);
                            sString::searchAndReplaceStrings(&brief, tmp, 0, "$_(t)"__, getType()->title(), 0, false);
                        }
                    }
                }
                if( !brief ) {
                    brief.printf(" Object of type '%s'", getType()->title());
                }
                res.add(*self, "_brief", 0, 0, brief.ptr(1), brief.length() - 1);
            }
            sStr sumflt;
            if( filter00 && sString::compareChoice("_summary", filter00, 0, true, 0, true) != sNotIdx ) {
                // re-expand _summary into filter00
                sVarSet sump;
                getType()->props(m_usr, sump, "_summary");
                idx cnm = sump.colId("name");
                for(idx r = 0; r < sump.rows; ++r) {
                    sumflt.printf("%s", sump.val(r, cnm));
                    sumflt.add0(1);
                }
                for(const char * f = filter00; f; f = sString::next00(f)) {
                    sumflt.printf("%s", f);
                    sumflt.add0(1);
                }
                filter00 = sumflt;
            }
            // keep only prop in filter00
            for(idx p = 0; p < self->dim(); ++p) {
                const char * pnm = (const char *) self->id(p);
                if( filter00 && sString::compareChoice(pnm, filter00, 0, true, 0, true) == sNotIdx ) {
                    res.del(*self, pnm);
                }
            }
        }
    }
}

void sUsrObj::propBulk(sVarSet & src, sVarSet & dst, const char* view_name, const char* filter00) const
{
    filter00 = (filter00 && sLen(filter00)) ? filter00 : 0;
    sVarSet props;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        if( const sUsrType2 * utype = getType() ) {
            //utype->props(m_usr, props, view_name, filter00);
            utype->props(m_usr, props, filter00);
        }
        if( props.rows > 0 ) {
            idx cnm = props.colId("name");
            idx cvrt = props.colId("is_virtual_fg");
            // first inject all virtual properties
            sVarSet vtmp;
            for(idx r = 0; r < props.rows; ++r) {
                const char * nm = props.val(r, cnm);
                if( props.uval(r, cvrt) ) {
                    vtmp.empty();
                    propGet(nm, vtmp);
                    for(idx t = 0; t < vtmp.rows; ++t) {
                        dst.addRow().addCol(Id()).addCol(nm).addCol(vtmp.val(t, 1)).addCol(vtmp.val(t, 0));
                    }
                }
            }
            // copy all visible properties, skipping virtual and not from filter00
            static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
            const udx shift = use_type_upobj ? 1 : 0;
            for(idx r = 0; r < src.rows; ++r) {
                // copy my properties only
                sHiveId rid(use_type_upobj ? src.uval(r, 0): 0, use_type_upobj ? src.uval(r, 1) : src.uval(r, 0), 0);
                if( rid == Id() ) {
                    const char * pnm = src.val(r, 1 + shift);
                    if( pnm && pnm[0] ) {
                        bool doCopy = false;
                        for(idx p = 0; p < props.rows; ++p) {
                            if( strcasecmp(pnm, props.val(p, cnm)) == 0 && !props.uval(p, cvrt) &&
                                (!filter00 || sString::compareChoice(pnm, filter00, 0, true, 0, true) != sNotIdx)) {
                                doCopy = true;
                                break;
                            }
                        }
                        if( doCopy ) {
                            dst.addRow().addCol(Id()).addCol(pnm).addCol(src.val(r, 2 + shift)).addCol(src.val(r, 3 + shift));
                        }
                    }
                }
            }
            if( filter00 && sString::compareChoice("_brief", filter00, 0, true, 0, true) != sNotIdx ) {
                sStr brief, tmp;
                idx cbrf = props.colId("brief");
                for(idx f = 0; f < props.rows; ++f) { // preserves order of fields in brief!
                    for(idx r = 0; r < src.rows; ++r) {
                        sHiveId rid(use_type_upobj ? src.uval(r, 0): 0, use_type_upobj ? src.uval(r, 1) : src.uval(r, 0), 0);
                        if( rid == Id() ) {
                            const char * pnm = src.val(r, 1 + shift);
                            if( strcasecmp(pnm, props.val(f, cnm)) == 0 ) {
                                const char * b = props.val(f, cbrf);
                                if( b && b[0] ) {
                                    brief.shrink00();
                                    brief.printf(" ");
                                    tmp.cut0cut(0);
                                    sString::searchAndReplaceStrings(&tmp, b, 0, "$_(v)"__, src.val(r, 3 + shift), 0, false);
                                    sString::searchAndReplaceStrings(&brief, tmp, 0, "$_(t)"__, getType()->title(), 0, false);
                                }
                            }
                        }
                    }
                }
                if( !brief ) {
                    brief.printf(" Object of type '%s'", getType()->title());
                }
                dst.addRow().addCol(Id()).addCol("_brief").addCol((const char*)0).addCol(brief.ptr(1));
            }
        }
    }
}

void sUsrObj::propBulk(const char * filter00, sVarSet & list) const
{
    static const bool use_type_upobj = sString::parseBool(getenv("TYPE_UPOBJ"));
    sStr props;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        sString::glue00(&props, filter00, "%s", ",");
        std::auto_ptr<sSql::sqlProc> p(m_usr.getProc(use_type_upobj ? "sp_obj_prop_list_v2_1" : "sp_obj_prop_list_v1_1"));
        if( use_type_upobj ) {
            sStr idstr;
            sSql::exprInList(idstr, "domainID", "objID", &m_id, 1, false);
            p->Add(idstr).Add(props);
        } else {
            p->Add(Id().objId()).Add(props).Add((udx) (ePermCanRead | ePermCanBrowse));
        }
        p->getTable(&list);
    }
}

udx sUsrObj::propBulk(sVar& form) const
{
    sVarSet list;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        propBulk(0, list);
        for(idx r = 0; r < list.rows; ++r) {
            // TODO multi-value support
            form.inp(list.val(r, 1), list.val(r, 3));
        }
    }
    return list.rows;
}

const sUsrObjPropsTree * sUsrObj::propsTree(const char* view_name, const char* filter00, bool force_reload) const
{
    bool is_default = !view_name && !filter00;

    if( !m_propsTree.get() )
        m_propsTree.reset(new CachedPropsTree(m_usr, getTypeName()));

    if( !m_propsTree.get() )
        return 0;

    if( !is_default || !m_propsTree->is_default || force_reload ) {
        propBulk(m_propsTree->t.getTable(), view_name, filter00);
        m_propsTree->t.useTable(m_propsTree->t.getTable());
        m_propsTree->is_default = is_default;
    }
    return &(m_propsTree->t);
}

idx sUsrObj::actions(sVec<sStr>& actions) const
{
    // very quick fix
    if( const sUsrType2 * utype = getType() ) {
        idx nact = utype->dimActions(m_usr);
        for(idx i = 0; i < nact; i++) {
            const sUsrAction * act = utype->getAction(m_usr, i);
            if( act->isObjAction() && m_usr.isAllowed(Id(), act->requiredPermission()) ) {
                actions.add(1)->printf("%s", act->name());
                // TODO: use ids instead of names?
            }
        }
    }
    return actions.dim();
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Files associated with object
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void sUsrObj::makeFileName(sStr & buf, const sStr & key)
{
    if( key ) {
        buf.printf("%s%s", key[0] == '.' ? "_" : "", key.ptr());
    }
}

const char * sUsrObj::addFilePathname(sStr & buf, bool overwrite, const char* key, ...) const
{
    if( m_usr.isAllowed(Id(), ePermCanWrite) ) {
        const idx pos = buf.pos();
        if( m_id && key ) {
            sStr ext;
            sCallVarg(ext.vprintf, key);
            if( getPath(m_path, m_id, true) ) {
                buf.printf("%s", m_path.ptr());
                makeFileName(buf, ext);
                if( sFile::exists(buf.ptr(pos)) ) {
                    if( !overwrite || (overwrite && !sFile::remove(buf.ptr(pos))) ) {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
            }
        }
        if( buf.pos() != pos ) {
            m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; file='%s'", Id().print(), sFilePath::nextToSlash(buf.ptr(pos)));
            return buf.ptr(pos);
        }
    }
    return 0;
}

char * sUsrObj::trashFilePathname(sStr & buf, bool overwrite, const char* key, ...)
{
    if( m_usr.isAllowed(Id(), ePermCanWrite) ) {
        const idx pos = buf.pos();
        if( m_id && key ) {
            sStr ext;
            sCallVarg(ext.vprintf, key);
            if( getPath(m_path, m_id, true) ) {
                buf.printf("%s._trash.", m_path.ptr());
                makeFileName(buf, ext);
                if( sFile::exists(buf.ptr(pos)) ) {
                    if( !overwrite || (overwrite && !sFile::remove(buf.ptr(pos))) ) {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
                if( buf.pos() != pos ) {
                    const idx pos_end = buf.length();
                    buf.add0();
                    buf.printf("%s", m_path.ptr());
                    makeFileName(buf, ext);
                    if( sFile::exists(buf.ptr(pos_end + 1)) && sFile::rename(buf.ptr(pos_end + 1), buf.ptr(pos)) ) {
                        buf.cut0cut(pos_end);
                    } else {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
            }
        }
        if( buf.pos() != pos ) {
            m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; file='%s'", Id().print(), sFilePath::nextToSlash(buf.ptr(pos)));
            return buf.ptr(pos);
        }
    }
    return 0;
}

char * sUsrObj::restoreFilePathname(sStr & buf, bool overwrite, const char* key, ...)
{
    if( m_usr.isAllowed(Id(), ePermCanWrite) ) {
        const idx pos = buf.pos();
        if( m_id && key ) {
            sStr ext;
            sCallVarg(ext.vprintf, key);
            if( getPath(m_path, m_id, true) ) {
                buf.printf("%s", m_path.ptr());
                makeFileName(buf, ext);
                if( sFile::exists(buf.ptr(pos)) ) {
                    if( !overwrite || (overwrite && !sFile::remove(buf.ptr(pos))) ) {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
                if( buf.pos() != pos ) {
                    const idx pos_end = buf.length();
                    buf.add0();
                    buf.printf("%s._trash.", m_path.ptr());
                    makeFileName(buf, ext);
                    if( sFile::exists(buf.ptr(pos_end + 1)) && sFile::rename(buf.ptr(pos_end + 1), buf.ptr(pos)) ) {
                        buf.cut0cut(pos_end);
                    } else {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
            }
        }
        if( buf.pos() != pos ) {
            m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; file='%s'", Id().print(), sFilePath::nextToSlash(buf.ptr(pos)));
            return buf.ptr(pos);
        }
    }
    return 0;
}

bool sUsrObj::delFilePathname(const char* key, ...) const
{
    if( m_usr.isAllowed(Id(), ePermCanWrite) ) {
        if( m_id && key ) {
            sStr ext;
            sCallVarg(ext.vprintf, key);
            if( getPath(m_path, m_id) ) {
                sStr buf("%s", m_path.ptr());
                makeFileName(buf, ext);
                if( sFile::remove(buf) ) {
                    m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; file='%s'", Id().print(), sFilePath::nextToSlash(buf.ptr()));
                    return true;
                }
            }
        }
    }
    return false;
}

const char * sUsrObj::getFilePathname(sStr & buf, const char* key, ...) const
{
    sStr s;
    sCallVarg(s.vprintf, key);
    return getFilePathnameX(buf, s, true);
}

const char * sUsrObj::getFilePathname00(sStr & buf, const char* key00) const
{
    const char * ret = 0;
    for(const char* p = key00; p; p = sString::next00(p) ) {
        sStr s("%s", p);
        if( (ret = getFilePathnameX(buf, s, true)) ) {
            break;
        }
    }
    return ret;
}

const char * sUsrObj::makeFilePathname(sStr & buf, const char* key, ...) const
{
    sStr s;
    sCallVarg(s.vprintf, key);
    return getFilePathnameX(buf, s, false);
}

const char * sUsrObj::getFilePathnameX(sStr & buf, const sStr & key, bool check_existence) const
{
    const idx pos = buf.pos();
    if( m_usr.isAllowed(Id(), ePermCanRead ) ) {
        if( m_id && getPath(m_path, m_id) ) {
            buf.printf("%s", m_path.ptr());
            makeFileName(buf, key);
            if( check_existence && !sFile::exists(buf.ptr(pos)) ) {
                buf[pos] = '\0';
                buf.cut(pos);
            }
        }
    }
    return buf.pos() != pos ? buf.ptr(pos) : 0;
}

udx sUsrObj::files(sDir & fileList00, idx dirListFlags, const char * mask /*  = "*" */, const char * relPath /* = 0 */) const
{
    if( m_usr.isAllowed(Id(), ePermCanRead | ePermCanRead ) ) {
        if( getPath(m_path, m_id) ) {
            fileList00.list(dirListFlags, m_path, mask, 0, relPath ? relPath : m_path.ptr());
        }
    }
    return m_path ? sString::cnt00(fileList00) : 0;
}

udx sUsrObj::fileProp(sStr& dst, bool as_csv, const char * wildcardList) const
{
    udx cntFiles = 0;
    if( m_usr.isAllowed(Id(), ePermCanRead | ePermCanDownload) ) {
        sStr tokenizedWildcardList;
        sString::searchAndReplaceSymbols(&tokenizedWildcardList, wildcardList, 0, ";", 0, 0, true, true, false, true);
        const idx flags = sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs) | sFlag(sDir::bitSubdirSlash);
        sDir fileList;
        for(const char * wildcard = tokenizedWildcardList.ptr(0); wildcard; wildcard = sString::next00(wildcard)) {
            cntFiles += files(fileList, flags, wildcard);
        }
        for(idx ie = 0; ie < fileList.dimEntries(); ie++) {
            if( as_csv ) {
                Id().print(dst);
                dst.printf(",_file,%"DEC",", ie);
                sString::escapeForCSV(dst, fileList.getEntryPath(ie));
                dst.addString("\n");
            } else {
                dst.addString("\nprop.");
                Id().print(dst);
                dst.printf("._file.%"DEC"=%s", ie, fileList.getEntryPath(ie));
            }
        }
    }
    return cntFiles;
}

udx sUsrObj::fileProp(sJSONPrinter& dst, const char * wildcardList, bool into_object) const
{
    udx cntFiles = 0;
    if( m_usr.isAllowed(Id(), ePermCanRead | ePermCanDownload) ) {
        if( !into_object ) {
            dst.startObject();
        }
        sStr tokenizedWildcardList;
        sString::searchAndReplaceSymbols(&tokenizedWildcardList, wildcardList, 0, ";", 0, 0, true, true, false, true);
        const idx flags = sFlag(sDir::bitFiles) | sFlag(sDir::bitSubdirs) | sFlag(sDir::bitSubdirSlash);
        sDir fileList;
        for(const char * wildcard = tokenizedWildcardList.ptr(0); wildcard; wildcard = sString::next00(wildcard)) {
            cntFiles += files(fileList, flags, wildcard);
        }
        if( cntFiles ) {
            dst.addKey("_file");
            dst.startArray();
            for(idx ie = 0; ie < fileList.dimEntries(); ie++) {
                dst.addValue(fileList.getEntryPath(ie));
            }
            dst.endArray();
        }
        if( !into_object ) {
            dst.endObject();
        }
    }
    return cntFiles;
}

// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Object Info
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

const sUsrType2 * sUsrObj::getType(void) const
{
    if( m_type_id ) {
        return sUsrType2::ensure(m_usr, m_type_id);
    } else {
        return m_usr.objType(Id(), &m_type_id);
    }
}
