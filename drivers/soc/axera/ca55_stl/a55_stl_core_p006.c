/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of GPR registers
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P006: " fmt

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

static inline int test_gpr_common(uint64_t input, uint64_t golden) {
    uint64_t gpr[32] = {0};
    int fail = 0;

    asm volatile(
        "mov %[x3], %[in]\n"
        "mov %[x4], %[in]\n"
        "mov %[x5], %[in]\n"
        "mov %[x6], %[in]\n"
        "mov %[x7], %[in]\n"
        "mov %[x8], %[in]\n"
        "mov %[x9], %[in]\n"
        "mov %[x10], %[in]\n"
        "mov %[x11], %[in]\n"
        "mov %[x12], %[in]\n"
        "mov %[x13], %[in]\n"
        "mov %[x14], %[in]\n"
        "mov %[x15], %[in]\n"
        "mov %[x16], %[in]\n"
        "mov %[x17], %[in]\n"
        "mov %[x18], %[in]\n"
        "mov %[x19], %[in]\n"
        "mov %[x20], %[in]\n"
        "mov %[x21], %[in]\n"
        "mov %[x22], %[in]\n"
        "mov %[x23], %[in]\n"
        "mov %[x24], %[in]\n"
        "mov %[x25], %[in]\n"
        "mov %[x26], %[in]\n"
        "mov %[x27], %[in]\n"
        "mov %[x28], %[in]\n"
        "mov %[x29], %[in]\n"
        : [x3] "=r"(gpr[3]), [x4] "=r"(gpr[4]), [x5] "=r"(gpr[5]), [x6] "=r"(gpr[6]),
          [x7] "=r"(gpr[7]), [x8] "=r"(gpr[8]), [x9] "=r"(gpr[9]), [x10] "=r"(gpr[10]),
          [x11] "=r"(gpr[11]), [x12] "=r"(gpr[12]), [x13] "=r"(gpr[13]), [x14] "=r"(gpr[14]),
          [x15] "=r"(gpr[15]), [x16] "=r"(gpr[16]), [x17] "=r"(gpr[17]), [x18] "=r"(gpr[18]),
          [x19] "=r"(gpr[19]), [x20] "=r"(gpr[20]), [x21] "=r"(gpr[21]), [x22] "=r"(gpr[22]),
          [x23] "=r"(gpr[23]), [x24] "=r"(gpr[24]), [x25] "=r"(gpr[25]), [x26] "=r"(gpr[26]),
          [x27] "=r"(gpr[27]), [x28] "=r"(gpr[28]), [x29] "=r"(gpr[29])
        : [in] "r"(input)
    );
    for (int i = 3; i <= 29; ++i) {
        if (gpr[i] != golden) {
            pr_err("GPR[%d] test failed: got=0x%llx, expect=0x%llx\n", i, gpr[i], golden);
            fail = -1;
        }
    }
    return fail;
}

#define TEST_GPR(input, golden) \
    do { \
        if (test_gpr_common((input), (golden)) != 0) return -1; \
    } while (0)

int a55_stl_core_diagnose_p006_n001_c_impl(void) {
    TEST_GPR(A55_STL_BM138_INPUT_VALUE_1, A55_STL_BM138_GOLDEN_VALUE_1);
    TEST_GPR(A55_STL_BM138_INPUT_VALUE_2, A55_STL_BM138_GOLDEN_VALUE_2);
    return 0;
}

void a55_stl_core_diagnose_p006(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p006_n001_c_impl() != 0) {
        pr_info("A55 STL Core P006: Test failed\n");
    } else {
        pr_info("A55 STL Core P006: Test passed\n");
    }
}
