/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The mox125 special registers, read with gsr and written with ssr.
 */
#ifndef __ASM_MOXIE_SR_H
#define __ASM_MOXIE_SR_H

#define SR_STATUS	0	/* interrupt enable, mode, condition codes */
#define SR_HANDLER	1	/* exception handler address */
#define SR_CAUSE	2	/* why the last exception was taken */
#define SR_SWI		3	/* number given to the last swi */
#define SR_SAVED_STATUS	4	/* status when the exception was taken */
#define SR_RESUME	5	/* where to resume after the exception */
#define SR_SCRATCH6	6
#define SR_SCRATCH7	7
#define SR_FAULT_ADDR	8	/* address that could not be translated */
#define SR_TLBHI	9	/* virtual page number, address space id */
#define SR_TLBLO	10	/* physical page number, flags */
#define SR_TLBWRITE	11	/* write: store tlbhi/tlblo, bit 0 picks ITLB */
#define SR_ASID		12
#define SR_TLB_HANDLER	13	/* TLB miss handler address */
#define SR_ICACHE_FLUSH	14	/* write: empty the instruction cache */

/* The status register. */
#define ST_IE		0x001	/* interrupts enabled */
#define ST_UM		0x004	/* user mode */
#define ST_MMU		0x008	/* address translation on */
#define ST_CC		0x1f0	/* condition codes */

/* What a TLB entry's low word holds besides the page number. */
#define TLB_V		0x01	/* valid */
#define TLB_W		0x02	/* writable */
#define TLB_U		0x08	/* user mode may use it */
#define TLB_G		0x10	/* matches every address space id */

#define TLB_ENTRIES	64

/* Exception causes. */
#define CAUSE_DIV0		0
#define CAUSE_UNDEFINED		1
#define CAUSE_IRQ		2
#define CAUSE_SWI		3
#define CAUSE_BRK		5
#define CAUSE_ITLB_MISS		6
#define CAUSE_DTLB_MISS_LOAD	7
#define CAUSE_DTLB_MISS_STORE	8
#define CAUSE_IPROT		9
#define CAUSE_DPROT_LOAD	10
#define CAUSE_DPROT_STORE	11
#define CAUSE_PRIV		12

#ifndef __ASSEMBLER__

/* The register number has to be a literal, hence the macros. */
#define mfsr(n)							\
({								\
	unsigned long __v;					\
	__asm__ __volatile__("gsr %0, " #n : "=r" (__v));	\
	__v;							\
})

#define mtsr(n, v)						\
	__asm__ __volatile__("ssr %0, " #n : : "r" ((unsigned long)(v)))

#endif /* __ASSEMBLER__ */

#endif /* __ASM_MOXIE_SR_H */
