/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_IO_H
#define __ASM_MOXIE_IO_H

#include <linux/types.h>
#include <asm/pgtable.h>

/*
 * Devices live at the top of the physical address space, and the TLB
 * miss handler maps that region one to one for the kernel.  So there
 * is nothing to set up.
 */
static inline void __iomem *ioremap(phys_addr_t offset, size_t size)
{
	if (offset < IO_BASE)
		return NULL;
	return (void __iomem *)(unsigned long)offset;
}
#define ioremap ioremap

static inline void iounmap(volatile void __iomem *addr)
{
}
#define iounmap iounmap

#define PIO_RESERVED		0X0UL
#define PIO_OFFSET		0
#define PIO_MASK		0

#include <asm-generic/io.h>

#endif
