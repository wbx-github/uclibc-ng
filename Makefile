# Makefile for uClibc
#
# Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

top_srcdir=./
top_builddir=$(if $(O),$(O),.)/
export top_builddir

# We do not need built-in implicit rules
MAKEFLAGS += -r
CONFIG_SHELL ?= /bin/sh
export CONFIG_SHELL

include $(top_srcdir)Makefile.in
include $(top_srcdir)Makerules
include $(top_srcdir)Makefile.help

# Like the kernel: an in-tree build leaves generated files in the source,
# and those win over the ones under $(O) ('#include "ucontext_i.h"' looks
# next to the including file first).
ifneq ($(O),)
ifneq ($(abspath $(O)),$(CURDIR))
ifeq ($(filter $(clean_targets),$(MAKECMDGOALS)),)
srctree-built := $(strip $(wildcard $(top_srcdir).config \
	$(top_srcdir)include/bits/uClibc_config.h) \
	$(call sym-generated,$(top_srcdir)))
ifneq ($(srctree-built),)
$(error $(CURDIR) was built in tree ($(firstword $(srctree-built))), run "make distclean" there before building with O=)
endif
endif
endif
endif
