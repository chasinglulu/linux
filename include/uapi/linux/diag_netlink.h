/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Diagnosis netlink definition
 * 
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */

#ifndef _DIAG_NETLINK_H_
#define _DIAG_NETLINK_H_

#include <linux/diag.h>

#define MAX_ERR_PAYLAOD		256
#define DIAG_MSG_VER		0x0100	// 1.0
#define MAX_NAME			32
// include/uapi/linux/netlink.h
#define NETLINK_SOC_DIAG	23

struct diag_msg_header {
	uint32_t version;
	uint64_t checksum;	/* checksum of diag msg data */
	uint64_t reseved;
}__attribute__((packed));

struct diag_msg_data {
	char name[MAX_NAME];
	enum diag_error_prio prio;
	struct diag_error err;
}__attribute__((packed));

struct diag_msg {
	struct diag_msg_header head;
	struct diag_msg_data data;
};

#endif
