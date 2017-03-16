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
#include <qlib/QPrideBase.hpp>
#include <ulib/utype2.hpp>
#include <ulib/uobj.hpp>
#include <ulib/upropset.hpp>

#include <assert.h>
#include <ctype.h>
#include <regex.h>

using namespace slib;

static sStr utype_errors;

#define TRACE_UTYPE2 0

#if TRACE_UTYPE2
#define LOG_TRACE(fmt, ...) \
do { \
    sStr local_log_buf; \
    fprintf(stderr, "Trace %s: %s():%u: " fmt "\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
} while( 0 )
#else
#define LOG_TRACE(fmt, ...)
#endif

#define LOG_WARNING(fmt, ...) \
do { \
    idx pos = utype_errors.length(); \
    sStr local_log_buf; \
    utype_errors.printf("WARNING %s: %s():%u: " fmt "\n", __FILE__, __func__, __LINE__,  __VA_ARGS__); \
    fprintf(stderr, "%s", utype_errors.ptr(pos)); \
} while( 0 )

#define LOG_ERROR(fmt, ...) \
do { \
    idx pos = utype_errors.length(); \
    sStr local_log_buf; \
    utype_errors.printf("ERROR %s: %s():%u: " fmt "\n", __FILE__, __func__, __LINE__,  __VA_ARGS__); \
    fprintf(stderr, "%s", utype_errors.ptr(pos)); \
} while( 0 )

static bool useTypeUPObj()
{
    static bool use_type_upobj = false;
    static bool first_call = true;

    if( first_call ) {
        // TYPE_UPOBJ env variable can be set via sUsrCGI::OnCGIInit() by &useTypeUPObj=1 url param
        if( const char * s = getenv("TYPE_UPOBJ") ) {
            use_type_upobj = sString::parseBool(s);
        }
        first_call = false;
    }
    return use_type_upobj;
}

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

static const char * canonicalCase(sStr & buf, const char * str, idx len = 0)
{
    if( !str ) {
        return sStr::zero;
    }
    if( !len ) {
        len = sLen(str);
    }
    for(idx i=0; i<len; i++) {
        if( str[i] >= 'A' && str[i] <= 'Z' ) {
            buf.cut0cut();
            sString::changeCase(&buf, str, 0, sString::eCaseLo);
            return buf.ptr(0);
        }
    }
    return str;
}

static idx strncmp_exact(const char * s1, idx len1, const char * s2, idx len2)
{
    if( !len1 ) {
        len1 = sLen(s1);
    }
    if( !len2 ) {
        len2 = sLen(s2);
    }
    if( int d = strncmp(s1, s2, sMin<idx>(len1, len2)) ) {
        return d;
    }
    return len1 - len2;
}

/// static data members ///

const udx sUsrType2::type_type_domain_id = sHiveId::encodeDomainId("type");
sStr sUsrType2::_name_buf;
sStr sUsrType2::_err_buf;
sMex sUsrType2::_deps_buf;
sUsrType2::sUsrType2List sUsrType2::_types;
sDic<idx> sUsrType2::_name_or_id2itype;
sVec<sUsrType2*> sUsrType2::_roots;
bool sUsrType2::_all_deps_loaded = false;
bool sUsrType2::_all_prefetched = false;

sDic<sUsrAction::ActRes> sUsrAction::_usr2actres;
sMex sUsrView::_fields_buf;
sDic<sUsrView::ViewRes> sUsrView::_usr2viewres;
sDic<bool> sUsrView::_names;

sStr sUsrTypeField::_name_buf;

// Keep this in sync with sUsrTypeField::eType in uprop2.hpp
static const char * field_type_names[] = {
    // skip "invalid", it's -1
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
    "file"
};

// convert a property type name ("obj", "bool", etc.) to an eUsrObjPropType code
inline static const sUsrTypeField::EType fieldTypeFromName(const char * name)
{
    for(idx i=0; i<sDim(field_type_names); i++) {
        if( sIs(name, field_type_names[i]) ) {
            return (sUsrTypeField::EType)i;
        }
    }
    return sUsrTypeField::eInvalid;
}

static const char * fieldTypeToName(sUsrTypeField::EType t)
{
    if( t >= 0 && t < sDim(field_type_names)) {
        return field_type_names[(int)t];
    }
    return "invalid";
}

// private interfaces

sUsrType2::sUsrType2(bool default_zero/* = false*/)
{
    _itype = _pos_name = _pos_title = _pos_description = -1;
    _is_virtual = default_zero ? false : true;
    _is_user = eLazyNotLoaded;
    _is_system = eLazyNotLoaded;
    _is_prefetch = false;
    _is_fetched = false;
    _is_broken = false;

    _created = _modified = 0;

    _parents.init(&_deps_buf);
    _children.init(&_deps_buf);
    _includes.init(&_deps_buf);

    _dim_explicit_fields = _dim_inherited_fields = _dim_included_fields = 0;
}

//static
sUsrType2 * sUsrType2::getRaw(const char * type_name, idx type_name_len/* = 0 */)
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

//static
sUsrType2 * sUsrType2::getRaw(const sHiveId & type_id)
{
    idx * pitype = _name_or_id2itype.get(&type_id, sizeof(sHiveId));
    return pitype ? _types[*pitype] : 0;
}

namespace {
    struct DepList {
        struct Entry {
            sHiveId type_id;
            sHiveId dep_id;
            idx index; // index of path in interned_strings, or of field if kind == eInclude
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
            if( !path ) {
                path = "";
            }
            interned_strings.setString(path, 0, &ep->index);

            Entry * ec = vec.add(1);
            ec->type_id = parent_id;
            ec->dep_id = type_id;
            ec->kind = Entry::eChild;
            ec->index = -sIdxMax;
        }

        void pushInclude(sHiveId type_id, sHiveId include_id, idx ifld)
        {
            Entry * ed = vec.add(1);
            ed->type_id = type_id;
            ed->dep_id = include_id;
            ed->index = ifld;
            ed->kind = Entry::eInclude;
        }

        static idx sort_cb(void * param, void * arr_param, idx i1, idx i2)
        {
            DepList * self = static_cast<DepList *>(param);
            idx * arr = static_cast<idx*>(arr_param);
            Entry & e1 = self->vec[arr[i1]];
            Entry & e2 = self->vec[arr[i2]];

            // compare by child id, then by path
            if( idx diff = e1.type_id.cmp(e2.type_id) ) {
                return diff;
            }

            if( idx diff = (idx)(e1.kind) - (idx)(e2.kind) ) {
                return diff;
            }

            if( e1.kind == Entry::eParent && e2.kind == Entry::eParent ) {
                const char * path1 = static_cast<const char *>(self->interned_strings.id(e1.index));
                const char * path2 = static_cast<const char *>(self->interned_strings.id(e2.index));
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

    // map from type id + first 2 elements of group path to field's index within the type
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

        // check if anything for this type id was set
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
            *dic.set(&type_id, sizeof(sHiveId)) = -sIdxMax; // record that *something* for this type id was set
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

class sUsrType2::DepForest
{
    public:
        DepForest() { ensureType2Inode(); }
        void add(sUsrType2 * utype)
        {
            ensureType2Inode();
            addWorker(utype);
        }
        bool has(idx itype) const
        {
            return itype >= 0 && itype < _itype2node.dim() && _itype2node[itype] >= 0;
        }
        // generate a traversal of the tree such that for any i, node i only depends only on nodes 0..i-1
        idx makeTraversal(sVec<idx> & out);

    private:
        sMex _mex;
        sVec< sKnot<udx> > _nodes;
        sVec<idx> _roots;
        sVec<idx> _itype2node;

        idx addWorker(const sUsrType2 * utype);
        void makeTreeTraversal(sVec<idx> & out, idx inode, sVec<bool> & visited);
        void ensureType2Inode()
        {
            if( !_itype2node.dim() ) {
                _itype2node.resize(sUsrType2::_types.dim());
                for( idx itype=0; itype<_itype2node.dim(); itype++ ) {
                    _itype2node[itype] = -sIdxMax;
                }
            }
        }
};

idx sUsrType2::DepForest::addWorker(const sUsrType2 * utype)
{
    if( utype->_is_fetched ) {
        // if the type was fetched already, don't refetch it
        return -1;
    }
    if( _itype2node[utype->_itype] >= 0 ) {
        // if the type was already pulled into the forest, use its node
        return _itype2node[utype->_itype];
    }
    LOG_TRACE("adding \"%s\" (%s) to forest", utype->name(), utype->id().print());
    idx inode = _itype2node[utype->_itype] = _nodes.dim();
    sKnot<udx> * node = _nodes.add(1);
    node->obj = utype->_itype;
    node->in.init(&_mex);
    node->out.init(&_mex);
    node = 0;

    // otherwise: pull in parents, then includes
    for(idx ip=0; ip<utype->dimParents(); ip++) {
        const sUsrType2 * par = utype->getParent(ip);
        idx ipar_node = addWorker(par);
        if( ipar_node >= 0 ) {
            *_nodes[inode].in.add(1) = ipar_node;
            *_nodes[ipar_node].out.add(1) = inode;
        }
    }
    for(idx iinc=0; iinc<utype->dimIncludes(); iinc++) {
        const sUsrType2 * inc = utype->getInclude(iinc);
        idx iinc_node = addWorker(inc);
        if( iinc_node >= 0 ) {
            *_nodes[inode].in.add(1) = iinc_node;
            *_nodes[iinc_node].out.add(1) = inode;
        }
    }
    return inode;
}

idx sUsrType2::DepForest::makeTraversal(sVec<idx> & out)
{
    idx start_dim = out.dim();
    sVec<bool> visited(sMex::fSetZero);
    visited.resize(_nodes.dim());
    for(idx inode=0; inode<_nodes.dim(); inode++) {
        makeTreeTraversal(out, inode, visited);
    }
    return out.dim() - start_dim;
}

void sUsrType2::DepForest::makeTreeTraversal(sVec<idx> & out, idx inode, sVec<bool> & visited)
{
    if( visited[inode] ) {
        return;
    }
    visited[inode] = true;
    sKnot<udx> & node = _nodes[inode];
    // first ensure we got all of the node's parents...
    for(idx iin=0; iin < node.in.dim(); iin++) {
        LOG_TRACE("Traversing parent %"DEC"/%"DEC" of type \"%s\": \"%s\"", iin, node.in.dim(), sUsrType2::_types[node.obj]->name(), sUsrType2::_types[_nodes[*node.in.ptr(iin)].obj]->name());
        makeTreeTraversal(out, *node.in.ptr(iin), visited);
    }

    // then add node to traversal order
    LOG_TRACE("Adding type \"%s\" to traversal", sUsrType2::_types[node.obj]->name());
    *out.add(1) = node.obj;
}


// magic flag for fld->_included_from_itype indicating that a valid field_include_type is expected
#define INCLUDE_TYPE_EXPECTED -999
class sUsrType2::LoadFromObjContext {
    public:
        const sUsr & user;
        sUsrType2 * utype;
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
            if( strcmp(prop_name, "name") == 0 ) {
                 if( isalpha(prop_value[0]) ) {
                     const char * canon_value = canonicalCase(case_buf, prop_value);
                     // valid type name, not something like "--DELETED" etc. (which was permitted in old pre-2016 UPType table, but is not permitted in UPObj-based type storage)
                     if( idx * pitype = _name_or_id2itype.get(canon_value) ) {
                         sUsrType2 * other_utype = _types[*pitype];
                         if( utype->_id != other_utype->_id ) {
                             LOG_ERROR("Types \"%s\" and \"%s\" have the same name \"%s\"", utype->_id.print(), other_utype->_id.print(local_log_buf), prop_value);
                             // FIXME: do we want to mark utype and/or other_utype as broken?
                         }
                     } else {
                         *_name_or_id2itype.setString(canon_value) = utype->_itype;
                     }
                     if( utype->_pos_name < 0 ) {
                         utype->_pos_name = _name_buf.length();
                         _name_buf.addString(prop_value);
                         _name_buf.add0();
                     } else if( strcmp(utype->name(), prop_value) != 0 ) {
                         LOG_ERROR("Type \"%s\" found with 2 different names: '%s' and '%s'", utype->_id.print(), utype->name(), prop_value);
                         utype->_is_broken = true;
                     }
                 } else {
                     LOG_ERROR("Type \"%s\" has invalid name \"%s\"", utype->_id.print(), prop_value);
                     utype->_is_broken = true;
                     return false;
                 }
             } else if( strcmp(prop_name, "created") == 0 ) {
                 struct tm unused;
                 utype->_created = sString::parseDateTime(&unused, prop_value);
             } else if( strcmp(prop_name, "modified") == 0 ) {
                 struct tm unused;
                 utype->_modified = sString::parseDateTime(&unused, prop_value);
             } else if( strcmp(prop_name, "title") == 0 ) {
                 if( utype->_pos_title < 0 ) {
                     utype->_pos_title = _name_buf.length();
                     _name_buf.addString(prop_value);
                     _name_buf.add0();
                 } else if( strcmp(utype->title(), prop_value) != 0 ) {
                     LOG_ERROR("Type \"%s\" found with 2 different titles: '%s' and '%s'", utype->_id.print(), utype->title(), prop_value);
                     utype->_is_broken = true;
                 }
             } else if( strcmp(prop_name, "description") == 0 ) {
                 if( utype->_pos_description < 0 ) {
                     utype->_pos_description = _name_buf.length();
                     _name_buf.addString(prop_value);
                     _name_buf.add0();
                 } else if( strcmp(utype->description(), prop_value) != 0 ) {
                     LOG_ERROR("Type \"%s\" found with 2 different descriptions: '%s' and '%s'", utype->_id.print(), utype->description(), prop_value);
                     utype->_is_broken = true;
                 }
             } else if( strcmp(prop_name, "is_abstract_fg") == 0 ) {
                 utype->_is_virtual = sString::parseBool(prop_value);
             } else if( strcmp(prop_name, "prefetch") == 0 )  {
                 utype->_is_prefetch = sString::parseBool(prop_value);
             } else if( strcmp(prop_name, "parent") == 0 ) {
                 // order of parent types matters, but db rows arrive unsorted - so save and sort by path later
                 if( !utype->dimParents() ) {
                     sHiveId parent_id;
                     if( !parent_id.parse(prop_value) ) {
                         LOG_ERROR("Type \"%s\" has invalid parent \"%s\"", utype->_id.print(), prop_value);
                         utype->_is_broken = true;
                         return false;
                     }
                     dep_list.pushParent(utype->_id, parent_id, prop_group);
                 }
             } else if( sIs("field_", prop_name) ) {
                 prop_name += 6; // strlen("field_");
                 idx ifld = -sIdxMax;

                 if( is_full_fetch ) {
                     if( utype->_fields.dim() && !field_path_dic.get(utype->_id) ) {
                         // for now, do not allow incomplete loading of fields into a type - hard to do correctly
                         LOG_ERROR("Type \"%s\" already had its field information loaded, refusing to overwrite", utype->_id.print());
                         return false;
                     }
                     ifld = field_path_dic.get(utype->_id, prop_group);
                     sUsrTypeField * fld = 0;
                     if( ifld >= 0 ) {
                         fld = utype->_fields.ptr(ifld);
                     } else {
                         // we have not seen field info with the first 2 elements of prop_group for this type before
                         ifld = utype->_fields.dim();
                         field_path_dic.set(utype->_id, prop_group, ifld);
                         fld = utype->_fields.addM(1);
                         new(fld) sUsrTypeField(true);
                         fld->_index = ifld;
                         fld->_owner_itype = fld->_definer_itype = utype->_itype;
                     }

                     if( strcmp(prop_name, "name") == 0 ) {
                         fld->_pos_name = fld->_pos_orig_name = sUsrTypeField::setString(prop_value);
                         *utype->_name2ifield.setString(canonicalCase(case_buf, prop_value)) = ifld;
                     } else if( strcmp(prop_name, "title") == 0 ) {
                         fld->_pos_title = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "type") == 0 ) {
                         if( sIs("type2", prop_value) ) {
                             prop_value += 5; // strlen("type2");
                             fld->_included_from_itype = INCLUDE_TYPE_EXPECTED; // magic flag indicating that a valid field_include_type is expected
                         } else {
                             fld->_included_from_itype = -1;
                         }
                         fld->_type = fieldTypeFromName(prop_value);
                         //LOG_WARNING("Type %s field %s type %s", type_id.print(), fld->name(), fld->typeName());
                     } else if( strcmp(prop_name, "parent") == 0 ) {
                         fld->_pos_parent_name = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "role") == 0 ) {
                         fld->_pos_role = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "is_key_fg") == 0 ) {
                         fld->_is_key = sString::parseBool(prop_value);
                     } else if( strcmp(prop_name, "is_readonly_fg") == 0 ) {
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
                     } else if( strcmp(prop_name, "is_optional_fg") == 0 ) {
                         fld->_is_optional = sString::parseBool(prop_value);
                     } else if( strcmp(prop_name, "is_multi_fg") == 0 ) {
                         fld->_is_multi = sString::parseBool(prop_value);
                     } else if( strcmp(prop_name, "is_hidden_fg") == 0 ) {
                         fld->_is_hidden = sString::parseBool(prop_value);
                     } else if( strcmp(prop_name, "brief") == 0 ) {
                         fld->_pos_brief = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "is_summary_fg") == 0 ) {
                         fld->_is_summary = sString::parseBool(prop_value);
                     } else if( strcmp(prop_name, "is_virtual_fg") == 0 ) {
                         fld->_is_virtual = sString::parseBool(prop_value);
                     } else if( strcmp(prop_name, "is_batch_fg") == 0 ) {
                         fld->_is_batch = sString::parseBool(prop_value);
                     } else if( strcmp(prop_name, "order") == 0 ) {
                         fld->_pos_order = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "default_value") == 0 ) {
                         fld->_pos_default_value = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "default_encoding") == 0 ) {
                         fld->_default_encoding = atoidx(prop_value);
                     } else if( strcmp(prop_name, "link_url") == 0 ) {
                         fld->_pos_link_url = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "constraint") == 0 ) {
                         fld->_pos_constraint = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "constraint_data") == 0 ) {
                         fld->_pos_constraint_data = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "constraint_description") == 0 ) {
                         fld->_pos_constraint_description = sUsrTypeField::setString(prop_value);
                     } else if( strcmp(prop_name, "description") == 0 ) {
                         fld->_pos_description = sUsrTypeField::setString(prop_value);
                     }
                 }

                 if( strcmp(prop_name, "include_type") == 0 ) {
                     sHiveId include_id;
                     if( !include_id.parse(prop_value) ) {
                         LOG_ERROR("Type \"%s\" has invalid field_include_type \"%s\" (group \"%s\")", utype->_id.print(), prop_value, prop_group);
                         return false;
                     }
                     dep_list.pushInclude(utype->_id, include_id, ifld);
                 }
             } else {
                 LOG_ERROR("Type \"%s\" has unexpected property \"%s\" = \"%s\"", utype->_id.print(), prop_name, prop_value);
             }

            return true;
        }

        bool assembleDeps()
        {
            dep_list.sort();
            sDic<idx> unique_includes; // key = included itype
            for(idx idep = 0; idep < dep_list.dim(); ) {
                idx istart = idep;
                const DepList::Entry & e_start = dep_list.getSorted(istart);
                if( sUsrType2 * utype = getRaw(e_start.type_id) ) {
                    unique_includes.empty();
                    for( idx iinc = 0; iinc < utype->_includes.dim(); iinc++ ) {
                        idx inc_itype = utype->_includes[iinc];
                        unique_includes.set(&inc_itype, sizeof(inc_itype));
                    }
                    for( ; idep < dep_list.dim() && e_start.type_id == dep_list.getSorted(idep).type_id; idep++ ) {
                        const DepList::Entry & e = dep_list.getSorted(idep);
                        LOG_TRACE("Got dep entry #%"DEC": %s (%s path)", idep, e.print(local_log_buf, false, false), e.index >= 0 ? static_cast<const char*>(dep_list.interned_strings.id(e.index)) : "no");
                        const DepList::Entry * e_prev = idep > istart ? &dep_list.getSorted(idep - 1) : 0;
                        sUsrType2 * udep = getRaw(e.dep_id);

                        if( !udep ) {
                            LOG_ERROR("%s which cannot be fetched", e.print(local_log_buf, true, false));
                            utype->_is_broken = true;
                            continue;
                        }

                        if( e.kind == DepList::Entry::eParent ) {
                            *utype->_parents.add(1) = udep->_itype;
                        } else if( e.kind == DepList::Entry::eChild ) {
                            *utype->_children.add(1) = udep->_itype;
                        } else if( e.kind == DepList::Entry::eInclude ) {
                            if( utype->_fields.dim() ) {
                                utype->_fields[e.index]._included_from_itype = udep->_itype;
                            }

                            if( !e_prev || e_prev->kind != e.kind || e_prev->dep_id != e.dep_id ) {
                                // copy only unique includes; note that we are assuming that for a given type,
                                // its field data will be loaded only once, so there is no need to check for
                                // pre-existing includes in utype->_includes before loop start
                                if( !unique_includes.get(&udep->_itype, sizeof(udep->_itype)) ) {
                                    *utype->_includes.add(1) = udep->_itype;
                                    unique_includes.set(&udep->_itype, sizeof(udep->_itype));
                                }
                            }
                        }
                    }
                } else {
                    LOG_ERROR("%s which cannot be fetched", e_start.print(local_log_buf, true, true));
                    if( sUsrType2 * udep = getRaw(e_start.dep_id) ) {
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
                    sUsrType2 & utype = *_types[itype];
                    forest.add(&utype);
                    for(idx ifld = 0; ifld < utype._fields.dim(); ifld++) {
                        sUsrTypeField * fld = utype._fields.ptr(ifld);
                        if( fld->_included_from_itype == INCLUDE_TYPE_EXPECTED ) {
                            LOG_ERROR("Type \"%s\" field \"%s\" is type \"type2*\" but its field_include_type was missing or invalid", utype.id().print(), fld->name());
                            fld->_is_broken = true;
                        }
                    }
                }
                sVec<idx> loaded_itypes;
                forest.makeTraversal(loaded_itypes);
                sUsrType2::linkFields(loaded_itypes);
            }

            return true;
        }
};

static sHiveId type_type_id; // caches ID of type "type" - which canonically ought to be id "type.1"

//static
void sUsrType2::loadFromObj(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch/* = false */, bool lazy_fetch_fields/* = false */)
{
    if( !name_len ) {
        name_len = sLen(name);
    }

    // if prefetch types were already prefetched, and name/type_id are among them or were not specified, nothing more to do here:
    if( _all_prefetched ) {
        if( (name && strncmp(name, "*", name_len) && getRaw(name, name_len) && (lazy_fetch_fields || getRaw(name, name_len)->_is_fetched)) ||
            (type_id && getRaw(*type_id) && (lazy_fetch_fields || getRaw(*type_id)->_is_fetched)) ||
            (!name && !type_id))
        {
            return;
        }
    }

    // name == "*" : load all
#define SP_TYPE_GET "sp_type_get_v6"
    std::auto_ptr<sSql::sqlProc> p(user.getProc(SP_TYPE_GET));
    if( name && strncmp(name, "*", name_len) == 0 ) {
        // load all types
        if( _all_deps_loaded ) {
            return;
        }
        _all_deps_loaded = true;
        p->Add("TRUE"); // all types
        if( no_prefetch && lazy_fetch_fields ) {
            // do not fetch full fields for any type
            p->Add("FALSE");
        } else {
            // load full fields only for prefetchable types
            p->Add("(f.`name` = 'prefetch' AND f.`value` > 0)");
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
            _all_prefetched = true;
        }
        p->Add(sql);
        if( lazy_fetch_fields ) {
            // do not fetch full fields for any type
            p->Add("FALSE");
        } else {
            // load full fields only for all selected types (the one specified by name/id + the prefetchable ones)
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
        unfetched_ids.cut();
        unfetched_full_fetch_ids.cut();
        if( p->resultOpen() && user.db().resultNext() ) {
            // first result is domain and objid of type "type" for us to cache
            if( !type_type_id && user.db().resultNextRow() ) {
                const idx domain_id_icol = user.db().resultColId("domainID");
                const idx obj_id_icol = user.db().resultColId("objID");
                type_type_id.set(user.db().resultUValue(domain_id_icol), user.db().resultUValue(obj_id_icol), 0);
            }
            // second result is the type data
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

                LOG_TRACE("Got row from " SP_TYPE_GET ": %s,%s,%s,\"%s\"", type_id.print(), prop_name, prop_group, prop_value);

                idx itype = -sIdxMax;
                sUsrType2 * utype = 0;
                if( idx * pitype = _name_or_id2itype.get(&type_id, sizeof(sHiveId)) ) {
                    itype = *pitype;
                    utype = _types[itype];
                    if( utype->_is_fetched ) {
                        // the entire type was already loaded; e.g. we fetched a few specific types,
                        // and then after we are fetching "*"
                        continue;
                    }
                } else {
                    itype = _types.dim();
                    utype = new sUsrType2(true);
                    *_types.add(1) = utype;
                    utype->_itype = itype;
                    utype->_id = type_id;
                    *_name_or_id2itype.set(&type_id, sizeof(sHiveId)) = itype;
                }

                ctx.utype = utype;

                if( is_full_fetch ) {
                    ctx.is_full_fetch = true;
                    *ctx.full_fetch_ids2itype.set(&type_id, sizeof(sHiveId)) = itype;
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
            if( !_name_or_id2itype.get(pdep, sizeof(sHiveId)) ) {
                *unfetched_ids.add(1) = *pdep;
                if( ctx.full_fetch_ids2itype.get(p_dep_of, sizeof(sHiveId)) ) {
                    *unfetched_full_fetch_ids.add(1) = *pdep;
                    LOG_TRACE("Type \"%s\" depends on \"%s\" which was not fetched yet, adding to fetch list (will fetch all fields)", ctx.dep_list.vec[id].type_id.print(), pdep->print());
                } else {
                    LOG_TRACE("Type \"%s\" depends on \"%s\" which was not fetched yet, adding to fetch list (in minimal mode)", ctx.dep_list.vec[id].type_id.print(), pdep->print());
                }
            }
        }

        if( unfetched_ids.dim() ) {
            p.reset(user.getProc(SP_TYPE_GET));
            sStr sql;
            p->Add(user.db().protectValue(sql, sSql::exprInList(sql_buf, "o.domainID", "o.objID", unfetched_ids)));
            if( unfetched_full_fetch_ids.dim() == 0 ) {
                p->Add("FALSE"); // load all unfetched ids in minimal mode
            } else if( unfetched_full_fetch_ids.dim() == unfetched_ids.dim() ) {
                p->Add("TRUE"); // load all unfetched ids in full mode
            } else {
                sql.cut0cut();
                p->Add(user.db().protectValue(sql, sSql::exprInList(sql_buf, "o.domainID", "o.objID", unfetched_full_fetch_ids)));
            }
            p->Add(type_type_id.domainId()).Add(type_type_id.objId());
        }
    } while( unfetched_ids.dim() );

    // assemble dependency info
    ctx.assembleDeps();
    ctx.linkFields();
}

class sUsrType2::JSONLoader : public sUsrPropSet
{
    private:
        struct BuiltinField {
            const char * name;
            sUsrTypeField::EType type;
            bool is_array_row;
            bool is_key;
            bool is_multi;
            bool is_global_multi;
            const char * parent;
            const char * flattened_non_array_row_parent;
        };
        static const BuiltinField _builtin_fields[];
        static const BuiltinField _unknown_field;
        static sDic<const BuiltinField*> _builtin_fields_dic;

        sUsrType2::LoadFromObjContext _ctx;
        sStr _value_buf;

    public:
        JSONLoader(const sUsr & usr) : sUsrPropSet(usr), _ctx(usr)
        {
            if( !_builtin_fields_dic.dim() ) {
                for(const BuiltinField * f = _builtin_fields; f && f->name; f++) {
                    *_builtin_fields_dic.setString(f->name) = f;
                }
            }
        }

        sUsrType2::LoadFromObjContext & getCtx() { return _ctx; }

        // overridden methods
        virtual bool ensureUTypeFor(ObjLoc * prop_obj, sJSONParser::ParseNode & node, sJSONParser & parser)
        {
            if( strncmp_exact(node.val.str, node.val_str_len, "type", 4) != 0 ) {
                parser.setValueError(node, "type type expeced");
                return false;
            }
            return true;
        }
        virtual const char * getUTypeName(ObjLoc * prop_obj) const { return "type"; }
        virtual const sHiveId & getUTypeId(ObjLoc * prop_obj) const
        {
            static sHiveId type_type_id;
            if( !type_type_id) {
                type_type_id.set("type", 1, 0);
            }
            return type_type_id;
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
            sUsrType2 * utype = 0;
            if( idx * pitype = _name_or_id2itype.get(&prop_obj->obj.id, sizeof(sHiveId)) ) {
                itype = *pitype;
                utype = _types[itype];
                if( utype->_is_fetched ) {
                    // the entire type was already loaded
                    _ctx.resetType();
                    return true;
                }
            } else {
                itype = sUsrType2::_types.dim();
                utype = new sUsrType2(true);
                *sUsrType2::_types.add(1) = utype;
                utype->_itype = itype;
                utype->_id = prop_obj->obj.id;
                *sUsrType2::_name_or_id2itype.set(&prop_obj->obj.id, sizeof(sHiveId)) = itype;
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
            // TODO
            return true;
        }
        virtual udx propSet(ObjLoc * prop_obj, const sUsrPropSet::TypeField * fld, const char * path, const char * value, udx value_len)
        {
            if( _ctx.utype ) {
                _value_buf.cutAddString(0, value, value_len);
                // ignore return value: allow partially-loaded types, just print debugging message
                _ctx.accumulateProp(fldName(fld), _value_buf.ptr(), path);
            }
            return 1;
        }
        virtual bool setPermission(ObjLoc * prop_obj, sJSONParser::ParseNode & node, Perm & perm)
        {
            // TODO
            return true;
        }
        virtual const char * fldName(const TypeField * fld) const
        {
            return static_cast<const BuiltinField*>(fld)->name;
        }
        virtual sUsrTypeField::EType fldType(const TypeField * fld) const
        {
            return static_cast<const BuiltinField*>(fld)->type;
        }
        virtual bool fldNeedsValidation(const TypeField * fld) const
        {
            return static_cast<const BuiltinField*>(fld) != &_unknown_field;
        }
        virtual bool fldCanHaveValue(const TypeField * fld) const
        {
            sUsrTypeField::EType t = static_cast<const BuiltinField*>(fld)->type;
            return t != sUsrTypeField::eArray && t != sUsrTypeField::eList;
        }
        virtual bool fldCanSetValue(const TypeField * fld) const
        {
            return fldCanHaveValue(fld);
        }
        virtual bool fldIsArrayRow(const TypeField * fld) const
        {
            return static_cast<const BuiltinField*>(fld)->is_array_row;
        }
        virtual bool fldIsKey(const TypeField * fld) const
        {
            return static_cast<const BuiltinField*>(fld)->is_key;
        }
        virtual bool fldIsMulti(const TypeField * fld) const
        {
            return static_cast<const BuiltinField*>(fld)->is_multi;
        }
        virtual bool fldIsGlobalMulti(const TypeField * fld) const
        {
            return static_cast<const BuiltinField*>(fld)->is_global_multi;
        }
        virtual const TypeField * fldGet(ObjLoc * prop_obj, const char * name, idx name_len) const
        {
            const BuiltinField ** pret = _builtin_fields_dic.get(name, name_len);
            if( pret && *pret ) {
                return *pret;
            } else {
                return &_unknown_field;
            }
        }
        virtual const TypeField * fldParent(const TypeField * fld) const
        {
            if( const char * parname = static_cast<const BuiltinField*>(fld)->parent ) {
                const BuiltinField ** pret = _builtin_fields_dic.get(parname);
                return pret ? *pret : 0;
            } else {
                return 0;
            }
        }
        virtual const TypeField * fldFlattenedNonArrayRowParent(const sUsrPropSet::TypeField * fld) const
        {
            if( const char * parname = static_cast<const BuiltinField*>(fld)->flattened_non_array_row_parent ) {
                const BuiltinField ** pret = _builtin_fields_dic.get(parname);
                return pret ? *pret : 0;
            } else {
                return 0;
            }
        }
};

//static
bool sUsrType2::loadFromJSON(const sUsr & user, const char * buf, idx buf_len/* = 0 */)
{
    sUsrType2::JSONLoader json_loader(user);
    json_loader.setSrc(buf, buf_len);
    if( !json_loader.run(0, sUsrPropSet::fInvalidUserGroupNonFatal) ) {
        fprintf(stderr, "Failed to load type(s) from JSON: %s\n", json_loader.getErr());
        return false;
    }
    json_loader.getCtx().assembleDeps();
    json_loader.getCtx().linkFields();

    return true;
}

//static
// Must not be called from sUsrType2::loadFromObj due to recursion
bool sUsrType2::loadFromJSONCache(const sUsr & user)
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

        std::auto_ptr<sSql::sqlProc> p(user.getProc("sp_type_get_latest_mtime"));
        p->Add(type_type_id.domainId()).Add(type_type_id.objId());
        if( p->resultOpen() && user.db().resultNext() ) {
            // first result is domain and objid of type "type" for us to cache
            if( !type_type_id && user.db().resultNextRow() ) {
                const idx domain_id_icol = user.db().resultColId("domainID");
                const idx obj_id_icol = user.db().resultColId("objID");
                type_type_id.set(user.db().resultUValue(domain_id_icol), user.db().resultUValue(obj_id_icol), 0);
            }
            // second result is the type data
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
        // for a systemwide cache, we want to read *all* types (with permissions), so superuser mode
        sUsr superuser("qpride", true);
        if( !superuser.Id() ) {
            LOG_TRACE("%s", "Failed to enter superuser mode for type loading");
            return false;
        }

        // in order to use sUsr::objs2() on type type objects, we first must have type type loaded in sUsrType2.
        if( !sUsrType2::get("type") ) {
            sUsrType2::loadFromObj(superuser, "type", 4, 0);
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
        superuser.propExport(ids, printer, true, true);
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

// note: all of these are "string" to avoid future compatibility issues
const sUsrType2::JSONLoader::BuiltinField sUsrType2::JSONLoader::_builtin_fields[] = {
    { "created", sUsrTypeField::eString, false, false, false, false, 0, 0 },
    { "modified", sUsrTypeField::eString, false, false, false, false, 0, 0 },
    { "name", sUsrTypeField::eString, false, true, false, false, 0, 0 },
    { "title", sUsrTypeField::eString, false, false, false, false, 0, 0 },
    { "description", sUsrTypeField::eString, false, false, false, false, 0, 0 },
    { "parent", sUsrTypeField::eString, false, false, true, true, 0, 0 },
    { "is_abstract_fg", sUsrTypeField::eString, false, false, false, false, 0, 0 },
    { "singleton", sUsrTypeField::eString, false, false, false, false, 0, 0 },
    { "prefetch", sUsrTypeField::eString, false, false, false, false, 0, 0 },
    { "fields", sUsrTypeField::eArray, false, false, false, false, 0, 0 },
    { "_row_fields", sUsrTypeField::eList, true, false, true, true, "fields", 0 },
    { "field_basics", sUsrTypeField::eList, false, false, true, true, "_row_fields", "fields" },
    { "field_flags", sUsrTypeField::eList, false, false, true, true, "_row_fields", "fields" },
    { "field_contraint_list", sUsrTypeField::eList, false, false, true, true, "_row_fields", "fields" },
    { "field_presentation", sUsrTypeField::eList, false, false, true, true, "_row_fields", "fields" },
    { "field_name", sUsrTypeField::eString, false, false, false, true, "field_basics", "fields" },
    { "field_title", sUsrTypeField::eString, false, false, false, true, "field_basics", "fields" },
    { "field_type", sUsrTypeField::eString, false, false, false, true, "field_basics", "fields" },
    { "field_parent", sUsrTypeField::eString, false, false, false, true, "field_basics", "fields" },
    { "field_order", sUsrTypeField::eReal, false, false, false, true, "field_basics", "fields" },
    { "field_default_value", sUsrTypeField::eString, false, false, false, true, "field_basics", "fields" },
    { "field_default_encoding", sUsrTypeField::eString, false, false, false, true, "field_basics", "fields" },
    { "field_include_type", sUsrTypeField::eString, false, false, false, true, "field_basics", "fields" },
    { "field_role", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_key_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_readonly_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_optional_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_multi_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_virtual_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_hidden_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_summary_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_is_batch_fg", sUsrTypeField::eString, false, false, false, true, "field_flags", "fields" },
    { "field_constraint", sUsrTypeField::eString, false, false, false, true, "field_contraint_list", "fields" },
    { "field_constraint_data", sUsrTypeField::eString, false, false, false, true, "field_contraint_list", "fields" },
    { "field_constraint_description", sUsrTypeField::eString, false, false, false, true, "field_contraint_list", "fields" },
    { "field_brief", sUsrTypeField::eString, false, false, false, true, "field_presentation", "fields" },
    { "field_link_url", sUsrTypeField::eString, false, false, false, true, "field_presentation", "fields" },
    { "field_description", sUsrTypeField::eString, false, false, false, true, "field_presentation", "fields" },
    0
};
const sUsrType2::JSONLoader::BuiltinField sUsrType2::JSONLoader::_unknown_field = { "__unknown_field__", sUsrTypeField::eInvalid, false, false, false, false, 0, 0 };
sDic<const sUsrType2::JSONLoader::BuiltinField*> sUsrType2::JSONLoader::_builtin_fields_dic;


//static
void sUsrType2::loadDeps(const sUsr & user)
{
    sStr case_buf;
    sStr parent_names00;
    sVec<idx> parent_names_vec(sMex::fSetZero);
    // table of types
    if( user.db().resultOpen("SELECT * FROM UPType") && user.db().resultNext() ) {
        const idx type_id_icol = user.db().resultColId("type_id");
        const idx name_icol = user.db().resultColId("name");
        const idx title_icol = user.db().resultColId("title");
        const idx description_icol = user.db().resultColId("description");
        const idx parent_icol = user.db().resultColId("parent");
        const idx is_virtual_fg_icol = user.db().resultColId("is_virtual_fg");
        const idx prefetch_fg_icol = user.db().resultColId("prefetch_fg");
        while( user.db().resultNextRow() ) {
            sHiveId type_id(sUsrType2::type_type_domain_id, user.db().resultUValue(type_id_icol), 0);
            const char * name = user.db().resultValue(name_icol);
            const char * canon_name = canonicalCase(case_buf, name);
            const char * title = user.db().resultValue(title_icol);
            const char * description = user.db().resultValue(description_icol);
            const char * parent_str = user.db().resultValue(parent_icol);
            LOG_TRACE("Got row from UPType: \"%s\" (%s)", name, type_id.print());

            bool is_virtual = sString::parseBool(user.db().resultValue(is_virtual_fg_icol));
            bool is_prefetch = sString::parseBool(user.db().resultValue(prefetch_fg_icol));

            idx itype = _types.dim();
            sUsrType2 * utype = new sUsrType2;
            *_types.add(1) = utype;
            utype->_itype = itype;
            parent_names_vec.resize(itype + 1);
            if( canon_name[0] >= 'a' && canon_name[0] <= 'z' ) {
                // valid type, not "--DELETED" or similar
                utype->_id = type_id;
                *_name_or_id2itype.setString(canon_name) = itype;
                *_name_or_id2itype.set(&type_id, sizeof(type_id)) = itype;

                utype->_pos_name = _name_buf.length();
                _name_buf.addString(name);
                _name_buf.add0();

                utype->_pos_title = _name_buf.length();
                _name_buf.addString(title);
                _name_buf.add0();

                utype->_pos_description = _name_buf.length();
                _name_buf.addString(description);
                _name_buf.add0();

                utype->_is_virtual = is_virtual;
                utype->_is_prefetch = is_prefetch;

                parent_names00.add0();
                parent_names_vec[itype] = parent_names00.length();
                sString::searchAndReplaceSymbols(&parent_names00, parent_str, 0, ",", 0, 0, true, true, true, true);
                parent_names00.add0(2);
            }
        }
        user.db().resultClose();
    }

    // tie together parents and children
    sMex deps_buf;
    sVec< sLst<sUsrType2*> > includes_lists;
    includes_lists.resize(_types.dim());
    for(idx i=0; i<_types.dim(); i++) {
        includes_lists[i].init(&deps_buf);
    }
    for(idx itype = 0; itype < _types.dim(); itype++) {
        for(const char * par_name = parent_names00.ptr(parent_names_vec[itype]); par_name && *par_name; par_name = sString::next00(par_name)) {
            sUsrType2 * par = getRaw(par_name);
            if( !par ) {
                LOG_ERROR("Type \"%s\" (%s) has invalid parent \"%s\"", _types[itype]->name(), _types[itype]->_id.print(), par_name);
                continue;
            }
            LOG_TRACE("Type \"%s\" (%s) has parent %"DEC" \"%s\" (%s)", _types[itype]->name(), _types[itype]->_id.print(), _types[itype]->_parents.dim(), par->name(), par->_id.print());
            *_types[itype]->_parents.add(1) = par->_itype;
            *_types[par->_itype]->_children.add(1) = itype;
        }
        if( !_types[itype]->_parents.dim() ) {
            *_roots.add(1) = _types[itype];
        }
    }
    // tie together includers and includes
    sDic<idx> unique_includes;
    sStr unique_include_key;
    if( user.db().resultOpen("SELECT type_id, name, constraint_data FROM UPTypeField WHERE type IN ('type2array', 'type2list')") && user.db().resultNext() ) {
        const idx type_id_icol = user.db().resultColId("type_id");
        const idx name_icol = user.db().resultColId("name");
        const idx constraint_data_icol = user.db().resultColId("constraint_data");
        while( user.db().resultNextRow() ) {
            sHiveId type_id(sUsrType2::type_type_domain_id, user.db().resultUValue(type_id_icol), 0);
            const char * fld_name = user.db().resultValue(name_icol);
            const char * include_name = user.db().resultValue(constraint_data_icol);
            sUsrType2 * utype = getRaw(type_id);

            if( !utype || utype->_id != type_id ) {
                LOG_ERROR("Field \"%s\" belongs to invalid type ID %s", fld_name, type_id.print());
                continue;
            }
            sUsrType2 * inc = getRaw(include_name);
            if( !inc ) {
                LOG_ERROR("Type \"%s\" (%s) has invalid include \"%s\" pulled by field \"%s\"", utype->name(), utype->id().print(), include_name, fld_name);
                continue;
            }
            LOG_TRACE("Type \"%s\" (%s) has include \"%s\" (%s) via field \"%s\"", utype->name(), utype->id().print(), include_name, inc->id().print(), fld_name);

            unique_include_key.cut0cut(0);
            utype->id().print(unique_include_key);
            unique_include_key.add0();
            inc->id().print(unique_include_key);
            unique_include_key.add0(2);
            if( !unique_includes.get(unique_include_key.ptr(), unique_include_key.length()) ) {
                // includes may legitimately repeat; we only want the unique ones
                *utype->_includes.add(1) = inc->_itype;
                unique_includes.set(unique_include_key.ptr(), unique_include_key.length());
            }
            *includes_lists[utype->_itype].add(1) = inc;
        }
        user.db().resultClose();
    }
    // mark descendents of base_user_type and base_system_type
    sDic<idx> seen_descendents;
    if( sUsrType2 * base_user_type = getRaw("base_user_type") ) {
        base_user_type->_is_user = eLazyTrue;
        seen_descendents.empty();
        base_user_type->recurseDescendents(seen_descendents);
        for(idx i=0; i<seen_descendents.dim(); i++) {
            if( *seen_descendents.ptr(i) ) {
                _types[*static_cast<idx*>(seen_descendents.id(i))]->_is_user = eLazyTrue;
            }
        }
    }
    if( sUsrType2 * base_system_type = getRaw("base_system_type") ) {
        base_system_type->_is_system = eLazyTrue;
        seen_descendents.empty();
        base_system_type->recurseDescendents(seen_descendents);
        for(idx i=0; i<seen_descendents.dim(); i++) {
            if( *seen_descendents.ptr(i) ) {
                _types[*static_cast<idx*>(seen_descendents.id(i))]->_is_system = eLazyTrue;
            }
        }
    }
}

//static
sUsrType2 * sUsrType2::load(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch/* = false*/, bool lazy_fetch_fields/* = false */)
{
    sUsrType2 * utype = 0;
    if( !name_len ) {
        name_len = sLen(name);
    }

    // Important optimization for find(): do not perform any queries if name and type_id both empty.
    if( name_len || (type_id && *type_id) ) {
        if( useTypeUPObj() ) {
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
        } else {
            DepForest forest;

            if( !_types.dim() ) {
                loadDeps(user);
                for(idx itype = 0; itype < _types.dim(); itype++) {
                    if( _types[itype]->_is_prefetch ) {
                        LOG_TRACE("Prefetch \"%s\" (%s) in load forest", _types[itype]->name(), _types[itype]->id().print());
                        forest.add(_types[itype]);
                    }
                }
            }

            if( name && strncmp(name, "*", name_len) != 0 ) {
                utype = getRaw(name, name_len);
            } else if( type_id ) {
                utype = getRaw(*type_id);
            }
            if( utype ) {
                LOG_TRACE("Add \"%s\" (%s) to load forest", utype->name(), utype->id().print());
                forest.add(utype);
            }

            loadFields(user, &forest);
        }
    }

    return utype;
}

struct sUsrType2::SetFieldChildrenParam
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

static idx fldCompare(void * param, void * arr, idx i1, idx i2)
{
    const sUsrTypeField * fields = static_cast<const sUsrTypeField*>(param);
    const idx * indices = static_cast<const idx*>(arr);
    return fields[indices[i1]].cmp(fields + indices[i2]);
}

//static
void sUsrType2::loadFields(const sUsr & user, sUsrType2::DepForest * type_forest)
{
    sStr case_buf;

    sVec<idx> itypes;
    type_forest->makeTraversal(itypes);
    if( !itypes.dim() ) {
        return;
    }

    // Own defined fields

    // We want to turn list of type indices into an efficient sql WHERE clause. Assuming
    // that the number of distinct domain IDs is small, this is best accomplished by sorting
    // the ids using default sHiveId comparison (by domain ID first), then handling each
    // domain ID separately.
    sStr sql("SELECT * from UPTypeField WHERE ");
    sVec<sHiveId> ids_for_sql(sMex::fExactSize);
    sVec<udx> obj_ids_for_sql;
    ids_for_sql.resize(itypes.dim());
    for(idx i = 0; i < itypes.dim(); i++) {
        ids_for_sql[i] = _types[itypes[i]]->id();
    }
    sSql::exprInList(sql, 0, "type_id", ids_for_sql);
    sql.printf(" ORDER BY type_id");
    LOG_TRACE("%s", sql.ptr());
    if( user.db().resultOpen(sql.ptr()) && user.db().resultNext() ) {
        const idx type_id_icol = user.db().resultColId("type_id");
        const idx name_icol = user.db().resultColId("name");
        const idx title_icol = user.db().resultColId("title");
        const idx type_icol = user.db().resultColId("type");
        const idx parent_icol = user.db().resultColId("parent");
        const idx role_icol = user.db().resultColId("role");
        const idx is_key_fg_icol = user.db().resultColId("is_key_fg");
        const idx is_readonly_fg_icol = user.db().resultColId("is_readonly_fg");
        const idx is_optional_fg_icol = user.db().resultColId("is_optional_fg");
        const idx is_multi_fg_icol = user.db().resultColId("is_multi_fg");
        const idx is_hidden_fg_icol = user.db().resultColId("is_hidden_fg");
        const idx brief_icol = user.db().resultColId("brief");
        const idx is_summary_fg_icol = user.db().resultColId("is_summary_fg");
        const idx is_virtual_fg_icol = user.db().resultColId("is_virtual_fg");
        const idx is_batch_fg_icol = user.db().resultColId("is_batch_fg");
        const idx order_icol = user.db().resultColId("order");
        const idx default_value_icol = user.db().resultColId("default_value");
        const idx default_encoding_icol = user.db().resultColId("default_encoding");
        const idx link_url_icol = user.db().resultColId("link_url");
        const idx constraint_icol = user.db().resultColId("constraint");
        const idx constraint_data_icol = user.db().resultColId("constraint_data");
        const idx constraint_description_icol = user.db().resultColId("constraint_description");
        const idx description_icol = user.db().resultColId("description");
        while( user.db().resultNextRow() ) {
            sHiveId type_id(sUsrType2::type_type_domain_id, user.db().resultUValue(type_id_icol), 0);
            sUsrType2 * utype = getRaw(type_id);
            if( !utype || !type_forest->has(utype->_itype) ) {
                LOG_ERROR("Unexpected type id %s in db output", type_id.print());
                continue;
            }

            LOG_TRACE("Get row from UPTypeField: \"%s\" for type \"%s\" (%s) to load forest", user.db().resultValue(name_icol), utype->name(), utype->id().print());

            idx ifld = utype->_fields.dim();
            *utype->_name2ifield.setString(canonicalCase(case_buf, user.db().resultValue(name_icol))) = ifld;
            sUsrTypeField * fld = utype->_fields.addM(1);
            new(fld) sUsrTypeField;

            fld->_index = ifld;
            fld->_owner_itype = fld->_definer_itype = utype->_itype;
            fld->_pos_name = fld->_pos_orig_name = sUsrTypeField::setString(user.db().resultValue(name_icol));
            fld->_pos_title = sUsrTypeField::setString(user.db().resultValue(title_icol));
            const char * fld_type_name = user.db().resultValue(type_icol);

            if( sIs(fld_type_name, "type2list") || sIs(fld_type_name, "type2array") ) {
                sUsrType2 * include_utype = getRaw(user.db().resultValue(constraint_data_icol));
                if( include_utype ) {
                    fld->_included_from_itype = include_utype->_itype;
                } else {
                    LOG_ERROR("Invalid type name \"%s\" as constraint_data in type2* field \"%s\" in type \"%s\" (%s)", user.db().resultValue(constraint_data_icol), user.db().resultValue(name_icol), utype->name(), utype->id().print());
                    fld->_is_broken = true;
                }
                fld->_type = sIs(fld_type_name, "type2list") ? sUsrTypeField::eList : sUsrTypeField::eArray;
            } else {
                fld->_included_from_itype = -1;
                fld->_type = fieldTypeFromName(user.db().resultValue(type_icol));
            }

            fld->_pos_parent_name = sUsrTypeField::setString(user.db().resultValue(parent_icol));
            fld->_pos_role = sUsrTypeField::setString(user.db().resultValue(role_icol));
            fld->_is_key = sString::parseBool(user.db().resultValue(is_key_fg_icol));

            switch(idx ro = user.db().resultIValue(is_readonly_fg_icol)) {
                case sUsrTypeField::eReadWrite:
                case sUsrTypeField::eWriteOnce:
                case sUsrTypeField::eSubmitOnce:
                case sUsrTypeField::eReadOnly:
                case sUsrTypeField::eReadOnlyAutofill:
                    fld->_readonly = (sUsrTypeField::EReadOnly)ro;
                    break;
                default:
                    LOG_ERROR("Invalid is_readonly_fg code %"DEC" in field \"%s\" in type \"%s\" (%s)", ro, user.db().resultValue(name_icol), utype->name(), utype->id().print());
                    fld->_readonly = sUsrTypeField::eReadOnly;
                    fld->_is_broken = true;
            }

            fld->_is_optional = sString::parseBool(user.db().resultValue(is_optional_fg_icol));
            fld->_is_multi = sString::parseBool(user.db().resultValue(is_multi_fg_icol));
            fld->_is_hidden = sString::parseBool(user.db().resultValue(is_hidden_fg_icol));
            fld->_is_summary = sString::parseBool(user.db().resultValue(is_summary_fg_icol));
            fld->_is_virtual = sString::parseBool(user.db().resultValue(is_virtual_fg_icol));
            fld->_is_batch = sString::parseBool(user.db().resultValue(is_batch_fg_icol));

            fld->_pos_brief = sUsrTypeField::setString(user.db().resultValue(brief_icol));
            fld->_pos_order = sUsrTypeField::setString(user.db().resultValue(order_icol));
            fld->_pos_default_value = sUsrTypeField::setString(user.db().resultValue(default_value_icol));
            fld->_default_encoding = user.db().resultIValue(default_encoding_icol);
            fld->_pos_link_url = sUsrTypeField::setString(user.db().resultValue(link_url_icol));
            fld->_pos_constraint = sUsrTypeField::setString(user.db().resultValue(constraint_icol));
            fld->_pos_constraint_data = sUsrTypeField::setString(user.db().resultValue(constraint_data_icol));
            fld->_pos_constraint_description = sUsrTypeField::setString(user.db().resultValue(constraint_description_icol));
            fld->_pos_description = sUsrTypeField::setString(user.db().resultValue(description_icol));
        }
        user.db().resultClose();
    }

    linkFields(itypes);
}

//static
void sUsrType2::linkFields(sVec<idx> & itypes)
{
    sStr inc_buf, case_buf, subst_buf;

    // builtin system fields
    struct
    {
        const char * name, * title;
        idx pos_name, pos_title;
        sUsrTypeField::EType type;
        bool is_multi;
        bool is_sysinternal;
    } system_fields[] = {
        { "_type", "Type name", -1, -1, sUsrTypeField::eString, false, false },
        { "_parent", "Parent object", -1, -1, sUsrTypeField::eObj, true, false },
        { "_brief", "Summary", -1, -1, sUsrTypeField::eString, false, false },
        { "_dir", "Location", -1, -1, sUsrTypeField::eString, true, true },
        { "_perm", "Permissions", -1, -1, sUsrTypeField::eString, true, false }
    };

    // system fields, hierarchy
    sDic<idx> overridden_dic, broken_ifld_dic;
    SetFieldChildrenParam children_param;
    sStr array_row_name;

    // inclusions and inherited fields
    for(idx it=0; it<itypes.dim(); it++) {
        sUsrType2 & utype = *_types[itypes[it]];
        utype._dim_explicit_fields = utype._fields.dim(); // fields explicitly defined in the database

        LOG_TRACE("Inheritance and inclusion for type '%s'", utype.name());

        // add inherited fields : due to traversal of type_ids, at this point they are all ready
        overridden_dic.empty();
        idx cnt_utype_par = utype.dimParents();
        for(idx itp=0; itp<cnt_utype_par; itp++) {
            sUsrType2 & utype_par = *_types[utype._parents[itp]];
            // traverse from root fields down : if utype_par's list/array field with children is overridden by utype's scalar field, the children must be ignored
            LOG_TRACE("Inheriting %"DEC" fields from parent %s to child %s", utype_par._root_ifields.dim(), utype_par.name(), utype.name());
            for(idx irif=0; irif<utype_par._root_ifields.dim(); irif++) {
                utype.inheritField(utype_par._fields.ptr(utype_par._root_ifields[irif]), overridden_dic, case_buf);
            }
        }

        // add inclusions - but only from the fields explicitly defined for this type (inherited ones were handled by the type's parents)
        utype._dim_inherited_fields = utype._fields.dim() - utype._dim_explicit_fields;
        for(idx ifld=0; ifld<utype._dim_explicit_fields; ifld++) {
            idx included_from_itype = utype._fields[ifld]._included_from_itype;
            if( included_from_itype >= 0 ) {
                if( !_types[included_from_itype]->_is_fetched ) {
                    LOG_ERROR("Type \"%s\" (%s): field \"%s\" (%"DEC") includes from type \"%s\" whose fields have not yet been processed!", utype.name(), utype.id().print(), utype._fields[ifld].name(), ifld, _types[included_from_itype]->name());
                }
                // inclusions : field "foo" included via type2* field "bar" gets added as "bar_foo"
                inc_buf.printf(0, "$(%s_", utype._fields[ifld].name());
                inc_buf.add0(2);
                idx inc_buf_name_pos = inc_buf.length();
                for(idx iinc = 0; iinc < _types[included_from_itype]->_fields.dim(); iinc++) {
                    const sUsrTypeField * inc_src_fld = _types[included_from_itype]->_fields.ptr(iinc);
                    if( !inc_src_fld->_is_broken && inc_src_fld->name()[0] != '_' ) {
                        LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%"DEC") includes field \"%s\" from type \"%s\"", utype.name(), utype.id().print(), utype._fields[ifld].name(), ifld, inc_src_fld->name(),
                            _types[inc_src_fld->_definer_itype]->name());
                        inc_buf.printf(inc_buf_name_pos, "%s_%s", utype._fields[ifld].name(), inc_src_fld->name());
                        inc_buf.add0(2);

                        const char * canon_inc_name = canonicalCase(case_buf, inc_buf.ptr(inc_buf_name_pos));
                        if( utype._name2ifield.get(canon_inc_name) ) {
                            LOG_ERROR("Type \"%s\" (%s) cannot add included field \"%s\" from type \"%s\" (%s) because field \"%s\" already exists in the type (via explicit definition or inheritance)", utype.name(), utype.id().print(), inc_src_fld->name(), _types[included_from_itype]->name(), _types[included_from_itype]->id().print(local_log_buf), inc_buf.ptr(inc_buf_name_pos));
                            // Is this an error? or should it be a warning?
                            continue;
                        }

                        idx iinc_dst = utype._fields.dim();
                        sUsrTypeField * inc_dst_fld = utype._fields.addM(1);
                        new (inc_dst_fld) sUsrTypeField;
                        *utype._name2ifield.setString(canon_inc_name) = iinc_dst;
                        canon_inc_name = 0; // guard against realloc

                        *inc_dst_fld = *inc_src_fld;
                        inc_dst_fld->_index = iinc_dst;
                        inc_dst_fld->_pos_name = sUsrTypeField::setString(inc_buf.ptr(inc_buf_name_pos));
                        inc_dst_fld->_owner_itype = utype._itype;

                        subst_buf.cut(0);
                        // Are there other fields which are allowed to use "$(fldname)" syntax?
                        sUsrTypeField::replaceString(inc_dst_fld->_pos_brief, "$("__, inc_buf.ptr(0), subst_buf);
                        sUsrTypeField::replaceString(inc_dst_fld->_pos_constraint_data, "$("__, inc_buf.ptr(0), subst_buf);
                        sUsrTypeField::replaceString(inc_dst_fld->_pos_default_value, "$("__, inc_buf.ptr(0), subst_buf);

                        if( sLen(sUsrTypeField::getString(inc_dst_fld->_pos_parent_name)) ) {
                            idx inc_buf_par_name_pos = inc_buf.length();
                            inc_buf.printf("%s_%s", utype._fields[ifld].name(), sUsrTypeField::getString(inc_dst_fld->_pos_parent_name));
                            inc_dst_fld->_pos_parent_name = sUsrTypeField::setString(inc_buf.ptr(inc_buf_par_name_pos));
                        } else {
                            inc_dst_fld->_pos_parent_name = utype._fields[ifld]._pos_name;
                        }
                        LOG_TRACE("Type \"%s\" (%s): create included field \"%s\" with parent \"%s\"", utype.name(), utype.id().print(), inc_dst_fld->name(), sUsrTypeField::getString(inc_dst_fld->_pos_parent_name));
                    }
                }
            }
        }
        utype._dim_included_fields = utype._fields.dim() - (utype._dim_explicit_fields + utype._dim_inherited_fields);

        broken_ifld_dic.empty();
        children_param.empty();

        // add system fields
        for(idx i=0; i<sDim(system_fields); i++) {
            idx ifield = utype._fields.dim();
            *utype._name2ifield.setString(system_fields[i].name) = ifield;
            sUsrTypeField * fld = utype._fields.addM(1);
            new(fld) sUsrTypeField;
            if( it == 0 ) {
                system_fields[i].pos_name = sUsrTypeField::setString(system_fields[i].name);
                system_fields[i].pos_title = sUsrTypeField::setString(system_fields[i].title);
            }
            fld->_owner_itype = utype._itype;
            fld->_definer_itype = -1;
            fld->_pos_name = fld->_pos_orig_name = system_fields[i].pos_name;
            fld->_pos_title = system_fields[i].pos_title;
            fld->_type = system_fields[i].type;
            fld->_is_multi = system_fields[i].is_multi;
            fld->_is_sysinternal = system_fields[i].is_sysinternal;
            fld->_is_hidden = true;
            fld->_is_virtual = true;
            fld->_readonly = sUsrTypeField::eReadOnly;
        }

        // hierarchy
        // step 1 : link child -> parent; ensure array rows exist; preliminary parent -> child links; find orphans
        idx cnt_non_array_row = utype._fields.dim();
        for(idx ifld=0; ifld<cnt_non_array_row; ifld++) {
            sUsrTypeField * fld = utype._fields.ptr(ifld);
            const char * fld_parent_name = sUsrTypeField::getString(fld->_pos_parent_name);
            if( sLen(fld_parent_name) ) {
                if( const idx * pifld_par = utype._name2ifield.get(canonicalCase(case_buf, fld_parent_name)) ) {
                    const idx ifld_par = *pifld_par; // stable value in case utype._name2ifield reallocs
                    pifld_par = 0; // detect invalid use
                    sUsrTypeField * par_fld = utype._fields.ptr(ifld_par);
                    switch( par_fld->type() ) {
                        case sUsrTypeField::eList:
                            fld->_parent = ifld_par;
                            children_param.addChild(ifld_par, ifld);
                            break;
                        case sUsrTypeField::eArray: {
                            array_row_name.cut(0);
                            idx ifld_arr_row = utype.ensureArrayFieldRow(par_fld, array_row_name, case_buf);

                            // realloc guard
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
                LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%"DEC") is a root field", utype.name(), utype.id().print(), utype._fields[ifld].name(), ifld);
            }
        }
        sSort::sortSimpleCallback<idx>(&fldCompare, utype._fields.ptr(), utype._root_ifields.dim(), utype._root_ifields.ptr());

        // step 2 : pack parent -> child links for quick access
        for(idx irif=0; irif<utype._root_ifields.dim(); irif++) {
            utype.setFieldChildren(utype._root_ifields[irif], utype._root_ifields[irif], &children_param, case_buf);
        }

        // step 3 : fields which were not seen by setFieldChildren() do not descend from any root field, so mark as broken
        for(idx ifld=0; ifld<utype._fields.dim(); ifld++) {
            const sUsrTypeField * fld = utype._fields.ptr(ifld);
            if( !children_param.done_dic.get(&ifld, sizeof(ifld)) ) {
                LOG_ERROR("Type \"%s\" (%s): field \"%s\" does not descend from any root field, seems to be orphan or in parentage loop", utype.name(), utype.id().print(), fld->name());
                *broken_ifld_dic.set(&ifld, sizeof(ifld)) = 0;
            }
        }

        // step 4 : recursively exclude broken fields from name index
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

void sUsrType2::inheritField(const sUsrTypeField * inh_fld, sDic<idx> & overridden, sStr & case_buf)
{
    if( !inh_fld ) {
        return;
    }
    // do not copy array rows, system fields etc.
    if( !inh_fld->isArrayRow() && inh_fld->name()[0] != '_' ) {
        const char * inh_fld_name = canonicalCase(case_buf, inh_fld->name());
        if( const idx * pifld = _name2ifield.get(inh_fld_name) ) {
            // we already have a field with this name
            if( *pifld < _dim_explicit_fields ) {
                // we overrode it with an own field
                if( overridden.get(inh_fld_name) ) {
                    // ... already for a field from an earlier parent type - therefore we are stuck
                    LOG_ERROR("Type \"%s\" (%s) overrides field \"%s\" inherited from multiple parents; keeping only the first", name(), id().print(), inh_fld->name());
                    return;
                }
                LOG_TRACE("Type \"%s\" (%s) overrides inherited field \"%s\"", name(), id().print(), inh_fld->name());
                overridden.setString(inh_fld_name);
                const sUsrTypeField * fld = _fields.ptr(*pifld);
                if( fld->type() != sUsrTypeField::eArray && fld->type() != sUsrTypeField::eList ) {
                    // ... and our own overriding field is a scalar. We cannot inherit any of inh_fld's children - therefore stop.
                    return;
                }
            } else {
                // we already inherited a field of this name from an earlier parent type - therefore we are stuck.
                LOG_ERROR("Type \"%s\" (%s) inherits field \"%s\" from multiple parents; keeping only the first", name(), id().print(), inh_fld->name());
                return;
            }
        } else {
            // we don't have a field of this name, so copy it
            idx idst = _fields.dim();
            sUsrTypeField * dst_fld = _fields.addM(1);
            new(dst_fld) sUsrTypeField;
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

idx sUsrType2::ensureArrayFieldRow(sUsrTypeField * fld, sStr & name_buf, sStr & case_buf)
{
    if( fld->type() != sUsrTypeField::eArray ) {
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
    sUsrTypeField * fld_row = _fields.addM(1);
    new(fld_row) sUsrTypeField;
    fld = _fields.ptr(ifld); // in case realloc

    fld_row->_index = irow;
    fld_row->_is_array_row = true;
    fld_row->_owner_itype = fld->_owner_itype;
    fld_row->_definer_itype = fld->_definer_itype;
    fld_row->_included_from_itype = fld->_included_from_itype;

    fld_row->_pos_name = sUsrTypeField::setString(name_buf.ptr());
    if( sIs(fld->name(), fld->originalName()) ) {
        fld_row->_pos_orig_name = fld_row->_pos_name;
    } else {
        name_buf.add0();
        idx l = name_buf.length();
        name_buf.addString("_row_");
        name_buf.addString(fld->originalName());
        fld_row->_pos_orig_name = sUsrTypeField::setString(name_buf.ptr(l));
    }

    name_buf.add0();
    idx l = name_buf.length();
    name_buf.addString("Array row for ");
    name_buf.addString(fld->title());
    fld_row->_pos_title = sUsrTypeField::setString(name_buf.ptr(l));

    fld_row->_type = sUsrTypeField::eList;
    fld_row->_pos_parent_name = fld->_pos_name;
    fld_row->_parent = ifld;
    fld_row->_readonly = sUsrTypeField::eReadOnly;
    fld_row->_is_hidden = false;
    fld_row->_is_virtual = true;
    fld_row->_is_multi = true;

    return irow;
}

void sUsrType2::setFieldChildren(idx ifld, idx iroot_fld, SetFieldChildrenParam * param, sStr & case_buf)
{
    param->done_dic.set(&ifld, sizeof(idx));
    LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%"DEC") descends from root field \"%s\" (%"DEC")", name(), id().print(), _fields[ifld].name(), ifld, _fields[iroot_fld].name(), iroot_fld);
    sUsrTypeField * fld = _fields.ptr(ifld);
    if( const sUsrTypeField * par = fld->parent() ) {
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
            LOG_TRACE("Type \"%s\" (%s): field \"%s\" (%"DEC"): packing child field \"%s\" (%"DEC")", name(), id().print(), fld->name(), ifld, _fields.ptr(ichild)->name(), ichild);
            _child_ifields[fld->_start_children + i] = ichild;
            setFieldChildren(ichild, iroot_fld, param, case_buf);
        }
        _child_ifields[fld->_start_children + fld->_dim_children] = -sIdxMax; // guard value
        sSort::sortSimpleCallback<idx>(fldCompare, _fields.ptr(), fld->_dim_children, _child_ifields.ptr(fld->_start_children));
    }
}

void sUsrType2::recurseField(idx ifld, sDic<idx> & seen_dic)
{
    idx * pseen = seen_dic.get(&ifld, sizeof(ifld));
    if( pseen && *pseen ) {
        return;
    }
    *seen_dic.set(&ifld, sizeof(ifld)) = 1;
    const sUsrTypeField * fld = _fields.ptr(ifld);
    for(idx i=0; i<fld->dimChildren(); i++) {
        recurseField(fld->getChild(i)->_index, seen_dic);
    }
}

void sUsrType2::recurseDescendents(sDic<idx> & seen_dic) const
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

bool sUsrType2::PerUserRes::ensureUserForWriting(udx user_id)
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        // user has bee written for ...
        if( *pilevel != _name2ires.dimStack() - 1 ) {
            // ... but can be written no longer - it's not the top level in the tic
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

idx sUsrType2::PerUserRes::dimForUser(udx user_id) const
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        return _name2ires.dimLevel(*pilevel);
    }
    return 0;
}

idx sUsrType2::PerUserRes::getILevel(udx user_id) const
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        return *pilevel;
    }
    return -sIdxMax;
}

idx sUsrType2::PerUserRes::getIRes(udx user_id, const char * name, idx name_len) const
{
    if( const idx * pilevel = _usr2ticlevel.get(&user_id, sizeof(user_id)) ) {
        if( const idx * pires = _name2ires.get(*pilevel, name, name_len) ) {
            return *pires;
        }
    }
    return -sIdxMax;
}

idx sUsrType2::PerUserRes::getIRes(udx user_id, idx index) const
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
    idx ires; // negative for error
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
    sUsrAction * act;
    sUsrView * view;

    sStr buf;
    sDic<LoadActionViewFinderVal> typeactview2val;
};

//static
void sUsrType2::loadActionsViewsFinder(const sUsrType2 * utype_, const sUsrType2 * recurse_start, idx depth_from_start, void * param_)
{
    sUsrType2 * utype = const_cast<sUsrType2*>(utype_);
    LoadActionViewFinderParam * param = static_cast<LoadActionViewFinderParam*>(param_);

    const char * name = param->act ? param->act->name() : param->view->name();
    idx ires = param->act ? param->act->_iaction : param->view->_iview;

    param->buf.cut0cut();
    utype->id().print(param->buf);
    param->buf.add0();
    param->buf.addNum(param->user_id);
    param->buf.add0();
    param->buf.addString(name);
    param->buf.add0(2);

    LoadActionViewFinderVal * val = param->typeactview2val.set(param->buf.ptr(), param->buf.length());

    PerUserRes & per_user_res = param->act ? utype->_actions : utype->_views;
    per_user_res.ensureUserForWriting(param->user_id);


    if( val->ires >= 0 ) {
        // utype has an action/view of this name - overload if the existing action/view is less specific
        if( val->utype_depth > depth_from_start ) {
            val->ires = ires;
            *per_user_res._name2ires.setString(name) = ires;
            val->utype_depth = depth_from_start;
        }
    } else {
        // utype doesn't have an action/view of this name yet
        val->ires = ires;
        *per_user_res._name2ires.setString(name) = ires;
        val->utype_depth = depth_from_start;
    }
}

//static
void sUsrType2::loadActions(const sUsr & user)
{
    udx user_id = user.Id();
    if( sUsrAction::_usr2actres.get(&user_id, sizeof(user_id)) ) {
        return;
    }

    // load all actions for specified user and tie to type
    sUsrAction::ActRes * act_res = sUsrAction::_usr2actres.set(&user_id, sizeof(user_id));
    user.objs2("^action$", act_res->res);
    act_res->acts.init(sMex::fExactSize);
    act_res->acts.resize(act_res->res.dim());
    LoadActionViewFinderParam param;
    param.user_id = user_id;
    param.view = 0;
    idx iact = 0;
    for(sUsrObjRes::IdIter it = act_res->res.first(); act_res->res.has(it); act_res->res.next(it), iact++) {
        sUsrAction * act = act_res->acts.ptr(iact);
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

//static
void sUsrType2::loadViews(const sUsr & user)
{
    udx user_id = user.Id();
    if( sUsrView::_usr2viewres.get(&user_id, sizeof(user_id)) ) {
        return;
    }

    // load all views for specified user and tie to type
    const sUsrType2 * view_type = ensure(user, "view");
    sUsrView::ViewRes * view_res = sUsrView::_usr2viewres.set(&user_id, sizeof(user_id));
    user.objs2("^view$", view_res->res);
    view_res->views.init(sMex::fExactSize);
    view_res->views.resize(view_res->res.dim());
    LoadActionViewFinderParam param;
    param.user_id = user_id;
    param.act = 0;
    sVarSet tree_table;
    idx iview = 0;
    for(sUsrObjRes::IdIter it = view_res->res.first(); view_res->res.has(it); view_res->res.next(it), iview++) {
        sUsrView * view = view_res->views.ptr(iview);
        view->_id = *view_res->res.id(it);
        view->_user_id = user_id;
        view->_iview = iview;
        view->_fields.init(&sUsrView::_fields_buf);

        const sUsrObjRes::TObjProp * props = view_res->res.get(it);
        const sUsrObjRes::TPropTbl * type_name_tbl = view_res->res.get(*props, "type_name");
        const char * type_name = view_res->res.getValue(type_name_tbl);
        param.view = view;
        findRaw(&user, 0, type_name, loadActionsViewsFinder, &param, true, true);

        // the following is inefficient, but what else can we do?
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
            sUsrView::Field * fld = view->_fields.add(1);
            if( const char * field_name = fields_row->findValue("field_name") ) {
                *sUsrView::_names.setString(field_name, 0, &fld->pos_name) = true;
            } else {
                fld->pos_name = -sIdxMax;
            }
            if( const char * field_default = fields_row->findValue("field_default") ) {
                *sUsrView::_names.setString(field_default, 0, &fld->pos_default_value) = true;
            } else {
                fld->pos_default_value = -sIdxMax;
            }
            fld->order = fields_row->findIValue("field_order");
            fld->readonly = fields_row->findBoolValue("field_readonly");
        }
    }
}

/// External API : sUsrAction ///

const char * sUsrAction::name() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "name");
    return res.getValue(tbl);
}

const char * sUsrAction::title() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "title");
    return res.getValue(tbl);
}

real sUsrAction::order() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "order");
    const char * sord = res.getValue(tbl);
    return sord ? strtod(sord, 0) : 0;
}

bool sUsrAction::isObjAction() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "is_obj_action");
    return sString::parseBool(res.getValue(tbl));
}

udx sUsrAction::requiredPermission() const
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
    const sUsrObjRes & lhs_res = objRes();
    const sUsrObjRes::TObjProp & lhs_props = *lhs_res.get(_id);
    const sUsrObjRes::TPropTbl * lhs_order_tbl = lhs_res.get(lhs_props, "order");
    const char * lhs_sord = lhs_res.getValue(lhs_order_tbl);
    if( !lhs_sord) {
        lhs_sord = sStr::zero;
    }
    const sUsrObjRes & rhs_res = rhs->objRes();
    const sUsrObjRes::TObjProp & rhs_props = *rhs_res.get(rhs->_id);
    const sUsrObjRes::TPropTbl * rhs_order_tbl = rhs_res.get(rhs_props, "order");
    const char * rhs_sord = rhs_res.getValue(rhs_order_tbl);
    if( !rhs_sord) {
        rhs_sord = sStr::zero;
    }

    if( strcmp(lhs_sord, rhs_sord) != 0 ) {
        return strtod(lhs_sord, 0) < strtod(rhs_sord, 0) ? -1 : 1;
    }

    const sUsrObjRes::TPropTbl * lhs_name_tbl = lhs_res.get(lhs_props, "name");
    const char * lhs_name = lhs_res.getValue(lhs_name_tbl);
    if( !lhs_name) {
        lhs_name = sStr::zero;
    }
    const sUsrObjRes::TPropTbl * rhs_name_tbl = rhs_res.get(rhs_props, "name");
    const char * rhs_name = rhs_res.getValue(rhs_name_tbl);
    if( !rhs_name) {
        rhs_name = sStr::zero;
    }
    return strcasecmp(lhs_name, rhs_name);
}

/// External API : sUsrView ///

const char * sUsrView::name() const
{
    const sUsrObjRes & res = objRes();
    const sUsrObjRes::TObjProp & props = *res.get(_id);
    const sUsrObjRes::TPropTbl * tbl = res.get(props, "name");
    return res.getValue(tbl);
}

idx sUsrView::dimFields() const
{
    return _fields.dim();
}

const char * sUsrView::fieldName(idx ifield) const
{
    idx pos = _fields[ifield].pos_name;
    return pos >= 0 ? static_cast<const char *>(_names.id(pos)) : sStr::zero;
}

const char * sUsrView::fieldDefaultValue(idx ifield) const
{
    idx pos = _fields[ifield].pos_default_value;
    return pos >= 0 ? static_cast<const char *>(_names.id(pos)) : sStr::zero;
}

idx sUsrView::fieldOrder(idx ifield) const
{
    return _fields[ifield].order;
}

bool sUsrView::fieldReadonly(idx ifield) const
{
    return _fields[ifield].readonly;
}

int sUsrView::cmp(const sUsrView * rhs) const
{
    if( !rhs ) {
        return 1;
    }
    const char * lhs_name = name();
    const char * rhs_name = rhs->name();
    return strcasecmp(lhs_name, rhs_name);
}

/// External API : sUsrType2 ///

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
                // override recurse_from if not recursing, of if recursing from a more specific type (higher in the tree)
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
                if( const sUsrType2 * utype = getRaw("base_user_type") ) {
                    sHiveId base_id = utype->id();
                    lister(utype, &base_id, 0, false);
                }
            }
        }

        virtual void checkQuery(const char * exact_name, regex_t * re, bool is_recurse, bool is_negate)
        {
            if( exact_name ) {
                if( const sUsrType2 * utype = getRaw(exact_name) ) {
                    sHiveId utype_id = utype->id();
                    lister(utype, is_recurse ? &utype_id : 0, 0, is_negate);
                }
            } else if( re ) {
                // only look for types that have names
                for(idx itype = 0; itype < sUsrType2::_types.dim(); itype++) {
                    const char * name = sUsrType2::_types[itype]->name();
                    if( name && regexec(re, name, 0, 0, 0) == 0 ) {
                        const sUsrType2 * utype = sUsrType2::_types[itype];
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
                        // common case optimization
                        checkQuery(type_name_buf.ptr(), 0, is_recurse, is_negate);
                    } else if( regcomp(&re, type_name_buf.ptr(), REG_EXTENDED | REG_NOSUB | REG_ICASE) == 0 ) {
                        // only look for types that have names
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

//static
idx sUsrType2::find(const sUsr & user, sVec<const sUsrType2 *> * out, const char * qry, FindCallback cb/* = 0*/, void * cb_param/* = 0*/, bool manual_ensure/* = false */, bool lazy_fetch_fields/* = false*/)
{
    if( !manual_ensure ) {
        // unless the caller has somehow already ensured that all necessary types are loaded ...
        idx qry_len = sLen(qry);
        if( is_potential_type_name(qry, qry_len) ) {
            // if the query looks like the exact name of a type, ensure the type is loaded (if it exists)
            ensure(user, qry, qry_len);
        } else if( qry && qry[0] == '^' && qry[qry_len - 1] == '$' && is_potential_type_name(qry + 1, qry_len - 2) ) {
            // if the query looks like a regexp for exactly matching a name of a type, ensure that type is loaded (if it exists)
            ensure(user, qry + 1, qry_len - 2);
        } else {
            // if the query is not the exact name of a type, load all types (including non-prefetched ones)
            load(user, "*", 1, 0);
        }
    }
    return findRaw(manual_ensure ? 0 : &user, out, qry, cb, cb_param, false, lazy_fetch_fields);
}

//static
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
            const sUsrType2 * utype = ensurer_usr ? ensure(*ensurer_usr, e->utype_id, false, lazy_fetch_fields) : getRaw(e->utype_id);
            if( out ) {
                *out->add(1) = utype;
            }
            if( cb ) {
                cb(utype, ensurer_usr ? ensure(*ensurer_usr, e->recurse_start_id, false, lazy_fetch_fields) : getRaw(e->recurse_start_id), e->recurse_depth, cb_param);
            }
        }
    }

    return num_added;
}

//static
idx sUsrType2::getRootTypes(const sUsr & user, sVec<const sUsrType2 *> & out)
{
    // root types might be non-prefetched - so ensure all types are loaded
    load(user, "*", 1, 0);

    idx start_pos = out.dim();
    out.add(_roots.dim());
    for(idx i=0; i<_roots.dim(); i++) {
        out[start_pos + i] = _roots[i];
    }
    return _roots.dim();
}

bool sUsrType2::isUser() const
{
    if( _is_user == eLazyNotLoaded ) {
        if( strcmp(name(), "base_user_type" ) == 0 ) {
            _is_user = eLazyTrue;
        } else {
            for(idx ip = 0; ip < dimParents(); ip++) {
                if( getParent(ip)->isUser() ) {
                    _is_user = eLazyTrue;
                }
            }
            _is_user = eLazyFalse;
        }
    }

    return (_is_user == eLazyTrue);
}

bool sUsrType2::isSystem() const
{
    if( _is_system == eLazyNotLoaded ) {
        if( strcmp(name(), "base_system_type" ) == 0 ) {
            _is_system = eLazyTrue;
        } else {
            for(idx ip = 0; ip < dimParents(); ip++) {
                if( getParent(ip)->isSystem() ) {
                    _is_system = eLazyTrue;
                }
            }
            _is_system = eLazyFalse;
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

        // return true if matched (positively or negatively)
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

idx sUsrType2::getParents(sVec<const sUsrType2*> & out) const
{
    idx cnt = dimParents();
    idx start_dim = out.dim();
    out.resize(start_dim + cnt);
    for(idx i=0; i<cnt; i++) {
        out[start_dim + i] = _types[_parents[i]];
    }
    return cnt;
}

idx sUsrType2::getChildren(sVec<const sUsrType2*> & out) const
{
    idx cnt = dimChildren();
    idx start_dim = out.dim();
    out.resize(start_dim + cnt);
    for(idx i=0; i<cnt; i++) {
        out[start_dim + i] = _types[_children[i]];
    }
    return cnt;
}

idx sUsrType2::getIncludes(sVec<const sUsrType2*> & out) const
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

const sUsrTypeField* sUsrType2::getField(const sUsr & user, const char * field_name, idx field_name_len/* = 0 */) const
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
    if( !_is_fetched ) {
        load(user, 0, 0, &_id);
    }

    idx start_dim = out.dim();
    idx cnt = dimFields(user);
    out.resize(start_dim + cnt);
    for(idx i=0; i<cnt; i++) {
        out[start_dim + i] = _fields.ptr(i);
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

idx sUsrType2::findFields(const sUsr & user, sVec<const sUsrTypeField*> & out, const char * filter00/* = 0*/) const
{
    if( !_is_fetched ) {
        load(user, 0, 0, &_id);
    }

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

idx sUsrType2::dimActions(const sUsr & user) const
{
    loadActions(user);
    udx user_id = user.Id();
    return _actions.dimForUser(user_id);
}

const sUsrAction * sUsrType2::getAction(const sUsr & user, const char * act_name, idx act_name_len/* = 0 */) const
{
    loadActions(user);
    udx user_id = user.Id();
    idx ires = _actions.getIRes(user_id, act_name, act_name_len);
    if( ires >= 0 ) {
        return sUsrAction::_usr2actres.get(&user_id, sizeof(user_id))->acts.ptr(ires);
    }
    return 0;
}

const sUsrAction * sUsrType2::getAction(const sUsr & user, idx iact) const
{
    loadActions(user);
    udx user_id = user.Id();
    idx ires = _actions.getIRes(user_id, iact);
    if( ires >= 0 ) {
        return sUsrAction::_usr2actres.get(&user_id, sizeof(user_id))->acts.ptr(ires);
    }
    return 0;
}

idx sUsrType2::getActions(const sUsr & user, sVec<const sUsrAction *> & out) const
{
    loadActions(user);
    udx user_id = user.Id();
    idx dim_act = _actions.dimForUser(user_id);

    if( dim_act ) {
        idx start_dim = out.dim();
        idx ilevel = _actions.getILevel(user_id);
        const sUsrAction::ActRes * act_res = sUsrAction::_usr2actres.get(&user_id, sizeof(user_id));

        out.add(dim_act);
        for(idx iact = 0; iact < dim_act; iact++) {
            idx ires = *_actions._name2ires.ptr(ilevel, iact);
            out[start_dim + iact] = act_res->acts.ptr(ires);
        }
    }
    return dim_act;
}

idx sUsrType2::dimViews(const sUsr & user) const
{
    loadViews(user);
    udx user_id = user.Id();
    return _views.dimForUser(user_id);
}

const sUsrView * sUsrType2::getView(const sUsr & user, const char * view_name, idx view_name_len/* = 0 */) const
{
    loadViews(user);
    udx user_id = user.Id();
    idx ires = _views.getIRes(user_id, view_name, view_name_len);
    if( ires >= 0 ) {
        return sUsrView::_usr2viewres.get(&user_id, sizeof(user_id))->views.ptr(ires);
    }
    return 0;
}

const sUsrView * sUsrType2::getView(const sUsr & user, idx iview) const
{
    loadViews(user);
    udx user_id = user.Id();
    idx ires = _views.getIRes(user_id, iview);
    if( ires >= 0 ) {
        return sUsrView::_usr2viewres.get(&user_id, sizeof(user_id))->views.ptr(ires);
    }
    return 0;
}

idx sUsrType2::getViews(const sUsr & user, sVec<const sUsrView *> & out) const
{
    loadViews(user);
    udx user_id = user.Id();
    idx dim_views = _views.dimForUser(user_id);

    if( dim_views ) {
        idx start_dim = out.dim();
        idx ilevel = _views.getILevel(user_id);
        const sUsrView::ViewRes * view_res = sUsrView::_usr2viewres.get(&user_id, sizeof(user_id));

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
        // add header if output table is empty
        idx icol = 0;
        list.setColId(icol++, "type_id"); // which maps to the type's *name* for legacy compatibility
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
        list.addRow().addCol(name()) // type's name
            .addCol(fld->name())
            .addCol(fld->title())
            .addCol(fld->typeName());

        if( const sUsrTypeField * par = fld->parent() ) {
            if( par->isArrayRow() ) {
                // for parent, use array instead of array row pseudo-field
                par = par->parent();
            }
            list.addCol(par ? par->name() : "");
        } else {
            list.addCol("");
        }

        list.addCol(fld->role())
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
    printer.addValue(_id);
    printer.addKey("name");
    printer.addValue(name(), 0, true);
    printer.addKey("title");
    printer.addValue(title(), 0, true);
    if( isUser() ) {
        printer.addKey("is_user");
        printer.addValue(true);
    }
    if( isSystem() ) {
        printer.addKey("is_system");
        printer.addValue(true);
    }
    if( description() && *description() ) {
        printer.addKey("description");
        printer.addValue(description(), 0, true);
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
        printer.startObject();
    }
}

/// External API : sUsrTypeField ///

sUsrTypeField::sUsrTypeField(bool default_zero)
{
    sSet(this, 0);
    _index = _pos_name = _pos_title = _pos_orig_name = _pos_parent_name = _pos_role = _pos_brief = _pos_order = _pos_default_value = _pos_constraint = _pos_constraint_data = _pos_constraint_description = _pos_description = _pos_link_url = -1;
    _definer_itype = _owner_itype = _included_from_itype = -1;
    _type = eInvalid;
    _parent = -1;
    _start_children = -1;
    _is_flattened_decor = eLazyNotLoaded;
    if( !default_zero ) {
        // if not loading from a source where absence of data about the field means "0" or "false" or ""
        _readonly = eReadOnly;
        _is_optional = true;
        _is_hidden = true;
        _is_virtual = true;
    }
}

const sUsrTypeField & sUsrTypeField::operator=(const sUsrTypeField & rhs)
{
    memcpy(this, &rhs, sizeof(sUsrTypeField));
    return *this;
}

int sUsrTypeField::cmp(const sUsrTypeField * rhs) const
{
    if( !rhs ) {
        return 1;
    }
    if( _owner_itype != rhs->_owner_itype ) {
        return ownerType()->id() < rhs->ownerType()->id() ? -1 : 1;
    }
    const sUsrTypeField * lhs = this;
    idx lhs_orig_depth = lhs->_ancestor_count;
    idx rhs_orig_depth = rhs->_ancestor_count;
    // first, move up to equal depth in ancestor tree of both fields
    while(lhs->_ancestor_count > rhs->_ancestor_count) {
        lhs = lhs->parent();
    }
    while(rhs->_ancestor_count > lhs->_ancestor_count) {
        rhs = rhs->parent();
    }
    if( lhs == rhs ) {
        return lhs_orig_depth - rhs_orig_depth;
    }
    // next, move up to level immediately below common ancestor, avoiding infinite loop even if field tree is messed up or circular
    for(idx i = 0; lhs->parent() != rhs->parent() && i < lhs->_ancestor_count; i++) {
        lhs = lhs->parent();
        rhs = rhs->parent();
    }
    const char * lhs_sorder = lhs->getString(lhs->_pos_order);
    const char * rhs_sorder = rhs->getString(rhs->_pos_order);
    if( strcmp(lhs_sorder, rhs_sorder) != 0 ) {
        return strtod(lhs_sorder, 0) < strtod(rhs_sorder, 0) ? -1 : 1;
    }
    return strcasecmp(lhs->name(), rhs->name());
}

const char * sUsrTypeField::typeName() const
{
    return fieldTypeToName(_type);
}

const sUsrTypeField* sUsrTypeField::parent() const
{
    return _parent >= 0 ? sUsrType2::_types[_owner_itype]->_fields.ptr(_parent) : 0;
}

const sUsrTypeField* sUsrTypeField::getChild(idx ichild) const
{
    const sUsrType2 & utype = *sUsrType2::_types[_owner_itype];
    return ichild >= 0 && ichild < _dim_children ? utype._fields.ptr(utype._child_ifields[_start_children + ichild]) : 0;
}

idx sUsrTypeField::getChildren(sVec<const sUsrTypeField*> &out) const
{
    idx start_dim = out.dim();
    out.resize(start_dim + _dim_children);
    const sUsrType2 & utype = *sUsrType2::_types[_owner_itype];
    for(idx i=0; i<_dim_children; i++) {
        out[start_dim + i] = utype._fields.ptr(utype._child_ifields[_start_children + i]);
    }
    return _dim_children;
}

bool sUsrTypeField::canSetValue() const
{
    const char * nm = name();
    // For now, assume virtual fields cannot be set (reasonable for current set of
    // fields that are marked virtual; maybe logic will change in the future).
    return canHaveValue() && !isVirtual() && nm[0] != '_' && strcmp(nm, "created") != 0 && strcmp(nm, "modified") != 0;
}

bool sUsrTypeField::isFlattenedDecor() const
{
    if( _is_flattened_decor == eLazyNotLoaded ) {
        // We consider arrays non-decorative if
        // a) the aray has multivalued cells (meaning multiple rows); or
        // b) the array itself is multivalued
        // We consider array rows non-decorative if there are multivalued cells (meaning multiple rows).
        // We consider non-array-row lists non-decorative only if:
        // 1) the list has >= 2 non-decorative children; and
        // 2a) one of those children is multivalued, or
        // 2b) the list itself is multivalued but is not the child of an array row.
        //
        // Note that if array row is non-decorative, the array must be non-decorative;
        // sUsrPropSet::readFieldNode() relies on this behavior.
        if( type() == eArray ) {
            if( isMulti() ) {
                _is_flattened_decor = eLazyFalse;
            } else {
                const sUsrTypeField * array_row = dimChildren() ? getChild(0) : 0;
                _is_flattened_decor = array_row && !array_row->isFlattenedDecor() ? eLazyFalse : eLazyTrue;
            }
        } else if( type() == eList ) {
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

bool sUsrTypeField::isFlattenedMulti() const
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

const sUsrType2 * sUsrTypeField::ownerType() const
{
    return sUsrType2::_types[_owner_itype];
}

const sUsrType2 * sUsrTypeField::definerType() const
{
    if( _definer_itype >= 0 ) {
        const sUsrType2 * definer = sUsrType2::_types[_definer_itype];
        return definer->_id.objId() ? definer : 0;
    } else {
        return 0;
    }
}

const sUsrType2 * sUsrTypeField::includedFromType() const
{
    if( _included_from_itype >= 0 ) {
        const sUsrType2 * included_from = sUsrType2::_types[_included_from_itype];
        return included_from->_id.objId() ? included_from : 0;
    } else {
        return 0;
    }
}


//static
bool sUsrTypeField::parseValue(sVariant &var, sUsrTypeField::EType type, const char * value, idx flags/*=0*/)
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
    case eReal:
        var.parseReal(value);
        break;
    case eBool:
        var.parseBool(value);
        break;
    case eList:
    case eArray:
        var.parseIntList(value); // maybe parseObjectIdList() ?
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
        } else {
            printer.addValue(0);
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

        if( _definer_itype > 0 && _definer_itype != _owner_itype ) {
            printer.addKey("_definer_type");
            printer.addValue(definerType()->name());
        }

        if( _included_from_itype >= 0 && _included_from_itype != _owner_itype ) {
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

//static
idx sUsrTypeField::setString(const char *s, bool canonicalize, bool allow_empty)
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

//static
void sUsrTypeField::replaceString(idx & pos, const char * find00, const char * replace00, sStr & buf)
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

