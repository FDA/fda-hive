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
#error "Deprecated, switch to sUsrType2"
#pragma once
#ifndef sLib_usrtype_h
#define sLib_usrtype_h

#include <ulib/uprop.hpp>
#include <ulib/usr.hpp>
#include <ulib/uobj.hpp>
#include <slib/core/def.hpp>
#include <memory>

namespace slib {

    typedef sDic<sVarSet> TActions;

    // Keep this in sync with propTypeNames in utype.cpp
    enum eUsrObjPropType
    {
        eUsrObjProp_invalid = -1,
        eUsrObjProp_string = 0,
        eUsrObjProp_text,
        eUsrObjProp_integer,
        eUsrObjProp_real,
        eUsrObjProp_bool,
        eUsrObjProp_array,
        eUsrObjProp_list,
        eUsrObjProp_date,
        eUsrObjProp_time,
        eUsrObjProp_datetime,
        eUsrObjProp_url,
        eUsrObjProp_obj,
        eUsrObjProp_password,
        eUsrObjProp_file
    };

    class sUsrObjTypeField {
    public:
        virtual ~sUsrObjTypeField() {}
        virtual const char * name() const = 0;
        virtual const char * title() const = 0;
        virtual eUsrObjPropType type() const = 0;
        virtual const char * typeName() const = 0;
        virtual const sUsrObjTypeField* parent() const = 0;
        virtual idx children(sVec<const sUsrObjTypeField*> &out) const = 0;
        virtual bool isKey() const = 0;
        virtual bool isReadonly() const = 0;
        virtual bool isWriteonce() const = 0;
        virtual bool isAutofill() const = 0;
        virtual bool isSubmitonce() const = 0;
        virtual bool isOptional() const = 0;
        virtual bool isMulti() const = 0;
        virtual bool isHidden() const = 0;
        virtual bool isSummary() const = 0;
        virtual bool isVirtual() const = 0;
        virtual bool isBatch() const = 0;
        virtual const char * brief() const = 0;
        virtual real order() const = 0;
        virtual const char * orderString() const = 0;
        virtual const char * defaultValue() const = 0;
        virtual const char * constraint() const = 0;
        virtual const char * constraintData() const = 0;
        virtual const char * constraintDescription() const = 0;
        virtual const char * description() const = 0;
        virtual const char * linkUrl() const = 0;

        virtual bool canHaveValue() const = 0;
        virtual bool flattenedDecor() const = 0;
        virtual bool flattenedMulti() const = 0;
        virtual const sUsrObjTypeField* flattenedParent() const = 0;
        virtual idx flattenedChildren(sVec<const sUsrObjTypeField*> &out) const = 0;
        virtual bool isArrayRow() const = 0;
        virtual bool globalMulti() const = 0;
        virtual idx ancestorCount() const = 0;
    };

    class sUsrObjType
    {
        public:
            sUsrObjType()
                : m_def(sNotIdx)
            {
            }
            sUsrObjType(const sUsr & usr, const char* type_name)
                : m_def(sNotIdx)
            {
                init(usr, type_name, sHiveId());
            }
            sUsrObjType(const sUsr & usr, const sHiveId & id)
                : m_def(sNotIdx)
            {
                init(usr, 0, id);
            }
            ~sUsrObjType()
            {
            }

            idx init(const sUsr& usr, const char * name, const sHiveId & id) const;

            //! For use in sVec and other containers
            void reset(const char* type_name)
            {
                m_def = sNotIdx;
            }

            const char* name() const;
            const char* Title() const;
            const sHiveId & id() const;

            // if view_name == 0 it returns ACTUAL TYPE DATA
            // default view is returned if view_name == ""
            // filter \0 separated and \0\0 terminated list of prop names
            // returns list dim()
            udx props(const sUsr & usr, sVarSet& list, const char* view_name = 0, const char* filter00 = 0) const;
            udx props(sVec<const sUsrObjTypeField*> &list) const;
            const sUsrObjPropsTree* propsTree(const sUsr & usr, const char* view_name = 0, const char * filter00 = 0) const;
            bool propsJSON(const sUsr & usr, sJSONPrinter & printer, bool into_object = false) const;
            const sUsrObjTypeField* prop(const char* prop_name) const;
            const TActions* actions(const sUsr& usr) const;

            eUsrObjPropType propType(const char* prop_name) const;

            bool isOk() const;
            bool isAbstract() const;
            //! check if the type is a descendent of base user type
            bool isUser() const;
            //! check if the type is a descendent of base system type
            bool isSystem() const;

            /*! \returns number of types in \a out, or 0 on failure */
            static idx getTree(const sUsr& usr, sVarSet& out_tbl, const char* root_name="base");

        private:

            idx m_def;
    };

    class sUsrType : public sUsrObj
    {
            typedef sUsrObj TParent;

        public:
            sUsrType()
            {
            }

            sUsrType(sUsr& usr, const sHiveId & objId)
                : sUsrObj(usr, objId)
            {
                m_id = objId;
            }

            virtual udx propGet(const char* prop, sVarSet& res, bool sort=false/* not used */) const;

            virtual const char * addFilePathname(sStr & buf, bool overwrite, const char* key, ...) const __attribute__((format(printf, 4, 5)))
            {
                return 0;
            }
            virtual bool delFilePathname(const char* key, ...) const __attribute__((format(printf, 2, 3)))
            {
                return false;
            }

        protected:
            bool onDelete(void)
            {
                return false;
            }
            bool onPurge(void)
            {
                return false;
            }

            void propBulk(const char * filter00, sVarSet& list) const;
            const char * getFilePathnameX(sStr & buf, const sStr & key) const
            {
                return 0;
            }
    };
}

#endif // sLib_usrtype_h
