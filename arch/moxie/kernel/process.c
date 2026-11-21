// SPDX-License-Identifier: GPL-2.0

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/task.h>
#include <linux/sched/task_stack.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <linux/ptrace.h>
#include <linux/elfcore.h>
#include <linux/init_task.h>
#include <linux/reboot.h>
#include <linux/io.h>
#include <linux/pm.h>

#include <asm/processor.h>
#include <asm/moxie_sr.h>

extern asmlinkage void ret_from_fork(void);

/* The ABI has every caller leave room for six argument registers. */
#define ARG_AREA	24

/* A write here ends a simulation. */
#define SIM_EXIT	((void __iomem *)0xf00000f0)

void machine_restart(char *cmd)
{
	do_kernel_restart(cmd);
	pr_emerg("Reboot failed -- System halted\n");
	__raw_writel(1, SIM_EXIT);
	while (1)
		;
}

void machine_halt(void)
{
	pr_info("*** MACHINE HALT ***\n");
	__raw_writel(0, SIM_EXIT);
	while (1)
		;
}

void machine_power_off(void)
{
	pr_info("*** MACHINE POWER OFF ***\n");
	do_kernel_power_off();
	__raw_writel(0, SIM_EXIT);
	while (1)
		;
}

static void moxie_sim_power_off(void)
{
	__raw_writel(0, SIM_EXIT);
}

void (*pm_power_off)(void) = moxie_sim_power_off;
EXPORT_SYMBOL(pm_power_off);

/*
 * wfi wakes for a pending interrupt even while interrupts are
 * disabled, which is how we are called, so nothing can be missed.
 */
void arch_cpu_idle(void)
{
	__asm__ __volatile__("wfi");
}

void flush_thread(void)
{
}

void show_regs(struct pt_regs *regs)
{
	show_regs_print_info(KERN_DEFAULT);
	show_registers(regs);
}

/*
 * A task that is not running is parked inside __switch_to(), and comes
 * back when that executes "ret", which takes what it needs from the
 * frame at $fp: the caller's $fp, the return address, and the static
 * chain slot.  A new task is given one of those, returning into
 * ret_from_fork with $sp just below its saved user registers.
 */
struct switch_frame {
	unsigned long fp;
	unsigned long return_address;
	unsigned long static_chain;
};

int copy_thread(struct task_struct *p, const struct kernel_clone_args *args)
{
	struct pt_regs *regs = task_pt_regs(p);
	struct switch_frame *frame;

	frame = (struct switch_frame *)((unsigned long)regs - ARG_AREA) - 1;
	frame->fp = 0;
	frame->return_address = (unsigned long)ret_from_fork;
	frame->static_chain = 0;

	memset(&p->thread, 0, sizeof(p->thread));
	p->thread.fp = (unsigned long)frame;

	if (unlikely(args->fn)) {
		/* A kernel thread: ret_from_fork calls fn(fn_arg). */
		memset(regs, 0, sizeof(*regs));
		regs->sr = ST_MMU | ST_IE;
		regs->syscallno = -1;
		p->thread.gpr6_11[0] = (unsigned long)args->fn;
		p->thread.gpr6_11[1] = (unsigned long)args->fn_arg;
	} else {
		*regs = *current_pt_regs();
		if (args->stack)
			regs->sp = args->stack;
		regs->gpr[0] = 0;	/* fork() returns 0 in the child */
	}

	return 0;
}

void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp)
{
	memset(regs, 0, sizeof(struct pt_regs));
	regs->pc = pc;
	regs->sp = sp;
	regs->sr = ST_MMU | ST_UM | ST_IE;
	regs->syscallno = -1;
}

unsigned long __get_wchan(struct task_struct *p)
{
	return 0;
}
