// SPDX-License-Identifier: GPL-2.0
/*
 * The TLBs are refilled by the miss handler in entry.S.  All that is
 * left to do here is throw entries away when a mapping changes.
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pgtable.h>

#include <asm/moxie_sr.h>
#include <asm/mmu_context.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>

unsigned long current_pgd_phys;

/* Overwrite the entry a page would use, in both TLBs, with nothing. */
static void tlb_invalidate(unsigned long addr)
{
	unsigned long flags;

	local_irq_save(flags);
	mtsr(9, addr & PAGE_MASK);
	mtsr(10, 0);
	mtsr(11, 0);
	mtsr(11, 1);
	local_irq_restore(flags);
}

void local_flush_tlb_all(void)
{
	unsigned long i;

	for (i = 0; i < TLB_ENTRIES; i++)
		tlb_invalidate(i << PAGE_SHIFT);

	/* Code fetched through a mapping that is gone must go too. */
	moxie_flush_icache();
}

void local_flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
	tlb_invalidate(addr);
	moxie_flush_icache();
}

void local_flush_tlb_range(struct vm_area_struct *vma,
			   unsigned long start, unsigned long end)
{
	if (end - start >= TLB_ENTRIES * PAGE_SIZE) {
		local_flush_tlb_all();
		return;
	}
	for (start &= PAGE_MASK; start < end; start += PAGE_SIZE)
		tlb_invalidate(start);
	moxie_flush_icache();
}

/*
 * Every address space runs as address space id 0, so changing to
 * another one means forgetting everything.
 */
void switch_mm(struct mm_struct *prev, struct mm_struct *next,
	       struct task_struct *tsk)
{
	if (prev == next)
		return;

	current_pgd_phys = __pa(next->pgd);
	local_flush_tlb_all();
}
