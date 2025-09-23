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

LIBRARIES += qlib
PROGRAMS += qapp qsrv qctl qhunt hive.cgi

OBJECT_TYPES=qphost qpsvc \
             svc-computations-base svc-data-loading-base svc-http-post

qlib_SRCS = QPrideBase.cpp QPride.cpp \
	QPrideDB.cpp QPrideCmdline.cpp \
	QPrideProc.cpp QPrideClient.cpp QPrideCGI.cpp \
	QPrideUtils.cpp \
	QPrideConnection.cpp QPrideSrv.cpp

CXXFLAGS=$(HIVE_DB_CXXFLAGS)

hive.cgi_SRCS = hivecgi.cpp
hive.cgi_LIBS = $(VLIB_LIB)
hive.cgi_XLIBS = $(MYSQL_LIB_STATIC)

qapp_SRCS = QPrideApp.cpp
qapp_LIBS = $(VLIB_LIB)
qapp_XLIBS = $(MYSQL_LIB_STATIC)

qctl_SRCS = QPrideCtl.cpp
qctl_LIBS = $(VLIB_LIB)
qctl_XLIBS = $(MYSQL_LIB)

qhunt_SRCS = QPrideHunt.cpp
qhunt_LIBS = $(VLIB_LIB)
qhunt_XLIBS = $(MYSQL_LIB)

qsrv_SRCS = usrv.cpp
qsrv_LIBS = $(VLIB_LIB)
qsrv_XLIBS = $(MYSQL_LIB)

ifeq ($(devdir),)
    devdir = $(abspath ../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist::
	$(call dist-service-register,qm,Queue Manager,/System/, \
            maxJobs=1 \
            sleepTime=3000 \
            maxLoops=1000 \
            knockoutSec=0 \
            noGrabDisconnect=0 \
            noGrabExit=0 \
            cmdLine='qsrv.os\$$(os)' \
            isUp=1 \
        )
	$(call dist-service-register,test,test service,/Test/System/, \
            isUp=0 \
            parallelJobs=1 \
            cmdLine='qhunt.os\$$(os)' \
        )
	$(call dist-service-register,human-interaction,Human Interaction,/System/, \
            isUp=0 \
            maxJobs=1 \
        )
	$(call dist-app,qm,qapp,qapp.os$(SLIB_PLATFORM))
	$(call dist-app,qm,qsrv,qsrv.os$(SLIB_PLATFORM))
	$(call dist-src,qm,qpstart.sh.os$(SLIB_PLATFORM),qpstart.sh.os$(SLIB_PLATFORM))
	$(call dist-src,qm,qlang.sh.os$(SLIB_PLATFORM),qlang.sh.os$(SLIB_PLATFORM))
	$(call dist-src,qm,qm_maintain.sh.os$(SLIB_PLATFORM),qm_maintain.sh.os$(SLIB_PLATFORM))
	$(call dist-src,qm,qputils.sh.os$(SLIB_PLATFORM),qputils.sh.os$(SLIB_PLATFORM))
	$(call dist-src,qm,'~/.hive_bash_rc',hive_bash_rc)
	$(call dist-src,qm,xlsx2csv.py,xlsx2csv.py)
	$(call dist-qry,qm,mico2-os$(SLIB_PLATFORM)/,alloftype('tool_pkg',{name:'MiCo2'}))
	$(call dist-qry,qm,mico3-os$(SLIB_PLATFORM)/,alloftype('tool_pkg',{name:'MiCo3'}))

