/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_PGALLOC_H
#define __ASM_MOXIE_PGALLOC_H

#include <asm/page.h>
#include <linux/threads.h>
#include <linux/mm.h>
#include <linux/memblock.h>

#include <asm-generic/pgalloc.h>

#define pmd_populate_kernel(mm, pmd, pte) \
	set_pmd(pmd, __pmd(_KERNPG_TABLE + __pa(pte)))

static inline void pmd_populate(struct mm_struct *mm, pmd_t *pmd,
				struct page *pte)
{
	set_pmd(pmd, __pmd(_KERNPG_TABLE +
		     ((unsigned long)page_to_pfn(pte) <<
		     (unsigned long) PAGE_SHIFT)));
}

/* Every address space shares the kernel's half of the page directory. */
static inline pgd_t *pgd_alloc(struct mm_struct *mm)
{
	pgd_t *ret = __pgd_alloc(mm, 0);

	if (ret)
		memcpy(ret + USER_PTRS_PER_PGD,
		       swapper_pg_dir + USER_PTRS_PER_PGD,
		       (PTRS_PER_PGD - USER_PTRS_PER_PGD) * sizeof(pgd_t));

	return ret;
}

#define __pte_free_tlb(tlb, pte, addr)	\
	tlb_remove_ptdesc((tlb), page_ptdesc(pte))

#endif
