// SPDX-License-Identifier: GPL-2.0+
/*
 * syscon based AB control driver
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "abc-syscon: " fmt
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

struct abc_syscon_platdata {
	struct device *dev;
	struct regmap *regmap;
	uint32_t offset;
};

static ssize_t
value_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct abc_syscon_platdata *priv = dev_get_drvdata(dev);
	uint32_t val;
	int ret;

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0) {
		dev_err(dev, "Invalid value: %s\n", buf);
		return ret;
	}

	ret = regmap_write(priv->regmap, priv->offset, val);
	if (ret < 0) {
		dev_err(dev, "Unable to write value to syscon (ret = %d)\n", ret);
		return ret;
	}
	pr_debug("%s: wrote value 0x%x to offset 0x%x\n", __func__, val, priv->offset);

	return count;
}

static ssize_t
value_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct abc_syscon_platdata *priv = dev_get_drvdata(dev);
	uint32_t val;
	int ret;

	ret = regmap_read(priv->regmap, priv->offset, &val);
	if (ret < 0) {
		dev_err(dev, "Unable to read value from syscon (ret = %d)\n", ret);
		return ret;
	}

	pr_debug("%s: read value 0x%x from offset 0x%x\n", __func__, val, priv->offset);
	ret = scnprintf(page, PAGE_SIZE - 1, "0x%x\n", val);

	return ret;
}
static DEVICE_ATTR(value, 0664, value_show, value_store);

static struct attribute *abc_syscon_attrs[] = {
	&dev_attr_value.attr,
	NULL,
};
ATTRIBUTE_GROUPS(abc_syscon);

static int abc_syscon_probe(struct platform_device *pdev)
{
	struct abc_syscon_platdata *priv = NULL;
	struct device *dev = &pdev->dev;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (IS_ERR_OR_NULL(priv)) {
		dev_err(dev, "Out of memory\n");
		return -ENOMEM;
	}

	priv->regmap = syscon_regmap_lookup_by_phandle(dev->of_node, "regmap");
	if (IS_ERR(priv->regmap)) {
		dev_err(dev, "Unable to get syscon regmap\n");
		return PTR_ERR(priv->regmap);
	}

	if (of_property_read_u32(dev->of_node, "offset", &priv->offset)) {
		dev_err(dev, "Unable to read 'offset' property\n");
		return -EINVAL;
	}

	priv->dev = dev;
	platform_set_drvdata(pdev, priv);

	pr_debug("%s: %s done\n", __func__, dev_name(dev));
	return 0;
}

static int abc_syscon_remove(struct platform_device *pdev)
{
	struct abc_syscon_platdata *pdata = platform_get_drvdata(pdev);

	if (pdata) {
		if (pdata->regmap)
			regmap_exit(pdata->regmap);
		devm_kfree(&pdev->dev, pdata);
	}

	return 0;
}

static const struct of_device_id abc_syscon_ids[] = {
	{ .compatible = "axera,laguna-abc-syscon" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, abc_syscon_ids);

static struct platform_driver abc_syscon_driver = {
	.probe = abc_syscon_probe,
	.remove = abc_syscon_remove,
	.driver = {
		.name = "abc-syscon",
		.of_match_table = abc_syscon_ids,
		.dev_groups = abc_syscon_groups,
	},
};
module_platform_driver(abc_syscon_driver);