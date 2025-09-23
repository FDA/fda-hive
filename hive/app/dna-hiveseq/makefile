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

PROGRAMS += dna-hiveseq

PROGRAMS_LIBS = -lviolib -ldna-denovo -lqpsvc $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB)

OBJECT_TYPES = svc-hiveseq

ACTIONS = hiveseq

dna-hiveseq_SRCS = dna-hiveseq.cpp

APPS = Contig-Extension:1 Hive-Seq:1 Sequence-Trimmer:1 Primers-Filter:1 Adapters-Filter:1 Quality-and-Length-Filter:1 Complexity-Filter:1 Identification-Filter:1 Sequence-Filter:1 Paired-End-Collapser:1 Simulated-Random-Sequence-Generation:1 Sequence-Combining-Tool:1

MENUITEMS = \
/Portal/General_DNA_Filters_and_Tools/Adapters_Filter \
/Portal/General_DNA_Filters_and_Tools/Complexity_Filter \
/Portal/General_DNA_Filters_and_Tools/Hive-Seq \
/Portal/General_DNA_Filters_and_Tools/Identification_Filter \
/Portal/General_DNA_Filters_and_Tools/Paired_End_Collapser \
/Portal/General_DNA_Filters_and_Tools/Primers_Filter \
/Portal/General_DNA_Filters_and_Tools/Quality_and_Length_Filter \
/Portal/General_DNA_Filters_and_Tools/Sequence_Combining_Tool \
/Portal/General_DNA_Filters_and_Tools/Sequence_Filter \
/Portal/General_DNA_Filters_and_Tools/Sequence_Trimmer \
/Portal/General_DNA_Filters_and_Tools/Simulated_Random_Sequence_Generation \
/Portal/Sequence_Assemblers/Contig_Extension \
/Portal/De_Novo_Sequence_Assemblers/Contig_Extension


ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist::
	$(call dist-service-register,dna-hiveseq,HIVE sequences manipulation,/Sequencing/)
	$(call dist-app,dna-hiveseq,dna-hiveseq)
	$(call dist-adoption,dna-hiveseq,velvet.2013_05_02.tar.gz,velvet.2013_05_02.tar.gz)
	$(call dist-adoption,dna-hiveseq,\>velvet_unpack.os$(SLIB_PLATFORM),velvet_unpack)
	$(call dist-src,dna-hiveseq,velvet.os$(SLIB_PLATFORM),velvet.sh)
	$(call dist-adoption,dna-hiveseq,oases_0.2.08.tar.gz,oases_0.2.08.tar.gz)
	$(call dist-adoption,dna-hiveseq,\>oases_unpack.os$(SLIB_PLATFORM),oases_unpack)
	$(call dist-src,dna-hiveseq,oases.os$(SLIB_PLATFORM),oases.sh)
	$(call dist-adoption,dna-hiveseq,abyss-1.3.5.tar.gz,abyss-1.3.5.tar.gz)
	$(call dist-adoption,dna-hiveseq,\>abyss_unpack.os$(SLIB_PLATFORM),abyss_unpack)
	$(call dist-src,dna-hiveseq,abyss.os$(SLIB_PLATFORM),abyss.sh)
