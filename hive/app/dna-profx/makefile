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

PROGRAMS += dna-profx
PROGRAMS_LIBS = -lviolib $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB)

OBJECT_TYPES = svc-profx \
    svc-profx-samtools \
    svc-profx-cuffdiff

dna-profx_SRCS = dna-profx.cpp dna-profx-samtools.cpp dna-profx-cuffdiff.cpp dna-profx-varscan.cpp


ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist::
	$(call dist-service-register,dna-profx,HIVE-optimized External Profiling,/Sequencing/)
	$(call dist-app,dna-profx,dna-profx)
	$(call dist-src,dna-profx,dna-profx.sh.os$(SLIB_PLATFORM),dna-profx.sh)
	
	
	$(call dist-src,dna-profx,dna-profx-samtools.sh.os$(SLIB_PLATFORM),dna-profx-samtools.sh)
	$(call dist-adoption,dna-profx,samtools-0.1.18.os$(SLIB_PLATFORM).tar.gz,samtools-0.1.18.os$(SLIB_PLATFORM).tar.gz)
	$(call dist-adoption,dna-profx,\>samtools_unpack.os$(SLIB_PLATFORM),samtools_unpack)

	$(call dist-src,dna-profx,dna-profx-varscan.sh.os$(SLIB_PLATFORM),dna-profx-varscan.sh)

	$(call dist-src,dna-profx,dna-profx-cuffdiff.sh.os$(SLIB_PLATFORM),dna-profx-cuffdiff.sh)
	$(call dist-adoption,dna-profx,cufflinks-2.0.2.os$(SLIB_PLATFORM).tar.gz,cufflinks-2.0.2.os$(SLIB_PLATFORM).tar.gz)
	$(call dist-adoption,dna-profx,\>cufflinks_unpack.os$(SLIB_PLATFORM),cufflinks_unpack)
	
