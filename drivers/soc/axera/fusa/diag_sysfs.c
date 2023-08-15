/*
 * SPDX-License-Identifier: GPL-2.0+
 * 
 * Diagnosis sysfs
 * 
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */
#define pr_fmt(fmt) "diag-sysfs: " fmt

#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include "diag_core.h"

static ssize_t
enable_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->handle.attr.enable);
}
static ssize_t
enable_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);
	struct diag_error_attr *de_attr = &de_mgt->handle.attr;
	uint32_t val;

	if (kstrtouint(buf, 0, &val) < 0)
		return -EINVAL;

	if (val > 1)
		return -EINVAL;

	WRITE_ONCE(de_attr->enable, val);

	return count;
}
DEVICE_ATTR_RW(enable);

static ssize_t
prio_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->handle.attr.prio);
}
static ssize_t
prio_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct diag_error_mgt *err_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);
	struct diag_error_attr *err_attr = &err_mgt->handle.attr;
	uint32_t val;

	if (kstrtouint(buf, 0, &val) < 0)
		return -EINVAL;

	if (val > DIAG_ERR_PRIO_MAX)
		return -EINVAL;

	WRITE_ONCE(err_attr->prio, val);

	return count;
}
DEVICE_ATTR_RW(prio);

static ssize_t
repeat_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
											struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->handle.attr.repeat);
}
static ssize_t
repeat_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct diag_error_mgt *err_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);
	struct diag_error_attr *err_attr = &err_mgt->handle.attr;
	uint32_t val;

	if (kstrtouint(buf, 0, &val) < 0)
		return -EINVAL;

	if (val > 1)
		return -EINVAL;

	WRITE_ONCE(err_attr->repeat, val);

	return count;
}
DEVICE_ATTR_RW(repeat);

static ssize_t
threshold_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->handle.attr.threshold);
}
static ssize_t
threshold_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct diag_error_mgt *err_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);
	struct diag_error_attr *err_attr = &err_mgt->handle.attr;
	uint32_t val;

	if (kstrtouint(buf, 0, &val) < 0)
		return -EINVAL;

	WRITE_ONCE(err_attr->threshold, val);

	return count;
}
DEVICE_ATTR_RW(threshold);

static ssize_t
number_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->number);
}
DEVICE_ATTR_RO(number);

static ssize_t
last_time_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%llu\n", de_mgt->last_time);
}
DEVICE_ATTR_RO(last_time);

static ssize_t
pending_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", atomic_read(&de_mgt->pending));
}
DEVICE_ATTR_RO(pending);

static ssize_t
eid_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->handle.eid);
}
DEVICE_ATTR_RO(eid);

static ssize_t
max_snd_ms_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->handle.max_snd_ms);
}
static ssize_t
max_snd_ms_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct diag_error_mgt *err_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);
	struct diag_error_handle *handle = &err_mgt->handle;
	uint32_t val;

	if (kstrtouint(buf, 0, &val) < 0)
		return -EINVAL;

	WRITE_ONCE(handle->max_snd_ms, val);

	return count;
}
DEVICE_ATTR_RW(max_snd_ms);

static ssize_t
min_snd_ms_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_error_mgt *de_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", de_mgt->handle.min_snd_ms);
}
static ssize_t
min_snd_ms_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct diag_error_mgt *err_mgt = container_of(dev,
										struct diag_error_mgt, err_dev);
	struct diag_error_handle *handle = &err_mgt->handle;
	uint32_t val;

	if (kstrtouint(buf, 0, &val) < 0)
		return -EINVAL;

	WRITE_ONCE(handle->min_snd_ms, val);

	return count;
}
DEVICE_ATTR_RW(min_snd_ms);

static ssize_t
name_show(struct device *dev, struct device_attribute *attr, char *page)
{
	const char *name = dev_name(dev);
	struct diag_error_mgt *err_mgt;
	struct diag_module_mgt *mod_mgt;

	if (strncmp(name, "error", 5)) {
		mod_mgt = container_of(dev, struct diag_module_mgt, mod_dev);
		return scnprintf(page, PAGE_SIZE - 1, "%s\n", mod_mgt->name);
	}
	else {
		err_mgt = container_of(dev, struct diag_error_mgt, err_dev);
		return scnprintf(page, PAGE_SIZE - 1, "%s\n", err_mgt->handle.name);
	}

	return 0;
}
DEVICE_ATTR_RO(name);

static struct attribute *diag_error_attrs[] = {
	&dev_attr_name.attr,
	&dev_attr_enable.attr,
	&dev_attr_prio.attr,
	&dev_attr_threshold.attr,
	&dev_attr_last_time.attr,
	&dev_attr_repeat.attr,
	&dev_attr_number.attr,
	&dev_attr_pending.attr,
	&dev_attr_eid.attr,
	&dev_attr_max_snd_ms.attr,
	&dev_attr_min_snd_ms.attr,
	NULL,
};
ATTRIBUTE_GROUPS(diag_error);

static ssize_t
count_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_module_mgt *mod_mgt = container_of(dev,
										struct diag_module_mgt, mod_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", mod_mgt->error_cnt);
}
DEVICE_ATTR_RO(count);

static ssize_t
mid_show(struct device *dev, struct device_attribute *attr, char *page)
{
	struct diag_module_mgt *mod_mgt = container_of(dev,
										struct diag_module_mgt, mod_dev);

	return scnprintf(page, PAGE_SIZE - 1, "%u\n", mod_mgt->mid);
}
DEVICE_ATTR_RO(mid);

static struct attribute *diag_module_attrs[] = {
	&dev_attr_name.attr,
	&dev_attr_count.attr,
	&dev_attr_mid.attr,
	NULL,
};
ATTRIBUTE_GROUPS(diag_module);

void diag_dev_set_attrs(struct device *dev)
{
	if (strncmp(dev_name(dev), "error", 5))
		dev->groups = diag_module_groups;
	else
		dev->groups = diag_error_groups;
}