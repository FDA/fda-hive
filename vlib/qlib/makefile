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
PROGRAMS += qapp qsrv qctl qhunt

qlib_SRCS = QPrideBase.cpp QPride.cpp \
	QPrideDB.cpp QPrideCmdline.cpp \
	QPrideProc.cpp QPrideClient.cpp QPrideCGI.cpp \
	QPrideUtils.cpp \
	QPrideConnection.cpp QPrideSrv.cpp QPrideGenericLauncher.cpp

# to privide default db connection information
CXXFLAGS=$(HIVE_DB_CXXFLAGS)

qapp_SRCS = QPrideApp.cpp
qapp_LIBS = $(VLIB_LIB)
qapp_XLIBS = $(MYSQL_LIB)

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

# config here goes before service since it he initaial service
PNOHY: dist
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
	$(call dist-service-register,human-interaction,Human Interaction,/System/, \
            isUp=0 \
            maxJobs=1 \
        )
	$(call dist-app,qm,qapp,qapp.os$(SLIB_PLATFORM))
	$(call dist-app,qm,qsrv,qsrv.os$(SLIB_PLATFORM))
	$(call dist-src,qm,qpstart.sh.os$(SLIB_PLATFORM),qpstart.sh.os$(SLIB_PLATFORM))
	$(call dist-src,qm,'~/.hive_bash_rc',hive_bash_rc)
