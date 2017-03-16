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
#include <assert.h>
#include <math.h>
#include <time.h>
#include <memory>
#include <slib/std/string.hpp>
#include <slib/utils/sort.hpp>
#include <ssci/bio/bioseq.hpp>
#include <ssci/math/clust/clust2.hpp>
#include <qlang/interpreter.hpp>

using namespace slib;
using namespace slib::qlang;

//! \cond 0

bool SourceLocation::print(sStr &s) const
{
    if (_line >= 0) {
        s.printf("at %" DEC ":%" DEC, _line, _col);
        return true;
    } else if (_col >= 0) {
        s.printf("at col %" DEC, _col);
        return true;
    }
    return false;
}

// keep in sync with qlang::eEvalStatus
static const char *ContextEvalStatusNames[] = {
    "success", // EVAL_SUCCESS = 1
    "error", // EVAL_OTHER_ERROR = 0,
    "not implemented", //EVAL_NOT_IMPLEMENTED
    "index out of range", // EVAL_RANGE_ERROR
    "invalid data type", // EVAL_TYPE_ERROR
    "expected list value", // EVAL_WANTED_LIST
    "expected dic value", // EVAL_WANTED_DIC
    "expected subscriptable value (list, dic, object, or string)", // EVAL_WANTED_SUBSCRIPTABLE
    "expected callable value (function or method)", // EVAL_WANTED_CALLABLE
    "expected object value", // EVAL_WANTED_OBJECT
    "wrong number of arguments", // EVAL_BAD_ARGS_NUMBER
    "permission error", // EVAL_PERMISSION_ERROR
    "attempted to write a read-only value", // EVAL_READ_ONLY_ERROR
    "syntax error", // EVAL_SYNTAX_ERROR
    "property not defined", // EVAL_NO_SUCH_PROP
    "invalid variable", // EVAL_VARIABLE_ERROR
    "system error", // EVAL_SYSTEM_ERROR
    NULL
};

const char *Context::statusName(eEvalStatus code) {
    return ContextEvalStatusNames[EVAL_SUCCESS - code];
}

void Context::Builtin::empty()
{
    if (type == BUILTIN_FUNCTION_OWNED)
        delete val.f;
    else if (type == BUILTIN_VARIANT_OWNED)
        delete val.pvar;

    sSet(this, 0);
}

Context::Context() : _rootScope(Scope::scope_LAMBDA)
{
    _curScope = &_rootScope;
    _curScope->incrementRefCount();
    _mode = MODE_NORMAL;
    registerDefaultBuiltins();
}

Context::~Context()
{
}

void Context::reset()
{
    _returnValue.setInt();
    _errorStr.cut(0);

    _rootScope.empty();
    _curScope = &_rootScope;
    _mode = MODE_NORMAL;
}

#define REGISTER_BOILERPLATE(b) \
Builtin * b = _builtins.get(name); \
if (b) \
    b->empty(); \
else \
    b = _builtins.set(name)

void Context::registerBuiltinIdxPtr(const char *name, idx *ptr, bool isConst)
{
    assert(ptr);
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_IDX_PTR;
    b->val.pi = ptr;
    b->isConst = isConst;
    sVariant::internString(name);
}

void Context::registerBuiltinIdxConst(const char *name, idx val)
{
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_IDX_VAL;
    b->val.i = val;
    b->isConst = true;
    sVariant::internString(name);
}

void Context::registerBuiltinUdxPtr(const char *name, udx *ptr, bool isConst)
{
    assert(ptr);
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_UDX_PTR;
    b->val.pu = ptr;
    b->isConst = isConst;
    sVariant::internString(name);
}

void Context::registerBuiltinUdxConst(const char *name, udx val)
{
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_UDX_VAL;
    b->val.u = val;
    b->isConst = true;
    sVariant::internString(name);
}

void Context::registerBuiltinRealPtr(const char *name, real *ptr, bool isConst)
{
    assert(ptr);
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_REAL_PTR;
    b->val.pr = ptr;
    b->isConst = isConst;
    sVariant::internString(name);
}

void Context::registerBuiltinRealConst(const char *name, real val)
{
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_REAL_VAL;
    b->val.r = val;
    b->isConst = true;
    sVariant::internString(name);
}

void Context::registerBuiltinStringPtr(const char *name, sStr *ptr, bool isConst)
{
    assert(ptr);
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_STRING_PTR;
    b->val.ps = ptr;
    b->isConst = isConst;
    sVariant::internString(name);
}

void Context::registerBuiltinFunction(const char *name, const BuiltinFunction &funcObj)
{
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_FUNCTION;
    b->val.f = &funcObj;
    b->isConst = true;
    sVariant::internString(name);
}

class RawCallbackWrapper : public BuiltinFunction {
protected:
    Context::BuiltinRawCallback _func;
    void * _param;
public:
    RawCallbackWrapper(const char *s, Context::BuiltinRawCallback func, void * param): _func(func), _param(param) { _name.printf(s); }
    RawCallbackWrapper(const RawCallbackWrapper &rhs): _func(rhs._func), _param(rhs._param) { _name.printf(rhs._name.ptr()); }
    virtual ~RawCallbackWrapper() {}
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        return _func(result, *this, ctx, topic, args, nargs, _param);
    }
};

void Context::registerBuiltinFunction(const char *name, BuiltinRawCallback funcPtr, void *param)
{
    assert(funcPtr);
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_FUNCTION_OWNED;
    b->val.f = new RawCallbackWrapper(name, funcPtr, param);
    b->isConst = true;
}

void Context::registerBuiltinGetter(const char *name, BuiltinGetter ptr, void *param)
{
    assert(ptr);
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_GETTER;
    b->val.g = ptr;
    b->param = param;
    b->isConst = true;
}

bool Context::registerBuiltinValue(const char *name, sVariant &value)
{
    REGISTER_BOILERPLATE(b);
    b->type = BUILTIN_VARIANT_OWNED;
    b->val.pvar = new sVariant(value);
    b->isConst = false;
    return true;
}

bool Context::copyBuiltin(const char *name, const char *oldname)
{
    Builtin * oldb = _builtins.get(oldname);
    if (!oldb)
        return false;

    REGISTER_BOILERPLATE(b);

    b->type = oldb->type;
    if (b->type == BUILTIN_FUNCTION_OWNED)
        b->val.f = new RawCallbackWrapper(*static_cast<const RawCallbackWrapper*>(oldb->val.f));
    else
        b->val = oldb->val;

    b->isConst = oldb->isConst;
    return true;
}

bool Context::removeBuiltin(const char *name)
{
    Builtin * b = _builtins.get(name);
    if (!b)
        return false;

    b->empty();
    return true;
}

void Context::pushScope(Scope::ScopeType type)
{
    _curScope = _curScope->addChild(type);
    _curScope->incrementRefCount();
}

void Context::popScope()
{
    _curScope->decrementRefCount();
    _curScope = _curScope->getParent();
    assert (_curScope);
}

bool Context::isScopeInLoop() const
{
    Scope *s = _curScope;
    while (s) {
        switch (s->getType()) {
        case Scope::scope_LAMBDA:
            return false;
        case Scope::scope_LOOP:
            return true;
        default:
            s = s->getParent();
        }
    }
    return false;
}

void Context::clearBreakContinue()
{
    if (_mode == MODE_BREAK || _mode == MODE_CONTINUE)
        _mode = MODE_NORMAL;
}

void Context::clearReturn(bool resetReturnValue/*=true*/)
{
    if (_mode == MODE_RETURN) {
        _mode = MODE_NORMAL;
        if (resetReturnValue)
            _returnValue.setNull();
    }
}

bool Context::setReturn(sVariant &val)
{
    if (_mode != MODE_NORMAL)
        return false;

    _mode = MODE_RETURN;
    _returnValue = val;
    return true;
}

bool Context::setBreak()
{
    if (_mode != MODE_NORMAL)
        return false;

    _mode = MODE_BREAK;
    return true;
}

bool Context::setContinue()
{
    if (_mode != MODE_NORMAL)
        return false;

    _mode = MODE_CONTINUE;
    return true;
}

bool Context::getVarValue(const char *name, sVariant &outval)
{
    if (Builtin *b = _builtins.get(name)) {
        switch (b->type) {
        case BUILTIN_IDX_PTR:
            outval.setInt(*(b->val.pi));
            return true;
        case BUILTIN_IDX_VAL:
            outval.setInt(b->val.i);
            return true;
        case BUILTIN_UDX_PTR:
            outval.setUInt(*(b->val.pu));
            return true;
        case BUILTIN_UDX_VAL:
            outval.setUInt(b->val.u);
            return true;
        case BUILTIN_REAL_PTR:
            outval.setReal(*(b->val.pr));
            return true;
        case BUILTIN_REAL_VAL:
            outval.setReal(b->val.r);
            return true;
        case BUILTIN_STRING_PTR:
            outval.setString(*(b->val.ps));
            return true;
        case BUILTIN_VARIANT_OWNED:
            outval = *(b->val.pvar);
            return true;
        case BUILTIN_GETTER:
            return b->val.g(outval, b->param);
        case BUILTIN_FUNCTION:
        case BUILTIN_FUNCTION_OWNED:
            CallableWrapper w(b->val.f);
            outval.setData(w);
            return true;
        }
        return false;
    }

    return _curScope->getValue(name, outval);
}

bool Context::addVarValue(const char *name, sVariant &val)
{
    // do not allow builtin values to be redefined
    if (_builtins.get(name))
        return false;

    return _curScope->addValue(name, val);
}

bool Context::setVarValue(const char *name, sVariant &val)
{
    if (Builtin *b = _builtins.get(name)) {
        if (b->isConst)
            return false;

        switch (b->type) {
        case BUILTIN_IDX_PTR:
            *(b->val.pi) = val.asInt();
            return true;
        case BUILTIN_IDX_VAL:
            b->val.i = val.asInt();
            return true;
        case BUILTIN_UDX_PTR:
            *(b->val.pu) = val.asUInt();
            return true;
        case BUILTIN_UDX_VAL:
            b->val.u = val.asUInt();
            return true;
        case BUILTIN_REAL_PTR:
            *(b->val.pr) = val.asReal();
            return true;
        case BUILTIN_REAL_VAL:
            b->val.r = val.asReal();
            return true;
        case BUILTIN_STRING_PTR:
            b->val.ps->cut(0);
            b->val.ps->printf("%s", val.asString());
            return true;
        case BUILTIN_VARIANT_OWNED:
            *(b->val.pvar) = val;
            return true;
        }
        // Do not allow builtin functions to be redefined
        return false;
    }

    return _curScope->setValue(name, val);
}

bool Context::isCallable(const sVariant &val)
{
    return val.isData() && dynamic_cast<const CallableWrapper*>(val.asData());
}

bool Context::setError(const SourceLocation &loc, eEvalStatus code, const char *fmt, ...)
{
    if (_mode == MODE_ERROR)
        return false;
    _errorLoc = loc;
    _errorCode = code;
    _errorStr.cut0cut();
    sCallVarg(_errorStr.vprintf, fmt);
    _mode = MODE_ERROR;
    return true;
}

bool Context::setError(eEvalStatus code, const char *fmt, ...)
{
    if (_mode == MODE_ERROR)
        return false;
    _errorCode = code;
    _errorStr.cut0cut();
    sCallVarg(_errorStr.vprintf, fmt);
    _mode = MODE_ERROR;
    return true;
}

const char * Context::printError(sStr &s)
{
    idx pos = s.length();

    if (_mode != MODE_ERROR) {
        s.add0cut();
        return s.ptr(pos);
    }

    if (_errorLoc.print(s))
        s.printf(": ");

    s.printf("%s; %s", statusName(_errorCode), _errorStr.ptr());
    return s.ptr(pos);
}

void Context::clearError()
{
    if (_mode == MODE_ERROR)
        _mode = MODE_NORMAL;
}

#define EVAL_BINARY_MATH(op) \
    if (lhs.isNumeric() || lhs.isDateOrTime()) \
        outval = lhs; \
    else \
        lhs.getNumericValue(outval); \
    \
    bool success; \
    if (rhs.isNumeric() || rhs.isDateOrTime()) { \
        success = outval op rhs; \
    } else { \
        sVariant rnumeric; \
        rhs.getNumericValue(rnumeric); \
        success = outval op rnumeric; \
    } \
    return success ? EVAL_SUCCESS : EVAL_TYPE_ERROR;

eEvalStatus Context::evalAdd(sVariant &outval, sVariant &lhs, sVariant &rhs)
{
    EVAL_BINARY_MATH(+=)
}

eEvalStatus Context::evalSubtract(sVariant &outval, sVariant &lhs, sVariant &rhs)
{
    EVAL_BINARY_MATH(-=)
}

eEvalStatus Context::evalMultiply(sVariant &outval, sVariant &lhs, sVariant &rhs)
{
    EVAL_BINARY_MATH(*=)
}

eEvalStatus Context::evalDivide(sVariant &outval, sVariant &lhs, sVariant &rhs)
{
    EVAL_BINARY_MATH(/=)
}

eEvalStatus Context::evalRemainder(sVariant &outval, sVariant &lhs, sVariant &rhs)
{
    EVAL_BINARY_MATH(%=)
}

#define ENSURE_NUMERIC_VAL(val, datetime_ok) \
do { \
    if ( !((val).isNumeric() || (datetime_ok && (val).isDateOrTime())) ) { \
        sVariant numeric; \
        (val).getNumericValue(numeric); \
        (val) = numeric; \
    } \
} while(0)

eEvalStatus Context::evalIncrement(sVariant &val)
{
    ENSURE_NUMERIC_VAL(val, 1);
    return (val += (idx)1) ? EVAL_SUCCESS : EVAL_TYPE_ERROR;
}

eEvalStatus Context::evalDecrement(sVariant &val)
{
    ENSURE_NUMERIC_VAL(val, 1);
    return (val -= (idx)1) ? EVAL_SUCCESS : EVAL_TYPE_ERROR;
}

eEvalStatus Context::evalUnaryPlus(sVariant &val)
{
    ENSURE_NUMERIC_VAL(val, 1);
    return EVAL_SUCCESS;
}

eEvalStatus Context::evalUnaryMinus(sVariant &val)
{
    ENSURE_NUMERIC_VAL(val, 0);
    return (val *= (idx)(-1)) ? EVAL_SUCCESS : EVAL_TYPE_ERROR;
}

static bool checkIndex(idx &i, idx listDim)
{
    if (i < 0)
        i += listDim;
    if (i < 0 || i >= listDim)
        return false;
    return true;
}

eEvalStatus Context::evalGetSubscript(sVariant &outval, sVariant &lhs, sVariant &index)
{
    if (lhs.isDic()) {
        if (!lhs.getElt(index.asString(), outval))
            outval.setNull();
        return EVAL_SUCCESS;
    } else if (lhs.isHiveId()) {
        return evalGetProperty(outval, lhs, index.asString());
    }

    if (!lhs.isList() && !lhs.isString()) {
        outval.setNull();
        return EVAL_SUCCESS;
    }

    idx i = index.asInt();
    if (!checkIndex(i, lhs.dim()) || !lhs.getElt(i, outval))
        outval.setNull();

    return EVAL_SUCCESS;
}

eEvalStatus Context::evalSetSubscript(sVariant &lhs, sVariant &index, sVariant &rhs)
{
    if (lhs.isDic()) {
        return lhs.setElt(index.asString(), rhs) ? EVAL_SUCCESS : EVAL_OTHER_ERROR;
    } else if (lhs.isHiveId()) {
        return evalSetProperty(lhs, index.asString(), rhs);
    }

    if (!lhs.isList())
        return EVAL_WANTED_SUBSCRIPTABLE;

    idx i = index.asInt();
    checkIndex(i, lhs.dim()); // ignore return value, let sVariant handle things

    return lhs.setElt(i, rhs) ? EVAL_SUCCESS : EVAL_OTHER_ERROR;
}

eEvalStatus Context::evalGetSlice(sVariant &outval, sVariant &lhs, sVariant &rhs1, sVariant &rhs2)
{
    if (!lhs.isList() && !lhs.isString()) {
        outval.setNull();
        return EVAL_SUCCESS;
    }

    idx i1 = rhs1.asInt();
    idx i2 = rhs2.asInt();
    checkIndex(i1, lhs.dim()); // ignore return value, let sVariant handle things
    checkIndex(i2, lhs.dim()); // ignore return value, let sVariant handle things

    idx step = i1 <= i2 ? 1 : -1;

    if (lhs.isList()) {
        outval.setList();
        for (idx j=i1; step*j <= step*i2; j += step) {
            if (j < 0 || j > lhs.dim())
                continue;

            sVariant v;
            if (!lhs.getElt(j, v))
                return EVAL_OTHER_ERROR;

            outval.push(v);
        }
    } else {
        // lhs is string
        const char *s = lhs.asString();
        idx dim = lhs.dim();
        sStr slice;
        for (idx j=i1; step*j <= step*i2; j += step) {
            if (j < 0 || j > dim)
                continue;
            slice.add(s + j, 1);
        }
        slice.add0();
        outval.setString(slice.ptr());
    }
    return EVAL_SUCCESS;
}

eEvalStatus Context::evalKeys(sVariant &outval, sVariant &lhs)
{
    if (lhs.isHiveId())
        return evalProps(outval, lhs);

    if (lhs.isList()) {
        outval.setList();
        for (idx i=0; i<lhs.dim(); i++)
            outval.push((idx)i);
    } else if (lhs.isDic()) {
        lhs.getDicKeys(outval);
    } else {
        return EVAL_WANTED_SUBSCRIPTABLE;
    }
    return EVAL_SUCCESS;
}

eEvalStatus Context::evalKV(sVariant &outval, sVariant &lhs)
{
    if (lhs.isHiveId()) {
        eEvalStatus status;
        sVariant props;
        if ((status = evalProps(props, lhs)) != EVAL_SUCCESS) {
            return status;
        }

        outval.setList();
        for (idx i=0; i<props.dim(); i++) {
            sVariant *key, pair, val;
            key = props.getListElt(i);
            if ((status = evalGetProperty(val, lhs, key->asString())) != EVAL_SUCCESS) {
                return status;
            }
            pair.setList();
            pair.push(*key);
            pair.push(val);
            outval.push(pair);
        }
    } else if (lhs.isList()) {
        outval.setList();
        for (idx i=0; i<lhs.dim(); i++) {
            sVariant pair;
            pair.setList();
            pair.push(i);
            pair.push(*lhs.getListElt(i));
            outval.push(pair);
        }
    } else if (lhs.isDic()) {
        outval.setList();
        for (idx i=0; i<lhs.dim(); i++) {
            sVariant pair, val;
            const char * key = lhs.getDicKeyVal(i, val);
            pair.setList();
            pair.push(key);
            pair.push(val);
            outval.push(pair);
        }
    } else {
        return EVAL_WANTED_SUBSCRIPTABLE;
    }
    return EVAL_SUCCESS;
}

eEvalStatus Context::evalHas(sVariant &outval, sVariant &lhs, sVariant *rhs, idx dimrhs)
{
    eEvalStatus status;
    if (lhs.isHiveId()) {
        for (idx j=0; j<dimrhs; j++) {
            if ((status = evalHasProperty(outval, lhs, rhs[j].asString())) != EVAL_SUCCESS)
                return status;
            if (outval.asBool())
                break;
        }
        return EVAL_SUCCESS;
    } else if (lhs.isDic()) {
        for (idx j=0; j<dimrhs; j++) {
            if (lhs.getDicElt(rhs[j].asString())) {
                outval.setInt(1);
                return EVAL_SUCCESS;
            }
        }
        outval.setInt(0);
        return EVAL_SUCCESS;
    } else if (lhs.isList()) {
        for (idx i=0; i<lhs.dim(); i++) {
            sVariant lhsi;
            lhs.getElt(i, lhsi);
            for (idx j=0; j<dimrhs; j++) {
                if (lhsi == rhs[j]) {
                    outval.setInt(1);
                    return EVAL_SUCCESS;
                }
            }
        }
        outval.setInt(0);
        return EVAL_SUCCESS;
    }
    return EVAL_TYPE_ERROR;
}

eEvalStatus Context::evalMatch(sVariant &outval, sVariant &val, const regex_t *re, idx nmatch, regmatch_t * pmatch)
{
    if (!val.isScalar())
        return EVAL_TYPE_ERROR;
    const char * sval = val.asString();
    if (regexec(re, sval, (size_t)nmatch, pmatch, 0) == 0) {
        if (nmatch) {
            outval.setList();
            for (idx i=0; i<nmatch && pmatch[i].rm_so >= 0; i++) {
                idx len = pmatch[i].rm_eo - pmatch[i].rm_so;
                outval.push(len ? sval + pmatch[i].rm_so : 0, len);
            }
        } else {
            outval.setInt(1);
        }
    } else {
        outval.setInt(0);
    }
    return EVAL_SUCCESS;
}

eEvalStatus Context::evalNMatch(sVariant &outval, sVariant &val, const regex_t *re)
{
    eEvalStatus status = evalMatch(outval, val, re, 0, NULL);
    if (status == EVAL_SUCCESS) {
        outval.setInt(!outval.asInt());
    }
    return status;
}

eEvalStatus Context::evalListUnion(sVariant &outval, sVariant *lists, idx nlists)
{
    outval.setList();

    sDic<sVariant*> dic;
    for (idx i=0; i<nlists; i++) {
        if (!lists[i].isList() && !lists[i].isNull())
            return EVAL_WANTED_LIST;

        for (idx j=0; j<lists[i].dim(); j++) {
            sVariant* pelt = lists[i].getListElt(j);
            assert(pelt);
            sStr key("%s:%s", pelt->getTypeName(), pelt->asString());
            if (!dic.get(key.ptr()))
                *(dic.set(key.ptr())) = pelt;
        }
    }

    for (idx i=0; i<dic.dim(); i++)
        outval.push(*(dic[i]));

    return EVAL_SUCCESS;
}

struct varcount {
    sVariant * pvar;
    idx cnt;
    varcount() { pvar = NULL; cnt = 1; }
};

eEvalStatus Context::evalListIntersection(sVariant &outval, sVariant *lists, idx nlists)
{
    outval.setList();

    sDic<varcount> dic;
    for (idx i=0; i<nlists; i++) {
        if (!lists[i].isList() && !lists[i].isNull())
            return EVAL_WANTED_LIST;

        for (idx j=0; j<lists[i].dim(); j++) {
            sVariant* pelt = lists[i].getListElt(j);
            assert(pelt);
            sStr key("%s:%s", pelt->getTypeName(), pelt->asString());
            varcount *vc = dic.get(key.ptr());
            if (vc)
                vc->cnt++;
            else
                dic.set(key.ptr())->pvar = pelt;
        }
    }

    for (idx i=0; i<dic.dim(); i++)
        if (dic[i].cnt == nlists)
            outval.push(*(dic[i].pvar));

    return EVAL_SUCCESS;
}

eEvalStatus Context::evalGetProperty(sVariant &outval, sVariant &obj, const char *name)
{
    if (obj.isDic()) {
        if (!obj.getElt(name, outval))
            outval.setNull();
        return EVAL_SUCCESS;
    }
    outval.setNull();
    return EVAL_SUCCESS;
}

eEvalStatus Context::evalGetProperty(sVariant &outval, const char *name)
{
    sVariant obj;
    if (!getVarValue("this", obj)) {
        setError(EVAL_VARIABLE_ERROR, "'this' is not defined in the current context");
        return EVAL_VARIABLE_ERROR;
    }
    return evalGetProperty(outval, obj, name);
}

eEvalStatus Context::evalSetProperty(sVariant &obj, const char *name, sVariant &val)
{
    if (obj.isDic()) {
        return obj.setElt(name, val) ? EVAL_SUCCESS : EVAL_OTHER_ERROR;
    }
    return EVAL_NOT_IMPLEMENTED;
}

eEvalStatus Context::evalSetProperty(const char *name, sVariant &val)
{
    sVariant obj;
    if (!getVarValue("this", obj)) {
        setError(EVAL_VARIABLE_ERROR, "'this' is not defined in the current context");
        return EVAL_VARIABLE_ERROR;
    }
    return evalSetProperty(obj, name, val);
}

/* Default builtins */

bool BuiltinFunction::checkTopicNone(Context &ctx, sVariant *topic) const
{
    if (topic) {
        ctx.setError(EVAL_SYNTAX_ERROR, "%s does not take a topic value", getName());
        return false;
    }
    return true;
}

bool BuiltinFunction::checkTopicString(Context &ctx, sVariant *topic) const
{
    if (!topic || !topic->isString()) {
        ctx.setError(EVAL_SYNTAX_ERROR, "%s requires a string value as topic", getName());
        return false;
    }
    return true;
}

bool BuiltinFunction::checkTopicList(Context &ctx, sVariant *topic) const
{
    if (!topic || !topic->isList()) {
        ctx.setError(EVAL_SYNTAX_ERROR, "%s requires a list value as topic", getName());
        return false;
    }
    return true;
}

bool BuiltinFunction::checkTopicListOrNull(Context &ctx, sVariant *topic) const
{
    if (!topic || (!topic->isList() && !topic->isNull())) {
        ctx.setError(EVAL_SYNTAX_ERROR, "%s requires a list or null value as topic", getName());
        return false;
    }
    return true;
}

bool BuiltinFunction::checkTopicSubscriptable(Context &ctx, sVariant *topic) const
{
    if (!topic || (!topic->isList() && !topic->isString())) {
        ctx.setError(EVAL_SYNTAX_ERROR, "%s requires a subscriptable (string or list) value as topic", getName());
        return false;
    }
    return true;
}

bool BuiltinFunction::checkTopicObjectId(Context &ctx, sVariant *topic) const
{
    if (!topic || !topic->isHiveId()) {
        ctx.setError(EVAL_SYNTAX_ERROR, "%s requires an object ID value as topic", getName());
        return false;
    }
    return true;
}

bool BuiltinFunction::checkArgsCallable(Context &ctx, sVariant *args, idx nargs) const
{
    for (idx i=0; i<nargs; i++)
        if (!dynamic_cast<CallableWrapper*>(args[i].asData())) {
            ctx.setError(EVAL_TYPE_ERROR, "%s requires all arguments to be functions", getName());
            return false;
        }
    return true;
}

bool BuiltinFunction::checkNArgs(Context &ctx, idx nargs, idx min, idx max) const
{
    if (max < 0) {
        if (nargs < min) {
            ctx.setError(EVAL_SYNTAX_ERROR, "%s requires at least %" DEC " arguments", getName(), min);
            return false;
        }
    } else if (min < 0) {
        if (nargs > max) {
            ctx.setError(EVAL_SYNTAX_ERROR, "%s requires at most %" DEC " arguments", getName(), max);
            return false;
        }
    } else if (max == min) {
        if (nargs < min || nargs > max) {
            ctx.setError(EVAL_SYNTAX_ERROR, "%s requires exactly %" DEC " arguments", getName(), min);
            return false;
        }
    } else {
        if (nargs < min || nargs > max) {
            ctx.setError(EVAL_SYNTAX_ERROR, "%s requires at between %" DEC " and %" DEC " arguments", getName(), min, max);
            return false;
        }
    }
    return true;
}

#define BASIC_BUILTIN(name, numargs, code) \
class sQLangBuiltin_ ## name : public BuiltinFunction { \
public: \
    sQLangBuiltin_ ## name() { _name.printf("builtin " #name "() function"); } \
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const \
    { \
        if (!checkTopicNone(ctx, topic) || !checkNArgs(ctx, nargs, numargs, numargs)) \
            return false; \
        code \
        return true; \
    } \
}

#define BASIC_UNARY_BUILTIN(name, code) \
class sQLangBuiltin_ ## name : public BuiltinFunction { \
public: \
    sQLangBuiltin_ ## name() { _name.printf("builtin " #name "() function"); } \
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const \
    { \
        if (topic) { \
            if (!checkNArgs(ctx, nargs, 0, 0)) \
                return false; \
        } else { \
            if (!checkNArgs(ctx, nargs, 1, 1)) \
                return false; \
            topic = args; \
        } \
        code \
        return true; \
    } \
}

#define BASIC_NUMERIC_LIST_BUILTIN(name, code_start, code_loop, code_end) \
class sQLangBuiltin_ ## name : public BuiltinFunction { \
public: \
    sQLangBuiltin_ ## name() { _name.printf("builtin " #name "() function"); } \
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const \
    { \
        if (topic && !checkTopicListOrNull(ctx, topic)) \
            return false; \
        result.setInt(); \
        if (!topic && nargs == 1 && args[0].isList()) \
            topic = args; \
        idx dim = topic ? topic->dim() : nargs; \
        sVariant numericVal; \
        code_start \
        for (idx i=0; i<dim; i++) { \
            if (topic) \
                topic->getListElt(i)->getNumericValue(numericVal); \
            else \
                args[i].getNumericValue(numericVal); \
            code_loop \
        } \
        code_end \
        return true; \
    } \
}

//! \endcond

/*! \page qlang_builtin_aa1letter aa1letter()
Finds the NCBI 1-letter code for an amino acid name or 3-letter code (case-insensitive)
\code
    aa1letter("Glutamine") // returns "Q"
    aa1letter("Gln") // returns "Q"
\endcode */
BASIC_BUILTIN(aa1letter, 1,
    {
        const char * name = args[0].asString();
        idx len = sLen(name);
        for (sBioseq::ProtAA * aa = sBioseq::getListAA(); aa->nm; aa++) {
            if ((len == 3 && strcasecmp(name, aa->let3) == 0) ||
                (len > 3 && strcasecmp(name, aa->nm) == 0))
            {
                result.setString(aa->let);
                return true;
            }
        }
        result.setNull();
    });

/*! \page qlang_builtin_aa3letter aa3letter()
Finds the standard 3-letter code for an amino acid name or NCBI 1-letter code (case-insensitive)
\code
    aa1letter("Q") // returns "Gln"
    aa1letter("Glutamine") // returns "Gln"
\endcode */
BASIC_BUILTIN(aa3letter, 1,
    {
        const char * name = args[0].asString();
        idx len = sLen(name);
        for (sBioseq::ProtAA * aa = sBioseq::getListAA(); aa->nm; aa++) {
            if ((len == 1 && strcasecmp(name, aa->let) == 0) ||
                (len > 1 && strcasecmp(name, aa->nm) == 0))
            {
                result.setString(aa->let3);
                return true;
            }
        }
        result.setNull();
    });

/*! \page qlang_builtin_abs abs()
Calculates the absolute value of a numeric argument
\code
    abs(-10) // returns 10
\endcode */
BASIC_BUILTIN(abs, 1,
    {
        sVariant numericVal;
        args[0].getNumericValue(numericVal);
        result = (numericVal.asReal() < 0) ? (numericVal *= (idx)(-1), numericVal) : numericVal;
    });

/*! \page qlang_builtin_append append()
Appends to a list; if topic is provided, it will be modified in place
\code
    [1, 2, 3].append([4, 5], [6, [7]]); // returns [1, 2, 3, 4, 5, 6, [7]]
\endcode
\see \ref qlang_builtin_push */
class sQLangBuiltin_append : public BuiltinFunction {
public:
    sQLangBuiltin_append() { _name.printf("builtin append() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic) {
            if (!checkTopicListOrNull(ctx, topic)) {
                return false;
            }
            result = *topic;
        }

        if (!result.isList()) {
            result.setList();
        }

        for(idx i = 0; i < nargs; i++) {
            if( args[i].isNull() ) {
                continue;
            } else if( args[i].isList() ) {
                if( !result.append(args[i]) ) {
                    ctx.setError(EVAL_OTHER_ERROR, "%s failed to append argument #%" DEC " to topic", getName(), i + 1);
                    return false;
                }
            } else {
                ctx.setError(EVAL_TYPE_ERROR, "%s requires all arguments to be lists or nulls", getName());
            }
        }
        return true;
    }
};

/*! \page qlang_builtin_apply apply()
Apply a function to a list of arguments.

Can be called as apply($FUNC, [$THIS], [$ARG_LIST]) or $ARG_LIST.apply($FUNC, [$THIS]).

Takes an optional callback argument which takes each value as topic, and the returns are counted.
\code
    some_list.apply(f, t); // equivalent to t.f(some_list[0], some_list[1], ...)
    apply(f, t, some_list); // same as above
    [[1, 2], [3, 4]].apply(concat); // returns [1, 2, 3, 4]
\endcode */
class sQLangBuiltin_apply : public BuiltinFunction {
    public:
        sQLangBuiltin_apply() { _name.printf("builtin apply() function"); }
        virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
        {
            if( !topic ) {
                if( nargs > 2 ) {
                    topic = args + 2;
                    nargs = 2;
                }
            }
            if( !checkNArgs(ctx, nargs, 1, 2) || !checkArgsCallable(ctx, args, 1) ) {
                return false;
            }
            CallableWrapper * callable = dynamic_cast<CallableWrapper*>(args[0].asData());
            sVariant * callable_topic = nargs > 1 ? args + 1 : 0;
            sVec<sVariant> callable_args;
            if( !callable ) {
                ctx.setError(EVAL_TYPE_ERROR, "%s requires first argument to be a function", getName());
                return false;
            }
            if( topic ) {
                if( topic->isList() ) {
                    callable_args.resize(topic->dim());
                    for(idx i=0; i<topic->dim(); i++) {
                        callable_args[i] = *topic->getListElt(i);
                    }
                } else {
                    callable_args.resize(1);
                    callable_args[0] = *topic;
                }
            }
            callable->call(result, ctx, callable_topic, callable_args.ptr(), callable_args.dim());
            return true;
        }
};

/*! \page qlang_builtin_cat cat()
Concatenates strings. Used frequently, since in the HIVE query language, the \a + operator is purely numeric.
\code
    cat("hello", " ", "world"); // returns "hello world"
    [1, 2, 3].cat(); // returns "123"
\endcode
\see \ref qlang_builtin_join */
class sQLangBuiltin_cat : public BuiltinFunction {
public:
    sQLangBuiltin_cat() { _name.printf("builtin cat() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic && !checkTopicListOrNull(ctx, topic))
            return false;
        sStr s;
        if (!topic && nargs == 1 && args[0].isList())
            topic = args;
        idx dim = topic ? topic->dim() : nargs;
        for (idx i=0; i<dim; i++) {
            if (topic)
                s.addString(topic->getListElt(i)->asString());
            else
                s.addString(args[i].asString());
        }
        result.setString(s.ptr());
        return true;
    }
};

/*! \page qlang_builtin_ceil ceil()
Finds the ceiling of a numerical argument (i.e. rounds up)
\code
    ceil(1.5); // returns 2.0
\endcode
\see \ref qlang_builtin_floor */
BASIC_BUILTIN(ceil, 1, { result.setInt((idx)ceil(args[0].asReal())); } );

/*! \page qlang_builtin_clamp clamp()
Clamp a value (or list of values) within a range
\code
    clamp(-2, -1, 1); // returns -1
    clamp([0, 1, 2], -1, 1); // returns [0, 1, 1]
    [1, 2, 3, 4].clamp(2, 3); // returns [2, 2, 3, 3]
\endcode
\see \ref qlang_builtin_min, \ref qlang_builtin_max */
class sQLangBuiltin_clamp : public BuiltinFunction {
public:
    sQLangBuiltin_clamp() { _name.printf("builtin clamp() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic) {
            if (!checkNArgs(ctx, nargs, 2, 2))
                return false;
        } else {
            if (!checkNArgs(ctx, nargs, 3, 3))
                return false;
            topic = args;
            nargs--;
            args++;
        }

        if (args[0] > args[1]) {
            sVariant tmp = args[0];
            args[0] = args[1];
            args[1] = tmp;
        }

        if (topic->isList()) {
            result.setList();
            for (idx i=0; i<topic->dim(); i++) {
                sVariant * elt = topic->getListElt(i);
                if (*elt < args[0]) {
                    result.push(args[0]);
                } else if (*elt > args[1]) {
                    result.push(args[1]);
                } else {
                    result.push(*elt);
                }
            }
        } else {
            if (*topic < args[0]) {
                result = args[0];
            } else if (*topic > args[1]) {
                result = args[1];
            } else {
                result = *topic;
            }
        }

        return true;
    }
};

//! \cond 0
class HierarchicalClusteringData : public sVariant::sVariantData
{
protected:
    idx _refCount;
    sHierarchicalClustering * _clust;
    sVariant _labels;
    static const char * _name;

    static idx labelPrintCallback(sStr & out, sHierarchicalClustering & clust, idx x, void * param)
    {
        sVariant * labels = static_cast<sVariant*>(param);
        if (x < labels->dim()) {
            sStr tmp;
            tmp.printf(labels->getListElt(x)->asString());
            sString::searchAndReplaceSymbols(&out, tmp, tmp.length(), "(),: \t\r\n", "_", 0, true, false, false, false);
            out.shrink00();
        } else {
            out.printf("%" DEC, x);
        }
        return 0;
    }

public:
    HierarchicalClusteringData(sHierarchicalClustering * clust = 0, sVariant * labels = 0) : _refCount(0), _clust(clust)
    {
        if (labels)
            _labels = *labels;
    }
    virtual HierarchicalClusteringData* clone()
    {
        incrementRefCount();
        return this;
    }
    virtual ~HierarchicalClusteringData() { delete _clust; }
    virtual void incrementRefCount() { _refCount++; }
    virtual void decrementRefCount() { _refCount--; }
    virtual bool hasRefs() const { return _refCount > 0; }
    virtual bool asBool() const { return asInt(); }
    virtual idx asInt() const { return _clust ? _clust->dim() : 0; }
    virtual real asReal() const { return asInt(); }
    virtual void getNumericValue(sVariant &numeric) const { numeric.setInt(asInt()); }
    virtual void print(sStr &s, sVariant::ePrintMode mode, const sVariant::Whitespace * whitespace, idx indent_level) const
    {
        if (!_clust)
            return;

        sStr tmp;
        sStr & out = mode == sVariant::eDefault ? s : tmp;

        _clust->printNewick(out, sHierarchicalClusteringTree::Newick_PRINT_DISTANCE|sHierarchicalClusteringTree::Newick_PRINT_LEAF_NAMES, labelPrintCallback, const_cast<sVariant*>(&_labels));
        if (mode == sVariant::eCSV) {
            sString::escapeForCSV(s, tmp);
        } else if (mode == sVariant::eJSON) {
            sString::escapeForJSON(s, tmp);
        }
    }
    virtual const char * getTypeName() const { return _name; }
    virtual bool operator==(const sVariant::sVariantData &rhs_) const
    {
        const HierarchicalClusteringData * rhs = dynamic_cast<const HierarchicalClusteringData*>(&rhs_);
        if (!rhs)
            return false;

        return _clust == rhs->_clust;
    }
    virtual bool operator<(const sVariant::sVariantData &rhs_) const
    {
        const HierarchicalClusteringData * rhs = dynamic_cast<const HierarchicalClusteringData*>(&rhs_);
        if (!rhs)
            return this < static_cast<const void *>(&rhs_);

        return _clust < rhs->_clust;
    }
    virtual bool operator>(const sVariant::sVariantData &rhs_) const
    {
        const HierarchicalClusteringData * rhs = dynamic_cast<const HierarchicalClusteringData*>(&rhs_);
        if (!rhs)
            return this > static_cast<const void *>(&rhs_);

        return _clust > rhs->_clust;
    }
};
const char * HierarchicalClusteringData::_name = "hierarchical clustering";

class VariantRealListIter : public sIter<real, VariantRealListIter>
{
protected:
    idx _i, _dim;
    sVariant * _v;
public:
    VariantRealListIter(sVariant *v = 0, idx vDim = 0) {
        _v = v;
        if (_v && !_v->isList())
            _v = 0;

        _i = 0;
        _dim = vDim > 0 ? vDim : _v ? _v->dim() : 0;
    }
    VariantRealListIter(const VariantRealListIter &rhs): _i(rhs._i), _dim(rhs._dim), _v(rhs._v) {}

    inline void requestData_impl() {}
    inline void releaseData_impl() {}
    inline bool readyData_impl() const { return true; }
    bool validData_impl() const { return _i < _dim; }
    inline idx pos_impl() const { return _i; }
    inline idx segment_impl() const { return 0; }
    inline idx segmentPos_impl() const { return pos_impl(); }
    VariantRealListIter* clone_impl() const { return new VariantRealListIter(*this); }
    VariantRealListIter& increment_impl() { _i++; return *this; }
    bool equals_impl(const VariantRealListIter &rhs) const { return _v == rhs._v && _i == rhs._i; }
    bool lessThan_impl(const VariantRealListIter &rhs) const { return _v == rhs._v && _i < rhs._i; }
    bool greaterThan_impl(const VariantRealListIter &rhs) const { return _v == rhs._v && _i > rhs._i; }
    real dereference_impl() const {
        if (!_v || _i >= _v->dim())
            return 0;
        real val = _v->getListElt(_i)->asReal();
        return isnan(val) ? 0 : val;
    }
};
//! \endcond

class sQLangBuiltin_clust : public BuiltinFunction {
public:
    typedef VariantRealListIter Titer;
    typedef sDist<real, Titer> Tdist;

    sQLangBuiltin_clust() { _name.printf("builtin clust() function"); }

    Tdist * makeDist(const char * name) const
    {
        if (name) {
            if (!strcmp(name, "manhattan")) {
                return new sManhattanDist<real,Titer>();
            } else if (!strcmp(name, "maximum")) {
                return new sMaximumDist<real,Titer>();
            } else if (!strcmp(name, "canberra")) {
                return new sCanberraDist<real,Titer>();
            } else if (!strcmp(name, "cosine")) {
                return new sCosineDist<real,Titer>();
            }
        }
        // "euclidean" is the default
        return new sEuclideanDist<real,Titer>();
    }

    sHierarchicalClustering * makeClust(const char * name) const
    {
        if (name) {
            if (!strcmp(name, "single-link")) {
                return new sSingleLinkClustering();
            } else if (!strcmp(name, "complete-link")) {
                return new sCompleteLinkClustering();
            } else if (!strcmp(name, "fast-neighbor-joining")) {
                return new sFastNeighborJoining();
            }
        }
        // "neighbor-joining" is the default
        return new sNeighborJoining();
    }

    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicNone(ctx, topic) || !checkNArgs(ctx, nargs, 1, 1))
            return false;

        if (!args->isDic()) {
            ctx.setError(EVAL_TYPE_ERROR, "%s expects a dic argument", getName());
            return false;
        }

        const char * distalgo = args->getDicElt("distalgo") ? args->getDicElt("distalgo")->asString() : 0;
        const char * clustalgo = args->getDicElt("clustalgo") ? args->getDicElt("clustalgo")->asString() : 0;

        sVariant * activity = args->getDicElt("activity");
        sVariant * distance = args->getDicElt("distance");
        sVariant * labels = args->getDicElt("labels");
        idx dim = 0;

        sHierarchicalClustering * clust = makeClust(clustalgo);
        HierarchicalClusteringData * clustdata = new HierarchicalClusteringData(clust, labels);
        result.setData(*clustdata);

        // Do we already have a distance matrix?
        if (distance && distance->isList()) {
            dim = distance->dim();
            for (idx i=0; i<distance->dim(); i++)
                dim = sMax<idx>(dim, distance->getListElt(i)->dim());

            clust->reset(dim);
            for (idx i=0; i<dim; i++) {
                for (idx j=i+1; j<dim; j++) {
                    sVariant * prow = distance->getListElt(i);
                    sVariant * pcell = prow ? prow->getListElt(j) : 0;
                    // Do not allow NAN values to avoid possible fp exceptions
                    real d =  pcell ? pcell->asReal() : 0;
                    clust->setDist(i, j, isnan(d) ? 0 : d);
                }
            }
        } else if (activity && activity->isList()) {
            // Calculate distances from the activity matrix
            dim = activity->dim();
            std::auto_ptr<Tdist> dist(makeDist(distalgo));
            sVec<Titer> iters;
            iters.resize(dim);

            idx dim_measurables = 0;
            for (idx i=0; i<dim; i++) {
                dim_measurables = sMax<idx>(dim_measurables, activity->getListElt(i)->dim());
            }

            for (idx i=0; i<dim; i++) {
                Titer it(activity->getListElt(i), dim_measurables);
                iters[i] = it;
            }

            clust->resetDistance<real, Titer>(iters.ptr(), dim, *dist);
        } else {
            result.setNull();
            ctx.setError(EVAL_TYPE_ERROR, "%s expects a dic argument containing either 'distance' (distance matrix) or 'activity' (activity matrix) field", getName());
            return false;
        }

        clust->recluster();

        return true;
    }
};

/*! \page qlang_builtin_counts counts()
Counts the number of times each value occurs.

Takes an optional callback argument which takes each value as topic, and the returns are counted.
\code
    ["a", "b", "c", "a"].counts(); // returns {"a": 2, "b": 1, "c": 1}
    [5, 10, -5].counts({abs(this)}); // returns {"5": 2, "10": 1}
\endcode */
class sQLangBuiltin_counts : public BuiltinFunction {
public:
    sQLangBuiltin_counts() { _name.printf("builtin counts() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicListOrNull(ctx, topic) || !checkNArgs(ctx, nargs, 0, 1))
            return false;

        if (nargs && !checkArgsCallable(ctx, args, nargs))
            return false;

        sDic<idx> counts;
        for (idx i=0; i<topic->dim(); i++) {
            sVariant elt;
            topic->getElt(i, elt);
            if (nargs) {
                sVariant tmp;
                if (!static_cast<CallableWrapper*>(args[0].asData())->call(tmp, ctx, &elt, NULL, 0)) {
                    ctx.setError(EVAL_OTHER_ERROR, "%s call to argument on topic list element #%" DEC " failed", getName(), i);
                    return false;
                }
                elt = tmp;
            }

            const char *key = elt.asString();
            if (idx *count = counts.get(key))
                (*count)++;
            else
                *(counts.set(key)) = 1;
        }

        result.setDic();
        for (idx i=0; i<counts.dim(); i++) {
            sVariant countval(counts[i]);
            result.setElt((const char *)(counts.id(i)), countval);
        }

        return true;
    }
};

/*! \page qlang_builtin_dic dic()
Creates a dictionary.

\code
    dic(); // returns an empty dictionary
    ["a", "b", "c", "d"].dic(); // returns {"a": "b", "c": "d"}
    (12345 as obj).dic(); // returns {"id": 12345, "_type": "foo", "created": 1366648266, ...}
\endcode */
class sQLangBuiltin_dic : public BuiltinFunction {
public:
    sQLangBuiltin_dic() { _name.printf("builtin dic() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkNArgs(ctx, nargs, 0, 0))
            return false;

        result.setDic();

        if (!topic)
            return true;

        if (topic->isList()) {
            for (idx i=0; i<topic->dim(); i+=2) {
                sVariant key, value;
                topic->getElt(i, key);
                topic->getElt(i+1, value);
                result.setElt(key.asString(), value);
            }
        } else if (topic->isHiveId()) {
            eEvalStatus status;
            sVariant props;
            if ((status = ctx.evalKeys(props, *topic)) != EVAL_SUCCESS) {
                sStr s;
                ctx.setError(status, "%s failed to get properties list for %s", getName(), topic->print(s));
                return false;
            }
            for (idx i=0; i<props.dim(); i++) {
                sVariant key;
                props.getElt(i, key);
                sVariant val;
                if ((status = ctx.evalGetProperty(val, *topic, key.asString())) != EVAL_SUCCESS) {
                    sStr s;
                    ctx.setError(status, "%s failed to get property \"%s\" for %s", getName(), key.asString(), topic->print(s));
                    return false;
                }
                result.setElt(key.asString(), val);
            }
        } else {
            ctx.setError(EVAL_SYNTAX_ERROR, "%s requires a list or object ID value as topic", getName());
            return false;
        }
        return true;
    }
};

/*! \page qlang_builtin_dim dim()
Finds the number of elements in a list, keys in a dictionary, or symbols in a string. \a dim() of a non-string scalar value is zero. If given a topic, treats it as the argument.

\code
    "hello".dim(); // returns 5
    dim("hello"); // returns 5
    ["a", "b", "c"].dim(); // returns 3
    {"a": 0, "b": 0}.dim(); // returns 2
    dim(123.4); // returns 0
\endcode */
BASIC_UNARY_BUILTIN(dim, { result.setInt(topic->dim()); });

/*! \page qlang_builtin_exp exp()
Calculates exponentials
\code
    exp(1) // returns 2.718282
\endcode */
BASIC_BUILTIN(exp, 1, { result.setReal(exp(args[0].asReal())); });

/*! \page qlang_builtin_filter filter()
Filters a list by running a callback on each element as a topic.
\code
    [1, 2, -3, 4].filter({this > 0}); // returns [1, 2, 4]
\endcode
\see \ref qlang_builtin_foreach, \ref qlang_builtin_map, \ref qlang_builtin_reduce */
class sQLangBuiltin_filter : public BuiltinFunction {
public:
    sQLangBuiltin_filter() { _name.printf("builtin filter() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicListOrNull(ctx, topic) || !checkArgsCallable(ctx, args, nargs))
            return false;

        idx dim = topic->dim();
        result.setList();
        sVariant argResult, topicElt;
        for (idx i=0; i<dim; i++) {
            topic->getElt(i, topicElt);
            for (idx j=0; j<nargs; j++) {
                if (!static_cast<CallableWrapper*>(args[j].asData())->call(argResult, ctx, &topicElt, NULL, 0)) {
                    ctx.setError(EVAL_OTHER_ERROR, "%s call to argument #%" DEC " on topic list element #%" DEC " failed", getName(), j+1, i);
                    return false;
                }
                if (!argResult.asBool())
                    break;
            }
            if (argResult.asBool())
                result.push(topicElt);
        }
        return true;
    }
};

/*! \page qlang_builtin_floor floor()
Finds the floor of a numerical argument (i.e. rounds down)
\code
    floor(1.5); // returns 1.0
\endcode
\see \ref qlang_builtin_ceil */
BASIC_BUILTIN(floor, 1, { result.setInt((idx)floor(args[0].asReal())); } );

/*! \page qlang_builtin_foreach foreach()
Runs a callback on each element of a list as a topic.
\code
    a = dic(); [1,2,3].foreach({a[this] = this}); // a == {"1": 1, "2": 2, "3": 3}
\endcode
\see \ref qlang_builtin_filter, \ref qlang_builtin_map, \ref qlang_builtin_reduce */
class sQLangBuiltin_foreach : public BuiltinFunction {
public:
    sQLangBuiltin_foreach() { _name.printf("bultin foreach() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicListOrNull(ctx, topic) || !checkNArgs(ctx, nargs, 1, 1) || !checkArgsCallable(ctx, args, nargs))
            return false;

        sVariant argResult;
        for (idx i=0; i<topic->dim(); i++) {
            if (!static_cast<CallableWrapper*>(args[0].asData())->call(argResult, ctx, topic->getListElt(i), NULL, 0)) {
                ctx.setError(EVAL_OTHER_ERROR, "%s: call to argument on topic list element #%" DEC " failed", getName(), i);
                return false;
            }
            result.setInt(i+1);
        }
        return true;
    }
};

// list.histogram(numBins, min?, max?) OR histogram(list, numBins, min?, max?)
class sQLangBuiltin_histogram : public BuiltinFunction {
public:
    sQLangBuiltin_histogram() { _name.printf("builtin histogram() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        idx numBins;
        real min = NAN;
        real max = NAN;

        if (topic) {
            if (!checkTopicList(ctx, topic) || !checkNArgs(ctx, nargs, 1, 3))
                return false;

            numBins = args[0].asInt();
            if (nargs >= 2)
                min = args[1].asReal();
            if (nargs >= 3)
                max = args[2].asReal();

        } else {
            if (!checkNArgs(ctx, nargs, 2, 4))
                return false;

            topic = args;
            if (!topic->isList()) {
                ctx.setError(EVAL_WANTED_LIST, "%s: expected numeric list as first argument", getName());
                return false;
            }
            numBins = args[1].asInt();
            if (nargs >= 3)
                min = args[2].asReal();
            if (nargs >= 4)
                max = args[3].asReal();
        }

        if (numBins < 1) {
            ctx.setError(EVAL_RANGE_ERROR, "%s: number of bins must be positive", getName());
            return false;
        }

        result.setList();
        for (idx ib=0; ib<numBins; ib++)
            result.push((idx)0);

        if (!topic->dim())
            return true;

        if (isnan(min) || isnan(max) || min >= max) {
            real tmp_min = INFINITY;
            real tmp_max = -INFINITY;
            for (idx i=0; i<topic->dim(); i++) {
                real val = topic->getListElt(i)->asReal();
                tmp_min = sMin<real>(tmp_min, val);
                tmp_max = sMax<real>(tmp_max, val);
            }
            min = tmp_min;
            max = tmp_max;
        }

        real width = (max - min) / numBins;

        if (isnan(width) || width <= 0)
            return true;

        for (idx i=0; i<topic->dim(); i++) {
            real val = topic->getListElt(i)->asReal();
            idx ib = (val == max) ? (numBins - 1) : floor((val - min) / width);
            if (ib < 0 || ib >= numBins)
                continue;
            *(result.getListElt(ib)) += (idx)1;
        }
        return true;
    }
};

/*! \page qlang_builtin_intersect intersect()
Finds the intersection of lists
\code
    intersect([1, 2, 3, 4], [2, 4], [4, 2, 42]); // returns [2, 4]
\endcode
\see \ref qlang_builtin_union */
class sQLangBuiltin_intersect : public BuiltinFunction {
public:
    sQLangBuiltin_intersect() { _name.printf("builtin intersect() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicNone(ctx, topic))
            return false;

        eEvalStatus status;
        if ((status = ctx.evalListIntersection(result, args, nargs)) != EVAL_SUCCESS) {
            ctx.setError(status, "%s failed", getName());
            return false;
        }
        return true;
    }
};

/*! \page qlang_builtin_isnull isnull()
Checks if topic or argument is the special \a null value, which is normally interpreted as
zero in numeric context and to the empty string in string context.
\code
    null.isnull(); // returns true
    isnull(null); // returns true
    isnull(false); // returns false
    isnull(0); // returns false
    isnull(""); // returns false
\endcode */
BASIC_UNARY_BUILTIN(isnull, { result.setInt(topic->isNull()); });

/*! \page qlang_builtin_isnumeric isnumeric()
Checks if topic or argument is a numeric scalar
\code
    PI.isnumeric(); // returns true
    isnumeric(PI); // returns true
    isnumeric("3.14"); // returns false
\endcode */
BASIC_UNARY_BUILTIN(isnumeric, { result.setInt(topic->isNumeric()); });

/*! \page qlang_builtin_isstring isstring()
Checks if topic or argument is a string scalar
\code
    s = "hello"; s.isstring(); // returns true
    isstring("hello"); // returns true
    ["hello", "bye"].isstring(); // returns false
\endcode */
BASIC_UNARY_BUILTIN(isstring, { result.setInt(topic->isString()); });

/*! \page qlang_builtin_isobj isobj()
Checks if topic or argument is an object ID
\code
    (12345 as obj).isobj(); // returns true
    isobj(12345 as obj); // returns true
    isobj(12345); // returns false
\endcode

\note \a isobj() does not check whether the object ID is a \em valid object.
For that, use \ref qlang_uquery_builtin_validobj, which is only available in
slib::qlang::sUsrContext and subclasses. */
BASIC_UNARY_BUILTIN(isobj, { result.setInt(topic->isHiveId()); });

/*! \page qlang_builtin_isdatetime isdatetime()
Checks if topic or argument is a date-time
\code
    isdatetime("2014-01-01 12:45" as datetime); // returns true
\endcode */
BASIC_UNARY_BUILTIN(isdatetime, { result.setInt(topic->isDateTime()); });

/*! \page qlang_builtin_isdate isdate()
Checks if topic or argument is a date-time
\code
    isdate("2014-01-01" as date); // returns true
\endcode */
BASIC_UNARY_BUILTIN(isdate, { result.setInt(topic->isDate()); });

/*! \page qlang_builtin_istime istime()
Checks if topic or argument is a date-time
\code
    istime("12:34:56-01:00" as time); // returns true
\endcode */
BASIC_UNARY_BUILTIN(istime, { result.setInt(topic->isTime()); });

/*! \page qlang_builtin_isdateortime isdateortime()
Checks if topic or argument is a date-time, or date, or time
\code
    isdateortime("2014-01-01T12:45:00" as datetime); // returns true
    isdateortime("12:34:56+07:00" as time); // returns true
\endcode */
BASIC_UNARY_BUILTIN(isdateortime, { result.setInt(topic->isDateOrTime()); });

/*! \page qlang_builtin_iscallable iscallable()
Checks if topic or argument is a callable function (built-in or defined at runtime)
\code
    iscallable.iscallable(); // returns true
    iscallable(function(x,y){x > y}); // returns true
    iscallable("iscallable"); // returns false
\endcode */
BASIC_UNARY_BUILTIN(iscallable, { result.setInt(ctx.isCallable(*topic)); });

/*! \page qlang_builtin_isscalar isscalar()
Checks if topic or argument is a scalar value
\code
    PI.isscalar(); // returns true
    isscalar("3.14"); // returns true
    isscalar(["3.14"]); // returns false
\endcode */
BASIC_UNARY_BUILTIN(isscalar, { result.setInt(topic->isScalar()); });

/*! \page qlang_builtin_islist islist()
Checks if topic or argument is a list
\code
    [].islist(); // returns true
    islist([1,2,3]); // returns true
    islist("123"); // returns false
\endcode */
BASIC_UNARY_BUILTIN(islist, { result.setInt(topic->isList()); });

/*! \page qlang_builtin_isdic isdic()
Checks if topic or argument is a dictionary
\code
    {"a": 1, "b": 2}.isdic(); // returns true
    isdic(dic()); // returns true
    isdic(["a", 1, "b", 2]); // returns false
\endcode */
BASIC_UNARY_BUILTIN(isdic, { result.setInt(topic->isDic()); });

/*! \page qlang_builtin_join join()
Joins a list using a given string (a space by default)
\code
    [1, 2, 3].join(); // returns "1 2 3"
    join(["a", "bc"]); // returns "a bc"
    join(["a", "bc"], "*"); // returns "a*bc"
\endcode
\see \ref qlang_builtin_cat, \ref qlang_builtin_split */
class sQLangBuiltin_join : public BuiltinFunction {
public:
    sQLangBuiltin_join() { _name.printf("builtin join() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        const char * joiner = " ";
        if (topic) {
            if (!checkTopicListOrNull(ctx, topic) || !checkNArgs(ctx, nargs, 0, 1))
                return false;
            if (nargs >= 1) {
                joiner = args[0].asString();
            }
        } else {
            if (!checkNArgs(ctx, nargs, 1, 2))
                return false;
            if (!args[0].isList() && !args[1].isNull()) {
                ctx.setError(EVAL_SYNTAX_ERROR, "%s in 2-argument form requires a list as first argument and a string as second argument", getName());
                return false;
            }
            topic = args;
            if (nargs >= 2) {
                joiner = args[1].asString();
            }
        }

        sStr s;
        for (idx i=0; i<topic->dim(); i++)
            s.printf("%s%s", i ? joiner : "", topic->getListElt(i)->asString());
        s.add0();
        result.setString(s.ptr());
        return true;
    }
};

/*! \page qlang_builtin_keys keys()
Retrieves the list of keys of a dictionary, array, or object topic
\code
    {"a": 1, "b": 2}.keys(); // returns ["a", "b"]
    (12345 as obj).keys(); // returns ["id", "_type", "created", ... ]
    ["a", "Z"].keys(); // returns [0, 1]
\endcode
\see \ref qlang_builtin_kv */
class sQLangBuiltin_keys : public BuiltinFunction {
public:
    sQLangBuiltin_keys() { _name.printf("builtin keys() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            ctx.setError(EVAL_SYNTAX_ERROR, "%s requires a topic value", getName());
            return false;
        }
        if (!checkNArgs(ctx, nargs, 0, 0))
            return false;

        eEvalStatus status;
        if ((status = ctx.evalKeys(result, *topic)) != EVAL_SUCCESS) {
            ctx.setError(status, "%s failed", getName());
            return false;
        }
        return true;
    }
};

/*! \page qlang_builtin_kv kv()
Retrieves key/value pairs of a dictionary, array, or object topic
\code
    {"a": 1, "b": 2}.kv(); // returns [["a", 1], ["b", 2]]
    (12345 as obj).kv(); // returns [["id", 12345], ["_type", "svc-foo"], ...]
    ["a", "Z"].kv(); // returns [[0, "a"], [1, "Z"]]
\endcode
\see \ref qlang_builtin_keys */
class sQLangBuiltin_kv : public BuiltinFunction {
public:
    sQLangBuiltin_kv() { _name.printf("builtin kv() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            ctx.setError(EVAL_SYNTAX_ERROR, "%s requires a topic value", getName());
            return false;
        }
        if (!checkNArgs(ctx, nargs, 0, 0))
            return false;

        eEvalStatus status;
        if ((status = ctx.evalKV(result, *topic)) != EVAL_SUCCESS) {
            ctx.setError(status, "%s failed", getName());
            return false;
        }
        return true;
    }
};

/*! \page qlang_builtin_log log()
Calculates the natural logarithm
\code
    log(exp(3)); // returns 3.0
\endcode */
BASIC_BUILTIN(log, 1, { result.setReal(log(args[0].asReal())); } );
/*! \page qlang_builtin_log2 log2()
Calculates the base-2 logarithm
\code
    log(8); // returns 3.0
\endcode */
BASIC_BUILTIN(log2, 1, { result.setReal(log2(args[0].asReal())); } );
/*! \page qlang_builtin_log10 log10()
Calculates the base-10 logarithm
\code
    log(1000); // returns 3.0
\endcode */
BASIC_BUILTIN(log10, 1, { result.setReal(log10(args[0].asReal())); } );

/*! \page qlang_builtin_map map()
Runs a specified callback on each element of a list as topic, and returns the list of resulting values
\code
    [1, 2, 3, 4].map({sqrt(this)}; // returns [1, 1.414, 1.73, 2.0]
    ([12345, 12346] as objlist).map({[.id, ._type]}); // returns [[12345, "foo"], [12346, "bar"]]
\endcode
\see \ref qlang_builtin_filter, \ref qlang_builtin_foreach, \ref qlang_builtin_reduce */
class sQLangBuiltin_map : public BuiltinFunction {
public:
    sQLangBuiltin_map() { _name.printf("bultin map() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicListOrNull(ctx, topic) || !checkNArgs(ctx, nargs, 1, 1) || !checkArgsCallable(ctx, args, nargs))
            return false;

        result.setList();
        sVariant argResult;
        for (idx i=0; i<topic->dim(); i++) {
            if (!static_cast<CallableWrapper*>(args[0].asData())->call(argResult, ctx, topic->getListElt(i), NULL, 0)) {
                ctx.setError(EVAL_OTHER_ERROR, "%s: call to argument on topic list element #%" DEC " failed", getName(), i);
                return false;
            }
            result.push(argResult);
        }
        return true;
    }
};

// list.max() OR max(x1, x2, x3, ...) OR max(list)
/*! \page qlang_builtin_max max()
Finds the numeric maximum of a list. If multiple arguments are given, each argument is cast to a numeric value and their maximum is found.
\code
    [1, 5, -2].max(); // returns 5
    max(["1", "5", "-2"]); // returns 5
    max([1, 1, 1, 1], 2, 3); // returns 4 - each argument is cast to a numeric value, and a list cast to a numeric value is interpreted as the number of elements in the list
\endcode
\see \ref qlang_builtin_min, \ref qlang_builtin_clamp */
BASIC_NUMERIC_LIST_BUILTIN(max,
    {
        if (!dim) return true;
        result.setReal(-INFINITY);
    },
    {
        if (result < numericVal)
            result = numericVal;
    },
    {
        /* nothing */
    }
);

/*! \page qlang_builtin_mean mean()
Finds the numeric mean of a list. If multiple arguments are given, each argument is cast to a numeric value and their mean is found.
\code
    [1, 2, 4].mean(); // returns 2.333
    mean([1, 2, 4]); // returns 2.333
    mean([1, 2], 4); // returns 3.0 - each argument is cast to a numeric value, and a list cast to a numeric value is interpreted as the number of elements in the list
\endcode */
BASIC_NUMERIC_LIST_BUILTIN(mean,
    {
        if (!dim) return true;
        result.setReal(0);
    },
    {
        result += numericVal;
    },
    {
        result /= (idx)dim;
    }
);

/*! \page qlang_builtin_min min()
Finds the numeric minimum of a list. If multiple arguments are given, each argument is cast to a numeric value and their minimum is found.
\code
    [1, 5, -2].min(); // returns -2
    min(["1", "5", "-2"]); // returns -2
    min([], 2, 3); // returns 0 - each argument is cast to a numeric value, and a list cast to a numeric value is interpreted as the number of elements in the list
\endcode
\see \ref qlang_builtin_max, \ref qlang_builtin_clamp */
BASIC_NUMERIC_LIST_BUILTIN(min,
    {
        if (!dim) return true;
        result.setReal(INFINITY);
    },
    {
        if (result > numericVal)
            result = numericVal;
    },
    {
        /* nothing */
    }
);

/*! \page qlang_builtin_now now()
Returns current time as a datetime value
\code
    now(); // returns "2014-12-28 12:34:56-05:00"
\endcode */
class sQLangBuiltin_now : public BuiltinFunction {
public:
    sQLangBuiltin_now() { _name.printf("builtin now() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicNone(ctx, topic) || !checkNArgs(ctx, nargs, 0, 0)) {
            return false;
        }

        result.setDateTime(::time(0));
        return true;
    }
};

/*! \page qlang_builtin_pow pow()
Raises to a power
\code
    pow(2, 3) // returns 8.0
\endcode */
BASIC_BUILTIN(pow, 2, { result.setReal(pow(args[0].asReal(), args[1].asReal())); });

class sQLangBuiltin_props : public BuiltinFunction {
public:
    sQLangBuiltin_props() { _name.printf("builtin props() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicObjectId(ctx, topic))
            return false;

        sVariant filterOptions;
        if (nargs) {
            filterOptions.setList();
            for (idx i=0; i<nargs; i++)
                filterOptions.push(args[i]);
        }

        eEvalStatus status;
        if ((status = ctx.evalProps(result, *topic, nargs ? &filterOptions : NULL)) != EVAL_SUCCESS) {
            ctx.setError(status, "%s failed", getName());
            return false;
        }
        return true;
    }
};

/*! \page qlang_builtin_push push()
Pushes arguments to the end of a list; without a topic, creates a new list
\code
    [1, 2].push(3, 4); // returns [1, 2, 3, 4]
\endcode
\see \ref qlang_builtin_append */
class sQLangBuiltin_push : public BuiltinFunction {
public:
    sQLangBuiltin_push() { _name.printf("builtin push() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic) {
            if (!checkTopicListOrNull(ctx, topic)) {
                return false;
            }
        }
        result = *topic;
        if (!result.isList()) {
            result.setList();
        }

        for (idx i=0; i<nargs; i++)
            topic->push(args[i]);

        result = *topic;
        return true;
    }
};

/*! \page qlang_builtin_range range()
Creates a list with a range of numeric values.

When run on a list argument or topic, returns the list's list of indices. With a single integer argument as value, calculates the range from 0 to this value. Given two integer arguments, calculates the range from the first to the second; an optional third argument specifies the step size.
\code
    ["a", "b", "c"].range(); // returns [0, 1, 2]
    range(["a", "b", "c"]); // retutns [0, 1, 2]
    range(8); // returns [0, 1, 2, 4, 5, 6, 7]
    range(2, 8); // returns [2, 3, 4, 5, 6, 7]
    range(2, 8, 4); // returns [2, 4, 6]
    range(2, -8, -2); // returns [2, 0, -2, -4, -6]
\endcode */
class sQLangBuiltin_range : public BuiltinFunction {
public:
    sQLangBuiltin_range() { _name.printf("bultin range() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (topic) {
            if (!checkNArgs(ctx, nargs, 0, 0))
                return false;

            result.setList();
            if (!topic->isList())
                return true;

            for (idx i=0; i<topic->dim(); i++)
                result.push(i);
            return true;
        }

        if (!checkNArgs(ctx, nargs, 1, 3))
            return false;

        idx start = 0, limit = 0, step = 1;
        if (nargs == 1) {
            if (args[0].isList())
                limit = args[0].dim();
            else
                limit = args[0].asInt();
        } else {
            start = args[0].asInt();
            limit = args[1].asInt();
            if (nargs > 2)
                step = args[2].asInt();
        }

        result.setList();
        if (step > 0 && limit > start) {
            for (idx i=start; i<limit; i+=step)
                result.push(i);
        } else if (step < 0 && limit < start) {
            for (idx i=start; i>limit; i+=step)
                result.push(i);
        }

        return true;
    }
};

/*! \page qlang_builtin_reduce reduce()
Repeatedly runs a 2-argument callback on the next list value and the result of the previous callback call.
\code
    [1, 2, 3, 4].reduce(function(x, y) {x + y}); // returns 10 - equivalent to ((1 + 2) + 3) + 4, or to [1, 2, 3, 4].sum()
\endcode
\see \ref qlang_builtin_filter, \ref qlang_builtin_foreach, \ref qlang_builtin_map */
class sQLangBuiltin_reduce : public BuiltinFunction {
public:
    sQLangBuiltin_reduce() { _name.printf("bultin reduce() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            result.setInt();
            return true;
        }

        if (!checkTopicListOrNull(ctx, topic) || !checkNArgs(ctx, nargs, 1, 1) || !checkArgsCallable(ctx, args, nargs))
            return false;

        if (topic->dim() == 0) {
            result.setInt();
            return true;
        }

        sVariant argsReduce[2];
        sVariant resultReduce;
        topic->getElt((idx)0, argsReduce[0]);

        for (idx i=1; i<topic->dim(); i++) {
            topic->getElt(i, argsReduce[1]);
            if (!static_cast<CallableWrapper*>(args[0].asData())->call(resultReduce, ctx, NULL, argsReduce, 2)) {
                ctx.setError(EVAL_OTHER_ERROR, "%s: call to argument on topic list element #%" DEC " failed", getName(), i);
                return false;
            }
            argsReduce[0] = resultReduce;
        }

        result = argsReduce[0];
        return true;
    }
};

/*! \page qlang_builtin_sort sort()
Sorts a list numerically by default, or using a given two-argument callback whose return value is interpreted as "greater than" if positive, "less than" if negative, or "equal" if zero.

\code
    ["10", 2.0, "1"].sort(); // returns ["1", "2", "10"]
    sort(["1", "10", 2.0], function(x, y) {y - x}); // returns ["10", 2.0, "1"]
\endcode

\note a separate function, \ref qlang_builtin_strsort, is provided for conveniently sorting by string comparison. */
class sQLangBuiltin_sort : public BuiltinFunction {
protected:
    struct CallbackSorterParam
    {
        Context &ctx;
        sVariant * lst;
        sVariant * cmp;
        CallbackSorterParam(Context &ctx_, sVariant * lst_, sVariant * cmp_) : ctx(ctx_), lst(lst_), cmp(cmp_) {}
    };

    sSort::sCallbackSorterSimple _defaultCallbackSorter;

public:
    sQLangBuiltin_sort() {
        _name.printf("bultin sort() function");
        _defaultCallbackSorter = callbackSorterNum;
    }

    static idx callbackSorterStr(void * param_, void * arr, idx i0, idx i1)
    {
        CallbackSorterParam * param = static_cast<CallbackSorterParam*>(param_);
        return strcmp(param->lst->getListElt(i0)->asString(), param->lst->getListElt(i1)->asString());
    }

    static idx callbackSorterNum(void * param_, void * arr, idx i0, idx i1)
    {
        CallbackSorterParam * param = static_cast<CallbackSorterParam*>(param_);
        sVariant a, b;
        param->lst->getListElt(i0)->getNumericValue(a);
        param->lst->getListElt(i1)->getNumericValue(b);
        if (a == b)
            return 0;

        a -= b;
        b.setInt(0);
        return a < b ? -1 : 1;
    }

    static idx callbackSorterCmp(void * param_, void * arr, idx i0, idx i1)
    {
        CallbackSorterParam * param = static_cast<CallbackSorterParam*>(param_);
        if (param->ctx.getMode() == Context::MODE_ERROR)
            return 0;

        sVariant argsCmp[2];
        sVariant resultCmp;
        argsCmp[0] = *(param->lst->getListElt(i0));
        argsCmp[1] = *(param->lst->getListElt(i1));

        if (!static_cast<CallableWrapper*>(param->cmp->asData())->call(resultCmp, param->ctx, NULL, argsCmp, 2)) {
            param->ctx.setError(EVAL_OTHER_ERROR, "builtin sort() function: call to argument on list elements #%" DEC " and #%" DEC " failed", i0, i1);
            return 0;
        }

        return resultCmp.asInt();
    }

    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        result.setList();

        if (!topic) {
            if (!nargs)
                return true;
            topic = args;
            nargs--;
        }

        if (!topic->isList())
            return true;

        sVec<idx> indices;
        indices.resize(topic->dim());
        for (idx i=0; i<indices.dim(); i++)
            indices[i] = i;

        CallbackSorterParam param(ctx, topic, nargs ? args : 0);

        sSort::sortSimpleCallback<idx>(nargs ? callbackSorterCmp : _defaultCallbackSorter, &param, indices.dim(), 0, indices.ptr());

        if (ctx.getMode() == Context::MODE_ERROR)
            return false;

        for (idx i=0; i<indices.dim(); i++)
            result.push(*(topic->getListElt(indices[i])));

        return true;
    }
};

/*! \page qlang_builtin_split split()
Splits a string using a specified string or list of strings. Splitting by "" means retrieves a string's list of characters. By default, splits by "," and ";"
\code
    a = "hello;world"; a.split(); // returns ["hello", "world"]
    split("123,456 789;0", [",", ";", " "]); // returns ["123", "456", "789", "0"]
    split("a*b**c", "*"); // returns ["a", "b", "", "c"]
    split("hello", ""); // returns ["h", "e", "l", "l", "o"]
\endcode
\see \ref qlang_builtin_join */
class sQLangBuiltin_split : public BuiltinFunction {
public:
    sQLangBuiltin_split() { _name.printf("builtin split() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            if (!checkNArgs(ctx, nargs, 1, 2))
                return false;
            topic = args++;
            nargs--;
        }

        sStr find00;
        bool by_char = false;
        if (nargs) {
            if (args->isList()) {
                for (idx i=0; i<args->dim(); i++) {
                    const char * s = args->getListElt(i)->asString();
                    if (*s) {
                        find00.add(s);
                    } else {
                        by_char = true;
                        break;
                    }
                }
            } else {
                find00.add(args->asString());
                if (!find00[0])
                    by_char = true;
            }
        } else {
            find00.add(",");
            find00.add(";");
        }
        find00.add0(2);
        result.setList();

        // We cannot use sString::searchAndReplaceStrings because we don't want
        // to replace two consecutive separators by \0\0 and then not know where
        // the result 00-terminated string really terminates

        sStr buf("%s", topic->asString());
        idx curpos = 0;
        for (idx i=0; i<buf.length(); i++) {
            // splitting by "" is special : we need to omit the first empty value
            // (so "ab".split([""]) is ["a", "b"] not ["", "a", "b"]) and "" cannot
            // be a non-first element of find00 since "a"_""__ equals "a"___

            if (by_char) {
                char c[2];
                c[0] = buf[i];
                c[1] = 0;
                result.push(c);
                curpos = sIdxMax;
                continue;
            }

            idx lenfnd = sString::compareChoice(buf.ptr(i), find00.ptr(), 0, false, 0, false);
            if (lenfnd < 0)
                continue;

            char bck = buf[i];
            buf[i] = 0;
            result.push(buf.ptr(curpos));
            buf[i] = bck;
            curpos = i + lenfnd;
            i = sMax<idx>(i, curpos - 1);
        }

        if (curpos <= buf.length())
            result.push(buf.ptr(curpos));

        return true;
    }
};

/*! \page qlang_builtin_sqrt sqrt()
Calculates the square root
\code
    sqrt(25) // returns 5.0
\endcode */
BASIC_BUILTIN(sqrt, 1, { result.setReal(sqrt(args[0].asReal())); } );

/*! \page qlang_builtin_strip strip()
Removes leading and trailing whitespace (including newlines) from a string
\code
    a = " hello\t\n "; a.strip(); // returns "hello"
\endcode */
class sQLangBuiltin_strip : public BuiltinFunction {
public:
    sQLangBuiltin_strip() { _name.printf("builtin strip() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            if (!checkNArgs(ctx, nargs, 1, 1))
                return false;
            topic = args++;
            nargs--;
        }

        sStr buf;
        sString::cleanEnds(&buf, topic->asString(), 0, " \t\r\n", 1);
        result.setString(buf.ptr());
        return true;
    }
};

/*! \page qlang_builtin_sum sum()
Finds the numeric sum of a list. If multiple arguments are given, each argument is cast to a numeric value and their sum is found.
\code
    [1, 2, 3].sum(); // returns 6
    sum([1, 2, 3]); // returns 6
    mean([1, 2], 3); // returns 5 - each argument is cast to a numeric value, and a list cast to a numeric value is interpreted as the number of elements in the list
\endcode */
BASIC_NUMERIC_LIST_BUILTIN(sum,
    {
        /* nothing */
    },
    {
        result += numericVal;
    },
    {
        /* nothing */
    }
);

/*! \page qlang_builtin_strsort strsort()
Like \ref qlang_builtin_sort, but uses case-sensitive string comparison by default.
\code
    ["10", 2.0, "1"].sort(); // returns ["1", "10", 2.0]
\endcode */
class sQLangBuiltin_strsort : public sQLangBuiltin_sort {
public:
    sQLangBuiltin_strsort() {
        _name.printf("bultin strsort() function");
        _defaultCallbackSorter = callbackSorterStr;
    }
};

/*! \page qlang_builtin_strftime strftime()
Convert date/time/datetime values into human-readable strings. See
<a href="http://pubs.opengroup.org/onlinepubs/009695399/functions/strftime.html">the C strftime() function documentation</a> for format string details.

If the argument is not a date/time/datetime, it will be parsed (if a string)
or interpreteted as a Unix timestamp (if a numeric value). If a format is not
provided, the value will be printed in modified RFC 3330 form.
\code
    strftime(1412817512); // returns "2014-10-08 21:18:32-04:00"
    strftime("2014-08-10 12:34 PM", "%d/%m/%Y"); // returns "10/08/2014"
\endcode
\see \ref qlang_builtin_strptime */
class sQLangBuiltin_strftime : public BuiltinFunction {
public:
    sQLangBuiltin_strftime() { _name.printf("builtin strftime() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if ((!topic && !checkNArgs(ctx, nargs, 1, 2)) || (topic && !checkNArgs(ctx, nargs, 0, 1))) {
            return false;
        }

        if (!topic) {
            topic = args++;
            nargs--;
        }

        if (nargs) {
            const char * fmt = args[0].asString();
            struct tm tm;
            sSet(&tm);
            topic->asDateTime(&tm);
            char buf[256];
            memset(buf, 0, 256);
            strftime(buf, 255, fmt, &tm);
            result.setString(buf);
        } else {
            sVariant tmp, *pdate_or_time;
            if (topic->isDateOrTime()) {
                pdate_or_time = topic;
            } else {
                tmp.setDateTime(*topic);
                pdate_or_time = &tmp;
            }
            result.setString(pdate_or_time->asString());
        }
        return true;
    }
};

/*! \page qlang_builtin_strptime strptime()
Explicitly parse time strings into datetime values; useful for date/time formats
which are not automatically detected by "foo as datetime". See
<a href="http://pubs.opengroup.org/onlinepubs/009695399/functions/strptime.html">the C strptime() function documentation</a> for format string details.
\code
    strptime("08.10.2014 21:18", "%d.%m.%Y %H:%M"); // returns "2014-10-08 21:18:00-04:00"
    strptime("08.10.2014 21:18", "%d.%m.%Y %H:%M") as int; // returns 1412817480
\endcode
\see \ref qlang_builtin_strftime */
class sQLangBuiltin_strptime : public BuiltinFunction {
public:
    sQLangBuiltin_strptime() { _name.printf("builtin strptime() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if ((!topic && !checkNArgs(ctx, nargs, 1, 2)) || (topic && !checkNArgs(ctx, nargs, 0, 1))) {
            return false;
        }

        if (!topic) {
            topic = args++;
            nargs--;
        }

        if (nargs) {
            const char * fmt = args[0].asString();
            struct tm tm;
            sSet(&tm);
            if (!strptime(topic->asString(), fmt, &tm)) {
                result.setNull();
                return true;
            }
            result.setDateTime(&tm);
        } else {
            result.setDateTime(*topic);
        }
        return true;
    }
};

/*! \page qlang_builtin_union union()
Finds the union of lists
\code
    union([1, 2, 3, 4], [4, 3, 5], [67]); // returns [1, 2, 3, 4, 5, 67]
\endcode
\see \ref qlang_builtin_intersect */
class sQLangBuiltin_union : public BuiltinFunction {
public:
    sQLangBuiltin_union() { _name.printf("builtin union() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!checkTopicNone(ctx, topic))
            return false;

        eEvalStatus status;
        if ((status = ctx.evalListUnion(result, args, nargs)) != EVAL_SUCCESS) {
            ctx.setError(status, "%s failed", getName());
            return false;
        }
        return true;
    }
};

#define REGISTER_BUILTIN_FUNC(name) \
static sQLangBuiltin_ ## name builtin_ ## name; \
registerBuiltinFunction(#name, builtin_ ## name)

/*! \page qlang_builtins Basic builtin functions

These are available in the basic slib::qlang::Context and all of its child classes.

- \subpage qlang_builtin_aa1letter
- \subpage qlang_builtin_aa3letter
- \subpage qlang_builtin_abs
- \subpage qlang_builtin_append
- \subpage qlang_builtin_apply
- \subpage qlang_builtin_cat
- \subpage qlang_builtin_ceil
- \subpage qlang_builtin_clamp
- \subpage qlang_builtin_clust
- \subpage qlang_builtin_counts
- \subpage qlang_builtin_dic
- \subpage qlang_builtin_dim
- \subpage qlang_builtin_exp
- \subpage qlang_builtin_filter
- \subpage qlang_builtin_floor
- \subpage qlang_builtin_foreach
- \subpage qlang_builtin_histogram
- \subpage qlang_builtin_intersect
- \subpage qlang_builtin_isnull
- \subpage qlang_builtin_isnumeric
- \subpage qlang_builtin_isstring
- \subpage qlang_builtin_isobj
- \subpage qlang_builtin_isdatetime
- \subpage qlang_builtin_isdate
- \subpage qlang_builtin_istime
- \subpage qlang_builtin_isdateortime
- \subpage qlang_builtin_iscallable
- \subpage qlang_builtin_isscalar
- \subpage qlang_builtin_islist
- \subpage qlang_builtin_isdic
- \subpage qlang_builtin_join
- \subpage qlang_builtin_keys
- \subpage qlang_builtin_kv
- \subpage qlang_builtin_log
- \subpage qlang_builtin_log2
- \subpage qlang_builtin_log10
- \subpage qlang_builtin_map
- \subpage qlang_builtin_max
- \subpage qlang_builtin_mean
- \subpage qlang_builtin_min
- \subpage qlang_builtin_now
- \subpage qlang_builtin_pow
- \subpage qlang_builtin_props
- \subpage qlang_builtin_push
- \subpage qlang_builtin_range
- \subpage qlang_builtin_reduce
- \subpage qlang_builtin_sort
- \subpage qlang_builtin_split
- \subpage qlang_builtin_sqrt
- \subpage qlang_builtin_strip
- \subpage qlang_builtin_strsort
- \subpage qlang_builtin_strftime
- \subpage qlang_builtin_strptime
- \subpage qlang_builtin_sum
- \subpage qlang_builtin_union

In addition, the mathematical constants \a E and \a PI are built in.

*/

void Context::registerDefaultBuiltins()
{
    REGISTER_BUILTIN_FUNC(aa1letter);
    REGISTER_BUILTIN_FUNC(aa3letter);
    REGISTER_BUILTIN_FUNC(abs);
    REGISTER_BUILTIN_FUNC(append);
    REGISTER_BUILTIN_FUNC(apply);
    REGISTER_BUILTIN_FUNC(cat);
    REGISTER_BUILTIN_FUNC(ceil);
    REGISTER_BUILTIN_FUNC(clamp);
    REGISTER_BUILTIN_FUNC(clust);
    REGISTER_BUILTIN_FUNC(counts);
    REGISTER_BUILTIN_FUNC(dic);
    REGISTER_BUILTIN_FUNC(dim);
    REGISTER_BUILTIN_FUNC(exp);
    REGISTER_BUILTIN_FUNC(filter);
    REGISTER_BUILTIN_FUNC(floor);
    REGISTER_BUILTIN_FUNC(foreach);
    REGISTER_BUILTIN_FUNC(histogram);
    REGISTER_BUILTIN_FUNC(intersect);
    REGISTER_BUILTIN_FUNC(isnull);
    REGISTER_BUILTIN_FUNC(isnumeric);
    REGISTER_BUILTIN_FUNC(isstring);
    REGISTER_BUILTIN_FUNC(isobj);
    REGISTER_BUILTIN_FUNC(isdatetime);
    REGISTER_BUILTIN_FUNC(isdate);
    REGISTER_BUILTIN_FUNC(istime);
    REGISTER_BUILTIN_FUNC(isdateortime);
    REGISTER_BUILTIN_FUNC(iscallable);
    REGISTER_BUILTIN_FUNC(isscalar);
    REGISTER_BUILTIN_FUNC(islist);
    REGISTER_BUILTIN_FUNC(isdic);
    REGISTER_BUILTIN_FUNC(join);
    REGISTER_BUILTIN_FUNC(keys);
    REGISTER_BUILTIN_FUNC(kv);
    REGISTER_BUILTIN_FUNC(log);
    REGISTER_BUILTIN_FUNC(log2);
    REGISTER_BUILTIN_FUNC(log10);
    REGISTER_BUILTIN_FUNC(map);
    REGISTER_BUILTIN_FUNC(max);
    REGISTER_BUILTIN_FUNC(mean);
    REGISTER_BUILTIN_FUNC(min);
    REGISTER_BUILTIN_FUNC(now);
    REGISTER_BUILTIN_FUNC(pow);
    REGISTER_BUILTIN_FUNC(props);
    REGISTER_BUILTIN_FUNC(push);
    REGISTER_BUILTIN_FUNC(range);
    REGISTER_BUILTIN_FUNC(reduce);
    REGISTER_BUILTIN_FUNC(sort);
    REGISTER_BUILTIN_FUNC(split);
    REGISTER_BUILTIN_FUNC(sqrt);
    REGISTER_BUILTIN_FUNC(strip);
    REGISTER_BUILTIN_FUNC(strsort);
    REGISTER_BUILTIN_FUNC(strftime);
    REGISTER_BUILTIN_FUNC(strptime);
    REGISTER_BUILTIN_FUNC(sum);
    REGISTER_BUILTIN_FUNC(union);

    registerBuiltinRealConst("E", M_E);
    registerBuiltinRealConst("PI", M_PI);
}
