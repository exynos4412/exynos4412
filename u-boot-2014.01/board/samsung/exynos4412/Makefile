#
# Copyright (C) 2010 Samsung Electronics
# Minkyu Kang <mk7.kang@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y := lowlevel_init.o mem_init_smdk4212.o clock_init_smdk4212.o irom_copy.o
ifeq ($(CONFIG_SAMSUNG_ONENAND),y)
obj-y	:= exynos4412.o  onenand.o
else
obj-y	:= exynos4412.o 
endif
