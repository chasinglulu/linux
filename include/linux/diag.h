/*
 * SPDX-License-Identifier: GPL-2.0+
 * 
 * Diagnosis header used for device driver
 *
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */

#ifndef DIAG_H_
#define DIAG_H_

#include <linux/types.h>

#define DIAG_ERROR_THRESHOLD    16

enum diag_error_id {
	DIAG_EID_RBACK_CHCEK_ERR,
	DIAG_EID_INT_MISSING_CHECK_ERR,
	DIAG_EID_WRITE_READ_BACK_CHECK_ERR,
};

enum diag_error_prio {
	DIAG_ERR_PRIO_0,
	DIAG_ERR_PRIO_1,
	DIAG_ERR_PRIO_2,
	DIAG_ERR_PRIO_3,
	DIAG_ERR_PRIO_MAX,
};

/**
 * struct diag_error - error message used to send
 *
 */
struct diag_error {
	uint64_t time;	/* The time at which the error occurred */
	uint32_t code;	/* error code number */
	uint16_t mid;	/* module ID to which the error belongs */
	uint16_t eid;	/* error ID */
	uint8_t length;		/* the length of vaild payload */
	uint8_t *payload[0];
}__attribute__((packed));

#define DIAG_ERROR(name, _mid, _eid)	\
	struct diag_error de_##name = {		\
		.mid = _mid,					\
		.eid = _eid,					\
	}

struct diag_error *diag_error_alloc(uint16_t mid, uint16_t eid);
void diag_error_set_time(struct diag_error *de);

struct diag_error_attr {
	enum diag_error_prio prio;	/* error priority */
	uint16_t threshold;			/* error criticality level boost threshold */
	bool enable;				/* error enable */
	bool repeat;				/* error repeat during the sending error cycle*/
}__attribute__((packed));

#define DIAG_ERROR_ATTR(ee, rt, po, td)		\
			.enable = ee,					\
			.repeat = rt,					\
			.prio = po,						\
			.threshold = td,				\

#define DIAG_ERROR_ATTR_DEFAULT(threshold)	\
			DIAG_ERROR_ATTR(true, false, DIAG_ERR_PRIO_0, threshold)

/**
 * struct diag_error_handle - record error info
 *
 * @eid: error ID
 * @name: error name
 * @min_snd_ms: minimum interval between error message send
 * @max_snd_ms: maximum interval between error message send
 * @callback: callback function used to handle error
 * @data: data used by callback function
 */
struct diag_error_handle {
	char *name;
	uint16_t eid;
	uint32_t min_snd_ms;
	uint32_t max_snd_ms;
	struct diag_error_attr attr;
	int (*err_handle_cb)(void *data);
};

#define DIAG_ERROR_HANDLE(id, min, max, cb)		\
				.eid = id,						\
				.name = #id,					\
				.min_snd_ms = min,				\
				.max_snd_ms = max,				\
				.err_handle_cb = cb				\

#define DIAG_ERROR_HANDLE_ATTR(threshold)			\
			.attr = {								\
				DIAG_ERROR_ATTR_DEFAULT(threshold)	\
			}

#define DIAG_ERROR_HANDLE_DEFAULT(eid, cb)			\
			DIAG_ERROR_HANDLE(eid, 0, 1000, cb)

/**
 * diag_register_info - info to register diag module and error.
 *
 * @mid: module ID
 * @error_cnt: the count of evnent to register
 * @handle: struct diag_error_handle objects
 */
struct diag_register_info {
	uint16_t mid;
	uint16_t error_cnt;
	int (*cycle)(void *data);
	void *data;
	struct diag_error_handle handle[];
};

#define DIAG_REGISTER_INFO(_mid, cnt, _cycle, _data)	\
				.mid = _mid,							\
				.error_cnt = cnt,						\
				.cycle = _cycle,						\
				.data = _data,							\
				.handle =

#define DIAG_REGISTER_INFO_HANDLE(_eid, cb)						\
			[_eid] = {											\
				DIAG_ERROR_HANDLE_DEFAULT(_eid, cb),			\
				DIAG_ERROR_HANDLE_ATTR(DIAG_ERROR_THRESHOLD),	\
			}

#define DIAG_REGISTER_INFO_HANDLE_GUARD(eid_max)				\
			[eid_max] = {										\
				DIAG_ERROR_HANDLE_DEFAULT(0xffff, NULL),		\
			}


int diagnosis_register(const struct diag_register_info *reg_info);
int diagnosis_error_send(struct diag_error *error);
int diagnosis_deregister(uint16_t mid);

#endif