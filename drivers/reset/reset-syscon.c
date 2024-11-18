// SPDX-License-Identifier: GPL-2.0-only
/*
 * SYSCON regmap reset driver
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>

struct syscon_reset_priv {
	struct reset_controller_dev rcdev;
	struct regmap *regmap;
	unsigned int offset;
	unsigned int mask;
	unsigned int assert_high;
};

#define to_lua_syscon_reset_data(_rcdev)	\
	container_of(_rcdev, struct syscon_reset_priv, rcdev)


static int syscon_reset_status(struct reset_controller_dev *rcdev,
				  unsigned long id)
{
	struct syscon_reset_priv *priv = to_lua_syscon_reset_data(rcdev);
	unsigned int reset_state;
	int ret;

	if (!(BIT(id) & priv->mask))
		return -EINVAL;

	ret = regmap_read(priv->regmap, priv->offset, &reset_state);
	if (ret)
		return ret;

	return (reset_state & BIT(id)) ? 1 : 0;
}

static int syscon_reset_assert(struct reset_controller_dev *rcdev,
				  unsigned long id)
{
	struct syscon_reset_priv *priv = to_lua_syscon_reset_data(rcdev);

	return regmap_update_bits(priv->regmap, priv->offset, BIT(id),
				  priv->assert_high ? BIT(id) : 0);
}

static int syscon_reset_deassert(struct reset_controller_dev *rcdev,
				  unsigned long id)
{
	struct syscon_reset_priv *priv = to_lua_syscon_reset_data(rcdev);

	return regmap_update_bits(priv->regmap, priv->offset, BIT(id),
				  priv->assert_high ? 0 : BIT(id));
}

static const struct reset_control_ops syscon_reset_ops = {
	.assert = syscon_reset_assert,
	.deassert = syscon_reset_deassert,
	.status = syscon_reset_status,
};

int syscon_reset_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct syscon_reset_priv *priv;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->regmap = syscon_regmap_lookup_by_phandle(np, "regmap");
	if (IS_ERR(priv->regmap)) {
		dev_err(dev, "unable to get syscon");
		return PTR_ERR(priv->regmap);
	}

	if (of_property_read_u32(np, "offset", &priv->offset)) {
		dev_err(&pdev->dev, "unable to read 'offset'");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "mask", &priv->mask)) {
		dev_err(&pdev->dev, "unable to read 'mask'");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "assert-high", &priv->assert_high)) {
		dev_err(&pdev->dev, "unable to read 'assert-high'");
		return -EINVAL;
	}

	priv->rcdev.ops = &syscon_reset_ops;
	priv->rcdev.owner = THIS_MODULE;
	priv->rcdev.of_node = np;
	priv->rcdev.nr_resets = fls(priv->mask);
	dev_dbg(dev, "rcdev nr_resets: %d\n", priv->rcdev.nr_resets);

	platform_set_drvdata(pdev, priv);
	if (devm_reset_controller_register(dev, &priv->rcdev)) {
		dev_err(dev, "register reset controller failed\n");
		return -ENXIO;
	}

	priv->rcdev.dev = dev;

	return 0;
}

static const struct of_device_id syscon_reset_ids[] = {
	{ .compatible = "syscon-reset" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, syscon_reset_ids);


static struct platform_driver syscon_reset_driver = {
	.probe = syscon_reset_probe,
	.driver = {
		.name = "syscon-reset",
		.of_match_table = syscon_reset_ids,
	},
};
module_platform_driver(syscon_reset_driver);

MODULE_AUTHOR("Charleye <wangkart@aliyun.com>");
MODULE_DESCRIPTION("SYSCON Regmap Reset Driver");
MODULE_LICENSE("GPL v2");
