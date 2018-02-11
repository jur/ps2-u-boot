/*
 * Copyright (C) 2014 Jürgen Urban
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PS2_CONFIG_H
#define _PS2_CONFIG_H

#include <asm/addrspace.h>
#include <asm/ps2.h>

//#define DEBUG 1

/*
 * System configuration
 */
#define CONFIG_PS2

/*
 * Only interrupt autoboot if <space> is pressed. Otherwise, garbage
 * data on the serial line may interrupt the boot sequence.
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_AUTOBOOT
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT				\
	"Press SPACE to abort autoboot in %d seconds\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	"d"
#define CONFIG_AUTOBOOT_STOP_STR	" "

#define CONFIG_BOOTCOMMAND \
	"usb start;" \
	"fatload usb 0:1 0x81000000 /ps2/vmlinux;" \
	"usb stop;" \
	"bootelf 0x81000000"

#define CONFIG_MEMSIZE_IN_BYTES

/*
 * CPU Configuration
 */
#define CONFIG_SYS_MHZ			295	/* arbitrary value */
#define CONFIG_SYS_MIPS_TIMER_FREQ	(CONFIG_SYS_MHZ * 1000000)

/*
 * Memory map
 */
#define CONFIG_SYS_TEXT_BASE		0x80010000 /* Behind SBIOS */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* Cached addr */
/** Memory size 32MiB, last page is reserved for bootinfo. */
#define CONFIG_SYS_MEM_SIZE		(32 * 1024 * 1024 - 4096)

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

#define CONFIG_SYS_LOAD_ADDR		0x81000000
#define CONFIG_SYS_MEMTEST_START	0x80100000
#define CONFIG_SYS_MEMTEST_END		0x81f00000

#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)
#define CONFIG_SYS_BOOTM_LEN		(64 * 1024 * 1024)

/*
 * Console configuration
 */
#define CONFIG_SYS_PROMPT		"ps2 # "

#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

/*
 * Serial driver
 */
#define CONFIG_BAUDRATE			38400 /* Default of PS2 kernel */

#define CONFIG_CONS_INDEX		1

/*
 * Environment
 */
#define CONFIG_SYS_NO_FLASH
#if 0
#define CONFIG_ENV_IS_IN_NVRAM
/* TBD: Store U-Boot environment on MC0. */
#define CONFIG_ENV_ADDR 0x81000000
#else
#define CONFIG_ENV_IS_IN_FAT
#define FAT_ENV_INTERFACE "usb"
#define FAT_ENV_DEVICE 0
#define FAT_ENV_PART 1
#define FAT_ENV_FILE "u-boot.cfg"
#define CONFIG_FAT_WRITE
#endif
#define CONFIG_ENV_SIZE 0x00002000

/*
 * Commands
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_NFS

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_IDE
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_ELF
#define CONFIG_SUPPORT_VFAT

#define CONFIG_CMD_USB

#define CONFIG_SYS_LONGHELP		/* verbose help, undef to save memory */

/* Don't provide lowlevel_init asm. */
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_SYS_CACHELINE_SIZE 64

/*
 * USB Host
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_OHCI_PS2
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0xbf801600
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"ohci"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

#define CONFIG_USB_STORAGE
#endif

/*
 * IDE Support on SATA ports
 */
#ifdef CONFIG_CMD_IDE
#define CONFIG_CMD_EXT2
/* Data, registers and alternate blocks are at the same offset */
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x040)
#define CONFIG_SYS_ATA_REG_OFFSET	(0x040)
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x040)
/* Each 8-bit ATA register is aligned to a 2-bytes address */
#define CONFIG_SYS_ATA_STRIDE		2
/* Controller supports 48-bits LBA addressing */
#define CONFIG_LBA48
/* CONFIG_CMD_IDE requires some #defines for ATA registers */
#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_IDE_MAXDEVICE	2
/* ATA registers base is at SATA controller base */
#define CONFIG_SYS_ATA_BASE_ADDR	0x14000000
#endif /* CONFIG_CMD_IDE */

#endif /* _PS2_CONFIG_H */
