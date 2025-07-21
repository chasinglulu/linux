/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of divide instructions (SDIV, UDIV)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P013: " fmt

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

// SDIV 测试宏
#define TEST_SDIV(input1, input2, golden) \
    do { \
        uint64_t result; \
        asm volatile("sdiv %[res], %[in1], %[in2]\n" \
            : [res] "=r"(result) \
            : [in1] "r"(input1), [in2] "r"(input2)); \
        if (result != golden) { \
            pr_err("[%d] SDIV: in1=0x%llx, in2=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)input1, (uint64_t)input2, (uint64_t)result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// UDIV 测试宏
#define TEST_UDIV(input1, input2, golden) \
    do { \
        uint64_t result; \
        asm volatile("udiv %[res], %[in1], %[in2]\n" \
            : [res] "=r"(result) \
            : [in1] "r"(input1), [in2] "r"(input2)); \
        if (result != golden) { \
            pr_err("[%d] UDIV: in1=0x%llx, in2=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)input1, (uint64_t)input2, (uint64_t)result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

int a55_stl_core_diagnose_p013_n001_c_impl(void) {
    // Basic Module 01: SDIV
    TEST_SDIV(A55_STL_BM01_INPUT_VALUE_1, A55_STL_BM01_INPUT_VALUE_2, A55_STL_BM01_GOLDEN_VALUE_1);
    TEST_SDIV(A55_STL_BM01_INPUT_VALUE_3, A55_STL_BM01_INPUT_VALUE_4, A55_STL_BM01_GOLDEN_VALUE_3);
    TEST_SDIV(A55_STL_BM01_INPUT_VALUE_5, A55_STL_BM01_INPUT_VALUE_6, A55_STL_BM01_GOLDEN_VALUE_3);
    TEST_SDIV(A55_STL_BM01_INPUT_VALUE_7, A55_STL_BM01_INPUT_VALUE_8, A55_STL_BM01_GOLDEN_VALUE_4);

    // Basic Module 02: UDIV
    TEST_UDIV(A55_STL_BM02_INPUT_VALUE_1, A55_STL_BM02_INPUT_VALUE_2, A55_STL_BM02_GOLDEN_VALUE_1);
    TEST_UDIV(A55_STL_BM02_INPUT_VALUE_2, A55_STL_BM02_INPUT_VALUE_1, A55_STL_BM02_GOLDEN_VALUE_2);
    TEST_UDIV(A55_STL_BM02_INPUT_VALUE_3, A55_STL_BM02_INPUT_VALUE_4, A55_STL_BM02_GOLDEN_VALUE_3);
    TEST_UDIV(A55_STL_BM02_INPUT_VALUE_5, A55_STL_BM02_INPUT_VALUE_6, A55_STL_BM02_GOLDEN_VALUE_4);

    return 0;
}

void a55_stl_core_diagnose_p013(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p013_n001_c_impl() != 0) {
        pr_info("A55 STL Core P013: Test failed\n");
    } else {
        pr_info("A55 STL Core P013: Test passed\n");
    }
}
