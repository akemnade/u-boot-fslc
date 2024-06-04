// SPDX-License-Identifier: GPL-2.0

#include <common.h>
#include <button.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <log.h>
#include <power/pmic.h>
#include <dt-bindings/input/linux-event-codes.h>

struct button_bd71828_priv {
	bool down;
};

static enum button_state_t button_bd71828_get_state(struct udevice *dev)
{
	struct udevice *pmic = dev_get_parent(dev);
	struct button_bd71828_priv *priv = dev_get_priv(dev);
	int ret;

	ret = pmic_reg_read(pmic, 0x02);
	if (ret < 0)
		return ret;

	printf("reg 02: %x\n", ret);
	/*
	if ((ret & 0x02) && ! (ret & 0x01))
		return BUTTON_OFF;
	*/

	ret = pmic_reg_read(pmic, 0xe2);
	if (ret < 0)
		return ret;

	printf("reg 02: %x\n", ret);
	if (ret & 0x20)
		priv->down = true;

	if (ret & 0x10)
		priv->down = false;

	if (ret & 0x08)
		priv->down = true;

	ret &= 0x3C;

	pmic_reg_write(pmic, 0xe2, ret);

	return priv->down ? BUTTON_ON : BUTTON_OFF;
}

static int button_bd71828_get_code(struct udevice *dev)
{
	return KEY_POWER;
}

static int button_bd71828_probe(struct udevice *dev)
{
	struct udevice *pmic = dev_get_parent(dev);
	pr_info("bd button probe\n");
	struct button_bd71828_priv *priv = dev_get_priv(dev);
	pmic_reg_write(pmic, 0xe2, 0x38);
	priv->down = false;
	return 0;
}

static int button_bd71828_remove(struct udevice *dev)
{
	return 0;
}

static int button_bd71828_bind(struct udevice *dev)
{
	struct button_uc_plat *uc_plat;

	pr_info("bd button bind\n");
	uc_plat = dev_get_uclass_plat(dev);
	uc_plat->label = "Power";

	return 0;
}

static const struct button_ops button_bd71828_ops = {
	.get_state	= button_bd71828_get_state,
	.get_code	= button_bd71828_get_code,
};

U_BOOT_DRIVER(button_bd71828) = {
	.name		= "button_bd71828",
	.id		= UCLASS_BUTTON,
	.ops		= &button_bd71828_ops,
	.priv_auto	= sizeof(struct button_bd71828_priv),
	.bind		= button_bd71828_bind,
	.probe		= button_bd71828_probe,
	.remove		= button_bd71828_remove,
};
