/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_SYSCALL_H__
#define __ASM_MOXIE_SYSCALL_H__

#include <uapi/linux/audit.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <asm/elf.h>

/* Big-endian, 32-bit: no flags. */
#define AUDIT_ARCH_MOXIE	EM_MOXIE

static inline int
syscall_get_nr(struct task_struct *task, struct pt_regs *regs)
{
	return regs->syscallno;
}

static inline void
syscall_set_nr(struct task_struct *task, struct pt_regs *regs, int nr)
{
	regs->syscallno = nr;
}

static inline void
syscall_rollback(struct task_struct *task, struct pt_regs *regs)
{
	regs->gpr[0] = regs->orig_r0;
}

static inline long
syscall_get_error(struct task_struct *task, struct pt_regs *regs)
{
	return IS_ERR_VALUE(regs->gpr[0]) ? regs->gpr[0] : 0;
}

static inline long
syscall_get_return_value(struct task_struct *task, struct pt_regs *regs)
{
	return regs->gpr[0];
}

static inline void
syscall_set_return_value(struct task_struct *task, struct pt_regs *regs,
			 int error, long val)
{
	regs->gpr[0] = (long) error ?: val;
}

static inline void
syscall_get_arguments(struct task_struct *task, struct pt_regs *regs,
		      unsigned long *args)
{
	args[0] = regs->orig_r0;
	memcpy(args + 1, &regs->gpr[1], 5 * sizeof(args[0]));
}

static inline void
syscall_set_arguments(struct task_struct *task, struct pt_regs *regs,
		      const unsigned long *args)
{
	memcpy(&regs->gpr[0], args, 6 * sizeof(args[0]));
}

static inline int syscall_get_arch(struct task_struct *task)
{
	return AUDIT_ARCH_MOXIE;
}

#endif
