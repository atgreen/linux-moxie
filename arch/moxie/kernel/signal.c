// SPDX-License-Identifier: GPL-2.0
/*
 * Signal delivery.  Derived in outline from arch/openrisc/kernel/signal.c.
 */

#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/resume_user_mode.h>

#include <asm/ptrace.h>
#include <asm/moxie_sr.h>
#include <asm/ucontext.h>
#include <asm/cacheflush.h>

void do_notify_resume(struct pt_regs *regs);

/* The ABI has every caller leave room for six argument registers. */
#define ARG_AREA	24

/* The condition codes are all of the status a program may change. */
#define ST_USER_MASK	0x1f0

/* A swi is six bytes; this is how far back the one we are in is. */
#define SWI_SIZE	6

/*
 * What a handler finds on its stack.  It is entered as if it had been
 * called: $fp and $sp are at "caller_fp", which is what "ret" unwinds.
 * That returns into retcode with $sp at "args", and retcode asks the
 * kernel to put everything back from the rest.
 */
struct rt_sigframe {
	unsigned long caller_fp;
	unsigned long return_address;
	unsigned long static_chain;
	unsigned long args[ARG_AREA / sizeof(unsigned long)];
	struct siginfo info;
	struct ucontext uc;
	unsigned short retcode[6];
};

#define frame_from_sp(sp) \
	((struct rt_sigframe __user *)((sp) - offsetof(struct rt_sigframe, info)))

static int restore_sigcontext(struct pt_regs *regs,
			      struct sigcontext __user *sc)
{
	unsigned long old_sr = regs->sr;
	int err;

	/* Always make any pending restarted system calls return -EINTR */
	current->restart_block.fn = do_no_restart_syscall;

	/* pt_regs begins with a user_regs_struct. */
	err = __copy_from_user(regs, &sc->regs, sizeof(sc->regs));

	regs->sr = (old_sr & ~ST_USER_MASK) | (regs->sr & ST_USER_MASK);
	regs->syscallno = -1;	/* this is not a system call to restart */

	return err;
}

SYSCALL_DEFINE0(rt_sigreturn)
{
	struct pt_regs *regs = current_pt_regs();
	struct rt_sigframe __user *frame = frame_from_sp(regs->sp + ARG_AREA);
	sigset_t set;

	if (((unsigned long)frame) & 3)
		goto badframe;
	if (!access_ok(frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&set, &frame->uc.uc_sigmask, sizeof(set)))
		goto badframe;

	set_current_blocked(&set);

	if (restore_sigcontext(regs, &frame->uc.uc_mcontext))
		goto badframe;
	if (restore_altstack(&frame->uc.uc_stack))
		goto badframe;

	/* The caller stores this in $r0, which we have just restored. */
	return regs->gpr[0];

badframe:
	force_sig(SIGSEGV);
	return 0;
}

static int setup_rt_frame(struct ksignal *ksig, sigset_t *set,
			  struct pt_regs *regs)
{
	struct rt_sigframe __user *frame;
	unsigned long sp;
	int err = 0;

	sp = sigsp(regs->sp, ksig);
	frame = (struct rt_sigframe __user *)((sp - sizeof(*frame)) & ~3UL);

	if (!access_ok(frame, sizeof(*frame)))
		return -EFAULT;

	if (ksig->ka.sa.sa_flags & SA_SIGINFO)
		err |= copy_siginfo_to_user(&frame->info, &ksig->info);

	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(NULL, &frame->uc.uc_link);
	err |= __save_altstack(&frame->uc.uc_stack, regs->sp);
	err |= __copy_to_user(&frame->uc.uc_mcontext.regs, regs,
			      sizeof(frame->uc.uc_mcontext.regs));
	err |= __copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));

	/*
	 * The way back:
	 *	ldi.l	$r12, __NR_rt_sigreturn
	 *	swi	0
	 */
	err |= __put_user(0x01e0, &frame->retcode[0]);
	err |= __put_user(0, &frame->retcode[1]);
	err |= __put_user(__NR_rt_sigreturn, &frame->retcode[2]);
	err |= __put_user(0x3000, &frame->retcode[3]);
	err |= __put_user(0, &frame->retcode[4]);
	err |= __put_user(0, &frame->retcode[5]);

	err |= __put_user(regs->fp, &frame->caller_fp);
	err |= __put_user((unsigned long)frame->retcode,
			  &frame->return_address);
	err |= __put_user(0, &frame->static_chain);

	if (err)
		return -EFAULT;

	/* Whatever was last fetched from there, it was not this. */
	moxie_flush_icache();

	regs->pc = (unsigned long)ksig->ka.sa.sa_handler;
	regs->fp = (unsigned long)frame;
	regs->sp = (unsigned long)frame;
	regs->gpr[0] = ksig->sig;
	regs->gpr[1] = (unsigned long)&frame->info;
	regs->gpr[2] = (unsigned long)&frame->uc;

	return 0;
}

static void handle_signal(struct ksignal *ksig, struct pt_regs *regs)
{
	int ret;

	ret = setup_rt_frame(ksig, sigmask_to_save(), regs);

	signal_setup_done(ret, ksig, 0);
}

static void do_signal(struct pt_regs *regs)
{
	struct ksignal ksig;
	unsigned long restart_pc = 0;
	unsigned long continue_pc = 0;
	long retval = 0;
	int restart = 0;

	/*
	 * If we were in a system call that wants restarting, set up to
	 * restart it now, and undo that below if a handler runs that
	 * says otherwise.
	 */
	if (regs->syscallno >= 0) {
		continue_pc = regs->pc;
		restart_pc = regs->pc - SWI_SIZE;
		retval = regs->gpr[0];

		switch (retval) {
		case -ERESTART_RESTARTBLOCK:
			restart = -2;
			fallthrough;
		case -ERESTARTNOHAND:
		case -ERESTARTSYS:
		case -ERESTARTNOINTR:
			restart++;
			regs->gpr[0] = regs->orig_r0;
			regs->pc = restart_pc;
			break;
		}
		regs->syscallno = -1;
	}

	if (get_signal(&ksig)) {
		if (unlikely(restart) && regs->pc == restart_pc) {
			if (retval == -ERESTARTNOHAND ||
			    retval == -ERESTART_RESTARTBLOCK ||
			    (retval == -ERESTARTSYS &&
			     !(ksig.ka.sa.sa_flags & SA_RESTART))) {
				regs->gpr[0] = -EINTR;
				regs->pc = continue_pc;
			}
		}
		handle_signal(&ksig, regs);
		return;
	}

	/* No handler: -ERESTART_RESTARTBLOCK restarts by another route. */
	if (restart < 0 && regs->pc == restart_pc)
		regs->gpr[PT_SYSCALLNO_REG] = __NR_restart_syscall;

	restore_saved_sigmask();
}

void do_notify_resume(struct pt_regs *regs)
{
	if (test_thread_flag(TIF_SIGPENDING) ||
	    test_thread_flag(TIF_NOTIFY_SIGNAL))
		do_signal(regs);

	if (test_thread_flag(TIF_NOTIFY_RESUME))
		resume_user_mode_work(regs);
}
