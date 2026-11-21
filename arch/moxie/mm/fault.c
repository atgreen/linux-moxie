// SPDX-License-Identifier: GPL-2.0
/*
 * Page faults.  Derived in outline from arch/openrisc/mm/fault.c.
 */

#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/extable.h>
#include <linux/sched/signal.h>
#include <linux/perf_event.h>
#include <linux/uaccess.h>
#include <linux/pgtable.h>

#include <asm/ptrace.h>
#include <asm/moxie_sr.h>
#include <asm/mmu_context.h>
#include <asm/tlbflush.h>

void __noreturn die(const char *str, struct pt_regs *regs, long err);
void do_page_fault(struct pt_regs *regs);

/*
 * The vmalloc area is mapped in the kernel's own page directory and
 * each address space gets a copy of that when it is created, so one
 * made earlier can be missing an entry added since.  Copy it over.
 */
static int vmalloc_fault(unsigned long address)
{
	pgd_t *pgd = (pgd_t *)__va(current_pgd_phys) + pgd_index(address);
	pgd_t *pgd_k = init_mm.pgd + pgd_index(address);
	pmd_t *pmd, *pmd_k;
	pte_t *pte_k;

	pmd = pmd_offset(pud_offset(p4d_offset(pgd, address), address), address);
	pmd_k = pmd_offset(pud_offset(p4d_offset(pgd_k, address), address),
			   address);
	if (!pmd_present(*pmd_k))
		return -1;
	set_pmd(pmd, *pmd_k);

	pte_k = pte_offset_kernel(pmd_k, address);
	if (!pte_present(*pte_k))
		return -1;
	return 0;
}

void do_page_fault(struct pt_regs *regs)
{
	unsigned long address = regs->ea;
	unsigned long cause = regs->cause;
	int write = cause == CAUSE_DTLB_MISS_STORE || cause == CAUSE_DPROT_STORE;
	int exec = cause == CAUSE_ITLB_MISS || cause == CAUSE_IPROT;
	struct task_struct *tsk = current;
	struct mm_struct *mm = tsk->mm;
	struct vm_area_struct *vma;
	int si_code = SEGV_MAPERR;
	vm_fault_t fault;
	unsigned int flags = FAULT_FLAG_DEFAULT;

	if (address >= VMALLOC_START && address < VMALLOC_END &&
	    !user_mode(regs)) {
		if (vmalloc_fault(address) == 0)
			return;
		goto no_context;
	}

	if (user_mode(regs)) {
		local_irq_enable();
		flags |= FAULT_FLAG_USER;
	} else if (regs->sr & ST_IE) {
		local_irq_enable();
	}

	if (faulthandler_disabled() || !mm)
		goto no_context;

	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, regs, address);

retry:
	vma = lock_mm_and_find_vma(mm, address, regs);
	if (!vma)
		goto bad_area_nosemaphore;

	si_code = SEGV_ACCERR;

	if (write) {
		if (!(vma->vm_flags & VM_WRITE))
			goto bad_area;
		flags |= FAULT_FLAG_WRITE;
	} else if (exec) {
		if (!(vma->vm_flags & VM_EXEC))
			goto bad_area;
		flags |= FAULT_FLAG_INSTRUCTION;
	} else {
		if (!(vma->vm_flags & (VM_READ | VM_EXEC | VM_WRITE)))
			goto bad_area;
	}

	fault = handle_mm_fault(vma, address, flags, regs);

	if (fault_signal_pending(fault, regs)) {
		if (!user_mode(regs))
			goto no_context;
		return;
	}

	/* The fault is fully completed (including releasing mmap lock) */
	if (fault & VM_FAULT_COMPLETED)
		return;

	if (unlikely(fault & VM_FAULT_ERROR)) {
		if (fault & VM_FAULT_OOM)
			goto out_of_memory;
		else if (fault & VM_FAULT_SIGSEGV)
			goto bad_area;
		else if (fault & VM_FAULT_SIGBUS)
			goto do_sigbus;
		BUG();
	}

	if (fault & VM_FAULT_RETRY) {
		flags |= FAULT_FLAG_TRIED;
		goto retry;
	}

	mmap_read_unlock(mm);

	/*
	 * The TLB may still hold what the page table used to say, such
	 * as a read-only entry for a page that is now writable.
	 */
	local_flush_tlb_page(vma, address);
	return;

bad_area:
	mmap_read_unlock(mm);

bad_area_nosemaphore:
	if (user_mode(regs)) {
		force_sig_fault(SIGSEGV, si_code, (void __user *)address);
		return;
	}

no_context:
	{
		const struct exception_table_entry *entry;

		entry = search_exception_tables(regs->pc);
		if (entry) {
			regs->pc = entry->fixup;
			return;
		}
	}

	if (address < PAGE_SIZE)
		pr_alert("Unable to handle kernel NULL pointer dereference");
	else
		pr_alert("Unable to handle kernel access");
	pr_cont(" at virtual address 0x%08lx\n", address);

	die("Oops", regs, cause);

out_of_memory:
	mmap_read_unlock(mm);
	if (!user_mode(regs))
		goto no_context;
	pagefault_out_of_memory();
	return;

do_sigbus:
	mmap_read_unlock(mm);
	force_sig_fault(SIGBUS, BUS_ADRERR, (void __user *)address);
	if (!user_mode(regs))
		goto no_context;
}
