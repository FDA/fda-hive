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

using namespace slib;

namespace {
    struct TZDic {
        private:
            sDic<pyhive::TZ*> _dic;
        public:
            pyhive::TZ * ensure(idx utc_offset);
            ~TZDic();
    } tz_dic;
};

static PyObject * TZ_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    idx utc_offset = 0;
    if( !PyArg_ParseTuple(args, "|L", &utc_offset) ) {
        return NULL;
    }
    pyhive::TZ * self = tz_dic.ensure(utc_offset);
    Py_XINCREF(self);
    return (PyObject*)self;
}

static PyObject* TZ_repr(pyhive::TZ * self)
{
    sStr buf("<pyhive.TZ %c%02" DEC "%02" DEC " at %p>", self->utc_offset < 0 ? '-' : '+', sAbs(self->utc_offset) / 3600, sAbs(self->utc_offset) % 3600, self);
    return PyUnicode_FromString(buf.ptr());
}

static PyObject* TZ_str(pyhive::TZ * self)
{
    sStr buf("%c%02" DEC "%02" DEC, self->utc_offset < 0 ? '-' : '+', sAbs(self->utc_offset) / 3600, sAbs(self->utc_offset) % 3600);
    return PyUnicode_FromString(buf.ptr());
}

static PyObject * TZ_utcoffset(pyhive::TZ *self, PyObject * args)
{
    return PyDelta_FromDSU(0, self->utc_offset, 0);
}

static PyObject * TZ_dst(pyhive::TZ *self, PyObject * args)
{
    Py_RETURN_NONE;
}

static PyObject * TZ_tzname(pyhive::TZ *self, PyObject * args)
{
    return TZ_str(self);
}

static PyMethodDef TZ_methods[] = {
    { "utcoffset", (PyCFunction)TZ_utcoffset, METH_VARARGS,
      "`datetime.timedelta` object representing UTC offset"
    },
    { "dst", (PyCFunction)TZ_dst, METH_VARARGS,
      "DST adjustment -- not implemented, always returns `None`"
    },
    { "tzname", (PyCFunction)TZ_tzname, METH_VARARGS,
      "Timezone name as '+HHMM' or '-HHMM'"
    },
    { NULL }
};

PyTypeObject TZType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.TZ",
    sizeof(pyhive::TZ),
    0,
    0,
    0,
    0,
    0,
    0,
    (reprfunc)TZ_repr,
    0,
    0,
    0,
    0,
    0,
    (reprfunc)TZ_str,
    0,
    0,
    0,
    Py_TPFLAGS_DEFAULT,
    "A barebones `datetime.tzinfo` implementation, used for formatting time & datetime values from HIVE object fields",
    0,
    0,
    0,
    0,
    0,
    0,
    TZ_methods,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    TZ_new,
};

pyhive::TZ * TZDic::ensure(idx utc_offset)
{
    pyhive::TZ ** ptz = _dic.get(&utc_offset, sizeof(idx));
    if( !ptz ) {
        ptz = _dic.set(&utc_offset, sizeof(idx));
        *ptz = (pyhive::TZ*)PyType_GenericAlloc(&TZType, 0);
        (*ptz)->utc_offset = utc_offset;
    }
    return *ptz;
}

TZDic::~TZDic()
{
    for(idx i=0; i<_dic.dim(); i++) {
        pyhive::TZ * t = *_dic.ptr(i);
        Py_XDECREF(t);
    }
}

bool pyhive::TZ::typeinit(PyObject * mod)
{
    PyDateTime_IMPORT;

    TZType.tp_base = PyDateTimeAPI->TZInfoType;
    if( PyType_Ready(&TZType) < 0 ) {
        return false;
    }
    Py_INCREF(&TZType);
    PyModule_AddObject(mod, "TZ", (PyObject*)&TZType);
    return true;
}

pyhive::TZ * pyhive::TZ::check(PyObject * o)
{
    if( o && o->ob_type == &TZType ) {
        return (pyhive::TZ*)o;
    } else {
        return NULL;
    }
}

PyObject * pyhive::parseDate(const char * s)
{
    sVariant var;
    struct tm tm;
    sSet(&tm);

    var.parseDate(s);
    if( var.asDateTime(&tm) == -sIdxMax ) {
        Py_RETURN_NONE;
    } else {
        return PyDate_FromDate(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    }
}

PyObject * pyhive::parseTime(const char * s)
{
    sVariant var;
    struct tm tm;
    sSet(&tm);
    var.parseTime(s);

    if( var.asDateTime(&tm) == -sIdxMax ) {
        Py_RETURN_NONE;
    } else if( PyDateTimeAPI ) {
#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
        if( tm.tm_zone ) {
            pyhive::TZ * tz = tz_dic.ensure(tm.tm_gmtoff);
            return PyDateTimeAPI->Time_FromTime(tm.tm_hour, tm.tm_min, tm.tm_sec, 0, (PyObject*)tz, PyDateTimeAPI->TimeType);
        }
#endif
        return PyTime_FromTime(tm.tm_hour, tm.tm_min, tm.tm_sec, 0);
    } else {
        PyErr_SetString(PyExc_NameError, "pyhive.TZ hasn't been initialized");
        return NULL;
    }
}

PyObject * pyhive::parseDateTime(const char * s)
{
    sVariant var;
    struct tm tm;
    sSet(&tm);
    var.parseDateTime(s);

    if( var.asDateTime(&tm) == -sIdxMax ) {
        Py_RETURN_NONE;
    } else if( PyDateTimeAPI ) {
#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
        if( tm.tm_zone ) {
            pyhive::TZ * tz = tz_dic.ensure(tm.tm_gmtoff);
            return PyDateTimeAPI->DateTime_FromDateAndTime(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 0, (PyObject*)tz, PyDateTimeAPI->DateTimeType);
        }
#endif
        return PyDateTime_FromDateAndTime(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 0);
    } else {
        PyErr_SetString(PyExc_NameError, "pyhive.TZ hasn't been initialized");
        return NULL;
    }
}

static const char * callIsoFormat(sStr & buf, PyObject * val)
{
    PyObject * iso_fmt_result = PyObject_CallMethod(val, (char*)"isoformat", 0);
    if( !iso_fmt_result ) {
        return 0;
    }
    if( !PyUnicode_Check(iso_fmt_result) ) {
        PyErr_SetString(PyExc_TypeError, "isoformat() did not return a string");
        Py_DECREF(iso_fmt_result);
        return 0;
    }
    idx pos = buf.length();
    const char * tmp_str = PyUnicode_AsUTF8AndSize(iso_fmt_result, NULL);
    if (tmp_str == NULL) {
        PyErr_SetString(PyExc_TypeError, "isoformat() did not return a string convertible to ASCII");
        Py_DECREF(iso_fmt_result);
        return 0;
    }
    buf.addString(tmp_str);
    Py_DECREF(iso_fmt_result);
    return buf.ptr(pos);
}

const char * pyhive::printDate(sStr & buf, PyObject * val)
{
    if( !PyDateTimeAPI ) {
        PyErr_SetString(PyExc_NameError, "pyhive.TZ hasn't been initialized");
        return 0;
    }
    if( !PyDate_Check(val) ) {
        PyErr_SetString(PyExc_TypeError, "datetime.date object expected");
        return 0;
    }
    return callIsoFormat(buf, val);
}

const char * pyhive::printTime(sStr & buf, PyObject * val)
{
    if( !PyDateTimeAPI ) {
        PyErr_SetString(PyExc_NameError, "pyhive.TZ hasn't been initialized");
        return 0;
    }
    if( !PyTime_Check(val) ) {
        PyErr_SetString(PyExc_TypeError, "datetime.time object expected");
        return 0;
    }
    return callIsoFormat(buf, val);
}

const char * pyhive::printDateTime(sStr & buf, PyObject * val)
{
    if( !PyDateTimeAPI ) {
        PyErr_SetString(PyExc_NameError, "pyhive.TZ hasn't been initialized");
        return 0;
    }
    if( !PyDateTime_Check(val) ) {
        PyErr_SetString(PyExc_TypeError, "datetime.datetime object expected");
        return 0;
    }
    return callIsoFormat(buf, val);
}
