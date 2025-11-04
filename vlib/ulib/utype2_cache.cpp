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

#include "utype2_cache.hpp"

#include <assert.h>
#include <ctype.h>

#include <slib/std/file.hpp>

using namespace slib;

sUsrTypeCache::sUsrTypeCache()
{
}

const char * sUsrTypeCache::getString(idx pos) const
{
    if( pos < 0 ) {
        return 0;
    }
    if( pos == 0 ) {
        return sStr::zero;
    }
    if( header() && pos < header()->size_strings ) {
        return static_cast<const char *>(_mex.ptr(header()->pos_strings + pos));
    }
    return 0;
}

idx sUsrTypeCache::dimFields() const
{
    return header() ? header()->dim_field_packs : 0;
}

const sUsrTypeCache::CachedField * sUsrTypeCache::getChildField(idx ichild) const
{
    if( header() && ichild >= 0 && ichild < header()->dim_child_ifields ) {
        idx ifield = static_cast<const idx *>(_mex.ptr(header()->pos_child_ifields))[ichild];
        return getField(ifield);
    }
    return 0;
}

namespace {
    struct CanonicalDictKey {
        char _static_buf[128];
        sStr _str_buf;
        const char * key;
        idx key_len;

        const char * reset(const sHiveId & id, char code, const char * name, idx len)
        {
            if( !len ) {
                len = sLen(name);
            }
            key_len = sizeof(sHiveId) + 3 + len + 1;
            char * buf = key_len < 128 ? _static_buf : _str_buf.add(0, key_len);
            key = buf;
            memcpy(buf, &id, sizeof(sHiveId));
            buf += sizeof(sHiveId);
            *buf = 0;
            buf++;
            *buf = code;
            buf++;
            *buf = 0;
            buf++;
            for(idx i = 0; i < len; i++) {
                buf[i] = tolower(name[i]);
            }
            buf[len] = 0;

            return key;
        }

        const char * reset(const char * name, idx len)
        {
            if( !len ) {
                len = sLen(name);
            }
            key_len = len + 1;
            char * buf = key_len < 128 ? _static_buf : _str_buf.add(0, key_len);
            key = buf;
            for(idx i = 0; i < len; i++) {
                buf[i] = tolower(name[i]);
            }
            buf[len] = 0;

            return key;
        }

        CanonicalDictKey(const sHiveId & id, char code, const char * name, idx len, idx flags = sMex::fExactSize) : _str_buf(flags)
        {
            reset(id, code, name, len);
        }

        CanonicalDictKey(const char * name, idx len, idx flags = sMex::fExactSize) : _str_buf(flags)
        {
            reset(name, len);
        }
    };
};

const sUsrTypeCache::CachedField * sUsrTypeCache::getTypeField(const sHiveId & type_id, const char * name, idx len) const
{
    CanonicalDictKey canon(type_id, 'f', name, len);
    if( const idx * pifield = getDict(canon.key, canon.key_len) ) {
        return getField(*pifield);
    }
    return 0;
}

const sUsrTypeCache::CachedField * sUsrTypeCache::getTypeRootField(idx iroot) const
{
    if( header() && iroot >= 0 && iroot < header()->dim_type_root_ifields ) {
        idx ifield = static_cast<const idx *>(_mex.ptr(header()->pos_type_root_ifields))[iroot];
        return getField(ifield);
    }
    return 0;
}

const sUsrTypeCache::FieldPack * sUsrTypeCache::getFieldPack(idx ifield) const
{
    if( header() && ifield >= 0 && ifield < header()->dim_field_packs ) {
        return &static_cast<const FieldPack*>(_mex.ptr(header()->pos_field_packs))[ifield];
    }
    return 0;
}

idx sUsrTypeCache::dimActions() const
{
    return header() ? header()->dim_action_packs : 0;
}

const sUsrTypeCache::CachedAction * sUsrTypeCache::getTypeAction(const sHiveId & type_id, const char * name, idx len) const
{
    CanonicalDictKey canon(type_id, 'a', name, len);
    if( const idx * piaction = getDict(canon.key, canon.key_len) ) {
        return getAction(*piaction);
    }
    return 0;
}

const sUsrTypeCache::CachedAction * sUsrTypeCache::getTypeAllAction(idx type_all_iaction) const
{
    if( header() && type_all_iaction >= 0 && type_all_iaction < header()->dim_type_all_iactions ) {
        idx iaction = static_cast<const idx *>(_mex.ptr(header()->pos_type_all_iactions))[type_all_iaction];
        return getAction(iaction);
    }
    return 0;
}

idx sUsrTypeCache::dimJSComponents() const
{
    return header() ? header()->dim_jsco_packs : 0;
}

const sUsrTypeCache::CachedJSComponent * sUsrTypeCache::getTypeJSComponent(const sHiveId & type_id, const char * name, idx len) const
{
    CanonicalDictKey canon(type_id, 'j', name, len);
    if( const idx * pi = getDict(canon.key, canon.key_len) ) {
        return getJSComponent(*pi);
    }
    return 0;
}

const sUsrTypeCache::CachedJSComponent * sUsrTypeCache::getTypeAllJSComponent(idx type_all_i) const
{
    if( header() && type_all_i >= 0 && type_all_i < header()->dim_type_all_ijscos ) {
        idx i = static_cast<const idx *>(_mex.ptr(header()->pos_type_all_ijscos))[type_all_i];
        return getJSComponent(i);
    }
    return 0;
}

idx sUsrTypeCache::dimViews() const
{
    return header() ? header()->dim_view_packs : 0;
}

const sUsrTypeCache::CachedView * sUsrTypeCache::getTypeView(const sHiveId & type_id, const char * name, idx len) const
{
    CanonicalDictKey canon(type_id, 'v', name, len);
    if( const idx * piview = getDict(canon.key, canon.key_len) ) {
        return getView(*piview);
    }
    return 0;
}

const sUsrTypeCache::CachedView * sUsrTypeCache::getTypeAllView(idx type_all_iview) const
{
    if( header() && type_all_iview >= 0 && type_all_iview < header()->dim_type_all_iviews ) {
        idx iview = static_cast<const idx *>(_mex.ptr(header()->pos_type_all_iviews))[type_all_iview];
        return getView(iview);
    }
    return 0;
}

const sUsrTypeCache::ActionPack * sUsrTypeCache::getActionPack(idx iaction) const
{
    if( header() && iaction >= 0 && iaction < header()->dim_action_packs ) {
        return &static_cast<const ActionPack*>(_mex.ptr(header()->pos_action_packs))[iaction];
    }
    return 0;
}

const sUsrTypeCache::JSComponentPack * sUsrTypeCache::getJSComponentPack(idx i) const
{
    if( header() && i >= 0 && i < header()->dim_jsco_packs ) {
        return &static_cast<const JSComponentPack*>(_mex.ptr(header()->pos_jsco_packs))[i];
    }
    return 0;
}

const sUsrTypeCache::ViewPack * sUsrTypeCache::getViewPack(idx iview) const
{
    if( header() && iview >= 0 && iview < header()->dim_view_packs ) {
        return &static_cast<const ViewPack*>(_mex.ptr(header()->pos_view_packs))[iview];
    }
    return 0;
}

const sUsrTypeCache::FieldPack * sUsrTypeCache::getViewFieldPack(idx ifield) const
{
    if( header() && ifield >= 0 && ifield < header()->dim_view_field_packs ) {
        return &static_cast<const FieldPack*>(_mex.ptr(header()->pos_view_field_packs))[ifield];
    }
    return 0;
}

idx sUsrTypeCache::dimTypes() const
{
    return header() ? header()->dim_type_packs : 0;
}

const sUsrTypeCache::CachedType * sUsrTypeCache::getType(const char * name, idx len) const
{
    CanonicalDictKey canon(name, len);
    if( const idx * pitype = getDict(canon.key, canon.key_len) ) {
        return getType(*pitype);
    }
    return 0;
}

const sUsrTypeCache::CachedType * sUsrTypeCache::getType(const sHiveId & id) const
{
    if( const idx * pitype = getDict(&id, sizeof(sHiveId)) ) {
        return getType(*pitype);
    }
    return 0;
}

const sUsrTypeCache::CachedType * sUsrTypeCache::getDepType(idx idep) const
{
    if( header() && idep >= 0 && idep < header()->dim_dep_itypes ) {
        idx itype = static_cast<const idx *>(_mex.ptr(header()->pos_dep_itypes))[idep];
        return getType(itype);
    }
    return 0;
}

const sUsrTypeCache::TypePack * sUsrTypeCache::getTypePack(idx itype) const
{
    if( header() && itype >= 0 && itype < header()->dim_type_packs ) {
        return &static_cast<const TypePack*>(_mex.ptr(header()->pos_type_packs))[itype];
    }
    return 0;
}

idx sUsrTypeCache::dimRootTypes() const
{
    return header() ? header()->dim_root_itypes : 0;
}

const sUsrTypeCache::CachedType * sUsrTypeCache::getRootType(idx iroot) const
{
    if( header() && iroot >= 0 && iroot < header()->dim_root_itypes ) {
        idx itype = static_cast<const idx *>(_mex.ptr(header()->pos_root_itypes))[iroot];
        return getType(itype);
    }
    return 0;
}

#define HEADER_TITLE "HIVE Type Cache"
#if SLIB64
#define HEADER_PLATFORM SLIB_PLATFORM " SLIB64"
#else
#define HEADER_PLATFORM SLIB_PLATFORM " SLIB32"
#endif

void sUsrTypeCache::reset()
{
    _mex.destroy();
    _mex.flags = 0;
    _fields.cut(0);
    _actions.cut(0);
    _jscos.cut(0);
    _views.cut(0);
    _user2av_permissions_ensured.empty();
    _types.cut(0);
}

static bool outOfRange(idx min, idx max, idx pos, idx length)
{
    if( length < 0 ) {
        return true;
    } else if( length > 0 && (pos < min || pos > max || pos + length > max) ) {
        return true;
    }
    return false;
}

static volatile idx alignMex(sMex * mex, idx pos, idx align)
{
    idx len = sAlign(pos, align) - pos;
    mex->resize(pos + len);
    return pos + len;
}

namespace {
    class UniqueStrings {
        public:
            sDic<idx> str2pos;
            idx next_pos;

            UniqueStrings()
            {
                *str2pos.setString(_, 1) = 0;
                next_pos = 2;
            }

            idx set(const char * s)
            {
                idx ret = -1;
                if( s ) {
                    idx len = sLen(s);
                    if( const idx * ppos = str2pos.get(s, len) ) {
                        ret = *ppos;
                    } else {
                        ret = *str2pos.setString(s, len) = next_pos;
                        next_pos += len + 1;
                    }
                }
                return ret;
            }
    };
};

sRC sUsrTypeCache::save(const sUsr & user, const char * filename)
{
    udx user_id = user.Id();

    reset();


    sFilePath dirname, extension;
    dirname.makeName(filename, "%%dir");
    dirname.shrink00();
    if( !dirname.length() ) {
        dirname.addString(".");
    }
    extension.makeName(filename, "%%ext");
    extension.shrink00();

    sStr temp_filename;
    if( !sFile::mktemp(temp_filename, dirname, extension.length() ? extension.ptr() : 0) ) {
        return RC(sRC::eCreating, sRC::eFile, sRC::eOperation, sRC::eFailed);
    }

    _mex.init(temp_filename, sMex::fBlockDoubling | sMex::fSetZero);
    if( !_mex.ok() ) {
        reset();
        return RC(sRC::eMapping, sRC::eFile, sRC::eOperation, sRC::eFailed);
    }
    _mex.cut(0);
    _mex.add(0, sizeof(Header));
    new (_mex.ptr(0)) Header;
    strncpy(const_cast<char *>(header()->title), HEADER_TITLE, sizeof(header()->title) - 1);
    snprintf(const_cast<char *>(header()->version), sizeof(header()->version) - 1, "%d.%d", USR_TYPE_CACHE_MAJOR_VERSION, USR_TYPE_CACHE_MINOR_VERSION);
    strncpy(const_cast<char *>(header()->platform), HEADER_PLATFORM, sizeof(header()->platform) - 1);


    sVec<FieldPack> field_packs;
    sVec<idx> child_ifields;
    sVec<ActionPack> action_packs;
    sVec<JSComponentPack> jsco_packs;
    sVec<ViewPack> view_packs;
    sVec<FieldPack> view_field_packs;
    sVec<TypePack> type_packs;
    sVec<idx> root_itypes, dep_itypes, type_root_ifields, type_all_iactions, type_all_ijscos, type_all_iviews;
    sDic<idx> dict;
    UniqueStrings unique_strings;


    CanonicalDictKey canon(0, 0, sMex::fBlockDoubling);

    for(idx itype = 0; itype < sUsrLoadingType::_types.dim(); itype++) {
        const sUsrLoadingType * utype = sUsrLoadingType::_types[itype];
        TypePack & type_pack = *type_packs.add(1);
        if( !utype->id() ) {
            continue;
        }
        if( utype->_is_ephemeral ) {
            continue;
        }
        if( !utype->_is_fetched ) {
            sUsrLoadingType::loadFromObj(user, 0, 0, &utype->id());
        }

        *dict.set(&utype->id(), sizeof(sHiveId)) = itype;
        canon.reset(utype->name(), 0);
        *dict.setString(canon.key, canon.key_len) = itype;

        type_pack.id = utype->id();
        type_pack.itype = itype;
        type_pack.pos_name = unique_strings.set(utype->name());
        type_pack.pos_title = unique_strings.set(utype->title());
        type_pack.pos_description = unique_strings.set(utype->description());
        type_pack.created = utype->_created;
        type_pack.modified = utype->_modified;
        type_pack.is_virtual = utype->isVirtual();
        type_pack.is_user = utype->isUser();
        type_pack.is_system = utype->isSystem();
        type_pack.is_prefetch = utype->_is_prefetch;
        type_pack.is_singleton = utype->_is_singleton;
        type_pack.is_broken = utype->_is_broken;

        if( utype->dimParents() ) {
            type_pack.start_parents = dep_itypes.dim();
            for(idx idep = 0; idep < utype->dimParents(); idep++) {
                *dep_itypes.add(1) = utype->getParent(idep)->_itype;
            }
            type_pack.dim_parents = utype->dimParents();
            *dep_itypes.add(1) = -1;
        }
        if( utype->dimChildren() ) {
            type_pack.start_children = dep_itypes.dim();
            for(idx idep = 0; idep < utype->dimChildren(); idep++) {
                *dep_itypes.add(1) = utype->getChild(idep)->_itype;
            }
            type_pack.dim_children = utype->dimChildren();
            *dep_itypes.add(1) = -1;
        }
        if( utype->dimIncludes() ) {
            type_pack.start_includes = dep_itypes.dim();
            for(idx idep = 0; idep < utype->dimIncludes(); idep++) {
                *dep_itypes.add(1) = utype->getInclude(idep)->_itype;
            }
            type_pack.dim_includes = utype->dimIncludes();
            *dep_itypes.add(1) = -1;
        }

        if( utype->_fields.dim() ) {
            type_pack.start_fields = field_packs.dim();
            for(idx utype_ifield = 0; utype_ifield < utype->_fields.dim(); utype_ifield++) {
                idx ifield = field_packs.dim();
                const sUsrLoadingTypeField * field = utype->_fields.ptr(utype_ifield);
                FieldPack & field_pack = *field_packs.add(1);

                canon.reset(utype->id(), 'f', field->name(), 0);
                *dict.set(canon.key, canon.key_len) = ifield;

                field_pack.ifield = ifield;
                field_pack.pos_name = unique_strings.set(field->name());
                field_pack.pos_orig_name = unique_strings.set(field->originalName());
                field_pack.pos_title = unique_strings.set(field->title());
                field_pack.type = field->type();
                field_pack.pos_parent_name = unique_strings.set(sUsrLoadingTypeField::getString(field->_pos_parent_name));
                field_pack.role = field->role();
                field_pack.readonly = field->readonly();
                field_pack.is_key = field->isKey();
                field_pack.is_optional = field->isOptional();
                field_pack.is_multi = field->isMulti();
                field_pack.is_hidden = field->isHidden();
                field_pack.is_summary = field->isSummary();
                field_pack.is_virtual = field->isVirtual();
                field_pack.is_batch = field->isBatch();
                field_pack.is_sysinternal = field->isSysInternal();
                field_pack.pos_brief = unique_strings.set(field->brief());
                field_pack.pos_order = unique_strings.set(field->orderString());
                field_pack.pos_default_value = unique_strings.set(field->defaultValue());
                field_pack.pos_constraint = unique_strings.set(field->constraint());
                field_pack.pos_constraint_data = unique_strings.set(field->constraintData());
                field_pack.pos_constraint_description = unique_strings.set(field->constraintDescription());
                field_pack.pos_description = unique_strings.set(field->description());
                field_pack.pos_link_url = unique_strings.set(field->linkUrl());
                field_pack.default_encoding = field->defaultEncoding();
                field_pack.is_global_multi = field->isGlobalMulti();
                field_pack.is_array_row = field->isArrayRow();
                field_pack.is_broken = field->_is_broken;
                field_pack.can_set_value = field->canSetValue();
                field_pack.ancestor_count = field->_ancestor_count;
                field_pack.definer_itype = field->_definer_itype;
                field_pack.owner_itype = field->_owner_itype;
                field_pack.included_from_itype = field->_included_from_itype;
                field_pack.is_flattened_decor = field->isFlattenedDecor();
                field_pack.is_flattened_multi = field->isFlattenedMulti();
                field_pack.is_weak_reference = field->isWeakReference();
            }
            type_pack.dim_fields = utype->_fields.dim();
        }

        if( utype->_root_ifields.dim() ) {
            type_pack.start_root_fields = type_root_ifields.dim();
            for(idx utype_root_ifield = 0; utype_root_ifield < utype->_root_ifields.dim(); utype_root_ifield++) {
                idx utype_ifield = utype->_root_ifields[utype_root_ifield];
                idx ifield = utype_ifield + type_pack.start_fields;
                *type_root_ifields.add(1) = ifield;
            }
            type_pack.dim_root_fields = utype->_root_ifields.dim();
            *type_root_ifields.add(1) = -1;
        }
    }

    for(idx iroot = 0; iroot < sUsrLoadingType::_roots.dim(); iroot++) {
        *root_itypes.add(1) = sUsrLoadingType::_roots[iroot]->_itype;
    }

    if( const sUsrLoadingAction::ActRes * act_res = sUsrLoadingAction::_usr2actres.get(&user_id, sizeof(user_id)) ) {
        for(idx iaction = 0; iaction < act_res->acts.dim(); iaction++) {
            const sUsrLoadingAction * action = act_res->acts.ptr(iaction);
            ActionPack & action_pack = *action_packs.add(1);

            action_pack.id = action->id();
            action_pack.iaction = iaction;
            assert(action->_iaction == iaction);
            action_pack.pos_name = unique_strings.set(action->name());
            action_pack.pos_title = unique_strings.set(action->title());
            action_pack.pos_description = unique_strings.set(action->description());
            action_pack.pos_order = unique_strings.set(action->orderString());
            action_pack.is_object_action = action->isObjAction();
            action_pack.required_permission = action->requiredPermission();

            const sUsrObjRes::TObjProp & props = action->objProps();
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "align") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.pos_align = unique_strings.set(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "confirmation") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.confirmation = sString::parseIBool(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "created") ) {
                const char * val = act_res->res.getValue(tbl);
                sVariant parser;
                parser.parseDateTime(val);
                action_pack.created = parser.asInt();
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "modified") ) {
                const char * val = act_res->res.getValue(tbl);
                sVariant parser;
                parser.parseDateTime(val);
                action_pack.modified = parser.asInt();
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "icon") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.pos_icon = unique_strings.set(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "path") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.pos_path = unique_strings.set(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "response") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.pos_response = unique_strings.set(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "single_obj_only") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.single_obj_only = sString::parseIBool(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "target") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.pos_target = unique_strings.set(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "type_name") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.pos_type_name = unique_strings.set(val);
            }
            if( const sUsrObjRes::TPropTbl * tbl = act_res->res.get(props, "url") ) {
                const char * val = act_res->res.getValue(tbl);
                action_pack.pos_url = unique_strings.set(val);
            }
        }
    }

    if( const sUsrLoadingJSComponent::JscoRes * res = sUsrLoadingJSComponent::_usr2jscores.get(&user_id, sizeof(user_id)) ) {
        for(idx i = 0; i < res->jscos.dim(); ++i) {
            const sUsrLoadingJSComponent * c = res->jscos.ptr(i);
            JSComponentPack & pack = *jsco_packs.add(1);

            pack.id = c->id();
            pack.ijsco = i;
            assert(c->_ijsco == i);
            pack.pos_name = unique_strings.set(c->name());
            pack.preview = c->isPreview();
            pack.algoview = c->isAlgoview();
            pack.pos_order = unique_strings.set(c->orderString());
            const sUsrObjRes::TObjProp & props = c->objProps();
            if( const sUsrObjRes::TPropTbl * tbl = res->res.get(props, "created") ) {
                const char * val = res->res.getValue(tbl);
                sVariant parser;
                parser.parseDateTime(val);
                pack.created = parser.asInt();
            }
            if( const sUsrObjRes::TPropTbl * tbl = res->res.get(props, "modified") ) {
                const char * val = res->res.getValue(tbl);
                sVariant parser;
                parser.parseDateTime(val);
                pack.modified = parser.asInt();
            }
            if( const sUsrObjRes::TPropTbl * tbl = res->res.get(props, "type_name") ) {
                const char * val = res->res.getValue(tbl);
                pack.pos_type_name = unique_strings.set(val);
            }
        }
    }

    if( const sUsrLoadingView::ViewRes * view_res = sUsrLoadingView::_usr2viewres.get(&user_id, sizeof(user_id)) ) {
        for(idx iview = 0; iview < view_res->views.dim(); iview++) {
            const sUsrLoadingView * view = view_res->views.ptr(iview);
            ViewPack & view_pack = *view_packs.add(1);

            view_pack.id = view->id();
            view_pack.iview = iview;
            assert(view->_iview == iview);
            view_pack.pos_name = unique_strings.set(view->name());
            if( view->dimFields() ) {
                view_pack.start_fields = view_field_packs.dim();
                for(idx view_ifield = 0; view_ifield < view->dimFields(); view_ifield++) {
                    idx ifield = view_field_packs.dim();
                    FieldPack & field_pack = *view_field_packs.add(1);

                    field_pack.ifield = ifield;
                    field_pack.pos_name = unique_strings.set(view->fieldName(view_ifield));
                    field_pack.pos_default_value = unique_strings.set(view->fieldDefaultValue(view_ifield));
                    field_pack.pos_order = unique_strings.set(view->fieldOrderString(view_ifield));
                    field_pack.readonly = view->fieldReadonly(view_ifield) ? sUsrTypeField::eReadOnly : sUsrTypeField::eReadWrite;
                }
                view_pack.dim_fields = view->dimFields();
            }
        }
    }

    for(idx itype = 0; itype < sUsrLoadingType::_types.dim(); itype++) {
        const sUsrLoadingType * utype = sUsrLoadingType::_types[itype];
        TypePack & type_pack = type_packs[itype];
        if( !utype || !utype->id() ) {
            continue;
        }

        for(idx ifield = 0; ifield < utype->_fields.dim(); ifield++) {
            idx global_ifield = type_pack.start_fields + ifield;
            const sUsrLoadingTypeField * field = utype->_fields.ptr(ifield);
            FieldPack & field_pack = field_packs[global_ifield];

            if( field->parent() ) {
                canon.reset(utype->id(), 'f', field->parent()->name(), 0);
                const idx * pparent_ifield = dict.get(canon.key, canon.key_len);
                assert(pparent_ifield);
                field_pack.parent = *pparent_ifield;
            }
            if( field->dimChildren() ) {
                field_pack.start_children = child_ifields.dim();
                for(idx ichild = 0; ichild < field->dimChildren(); ichild++) {
                    canon.reset(utype->id(), 'f', field->getChild(ichild)->name(), 0);
                    const idx * pchild_ifield = dict.get(canon.key, canon.key_len);
                    assert(pchild_ifield);
                    *child_ifields.add(1) = *pchild_ifield;
                }
                field_pack.dim_children = field->dimChildren();
            }
        }

        if( idx dim_actions = utype->dimActions(user) ) {
            type_pack.start_all_actions = type_all_iactions.dim();
            for(idx type_iaction = 0; type_iaction < dim_actions; type_iaction++) {
                const sUsrLoadingAction * act = utype->getAction(user, type_iaction);
                idx iaction = act->_iaction;

                canon.reset(utype->id(), 'a', act->name(), 0);
                *dict.set(canon.key, canon.key_len) = iaction;
                *type_all_iactions.add(1) = iaction;
            }
            type_pack.dim_all_actions = dim_actions;
        }
        if( idx dim_jscos = utype->dimJSComponents(user) ) {
            type_pack.start_all_jscos = type_all_ijscos.dim();
            for(idx type_i = 0; type_i < dim_jscos; ++type_i) {
                const sUsrLoadingJSComponent * c = utype->getJSComponent(user, type_i);
                idx i = c->_ijsco;
                canon.reset(utype->id(), 'j', c->name(), 0);
                *dict.set(canon.key, canon.key_len) = i;
                *type_all_ijscos.add(1) = i;
            }
            type_pack.dim_all_jscos = dim_jscos;
        }
        if( idx dim_views = utype->dimViews(user) ) {
            type_pack.start_all_views = type_all_iviews.dim();
            for(idx type_iview = 0; type_iview < dim_views; type_iview++) {
                const sUsrLoadingView * view = utype->getView(user, type_iview);
                idx iview = view->_iview;

                canon.reset(utype->id(), 'v', view->name(), 0);
                *dict.set(canon.key, canon.key_len) = iview;
                *type_all_iactions.add(1) = iview;
            }
            type_pack.dim_all_views = dim_views;
        }
    }


    if( unique_strings.str2pos.dim() ) {
        const idx pos_strings = alignMex(&_mex, _mex.pos(), sSizePage);
        header()->pos_strings = pos_strings;
        for(idx i = 0; i < unique_strings.str2pos.dim(); i++) {
            idx s_len = 0;
            const char * s = static_cast<const char *>(unique_strings.str2pos.id(i, &s_len));
            if( s_len ) {
                _mex.add(s, s_len);
            }
            _mex.add(_, 1);
        }
        header()->size_strings = _mex.pos() - header()->pos_strings;
        assert(header()->size_strings == unique_strings.next_pos);
    }

#define COPY_PACKS(packname) \
    do { \
        if( packname.dim() ) { \\
            const idx pos_ ## packname = alignMex(&_mex, _mex.pos(), sSizePage); \
            header()->pos_ ## packname = pos_ ## packname; \
            _mex.add(packname.mex()->ptr(), packname.mex()->pos()); \
            header()->dim_ ## packname = packname.dim(); \
        } \
    } while( 0 )

    COPY_PACKS(field_packs);
    COPY_PACKS(child_ifields);
    COPY_PACKS(action_packs);
    COPY_PACKS(jsco_packs);
    COPY_PACKS(view_packs);
    COPY_PACKS(view_field_packs);
    COPY_PACKS(type_packs);
    COPY_PACKS(root_itypes);
    COPY_PACKS(dep_itypes);
    COPY_PACKS(type_root_ifields);
    COPY_PACKS(type_all_iactions);
    COPY_PACKS(type_all_ijscos);
    COPY_PACKS(type_all_iviews);

    if( dict.dim() ) {
        const idx pos_dict = alignMex(&_mex, _mex.pos(), sSizePage);
        header()->pos_dict = pos_dict;
        _mex.add(0, sizeof(DictHeader));
        new (_mex.ptr(header()->pos_dict)) DictHeader;
        for(idx i = 0; i < dict.dim(); i++) {
            idx key_len = 0;
            void * key = dict.id(i, &key_len);
            idx val = *dict.ptr(i);
            *setDict(key, key_len) = val;
        }
        header()->size_dict = _mex.pos() - header()->pos_dict;
    }


    header()->mtime = time(0);
    reset();

    if( !sFile::rename(temp_filename, filename) ) {
        reset();
        sFile::remove(temp_filename);
        return RC(sRC::eRenaming, sRC::eFile, sRC::eOperation, sRC::eFailed);
    }

    return sRC::zero;
}

sRC sUsrTypeCache::load(const char * filename)
{
    sStr buf;

    reset();

   _mex.init(filename, sMex::fReadonly);
   if( !_mex.ok() || _mex.pos() < (idx)sizeof(Header) ) {
       return RC(sRC::eOpening, sRC::eFile, sRC::eFile, sRC::eInvalid);
   }

   if( strncmp(const_cast<const char *>(header()->title), HEADER_TITLE, sizeof(header()->title)) ) {
       reset();
       return RC(sRC::eOpening, sRC::eFile, sRC::eHeader, sRC::eWrongFormat);
   }
   buf.cut0cut();
   buf.add(const_cast<const char *>(header()->version), sizeof(header()->version));
   buf.add0();

   char * dot = 0;
   idx major_version = strtoidx(buf.ptr(), &dot, 10);
   idx minor_version = dot && dot[0] == '.' ? atoidx(dot + 1) : 0;
   if( major_version != USR_TYPE_CACHE_MAJOR_VERSION || minor_version < USR_TYPE_CACHE_MINOR_VERSION ) {
       reset();
       return RC(sRC::eOpening, sRC::eFile, sRC::eVersion, sRC::eNotSupported);
   }
   if( strncmp(const_cast<const char *>(header()->platform), HEADER_PLATFORM, sizeof(header()->platform)) ) {
       reset();
       return RC(sRC::eOpening, sRC::eFile, sRC::eFormat, sRC::eNotSupported);
   }

   if( outOfRange(sizeof(Header), _mex.pos(), header()->pos_field_packs, header()->dim_field_packs * sizeof(FieldPack)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_child_ifields, header()->dim_child_ifields * sizeof(idx)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_action_packs, header()->dim_action_packs * sizeof(ActionPack)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_jsco_packs, header()->dim_jsco_packs * sizeof(JSComponentPack)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_view_packs, header()->dim_view_packs * sizeof(ViewPack)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_view_field_packs, header()->dim_view_field_packs * sizeof(FieldPack)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_type_packs, header()->dim_type_packs * sizeof(TypePack)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_root_itypes, header()->dim_root_itypes * sizeof(idx)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_dep_itypes, header()->dim_dep_itypes * sizeof(idx)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_type_root_ifields, header()->dim_type_root_ifields * sizeof(idx)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_type_all_iactions, header()->dim_type_all_iviews * sizeof(idx)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_type_all_ijscos, header()->dim_type_all_ijscos * sizeof(idx)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_type_all_iviews, header()->dim_type_all_iviews * sizeof(idx)) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_strings, header()->size_strings) ||
       outOfRange(sizeof(Header), _mex.pos(), header()->pos_dict, header()->size_dict) )
   {
       reset();
       return RC(sRC::eOpening, sRC::eFile, sRC::eHeader, sRC::eInvalid);
   }

   _fields.resizeM(header()->dim_field_packs);
   for(idx ifield = 0; ifield < header()->dim_field_packs; ifield++) {
       new (_fields.ptr(ifield)) CachedField(*this, *getFieldPack(ifield));
   }

   _actions.resizeM(header()->dim_action_packs);
   for(idx iaction = 0; iaction < header()->dim_action_packs; iaction++) {
       new (_actions.ptr(iaction)) CachedAction(*this, *getActionPack(iaction));
   }

   _jscos.resizeM(header()->dim_jsco_packs);
   for(idx i = 0; i < header()->dim_jsco_packs; ++i) {
       new (_jscos.ptr(i)) CachedJSComponent(*this, *getJSComponentPack(i));
   }

   _views.resizeM(header()->dim_view_packs);
   for(idx iview = 0; iview < header()->dim_view_packs; iview++) {
       new (_views.ptr(iview)) CachedView(*this, *getViewPack(iview));
   }

   _types.resizeM(header()->dim_type_packs);
   for(idx itype = 0; itype < header()->dim_type_packs; itype++) {
       new (_types.ptr(itype)) CachedType(*this, *getTypePack(itype));
   }

   return sRC::zero;
}

idx sUsrTypeCache::mtime() const
{
    return header() ? header()->mtime : 0;
}

void sUsrTypeCache::ensureActionViewPermissions(const sUsr & user) const
{
    udx user_id = user.Id();
    if( !_user2av_permissions_ensured.get(&user_id, sizeof(user_id)) ) {
        sVec<sHiveId> ids, ids_out;
        for(idx i = 0; i < dimActions(); i++) {
            *ids.add(1) = getAction(i)->id();
        }
        for(idx i = 0; i < dimJSComponents(); i++) {
            *ids.add(1) = getJSComponent(i)->id();
        }
        for(idx i = 0; i < dimViews(); i++) {
            *ids.add(1) = getView(i)->id();
        }
        user.objs(ids.ptr(), ids.dim(), ids_out);
        *_user2av_permissions_ensured.set(&user_id, sizeof(user_id)) = true;
    }
}

void sUsrTypeCache::CachedType::ensureActions(const sUsr & user) const
{
    udx user_id = user.Id();
    if( !_actions._usr2start.get(&user_id, sizeof(user_id)) ) {
        _cache.ensureActionViewPermissions(user);
        sMex::Pos * pos = _actions._usr2start.set(&user_id, sizeof(user_id));
        pos->pos = _actions._indices.dim();
        pos->size = 0;

        for(idx i = 0; i < _pack.dim_all_actions; i++) {
            idx type_all_iaction = _pack.start_all_actions + i;
            if( const CachedAction * act = _cache.getTypeAllAction(type_all_iaction) ) {
                if( user.objGet(act->id()) ) {
                    *_actions._indices.add(1) = act->_pack.iaction;
                    pos->size++;
                }
            }
        }
    }
}

void sUsrTypeCache::CachedType::ensureJSComponents(const sUsr & user) const
{
    udx user_id = user.Id();
    if( !_jscos._usr2start.get(&user_id, sizeof(user_id)) ) {
        _cache.ensureActionViewPermissions(user);
        sMex::Pos * pos = _jscos._usr2start.set(&user_id, sizeof(user_id));
        pos->pos = _jscos._indices.dim();
        pos->size = 0;

        for(idx i = 0; i < _pack.dim_all_jscos; i++) {
            idx type_all_i = _pack.start_all_jscos + i;
            if( const CachedJSComponent * c = _cache.getTypeAllJSComponent(type_all_i) ) {
                if( user.objGet(c->id()) ) {
                    *_jscos._indices.add(1) = c->_pack.ijsco;
                    pos->size++;
                }
            }
        }
    }
}

void sUsrTypeCache::CachedType::ensureViews(const sUsr & user) const
{
    udx user_id = user.Id();
    if( !_views._usr2start.get(&user_id, sizeof(user_id)) ) {
        _cache.ensureActionViewPermissions(user);
        sMex::Pos * pos = _views._usr2start.set(&user_id, sizeof(user_id));
        pos->pos = _views._indices.dim();
        pos->size = 0;

        for(idx i = 0; i < _pack.dim_all_views; i++) {
            idx type_all_iview = _pack.start_all_views + i;
            if( const CachedView * view = _cache.getTypeAllView(type_all_iview) ) {
                if( user.objGet(view->id()) ) {
                    *_views._indices.add(1) = view->_pack.iview;
                    pos->size++;
                }
            }
        }
    }
}


#define TBITS 3
#define DICT_COLLISION_REDUCER 2

idx sUsrTypeCache::dimDict() const
{
    return dictHeader() ? sAlgo::lix_cnt(&_mex, dictHeader()->lst0) : 0;
}

idx * sUsrTypeCache::setDict(const void * id, idx len_id, idx * piobj)
{
    if( dictHeader() ) {
        idx iobj = sAlgo::hax_find(&_mex, id, len_id, 0, 0, dictHeader()->hax0) - 1;
        if( iobj < 0 ) {
            iobj = dimDict();
            sAlgo::lix_add(&_mex, 1, sizeof(idx), &(dictHeader()->lst0), TBITS);
            *static_cast<idx*>(_mex.ptr(_mex.add(0, sizeof(idx)))) = 0;
            idx id_pos = _mex.add(0, len_id + 1);
            memcpy(_mex.ptr(id_pos), id, len_id);
            *static_cast<char*>(_mex.ptr(id_pos + len_id)) = 0;
            sAlgo::hax_map(&_mex, iobj, id_pos, len_id, 0, &(dictHeader()->hax0), TBITS, DICT_COLLISION_REDUCER, TBITS);
        }
        if( piobj ) {
            *piobj = iobj;
        }
        return ptrDict(iobj);
    }
    return 0;
}

idx * sUsrTypeCache::getDict(const void * id, idx len_id, idx * piobj)
{
    idx iobj = dictHeader() ? sAlgo::hax_find(&_mex, id, len_id, 0, 0, dictHeader()->hax0) - 1 : -1;
    if( piobj ) {
        *piobj = iobj;
    }

    if( iobj >= 0 ) {
        return ptrDict(iobj);
    } else {
        return 0;
    }
}

idx * sUsrTypeCache::ptrDict(idx iobj)
{
    return static_cast<idx*>(sAlgo::lix_ptr(&_mex, iobj, sizeof(idx), (dictHeader()->lst0)));
}
