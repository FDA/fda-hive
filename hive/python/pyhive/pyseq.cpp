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

#include <violin/hiveseq.hpp>

using namespace slib;

static PyObject * Seq_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::Seq * self = (pyhive::Seq*)type->tp_alloc(type, 0);
    self->hive_ids = 0;
    self->phive_seq = 0;
    return (PyObject*)self;
}

static void Seq_dealloc(pyhive::Seq *self)
{
    Py_XDECREF(self->hive_ids);
    delete self->phive_seq;
    self->ob_type->tp_free((PyObject*)self);
}

static int Seq_init(pyhive::Seq *self, PyObject * args, PyObject * kwds);

static PyObject* Seq_repr(pyhive::Seq * self)
{
    sStr buf("<%s ", self->ob_type->tp_name);
    if( self->hive_ids && PySequence_Check(self->hive_ids) ) {
        idx cnt = PySequence_Length(self->hive_ids);
        for(idx i = 0; i < cnt; i++) {
            pyhive::Id * id = pyhive::Id::check(PySequence_GetItem(self->hive_ids, i));
            if( id ) {
                if( i ) {
                    buf.addString(";");
                }
                id->hive_id.print(buf);
            }
        }
        if( cnt ) {
            buf.addString(" ");
        }
    }
    buf.printf("at %p>", self);
    return PyString_FromString(buf.ptr());
}

enum PyhiveSeqModes {
    Pyhive_FASTA = 0,
    Pyhive_FASTQ
};

static bool is_pyhive_hive_seq_mode(int i)
{
    return i == Pyhive_FASTA || i == Pyhive_FASTQ;
}

static PyObject * Seq_row_len(pyhive::Seq * self, PyObject * args, PyObject * kwds)
{
    idx irow = 0;

    static const char * kwlist[] = { "row", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "L", (char**)kwlist, &irow) ) {
        return 0;
    }
    if( irow < 0 || irow >= self->phive_seq->dim() ) {
        PyErr_SetString(PyExc_TypeError, "valid row number expected");
        return NULL;
    }
    return pyhive::idx2py(self->phive_seq->len(irow));
}

static PyObject * Seq_row_name(pyhive::Seq * self, PyObject * args, PyObject * kwds)
{
    idx irow = 0;

    static const char * kwlist[] = { "row", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "L", (char**)kwlist, &irow) ) {
        return 0;
    }
    if( irow < 0 || irow >= self->phive_seq->dim() ) {
        PyErr_SetString(PyExc_TypeError, "valid row number expected");
        return NULL;
    }
    const char * id = self->phive_seq->id(irow);
    if( !id ) {
        PyErr_SetString(PyExc_SystemError, "row name could not be retrieved");
        return NULL;
    }
    return PyString_FromString(id);
}

static PyObject * Seq_print_fastx(pyhive::Seq * self, PyObject * args, PyObject * kwds)
{
    idx start = 0;
    idx end = 0;
    int seq_mode = 0;
    PyObject * into_obj = 0;

    static const char * kwlist[] = { "mode", "start", "length", "into", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|iLLO", (char**)kwlist, &seq_mode, &start, &end, &into_obj) ) {
        return 0;
    }

    if( start < 0 || start >= self->phive_seq->dim() ) {
        PyErr_SetString(PyExc_TypeError, "valid start row number expected");
        return NULL;
    }
    if( end < 0 || end > self->phive_seq->dim() ) {
        PyErr_SetString(PyExc_TypeError, "valid end row number expected");
        return NULL;
    } else if( end == 0 ) {
        end = self->phive_seq->dim();
    }

    if( !is_pyhive_hive_seq_mode(seq_mode) ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.seq_mode constant expected");
        return NULL;
    }
    pyhive::Mex * into_mex = 0;
    if( into_obj ) {
        into_mex = pyhive::Mex::check(into_obj);
        if( !into_mex ) {
            PyErr_SetString(PyExc_TypeError, "pyhive.Mex expected");
            return NULL;
        }
        if( into_mex->str.flags & sMex::fReadonly ) {
            PyErr_SetString(PyExc_TypeError, "writeable pyhive.Mex expected");
            return NULL;
        }
        Py_INCREF(into_mex);
    } else {
        into_mex = pyhive::Mex::create();
    }

    if( !self->phive_seq->printFastX(&into_mex->str, (seq_mode == Pyhive_FASTQ), start, end, 0, true, false, 0, 0, true, 0) ) {
        Py_DECREF(into_mex);
        PyErr_SetString(PyExc_SystemError, "sequence I/O failed");
        return 0;
    }
    return (PyObject*)into_mex;
}

static PyObject * Seq_print_fastx_row(pyhive::Seq * self, PyObject * args, PyObject * kwds)
{
    idx irow = 0;
    idx start = 0;
    idx length = 0;
    int seq_mode = 0;
    PyObject * into_obj = 0;

    static const char * kwlist[] = { "row", "mode", "start", "length", "into", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "L|iLLO", (char**)kwlist, &irow, &seq_mode, &start, &length, &into_obj) ) {
        return 0;
    }
    if( irow < 0 || irow >= self->phive_seq->dim() ) {
        PyErr_SetString(PyExc_TypeError, "valid row number expected");
        return NULL;
    }
    if( !is_pyhive_hive_seq_mode(seq_mode) ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.seq_mode constant expected");
        return NULL;
    }
    pyhive::Mex * into_mex = 0;
    if( into_obj ) {
        into_mex = pyhive::Mex::check(into_obj);
        if( !into_mex ) {
            PyErr_SetString(PyExc_TypeError, "pyhive.Mex expected");
            return NULL;
        }
        if( into_mex->str.flags & sMex::fReadonly ) {
            PyErr_SetString(PyExc_TypeError, "writeable pyhive.Mex expected");
            return NULL;
        }
        Py_INCREF(into_mex);
    } else {
        into_mex = pyhive::Mex::create();
    }

    if( !self->phive_seq->printFastXRow(&into_mex->str, (seq_mode == Pyhive_FASTQ), irow, start, length, 0, true) ) {
        Py_DECREF(into_mex);
        PyErr_SetString(PyExc_SystemError, "sequence I/O failed");
        return 0;
    }
    return (PyObject*)into_mex;
}

static PyMethodDef Seq_methods[] = {
    {
        "row_len", (PyCFunction)Seq_row_len,
        METH_VARARGS | METH_KEYWORDS,
        "row_len(row)\n\nLength of specified row (subsequence)\n\n"\
        ":arg row: sequence row number, 0-based\n:type row: `int`\n"\
        ":rtype: int"
    },
    {
        "row_name", (PyCFunction)Seq_row_name,
        METH_VARARGS | METH_KEYWORDS,
        "row_name(row)\n\nName (identifier) of specified row (subsequence)\n\n"\
        ":arg row: sequence row number, 0-based\n:type row: `int`\n"\
        ":rtype: str"
    },
    {
        "print_fastx", (PyCFunction)Seq_print_fastx,
        METH_VARARGS | METH_KEYWORDS,
        "print_fastx(mode = pyhive.seq_mode.FASTA, start = 0, length = 0, into = None)\n\nPrint in FastA/FastQ format\n\n"\
        ":arg mode: printing mode\n:type mode: `pyhive.seq_mode`\n"\
        ":arg start: start row, 0-based\n:type start: `int`\n"\
        ":arg end: 1 higher than index of last row to print, or 0 (default) to print all rows\n:type length: `int`\n"\
        ":arg into: buffer/file into which to print (or `None` to print into a new `pyhive.Mex`)\n:type mode: `pyhive.Mex`\n"\
        ":returns: new `pyhive.Mex` object with printed sequence (or the `into` argument into which the sequence was printed)"
    },
    {
        "print_fastx_row", (PyCFunction)Seq_print_fastx_row,
        METH_VARARGS | METH_KEYWORDS,
        "print_fastx_row(row, mode = pyhive.seq_mode.FASTA, start = 0, length = 0, into = None)\n\nPrint specific row in FastA/FastQ format\n\n"\
        ":arg row: sequence row number, 0-based\n:type row: `int`\n"\
        ":arg mode: printing mode\n:type mode: `pyhive.seq_mode`\n"\
        ":arg start: start position, 0-based\n:type start: `int`\n"\
        ":arg length: length to print, or 0 (default) to print all\n:type length: `int`\n"\
        ":arg into: buffer/file into which to print (or `None` to print into a new `pyhive.Mex`)\n:type mode: `pyhive.Mex`\n"\
        ":returns: new `pyhive.Mex` object with printed sequence row (or the `into` argument into which the sequence row was printed)"
    },
    { NULL }
};

static PyObject * Seq_get_hive_ids(pyhive::Seq * self, void * closure)
{
    Py_INCREF(self->hive_ids);
    return self->hive_ids;
}

static PyObject * Seq_get_is_fasta(pyhive::Seq * self, void * closure)
{
    if( self->phive_seq->isFastA() ) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject * Seq_get_dim(pyhive::Seq * self, void * closure)
{
    return pyhive::idx2py(self->phive_seq->dim());
}

static PyGetSetDef Seq_getsetters[] = {
    { (char*)"hive_ids", (getter)Seq_get_hive_ids, NULL, (char*)"List of HIVE IDs identifying objects from which the sequence is created", NULL },
    { (char*)"is_fasta", (getter)Seq_get_is_fasta, NULL, (char*)"`True` if all rows in the sequence are FastA format", NULL },
    { (char*)"dim", (getter)Seq_get_dim, NULL, (char*)"Number of rows (subsequences) in the sequence", NULL },
    { NULL }
};

PyTypeObject SeqType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.Seq",              // tp_name
    sizeof(pyhive::Seq),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)Seq_dealloc,   // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)Seq_repr,        // tp_repr
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
    "HIVE genomic sequence\n\n"\
    "Initialized from a HIVE ID (as integer or string or `pyhive.Id` object), or list of ids:\n"\
    "   >>> pyhive.Seq(12345)\n"
    "   <pyhive.Seq 12345 at 0x7fa2be26a0f0>\n"\
    "   >>> pyhive.Seq([12345, pyhive.Id(12346)])\n"\
    "   <pyhive.Seq 12345;12346 at 0x7fa2be26a0f0>\n",   // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    Seq_methods,               // tp_methods
    0,                         // tp_members
    Seq_getsetters,            // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)Seq_init,        // tp_init
    0,                         // tp_alloc
    Seq_new,                   // tp_new
};

static int Seq_init(pyhive::Seq * self, PyObject *args, PyObject *kwds)
{
    pyhive::Proc * pyhive_proc = pyhive::Proc::singleton();
    if( !pyhive_proc ) {
        PyErr_SetString(PyExc_AssertionError, "pyhive.proc has not been initialized");
        return -1;
    }

    PyObject * ids_arg = 0;
    if( !PyArg_ParseTuple(args, "O", &ids_arg) ) {
        return -1;
    }

    self->hive_ids = pyhive::Id::parseList(ids_arg);
    if( !self->hive_ids || !PyList_Check(self->hive_ids) ) {
        Py_XDECREF(self->hive_ids);
        self->hive_ids = 0;
        return -1;
    }

    sStr buf;
    idx cnt = PyList_GET_SIZE(self->hive_ids);
    for(idx i = 0; i < cnt; i++) {
        if( i ) {
            buf.addString(";", 1);
        }
        pyhive::Id::check(PyList_GET_ITEM(self->hive_ids, i))->hive_id.print(buf);
    }

    self->phive_seq = new sviolin::sHiveseq(pyhive_proc->proc->user, buf.ptr(0), sBioseq::eBioModeLong);
    return 0;
}

//static
bool pyhive::Seq::typeinit(PyObject * mod)
{
    if( PyType_Ready(&SeqType) < 0 ) {
        return false;
    }
    Py_INCREF(&SeqType);
    PyModule_AddObject(mod, "Seq", (PyObject*)&SeqType);

    PyObject * seq_modes_mod = Py_InitModule3("pyhive.seq_mode", NULL, "pyhive.Seq modes\n\n"\
        ".. py:data:: FASTA\n\n    FastA format (no qualities)\n\n"\
        ".. py:data:: FASTQ\n\n    FastQ format (with qualities)\n\n"\
    );
    Py_INCREF(seq_modes_mod);
    PyModule_AddObject(mod, "seq_mode", seq_modes_mod);

    PyModule_AddIntConstant(seq_modes_mod, "FASTA", Pyhive_FASTA);
    PyModule_AddIntConstant(seq_modes_mod, "FASTQ", Pyhive_FASTQ);

    sBioseq::initModule(sBioseq::eACGT);

    return true;
}
