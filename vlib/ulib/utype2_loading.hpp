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
#ifndef sLib_usrtype2_loading_hpp
#define sLib_usrtype2_loading_hpp

#include <ulib/utype2.hpp>
#include <slib/core/net.hpp>

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

namespace {
};

class slib::sUsrLoadingTypeField : public sUsrTypeField {
    public:
        virtual ~sUsrLoadingTypeField() {}

        virtual const char * name() const { return getString(_pos_name); }
        virtual const char * originalName() const { return getString(_pos_orig_name); }
        virtual const char * title() const { return getString(_pos_title); }
        virtual EType type() const { return _type; }
        virtual const sUsrLoadingTypeField * parent() const;
        virtual ERole role() const { return _role; }
        virtual idx dimChildren() const { return _dim_children; }
        virtual const sUsrLoadingTypeField * getChild(idx ichild) const;
        virtual idx getChildren(sVec<const sUsrTypeField*> &out) const;
        virtual bool isKey() const { return _is_key; }
        virtual EReadOnly readonly() const { return _readonly; }
        virtual bool isOptional() const { return _is_optional; }
        virtual bool isMulti() const { return _is_multi; }
        virtual bool isHidden() const { return _is_hidden; }
        virtual bool isSummary() const { return _is_summary; }
        virtual bool isVirtual() const { return _is_virtual; }
        virtual bool isBatch() const { return _is_batch; }
        virtual bool isWeakReference() const { return _is_weak_reference; }
        virtual bool isSysInternal() const { return _is_sysinternal; }
        virtual const char * brief() const { return getString(_pos_brief); }
        virtual real order() const { return strtod(getString(_pos_order), 0); }
        virtual const char * orderString() const { return getString(_pos_order); }
        virtual const char * defaultValue() const { return getString(_pos_default_value); }
        virtual idx defaultEncoding() const { return _default_encoding; }
        virtual const char * constraint() const { return getString(_pos_constraint); }
        virtual const char * constraintData() const { return getString(_pos_constraint_data); }
        virtual const char * constraintDescription() const { return getString(_pos_constraint_description); }
        virtual const char * description() const { return getString(_pos_description); }
        virtual const char * linkUrl() const { return getString(_pos_link_url); }

        virtual bool canHaveValue() const { return _type != eInvalid && _type != eArray && _type != eList && _type != eArrayTab && _type != eListTab; }
        virtual bool canSetValue() const;
        virtual bool isArrayRow() const { return _is_array_row; }
        virtual bool isGlobalMulti() const { return _is_global_multi; }
        virtual bool isFlattenedDecor() const;
        virtual bool isFlattenedMulti() const;
        virtual idx ancestorCount() const { return _ancestor_count; }

        virtual const sUsrType2 * ownerType() const;
        virtual const sUsrType2 * definerType() const;
        virtual const sUsrType2 * includedFromType() const;

    private:
        sUsrLoadingTypeField(bool default_zero = false);

        idx _index;
        idx _pos_name, _pos_orig_name;
        idx _pos_title;
        EType _type;
        idx _pos_parent_name;
        idx _parent;
        ERole _role;
        idx _dim_children;
        idx _start_children;
        EReadOnly _readonly;
        bool _is_key;
        bool _is_optional;
        bool _is_multi;
        bool _is_hidden;
        bool _is_summary;
        bool _is_virtual;
        bool _is_batch;
        bool _is_weak_reference;
        bool _is_sysinternal;
        idx _pos_brief;
        idx _pos_order;
        idx _pos_default_value;
        idx _pos_constraint, _pos_constraint_data, _pos_constraint_description;
        idx _pos_description;
        idx _pos_link_url;
        idx _default_encoding;

        bool _is_global_multi, _is_array_row, _is_broken;
        idx _ancestor_count;
        idx _definer_itype;
        idx _owner_itype;
        idx _included_from_itype;

        mutable enum ELazy {
            eLazyNotLoaded,
            eLazyTrue,
            eLazyFalse
        } _is_flattened_decor;

        static sStr _name_buf;

        friend class sUsrLoadingType;
        friend class sUsrTypeCache;

        static const char * getString(idx pos) { return pos >= 0 ? _name_buf.ptr(pos) : sStr::zero; }
        static idx setString(const char * s, bool canonicalize=false, bool allow_empty=false);
        static void replaceString(idx & pos, const char * find00, const char * replace00, sStr & buf);
};

class slib::sUsrLoadingAction : public sUsrAction
{
    public:
        static const sUsrLoadingAction * get(const sUsr & user, const char * act_name, udx type_id = 0);
        static const sUsrLoadingAction * get(const sUsr & user, const char * act_name, const char * type_name);

        virtual const sHiveId & id() const { return _id; }
        virtual const char * name() const;
        virtual const char * title() const;
        virtual const char * description() const;
        virtual real order() const;
        virtual const char * orderString() const;
        virtual bool isObjAction() const;
        virtual udx requiredPermission() const;


        const sUsrObjRes & objRes() const
        {
            return _usr2actres.get(&_user_id, sizeof(_user_id))->res;
        }
        const sUsrObjRes::TObjProp & objProps() const
        {
            return *objRes().get(_id);
        }

        virtual ~sUsrLoadingAction() {}

    private:
        sHiveId _id;
        udx _user_id;
        idx _iaction;

        friend class sUsrType2;
        friend class sUsrLoadingType;
        friend class sUsrTypeCache;

        struct ActRes {
            sVec<sUsrLoadingAction> acts;
            sUsrObjRes res;
        };

        static sDic<ActRes> _usr2actres;
        static sStr _name_buf;
};

class slib::sUsrLoadingJSComponent : public sUsrJSComponent
{
    public:
        static const sUsrLoadingJSComponent * get(const sUsr & user, const char * name, udx type_id = 0);
        static const sUsrLoadingJSComponent * get(const sUsr & user, const char * name, const char * type_name);

        virtual const sHiveId & id() const { return _id; }
        virtual const char * name() const;
        virtual bool isPreview() const;
        virtual bool isAlgoview() const;
        virtual real order() const;
        virtual const char * orderString() const;


        const sUsrObjRes & objRes() const
        {
            return _usr2jscores.get(&_user_id, sizeof(_user_id))->res;
        }
        const sUsrObjRes::TObjProp & objProps() const
        {
            return *objRes().get(_id);
        }

        virtual ~sUsrLoadingJSComponent() {}

    private:
        sHiveId _id;
        udx _user_id;
        idx _ijsco;

        friend class sUsrType2;
        friend class sUsrLoadingType;
        friend class sUsrTypeCache;

        struct JscoRes {
            sVec<sUsrLoadingJSComponent> jscos;
            sUsrObjRes res;
        };

        static sDic<JscoRes> _usr2jscores;
        static sStr _name_buf;
};

class slib::sUsrLoadingView : public sUsrView
{
    public:
        static const sUsrLoadingView * get(const sUsr & user, const char * view_name, udx type_id = 0);
        static const sUsrLoadingView * get(const sUsr & user, const char * view_name, const char * type_name);

        virtual const sHiveId & id() const { return _id; }
        virtual const char * name() const;

        virtual idx dimFields() const;
        virtual const char * fieldName(idx ifield) const;
        virtual const char * fieldDefaultValue(idx ifield) const;
        virtual real fieldOrder(idx ifield) const
        {
            const char * sorder = fieldOrderString(ifield);
            return sorder ? strtod(sorder, 0) : 0;
        }
        virtual const char * fieldOrderString(idx ifield) const;
        virtual bool fieldReadonly(idx ifield) const;

        const sUsrObjRes & objRes() const
        {
            return _usr2viewres.get(&_user_id, sizeof(_user_id))->res;
        }
        const sUsrObjRes::TObjProp & objProps() const
        {
            return *objRes().get(_id);
        }

        virtual ~sUsrLoadingView() {}

    private:
        sHiveId _id;
        udx _user_id;
        idx _iview;

        friend class sUsrType2;
        friend class sUsrLoadingType;
        friend class sUsrTypeCache;

        struct ViewRes {
            sVec<sUsrLoadingView> views;
            sUsrObjRes res;
        };
        struct Field {
            idx pos_name;
            idx pos_default_value;
            idx pos_order_string;
            bool readonly;
        };
        sLst<Field> _fields;

        static sMex _fields_buf;
        static sDic<ViewRes> _usr2viewres;
        static sDic<bool> _names;
        static sUsrType2 * _view_type;
};

class slib::sUsrLoadingType : public sUsrType2
{
    public:
        static sHiveId type_type_id;

        virtual ~sUsrLoadingType() {}


        virtual const sHiveId & id() const { return _id; }
        virtual const char * name() const { return _pos_name >= 0 ? _name_buf.ptr(_pos_name) : 0; }
        virtual const char * title() const { return _pos_title >= 0 ? _name_buf.ptr(_pos_title) : 0; }
        virtual const char * description() const { return _pos_description >= 0 ? _name_buf.ptr(_pos_description) : 0; }
        virtual bool isVirtual() const { return _is_virtual; }
        virtual bool isPrefetch() const { return _is_prefetch; }
        virtual bool isUser() const;
        bool isUser(sDic<idx> & seen_dic) const;
        virtual bool isSystem() const;
        bool isSystem(sDic<idx> & seen_dic) const;

        virtual ESingleton isSingleton() const { return _is_singleton; }


        virtual idx dimParents() const { return _parents.dim(); }
        virtual const sUsrLoadingType * getParent(idx iparent) const { return iparent >= 0 && iparent < dimParents() ? _types[_parents[iparent]] : 0; }
        virtual idx getParents(sVec<const sUsrType2*> & out) const;

        virtual idx dimChildren() const { return _children.dim(); }
        virtual const sUsrLoadingType * getChild(idx ichild) const { return ichild >= 0 && ichild < dimChildren() ? _types[_children[ichild]] : 0; }
        virtual idx getChildren(sVec<const sUsrType2*> & out) const;

        virtual idx dimIncludes() const { return _includes.dim(); }
        virtual const sUsrLoadingType * getInclude(idx iinc) const { return iinc >= 0 && iinc < dimIncludes() ? _types[_includes[iinc]] : 0; }
        virtual idx getIncludes(sVec<const sUsrType2*> & out) const;


        virtual idx dimFields(const sUsr & user) const
        {
            if( !_is_fetched ) {
                load(user, 0, 0, &_id);
            }
            return _fields.dim();
        }
        virtual idx dimRootFields(const sUsr & user) const
        {
            if( !_is_fetched ) {
                load(user, 0, 0, &_id);
            }
            return _root_ifields.dim();
        }
        virtual const sUsrLoadingTypeField* getField(const sUsr & user, idx ifield) const
        {
            if( !_is_fetched ) {
                load(user, 0, 0, &_id);
            }
            return _fields.ptr(ifield);
        }
        virtual sUsrTypeField::EType getFieldType(const sUsr & user, idx ifield) const
        {
            if( !_is_fetched ) {
                load(user, 0, 0, &_id);
            }
            return getField(user, ifield)->type();
        }
        virtual const sUsrLoadingTypeField* getRootField(const sUsr & user, idx irootfld) const
        {
            if( !_is_fetched ) {
                load(user, 0, 0, &_id);
            }
            return _fields.ptr(_root_ifields[irootfld]);
        }
        virtual const sUsrLoadingTypeField* getField(const sUsr & user, const char * field_name, idx field_name_len = 0) const;
        virtual sUsrTypeField::EType getFieldType(const sUsr & user, const char * field_name, idx field_name_len = 0) const
        {
            const sUsrLoadingTypeField * fld = getField(user, field_name, field_name_len);
            return fld ? fld->type() : sUsrTypeField::eInvalid;
        }

        virtual idx dimActions(const sUsr & user) const;
        virtual const sUsrLoadingAction * getAction(const sUsr & user, const char * act_name, idx act_name_len = 0) const;
        virtual const sUsrLoadingAction * getAction(const sUsr & user, idx iact) const;
        virtual idx getActions(const sUsr & user, sVec<const sUsrAction *> & out) const;

        virtual idx dimJSComponents(const sUsr & user) const;
        virtual const sUsrLoadingJSComponent * getJSComponent(const sUsr & user, const char * name, idx name_len = 0) const;
        virtual const sUsrLoadingJSComponent * getJSComponent(const sUsr & user, idx iact) const;
        virtual idx getJSComponents(const sUsr & user, sVec<const sUsrJSComponent *> & out) const;

        virtual idx dimViews(const sUsr & user) const;
        virtual const sUsrLoadingView * getView(const sUsr & user, const char * view_name, idx view_name_len = 0) const;
        virtual const sUsrLoadingView * getView(const sUsr & user, idx iview) const;
        virtual idx getViews(const sUsr & user, sVec<const sUsrView *> & out) const;

    private:
        sHiveId _id;
        idx _itype;
        idx _pos_name;
        idx _pos_title;
        idx _pos_description;
        idx _created;
        idx _modified;
        bool _is_virtual;
        mutable enum ELazy {
            eLazyNotLoaded,
            eLazyTrue,
            eLazyFalse
        } _is_user, _is_system;
        bool _is_prefetch;
        bool _is_fetched;
        ESingleton _is_singleton;
        mutable bool _is_broken;
        bool _is_ephemeral;

        sLst<idx> _parents;
        sLst<idx> _children;
        sLst<idx> _includes;

        static sVec<sUsrLoadingType*> _roots;

        sVec<sUsrLoadingTypeField> _fields;
        sVec<idx> _child_ifields;
        sVec<idx> _root_ifields;
        sDic<idx> _name2ifield;
        idx _dim_explicit_fields;
        idx _dim_inherited_fields;
        idx _dim_included_fields;

        struct PerUserRes {
            sDic<idx> _usr2ticlevel;
            sTic<idx> _name2ires;

            bool ensureUserForWriting(udx user_id);
            idx dimForUser(udx user_id) const;
            idx getILevel(udx user_id) const;
            idx getIRes(udx user_id, const char * name, idx name_len) const;
            idx getIRes(udx user_id, idx index) const;
        } _actions, _views, _jscos;

        static sStr _name_buf, _err_buf;
        static sMex _deps_buf;
        static class sUsrLoadingTypeList : public sVec<sUsrLoadingType*> {
            public:
                sUsrLoadingTypeList() {}
                ~sUsrLoadingTypeList() { for(idx i = 0; i < dim(); i++) { delete *ptr(i); } }
        } _types;
        static sDic<idx> _name_or_id2itype, _name_or_id2ephemeral_itype;

        static bool _all_deps_loaded;
        static bool _all_prefetched;

        sUsrLoadingType(bool default_zero = false);

        static sUsrLoadingType * getRaw(const char * type_name, idx type_name_len = 0);
        static sUsrLoadingType * getRaw(const sHiveId & type_id);
        static sUsrLoadingType * getRawEphemeral(const sHiveId & type_id);

        static sUsrLoadingType * load(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch = false, bool lazy_fetch_fields = false);
        static bool loadFromJSONCache(const sUsr & user);
        static bool ensureBinCache(const sUsr & user);
        static void loadFromObj(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch = false, bool lazy_fetch_fields = false);

        static sUsrLoadingType * ensureEphemeral(const sHiveId & type_id);

        class LoadFromObjContext;
        friend class LoadFromObjContext;

        class DepForest {
            public:
                DepForest() { ensureType2Inode(); }
                void add(sUsrLoadingType * utype)
                {
                    ensureType2Inode();
                    addWorker(utype);
                }
                bool has(idx itype) const
                {
                    return itype >= 0 && itype < _itype2node.dim() && _itype2node[itype] >= 0;
                }
                idx makeTraversal(sVec<idx> & out);

            private:
                sMex _mex;
                sVec< sKnot<udx> > _nodes;
                sVec<idx> _roots;
                sVec<idx> _itype2node;

                idx addWorker(const sUsrLoadingType * utype);
                void makeTreeTraversal(sVec<idx> & out, idx inode, sVec<bool> & visited);
                void ensureType2Inode()
                {
                    if( !_itype2node.dim() ) {
                        _itype2node.resize(sUsrLoadingType::_types.dim());
                        for( idx itype=0; itype<_itype2node.dim(); itype++ ) {
                            _itype2node[itype] = -sIdxMax;
                        }
                    }
                }
        };
        struct SetFieldChildrenParam;
        friend class DepForest;
        static void linkFields(sVec<idx> & itypes);
        static void loadActionsViewsFinder(const sUsrType2 * utype, const sUsrType2 * recurse_start, idx depth_from_start, void * param);
        static void loadActions(const sUsr & user);
        static void loadJSComponents(const sUsr & user);
        static void loadViews(const sUsr & user);
        void inheritField(const sUsrLoadingTypeField * inh_fld, sDic<idx> & overridden, sStr & case_buf);
        idx ensureArrayFieldRow(sUsrLoadingTypeField * fld, sStr & name_buf, sStr & case_buf);
        void setFieldChildren(idx ifld, idx iroot_fld, SetFieldChildrenParam * param, sStr & case_buf);
        void recurseField(idx ifld, sDic<idx> & seen_dic);
        void recurseDescendents(sDic<idx> & seen_dic) const;

        class JSONLoader;
        friend class JSONLoader;

        friend class sUsrLoadingTypeField;
        friend class sUsrType2;
        friend class sUsrTypeCache;
};

#endif 