/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Page tables for the mox125 software-refilled TLB.  Derived in
 * outline from arch/openrisc.
 */
#ifndef __ASM_MOXIE_PGTABLE_H
#define __ASM_MOXIE_PGTABLE_H

#include <asm/page.h>
#include <asm-generic/pgtable-nopmd.h>

#ifndef __ASSEMBLER__
#include <asm/mmu.h>

extern void paging_init(void);

#define set_pte(pteptr, pteval) ((*(pteptr)) = (pteval))
#define set_pmd(pmdptr, pmdval) (*(pmdptr) = pmdval)
#endif

/* Two levels, 1024 entries each, 4 KiB pages. */
#define PGDIR_SHIFT	(PAGE_SHIFT + (PAGE_SHIFT-2))
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

#define PTRS_PER_PTE	(1UL << (PAGE_SHIFT-2))
#define PTRS_PER_PGD	(1UL << (32-PGDIR_SHIFT))

#define USER_PTRS_PER_PGD       (TASK_SIZE/PGDIR_SIZE)

/*
 * Kernel virtual memory:
 *
 *   0xc0000000  RAM, mapped by the TLB miss handler by arithmetic
 *   0xd0000000  vmalloc, mapped through the page tables
 *   0xf0000000  devices, mapped one to one by the TLB miss handler
 */
#define VMALLOC_START	0xd0000000UL
#define VMALLOC_END	0xf0000000UL
#define IO_BASE		0xf0000000UL

/*
 * The low bits of a page table entry are the low bits of a TLB entry,
 * so the miss handler can load one with a mask and nothing else:
 *
 *   _PAGE_PRESENT   the TLB's valid bit
 *   _PAGE_HWWRITE   the TLB's writable bit; set only while the page is
 *                   both writable and dirty, so that the first write
 *                   to a clean page faults and we get to mark it dirty
 *   _PAGE_USER      the TLB's user bit
 *   _PAGE_GLOBAL    the TLB's global bit
 *
 * The rest are the kernel's own.  The miss handler refuses an entry
 * that is not both present and accessed, which is how the accessed
 * bit gets set.
 */
#define _PAGE_PRESENT	0x001
#define _PAGE_HWWRITE	0x002
#define _PAGE_USER	0x008
#define _PAGE_GLOBAL	0x010
#define _PAGE_ACCESSED	0x020
#define _PAGE_DIRTY	0x040
#define _PAGE_WRITE	0x080
#define _PAGE_EXEC	0x100

/* What the TLB miss handler copies into the TLB. */
#define _PAGE_TLB_MASK	(PAGE_MASK | _PAGE_PRESENT | _PAGE_HWWRITE \
			 | _PAGE_USER | _PAGE_GLOBAL)

#define _PAGE_CHG_MASK	(PAGE_MASK | _PAGE_ACCESSED | _PAGE_DIRTY)

/* Only meaningful when the entry is not present. */
#define _PAGE_SWP_EXCLUSIVE	_PAGE_USER

#define _KERNPG_TABLE	(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_DIRTY \
			 | _PAGE_WRITE | _PAGE_HWWRITE)

#define PAGE_NONE	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED)
#define PAGE_READONLY	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_USER)
#define PAGE_READONLY_X	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_USER \
				 | _PAGE_EXEC)
#define PAGE_SHARED	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_USER \
				 | _PAGE_WRITE)
#define PAGE_SHARED_X	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_USER \
				 | _PAGE_WRITE | _PAGE_EXEC)
#define PAGE_COPY	PAGE_READONLY
#define PAGE_COPY_X	PAGE_READONLY_X

#define PAGE_KERNEL	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_DIRTY \
				 | _PAGE_WRITE | _PAGE_HWWRITE | _PAGE_GLOBAL \
				 | _PAGE_EXEC)
#define PAGE_KERNEL_RO	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_GLOBAL \
				 | _PAGE_EXEC)

#ifndef __ASSEMBLER__

#define pte_none(x)	(!pte_val(x))
#define pte_present(x)	(pte_val(x) & _PAGE_PRESENT)
#define pte_clear(mm, addr, xp)	do { pte_val(*(xp)) = 0; } while (0)

#define pmd_none(x)	(!pmd_val(x))
#define	pmd_bad(x)	((pmd_val(x) & (~PAGE_MASK)) != _KERNPG_TABLE)
#define pmd_present(x)	(pmd_val(x) & _PAGE_PRESENT)
#define pmd_clear(xp)	do { pmd_val(*(xp)) = 0; } while (0)

static inline int pte_write(pte_t pte) { return pte_val(pte) & _PAGE_WRITE; }
static inline int pte_exec(pte_t pte)  { return pte_val(pte) & _PAGE_EXEC; }
static inline int pte_dirty(pte_t pte) { return pte_val(pte) & _PAGE_DIRTY; }
static inline int pte_young(pte_t pte) { return pte_val(pte) & _PAGE_ACCESSED; }

/* Keep the TLB's writable bit equal to "writable and dirty". */
static inline pte_t pte_fix_hwwrite(pte_t pte)
{
	if ((pte_val(pte) & (_PAGE_WRITE | _PAGE_DIRTY)) ==
	    (_PAGE_WRITE | _PAGE_DIRTY))
		pte_val(pte) |= _PAGE_HWWRITE;
	else
		pte_val(pte) &= ~_PAGE_HWWRITE;
	return pte;
}

static inline pte_t pte_wrprotect(pte_t pte)
{
	pte_val(pte) &= ~_PAGE_WRITE;
	return pte_fix_hwwrite(pte);
}

static inline pte_t pte_mkclean(pte_t pte)
{
	pte_val(pte) &= ~_PAGE_DIRTY;
	return pte_fix_hwwrite(pte);
}

static inline pte_t pte_mkold(pte_t pte)
{
	pte_val(pte) &= ~_PAGE_ACCESSED;
	return pte;
}

static inline pte_t pte_mkwrite_novma(pte_t pte)
{
	pte_val(pte) |= _PAGE_WRITE;
	return pte_fix_hwwrite(pte);
}

static inline pte_t pte_mkdirty(pte_t pte)
{
	pte_val(pte) |= _PAGE_DIRTY;
	return pte_fix_hwwrite(pte);
}

static inline pte_t pte_mkyoung(pte_t pte)
{
	pte_val(pte) |= _PAGE_ACCESSED;
	return pte;
}

static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{
	pte_val(pte) = (pte_val(pte) & _PAGE_CHG_MASK) | pgprot_val(newprot);
	return pte_fix_hwwrite(pte);
}

#define pte_page(pte)		(pfn_to_page(pte_pfn(pte)))

#define pmd_pfn(pmd)		(pmd_val(pmd) >> PAGE_SHIFT)
#define pmd_page(pmd)		(pfn_to_page(pmd_val(pmd) >> PAGE_SHIFT))

static inline unsigned long pmd_page_vaddr(pmd_t pmd)
{
	return ((unsigned long) __va(pmd_val(pmd) & PAGE_MASK));
}

#define PFN_PTE_SHIFT		PAGE_SHIFT
#define pte_pfn(x)		((unsigned long)(((x).pte)) >> PAGE_SHIFT)
#define pfn_pte(pfn, prot) \
	pte_fix_hwwrite(__pte((((pfn) << PAGE_SHIFT)) | pgprot_val(prot)))

#define pte_ERROR(e) \
	printk(KERN_ERR "%s:%d: bad pte %p(%08lx).\n", \
	       __FILE__, __LINE__, &(e), pte_val(e))
#define pgd_ERROR(e) \
	printk(KERN_ERR "%s:%d: bad pgd %p(%08lx).\n", \
	       __FILE__, __LINE__, &(e), pgd_val(e))

extern pgd_t swapper_pg_dir[PTRS_PER_PGD];

struct vm_area_struct;
struct vm_fault;

/* The TLB is refilled on demand; nothing to preload. */
static inline void update_mmu_cache_range(struct vm_fault *vmf,
		struct vm_area_struct *vma, unsigned long address,
		pte_t *ptep, unsigned int nr)
{
}

#define update_mmu_cache(vma, addr, ptep) \
	update_mmu_cache_range(NULL, vma, addr, ptep, 1)

/*
 * A swap entry lives in a non-present pte: bit 0 clear, the exclusive
 * marker in bit 3, the type in bits 9:4 and the offset above that.
 */
#define __swp_type(x)			(((x).val >> 4) & 0x3f)
#define __swp_offset(x)			((x).val >> 12)
#define __swp_entry(type, offset) \
	((swp_entry_t) { (((type) & 0x3f) << 4) | ((offset) << 12) })
#define __pte_to_swp_entry(pte)		((swp_entry_t) { pte_val(pte) })
#define __swp_entry_to_pte(x)		((pte_t) { (x).val })

static inline bool pte_swp_exclusive(pte_t pte)
{
	return pte_val(pte) & _PAGE_SWP_EXCLUSIVE;
}

static inline pte_t pte_swp_mkexclusive(pte_t pte)
{
	pte_val(pte) |= _PAGE_SWP_EXCLUSIVE;
	return pte;
}

static inline pte_t pte_swp_clear_exclusive(pte_t pte)
{
	pte_val(pte) &= ~_PAGE_SWP_EXCLUSIVE;
	return pte;
}

typedef pte_t *pte_addr_t;

#endif /* __ASSEMBLER__ */
#endif /* __ASM_MOXIE_PGTABLE_H */
