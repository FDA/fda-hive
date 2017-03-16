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
default: std

TOP ?= $(shell ../build/abspath.sh ..)
MODULE = hive

INT_LIBS = \

EXT_LIBS = \
	libsra

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS)) \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# tag
#
tag: \
	$(addsuffix _tag,$(ALL_LIBS))

.PHONY: tag $(addsuffix _tag,$(ALL_LIBS))


#-------------------------------------------------------------------------------
# libsra
#
$(LIBDIR)/libsra: $(addprefix $(LIBDIR)/libsra.,$(LIBEXT))

LIBSRA_SRC = reader libsra

LIBSRA_OBJ = \
	$(addsuffix .$(LOBX),$(LIBSRA_SRC))

# use fastq-dump lib list, exclude standard
LIBSRA_LIB = \
	-lsradb \
	-lsrareader \
	-lsraxf \
	-lalign-reader \
	-laxf \
	-lvxf \
	-lvdb \
	-lkdb \
	-lvfs \
	-lkrypto \
	-lkfg \
	-lkfs \
	-lksrch \
	-lkproc \
	-lklib \
	-lz \
	-lbz2

LIBDIR_STAT = $(subst /dyn/,/stat/,$(LIBDIR))
ILIBDIR_STAT = $(subst /dyn/,/stat/,$(ILIBDIR))
MODLIB_STAT = $(subst /dyn/,/stat/,$(MODDIR))
LIBSRA_V = $(shell cat $(SRCDIR)/libsra.vers)
LIBSRA_VM = $(word 1,$(subst ., ,$(LIBSRA_V)))

$(LIBDIR)/libsra.$(SHLX): libsra.vers.h $(LIBSRA_OBJ)
	gcc -shared -o $@.$(LIBSRA_V) -Wl,-soname,libsra.$(SHLX).$(LIBSRA_VM) -m64 $(LIBSRA_OBJ) -L$(LIBDIR_STAT) -L$(ILIBDIR_STAT) -L$(MODDIR_STAT) -Wl,-Bstatic -Wl,-whole-archive $(LIBSRA_LIB) -Wl,-no-whole-archive -Wl,-Bdynamic -lm -ldl -lpthread
	@ln -fs libsra.$(SHLX).$(LIBSRA_V) $@.$(LIBSRA_VM)
	@ln -fs libsra.$(SHLX).$(LIBSRA_VM) $@

$(LIBDIR)/libsra.$(LIBX): libsra.vers.h $(LIBSRA_OBJ)

libsra_tag:
	@ $(TOP)/build/tag-module.sh $(MODULE) libsra $(LIBSRA_OBJ)
