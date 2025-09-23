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
    algo-annotMapper dna-alignmulti dna-clust dna-hiveseq dna-qc svc-batcher \
    dmAnnot dna-align-parser dna-denovoassembly dna-insilico dna-screening tblqryX4 \
    dmArchiver dna-alignQC dna-differential-profiler dna-multi-qc dna-tnseq ViralMutationComparator \
    dmDownloader dna-alignx dna-heptagon dna-parser ion \
    dna-alignment-comparator dna-cgi dna-hexagon dna-profx

BUILD_LIST = \
    $(DIST_LIST) \
    viocgi \
    dna-denovoassembly \
    dna-fingerprint

.PHONY: sync
sync::
	$(call make-list,$@,$(BUILD_LIST))

.PHONY: dist
dist::
	$(call make-list,$@,$(DIST_LIST))


