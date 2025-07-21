// SPDX-License-Identifier: GPL-2.0+
/*
 * Hardware-specific information display driver
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL: " fmt

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

#include "a55_stl.h"

extern void a55_stl_core_diagnose_p001(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p002(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p003(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p004(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p005(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p006(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p007(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p008(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p009(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p010(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p011(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p012(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p013(a55_stl_state_t *fctlr_base);
extern void a55_stl_core_diagnose_p014(a55_stl_state_t *fctlr_base);

static int __init a55_stl_init(void)
{
	pr_info("A55 STL initialization started\n");
	a55_stl_core_diagnose_p001(NULL);
	a55_stl_core_diagnose_p002(NULL);
	a55_stl_core_diagnose_p003(NULL);
	a55_stl_core_diagnose_p004(NULL);
	a55_stl_core_diagnose_p005(NULL);
	a55_stl_core_diagnose_p006(NULL);
	a55_stl_core_diagnose_p007(NULL);
	a55_stl_core_diagnose_p008(NULL);
	a55_stl_core_diagnose_p009(NULL);
	a55_stl_core_diagnose_p010(NULL);
	a55_stl_core_diagnose_p011(NULL);
	a55_stl_core_diagnose_p012(NULL);
	a55_stl_core_diagnose_p013(NULL);
	a55_stl_core_diagnose_p014(NULL);
	pr_info("A55 STL initialization completed\n");
	return 0;
}

static void __exit a55_stl_exit(void)
{
	pr_err("A55 STL exit called\n");
}

late_initcall(a55_stl_init);
module_exit(a55_stl_exit);

MODULE_DESCRIPTION("A55 STL");
MODULE_AUTHOR("Charleye <wangkart@aliyun.com>");
MODULE_LICENSE("GPL v2");