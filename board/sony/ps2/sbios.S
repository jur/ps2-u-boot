/*
 *  SBIOS interface for MIPS r5900
 *
 *  Copyright (c) 2014 Juergen Urban
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.text
.global sbios_call

/*
 * extern int sbios_call(int func, void *arg, void *sbiosfunc);
 * a0 = func
 * a1 = arg
 * a2 = sbiosfunc
 *
 * Calls sbiosfunc with 16 Byte aligned stack and a0 + a1 as
 * parameter.
 * Returns v0 return value of SBIOS call.
 */
sbios_call:
	move	$t0, $sp
	addiu	$sp,$sp,-32

	# Align stack to 16 Bytes:
	li	$t1,-16
	and     $sp,$sp,$t1

	sw	$ra,28($sp)
	sw	$t0,24($sp)
	move	$t9, $a2
	jalr	$t9
	lw	$ra,28($sp)
	lw	$sp,24($sp)
	jr	$ra
