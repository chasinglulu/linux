// SPDX-License-Identifier: GPL-2.0+
/*
 * Hardware-specific information display driver
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "hwinfo: " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/nvmem-consumer.h>

#include "hwinfo.h"

static struct hwinfo_priv *hwinfo_priv = NULL;
static struct proc_dir_entry *root_hwinfo_dir;
static u64 hwinfo_uid = 0;

static const char *proc_entry_name[] = {
	[PROC_UID]          = "uid",
	[PROC_BOARD_ID]     = "boardid",
	[PROC_CHIP_NAME]    = "chip-name",
	[PROC_BOOT_LOG]     = "bootlog",
	[PROC_BOOT_DEVICE]  = "bootdev",
	[PROC_ABORT_MAIN]   = "main-abort",
	[PROC_ABORT_SAFETY] = "safety-abort",
	[PROC_VERSION]      = "sdk_version",
	[PROC_CHIP_TYPE]    = "chip-type",
	[PROC_BOOT_SLOT]    = "boot-slot",
};

#define UID_EFUSE_OFFSET    0x0
#define UID_EFUSE_SIZE      0x8

static int hwinfo_efuse_match_nvmem(struct device *dev, const void *data)
{
	if (strcmp(dev_name(dev), "ax-efuse0") == 0)
		return 1;
	else
		return 0;
}

static int read_efuse(unsigned int offset, size_t bytes, void *buf)
{
	struct nvmem_device *nvmem;

	nvmem = nvmem_device_find(NULL, &hwinfo_efuse_match_nvmem);
	if (IS_ERR_OR_NULL(nvmem)) {
		return -ENODEV;
	}
	return nvmem_device_read(nvmem, offset, bytes, buf);
}

static int hwinfo_get_uid(void)
{
	int ret;

	ret = read_efuse(UID_EFUSE_OFFSET, UID_EFUSE_SIZE, (void *)&hwinfo_uid);
	if (ret < 0) {
		pr_err("Failed to read uid from efuse\n");
		return ret;
	}

	return 0;
}

static int hwinfo_uid_proc_show(struct seq_file *m, void *v)
{
	int ret;

	ret = hwinfo_get_uid();
	if (ret < 0) {
		pr_err("Failed to get uid\n");
		return ret;
	}

	seq_printf(m, "UID: 0x%016llx\n", hwinfo_uid);
	return 0;
}

static int hwinfo_uid_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_uid_proc_show, NULL);
}

static struct proc_ops hwinfo_uid_fops = {
	.proc_open      = hwinfo_uid_proc_open,
	.proc_release   = single_release,
	.proc_read      = seq_read,
	.proc_lseek     = seq_lseek,
};

static int hwinfo_board_id_proc_show(struct seq_file *m, void *v)
{
	if (unlikely(!hwinfo_priv)) {
		pr_err("Could not get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	seq_printf(m, "0x%x\n", hwinfo_priv->borad_id);
	return 0;
}

static int hwinfo_board_id_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_board_id_proc_show, NULL);
}

static struct proc_ops hwinfo_board_id_fops = {
	.proc_open       = hwinfo_board_id_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int hwinfo_chip_name_proc_show(struct seq_file *m, void *v)
{
	if (unlikely(!hwinfo_priv)) {
		pr_err("Could not get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	seq_printf(m, "%s\n", hwinfo_priv->chip_name ?: "Unknown");
	return 0;
}

static int hwinfo_chip_name_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_chip_name_proc_show, NULL);
}

static struct proc_ops hwinfo_chip_name_fops = {
	.proc_open       = hwinfo_chip_name_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int hwinfo_boot_log_proc_show(struct seq_file *m, void *v)
{
	void __iomem *console_record;
	char *line, *tmp, *console_buf;

	if (unlikely(!hwinfo_priv)) {
		pr_err("Could get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL((void *)hwinfo_priv->bootlog_record_addr)) {
		dev_err(hwinfo_priv->dev, "Invaild bootloader console record address\n");
		return 0;
	}

	console_record = memremap(hwinfo_priv->bootlog_record_addr,
	                    hwinfo_priv->bootlog_record_len, MEMREMAP_WB);
	if (IS_ERR_OR_NULL(console_record)) {
		dev_err(hwinfo_priv->dev, "Failed to remap bootloader console record\n");
		return 0;
	}

	console_buf = kmalloc(hwinfo_priv->bootlog_record_len, GFP_KERNEL);
	if (!console_buf) {
		dev_err(hwinfo_priv->dev, "Failed to allocate memory for console record buffer\n");
		iounmap(console_record);
		return -ENOMEM;
	}
	memcpy(console_buf, console_record, hwinfo_priv->bootlog_record_len);

	seq_printf(m, "\nBootloader Console Record: \n");
	seq_printf(m, "-------------------------------BEGIN------------------------------------\n");

	line = console_buf;
	while (line < console_buf + hwinfo_priv->bootlog_record_len) {
		tmp = strchr(line, '\n');
		if (tmp) {
			*tmp = '\0';
			if (line[0] != '\0') {
				seq_printf(m, "%s\n", line);
			} else {
				seq_printf(m, "\n");
			}
			line = tmp + 1;
		} else {
			if (line[0] != '\0')
				seq_printf(m, "%s\n", line);
			break;
		}
	}
	seq_printf(m, "--------------------------------END-------------------------------------\n");

	memunmap(console_record);
	kfree(console_buf);

	return 0;
}

static int hwinfo_boot_log_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_boot_log_proc_show, NULL);
}

static struct proc_ops hwinfo_boot_log_fops = {
	.proc_open       = hwinfo_boot_log_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int hwinfo_bootdev_proc_show(struct seq_file *m, void *v)
{
	if (unlikely(!hwinfo_priv)) {
		pr_err("Could get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	if (hwinfo_priv->bootdev_name)
		seq_printf(m, "%s (0x%x)\n", hwinfo_priv->bootdev_name, hwinfo_priv->bootdev);
	else
		seq_printf(m, "Unknown\n");

	return 0;
}

static int hwinfo_bootdev_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_bootdev_proc_show, NULL);
}

static struct proc_ops hwinfo_bootdev_fops = {
	.proc_open       = hwinfo_bootdev_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int hwinfo_abort_safety_proc_show(struct seq_file *m, void *v)
{
	if (unlikely(!hwinfo_priv)) {
		pr_err("Could get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	if (hwinfo_priv->safety_abort_name)
		seq_printf(m, "%s (0x%x)\n", hwinfo_priv->safety_abort_name, hwinfo_priv->safety_abort);
	else
		seq_printf(m, "Unknown\n");

	return 0;
}

static int hwinfo_abort_safety_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_abort_safety_proc_show, NULL);
}

static struct proc_ops hwinfo_abort_safety_fops = {
	.proc_open       = hwinfo_abort_safety_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int hwinfo_abort_main_proc_show(struct seq_file *m, void *v)
{
	if (unlikely(!hwinfo_priv)) {
		pr_err("Could get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	if (hwinfo_priv->main_abort_name)
		seq_printf(m, "%s (0x%x)\n", hwinfo_priv->main_abort_name, hwinfo_priv->main_abort);
	else
		seq_printf(m, "Unknown\n");

	return 0;
}

static int hwinfo_abort_main_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_abort_main_proc_show, NULL);
}

static struct proc_ops hwinfo_abort_main_fops = {
	.proc_open       = hwinfo_abort_main_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int hwinfo_version_proc_show(struct seq_file *m, void *v)
{
	if (unlikely(!hwinfo_priv)) {
		pr_err("Could get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	if (hwinfo_priv->version)
		seq_printf(m, "SDK Version %s\n", hwinfo_priv->version);
	else
		seq_printf(m, "SDK Version Unknown\n");

	return 0;
}

static int hwinfo_version_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_version_proc_show, NULL);
}

static struct proc_ops hwinfo_version_fops = {
	.proc_open       = hwinfo_version_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int hwinfo_bootslot_proc_show(struct seq_file *m, void *v)
{
	if (unlikely(!hwinfo_priv)) {
		pr_err("Could get hwinfo within %s\n", __func__);
		return -EINVAL;
	}

	if (hwinfo_priv->bootslot >= 0 && hwinfo_priv->bootslot <= 1)
		seq_printf(m, "Boot Slot: %c\n", 'A' + hwinfo_priv->bootslot);
	else
		seq_printf(m, "Boot Slot: Unknown\n");

	return 0;
}

static int hwinfo_bootslot_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwinfo_bootslot_proc_show, NULL);
}

static struct proc_ops hwinfo_bootslot_fops = {
	.proc_open       = hwinfo_bootslot_proc_open,
	.proc_release    = single_release,
	.proc_read       = seq_read,
	.proc_lseek      = seq_lseek,
};

static int init_hwinfo_proc(void)
{
	struct proc_ops *hwinfo_proc_ops[] = {
		[PROC_UID]          = &hwinfo_uid_fops,
		[PROC_BOARD_ID]     = &hwinfo_board_id_fops,
		[PROC_CHIP_NAME]    = &hwinfo_chip_name_fops,
		[PROC_BOOT_LOG]     = &hwinfo_boot_log_fops,
		[PROC_BOOT_DEVICE]  = &hwinfo_bootdev_fops,
		[PROC_ABORT_MAIN]   = &hwinfo_abort_main_fops,
		[PROC_ABORT_SAFETY] = &hwinfo_abort_safety_fops,
		[PROC_VERSION]      = &hwinfo_version_fops,
		[PROC_CHIP_TYPE]    = NULL,  // Not implemented
		[PROC_BOOT_SLOT]    = &hwinfo_bootslot_fops,
	};
	struct proc_dir_entry *proc_entry;
	int i;

	/* create /proc/hwinfo */
	root_hwinfo_dir = proc_mkdir("hwinfo", NULL);
	if (!root_hwinfo_dir) {
		pr_err("Failed to create /proc/hwinfo\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(proc_entry_name); i++) {
		if (!proc_entry_name[i] || !hwinfo_proc_ops[i]) {
			pr_warn("Invalid proc entry name or ops\n");
			continue;
		}

		proc_entry = proc_create_data(proc_entry_name[i], 0444, root_hwinfo_dir,
						   hwinfo_proc_ops[i], NULL);
		if (IS_ERR_OR_NULL(proc_entry)) {
			pr_warn("Failed to create %s\n", proc_entry_name[i]);
		}
	}

	return 0;
}

static int remove_hwinfo_proc(void)
{
	int i;

	if (unlikely(!root_hwinfo_dir)) {
		pr_debug("Not initialized root_hwinfo_dir\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(proc_entry_name); i++) {
		if (!proc_entry_name[i]) {
			pr_warn("Invalid proc entry name\n");
			continue;
		}

		remove_proc_entry(proc_entry_name[i], root_hwinfo_dir);
	}
	remove_proc_entry("hwinfo", NULL);
	root_hwinfo_dir = NULL;

	return 0;
}

static int hwinfo_probe(struct platform_device *pdev)
{
	struct hwinfo_priv *priv;
	struct device *dev = &pdev->dev;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (IS_ERR_OR_NULL(priv)) {
		dev_err(dev, "out of memory\n");
		return -ENOMEM;
	}
	priv->dev = dev;
	hwinfo_priv = priv;

	if (of_property_read_u32(dev->of_node, "axera,board-id",
			   &priv->borad_id)) {
		dev_err(dev, "Unable to read 'axera,board-id'");
		return -EINVAL;
	}

	if (of_property_read_string(dev->of_node, "axera,chip-name",
			   &priv->chip_name)) {
		dev_err(dev, "Unable to read 'axera,chip-name'");
		return -EINVAL;
	}

	if (of_property_read_u64(dev->of_node, "axera,bootlog-record-addr",
				&priv->bootlog_record_addr)) {
		dev_warn(dev, "Unable to read 'axera,bootlog-record-addr'");
		priv->bootlog_record_addr = -1ULL;
	}

	if (of_property_read_u64(dev->of_node, "axera,bootlog-record-len",
				&priv->bootlog_record_len)) {
		dev_warn(dev, "Unable to read 'axera,bootlog-record-len'");
		priv->bootlog_record_len = 0;
	}

	if (of_property_read_string(dev->of_node, "axera,boot-device",
				&priv->bootdev_name)) {
		dev_warn(dev, "Unable to read 'axera,boot-device'");
		priv->bootdev_name = NULL;
	}

	if (of_property_read_u32(dev->of_node, "axera,boot-device-id",
				&priv->bootdev)) {
		dev_warn(dev, "Unable to read 'axera,boot-device-id'");
		priv->bootdev = -1U;
	}

	if (of_property_read_string(dev->of_node, "axera,safety-abort-name",
				&priv->safety_abort_name)) {
		dev_warn(dev, "Unable to read 'axera,safety-abort-name'");
		priv->safety_abort_name = NULL;
	}

	if (of_property_read_u32(dev->of_node, "axera,safety-abort",
				&priv->safety_abort)) {
		dev_warn(dev, "Unable to read 'axera,safety-abort'");
		priv->safety_abort = -1U;
	}

	if (of_property_read_string(dev->of_node, "axera,main-abort-name",
				&priv->main_abort_name)) {
		dev_warn(dev, "Unable to read 'axera,main-abort-name'");
		priv->main_abort_name = NULL;
	}

	if (of_property_read_u32(dev->of_node, "axera,main-abort",
				&priv->main_abort)) {
		dev_warn(dev, "Unable to read 'axera,main-abort'");
		priv->main_abort = -1U;
	}

	if (of_property_read_string(dev->of_node, "axera,version",
				&priv->version)) {
		dev_warn(dev, "Unable to read 'axera,version'");
		priv->version = NULL;
	}

	if (of_property_read_u32(dev->of_node, "axera,boot-slot",
				&priv->bootslot)) {
		dev_warn(dev, "Unable to read 'axera,boot-slot'");
		priv->bootslot = -1U;
	}

	dev_info(dev, "%s: done\n", __func__);
	return 0;
}

static int hwinfo_remove(struct platform_device *pdev)
{
	if (hwinfo_priv) {
		devm_kfree(hwinfo_priv->dev, hwinfo_priv);
		hwinfo_priv = NULL;
	}
	return 0;
}

static const struct of_device_id hwinfo_ids[] = {
	{ .compatible = "axera,hwinfo-display" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, hwinfo_ids);

static struct platform_driver hwinfo_driver = {
	.probe = hwinfo_probe,
	.remove = hwinfo_remove,
	.driver = {
		.name = "hwinfo",
		.of_match_table = hwinfo_ids,
	},
};
module_platform_driver(hwinfo_driver);

static int __init hwinfo_init(void)
{
	int ret;

	ret = init_hwinfo_proc();
	if (ret < 0) {
		pr_err("Failed to create hwinfo proc\n");
		return ret;
	}
	return 0;
}

static void __exit hwinfo_exit(void)
{
	remove_hwinfo_proc();
}

late_initcall(hwinfo_init);
module_exit(hwinfo_exit);

MODULE_DESCRIPTION("hardware-specific information display driver");
MODULE_AUTHOR("Charleye <wangkart@aliyun.com>");
MODULE_LICENSE("GPL v2");