/**
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Diagnosis Framework Core
 * Responsible for collecting errors reported by various drivers.
 *
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 *
 */
#define pr_fmt(fmt) "DFC: " fmt

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/diag.h>
#include <linux/kthread.h>
#include <linux/sched/clock.h>
#include <linux/netlink.h>

#include "diag_core.h"

static struct diag_core *dc_ctrl = NULL;

static ssize_t
selftest_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{

	pr_info("Not yet implemented\n");
	return count;
}
DEVICE_ATTR_WO(selftest);

static ssize_t
count_show(struct device *dev, struct device_attribute *attr, char *page)
{
	int ret;
	struct diag_core *dc = dev_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&dc->dm_lock, flags);
	ret = scnprintf(page, PAGE_SIZE - 1, "%u\n", dc->module_cnt);
	spin_unlock_irqrestore(&dc->dm_lock, flags);

	return ret;
}
static DEVICE_ATTR_RO(count);

static struct attribute *diag_core_attrs[] = {
	&dev_attr_count.attr,
	&dev_attr_selftest.attr,
	NULL,
};
ATTRIBUTE_GROUPS(diag_core);

static struct diag_module_mgt *diagnosis_lookup_module(uint16_t mid)
{
	struct diag_module_mgt *dmm = NULL, *tmp;
	unsigned long flags;

	if (!dc_ctrl)
		return NULL;

	spin_lock_irqsave(&dc_ctrl->dm_lock, flags);
	if (list_empty(&dc_ctrl->dm_head)) {
		spin_unlock_irqrestore(&dc_ctrl->dm_lock, flags);
		return NULL;
	}

	list_for_each_entry_safe(dmm, tmp, &dc_ctrl->dm_head, node) {
		if(dmm->mid == mid) {
			spin_unlock_irqrestore(&dc_ctrl->dm_lock, flags);
			return dmm;
		}
	}
	spin_unlock_irqrestore(&dc_ctrl->dm_lock, flags);

	return NULL;
}

struct diag_error_mgt *diagnosis_lookup_error(uint16_t mid, uint16_t eid)
{
	struct diag_module_mgt *dmm;
	struct diag_error_mgt *dem = NULL, *tmp;

	if (!dc_ctrl)
		return NULL;

	dmm = diagnosis_lookup_module(mid);
	if (!dmm)
		return NULL;

	list_for_each_entry_safe(dem, tmp, &dmm->err_head, err_node) {
		if (dem->handle.eid == eid)
			return dem;
	}

	return NULL;
}

static void diag_dev_release(struct device *dev)
{
}

static int diagnosis_sanity_check(const struct diag_register_info * ri)
{
	struct diag_module_mgt *mod_mgt;
	const struct diag_error_handle *ri_err;
	uint16_t count = 0;

	if (ri == NULL)
		return -EINVAL;

	mod_mgt = diagnosis_lookup_module(ri->mid);
	if (mod_mgt) {
		pr_err("The module %d exist\n", ri->mid);
		return -EEXIST;
	}

	for (ri_err = &ri->handle[0]; count < ri->error_cnt &&
						ri_err->eid != 0xffff; ri_err++) {
		if (ri_err->name)
			count++;
	}

	/* actual error count fewer */
	if (count < ri->error_cnt) {
		pr_err("Actual vaild error count fewer\n");
		return -EINVAL;
	}

	return 0;
}

int diagnosis_register(const struct diag_register_info *reg_info)
{
	int ret;
	struct diag_core *dc = dc_ctrl;
	struct device *dev;
	struct diag_module_mgt *dmm;
	struct diag_error_mgt *de_mgt, *tmp;
	const struct diag_error_handle *ri_err;
	int count = 0;
	struct dentry *dir = NULL;

	if (!dc) {
		pr_err("The diagnosis core is not yet ready!\n");
		return -EBUSY;
	}
	dev = &dc->pdev->dev;

	ret = diagnosis_sanity_check(reg_info);
	if (ret < 0) {
		dev_err(dev, "The register information is not valid!\n");
		return -EINVAL;
	}

	dmm = (struct diag_module_mgt *)devm_kzalloc(dev, sizeof(*dmm), GFP_KERNEL);
	if (!dmm) {
		dev_err(dev, "Out of memory\n");
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&dmm->err_head);
	dmm->mid = reg_info->mid;
	dmm->cycle = reg_info->cycle;
	dmm->data = reg_info->data;

	dmm->mod_dev.parent = dev;
	dmm->mod_dev.release = diag_dev_release;
	dev_set_name(&dmm->mod_dev, "module%d", dmm->mid);
	diag_dev_set_attrs(&dmm->mod_dev);
	if (!dmm->name)
		dmm->name = kstrdup_const(dev_name(&dmm->mod_dev), GFP_KERNEL);

	ret = device_register(&dmm->mod_dev);
	if (ret) {
		dev_err(dev, "Failed to register module device, ret= %d\n", ret);
		put_device(&dmm->mod_dev);
		goto fail_reg;
	}

	dmm->dir = diag_fi_create_dir(&dmm->mod_dev, dc->diag_fi_dir);
	if (IS_ERR(dmm->dir)) {
		dev_err(dev, "Failed to create debugfs entry\n");
		ret = PTR_ERR(dmm->dir);
		dmm->dir = NULL;
		goto fail_fi;
	}

	for (ri_err = &reg_info->handle[0]; count < reg_info->error_cnt &&
								ri_err->eid != 0xffff; ri_err++) {
		if (!ri_err->name)
			continue;
		else
			count++;

		de_mgt = diagnosis_lookup_error(dmm->mid, ri_err->eid);
		if (de_mgt) {
			dev_err(dev, "Error %d exist\n", ri_err->eid);
			goto fail_err;
		}
		de_mgt = NULL;

		de_mgt = (struct diag_error_mgt *)devm_kzalloc(dev,
												sizeof(*de_mgt), GFP_KERNEL);
		if (!de_mgt) {
			dev_err(dev, "Out of memory");
			ret = -ENOMEM;
			goto fail_err;
		}
		memcpy(&de_mgt->handle, ri_err, sizeof(*ri_err));
		INIT_LIST_HEAD(&de_mgt->err_node);

		de_mgt->err_dev.parent = &dmm->mod_dev;
		de_mgt->err_dev.release = diag_dev_release;
		dev_set_name(&de_mgt->err_dev, "error%d", de_mgt->handle.eid);
		diag_dev_set_attrs(&de_mgt->err_dev);

		ret = device_register(&de_mgt->err_dev);
		if (ret) {
			dev_err(dev, "Failed to register error device, ret= %d\n", ret);
			put_device(&de_mgt->err_dev);
			devm_kfree(dev, de_mgt);
			goto fail_err;
		}

		dir = diag_fi_create_dir(&de_mgt->err_dev, dmm->dir);
		if (IS_ERR(dir)) {
			dev_err(dev, "Failed to create error debugfs entry\n");
			ret = PTR_ERR(dir);
			goto fail_err;
		} else if (dir != NULL) {
			de_mgt->handle.attr.injectable = true;
		}

		list_add_tail(&de_mgt->err_node, &dmm->err_head);
		dmm->error_cnt++;
	}

	spin_lock(&dc->dm_lock);
	dc->module_cnt++;
	list_add_tail(&dmm->node, &dc->dm_head);
	spin_unlock(&dc->dm_lock);

	return 0;

fail_err:
	list_for_each_entry_safe(de_mgt, tmp, &dmm->err_head, err_node) {
		list_del(&de_mgt->err_node);
		diag_fi_remove_dir(&de_mgt->err_dev);
		device_unregister(&de_mgt->err_dev);
		devm_kfree(dev, de_mgt);
	}

fail_fi:
	device_unregister(&dmm->mod_dev);

fail_reg:
	devm_kfree(dev, dmm);

	return ret;
}
EXPORT_SYMBOL(diagnosis_register);

static bool diag_error_node_insert(struct rb_root_cached *root,
									struct diag_core_error *dce)
{
	struct rb_node **new = &(root->rb_root.rb_node), *parent = NULL;
	bool leftmost = true;

	/* Figure out where to put new node */
	while (*new) {
		struct diag_core_error *this = container_of(*new,
									struct diag_core_error, rb_node);

		uint64_t this_key, curr_key;
		this_key = this->err.time | ((uint64_t)this->prio << 60);
		curr_key = dce->err.time | ((uint64_t)dce->prio << 60);

		parent = *new;
		if (this_key > curr_key)
			new = &((*new)->rb_left);
		else if (this_key < curr_key) {
			new = &((*new)->rb_right);
			leftmost = false;
		} else {
			pr_err("equal key value\n");
			return false;
		}
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&dce->rb_node, parent, new);
	rb_insert_color_cached(&dce->rb_node, root, leftmost);

	return true;
}

static int diagnosis_available_check(struct diag_error_mgt *err_mgt)
{
	struct diag_error_handle *handle = &err_mgt->handle;
	struct diag_error_attr *attr = &handle->attr;
	uint64_t delta = ktime_get_ns() - err_mgt->last_time;
	int ret = -EINVAL;

	if (!attr->enable)
		return ret;

	if (delta < (uint64_t)handle->min_snd_ms * 1000 * 1000)
		return ret;

	if (delta > (uint64_t)handle->max_snd_ms * 1000 * 1000)
		return 0;

	if (attr->repeat) {
		return 0;
	} else {
		return ret;
	}

	return ret;
}

int diagnosis_error_send(struct diag_error *error)
{
	struct diag_core *dc = dc_ctrl;
	struct device *dev;
	struct diag_error_mgt *err_mgt;
	struct diag_module_mgt *mod_mgt;
	struct diag_core_error *dce;
	struct diag_rbtree_ctx *rbtree;
	unsigned long flags;
	int ret;

	if(!dc) {
		pr_err("The diagnosis core is not yet ready!\n");
		return -EBUSY;
	}
	dev = &dc->pdev->dev;

	err_mgt = diagnosis_lookup_error(error->mid, error->eid);
	if(!err_mgt) {
		dev_err(dev, "The error %d was not registered in advance.\n", error->eid);
		return -EINVAL;
	}

	ret = diagnosis_available_check(err_mgt);
	if (ret)
		return ret;

	dce = (struct diag_core_error *)devm_kzalloc(dev, sizeof(*dce), GFP_ATOMIC);
	if(!dce) {
		dev_err(dev, "Failed to allocate error node memory.\n");
		return -ENOMEM;
	}
	memcpy(&dce->err, error, sizeof(*error));
	rbtree = dc->rbtree;
	dce->err.time = ktime_get_ns();
	dce->prio = err_mgt->handle.attr.prio;

	spin_lock_irqsave(&rbtree->rb_lock, flags);
	ret = diag_error_node_insert(&rbtree->root, dce);
	spin_unlock_irqrestore(&rbtree->rb_lock, flags);
	if (!ret) {
		dev_err(dev, "Failed to insert error node.\n");
		ret = -EAGAIN;
		goto failed;
	}

	mod_mgt = diagnosis_lookup_module(error->mid);
	if(err_mgt->handle.err_handle_cb)
		err_mgt->handle.err_handle_cb(mod_mgt->data);

	err_mgt->number++;
	atomic_inc(&err_mgt->pending);

	wake_up_interruptible(&dc->wq_head);
	return 0;

failed:
	devm_kfree(dev, dce);
	return ret;
}
EXPORT_SYMBOL(diagnosis_error_send);

int diagnosis_deregister(uint16_t mid)
{
	struct diag_core *dc = dc_ctrl;
	struct device *dev;
	struct diag_module_mgt *dcm, *tmp;
	struct diag_error_mgt *de_mgt, *tmp1;

	if (!dc) {
		pr_err("The diagnosis core is not yet ready!\n");
		return -EBUSY;
	}
	dev = &dc->pdev->dev;

	spin_lock(&dc->dm_lock);
	if (list_empty(&dc->dm_head)) {
		spin_unlock(&dc->dm_lock);
		return 0;
	}

	list_for_each_entry_safe(dcm, tmp, &dc->dm_head, node) {
		if (dcm->mid == mid) {
			list_del(&dcm->node);
			dc->module_cnt--;
			break;
		}
	}
	spin_unlock(&dc->dm_lock);

	if (list_entry_is_head(dcm, &dc->dm_head, node))
		return 0;

	list_for_each_entry_safe(de_mgt, tmp1, &dcm->err_head, err_node) {
		list_del(&de_mgt->err_node);
		diag_fi_remove_dir(&de_mgt->err_dev);
		device_unregister(&de_mgt->err_dev);
		devm_kfree(dev, de_mgt);
	}

	diag_fi_remove_dir(&dcm->mod_dev);
	device_unregister(&dcm->mod_dev);
	kfree_const(dcm->name);
	devm_kfree(dev, dcm);

	return 0;
}
EXPORT_SYMBOL(diagnosis_deregister);

static int diag_report_up_thread(void *data)
{
	struct diag_core *dc = (struct diag_core *)data;
	struct diag_rbtree_ctx *rbtree = dc->rbtree;
	struct diag_core_error *dce;
	struct device *dev = &dc->pdev->dev;
	struct diag_error_mgt *err_mgt;
	int ret;

	do {
		if (signal_pending(current))
			flush_signals(current);

		wait_event_interruptible(dc->wq_head,
				!RB_EMPTY_ROOT(&rbtree->root.rb_root) ||
				kthread_should_stop());

		spin_lock(&rbtree->rb_lock);
		dce = rb_entry(rb_first_cached(&rbtree->root), struct diag_core_error, rb_node);
		rb_erase_cached(&dce->rb_node, &rbtree->root);
		spin_unlock(&rbtree->rb_lock);

		err_mgt = diagnosis_lookup_error(dce->err.mid, dce->err.eid);
		atomic_dec(&err_mgt->pending);
		err_mgt->last_time = dce->err.time;

		ret = diag_netlink_send_msg(dc->sk, &dce->err);
		if (ret)
			dev_warn(dev, "Failed to send message into netlink socket, ret = %d\n", ret);

		devm_kfree(dev, dce);

	}while(!kthread_should_stop());

	return 0;
}

static int diag_cycle_check_thread(void *data)
{
	struct diag_core *dc = data;
	struct diag_module_mgt *dcm;
	int retval;

	do {
		set_current_state(TASK_UNINTERRUPTIBLE);

		spin_lock(&dc->dm_lock);
		list_for_each_entry(dcm, &dc->dm_head, node) {
			if (dcm->cycle) {
				spin_unlock(&dc->dm_lock);
				retval = dcm->cycle(dcm->data);
				if (retval)
					pr_err("The module %d (%s) callback failed\n", dcm->mid, dcm->name);

				spin_lock(&dc->dm_lock);
			}
		}
		spin_unlock(&dc->dm_lock);

		schedule_timeout(msecs_to_jiffies(CYCLE_CHK_PERIOD));
	}while(!kthread_should_stop());

	return 0;
}

static int diag_rbtree_init(struct diag_core *dc)
{
	struct device *dev = &dc->pdev->dev;
	struct diag_rbtree_ctx *rbtree_ctx;

	dc->rbtree = devm_kzalloc(dev, sizeof(*rbtree_ctx), GFP_KERNEL);
	if (!dc->rbtree)
		return -ENOMEM;

	rbtree_ctx = dc->rbtree;
	rbtree_ctx->root = RB_ROOT_CACHED;
	spin_lock_init(&rbtree_ctx->rb_lock);

	return 0;
}

static int diag_core_probe(struct platform_device *pdev)
{
	int ret;
	struct diag_core *dc;
	struct device *dev = &pdev->dev;

	dc = devm_kzalloc(dev, sizeof(*dc), GFP_KERNEL);
	if (dc == NULL) {
		dev_err(dev, "Out of memory\n");
		return -ENOMEM;
	}
	dc->pdev = pdev;

	INIT_LIST_HEAD(&dc->dm_head);
	spin_lock_init(&dc->dm_lock);
	init_waitqueue_head(&dc->wq_head);

	ret = diag_rbtree_init(dc);
	if(ret) {
		dev_err(dev, "Failed to initialize rbtree.\n");
		return ret;
	}

	dc->cycle_chk_thread = kthread_run(diag_cycle_check_thread, dc,
									"diag/periodic_chk");
	if(IS_ERR(dc->cycle_chk_thread)) {
		dev_err(dev, "Failed to create periodic check thread\n");
		dc->cycle_chk_thread = NULL;
		return -ENOMEM;
	}

	ret = diag_netlink_init(dc);
	if (ret) {
		dev_err(dev, "Failed to initialize diag netlink socket.\n");
		goto fail_nl;
	}

	dc->reportup_thread = kthread_run(diag_report_up_thread, dc,
									"diag/report_up");
	if (IS_ERR(dc->reportup_thread)) {
		dev_err(dev, "Fail to create event report-up thread\n");
		dc->reportup_thread = NULL;
		ret = -ENOMEM;
		goto fail_report;
	}

#ifdef CONFIG_DIAG_FAULT_INJECT
	dc->diag_fi_dir = debugfs_create_dir("diag_fault_inject", NULL);
	if (IS_ERR(dc->diag_fi_dir)) {
		dev_err(dev, "Failed to create fault inject debugfs entry\n");
		ret = PTR_ERR(dc->diag_fi_dir);
		goto fail_fi;
	}
#endif

	platform_set_drvdata(pdev, dc);
	dc_ctrl = dc;

	dev_info(dev, "Diagnonsis core probe successfully\n");

	return 0;

#ifdef CONFIG_DIAG_FAULT_INJECT
fail_fi:
	kthread_stop(dc->reportup_thread);
	dc->reportup_thread = NULL;
#endif
fail_report:
	netlink_kernel_release(dc->sk);
	dc->sk = NULL;
fail_nl:
	kthread_stop(dc->cycle_chk_thread);
	dc->cycle_chk_thread = NULL;
	return ret;
}

static int diag_core_remove(struct platform_device *pdev)
{
	struct diag_core *dc = platform_get_drvdata(pdev);

	if (dc == NULL)
		return 0;

	if (dc->cycle_chk_thread && !IS_ERR(dc->cycle_chk_thread))
		kthread_stop(dc->cycle_chk_thread);

	if (dc->reportup_thread && !IS_ERR(dc->reportup_thread))
		kthread_stop(dc->reportup_thread);

	debugfs_remove(dc->diag_fi_dir);

	dc_ctrl = NULL;

	return 0;
}

static const struct of_device_id diag_core_match[] = {
	{ .compatible = "axera,diag-core" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, diag_core_match);

static struct platform_driver diag_core_driver = {
	.probe	= diag_core_probe,
	.remove = diag_core_remove,
	.driver = {
		.name = "diag-core",
		.of_match_table = diag_core_match,
		.dev_groups = diag_core_groups,
	}
};
module_platform_driver(diag_core_driver);

MODULE_AUTHOR("xinlu.wang");
MODULE_DESCRIPTION("Diagnosis Framework Core");
MODULE_LICENSE("GPL v2");