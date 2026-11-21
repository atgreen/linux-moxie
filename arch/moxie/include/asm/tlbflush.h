/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_TLBFLUSH_H
#define __ASM_MOXIE_TLBFLUSH_H

#include <linux/mm.h>

extern void local_flush_tlb_all(void);
extern void local_flush_tlb_page(struct vm_area_struct *vma,
				 unsigned long addr);
extern void local_flush_tlb_range(struct vm_area_struct *vma,
				  unsigned long start, unsigned long end);

/* One address space id for everyone, so there is no per-mm flush. */
#define flush_tlb_all()			local_flush_tlb_all()
#define flush_tlb_mm(mm)		local_flush_tlb_all()
#define flush_tlb_page(vma, addr)	local_flush_tlb_page(vma, addr)
#define flush_tlb_range(vma, s, e)	local_flush_tlb_range(vma, s, e)

static inline void flush_tlb_kernel_range(unsigned long start,
					  unsigned long end)
{
	local_flush_tlb_range(NULL, start, end);
}

#endif /* __ASM_MOXIE_TLBFLUSH_H */
