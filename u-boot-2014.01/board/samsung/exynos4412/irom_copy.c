/*
 * (C) Copyright 2011 Samsung Electronics Co. Ltd
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>
#include <asm/arch/movi_partition.h>
#ifdef CONFIG_SECURE_BOOT
#include "UBOOT_SB20_S5PC210S.h"
#endif

typedef u32 (*copy_sd_mmc_to_mem) \
	(u32 start_block, u32 block_count, u32* dest_addr);

#define ISRAM_ADDRESS	0x02020000
#define SECURE_CONTEXT_BASE 0x02023000
#define EXTERNAL_FUNC_ADDRESS	(ISRAM_ADDRESS + 0x0030)  /*0x02023030 IromF:SDMMC_ReadBlocks */
#define EXT_eMMC43_BL2_ByCPU_ADDRESS	(EXTERNAL_FUNC_ADDRESS + 0x4) 
#define EXT_eMMC43_BL2_ToIram_ADDRESS    (EXTERNAL_FUNC_ADDRESS + 0xC)/*0x0202303C IromF:LoadBL2FromEmmc43Ch0*/
#define MSH_ReadFromFIFO_eMMC_ADDRESS	(EXTERNAL_FUNC_ADDRESS + 0x14) /*0x02020044  IromF:MSH_ReadFromFIFO_eMMC*/
#define MSH_EndBootOp_eMMC_ADDRESS	(EXTERNAL_FUNC_ADDRESS + 0x18)/*0x02020048  IromF:MSH_EndBootOp_eMMC*/
#define LoadImageFromUsb_ADDRESS	(EXTERNAL_FUNC_ADDRESS + 0x40)/*0x02020070  IromF:LoadImageFromUsb */

#define SDMMC_ReadBlocks_eMMC_ByCPU(uNumOfBlks, uDstAddr)	\
	(((void(*)(u32, u32*))(*((u32 *)EXT_eMMC43_BL2_ByCPU_ADDRESS)))(uNumOfBlks, uDstAddr))

#define SDMMC_ReadBlocks(uStartBlk, uNumOfBlks, uDstAddr)	\
	(((void(*)(u32, u32, u32*))(*((u32 *)EXTERNAL_FUNC_ADDRESS)))(uStartBlk, uNumOfBlks, uDstAddr))

#define LoadImageFromUsb()	\
	(((void(*)())(*((u32 *)LoadImageFromUsb_ADDRESS)))())

#if defined (CONFIG_EXYNOS4212) || defined (CONFIG_ARCH_EXYNOS5) || defined (CONFIG_EXYNOS4412) 
typedef u32(*MSH_ReadFromFIFO_eMMC)
(u32 uNumOfBlks, u32 *uDstAddr);
#else
typedef u32(*MSH_ReadFromFIFO_eMMC)
(u32 RxWaterMark,u32 uNumOfBlks, u32 *uDstAddr);
#endif
typedef u32(*MSH_EndBootOp_eMMC)
(void);

#ifdef CONFIG_CORTEXA5_ENABLE
void * uboot_memcpy(void * dest,const void *src,size_t count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}
#endif

void movi_uboot_copy(void)
{
#ifdef CONFIG_CORTEXA5_ENABLE
	SDMMC_ReadBlocks(MOVI_UBOOT_POS, MOVI_UBOOT_BLKCNT, 0x40000000);
#endif
	SDMMC_ReadBlocks(MOVI_UBOOT_POS, MOVI_UBOOT_BLKCNT, CONFIG_PHY_UBOOT_BASE);

#ifdef CONFIG_SECURE_BOOT
	if(Check_Signature( (SB20_CONTEXT *)SECURE_CONTEXT_BASE, (unsigned char*)CONFIG_PHY_UBOOT_BASE, 
			PART_SIZE_UBOOT-256, (unsigned char*)(CONFIG_PHY_UBOOT_BASE+PART_SIZE_UBOOT-256), 256 ) != 0) {
		while(1);
	}
#endif
}

void usb_device_copy(void)
{
	LoadImageFromUsb();
}

void emmc_uboot_copy(void)
{
	SDMMC_ReadBlocks_eMMC_ByCPU(MOVI_UBOOT_BLKCNT, CONFIG_PHY_UBOOT_BASE);
}

void emmc_4_4_uboot_copy(void)
{
	MSH_ReadFromFIFO_eMMC bl2_copy =
	(MSH_ReadFromFIFO_eMMC) (*(u32 *) (MSH_ReadFromFIFO_eMMC_ADDRESS));

	MSH_EndBootOp_eMMC end_copy =
	(MSH_EndBootOp_eMMC) (*(u32 *) (MSH_EndBootOp_eMMC_ADDRESS));
#if defined (CONFIG_EXYNOS4212) || defined (CONFIG_ARCH_EXYNOS5) || defined(CONFIG_EXYNOS4412)
	bl2_copy(MOVI_UBOOT_BLKCNT, CONFIG_PHY_UBOOT_BASE);
#ifdef CONFIG_CORTEXA5_ENABLE
	uboot_memcpy(0x40000000, CONFIG_PHY_UBOOT_BASE, PART_SIZE_UBOOT);
#endif
#else
	bl2_copy(0x10, MOVI_UBOOT_BLKCNT, CONFIG_PHY_UBOOT_BASE);
#endif

#ifdef CONFIG_SECURE_BOOT
	if(Check_Signature( (SB20_CONTEXT *)SECURE_CONTEXT_BASE, (unsigned char*)CONFIG_PHY_UBOOT_BASE,
			PART_SIZE_UBOOT-256, (unsigned char*)(CONFIG_PHY_UBOOT_BASE+PART_SIZE_UBOOT-256), 256 ) != 0) {
		while(1);
	}
#endif
	end_copy();
}

void emmc_4_4_endbootOp_eMMC(void)
{
	MSH_EndBootOp_eMMC bl2_copy =
	(MSH_EndBootOp_eMMC) (*(u32 *) (MSH_EndBootOp_eMMC_ADDRESS));

	bl2_copy();
}

