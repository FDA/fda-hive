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

PROGRAMS = dna-hexagon dna-hexagon-collector dna-hexagon-batcher

PROGRAMS_LIBS = -lviolib -ldmlib $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB) $(LIBZIP_LIB)

dna-hexagon_SRCS = dna-hexagon.cpp
dna-hexagon-collector_SRCS = dna-hexagon-collector.cpp
dna-hexagon-batcher_SRCS = dna-hexagon-batcher.cpp

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PNOHY: dist
dist::
	$(call dist-service-register,dna-hexagon,HIVE-hexagon Alignment,/Sequencing/)
	$(call dist-service-register,dna-hexagon-collector,HIVE Alignment Collector,/Sequencing/)
	$(call dist-service-register,dna-hexagon-batcher,HIVE Alignment Batch-Mode,/Sequencing/)
	$(call dist-app,dna-hexagon,dna-hexagon)
	$(call dist-app,dna-hexagon-collector,dna-hexagon-collector)
	
	
	
#	$(call dist-service-register,dna-hexagon~test,Test HIVE-hexagon Alignment,/Test/Sequencing/)
	
