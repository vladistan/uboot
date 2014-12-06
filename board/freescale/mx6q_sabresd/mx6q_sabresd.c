/*
 * Copyright (C) 2012-2013 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <asm/arch/mx6_pins.h>
#if defined(CONFIG_SECURE_BOOT)
#include <asm/arch/mx6_secure.h>
#endif
#include <asm/arch/mx6dl_pins.h>
#include <asm/arch/iomux-v3.h>
#include <asm/arch/regs-anadig.h>
#include <asm/errno.h>
#ifdef CONFIG_MXC_FEC
#include <miiphy.h>
#endif

#if defined(CONFIG_VIDEO_MX5)
#include <asm/imx_pwm.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <linux/mxcfb.h>
#include <ipu.h>
#endif
#if defined(CONFIG_VIDEO_MX5) || defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#endif

#include "../../../drivers/video/mxc_epdc_fb.h"

#ifdef CONFIG_IMX_ECSPI
#include <imx_spi.h>
#endif

#if CONFIG_I2C_MXC
#include <i2c.h>
#endif

#ifdef CONFIG_CMD_MMC
#include <mmc.h>
#include <fsl_esdhc.h>
#endif

#ifdef CONFIG_ARCH_MMU
#include <asm/mmu.h>
#include <asm/arch/mmu.h>
#endif

#ifdef CONFIG_CMD_CLOCK
#include <asm/clock.h>
#endif

#ifdef CONFIG_CMD_IMXOTP
#include <imx_otp.h>
#endif

#ifdef CONFIG_MXC_GPIO
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#endif

#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif

#ifdef CONFIG_DWC_AHSATA
#include <ahci.h>
#endif

#ifdef CONFIG_PPLANS_PMIC
#include <pplans_pmic.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static enum boot_device boot_dev;

#define GPIO_VOL_DN_KEY IMX_GPIO_NR(1, 5)
#define USB_OTG_PWR IMX_GPIO_NR(3, 22)
#define USB_H1_POWER IMX_GPIO_NR(1, 29)

extern int sata_curr_device;

#ifdef CONFIG_VIDEO_MX5
extern unsigned char fsl_bmp_reversed_600x400[];
extern int fsl_bmp_reversed_600x400_size;
extern int g_ipu_hw_rev;

#if defined(CONFIG_BMP_8BPP)
unsigned short colormap[256];
#elif defined(CONFIG_BMP_16BPP)
unsigned short colormap[65536];
#else
unsigned short colormap[16777216];
#endif

static struct pwm_device pwm0 = {
	.pwm_id = 0,
	.pwmo_invert = 0,
};

static int di = 1;

extern int ipuv3_fb_init(struct fb_videomode *mode, int di,
			int interface_pix_fmt,
			ipu_di_clk_parent_t di_clk_parent,
			int di_clk_val);

static struct fb_videomode lvds_xga = {
	 "XGA", 60, 1024, 768, 15385, 220, 40, 21, 7, 60, 10,
	 FB_SYNC_EXT,
	 FB_VMODE_NONINTERLACED,
	 0,
};

vidinfo_t panel_info;
#endif

static inline void setup_boot_device(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	uint bt_mem_ctl = (soc_sbmr & 0x000000FF) >> 4 ;
	uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;

	switch (bt_mem_ctl) {
	case 0x0:
		if (bt_mem_type)
			boot_dev = ONE_NAND_BOOT;
		else
			boot_dev = WEIM_NOR_BOOT;
		break;
	case 0x2:
			boot_dev = SATA_BOOT;
		break;
	case 0x3:
		if (bt_mem_type)
			boot_dev = I2C_BOOT;
		else
			boot_dev = SPI_NOR_BOOT;
		break;
	case 0x4:
	case 0x5:
		boot_dev = SD_BOOT;
		break;
	case 0x6:
	case 0x7:
		boot_dev = MMC_BOOT;
		break;
	case 0x8 ... 0xf:
		boot_dev = NAND_BOOT;
		break;
	default:
		boot_dev = UNKNOWN_BOOT;
		break;
	}
}

enum boot_device get_boot_device(void)
{
	return boot_dev;
}

u32 get_board_rev(void)
{
	return fsl_system_rev;
}

#ifdef CONFIG_ARCH_MMU
void board_mmu_init(void)
{
	unsigned long ttb_base = PHYS_SDRAM_1 + 0x4000;
	unsigned long i;

	/*
	* Set the TTB register
	*/
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

	/*
	* Set the Domain Access Control Register
	*/
	i = ARM_ACCESS_DACR_DEFAULT;
	asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

	/*
	* First clear all TT entries - ie Set them to Faulting
	*/
	memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);
	/* Actual   Virtual  Size   Attributes          Function */
	/* Base     Base     MB     cached? buffered?  access permissions */
	/* xxx00000 xxx00000 */
	X_ARM_MMU_SECTION(0x000, 0x000, 0x001,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* ROM, 1M */
	X_ARM_MMU_SECTION(0x001, 0x001, 0x008,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* 8M */
	X_ARM_MMU_SECTION(0x009, 0x009, 0x001,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* IRAM */
	X_ARM_MMU_SECTION(0x00A, 0x00A, 0x0F6,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* 246M */

	/* 2 GB memory starting at 0x10000000, only map 1.875 GB */
	X_ARM_MMU_SECTION(0x100, 0x100, 0x780,
			ARM_CACHEABLE, ARM_BUFFERABLE,
			ARM_ACCESS_PERM_RW_RW);
	/* uncached alias of the same 1.875 GB memory */
	X_ARM_MMU_SECTION(0x100, 0x880, 0x780,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW);

	/* Enable MMU */
	MMU_ON();
}
#endif

#define ANATOP_PLL_LOCK                 0x80000000
#define ANATOP_PLL_ENABLE_MASK          0x00002000
#define ANATOP_PLL_BYPASS_MASK          0x00010000
#define ANATOP_PLL_PWDN_MASK            0x00001000
#define ANATOP_PLL_HOLD_RING_OFF_MASK   0x00000800
#define ANATOP_SATA_CLK_ENABLE_MASK     0x00100000

#ifdef CONFIG_DWC_AHSATA
/* Staggered Spin-up */
#define	HOST_CAP_SSS			(1 << 27)
/* host version register*/
#define	HOST_VERSIONR			0xfc
#define PORT_SATA_SR			0x128

int sata_initialize(void)
{
	u32 reg = 0;
	u32 iterations = 1000000;

	if (sata_curr_device == -1) {
		/* Reset HBA */
		writel(HOST_RESET, SATA_ARB_BASE_ADDR + HOST_CTL);

		reg = 0;
		while (readl(SATA_ARB_BASE_ADDR + HOST_VERSIONR) == 0) {
			reg++;
			if (reg > 1000000)
				break;
		}

		reg = readl(SATA_ARB_BASE_ADDR + HOST_CAP);
		if (!(reg & HOST_CAP_SSS)) {
			reg |= HOST_CAP_SSS;
			writel(reg, SATA_ARB_BASE_ADDR + HOST_CAP);
		}

		reg = readl(SATA_ARB_BASE_ADDR + HOST_PORTS_IMPL);
		if (!(reg & 0x1))
			writel((reg | 0x1),
					SATA_ARB_BASE_ADDR + HOST_PORTS_IMPL);

		/* Release resources when there is no device on the port */
		do {
			reg = readl(SATA_ARB_BASE_ADDR + PORT_SATA_SR) & 0xF;
			if ((reg & 0xF) == 0)
				iterations--;
			else
				break;

		} while (iterations > 0);
	}

	return __sata_initialize();
}
#endif

/* Note: udelay() is not accurate for i2c timing */
static void __udelay(int time)
{
	int i, j;

	for (i = 0; i < time; i++) {
		for (j = 0; j < 200; j++) {
			asm("nop");
			asm("nop");
		}
	}
}

static int setup_sata(void)
{
	u32 reg = 0;
	s32 timeout = 100000;

	/* Enable sata clock */
	reg = readl(CCM_BASE_ADDR + 0x7c); /* CCGR5 */
	reg |= 0x30;
	writel(reg, CCM_BASE_ADDR + 0x7c);

	/* Enable PLLs */
	reg = readl(ANATOP_BASE_ADDR + 0xe0); /* ENET PLL */
	reg &= ~ANATOP_PLL_PWDN_MASK;
	writel(reg, ANATOP_BASE_ADDR + 0xe0);
	reg |= ANATOP_PLL_ENABLE_MASK;
	while (timeout--) {
		if (readl(ANATOP_BASE_ADDR + 0xe0) & ANATOP_PLL_LOCK)
			break;
	}
	if (timeout <= 0)
		return -1;
	reg &= ~ANATOP_PLL_BYPASS_MASK;
	writel(reg, ANATOP_BASE_ADDR + 0xe0);
	reg |= ANATOP_SATA_CLK_ENABLE_MASK;
	writel(reg, ANATOP_BASE_ADDR + 0xe0);

	/* Enable sata phy */
	reg = readl(IOMUXC_BASE_ADDR + 0x34); /* GPR13 */

	reg &= ~0x07ffffff;
	/*
	 * rx_eq_val_0 = 5 [26:24]
	 * los_lvl = 0x12 [23:19]
	 * rx_dpll_mode_0 = 0x3 [18:16]
	 * mpll_ss_en = 0x0 [14]
	 * tx_atten_0 = 0x4 [13:11]
	 * tx_boost_0 = 0x0 [10:7]
	 * tx_lvl = 0x11 [6:2]
	 * mpll_ck_off_b = 0x1 [1]
	 * tx_edgerate_0 = 0x0 [0]
	 * */
	reg |= 0x59124c6;
	writel(reg, IOMUXC_BASE_ADDR + 0x34);

	return 0;
}

int dram_init(void)
{
	/*
	 * Switch PL301_FAST2 to DDR Dual-channel mapping
	 * however this block the boot up, temperory redraw
	 */
	/*
	 * u32 reg = 1;
	 * writel(reg, GPV0_BASE_ADDR);
	 */

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

void setup_uart1(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT10__UART1_RXD); // UART1_TX_DATA -- CSI0_DATA10 (0x0360)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT11__UART1_RXD); // UART1_RX_DATA -- CSI0_DATA11 (0x0364)
}

void setup_uart5(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL1__UART5_RXD); // UART5_TX_DATA -- KEY_COL1 (0x0630)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL4__UART5_RTS); // UART5_RTS_B -- KEY_COL4 (0x063C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW1__UART5_RXD); // UART5_RX_DATA -- KEY_ROW1 (0x0644)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW4__UART5_RTS); // UART5_CTS_B -- KEY_ROW4 (0x0650)
}

static void setup_uart(void)
{
	setup_uart1();
	setup_uart5();

}

#ifdef CONFIG_VIDEO_MX5
void setup_lvds_poweron(void)
{
	int reg;
	/* AUX_5V_EN: GPIO(6, 10) */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_RB0__GPIO_6_10);

	reg = readl(GPIO6_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 10);
	writel(reg, GPIO6_BASE_ADDR + GPIO_GDIR);

	reg = readl(GPIO6_BASE_ADDR + GPIO_DR);
	reg |= (1 << 10);
	writel(reg, GPIO6_BASE_ADDR + GPIO_DR);
}
#endif

#ifdef CONFIG_I2C_MXC
#define I2C1_SDA_GPIO5_26_BIT_MASK  (1 << 26)
#define I2C1_SCL_GPIO5_27_BIT_MASK  (1 << 27)
#define I2C2_SCL_GPIO4_12_BIT_MASK  (1 << 12)
#define I2C2_SDA_GPIO4_13_BIT_MASK  (1 << 13)
#define I2C3_SCL_GPIO1_3_BIT_MASK   (1 << 3)
#define I2C3_SDA_GPIO1_6_BIT_MASK   (1 << 6)

void mx6dl_iomux_setup_i2c1(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT8__I2C1_SDA); // I2C1_SDA -- CSI0_DATA08 (0x0398)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT9__I2C1_SCL); // I2C1_SCL -- CSI0_DATA09 (0x039C)
}

void mx6dl_iomux_setup_i2c4(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_CS3__I2C4_SDA); // I2C4_SDA -- NAND_CS3_B (0x0668)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_WP_B__I2C4_SCL); // I2C4_SCL -- NAND_WP_B (0x0690)
}

static void setup_i2c(unsigned int module_base)
{
	unsigned int reg;

	switch (module_base) {
	case I2C1_BASE_ADDR:

		mx6dl_iomux_setup_i2c1();

		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0xC0;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);

		break;
	default:
		printf("Invalid I2C base: 0x%x\n", module_base);
		break;
	}
}

static void mx6q_i2c_gpio_scl_direction(int bus, int output)
{
	u32 reg;

	switch (bus) {
	case 1:
		x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT9__GPIO_5_27);
		reg = readl(GPIO5_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C1_SCL_GPIO5_27_BIT_MASK;
		else
			reg &= ~I2C1_SCL_GPIO5_27_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_GDIR);
		break;
	}
}

/* set 1 to output, sent 0 to input */
static void mx6q_i2c_gpio_sda_direction(int bus, int output)
{
	u32 reg;

	switch (bus) {
	case 1:
		x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT8__GPIO_5_26);
		reg = readl(GPIO5_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C1_SDA_GPIO5_26_BIT_MASK;
		else
			reg &= ~I2C1_SDA_GPIO5_26_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_GDIR);
		break;
	default:
		break;
	}
}

/* set 1 to high 0 to low */
static void mx6q_i2c_gpio_scl_set_level(int bus, int high)
{
	u32 reg;

	switch (bus) {
	case 1:
		reg = readl(GPIO5_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C1_SCL_GPIO5_27_BIT_MASK;
		else
			reg &= ~I2C1_SCL_GPIO5_27_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_DR);
		break;
	}
}

/* set 1 to high 0 to low */
static void mx6q_i2c_gpio_sda_set_level(int bus, int high)
{
	u32 reg;

	switch (bus) {
	case 1:
		reg = readl(GPIO5_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C1_SDA_GPIO5_26_BIT_MASK;
		else
			reg &= ~I2C1_SDA_GPIO5_26_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_DR);
		break;
	}
}

static int mx6q_i2c_gpio_check_sda(int bus)
{
	u32 reg;
	int result = 0;

	switch (bus) {
	case 1:
		reg = readl(GPIO5_BASE_ADDR + GPIO_PSR);
		result = !!(reg & I2C1_SDA_GPIO5_26_BIT_MASK);
		break;

	return result;
	}
}

 /* Random reboot cause i2c SDA low issue:
  * the i2c bus busy because some device pull down the I2C SDA
  * line. This happens when Host is reading some byte from slave, and
  * then host is reset/reboot. Since in this case, device is
  * controlling i2c SDA line, the only thing host can do this give the
  * clock on SCL and sending NAK, and STOP to finish this
  * transaction.
  *
  * How to fix this issue:
  * detect if the SDA was low on bus send 8 dummy clock, and 1
  * clock + NAK, and STOP to finish i2c transaction the pending
  * transfer.
  */
int i2c_bus_recovery(void)
{
	int i, bus, result = 0;

	for (bus = 1; bus <= 3; bus++) {
        
        if ( bus == 2 || bus == 3 ) 
            continue;
        
		mx6q_i2c_gpio_sda_direction(bus, 0);

		if (mx6q_i2c_gpio_check_sda(bus) == 0) {
			printf("i2c: I2C%d SDA is low, start i2c recovery...\n", bus);
			mx6q_i2c_gpio_scl_direction(bus, 1);
			mx6q_i2c_gpio_scl_set_level(bus, 1);
			__udelay(10000);

			for (i = 0; i < 9; i++) {
				mx6q_i2c_gpio_scl_set_level(bus, 1);
				__udelay(5);
				mx6q_i2c_gpio_scl_set_level(bus, 0);
				__udelay(5);
			}

			/* 9th clock here, the slave should already
			   release the SDA, we can set SDA as high to
			   a NAK.*/
			mx6q_i2c_gpio_sda_direction(bus, 1);
			mx6q_i2c_gpio_sda_set_level(bus, 1);
			__udelay(1); /* Pull up SDA first */
			mx6q_i2c_gpio_scl_set_level(bus, 1);
			__udelay(5); /* plus pervious 1 us */
			mx6q_i2c_gpio_scl_set_level(bus, 0);
			__udelay(5);
			mx6q_i2c_gpio_sda_set_level(bus, 0);
			__udelay(5);
			mx6q_i2c_gpio_scl_set_level(bus, 1);
			__udelay(5);
			/* Here: SCL is high, and SDA from low to high, it's a
			 * stop condition */
			mx6q_i2c_gpio_sda_set_level(bus, 1);
			__udelay(5);

			mx6q_i2c_gpio_sda_direction(bus, 0);
			if (mx6q_i2c_gpio_check_sda(bus) == 1)
				printf("I2C%d Recovery success\n", bus);
			else {
				printf("I2C%d Recovery failed, I2C1 SDA still low!!!\n", bus);
				result |= 1 << bus;
			}
		}

		/* configure back to i2c */
		switch (bus) {
		case 1:
			setup_i2c(I2C1_BASE_ADDR);
			break;
		}
	}

	return result;
}

#define PMIC_CHK if(rv == - 1) {return -1;}

static int setup_pmic_voltages(void)
{
	unsigned char value, rev_id = 0 ;
	int rv;

	printf ("Setting up PMIC voltages\n");
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (!i2c_probe(0x8)) {

	    if (probe_pfuze100())
	    {
	       printf ("PFUZER100 Device not found.\n");
	       return -1;
	    }

	    rv = pplans_pmic_basic_reg_setup(); PMIC_CHK;
	    rv = pplans_pmic_sw3_reg_setup(); PMIC_CHK;

        rv = pplans_pmic_handle_sw3(); PMIC_CHK;
	}
}
#endif

#ifdef CONFIG_IMX_ECSPI
s32 spi_get_cfg(struct imx_spi_dev_t *dev)
{
	switch (dev->slave.cs) {
	case 0:
		/* SPI-NOR */
		dev->base = ECSPI1_BASE_ADDR;
		dev->freq = 25000000;
		dev->ss_pol = IMX_SPI_ACTIVE_LOW;
		dev->ss = 0;
		dev->fifo_sz = 64 * 4;
		dev->us_delay = 0;
		break;
	case 1:
		/* SPI-NOR */
		dev->base = ECSPI1_BASE_ADDR;
		dev->freq = 25000000;
		dev->ss_pol = IMX_SPI_ACTIVE_LOW;
		dev->ss = 1;
		dev->fifo_sz = 64 * 4;
		dev->us_delay = 0;
		break;
	default:
		printf("Invalid Bus ID!\n");
	}

	return 0;
}

void spi_io_init(struct imx_spi_dev_t *dev)
{
	u32 reg;

	switch (dev->base) {
	case ECSPI1_BASE_ADDR:
		/* Enable clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR1);
		reg |= 0x3;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR1);

		/* SCLK */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL0__ECSPI1_SCLK);

		/* MISO */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL1__ECSPI1_MISO);

		/* MOSI */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW0__ECSPI1_MOSI);

		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW1__ECSPI1_SS0);
		break;
	case ECSPI2_BASE_ADDR:
	case ECSPI3_BASE_ADDR:
		/* ecspi2-3 fall through */
		break;
	default:
		break;
	}
}
#endif
#if 0
#ifdef CONFIG_NAND_GPMI

iomux_v3_cfg_t nfc_pads[] = {
	MX6Q_PAD_NANDF_CLE__RAWNAND_CLE,
	MX6Q_PAD_NANDF_ALE__RAWNAND_ALE,
	MX6Q_PAD_NANDF_WP_B__RAWNAND_RESETN,
	MX6Q_PAD_NANDF_RB0__RAWNAND_READY0,
	MX6Q_PAD_NANDF_CS0__RAWNAND_CE0N,
	MX6Q_PAD_NANDF_CS1__RAWNAND_CE1N,
	MX6Q_PAD_NANDF_CS2__RAWNAND_CE2N,
	MX6Q_PAD_NANDF_CS3__RAWNAND_CE3N,
	MX6Q_PAD_SD4_CMD__RAWNAND_RDN,
	MX6Q_PAD_SD4_CLK__RAWNAND_WRN,
	MX6Q_PAD_NANDF_D0__RAWNAND_D0,
	MX6Q_PAD_NANDF_D1__RAWNAND_D1,
	MX6Q_PAD_NANDF_D2__RAWNAND_D2,
	MX6Q_PAD_NANDF_D3__RAWNAND_D3,
	MX6Q_PAD_NANDF_D4__RAWNAND_D4,
	MX6Q_PAD_NANDF_D5__RAWNAND_D5,
	MX6Q_PAD_NANDF_D6__RAWNAND_D6,
	MX6Q_PAD_NANDF_D7__RAWNAND_D7,
	MX6Q_PAD_SD4_DAT0__RAWNAND_DQS,
};

int setup_gpmi_nand(void)
{
	unsigned int reg;

	/* config gpmi nand iomux */
	mxc_iomux_v3_setup_multiple_pads(nfc_pads,
			ARRAY_SIZE(nfc_pads));


	/* config gpmi and bch clock to 11Mhz*/
	reg = readl(CCM_BASE_ADDR + CLKCTL_CS2CDR);
	reg &= 0xF800FFFF;
	reg |= 0x01E40000;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CS2CDR);

	/* enable gpmi and bch clock gating */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR4);
	reg |= 0xFF003000;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR4);

	/* enable apbh clock gating */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR0);
	reg |= 0x0030;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR0);

}
#endif
#endif

#ifdef CONFIG_NET_MULTI
int board_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

	return rc;
}
#endif

#ifdef CONFIG_CMD_MMC

/* On this board, only SD3 can support 1.8V signalling
 * that is required for UHS-I mode of operation.
 * Last element in struct is used to indicate 1.8V support.
 */
struct fsl_esdhc_cfg usdhc_cfg[4] = {
	{USDHC1_BASE_ADDR, 1, 1, 1, 0},
	{USDHC2_BASE_ADDR, 1, 1, 1, 0},
	{USDHC3_BASE_ADDR, 1, 1, 1, 0},
	{USDHC4_BASE_ADDR, 1, 1, 1, 0},
};



void setup_usdhc1(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_1__USDHC1_CD); // SD1_CD_B -- GPIO01 (0x05E0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_CLK__USDHC1_CLK); // SD1_CLK -- SD1_CLK (0x06C4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_CMD__USDHC1_CMD); // SD1_CMD -- SD1_CMD (0x06C8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT0__USDHC1_DAT0); // SD1_DATA0 -- SD1_DATA0 (0x06CC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT1__USDHC1_DAT1); // SD1_DATA1 -- SD1_DATA1 (0x06D0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT2__USDHC1_DAT2); // SD1_DATA2 -- SD1_DATA2 (0x06D4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT3__USDHC1_DAT3); // SD1_DATA3 -- SD1_DATA3 (0x06D8)
}

void setup_usdhc2(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_4__USDHC2_CD); // SD2_CD_B -- GPIO04 (0x05FC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD2_CLK__USDHC2_CLK); // SD2_CLK -- SD2_CLK (0x06DC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD2_CMD__USDHC2_CMD); // SD2_CMD -- SD2_CMD (0x06E0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD2_DAT0__USDHC2_DAT0); // SD2_DATA0 -- SD2_DATA0 (0x06E4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD2_DAT1__USDHC2_DAT1); // SD2_DATA1 -- SD2_DATA1 (0x06E8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD2_DAT2__USDHC2_DAT2); // SD2_DATA2 -- SD2_DATA2 (0x06EC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD2_DAT3__USDHC2_DAT3); // SD2_DATA3 -- SD2_DATA3 (0x06F0)
}

void setup_usdhc3(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD3_CLK__USDHC3_CLK); // SD3_CLK -- SD3_CLK (0x06F4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD3_CMD__USDHC3_CMD); // SD3_CMD -- SD3_CMD (0x06F8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD3_DAT0__USDHC3_DAT0); // SD3_DATA0 -- SD3_DATA0 (0x06FC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD3_DAT1__USDHC3_DAT1); // SD3_DATA1 -- SD3_DATA1 (0x0700)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD3_DAT2__USDHC3_DAT2); // SD3_DATA2 -- SD3_DATA2 (0x0704)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD3_DAT3__USDHC3_DAT3); // SD3_DATA3 -- SD3_DATA3 (0x0708)
}

void setup_usdhc4(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_ALE__USDHC4_RST); // SD4_RESET -- NAND_ALE (0x0654)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_CLK__USDHC4_CLK); // SD4_CLK -- SD4_CLK (0x0720)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_CMD__USDHC4_CMD); // SD4_CMD -- SD4_CMD (0x0724)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT0__USDHC4_DAT0); // SD4_DATA0 -- SD4_DATA0 (0x0728)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT1__USDHC4_DAT1); // SD4_DATA1 -- SD4_DATA1 (0x072C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT2__USDHC4_DAT2); // SD4_DATA2 -- SD4_DATA2 (0x0730)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT3__USDHC4_DAT3); // SD4_DATA3 -- SD4_DATA3 (0x0734)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT4__USDHC4_DAT4); // SD4_DATA4 -- SD4_DATA4 (0x0738)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT5__USDHC4_DAT5); // SD4_DATA5 -- SD4_DATA5 (0x073C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT6__USDHC4_DAT6); // SD4_DATA6 -- SD4_DATA6 (0x0740)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_SD4_DAT7__USDHC4_DAT7); // SD4_DATA7 -- SD4_DATA7 (0x0744)
}

int usdhc_gpio_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;

	for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM;
		++index) {
		switch (index) {
		case 0:
			setup_usdhc1();
			break;
		case 1:
			setup_usdhc2();
			break;
		case 2:
			setup_usdhc3();
			break;
		case 3:
			setup_usdhc4();
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) then supported by the board (%d)\n",
				index+1, CONFIG_SYS_FSL_USDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
	}

	return status;
}

int board_mmc_init(bd_t *bis)
{
	if (!usdhc_gpio_init(bis))
		return 0;
	else
		return -1;
}

#ifdef CONFIG_MXC_EPDC
#ifdef CONFIG_SPLASH_SCREEN
int setup_splash_img(void)
{
#ifdef CONFIG_SPLASH_IS_IN_MMC
	int mmc_dev = get_mmc_env_devno();
	ulong offset = CONFIG_SPLASH_IMG_OFFSET;
	ulong size = CONFIG_SPLASH_IMG_SIZE;
	ulong addr = 0;
	char *s = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	s = getenv("splashimage");

	if (NULL == s) {
		puts("env splashimage not found!\n");
		return -1;
	}
	addr = simple_strtoul(s, NULL, 16);

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return  -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
					blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;
#endif

	return 0;
}
#endif

vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 800,
	.vl_row = 600,
	.vl_pixclock = 26666667,
	.vl_left_margin = 8,
	.vl_right_margin = 100,
	.vl_upper_margin = 4,
	.vl_lower_margin = 8,
	.vl_hsync = 4,
	.vl_vsync = 1,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	cmap:0,
};

struct epdc_timing_params panel_timings = {
	.vscan_holdoff = 4,
	.sdoed_width = 10,
	.sdoed_delay = 20,
	.sdoez_width = 10,
	.sdoez_delay = 20,
	.gdclk_hp_offs = 419,
	.gdsp_offs = 20,
	.gdoe_offs = 0,
	.gdclk_offs = 5,
	.num_ce = 1,
};

static void setup_epdc_power(void)
{
	unsigned int reg;

	/* Setup epdc voltage */

	/* EIM_A17 - GPIO2[21] for PWR_GOOD status */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A17__GPIO_2_21);

	/* EIM_D17 - GPIO3[17] for VCOM control */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D17__GPIO_3_17);

	/* Set as output */
	reg = readl(GPIO3_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 17);
	writel(reg, GPIO3_BASE_ADDR + GPIO_GDIR);

	/* EIM_D20 - GPIO3[20] for EPD PMIC WAKEUP */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D20__GPIO_3_20);
	/* Set as output */
	reg = readl(GPIO3_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 20);
	writel(reg, GPIO3_BASE_ADDR + GPIO_GDIR);

	/* EIM_A18 - GPIO2[20] for EPD PWR CTL0 */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A18__GPIO_2_20);
	/* Set as output */
	reg = readl(GPIO2_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 20);
	writel(reg, GPIO2_BASE_ADDR + GPIO_GDIR);
}

int setup_waveform_file(void)
{
#ifdef CONFIG_WAVEFORM_FILE_IN_MMC
	int mmc_dev = get_mmc_env_devno();
	ulong offset = CONFIG_WAVEFORM_FILE_OFFSET;
	ulong size = CONFIG_WAVEFORM_FILE_SIZE;
	ulong addr = CONFIG_WAVEFORM_BUF_ADDR;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
		blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;
#else
	return -1;
#endif
}

static void epdc_enable_pins(void)
{
	/* epdc iomux settings */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A16__EPDC_SDDO_0);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA10__EPDC_SDDO_1);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA12__EPDC_SDDO_2);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA11__EPDC_SDDO_3);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_LBA__EPDC_SDDO_4);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB2__EPDC_SDDO_5);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_CS0__EPDC_SDDO_6);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_RW__EPDC_SDDO_7);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A21__EPDC_GDCLK);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A22__EPDC_GDSP);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A23__EPDC_GDOE);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A24__EPDC_GDRL);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D31__EPDC_SDCLK);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D27__EPDC_SDOE);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA1__EPDC_SDLE);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB1__EPDC_SDSHR);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA2__EPDC_BDR_0);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA4__EPDC_SDCE_0);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA5__EPDC_SDCE_1);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA6__EPDC_SDCE_2);
}

static void epdc_disable_pins(void)
{
	/* Configure MUX settings for EPDC pins to GPIO */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A16__GPIO_2_22);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA10__GPIO_3_10);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA12__GPIO_3_12);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA11__GPIO_3_11);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_LBA__GPIO_2_27);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB2__GPIO_2_30);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_CS0__GPIO_2_23);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_RW__GPIO_2_26);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A21__GPIO_2_17);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A22__GPIO_2_16);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A23__GPIO_6_6);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A24__GPIO_5_4);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D31__GPIO_3_31);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D27__GPIO_3_27);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA1__GPIO_3_1);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB1__GPIO_2_29);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA2__GPIO_3_2);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA4__GPIO_3_4);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA5__GPIO_3_5);
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA6__GPIO_3_6);
}

static void setup_epdc(void)
{
	unsigned int reg;

	/*** epdc Maxim PMIC settings ***/

	/* EPDC PWRSTAT - GPIO2[21] for PWR_GOOD status */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A17__GPIO_2_21);

	/* EPDC VCOM0 - GPIO3[17] for VCOM control */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D17__GPIO_3_17);

	/* UART4 TXD - GPIO3[20] for EPD PMIC WAKEUP */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D20__GPIO_3_20);

	/* EIM_A18 - GPIO2[20] for EPD PWR CTL0 */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A18__GPIO_2_20);

	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk (IPU2_CLK) from PFD_400M, set to 396/2 = 198MHz */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR3);
	reg &= ~0x7C000;
	reg |= (1 << 16) | (1 << 14);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR3);

	/* EPDC AXI clk enable */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg |= 0x00C0;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);

	/* EPDC PIX clk (IPU2_DI1_CLK) from PLL5, set to 650/4/6 = ~27MHz */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR2);
	reg &= ~0x3FE00;
	reg |= (2 << 15) | (5 << 12);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR2);

	/* PLL5 enable (defaults to 650) */
	reg = readl(ANATOP_BASE_ADDR + ANATOP_PLL_VIDEO);
	reg &= ~((1 << 16) | (1 << 12));
	reg |= (1 << 13);
	writel(reg, ANATOP_BASE_ADDR + ANATOP_PLL_VIDEO);

	/* EPDC PIX clk enable */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg |= 0x0C00;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);

	panel_info.epdc_data.working_buf_addr = CONFIG_WORKING_BUF_ADDR;
	panel_info.epdc_data.waveform_buf_addr = CONFIG_WAVEFORM_BUF_ADDR;

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	panel_info.epdc_data.epdc_timings = panel_timings;

	setup_epdc_power();

	/* Assign fb_base */
	gd->fb_base = CONFIG_FB_BASE;
}

void epdc_power_on()
{
	unsigned int reg;

	/* Set EPD_PWR_CTL0 to high - enable EINK_VDD (3.15) */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg |= (1 << 20);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);
	udelay(1000);

	/* Enable epdc signal pin */
	epdc_enable_pins();

	/* Set PMIC Wakeup to high - enable Display power */
	reg = readl(GPIO3_BASE_ADDR + GPIO_DR);
	reg |= (1 << 20);
	writel(reg, GPIO3_BASE_ADDR + GPIO_DR);

	/* Wait for PWRGOOD == 1 */
	while (1) {
		reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
		if (!(reg & (1 << 21)))
			break;

		udelay(100);
	}

	/* Enable VCOM */
	reg = readl(GPIO3_BASE_ADDR + GPIO_DR);
	reg |= (1 << 17);
	writel(reg, GPIO3_BASE_ADDR + GPIO_DR);

	reg = readl(GPIO3_BASE_ADDR + GPIO_DR);

	udelay(500);
}

void  epdc_power_off()
{
	unsigned int reg;
	/* Set PMIC Wakeup to low - disable Display power */
	reg = readl(GPIO3_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 20);
	writel(reg, GPIO3_BASE_ADDR + GPIO_DR);

	/* Disable VCOM */
	reg = readl(GPIO3_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 17);
	writel(reg, GPIO3_BASE_ADDR + GPIO_DR);

	epdc_disable_pins();

	/* Set EPD_PWR_CTL0 to low - disable EINK_VDD (3.15) */
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 20);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);
}
#endif

/* For DDR mode operation, provide target delay parameter for each SD port.
 * Use cfg->esdhc_base to distinguish the SD port #. The delay for each port
 * is dependent on signal layout for that particular port.  If the following
 * CONFIG is not defined, then the default target delay value will be used.
 */
#ifdef CONFIG_GET_DDR_TARGET_DELAY
u32 get_ddr_delay(struct fsl_esdhc_cfg *cfg)
{
	/* No delay required on SABRESD board SD ports */
	return 0;
}
#endif

#endif

#ifndef CONFIG_MXC_EPDC
#ifdef CONFIG_LCD
void lcd_enable(void)
{
	char *s;
	int ret;
	unsigned int reg;

	s = getenv("lvds_num");
	di = simple_strtol(s, NULL, 10);

	/*
	* hw_rev 2: IPUV3DEX
	* hw_rev 3: IPUV3M
	* hw_rev 4: IPUV3H
	*/
	g_ipu_hw_rev = IPUV3_HW_REV_IPUV3H;

	imx_pwm_config(pwm0, 25000, 50000);
	imx_pwm_enable(pwm0);

#if defined CONFIG_MX6Q
	/* PWM backlight */
	mxc_iomux_v3_setup_pad(MX6Q_PAD_SD1_DAT3__PWM1_PWMO);
	/* LVDS panel CABC_EN0 */
	mxc_iomux_v3_setup_pad(MX6Q_PAD_NANDF_CS2__GPIO_6_15);
	/* LVDS panel CABC_EN1 */
	mxc_iomux_v3_setup_pad(MX6Q_PAD_NANDF_CS3__GPIO_6_16);
#elif defined CONFIG_MX6DL
	/* PWM backlight */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT3__PWM1_PWMO);
	/* LVDS panel CABC_EN0 */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_CS2__GPIO_6_15);
	/* LVDS panel CABC_EN1 */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_CS3__GPIO_6_16);
#endif
	/*
	 * Set LVDS panel CABC_EN0 to low to disable
	 * CABC function. This function will turn backlight
	 * automatically according to display content, so
	 * simply disable it to get rid of annoying unstable
	 * backlight phenomena.
	 */
	reg = readl(GPIO6_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 15);
	writel(reg, GPIO6_BASE_ADDR + GPIO_GDIR);

	reg = readl(GPIO6_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 15);
	writel(reg, GPIO6_BASE_ADDR + GPIO_DR);

	/*
	 * Set LVDS panel CABC_EN1 to low to disable
	 * CABC function.
	 */
	reg = readl(GPIO6_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 16);
	writel(reg, GPIO6_BASE_ADDR + GPIO_GDIR);

	reg = readl(GPIO6_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 16);
	writel(reg, GPIO6_BASE_ADDR + GPIO_DR);

	/* Disable ipu1_clk/ipu1_di_clk_x/ldb_dix_clk. */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg &= ~0xC033;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);

#if defined CONFIG_MX6Q
	/*
	 * Align IPU1 HSP clock and IPU1 DIx pixel clock
	 * with kernel setting to avoid screen flick when
	 * booting into kernel. Developer should change
	 * the relevant setting if kernel setting changes.
	 * IPU1 HSP clock tree:
	 * osc_clk(24M)->pll2_528_bus_main_clk(528M)->
	 * periph_clk(528M)->mmdc_ch0_axi_clk(528M)->
	 * ipu1_clk(264M)
	 */
	/* pll2_528_bus_main_clk */
	/* divider */
	writel(0x1, ANATOP_BASE_ADDR + 0x34);

	/* periph_clk */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCMR);
	reg &= ~(0x3 << 18);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCMR);

	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
	reg &= ~(0x1 << 25);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCDR);

	/*
	 * Check PERIPH_CLK_SEL_BUSY in
	 * MXC_CCM_CDHIPR register.
	 */
	do {
		udelay(5);
		reg = readl(CCM_BASE_ADDR + CLKCTL_CDHIPR);
	} while (reg & (0x1 << 5));

	/* mmdc_ch0_axi_clk */
	/* divider */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
	reg &= ~(0x7 << 19);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCDR);

	/*
	 * Check MMDC_CH0PODF_BUSY in
	 * MXC_CCM_CDHIPR register.
	 */
	do {
		udelay(5);
		reg = readl(CCM_BASE_ADDR + CLKCTL_CDHIPR);
	} while (reg & (0x1 << 4));

	/* ipu1_clk */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR3);
	/* source */
	reg &= ~(0x3 << 9);
	/* divider */
	reg &= ~(0x7 << 11);
	reg |= (0x1 << 11);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR3);

	/*
	 * ipu1_pixel_clk_x clock tree:
	 * osc_clk(24M)->pll2_528_bus_main_clk(528M)->
	 * pll2_pfd_352M(452.57M)->ldb_dix_clk(64.65M)->
	 * ipu1_di_clk_x(64.65M)->ipu1_pixel_clk_x(64.65M)
	 */
	/* pll2_pfd_352M */
	/* disable */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x104);
	/* divider */
	writel(0x3F, ANATOP_BASE_ADDR + 0x108);
	writel(0x15, ANATOP_BASE_ADDR + 0x104);

	/* ldb_dix_clk */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CS2CDR);
	reg &= ~(0x3F << 9);
	reg |= (0x9 << 9);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CS2CDR);
	/* divider */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCMR2);
	reg |= (0x3 << 10);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCMR2);

	/* pll2_pfd_352M */
	/* enable after ldb_dix_clk source is set */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x108);

	/* ipu1_di_clk_x */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CHSCCDR);
	reg &= ~0xE07;
	reg |= 0x803;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CHSCCDR);
#elif defined CONFIG_MX6DL /* CONFIG_MX6Q */
	/*
	 * IPU1 HSP clock tree:
	 * osc_clk(24M)->pll3_usb_otg_main_clk(480M)->
	 * pll3_pfd_540M(540M)->ipu1_clk(270M)
	 */
	/* pll3_usb_otg_main_clk */
	/* divider */
	writel(0x3, ANATOP_BASE_ADDR + 0x18);

	/* pll3_pfd_540M */
	/* divider */
	writel(0x3F << 8, ANATOP_BASE_ADDR + 0xF8);
	writel(0x10 << 8, ANATOP_BASE_ADDR + 0xF4);
	/* enable */
	writel(0x1 << 15, ANATOP_BASE_ADDR + 0xF8);

	/* ipu1_clk */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR3);
	/* source */
	reg |= (0x3 << 9);
	/* divider */
	reg &= ~(0x7 << 11);
	reg |= (0x1 << 11);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR3);

	/*
	 * ipu1_pixel_clk_x clock tree:
	 * osc_clk(24M)->pll2_528_bus_main_clk(528M)->
	 * pll2_pfd_352M(452.57M)->ldb_dix_clk(64.65M)->
	 * ipu1_di_clk_x(64.65M)->ipu1_pixel_clk_x(64.65M)
	 */
	/* pll2_528_bus_main_clk */
	/* divider */
	writel(0x1, ANATOP_BASE_ADDR + 0x34);

	/* pll2_pfd_352M */
	/* disable */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x104);
	/* divider */
	writel(0x3F, ANATOP_BASE_ADDR + 0x108);
	writel(0x15, ANATOP_BASE_ADDR + 0x104);

	/* ldb_dix_clk */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CS2CDR);
	reg &= ~(0x3F << 9);
	reg |= (0x9 << 9);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CS2CDR);
	/* divider */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCMR2);
	reg |= (0x3 << 10);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCMR2);

	/* pll2_pfd_352M */
	/* enable after ldb_dix_clk source is set */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x108);

	/* ipu1_di_clk_x */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CHSCCDR);
	reg &= ~0xE07;
	reg |= 0x803;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CHSCCDR);
#endif	/* CONFIG_MX6DL */

	/* Enable ipu1/ipu1_dix/ldb_dix clocks. */
	if (di == 1) {
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
		reg |= 0xC033;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);
	} else {
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
		reg |= 0x300F;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);
	}

	ret = ipuv3_fb_init(&lvds_xga, di, IPU_PIX_FMT_RGB666,
			DI_PCLK_LDB, 65000000);
	if (ret)
		puts("LCD cannot be configured\n");

	/*
	 * LVDS0 mux to IPU1 DI0.
	 * LVDS1 mux to IPU1 DI1.
	 */
	reg = readl(IOMUXC_BASE_ADDR + 0xC);
	reg &= ~(0x000003C0);
	reg |= 0x00000100;
	writel(reg, IOMUXC_BASE_ADDR + 0xC);

	if (di == 1)
		writel(0x40C, IOMUXC_BASE_ADDR + 0x8);
	else
		writel(0x201, IOMUXC_BASE_ADDR + 0x8);
}
#endif

#ifdef CONFIG_VIDEO_MX5
void panel_info_init(void)
{
	panel_info.vl_bpix = LCD_BPP;
	panel_info.vl_col = lvds_xga.xres;
	panel_info.vl_row = lvds_xga.yres;
	panel_info.cmap = colormap;
}
#endif

#ifdef CONFIG_SPLASH_SCREEN
void setup_splash_image(void)
{
	char *s;
	ulong addr;

	s = getenv("splashimage");

	if (s != NULL) {
		addr = simple_strtoul(s, NULL, 16);

#if defined(CONFIG_ARCH_MMU)
		addr = ioremap_nocache(iomem_to_phys(addr),
				fsl_bmp_reversed_600x400_size);
#endif
		memcpy((char *)addr, (char *)fsl_bmp_reversed_600x400,
				fsl_bmp_reversed_600x400_size);
	}
}
#endif
#endif /* !CONFIG_MXC_EPDC */

void setup_ecspi3(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT0__ECSPI3_SCLK); // ECSPI3_SCLK -- DISP0_DATA00 (0x03C4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT1__ECSPI3_MOSI); // ECSPI3_MOSI -- DISP0_DATA01 (0x03C8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT2__ECSPI3_MISO); // ECSPI3_MISO -- DISP0_DATA02 (0x03F4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT3__ECSPI3_SS0); // ECSPI3_SS0 -- DISP0_DATA03 (0x0408)
}

void setup_eim(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA0__WEIM_WEIM_DA_A_0); // EIM_AD00 -- EIM_AD00 (0x0554)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA8__WEIM_WEIM_DA_A_8); // EIM_AD08 -- EIM_AD08 (0x058C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA9__WEIM_WEIM_DA_A_9); // EIM_AD09 -- EIM_AD09 (0x0590)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB3__WEIM_WEIM_EB_3); // EIM_EB3 -- EIM_EB3 (0x05A0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_WAIT__WEIM_WEIM_WAIT); // EIM_WAIT -- EIM_WAIT (0x05B0)
}

void setup_enet(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_CRS_DV__ENET_RX_EN); // ENET_RX_EN -- ENET_CRS_DV (0x05B4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_MDC__ENET_MDC); // ENET_MDC -- ENET_MDC (0x05B8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_MDIO__ENET_MDIO); // ENET_MDIO -- ENET_MDIO (0x05BC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_REF_CLK__ENET_TX_CLK); // ENET_TX_CLK -- ENET_REF_CLK (0x05C0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_RXD0__ENET_RDATA_0); // ENET_RX_DATA0 -- ENET_RX_DATA0 (0x05C8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_RXD1__ENET_RDATA_1); // ENET_RX_DATA1 -- ENET_RX_DATA1 (0x05CC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_TX_EN__ENET_TX_EN); // ENET_TX_EN -- ENET_TX_EN (0x05D0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_TXD0__ENET_TDATA_0); // ENET_TX_DATA0 -- ENET_TX_DATA0 (0x05D4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_ENET_TXD1__ENET_TDATA_1); // ENET_TX_DATA1 -- ENET_TX_DATA1 (0x05D8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_16__ENET_ANATOP_ETHERNET_REF_OUT); // ENET_REF_CLK -- GPIO16 (0x05E4)
}

void setup_epdc(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A16__EPDC_SDDO_0); // EPDC_DATA00 -- EIM_ADDR16 (0x04E0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A17__EPDC_PWRSTAT); // EPDC_PWR_STAT -- EIM_ADDR17 (0x04E4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A18__EPDC_PWRCTRL_0); // EPDC_PWR_CTRL0 -- EIM_ADDR18 (0x04E8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A19__EPDC_PWRCTRL_1); // EPDC_PWR_CTRL1 -- EIM_ADDR19 (0x04EC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A20__EPDC_PWRCTRL_2); // EPDC_PWR_CTRL2 -- EIM_ADDR20 (0x04F0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A21__EPDC_GDCLK); // EPDC_GDCLK -- EIM_ADDR21 (0x04F4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A22__EPDC_GDSP); // EPDC_GDSP -- EIM_ADDR22 (0x04F8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A23__EPDC_GDOE); // EPDC_GDOE -- EIM_ADDR23 (0x04FC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A24__EPDC_GDRL); // EPDC_GDRL -- EIM_ADDR24 (0x0500)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_A25__EPDC_SDDO_15); // EPDC_DATA15 -- EIM_ADDR25 (0x0504)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_CS0__EPDC_SDDO_6); // EPDC_DATA06 -- EIM_CS0 (0x050C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_CS1__EPDC_SDDO_8); // EPDC_DATA08 -- EIM_CS1 (0x0510)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D16__EPDC_SDDO_10); // EPDC_DATA10 -- EIM_DATA16 (0x0514)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D17__EPDC_VCOM_0); // EPDC_VCOM0 -- EIM_DATA17 (0x0518)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D18__EPDC_VCOM_1); // EPDC_VCOM1 -- EIM_DATA18 (0x051C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D19__EPDC_SDDO_12); // EPDC_DATA12 -- EIM_DATA19 (0x0520)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D23__EPDC_SDDO_11); // EPDC_DATA11 -- EIM_DATA23 (0x0530)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D26__EPDC_SDOED); // EPDC_SDOED -- EIM_DATA26 (0x053C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D27__EPDC_SDOE); // EPDC_SDOE -- EIM_DATA27 (0x0540)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D28__EPDC_PWRCTRL_3); // EPDC_PWR_CTRL3 -- EIM_DATA28 (0x0544)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D29__EPDC_PWRWAKE); // EPDC_PWR_WAKE -- EIM_DATA29 (0x0548)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D30__EPDC_SDOEZ); // EPDC_SDOEZ -- EIM_DATA30 (0x054C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_D31__EPDC_SDCLK); // EPDC_SDCLK_P -- EIM_DATA31 (0x0550)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA1__EPDC_SDLE); // EPDC_SDLE -- EIM_AD01 (0x0558)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA10__EPDC_SDDO_1); // EPDC_DATA01 -- EIM_AD10 (0x055C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA11__EPDC_SDDO_3); // EPDC_DATA03 -- EIM_AD11 (0x0560)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA12__EPDC_SDDO_2); // EPDC_DATA02 -- EIM_AD12 (0x0564)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA13__EPDC_SDDO_13); // EPDC_DATA13 -- EIM_AD13 (0x0568)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA14__EPDC_SDDO_14); // EPDC_DATA14 -- EIM_AD14 (0x056C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA15__EPDC_SDDO_9); // EPDC_DATA09 -- EIM_AD15 (0x0570)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA2__EPDC_BDR_0); // EPDC_BDR0 -- EIM_AD02 (0x0574)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA3__EPDC_BDR_1); // EPDC_BDR1 -- EIM_AD03 (0x0578)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA4__EPDC_SDCE_0); // EPDC_SDCE0 -- EIM_AD04 (0x057C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA5__EPDC_SDCE_1); // EPDC_SDCE1 -- EIM_AD05 (0x0580)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA6__EPDC_SDCE_2); // EPDC_SDCE2 -- EIM_AD06 (0x0584)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_DA7__EPDC_SDCE_3); // EPDC_SDCE3 -- EIM_AD07 (0x0588)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB0__EPDC_PWRCOM); // EPDC_PWR_COM -- EIM_EB0 (0x0594)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB1__EPDC_SDSHR); // EPDC_SDSHR -- EIM_EB1 (0x0598)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_EB2__EPDC_SDDO_5); // EPDC_DATA05 -- EIM_EB2 (0x059C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_LBA__EPDC_SDDO_4); // EPDC_DATA04 -- EIM_LBA (0x05A4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_OE__EPDC_PWRIRQ); // EPDC_PWR_IRQ -- EIM_OE (0x05A8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_EIM_RW__EPDC_SDDO_7); // EPDC_DATA07 -- EIM_RW (0x05AC)
}

void setup_gpio1(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_0__GPIO_1_0); // GPIO1_IO00 -- GPIO00 (0x05DC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_2__GPIO_1_2); // GPIO1_IO02 -- GPIO02 (0x05F4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_3__GPIO_1_3); // GPIO1_IO03 -- GPIO03 (0x05F8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_5__GPIO_1_5); // GPIO1_IO05 -- GPIO05 (0x0600)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_6__GPIO_1_6); // GPIO1_IO06 -- GPIO06 (0x0604)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_7__GPIO_1_7); // GPIO1_IO07 -- GPIO07 (0x0608)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_8__GPIO_1_8); // GPIO1_IO08 -- GPIO08 (0x060C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_9__GPIO_1_9); // GPIO1_IO09 -- GPIO09 (0x0610)
}

void setup_gpio2(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D0__GPIO_2_0); // GPIO2_IO00 -- NAND_DATA00 (0x066C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D1__GPIO_2_1); // GPIO2_IO01 -- NAND_DATA01 (0x0670)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D2__GPIO_2_2); // GPIO2_IO02 -- NAND_DATA02 (0x0674)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D3__GPIO_2_3); // GPIO2_IO03 -- NAND_DATA03 (0x0678)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D4__GPIO_2_4); // GPIO2_IO04 -- NAND_DATA04 (0x067C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D5__GPIO_2_5); // GPIO2_IO05 -- NAND_DATA05 (0x0680)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D6__GPIO_2_6); // GPIO2_IO06 -- NAND_DATA06 (0x0684)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_D7__GPIO_2_7); // GPIO2_IO07 -- NAND_DATA07 (0x0688)
}

void setup_gpio4(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DI0_DISP_CLK__GPIO_4_16); // GPIO4_IO16 -- DI0_DISP_CLK (0x03B0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DI0_PIN15__GPIO_4_17); // GPIO4_IO17 -- DI0_PIN15 (0x03B4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DI0_PIN2__GPIO_4_18); // GPIO4_IO18 -- DI0_PIN02 (0x03B8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DI0_PIN3__GPIO_4_19); // GPIO4_IO19 -- DI0_PIN03 (0x03BC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT10__GPIO_4_31); // GPIO4_IO31 -- DISP0_DATA10 (0x03CC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT7__GPIO_4_28); // GPIO4_IO28 -- DISP0_DATA07 (0x0418)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT9__GPIO_4_30); // GPIO4_IO30 -- DISP0_DATA09 (0x0420)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_19__GPIO_4_5); // GPIO4_IO05 -- GPIO19 (0x05F0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL0__GPIO_4_6); // GPIO4_IO06 -- KEY_COL0 (0x062C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL2__GPIO_4_10); // GPIO4_IO10 -- KEY_COL2 (0x0634)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW0__GPIO_4_7); // GPIO4_IO07 -- KEY_ROW0 (0x0640)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW2__GPIO_4_11); // GPIO4_IO11 -- KEY_ROW2 (0x0648)
}

void setup_gpio5(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT12__GPIO_5_30); // GPIO5_IO30 -- CSI0_DATA12 (0x0368)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT13__GPIO_5_31); // GPIO5_IO31 -- CSI0_DATA13 (0x036C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT4__GPIO_5_22); // GPIO5_IO22 -- CSI0_DATA04 (0x0388)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT5__GPIO_5_23); // GPIO5_IO23 -- CSI0_DATA05 (0x038C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT6__GPIO_5_24); // GPIO5_IO24 -- CSI0_DATA06 (0x0390)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT7__GPIO_5_25); // GPIO5_IO25 -- CSI0_DATA07 (0x0394)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT11__GPIO_5_5); // GPIO5_IO05 -- DISP0_DATA11 (0x03D0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT12__GPIO_5_6); // GPIO5_IO06 -- DISP0_DATA12 (0x03D4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT13__GPIO_5_7); // GPIO5_IO07 -- DISP0_DATA13 (0x03D8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT14__GPIO_5_8); // GPIO5_IO08 -- DISP0_DATA14 (0x03DC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT15__GPIO_5_9); // GPIO5_IO09 -- DISP0_DATA15 (0x03E0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT16__GPIO_5_10); // GPIO5_IO10 -- DISP0_DATA16 (0x03E4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT17__GPIO_5_11); // GPIO5_IO11 -- DISP0_DATA17 (0x03E8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT18__GPIO_5_12); // GPIO5_IO12 -- DISP0_DATA18 (0x03EC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT19__GPIO_5_13); // GPIO5_IO13 -- DISP0_DATA19 (0x03F0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT20__GPIO_5_14); // GPIO5_IO14 -- DISP0_DATA20 (0x03F8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT21__GPIO_5_15); // GPIO5_IO15 -- DISP0_DATA21 (0x03FC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT22__GPIO_5_16); // GPIO5_IO16 -- DISP0_DATA22 (0x0400)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT23__GPIO_5_17); // GPIO5_IO17 -- DISP0_DATA23 (0x0404)
}

void setup_gpio6(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT14__GPIO_6_0); // GPIO6_IO00 -- CSI0_DATA14 (0x0370)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT15__GPIO_6_1); // GPIO6_IO01 -- CSI0_DATA15 (0x0374)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_CLE__GPIO_6_7); // GPIO6_IO07 -- NAND_CLE (0x0658)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_CS0__GPIO_6_11); // GPIO6_IO11 -- NAND_CS0_B (0x065C)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_CS1__GPIO_6_14); // GPIO6_IO14 -- NAND_CS1_B (0x0660)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_CS2__GPIO_6_15); // GPIO6_IO15 -- NAND_CS2_B (0x0664)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_RGMII_TD0__GPIO_6_20); // GPIO6_IO20 -- RGMII_TD0 (0x06AC)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_RGMII_TD1__GPIO_6_21); // GPIO6_IO21 -- RGMII_TD1 (0x06B0)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_RGMII_TD2__GPIO_6_22); // GPIO6_IO22 -- RGMII_TD2 (0x06B4)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_RGMII_TD3__GPIO_6_23); // GPIO6_IO23 -- RGMII_TD3 (0x06B8)
}

void setup_gpio7(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_17__GPIO_7_12); // GPIO7_IO12 -- GPIO17 (0x05E8)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_18__GPIO_7_13); // GPIO7_IO13 -- GPIO18 (0x05EC)
}

void setup_hdmi(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL3__HDMI_TX_DDC_SCL); // HDMI_TX_DDC_SCL -- KEY_COL3 (0x0638)
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW3__HDMI_TX_DDC_SDA); // HDMI_TX_DDC_SDA -- KEY_ROW3 (0x064C)
}


void setup_wdog1(){
	x_mxc_iomux_v3_setup_pad(MX6DL_PAD_DISP0_DAT8__WDOG1_WDOG_B); // WDOG1_B -- DISP0_DATA08 (0x041C)
}

int board_init(void)
{
/* need set Power Supply Glitch to 0x41736166
*and need clear Power supply Glitch Detect bit
* when POR or reboot or power on Otherwise system
*could not be power off anymore*/
	u32 reg;
	writel(0x41736166, SNVS_BASE_ADDR + 0x64);/*set LPPGDR*/
	udelay(10);
	reg = readl(SNVS_BASE_ADDR + 0x4c);
	reg |= (1 << 3);
	writel(reg, SNVS_BASE_ADDR + 0x4c);/*clear LPSR*/

	mxc_iomux_v3_init((void *)IOMUXC_BASE_ADDR);
	setup_boot_device();
	fsl_set_system_rev();

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_MX6Q_SABRESD;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	setup_uart();
	if (cpu_is_mx6q())
		setup_sata();

#ifdef CONFIG_VIDEO_MX5
	/* Enable lvds power */
	setup_lvds_poweron();

	panel_info_init();

	gd->fb_base = CONFIG_FB_BASE;
#ifdef CONFIG_ARCH_MMU
	gd->fb_base = ioremap_nocache(iomem_to_phys(gd->fb_base), 0);
#endif
#endif

	setup_eim();
	setup_ecspi3();

	setup_epdc();

	setup_gpio1();
	setup_gpio2();
	setup_gpio4();
	setup_gpio5();
	setup_gpio6();
	setup_gpio7();

	setup_wdog1();
	setup_hdmi();


#ifdef CONFIG_NAND_GPMI
	setup_gpmi_nand();
#endif




#ifdef CONFIG_MXC_EPDC
	setup_epdc();
#endif

	return 0;
}


#ifdef CONFIG_ANDROID_RECOVERY

int check_recovery_cmd_file(void)
{
	int button_pressed = 0;
	int recovery_mode = 0;

	recovery_mode = check_and_clean_recovery_flag();

	/* Check Recovery Combo Button press or not. */
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_5__GPIO_1_5));

	gpio_direction_input(GPIO_VOL_DN_KEY);

	if (gpio_get_value(GPIO_VOL_DN_KEY) == 0) { /* VOL_DN key is low assert */
		button_pressed = 1;
		printf("Recovery key pressed\n");
	}

	return recovery_mode || button_pressed;
}
#endif

int board_late_init(void)
{
	int ret = 0;
	setup_i2c(CONFIG_SYS_I2C_PORT);
	i2c_bus_recovery();

	ret = setup_pmic_voltages();
	if (ret) return -1;


	return 0;
}

#ifdef CONFIG_MXC_FEC
static int phy_read(char *devname, unsigned char addr, unsigned char reg,
		    unsigned short *pdata)
{
	int ret = miiphy_read(devname, addr, reg, pdata);
	if (ret)
		printf("Error reading from %s PHY addr=%02x reg=%02x\n",
		       devname, addr, reg);
	return ret;
}

static int phy_write(char *devname, unsigned char addr, unsigned char reg,
		     unsigned short value)
{
	int ret = miiphy_write(devname, addr, reg, value);
	if (ret)
		printf("Error writing to %s PHY addr=%02x reg=%02x\n", devname,
		       addr, reg);
	return ret;
}

int mx6_rgmii_rework(char *devname, int phy_addr)
{
	unsigned short val;

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(devname, phy_addr, 0xd, 0x7);
	phy_write(devname, phy_addr, 0xe, 0x8016);
	phy_write(devname, phy_addr, 0xd, 0x4007);
	phy_read(devname, phy_addr, 0xe, &val);

	val &= 0xffe3;
	val |= 0x18;
	phy_write(devname, phy_addr, 0xe, val);

	/* introduce tx clock delay */
	phy_write(devname, phy_addr, 0x1d, 0x5);
	phy_read(devname, phy_addr, 0x1e, &val);
	val |= 0x0100;
	phy_write(devname, phy_addr, 0x1e, val);

	return 0;
}


void enet_board_init(void)
{
	unsigned int reg;

	iomux_v3_cfg_t enet_reset =
			(MX6DL_PAD_ENET_CRS_DV__GPIO_1_25 & ~MUX_PAD_CTRL_MASK)           |
			 MUX_PAD_CTRL(0x88);

	setup_enet();
	x_mxc_iomux_v3_setup_pad(enet_reset);

	/* phy reset: gpio1-25 */
	reg = readl(GPIO1_BASE_ADDR + 0x0);
	reg &= ~0x2000000;
	writel(reg, GPIO1_BASE_ADDR + 0x0); 
	reg = readl(GPIO1_BASE_ADDR + 0x4);
	reg |= 0x2000000;
	writel(reg, GPIO1_BASE_ADDR + 0x4);

	udelay(500);

	reg = readl(GPIO1_BASE_ADDR + 0x0);
	reg |= 0x2000000;
	writel(reg, GPIO1_BASE_ADDR + 0x0);
}
#endif

int checkboard(void)
{
	printf("Board: %s-SABRESD: %s Board: 0x%x [",
	mx6_chip_name(),
	mx6_board_rev_name(),
	fsl_system_rev);

	switch (__REG(SRC_BASE_ADDR + 0x8)) {
	case 0x0001:
		printf("POR");
		break;
	case 0x0009:
		printf("RST");
		break;
	case 0x0010:
	case 0x0011:
		printf("WDOG");
		break;
	default:
		printf("unknown");
	}
	printf(" ]\n");

	printf("Boot Device: ");
	switch (get_boot_device()) {
	case WEIM_NOR_BOOT:
		printf("NOR\n");
		break;
	case ONE_NAND_BOOT:
		printf("ONE NAND\n");
		break;
	case PATA_BOOT:
		printf("PATA\n");
		break;
	case SATA_BOOT:
		printf("SATA\n");
		break;
	case I2C_BOOT:
		printf("I2C\n");
		break;
	case SPI_NOR_BOOT:
		printf("SPI NOR\n");
		break;
	case SD_BOOT:
		printf("SD\n");
		break;
	case MMC_BOOT:
		printf("MMC\n");
		break;
	case NAND_BOOT:
		printf("NAND\n");
		break;
	case UNKNOWN_BOOT:
	default:
		printf("UNKNOWN\n");
		break;
	}

#ifdef CONFIG_SECURE_BOOT
	if (check_hab_enable() == 1)
		get_hab_status();
#endif

	return 0;
}


#ifdef CONFIG_IMX_UDC

void udc_pins_setting(void)
{
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_ENET_RX_ER__ANATOP_USBOTG_ID));
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_EIM_D22__GPIO_3_22));
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_ENET_TXD1__GPIO_1_29));

	/* USB_OTG_PWR = 0 */
	gpio_direction_output(USB_OTG_PWR, 0);
	/* USB_H1_POWER = 1 */
	gpio_direction_output(USB_H1_POWER, 1);

	mxc_iomux_set_gpr_register(1, 13, 1, 0);

}
#endif
