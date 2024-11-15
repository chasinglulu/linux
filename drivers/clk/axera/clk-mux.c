// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera Laguna SoC mux clock support
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "mux-clk: " fmt

#include <linux/clk-provider.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>

DEFINE_SPINLOCK(clk_mux_lock);

static int mux_clk_bind(struct device_node *np)
{
	struct device_node *subnode, *regmap;
	const char *name;
	struct clk_hw *hw;
	void __iomem *reg;
	uint32_t shift, width, count;
	const char **parents;
	ulong flags = 0;
	uint32_t offset;
	int i, ret;

	regmap = of_parse_phandle(np, "regmap", 0);
	if (IS_ERR(regmap)) {
		pr_err("Failed to get 'regmap' property\n");
		return -ENXIO;
	}
	if (!of_device_is_available(regmap))
		return -ENXIO;
	pr_debug("regmap node name: %s\n", of_node_full_name(regmap));
	reg = of_iomap(regmap, 0);

	ret = of_property_read_u32_index(np, "offset", 0, &offset);
	if (ret) {
		pr_err("Failed to get 'offset' property\n");
		return -EINVAL;
	}

	for_each_child_of_node(np, subnode) {
		name = of_node_full_name(subnode);

		ret = of_property_read_u32_index(subnode, "bit-shift",
					0, &shift);
		if (ret) {
			pr_err("Failed to get 'bit-shift' in %s node\n", name);
			continue;
		}
		ret = of_property_read_u32_index(subnode, "bit-width",
					0, &width);
		if (ret) {
			pr_err("Failed to get 'bit-width' in %s node\n", name);
			continue;
		}

		count = of_count_phandle_with_args(subnode, "clocks",
				"#clock-cells");
		parents = kzalloc(count * sizeof(parents), GFP_KERNEL);
		if (IS_ERR_OR_NULL(parents)) {
			pr_err("out of memory\n");
			continue;
		}

		for (i = 0; i < count; i++) {
			struct of_phandle_args args;
			ret = of_parse_phandle_with_args(subnode, "clocks",
					"#clock-cells", i, &args);
			if (ret)
				pr_warn("Failed to get parent clk@%d\n ret=%d", i, ret);

			parents[i] = of_node_full_name(args.np);
		}

		flags |= CLK_SET_RATE_PARENT;
		if (of_property_read_bool(subnode, "laguna,clk-is-critical"))
			flags |= CLK_IS_CRITICAL;

		hw = clk_hw_register_mux(NULL, name, parents, count,
				flags, reg + offset, shift, width, 0, &clk_mux_lock);
		if (IS_ERR(hw))
			pr_warn("Failed to register '%s' clk\n", name);
		ret = of_clk_add_hw_provider(subnode, of_clk_hw_simple_get, hw);
		if (ret) {
			clk_hw_unregister_mux(hw);
		}

		kfree(parents);
	}

	pr_debug("Register '%s' mux clocks successfully\n", of_node_full_name(np));
	return 0;
}

static void __init of_mux_clk_setup(struct device_node *node)
{
	mux_clk_bind(node);
}
CLK_OF_DECLARE(axera_mux_clk, "axera,lua-mux-clocks", of_mux_clk_setup);
