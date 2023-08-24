/*
 * SPDX-License-Identifier: GPL-2.0+
 * 
 * Diagnosis Core test
 * 1. two register thread, used to test register and deregister API concurrently
 * 2. three send thread, used to test send API concurrently
 * 
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */

#define pr_fmt(fmt) "diag-test: " fmt

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/diag.h>
#include <linux/kthread.h>
#include <linux/sched.h>

enum test_module_id {
	TEST_SEND_ID = DIAG_MID_MAX,
	TEST_REG0_ID,
	TEST_REG1_ID,
};

enum test_err_cnt {
	TEST_ERR_CNT = 5,
};

enum err_code {
	test_err_0,
	test_err_1,
	test_err_2,
	test_err_3,
	test_err_4,
	test_err_max,
};

struct task_struct *t1, *t2, *t3, *t4, *t5;

int send_err_thread1(void *data)
{
	DIAG_ERROR(err0, TEST_SEND_ID, test_err_0);
	do {
		set_current_state(TASK_INTERRUPTIBLE);
		diagnosis_error_send(&de_err0);
		schedule_timeout(msecs_to_jiffies(100));
	}while(!kthread_should_stop());

	return 0;
}

int send_err_thread2(void *data)
{
	DIAG_ERROR(err1, TEST_SEND_ID, test_err_1);
	do {
		set_current_state(TASK_INTERRUPTIBLE);
		diagnosis_error_send(&de_err1);
		schedule_timeout(msecs_to_jiffies(150));
	}while(!kthread_should_stop());

	return 0;
}

int send_err_thread3(void *data)
{
	DIAG_ERROR(err2, TEST_SEND_ID, test_err_2);
	do {
		set_current_state(TASK_INTERRUPTIBLE);
		diagnosis_error_send(&de_err2);
		schedule_timeout(msecs_to_jiffies(200));
	}while(!kthread_should_stop());

	return 0;
}

int cycle_check(void *data)
{
	static int count = 0;
	pr_info("curr count = %d\n", count++);

	return 0;
}

struct diag_register_info axera_test = {
	DIAG_REGISTER_INFO(TEST_SEND_ID, TEST_ERR_CNT, cycle_check, NULL) {
		DIAG_REGISTER_INFO_HANDLE(test_err_0, NULL),
		DIAG_REGISTER_INFO_HANDLE(test_err_1, NULL),
		DIAG_REGISTER_INFO_HANDLE(test_err_2, NULL),
		DIAG_REGISTER_INFO_HANDLE(test_err_3, NULL),
		DIAG_REGISTER_INFO_HANDLE(test_err_4, NULL),
		DIAG_REGISTER_INFO_HANDLE_GUARD(test_err_max),	//sentinel
	},
};

struct diag_register_info axera_test_reg = {
	DIAG_REGISTER_INFO(TEST_REG0_ID, 2, NULL, NULL) {
		DIAG_REGISTER_INFO_HANDLE(test_err_0, NULL),
		DIAG_REGISTER_INFO_HANDLE(test_err_1, NULL),
		DIAG_REGISTER_INFO_HANDLE_GUARD(test_err_max),	//sentinel
	},
};

int test_register_thread4(void *data)
{
	static int loop = 0;
	do {
		set_current_state(TASK_INTERRUPTIBLE);
		pr_info("t4: loop = %d\n", loop++);
		diagnosis_register(&axera_test_reg);
		pr_info("t4: \tregistered\n");
		schedule_timeout(msecs_to_jiffies(1000));
		diagnosis_deregister(TEST_REG0_ID);
		pr_info("t4: \tderegistered\n");
	} while (!kthread_should_stop());

	return 0;
}

struct diag_register_info axera_test1_reg = {
	DIAG_REGISTER_INFO(TEST_REG1_ID, 3, NULL, NULL) {
		DIAG_REGISTER_INFO_HANDLE(test_err_0, NULL),
		DIAG_REGISTER_INFO_HANDLE(test_err_1, NULL),
		DIAG_REGISTER_INFO_HANDLE(test_err_2, NULL),
		DIAG_REGISTER_INFO_HANDLE_GUARD(test_err_max),	//sentinel
	},
};

int test_register_thread5(void *data)
{
	static int loop = 0;
	do {
		set_current_state(TASK_INTERRUPTIBLE);
		pr_info("t5: loop = %d\n", loop++);
		diagnosis_register(&axera_test1_reg);
		pr_info("t5: \tregistered\n");
		schedule_timeout(msecs_to_jiffies(2000));
		diagnosis_deregister(TEST_REG1_ID);
		pr_info("t5: \tderegistered\n");
	} while (!kthread_should_stop());

	return 0;
}

static int __init diag_test_init(void)
{
	pr_info("Entering: %s\n", __FUNCTION__);

	diagnosis_register(&axera_test);

	t1 = kthread_run(send_err_thread1, NULL, "diag-test/t1");
	if(IS_ERR(t1)) {
		pr_err("Failed to thread1\n");
		t1 = NULL;
	}

	t2 = kthread_run(send_err_thread2, NULL, "diag-test/t2");
	if(IS_ERR(t2)) {
		pr_err("Failed to thread2\n");
		t2 = NULL;
	}

	t3 = kthread_run(send_err_thread3, NULL, "diag-test/t3");
	if(IS_ERR(t3)) {
		pr_err("Failed to thread3\n");
		t3 = NULL;
	}

	t4 = kthread_run(test_register_thread4, NULL, "diag-test/t4");
	if(IS_ERR(t4)) {
		pr_err("Failed to thread4\n");
		t4 = NULL;
	}

	t5 = kthread_run(test_register_thread5, NULL, "diag-test/t5");
	if(IS_ERR(t5)) {
		pr_err("Failed to thread5\n");
		t5 = NULL;
	}

	return 0;
}
module_init(diag_test_init);

static void __exit diag_test_exit(void)
{
	pr_info("exiting diag test module.\n");

	if (t1) {
		kthread_stop(t1);
		t1 = NULL;
	}

	if (t2) {
		kthread_stop(t2);
		t2 = NULL;
	}

	if (t3) {
		kthread_stop(t3);
		t3 = NULL;
	}

	if (t4) {
		kthread_stop(t4);
		t4 = NULL;
	}

	if (t5) {
		kthread_stop(t5);
		t5 = NULL;
	}

	diagnosis_deregister(TEST_SEND_ID);
}
module_exit(diag_test_exit);

MODULE_AUTHOR("xinlu.wang");
MODULE_DESCRIPTION("Diagnosis API Test");
MODULE_LICENSE("GPL v2");
