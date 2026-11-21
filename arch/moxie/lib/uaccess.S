/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copying to and from user space, a byte at a time.  A fault on
 * either side lands on the fixup through the exception table, with
 * the count of bytes still to do in $r2.
 */

#include <linux/linkage.h>

/* unsigned long __copy_tofrom_user(void *to, const void *from,
 *				    unsigned long size) */
ENTRY(__copy_tofrom_user)
	xor	$r3, $r3
1:	cmp	$r2, $r3
	beq	3f
10:	ld.b	$r4, ($r1)
11:	st.b	($r0), $r4
	inc	$r0, 1
	inc	$r1, 1
	dec	$r2, 1
	jmpa	1b
3:	mov	$r0, $r2
	ret
END(__copy_tofrom_user)

/* unsigned long __clear_user(void *addr, unsigned long size) */
ENTRY(__clear_user)
	xor	$r3, $r3
1:	cmp	$r1, $r3
	beq	3f
20:	st.b	($r0), $r3
	inc	$r0, 1
	dec	$r1, 1
	jmpa	1b
3:	mov	$r0, $r1
	ret
END(__clear_user)

.Lcopy_fault:
	mov	$r0, $r2
	ret
.Lclear_fault:
	mov	$r0, $r1
	ret

	.section __ex_table, "a"
	.balign	4
	.long	10b, .Lcopy_fault
	.long	11b, .Lcopy_fault
	.long	20b, .Lclear_fault
	.previous
