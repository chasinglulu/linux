// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera Laguna SoC Fixed Factor Clock driver
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */
#define DEBUG
#define pr_fmt(fmt) "fixed-factor-clk: " fmt

#include <linux/clk-provider.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>

static int fixed_factor_clk_bind(struct device_node *np)
{
	struct device_node *subnode;
	struct of_phandle_args args;
	uint32_t div, mult;
	struct clk_hw *hw;
	const char *name;
	ulong flags = 0;
	int ret;

	for_each_child_of_node(np, subnode) {
		name = of_node_full_name(subnode);

		ret = of_property_read_u32_index(subnode, "clock-mult",
					0, &mult);
		if (unlikely(ret)) {
			pr_err("Failed to get 'clock-mult' in %s node\n", name);
			continue;
		}

		ret = of_property_read_u32_index(subnode, "clock-div",
					0, &div);
		if (unlikely(ret)) {
			pr_err("Failed to get 'clock-div' in %s node\n", name);
			continue;
		}

		ret = of_parse_phandle_with_args(subnode, "clocks",
					"#clock-cells", 0, &args);
		if (unlikely(ret)) {
			pr_err("Failed to get parent clk of %s\n ret=%d", name, ret);
			continue;
		}

		flags |= CLK_SET_RATE_PARENT;
		if (of_property_read_bool(subnode, "laguna,clk-is-critical"))
			flags |= CLK_IS_CRITICAL;

		hw = clk_hw_register_fixed_factor(NULL, name,
		        of_node_full_name(args.np),
		                    flags, mult, div);
		if (IS_ERR(hw)) {
			pr_warn("Failed to register '%s' clk\n", name);
			continue;
		}

		ret = of_clk_add_hw_provider(subnode, of_clk_hw_simple_get, hw);
		if (ret) {
			clk_hw_unregister_gate(hw);
		}
	}

	pr_debug("Register '%s' fixed factor clocks successfully\n", of_node_full_name(np));
	return 0;
}

static void __init of_ff_clk_setup(struct device_node *node)
{
	fixed_factor_clk_bind(node);
}
CLK_OF_DECLARE(axera_div_clk, "axera,lua-fixed-factor-clocks", of_ff_clk_setup);