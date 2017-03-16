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
#include <qpsvc/qpsvc.hpp>

namespace slib {
    namespace pyhive {
        class sQPySvc: public sQPSvc {
                typedef sQPSvc TParent;
                struct sQPrideBase::Service * _svc;

            public:
                sQPySvc(sQPride & qp, const char * svc_name, sQPrideBase::Service * svc): TParent(qp)
                {
                    _svc = svc;
                    if( !qp.serviceGet(_svc, svc_name) ) {
                        sSet(_svc);
                    }
                }

                virtual ~sQPySvc() {}

                virtual const char * getSvcName() const
                {
                    return _svc->name;
                }
        };
    };
};

using namespace slib;

static PyObject * QPSvc_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::QPSvc * self = (pyhive::QPSvc*)type->tp_alloc(type, 0);
    self->qpysvc = 0;
    self->svc_obj = 0;
    self->req_ids = 0;
    return (PyObject*)self;
}

static void QPSvc_dealloc(pyhive::QPSvc *self)
{
    delete self->qpysvc;
    Py_XDECREF(self->svc_obj);
    Py_XDECREF(self->req_ids);
    self->ob_type->tp_free((PyObject*)self);
}

static int QPSvc_init(pyhive::QPSvc *self, PyObject * args, PyObject * kwds);

static PyObject * QPSvc_repr(pyhive::QPSvc * self)
{
    const char * name = self->qpysvc ? self->qpysvc->getSvcName() : 0;
    sStr buf("<%s %s at %p>", self->ob_type->tp_name, name ? name : "(null)", self);
    return PyString_FromString(buf.ptr());
}

static PyObject * QPSvc_get_svc(pyhive::QPSvc * self, void * closure)
{
    if( self->svc_obj ) {
        Py_INCREF(self->svc_obj);
        return (PyObject*)self->svc_obj;
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject * QPSvc_get_req_ids(pyhive::QPSvc * self, void * closure)
{
    if( self->req_ids ) {
        Py_INCREF(self->req_ids);
        return self->req_ids;
    } else {
        Py_RETURN_NONE;
    }
}

static PyGetSetDef QPSvc_getsetters[] = {
    { (char*)"svc", (getter)QPSvc_get_svc, NULL, (char*)"service which will be launched (`pyhive.Svc`), or `None`", NULL },
    { (char*)"req_ids", (getter)QPSvc_get_req_ids, NULL, (char*)"list of launched request IDs, or `None`", NULL },
    { NULL }
};

static PyObject * QPSvc_set_form(pyhive::QPSvc * self, PyObject * args, PyObject * kwds)
{
    static const char * kwlist[] = { "form", NULL };
    PyObject * form_arg = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "O", (char**)kwlist, &form_arg) ) {
        return NULL;
    }

    sVar form;
    if( !pyhive::py2form(form, form_arg) ) {
        return NULL;
    }
    self->qpysvc->setForm(&form);
    Py_RETURN_TRUE;
}

static PyObject * QPSvc_set_var(pyhive::QPSvc * self, PyObject * args, PyObject * kwds)
{
    static const char * kwlist[] = { "name", "value", NULL };
    const char *name = 0, *value = 0;
    Py_ssize_t value_len = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "ss#", (char**)kwlist, &name, &value, &value_len) ) {
        return NULL;
    }

    sStr value_buf;
    if( value_len ) {
        value_buf.addString(value, value_len);
    }
    value_buf.add0(2); // add0, not add0cut - see setVar and sVar::inp implementations; need terminal 00 to be added to form

    self->qpysvc->setVar(name, value_buf);
    Py_RETURN_TRUE;
}

static PyObject * QPSvc_set_session_id(pyhive::QPSvc * self, PyObject * args, PyObject * kwds)
{
    static const char * kwlist[] = { "form", NULL };
    PyObject * form_arg = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "O", (char**)kwlist, &form_arg) ) {
        return NULL;
    }

    sVar form;
    if( !pyhive::py2form(form, form_arg) ) {
        return NULL;
    }
    self->qpysvc->setSessionID(&form);
    Py_RETURN_TRUE;
}

static PyObject * QPSvc_launch(pyhive::QPSvc * self, PyObject * args, PyObject * kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc || !pyhive_proc->proc || !pyhive_proc->proc->user ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return NULL;
    }

    static const char * kwlist[] = { "grp", NULL };
    idx grpID = pyhive_proc->proc->grpId;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|L", (char**)kwlist, &grpID) ) {
        return NULL;
    }

    if( idx reqID = self->qpysvc->launch(*pyhive_proc->proc->user, grpID) ) {
        Py_XDECREF(self->req_ids);
        self->req_ids = 0;

        sVec<idx> req_ids_vec;
        self->qpysvc->getReqIds(req_ids_vec);
        self->req_ids = PyList_New(req_ids_vec.dim());
        for(idx i = 0; i < req_ids_vec.dim(); i++) {
            PyList_SET_ITEM(self->req_ids, i, pyhive::idx2py(req_ids_vec[i]));
        }

        return pyhive::idx2py(reqID);
    } else {
        PyErr_SetString(PyExc_SystemError, "failed to launch new request");
        return NULL;
    }
}

static PyMethodDef QPSvc_methods[] = {
    {
        "set_form", (PyCFunction)QPSvc_set_form,
        METH_VARARGS | METH_KEYWORDS,
        "set_form(form)\nSet parameters from a form dictionary (as in `form` field of `pyhive.Proc` objects)\n\n"\
        ":arg form: form dictionary (same format as `form` field of a `pyhive.Proc` objects)\n"\
        ":type form: `dict`\n"\
        ":returns: `True`\n"
    },
    {
        "set_var", (PyCFunction)QPSvc_set_var,
        METH_VARARGS | METH_KEYWORDS,
        "set_var(name, value)\nSet form parameter (as in `form` field of `pyhive.Proc` objects)\n\n"\
        ":arg name: form parameter (same format as a key of `form` field of a `pyhive.Proc` objects)\n"\
        ":type name: `str`\n"\
        ":arg value: parameter value\n"\
        ":type value: `str`\n"\
        ":returns: `True`\n"
    },
    {
        "set_session_id", (PyCFunction)QPSvc_set_session_id,
        METH_VARARGS | METH_KEYWORDS,
        "set_session_id(form)\nSet only session-related (i.e. user authentication) parameters from a `form` field of a `pyhive.Proc` object\n\n"\
        ":arg form: form dictionary (same format as `form` field of a `pyhive.Proc` object)\n"\
        ":type form: `dict`\n"\
        ":returns: `True`\n"
    },
    {
        "launch", (PyCFunction)QPSvc_launch,
        METH_VARARGS | METH_KEYWORDS,
        "launch(grp = pyhive.proc.grp_id)\nLaunch a new request\n\n"\
        ":arg grp: ID of request group into which to insert the new request (pyhive.proc.grp_id by default)\n"\
        ":type grp: int\n"\
        ":returns: request ID which was launched\n"\
        ":raises SystemError: if request failed to launch\n"
    },
    { NULL }
};

PyTypeObject QPSvcType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.QPSvc",            // tp_name
    sizeof(pyhive::QPSvc),     // tp_basicsize
    0,                         // tp_itemsize
    (destructor)QPSvc_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)QPSvc_repr,      // tp_repr
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
    "Interface for launching new HIVE requests\n\n"\
    "Initialized from a service name:\n"\
    "   >>> demo = pyhive.QPSvc('pyhive_demo')\n"
    "   <pyhive.QPSvc pyhive_demo at 0x7fa2be26a0f0>\n",   // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    QPSvc_methods,             // tp_methods
    0,                         // tp_members
    QPSvc_getsetters,          // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)QPSvc_init,      // tp_init
    0,                         // tp_alloc
    QPSvc_new,                 // tp_new
};

static int QPSvc_init(pyhive::QPSvc *self, PyObject * args, PyObject * kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc || !pyhive_proc->proc ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return -1;
    }

    static const char * kwlist[] = { "svc_name", NULL };
    const char * svc_name = 0;
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &svc_name) ) {
        return -1;
    }

    Py_XDECREF(self->svc_obj);
    self->svc_obj = pyhive::SvcObj::create();

    if( self->qpysvc ) {
        delete self->qpysvc;
    }
    self->qpysvc = new pyhive::sQPySvc(*pyhive_proc->proc, svc_name, &self->svc_obj->svc);

    if( !self->svc_obj->svc.svcID || !self->svc_obj->svc.name ) {
        sStr err_buf("failed to find service %s", svc_name);
        PyErr_SetString(PyExc_ValueError, err_buf.ptr());
        return -1;
    }

    return 0;
}

//static
bool pyhive::QPSvc::typeinit(PyObject * mod)
{
    if( PyType_Ready(&QPSvcType) < 0 ) {
        return false;
    }
    Py_INCREF(&QPSvcType);
    PyModule_AddObject(mod, "QPSvc", (PyObject*)&QPSvcType);
    return true;
}

//static
pyhive::QPSvc * pyhive::QPSvc::check(PyObject * o)
{
    if( o && o->ob_type == &QPSvcType ) {
        return (pyhive::QPSvc*)o;
    } else {
        return NULL;
    }
}
