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
                eFile,

                eUnsigned,
                eMemory,
                eVersion,
                eBlob,
                eJSON,
                eXML,
                eArrayTab,
                eListTab
            };
            enum EReadOnly {
                eReadWrite = 0,
                eWriteOnce = -1,
                eSubmitOnce = -2,
                eReadOnly = 1,
                eReadOnlyAutofill = 2
            };
            enum ERole {
                eRole_unknown = -1,
                eRole_input,
                eRole_output,
                eRole_parameter,
                eRole_state
            };

            virtual ~sUsrTypeField() {}

            virtual const char * name() const = 0;
            virtual const char * originalName() const = 0;
            virtual const char * title() const = 0;
            virtual EType type() const = 0;
            const char * typeName() const;
            const char * typeName2() const;
            virtual const sUsrTypeField* parent() const = 0;
            virtual ERole role() const = 0;
            virtual idx dimChildren() const = 0;
            virtual const sUsrTypeField * getChild(idx ichild) const = 0;
            virtual idx getChildren(sVec<const sUsrTypeField*> &out) const = 0;
            virtual bool isKey() const = 0;
            virtual EReadOnly readonly() const = 0;
            virtual bool isOptional() const = 0;
            virtual bool isMulti() const = 0;
            virtual bool isHidden() const = 0;
            virtual bool isSummary() const = 0;
            virtual bool isVirtual() const = 0;
            virtual bool isBatch() const = 0;
            virtual bool isWeakReference() const = 0;
            virtual bool isSysInternal() const = 0;
            virtual const char * brief() const = 0;
            virtual real order() const = 0;
            int cmp(const sUsrTypeField * rhs) const;
            virtual const char * orderString() const = 0;
            virtual const char * defaultValue() const = 0;
            virtual idx defaultEncoding() const = 0;
            virtual const char * constraint() const = 0;
            virtual const char * constraintData() const = 0;
            virtual const char * constraintDescription() const = 0;
            virtual const char * description() const = 0;
            virtual const char * linkUrl() const = 0;

            bool canHaveValue() const
            {
                EType t = type();
                return t != eInvalid && t != eArray && t != eList && t != eArrayTab && t != eListTab;
            }
            virtual bool canSetValue() const = 0;
            virtual bool isArrayRow() const = 0;
            virtual bool isGlobalMulti() const = 0;
            virtual bool isFlattenedDecor() const = 0;
            virtual bool isFlattenedMulti() const = 0;
            const sUsrTypeField* flattenedParent() const;
            virtual idx ancestorCount() const = 0;

            virtual const sUsrType2 * ownerType() const = 0;
            virtual const sUsrType2 * definerType() const = 0;
            virtual const sUsrType2 * includedFromType() const = 0;

            enum eUsrTypeFieldParseFlags
            {
                fPasswordErr = 1,
                fPasswordHide = 1<<1
            };
            static bool parseValue(sVariant & out, EType type, const char * value, idx flags=0);
            bool parseValue(sVariant & out, const char * value, idx flags=0) const { return parseValue(out, type(), value, flags); }

            virtual void printJSON(sJSONPrinter & printer, bool recurse = true, bool into_object = false) const;
            virtual void printJSON2(sJSONPrinter & printer, bool recurse = true, bool into_object = false) const;
    };

    class sUsrAction
    {
        public:
            static const sUsrAction * get(const sUsr & user, const char * act_name, udx type_id = 0);
            static const sUsrAction * get(const sUsr & user, const char * act_name, const char * type_name);

            virtual const sHiveId & id() const = 0;
            virtual const char * name() const = 0;
            virtual const char * title() const = 0;
            virtual const char * description() const = 0;
            virtual real order() const = 0;
            virtual const char * orderString() const = 0;
            virtual bool isObjAction() const = 0;
            virtual udx requiredPermission() const = 0;
            int cmp(const sUsrAction * rhs) const;


            void printJSON(sJSONPrinter & printer, bool into_object = false) const;

            virtual ~sUsrAction() {}
    };

    class sUsrView
    {
        public:
            static const sUsrView * get(const sUsr & user, const char * view_name, udx type_id = 0);
            static const sUsrView * get(const sUsr & user, const char * view_name, const char * type_name);

            virtual const sHiveId & id() const= 0;
            virtual const char * name() const = 0;

            virtual idx dimFields() const = 0;
            virtual const char * fieldName(idx ifield) const = 0;
            virtual const char * fieldDefaultValue(idx ifield) const = 0;
            virtual real fieldOrder(idx ifield) const = 0;
            virtual const char * fieldOrderString(idx ifield) const = 0;
            virtual bool fieldReadonly(idx ifield) const = 0;

            int cmp(const sUsrView * rhs) const;

            virtual ~sUsrView() {}
    };
    class sUsrJSComponent
    {
        public:
            static const sUsrJSComponent * get(const sUsr & user, const char * name, udx type_id = 0);
            static const sUsrJSComponent * get(const sUsr & user, const char * name, const char * type_name);

            virtual const sHiveId & id() const = 0;
            virtual const char * name() const = 0;
            virtual bool isPreview() const = 0;
            virtual bool isAlgoview() const = 0;
            virtual real order() const = 0;
            virtual const char * orderString() const = 0;
            int cmp(const sUsrJSComponent * rhs) const;


            void printJSON(sJSONPrinter & printer, bool into_object = false) const;

            virtual ~sUsrJSComponent() {}
    };


    class sUsrLoadingAction;
    class sUsrLoadingJSComponent;
    class sUsrLoadingView;
    class sUsrLoadingType;
    class sUsrLoadingTypeField;
    class sUsrTypeCache;

    class sUsrType2
    {
        public:
            static const udx type_type_domain_id;

            virtual ~sUsrType2() {}

            static const sUsrType2 * ensure(const sUsr & user, const char * type_name, idx type_name_len = 0, bool no_prefetch = false, bool lazy_fetch_fields = false);
            static const sUsrType2 * ensure(const sUsr & user, const sHiveId & type_id, bool no_prefetch = false, bool lazy_fetch_fields = false, bool ephemeral = false);
            static const sUsrType2 * ensureTypeType(const sUsr & user);

            static const sUsrType2 * get(const char * type_name, idx type_name_len = 0);
            static const sUsrType2 * get(const sHiveId & type_id);
            static const sUsrType2 * getTypeType();

            typedef void (*FindCallback)(const sUsrType2 * utype, const sUsrType2 * recurse_start, idx depth_from_start, void * param);

            static idx find(const sUsr & user, sVec<const sUsrType2 *> * out, const char * qry, FindCallback cb = 0, void * cb_param = 0, bool manual_ensure = false, bool lazy_fetch_fields = false);

            static idx getRootTypes(const sUsr & user, sVec<const sUsrType2 *> & out);


            virtual const sHiveId & id() const = 0;
            virtual const char * name() const = 0;
            virtual const char * title() const = 0;
            virtual const char * description() const = 0;
            virtual bool isVirtual() const = 0;
            virtual bool isPrefetch() const = 0;
            virtual bool isUser() const = 0;
            virtual bool isSystem() const = 0;

            enum ESingleton {
                eNotSingleton,
                eSingletonPerUser,
                eSingletonPerSystem
            };
            virtual ESingleton isSingleton() const = 0;

            bool nameMatch(const char * qry) const;


            virtual idx dimParents() const = 0;
            virtual const sUsrType2 * getParent(idx iparent) const = 0;
            virtual idx getParents(sVec<const sUsrType2*> & out) const = 0;

            virtual idx dimChildren() const = 0;
            virtual const sUsrType2 * getChild(idx ichild) const = 0;
            virtual idx getChildren(sVec<const sUsrType2*> & out) const = 0;

            virtual idx dimIncludes() const = 0;
            virtual const sUsrType2 * getInclude(idx iinc) const = 0;
            virtual idx getIncludes(sVec<const sUsrType2*> & out) const = 0;

            bool isDescendentOf(const sUsrType2 * rhs) const;
            bool isDescendentOf(const char * rhs_name) const { return isDescendentOf(get(rhs_name)); }


            virtual idx dimFields(const sUsr & user) const = 0;
            virtual idx dimRootFields(const sUsr & user) const = 0;
            virtual const sUsrTypeField* getField(const sUsr & user, idx ifield) const = 0;
            sUsrTypeField::EType getFieldType(const sUsr & user, idx ifield) const
            {
                const sUsrTypeField * fld = getField(user, ifield);
                return fld ? fld->type() : sUsrTypeField::eInvalid;
            }
            virtual const sUsrTypeField* getRootField(const sUsr & user, idx irootfld) const = 0;
            virtual const sUsrTypeField* getField(const sUsr & user, const char * field_name, idx field_name_len = 0) const = 0;
            sUsrTypeField::EType getFieldType(const sUsr & user, const char * field_name, idx field_name_len = 0) const
            {
                const sUsrTypeField * fld = getField(user, field_name, field_name_len);
                return fld ? fld->type() : sUsrTypeField::eInvalid;
            }
            idx getFields(const sUsr & user, sVec<const sUsrTypeField*> & out) const;
            idx findFields(const sUsr & user, sVec<const sUsrTypeField*> & out, const char * filter00 = 0) const;

            virtual idx dimActions(const sUsr & user) const = 0;
            virtual const sUsrAction * getAction(const sUsr & user, const char * act_name, idx act_name_len = 0) const = 0;
            virtual const sUsrAction * getAction(const sUsr & user, idx iact) const = 0;
            virtual idx getActions(const sUsr & user, sVec<const sUsrAction *> & out) const = 0;

            virtual idx dimJSComponents(const sUsr & user) const = 0;
            virtual const sUsrJSComponent * getJSComponent(const sUsr & user, const char * name, idx name_len = 0) const = 0;
            virtual const sUsrJSComponent * getJSComponent(const sUsr & user, idx ijsco) const = 0;
            virtual idx getJSComponents(const sUsr & user, sVec<const sUsrJSComponent *> & out) const = 0;

            virtual idx dimViews(const sUsr & user) const = 0;
            virtual const sUsrView * getView(const sUsr & user, const char * view_name, idx view_name_len = 0) const = 0;
            virtual const sUsrView * getView(const sUsr & user, idx iview) const = 0;
            virtual idx getViews(const sUsr & user, sVec<const sUsrView *> & out) const = 0;


            idx props(const sUsr & user, sVarSet & list, const char * fiter00 = 0) const;
            void printJSON(const sUsr & user, sJSONPrinter & printer, bool into_object = false) const;
            void printJSON2(const sUsr & user, sJSONPrinter & printer, bool into_object = false) const;

            static bool loadFromJSON(const sUsr & user, const char * buf, idx buf_len = 0);

        protected:
            static idx dimRaw();
            static const sUsrType2 * getNthRaw(idx itype);
            static idx findRaw(const sUsr * ensurer_usr, sVec<const sUsrType2 *> * out, const char * qry, FindCallback cb = 0, void * cb_param = 0, bool no_regexp = false, bool lazy_fetch_fields = false);

            static sUsrType2 * load(const sUsr & user, const char * name, idx name_len, const sHiveId * type_id, bool no_prefetch = false, bool lazy_fetch_fields = false);
            static const sUsrTypeCache * getBinCache(const sUsr * user);

            class FindWorker;
            class NameMatchWorker;

            friend class slib::sUsrLoadingTypeField;
            friend class slib::sUsrLoadingType;
    };
};
#endif
