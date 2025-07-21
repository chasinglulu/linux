/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of subtraction instructions (immediate, shifted register, extended register)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P009: " fmt

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

// SUB immediate
#define TEST_SUB_IMM(in1, in2, in3, in4, in5, golden) \
    do { \
        uint64_t result = in1; \
        asm volatile( \
            "sub %[res], %[a], %[b]\n\t" \
            "sub %[res], %[res], %[c]\n\t" \
            "sub %[res], %[res], %[d]\n\t" \
            "sub %[res], %[res], %[e]\n" \
            : [res] "+r"(result) \
            : [a] "r"(result), [b] "r"(in2), [c] "r"(in3), [d] "r"(in4), [e] "r"(in5)); \
        if (result != golden) { \
            pr_err("[%d] SUB_IMM: got=0x%llx, expect=0x%llx\n", __LINE__, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// SUBS immediate, check NZCV
#define TEST_SUBS_IMM(in, imm, golden, golden_flags) \
    do { \
        uint64_t result, flags64; \
        asm volatile( \
            "msr nzcv, xzr\n" \
            "subs %[res], %[a], %c[imm_val]\n" \
            "mrs %[flg], nzcv\n" \
            : [res] "=&r"(result), [flg] "=&r"(flags64) \
            : [a] "r"(in), [imm_val] "i"(imm) \
            : "cc"); \
        uint32_t flags = (uint32_t)flags64; \
        if ((result != golden) && (flags != (uint32_t)(golden_flags))) { \
            pr_err("[%d] SUBS_IMM: in=0x%llx, imm=0x%llx, got=0x%llx/0x%08x, expect=0x%llx/0x%08x\n", \
                __LINE__, (uint64_t)in, (uint64_t)imm, result, flags, (uint64_t)golden, (uint32_t)(golden_flags)); \
            return -1; \
        } \
    } while (0)

// SUB register
#define TEST_SUB_REG(in1, in2, golden) \
    do { \
        uint64_t result; \
        asm volatile("sub %[res], %[a], %[b]\n" \
            : [res] "=r"(result) \
            : [a] "r"(in1), [b] "r"(in2)); \
        if (result != golden) { \
            pr_err("[%d] SUB_REG: in1=0x%llx, in2=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// SUB shifted register
#define TEST_SUB_SHIFT(in1, in2, shift_type, shift_amt, golden) \
    do { \
        uint64_t result; \
        if (shift_type == 0) { \
            asm volatile("sub %[res], %[a], %[b], asr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 1) { \
            asm volatile("sub %[res], %[a], %[b], lsr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 2) { \
            asm volatile("sub %[res], %[a], %[b], lsl %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 3) { \
            asm volatile("sub %[res], %[a], %[b], ror %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } \
        if (result != golden) { \
            pr_err("[%d] SUB_SHIFT: in1=0x%llx, in2=0x%llx, type=%d, amt=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, shift_type, shift_amt, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// SUB extended register
#define TEST_SUB_EXTEND(in1, in2, ext_type, ext_amt, golden) \
    do { \
        uint64_t result; \
        if (ext_type == 0) { \
            asm volatile("sub %[res], %[a], %[b], uxtb %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(ext_amt)); \
        } else if (ext_type == 1) { \
            asm volatile("sub %[res], %[a], %[b], uxth %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(ext_amt)); \
        } else if (ext_type == 2) { \
            asm volatile("sub %[res], %[a], %[b], uxtw %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(ext_amt)); \
        } else if (ext_type == 3) { \
            asm volatile("sub %[res], %[a], %[b], uxtx %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(ext_amt)); \
        } \
        if (result != golden) { \
            pr_err("[%d] SUB_EXTEND: in1=0x%llx, in2=0x%llx, ext_type=%d, amt=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, ext_type, ext_amt, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

static inline int test_bm23_case1(uint64_t flags, uint64_t golden)
{
    uint64_t x15 = A55_STL_BM23_INPUT_VALUE_3;
    uint64_t x2  = A55_STL_BM23_INPUT_VALUE_3;
    uint64_t x3  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x4  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x5  = A55_STL_BM23_INPUT_VALUE_1;
    uint64_t x6  = A55_STL_BM23_INPUT_VALUE_2;
    uint64_t x7  = A55_STL_BM23_INPUT_VALUE_3;
    uint64_t x8  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x9  = A55_STL_BM23_INPUT_VALUE_1;
    uint64_t x10 = A55_STL_BM23_INPUT_VALUE_2;

    uint64_t x17, x18, x19, x20, x21, x22, x23;

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x18], %[x15], %[x3]\n\t"
        "sub %[x19], %[x2], %[x4]\n\t"
        "subs %[x20], %[x5], %[x6]\n\t"
        "mrs %[x17], nzcv\n"
        : [x18] "=&r"(x18), [x19] "=&r"(x19), [x20] "=&r"(x20), [x17] "=&r"(x17)
        : [x15] "r"(x15), [x3] "r"(x3), [x2] "r"(x2), [x4] "r"(x4), [x5] "r"(x5), [x6] "r"(x6)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE1: NZCV1 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x21], %[x7], %[x8]\n\t"
        "subs %[x22], %[x9], %[x10]\n\t"
        "mov %[x23], %[x21]\n\t"
        "mrs %[x17], nzcv\n"
        : [x21] "=&r"(x21), [x22] "=&r"(x22), [x23] "=&r"(x23), [x17] "=&r"(x17)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE1: NZCV2 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    if (x20 != golden) {
        pr_err("[%d] BM23_CASE1: result1 got=0x%llx, expect=0x%llx\n", __LINE__, x20, golden);
        return -1;
    }
    if (x22 != golden) {
        pr_err("[%d] BM23_CASE1: result2 got=0x%llx, expect=0x%llx\n", __LINE__, x22, golden);
        return -1;
    }
    return 0;
}

#define TEST_SUBS_REG1(flags, golden) \
	do { \
		if (test_bm23_case1(flags, golden) < 0) { \
			return -1; \
		} \
	} while (0)

static inline int test_bm23_case2(uint64_t flags, uint64_t golden)
{
    uint64_t x15 = A55_STL_BM23_INPUT_VALUE_7;
    uint64_t x2  = A55_STL_BM23_INPUT_VALUE_7;
    uint64_t x3  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x4  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x5  = A55_STL_BM23_INPUT_VALUE_5;
    uint64_t x6  = A55_STL_BM23_INPUT_VALUE_5;
    uint64_t x7  = A55_STL_BM23_INPUT_VALUE_7;
    uint64_t x8  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x9  = A55_STL_BM23_INPUT_VALUE_5;
    uint64_t x10 = A55_STL_BM23_INPUT_VALUE_5;

    uint64_t x17, x18, x19, x20, x21, x22, x23;

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x18], %[x15], %[x3]\n\t"
        "sub %[x19], %[x2], %[x4]\n\t"
        "subs %[x20], %[x5], %[x6]\n\t"
        "mrs %[x17], nzcv\n"
        : [x18] "=&r"(x18), [x19] "=&r"(x19), [x20] "=&r"(x20), [x17] "=&r"(x17)
        : [x15] "r"(x15), [x3] "r"(x3), [x2] "r"(x2), [x4] "r"(x4), [x5] "r"(x5), [x6] "r"(x6)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE2: NZCV1 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x21], %[x7], %[x8]\n\t"
        "subs %[x22], %[x9], %[x10]\n\t"
        "mov %[x23], %[x21]\n\t"
        "mrs %[x17], nzcv\n"
        : [x21] "=&r"(x21), [x22] "=&r"(x22), [x23] "=&r"(x23), [x17] "=&r"(x17)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE2: NZCV2 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    if (x20 != golden) {
        pr_err("[%d] BM23_CASE2: result1 got=0x%llx, expect=0x%llx\n", __LINE__, x20, golden);
        return -1;
    }
    if (x22 != golden) {
        pr_err("[%d] BM23_CASE2: result2 got=0x%llx, expect=0x%llx\n", __LINE__, x22, golden);
        return -1;
    }
    return 0;
}

#define TEST_SUBS_REG2(flags, golden) \
    do { \
        if (test_bm23_case2((flags), (golden)) < 0) { \
            return -1; \
        } \
    } while (0)

// Test N = 0, Z = 1, C = 1, V = 0 with CMP alias
static inline int test_bm23_case3(uint64_t imm1, uint64_t imm2, uint64_t flags)
{
    uint64_t x15 = A55_STL_BM23_INPUT_VALUE_8;
    uint64_t x2  = A55_STL_BM23_INPUT_VALUE_8;
    uint64_t x3  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x4  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x5  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x6  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x7  = A55_STL_BM23_INPUT_VALUE_8;
    uint64_t x8  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x9  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x10 = A55_STL_BM23_INPUT_VALUE_4;

    uint64_t x17, x18, x19, x21, x23;

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x18], %[x15], %[x3]\n\t"
        "sub %[x19], %[x2], %[x4]\n\t"
        "subs xzr, %[x5], %[x6], lsl %[imm_val]\n\t"
        "mrs %[x17], nzcv\n"
        : [x18] "=&r"(x18), [x19] "=&r"(x19), [x17] "=&r"(x17)
        : [x15] "r"(x15), [x3] "r"(x3), [x2] "r"(x2), [x4] "r"(x4),
          [x5] "r"(x5), [x6] "r"(x6), [imm_val] "I"(imm1)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE3: NZCV1 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x21], %[x7], %[x8]\n\t"
        "subs xzr, %[x9], %[x10], lsl %[imm_val]\n\t"
        "mov %[x23], %[x21]\n\t"
        "mrs %[x17], nzcv\n"
        : [x21] "=&r"(x21), [x23] "=&r"(x23), [x17] "=&r"(x17)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10),
          [imm_val] "I"(imm2)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE3: NZCV2 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    return 0;
}

#define TEST_SUBS_REG3(imm1, imm2, flags) \
    do { \
        if (test_bm23_case3((imm1), (imm2), (flags)) < 0) { \
            return -1; \
        } \
    } while (0)

// Test N = 1, Z = 0, C = 0, V = 0 with NEGS alias
static inline int test_bm23_case4(uint64_t imm, uint64_t flags, uint64_t golden)
{
    uint64_t x15 = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x2  = A55_STL_BM23_INPUT_VALUE_4;
    uint64_t x3  = A55_STL_BM23_INPUT_VALUE_9;
    uint64_t x4  = A55_STL_BM23_INPUT_VALUE_9;
    uint64_t x6  = A55_STL_BM23_INPUT_VALUE_9;
    uint64_t x7  = A55_STL_BM23_INPUT_VALUE_9;
    uint64_t x8  = A55_STL_BM23_INPUT_VALUE_9;
    uint64_t x10 = A55_STL_BM23_INPUT_VALUE_9;

    uint64_t x17, x18, x19, x20, x21, x22, x23;

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x18], %[x15], %[x3]\n\t"
        "sub %[x19], %[x2], %[x4]\n\t"
        "subs %[x20], xzr, %[x6], lsr %[imm_val]\n\t"
        "mrs %[x17], nzcv\n"
        : [x18] "=&r"(x18), [x19] "=&r"(x19), [x20] "=&r"(x20), [x17] "=&r"(x17)
        : [x15] "r"(x15), [x3] "r"(x3), [x2] "r"(x2), [x4] "r"(x4),
          [x6] "r"(x6), [imm_val] "I"(imm)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE4: NZCV1 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x21], %[x7], %[x8]\n\t"
        "subs %[x22], xzr, %[x10], lsr %[imm_val]\n\t"
        "mov %[x23], %[x21]\n\t"
        "mrs %[x17], nzcv\n"
        : [x21] "=&r"(x21), [x22] "=&r"(x22), [x23] "=&r"(x23), [x17] "=&r"(x17)
        : [x7] "r"(x7), [x8] "r"(x8), [x10] "r"(x10), [imm_val] "I"(imm)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM23_CASE4: NZCV2 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    if (x20 != golden) {
        pr_err("[%d] BM23_CASE4: result1 got=0x%llx, expect=0x%llx\n", __LINE__, x20, golden);
        return -1;
    }
    if (x22 != golden) {
        pr_err("[%d] BM23_CASE4: result2 got=0x%llx, expect=0x%llx\n", __LINE__, x22, golden);
        return -1;
    }
    return 0;
}

#define TEST_SUBS_REG4(imm, flags, golden) \
    do { \
        if (test_bm23_case4((imm), (flags), (golden)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_bm27_case1(uint64_t flags, uint64_t golden)
{
    uint64_t x15 = A55_STL_BM27_INPUT_VALUE_3;
    uint64_t x2  = A55_STL_BM27_INPUT_VALUE_3;
    uint64_t x3  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x4  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x5  = A55_STL_BM27_INPUT_VALUE_1;
    uint64_t x6  = A55_STL_BM27_INPUT_VALUE_2;
    uint64_t x7  = A55_STL_BM27_INPUT_VALUE_3;
    uint64_t x8  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x9  = A55_STL_BM27_INPUT_VALUE_1;
    uint64_t x10 = A55_STL_BM27_INPUT_VALUE_2;

    uint64_t x17, x18, x19, x20, x21, x22, x23;

    // 第一组
    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x18], %[x15], %[x3], uxtx %[imm]\n\t"
        "sub %[x19], %[x2], %[x4], uxtx %[imm]\n\t"
        "subs %[x20], %[x5], %[x6], uxtx %[imm]\n\t"
        "mrs %[x17], nzcv\n"
        : [x18] "=&r"(x18), [x19] "=&r"(x19), [x20] "=&r"(x20), [x17] "=&r"(x17)
        : [x15] "r"(x15), [x3] "r"(x3), [x2] "r"(x2), [x4] "r"(x4),
          [x5] "r"(x5), [x6] "r"(x6), [imm] "I"(A55_STL_BM27_IMM_VALUE_1)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM27_CASE1: NZCV1 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    // 第二组
    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x21], %[x7], %[x8], uxtx %[imm]\n\t"
        "subs %[x22], %[x9], %[x10], uxtx %[imm]\n\t"
        "mov %[x23], %[x21]\n\t"
        "mrs %[x17], nzcv\n"
        : [x21] "=&r"(x21), [x22] "=&r"(x22), [x23] "=&r"(x23), [x17] "=&r"(x17)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10),
          [imm] "I"(A55_STL_BM27_IMM_VALUE_1)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM27_CASE1: NZCV2 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    if (x20 != golden) {
        pr_err("[%d] BM27_CASE1: result1 got=0x%llx, expect=0x%llx\n", __LINE__, x20, golden);
        return -1;
    }
    if (x22 != golden) {
        pr_err("[%d] BM27_CASE1: result2 got=0x%llx, expect=0x%llx\n", __LINE__, x22, golden);
        return -1;
    }
    return 0;
}

#define TEST_SUBS_EXTEND1(flags, golden) \
    do { \
        if (test_bm27_case1((flags), (golden)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_bm27_case2(uint64_t flags, uint64_t golden)
{
    uint64_t x15 = A55_STL_BM27_INPUT_VALUE_5;
    uint64_t x2  = A55_STL_BM27_INPUT_VALUE_5;
    uint64_t x3  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x4  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x5  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x6  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x7  = A55_STL_BM27_INPUT_VALUE_5;
    uint64_t x8  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x9  = A55_STL_BM27_INPUT_VALUE_4;
    uint64_t x10 = A55_STL_BM27_INPUT_VALUE_4;

    uint64_t x17, x18, x19, x20, x21, x22, x23;

    // 第一组
    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x18], %[x15], %w[x3], uxth %[imm]\n\t"
        "sub %[x19], %[x2], %w[x4], uxth %[imm]\n\t"
        "subs %[x20], %[x5], %w[x6], uxth %[imm]\n\t"
        "mrs %[x17], nzcv\n"
        : [x18] "=&r"(x18), [x19] "=&r"(x19), [x20] "=&r"(x20), [x17] "=&r"(x17)
        : [x15] "r"(x15), [x3] "r"(x3), [x2] "r"(x2), [x4] "r"(x4),
          [x5] "r"(x5), [x6] "r"(x6), [imm] "I"(A55_STL_BM27_IMM_VALUE_1)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM27_CASE2: NZCV1 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    // 第二组
    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x21], %[x7], %w[x8], uxth %[imm]\n\t"
        "subs %[x22], %[x9], %w[x10], uxth %[imm]\n\t"
        "mov %[x23], %[x21]\n\t"
        "mrs %[x17], nzcv\n"
        : [x21] "=&r"(x21), [x22] "=&r"(x22), [x23] "=&r"(x23), [x17] "=&r"(x17)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10),
          [imm] "I"(A55_STL_BM27_IMM_VALUE_1)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM27_CASE2: NZCV2 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    if (x20 != golden) {
        pr_err("[%d] BM27_CASE2: result1 got=0x%llx, expect=0x%llx\n", __LINE__, x20, golden);
        return -1;
    }
    if (x22 != golden) {
        pr_err("[%d] BM27_CASE2: result2 got=0x%llx, expect=0x%llx\n", __LINE__, x22, golden);
        return -1;
    }
    return 0;
}

#define TEST_SUBS_EXTEND2(flags, golden) \
    do { \
        if (test_bm27_case2((flags), (golden)) < 0) { \
            return -1; \
        } \
    } while (0)

// Test N = 1, Z = 0, C = 0, V = 1 with CMP alias
static inline int test_bm27_case3(uint64_t imm, uint64_t flags)
{
    uint64_t x15 = A55_STL_BM27_INPUT_VALUE_6;
    uint64_t x2  = A55_STL_BM27_INPUT_VALUE_6;
    uint64_t x3  = A55_STL_BM27_INPUT_VALUE_7;
    uint64_t x4  = A55_STL_BM27_INPUT_VALUE_7;
    uint64_t x5  = A55_STL_BM27_INPUT_VALUE_6;
    uint64_t x6  = A55_STL_BM27_INPUT_VALUE_7;
    uint64_t x7  = A55_STL_BM27_INPUT_VALUE_6;
    uint64_t x8  = A55_STL_BM27_INPUT_VALUE_7;
    uint64_t x9  = A55_STL_BM27_INPUT_VALUE_6;
    uint64_t x10 = A55_STL_BM27_INPUT_VALUE_7;

    uint64_t x17, x18, x19, x21, x23;

    // 第一组
    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x18], %[x15], %[x3], uxtx %[imm_val]\n\t"
        "sub %[x19], %[x2], %[x4], uxtx %[imm_val]\n\t"
        "subs xzr, %[x5], %[x6], uxtx %[imm_val]\n\t"
        "mrs %[x17], nzcv\n"
        : [x18] "=&r"(x18), [x19] "=&r"(x19), [x17] "=&r"(x17)
        : [x15] "r"(x15), [x3] "r"(x3), [x2] "r"(x2), [x4] "r"(x4),
          [x5] "r"(x5), [x6] "r"(x6), [imm_val] "I"(imm)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM27_CASE3: NZCV1 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    // 第二组
    asm volatile(
        "msr nzcv, xzr\n\t"
        "sub %[x21], %[x7], %[x8], uxtx %[imm_val]\n\t"
        "subs xzr, %[x9], %[x10], uxtx %[imm_val]\n\t"
        "mov %[x23], %[x21]\n\t"
        "mrs %[x17], nzcv\n"
        : [x21] "=&r"(x21), [x23] "=&r"(x23), [x17] "=&r"(x17)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10),
          [imm_val] "I"(imm)
        : "cc"
    );

    if (x17 != flags) {
        pr_err("[%d] BM27_CASE3: NZCV2 got=0x%llx, expect=0x%llx\n", __LINE__, x17, flags);
        return -1;
    }

    return 0;
}

#define TEST_SUBS_EXTEND3(imm, flags) \
    do { \
        if (test_bm27_case3((imm), (flags)) < 0) { \
            return -1; \
        } \
    } while (0)

int a55_stl_core_diagnose_p009_n001_c_impl(void) {
    // BM09: SUB immediate
    TEST_SUB_IMM(
        A55_STL_BM09_INPUT_VALUE_1,
		A55_STL_BM09_IMM_VALUE_1,
		A55_STL_BM09_IMM_VALUE_2,
		A55_STL_BM09_IMM_VALUE_3,
		A55_STL_BM09_IMM_VALUE_4,
		A55_STL_BM09_GOLDEN_SIGN_1
	);
    TEST_SUB_IMM(
        A55_STL_BM09_INPUT_VALUE_2,
		A55_STL_BM09_IMM_VALUE_1,
		A55_STL_BM09_IMM_VALUE_2,
		A55_STL_BM09_IMM_VALUE_3,
		A55_STL_BM09_IMM_VALUE_4,
		A55_STL_BM09_GOLDEN_SIGN_2
	);

    // BM10: SUBS immediate, check NZCV
    TEST_SUBS_IMM(A55_STL_BM10_INPUT_VALUE_1, A55_STL_BM10_IMM_VALUE_1, A55_STL_BM10_GOLDEN_VALUE_1, A55_STL_BM10_GOLDEN_FLAGS_1);
    TEST_SUBS_IMM(A55_STL_BM10_INPUT_VALUE_2, A55_STL_BM10_IMM_VALUE_1, A55_STL_BM10_GOLDEN_VALUE_2, A55_STL_BM10_GOLDEN_FLAGS_2);
    TEST_SUBS_IMM(A55_STL_BM10_INPUT_VALUE_6, A55_STL_BM10_IMM_VALUE_1, A55_STL_BM10_GOLDEN_VALUE_3, A55_STL_BM10_GOLDEN_FLAGS_3);

    // BM22: SUB register
    TEST_SUB_REG(A55_STL_BM22_INPUT_VALUE_1, A55_STL_BM22_INPUT_VALUE_2, A55_STL_BM22_GOLDEN_VALUE_1);
    TEST_SUB_REG(A55_STL_BM22_INPUT_VALUE_3, A55_STL_BM22_INPUT_VALUE_4, A55_STL_BM22_GOLDEN_VALUE_2);
    TEST_SUB_REG(0, A55_STL_BM22_INPUT_VALUE_5, A55_STL_BM22_GOLDEN_VALUE_3); // NEG alias
    TEST_SUB_REG(0, A55_STL_BM22_INPUT_VALUE_6, A55_STL_BM22_GOLDEN_VALUE_4); // NEG alias

    // BM23: SUBS register, check NZCV, case 1
	TEST_SUBS_REG1(A55_STL_BM23_GOLDEN_FLAGS_1, A55_STL_BM23_GOLDEN_VALUE_1);
    // BM23: SUBS register, check NZCV, case 2
    TEST_SUBS_REG2(A55_STL_BM23_GOLDEN_FLAGS_2, A55_STL_BM23_GOLDEN_VALUE_2);
    // BM23: SUBS register, check NZCV, case 3
	TEST_SUBS_REG3(A55_STL_BM23_IMM_VALUE_1, A55_STL_BM23_IMM_VALUE_2, A55_STL_BM23_GOLDEN_FLAGS_2);
    // BM23: SUBS register, check NZCV, case 4
	TEST_SUBS_REG4(A55_STL_BM23_IMM_VALUE_3, A55_STL_BM23_GOLDEN_FLAGS_3, A55_STL_BM23_GOLDEN_VALUE_3);

    // BM26: SUB extended register
    TEST_SUB_EXTEND(A55_STL_BM26_INPUT_VALUE_1, A55_STL_BM26_INPUT_VALUE_2, 0, A55_STL_BM26_IMM_VALUE_1, A55_STL_BM26_GOLDEN_VALUE_1);
    TEST_SUB_EXTEND(A55_STL_BM26_INPUT_VALUE_3, A55_STL_BM26_INPUT_VALUE_4, 1, A55_STL_BM26_IMM_VALUE_2, A55_STL_BM26_GOLDEN_VALUE_2);
    TEST_SUB_EXTEND(A55_STL_BM26_INPUT_VALUE_5, A55_STL_BM26_INPUT_VALUE_6, 2, A55_STL_BM26_IMM_VALUE_1, A55_STL_BM26_GOLDEN_VALUE_3);
    TEST_SUB_EXTEND(A55_STL_BM26_INPUT_VALUE_7, A55_STL_BM26_INPUT_VALUE_8, 3, A55_STL_BM26_IMM_VALUE_1, A55_STL_BM26_GOLDEN_VALUE_4);

    // BM27: SUBS extended register, check NZCV
    TEST_SUBS_EXTEND1(A55_STL_BM27_GOLDEN_FLAGS_1, A55_STL_BM27_GOLDEN_VALUE_1);
    TEST_SUBS_EXTEND2(A55_STL_BM27_GOLDEN_FLAGS_2, A55_STL_BM27_GOLDEN_VALUE_2);
    TEST_SUBS_EXTEND3(A55_STL_BM27_IMM_VALUE_1, A55_STL_BM27_GOLDEN_FLAGS_3);

    return 0;
}

void a55_stl_core_diagnose_p009(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p009_n001_c_impl() != 0) {
        pr_info("A55 STL Core P009: Test failed\n");
    } else {
        pr_info("A55 STL Core P009: Test passed\n");
    }
}

