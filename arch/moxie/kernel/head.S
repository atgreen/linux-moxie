/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Where the kernel starts, and how it gets from physical addresses
 * to virtual ones.
 */

#include <linux/linkage.h>
#include <linux/init.h>
#include <linux/pgtable.h>
#include <asm/page.h>
#include <asm/moxie_sr.h>
#include <asm/thread_info.h>
#include <asm/asm-offsets.h>

/*
 * The kernel is linked at PAGE_OFFSET.  While translation is off the
 * core is executing it at its physical address, and anything absolute
 * has to say so.  Conditional branches are relative and work anywhere.
 */
#define PHYS(x)		((x) - PAGE_OFFSET)

/* The ABI has every caller leave room for six argument registers. */
#define ARG_AREA	24

	__HEAD
ENTRY(_start)
	/*
	 * The core starts here, at physical 0x1000, in supervisor mode
	 * with interrupts and translation off.  Nothing is known about
	 * what the TLBs hold, so make every entry invalid.
	 */
	xor	$r0, $r0
	ssr	$r0, SR_TLBLO
	ssr	$r0, SR_ASID
	ldi.l	$r1, TLB_ENTRIES
	ldi.l	$r2, PAGE_SIZE
	xor	$r3, $r3
	ldi.l	$r4, 1
1:	ssr	$r0, SR_TLBHI
	ssr	$r3, SR_TLBWRITE
	ssr	$r4, SR_TLBWRITE
	add	$r0, $r2
	dec	$r1, 1
	cmp	$r1, $r3
	bne	1b

	/* Exception handlers run with translation off, hence PHYS. */
	ldi.l	$r0, PHYS(moxie_exception_entry)
	ssr	$r0, SR_HANDLER
	ldi.l	$r0, PHYS(moxie_tlb_miss)
	ssr	$r0, SR_TLB_HANDLER

	/*
	 * Turn translation on.  The first fetch at the virtual address
	 * misses the TLB, and the miss handler maps the kernel's view
	 * of RAM by arithmetic, so no page tables are needed yet.
	 */
	ldi.l	$r0, ST_MMU
	ssr	$r0, SR_SAVED_STATUS
	ldi.l	$r0, _start_virtual
	ssr	$r0, SR_RESUME
	eret

_start_virtual:
	ldi.l	$sp, init_thread_union + THREAD_SIZE - ARG_AREA
	xor	$fp, $fp

	ldi.l	$r0, __bss_start
	ldi.l	$r1, __bss_stop
	xor	$r2, $r2
2:	cmp	$r0, $r1
	beq	3f
	st.l	($r0), $r2
	inc	$r0, 4
	jmpa	2b
3:
	jsra	start_kernel
4:	jmpa	4b
END(_start)
