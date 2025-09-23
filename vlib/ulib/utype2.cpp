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

#include <slib/core/net.hpp>
#include <slib/std/file.hpp>
#include <slib/utils/sort.hpp>
#include <slib/utils/tbl.hpp>
#include <qlang/engine.hpp>
#include <qlib/QPrideBase.hpp>
#include <ulib/utype2.hpp>
#include <ulib/uobj.hpp>
#include <ulib/upropset.hpp>

#include "utype2_loading.hpp"
#include "utype2_cache.hpp"

#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <stdlib.h>

using namespace slib;
static const char * canonicalCase(slib::sStr & buf, const char * str, idx len = 0)
    {
        if( !str ) {
            return slib::sStr::zero;
        }
        if( !len ) {
            len = slib::sLen(str);
        }
        for(idx i=0; i<len; i++) {
            if( str[i] >= 'A' && str[i] <= 'Z' ) {
                buf.cut0cut();
                slib::sString::changeCase(&buf, str, 0, slib::sString::eCaseLo);
                return buf.ptr(0);
            }
        }
        return str;
    }

    static idx strncmp_exact(const char * s1, idx len1, const char * s2, idx len2)
    {
        if( !len1 ) {
            len1 = slib::sLen(s1);
        }
        if( !len2 ) {
            len2 = slib::sLen(s2);
        }
        if( int d = strncmp(s1, s2, slib::sMin<idx>(len1, len2)) ) {
            return d;
        }
        return len1 - len2;
    }


static sStr utype_errors;

static bool useJSONCache(bool change_setting = false, bool new_setting = false)
{
    static bool use_json_cache = false;
    static bool first_call = true;

    if( first_call ) {
        if( const char * s = getenv("TYPE_JSON_CACHE") ) {
            use_json_cache = sString::parseBool(s);
        }
        first_call = false;
    }

    if( change_setting ) {
        use_json_cache = new_setting;
    }

    return use_json_cache;
}

static bool useBinCache(bool change_setting = false, bool new_setting = false)
{
    static bool use_bin_cache = true;
    static bool first_call = true;

    if( first_call ) {
        if( const char * s = getenv("TYPE_BIN_CACHE") ) {
            use_bin_cache = sString::parseBool(s);
        }
        first_call = false;
    }

    if( change_setting ) {
        use_bin_cache = new_setting;
    }

    return use_bin_cache;
}


const udx sUsrType2::type_type_domain_id = sHiveId::encodeDomainId("type");
sStr sUsrLoadingType::_name_buf;
sMex sUsrLoadingType::_deps_buf;
sUsrLoadingType::sUsrLoadingTypeList sUsrLoadingType::_types;
sDic<idx> sUsrLoadingType::_name_or_id2itype;
sDic<idx> sUsrLoadingType::_name_or_id2ephemeral_itype;
sVec<sUsrLoadingType*> sUsrLoadingType::_roots;
bool sUsrLoadingType::_all_deps_loaded = false;
bool sUsrLoadingType::_all_prefetched = false;

sDic<sUsrLoadingAction::ActRes> sUsrLoadingAction::_usr2actres;
sDic<sUsrLoadingJSComponent::JscoRes> sUsrLoadingJSComponent::_usr2jscores;
sMex sUsrLoadingView::_fields_buf;
sDic<sUsrLoadingView::ViewRes> sUsrLoadingView::_usr2viewres;
sDic<bool> sUsrLoadingView::_names;

sStr sUsrLoadingTypeField::_name_buf;

static const char * field_type_names[] = {
    "string",
    "text",
    "integer",
    "real",
    "bool",
    "array",
    "list",
    "date",
    "time",
    "datetime",
    "url",
    "obj",
    "password",
    "file",

    "integer",
    "string",
    "string",
    "string",
    "string",
    "string",
    "listtab",
    "arraytab"
};

static const char * field_type_names2[] = {
    "string",
    "text",
    "int",
    "real",
    "bool",
    "array",
    "list",
    "date",
    "time",
    "datetime",
    "url",
    "hiveid",
    "password",
    "file",
    "uint",
    "memory",
    "version",
    "blob",
    "json",
    "xml",
    "arraytab",
    "listtab"
};

static const char * field_role_names00[] = {
    "input" _ "in" __,
    "output" _ "out" __,
    "parameter" __,
    "state" __
};

inline static const sUsrTypeField::EType fieldTypeFromName(const char * name)
{
    for(idx i=0; i<sDim(field_type_names); i++) {
        if( sIsExactly(name, field_type_names[i]) ) {
            return (sUsrTypeField::EType)i;
        }
    }
    return sUsrTypeField::eInvalid;
}

inline static const sUsrTypeField::EType fieldTypeFromName2(const char * name)
{
    for(idx i=0; i<sDim(field_type_names2); i++) {
        if( sIsExactly(name, field_type_names2[i]) ) {
            return (sUsrTypeField::EType)i;
        }
    }
    return sUsrTypeField::eInvalid;
}

inline static const sUsrTypeField::ERole fieldRoleFromName(const char * name)
{
    for(idx i=0; i<sDim(field_role_names00); i++) {
        for(const char * s = field_role_names00[i]; s && *s; s = sString::next00(s)) {
            if( sIsExactly(name, s) ) {
                return (sUsrTypeField::ERole)i;
            }
        }
    }
    return sUsrTypeField::eRole_unknown;
}

static const char * fieldTypeToName(sUsrTypeField::EType t)
{
    if( t >= 0 && t < sDim(field_type_names)) {
        return field_type_names[(int)t];
    }
    return "invalid";
}

static const char * fieldTypeToName2(sUsrTypeField::EType t)
{
    if( t >= 0 && t < sDim(field_type_names2)) {
        return field_type_names2[(int)t];
    }
    return "invalid";
}

static const char * fieldRoleToName(sUsrTypeField::ERole r)
{
    if( r >= 0 && r < sDim(field_role_names00)) {
        return field_role_names00[(int)r];
    }
    return 0;
}


sUsrLoadingType::sUsrLoadingType(bool default_zero)
{
    _itype = _pos_name = _pos_title = _pos_description = -1;
    _is_virtual = default_zero ? false : true;
    _is_user = eLazyNotLoaded;
    _is_system = eLazyNotLoaded;
    _is_prefetch = false;
    _is_fetched = false;
    _is_singleton = eNotSingleton;
    _is_broken = false;
    _is_ephemeral = false;

    _created = _modified = 0;

    _parents.init(&_deps_buf);
    _children.init(&_deps_buf);
    _includes.init(&_deps_buf);

    _dim_explicit_fields = _dim_inherited_fields = _dim_included_fields = 0;
}

sUsrLoadingType * sUsrLoadingType::getRaw(const char * type_name, idx type_name_len)
{
    sStr case_buf;
    if( !type_name ) {
        return 0;
    }
    if( !type_name_len ) {
        type_name_len = sLen(type_name);
    }
    idx * pitype = _name_or_id2itype.get(canonicalCase(case_buf, type_name, type_name_len), type_name_len);
    return pitype ? _types[*pitype] : 0;
}

sUsrLoadingType * sUsrLoadingType::getRaw(const sHiveId & type_id)
{
    idx * pitype = _name_or_id2itype.get(&type_id, sizeof(sHiveId));
    return pitype ? _types[*pitype] : 0;
}

sUsrLoadingType * sUsrLoadingType::getRawEphemeral(const sHiveId & type_id)
{
    idx * pitype = _name_or_id2ephemeral_itype.get(&type_id, sizeof(sHiveId));
    return pitype ? _types[*pitype] : 0;
}

namespace {
    struct DepList {
        struct Entry {
            sHiveId type_id;
            sHiveId dep_id;
            idx path_index;
            idx ifld;
            enum EKind {
                eParent,
                eChild,
                eInclude
            } kind;

            const char * print(sStr & buf, bool capital, bool reverse) const
            {
                idx pos = buf.length();
                buf.addString(capital ? "Type \"" : "type \"");
                if( reverse ) {
                    dep_id.print(buf);
                } else {
                    type_id.print(buf);
                }
                buf.addString("\" ");
                switch(kind) {
                    case eParent:
                        buf.addString(reverse ? "has child " : "has parent ");
                        break;
                    case eChild:
                        buf.addString(reverse ? "has parent " : "has child ");
                        break;
                    case eInclude:
                        buf.addString(reverse ? "is included by a field of " : "has a field which includes ");
                        break;
                }
                buf.addString("type \"");
                if( reverse ) {
                    type_id.print(buf);
                } else {
                    dep_id.print(buf);
                }
                buf.addString("\"");
                return buf.ptr(pos);
            }
        };
        sVec<Entry> vec;
        sVec<idx> sort_order;
        sDic<bool> interned_strings;
        idx icur;

        void cut()
        {
            vec.cut();
            sort_order.cut();
            interned_strings.empty();
            icur = 0;
        }

        void pushParent(sHiveId type_id, sHiveId parent_id, const char * path)
        {
            Entry * ep = vec.add(1);
            ep->type_id = type_id;
            ep->dep_id = parent_id;
            ep->kind = Entry::eParent;
            ep->ifld = -sIdxMax;
            if( !path ) {
                path = "";
            }
            interned_strings.setString(path, 0, &ep->path_index);

            Entry * ec = vec.add(1);
            ec->type_id = parent_id;
            ec->dep_id = type_id;
            ec->kind = Entry::eChild;
            ec->path_index = -sIdxMax;
            ec->ifld = -sIdxMax;
        }

        void pushInclude(sHiveId type_id, sHiveId include_id, const char * path, idx ifld)
        {
            Entry * ed = vec.add(1);
            ed->type_id = type_id;
            ed->dep_id = include_id;
            ed->kind = Entry::eInclude;
            ed->ifld = ifld;
            if( !path ) {
                path = "";
            }
            interned_strings.setString(path, 0, &ed->path_index);
        }

        static idx sort_cb(void * param, void * arr_param, idx i1, idx i2)
        {
            DepList * self = static_cast<DepList *>(param);
            idx * arr = static_cast<idx*>(arr_param);
            Entry & e1 = self->vec[arr[i1]];
            Entry & e2 = self->vec[arr[i2]];

            if( idx diff = e1.type_id.cmp(e2.type_id) ) {
                return diff;
            }

            if( idx diff = (idx)(e1.kind) - (idx)(e2.kind) ) {
                return diff;
            }

            if( e1.kind == Entry::eParent && e2.kind == Entry::eParent ) {
                const char * path1 = static_cast<const char *>(self->interned_strings.id(e1.path_index));
                const char * path2 = static_cast<const char *>(self->interned_strings.id(e2.path_index));
                do {
                    idx cur_elt1 = sUsrObj::readPathElt(path1, &path1);
                    idx cur_elt2 = sUsrObj::readPathElt(path2, &path2);
                    if( cur_elt1 != cur_elt2 ) {
                        return cur_elt1 - cur_elt2;
                    }
                } while( path1 && path2 );
            }

            return e1.dep_id.cmp(e2.dep_id);
        }

        void sort()
        {
            sort_order.resize(vec.dim());
            for(idx i = 0; i < sort_order.dim(); i++) {
                sort_order[i] = i;
            }
            sSort::sortSimpleCallback<idx>(sort_cb, this, sort_order.dim(), sort_order.ptr());
            icur = 0;
        }

        idx dim() const
        {
            return vec.dim();
        }

        Entry & getSorted(idx i)
        {
            return vec[sort_order[i]];
        }
    };

    struct FieldPathDic {
        sDic<idx> dic;
        sStr buf;

        void makeBuf(const sHiveId & type_id, const char * prop_group)
        {
            buf.cut(0);
            type_id.print(buf);
            buf.add(0);
            const char * dot = strchr(prop_group, '.');
            if( dot ) {
                dot = strchr(dot + 1, '.');
            }
            if( dot ) {
                buf.addString(prop_group, dot - prop_group);
            } else {
                buf.addString(prop_group);
            }
        }

        bool get(const sHiveId & type_id)
        {
            return dic.get(&type_id, sizeof(sHiveId));
        }

        idx get(const sHiveId & type_id, const char * prop_group)
        {
            makeBuf(type_id, prop_group);
            idx * pifld = dic.get(buf.ptr(), buf.length());
            return pifld ? *pifld : -sIdxMax;
        }

        void set(const sHiveId & type_id, const char * prop_group, idx ifld)
        {
            *dic.set(&type_id, sizeof(sHiveId)) = -sIdxMax;
            makeBuf(type_id, prop_group);
            *dic.setString(buf.ptr(), buf.length(), 0) = ifld;
        }

        void cut()
        {
            dic.empty();
            buf.cut(0);
        }
    };
};

idx sUsrLoadingType::DepForest::addWorker(const sUsrLoadingType * utype)
{
    if( utype->_is_fetched ) {
        return -1;
    }
    if( _itype2node[utype->_itype] >= 0 ) {
        return _itype2node[utype->_itype];
    }
    LOG_TRACE("adding \"%s\" (%s) to forest", utype->name(), utype->id().print());
    idx inode = _itype2node[utype->_itype] = _nodes.dim();
    sKnot<udx> * node = _nodes.add(1);
    node->obj = utype->_itype;
    node->in.init(&_mex);
    node->out.init(&_mex);
    node = 0;

    for(idx ip=0; ip<utype->dimParents(); ip++) {
        if( const sUsrLoadingType * par = dynamic_cast<const sUsrLoadingType*>(utype->getParent(ip)) ) {
            idx ipar_node = addWorker(par);
            if( ipar_node >= 0 ) {
                *_nodes[inode].in.add(1) = ipar_node;
                *_nodes[ipar_node].out.add(1) = inode;
            }
        }
    }
    for(idx iinc=0; iinc<utype->dimIncludes(); iinc++) {
        if( const sUsrLoadingType * inc = dynamic_cast<const sUsrLoadingType*>(utype->getInclude(iinc)) ) {
            idx iinc_node = addWorker(inc);
            if( iinc_node >= 0 ) {
                *_nodes[inode].in.add(1) = iinc_node;
                *_nodes[iinc_node].out.add(1) = inode;
            }
        }
    }
    return inode;
}

idx sUsrLoadingType::DepForest::makeTraversal(sVec<idx> & out)
{
    idx start_dim = out.dim();
    sVec<bool> visited(sMex::fSetZero);
    visited.resize(_nodes.dim());
    for(idx inode=0; inode<_nodes.dim(); inode++) {
        makeTreeTraversal(out, inode, visited);
    }
    return out.dim() - start_dim;
}

void sUsrLoadingType::DepForest::makeTreeTraversal(sVec<idx> & out, idx inode, sVec<bool> & visited)
{
    if( visited[inode] ) {
        return;
    }
    visited[inode] = true;
    sKnot<udx> & node = _nodes[inode];
    for(idx iin=0; iin < node.in.dim(); iin++) {
        LOG_TRACE("Traversing parent %" DEC "/%" DEC " of type \"%s\": \"%s\"", iin, node.in.dim(), sUsrLoadingType::_types[node.obj]->name(), sUsrLoadingType::_types[_nodes[*node.in.ptr(iin)].obj]->name());
        makeTreeTraversal(out, *node.in.ptr(iin), visited);
    }

    LOG_TRACE("Adding type \"%s\" to traversal", sUsrLoadingType::_types[node.obj]->name());
    *out.add(1) = node.obj;
}

static bool isFieldWeakReference(const char * type_name, const char * field_name)
{
    if( !type_name ) {
        type_name = sStr::zero;
    }
    if( !field_name ) {
        field_name = sStr::zero;
    }

    if( sIsExactly(type_name, "process") && sIsExactly(field_name, "folder") ) {
        return true;
    }
    return false;
}

#define INCLUDE_TYPE_EXPECTED -999
class sUsrLoadingType::LoadFromObjContext {
    public:
        const sUsr & user;
        sUsrLoadingType * utype;
        bool is_full_fetch;
        DepList dep_list;
        FieldPathDic field_path_dic;
        sDic<idx> full_fetch_ids2itype;
        sStr case_buf;

        LoadFromObjContext(const sUsr & u): user(u)
        {
            utype = 0;
            is_full_fetch = false;
        }

        void resetType()
        {
            utype = 0;
            is_full_fetch = false;
            field_path_dic.cut();
            case_buf.cut0cut();
        }

        bool accumulateProp(const char * prop_name, const char * prop_value, const char * prop_group)
        {
            if( sIsExactly(prop_name, "name") ) {
                 if( isalpha(prop_value[0]) ) {
                     const char * canon_value = canonicalCase(case_buf, prop_value);
                     if( idx * pitype = _name_or_id2itype.get(canon_value) ) {
                         sUsrLoadingType * other_utype = _types[*pitype];
                         if( utype->_id != other_utype->_id ) {
                             LOG_ERROR("Types \"%s\" and \"%s\" have the same name \"%s\"", utype->_id.print(), other_utype->_id.print(local_log_buf), prop_value);
                         }
                     } else {
                         *_name_or_id2itype.setString(canon_value) = utype->_itype;
                     }
                     if( utype->_pos_name < 0 ) {
                         utype->_pos_name = _name_buf.length();
                         _name_buf.addString(prop_value);
                         _name_buf.add0();
                     } else if( !sIsExactly(utype->name(), prop_value) ) {
                         LOG_ERROR("Type \"%s\" found with 2 different names: '%s' and '%s'", utype->_id.print(), utype->name(), prop_value);
                         utype->_is_broken = true;
                     }
                 } else {
                     LOG_ERROR("Type \"%s\" has invalid name \"%s\"", utype->_id.print(), prop_value);
                     utype->_is_broken = true;
                     return false;
                 }
             } else if( sIsExactly(prop_name, "created") ) {
                 struct tm unused;
                 utype->_created = sString::parseDateTime(&unused, prop_value);
             } else if( sIsExactly(prop_name, "modified") ) {
                 struct tm unused;
                 utype->_modified = sString::parseDateTime(&unused, prop_value);
             } else if( sIsExactly(prop_name, "title") ) {
                 if( utype->_pos_title < 0 ) {
                     utype->_pos_title = _name_buf.length();
                     _name_buf.addString(prop_value);
                     _name_buf.add0();
                 } else if( !sIsExactly(utype->title(), prop_value) ) {
                     LOG_ERROR("Type \"%s\" found with 2 different titles: '%s' and '%s'", utype->_id.print(), utype->title(), prop_value);
                     utype->_is_broken = true;
                 }
             } else if( sIsExactly(prop_name, "description") ) {
                 if( utype->_pos_description < 0 ) {
                     utype->_pos_description = _name_buf.length();
                     _name_buf.addString(prop_value);
                     _name_buf.add0();
                 } else if( !sIsExactly(utype->description(), prop_value) ) {
                     LOG_ERROR("Type \"%s\" found with 2 different descriptions: '%s' and '%s'", utype->_id.print(), utype->description(), prop_value);
                     utype->_is_broken = true;
                 }
             } else if( sIsExactly(prop_name, "is_abstract_fg") ) {
                 utype->_is_virtual = sString::parseBool(prop_value);
             } else if( sIsExactly(prop_name, "prefetch") )  {
                 utype->_is_prefetch = sString::parseBool(prop_value);
             } else if( sIsExactly(prop_name, "singleton") )  {
                 idx iprop_value = atoidx(prop_value);
                 if( iprop_value < eNotSingleton || iprop_value > eSingletonPerSystem ) {
                     LOG_WARNING("Type \"%s\" has unknown singleton value % " DEC "; treating as %d (systemwide singleton)", utype->_id.print(), iprop_value, eSingletonPerSystem);
                     iprop_value = eSingletonPerSystem;
                 }
                 utype->_is_singleton = (ESingleton)iprop_value;
             } else if( sIsExactly(prop_name, "parent") ) {
                 if( !utype->dimParents() || is_full_fetch ) {
                     sHiveId parent_id;
                     if( !parent_id.parse(prop_value) ) {
                         LOG_ERROR("Type \"%s\" has invalid parent \"%s\"", utype->_id.print(), prop_value);
                         utype->_is_broken = true;
                         return false;
                     }
                     dep_list.pushParent(utype->_id, parent_id, prop_group);
                 }
             } else if( sIs("field_", prop_name) ) {
                 prop_name += 6;
                 idx ifld = -sIdxMax;

                 if( is_full_fetch ) {
                     if( utype->_fields.dim() && !field_path_dic.get(utype->_id) ) {
                         LOG_ERROR("Type \"%s\" already had its field information loaded, refusing to overwrite", utype->_id.print());
                         return false;
                     }
                     ifld = field_path_dic.get(utype->_id, prop_group);
                     sUsrLoadingTypeField * fld = 0;
                     if( ifld >= 0 ) {
                         fld = utype->_fields.ptr(ifld);
                     } else {
                         ifld = utype->_fields.dim();
                         field_path_dic.set(utype->_id, prop_group, ifld);
                         fld = utype->_fields.addM(1);
                         new(fld) sUsrLoadingTypeField(true);
                         fld->_index = ifld;
                         fld->_owner_itype = fld->_definer_itype = utype->_itype;
                         LOG_TRACE("Type \"%s\" constructing new field %" DEC " for path \"%s\"", utype->_id.print(), ifld, prop_group ? prop_group : "");
                     }

                     if( sIsExactly(prop_name, "name") ) {
                         fld->_pos_name = fld->_pos_orig_name = sUsrLoadingTypeField::setString(prop_value);
                         *utype->_name2ifield.setString(canonicalCase(case_buf, prop_value)) = ifld;
                         if( isFieldWeakReference(utype->name(), prop_value) ) {
                             fld->_is_weak_reference = true;
                         }
                     } else if( sIsExactly(prop_name, "title") ) {
                         fld->_pos_title = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "type") ) {
                         if( sIs("type2", prop_value) ) {
                             prop_value += 5;
                             fld->_included_from_itype = INCLUDE_TYPE_EXPECTED;
                         } else {
                             fld->_included_from_itype = -1;
                         }
                         fld->_type = fieldTypeFromName(prop_value);
                     } else if( sIsExactly(prop_name, "parent") ) {
                         fld->_pos_parent_name = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "role") ) {
                         fld->_role = fieldRoleFromName(prop_value);
                     } else if( sIsExactly(prop_name, "is_key_fg") ) {
                         fld->_is_key = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "is_readonly_fg") ) {
                         switch(idx ro = atoidx(prop_value)) {
                             case sUsrTypeField::eReadWrite:
                             case sUsrTypeField::eWriteOnce:
                             case sUsrTypeField::eSubmitOnce:
                             case sUsrTypeField::eReadOnly:
                             case sUsrTypeField::eReadOnlyAutofill:
                                 fld->_readonly = (sUsrTypeField::EReadOnly)ro;
                                 break;
                             default:
                                 LOG_ERROR("Type \"%s\" has invalid field_is_readonly_fg value \"%s\" (group \"%s\")", utype->_id.print(), prop_value, prop_group);
                                 fld->_readonly = sUsrTypeField::eReadOnly;
                                 fld->_is_broken = true;
                         }
                     } else if( sIsExactly(prop_name, "is_optional_fg") ) {
                         fld->_is_optional = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "is_multi_fg") ) {
                         fld->_is_multi = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "is_hidden_fg") ) {
                         fld->_is_hidden = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "brief") ) {
                         fld->_pos_brief = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "is_summary_fg") ) {
                         fld->_is_summary = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "is_virtual_fg") ) {
                         fld->_is_virtual = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "is_batch_fg") ) {
                         fld->_is_batch = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "weakref") ) {
                         fld->_is_weak_reference = sString::parseBool(prop_value);
                     } else if( sIsExactly(prop_name, "order") ) {
                         fld->_pos_order = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "default_value") ) {
                         fld->_pos_default_value = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "default_encoding") ) {
                         fld->_default_encoding = atoidx(prop_value);
                     } else if( sIsExactly(prop_name, "link_url") ) {
                         fld->_pos_link_url = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "constraint") ) {
                         fld->_pos_constraint = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "constraint_data") ) {
                         fld->_pos_constraint_data = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "constraint_description") ) {
                         fld->_pos_constraint_description = sUsrLoadingTypeField::setString(prop_value);
                     } else if( sIsExactly(prop_name, "description") ) {
                         fld->_pos_description = sUsrLoadingTypeField::setString(prop_value);
                     }
                 }

                 if( sIsExactly(prop_name, "include_type") ) {
                     sHiveId include_id;
                     if( !include_id.parse(prop_value) ) {
                         LOG_ERROR("Type \"%s\" has invalid field_include_type \"%s\" (group \"%s\")", utype->_id.print(), prop_value, prop_group);
                         return false;
                     }
                     LOG_TRACE("Type \"%s\" has field_include_type \"%s\" (group \"%s\"); adding to dep list", utype->_id.print(), prop_value, prop_group);
                     dep_list.pushInclude(utype->_id, include_id, prop_group, ifld);
                 }
             } else {
                 LOG_ERROR("Type \"%s\" has unexpected property \"%s\" = \"%s\"", utype->_id.print(), prop_name, prop_value);
             }

            return true;
        }

        void addIfNotInLst(sLst<idx> & lst, idx val)
        {
            for(idx i = 0; i < lst.dim(); i++) {
                if( *lst.ptr(i) == val) {
                    return;
                }
            }
            *lst.add(1) = val;
        }

        bool assembleDeps()
        {
            dep_list.sort();
            sDic<idx> unique_includes;
            for(idx idep = 0; idep < dep_list.dim(); ) {
                idx istart = idep;
                const DepList::Entry & e_start = dep_list.getSorted(istart);
                if( sUsrLoadingType * utype = getRaw(e_start.type_id) ) {
                    unique_includes.empty();
                    for( idx iinc = 0; iinc < utype->_includes.dim(); iinc++ ) {
                        idx inc_itype = utype->_includes[iinc];
                        unique_includes.set(&inc_itype, sizeof(inc_itype));
                    }
                    for( ; idep < dep_list.dim() && e_start.type_id == dep_list.getSorted(idep).type_id; idep++ ) {
                        const DepList::Entry & e = dep_list.getSorted(idep);
                        LOG_TRACE("Got dep entry #%" DEC ": %s (%s path)", idep, e.print(local_log_buf, false, false), e.path_index >= 0 ? static_cast<const char*>(dep_list.interned_strings.id(e.path_index)) : "no");
                        const DepList::Entry * e_prev = idep > istart ? &dep_list.getSorted(idep - 1) : 0;
                        sUsrLoadingType * udep = getRaw(e.dep_id);

                        if( !udep ) {
                            LOG_ERROR("%s which cannot be fetched", e.print(local_log_buf, true, false));
                            utype->_is_broken = true;
                            continue;
                        }

                        if( e.kind == DepList::Entry::eParent ) {
                            addIfNotInLst(utype->_parents, udep->_itype);
                        } else if( e.kind == DepList::Entry::eChild ) {
                            addIfNotInLst(utype->_children, udep->_itype);
                        } else if( e.kind == DepList::Entry::eInclude ) {
                            if( utype->_fields.dim() && e.ifld >= 0 ) {
                                utype->_fields[e.ifld]._included_from_itype = udep->_itype;
                                LOG_TRACE("Type \"%s\" (%s) field \"%s\" (%" DEC ") is being marked as included from type \"%s\" (%s)", utype->id().print(), utype->name(), utype->_fields[e.ifld].name(), e.ifld, udep->id().print(local_log_buf), udep->name());
                            }

                            if( !e_prev || e_prev->kind != e.kind || e_prev->dep_id != e.dep_id ) {
                                if( !unique_includes.get(&udep->_itype, sizeof(udep->_itype)) ) {
                                    *utype->_includes.add(1) = udep->_itype;
                                    unique_includes.set(&udep->_itype, sizeof(udep->_itype));
                                }
                            }
                        }
                    }
                } else {
                    LOG_ERROR("%s which cannot be fetched", e_start.print(local_log_buf, true, true));
                    if( sUsrLoadingType * udep = getRaw(e_start.dep_id) ) {
                        udep->_is_broken = true;
                    }
                    idep++;
                }
            }

            return true;
        }

        bool linkFields()
        {
            if( full_fetch_ids2itype.dim() ) {
                DepForest forest;
                for(idx i = 0; i < full_fetch_ids2itype.dim(); i++) {
                    idx itype = *full_fetch_ids2itype.ptr(i);
                    sUsrLoadingType & utype = *_types[itype];
                    forest.add(&utype);
                    for(idx ifld = 0; ifld < utype._fields.dim(); ifld++) {
                        sUsrLoadingTypeField * fld = utype._fields.ptr(ifld);
                        if( fld->_included_from_itype == INCLUDE_TYPE_EXPECTED ) {
                            LOG_ERROR("Type \"%s\" field \"%s\" is type \"type2*\" but its field_include_type was missing or invalid", utype.id().print(), fld->name());
                            fld->_is_broken = true;
                        }
                    }
                }
                sVec<idx> loaded_itypes;
                forest.makeTraversal(loaded_itypes);
                sUsrLoadingType::linkFields(loaded_itypes);
            }

            return true;
        }
};

sHiveId sUsrLoadingType::type_type_id;

void sUsrLoadingType::loadFromObj(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch, bool lazy_fetch_fields)
{
    if( !name_len ) {
        name_len = sLen(name);
    }

    if( _all_prefetched ) {
        if( (name && strncmp(name, "*", name_len) && getRaw(name, name_len) && (lazy_fetch_fields || getRaw(name, name_len)->_is_fetched)) ||
            (type_id && getRaw(*type_id) && (lazy_fetch_fields || getRaw(*type_id)->_is_fetched)) ||
            (!name && !type_id))
        {
            return;
        }
    }

#define SP_TYPE_GET "sp_type_get_v6"
    std::unique_ptr<sSql::sqlProc> p(user.getProc(SP_TYPE_GET));
    if( name && strncmp(name, "*", name_len) == 0 ) {
        if( _all_deps_loaded ) {
            return;
        }
        _all_deps_loaded = true;
        p->Add("TRUE");
        if( no_prefetch && lazy_fetch_fields ) {
            p->Add("FALSE");
        } else {
            p->Add("(f.`name` = 'prefetch' AND f.`value` > 0)");
            _all_prefetched = true;
        }
    } else {
        sStr sql;
        if( _all_prefetched || no_prefetch ) {
            if( name ) {
                sql.addString("f.`name` = 'name' AND f.`value` = ");
                user.db().protectValue(sql, name, name_len);
            } else if( type_id ) {
                type_id->printSQL(sql, "o", true);
            }
        } else {
            sql.addString("(f.`name` = 'prefetch' AND f.`value` > 0)");
            if( name ) {
                sql.addString(" OR (f.`name` = 'name' AND f.`value` = ");
                user.db().protectValue(sql, name, name_len);
                sql.addString(")");
            } else if( type_id ) {
                sql.addString(" OR ");
                type_id->printSQL(sql, "o", true);
            }
            _all_prefetched = !lazy_fetch_fields;
        }
        p->Add(sql);
        if( lazy_fetch_fields ) {
            p->Add("FALSE");
        } else {
            p->Add("TRUE");
        }
    }
    p->Add(type_type_id.domainId()).Add(type_type_id.objId());

    sStr sql_buf;
    LoadFromObjContext ctx(user);
    sVec<sHiveId> unfetched_ids, unfetched_full_fetch_ids;
    do {
        idx dep_list_start = ctx.dep_list.vec.dim();
        ctx.resetType();
        unfetched_ids.empty();
        unfetched_full_fetch_ids.empty();
        if( p->resultOpen() && user.db().resultNext() ) {
            LOG_TRACE("%s;", p->rawStatement());
            if( !type_type_id && user.db().resultNextRow() ) {
                const idx domain_id_icol = user.db().resultColId("domainID");
                const idx obj_id_icol = user.db().resultColId("objID");
                type_type_id.set(user.db().resultUValue(domain_id_icol), user.db().resultUValue(obj_id_icol), 0);
            }
            if( !user.db().resultNext() ) {
                break;
            }
            const idx domain_id_icol = user.db().resultColId("domainID");
            const idx obj_id_icol = user.db().resultColId("objID");
            const idx is_full_fetch_fg_icol = user.db().resultColId("is_full_fetch_fg");
            const idx prop_name_icol = user.db().resultColId("name");
            const idx prop_group_icol = user.db().resultColId("group");
            const idx prop_value_icol = user.db().resultColId("value");
            while( user.db().resultNextRow() ) {
                sHiveId type_id(user.db().resultUValue(domain_id_icol), user.db().resultUValue(obj_id_icol), 0);
                bool is_full_fetch = user.db().resultIValue(is_full_fetch_fg_icol) > 0;
                const char * prop_name = user.db().resultValue(prop_name_icol);
                if( !prop_name ) {
                    prop_name = sStr::zero;
                }
                const char * prop_group = user.db().resultValue(prop_group_icol);
                if( !prop_group ) {
                    prop_group = sStr::zero;
                }
                const char * prop_value = user.db().resultValue(prop_value_icol);
                if( !prop_value ) {
                    prop_value = sStr::zero;
                }

                LOG_TRACE("Got row from " SP_TYPE_GET ": %s,%" DEC ",%s,%s,\"%s\"", type_id.print(), user.db().resultIValue(is_full_fetch_fg_icol), prop_name, prop_group, prop_value);

                idx itype = -sIdxMax;
                sUsrLoadingType * utype = 0;
                if( idx * pitype = _name_or_id2itype.get(&type_id, sizeof(sHiveId)) ) {
                    itype = *pitype;
                    utype = _types[itype];
                    if( utype->_is_fetched ) {
                        continue;
                    }
                } else {
                    itype = _types.dim();
                    utype = new sUsrLoadingType(true);
                    *_types.add(1) = utype;
                    utype->_itype = itype;
                    utype->_id = type_id;
                    *_name_or_id2itype.set(&type_id, sizeof(sHiveId)) = itype;
                }

                ctx.utype = utype;

                if( is_full_fetch ) {
                    ctx.is_full_fetch = true;
                    *ctx.full_fetch_ids2itype.set(&type_id, sizeof(sHiveId)) = itype;
                } else {
                    ctx.is_full_fetch = false;
                }

                if( !ctx.accumulateProp(prop_name, prop_value, prop_group) ) {
                    continue;
                }
            }

            user.db().resultClose();
        }

        for(idx id = dep_list_start; id < ctx.dep_list.vec.dim(); id++) {
            const sHiveId * pdep = &ctx.dep_list.vec[id].dep_id;
            const sHiveId * p_dep_of = &ctx.dep_list.vec[id].type_id;
            const sUsrLoadingType * udep = getRaw(*pdep);

            if( ctx.full_fetch_ids2itype.get(p_dep_of, sizeof(sHiveId)) ) {
                if( (!udep || !udep->_is_fetched) && !ctx.full_fetch_ids2itype.get(pdep, sizeof(sHiveId)) ) {
                    *unfetched_ids.add(1) = *pdep;
                    *unfetched_full_fetch_ids.add(1) = *pdep;
                    LOG_TRACE("Type \"%s\" depends on \"%s\" which was not fully fetched yet, adding to fetch list (will fetch all fields)", ctx.dep_list.vec[id].type_id.print(), pdep->print(local_log_buf));
                }
            } else {
                if( !udep ) {
                    *unfetched_ids.add(1) = *pdep;
                    LOG_TRACE("Type \"%s\" depends on \"%s\" which was not fetched yet, adding to fetch list (in minimal mode)", ctx.dep_list.vec[id].type_id.print(), pdep->print(local_log_buf));
                }
            }
        }

        if( unfetched_ids.dim() ) {
            p.reset(user.getProc(SP_TYPE_GET));
            sql_buf.cut0cut();
            p->Add(sSql::exprInList(sql_buf, "o.domainID", "o.objID", unfetched_ids));
            if( unfetched_full_fetch_ids.dim() == 0 ) {
                p->Add("FALSE");
            } else if( unfetched_full_fetch_ids.dim() == unfetched_ids.dim() ) {
                p->Add("TRUE");
            } else {
                sql_buf.cut0cut();
                p->Add(sSql::exprInList(sql_buf, "o.domainID", "o.objID", unfetched_full_fetch_ids));
            }
            p->Add(type_type_id.domainId()).Add(type_type_id.objId());
        }
    } while( unfetched_ids.dim() );

    ctx.assembleDeps();
    ctx.linkFields();
}

sUsrLoadingType * sUsrLoadingType::ensureEphemeral(const sHiveId & type_id)
{
    sUsrLoadingType * utype = getRaw(type_id);
    if( !utype ) {
        idx itype = _types.dim();
        utype = new sUsrLoadingType();
        *_types.add(1) = utype;
        utype->_itype = itype;
        utype->_id = type_id;
        utype->_is_fetched = true;
        utype->_is_broken = true;
        utype->_is_ephemeral = true;
        *_name_or_id2itype.set(&type_id, sizeof(sHiveId)) = itype;
        *_name_or_id2ephemeral_itype.set(&type_id, sizeof(sHiveId)) = itype;

        utype->_pos_name = _name_buf.length();
        _name_buf.addString("_unknown-");
        type_id.print(_name_buf);
        _name_buf.add0();
        *_name_or_id2itype.setString(_name_buf.ptr(utype->_pos_name)) = itype;
        *_name_or_id2ephemeral_itype.setString(_name_buf.ptr(utype->_pos_name)) = itype;

        utype->_pos_title = _name_buf.length();
        _name_buf.addString("Unknown type (");
        type_id.print(_name_buf);
        _name_buf.addString(")");
        _name_buf.add0();
    }
    return utype->_is_ephemeral ? utype : 0;
}

class sUsrLoadingType::JSONLoader : public sUsrPropSet
{
    private:
        struct BuiltinFieldPack {
            const char * name;
            sUsrTypeField::EType type;
            sUsrTypeField::ERole role;
            bool is_array_row;
            bool is_key;
            bool is_optional;
            bool is_multi;
            bool is_global_multi;
            bool is_flattened_decor;
            bool is_flattened_multi;
            const char * order_string;
            const char * parent;
            idx ancestor_count;
            const char * flattened_parent;
            const char ** children;
        };

        class BuiltinType;
        class BuiltinField : public sUsrTypeField {
            private:
                const BuiltinFieldPack * _pack;
                friend class BuiltinType;

            public:
                static const BuiltinFieldPack builtin_field_packs[];
                static sDic<BuiltinField> builtin_fields;
                static const BuiltinField unknown_field;

                BuiltinField(const BuiltinFieldPack * pack = 0) { _pack = pack; }

                virtual ~BuiltinField() {}
                virtual const char * name() const { return _pack ? _pack->name : "__unknown_field__"; }
                virtual const char * originalName() const { return name(); }
                virtual const char * title() const { return name(); }
                virtual EType type() const { return _pack ? _pack->type : sUsrTypeField::eInvalid; }
                virtual const BuiltinField* parent() const { return _pack && _pack->parent ? builtin_fields.get(_pack->parent) : 0; }
                virtual ERole role() const { return _pack ? _pack->role : eRole_unknown; }
                virtual idx dimChildren() const
                {
                    idx cnt = 0;
                    if( _pack && _pack->children ) {
                        for(; _pack->children[cnt]; cnt++);
                    }
                    return cnt;
                }
                virtual const BuiltinField * getChild(idx ichild) const { return _pack && _pack->children ? builtin_fields.get(_pack->children[ichild]) : 0; }
                virtual idx getChildren(sVec<const sUsrTypeField*> &out) const
                {
                    idx cnt = 0;
                    if( _pack && _pack->children ) {
                        for(; _pack->children[cnt]; cnt++) {
                            *out.add(1) = builtin_fields.get(_pack->children[cnt]);
                        }
                    }
                    return cnt;
                }
                virtual bool isKey() const { return _pack ? _pack->is_key : false; }
                virtual EReadOnly readonly() const { return _pack ? eReadWrite : eReadOnly; }
                virtual bool isOptional() const { return _pack ? _pack->is_optional : true; }
                virtual bool isMulti() const { return _pack ? _pack->is_multi : true; }
                virtual bool isHidden() const { return false; }
                virtual bool isSummary() const { return false; }
                virtual bool isVirtual() const { return false; }
                virtual bool isBatch() const { return false; }
                virtual bool isWeakReference() const { return false; }
                virtual bool isSysInternal() const { return false; }
                virtual const char * brief() const { return 0; }
                virtual real order() const { return _pack && _pack->order_string ? strtod(_pack->order_string, 0) : 0; }
                virtual const char * orderString() const { return _pack ? _pack->order_string : 0; }
                virtual const char * defaultValue() const { return 0; }
                virtual idx defaultEncoding() const { return 0; }
                virtual const char * constraint() const { return 0; }
                virtual const char * constraintData() const { return 0; }
                virtual const char * constraintDescription() const { return 0; }
                virtual const char * description() const { return 0; }
                virtual const char * linkUrl() const { return 0; }

                virtual bool canSetValue() const { return canHaveValue(); }
                virtual bool isArrayRow() const { return _pack ? _pack->is_array_row : false; }
                virtual bool isGlobalMulti() const { return _pack ? _pack->is_global_multi : false; }
                virtual bool isFlattenedDecor() const { return _pack ? _pack->is_flattened_decor : false; }
                virtual bool isFlattenedMulti() const { return _pack ? _pack->is_flattened_multi : false; }
                const sUsrTypeField* flattenedParent() const { return _pack && _pack->flattened_parent ? builtin_fields.get(_pack->flattened_parent) : 0; }
                virtual idx ancestorCount() const { return _pack ? _pack->ancestor_count : false; }

                virtual const sUsrType2 * ownerType() const;
                virtual const sUsrType2 * definerType() const { return ownerType(); }
                virtual const sUsrType2 * includedFromType() const { return 0; }
        };

        static class BuiltinType : public sUsrType2 {
            public:
                BuiltinType()
                {
                    if( !BuiltinField::builtin_fields.dim() ) {
                        for(const BuiltinFieldPack * pack = BuiltinField::builtin_field_packs; pack && pack->name; pack++) {
                            BuiltinField::builtin_fields.setString(pack->name)->_pack = pack;
                        }
                        for(idx i = 0; i < BuiltinField::builtin_fields.dim(); i++) {
                            const BuiltinField * fld = BuiltinField::builtin_fields.ptr(i);
                            if( !fld->parent() ) {
                                *_root_fields.add(1) = fld;
                            }
                        }
                    }
                }

                virtual ~BuiltinType() {}

                virtual const sHiveId & id() const
                {
                    static const sHiveId _id("type", 1, 0);
                    return _id;
                }
                virtual const char * name() const { return "type"; }
                virtual const char * title() const { return 0; }
                virtual const char * description() const { return 0; }
                virtual bool isVirtual() const { return 0; }
                virtual bool isPrefetch() const { return 1; }
                virtual bool isUser() const { return 0; }
                virtual bool isSystem() const { return 0; }
                virtual ESingleton isSingleton() const { return eNotSingleton; }

                virtual idx dimParents() const { return 0; }
                virtual const sUsrType2 * getParent(idx iparent) const { return 0; }
                virtual idx getParents(sVec<const sUsrType2*> & out) const { return 0; }
                virtual idx dimChildren() const { return 0; }
                virtual const sUsrType2 * getChild(idx ichild) const { return 0; }
                virtual idx getChildren(sVec<const sUsrType2*> & out) const { return 0; }
                virtual idx dimIncludes() const { return 0; }
                virtual const sUsrType2 * getInclude(idx iinc) const { return 0; }
                virtual idx getIncludes(sVec<const sUsrType2*> & out) const { return 0; }

                virtual idx dimFields(const sUsr & user) const { return BuiltinField::builtin_fields.dim(); }
                virtual idx dimRootFields(const sUsr & user) const { return _root_fields.dim(); }
                virtual const BuiltinField* getField(const sUsr & user, idx ifield) const { return BuiltinField::builtin_fields.ptr(ifield); }
                virtual const BuiltinField* getRootField(const sUsr & user, idx irootfld) const { return _root_fields[irootfld]; }
                virtual const BuiltinField* getField(const sUsr & user, const char * field_name, idx field_name_len = 0) const { return BuiltinField::builtin_fields.get(field_name, field_name_len); }

                virtual idx dimActions(const sUsr & user) const { return 0; }
                virtual const sUsrAction * getAction(const sUsr & user, const char * act_name, idx act_name_len = 0) const { return 0; }
                virtual const sUsrAction * getAction(const sUsr & user, idx iact) const { return 0; }
                virtual idx getActions(const sUsr & user, sVec<const sUsrAction *> & out) const { return 0; }

                virtual idx dimJSComponents(const sUsr & user) const { return 0; }
                virtual const sUsrJSComponent * getJSComponent(const sUsr & user, const char * name, idx name_len = 0) const { return 0; }
                virtual const sUsrJSComponent * getJSComponent(const sUsr & user, idx ijsco) const { return 0; }
                virtual idx getJSComponents(const sUsr & user, sVec<const sUsrJSComponent *> & out) const { return 0; }

                virtual idx dimViews(const sUsr & user) const { return 0; }
                virtual const sUsrView * getView(const sUsr & user, const char * view_name, idx view_name_len = 0) const { return 0; }
                virtual const sUsrView * getView(const sUsr & user, idx iview) const { return 0; }
                virtual idx getViews(const sUsr & user, sVec<const sUsrView *> & out) const { return 0; }

            private:
                sVec<const BuiltinField*> _root_fields;
        } _builtin_type;

        sUsrLoadingType::LoadFromObjContext _ctx;
        sStr _value_buf;

    public:
        JSONLoader(const sUsr & usr) : sUsrPropSet(usr), _ctx(usr) { }

        sUsrLoadingType::LoadFromObjContext & getCtx() { return _ctx; }

        virtual bool ensureUTypeFor(ObjLoc * prop_obj, sJSONParser::ParseNode & node, sJSONParser & parser)
        {
            if( strncmp_exact(node.val.str, node.val_str_len, "type", 4) != 0 ) {
                parser.setValueError(node, "type type expeced");
                return false;
            }
            prop_obj->obj.utype = &_builtin_type;
            return true;
        }
        virtual bool checkCurObjSanity(sJSONParser::ParseNode & node, sJSONParser & parser)
        {
            ObjLoc * prop_obj = _objs.ptr(_cur_iobj);
            if( !prop_obj->obj.id ) {
                parser.setValueError(node, "missing or invalid _id for object");
                _ctx.resetType();
                return false;
            }

            idx itype = -sIdxMax;
            sUsrLoadingType * utype = 0;
            if( idx * pitype = _name_or_id2itype.get(&prop_obj->obj.id, sizeof(sHiveId)) ) {
                itype = *pitype;
                utype = _types[itype];
                if( utype->_is_fetched ) {
                    _ctx.resetType();
                    return true;
                }
            } else {
                itype = sUsrLoadingType::_types.dim();
                utype = new sUsrLoadingType(true);
                *sUsrLoadingType::_types.add(1) = utype;
                utype->_itype = itype;
                utype->_id = prop_obj->obj.id;
                *sUsrLoadingType::_name_or_id2itype.set(&prop_obj->obj.id, sizeof(sHiveId)) = itype;
            }

            if( _ctx.utype != utype ) {
                _ctx.resetType();
                *_ctx.full_fetch_ids2itype.set(&prop_obj->obj.id, sizeof(sHiveId)) = itype;
                _ctx.is_full_fetch = true;
                _ctx.utype = utype;

            }

            return true;
        }
        virtual udx getUpdateLevel() { return 1; }
        virtual bool updateStart() { return true; }
        virtual bool hadDeadlocked() { return true; }
        virtual bool updateAbandon() { return true; }
        virtual bool updateComplete() { return true; }
        virtual bool propInit(ObjLoc * prop_obj)
        {
            return true;
        }
        virtual udx propSet(ObjLoc * prop_obj, const sUsrTypeField * fld, const char * path, const char * value, udx value_len)
        {
            if( _ctx.utype ) {
                _value_buf.cutAddString(0, value, value_len);
                _ctx.accumulateProp(fld->name(), _value_buf.ptr(), path);
            }
            return 1;
        }
        virtual bool setPermission(ObjLoc * prop_obj, sJSONParser::ParseNode & node, Perm & perm)
        {
            return true;
        }
        virtual bool fldNeedsValidation(const sUsrTypeField * fld) const
        {
            return dynamic_cast<const BuiltinField*>(fld) != &BuiltinField::unknown_field;
        }
};

const sUsrType2 * sUsrLoadingType::JSONLoader::BuiltinField::ownerType() const { return &_builtin_type; }

const sUsrTypeCache * sUsrType2::getBinCache(const sUsr * user)
{
    bool need_regen = true;

    static enum {
        eNotCached,
        eCached,
        eFailed
    } status = eNotCached;
    static sUsrTypeCache bin_cache;

    if( !useBinCache() ) {
        return 0;
    }

    if( status == eCached ) {
        return &bin_cache;
    } else if( status == eFailed ) {
        return 0;
    }

    if( !user ) {
        return 0;
    }

    sStr dirname;
    dirname.addString(getenv("QM_LOCAL_CACHE_DIRECTORY"));
    if( !dirname.length() ) {
        user->QPride()->cfgStr(&dirname, 0, "qm.localCacheDirectory");
        dirname.shrink00();
    }
    if( !dirname.length() ) {
        user->QPride()->cfgStr(&dirname, 0, "qm.resourceRoot");
        dirname.shrink00();
        dirname.printf("cache%c", sDir::sep);
    }
    if( dirname[dirname.length() - 1] != sDir::sep ) {
        dirname.addString(&sDir::sep, 1);
    }

    if( !sDir::exists(dirname) ) {
        if( !sDir::makeDir(dirname) ) {
            LOG_ERROR("Failed to make directory \"%s\"", dirname.ptr());
            return 0;
        }
    }

    sStr filename("%sqpride_type_cache.v%d.%d.os%s.bin", dirname.ptr(), USR_TYPE_CACHE_MAJOR_VERSION, USR_TYPE_CACHE_MINOR_VERSION, SLIB_PLATFORM);

    if( !bin_cache.load(filename) ) {
        status = eCached;

        idx cache_mtime = bin_cache.mtime();
        idx db_mtime = 0;

        std::unique_ptr<sSql::sqlProc> p(user->getProc("sp_type_get_latest_mtime"));
        p->Add(sUsrLoadingType::type_type_id.domainId()).Add(sUsrLoadingType::type_type_id.objId());
        if( p->resultOpen() && user->db().resultNext() ) {
            if( !sUsrLoadingType::type_type_id && user->db().resultNextRow() ) {
                const idx domain_id_icol = user->db().resultColId("domainID");
                const idx obj_id_icol = user->db().resultColId("objID");
                sUsrLoadingType::type_type_id.set(user->db().resultUValue(domain_id_icol), user->db().resultUValue(obj_id_icol), 0);
            }
            static const char * result_kind[] = { "type", "action", "view", "js_component" };
            for(idx ikind = 0; ikind < sDim(result_kind); ikind++) {
                if( user->db().resultNext() && user->db().resultNextRow() ) {
                    const idx domain_id_icol = user->db().resultColId("domainID");
                    const idx obj_id_icol = user->db().resultColId("objID");
                    const idx value_icol = user->db().resultColId("value");

                    sVariant date_parser;
                    date_parser.parseDateTime(user->db().resultValue(value_icol));

                    sHiveId latest_type_id;
                    latest_type_id.set(user->db().resultUValue(domain_id_icol), user->db().resultUValue(obj_id_icol), 0);
                    LOG_TRACE("Latest %s object mtime is %s for type \"%s\"", result_kind[ikind], date_parser.asString(), latest_type_id.print());

                    db_mtime = sMax(db_mtime, date_parser.asDateTime());
                } else {
                    break;
                }
            }
        }

        if( db_mtime && db_mtime < sIdxMax && cache_mtime && cache_mtime < sIdxMax && cache_mtime > db_mtime ) {
            need_regen = false;
        }
    }

    if( need_regen ) {
        status = eFailed;

        sUsr superuser("qpride", true);
        if( !superuser.Id() ) {
            LOG_TRACE("%s", "Failed to enter superuser mode for type loading");
            return 0;
        }

        sUsrLoadingType::load(superuser, "*", 0, 0, false, false);
        sUsrLoadingType::loadActions(superuser);
        sUsrLoadingType::loadJSComponents(superuser);
        sUsrLoadingType::loadViews(superuser);

        if( sRC rc = bin_cache.save(superuser, filename) ) {
            LOG_ERROR("Failed to save type cache: %s", rc.print());
        } else if( sRC rc = bin_cache.load(filename) ) {
            LOG_ERROR("Failed to load type cache: %s", rc.print());
        } else {
            status = eCached;
        }
    }

    return status == eCached ? &bin_cache : 0;
}

bool sUsrType2::loadFromJSON(const sUsr & user, const char * buf, idx buf_len)
{
    sUsrLoadingType::JSONLoader json_loader(user);
    json_loader.setSrc(buf, buf_len);
    if( !json_loader.run(0, sUsrPropSet::fInvalidUserGroupNonFatal | sUsrPropSet::fOverwriteExistingSameType) ) {
        fprintf(stderr, "Failed to load type(s) from JSON: %s\n", json_loader.getErr());
        return false;
    }
    json_loader.getCtx().assembleDeps();
    json_loader.getCtx().linkFields();

    return true;
}

bool sUsrLoadingType::loadFromJSONCache(const sUsr & user)
{
    bool need_regen = true;

    sStr tempdir;
    user.QPride()->cfgStr(&tempdir, 0, "qm.tempDirectory", "/tmp");
    tempdir.shrink00();
    if( tempdir[tempdir.length() - 1] != sDir::sep ) {
        tempdir.addString(&sDir::sep, 1);
    }
    sStr type_cache_filename("%sqpride_type_cache.v1.json", tempdir.ptr());

    sFil type_cache_fil(type_cache_filename, sMex::fReadonly);
    if( type_cache_fil.ok() && type_cache_fil.length() ) {
        idx file_mtime = type_cache_fil.mtime();
        idx db_mtime = 0;

        std::unique_ptr<sSql::sqlProc> p(user.getProc("sp_type_get_latest_mtime"));
        p->Add(type_type_id.domainId()).Add(type_type_id.objId());
        if( p->resultOpen() && user.db().resultNext() ) {
            if( !type_type_id && user.db().resultNextRow() ) {
                const idx domain_id_icol = user.db().resultColId("domainID");
                const idx obj_id_icol = user.db().resultColId("objID");
                type_type_id.set(user.db().resultUValue(domain_id_icol), user.db().resultUValue(obj_id_icol), 0);
            }
            if( user.db().resultNext() && user.db().resultNextRow() ) {
                const idx domain_id_icol = user.db().resultColId("domainID");
                const idx obj_id_icol = user.db().resultColId("objID");
                const idx value_icol = user.db().resultColId("value");

                sVariant date_parser;
                date_parser.parseDateTime(user.db().resultValue(value_icol));

                sHiveId latest_type_id;
                latest_type_id.set(user.db().resultUValue(domain_id_icol), user.db().resultUValue(obj_id_icol), 0);
                LOG_TRACE("Latest type object mtime is %s for type \"%s\"", date_parser.asString(), latest_type_id.print());

                db_mtime = date_parser.asDateTime();
            }
        }

        if( db_mtime && db_mtime < sIdxMax && file_mtime && file_mtime < sIdxMax && file_mtime > db_mtime ) {
            if( sUsrType2::loadFromJSON(user, type_cache_fil.ptr(), type_cache_fil.length()) ) {
                _all_prefetched = true;
                need_regen = false;
            }
        }
    }
    type_cache_fil.destroy();

    if( need_regen ) {
        sUsr superuser("qpride", true);
        if( !superuser.Id() ) {
            LOG_TRACE("%s", "Failed to enter superuser mode for type loading");
            return false;
        }

        if( !sUsrType2::get("type") ) {
            sUsrLoadingType::loadFromObj(superuser, "type", 4, 0);
            if( !sUsrType2::get("type") ) {
                LOG_ERROR("%s", "Failed to get type type");
                return false;
            }
        }

        sUsrObjRes res;
        sVec<sHiveId> ids;
        superuser.objs2("^type$", res);
        for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it)) {
            const sHiveId * oid = res.id(it);
            *ids.add(1) = *oid;
        }

        sStr type_cache_temp_filename;
        if( !sFile::mktemp(type_cache_temp_filename, tempdir, "json") ) {
            LOG_ERROR("%s", "Failed to make temp file");
            return false;
        }
        sFil type_cache_temp_fil(type_cache_temp_filename);
        if( !type_cache_temp_fil.ok() ) {
            LOG_ERROR("%s", "Failed to open temp file for writing");
            sFile::remove(type_cache_temp_filename);
            return false;
        }

        sJSONPrinter printer(&type_cache_temp_fil);
        superuser.propExport(ids, printer, sUsr::ePermExportGroups, 0, true);
        printer.finish();

        sFile::remove(type_cache_filename);
        if( !sFile::rename(type_cache_temp_filename, type_cache_filename) ) {
            LOG_ERROR("%s", "Failed to move type cache file");
            sFile::remove(type_cache_temp_filename);
            return false;
        }
    }

    return true;
}

static const char * builtin_field_packs__row_fields_children[] = { "field_basics", "field_flags", "field_constraint_list", "field_presentation", 0 };
static const char * builtin_field_packs_field_basics_children[] = { "field_name", "field_title", "field_type",  "field_parent",  "field_order", "field_default_value", "field_default_encoding", "field_include_type", 0 };
static const char * builtin_field_packs_field_flags_children[] = { "field_role", "field_is_key_fg", "field_is_readonly_fg", "field_is_optional_fg", "field_is_multi_fg", "field_is_virtual_fg", "field_is_hidden_fg", "field_is_summary_fg", "field_is_batch_fg", "field_weakref", 0 };
static const char * builtin_field_packs_field_constraint_list_children[] = { "field_constraint", "field_constraint_data", "field_constraint_description", 0 };
static const char * builtin_field_packs_field_presentation_children[] = { "field_brief", "field_link_url",  "field_description", 0 };
const sUsrLoadingType::JSONLoader::BuiltinFieldPack sUsrLoadingType::JSONLoader::BuiltinField::builtin_field_packs[] = {
    { "created", sUsrTypeField::eString, sUsrTypeField::eRole_state, false, false, true, false, false, false, false, 0, 0, 0, 0, 0 },
    { "modified", sUsrTypeField::eString, sUsrTypeField::eRole_state, false, false, true, false, false, false, false, 0, 0, 0, 0, 0 },
    { "name", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, true, false, false, false, false, false, "1", 0, 0, 0, 0 },
    { "title", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, false, false, false, "2", 0, 0, 0, 0 },
    { "description", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, false, false, false, "3", 0, 0, 0, 0 },
    { "parent", sUsrTypeField::eString, sUsrTypeField::eRole_input, false, false, true, true, true, false, true, "4", 0, 0, 0, 0 },
    { "is_abstract_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, false, false, false, "5", 0, 0, 0, 0 },
    { "singleton", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, false, false, false, "6", 0, 0, 0, 0 },
    { "prefetch", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, false, false, false, "7", 0, 0, 0, 0 },
    { "fields", sUsrTypeField::eArray, sUsrTypeField::eRole_parameter, false, false, true, false, false, false, false, "8", 0, 0, 0, 0 },
    { "_row_fields", sUsrTypeField::eList, sUsrTypeField::eRole_parameter, true, false, true, true, true, false, true, 0, "fields", 1, "fields", builtin_field_packs__row_fields_children },
    { "field_basics", sUsrTypeField::eList, sUsrTypeField::eRole_parameter, false, false, true, true, true, true, false, "1", "_row_fields", 2, "_row_fields", builtin_field_packs_field_basics_children },
    { "field_flags", sUsrTypeField::eList, sUsrTypeField::eRole_parameter, false, false, true, true, true, true, false, "2", "_row_fields", 2, "_row_fields", builtin_field_packs_field_flags_children },
    { "field_constraint_list", sUsrTypeField::eList, sUsrTypeField::eRole_parameter, false, false, true, true, true, true, false, "3", "_row_fields", 2, "_row_fields", builtin_field_packs_field_constraint_list_children },
    { "field_presentation", sUsrTypeField::eList, sUsrTypeField::eRole_parameter, false, false, true, true, true, true, false, "4", "_row_fields", 2, "_row_fields", builtin_field_packs_field_presentation_children },
    { "field_name", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "1", "field_basics", 3, "_row_fields", 0 },
    { "field_title", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "2", "field_basics", 3, "_row_fields", 0 },
    { "field_type", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "3", "field_basics", 3, "_row_fields", 0 },
    { "field_parent", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "4", "field_basics", 3, "_row_fields", 0 },
    { "field_order", sUsrTypeField::eReal, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "5", "field_basics", 3, "_row_fields", 0 },
    { "field_default_value", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "6", "field_basics", 3, "_row_fields", 0 },
    { "field_default_encoding", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "7", "field_basics", 3, "_row_fields", 0 },
    { "field_include_type", sUsrTypeField::eString, sUsrTypeField::eRole_input, false, false, true, false, true, false, false, "8", "field_basics", 3, "_row_fields", 0 },
    { "field_role", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "1", "field_flags", 3, "_row_fields", 0 },
    { "field_is_key_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "2", "field_flags", 3, "_row_fields", 0 },
    { "field_is_readonly_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "3", "field_flags", 3, "_row_fields", 0 },
    { "field_is_optional_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "4", "field_flags", 3, "_row_fields", 0 },
    { "field_is_multi_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "5", "field_flags", 3, "_row_fields", 0 },
    { "field_is_virtual_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "6", "field_flags", 3, "_row_fields", 0 },
    { "field_is_hidden_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "7", "field_flags", 3, "_row_fields", 0 },
    { "field_is_summary_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "8", "field_flags", 3, "_row_fields", 0 },
    { "field_is_batch_fg", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "9","field_flags", 3, "_row_fields", 0 },
    { "field_weakref", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "10","field_flags", 3, "_row_fields", 0 },
    { "field_constraint", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "1", "field_contraint_list", 3, "_row_fields", 0 },
    { "field_constraint_data", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "2", "field_contraint_list", 3, "_row_fields", 0 },
    { "field_constraint_description", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "3", "field_contraint_list", 3, "_row_fields", 0 },
    { "field_brief", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "1", "field_presentation", 3, "_row_fields", 0 },
    { "field_link_url", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "2", "field_presentation", 3, "_row_fields", 0 },
    { "field_description", sUsrTypeField::eString, sUsrTypeField::eRole_parameter, false, false, true, false, true, false, false, "3", "field_presentation", 3, "_row_fields", 0 },
    0
};
sDic<sUsrLoadingType::JSONLoader::BuiltinField> sUsrLoadingType::JSONLoader::BuiltinField::builtin_fields;
const sUsrLoadingType::JSONLoader::BuiltinField sUsrLoadingType::JSONLoader::BuiltinField::unknown_field;
sUsrLoadingType::JSONLoader::BuiltinType sUsrLoadingType::JSONLoader::_builtin_type;

sUsrType2 * sUsrType2::load(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch, bool lazy_fetch_fields)
{
    return sUsrLoadingType::load(user, name, name_len, type_id, no_prefetch, lazy_fetch_fields);
}

sUsrLoadingType * sUsrLoadingType::load(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch, bool lazy_fetch_fields)
{
    sUsrLoadingType * utype = 0;
    if( !name_len ) {
        name_len = sLen(name);
    }

    if( name_len || (type_id && *type_id) ) {
        if( useJSONCache() ) {
            useJSONCache(true, false);
            loadFromJSONCache(user);
        }
        loadFromObj(user, name, name_len, type_id, no_prefetch, lazy_fetch_fields);

        if( name && strncmp(name, "*", name_len) != 0 ) {
            utype = getRaw(name, name_len);
        } else if( type_id ) {
            utype = getRaw(*type_id);
        }
    }

    return utype;
}

struct sUsrLoadingType::SetFieldChildrenParam
{
    sDic<idx> done_dic;
    sMex children_buf;
    sDic< sLst<idx> > children_dic;

    void empty()
    {
        done_dic.empty();
        children_dic.empty();
        children_buf.empty();
    }

    bool hasChildren(idx ipar_fld)
    {
        sLst<idx> * children = children_dic.get(&ipar_fld, sizeof(idx));
        return children && children->_mex && children->dim();
    }

    void addChild(idx ipar_fld, idx ichld_fld)
    {
        sLst<idx> * children = children_dic.set(&ipar_fld, sizeof(idx));
        if( !children->_mex ) {
            children->init(&children_buf);
        }
        *children->add(1) = ichld_fld;
    }
};

static idx loadingFldCompare(void * param, void * arr, idx i1, idx i2)
{
    const sUsrLoadingTypeField * fields = static_cast<const sUsrLoadingTypeField*>(param);
    const idx * indices = static_cast<const idx*>(arr);
    return fields[indices[i1]].cmp(fields + indices[i2]);
}

void sUsrLoadingType::linkFields(sVec<idx> & itypes)
{
    sStr inc_buf, case_buf, subst_buf;

    struct
    {
        const char * name, * title;
        idx pos_name, pos_title;
        sUsrTypeField::EType type;
        sUsrTypeField::ERole role;
        sUsrTypeField::EReadOnly readonly;
        bool is_multi;
        bool is_weak_reference;
        bool is_sysinternal;
    } system_fields[] = {
        { "_id", "ID", -1, -1, sUsrTypeField::eString, sUsrTypeField::eRole_state, sUsrTypeField::eReadOnly, false, false, false },
        { "_type", "Type name", -1, -1, sUsrTypeField::eString, sUsrTypeField::eRole_state, sUsrTypeField::eWriteOnce, false, false, false },
        { "_parent", "Parent object", -1, -1, sUsrTypeField::eObj, sUsrTypeField::eRole_state, sUsrTypeField::eReadOnly, true, true, false },
        { "_brief", "Summary", -1, -1, sUsrTypeField::eString, sUsrTypeField::eRole_state, sUsrTypeField::eReadOnly, false, false, false },
        { "_dir", "Location", -1, -1, sUsrTypeField::eString, sUsrTypeField::eRole_state, sUsrTypeField::eReadOnly, true, true, true },
        { "_perm", "Permissions", -1, -1, sUsrTypeField::eString, sUsrTypeField::eRole_state, sUsrTypeField::eReadOnly, true, false, false },
        { "_effperm", "Effective permissions", -1, -1, sUsrTypeField::eJSON, sUsrTypeField::eRole_state, sUsrTypeField::eReadOnly, true, false, false },
        { "_domain", "Create in domain", -1, -1, sUsrTypeField::eJSON, sUsrTypeField::eRole_state, sUsrTypeField::eWriteOnce, false, false, false }
    };

    sDic<idx> overridden_dic, broken_ifld_dic;
    SetFieldChildrenParam children_param;
    sStr array_row_name;

    for(idx it=0; it<itypes.dim(); it++) {
        sUsrLoadingType & utype = *_types[itypes[it]];
        utype._dim_explicit_fields = utype._fields.dim();

        LOG_TRACE("Inheritance and inclusion for type '%s'", utype.name());

        overridden_dic.empty();
        idx cnt_utype_par = utype.dimParents();
        for(idx itp=0; itp<cnt_utype_par; itp++) {
            sUsrLoadingType & utype_par = *_types[utype._parents[itp]];
            LOG_TRACE("Inheriting %" DEC " fields from parent %s to child %s", utype_par._root_ifields.dim(), utype_par.name(), utype.name());
            for(idx irif=0; irif<utype_par._root_ifields.dim(); irif++) {
                utype.inheritField(utype_par._fields.ptr(utype_par._root_ifields[irif]), overridden_dic, case_buf);
            }
        }

        utype._dim_inherited_fields = utype._fields.dim() - utype._dim_explicit_fields;
        for(idx ifld=0; ifld<utype._dim_explicit_fields; ifld++) {
            idx included_from_itype = utype._fields[ifld]._included_from_itype;
            if( included_from_itype >= 0 ) {
                if( !_types[included_from_itype]->_is_fetched ) {
                    LOG_ERROR("Type \"%s\" (%s): field \"%s\" (%" DEC ") includes from type \"%s\" whose fields have not yet been processed!", utype.name(), utype.id().print(), utype._fields[ifld].name(), ifld, _types[included_from_itype]->name());
                }
                inc_buf.printf(0, "$(%s_", utype._fields[ifld].name());
                inc_buf.add0(2);
                idx inc_buf_name_pos = inc_buf.length();
                for(idx iinc = 0; iinc < _types[included_from_itype]->_fields.dim(); iinc++) {
                    const sUsrLoadingTypeField * inc_src_fld = _types[included_from_itype]->_fields.ptr(iinc);
                    if( !inc_src_fld->_is_broken && inc_src_fld->name()[0] != '_' ) {
                        LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%" DEC ") includes field \"%s\" from type \"%s\"", utype.name(), utype.id().print(), utype._fields[ifld].name(), ifld, inc_src_fld->name(),
                            _types[inc_src_fld->_definer_itype]->name());
                        inc_buf.printf(inc_buf_name_pos, "%s_%s", utype._fields[ifld].name(), inc_src_fld->name());
                        inc_buf.add0(2);

                        const char * canon_inc_name = canonicalCase(case_buf, inc_buf.ptr(inc_buf_name_pos));
                        if( utype._name2ifield.get(canon_inc_name) ) {
                            LOG_WARNING("Type \"%s\" (%s) cannot add included field \"%s\" from type \"%s\" (%s) because field \"%s\" already exists in the type (via explicit definition or inheritance)", utype.name(), utype.id().print(), inc_src_fld->name(), _types[included_from_itype]->name(), _types[included_from_itype]->id().print(local_log_buf), inc_buf.ptr(inc_buf_name_pos));
                            continue;
                        }

                        idx iinc_dst = utype._fields.dim();
                        sUsrLoadingTypeField * inc_dst_fld = utype._fields.addM(1);
                        new (inc_dst_fld) sUsrLoadingTypeField;
                        *utype._name2ifield.setString(canon_inc_name) = iinc_dst;
                        canon_inc_name = 0;

                        *inc_dst_fld = *inc_src_fld;
                        inc_dst_fld->_index = iinc_dst;
                        inc_dst_fld->_pos_name = sUsrLoadingTypeField::setString(inc_buf.ptr(inc_buf_name_pos));
                        inc_dst_fld->_owner_itype = utype._itype;

                        subst_buf.cut(0);
                        sUsrLoadingTypeField::replaceString(inc_dst_fld->_pos_brief, "$(" __, inc_buf.ptr(0), subst_buf);
                        sUsrLoadingTypeField::replaceString(inc_dst_fld->_pos_constraint_data, "$(" __, inc_buf.ptr(0), subst_buf);
                        sUsrLoadingTypeField::replaceString(inc_dst_fld->_pos_default_value, "$(" __, inc_buf.ptr(0), subst_buf);

                        if( sLen(sUsrLoadingTypeField::getString(inc_dst_fld->_pos_parent_name)) ) {
                            idx inc_buf_par_name_pos = inc_buf.length();
                            inc_buf.printf("%s_%s", utype._fields[ifld].name(), sUsrLoadingTypeField::getString(inc_dst_fld->_pos_parent_name));
                            inc_dst_fld->_pos_parent_name = sUsrLoadingTypeField::setString(inc_buf.ptr(inc_buf_par_name_pos));
                        } else {
                            inc_dst_fld->_pos_parent_name = utype._fields[ifld]._pos_name;
                        }
                        LOG_TRACE("Type \"%s\" (%s): create included field \"%s\" with parent \"%s\"", utype.name(), utype.id().print(), inc_dst_fld->name(), sUsrLoadingTypeField::getString(inc_dst_fld->_pos_parent_name));
                    }
                }
            }
        }
        utype._dim_included_fields = utype._fields.dim() - (utype._dim_explicit_fields + utype._dim_inherited_fields);

        broken_ifld_dic.empty();
        children_param.empty();

        for(idx i=0; i<sDim(system_fields); i++) {
            idx ifield = utype._fields.dim();
            *utype._name2ifield.setString(system_fields[i].name) = ifield;
            sUsrLoadingTypeField * fld = utype._fields.addM(1);
            new(fld) sUsrLoadingTypeField;
            if( it == 0 ) {
                system_fields[i].pos_name = sUsrLoadingTypeField::setString(system_fields[i].name);
                system_fields[i].pos_title = sUsrLoadingTypeField::setString(system_fields[i].title);
            }
            fld->_owner_itype = utype._itype;
            fld->_definer_itype = -1;
            fld->_pos_name = fld->_pos_orig_name = system_fields[i].pos_name;
            fld->_pos_title = system_fields[i].pos_title;
            fld->_type = system_fields[i].type;
            fld->_role = system_fields[i].role;
            fld->_is_multi = system_fields[i].is_multi;
            fld->_is_weak_reference = system_fields[i].is_weak_reference;
            fld->_is_sysinternal = system_fields[i].is_sysinternal;
            fld->_is_hidden = true;
            fld->_is_virtual = true;
            fld->_readonly = system_fields[i].readonly;
        }
        LOG_TRACE("Type \"%s\" (%s) added default system fields", utype.name(), utype.id().print());

        idx cnt_non_array_row = utype._fields.dim();
        for(idx ifld=0; ifld<cnt_non_array_row; ifld++) {
            sUsrLoadingTypeField * fld = utype._fields.ptr(ifld);
            const char * fld_parent_name = sUsrLoadingTypeField::getString(fld->_pos_parent_name);
            if( sLen(fld_parent_name) ) {
                if( const idx * pifld_par = utype._name2ifield.get(canonicalCase(case_buf, fld_parent_name)) ) {
                    const idx ifld_par = *pifld_par;
                    pifld_par = 0;
                    sUsrLoadingTypeField * par_fld = utype._fields.ptr(ifld_par);
                    switch( par_fld->type() ) {
                        case sUsrTypeField::eList:
                        case sUsrTypeField::eListTab:
                            fld->_parent = ifld_par;
                            children_param.addChild(ifld_par, ifld);
                            break;
                        case sUsrTypeField::eArray:
                        case sUsrTypeField::eArrayTab:
                        {
                            array_row_name.cut(0);
                            idx ifld_arr_row = utype.ensureArrayFieldRow(par_fld, array_row_name, case_buf);

                            fld = utype._fields.ptr(ifld);
                            par_fld = utype._fields.ptr(ifld_par);

                            if( !children_param.hasChildren(ifld_par) ) {
                                children_param.addChild(ifld_par, ifld_arr_row);
                            }
                            fld->_parent = ifld_arr_row;
                            children_param.addChild(ifld_arr_row, ifld);
                            break;
                        }
                        default:
                            LOG_ERROR("Non-list, non-array parent \"%s\" in field \"%s\" in type \"%s\" (%s)", fld_parent_name, fld->name(), utype.name(), utype.id().print());
                            *broken_ifld_dic.set(&ifld, sizeof(ifld)) = 0;
                    }
                } else {
                    LOG_ERROR("Unknown parent \"%s\" in field \"%s\" in type \"%s\" (%s)", fld_parent_name, fld->name(), utype.name(), utype.id().print());
                    *broken_ifld_dic.set(&ifld, sizeof(ifld)) = 0;
                }
            } else {
                *utype._root_ifields.add(1) = ifld;
                LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%" DEC ") is a root field", utype.name(), utype.id().print(), utype._fields[ifld].name(), ifld);
            }
        }
        sSort::sortSimpleCallback<idx>(&loadingFldCompare, utype._fields.ptr(), utype._root_ifields.dim(), utype._root_ifields.ptr());

        for(idx irif=0; irif<utype._root_ifields.dim(); irif++) {
            utype.setFieldChildren(utype._root_ifields[irif], utype._root_ifields[irif], &children_param, case_buf);
        }

        for(idx ifld=0; ifld<utype._fields.dim(); ifld++) {
            const sUsrTypeField * fld = utype._fields.ptr(ifld);
            if( !children_param.done_dic.get(&ifld, sizeof(ifld)) ) {
                LOG_ERROR("Type \"%s\" (%s): field \"%s\" does not descend from any root field, seems to be orphan or in parentage loop", utype.name(), utype.id().print(), fld->name());
                *broken_ifld_dic.set(&ifld, sizeof(ifld)) = 0;
            }
        }

        for(idx ibrk=0; ibrk<broken_ifld_dic.dim(); ibrk++) {
            idx ifld = *static_cast<const idx *>(broken_ifld_dic.id(ibrk));
            utype.recurseField(ifld, broken_ifld_dic);
        }
        if( broken_ifld_dic.dim() ) {
            sDic<idx> good_name2ifield;
            for(idx in=0; in<utype._name2ifield.dim(); in++) {
                const char * name = static_cast<const char*>(utype._name2ifield.id(in));
                idx ifld = *utype._name2ifield.ptr(in);
                if( broken_ifld_dic.get(&ifld, sizeof(ifld)) ) {
                    utype._fields[ifld]._is_broken = true;
                } else {
                    *good_name2ifield.setString(name) = ifld;
                }
            }
            utype._name2ifield.empty();
            utype._name2ifield.mex()->swap(good_name2ifield.mex());
        }

        utype._is_fetched = true;
    }
}

void sUsrLoadingType::inheritField(const sUsrLoadingTypeField * inh_fld, sDic<idx> & overridden, sStr & case_buf)
{
    if( !inh_fld ) {
        return;
    }
    if( !inh_fld->isArrayRow() && inh_fld->name()[0] != '_' ) {
        const char * inh_fld_name = canonicalCase(case_buf, inh_fld->name());
        if( const idx * pifld = _name2ifield.get(inh_fld_name) ) {
            if( *pifld < _dim_explicit_fields ) {
                if( overridden.get(inh_fld_name) ) {
                    LOG_WARNING("Type \"%s\" (%s) overrides field \"%s\" inherited from multiple parents; keeping only the first", name(), id().print(), inh_fld->name());
                    return;
                }
                LOG_TRACE("Type \"%s\" (%s) overrides inherited field \"%s\"", name(), id().print(), inh_fld->name());
                overridden.setString(inh_fld_name);
                const sUsrTypeField * fld = _fields.ptr(*pifld);
                if( fld->type() != sUsrTypeField::eArray && fld->type() != sUsrTypeField::eList &&
                    fld->type() != sUsrTypeField::eArrayTab && fld->type() != sUsrTypeField::eListTab ) {
                    return;
                }
            } else {
                if( _fields.ptr(*pifld)->_definer_itype == inh_fld->_definer_itype ) {
                    LOG_TRACE("Type \"%s\" (%s) inherits field \"%s\" from multiple parents, both descending from the same type; keeping only one copy", name(), id().print(), inh_fld->name());
                } else {
                    LOG_WARNING("Type \"%s\" (%s) inherits field \"%s\" from multiple parents; keeping only the first", name(), id().print(), inh_fld->name());
                }
                return;
            }
        } else {
            idx idst = _fields.dim();
            sUsrLoadingTypeField * dst_fld = _fields.addM(1);
            new(dst_fld) sUsrLoadingTypeField;
            *_name2ifield.setString(inh_fld_name) = idst;

            LOG_TRACE("Type \"%s\" (%s) inherits field \"%s\" from \"%s\" (%s)", name(), id().print(), inh_fld->name(), _types[inh_fld->_owner_itype]->name(), _types[inh_fld->_owner_itype]->id().print());

            *dst_fld = *inh_fld;
            dst_fld->_index = idst;
            dst_fld->_owner_itype = _itype;
        }
    }

    for(idx ic=0; ic<inh_fld->dimChildren(); ic++) {
        inheritField(inh_fld->getChild(ic), overridden, case_buf);
    }
}

idx sUsrLoadingType::ensureArrayFieldRow(sUsrLoadingTypeField * fld, sStr & name_buf, sStr & case_buf)
{
    if( fld->type() != sUsrTypeField::eArray && fld->type() != sUsrTypeField::eArrayTab ) {
        return -sIdxMax;
    }

    name_buf.cutAddString(0, "_row_");
    name_buf.addString(fld->name());
    if( const idx * pifld_arr_row = _name2ifield.get(canonicalCase(case_buf, name_buf.ptr())) ) {
        return *pifld_arr_row;
    }

    idx ifld = fld->_index;
    idx irow = _fields.dim();
    *_name2ifield.setString(canonicalCase(case_buf, name_buf.ptr())) = irow;
    sUsrLoadingTypeField * fld_row = _fields.addM(1);
    new(fld_row) sUsrLoadingTypeField;
    fld = _fields.ptr(ifld);

    fld_row->_index = irow;
    fld_row->_is_array_row = true;
    fld_row->_owner_itype = fld->_owner_itype;
    fld_row->_definer_itype = fld->_definer_itype;
    fld_row->_included_from_itype = fld->_included_from_itype;

    fld_row->_pos_name = sUsrLoadingTypeField::setString(name_buf.ptr());
    if( sIs(fld->name(), fld->originalName()) ) {
        fld_row->_pos_orig_name = fld_row->_pos_name;
    } else {
        name_buf.add0();
        idx l = name_buf.length();
        name_buf.addString("_row_");
        name_buf.addString(fld->originalName());
        fld_row->_pos_orig_name = sUsrLoadingTypeField::setString(name_buf.ptr(l));
    }

    name_buf.add0();
    idx l = name_buf.length();
    name_buf.addString("Array row for ");
    name_buf.addString(fld->title());
    fld_row->_pos_title = sUsrLoadingTypeField::setString(name_buf.ptr(l));

    fld_row->_type = sUsrTypeField::eList;
    fld_row->_pos_parent_name = fld->_pos_name;
    fld_row->_parent = ifld;
    fld_row->_readonly = sUsrTypeField::eReadOnly;
    fld_row->_is_hidden = false;
    fld_row->_is_virtual = true;
    fld_row->_is_multi = true;

    LOG_TRACE("Type \"%s\" (%s) added array row field \"%s\" for \"%s\"", name(), id().print(), fld_row->name(), fld->name());

    return irow;
}

void sUsrLoadingType::setFieldChildren(idx ifld, idx iroot_fld, SetFieldChildrenParam * param, sStr & case_buf)
{
    param->done_dic.set(&ifld, sizeof(idx));
    LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%" DEC ") descends from root field \"%s\" (%" DEC ")", name(), id().print(), _fields[ifld].name(), ifld, _fields[iroot_fld].name(), iroot_fld);
    sUsrLoadingTypeField * fld = _fields.ptr(ifld);
    if( const sUsrLoadingTypeField * par = fld->parent() ) {
        fld->_ancestor_count = par->_ancestor_count + 1;
        fld->_is_global_multi = fld->_is_multi || fld->_is_array_row || par->_is_global_multi;
    } else {
        fld->_ancestor_count = 0;
        fld->_is_global_multi = fld->_is_multi || fld->_is_array_row;
    }

    sLst<idx> * children = param->children_dic.get(&ifld, sizeof(idx));
    if( children && children->dim() ) {
        fld->_dim_children = children->dim();
        fld->_start_children = _child_ifields.dim();
        _child_ifields.add(fld->_dim_children + 1);
        for(idx i=0; i<fld->_dim_children; i++) {
            idx ichild = *children->ptr(i);
            LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%" DEC "): packing child field \"%s\" (%" DEC ")", name(), id().print(), fld->name(), ifld, _fields.ptr(ichild)->name(), ichild);
            _child_ifields[fld->_start_children + i] = ichild;
            setFieldChildren(ichild, iroot_fld, param, case_buf);
        }
        _child_ifields[fld->_start_children + fld->_dim_children] = -sIdxMax;
        sSort::sortSimpleCallback<idx>(loadingFldCompare, _fields.ptr(), fld->_dim_children, _child_ifields.ptr(fld->_start_children));
    }
}

void sUsrLoadingType::recurseField(idx ifld, sDic<idx> & seen_dic)
{
    idx * pseen = seen_dic.get(&ifld, sizeof(ifld));
    if( pseen && *pseen ) {
        return;
    }
    *seen_dic.set(&ifld, sizeof(ifld)) = 1;
    const sUsrLoadingTypeField * fld = _fields.ptr(ifld);
    for(idx i=0; i<fld->dimChildren(); i++) {
        recurseField(fld->getChild(i)->_index, seen_dic);
    }
}

void sUsrLoadingType::recurseDescendents(sDic<idx> & seen_dic) const
{
    idx * pseen = seen_dic.get(&_itype, sizeof(_itype));
    if( pseen && *pseen ) {
        return;
    }
    *seen_dic.set(&_itype, sizeof(_itype)) = 1;
    for(idx i=0; i<dimChildren(); i++) {
        getChild(i)->recurseDescendents(seen_dic);
    }
}

bool sUsrLoadingType::PerUserRes::ensureUserForWriting(udx user_id)
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        if( *pilevel != _name2ires.dimStack() - 1 ) {
            return false;
        }
        return true;
    }
    idx ilevel = _usr2ticlevel.dim();
    *_usr2ticlevel.set(&user_id, sizeof(user_id)) = ilevel;
    if( ilevel > 0 ) {
        _name2ires.push();
        assert(ilevel == _name2ires.dimStack() - 1);
    }
    return true;
}

idx sUsrLoadingType::PerUserRes::dimForUser(udx user_id) const
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        return _name2ires.dimLevel(*pilevel);
    }
    return 0;
}

idx sUsrLoadingType::PerUserRes::getILevel(udx user_id) const
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        return *pilevel;
    }
    return -sIdxMax;
}

idx sUsrLoadingType::PerUserRes::getIRes(udx user_id, const char * name, idx name_len) const
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        if( const idx * pires = _name2ires.get(*pilevel, name, name_len) ) {
            return *pires;
        }
    }
    return -sIdxMax;
}

idx sUsrLoadingType::PerUserRes::getIRes(udx user_id, idx index) const
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        if( const idx * pires = _name2ires.ptr(*pilevel, index) ) {
            return *pires;
        }
    }
    return -sIdxMax;
}

struct LoadActionViewFinderVal
{
    idx ires;
    idx utype_depth;

    LoadActionViewFinderVal()
    {
        ires = -sIdxMax;
        utype_depth = 0;
    }
};

struct LoadActionViewFinderParam
{
    udx user_id;
    sUsrLoadingAction * act;
    sUsrLoadingView * view;
    sUsrLoadingJSComponent * jsco;

    sStr buf;
    sDic<LoadActionViewFinderVal> typeactview2val;
};

void sUsrLoadingType::loadActionsViewsFinder(const sUsrType2 * utype_, const sUsrType2 * recurse_start, idx depth_from_start, void * param_)
{
    sUsrLoadingType * utype = dynamic_cast<sUsrLoadingType*>(const_cast<sUsrType2*>(utype_));
    LoadActionViewFinderParam * param = static_cast<LoadActionViewFinderParam*>(param_);

    const char * name = param->act ? param->act->name() : (param->view ? param->view->name() : param->jsco->name());
    idx ires = param->act ? param->act->_iaction : (param->view ? param->view->_iview : param->jsco->_ijsco);

    param->buf.cut0cut();
    utype->id().print(param->buf);
    param->buf.add0();
    param->buf.addNum(param->user_id);
    param->buf.add0();
    param->buf.addString(name);
    param->buf.add0(2);

    LoadActionViewFinderVal * val = param->typeactview2val.set(param->buf.ptr(), param->buf.length());

    PerUserRes & per_user_res = param->act ? utype->_actions : (param->view ? utype->_views : utype->_jscos);
    per_user_res.ensureUserForWriting(param->user_id);


    if( val->ires >= 0 ) {
        if( val->utype_depth > depth_from_start ) {
            val->ires = ires;
            *per_user_res._name2ires.setString(name) = ires;
            val->utype_depth = depth_from_start;
        }
    } else {
        val->ires = ires;
        *per_user_res._name2ires.setString(name) = ires;
        val->utype_depth = depth_from_start;
    }
}

void sUsrLoadingType::loadActions(const sUsr & user)
{
    udx user_id = user.Id();
    if( sUsrLoadingAction::_usr2actres.get(&user_id, sizeof(user_id)) ) {
        return;
    }

    sUsrLoadingAction::ActRes * act_res = sUsrLoadingAction::_usr2actres.set(&user_id, sizeof(user_id));
    user.objs2("^action$", act_res->res);
    act_res->acts.init(sMex::fExactSize);
    act_res->acts.resize(act_res->res.dim());
    LoadActionViewFinderParam param;
    param.user_id = user_id;
    param.jsco = 0;
    param.view = 0;
    idx iact = 0;
    for(sUsrObjRes::IdIter it = act_res->res.first(); act_res->res.has(it); act_res->res.next(it), iact++) {
        sUsrLoadingAction * act = act_res->acts.ptr(iact);
        act->_id = *act_res->res.id(it);
        act->_user_id = user_id;
        act->_iaction = iact;
        const sUsrObjRes::TObjProp * props = act_res->res.get(it);
        const sUsrObjRes::TPropTbl * type_name_tbl = act_res->res.get(*props, "type_name");
        const char * type_name = act_res->res.getValue(type_name_tbl);
        param.act = act;
        findRaw(&user, 0, type_name, loadActionsViewsFinder, &param, true, true);
    }
}

void sUsrLoadingType::loadJSComponents(const sUsr & user)
{
    udx user_id = user.Id();
    if( sUsrLoadingJSComponent::_usr2jscores.get(&user_id, sizeof(user_id)) ) {
        return;
    }

    sUsrLoadingJSComponent::JscoRes * res = sUsrLoadingJSComponent::_usr2jscores.set(&user_id, sizeof(user_id));
    user.objs2("^js_component$", res->res);
    res->jscos.init(sMex::fExactSize);
    res->jscos.resize(res->res.dim());
    LoadActionViewFinderParam param;
    param.user_id = user_id;
    param.act = 0;
    param.view = 0;
    idx i = 0;
    for(sUsrObjRes::IdIter it = res->res.first(); res->res.has(it); res->res.next(it), ++i) {
        sUsrLoadingJSComponent * c = res->jscos.ptr(i);
        c->_id = *res->res.id(it);
        c->_user_id = user_id;
        c->_ijsco = i;
        const sUsrObjRes::TObjProp * props = res->res.get(it);
        const sUsrObjRes::TPropTbl * type_name_tbl = res->res.get(*props, "type_name");
        const char * type_name = res->res.getValue(type_name_tbl);
        param.jsco = c;
        findRaw(&user, 0, type_name, loadActionsViewsFinder, &param, true, true);
    }
}

void sUsrLoadingType::loadViews(const sUsr & user)
{
    udx user_id = user.Id();
    if( sUsrLoadingView::_usr2viewres.get(&user_id, sizeof(user_id)) ) {
        return;
    }

    const sUsrType2 * view_type = ensure(user, "view");
    sUsrLoadingView::ViewRes * view_res = sUsrLoadingView::_usr2viewres.set(&user_id, sizeof(user_id));
    user.objs2("^view$", view_res->res);
    view_res->views.init(sMex::fExactSize);
    view_res->views.resize(view_res->res.dim());
    LoadActionViewFinderParam param;
    param.user_id = user_id;
    param.act = 0;
    param.jsco = 0;
    sVarSet tree_table;
    idx iview = 0;
    for(sUsrObjRes::IdIter it = view_res->res.first(); view_res->res.has(it); view_res->res.next(it), iview++) {
        sUsrLoadingView * view = view_res->views.ptr(iview);
        view->_id = *view_res->res.id(it);
        view->_user_id = user_id;
        view->_iview = iview;
        view->_fields.init(&sUsrLoadingView::_fields_buf);

        const sUsrObjRes::TObjProp * props = view_res->res.get(it);
        const sUsrObjRes::TPropTbl * type_name_tbl = view_res->res.get(*props, "type_name");
        const char * type_name = view_res->res.getValue(type_name_tbl);
        param.view = view;
        findRaw(&user, 0, type_name, loadActionsViewsFinder, &param, true, true);

        tree_table.empty();
        for(idx p = 0; props && p < props->dim(); ++p) {
            const char * prop_name = (const char *) props->id(p);
            const sUsrObjRes::TPropTbl * tbl = view_res->res.get(*props, prop_name);
            while( tbl ) {
                tree_table.addRow().addCol(view->_id.print()).addCol(prop_name).addCol(view_res->res.getPath(tbl)).addCol(view_res->res.getValue(tbl));
                tbl = view_res->res.getNext(tbl);
            }
        }
        sUsrObjPropsTree tree(user, view_type, tree_table);
        const sUsrObjPropsNode * fields_array = tree.find("fields");
        for(const sUsrObjPropsNode * fields_row = fields_array ? fields_array->firstChild() : 0; fields_row; fields_row = fields_row->nextSibling() ) {
            sUsrLoadingView::Field * fld = view->_fields.add(1);
            if( const char * field_name = fields_row->findValue("field_name") ) {
                *sUsrLoadingView::_names.setString(field_name, 0, &fld->pos_name) = true;
            } else {
                fld->pos_name = -sIdxMax;
            }
            if( const char * field_default = fields_row->findValue("field_default") ) {
                *sUsrLoadingView::_names.setString(field_default, 0, &fld->pos_default_value) = true;
            } else {
                fld->pos_default_value = -sIdxMax;
            }
            if( const char * field_order_string = fields_row->findValue("field_order") ) {
                *sUsrLoadingView::_names.setString(field_order_string, 0, &fld->pos_order_string) = true;
            } else {
                fld->pos_order_string = -sIdxMax;
            }
            fld->readonly = fields_row->findBoolValue("field_readonly");
        }
    }
}


const char * sUsrLoadingAction::name() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "name");
    return res.getValue(tbl);
}

const char * sUsrLoadingAction::title() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "title");
    return res.getValue(tbl);
}

const char * sUsrLoadingAction::description() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "description");
    return res.getValue(tbl);
}

real sUsrLoadingAction::order() const
{
    const char * sord = orderString();
    return sord ? strtod(sord, 0) : 0;
}

const char * sUsrLoadingAction::orderString() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "order");
    return res.getValue(tbl);
}

bool sUsrLoadingAction::isObjAction() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "is_obj_action");
    return sString::parseBool(res.getValue(tbl));
}

udx sUsrLoadingAction::requiredPermission() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "required_permission");
    const char * sval = res.getValue(tbl);
    return sval ? atoudx(sval) : 0;
}

int sUsrAction::cmp(const sUsrAction * rhs) const
{
    if( !rhs ) {
        return 1;
    }
    const char * lhs_sord = orderString();
    if( !lhs_sord) {
        lhs_sord = sStr::zero;
    }
    const char * rhs_sord = rhs->orderString();
    if( !rhs_sord) {
        rhs_sord = sStr::zero;
    }

    if( !sIsExactly(lhs_sord, rhs_sord) ) {
        real diff = order() - rhs->order();
        if( diff < 0 ) {
            return -1;
        } else if( diff > 0 ) {
            return 1;
        }
    }

    const char * lhs_name = name();
    if( !lhs_name) {
        lhs_name = sStr::zero;
    }
    const char * rhs_name = rhs->name();
    if( !rhs_name) {
        rhs_name = sStr::zero;
    }
    return strcasecmp(lhs_name, rhs_name);
}


const char * sUsrLoadingJSComponent::name() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "name");
    return res.getValue(tbl);
}

bool sUsrLoadingJSComponent::isPreview() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "preview");
    return res.getValue(tbl);
}

bool sUsrLoadingJSComponent::isAlgoview() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "algoview");
    return res.getValue(tbl);
}

real sUsrLoadingJSComponent::order() const
{
    const char * sord = orderString();
    return sord ? strtod(sord, 0) : 0;
}

const char * sUsrLoadingJSComponent::orderString() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "order");
    return res.getValue(tbl);
}

int sUsrJSComponent::cmp(const sUsrJSComponent * rhs) const
{
    if( !rhs ) {
        return 1;
    }
    const char * lhs_sord = orderString();
    if( !lhs_sord) {
        lhs_sord = sStr::zero;
    }
    const char * rhs_sord = rhs->orderString();
    if( !rhs_sord) {
        rhs_sord = sStr::zero;
    }

    if( strcmp(lhs_sord, rhs_sord) != 0 ) {
        real diff = order() - rhs->order();
        if( diff < 0 ) {
            return -1;
        } else if( diff > 0 ) {
            return 1;
        }
    }

    const char * lhs_name = name();
    if( !lhs_name) {
        lhs_name = sStr::zero;
    }
    const char * rhs_name = rhs->name();
    if( !rhs_name) {
        rhs_name = sStr::zero;
    }
    return strcasecmp(lhs_name, rhs_name);
}


const char * sUsrLoadingView::name() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "name");
    return res.getValue(tbl);
}

idx sUsrLoadingView::dimFields() const
{
    return _fields.dim();
}

const char * sUsrLoadingView::fieldName(idx ifield) const
{
    idx pos = _fields[ifield].pos_name;
    return pos >= 0 ? static_cast<const char *>(_names.id(pos)) : sStr::zero;
}

const char * sUsrLoadingView::fieldDefaultValue(idx ifield) const
{
    idx pos = _fields[ifield].pos_default_value;
    return pos >= 0 ? static_cast<const char *>(_names.id(pos)) : sStr::zero;
}

const char * sUsrLoadingView::fieldOrderString(idx ifield) const
{
    idx pos = _fields[ifield].pos_order_string;
    return pos >= 0 ? static_cast<const char *>(_names.id(pos)) : sStr::zero;
}

bool sUsrLoadingView::fieldReadonly(idx ifield) const
{
    return _fields[ifield].readonly;
}

int sUsrView::cmp(const sUsrView * rhs) const
{
    if( !rhs ) {
        return 1;
    }
    const char * lhs_name = name();
    if( !lhs_name) {
        lhs_name = sStr::zero;
    }
    const char * rhs_name = rhs->name();
    if( !rhs_name) {
        rhs_name = sStr::zero;
    }
    return strcasecmp(lhs_name, rhs_name);
}


const sUsrType2 * sUsrType2::ensure(const sUsr & user, const char * type_name, idx type_name_len, bool no_prefetch, bool lazy_fetch_fields)
{
    if( const sUsrTypeCache * bincache = getBinCache(&user) ) {
        return bincache->getType(type_name, type_name_len);
    }
    const sUsrLoadingType * ret = sUsrLoadingType::getRaw(type_name, type_name_len);
    if( ret && (lazy_fetch_fields || ret->_is_fetched) ) {
        return ret;
    } else {
        return load(user, type_name, type_name_len, 0, no_prefetch);
    }
}

const sUsrType2 * sUsrType2::ensure(const sUsr & user, const sHiveId & type_id, bool no_prefetch, bool lazy_fetch_fields, bool ephemeral)
{
    const sUsrType2 * ret = 0;
    const sUsrLoadingType * loading_ret = 0;

    if( const sUsrTypeCache * bincache = getBinCache(&user) ) {
        ret = bincache->getType(type_id);
    } else {
        ret = loading_ret = sUsrLoadingType::getRaw(type_id);
        if( !(loading_ret && (lazy_fetch_fields || loading_ret->_is_fetched)) ) {
            ret = load(user, 0, 0, &type_id, no_prefetch);
        }
    }

    if( !ret && ephemeral ) {
        ret = loading_ret = sUsrLoadingType::ensureEphemeral(type_id);
    }

    return ret;
}

const sUsrType2 * sUsrType2::ensureTypeType(const sUsr & user)
{
    const sUsrType2 * ret = ensure(user, "type");
    return ret;
}

const sUsrType2 * sUsrType2::get(const char * type_name, idx type_name_len)
{
    if( const sUsrTypeCache * bincache = getBinCache(0) ) {
        return bincache->getType(type_name, type_name_len);
    }
    return sUsrLoadingType::getRaw(type_name, type_name_len);
}

const sUsrType2 * sUsrType2::get(const sHiveId & type_id)
{
    const sUsrType2 * ret = 0;

    if( const sUsrTypeCache * bincache = getBinCache(0) ) {
        ret = bincache->getType(type_id);
        if( !ret ) {
            ret = sUsrLoadingType::getRawEphemeral(type_id);
        }
    } else {
        ret = sUsrLoadingType::getRaw(type_id);
    }

    return ret;
}

const sUsrType2 * sUsrType2::getTypeType()
{
    const sUsrType2 * ret = get("type");
    return ret;
}

class sUsrType2::FindWorker {
    public:
        struct Entry
        {
            bool added;
            sHiveId utype_id;
            sHiveId recurse_start_id;
            idx recurse_depth;
            Entry()
            {
                added = true;
                recurse_depth = 0;
            }
        };
        sDic<Entry> found_entries;
        idx num_queries;
        idx num_negates;

        FindWorker()
        {
            num_queries = num_negates = 0;
        }

        virtual ~FindWorker()
        {
        }

        void lister(const sUsrType2 * utype, const sHiveId * recurse_start_id, const idx recurse_depth, bool is_negate)
        {
            if( is_negate || !found_entries.get(&utype->id(), sizeof(sHiveId)) ) {
                Entry * e = found_entries.set(&utype->id(), sizeof(sHiveId));
                e->added = !is_negate;
                e->utype_id = utype->id();
                if( recurse_start_id ) {
                    if( recurse_depth < e->recurse_depth ) {
                        e->recurse_start_id = *recurse_start_id;
                        e->recurse_depth = recurse_depth;
                    }
                } else {
                    e->recurse_start_id = utype->id();
                    e->recurse_depth = 0;
                }
            }
            if( recurse_start_id ) {
                idx cnt = utype->dimChildren();
                for(idx ic = 0; ic < cnt; ic++) {
                    lister(utype->getChild(ic), recurse_start_id, recurse_depth + 1, is_negate);
                }
            }
        }

        virtual void postRun(const char * qry, bool no_regexp)
        {
            if( num_queries == num_negates ) {
                if( const sUsrType2 * utype = get("base_user_type") ) {
                    sHiveId base_id = utype->id();
                    lister(utype, &base_id, 0, false);
                }
            }
        }

        virtual void checkQuery(const char * exact_name, regex_t * re, bool is_recurse, bool is_negate)
        {
            if( exact_name ) {
                if( const sUsrType2 * utype = get(exact_name) ) {
                    sHiveId utype_id = utype->id();
                    lister(utype, is_recurse ? &utype_id : 0, 0, is_negate);
                }
            } else if( re ) {
                const idx dim_raw = sUsrType2::dimRaw();
                for(idx itype = 0; itype < dim_raw; itype++) {
                    const sUsrType2 * utype = sUsrType2::getNthRaw(itype);
                    const char * name = utype->name();
                    if( name && regexec(re, name, 0, 0, 0) == 0 ) {
                        sHiveId utype_id = utype->id();
                        lister(utype, is_recurse ? &utype_id : 0, 0, is_negate);
                    }
                }
            }
        }

        void run(const char * qry, bool no_regexp)
        {
            sStr type_name_buf;
            regex_t re;

            num_queries = 0;
            num_negates = 0;
            bool done = false;
            for(idx istart = 0; qry[istart] && !done;) {
                for(; qry[istart] && (isspace(qry[istart]) || qry[istart] == ','); istart++);

                idx iend;
                for(iend = istart; qry[iend] && !isspace(qry[iend]) && qry[iend] != ','; iend++);
                idx inext = iend;
                if( iend > istart ) {
                    num_queries++;
                    bool is_negate = qry[istart] == '!';
                    if( is_negate ) {
                        istart++;
                        num_negates++;
                    }
                    bool is_recurse = qry[iend - 1] == '+';
                    if( is_recurse ) {
                        iend--;
                    }
                    bool is_exact = false;
                    if( no_regexp ) {
                        is_exact = true;
                    } else {
                        is_exact = qry[istart] == '^' && qry[iend - 1] == '$';
                        if( is_exact ) {
                            istart++;
                            iend--;
                        }
                    }

                    type_name_buf.cutAddString(0, qry + istart, iend - istart);
                    if( is_exact ) {
                        checkQuery(type_name_buf.ptr(), 0, is_recurse, is_negate);
                    } else if( regcomp(&re, type_name_buf.ptr(), REG_EXTENDED | REG_NOSUB | REG_ICASE) == 0 ) {
                        checkQuery(0, &re, is_recurse, is_negate);
                        regfree(&re);
                    }
                }
                istart = inext;
            }

            postRun(qry, no_regexp);
        }
};

static bool is_potential_type_name(const char * s, idx len)
{
    if( !s || !len ) {
        return false;
    }

    if( !isalpha(s[0]) ) {
        return false;
    }
    for(idx i = 1; i < len && s[i]; i++) {
        if( !isalnum(s[i]) && s[i] != '_' && s[i] != '-' ) {
            return false;
        }
    }
    return true;
}

idx sUsrType2::find(const sUsr & user, sVec<const sUsrType2 *> * out, const char * qry, FindCallback cb, void * cb_param, bool manual_ensure, bool lazy_fetch_fields)
{
    if( !manual_ensure ) {
        idx qry_len = sLen(qry);
        if( is_potential_type_name(qry, qry_len) ) {
            ensure(user, qry, qry_len);
        } else if( qry && qry[0] == '^' && qry[qry_len - 1] == '$' && is_potential_type_name(qry + 1, qry_len - 2) ) {
            ensure(user, qry + 1, qry_len - 2);
        } else {
            if( !getBinCache(&user) ) {
                load(user, "*", 1, 0);
            }
        }
    }
    return findRaw(manual_ensure ? 0 : &user, out, qry, cb, cb_param, false, lazy_fetch_fields);
}

idx sUsrType2::dimRaw()
{
    idx cnt = 0;
    if( const sUsrTypeCache * bincache = getBinCache(0) ) {
        cnt += bincache->dimTypes();
    }
    cnt += sUsrLoadingType::_types.dim();
    return cnt;
}

const sUsrType2 * sUsrType2::getNthRaw(idx itype)
{
    idx dim_cached = 0;
    if( const sUsrTypeCache * bincache = getBinCache(0) ) {
        dim_cached = bincache->dimTypes();
        if( itype < dim_cached ) {
            return bincache->getType(itype);
        }
    }
    return sUsrLoadingType::_types[itype - dim_cached];
}

idx sUsrType2::findRaw(const sUsr * ensurer_usr, sVec<const sUsrType2 *> * out, const char * qry, sUsrType2::FindCallback cb, void * cb_param, bool no_regexp, bool lazy_fetch_fields)
{
    if( !qry ) {
        qry = "base_user_type+";
        no_regexp = true;
    }

    FindWorker worker;
    worker.run(qry, no_regexp);

    idx num_added = 0;
    for(idx ia=0; ia<worker.found_entries.dim(); ia++) {
        const FindWorker::Entry * e = worker.found_entries.ptr(ia);
        if( e->added ) {
            const sUsrType2 * utype = ensurer_usr ? ensure(*ensurer_usr, e->utype_id, false, lazy_fetch_fields) : get(e->utype_id);
            if( out ) {
                *out->add(1) = utype;
            }
            if( cb ) {
                cb(utype, ensurer_usr ? ensure(*ensurer_usr, e->recurse_start_id, false, lazy_fetch_fields) : get(e->recurse_start_id), e->recurse_depth, cb_param);
            }
        }
    }

    return num_added;
}

idx sUsrType2::getRootTypes(const sUsr & user, sVec<const sUsrType2 *> & out)
{
    idx start_pos = out.dim();

    if( const sUsrTypeCache * bincache = getBinCache(&user) ) {
        idx cnt = bincache->dimRootTypes();
        for(idx i = 0; i < cnt; i++) {
            out[start_pos + i] = bincache->getRootType(i);
        }
        return cnt;
    }

    load(user, "*", 1, 0);

    out.add(sUsrLoadingType::_roots.dim());
    for(idx i=0; i<sUsrLoadingType::_roots.dim(); i++) {
        out[start_pos + i] = sUsrLoadingType::_roots[i];
    }
    return sUsrLoadingType::_roots.dim();
}

bool sUsrLoadingType::isUser() const
{
    if( _is_user == eLazyNotLoaded ) {
        sDic<idx> seen_dic;
        return isUser(seen_dic);
    }

    return (_is_user == eLazyTrue);
}

bool sUsrLoadingType::isUser(sDic<idx> & seen_dic) const
{
    if( _is_user == eLazyNotLoaded ) {
        if( seen_dic.get(&id(), sizeof(sHiveId)) ) {
            LOG_ERROR("Type \"%s\" (%s) is in a parentage loop; failed determine if it descends from base_user_type", name(), id().print());
            _is_broken = true;
            return false;
        }
        *seen_dic.set(&id(), sizeof(sHiveId)) = 1;

        if( sIsExactly(name(), "base_user_type" ) ) {
            _is_user = eLazyTrue;
        } else {
            _is_user = eLazyFalse;
            for(idx ip = 0; ip < dimParents(); ip++) {
                if( getParent(ip)->isUser(seen_dic) ) {
                    _is_user = eLazyTrue;
                    break;
                }
            }
        }
    }

    return (_is_user == eLazyTrue);
}

bool sUsrLoadingType::isSystem() const
{
    if( _is_system == eLazyNotLoaded ) {
        sDic<idx> seen_dic;
        return isSystem(seen_dic);
    }

    return (_is_system == eLazyTrue);
}

bool sUsrLoadingType::isSystem(sDic<idx> & seen_dic) const
{
    if( _is_system == eLazyNotLoaded ) {
        if( seen_dic.get(&id(), sizeof(sHiveId)) ) {
            LOG_ERROR("Type \"%s\" (%s) is in a parentage loop; failed determine if it descends from base_system_type", name(), id().print());
            _is_broken = true;
            return false;
        }
        *seen_dic.set(&id(), sizeof(sHiveId)) = 1;

        if( sIsExactly(name(), "base_system_type" ) ) {
            _is_system = eLazyTrue;
        } else {
            _is_system = eLazyFalse;
            for(idx ip = 0; ip < dimParents(); ip++) {
                if( getParent(ip)->isSystem(seen_dic) ) {
                    _is_system = eLazyTrue;
                    break;
                }
            }
        }
    }

    return (_is_system == eLazyTrue);
}

class sUsrType2::NameMatchWorker : public sUsrType2::FindWorker {
    public:
        const sUsrType2 * topic;
        idx num_pos_matches;
        idx num_neg_matches;

        NameMatchWorker(const sUsrType2 * topic_)
        {
            topic = topic_;
            num_pos_matches = num_neg_matches = 0;
        }
        virtual ~NameMatchWorker() {}

        void ensureAncestors(const sUsrType2 * utype, bool force = false)
        {
            if( utype && (force || !found_entries.dim()) ) {
                sHiveId id = utype->id();
                if( !found_entries.get(&id, sizeof(id)) ) {
                    Entry * e = found_entries.set(&id, sizeof(id));
                    e->utype_id = id;
                    idx cnt = utype->dimParents();
                    for(idx ip = 0; ip < cnt; ip++) {
                        ensureAncestors(utype->getParent(ip), true);
                    }
                }
            }
        }

        virtual void postRun(const char * qry, bool no_regexp) {}

        bool checkQuery_nonrecursive(const sUsrType2 * utype, const char * name, regex_t * re, bool is_negate)
        {
            const char * utype_name = utype ? utype->name() : 0;
            if( utype_name ) {
                if( name && strcasecmp(name, utype_name) == 0 ) {
                    if( is_negate ) {
                        num_neg_matches++;
                    } else {
                        num_pos_matches++;
                    }
                    return true;
                } else if( re && regexec(re, utype_name, 0, 0, 0) == 0 ) {
                    if( is_negate ) {
                        num_neg_matches++;
                    } else {
                        num_pos_matches++;
                    }
                    return true;
                }
            }
            return false;
        }

        virtual void checkQuery(const char * exact_name, regex_t * re, bool is_recurse, bool is_negate)
        {
            if( is_recurse ) {
                ensureAncestors(topic, false);
                for(idx i = 0; i < found_entries.dim(); i++) {
                    Entry * e = found_entries.ptr(i);
                    if( checkQuery_nonrecursive(sUsrType2::get(e->utype_id), exact_name, re, is_negate) ) {
                        break;
                    }
                }
            } else {
                checkQuery_nonrecursive(topic, exact_name, re, is_negate);
            }
        }
};

bool sUsrType2::nameMatch(const char * qry) const
{
    if( !qry ) {
        return false;
    }

    NameMatchWorker worker(this);
    worker.run(qry, false);
    if( worker.num_negates > 0 && worker.num_negates == worker.num_queries ) {
        return !worker.num_neg_matches;
    } else {
        return worker.num_pos_matches > 0 && !worker.num_neg_matches;
    }
}

idx sUsrLoadingType::getParents(sVec<const sUsrType2*> & out) const
{
    idx cnt = dimParents();
    idx start_dim = out.dim();
    out.resize(start_dim + cnt);
    for(idx i=0; i<cnt; i++) {
        out[start_dim + i] = _types[_parents[i]];
    }
    return cnt;
}

idx sUsrLoadingType::getChildren(sVec<const sUsrType2*> & out) const
{
    idx cnt = dimChildren();
    idx start_dim = out.dim();
    out.resize(start_dim + cnt);
    for(idx i=0; i<cnt; i++) {
        out[start_dim + i] = _types[_children[i]];
    }
    return cnt;
}

idx sUsrLoadingType::getIncludes(sVec<const sUsrType2*> & out) const
{
    idx cnt = dimIncludes();
    for(idx i=0; i<cnt; i++) {
        *out.add(1) = _types[_includes[i]];
    }
    return cnt;
}

bool sUsrType2::isDescendentOf(const sUsrType2 * rhs) const
{
    if( !rhs ) {
        return false;
    }
    if( id() == rhs->id() ) {
        return true;
    }

    idx cnt = dimParents();
    for(idx i=0; i<cnt; i++) {
        if( getParent(i)->isDescendentOf(rhs) ) {
            return true;
        }
    }
    return false;
}

const sUsrLoadingTypeField* sUsrLoadingType::getField(const sUsr & user, const char * field_name, idx field_name_len) const
{
    if( !_is_fetched ) {
        load(user, 0, 0, &_id);
    }

    sStr case_buf;
    if( const idx * pifield = field_name ? _name2ifield.get(canonicalCase(case_buf, field_name, field_name_len), field_name_len) : 0 ) {
        return _fields.ptr(*pifield);
    }
    return 0;
}

idx sUsrType2::getFields(const sUsr & user, sVec<const sUsrTypeField*> & out) const
{
    ensure(user, id());

    idx start_dim = out.dim();
    idx cnt = dimFields(user);
    out.resize(start_dim + cnt);
    for(idx i=0; i<cnt; i++) {
        out[start_dim + i] = getField(user, i);
    }
    return cnt;
}

static idx fldDicCompare(void * param, void * arr, idx i1, idx i2)
{
    const sDic<const sUsrTypeField*> * fields = static_cast<const sDic<const sUsrTypeField*>*>(param);
    const idx * indices = static_cast<const idx*>(arr);
    const sUsrTypeField * fld1 = *fields->ptr(indices[i1]);
    const sUsrTypeField * fld2 = *fields->ptr(indices[i2]);
    idx diff = fld1->cmp(fld2);
    return diff;
}

idx sUsrType2::findFields(const sUsr & user, sVec<const sUsrTypeField*> & out, const char * filter00) const
{
    ensure(user, id());

    bool with_brief = false, with_summary = false, with_all = !filter00;
    sDic<const sUsrTypeField*> fields;
    for(const char * f = filter00; f; f = sString::next00(f)) {
        if( strcasecmp(f, "_brief") == 0 ) {
            with_brief = true;
        } else if( strcasecmp(f, "_summary") == 0 ) {
            with_summary = true;
        } else if( strcasecmp(f, "_all") == 0 ) {
            with_all = true;
        } else if( const sUsrTypeField * fld = getField(user, f) ) {
            *fields.set(&fld, sizeof(fld)) = fld;
        }
    }

    if( with_all || with_brief || with_summary ) {
        for(idx ifield = 0; ifield < dimFields(user); ifield++) {
            if( const sUsrTypeField * fld = getField(user, ifield) ) {
                if( (with_all && fld->name()[0] != '_') ||
                    (with_brief && fld->brief()[0]) ||
                    (with_summary && fld->isSummary())
                  )
                {
                    *fields.set(&fld, sizeof(fld)) = fld;
                }
            }
        }
    }

    idx start_dim = out.dim();
    idx cnt = fields.dim();
    sVec<idx> fields_order(sMex::fExactSize);
    fields_order.resize(cnt);
    for(idx i = 0; i < cnt; i++) {
        fields_order[i] = i;
    }
    sSort::sortSimpleCallback<idx>(&fldDicCompare, &fields, fields_order.dim(), fields_order.ptr());
    out.resize(start_dim + cnt);
    for(idx i = 0; i < fields_order.dim(); i++) {
        out[start_dim + i] = *fields.ptr(fields_order[i]);
    }

    return cnt;
}

idx sUsrLoadingType::dimActions(const sUsr & user) const
{
    loadActions(user);
    udx user_id = user.Id();
    return _actions.dimForUser(user_id);
}

const sUsrLoadingAction * sUsrLoadingType::getAction(const sUsr & user, const char * act_name, idx act_name_len) const
{
    loadActions(user);
    udx user_id = user.Id();
    idx ires = _actions.getIRes(user_id, act_name, act_name_len);
    if( ires >= 0 ) {
        return sUsrLoadingAction::_usr2actres.get(&user_id, sizeof(user_id))->acts.ptr(ires);
    }
    return 0;
}

const sUsrLoadingAction * sUsrLoadingType::getAction(const sUsr & user, idx iact) const
{
    loadActions(user);
    udx user_id = user.Id();
    idx ires = _actions.getIRes(user_id, iact);
    if( ires >= 0 ) {
        return sUsrLoadingAction::_usr2actres.get(&user_id, sizeof(user_id))->acts.ptr(ires);
    }
    return 0;
}

idx sUsrLoadingType::getActions(const sUsr & user, sVec<const sUsrAction *> & out) const
{
    loadActions(user);
    udx user_id = user.Id();
    idx dim_act = _actions.dimForUser(user_id);

    if( dim_act ) {
        idx start_dim = out.dim();
        idx ilevel = _actions.getILevel(user_id);
        const sUsrLoadingAction::ActRes * act_res = sUsrLoadingAction::_usr2actres.get(&user_id, sizeof(user_id));

        out.add(dim_act);
        for(idx iact = 0; iact < dim_act; iact++) {
            idx ires = *_actions._name2ires.ptr(ilevel, iact);
            out[start_dim + iact] = act_res->acts.ptr(ires);
        }
    }
    return dim_act;
}

idx sUsrLoadingType::dimJSComponents(const sUsr & user) const
{
    loadJSComponents(user);
    udx user_id = user.Id();
    return _jscos.dimForUser(user_id);
}

const sUsrLoadingJSComponent * sUsrLoadingType::getJSComponent(const sUsr & user, const char * name, idx name_len) const
{
    loadJSComponents(user);
    udx user_id = user.Id();
    idx ires = _jscos.getIRes(user_id, name, name_len);
    if( ires >= 0 ) {
        return sUsrLoadingJSComponent::_usr2jscores.get(&user_id, sizeof(user_id))->jscos.ptr(ires);
    }
    return 0;
}

const sUsrLoadingJSComponent * sUsrLoadingType::getJSComponent(const sUsr & user, idx ijsco) const
{
    loadJSComponents(user);
    udx user_id = user.Id();
    idx ires = _jscos.getIRes(user_id, ijsco);
    if( ires >= 0 ) {
        return sUsrLoadingJSComponent::_usr2jscores.get(&user_id, sizeof(user_id))->jscos.ptr(ires);
    }
    return 0;
}

idx sUsrLoadingType::getJSComponents(const sUsr & user, sVec<const sUsrJSComponent *> & out) const
{
    loadJSComponents(user);
    udx user_id = user.Id();
    idx dim_jsco = _jscos.dimForUser(user_id);

    if( dim_jsco ) {
        idx start_dim = out.dim();
        idx ilevel = _jscos.getILevel(user_id);
        const sUsrLoadingJSComponent::JscoRes * jsco_res = sUsrLoadingJSComponent::_usr2jscores.get(&user_id, sizeof(user_id));
        out.add(dim_jsco);
        for(idx i = 0; i < dim_jsco; i++) {
            idx ires = *_jscos._name2ires.ptr(ilevel, i);
            out[start_dim + i] = jsco_res->jscos.ptr(ires);
        }
    }
    return dim_jsco;
}

idx sUsrLoadingType::dimViews(const sUsr & user) const
{
    loadViews(user);
    udx user_id = user.Id();
    return _views.dimForUser(user_id);
}

const sUsrLoadingView * sUsrLoadingType::getView(const sUsr & user, const char * view_name, idx view_name_len) const
{
    loadViews(user);
    udx user_id = user.Id();
    idx ires = _views.getIRes(user_id, view_name, view_name_len);
    if( ires >= 0 ) {
        return sUsrLoadingView::_usr2viewres.get(&user_id, sizeof(user_id))->views.ptr(ires);
    }
    return 0;
}

const sUsrLoadingView * sUsrLoadingType::getView(const sUsr & user, idx iview) const
{
    loadViews(user);
    udx user_id = user.Id();
    idx ires = _views.getIRes(user_id, iview);
    if( ires >= 0 ) {
        return sUsrLoadingView::_usr2viewres.get(&user_id, sizeof(user_id))->views.ptr(ires);
    }
    return 0;
}

idx sUsrLoadingType::getViews(const sUsr & user, sVec<const sUsrView *> & out) const
{
    loadViews(user);
    udx user_id = user.Id();
    idx dim_views = _views.dimForUser(user_id);

    if( dim_views ) {
        idx start_dim = out.dim();
        idx ilevel = _views.getILevel(user_id);
        const sUsrLoadingView::ViewRes * view_res = sUsrLoadingView::_usr2viewres.get(&user_id, sizeof(user_id));

        out.add(dim_views);
        for(idx iview = 0; iview < dim_views; iview++) {
            idx ires = *_views._name2ires.ptr(ilevel, iview);
            out[start_dim + iview] = view_res->views.ptr(ires);
        }
    }
    return dim_views;
}

idx sUsrType2::props(const sUsr & user, sVarSet & list, const char * filter00) const
{
    sVec<const sUsrTypeField *> fields;
    findFields(user, fields, filter00);

    if( list.rows == 0 ) {
        idx icol = 0;
        list.setColId(icol++, "type_id");
        list.setColId(icol++, "name");
        list.setColId(icol++, "title");
        list.setColId(icol++, "type");
        list.setColId(icol++, "parent");
        list.setColId(icol++, "role");
        list.setColId(icol++, "is_key_fg");
        list.setColId(icol++, "is_readonly_fg");
        list.setColId(icol++, "is_optional_fg");
        list.setColId(icol++, "is_multi_fg");
        list.setColId(icol++, "is_hidden_fg");
        list.setColId(icol++, "brief");
        list.setColId(icol++, "is_summary_fg");
        list.setColId(icol++, "is_virtual_fg");
        list.setColId(icol++, "is_batch_fg");
        list.setColId(icol++, "order");
        list.setColId(icol++, "default_value");
        list.setColId(icol++, "default_encoding");
        list.setColId(icol++, "link_url");
        list.setColId(icol++, "constraint");
        list.setColId(icol++, "constraint_data");
        list.setColId(icol++, "constraint_description");
        list.setColId(icol++, "description");
    }

    for(idx i = 0; i < fields.dim(); i++) {
        const sUsrTypeField * fld = fields[i];
        list.addRow().addCol(name())
            .addCol(fld->name())
            .addCol(fld->title())
            .addCol(fld->typeName());

        if( const sUsrTypeField * par = fld->parent() ) {
            if( par->isArrayRow() ) {
                par = par->parent();
            }
            list.addCol(par ? par->name() : "");
        } else {
            list.addCol("");
        }

        const char * old_role_name = fieldRoleToName(fld->role());
        list.addCol(old_role_name ? old_role_name : "")
            .addCol(fld->isKey())
            .addCol((idx)fld->readonly())
            .addCol(fld->isOptional())
            .addCol(fld->isMulti())
            .addCol(fld->isHidden())
            .addCol(fld->brief())
            .addCol(fld->isSummary())
            .addCol(fld->isVirtual())
            .addCol(fld->isBatch())
            .addCol(fld->orderString())
            .addCol(fld->defaultValue())
            .addCol(fld->defaultEncoding())
            .addCol(fld->linkUrl())
            .addCol(fld->constraint())
            .addCol(fld->constraintData())
            .addCol(fld->constraintDescription())
            .addCol(fld->description());
    }

    return fields.dim();
}

void sUsrType2::printJSON(const sUsr & user, sJSONPrinter & printer, bool into_object) const
{
    if( !into_object ) {
        printer.startObject();
    }
    printer.addKey("_id");
    printer.addValue(id());
    printer.addKey("name");
    printer.addValue(name(), 0, true);
    printer.addKey("title");
    printer.addValue(title(), 0, true);
    if( description() && *description() ) {
        printer.addKey("description");
        printer.addValue(description(), 0, true);
    }
    if( isVirtual() ) {
        printer.addKey("is_virtual");
        printer.addValue(isVirtual());
    }
    if( isPrefetch() ) {
        printer.addKey("is_prefetch");
        printer.addValue(isPrefetch());
    }
    if( isUser() ) {
        printer.addKey("is_user");
        printer.addValue(true);
    }
    if( isSystem() ) {
        printer.addKey("is_system");
        printer.addValue(true);
    }
    if( isSingleton() ) {
        printer.addKey("singleton");
        printer.addValue(isSingleton());
    }
    if( dimParents() ) {
        printer.addKey("_parents");
        printer.startArray();
        for(idx ic=0; ic<dimParents(); ic++) {
            printer.addValue(getParent(ic)->name());
        }
        printer.endArray();
    }
    if( dimIncludes() ) {
        printer.addKey("_includes");
        printer.startArray();
        for(idx ic=0; ic<dimIncludes(); ic++) {
            printer.addValue(getInclude(ic)->name());
        }
        printer.endArray();
    }
    if( dimChildren() ) {
        printer.addKey("_children");
        printer.startArray();
        for(idx ic=0; ic<dimChildren(); ic++) {
            printer.addValue(getChild(ic)->name());
        }
        printer.endArray();
    }

    if( dimRootFields(user) ) {
        printer.addKey("_attributes");
        printer.startObject();
        for(idx irif=0; irif<dimRootFields(user); irif++) {
            getRootField(user, irif)->printJSON(printer, true, true);
        }
        printer.endObject();
    }

    if( !into_object ) {
        printer.endObject();
    }
}

void sUsrType2::printJSON2(const sUsr & user, sJSONPrinter & printer, bool into_object) const
{
    if( !into_object ) {
        printer.startObject();
    }
    printer.addKey("_id");
    printer.addValue(id());
    printer.addKey("name");
    printer.addValue(name(), 0, true);
    printer.addKey("title");
    printer.addValue(title(), 0, true);
    if( description() && *description() ) {
        printer.addKey("descr");
        printer.addValue(description(), 0, true);
    }
    if( isVirtual() ) {
        printer.addKey("_abstr");
        printer.addValue(isVirtual());
    }
    if( isPrefetch() ) {
        printer.addKey("__is_prefetch");
        printer.addValue(isPrefetch());
    }
    if( isUser() ) {
        printer.addKey("__is_user");
        printer.addValue(true);
    }
    if( isSystem() ) {
        printer.addKey("__is_system");
        printer.addValue(true);
    }
    if( isSingleton() ) {
        printer.addKey("singleton");
        printer.addValue(isSingleton());
    }
    if( dimParents() ) {
        printer.addKey("__parents");
        printer.startArray();
        for(idx ic=0; ic<dimParents(); ic++) {
            printer.addValue(getParent(ic)->name());
        }
        printer.endArray();
    }
    if( dimIncludes() ) {
        printer.addKey("__includes");
        printer.startArray();
        for(idx ic=0; ic<dimIncludes(); ic++) {
            printer.addValue(getInclude(ic)->name());
        }
        printer.endArray();
    }
    if( dimChildren() ) {
        printer.addKey("__children");
        printer.startArray();
        for(idx ic=0; ic<dimChildren(); ic++) {
            printer.addValue(getChild(ic)->name());
        }
        printer.endArray();
    }

    if( dimRootFields(user) ) {
        printer.addKey("_field");
        printer.startObject();
        for(idx irif=0; irif<dimRootFields(user); irif++) {
            getRootField(user, irif)->printJSON2(printer, true, true);
        }
        printer.endObject();
    }

    if( !into_object ) {
        printer.endObject();
    }
}


sUsrLoadingTypeField::sUsrLoadingTypeField(bool default_zero)
{
    _is_key = _is_optional = _is_multi = _is_hidden = _is_summary = _is_virtual = _is_batch = _is_weak_reference = _is_sysinternal =
        _is_global_multi = _is_array_row = _is_broken = false;
    _ancestor_count = _dim_children = _default_encoding = 0;

    _index = _pos_name = _pos_title = _pos_orig_name = _pos_parent_name = _pos_brief = _pos_order = _pos_default_value = _pos_constraint = _pos_constraint_data = _pos_constraint_description = _pos_description = _pos_link_url = -1;
    _definer_itype = _owner_itype = _included_from_itype = -1;
    _type = eInvalid;
    _parent = -1;
    _start_children = -1;
    _is_flattened_decor = eLazyNotLoaded;
    _role = eRole_unknown;
    if( default_zero ) {
        _readonly = eReadWrite;
    } else {
        _readonly = eReadOnly;
        _is_optional = true;
        _is_hidden = true;
        _is_virtual = true;
    }
}

int sUsrTypeField::cmp(const sUsrTypeField * rhs) const
{
    if( !rhs ) {
        return 1;
    }
    const sHiveId & lhs_owner_type_id = ownerType() ? ownerType()->id() : sHiveId::zero;
    const sHiveId & rhs_owner_type_id = rhs->ownerType() ? rhs->ownerType()->id() : sHiveId::zero;
    if( lhs_owner_type_id != rhs_owner_type_id ) {
        return lhs_owner_type_id < rhs_owner_type_id ? -1 : 1;
    }
    const sUsrTypeField * lhs = this;
    idx lhs_orig_depth = lhs->ancestorCount();
    idx rhs_orig_depth = rhs->ancestorCount();
    while(lhs->ancestorCount() > rhs->ancestorCount()) {
        lhs = lhs->parent();
    }
    while(rhs->ancestorCount() > lhs->ancestorCount()) {
        rhs = rhs->parent();
    }
    if( lhs == rhs ) {
        return lhs_orig_depth - rhs_orig_depth;
    }
    for(idx i = 0; lhs->parent() != rhs->parent() && i < lhs->ancestorCount(); i++) {
        lhs = lhs->parent();
        rhs = rhs->parent();
    }
    if( !sIsExactly(lhs->orderString(), rhs->orderString()) ) {
        return lhs->order() < rhs->order() ? -1 : 1;
    }
    const char * lhs_name = lhs->name();
    if( !lhs_name) {
        lhs_name = sStr::zero;
    }
    const char * rhs_name = rhs->name();
    if( !rhs_name) {
        rhs_name = sStr::zero;
    }
    return strcasecmp(lhs_name, rhs_name);
}

const char * sUsrTypeField::typeName() const
{
    return fieldTypeToName(type());
}

const char * sUsrTypeField::typeName2() const
{
    return fieldTypeToName2(type());
}

const sUsrLoadingTypeField* sUsrLoadingTypeField::parent() const
{
    return _parent >= 0 ? sUsrLoadingType::_types[_owner_itype]->_fields.ptr(_parent) : 0;
}

const sUsrLoadingTypeField* sUsrLoadingTypeField::getChild(idx ichild) const
{
    const sUsrLoadingType & utype = *sUsrLoadingType::_types[_owner_itype];
    return ichild >= 0 && ichild < _dim_children ? utype._fields.ptr(utype._child_ifields[_start_children + ichild]) : 0;
}

idx sUsrLoadingTypeField::getChildren(sVec<const sUsrTypeField*> &out) const
{
    idx start_dim = out.dim();
    out.resize(start_dim + _dim_children);
    const sUsrLoadingType & utype = *sUsrLoadingType::_types[_owner_itype];
    for(idx i=0; i<_dim_children; i++) {
        out[start_dim + i] = utype._fields.ptr(utype._child_ifields[_start_children + i]);
    }
    return _dim_children;
}

bool sUsrLoadingTypeField::canSetValue() const
{
    const char * nm = name();
    return canHaveValue() && !isVirtual() && nm[0] != '_' && !sIsExactly(nm, "created") && !sIsExactly(nm, "modified");
}

bool sUsrLoadingTypeField::isFlattenedDecor() const
{
    if( _is_flattened_decor == eLazyNotLoaded ) {
        if( type() == eArray || type() == eArrayTab ) {
            if( isMulti() ) {
                _is_flattened_decor = eLazyFalse;
            } else {
                const sUsrTypeField * array_row = dimChildren() ? getChild(0) : 0;
                _is_flattened_decor = array_row && !array_row->isFlattenedDecor() ? eLazyFalse : eLazyTrue;
            }
        } else if( type() == eList || type() == eListTab ) {
            if( isArrayRow() ) {
                bool multi_row = false;
                for(idx i = 0; i < dimChildren(); i++) {
                    if( getChild(i)->isMulti() ) {
                        multi_row = true;
                        break;
                    }
                }
                _is_flattened_decor = multi_row ? eLazyFalse : eLazyTrue;
            } else {
                idx cnt_nondec = 0;
                idx cnt_nondec_multi = 0;
                for(idx i = 0; i < dimChildren(); i++) {
                    if( !getChild(i)->isFlattenedDecor() ) {
                        cnt_nondec++;
                        if( getChild(i)->isMulti() ) {
                            cnt_nondec_multi++;
                            if( cnt_nondec_multi >= 2 ) {
                                _is_flattened_decor = eLazyFalse;
                                break;
                            }
                        }
                    }
                }
                if( _is_flattened_decor == eLazyNotLoaded && cnt_nondec >= 2 && isMulti() && (!parent() || !parent()->isArrayRow()) ) {
                    _is_flattened_decor = eLazyFalse;
                } else {
                    _is_flattened_decor = eLazyTrue;
                }
            }
        } else {
            _is_flattened_decor = eLazyFalse;
        }
    }
    return _is_flattened_decor == eLazyTrue;
}

bool sUsrLoadingTypeField::isFlattenedMulti() const
{
    if( isMulti() && (!parent() || !parent()->isArrayRow()) ) {
        return true;
    }

    if( isArrayRow() ) {
        for(idx i = 0; i < dimChildren(); i++) {
            if( getChild(i)->isMulti() ) {
                return true;
            }
        }
    }

    const sUsrTypeField * par = parent();
    while( par && par->isFlattenedDecor() ) {
        if( par->isMulti() ) {
            return true;
        }
        par = par->parent();
    }

    return false;
}

const sUsrTypeField * sUsrTypeField::flattenedParent() const
{
    const sUsrTypeField * par = parent();
    while( par && par->isFlattenedDecor() ) {
        par = par->parent();
    }
    return par;
}

const sUsrType2 * sUsrLoadingTypeField::ownerType() const
{
    if( _owner_itype >= 0 ) {
        const sUsrLoadingType * owner = sUsrLoadingType::_types[_owner_itype];
        return owner->_id.objId() ? owner : 0;
    } else {
        return 0;
    }
}

const sUsrType2 * sUsrLoadingTypeField::definerType() const
{
    if( _definer_itype >= 0 ) {
        const sUsrLoadingType * definer = sUsrLoadingType::_types[_definer_itype];
        return definer->_id.objId() ? definer : 0;
    } else {
        return 0;
    }
}

const sUsrType2 * sUsrLoadingTypeField::includedFromType() const
{
    if( _included_from_itype >= 0 ) {
        const sUsrLoadingType * included_from = sUsrLoadingType::_types[_included_from_itype];
        return included_from->_id.objId() ? included_from : 0;
    } else {
        return 0;
    }
}


bool sUsrTypeField::parseValue(sVariant &var, sUsrTypeField::EType type, const char * value, idx flags)
{
    if (!value) {
        var.setNull();
        return false;
    }

    switch (type) {
    case eString:
    case eUrl:
    case eText:
    case eFile:
        var.setString(value);
        break;
    case eInteger:
        var.parseInt(value);
        break;
    case eUnsigned:
        var.parseUInt(value);
        break;
    case eReal:
        var.parseReal(value);
        break;
    case eBool:
        var.parseBool(value);
        break;
    case eList:
    case eArray:
    case eListTab:
    case eArrayTab:
        var.parseIntList(value);
        break;
    case eDate:
        var.parseDate(value);
        break;
    case eTime:
        var.parseTime(value);
        break;
    case eDateTime:
        var.parseDateTime(value);
        break;
    case eObj:
        var.parseHiveIdList(value);
        if (var.dim() == 0) {
            var.setHiveId(0, 0, 0);
        } else if (var.dim() == 1) {
            sHiveId tmp;
            var.getListElt(0)->asHiveId(&tmp);
            var.setHiveId(tmp);
        }
        break;
    case ePassword:
        if (flags & fPasswordErr) {
            var.setNull();
            return false;
        } else if (flags & fPasswordHide) {
            var.setString("*****");
        } else {
            var.setString(value);
        }
        break;
    case eMemory:
    case eVersion:
    case eBlob:
    case eJSON:
    case eXML:
        var.setString(value);
        break;
    case eInvalid:
    default:
        var.setNull();
        return false;
    }
    return true;
}

void sUsrTypeField::printJSON(sJSONPrinter & printer, bool recurse, bool into_object) const
{
    const char * nm = name();
    if( !nm || !*nm ) {
        return;
    }

    bool print_self = !isArrayRow();

    if( !into_object ) {
        printer.startObject();
    }

    if( print_self ) {
        printer.addKey(nm);
        printer.startObject();

        printer.addKey("type");
        printer.addValue(typeName(), 0, true);
        if( *defaultValue() ) {
            printer.addKey("default_value");
            printer.addValue(defaultValue(), 0, true);
        }
        if( defaultEncoding() ) {
            printer.addKey("default_encoding");
            printer.addValue(defaultEncoding());
        }
        printer.addKey("title");
        printer.addValue(title(), 0, true);
        if( *description() ) {
            printer.addKey("description");
            printer.addValue(description());
        }
        printer.addKey("order");
        const char * order_string = orderString();
        if( *order_string ) {
            printer.addValue(strtod(order_string, 0), order_string);
        }
        if( isKey() ) {
            printer.addKey("is_key");
            printer.addValue(isKey());
        }
        switch( readonly() ) {
            case eReadWrite:
                printer.addKey("is_readonly");
                printer.addValue(false);
                break;
            case eReadOnly:
                printer.addKey("is_readonly");
                printer.addValue(true);
                break;
            case eWriteOnce:
                printer.addKey("is_readonly");
                printer.addValue(true);
                printer.addKey("is_writeonce");
                printer.addValue(true);
                break;
            case eSubmitOnce:
                printer.addKey("is_readonly");
                printer.addValue(true);
                printer.addKey("is_submitonce");
                printer.addValue(true);
                break;
            case eReadOnlyAutofill:
                printer.addKey("is_readonly");
                printer.addValue(true);
                printer.addKey("is_autofill");
                printer.addValue(true);
                break;
        }
        if( isOptional() ) {
            printer.addKey("is_optional");
            printer.addValue(true);
        }
        if( isMulti() ) {
            printer.addKey("is_multi");
            printer.addValue(true);
        }
        if( isFlattenedDecor() ) {
            printer.addKey("is_flattened_decor");
            printer.addValue(true);
        }
        if( isFlattenedMulti() ) {
            printer.addKey("is_flattened_multi");
            printer.addValue(true);
        }
        if( isHidden() ) {
            printer.addKey("is_hidden");
            printer.addValue(true);
        }
        if( isSummary() ) {
            printer.addKey("is_summary");
            printer.addValue(true);
        }
        if( isVirtual() ) {
            printer.addKey("is_virtual");
            printer.addValue(true);
        }
        if( isBatch() ) {
            printer.addKey("is_batch");
            printer.addValue(true);
        }
        if( isWeakReference() ) {
            printer.addKey("is_weak_reference");
            printer.addValue(true);
        }
        if( role() != eRole_unknown ) {
            printer.addKeyValue("role", fieldRoleToName(role()));
        }
        if( *brief() ) {
            printer.addKey("brief");
            printer.addValue(brief());
        }
        if( *constraint() ) {
            printer.addKey("constraint");
            printer.addValue(constraint());
        }
        if( *constraintData() ) {
            printer.addKey("constraint_data");
            printer.addValue(constraintData());
        }
        if( *constraintDescription() ) {
            printer.addKey("constraint_description");
            printer.addValue(constraintDescription());
        }
        if( *linkUrl() ) {
            printer.addKey("link_url");
            printer.addValue(linkUrl());
        }

        if( definerType() && definerType()->id() != ownerType()->id() ) {
            printer.addKey("_definer_type");
            printer.addValue(definerType()->name());
        }

        if( includedFromType() && includedFromType()->id() != ownerType()->id() ) {
            printer.addKey("_included_from_type");
            printer.addValue(includedFromType()->name());
        }

        if( dimChildren() ) {
            printer.addKey("_children");
        }
    }

    if( dimChildren() ) {
        if( recurse ) {
            if( print_self ) {
                printer.startObject();
            }
            for(idx ic=0; ic<dimChildren(); ic++) {
                getChild(ic)->printJSON(printer, recurse, true);
            }
            if( print_self ) {
                printer.endObject();
            }
        } else {
            if( print_self ) {
                printer.startArray();
            }
            for(idx ic=0; ic<dimChildren(); ic++) {
                printer.addValue(getChild(ic)->name());
            }
            if( print_self ) {
                printer.endArray();
            }
        }
    }

    if( print_self ) {
        printer.endObject();
    }

    if( !into_object ) {
        printer.endObject();
    }
}

static void printParsedJSONValue(sJSONPrinter & printer, const sUsrTypeField * fld, const char * s, idx len = 0)
{
    sVariant val;
    sStr buf;
    if( len ) {
        buf.addString(s, len);
        s = buf.ptr();
    }


    if( fld->parseValue(val, s) ) {
        if( s[0] && fld->type() == sUsrTypeField::eReal ) {
            printer.addValue(val.asReal(), s);
        } else {
            printer.addValue(val);
        }
    } else {
        printer.addValue(s);
    }
}

void sUsrTypeField::printJSON2(sJSONPrinter & printer, bool recurse, bool into_object) const
{
    const char * nm = name();
    if( !nm || !*nm ) {
        return;
    }

    bool print_self = !isArrayRow();

    if( !into_object ) {
        printer.startObject();
    }

    if( print_self ) {
        printer.addKey(nm);
        printer.startObject();

        if( canHaveValue() ) {
            printer.addKey("_type");
            printer.addValue(typeName2(), 0, true);
        } else if( type() == eArray ) {
            printer.addKeyValue("_layout", "table");
        } else if( type() == eList ) {
            printer.addKeyValue("_layout", "struct");
        } else if( type() == eArrayTab ) {
            printer.addKeyValue("_layout", "table_tab");
        } else if( type() == eListTab ) {
            printer.addKeyValue("_layout", "struct_tab");
        }
        if( canHaveValue() && *defaultValue() ) {
            printer.addKey("_default");
            printParsedJSONValue(printer, this, defaultValue());
        }
        if( defaultEncoding() ) {
            printer.addKeyValue("_encode", defaultEncoding());
        }
        printer.addKey("title");
        printer.addValue(title(), 0, true);
        if( *description() ) {
            printer.addKeyValue("descr", description());
        }
        const char * order_string = orderString();
        if( order_string && *order_string ) {
            printer.addKeyValue("_order", strtod(order_string, 0), order_string);
        }
        if( isKey() ) {
            printer.addKeyValue("_is_key", isKey());
        }
        printer.addKey("_write");
        switch( readonly() ) {
            case eReadWrite:
                printer.addValue(true);
                break;
            case eReadOnly:
                printer.addValue(false);
                break;
            case eWriteOnce:
                printer.addValue("once");
                break;
            case eSubmitOnce:
                printer.addValue("noresub");
                break;
            case eReadOnlyAutofill:
                printer.addValue("onlyauto");
                break;
        }
        printer.addKeyValue("_vital", !isOptional());
        if( isMulti() ) {
            printer.addKeyValue("_plural", true);
        }
        if( isFlattenedDecor() ) {
            printer.addKeyValue("__flattened_decor", true);
        }
        if( isFlattenedMulti() ) {
            printer.addKeyValue("__flattened_plural", true);
        }
        if( isHidden() ) {
            printer.addKeyValue("_hidden", true);
        }
        if( isSummary() ) {
            printer.addKeyValue("_public", true);
        }
        if( isVirtual() ) {
            printer.addKeyValue("_virtual", true);
        }
        if( isBatch() ) {
            printer.addKeyValue("_batched", true);
        }
        if( role() != eRole_unknown ) {
            printer.addKeyValue("_role", fieldRoleToName(role()));
        }
        if( isWeakReference() ) {
            printer.addKeyValue("_weakref", true);
        }
        if( *brief() ) {
            printer.addKeyValue("brief", brief());
        }
        if( *constraint() || *constraintDescription() ) {
            printer.addKey("_limit");
            printer.startObject();

            if( sIsExactly(constraint(), "choice") || sIsExactly(constraint(), "choice+") ) {
                printer.addKey(constraint());
                printer.startArray();

                sStr choices00, value_buf;
                sVariant value_val;
                sString::searchAndReplaceSymbols(&choices00, constraintData(), 0, "|", 0, 0, true, true, false);
                choices00.add0(2);
                choices00.shrink00();

                for(const char * value = choices00.ptr(); value && *value; value = sString::next00(value)) {
                    while(isspace(*value)) {
                        value++;
                    }
                    idx value_len = 0;
                    const char * title = 0;
                    idx title_len = 0;
                    if( const char * slashes = strstr(value, "///") ) {
                        value_len = slashes - value;
                        title = slashes + 3;
                        while(isspace(*title)) {
                            title++;
                        }
                        title_len = sLen(title);
                        while (title_len > 0 && isspace(title[title_len - 1])) {
                            title_len--;
                        }
                    } else {
                        value_len = sLen(value);
                    }

                    while( value_len > 0 && isspace(value[value_len - 1]) ) {
                        value_len--;
                    }

                    if( title_len ) {
                        printer.startObject();
                        printer.addKeyValue("title", title, title_len);
                        printer.addKey("value");
                    }

                    printParsedJSONValue(printer, this, value, value_len);

                    if( title_len ) {
                        printer.endObject();
                    }
                }

                printer.endArray();
            } else if( sIsExactly(constraint(), "regexp") ) {
                printer.addKeyValue("regexp", constraintData());
            } else if( sIsExactly(constraint(), "range") ) {
                printer.addKey("range");
                printer.startObject();
                const char * constraint_data = constraintData();
                const char * dash = constraint_data ? strchr(constraint_data, '-') : 0;
                if( constraint_data && (!dash || dash > constraint_data) ) {
                    printer.addKey("min");
                    printParsedJSONValue(printer, this, constraint_data, dash ? dash - constraint_data : 0);
                }
                if( dash && *dash ) {
                    printer.addKey("max");
                    printParsedJSONValue(printer, this, dash + 1);
                }
                printer.endObject();
            } else if( sIsExactly(constraint(), "url") ) {
                printer.addKey("search");
                printer.startObject();
                printer.addKeyValue("url", constraintData());
                printer.endObject();
            } else if( sIsExactly(constraint(), "search") || sIsExactly(constraint(), "search+") ) {
                printer.addKey(constraint());
                printer.startObject();

                static qlang::Engine engine;
                if( engine.parse(constraintData()) ) {
                    sVariant * result = engine.run();
                    if( result && result->isDic() ) {
                        if( sVariant * url_val = result->getDicElt("url") ) {
                            printer.addKeyValue("url", *url_val);
                        }
                        sVariant * fetch_val = result->getDicElt("fetch");
                        if( fetch_val ) {
                            printer.addKeyValue("value", *fetch_val);
                        }
                        if( sVariant * inline_val = result->getDicElt("inline") ) {
                            sStr title;
                            if( inline_val->isScalar() ) {
                                sTxtTbl tbl;
                                tbl.setBuf(inline_val->asString());
                                tbl.parse();
                                for(idx i = 0; i < tbl.cols(); i++) {
                                    title.cut0cut();
                                    tbl.printTopHeader(title, i);
                                    if( sIs(title.ptr(), fetch_val ? fetch_val->asString() : "id") ) {
                                        continue;
                                    }
                                    break;
                                }
                            } else if( inline_val->isList() ) {
                                for(idx i = 0; i < inline_val->dim(); i++) {
                                    title.cut0cut();
                                    sVariant * col_val = inline_val->getListElt(i);
                                    if( col_val->getDicElt("hidden") && col_val->getDicElt("hidden")->asBool() ) {
                                        continue;
                                    }
                                    if( col_val->isString() ) {
                                        col_val->print(title);
                                    } else if( col_val->getDicElt("name") ) {
                                        col_val->getDicElt("name")->print(title);
                                    }
                                    if( sIs(title.ptr(), fetch_val ? fetch_val->asString() : "id") ) {
                                        continue;
                                    }
                                    break;
                                }
                            }
                            if( title.length() ) {
                                printer.addKeyValue("title", title.ptr());
                            }
                        }
                        if( sVariant * outline_val = result->getDicElt("outline") ) {
                            sStr buf;

                            printer.addKey("show");
                            printer.startArray();

                            if( outline_val->isScalar() ) {
                                sTxtTbl tbl;
                                tbl.setBuf(outline_val->asString());
                                tbl.parse();
                                for(idx i = 0; i < tbl.cols(); i++) {
                                    buf.cut0cut();
                                    tbl.printTopHeader(buf, i);
                                    printer.addValue(buf.ptr());
                                }
                            } else if( outline_val->isList() ) {
                                for(idx i = 0; i < outline_val->dim(); i++) {
                                    buf.cut0cut();
                                    sVariant * col_val = outline_val->getListElt(i);
                                    if( col_val->getDicElt("hidden") && col_val->getDicElt("hidden")->asBool() ) {
                                        continue;
                                    }
                                    if( col_val->isString() ) {
                                        col_val->print(buf);
                                        printer.addValue(buf.ptr());
                                    } else if( col_val->getDicElt("name") ) {
                                        printer.addValue(*col_val);
                                    }
                                }
                            }

                            printer.endArray();
                        }
                        printer.addKeyValue("format", "csv");

                        if( sVariant * qryLang_val = result->getDicElt("qryLang") ) {
                            printer.addKeyValue("url", *qryLang_val);
                        }
                        if( sVariant * explorer_val = result->getDicElt("explorer") ) {
                            printer.addKeyValue("url", explorer_val->asBool());
                        }
                    }
                }
                printer.endObject();
            } else if( sIsExactly(constraint(), "type" ) ) {
                printer.addKeyValue("type", constraintData());
            } else if( sIsExactly(constraint(), "eval" ) ) {
                sStr out;
                sString::searchAndReplaceStrings(&out, constraintData(), 0, "$_(val)" __, "${_val}" __, 0, true);
                printer.addKeyValue("eval", out.ptr());
            }

            if( *constraintDescription() ) {
                printer.addKeyValue("descr", constraintDescription());
            }

            printer.endObject();
        }

        if( *linkUrl() ) {
            sStr out;
            sString::searchAndReplaceStrings(&out, linkUrl(), 0, "$_(val)" __, "${_val}" __, 0, true);
            printer.addKeyValue("link_url", out.ptr());;
        }

        if( definerType() && definerType()->id() != ownerType()->id() ) {
            printer.addKey("__definer_type");
            printer.addValue(definerType()->name());
        }

        if( includedFromType() && includedFromType()->id() != ownerType()->id() ) {
            printer.addKey("__included_from_type");
            printer.addValue(includedFromType()->name());
        }

        if( dimChildren() ) {
            printer.addKey("_field");
        }
    }

    if( dimChildren() ) {
        if( recurse ) {
            if( print_self ) {
                printer.startObject();
            }
            for(idx ic=0; ic<dimChildren(); ic++) {
                getChild(ic)->printJSON2(printer, recurse, true);
            }
            if( print_self ) {
                printer.endObject();
            }
        } else {
            if( print_self ) {
                printer.startArray();
            }
            for(idx ic=0; ic<dimChildren(); ic++) {
                printer.addValue(getChild(ic)->name());
            }
            if( print_self ) {
                printer.endArray();
            }
        }
    }

    if( print_self ) {
        printer.endObject();
    }

    if( !into_object ) {
        printer.endObject();
    }
}

idx sUsrLoadingTypeField::setString(const char *s, bool canonicalize, bool allow_empty)
{
    if( (!s || !*s) && !allow_empty ) {
        return -1;
    }

    sStr case_buf;
    if( canonicalize ) {
        s = canonicalCase(case_buf, s);
    } else if( !s ) {
        s = sStr::zero;
    }
    idx ret = _name_buf.length();
    _name_buf.add(s);
    return ret;
}

void sUsrLoadingTypeField::replaceString(idx & pos, const char * find00, const char * replace00, sStr & buf)
{
    idx need_replace = false;
    if( pos >= 0 ) {
        for(const char * f = find00; f && *f; f = sString::next00(f)) {
            if( strstr(_name_buf.ptr(pos), f) ) {
                need_replace = true;
                break;
            }
        }
    }
    if( need_replace ) {
        idx buf_pos = buf.length();
        buf.addString(_name_buf.ptr(pos));
        pos = _name_buf.length();
        sString::searchAndReplaceStrings(&_name_buf, buf.ptr(buf_pos), 0, find00, replace00, 0, true);
    }
}

