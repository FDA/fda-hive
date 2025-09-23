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

createdb:
	@./db_create.sh $(HIVE_DB) $(HIVE_DB_USER) $(HIVE_DB_PWD) $(HIVE_DB_HOST) $(HIVE_DB_PORT)

initdb:
	@./db_init.sh $(HIVE_DB) $(HIVE_DB_USER) $(HIVE_DB_PWD) $(HIVE_DB_HOST) $(HIVE_DB_PORT)

sync::
	@echo "Reading database: -db $(HIVE_DB) -host $(HIVE_DB_HOST) -port $(HIVE_DB_PORT) -u $(HIVE_DB_USER) -p xxxx"
	@perl db2svn.pl -db $(HIVE_DB) -host $(HIVE_DB_HOST) -port $(HIVE_DB_PORT) -u $(HIVE_DB_USER) -p $(HIVE_DB_PWD)
	@if [ -f ~/work/SVN/git-set-keywords.pl ]; then \
	    ~/work/SVN/git-set-keywords.pl; \
	fi

chicken-egg:
	@$(call run-qapp,,,>"$(srcdir)/db_init_data_type.sql",-user qapp -db_init_data_type_sql);
	@$(call run-qapp,,,>"$(srcdir)/db_init_data_domain.sql",-user qapp -db_init_data_domain_sql);

.PHONY: dist sync
dist::
ifndef PACKAGING
ifdef INSTALLING
	@./db_init_sp.sh $(HIVE_DB) $(HIVE_DB_USER) $(HIVE_DB_PWD) $(HIVE_DB_HOST) $(HIVE_DB_PORT)
endif
endif
	$(call copy-src,*)
