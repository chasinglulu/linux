// SPDX-License-Identifier: GPL-2.0
/*
 * Hardware-specific information display driver
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com.com>
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
};

struct hwinfo_priv {
	struct device *dev;
	uint32_t borad_id;
	const char *chip_name;
	uint64_t bootlog_record_addr;
	uint64_t bootlog_record_len;
	uint32_t bootdev;
	const char *bootdev_name;
};

#endif