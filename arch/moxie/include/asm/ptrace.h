/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_PTRACE_H
#define __ASM_MOXIE_PTRACE_H

#include <asm/moxie_sr.h>
#include <uapi/asm/ptrace.h>

#ifndef __ASSEMBLER__
/*
 * What the exception entry saves on the kernel stack.  The first part
 * is laid out like user_regs_struct.
 */
struct pt_regs {
	unsigned long fp;
	unsigned long sp;
	unsigned long gpr[14];		/* $r0 to $r13 */
	unsigned long pc;
	unsigned long sr;
	unsigned long cause;		/* why we are here */
	unsigned long ea;		/* address that faulted, if one did */
	unsigned long orig_r0;		/* $r0 as a system call was entered */
	long syscallno;			/* -1 unless in a system call */
};

/* System calls: number in $r12, arguments in $r0 to $r5, result in $r0. */
#define PT_SYSCALLNO_REG	12

#define user_mode(regs)			(((regs)->sr & ST_UM) != 0)
#define instruction_pointer(regs)	((regs)->pc)
#define user_stack_pointer(regs)	((regs)->sp)
#define profile_pc(regs)		instruction_pointer(regs)
#define regs_return_value(regs)		((regs)->gpr[0])

static inline void instruction_pointer_set(struct pt_regs *regs,
					   unsigned long val)
{
	regs->pc = val;
}

#endif /* __ASSEMBLER__ */

#endif /* __ASM_MOXIE_PTRACE_H */
