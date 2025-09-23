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

PROGRAMS = dmDownloader.cgi dmDownloaderproc

OBJECT_TYPES=svc-download

dmDownloader.cgi_LIBS = -lqpsvc $(VLIB_LIB)
dmDownloader.cgi_XLIBS = $(MYSQL_LIB_STATIC)

dmDownloaderproc_LIBS = -lqpsvc $(VLIB_LIB)
dmDownloaderproc_XLIBS = $(MYSQL_LIB)

dmDownloader.cgi_SRCS = dmDownloadercgi.cpp
dmDownloaderproc_SRCS = dmDownloaderproc.cpp

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

.PHONY: dist
dist::
	$(call copy-bin,dmDownloader.cgi)
	$(call dist-service-register,dmDownloader,Downloader Engine,/Utils/, \
        maxJobs=4 \
    )
	$(call dist-app,dmDownloader,dmDownloaderproc)
