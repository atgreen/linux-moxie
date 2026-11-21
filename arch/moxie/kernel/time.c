// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_clk.h>
#include <linux/clk.h>

/*
 * The simulated board's timer: a free-running count of clocks, and an
 * interrupt every so many of them.  Registers are native big-endian
 * words.
 */
#define TIMER_CTRL	0x0	/* write 1 to run */
#define TIMER_PERIOD	0x4	/* clocks between interrupts */
#define TIMER_ACK	0x8	/* write to drop the interrupt */
#define TIMER_COUNT	0xc	/* clocks since reset */

static void __iomem *timer_base;
static unsigned long timer_rate;

static int moxie_timer_set_periodic(struct clock_event_device *evt)
{
	__raw_writel(timer_rate / HZ, timer_base + TIMER_PERIOD);
	__raw_writel(1, timer_base + TIMER_CTRL);
	return 0;
}

static int moxie_timer_shutdown(struct clock_event_device *evt)
{
	__raw_writel(0, timer_base + TIMER_CTRL);
	return 0;
}

static struct clock_event_device moxie_clockevent = {
	.name			= "moxie-sim-timer",
	.features		= CLOCK_EVT_FEAT_PERIODIC,
	.rating			= 300,
	.set_state_periodic	= moxie_timer_set_periodic,
	.set_state_shutdown	= moxie_timer_shutdown,
};

static irqreturn_t moxie_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	__raw_writel(1, timer_base + TIMER_ACK);
	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static u64 moxie_timer_read(struct clocksource *cs)
{
	return __raw_readl(timer_base + TIMER_COUNT);
}

static struct clocksource moxie_clocksource = {
	.name	= "moxie-sim-timer",
	.rating	= 300,
	.read	= moxie_timer_read,
	.mask	= CLOCKSOURCE_MASK(32),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
};

static int __init moxie_timer_init(struct device_node *node)
{
	struct clk *clk;
	int irq;

	timer_base = of_iomap(node, 0);
	if (!timer_base)
		return -ENXIO;

	clk = of_clk_get(node, 0);
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	timer_rate = clk_get_rate(clk);

	irq = irq_of_parse_and_map(node, 0);
	if (!irq)
		return -EINVAL;

	if (request_irq(irq, moxie_timer_interrupt, IRQF_TIMER,
			"timer", &moxie_clockevent))
		return -EBUSY;

	clocksource_register_hz(&moxie_clocksource, timer_rate);

	moxie_clockevent.cpumask = cpumask_of(0);
	clockevents_register_device(&moxie_clockevent);
	return 0;
}
TIMER_OF_DECLARE(moxie_sim_timer, "moxie,sim-timer", moxie_timer_init);

void __init time_init(void)
{
	of_clk_init(NULL);
	timer_probe();
}
