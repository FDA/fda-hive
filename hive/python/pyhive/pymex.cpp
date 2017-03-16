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

static PyObject * Mex_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    pyhive::Mex * self = (pyhive::Mex*)type->tp_alloc(type, 0);
    if( self ) {
        new(&self->str) sStr;
        self->str.init(sMex::fBlockDoubling);
        self->last_line.pos = sIdxMax;
        self->last_line.size = 0;
    }
    return (PyObject*)self;
}

static int Mex_init(pyhive::Mex * self, PyObject *args, PyObject *kwds)
{
    const char * src = 0;
    Py_ssize_t src_len = 0;
    const char * flnm = 0;
    idx flg = sMex::fBlockDoubling;
    static const char * kwlist[] = { "data", "filename", "flags", NULL };
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|s#ss:pyhive.Mex.__init__", (char**)kwlist, &src, &src_len, &flnm, &flg) ) {
        return -1;
    }
    if( flg < 0 || flg >= sMex::fLast ) {
        PyErr_SetString(PyExc_TypeError, "invalid flag; expected bitwise-or of pyhive.mex_flag constants");
        return -1;
    }
    if( !self->init(flnm, flg) ) {
        return -1;
    }
    if( src && src_len && !flnm ) {
        self->str.add(src, src_len);
        self->str.add0cut();
    }
    return 0;
}

static void Mex_dealloc(pyhive::Mex *self)
{
    self->str.destroy();
    self->ob_type->tp_free((PyObject*)self);
}

static inline int getFilehandle(pyhive::Mex *self)
{
    return (int)((self->str.flags>>(sMex_FileHandleShift)) & 0xFFFF);
}

// sequence protocol

static Py_ssize_t Mex_length(pyhive::Mex *self)
{
    return self->str.length();
}

static bool getPtrLength(PyObject * o, char ** pptr, idx * plength)
{
    *pptr = 0;
    *plength = 0;
    if( pyhive::Mex * s = pyhive::Mex::check(o) ) {
        *pptr = s->str.ptr();
        *plength = s->str.length();
        return true;
    } else if( PyString_Check(o) ) {
        Py_ssize_t pylength = 0;
        if( PyString_AsStringAndSize(o, pptr, &pylength) != 0 ) {
            return false;
        }
        *plength = pylength;
        return true;
    } else {
        PyErr_SetString(PyExc_TypeError, "string expected");
        return false;
    }
}

static PyObject* Mex_concat(PyObject *o1, PyObject *o2)
{
    char * src1 = 0, * src2 = 0;
    idx len1 = 0, len2 = 0;
    if( !getPtrLength(o1, &src1, &len1) || !getPtrLength(o2, &src2, &len2) ) {
        return NULL;
    }
    pyhive::Mex * out = pyhive::Mex::create();
    if( len1 ) {
        out->str.add(src1, len1);
    }
    if( len2 ) {
        out->str.add(src2, len2);
    }
    out->str.add0cut();
    return (PyObject*)out;
}

static PyObject* Mex_getitem(pyhive::Mex * self, Py_ssize_t i)
{
    if( i < 0 ) {
        i += self->str.length();
    }
    if( i < 0 || i >= self->str.length() ) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }
    return PyString_FromStringAndSize(self->str.ptr(i), 1);
}

static int Mex_setitem(pyhive::Mex * self, Py_ssize_t i, PyObject * v)
{
    if( self->str.flags & sMex::fReadonly ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.Mex is in read-only mode");
        return -1;
    }
    if( i < 0 ) {
        i += self->str.length();
    }
    if( i < 0 || i > self->str.length() /* note we allow write to last character */ ) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return -1;
    }
    if( !v ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.Mex does not currently support __del__");
        return -1;
    }
    char * src = 0;
    idx len = 0;
    if( !getPtrLength(v, &src, &len) ) {
        return -1;
    }
    if( len != 1 ) {
        PyErr_SetString(PyExc_ValueError, "1 byte expected");
        return -1;
    }
    self->str.resize(i + 1);
    self->str[i] = *src;
    return 0;
}

PyObject* Mex_getslice(pyhive::Mex * self, Py_ssize_t i1, Py_ssize_t i2)
{
    if( i1 < 0 ) {
        i1 += self->str.length();
    }
    if( i2 < 0 ) {
        i2 += self->str.length();
    }
    idx start = sMax<idx>(i1, 0);
    idx end = sMin<idx>(i2, self->str.length());
    idx len = end - start;
    if( start < self->str.length() && len > 0 ) {
        return PyString_FromStringAndSize(self->str.ptr(start), len);
    } else {
        return PyString_FromString("");
    }
}

static PyObject* Mex_inplaceconcat(PyObject *o1, PyObject *o2)
{
    pyhive::Mex * self = pyhive::Mex::check(o1);
    if( !self ) {
        PyErr_SetString(PyExc_TypeError, "pyhive::Mex expected as left hand side of +=");
        return NULL;
    }
    if( self->str.flags & sMex::fReadonly ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.Mex is in read-only mode");
        return NULL;
    }
    char * src = 0;
    idx len = 0;
    if( !getPtrLength(o2, &src, &len) ) {
        return NULL;
    }
    if( len ) {
        self->str.add(src, len);
    }
    if( !getFilehandle(self) ) {
        self->str.add0cut();
    }
    Py_INCREF(self);
    return (PyObject*)self;
}

// stringification: methods that operate on the whole buffer

static PyObject * Mex_str(pyhive::Mex *self)
{
    if( self->str.length() ) {
        return PyString_FromStringAndSize(self->str.ptr(), self->str.length());
    } else {
        return PyString_FromString("");
    }
}

static int Mex_print(pyhive::Mex *self, FILE * file, int flags)
{
    if( flags & Py_PRINT_RAW ) {
        if( !file ) {
            PyErr_SetString(PyExc_ValueError, "Null FILE pointer");
            return -1;
        }
        idx pos = 0;
        idx len_remaining = self->str.length() - pos;
        while(len_remaining > 0 ) {
            idx len_printed = fwrite(self->str.ptr(pos), 1, len_remaining, file);
            if( len_printed <= 0 ) {
                break;
            }
            len_remaining -= len_printed;
            pos += len_printed;
        }
        return 0;
    } else {
        PyObject * r = PyObject_Repr((PyObject*)self);
        int ret = PyObject_Print(r, file, flags);
        Py_XDECREF(r);
        return ret;
    }
}

// file-like object API; methods operate relative to self->last_line

static PyObject * Mex_close(pyhive::Mex *self)
{
    self->str.destroy();
    Py_RETURN_NONE;
}

static PyObject * Mex_flush(pyhive::Mex *self)
{
    // no-op
    Py_RETURN_NONE;
}

static PyObject * Mex_fileno(pyhive::Mex *self)
{
    if( int filehandle = getFilehandle(self) ) {
        return PyInt_FromLong(filehandle);
    } else {
        Py_RETURN_NONE;
    }
}

static idx newline_length(const char * buf, idx len, idx pos)
{
    if( pos < len ) {
        char c = buf[pos];
        if( c == '\n' ) {
            return 1;
        } else if( c == '\r' ) {
            if( pos + 1 < len && buf[pos + 1] == '\n' ) {
                return 2;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

static void expand_for_newline(sStr & s, sMex::Pos & rec)
{
    if( rec.pos == sIdxMax ) {
        return;
    }
    rec.size += newline_length(s.ptr(), s.length(), rec.pos + rec.size);
}

static PyObject * Mex_iter(PyObject * self)
{
    // we are our own iterator - simply return a copy of ourselves
    Py_INCREF(self);
    return self;
}

static PyObject * Mex_next(pyhive::Mex *self)
{
    if( self->str.recNext(self->last_line, false, 0) ) {
        expand_for_newline(self->str, self->last_line);
        return PyString_FromStringAndSize(self->str.ptr(self->last_line.pos), self->last_line.size);
    } else {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }
}

static PyObject * Mex_read(pyhive::Mex *self, PyObject * args)
{
    idx size = -1;
    if( !PyArg_ParseTuple(args, "|L:pyhive.Mex.read", &size) ) {
        return NULL;
    }

    idx pos = self->last_line.pos == sIdxMax ? 0 : self->last_line.pos + self->last_line.size;
    if( size < 0 || size > self->str.length() - pos ) {
        size = self->str.length() - pos;
    }
    if( pos >= self->str.length() ) {
        size = 0;
    }
    self->last_line.pos = pos;
    self->last_line.size = 0;
    return size ? PyString_FromStringAndSize(self->str.ptr(pos), size) : PyString_FromString("");
}

static PyObject * Mex_readline(pyhive::Mex *self, PyObject * args)
{
    idx size = -1;
    if( !PyArg_ParseTuple(args, "|L:pyhive.Mex.readline", &size) ) {
        return NULL;
    }

    if( self->str.recNext(self->last_line, false, 0) ) {
        expand_for_newline(self->str, self->last_line);
        if( size < 0 || size > self->last_line.size ) {
            size = self->last_line.size;
        }
        return PyString_FromStringAndSize(self->str.ptr(self->last_line.pos), size);
    } else {
        return PyString_FromString("");
    }
}

static PyObject * Mex_readlines(pyhive::Mex *self, PyObject * args)
{
    idx size = -1;
    if( !PyArg_ParseTuple(args, "|L:pyhive.Mex.readlines", &size) ) {
        return NULL;
    }

    idx start_pos = self->last_line.pos == sIdxMax ? 0 : self->last_line.pos + self->last_line.size;

    while( self->str.recNext(self->last_line, false, 0) ) {
        expand_for_newline(self->str, self->last_line);
        if( size >= 0 && start_pos + size >= self->last_line.pos + self->last_line.size ) {
            break;
        }
    }

    if( start_pos < self->str.length() ) {
        return PyString_FromStringAndSize(self->str.ptr(start_pos), self->last_line.pos + self->last_line.size - start_pos);
    } else {
        return PyString_FromString("");
    }
}

static PyObject * Mex_seek(pyhive::Mex *self, PyObject * args)
{
    idx offset = 0;
    int whence = 0;
    if( !PyArg_ParseTuple(args, "L|i:pyhive.Mex.seek", &offset, &whence) ) {
        return NULL;
    }

    idx pos = self->last_line.pos == sIdxMax ? 0 : self->last_line.pos + self->last_line.size;
    switch(whence) {
        case 0:
            pos = offset;
            break;
        case 1:
            pos += offset;
            break;
        case 2:
            pos = self->str.length() + offset;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "'whence' must equal os.SEEK_SET, os.SEEK_CUR, or os.SEEK_END");
            return NULL;
    }

    if( pos <= 0 ) {
        self->last_line.pos = sIdxMax;
        self->last_line.size = 0;
    } else {
        self->last_line.pos = pos;
        self->last_line.size = 0;
    }

    Py_RETURN_NONE;
}

static PyObject * Mex_tell(pyhive::Mex *self)
{
    idx pos = self->last_line.pos == sIdxMax ? 0 : self->last_line.pos + self->last_line.size;
    return PyLong_FromLongLong(pos);
}

static PyObject * Mex_truncate(pyhive::Mex *self, PyObject * args)
{
    idx pos = self->last_line.pos == sIdxMax ? 0 : self->last_line.pos + self->last_line.size;
    if( !PyArg_ParseTuple(args, "|L:pyhive.Mex.truncate", &pos) ) {
        return NULL;
    }

    self->str.cut(sMax<idx>(0, pos));
    Py_RETURN_NONE;
}

static PyObject * Mex_write_buf(pyhive::Mex *self, const char * src, Py_ssize_t len)
{
    if( self->str.flags & sMex::fReadonly ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.Mex is in read-only mode");
        return NULL;
    }
    if( len ) {
        idx pos = self->last_line.pos == sIdxMax ? 0 : self->last_line.pos + self->last_line.size;
        idx del_len = 0;
        if( pos < self->str.length() ) {
            del_len = sMin<idx>(len, self->str.length() - pos);
        }
        if( ((sMex&)self->str).replace(pos, src, len, del_len) == sNotIdx ) {
            PyErr_SetString(PyExc_IOError, "pyhive.Mex write failed");
            return NULL;
        }
        self->last_line.pos = pos + len;
        self->last_line.size = 0;
    }
    Py_RETURN_NONE;
}

static PyObject * Mex_write(pyhive::Mex *self, PyObject * args)
{
    const char * src = 0;
    Py_ssize_t len = 0;
    if( !PyArg_ParseTuple(args, "s#:pyhive.Mex.write", &src, &len) ) {
        return NULL;
    }
    return Mex_write_buf(self, src, len);
}

static PyObject * Mex_writelines(pyhive::Mex *self, PyObject * args)
{
    PyObject * seq = 0;
    if( !PyArg_ParseTuple(args, "O:pyhive.Mex.writelines", &seq) ) {
        return NULL;
    }
    if( !PySequence_Check(seq) ) {
        PyErr_SetString(PyExc_TypeError, "sequence of lines expected");
        return NULL;
    }
    idx num = PySequence_Size(seq);
    for(idx i=0; i<num; i++) {
        PyObject * str_obj = PySequence_GetItem(seq, i);
        if( !str_obj || !PyString_Check(str_obj) ) {
            PyErr_SetString(PyExc_TypeError, "sequence of lines expected");
            return NULL;
        }
        const char * src = PyString_AsString(str_obj);
        if( !src || !Mex_write_buf(self, src, PyString_Size(str_obj)) ) {
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

Py_ssize_t Mex_getreadbuffer(pyhive::Mex * self, Py_ssize_t seg, void ** ptrptr)
{
    if( ptrptr ) {
        *ptrptr = self->str.ptr();
    }
    return self->str.length();
}

Py_ssize_t Mex_getwritebuffer(pyhive::Mex * self, Py_ssize_t segment, void **ptrptr)
{
    if( self->str.flags & sMex::fReadonly ) {
        PyErr_SetString(PyExc_TypeError, "pyhive.Mex is in read-only mode");
        return -1;
    }
    if( ptrptr ) {
        *ptrptr = self->str.ptr();
    }
    return self->str.length();
}

Py_ssize_t Mex_getsegcount(pyhive::Mex * self, Py_ssize_t *lenp)
{
    if( lenp ) {
        *lenp = self->str.length();
    }
    return 1;
}

static PyObject * Mex_is_readonly(pyhive::Mex * self, void * closure)
{
    if( self->str.flags & sMex::fReadonly ) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject * Mex_flags(pyhive::Mex * self, void * closure)
{
    static const idx flagmask = 0xFFFF; // upper bits used for file handle - don't expose it to python!
    return pyhive::idx2py(self->str.flags & flagmask);
}

static PySequenceMethods Mex_seqmethods = {
    (lenfunc)Mex_length, // sq_length
    Mex_concat, // sq_concat
    0, // sq_repeat
    (ssizeargfunc)Mex_getitem, // sq_item
    (ssizessizeargfunc)Mex_getslice, // sq_slice
    (ssizeobjargproc)Mex_setitem, // sq_ass_item
    0, // sq_ass_slice
    0, // sq_contains
    Mex_inplaceconcat, // sq_inplace_concat
    0 // sq_inplace_repeat
};

static PyMethodDef Mex_methods[] = {
    { "close", (PyCFunction)Mex_close, METH_NOARGS,
      "Close the mapped file; see `file.close`" },
    { "flush", (PyCFunction)Mex_flush, METH_NOARGS,
      "No-op; included for Python file-like API completeness; see `file.flush`" },
    { "fileno", (PyCFunction)Mex_fileno, METH_NOARGS,
      "File handle; see `file.fileno`" },
    { "next", (PyCFunction)Mex_next, METH_NOARGS,
      "Return next line; see `file.next`" },
    { "read", (PyCFunction)Mex_read, METH_VARARGS,
      "read([size])\nRead some bytes; see `file.read`" },
    { "readline", (PyCFunction)Mex_readline, METH_VARARGS,
      "readline([size])\nRead next line; see `file.readline`" },
    { "readlines", (PyCFunction)Mex_readlines, METH_VARARGS,
      "readlines([sizehint])\nRead next lines; see `file.readlines`" },
    { "seek", (PyCFunction)Mex_seek, METH_VARARGS,
      "seek(offset, [whence])\nSeek to offset; see `file.seek`" },
    { "tell", (PyCFunction)Mex_tell, METH_NOARGS,
      "Return current offset; see `file.tell`" },
    { "truncate", (PyCFunction)Mex_truncate, METH_VARARGS,
      "truncate([size])\nTruncate to offset; see `file.truncate`" },
    { "write", (PyCFunction)Mex_write, METH_VARARGS,
      "write(str)\nWrite a string to current offset; see `file.write`" },
    { "writelines", (PyCFunction)Mex_writelines, METH_VARARGS,
      "writelines(sequence)\nWrite a sequence of strings at current offset; see `file.writelines`" },
    { NULL }
};

static PyBufferProcs Mex_as_buffer = {
    (readbufferproc)Mex_getreadbuffer, // bf_getreadbuffer;
    (writebufferproc)Mex_getwritebuffer, // bf_getwritebuffer;
    (segcountproc)Mex_getsegcount, // bf_getsegcount;
    0, // bf_getcharbuffer;
    0, // bf_getbuffer;
    0  // bf_releasebuffer;
};

static PyGetSetDef Mex_getsetters[] = {
    { (char*)"is_readonly", (getter)Mex_is_readonly, NULL, (char*)"Whether buffer is in read-only mode", NULL },
    { (char*)"flags", (getter)Mex_flags, NULL, (char*)"Bitwise-or of `pyhive.mex_flag` constants", NULL },
    { NULL }
};

static PyTypeObject MexType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "pyhive.Mex",              // tp_name
    sizeof(pyhive::Mex),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)Mex_dealloc,   // tp_dealloc
    (printfunc)Mex_print,      // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    0,                         // tp_repr
    0,                         // tp_as_number
    &Mex_seqmethods,           // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    (reprfunc)Mex_str,         // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    &Mex_as_buffer,            // tp_as_buffer
    Py_TPFLAGS_DEFAULT,        // tp_flags
    "HIVE extensible memory buffer or mem-mapped file. Implements Python's string API (minimally; no encoding etc.), :ref:`file-like API <python:bltin-file-objects>`, and buffer API.", // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    Mex_iter,                  // tp_iter
    (iternextfunc)Mex_next,    // tp_iternext
    Mex_methods,               // tp_methods
    0,                         // tp_members
    Mex_getsetters,            // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)Mex_init,        // tp_init
    0,                         // tp_alloc
    Mex_new,                   // tp_new
};

//static
bool pyhive::Mex::typeinit(PyObject * mod)
{
    if( PyType_Ready(&MexType) < 0 ) {
        return false;
    }
    Py_INCREF(&MexType);
    PyModule_AddObject(mod, "Mex", (PyObject*)&MexType);

    PyObject * mex_flag_mod = Py_InitModule3("pyhive.mex_flag", NULL, "pyhive.Mex flags\n\n"\
        ".. py:data:: DEFAULT\n\n    = `pyhive.mex_flag.BLOCK_DOUBLING`\n\n"\
        ".. py:data:: ALIGN_INTEGER\n\n    Align to 8 bytes (on 64-bit platforms)\n\n"\
        ".. py:data:: ALIGN_PARAGRAPH\n\n    Align to 16 byes\n\n"\
        ".. py:data:: ALIGN_PAGE\n\n    Align to page size\n\n"\
        ".. py:data:: BLOCK_NORMAL\n\n    Grow by 1 KB\n\n"\
        ".. py:data:: BLOCK_COMPACT\n\n    Grow by 128 bytes\n\n"\
        ".. py:data:: BLOCK_PAGE\n\n    Grow by page size\n\n"\
        ".. py:data:: BLOCK_DOUBLING\n\n    Grow by doubling existing allocation\n\n"\
        ".. py:data:: SET_ZERO\n\n    Fill allocated memory with zeros\n\n"\
        ".. py:data:: NO_REALLOC\n\n    Single allocation, no extension allowed\n\n"\
        ".. py:data:: EXACT_SIZE\n\n    Do not allocate more memory than minimum required\n\n"\
        ".. py:data:: READONLY\n\n    Readonly mode\n\n"\
        ".. py:data:: MAP_REMOVE_FILE\n\n    If file already exists on disk, remove it\n\n"\
        ".. py:data:: MAP_PRELOAD_PAGES\n\n    Pre-load mapped file into memory immediately\n\n"\
        ".. py:data:: CREAT_EXCL\n\n    Open file handle in O_CREAT|O_EXCL POSIX mode\n\n"
    );
    Py_INCREF(mex_flag_mod);
    PyModule_AddObject(mod, "mex_flag", mex_flag_mod);

    PyModule_AddIntConstant(mex_flag_mod, "DEFAULT", sMex::fBlockDoubling);
    PyModule_AddIntConstant(mex_flag_mod, "ALIGN_INTEGER", sMex::fAlignInteger);
    PyModule_AddIntConstant(mex_flag_mod, "ALIGN_PARAGRAPH", sMex::fAlignParagraph);
    PyModule_AddIntConstant(mex_flag_mod, "ALIGN_PAGE", sMex::fAlignPage);
    PyModule_AddIntConstant(mex_flag_mod, "BLOCK_NORMAL", sMex::fBlockNormal);
    PyModule_AddIntConstant(mex_flag_mod, "BLOCK_COMPACT", sMex::fBlockCompact);
    PyModule_AddIntConstant(mex_flag_mod, "BLOCK_PAGE", sMex::fBlockPage);
    PyModule_AddIntConstant(mex_flag_mod, "BLOCK_DOUBLING", sMex::fBlockDoubling);
    PyModule_AddIntConstant(mex_flag_mod, "SET_ZERO", sMex::fSetZero);
    PyModule_AddIntConstant(mex_flag_mod, "NO_REALLOC", sMex::fNoRealloc);
    PyModule_AddIntConstant(mex_flag_mod, "EXACT_SIZE", sMex::fExactSize);
    PyModule_AddIntConstant(mex_flag_mod, "READONLY", sMex::fReadonly);
    PyModule_AddIntConstant(mex_flag_mod, "MAP_REMOVE_FILE", sMex::fMapRemoveFile);
    PyModule_AddIntConstant(mex_flag_mod, "MAP_PRELOAD_PAGES", sMex::fMapPreloadPages);
    PyModule_AddIntConstant(mex_flag_mod, "CREAT_EXCL", sMex::fCreatExcl);

    return true;
}

//static
pyhive::Mex * pyhive::Mex::check(PyObject * o)
{
    if( o && o->ob_type == &MexType ) {
        return (pyhive::Mex*)o;
    } else {
        return NULL;
    }
}

//static
pyhive::Mex * pyhive::Mex::create()
{
    pyhive::Mex * self = (pyhive::Mex*)Mex_new(&MexType, 0, 0);
    Py_XINCREF(self);
    return self;
}

bool pyhive::Mex::init(const char * flnm, idx flags)
{
    last_line.pos = sIdxMax;
    last_line.size = 0;
    str.destroy();
    if( flnm ) {
        str.init(flnm, flags);
        if( !str.ok() ) {
            PyErr_SetString(PyExc_OSError, "Failed to open file");
            return false;
        }
    } else {
        str.init(flags);
    }
    return true;
}
