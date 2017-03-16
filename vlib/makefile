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
    devdir = $(abspath ..)
endif

include $(devdir)/build/makefile.meta

DIST_LIST = qlib db xlib
BUILD_LIST = slib ssql ssci qlang ulib xlib ion qlib db
DLL_NAME = hive-vlib
DLL_DEP_LIST = slib ssql ssci qlang ulib xlib qlib ion
DLL_FILE = $(libdir)/lib$(DLL_NAME).so
VLIB_STAMP = $(libdir)/vlib.stamp

.PHONY: all makefile

all:: $(VLIB_STAMP) $(DLL_FILE)

$(VLIB_STAMP):: makefile
	$(call make-list,all,$(BUILD_LIST))
	@# Set VLIB_STAMP's mtime to greatest mtime of DLL_DEP_LIST's static libraries
	@touch -d "$$(stat -c %y $$(ls -t $(foreach tgt,$(DLL_DEP_LIST),$(libdir)/lib$(tgt).a) | head -1))" $(VLIB_STAMP)

$(DLL_FILE):: $(VLIB_STAMP)
	@echo "================ $(MAKE_MODE) - SHARED LIB - lib$(DLL_NAME).so"
	$(CXX) -pthread -shared -o $@ $(LDFLAGS) -Wl,--whole-archive $(foreach tgt,$(DLL_DEP_LIST),$(libdir)/lib$(tgt).a) -Wl,--no-whole-archive $(IMAGEMAGICK_LIB) $(LIBXLS_LIB) $(PCRE_LIB) $(TOP_LDFLAGS)

.PHONY: clean
clean::
	$(call make-list,$@,$(BUILD_LIST))
	rm -f $(DLL_FILE)
	rm -f $(VLIB_STAMP)

.PHONY: dist
dist::
	$(call make-list,$@,$(DIST_LIST))
