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
#include <qlang/interpreter.hpp>

using namespace slib;
using namespace slib::qlang;

const char * CallableWrapperName = "callable value";

const char * CallableWrapper::getTypeName() const
{
    return CallableWrapperName;
}

real CallableWrapper::asReal() const
{
    return NAN;
}

CallableWrapper::CallableWrapper(const Callable *callable, Scope *scope) : _callable(callable), _scope(scope)
{
    if (_scope) {
        _scope->incrementRefCount();
        _scope->registerCallable(this);
    }
}

CallableWrapper::CallableWrapper(const CallableWrapper &rhs) : _callable(rhs._callable), _scope(rhs._scope)
{
    if (_scope) {
        _scope->incrementRefCount();
        _scope->registerCallable(this);
    }
}

CallableWrapper& CallableWrapper::operator=(const CallableWrapper &rhs)
{
    if (_scope != rhs._scope) {
        if (rhs._scope) {
            rhs._scope->incrementRefCount();
            rhs._scope->registerCallable(this);
        }

        if (_scope) {
            _scope->unregisterCallable(this);
            _scope->decrementRefCount();
        }
    }

    _callable = rhs._callable;
    _scope = rhs._scope;

    return *this;
}

CallableWrapper* CallableWrapper::clone()
{
    return new CallableWrapper(*this);
}

void CallableWrapper::empty()
{
    if (Scope* scope_bck = _scope) {
        // avoid possible destruction loops
        _scope = NULL;
        scope_bck->unregisterCallable(this);
        scope_bck->decrementRefCount();
    }

    _callable = NULL;
}

bool CallableWrapper::call(sVariant &result, Context &ctx, sVariant *topic, sVariant *args, idx nargs) const
{
    Scope *ctxScope_bck = NULL;

    if (_scope) {
        ctxScope_bck = ctx.getScope();
        ctx.setScope(_scope);
    }

    bool ret = _callable->call(result, ctx, topic, args, nargs);

    if (ctxScope_bck)
        ctx.setScope(ctxScope_bck);

    return ret;
}

void CallableWrapper::markScope()
{
    if (_scope)
        _scope->mark();
}

bool CallableWrapper::operator==(const CallableWrapper &rhs) const
{
    return _callable == rhs._callable && _scope == rhs._scope;
}

bool CallableWrapper::operator<(const CallableWrapper &rhs) const
{
    if (_callable < rhs._callable)
        return true;

    return _scope < rhs._scope;
}

bool CallableWrapper::operator>(const CallableWrapper &rhs) const
{
    if (_callable > rhs._callable)
        return true;

    return _scope > rhs._scope;
}

void CallableWrapper::print(sStr &s, sVariant::ePrintMode mode, const sVariant::Whitespace * whitespace, idx indent_level) const
{
    if (mode == sVariant::eJSON)
        s.printf("\"");

    if (_scope) {
        s.printf("<%s in scope %p>", _callable->getName(), _scope);
    } else {
        s.printf("<%s>", _callable->getName());
    }

    if (mode == sVariant::eJSON)
        s.printf("\"");
}

Scope::Scope(Scope::ScopeType type): _children(sMex::fBlockDoubling|sMex::fSetZero)
{
    _parent = NULL;
    _refCount = 0;
    _state = 0;
    _type = type;
}

void Scope::deactivate()
{
    if (_state & (scope_DEACTIVATING|scope_INACTIVE))
        return;

    _state |= scope_DEACTIVATING;

    _namespace.empty();
    for (idx i = 0; i<_children.dim(); i++) {
        delete _children[i];
        _children[i] = 0;
    }
    _children.empty();
    unregisterAllCallables();

    if (_parent) {
        _parent->_refCount--;
    }

    _state &= ~scope_DEACTIVATING;
    _state |= scope_INACTIVE;
}

void Scope::empty()
{
    deactivate();
    _state = 0;
}

Scope::~Scope()
{
    deactivate();
    unregisterAllCallables();
}

Scope* Scope::addChild(Scope::ScopeType type)
{
    assert(!(_state & scope_INACTIVE));
    _refCount++;
    for (idx i=0; i<_children.dim(); i++) {
        Scope * child = _children[i];
        if (child->_state & scope_INACTIVE) {
            child->_state = 0;
            child->_type = type;
            return child;
        }
    }

    Scope *ret = new Scope();
    *_children.add(1) = ret;
    ret->_parent = this;
    ret->_type = type;
    return ret;
}

void Scope::registerCallable(CallableWrapper *cwrapper)
{
    assert(cwrapper);

    if (CallableWrapper **my = _callables.get(&cwrapper, sizeof(cwrapper)))
        *my = cwrapper;
    else
        *(_callables.set(&cwrapper, sizeof(cwrapper))) = cwrapper;
}

void Scope::unregisterCallable(CallableWrapper *cwrapper)
{
    assert(cwrapper);

    if (CallableWrapper **my = _callables.get(&cwrapper, sizeof(cwrapper)))
        *my = NULL;
}

void Scope::unregisterAllCallables()
{
    for (idx i=0; i<_callables.dim(); i++) {
        if (_callables[i])
            _callables[i]->unregisterScope();
    }
    _callables.empty();
}

void Scope::incrementRefCount()
{
    assert(!(_state & scope_INACTIVE));
    _refCount++;
}

void Scope::decrementRefCount()
{
    _refCount--;
    assert (_refCount >= 0);
    if (!_refCount)
        deactivate();
}

bool Scope::getValue(const char *name, sVariant &outval)
{
    assert(!(_state & scope_INACTIVE));

    for (Scope *scope = this; scope; scope = scope->_parent)
        if (ScopeEntry *entry = scope->_namespace.get(name)) {
            outval = entry->value;
            return true;
        }

    return false;
}

bool Scope::addValue(const char *name, sVariant &val, bool readOnly)
{
    assert(!(_state & scope_INACTIVE));

    ScopeEntry *curEntry = _namespace.get(name);
    if (curEntry) {
        if (curEntry->readOnly)
            return false;

        curEntry->set(val, readOnly);
    } else
        (_namespace.set(name))->set(val, readOnly);

    return true;
}

bool Scope::setValue(const char *name, sVariant &val, bool readOnly)
{
    assert(!(_state & scope_INACTIVE));

    for (Scope *scope = this; scope; scope = scope->_parent)
        if (ScopeEntry *curEntry = scope->_namespace.get(name)) {
            if (curEntry->readOnly)
                return false;

            curEntry->set(val, readOnly);
            return true;
        }

    return addValue(name, val, readOnly);
}

static bool markScopes(sVariant *v)
{
    if (v->isData()) {
        static_cast<CallableWrapper*>(v->asData())->markScope();
        return true;
    }
    if (v->isList()) {
        bool ret = false;
        for (idx i=0; i < v->dim(); i++) {
            if (markScopes(v->getListElt(i)))
                ret = true;
        }
        return ret;
    }
    return false;
}

// Mark the current scope, its ancestors, and whatever is reachable from named closures
void Scope::mark()
{
    if (_state & scope_MARKED)
        return;

    _state |= scope_MARKED;
    for (idx i=0; i<_namespace.dim(); i++)
        markScopes(&(_namespace[i].value));

    if (_parent)
        _parent->mark();
}

// remove nodes which are not marked.
idx Scope::sweepRecurse()
{
    if (!(_state & scope_MARKED)) {
        deactivate();
        return 1;
    }

    idx ret = 0;
    for (idx i=0; i<_children.dim(); i++)
        ret += _children[i]->sweepRecurse();

    _state &= ~scope_MARKED;
    return ret;
}

idx Scope::sweep()
{
    assert (_state & scope_MARKED);

    Scope *root = this;
    while (root->_parent) {
        root = root->_parent;
        assert (root->_state & scope_MARKED);
    }

    return root->sweepRecurse();
}

void Scope::print(sStr &out)
{
    out.printf("scope %p: #refs == %"DEC" / state == %"DEC" / type == %d\n", this, _refCount, _state, _type);
    out.printf("\tNamespace (%"DEC"):\n", _namespace.dim());
    for (idx i=0; i<_namespace.dim(); i++) {
        out.printf("\t\t\"%s\"%s == ", (const char*)(_namespace.id(i)), _namespace[i].readOnly ? " (read-only)" : "");
        _namespace[i].value.print(out);
        out.printf(";\n");
    }
    if (_callables.dim()) {
        out.printf("\tCallables (%"DEC"): ", _callables.dim());
        for (idx i=0; i<_callables.dim(); i++) {
            out.printf("%p ", _callables[i]);
        }
        out.printf(";\n");
    }
    if (_children.dim()) {
        out.printf("\tChildren (%"DEC"): ", _children.dim());
        for (idx i=0; i<_children.dim(); i++) {
            out.printf("%p ", _children[i]);
        }
        out.printf(";\n");
    }
    for (idx i=0; i<_children.dim(); i++) {
        _children[i]->print(out);
    }
}
