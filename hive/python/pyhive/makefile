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

OBJECT_TYPES = svc-test-pyhive

MODULES += py3hive
PROGRAMS += py3-modrun

py3hive_NAME = python3/pyhive
py3hive_SRCS = pyhiveid.cpp pymex.cpp pytime.cpp pyutype.cpp pyuobj.cpp pyproc.cpp qpysvc.cpp pyion.cpp pyionwander.cpp pyseq.cpp
py3-modrun_SRCS = py-modrun.cpp

CXXFLAGS+=$(shell pkg-config --cflags 'python-3.9')
LDFLAGS+=$(shell pkg-config --libs 'python-3.9')

py3hive_LIBS = -lviolib -lqpsvc $(VLIB_LIB)
py3hive_XLIBS = $(MYSQL_LIB)
py3-modrun_LIBS = $(VLIB_LIB)
py3-modrun_XLIBS = $(PCRE_LIB)

ifeq ($(devdir),)
	devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

doc: $(libdir)/$(py3hive_NAME).$(DLL_EXT)
	PYTHONPATH="${libdir}/python3:${PYTHONPATH}" sphinx-build -b html . "${devdir}"/hive/html/doc/pyhive/

PHONY: dist
dist::
	$(call dist-lib,qm,$(py3hive_NAME).$(DLL_EXT))
	$(call dist-app,qm,py3-modrun,py-modrun.os$(SLIB_PLATFORM))
	$(call dist-service-register,test_pyhive,PyHive Test,/Utils/, \
		capacity=1 \
	)
