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

PROGRAMS += dna-insilico

PROGRAMS_LIBS = -lviolib $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB)

OBJECT_TYPES = svc-dna-insilico

USER_PROJ = insilico_readme_to_bin

dna-insilico_SRCS = dna-insilico.cpp

APPS = Insilico-sequence-generator:1

MENUITEMS = \
/Portal/General_DNA_Filters_and_Tools/Genome_Randomizer \
/Portal/General_DNA_Filters_and_Tools/Insilico_sequence_generator


ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist::
	$(call dist-service-register,dna-insilico,Next-Gen Genome Sequence Randomizer,/Sequencing/)
	$(call dist-app,dna-insilico,dna-insilico)

$(bindir)/hive_insilico_README.txt: README.txt
	cp -p README.txt $(bindir)/hive_insilico_README.txt

.PHONY: insilico_readme_to_bin clean_local
insilico_readme_to_bin:: $(bindir)/hive_insilico_README.txt

clean: clean_local
clean_local::
	rm -f $(bindir)/hive_insilico_README.txt

