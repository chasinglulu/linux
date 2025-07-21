/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of multiplication instructions (MUL, MADD, MSUB, SMADDL, SMSUBL, SMULH, UMADDL, UMSUBL, UMULH)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P004: " fmt

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

// MUL
#define TEST_MUL(input1, input2, golden) \
    do { \
        uint64_t result; \
        asm volatile("mul %[res], %[in1], %[in2]\n" \
            : [res] "=r"(result) \
            : [in1] "r"(input1), [in2] "r"(input2)); \
        if (result != golden) { \
            pr_err("[%d] MUL: in1=0x%llx, in2=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)input1, (uint64_t)input2, (uint64_t)result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// SMSUBL
#define TEST_SMSUBL(in1, in2, in3, in4, in5, in6, in7, in8, golden) \
    do { \
        if (test_smsubl((in1), (in2), (in3), (in4), (in5), (in6), (in7), (in8), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_smsubl(
    uint64_t in1, uint64_t in2, uint64_t in3,
    uint64_t in4, uint64_t in5, uint64_t in6,
    uint64_t in7, uint64_t in8, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
         "mov %w[r], %w[in1]\n\t"
        // 1st: smsubl res, w(res), w[in8], in7
        "smsubl %[r], %w[r], %w[in8], %[in7]\n\t"
        // 2nd: smsubl res, w(res), w[in3], in4
        "smsubl %[r], %w[r], %w[in3], %[in4]\n\t"
        // 3rd: smsubl res, w(res), w[in4], in3
        "smsubl %[r], %w[r], %w[in4], %[in3]\n\t"
        // 4th: smsubl res, w(res), w[in5], xzr
        "smsubl %[r], %w[r], %w[in5], xzr\n\t"
        // 5th: smsubl res, w(res), w[in6], xzr
        "smsubl %[r], %w[r], %w[in6], xzr\n\t"
        : [r] "+r"(res)
        : [in1] "r"(in1), [in2] "r"(in2), [in3] "r"(in3),
          [in4] "r"(in4), [in5] "r"(in5),
          [in6] "r"(in6), [in7] "r"(in7), [in8] "r"(in8)
        );
    if (res != golden) {
        pr_err("[%d] SMSUBL: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

// SMULH
#define TEST_SMULH(in1, in2, in3, in4, golden) \
    do { \
        if (test_smulh((in1), (in2), (in3), (in4), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_smulh(
    uint64_t in1, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
        // 1st test with Xm negative
        "smulh %[r], %[r], %[in3]\n\t"
        // 2nd test with Xm positive
        "smulh %[r], %[r], %[in4]\n\t"
        : [r] "+r"(res)
        : [in3] "r"(in3), [in4] "r"(in4)
    );
    if (res != golden) {
        pr_err("[%d] SMULH: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

// UMADDL
#define TEST_UMADDL(in1, in2, in3, in4, in5, in6, in7, in8, golden) \
    do { \
        if (test_umaddl((in1), (in2), (in3), (in4), (in5), (in6), (in7), (in8), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_umaddl(
    uint64_t in1, uint64_t in2, uint64_t in3,
    uint64_t in4, uint64_t in5, uint64_t in6,
    uint64_t in7, uint64_t in8, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
         "mov %w[r], %w[in1]\n\t"
        // 1st: umaddl res, w(res), w[in8], in7
        "umaddl %[r], %w[r], %w[in8], %[in7]\n\t"
        // 2nd: umaddl res, w(res), w[in3], in4
        "umaddl %[r], %w[r], %w[in3], %[in4]\n\t"
        // 3rd: umaddl res, w(res), w[in4], in3
        "umaddl %[r], %w[r], %w[in4], %[in3]\n\t"
        // 4th: umaddl res, w(res), w[in5], xzr
        "umaddl %[r], %w[r], %w[in5], xzr\n\t"
        // 5th: umaddl res, w(res), w[in6], xzr
        "umaddl %[r], %w[r], %w[in6], xzr\n\t"
        : [r] "+r"(res)
        : [in1] "r"(in1), [in2] "r"(in2), [in3] "r"(in3),
          [in4] "r"(in4), [in5] "r"(in5),
          [in6] "r"(in6), [in7] "r"(in7), [in8] "r"(in8)
        );
    if (res != golden) {
        pr_err("[%d] UMADDL: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

// UMSUBL
#define TEST_UMSUBL(in1, in2, in3, in4, in5, in6, in7, in8, golden) \
    do { \
        if (test_umsubl((in1), (in2), (in3), (in4), (in5), (in6), (in7), (in8), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_umsubl(
    uint64_t in1, uint64_t in2, uint64_t in3,
    uint64_t in4, uint64_t in5, uint64_t in6,
    uint64_t in7, uint64_t in8, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
         "mov %w[r], %w[in1]\n\t"
        // 1st: umsubl res, w(res), w[in8], in7
        "umsubl %[r], %w[r], %w[in8], %[in7]\n\t"
        // 2nd: umsubl res, w(res), w[in3], in4
        "umsubl %[r], %w[r], %w[in3], %[in4]\n\t"
        // 3rd: umsubl res, w(res), w[in4], in3
        "umsubl %[r], %w[r], %w[in4], %[in3]\n\t"
        // 4th: umsubl res, w(res), w[in5], xzr
        "umsubl %[r], %w[r], %w[in5], xzr\n\t"
        // 5th: umsubl res, w(res), w[in6], xzr
        "umsubl %[r], %w[r], %w[in6], xzr\n\t"
        : [r] "+r"(res)
        : [in1] "r"(in1), [in2] "r"(in2), [in3] "r"(in3),
          [in4] "r"(in4), [in5] "r"(in5),
          [in6] "r"(in6), [in7] "r"(in7), [in8] "r"(in8)
        );
    if (res != golden) {
        pr_err("[%d] UMSUBL: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

// UMULH
#define TEST_UMULH(in1, in2, in3, in4, golden) \
    do { \
        if (test_umulh((in1), (in2), (in3), (in4), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_umulh(
    uint64_t in1, uint64_t in2, uint64_t in3, uint64_t in4, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
        // 1st test with Xm negative
        "umulh %[r], %[r], %[in3]\n\t"
        // 2nd test with Xm positive
        "umulh %[r], %[r], %[in4]\n\t"
        : [r] "+r"(res)
        : [in3] "r"(in3), [in4] "r"(in4)
    );
    if (res != golden) {
        pr_err("[%d] UMULH: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

#define TEST_MADD(in1, in2, in3, in4, in5, in6, golden) \
    do { \
        if (test_madd((in1), (in2), (in3), (in4), (in5), (in6), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_madd(
    uint64_t in1, uint64_t in2, uint64_t in3,
    uint64_t in4, uint64_t in5, uint64_t in6, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
        "madd %[r], %[r], %[in2], %[in3]\n\t"
        "madd %[r], %[r], %[in4], %[in5]\n\t"
        "madd %[r], %[r], %[in5], %[in4]\n\t"
        : [r] "+r"(res)
        : [in2] "r"(in4), [in3] "r"(in3),
          [in4] "r"(in5), [in5] "r"(in6)
        );
    if (res != golden) {
        pr_err("[%d] MADD: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

#define TEST_MSUB(in1, in2, in3, in4, in5, in6, in7, in8, golden) \
    do { \
        if (test_msub((in1), (in2), (in3), (in4), (in5), (in6), (in7), (in8), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_msub(
    uint64_t in1, uint64_t in2, uint64_t in3,
    uint64_t in4, uint64_t in5, uint64_t in6,
    uint64_t in7, uint64_t in8, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
        // 1st: msub res, res, in4, in3
        "msub %[r], %[r], %[in4], %[in3]\n\t"
        // 2nd: msub res, res, in5, in6
        "msub %[r], %[r], %[in5], %[in6]\n\t"
        // 3rd: msub res, res, in6, in5
        "msub %[r], %[r], %[in6], %[in5]\n\t"
        // 4th: msub res, res, in7, xzr
        "msub %[r], %[r], %[in7], xzr\n\t"
        // 5th: msub res, res, in8, xzr
        "msub %[r], %[r], %[in8], xzr\n\t"
        : [r] "+r"(res)
        : [in3] "r"(in3), [in4] "r"(in4),
          [in5] "r"(in5), [in6] "r"(in6),
          [in7] "r"(in7), [in8] "r"(in8)
        );
    if (res != golden) {
        pr_err("[%d] MSUB: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

#define TEST_SMADDL(in1, in2, in3, in4, in5, in6, in7, in8, golden) \
    do { \
        if (test_smaddl((in1), (in2), (in3), (in4), (in5), (in6), (in7), (in8), (golden)) != 0) \
            return -1; \
    } while (0)

static inline int test_smaddl(
    uint64_t in1, uint64_t in2, uint64_t in3,
    uint64_t in4, uint64_t in5, uint64_t in6,
    uint64_t in7, uint64_t in8, uint64_t golden)
{
    uint64_t res = in1;
    asm volatile(
         "mov %w[r], %w[in1]\n\t"
        // 1st: smaddl res, w(res), w[in8], in7
        "smaddl %[r], %w[r], %w[in8], %[in7]\n\t"
        // 2nd: smaddl res, w(res), w[in3], in4
        "smaddl %[r], %w[r], %w[in3], %[in4]\n\t"
        // 3rd: smaddl res, w(res), w[in4], in3
        "smaddl %[r], %w[r], %w[in4], %[in3]\n\t"
        // 4th: smaddl res, w(res), w[in5], xzr
        "smaddl %[r], %w[r], %w[in5], xzr\n\t"
        // 5th: smaddl res, w(res), w[in6], xzr
        "smaddl %[r], %w[r], %w[in6], xzr\n\t"
        : [r] "+r"(res)
        : [in1] "r"(in1), [in2] "r"(in2), [in3] "r"(in3),
          [in4] "r"(in4), [in5] "r"(in5),
          [in6] "r"(in6), [in7] "r"(in7), [in8] "r"(in8)
        );
    if (res != golden) {
        pr_err("[%d] SMADDL: got=0x%llx, expect=0x%llx\n", __LINE__, res, (uint64_t)golden);
        return -1;
    }
    return 0;
}

int a55_stl_core_diagnose_p004_n001_c_impl(void) {
    // Basic Module 227: MUL
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_1, A55_STL_BM227_INPUT_VALUE_2, A55_STL_BM227_GOLDEN_VALUE_1);
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_2, A55_STL_BM227_INPUT_VALUE_1, A55_STL_BM227_GOLDEN_VALUE_1);
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_3, A55_STL_BM227_INPUT_VALUE_4, A55_STL_BM227_GOLDEN_VALUE_2);
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_4, A55_STL_BM227_INPUT_VALUE_3, A55_STL_BM227_GOLDEN_VALUE_2);
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_5, A55_STL_BM227_INPUT_VALUE_6, A55_STL_BM227_GOLDEN_VALUE_3);
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_6, A55_STL_BM227_INPUT_VALUE_5, A55_STL_BM227_GOLDEN_VALUE_3);
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_7, A55_STL_BM227_INPUT_VALUE_8, A55_STL_BM227_GOLDEN_VALUE_4);
    TEST_MUL(A55_STL_BM227_INPUT_VALUE_8, A55_STL_BM227_INPUT_VALUE_7, A55_STL_BM227_GOLDEN_VALUE_4);

    // Basic Module 301: MADD (chain accumulate)
    TEST_MADD(
        A55_STL_BM301_INPUT_VALUE_1,
        A55_STL_BM301_INPUT_VALUE_2,
        A55_STL_BM301_INPUT_VALUE_3,
        A55_STL_BM301_INPUT_VALUE_4,
        A55_STL_BM301_INPUT_VALUE_5,
        A55_STL_BM301_INPUT_VALUE_6,
        A55_STL_BM301_GOLDEN_VALUE_1
    );
    TEST_MADD(
        A55_STL_BM301_INPUT_VALUE_2,
        A55_STL_BM301_INPUT_VALUE_1,
        A55_STL_BM301_INPUT_VALUE_3,
        A55_STL_BM301_INPUT_VALUE_4,
        A55_STL_BM301_INPUT_VALUE_5,
        A55_STL_BM301_INPUT_VALUE_6,
        A55_STL_BM301_GOLDEN_VALUE_2
    );

    // Basic Module 302: MSUB (chain subtract)
    TEST_MSUB(
        A55_STL_BM302_INPUT_VALUE_1,
        A55_STL_BM302_INPUT_VALUE_2,
        A55_STL_BM302_INPUT_VALUE_3,
        A55_STL_BM302_INPUT_VALUE_4,
        A55_STL_BM302_INPUT_VALUE_5,
        A55_STL_BM302_INPUT_VALUE_6,
        A55_STL_BM302_INPUT_VALUE_7,
        A55_STL_BM302_INPUT_VALUE_8,
        A55_STL_BM302_GOLDEN_VALUE_1
    );
    TEST_MSUB(
        A55_STL_BM302_INPUT_VALUE_2,
        A55_STL_BM302_INPUT_VALUE_1,
        A55_STL_BM302_INPUT_VALUE_3,
        A55_STL_BM302_INPUT_VALUE_4,
        A55_STL_BM302_INPUT_VALUE_5,
        A55_STL_BM302_INPUT_VALUE_6,
        A55_STL_BM302_INPUT_VALUE_7,
        A55_STL_BM302_INPUT_VALUE_8,
        A55_STL_BM302_GOLDEN_VALUE_2
    );

    // Basic Module 303: SMADDL (chain accumulate)
    TEST_SMADDL(
        A55_STL_BM303_INPUT_VALUE_1,
        A55_STL_BM303_INPUT_VALUE_2,
        A55_STL_BM303_INPUT_VALUE_3,
        A55_STL_BM303_INPUT_VALUE_4,
        A55_STL_BM303_INPUT_VALUE_5,
        A55_STL_BM303_INPUT_VALUE_6,
        A55_STL_BM303_INPUT_VALUE_7,
        A55_STL_BM303_INPUT_VALUE_8,
        A55_STL_BM303_GOLDEN_VALUE_1
    );
    TEST_SMADDL(
        A55_STL_BM303_INPUT_VALUE_2,
        A55_STL_BM303_INPUT_VALUE_1,
        A55_STL_BM303_INPUT_VALUE_3,
        A55_STL_BM303_INPUT_VALUE_4,
        A55_STL_BM303_INPUT_VALUE_5,
        A55_STL_BM303_INPUT_VALUE_6,
        A55_STL_BM303_INPUT_VALUE_7,
        A55_STL_BM303_INPUT_VALUE_8,
        A55_STL_BM303_GOLDEN_VALUE_2
    );

    // Basic Module 304: SMSUBL
    TEST_SMSUBL(
        A55_STL_BM304_INPUT_VALUE_1,
        A55_STL_BM304_INPUT_VALUE_2,
        A55_STL_BM304_INPUT_VALUE_3,
        A55_STL_BM304_INPUT_VALUE_4,
        A55_STL_BM304_INPUT_VALUE_5,
        A55_STL_BM304_INPUT_VALUE_6,
        A55_STL_BM304_INPUT_VALUE_7,
        A55_STL_BM304_INPUT_VALUE_8,
        A55_STL_BM304_GOLDEN_VALUE_1
    );
    TEST_SMSUBL(
        A55_STL_BM304_INPUT_VALUE_2,
        A55_STL_BM304_INPUT_VALUE_1,
        A55_STL_BM304_INPUT_VALUE_3,
        A55_STL_BM304_INPUT_VALUE_4,
        A55_STL_BM304_INPUT_VALUE_5,
        A55_STL_BM304_INPUT_VALUE_6,
        A55_STL_BM304_INPUT_VALUE_7,
        A55_STL_BM304_INPUT_VALUE_8,
        A55_STL_BM304_GOLDEN_VALUE_2
    );

    // Basic Module 305: SMULH
    TEST_SMULH(
        A55_STL_BM305_INPUT_VALUE_1,
        A55_STL_BM305_INPUT_VALUE_2,
        A55_STL_BM305_INPUT_VALUE_3,
        A55_STL_BM305_INPUT_VALUE_4,
        A55_STL_BM305_GOLDEN_VALUE_1
    );
    TEST_SMULH(
        A55_STL_BM305_INPUT_VALUE_2,
        A55_STL_BM305_INPUT_VALUE_1,
        A55_STL_BM305_INPUT_VALUE_3,
        A55_STL_BM305_INPUT_VALUE_4,
        A55_STL_BM305_GOLDEN_VALUE_2
    );

    // Basic Module 306: UMADDL
    TEST_UMADDL(
        A55_STL_BM306_INPUT_VALUE_1,
        A55_STL_BM306_INPUT_VALUE_2,
        A55_STL_BM306_INPUT_VALUE_3,
        A55_STL_BM306_INPUT_VALUE_4,
        A55_STL_BM306_INPUT_VALUE_5,
        A55_STL_BM306_INPUT_VALUE_6,
        A55_STL_BM306_INPUT_VALUE_7,
        A55_STL_BM306_INPUT_VALUE_8,
        A55_STL_BM306_GOLDEN_VALUE_1
    );
    TEST_UMADDL(
        A55_STL_BM306_INPUT_VALUE_2,
        A55_STL_BM306_INPUT_VALUE_1,
        A55_STL_BM306_INPUT_VALUE_3,
        A55_STL_BM306_INPUT_VALUE_4,
        A55_STL_BM306_INPUT_VALUE_5,
        A55_STL_BM306_INPUT_VALUE_6,
        A55_STL_BM306_INPUT_VALUE_7,
        A55_STL_BM306_INPUT_VALUE_8,
        A55_STL_BM306_GOLDEN_VALUE_2
    );

    // Basic Module 307: UMSUBL
    TEST_UMSUBL(
        A55_STL_BM307_INPUT_VALUE_1,
        A55_STL_BM307_INPUT_VALUE_2,
        A55_STL_BM307_INPUT_VALUE_3,
        A55_STL_BM307_INPUT_VALUE_4,
        A55_STL_BM307_INPUT_VALUE_5,
        A55_STL_BM307_INPUT_VALUE_6,
        A55_STL_BM307_INPUT_VALUE_7,
        A55_STL_BM307_INPUT_VALUE_8,
        A55_STL_BM307_GOLDEN_VALUE_1
    );
    TEST_UMSUBL(
        A55_STL_BM307_INPUT_VALUE_2,
        A55_STL_BM307_INPUT_VALUE_1,
        A55_STL_BM307_INPUT_VALUE_3,
        A55_STL_BM307_INPUT_VALUE_4,
        A55_STL_BM307_INPUT_VALUE_5,
        A55_STL_BM307_INPUT_VALUE_6,
        A55_STL_BM307_INPUT_VALUE_7,
        A55_STL_BM307_INPUT_VALUE_8,
        A55_STL_BM307_GOLDEN_VALUE_2
    );

    // Basic Module 308: UMULH
    TEST_UMULH(
        A55_STL_BM308_INPUT_VALUE_1,
        A55_STL_BM308_INPUT_VALUE_2,
        A55_STL_BM308_INPUT_VALUE_3,
        A55_STL_BM308_INPUT_VALUE_4,
        A55_STL_BM308_GOLDEN_VALUE_1
    );
    TEST_UMULH(
        A55_STL_BM308_INPUT_VALUE_2,
        A55_STL_BM308_INPUT_VALUE_1,
        A55_STL_BM308_INPUT_VALUE_3,
        A55_STL_BM308_INPUT_VALUE_4,
        A55_STL_BM308_GOLDEN_VALUE_2
    );

    return 0;
}

void a55_stl_core_diagnose_p004(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p004_n001_c_impl() != 0) {
        pr_info("A55 STL Core P004: Test failed\n");
    } else {
        pr_info("A55 STL Core P004: Test passed\n");
    }
}
