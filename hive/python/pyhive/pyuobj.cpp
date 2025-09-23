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
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int Obj_init(pyhive::Obj *self, PyObject * args, PyObject * kwds);

static PyObject* Obj_repr(pyhive::Obj * self)
{
    sStr buf("<%s ", Py_TYPE(self)->tp_name);
    if( self->uobj ) {
        self->uobj->Id().print(buf);
        buf.addString(" of type ");
        buf.addString(self->uobj->getType()->name());
        buf.addString(" ");
    }
    buf.printf("at %p>", self);
    return PyUnicode_FromString(buf.ptr());
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
    return (PyObject*)pyhive::Type::ensure(self->uobj->getType()->id());
}

static PyObject * FileObj_get_file_path(pyhive::Obj * self, void * closure)
{
    if( sUsrFile * ufile = dynamic_cast<sUsrFile*>(self->uobj) ) {
        sStr buf;
        if( const char * path = ufile->getFile(buf) ) {
            return PyUnicode_FromString(path);
        } else {
            PyErr_SetString(pyhive::RuntimeError, "Failed to find primary file for HIVE file object");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "HIVE object is not a file object, has no primary filels");
        return NULL;
    }
}

static PyObject * Obj_is_type_of(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
    const char * query = 0;
    static const char * kwlist[] = { "query", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &query) ) {
        return NULL;
    }

    if( !query ) {
        query = "";
    }
    if( self->uobj->isTypeOf(query) ) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
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
        PyObject * s = PyUnicode_FromString(file_list.getEntryPath(i));
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
        return PyUnicode_FromString(path);
    } else {
        PyErr_SetString(pyhive::RuntimeError, "Failed to add file to HIVE object");
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
        return PyUnicode_FromString(path);
    } else {
        PyErr_SetString(pyhive::RuntimeError, "Failed to retrieve file from HIVE object");
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
        PyErr_SetString(pyhive::RuntimeError, "Failed to delete file from HIVE object");
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
            return PyUnicode_FromString(s);
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
            if( PyUnicode_Check(val) ) {
                buf.addString(PyUnicode_AsUTF8AndSize(val, NULL));
            } else if( PyLong_Check(val) ) {
                buf.addNum((idx)PyLong_AsLongLong(val));
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
    int allow_sys_internal = false;
    static const char * kwlist[] = { "name", "allow_sys_internal", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s|i:pyhive.Obj.prop_get", (char**)kwlist, &name, &allow_sys_internal) ) {
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
        sVarSet brief_tbl;
        self->uobj->propBulk(brief_tbl, 0, "_brief" __, allow_sys_internal);
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
        rows = self->uobj->propGet(name, tbl, true, allow_sys_internal);
    }

    if( fld->isGlobalMulti() ) {
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
        PyErr_SetString(pyhive::RuntimeError, "failed to set property value");
        return NULL;
    }
    Py_RETURN_TRUE;
}

static PyObject * Obj_obj_list(PyObject *cls, PyObject * args, PyObject * kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.proc has not been initialized");
        return NULL;
    }

    PyObject * types = 0;
    const char * prop_val = 0;
    const char * prop_name = 0;
    idx start = 0;
    idx cnt = 1000;
    static const char * kwlist[] = { "types", "prop_val", "prop_name", "start", "cnt", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|OzzLL:pyhive.Obj.obj_list", (char**)kwlist, &types, &prop_val, &prop_name, &start, &cnt) ) {
        return NULL;
    }

    sStr type_names_csv;
    if( types ) {
        if( PyUnicode_Check(types) ) {
            type_names_csv.addString(PyUnicode_AsUTF8AndSize(types, NULL));
        } else if( PySequence_Check(types) ) {
            idx len = PySequence_Length(types);
            for(idx i = 0; i < len; i++) {
                if( i ) {
                    type_names_csv.addString(",");
                }
                if( pyhive::Type * type_obj = pyhive::Type::check(types) ) {
                    type_names_csv.printf("^%s$", type_obj->utype->name());
                } else {
                    PyErr_SetString(PyExc_TypeError, "Invalid types parameter; query string or pyhive.Type or list of pyhive.Type expected");
                    return NULL;
                }
            }
        } else if( pyhive::Type * type_obj = pyhive::Type::check(types) ) {
            type_names_csv.printf("^%s$", type_obj->utype->name());
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid types parameter; query string or pyhive.Type or list of pyhive.Type expected");
            return NULL;
        }
    }

    sUsrObjRes res;
    pyhive_proc->proc->user->objs2(type_names_csv ? type_names_csv.ptr() : 0, res, 0, prop_name, prop_val, 0, false, start, cnt);
    PyObject * lst = PyList_New(res.dim());
    idx i = 0;
    for(sUsrObjRes::IdIter it = res.first(); res.has(it); res.next(it), i++) {
        pyhive::Obj * obj = pyhive::Obj::create();
        const sHiveId * phive_id = res.id(it);
        if( phive_id && obj->init(*phive_id) ) {
            PyList_SET_ITEM(lst, i, (PyObject*)obj);
        } else {
            Py_DECREF(obj);
            Py_DECREF(lst);
            return 0;
        }
    }

    return lst;
}

static PyMethodDef Obj_methods[] = {
    { "is_type_of", (PyCFunction)Obj_is_type_of, METH_VARARGS | METH_KEYWORDS,
      "is_type_of(query)\nCheck if object's type matches a query (same syntax as `pyhive.Type.find`)\n\n"\
      ":arg query: comma-separated list of regexps, each optionally prefixed with `'!'` to negate, or suffixed with `'+'` to retrieve all children of match; "\
      "`''` is equivalent to `'^base_user_type$+'`\n"\
      ":type query: str\n"\
      ":returns: `True` or `False`\n"\
      ":raises TypeError: if parameters are of the wrong type\n"
    },
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
      ":raises pyhive.RuntimeError: if for whatever reason the path could not be retrieved; or if *overwrite* is `False` and the file exists already\n"
    },
    { "get_file_path", (PyCFunction)Obj_get_file_path, METH_VARARGS | METH_KEYWORDS,
      "get_file_path(name)\nRetrieve a path for an existing file in a HIVE object\n\n"\
      ":arg name: file name (without path) to retrieve\n"\
      ":type name: str\n"\
      ":returns: path to existing data file\n"\
      ":rtype: str\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if for whatever reason the path could not be retrieved\n"
    },
    { "del_file_path", (PyCFunction)Obj_del_file_path, METH_VARARGS | METH_KEYWORDS,
      "del_file_path(name)\nDelete an existing file in a HIVE object\n\n"\
      ":arg name: file name (without path)\n"\
      ":type name: str\n"\
      ":returns: `True`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if the file's path could not be retrieved, or if the file did not exist or could not be deleted\n"
    },
    { "prop_get", (PyCFunction)Obj_prop_get, METH_VARARGS | METH_KEYWORDS,
      "prop_get(name, allow_sys_internal = False)\nRetrieve property value(s) with specified field name.\n\n"\
      ":arg name: property field name\n"\
      ":type name: str\n"\
      ":arg allow_sys_internal: allow retrieval of system-internal fields which cannot be exposed to web UI -- dangerous!\n"\
      ":type allow_sys_internal: bool\n"\
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
      ":raises pyhive.RuntimeError: if the property could not be set\n"
    },
    { "obj_list", (PyCFunction)Obj_obj_list, METH_CLASS | METH_VARARGS | METH_KEYWORDS,
      "obj_list(types = `None`, prop_val = `None`, prop_name = `None`, start = 0, cnt = 1000)\n"\
      "Retrieve list of matching objects\n\n"\
      ":arg types: `pyhive.Type` object, or list of `pyhive.Type` objects, or type query string (`pyhive.Type.find` format)\n:type types: `str` or `pyhive.Type` or `list`\n"\
      ":arg prop_val: value or comma-separated list of values of fields to search for by substring match\n:type prop_val: str\n"\
      ":arg prop_name: name or comma-separated list of names of fields to search prop_val in\n:type prop_name: `str` or `list`\n"\
      ":arg start: from result set, return objects starting with this index\n:type start: int\n"\
      ":arg cnt: from result set, return up to this number of objects\n:type cnt: int\n"\
      ":returns: list of `pyhive.Obj` objects\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
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
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.Obj",
    sizeof(pyhive::Obj),
    0,
    (destructor)Obj_dealloc,
    0,
    0,
    0,
    0,
    (reprfunc)Obj_repr,
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
    "HIVE object\n\n"\
    "Initialized from the HIVE ID (as integer or string or `pyhive.Id` object):\n"\
    "    >>> pyhive.Obj(12345)\n"\
    "    <pyhive.Obj 12345 of type svc-align-hexagon at 0x7fa2be26a0f0>\n"\
    "    >>> pyhive.Obj(pyhive.Id('12345'))\n"\
    "    <pyhive.Obj 12345 of type svc-align-hexagon at 0x7fa2be26a0f0>\n"\
    "    >>> pyhive.Obj(123456)\n"\
    "    Traceback (most recent call last):\n"\
    "      File \"<stdin>\", line 1, in <module>\n"\
    "    pyhive.RuntimeError: Failed to load HIVE object with requested ID",
    0,
    0,
    0,
    0,
    0,
    0,
    Obj_methods,
    0,
    Obj_getsetters,
    0,
    0,
    0,
    0,
    0,
    (initproc)Obj_init,
    0,
    Obj_new,
};

PyTypeObject FileObjType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.FileObj",
    sizeof(pyhive::Obj),
    0,
    (destructor)Obj_dealloc,
    0,
    0,
    0,
    0,
    (reprfunc)Obj_repr,
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
    "HIVE file object\n\nChild class of `pyhive.Obj`; `pyhive.Obj(id)` will automatically\n"\
    "construct a `pyhive.FileObj` instance if the *id* refers to a HIVE object of HIVE type 'u-file'\n"\
    "or a descendent.",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    FileObj_getsetters,
    &ObjType,
    0,
    0,
    0,
    0,
    (initproc)Obj_init,
    0,
    Obj_new,
};

static int Obj_init(pyhive::Obj *self, PyObject * args, PyObject * kwds)
{
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

    int ret = -1;
    if( self->init(id_obj->hive_id) ) {
        ret = 0;
    }

    Py_DECREF(id_obj);
    id_obj = 0;

    return ret;
}

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

pyhive::Obj * pyhive::Obj::check(PyObject * o)
{
    if( o && (o->ob_type == &ObjType || o->ob_type == &FileObjType) ) {
        return (pyhive::Obj*)o;
    } else {
        return NULL;
    }
}

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

bool pyhive::Obj::init(const sHiveId & hive_id)
{
    if( uobj ) {
        delete uobj;
        uobj = 0;
    }

    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return false;
    }

    uobj = pyhive_proc->proc->user->objFactory(hive_id);

    if( !uobj ) {
        PyErr_SetString(pyhive::RuntimeError, "Failed to load HIVE object with requested ID");
        return -1;
    }

    sUsrFile* ufile = dynamic_cast<sUsrFile*>(uobj);
    if( ufile && Py_TYPE(this) == &ObjType ) {
        Py_TYPE(this) = &FileObjType;
    } else if( !ufile && Py_TYPE(this) == &FileObjType ) {
        delete uobj;
        uobj = 0;
        PyErr_SetString(pyhive::RuntimeError, "HIVE object with requested ID is not a HIVE file object");
        return false;
    }
    return true;
}
