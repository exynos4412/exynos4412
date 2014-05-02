/*
 *  Copyright (C) 2010 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#ifdef CONFIG_SOFT_SPI
#include <spi.h>
#endif
#include <lcd.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/adc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/watchdog.h>
#include <asm/arch/sromc.h>
#include <libtizen.h>
#ifdef CONFIG_LD9040
#include <ld9040.h>
#endif
#include <power/pmic.h>
#include <usb/s3c_udc.h>
#include <usb.h>
#include <asm/arch/cpu.h>
#ifdef CONFIG_POWER_MAX8998
#include <power/max8998_pmic.h>
#endif
#ifdef CONFIG_POWER_S5M8767A
#include <power/s5m8767a_pmic.h>
#endif
DECLARE_GLOBAL_DATA_PTR;

struct exynos4_gpio_part1 *gpio1;
struct exynos4_gpio_part2 *gpio2;
unsigned int board_rev = 2;

u32 get_board_rev(void)
{
	return board_rev;
}

static int get_hwrev(void)
{
	return board_rev & 0xFF;
}

static void init_pmic_lcd(void);

int power_init_board(void)
{
#ifdef CONFIG_POWER_S5M8767A
	int ret, val=-1;
	struct pmic *pmc;
	/*
	 * For PMIC the I2C bus is named as I2C5, but it is connected
	 * to logical I2C adapter 1
	 */
	ret = pmic_init(I2C_1);
	if (ret)
		return ret;
	pmc = pmic_get(CONFIG_PMIC_NAME);
	if (pmic_probe(pmc)){  //set cur gd->cur_i2c_bus = pmic, run s3c24x0_probe
		printf("###ERROR:Probe I2c%d\n",pmc->bus);
		return -1;
	}
	pmic_reg_read(pmc, 0, &val);
	printf("S5M8767A Version:%d\n", val);
#if 0  //removed by jf.s, we will config S5M8767A
	init_pmic_lcd();
#endif
#endif
	return 0;
}

int dram_init(void)
{
	#ifdef USE_2G_DRAM
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE) + 
		get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_5, PHYS_SDRAM_5_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_6, PHYS_SDRAM_6_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_7, PHYS_SDRAM_7_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_8, PHYS_SDRAM_8_SIZE) ;
	#else
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE)+
		get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE) ;
	#endif
	return 0;
}

void dram_init_banksize(void)
{
	#ifdef USE_2G_DRAM
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;
	gd->bd->bi_dram[4].start = PHYS_SDRAM_5;
	gd->bd->bi_dram[4].size = PHYS_SDRAM_5_SIZE;
	gd->bd->bi_dram[5].start = PHYS_SDRAM_6;
	gd->bd->bi_dram[5].size = PHYS_SDRAM_6_SIZE;
	gd->bd->bi_dram[6].start = PHYS_SDRAM_7;
	gd->bd->bi_dram[6].size = PHYS_SDRAM_7_SIZE;
	gd->bd->bi_dram[7].start = PHYS_SDRAM_8;
	gd->bd->bi_dram[7].size = PHYS_SDRAM_8_SIZE;
	#else
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;
	#endif
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tExynos4412\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int err;
#if 0
	switch (get_hwrev()) {
	case 0:
		/*
		 * Set the low to enable LDO_EN
		 * But when you use the test board for eMMC booting
		 * you should set it HIGH since it removes the inverter
		 */
		/* MASSMEMORY_EN: XMDMDATA_6: GPE3[6] */
		s5p_gpio_direction_output(&gpio1->e3, 6, 0);
		break;
	default:
		/*
		 * Default reset state is High and there's no inverter
		 * But set it as HIGH to ensure
		 */
		/* MASSMEMORY_EN: XMDMADDR_3: GPE1[3] */
		s5p_gpio_direction_output(&gpio1->e1, 3, 1);
		break;
	}
#endif
	/*
	 * MMC device init
	 * mmc4	 : eMMC (8-bit buswidth)
	 * mmc2	 : SD card (4-bit buswidth)
	 */
	err = exynos_pinmux_config(PERIPH_ID_SDMMC4, PINMUX_FLAG_8BIT_MODE);
	if (err)
		printf("SDMMC4 not configured\n");
	else
		err = s5p_mshc_initialize();//err = s5p_mmc_init(0, 8);

	/* T-flash detect */
	s5p_gpio_cfg_pin(&gpio2->x0, 7, 0xf);
	s5p_gpio_set_pull(&gpio2->x0, 7, GPIO_PULL_NONE);

	/*
	 * Check the T-flash  detect pin
	 * GPX1[7] T-flash detect pin
	 */
	if (!s5p_gpio_get_value(&gpio2->x0, 7)) {
		err = exynos_pinmux_config(PERIPH_ID_SDMMC2, PINMUX_FLAG_NONE);
		if (err)
			debug("SDMMC2 not configured\n");
		else
			err = s5p_mmc_init(2, 4);
	}

	return err;

}
#endif

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	int ret = 0;
#ifdef CONFIG_POWER_MAX8998
	struct pmic *p = pmic_get("MAX8998_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return -1;

	if (on) {
		ret |= pmic_set_output(p,
				       MAX8998_REG_BUCK_ACTIVE_DISCHARGE3,
				       MAX8998_SAFEOUT1, LDO_ON);
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF1,
				      MAX8998_LDO3, LDO_ON);
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF2,
				      MAX8998_LDO8, LDO_ON);

	} else {
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF2,
				      MAX8998_LDO8, LDO_OFF);
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF1,
				      MAX8998_LDO3, LDO_OFF);
		ret |= pmic_set_output(p,
				       MAX8998_REG_BUCK_ACTIVE_DISCHARGE3,
				       MAX8998_SAFEOUT1, LDO_OFF);
	}

	if (ret) {
		puts("MAX8998 LDO setting error!\n");
		return -1;
	}
#endif
	return 0;
}

struct s3c_plat_otg_data s5pc210_otg_data = {
	.phy_control = s5pc210_phy_control,
	.regs_phy = EXYNOS4_USBPHY_BASE,
	.regs_otg = EXYNOS4_USBOTG_BASE,
	.usb_phy_ctrl = EXYNOS4_USBPHY_CONTROL,
	.usb_flags = PHY0_SLEEP,
};

int board_usb_init(int index, enum usb_init_type init)
{
        debug("USB_udc_probe\n");
        return s3c_udc_probe(&s5pc210_otg_data);
}
#endif

#ifdef CONFIG_DDR_DEBUG
extern void asm_printc(char c);
#endif
int board_early_init_f(void)
{
	wdt_stop();
#ifdef CONFIG_DDR_DEBUG
	asm_printc('#');
#endif
	return 0;
}

#ifdef CONFIG_SOFT_SPI
static void soft_spi_init(void)
{
	gpio_direction_output(CONFIG_SOFT_SPI_GPIO_SCLK,
		CONFIG_SOFT_SPI_MODE & SPI_CPOL);
	gpio_direction_output(CONFIG_SOFT_SPI_GPIO_MOSI, 1);
	gpio_direction_input(CONFIG_SOFT_SPI_GPIO_MISO);
	gpio_direction_output(CONFIG_SOFT_SPI_GPIO_CS,
		!(CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH));
}

void spi_cs_activate(struct spi_slave *slave)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_CS,
		!(CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH));
	SPI_SCL(1);
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_CS,
		CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_CS,
		!(CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH));
}

int  spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void universal_spi_scl(int bit)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_SCLK, bit);
}

void universal_spi_sda(int bit)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_MOSI, bit);
}

int universal_spi_read(void)
{
	return gpio_get_value(CONFIG_SOFT_SPI_GPIO_MISO);
}
#endif

static void init_pmic_lcd(void)
{
	unsigned char val;
	int ret = 0;
#ifdef CONFIG_POWER_MAX8998
	struct pmic *p = pmic_get("MAX8998_PMIC");

	if (!p)
		return;

	if (pmic_probe(p))
		return;

	/* LDO7 1.8V */
	val = 0x02; /* (1800 - 1600) / 100; */
	ret |= pmic_reg_write(p,  MAX8998_REG_LDO7, val);

	/* LDO17 3.0V */
	val = 0xe; /* (3000 - 1600) / 100; */
	ret |= pmic_reg_write(p,  MAX8998_REG_LDO17, val);

	/* Disable unneeded regulators */
	/*
	 * ONOFF1
	 * Buck1 ON, Buck2 OFF, Buck3 ON, Buck4 ON
	 * LDO2 ON, LDO3 OFF, LDO4 OFF, LDO5 ON
	 */
	val = 0xB9;
	ret |= pmic_reg_write(p,  MAX8998_REG_ONOFF1, val);

	/* ONOFF2
	 * LDO6 OFF, LDO7 ON, LDO8 OFF, LDO9 ON,
	 * LDO10 OFF, LDO11 OFF, LDO12 OFF, LDO13 OFF
	 */
	val = 0x50;
	ret |= pmic_reg_write(p,  MAX8998_REG_ONOFF2, val);

	/* ONOFF3
	 * LDO14 OFF, LDO15 OFF, LGO16 OFF, LDO17 OFF
	 * EPWRHOLD OFF, EBATTMON OFF, ELBCNFG2 OFF, ELBCNFG1 OFF
	 */
	val = 0x00;
	ret |= pmic_reg_write(p,  MAX8998_REG_ONOFF3, val);
#endif
	if (ret)
		puts("LCD pmic initialisation error!\n");
}

void exynos_cfg_lcd_gpio(void)
{
	unsigned int i, f3_end = 4;

	for (i = 0; i < 8; i++) {
		/* set GPF0,1,2[0:7] for RGB Interface and Data lines (32bit) */
		s5p_gpio_cfg_pin(&gpio1->f0, i, GPIO_FUNC(2));
		s5p_gpio_cfg_pin(&gpio1->f1, i, GPIO_FUNC(2));
		s5p_gpio_cfg_pin(&gpio1->f2, i, GPIO_FUNC(2));
		/* pull-up/down disable */
		s5p_gpio_set_pull(&gpio1->f0, i, GPIO_PULL_NONE);
		s5p_gpio_set_pull(&gpio1->f1, i, GPIO_PULL_NONE);
		s5p_gpio_set_pull(&gpio1->f2, i, GPIO_PULL_NONE);

		/* drive strength to max (24bit) */
		s5p_gpio_set_drv(&gpio1->f0, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f0, i, GPIO_DRV_SLOW);
		s5p_gpio_set_drv(&gpio1->f1, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f1, i, GPIO_DRV_SLOW);
		s5p_gpio_set_drv(&gpio1->f2, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f0, i, GPIO_DRV_SLOW);
	}

	for (i = 0; i < f3_end; i++) {
		/* set GPF3[0:3] for RGB Interface and Data lines (32bit) */
		s5p_gpio_cfg_pin(&gpio1->f3, i, GPIO_FUNC(2));
		/* pull-up/down disable */
		s5p_gpio_set_pull(&gpio1->f3, i, GPIO_PULL_NONE);
		/* drive strength to max (24bit) */
		s5p_gpio_set_drv(&gpio1->f3, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f3, i, GPIO_DRV_SLOW);
	}

	/* gpio pad configuration for LCD reset. */
	//s5p_gpio_cfg_pin(&gpio2->y4, 5, GPIO_OUTPUT);
	/*enable sn75lvds83b*/
	s5p_gpio_direction_output(&gpio2->l1, 0, 1);  //lcd_pwdn:GPL1_0, sn75lvds83b shutdown pin. active:high
	s5p_gpio_direction_output(&gpio1->d0,1, 1);  //pwm:high
	#ifdef CONFIG_SOFT_SPI
	spi_init();
	#endif
}

void exynos_reset_lcd(void)
{
#if 0
	s5p_gpio_set_value(&gpio2->y4, 5, 1);
	udelay(10000);
	s5p_gpio_set_value(&gpio2->y4, 5, 0);
	udelay(10000);
	s5p_gpio_set_value(&gpio2->y4, 5, 1);
	udelay(100);
#endif
}

void exynos_lcd_power_on(void)
{
#ifdef CONFIG_POWER_MAX8998
	struct pmic *p = pmic_get("MAX8998_PMIC");

	if (!p)
		return;

	if (pmic_probe(p))
		return;

	pmic_set_output(p, MAX8998_REG_ONOFF3, MAX8998_LDO17, LDO_ON);
	pmic_set_output(p, MAX8998_REG_ONOFF2, MAX8998_LDO7, LDO_ON);
#endif

#ifdef CONFIG_POWER_S5M8767A
	struct pmic *p = pmic_get(CONFIG_PMIC_NAME);

	if (!p)
		return;

	if (pmic_probe(p))
		return;
	/*Lcd 3V3 on S5m8767A:ldo9*/
	pmic_reg_write(p, S5M8767_REG_LDO9CTRL, 0x80|0x32);
#endif
}

vidinfo_t panel_info = {
	.vl_freq	= 70,
	.vl_col		= 1024,
	.vl_row		= 768,
	.vl_width	= 1024,
	.vl_height	= 768,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_hsp		= CONFIG_SYS_LOW,
	.vl_vsp		= CONFIG_SYS_HIGH,
	.vl_dp		= CONFIG_SYS_LOW,

	.vl_bpix	= 5,	/* Bits per pixel */ //32bpp

	/* LD9040 LCD Panel */
	.vl_hspw	= 32,
	.vl_hbpd	= 80,
	.vl_hfpd	= 48,

	.vl_vspw	= 5,
	.vl_vbpd	= 14,
	.vl_vfpd	= 3,
	.vl_cmd_allow_len = 0xf,

	.win_id		= 0,
	.dual_lcd_enabled = 0,

	.init_delay	= 0,
	.power_on_delay = 10000,
	.reset_delay	= 10000,
	.interface_mode = FIMD_RGB_INTERFACE,
	.mipi_enabled	= 0,
};

void exynos_cfg_ldo(void)
{
	#ifdef CONFIG_LD9040
	ld9040_cfg_ldo();
	#endif
}

void exynos_enable_ldo(unsigned int onoff)
{
	#ifdef CONFIG_LD9040
	ld9040_enable_ldo(onoff)
	#endif;
}

void init_panel_info(vidinfo_t *vid)
{
	vid->logo_on	= 1;
	vid->resolution	= HD_RESOLUTION;
	vid->rgb_mode	= MODE_RGB_P;

#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif

	/* for LD9040. */
	vid->pclk_name = 1;	/* MPLL */
	vid->sclk_div = 1;

	setenv("lcdinfo", "lcd=ld9040");
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SMC911X
        u32 smc_bw_conf, smc_bc_conf;
        struct fdt_sromc config;
        fdt_addr_t base_addr;

        /* Non-FDT configuration - bank number and timing parameters*/
        config.bank = CONFIG_ENV_SROM_BANK;
        config.width = 2;

        config.timing[FDT_SROM_TACS] = 0x01;
        config.timing[FDT_SROM_TCOS] = 0x01;
        config.timing[FDT_SROM_TACC] = 0x06;
        config.timing[FDT_SROM_TCOH] = 0x01;
        config.timing[FDT_SROM_TAH] = 0x0C;
        config.timing[FDT_SROM_TACP] = 0x09;
        config.timing[FDT_SROM_PMC] = 0x01;
        base_addr = CONFIG_SMC911X_BASE;

        /* Ethernet needs data bus width of 16 bits */
        if (config.width != 2) {
                debug("%s: Unsupported bus width %d\n", __func__,
                        config.width);
                return -1;
        }
        smc_bw_conf = SROMC_DATA16_WIDTH(config.bank)
                        | SROMC_BYTE_ENABLE(config.bank);

        smc_bc_conf = SROMC_BC_TACS(config.timing[FDT_SROM_TACS])   |\
                        SROMC_BC_TCOS(config.timing[FDT_SROM_TCOS]) |\
                        SROMC_BC_TACC(config.timing[FDT_SROM_TACC]) |\
                        SROMC_BC_TCOH(config.timing[FDT_SROM_TCOH]) |\
                        SROMC_BC_TAH(config.timing[FDT_SROM_TAH])   |\
                        SROMC_BC_TACP(config.timing[FDT_SROM_TACP]) |\
                        SROMC_BC_PMC(config.timing[FDT_SROM_PMC]);

        /* Select and configure the SROMC bank */
        exynos_pinmux_config(PERIPH_ID_SROMC, config.bank);
        s5p_config_sromc(config.bank, smc_bw_conf, smc_bc_conf);
        return smc911x_initialize(0, base_addr);
#endif
        return 0;
}

#ifdef CONFIG_SMC911X   //copy form s5pv310. 
static void smc9115_pre_init(void)
{
        u32 smc_bw_conf, smc_bc_conf;

        /* gpio configuration GPY0CON */
        s5p_gpio_cfg_pin(&gpio2->y0, CONFIG_ENV_SROM_BANK, GPIO_FUNC(2));

        /* Ethernet needs bus width of 16 bits */
        smc_bw_conf = SROMC_DATA16_WIDTH(CONFIG_ENV_SROM_BANK);
        smc_bc_conf = SROMC_BC_TACS(0x0F) | SROMC_BC_TCOS(0x0F)
                        | SROMC_BC_TACC(0x0F) | SROMC_BC_TCOH(0x0F)
                        | SROMC_BC_TAH(0x0F)  | SROMC_BC_TACP(0x0F)
                        | SROMC_BC_PMC(0x0F);

        /* Select and configure the SROMC bank */
        s5p_config_sromc(CONFIG_ENV_SROM_BANK, smc_bw_conf, smc_bc_conf);
}
#endif

#ifdef CONFIG_SYS_I2C_INIT_BOARD
void i2c_init_board(void)
{
	int err;
#ifdef CONFIG_POWER_S5M8767A
	/* I2C_1 */
        err = exynos_pinmux_config(PERIPH_ID_I2C1, PINMUX_FLAG_NONE);
        if (err) {
                debug("I2C%d not configured\n", (I2C_1));
                return;
        }
#endif

}
#endif

#ifdef CONFIG_USE_IRQ
int irq_init_board(void)
{
	int ctrl;
	  /* gpio configuration GPX2CON[0-1] */
        s5p_gpio_cfg_pin(&gpio2->x2, 0, GPIO_IRQ); 
        s5p_gpio_cfg_pin(&gpio2->x2, 1, GPIO_IRQ);
	ctrl = readl(0x11000E08); //GPX2-1/1 failing irq type
	ctrl &= 0xffffff00;
	ctrl |= 0x22;
	writel(ctrl,0x11000E08 );
	writel(1,0x10480000);
	writel(0xffffffff,0x10490080);
	writel(0xffffffff,0x10490100);
	return 0;
}
#endif
int board_init(void)
{
	gpio1 = (struct exynos4_gpio_part1 *) EXYNOS4_GPIO_PART1_BASE;
	gpio2 = (struct exynos4_gpio_part2 *) EXYNOS4_GPIO_PART2_BASE;

	//gd->bd->bi_arch_number = MACH_TYPE_UNIVERSAL_C210;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

#ifdef CONFIG_SOFT_SPI
	soft_spi_init();
#endif
#ifdef CONFIG_SMC911X
	//smc9115_pre_init();
#endif
	//check_hw_revision();  //exynos4412 Not use
	printf("HW Revision:\t0x%x\n", board_rev);

	return 0;
}
