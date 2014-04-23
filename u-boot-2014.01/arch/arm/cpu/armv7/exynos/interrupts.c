/*
 *  Copyright (C) 2014 Emdoor Electronics
 *  jingfeng.shi <jingfeng.shi@emdoor.com>
 *  Based on ixp and linux/arch/arm/mach-exynos/irq-eint.c
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
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
