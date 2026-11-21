/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_PROCESSOR_H
#define __ASM_MOXIE_PROCESSOR_H

#include <asm/page.h>
#include <asm/ptrace.h>

#define TASK_SIZE       (0x80000000UL)
#define TASK_UNMAPPED_BASE      (TASK_SIZE / 8 * 3)

#define STACK_TOP       TASK_SIZE
#define STACK_TOP_MAX	STACK_TOP

#ifndef __ASSEMBLER__

struct task_struct;

/* What __switch_to saves and restores; see entry.S. */
struct thread_struct {
	unsigned long fp;
	unsigned long gpr6_11[6];
};

#define INIT_THREAD  { }

#define task_pt_regs(task) \
	((struct pt_regs *)(task_stack_page(task) + THREAD_SIZE) - 1)

#define KSTK_EIP(tsk)   (task_pt_regs(tsk)->pc)
#define KSTK_ESP(tsk)   (task_pt_regs(tsk)->sp)

void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp);
unsigned long __get_wchan(struct task_struct *p);
void show_registers(struct pt_regs *regs);

#define cpu_relax()     barrier()

#endif /* __ASSEMBLER__ */
#endif /* __ASM_MOXIE_PROCESSOR_H */
