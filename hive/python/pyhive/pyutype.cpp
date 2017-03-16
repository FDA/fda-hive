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
            // retrieve (allocate if needed) pyhive::Type which is visible to given user with given id or name
            pyhive::Type * ensure(const sUsr & user, const sHiveId * id, const char * name = 0);
            ~TypeDic();
    } type_dic;
}

// relies on type_dic for allocation (if any)
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
            PyErr_SetString(PyExc_SystemError, "Failed to find HIVE type with this ID");
            return NULL;
        }
    } else if( PyString_Check(arg) ) {
        self = type_dic.ensure(*pyhive_proc->proc->user, 0, PyString_AsString(arg));
        if( !self ) {
            PyErr_SetString(PyExc_SystemError, "Failed to find HIVE type with this name");
            return NULL;
        }
    }

    Py_XDECREF(id);
    Py_XINCREF(self);

    return (PyObject*)self;
}

static PyObject* Type_repr(pyhive::Type * self)
{
    sStr buf("<pyhive.Type ");
    self->utype->id().print(buf);
    buf.printf(" %s at %p>", self->utype->name(), self);
    return PyString_FromString(buf.ptr());
}

static PyObject * Type_get_id(pyhive::Type * self, void * closure)
{
    pyhive::Id * id_obj = pyhive::Id::create();
    id_obj->hive_id = self->utype->id();
    return (PyObject*)id_obj;
}

static PyObject * Type_get_name(pyhive::Type * self, void * closure)
{
    return PyString_FromString(self->utype->name());
}

static PyObject * Type_get_title(pyhive::Type * self, void * closure)
{
    const char * s = self->utype->title();
    return PyString_FromString(s ? s : "");
}

static PyObject * Type_get_description(pyhive::Type * self, void * closure)
{
    const char * s = self->utype->description();
    return PyString_FromString(s ? s : "");
}

static PyObject * Type_get_field_names(pyhive::Type * self, void * closure)
{
    if( !self->cached_field_names ) {
        pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
        idx nfields = self->utype->dimFields(*pyhive_proc->proc->user);
        self->cached_field_names = PyTuple_New(nfields);
        for(idx i = 0; i < nfields; i++) {
            const char * s = self->utype->getField(*pyhive_proc->proc->user, i)->name();
            PyTuple_SET_ITEM(self->cached_field_names, i, PyString_FromString(s ? s : ""));
        }
    }
    Py_INCREF(self->cached_field_names);
    return self->cached_field_names;
}

static PyGetSetDef Type_getsetters[] = {
    { (char*)"id", (getter)Type_get_id, NULL, (char*)"Type's HIVE ID (`pyhive.Id`)", NULL },
    { (char*)"name", (getter)Type_get_name, NULL, (char*)"Type's name (str)", NULL },
    { (char*)"title", (getter)Type_get_title, NULL, (char*)"Type's title (str)", NULL },
    { (char*)"description", (getter)Type_get_description, NULL, (char*)"Type's description (str)", NULL },
    { (char*)"field_names", (getter)Type_get_field_names, NULL, (char*)"List of field names (list of strings)", NULL },
    { NULL }
};

PyTypeObject TypeType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.Type",             // tp_name
    sizeof(pyhive::Type),      // tp_basicsize
    0,                         // tp_itemsize
    0,                         // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)Type_repr,       // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    0,                         // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT,        // tp_flags
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
    "    SystemError: Failed to find HIVE type with this ID", // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    0,                         // tp_methods
    0,                         // tp_members
    Type_getsetters,           // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    0,                         // tp_init
    0,                         // tp_alloc
    Type_new,                  // tp_new
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
    }
    return *pt;
}

TypeDic::~TypeDic()
{
    for(idx i=0; i<_dic.dim(); i++) {
        pyhive::Type * t = *_dic.ptr(i);
        Py_XDECREF(t->cached_field_names);
        Py_XDECREF(t);
    }
}

//static
bool pyhive::Type::typeinit(PyObject * mod)
{
    if( PyType_Ready(&TypeType) < 0 ) {
        return false;
    }
    Py_INCREF(&TypeType);
    PyModule_AddObject(mod, "Type", (PyObject*)&TypeType);
    return true;
}

//static
pyhive::Type * pyhive::Type::check(PyObject * o)
{
    if( o && o->ob_type == &TypeType ) {
        return (pyhive::Type*)o;
    } else {
        return NULL;
    }
}

//static
pyhive::Type * pyhive::Type::find(const sHiveId & id)
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

