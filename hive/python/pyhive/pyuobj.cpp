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

#include <datetime.h>

#include <ulib/utype2.hpp>

using namespace slib;

static PyObject * Obj_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::Obj * self = (pyhive::Obj*)type->tp_alloc(type, 0);
    self->uobj = 0;
    return (PyObject*)self;
}

static void Obj_dealloc(pyhive::Obj *self)
{
    delete self->uobj;
    self->ob_type->tp_free((PyObject*)self);
}

static int Obj_init(pyhive::Obj *self, PyObject * args, PyObject * kwds);

static PyObject* Obj_repr(pyhive::Obj * self)
{
    sStr buf("<%s ", self->ob_type->tp_name);
    if( self->uobj ) {
        self->uobj->Id().print(buf);
        buf.addString(" of type ");
        buf.addString(self->uobj->getType()->name());
        buf.addString(" ");
    }
    buf.printf("at %p>", self);
    return PyString_FromString(buf.ptr());
}

static PyObject * Obj_get_id(pyhive::Obj * self, void * closure)
{
    pyhive::Id * id_obj = pyhive::Id::create();
    if( self->uobj ) {
        id_obj->hive_id = self->uobj->Id();
    }
    return (PyObject*)id_obj;
}

static PyObject * Obj_get_type(pyhive::Obj * self, void * closure)
{
    return (PyObject*)pyhive::Type::find(self->uobj->getType()->id());
}

static PyObject * FileObj_get_file_path(pyhive::Obj * self, void * closure)
{
    if( sUsrFile * ufile = dynamic_cast<sUsrFile*>(self->uobj) ) {
        sStr buf;
        if( const char * path = ufile->getFile(buf) ) {
            return PyString_FromString(path);
        } else {
            PyErr_SetString(PyExc_SystemError, "Failed to find primary file for HIVE file object");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "HIVE object is not a file object, has no primary filels");
        return NULL;
    }
}

static PyObject * Obj_files(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    const char * mask = "*";
    static const char * kwlist[] = { "mask", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|z:pyhive.Obj.files", (char**)kwlist, &mask) ) {
        return NULL;
    }

    if( !mask ) {
        mask = "*";
    }
    sDir file_list;
    idx nfiles = self->uobj ? self->uobj->files(file_list, sFlag(sDir::bitFiles), mask) : 0;
    PyObject * out = PyList_New(nfiles);
    for(idx i = 0; i < nfiles; i++) {
        PyObject * s = PyString_FromString(file_list.getEntryPath(i));
        PyList_SET_ITEM(out, i, s);
    }
    return out;
}

static PyObject * Obj_add_file_path(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    if( !self->uobj ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.Obj object has not been initialized");
        return NULL;
    }

    const char * name = 0;
    int overwrite = 0;
    static const char * kwlist[] = { "name", "overwrite", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "si:pyhive.Obj.add_file_path", (char**)kwlist, &name, &overwrite) ) {
        return NULL;
    }

    sStr buf;
    if( const char * path = self->uobj->addFilePathname(buf, overwrite, name) ) {
        return PyString_FromString(path);
    } else {
        PyErr_SetString(PyExc_SystemError, "Failed to add file to HIVE object");
        return NULL;
    }
}

static PyObject * Obj_get_file_path(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    if( !self->uobj ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.Obj object has not been initialized");
        return NULL;
    }

    const char * name = 0;
    static const char * kwlist[] = { "name", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s:pyhive.Obj.get_file_path", (char**)kwlist, &name) ) {
        return NULL;
    }

    sStr buf;
    if( const char * path = self->uobj->getFilePathname(buf, name) ) {
        return PyString_FromString(path);
    } else {
        PyErr_SetString(PyExc_SystemError, "Failed to retrieve file from HIVE object");
        return NULL;
    }
}

static PyObject * Obj_del_file_path(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    if( !self->uobj ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.Obj object has not been initialized");
        return NULL;
    }

    const char * name = 0;
    static const char * kwlist[] = { "name", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s:pyhive.Obj.del_file_path", (char**)kwlist, &name) ) {
        return NULL;
    }

    if( self->uobj->delFilePathname(name) ) {
        Py_RETURN_TRUE;
    } else {
        PyErr_SetString(PyExc_SystemError, "Failed to delete file from HIVE object");
        return NULL;
    }
}

static PyObject * Obj_parse_field_value(const char * s, sUsrTypeField::EType type)
{
    if( !s ) {
        s = sStr::zero;
    }

    sVariant var;
    struct tm tm;
    sSet(&tm);

    switch( type ) {
        case sUsrTypeField::eString:
        case sUsrTypeField::eUrl:
        case sUsrTypeField::eText:
        case sUsrTypeField::eFile:
            return PyString_FromString(s);
        case sUsrTypeField::eInteger:
            return pyhive::idx2py(strtoidx((char*)s, 0, 10));
        case sUsrTypeField::eReal:
            return PyFloat_FromDouble(strtod(s, 0));
        case sUsrTypeField::eBool:
            if( sString::parseBool(s) ) {
                Py_RETURN_TRUE;
            } else {
                Py_RETURN_FALSE;
            }
        case sUsrTypeField::eDate:
            return pyhive::parseDate(s);
        case sUsrTypeField::eTime:
            return pyhive::parseTime(s);
        case sUsrTypeField::eDateTime:
            return pyhive::parseDateTime(s);
        case sUsrTypeField::eObj:
        {
            pyhive::Id * id = pyhive::Id::create();
            idx len_parsed = id->hive_id.parse(s);
            if( s[len_parsed] && !isspace(s[len_parsed]) ) {
                id->hive_id.reset();
            }
            return (PyObject*)id;
        }
        default:
            Py_RETURN_NONE;
    }
}

static const char * Obj_print_field_value(sStr & buf, sUsrTypeField::EType type, PyObject * val)
{
    idx pos = buf.length();
    switch( type ) {
        case sUsrTypeField::eString:
        case sUsrTypeField::eUrl:
        case sUsrTypeField::eText:
        case sUsrTypeField::eFile:
            if( PyString_Check(val) ) {
                buf.addString(PyString_AsString(val));
            } else if( PyLong_Check(val) ) {
                buf.addNum((idx)PyLong_AsLongLong(val));
            } else if( PyInt_Check(val) ) {
                buf.addNum((idx)PyInt_AsLong(val));
            } else if( val == Py_None ) {
                buf.add0cut();
            } else {
                PyErr_SetString(PyExc_TypeError, "String value expected");
                return 0;
            }
            break;
        case sUsrTypeField::eInteger:
            if( PyLong_Check(val) ) {
                buf.addNum((idx)PyLong_AsLongLong(val));
            } else if( PyInt_Check(val) ) {
                buf.addNum((idx)PyInt_AsLong(val));
            } else if( val == Py_None ) {
                buf.add0cut();
            } else {
                PyErr_SetString(PyExc_TypeError, "Integer value expected");
                return 0;
            }
            break;
        case sUsrTypeField::eReal:
            if( PyFloat_Check(val) ) {
                buf.printf("%lf", PyFloat_AsDouble(val));
            } else if( PyLong_Check(val) ) {
                buf.addNum((idx)PyLong_AsLongLong(val));
            } else if( PyInt_Check(val) ) {
                buf.addNum((idx)PyInt_AsLong(val));
            } else if( val == Py_None ) {
                buf.add0cut();
            } else {
                PyErr_SetString(PyExc_TypeError, "Numeric value expected");
                return 0;
            }
            break;
        case sUsrTypeField::eBool:
            if( val == Py_True ) {
                buf.printf("true");
            } else if( val == Py_False || val == Py_None ) {
                buf.add0cut();
            } else {
                PyErr_SetString(PyExc_TypeError, "Boolean value expected");
                return 0;
            }
            break;
        case sUsrTypeField::eDate:
            return pyhive::printDate(buf, val);
        case sUsrTypeField::eTime:
            return pyhive::printTime(buf, val);
        case sUsrTypeField::eDateTime:
            return pyhive::printDateTime(buf, val);
        case sUsrTypeField::eObj:
            if( pyhive::Id * id = pyhive::Id::check(val) ) {
                id->hive_id.print(buf);
            } else {
                id = pyhive::Id::create();
                if( id->init(val) ) {
                    id->hive_id.print(buf);
                    Py_DECREF(id);
                } else {
                    Py_DECREF(id);
                    return 0;
                }
            }
            break;
        default:
            return 0;
    }

    return buf.ptr(pos);
}

static PyObject * Obj_prop_get(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    if( !self->uobj ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.Obj object has not been initialized");
        return NULL;
    }

    const char * name = 0;
    static const char * kwlist[] = { "name", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s:pyhive.Obj.prop_get", (char**)kwlist, &name) ) {
        return NULL;
    }

    const sUsrTypeField * fld = self->uobj->propGetTypeField(name);
    if( !fld ) {
        PyErr_SetString(PyExc_KeyError, "invalid field name");
        return NULL;
    }
    if( !fld->canHaveValue() ) {
        PyErr_SetString(PyExc_KeyError, "requested field cannot have a scalar value");
        return NULL;
    }

    sVarSet tbl;
    idx rows = 0;
    if( strcasecmp(name, "_brief") == 0 ) {
        // _brief is special: must be fetched via propBulk since it's generated from other fields
        sVarSet brief_tbl;
        self->uobj->propBulk(brief_tbl, 0, "_brief" __);
        for(idx i=0; i<brief_tbl.rows; i++) {
            const char * row_name = brief_tbl.val(i, 1);
            if( row_name && strcmp(row_name, "_brief") == 0 ) {
                const char * path = brief_tbl.val(i, 2);
                const char * value = brief_tbl.val(i, 3);
                tbl.addRow().addCol(value).addCol(path);
                rows = tbl.rows;
                break;
            }
        }
    } else {
        rows = self->uobj->propGet(name, tbl, true);
    }

    if( fld->isGlobalMulti() ) {
        // for fields that *can* have multiple values, always return a list for safety and developer sanity
        PyObject * lst = PyList_New(rows);
        for(idx ir = 0; ir < rows; ir++) {
            PyList_SET_ITEM(lst, ir, Obj_parse_field_value(tbl.val(ir, 0), fld->type()));
        }
        return lst;
    } else {
        return Obj_parse_field_value(tbl.val(0, 0), fld->type());
    }
}

static PyObject * Obj_prop_set(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    if( !self->uobj ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.Obj object has not been initialized");
        return NULL;
    }

    const char * name = 0;
    PyObject * value = 0;
    static const char * kwlist[] = { "name", "value", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "sO:pyhive.Obj.prop_set", (char**)kwlist, &name, &value) ) {
        return NULL;
    }

    const sUsrTypeField * fld = self->uobj->propGetTypeField(name);
    if( !fld ) {
        PyErr_SetString(PyExc_KeyError, "invalid field name");
        return NULL;
    }
    if( !fld->canHaveValue() ) {
        PyErr_SetString(PyExc_KeyError, "requested field cannot have a scalar value");
        return NULL;
    }
    if( fld->isGlobalMulti() ) {
        PyErr_SetString(PyExc_KeyError, "requested field can have multiple values; setting its value from Python is currently not supported");
        return NULL;
    }
    sStr buf;
    const char * val_string = Obj_print_field_value(buf, fld->type(), value);
    if( !val_string ) {
        return NULL;
    }
    if( !self->uobj->propSet(name, 0, &val_string, 1) ) {
        PyErr_SetString(PyExc_SystemError, "failed to set property value");
        return NULL;
    }
    Py_RETURN_TRUE;
}

static PyMethodDef Obj_methods[] = {
    { "files", (PyCFunction)Obj_files, METH_VARARGS | METH_KEYWORDS,
      "files(mask = '*')\nList of data file names belonging to object (without paths)\n\n"\
      ":arg mask: shell-style glob pattern for which file names to select\n"\
      ":type mask: str\n"\
      ":returns: list of file names\n"\
      ":raises TypeError: if parameters are of the wrong type\n"
    },
    { "add_file_path", (PyCFunction)Obj_add_file_path, METH_VARARGS | METH_KEYWORDS,
      "add_file_path(name, overwrite)\nRetrieve a path for a new file in a HIVE object\n\n"\
      ":arg name: file name (without path) to add\n"\
      ":type name: str\n"\
      ":arg overwrite: if `True` and this file already exists, it will be removed on disk; if `False` and this file already exists, an exception is raised.\n"\
      ":type overwrite: bool\n"\
      ":returns: path to new data file\n"\
      ":rtype: str\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises SystemError: if for whatever reason the path could not be retrieved; or if *overwrite* is `False` and the file exists already\n"
    },
    { "get_file_path", (PyCFunction)Obj_get_file_path, METH_VARARGS | METH_KEYWORDS,
      "get_file_path(name)\nRetrieve a path for an existing file in a HIVE object\n\n"\
      ":arg name: file name (without path) to retrieve\n"\
      ":type name: str\n"\
      ":returns: path to existing data file\n"\
      ":rtype: str\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises SystemError: if for whatever reason the path could not be retrieved\n"
    },
    { "del_file_path", (PyCFunction)Obj_del_file_path, METH_VARARGS | METH_KEYWORDS,
      "del_file_path(name)\nDelete an existing file in a HIVE object\n\n"\
      ":arg name: file name (without path)\n"\
      ":type name: str\n"\
      ":returns: `True`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises SystemError: if the file's path could not be retrieved, or if the file did not exist or could not be deleted\n"
    },
    { "prop_get", (PyCFunction)Obj_prop_get, METH_VARARGS | METH_KEYWORDS,
      "prop_get(name)\nRetrieve property value(s) with specified field name.\n\n"\
      ":arg name: property field name\n"\
      ":type name: str\n"\
      ":returns: if the field can have only one value for this object type, returns a single "
      "value: `str`, `int`, `long`, `float`, `bool`, `pyhive.Id`, `datetime.date`, `datetime.time`, `datetime.datetime`, or `None`; "\
      "if the field potentially can have multiple values, returns a list of values "\
      "(possibly with no elements or only one element).\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises KeyError: if the field name is invalid or named field cannot have scalar values\n"
    },
    { "prop_set", (PyCFunction)Obj_prop_set, METH_VARARGS | METH_KEYWORDS,
      "prop_set(name, value)\nSet property value with specified field name.\n\n"\
      "Setting values of fields that can have multiple values is currently not supported\n"
      "from HIVE Python API.\n\n"\
      ":arg name: property field name\n"\
      ":type name: str\n"\
      ":arg value: value, of type appropriate for the specified field\n"\
      ":returns: `True`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises KeyError: if the field name is invalid or named field cannot have a value or can have multiple values\n"\
      ":raises SystemError: if the property could not be set\n"
    },
    { NULL }
};

static PyGetSetDef Obj_getsetters[] = {
    { (char*)"id", (getter)Obj_get_id, NULL, (char*)"Object's HIVE ID (`pyhive.Id`)", NULL },
    { (char*)"type", (getter)Obj_get_type, NULL, (char*)"Object HIVE type (`pyhive.Type`)", NULL },
    { NULL }
};

static PyGetSetDef FileObj_getsetters[] = {
    { (char*)"id", (getter)Obj_get_id, NULL, (char*)"Object's HIVE ID (`pyhive.Id`)", NULL },
    { (char*)"type", (getter)Obj_get_type, NULL, (char*)"Object HIVE type (`pyhive.Type`)", NULL },
    { (char*)"file_path", (getter)FileObj_get_file_path, NULL, (char*)"HIVE file object's primary file path (str)", NULL },
    { NULL }
};

PyTypeObject ObjType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.Obj",              // tp_name
    sizeof(pyhive::Obj),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)Obj_dealloc,   // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)Obj_repr,        // tp_repr
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
    "HIVE object\n\n"\
    "Initialized from the HIVE ID (as integer or string or `pyhive.Id` object):\n"\
    "    >>> pyhive.Obj(12345)\n"\
    "    <pyhive.Obj 12345 of type svc-align-hexagon at 0x7fa2be26a0f0>\n"\
    "    >>> pyhive.Obj(pyhive.Id('12345'))\n"\
    "    <pyhive.Obj 12345 of type svc-align-hexagon at 0x7fa2be26a0f0>\n"\
    "    >>> pyhive.Obj(123456)\n"\
    "    Traceback (most recent call last):\n"\
    "      File \"<stdin>\", line 1, in <module>\n"\
    "    SystemError: Failed to load HIVE object with requested ID", // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    Obj_methods,               // tp_methods
    0,                         // tp_members
    Obj_getsetters,            // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)Obj_init,        // tp_init
    0,                         // tp_alloc
    Obj_new,                   // tp_new
};

PyTypeObject FileObjType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.FileObj",          // tp_name
    sizeof(pyhive::Obj),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)Obj_dealloc,   // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)Obj_repr,        // tp_repr
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
    "HIVE file object\n\nChild class of `pyhive.Obj`; `pyhive.Obj(id)` will automatically\n"\
    "construct a `pyhive.FileObj` instance if the *id* refers to a HIVE object of HIVE type 'u-file'\n"\
    "or a descendent.",        // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    0,                         // tp_methods
    0,                         // tp_members
    FileObj_getsetters,        // tp_getset
    &ObjType,                  // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)Obj_init,        // tp_init
    0,                         // tp_alloc
    Obj_new,                   // tp_new
};

static int Obj_init(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return -1;
    }

    if( self->uobj ) {
        delete self->uobj;
        self->uobj = 0;
    }

    PyObject * id_arg = 0;
    if( !PyArg_ParseTuple(args, "O", &id_arg) ) {
        return -1;
    }

    pyhive::Id * id_obj = pyhive::Id::check(id_arg);
    if( id_obj ) {
        Py_INCREF(id_obj);
    } else {
        id_obj = pyhive::Id::create();
        if( !id_obj->init(id_arg) ) {
            Py_DECREF(id_obj);
            return -1;
        }
    }

    self->uobj = pyhive_proc->proc->user->objFactory(id_obj->hive_id);
    Py_DECREF(id_obj);
    id_obj = 0;

    if( !self->uobj ) {
        PyErr_SetString(PyExc_SystemError, "Failed to load HIVE object with requested ID");
        return -1;
    }

    sUsrFile* ufile = dynamic_cast<sUsrFile*>(self->uobj);
    if( ufile && self->ob_type == &ObjType ) {
        // if we got a u-file, auto-cast to FileObj
        self->ob_type = &FileObjType;
    } else if( !ufile && self->ob_type == &FileObjType ) {
        // if we did not get a u-file and requested a FileObj, error out
        delete self->uobj;
        self->uobj = 0;
        PyErr_SetString(PyExc_SystemError, "HIVE object with requested ID is not a HIVE file object");
        return -1;
    }
    return 0;
}

//static
bool pyhive::Obj::typeinit(PyObject * mod)
{
    if( PyType_Ready(&ObjType) < 0 ) {
        return false;
    }
    Py_INCREF(&ObjType);
    PyModule_AddObject(mod, "Obj", (PyObject*)&ObjType);
    if( PyType_Ready(&FileObjType) < 0 ) {
        return false;
    }
    Py_INCREF(&FileObjType);
    PyModule_AddObject(mod, "FileObj", (PyObject*)&FileObjType);
    return true;
}

//static
pyhive::Obj * pyhive::Obj::check(PyObject * o)
{
    if( o && (o->ob_type == &ObjType || o->ob_type == &FileObjType) ) {
        return (pyhive::Obj*)o;
    } else {
        return NULL;
    }
}

//static
pyhive::Obj * pyhive::Obj::create()
{
    pyhive::Obj * self = (pyhive::Obj*)Obj_new(&ObjType, 0, 0);
    Py_XINCREF(self);
    return self;
}

bool pyhive::Obj::init(PyObject * arg)
{
    PyObject * args = Py_BuildValue("(O)", arg);
    bool ret = !Obj_init(this, args, 0);
    Py_XDECREF(args);
    return ret;
}
