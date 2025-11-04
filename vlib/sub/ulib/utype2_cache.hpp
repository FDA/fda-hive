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
#ifndef sLib_usrtype2_cache_hpp
#define sLib_usrtype2_cache_hpp

#include "utype2_loading.hpp"

#define USR_TYPE_CACHE_MAJOR_VERSION 1
#define USR_TYPE_CACHE_MINOR_VERSION 3

namespace slib {
    using namespace slib;
    class sUsrTypeCache {
        public:
            class CachedField;
            class CachedAction;
            class CachedView;
            class CachedType;

            sUsrTypeCache();

            const char * getString(idx pos) const;

            idx dimFields() const;
            const CachedField * getField(idx ifield) const { return ifield >= 0 && ifield < _fields.dim() ? _fields.ptr(ifield) : 0; }
            const CachedField * getTypeField(const sHiveId & type_id, const char * name, idx len = 0) const;
            const CachedField * getChildField(idx ichild) const;

            const CachedField * getTypeRootField(idx iroot) const;

            idx dimActions() const;
            const CachedAction * getAction(idx iaction) const { return iaction >= 0 && iaction < _actions.dim() ? _actions.ptr(iaction) : 0; }
            const CachedAction * getTypeAction(const sHiveId & type_id, const char * name, idx len = 0) const;
            const CachedAction * getTypeAllAction(idx type_iaction) const;

            idx dimViews() const;
            const CachedView * getView(idx iview) const { return iview >= 0 && iview < _views.dim() ? _views.ptr(iview) : 0; }
            const CachedView * getTypeView(const sHiveId & type_id, const char * name, idx len = 0) const;
            const CachedView * getTypeAllView(idx type_iview) const;

            idx dimTypes() const;
            const CachedType * getType(idx itype) const { return itype >= 0 && itype < _types.dim() ? _types.ptr(itype) : 0; }
            const CachedType * getType(const char * name, idx len = 0) const;
            const CachedType * getType(const sHiveId & id) const;
            const CachedType * getDepType(idx idep) const;

            idx dimRootTypes() const;
            const CachedType * getRootType(idx iroot) const;

            sRC save(const sUsr & user, const char * filename);
            sRC load(const char * filename);
            idx mtime() const;

        private:
            struct Header {
                char title[32];
                char version[32];
                char platform[32];
                int64_t mtime;
                int64_t pos_field_packs;
                int64_t dim_field_packs;
                int64_t pos_child_ifields;
                int64_t dim_child_ifields;
                int64_t pos_action_packs;
                int64_t dim_action_packs;
                int64_t pos_view_packs;
                int64_t dim_view_packs;
                int64_t pos_view_field_packs;
                int64_t dim_view_field_packs;
                int64_t pos_type_packs;
                int64_t dim_type_packs;
                int64_t pos_root_itypes;
                int64_t dim_root_itypes;
                int64_t pos_dep_itypes;
                int64_t dim_dep_itypes;
                int64_t pos_type_root_ifields;
                int64_t dim_type_root_ifields;
                int64_t pos_type_all_iactions;
                int64_t dim_type_all_iactions;
                int64_t pos_type_all_iviews;
                int64_t dim_type_all_iviews;
                int64_t pos_strings;
                int64_t size_strings;
                int64_t pos_dict;
                int64_t size_dict;

                Header() { sSet(this, 0); }
            };
            struct DictHeader {
                sAlgo::lix lst0;
                sAlgo::lix hax0;
                DictHeader() { lst0 = hax0 = sAlgo::emptyLix(); }
            };
            struct FieldPack {
                idx ifield;
                idx pos_name, pos_orig_name;
                idx pos_title;
                sUsrTypeField::EType type;
                idx pos_parent_name;
                idx parent;
                sUsrTypeField::ERole role;
                idx dim_children;
                idx start_children;
                sUsrTypeField::EReadOnly readonly;
                idx is_key;
                idx is_optional;
                idx is_multi;
                idx is_hidden;
                idx is_summary;
                idx is_virtual;
                idx is_batch;
                idx is_sysinternal;
                idx pos_brief;
                idx pos_order;
                idx pos_default_value;
                idx pos_constraint, pos_constraint_data, pos_constraint_description;
                idx pos_description;
                idx pos_link_url;
                idx default_encoding;

                idx is_global_multi, is_array_row, is_broken;
                idx can_set_value;
                idx ancestor_count;
                idx definer_itype;
                idx owner_itype;
                idx included_from_itype;
                idx is_flattened_decor;
                idx is_flattened_multi;

                idx is_weak_reference;

                idx _reserved[3];

                FieldPack() {
                    sSet(this, 0);
                    type = sUsrTypeField::eInvalid;
                    role = sUsrTypeField::eRole_unknown;
                    ifield = pos_name = pos_orig_name = pos_title = pos_parent_name = parent = start_children = pos_brief = pos_order = pos_default_value = pos_constraint = pos_constraint_data = pos_constraint_description = pos_description = pos_link_url = definer_itype = owner_itype = included_from_itype = -1;
                }
            };
            const FieldPack * getFieldPack(idx ifield) const;

            struct ActionPack {
                sHiveId id;
                idx iaction;
                idx pos_name;
                idx pos_title;
                idx pos_description;
                idx pos_order;
                idx is_object_action;
                udx required_permission;

                idx pos_align;
                idx confirmation;
                idx created;
                idx modified;
                idx pos_icon;
                idx pos_path;
                idx pos_response;
                idx single_obj_only;
                idx pos_target;
                idx pos_type_name;
                idx pos_url;

                idx _reserved[4];

                ActionPack()
                {
                    sSet(this, 0);
                    iaction = pos_name = pos_title = pos_order = -1;
                }
            };
            const ActionPack * getActionPack(idx iaction) const;

            struct ViewPack {
                sHiveId id;
                idx iview;
                idx pos_name;
                idx start_fields;
                idx dim_fields;

                idx _reserved[4];

                ViewPack()
                {
                    sSet(this, 0);
                    iview = pos_name = start_fields = -1;
                }
            };
            const ViewPack * getViewPack(idx iview) const;
            const FieldPack * getViewFieldPack(idx ifield) const;

            struct TypePack {
                sHiveId id;
                idx itype;
                idx pos_name;
                idx pos_title;
                idx pos_description;
                idx created;
                idx modified;
                idx is_virtual;
                idx is_user;
                idx is_system;
                idx is_prefetch;
                sUsrType2::ESingleton is_singleton;
                idx is_broken;

                idx start_parents;
                idx dim_parents;
                idx start_children;
                idx dim_children;
                idx start_includes;
                idx dim_includes;

                idx start_fields;
                idx dim_fields;

                idx start_root_fields;
                idx dim_root_fields;

                idx start_all_actions;
                idx dim_all_actions;
                idx start_all_views;
                idx dim_all_views;

                idx _reserved[4];

                TypePack()
                {
                    sSet(this, 0);
                    itype = pos_name = pos_title = pos_description = start_parents = start_children = start_includes = start_fields = start_root_fields = start_all_actions = start_all_views = -1;
                }
            };
            const TypePack * getTypePack(idx itype) const;

        public:
            class CachedField : public sUsrTypeField {
                public:
                    CachedField(const sUsrTypeCache & cache, const FieldPack & pack) : _cache(cache), _pack(pack) {}
                    virtual ~CachedField() {}

                    virtual const char * name() const { return _cache.getString(_pack.pos_name); }
                    virtual const char * originalName() const { return _cache.getString(_pack.pos_orig_name); }
                    virtual const char * title() const { return _cache.getString(_pack.pos_title); }
                    virtual EType type() const { return _pack.type; }
                    virtual const CachedField * parent() const { return _cache.getField(_pack.parent); }
                    virtual ERole role() const { return _pack.role; }
                    virtual idx dimChildren() const { return _pack.dim_children; }
                    virtual const CachedField * getChild(idx ichild) const
                    {
                        if( ichild >= 0 && ichild < dimChildren() ) {
                            return _cache.getChildField(_pack.start_children + ichild);
                        }
                        return 0;
                    }
                    virtual idx getChildren(sVec<const sUsrTypeField*> &out) const
                    {
                        idx start = out.dim();
                        idx cnt = dimChildren();
                        out.add(cnt);
                        for(idx ichild = 0; ichild < cnt; ichild++) {
                            out[start + ichild] = _cache.getChildField(_pack.start_children + ichild);
                        }
                        return cnt;
                    }
                    virtual bool isKey() const { return _pack.is_key; }
                    virtual EReadOnly readonly() const { return _pack.readonly; }
                    virtual bool isOptional() const { return _pack.is_optional; }
                    virtual bool isMulti() const { return _pack.is_multi; }
                    virtual bool isHidden() const { return _pack.is_hidden; }
                    virtual bool isSummary() const { return _pack.is_summary; }
                    virtual bool isVirtual() const { return _pack.is_virtual; }
                    virtual bool isBatch() const { return _pack.is_batch; }
                    virtual bool isWeakReference() const { return _pack.is_weak_reference; }
                    virtual bool isSysInternal() const { return _pack.is_sysinternal; }
                    virtual const char * brief() const { return _cache.getString(_pack.pos_brief); }
                    virtual real order() const
                    {
                        const char * ostring = orderString();
                        return ostring ? strtod(ostring, 0) : 0;
                    }
                    virtual const char * orderString() const { return _cache.getString(_pack.pos_order); }
                    virtual const char * defaultValue() const { return _cache.getString(_pack.pos_default_value); }
                    virtual idx defaultEncoding() const { return _pack.default_encoding; }
                    virtual const char * constraint() const { return _cache.getString(_pack.pos_constraint); }
                    virtual const char * constraintData() const { return _cache.getString(_pack.pos_constraint_data); }
                    virtual const char * constraintDescription() const { return _cache.getString(_pack.pos_constraint_description); }
                    virtual const char * description() const { return _cache.getString(_pack.pos_description); }
                    virtual const char * linkUrl() const { return _cache.getString(_pack.pos_link_url); }

                    virtual bool canSetValue() const { return _pack.can_set_value; }
                    virtual bool isArrayRow() const { return _pack.is_array_row; }
                    virtual bool isGlobalMulti() const { return _pack.is_global_multi; }
                    virtual bool isFlattenedDecor() const { return _pack.is_flattened_decor; }
                    virtual bool isFlattenedMulti() const { return _pack.is_flattened_multi; }
                    virtual idx ancestorCount() const { return _pack.ancestor_count; }

                    virtual const sUsrType2 * ownerType() const { return _cache.getType(_pack.owner_itype); }
                    virtual const sUsrType2 * definerType() const { return _cache.getType(_pack.definer_itype); }
                    virtual const sUsrType2 * includedFromType() const { return _cache.getType(_pack.included_from_itype); }

                private:
                    const sUsrTypeCache & _cache;
                    const FieldPack & _pack;
            };
            friend class CachedField;

            class CachedAction : public sUsrAction {
                public:
                    CachedAction(const sUsrTypeCache & cache, const ActionPack & pack) : _cache(cache), _pack(pack) {}
                    virtual ~CachedAction() {}

                    virtual const sHiveId & id() const { return _pack.id; }
                    virtual const char * name() const { return _cache.getString(_pack.pos_name); }
                    virtual const char * title() const { return _cache.getString(_pack.pos_title); }
                    virtual const char * description() const { return _cache.getString(_pack.pos_description); }
                    virtual real order() const
                    {
                        const char * ostring = orderString();
                        return ostring ? strtod(ostring, 0) : 0;
                    }
                    virtual const char * orderString() const { return _cache.getString(_pack.pos_order); }
                    virtual bool isObjAction() const { return _pack.is_object_action; }
                    virtual udx requiredPermission() const { return _pack.required_permission; }

                private:
                    const sUsrTypeCache & _cache;
                    const ActionPack & _pack;

                    friend class CachedType;
            };
            friend class CachedAction;

            class CachedView : public sUsrView {
                public:
                    CachedView(const sUsrTypeCache & cache, const ViewPack & pack) : _cache(cache), _pack(pack) {}
                    virtual ~CachedView() {}

                    virtual const sHiveId & id() const { return _pack.id; }
                    virtual const char * name() const { return _cache.getString(_pack.pos_name); }

                    virtual idx dimFields() const { return _pack.dim_fields; }
                    const FieldPack * fieldPack(idx ifield) const
                    {
                        if( ifield >= 0 && ifield < dimFields() ) {
                            return _cache.getViewFieldPack(_pack.start_fields + ifield);
                        }
                        return 0;
                    }
                    virtual const char * fieldName(idx ifield) const
                    {
                        if( const FieldPack * fld_pack = fieldPack(ifield) ) {
                            return _cache.getString(fld_pack->pos_name);
                        }
                        return 0;
                    }
                    virtual const char * fieldDefaultValue(idx ifield) const
                    {
                        if( const FieldPack * fld_pack = fieldPack(ifield) ) {
                            return _cache.getString(fld_pack->pos_default_value);
                        }
                        return 0;
                    }
                    virtual real fieldOrder(idx ifield) const
                    {
                        const char * sorder = fieldOrderString(ifield);
                        return sorder ? strtod(sorder, 0) : 0;
                    }
                    virtual const char * fieldOrderString(idx ifield) const
                    {
                        if( const FieldPack * fld_pack = fieldPack(ifield) ) {
                            return _cache.getString(fld_pack->pos_order);
                        }
                        return 0;
                    }
                    virtual bool fieldReadonly(idx ifield) const
                    {
                        if( const FieldPack * fld_pack = fieldPack(ifield) ) {
                            return fld_pack->readonly != sUsrTypeField::eReadWrite;
                        }
                        return 0;
                    }

                private:
                    const sUsrTypeCache & _cache;
                    const ViewPack & _pack;

                    friend class CachedType;
            };
            friend class CachedView;

            void ensureActionViewPermissions(const sUsr & user) const;

            class CachedType : public sUsrType2 {
                public:
                    CachedType(const sUsrTypeCache & cache, const TypePack & pack) : _cache(cache), _pack(pack) {}
                    virtual ~CachedType() {}


                    virtual const sHiveId & id() const { return _pack.id; }
                    virtual const char * name() const { return _cache.getString(_pack.pos_name); }
                    virtual const char * title() const { return _cache.getString(_pack.pos_title); }
                    virtual const char * description() const { return _cache.getString(_pack.pos_description); }
                    virtual bool isVirtual() const { return _pack.is_virtual; }
                    virtual bool isPrefetch() const { return _pack.is_prefetch; }
                    virtual bool isUser() const { return _pack.is_user; }
                    virtual bool isSystem() const { return _pack.is_system; }

                    virtual ESingleton isSingleton() const { return _pack.is_singleton; }


                    virtual idx dimParents() const { return _pack.dim_parents; }
                    virtual const CachedType * getParent(idx iparent) const
                    {
                        if( iparent >= 0 && iparent < dimParents() ) {
                            return _cache.getDepType(_pack.start_parents + iparent);
                        }
                        return 0;
                    }
                    virtual idx getParents(sVec<const sUsrType2*> & out) const
                    {
                        idx start = out.dim();
                        idx cnt = dimParents();
                        out.add(cnt);
                        for(idx iparent = 0; iparent < cnt; iparent++) {
                            out[start + iparent] = _cache.getDepType(_pack.start_parents + iparent);
                        }
                        return cnt;
                    }

                    virtual idx dimChildren() const { return _pack.dim_children; }
                    virtual const CachedType * getChild(idx ichild) const
                    {
                        if( ichild >= 0 && ichild < dimChildren() ) {
                            return _cache.getDepType(_pack.start_children + ichild);
                        }
                        return 0;
                    }
                    virtual idx getChildren(sVec<const sUsrType2*> & out) const
                    {
                        idx start = out.dim();
                        idx cnt = dimChildren();
                        out.add(cnt);
                        for(idx ichild = 0; ichild < cnt; ichild++) {
                            out[start + ichild] = _cache.getDepType(_pack.start_children + ichild);
                        }
                        return cnt;
                    }

                    virtual idx dimIncludes() const { return _pack.dim_includes; }
                    virtual const CachedType * getInclude(idx iinc) const
                    {
                        if( iinc >= 0 && iinc < dimIncludes() ) {
                            return _cache.getDepType(_pack.start_includes + iinc);
                        }
                        return 0;
                    }
                    virtual idx getIncludes(sVec<const sUsrType2*> & out) const
                    {
                        idx start = out.dim();
                        idx cnt = dimIncludes();
                        out.add(cnt);
                        for(idx iinc = 0; iinc < cnt; iinc++) {
                            out[start + iinc] = _cache.getDepType(_pack.start_includes + iinc);
                        }
                        return cnt;
                    }


                    virtual idx dimFields(const sUsr & user) const { return _pack.dim_fields; }
                    virtual idx dimRootFields(const sUsr & user) const { return _pack.dim_root_fields; }
                    virtual const CachedField* getField(const sUsr & user, idx ifield) const
                    {
                        if( ifield >= 0 && ifield < dimFields(user) ) {
                            return _cache.getField(_pack.start_fields + ifield);
                        }
                        return 0;
                    }
                    virtual const CachedField* getRootField(const sUsr & user, idx irootfld) const
                    {
                        if( irootfld >= 0 && irootfld < dimRootFields(user) ) {
                            return _cache.getTypeRootField(_pack.start_root_fields + irootfld);
                        }
                        return 0;
                    }
                    virtual const CachedField* getField(const sUsr & user, const char * field_name, idx field_name_len = 0) const { return _cache.getTypeField(id(), field_name, field_name_len); }

                    virtual idx dimActions(const sUsr & user) const
                    {
                        udx user_id = user.Id();
                        ensureActions(user);
                        if( const sMex::Pos * pos = _actions._usr2start.get(&user_id, sizeof(user_id)) ) {
                            return pos->size;
                        }
                        return 0;
                    }
                    virtual const CachedAction * getAction(const sUsr & user, const char * act_name, idx act_name_len = 0) const
                    {
                        ensureActions(user);
                        const CachedAction * act = _cache.getTypeAction(id(), act_name, act_name_len);
                        if( act && user.objGet(act->id()) ) {
                            return act;
                        }
                        return 0;
                    }
                    virtual const CachedAction * getAction(const sUsr & user, idx iact) const
                    {
                        udx user_id = user.Id();
                        ensureActions(user);
                        if( const sMex::Pos * pos = _actions._usr2start.get(&user_id, sizeof(user_id)) ) {
                            if( iact >= 0 && iact < pos->size ) {
                                return _cache.getAction(_actions._indices[pos->pos + iact]);
                            }
                        }
                        return 0;
                    }
                    virtual idx getActions(const sUsr & user, sVec<const sUsrAction *> & out) const
                    {
                        udx user_id = user.Id();
                        ensureActions(user);
                        idx cnt = 0;
                        if( const sMex::Pos * pos = _actions._usr2start.get(&user_id, sizeof(user_id)) ) {
                            if( pos->size ) {
                                idx out_start = out.dim();
                                out.add(pos->size);
                                for(idx iact = 0; iact < pos->size; iact++) {
                                    out[out_start + iact] = _cache.getAction(_actions._indices[pos->pos + iact]);
                                }
                            }
                            return pos->size;
                        }
                        return cnt;
                    }

                    virtual idx dimViews(const sUsr & user) const
                    {
                        udx user_id = user.Id();
                        ensureViews(user);
                        if( const sMex::Pos * pos = _views._usr2start.get(&user_id, sizeof(user_id)) ) {
                            return pos->size;
                        }
                        return 0;
                    }
                    virtual const sUsrView * getView(const sUsr & user, const char * view_name, idx view_name_len = 0) const
                    {
                        ensureViews(user);
                        const CachedView * view = _cache.getTypeView(id(), view_name, view_name_len);
                        if( view && user.objGet(view->id()) ) {
                            return view;
                        }
                        return 0;
                    }
                    virtual const sUsrView * getView(const sUsr & user, idx iview) const
                    {
                        udx user_id = user.Id();
                        ensureViews(user);
                        if( const sMex::Pos * pos = _views._usr2start.get(&user_id, sizeof(user_id)) ) {
                            if( iview >= 0 && iview < pos->size ) {
                                return _cache.getView(_views._indices[pos->pos + iview]);
                            }
                        }
                        return 0;
                    }
                    virtual idx getViews(const sUsr & user, sVec<const sUsrView *> & out) const
                    {
                        udx user_id = user.Id();
                        ensureViews(user);
                        idx cnt = 0;
                        if( const sMex::Pos * pos = _views._usr2start.get(&user_id, sizeof(user_id)) ) {
                            if( pos->size ) {
                                idx out_start = out.dim();
                                out.add(pos->size);
                                for(idx iview = 0; iview < pos->size; iview++) {
                                    out[out_start + iview] = _cache.getView(_views._indices[pos->pos + iview]);
                                }
                            }
                            return pos->size;
                        }
                        return cnt;
                    }

                private:
                    void ensureActions(const sUsr & user) const;
                    void ensureViews(const sUsr & user) const;

                    const sUsrTypeCache & _cache;
                    const TypePack & _pack;

                    mutable struct PerUserRes {
                        sDic<sMex::Pos> _usr2start;
                        sVec<idx> _indices;

                        idx dimForUser(const sUsr & user) const;
                        const idx * startForUser(const sUsr & user) const;
                    } _actions, _views;
            };
            friend class CachedType;

        private:
            volatile Header * header() { return static_cast<Header*>(_mex.ptr()); }
            const Header * header() const { return static_cast<const Header*>(_mex.ptr()); }
            DictHeader * dictHeader()
            {
                volatile Header * hdr = header();
                return hdr ? static_cast<DictHeader*>(_mex.ptr(hdr->pos_dict)) : 0;
            }
            const DictHeader * dictHeader() const
            {
                const Header * hdr = header();
                return hdr ? static_cast<const DictHeader*>(_mex.ptr(hdr->pos_dict)) : 0;
            }

            idx * setDict(const void * id, idx len_id, idx * piobj = 0);
            idx * getDict(const void * id, idx len_id, idx * piobj = 0);
            const idx * getDict(const void * id, idx len_id, idx * piobj = 0) const { return const_cast<sUsrTypeCache*>(this)->getDict(id, len_id, piobj); }
            idx * ptrDict(idx iobj);
            const idx * ptrDict(idx iobj) const { return const_cast<sUsrTypeCache*>(this)->ptrDict(iobj); }
            idx dimDict() const;

            void reset();

            sMex _mex;
            sVec<CachedField> _fields;
            sVec<CachedAction> _actions;
            sVec<CachedView> _views;
            mutable sDic<bool> _user2av_permissions_ensured;
            sVec<CachedType> _types;
    };
};

#endif 