/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_ASM_MOXIE_SIGCONTEXT_H
#define _UAPI_ASM_MOXIE_SIGCONTEXT_H

#include <asm/ptrace.h>

struct sigcontext {
	struct user_regs_struct regs;
	unsigned long oldmask;
};

#endif /* _UAPI_ASM_MOXIE_SIGCONTEXT_H */
