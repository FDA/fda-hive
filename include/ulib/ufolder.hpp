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
#ifndef sLib_usrfolder_h
#define sLib_usrfolder_h

#include <ulib/uobj.hpp>

namespace slib {

    class sUsrFolder: public sUsrObj
    {
            typedef sUsrObj TParent;

        public:
            // this will CREATE in DB the whole path and establish instance at its end
            sUsrFolder(const sUsr& usr, const char * path);
            sUsrFolder(const sUsr& usr, const sHiveId & objId);
            //! safe constructor for use in propset in case of write-only permissions
            sUsrFolder(const sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission);

            typedef idx (*callbackType)(void * param, idx reqId, idx lazysecs, idx countDone, idx progressCur, idx percentMax);
            callbackType progress_CallbackFunction;
            void * progress_CallbackParamObject;
            void * progress_CallbackParamReqID;
            /**
             * returns list of object ids of given types which are not attached to any folder
             * type_names, prop, value - see sUsr::objs method
             */
            static idx orphans(const sUsr & usr, sVec<sHiveId> & ids, const char * type_names, const char* prop = 0, const char* value = 0);
            /*! get list of folders to which a particular object id is attached */
            static idx attachedTo(sVec<sHiveId> * out_folders, const sUsr & usr, const sHiveId & id);
            static sUsrFolder * find(const sUsr & usr, const char * path);


            sUsrFolder * createSubFolder(const char * fmt, ...) __attribute__((format(printf, 2, 3)));
            sUsrFolder * find (const char * path) const;
            /**
             *  Checks if 'obj' is neither a descendant of 'this' nor 'this' itself
             *  and sets property child of 'this' folder with the id of 'obj'. If 'obj'
             *  is a folder then it prevents from creating multiple links
             */
            bool attach(const sUsrObj & obj);
            bool detach(const sUsrObj & obj);

            udx attachCopy(sVec<sHiveId>* objIds, sUsrFolder* dst_obj, sStr * buf = 0, sVec<sHiveId>* cpied_ids = 0, bool checkIfIsChild = false);
            udx attachMove(sVec<sHiveId>* objIds, sUsrFolder* dst_obj, sStr * buf = 0, bool checkIfIsChild = false);

            bool actRemove(sVec<sHiveId>* src_ids, sStr * buf = 0, bool checkIfIsChild = false, bool forceDelete = false);

            bool isChild(const sUsrObj & obj) const;
            bool isChild(const sHiveId & Id) const;

            /**
             * returns true if there is a subfolder with this name
             */
            bool isSubFolder(const char* name) const;
            /**
             * returns true if there is an object with this Id somewhere in this subTree
             */
            bool isDescendant(const sHiveId & Id) const;
            /**
             * returns true if this obj is somewhere in this subTree
             */
            bool isDescendant(const sUsrObj & obj) const;
            /**
             * returns true if this folder in in Trash using isDescendant
             */
            bool isInTrash(void) const;
            /**
             * returns true if this folder in the Trash using isDescendant
             */
            bool isTrash(void) const;


            virtual udx propSet(const char* prop, const char** groups, const char** values, udx cntValues, bool isAppend = false, const udx * path_lens = 0, const udx * value_lens = 0);
            virtual udx propGet(const char* prop, sVarSet& res, bool sort=false) const;

            bool isReserved(void) const;

            const char* name(void) const
            {
                return TParent::propGet("name");
            }
            udx name(const char* name);


            bool fixChildrenPath();

        protected:
            virtual bool onDelete(void);
            static bool folder(sHiveId & out_id, const sUsr& usr, const char * path, bool doCreate, sUsrFolder * parentFolder = 0, const char * s_type = 0);

        private:

            static bool su(const sUsr& usr, const bool newval);
            // mutable because const methods can initialize it
            // maps child's sHiveId -> group
            mutable sDic<udx> m_children;
    };


    class sSysFolder: public sUsrFolder
    {
            typedef sUsrFolder TParent;

        public:

            sSysFolder(const sUsr& usr, const char * path);
            sSysFolder(const sUsr& usr, const sHiveId & objId)
                :sUsrFolder(usr,objId)
            {

            }
            //! safe constructor for use in propset in case of write-only permissions
            sSysFolder(const sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission)
                :sUsrFolder(usr, objId, ptypeId, permission)
            {
            }

            static sUsrFolder * find(const sUsr & usr, const char * path);
            static sUsrFolder * Home(const sUsr & usr, bool autoCreate = false);
            static sUsrFolder * Inbox(const sUsr & usr, bool autoCreate = false);
            static sUsrFolder * Trash(const sUsr & usr, bool autoCreate = false);

        protected:

            virtual bool onDelete(void)
            {
                return false;
            }

        private:
            static bool sysfolder(sHiveId & out_id, const sUsr& usr, const char * path, bool doCreate);
            // mutable because const methods can initialize it
            mutable sDic<sHiveId> m_children;
    };
}

#endif // sLib_usrfolder_h
