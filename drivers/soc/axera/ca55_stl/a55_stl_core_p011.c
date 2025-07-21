/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of CRC32 and CRC32C instructions (continuous calculation and comparison of two golden values)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P011: " fmt

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>

#include "a55_stl_constants.h"
#include "a55_stl_utils.h"
#include "a55_stl.h"

static inline int test_crc32(
    uint64_t init, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t in5,
    uint64_t in6, uint64_t in7, uint64_t in8, uint64_t in9, uint64_t golden)
{
    uint64_t r = init;
    asm volatile(
        "crc32b %w[r], %w[r], %w[in2]\n\t"
        "crc32b %w[r], %w[r], %w[in3]\n\t"
        "crc32h %w[r], %w[r], %w[in4]\n\t"
        "crc32h %w[r], %w[r], %w[in5]\n\t"
        "crc32w %w[r], %w[r], %w[in6]\n\t"
        "crc32w %w[r], %w[r], %w[in7]\n\t"
        "crc32x %w[r], %w[r], %x[in8]\n\t"
        "crc32x %w[r], %w[r], %x[in9]\n\t"
        : [r] "+r"(r)
        : [in2] "r"(in2), [in3] "r"(in3), [in4] "r"(in4), [in5] "r"(in5),
          [in6] "r"(in6), [in7] "r"(in7), [in8] "r"(in8), [in9] "r"(in9)
    );
    if ((r & 0xFFFFFFFF) != (golden & 0xFFFFFFFF)) {
        pr_err("[%d] CRC32: got=0x%llx, expect=0x%llx\n", __LINE__, r & 0xFFFFFFFF, golden & 0xFFFFFFFF);
        return -1;
    }
    return 0;
}

static inline int test_crc32c(
    uint64_t init, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t in5,
    uint64_t in6, uint64_t in7, uint64_t in8, uint64_t in9, uint64_t golden)
{
    uint64_t r = init;
    asm volatile(
        "crc32cb %w[r], %w[r], %w[in2]\n\t"
        "crc32cb %w[r], %w[r], %w[in3]\n\t"
        "crc32ch %w[r], %w[r], %w[in4]\n\t"
        "crc32ch %w[r], %w[r], %w[in5]\n\t"
        "crc32cw %w[r], %w[r], %w[in6]\n\t"
        "crc32cw %w[r], %w[r], %w[in7]\n\t"
        "crc32cx %w[r], %w[r], %x[in8]\n\t"
        "crc32cx %w[r], %w[r], %x[in9]\n\t"
        : [r] "+r"(r)
        : [in2] "r"(in2), [in3] "r"(in3), [in4] "r"(in4), [in5] "r"(in5),
          [in6] "r"(in6), [in7] "r"(in7), [in8] "r"(in8), [in9] "r"(in9)
    );
    if ((r & 0xFFFFFFFF) != (golden & 0xFFFFFFFF)) {
        pr_err("[%d] CRC32C: got=0x%llx, expect=0x%llx\n", __LINE__, r & 0xFFFFFFFF, golden & 0xFFFFFFFF);
        return -1;
    }
    return 0;
}

#define TEST_CRC32(init, in2, in3, in4, in5, in6, in7, in8, in9, golden) \
    do { \
        if (test_crc32(init, in2, in3, in4, in5, in6, in7, in8, in9, golden) != 0) { \
            return -1; \
        } \
    } while (0)

#define TEST_CRC32C(init, in2, in3, in4, in5, in6, in7, in8, in9, golden) \
    do { \
        if (test_crc32c(init, in2, in3, in4, in5, in6, in7, in8, in9, golden) != 0) { \
            return -1; \
        } \
    } while (0)

static int a55_stl_core_diagnose_p011_n001_c_impl(void)
{
	// Basic Module 44
	// CRC32B, CRC32H, CRC32W, CRC32X test
    TEST_CRC32(
        A55_STL_BM44_INPUT_VALUE_1,
        A55_STL_BM44_INPUT_VALUE_3,
        A55_STL_BM44_INPUT_VALUE_4,
        A55_STL_BM44_INPUT_VALUE_5,
        A55_STL_BM44_INPUT_VALUE_6,
        A55_STL_BM44_INPUT_VALUE_7,
        A55_STL_BM44_INPUT_VALUE_8,
        A55_STL_BM44_INPUT_VALUE_9,
        A55_STL_BM44_INPUT_VALUE_10,
        A55_STL_BM44_GOLDEN_VALUE_1
    );
    TEST_CRC32(
        A55_STL_BM44_INPUT_VALUE_2,
        A55_STL_BM44_INPUT_VALUE_3,
        A55_STL_BM44_INPUT_VALUE_4,
        A55_STL_BM44_INPUT_VALUE_5,
        A55_STL_BM44_INPUT_VALUE_6,
        A55_STL_BM44_INPUT_VALUE_7,
        A55_STL_BM44_INPUT_VALUE_8,
        A55_STL_BM44_INPUT_VALUE_9,
        A55_STL_BM44_INPUT_VALUE_10,
        A55_STL_BM44_GOLDEN_VALUE_2
    );

	// Basic Module 45
	// CRC32CB, CRC32CH, CRC32CW, CRC32CX tests
    TEST_CRC32C(
        A55_STL_BM45_INPUT_VALUE_1,
        A55_STL_BM45_INPUT_VALUE_3,
        A55_STL_BM45_INPUT_VALUE_4,
        A55_STL_BM45_INPUT_VALUE_5,
        A55_STL_BM45_INPUT_VALUE_6,
        A55_STL_BM45_INPUT_VALUE_7,
        A55_STL_BM45_INPUT_VALUE_8,
        A55_STL_BM45_INPUT_VALUE_9,
        A55_STL_BM45_INPUT_VALUE_10,
        A55_STL_BM45_GOLDEN_VALUE_1
    );
    TEST_CRC32C(
        A55_STL_BM45_INPUT_VALUE_2,
        A55_STL_BM45_INPUT_VALUE_3,
        A55_STL_BM45_INPUT_VALUE_4,
        A55_STL_BM45_INPUT_VALUE_5,
        A55_STL_BM45_INPUT_VALUE_6,
        A55_STL_BM45_INPUT_VALUE_7,
        A55_STL_BM45_INPUT_VALUE_8,
        A55_STL_BM45_INPUT_VALUE_9,
        A55_STL_BM45_INPUT_VALUE_10,
        A55_STL_BM45_GOLDEN_VALUE_2
    );

    return 0;
}

void a55_stl_core_diagnose_p011(a55_stl_state_t *fctlr_base)
{
    if (a55_stl_core_diagnose_p011_n001_c_impl() != 0) {
        pr_info("A55 STL Core P011: Test failed\n");
    } else {
        pr_info("A55 STL Core P011: Test passed\n");
    }
}
