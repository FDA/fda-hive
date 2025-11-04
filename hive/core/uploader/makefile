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

PROGRAMS += upload-processor upload.cgi

upload-processor_LIBS = -lqpsvc -lviolib $(VLIB_LIB)
upload-processor_XLIBS = $(MYSQL_LIB)

upload.cgi_XLIBS = $(MYSQL_LIB_STATIC)
upload.cgi_LIBS = -lviolib -lqpsvc $(VLIB_LIB)

upload-processor_SRCS = upload-proc.cpp

upload.cgi_SRCS = upload-cgi.cpp

OBJECT_TYPES = \
    svc-pipeline-upload \
    svc-upload-processor

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist:: dist_upload-processor

dist_upload-processor:
	$(call dist-service-register,upload-processor,HIVE upload processor service,/upload-processor/)
	$(call dist-app,upload-processor,upload-processor)
	$(call dist-service-set-config,upload-processor, \
        ResubmitSec='5' \
        IdleTimeoutSec='86400' \
        OnPauseResubmitSec='600' \
        OnPauseIdleTimeoutSec='432000' \
        concurrency='4' \
        chunkSize='10485760' \
        compress='false' \
        reverseFileRegex='\\\\\\\.(tar|zip)' \
        alignments='[\"sam\"\054\054 \"bam\"\054\054 \"blast_out\"]' \
        reference_genomes='[ \"fa\"\054\054 \"fa.gz\"\054\054 \"fas\"\054\054 \"fas.gz\"\054\054 \
                             \"fasta\"\054\054 \"fasta.gz\"\054\054 \"fsa\"\054\054 \"fsa.gz\"\054\054 \
                             \"gb\"\054\054 \"gb.gz\"\054\054 \"gbk\"\054\054 \"gbk.gz\" ]' \
    )
	$(call copy-bin,upload.cgi)



