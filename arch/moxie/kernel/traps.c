// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/signal.h>
#include <linux/sched/task_stack.h>
#include <linux/kernel.h>
#include <linux/extable.h>
#include <linux/kallsyms.h>
#include <linux/ptrace.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/hardirq.h>
#include <linux/syscalls.h>
#include <linux/resume_user_mode.h>

#include <asm/ptrace.h>
#include <asm/moxie_sr.h>
#include <asm/unistd.h>
#include <asm/irq_regs.h>

void do_page_fault(struct pt_regs *regs);
void do_notify_resume(struct pt_regs *regs);
extern void *sys_call_table[];

void show_registers(struct pt_regs *regs)
{
	int i;

	pr_info("pc: %08lx  sr: %08lx  cause: %lu  ea: %08lx\n",
		regs->pc, regs->sr, regs->cause, regs->ea);
	pr_info("fp: %08lx  sp: %08lx\n", regs->fp, regs->sp);
	for (i = 0; i < 14; i += 2)
		pr_info("r%-2d: %08lx  r%-2d: %08lx\n",
			i, regs->gpr[i], i + 1, regs->gpr[i + 1]);
}

void show_stack(struct task_struct *task, unsigned long *sp, const char *loglvl)
{
	unsigned long addr;
	int i = 0;

	if (!sp)
		sp = (unsigned long *)current_stack_pointer;

	printk("%sCall trace:\n", loglvl);
	while (!kstack_end(sp) && i < 32) {
		addr = *sp++;
		if (__kernel_text_address(addr)) {
			printk("%s [<%08lx>] %pS\n", loglvl, addr, (void *)addr);
			i++;
		}
	}
}

void __noreturn die(const char *str, struct pt_regs *regs, long err)
{
	console_verbose();
	pr_emerg("\n%s#: %04lx\n", str, err & 0xffff);
	show_registers(regs);
	show_stack(NULL, (unsigned long *)regs->sp, KERN_EMERG);
	make_task_dead(SIGSEGV);
}

static void do_trap(struct pt_regs *regs, int signo, int code,
		    const char *what)
{
	if (user_mode(regs)) {
		force_sig_fault(signo, code, (void __user *)regs->pc);
		return;
	}
	die(what, regs, regs->cause);
}

static void do_syscall(struct pt_regs *regs)
{
	unsigned long nr = regs->gpr[PT_SYSCALLNO_REG];
	long (*fn)(unsigned long, unsigned long, unsigned long,
		   unsigned long, unsigned long, unsigned long);
	long ret = -ENOSYS;

	regs->orig_r0 = regs->gpr[0];
	regs->syscallno = nr;

	local_irq_enable();

	if (nr < __NR_syscalls) {
		fn = sys_call_table[nr];
		ret = fn(regs->gpr[0], regs->gpr[1], regs->gpr[2],
			 regs->gpr[3], regs->gpr[4], regs->gpr[5]);
	}
	regs->gpr[0] = ret;
}

/* Called from entry.S for every exception but a handled TLB miss. */
void moxie_do_exception(struct pt_regs *regs)
{
	struct pt_regs *old_regs;

	switch (regs->cause) {
	case CAUSE_IRQ:
		old_regs = set_irq_regs(regs);
		irq_enter();
		handle_arch_irq(regs);
		irq_exit();
		set_irq_regs(old_regs);
		break;
	case CAUSE_SWI:
		if (user_mode(regs))
			do_syscall(regs);
		else
			die("swi in kernel", regs, regs->cause);
		break;
	case CAUSE_ITLB_MISS:
	case CAUSE_DTLB_MISS_LOAD:
	case CAUSE_DTLB_MISS_STORE:
	case CAUSE_IPROT:
	case CAUSE_DPROT_LOAD:
	case CAUSE_DPROT_STORE:
		do_page_fault(regs);
		break;
	case CAUSE_DIV0:
		do_trap(regs, SIGFPE, FPE_INTDIV, "Divide by zero");
		break;
	case CAUSE_BRK:
		do_trap(regs, SIGTRAP, TRAP_BRKPT, "Breakpoint");
		break;
	case CAUSE_PRIV:
		do_trap(regs, SIGILL, ILL_PRVOPC, "Privileged instruction");
		break;
	case CAUSE_UNDEFINED:
	default:
		do_trap(regs, SIGILL, ILL_ILLOPC, "Undefined instruction");
		break;
	}
}

/*
 * Called from entry.S on the way out of every exception.  Returns
 * with interrupts off, and nothing left to do if we are going back to
 * user mode.
 */
void moxie_prepare_return(struct pt_regs *regs)
{
	unsigned long flags;

	local_irq_disable();
	if (!user_mode(regs))
		return;

	while ((flags = read_thread_flags()) & _TIF_WORK_MASK) {
		local_irq_enable();
		if (flags & _TIF_NEED_RESCHED)
			schedule();
		if (flags & (_TIF_SIGPENDING | _TIF_NOTIFY_SIGNAL
			     | _TIF_NOTIFY_RESUME))
			do_notify_resume(regs);
		local_irq_disable();
	}
}

void __init trap_init(void)
{
}
