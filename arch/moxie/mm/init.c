// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/memblock.h>
#include <linux/init.h>
#include <linux/pgtable.h>

#include <asm/pgalloc.h>
#include <asm/mmu_context.h>
#include <asm/tlbflush.h>
#include <asm/sections.h>

pgd_t swapper_pg_dir[PTRS_PER_PGD] __page_aligned_bss;

void __init arch_zone_limits_init(unsigned long *max_zone_pfns)
{
	max_zone_pfns[ZONE_NORMAL] = max_low_pfn;
}

/*
 * The kernel's view of RAM needs no page tables: the TLB miss handler
 * maps it by arithmetic.  The tables are only for the vmalloc area
 * and for user space.
 */
void __init paging_init(void)
{
	current_pgd_phys = __pa(swapper_pg_dir);
	flush_tlb_all();
}

void __init mem_init(void)
{
}

static const pgprot_t protection_map[16] = {
	[VM_NONE]					= PAGE_NONE,
	[VM_READ]					= PAGE_READONLY_X,
	[VM_WRITE]					= PAGE_COPY,
	[VM_WRITE | VM_READ]				= PAGE_COPY_X,
	[VM_EXEC]					= PAGE_READONLY_X,
	[VM_EXEC | VM_READ]				= PAGE_READONLY_X,
	[VM_EXEC | VM_WRITE]				= PAGE_COPY_X,
	[VM_EXEC | VM_WRITE | VM_READ]			= PAGE_COPY_X,
	[VM_SHARED]					= PAGE_NONE,
	[VM_SHARED | VM_READ]				= PAGE_READONLY_X,
	[VM_SHARED | VM_WRITE]				= PAGE_SHARED,
	[VM_SHARED | VM_WRITE | VM_READ]		= PAGE_SHARED_X,
	[VM_SHARED | VM_EXEC]				= PAGE_READONLY_X,
	[VM_SHARED | VM_EXEC | VM_READ]			= PAGE_READONLY_X,
	[VM_SHARED | VM_EXEC | VM_WRITE]		= PAGE_SHARED_X,
	[VM_SHARED | VM_EXEC | VM_WRITE | VM_READ]	= PAGE_SHARED_X
};
DECLARE_VM_GET_PAGE_PROT
