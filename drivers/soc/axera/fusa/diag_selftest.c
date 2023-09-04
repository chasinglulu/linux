/*
 * SPDX-License-Identifier: GPL-2.0+
 * 
 * Diagnosis selftest
 * 
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */
#define pr_fmt(fmt) "diag-selftest: " fmt

#include <linux/delay.h>
#include "diag_core.h"

#define SELFTEST_ERR_CNT	9

static bool initialized = false;
static bool executed = false;

struct diag_selftest_data {
	int cycle_chk_cnt;
	int error_cb_cnt;
} data = { 0, 0};

static int diag_selftest_cycle_check(void *data)
{
	struct diag_selftest_data *d = data;

	d->cycle_chk_cnt++;

	pr_debug("running diag selftest periodic check, %d\n", d->cycle_chk_cnt);
	return 0;
}

static int error_cb(void *data)
{
	struct diag_selftest_data *d = data;

	d->error_cb_cnt++;

	pr_debug("running diag selftest error callback\n");
	return 0;
}

static struct diag_register_info diag_selftest = {
	DIAG_REGISTER_INFO(DIGA_MID_DFC, SELFTEST_ERR_CNT,
						diag_selftest_cycle_check, &data) {
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_RBACK_CHCEK, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_INT_MISSING_CHECK, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_WRITE_RBACK_CHECK, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_PERIOD_CALLBACK, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_ECC_CHECK, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_ECC_CORRECTABLE, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_PARITY_CHECK, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_INT_CALLBACK, error_cb),
		DIAG_REGISTER_INFO_HANDLE(DIAG_EID_LOCKSTEP_CHECK, error_cb),
		DIAG_REGISTER_INFO_HANDLE_GUARD(DIAG_EID_COMMON_MAX),	//sentinel
	},
};

DIAG_ERROR(rb_chk, DIGA_MID_DFC, DIAG_EID_RBACK_CHCEK);
DIAG_ERROR(int_mis_chk, DIGA_MID_DFC, DIAG_EID_INT_MISSING_CHECK);
DIAG_ERROR(wrb_chk, DIGA_MID_DFC, DIAG_EID_WRITE_RBACK_CHECK);
DIAG_ERROR(period_chk, DIGA_MID_DFC, DIAG_EID_PERIOD_CALLBACK);
DIAG_ERROR(ecc_chk, DIGA_MID_DFC, DIAG_EID_ECC_CHECK);
DIAG_ERROR(ecc_cor_chk, DIGA_MID_DFC, DIAG_EID_ECC_CORRECTABLE);
DIAG_ERROR(parity_chk, DIGA_MID_DFC, DIAG_EID_PARITY_CHECK);
DIAG_ERROR(int_cb_chk, DIGA_MID_DFC, DIAG_EID_INT_CALLBACK);
DIAG_ERROR(lockstep_chk, DIGA_MID_DFC, DIAG_EID_LOCKSTEP_CHECK);
struct diag_error *errors[] = {
	&de_rb_chk,
	&de_int_mis_chk,
	&de_wrb_chk,
	&de_period_chk,
	&de_ecc_chk,
	&de_ecc_cor_chk,
	&de_parity_chk,
	&de_int_cb_chk,
	&de_lockstep_chk,
	NULL,
};

int diag_selftest_check(void)
{
	struct diag_error_mgt *mgt;
	int i;

	if (!initialized) {
		pr_err("The module%d not registered\n", DIGA_MID_DFC);
		return 0;
	}

	if (!executed) {
		pr_err("the diag selftest not run\n");
		return 0;
	}

	//TODO: msleep(1000);
	BUG_ON(data.cycle_chk_cnt <= 0);

	for (i = DIAG_EID_RBACK_CHCEK; i <= DIAG_EID_LOCKSTEP_CHECK; i++) {
		mgt = diagnosis_lookup_error(DIGA_MID_DFC, i);
		if (!mgt) {
			WARN(1, "failed to find out the error [%d]\n", i);
			continue;
		}

		BUG_ON(mgt->number != 1 || atomic_read(&mgt->pending));
	}

	return 0;
}

int diag_selftest_run(void)
{
	int retval;
	int i;

	if (!initialized) {
		pr_err("The module[%d] not registered\n", DIGA_MID_DFC);
		return 0;
	}

	for (i = DIAG_EID_RBACK_CHCEK; i <= DIAG_EID_LOCKSTEP_CHECK; i++) {
		retval = diagnosis_error_send(errors[i]);
		if (retval)
			WARN(1, "failed to report the error [%d]\n", i);

		BUG_ON(data.error_cb_cnt != i + 1);
	}

	executed = true;

	return 0;
}

int diag_selftest_init(void)
{
	int retval;

	retval = diagnosis_register(&diag_selftest);
	if (retval)
		BUG();

	initialized = true;

	return 0;
}

int diag_selftest_deinit(void)
{
	int retval;

	retval = diagnosis_deregister(DIGA_MID_DFC);
	if (retval)
		WARN(1, "failed to deregister the module [%d]\n", DIGA_MID_DFC);

	initialized = false;
	executed = false;
	memset(&data, 0, sizeof(data));

	return 0;
}