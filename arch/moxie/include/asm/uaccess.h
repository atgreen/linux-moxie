/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_UACCESS_H
#define __ASM_MOXIE_UACCESS_H

#include <linux/string.h>
#include <asm/page.h>
#include <asm/extable.h>
#include <asm-generic/access_ok.h>

/*
 * The kernel reaches user memory through the user's own mappings.
 * These return the number of bytes they could not copy; a fault part
 * way through is caught through the exception table.
 */
extern unsigned long __copy_tofrom_user(void *to, const void *from,
					unsigned long size);

static inline unsigned long
raw_copy_from_user(void *to, const void __user *from, unsigned long size)
{
	return __copy_tofrom_user(to, (__force const void *)from, size);
}

static inline unsigned long
raw_copy_to_user(void __user *to, const void *from, unsigned long size)
{
	return __copy_tofrom_user((__force void *)to, from, size);
}
#define INLINE_COPY_FROM_USER
#define INLINE_COPY_TO_USER

extern unsigned long __clear_user(void __user *addr, unsigned long size);
#define __clear_user __clear_user

#include <asm-generic/uaccess.h>

#endif /* __ASM_MOXIE_UACCESS_H */
