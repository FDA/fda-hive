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
#include <ulib/ufolder.hpp>

using namespace slib;

const char * s_roots00 = "HIVE Space" _ "Inbox" _ "Trash" _ "Shared with me" __;
const char * s_Home = s_roots00;
const char * s_Inbox = s_Home + sLen(s_Home) + 1;
const char * s_Trash = s_Inbox + sLen(s_Inbox) + 1;
const char * s_SharedWithMe = s_Trash + sLen(s_Trash) + 1;

const char * const s_myTypeName = "folder";
const char * const s_sysTypeName = "sysfolder";

bool sUsrFolder::folder(sHiveId & out_id, const sUsr& usr, const char * path, bool doCreate, sUsrFolder * parentFolder, const char * s_type)
{
    out_id.reset();
    if( !path || !path[0] ) {
        return false;
    }
    s_type = s_type ? s_type : s_myTypeName;
    std::unique_ptr<sUsrObj> obj(parentFolder ? new sUsrFolder(usr, parentFolder->Id()) : 0);
    sStr p00;
    sString::searchAndReplaceSymbols(&p00, path, 0, "/\\", 0, 0, true, true, false, true);
    std::unique_ptr<sUsrObj> parent;
    const bool isSys = strcmp(s_type, s_sysTypeName) == 0;
    for(const char * p = p00; p && *p; p = sString::next00(p)) {
        parent.reset(obj.release());
        if( parent.get() && !(parent.get()->isTypeOf(s_sysTypeName) && isSys )  ) {
            sVarSet children;
            parent->propGet("child", children);
            for(idx r = 0; r < children.rows; ++r) {
                sHiveId child(children.val(r, 0));
                std::unique_ptr<sUsrObj> o(usr.objFactory(child));
                if( o.get() && o->Id() && o->isTypeOf(s_type) ) {
                    const char * n = o->propGet("name");
                    if( n && strcmp(n, p) == 0 ) {
                        obj.reset(o.release());
                        break;
                    }
                }
            }
        } else {
            if( !isSys ) {
                std::unique_ptr<sUsrFolder> p_obj(parent.get() ? (sUsrFolder *) usr.objFactory(parent->Id()) : 0);
                sHiveId s_folderId;
                folder(s_folderId, usr, p, doCreate, p_obj.get(), s_type);
                if( s_folderId ) {
                    obj.reset(usr.objFactory(s_folderId));
                }
            } else {
                sUsrObjRes roots;
                usr.objs2(s_sysTypeName, roots, 0, "name", p);
                if( roots.dim() >= 1 ) {
                    obj.reset(usr.objFactory(*roots.firstId()));
                }
            }
        }
        if( !obj.get() ) {
            if( !doCreate ) {
                break;
            } else {
                bool su = false;
                if( isSys ) {
                    su = sSysFolder::su(usr, true);
                }
                obj.reset(new sUsrObj(usr, s_type));
                if( obj->Id() ) {
                    obj->propSet("name", p);
                    if( parent.get() ) {
                        sUsrFolder * parentF = (sUsrFolder *) usr.objFactory(parent->Id());
                        parentF->attach(*obj);
                        delete parentF;
                    }
                }
                if( isSys ) {
                    sSysFolder::su(usr, su);
                }
            }
        }
    }
    if( obj.get() ) {
        out_id = obj->Id();
    }
    return out_id;
}

bool sUsrFolder::su(const sUsr& usr, const bool newval)
{
    const bool was = usr.m_SuperUserMode;
    const_cast<sUsr&>(usr).m_SuperUserMode = newval;
    return was;
}


sUsrFolder::sUsrFolder(const sUsr& usr, const char * path)
    : sUsrObj(usr, sHiveId::zero), progress_CallbackFunction(0), progress_CallbackParamObject(0), progress_CallbackParamReqID(0)
{
    folder(m_id, usr, path, true);
    if( !m_usr.objGet(m_id) ) {
        m_id.reset();
    }
}

sUsrFolder::sUsrFolder(const sUsr& usr, const sHiveId & objId)
    : sUsrObj(usr, objId), progress_CallbackFunction(0), progress_CallbackParamObject(0), progress_CallbackParamReqID(0)
{
}

sUsrFolder::sUsrFolder(const sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission)
    : sUsrObj(usr, objId, ptypeId, permission), progress_CallbackFunction(0), progress_CallbackParamObject(0), progress_CallbackParamReqID(0)
{
}

sUsrFolder * sUsrFolder::find(const sUsr & usr, const char * path)
{
    sHiveId id;
    folder(id, usr, path, false);
    return (sUsrFolder*) (usr.objFactory(id));
}

sUsrFolder * sUsrFolder::createSubFolder(const char * fmt, ...)
{
    sStr x;
    sCallVarg(x.vprintf, fmt);
    sHiveId id;
    folder(id, m_usr, x, true, this);
    return (sUsrFolder*) (m_usr.objFactory(id));
}

sUsrFolder * sUsrFolder::find(const char* path) const
{
    sHiveId id;
    folder(id, m_usr, path, false, const_cast<sUsrFolder*>(this));
    return (sUsrFolder*) (m_usr.objFactory(id));
}

bool sUsrFolder::attach(const sUsrObj & obj)
{
    bool allowed = true;
    if( obj.isTypeOf(s_myTypeName) ) {
        sUsrFolder * f_obj = (sUsrFolder *)m_usr.objFactory(obj.Id());
        sUsrObjRes parents;
        m_usr.objs2(s_myTypeName, parents, 0, "child", f_obj->IdStr());
        if( parents.dim() || f_obj->isDescendant(this->Id()) || f_obj->Id() == this->Id() ) {
            allowed = false;
        }
        delete f_obj;
    }
    else if( obj.isTypeOf(s_sysTypeName) ) {
        allowed = false;
    }
    if(allowed) {
        const char * c = obj.IdStr();
        allowed = false;
        if(!isChild(obj.Id())) {
            allowed = TParent::propSet("child", 0, &c, 1, true);
        }
    }

    if( allowed ) {
        m_children.empty();
    }
    return allowed;
}

bool sUsrFolder::detach(const sUsrObj & obj) {
    const char * c = obj.IdStr();

    if( !obj.isTypeOf(s_sysTypeName)) {
        if( propDel("child", 0, c) ) {
            m_children.empty();
            return true;
        }
    }
    return false;
}

udx sUsrFolder::attachCopy(sVec<sHiveId>* obj_ids, sUsrFolder * dst_obj, sStr * buf, sVec<sHiveId>* dst_obj_ids, bool isNcheckIfIsChild )
{
    if ( !dst_obj ) {
        return 0;
    }

    udx attached=0;
    sStr this_id_buf;
    IdStr(&this_id_buf);
    sVec<sHiveId> t_p,*p=obj_ids;
    sDic<bool> obj_dic;

    if( !p ) {
        isChild(sHiveId::zero);
        for( idx i = 0 ; i < m_children.dim() ; ++i ) {
            *t_p.add(1) = *static_cast<const sHiveId*>(m_children.id(i));
        }
        p=&t_p;
    }

    for( idx i = 0 ; i < p->dim() ; ++i ) {
        obj_dic.set(p->ptr(i),sizeof(*p->ptr(i)));
    }

    sHiveId copied_id;
    sHiveId * cp_id = &copied_id;

    for(idx i = 0 ; i < obj_dic.dim() ; ++i) {
        if(dst_obj_ids) {
            cp_id = dst_obj_ids->ptrx(i);
        }
        const sHiveId * obId = static_cast<const sHiveId*>(obj_dic.id(i));
        if(  !isNcheckIfIsChild && !isChild( *obId) ) {
            continue;
        }
        std::unique_ptr<sUsrObj> obj(m_usr.objFactory( *obId ));
        *cp_id = *obId;

        if( obj.get() && obj->Id() ) {
            if( obj->isTypeOf(s_myTypeName) || obj->isTypeOf(s_sysTypeName) ) {
                if( !((sUsrFolder *)obj.get())->isDescendant(dst_obj->Id()) ) {
                    sUsrFolder * c_obj = dst_obj->createSubFolder("%s", obj->propGet("name"));
                    if( c_obj ) {
                        ((sUsrFolder *)obj.get())->attachCopy(0, c_obj);
                        ++attached;
                        if( buf ) {
                            buf->printf("\"%s\" : { \"signal\" : \"copy\", \"data\" : { \"from\":\"%s\",\"to\":\"%s\"  } }\n,", obj->Id().print(), this_id_buf.ptr(), dst_obj->IdStr());
                        }
                    } else if( buf ) {
                        buf->printf("\"%s\" : { \"signal\" : \"copy\", \"data\" : { \"error\":\"cannot attach to %s\" } }\n,", obj->Id().print(), dst_obj->IdStr());
                    }
                    *cp_id = c_obj->Id();
                    delete c_obj;
                }
            } else {
                if( dst_obj->attach(*obj.get()) ) {
                    if( buf ) {
                        buf->printf("\"%s\" : { \"signal\" : \"copy\", \"data\" : { \"from\":\"%s\",\"to\":\"%s\"  } }\n,", obj->Id().print(), this_id_buf.ptr(), dst_obj->IdStr());
                    }
                    ++attached;
                } else if( buf ) {
                    buf->printf("\"%s\" : { \"signal\" : \"copy\", \"data\" : { \"error\":\"cannot attach to %s\" } }\n,", obj->Id().print(), dst_obj->IdStr());
                }
            }
        }
    }
    return attached;
}

udx sUsrFolder::attachMove(sVec<sHiveId>* obj_ids, sUsrFolder * dst_obj, sStr * buf, bool isNcheckIfIsChild)
{
    if( !dst_obj ) {
        return 0;
    }

    sStr this_id_buf;
    IdStr(&this_id_buf);

    udx attached = 0;

    sVec<sHiveId> t_p,*p=obj_ids;
    sDic<bool> obj_dic;

    if( !p ) {
        isChild(sHiveId::zero);
        for( idx i = 0 ; i < m_children.dim() ; ++i ) {
            *t_p.add(1) = *static_cast<const sHiveId*>(m_children.id(i));
        }
        p=&t_p;
    }

    for( idx i = 0 ; i < p->dim() ; ++i ) {
        obj_dic.set(p->ptr(i),sizeof(*p->ptr(i)));
    }
    for(idx i = 0 ; i < obj_dic.dim() ; ++i) {
        const sHiveId* obId = static_cast<const sHiveId*>(obj_dic.id(i));
        if( !isNcheckIfIsChild && !isChild(*obId) ) {
            continue;
        }
        std::unique_ptr<sUsrObj> obj(m_usr.objFactory( *obId ));
        if( obj.get() && obj->Id() ) {
            if( isNcheckIfIsChild || detach( *obj.get() ) ) {
                if( isNcheckIfIsChild && obj.get()->isTypeOf(s_myTypeName) ) {
                    sVec<sHiveId> toRemove;*toRemove.add()=obj.get()->Id();
                    if(attachCopy(&toRemove,dst_obj,0,0, true)){
                        actRemove(&toRemove,0, true,true);
                        ++attached;
                        if(buf)buf->printf("\"%s\" : { \"signal\" : \"move\", \"data\" : { \"from\":\"%s\",\"to\":\"%s\"  } }\n,",obj->Id().print(), this_id_buf.ptr(), dst_obj->IdStr() );
                    }
                    else{
                        if(buf)buf->printf("\"%s\" : { \"signal\" : \"move\", \"data\" : { \"error\":\"cannot attach to %s\"  } }\n,",obj->Id().print(), dst_obj->IdStr() );
                    }
                }
                else {
                    if( !dst_obj->attach( *obj.get() ) && !dst_obj->isChild(*obj.get()) ) {
                        if( isNcheckIfIsChild ) {
                            attach( *obj.get() );
                        }
                        if(buf)buf->printf("\"%s\" : { \"signal\" : \"move\", \"data\" : { \"error\":\"cannot attach to %s\"  } }\n,",obj->Id().print(), dst_obj->IdStr() );
                    }
                    else {
                        ++attached;
                        if(buf)buf->printf("\"%s\" : { \"signal\" : \"move\", \"data\" : { \"from\":\"%s\",\"to\":\"%s\"  } }\n,",obj->Id().print(), this_id_buf.ptr(), dst_obj->IdStr() );
                    }
                }
            }
            else{
                if(buf)buf->printf("\"%s\" : { \"signal\" : \"move\", \"data\" : { \"error\":\"cannot detach from folder %s\"  } }\n,",obj->Id().print(), this_id_buf.ptr() );
            }
        }
    }

    return attached;
}

bool sUsrFolder::isSubFolder(const char * value) const
{
    isChild(sHiveId::zero);
    for(idx r = 0; r < m_children.dim(); ++r) {
        sHiveId id(*static_cast<const sHiveId*>(m_children.id(r)));
        sUsrObj * c = m_usr.objFactory(id);
        if( c && c->Id() && (c->isTypeOf(s_myTypeName) || c->isTypeOf(s_sysTypeName) ) ) {
            const char* c_value = c->propGet("child");
            if( c && strcmp(value, c_value)==0 ) {
                return true;
            }
        }
        delete c;
    }
    return false;
}

bool sUsrFolder::isChild(const sUsrObj & obj) const
{
    return isChild(obj.Id());
}

bool sUsrFolder::isChild(const sHiveId & id) const
{
    if( !m_children.dim() ) {
        sVarSet ch;
        propGet("child", ch);
        udx c_group = 0;
        for(idx r = 0; r < ch.rows; ++r) {
            sHiveId r_id(ch.uval(r, 0), 0);
            udx group = ch.uval(r, 1, ++c_group);
            *m_children.set(&r_id, sizeof(sHiveId)) = group;
        }
    }
    return m_children.get(&id, sizeof(sHiveId));
}

bool sUsrFolder::isDescendant(const sHiveId & id) const
{
    if( !m_children.dim() ) {
        isChild(sHiveId::zero);
    }
    bool isDes=m_children.get(&id, sizeof(sHiveId));

    for(idx r = 0; !isDes && r < m_children.dim(); ++r) {
        sHiveId c(*static_cast<const sHiveId*>(m_children.id(r)));
        std::unique_ptr<sUsrObj> obj(m_usr.objFactory(c));
        if( obj.get() && obj->Id() && obj->isTypeOf(s_myTypeName) ) {
            isDes = ((sUsrFolder *)obj.get())->isDescendant(id);
        }
    }
    return isDes;
}

bool sUsrFolder::isDescendant(const sUsrObj & obj) const
{
    return isDescendant(obj.Id());
}

bool sUsrFolder::isInTrash() const
{
    sUsrFolder * sys_trash = sSysFolder::Trash(m_usr);
    bool ret = sys_trash->isDescendant(this->Id());
    delete sys_trash;
    return ret;
}

bool sUsrFolder::isTrash() const
{
    sUsrFolder * sys_trash = sSysFolder::Trash(m_usr);
    bool ret =  sys_trash->Id()==this->Id();
    delete sys_trash;
    return ret;
}

bool sUsrFolder::actRemove(sVec<sHiveId>* obj_ids, sStr * buf, bool isNcheckIfIsChild, bool forceDelete )
{
    sVec<sHiveId> t_p, *p = obj_ids;
    sDic<bool> obj_dic;

    sStr this_id_buf;
    IdStr(&this_id_buf);
    if( !p ) {
        isChild(sHiveId::zero);
        for( idx i = 0; i < m_children.dim(); ++i ) {
            *t_p.add(1) = *static_cast<const sHiveId *>(m_children.id(i));
        }
        p = &t_p;
    }
    for( idx i = 0; i < p->dim(); ++i ) {
        obj_dic.set(p->ptr(i),sizeof(*p->ptr(i)));
    }
    sVec<sHiveId> set, orphs;
    sDic<bool> d_orph;
    sUsrFolder::orphans(m_usr, orphs, "");
    for( idx i = 0; i < orphs.dim(); ++i ) {
        d_orph.set(orphs.ptr(i), sizeof(sHiveId));
    }
    sUsrFolder * _trash= sSysFolder::Trash(m_usr);
    bool isintrash = isInTrash() || isTrash();
    for(idx i = 0 ; i < obj_dic.dim() ; ++i) {
        const sHiveId *obId = static_cast<const sHiveId*>(obj_dic.id(i));
        if( !isNcheckIfIsChild && !d_orph.get(obId, sizeof(*obId)) && !isChild(*obId) ) {
            continue;
        }
        std::unique_ptr<sUsrObj> obj(m_usr.objFactory( *obId ));
        const char * objIdStr = obj->IdStr();
        bool isSys = obj->isTypeOf(s_sysTypeName);
        if( obj.get() && obj->Id() ) {
            if( !isintrash && !forceDelete ) {
                if( detach(*obj.get()) && !isNcheckIfIsChild ) {
                    if( _trash->attach(*obj.get()) ) {
                        if( buf ) {
                            buf->printf("\"%s\" : { \"signal\" : \"trash\",\"data\" : { \"from\" : \"%s\"} }\n,", obj->Id().print(), this_id_buf.ptr());
                        }
                    } else if( buf ) {
                        buf->printf("\"%s\" : { \"signal\" : \"trash\", \"data\" : { \"error\":\"cannot attach to Trash\"  } }\n,", obj->Id().print());
                    }
                } else if( buf ) {
                    buf->printf("\"%s\" : { \"signal\" : \"trash\", \"data\" : { \"error\":\"cannot detach from folder %s\"  } }\n,", obj->Id().print(), this_id_buf.ptr());
                }
            } else {
                sUsrObjRes parents;
                sStr foldstypes("%s,%s", s_sysTypeName, s_myTypeName);
                m_usr.objs2(foldstypes.ptr(), parents, 0, "child", obj->IdStr());
                bool doDetach = true;
                if( parents.dim() == 1 || forceDelete ) {
                    if( !obj->actDelete() ) {
                        doDetach = false;
                        if( buf ) {
                            buf->printf("\"%s\" : { \"signal\" : \"delete\", \"data\" : { \"error\":\"You cannot PERMANENTLY delete this object\"  } }\n,", obj->Id().print());
                        }
                    }
                }
                if( doDetach && !isNcheckIfIsChild && !isSys ) {
                    if( propDel("child", 0, objIdStr) ) {
                        if( buf )
                            buf->printf("\"%s\" : { \"signal\" : \"delete\",\"data\":{\"info\":\"unlinked\"} }\n,", obj->Id().print());
                    } else if( buf ) {
                        buf->printf("\"%s\" : { \"signal\" : \"delete\", \"data\" : { \"warning\":\"cannot remove link from folder\"  }  }\n,", obj->Id().print());
                    }
                }
            }
        }
        if( progress_CallbackFunction && !progress_CallbackFunction(progress_CallbackParamObject, *(idx *)progress_CallbackParamReqID, 2, i, i, obj_dic.dim()) ) {
            delete _trash;
            return 0;
        }
    }
    delete _trash;
    return true;
}

idx sUsrFolder::orphans(const sUsr & usr, sVec<sHiveId> & ids, const char * type_names, const char* prop, const char* value)
{
    sUsrObjRes all, parents;
    usr.objs2(type_names, all, 0, prop, value, "created");
    if( all.dim() ) {
        sStr typeNames("%s,%s",s_myTypeName,s_sysTypeName);
        usr.objs2(typeNames, parents, 0, 0, 0, "child");
        sDic<sHiveId> children;
        for(sUsrObjRes::IdIter it = parents.first(); parents.has(it); parents.next(it)) {
            const sUsrObjRes::TObjProp * obj =  parents.get(it);
            const sUsrObjRes::TPropTbl * tbl = parents.get(*obj, "child");
            while( tbl ) {
                sHiveId c(parents.getValue(tbl));
                if( c ) {
                    children.set(&c, sizeof(c));
                }
                tbl = parents.getNext(tbl);
            }
        }
        for(sUsrObjRes::IdIter it = all.first(); all.has(it); all.next(it)) {
            const sHiveId * id = all.id(it);
            if( id && !children.get(id, sizeof(*id)) ) {
                std::unique_ptr<sUsrObj> o(usr.objFactory(*all.id(it)));
                if( o.get() && o->isTypeOf(s_sysTypeName) ) {
                    continue;
                }
                sHiveId * p = ids.add();
                if( p ) {
                    *p = *all.id(it);
                }
            }
        }
    }
    return ids.dim();
}

idx sUsrFolder::attachedTo(sVec<sHiveId> * out_folders, const sUsr & usr, const sHiveId & id)
{
    sUsrObjRes parents;
    sStr buf;
    static const idx typeNames_pos = 0;
    buf.printf(0, "%s,%s", s_myTypeName, s_sysTypeName);
    buf.add0(2);
    idx id_pos = buf.length();
    buf.add("^", 1);
    id.print(buf);
    buf.add("$", 1);
    buf.add0(2);
    usr.objs2(buf.ptr(typeNames_pos), parents, 0, "child", buf.ptr(id_pos), "child");
    if( out_folders ) {
        for(sUsrObjRes::IdIter it = parents.first(); parents.has(it); parents.next(it)) {
            *out_folders->add(1) = *parents.id(it);
        }
    }
    return parents.dim();
}

udx sUsrFolder::propSet(const char* prop, const char** groups, const char** values, udx cntValues, bool isAppend, const udx * path_lens, const udx * value_lens)
{
    if( strcmp(prop, "name") == 0 && isReserved() ) {
        return 0;
    }
    return TParent::propSet(prop, groups, values, cntValues, isAppend, path_lens, value_lens);
}

udx sUsrFolder::propGet(const char* prop, sVarSet& res, bool sort, bool allowSysInternal) const
{
    if( strcasecmp(prop, "type_count") == 0 ) {
        sDic<udx> list;
        sDic<sHiveId> kids;
        list.mex()->flags |= sMex::fSetZero;
        sVarSet children;
        propGet("child", children, false);
        for(idx i = 0; i < children.rows; ++i) {
            sHiveId id(children.val(i, 0));
            if( id ) {
                kids.set(&id, sizeof(id));
            }
        }
        if( kids.dim() ) {
            sVec<sHiveId> in, out;
            in.resize(kids.dim());
            for(idx i = 0; i < kids.dim(); ++i) {
                in[i] = *((sHiveId*)kids.id(i));
            }
            m_usr.objs(in, out);
            for(idx i = 0; i < out.dim(); ++i) {
                sHiveId id(out[i]);
                sUsrObj * c = new sUsrObj(m_usr, id);
                if( c && c->Id() ) {
                    if( const char * tnm = c->getTypeName() ) {
                        udx * q = list.set(tnm, sLen(tnm) + 1);
                        if( q ) {
                            *q = *q + 1;
                        }
                    }
                }
                delete c;
            }
            for(idx i = 0; i < list.dim(); ++i) {
                res.addRow().addCol(*list.ptr(i)).addCol((const char*) list.id(i));
            }
        }
        return res.rows;
    }
    return TParent::propGet(prop, res, sort, allowSysInternal);
}

udx sUsrFolder::name(const char* name)
{

    udx homonyms = 0;
    sStr obj_name;
    obj_name.printf("%s",name);

    if( isSubFolder( obj_name ) ) {
        obj_name.printf(" - Copy");
        ++homonyms;
    }
    idx buflen = obj_name.length();
    while( isSubFolder( obj_name ) ) {
        obj_name.cut(buflen);
        obj_name.printf(" (%" UDEC ")", ++homonyms);
    }

    return TParent::propSet("name", obj_name);
}

bool sUsrFolder::onDelete(void)
{
    if( TParent::onDelete() && !this->isTypeOf(s_sysTypeName) ) {
        sVarSet children;
        propGet("child", children);
        TParent::propSet("child", (char*) 0);
        for(idx r = 0; r < children.rows; ++r) {
            sHiveId child(children.val(r, 0));
            std::unique_ptr<sUsrObj> o(m_usr.objFactory(child));
            if( o.get() && o->Id() ) {
                if( o->isTypeOf(s_myTypeName) ) {
                    o->actDelete();
                } else {
                    sUsrObjRes parents;
                    m_usr.objs2(s_myTypeName, parents, 0, "child", o->IdStr());
                    if( parents.dim() == 0 ) {
                        o->actDelete();
                    }

                }
            }
        }
        return true;
    }
    return false;
}

bool sUsrFolder::fixChildrenPath(void)
{
    sVarSet ch;
    propGet("child", ch);
    udx max_group = 0;
    sDic<udx> grps;
    bool doFix = false;

    for(idx r = 0; r < ch.rows; ++r) {

        udx t_group = ch.uval(r,1, 0);

        if(!t_group) {
            doFix = true;
            continue;
        }
        udx * grpCnt = grps.get(&t_group, sizeof(t_group));
        if( grpCnt ) {
            doFix = true;
            (*grpCnt)++;
            continue;
        }
        if( max_group < t_group ) {
            max_group = t_group;
        }
        *grps.set(&t_group, sizeof(t_group)) = 1;
    }


    if( !doFix ) {
        return false;
    }

    sVec<const char *> t_ids;
    sVec<char *> t_grps;
    sStr strbuf;
    char * t_ptr = 0;

    t_ids.add(ch.rows);
    t_grps.add(ch.rows);
    t_grps.set(0);

    for(idx r = 0; r < ch.rows; ++r) {
        t_ids[r] = ch.val(r,0);
        idx t_group = ch.uval(r,1, 0);
        udx * grpCnt = grps.get(&t_group, sizeof(t_group));


        if( t_group &&  (*grpCnt) == 1 ) {
            t_ptr = strbuf.printf("%s", ch.val(r,1,0));
        }
        else {
            t_ptr = strbuf.printf("%" UDEC,++max_group);

            if( grpCnt && (*grpCnt) > 1 ) {
                --(*grpCnt);
            }
        }

        if( r ) {
            if (r+1 < ch.rows ) {
                t_grps[r+1] = t_grps[r] + sLen(t_ptr) + 1;
            }
        }
        else {
            t_grps[r] = 0;
            t_grps[r+1] = t_grps[r] + sLen(t_ptr) + 1;
        }

        strbuf.add0();

    }
    for(idx r = 0 ; r < ch.rows; ++r) {
        t_grps[r] += (idx)strbuf.ptr(0);
    }

    strbuf.add0();

    TParent::propSet("child", (const char **)&(t_grps[0]), &(t_ids[0]), t_ids.dim(), false );
    return true;
}

bool sSysFolder::sysfolder(sHiveId & out_id, const sUsr& usr, const char * path, bool doCreate)
{
    return folder(out_id, usr, path, doCreate, 0, s_sysTypeName);
}

sSysFolder::sSysFolder(const sUsr& usr, const char * path)
    : sUsrFolder(usr, sHiveId::zero)
{
    sysfolder(m_id, usr, path, true);
    if( !m_usr.objGet(m_id) ) {
        m_id.reset();
    }
}

sUsrFolder * sSysFolder::find(const sUsr & usr, const char * path)
{
    sHiveId id;
    sysfolder(id, usr, path, false);
    return (sUsrFolder*) (usr.objFactory(id));
}

sUsrFolder * sSysFolder::Home(const sUsr & usr, bool autoCreate)
{
    sHiveId id;
    sysfolder(id, usr, s_Home, autoCreate);
    return (sUsrFolder*) (usr.objFactory(id));
}

sUsrFolder * sSysFolder::Inbox(const sUsr & usr, bool autoCreate)
{
    sHiveId id;
    sysfolder(id, usr, s_Inbox, autoCreate);
    return (sUsrFolder*) (usr.objFactory(id));
}

sUsrFolder * sSysFolder::Trash(const sUsr & usr, bool autoCreate)
{
    sHiveId id;
    sysfolder(id, usr, s_Trash, autoCreate);
    return (sUsrFolder*) (usr.objFactory(id));
}

sUsrFolder * sSysFolder::SharedWithMe(const sUsr & usr, bool autoCreate)
{
    sHiveId id;
    sysfolder(id, usr, s_SharedWithMe, autoCreate);
    return (sUsrFolder*) (usr.objFactory(id));
}

udx sSysFolder::propGet(const char* prop, sVarSet& res, bool sort, bool allowSysInternal) const
{
    if( strcasecmp(prop, "type_count") == 0 ) {
        sUsrFolder * shared = SharedWithMe(m_usr, false);
        if( shared && shared->Id() == Id() ) {
            sStr lst;
            sUsrObjRes all, parents;
            sStr gid("%" UDEC, m_usr.groupId());
            m_usr.objs2(0, all, 0, "!_creator", gid.ptr(), "created", true, 0, 0, allowSysInternal);
            if( all.dim() ) {
                for(sUsrObjRes::IdIter it = all.first(); all.has(it); all.next(it)) {
                    lst.printf("|");
                    all.id(it)->print(lst);
                }
                sStr typeNames("%s,%s",s_myTypeName,s_sysTypeName);
                m_usr.objs2(typeNames, parents, 0, "child", lst.ptr(1), "child");
                sDic<sHiveId> children;
                for(sUsrObjRes::IdIter it = parents.first(); parents.has(it); parents.next(it)) {
                    const sUsrObjRes::TObjProp * obj =  parents.get(it);
                    const sUsrObjRes::TPropTbl * tbl = parents.get(*obj, "child");
                    while( tbl ) {
                        sHiveId c(parents.getValue(tbl));
                        if( c ) {
                            children.set(&c, sizeof(c));
                        }
                        tbl = parents.getNext(tbl);
                    }
                }
                sDic<udx> list;
                list.mex()->flags |= sMex::fSetZero;
                for(sUsrObjRes::IdIter it = all.first(); all.has(it); all.next(it)) {
                    const sHiveId * id = all.id(it);
                    if( id && !children.get(id, sizeof(*id)) ) {
                        sUsrObj * o = m_usr.objFactory(*all.id(it));
                        if( o ) {
                            if( !o->isTypeOf(s_sysTypeName) ) {
                                if( const char * tnm = o->getTypeName() ) {
                                    udx * q = list.setString(tnm);
                                    if( q ) {
                                        *q = *q + 1;
                                    }
                                }
                            }
                            delete o;
                        }
                    }
                }
                for(idx i = 0; i < list.dim(); ++i) {
                    res.addRow().addCol(*list.ptr(i)).addCol((const char*) list.id(i));
                }

            }
            delete shared;
            return res.rows;
        }
        delete shared;
    } else if( strcasecmp(prop, "child") == 0 ) {
        sUsrFolder * shared = SharedWithMe(m_usr, false);
        if( shared && shared->Id() == Id() ) {
            sStr lst;
            sUsrObjRes all, parents;
            sStr gid("%" UDEC, m_usr.groupId());
            m_usr.objs2(0, all, 0, "!_creator", gid.ptr(), "created", true, 0, 0, allowSysInternal);
            if( all.dim() ) {
                for(sUsrObjRes::IdIter it = all.first(); all.has(it); all.next(it)) {
                    lst.printf("|");
                    all.id(it)->print(lst);
                }
                sStr typeNames("%s,%s",s_myTypeName,s_sysTypeName);
                m_usr.objs2(typeNames, parents, 0, "child", lst.ptr(1), "child");
                sDic<sHiveId> children;
                for(sUsrObjRes::IdIter it = parents.first(); parents.has(it); parents.next(it)) {
                    const sUsrObjRes::TObjProp * obj =  parents.get(it);
                    const sUsrObjRes::TPropTbl * tbl = parents.get(*obj, "child");
                    while( tbl ) {
                        sHiveId c(parents.getValue(tbl));
                        if( c ) {
                            children.set(&c, sizeof(c));
                        }
                        tbl = parents.getNext(tbl);
                    }
                }
                idx i = 1;
                for(sUsrObjRes::IdIter it = all.first(); all.has(it); all.next(it), ++i) {
                    const sHiveId * id = all.id(it);
                    if( id && !children.get(id, sizeof(*id)) ) {
                        typeNames.printf(0, "1.%" DEC, i);
                        res.addRow().addCol(id->print()).addCol(typeNames.ptr());
                    }
                }
            }
            delete shared;
            return res.rows;
        }
        delete shared;
    }
    return TParent::propGet(prop, res, sort, allowSysInternal);
}
