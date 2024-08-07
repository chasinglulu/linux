// SPDX-License-Identifier: GPL-2.0
/*
 * Subsystem Global Register Controller driver for Laguna SoCs
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/glbcon.h>

struct lua_sysctl_priv {
	struct subsysctl_dev scdev;
	struct regmap *regmap;
	uint32_t offset;
	uint32_t mask;
	uint32_t field_width;
	struct mutex lock;
};

#define to_lua_sysctl_data(_scdev)	\
	container_of(_scdev, struct lua_sysctl_priv, scdev)

static int lua_sysctl_status(struct subsysctl_dev *scdev,
			    uint32_t shift, uint32_t width, uint32_t data)
{
	struct lua_sysctl_priv *priv = to_lua_sysctl_data(scdev);
	uint16_t len = width ?: priv->field_width;
	uint32_t val;
	uint32_t mask;

	if (len >= 32 || len + shift >= 32)
		return -EINVAL;

	mask = ((~0U) >> (32 - len)) << shift;
	if (!(priv->mask & mask))
		return -EINVAL;

	mutex_lock(&priv->lock);
	regmap_read(priv->regmap, priv->offset, &val);
	val &= mask;
	mutex_unlock(&priv->lock);

	if ((val >> shift) == data)
		return 1;

	return 0;
}

static int lua_sysctl_write(struct subsysctl_dev *scdev,
			    uint32_t shift, uint32_t width, uint32_t data)
{
	struct lua_sysctl_priv *priv = to_lua_sysctl_data(scdev);
	uint16_t len = width ?: priv->field_width;
	uint32_t val;
	uint32_t mask;

	if (len >= 32 || len + shift >= 32)
		return -EINVAL;

	mask = ((~0U) >> (32 - len)) << shift;
	if (!(priv->mask & mask))
		return -EINVAL;

	mutex_lock(&priv->lock);
	regmap_read(priv->regmap, priv->offset, &val);
	val &= ~mask;
	val |= data << shift;

	regmap_write(priv->regmap, priv->offset, val);
	mutex_unlock(&priv->lock);

	return 0;
}

static const struct subsysctl_ops lua_sysctl_ops = {
	.status = lua_sysctl_status,
	.write = lua_sysctl_write,
};

static int lua_sysctl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct lua_sysctl_priv *priv;

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

	if (of_property_read_u32(np, "field-width", &priv->field_width)) {
		dev_err(&pdev->dev, "unable to read 'field-width'");
		return -EINVAL;
	}

	mutex_init(&priv->lock);
	priv->scdev.ops = &lua_sysctl_ops;
	priv->scdev.owner = THIS_MODULE;
	priv->scdev.of_node = np;
	priv->scdev.nr_syscons = fls(priv->mask);
	priv->scdev.dev = &pdev->dev;

	platform_set_drvdata(pdev, priv);

	return devm_subsysctl_dev_register(dev, &priv->scdev);
}

static const struct of_device_id lua_sysctl_ids[] = {
	{ .compatible = "axera,laguna-sysctl" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, lua_sysctl_ids);

static struct platform_driver lua_sysctl_driver = {
	.probe = lua_sysctl_probe,
	.driver = {
		.name = "sysctl",
		.of_match_table = lua_sysctl_ids,
	},
};
module_platform_driver(lua_sysctl_driver);

MODULE_AUTHOR("Charleye <wangkart@aliyun.com>");
MODULE_DESCRIPTION("Subsystem Global Register Contrller Driver");
MODULE_LICENSE("GPL v2");
