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
#include <violin/hivespecobj.hpp>
#include <ssci/bio/tax-ion.hpp>

using namespace slib;

static PyObject * Ion_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::Ion * self = (pyhive::Ion*)type->tp_alloc(type, 0);
    self->ion = 0;
    self->obj_id.reset();
    self->name.cut(0);
    return (PyObject*)self;
}

static void Ion_dealloc(pyhive::Ion *self)
{
    delete self->ion;
    self->ob_type->tp_free((PyObject*)self);
}

static int Ion_init(pyhive::Ion *self, PyObject * args, PyObject * kwds);

static PyObject * Ion_repr(pyhive::Ion * self)
{
    sStr buf("<%s ", self->ob_type->tp_name);
    if( self->name.length() ) {
        if( self->obj_id ) {
            self->obj_id.print(buf);
            buf.addString("/");
        }
        buf.addString(self->name.ptr(), self->name.length());
        buf.addString(" ");
    }
    buf.printf("at %p>", self);
    return PyString_FromString(buf.ptr());
}

static PyObject * Ion_get_name(pyhive::Ion * self, void * closure)
{
    if( self->name.ptr() ) {
        return PyString_FromString(self->name.ptr());
    } else {
        Py_RETURN_NONE;
    }
}

static PyGetSetDef Ion_getsetters[] = {
    { (char*)"name", (getter)Ion_get_name, NULL, (char*)"ION name (`str`)", NULL },
    { NULL }
};

PyTypeObject IonType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.Ion",              // tp_name
    sizeof(pyhive::Ion),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)Ion_dealloc,   // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)Ion_repr,        // tp_repr
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
    "ION graph database handle\n\n"\
    "Initialized from a HIVE ID (as integer or string or `pyhive.Id` object) and basename, or as full path:\n"\
    "   >>> pyhive.Ion(12345, 'foobar')\n"
    "   <pyhive.Ion 12345/foobar.ion at 0x7fa2be26a0f0>\n"\
    "   >>> pyhive.Ion(path='/tmp/dir/my_ions/foobar.ion')\n"\
    "   <pyhive.Ion foobar.ion at 0x7fa2be26a0f0>\n",   // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    0,                         // tp_methods
    0,                         // tp_members
    Ion_getsetters,            // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)Ion_init,        // tp_init
    0,                         // tp_alloc
    Ion_new,                   // tp_new
};

static int Ion_init(pyhive::Ion *self, PyObject * args, PyObject * kwds)
{
    if( self->ion ) {
        delete self->ion;
        self->ion = 0;
    }
    self->name.cut0cut();
    self->obj_id.reset();

    static const char * kwlist[] = { "id", "basename", "path", NULL };
    PyObject * id_arg = 0;
    const char * basename_arg = 0, * path_arg = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|Oss", (char**)kwlist, &id_arg, &basename_arg, &path_arg) ) {
        return -1;
    }

    if( id_arg && basename_arg ) {
        pyhive::Obj obj;
        if( !obj.init(id_arg) ) {
            return -1;
        }
        sStr ion_path;
        if( !obj.uobj || !obj.uobj->getFilePathname(ion_path, "%s.ion", basename_arg) ) {
            sStr err_string("Failed to find ION database %s in object %s", basename_arg, obj.uobj ? obj.uobj->Id().print() : "0");
            PyErr_SetString(PyExc_SystemError, err_string.ptr());
            return -1;
        }
        self->ion = new sIon(ion_path.ptr(), sMex::fReadonly);
        if( !self->ion->ok() ) {
            delete self->ion;
            self->ion = 0;
            sStr err_string("Failed to load ION database %s from object %s", basename_arg, obj.uobj ? obj.uobj->Id().print() : "0");
            PyErr_SetString(PyExc_SystemError, err_string.ptr());
            return -1;
        }
    } else if( path_arg ) {
        self->ion = new sIon(path_arg, sMex::fReadonly);
        if( !self->ion->ok() ) {
            delete self->ion;
            self->ion = 0;
            sStr err_string("Failed to load ION database at %s", path_arg);
            PyErr_SetString(PyExc_SystemError, err_string.ptr());
            return -1;
        }
        self->name.cutAddString(0, sFilePath::nextToSlash(path_arg));
    } else {
        PyErr_SetString(PyExc_TypeError, "Either id and basename arguments, or path argument, must be specified");
        return -1;
    }

    return 0;
}

//static
bool pyhive::Ion::typeinit(PyObject * mod)
{
    if( PyType_Ready(&IonType) < 0 ) {
        return false;
    }
    Py_INCREF(&IonType);
    PyModule_AddObject(mod, "Ion", (PyObject*)&IonType);

    return true;
}

static PyObject * TaxIon_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::TaxIon * self = (pyhive::TaxIon*)type->tp_alloc(type, 0);
    self->head.ion = 0;
    self->head.obj_id.reset();
    self->head.name.cut(0);
    self->tax_db_obj = 0;
    self->ptax_ion = 0;
    return (PyObject*)self;
}

static void TaxIon_dealloc(pyhive::TaxIon *self)
{
    Py_XDECREF(self->tax_db_obj);
    delete self->ptax_ion;
    self->head.ob_type->tp_free((PyObject*)self);
}

static int TaxIon_init(pyhive::TaxIon *self, PyObject * args, PyObject * kwds);

static PyObject* TaxIon_repr(pyhive::TaxIon * self)
{
    sStr buf("<%s ", self->head.ob_type->tp_name);
    pyhive::Obj * obj = pyhive::Obj::check(self->tax_db_obj);
    if( obj && obj->uobj ) {
        obj->uobj->Id().print(buf);
        buf.addString(" ");
    }
    buf.printf("at %p>", self);
    return PyString_FromString(buf.ptr());
}

static PyObject * TaxIon_get_tax_id_info(pyhive::TaxIon * self, PyObject * args, PyObject * kwds)
{
    static const char * kwlist[] = { "tax_id", NULL };
    udx tax_id;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "K", (char**)kwlist, &tax_id) ) {
        return 0;
    }
    sStr buf("%" UDEC, tax_id);
    const char * tax_result = self->ptax_ion->getTaxIdInfo(buf.ptr(), buf.length());
    idx tax_result_len = sLen(tax_result);
    if( !tax_result_len ) {
        PyErr_SetString(PyExc_ValueError, "Taxonomy ID not found in database");
        return 0;
    }
    sTxtTbl result_parser;
    result_parser.parseOptions().flags = 0; // no header!
    result_parser.setBuf(tax_result, tax_result_len);
    if( !result_parser.parse() || result_parser.cols() < 5  ) {
        PyErr_SetString(PyExc_SystemError, "Taxonomy database result in unexpected format");
        return 0;
    }
    PyObject * ret_dict = PyDict_New();

    PyObject * ret_tax_id = pyhive::udx2py(result_parser.uval(0, 0));
    PyDict_SetItemString(ret_dict, "tax_id", ret_tax_id);
    Py_DECREF(ret_tax_id);

    PyObject * ret_parent_tax_id = pyhive::udx2py(result_parser.uval(0, 1));
    PyDict_SetItemString(ret_dict, "parent_tax_id", ret_parent_tax_id);
    Py_DECREF(ret_parent_tax_id);

    PyObject * ret_rank = PyString_FromString(result_parser.printCell(buf, 0, 2));
    PyDict_SetItemString(ret_dict, "rank", ret_rank);
    Py_DECREF(ret_rank);

    PyObject * ret_name = PyString_FromString(result_parser.printCell(buf, 0, 3));
    PyDict_SetItemString(ret_dict, "name", ret_name);
    Py_DECREF(ret_name);

    PyObject * ret_num_children = pyhive::idx2py(result_parser.ival(0, 4));
    PyDict_SetItemString(ret_dict, "num_children", ret_num_children);
    Py_DECREF(ret_num_children);

    return ret_dict;
}

static PyObject * TaxIon_get_tax_ids_by_name(pyhive::TaxIon * self, PyObject * args, PyObject * kwds)
{
    static const char * kwlist[] = { "name", "limit", NULL };
    const char * name = 0;
    idx limit = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s|L", (char**)kwlist, &name, &limit) ) {
        return 0;
    }

    const char * tax_result = self->ptax_ion->getTaxIdsByName(name, limit);
    idx tax_result_len = sLen(tax_result);
    if( !tax_result_len ) {
        tax_result = sStr::zero;
    }

    sTxtTbl result_parser;
    sStr buf;
    result_parser.parseOptions().flags = 0; // no header!
    result_parser.setBuf(tax_result, tax_result_len);
    if( (tax_result_len && !result_parser.parse()) || (result_parser.rows() && result_parser.cols() < 2) ) {
        PyErr_SetString(PyExc_SystemError, "Taxonomy database result in unexpected format");
        return 0;
    }

    PyObject * ret_list = PyList_New(result_parser.rows());
    for(idx ir = 0; ir < result_parser.rows(); ir++) {
        PyObject * ret_tuple = PyTuple_New(2);

        if( udx tax_id = result_parser.uval(ir, 0) ) {
            PyTuple_SET_ITEM(ret_tuple, 0, pyhive::udx2py(tax_id));
        } else {
            PyErr_SetString(PyExc_SystemError, "Taxonomy database result in unexpected format");
            Py_DECREF(ret_tuple);
            Py_DECREF(ret_list);
            ret_list = 0;
            break;
        }

        buf.cut0cut(0);
        if( result_parser.printCell(buf, ir, 1) ) {
            PyTuple_SET_ITEM(ret_tuple, 1, PyString_FromString(buf.ptr()));
        } else {
            PyErr_SetString(PyExc_SystemError, "Taxonomy database result in unexpected format");
            Py_DECREF(ret_tuple);
            Py_DECREF(ret_list);
            ret_list = 0;
            break;
        }

        PyList_SET_ITEM(ret_list, ir, ret_tuple);
    }

    return ret_list;
}

static PyMethodDef TaxIon_methods[] = {
    {
        "get_tax_id_info", (PyCFunction)TaxIon_get_tax_id_info,
        METH_VARARGS | METH_KEYWORDS,
        "get_tax_id_info(tax_id)\n\nRetrieve information about a TaxID\n\n"\
        ":arg tax_id: NCBI taxonomy ID\n:type tax_id: int\n"\
        ":returns: Dict of the following form: `{'tax_id': 9606, 'parent_tax_id': 9605, 'rank': 'species', 'name': 'Homo sapiens', 'num_children': 2}`"
    },
    {
        "get_tax_ids_by_name", (PyCFunction)TaxIon_get_tax_ids_by_name,
        METH_VARARGS | METH_KEYWORDS,
        "get_tax_ids_by_name(name, limit = 0)\n\nRetrieve list of TaxIDs and names matching a name pattern\n\n"\
        ":arg name: name pattern for which to search (POSIX regular expression syntax)\n:type name: str\n"\
        ":arg limit: max number of items to retrieve, or 0 (default) to retrieve all\n:type limit: int\n"\
        ":returns: list of (TaxID, name) tuples, for example: `[(9606, 'Homo sapiens'), (9606, 'Homo sapiens Linnaeus'), (63221, 'Homo sapiens neanderthalensis')]`"
    },
    { NULL }
};

static PyObject * TaxIon_get_obj(pyhive::TaxIon * self, void * closure)
{
    Py_INCREF(self->tax_db_obj);
    return self->tax_db_obj;
}

static PyGetSetDef TaxIon_getsetters[] = {
    { (char*)"obj", (getter)TaxIon_get_obj, NULL, (char*)"Taxonomy database HIVE object (:py:class:`pyhive.Obj` instance)", NULL },
    { NULL }
};

PyTypeObject TaxIonType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.TaxIon",           // tp_name
    sizeof(pyhive::TaxIon),    // tp_basicsize
    0,                         // tp_itemsize
    (destructor)TaxIon_dealloc,// tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)TaxIon_repr,     // tp_repr
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
    "HIVE ION interface to NCBI taxonomy database\n\n"\
    "Child class of `pyhive.Ion`", // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    TaxIon_methods,            // tp_methods
    0,                         // tp_members
    TaxIon_getsetters,         // tp_getset
    &IonType,                  // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)TaxIon_init,     // tp_init
    0,                         // tp_alloc
    TaxIon_new,                // tp_new
};

static int TaxIon_init(pyhive::TaxIon *self, PyObject * args, PyObject * kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return -1;
    }

    if( self->tax_db_obj ) {
        Py_DECREF(self->tax_db_obj);
        self->tax_db_obj = 0;
    }
    if( self->ptax_ion ) {
        delete self->ptax_ion;
        self->ptax_ion = 0;
    }

    sHiveId tax_db_hive_id;
    sStr ionP, error_log;
    if( !sviolin::SpecialObj::findTaxDbIonPath(ionP, *pyhive_proc->proc->user, 0, &tax_db_hive_id, &error_log) ) {
        PyErr_SetString(PyExc_SystemError, error_log.ptr());
        return -1;
    }

    pyhive::Id * tax_db_id = pyhive::Id::create();
    if( !tax_db_id->init(tax_db_hive_id) ) {
        Py_DECREF(tax_db_id);
        return -1;
    }

    pyhive::Obj * tax_db_obj = pyhive::Obj::create();
    if( !tax_db_obj->init(tax_db_id) ) {
        Py_DECREF(tax_db_obj);
        Py_DECREF(tax_db_id);
        return -1;
    }

    Py_DECREF(tax_db_id);
    tax_db_id = 0;

    self->ptax_ion = new sTaxIon(ionP);
    self->tax_db_obj = (PyObject*)tax_db_obj;
    self->head.ion = self->ptax_ion->getIon();
    self->head.obj_id = tax_db_hive_id;
    self->head.name.cutAddString(0, "ncbiTaxonomy.ion");

    return 0;
}

//static
bool pyhive::TaxIon::typeinit(PyObject * mod)
{
    if( PyType_Ready(&TaxIonType) < 0 ) {
        return false;
    }
    Py_INCREF(&TaxIonType);
    PyModule_AddObject(mod, "TaxIon", (PyObject*)&TaxIonType);
    return true;
}

//static
pyhive::TaxIon * pyhive::TaxIon::check(PyObject * o)
{
    if( o && o->ob_type == &TaxIonType ) {
        return (pyhive::TaxIon*)o;
    } else {
        return NULL;
    }
}

//static
pyhive::Ion * pyhive::Ion::check(PyObject * o)
{
    if( o && o->ob_type == &IonType ) {
        return (pyhive::Ion*)o;
    } else if( o && o->ob_type == &TaxIonType ) {
        return &((pyhive::TaxIon*)o)->head;
    } else {
        return NULL;
    }
}

//static
sIon * pyhive::Ion::getIon(PyObject * o)
{
    sIon * ret = NULL;
    if( Ion * ion = Ion::check(o) ) {
        ret = ion->ion;
    }
    if( !ret ) {
        PyErr_SetString(PyExc_ValueError, "expecting valid object of type pyhive.Ion or descendent type (pyhive.TaxIon)");
    }
    return ret;
}
