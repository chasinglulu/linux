// SPDX-License-Identifier: GPL-2.0
/*
 * Hardware-specific information display driver
 *
 * Copyright (C) 2024 Charleye <wangkart@aliyun.com>
 */

#ifndef _HWINFO_H_
#define _HWINFO_H_

#include <linux/ctype.h>

enum {
	PROC_UID,
	PROC_BOARD_ID,
	PROC_CHIP_NAME,
	PROC_BOOT_LOG,
	PROC_BOOT_DEVICE,
	PROC_ABORT_MAIN,
	PROC_ABORT_SAFETY,
	PROC_VERSION,
	PROC_CHIP_TYPE,
	PROC_BOOT_SLOT,
};

struct hwinfo_priv {
	struct device *dev;
	uint32_t borad_id;
	const char *chip_name;
	uint64_t bootlog_record_addr;
	uint64_t bootlog_record_len;
	uint32_t bootdev;
	const char *bootdev_name;
	uint32_t main_abort;
	uint32_t safety_abort;
	const char *main_abort_name;
	const char *safety_abort_name;
	const char *version;
	uint32_t bootslot;
};

#endif