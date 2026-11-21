// SPDX-License-Identifier: GPL-2.0

#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>

/*
 * The simulated board's interrupt controller.  The core has a single
 * interrupt input; this says which sources want it and lets each be
 * masked.  Registers are native big-endian words.
 */
#define PIC_PENDING	0
#define PIC_ENABLE	4

static void __iomem *pic_base;
static struct irq_domain *pic_domain;

static void pic_mask(struct irq_data *d)
{
	__raw_writel(__raw_readl(pic_base + PIC_ENABLE) & ~BIT(d->hwirq),
		     pic_base + PIC_ENABLE);
}

static void pic_unmask(struct irq_data *d)
{
	__raw_writel(__raw_readl(pic_base + PIC_ENABLE) | BIT(d->hwirq),
		     pic_base + PIC_ENABLE);
}

static struct irq_chip pic_chip = {
	.name		= "moxie-sim-pic",
	.irq_mask	= pic_mask,
	.irq_unmask	= pic_unmask,
};

static int pic_map(struct irq_domain *d, unsigned int irq,
		   irq_hw_number_t hw)
{
	irq_set_chip_and_handler(irq, &pic_chip, handle_level_irq);
	return 0;
}

static const struct irq_domain_ops pic_domain_ops = {
	.map	= pic_map,
	.xlate	= irq_domain_xlate_onecell,
};

static void pic_handle_irq(struct pt_regs *regs)
{
	u32 pending;

	while ((pending = __raw_readl(pic_base + PIC_PENDING) &
			  __raw_readl(pic_base + PIC_ENABLE)) != 0)
		generic_handle_domain_irq(pic_domain, __ffs(pending));
}

static int __init pic_init(struct device_node *node,
			   struct device_node *parent)
{
	pic_base = of_iomap(node, 0);
	if (!pic_base)
		return -ENXIO;

	__raw_writel(0, pic_base + PIC_ENABLE);

	pic_domain = irq_domain_create_linear(of_fwnode_handle(node), 16,
					      &pic_domain_ops, NULL);
	if (!pic_domain)
		return -ENOMEM;

	set_handle_irq(pic_handle_irq);
	return 0;
}
IRQCHIP_DECLARE(moxie_sim_pic, "moxie,sim-pic", pic_init);

void __init init_IRQ(void)
{
	irqchip_init();
}
