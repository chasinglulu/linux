// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera Laguna SoC PLL clock driver
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "pll-clk: " fmt

#include <linux/clk-provider.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>

#define PLL_DRV_NAME "clk_pll"

enum {
	PLL_REG,
	PLL_ON_CFG,
	PLL_RDY,
	PLL_FRC_EN,
	PLL_FRC_EN_SW,
	PLL_REOPEN,
	PLL_REG_CNT,
};

#define PLL_STAT_DIGCK_MISS     BIT(6)
#define PLL_STAT_FBKCK_MISS     BIT(5)
#define PLL_STAT_REFCK_MISS     BIT(4)
#define PLL_STAT_FM_UNDER       BIT(3)
#define PLL_STAT_FM_OVER        BIT(2)
#define PLL_STAT_FM_CPLT        BIT(1)
#define PLL_STAT_LOCKED         BIT(0)

#define PLL_CFG0_FBK_FRA_MASK     GENMASK(25, 2)
#define PLL_CFG0_FBK_FRA_SHIFT    2
#define PLL_CFG0_FBK_CHG          BIT(1)
#define PLL_CFG0_BYP_MODE         BIT(0)

#define PLL_CFG1_CKMON_EN         BIT(25)
#define PLL_CFG1_POST_DIV_MASK    GENMASK(24, 23)
#define PLL_CFG1_POST_DIV_SHIFT   23
#define PLL_CFG1_WATCHDOG_EN      BIT(22)
#define PLL_CFG1_TEST_SEL_MASK    GENMASK(21, 19)
#define PLL_CFG1_TEST_SEL_SHIFT   19
#define PLL_CFG1_PRE_DIV_MASK     GENMASK(18, 17)
#define PLL_CFG1_PRE_DIV_SHIFT    17
#define PLL_CFG1_MON_CTRL_MASK    GENMASK(16, 15)
#define PLL_CFG1_MON_CTRL_SHIFT   15
#define PLL_CFG1_LDO_STB_X2_EN    BIT(14)
#define PLL_CFG1_LDO_STB_MASK     GENMASK(13, 12)
#define PLL_CFG1_LDO_STB_SHIFT    12
#define PLL_CFG1_FM_TOR_MASK      GENMASK(11, 10)
#define PLL_CFG1_FM_TOR_SHIFT     10
#define PLL_CFG1_FM_EN            BIT(9)
#define PLL_CFG1_FBK_INT_MASK     GENMASK(8, 0)
#define PLL_CFG1_FBK_INT_SHIFT    0

struct pll_reg {
	uint32_t stat;
	uint32_t cfg0;
	uint32_t rsvd0[2];
	uint32_t cfg1;
	uint32_t rsvd1[2];
	uint32_t ssc0;
	uint32_t rsvd2[2];
	uint32_t ssc1;
	uint32_t rsvd3[2];
};

static const char *pll_prop[] = {
	"pll-reg-offset",
	"pll-on-cfg",
	"pll-rdy-bit",
	"pll-frc-en-bit",
	"pll-frc-en-sw-bit",
	"pll-reopen-bit",
};

static const char *pll_reg_prop[] = {
	"pll-ready",
	"pll-frc-en",
	"pll-frc-en-sw",
	"pll-reopen",
};

/*
 * Calculation formula for frequency:
 *
 * fpfd = fref / 2 ^ (PRE_DIV[1:0])
 * fpll = fpfd * (FBK_INT[8:0] + FBK_FRA[23:0] / 2 ^ 24)
 * fvco = fpll / 2 ^ (POST_DIV[1:0])
 *
 */
static __maybe_unused
ulong pll_clac_rate(struct pll_reg *reg, ulong parent_rate)
{
	uint8_t pre_div, post_div;
	uint32_t cfg;
	uint16_t fbk_int;
	uint32_t fbk_fra;
	ulong rate;

	cfg = readl(&reg->cfg1);

	pre_div = (cfg & PLL_CFG1_PRE_DIV_MASK) >> PLL_CFG1_PRE_DIV_SHIFT;
	post_div = (cfg & PLL_CFG1_POST_DIV_MASK) >> PLL_CFG1_POST_DIV_SHIFT;
	fbk_int = (cfg & PLL_CFG1_FBK_INT_MASK) >> PLL_CFG1_FBK_INT_SHIFT;

	cfg = readl(&reg->cfg0);
	fbk_fra = (cfg & PLL_CFG0_FBK_FRA_MASK) >> PLL_CFG0_FBK_FRA_SHIFT;

	rate = parent_rate >> pre_div;
	rate = rate * (fbk_int + (fbk_fra >> 24));
	rate >>= post_div;

	return rate;
}

static __maybe_unused
ulong pll_fixed_rate(ulong base, int id)
{
	ulong rate = 0;

	base &= 0xFFFFF000;

	if (base == 0x18001000) {
		/* main pllctrl */
		switch (id) {
		case 0: // fab_pll
			rate = 2400000000UL;
			break;
		case 1: // cpu_pll
			rate = 1296000000UL;
			break;
		case 2: // npu_pll
			rate = 996000000UL;
			break;
		case 3: // apll
			rate = 2700000000UL;
			break;
		case 4: // epll
			rate = 1500000000UL;
			break;
		}
	} else if (base == 0x006C2000) {
		/* safety pllctrl */
		switch (id) {
		case 0: // safe_epll
			rate = 1500000000UL;
			break;
		case 1: // safe_rpll
			rate = 996000000UL;
			break;
		case 2: // safe_fab_pll
			rate = 2400000000UL;
			break;
		}
	} else {
		pr_err("Not supported PLL Controller device (base = 0x%lx)\n", base);
	}

	return rate;
}

struct pll_clock_priv {
	/*
	 * 0: pll_ctrlreg address
	 * 1: pll_on_cfg address
	 * 2: pll_rdy address
	 * 3: pll_frc_en address
	 * 4: pll_frc_en_sw address
	 * 5: pll_reopen address
	 */
	void __iomem *addr[PLL_REG_CNT];
	phys_addr_t pllc_base;
	uint8_t rdy_bit;
	uint8_t frc_en_bit;
	uint8_t frc_en_sw_bit;
	uint8_t reopen_bit;
};

struct pll_clock_plat {
	uint8_t pll_count;
	struct pll_clock_priv pll_priv[];
};

struct lua_pll_clk {
	struct clk_hw hw;
	struct pll_clock_priv *priv;
};
#define to_pll_clk(_clk) container_of(_clk, struct lua_pll_clk, hw)

int __maybe_unused
pll_clk_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
#if !defined(CONFIG_LUA_PLL_SET_RATE) || defined(CONFIG_LUA_PLL_FIXED_FREQ)
	/* FIXME: TODO */
	pr_err("Not allow seting PLL rate.\n");
#else
	struct lua_pll_clk *pll = to_pll_clk(clk);
	struct pll_clock_priv *priv = pll->priv;
	struct pll_reg *reg = priv->addr[PLL_REG];
	ulong dividend, remainder;
	uint32_t cfg;

	if (!(clk_hw_get_flags(hw) & CLK_SET_RATE_PARENT)) {
		pr_err("Disallow to propagate rate change up one level\n");
		return -EINVAL;
	}

	/* clear reopen and on bit */
	clrbits_le32(priv->addr[PLL_REOPEN], BIT(priv->reopen_bit));
	clrbits_le32(priv->addr[PLL_ON_CFG], BIT(0));

	/*
	 * FBK_INT configure the integer part of frequency
	 * FBK_FRA configure the fraction part of frequency
	 *    - Currently not supported
	 */

	/* Clear POST_DIV, PRE_DIV and FBK_INT */
	cfg = readl(&reg->cfg1);
	cfg &= ~(PLL_CFG1_POST_DIV_MASK | PLL_CFG1_PRE_DIV_MASK | PLL_CFG1_FBK_INT_MASK);
	writel(cfg, &reg->cfg1);

	dividend = rate;
	remainder = do_div(dividend, parent_rate);

	/*
	 * four case:
	 * 1. PRE_DIV = 0, POST_DIV = 0
	 * 2. PRE_DIV = 1, POST_DIV = 0
	 * 3. PRE_DIV = 0, POST_DIV = 1
	 * 2. PRE_DIV = 1, POST_DIV = 1
	 *
	 */
	cfg = readl(&reg->cfg1);
	cfg |= dividend;
	writel(cfg, &reg->cfg1);

	/* set on bit*/
	setbits_le32(priv->addr[PLL_ON_CFG], BIT(0));

	/* wait for LOCKED */
	while (true) {
		if (readl(&reg->stat) & BIT(0))
			break;

		udelay(10);
	}

	while (true) {
		if (readl(priv->addr[PLL_RDY]) & BIT(priv->rdy_bit))
			break;

		udelay(10);
	}

	/* set reopen bit */
	setbits_le32(priv->addr[PLL_REOPEN], BIT(priv->reopen_bit));
#endif

	return -EINVAL;
}

static unsigned long
pll_clk_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct lua_pll_clk *pll = to_pll_clk(hw);
	struct pll_clock_priv *priv = pll->priv;
	ulong rate = 0UL;

#if defined(CONFIG_LUA_PLL_FIXED_FREQ)
	phys_addr_t base = priv->pllc_base;
	rate = pll_fixed_rate(base, priv->rdy_bit);
#else
	struct pll_reg *reg = priv->addr[PLL_REG];

	rate = pll_clac_rate(reg, parent_rate);
#endif

	pr_debug("%s: '%s' clk rate: %lu\n", __func__, clk_hw_get_name(hw), rate);
	return rate;
}

long __maybe_unused
pll_clk_round_rate(struct clk_hw *hw, unsigned long rate,
	unsigned long *parent_rate)
{
	/* FIXME: TODO */
	return 0;
}

const struct clk_ops pll_clk_ops = {
	.recalc_rate = pll_clk_recalc_rate,
	// .round_rate = pll_clk_round_rate,
	// .set_rate = pll_clk_set_rate,
};

struct clk_hw *clk_register_pll(struct device_node *np,
                    const char *name, const char *parent_name, u8 flags,
                       void *priv_data)
{
	struct lua_pll_clk *pll_clk;
	struct clk_hw *hw;
	struct clk_init_data init = {};
	int ret;

	pll_clk = kzalloc(sizeof(*pll_clk), GFP_KERNEL);
	if (!pll_clk)
		return ERR_PTR(-ENOMEM);

	pll_clk->priv = priv_data;

	init.name = name;
	init.ops = &pll_clk_ops;
	init.flags = flags;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.parent_hws = NULL;
	init.parent_data = NULL;
	if (parent_name)
		init.num_parents = 1;
	else
		init.num_parents = 0;

	pll_clk->hw.init = &init;

	/* register the clock */
	hw = &pll_clk->hw;
	ret = of_clk_hw_register(np, hw);
	if (ret) {
		kfree(pll_clk);
		hw = ERR_PTR(ret);
	}

	return hw;
}

static int pll_clock_bind(struct device_node *np)
{
	uint32_t reg_offset[ARRAY_SIZE(pll_reg_prop)];
	struct pll_clock_plat *plat = NULL;
	struct pll_clock_priv *priv = NULL;
	struct of_phandle_args args;
	struct device_node *subnode, *regmap;
	struct clk_hw *hw;
	struct resource res;
	ulong flags = 0;
	uint32_t val;
	void __iomem *base = NULL;
	const char *name;
	int i, count, ret;

	regmap = of_parse_phandle(np, "regmap", 0);
	if (IS_ERR(regmap)) {
		pr_err("Failed to get 'regmap' property\n");
		return -ENXIO;
	}
	if (!of_device_is_available(regmap))
		return -ENXIO;
	pr_debug("regmap node name: %s\n", regmap->full_name);
	base = of_iomap(regmap, 0);

	if (of_address_to_resource(regmap, 0, &res)) {
		pr_err("Failed to get 'regmap' property\n");
		return -ENXIO;
	}
	pr_debug("regmap physical base address: 0x%llx\n", res.start);

	count = of_get_child_count(np);
	plat = kzalloc(sizeof(*plat) + count * sizeof(*priv), GFP_KERNEL);
	if (!plat)
		return -ENOMEM;

	plat->pll_count = count;
	priv = plat->pll_priv;

	for_each_child_of_node(np, subnode) {
		name = of_node_full_name(subnode);

		for (i = 0; i < ARRAY_SIZE(pll_reg_prop); i++) {
			ret = of_property_read_u32_index(regmap, pll_reg_prop[i], 0,
			             &reg_offset[i]);
			if (ret) {
				pr_err("Failed to get '%s' value on '%s' node\n",
				                           pll_reg_prop[i], name);
				return ret;
			}
		}
		priv->pllc_base = res.start;

		for (i = 0; i < ARRAY_SIZE(pll_prop); i++) {
			ret = of_property_read_u32_index(subnode, pll_prop[i], 0, &val);
			if (ret) {
				pr_err("Failed to get '%s' value on '%s' node\n",
				                           pll_prop[i], name);
				return ret;
			}

			if (i < PLL_RDY)
				priv->addr[i] = base + val;
			else
				priv->addr[i] = base + reg_offset[i - PLL_RDY];

			switch (i) {
			case PLL_RDY:
				priv->rdy_bit = val;
				break;
			case PLL_FRC_EN:
				priv->frc_en_bit = val;
				break;
			case PLL_FRC_EN_SW:
				priv->frc_en_sw_bit = val;
				break;
			case PLL_REOPEN:
				priv->reopen_bit = val;
				break;
			}
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

		hw = clk_register_pll(subnode, name,
		        of_node_full_name(args.np),
		                    flags, priv);
		if (IS_ERR(hw)) {
			pr_warn("Failed to register '%s' clk\n", name);
			continue;
		}

		ret = of_clk_add_hw_provider(subnode, of_clk_hw_simple_get, hw);
		if (ret) {
			clk_hw_unregister_gate(hw);
		}
		priv++;
	}

	pr_debug("Register '%s' PLL clocks successfully\n", of_node_full_name(np));
	return 0;
}

static void __init of_pll_clk_setup(struct device_node *node)
{
	pll_clock_bind(node);
}
CLK_OF_DECLARE(axera_div_clk, "axera,lua-pll-clocks", of_pll_clk_setup);