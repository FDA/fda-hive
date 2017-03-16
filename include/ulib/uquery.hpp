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
#ifndef sLib_uquery_h
#define sLib_uquery_h

#include <qlang/interpreter.hpp>
#include <qlang/engine.hpp>
#include <ulib/usr.hpp>
#include <ulib/uobj.hpp>

namespace slib {
    namespace qlang {
        class sUsrContext : public Context {
        protected:
            struct sUsrObjWrapper {
                sUsrObj *obj;
                bool allowed;
                sDic<sVariant> fieldCache;
                sDic<sVariant> fieldOverride;
                sDic<sVariant> fieldBuiltin;

                sUsrObjWrapper()
                {
                    obj = 0;
                    allowed = false;
                }
            };

            const sUsr * _usr;
            sDic<sUsrObjWrapper> _objCache;
            idx _flags;

            virtual void registerDefaultBuiltins();
            sUsrObjWrapper* declareObjectId(const sHiveId & objId);
            eEvalStatus evalUsrObjWrapper(sUsrObjWrapper **pwrapper, sVariant &objVal);
            void parseField(sVariant &outval, sUsrObjWrapper *wrapper, const char *name, const sVarSet &propValSet, const sVec<idx> *rows=NULL, idx pathCol=1, idx valueCol=0);
            virtual bool isUsableField(const sUsrTypeField * fld);
            const sUsrTypeField * getTypeField(sUsrObjWrapper * objWrapper, const char * name);

        public:
            enum EFlags {
                fUnderscoreId = 1 << 0
            };
            sUsrContext();
            sUsrContext(const sUsr & usr, idx flags=0) { init(usr, flags); }
            virtual ~sUsrContext();
            virtual void init(const sUsr & usr, idx flags=0);
            virtual void reset();

            idx getFlags() const { return _flags; }

            // builtins
            using Context::registerBuiltinValue; // silly c++
            using Context::registerBuiltinThis; // silly c++
            bool registerBuiltinProperty(sVariant &objVal, const char * name, sVariant &value);
            bool registerBuiltinProperties(sVariant &objVal, const sUsrObjPropsTree &tree);
            bool registerBuiltinValue(const char * name, const sUsrObjPropsTree &tree);
            inline bool registerBuiltinThis(const sUsrObjPropsTree &tree)
            {
                return registerBuiltinValue("this", tree);
            }

            const sUsr * getUsr() const { return _usr; }

            eEvalStatus evalUsrObj(sUsrObj **pobj, sVariant &objVal);
            virtual bool evalValidObjectId(sVariant &objVal);
            virtual eEvalStatus evalHasProperty(sVariant &outval, sVariant &objVal, const char *name);
            virtual eEvalStatus evalGetProperty(sVariant &outval, sVariant &objVal, const char *name);
            virtual eEvalStatus evalSetProperty(sVariant &objVal, const char *name, sVariant &val);
            virtual eEvalStatus evalProps(sVariant &outval, sVariant &objVal, sVariant *options=NULL);
            virtual eEvalStatus evalProps(sVariant &outval, sVariant &objVal, const char * filter00);
            virtual bool isPropObjectValued(sVariant &objVal, const char *name);
            //! return all objects of a given type, matching a prop filter as in alloftype() builtin function
            /*! \param[out] res Optionally, retrieve the raw sUsrObjRes result set of found objects (0 to not retrieve)
             *  \param res_props List of field names to retrieve into res, if res is non-NULL */
            virtual void getAllObjsOfType(sVariant &outval, const char *typeName, sVariant *propFilter=0, sUsrObjRes * res=0, sVariant * res_props=0);

            virtual void declareObjectId(sVariant &objVal);
            virtual void declareObjlist(sVariant &objlist);

            void cacheObjectFields(sVariant &objVal, const char *typeNames00=NULL);
            void uncacheObjectFields(sVariant &objVal);
        };

        //! Like sUsrContext, but with additional (unsafe) capabilites for internal use in HIVE's backend
        class sUsrInternalContext : public sUsrContext {
        protected:
            virtual void registerDefaultBuiltins();
            virtual bool isUsableField(const sUsrTypeField * fld);

        public:
            sUsrInternalContext() : sUsrContext() {}
            sUsrInternalContext(sUsr &usr, idx flags=0) : sUsrContext(usr, flags)
            {
                registerDefaultBuiltins();
            }
            virtual void init(sUsr & usr, idx flags=0)
            {
                sUsrContext::init(usr, flags);
                registerDefaultBuiltins();
            }
            virtual ~sUsrInternalContext() {}

            sUsr * getUsr() const { return const_cast<sUsr*>(_usr); }

            //  generic dispatcher callback
            static bool dispatcher_callback(sVariant &result, const qlang::BuiltinFunction &funcObj, qlang::Context &ctx, sVariant *topic, sVariant *args, idx nargs, void *param);
            sVariant WorkDirectory;
            const char * workDir(void)
            {
                const char * res = WorkDirectory.asString();
                if( res && *res )
                    return res;
                getVarValue("workDir", WorkDirectory);
                return WorkDirectory.asString();
            }
        };

        class sUsrEngine : public Engine {
        protected:
            const sUsr * _usr;

        public:
            sUsrEngine();
            sUsrEngine(const sUsr &usr, idx ctx_flags=0);
            virtual void init(const sUsr &usr, idx ctx_flags=0);
            virtual ~sUsrEngine() {}
            virtual sUsrContext& getContext() { return *static_cast<sUsrContext*>(_ctx); }

            using Engine::registerBuiltinThis; // silly c++
            using Engine::registerBuiltinValue; // silly c++

            inline virtual bool registerBuiltinValue(const char * name, const sUsrObjPropsTree & tree)
            {
                return getContext().registerBuiltinValue(name, tree);
            }
            virtual bool registerBuiltinValuePropertiesTable(const char * name, const char * typeName, sVarSet & propsTable);
            virtual bool registerBuiltinValuePropertiesForm(const char * name, const char * typeName, const sVar & form);

            virtual bool registerBuiltinThis(const sHiveId & id)
            {
                sVariant o;
                o.setHiveId(id);
                return registerBuiltinValue("this", o);
            }
            inline virtual bool registerBuiltinThis(const sUsrObjPropsTree & tree)
            {
                return getContext().registerBuiltinThis(tree);
            }
            inline virtual bool registerBuiltinThisPropertiesTable(const char * typeName, sVarSet & propsTable)
            {
                return registerBuiltinValuePropertiesTable("this", typeName, propsTable);
            }
            inline virtual bool registerBuiltinThisPropertiesForm(const char * typeName, const sVar & form)
            {
                return registerBuiltinValuePropertiesForm("this", typeName, form);
            }

            virtual bool registerBuiltinPropertiesTable(const char * typeName, const sHiveId & objId, sVarSet & propsTable);
            virtual bool registerBuiltinPropertiesForm(const char * typeName, const sHiveId & objId, const sVar & form);
            virtual bool registerBuiltinProperties(const sHiveId & id, const sUsrObjPropsTree & tree)
            {
                sVariant o;
                o.setHiveId(id);
                return getContext().registerBuiltinProperties(o, tree);
            }
            virtual bool registerBuiltinProperty(const sHiveId & id, const char * name, sVariant & value)
            {
                sVariant o;
                o.setHiveId(id);
                return getContext().registerBuiltinProperty(o, name, value);
            }
            inline virtual bool registerBuiltinIdxProperty(const sHiveId & objId, const char * name, idx i)
            {
                sVariant value(i);
                return registerBuiltinProperty(objId, name, value);
            }
            inline virtual bool registerBuiltinUdxProperty(const sHiveId & objId, const char * name, udx u)
            {
                sVariant value;
                value.setUInt(u);
                return registerBuiltinProperty(objId, name, value);
            }
            inline virtual bool registerBuiltinRealProperty(const sHiveId & objId, const char * name, real r)
            {
                sVariant value(r);
                return registerBuiltinProperty(objId, name, value);
            }
            inline virtual bool registerBuiltinStringProperty(const sHiveId & objId, const char * name, const char * s)
            {
                sVariant value(s);
                return registerBuiltinProperty(objId, name, value);
            }

        };

        class sUsrInternalEngine : public sUsrEngine {
        public:
            sUsrInternalEngine() : sUsrEngine() {}
            sUsrInternalEngine(sUsr &usr, idx ctx_flags=0);
            virtual void init(sUsr &usr, idx ctx_flags=0);
            virtual ~sUsrInternalEngine() {}
            virtual sUsrInternalContext& getContext() { return *static_cast<sUsrInternalContext*>(_ctx); }
        };
    };

    typedef qlang::sUsrContext sUsrQueryContext;
    typedef qlang::sUsrEngine sUsrQueryEngine;
    typedef qlang::sUsrInternalContext sUsrInternalQueryContext;
    typedef qlang::sUsrInternalEngine sUsrInternalQueryEngine;
};

#endif
