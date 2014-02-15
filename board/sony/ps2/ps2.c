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

#define SBIOS_BASE	0x80001000
#define SBIOS_MAGIC	0x80001004
#define SBIOS_MAGICVAL	0x62325350

static struct ps2_bootinfo bootinfo;

static int (*_sbios)(int, void *) = NULL;
static int sbversion = 0;

int sbios(int func, void *arg)
{
	if (_sbios == NULL) {
		return -1;
	}
	return _sbios(func, arg);
}

phys_size_t initdram(int board_type)
{
	return CONFIG_SYS_MEM_SIZE;
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
	/* TBD */

	puts("_machine_restart\n");
	arg.mode = SB_HALT_MODE_RESTART;
	arg.mode = SB_HALT_MODE_PWROFF;
	sbios(SB_HALT, &arg);
	puts("_machine_restart 2\n");
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
