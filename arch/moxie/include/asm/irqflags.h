/* SPDX-License-Identifier: GPL-2.0 */
#ifndef ___ASM_MOXIE_IRQFLAGS_H
#define ___ASM_MOXIE_IRQFLAGS_H

#include <asm/moxie_sr.h>

/*
 * Writing the status register sets the condition codes as well as the
 * interrupt enable.  So each of these reads the status and writes it
 * back within one asm statement, where the compiler cannot put a
 * compare in between, and only the interrupt enable is ever returned
 * or passed around.
 */

static inline unsigned long arch_local_save_flags(void)
{
	return mfsr(0) & ST_IE;
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long status, masked;

	__asm__ __volatile__("gsr %0, 0\n\t"
			     "mov %1, %0\n\t"
			     "and %1, %2\n\t"
			     "ssr %1, 0"
			     : "=&r" (status), "=&r" (masked)
			     : "r" (~(unsigned long)ST_IE));
	return status & ST_IE;
}

static inline void arch_local_irq_enable(void)
{
	unsigned long status;

	__asm__ __volatile__("gsr %0, 0\n\t"
			     "or %0, %1\n\t"
			     "ssr %0, 0"
			     : "=&r" (status)
			     : "r" ((unsigned long)ST_IE));
}

static inline void arch_local_irq_disable(void)
{
	(void)arch_local_irq_save();
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	if (flags & ST_IE)
		arch_local_irq_enable();
	else
		arch_local_irq_disable();
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return !(flags & ST_IE);
}

static inline int arch_irqs_disabled(void)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

#endif /* ___ASM_MOXIE_IRQFLAGS_H */
