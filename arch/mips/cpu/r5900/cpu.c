/*
 *  Copyright (c) 2003 Wolfgang Denk <wd@denx.de>
 *  Copyright (c) 2014 Juergen Urban
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>
#include <asm/reboot.h>

#define dcache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	sync.l						\n"	\
	"	cache	%0, %1					\n"	\
	"	sync.l						\n"	\
	"	.set	pop					\n"	\
	:								\
	: "i" (op), "R" (*(unsigned char *)(addr)))

#define icache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	sync.p						\n"	\
	"	cache	%0, %1					\n"	\
	"	sync.p						\n"	\
	"	.set	pop					\n"	\
	:								\
	: "i" (op), "R" (*(unsigned char *)(addr)))

void __attribute__((weak)) _machine_restart(void)
{
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	_machine_restart();

	fprintf(stderr, "*** reset failed ***\n");
	return 0;
}

#ifdef CONFIG_SYS_CACHELINE_SIZE
static inline unsigned long icache_line_size(void)
{
	return CONFIG_SYS_CACHELINE_SIZE;
}

static inline unsigned long dcache_line_size(void)
{
	return CONFIG_SYS_CACHELINE_SIZE;
}
#endif /* CONFIG_SYS_CACHELINE_SIZE */

void flush_cache(ulong start_addr, ulong size)
{
	unsigned long ilsize = icache_line_size();
	unsigned long dlsize = dcache_line_size();
	unsigned long addr, aend;

	/* aend will be miscalculated when size is zero, so we return here */
	if (size == 0)
		return;

	addr = start_addr & ~(dlsize - 1);
	aend = (start_addr + size - 1) & ~(dlsize - 1);

	if (ilsize == dlsize) {
		/* flush I-cache & D-cache simultaneously */
		while (1) {
			dcache_op(R5900_HIT_WRITEBACK_INV_D, addr);
			icache_op(R5900_HIT_INVALIDATE_I, addr);
			if (addr == aend)
				break;
			addr += dlsize;
		}
		return;
	}

	/* flush D-cache */
	while (1) {
		dcache_op(R5900_HIT_WRITEBACK_INV_D, addr);
		if (addr == aend)
			break;
		addr += dlsize;
	}

	/* flush I-cache */
	addr = start_addr & ~(ilsize - 1);
	aend = (start_addr + size - 1) & ~(ilsize - 1);
	while (1) {
		icache_op(R5900_HIT_INVALIDATE_I, addr);
		if (addr == aend)
			break;
		addr += ilsize;
	}
}

void flush_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) {
		dcache_op(R5900_HIT_WRITEBACK_INV_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) {
		dcache_op(R5900_HIT_INVALIDATE_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}

int cpu_eth_init(bd_t *bis)
{
	return 0;
}
