/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Exception entry and exit, the TLB miss handler and the context
 * switch.
 *
 * The core takes every exception with translation off, in supervisor
 * mode with interrupts disabled.  It leaves the interrupted status in
 * SR_SAVED_STATUS and the address to resume at in SR_RESUME, and eret
 * puts both back.  A TLB miss taken by the kernel overwrites those two
 * registers, so nothing here may touch a virtual address while they
 * hold something we still need.
 */

#include <linux/linkage.h>
#include <linux/pgtable.h>
#include <asm/page.h>
#include <asm/moxie_sr.h>
#include <asm/thread_info.h>
#include <asm/asm-offsets.h>

#define PHYS(x)		((x) - PAGE_OFFSET)
#define ARG_AREA	24

	.text

/*
 * TLB miss.  Runs entirely with translation off and puts back every
 * register it uses, so the interrupted code never knows.
 *
 * The kernel's view of RAM and the device region are mapped by
 * arithmetic.  Everything else is looked up in the two-level page
 * table of the current address space, and only an entry that is both
 * present and accessed is loaded; anything less is handed to the page
 * fault code as if this handler had not run.
 */
ENTRY(moxie_tlb_miss)
	sta.l	PHYS(moxie_tlb_save + 0), $r0
	sta.l	PHYS(moxie_tlb_save + 4), $r1
	sta.l	PHYS(moxie_tlb_save + 8), $r2
	sta.l	PHYS(moxie_tlb_save + 12), $r3

	gsr	$r0, SR_FAULT_ADDR
	ldi.l	$r1, PAGE_OFFSET
	cmp	$r0, $r1
	bltu	.Lwalk
	ldi.l	$r2, VMALLOC_START
	cmp	$r0, $r2
	bltu	.Llinear
	ldi.l	$r2, IO_BASE
	cmp	$r0, $r2
	bltu	.Lwalk

	/* A device: virtual and physical addresses are the same. */
	ldi.l	$r1, PAGE_MASK
	and	$r0, $r1
	ldi.l	$r1, TLB_V | TLB_W | TLB_G
	or	$r0, $r1
	jmpa	PHYS(.Lload)

.Llinear:
	sub	$r0, $r1
	ldi.l	$r1, PAGE_MASK
	and	$r0, $r1
	ldi.l	$r1, TLB_V | TLB_W | TLB_G
	or	$r0, $r1
	jmpa	PHYS(.Lload)

.Lwalk:
	/* The directory entry for this address... */
	lda.l	$r1, PHYS(current_pgd_phys)
	mov	$r2, $r0
	ldi.l	$r3, PGDIR_SHIFT
	lshr	$r2, $r3
	ldi.l	$r3, 2
	ashl	$r2, $r3
	add	$r1, $r2
	ld.l	$r1, ($r1)
	ldi.l	$r2, _PAGE_PRESENT
	and	$r2, $r1
	xor	$r3, $r3
	cmp	$r2, $r3
	beq	.Lfault

	/* ...names a page of entries, by physical address. */
	ldi.l	$r2, PAGE_MASK
	and	$r1, $r2
	mov	$r2, $r0
	ldi.l	$r3, PAGE_SHIFT
	lshr	$r2, $r3
	ldi.l	$r3, PTRS_PER_PTE - 1
	and	$r2, $r3
	ldi.l	$r3, 2
	ashl	$r2, $r3
	add	$r1, $r2
	ld.l	$r0, ($r1)

	ldi.l	$r2, _PAGE_PRESENT | _PAGE_ACCESSED
	mov	$r1, $r0
	and	$r1, $r2
	cmp	$r1, $r2
	bne	.Lfault

	/* Code may only be fetched from a page that allows it. */
	gsr	$r1, SR_CAUSE
	ldi.l	$r2, CAUSE_ITLB_MISS
	cmp	$r1, $r2
	bne	1f
	ldi.l	$r2, _PAGE_EXEC
	mov	$r1, $r0
	and	$r1, $r2
	cmp	$r1, $r2
	bne	.Lfault
1:
	ldi.l	$r1, _PAGE_TLB_MASK
	and	$r0, $r1

.Lload:
	ssr	$r0, SR_TLBLO
	gsr	$r0, SR_CAUSE
	ldi.l	$r1, CAUSE_ITLB_MISS
	xor	$r2, $r2
	cmp	$r0, $r1
	bne	2f
	ldi.l	$r2, 1
2:	ssr	$r2, SR_TLBWRITE

	lda.l	$r0, PHYS(moxie_tlb_save + 0)
	lda.l	$r1, PHYS(moxie_tlb_save + 4)
	lda.l	$r2, PHYS(moxie_tlb_save + 8)
	lda.l	$r3, PHYS(moxie_tlb_save + 12)
	eret

.Lfault:
	lda.l	$r0, PHYS(moxie_tlb_save + 0)
	lda.l	$r1, PHYS(moxie_tlb_save + 4)
	lda.l	$r2, PHYS(moxie_tlb_save + 8)
	lda.l	$r3, PHYS(moxie_tlb_save + 12)
	/* fall through, with everything as the core left it */
END(moxie_tlb_miss)

/*
 * Every other exception.  Still with translation off, find the kernel
 * stack, make room for a struct pt_regs on it and fill that in through
 * its physical address.  Only once everything the core told us is
 * safely in memory do we turn translation back on.
 */
ENTRY(moxie_exception_entry)
	sta.l	PHYS(moxie_exc_save + 0), $r0
	sta.l	PHYS(moxie_exc_save + 4), $r1
	sta.l	PHYS(moxie_exc_save + 8), $r2

	/* From user mode start at the top of the task's kernel stack;
	 * from the kernel carry on down the one we are on. */
	gsr	$r0, SR_SAVED_STATUS
	ldi.l	$r1, ST_UM
	and	$r1, $r0
	xor	$r2, $r2
	cmp	$r1, $r2
	beq	1f
	lda.l	$r1, PHYS(moxie_kernel_sp)
	jmpa	PHYS(2f)
1:	mov	$r1, $sp
2:	dec	$r1, PT_SIZE

	mov	$r2, $r1
	ldi.l	$r0, PAGE_OFFSET
	sub	$r2, $r0

	sto.l	PT_SP($r2), $sp
	sto.l	PT_FP($r2), $fp
	lda.l	$r0, PHYS(moxie_exc_save + 0)
	sto.l	PT_R0($r2), $r0
	lda.l	$r0, PHYS(moxie_exc_save + 4)
	sto.l	PT_R1($r2), $r0
	lda.l	$r0, PHYS(moxie_exc_save + 8)
	sto.l	PT_R2($r2), $r0
	sto.l	PT_R3($r2), $r3
	sto.l	PT_R4($r2), $r4
	sto.l	PT_R5($r2), $r5
	sto.l	PT_R6($r2), $r6
	sto.l	PT_R7($r2), $r7
	sto.l	PT_R8($r2), $r8
	sto.l	PT_R9($r2), $r9
	sto.l	PT_R10($r2), $r10
	sto.l	PT_R11($r2), $r11
	sto.l	PT_R12($r2), $r12
	sto.l	PT_R13($r2), $r13
	gsr	$r0, SR_RESUME
	sto.l	PT_PC($r2), $r0
	gsr	$r0, SR_SAVED_STATUS
	sto.l	PT_SR($r2), $r0
	gsr	$r0, SR_CAUSE
	sto.l	PT_CAUSE($r2), $r0
	gsr	$r0, SR_FAULT_ADDR
	sto.l	PT_EA($r2), $r0
	ldi.l	$r0, -1
	sto.l	PT_SYSCALLNO($r2), $r0

	mov	$sp, $r1
	ldi.l	$r0, ST_MMU
	ssr	$r0, SR_SAVED_STATUS
	ldi.l	$r0, moxie_exception_virtual
	ssr	$r0, SR_RESUME
	eret
END(moxie_exception_entry)

/* With translation on, $sp at the saved registers, interrupts off. */
moxie_exception_virtual:
	xor	$fp, $fp
	mov	$r0, $sp
	dec	$sp, ARG_AREA
	jsra	moxie_do_exception

ENTRY(ret_from_exception)
	/* $sp is ARG_AREA below the saved registers. */
	mov	$r0, $sp
	inc	$r0, ARG_AREA
	jsra	moxie_prepare_return
	inc	$sp, ARG_AREA

	/*
	 * Restore with translation off, the way we saved, so that no
	 * TLB miss can disturb the status and resume address once they
	 * are loaded.  Getting there is itself an eret, and the three
	 * instructions that do it are kept within one page so that the
	 * instruction TLB entry that fetched the first also covers the
	 * last.
	 */
	xor	$r0, $r0
	ldi.l	$r1, PHYS(.Lrestore)
	jmpa	1f
	.balign	16
1:	ssr	$r0, SR_SAVED_STATUS
	ssr	$r1, SR_RESUME
	eret

.Lrestore:
	mov	$r1, $sp
	ldi.l	$r0, PAGE_OFFSET
	sub	$r1, $r0
	ldo.l	$r0, PT_PC($r1)
	ssr	$r0, SR_RESUME
	ldo.l	$r0, PT_SR($r1)
	ssr	$r0, SR_SAVED_STATUS
	ldo.l	$fp, PT_FP($r1)
	ldo.l	$r2, PT_R2($r1)
	ldo.l	$r3, PT_R3($r1)
	ldo.l	$r4, PT_R4($r1)
	ldo.l	$r5, PT_R5($r1)
	ldo.l	$r6, PT_R6($r1)
	ldo.l	$r7, PT_R7($r1)
	ldo.l	$r8, PT_R8($r1)
	ldo.l	$r9, PT_R9($r1)
	ldo.l	$r10, PT_R10($r1)
	ldo.l	$r11, PT_R11($r1)
	ldo.l	$r12, PT_R12($r1)
	ldo.l	$r13, PT_R13($r1)
	ldo.l	$sp, PT_SP($r1)
	ldo.l	$r0, PT_R0($r1)
	ldo.l	$r1, PT_R1($r1)
	eret
END(ret_from_exception)

/*
 * struct task_struct *__switch_to(struct task_struct *prev,
 *				   struct task_struct *next)
 *
 * The call that got us here left a frame at $fp holding the caller's
 * $fp and the return address, and "ret" needs nothing else: it even
 * reloads $sp from $fp.  So a task is suspended by saving $fp and the
 * callee-saved registers, and resumed by loading them and returning.
 * prev is still in $r0 when we do, which is the return value.
 */
ENTRY(__switch_to)
	ldi.l	$r2, TASK_THREAD
	mov	$r3, $r0
	add	$r3, $r2
	sto.l	THREAD_FP($r3), $fp
	sto.l	THREAD_R6($r3), $r6
	sto.l	THREAD_R7($r3), $r7
	sto.l	THREAD_R8($r3), $r8
	sto.l	THREAD_R9($r3), $r9
	sto.l	THREAD_R10($r3), $r10
	sto.l	THREAD_R11($r3), $r11

	/* Exceptions from user mode start at the top of next's stack. */
	ldo.l	$r4, TASK_STACK($r1)
	ldi.l	$r5, THREAD_SIZE
	add	$r4, $r5
	sta.l	moxie_kernel_sp, $r4

	mov	$r3, $r1
	add	$r3, $r2
	ldo.l	$fp, THREAD_FP($r3)
	ldo.l	$r6, THREAD_R6($r3)
	ldo.l	$r7, THREAD_R7($r3)
	ldo.l	$r8, THREAD_R8($r3)
	ldo.l	$r9, THREAD_R9($r3)
	ldo.l	$r10, THREAD_R10($r3)
	ldo.l	$r11, THREAD_R11($r3)
	ret
END(__switch_to)

/*
 * Where a new task lands the first time it is switched to.
 * copy_thread() arranged for $sp to be ARG_AREA below the task's
 * saved user registers, and for a kernel thread put its function in
 * $r6 and the argument in $r7.
 */
ENTRY(ret_from_fork)
	jsra	schedule_tail
	xor	$r0, $r0
	cmp	$r6, $r0
	beq	1f
	mov	$r0, $r7
	jsr	$r6
1:	jmpa	ret_from_exception
END(ret_from_fork)

	.data
	.balign	4
/* Scratch for the handlers above, which have no stack to use. */
moxie_tlb_save:
	.space	16
moxie_exc_save:
	.space	12
/* The top of the current task's kernel stack. */
	.globl	moxie_kernel_sp
moxie_kernel_sp:
	.long	init_thread_union + THREAD_SIZE
