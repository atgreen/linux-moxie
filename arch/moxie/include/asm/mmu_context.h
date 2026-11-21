/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_MMU_CONTEXT_H
#define __ASM_MOXIE_MMU_CONTEXT_H

#include <asm-generic/mm_hooks.h>

struct task_struct;
struct mm_struct;

extern void switch_mm(struct mm_struct *prev, struct mm_struct *next,
		      struct task_struct *tsk);

#define activate_mm(prev, next) switch_mm((prev), (next), NULL)

/* The page directory the TLB miss handler walks, as a physical address. */
extern unsigned long current_pgd_phys;

#include <asm-generic/mmu_context.h>

#endif
