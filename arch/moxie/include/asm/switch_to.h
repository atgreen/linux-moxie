/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_SWITCH_TO_H
#define __ASM_MOXIE_SWITCH_TO_H

struct task_struct;

extern struct task_struct *__switch_to(struct task_struct *prev,
				       struct task_struct *next);

#define switch_to(prev, next, last)	((last) = __switch_to((prev), (next)))

#endif /* __ASM_MOXIE_SWITCH_TO_H */
