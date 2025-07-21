/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of shift instructions ASRV, LSLV, LSRV, RORV
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P010: " fmt

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>

#include "a55_stl_constants.h"
#include "a55_stl_utils.h"
#include "a55_stl.h"

// ASRV
#define TEST_ASRV(xn, xm, golden) \
    do { \
        uint64_t result; \
        asm volatile("asrv %0, %1, %2" : "=r"(result) : "r"(xn), "r"(xm)); \
        if (result != (golden)) { \
            pr_err("[%d] ASRV: xn=0x%llx, xm=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(xn), (uint64_t)(xm), result, (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

// LSLV
#define TEST_LSLV(xn, xm, golden) \
    do { \
        uint64_t result; \
        asm volatile("lslv %0, %1, %2" : "=r"(result) : "r"(xn), "r"(xm)); \
        if (result != (golden)) { \
            pr_err("[%d] LSLV: xn=0x%llx, xm=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(xn), (uint64_t)(xm), result, (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

// LSRV
#define TEST_LSRV(xn, xm, golden) \
    do { \
        uint64_t result; \
        asm volatile("lsrv %0, %1, %2" : "=r"(result) : "r"(xn), "r"(xm)); \
        if (result != (golden)) { \
            pr_err("[%d] LSRV: xn=0x%llx, xm=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(xn), (uint64_t)(xm), result, (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

// RORV
#define TEST_RORV(xn, xm, golden) \
    do { \
        uint64_t result; \
        asm volatile("rorv %0, %1, %2" : "=r"(result) : "r"(xn), "r"(xm)); \
        if (result != (golden)) { \
            pr_err("[%d] RORV: xn=0x%llx, xm=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(xn), (uint64_t)(xm), result, (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

static int a55_stl_core_diagnose_p010_n001_c_impl(void)
{
    // BM40: ASRV
    TEST_ASRV(A55_STL_BM40_INPUT_VALUE_1, A55_STL_BM40_INPUT_VALUE_5, A55_STL_BM40_GOLDEN_VALUE_1);
    TEST_ASRV(A55_STL_BM40_INPUT_VALUE_2, A55_STL_BM40_INPUT_VALUE_6, A55_STL_BM40_GOLDEN_VALUE_2);
    TEST_ASRV(A55_STL_BM40_INPUT_VALUE_3, A55_STL_BM40_INPUT_VALUE_7, A55_STL_BM40_GOLDEN_VALUE_3);
    TEST_ASRV(A55_STL_BM40_INPUT_VALUE_4, A55_STL_BM40_INPUT_VALUE_8, A55_STL_BM40_GOLDEN_VALUE_4);

    // BM41: LSLV
    TEST_LSLV(A55_STL_BM41_INPUT_VALUE_1, A55_STL_BM41_INPUT_VALUE_2, A55_STL_BM41_GOLDEN_VALUE_1);
    TEST_LSLV(A55_STL_BM41_INPUT_VALUE_3, A55_STL_BM41_INPUT_VALUE_4, A55_STL_BM41_GOLDEN_VALUE_2);
    TEST_LSLV(A55_STL_BM41_INPUT_VALUE_5, A55_STL_BM41_INPUT_VALUE_6, A55_STL_BM41_GOLDEN_VALUE_3);
    TEST_LSLV(A55_STL_BM41_INPUT_VALUE_7, A55_STL_BM41_INPUT_VALUE_8, A55_STL_BM41_GOLDEN_VALUE_4);

    // BM42: LSRV
    TEST_LSRV(A55_STL_BM42_INPUT_VALUE_1, A55_STL_BM42_INPUT_VALUE_5, A55_STL_BM42_GOLDEN_VALUE_1);
    TEST_LSRV(A55_STL_BM42_INPUT_VALUE_2, A55_STL_BM42_INPUT_VALUE_6, A55_STL_BM42_GOLDEN_VALUE_2);
    TEST_LSRV(A55_STL_BM42_INPUT_VALUE_3, A55_STL_BM42_INPUT_VALUE_7, A55_STL_BM42_GOLDEN_VALUE_3);
    TEST_LSRV(A55_STL_BM42_INPUT_VALUE_4, A55_STL_BM42_INPUT_VALUE_8, A55_STL_BM42_GOLDEN_VALUE_4);

    // BM43: RORV
    TEST_RORV(A55_STL_BM43_INPUT_VALUE_1, A55_STL_BM43_INPUT_VALUE_5, A55_STL_BM43_GOLDEN_VALUE_1);
    TEST_RORV(A55_STL_BM43_INPUT_VALUE_2, A55_STL_BM43_INPUT_VALUE_6, A55_STL_BM43_GOLDEN_VALUE_2);
    TEST_RORV(A55_STL_BM43_INPUT_VALUE_3, A55_STL_BM43_INPUT_VALUE_7, A55_STL_BM43_GOLDEN_VALUE_3);
    TEST_RORV(A55_STL_BM43_INPUT_VALUE_4, A55_STL_BM43_INPUT_VALUE_8, A55_STL_BM43_GOLDEN_VALUE_4);

    // All tests passed
    return 0;
}

void a55_stl_core_diagnose_p010(a55_stl_state_t *fctlr_base)
{
    if (a55_stl_core_diagnose_p010_n001_c_impl() != 0) {
        pr_info("A55 STL Core P010: Test failed\n");
    } else {
        pr_info("A55 STL Core P010: Test passed\n");
    }
}
