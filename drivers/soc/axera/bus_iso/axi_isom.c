// SPDX-License-Identifier: GPL-2.0
/*
 * AXI Isolation Master driver for Laguna SoCs
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com.com>
 */

#define pr_fmt(fmt) "bus_isom: " fmt

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/dev_printk.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

struct isom_rch {
	uint32_t raddr_h;
	uint32_t raddr_l;
	uint32_t rid;
};

struct isom_wch {
	uint32_t waddr_h;
	uint32_t waddr_l;
	uint32_t wid;
};

struct isom_reg {
	uint32_t sw_rst;
	char rsvd0[8];
	uint32_t cfg_update;
	char rsvd1[8];
	uint32_t fault_inject;
	char rsvd2[8];
	uint32_t flush;
	uint32_t flush_set;
	uint32_t flush_clr;
	uint32_t glb_cfg;
	char rsvd4[8];
	uint32_t int_cfg;
	uint32_t int_cfg_set;
	uint32_t int_cfg_clr;
	uint32_t fault_cfg;
	char rsvd6[8];
	uint32_t twos;
	uint32_t tros;
	uint32_t bypass_sta;
};

struct axi_isom_priv {
	struct device *dev;
	void __iomem * reg;
	struct reset_control_bulk_data resets[2];
	struct clk_bulk_data clks[2];
	struct regmap *fabirq;
	uint32_t offset;
	uint32_t timeout;
	bool bypass;
	int irq;
};

/* list of resets */
static const char * const isom_reset_l[] = {
	"apb_prst", "isom_rst",
};

/* list of clocks required */
static const char * const isom_clk_l[] = {
	"apb_pclk", "isom_clk",
};

uint32_t threshold = 8;

#define ISOM_GLBCFG_TIMEOUT_OFFSET  16
#define ISOM_GLBCFG_TIMEOUT_MASK    (BIT(16) - 1)
#define ISOM_GLBCFG_FAULT_EN        BIT(3)
#define ISOM_GLBCFG_RCH_BYPASS      BIT(2)
#define ISOM_GLBCFG_WCH_BYPASS      BIT(1)
#define ISOM_GLBCFG_AUTO_GATE_EN    BIT(0)

#define ISOM_CFG_UPDATE             BIT(0)

#define ISOM_INTCFG_CLR             BIT(0)
#define ISOM_INTCFG_MASK_OFFSET     8
#define ISOM_INTCFG_MASK            (BIT(7) - 1)

#define ISOM_FLUSH_EXIT_SW          BIT(0)
#define ISOM_FLUSH_ENTER_SW         BIT(1)
#define ISOM_FLUSH_STATUS           BIT(2)
#define ISOM_FLUSH_RW_MASK          (BIT(2) - 1)

#define ISOM_INT_CFG_CLR            BIT(0)

#define ISOM_RCH_OFFSET             0x3000
#define ISOM_WCH_OFFSET             0x2000

static inline int axi_isom_status_check(struct axi_isom_priv *priv)
{
	struct isom_reg *base = priv->reg;
	struct isom_rch *rch = priv->reg + ISOM_RCH_OFFSET;
	struct isom_wch *wch = priv->reg + ISOM_WCH_OFFSET;

	dev_info_ratelimited(priv->dev, "Regs dump:\n");
	dev_info_ratelimited(priv->dev, "    sw_rst:       0x%08x\n", readl(&base->sw_rst));
	dev_info_ratelimited(priv->dev, "    cfg_update:   0x%08x\n", readl(&base->cfg_update));
	dev_info_ratelimited(priv->dev, "    fault_inject: 0x%08x\n", readl(&base->fault_inject));
	dev_info_ratelimited(priv->dev, "    flush:        0x%08x\n", readl(&base->flush));
	dev_info_ratelimited(priv->dev, "    glb_cfg:      0x%08x\n", readl(&base->glb_cfg));
	dev_info_ratelimited(priv->dev, "    int_cfg:      0x%08x\n", readl(&base->int_cfg));
	dev_info_ratelimited(priv->dev, "    fault_cfg:    0x%08x\n", readl(&base->fault_cfg));
	dev_info_ratelimited(priv->dev, "    twos:         0x%08x\n", readl(&base->twos));
	dev_info_ratelimited(priv->dev, "    tros:         0x%08x\n", readl(&base->tros));
	dev_info_ratelimited(priv->dev, "    bypass_sta:   0x%08x\n", readl(&base->bypass_sta));

	dev_info_ratelimited(priv->dev, "wch info: 0x%x 0x%x 0x%x\n",
	                  readl(&wch->waddr_h), readl(&wch->waddr_l), readl(&wch->wid));

	dev_info_ratelimited(priv->dev, "rch info: 0x%x 0x%x 0x%x\n",
	                  readl(&rch->raddr_h), readl(&rch->raddr_l), readl(&rch->rid));

	return 0;
}

static inline int axi_iso_irq_clr_and_exit_sw(struct axi_isom_priv *priv)
{
	struct isom_reg *base = priv->reg;
	uint32_t val;

	dev_dbg_ratelimited(priv->dev, "sw irq_clr and exit flush mode\n");
	val = readl(&base->flush);
	if (val & ISOM_FLUSH_ENTER_SW) {
		udelay(1);
		writel(ISOM_FLUSH_ENTER_SW, &base->flush_clr);
		udelay(1);
	}

	if (val & ISOM_FLUSH_STATUS) {
		writel(ISOM_INT_CFG_CLR, &base->int_cfg_set);
		udelay(1);
		writel(ISOM_INT_CFG_CLR, &base->int_cfg_clr);
		writel(ISOM_FLUSH_EXIT_SW, &base->flush_set);
		udelay(1);
		writel(ISOM_FLUSH_EXIT_SW, &base->flush_clr);
	}
	return 0;
}

static void axi_isom_mask_fabirq_for_rsvd_addr(struct axi_isom_priv *priv)
{
	uint32_t fabirq_isom;
	uint32_t val = 0;

	/* clear axi_isom irq mask */
	fabirq_isom = BIT(0) | BIT(1) | BIT(2);
	val &= ~fabirq_isom;

	val |= (BIT(3) | BIT(4) | BIT(5) | BIT(6));
	val <<= 14;
	regmap_write(priv->fabirq, priv->offset, val);
}

static irqreturn_t axi_isom_irq_handle(int irq, void *data)
{
	static uint32_t hit_isom = 0, hit = 0;
	struct axi_isom_priv *priv = data;
	struct isom_reg *base = priv->reg;
	int ret;
	uint32_t fabirq;
	uint32_t mask;

	ret = regmap_read(priv->fabirq, priv->offset, &fabirq);
	if (ret < 0) {
		dev_err_ratelimited(priv->dev, "failed to read 'fabirq' register\n");
		return IRQ_HANDLED;
	}

	dev_dbg_ratelimited(priv->dev, "fab_irq register: 0x%08x\n", fabirq);
	if ((fabirq & BIT(3)) || (fabirq & BIT(4)) ||
	       (fabirq & BIT(5)) || (fabirq & BIT(6))) {
		dev_info_ratelimited(priv->dev, "fab irq triggered by accessing reserved address\n");
		hit++;

		if (hit > threshold) {
			dev_notice_ratelimited(priv->dev, "Mask fabirq source for accessing reserved address\n");
			axi_isom_mask_fabirq_for_rsvd_addr(priv);
			hit = 0;
		}

		return IRQ_HANDLED;
	}

	axi_isom_status_check(priv);
	axi_iso_irq_clr_and_exit_sw(priv);

	hit_isom++;
	if (hit_isom > threshold) {
		/* mask isom irq source */
		dev_notice_ratelimited(priv->dev, "Mask bus isom irq source\n");
		mask = ISOM_INTCFG_MASK << ISOM_INTCFG_MASK_OFFSET;
		writel(mask, &base->int_cfg);
	}

	return IRQ_HANDLED;
}

static inline int axi_isom_init(struct axi_isom_priv *priv)
{
	struct isom_reg *base = priv->reg;
	uint32_t val;

	if (priv->bypass) {
		dev_info(priv->dev, "bypass bus isom\n");
		return 0;
	}

	val = readl(&base->glb_cfg);
	val |= (ISOM_GLBCFG_AUTO_GATE_EN | ISOM_GLBCFG_FAULT_EN);
	val &= ~(ISOM_GLBCFG_RCH_BYPASS | ISOM_GLBCFG_WCH_BYPASS);
	val &= ~(ISOM_GLBCFG_TIMEOUT_MASK << ISOM_GLBCFG_TIMEOUT_OFFSET);
	val |= (priv->timeout << ISOM_GLBCFG_TIMEOUT_OFFSET);
	writel(val, &base->glb_cfg);

	val = readl(&base->cfg_update);
	val |= ISOM_CFG_UPDATE;
	writel(val, &base->cfg_update);
	udelay(1);
	val &= ~ISOM_CFG_UPDATE;
	writel(val, &base->cfg_update);

	val = readl(&base->int_cfg);
	val &= ~(ISOM_INTCFG_MASK << ISOM_INTCFG_MASK_OFFSET);
	writel(val, &base->int_cfg);

	dev_info(priv->dev, "%s: done\n", __func__);
	return 0;
}

static int axi_isom_probe(struct platform_device *pdev)
{
	struct axi_isom_priv *priv;
	struct device *dev = &pdev->dev;
	int ret, i;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (IS_ERR_OR_NULL(priv)) {
		dev_err(dev, "out of memory\n");
		return -ENOMEM;
	}
	priv->dev = dev;

	priv->reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR_OR_NULL(priv->reg)) {
		dev_err(dev, "faile to get addr\n");
		return PTR_ERR(priv->reg);
	}
	priv->irq = platform_get_irq(pdev, 0);

	if (of_property_read_u32(dev->of_node, "axera,isom-timeout",
	       &priv->timeout)) {
		dev_err(dev, "unable to read 'axera,isom-timeout'");
		return -EINVAL;
	}

	if (of_property_read_bool(dev->of_node, "axera,isom-bypass"))
		priv->bypass = true;

	priv->fabirq = syscon_regmap_lookup_by_phandle(dev->of_node, "axera,isom-fabirq");
	if (IS_ERR_OR_NULL(priv->fabirq)) {
		dev_err(dev, "syscon_node_to_regmap failed\n");
		return -EINVAL;
	}

	if (of_property_read_u32_index(dev->of_node, "axera,isom-fabirq",
	        1, &priv->offset)) {
		dev_err(dev, "unable to read 'axera,isom-fabirq'");
		return -EINVAL;
	}

	for (i = 0; i < 2; i++)
		priv->resets[i].id = isom_reset_l[i];

	ret = devm_reset_control_bulk_get_exclusive(dev, 2, priv->resets);
	if (ret) {
		dev_err(dev, "failed to get resets\n");
		return ret;
	}

	reset_control_bulk_assert(2, priv->resets);
	udelay(1);
	reset_control_bulk_deassert(2, priv->resets);

	for (i = 0; i < 2; i++)
		priv->clks[i].id = isom_clk_l[i];

	ret = devm_clk_bulk_get_optional(dev, 2, priv->clks);
	if (ret) {
		dev_err(dev, "failed to get clocks\n");
		return ret;
	}

	ret = clk_bulk_prepare_enable(2, priv->clks);
	if (ret) {
		dev_err(dev, "faile to enable clocks\n");
		return ret;
	}

	axi_isom_init(priv);

	ret = devm_request_threaded_irq(dev, priv->irq, NULL,
	               axi_isom_irq_handle, IRQF_ONESHOT,
	               dev_name(dev), priv);
	if (ret) {
		dev_err(dev, "Unable to request irq: %d for probe\n", priv->irq);
		goto failed;
	};

	dev_info(dev, "probe done\n");
	return 0;

failed:
	clk_bulk_disable_unprepare(2, priv->clks);
	return ret;
}

static const struct of_device_id axi_isom_ids[] = {
	{ .compatible = "axera,m57h-bus-isom" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, axi_iso_mr_ids);

static struct platform_driver axi_isom_driver = {
	.probe = axi_isom_probe,
	.driver = {
		.name = "bus_isom",
		.of_match_table = axi_isom_ids,
	},
};
module_platform_driver(axi_isom_driver);

MODULE_AUTHOR("Charleye <wangkart@aliyun.com.com>");
MODULE_DESCRIPTION("Bus Master Isolation Driver");
MODULE_LICENSE("GPL v2");