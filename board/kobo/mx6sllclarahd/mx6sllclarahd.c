// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Andreas Kemnade
 * based on mx6sllevk 
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/io.h>
#include <common.h>
#include <command.h>
#include <linux/sizes.h>
#include <env.h>
#include <mmc.h>
#include <power/pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TXD__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RXD__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	MX6_PAD_WDOG_B__WDOG1_B | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#ifdef CONFIG_DM_PMIC_PFUZE100
int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	u32 dev_id, rev_id, i;
	u32 switch_num = 6;

	return 0;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	return 0;
}

int checkboard(void)
{
	puts("Board: MX6SLL Kobo Clara HD\n");

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return CONFIG_SYS_MMC_ENV_DEV;
}

int mmc_map_to_kernel_blk(int devno)
{
	return devno;
}

int do_detect_clara_rev(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct udevice *pmic = NULL;
	int dcdc[5];
	int i;

	i2c_get_chip_for_busnum(2, 0x32, 1, &pmic);
	if (!pmic) {
		puts("cannot find PMIC\n");
		return CMD_RET_FAILURE;
	}

	for(i = 0; i < 5; i++) {
		dcdc[i] = dm_i2c_reg_read(pmic, 0x36 + i);
		if (dcdc[i] < 0) {
			puts("cannot read dcdc volts\n");
			return CMD_RET_FAILURE;
		}
		dcdc[i] = dcdc[i] * 50 / 4 + 600; /* convert to mV */
		printf("DCDC%d at %d mV\n", i + 1, dcdc[i]);
	}

	/* Rev A: 1200 (arm) 3300 1300 (soc) 1200 1800 */
	if ((dcdc[0] < 1400) && (dcdc[1] == 3300) && (dcdc[2] < 1400) && (dcdc[3] <1400) && (dcdc[4] == 1800)) {
		puts("rev A detected\n");
		env_set("fdtfile", "imx6sll-kobo-clarahd.dtb");
		return 0;
	}

	/* Rev B: 1100 1000 3300 1800 1100 */
	if ((dcdc[0] < 1400) && (dcdc[1] < 1400) && (dcdc[2] == 3300) && (dcdc[3] == 1800) && (dcdc[4] < 1400)) {
		puts("rev B detected\n");
		env_set("fdtfile", "imx6sll-kobo-clarahd-b.dtb");
		return 0;
	}

	puts("regulators do not match any pattern, please report!\n");
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
        detect_clara_rev, 1, 0,    do_detect_clara_rev,
        "Detect Kobo Clara HD board revision",
        ""
);
