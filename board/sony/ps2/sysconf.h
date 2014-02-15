/*
 *	Copyright (c) 2000 Sony Computer Entertainment Inc.
 *	Copyright (c) 2014 Juergen Urban
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __ASM_PS2_SYSCONF_H
#define __ASM_PS2_SYSCONF_H

struct ps2_sysconf {
	short timezone;
	uint8_t aspect;
	uint8_t datenotation;
	uint8_t language;
	uint8_t spdif;
	uint8_t summertime;
	uint8_t timenotation;
	uint8_t video;
};

#endif /* __ASM_PS2_SYSCONF_H */
