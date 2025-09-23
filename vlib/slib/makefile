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

LIBRARIES += slib

slib_SRCS =  core/core.cpp core/mex.cpp core/algo.cpp core/hash.cpp core/hash2.cpp core/sIO.cpp core/str.cpp core/olst.cpp core/word.cpp \
	core/id.cpp core/tim.cpp core/rc.cpp \
	std/app.cpp std/cmdline.cpp std/cryptohash.cpp std/dll.cpp std/gate.cpp \
	std/file.cpp std/filedir.cpp std/filepath.cpp std/filepipe.cpp std/filepipe2.cpp \
	std/filesorted.cpp std/fileps.cpp std/http.cpp std/online_connection.cpp \
	std/online_protocols.cpp std/regexp.cpp std/string.cpp std/stringparse.cpp \
	std/stringscanf.cpp std/stringxprintscanf.cpp std/stringsearch.cpp \
	std/stringstyle.cpp std/stringcode.cpp std/stringdatetime.cpp std/clr.cpp \
	std/varset.cpp std/dbg.cpp std/cgi.cpp std/crypt.cpp std/url.cpp \
	std/sha1.cpp std/sha256.cpp std/storage.cpp std/variant.cpp std/cryptocipher.cpp \
	utils/heatmap.cpp utils/txtcateg.cpp utils/txtformat.cpp utils/tbl.cpp \
	utils/tbltbl.cpp utils/sort.cpp utils/sVioDB.cpp \
	utils/intervalTree.cpp utils/tree.cpp utils/som.cpp \
	utils/vtree.cpp utils/basicCollapse.cpp \
	utils/json/json-lexer.cpp utils/json/parser.cpp utils/json/printer.cpp \
	utils/multipart_parser.cpp utils/cron.cpp \
	lang/calc.cpp lang/lang.cpp lang/lang_stdlib.cpp

std/regexp.cpp_CXXFLAGS = $(if $(HAS_PCRE),$(PCRE_CFLAGS))

mbedtls_CXXFLAGS = -Istd -Istd/mbedtls
ifneq ($(shell $(CXX) -v 2>&1 | grep 'Target: x86_64'),)
    mbedtls_CXXFLAGS += -DMBEDTLS_AESNI_C -DMBEDTLS_HAVE_X86_64
    std/mbedtls/aesni.c_CXXFLAGS = $(mbedtls_CXXFLAGS)
    slib_SRCS += std/mbedtls/aesni.c
endif
std/mbedtls/aes.c_CXXFLAGS = $(mbedtls_CXXFLAGS)
slib_SRCS += std/mbedtls/aes.c

ifeq ($(devdir),)
    devdir = $(abspath ../..)
endif
include $(devdir)/build/makefile.meta
