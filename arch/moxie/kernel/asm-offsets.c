// SPDX-License-Identifier: GPL-2.0
/*
 * Generate definitions needed by assembly language modules.
 */

#define COMPILE_OFFSETS

#include <linux/kbuild.h>
#include <linux/sched.h>
#include <linux/thread_info.h>
#include <linux/mm.h>
#include <asm/ptrace.h>
#include <asm/processor.h>

int main(void)
{
	DEFINE(PT_FP, offsetof(struct pt_regs, fp));
	DEFINE(PT_SP, offsetof(struct pt_regs, sp));
	DEFINE(PT_R0, offsetof(struct pt_regs, gpr[0]));
	DEFINE(PT_R1, offsetof(struct pt_regs, gpr[1]));
	DEFINE(PT_R2, offsetof(struct pt_regs, gpr[2]));
	DEFINE(PT_R3, offsetof(struct pt_regs, gpr[3]));
	DEFINE(PT_R4, offsetof(struct pt_regs, gpr[4]));
	DEFINE(PT_R5, offsetof(struct pt_regs, gpr[5]));
	DEFINE(PT_R6, offsetof(struct pt_regs, gpr[6]));
	DEFINE(PT_R7, offsetof(struct pt_regs, gpr[7]));
	DEFINE(PT_R8, offsetof(struct pt_regs, gpr[8]));
	DEFINE(PT_R9, offsetof(struct pt_regs, gpr[9]));
	DEFINE(PT_R10, offsetof(struct pt_regs, gpr[10]));
	DEFINE(PT_R11, offsetof(struct pt_regs, gpr[11]));
	DEFINE(PT_R12, offsetof(struct pt_regs, gpr[12]));
	DEFINE(PT_R13, offsetof(struct pt_regs, gpr[13]));
	DEFINE(PT_PC, offsetof(struct pt_regs, pc));
	DEFINE(PT_SR, offsetof(struct pt_regs, sr));
	DEFINE(PT_CAUSE, offsetof(struct pt_regs, cause));
	DEFINE(PT_EA, offsetof(struct pt_regs, ea));
	DEFINE(PT_ORIG_R0, offsetof(struct pt_regs, orig_r0));
	DEFINE(PT_SYSCALLNO, offsetof(struct pt_regs, syscallno));
	DEFINE(PT_SIZE, sizeof(struct pt_regs));

	DEFINE(TASK_THREAD, offsetof(struct task_struct, thread));
	DEFINE(TASK_STACK, offsetof(struct task_struct, stack));

	DEFINE(THREAD_FP, offsetof(struct thread_struct, fp));
	DEFINE(THREAD_R6, offsetof(struct thread_struct, gpr6_11[0]));
	DEFINE(THREAD_R7, offsetof(struct thread_struct, gpr6_11[1]));
	DEFINE(THREAD_R8, offsetof(struct thread_struct, gpr6_11[2]));
	DEFINE(THREAD_R9, offsetof(struct thread_struct, gpr6_11[3]));
	DEFINE(THREAD_R10, offsetof(struct thread_struct, gpr6_11[4]));
	DEFINE(THREAD_R11, offsetof(struct thread_struct, gpr6_11[5]));

	return 0;
}
