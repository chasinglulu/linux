/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Diagnosis Framework Core header
 *
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */

#ifndef DIAG_CORE_H
#define DIAG_CORE_H

#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/diag.h>
#include <linux/rbtree.h>
#include <linux/string.h>

#define CYCLE_CHK_PERIOD	3000	/* in ms */

enum diag_criticality {
	DIAG_NONCRITICAL,
	DIAG_CRITICAL,
};

/* Used to management errors */
struct diag_error_mgt {
	struct diag_error_handle handle;
	uint32_t number;	/* Number of occurrences of the error */
	uint64_t last_time;
	atomic_t pending;	/* Number of pending errors */
	struct list_head err_node;
	struct device err_dev;	/* for sysfs */
};

struct diag_core_error {
	struct rb_node rb_node;
	struct diag_error err;
	enum diag_error_prio prio;	/* use as a part of rbtree key*/
};

struct diag_rbtree_ctx {
	struct rb_root_cached root;
	spinlock_t rb_lock;
};

/* Used to management module */
struct diag_module_mgt {
	uint32_t mid;
	const char *name;
	struct list_head node;
	int (*cycle)(void *data);
	void *data;

	uint32_t error_cnt;
	struct list_head err_head;

	/* for sysfs */
	struct device mod_dev;
};

struct diag_core {
	struct platform_device *pdev;
	dev_t cdevno;
	struct cdev cdev;
	struct class *class;
	struct device *dev;

	uint32_t module_cnt;
	struct list_head dm_head;
	spinlock_t dm_lock;

	struct task_struct *cycle_chk_thread;
	struct task_struct *reportup_thread;
	wait_queue_head_t wq_head;

	struct sock *sk;
	void *rbtree;
};

uint32_t dc_get_module_count(struct diag_core *dc);

struct diag_core_module *dcm_alloc(uint32_t mid, uint32_t evtcnt);
uint32_t dcm_get_evtcnt(struct diag_core_module *dcm);

struct diag_error_mgt *diagnosis_lookup_error(uint16_t mid, uint16_t eid);

static inline const char *dcm_get_name(struct diag_module_mgt *dmm)
{
	return dmm->name;
}

static inline void dcm_set_name(struct diag_module_mgt *dmm, const char *name)
{
	dmm->name = kstrdup(name, GFP_KERNEL);
}

int diag_netlink_init(struct diag_core *dc);
int diag_netlink_send_msg(struct sock *sk, struct diag_error *err);

void diag_dev_set_attrs(struct device *dev);

#endif /* DIAG_CORE_H */
