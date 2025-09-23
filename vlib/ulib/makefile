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

LIBRARIES += ulib

CXXFLAGS=$(HIVE_DB_CXXFLAGS)

OBJECT_TYPES = type domain-id-descr \
               base base_user_type base_system_type \
               group user file u-file sysfile \
               webpage u-usage user-settings user-info \
               view action menuitem js_component \
               email notification process \
               directory folder sysfolder \
               special dropbox sysconfig project \
               tool_pkg \
               tool \
			   user_app_argument user_app_output user_app_base user_app \
			   container_image container_image_docker container_image_singularity

ACTIONS = create search detail edit export_json open \
          copy cut paste delete admin share rename subfolder download \
          cast2systemImage cast2multimedia procClone

MENUITEMS = /main /home /contact /help /login /register /logout /news /profile /profile/info /profile/settings /profile/usage \
			/about/main \
			/about/events \
			/about/tutorials \
			/about/publications \
			/about/people \
			/about


ulib_SRCS = usr.cpp utype2.cpp utype2_cache.cpp uobj.cpp uperm.cpp \
    ufile.cpp ucgi.cpp uemail.cpp uproc.cpp ufolder.cpp \
    uusage.cpp uprop.cpp uquery.cpp uhousekeeper.cpp upropset.cpp

ifeq ($(devdir),)
    devdir = $(abspath ../..)
endif
include $(devdir)/build/makefile.meta

.PHONY: dist domains_dist sysfolders_dist
dist:: domains_dist sysfolders_dist

domains_dist::
	$(call prop-import-json,domains.json,query://alloftype("^domain-id-descr$$",{"domain-id-descr_ascii":["type"]}))

sysfolders_dist::
	$(call prop-import-json,sysfolders.json)

.PHONY: sync domains
sync:: domains

domains::
	$(call prop-export-json-noids,domains.json,query://alloftype("^domain-id-descr$$",{"domain-id-descr_ascii":["type"]}))


.PHONY: change_log
change_log::
	$(call prop-import-object,change_log.json)
