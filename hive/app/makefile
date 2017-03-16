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

ifeq ($(devdir),)
    devdir = $(abspath ../..)
endif

include $(devdir)/build/makefile.meta

DIST_LIST = \
    dna-cgi dmDownloader \
    dna-alignment-comparator dna-tnseq \
    dna-hexagon dna-heptagon \
    dna-alignx tblqryX4 \
    dmArchiver dmAnnot \
    dna-align-parser dna-parser dna-screening \
    dna-profx dna-qc dna-clust algo-annotMapper \
    dna-hiveseq \
    svc-batcher generic-launcher dna-multi-qc \
    dna-insilico \
    dna-differential-profiler

# MOVE!! your app from BUILD_LIST to DIST_LIST when it is ready to be deployed, DO NOT COPY!
BUILD_LIST = \
    $(DIST_LIST)

.PHONY: all clean
all clean::
	$(call make-list,$@,$(BUILD_LIST))

.PHONY: dist
dist::
	$(call make-list,$@,$(DIST_LIST))
