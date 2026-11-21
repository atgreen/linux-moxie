// SPDX-License-Identifier: GPL-2.0

#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/unistd.h>

#include <asm/syscalls.h>

#define __SYSCALL(nr, call) [nr] = (call),
#define __SYSCALL_WITH_COMPAT(nr, native, compat) __SYSCALL(nr, native)

#define sys_mmap2 sys_mmap_pgoff

void *sys_call_table[__NR_syscalls] = {
#include <asm/syscall_table_32.h>
};
