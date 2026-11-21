/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_CACHEFLUSH_H
#define __ASM_MOXIE_CACHEFLUSH_H

#include <linux/mm.h>
#include <asm/moxie_sr.h>

/*
 * There is no data cache.  The instruction cache is tagged with
 * virtual addresses and knows nothing of what a page holds, so it has
 * to be emptied whenever code may have changed or a mapping of code
 * may have gone away.  It can only be emptied whole.
 */
static inline void moxie_flush_icache(void)
{
	mtsr(14, 0);
}

#define flush_icache_range(start, end)		moxie_flush_icache()
#define flush_icache_user_page(vma, page, addr, len) moxie_flush_icache()
#define flush_cache_mm(mm)			moxie_flush_icache()
#define flush_cache_dup_mm(mm)			moxie_flush_icache()
#define flush_cache_range(vma, start, end)	moxie_flush_icache()
#define flush_cache_page(vma, vmaddr, pfn)	moxie_flush_icache()

#include <asm-generic/cacheflush.h>

#endif /* __ASM_MOXIE_CACHEFLUSH_H */
