/*
 *  Copyright (C) 2014 Emdoor Electronics
 *  jingfeng.shi <jingfeng.shi@emdoor.com>
 *  Based on ixp and linux/arch/arm/mach-exynos/irq-eint.c
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/cpu.h>
#include <asm/arch/interrupts.h>
#include <asm/proc-armv/ptrace.h>

struct _irq_handler {
	void                *m_data;
	void (*m_func)( void *data);
};

static struct _irq_handler IRQ_HANDLER[ NR_IRQS];

static void default_isr(void *data)
{
	printf("default_isr():  called for IRQ %d\n", (int)data);
}

static int get_irq(void)
{
	return 0;
}

void do_irq (struct pt_regs *pt_regs)
{
	int irq = get_irq();

	IRQ_HANDLER[irq].m_func(IRQ_HANDLER[irq].m_data);
}

void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data)
{
	if (irq >= NR_IRQS || !handle_irq)
		return;

	IRQ_HANDLER[irq].m_data = data;
	IRQ_HANDLER[irq].m_func = handle_irq;
}

 int exynos_irq_eint_set_type(int irq, unsigned int type)
{
	int offs = EINT_OFFSET(irq);
	int shift;
	u32 ctrl, mask;
	u32 newvalue = 0;

	switch (type) {
	case IRQ_TYPE_EDGE_RISING:
		newvalue = S5P_IRQ_TYPE_EDGE_RISING;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		newvalue = S5P_IRQ_TYPE_EDGE_FALLING;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		newvalue = S5P_IRQ_TYPE_EDGE_BOTH;
		break;

	case IRQ_TYPE_LEVEL_LOW:
		newvalue = S5P_IRQ_TYPE_LEVEL_LOW;
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		newvalue = S5P_IRQ_TYPE_LEVEL_HIGH;
		break;

	default:
		printf( "No such irq type %d", type);
		return -EINVAL;
	}

	shift = (offs & 0x7) * 4;
	mask = 0x7 << shift;

	//ctrl = readl(S5P_EINT_CON(EINT_REG_NR(irq)));
	ctrl &= ~mask;
	ctrl |= newvalue << shift;
	//writel(ctrl, S5P_EINT_CON(EINT_REG_NR(irq)));

	return 0;
}

int __irq_init_board(void)
{
	return 0;
}
int irq_init_board(void)
	__attribute__((weak, alias("__irq_init_board")));

int arch_interrupt_init (void)
{
	int irq;

	/* install ext1-31 interrupt handlers */
	for (irq = 0; irq < 31; irq++)
		irq_install_handler(IRQ_EINT(irq), default_isr, (void *)irq);

	/* configure interrupts for IRQ mode */
	irq_init_board();

	return (0);
}
