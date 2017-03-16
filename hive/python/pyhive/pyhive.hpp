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

#pragma once
#ifndef sLib_pyhive_h
#define sLib_pyhive_h

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <qlib/QPrideProc.hpp>
#include <ulib/ulib.hpp>
#include <ulib/utype2.hpp>

namespace sviolin {
    class sHiveseq;
};

namespace slib {
    class sTaxIon;
    class sIon;
    class sIonWander;

    namespace pyhive {
        PyObject * udx2py(udx u);
        PyObject * idx2py(idx i);
        bool py2form(sVar & form_out, PyObject * pydict);

        struct Proc;
        class sQPyProc: public sQPrideProc {
            private:
                static sQPyProc * _singleton;
                pyhive::Proc * _pyproc;
                PyObject * _run_mod;
                sStr _srv_name;

            public:
                typedef sQPrideProc Tparent;
                sQPyProc(pyhive::Proc * pyproc, const char * defline00, const char * srv) : sQPrideProc(defline00, srv)
                {
                    _pyproc = pyproc;
                    _srv_name.addString(srv);
                    _run_mod = 0;
                }

                virtual idx OnGrab(idx forceReq=0);
                virtual idx OnExecute(idx req);

                void setRunMod(PyObject * run_mod) { _run_mod = run_mod; }
                const char * getSrvName() const { return _srv_name; }

                static sQPyProc * getSingleton() { return _singleton; }
                static void setSingleton(sQPyProc * p) { _singleton = p; }

                void logPythonTraceback();
        };

        struct Mex {
            PyObject_HEAD;
            sIO str;
            sMex::Pos last_line;

            static bool typeinit(PyObject * mod);
            static Mex * check(PyObject * o);
            static Mex * create(); //!< allocate and incref new pyhive::Mex

            bool init(const char * flnm = 0, idx flags = sMex::fBlockDoubling);
        };

        struct Id {
            PyObject_HEAD;
            sHiveId hive_id;

            static bool typeinit(PyObject * mod);
            static Id * check(PyObject * o);
            static Id * create(); //!< allocate and incref new pyhive::Id

            bool init(PyObject * arg);
            bool init(const sHiveId & id);

            static PyObject * parseList(PyObject * arg); //!< parse argument as a PyList of pyhive::Id
        };

        // super-naive timezone info class, since python doesn't provide any :/
        struct TZ {
            PyObject_HEAD;
            idx utc_offset;

            static bool typeinit(PyObject * mod);
            static TZ * check(PyObject * o);
        };
        PyObject * parseDate(const char * s);
        PyObject * parseTime(const char * s);
        PyObject * parseDateTime(const char * s);
        // print in ISO 8601 format
        const char * printDate(sStr & buf, PyObject * val);
        const char * printTime(sStr & buf, PyObject * val);
        const char * printDateTime(sStr & buf, PyObject * val);

        struct Type {
            PyObject_HEAD;
            const sUsrType2 * utype;
            PyObject * cached_field_names;

            static bool typeinit(PyObject * mod);
            static Type * check(PyObject * o);
            static Type * find(const sHiveId & id); //!< find and incref pyhive::Type (allocate if needed)
        };

        struct Obj {
            PyObject_HEAD;
            sUsrObj * uobj;

            static bool typeinit(PyObject * mod);
            static Obj * check(PyObject * o);
            static Obj * create(); //!< allocate and incref new pyhive::Obj

            bool init(PyObject * arg);
            bool init(Id * arg) { return init((PyObject*)arg); }
        };

        struct SvcObj {
            PyObject_HEAD;
            sQPrideBase::Service svc;
            PyObject * cdate;
            PyObject * name;
            PyObject * title;
            PyObject * cmd_line;
            PyObject * hosts;
            PyObject * emails;
            PyObject * categories;

            static SvcObj * check(PyObject * o);
            static SvcObj * create(); //!< allocate and incref new pyhive::SvcObj
        };

        struct Proc {
            PyObject_HEAD;
            sQPyProc * proc;

            // proc->form access
            PyObject * cached_form;
            PyObject * cached_form_proxy;
            // proc->svc access
            pyhive::SvcObj * cached_svc;
            // copy of proc->objs[0]
            PyObject * cached_obj;

            //! get initialized singleton pyhive.proc object (without changing its reference count) or 0
            static Proc * singleton();
        };

        struct Seq {
            PyObject_HEAD;
            PyObject * hive_ids;
            sviolin::sHiveseq * phive_seq;

            static bool typeinit(PyObject * mod);
            static Seq * check(PyObject * o);
        };

        struct Ion {
            PyObject_HEAD;
            sIon * ion;
            sHiveId obj_id;
            sStr name;

            static bool typeinit(PyObject * mod);
            static Ion * check(PyObject * o);
            static sIon * getIon(PyObject * o);
        };

        struct TaxIon {
            Ion head;
            PyObject * tax_db_obj;
            sTaxIon * ptax_ion;

            static bool typeinit(PyObject * mod);
            static TaxIon * check(PyObject * o);
        };

        struct IonWander {
            PyObject_HEAD;
            sStr query;
            sIonWander * wander;
            PyObject * ions;
            pyhive::Mex * result;

            static bool typeinit(PyObject * mod);
            static IonWander * check(PyObject * o);
        };

        class sQPySvc;
        struct QPSvc {
            PyObject_HEAD;
            sQPySvc * qpysvc;
            pyhive::SvcObj * svc_obj;
            PyObject * req_ids;

            static bool typeinit(PyObject * mod);
            static QPSvc * check(PyObject * o);
        };

        PyObject * getModule();
    };
};

#endif
