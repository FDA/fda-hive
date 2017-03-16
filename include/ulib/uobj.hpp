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
#ifndef sLib_usrobj_h
#define sLib_usrobj_h

#include <slib/std/string.hpp>
#include <ulib/uprop.hpp>
#include <ulib/usr.hpp>
#include <ulib/utype2.hpp>
#include <memory>

namespace slib {

    class sUsrObj
    {

        public:

            // default constructor so this object can be used in containers
            sUsrObj()
                :m_usr(*(sUsr*)(&sMex::_zero)) // this is a clear HACK: the only reason this is here beacause m_usr is a reference , not a pointer which can be safely put to zero for unitilized objects of this kind
            {
                is_auditable=true;
            }
            // creates new object of a given type
            sUsrObj(const sUsr& usr, const char* type_name)
                    : m_usr(usr)
            {
                m_usr.objCreate(m_id, type_name);
                is_auditable=true;
            }
            // access generic object by its id
            sUsrObj(const sUsr& usr, const sHiveId & objId)
                    : m_usr(usr), m_id(objId)
            {
                if( !m_usr.objGet(m_id) ) {
                    m_id.reset();
                }
                is_auditable=true;
            }
            //! safe constructor for use in propset in case of write-only permissions
            sUsrObj(const sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission)
                    : m_usr(usr), m_id(objId)
            {
                if( !m_usr.objGet(m_id, ptypeId, permission) ) {
                    m_id.reset();
                }
                is_auditable=true;
            }
            // destructor MUST be implemented in uobj.cpp to avoid memory leak
            virtual ~sUsrObj();

            const sHiveId & Id(void) const
            {
                return m_id;
            }
            const char * IdStr(sStr * buf = 0) const
            {
                static sStr lclBuf;
                if( !buf ) {
                    lclBuf.cut(0);
                    buf = &lclBuf;
                }
                Id().print(*buf);
                return buf->ptr();
            }

            //! check if object's type name matches a pattern (comma-separated list of regexps, optionally followed by '+' to check for child types or preceded by '!' for negate)
            /*! Examples:
                \code
                bool b1 = obj.isTypeOf("^exact_type_name$"); // check for exact type name
                bool b2 = obj.isTypeOf("^type_x$,^type_y$"); // check for multiple exact type names
                bool b3 = obj.isTypeOf("svc-"); // regexp-based check: type name has "svc-" substring
                bool b4 = obj.isTypeOf("^svc-align$+"); // is of type svc-align or a descendent type
                bool b5 = obj.isTypeOf("!^svc-align$+,!^u-file$+"); // is *not* of type svc-align or u-file or any descendent type
                bool b6 = obj.isTypeOf("^svc-align-hexagon$+,!^svc-align-hexagon$"); // is of type that descends from svc-align-hexagon, but is not svc-align-hexagon itself
                \endcode */
            bool isTypeOf(const char* pattern) const;

            const char* getTypeName(void) const;

            const sUsrType2 * getType(void) const;


            //! return path to a new file in object directory using key
            /*!
             * \param buf new file path is appended to buffer via printf
             * \param key if starts with '.' it is treated as extension, name is constructed
             * \param overwrite if true and file exists it is deleted and pointer is returned. if false and file exists returns 0
             * \return pointer within buf to a new file path name, 0 - see overwrite
             * \note doesn't actually creates the file if not exists
             */
            virtual const char * addFilePathname(sStr & buf, bool overwrite, const char* key, ...) const __attribute__((format(printf, 4, 5)));
            //! move file to a name prefixed with "._trash" (for implementing safe delete/overwrite of object files)
            /*! \param overwrite If a trash file with same name exists, overwrite it
                \returns pointer into buf with printed trash file name, or 0 on failure  */
            virtual char * trashFilePathname(sStr & buf, bool overwrite, const char* key, ...) __attribute__((format(printf, 4, 5)));
            //! move trashed file back to original name (for implementing safe delete/overwrite of object files)
            /*! \param overwrite If original non-trash file with same name exists, overwrite it
                \returns pointer into buf with printed restored file name, or 0 on failure */
            virtual char * restoreFilePathname(sStr & buf, bool overwrite, const char* key, ...) __attribute__((format(printf, 4, 5)));
            // unconditionally drops the file
            virtual bool delFilePathname(const char* key, ...) const __attribute__((format(printf, 2, 3)));
            //! return path to an existing file in object directory using key list
            /*! \param[out] buf buffer into which the path is printed
                \param key printf()-style format string; if the result starts with '.' it is treated as file
                           extension, and the file name is constructed
             *  \returns pointer to printed path if permissions are satisfied, the file exists and has non-zero size,
                         or 0 on failure */
            const char * getFilePathname(sStr & buf, const char* key, ...) const __attribute__((format(printf, 3, 4)));
            // finds first file by key
            const char * getFilePathname00(sStr & buf, const char* key00) const;
            //! Return path to a potential file in object directory without checking or ensuring for the file's existence on disk
            /*! \param[out] buf buffer into which the path is printed
                \param key printf()-style format string; if the result starts with '.' it is treated as file
                           extension, and the file name is constructed
             *  \returns pointer to printed path if object permissions are satisfied, or 0 on failure */
            const char * makeFilePathname(sStr & buf, const char* key, ...) const __attribute__((format(printf, 3, 4)));

            // lists all files in objects directory by mask
            udx files(sDir & fieList00, idx dirListFlags, const char * mask = "*", const char * relPath = 0 ) const;
            udx fileProp(sStr& dst, bool as_csv, const char * wildcard) const;
            udx fileProp(sJSONPrinter& dst, const char * wildcard, bool into_object) const;

        public:

            // global initialization, is done on top common level
            static void initStorage(const char * pathList, const udx default_min_free_space_per_volume);

            // action commands and related
            idx actions(sVec<sStr>& actions) const;


            virtual bool onCreate(void)
            { return true; }

            bool actExecute(void) const
            { return true; }

            bool actDelete(void);
            bool purge(void);
            void cleanup(void);

            sUsrObj* cast(const char* type_name);

            virtual udx propBulk(sVarSet & list, const char* view_name = 0, const char * filter00 = 0) const;
            virtual void propBulk(sVarSet & src, sVarSet & dst, const char* view_name, const char* filter00) const;
            virtual udx propBulk(sVar & form) const;
            const sUsrObjPropsTree * propsTree(const char* view_name = 0, const char * filter00 = 0, bool force_reload = false) const;

            // property must be defined in type and pass validation
            // multi value properties with row ids
            virtual udx propSet(const char* prop, const char** paths, const char** values, udx cntValues, bool isAppend = false, const udx * path_lens = 0, const udx * value_lens = 0);
            // groups will be auto generated
            udx propSet(const char* prop, const char** values, udx cntValues, bool isAppend = false, const udx * path_lens = 0, const udx * value_lens = 0)
            {
                return propSet(prop, 0, values, cntValues, isAppend, path_lens, value_lens);
            }
            udx propSet(const char* prop, sVec<const char*>& valuesVec, bool isAppend = false)
            {
                return propSet(prop, &valuesVec[0], valuesVec.dim(), isAppend);
            }
            // single value props only -> overwrite mode always!!!
            udx propSet(const char* prop, const char* value)
            {
                return propSet(prop, 0, &value, 1);
            }
            udx propSetI(const char* prop, idx value)
            {
                char strBuf[64];
                sprintf(strBuf, "%" DEC, value);
                char * strBufPtr=strBuf;
                return propSet(prop, 0, (const char**)&strBufPtr, 1);
            }
            udx propSetU(const char* prop, udx value)
            {
                char strBuf[64];
                sprintf(strBuf, "%" UDEC, value);
                char * strBufPtr=strBuf;
                return propSet(prop, 0, (const char**)&strBufPtr, 1);
            }
            udx propSetR(const char* prop, real value)
            {
                char strBuf[64];
                sprintf(strBuf, "%lf", value);
                char * strBufPtr=strBuf;
                return propSet(prop, 0, (const char**)&strBufPtr, 1);
            }
            udx propSetBool(const char* prop, bool value)
            {
                char strBuf[64];
                sprintf(strBuf, "%s", value ? "true" : "");
                char * strBufPtr=strBuf;
                return propSet(prop, 0, (const char**)&strBufPtr, 1);
            }
            udx propSetDTM(const char* prop, time_t value)
            {
                char strBuf[64];
                udx t = value;
                sprintf(strBuf, "%" UDEC, t);
                char * strBufPtr=strBuf;
                return propSet(prop, 0, (const char**)&strBufPtr, 1);
            }
            udx propSetHiveId(const char* prop, const sHiveId & value)
            {
                char strBuf[S_HIVE_ID_MAX_BUFLEN];
                value.print(strBuf);
                const char * strBufPtr=strBuf;
                return propSet(prop, 0, &strBufPtr, 1);
            }
            udx propSetHiveIds(const char* prop, const sVec <sHiveId> & values)
            {
                sVec <const char*> posValues;
                posValues.resize(values.dim());
                sStr objHiveIds;
                for (idx ivalue = 0; ivalue < values.dim(); ++ivalue){
                    posValues[ivalue] = values[ivalue].print(objHiveIds);
                    objHiveIds.add0();
                }
                return propSet(prop, posValues);
            }

            //! utility function for parsing '.'-delimeted prop path (group) strings
            static idx readPathElt(const char * path, const char ** next);
            //! sort 2- or 4-column props table in place first by id, then by path (group), then by prop name
            static idx sortProps(sVarSet & res, idx start = 0, idx cnt = sIdxMax);
            // these read props: value, group-row
            //! retrieve 2-column prop table; each row = value, path
            /*! \param res table to which to append
                \param sort if true, sort the appended rows by path, using sortProps() */
            virtual udx propGet(const char* prop, sVarSet& res, bool sort=false) const;
            const char* propGet(const char* prop, sStr* buffer = 0) const;
            //! retrieve 00-list of prop values, sorted by path
            const char* propGet00(const char* prop, sStr* buffer00 = 0, const char * altSeparator = 0 ) const;
            idx propGetI(const char* prop) const
            {
                idx retval = 0;
                const char* p = propGet(prop);
                if( p ) {
                    sscanf(p, "%" DEC, &retval);
                }
                return retval;
            }
            udx propGetU(const char* prop) const
            {
                udx retval = 0;
                const char* p = propGet(prop);
                if( p ) {
                    sscanf(p, "%" UDEC, &retval);
                }
                return retval;
            }
            real propGetR(const char* prop)
            {
                real retval = 0;
                const char* p = propGet(prop);
                if( p ) {
                    sscanf(p, "%lf", &retval);
                }
                return retval;
            }
            bool propGetBool(const char* prop) const
            {
                return sString::parseBool(propGet(prop));
            }
            time_t propGetDTM(const char* prop) const
            {
                udx retval = 0;
                const char* p = propGet(prop);
                if( p ) {
                    sscanf(p, "%" UDEC, &retval);
                }
                return retval;
            }
            bool propGetHiveId(const char * prop, sHiveId & res) const
            {
                res.parse(propGet(prop));
                return res;
            }
            idx propGetHiveIds(const char * prop, sVec<sHiveId> & res) const;
            sUsrTypeField::EType propGetValueType(const char* prop) const;
            const sUsrTypeField * propGetTypeField(const char* prop) const;

            udx propDel(const char * prop, const char * group, const char * value);

            //! Clear all object properties except "created" and "modified" (which will be updated appropriately)
            /*! \param keep_autofill do not clear autofill fields ("reqID", "svc", "progress", etc.) except for "modified"
                \returns true on success */
            static bool propInit(const sUsr& usr, const sHiveId & id, bool keep_autofill = false);

            //! Clear all object properties except "created" and "modified" (which will be updated appropriately)
            /*! \param keep_autofill do not clear autofill fields ("reqID", "svc", "progress", etc.) except for "modified"
                \returns true on success */
            bool propInit(bool keep_autofill = false);

        protected:

            struct CachedPropsTree
            {
                sUsrObjPropsTree t;
                bool is_default;
                CachedPropsTree(const sUsr& usr, const char * typeName): t(usr, typeName), is_default(false) {}
            };

            bool is_auditable;  //workaround to exclude progress reporting from audit logs. By defaults is true (check constructors of object) but uproc sets it to false only for the proc specifc props.
            const sUsr& m_usr;
            sHiveId m_id;
            mutable sHiveId m_type_id;
            mutable std::auto_ptr<CachedPropsTree> m_propsTree;

            static void makeFileName(sStr & buf, const sStr & key);

            virtual bool onDelete(void)
            {
                return true;
            }
            virtual bool onPurge(void)
            {
                return true;
            }

            virtual const char * getFilePathnameX(sStr & buf, const sStr & key, bool check_existence) const;

            // internal for 'type' objects override
            virtual void propBulk(const char * filter00, sVarSet & list) const;
            virtual void propEval(sUsrObjRes & res, const char * filter00) const;

        private:

            mutable sStr locBuf;
            mutable sStr m_path;

            // no one can create empty object!
            // commented by Vahan - in fact one must be able to create an empty object to be able to use this type in containers.
            // otherwise pointers or newly allocated small and inefficient pointers or autopointers should be always carried around
            // sUsrObj();

            sUsrObj& operator =(const sUsrObj&)
            {
                return *this;
            }

            static char * getPath(sStr & path, const sHiveId & id, bool create = false);
            static bool propInitInternal(const sUsr& usr, sUsrObj * uobj, const sHiveId & id, bool keep_autofill);

            friend class sUsr;
            friend class sUsrUsage2;
    };
}

#endif // sLib_usrobj_h

