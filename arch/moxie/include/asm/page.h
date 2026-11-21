/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_PAGE_H
#define __ASM_MOXIE_PAGE_H

#include <vdso/page.h>

/*
 * The kernel's view of RAM starts here.  The TLB miss handler maps
 * this region by arithmetic, without page tables.
 */
#define PAGE_OFFSET	0xc0000000

#ifndef __ASSEMBLER__

#define clear_page(page)	memset((page), 0, PAGE_SIZE)
#define copy_page(to, from)	memcpy((to), (from), PAGE_SIZE)

#define copy_user_page(to, from, vaddr, pg)     copy_page(to, from)

typedef struct {
	unsigned long pte;
} pte_t;
typedef struct {
	unsigned long pgd;
} pgd_t;
typedef struct {
	unsigned long pgprot;
} pgprot_t;
typedef struct page *pgtable_t;

#define pte_val(x)	((x).pte)
#define pgd_val(x)	((x).pgd)
#define pgprot_val(x)	((x).pgprot)

#define __pte(x)	((pte_t) { (x) })
#define __pgd(x)	((pgd_t) { (x) })
#define __pgprot(x)	((pgprot_t) { (x) })

#define __va(x) ((void *)((unsigned long)(x) + PAGE_OFFSET))
#define __pa(x) ((unsigned long) (x) - PAGE_OFFSET)

static inline unsigned long virt_to_pfn(const void *kaddr)
{
	return __pa(kaddr) >> PAGE_SHIFT;
}

#define virt_to_page(addr) \
	(mem_map + (((unsigned long)(addr)-PAGE_OFFSET) >> PAGE_SHIFT))

#define virt_addr_valid(kaddr)	(pfn_valid(virt_to_pfn(kaddr)))

#endif /* __ASSEMBLER__ */

#include <asm-generic/memory_model.h>
#include <asm-generic/getorder.h>

#endif /* __ASM_MOXIE_PAGE_H */
