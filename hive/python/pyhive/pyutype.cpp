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

#include "pyhive.hpp"

using namespace slib;

namespace {
    struct TypeDic {
        private:
            sDic<pyhive::Type*> _dic;

        public:
            pyhive::Type * ensure(const sUsr & user, const sHiveId * id, const char * name = 0);
            ~TypeDic();
    } type_dic;
}

static PyObject * Type_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc || !pyhive_proc->proc || !pyhive_proc->proc->user ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return NULL;
    }

    PyObject * arg = 0;
    if( !PyArg_ParseTuple(args, "O", &arg) ) {
        return NULL;
    }

    pyhive::Type * self = 0;
    pyhive::Id * id = pyhive::Id::check(arg);
    if( id ) {
        Py_INCREF(id);
    } else {
        id = pyhive::Id::create();
        if( !id->init(arg) ) {
            Py_DECREF(id);
            id = 0;
        }
    }

    if( id ) {
        self = type_dic.ensure(*pyhive_proc->proc->user, &id->hive_id, 0);
        if( !self ) {
            PyErr_SetString(pyhive::RuntimeError, "Failed to find HIVE type with this ID");
            return NULL;
        }
    } else if( PyUnicode_Check(arg) ) {
        self = type_dic.ensure(*pyhive_proc->proc->user, 0, PyUnicode_AsUTF8AndSize(arg, NULL));
        if( !self ) {
            PyErr_SetString(pyhive::RuntimeError, "Failed to find HIVE type with this name");
            return NULL;
        }
    }

    Py_XDECREF(id);
    Py_XINCREF(self);
    if( self ) {
        PyErr_Clear();
    }

    return (PyObject*)self;
}

static PyObject* Type_repr(pyhive::Type * self)
{
    sStr buf("<pyhive.Type ");
    self->utype->id().print(buf);
    buf.printf(" %s at %p>", self->utype->name(), self);
    return PyUnicode_FromString(buf.ptr());
}

static PyObject * Type_name_match(pyhive::Type * self, PyObject * args, PyObject * kwds)
{
    const char * query = 0;
    static const char * kwlist[] = { "query", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &query) ) {
        return NULL;
    }

    if( self->utype->nameMatch(query) ) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * Type_is_descendent_of(pyhive::Type * self, PyObject * args, PyObject * kwds)
{
    PyObject * rhs = 0;
    static const char * kwlist[] = { "rhs", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "O", (char**)kwlist, &rhs) ) {
        return NULL;
    }

    pyhive::Type * rhs_type = pyhive::Type::check(rhs);
    if( !rhs_type ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.Type argument expected");
        return NULL;
    }

    if( self->utype->isDescendentOf(rhs_type->utype) ) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * Type_find(PyObject * cls, PyObject * args, PyObject * kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return NULL;
    }

    const char * query = 0;
    static const char * kwlist[] = { "query", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &query) ) {
        return NULL;
    }

    if( !query ) {
        query = "";
    }

    sVec<const sUsrType2 *> utypes;
    sUsrType2::find(*pyhive_proc->proc->user, &utypes, query);

    PyObject * lst = PyList_New(utypes.dim());
    for(idx i = 0; i < utypes.dim(); i++) {
        if( pyhive::Type * utype_obj = pyhive::Type::ensure(utypes[i]->id()) ) {
            PyList_SET_ITEM(lst, i, (PyObject*)utype_obj);
        } else {
            Py_DECREF(lst);
            return 0;
        }
    }

    return lst;
}

static PyMethodDef Type_methods[] = {
    { "name_match", (PyCFunction)Type_name_match, METH_VARARGS | METH_KEYWORDS,
      "name_match(query)\nCheck if type's name matches a query string\n\n"\
      ":arg query: comma-separated list of regexps, each optionally prefixed with `'!'` to negate, or suffixed with `'+'` to check ancestors\n"
      ":type query: str\n"\
      ":returns: `True` or `False`\n"\
      ":raises TypeError: if parameters are of the wrong type\n"
    },
    { "is_descendent_of", (PyCFunction)Type_is_descendent_of, METH_VARARGS | METH_KEYWORDS,
      "is_descendent_of(rhs)\nCheck if type is a descendent of another one\n\n"\
      ":arg rhs: type object\n"
      ":type rhs: `pyhive.Type`\n"\
      ":returns: `True` or `False`\n"\
      ":raises TypeError: if parameters are of the wrong type\n"
    },
    { "find", (PyCFunction)Type_find, METH_CLASS | METH_VARARGS | METH_KEYWORDS,
      "find(query)\nList of types whose names match the query\n\n"\
      ":arg query: comma-separated list of regexps, each optionally prefixed with `'!'` to negate, or suffixed with `'+'` to retrieve all children of match; "\
      "`''` is equivalent to `'^base_user_type$+'`\n"\
      ":type query: str\n"\
      ":returns: list of `pyhive.Type` objects\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
    },
    { NULL }
};

static PyObject * Type_get_id(pyhive::Type * self, void * closure)
{
    pyhive::Id * id_obj = pyhive::Id::create();
    id_obj->hive_id = self->utype->id();
    return (PyObject*)id_obj;
}

static PyObject * Type_get_name(pyhive::Type * self, void * closure)
{
    return PyUnicode_FromString(self->utype->name());
}

static PyObject * Type_get_title(pyhive::Type * self, void * closure)
{
    const char * s = self->utype->title();
    return PyUnicode_FromString(s ? s : "");
}

static PyObject * Type_get_description(pyhive::Type * self, void * closure)
{
    const char * s = self->utype->description();
    return PyUnicode_FromString(s ? s : "");
}

static PyObject * Type_get_parents(pyhive::Type * self, void * closure)
{
    if( !self->cached_parents ) {
        idx nparents = self->utype->dimParents();
        self->cached_parents = PyList_New(nparents);
        for(idx i = 0; i < nparents; i++) {
            PyList_SET_ITEM(self->cached_parents, i, (PyObject*)pyhive::Type::ensure(self->utype->getParent(i)->id()));
        }
    }
    Py_INCREF(self->cached_parents);
    return self->cached_parents;
}

static PyObject * Type_get_children(pyhive::Type * self, void * closure)
{
    if( !self->cached_parents ) {
        idx nchildren = self->utype->dimChildren();
        self->cached_children = PyList_New(nchildren);
        for(idx i = 0; i < nchildren; i++) {
            PyList_SET_ITEM(self->cached_children, i, (PyObject*)pyhive::Type::ensure(self->utype->getChild(i)->id()));
        }
    }
    Py_INCREF(self->cached_children);
    return self->cached_children;
}

static PyObject * Type_is_virtual(pyhive::Type * self, void * closure)
{
    if( self->utype->isVirtual() ) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * Type_is_user(pyhive::Type * self, void * closure)
{
    if( self->utype->isUser() ) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * Type_is_system(pyhive::Type * self, void * closure)
{
    if( self->utype->isSystem() ) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * Type_get_field_names(pyhive::Type * self, void * closure)
{
    if( !self->cached_field_names ) {
        pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
        idx nfields = self->utype->dimFields(*pyhive_proc->proc->user);
        self->cached_field_names = PyList_New(nfields);
        for(idx i = 0; i < nfields; i++) {
            const char * s = self->utype->getField(*pyhive_proc->proc->user, i)->name();
            PyList_SET_ITEM(self->cached_field_names, i, PyUnicode_FromString(s ? s : ""));
        }
    }
    Py_INCREF(self->cached_field_names);
    return self->cached_field_names;
}

static PyGetSetDef Type_getsetters[] = {
    { (char*)"id", (getter)Type_get_id, NULL, (char*)"Type's HIVE ID (`pyhive.Id`)", NULL },
    { (char*)"name", (getter)Type_get_name, NULL, (char*)"Type's name (`str`)", NULL },
    { (char*)"title", (getter)Type_get_title, NULL, (char*)"Type's title (`str`)", NULL },
    { (char*)"description", (getter)Type_get_description, NULL, (char*)"Type's description (`str`)", NULL },
    { (char*)"parents", (getter)Type_get_parents, NULL, (char*)"Type's parents (list of `pyhive.Type` objects)", NULL },
    { (char*)"children", (getter)Type_get_children, NULL, (char*)"Type's children (list of `pyhive.Type` objects)", NULL },
    { (char*)"is_virtual", (getter)Type_is_virtual, NULL, (char*)"Is the type virtual? (`bool`)", NULL },
    { (char*)"is_user", (getter)Type_is_user, NULL, (char*)"Is the type a descendent of base user type? (`bool`)", NULL },
    { (char*)"is_system", (getter)Type_is_system, NULL, (char*)"Is the type a descendent of base system type? (`bool`)", NULL },
    { (char*)"field_names", (getter)Type_get_field_names, NULL, (char*)"List of field names (list of strings)", NULL },
    { NULL }
};

PyTypeObject TypeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.Type",
    sizeof(pyhive::Type),
    0,
    0,
    0,
    0,
    0,
    0,
    (reprfunc)Type_repr,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    Py_TPFLAGS_DEFAULT,
    "HIVE object type\n\n"\
    "Initialized from the type name or type's HIVE ID (either as string or `pyhive.Id` object):\n"\
    "    >>> pyhive.Type('u-file')\n"\
    "    <pyhive.Type type.19 u-file at 0x7fbf19e801d0>\n"\
    "    >>> pyhive.Type(pyhive.Id('type.19'))\n"\
    "    <pyhive.Type type.19 u-file at 0x7fbf19e801d0>\n"\
    "    >>> pyhive.Type('type.19')\n"\
    "    <pyhive.Type type.19 u-file at 0x7fbf19e801d0>\n"\
    "    >>> pyhive.Type('type.9999')\n"\
    "    Traceback (most recent call last):\n"\
    "      File \"<stdin>\", line 1, in <module>\n"\
    "    pyhive.RuntimeError: Failed to find HIVE type with this ID",
    0,
    0,
    0,
    0,
    0,
    0,
    Type_methods,
    0,
    Type_getsetters,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    Type_new,
};

pyhive::Type * TypeDic::ensure(const sUsr & user, const sHiveId * id, const char * name)
{
    const sUsrType2 * utype = id ? sUsrType2::ensure(user, *id) : sUsrType2::ensure(user, name);
    if( !utype ) {
        return 0;
    }
    if( !id ) {
        id = &utype->id();
    }

    pyhive::Type ** pt = _dic.get(id, sizeof(sHiveId));
    if( !pt ) {
        pt = _dic.set(id, sizeof(sHiveId));
        *pt = (pyhive::Type*)PyType_GenericAlloc(&TypeType, 0);
        (*pt)->utype = utype;
        (*pt)->cached_field_names = 0;
        (*pt)->cached_parents = 0;
        (*pt)->cached_children = 0;
    }
    return *pt;
}

TypeDic::~TypeDic()
{
    for(idx i=0; i<_dic.dim(); i++) {
        pyhive::Type * t = *_dic.ptr(i);
        Py_XDECREF(t->cached_field_names);
        Py_XDECREF(t->cached_parents);
        Py_XDECREF(t->cached_children);
        Py_XDECREF(t);
    }
}

bool pyhive::Type::typeinit(PyObject * mod)
{
    if( PyType_Ready(&TypeType) < 0 ) {
        return false;
    }
    Py_INCREF(&TypeType);
    PyModule_AddObject(mod, "Type", (PyObject*)&TypeType);
    return true;
}

pyhive::Type * pyhive::Type::check(PyObject * o)
{
    if( o && o->ob_type == &TypeType ) {
        return (pyhive::Type*)o;
    } else {
        return NULL;
    }
}

pyhive::Type * pyhive::Type::ensure(const sHiveId & id)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc || !pyhive_proc->proc || !pyhive_proc->proc->user ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return NULL;
    }

    pyhive::Type * self = type_dic.ensure(*pyhive_proc->proc->user, &id, 0);
    if( !self ) {
        PyErr_SetString(PyExc_TypeError, "Failed to find HIVE type with this ID");
        return NULL;
    }

    Py_INCREF(self);
    return self;
}
