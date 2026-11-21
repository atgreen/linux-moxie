// SPDX-License-Identifier: GPL-2.0
/*
 * Tracing other processes is not implemented beyond what the core
 * kernel needs in order to link.
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/regset.h>
#include <linux/elf.h>

#include <asm/ptrace.h>

static int genregs_get(struct task_struct *target,
		       const struct user_regset *regset,
		       struct membuf to)
{
	const struct pt_regs *regs = task_pt_regs(target);

	return membuf_write(&to, regs, sizeof(struct user_regs_struct));
}

static const struct user_regset moxie_regsets[] = {
	{
		USER_REGSET_NOTE_TYPE(PRSTATUS),
		.n = ELF_NGREG,
		.size = sizeof(long),
		.align = sizeof(long),
		.regset_get = genregs_get,
	},
};

static const struct user_regset_view user_moxie_native_view = {
	.name = "moxie",
	.e_machine = EM_MOXIE,
	.regsets = moxie_regsets,
	.n = ARRAY_SIZE(moxie_regsets),
};

const struct user_regset_view *task_user_regset_view(struct task_struct *task)
{
	return &user_moxie_native_view;
}

void ptrace_disable(struct task_struct *child)
{
}

long arch_ptrace(struct task_struct *child, long request, unsigned long addr,
		 unsigned long data)
{
	return ptrace_request(child, request, addr, data);
}
