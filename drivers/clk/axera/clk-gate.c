// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera Laguna SoC gate clock support
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "gate-clk: " fmt

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/spinlock_types.h>

DEFINE_SPINLOCK(clk_gate_lock);

static int gate_clk_bind(struct device_node *node)
{
	struct of_phandle_args args;
	struct device_node *subnode;
	struct clk_hw *hw;
	const char *name;
	void __iomem *reg;
	struct device_node *regmap;
	uint32_t offset;
	uint32_t bit_idx;
	int ret;

	regmap = of_parse_phandle(node, "regmap", 0);
	if (IS_ERR(regmap)) {
		pr_err("Failed to get 'regmap' property\n");
		return -ENXIO;
	}
	if (!of_device_is_available(regmap))
		return -ENXIO;
	pr_debug("regmap node name: %s\n", regmap->full_name);
	reg = of_iomap(regmap, 0);

	ret = of_property_read_u32_index(node, "offset", 0, &offset);
	if (ret) {
		pr_err("Failed to get 'offset' property\n");
		return -EINVAL;
	}

	for_each_child_of_node(node, subnode) {
		name = of_node_full_name(subnode);

		ret = of_property_read_u32_index(subnode, "bit-shift",
					0, &bit_idx);
		if (unlikely(ret)) {
			pr_err("Failed to get 'bit-shift' in %s node\n", name);
			continue;
		}
		ret = of_parse_phandle_with_args(subnode, "clocks",
					"#clock-cells", 0, &args);
		if (unlikely(ret)) {
			pr_err("Failed to get parent clk of %s\n ret=%d", name, ret);
			continue;
		}

		hw = clk_hw_register_gate(NULL, name, of_node_full_name(args.np),
				0, reg + offset, bit_idx, 0, &clk_gate_lock);
		if (IS_ERR(hw))
			pr_warn("Failed to register '%s' clk\n", name);

		ret = of_clk_add_hw_provider(subnode, of_clk_hw_simple_get, hw);
		if (ret) {
			clk_hw_unregister_gate(hw);
		}
	}

	pr_debug("Register '%s' gate clocks successfully\n", of_node_full_name(node));
	return 0;
}


static void __init of_gate_clk_setup(struct device_node *node)
{
	gate_clk_bind(node);
}
CLK_OF_DECLARE(axera_gate_clk, "axera,lua-gate-clocks", of_gate_clk_setup);
