/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of bit operation instructions: CLS, CLZ, RBIT, REV, REV16, REV32
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P008: " fmt

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

// CLS
#define TEST_CLS(in, golden) \
    do { \
        uint64_t result; \
        asm volatile("cls %[res], %[a]\n" \
            : [res] "=r"(result) \
            : [a] "r"(in)); \
        if (result != golden) { \
            pr_err("[%d] CLS: in=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// CLZ
#define TEST_CLZ(in, golden) \
    do { \
        uint64_t result; \
        asm volatile("clz %[res], %[a]\n" \
            : [res] "=r"(result) \
            : [a] "r"(in)); \
        if (result != golden) { \
            pr_err("[%d] CLZ: in=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// RBIT
#define TEST_RBIT(in, golden) \
    do { \
        uint64_t result; \
        asm volatile("rbit %[res], %[a]\n" \
            : [res] "=r"(result) \
            : [a] "r"(in)); \
        if (result != golden) { \
            pr_err("[%d] RBIT: in=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// REV
#define TEST_REV(in, golden) \
    do { \
        uint64_t result; \
        asm volatile("rev %[res], %[a]\n" \
            : [res] "=r"(result) \
            : [a] "r"(in)); \
        if (result != golden) { \
            pr_err("[%d] REV: in=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// REV16
#define TEST_REV16(in, golden) \
    do { \
        uint64_t result; \
        asm volatile("rev16 %[res], %[a]\n" \
            : [res] "=r"(result) \
            : [a] "r"(in)); \
        if (result != golden) { \
            pr_err("[%d] REV16: in=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// REV32
#define TEST_REV32(in, golden) \
    do { \
        uint64_t result; \
        asm volatile("rev32 %[res], %[a]\n" \
            : [res] "=r"(result) \
            : [a] "r"(in)); \
        if (result != golden) { \
            pr_err("[%d] REV32: in=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

int a55_stl_core_diagnose_p008_n001_c_impl(void) {
    // CLS
    TEST_CLS(A55_STL_BM52_INPUT_VALUE_1, A55_STL_BM52_GOLDEN_VALUE_1);
    TEST_CLS(A55_STL_BM52_INPUT_VALUE_2, A55_STL_BM52_GOLDEN_VALUE_2);
    TEST_CLS(A55_STL_BM52_INPUT_VALUE_3, A55_STL_BM52_GOLDEN_VALUE_3);
    TEST_CLS(A55_STL_BM52_INPUT_VALUE_4, A55_STL_BM52_GOLDEN_VALUE_4);

    // CLZ
    TEST_CLZ(A55_STL_BM53_INPUT_VALUE_1, A55_STL_BM53_GOLDEN_VALUE_1);
    TEST_CLZ(A55_STL_BM53_INPUT_VALUE_2, A55_STL_BM53_GOLDEN_VALUE_2);
    TEST_CLZ(A55_STL_BM53_INPUT_VALUE_3, A55_STL_BM53_GOLDEN_VALUE_3);
    TEST_CLZ(A55_STL_BM53_INPUT_VALUE_4, A55_STL_BM53_GOLDEN_VALUE_4);

    // RBIT
    TEST_RBIT(A55_STL_BM54_INPUT_VALUE_1, A55_STL_BM54_GOLDEN_VALUE_1);
    TEST_RBIT(A55_STL_BM54_INPUT_VALUE_2, A55_STL_BM54_GOLDEN_VALUE_2);
    TEST_RBIT(A55_STL_BM54_INPUT_VALUE_3, A55_STL_BM54_GOLDEN_VALUE_3);
    TEST_RBIT(A55_STL_BM54_INPUT_VALUE_4, A55_STL_BM54_GOLDEN_VALUE_4);

    // REV
    TEST_REV(A55_STL_BM55_INPUT_VALUE_1, A55_STL_BM55_GOLDEN_VALUE_1);
    TEST_REV(A55_STL_BM55_INPUT_VALUE_2, A55_STL_BM55_GOLDEN_VALUE_2);
    // REV64 alias
    TEST_REV(A55_STL_BM55_INPUT_VALUE_3, A55_STL_BM55_GOLDEN_VALUE_3);
    TEST_REV(A55_STL_BM55_INPUT_VALUE_4, A55_STL_BM55_GOLDEN_VALUE_4);

    // REV16
    TEST_REV16(A55_STL_BM56_INPUT_VALUE_1, A55_STL_BM56_GOLDEN_VALUE_1);
    TEST_REV16(A55_STL_BM56_INPUT_VALUE_2, A55_STL_BM56_GOLDEN_VALUE_2);
    TEST_REV16(A55_STL_BM56_INPUT_VALUE_3, A55_STL_BM56_GOLDEN_VALUE_3);
    TEST_REV16(A55_STL_BM56_INPUT_VALUE_4, A55_STL_BM56_GOLDEN_VALUE_4);

    // REV32
    TEST_REV32(A55_STL_BM57_INPUT_VALUE_1, A55_STL_BM57_GOLDEN_VALUE_1);
    TEST_REV32(A55_STL_BM57_INPUT_VALUE_2, A55_STL_BM57_GOLDEN_VALUE_2);
    TEST_REV32(A55_STL_BM57_INPUT_VALUE_3, A55_STL_BM57_GOLDEN_VALUE_3);
    TEST_REV32(A55_STL_BM57_INPUT_VALUE_4, A55_STL_BM57_GOLDEN_VALUE_4);

    return 0;
}

void a55_stl_core_diagnose_p008(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p008_n001_c_impl() != 0) {
        pr_info("A55 STL Core P008: Test failed\n");
    } else {
        pr_info("A55 STL Core P008: Test passed\n");
    }
}
