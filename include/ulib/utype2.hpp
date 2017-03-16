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
#ifndef sLib_usrtype2_h
#define sLib_usrtype2_h

#include <slib/utils/tic.hpp>
#include <ulib/usr.hpp>

namespace slib {
    class sUsrType2;

    class sUsrTypeField {
        public:
            // Keep this in sync with propTypeNames in utype2.cpp
            enum EType {
                eInvalid = -1,
                eString = 0,
                eText,
                eInteger,
                eReal,
                eBool,
                eArray,
                eList,
                eDate,
                eTime,
                eDateTime,
                eUrl,
                eObj,
                ePassword,
                eFile
            };
            enum EReadOnly {
                eReadWrite = 0,
                eWriteOnce = -1,
                eSubmitOnce = -2,
                eReadOnly = 1,
                eReadOnlyAutofill = 2
            };
            const char * name() const { return getString(_pos_name); }
            const char * originalName() const { return getString(_pos_orig_name); }
            const char * title() const { return getString(_pos_title); }
            EType type() const { return _type; }
            const char * typeName() const;
            const sUsrTypeField* parent() const;
            const char * role() const { return getString(_pos_role); }
            idx dimChildren() const { return _dim_children; }
            //! get child field in range [0, dimChildren() - 1]; child fields are guaranteed to be sorted by order()
            const sUsrTypeField * getChild(idx ichild) const;
            idx getChildren(sVec<const sUsrTypeField*> &out) const;
            bool isKey() const { return _is_key; }
            EReadOnly readonly() const { return _readonly; }
            bool isOptional() const { return _is_optional; }
            bool isMulti() const { return _is_multi; }
            bool isHidden() const { return _is_hidden; }
            bool isSummary() const { return _is_summary; }
            bool isVirtual() const { return _is_virtual; }
            bool isBatch() const { return _is_batch; }
            //! contains data which must not be exposed to web UI, but can be used on the backend
            bool isSysInternal() const { return _is_sysinternal; }
            const char * brief() const { return getString(_pos_brief); }
            real order() const { return strtod(getString(_pos_order), 0); }
            //! comparison function for sorting : by owner sUsrType2, then order(), then name()
            int cmp(const sUsrTypeField * rhs) const;
            const char * orderString() const { return getString(_pos_order); }
            const char * defaultValue() const { return getString(_pos_default_value); }
            idx defaultEncoding() const { return _default_encoding; }
            const char * constraint() const { return getString(_pos_constraint); }
            const char * constraintData() const { return getString(_pos_constraint_data); }
            const char * constraintDescription() const { return getString(_pos_constraint_description); }
            const char * description() const { return getString(_pos_description); }
            const char * linkUrl() const { return getString(_pos_link_url); }

            bool canHaveValue() const { return _type != eInvalid && _type != eArray && _type != eList; }
            //! value can be be modified by sUsrObj::propSet() and similar mechanisms at the C++ level
            bool canSetValue() const;
            bool isArrayRow() const { return _is_array_row; }
            bool isGlobalMulti() const { return _is_global_multi; }
            bool isFlattenedDecor() const;
            bool isFlattenedMulti() const;
            const sUsrTypeField* flattenedParent() const;
            idx ancestorCount() const { return _ancestor_count; }

            const sUsrType2 * ownerType() const;
            const sUsrType2 * definerType() const;
            const sUsrType2 * includedFromType() const;

            enum eUsrTypeFieldParseFlags
            {
                fPasswordErr = 1,
                fPasswordHide = 1<<1
            };
            static bool parseValue(sVariant & out, EType type, const char * value, idx flags=0);
            bool parseValue(sVariant & out, const char * value, idx flags=0) const { return parseValue(out, type(), value, flags); }

            void printJSON(sJSONPrinter & printer, bool recurse = true, bool into_object = false) const;

        private:
            sUsrTypeField(bool default_zero = false);
            const sUsrTypeField & operator=(const sUsrTypeField & rhs);
            idx _index;
            idx _pos_name, _pos_orig_name;
            idx _pos_title;
            EType _type;
            idx _pos_parent_name;
            idx _parent;
            idx _pos_role;
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

            friend class sUsrType2;
            static const char * getString(idx pos) { return pos >= 0 ? _name_buf.ptr(pos) : sStr::zero; }
            static idx setString(const char * s, bool canonicalize=false, bool allow_empty=false);
            static void replaceString(idx & pos, const char * find00, const char * replace00, sStr & buf);
    };

    class sUsrAction
    {
        public:
            static const sUsrAction * get(const sUsr & user, const char * act_name, udx type_id = 0);
            static const sUsrAction * get(const sUsr & user, const char * act_name, const char * type_name);

            const sHiveId & id() const { return _id; }
            const char * name() const;
            const char * title() const;
            real order() const;
            bool isObjAction() const;
            udx requiredPermission() const;
            //! comparison function for sorting : by order(), then name()
            int cmp(const sUsrAction * rhs) const;

            // bool apply(const sUsr & user, const sVar * form) const;

            // retrieve properties for action's underlying sUsrObj
            const sUsrObjRes & objRes() const
            {
                return _usr2actres.get(&_user_id, sizeof(_user_id))->res;
            }
            const sUsrObjRes::TObjProp & objProps() const
            {
                return *objRes().get(_id);
            }
            void printJSON(sJSONPrinter & printer, bool into_object = false) const;

        private:
            sHiveId _id;
            udx _user_id;
            idx _iaction; // in ActRes::acts for the user

            friend class sUsrType2;

            struct ActRes {
                sVec<sUsrAction> acts;
                sUsrObjRes res;
            };

            static sDic<ActRes> _usr2actres;
            static sStr _name_buf;
    };

    class sUsrView
    {
        public:
            static const sUsrView * get(const sUsr & user, const char * view_name, udx type_id = 0);
            static const sUsrView * get(const sUsr & user, const char * view_name, const char * type_name);

            const sHiveId & id() const { return _id; }
            const char * name() const;

            idx dimFields() const;
            const char * fieldName(idx ifield) const;
            const char * fieldDefaultValue(idx ifield) const;
            idx fieldOrder(idx ifield) const;
            bool fieldReadonly(idx ifield) const;

            //! comparison function for sorting : by name()
            int cmp(const sUsrView * rhs) const;

            // retrieve properties for view's underlying sUsrObj
            const sUsrObjRes & objRes() const
            {
                return _usr2viewres.get(&_user_id, sizeof(_user_id))->res;
            }
            const sUsrObjRes::TObjProp & objProps() const
            {
                return *objRes().get(_id);
            }

        private:
            sHiveId _id;
            udx _user_id;
            idx _iview; // in ViewRes::views for the user

            friend class sUsrType2;

            struct ViewRes {
                sVec<sUsrView> views;
                sUsrObjRes res;
            };
            struct Field {
                idx pos_name;
                idx pos_default_value;
                idx order;
                bool readonly;
            };
            sLst<Field> _fields;

            static sMex _fields_buf;
            static sDic<ViewRes> _usr2viewres;
            static sDic<bool> _names;
            static sUsrType2 * _view_type;
    };

    class sUsrType2
    {
        public:
            /* constructors, initializers */
            static const udx type_type_domain_id;

            //! get type with given name, loading it from DB if necessary
            /*! \param no_prefetch if true, load only the specified type from DB - do not preload other automatically preloadable types
                \param lazy_fetch_fields if true, load only the name/title/description and hierarchy info from DB - do not fetch the type's fields until required */
            static const sUsrType2 * ensure(const sUsr & user, const char * type_name, idx type_name_len = 0, bool no_prefetch = false, bool lazy_fetch_fields = false)
            {
                const sUsrType2 * ret = getRaw(type_name, type_name_len);
                return ret && (lazy_fetch_fields || ret->_is_fetched) ? ret : load(user, type_name, type_name_len, 0, no_prefetch);
            }
            //! get type with given ID, loading it from DB if necessary
            /*! \param no_prefetch if true, load only the specified type from DB - do not preload other automatically preloadable types
                \param lazy_fetch_fields if true, load only the name/title/description and hierarchy info from DB - do not fetch the type's fields until required */
            static const sUsrType2 * ensure(const sUsr & user, const sHiveId & type_id, bool no_prefetch = false, bool lazy_fetch_fields = false)
            {
                const sUsrType2 * ret = getRaw(type_id);
                return ret && (lazy_fetch_fields || ret->_is_fetched) ? ret : load(user, 0, 0, &type_id, no_prefetch);
            }

            //! get type with given name. If it has not been loaded already, will return 0.
            static const sUsrType2 * get(const char * type_name, idx type_name_len = 0) { return getRaw(type_name, type_name_len); }
            //! get type with given ID. If it has not been loaded already, will return 0.
            static const sUsrType2 * get(const sHiveId & type_id) { return getRaw(type_id); }

            typedef void (*FindCallback)(const sUsrType2 * utype, const sUsrType2 * recurse_start, idx depth_from_start, void * param);

            //! retrieve unique types with names matching any of a comma-delimeted list of regexps; "regexp+" retrieves types whose names match regexp - and all their descendents; "!regexp" negates
            /*! \param user user under whose privelege to search
                \param[out] out vector in which to retrieve found types, or 0 to not use (for callback-based retrieval)
                \param qry comma-separated list of regexps, each optionally prefixed with "!" for negate or postfixed with "+" for recurse;
                    "R1,!R2,R3,!R4+" means "(match R1 or match R3) and not (match R2 or match R4 or has ancestor matching R4)";
                    a query containing only negations is taken relative to all types descending from base_user_type;
                    a NULL qry retrieves all types descending from base_user_type
                \param cb callback to run on each matching type, or 0 to not use
                \param cb_param parameter for the callback, or 0 to not use
                \param manual_ensure if true, do not load any additional type information from the database - the caller must guarantee that all types worth querying must have already been loaded using ensure()
                \param lazy_fetch_fields if true, load only the name/title/description and hierarchy info from DB - do not fetch the type's fields until required
                \returns number of matching types found */
            static idx find(const sUsr & user, sVec<const sUsrType2 *> * out, const char * qry, FindCallback cb = 0, void * cb_param = 0, bool manual_ensure = false, bool lazy_fetch_fields = false);

            //! retrieve all types that don't have a parent type; no specific order is guaranteed
            static idx getRootTypes(const sUsr & user, sVec<const sUsrType2 *> & out);

            /* basic info */

            const sHiveId & id() const { return _id; }
            const char * name() const { return _pos_name >= 0 ? _name_buf.ptr(_pos_name) : 0; }
            const char * title() const { return _pos_title >= 0 ? _name_buf.ptr(_pos_title) : 0; }
            const char * description() const { return _pos_description >= 0 ? _name_buf.ptr(_pos_description) : 0; }
            bool isVirtual() const { return _is_virtual; }
            //! check if the type is a descendent of base user type
            bool isUser() const;
            //! check if the type is a descendent of base system type
            bool isSystem() const;

            //! check if type's name matches any of a comma-delimeted list of regexps; "regexp+" will check for a match of the name of the type or any ancestor type; "!regexp" negates
            /*! \param qry comma-separated list of regexps, each optionally prefixed with "!" for negate or postfixed with "+" for check ancestors;
                     "R1,!R2,R3,!R4+" means "(match R1 or match R3) and not (match R2 or match R4 or has ancestor matching R4)";
                     a NULL qry is invalid (always returns false)
                \returns true if the name of the type (or of its ancestors, in the case of "+" query) matches */
            bool nameMatch(const char * qry) const;

            /* hierarchy */

            idx dimParents() const { return _parents.dim(); }
            const sUsrType2 * getParent(idx iparent) const { return iparent >= 0 && iparent < dimParents() ? _types[_parents[iparent]] : 0; }
            idx getParents(sVec<const sUsrType2*> & out) const;

            idx dimChildren() const { return _children.dim(); }
            const sUsrType2 * getChild(idx ichild) const { return ichild >= 0 && ichild < dimChildren() ? _types[_children[ichild]] : 0; }
            idx getChildren(sVec<const sUsrType2*> & out) const;

            idx dimIncludes() const { return _includes.dim(); }
            const sUsrType2 * getInclude(idx iinc) const { return iinc >= 0 && iinc < dimIncludes() ? _types[_includes[iinc]] : 0; }
            idx getIncludes(sVec<const sUsrType2*> & out) const;

            bool isDescendentOf(const sUsrType2 * rhs) const;
            bool isDescendentOf(const char * rhs_name) const { return isDescendentOf(get(rhs_name)); }

            /* fields */

            //! number of fields
            idx dimFields(const sUsr & user) const
            {
                if( !_is_fetched ) {
                    load(user, 0, 0, &_id);
                }
                return _fields.dim();
            }
            //! number of root fields (non-broken fields having no parents)
            idx dimRootFields(const sUsr & user) const
            {
                if( !_is_fetched ) {
                    load(user, 0, 0, &_id);
                }
                return _root_ifields.dim();
            }
            //! retrieve a field by its numeric index in range [0 .. dimFields() - 1]
            const sUsrTypeField* getField(const sUsr & user, idx ifield) const
            {
                if( !_is_fetched ) {
                    load(user, 0, 0, &_id);
                }
                return _fields.ptr(ifield);
            }
            //! retrieve field's data type
            sUsrTypeField::EType getFieldType(const sUsr & user, idx ifield) const
            {
                if( !_is_fetched ) {
                    load(user, 0, 0, &_id);
                }
                return getField(user, ifield)->type();
            }
            //! retrieve a root field by its numeric index in range [0 .. dimRootFields() - 1]
            const sUsrTypeField* getRootField(const sUsr & user, idx irootfld) const
            {
                if( !_is_fetched ) {
                    load(user, 0, 0, &_id);
                }
                return _fields.ptr(_root_ifields[irootfld]);
            }
            //! retrieve a field by name
            const sUsrTypeField* getField(const sUsr & user, const char * field_name, idx field_name_len = 0) const;
            //! retrieve field's data type by name
            sUsrTypeField::EType getFieldType(const sUsr & user, const char * field_name, idx field_name_len = 0) const
            {
                const sUsrTypeField * fld = getField(user, field_name, field_name_len);
                return fld ? fld->type() : sUsrTypeField::eInvalid;
            }
            //! retrieve all fields
            idx getFields(const sUsr & user, sVec<const sUsrTypeField*> & out) const;
            //! query for fields by name
            idx findFields(const sUsr & user, sVec<const sUsrTypeField*> & out, const char * filter00 = 0) const;

            /* actions */
            idx dimActions(const sUsr & user) const;
            const sUsrAction * getAction(const sUsr & user, const char * act_name, idx act_name_len = 0) const;
            const sUsrAction * getAction(const sUsr & user, idx iact) const;
            idx getActions(const sUsr & user, sVec<const sUsrAction *> & out) const;

            /* views */
            idx dimViews(const sUsr & user) const;
            const sUsrView * getView(const sUsr & user, const char * view_name, idx view_name_len = 0) const;
            const sUsrView * getView(const sUsr & user, idx iview) const;
            idx getViews(const sUsr & user, sVec<const sUsrView *> & out) const;

            /* output */

            idx props(const sUsr & user, sVarSet & list, const char * fiter00 = 0) const;
            void printJSON(const sUsr & user, sJSONPrinter & printer, bool into_object = false) const;

            static const char * errors() { return _err_buf.ptr(); }

            //! add definitions of types from a JSON buffer
            static bool loadFromJSON(const sUsr & user, const char * buf, idx buf_len = 0);

        private:
            sHiveId _id;
            idx _itype; // index of self in _types
            idx _pos_name; // index of (name -> itype) in _name_or_id2itype
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
            bool _is_broken;

            sLst<idx> _parents; // itypes of parents, stored in _deps_buf
            sLst<idx> _children; // itypes of children, stored in _deps_buf
            sLst<idx> _includes; // itypes of includes, stored in _deps_buf

            sVec<sUsrTypeField> _fields;
            sVec<idx> _child_ifields;
            sVec<idx> _root_ifields;
            sDic<idx> _name2ifield;
            idx _dim_explicit_fields;
            idx _dim_inherited_fields;
            idx _dim_included_fields;

            struct PerUserRes {
                sDic<idx> _usr2ticlevel; // map from user id to level in _name2ires
                sTic<idx> _name2ires; // tic: at _usr2ticlevel level, map from name to index in the ActRes/ViewRes for the user

                bool ensureUserForWriting(udx user_id);
                idx dimForUser(udx user_id) const;
                idx getILevel(udx user_id) const;
                idx getIRes(udx user_id, const char * name, idx name_len) const;
                idx getIRes(udx user_id, idx index) const;
            } _actions, _views;

            static sStr _name_buf, _err_buf;
            static sMex _deps_buf;
            static class sUsrType2List : public sVec<sUsrType2*> {
                public:
                    sUsrType2List() {}
                    ~sUsrType2List() { for(idx i = 0; i < dim(); i++) { delete *ptr(i); } }
            } _types;
            static sDic<idx> _name_or_id2itype; // map name OR hive id -> index into _types
            static sVec<sUsrType2*> _roots;

            static bool _all_deps_loaded; // are all type names & dependencies loaded, or only a subset?
            static bool _all_prefetched; // are all types marked "prefetch" loaded?

            sUsrType2(bool default_zero = false);

            static sUsrType2 * getRaw(const char * type_name, idx type_name_len = 0);
            static sUsrType2 * getRaw(const sHiveId & type_id);
            static idx findRaw(const sUsr * ensurer_usr, sVec<const sUsrType2 *> * out, const char * qry, FindCallback cb = 0, void * cb_param = 0, bool no_regexp = false, bool lazy_fetch_fields = false);

            static sUsrType2 * load(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch = false, bool lazy_fetch_fields = false);
            static bool loadFromJSONCache(const sUsr & user);
            static void loadFromObj(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch = false, bool lazy_fetch_fields = false);
            class LoadFromObjContext;
            friend class LoadFromObjContext;
            static void loadDeps(const sUsr & user);

            class DepForest;
            struct SetFieldChildrenParam;
            friend class DepForest;
            static void loadFields(const sUsr & db, DepForest * type_forest);
            static void linkFields(sVec<idx> & itypes);
            static void loadActionsViewsFinder(const sUsrType2 * utype, const sUsrType2 * recurse_start, idx depth_from_start, void * param);
            static void loadActions(const sUsr & user);
            static void loadViews(const sUsr & user);
            void inheritField(const sUsrTypeField * inh_fld, sDic<idx> & overridden, sStr & case_buf);
            idx ensureArrayFieldRow(sUsrTypeField * fld, sStr & name_buf, sStr & case_buf);
            void setFieldChildren(idx ifld, idx iroot_fld, SetFieldChildrenParam * param, sStr & case_buf);
            void recurseField(idx ifld, sDic<idx> & seen_dic);
            //! collect itype of this type and all descendents
            void recurseDescendents(sDic<idx> & seen_dic) const;

            class FindWorker;
            friend class FindWorker;
            class NameMatchWorker;
            friend class NameMatchWorker;
            class JSONLoader;
            friend class JSONLoader;

            friend class sUsrTypeField;
    };
};
#endif
