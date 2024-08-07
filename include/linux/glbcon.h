/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Global SYSCTL driver for Laguna SoCs
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#ifndef __LAGUNA_GLBCON_H
#define __LAGUNA_GLBCON_H

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/types.h>

struct device_node;
struct device;
struct of_phandle_args;
struct subsysctl;
struct subsysctl_dev;

struct subsysctl_ops {
	int (*status)(struct subsysctl_dev *scdev, uint32_t shift, uint32_t width, uint32_t data);
	int (*write)(struct subsysctl_dev *scdev, uint32_t shift, uint32_t width, uint32_t data);
};

struct subsysctl_dev {
	const struct subsysctl_ops *ops;
	struct device *dev;
	struct device_node *of_node;
	struct module *owner;
	struct list_head list;
	struct list_head subsysctl_head;
	int of_syscon_n_cells;
	uint32_t nr_syscons;

	int (*of_xlate)(struct subsysctl_dev *subsys_ctl,
			const struct of_phandle_args *args, struct subsysctl *sc);
};

struct subsysctl_bulk {
	struct subsysctl *scs;
	unsigned int count;
};

#if IS_ENABLED(CONFIG_SUBSYS_CONTROLLER)

int subsysctl_assert(struct subsysctl *sc);
int subsysctl_status(struct subsysctl *sc);
int subsysctl_update(struct subsysctl *sc, uint32_t data);

int subsysctl_dev_register(struct subsysctl_dev *scdev);
void subsysctl_dev_unregister(struct subsysctl_dev *scdev);
int devm_subsysctl_dev_register(struct device *dev, struct subsysctl_dev *scdev);
struct subsysctl * __of_subsysctl_get(struct device_node *node, const char *id, int index);
struct subsysctl *__devm_subsysctl_get(struct device *dev, const char *id, int index);
#else
static inline int subsysctl_assert(struct subsysctl *sc)
{
	return 0;
}

static inline int subsysctl_status(struct subsysctl *sc)
{
	return 0;
}

static inline int subsysctl_update(struct subsysctl *sc, uint32_t data)
{
	return 0;
}

static inline int subsysctl_dev_register(struct subsysctl_dev *scdev)
{
	return 0;
}

static inline void subsysctl_dev_unregister(struct subsysctl_dev *scdev)
{
}

static inline int devm_subsysctl_dev_register(struct device *dev, struct subsysctl_dev *scdev)
{
	return 0;
}

static inline struct subsysctl * __of_subsysctl_get(struct device_node *node, const char *id, int index)
{
	return ERR_PTR(-ENOTSUPP);
}

static inline struct subsysctl *__devm_subsysctl_get(struct device *dev, const char *id, int index)
{
	return ERR_PTR(-ENOTSUPP);
}
#endif

static inline
struct subsysctl *of_subsysctl_get_by_index(struct device_node *node, int index)
{
	return __of_subsysctl_get(node, NULL, index);
}

static inline
struct subsysctl *of_subsysctl_get(struct device_node *node, const char *id)
{
	return __of_subsysctl_get(node, id, 0);
}

static inline struct subsysctl *
__must_check devm_subsysctl_get_by_index(struct device *dev, int index)
{
	return __devm_subsysctl_get(dev, NULL, index);
}

static inline struct subsysctl *
__must_check devm_subsysctl_get(struct device *dev,
					      const char *id)
{
	return __devm_subsysctl_get(dev, id, 0);
}

#endif