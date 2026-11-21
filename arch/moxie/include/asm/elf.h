/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MOXIE_ELF_H
#define __ASM_MOXIE_ELF_H

#include <linux/types.h>
#include <asm/ptrace.h>

#define ELF_NGREG	(sizeof(struct user_regs_struct) / sizeof(unsigned long))
typedef unsigned long elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREG];
typedef unsigned long elf_fpregset_t;

/* Not in <linux/elf-em.h>: the official number, and the one used before
 * it was assigned. */
#ifndef EM_MOXIE
#define EM_MOXIE	223
#define EM_MOXIE_OLD	0xFEED
#endif

#define ELF_ARCH	EM_MOXIE
#define ELF_CLASS	ELFCLASS32
#define ELF_DATA	ELFDATA2MSB

#define elf_check_arch(x) \
	(((x)->e_machine == EM_MOXIE) || ((x)->e_machine == EM_MOXIE_OLD))

/* Where the dynamic loader goes when the kernel runs it directly. */
#define ELF_ET_DYN_BASE         (0x08000000)

#define ELF_EXEC_PAGESIZE	PAGE_SIZE

#define ELF_HWCAP	(0)
#define ELF_PLATFORM	(NULL)

#endif
