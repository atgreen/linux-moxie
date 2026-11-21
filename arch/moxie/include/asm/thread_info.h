/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_MOXIE_THREAD_INFO_H
#define _ASM_MOXIE_THREAD_INFO_H

#ifdef __KERNEL__

#ifndef __ASSEMBLER__
#include <asm/types.h>
#include <asm/processor.h>
#endif

#define THREAD_SIZE_ORDER 1
#define THREAD_SIZE       (PAGE_SIZE << THREAD_SIZE_ORDER)

#ifndef __ASSEMBLER__

struct thread_info {
	struct task_struct	*task;		/* main task structure */
	unsigned long		flags;		/* low level flags */
	__u32			cpu;		/* current CPU */
	__s32			preempt_count; /* 0 => preemptable, <0 => BUG */
};

#define INIT_THREAD_INFO(tsk)				\
{							\
	.task		= &tsk,				\
	.flags		= 0,				\
	.cpu		= 0,				\
	.preempt_count	= INIT_PREEMPT_COUNT,		\
}

register unsigned long current_stack_pointer asm("$sp");

/* The thread_info is at the bottom of the kernel stack we are on. */
static inline struct thread_info *current_thread_info(void)
{
	return (struct thread_info *)
		(current_stack_pointer & ~(THREAD_SIZE - 1));
}

#endif /* !__ASSEMBLER__ */

#define TIF_SYSCALL_TRACE	0	/* syscall trace active */
#define TIF_NOTIFY_RESUME	1	/* resumption notification requested */
#define TIF_SIGPENDING		2	/* signal pending */
#define TIF_NEED_RESCHED	3	/* rescheduling necessary */
#define TIF_NOTIFY_SIGNAL	5	/* signal notifications exist */
#define TIF_RESTORE_SIGMASK     9
#define TIF_POLLING_NRFLAG	16
#define TIF_MEMDIE              17

#define _TIF_SYSCALL_TRACE	(1<<TIF_SYSCALL_TRACE)
#define _TIF_NOTIFY_RESUME	(1<<TIF_NOTIFY_RESUME)
#define _TIF_SIGPENDING		(1<<TIF_SIGPENDING)
#define _TIF_NEED_RESCHED	(1<<TIF_NEED_RESCHED)
#define _TIF_NOTIFY_SIGNAL	(1<<TIF_NOTIFY_SIGNAL)
#define _TIF_POLLING_NRFLAG	(1<<TIF_POLLING_NRFLAG)

/* Work to do before going back to user mode. */
#define _TIF_WORK_MASK	(_TIF_NOTIFY_RESUME | _TIF_SIGPENDING \
			 | _TIF_NEED_RESCHED | _TIF_NOTIFY_SIGNAL)

#endif /* __KERNEL__ */

#endif /* _ASM_MOXIE_THREAD_INFO_H */
