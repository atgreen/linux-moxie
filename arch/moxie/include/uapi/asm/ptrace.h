/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_ASM_MOXIE_PTRACE_H
#define _UAPI_ASM_MOXIE_PTRACE_H

#ifndef __ASSEMBLER__
/*
 * The register state as user space sees it, in the order the core
 * numbers its registers: $fp, $sp, then $r0 to $r13.
 */
struct user_regs_struct {
	unsigned long fp;
	unsigned long sp;
	unsigned long gpr[14];
	unsigned long pc;
	unsigned long sr;
};
#endif

#endif /* _UAPI_ASM_MOXIE_PTRACE_H */
