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
#ifndef sLib_qlang_interpreter_h
#define sLib_qlang_interpreter_h

#include <slib/core/dic.hpp>
#include <slib/core/str.hpp>
#include <slib/std/variant.hpp>
#include <slib/core/vec.hpp>
#include <regex.h>

/*! \file
 *  \ref qlang "See README.qlang.doxygen for information about the query language" */

namespace slib {
    //! Query language namespace
    namespace qlang {
        //! line/column location in query language program source
        class SourceLocation {
        protected:
            idx _line;
            idx _col;
        public:
            SourceLocation(idx line=-1, idx col=-1): _line(line), _col(col) {}
            void set(idx line, idx col) { _line = line; _col = col; }
            bool print(sStr &s) const;
        };

        class Scope; class Context; class BuiltinFunction;

        //! Interface for a callable entity in the query language, e.g. a builtin function, or a lambda expression defined in the query text
        class Callable {
        public:
            virtual ~Callable() {}
            virtual const char *getName() const = 0;
            virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const = 0;
        };

        //! Wrapper allowing a qlang::Callable to be used as an sVariant and to be called in a specific scope
        class CallableWrapper: public sVariant::sVariantData {
        public:
            typedef bool (*CallableBuiltin)(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs);

        protected:
            const Callable *_callable;
            Scope *_scope;

        public:
            virtual CallableWrapper* clone();
            CallableWrapper(const Callable *callable, Scope *scope=NULL);
            CallableWrapper(const CallableWrapper &rhs);
            CallableWrapper& operator=(const CallableWrapper &rhs);
            virtual ~CallableWrapper() { empty(); }
            void markScope();
            void empty();
            void unregisterScope() { _scope = NULL; }
            bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const;

            bool operator==(const CallableWrapper& rhs) const;
            bool operator<(const CallableWrapper& rhs) const;
            bool operator>(const CallableWrapper& rhs) const;

            bool operator==(const sVariant::sVariantData& rhs_) const
            {
                const CallableWrapper * rhs = dynamic_cast<const CallableWrapper*>(&rhs_);
                if (!rhs)
                    return false;

                return operator==(*rhs);
            }
            bool operator<(const sVariant::sVariantData& rhs_) const
            {
                const CallableWrapper * rhs = dynamic_cast<const CallableWrapper*>(&rhs_);
                if (!rhs)
                    return this < static_cast<const void *>(&rhs_);

                return operator<(*rhs);
            }
            bool operator>(const slib::sVariant::sVariantData& rhs_) const
            {
                const CallableWrapper * rhs = dynamic_cast<const CallableWrapper*>(&rhs_);
                if (!rhs)
                    return this > static_cast<const void *>(&rhs_);

                return operator>(*rhs);
            }

            virtual const char* getTypeName() const;
            virtual void print(sStr &s, sVariant::ePrintMode mode=sVariant::eDefault, const sVariant::Whitespace * whitespace=0, idx indent_level=0) const;
            virtual bool asBool() const { return true; }
            virtual idx asInt() const { return 0; }
            virtual real asReal() const;
            virtual void getNumericValue(sVariant &numeric) const { numeric.setReal(asReal()); }
        };

        //! Query language scope frame. These form a tree to allow lexical closure.
        class Scope {
        public:
            enum ScopeType {
                scope_BASIC = 0,
                scope_LOOP,
                scope_LAMBDA
            };

        protected:
            enum ScopeState {
                scope_INACTIVE = 1,
                scope_DEACTIVATING = 1 >> 2,
                scope_MARKED = 1 >> 3
            };

            struct ScopeEntry {
                sVariant value;
                bool readOnly;

                ScopeEntry(): readOnly(false) {}
                void set(sVariant &v, bool ro) { value = v; readOnly = ro; }
            };

            sDic<ScopeEntry> _namespace;
            sVec<Scope*> _children;
            sDic<CallableWrapper*> _callables;
            Scope *_parent;

            idx _refCount;
            idx _state;
            ScopeType _type;

            idx sweepRecurse();

        public:
            Scope(ScopeType type=scope_BASIC);
            virtual ~Scope();
            void deactivate();
            void empty();

            Scope* addChild(ScopeType type=scope_BASIC);
            inline Scope* getParent() const { return _parent; }

            void registerCallable(CallableWrapper *cwrapper);
            void unregisterCallable(CallableWrapper *cwrapper);
            void unregisterAllCallables();

            void incrementRefCount();
            void decrementRefCount();
            bool hasRefs() const { return _refCount > 0; }

            bool getValue(const char *name, sVariant &outval);
            bool addValue(const char *name, sVariant &val, bool readOnly=false);
            bool setValue(const char *name, sVariant &val, bool readOnly=false);

            void mark();
            // \returns Number of scopes garbage-collected
            idx sweep();

            ScopeType getType() const { return _type; }
            void print(sStr &out);
        };

        enum eEvalStatus {
            // warning: keep in sync with ContextEvalStatusNames in interpreter.cpp
            EVAL_SUCCESS = 1,
            EVAL_OTHER_ERROR = 0,
            EVAL_NOT_IMPLEMENTED = -1,
            EVAL_RANGE_ERROR = -2,
            EVAL_TYPE_ERROR = -3,
            EVAL_WANTED_LIST = -4,
            EVAL_WANTED_DIC = -5,
            EVAL_WANTED_SUBSCRIPTABLE = -6,
            EVAL_WANTED_CALLABLE = -7,
            EVAL_WANTED_OBJECT = -8,
            EVAL_BAD_ARGS_NUMBER = -9,
            EVAL_PERMISSION_ERROR = -10,
            EVAL_READ_ONLY_ERROR = -11,
            EVAL_SYNTAX_ERROR = -12,
            EVAL_NO_SUCH_PROP = -13,
            EVAL_VARIABLE_ERROR = -14,
            EVAL_SYSTEM_ERROR = -15
        };

        //! Interpreter context for query language
        class Context {
        public:
            enum eRunningMode {
                MODE_NORMAL = 0,
                MODE_RETURN,
                MODE_BREAK,
                MODE_CONTINUE,
                MODE_ERROR
            };

            typedef bool (*BuiltinRawCallback)(sVariant &result, const BuiltinFunction &funcObj, Context &ctx, sVariant *topic, sVariant *args, idx nargs, void *param);
            typedef bool (*BuiltinGetter)(sVariant &result, void *param);

        protected:
            enum eBuiltinTypes {
                BUILTIN_INVALID = 0,
                BUILTIN_IDX_PTR,
                BUILTIN_IDX_VAL,
                BUILTIN_UDX_PTR,
                BUILTIN_UDX_VAL,
                BUILTIN_REAL_PTR,
                BUILTIN_REAL_VAL,
                BUILTIN_STRING_PTR,
                BUILTIN_VARIANT_OWNED,
                BUILTIN_FUNCTION,
                BUILTIN_FUNCTION_OWNED,
                BUILTIN_GETTER
            };

            struct Builtin {
                idx type;
                union {
                    idx * pi;
                    idx i;
                    udx * pu;
                    udx u;
                    real * pr;
                    real r;
                    sStr * ps;
                    sVariant * pvar;
                    const BuiltinFunction * f;
                    BuiltinGetter g;
                } val;
                void * param;
                bool isConst;
                Builtin() { sSet(this, 0); }
                ~Builtin() { empty(); }
                void empty();
            };

            sDic<Builtin> _builtins;

            Scope _rootScope;
            Scope *_curScope;

            idx _mode;
            sVariant _returnValue;

            eEvalStatus _errorCode;
            sStr _errorStr;
            SourceLocation _errorLoc;

            const char* statusName(eEvalStatus status);
            virtual void registerDefaultBuiltins();

        public:
            Context();
            virtual ~Context();
            virtual void reset();

            // builtins
            bool hasBuiltin(const char *name) { return _builtins.get(name); }
            bool copyBuiltin(const char *newname, const char *oldname);
            bool removeBuiltin(const char *name);
            void registerBuiltinIdxPtr(const char *name, idx *ptr, bool isConst=false);
            void registerBuiltinIdxConst(const char *name, idx val);
            void registerBuiltinUdxPtr(const char *name, udx *ptr, bool isConst=false);
            void registerBuiltinUdxConst(const char *name, udx val);
            void registerBuiltinRealPtr(const char *name, real *ptr, bool isConst=false);
            void registerBuiltinRealConst(const char *name, real val);
            void registerBuiltinStringPtr(const char *name, sStr *ptr, bool isConst=false);
            void registerBuiltinFunction(const char *name, const BuiltinFunction &funcObj);
            void registerBuiltinFunction(const char *name, BuiltinRawCallback funcPtr, void *param);
            void registerBuiltinGetter(const char *name, BuiltinGetter funcPtr, void *param);
            bool registerBuiltinValue(const char *name, sVariant &value);
            inline bool registerBuiltinThis(sVariant &value) { return registerBuiltinValue("this", value); }

            void pushScope(Scope::ScopeType type=Scope::scope_BASIC);
            void pushLoop() { pushScope(Scope::scope_LOOP); }
            void pushLambda() { pushScope(Scope::scope_LAMBDA); }
            void popScope();
            Scope* getScope() { return _curScope; }
            void setScope(Scope* scope) { _curScope = scope; }
            bool isScopeInLoop() const;

            idx getMode() { return _mode; }
            void clearBreakContinue();
            void clearReturn(bool resetReturnValue=true);

            bool setReturn(sVariant &val);
            sVariant& getReturnValue() { return _returnValue; }
            bool setBreak();
            bool setContinue();

            bool getVarValue(const char *name, sVariant &outval);
            bool addVarValue(const char *name, sVariant &val);
            bool setVarValue(const char *name, sVariant &val);
            bool isCallable(const sVariant &val);

            bool setError(const SourceLocation &loc, eEvalStatus code, const char *fmt, ...) __attribute__((format(printf, 4, 5)));
            bool setError(eEvalStatus code, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
            const char * printError(sStr &s);
            void clearError();

            void printScopes(sStr &s) { _rootScope.print(s); }

            // The standard operations
            virtual eEvalStatus evalAdd(sVariant &outval, sVariant &lhs, sVariant &rhs);
            virtual eEvalStatus evalSubtract(sVariant &outval, sVariant &lhs, sVariant &rhs);
            virtual eEvalStatus evalMultiply(sVariant &outval, sVariant &lhs, sVariant &rhs);
            virtual eEvalStatus evalDivide(sVariant &outval, sVariant &lhs, sVariant &rhs);
            virtual eEvalStatus evalRemainder(sVariant &outval, sVariant &lhs, sVariant &rhs);
            virtual eEvalStatus evalIncrement(sVariant &val);
            virtual eEvalStatus evalDecrement(sVariant &val);
            virtual eEvalStatus evalUnaryPlus(sVariant &val);
            virtual eEvalStatus evalUnaryMinus(sVariant &val);
            virtual eEvalStatus evalGetSubscript(sVariant &outval, sVariant &lhs, sVariant &index);
            virtual eEvalStatus evalSetSubscript(sVariant &lhs, sVariant &index, sVariant &rhs);
            virtual eEvalStatus evalGetSlice(sVariant &outval, sVariant &lhs, sVariant &index1, sVariant &index2);
            virtual eEvalStatus evalKeys(sVariant &outval, sVariant &dic);
            virtual eEvalStatus evalKV(sVariant &outval, sVariant &dic);
            // would evalSetSlice make sense or be useful?..

            // Comparison operators are syntactically valid for any lhs/rhs values!
            inline virtual bool evalEquality(sVariant &lhs, sVariant &rhs) { return lhs == rhs; }
            inline virtual bool evalLess(sVariant &lhs, sVariant &rhs) { return lhs < rhs; }
            inline virtual bool evalGreater(sVariant &lhs, sVariant &rhs) { return lhs > rhs; }
            inline virtual bool evalLessOrEqual(sVariant &lhs, sVariant &rhs) { return lhs <= rhs; }
            inline virtual bool evalGreaterOrEqual(sVariant &lhs, sVariant &rhs) { return lhs >= rhs; }

            virtual eEvalStatus evalHas(sVariant &outval, sVariant &lhs, sVariant *rhs, idx dimrhs);
            virtual eEvalStatus evalMatch(sVariant &outval, sVariant &val, const regex_t *re, idx nmatch, regmatch_t * pmatch);
            virtual eEvalStatus evalNMatch(sVariant &outval, sVariant &val, const regex_t *re);

            virtual eEvalStatus evalListUnion(sVariant &outval, sVariant *lists, idx nlists);
            virtual eEvalStatus evalListIntersection(sVariant &outval, sVariant *lists, idx nlists);

            // Override these in subclasses
            virtual bool evalValidObjectId(sVariant &obj) { return false; }
            virtual eEvalStatus evalHasProperty(sVariant &outval, sVariant &obj, const char *name) { return EVAL_NOT_IMPLEMENTED; }
            virtual eEvalStatus evalGetProperty(sVariant &outval, sVariant &obj, const char *name);
            virtual eEvalStatus evalGetProperty(sVariant &outval, const char *name);
            virtual eEvalStatus evalSetProperty(sVariant &obj, const char *name, sVariant &val);
            virtual eEvalStatus evalSetProperty(const char *name, sVariant &val);
            virtual eEvalStatus evalProps(sVariant &outval, sVariant &obj, sVariant *options=NULL) { return EVAL_NOT_IMPLEMENTED; }
            // Declare a new object ID (or list) - cache, validate, etc. if needed
            virtual void declareObjectId(sVariant &obj) {}
            virtual void declareObjlist(sVariant &objlist) {}

            virtual bool evalGetDollarNumValue(sVariant &outval, idx i) { return false; }
            virtual bool evalGetDollarNameValue(sVariant &outval, const char *s) { return false; }
        };

        //! Query language builtin function class
        class BuiltinFunction : public Callable {
        protected:
            sStr _name;
            bool checkTopicNone(Context &ctx, sVariant *topic) const;
            bool checkTopicString(Context &ctx, sVariant *topic) const;
            bool checkTopicList(Context &ctx, sVariant *topic) const;
            bool checkTopicListOrNull(Context &ctx, sVariant *topic) const;
            bool checkTopicSubscriptable(Context &ctx, sVariant *topic) const;
            bool checkTopicObjectId(Context &ctx, sVariant *topic) const;
            bool checkArgsCallable(Context &ctx, sVariant *args, idx nargs) const;
            bool checkNArgs(Context &ctx, idx nargs, idx min, idx max=-1) const;
        public:
            virtual const char *getName() const { return _name.ptr(); }
            virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const = 0;
            virtual ~BuiltinFunction() {}
        };
    };
};

#endif
