// SPDX-License-Identifier: GPL-2.0
/*
 * Delay loops.  There is no cycle counter in the core, so these spin.
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/param.h>
#include <asm/processor.h>

void __delay(unsigned long loops)
{
	while (loops--)
		barrier();
}
EXPORT_SYMBOL(__delay);

void __const_udelay(unsigned long xloops)
{
	unsigned long long loops;

	loops = (unsigned long long)xloops * loops_per_jiffy * HZ;

	__delay(loops >> 32);
}
EXPORT_SYMBOL(__const_udelay);

void __udelay(unsigned long usecs)
{
	__const_udelay(usecs * 0x10C7UL); /* 2**32 / 1000000 (rounded up) */
}
EXPORT_SYMBOL(__udelay);

void __ndelay(unsigned long nsecs)
{
	__const_udelay(nsecs * 0x5UL); /* 2**32 / 1000000000 (rounded up) */
}
EXPORT_SYMBOL(__ndelay);
