/*
 *  Copyright (C) 2014 Juergen Urban
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <rtc.h>
#include <serial.h>

#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/ps2.h>

#include "bootinfo.h"
#include "sbios.h"

/* EE SIO registers */
#define EE_SIO_BASE 0x1000f100
#define EE_SIO_SIZE 0x100

#define EE_SIO_LCR 0x1000f100 /* Line Control Register */
#define LCR_UMODE_8BIT	0x00
#define LCR_UMODE_7BIT	0x01
#define LCR_USBL_1BIT	0x00 /* UART Stop Bit Length */
#define LCR_USBL_2BITS	0x04
#define LCR_UPEN_OFF	0x00 /* UART Parity Enable */
#define LCR_UPEN_ON	0x08
#define LCR_UEPS_ODD	0x00 /* UART Even Parity Select */
#define LCR_UEPS_EVEN	0x10
#define LCR_SCS_VAL	0x20 /* Baud rate generator output that divided CPUCLK */

#define EE_SIO_LSR 0x1000f110 /* Line Status Register */
#define LSR_OE 0x02 /* Overrun error */
#define LSR_PE 0x04 /* Parity error */
#define LSR_FE 0x08 /* Framing error */

#define EE_SIO_IER 0x1000f120 /* Interrupt Enable Register */
#define IER_RX_DATA	0x0001
#define IER_TX_EMPTY	0x0002
#define IER_BI		0x0004 /* Break signal */

#define EE_SIO_ISR 0x1000f130 /* Interrupt Status Register */
#define ISR_RX_DATA	0x0001
#define ISR_TX_EMPTY	0x0002
#define ISR_BI		0x0004 /* Break signal */
#define ISR_TXFIFO_MASK 0xf000
#define ISR_TXFIFO_FULL 0x8000
#define ISR_RXFIFO_MASK 0x0f00

#define EE_SIO_FCR 0x1000f140 /* FIFO Control Register */
#define FCR_FRSTE 0x01 /* FIFO Reset Enable */
#define FCR_RFRST 0x02 /* RX FIFO Reset */
#define SIO_FCR_TFRST 0x04 /* TX FIFO Reset */

#define SIO_BGR 0x1000f150 /* Baud Rate Control Register */

#define EE_SIO_TXFIFO 0x1000f180
#define EE_SIO_RXFIFO 0x1000f1c0

#define EE_DMAC_SIF0_CHCR 0x1000c000
#define CHCR_STR 0x0100

#define SBIOS_BASE	0x80001000
#define SBIOS_MAGIC	0x80001004
#define SBIOS_MAGICVAL	0x62325350

/* Base address for IOP memory. */
#define PS2_IOP_HEAP_BASE 0x1c000000
#define PS2_IOP_HEAP_END  0x20000000

static struct ps2_bootinfo bootinfo;

static int (*_sbios)(int, void *) = NULL;
static int sbversion = 0;

int sbios(int func, void *arg)
{
	if (_sbios == NULL) {
		extern void _start(void);
		error("SBIOS not found\n");
		/* TBD: Is data segment lost after reloc? */
		_sbios = *(int (**)(int, void *))(SBIOS_BASE); // TBD
		error("_start func at 0x%08x\n", (unsigned int) _start);
		/* TBD: return -1; */
	}
	return _sbios(func, arg);
}

phys_size_t initdram(int board_type)
{
	return CONFIG_SYS_MEM_SIZE;
}

static void check_sif_irq(void)
{
	if (inl(EE_DMAC_SIF0_CHCR) & CHCR_STR) {
		/* DMA not finished, no interrupt. */
		return;
	}

	/* Emulate interrupt. */
	sbios(SB_SIFCMDINTRHDLR, NULL);
}

static void rpc_wakeup(void *p, int result)
{
	volatile int *woken;

	woken = (int *) p;
	*woken = 1;
}

static int sbios_rpc(int func, void *arg, int *result)
{
	int ret;
	struct sbr_common_arg carg;
	volatile int woken = 0;

	carg.arg = arg;
	carg.func = rpc_wakeup;
	carg.para = (void *) &woken;
	carg.result = -1;

	do {
		debug("Call SBIOS %d\n", func);
		ret = sbios(func, &carg);
		debug("ret = %d\n", ret);
		switch (ret) {
			case 0:
				break;
			case -SIF_RPCE_SENDP:
				/* resource temporarily unavailable */

				/* Time to check for interrupts. */
				check_sif_irq();
				break;
			default:
				*result = ret;
				error("sbios_rpc: RPC failed, func=%d result=%d\n", func, ret);
				return ret;
		}
	} while (ret < 0);

	while(woken == 0) {
		debug("check_sif_irq\n");

		/* Time to check for interrupts. */
		check_sif_irq();
	}
	debug("carg.result %d\n", carg.result);

	*result = carg.result;

	return 0;
}

static void driver_init(void)
{
	int rv;
	int result;

	rv = sbios(SB_SIFINIT, NULL);
	if (rv < 0) {
		error("SB_SIFINIT failed rv = %d\n", rv);
		return;
	}

	rv = sbios(SB_SIFINITCMD, NULL);
	if (rv < 0) {
		error("SB_SIFINITCMD failed rv = %d\n", rv);
		return;
	}

	rv = sbios(SB_SIFINITRPC, NULL);
	if (rv < 0) {
		error("SB_SIFINITRPC failed rv = %d\n", rv);
		return;
	}

	for (;;) {
		debug("Call SBR_IOPH_INIT\n");
		rv = sbios_rpc(SBR_IOPH_INIT, NULL, &result);
		debug("rv = %d\n", rv);
		if (rv < 0) {
			error("SBR_IOPH_INIT failed rv = %d\n", rv);
			return;
		}
		debug("result = %d\n", result);
		if (result == 0) {
			/* Success */
			break;
		}
	}
	debug("SBIOS drivers successfully initialized.\n");
}

int checkboard(void)
{
	struct ps2_bootinfo *info = (void *) CKSEG0ADDR(PS2_BOOTINFO_OLDADDR);
	uint32_t size;
	volatile uint32_t *sbios_magic;

	puts("Board: Sony Playstation 2 MIPS r5900\n");
	memset(&bootinfo, 0, sizeof(bootinfo));
	size = info->size;
	if (size > sizeof(bootinfo)) {
		size = sizeof(bootinfo);
	}
	memcpy(&bootinfo, info, size);


	sbios_magic = (uint32_t *) SBIOS_MAGIC;
	if (*sbios_magic == SBIOS_MAGICVAL) {
		_sbios = *(int (**)(int, void *))(SBIOS_BASE);
	}
	else
		_sbios = NULL;

	sbversion = sbios(SB_GETVER, NULL);
	printf("SBIOS Version 0x%08x\n", sbversion);

	driver_init();
	return 0;
}

int board_eth_init(bd_t *bis)
{
	/* TBD */
	return 0;
}

void _machine_restart(void)
{
	struct sb_halt_arg arg;

	arg.mode = SB_HALT_MODE_PWROFF;
	sbios(SB_HALT, &arg);
}

int board_early_init_f(void)
{
	return 0;
}

int sio_start(void)
{
	return 0;
}

int sio_stop(void)
{
	return 0;
}

void sio_setbrg(void)
{
	/* TBD: Configure baudrate */
}

int sio_getc(void)
{
	uint32_t isr;
	uint8_t ch;

	while((((isr = inl(EE_SIO_ISR)) & ISR_RXFIFO_MASK) >> 8) == 0) {
		/* Wait for input */
	}
	ch = inb(EE_SIO_RXFIFO);
	return ch;
}

int sio_tstc(void)
{
	/* Return number of characters in RX FIFO. */
	return (((inl(EE_SIO_ISR)) & ISR_RXFIFO_MASK) >> 8);
}

void sio_putc(const char c)
{
	if (c == '\n')
		sio_putc('\r');
	while ((inl(EE_SIO_ISR) & ISR_TXFIFO_MASK) == ISR_TXFIFO_FULL) {
		/* Wait until data can be send. */
	};
	outb(c, EE_SIO_TXFIFO);
}

void sio_puts(const char *s)
{
	while(*s != 0) {
		sio_putc(*s);
		s++;
	}
}

struct serial_device sio_device = {
	.name = "PS2SIO",
	.start = sio_start,
	.stop = sio_stop,
	.setbrg = sio_setbrg,
	.getc = sio_getc,
	.tstc = sio_tstc,
	.putc = sio_putc,
	.puts = sio_puts,
};

struct serial_device *default_serial_console(void)
{
	set_io_port_base(CKSEG1ADDR(0));
	return &sio_device;
}

void rtc_reset (void)
{
	/* TBD */
	return;
}

int rtc_get (struct rtc_time *tmp)
{
	/* TBD */
	return 0;
}

int rtc_set (struct rtc_time *tmp)
{
	/* TBD */
	return 0;
}

/** Allocate bus address. Returns 0 on error. */
uint32_t ps2sif_allociopheap(int size)
{
	struct sbr_ioph_alloc_arg arg;
	int result;
	int err;

	arg.size = size;
	err = sbios_rpc(SBR_IOPH_ALLOC, &arg, &result);

	if (err < 0) {
		return 0;
	}
	return result;
}

int ps2sif_freeiopheap(uint32_t busaddr)
{
	struct sbr_ioph_free_arg arg;
	int result;
	int err;

	arg.addr = busaddr;
	err = sbios_rpc(SBR_IOPH_FREE, &arg, &result);

	if (err < 0) {
		return -1;
	}
	return result;
}

uint32_t ps2sif_phystobus(uint32_t physaddr)
{
	if (physaddr <= PS2_IOP_HEAP_BASE) {
		error("Bad physical address 0x%08x used\n",
			physaddr);
	}
	if (physaddr >= PS2_IOP_HEAP_END) {
		error("Bad physical address 0x%08x used\n",
			physaddr);
	}
	return (physaddr - PS2_IOP_HEAP_BASE);
}

/** Converts iopheap bus addr to phys addr for use with inl() and outl(). */
uint32_t ps2sif_bustophys(uint32_t busaddr)
{
	uint32_t physaddr;

	assert(busaddr != 0);

	physaddr = busaddr + PS2_IOP_HEAP_BASE;
	if (physaddr < PS2_IOP_HEAP_BASE) {
		error("Bad bus address 0x%08x used\n",
			busaddr);
	}
	if (physaddr >= PS2_IOP_HEAP_END) {
		error("Bad bus address 0x%08x used\n",
			busaddr);
	}
	return physaddr;
}
