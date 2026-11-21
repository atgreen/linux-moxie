// SPDX-License-Identifier: GPL-2.0

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include <linux/seq_file.h>
#include <linux/initrd.h>
#include <linux/of_fdt.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/pgtable.h>

#include <asm/sections.h>
#include <asm/setup.h>
#include <asm/mmu_context.h>

void __init early_init_devtree(void *params);

static void __init setup_memory(void)
{
	phys_addr_t memory_end = memblock_end_of_DRAM();

	if (!memory_end)
		panic("No memory!");

	/* The miss handler maps RAM by arithmetic up to the vmalloc area. */
	if (memory_end > VMALLOC_START - PAGE_OFFSET)
		panic("More memory than the kernel can map");

	min_low_pfn = PFN_UP(memblock_start_of_DRAM());
	max_low_pfn = PFN_DOWN(memory_end);
	max_pfn = max_low_pfn;

	/* The kernel, and everything below it. */
	memblock_reserve(0, __pa(_end));

#ifdef CONFIG_BLK_DEV_INITRD
	if (initrd_start && (initrd_end > initrd_start)) {
		unsigned long aligned_start = ALIGN_DOWN(initrd_start, PAGE_SIZE);
		unsigned long aligned_end = ALIGN(initrd_end, PAGE_SIZE);

		memblock_reserve(__pa(aligned_start), aligned_end - aligned_start);
	}
#endif

	early_init_fdt_reserve_self();
	early_init_fdt_scan_reserved_mem();
}

void __init early_init_devtree(void *params)
{
	early_init_dt_scan(params, __pa(params));
	memblock_allow_resize();
}

void calibrate_delay(void)
{
	struct device_node *cpu = of_get_cpu_node(0, NULL);
	u32 freq = 5000000;

	if (cpu)
		of_property_read_u32(cpu, "clock-frequency", &freq);
	of_node_put(cpu);

	/* A delay loop iteration is a handful of clocks; call it eight. */
	loops_per_jiffy = freq / 8 / HZ;
	pr_cont("%lu.%02lu BogoMIPS (lpj=%lu)\n",
		loops_per_jiffy / (500000 / HZ),
		(loops_per_jiffy / (5000 / HZ)) % 100, loops_per_jiffy);
}

void __init setup_arch(char **cmdline_p)
{
	early_init_devtree(__dtb_start);

	setup_memory();

	unflatten_and_copy_device_tree();

	setup_initial_init_mm(_stext, _etext, _edata, _end);

	paging_init();

	*cmdline_p = boot_command_line;
}

static int show_cpuinfo(struct seq_file *m, void *v)
{
	seq_puts(m, "cpu\t\t: mox125\n");
	seq_printf(m, "bogomips\t: %lu.%02lu\n",
		   (loops_per_jiffy * HZ) / 500000,
		   ((loops_per_jiffy * HZ) / 5000) % 100);
	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations cpuinfo_op = {
	.start = c_start,
	.next = c_next,
	.stop = c_stop,
	.show = show_cpuinfo,
};
