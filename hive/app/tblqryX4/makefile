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

PROGRAMS += tblqryx4

APPS = Table-Query:1

MENUITEMS = \
	/Portal/General_DNA_Filters_and_Tools/Table_Query

OBJECT_TYPES = tblqryx4 plugin_tblqry u-tqs

ACTIONS = jumpToTblqry cast2tqs tblQry

PROGRAMS_LIBS = -lviolib -lqpsvc $(VLIB_LIB)
PROGRAMS_XLIBS = $(MYSQL_LIB) $(IMAGEMAGICK_LIB) $(SQLITE3_LIB)

tblqryx4_SRCS = tblqryX4_cmdlist.cpp tblqryX4_cmd_rlda1.cpp \
	exec-context.cpp exec-context-files.cpp rowscols.cpp tblqryX4.cpp \
	tblqryX4_cmd_tree.cpp tblqryX4_cmd_stat.cpp \
	$(if $(HAS_IMAGEMAGICK),tblqryX4_cmd_heatmap.cpp) \
	tblqryX4_cmd_som.cpp tblqryX4_cmd_collapse.cpp \
	tblqryX4_cmd_compAnalysis.cpp tblqryX4_cmd_kaplanmeier.cpp \
	tblqryX4_cmd_dataTrending.cpp tblqryX4_cmd_dataCalib.cpp \
	$(if $(HAS_SQLITE3),tblqryX4_cmd_uusage.cpp) tblqryX4_cmd_bayesianStat.cpp \
	tblqryX4_cmd_pearsonCorr.cpp tblqryX4_cmd_cleaner.cpp \
	tblqryX4_cmd_extractTbl.cpp tblqryX4_cmd_fourier.cpp \
	tblqryX4_cmd_addmissingrows.cpp tblqryX4_cmd_load_SNPprofile.cpp \
	tblqryX4_cmd_simulation.cpp tblqryX4_cmd_foldChange.cpp tblqryX4_cmd_sdtmSummary.cpp \
	tblqryX4_cmd_generic.cpp tblqryX4_cmd_dictionaryTable.cpp \
	tblqryX4_cmd_variance.cpp tblqryX4_cmd_pca.cpp \
    tblqryX4_cmd_geneOntology.cpp tblqryX4_cmd_rankCollapser.cpp 

ifeq ($(devdir),)
    devdir = $(abspath ../../..)
endif
include $(devdir)/build/makefile.meta

PHONY: dist
dist::
	$(call dist-service-register,tblqryx4,Table Data Query Extended Engine v4,/General/, \
		maxJobs=8 \
		maxLoops=20 \
		sleepTime=10000 \
	)
	$(call dist-service-set-config,tblqryx, \
        tableRepository='@global_scratch_dir@tables/' \
    )
	$(call dist-service-set-config-path,tblqryx,tableRepository,30,/obj@objID@-/req@reqID@-/)
	$(call dist-app,tblqryx4,tblqryx4)
	$(call dist-src,tblqryx4,tblqry-generic-cmd-test.py,tblqry-generic-cmd-test.py)
	$(call prop-import-json,plugin_tblqry.json,query://alloftype("^plugin_tblqry$$"))

.PHONY: sync plugins
sync:: plugins

plugins::
	$(call prop-export-json-noids,plugin_tblqry.json,query://alloftype("^plugin_tblqry$$"))
