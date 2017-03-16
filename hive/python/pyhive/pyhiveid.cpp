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

static PyObject * Id_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::Id * self = (pyhive::Id*)type->tp_alloc(type, 0);
    if( self ) {
        self->hive_id.reset();
    }
    return (PyObject*)self;
}

static int Id_init(pyhive::Id * self, PyObject *args, PyObject *kwds)
{
    PyObject * arg = 0;
    if( !PyArg_ParseTuple(args, "|O", &arg) ) {
        return -1;
    }
    if( !self->init(arg) ) {
        return -1;
    }

    return 0;
}

static PyObject* Id_repr(pyhive::Id * self)
{
    sStr buf("<pyhive.Id ");
    self->hive_id.print(buf);
    buf.printf(" at %p>", self);
    return PyString_FromString(buf.ptr());
}

static PyObject* Id_str(pyhive::Id * self)
{
    sStr buf;
    return PyString_FromString(self->hive_id.print(buf));
}

static int Id_compare(pyhive::Id * o1, pyhive::Id * o2)
{
    idx res = o1->hive_id.cmp(o2->hive_id);
    return res < 0 ? -1 : res > 0 ? 1 : 0;
}

static PyObject * Id_get_obj_id(pyhive::Id * self, void * closure)
{
    return pyhive::udx2py(self->hive_id.objId());
}

static PyObject * Id_get_ion_id(pyhive::Id * self, void * closure)
{
    return pyhive::udx2py(self->hive_id.ionId());
}

static PyObject * Id_get_domain(pyhive::Id * self, void * closure)
{
    char buf[sizeof(udx) + 1];
    sHiveId::decodeDomainId(buf, self->hive_id.domainId());
    return PyString_FromString(buf);
}

static PyGetSetDef Id_getsetters[] = {
    { (char*)"obj_id", (getter)Id_get_obj_id, NULL, (char*)"HIVE object ID (non-negative integer; 0 denotes an invalid or null object)", NULL },
    { (char*)"ion_id", (getter)Id_get_ion_id, NULL, (char*)"HIVE Ion ID (non-negative integer; 0 means that ion is not used)", NULL },
    { (char*)"domain", (getter)Id_get_domain, NULL, (char*)"HIVE domain or namespace (string of 0-8 characters; empty string means normal, locally-created HIVE objects)", NULL },
    { NULL }
};

PyTypeObject IdType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.Id",               // tp_name
    sizeof(pyhive::Id),        // tp_basicsize
    0,                         // tp_itemsize
    0,                         // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    (cmpfunc)Id_compare,       // tp_compare
    (reprfunc)Id_repr,         // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    (reprfunc)Id_str,          // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT,        // tp_flags
    "HIVE identifier\n\n" \
    "Unique identifier for :doc:`HIVE objects <pyuobj>`, :doc:`types <pyutype>`, etc. Consists of 3 parts: domain.obj_id.ion_id\n\nCan be constructed from an integer\n"\
    "(will be treated as the object ID, with ion ID and domain empty) or a string::\n\n"\
    "    >>> pyhive.Id(12345)\n"\
    "    <pyhive.Id 12345 at 0x7f1c4f235670>\n"\
    "    >>> pyhive.Id('12345')\n"\
    "    <pyhive.Id 12345 at 0x7f1c4f2358f0>\n"\
    "    >>> pyhive.Id('type.99')\n"\
    "    <pyhive.Id type.99 at 0x7f1c4f235918>", // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    0,                         // tp_methods
    0,                         // tp_members
    Id_getsetters,             // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)Id_init,         // tp_init
    0,                         // tp_alloc
    Id_new,                    // tp_new
};

//static
bool pyhive::Id::typeinit(PyObject * mod)
{
    if( PyType_Ready(&IdType) < 0 ) {
        return false;
    }
    Py_INCREF(&IdType);
    PyModule_AddObject(mod, "Id", (PyObject*)&IdType);
    return true;
}

//static
pyhive::Id * pyhive::Id::check(PyObject * o)
{
    if( o && o->ob_type == &IdType ) {
        return (pyhive::Id*)o;
    } else {
        return NULL;
    }
}

//static
pyhive::Id * pyhive::Id::create()
{
    pyhive::Id * self = (pyhive::Id*)Id_new(&IdType, 0, 0);
    Py_XINCREF(self);
    return self;
}

bool pyhive::Id::init(PyObject * arg)
{
    if( !arg || arg == Py_None ) {
        hive_id.reset();
        return true;
    } else if( PyLong_Check(arg) ) {
        hive_id.set((udx)0, PyLong_AsUnsignedLongLong(arg), 0);
        return true;
    } else if( PyInt_Check(arg) ) {
        hive_id.set((udx)0, PyInt_AsUnsignedLongMask(arg), 0);
        return true;
    } else if( PyString_Check(arg) ) {
        const char * s = PyString_AsString(arg);
        if( hive_id.parse(s) == sLen(s) ) {
            return true;
        } else {
            PyErr_SetString(PyExc_ValueError, "Invalid HIVE identifier string");
        }
    } else if( pyhive::Id * rhs = pyhive::Id::check(arg) ) {
        hive_id = rhs->hive_id;
        return true;
    } else if( pyhive::Obj * obj = pyhive::Obj::check(arg) ) {
        if( obj->uobj ) {
            hive_id = obj->uobj->Id();
            return true;
        } else {
            PyErr_SetString(PyExc_ValueError, "pyhive.Obj object has not been initialized");
        }
    } else if( pyhive::Type * tp = pyhive::Type::check(arg) ) {
        if( tp->utype ) {
            hive_id = tp->utype->id();
            return true;
        } else {
            PyErr_SetString(PyExc_ValueError, "pyhive.Type object has not been initialized");
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "pyhive.Id cannot be initialized from this argument");
    }

    hive_id.reset();
    return false;
}

bool pyhive::Id::init(const sHiveId & id)
{
    hive_id = id;
    return true;
}

PyObject * pyhive::Id::parseList(PyObject * arg)
{
    PyObject * lst = 0;
    if( PyString_Check(arg) ) {
        char * s = 0;
        Py_ssize_t s_len = 0;
        PyString_AsStringAndSize(arg, &s, &s_len);

        sVec<sHiveId> ids;
        idx len_parsed = 0;
        sHiveId::parseRangeSet(ids, s, s_len, &len_parsed);
        if( len_parsed != s_len ) {
            PyErr_SetString(PyExc_TypeError, "String could not be parsed as list of HIVE IDs");
        } else {
            lst = PyList_New(ids.dim());
            for(idx i = 0; i < ids.dim(); i++) {
                pyhive::Id * id = pyhive::Id::create();
                id->init(ids[i]);
                PyList_SET_ITEM(lst, i, (PyObject*)id);
            }
        }
    } else if( PySequence_Check(arg) ) {
        idx cnt = PySequence_Length(arg);
        lst = PyList_New(cnt);
        for(idx i = 0; i < cnt; i++) {
            PyObject * subarg = PySequence_GetItem(arg, i);
            pyhive::Id * id = pyhive::Id::create();
            if( id->init(subarg) ) {
                PyList_SET_ITEM(lst, i, (PyObject*)id);
                Py_DECREF(subarg);
                subarg = 0;
            } else {
                Py_DECREF(id);
                Py_DECREF(lst);
                Py_DECREF(subarg);
                return 0;
            }
        }
    } else {
        pyhive::Id * id = pyhive::Id::create();
        if( id->init(arg) ) {
            lst = PyList_New(1);
            PyList_SET_ITEM(lst, 0, (PyObject*)id);
        } else {
            Py_DECREF(id);
        }
    }

    return lst;
}
