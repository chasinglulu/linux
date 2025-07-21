/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of fast operations strategy (BM167)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P014: " fmt

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

// BM167 测试宏
#define TEST_BM167(input1, input2, golden) \
    do { \
        uint64_t x2 = (input1); \
        uint64_t x11 = (input2); \
        uint64_t x3 = (golden)[0]; \
        uint64_t x4 = (golden)[1]; \
        uint64_t x5 = (golden)[2]; \
        uint64_t x6 = (golden)[3]; \
        uint64_t x7 = (golden)[4]; \
        uint64_t x8 = (golden)[5]; \
        uint64_t x9 = (golden)[6]; \
        uint64_t x10 = (golden)[7]; \
        uint64_t x12, x13, x14, x15, x16, x17, x18, x19; \
        uint64_t x20, x21, x22, x23, x24, x25, x26, x27; \
        int i; \
        for (i = 0; i < 10; ++i) { \
            asm volatile( \
                "orn %[x12], %[x11], %[x2]\n\t" \
                "eor %[x13], %[x12], %[x11]\n\t" \
                "add %[x14], %[x13], %[x2]\n\t" \
                "eor %[x15], %[x14], %[x11]\n\t" \
                "and %[x16], %[x15], %[x2]\n\t" \
                "and %[x17], %[x15], %[x11]\n\t" \
                "orr %[x18], %[x17], %[x16]\n\t" \
                "and %[x19], %[x18], %[x13]\n\t" \
                "orn %[x20], %[x17], %[x16]\n\t" \
                "eor %[x21], %[x20], %[x17]\n\t" \
                "add %[x22], %[x21], %[x16]\n\t" \
                "eor %[x23], %[x22], %[x17]\n\t" \
                "and %[x24], %[x23], %[x16]\n\t" \
                "and %[x25], %[x23], %[x17]\n\t" \
                "orr %[x26], %[x25], %[x24]\n\t" \
                "and %[x27], %[x26], %[x21]\n\t" \
                : [x12] "=&r"(x12), [x13] "=&r"(x13), [x14] "=&r"(x14), [x15] "=&r"(x15), \
                  [x16] "=&r"(x16), [x17] "=&r"(x17), [x18] "=&r"(x18), [x19] "=&r"(x19), \
                  [x20] "=&r"(x20), [x21] "=&r"(x21), [x22] "=&r"(x22), [x23] "=&r"(x23), \
                  [x24] "=&r"(x24), [x25] "=&r"(x25), [x26] "=&r"(x26), [x27] "=&r"(x27) \
                : [x2] "r"(x2), [x11] "r"(x11) \
            ); \
            if (x20 != x5) { pr_err("[%d] BM167 x20 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x20, x5); return -1; } \
            if (x21 != x6) { pr_err("[%d] BM167 x21 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x21, x6); return -1; } \
            if (x22 != x7) { pr_err("[%d] BM167 x22 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x22, x7); return -1; } \
            if (x23 != x8) { pr_err("[%d] BM167 x23 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x23, x8); return -1; } \
            if (x24 != x3) { pr_err("[%d] BM167 x24 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x24, x3); return -1; } \
            if (x25 != x4) { pr_err("[%d] BM167 x25 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x25, x4); return -1; } \
            if (x26 != x9) { pr_err("[%d] BM167 x26 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x26, x9); return -1; } \
            if (x27 != x10) { pr_err("[%d] BM167 x27 fail: got=0x%llx, expect=0x%llx\n", __LINE__, x27, x10); return -1; } \
        } \
    } while (0)

int a55_stl_core_diagnose_p014_n001_c_impl(void) {
    static const uint64_t golden[8] = {
        A55_STL_BM167_GOLDEN_VALUE_1,
        A55_STL_BM167_GOLDEN_VALUE_2,
        A55_STL_BM167_GOLDEN_VALUE_3,
        A55_STL_BM167_GOLDEN_VALUE_4,
        A55_STL_BM167_GOLDEN_VALUE_5,
        A55_STL_BM167_GOLDEN_VALUE_6,
        A55_STL_BM167_GOLDEN_VALUE_7,
        A55_STL_BM167_GOLDEN_VALUE_8
    };
    TEST_BM167(A55_STL_BM167_INPUT_VALUE_1, A55_STL_BM167_INPUT_VALUE_2, golden);
    return 0;
}

void a55_stl_core_diagnose_p014(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p014_n001_c_impl() != 0) {
        pr_info("A55 STL Core P014: Test failed\n");
    } else {
        pr_info("A55 STL Core P014: Test passed\n");
    }
}
