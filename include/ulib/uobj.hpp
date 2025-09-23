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

            sUsrObj()
                : is_auditable(true), m_usr(*(sUsr*)(&sMex::_zero))
            {
            }
            sUsrObj(const sUsr& usr, const char* type_name)
                : is_auditable(true), m_usr(usr)
            {
                m_usr.objCreate(m_id, type_name);
            }
            sUsrObj(const sUsr& usr, const sHiveId & objId)
                : is_auditable(true), m_usr(usr), m_id(objId)
            {
                if( !m_usr.objGet(m_id) ) {
                    m_id.reset();
                }
            }
            sUsrObj(const sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission)
                : is_auditable(true), m_usr(usr), m_id(objId)
            {
                if( !m_usr.objGet(m_id, ptypeId, permission) ) {
                    m_id.reset();
                }
            }
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

            bool isTypeOf(const char* pattern) const;

            const char* getTypeName(void) const;

            const sUsrType2 * getType(void) const;


            virtual const char * addFilePathname(sStr & buf, bool overwrite, const char* key, ...) const __attribute__((format(printf, 4, 5)));
            virtual char * trashFilePathname(sStr & buf, bool overwrite, const char* key, ...) __attribute__((format(printf, 4, 5)));
            virtual char * restoreFilePathname(sStr & buf, bool overwrite, const char* key, ...) __attribute__((format(printf, 4, 5)));
            virtual bool delFilePathname(const char* key, ...) const __attribute__((format(printf, 2, 3)));
            const char * getFilePathname(sStr & buf, const char* key, ...) const __attribute__((format(printf, 3, 4)));
            const char * getFilePathname00(sStr & buf, const char* key00) const;
            const char * makeFilePathname(sStr & buf, const char* key, ...) const __attribute__((format(printf, 3, 4)));

            udx files(sDir & fieList00, idx dirListFlags, const char * mask = "*", const char * relPath = 0 ) const;
            udx fileProp(sStr & dst, const bool as_csv, const char * wildcard, const bool show_file_size) const;
            udx fileProp(sJSONPrinter & dst, const char * wildcard, const bool into_object, const bool show_file_size) const;

        public:

            static sRC initStorage(const char * pathList, const udx default_min_free_space_per_volume, sQPrideBase * qp_for_logging = 0);

            idx actions(sVec<sStr>& actions, const bool use_ids) const;
            idx jscomponents(sVec<sStr>& components, const bool use_ids) const;


            virtual bool onCreate(void)
            { return true; }

            bool actExecute(void) const
            { return true; }

            bool actDelete(const udx days = 0);
            bool purge(void);
            void cleanup(void);

            sUsrObj* cast(const char* type_name);

            virtual udx propBulk(sVarSet & list, const char* view_name = 0, const char * filter00 = 0, bool allowSysInternal = false) const;
            virtual void propBulk(sVarSet & src, sVarSet & dst, const char* view_name, const char* filter00, bool allowSysInternal) const;
            virtual udx propBulk(sVar & form) const;
            const sUsrObjPropsTree * propsTree(const char* view_name = 0, const char * filter00 = 0, bool force_reload = false) const;

            virtual udx propSet(const char* prop, const char** paths, const char** values, const udx cntValues, bool isAppend = false, const udx * path_lens = 0, const udx * value_lens = 0);
            udx propSet(const char* prop, const char** values, udx cntValues, bool isAppend = false, const udx * path_lens = 0, const udx * value_lens = 0)
            {
                return propSet(prop, 0, values, cntValues, isAppend, path_lens, value_lens);
            }
            udx propSet(const char* prop, sVec<const char*>& valuesVec, bool isAppend = false)
            {
                return propSet(prop, &valuesVec[0], valuesVec.dim(), isAppend);
            }
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
            udx propSetHiveIds(const char * prop, const sVec<sHiveId> & values)
            {
                sVec<idx> posValues;
                posValues.resize(values.dim());
                sStr objHiveIds;
                for( idx i = 0; i < values.dim(); ++i ) {
                    posValues[i] = values[i].print(objHiveIds) - objHiveIds.ptr();
                    objHiveIds.add0();
                }
                sVec<const char *> ptrValues;
                ptrValues.resize(values.dim());
                for( idx i = 0; i < values.dim(); ++i ) {
                    ptrValues[i] = objHiveIds.ptr(posValues[i]);
                }
                return propSet(prop, ptrValues);
            }

            static idx readPathElt(const char * path, const char ** next);
            static idx sortProps(sVarSet & res, idx start = 0, idx cnt = sIdxMax);
            virtual udx propGet(const char* prop, sVarSet& res, bool sort = false, bool allowSysInternal = false) const;
            const char* propGet(const char* prop, sStr* buffer = 0, bool allowSysInternal = false) const;
            const char* propGet00(const char* prop, sStr* buffer00 = 0, const char * altSeparator = 0, bool allowSysInternal = false) const;
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

            static bool propInit(const sUsr& usr, const sHiveId & id, bool keep_autofill = false);

            bool propInit(bool keep_autofill = false);

        protected:

            struct CachedPropsTree
            {
                sUsrObjPropsTree t;
                bool is_default;
                CachedPropsTree(const sUsr& usr, const char * typeName): t(usr, typeName), is_default(false) {}
            };

            bool is_auditable;
            const sUsr& m_usr;
            sHiveId m_id;
            mutable sHiveId m_type_id;
            mutable std::unique_ptr<CachedPropsTree> m_propsTree;

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

            virtual void propBulk(const char * filter00, sVarSet & list) const;
            virtual void propEval(sUsrObjRes & res, const char * filter00, bool allowSysInternal = false) const;

        private:

            mutable sStr locBuf;
            mutable sStr m_path;


            sUsrObj& operator =(const sUsrObj&)
            {
                return *this;
            }

            static char * getPath(sStr & path, const sHiveId & id, bool create = false, sQPrideBase * qp_for_logging = 0);
            static bool propInitInternal(const sUsr& usr, sUsrObj * uobj, const sHiveId & id, bool keep_autofill);

            friend class sUsr;
            friend class sUsrUsage2;
    };
}

#endif 