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

PROGRAMS += download-processor

download-processor_LIBS = -lqpsvc -lviolib $(VLIB_LIB)
download-processor_XLIBS = $(MYSQL_LIB)


download-processor_SRCS = download-proc.cpp

OBJECT_TYPES = \
    svc-download-base \
    svc-download-dropbox \
    svc-download-tp \
    svc-download-http \
    svc-download-ftp \
    svc-download-ncbi-genbank \
    svc-download-ncbi-genome \
    svc-download-ncbi-exome \
    svc-pipeline-download-base \
    svc-pipeline-download-tp \
    svc-pipeline-download-http \
    svc-pipeline-download-ftp \
    svc-pipeline-download-dropbox \
    svc-pipeline-download-ncbi-genbank \
    svc-pipeline-download-ncbi-genome \
    svc-pipeline-download-ncbi-exome 

APPS = HTTP-Downloader:1 FTP-Downloader:1 HIVE_Network_Downloader:1 NCBI_GenBank_Downloader:1

MENUITEMS = \
    /Portal/Downloaders/HTTP_Downloader \
    /Portal/Downloaders/FTP_Downloader \
    /Portal/Downloaders/HIVE_Network_Downloader \
    /Portal/Downloaders/NCBI_GenBank_Downloader 
   

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist:: dist_download-processor

dist_download-processor:
	$(call dist-service-register,download-processor,HIVE download processor service,/download-processor/)
	$(call dist-app,download-processor,download-processor)



