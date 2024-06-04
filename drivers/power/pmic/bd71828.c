// SPDX-License-Identifier:      GPL-2.0+
/*
 * based on bd71837 which is
 * Copyright 2018 NXP
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <dm/lists.h>
#include <i2c.h>
#include <log.h>
#include <asm/global_data.h>
#include <power/pmic.h>
#include <power/bd71837.h>


DECLARE_GLOBAL_DATA_PTR;

static int bd71828_reg_count(struct udevice *dev)
{
	return BD718XX_MAX_REGISTER - 1;
}

static int bd71828_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int bd71828_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int bd71828_bind(struct udevice *dev)
{
	struct udevice *gpio_dev;
	int ret;

	device_bind_driver(dev, "button_bd71828", "button_bd71828", &gpio_dev);

        pr_info("bd71828_bind\n");
	/* Always return success for this device */
	return 0;
}

static int bd718x7_probe(struct udevice *dev)
{
	//int ret;
        pr_info("bd71828_probe\n");
	return 0;
}

static struct dm_pmic_ops bd71828_ops = {
	.reg_count = bd71828_reg_count,
	.read = bd71828_read,
	.write = bd71828_write,
};

static const struct udevice_id bd71828_ids[] = {
	{ .compatible = "rohm,bd71828" },
	{ }
};

U_BOOT_DRIVER(pmic_bd71828) = {
	.name = "bd71828 pmic",
	.id = UCLASS_PMIC,
	.of_match = bd71828_ids,
	.bind = bd71828_bind,
	.probe = bd718x7_probe,
	.ops = &bd71828_ops,
};
