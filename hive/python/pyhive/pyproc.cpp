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

#include <structmember.h>

using namespace slib;

#ifdef SLIB64
#define PY_DEC "lld"
#else
#define PY_DEC DEC
#endif

PyObject * pyhive::RuntimeError = PyExc_EnvironmentError;

PyObject * pyhive::udx2py(udx u)
{
    if( u <= LONG_MAX ) {
        return PyLong_FromSize_t(u);
    } else {
        return PyLong_FromUnsignedLongLong(u);
    }
}

PyObject * pyhive::idx2py(idx i)
{
    if( i >= LONG_MIN && i <= LONG_MAX ) {
        return PyLong_FromLong((long)i);
    } else {
        return PyLong_FromLongLong(i);
    }
}

bool pyhive::py2form(sVar & form_out, PyObject * pydict)
{
    if( !PyMapping_Check(pydict) ) {
        PyErr_SetString(PyExc_ValueError, "Expected dict or other mapping type");
        return false;
    }
    PyObject * form_items = PyMapping_Items(pydict);
    idx form_len = PyMapping_Length(form_items);
    for(idx i = 0; i < form_len; i++) {
        const char *key = 0, *value = 0;
        Py_ssize_t value_len = 0;
        PyObject * item = PySequence_GetItem(form_items, i);
        if( !PyArg_ParseTuple(item, "ss#", &key, &value, &value_len) ) {
            Py_XDECREF(item);
            Py_XDECREF(form_items);
            PyErr_SetString(PyExc_ValueError, "Expected str -> str mapping");
            return false;
        }
        if( value_len == sLen(value) ) {
            form_out.inp(key, value);
        } else {
            form_out.inp(key, value, value_len);
        }
        Py_XDECREF(item);
    }
    Py_XDECREF(form_items);

    return true;
}

static PyObject * pyhive_request_killed = 0;


pyhive::sQPyProc * pyhive::sQPyProc::_singleton = 0;

idx pyhive::sQPyProc::OnGrab(idx forceReq)
{
    Py_XDECREF(_pyproc->cached_form);
    Py_XDECREF(_pyproc->cached_form_proxy);
    Py_XDECREF(_pyproc->cached_svc);
    Py_XDECREF(_pyproc->cached_obj);
    _pyproc->cached_form = _pyproc->cached_form_proxy = 0;
    _pyproc->cached_svc = 0;
    _pyproc->cached_obj = 0;

    return Tparent::OnGrab(forceReq);
}

sRC pyhive::sQPyProc::OnSplit(idx req, idx &cnt)
{
    PyObject * on_split = 0;
    if( _run_mod && PyModule_Check(_run_mod) && PyObject_HasAttrString(_run_mod, "on_split") ) {
        on_split = PyObject_GetAttrString(_run_mod, "on_split");
        if( !on_split || !PyCallable_Check(on_split) ) {
            logOut(eQPLogType_Error, "'on_split' in module '%s' is defined but not callable", getModName());
            reqSetStatus(req, eQPReqStatus_ProgError);
            Py_XDECREF(on_split);
            return RC(sRC::eSplitting, sRC::eRequest, sRC::eFunction, sRC::eInvalid);
        }
    }

    if( !on_split ) {
        return Tparent::OnSplit(req, cnt);
    }

    PyObject * args = Py_BuildValue("(L)", req);
    PyObject * result = PyObject_Call(on_split, args, 0);
    Py_XDECREF(args);

    if( result == NULL ) {
        PyObject *etype = 0, *evalue = 0, *etb = 0;
        PyErr_Fetch(&etype, &evalue, &etb);
        if( etype == pyhive_request_killed ) {
            logOut(eQPLogType_Info, "Caught pyhive.RequestKilledError exception; killing request");
            reqSetStatus(req, eQPReqStatus_Killed);
            Py_XDECREF(on_split);
            return RC(sRC::eSplitting, sRC::eRequest, sRC::eOperation, sRC::eKilled);
        } else {
            PyErr_Restore(etype, evalue, etb);
            logPythonTraceback();
            reqSetStatus(req, eQPReqStatus_ProgError);
            Py_XDECREF(on_split);
            return RC(sRC::eSplitting, sRC::eRequest, sRC::eOperation, sRC::eFailed);
        }
    }

    cnt = PyLong_AsLong(result);
    if( cnt < 1 ) {
        if( PyErr_Occurred() ) {
            logPythonTraceback();
        }
        reqSetStatus(req, eQPReqStatus_ProgError);
        Py_XDECREF(result);
        Py_XDECREF(on_split);
        return RC(sRC::eSplitting, sRC::eRequest, sRC::eCount, sRC::eInvalid);
    }

    Py_XDECREF(result);
    Py_XDECREF(on_split);
    return sRC::zero;
}

idx pyhive::sQPyProc::OnExecute(idx req)
{
    if( !_run_mod || !PyModule_Check(_run_mod) ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.proc.run() was not called on a valid module");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    PyObject * on_execute = PyObject_GetAttrString(_run_mod, "on_execute");
    if( !on_execute ) {
        logOut(eQPLogType_Error, "'on_execute' not found in module '%s'", getModName());
        logPythonTraceback();
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    if( !PyCallable_Check(on_execute) ) {
        logOut(eQPLogType_Error, "'on_execute' in module '%s' is not callable", getModName());
        reqSetStatus(req, eQPReqStatus_ProgError);
        Py_XDECREF(on_execute);
        return 0;
    }

    PyObject * args = Py_BuildValue("(L)", req);
    PyObject * result = PyObject_Call(on_execute, args, 0);
    Py_XDECREF(args);

    if( result == NULL ) {
        PyObject *etype = 0, *evalue = 0, *etb = 0;
        PyErr_Fetch(&etype, &evalue, &etb);
        if( etype == pyhive_request_killed ) {
            logOut(eQPLogType_Info, "Caught pyhive.RequestKilledError exception; killing request");
            reqSetStatus(req, eQPReqStatus_Killed);
        } else {
            PyErr_Restore(etype, evalue, etb);
            logPythonTraceback();
            reqSetStatus(req, eQPReqStatus_ProgError);
        }
        Py_XDECREF(on_execute);
        return 0;
    }
    Py_XDECREF(result);
    Py_XDECREF(on_execute);
    return 0;
}

void pyhive::sQPyProc::setRunMod(PyObject * run_mod)
{
    _run_mod = run_mod;
    _run_mod_name.cut0cut();
    if( run_mod && PyObject_HasAttrString(run_mod, "__name__") ) {
        if( PyObject * name = PyObject_GetAttrString(run_mod, "__name__") ) {
            _run_mod_name.addString(PyUnicode_AsUTF8AndSize(name, NULL));
            Py_XDECREF(name);
        } else {
            PyErr_Clear();
        }
    }
}

void pyhive::sQPyProc::logPythonTraceback()
{
    PyObject *etype = 0, *evalue = 0, *etb = 0;
    PyErr_Fetch(&etype, &evalue, &etb);
    if( !etype ) {
        return;
    }

    pyhive::Mex * buf = pyhive::Mex::create();

    PyObject * traceback_mod = PyImport_ImportModule("traceback");
    PyObject * print_exception = traceback_mod ? PyObject_GetAttrString(traceback_mod, "print_exception") : 0;
    PyObject * args = Py_BuildValue("(OOO)", etype, evalue, etb);
    PyObject * kwds = Py_BuildValue("{s:O}", "file", buf);
    PyObject * result = print_exception ? PyObject_Call(print_exception, args, kwds) : 0;
    if( result ) {
        buf->str.add0cut();
        logOut(eQPLogType_Error, "%s", buf->str.ptr(0));
        Py_XDECREF(result);
    }
    Py_XDECREF(kwds);
    Py_XDECREF(args);
    Py_XDECREF(print_exception);
    Py_XDECREF(traceback_mod);
    Py_XDECREF(buf);
}



static pyhive::SvcObj * Svc_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::SvcObj * self = (pyhive::SvcObj*)type->tp_alloc(type, 0);
    if( self ) {
        sSet(&(self->svc));
        self->cdate = self->name = self->title = self->cmd_line = self->hosts = self->emails = self->categories = 0;
    }
    return self;
}

static void Svc_dealloc(pyhive::SvcObj *self)
{
    Py_XDECREF(self->cdate);
    Py_XDECREF(self->name);
    Py_XDECREF(self->title);
    Py_XDECREF(self->cmd_line);
    Py_XDECREF(self->hosts);
    Py_XDECREF(self->emails);
    Py_XDECREF(self->categories);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* Svc_repr(pyhive::SvcObj * self)
{
    sStr buf("<%s %" DEC " %s at %p>", Py_TYPE(self)->tp_name, self->svc.svcID, self->svc.name, self);
    return PyUnicode_FromString(buf.ptr());
}

#define SVC_OFFSETOF(member) (offsetof(pyhive::SvcObj, svc) + offsetof(sQPrideBase::Service, member))

static PyMemberDef Svc_members[] = {
    { (char*)"svc_id", T_LONGLONG, SVC_OFFSETOF(svcID), READONLY, (char*)"service ID" },
    { (char*)"perm_id", T_LONGLONG, SVC_OFFSETOF(permID), READONLY, (char*)"perm ID" },
    { (char*)"svc_type", T_LONGLONG, SVC_OFFSETOF(svcType), READONLY, (char*)"service type" },
    { (char*)"knockout_sec", T_LONGLONG, SVC_OFFSETOF(knockoutSec), READONLY, (char*)"threshold for killing hung jobs (seconds)" },
    { (char*)"max_jobs", T_LONGLONG, SVC_OFFSETOF(maxJobs), READONLY, (char*)"max jobs" },
    { (char*)"nice", T_LONGLONG, SVC_OFFSETOF(nice), READONLY, (char*)"niceness" },
    { (char*)"sleep_time", T_LONGLONG, SVC_OFFSETOF(sleepTime), READONLY, (char*)"sleep time (seconds)" },
    { (char*)"max_loops", T_LONGLONG, SVC_OFFSETOF(maxLoops), READONLY, (char*)"max loops" },
    { (char*)"parallel_jobs", T_LONGLONG, SVC_OFFSETOF(parallelJobs), READONLY, (char*)"parallel jobs" },
    { (char*)"delay_launch_sec", T_LONGLONG, SVC_OFFSETOF(delayLaunchSec), READONLY, (char*)"launch delay (seconds)" },
    { (char*)"polite_exit_timeout_sec", T_LONGLONG, SVC_OFFSETOF(politeExitTimeoutSec), READONLY, (char*)"polite threshold for detecting hung jobs (seconds)" },
    { (char*)"max_trials", T_LONGLONG, SVC_OFFSETOF(maxTrials), READONLY, (char*)"max number of times to restart crashed process" },
    { (char*)"restart_sec", T_LONGLONG, SVC_OFFSETOF(restartSec), READONLY, (char*)"threshold for restarting crashed processes (seconds)" },
    { (char*)"priority", T_LONGLONG, SVC_OFFSETOF(priority), READONLY, (char*)"priority" },
    { (char*)"clean_up_days", T_LONGLONG, SVC_OFFSETOF(cleanUpDays), READONLY, (char*)"old request cleanup threshold (days)" },
    { (char*)"run_in_mt", T_LONGLONG, SVC_OFFSETOF(runInMT), READONLY, (char*)"run in multithreaded mode with this number of threads" },
    { (char*)"no_grab_disconnect", T_LONGLONG, SVC_OFFSETOF(noGrabDisconnect), READONLY, (char*)"number of times to try grabbing a request before disconnecting from database" },
    { (char*)"lazy_report_sec", T_LONGLONG, SVC_OFFSETOF(lazyReportSec), READONLY, (char*)"minimum update interval for reporting progress (seconds)" },
    { (char*)"active_job_reserve", T_LONGLONG, SVC_OFFSETOF(activeJobReserve), READONLY, (char*)"minimum number of jobs to keep running overall on the cluster" },
    { (char*)"maxmem_soft", T_LONGLONG, SVC_OFFSETOF(maxmemSoft), READONLY, (char*)"soft memory limit" },
    { (char*)"maxmem_hard", T_LONGLONG, SVC_OFFSETOF(maxmemHard), READONLY, (char*)"hard memory limit" },
    { (char*)"capacity", T_DOUBLE, SVC_OFFSETOF(capacity), READONLY, (char*)"capacity" },
    { NULL }
};

static PyObject * Svc_get_is_up(pyhive::SvcObj * self, void * closure)
{
    if( self->svc.isUp ) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject * Svc_get_cdate(pyhive::SvcObj * self, void * closure)
{
    if( !self->cdate ) {
        sStr buf;
        self->cdate = pyhive::parseDateTime(buf.printf("%" DEC, self->svc.cdate));
    }
    Py_XINCREF(self->cdate);
    return self->cdate;
}

#define SVC_GET_STRMEMBER(pyname, cppname) \
    static PyObject * Svc_get_ ## pyname (pyhive::SvcObj * self, void * closure) \
    { \
        if( !self->pyname ) { \
            self->pyname = PyUnicode_FromString(self->svc.cppname); \
        } \
        Py_XINCREF(self->pyname); \
        return self->pyname; \
    }

SVC_GET_STRMEMBER(name, name)
SVC_GET_STRMEMBER(title, title)
SVC_GET_STRMEMBER(cmd_line, cmdLine)
SVC_GET_STRMEMBER(hosts, hosts)
SVC_GET_STRMEMBER(emails, emails)
SVC_GET_STRMEMBER(categories, categories)

static PyGetSetDef Svc_getsetters[] = {
    { (char*)"is_up", (getter)Svc_get_is_up, NULL, (char*)"whether service is up", NULL },
    { (char*)"cdate", (getter)Svc_get_cdate, NULL, (char*)"creation timestamp", NULL },
    { (char*)"name", (getter)Svc_get_name, NULL, (char*)"name", NULL },
    { (char*)"title", (getter)Svc_get_title, NULL, (char*)"title", NULL },
    { (char*)"cmd_line", (getter)Svc_get_cmd_line, NULL, (char*)"command line", NULL },
    { (char*)"hosts", (getter)Svc_get_hosts, NULL, (char*)"hosts", NULL },
    { (char*)"emails", (getter)Svc_get_emails, NULL, (char*)"emails", NULL },
    { (char*)"categories", (getter)Svc_get_categories, NULL, (char*)"categories", NULL },
    { NULL }
};

static PyTypeObject SvcType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.Svc",
    sizeof(pyhive::SvcObj),
    0,
    (destructor)Svc_dealloc,
    0,
    0,
    0,
    0,
    (reprfunc)Svc_repr,
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
    "HIVE service",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    Svc_members,
    Svc_getsetters,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (newfunc)Svc_new,
};

pyhive::SvcObj * pyhive::SvcObj::check(PyObject * o)
{
    if( o && o->ob_type == &SvcType ) {
        return (pyhive::SvcObj*)o;
    } else {
        return NULL;
    }
}

pyhive::SvcObj * pyhive::SvcObj::create()
{
    pyhive::SvcObj * self = (pyhive::SvcObj*)Svc_new(&SvcType, 0, 0);
    Py_XINCREF(self);
    return self;
}


static PyObject* AlreadyLockedError_str(PyObject * self)
{
    PyObject * args = PyObject_GetAttrString(self, "args");
    PyObject * ret = PyTuple_GetItem(args, 0);
    Py_XINCREF(ret);
    Py_XDECREF(args);
    return ret;
}

static PyTypeObject AlreadyLockedError = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.AlreadyLockedError",
    sizeof(AlreadyLockedError),
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    AlreadyLockedError_str,
    0,
    0,
    0,
    Py_TPFLAGS_DEFAULT,
    "Special exception indicating that another request is already holding the lock\n\n"\
    "The request ID holding the lock is stored in `args[1]` of the exception object",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (PyTypeObject*)PyExc_Exception,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};


static PyObject * Proc_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    if( pyhive::sQPyProc::getSingleton() ) {
        PyErr_SetString(PyExc_IndexError, "pyhive.proc has already been initialized");
        return NULL;
    }
    pyhive::Proc * self = (pyhive::Proc*)type->tp_alloc(type, 0);
    self->proc = 0;
    self->cached_form = self->cached_form_proxy = 0;
    self->cached_svc = 0;
    self->cached_obj = 0;
    return (PyObject*)self;
}

static void Proc_dealloc(pyhive::Proc *self)
{
    Py_XDECREF(self->cached_form_proxy);
    Py_XDECREF(self->cached_form);
    Py_XDECREF(self->cached_svc);
    delete self->proc;
    self->proc = 0;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int Proc_init(pyhive::Proc *self, PyObject * args, PyObject * kwds)
{
    if( pyhive::sQPyProc::getSingleton() || self->proc ) {
        PyErr_SetString(PyExc_IndexError, "pyhive.proc has already been initialized");
        return -1;
    }
    const char * srv = 0;
    static const char * kwlist[] = { "srv", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &srv) ) {
        return -1;
    }
    if( !srv[0] ) {
        PyErr_SetString(PyExc_ValueError, "srv must be a non-empty string");
        return -1;
    }

    self->proc = new pyhive::sQPyProc(self, "config=qapp.cfg" __, srv);
    pyhive::sQPyProc::setSingleton(self->proc);
    return 0;
}

#define ASSERT_SELF_PROC(err) \
do { \
    if( !self->proc ) { \
        PyErr_SetString(PyExc_TypeError, "pyhive.proc has not been initialized"); \
        return (err); \
    } \
} while (0)

static PyObject * Proc_run(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    PyObject * run_mod = 0;
    PyObject * argv = 0;
    ASSERT_SELF_PROC(NULL);
    static const char * kwlist[] = { "module", "argv", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "OO", (char**)kwlist, &run_mod, &argv) ) {
        return 0;
    }
    if( !PyModule_Check(run_mod) ) {
        PyErr_SetString(PyExc_TypeError, "module must be a Python module with an 'on_execute' function");
        return 0;
    }
    if( !PySequence_Check(argv) ) {
        PyErr_SetString(PyExc_TypeError, "argv must be a sequence of strings");
        return 0;
    }

    int argc = 1 + PySequence_Length(argv);
    sStr argv00;
    PyObject * filenamePyObj = PyModule_GetFilenameObject(run_mod);
    if( filenamePyObj ) {
        const char * filename = PyUnicode_AsUTF8(filenamePyObj);
        argv00.addString(filename);
        argv00.add0();
    } else {
        PyErr_Clear();
        argv00.addString(self->proc->getSrvName());
        argv00.add0();
    }
    for(idx i=0; i<argc-1; i++) {
        if( PyObject * arg = PySequence_GetItem(argv, i) ) {
            if( const char * s = PyUnicode_AsUTF8AndSize(arg, NULL) ) {
                argv00.add(s);
            } else {
                PyErr_SetString(PyExc_ValueError, "argv must be a sequence of strings");
                return 0;
            }
        } else {
            PyErr_SetString(PyExc_ValueError, "argv must be a sequence of strings");
            return 0;
        }
    }
    argv00.add0(2);

    sVec<const char *>argv_vec;
    argv_vec.resize(argc);
    const char * s = argv00;
    for(idx i=0; i<argc; i++) {
        argv_vec[i] = s;
        s = sString::next00(s);
    }

    Py_XINCREF(run_mod);
    self->proc->setRunMod(run_mod);
    idx result = self->proc->run(argc, argv_vec);
    self->proc->setRunMod(0);
    Py_XDECREF(run_mod);

    return pyhive::idx2py(result);
}

static PyObject * Proc_config_get(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);

    const char * name = 0;
    static const char * kwlist[] = { "par", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s:pyhive.Proc.config_get", (char**)kwlist, &name) ) {
        return 0;
    }

    sStr buf;
    if( self->proc->configGet(&buf, 0, name, 0, 0) ) {
        return PyUnicode_FromString(buf.ptr());
    } else {
        PyErr_SetString(PyExc_KeyError, "invalid config par");
        return NULL;
    }
}

static PyObject * Proc_config_get_all(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);

    static const char * kwlist[] = { NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, ":pyhive.Proc.config_get_all", (char**)kwlist) ) {
        return 0;
    }

    sStr buf;
    self->proc->configGetAll(&buf, 0);

    PyObject * ret_dict = PyDict_New();
    for(const char * s = buf.ptr(); s; ) {
        const char * value = s;
        const char * key = sString::next00(s);
        PyDict_SetItemString(ret_dict, key, PyUnicode_FromString(value));
        s = sString::next00(key);
    }
    return ret_dict;
}

static PyObject * Proc_req_progress(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);

    idx progress = sNotIdx, progress_max = 100, items = -1;
    static const char * kwlist[] = { "progress", "max", "items", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "L|LL:pyhive.Proc.req_progress", (char**)kwlist, &progress, &progress_max, &items) ) {
        return 0;
    }
    if( items < -1 ) {
        PyErr_SetString(PyExc_ValueError, "non-negative number of items expected");
        return 0;
    }
    if( progress_max <= 0 ) {
        PyErr_SetString(PyExc_ValueError, "non-negative progress_max expected");
        return 0;
    }
    if( progress < 0 || progress > progress_max ) {
        PyErr_SetString(PyExc_ValueError, "progress is expected to be in 0..max range");
        return 0;
    }

    if( self->proc->reqProgress(items, progress, progress_max) ) {
        Py_RETURN_TRUE;
    } else {
        PyErr_SetString(pyhive_request_killed, "Current HIVE request has been killed");
        return 0;
    }
}

static PyObject * Proc_req_get_status(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx reqID = self->proc->reqId;
    static const char * kwlist[] = { "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|L", (char**)kwlist, &reqID) ) {
        return 0;
    }
    idx status = self->proc->reqGetStatus(reqID);
    if( status == 0 ) {
        PyErr_SetString(pyhive::RuntimeError, "failed to get status for given request ID");
        return 0;
    } else if( status <= 0 || status >= sQPrideBase::eQPReqStatus_Max ) {
        PyErr_SetString(pyhive::RuntimeError, "system returned invalid status value");
        return 0;
    }
    return pyhive::idx2py(status);
}

static PyObject * Proc_req_set_status(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    int status;
    static const char * kwlist[] = { "status", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "i", (char**)kwlist, &status) ) {
        return 0;
    }
    if( status <= 0 || status >= sQPrideBase::eQPReqStatus_Max ) {
        PyErr_SetString(PyExc_ValueError, "status must be one of pyhive.req_status constants");
        return 0;
    }
    idx ret = self->proc->reqSetStatus(self->proc->reqId, status);
    return pyhive::idx2py(ret);
}

static PyObject * Proc_req_get_action(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx reqID = self->proc->reqId;
    static const char * kwlist[] = { "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|L", (char**)kwlist, &reqID) ) {
        return 0;
    }
    idx action = self->proc->reqGetAction(reqID);
    if( action == 0 ) {
        PyErr_SetString(pyhive::RuntimeError, "failed to get action code for given request ID");
        return 0;
    } else if( action <= 0 || action >= sQPrideBase::eQPReqAction_Max ) {
        PyErr_SetString(pyhive::RuntimeError, "system returned invalid action value");
        return 0;
    }
    return pyhive::idx2py(action);
}

static PyObject * Proc_req_set_action(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    int action;
    idx reqID = self->proc->reqId;
    static const char * kwlist[] = { "action", "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "i|L", (char**)kwlist, &action, &reqID) ) {
        return 0;
    }
    if( action <= 0 || action >= sQPrideBase::eQPReqAction_Max ) {
        PyErr_SetString(PyExc_ValueError, "action must be one of pyhive.req_action constants");
        return 0;
    }
    idx ret = self->proc->reqSetAction(self->proc->reqId, action);
    return pyhive::idx2py(ret);
}

static PyObject * Proc_log(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    int log_type = 0;
    const char * msg = 0;
    static const char * kwlist[] = { "log_type", "message", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "is", (char**)kwlist, &log_type, &msg) ) {
        return 0;
    }
    if( log_type < sQPrideBase::eQPLogType_Min || log_type > sQPrideBase::eQPLogType_Max ) {
        PyErr_SetString(PyExc_ValueError, "log_type must be one of pyhive.log_type constants");
        return 0;
    }
    self->proc->logOut((sQPrideBase::eQPLogType)log_type, "%s", msg);
    Py_RETURN_NONE;
}

static PyObject * Proc_req2grp(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx reqID = self->proc->reqId;
    static const char * kwlist[] = { "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|L", (char**)kwlist, &reqID) ) {
        return 0;
    }
    if( idx grpID = self->proc->req2Grp(reqID) ) {
        return pyhive::idx2py(grpID);
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject * Proc_grp2req(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx grpID = self->proc->grpId;
    static const char * kwlist[] = { "grp", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|L", (char**)kwlist, &grpID) ) {
        return 0;
    }
    sVec<idx> reqIDs;
    self->proc->grp2Req(grpID, &reqIDs);
    PyObject * out = PyList_New(reqIDs.dim());
    for (idx i = 0; i < reqIDs.dim(); i++) {
        PyList_SET_ITEM(out, i, pyhive::idx2py(reqIDs[i]));
    }
    return out;
}

static PyObject * Proc_is_last_in_group(pyhive::Proc * self, PyObject * args_not_used)
{
    ASSERT_SELF_PROC(NULL);
    if( self->proc->isLastInGroup() ) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject * Proc_req_set_info(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    int log_type = 0;
    const char * msg = 0;
    static const char * kwlist[] = { "log_type", "message", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "is", (char**)kwlist, &log_type, &msg) ) {
        return 0;
    }
    sQPrideBase::eQPInfoLevel level;
    switch(log_type) {
        case sQPrideBase::eQPLogType_Trace:
            level = sQPrideBase::eQPInfoLevel_Trace;
            break;
        case sQPrideBase::eQPLogType_Debug:
            level = sQPrideBase::eQPInfoLevel_Debug;
            break;
        case sQPrideBase::eQPLogType_Info:
            level = sQPrideBase::eQPInfoLevel_Info;
            break;
        case sQPrideBase::eQPLogType_Warning:
            level = sQPrideBase::eQPInfoLevel_Warning;
            break;
        case sQPrideBase::eQPLogType_Error:
            level = sQPrideBase::eQPInfoLevel_Error;
            break;
        case sQPrideBase::eQPLogType_Fatal:
            level = sQPrideBase::eQPInfoLevel_Fatal;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "info_level must be one of pyhive.log_type constants");
            return 0;
    }
    self->proc->reqSetInfo(self->proc->reqId, level, "%s", msg);
    Py_RETURN_NONE;
}

static PyObject * Proc_req_resubmit(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx delay = 0;
    idx reqID = self->proc->reqId;
    static const char * kwlist[] = { "delay", "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "L|L", (char**)kwlist, &delay, &reqID) ) {
        return 0;
    }

    idx ret = self->proc->reqReSubmit(delay, reqID);
    return pyhive::idx2py(ret);
}

static PyObject * Proc_add_file_path(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    const char * name = 0;
    static const char * kwlist[] = { "name", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &name) ) {
        return 0;
    }

    sStr buf;
    if( const char * path = self->proc->reqAddFile(buf, "%s", name) ) {
        return PyUnicode_FromString(path);
    } else {
        PyErr_SetString(pyhive::RuntimeError, "Failed to add file to HIVE object, request, or group");
        return NULL;
    }
}

static PyObject * Proc_get_file_path(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    const char * name = 0;
    static const char * kwlist[] = { "name", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &name) ) {
        return 0;
    }

    sStr buf;
    if( const char * path = self->proc->reqGetFile(buf, "%s", name) ) {
        return PyUnicode_FromString(path);
    } else {
        PyErr_SetString(pyhive::RuntimeError, "Failed to retrieve file from HIVE object, request, or group");
        return NULL;
    }
}

static PyObject * Proc_req_data_names(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx reqID = self->proc->reqId;
    static const char * kwlist[] = { "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|L", (char**)kwlist, &reqID) ) {
        return 0;
    }

    sStr dnames00;
    idx ndata = self->proc->dataGetAll(reqID, 0, &dnames00);
    PyObject * out = PyList_New(ndata);
    const char * dname = dnames00.ptr();
    for (idx i = 0; i < ndata; i++) {
        if( dname && *dname ) {
            PyObject * s = PyUnicode_FromString(dname);
            PyList_SET_ITEM(out, i, s);
            dname = sString::next00(dname);
        } else {
            Py_INCREF(Py_None);
            PyList_SET_ITEM(out, i, Py_None);
        }
    }
    return out;
}

static PyObject * Proc_req_get_data(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx reqID = self->proc->reqId;
    const char * name = 0;
    static const char * kwlist[] = { "name", "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s|L", (char**)kwlist, &name, &reqID) ) {
        return NULL;
    }
    if( !name[0] ) {
        PyErr_SetString(PyExc_TypeError, "Non-empty data name expected");
        return NULL;
    }

    sMex mex;
    if( self->proc->reqGetData(reqID, name, &mex) ) {
        pyhive::Mex * out = pyhive::Mex::create();
        out->str.destroy();
        out->str.borrow(&mex);
        return (PyObject*)out;
    } else {
        PyErr_SetString(pyhive::RuntimeError, "Failed to retrieve data blob from HIVE request");
        return NULL;
    }
}

static PyObject * Proc_req_set_data(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx reqID = self->proc->reqId;
    const char * name = 0, * data = 0;
    Py_ssize_t data_len = 0;
    static const char * kwlist[] = { "name", "data", "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s|s#L", (char**)kwlist, &name, &data, &data_len, &reqID) ) {
        return 0;
    }

    if( !data_len ) {
        data = 0;
    }

    if( self->proc->reqSetData(reqID, name, data_len, data) ) {
        Py_RETURN_TRUE;
    } else {
        PyErr_SetString(pyhive::RuntimeError, "Failed to set data blob for HIVE request");
        return NULL;
    }
}

static PyObject * Proc_req_get_data_path(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    idx reqID = self->proc->reqId;
    const char * name = 0;
    static const char * kwlist[] = { "name", "req", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s|L", (char**)kwlist, &name, &reqID) ) {
        return 0;
    }

    sStr path_buf;
    if( self->proc->reqDataPath(reqID, name, &path_buf) ) {
        PyObject * s = PyUnicode_FromString(path_buf.ptr());
        return s;
    } else {
        PyErr_SetString(pyhive::RuntimeError, "Failed to retrieve storage path for data blob for HIVE request");
        return NULL;
    }
}

static PyObject * Proc_req_lock(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    const char * key = 0;
    idx reqID = self->proc->reqId;
    idx max_lifetime = 48*60*60;
    int force = false;
    static const char * kwlist[] = { "key", "req", "max_lifetime", "force", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s|LLi", (char**)kwlist, &key, &reqID, &max_lifetime, &force) ) {
        return 0;
    }

    idx reqLockedBy = 0;
    if( self->proc->reqLock(reqID, key, &reqLockedBy, max_lifetime, force) ) {
        Py_RETURN_TRUE;
    } else {
        PyObject * eargs = Py_BuildValue("(NN)", PyUnicode_FromFormat("Already locked by request %" PY_DEC, reqLockedBy), pyhive::idx2py(reqLockedBy));
        PyErr_SetObject((PyObject*)&AlreadyLockedError, eargs);
        Py_XDECREF(eargs);
        return 0;
    }
}

static PyObject * Proc_req_unlock(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    const char * key = 0;
    idx reqID = self->proc->reqId;
    int force = false;
    static const char * kwlist[] = { "key", "req", "force", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s|Li", (char**)kwlist, &key, &reqID, &force) ) {
        return 0;
    }
    if( self->proc->reqUnlock(reqID, key, force) ) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject * Proc_req_checklock(pyhive::Proc * self, PyObject * args, PyObject * kwds)
{
    ASSERT_SELF_PROC(NULL);
    const char * key = 0;
    static const char * kwlist[] = { "key", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "s", (char**)kwlist, &key) ) {
        return 0;
    }
    if( idx reqID = self->proc->reqCheckLock(key) ) {
        return pyhive::idx2py(reqID);
    } else {
        Py_RETURN_NONE;
    }
}

static PyMethodDef Proc_methods[] = {
    { "run", (PyCFunction)Proc_run, METH_VARARGS | METH_KEYWORDS,
      "run(module, argv)\n\nRun the process\n\n"\
      ":arg module: Python module with an `on_execute` function, which will be executed, possibly multiple times.\n"
      ":arg argv: appropriate slice of `sys.argv`"
    },
    { "config_get", (PyCFunction)Proc_config_get, METH_VARARGS | METH_KEYWORDS,
      "config_get(par)\n\n"\
      "Retrieve system configuration entry.\n\n"\
      ":arg par: name of configuration entry.\n:type par: str\n"\
      ":returns: configuration entry value\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises KeyError: if there is no configuration entry with specified name.\n"
    },
    { "config_get_all", (PyCFunction)Proc_config_get_all, METH_VARARGS | METH_KEYWORDS,
      "config_get_all()\n\n"\
      "Retrieve all system configuration entries as a dictionary.\n\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
    },
    { "req_progress", (PyCFunction)Proc_req_progress, METH_VARARGS | METH_KEYWORDS,
      "req_progress(progress, max = 100[, items])\n\n"\
      "Update progress level for current request and record it as still alive.\n"\
      "You **MUST** call :py:meth:`pyhive.proc.req_progress` regularly; if progress is not updated for\n"\
      "`pyhive.proc.svc.knockout_sec` or `pyhive.proc.svc.restart_sec` seconds (usually on the order of\n"\
      "10-15 minutes), the request will be assumed to have hung or crashed.\n\n"
      ":arg progress: completion level between 0 and *max*.\n"\
      ":type progress: int\n"\
      ":arg max: maximum possible value of *progress* (100 by default).\n"\
      ":type max: int\n"\
      ":arg items: monotonically increasing, non-negative count of processed rows, records etc.\n"\
      ":type items: int\n"\
      ":returns: `True`\n"\
      ":raises pyhive.RequestKilledError: if current request has been killed. If you want to handle this exception (e.g. to delete temp data), you must call `pyhive.proc.req_set_status(pyhive.req_status.KILLED)`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises ValueError: if parameters are out of range\n"
    },
    { "req_get_status", (PyCFunction)Proc_req_get_status, METH_VARARGS | METH_KEYWORDS,
      "req_get_status(req = pyhive.proc.req_id)\n\nGet status code for a request\n\n"\
      ":arg req: request ID (`pyhive.proc.req_id` by default)\n"\
      ":returns: *status* (one of `pyhive.req_status` constants)\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if status code could not be retrieved, e.g. because *req* is invalid\n"
    },
    { "req_set_status", (PyCFunction)Proc_req_set_status, METH_VARARGS | METH_KEYWORDS,
      "req_set_status(status)\n\nSet status code for current request\n\n"\
      ":arg status: one of `pyhive.req_status` constants\n"\
      ":returns: *status* back\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises ValueError: if parameters are out of range\n"
    },
    { "req_set_info", (PyCFunction)Proc_req_set_info, METH_VARARGS | METH_KEYWORDS,
      "req_set_info(log_type, message)\n\nAdd a user-visible info string for current request\n\n"\
      ":arg log_type: one of `pyhive.log_type` constants\n"\
      ":arg message: message string\n"\
      ":returns: `None`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises ValueError: if parameters are out of range\n"
    },
    { "req_resubmit", (PyCFunction)Proc_req_resubmit, METH_VARARGS | METH_KEYWORDS,
      "req_resubmit(delay, req = pyhive.proc.req_id)\n\nSchedule a request to be picked up and processed again\n\n"\
      ":arg delay: the request will be scheduled for processing again after this number of seconds\n"\
      ":type delay: int\n"\
      ":arg req: request ID to resubmit\n"\
      ":type req: int\n"\
      ":returns: request ID back\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
    },
    { "req_get_action", (PyCFunction)Proc_req_get_action, METH_VARARGS | METH_KEYWORDS,
      "req_get_action(req = pyhive.proc.req_id)\n\nGet job handler action code for specified request\n\n"\
      ":arg req: request ID (`pyhive.proc.req_id` by default)\n"\
      ":returns: *action* (one of `pyhive.req_action` constants)\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if action code could not be retrieved, e.g. because *req* is invalid\n"
    },
    { "req_set_action", (PyCFunction)Proc_req_set_action, METH_VARARGS | METH_KEYWORDS,
      "req_set_action(action, req = pyhive.proc.req_id)\n\nSet job handler action code for specified request\n\n"\
      ":arg action: one of `pyhive.req_action` constants\n"\
      ":arg req: request ID (`pyhive.proc.req_id` by default)\n"\
      ":returns: *action* back\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises ValueError: if parameters are out of range\n"
    },
    { "log", (PyCFunction)Proc_log, METH_VARARGS | METH_KEYWORDS,
      "log(log_type, message)\n\nAdd a logging message for developers and system administrators\n\n"\
      ":arg log_type: one of `pyhive.log_type` constants\n"\
      ":arg message: message string\n"\
      ":returns: `None`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises ValueError: if parameters are out of range\n"
    },
    { "req2grp", (PyCFunction)Proc_req2grp, METH_VARARGS | METH_KEYWORDS,
      "req2grp(req = pyhive.proc.req_id)\n\nFind group ID for specified request ID\n\n"\
      ":arg req: request ID\n:type req: int\n"\
      ":returns: group ID (positive int), or `None` on failure\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
    },
    { "grp2req", (PyCFunction)Proc_grp2req, METH_VARARGS | METH_KEYWORDS,
      "grp2req(grp = pyhive.proc.grp_id)\n\nFind list of request IDs for specified request group ID\n\n"\
      ":arg req: request group ID\n:type req: int\n"\
      ":returns: list of request IDs (positive integers)\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
    },
    { "is_last_in_group", (PyCFunction)Proc_is_last_in_group, METH_NOARGS,
      "Check if current request is the last one in its request group which is still running\n\n"\
      ":returns: `True` or `False`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized\n"
    },
    { "add_file_path", (PyCFunction)Proc_add_file_path, METH_VARARGS | METH_KEYWORDS,
      "add_file_path(name)\n\nRetrieve a path for a new file in current HIVE object or HIVE request or group.\n"\
      "Any existing file found in the relevant object/request/group with the specified\n"\
      "name will be deleted.\n\n"\
      " * By default, if request is associated with a HIVE object (`pyhive.proc.obj`), will\n"\
      "   try to create an object file\n"\
      " * If request is not associated with a HIVE object, will try to create a file-based\n"\
      "   data blob for `pyhive.proc.req_id`\n"\
      " * If name starts with 'reqgrp-', will try to create a file-based data blob for\n"\
      "   pyhive.proc.grp_id if it exists, or `pyhive.proc.req_id` otherwise.\n"\
      " * If name starts with 'req-' or 'reqself-', will try to create a file-based data blob for\n"\
      "   `pyhive.proc.req_id`\n\n"\
      ":arg name: file name\n:type name: str\n\n"\
      ":returns: path to file\n:rtype: str\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if the file could not be added"
    },
    { "get_file_path", (PyCFunction)Proc_get_file_path, METH_VARARGS | METH_KEYWORDS,
      "get_file_path(name)\nRetrieve a path for an existing file in current HIVE object or HIVE request or group\n\n"\
      " * By default, if request is associated with a HIVE object (`pyhive.proc.obj`), will\n"\
      "   retrieve object file\n"\
      " * If request is not associated with a HIVE object, will try to retrieve file-based\n"\
      "   data blob for `pyhive.proc.req_id`\n"\
      " * If name starts with 'reqgrp-', will try to retrieve file-based data blob for\n"\
      "   pyhive.proc.grp_id if it exists, or `pyhive.proc.req_id` otherwise.\n"\
      " * If name starts with 'req-' or 'reqself-', will try to retrieve file-based data blob for\n"\
      "   `pyhive.proc.req_id`\n\n"\
      ":arg name: file name\n:type name: str\n\n"\
      ":returns: path to file\n:rtype: str\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if the file could not be retrieved"
    },
    { "req_data_names", (PyCFunction)Proc_req_data_names, METH_VARARGS | METH_KEYWORDS,
      "req_data_names(req = pyhive.proc.req_id)\nList data names for specified request\n\n"\
      ":arg req: request ID\n:type req: int\n"\
      ":returns: list of data names (strings)\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type"
    },
    { "req_get_data", (PyCFunction)Proc_req_get_data, METH_VARARGS | METH_KEYWORDS,
      "req_get_data(name, req = pyhive.proc.req_id)\nRetrieve named data blob for specified request\n\n"\
      ":arg name: data blob name\n:type name: str\n"\
      ":arg req: request ID\n:type req: int\n"\
      ":returns: data blob\n"\
      ":rtype: `pyhive.Mex`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if the named data blob could not be retrieved"
    },
    { "req_get_data_path", (PyCFunction)Proc_req_get_data_path, METH_VARARGS | METH_KEYWORDS,
      "req_get_data_path(name, req = pyhive.proc.req_id)\nRetrieve disk storage path for named data blob for specified request\n\n"\
      ":arg name: data blob name\n:type name: str\n"\
      ":arg req: request ID\n:type req: int\n"\
      ":returns: disk path\n"\
      ":rtype: `str`\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if the named data blob could not be retrieved"
    },
    { "req_set_data", (PyCFunction)Proc_req_set_data, METH_VARARGS | METH_KEYWORDS,
      "req_set_data(name, data = '', req = pyhive.proc.req_id)\nSave named data blob for specified request\n\n"\
      ":arg name: data blob name\n:type name: str\n"\
      ":arg data: blob contents\n:type data: `str` or buffer or `pyhive.Mex`\n"\
      ":arg req: request ID\n:type req: int\n"\
      ":returns: `True`\n"
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.RuntimeError: if the named data blob could not be saved"
    },
    { "req_lock", (PyCFunction)Proc_req_lock, METH_VARARGS | METH_KEYWORDS,
      "req_lock(key, req = pyhive.proc.req_id, max_lifetime = 48*60*60, force = False)\n"\
      "Lock a named entity (typically filesystem path) for the specified request\n\n"\
      ":arg key: name of entity (typically filesystem path) to lock\n:type key: str\n"\
      ":arg req: request ID on whose behalf to lock (`pyhive.proc.req_id` by default)\n:type req: int\n"\
      ":arg max_lifetime: max length of time to hold the lock (in seconds; 48 hours by default)\n:type max_lifetime: int\n"\
      ":arg force: take the lock even if another request is currently hold it -- dangerous!\n:type force: bool\n"\
      ":returns: `True` on success\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"\
      ":raises pyhive.AlreadyLockedError: if another request is currently holding the lock (its request ID is stored in the exception object's `args[1]`)\n"
    },
    { "req_unlock", (PyCFunction)Proc_req_unlock, METH_VARARGS | METH_KEYWORDS,
      "req_unlock(key, req = pyhive.proc.req_id, force = False)\n"\
      "Remove a lock on a named entity (typically filesystem path) for the specified request\n\n"\
      ":arg key: name of entity (typically filesystem path) to unlock\n:type key: str\n"\
      ":arg req: request ID on whose behalf to unlock (pyhive.proc.req_id by default)\n:type req: int\n"\
      ":arg force: remove the lock even if another request is currently hold it -- dangerous!\n:type force: bool\n"\
      ":returns: `True` if the lock was removed, `False` if there wasn't any lock to remove\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
    },
    { "req_checklock", (PyCFunction)Proc_req_checklock, METH_VARARGS | METH_KEYWORDS,
      "req_checklock(key)\n"\
      "Check which request, if any, is holding a lock on a named entity\n\n"\
      ":arg key: name of entity (typically filesystem path) that might be locked\n:type key: str\n"\
      ":returns: request ID (non-zero integer) holding the lock, or `None` if the key is not locked\n"\
      ":raises TypeError: if `pyhive.proc` is not initialized or parameters are of the wrong type\n"
    },
    { NULL }
};

static PyObject * Proc_get_form(pyhive::Proc * self, void * closure)
{
    ASSERT_SELF_PROC(NULL);
    if( !self->cached_form_proxy ) {
        self->cached_form = PyDict_New();
        for (idx i=0; i<self->proc->pForm->dim(); i++) {
            const char * key = static_cast<const char*>(self->proc->pForm->id(i));
            const char * value = self->proc->pForm->value(key);
            PyDict_SetItemString(self->cached_form, key, PyUnicode_FromString(value));
        }
        self->cached_form_proxy = PyDictProxy_New(self->cached_form);
    }
    Py_INCREF(self->cached_form_proxy);
    return self->cached_form_proxy;
}

static PyObject * Proc_get_svc(pyhive::Proc * self, void * closure)
{
    ASSERT_SELF_PROC(NULL);
    if( !self->cached_svc ) {
        if( self->proc->svc.svcID ) {
            if( pyhive::SvcObj * sobj = Svc_new(&SvcType, 0, 0) ) {
                memcpy(&sobj->svc, &self->proc->svc, sizeof(self->proc->svc));
                self->cached_svc = sobj;
                Py_INCREF(self->cached_svc);
                return (PyObject*)self->cached_svc;
            }
        }
        Py_RETURN_NONE;
    } else {
        Py_XINCREF(self->cached_svc);
        return (PyObject*)self->cached_svc;
    }
}

static PyObject * Proc_get_req_id(pyhive::Proc * self, void * closure)
{
    ASSERT_SELF_PROC(NULL);
    if( self->proc->reqId ) {
        return pyhive::idx2py(self->proc->reqId);
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject * Proc_get_req_slice_id(pyhive::Proc * self, void * closure)
{
    ASSERT_SELF_PROC(NULL);
    return pyhive::idx2py(self->proc->reqSliceId);
}

static PyObject * Proc_get_req_slice_cnt(pyhive::Proc * self, void * closure)
{
    ASSERT_SELF_PROC(NULL);
    return pyhive::idx2py(self->proc->reqSliceCnt);
}

static PyObject * Proc_get_grp_id(pyhive::Proc * self, void * closure)
{
    ASSERT_SELF_PROC(NULL);
    if( self->proc->grpId ) {
        return pyhive::idx2py(self->proc->grpId);
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject * Proc_get_obj(pyhive::Proc * self, void * closure)
{
    ASSERT_SELF_PROC(NULL);
    if( !self->cached_obj ) {
        if( self->proc->objs.dim() ) {
            pyhive::Obj * obj = pyhive::Obj::create();
            obj->uobj = self->proc->user->objFactory(self->proc->objs[0].Id());
            self->cached_obj = (PyObject*)obj;
        } else {
            self->cached_obj = Py_None;
        }
    }
    Py_INCREF(self->cached_obj);
    return self->cached_obj;
}

static PyGetSetDef Proc_getsetters[] = {
    { (char*)"svc", (getter)Proc_get_svc, NULL, (char*)"`pyhive.Svc` object describing current HIVE service, or `None` if current HIVE service is undefined", NULL },
    { (char*)"form", (getter)Proc_get_form, NULL, (char*)"Current request form as a dictionary", NULL },
    { (char*)"req_id", (getter)Proc_get_req_id, NULL, (char*)"Current request ID (positive integer), or `None`", NULL },
    { (char*)"req_slice_id", (getter)Proc_get_req_slice_id, NULL, (char*)"ID (in range 0 .. `req_slice_cnt` - 1) of current request within automatically parallelized group of requests", NULL },
    { (char*)"req_slice_cnt", (getter)Proc_get_req_slice_cnt, NULL, (char*)"Number of requests within automatically generated parallelized group of requests", NULL },
    { (char*)"grp_id", (getter)Proc_get_grp_id, NULL, (char*)"Current request group ID (positive integer), or `None`", NULL },
    { (char*)"obj", (getter)Proc_get_obj, NULL, (char*)"Current HIVE process object (:py:class:`pyhive.Obj` instance), or `None`", NULL },
    { NULL }
};

static PyTypeObject ProcType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.Proc",
    sizeof(pyhive::Proc),
    0,
    (destructor)Proc_dealloc,
    0,
    0,
    0,
    0,
    0,
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
    "HIVE process\n\nClass with a unique singleton instance -- `pyhive.proc`",
    0,
    0,
    0,
    0,
    0,
    0,
    Proc_methods,
    0,
    Proc_getsetters,
    0,
    0,
    0,
    0,
    0,
    (initproc)Proc_init,
    0,
    Proc_new,
};

static PyMethodDef pyhive_methods[] = {
    { NULL }
};

static PyObject * pyhive_mod = 0;

PyObject * pyhive::getModule() { return pyhive_mod; }

pyhive::Proc * pyhive::Proc::singleton()
{
    if( PyObject * pyhive_mod = pyhive::getModule() ) {
        PyObject * pyhive_proc_obj = PyObject_GetAttrString(pyhive_mod, "proc");
        if( pyhive_proc_obj && pyhive_proc_obj->ob_type == &ProcType ) {
            Py_DECREF(pyhive_proc_obj);
            pyhive::Proc * pyhive_proc = (pyhive::Proc*)pyhive_proc_obj;
            if( likely(pyhive_proc->proc) ) {
                return pyhive_proc;
            } else {
                return NULL;
            }
        } else {
            Py_XDECREF(pyhive_proc_obj);
        }
    }
    return NULL;
}

PyMODINIT_FUNC
PyInit_pyhive(void)
{
    static struct PyModuleDef pyhive_mod_def = {
        PyModuleDef_HEAD_INIT,
        "pyhive",
        "HIVE Python interface",
        -1,
        pyhive_methods,
        NULL,
        NULL,
        NULL,
        NULL,
    };
    pyhive_mod = PyModule_Create(&pyhive_mod_def);

    if( !pyhive::Mex::typeinit(pyhive_mod) ||
        !pyhive::Id::typeinit(pyhive_mod) ||
        !pyhive::TZ::typeinit(pyhive_mod) ||
        !pyhive::Type::typeinit(pyhive_mod) ||
        !pyhive::Obj::typeinit(pyhive_mod) ||
        !pyhive::QPSvc::typeinit(pyhive_mod) ||
        !pyhive::Seq::typeinit(pyhive_mod) ||
        !pyhive::Ion::typeinit(pyhive_mod) ||
        !pyhive::IonWander::typeinit(pyhive_mod) ||
        !pyhive::TaxIon::typeinit(pyhive_mod) )
    {
        return NULL;
    }

    if( PyType_Ready(&ProcType) < 0 ) {
        return NULL;
    }

    pyhive::RuntimeError = PyErr_NewExceptionWithDoc((char*)"pyhive.RuntimeError", (char*)"Exception raised in HIVE runtime libraries", PyExc_EnvironmentError, NULL);
    Py_INCREF(pyhive::RuntimeError);
    PyModule_AddObject(pyhive_mod, "RuntimeError", pyhive::RuntimeError);

    Py_INCREF(&ProcType);
    PyModule_AddObject(pyhive_mod, "Proc", (PyObject*)&ProcType);

    if( PyType_Ready(&SvcType) < 0 ) {
        return NULL;
    }
    Py_INCREF(&SvcType);
    PyModule_AddObject(pyhive_mod, "Svc", (PyObject*)&SvcType);

    pyhive_request_killed = PyErr_NewExceptionWithDoc((char*)"pyhive.RequestKilledError", (char*)"Special exception indicating that the current HIVE request has been killed\n\nIf for some reason you wish to handle this exception, at the end of the handling code you must call::\n\n    pyhive.proc.req_set_status(pyhive.req_status.KILLED)", NULL, NULL);
    Py_INCREF(pyhive_request_killed);
    PyModule_AddObject(pyhive_mod, "RequestKilledError", pyhive_request_killed);

    if( PyType_Ready(&AlreadyLockedError) < 0 ) {
        return NULL;
    }
    Py_INCREF(&AlreadyLockedError);
    PyModule_AddObject(pyhive_mod, "AlreadyLockedError", (PyObject*)&AlreadyLockedError);

    static struct PyModuleDef req_status_mod_def = {
        PyModuleDef_HEAD_INIT,
        "pyhive.req_status",
        "Request statuses\n\n"\
                ".. py:data:: ANY\n\n    Unknown status\n\n"\
                ".. py:data:: WAITING\n\n    Waiting to be grabbed\n\n"\
                ".. py:data:: PROCESSING\n\n    Grabbed, but computation not yet started\n\n"\
                ".. py:data:: RUNNING\n\n    Computation is running\n\n"\
                ".. py:data:: SUSPENDED\n\n    Suspended by user\n\n"\
                ".. py:data:: DONE\n\n    Computation finished\n\n"\
                ".. py:data:: KILLED\n\n    Killed by user\n\n"\
                ".. py:data:: PROG_ERROR\n\n    Computation stopped due to error\n\n"\
                ".. py:data:: SYS_ERROR\n\n    Computation stopped due to low-level system error\n\n",
        -1,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
    static PyObject * req_status_mod = PyModule_Create(&req_status_mod_def);
    Py_INCREF(req_status_mod);
    PyModule_AddObject(pyhive_mod, "req_status", req_status_mod);

    PyModule_AddIntConstant(req_status_mod, "ANY", sQPrideBase::eQPReqStatus_Any);
    PyModule_AddIntConstant(req_status_mod, "WAITING", sQPrideBase::eQPReqStatus_Waiting);
    PyModule_AddIntConstant(req_status_mod, "PROCESSING", sQPrideBase::eQPReqStatus_Processing);
    PyModule_AddIntConstant(req_status_mod, "RUNNING", sQPrideBase::eQPReqStatus_Running);
    PyModule_AddIntConstant(req_status_mod, "SUSPENDED", sQPrideBase::eQPReqStatus_Suspended);
    PyModule_AddIntConstant(req_status_mod, "DONE", sQPrideBase::eQPReqStatus_Done);
    PyModule_AddIntConstant(req_status_mod, "KILLED", sQPrideBase::eQPReqStatus_Killed);
    PyModule_AddIntConstant(req_status_mod, "PROG_ERROR", sQPrideBase::eQPReqStatus_ProgError);
    PyModule_AddIntConstant(req_status_mod, "SYS_ERROR", sQPrideBase::eQPReqStatus_SysError);

    static struct PyModuleDef req_action_mod_def = {
         PyModuleDef_HEAD_INIT,
         "pyhive.req_action",
         "Request job handler actions\n\n"\
                 ".. py:data:: ANY\n\n    Unknown action\n\n"\
                 ".. py:data:: NONE\n\n    On hold (will not be executed by job handler) until action is changed\n\n"\
                 ".. py:data:: RUN\n\n    Job handler should execute this job\n\n"\
                 ".. py:data:: KILL\n\n    Job handler should kill this job\n\n"\
                 ".. py:data:: SUSPEND\n\n    Job handler should suspend this job\n\n"\
                 ".. py:data:: RESUME\n\n    Job handler should resume the previously suspended job\n\n"\
                 ".. py:data:: SPLIT\n\n    Job handler should split this job\n\n"\
                 ".. py:data:: POSTPONE\n\n    Job handler should wait for the user to run the job\n\n",
         -1,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
     };
    static PyObject * req_action_mod = PyModule_Create(&req_action_mod_def);

    Py_INCREF(req_action_mod);
    PyModule_AddObject(pyhive_mod, "req_action", req_action_mod);

    PyModule_AddIntConstant(req_action_mod, "ANY", sQPrideBase::eQPReqAction_Any);
    PyModule_AddIntConstant(req_action_mod, "NONE", sQPrideBase::eQPReqAction_None);
    PyModule_AddIntConstant(req_action_mod, "RUN", sQPrideBase::eQPReqAction_Run);
    PyModule_AddIntConstant(req_action_mod, "KILL", sQPrideBase::eQPReqAction_Kill);
    PyModule_AddIntConstant(req_action_mod, "SUSPEND", sQPrideBase::eQPReqAction_Suspend);
    PyModule_AddIntConstant(req_action_mod, "RESUME", sQPrideBase::eQPReqAction_Resume);
    PyModule_AddIntConstant(req_action_mod, "SPLIT", sQPrideBase::eQPReqAction_Split);
    PyModule_AddIntConstant(req_action_mod, "POSTPONE", sQPrideBase::eQPReqAction_Postpone);


    static struct PyModuleDef log_type_mod_def = {
        PyModuleDef_HEAD_INIT,
        "pyhive.log_type",
        "Request logging types\n\n"\
                ".. py:data:: MIN\n\n    Minimum ( = `pyhive.log_type.TRACE`)\n\n"\
                ".. py:data:: TRACE\n\n    Trace\n\n"\
                ".. py:data:: DEBUG\n\n    Debug\n\n"\
                ".. py:data:: INFO\n\n    Info\n\n"\
                ".. py:data:: WARNING\n\n    Warning\n\n"\
                ".. py:data:: ERROR\n\n    Error\n\n"\
                ".. py:data:: FATAL\n\n    Fatal\n\n"\
                ".. py:data:: MAX\n\n    Maximum ( = `pyhive.log_type.FATAL`)\n\n",
        -1,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
    static PyObject * log_type_mod = PyModule_Create(&log_type_mod_def);

    Py_INCREF(log_type_mod);
    PyModule_AddObject(pyhive_mod, "log_type", log_type_mod);

    PyModule_AddIntConstant(log_type_mod, "MIN", sQPrideBase::eQPLogType_Min);
    PyModule_AddIntConstant(log_type_mod, "TRACE", sQPrideBase::eQPLogType_Trace);
    PyModule_AddIntConstant(log_type_mod, "DEBUG", sQPrideBase::eQPLogType_Debug);
    PyModule_AddIntConstant(log_type_mod, "INFO", sQPrideBase::eQPLogType_Info);
    PyModule_AddIntConstant(log_type_mod, "WARNING", sQPrideBase::eQPLogType_Warning);
    PyModule_AddIntConstant(log_type_mod, "ERROR", sQPrideBase::eQPLogType_Error);
    PyModule_AddIntConstant(log_type_mod, "FATAL", sQPrideBase::eQPLogType_Fatal);
    PyModule_AddIntConstant(log_type_mod, "MAX", sQPrideBase::eQPLogType_Max);

    return pyhive_mod;
}
