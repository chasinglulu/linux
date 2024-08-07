// SPDX-License-Identifier: GPL-2.0
/*
 * Subsystem Global Register Controller driver for Laguna SoCs
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "GLBCON: " fmt

#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/glbcon.h>

static DEFINE_MUTEX(subsysctl_list_mutex);
static LIST_HEAD(subsysctl_list);

static DEFINE_MUTEX(subsysctl_lookup_mutex);
static LIST_HEAD(subsysctl_lookup_list);

struct subsysctl {
	struct subsysctl_dev *scdev;
	struct list_head list;
	unsigned int id;
	struct kref refcnt;
	uint16_t shift;
	uint16_t width;
	uint32_t data;
};

static int subsysctl_of_xlate_default(struct subsysctl_dev *scdev,
				  const struct of_phandle_args *args, struct subsysctl *sc)
{
	dev_dbg(scdev->dev, "%s(subsysctl_dev = %p)\n", __func__, scdev);

	if (args->args_count < 2 || args->args_count > 3) {
		dev_err(scdev->dev, "Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	sc->shift = args->args[0];
	if (args->args_count == 3) {
		sc->width = args->args[1];
		sc->data = args->args[2];
	} else {
		sc->data = args->args[1];
	}

	return 0;
}

int subsysctl_dev_register(struct subsysctl_dev *scdev)
{
	if (!scdev->of_xlate) {
		scdev->of_syscon_n_cells = 2;
		scdev->of_xlate = subsysctl_of_xlate_default;
	}

	INIT_LIST_HEAD(&scdev->subsysctl_head);

	mutex_lock(&subsysctl_list_mutex);
	list_add(&scdev->list, &subsysctl_list);
	mutex_unlock(&subsysctl_list_mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(subsysctl_dev_register);

void subsysctl_dev_unregister(struct subsysctl_dev *scdev)
{
	mutex_lock(&subsysctl_list_mutex);
	list_del(&scdev->list);
	mutex_unlock(&subsysctl_list_mutex);
}
EXPORT_SYMBOL_GPL(subsysctl_dev_unregister);

static void devm_subsysctl_dev_release(struct device *dev, void *res)
{
	subsysctl_dev_unregister(*(struct subsysctl_dev **)res);
}

int devm_subsysctl_dev_register(struct device *dev, struct subsysctl_dev *scdev)
{
	struct subsysctl_dev **scdevp;
	int ret;

	scdevp = devres_alloc(devm_subsysctl_dev_release, sizeof(*scdevp),
			      GFP_KERNEL);
	if (!scdevp)
		return -ENOMEM;

	ret = subsysctl_dev_register(scdev);
	if (ret) {
		devres_free(scdevp);
		return ret;
	}

	*scdevp = scdev;
	devres_add(dev, scdevp);

	return ret;
}
EXPORT_SYMBOL_GPL(devm_subsysctl_dev_register);

static struct subsysctl *
__subsysctl_get_internal(struct subsysctl_dev *scdev, unsigned int index)
{
	struct subsysctl *sc;

	lockdep_assert_held(&subsysctl_list_mutex);

	// list_for_each_entry(sc, &scdev->subsysctl_head, list) {
	// 	if (sc->id == index) {
	// 		kref_get(&sc->refcnt);
	// 		return sc;
	// 	}
	// }

	sc = kzalloc(sizeof(*sc), GFP_KERNEL);
	if (!sc)
		return ERR_PTR(-ENOMEM);

	if (!try_module_get(scdev->owner)) {
		kfree(sc);
		return ERR_PTR(-ENODEV);
	}

	sc->scdev = scdev;
	list_add(&sc->list, &scdev->subsysctl_head);
	sc->id = index;
	kref_init(&sc->refcnt);

	return sc;
}

struct subsysctl *
__of_subsysctl_get(struct device_node *node, const char *id, int index)
{
	struct subsysctl *sc;
	struct subsysctl_dev *r, *scdev;
	struct of_phandle_args args;
	int ret;

	if (!node)
		return ERR_PTR(-EINVAL);

	if (id) {
		index = of_property_match_string(node,
						 "syscon-names", id);
		if (index == -EILSEQ)
			return ERR_PTR(index);
		if (index < 0)
			return ERR_PTR(-ENOENT);
	}

	ret = of_parse_phandle_with_args(node, "syscons", "#syscon-cells",
					index, &args);
	if (ret)
		return ERR_PTR(ret);

	mutex_lock(&subsysctl_list_mutex);
	scdev = NULL;
	list_for_each_entry(r, &subsysctl_list, list) {
		if (args.np == r->of_node) {
			scdev = r;
			break;
		}
	}

	if (!scdev) {
		sc = ERR_PTR(-EPROBE_DEFER);
		goto out;
	}

	if (WARN_ON(args.args_count != scdev->of_syscon_n_cells)) {
		sc = ERR_PTR(-EINVAL);
		goto out;
	}

	sc = __subsysctl_get_internal(scdev, index);
	if (IS_ERR(sc))
		goto out;

	ret = scdev->of_xlate(scdev, &args, sc);
	if (ret < 0)
		sc = ERR_PTR(ret);

out:
	mutex_unlock(&subsysctl_list_mutex);
	of_node_put(args.np);

	return sc;
}
EXPORT_SYMBOL_GPL(__of_subsysctl_get);

static void __subsysctl_release(struct kref *kref)
{
	struct subsysctl *sc = container_of(kref, struct subsysctl, refcnt);

	lockdep_assert_held(&subsysctl_list_mutex);

	module_put(sc->scdev->owner);

	list_del(&sc->list);
	kfree(sc);
}

static void __subsysctl_put_internal(struct subsysctl *sc)
{
	lockdep_assert_held(&subsysctl_list_mutex);

	if (IS_ERR_OR_NULL(sc))
		return;

	kref_put(&sc->refcnt, __subsysctl_release);
}

void subsysctl_put(struct subsysctl *sc)
{
	if (IS_ERR_OR_NULL(sc))
		return;

	mutex_lock(&subsysctl_list_mutex);
	__subsysctl_put_internal(sc);
	mutex_unlock(&subsysctl_list_mutex);
}
EXPORT_SYMBOL_GPL(subsysctl_put);

struct subsysctl *__subsysctl_get(struct device *dev, const char *id, int index)
{
	if (dev->of_node) {
		return __of_subsysctl_get(dev->of_node, id, index);
	}

	return ERR_PTR(-EINVAL);
}
EXPORT_SYMBOL_GPL(__subsysctl_get);

static void devm_subsysctl_release(struct device *dev, void *res)
{
	subsysctl_put(*(struct subsysctl **)res);
}

struct subsysctl *
__devm_subsysctl_get(struct device *dev, const char *id, int index)
{
	struct subsysctl **ptr, *sc;

	ptr = devres_alloc(devm_subsysctl_release, sizeof(*ptr),
			   GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	sc = __subsysctl_get(dev, id, index);
	if (IS_ERR_OR_NULL(sc)) {
		devres_free(ptr);
		return sc;
	}

	*ptr = sc;
	devres_add(dev, ptr);

	return sc;
}
EXPORT_SYMBOL_GPL(__devm_subsysctl_get);

/**
 * subsysctl_assert - asserts the syscon signal
 * @sc: subsys controller
 *
 * Calling this on an subsys controller guarantees that the syscon signal
 * will be asserted.
 *
 */
int subsysctl_assert(struct subsysctl *sc)
{
	if (!sc)
		return 0;

	if (WARN_ON(IS_ERR(sc)))
		return -EINVAL;

	/*
	 * If the reset controller does not implement .write(), there
	 * is no way to guarantee that the syscon signal is asserted after
	 * this call.
	 */
	if (!sc->scdev->ops->write)
		return -ENOTSUPP;

	return sc->scdev->ops->write(sc->scdev, sc->shift, sc->width, sc->data);
}
EXPORT_SYMBOL_GPL(subsysctl_assert);

int subsysctl_update(struct subsysctl *sc, uint32_t data)
{
	if (!sc)
		return 0;

	if (WARN_ON(IS_ERR(sc)))
		return -EINVAL;

	sc->data = data;

	return 0;
}
EXPORT_SYMBOL_GPL(subsysctl_update);

/**
 * subsysctl_status - returns a negative errno if not supported, a
 * positive value if the syscon signal is asserted, or zero if the syscon
 * signal is not asserted.
 * @sc: subsys controller
 */
int subsysctl_status(struct subsysctl *sc)
{
	if (!sc)
		return 0;

	if (WARN_ON(IS_ERR(sc)))
		return -EINVAL;

	if (sc->scdev->ops->status)
		return sc->scdev->ops->status(sc->scdev, sc->shift, sc->width, sc->data);

	return -ENOTSUPP;
}
EXPORT_SYMBOL_GPL(subsysctl_status);
