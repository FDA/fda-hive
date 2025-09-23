
ifeq ($(devdir),)
    devdir = $(abspath .)
endif

include $(devdir)/build/makefile.meta

BUILD_LIST = vlib hive
DOC_LIST = hive

.PHONY: dist sync
dist sync::
	$(call make-list,$@,$(BUILD_LIST))

.PHONY: doc
doc:
	doxygen
	$(call make-list,$@,$(DOC_LIST))
