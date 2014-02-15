/*
 *  Cache-handling routined for MIPS r5900
 *
 *  Copyright (c) 2003 Wolfgang Denk <wd@denx.de>
 *  Copyright (c) 2014 Juergen Urban
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>

#ifndef CONFIG_SYS_MIPS_CACHE_MODE
#define CONFIG_SYS_MIPS_CACHE_MODE CONF_CM_CACHABLE_NONCOHERENT
#endif

/*
 * dcache_status - get cache status
 *
 * RETURNS: 0 - cache disabled; 1 - cache enabled
 *
 */
LEAF(dcache_status)
	sync
	mfc0	t0, CP0_CONFIG
	li	t1, CONF_CM_UNCACHED
	andi	t0, t0, CONF_CM_CMASK
	move	v0, zero
	beq	t0, t1, 2f
	li	v0, 1
2:	jr	ra
	END(dcache_status)

/*
 * dcache_disable - disable cache
 *
 * RETURNS: N/A
 *
 */
LEAF(dcache_disable)
	sync
	mfc0	t0, CP0_CONFIG
	li	t1, -8
	and	t0, t0, t1
	ori	t0, t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
	sync
	jr	ra
	END(dcache_disable)

/*
 * dcache_enable - enable cache
 *
 * RETURNS: N/A
 *
 */
LEAF(dcache_enable)
	sync
	mfc0	t0, CP0_CONFIG
	ori	t0, CONF_CM_CMASK
	xori	t0, CONF_CM_CMASK
	ori	t0, CONFIG_SYS_MIPS_CACHE_MODE
	mtc0	t0, CP0_CONFIG
	sync
	jr	ra
	END(dcache_enable)
