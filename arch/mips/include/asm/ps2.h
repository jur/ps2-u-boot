/*
 * Copyright (C) 2014 Juergen Urban
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MIPS_ASM_PS2_H
#define _MIPS_ASM_PS2_H

#ifndef __ASSEMBLY__
#include <compiler.h>

uint32_t ps2sif_allociopheap(int size);
int ps2sif_freeiopheap(uint32_t busaddr);
uint32_t ps2sif_phystobus(uint32_t physaddr);
uint32_t ps2sif_bustophys(uint32_t busaddr);
#endif

#endif /* _MIPS_ASM_PS2_H */
