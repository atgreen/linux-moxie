/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The kernel is linked to run at PAGE_OFFSET and loaded at physical
 * address zero, with its entry point at 0x1000 where the core starts
 * executing.
 */

#define LOAD_OFFSET	PAGE_OFFSET

#include <asm/page.h>
#include <asm/cache.h>
#include <asm/thread_info.h>
#include <asm-generic/vmlinux.lds.h>

OUTPUT_FORMAT("elf32-bigmoxie", "elf32-bigmoxie", "elf32-bigmoxie")
ENTRY(_start_phys)

jiffies = jiffies_64 + 4;

SECTIONS
{
	. = PAGE_OFFSET + 0x1000;
	_text = .;
	_start_phys = _start - LOAD_OFFSET;

	.text : AT(ADDR(.text) - LOAD_OFFSET)
	{
		_stext = .;
		HEAD_TEXT
		TEXT_TEXT
		SCHED_TEXT
		LOCK_TEXT
		KPROBES_TEXT
		IRQENTRY_TEXT
		SOFTIRQENTRY_TEXT
		_etext = .;
	}

	_sdata = .;

	RO_DATA(PAGE_SIZE)

	RW_DATA(32, PAGE_SIZE, THREAD_SIZE)

	_edata  =  .;

	EXCEPTION_TABLE(4)

	. = ALIGN(PAGE_SIZE);
	__init_begin = .;

	INIT_TEXT_SECTION(PAGE_SIZE)
	INIT_DATA_SECTION(16)
	PERCPU_SECTION(L1_CACHE_BYTES)

	__init_end = .;

	BSS_SECTION(0, 0, 0x20)

	_end = .;

	STABS_DEBUG
	DWARF_DEBUG
	MODINFO
	ELF_DETAILS

	DISCARDS
}
