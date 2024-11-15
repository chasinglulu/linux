// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera Laguna SoC divider clock support
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "divider-clk: " fmt

#include <linux/clk-provider.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>

#define CLK_DIVIVER_UPDATE_BIT   BIT(31)
DEFINE_SPINLOCK(clk_divider_lock);

static inline u32 clk_div_readl(struct clk_divider *divider)
{
	if (divider->flags & CLK_DIVIDER_BIG_ENDIAN)
		return ioread32be(divider->reg);

	return readl(divider->reg);
}

static inline void clk_div_writel(struct clk_divider *divider, u32 val)
{
	if (divider->flags & CLK_DIVIDER_BIG_ENDIAN)
		iowrite32be(val, divider->reg);
	else
		writel(val, divider->reg);
}

static unsigned long clk_divider_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	struct clk_divider *divider = to_clk_divider(hw);
	unsigned int val;

	val = clk_div_readl(divider) >> divider->shift;
	val &= clk_div_mask(divider->width);

	return divider_recalc_rate(hw, parent_rate, val, divider->table,
	                   divider->flags, divider->width);
}

static int clk_divider_set_rate(struct clk_hw *hw, unsigned long rate,
                    unsigned long parent_rate)
{
	struct clk_divider *divider = to_clk_divider(hw);
	int value;
	unsigned long flags = 0;
	u32 val, bit_idx;

	value = divider_get_val(rate, parent_rate, divider->table,
				divider->width, divider->flags);
	if (value < 0)
		return value;

	if (divider->lock)
		spin_lock_irqsave(divider->lock, flags);
	else
		__acquire(divider->lock);

	if (divider->flags & CLK_DIVIDER_HIWORD_MASK) {
		val = clk_div_mask(divider->width) << (divider->shift + 16);
	} else {
		val = clk_div_readl(divider);
		val &= ~(clk_div_mask(divider->width) << divider->shift);
	}
	val |= (u32)value << divider->shift;
	clk_div_writel(divider, val);

	/* Update Flags */
	if (clk_hw_get_flags(hw) & CLK_DIVIVER_UPDATE_BIT) {
		val = clk_div_readl(divider);
		bit_idx = (divider->flags >> 24) & 0x1F;
		val &= ~BIT(bit_idx);
		val |= BIT(bit_idx);

		clk_div_writel(divider, val);
	}

	if (divider->lock)
		spin_unlock_irqrestore(divider->lock, flags);
	else
		__release(divider->lock);

	return 0;
}

static long clk_divider_round_rate(struct clk_hw *hw, unsigned long rate,
                                    unsigned long *prate)
{
	struct clk_divider *divider = to_clk_divider(hw);

	/* if read only, just return current value */
	if (divider->flags & CLK_DIVIDER_READ_ONLY) {
		u32 val;

		val = clk_div_readl(divider) >> divider->shift;
		val &= clk_div_mask(divider->width);

		return divider_ro_round_rate(hw, rate, prate, divider->table,
		                        divider->width, divider->flags,
		                        val);
	}

	return divider_round_rate(hw, rate, prate, divider->table,
	                  divider->width, divider->flags);
}

static int clk_divider_determine_rate(struct clk_hw *hw,
                            struct clk_rate_request *req)
{
	struct clk_divider *divider = to_clk_divider(hw);

	/* if read only, just return current value */
	if (divider->flags & CLK_DIVIDER_READ_ONLY) {
		u32 val;

		val = clk_div_readl(divider) >> divider->shift;
		val &= clk_div_mask(divider->width);

		return divider_ro_determine_rate(hw, req, divider->table,
		                                   divider->width,
		                                   divider->flags, val);
	}

	return divider_determine_rate(hw, req, divider->table, divider->width,
	                                divider->flags);
}

const struct clk_ops ax_clk_divider_ops = {
	.recalc_rate = clk_divider_recalc_rate,
	.round_rate = clk_divider_round_rate,
	.determine_rate = clk_divider_determine_rate,
	.set_rate = clk_divider_set_rate,
};

static struct clk_hw *_clk_hw_register_divider_table(struct device *dev,
            struct device_node *np, const char *name,
            const char *parent_name, const struct clk_hw *parent_hw,
            const struct clk_parent_data *parent_data, unsigned long flags,
            void __iomem *reg, u8 shift, u8 width, u8 clk_divider_flags,
            const struct clk_div_table *table, spinlock_t *lock)
{
	struct clk_divider *div;
	struct clk_hw *hw;
	struct clk_init_data init = {};
	int ret;

	if (clk_divider_flags & CLK_DIVIDER_HIWORD_MASK) {
		if (width + shift > 16) {
			pr_warn("divider value exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	/* allocate the divider */
	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &ax_clk_divider_ops;
	init.flags = flags;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.parent_hws = parent_hw ? &parent_hw : NULL;
	init.parent_data = parent_data;
	if (parent_name || parent_hw || parent_data)
		init.num_parents = 1;
	else
		init.num_parents = 0;

	/* struct clk_divider assignments */
	div->reg = reg;
	div->shift = shift;
	div->width = width;
	div->flags = clk_divider_flags;
	div->lock = lock;
	div->hw.init = &init;
	div->table = table;

	/* register the clock */
	hw = &div->hw;
	ret = clk_hw_register(dev, hw);
	if (ret) {
		kfree(div);
		hw = ERR_PTR(ret);
	}

	return hw;
}

static struct clk *_clk_register_divider(struct device *dev, const char *name,
            const char *parent_name, unsigned long flags,
            void __iomem *reg, u8 shift, u8 width,
            u8 clk_divider_flags)
{
	struct clk_hw *hw;

	hw = _clk_hw_register_divider_table(dev, NULL, name, parent_name, NULL,
	             NULL, flags, reg, shift, width, clk_divider_flags,
	             NULL, &clk_divider_lock);
	if (IS_ERR(hw))
		return ERR_CAST(hw);
	return hw->clk;
}

static int divider_clk_bind(struct device_node *np)
{
	struct device_node *subnode, *regmap;
	struct of_phandle_args args;
	struct clk *clk;
	const char *name;
	void __iomem *reg;
	uint32_t shift, width, update;
	uint32_t divider_flags = 0;
	uint32_t offset;
	ulong flags = 0;
	int ret;

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

		ret = of_property_read_u32_index(subnode, "bit-update",
		          0, &update);
		if (ret) {
			pr_err("Failed to get 'bit-update' in %s node\n", name);
			continue;
		}
		divider_flags &= ~0xFF000000U;
		divider_flags |= update << 24;

		ret = of_parse_phandle_with_args(subnode, "clocks",
		               "#clock-cells", 0, &args);
		if (unlikely(ret)) {
			pr_err("Failed to get parent clk of %s\n ret=%d", name, ret);
			continue;
		}

		flags |= CLK_SET_RATE_PARENT;
		flags |= CLK_DIVIVER_UPDATE_BIT;
		if (of_property_read_bool(subnode, "laguna,clk-is-critical"))
			flags |= CLK_IS_CRITICAL;

		clk = _clk_register_divider(NULL, name, of_node_full_name(args.np),
		             flags, reg + offset,
		             shift, width, divider_flags);
		if (IS_ERR(clk))
			pr_warn("Failed to register '%s' clk\n", name);
	}

	return 0;
}

static void __init of_divider_clk_setup(struct device_node *node)
{
	divider_clk_bind(node);
}
CLK_OF_DECLARE(axera_div_clk, "axera,lua-divider-clocks", of_divider_clk_setup);
