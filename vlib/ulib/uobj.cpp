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
        abs_time.tv_sec += 5;
        if( pthread_mutex_timedlock(&g_lock, &abs_time) == 0 ) {
            fsz = r->free;
        }
#endif
    }
    pthread_mutex_unlock(&g_lock);

#endif
    return fsz;
}

char * sUsrObj::getPath(sStr & path, const sHiveId & id, bool create, sQPrideBase * qp_for_logging)
{
    if( id && !path ) {
        sStr suffix;
        if( id.domainId() != 0 ) {
            char buf[S_HIVE_ID_SHORT_BUFLEN];
            sHiveId::decodeDomainId(buf, id.domainId());
            suffix.printf("%s/", buf);
        }
        udx levelId = id.objId();
        const udx maxLevels = 2, levelBase = 1000;
        for(udx ilevel = 0; ilevel < maxLevels; ++ilevel) {
            udx curLevel = levelId % levelBase;
            suffix.printf("%03" UDEC "/", curLevel);
            levelId /= levelBase;
        }
        suffix.printf("%" UDEC, id.objId());
        for(idx i = 0; i < s_roots.dim() && !path; ++i) {
            if( s_roots.ptr(i)->path ) {
                for(const char * realStoragePath = s_roots.ptr(i)->path; realStoragePath && *realStoragePath; realStoragePath = sString::next00(realStoragePath)) {
                    path.printf("%s%s/", realStoragePath, suffix.ptr());
                    if( sDir::exists(path) ) {
                        break;
                    }
                    path.cut0cut();
                }
            }
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
            if( const char * realStoragePath = sString::next00(s_roots.ptr(found)->path, -1) ) {
                path.printf("%s%s/", realStoragePath, suffix.ptr());
                if( !sDir::makeDir(path) ) {
                    if( qp_for_logging ) {
                        qp_for_logging->logOut(sQPrideBase::eQPLogType_Error, "sUsrObj::%s() : mkdir '%s' failed : error %d - %s", __func__, path.ptr(), errno, strerror(errno));
                    } else {
#if _DEBUG
                        ::fprintf(stderr, "sUsrObj::%s() : mkdir '%s' failed : error %d - %s\n", __func__, path.ptr(), errno, strerror(errno));
#endif
                    }
                    path.cut0cut();
                }
            }
        }
    }
    return path ? path.ptr() : 0;
}

sRC sUsrObj::initStorage(const char * pathList, const udx default_min_free_space_per_volume, sQPrideBase * qp_for_logging)
{
    sStr path00, mpath;
    sString::searchAndReplaceSymbols(&path00, pathList, 0, ";,", 0, 0, true, true, false, true);
    path00.add0(3);
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
                *x = 0;
                x = sString::next00(x);
                if( x ) {
                    sscanf(x, "%" UDEC, &r->min_free);
                }
            }
            sString::searchAndReplaceSymbols(mpath.ptr(), 0, "|", 0, 0, true, false, false, false);
            for(const char * mp = mpath; mp && *mp; mp = sString::next00(mp)) {
                if( sDir::exists(mp) ) {
                    r->path.printf("%s", mp);
                    r->path.add0();
                } else {
                    if( qp_for_logging ) {
                        qp_for_logging->logOut(sQPrideBase::eQPLogType_Error, "sUsrObj::%s() : failed to read storage path '%s'", __func__, mp);
                    } else {
#if _DEBUG
                        ::fprintf(stderr, "sUsrObj::%s() : failed to read storage path '%s'\n", __func__, mp);
#endif
                    }
                    s_roots.empty();
                    return RC(sRC::eInitializing, sRC::eDiskSpace, sRC::eDirectory, sRC::eNotFound);
                }
            }

            if( r->path ) {
                r->path.add0(2);
            } else {
                r = 0;
                s_roots.cut(-1);
            }
        }
    }
    return sRC::zero;
}

sUsrObj::~sUsrObj()
{
}

udx sUsrObj::propSet(const char* prop, const char** paths, const char** values, const udx cntValues, bool isAppend, const udx * path_lens, const udx * value_lens)
{
    udx qty = 0;
    const sUsrTypeField * tf = propGetTypeField(prop);
    bool err = !tf;
    if( m_usr.isAllowed(Id(), ePermCanWrite) && !err ) {
        if( sIsExactly(prop, "created") || sIsExactly(prop, "modified") ) {
            qty = 1;
        } else {
            const bool single = !tf->isMulti();
            err = sLen(prop) <= 0 || sLen(prop) >= 256;
            const udx max_value_len = 23 * 1024 * 1024;
            udx skip_transaction = 0;
            for(udx i = 0; !err && i < cntValues; ++i ) {
                udx path_len = path_lens ? path_lens[i] : (paths ? sLen(paths[i]) : 0);
                udx value_len = value_lens ? value_lens[i] : (values ? sLen(values[i]) : 0);
                skip_transaction += path_len + value_len + 5;
                if( skip_transaction > max_value_len ) {
                    skip_transaction = 0;
                    break;
                }
            }
            const bool is_our_transaction = !skip_transaction && !m_usr.getUpdateLevel();
            for( idx itry = 0; !err && itry < sSql::max_deadlock_retries; itry++ ) {
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
                            if( value_len >= max_value_len ) {
                                err = true;
                                break;
                            }
                            if( blob_buf.pos() >= (idx)max_value_len ) {
                                err = true;
                                break;
                            }
                            if( i > chunk ) {
                                vsql.addString(",");
                            }
                            vsql.addString("(");
                            if( m_id.domainId() ) {
                                vsql.printf("%" UDEC ",", m_id.domainId());
                            } else {
                                vsql.addString("NULL,");
                            }
                            vsql.printf("%" UDEC ",", m_id.objId());
                            m_usr.db().protectValue(vsql, prop);
                            vsql.addString(",");
                            if( paths && paths[i] && (path_lens ? path_lens[i] : paths[i][0]) ) {
                                udx path_len = path_lens ? path_lens[i] : sLen(paths[i]);
                                if( path_len > 255 ) {
                                    err = true;
                                    break;
                                }
                                m_usr.db().protectValue(vsql, paths[i], path_lens ? path_lens[i] : 0);
                                vsql.addString(",");
                            } else if( !single ) {
                                vsql.printf("'1.%" UDEC "',", chunk + i + 1);
                            } else {
                                vsql.addString("NULL,");
                            }
                            if( value ) {
                                m_usr.db().protectValue(vsql, value, value_len);
                                vsql.addString(",");
                            } else {
                                vsql.addString("'',");
                            }
                            if( tf->defaultEncoding() ) {
                                vsql.printf("%" DEC ",", tf->defaultEncoding());
                            } else {
                                vsql.addString("NULL,");
                            }

                            if( blob_buf.pos() ) {
                                m_usr.db().protectBlob(vsql, blob_buf.ptr(), blob_buf.pos());
                            } else {
                                vsql.addString("NULL");
                            }
                            vsql.addString(")");
                        } while( ++i < cntValues );
                        if( !err ) {
                            std::unique_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_prop_set_v3"));
                            p->Add(m_id.domainId()).Add(m_id.objId()).Add((udx)ePermCanWrite).Add(vsql).Add(!isAppend && chunk == 0 ? prop : 0);
                            udx q = p->uvalue(0);
                            if( q == 0 ) {
                                err = true;
                                if( m_usr.db().HasFailed() && !(is_our_transaction && m_usr.hadDeadlocked()) ) {
                                    fprintf(stderr, "propSet() DB error %" UDEC " on objID = %s, prop = '%s' : %s\n", m_usr.db().Get_errno(), m_id.print(), prop, m_usr.db().Get_error().ptr());
                                    if( m_usr.QPride() ) {
                                        m_usr.QPride()->logOut(m_usr.db().in_transaction() ? sQPrideBase::eQPLogType_Warning : sQPrideBase::eQPLogType_Error,
                                            "propSet() DB error %" UDEC " on objID = %s, prop = '%s' : %s", m_usr.db().Get_errno(), m_id.print(), prop, m_usr.db().Get_error().ptr());
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
                            m_usr.updateAbandon();
                            err = false;
                            qty = 0;
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
        m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; prop='%s'; updated='%" UDEC "'", Id().print(), prop, qty);
    }
    return qty;
}

udx sUsrObj::propDel(const char * prop, const char * group, const char * value)
{
    if( !prop || !prop[0] || !m_usr.isAllowed(Id(), ePermCanWrite) ) {
        return 0;
    }
    std::unique_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_prop_del_v2"));
    p->Add(m_id.domainId()).Add(m_id.objId()).Add((udx)ePermCanWrite).Add(prop).Add(group).Add(value);
    udx qty = p->uvalue(0);
    if( qty ) {
        m_usr.audit(sUsr::eUserAuditFull, __func__, "objID='%s'; prop='%s'; path='%s'; value='%s'; updated='%" UDEC "'", Id().print(), prop, group ? group : "", value ? value : "", qty);
    }
    return qty;
}

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
        std::unique_ptr<sSql::sqlProc> p(usr.getProc("sp_obj_prop_init_v2"));
        p->Add(id.domainId()).Add(id.objId()).Add((udx)ePermCanWrite).Add(autofill_fld_csv.ptr());
        sVarSet tbl;
        p->getTable(&tbl);
        if( tbl.rows && tbl.cols && tbl.ival(0, 0) > 0 ) {
            return true;
        }
    }
    return false;
}

bool sUsrObj::propInit(bool keep_autofill)
{
    return propInitInternal(m_usr, this, m_id, keep_autofill);
}

bool sUsrObj::propInit(const sUsr& usr, const sHiveId & id, bool keep_autofill)
{
    return propInitInternal(usr, 0, id, keep_autofill);
}

idx sUsrObj::readPathElt(const char * path, const char ** next)
{
    if( !path ) {
        *next = 0;
        return -sIdxMax;
    }

    idx elt = strtoidx(path, (char**)next, 10);
    if( **next == '.' ) {
        (*next)++;
    } else if( **next ) {
        *next = 0;
        elt = -sIdxMax;
    } else if( path == *next ) {
        *next = 0;
    }
    return elt;
}

static idx sortPropsCallback(void * param, void * arr_param, idx i1, idx i2)
{
    sVarSet * res = static_cast<sVarSet*>(param);
    idx * arr = static_cast<idx*>(arr_param);

    if( res->cols == 4 ) {
        sHiveId id1(res->val(arr[i1], 0));
        sHiveId id2(res->val(arr[i2], 0));
        if( idx diff = id1.cmp(id2) ) {
            return diff;
        }
    }

    idx path_col = res->cols == 4 ? 2 : 1;

    const char * path1 = res->val(arr[i1], path_col);
    const char * path2 = res->val(arr[i2], path_col);
    do {
        idx cur_elt1 = sUsrObj::readPathElt(path1, &path1);
        idx cur_elt2 = sUsrObj::readPathElt(path2, &path2);
        if( cur_elt1 != cur_elt2 ) {
            return cur_elt1 - cur_elt2;
        }
    } while( path1 && path2 );

    if( res->cols == 4 ) {
        const char * name1 = res->val(arr[i1], 1);
        const char * name2 = res->val(arr[i2], 1);
        return strcasecmp(name1 ? name1 : "", name2 ? name2 : "");
    }
    return 0;
}

idx sUsrObj::sortProps(sVarSet & res, idx start_row, idx cnt)
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

udx sUsrObj::propGet(const char* prop, sVarSet& res, bool sort, bool allowSysInternal) const
{
    if( !allowSysInternal ) {
        const sUsrType2 * utype = getType();
        const sUsrTypeField * ufield = utype ? utype->getField(m_usr, prop) : 0;
        if( ufield && ufield->isSysInternal() ) {
            prop = 0;
        }
    }
    if( prop && prop[0] == '_' ) {
        if( strcasecmp(&prop[1], "type") == 0 ) {
            res.addRow().addCol(getTypeName()).addCol((const char*)0);
        } else if( strcasecmp(&prop[1], "effperm") == 0 ) {
            sStr tmp;
            tmp.printf("%" UDEC ",,", m_usr.groupId());
            permPrettyPrint(tmp, m_usr.objPermEffective(Id()), eFlagNone);
            res.addRow().addCol(tmp.ptr()).addCol("1.1");
        } else if( m_usr.isAllowed(Id(), ePermCanRead) ) {
            if( strcasecmp(&prop[1], "parent") == 0 ) {
                sUsrObjRes p;
                m_usr.objs2("^directory$+", p, 0, "child", IdStr());
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
    } else if( prop && m_usr.isAllowed(Id(), ePermCanRead) ) {
        std::unique_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_prop_get_v2_1"));
        idx start_row = res.rows;
        p->Add(m_id.domainId()).Add(m_id.objId()).Add(prop).Add((udx)(ePermCanRead | ePermCanBrowse));

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

const char* sUsrObj::propGet(const char* prop, sStr* buffer, bool allowSysInternal) const
{
    sVarSet res;
    propGet(prop, res, false, allowSysInternal);
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

const char* sUsrObj::propGet00(const char* prop, sStr* buffer00, const char * altSeparator, bool allowSysInternal) const
{
    sVarSet res;
    propGet(prop, res, true, allowSysInternal);
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
        std::unique_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_cast"));
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

bool sUsrObj::actDelete(const udx days)
{
    if( m_usr.isAllowed(Id(), ePermCanDelete) && onDelete() ) {
        std::unique_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_erase_v2"));
        p->Add(m_id.domainId()).Add(m_id.objId()).Add((udx) ePermCanDelete).Add((udx) (ePermCanBrowse | ePermCanRead)).Add((udx) eFlagRestrictive).Add(days);
        if( p->execute() ) {
            m_usr.audit(sUsr::eUserAuditActions, __func__, "objID='%s'; inDays='%" UDEC "'", Id().print(), days);
            if( days <= 0 ) {
                m_id.reset();
            }
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
    if( m_usr.isAllowed(Id(), ePermCanDelete) && onPurge() ) {
        if( m_usr.updateStart() ) {
            std::unique_ptr<sSql::sqlProc> p(m_usr.db().Proc("sp_obj_delete_v2"));
            p->Add(m_id.domainId()).Add(m_id.objId());
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

udx sUsrObj::propBulk(sVarSet & list, const char* view_name, const char* filter00, bool allowSysInternal) const
{
    filter00 = (filter00 && sLen(filter00)) ? filter00 : 0;
    sVarSet props;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        if( const sUsrType2 * utype = getType() ) {
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
            propBulk(tmp, list, view_name, filter00, allowSysInternal);
        }
    }
    return list.rows;
}

void sUsrObj::propEval(sUsrObjRes & res, const char * filter00, bool allowSysInternal) const
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
            sVarSet vtmp;
            for(idx r = 0; r < props.rows; ++r) {
                const char * nm = props.val(r, cnm);
                if( props.uval(r, cvrt) ) {
                    if( !res.get(*self, nm) ) {
                        vtmp.empty();
                        propGet(nm, vtmp, false, allowSysInternal);
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
                for(idx f = 0; f < props.rows; ++f) {
                    const char * b = props.val(f, cbrf);
                    if( b && b[0] ) {
                        const sUsrObjRes::TPropTbl * tbl = res.get(*self, props.val(f, cnm));
                        if( tbl ) {
                            brief.shrink00();
                            brief.printf(" ");
                            tmp.cut0cut(0);
                            sString::searchAndReplaceStrings(&tmp, b, 0, "$_(v)" __, res.getValue(tbl), 0, false);
                            sString::searchAndReplaceStrings(&brief, tmp, 0, "$_(t)" __, getType()->title(), 0, false);
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
            for(idx p = 0; p < self->dim(); ++p) {
                const char * pnm = (const char *) self->id(p);
                if( filter00 && sString::compareChoice(pnm, filter00, 0, true, 0, true) == sNotIdx ) {
                    res.del(*self, pnm);
                }
            }
        }
    }
}

void sUsrObj::propBulk(sVarSet & src, sVarSet & dst, const char* view_name, const char* filter00, bool allowSysInternal) const
{
    filter00 = (filter00 && sLen(filter00)) ? filter00 : 0;
    sVarSet props;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        if( const sUsrType2 * utype = getType() ) {
            utype->props(m_usr, props, filter00);
        }
        if( props.rows > 0 ) {
            idx cnm = props.colId("name");
            idx cvrt = props.colId("is_virtual_fg");
            sVarSet vtmp;
            for(idx r = 0; r < props.rows; ++r) {
                const char * nm = props.val(r, cnm);
                if( props.uval(r, cvrt) ) {
                    vtmp.empty();
                    propGet(nm, vtmp, false, allowSysInternal);
                    for(idx t = 0; t < vtmp.rows; ++t) {
                        dst.addRow().addCol(Id()).addCol(nm).addCol(vtmp.val(t, 1)).addCol(vtmp.val(t, 0));
                    }
                }
            }
            for(idx r = 0; r < src.rows; ++r) {
                sHiveId rid(src.uval(r, 0), src.uval(r, 1), 0);
                if( rid == Id() ) {
                    const char * pnm = src.val(r, 2);
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
                            dst.addRow().addCol(Id()).addCol(pnm).addCol(src.val(r, 3)).addCol(src.val(r, 4));
                        }
                    }
                }
            }
            if( filter00 && sString::compareChoice("_brief", filter00, 0, true, 0, true) != sNotIdx ) {
                sStr brief, tmp;
                idx cbrf = props.colId("brief");
                for(idx f = 0; f < props.rows; ++f) {
                    for(idx r = 0; r < src.rows; ++r) {
                        sHiveId rid(src.uval(r, 0), src.uval(r, 1), 0);
                        if( rid == Id() ) {
                            const char * pnm = src.val(r, 2);
                            if( strcasecmp(pnm, props.val(f, cnm)) == 0 ) {
                                const char * b = props.val(f, cbrf);
                                if( b && b[0] ) {
                                    brief.shrink00();
                                    brief.printf(" ");
                                    tmp.cut0cut(0);
                                    sString::searchAndReplaceStrings(&tmp, b, 0, "$_(v)" __, src.val(r, 4), 0, false);
                                    sString::searchAndReplaceStrings(&brief, tmp, 0, "$_(t)" __, getType()->title(), 0, false);
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
    sStr props;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        sString::glue00(&props, filter00, "%s", ",");
        std::unique_ptr<sSql::sqlProc> p(m_usr.getProc("sp_obj_prop_list_v2_1"));
        sStr idstr;
        sSql::exprInList(idstr, "domainID", "objID", &m_id, 1, false);
        p->Add(idstr).Add(props);
        p->getTable(&list);
    }
}

udx sUsrObj::propBulk(sVar& form) const
{
    sVarSet list;
    if( m_usr.isAllowed(Id(), ePermCanRead) ) {
        propBulk(0, list);
        for(idx r = 0; r < list.rows; ++r) {
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

idx sUsrObj::actions(sVec<sStr>& actions, const bool use_ids) const
{
    if( const sUsrType2 * utype = getType() ) {
        idx nact = utype->dimActions(m_usr);
        for(idx i = 0; i < nact; i++) {
            const sUsrAction * act = utype->getAction(m_usr, i);
            if( act->isObjAction() && m_usr.isAllowed(Id(), act->requiredPermission()) ) {
                actions.add(1)->printf("%s", use_ids ? act->id().print() : act->name() );
            }
        }
    }
    return actions.dim();
}

idx sUsrObj::jscomponents(sVec<sStr>& components, const bool use_ids) const
{
    if( const sUsrType2 * utype = getType() ) {
        idx q = utype->dimJSComponents(m_usr);
        for(idx i = 0; i < q; i++) {
            const sUsrJSComponent * c = utype->getJSComponent(m_usr, i);
            components.add(1)->printf("%s", use_ids ? c->id().print() : c->name());
        }
    }
    return components.dim();
}


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
            if( getPath(m_path, m_id, true, m_usr.QPride()) ) {
                buf.printf("%s", m_path.ptr());
                makeFileName(buf, ext);
                if( sDir::exists(buf.ptr(pos)) ) {
                    if( !overwrite || (overwrite && !sDir::removeDir(buf.ptr(pos), true)) ) {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
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
                if( sDir::exists(buf.ptr(pos)) ) {
                    if( !overwrite || (overwrite && !sDir::removeDir(buf.ptr(pos), true)) ) {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
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
                    if( sDir::exists(buf.ptr(pos_end + 1)) && sFile::rename(buf.ptr(pos_end + 1), buf.ptr(pos)) ) {
                        buf.cut0cut(pos_end);
                    } else if( sFile::exists(buf.ptr(pos_end + 1)) && sFile::rename(buf.ptr(pos_end + 1), buf.ptr(pos)) ) {
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
            if( getPath(m_path, m_id, true, m_usr.QPride()) ) {
                buf.printf("%s", m_path.ptr());
                makeFileName(buf, ext);
                if( sDir::exists(buf.ptr(pos)) ) {
                    if( !overwrite || (overwrite && !sDir::removeDir(buf.ptr(pos), true)) ) {
                        buf[pos + 1] = '\0';
                        buf.cut(pos);
                    }
                }
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
                    if( sDir::exists(buf.ptr(pos_end + 1)) && sDir::rename(buf.ptr(pos_end + 1), buf.ptr(pos)) ) {
                        buf.cut0cut(pos_end);
                    } else if( sFile::exists(buf.ptr(pos_end + 1)) && sFile::rename(buf.ptr(pos_end + 1), buf.ptr(pos)) ) {
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
                if( (sFile::exists(buf) && sFile::remove(buf)) || (sDir::exists(buf) && sDir::removeDir(buf, true)) ) {
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
            if( check_existence && ((!strchr("\\/", *(buf.last()-1)) && !sFile::exists(buf.ptr(pos))) ||
                                    ( strchr("\\/", *(buf.last()-1)) &&  !sDir::exists(buf.ptr(pos)))) ) {
                buf[pos] = '\0';
                buf.cut(pos);
            }
        }
    }
    return buf.pos() != pos ? buf.ptr(pos) : 0;
}

udx sUsrObj::files(sDir & fileList00, idx dirListFlags, const char * mask, const char * relPath) const
{
    if( m_usr.isAllowed(Id(), ePermCanRead | ePermCanDownload) ) {
        if( getPath(m_path, m_id) ) {
            sStr canon;
            sString::searchAndReplaceStrings(&canon, mask, 0, "../" __, "" __, sIdxMax, true);
            fileList00.find(dirListFlags, m_path.ptr(), canon, 0, sIdxMax, relPath ? relPath : m_path.ptr());
        }
    }
    return m_path ? sString::cnt00(fileList00) : 0;
}

udx sUsrObj::fileProp(sStr & dst, const bool as_csv, const char * wildcard, const bool show_file_size) const
{
    udx cntFiles = 0;
    if( m_usr.isAllowed(Id(), ePermCanRead | ePermCanDownload) ) {
        sStr tokenizedWildcardList;
        sString::searchAndReplaceSymbols(&tokenizedWildcardList, wildcard, 0, ";", 0, 0, true, true, false, true);
        const idx flags = sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive) | sFlag(sDir::bitEntryFlags);
        sDir fileList;
        for(const char * wildcard = tokenizedWildcardList.ptr(0); wildcard; wildcard = sString::next00(wildcard)) {
            cntFiles += files(fileList, flags, wildcard);
        }
        for(idx ie = 0; ie < fileList.dimEntries(); ie++) {
            const char * flnm = fileList.getEntryPath(ie);
            if( flnm) {
                if( as_csv ) {
                    Id().print(dst);
                    dst.printf(",_file,%" DEC ",", ie);
                    sString::escapeForCSV(dst, flnm);
                    if( show_file_size ) {
                        const char * aflnm = fileList.getEntryAbsPath(ie);
                        dst.printf(",%" DEC, sFile::size(aflnm));
                    }
                    dst.addString("\n");
                } else {
                    dst.addString("\nprop.");
                    Id().print(dst);
                    dst.printf("._file.%" DEC "=%s", ie, flnm);
                }
            }
        }
    }
    return cntFiles;
}

udx sUsrObj::fileProp(sJSONPrinter & dst, const char * wildcard, const bool into_object, const bool show_file_size) const
{
    udx cntFiles = 0;
    if( m_usr.isAllowed(Id(), ePermCanRead | ePermCanDownload) ) {
        if( !into_object ) {
            dst.startObject();
        }
        sStr tokenizedWildcardList;
        sString::searchAndReplaceSymbols(&tokenizedWildcardList, wildcard, 0, ";", 0, 0, true, true, false, true);
        const idx flags = sFlag(sDir::bitFiles) | sFlag(sDir::bitRecursive) | sFlag(sDir::bitEntryFlags);
        sDir fileList;
        for(const char * wildcard = tokenizedWildcardList.ptr(0); wildcard; wildcard = sString::next00(wildcard)) {
            cntFiles += files(fileList, flags, wildcard);
        }
        if( cntFiles ) {
            dst.addKey("_file");
            dst.startArray();
            for(idx ie = 0; ie < fileList.dimEntries(); ie++) {
                const char * flnm = fileList.getEntryPath(ie);
                if( flnm) {
                    if( show_file_size ) {
                        dst.startArray();
                        dst.addValue(flnm);
                        const char * aflnm = fileList.getEntryAbsPath(ie);
                        dst.addValue(sFile::size(aflnm));
                        dst.endArray();
                    } else {
                        dst.addValue(flnm);
                    }
                }
            }
            dst.endArray();
        }
        if( !into_object ) {
            dst.endObject();
        }
    }
    return cntFiles;
}


const sUsrType2 * sUsrObj::getType(void) const
{
    if( m_type_id ) {
        return sUsrType2::ensure(m_usr, m_type_id, false, false, true);
    } else {
        return m_usr.objType(Id(), &m_type_id);
    }
}
