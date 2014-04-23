/*
 * Copyright (C) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_ARCH_INTERRUPTS_H_
#define __ASM_ARCH_INTERRUPTS_H_
#include "irqs.h"
/* SGI: Software Generated Interrupt */
#define IRQ_SGI(x)		S5P_IRQ(x)

/* PPI: Private Peripheral Interrupt */
#define IRQ_PPI(x)		S5P_IRQ(x+16)
#define IRQ_PPI_MCT_L		IRQ_PPI(12)

/* SPI: Shared Peripheral Interrupt */
#define IRQ_SPI(x)		S5P_IRQ(x+32)
#include "irqs-exynos4.h"
#endif
