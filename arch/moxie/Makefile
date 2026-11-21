# SPDX-License-Identifier: GPL-2.0
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# Derived from arch/openrisc/Makefile.

KBUILD_DEFCONFIG := mox125_sim_defconfig

OBJCOPYFLAGS    := -O binary -R .note -R .comment -S
LIBGCC 		:= $(shell $(CC) $(KBUILD_CFLAGS) -print-libgcc-file-name)

KBUILD_CFLAGS	+= -pipe -D__linux__

libs-y		+= $(LIBGCC)

all: vmlinux
