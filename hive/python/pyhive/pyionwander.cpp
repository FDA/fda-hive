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

#include <ion/sIon.hpp>

using namespace slib;

static PyObject * IonWander_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::IonWander * self = (pyhive::IonWander*)type->tp_alloc(type, 0);
    self->wander = 0;
    self->ions = 0;
    self->result = 0;
    return (PyObject*)self;
}

static void IonWander_dealloc(pyhive::IonWander *self)
{
    delete self->wander;
    Py_XDECREF(self->ions);
    Py_XDECREF(self->result);
    self->ob_type->tp_free((PyObject*)self);
}

static int IonWander_init(pyhive::IonWander *self, PyObject * args, PyObject * kwds);

static PyObject * IonWander_repr(pyhive::IonWander * self)
{
    sStr buf("<%s ", self->ob_type->tp_name);
    if( self->query.length() ) {
        buf.addString("\"");
        buf.addString(self->query.ptr(), self->query.length());
        buf.addString("\" ");
    }
    buf.printf("at %p>", self);
    return PyString_FromString(buf.ptr());
}

static PyObject * IonWander_get_ions(pyhive::IonWander * self, void * closure)
{
    Py_XINCREF(self->ions);
    return self->ions;
}

static PyObject * IonWander_get_query(pyhive::IonWander * self, void * closure)
{
    if( self->query.ptr() ) {
        return PyString_FromString(self->query.ptr());
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject * IonWander_get_result(pyhive::IonWander * self, void * closure)
{
    PyObject * ret = (PyObject*)self->result;
    Py_XINCREF(ret);
    return ret;
}

static PyGetSetDef IonWander_getsetters[] = {
    { (char*)"ions", (getter)IonWander_get_ions, NULL, (char*)"ION handles which will be traversed (`list` of `pyhive.Ion`)", NULL },
    { (char*)"query", (getter)IonWander_get_query, NULL, (char*)"query (`str`)", NULL },
    { (char*)"result", (getter)IonWander_get_result, NULL, (char*)"result buffer (`pyhive.Mex`)", NULL },
    { NULL }
};

static PyObject * IonWander_set_search_template_variable(pyhive::IonWander * self, PyObject * args, PyObject * kwds)
{
    static const char * kwlist[] = { "name", "value", NULL };
    const char * name_arg = 0, * value_arg = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "ss", (char**)kwlist, &name_arg, &value_arg) ) {
        return 0;
    }
    self->wander->setSearchTemplateVariable(name_arg, sLen(name_arg), value_arg, sLen(value_arg));

    Py_RETURN_TRUE;
}

static PyObject * IonWander_reset_result_buf(pyhive::IonWander * self, PyObject * args, PyObject * kwds)
{
    self->wander->resetResultBuf();
    Py_RETURN_TRUE;
}

static PyObject * IonWander_traverse(pyhive::IonWander * self, PyObject * args, PyObject * kwds)
{
    self->wander->traverse();
    Py_RETURN_TRUE;
}

static PyMethodDef TaxIon_methods[] = {
    {
        "set_search_template_variable", (PyCFunction)IonWander_set_search_template_variable,
        METH_VARARGS | METH_KEYWORDS,
        "set_search_template_variable(name, value)\n\n"\
        "Set query template variable value for this traverse\n\n"\
        ":arg name: template variable name (must start with `'$'`)\n:type name: `str`\n"\
        ":arg value: template variable value\n:type value: `str`\n"\
        ":returns: `True`"
    },
    {
        "reset_result_buf", (PyCFunction)IonWander_reset_result_buf,
        METH_NOARGS,
        "reset_result_buf()\n\nReset result buffer (if traversing multiple times, e.g. with different template values)\n\n"\
        ":returns: `True`"
    },
    {
        "traverse", (PyCFunction)IonWander_traverse,
        METH_NOARGS,
        "traverse()\n\nRun the query; results will be appended to `result`\n\n"\
        ":returns: `True`"
    },
    { NULL }
};

PyTypeObject IonWanderType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.IonWander",        // tp_name
    sizeof(pyhive::IonWander), // tp_basicsize
    0,                         // tp_itemsize
    (destructor)IonWander_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)IonWander_repr,  // tp_repr
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
    "ION Wander low-level query interface\n\n"\
    "Initialized from a pyhive.Ion (or list of ions) and query string:\n"\
    "    >>> tax_ion = pyhive.TaxIon()\n"\
    "    <pyhive.TaxIon 123456 at 0x7fa2be26a0e0>\n"\
    "    >>> wander = pyhive.IonWander(tax_ion, \"o=find.taxid_name(taxid=$t, tag='scientific name'); printCSV(o.taxid, o.name)\")\n"
    "    <pyhive.IonWander \"o=find.taxid_name(taxid=$t, tag='scientific name'); printCSV(o.taxid, o.name)\" at 0x7fa2be26a0f0>\n"\
    "    >>> wander.set_search_template_variable('$t', '9606')\n"\
    "    True\n"
    "    >>> wander.traverse()\n"\
    "    True\n"
    "    >>> wander.set_search_template_variable('$t', '9605')\n"\
    "    True\n"
    "    >>> wander.traverse()\n"\
    "    True\n"
    "    >>> print(wander.result)\n"\
    "    9606,Homo sapiens\n"\
    "    9605,Homo\n\n"\
    "Constructor will raise `ValueError` if query string or ION handle is invalid.\n",   // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    TaxIon_methods,            // tp_methods
    0,                         // tp_members
    IonWander_getsetters,      // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)IonWander_init,  // tp_init
    0,                         // tp_alloc
    IonWander_new,             // tp_new
};

static int IonWander_init(pyhive::IonWander *self, PyObject * args, PyObject * kwds)
{
    static const char * kwlist[] = { "ion", "query", NULL };
    PyObject * ions_arg = 0;
    const char * query_arg = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "Os", (char**)kwlist, &ions_arg, &query_arg) ) {
        return -1;
    }

    sIonWander * wander = new sIonWander();

    if( pyhive::Ion * ion = pyhive::Ion::check(ions_arg) ) {
        self->ions = PyList_New(1);
        Py_XINCREF(ion);
        PyList_SET_ITEM(self->ions, 0, ions_arg);
        wander->addIon(ion->ion);
    } else if( PySequence_Check(ions_arg) ) {
        idx len = PySequence_Length(ions_arg);
        for(idx i = 0; i < len; i++) {
            PyObject * item = PySequence_GetItem(ions_arg, i);
            if( !pyhive::Ion::check(item) ) {
                Py_XDECREF(item);
                PyErr_SetString(PyExc_ValueError, "Expected pyhive.Ion or list of ions");
                delete wander;
                return -1;
            }
            Py_XDECREF(item);
        }
        self->ions = PyList_New(len);
        for(idx i = 0; i < len; i++) {
            PyObject * item = PySequence_GetItem(ions_arg, i);
            PyList_SET_ITEM(self->ions, i, item);
            wander->addIon(pyhive::Ion::getIon(item));
        }
    } else {
        PyErr_SetString(PyExc_ValueError, "Expected pyhive.Ion or list of ions");
        delete wander;
        return -1;
    }

    self->query.cutAddString(0, query_arg);

    sIO compile_err_buf;
    compile_err_buf.addString("Failed to compile query: ");
    if( wander->traverseCompile(query_arg, 0, &compile_err_buf) ) {
        compile_err_buf.add0cut();
        PyErr_SetString(PyExc_ValueError, compile_err_buf.ptr());
        delete wander;
        Py_XDECREF(self->ions);
        self->ions = 0;
        return -1;
    }

    self->result = pyhive::Mex::create();
    wander->pTraverseBuf = &self->result->str;

    self->wander = wander;

    return 0;
}

//static
bool pyhive::IonWander::typeinit(PyObject * mod)
{
    if( PyType_Ready(&IonWanderType) < 0 ) {
        return false;
    }
    Py_INCREF(&IonWanderType);
    PyModule_AddObject(mod, "IonWander", (PyObject*)&IonWanderType);

    return true;
}
