/*
 * SPDX-License-Identifier: GPL-2.0+
 * 
 * Diaginosis Core Fault Injection
 * 
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */
#define pr_fmt(fmt) "diag-fault-inject: " fmt

#include <linux/fault-inject.h>

#include "diag_core.h"

#ifdef CONFIG_DIAG_FAULT_INJECT
struct dentry *diag_fi_create_dir(struct device *dev, struct dentry *parent)
{
	struct dentry *dir;
	struct diag_module_mgt *dmm = NULL;
	struct diag_error_mgt *mgt = NULL;
	DECLARE_FAULT_ATTR(fi);

	if (IS_ERR_OR_NULL(dev))
		return ERR_PTR(-ENODEV);

	if (strncmp(dev_name(dev), "error", 5)) {
		dmm = container_of(dev, struct diag_module_mgt, mod_dev);
		dir = debugfs_create_dir(dev_name(dev), parent);
	} else {
		mgt = container_of(dev, struct diag_error_mgt, err_dev);
		memcpy(&mgt->fi, &fi, sizeof(fi));
		dir = fault_create_debugfs_attr(dev_name(dev), parent, &mgt->fi);
	}

	if (IS_ERR(dir))
		return dir;

	return dir;
}

void diag_fi_remove_dir(struct device *dev)
{
	struct diag_module_mgt *dmm = NULL;
	struct diag_error_mgt *mgt = NULL;
	DECLARE_FAULT_ATTR(fi);

	if (strncmp(dev_name(dev), "error", 5)) {
		dmm = container_of(dev, struct diag_module_mgt, mod_dev);
		debugfs_remove(dmm->dir);
		dmm->dir = NULL;
	} else {
		mgt = container_of(dev, struct diag_error_mgt, err_dev);
		dput(mgt->fi.dname);
		debugfs_remove_recursive(mgt->fi.dname);
		memcpy(&mgt->fi, &fi, sizeof(fi));
	}
}

static inline bool __should_fail_diag(struct diag_error_mgt *mgt)
{
	return should_fail(&mgt->fi, 1);
}

bool should_fail_diag(struct diag_error *err)
{
	struct diag_error_mgt *mgt;

	mgt = diagnosis_lookup_error(err->mid, err->eid);
	if (!mgt) {
		pr_err("Invalid error information\n");
		return false;
	}

	if (__should_fail_diag(mgt))
		return true;

	return false;
}
ALLOW_ERROR_INJECTION(should_fail_diag, ERRNO);
EXPORT_SYMBOL(should_fail_diag);

#else
inline struct dentry *diag_fi_create_dir(struct device *dev, struct dentry *parent)
{
	return NULL;
}

inline void diag_fi_remove_dir(struct device *dev)
{
}

bool should_fail_diag(struct diag_error *err)
{
	return false;
}
#endif