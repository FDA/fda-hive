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

PROGRAMS += dmArchiverproc dmCompressor
PROGRAMS_LIBS = -lqpsvc $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB)

OBJECT_TYPES= svc-archiver svc-compressor \
    table csv-table tsv-table excel-file \
    image system-image multimedia \
    algorithm-script \
    viodb u-hiveseq u-annot prot-seq \
    u-ionExpress u-idList target_library

ACTIONS = TreatAsAuto TreatAsbz2 TreatAstbz2 TreatAsgz TreatAstgz TreatAstar TreatAszip \
          TreatAsbam convert2protein upload export_hivepack exportEx \
		          
dmArchiverproc_SRCS = dmArchiverproc.cpp
dmArchiverproc_XLIBS = $(IMAGEMAGICK_LIB) $(LIBXLS_LIB)
dmCompressor_SRCS = dmCompressorproc.cpp

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

.PHONY: dist
dist::
	$(call dist-service-register,dmArchiver,Data Archivation Engine,/Data Mining/)
	$(call dist-service-register,dmCompressor,Data Compression Engine,/Data Mining/)
	$(call dist-service-set-config,dmCompressor, \
        selfExpirationDays='30' \
    )
	$(call dist-app,dmArchiver,dmArchiverproc)
	$(call dist-adoption,dmArchiver,fastq-dump,fastq-dump)
	$(call dist-adoption,dmArchiver,sra_prefetch,prefetch)
	$(call dist-adoption,dmArchiver,samtools,samtools)
	$(call dist-src,dmArchiver,sra2fastq.os$(SLIB_PLATFORM),sra2fastq.sh)
	$(call dist-app,dmCompressor,dmCompressor)
