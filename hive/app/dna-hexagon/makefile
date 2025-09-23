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

PROGRAMS = dna-hexagon dna-hexagon-collector

PROGRAMS_LIBS = -lviolib $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB)

OBJECT_TYPES = svc-align-hexagon

ACTIONS = hexagon_nuc_read hexagon_genome

dna-hexagon_SRCS = dna-hexagon.cpp
dna-hexagon-collector_SRCS = dna-hexagon-collector.cpp

APPS = Standard:1 Virus-specific:1 Bacteria-specific:1 Human-whole-genome-specific:1 Exome-specific:1 Transcriptome-specific:1 Distant-relative:1 \
HIVE-Hexagon_(Standard):2.1 \
HIVE-Hexagon_(Virus-specific):2.1 \
HIVE-Hexagon_(Bacteria-specific):2.1 \
HIVE-Hexagon_(Human-whole-genome-specific):2.1 \
HIVE-Hexagon_(Exome-specific):2.1 \
HIVE-Hexagon_(Transcriptome-specific):2.1 \
HIVE-Hexagon_(Distant-relative):2.1

MENUITEMS = \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon/Bacteria_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon/Different_Levels_of_Homology_discovery/Distant_relative \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon/Exome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon/Human_whole_genome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon/Standard \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon/Transcriptome_specific \
/Portal/Sequence_Alignment_on_Genome/DNA-seq/HIVE-Hexagon/Virus_specific \

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist:: algo_install
	$(call dist-service-register,dna-hexagon,HIVE-hexagon Alignment,/Sequencing/)
	$(call dist-service-register,dna-hexagon-collector,HIVE Alignment Collector,/Sequencing/)
	$(call dist-app,dna-hexagon,dna-hexagon)
	$(call dist-app,dna-hexagon-collector,dna-hexagon-collector)

.PHONY: algo_install
algo_install::
	$(call prop-import-object,algo-hexagon-2.0.json)
