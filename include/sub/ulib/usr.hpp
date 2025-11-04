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
#ifndef sLib_usr_h
#define sLib_usr_h

#include <slib/core/rc.hpp>
#include <slib/utils/json/printer.hpp>
#include <slib/std/file.hpp>
#include <ssql/mysql.hpp>
#include <ion/sJson.hpp>
#include <memory>

namespace slib {

    class sDir;
    class sUsr;
    class sUsrObj;
    class sUsrType2;
    class sUsrFolder;
    class sQPrideBase;
    namespace qlang { class sUsrContext; }

    class sUsrObjRes
    {
        public:
            class IdIter {
                private:
                    friend class sUsrObjRes;
                    idx pos;

                public:
                    IdIter()
                    {
                        pos = 0;
                    }
                    IdIter(const IdIter& rhs)
                    {
                        pos = rhs.pos;
                    }
                    bool operator==(const IdIter & rhs) const
                    {
                        return pos == rhs.pos;
                    }
                    bool operator>(const IdIter & rhs) const
                    {
                        return pos > rhs.pos;
                    }
                    bool operator<(const IdIter & rhs) const
                    {
                        return pos < rhs.pos;
                    }
            };

            sUsrObjRes()
                : _total(0), _start(0), _table_cnt(0)
            {
                _buf.mex()->add(0, sizeof(idx));
            }

            udx total(void) const
            {
                return _total;
            }
            idx dim(void) const
            {
                return _table_cnt;
            }
            udx start(void) const
            {
                return _start;
            }
            const IdIter & first(void) const
            {
                return _first;
            }
            const IdIter & last(void) const
            {
                return _last;
            }
            IdIter & next(IdIter & it) const
            {
                do {
                    it.pos++;
                } while( it.pos < _table.dim() && !_table.ptr(it.pos)->exists );
                return it;
            }
            IdIter & prev(IdIter & it) const
            {
                do {
                    it.pos--;
                } while( it.pos >= 0 && it.pos < _table.dim() && !_table.ptr(it.pos)->exists );
                return it;
            }
            IdIter & resetIter(IdIter & it, const sHiveId & id) const
            {
                const Optional<TObjProp> * opt = _table.get(&id, sizeof(id), &it.pos);
                if( !opt || !opt->exists ) {
                    it.pos = _last.pos + 1;
                }
                return it;
            }

            typedef struct TPropTbl_struct
            {
                idx path, value, next;
            } TPropTbl;
            typedef sDic<idx> TObjProp;
            bool has(const IdIter & it) const
            {
                if( it.pos >= 0 && it.pos < _table.dim() ) {
                    return _table.ptr(it.pos)->exists;
                }
                return false;
            }
            const sHiveId * id(const IdIter & it) const
            {
                return get(it) ? static_cast<const sHiveId*>(_table.id(it.pos)) : 0;
            }
            const sHiveId * firstId(void) const
            {
                return id(_first);
            }
            const sHiveId * lastId(void) const
            {
                return id(_last);
            }
            const TObjProp * get(const IdIter & it) const
            {
                if( it.pos >= 0 && it.pos < _table.dim() ) {
                    return _table.ptr(it.pos)->get();
                }
                return 0;
            }
            const TObjProp * getFirst(void) const
            {
                return get(_first);
            }
            TObjProp * getFirst(void)
            {
                return get(_first);
            }
            const TObjProp * getLast(void) const
            {
                return get(_last);
            }
            TObjProp * getLast(void)
            {
                return get(_last);
            }
            TObjProp * get(const IdIter & it)
            {
                if( it.pos >= 0 && it.pos < _table.dim() ) {
                    return _table.ptr(it.pos)->get();
                }
                return 0;
            }
            const TObjProp * get(const sHiveId & id) const
            {
                const Optional<TObjProp> * opt = _table.get(&id, sizeof(id));
                return opt ? opt->get() : 0;
            }
            TObjProp * get(const sHiveId & id)
            {
                Optional<TObjProp> * opt = _table.get(&id, sizeof(id));
                return opt ? opt->get() : 0;
            }
            void empty()
            {
                _total = 0;
                _start = 0;
                _table_cnt = 0;

                _table.empty();
                _buf.empty();
            }
            bool del(const IdIter & it)
            {
                if( it.pos >= 0 && it.pos < _table.dim() ) {
                    Optional<TObjProp> * opt = _table.ptr(it.pos);
                    if( opt && opt->exists ) {
                        opt->value.empty();
                        opt->exists = false;
                        _table_cnt--;
                        resetFirstLast();
                        return true;
                    }
                }
                return false;
            }
            bool del(const sHiveId & id)
            {
                Optional<TObjProp> * opt = _table.get(&id, sizeof(id));
                if( opt && opt->exists ) {
                    opt->value.empty();
                    opt->exists = false;
                    _table_cnt--;
                    resetFirstLast();
                    return true;
                }
                return false;
            }

            const TPropTbl * get(const TObjProp & obj, const char * name) const
            {
                const idx * t = obj.get(name, sLen(name) + 1);
                return (t && *t > 0) ? (const TPropTbl *)_buf.mex()->ptr(*t) : 0;
            }
            bool del(TObjProp & obj, const char * name) const;

            const TPropTbl * getNext(const TPropTbl * tbl) const
            {
                if( tbl && tbl->next ) {
                    do {
                        tbl = (const TPropTbl *)_buf.mex()->ptr(abs(tbl->next));
                    } while(tbl->path < 0);
                    return tbl;
                }
                return 0;
            }
            bool del(TPropTbl * tbl) const;
            const char * getPath(const TPropTbl * tbl) const
            {
                return tbl ? (const char*)_buf.id(tbl->path) : 0;
            }
            const char * getValue(const TPropTbl * tbl) const
            {
                return tbl ? (const char*)_buf.id(tbl->value) : 0;
            }
            void csv(const sUsrObjRes::IdIter & it, sStr & buf) const;
            void dcsv(const sUsrObjRes::IdIter & it, sStr & buf, sDic < idx > * propDic) const;
            void prop(const sUsrObjRes::IdIter & it, sStr & buf) const;

            void json(const sUsr & user, const sUsrObjRes::IdIter & it, sJSONPrinter & printer, bool into_object, bool flatten = false, bool upsert = false, const char * upsert_qry = 0, const char * prop_filter00 = 0, const char * prop_exclude00 = 0) const;

        private:

            template <class T>
            struct Optional {
                T value;
                bool exists;

                Optional<T>()
                {
                    exists = true;
                }

                T * get()
                {
                    return exists ? &value : 0;
                }

                const T * get() const
                {
                    return exists ? &value : 0;
                }
            };

            TObjProp * add(const sHiveId & id);
            bool add(TObjProp & obj, const char * prop, const char * path, const idx path_len, const char * value, const idx value_len);
            void resetFirstLast()
            {
                for(_first.pos = 0; _first.pos < _table.dim() && !_table.ptr(_first.pos)->exists; _first.pos++);
                for(_last.pos = _table.dim() - 1; _last.pos >= 0 && !_table.ptr(_last.pos)->exists; _last.pos--);
            }

            friend class IdIter;
            friend class sUsr;
            friend class sUsrObj;
            udx _total, _start, _table_cnt;
            IdIter _first, _last;
            sDic< Optional<TObjProp> > _table;
            sDic<idx> _buf;
    };

    class sUsr
    {
        public:
            sVar * pForm;
            sUsr(idx usrid = 0);
            ~sUsr()
            {
                reset();
            }

        public:
            typedef enum
            {
                eUserBlocked = 0,
                eUserOperational,
                eUserEmailNotValidated,
                eUserNotFound,
                eUserAccountExpired,
                eUserPswdExpired,
                eUserNotSet,
                eUserInternalError,
                eUserLoginAttemptsWarn1Left,
                eUserLoginAttemptsNowLocked,
                eUserLoginAttemptsTooMany,
                eUserLoginAttemptsTooManyTryLater
            } ELoginResult;

            idx setAdmin(const char * email, idx super);
            ELoginResult login(const char * email, const char * pswd, const udx token, const char * ipaddr, idx * plogCount = 0);
            ELoginResult loginAsGuest(void);
            void logout(const char * ipaddr);
            void session(udx sid, udx uid, idx key, const char* ipaddr);
            void batch(const char * ipaddr);
            ELoginResult token(const char * email, sStr & token);

            bool passwordCheckQuality(const char * mod, const char * mod1);
            bool passwordReset(const char* email, udx pswd_reset_id, const char * mod, const char * mod1);
            idx update(const bool isnew, const char * email, const char * password, const char * newpass1, const char * newpass2, idx statusNeed,
                    const char * firstName, const char * lastName, sVec<idx>& groups, idx softExpiration, idx hardExpiration, const char* baseURL, bool checkPwd=true);

            bool sendEmailValidation(const char* baseURL, const char* email, const char* firstName = 0, const char* lastName = 0);
            bool sendAccountActivation(const char* baseURL, const char* email);
            udx addPasswordResetID(const char* email);
            bool sendForgotten(const char* baseURL, const char* email);
            bool verifyEmail(const char* baseURL, const char* email);
            bool accountActivate(const char* baseURL, const char* email);
            bool groupActivate(idx groupId, bool isUsr=false);
            bool groupActivate(const char * groupPath);
            const char * groupName(idx groupId, sStr * groupName=0, bool isGroupUsrID=false);
            idx groupIdFromPath(const char * groupPathList, sVec<idx> * groups, const char * mygroup=0, sVec<idx> * guids=0);
            idx groupList( sVec<idx> * groups, udx usrID=0, sVec <idx > * guids=0);
            bool groupCreate(const char* name, const char* abbr, const char* parent, const char * email);
            bool groupEdit(const char* path, const char* firstName, const char * lastName);
            bool groupDelete(const char* path){return groupEdit(path,0,0);}
            bool contact(const char * from_email, const char * subject, const char * body);

            idx listUsr(sVec<sStr> * userList, idx isgrp = 0, bool allUsr = false, bool active = false, const char * search = 0, bool primaryGrpOnly = false, bool billable = false, bool with_system = false) const;
            idx listGrp(sVec<sStr> * userList, idx isgrp = 0, idx usrOnly = 0, const char * search = 0, bool with_system = false, bool with_service = false) const;
            idx printUserInfo(sJSONPrinter & out, bool into_object, const sVec<udx> * user_ids = 0, bool without_current = false, const char * path_prefix = 0, bool with_inactive = false, bool with_system = false);
            idx listUserGroups(sVarSet & tbl, udx user_id, bool active) const;
            idx listInactive(sVec<sStr> * userList, idx isgrp = 0);
            idx exportUsrGrp4Ion(sJSONPrinter & out);
            udx getGroupId(const char * grp_name, bool reset_cache = false) const;
            idx permPropagate(const char * objIdList,const char * permObjIdList=0, const char * objTypeID=0,const char * sharerOverride=0);
            idx usrExpand(const char * hugoObjList, sVar * pForm, sVec <sHiveId> & newObjList);
            const char * objJS(idx obj, sStr * json=0, idx * plen=0, bool respectArr=true);
            sJson * objJson(idx obj, sJson * json=0, bool respectArr=true);

            typedef enum {
                eUserAuditOff,
                eUserAuditAdmin,
                eUserAuditLogin,
                eUserAuditActions,
                eUserAuditFull
            } EAuditMode;
            EAuditMode audit() const;
            bool audit(sUsr::EAuditMode mode, const char * oper, const char * fmt, ...) const __attribute__((format(printf, 4, 5)));

            udx Id(void) const
            {
                return m_Id;
            }
            const char * IdStr(sStr * buf = 0) const
            {
                static sStr lclBuf;
                if( !buf ) {
                    lclBuf.cut(0);
                    buf = &lclBuf;
                }
                buf->printf("%" UDEC, Id());
                return buf->ptr();
            }
            udx groupId(void) const
            {
                return m_PrimaryGroup;
            }
            const char* Name(sStr* buf = 0) const;
            const char* firstName(void) const
            {
                return m_First.ptr();
            }
            const char* lastName(void) const
            {
                return m_Last.ptr();
            }
            bool isAdmin() const
            {
                return m_IsAdmin;
            }
            bool isGuest() const
            {
                return m_IsGuest;
            }
            bool isEmailValid() const
            {
                return m_IsEmailValid;
            }
            const char* Email() const
            {
                return m_Email.ptr();
            }
            udx SID() const
            {
                return m_SID;
            }
            const char* encodeSID(sStr & sid, sStr & buf);
            const char* groupList(bool inactive = false) const;

            bool hasGroup(const char * path_prefix, bool direct_member_only = false, bool with_inactive = false) const;


            const char * getSingletonVar(const char * type_name, const char * var);
            bool setSingletonVar(const char * type_name, ... );
            udx getObjOwner(const sHiveId & obj, udx * userId=0);
            udx getGroupUser(udx userId);

            sRC objCreate(sHiveId & out_id, const char* type_name, const udx in_domainID = 0, const udx in_objID = 0) const;
            bool objGet(const sHiveId & id) const;
            bool objGet(const sHiveId & id, const sHiveId * ptypeHiveId, udx permission) const;
            sUsrObj* objFactory(const sHiveId & id) const;
            sUsrObj* objFactory(const sHiveId & id, const sHiveId * ptypeId, udx permission) const;
            bool allowExpiredObjects(bool allowed);
            const char * createSharedUniqueObject(sUsrObj * objDst, const char * objType, const char * uniqueVar,const char * uniqueVal,  const char * folder=0, const char * shareTo=0);
            const char * findUniqueObject(sUsrObj * objDst, const char * objType, const char * uniqueVar,const char * uniqueVal);
            const char * ensureUniqueObjectProvided(sUsrObj * objSrc, const char * objType, const char * uniqueVar,const char * uniqueVal, const char * typeSearch=0);
            idx replaceVarsFromObjForm(sStr * dst, const char * script,sUsrObj * obj, sVar * pForm=0, sStr * log=0, bool jsonMode=false) const;
            const char * uniqueObjectAndPath( const char * objType, const char * idorname, sUsrObj * obj, sStr * pathAlgo);


            udx objs2(const char* type_names, sUsrObjRes & res, udx* total_qty = 0, const char* prop = 0, const char* value = 0, const char * prop_name_csv = 0, bool permissions = false, const udx from = 0, const udx qty = 0, bool allowSysInternal = false, const char * roles=0 ) const;
            udx objs(const sHiveId* ids, const udx cnt_ids, sVec<sHiveId>& out) const;
            udx objs(sVec<sHiveId> &ids, sVec<sHiveId>& out) const
            {
                return objs(&ids[0], ids.dim(), out);
            }
            udx all(sDic<udx> & list, const char* types = 0) const;
            udx removeTrash(sUsrObjRes & res, bool return_total_count = false) const;

            void propBulk(sVec<sHiveId> &ids, sVarSet & list, const char* view_name, const char* filter00, bool allowSysInternal = false) const;

            bool propSetJson(sJson * jsonObj, const char * jsonText=0, sStr * log=0);

            bool propSet(sVar & form, sStr & log, sVec<sHiveId>* new_ids = 0, sVec<sHiveId>* updated_ids = 0, sDic<sHiveId> * new_ids_map = 0) const;

            bool propSet( sVar & form, sStr & log, sVarSet & result, bool not_for_db = false) const;
            bool propSet(const char * srcbuf, idx len, sStr & log, sVarSet & result, bool not_for_db = false) const;

            bool propSet(const char * srcbuf, idx len, sStr& log, sVec<sHiveId>* new_ids=0, sVec<sHiveId>* updated_ids=0, sDic<sHiveId> * new_ids_map = 0);
            bool propSet(const char * propFileName, sStr& log, sVec<sHiveId>* new_ids = 0, sVec<sHiveId>* updated_ids = 0, sDic<sHiveId> * new_ids_map = 0);

            template<typename Tobjtype> bool propSet(sVar& form, sStr& log, sVec<Tobjtype> & objList, sStr * strObjIdList)
            {
                sVec<sHiveId> ids;
                if( !propSet(form, log, &ids, &ids) ) {
                    return false;
                }
                Tobjtype * newadded = objList.add(ids.dim());
                for(idx i = 0; i < ids.dim(); ++i) {
                    new (newadded + i) Tobjtype(*this, ids[i]);
                }
                if( strObjIdList ) {
                    sHiveId::printVec(*strObjIdList, ids);
                    strObjIdList->add0();
                }
                return true;
            }

            enum EPermExport {
                ePermExportNone = 0,
                ePermExportGroups = 1,
                ePermExportAll = 2
            };
            sHiveId propExport(sVec<sHiveId>& ids, sVarSet & v, EPermExport permissions) const;
            sHiveId propExport(sVec<sHiveId>& ids, sJSONPrinter & printer, EPermExport permissions, bool flatten = false, bool upsert = false, const char * upsert_qry = 0, const char * prop_filter00 = 0, const char * prop_exclude00 = 0) const;
            bool objFilesExport(sVec<sHiveId>& ids, sVarSet & v, const char * dst, const char * mask = 0) const;
            sRC objHivepack(sVec<sHiveId>& ids, const char * dstName, sPipe::callbackFuncType callback, void * callbackParam) const;

            bool setPermission(udx groupId, const sHiveId & objHiveId, udx permission, udx flags, sHiveId * viewId = 0, const char * forObjID = 0) const;
            bool copyPermission(const sHiveId & objHiveIdFrom, const sHiveId & objHiveIdTo) const;
            bool allow4admins(const sHiveId & objHiveId) const;
            bool allowRead4users(const sHiveId & objHiveId) const;
            void permPrettyScanf(const char * group, const char * view, const char* sperm, const char* sflags, const sUsrType2 * type, udx * groupId, sHiveId * viewId, udx * perm, udx * flags) const;

            bool encodeField(sStr * out_encoded_value, sMex * out_encoded_blob, idx encoding, const void * orig_value, idx len) const;
            bool decodeField(sMex * out, idx encoding, const void * encoded_value, idx value_len, const void * encoded_blob, idx blob_len) const;

        protected:

            bool isAllowed(const sHiveId & objHiveId, udx permission) const;
            bool isAllowedAndHasType(const sHiveId & objHiveId, const sHiveId * ptypeHiveId, udx permission) const;
            const sUsrType2 * objGetType(const sHiveId & objHiveId, udx permission) const;
            udx objPermEffective(const sHiveId & objHiveId) const;
            sUsrObj * getDomainIdDescr(udx domain_id) const;

        public:
            void objPermAll(sVec<sHiveId>& ids, sVarSet &tbl, bool expand_grp = false) const;

            static sQPrideBase * QPride()
            {
                return sm_qpride;
            }

            static void setQPride(sQPrideBase * qpride);
            static const char* const getKey(void);

            bool updateStart() const
            {
                return db().start_transaction();
            }
            bool updateComplete() const
            {
                return db().commit();
            }
            bool updateAbandon() const
            {
                return db().rollback();
            }
            udx getUpdateLevel() const
            {
                return db().in_transaction();
            }
            bool hadDeadlocked() const
            {
                return db().hadDeadlocked();
            }
            void createGroup (const char * grp)
            {
                db().execute("call sp_create_group('%s');",grp);
            }

        public:
            sStr err;
            bool init(udx userId = 0);
            sUsr(const char* service_name, bool su_mode = false);
            bool m_SuperUserMode;

        protected:

            friend class sUsrObjRes;
            friend class sUsrObj;
            friend class sUsrType;
            friend class sUsrType2;
            friend class sUsrLoadingType;
            friend class sUsrObjType;
            friend class sUsrCGI;
            friend class sUsrFolder;
            friend class sUsrEmail;
            friend class sUsrProc;

            friend class sUSrv;
            friend class sQPrideSrv;
            friend class sUsrUsage;
            friend class sUsrUsage2;
            friend class sQPrideDB2;
            friend class qlang::sUsrContext;
            friend class sUsrHousekeeper;
            friend class sUsrPropSet;

            sSql* pdb(bool initIfUndefined) const;
            sSql& db(void) const
            {
                return *pdb(true);
            }

            bool cacheObjPerm(const sVarSet& tbl) const;
            bool cacheObj(const sHiveId & id, const sHiveId * type, udx flags, udx bits) const;
        public:
            const sUsrType2 * objType(const sHiveId & objHiveId, sHiveId * out_objTypeId = 0) const;
        protected:
            void reset(void);
            sSql::sqlProc* getProc(const char* sp_name) const;

            void initFolders(bool keepHierarchy);



            udx objsLowLevel(const char * type_names, const char * obj_filter_sql, const char * prop_filter_sql, const char * prop_name_csv, bool permissions, const udx start, const udx count, sUsrObjRes * res = 0, udx * total_qty = 0, bool allowSysInternal = false, const char * roles=0) const;

        public:
            char * dbProtectValue(sStr& to, const char* from, udx length = 0) const { return db().protectValue(to, from, length); }
            bool m_printAutoType;

        private:

            bool copy2res(sUsrObjRes & res) const;

            idx prohibitSelfRegistration, checkComplexity;

            static sQPrideBase * sm_qpride;
            static sSql sm_cfg_db;
            static sSql * sm_actual_db;

            udx m_Id;
            udx m_PrimaryGroup;
            bool m_IsAdmin;
            bool m_IsGuest;
            bool m_IsEmailValid;
            sStr m_Email;
            sStr m_First;
            sStr m_Last;
            sStr m_membership;
            udx m_SID;
            idx m_SIDrnd;

            mutable bool m_AllowExpiredObjects;

            struct sObjPerm
            {
                    sHiveId type;
                    udx allow, deny;
                    enum EExpiration {
                        eUnexpired,
                        eExpired,
                        eMaybeExpired
                    } expiration;
            };
            typedef std::auto_ptr< sDic<sObjPerm> > TPermCache;
            mutable TPermCache m_ObjPermission;

            void cacheRemove(const sHiveId & objHiveId) const;
            bool passwordReset(udx userId, const char * mod, const char * mod1);
    };

}
;

#endif 