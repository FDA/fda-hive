#/*
# *  ::718604!
# * 
# * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
# * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
# * Affiliation: Food and Drug Administration (1), George Washington University (2)
# * 
# * All rights Reserved.
# * 
# * The MIT License (MIT)
# * 
# * Permission is hereby granted, free of charge, to any person obtaining
# * a copy of this software and associated documentation files (the "Software"),
# * to deal in the Software without restriction, including without limitation
# * the rights to use, copy, modify, merge, publish, distribute, sublicense,
# * and/or sell copies of the Software, and to permit persons to whom the
# * Software is furnished to do so, subject to the following conditions:
# * 
# * The above copyright notice and this permission notice shall be included
# * in all copies or substantial portions of the Software.
# * 
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# * DEALINGS IN THE SOFTWARE.
# */

MODULES += pyhive
PROGRAMS += py-modrun

# subdir to allow future expansion: we may also want a python-3.x module called 'pyhive.so'
pyhive_NAME = python2.7/pyhive
pyhive_SRCS = pyhiveid.cpp pymex.cpp pytime.cpp pyutype.cpp pyuobj.cpp pyproc.cpp qpysvc.cpp pyion.cpp pyionwander.cpp pyseq.cpp
py-modrun_SRCS = py-modrun.cpp

# python API docs specifically ask to add $incldir/python2.7 to search path instead of #including <python2.7/Python.h>
# -fno-strict-aliasing needed because core parts of Python C API require aliasing (PyTypeObject* cast to PyObject* etc.)
CXXFLAGS=$(shell pkg-config python-2.7 --cflags) -fno-strict-aliasing

pyhive_LIBS = -lviolib -lqpsvc $(VLIB_LIB)
pyhive_XLIBS = $(MYSQL_LIB) $(shell pkg-config python-2.7 --libs)
py-modrun_LIBS = $(VLIB_LIB)
py-modrun_XLIBS = $(PCRE_LIB)

ifeq ($(devdir),)
	devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

doc: $(libdir)/$(pyhive_NAME).so
	PYTHONPATH="${libdir}/python2.7:${PYTHONPATH}" sphinx-build -b html . "${devdir}"/hive/html/doc/pyhive/

PHONY: dist
dist::
	$(call run-qapp,qm,$(pyhive_NAME).so,$(libdir)/$(pyhive_NAME).so)
	$(call dist-src,qm,py-modrun.sh.os$(SLIB_PLATFORM),py-modrun.sh.os$(SLIB_PLATFORM))
	$(call dist-app,qm,py-modrun,py-modrun.os$(SLIB_PLATFORM))
