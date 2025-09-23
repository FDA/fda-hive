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
    if( !PyArg_ParseTupleAndKeywords(args, kwds, "|s#sl:pyhive.Mex.__init__", (char**)kwlist, &src, &src_len, &flnm, &flg) ) {
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
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static inline int getFilehandle(pyhive::Mex *self)
{
    return (int)((self->str.flags>>(sMex_FileHandleShift)) & 0xFFFF);
}


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
    return PyUnicode_FromStringAndSize(self->str.ptr(i), 1);
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
    if( i < 0 || i > self->str.length()) {
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
        return PyUnicode_FromStringAndSize(self->str.ptr(start), len);
    } else {
        return PyUnicode_FromString("");
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


static PyObject * Mex_str(pyhive::Mex *self)
{
    if( self->str.length() ) {
        return PyUnicode_FromStringAndSize(self->str.ptr(), self->str.length());
    } else {
        return PyUnicode_FromString("");
    }
}


static PyObject * Mex_close(pyhive::Mex *self)
{
    self->str.destroy();
    Py_RETURN_NONE;
}

static PyObject * Mex_flush(pyhive::Mex *self)
{
    Py_RETURN_NONE;
}

static PyObject * Mex_fileno(pyhive::Mex *self)
{
    if( int filehandle = getFilehandle(self) ) {
        return PyLong_FromLong(filehandle);
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
    Py_INCREF(self);
    return self;
}

static PyObject * Mex_next(pyhive::Mex *self)
{
    if( self->str.recNext(self->last_line, false, 0) ) {
        expand_for_newline(self->str, self->last_line);
        return PyUnicode_FromStringAndSize(self->str.ptr(self->last_line.pos), self->last_line.size);
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
    return size ? PyUnicode_FromStringAndSize(self->str.ptr(pos), size) : PyUnicode_FromString("");
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
        return PyUnicode_FromStringAndSize(self->str.ptr(self->last_line.pos), size);
    } else {
        return PyUnicode_FromString("");
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
        return PyUnicode_FromStringAndSize(self->str.ptr(start_pos), self->last_line.pos + self->last_line.size - start_pos);
    } else {
        return PyUnicode_FromString("");
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
        if( !str_obj || !PyUnicode_Check(str_obj) ) {
            PyErr_SetString(PyExc_TypeError, "sequence of lines expected");
            return NULL;
        }
        const char * src = PyUnicode_AsUTF8(str_obj);
        if( !src || !Mex_write_buf(self, src, PyUnicode_GET_LENGTH(str_obj)) ) {
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
    static const idx flagmask = 0xFFFF;
    return pyhive::idx2py(self->str.flags & flagmask);
}

static PySequenceMethods Mex_seqmethods = {
    (lenfunc)Mex_length,
    Mex_concat,
    0,
    (ssizeargfunc)Mex_getitem,
    0,
    (ssizeobjargproc)Mex_setitem,
    0,
    0,
    Mex_inplaceconcat,
    0
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

static int Mex_getbuffer(pyhive::Mex * self, Py_buffer * view, int flags)
{
    return PyBuffer_FillInfo(view, (PyObject *)self, self->str.ptr(), self->str.length(), self->str.flags & sMex::fReadonly, flags);
}

static PyBufferProcs Mex_as_buffer = {
    (getbufferproc)Mex_getbuffer,
    0
};

static PyGetSetDef Mex_getsetters[] = {
    { (char*)"is_readonly", (getter)Mex_is_readonly, NULL, (char*)"Whether buffer is in read-only mode", NULL },
    { (char*)"flags", (getter)Mex_flags, NULL, (char*)"Bitwise-or of `pyhive.mex_flag` constants", NULL },
    { NULL }
};

static PyTypeObject MexType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyhive.Mex",
    sizeof(pyhive::Mex),
    0,
    (destructor)Mex_dealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    &Mex_seqmethods,
    0,
    0,
    0,
    (reprfunc)Mex_str,
    0,
    0,
    &Mex_as_buffer,
    Py_TPFLAGS_DEFAULT,
    "HIVE extensible memory buffer or mem-mapped file. Implements Python's string API (minimally; no encoding etc.), :ref:`file-like API <python:bltin-file-objects>`, and buffer API.",
    0,
    0,
    0,
    0,
    Mex_iter,
    (iternextfunc)Mex_next,
    Mex_methods,
    0,
    Mex_getsetters,
    0,
    0,
    0,
    0,
    0,
    (initproc)Mex_init,
    0,
    Mex_new,
};

bool pyhive::Mex::typeinit(PyObject * mod)
{
    if( PyType_Ready(&MexType) < 0 ) {
        return false;
    }
    Py_INCREF(&MexType);
    PyModule_AddObject(mod, "Mex", (PyObject*)&MexType);

    static struct PyModuleDef mex_flag_mod_def = {
        PyModuleDef_HEAD_INIT,
        "pyhive.mex_flag",
        "pyhive.Mex flags\n\n"\
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
                ".. py:data:: CREAT_EXCL\n\n    Open file handle in O_CREAT|O_EXCL POSIX mode\n\n",
        0
    };
    PyObject * mex_flag_mod = PyModule_Create(&mex_flag_mod_def);

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

pyhive::Mex * pyhive::Mex::check(PyObject * o)
{
    if( o && o->ob_type == &MexType ) {
        return (pyhive::Mex*)o;
    } else {
        return NULL;
    }
}

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
