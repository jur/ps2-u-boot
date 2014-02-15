#
# (C) Copyright 2014 Juergen Urban
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# Default optimization level for MIPS r5900
#
MIPSFLAGS := -march=r5900

PLATFORM_CPPFLAGS += $(MIPSFLAGS)
PLATFORM_CPPFLAGS += -mabi=32 -DCONFIG_32BIT
ifdef CONFIG_SYS_BIG_ENDIAN
PLATFORM_LDFLAGS  += -m elf32btsmip
else
PLATFORM_LDFLAGS  += -m elf32ltsmip
endif

CONFIG_STANDALONE_LOAD_ADDR ?= 0x80200000 -T mips.lds

USE_PRIVATE_LIBGCC = yes
