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

static const char *ContextEvalStatusNames[] = {
    "success",
    "error",
    "not implemented",
    "index out of range",
    "invalid data type",
    "expected list value",
    "expected dic value",
    "expected subscriptable value (list, dic, object, or string)",
    "expected callable value (function or method)",
    "expected object value",
    "wrong number of arguments",
    "permission error",
    "attempted to write a read-only value",
    "syntax error",
    "property not defined",
    "invalid variable",
    "system error",
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
    _returnValue.setNull();
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
    RawCallbackWrapper(const char *s, Context::BuiltinRawCallback func, void * param): _func(func), _param(param) { _name.printf("%s", s); }
    RawCallbackWrapper(const RawCallbackWrapper &rhs): _func(rhs._func), _param(rhs._param) { _name.printf("%s", rhs._name.ptr()); }
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

void Context::clearReturn(bool resetReturnValue)
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
    checkIndex(i, lhs.dim());

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
    checkIndex(i1, lhs.dim());
    checkIndex(i2, lhs.dim());

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
                outval.setBool(true);
                return EVAL_SUCCESS;
            }
        }
        outval.setBool(false);
        return EVAL_SUCCESS;
    } else if (lhs.isList()) {
        for (idx i=0; i<lhs.dim(); i++) {
            sVariant lhsi;
            lhs.getElt(i, lhsi);
            for (idx j=0; j<dimrhs; j++) {
                if (lhsi == rhs[j]) {
                    outval.setBool(true);
                    return EVAL_SUCCESS;
                }
            }
        }
        outval.setBool(false);
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
            outval.setBool(true);
        }
    } else {
        outval.setBool(false);
    }
    return EVAL_SUCCESS;
}

eEvalStatus Context::evalNMatch(sVariant &outval, sVariant &val, const regex_t *re)
{
    eEvalStatus status = evalMatch(outval, val, re, 0, NULL);
    if (status == EVAL_SUCCESS) {
        outval.setBool(!outval.asBool());
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

idx BuiltinFunction::getArgAsHiveIds(const idx arg_num, sVariant *args, const idx &nargs, sVec<sHiveId> &out) const
{
    if( args && (nargs > arg_num) && !args[arg_num].isNull() ) {
        return args[arg_num].asHiveIds(out);
    }
    return 0;
}

const sHiveId* BuiltinFunction::getArgAsHiveId(const idx arg_num, sVariant *args, const idx &nargs) const
{
    return (args && (nargs > arg_num)) ? args[arg_num].asHiveId() : 0;
}

const char* BuiltinFunction::getArgAsString(const idx arg_num, sVariant *args, const idx &nargs, const char *dflt) const
{
    return (args && (nargs > arg_num) && !args[arg_num].isNull()) ? args[arg_num].asString() : dflt;
}

idx BuiltinFunction::getArgAsInt(const idx arg_num, sVariant *args, const idx &nargs, const idx dflt) const
{
    return (args && (nargs > arg_num) && !args[arg_num].isNull()) ? args[arg_num].asInt() : dflt;
}

udx BuiltinFunction::getArgAsUInt(const idx arg_num, sVariant *args, const idx &nargs, const udx dflt) const
{
    return (args && (nargs > arg_num) && !args[arg_num].isNull()) ? args[arg_num].asUInt() : dflt;
}

bool BuiltinFunction::getArgAsBool(const idx arg_num, sVariant *args, const idx &nargs, const bool dflt) const
{
    return (args && (nargs > arg_num) && !args[arg_num].isNull()) ? args[arg_num].asBool() : dflt;
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

BASIC_BUILTIN(abs, 1,
    {
        sVariant numericVal;
        args[0].getNumericValue(numericVal);
        result = (numericVal.asReal() < 0) ? (numericVal *= (idx)(-1), numericVal) : numericVal;
    });

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

BASIC_BUILTIN(ceil, 1, { result.setInt((idx)ceil(args[0].asReal())); } );

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
            tmp.printf("%s", labels->getListElt(x)->asString());
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
    virtual bool isNullish() const { return asInt() == 0; }
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

        if (distance && distance->isList()) {
            dim = distance->dim();
            for (idx i=0; i<distance->dim(); i++)
                dim = sMax<idx>(dim, distance->getListElt(i)->dim());

            clust->reset(dim);
            for (idx i=0; i<dim; i++) {
                for (idx j=i+1; j<dim; j++) {
                    sVariant * prow = distance->getListElt(i);
                    sVariant * pcell = prow ? prow->getListElt(j) : 0;
                    real d =  pcell ? pcell->asReal() : 0;
                    clust->setDist(i, j, isnan(d) ? 0 : d);
                }
            }
        } else if (activity && activity->isList()) {
            dim = activity->dim();
            std::unique_ptr<Tdist> dist(makeDist(distalgo));
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

BASIC_UNARY_BUILTIN(dim, { result.setInt(topic->dim()); });

BASIC_BUILTIN(exp, 1, { result.setReal(exp(args[0].asReal())); });

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

BASIC_BUILTIN(floor, 1, { result.setInt((idx)floor(args[0].asReal())); } );

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

BASIC_UNARY_BUILTIN(isnull, { result.setBool(topic->isNull()); });

BASIC_UNARY_BUILTIN(isnumeric, { result.setBool(topic->isNumeric()); });

BASIC_UNARY_BUILTIN(isstring, { result.setBool(topic->isString()); });

BASIC_UNARY_BUILTIN(isobj, { result.setBool(topic->isHiveId()); });

BASIC_UNARY_BUILTIN(isdatetime, { result.setBool(topic->isDateTime()); });

BASIC_UNARY_BUILTIN(isdate, { result.setBool(topic->isDate()); });

BASIC_UNARY_BUILTIN(istime, { result.setBool(topic->isTime()); });

BASIC_UNARY_BUILTIN(isdateortime, { result.setBool(topic->isDateOrTime()); });

BASIC_UNARY_BUILTIN(iscallable, { result.setBool(ctx.isCallable(*topic)); });

BASIC_UNARY_BUILTIN(isscalar, { result.setBool(topic->isScalar()); });

BASIC_UNARY_BUILTIN(islist, { result.setBool(topic->isList()); });

BASIC_UNARY_BUILTIN(isdic, { result.setBool(topic->isDic()); });

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

BASIC_BUILTIN(log, 1, { result.setReal(log(args[0].asReal())); } );
BASIC_BUILTIN(log2, 1, { result.setReal(log2(args[0].asReal())); } );
BASIC_BUILTIN(log10, 1, { result.setReal(log10(args[0].asReal())); } );

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
    }
);

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
    }
);

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

class sQLangBuiltin_reduce : public BuiltinFunction {
public:
    sQLangBuiltin_reduce() { _name.printf("bultin reduce() function"); }
    virtual bool call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
    {
        if (!topic) {
            result.setNull();
            return true;
        }

        if (!checkTopicListOrNull(ctx, topic) || !checkNArgs(ctx, nargs, 1, 1) || !checkArgsCallable(ctx, args, nargs))
            return false;

        if (topic->dim() == 0) {
            result.setNull();
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


        sStr buf("%s", topic->asString());
        idx curpos = 0;
        for (idx i=0; i<buf.length(); i++) {

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

BASIC_BUILTIN(sqrt, 1, { result.setReal(sqrt(args[0].asReal())); } );

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

BASIC_NUMERIC_LIST_BUILTIN(sum,
    {
    },
    {
        result += numericVal;
    },
    {
    }
);

class sQLangBuiltin_strsort : public sQLangBuiltin_sort {
public:
    sQLangBuiltin_strsort() {
        _name.printf("bultin strsort() function");
        _defaultCallbackSorter = callbackSorterStr;
    }
};

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
